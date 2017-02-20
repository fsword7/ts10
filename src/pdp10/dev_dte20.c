// dte20.c - DTE20 Ten-Eleven Interface Support Routines
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

// Device Name: DTE (4 Channels)
// Device Code: 200,204,210,214

#include "pdp10/defs.h"
#include "emu/proto.h"
#include "emu/socket.h"
#include "pdp10/kl10.h"
#include "pdp10/dte20.h"
#include "pdp10/proto.h"

#ifdef HAVE_SIGACTION
#include <sys/file.h>
#include <unistd.h>
#include <stropts.h>
#include <signal.h>
#endif /* HAVE_SIGACTION */

#include <time.h>
#include <sys/time.h>

#if defined(linux)
#include <endian.h>
#endif /* linux */

//****************************************************************

int30 eptKeepAlive = 0;

// Get EPT base address from KL10 Processor or so.
extern int KL10_eptAddr;

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

static int (*fncExecute[])(DTE_DEVICE *);

#ifdef DEBUG
static char *fncNames[] =
{
	"*Unknown Function",             // 000
	"TO-11 Initial Message",         // 001
	"TO-10 Initial Message",         // 002
	"String Data",                   // 003
	"Line-Char, Line-Char",          // 004
	"Request Device Status",         // 005
	"*Unknown Function",             // 006
	"Here is Device Status",         // 007
	"*Unknown Function",             // 010
	"Request Date/Time",             // 011
	"Here is Date/Time",             // 012
	"Flush Output",                  // 013
	"Send All TTYs",                 // 014
	"Dataset Connected",             // 015
	"Hang Up Dataset",               // 016
	"Acknowledge",                   // 017
	"XOFF TTY Line",                 // 020
	"XON TTY Line",                  // 021
	"Here are Line Speeds for TTYs", // 022
	"Here are Allocations",          // 023
	"TO10 Reboot Information",       // 024
	"Acknowledge All",               // 025
	"Turn Device (On/Off)",          // 026
	"Enable/Disable Remotes",        // 027
	"Load Line Printer RAM",         // 030
	"Load Line Printer VFU",         // 031
	"DAS60 Data",                    // 032
	"KLINIK Parameter Status",       // 033
	"Enable/Disable Auto-XOFF",      // 034
	"TO11 Break Through Char Data",  // 035
	"TO11 Turn On Debug Mode",       // 036
	"TO11 Turn Off Debug Mode",      // 037
};
#endif /* DEBUG */

void dte20_ctyAccept(SOCKET *);
void dte20_ctyEof(SOCKET *, int, int);
void dte20_ctyInput(SOCKET *, char *, int);

void dte20_InitSockets(DTE_DEVICE *dte20)
{
	SOCKET *newServer;

	// Set up the listing socket for CTY device.
	newServer = sock_Open("(dte20/cty)", CTY_PORT, NET_SERVER);
	if (newServer != NULL) {
		newServer->maxConns = 1;
		newServer->nConns   = 0;
		newServer->Accept   = dte20_ctyAccept;
		newServer->Eof      = NULL;
		newServer->Process  = NULL;
		newServer->Device   = (void *)dte20;
		dte20->ctyServer    = newServer;

		// Now accept incoming connections;
		sock_Listen(newServer, 5);

		// Tell operator that.
		printf("DTE20: Activating console on TCP port %d.\n", CTY_PORT);
		printf("DTE20: Enter 'telnet localhost %d' to log in\n", CTY_PORT);
	}
}

void dte20_ctyAccept(SOCKET *srvSocket)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)srvSocket->Device;
	SOCKET *newSocket;

	if (newSocket = sock_Accept(srvSocket)) {
		// First, check if CTY connection already was taken.
		// If so, tell operator that.
		if (dte20->ctySocket != NULL) {

			sock_Print(newSocket, "Console (CTY) connection already was taken.\r\n", 0);
			sock_Print(newSocket, "Check other terminal which has that connection.\r\n", 0);
			sock_Print(newSocket, "\r\nTerminated.\r\n", 0);
			sock_Close(newSocket);

			return;
		}
 
		// Set up the CTY socket connection.
		newSocket->Accept  = NULL;
		newSocket->Eof     = dte20_ctyEof;
		newSocket->Process = dte20_ctyInput;
		newSocket->Device  = (void *)dte20;

		// Reset all buffer for this new connection.
		dte20->idxInQueue   = 0;
		dte20->idxOutQueue  = 0;
		dte20->idxOutBuffer = 0;
		dte20->ctySocket    = newSocket;

		// Send initialization codes and welcome messages
		sock_Print(newSocket, telnetInit, n_telnetInit);
		sock_Print(newSocket, "Welcome to KL10 Emulator\r\n\r\n", 0);
	}
}

void dte20_ctyEof(SOCKET *Socket, int rc, int nError)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)Socket->Device;

	sock_Close(dte20->ctySocket);
	dte20->ctySocket = NULL;
}

int dte20_SendOneChar10(register DTE_DEVICE *);

void dte20_ctyInput(SOCKET *Socket, char *keyBuffer, int len)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)Socket->Device;
	int     okSend = FALSE;
	int     idx;

	// Process telnet codes and filter them out of data stream.
	if (len > 1) {
		if ((len = sock_ProcessTelnet((uchar *)keyBuffer, len)) == 0)
			return;
	}

	for (idx = 0; idx < len; idx++) {
		uchar ch = keyBuffer[idx];

		// Press ^\ twice to disconnect.
		if (ch == CTY_ESCAPE) {
			if (dte20->lastSeen == CTY_ESCAPE) {
				dte20_ctyEof(dte20->ctySocket, 0, 0);
				return;
			}
			dte20->lastSeen = ch;
			continue;
		}

		// Convert CR NL to CR line.
		if ((ch == 012) && (dte20->lastSeen == 015))
			continue;
		dte20->lastSeen = ch;

		okSend = TRUE;
		dte20->inBuffer[dte20->idxInQueue] = ch;
		if (++dte20->idxInQueue == 4096)
			dte20->idxInQueue = 0;
		if (dte20->idxInQueue == dte20->idxOutQueue) {
#ifdef DEBUG
			dbg_Printf("CTY: Error - Overrun!!\n");
#endif /* DEBUG */
			break;
		}
	}

	if (!okSend)
		return;

	if (dte20->Protocol == PRIMARY) {
		// Primary Protocol
		dte20_SendOneChar10(dte20);
	} else {
		// Secondary Protocol
		if (p10_pRead(KL10_eptAddr + DTEMTI, 0) == 0) {
			p10_pWrite(KL10_eptAddr + DTEF11,
				dte20->inBuffer[dte20->idxOutQueue], 0);
			p10_pWrite(KL10_eptAddr + DTEMTI, -1, 0);
			if (++dte20->idxOutQueue == 4096)
				dte20->idxOutQueue = 0;
		}
	}

	// When buffer still remaining, re-activate queue timer.
	if (dte20->idxOutQueue != dte20->idxInQueue)
		ts10_SetTimer(&dte20->cinTimer);
}

