// dl.c - DL11 Serial Line Emulation
//
// Copyright (c) 2002, Timothy M. Stark
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

#include "dec/dl.h"

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

void dl_Accept(SOCKET *);
void dl_Eof(SOCKET *, int, int);
void dl_Input(SOCKET *, char *, int);
void dl_ReceiveDone(void *);
void dl_TransmitDone(void *);
void dl_CheckQueue(void *);

SOCKET *dl_InitServer(DL_TTY *dl)
{
	SOCKET    *newServer;
	CLK_QUEUE *timer;

	// Set up the listing socket for CTY device.
	if ((newServer = sock_Open("(DL11)", TTY_PORT, NET_SERVER)) == NULL) {
		printf("%s: Can't activate DL11 line - Aborted.\n", dl->Unit.devName);
		return NULL;
	}

	newServer->maxConns = 1;
	newServer->nConns   = 0;
	newServer->Accept   = dl_Accept;
	newServer->Eof      = NULL;
	newServer->Process  = NULL;
	newServer->Device   = (void *)dl;
	dl->Server          = newServer;

	// Set up receive timer
	timer           = &dl->rxTimer;
	timer->Next     = NULL;
	timer->Name     = "DL11 Receive Timer";
	timer->Flags    = 0;
	timer->outTimer = TTY_RDELAY;
	timer->nxtTimer = TTY_RDELAY;
	timer->Device   = (void *)dl;
	timer->Execute  = dl_ReceiveDone;

	// Set up transmit timer
	timer           = &dl->txTimer;
	timer->Next     = NULL;
	timer->Name     = "DL11 Transmit Timer";
	timer->Flags    = 0;
	timer->outTimer = TTY_TDELAY;
	timer->nxtTimer = TTY_TDELAY;
	timer->Device   = (void *)dl;
	timer->Execute  = dl_TransmitDone;

	// Set up queue timer
	timer           = &dl->qTimer;
	timer->Next     = NULL;
	timer->Name     = "DL11 Receive Queue Timer";
	timer->Flags    = 0;
	timer->outTimer = TTY_QDELAY;
	timer->nxtTimer = TTY_QDELAY;
	timer->Device   = (void *)dl;
	timer->Execute  = dl_CheckQueue;

	// Now accept incoming connections;
	sock_Listen(newServer, 5);

	// Tell operator that.
	printf("%s: Activating console on TCP port %d.\n",
		dl->Unit.devName, TTY_PORT);

	return newServer;
}

void dl_Cleanup(DL_TTY *dl)
{
	MAP_IO *io = &dl->ioMap;

	// Disable CTY device first.
	XCSR &= ~XCSR_RDY;

	// Cancel all timers.
	ts10_CancelTimer(&dl->txTimer);
	ts10_CancelTimer(&dl->rxTimer);
	ts10_CancelTimer(&dl->qTimer);

	// Cancel all pending interrupts.
	io->CancelInterrupt(io, RX);
	io->CancelInterrupt(io, TX);

	// Close and release all sockets.
	sock_Close(dl->Socket);
	sock_Close(dl->Server);

	free(dl);
}

void dl_ResetDevice(DL_TTY *dl)
{
}

void dl_ReceiveDone(void *dptr)
{
	DL_TTY *dl = (DL_TTY *)dptr;
	MAP_IO *io = &dl->ioMap;

	RCSR &= ~RCSR_ACT;
	RCSR |= RCSR_RDY;
	if (XCSR & XCSR_MAINT)
		XCSR |= XCSR_RDY;
	if (RCSR & RCSR_IE)
		io->SendInterrupt(io, RX);
}

void dl_TransmitDone(void *dptr)
{
	DL_TTY *dl = (DL_TTY *)dptr;
	MAP_IO *io = &dl->ioMap;

	XCSR |= XCSR_RDY;
	if (XCSR & XCSR_IE)
		io->SendInterrupt(io, TX);
}

void dl_Accept(SOCKET *srvSocket)
{
	DL_TTY *dl = (DL_TTY *)srvSocket->Device;
	MAP_IO *io = &dl->ioMap;
	SOCKET *newSocket;

	if ((newSocket = sock_Accept(srvSocket)) == NULL)
  		return;

	// First, check if CTY connection already was taken.
	// If so, tell operator that.
	if (dl->Socket != NULL) {

		sock_Print(newSocket, "Console (CTY) connection already was taken.\r\n", 0);
		sock_Print(newSocket, "Check other terminal which has that connection.\r\n", 0);
		sock_Print(newSocket, "\r\nTerminated.\r\n", 0);
		sock_Close(newSocket);

		return;
	}
 
	// Set up the TTY socket connection.
	newSocket->Accept  = NULL;
	newSocket->Eof     = dl_Eof;
	newSocket->Process = dl_Input;
	newSocket->Device  = (void *)dl;
	dl->Socket         = newSocket;

	// Reset all buffer for this new connection.
	dl->idxInQueue    = 0;
	dl->idxOutQueue   = 0;
	dl->idxOutBuffer  = 0;

	// Send initialization codes and welcome messages
	sock_Print(newSocket, telnetInit, n_telnetInit);
	sock_Print(newSocket, "Welcome to TS10 Emulator\r\n\r\n", 0);

	XCSR |= XCSR_RDY;
	if (XCSR & XCSR_IE)
		io->SendInterrupt(io, TX);
}

