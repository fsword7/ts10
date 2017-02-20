// dhu.c - DHU11/DHV11/DHQ11 Terminal Controller
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
#include "dec/dhu.h"

// *************************************************************

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

void dhu_Accept(SOCKET *);
void dhu_Eof(SOCKET *, int, int);
void dhu_Input(SOCKET *, char *, int);
void dhu_TransmitDMA(void *);
void dhu_Transmit(void *);
void dhu_Queue(void *);

// Inline Macro definitions here
inline void dhu_RingRX(DH_DEVICE *dh)
{
	MAP_IO *io = &dh->ioMap;

	if (dh->csr & CSR_RXIE) {
		io->SendInterrupt(io, RXINT);
#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Ring for receive interrupt.\n", dh->Unit.devName);
#endif /* DEBUG */
	}
}

inline void dhu_RingTX(DH_DEVICE *dh)
{
	MAP_IO *io = &dh->ioMap;

	if (dh->csr & CSR_TXIE) {
		io->SendInterrupt(io, TXINT);
#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Ring for transmit interrupt.\n", dh->Unit.devName);
#endif /* DEBUG */
	}
}

inline void dhu_UpdateStatus(DH_TTY *tty, uint16 set, uint16 clr)
{
	DH_DEVICE *dh = tty->Device;
	uint16    rx;
	
	// Update line status register.
	STAT = (STAT | set) & ~clr;

	// If a link type is set, put that in receiving FIFO.
	if (LNC & LNC_LINK) {
		rx = (RBUF_VALID|RBUF_STATUS) | ((tty->idPort) << 8) |
			((STAT >> 8) & ~1);
		dh->rbuf[dh->idxIn++] = rx;
		if (dh->idxIn >= LEN_RBUF)
			dh->idxIn = 0;
	}

	// Finally, ring host for receive interrupt.
	dhu_RingRX(dh);
}

// Initialize DHx11 Server
SOCKET *dhu_InitServer(DH_DEVICE *dh)
{
	SOCKET *newServer = NULL;

	// Set up the listing socket for CTY device.
	if (newServer = sock_Open(dh->Unit.keyName, TTY_PORT, NET_SERVER)) {
		newServer->maxConns = dh->nPorts;
		newServer->nConns   = 0;
		newServer->Accept   = dhu_Accept;
		newServer->Eof      = NULL;
		newServer->Process  = NULL;
		newServer->Device   = dh;
		dh->Server          = newServer;

		// Now accept incoming connections;
		sock_Listen(newServer, 5);

		// Tell operator that.
		printf("Activating %s: (%s) on TCP port %d.\n",
			dh->Unit.devName, dh->Unit.keyName, TTY_PORT);
	} else {
		printf("Can't activate %s: (%s) - Aborted.\n",
			dh->Unit.devName, dh->Unit.keyName);
	}

	return newServer;
}

// Initialize TTY lines
void dhu_InitLines(DH_DEVICE *dh)
{
	DH_TTY    *tty;
	CLK_QUEUE *timer;
	int       idx;

	// Initialize Ports
	for (idx = 0; idx < dh->nPorts; idx++) {
		tty = &dh->Ports[idx];
		tty->Device = dh;
		tty->idPort = idx;

		timer           = &tty->dmaTimer;
		timer->Next     = NULL;
		timer->Name     = "DHV11 Transmit DMA Timer";
		timer->Flags    = 0;
		timer->outTimer = TTY_TBDELAY;
		timer->nxtTimer = TTY_TBDELAY;
		timer->Device   = tty;
		timer->Execute  = dhu_TransmitDMA;

		timer           = &tty->txTimer;
		timer->Next     = NULL;
		timer->Name     = "DHV11 Transmit Timer";
		timer->Flags    = 0;
		timer->outTimer = TTY_TXDELAY;
		timer->nxtTimer = TTY_TXDELAY;
		timer->Device   = tty;
		timer->Execute  = dhu_Transmit;

		timer           = &tty->qTimer;
		timer->Next     = NULL;
		timer->Name     = "DHV11 Queue Timer";
		timer->Flags    = 0;
		timer->outTimer = TTY_QDELAY;
		timer->nxtTimer = TTY_QDELAY;
		timer->Device   = tty;
		timer->Execute  = dhu_Queue;
	}

	// Mark ports as non-existant.
	for (; idx < DH_NPORTS; idx++)
		dh->Ports[idx].idPort = -1;
}