void dte20_CheckQueue(void *dptr)
{
	register DTE_DEVICE *dte = (DTE_DEVICE *)dptr;

	if (dte->idxOutQueue == dte->idxInQueue)
		return;

	if (dte->Protocol == PRIMARY) {
		// Primary Protocol
		dte20_SendOneChar10(dte);
	} else {
		// Secondary Protocol
		if (p10_pRead(KL10_eptAddr + DTEMTI, 0) == 0) {
			p10_pWrite(KL10_eptAddr + DTEF11,
				dte->inBuffer[dte->idxOutQueue], 0);
			p10_pWrite(KL10_eptAddr + DTEMTI, -1, 0);
			if (++dte->idxOutQueue == 4096)
				dte->idxOutQueue = 0;
		}
	}

	// When buffer still remaining, re-activate queue timer.
	if (dte->idxOutQueue != dte->idxInQueue)
		ts10_SetTimer(&dte->cinTimer);

//	return TS10_OK;
}

void dte20_ctyCharOut(DTE_DEVICE *dte20, uchar ch)
{
	if (dte20->ctySocket) {
		ch &= 0177;
		sock_Print(dte20->ctySocket, (char *)&ch, 1);

		// Log a character into a log file.
		if (ch == '\n') {
			dte20->outBuffer[dte20->idxOutBuffer++] = ch;
			dte20->outBuffer[dte20->idxOutBuffer++] = '\0';
#ifdef DEBUG
			if (dbg_Check(DBG_CONSOLE))
				dbg_Printf("CTY: %s", dte20->outBuffer);
#endif /* DEBUG */
			if (emu_logFile >= 0)
				write(emu_logFile, dte20->outBuffer, dte20->idxOutBuffer);
			dte20->idxOutBuffer = 0;
		} else {
			if (ch == '\b' || ch == 127) {
				if (dte20->idxOutBuffer > 0)
					dte20->idxOutBuffer--;
			} else if (ch != '\r' && ch != '\0') {
				if (dte20->idxOutBuffer < 4095)
					dte20->outBuffer[dte20->idxOutBuffer++] = ch;
			}
		}
	}
}

void dte20_ctyStrOut(DTE_DEVICE *dte, uchar *out, int len)
{
#ifdef DEBUG
	int idx;
#endif /* DEBUG */

	if (dte->ctySocket) {
		sock_Print(dte->ctySocket, (char *)out, len);

#ifdef DEBUG
		for (idx = 0; idx < len; idx++) {
			uchar ch = out[idx];

			// Log a character into a log file.
			if (ch == '\n') {
				dte->outBuffer[dte->idxOutBuffer++] = ch;
				dte->outBuffer[dte->idxOutBuffer++] = '\0';
				if (dbg_Check(DBG_CONSOLE))
					dbg_Printf("CTY: %s", dte->outBuffer);
				if (emu_logFile >= 0)
					write(emu_logFile, dte->outBuffer, dte->idxOutBuffer);
				dte->idxOutBuffer = 0;
			} else {
				if (ch == '\b' || ch == 127) {
					if (dte->idxOutBuffer > 0)
						dte->idxOutBuffer--;
				} else if (ch != '\r' && ch != '\0') {
					if (dte->idxOutBuffer < 4095)
						dte->outBuffer[dte->idxOutBuffer++] = ch;
				}
			}
		}
#endif /* DEBUG */
	}
}

// ****************************************************************

void dte20_ResetDevice(DTE_DEVICE *dte)
{
	int idx;

	dte->Protocol = SECONDARY;
	dte->srFlags  = 0;
	dte->mtdFlags = 0;

	// Get which communication base area from EPT address.
	dte->eptOffset  = DTE_GETCB(dte->idDevice);
	dte->eptDepAddr = 0;
	dte->eptDepSize = 0;
	dte->eptExaAddr = 0;
	dte->eptExaSize = 0;

	dte->to10st = (ST_VALID|ST_QPIU);
	dte->to11qc = 0;

	// Initialize message packets
	for (idx = 0; idx < DTE_NPKTS; idx++) {
		dte->pktData[idx].Next =
			(idx < DTE_NPKTS) ? &dte->pktData[idx+1] : NULL;
		dte->pktData[idx].idPacket = idx;
	}
	dte->pktFree  = &dte->pktData[0];
	dte->pktQueue = NULL;
	dte->pktTail  = NULL;
}

DTE_PACKET *dte20_GetFreePacket(DTE_DEVICE *dte)
{
	DTE_PACKET *newPacket = NULL;

	if (dte->pktFree) {
		newPacket       = dte->pktFree;
		dte->pktFree    = newPacket->Next;
		newPacket->Next = NULL;
	}
#ifdef DEBUG
	else if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: *** Error: No free packets.\n", dte->Unit.devName);
#endif /* DEBUG */

	return newPacket;
}

void dte20_Enqueue(DTE_DEVICE *dte, DTE_PACKET *pkt, int which)
{
	if (pkt) {
		if (which) {
			// Enqueue at head queue.
			pkt->Next = dte->pktQueue;
			dte->pktQueue = pkt;
			if (pkt->Next == NULL)
				dte->pktTail = pkt;
		} else if (dte->pktQueue) {
			// Enqueue at tail queue.
			dte->pktTail->Next = pkt;
			dte->pktTail       = pkt;
		} else {
			// Enqueue at empty queue.
			dte->pktQueue = pkt;
			dte->pktTail = pkt;
		}
	}
}

DTE_PACKET *dte20_Dequeue(DTE_DEVICE *dte)
{
	DTE_PACKET *pkt = NULL;

	if (dte->pktQueue) {
		pkt           = dte->pktQueue;
		dte->pktQueue = pkt->Next;
	}
	return pkt;
}

// Ring Doorbell to PDP-10.
inline void dte20_Ring10(DTE_DEVICE *dte)
{
	// If PI is enabled, do an interrupt.
	// Otherwise, do nothing and return.
	if ((dte->srFlags & DTE_PIOENB) == 0)
		return;

	// Check any interrupt bits, if so,
	// ring a doorbell to PDP-10.
	if (dte->srFlags & DTE_DONE) {
		int36 iop = IRQ_VEC | (dte->idChannel << IRQ_P_DEV) | DTEDII;

#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Ring Doorbell to PDP-10\n", dte->Unit.devName);
#endif /* DEBUG */

		KL10pi_RequestIO(dte->srFlags & DTE_PIA, iop);
	}
}

