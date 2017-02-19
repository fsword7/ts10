// rq.c - RQDX3 MSCP Disk Controller Emulator
//
// Copyright (c) 2001-2002, Timothy M. Stark
// Copyright (c) 2001-2002, Robert M. Supnik
// Derived from work by Stephen F. Shirron
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
//
// -------------------------------------------------------------------------
//
// Modification History:
//
// 04/15/03  TMS  Fixed ordering bug in packet queue process.
// 01/13/03  TMS  Renamed RW_LNT to RQ_DSK_LNT.
// 01/13/03  TMS  Modification history starts here.
//
// -------------------------------------------------------------------------

#include "emu/defs.h"
#include "dec/rq.h"

// ********************************************************************

// RQDX3 controller supports that disk drive types:
//
//  Type   sec  surf  cyl    tpg  gpc    RCT       LBNs
// 
//  RX50    10   1    80     5    16     -         800
//  RX33    15   2    80     2    1      -         2400
//  RD51    18   4    306    4    1      36*4      21600
//  RD31    17   4    615    4    1      3*8       41500
//  RD52    17   8    512    8    1      4*8       60480
//  RD53    17   7    1024   7    1      5*8       138672
//  RD54    18   15   1225   15   1      7*8       311200

//  RQDX3 emulator also supports following drives that existed on
//  other controllers like SDI controller.  XBN, DBN, RCTS, and RCTC
//  are not known yet but are not being used by this RQDX3 emulator.

//  RA72    51   20   1921?  20   1      ?*8       1953300
//  RA82    57   15   1435   15   1      ?*8       1216665
//  RA90    69   13   2656   13   1      ?*8       2376153
//  RA92    73   13   3101   13   1      ?*8       2940951

RQ_DTYPE rq_Drives[] =
{
	{
		"RX50", "400K Floppy Drive",
		7, 0x25658032, UQI_ESDI, UF_RMV,
	 	10,  1,    80,  5, 16,  0,   0,    800,  0, 0,   0
	},

	{
		"RX33", "1.2MB 5.25 Floppy Drive",
		10, 0x25658021, UQI_ESDI, UF_RMV,
	 	15,  2,    80,  2,  1,  0,   0,   2400,  0, 0,   0
	},

	{
		"RD51", "",
		6, 0x25644033, UQI_ESDI, 0, 
	 	18,  4,   306,  4,  1, 57,  87,  21600, 36, 4, 144
	},

	{
		"RD31", "",
		12, 0x2564401F, UQI_ESDI, 0,
		17,  4,   615,  4,  1, 54,  14,  41560,  3, 8, 100
	},

	{
		"RD52", "31MB ST506 Fixed Drive",
		8, 0x25644034, UQI_ESDI, 0,
		17,  8,   512,  8,  1, 54,  82,  60488,  4, 8, 168
	},

	{
		"RD53", "71MB ST506 Fixed Drive",
		9, 0x25644035, UQI_ESDI, 0,
		17,  8,  1024,  8,  1, 54,  82, 138672,  5, 8, 280
	},

	{
		"RD54", "159MB ST506 Fixed Drive",
		13, 0x25644036, UQI_ESDI, 0,
		17, 15, 1225, 15,  1, 54, 201, 311200,  7, 8, 609
	},

	{
		"RRD40", "CD-ROM Drive",
		26, 0x25652228, UQI_SCSI, (UF_RMV|UF_WPH),
		32,  1, 41625, 1, 1,  0, 0, 1332000, 0, 0, 0
	},

	{
		"RA72", "1GB SDI Fixed Disk",
		37, 0x25641048, UQI_SDI, 0,
		51, 20, 1921, 20, 1, 2040, 2040, 1953300, 400, 5, 38300
	},

	{
		"RA82", "623MB SDI Fixed Disk",
		11, 0x25641052, UQI_SDI, 0,
		57, 15, 1435, 15, 1, 3420, 3420, 1216665, 400, 8, 21345
	},

	{
		"RA90", "1.26GB SDI Fixed Disk",
		19, 0x2564105A, UQI_SDI, 0,
		69, 13, 2656, 13, 1, 1794, 1794, 2376153, 400, 6, 34437
	},

	{
		"RA92", "1.5GB SDI Fixed Disk",
		29, 0x2564105C, UQI_SDI, 0,
		73, 13, 3101, 13, 1, 174, 775, 2940951, 316, 3, 40287
	},
/*
	{
		"RX23", "1.44MB 3.5 Floppy Drive",
		x, x, UQI_UNKNOWN, UF_RMV,
		18, 2, 80, 2, 1, 0, 0, 2880, 0, 0, 0
	},

	{
		"RX26", "2.88MB 3.5 Floppy Drive",
		x, x, UQI_UNKNOWN, UF_RMV,
		36, 2, 80, 2, 1, 0, 0, 5760, 0, 0, 0
	},
*/
	{
		"RZ24", "200MB SCSI Fixed Disk",
		50, 0x22E5A018, UQI_SCSI, 0,
		38, 8, 1348, 8, 1, 0, 0, 409792, 0, 0, 0
	},

	{
		"RZ25", "425MB SCSI Fixed Disk",
		59, 0x22E5A019, UQI_SCSI, 0,
		62, 9, 1476, 9, 1, 0, 0, 832608, 0, 0, 0
	},

	{
		"RZ56", "635MB SCSI Fixed Disk",
		60, 0x22E5A038, UQI_SCSI, 0,
		54, 15, 1604, 15, 1, 0, 0, 1299174, 0, 0, 0
	},

	{
		"RZ57", "1GB SCSI Fixed Disk",
		61, 0x22E5A039, UQI_SCSI, 0,
		71, 15, 1835, 15, 1, 0, 0, 1954050, 0, 0, 0
	},

	{ NULL } // Terminator
};

// ********************************************************************

// Media ID Format
//
// |<---- Port ------->|<---------------- Drive ------------------>|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   P1    |    P2   |    D1   |    D2   |    D3   |    Number   |
// +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0

char *mscp_GetMediaID(uint32 idMedia)
{
	static char ascMedia[11];
	char *str = ascMedia;
	char ch;
	int  idx;

	for (idx = 4; idx >= 0; idx--) {
		ch = (idMedia >> ((idx * 5) + 7)) & 037;
		*str++ = (ch ? (ch + '@') : ' ');
	}
	str = ToRadix(str, idMedia & 0x7F, 10, 2);
	*str = '\0';

	return ascMedia;
}

#ifdef DEBUG

void rq_DumpPacket(RQ_DEVICE *rq, RQ_PACKET *pkt, cchar *name)
{
	char outBuffer[80], *str;
	int  idx1, idx2;

	dbg_Printf("%s: %s Packet #%d\n", rq->keyName, name, pkt->idPacket);
	dbg_Printf("%s:\n", rq->keyName);
	
	for (idx1 = 0; idx1 < RQ_PKT_WSIZE;) {
		str = ToRadix(outBuffer, idx1, 16, 4);
		*str++ = ' ';
		for (idx2 = 0; (idx1 < RQ_PKT_WSIZE) && (idx2 < 8); idx2++) {
			*str++ = ' ';
			str = ToRadix(str, (uint32)pkt->Data[idx1++], 16, 4);
		}
		*str++ = '\0';
		dbg_Printf("%s: %s\n", rq->keyName, outBuffer);
	};
}

void rq_DumpInitInfo(RQ_DEVICE *rq)
{
	dbg_Printf("%s: Initialization Complete\n", rq->keyName);
	dbg_Printf("%s:\n", rq->keyName);
	dbg_Printf("%s: Step   Controller       Host\n", rq->keyName);
	dbg_Printf("%s: ----   -------------    -------------\n", rq->keyName);
	dbg_Printf("%s: #1     %06o (%04X)    %06o (%04X)\n",
		rq->keyName, S1C, S1C, S1H, S1H);
	dbg_Printf("%s: #2     %06o (%04X)    %06o (%04X)\n",
		rq->keyName, S2C, S2C, S2H, S2H);
	dbg_Printf("%s: #3     %06o (%04X)    %06o (%04X)\n",
		rq->keyName, S3C, S3C, S3H, S3H);
	dbg_Printf("%s: #4     %06o (%04X)    %06o (%04X)\n",
	 	rq->keyName, S4C, S4C, S4H, S4H);
	dbg_Printf("%s:\n", rq->keyName);

	dbg_Printf("%s: Ringbase Address: %08o (hex %06X)\n",
		rq->keyName, rq->rbAddr, rq->rbAddr);
	dbg_Printf("%s: Descriptors:      %d Command Slots, %d Response Slots\n",
		rq->keyName, rq->cmdRing.szDesc, rq->resRing.szDesc);
	dbg_Printf("%s: Interrupt Vector: %03o (hex %03X), %s\n",
		rq->keyName, rq->intVector, rq->intVector,
		((S1H & SA_S1H_IE) ? "Enabled" : "Disabled"));
}

#endif /* DEBUG */

// ********************************************************************

// Fatal error - Controller now is dead.
int rq_SetFatalError(RQ_DEVICE *rq, int error)
{
	rq->State = CST_DEAD;
	rq->errCode = error;
	SA = SA_ER | error;

	return UQ_ERROR;
}