void dl_Eof(SOCKET *Socket, int rc, int nError)
{
	DL_TTY *dl = (DL_TTY *)Socket->Device;
	MAP_IO *io = &dl->ioMap;

	// Turn TTY device off.
	XCSR &= ~XCSR_RDY;

	// Cancel all pending interrupts.
	io->CancelInterrupt(io, RX);
	io->CancelInterrupt(io, TX);

	sock_Close(Socket);
	dl->Socket = NULL;
}

void dl_Input(SOCKET *Socket, char *keyBuffer, int len)
{
	DL_TTY  *dl = (DL_TTY *)Socket->Device;
	MAP_IO  *io = &dl->ioMap;
	int      okSend = FALSE;
	int     idx;

	// Process telnet codes and filter them out of data stream.
	if (len > 1) {
		if ((len = sock_ProcessTelnet((uchar *)keyBuffer, len)) == 0)
			return;
	}

	for (idx = 0; idx < len; idx++) {
		uchar ch = keyBuffer[idx];

		// Press ^\ twice to disconnect.
		if (ch == TTY_ESCAPE) {
			if (dl->lastSeen == TTY_ESCAPE) {
				dl_Eof(Socket, 0, 0);
				return;
			}
			dl->lastSeen = ch;
			continue;
		}

		// Convert CR NL to CR line.
		if ((ch == 012) && (dl->lastSeen == 015))
			continue;
		dl->lastSeen = ch;

		okSend = TRUE;
		dl->inBuffer[dl->idxInQueue] = ch;
		if (++dl->idxInQueue == TTY_BUFFER)
			dl->idxInQueue = 0;
		if (dl->idxInQueue == dl->idxOutQueue) {
#ifdef DEBUG
			dbg_Printf("%s: Error - Overrun!!\n", dl->Unit.devName);
#endif /* DEBUG */
			RBUF |= (RBUF_ERR|RBUF_OR);
			break;
		}
	}

	if (okSend && ((RCSR & RCSR_RDY) == 0)) {
		RBUF = dl->inBuffer[dl->idxOutQueue];
		if (++dl->idxOutQueue == TTY_BUFFER)
			dl->idxOutQueue = 0;
		RCSR |= RCSR_RDY;
		if (RCSR & RCSR_IE)
			io->SendInterrupt(io, RX);
	}

	// Activate queue timer if any characters still are
	// remaining in their input queue.
	if (dl->idxOutQueue != dl->idxInQueue)
		ts10_SetTimer(&dl->qTimer);
}

void dl_Output(DL_TTY *dl)
{
	char ch;

	XCSR &= ~XCSR_RDY;
	ch = XBUF;

	sock_Print(dl->Socket, &ch, 1);

	// Log a character into a log file.
	if (ch == '\n') {
		dl->outBuffer[dl->idxOutBuffer++] = ch;
		dl->outBuffer[dl->idxOutBuffer++] = '\0';
#ifdef DEBUG
		if (dbg_Check(DBG_CONSOLE))
			dbg_Printf("%s: %s", dl->Unit.devName, dl->outBuffer);
#endif /* DEBUG */
		if (emu_logFile >= 0)
			write(emu_logFile, dl->outBuffer, dl->idxOutBuffer);
		dl->idxOutBuffer = 0;
	} else {
		if (ch == '\b' || ch == 127) {
			if (dl->idxOutBuffer > 0)
				dl->idxOutBuffer--;
		} else if (ch != '\r' && ch != '\0') {
			if (dl->idxOutBuffer < TTY_BUFFER-1)
				dl->outBuffer[dl->idxOutBuffer++] = ch;
		}
	}

	// When done, set ready and tell system that
	// ready to receive another character.
	ts10_SetTimer(&dl->txTimer);
}

// Check Queue for Console TTY
void dl_CheckQueue(void *dptr)
{
	DL_TTY *dl = (DL_TTY *)dptr;
	MAP_IO *io = &dl->ioMap;

	if (dl->idxOutQueue == dl->idxInQueue)
		return;

	if ((RCSR & RCSR_RDY) == 0) {
		RBUF = dl->inBuffer[dl->idxOutQueue];
		if (++dl->idxOutQueue == TTY_BUFFER)
			dl->idxOutQueue = 0;
		RCSR |= RCSR_RDY;
		if (RCSR & RCSR_IE)
			io->SendInterrupt(io, RX);
	}

	// Reactivate queue timer if characters still are
	// remaining in their input queue.
	if (dl->idxOutQueue != dl->idxInQueue)
		ts10_SetTimer(&dl->qTimer);
}