// **************************************************
// *********** Primary Protocol Process *************
// **************************************************

inline int36 dte20_Fetch(register DTE_DEVICE *dte, uint32 idx)
{
#ifdef DEBUG
	if (idx <= dte->eptExaSize) {
		uint32 addr = dte->eptExaAddr + idx;
		int36  data = dte->eptExamine[idx];

		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: Examine %o (%o,,%o) => %o,,%o\n",
				dte->Unit.devName, idx, LH18(addr), RH18(addr),
				LH18(data), RH18(data));

		return data;
	}
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Bad Examine %o (Prot = %o)\n",
			dte->Unit.devName, idx, dte->eptExaSize);
	return 0;
#else  /* DEBUG */
	return (idx <= dte->eptExaSize) ? dte->eptExamine[idx] : 0;
#endif /* DEBUG */
}

inline void dte20_Store(register DTE_DEVICE *dte, uint32 idx, int36 data)
{
#ifdef DEBUG
	if (idx <= dte->eptDepSize) {
		uint32 addr = dte->eptDepAddr + idx;

		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: Deposit %o (%o,,%o) <= %o,,%o\n",
				dte->Unit.devName, idx, LH18(addr), RH18(addr),
				LH18(data), RH18(data));

		dte->eptDeposit[idx] = SXT36(data);
		return;
	}
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Bad Deposit %o (Prot = %o)\n",
			dte->Unit.devName, idx, dte->eptDepSize);
#else  /* DEBUG */
	if (idx <= dte->eptDepSize)
		dte->eptDeposit[idx] = SXT36(data);
#endif /* DEBUG */
}

// Initialize EPT addresses for communication base area.
inline void dte20_InitCommBase(DTE_DEVICE *dte)
{
	uint32 eptAddr = KL10_eptAddr;
	uint32 cbAddr  = eptAddr + DTE_GETCB(dte->idDevice);

	// Get Comm Base Address in EPT block area.
	dte->eptCommBase = p10_pAccess(cbAddr);
	dte->eptExamine  = p10_pAccess(DTE_CBERW);
	dte->eptDeposit  = p10_pAccess(DTE_CBDRW);

	dte->eptExaAddr  = DTE_CBERW;
	dte->eptExaSize  = RH18(DTE_CBEPW);
	dte->eptDepAddr  = DTE_CBDRW;
	dte->eptDepSize  = RH18(DTE_CBDPW);

	eptKeepAlive = DTE_CBDRW + 5;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("%s: Comm Base: %o,,%06o (EPT: %o,,%06o)\n",
			dte->Unit.devName, LH18(cbAddr), RH18(cbAddr),
			LH18(eptAddr), RH18(eptAddr));
		dbg_Printf("%s: Byte Pointer: To11: %06o,,%06o To10: %06o,,%06o\n",
			dte->Unit.devName, LH18(DTE_CB11BP), RH18(DTE_CB11BP),
			LH18(DTE_CB10BP), RH18(DTE_CB10BP));
		dbg_Printf("%s: Interrupt: %06o,,%06o\n",
			dte->Unit.devName, LH18(DTE_CBINT), RH18(DTE_CBINT));
		dbg_Printf("%s: Examine: %o,,%06o  Size: %06o\n", dte->Unit.devName,
			LH18(DTE_CBERW), RH18(DTE_CBERW), dte->eptExaSize);
		dbg_Printf("%s: Deposit: %o,,%06o  Size: %06o\n", dte->Unit.devName,
			LH18(DTE_CBDRW), RH18(DTE_CBDRW), dte->eptDepSize);
	}
#endif /* DEBUG */
}

// Enter primary protocol and initialize communication region areas.
int dte20_EnterPrimary(DTE_DEVICE *dte)
{
	int36  wd;
	uint32 cpun, cbAddr;
	uint32 idx;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("%s: Enter Primary Protocol.\n", dte->Unit.devName);
		dbg_Printf("%s: Initializing Communication Region Area.\n",
			dte->Unit.devName);
	}
#endif /* DEBUG */

	// Initialize Communication Base from current EPT base address.
	dte20_InitCommBase(dte);

	// Attempt to read first word from examine data area.
	// if a word is not valid, go back to secondary protocol.
	if ((wd = dte20_Fetch(dte, 0)) == 0LL) {
#ifdef DEBUG
		dbg_Printf("%s: Error COMPTR = 0 - Aborted.\n",
			dte->Unit.devName);
#endif /* DEBUG */
		return DTE_ERROR;
	}

	dte->cmbProc = (wd >> CMP_P_CPUN) & CMP_CPUN;
	dte->cmbAddr = dte->cmbProc + 1;

	// Set up comm region offsets in examine/deposit reloc area.
	dte->depAddr  = dte->cmbAddr + (wd & CMP_ADDR);
	dte->dt10Addr = 16;
	dte->et10Addr = dte->depAddr + 16;
	dte->ec10Addr = dte->cmbAddr;
	dte->et11Addr = dte->ec10Addr + 16 + (dte->idDevice * 8);

#if 0
	dbg_Printf("%s: COMBUF %06o,,%06o => CPU #%d Base %06o\n",
		dte->Unit.devName, LH18(wd), RH18(wd), cpun, cbAddr);

	for (idx = 0; idx < dte->eptExaSize; idx++) {
		wd = dte20_Fetch(dte, idx);
		dbg_Printf("%s: Examine %d - %06o,,%06o\n",
			dte->Unit.devName, idx, LH18(wd), RH18(wd));
	}
#endif

	// Finally, switch mode to primary protocol.
	dte->Protocol = PRIMARY;
	dte->Flags    = 0;

	ts10_SetRealTimer(&dte->kacTimer);

	return DTE_OK;
}

// Tell PDP-10 that xfer is done.
void dte20_XferDone11(DTE_DEVICE *dte)
{
	// Clear TOIT bit and send status to PDP-10.
	dte->to10st &= ~ST_TOIT;
	dte20_Store(dte, dte->dt10Addr + 2, dte->to10st);

	// Set TO11DN (TO-11 Done) bit on status flags that
	// can be read by CONx instructions.
	dte->srFlags |= DTE_TO11DN;
}

