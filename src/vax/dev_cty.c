// dev_cty.c - Console TTY routines for VAX system
//
// Copyright (c) 2001-2002, Timothy M. Stark
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// TIMOTHY M STARK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Timothy M Stark shall not
// be used in advertising or otherwise to promote the sale, use or other 
// dealings in this Software without prior written authorization from
// Timothy M Stark.

// Device: RXCS, RXDB, TXCS, TXDB

#include "vax/defs.h"
#include "vax/dev_cty.h"

#ifdef HAVE_SIGACTION
#include <sys/file.h>
#include <unistd.h>
#include <stropts.h>
#include <signal.h>
#endif /* HAVE_SIGACTION */

// Send initialization codes to make sure that telnet behave correctly.
static int  n_telnetInit = 15;
static char telnetInit[] =
{
	255, 251,  34, // IAC WILL LINEMODE
	255, 251,   3, // IAC WILL SGA
	255, 251,   1, // IAC WILL ECHO
	255, 251,   0, // IAC WILL BINARY
	255, 253,   0, // IAC DO BINARY
};

void vax_ConsoleAccept(SOCKET *);
void vax_ConsoleEof(SOCKET *, int, int);
void vax_ConsoleInput(SOCKET *, char *, int);
void vax_ConsoleReceiveDone(void *);
void vax_ConsoleTransmitDone(void *);
void vax_ConsoleCheck(void *);

VAX_CONSOLE *vax_ConsoleInit(VAX_CPU *vax)
{
	VAX_CONSOLE *cty;
	SOCKET      *newServer;
	CLK_QUEUE   *Clock;

	if((cty = (VAX_CONSOLE *)calloc(1, sizeof(VAX_CONSOLE))) == NULL)
		return NULL;

	// Belong to its processor.
	cty->Processor = vax;
	cty->mCount    = 0;

	// Set up receive timer
	if (Clock = (CLK_QUEUE *)calloc(1, sizeof(CLK_QUEUE))) {
		Clock->Next     = NULL;
		Clock->Name     = "Console Receive Timer";
		Clock->Flags    = 0;
		Clock->outTimer = CTY_RDELAY;
		Clock->nxtTimer = CTY_RDELAY;
		Clock->Device   = (void *)cty;
		Clock->Execute  = vax_ConsoleReceiveDone;
		cty->rxTimer    = Clock;
	}

	// Set up transmit timer
	if (Clock = (CLK_QUEUE *)calloc(1, sizeof(CLK_QUEUE))) {
		Clock->Next     = NULL;
		Clock->Name     = "Console Transmit Timer";
		Clock->Flags    = 0;
		Clock->outTimer = CTY_TDELAY;
		Clock->nxtTimer = CTY_TDELAY;
		Clock->Device   = (void *)cty;
		Clock->Execute  = vax_ConsoleTransmitDone;
		cty->txTimer    = Clock;
	}

	// Set up queue timer
	if (Clock = (CLK_QUEUE *)calloc(1, sizeof(CLK_QUEUE))) {
		Clock->Next     = NULL;
		Clock->Name     = "Console Receive Queue Timer";
		Clock->Flags    = 0;
		Clock->outTimer = CTY_QDELAY;
		Clock->nxtTimer = CTY_QDELAY;
		Clock->Device   = (void *)cty;
		Clock->Execute  = vax_ConsoleCheck;
		cty->qTimer     = Clock;
	}

	// Set up the listing socket for CTY device.
	if (newServer = sock_Open("(vax/cty)", CTY_PORT, NET_SERVER)) {
		newServer->maxConns = 1;
		newServer->nConns   = 0;
		newServer->Accept   = vax_ConsoleAccept;
		newServer->Eof      = NULL;
		newServer->Process  = NULL;
		newServer->Device   = (void *)cty;
		cty->Server         = newServer;

		// Now accept incoming connections;
		sock_Listen(newServer, 5);

		// Tell operator that.
		printf("Activating console on TCP port %d.\n", CTY_PORT);
	} else {
		printf("Can't activate console - Aborted.\n");
		free(cty);
		return NULL;
	}

	return cty;
}

