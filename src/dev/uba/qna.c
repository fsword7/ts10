// qna.c - DEQNA/DELQA Qbus Ethernet Emulator System
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

/*
  -------------------------------------------------------------------------

  This DEQNA/DELQA emulation is based on:
    Digital DEQNA Users Guide: Part# EK-DEQNA-UG-001
    Digital DELQA Users Guide: Part# EK-DELQA-UG-002

  Online manuals are available on Al Kossow's PDF archives:
    http://www.spies.com/~aek/pdf/dec/qbus
 
  -------------------------------------------------------------------------

  Modification History:

  01/21/03  TMS  Added DELQA-PLUS (Turbo).
  01/13/03  TMS  Re-implemented the new circular packet queue management.
  01/13/03  TMS  Removed all SQA settings because I learned that DEQNA
                 and DELQA are compitable but not DESQA.  Also, removed
                 DESQA device information.
  01/13/03  TMS  Modification history starts here.

  -------------------------------------------------------------------------
*/

#include "emu/defs.h"
#include "emu/socket.h"
#include "dev/dec/qna.h"
#include "dev/dp/epp.h"

// Sanity Timeout Table
uint32 qna_Timeout[] =
{
	TIMEOUT0, // 0 (000) - 1/4 second
	TIMEOUT1, // 1 (001) -  1  second
	TIMEOUT2, // 2 (010) -  4  seconds
	TIMEOUT3, // 3 (011) - 16  seconds
	TIMEOUT4, // 4 (100) -  1  minute
	TIMEOUT5, // 5 (101) -  4  minutes
	TIMEOUT6, // 6 (110) - 16  minutes
	TIMEOUT7, // 7 (111) - 64  minutes
};

#ifdef DEBUG
static cchar *regNameR[] =
{
	"PROM0",     // PROM Station Address #0
	"PROM1",     // PROM Station Address #1
	"PROM2",     // PROM Station Address #2
	"PROM3",     // PROM Station Address #3
	"PROM4",     // PROM Station Address #4
	"PROM5",     // PROM Station Address #5
	"VECTOR",    // Vector Address Register
	"CSR"        // Control and Status Register
};

static cchar *regNameW[] =
{
	"<Unknown>",
	"<Unknown>",
	"RBDL1",     // Receive BDL Start Address #1
	"RBDL2",     // Receive BDL Start Address #2
	"TBDL1",     // Transmit BDL Start Address #1
	"TBDL2",     // Transmit BDL Start Address #2
	"VECTOR",    // Vector Address Register
	"CSR"        // Control and Status Register
};
#endif /* DEBUG */
// ***************************************************************

#ifdef DEBUG
void epp_DumpPacket(uchar *pkt, int len)
{
	uchar ascBuffer[17];
	uchar ch, *pasc;
	int   idxAddr, idx;

	// Display Ethernet Head Information
	dbg_Printf("Ethernet %02X:%02X:%02X:%02X:%02X:%02X to %02X:%02X:%02X:%02X:%02X:%02X, type %04X\n",
		pkt[6], pkt[7], pkt[8], pkt[9], pkt[10], pkt[11], // Source
		pkt[0], pkt[1], pkt[2], pkt[3], pkt[4], pkt[5],   // Destination
		(pkt[12] << 8) | pkt[13]);                        // Type

	// Display Ethernet Data Information
	for (idxAddr = 14; idxAddr < len;) {
		dbg_Printf("%04X: ", idxAddr - 14);

		pasc = ascBuffer;
		for (idx = 0; (idx < 16) && (idxAddr < len); idx++) {
			ch = pkt[idxAddr++];
			dbg_Printf("%c%02X", (idx == 8) ? '-' : ' ', ch);
			*pasc++ = ((ch >= 32) && (ch < 127)) ? ch : '.';
		}

		for (; idx < 16; idx++)
			dbg_Printf("   ");
		*pasc = '\0';
		dbg_Printf(" |%-16s|\n", ascBuffer);
	}
}
#endif /* DEBUG */

int  qna_EnterFIFO(QNA_DEVICE *, uint32, void *, uint32);
int  qna_FlushFIFO(QNA_DEVICE *);

void epp_InputWorld(SOCKET *, char *, int);
void qna_Enqueue(QNA_DEVICE *, int, ETH_PACKET *, uint16 *);
void qna_Dequeue(QNA_DEVICE *);
int  qna_PutStatus(QNA_DEVICE *, int, uint8 *);

void epp_MakeChecksum(QNA_DEVICE *qna, uint8 *ethAddr)
{
	uint32 cksum, add;
	uint32 idx;

	// Calculate a new checksum with Ethernet address.
	cksum = 0;
	for (idx = 0; idx < 6; idx += 2) {
		if ((cksum <<= 1) > WMASK)
			cksum -= WMASK;
		add = ethAddr[idx] | (ethAddr[idx+1] << 8);
		if ((cksum += add) > WMASK)
			cksum -= WMASK;
	}
	if (cksum == WMASK)
		cksum = 0;

	// Put new checksum value into last two bytes of
	// station address in PROM.
//	ethAddr[6] = ((uint8 *)&cksum)[0];
//	ethAddr[7] = ((uint8 *)&cksum)[1];
	ethAddr[6] = cksum & 0xFF;
	ethAddr[7] = (cksum >> 8) & 0xFF;

#ifdef DEBUG
	dbg_Printf("%s: Ethernet: %02X:%02X:%02X:%02X:%02X:%02X  Checksum: %02X:%02X\n",
		qna->Unit.devName,
		ethAddr[0], ethAddr[1], ethAddr[2],
		ethAddr[3], ethAddr[4], ethAddr[5],
		ethAddr[6], ethAddr[7]);
#endif /* DEBUG */
}