// Find a free packet in internal queue.
RQ_PACKET *rq_GetFreePacket(RQ_DEVICE *rq)
{
	RQ_PACKET *newPacket = NULL;

	if (rq->pktFree) {
		newPacket   = rq->pktFree;
		rq->pktFree = newPacket->Next;
		rq->pktBusy++;
	}

	return newPacket;
}

void rq_Enqueue(RQ_PACKET **list, RQ_PACKET *pkt, int which)
{
	RQ_PACKET *ptr;

	if (pkt) {
		if (which == RQ_QHEAD) {
			// Enqueue a head of queue
			pkt->Next = *list;
			*list     = pkt;
		} else {
			// Enqueue a tail of queue
			pkt->Next = NULL;
			if (*list == NULL)
				*list = pkt;
			else {
				for (ptr = *list; ptr->Next; ptr = ptr->Next);
				ptr->Next = pkt;
			}
		}
	}
}

RQ_PACKET *rq_Dequeue(RQ_PACKET **list, int which)
{
	RQ_PACKET *pkt = NULL;

	if (*list) {
		pkt   = *list;
		*list = pkt->Next;
	}

	return pkt;
}

// Get a descriptor from host
int rq_GetDesc(RQ_DEVICE *rq, UQ_RING *ring, uint32 *desc)
{
	UQ_CALL *call = rq->Callback;
	uint32  addr  = ring->baseAddr + (ring->idxDesc << 2);

	if (call->ReadBlock(rq->System, addr, (uint8 *)desc, 4, 0))
		return rq_SetFatalError(rq, ER_QRE);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: (R) %s %d (%08o, hex %06X) => %08X\n",
			rq->keyName, ring->Name, ring->idxDesc, addr, addr, *desc);
#endif /* DEBUG */

	return UQ_OK;
}

// Send a descriptor to host and send an interrupt if request
int rq_PutDesc(RQ_DEVICE *rq, UQ_RING *ring, uint32 desc)
{
	MAP_IO  *io     = &rq->ioMap;
	UQ_CALL *call   = rq->Callback;
	uint32  addr    = ring->baseAddr + (ring->idxDesc << 2);
	uint32  newDesc = (desc & ~UQ_DESC_OWN) | UQ_DESC_DONE;
	uint16  flag    = 1;

	if (call->WriteBlock(rq->System, addr, (uint8 *)&newDesc, 4, 0))
		return rq_SetFatalError(rq, ER_QWE);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: (W) %s %d (%08o, hex %06X) <= %08X\n",
			rq->keyName, ring->Name, ring->idxDesc, addr, addr, newDesc);
#endif /* DEBUG */

	if (desc & UQ_DESC_INT) {
		if (ring->szDesc > 1) {
			uint32 prvAddr, prvDesc;

			prvAddr = ring->baseAddr +
				(((ring->idxDesc - 1) & ring->mskDesc) << 2);
			if (call->ReadBlock(rq->System, prvAddr, (uint8 *)&prvDesc, 4, 0))
				return rq_SetFatalError(rq, ER_QRE);
			if (prvDesc & UQ_DESC_OWN) {
				call->WriteBlock(rq->System, ring->intAddr, (uint8 *)&flag, 2, 0);
				RQ_DOINT;
			}
		} else {
			call->WriteBlock(rq->System, ring->intAddr, (uint8 *)&flag, 2, 0);
			RQ_DOINT;
		}
	}

	// Increment Descriptor Index
	ring->idxDesc = (ring->idxDesc + 1) & ring->mskDesc;

	return UQ_OK;
}

// Get a message packet from host
int rq_GetPacket(RQ_DEVICE *rq, RQ_PACKET **pkt)
{
	UQ_CALL *call = rq->Callback;
	uint32  desc;    // Current descriptor
	uint32  pktAddr; // Packet Address

	// Get a command descriptor from host
	if (rq_GetDesc(rq, &rq->cmdRing, &desc))
		return UQ_ERROR;

	// Check if descriptor belongs to us.
	if (desc & UQ_DESC_OWN) {
		if (*pkt = rq_GetFreePacket(rq)) {
			pktAddr = (desc & UQ_DESC_ADDR) + UQ_HDR_OFF;

			// Disable Host Timer.
			rq->hstTimer = 0;

#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: (R) Command Packet #%d (%08o, hex %06X)\n",
					rq->keyName, (*pkt)->idPacket, pktAddr, pktAddr);
#endif /* DEBUG */

			// Get a command packet from host
			if (call->ReadBlock(rq->System, pktAddr, (uint8 *)&(*pkt)->Data, RQ_PKT_BSIZE, 0))
				return rq_SetFatalError(rq, ER_PRE);

#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				rq_DumpPacket(rq, *pkt, "Command");
#endif /* DEBUG */

			// Now release a command descriptor to host.
			rq_PutDesc(rq, &rq->cmdRing, desc);
		} else {
			// ** Fatal Error - No such free packets in internal queue.
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: *** Fatal Error - No such free packets\n",
					rq->keyName);
#endif /* DEBUG */
			return rq_SetFatalError(rq, ER_NSR);
		}
	} else
		*pkt = NULL;

	return UQ_OK;
}

// Put a message packet into host
int rq_PutPacket(RQ_DEVICE *rq, RQ_PACKET *pkt, int wq)
{
	UQ_CALL *call = rq->Callback;
	uint32  desc, cr;
	uint32  pktAddr, pktLen;

	if (pkt == NULL)
		return UQ_OK;

	// Get a command descriptor from host
	if (rq_GetDesc(rq, &rq->resRing, &desc))
		return UQ_ERROR;

	if (desc & UQ_DESC_OWN) {
		pktAddr = (desc & UQ_DESC_ADDR) + UQ_HDR_OFF;
		pktLen  = pkt->Data[UQ_LNT] - UQ_HDR_OFF;

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: (R) Response Packet #%d (%08o, hex %06X)\n",
				rq->keyName, pkt->idPacket, pktAddr, pktAddr);
#endif /* DEBUG */

		// Issue credits if any
		if ((UQ_GETP(pkt, UQ_CTC, TYP) == UQ_TYP_SEQ) &&
		    (UQ_GETP(pkt, CMD_OPC, OPC) & OP_END)) {
			cr = (rq->Credits > 14) ? 14 : rq->Credits;
			rq->Credits -= cr;
			pkt->Data[UQ_CTC] |= (++cr << UQ_CTC_P_CR);

#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: %d Credit%s Issued - Remaining %d Credits.\n",
					rq->keyName, cr, (cr == 1 ? "s" : ""), rq->Credits);
#endif /* DEBUG */
		}

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			rq_DumpPacket(rq, pkt, "Response");
#endif /* DEBUG */

		// Now send a packet to host.
		if (call->WriteBlock(rq->System, pktAddr, (uint8 *)&pkt->Data, pktLen, 0))
			return rq_SetFatalError(rq, ER_PWE);

		// Release a old packet to free list.
		// Enable host timer if idle (no busy packets).
		rq_Enqueue(&rq->pktFree, pkt, RQ_QHEAD);
		if (--rq->pktBusy == 0)
			rq->hstTimer = rq->hstTimeout;

		// Release a response descriptor to host and return.
		return rq_PutDesc(rq, &rq->resRing, desc);
	} else {
		rq_Enqueue(&rq->pktResp, pkt, wq);
		ts10_SetTimer(&rq->queTimer);

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: Packet Enqueued.\n", rq->keyName);
#endif /* DEBUG */
	}

	return UQ_OK;
}

RQ_DRIVE *rq_GetUnit(RQ_DRIVE *drv, uint32 lun)
{
	if ((lun < RQ_NDRVS) && (drv[lun].Flags & DFL_EXIST))
		return &drv[lun];
	return NULL;
}

void rq_SetUnitFlags(RQ_DRIVE *drv, RQ_PACKET *pkt)
{
	// Update unit flags.
	drv->uFlags = (drv->uFlags & (UF_RPL|UF_WPH|UF_RMV)) |
		(pkt->Data[ONL_UFLG] & UF_MASK);

	// Update software write lock enable.
	if ((pkt->Data[CMD_MOD] & MD_SWP) &&
		 (pkt->Data[ONL_UFLG] & UF_WPS))
		drv->uFlags |= UF_WPS;
}

void rq_SetResults(RQ_PACKET *pkt, uint32 cmd, uint32 flg, uint32 sts, uint32 lnt, uint32 typ)
{
	pkt->Data[UQ_LNT]  = lnt;
	pkt->Data[UQ_CTC]  = (typ << UQ_CTC_P_TYP);
	pkt->Data[RSP_OPF] = (cmd << RSP_OPF_P_OPC) | (flg << RSP_OPF_P_FLG);
	pkt->Data[RSP_STS] = sts;
}