int dl_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	DL_TTY *dl = (DL_TTY *)dptr;
	int    reg = (pAddr >> 1) & 3;

	switch(reg) {
		case nRCSR: // Receive Control/Status Register
			*data = RCSR;
			break;

		case nRBUF: // Receive Data Buffer
			// Perform maintenance if bit set.
			if (XCSR & XCSR_MAINT) {
				// Loopback if maintenance bit is set.
				if (--dl->mCount > 0) {
					RCSR |= RCSR_RDY;
					ts10_SetTimer(&dl->rxTimer);
				}
				ts10_SetTimer(&dl->txTimer);
			}

			// Get data from receive buffer.
			*data = RBUF;

			// Clear RBUF and READY bit in RCSR after read access.
			RBUF  = 0;
			RCSR &= ~RCSR_RDY;
			break;

		case nXCSR: // Transmit Control/Status Register
			*data = XCSR;
			break;

		case nXBUF: // Transmit Data Buffer
			// Write-Only Register - Do Nothing
			break;
	}

	return UQ_OK;
}

int dl_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	DL_TTY *dl = (DL_TTY *)dptr;
	MAP_IO *io = &dl->ioMap;
	int    reg = (pAddr >> 1) & 3;

	switch(reg) {
		case nRCSR: // Receive Control/Status Register
			if ((data & RCSR_IE) == 0)
				io->CancelInterrupt(io, RX);
			else if ((RCSR & (RCSR_RDY|RCSR_IE)) == RCSR_RDY)
				io->SendInterrupt(io, RX);
			RCSR = (data & RCSR_RW) | (RCSR & ~RCSR_RW);
			break;

		case nRBUF: // Receive Data Buffer
			RBUF = data;
			break;

		case nXCSR: // Transmit Control/Status Register
			if ((data & XCSR_IE) == 0)
				io->CancelInterrupt(io, TX);
			else if ((XCSR & (XCSR_RDY|XCSR_IE)) == XCSR_RDY)
				io->SendInterrupt(io, TX);
			XCSR = (data & XCSR_RW) | (XCSR & ~XCSR_RW);
			break;

		case nXBUF: // Transmit Data Buffer
			if (XCSR & XCSR_MAINT) {
				// Loopback if maintenance bit is set.
				RCSR |= RCSR_ACT;
				RBUF  = data & RBUF_DATA;
				dl->mCount++;
				ts10_SetTimer(&dl->rxTimer);
			}

			XCSR &= ~XCSR_RDY;
			XBUF  = data;
			if (dl->Socket)
				dl_Output(dl);
			break;
	}

	return UQ_OK;
}

// Bus Initialization
void dl_ResetIO(void *dptr)
{
	dl_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// **************************************************************

void *dl_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	DL_TTY *dl = NULL;
	MAP_IO *io;

	if (dl = (DL_TTY *)calloc(1, sizeof(DL_TTY))) {
		// First, set up its description and
		// link it to its parent device.
		dl->Unit.devName    = newMap->devName;
		dl->Unit.keyName    = newMap->keyName;
		dl->Unit.emuName    = newMap->emuName;
		dl->Unit.emuVersion = newMap->emuVersion;
		dl->Device          = newMap->devParent->Device;
		dl->Callback        = newMap->devParent->Callback;
		dl->System          = newMap->devParent->sysDevice;

#if 0
		// Recognize which model - DL11.
		if (!strcmp(dl->Unit.keyName, DL_KEY)) {
			dl->Flags  = CFLG_DL;
		else {
			printf("%s: *** Bug Check: Unknown device name - %s\n",
				dl->Unit.devName, dl->Unit.keyName);
			free(dl);
			return NULL;
		}
#endif

		dl->csrAddr = DL_CSRADR;

		// Initialize Server, TTY lines, etc.
		if (dl_InitServer(dl) == NULL) {
			free(dl);
			return NULL;
		}
		dl_ResetDevice(dl);

		// Set up an I/O space.
		io                = &dl->ioMap;
		io->devName       = dl->Unit.devName;
		io->keyName       = dl->Unit.keyName;
		io->emuName       = dl->Unit.emuName;
		io->emuVersion    = dl->Unit.emuVersion;
		io->Device        = dl;
		io->csrAddr       = dl->csrAddr;
		io->nRegs         = DL_NREGS;
		io->nVectors      = DL_NVECS;
		io->intIPL        = DL_IPL;
		io->intVector[RX] = DL_RXVEC;
		io->intVector[TX] = DL_TXVEC;
		io->ReadIO        = dl_ReadIO;
		io->WriteIO       = dl_WriteIO;
		io->ResetIO       = dl_ResetIO;

		// Assign that registers to QBA's I/O space.
		dl->Callback->SetMap(dl->Device, io);

		// Finally, link it to its mapping device and return.
		newMap->Device = dl;
	}

	return dl;
}

//int dhu_Reset(MAP_DEVICE *map)
int dl_Reset(void *dptr)
{
	dl_ResetDevice(dptr);
}

DEVICE dl_Device =
{
	DL_KEY,           // Key Name
	DL_NAME,          // Emulator Name
	DL_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	dl_Create,        // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	dl_Reset,         // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