// Usage: create ... <tun|tap>
SOCKET *epp_OpenTUN(QNA_DEVICE *qna, char *name)
{
	SOCKET *tun;
	char   tunName[40];

	strcpy(tunName, name);
	if(tun = sock_Open(tunName, 0, NET_TUN)) {
		tun->Accept  = NULL;
		tun->Eof     = NULL;
		tun->Process = epp_InputWorld;
		tun->Device  = qna;
		qna->World   = tun;

		// Get Ethernet Address from TUN/TAP connection.
		SockGetEtherAddr(tun->Name, qna->tunAddr);
		epp_MakeChecksum(qna, qna->tunAddr);

		qna->ownAddr[0] = 0x00;
		qna->ownAddr[1] = 0xFF;
		qna->ownAddr[2] = 0x10;
		qna->ownAddr[3] = 0x20;
		qna->ownAddr[4] = 0x30;
		qna->ownAddr[5] = 0x40;
		epp_MakeChecksum(qna, qna->ownAddr);

		// Tell operator that.
		printf("Opening TUN connection (%s) on %s:\n",
			tunName, qna->Unit.devName);
		return tun;
	}

	printf("Can't open TUN connection (%s) on %s: - Aborted.\n",
		tunName, qna->Unit.devName);
	return NULL;
}

void epp_InputWorld(SOCKET *tun, char *pkt, int len)
{
	QNA_DEVICE *qna = (QNA_DEVICE *)tun->Device;
	ETH_PACKET epkt;
	uint16     status[2];

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		epp_DumpPacket((uchar *)pkt, len);
#endif /* DEBUG */

	// Check if receiver is enabled.
	// Otherwise, frames will be dropped.
	if ((qna->csr & CSR_RE) == 0) {
#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: Packet dropped (Receiver is disabled)\n",
				qna->Unit.devName);
#endif /* DEBUG */
		return;
	}

	// Enter that packet into FIFO queue.
	qna_EnterFIFO(qna, EPP_RECEIVE, pkt, len);

	// Enter that RSTATUS information into FIFO queue.
	if (len < ETH_MIN)
		len = ETH_MIN;
	status[0]  = (len - ETH_MIN) & RSW_RBLH;
	status[1]  = (len - ETH_MIN) & RSW_RBLL;
	status[1] |= (status[1] << 8);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Receive Status: %06o %06o\n",
			qna->Unit.devName, status[0], status[1]);
#endif /* DEBUG */

	// Enqueue a new packet into the circular queue.
//	memcpy(epkt.msg, pkt, len);
//	epkt.len = len;
//	qna_Enqueue(qna, PKT_NORMAL, &pkt, status);

	qna_EnterFIFO(qna, EPP_RSTATUS, status, sizeof(status));

	// Finally, flush FIFO queue into host memory if available
	// and ring host's doorbell.
	qna_FlushFIFO(qna);
}