int rq_SendLastFail(RQ_DEVICE *rq, int errCode)
{
	RQ_PACKET *pkt;

	if (pkt = rq_GetFreePacket(rq)) {
		pkt->Data[ELP_REFL] = 0;
		pkt->Data[ELP_REFH] = 0;
		pkt->Data[ELP_UNIT] = 0;
		pkt->Data[ELP_SEQ]  = 0;

		pkt->Data[PLF_CIDA] = 0;
		pkt->Data[PLF_CIDB] = 0;
		pkt->Data[PLF_CIDC] = 0;
		pkt->Data[PLF_CIDD] = (RQ_CLASS << PLF_CIDD_P_CLS) |
			(RQ_MODEL << PLF_CIDD_P_MOD);
		pkt->Data[PLF_VER]  = (RQ_SVER << PLF_VER_P_SVER) |
			(RQ_HVER << PLF_VER_P_HVER);
		pkt->Data[PLF_ERR]  = errCode;

		rq_SetResults(pkt, FM_CNT, LF_SNR, ST_CNT, PLF_LNT, UQ_TYP_DATA);
		pkt->Data[UQ_CTC] |= (UQ_CID_DIAG << UQ_CTC_P_CID);

		// Ok, Send a last fail packet to host.
		return rq_PutPacket(rq, pkt, RQ_QTAIL);
	} else
		return UQ_ERROR;
}

// Data Transfer Error Log Packet
int rq_SendDataError(RQ_DRIVE *drv, RQ_PACKET *pkt, uint32 errCode)
{
	RQ_DEVICE *rq = drv->rqDevice;
	RQ_PACKET *epkt; // Error packet
	uint32    lun, lbn;
	uint32    cyl, trk, sec; 

	if (rq->cFlags & CF_THS)
		return RQ_OK;
	if ((epkt = rq_GetFreePacket(rq)) == NULL)
		return RQ_ERROR;
	lun = pkt->Data[CMD_UNIT];
	lbn = UQ_GETP32(pkt, RW_WLBNL);

	epkt->Data[ELP_REFL] = pkt->Data[CMD_REFL];
	epkt->Data[ELP_REFH] = pkt->Data[CMD_REFH];
	epkt->Data[ELP_UNIT] = lun;
	epkt->Data[ELP_SEQ]  = 0;
	epkt->Data[DTE_CIDA] = 0;
	epkt->Data[DTE_CIDB] = 0;
	epkt->Data[DTE_CIDC] = 0;
	epkt->Data[DTE_CIDD] = (RQ_CLASS << DTE_CIDD_P_CLS) |
		(RQ_MODEL << DTE_CIDD_P_MOD);
	epkt->Data[DTE_CVER]  = (RQ_SVER << DTE_CVER_P_SVER) |
		(RQ_HVER << DTE_CVER_P_HVER);
	epkt->Data[DTE_MLUN] = lun;
	epkt->Data[DTE_UIDA] = lun;
	epkt->Data[DTE_UIDB] = 0;
	epkt->Data[DTE_UIDC] = 0;
	epkt->Data[DTE_UIDD] = (CLS_DISK << DTE_UIDD_P_CLS) |
		(drv->dtInfo->idModel << DTE_UIDD_P_MOD);
	epkt->Data[DTE_UVER] = 0;
	epkt->Data[DTE_SCYL] = cyl;
	epkt->Data[DTE_VSNL] = 01000 + lun;
	epkt->Data[DTE_VSNH] = 0;
	epkt->Data[DTE_D1]   = 0;
	epkt->Data[DTE_D2]   = sec << DTE_D2_P_SECT;
	epkt->Data[DTE_D3]   = (cyl << DTE_D3_P_CYL) |
		(trk << DTE_D3_P_SURF);

	rq_SetResults(epkt, FM_SDE, LF_SNR, errCode, DTE_LNT, UQ_TYP_DATA);
	return rq_PutPacket(rq, epkt, RQ_QTAIL);
}

// Host Bus Error Log Packet
int rq_SendHostError(RQ_DEVICE *rq, RQ_PACKET *pkt)
{
	RQ_PACKET *epkt; // Error packet

	if (rq->cFlags & CF_THS)
		return RQ_OK;
	if ((epkt = rq_GetFreePacket(rq)) == NULL)
		return RQ_ERROR;

	epkt->Data[ELP_REFL] = pkt->Data[CMD_REFL];
	epkt->Data[ELP_REFH] = pkt->Data[CMD_REFH];
	epkt->Data[ELP_UNIT] = pkt->Data[CMD_UNIT];
	epkt->Data[ELP_SEQ]  = 0;
	epkt->Data[HBE_CIDA] = 0;
	epkt->Data[HBE_CIDB] = 0;
	epkt->Data[HBE_CIDC] = 0;
	epkt->Data[HBE_CIDD] = (RQ_CLASS << DTE_CIDD_P_CLS) |
		(RQ_MODEL << DTE_CIDD_P_MOD);
	epkt->Data[HBE_CVER] = (RQ_SVER << DTE_CVER_P_SVER) |
		(RQ_HVER << DTE_CVER_P_HVER);
	epkt->Data[HBE_RSV]  = 0;
	epkt->Data[HBE_BADL] = pkt->Data[RW_WBAL];
	epkt->Data[HBE_BADH] = pkt->Data[RW_WBAH];

	rq_SetResults(epkt, FM_BAD, LF_SNR, ST_HST|SB_HST_NXM, HBE_LNT, UQ_TYP_DATA);
	return rq_PutPacket(rq, epkt, RQ_QTAIL);
}

// Attention Message - Unit Now Available
int rq_MscpUnitNowAvailable(RQ_DEVICE *rq, RQ_DRIVE *drv)
{
	RQ_PACKET *pkt;

	if (pkt = rq_GetFreePacket(rq)) {
		pkt->Data[RSP_REFL] = 0;
		pkt->Data[RSP_REFL] = 0;
		pkt->Data[RSP_UNIT] = drv->idUnit;
		pkt->Data[RSP_RSV]  = 0;

		pkt->Data[UNA_MLUN] = drv->idUnit;
		pkt->Data[UNA_UFLG] = drv->uFlags;
		pkt->Data[UNA_RSVL] = 0;
		pkt->Data[UNA_RSVH] = 0;
		pkt->Data[UNA_UIDA] = drv->idUnit;
		pkt->Data[UNA_UIDB] = 0;
		pkt->Data[UNA_UIDC] = 0;
		pkt->Data[UNA_UIDD] = (CLS_DISK << UNA_UIDD_P_CLS) |
			(drv->dtInfo->idModel << UNA_UIDD_P_MOD);

		// Ok, Send an attention packet to host.
		rq_SetResults(pkt, OP_AVA, 0, 0, UNA_LNT, UQ_TYP_SEQ);
		return rq_PutPacket(rq, pkt, RQ_QTAIL);
	} else
		return UQ_ERROR;
}

