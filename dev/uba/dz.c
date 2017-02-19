// dz.c - DZ11/DZV11 Terminal Communications
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

#include "emu/defs.h"
#include "dec/dz.h"

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

void dz_Accept(SOCKET *);
void dz_Eof(SOCKET *, int, int);
void dz_Input(SOCKET *, char *, int);
void dz_ReceiveDone(void *);
void dz_TransmitDone(void *);
void dz_Queue(void *);

// Inline Macro definitions here
inline void dz_RingRX(DZ_MUX *mux)
{
	DZ_DEVICE *dz = mux->Device;
	MAP_IO    *io = &mux->ioMap;

	if (CSR & CSR_RIE) {
		io->SendInterrupt(io, RX);
#ifdef DEBUG
		dbg_Printf("%s: Ring for receive interrupt.\n", dz->Unit.devName);
#endif /* DEBUG */
	}
}

inline void dz_RingTX(DZ_MUX *mux)
{
	DZ_DEVICE *dz = mux->Device;
	MAP_IO    *io = &mux->ioMap;

	if (CSR & CSR_TIE) {
		io->SendInterrupt(io, TX);
#ifdef DEBUG
		dbg_Printf("%s: Ring for transmit interrupt.\n", dz->Unit.devName);
#endif /* DEBUG */
	}
}

#if 0
inline void dz_UpdateStatus(DZ_TTY *tty, uint16 set, uint16 clr)
{
	DZ_DEVICE *dz = tty->Device;
	uint16    rx;
	
	// Update line status register.
	STAT = (STAT | set) & ~clr;

	// If a link type is set, put that in receiving FIFO.
	if (LNC & LNC_LINK) {
		rx = (RBUF_VALID|RBUF_STATUS) | ((tty->idPort) << 8) |
			((STAT >> 8) & ~1);
		dz->rbuf[dz->idxIn++] = rx;
		if (dz->idxIn >= LEN_RBUF)
			dz->idxIn = 0;
	}

	// Finally, ring host for receive interrupt.
	dz_RingRX(dz);
}
#endif

// Initialize DZ11 Server
SOCKET *dz_InitServer(DZ_DEVICE *dz)
{
	SOCKET *newServer = NULL;

#if 0
	CLK_QUEUE   *Clock;

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
#endif

	// Set up the listing socket for CTY device.
	if (newServer = sock_Open(dz->Unit.keyName, TTY_PORT, NET_SERVER)) {
		newServer->maxConns = dz->nLines;
		newServer->nConns   = 0;
		newServer->Accept   = dz_Accept;
		newServer->Eof      = NULL;
		newServer->Process  = NULL;
		newServer->Device   = dz;
		dz->Server          = newServer;

		// Now accept incoming connections;
		sock_Listen(newServer, 5);

		// Tell operator that.
		printf("Activating %s: (%s) on TCP port %d.\n",
			dz->Unit.devName, dz->Unit.keyName, TTY_PORT);
	} else {
		printf("Can't activate %s: (%s) - Aborted.\n",
			dz->Unit.devName, dz->Unit.keyName);
	}

	return newServer;
}

// Initialize TTY lines
void dz_InitLines(DZ_DEVICE *dz)
{
	DZ_MUX    *mux;
	DZ_TTY    *tty;
	CLK_QUEUE *timer;
	int       idx1, idx2;
	int       cntLines = 0;

	// Initialize global TTY lines
	cntLines = 0;
	for (idx1 = 0; idx1 < dz->nMuxes; idx1++) {
		mux = &dz->Muxes[idx1];
		mux->Device = dz;
		mux->nLines = dz->nPorts;
		mux->idMux  = idx1;

		// Initialize local TTY lines
		for (idx2 = 0; idx2 < mux->nLines; idx2++) {
			tty = &mux->Lines[idx2];

			// Set TTY settings
			tty->Flags  = TTY_EXIST;
			tty->Device = dz;
			tty->Mux    = mux;
			tty->idMux  = idx1;
			tty->idPort = idx2;
			tty->idLine = cntLines;

			dz->Lines[cntLines++] = tty;

			timer           = &tty->qTimer;
			timer->Next     = NULL;
			timer->Name     = "DZ11 Queue Timer";
			timer->Flags    = 0;
			timer->outTimer = TTY_QDELAY;
			timer->nxtTimer = TTY_QDELAY;
			timer->Device   = (void *)tty;
			timer->Execute  = dz_Queue;
		}
	}

	// Set total number of lines
	dz->nLines = cntLines;
}