void epp_ProcessLoopback(QNA_DEVICE *qna, int type, uint8 *frame, int len)
{
	uint16 status[2];

	if (type == EPP_ELOOP)
		SockSendPacket(qna->World, frame, len);

	// Check its frame length and set up
	// results of RSTATUS words.
	if (len < 6) {
		status[0] = RSW_ERROR|RSW_ESETUP|RSW_DISCARD;
		status[1] = (len & RSW_RBLL) | ((len & RSW_RBLL) << 8);
	} else if (len < 03000) {
		status[0] = RSW_ESETUP | (len & RSW_RBLH);
		status[1] = (len & RSW_RBLL) | ((len & RSW_RBLL) << 8);
	} else {
		status[0] = RSW_ERROR|RSW_ESETUP|RSW_DISCARD|RSW_OVF;
		status[1] = 0;
		len = 03000;
	}

	// Send packet back to host system for loopback tests.
	qna_EnterFIFO(qna, EPP_RECEIVE, frame, len);
	qna_EnterFIFO(qna, EPP_RSTATUS, status, sizeof(status));

	// Finally, flush FIFO queue into host memory if available
	// and ring host's doorbell.
	qna_FlushFIFO(qna);
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void epp_ParseSetup(QNA_DEVICE *qna, uint8 *frame, int len)
{
	int   a, b, c;
	int   nAddrs = 0;
	uint8 *ethAddr;

	// Update promiscious and all-multicast bits
	qna->Flags &= ~(CFLG_PROMISC|CFLG_ALLMULTI);
	if (len > 0200) {
		if (len & SETUP_PROMISC)
			qna->Flags |= CFLG_PROMISC;
		if (len & SETUP_ALLMULTI)
			qna->Flags |= CFLG_ALLMULTI;
	}

	// Extract Ethernet addresses from setup packet.
	for (c = 0; c <= 0100; c += 0100) {
		if (len > 051 + c) {
			for (a = 0; a < MIN(7, (len - 051 - c)); a++) {
				ethAddr = qna->ethAddr[nAddrs++];
				for (b = 0; b < 060; b += 010)
					*ethAddr++ = frame[a + b + c + 1];
			}
		}
	}
	qna->nAddrs = nAddrs;

#ifdef DEBUG
//	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("%s: Setup Packet Information:\n", qna->Unit.devName);
		dbg_Printf("%s:   Length:    %03o (%d bytes)\n",
			qna->Unit.devName, len, len);
		dbg_Printf("%s:   Flags:     %c%c%c\n", qna->Unit.devName,
			((qna->Flags & CFLG_PROMISC) ?  'p' : '-'),
			((qna->Flags & CFLG_ALLMULTI) ? 'a' : '-'),
			(frame[0] ? 'm' : '-'));
		dbg_Printf("%s:   Addresses: (%d entries)\n",
			qna->Unit.devName, qna->nAddrs);
		for (a = 0; a < qna->nAddrs; a++) {
			ethAddr = qna->ethAddr[a];
			dbg_Printf("%s:    %2d: %02X:%02X:%02X:%02X:%02X:%02X\n",
				qna->Unit.devName, a,
				ethAddr[0], ethAddr[1], ethAddr[2],
			  	ethAddr[3], ethAddr[4], ethAddr[5]);
		}
//	}
#endif /* DEBUG */
}

void epp_ProcessSetup(QNA_DEVICE *qna, uint8 *frame, int len)
{
	uint16 status[2];

	if (len <= 0400) {
		// Parse Ethernet Addresses and MOP information
		epp_ParseSetup(qna, frame, len);
		// tell host that loopbacked packet is setup.
		status[0] = RSW_ESETUP|RSW_RBLH;
	} else {
		status[0] = (RSW_ERROR|RSW_ESETUP|RSW_DISCARD|RSW_RBLH) |
			(len > 0400); // Implies RSW_OVF.
		len = 0;
	}

	// Send SETUP packet back to host system.
	qna_EnterFIFO(qna, EPP_RECEIVE, frame, len);

	status[1] = (len & RSW_RBLL) | ((len & RSW_RBLL) << 8);
	qna_EnterFIFO(qna, EPP_RSTATUS, status, sizeof(status));

	// Finally, flush FIFO queue into host memory if available
	// and ring host's doorbell.
	qna_FlushFIFO(qna);
}

void epp_SendFrame(QNA_DEVICE *qna, int type, uint8 *frame, int len)
{
	uint16 status[2]; // Transmit Status Words
	int    rc;

	// Send XSTATUS to host system.
	status[0] = 0;
	status[1] = 0140 + (len * 010);
//	qna_EnterFIFO(qna, EPP_XSTATUS, status, sizeof(status));
	qna_PutStatus(qna, EPP_XSTATUS, (uint8 *)status);

	switch (type) {
		case EPP_TRANSMIT:
			if ((rc = SockSendPacket(qna->World, frame, len)) < 0) {
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: Send Packet Error: %s\n",
						qna->Unit.devName, strerror(errno));
#endif /* DEBUG */
			}
			break;

		case EPP_ILOOP:
		case EPP_ELOOP:
		case EPP_EILOOP:
			epp_ProcessLoopback(qna, type, frame, len);
			break;

		case EPP_SETUP:
			epp_ProcessSetup(qna, frame, len);
			break;

#ifdef DEBUG
		default:
			dbg_Printf("%s: Undefined type %d at line %d\n",
				qna->Unit.devName, type, __LINE__);
#endif /* DEBUG */
	};
}

// ***************************************************************

// Enter a packet entry into the circular queue.
void qna_Enqueue(QNA_DEVICE *qna, int type, ETH_PACKET *pkt, uint16 *Status)
{
	QNA_PACKET *qpkt = &qna->pktList[qna->pktHead++];

	// If queue is empty, start with new head/tail pointers.
	if (qna->pktCount == 0) {
		qna->pktHead = 0;
		qna->pktTail = -1;
	}

	// Get new tail queue to enqueue.
	if (++qna->pktTail == QNA_NPKTS)
		qna->pktTail = 0;
	if (++qna->pktCount >= QNA_NPKTS) {
		qna->pktCount = QNA_NPKTS;
		// Oldest packet is lost.
		if (++qna->pktHead == QNA_NPKTS)
			qna->pktHead = 0;
		qna->pktLoss++;
	}

	// Insert Ethernet Packet into Queue.
	qpkt = &qna->pktList[qna->pktTail];
	memcpy(qpkt->Data.msg, pkt->msg, pkt->len);
	qpkt->Data.len  = pkt->len;
	qpkt->Type      = type;
	qpkt->Status[0] = Status[0];
	qpkt->Status[1] = Status[1];
}

// Remove a packet entry from the circular queue.
void qna_Dequeue(QNA_DEVICE *qna)
{
	QNA_PACKET *pkt;

	if (qna->pktCount) {
		// First, mark the packet as invalid.
		pkt = &qna->pktList[qna->pktHead];
		pkt->Type = PKT_INVALID;

		// Finally, remove the packet.
		if (++qna->pktHead == QNA_NPKTS)
			qna->pktHead = 0;
		qna->pktCount--;
	}
}