// 01 - Abort Command
int rq_MscpAbort(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 q)
{
	RQ_DRIVE  *drv;
	RQ_PACKET *tpkt, *prv;
	uint32    lun = pkt->Data[CMD_UNIT];
	uint32    cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32    ref = UQ_GETP32(pkt, ABO_REFL);

	if (drv = rq_GetUnit(rq->Drives, lun)) {
		// Find a packet in progress or pending on that unit.
		tpkt = NULL;
		if (drv->pktWork && (UQ_GETP32(drv->pktWork, CMD_REFL) == ref)) {
			ts10_CancelTimer(&drv->xfrTimer);
			tpkt = drv->pktWork;
			drv->pktWork = NULL;
		} else if (drv->pktQueue &&
		           (UQ_GETP32(drv->pktQueue, CMD_REFL) == ref)) {
			tpkt = drv->pktQueue;
			drv->pktQueue = tpkt->Next;
		} else if (prv = drv->pktQueue) {
			while (tpkt = prv->Next) {
				if (UQ_GETP32(tpkt, CMD_REFL) == ref) {
					prv->Next = tpkt->Next;
					break;
				}
			}
		}

		// Now abort that command packet that is in progress or pending.
		if (tpkt) {
			uint32 tcmd = UQ_GETP(tpkt, CMD_OPC, OPC);
			rq_SetResults(tpkt, tcmd | OP_END, 0, ST_ABO, RSP_LNT, UQ_TYP_SEQ);
			if (rq_PutPacket(rq, tpkt, RQ_QTAIL))
				return UQ_ERROR;
		}
	}

	// Ok, send a response packet back to host.
	rq_SetResults(pkt, cmd | OP_END, 0, ST_SUC, ABO_LNT, UQ_TYP_SEQ);
	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

// 02 - Get Command Status Command
int rq_MscpGetCommandStatus(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 q)
{
	RQ_DRIVE  *drv;
	RQ_PACKET *tpkt;
	uint32    lun = pkt->Data[CMD_UNIT];
	uint32    cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32    ref = UQ_GETP32(pkt, GCS_REFL);

	if ((drv = rq_GetUnit(rq->Drives, lun)) &&
	    (tpkt = drv->pktWork) &&
		 (UQ_GETP32(tpkt, CMD_REFL) == ref) &&
		 (UQ_GETP(tpkt, CMD_OPC, OPC) >= OP_ACC)) {
		pkt->Data[GCS_STSL] = tpkt->Data[RW_WBCL];
		pkt->Data[GCS_STSH] = tpkt->Data[RW_WBCH];
	} else {
		pkt->Data[GCS_STSL] = 0;
		pkt->Data[GCS_STSH] = 0;
	}

	// Ok, Send a command status packet back to host.
	rq_SetResults(pkt, cmd | OP_END, 0, ST_SUC, GCS_LNT, UQ_TYP_SEQ);
	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

// 03 - Get Unit Status Command
int rq_MscpGetUnitStatus(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 wq)
{
	RQ_DRIVE *drv;
	RQ_DTYPE *dtInfo;
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts, rb;

	// Check unit # as next unit or not.
	// If not, wrap back to unit #0.
	if (pkt->Data[CMD_MOD] & MD_NXU) {
		if (lun >= rq->nDrives) {
			lun = 0;
			pkt->Data[RSP_UNIT] = lun;
		}
	}

	if (drv = rq_GetUnit(rq->Drives, lun)) {
		dtInfo = drv->dtInfo;

		// Determine unit attributes and put results in status.
		if ((drv->Flags & DFL_ATTACHED) == 0)
			sts = ST_OFL | SB_OFL_NV; // No volume
		else if (drv->Flags & DFL_ONLINE)
			sts = ST_SUC; // Online - Success
		else
			sts = ST_AVL; // Available - Success
		rb = (dtInfo->rcts) ? 1 : 0;

		// Set up a new response packet.
		pkt->Data[GUS_MLUN] = lun;
		pkt->Data[GUS_UFLG] = drv->uFlags;
		pkt->Data[GUS_RSVL] = 0;
		pkt->Data[GUS_RSVH] = 0;
		pkt->Data[GUS_UIDA] = lun;
		pkt->Data[GUS_UIDB] = 0;
		pkt->Data[GUS_UIDC] = 0;
		pkt->Data[GUS_UIDD] = (CLS_DISK << GUS_UIDD_P_CLS) |
			(dtInfo->idModel << GUS_UIDD_P_MOD);
		UQ_PUTP32(pkt, GUS_MEDL, drv->dtInfo->idMedia);
		pkt->Data[GUS_TRK]  = dtInfo->sec;
		pkt->Data[GUS_GRP]  = dtInfo->tpg;
		pkt->Data[GUS_CYL]  = dtInfo->gpc;
		pkt->Data[GUS_UVER] = 0;
		pkt->Data[GUS_RCTS] = dtInfo->rcts;
		pkt->Data[GUS_RBSC] = (rb << GUS_RB_P_RBNS) | (rb << GUS_RB_P_RCTC);
	} else
		sts = ST_OFL; // Offline

	pkt->Data[GUS_SHUN] = lun;
	pkt->Data[GUS_SHST] = 0;
	rq_SetResults(pkt, cmd | OP_END, 0, sts, GUS_LNT, UQ_TYP_SEQ);

	// Ok, Send a response packet back to host.
	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

// 04 - Set Controller Characteristics Command
int rq_MscpSetCtlrChar(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 wq)
{
	uint32 sts, cmd, tmo;

	if (pkt->Data[SCC_MSV]) {
		sts = ST_CMD | I_VRSN;
		cmd = 0;
	} else {
		// Set up controller flags and host timeout.
		rq->cFlags = (rq->cFlags & CF_RPL) | pkt->Data[SCC_CFLG];
		if (tmo = pkt->Data[SCC_TMO])
			rq->hstTimeout = tmo + 2;

		// Set up a new response packet.
		pkt->Data[SCC_CFLG] = rq->cFlags;
		pkt->Data[SCC_TMO]  = RQ_DCTMO;
		pkt->Data[SCC_VER]  = (RQ_HVER << SCC_VER_P_HVER) |
			(RQ_SVER << SCC_VER_P_SVER);
		pkt->Data[SCC_CIDA] = 0;
		pkt->Data[SCC_CIDB] = 0;
		pkt->Data[SCC_CIDC] = 0;
		pkt->Data[SCC_CIDD] = (RQ_CLASS << SCC_CIDD_P_CLS) |
			(RQ_MODEL << SCC_CIDD_P_MOD);

		// Successful operation.
		sts = ST_SUC;
		cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	}

	// Set up results.
	rq_SetResults(pkt, cmd | OP_END, 0, sts, SCC_LNT, UQ_TYP_SEQ);

	// Ok, Send a new response packet back to host.
	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

// 08 - Available Command
int rq_MscpAvailable(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];        // Logical Unit
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC); // MSCP Opcode
	uint32   sts;
	RQ_DRIVE *drv;

	if (drv = rq_GetUnit(rq->Drives, lun)) {
		// Check if that drive still is in use.
		// If so, enter that packet into its queue to
		// process later.
		if (q && drv->pktWork) {
			rq_Enqueue(&drv->pktQueue, pkt, RQ_QTAIL);
			return UQ_OK;
		}

		// Set this unit available to other ports
		drv->Flags  &= ~DFL_ONLINE;
		drv->uFlags &= (UF_RPL|UF_WPH|UF_RMV);

		sts = ST_SUC; // Successful
	} else
		sts = ST_OFL; // Offline

	rq_SetResults(pkt, cmd | OP_END, 0, sts, AVL_LNT, UQ_TYP_SEQ);

	// Ok, Send a response packet back to host.
	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

// 09 - Online Command
int rq_MscpOnline(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];        // Logical Unit
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC); // MSCP Opcode
	uint32   sts;
	RQ_DRIVE *drv;

	if (drv = rq_GetUnit(rq->Drives, lun)) {
		// Check if that drive still is in use.
		// If so, enter that packet into its queue to
		// process later.
		if (q && drv->pktWork) {
			rq_Enqueue(&drv->pktQueue, pkt, RQ_QTAIL);
			return UQ_OK;
		}

		if ((drv->Flags & DFL_ATTACHED) == 0)
			sts = ST_OFL | SB_OFL_NV;
		else if (drv->Flags & DFL_ONLINE)
			sts = ST_SUC | SB_SUC_ON;
		else {
			sts = ST_SUC;
			drv->Flags |= DFL_ONLINE;
			rq_SetUnitFlags(drv, pkt);
		}

		// Set up a new response packet.
		pkt->Data[ONL_MLUN] = lun;
		pkt->Data[ONL_UFLG] = drv->uFlags;
		pkt->Data[ONL_RSVL] = 0;
		pkt->Data[ONL_RSVH] = 0;
		pkt->Data[ONL_UIDA] = lun;
		pkt->Data[ONL_UIDB] = 0;
		pkt->Data[ONL_UIDC] = 0;
		pkt->Data[ONL_UIDD] = (CLS_DISK << ONL_UIDD_P_CLS) |
			(drv->dtInfo->idModel << ONL_UIDD_P_MOD);
		pkt->Data[ONL_VSNL] = 01000 + lun;
		pkt->Data[ONL_VSNH] = 0;
		UQ_PUTP32(pkt, ONL_MEDL, drv->dtInfo->idMedia);
		UQ_PUTP32(pkt, ONL_SIZL, drv->dtInfo->lbn);
	} else
		sts = ST_OFL; // Offline

	pkt->Data[ONL_SHUN] = lun;
	pkt->Data[ONL_SHST] = 0;
	rq_SetResults(pkt, cmd | OP_END, 0, sts, ONL_LNT, UQ_TYP_SEQ);

	// Ok, Send a response packet back to host.
	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

// 0A - Set Unit Characteristics
int rq_MscpSetUnitChar(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 q)
{
	RQ_DRIVE *drv;
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts;

	if (drv = rq_GetUnit(rq->Drives, lun)) {
		// If that packet is processing in progress,
		// enqueue this packet into unit's queue.
		if (q && drv->pktWork) {
			rq_Enqueue(&drv->pktQueue, pkt, RQ_QTAIL);
			return UQ_OK;
		}

		if ((drv->Flags & DFL_ATTACHED) == 0)
			sts = ST_OFL | SB_OFL_NV;
		else {
			sts = ST_SUC;
			rq_SetUnitFlags(drv, pkt);
		}

		pkt->Data[SUC_MLUN] = lun;
		pkt->Data[SUC_UFLG] = drv->uFlags;
		pkt->Data[SUC_RSVL] = 0;
		pkt->Data[SUC_RSVH] = 0;
		pkt->Data[SUC_UIDA] = lun;
		pkt->Data[SUC_UIDB] = 0;
		pkt->Data[SUC_UIDC] = 0;
		pkt->Data[SUC_UIDD] = (CLS_DISK << SUC_UIDD_P_CLS) |
			(drv->dtInfo->idModel << SUC_UIDD_P_MOD);
		UQ_PUTP32(pkt, SUC_MEDL, drv->dtInfo->idMedia);
		UQ_PUTP32(pkt, SUC_SIZL, drv->dtInfo->lbn);
		pkt->Data[SUC_VSNL] = 01000 + lun;
		pkt->Data[SUC_VSNH] = 0;
	} else
		sts = ST_OFL; // Offline

	// Set up shadowing data.
	pkt->Data[SUC_SHUN] = lun;
	pkt->Data[SUC_SHST] = 0;

	// Ok, send a response packet back to host.
	rq_SetResults(pkt, cmd | OP_END, 0, sts, SUC_LNT, UQ_TYP_SEQ);
	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

// *******************************************************************

// Validity Checks
int rq_CheckValid(RQ_DRIVE *drv, RQ_PACKET *pkt, uint32 cmd)
{
	uint32 lbnUser  = drv->dtInfo->lbn;
	uint32 lbnBlock = UQ_GETP32(pkt, RW_LBNL);
	uint32 cntBytes = UQ_GETP32(pkt, RW_BCL);
	uint32 cntBlocks;

	// Check unit for existant and attached.
	// if not, tell host that unit is available or offline.
	if ((drv->Flags & DFL_ATTACHED) == 0)
		return (ST_OFL | SB_OFL_NV);
	if ((drv->Flags & DFL_ONLINE) == 0)
		return ST_AVL;

	// Check address and byte count and make sure they are not odd.
	// if so, tell host that either is odd.
	if ((cmd != OP_ACC) && (cmd != OP_ERS) && (pkt->Data[RW_BAL] & 1))
		return (ST_HST | SB_HST_OA);
	if (cntBytes & 1)
		return (ST_HST | SB_HST_OC);

	// Check 'reasonable' byte count and LBN number.
	// If so, tell host that either is invalid.
	if (cntBytes & 0xF0000000)
		return (ST_CMD | I_BCNT);
	if (lbnBlock & 0xF0000000)
		return (ST_CMD | I_LBN);
	
	// Check limitation for logical block number and byte count.
	// If so, tell host that LBN is exceeding hardware limitation.
	if (lbnBlock >= lbnUser) {
		if (lbnBlock > (drv->dtInfo->rcts + lbnUser))
			return (ST_CMD | I_LBN);
		if (cntBytes != RQ_BLKSZ)
			return (ST_CMD | I_BCNT);
	} else {
		cntBlocks = (cntBytes + (RQ_BLKSZ - 1)) / RQ_BLKSZ;
		if ((lbnBlock + cntBlocks) > lbnUser)
			return (ST_CMD | I_BCNT);
	}

	// Check write protection aganist write accesses
	// If so, tell host that write access is denied.
	if ((cmd == OP_WR) || (cmd == OP_ERS)) {
		if (lbnBlock > lbnUser)
			return (ST_CMD | I_LBN);
		if (drv->uFlags & UF_WPS)
			return (ST_WPR | SB_WPR_SW);
		if (drv->uFlags & UF_WPH)
			return (ST_WPR | SB_WPR_HW);
	}

	// Won! Success! All data passed huge tests.
	return 0;
}

// Read/Write Data Done
int rq_DataDone(RQ_DRIVE *drv, uint32 flg, uint32 sts)
{
	RQ_DEVICE *rq  = drv->rqDevice;
	RQ_PACKET *pkt = drv->pktWork;
	uint32    cmd  = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32    bc   = UQ_GETP32(pkt, RW_BCL);
	uint32    wbc  = UQ_GETP32(pkt, RW_WBCL);

	drv->pktWork = NULL; // Done
	UQ_PUTP32(pkt, RW_BCL, bc - wbc); // Bytes Processed

	// Clear all working data
	pkt->Data[RW_WBAL]  = 0;
	pkt->Data[RW_WBAH]  = 0;
	pkt->Data[RW_WBCL]  = 0;
	pkt->Data[RW_WBCH]  = 0;
	pkt->Data[RW_WLBNL] = 0;
	pkt->Data[RW_WLBNH] = 0;

	// Send a response packet back to host.
	rq_SetResults(pkt, cmd | OP_END, flg, sts, RW_DSK_LNT, UQ_TYP_SEQ);
	if (rq_PutPacket(rq, pkt, RQ_QTAIL))
		return UQ_ERROR;

	// Any next packet?
	if (drv->pktQueue)
		ts10_SetTimer(&rq->queTimer);
}

// Read/Write Data Processing during queue timer
void rq_ProcessData(void *dptr)
{
	RQ_DRIVE  *drv     = (RQ_DRIVE *)dptr;
	RQ_DEVICE *rq      = drv->rqDevice;
	RQ_PACKET *pkt     = drv->pktWork;
	UQ_CALL   *call    = rq->Callback;
	uint8     *bufData = rq->bufData;
	uint32    cmd, hstAddr, cntBytes;
	uint32    lbnBlock, dskAddr;
	uint32    wbc, tbc, abc, sts, err;
	int32     rbc;

	// Check if the packet is existing.
	if (pkt == NULL) {
#ifdef DEBUG
		dbg_Printf("%s: *** Non-existing packet on %s:\n",
			rq->devName, drv->devName);
#endif /* DEBUG */
		return;
	}

	// Decode packet to process data.
	cmd      = UQ_GETP(pkt, CMD_OPC, OPC);
	hstAddr  = UQ_GETP32(pkt, RW_WBAL);
	cntBytes = UQ_GETP32(pkt, RW_WBCL);
	lbnBlock = UQ_GETP32(pkt, RW_WLBNL);
	dskAddr  = lbnBlock * RQ_BLKSZ;

	if ((drv->Flags & DFL_ATTACHED) == 0) {
		// Unit not attached.
		rq_DataDone(drv, 0, ST_OFL | SB_OFL_NV);
		return /* TS10_OK */;
	}

	if (cntBytes == 0) {
		// No data transfers
		rq_DataDone(drv, 0, ST_SUC);
		return /* TS10_OK */;
	}

	// Up to maximum bytes of data transfer each time.
	tbc = (cntBytes > RQ_MAXFR) ? RQ_MAXFR : cntBytes;

	// Check write protection aganist write access first
	if ((cmd == OP_ERS) || (cmd == OP_WR)) {
		if (drv->uFlags & (UF_WPH|UF_WPS)) {
			sts = ST_WPR | ((drv->uFlags & UF_WPH) ? SB_WPR_HW : SB_WPR_SW);
			rq_DataDone(drv, 0, sts);
			return /* TS10_OK */;
		}
	}

	// Now process data transfers
	if (cmd == OP_ERS) {
		wbc = ((tbc + (RQ_BLKSZ - 1)) & ~(RQ_BLKSZ - 1));
		memset(bufData, 0, wbc);

		// Write a block to virtual disk.
		if ((err = lseek(drv->File, dskAddr, SEEK_SET)) == 0)
			err = write(drv->File, bufData, wbc);
	} else if (cmd == OP_WR) {
		// Get a block from host.
		if (rbc = call->ReadBlock(rq->System, hstAddr, bufData, tbc, 0)) {
			abc = tbc - rbc;
			UQ_PUTP32(pkt, RW_WBCL, cntBytes - abc);
			UQ_PUTP32(pkt, RW_WBAL, hstAddr + abc);
			if (rq_SendHostError(rq, pkt))
				rq_DataDone(drv, EF_LOG, ST_HST|SB_HST_NXM);
			return;
		}

		// Write a block to virtual disk.
		err = 0;
		wbc = ((tbc + (RQ_BLKSZ - 1)) & ~(RQ_BLKSZ - 1));
		if (lseek(drv->File, dskAddr, SEEK_SET) >= 0) {
			if (wbc - tbc)
				memset(&bufData[tbc], 0, wbc - tbc);
			if (write(drv->File, bufData, wbc) < 0)
				err = errno;
		} else
			err = errno;

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: LBN = %d  Write %d of %d bytes\n",
				rq->keyName, lbnBlock, wbc, cntBytes);
#endif /* DEBUG */
	} else {
		if (lseek(drv->File, dskAddr, SEEK_SET) >= 0) {
			if ((rbc = read(drv->File, bufData, tbc)) >= 0) {
				for (; rbc < (tbc - 1); rbc++)
					bufData[rbc] = 0;
			}
			err = 0;
		} else
			err = errno;

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: LBN = %d  Read %d of %d bytes\n",
				rq->keyName, lbnBlock, rbc, cntBytes);
#endif /* DEBUG */

		if ((cmd == OP_RD) && !err) {
			if (rbc = call->WriteBlock(rq->System, hstAddr, bufData, tbc, 0)) {
				abc = tbc - rbc;
				UQ_PUTP32(pkt, RW_WBCL, cntBytes - abc);
				UQ_PUTP32(pkt, RW_WBAL, hstAddr + abc);
				if (rq_SendHostError(rq, pkt))
					rq_DataDone(drv, EF_LOG, ST_HST|SB_HST_NXM);
				return;
			}
		} else if ((cmd == OP_CHD) && !err) {
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: Compare is not implemented yet.\n",
					rq->keyName);
#endif /* DEBUG */
		}
	}

	if (err != 0) {
#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: ** Error Code: %d (%s)\n",
				rq->keyName, err, strerror(err));
#endif /* DEBUG */
		if (rq_SendDataError(drv, pkt, ST_DRV))
			rq_DataDone(drv, EF_LOG, ST_DRV);
		return;
	}

	// Update working data.
	hstAddr  += tbc;
	cntBytes -= tbc;
	lbnBlock += ((tbc + (RQ_BLKSZ - 1)) / RQ_BLKSZ);
	UQ_PUTP32(pkt, RW_WBAL,  hstAddr);
	UQ_PUTP32(pkt, RW_WBCL,  cntBytes);
	UQ_PUTP32(pkt, RW_WLBNL, lbnBlock);

	if (cntBytes)
		ts10_SetTimer(&drv->xfrTimer);
	else
		rq_DataDone(drv, 0, ST_SUC);

	return /* TS10_OK */;
}