void dz_Cleanup(DZ_DEVICE *dz)
{
}

#if 0
void dz_ConsoleReceiveDone(void *dev)
{
	VAX_CONSOLE *cty = (VAX_CONSOLE *)dev;
	VAX_CPU     *vax = (VAX_CPU *)cty->Processor;

	// Send an interrupt to VAX processor.
	RXCS &= ~RXCS_ACT;
	if (RXCS & RXCS_RDY)
		RXDB |= (RXDB_ERR|RXDB_OVR);
	RXCS |= RXCS_RDY;
	if (TXCS & TXCS_MAINT)
		TXCS |= TXCS_RDY;
	if (RXCS & RXCS_IE) {
		HIRQ |= RXCS_INT;
		SET_IRQ;
	}
}

void vax_ConsoleTransmitDone(void *dev)
{
	VAX_CONSOLE *cty = (VAX_CONSOLE *)dev;
	VAX_CPU     *vax = (VAX_CPU *)cty->Processor;

	// Send an interrupt to VAX processor.
	TXCS |= TXCS_RDY;
	if (TXCS & TXCS_IE) {
		HIRQ |= TXCS_INT;
		SET_IRQ;
	}
}
#endif

void dz_Accept(SOCKET *srvSocket)
{
	DZ_DEVICE *dz  = (DZ_DEVICE *)srvSocket->Device;
	DZ_TTY    *tty = NULL;
	DZ_MUX    *mux;
	SOCKET    *newSocket;
	uint32    idx;

	if (newSocket = sock_Accept(srvSocket)) {
		// Find a free tty line to assign.
		for (idx = 0; idx < dz->nLines; idx++) {
			if (dz->Lines[idx]->Socket == NULL) {
				tty = dz->Lines[idx];
				break;
			}
		}

		// If a tty line not found, tell user that.
		if (tty == NULL) {

			sock_Print(newSocket, "All circuits are busy. Try again later.\r\n", 0);
			sock_Print(newSocket, "\r\nTerminated.\r\n", 0);
			sock_Close(newSocket);

			return;
		}
 
		mux = tty->Mux;
		if ((CSR & CSR_MSE) == 0) {
			sock_Print(newSocket, "Sorry - MUX is busy.\r\n\r\n", 0);
			sock_Close(newSocket);
		}

		// Set up the CTY socket connection.
		newSocket->Accept  = NULL;
		newSocket->Eof     = dz_Eof;
		newSocket->Process = dz_Input;
		newSocket->Device  = tty;
		tty->Socket        = newSocket;

		// Reset all buffer for this new connection.
		tty->idxInQueue    = 0;
		tty->idxOutQueue   = 0;
		tty->idxOutBuffer  = 0;

		// Send initialization codes and welcome messages
		sock_Print(newSocket, telnetInit, n_telnetInit);
		sock_Print(newSocket, "Welcome to TS10 Emulator\r\n\r\n", 0);
		sock_Print(newSocket, "Connecting...", 0);

		// Finally, set CD or RI bit depending on DTR bit set
		// in order to ring host.
		MSR |= (TCR & LINE(TCR_DTR)) ? LINE(MSR_CD) : LINE(MSR_RI);
	}
}

void dz_Eof(SOCKET *Socket, int rc, int nError)
{
	DZ_TTY *tty = (DZ_TTY *)Socket->Device;

	// Drop CTS, DCD and DSR bit to sign off.

	sock_Close(Socket);
	tty->Socket = NULL;
}