inline void qna_DoIRQ(QNA_DEVICE *qna, uint16 bit)
{
	MAP_IO *io = &qna->ioMap;

	if ((qna->csr & bit) == 0) {
		qna->csr |= bit;
		if (qna->csr & CSR_IE) {
#ifdef DEBUG
			if (dbg_Check(DBG_INTERRUPT))
				dbg_Printf("%s: (%s) Ring Doorbell\n", qna->Unit.devName,
					((bit & CSR_XI) ? "XI" : (bit & CSR_RI) ? "RI" : "--"));
#endif /* DEBUG */
			io->SendInterrupt(io, 0);
		}
	}
}

#ifdef DEBUG

inline void qna_DumpDesc(QNA_DEVICE *qna, uint32 addr, QNA_BDL *desc,
	cchar *name, cchar *dir)
{
	dbg_Printf("%s: %s Buffer Descriptor List\n", qna->Unit.devName, name);
	dbg_Printf("%s: %06X %s %04X %04X %04X %04X %04X %04X\n",
		qna->Unit.devName, addr, dir, desc->Flag, desc->hiAddr, desc->loAddr,
		(uint16)desc->szBuffer, desc->Status1, desc->Status2);
}
#endif /* DEBUG */

// Display 3 LEDs.
char *qna_GetLED(int LED)
{
	static char dspl[5];

	dspl[0] = (LED & 8) ? 'o' : '.';
	dspl[1] = (LED & 4) ? 'o' : '.';
	dspl[3] = (LED & 2) ? 'o' : '.';
	dspl[4] = '\0';

	return dspl;
}

int qna_GetData(QNA_DEVICE *qna, uint32 addr, void *data, uint32 size)
{
	UQ_CALL *call = qna->Callback;
	void    *uq   = qna->System;

	if (call->ReadBlock(uq, addr, data, size, 0)) {
		qna->csr |= (CSR_XL|CSR_RL|CSR_NI);
		qna_DoIRQ(qna, CSR_XI);
		return QNA_NXM;
	}

	return QNA_OK;
}

int qna_PutData(QNA_DEVICE *qna, uint32 addr, void *data, uint32 size)
{
	UQ_CALL *call = qna->Callback;
	void    *uq   = qna->System;
	int32   rc;

	if (call->WriteBlock(uq, addr, data, size, 0)) {
		qna->csr |= (CSR_XL|CSR_RL|CSR_NI);
		qna_DoIRQ(qna, CSR_XI);
		return QNA_NXM;
	}

	return QNA_OK;
}

int32 qna_GetFrame(QNA_DEVICE *qna, uint8 *frame)
{
	QNA_BDL desc;
	int32   idxFrame = 0;
	uint32  bufAddr;
	int32   bc;

	forever {
		if (qna_GetData(qna, qna->txBDLAddr, &desc, BDL_SIZE))
			return 0;

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			qna_DumpDesc(qna, qna->txBDLAddr, &desc, "Transmit", "=>");
#endif /* DEBUG */

		// Mark flagword as used now.
		desc.Flag = WMASK;
		if (qna_PutData(qna, qna->txBDLAddr, &desc.Flag, WSIZE))
			return 0;
	
		// First, check BDL is last descriptor or not.
		// If so, transmit process is done and BDL address
		// now is invalid.
		if ((desc.hiAddr & (BDL_VALID|BDL_CHAIN)) == 0) {
			qna->csr |= CSR_XL;
			return 0;
		}

		bufAddr = (((desc.hiAddr & 077) << 16) | desc.loAddr) & ~1;

		// If address is chain, set up and continue;
		if (desc.hiAddr & BDL_CHAIN) {
			qna->txBDLAddr = bufAddr;
			continue;
		}

		// Read buffer from host system.
		if (desc.szBuffer < 0) bc = -((int16)desc.szBuffer) << 1;
		else                   bc = desc.szBuffer << 1;
		if (desc.hiAddr & BDL_LBYTE)
			bc--;
		if (desc.hiAddr & BDL_HBYTE) {
			bc--;
			bufAddr++;
		}
		bc = (bc < (QNA_MAXBUF - idxFrame)) ? bc : (QNA_MAXBUF - idxFrame);

		if (qna_GetData(qna, bufAddr, frame, bc))
			return 0;
		idxFrame += bc;
		frame    += bc;

		// Check descriptor for "end of message" flag.
		if (desc.hiAddr & BDL_EOM)
			return (desc.hiAddr & BDL_SETUP) ? -idxFrame : idxFrame;

		// Mark two status flags for continuing.
		desc.Status1 = WMASK;
		desc.Status2 = WMASK;
		if (qna_PutData(qna, qna->txBDLAddr + 8, &desc.Status1, WSIZE*2))
			return 0;

		// Go next BDL descriptor.
		qna->txBDLAddr += BDL_NEXT;
	}	
}