DTE_PACKET *dte20_ReadHeader(register DTE_DEVICE *dte)
{
	int36      ebp = DTE_CB11BP;
	uint8      to11msg[QMH_SIZ], *msg;
	DTE_PACKET *pkt;
	uint32     ebpAddr;
	int32      pos, size, mask;
	int36      data;
	int        idx;

	// Extract address, position, and size
	// from byte pointer word.
	ebpAddr = RH18(ebp);
	pos     = BP_GETPOS(ebp);
	size    = BP_GETSIZE(ebp);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("%s: Read Message Header\n", dte->Unit.devName);
		dbg_Printf("%s: DTEEBP (TO-11 Byte Pointer) => %06o,,%06o\n",
			dte->Unit.devName, LH18(ebp), RH18(ebp));
		dbg_Printf("%s:   (Position: %d  Size: %d  Address: %o)\n",
			dte->Unit.devName, pos, size, ebpAddr);
	}
#endif /* DEBUG */

	// Transfer TO11 Message
	data = p10_pRead(ebpAddr, 0);
	mask = (1 << size) - 1;
	msg  = to11msg;
	for (idx = 0; idx < QMH_SIZ; idx++) {
		if ((pos -= size) < 0) {
			pos = 36 - size;
			data = p10_pRead(++ebpAddr, 0);
		}
		*msg++ = (uint8)(data >> pos) & mask;
	}

	// Update TO-11 Byte Pointer
	ebp = BP_PUTPOS(pos) | BP_PUTSIZE(size) | ebpAddr;
	DTE_CB11BP = SXT36(ebp);

	// Set up a TO11 message packet.
	pkt = &dte->pktMsg;
	memset(&pkt->Data[0], 0, 256);
	pkt->xfrCount = QMH_SIZ;

	pkt->cnt = (to11msg[0] << 8) | to11msg[1]; // Message Length
	pkt->fnc = (to11msg[2] << 8) | to11msg[3]; // Function Code
	pkt->dev = (to11msg[4] << 8) | to11msg[5]; // Device Code
	pkt->wd1 = (to11msg[8] << 8) | to11msg[9]; // First two bytes of data

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: TO11 Message: Cnt=%04X Fnc=%04X Dev=%04X Wd1=%04X\n",
			dte->Unit.devName, pkt->cnt, pkt->fnc, pkt->dev, pkt->wd1);
#endif /* DEBUG */

	return pkt;
}

void dte20_WriteHeader(register DTE_DEVICE *dte, register DTE_PACKET *pkt)
{
	int36      tbp = DTE_CB10BP;
	int36      msgData;
	uint8      to10msg[QMH_SIZ], *msg;
	uint32     tbpAddr;
	int32      pos, size, mask;
	int36      data;
	int        idx;

	// Extract address, position, and size
	// from byte pointer word.
	tbpAddr = RH18(tbp);
	pos     = BP_GETPOS(tbp);
	size    = BP_GETSIZE(tbp);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("%s: Write Message Header\n", dte->Unit.devName);
		dbg_Printf("%s: DTETBP (TO-10 Byte Pointer) => %06o,,%06o\n",
			dte->Unit.devName, LH18(tbp), RH18(tbp));
		dbg_Printf("%s:   (Position: %d  Size: %d  Address: %o)\n",
			dte->Unit.devName, pos, size, tbpAddr);
	}
#endif /* DEBUG */

	// Set up a TO10 message packet.
	to10msg[0] = pkt->cnt;
	to10msg[1] = pkt->cnt >> 8;
	to10msg[2] = pkt->fnc;
	to10msg[3] = pkt->fnc >> 8;
	to10msg[4] = pkt->dev;
	to10msg[5] = pkt->dev >> 8;
	to10msg[6] = 0;
	to10msg[7] = 0;
	to10msg[8] = pkt->wd1;
	to10msg[9] = pkt->wd1 >> 8;
	msg        = to10msg;

	// Transfer TO10 Message
	mask = (1 << size) - 1;
	msgData = 0;
	for (idx = 0; idx < QMH_SIZ; idx++) {
		if ((pos -= size) < 0) {
			pos = 36 - size;
			p10_eWrite(tbpAddr++, msgData);
			msgData = 0;
		}
		msgData |= (int36)(*msg++ & mask) << pos;
	}
	p10_eWrite(tbpAddr, msgData);

	// Update TO-11 Byte Pointer
	tbp = BP_PUTPOS(pos) | BP_PUTSIZE(size) | tbpAddr;
	DTE_CB10BP = SXT36(tbp);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: TO10 Message: Cnt=%04X Fnc=%04X Dev=%04X Wd1=%04X\n",
			dte->Unit.devName, pkt->cnt, pkt->fnc, pkt->dev, pkt->wd1);
#endif /* DEBUG */
}


void dte20_ReadData(register DTE_DEVICE *dte,
	register DTE_PACKET *pkt, uint16 qct)
{
	int36      ebp = DTE_CB11BP;
	uint32     ebpAddr;
	int32      pos, size, mask;
	int36      data;
	uint8      *msg;
	int        idx;

	// Extract address, position, and size
	// from byte pointer word.
	ebpAddr = RH18(ebp);
	pos     = BP_GETPOS(ebp);
	size    = BP_GETSIZE(ebp);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("%s: Read Message Data\n", dte->Unit.devName);
		dbg_Printf("%s: DTEEBP (TO-11 Byte Pointer) => %06o,,%06o\n",
			dte->Unit.devName, LH18(ebp), RH18(ebp));
		dbg_Printf("%s:   (Position: %d  Size: %d  Address: %o)\n",
			dte->Unit.devName, pos, size, ebpAddr);
	}
#endif /* DEBUG */

	// Transfer TO11 Message
	data = p10_eRead(ebpAddr);
	mask = (1 << size) - 1;
	msg  = &pkt->Data[0];
	for (idx = 0; idx < qct; idx++) {
		if ((pos -= size) < 0) {
			pos = 36 - size;
			data = p10_eRead(++ebpAddr);
		}
		*msg++ = (uint8)(data >> pos) & mask;
	}
	pkt->xfrCount += qct;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		PrintDump(0, &pkt->Data[0], qct);
#endif /* DEBUG */

	// Update TO-11 Byte Pointer
	ebp = BP_PUTPOS(pos) | BP_PUTSIZE(size) | ebpAddr;
	DTE_CB11BP = SXT36(ebp);
}