void dz_Input(SOCKET *Socket, char *keyBuffer, int len)
{
	DZ_TTY    *tty   = (DZ_TTY *)Socket->Device;
	DZ_DEVICE *dz    = tty->Device;
	DZ_MUX    *mux   = tty->Mux;
	int       okSend = FALSE;
	uint16    rx;
	uchar     ch;
	int       idx;

	// Process telnet codes and filter them out of data stream.
	if (len > 1) {
		if ((len = sock_ProcessTelnet((uchar *)keyBuffer, len)) == 0)
			return;
	}

	for (idx = 0; idx < len; idx++) {
		ch = keyBuffer[idx];

		// Press ^\ twice to disconnect.
		if (ch == TTY_ESCAPE) {
			if (tty->lastSeen == TTY_ESCAPE) {
				dz_Eof(Socket, 0, 0);
				return;
			}
			tty->lastSeen = ch;
			continue;
		}

		// Convert CR NL to CR line.
		if ((ch == 012) && (tty->lastSeen == 015))
			continue;
		tty->lastSeen = ch;

		okSend = TRUE;
		tty->inBuffer[tty->idxInQueue] = ch;
		if (++tty->idxInQueue == TTY_BUFFER)
			tty->idxInQueue = 0;
		if (tty->idxInQueue == tty->idxOutQueue) {
			// When data overrun occurs, rest of input buffer is lost.
			// Also tell VAX system that it is data overrun.
			break;
		}
	}

	if (okSend) {
		// Dequeue a oldest character from internal receive buffer.
		ch = tty->inBuffer[tty->idxOutQueue++];
		if (tty->idxOutQueue == TTY_BUFFER)
			tty->idxOutQueue = 0;

		// Put that in receiving FIFO buffer and
		// ring host for receive interrupt.
//		rx = RBUF_VALID | ((tty->idPort) << 8) | ch;
//		dz->rbuf[dz->idxIn++] = rx;
//		if (dz->idxIn >= LEN_RBUF)
//			dz->idxIn = 0;
//		dz->csr |= CSR_RXAVAIL;
//		dz_RingRX(dz);
	}

	// Activate queue timer if any characters still are
	// remaining in their input queue.
	if (tty->idxOutQueue != tty->idxInQueue)
		ts10_SetTimer(&tty->qTimer);
}

void dz_Output(DZ_TTY *tty)
{
	DZ_DEVICE *dz  = tty->Device;
	DZ_MUX    *mux = tty->Mux;
	char      ch;

//	if (tty->Socket && (TXCH & TXCH_VALID)) {
	if (tty->Socket) {
		ch = TDR;
		sock_Print(tty->Socket, &ch, 1);

		// Done, ring host for next transmit.
//		TXCH &= ~TXCH_VALID;
//		CSR  = (CSR & ~CSR_TXLINE) | (tty->idPort << 8) | CSR_TXACT;
		dz_RingTX(mux);

#if 0
		// Log a character into a log file.
		if (ch == '\n') {
			tty->outBuffer[tty->idxOutBuffer++] = ch;
			tty->outBuffer[tty->idxOutBuffer++] = '\0';
			// Implement: Log session here.
			tty->idxOutBuffer = 0;
		} else {
			if (ch == '\b' || ch == 127) {
				if (tty->idxOutBuffer > 0)
					tty->idxOutBuffer--;
			} else if (ch != '\r' && ch != '\0') {
				if (tty->idxOutBuffer < TTY_BUFFER-1)
					tty->outBuffer[tty->idxOutBuffer++] = ch;
			}
		}
#endif 

		// When done, set ready and tell VAX system that
		// ready to receive another character.
//		ts10_SetTimer(cty->txTimer);
//		TXCS |= TXCS_RDY;
//		if (TXCS & TXCS_IE) {
//			HIRQ |= TXCS_INT;
//			SET_IRQ;
//		}
	}
}