int qna_PutFrame(QNA_DEVICE *qna, uchar *frame, int len)
{
	QNA_BDL desc;
	int32   idxFrame = 0;
	uint32  bufAddr;
	int32   bc;

	forever {
		if (qna_GetData(qna, qna->rxBDLAddr, &desc, BDL_SIZE))
			return 0;

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			qna_DumpDesc(qna, qna->rxBDLAddr, &desc, "Receive", "=>");
#endif /* DEBUG */

		// Mark flagword as used now.
		desc.Flag = WMASK;
		if (qna_PutData(qna, qna->rxBDLAddr, &desc.Flag, WSIZE))
			return 0;
	
		// First, check BDL is last descriptor or not.
		// If so, transmit process is done and BDL address
		// now is invalid.
		if ((desc.hiAddr & (BDL_VALID|BDL_CHAIN)) == 0) {
			qna->csr |= CSR_RL;
			return idxFrame;
		}

		bufAddr = (((desc.hiAddr & 077) << 16) | desc.loAddr) & ~1;

		// If address is chain, set up and continue;
		if (desc.hiAddr & BDL_CHAIN) {
			qna->rxBDLAddr = bufAddr;
			continue;
		}

		// Write frame to host system.
		// Note: Buffer address must be in
		//       word alignment (boundary).
		if (desc.szBuffer < 0) bc = -((int16)desc.szBuffer) << 1;
		else                   bc = desc.szBuffer << 1;
		bc = (bc < (len - idxFrame)) ? bc : (len - idxFrame);

		if (qna_PutData(qna, bufAddr, frame, bc))
			return idxFrame;
		idxFrame += bc;
		frame    += bc;

		// Check if done.
		if (idxFrame == len)
			return idxFrame;

		// Mark two status flags for continuing.
		desc.Status1 = WMASK;
		desc.Status2 = WMASK;
		if (qna_PutData(qna, qna->rxBDLAddr + 8, &desc.Status1, WSIZE*2))
			return 0;

		// Go next BDL descriptor.
		qna->rxBDLAddr += BDL_NEXT;
	}	

}

int qna_PutStatus(QNA_DEVICE *qna, int type, uint8 *sts)
{
	switch (type) {
		case EPP_XSTATUS:
			if ((qna->csr & CSR_XL) == 0) {
				if (qna_PutData(qna, qna->txBDLAddr + BDL_STATUS1, sts, WSIZE*2))
					return QNA_NXM;
				qna->txBDLAddr += BDL_SIZE;

				// Ring host's doorbell.
				qna_DoIRQ(qna, CSR_XI);
			}
			return QNA_OK;

		case EPP_RSTATUS:
			if ((qna->csr & CSR_RL) == 0) {
				if (qna_PutData(qna, qna->rxBDLAddr + BDL_STATUS1, sts, WSIZE*2))
					return QNA_NXM;
				qna->rxBDLAddr += BDL_SIZE;

				// Ring host's doorbell.
				qna_DoIRQ(qna, CSR_RI);
			}
			return QNA_OK;

#ifdef DEBUG
		default:
			dbg_Printf("%s: Unknown type %d at line %d\n",
				qna->Unit.devName, type, __LINE__);
#endif /* DEBUG */
	}

	return QNA_ERR;
}

int qna_SendFrame(QNA_DEVICE *qna)
{
	uint8  txFrame[QNA_MAXBUF];
	int32  rcBytes;

	if ((rcBytes = qna_GetFrame(qna, txFrame)) == 0)
		return 0;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("%s: %s Frame (Size: %d bytes)\n",
			qna->Unit.devName, ((rcBytes < 0) ? "Setup" : "Ethernet"),
			abs(rcBytes));
		PrintDump(0, txFrame, abs(rcBytes));
	}
#endif /* DEBUG */

	if (rcBytes < 0) {
		// Setup Frame

		rcBytes = -rcBytes;

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: Setup Frame - Loopback bits %06o\n",
				qna->Unit.devName, qna->csr & (CSR_IL|CSR_EL));
#endif /* DEBUG */

		// Transmit a setup frame to Ethernet Protocol Program.
		epp_SendFrame(qna, EPP_SETUP, txFrame, rcBytes);

#if 0
		if (rcBytes > 0200 && rcBytes <= 0400) {
			// Update LED display.
//			qna->LED &= ~(1u << ((rcBytes >> 2) & 3));

			// Set up sanity timer.
//			qna->outTimer = (rcBytes >> 4) & 7;
		}
#endif

		return (qna->csr & CSR_EL);
		/* return 1; */
	}

	// Transmit an Ethernet frame.
	switch (qna->csr & (CSR_IL|CSR_EL)) {
		case CSR_IL:        // Normal Transmission
			epp_SendFrame(qna, EPP_TRANSMIT, txFrame, rcBytes);
			break;

		case CSR_EL:        // Extended Internal Loopback
			epp_SendFrame(qna, EPP_EILOOP, txFrame, rcBytes);
			break;

		case CSR_IL|CSR_EL: // External loopback
			epp_SendFrame(qna, EPP_ELOOP, txFrame, rcBytes);
			break;

		case 0:             // Internal Loopback
			epp_SendFrame(qna, EPP_ILOOP, txFrame, rcBytes);
			break;

#ifdef DEBUG
		default:
			dbg_Printf("%s: Bad loopback bits at line %d\n",
				qna->Unit.devName, __LINE__);
#endif /* DEBUG */
	}

	return 0;
}