void dte20_WriteData(register DTE_DEVICE *dte, register DTE_PACKET *pkt)
{
	int36      tbp = DTE_CB10BP;
	int36      msgData;
	uint32     tbpAddr;
	int32      pos, size, mask;
	int36      data;
	uint8      *msg;
	int        idx;

	// Extract address, position, and size
	// from byte pointer word.
	tbpAddr = RH18(tbp);
	pos     = BP_GETPOS(tbp);
	size    = BP_GETSIZE(tbp);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("%s: Write Message Data\n", dte->Unit.devName);
		dbg_Printf("%s: DTETBP (TO-10 Byte Pointer) => %06o,,%06o\n",
			dte->Unit.devName, LH18(tbp), RH18(tbp));
		dbg_Printf("%s:   (Position: %d  Size: %d  Address: %o)\n",
			dte->Unit.devName, pos, size, tbpAddr);
	}
#endif /* DEBUG */

	// Transfer TO10 Message
	mask = (1 << size) - 1;
	msg  = &pkt->Data[0];
	msgData = 0;
	for (idx = 0; idx < (pkt->cnt - QMH_SIZ); idx++) {
		if ((pos -= size) < 0) {
			pos = 36 - size;
			p10_eWrite(tbpAddr++, msgData);
			msgData = 0;
		}
		msgData |= (int36)(*msg++ & mask) << pos;
	}
	p10_eWrite(tbpAddr, msgData);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		PrintDump(0, &pkt->Data[0], (pkt->cnt - QMH_SIZ));
#endif /* DEBUG */

	// Update TO-10 Byte Pointer
	tbp = BP_PUTPOS(pos) | BP_PUTSIZE(size) | tbpAddr;
	DTE_CB10BP = SXT36(tbp);
}

// Start message transfers to PDP-10.
void dte20_Start10(register DTE_DEVICE *dte)
{
	DTE_PACKET *pkt;

	if (pkt = dte->pktQueue) {
		// Increment TO10IC by one and
		// and send status to PDP-10.
		dte->to10st =
			((dte->to10st + (1 << 8)) & ST_TO10IC) | (dte->to10st & ~ST_TO10IC);
		dte20_Store(dte, dte->dt10Addr + 2, dte->to10st);

		// Send TO10 queue size to PDP-10.
		dte20_Store(dte, dte->dt10Addr + 3, pkt->cnt);

		// Finally ring doorbell.
		dte->srFlags |= DTE_TO10DB;
	}

}

inline void dte20_SendPacket(register DTE_DEVICE *dte,
	register DTE_PACKET *pkt)
{
	if (dte->pktQueue) {
		dte20_Enqueue(dte, pkt, TAIL);
		dte->srFlags |= DTE_TO10DB;
	} else {
		dte20_Enqueue(dte, pkt, TAIL);
		dte20_Start10(dte);
	}
}

void dte20_DoXfer10(register DTE_DEVICE *dte)
{
	DTE_PACKET *pkt;

	// If no packets in queue, do nothing and return.
	if ((pkt = dte->pktQueue) == NULL)
		return;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Transfer to PDP-10: Cnt=%04X Fnc=%04X Dev=%04X Wd1=%04X\n",
			dte->Unit.devName, pkt->cnt, pkt->fnc, pkt->dev, pkt->wd1);
#endif /* DEBUG */

	dte20_WriteHeader(dte, pkt);
	if ((pkt->cnt - QMH_SIZ) > 0)
		dte20_WriteData(dte, pkt);

	// Transmission done.
	dte->srFlags |= DTE_TO10DN;

	if (dte->t10bCount & TO10IB) {
		// Finally, dequeue a packet from queue
		// and place it back to free list.
		pkt = dte20_Dequeue(dte);
		pkt->Next = dte->pktFree;
		dte->pktFree = pkt;

		// If any packets still are remaining in the
		// queue, tell PDP-10 to start xfers again.
		if (dte->pktQueue)
			dte20_Start10(dte);
	}

	// Finally, ring doorbell to PDP-10.
	dte20_Ring10(dte);
}

int dte20_AckLine(register DTE_DEVICE *);
int dte20_ReqDayTime(register DTE_DEVICE *);

// 01 - Line Count Is (Request Devices)
int dte20_ReqDevices(register DTE_DEVICE *dte)
{
	register DTE_PACKET *pkt;

	// Set up and send a ALS (CTY Alias Is) packet
	if ((pkt = dte20_GetFreePacket(dte)) == NULL)
		return DTE_ERROR;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Line Count Is\n", dte->Unit.devName);
#endif /* DEBUG */

	pkt->cnt = QMH_SIZ;
	pkt->fnc = FNC_ALS; 
	pkt->dev = DEV_CTY;
	pkt->wd1 = DLS_CTY;

	dte20_SendPacket(dte, pkt);

	// Finally, send time of day message
	// packet to PDP-10 and return.
	return dte20_ReqDayTime(dte);
}

// 03 - Here is String
int dte20_SendOneString(register DTE_DEVICE *dte, register DTE_PACKET *msg)
{
	int len;

	if ((msg->wd1 >> 8) == DLS_CTY) {
		// Send a string to the console terminal.
		dte20_ctyStrOut(dte, &msg->Data[0], msg->wd1);

		// Acknowledge CTY device that it was done.
//		return dte20_AckLine(dte);
		ts10_SetTimer(&dte->ackTimer);
	}

	return DTE_OK;
}

// 04 - Line-Char
int dte20_SendOneChar10(register DTE_DEVICE *dte) {
	DTE_PACKET *pkt;

	if ((pkt = dte20_GetFreePacket(dte)) == NULL)
		return DTE_ERROR;

	pkt->cnt = QMH_SIZ;
	pkt->fnc = FNC_LNC; // Line-Char Function
	pkt->dev = DEV_CTY;
	pkt->wd1 = (DLS_CTY << 8) |
		dte->inBuffer[dte->idxOutQueue];
	if (++dte->idxOutQueue == 4096)
		dte->idxOutQueue = 0;

	// Enqueue a packet to queue to being sent to PDP-10.
	dte20_SendPacket(dte, pkt);
	dte20_Ring10(dte);

	return DTE_OK;
}