// Format Command (for floppy drives only)
int rq_MscpFormat(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts;
	RQ_DRIVE *drv;

	if (drv = rq_GetUnit(rq->Drives, lun)) {
		// Check if that drive still is in use.
		// If so, enter that packet into its queue to
		// process later.
		if (q && drv->pktWork) {
			rq_Enqueue(&drv->pktQueue, pkt, RQ_QTAIL);
			return UQ_OK;
		}

		if ((pkt->Data[FMT_IH] & 0100000) == 0)
			sts = ST_CMD|I_FMTI;
		else if ((drv->Flags & DFL_ATTACHED) == 0)
			sts = ST_OFL|SB_OFL_NV;
		else if ((drv->Flags & DFL_ONLINE)) {
			drv->Flags &= ~DFL_ONLINE;
			drv->uFlags = 0;
			sts = ST_AVL|SB_AVL_INU;
		} else
			sts = ST_SUC; // Successful
	} else
		sts = ST_OFL; // Offline

	rq_SetResults(pkt, cmd | OP_END, 0, sts, FMT_LNT, UQ_TYP_SEQ);

	// Ok, send a response packet back to host and return.
	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

// Read/Write Data Accesses
int rq_MscpData(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts;
	RQ_DRIVE *drv;
	
	if (drv = rq_GetUnit(rq->Drives, lun)) {
		// Check if that drive still is in use.
		// If so, enter that packet into its queue to
		// process later.
		if (q && drv->pktWork) {
			rq_Enqueue(&drv->pktQueue, pkt, RQ_QTAIL);
			return UQ_OK;
		}

		if ((sts = rq_CheckValid(drv, pkt, cmd)) == 0) {
			// Upon all data met valid, set up a working
			// packet with initial working data.
			drv->pktWork = pkt;
			pkt->Data[RW_WBAL]  = pkt->Data[RW_BAL];
			pkt->Data[RW_WBAH]  = pkt->Data[RW_BAH];
			pkt->Data[RW_WBCL]  = pkt->Data[RW_BCL];
			pkt->Data[RW_WBCH]  = pkt->Data[RW_BCH];
			pkt->Data[RW_WLBNL] = pkt->Data[RW_LBNL];
			pkt->Data[RW_WLBNH] = pkt->Data[RW_LBNH];

			// Enter timer queue now
			ts10_SetTimer(&drv->xfrTimer);

			return UQ_OK;
		}
	} else
		sts = ST_OFL; // Offline

	// Set up a new response packet.
	pkt->Data[RW_BCL] = 0;
	pkt->Data[RW_BCH] = 0;
	rq_SetResults(pkt, cmd | OP_END, 0, sts, RW_DSK_LNT, UQ_TYP_SEQ);

	// Ok, send a response packet back to host and return.
	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

// ********************************************************************

int rq_ProcessMSCP(RQ_DEVICE *rq, RQ_PACKET *pkt, uint32 q)
{
	uint32 sts, cmd = UQ_GETP(pkt, CMD_OPC, OPC);

	switch (cmd) {
		case OP_ABO: // 01 - Abort Command
			return rq_MscpAbort(rq, pkt, q);

		case OP_GCS: // 02 - Get Command Status
			return rq_MscpGetCommandStatus(rq, pkt, q);

		case OP_GUS: // 03 - Get Unit Status
			return rq_MscpGetUnitStatus(rq, pkt, q);

		case OP_SCC: // 04 - Set Controller Characteristics
			return rq_MscpSetCtlrChar(rq, pkt, q);

		case OP_AVL: // 08 - Available Command
			return rq_MscpAvailable(rq, pkt, q);

		case OP_ONL: // 09 - Online Command
			return rq_MscpOnline(rq, pkt, q);

		case OP_SUC: // 0A - Set Unit Characteristics
			return rq_MscpSetUnitChar(rq, pkt, q);

		case OP_ACC: // 10 - Access Command
		case OP_ERS: // 12 - Erase Command
		case OP_CHD: // 20 - Compare Host Data Command
		case OP_RD:  // 21 - Read Command
		case OP_WR:  // 22 - Write Command
			return rq_MscpData(rq, pkt, q);

		case OP_DAP: // 0B - Determine Access Paths
		case OP_CCD: // 11 - Compare Controller Data
		case OP_FLU: // 13 - Flush
			cmd |= OP_END; // No Operations
			sts = ST_SUC;
			break;
 
		default:
			cmd = OP_END;           // Set End Flag
			sts = ST_CMD | I_OPCD;  // Invalid Opcode
	}

	// Set up response packet
	rq_SetResults(pkt, cmd, 0, sts, RSP_LNT, UQ_TYP_SEQ);

	return rq_PutPacket(rq, pkt, RQ_QTAIL);
}

void rq_QueService(void *dptr)
{
	RQ_DEVICE *rq = (RQ_DEVICE *)dptr;
	RQ_DRIVE  *drv;
	RQ_PACKET *pkt = NULL;
	uint16    idConn;
	uint32    idx;

	// Initialization Steps here.

	// First, check and process any pending packets that
	// already had been queued for services.
	for (idx = 0; idx < rq->nDrives; idx++) {
		drv = &rq->Drives[idx];
		if (drv->Flags & DFL_EXIST) {
			// Now process next MSCP commands if available.
			if (drv->pktWork || (drv->pktQueue == NULL))
				continue;
			pkt = rq_Dequeue(&drv->pktQueue, RQ_QHEAD);
			if (rq_ProcessMSCP(rq, pkt, FALSE))
				return;
		}
	}

	// When all pending packet queues are finished,
	// can now poll new incoming packets to being
	// processed.
	if ((pkt == NULL) && (rq->Flags & CFLG_POLL)) {
		if (rq_GetPacket(rq, &pkt))
			return;
		if (pkt) {
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA)) {
				dbg_Printf(">>%s: MSCP Cmd=%04X Mode=%04X Unit=%d\n",
					rq->devName, pkt->Data[CMD_OPC], pkt->Data[CMD_MOD],
					pkt->Data[CMD_UNIT]);
			}
#endif /* DEBUG */

			// Make ensure that packet is sequence type. If not, die now.
			if (UQ_GETP(pkt, UQ_CTC, TYP) != UQ_TYP_SEQ) {
				rq_SetFatalError(rq, ER_PIE);
				return;
			}

			// Now process a packet at all time.
			idConn = UQ_GETP(pkt, UQ_CTC, CID);
			if (idConn == UQ_CID_MSCP) {
				// MSCP command processing
				if (rq_ProcessMSCP(rq, pkt, TRUE))
					return;
			} else if (idConn == UQ_CID_DUP) {
				// DUP protocol is not supported at this time.
				rq_SetResults(pkt, OP_END, 0, ST_CMD|I_OPCD, RSP_LNT, UQ_TYP_SEQ);
				if (rq_PutPacket(rq, pkt, RQ_QTAIL))
					return;
			} else {
				// Other protocols are not supported, die now.
				rq_SetFatalError(rq, ER_ICI);
				return;
			}
		} else {
			// After all packets were processed,
			// discontinue poll state.
			rq->Flags &= ~CFLG_POLL;
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: End of Poll.\n", rq->devName);
#endif /* DEBUG */
		}
	}

	// Send next response that are remining in queues.
	if (rq->pktResp) {
		pkt = rq_Dequeue(&rq->pktResp, RQ_QHEAD);
		if (rq_PutPacket(rq, pkt, RQ_QHEAD))
			return;
	}
	
	if (pkt) ts10_SetTimer(&rq->queTimer);
}