int qna_EnterFIFO(QNA_DEVICE *qna, uint32 type, void *data, uint32 size)
{
	if ((qna->idxFifo + size + (WSIZE*2)) >= FIFO_SIZE)
		return -1; // FIFO queue is full.

	// Enter data into FIFO queue.
	*((uint16 *)&qna->fifo[qna->idxFifo])   = type;
	*((uint16 *)&qna->fifo[qna->idxFifo+2]) = size;
	memcpy(&qna->fifo[qna->idxFifo+4], data, size);

	qna->idxFifo += size + (WSIZE*2);

	return size;
}

int qna_FlushFIFO(QNA_DEVICE *qna)
{
	uint16 type, size;
	uint8  *data;
	int32  idxFifo = 0;
	int32  rc = 0;

	while (idxFifo < qna->idxFifo) {
		type = *((uint16 *)&qna->fifo[idxFifo]);
		size = *((uint16 *)&qna->fifo[idxFifo+2]);
		data = &qna->fifo[idxFifo+4];

		switch (type) {
			case EPP_RECEIVE:
				// Put a frame into host memory.
				if ((qna->csr & CSR_RL) == 0)
					rc = qna_PutFrame(qna, data, size);
				break;

			case EPP_RSTATUS:
			case EPP_XSTATUS:

#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("FIFO: %cSTATUS %06o %06o\n",
						(type == EPP_RSTATUS) ? 'R' : 'X',
						*((uint16 *)&data[0]), *((uint16 *)&data[2]));
#endif /* DEBUG */

				qna_PutStatus(qna, type, data);
				rc = size;
				break;

#ifdef DEBUG
			default:
				dbg_Printf("%s: Undefined EPP type %d at line %d\n",
					qna->Unit.devName, type, __LINE__);
#endif /* DEBUG */
		}

		if (rc <= 0)
			break;
		idxFifo += size + (WSIZE*2);
	}

	// Move down rest of queued data to beginning of FIFO area.
	if (idxFifo < qna->idxFifo) {
		memcpy(&qna->fifo[0], &qna->fifo[idxFifo], qna->idxFifo - idxFifo);
		qna->idxFifo -= idxFifo;
	} else
		qna->idxFifo = 0;

	return QNA_OK;
}


void qna_ProcessFrames(void *dptr)
{
	QNA_DEVICE *qna = (QNA_DEVICE *)dptr;

	if ((qna->idxFifo > 0) && ((qna->csr & CSR_RL) == 0))
		qna_FlushFIFO(qna);
	if (qna->csr & CSR_XL)
		return;
	if (qna_SendFrame(qna))
		return;
	ts10_SetTimer(&qna->txTimer);
}

// ***************************************************************

inline void qna_ResetEther(QNA_DEVICE *qna)
{
	int idx;

	qna->csr = (CSR_XL|CSR_RL);
	if (qna->World)
		qna->csr |= CSR_OK;
	if (qna->Flags & CFLG_LQA) {
		qna->var = LQA_MODE; // Mode Select Bit
	}
	qna->idxFifo = 0;

	// Clear all Ethernet addresses (Setup Packet)
	qna->nAddrs = 0;
	memset(&qna->ethAddr, 0, 14*8);

	// Initialize circular packet queue management
	memset(&qna->pktList[0], 0, sizeof(QNA_PACKET) * QNA_NPKTS);
	for (idx = 0; idx < QNA_NPKTS; idx++)
		qna->pktList[0].idPacket = idx;
	qna->pktHead = qna->pktTail = qna->pktCount = qna->pktLoss = 0;
}

inline void qna_UpdateCSR(QNA_DEVICE *qna, uint16 ncsr)
{
	if (qna->csr & CSR_SR) {
		qna->csr = (ncsr & (CSR_IE|CSR_SR)) |
			(qna->csr & ~(CSR_IE|CSR_SR));
	} else {
		if (ncsr & CSR_SR) {
			qna_ResetEther(qna);
			qna->csr |= CSR_SR;
			return;
		}
		// Update CSR bits now
		qna->csr =
			((ncsr & CSR_RW) | (qna->csr & ~CSR_RW)) & ~(ncsr & CSR_W1C);
	}
}

int qna_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	QNA_DEVICE *qna = (QNA_DEVICE *)dptr;
	uint32     reg  = (pAddr - (qna->csrAddr & 0x1FFF)) >> 1;

	switch (reg) {
		case nPROM0: case nPROM1: case nPROM2:
		case nPROM3: case nPROM4: case nPROM5:
			// If EL bit is set in CSR, get checksum bytes in first
			// two locations in reverse order (last 2 bytes).  Otherwise,
			// get Ethernet address in first 6 bytes.
			*data = PROM_MBO |
				qna->ownAddr[(qna->csr & CSR_EL) ? (7 - reg) : reg];
			break;

		case nVAR:
			*data = qna->var;
			break;

		case nCSR:
			*data = qna->csr;
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) Unknown Register %d - PA=%08o D=%06o (hex %04x)\n",
					qna->Unit.devName, reg, pAddr, *data, *data);