// 11 - Request Day and Time
int dte20_ReqDayTime(register DTE_DEVICE *dte)
{
	register DTE_PACKET *pkt;
	register struct tm *tm;
	struct timezone tz;
	time_t now;
	uint32 year, secs;

	if ((pkt = dte20_GetFreePacket(dte)) == NULL)
		return DTE_ERROR;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Request Day and Time\n", dte->Unit.devName);
#endif /* DEBUG */
	
	// Get time information from native system.
	now = time(NULL);
	tm = localtime(&now);
//	gettimeofday(NULL, &tz);

	// Set up message header
	pkt->cnt = DTM_LEN;   // Date/Time Packet Length
	pkt->fnc = FNC_HDT;   // Here is Date and Time.
	pkt->dev = DEV_CLK;   // Clock Device
	pkt->wd1 = DTM_VALID; // Valid Mark
	
	// Set up message data
	year = tm->tm_year + 1900;
	secs = ((((tm->tm_hour * 60) + tm->tm_min) * 60) + tm->tm_sec) >> 1;

	pkt->Data[0] = year >> 8;
	pkt->Data[1] = year;
	pkt->Data[2] = tm->tm_mon;
	pkt->Data[3] = tm->tm_mday - 1;
	pkt->Data[4] = (tm->tm_wday + 6) % 7;
	pkt->Data[5] = (tm->tm_isdst ? DTM_DST : 0) | 5;
	pkt->Data[6] = secs >> 8;
	pkt->Data[7] = secs;

	// Enqueue a DTM packet to queue to being transferred.
	dte20_SendPacket(dte, pkt);

	dte->Flags |= DTE_DTMSENT;

	return DTE_OK;
}

// 17 - Line Buffer Empty (Acknowledge)
int dte20_AckLine(register DTE_DEVICE *dte)
{
	DTE_PACKET *pkt;

	if ((pkt = dte20_GetFreePacket(dte)) == NULL)
		return DTE_ERROR;

	// Set up message header
	pkt->cnt = QMH_SIZ;   // Message Length
	pkt->fnc = FNC_ACK;   // Acknowledge (Line Buffer Empty).
	pkt->dev = DEV_CTY;   // CTY Device
	pkt->wd1 = DLS_CTY;   // CTY Line

	// Enqueue a ACK packet to queue to being transferred.
	dte20_SendPacket(dte, pkt);

	return DTE_OK;
}

// 25 - Acknowledge All Devices/Units
int dte20_AckAllDevices(register DTE_DEVICE *dte)
{
	register DTE_PACKET *pkt;

	// Set up and send a ALS (CTY Alias Is) packet
	if ((pkt = dte20_GetFreePacket(dte)) == NULL)
		return DTE_ERROR;
	
#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Acknowledge All Devices\n", dte->Unit.devName);
#endif /* DEBUG */

	pkt->cnt = QMH_SIZ;
	pkt->fnc = FNC_AKA;
	pkt->dev = 7;
	pkt->wd1 = 0;

	dte20_SendPacket(dte, pkt);

	return DTE_OK;
}

// Callback from timer routine
void dte20_Service(void *dptr)
{
	DTE_DEVICE *dte = (DTE_DEVICE *)dptr;
}

int dte20_Execute(register DTE_DEVICE *dte, register DTE_PACKET *pkt)
{
	switch (pkt->fnc & 0xFF) {
		case FNC_LCI: // (01) Line Count Is (Request Devices).
			return dte20_ReqDevices(dte);
		case FNC_STR: // (03) Here is String.
			return dte20_SendOneString(dte, pkt);
		case FNC_RDT: // (11) Request Day and Time.
			return dte20_ReqDayTime(dte);
		case FNC_HDT: // (12) Here is Day and Time.
		case FNC_HLS: // (22) Here are line speeds.
		case FNC_EDR: // (27) Enable/Disable Datasets.
		case FNC_AXF: // (34) Enable/Disable Auto-XOFF.
		case FNC_DBF: // (37) Debug Turned Off
			return DTE_OK;
	}

#ifdef DEBUG
	dbg_Printf("%s: Unknown Function: %o\n",
		dte->Unit.devName, pkt->fnc & 0xFF);
#endif /* DEBUG */

	return DTE_ERROR;
}

// Send Delayed Acknowledge to PDP-10.
void dte20_AckTimer(void *dptr)
{
	DTE_DEVICE *dte = (DTE_DEVICE *)dptr;

	if (dte20_AckLine(dte) == DTE_OK)
		dte20_Ring10(dte);

//	return TS10_OK;
}

// Keep Alive Counter - Once each 1/2 second.
void dte20_KeepAlive(void *dptr)
{
	DTE_DEVICE *dte = (DTE_DEVICE *)dptr;
	int36      kac;

	if (dte->Protocol != PRIMARY)
		return /* TS10_OK */;
	if (dte->eptDepSize <= 5)
		return /* TS10_OK */;

	// Increment KAC by one.
	kac = dte->eptDeposit[5] + 1;
	dte->eptDeposit[5] = SXT36(kac);

//	return TS10_OK;
}

// Perform primary protocol.
int dte20_DoPrimary(register DTE_DEVICE *dte)
{
	DTE_PACKET *pkt; // TO11 Message Packet
	int36  st;       // STATUS Word
	uint16 to10ic, to11ic;
	uint16 qct;

	// First, check valid examine access and valid bit on status
	// word.  If not, return back to secondary protocol.
	if ((DTE_CBEPW == 0) ||
	    ((st = dte20_Fetch(dte, dte->et11Addr + 2)) == 0) ||
	    !(st & ST_VALID)) {
#ifdef DEBUG
		if (dbg_Check(DBG_IODATA)) {
			if (DTE_CBEPW)
				dbg_Printf("%s: Not Valid Status: %06o,,%06o\n",
					dte->Unit.devName, LH18(st), RH18(st));
			dbg_Printf("%s: Enter Secondary Protocol.\n",
				dte->Unit.devName);
		}
#endif /* DEBUG */
		ts10_CancelRealTimer(&dte->kacTimer);
		dte->Protocol = SECONDARY;
		return DTE_ERROR;
	}

	// PDP-10 sends indirect data message here.
	if (st & ST_IND) {
//		if ((dte->t11Flags & TO11_INDIR) == 0) {
//		}
		if (st & ST_16BIT)
			dte->t11Flags |= TO11_16BIT;

		qct = dte20_Fetch(dte, dte->et11Addr + 3);
//		dbg_Printf("%s: Word Count = %d\n", dte->Unit.devName, qct);
		
		pkt = &dte->pktMsg;
		dte20_ReadData(dte, pkt, qct);
	} else {

		// PDP-10 sends direct data message here.
		to11ic = (uint16)st & ST_M_IC;
		if (to11ic == dte->to11qc) {
			// No difference means that no queue requests
			// Just set TOIT bit and send status to PDP-10.
			dte->to10st |= ST_TOIT;
			dte20_Store(dte, dte->dt10Addr + 2, dte->to10st);
			return DTE_OK;
		}
#ifdef DEBUG
		else if (to11ic != (dte->to11qc + 1)) {
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: *** Mismatch Queue Count (11QC %d != 11IC %d)\n",
					dte->Unit.devName, dte->to11qc + 1, to11ic);
		}
#endif /* DEBUG */

		// Bump up queue count.
		dte->to11qc = to11ic;

		// Get message header from PDP-10.
		pkt = dte20_ReadHeader(dte);
		dte->t11Flags |= TO11_GOTHDR;

		// Check message header that request indirect data.
		if (pkt->fnc & QMH_IND) {
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: Message packet has indirect data.\n",
					dte->Unit.devName);
#endif /* DEBUG */
			dte->t11Flags |= TO11_INDIR;
		
			// Tell PDP-10 xfer is done and return.
			dte20_XferDone11(dte);
			return DTE_OK;
		}

		// Get message data from PDP-10.
		if ((qct = (pkt->cnt - QMH_SIZ)) > 0)
			dte20_ReadData(dte, pkt, qct);
	}

	// Now execute function.  When it is
	// successful, tell PDP-10 to continue.
	dte20_XferDone11(dte);
	dte20_Execute(dte, pkt);
	dte->t11Flags &= ~(TO11_GOTHDR|TO11_INDIR);

	return DTE_OK;
}