// Check Queue for this TTY line.
void dz_Queue(void *dev)
{
	DZ_TTY      *tty = (DZ_TTY *)dev;
	DZ_DEVICE   *dz  = tty->Device;
	uint16      rx;
	uchar       ch;

	if (tty->idxOutQueue != tty->idxInQueue) {
		// Dequeue a oldest character from internal receive buffer.
		ch = tty->inBuffer[tty->idxOutQueue++];
		if (tty->idxOutQueue == TTY_BUFFER)
			tty->idxOutQueue = 0;

		// Finally, put data into FIFO receive buffer and
		// ring host for receive interrupt.
//		rx = RBUF_VALID | ((tty->idPort) << 8) | ch;
//		dz->rbuf[dz->idxIn++] = rx;
//		if (dz->idxIn >= LEN_RBUF)
//			dz->idxIn = 0;
//		dz->csr |= CSR_RXAVAIL;
//		dz_RingRX(mux);

		// Reactivate queue timer if characters still are
		// remaining in their input queue.
		if (tty->idxOutQueue != tty->idxInQueue)
			ts10_SetTimer(&tty->qTimer);
	}
}

// ********************************************************************

void dz_ResetDevice(DZ_MUX *mux)
{
	CSR  = 0;
	RBUF = 0;
	LPR  = 0;
	TDR  = 0;
	TCR  = 0;
}

int dz_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	uint32    reg  = (pAddr >> 1) & 03;
	DZ_DEVICE *dz  = (DZ_DEVICE *)dptr;
	DZ_MUX    *mux = &dz->Muxes[(pAddr >> 3) & DZ_MUXMASK];
	MAP_IO    *io  = &mux->ioMap;
	DZ_TTY    *tty;

	switch (reg) {
		case DZ_CSR: // Control and Status Register
			if (acc == ACC_BYTE) // if access is byte, merge with DZCSR.
				data = (pAddr & 1) ? ((data << 8)   | (CSR & 0377)) :
				                     ((data & 0377) | (CSR & ~0377));

			// Reset all DZ device if request.
			if (data & CSR_CLR)
				dz_ResetDevice(mux);

			// Check Master Scan Enable
			if (data & CSR_MSE);
				// Enter master scane timer.

			// Check Receive Interrupt Enable
			if ((data & CSR_RIE) == 0)
				io->CancelInterrupt(io, RX);
			else if (((CSR & CSR_RIE) == 0) && 
			         ((CSR & CSR_SAE) ?
			          (CSR & CSR_SA) : (CSR & CSR_RDONE)))
				io->SendInterrupt(io, RX);

			// Check Transmit Interrupt Enable
			if ((data & CSR_TIE) == 0)
				io->CancelInterrupt(io, TX);
			else if ((CSR & (CSR_TIE|CSR_TRDY)) == CSR_TRDY)
				io->SendInterrupt(io, TX);

			// Finally, update CSR register.
			CSR = (data & CSR_RW) | (CSR & ~CSR_RW);
			break;

		case DZ_LPR:   // Line Parameter Register
			LPR = data;

			// Enable/Disable TTY line to receive.
			tty = &mux->Lines[data & LPR_LINE];
			if (data & LPR_RX_ON)
				tty->Flags |= TTY_RXON;
			else
				tty->Flags &= ~TTY_RXON;
			break;

		case DZ_TCR:   // Transmit Control Register
			if (acc == ACC_BYTE) // if access is byte, merge with DZTCR.
				data = (pAddr & 1) ? ((data << 8)   | (TCR & 0377)) :
				                     ((data & 0377) | (TCR & ~0377));
			// Modem Control Updates
			TDR = data;
			break;

		case DZ_TDR:   // Transmit Data Register
			TDR = data;
			if (CSR & CSR_MSE) {
			}
			break;
	}

	return UQ_OK;
}