void vax_ConsoleInit2(VAX_CONSOLE *cty, UQ_CALL *cb)
{
	VAX_CPU *vax = cty->Processor;
	MAP_IO  *io  = &cty->ioMap;

	io->devName       = "CTY";
	io->keyName       = "DL11";
	io->emuName       = "Console Terminal";
	io->emuVersion    = NULL;
	io->nVectors      = CTY_NVECS;
	io->intIPL        = CTY_IPL;
	io->intVector[RX] = CTY_RXVEC;
	io->intVector[TX] = CTY_TXVEC;
	cb->SetMap(vax->uqba, io);
}

void vax_ConsoleCleanup(VAX_CONSOLE *cty)
{
	VAX_CPU *vax = cty->Processor;

	// Disable CTY device first.
	TXCS &= ~TXCS_RDY;

	// Close and release all sockets.
	sock_Close(cty->Socket);
	sock_Close(cty->Server);
	cty->Socket = NULL;
	cty->Server = NULL;

	// Now release CTY device.
	vax->Console = NULL;
	free(cty);
}

void vax_ConsoleReceiveDone(void *dev)
{
	VAX_CONSOLE *cty = (VAX_CONSOLE *)dev;
	VAX_CPU     *vax = (VAX_CPU *)cty->Processor;
	MAP_IO      *io  = &cty->ioMap;

	// Send an interrupt to VAX processor.
	RXCS &= ~RXCS_ACT;
	if (RXCS & RXCS_RDY)
		RXDB |= (RXDB_ERR|RXDB_OVR);
	RXCS |= RXCS_RDY;
	if (TXCS & TXCS_MAINT)
		TXCS |= TXCS_RDY;
	if (RXCS & RXCS_IE)
		io->SendInterrupt(io, RX);
}

void vax_ConsoleTransmitDone(void *dev)
{
	VAX_CONSOLE *cty = (VAX_CONSOLE *)dev;
	VAX_CPU     *vax = (VAX_CPU *)cty->Processor;
	MAP_IO      *io  = &cty->ioMap;

	// Send an interrupt to VAX processor.
	TXCS |= TXCS_RDY;
	if (TXCS & TXCS_IE)
		io->SendInterrupt(io, TX);
}

void vax_ConsoleAccept(SOCKET *srvSocket)
{
	VAX_CONSOLE *cty = (VAX_CONSOLE *)srvSocket->Device;
	VAX_CPU     *vax = cty->Processor;
	MAP_IO      *io  = &cty->ioMap;
	SOCKET      *newSocket;

	if (newSocket = sock_Accept(srvSocket)) {
		// First, check if CTY connection already was taken.
		// If so, tell operator that.
		if (cty->Socket != NULL) {

			sock_Print(newSocket, "Console (CTY) connection already was taken.\r\n", 0);
			sock_Print(newSocket, "Check other terminal which has that connection.\r\n", 0);
			sock_Print(newSocket, "\r\nTerminated.\r\n", 0);
			sock_Close(newSocket);

			return;
		}
 
		// Set up the CTY socket connection.
		newSocket->Accept  = NULL;
		newSocket->Eof     = vax_ConsoleEof;
		newSocket->Process = vax_ConsoleInput;
		newSocket->Device  = (void *)cty;
		cty->Socket        = newSocket;

		// Reset all buffer for this new connection.
		cty->idxInQueue    = 0;
		cty->idxOutQueue   = 0;
		cty->idxOutBuffer  = 0;

		// Send initialization codes and welcome messages
		sock_Print(newSocket, telnetInit, n_telnetInit);
		sock_Print(newSocket, "Welcome to VAX Emulator\r\n\r\n", 0);

		// Tell VAX system that CTY device is ready
		// so that VAX now can start to output data.
		// Note: delayed I/O is not necessary.
		TXCS |= TXCS_RDY;
		if (TXCS & TXCS_IE)
			io->SendInterrupt(io, TX);
	}
}

void vax_ConsoleEof(SOCKET *Socket, int rc, int nError)
{
	VAX_CONSOLE *cty = (VAX_CONSOLE *)Socket->Device;
	VAX_CPU     *vax = cty->Processor;

	// Turn CTY device off.
	TXCS &= ~TXCS_RDY;

	sock_Close(Socket);
	cty->Socket = NULL;
}