// ****************************************************
// *********** Secondary Protocol Process *************
// ****************************************************

// Get Time/Date Command (Secondary Protocol)
void dte20_GetTOD(DTE_DEVICE *dte, int32 dteCmd)
{
	uint30 eptAddr = KL10_eptAddr + (dteCmd >> 16);
	uint32 tod0, tod1, tod2;
	struct tm *tm;
	struct timezone tz;
	time_t now;

	// Get time information from native system.
	now = time(NULL);
	tm = localtime(&now);
	gettimeofday(NULL, &tz);

	// Packing time information into three TOD words.
	tod0 = TOD_VALID | (tm->tm_year + 1900);
	tod1 = (tm->tm_mon << 24) | ((tm->tm_mday - 1) << 16) |
	  (((tm->tm_wday + 6) % 7) << 8) | (tm->tm_isdst > 0 ? TOD_DST : 0);
	tod2 = ((((tm->tm_hour * 60) + tm->tm_min) * 60 + tm->tm_sec) >> 1) << 16;

	// Put three TOD words into EPT area.
	p10_pWrite(eptAddr,   tod0, 0);
	p10_pWrite(eptAddr+1, tod1, 0);
	p10_pWrite(eptAddr+2, tod2, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		dbg_Printf("%s: %o,,%06o <= TOD0 %06o,,%06o (%08X)\n",
			dte->Unit.devName, LH18(eptAddr), RH18(eptAddr),
			LH18(tod0), RH18(tod0), tod0);
		dbg_Printf("%s: %o,,%06o <= TOD1 %06o,,%06o (%08X)\n",
			dte->Unit.devName, LH18(eptAddr+1), RH18(eptAddr+1),
		  	LH18(tod1), RH18(tod1), tod1);
		dbg_Printf("%s: %o,,%06o <= TOD2 %06o,,%06o (%08X)\n",
			dte->Unit.devName, LH18(eptAddr+2), RH18(eptAddr+2),
		  	LH18(tod2), RH18(tod2), tod2);
	}
#endif /* DEBUG */
}

// Secondary Protocol Process
void dte20_DoSecondary(DTE_DEVICE *dte20)
{
	int32 dteCmd = p10_pRead(KL10_eptAddr + DTECMD, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: DTECMD => %06o,,%06o\n",
			dte20->Unit.devName, LH18(dteCmd), RH18(dteCmd));
#endif /* DEBUG */

	switch ((dteCmd >> CMD_P_CODE) & CMD_M_CODE) {
		case CMD_MTO:
			// Monitor Output Command
			dte20_ctyCharOut(dte20, dteCmd & MTO_CHAR);
			p10_pWrite(KL10_eptAddr + DTEMTD, dte20->mtdFlags | MTD_ODN, 0);
			if (dte20->srFlags & DTE_PIOENB) {
				dte20->srFlags |= DTE_TO10DB;
				dte20_Ring10(dte20);
			}
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS)) {
				uint8 ch = dteCmd & 0277;
				dbg_Printf("%s: Output Character = %c (%02X)\n",
					dte20->Unit.devName, ((ch >= ' ' && ch < 127) ? ch : '.'), ch);
			}
#endif /* DEBUG */
			break;

		case CMD_ESP:
			dte20->Protocol = SECONDARY;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Enter Secondary Protocol\n",
					dte20->Unit.devName);
#endif /* DEBUG */
			break;

		case CMD_LSP:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Leave Secondary Protocol\n",
					dte20->Unit.devName);
#endif /* DEBUG */

			// Enter primary protocol.
			if (dte20_EnterPrimary(dte20))
				break;

			// If successful, do primary protocol to
			// process initial packets.
			if (dte20_DoPrimary(dte20))
				break;

			// If all packets are successful, finally send
			// an ACK packet to acknowledge all devices
			// and units.
			dte20_AckAllDevices(dte20);
			if ((dte20->Flags & DTE_DTMSENT) == 0)
				dte20_ReqDayTime(dte20);
			break;

		case CMD_GDT:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Get Date/Time.\n",
					dte20->Unit.devName);
#endif /* DEBUG */
			dte20_GetTOD(dte20, dteCmd);
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unknown command code %02o\n", 
					dte20->Unit.devName, (dteCmd >> 8) & 017);
#endif /* DEBUG */
	}

	// Tell KL10 that DTE command was done.
	p10_pWrite(KL10_eptAddr + DTEFLG, -1, 0);

	// Clear TO-11 doorbell.
	dte20->srFlags &= ~DTE_TO11DB;
}

//****************************************************************

void dte20_ResetIO(void *dptr)
{
}

void dte20_Opcode_DODTE(void *dptr)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)dptr;

	dte20->t10bCount = p10_vRead(eAddr, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		int32 bCount = dte20->t10bCount & TO10BC;
		dbg_Printf("%s: TO10 Byte Count <= %o (%d)  Interrupt: %s\n",
			dte20->Unit.devName, bCount, bCount,
			(dte20->t10bCount & TO10IB ? "Yes" : "No"));
	}
#endif /* DEBUG */

	// Transfer TO10 message to 10's memory.
	dte20_DoXfer10(dte20);
}

void dte20_Opcode_WRDTE(void *dptr)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)dptr;

	// Clear done/doorbell/error flags