int rq_Timer(void *dptr)
{
	RQ_DEVICE *rq = (RQ_DEVICE *)dptr;

	// Host timeout? 
	if ((rq->hstTimer > 0) && (--rq->hstTimer == 0))
		rq_SetFatalError(rq, ER_HAT);

	return UQ_OK;
}

// *********************************************************************

// Reset entire controller
void rq_ResetDevice(RQ_DEVICE *rq)
{
	RQ_DRIVE  *drv;
	UQ_RING   *resRing = &rq->resRing;
	UQ_RING   *cmdRing = &rq->cmdRing;
	uint32    idx;

	// Put controller in reset state.
	rq->Flags |= CFLG_RESET;

	// Cancel all queue timers first.
	ts10_CancelTimer(&rq->queTimer);

	// Reset all drives first.
	for (idx = 0; idx < rq->nDrives; idx++) {
		drv = &rq->Drives[idx];
		ts10_CancelTimer(&drv->xfrTimer);
		drv->Flags    &= ~DFL_ONLINE;
		if (drv->dtInfo)
			drv->uFlags = drv->dtInfo->Flags | UF_RPL;
		drv->pktWork  = NULL;
		drv->pktQueue = NULL;
	}

	// Clear all ring queues and free all packets.
	memset(resRing, 0, sizeof(UQ_RING));
	memset(cmdRing, 0, sizeof(UQ_RING));
	memset(&rq->pktList, 0, sizeof(RQ_PACKET) * RQ_NPKTS);

	resRing->Name = "Response Descriptor"; // Name of Response Descriptor
	cmdRing->Name = "Command Descriptor";  // Name of Command Descriptor

	// Initialize internal packet queue.
	for (idx = 0; idx < RQ_NPKTS; idx++) {
		rq->pktList[idx].Next =
			((idx + 1) < RQ_NPKTS) ? &rq->pktList[idx + 1] : NULL;
		rq->pktList[idx].idPacket = idx;
	}
	rq->pktFree = &rq->pktList[0];
	rq->pktResp = NULL;
	rq->pktBusy = 0;

	// Reset all controller data
	rq->cFlags     = CF_RPL;
	rq->Credits    = (RQ_NPKTS / 2) - 1;
	rq->xfrTimer   = RQ_XTIMER;
	rq->hstTimeout = RQ_DHTMO+1;
	rq->hstTimer   = rq->hstTimeout;

	// Return back to initialization step #1
	S1C = SA = (SA_S1|SA_S1C_QB|SA_S1C_DI|SA_S1C_MP);
	rq->State = CST_S1;

	// Finally reset all flags.
	rq->Flags = 0; 
}