void dhu_Cleanup(DH_DEVICE *dh)
{
}

void dhu_Accept(SOCKET *srvSocket)
{
	DH_DEVICE *dh  = (DH_DEVICE *)srvSocket->Device;
	DH_TTY    *tty = NULL;
	SOCKET    *newSocket;
	uint32    idx;

	if (newSocket = sock_Accept(srvSocket)) {
		// Find a free tty line to assign.
		for (idx = 0; idx < dh->nPorts; idx++) {
			if (dh->Ports[idx].Socket == NULL) {
				tty = &dh->Ports[idx];
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
 
		// Set up the CTY socket connection.
		newSocket->Accept  = NULL;
		newSocket->Eof     = dhu_Eof;
		newSocket->Process = dhu_Input;
		newSocket->Device  = tty;
		tty->Socket        = newSocket;

		// Reset all buffer for this new connection.
		tty->idxInQueue    = 0;
		tty->idxOutQueue   = 0;
		tty->idxOutBuffer  = 0;

		// Send initialization codes and welcome messages
		sock_Print(newSocket, telnetInit, n_telnetInit);
		sock_Print(newSocket, "Welcome to TS10 Emulator\r\n\r\n", 0);
		sock_Print(newSocket, "Please press RETURN to enter.\r\n", 0);

//		dbg_SetMode(DBG_TRACE|DBG_DATA|DBG_PAGEFAULT|DBG_INTERRUPT);

		// Initially, Set DSR, DCD and CTS bits and ring host.
		dhu_UpdateStatus(tty, STAT_DSR|STAT_DCD|STAT_CTS, 0);
	}
}

void dhu_Eof(SOCKET *Socket, int rc, int nError)
{
	DH_TTY *tty = (DH_TTY *)Socket->Device;

	// Drop CTS, DCD and DSR bit to sign off.
	dhu_UpdateStatus(tty, 0, STAT_DCD|STAT_DSR|STAT_CTS);

	sock_Close(Socket);
	tty->Socket = NULL;
}

void dhu_Input(SOCKET *Socket, char *keyBuffer, int len)
{
	DH_TTY    *tty   = (DH_TTY *)Socket->Device;
	DH_DEVICE *dh    = tty->Device;
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
				dhu_Eof(Socket, 0, 0);
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
		rx = RBUF_VALID | ((tty->idPort) << 8) | ch;
		dh->rbuf[dh->idxIn++] = rx;
		if (dh->idxIn >= LEN_RBUF)
			dh->idxIn = 0;
		dh->csr |= CSR_RXAVAIL;
		dhu_RingRX(dh);
	}

	// Activate queue timer if any characters still are
	// remaining in their input queue.
	if (tty->idxOutQueue != tty->idxInQueue)
		ts10_SetTimer(&tty->qTimer);
}

// Transmit DMA Process
void dhu_TransmitDMA(void *dev)
{
	DH_TTY    *tty     = (DH_TTY *)dev;
	DH_DEVICE *dh      = tty->Device;
	uint32    cntBytes = TBCNT;
	uint32    errFlag  = 0;

	if (cntBytes > 0) {
		UQ_CALL *call    = dh->Callback;
		uint32  hstAddr  = ((TBAD2 & TBAD2_ADDR) << 16) | TBAD1;
		uint32  len      = (cntBytes > 256) ? 256 : cntBytes;
		uchar   bufData[256];
		uint32  txBytes;

		// Attempts transfer data from host.
		if (txBytes = call->ReadBlock(dh->System, hstAddr, bufData, len, 0)) {
			errFlag = CSR_TXERR;
			len    -= txBytes;
		}

		// Send good data to outside.
		if (len) {
			sock_Print(tty->Socket, (char *)bufData, len);
			hstAddr  += len;
			cntBytes -= len;
		}

		// Update registers
		TBAD1 = hstAddr;
		TBAD2 = ((hstAddr >> 16) & TBAD2_ADDR) | (TBAD2 & ~TBAD2_ADDR);
		TBCNT = errFlag ? 0 : cntBytes;

		if ((errFlag == 0) && (cntBytes > 0)) {
			ts10_SetTimer(&tty->dmaTimer);
			return;
		}
	}

	// Transmit DMA Completed
	TBAD2 &= ~TBAD2_START;
	if ((CSR & CSR_TXACT) == 0)
		CSR |= (tty->idPort << 8) | errFlag | CSR_TXACT;
	else {
		TBUF[dh->idxTxIn++] = (tty->idPort << 8) | errFlag | CSR_TXACT;
		if (dh->idxTxIn >= LEN_TBUF)
			dh->idxTxIn = 0;
	}
	dhu_RingTX(dh);
}

void dhu_AbortDMA(DH_TTY *tty)
{
	DH_DEVICE *dh = tty->Device;

	// Abort all DMA transfers for this TTY line.
	if (tty->dmaTimer.Flags & CLK_PENDING)
		ts10_CancelTimer(&tty->dmaTimer);

	// Abort DMA Done.
	if ((CSR & CSR_TXACT) == 0)
		CSR |= (tty->idPort << 8) | CSR_TXACT;
	else {
		TBUF[dh->idxTxIn++] = (tty->idPort << 8) | CSR_TXACT;
		if (dh->idxTxIn >= LEN_TBUF)
			dh->idxTxIn = 0;
	}
	dhu_RingTX(dh);
}

void dhu_Transmit(void *dev)
{
	DH_TTY    *tty = (DH_TTY *)dev;
	DH_DEVICE *dh  = tty->Device;
	char      ch;

	if (tty->Socket && (TXCH & TXCH_VALID)) {
		ch = TXCH;
		sock_Print(tty->Socket, &ch, 1);

		// Done, ring host for next transmit.
		TXCH &= ~TXCH_VALID;
		if ((CSR & CSR_TXACT) == 0)
			CSR |= (tty->idPort << 8) | CSR_TXACT;
		else {
			TBUF[dh->idxTxIn++] = (tty->idPort << 8) | CSR_TXACT;
			if (dh->idxTxIn >= LEN_TBUF)
				dh->idxTxIn = 0;
		}
		dhu_RingTX(dh);

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
	}
}

// Check Queue for this TTY line.
void dhu_Queue(void *dev)
{
	DH_TTY      *tty = (DH_TTY *)dev;
	DH_DEVICE   *dh  = tty->Device;
	uint16      rx;
	uchar       ch;

	if (tty->idxOutQueue != tty->idxInQueue) {
		// Dequeue a oldest character from internal receive buffer.
		ch = tty->inBuffer[tty->idxOutQueue++];
		if (tty->idxOutQueue == TTY_BUFFER)
			tty->idxOutQueue = 0;

		// Finally, put data into FIFO receive buffer and
		// ring host for receive interrupt.
		rx = RBUF_VALID | ((tty->idPort) << 8) | ch;
		dh->rbuf[dh->idxIn++] = rx;
		if (dh->idxIn >= LEN_RBUF)
			dh->idxIn = 0;
		dh->csr |= CSR_RXAVAIL;
		dhu_RingRX(dh);

		// Reactivate queue timer if characters still are
		// remaining in their input queue.
		if (tty->idxOutQueue != tty->idxInQueue)
			ts10_SetTimer(&tty->qTimer);
	}
}

// *************************************************************

#ifdef DEBUG

static cchar *regNameR[] =
{
	"CSR",      // Control and Status Register
	"RBUF",     // Receive Buffer
	"LPR",      // Line Parameter Register
	"STAT",     // Line Status Register
	"LNCTRL",   // Line Control Register
	"TBUFFAD1", // Transmit Buffer Address 1
	"TBUFFAD2", // Transmit Buffer Address 2
	"TBUFFCT"   // Transmit Buffer Count
};

static cchar *regNameW[] =
{
	"CSR",      // Control and Status Register
	"TXCHAR",   // Transmit Character
	"LPR",      // Line Parameter Register
	"STAT",     // Line Status Register (Read-Only)
	"LNCTRL",   // Line Control Register
	"TBUFFAD1", // Transmit Buffer Address 1
	"TBUFFAD2", // Teansmit Buffer Address 2
	"TBUFFCT"   // Transmit Buffer Count
};

#endif /* DEBUG */

void dhu_MasterReset(DH_DEVICE *dh)
{
	DH_TTY *tty;
	uint32 idx;
	uint16 rx;

	// Initialize Registers
	dh->csr = (dh->csr | CSR_RXAVAIL) & ~CSR_TXACT;
	for (idx = 0; idx < dh->nPorts; idx++) {
		tty   = &dh->Ports[idx];

		// Disconnect sockets if online.
		if (tty->Socket) {
			// Close existing socket on this TTY line.
			sock_Close(tty->Socket);
			tty->Socket = NULL;
		}

		// Initialize line registers.
		LPR   = PUT_TXSP(BAUD_9600) | PUT_RXSP(BAUD_9600)
			| PUT_CHLEN(BIT_8);
		STAT  = 0;
		LNC   = 0;
		TBAD1 = 0;
		TBAD2 = TBAD2_TXEN;
		TBCNT = 0;
	}

	// Initialize and put diagnostic codes
	// into FIFO receive buffer.
	rx = (RBUF_VALID|RBUF_STATUS);
	dh->idxIn  = 0;
	dh->idxOut = 0;
	for (idx = 0; idx < 6; idx++)
		dh->rbuf[dh->idxIn++] = rx | 0203; // Self-test skipped code.
	dh->rbuf[dh->idxIn++]    = rx | 0001; // Two ROM Version Codes.
	dh->rbuf[dh->idxIn++]    = rx | 0003;

	dh->idxTxIn  = 0;
	dh->idxTxOut = 0;

	// When Master Reset is done, clear it.
	dh->csr &= ~CSR_RESET;
}

// Bus Initialization
void dhu_ResetDevice(DH_DEVICE *dh)
{
	MAP_IO *io = &dh->ioMap;

	// Reset Control/Status Register
	dh->csr = (dh->csr | CSR_RESET) & ~(CSR_RXIE|CSR_TXIE);
	if (io->CancelInterrupt) {
		io->CancelInterrupt(io, RXINT);
		io->CancelInterrupt(io, TXINT);
	}
	dhu_MasterReset(dh);
}

int dhu_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	DH_DEVICE *dh = (DH_DEVICE *)dptr;
	DH_TTY    *tty;
	uint32    reg = (pAddr - (dh->csrAddr & 0x1FFF)) >> 1;

	if (reg == DH_CSR) {
		// Control and Status Register
		*data = dh->csr;
		// Clear TX.ACTION bit after each read.
		dh->csr &= ~(CSR_TXACT|CSR_TXLINE);
		if (dh->idxTxIn != dh->idxTxOut) {
			CSR |= TBUF[dh->idxTxOut++];
			if (dh->idxTxOut >= LEN_TBUF)
				dh->idxTxOut = 0;
			dhu_RingTX(dh);
		}
	} else if (reg == DH_RBUF) {
		// Receive Buffer Register
		if (dh->idxIn != dh->idxOut) {

			// Dequeue oldest character in RBUF queue.
			*data = dh->rbuf[dh->idxOut++];
			if (dh->idxOut >= LEN_RBUF)
				dh->idxOut = 0;

			// When FIFO is empty, clear RX.DATA.AVAIL bit.
			if (dh->idxOut == dh->idxIn)
				dh->csr &= ~CSR_RXAVAIL;

		} else
			*data = 0;
	} else {
		tty   = &dh->Ports[dh->csr & CSR_LINE];
		*data = MREG(reg);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%o) => %06o (%04X) (Size: %d bytes)\n",
			dh->Unit.devName, regNameR[reg], pAddr, *data, *data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

int dhu_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	DH_DEVICE *dh = (DH_DEVICE *)dptr;
	DH_TTY    *tty;
	uint32    reg = (pAddr - (dh->csrAddr & 0x1FFF)) >> 1;

	if (reg == DH_CSR) {
		MAP_IO *io = &dh->ioMap;

		if (acc == ACC_BYTE) // If access is byte, merge data with CSR.
			data = (pAddr & 1) ? ((data << 8)   | (dh->csr & 0377)) :
			                     ((data & 0377) | (dh->csr & ~0377));

		// If set, execute master reset.
		if (data & CSR_RESET) {
			data &= ~CSR_RESET; // Temp. patch
			dhu_MasterReset(dh);
		}

		// Check Receive Interrupt Enable
		if ((data & CSR_RXIE) == 0)
			io->CancelInterrupt(io, RXINT);
		else if (dh->csr & CSR_RXAVAIL)
			io->SendInterrupt(io, RXINT);

		// Check Transmit Interrupt Enable
		if ((data & CSR_TXIE) == 0)
			io->CancelInterrupt(io, TXINT);
		else if (dh->csr & CSR_TXACT)
			io->SendInterrupt(io, TXINT);

		dh->csr = (data & CSR_RW) | (dh->csr & ~CSR_RW);
	} else {
		tty = &dh->Ports[dh->csr & CSR_LINE];
		switch (reg) {
			case DH_TXCHAR:   // Transmit Character
				if (acc == ACC_BYTE) // If access is byte, merge with TXCH.
					data = (pAddr & 1) ? ((data << 8)   | (TXCH & 0377)) :
			   		                  ((data & 0377) | (TXCH & ~0377));
				TXCH = data & TXCH_RW;
				if (TXCH & TXCH_VALID)
					ts10_SetTimer(&tty->txTimer);
				break;

			case DH_LPR:      // Line Parameter Register
				if (acc == ACC_BYTE) // if access is byte, merge with LPR.
					data = (pAddr & 1) ? ((data << 8)   | (LPR & 0377)) :
						                  ((data & 0377) | (LPR & ~0377));
				LPR = data & LPR_RW;
				break;

			case DH_STAT:     // Line Status Register (Read-Only)
				break;

			case DH_LNCTRL:   // Line Control Register
				if (acc == ACC_BYTE) // if access is byte, merge with LNC.
					data = (pAddr & 1) ? ((data << 8)   | (LNC & 0377)) :
						                  ((data & 0377) | (LNC & ~0377));
				LNC = data & LNC_RW;
				if (data & LNC_ABORT)
					dhu_AbortDMA(tty);
				break;

			case DH_TBUFFAD1: // Transmit Buffer Address #1 Register
				if (acc == ACC_BYTE) // if access is byte, merge with TBAD1.
					data = (pAddr & 1) ? ((data << 8)   | (TBAD1 & 0377)) :
						                  ((data & 0377) | (TBAD1 & ~0377));
				TBAD1 = data;
				break;

			case DH_TBUFFAD2: // Transmit Buffer Address #2 Register
				if (acc == ACC_BYTE) // if access is byte, merge with TBAD2.
					data = (pAddr & 1) ? ((data << 8)   | (TBAD2 & 0377)) :
						                  ((data & 0377) | (TBAD2 & ~0377));
				TBAD2 = data & TBAD2_RW;

				// Start transmit DMA if request.
				if (TBAD2 & TBAD2_START)
					ts10_SetTimer(&tty->dmaTimer);
				break;

			case DH_TBUFFCT:  // Transmit Buffer Count Register
				if (acc == ACC_BYTE) // if access is byte, merge with TBCNT.
					data = (pAddr & 1) ? ((data << 8)   | (TBCNT & 0377)) :
						                  ((data & 0377) | (TBCNT & ~0377));
				TBCNT = data;
				break;

#ifdef DEBUG
			default:
				dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
					dh->Unit.devName, reg, pAddr & 0x1FFF, __LINE__, __FILE__);
				return UQ_NXM;
#endif /* DEBUG */
		}
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) %s (%o) <= %06o (%04X) (Size: %d bytes)\n",
			dh->Unit.devName, regNameW[reg], pAddr, data, data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