//	dte20->srFlags &= ~(eAddr & DTE_CLR);
	if (eAddr & DTE_CLTO11) // Clear TO-11 Done and Error
		dte20->srFlags &= ~(DTE_TO11DN|DTE_TO11ER);
	if (eAddr & DTE_CLTO10) // Clear TO-10 Done and Error
		dte20->srFlags &= ~(DTE_TO10DN|DTE_TO10ER);
	if (eAddr & DTE_CL11PT) // Clear TO-10 Doorbell
		dte20->srFlags &= ~(DTE_TO10DB);

	// Load PI assignment.
	if (eAddr & DTE_PILDEN) {
		dte20->srFlags = (dte20->srFlags & ~(DTE_PIOENB|DTE_PIA)) |
			(eAddr & (DTE_PIOENB|DTE_PIA));
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (WRDTE) SR %06o <= %06o\n",
			dte20->Unit.devName, dte20->srFlags, (uint16)eAddr);
#endif /* DEBUG */

	// Ring doorbell to PDP11.
	if (eAddr & DTE_TO11DB) {
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: Rings Doorbell at %o,,%o\n",
				dte20->Unit.devName, LH18(pager_PC), RH18(pager_PC));
#endif /* DEBUG */

		// Set TO-11 Doorbell
		dte20->srFlags |= DTE_TO11DB;

		// Finally, perform primary/secondary protocol.
		if (dte20->Protocol == PRIMARY) {
			if (dte20_DoPrimary(dte20))
				dte20_DoSecondary(dte20);
		} else
			dte20_DoSecondary(dte20);
		dte20->srFlags &= ~DTE_TO11DB;
	}

	// If any interrupt bits are remaing,
	// ring doorbell again, please.
	dte20_Ring10(dte20);
}

void dte20_Opcode_RDDTE(void *dptr)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)dptr;

	p10_vWrite(eAddr, dte20->srFlags, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (RDDTE) SR => %06o\n",
			dte20->Unit.devName, dte20->srFlags);
#endif /* DEBUG */
}

void dte20_Opcode_CZDTE(void *dptr)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)dptr;

	if ((dte20->srFlags & eAddr) == 0)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		int32 result = dte20->srFlags & eAddr;
		dbg_Printf("%s: (CZDTE) SR => %06o & %06o = %06o : %s\n",
			dte20->Unit.devName, dte20->srFlags, (int18)eAddr, result,
			((result == 0) ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

void dte20_Opcode_CODTE(void *dptr)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)dptr;

	if (dte20->srFlags & eAddr)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		int32 result = dte20->srFlags & eAddr;
		dbg_Printf("%s: (CODTE) SR => %06o & %06o = %06o : %s\n",
			dte20->Unit.devName, dte20->srFlags, (int18)eAddr, result,
			(result ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

void *dte20_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	DTE_DEVICE *dte20 = NULL;
	P10_IOMAP  *io;
	CLK_QUEUE  *timer;
	uint32     idUnit;
	uint32     devAddr;

	if (dte20 = (DTE_DEVICE *)calloc(1, sizeof(DTE_DEVICE))) {
		// Set up its descriptions on new device.
		dte20->Unit.devName    = newMap->devName;
		dte20->Unit.keyName    = newMap->keyName;
		dte20->Unit.emuName    = newMap->emuName;
		dte20->Unit.emuVersion = newMap->emuVersion;

		// Get unit # from device name.
		idUnit = GetDeviceUnit(newMap->devName);
		if (idUnit < DTE_MAXUNITS)
			devAddr = DTE_BASE + (idUnit << 2);
		printf("%s: Unit: %d Address: %03o\n",
			newMap->devName, idUnit, devAddr);

		dte20->idDevice  = idUnit;
		dte20->idChannel = idUnit + 010;
		dte20_ResetDevice(dte20);

		timer           = &dte20->kacTimer;
		timer->Next     = NULL;
		timer->Name     = "DTE20 Keep Alive Timer";
		timer->Flags    = CLK_ENABLE|CLK_REACTIVE;
		timer->outTimer = 50000;
		timer->nxtTimer = 50000;
		timer->Device   = dte20;
		timer->Execute  = dte20_KeepAlive;

		// Acknowledge Delay Timer for CTY device.
		timer           = &dte20->ackTimer;
		timer->Next     = NULL;
		timer->Name     = "DTE20 Acknowledge Line";
		timer->Flags    = 0;
		timer->outTimer = 2500;
		timer->nxtTimer = 2500;
		timer->Device   = dte20;
		timer->Execute  = dte20_AckTimer;

		// Acknowledge Delay Timer for CTY device.
		timer           = &dte20->cinTimer;
		timer->Next     = NULL;
		timer->Name     = "DTE20 Console Input Queue";
		timer->Flags    = 0;
		timer->outTimer = 500;
		timer->nxtTimer = 500;
		timer->Device   = dte20;
		timer->Execute  = dte20_CheckQueue;

		// Set up I/O mapping
		io             = &dte20->ioMap;
		io->devName    = dte20->Unit.devName;
		io->keyName    = dte20->Unit.keyName;
		io->emuName    = dte20->Unit.emuName;
		io->emuVersion = dte20->Unit.emuVersion;
		io->Device     = dte20;
		io->idDevice   = devAddr;
		io->ResetIO    = dte20_ResetIO;

		// Set up I/O instructions
		io->Function[IOF_BLKI]  = NULL;
		io->Function[IOF_DATAI] = NULL;
		io->Function[IOF_BLKO]  = NULL;
		io->Function[IOF_DATAO] = dte20_Opcode_DODTE;
		io->Function[IOF_CONO]  = dte20_Opcode_WRDTE;
		io->Function[IOF_CONI]  = dte20_Opcode_RDDTE;
		io->Function[IOF_CONSZ] = dte20_Opcode_CZDTE;
		io->Function[IOF_CONSO] = dte20_Opcode_CODTE;

		// Assign I/O map to PDP-10 device table
		kx10_SetMap(io);
	
		// Set up server socket
		dte20_InitSockets(dte20);

		// Finally, link its device to mapping
		// device and return.
		newMap->Device = dte20;
	}

	return dte20;	
}

int dte20_Delete(void *dptr)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)dptr;

	sock_Close(dte20->ctySocket);
	sock_Close(dte20->ctyServer);

	dte20->ctySocket = NULL;
	dte20->ctyServer = NULL;

	return 0;
}

int dte20_Reset(void *dptr)
{
	DTE_DEVICE *dte20 = (DTE_DEVICE *)dptr;

	dte20_ResetDevice(dte20);
}

DEVICE dte20_Device =
{
	DTE20_KEY,        // Key Name (Device Type)
	DTE20_NAME,       // Emulator Name
	DTE20_VERSION,    // Emulator Version
	NULL,             // Lisiting of KL10 Devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	NULL, NULL, NULL,

	dte20_Create,     // Create Routine
	NULL,             // Configure Routine
	dte20_Delete,     // Delete Routine
	dte20_Reset,      // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Routine
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