#endif /* DEBUG */
			return UQ_NXM;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS) && (reg < QNA_NREGS))
		dbg_Printf("%s: (R) %s (%08o) => %06o (hex %04X) (Size: %d bytes)\n",
			qna->Unit.devName, regNameR[reg], pAddr, *data, *data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

int qna_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	QNA_DEVICE *qna = (QNA_DEVICE *)dptr;
	MAP_IO     *io  = &qna->ioMap;
	uint32     reg  = (pAddr - (qna->csrAddr & 0x1FFF)) >> 1;

	switch (reg) {
		case nPROM0: case nPROM1:
			// Read Only Registers - Do nothing.
			break;

		case nRBADR0: // Receive BDL Address (Low)
			qna->rxBDLAddr = (qna->rxBDLAddr & 0x3F0000) | (data & ~1);
			break;

		case nRBADR1: // Receive BDL Address (High)
			qna->rxBDLAddr = (qna->rxBDLAddr & 0xFFFF) | ((data & 0x3F) << 16);
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS) && (qna->csr & CSR_RL))
				dbg_Printf("%s: New Receive BDL Address: %08o (%06X)\n",
					qna->Unit.devName, qna->rxBDLAddr, qna->rxBDLAddr);
#endif /* DEBUG */
			qna->csr &= ~CSR_RL; // BDL Valid Now

			// Now flush all queues into host memory.
			qna_FlushFIFO(qna);
			break;

		case nTBADR0: // Transmit BDL Address (Low)
			qna->txBDLAddr = (qna->txBDLAddr & 0x3F0000) | (data & ~1);
			break;

		case nTBADR1: // Transmit BDL Address (High)
			qna->txBDLAddr = (qna->txBDLAddr & 0xFFFF) | ((data & 0x3F) << 16);
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS) && (qna->csr & CSR_XL))
				dbg_Printf("%s: New Transmit BDL Address: %08o (%06X)\n",
					qna->Unit.devName, qna->txBDLAddr, qna->txBDLAddr);
#endif /* DEBUG */
			qna->csr &= ~CSR_XL; // BDL Valid Now

			ts10_SetTimer(&qna->txTimer);
			break;

		case nVAR:
			if (qna->Flags & CFLG_LQA) {
				uint16 mask =
					(qna->var & LQA_MODE) ? VAR_LQA_LRW : VAR_LQA_QRW;
				qna->var = (data & mask) | (qna->var & ~mask);
			} else
				qna->var = data & VAR_QNA_RW;

			// Set vector address for interrupts
			qna->intVector = data & VAR_IV;
			io->SetVector(io, qna->intVector, 0);
			break;

		case nCSR:
			if (acc == ACC_BYTE) // If access is byte, merge with CSR
				data = (pAddr & 1) ? ((data << 8)   | (qna->csr & 0377)) :
				                     ((data & 0377) | (qna->csr & ~0377));
			qna_UpdateCSR(qna, data);
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) Unknown Register %d - PA=%08o D=%06o (hex %04x)\n",
					qna->Unit.devName, reg, pAddr, data, data);
#endif /* DEBUG */
			return UQ_NXM;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS) && (reg < QNA_NREGS))
		dbg_Printf("%s: (W) %s (%08o) <= %06o (hex %04X) (size: %d bytes)\n",
			qna->Unit.devName, regNameW[reg], pAddr, data, data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

// ****************************************************************

// Create/Initialize QNA device.
// Usage: create <device> <qna|lqa|sqa> ...
void *qna_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	QNA_DEVICE *qna = NULL;
	CLK_QUEUE  *newTimer;
	MAP_IO     *io;

	if (qna = (QNA_DEVICE *)calloc(1, sizeof(QNA_DEVICE))) {
		// First, set up its description,
		qna->Unit.devName    = newMap->devName;
		qna->Unit.keyName    = newMap->keyName;
		qna->Unit.emuName    = newMap->emuName;
		qna->Unit.emuVersion = newMap->emuVersion;

		// Recongize which model - DEQNA, DELQA, or DELQA-PLUS.
		if (!strcmp(qna->Unit.keyName, LQA_KEY))
			qna->Flags = CFLG_LQA;
		else if (!strcmp(qna->Unit.keyName, LQAT_KEY))
			qna->Flags = CFLG_LQA|CFLG_TURBO;
		else if (!strcmp(qna->Unit.keyName, QNA_KEY))
			qna->Flags = CFLG_QNA;
		else {
			printf("%s: *** Bug Check: Unknown device name - %s\n",
				qna->Unit.devName, qna->Unit.keyName);
			free(qna);
			return NULL;
		}

#if 0
		// Initialize Communication/Doorbell Area
		dp_Create(&qna->dp, sizeof(DPC), 0, SIGUSR1, 1600, 0, SIGUSR1, 1600);
		if (qna->dp.dpc)
			dp_Start(&qna->dp.dpc);
#endif
		if (epp_OpenTUN(qna, "tap") == NULL) {
			free(qna);
			return NULL;
		}

		// Controller Flags
		qna->csrAddr    = QNA_IOADDR;

		qna->Device     = newMap->devParent->Device;
		qna->Callback   = newMap->devParent->Callback;
		qna->System     = newMap->devParent->sysDevice;

		// Set up I/O map settings
		io               = &qna->ioMap;
		io->devName      = qna->Unit.devName;
		io->keyName      = qna->Unit.keyName;
		io->emuName      = qna->Unit.emuName;
		io->emuVersion   = qna->Unit.emuVersion;
		io->Device       = qna;
		io->csrAddr      = qna->csrAddr;
		io->nRegs        = QNA_NREGS;
		io->nVectors     = QNA_NVECS;
		io->intIPL       = QNA_IPL;
		io->intVector[0] = 0;
		io->ReadIO       = qna_ReadIO;
		io->WriteIO      = qna_WriteIO;

		// Assign that registers to QBA's I/O space.
		qna->Callback->SetMap(qna->Device, io);

		// Set up receive/transmit delay timers
		newTimer           = &qna->txTimer;
		newTimer->Next     = NULL;
		newTimer->outTimer = QNA_DELAY;
		newTimer->nxtTimer = QNA_DELAY;
		newTimer->Device   = qna;
		newTimer->Execute  = qna_ProcessFrames;

		// Power-up Initialization
		qna_ResetEther(qna);

		// Finally, link it to its mapping device.
		newMap->Device   = qna;
		newMap->Callback = qna->Callback;
	}

	return qna;
}