// Bus Initialization
void dhu_ResetIO(void *dptr)
{
	dhu_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// **************************************************************

void *dhu_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	DH_DEVICE *dh = NULL;
	MAP_IO    *io;

	if (dh = (DH_DEVICE *)calloc(1, sizeof(DH_DEVICE))) {
		// First, set up its description and
		// link it to its parent device.
		dh->Unit.devName    = newMap->devName;
		dh->Unit.keyName    = newMap->keyName;
		dh->Unit.emuName    = newMap->emuName;
		dh->Unit.emuVersion = newMap->emuVersion;
		dh->Device          = newMap->devParent->Device;
		dh->Callback        = newMap->devParent->Callback;
		dh->System          = newMap->devParent->sysDevice;

		// Recognize which model - DHU11, DHV11, or DHQ11.
		if (!strcmp(dh->Unit.keyName, DHU_KEY)) {
			dh->Flags  = CFLG_DHU;
			dh->nPorts = DHU_PORTS; // 16 ports
		} else if (!strcmp(dh->Unit.keyName, DHV_KEY)) {
			dh->Flags  = CFLG_DHV;
			dh->nPorts = DHV_PORTS; // 8 ports
		} else if (!strcmp(dh->Unit.keyName, DHQ_KEY)) {
			dh->Flags  = CFLG_DHQ;
			dh->nPorts = DHQ_PORTS; // 8 ports
		} else {
			printf("%s: *** Bug Check: Unknown device name - %s\n",
				dh->Unit.devName, dh->Unit.keyName);
			free(dh);
			return NULL;
		}
		dh->csrAddr = DH_CSRADDR;

		// Initialize Server, TTY lines, etc.
		if (dhu_InitServer(dh) == NULL) {
			free(dh);
			return NULL;
		}
		dhu_InitLines(dh);
		dhu_ResetDevice(dh);

		// Set up an I/O space.
		io               = &dh->ioMap;
		io->devName      = dh->Unit.devName;
		io->keyName      = dh->Unit.keyName;
		io->emuName      = dh->Unit.emuName;
		io->emuVersion   = dh->Unit.emuVersion;
		io->Device       = dh;
		io->csrAddr      = dh->csrAddr;
		io->nRegs        = DH_NREGS;
		io->nVectors     = DH_NVECS;
		io->intIPL       = DH_IPL;
		io->intVector[0] = DH_RXVEC;
		io->intVector[1] = DH_TXVEC;
		io->ReadIO       = dhu_ReadIO;
		io->WriteIO      = dhu_WriteIO;
		io->ResetIO      = dhu_ResetIO;

		// Assign that registers to QBA's I/O space.
		dh->Callback->SetMap(dh->Device, io);

		// Finally, link it to its mapping device and return.
		newMap->Device = dh;
	}

	return dh;
}