void vax_ConsoleInput(SOCKET *Socket, char *keyBuffer, int len)
{
	VAX_CONSOLE *cty = (VAX_CONSOLE *)Socket->Device;
	VAX_CPU     *vax = cty->Processor;
	MAP_IO      *io  = &cty->ioMap;
	int         okSend = FALSE;
	int         idx;

	// Process telnet codes and filter them out of data stream.
	if (len > 1) {
		if ((len = sock_ProcessTelnet((uchar *)keyBuffer, len)) == 0)
			return;
	}

	for (idx = 0; idx < len; idx++) {
		uchar ch = keyBuffer[idx];

		// Press ^\ twice to disconnect.
		if (ch == CTY_ESCAPE) {
			if (cty->lastSeen == CTY_ESCAPE) {
				vax_ConsoleEof(Socket, 0, 0);
				return;
			}
			cty->lastSeen = ch;
			continue;
		}

		// Convert CR NL to CR line.
		if ((ch == 012) && (cty->lastSeen == 015))
			continue;
		cty->lastSeen = ch;

		// If Ctrl-P is depressed, signal processor to halt
		// and return back to console program in ROM.
		if (ch == CTY_HALT) {
			if (vax->HaltAction) {
				emu_State = VAX_SWHALT;
				continue;
			}
		}

		okSend = TRUE;
		cty->inBuffer[cty->idxInQueue] = ch;
		if (++cty->idxInQueue == CTY_BUFFER)
			cty->idxInQueue = 0;
		if (cty->idxInQueue == cty->idxOutQueue) {
			// When data overrun occurs, rest of input buffer is lost.
			// Also tell VAX system that it is data overrun.
#ifdef DEBUG
			dbg_Printf("CTY: Error - Overrun!!\n");
#endif /* DEBUG */
			RXDB |= (RXDB_ERR|RXDB_OVR); // Tell VAX that data overrun
			break;
		}
	}

	if (okSend && ((RXCS & RXCS_RDY) == 0)) {
		RXDB = cty->inBuffer[cty->idxOutQueue];
		if (++cty->idxOutQueue == CTY_BUFFER)
			cty->idxOutQueue = 0;
		if ((RXDB & RXDB_CHAR) == 0)
			RXDB |= RXDB_BRK;
		RXCS |= RXCS_RDY;
		if (RXCS & RXCS_IE)
			io->SendInterrupt(io, RX);
	}

	// Activate queue timer if any characters still are
	// remaining in their input queue.
	if (cty->idxOutQueue != cty->idxInQueue)
		ts10_SetTimer(cty->qTimer);
}

void vax_ConsoleOutput(VAX_CONSOLE *cty)
{
	VAX_CPU *vax = cty->Processor;
	char    ch;

	if (cty->Socket) {
		TXCS &= ~TXCS_RDY;
		ch = TXDB;

		sock_Print(cty->Socket, &ch, 1);

		// Log a character into a log file.
		if (ch == '\n') {
			cty->outBuffer[cty->idxOutBuffer++] = ch;
			cty->outBuffer[cty->idxOutBuffer++] = '\0';
#ifdef DEBUG
			if (dbg_Check(DBG_CONSOLE))
				dbg_Printf("CTY: %s", cty->outBuffer);
//			if (!strncmp(cty->outBuffer, "KA655-B", 7)) {
//				printf("[Trace Turned On]\n");
//				dbg_SetMode(DBG_TRACE|DBG_DATA|DBG_INTERRUPT);
//			}
#endif /* DEBUG */
			if (emu_logFile >= 0)
				write(emu_logFile, cty->outBuffer, cty->idxOutBuffer);
			cty->idxOutBuffer = 0;
		} else {
			if (ch == '\b' || ch == 127) {
				if (cty->idxOutBuffer > 0)
					cty->idxOutBuffer--;
			} else if (ch != '\r' && ch != '\0') {
				if (cty->idxOutBuffer < CTY_BUFFER-1)
					cty->outBuffer[cty->idxOutBuffer++] = ch;
			}
		}

		// When done, set ready and tell VAX system that
		// ready to receive another character.
		ts10_SetTimer(cty->txTimer);
	}
}