int qna_Info(MAP_DEVICE *map, int argc, char **argv)
{
	QNA_DEVICE *qna = (QNA_DEVICE *)map->Device;
	int idx;

	printf("\nDevice:           %s  Type: %s\n",
		qna->Unit.devName, qna->Unit.keyName);
	printf("TUN/TAP Address:  %02X:%02X:%02X:%02X:%02X:%02X\n",
		qna->tunAddr[0], qna->tunAddr[1], qna->tunAddr[2],
		qna->tunAddr[3], qna->tunAddr[4], qna->tunAddr[5]);
	printf("PROM Address:     %02X:%02X:%02X:%02X:%02X:%02X  %02X:%02X\n",
		qna->ownAddr[0], qna->ownAddr[1], qna->ownAddr[2],
		qna->ownAddr[3], qna->ownAddr[4], qna->ownAddr[5],
		qna->ownAddr[6], qna->ownAddr[7]);

	printf("Ethernet Address: (%d Entries)\n", qna->nAddrs);
	for (idx = 0; idx < qna->nAddrs; idx++) {
		uint8 *ethAddr = qna->ethAddr[idx];
		printf("   %2d: %02X:%02X:%02X:%02X:%02X:%02X\n",
			ethAddr[0], ethAddr[1], ethAddr[2],
			ethAddr[3], ethAddr[4], ethAddr[5]);
	}

	return TS10_OK;
}

// DEQNA Ethernet Controller
DEVICE qna_Device =
{
	QNA_KEY,      // Device Type (Key) Name
	QNA_NAME,     // Emulator Name
	QNA_VERSION,  // Emulator Version

	NULL,         // Listing of devices
	DF_SYSMAP,    // Device Flags
	DT_NETWORK,   // Device Type

	NULL,         // Commands
	NULL,         // Set Commands
	NULL,         // Show Commands

	qna_Create,   // Create Routine
	NULL,         // Configure Routine
	NULL,         // Delete Routine
	NULL,         // Reset Routine
	NULL,         // Attach Routine
	NULL,         // Detach Routine
	qna_Info,     // Info Routine
	NULL,         // Boot Routine
	NULL,         // Execute Routine
#ifdef DEBUG
	NULL,         // Debug Routine
#endif /* DEBUG */
};

// DELQA Ethernet Controller
DEVICE lqa_Device =
{
	LQA_KEY,      // Device Type (Key) Name
	QNA_NAME,     // Emulator Name
	QNA_VERSION,  // Emulator Version

	NULL,         // Listing of devices
	DF_SYSMAP,    // Device Flags
	DT_NETWORK,   // Device Type

	NULL,         // Commands
	NULL,         // Set Commands
	NULL,         // Show Commands

	qna_Create,   // Create Routine
	NULL,         // Configure Routine
	NULL,         // Delete Routine
	NULL,         // Reset Routine
	NULL,         // Attach Routine
	NULL,         // Detach Routine
	qna_Info,     // Info Routine
	NULL,         // Boot Routine
	NULL,         // Execute Routine
#ifdef DEBUG
	NULL,         // Debug Routine
#endif /* DEBUG */
};

// DELQA-PLUS Ethernet Controller
DEVICE lqat_Device =
{
	LQAT_KEY,     // Device Type (Key) Name
	QNA_NAME,     // Emulator Name
	QNA_VERSION,  // Emulator Version

	NULL,         // Listing of devices
	DF_SYSMAP,    // Device Flags
	DT_NETWORK,   // Device Type

	NULL,         // Commands
	NULL,         // Set Commands
	NULL,         // Show Commands

	qna_Create,   // Create Routine
	NULL,         // Configure Routine
	NULL,         // Delete Routine
	NULL,         // Reset Routine
	NULL,         // Attach Routine
	NULL,         // Detach Routine
	qna_Info,     // Info Routine
	NULL,         // Boot Routine
	NULL,         // Execute Routine
#ifdef DEBUG
	NULL,         // Debug Routine
#endif /* DEBUG */
};