//int dhu_Reset(MAP_DEVICE *map)
int dhu_Reset(void *dptr)
{
//	DH_DEVICE *dh = (DH_DEVICE *)map->Device;
	DH_DEVICE *dh = (DH_DEVICE *)dptr;

	dhu_ResetDevice(dh);
}

DEVLIST dhu_List[] =
{
	{ 1, DHU_KEY, DH_NAME, DH_VERSION }, // DHU11 (Unibus)
	{ 2, DHV_KEY, DH_NAME, DH_VERSION }, // DHV11 (QBus)
	{ 3, DHQ_KEY, DH_NAME, DH_VERSION }, // DHQ11 (QBus)
	{ 0, NULL,    NULL,    NULL       }, // Terminator
};

DEVICE dhu_Device =
{
	DHU_KEY,          // Key Name
	DH_NAME,          // Emulator Name
	DH_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	dhu_Create,       // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	dhu_Reset,        // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};

DEVICE dhv_Device =
{
	DHV_KEY,          // Key Name
	DH_NAME,          // Emulator Name
	DH_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	dhu_Create,       // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	dhu_Reset,        // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};

DEVICE dhq_Device =
{
	DHQ_KEY,          // Key Name
	DH_NAME,          // Emulator Name
	DH_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	dhu_Create,       // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	dhu_Reset,        // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