// Check Queue for Console TTY
void vax_ConsoleCheck(void *dev)
{
	VAX_CONSOLE *cty = (VAX_CONSOLE *)dev;
	VAX_CPU     *vax = cty->Processor;
	MAP_IO      *io  = &cty->ioMap;

	if (cty == NULL)
		return;
	vax = cty->Processor;

	if (cty->idxOutQueue != cty->idxInQueue) {
		if ((RXCS & RXCS_RDY) == 0) {
			RXDB = cty->inBuffer[cty->idxOutQueue];
			if (++cty->idxOutQueue == CTY_BUFFER)
				cty->idxOutQueue = 0;
			RXCS |= RXCS_RDY;
			if (RXCS & RXCS_IE)
				io->SendInterrupt(io, RX);
		}

		// Reactivate queue timer if characters still are
		// remaining in their input queue.
		if (cty->idxOutQueue != cty->idxInQueue)
			ts10_SetTimer(cty->qTimer);
	}
}

// **********************************************
// ****** Privileged (Processor) Register *******
// **********************************************
// Routines are for MicroVAX series only at this time.

// RXCS - Console Receive Control and Status Register

void vax_WriteRXCS(VAX_CPU *vax, uint32 data)
{
	VAX_CONSOLE *cty = vax->Console;
	MAP_IO      *io  = &cty->ioMap;

	if ((data & RXCS_IE) == 0)
		io->CancelInterrupt(io, RX);
	else if ((RXCS & (RXCS_RDY|RXCS_IE)) == RXCS_RDY)
		io->SendInterrupt(io, RX);
	RXCS = (RXCS & ~RXCS_WMASK) | (data & RXCS_WMASK);
}

uint32 vax_ReadRXCS(VAX_CPU *vax)
{
	VAX_CONSOLE *cty = vax->Console;

	return RXCS;
}

// RXDB - Console Receive Data Buffer Register

uint32 vax_ReadRXDB(VAX_CPU *vax)
{
	VAX_CONSOLE *cty = vax->Console;
	uint32      data = RXDB;

	// Clear RXDB and reset READY in RXCS by reading RXDB.
	RXDB = 0; // Cleared by reading RXDB
	RXCS &= ~RXCS_RDY;
	if (TXCS & TXCS_MAINT) {
		// Loopback if maintenance bit is set.
		if (--cty->mCount > 0) {
			RXCS |= RXCS_RDY;
			ts10_SetTimer(cty->rxTimer);
		}
		ts10_SetTimer(cty->txTimer);
	}

	return data;
}

// TXCS - Console Transmit Control and Status Register

void vax_WriteTXCS(VAX_CPU *vax, uint32 data)
{
	VAX_CONSOLE *cty = vax->Console;
	MAP_IO      *io  = &cty->ioMap;

	if ((data & TXCS_IE) == 0)
		io->CancelInterrupt(io, TX);
	else if ((TXCS & (TXCS_RDY|TXCS_IE)) == TXCS_RDY)
		io->SendInterrupt(io, TX);
	TXCS = (TXCS & ~TXCS_WMASK) | (data & TXCS_WMASK);
}

uint32 vax_ReadTXCS(VAX_CPU *vax)
{
	VAX_CONSOLE *cty = vax->Console;

	return TXCS;
}

// TXDB - Console Transmit Data Buffer Register

void vax_WriteTXDB(VAX_CPU *vax, uint32 data)
{
	VAX_CONSOLE *cty = vax->Console;

	TXCS &= ~TXCS_RDY;
	TXDB = data;
	if (TXCS & TXCS_MAINT) {
		// Loopback if maintenance bit is set.
		RXCS |= RXCS_ACT;
		RXDB = data & RXDB_CHAR;
		cty->mCount++;
		ts10_SetTimer(cty->rxTimer);
	} else if (vax->Console) {
		vax_ConsoleOutput(vax->Console);
	}
}