int dz_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	uint32    reg =  (pAddr >> 1) & 03;
	DZ_DEVICE *dz =  (DZ_DEVICE *)dptr;
	DZ_MUX    *mux = &dz->Muxes[(pAddr >> 3) & DZ_MUXMASK];

	switch (reg) {
		case DZ_CSR:   // Control and Status Register
			*data = CSR;
			break;

		case DZ_RBUF:  // Receive Buffer Register
			*data = RBUF;
			break;

		case DZ_TCR:   // Transmit Control Register
			*data = TCR;
			break;

		case DZ_MSR:   // Modem Status Register
			*data = MSR;
			break;
	}

	return UQ_OK;
}

// Bus Initialization
void dz_ResetIO(void *dptr)
{
	dz_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// ********************************************************************

void *dz_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	DZ_DEVICE *dz = NULL;
	MAP_IO    *io;
	int       idx;

	if (dz = (DZ_DEVICE *)calloc(1, sizeof(DZ_DEVICE))) {
		// First, set up its description and
		// link it to its parent device.
		dz->Unit.devName    = newMap->devName;
		dz->Unit.keyName    = newMap->keyName;
		dz->Unit.emuName    = newMap->emuName;
		dz->Unit.emuVersion = newMap->emuVersion;
		dz->Device          = newMap->devParent->Device;
		dz->Callback        = newMap->devParent->Callback;
		dz->System          = newMap->devParent->sysDevice;

		// Recognize which model - DZ11 or DZV11.
		if (!strcmp(dz->Unit.keyName, DZ_KEY)) {
			dz->Flags  = CFLG_DZ11;
			dz->nPorts = DZ_PORTS;  // 8 ports
		} else if (!strcmp(dz->Unit.keyName, DZV_KEY)) {
			dz->Flags  = CFLG_DZV11;
			dz->nPorts = DZV_PORTS; // 4 ports
		} else {
			printf("%s: *** Bug Check: Unknown device name - %s\n",
				dz->Unit.devName, dz->Unit.keyName);
			free(dz);
			return NULL;
		}
		dz->csrAddr = DZ_CSRADR;
		dz->nMuxes  = 1;

		// Initialize Server, TTY lines, etc.
		if (dz_InitServer(dz) == NULL) {
			free(dz);
			return NULL;
		}
		dz_InitLines(dz);
		for (idx = 0; idx < dz->nMuxes; idx++) {
			DZ_MUX *mux = &dz->Muxes[idx];

			// Reset MUX device
			dz_ResetDevice(mux);

			// Set up an I/O space.
			io                = &mux->ioMap;
			io->devName       = dz->Unit.devName;
			io->keyName       = dz->Unit.keyName;
			io->emuName       = dz->Unit.emuName;
			io->emuVersion    = dz->Unit.emuVersion;
			io->Device        = mux;
			io->csrAddr       = dz->csrAddr;
			io->nRegs         = DZ_NREGS;
			io->nVectors      = DZ_NVECS;
			io->intIPL        = DZ_IPL;
			io->intVector[RX] = DZ_RXVEC;
			io->intVector[TX] = DZ_TXVEC;
			io->ReadIO        = dz_ReadIO;
			io->WriteIO       = dz_WriteIO;
			io->ResetIO       = dz_ResetIO;
	
			// Assign that registers to QBA's I/O space.
			dz->Callback->SetMap(dz->Device, io);
		}

		// Finally, link it to its mapping device and return.
		newMap->Device = dz;
	}

	return dz;
}

//int dz_Reset(MAP_DEVICE *map)
int dz_Reset(void *dptr)
{
//	DZ_DEVICE *dz = (DZ_DEVICE *)map->Device;
	DZ_DEVICE *dz = (DZ_DEVICE *)dptr;
	int       idx;

	for (idx = 0; idx < dz->nMuxes; idx++)
		dz_ResetDevice(&dz->Muxes[idx]);
}

DEVICE dz_Device =
{
	DZ_KEY,           // Key Name
	DZ_NAME,          // Emulator Name
	DZ_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	dz_Create,        // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	dz_Reset,         // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};

DEVICE dzv_Device =
{
	DZV_KEY,          // Key Name
	DZ_NAME,          // Emulator Name
	DZ_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	dz_Create,        // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	dz_Reset,         // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