// Transition to Intiialization Step #4
int rq_Step4(RQ_DEVICE *rq)
{
	MAP_IO   *io      = &rq->ioMap;
	UQ_CALL *call    = rq->Callback; 
	UQ_RING  *resRing = &rq->resRing;
	UQ_RING  *cmdRing = &rq->cmdRing;
	uint32   base, len, idx;
	uint16   zero[CA_MAX];

	// Initialize Communication Region Area
	resRing->intAddr  = rq->rbAddr + CA_RI;
	resRing->baseAddr = rq->rbAddr;
	resRing->szDesc   = GET_RQ(S1H);
	resRing->mskDesc  = resRing->szDesc - 1;
	resRing->idxDesc  = 0;

	cmdRing->intAddr  = rq->rbAddr + CA_CI;
	cmdRing->baseAddr = rq->rbAddr + (resRing->szDesc << 2);
	cmdRing->szDesc   = GET_CQ(S1H);
	cmdRing->mskDesc  = cmdRing->szDesc - 1;
	cmdRing->idxDesc  = 0;

	base = rq->rbAddr + ((rq->Flags & CFLG_PI) ? CA_QQ : CA_CI);
	len  = ((resRing->szDesc + cmdRing->szDesc) << 2) + (rq->rbAddr - base);
	if (len > CA_MAX)
		len = CA_MAX;

	for (idx = 0; idx < (len >> 1); idx++)
		zero[idx] = 0;
	if (call->WriteBlock(rq->System, base, (uint8 *)zero, len, 0))
		return rq_SetFatalError(rq, ER_QWE);

	S4C = SA = SA_S4 | (RQ_MODEL << SA_S4C_P_MOD) | (RQ_SVER << SA_S4C_P_VER);
	rq->State = CST_S4;
	RQ_DOINTI; // Send an interrupt if enabled
}

int rq_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 size)
{
	RQ_DEVICE *rq = (RQ_DEVICE *)dptr;
	uint32    reg = (pAddr - (rq->csrAddr & 0x1FFF)) >> 1;

	switch (reg) {
		case UQ_REG_IP: // Initialization and Poll Register
			if (rq->State == CST_S3_PPB)
				rq_Step4(rq);
			else if (rq->State == CST_UP) {
				rq->Flags |= CFLG_POLL;
				ts10_SetTimer(&rq->queTimer);
			}
			*data = rq->ip;
			break;

		case UQ_REG_SA: // Status, Address, and Purge Register
			*data = rq->sa;
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) Unknown Register %d - PA=%08o D=%06o (%04X)\n",
					rq->keyName, reg, pAddr, *data, *data);
#endif /* DEBUG */
			return UQ_NXM;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%08o) => %06o (hex %04X)\n",
			rq->keyName, regName[reg], pAddr, *data, *data);
#endif /* DEBUG */

	return UQ_OK;
}

int rq_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 size)
{
	RQ_DEVICE *rq = (RQ_DEVICE *)dptr;
	MAP_IO    *io = &rq->ioMap;
	uint32    reg = (pAddr - (rq->csrAddr & 0x1FFF)) >> 1;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS) && (reg < UQ_NREGS))
		dbg_Printf("%s: (W) %s (%08o) <= %06o (hex %04X)\n",
			rq->keyName, regName[reg], pAddr, data, data);
#endif /* DEBUG */

	if (reg == UQ_REG_IP) {
		// Initialization and Poll Register
		rq_ResetDevice(rq);
	} else if (reg == UQ_REG_SA) {
		// Status, Address, and Purge Register

		switch (rq->State) {
			case CST_S1:
				// Initialization Step #1
				if (data & SA_S1H_VL) {
					if (data & SA_S1H_WR) {
						SA = data;
						rq->State = CST_S1_WRAP;
					} else {
						// Transition to Step #2
						S1H = data;
						S2C = SA = (SA_S2|SA_S2C_PT) | GET_S1H(S1H);
						rq->State = CST_S2;

						// Enable interrupts if request
						if (S1H & SA_S1H_IE)
							rq->Flags |= CFLG_IE;
						if (rq->intVector = (data & SA_S1H_VE) << 2)
							io->SetVector(io, rq->intVector, 0);
						RQ_DOINTI; // Send an interrupt if enabled
					}
				}
				break;

			case CST_S1_WRAP:
				// Wrap Mode
				SA = data;
				break;

			case CST_S2:
				// Initialization Step #2
				rq->rbAddr = data & SA_S2H_CLO; // Comm Address Low
				if (data & SA_S2H_PI)           // Purge Interrupt Request
					rq->Flags |= CFLG_PI;

				// Transition to Step #3
				S2H = data;
				S3C = SA = SA_S3 | GET_S1L(S1H);
				rq->State = CST_S3;
				RQ_DOINTI; // Send an interrupt if enabled
				break;

			case CST_S3:
				// Initialization Step #3
				rq->rbAddr |= (((uint32)(data & SA_S3H_CHI)) << 16);
				if (data & SA_S3H_PP) {
					SA = 0;
					rq->State = CST_S3_PPA;
				} else 
					rq_Step4(rq);
				break;

			case CST_S3_PPA:
				if (data)
					rq_SetFatalError(rq, ER_PPF);
				else
					rq->State = CST_S3_PPB;
				break;

			case CST_S4:
				// Initialization Step #4
				if (data & SA_S4H_GO) {
					S4H = data;
					SA  = 0;
					rq->State = CST_UP;

#ifdef DEBUG
					if (dbg_Check(DBG_IOREGS))
						rq_DumpInitInfo(rq);
#endif /* DEBUG */

					// If last error occured, send last fail packet.
					if ((data & SA_S4H_LF) && rq->errCode)
						rq_SendLastFail(rq, rq->errCode);
					rq->errCode = 0;
				}
				break;
		}
		
	} else {
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (W) Unknown Register %d - PA=%08o D=%06o (%04X)\n",
				rq->keyName, reg, pAddr, data, data);
#endif /* DEBUG */
		return UQ_NXM;
	}

	return UQ_OK;
}

// Bus Initialization from Host.
void rq_ResetIO(void *dptr)
{
	rq_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// *********************************************************************

DEVICE rq_Device;

void *rq_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	RQ_DEVICE *rq = NULL;
	CLK_QUEUE *newTimer;
	RQ_DRIVE  *drv;
	MAP_IO    *io;
	uint32    idx;

	if (rq = (RQ_DEVICE *)calloc(1, sizeof(RQ_DEVICE))) {
		// First, set up its descriptions and
		// link it to its mapping devices.
		rq->devName    = newMap->devName;
		rq->keyName    = newMap->keyName;
		rq->emuName    = newMap->emuName;
		rq->emuVersion = newMap->emuVersion;
		rq->Device     = newMap->devParent->Device;
		rq->Callback   = newMap->devParent->Callback;
		rq->System     = newMap->devParent->sysDevice;

		// Controller Flags
		rq->Flags      = 0;
		rq->csrAddr    = UQ_IOADDR;
		rq->intVector  = 0;

		// Create data buffer for data transfers.
		rq->bufData = (uint8 *)calloc(1, RQ_MAXFR);

		// Set up the new queue timer.
		newTimer           = &rq->queTimer;
		newTimer->Next     = NULL;
		newTimer->Flags    = 0;
		newTimer->outTimer = RQ_QTIMER;
		newTimer->nxtTimer = RQ_QTIMER;
		newTimer->Device   = (void *)rq;
		newTimer->Execute  = rq_QueService;

		// Create drives on depending controller.
		rq->nDrives = RQ_NDRVS;
		rq->Drives  =
			(RQ_DRIVE *)calloc(rq->nDrives, sizeof(RQ_DRIVE));
		for (idx = 0; idx < rq->nDrives; idx++) {
			drv = &rq->Drives[idx];
			drv->uFlags   = UF_RPL;
			drv->idUnit   = idx;
			drv->rqDevice = rq;

			// Set up the new response timer.
			newTimer           = &drv->xfrTimer;
			newTimer->Next     = NULL;
			newTimer->Flags    = 0;
			newTimer->outTimer = RQ_XTIMER;
			newTimer->nxtTimer = RQ_XTIMER;
			newTimer->Device   = drv;
			newTimer->Execute  = rq_ProcessData;
		}

		// Set up I/O Entry
		io               = &rq->ioMap;
		io->devName      = rq->devName;
		io->keyName      = rq->keyName;
		io->emuName      = rq->emuName;
		io->emuVersion   = rq->emuVersion;
		io->Device       = rq;
		io->csrAddr      = UQ_IOADDR;
		io->nRegs        = RQ_NREGS;
		io->intIPL       = RQ_IPL;
		io->nVectors     = RQ_NVECS;
		io->intVector[0] = 0;
		io->ReadIO       = rq_ReadIO;
		io->WriteIO      = rq_WriteIO;
		io->ResetIO      = rq_ResetIO;

		// Assign that registers to QBA's I/O Space.
		rq->Callback->SetMap(rq->Device, io);
		rq_ResetDevice(rq);

		// Finally, link it to its mapping device.
		newMap->Device = rq;
	}

	return rq;
}

void *rq_Configure(MAP_DEVICE *newMap, int argc, char **argv)
{
	RQ_DEVICE *rq  = (RQ_DEVICE *)newMap->devParent->Device;
	RQ_DRIVE  *drv = NULL;
	uint32    unit, idx;

	unit = GetDeviceUnit(newMap->devName);
	if (unit < rq->nDrives) {

		drv = &rq->Drives[unit];

		// Check if unit is existing or not first.
		if (drv->Flags & DFL_EXIST) {
			printf("%s: Already created on %s:\n",
				newMap->devName, rq->devName);
			return NULL;
		}

		for (idx = 0; rq_Drives[idx].dtName; idx++)
			if (!strcasecmp(argv[2], rq_Drives[idx].dtName))
				break;

		if (rq_Drives[idx].dtName == NULL) {
			printf("%s: No such device type on %s:\n",
				argv[2], newMap->devName);
			return NULL;
		}

		drv->devName   = newMap->devName;
		drv->dtName    = rq_Drives[idx].dtName;
		drv->dtDescrip = rq_Drives[idx].dtDescrip;
		drv->idMedia   = rq_Drives[idx].idMedia;
		drv->Flags     = rq_Drives[idx].Flags | DFL_EXIST;
		drv->uFlags    = rq_Drives[idx].Flags | UF_RPL;
		drv->dtInfo    = &rq_Drives[idx];

		newMap->idUnit     = drv->idUnit;
		newMap->keyName    = drv->dtName;
		newMap->emuName    = drv->dtDescrip;
		newMap->emuVersion = "";
		newMap->devInfo    = &rq_Device;
		newMap->Device     = rq;
	} else {
		printf("%s: No such drive on %s: - (Only %d drives)\n",
			newMap->devName, rq->devName, rq->nDrives);
		return NULL;
	}

	// Send MSCP drive information.
	return drv;
}

int rq_Reset(void *dptr)
{
	RQ_DEVICE *rq = (RQ_DEVICE *)dptr;

	rq_ResetDevice(rq);
	return EMU_OK;
}

int rq_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	RQ_DEVICE *rq  = (RQ_DEVICE *)map->Device;
	RQ_DRIVE  *drv = &rq->Drives[map->idUnit];
	uint32    mode = (drv->dtInfo->Flags & UF_WPH) ? O_RDONLY : O_RDWR|O_CREAT;

	if (drv->Flags & DFL_ATTACHED) {
		printf("%s: Already attached.  Please use 'detach %s:' first.\n",
			drv->devName, drv->devName);
		return EMU_OK;
	}

	if ((drv->File = open(argv[2], mode, 0700)) < 0) {
		printf("%s: File '%s' not attached - %s.\n",
			drv->devName, argv[2], strerror(errno));
		return EMU_OPENERR;
	}

	// Set flags as attached status.
	drv->Flags |= DFL_ATTACHED;
	if (rq->State == CST_UP)
		drv->Flags |= DFL_ATNPEND;
	drv->uFlags = drv->dtInfo->Flags | UF_RPL;

	// Tell operator that and save it.
	printf("%s: file '%s' attached.\n", drv->devName, argv[2]);
	if (drv->fileName = (char *)malloc(strlen(argv[2])+1))
		strcpy(drv->fileName, argv[2]);
	else {
		printf("%s: Can't assign the name of '%s' - Not enough memory.\n",
			map->devName, argv[2]);
	}

	return EMU_OK;
}

int rq_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	RQ_DEVICE *rq  = (RQ_DEVICE *)map->Device;
	RQ_DRIVE  *drv = &rq->Drives[map->idUnit];

	if ((drv->Flags & DFL_ATTACHED) == 0) {
		printf("%s: Already detached.\n", drv->devName);
		return EMU_OK;
	}

	close(drv->File);

	// Clear flags as detached status.
	drv->Flags  &= ~(DFL_ATTACHED|DFL_ONLINE|DFL_ATNPEND);
	drv->uFlags &= (UF_RPL|UF_WPH|UF_RMV);
	drv->File   =  0;

	// Tell operator that.
	printf("%s: file '%s' detached.\n", drv->devName,
		drv->fileName ? drv->fileName : "<Unknown filename>");

	// Free the allocation of filename if available.
	if (drv->fileName)
		free(drv->fileName);
	drv->fileName = NULL;

	return EMU_OK;
}


void rq_InfoPacket(RQ_PACKET *pkt, cchar *pktName)
{
	char outBuffer[80], *str;
	int  idx1, idx2;

	printf("%s Packet #%d\n", pktName, pkt->idPacket);
	
	for (idx1 = 0; idx1 < RQ_PKT_WSIZE;) {
		str = ToRadix(outBuffer, idx1, 16, 4);
		*str++ = ' ';
		for (idx2 = 0; (idx1 < RQ_PKT_WSIZE) && (idx2 < 8); idx2++) {
			*str++ = ' ';
			str = ToRadix(str, (uint32)pkt->Data[idx1++], 16, 4);
		}
		*str++ = '\0';
		printf("%s\n", outBuffer);
	};
	printf("\n");
}

void rq_InfoQueue(RQ_PACKET *pktList, cchar *pktName, int32 flags)
{
	RQ_PACKET *pkt;
	int32     cnt = 0;

	if (pkt = pktList) {
		printf("%s Queue:\n", pktName);
		while (pkt) {
			if (flags) {
				rq_InfoPacket(pkt, pktName);
			} else {
				if (cnt == 0)      printf("%d", pkt->idPacket);
				else if (cnt % 16) printf (", %d\n", pkt->idPacket);
				else               printf (", %d", pkt->idPacket);
			}

			// Next packet, please.
			pkt = pkt->Next;
			cnt++;
		}
		printf("\nTotal %d %s Packet%s.\n\n",
			cnt, pktName, ((cnt != 1) ? "s" : ""));
	} else
		printf("%s queue is empty.\n", pktName);
}

// Show information about MSCP controller and drives
int rq_Info(MAP_DEVICE *map, int argc, char **argv)
{
	RQ_DEVICE *rq = (RQ_DEVICE *)map->Device;
	RQ_DRIVE  *drv;
	RQ_PACKET *pkt;
	int       idx, count = 0;

	// Information about MSCP controller
	printf("\nDevice:           %s  Type: %s  State: %s\n",
		map->devName, map->keyName, cstName[rq->State]);
	printf("CSR Address:      %06X (%08o)\n", rq->csrAddr, rq->csrAddr);
	printf("Interrupt Vector: %03X (%03o)\n", rq->intVector, rq->intVector);
	printf("Ringbase Address: %06X (%08o)\n", rq->rbAddr, rq->rbAddr);
	printf("Descriptors:      %d Command Slots, %d Response Slots\n\n",
		&rq->cmdRing.szDesc, &rq->resRing.szDesc);
	printf("Status:           %s\n\n",
		(rq->Flags & CFLG_POLL) ? "Polling in progress" : "Idle");

	// Information about MSCP drives
	for (idx = 0; idx < rq->nDrives; idx++) {
		drv = &rq->Drives[idx];
		if (drv->Flags & DFL_EXIST) {
			if (count++ == 0) {
				printf(
					"Device   Type     Media ID Description\n"
					"------   ----     -------- -----------\n"
				);
			}
			printf("%-8s %-8s %-8s %s\n",
				drv->devName, drv->dtName, mscp_GetMediaID(drv->idMedia),
				drv->dtDescrip);
		}
	}

	if (count)
		printf("\nTotal %d drives.\n\n", count);
	else
		printf("\nNo drives.\n\n");

	printf("Packet Queues:\n\n");
	if (rq->State == CST_UP) {
		for (idx = 0; idx < rq->nDrives; idx++) {
			drv = &rq->Drives[idx];

			if ((drv->Flags & DFL_EXIST) == 0)
				continue;
			if (drv->pktWork) {
				printf("%s: In Progress\n", drv->devName);
				rq_InfoPacket(drv->pktWork, "Current");
			} else
				printf("%s: Idle\n", drv->devName);
			rq_InfoQueue(drv->pktQueue, "Queue", 1);
		}

		// Information about response packet queues.
		rq_InfoQueue(rq->pktResp, "Response", 1);
		rq_InfoQueue(rq->pktFree, "Free",     0);
	} else
		printf("Controller is not initialized.\n");

	return EMU_OK;
}

DEVICE rq_Device =
{
	RQ_KEY,           // Device Type Name
	RQ_NAME,          // Emulator Name
	RQ_VERSION,       // Emulator Version
	NULL,             // Listing of Devices
	DF_SYSMAP,        // Device Flags
	DT_DEVICE,        // Device Type

	NULL, NULL, NULL,

	rq_Create,        // Create Routine
	rq_Configure,     // Configure Routine
	NULL,             // Delete Routine
	rq_Reset,         // Reset Routine
	rq_Attach,        // Attach Routine  - Not Used
	rq_Detach,        // Detach Routine  - Not Used
	rq_Info,          // Info Routine
	NULL,             // Boot Routine    - Not Used
	NULL,             // Execute Routine - Not Used
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
