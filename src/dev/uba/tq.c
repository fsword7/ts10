// tq.c - TMSCP Tape Subsystem Emulator
//
// Copyright (c) 2001-2003, Timothy M. Stark
// Copyright (c) 2001-2003, Robert M. Supnik
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
// 02/05/03  TMS  Successfully created and booted the stand-alone backup
//                OpenVMS tape.
// 02/05/03  TMS  Fixed another show-stopper bug in tq_Space routine that
//                cause OpenVMS to complain about tape position lost.  When
//                detects LEOT mark (two tape marks) per request, should
//                step back (skip a tape mark backward).  Added
//                'fmt->Skip(vmt, -1);' to fix that bug.
// 02/05/03  TMS  Fixed a show-stopper bug in tq_ProcessTMSCP routine
//                that cause that cause the recent packet jumped into
//                the working queue line if working queue is empty but
//                pending queue is not. That is much like someone
//                jumped into middle of waiting line at the amusement park.
//                That's why the data misalignement results (wrong record
//                or wrong memory area).
// 02/03/03  TMS  Changed queue timer from 250 to 200 because bootstrap
//                routine hung up (a packet was "lost".)  Unforeseen
//                bugs still remain - continue debugging.
// 01/31/03  TMS  Fixed another bug in tq_Space routine that cause
//                EOF flag raise when skipping records includes tape
//                marks when attempts to boot a tape on the chveron
//                prompt (>>> b mua0).
// 01/30/03  TMS  Debugged more position/transfer routines and
//                successfully make backups and list files, etc.
// 01/30/03  TMS  Finished and debugged read/write data routines and
//                successfully mounted a tape online on OpenVMS.
// 01/29/03  TMS  Fixed a fatal bug in tq_Service routine that cause
//                TS10 system to crash with segmentation fault.
// 01/28/03  TMS  Added Command Flags for TMSCP opcodes.
// 01/13/03  TMS  Renamed RW_LNT to RW_TAP_LNT because Bob's posting
//                said that he discovered TMSCP and MSCP read/write
//                commands have different packet size.
// 10/18/02  TMS  Converted my RQDX3 emulation (rq.c file) to TMSCP
//                (tq.c file) emulation for TK50, TK70, and TU81
//                devices.
//
// -------------------------------------------------------------------------

#include "emu/defs.h"
#include "dec/tq.h"

// ********************************************************************

TQ_DTYPE tq_Drives[] = {
	{
		"TK50", "95MB Cartridge Tape Drive",
		0x6D68B032,        // Media ID
		UF_RMV,            // Unit Flags
		TF_CTP|TF_CTP_LO,  // Format Menu
		(94 * (1 << 20)),  // Capacity
		3,                 // UQ Port ID
		9,                 // Controller ID
		3,                 // Unit ID
		((1 << 8) | 5),    // Controller Version
		0,                 // Formatter Version
		0,                 // Unit Version
	},

	{
		"TK70", "300MB Cartridge Tape Drive",
		0x6A68B046,        // Media ID
		UF_RMV,            // Unit Flags
		TF_CTP|TF_CTP_LO,  // Format Menu
		(300 * (1 << 20)), // Capacity
		14,                // UQ Port ID
		14,                // Controller ID
		11,                // Unit ID
		((1 << 8) | 5),    // Controller Version
		0,                 // Formatter Version
		0,                 // Unit Version
	},

	{
		"TU81", "180MB 9-Track Tape Drive",
		0x6D695051,        // Media ID
		UF_RMV,            // Unit Flags
		TF_9TK|TF_9TK_GRP, // Format Menu
		5,                 // UQ Port ID
		5,                 // Controller ID
		2,                 // Unit ID
		((1 << 8) | 5),    // Controller Version
		0,                 // Formatter Version
		0,                 // Unit Version
	},

	{ NULL } // Null Terminator
};

// Command Flags Table
static uint32 tq_CmdFlags[64] = {
	0,
	CMF_IMM,        // Abort
	CMF_IMM|MD_CSE,        // Get Command Status
	CMF_IMM|MD_CSE|MD_NXU, // Get Unit Status
	CMF_IMM|MD_CSE,        // Set Controller Characteristics
	0, 0, 0,
	CMF_SEQ|MD_ACL|MD_CDL|MD_CSE|MD_EXA|MD_UNL, // Available
	CMF_SEQ|MD_CDL|MD_CSE|MD_SWP|MD_EXA,        // Online
	CMF_SEQ|MD_CDL|MD_CSE|MD_SWP|MD_EXA,        // Set Unit Characteristics
	CMF_IMM,                                    // Define Access Paths
	0, 0, 0, 0,
	CMF_SEQ|CMF_RW|MD_CDL|MD_CSE|MD_REV|        // Access
		MD_SCH|MD_SEC|MD_SER,
	0,
	CMF_SEQ|CMF_WR|MD_CDL|MD_CSE|MD_IMM,        // Erase
	CMF_SEQ|CMF_WR|MD_CDL|MD_CSE,               // Flush
	0, 0,
	CMF_SEQ|CMF_WR|MD_CDL|MD_CSE|MD_IMM,        // Erase Gap
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	CMF_SEQ|CMF_RW|MD_CDL|MD_CSE|MD_REV|        // Compare
		MD_SCH|MD_SEC|MD_SER,
	CMF_SEQ|CMF_RW|MD_CDL|MD_CSE|MD_REV|        // Read
		MD_CMP|MD_SCH|MD_SEC|MD_SER,
	CMF_SEQ|CMF_RW|CMF_WR|MD_CDL|MD_CSE|        // Write
		MD_IMM|MD_CMP|MD_ERW|MD_SEC|MD_SER,
	0,
	CMF_SEQ|MD_CDL|MD_CSE|MD_IMM,               // Write Tape Mark
	CMF_SEQ|MD_CDL|MD_CSE|MD_IMM|MD_OBC|        // Reposition
		MD_REV|MD_RWD|MD_DLE|MD_SCH|MD_SEC|MD_SER,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
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

char *tq_GetMediaID(uint32 idMedia)
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

void tq_DumpPacket(TQ_DEVICE *tq, TQ_PACKET *pkt, cchar *name)
{
	char outBuffer[80], *str;
	int  idx1, idx2;

	dbg_Printf("%s: %s Packet #%d\n", tq->Unit.devName, name, pkt->idPacket);
	dbg_Printf("%s:\n", tq->Unit.devName);
	
	for (idx1 = 0; idx1 < TQ_PKT_WSIZE;) {
		str = ToRadix(outBuffer, idx1, 16, 4);
		*str++ = ' ';
		for (idx2 = 0; (idx1 < TQ_PKT_WSIZE) && (idx2 < 8); idx2++) {
			*str++ = ' ';
			str = ToRadix(str, (uint32)pkt->Data[idx1++], 16, 4);
		}
		*str++ = '\0';
		dbg_Printf("%s: %s\n", tq->Unit.devName, outBuffer);
	};
}

void tq_DumpInitInfo(TQ_DEVICE *tq)
{
	dbg_Printf("%s: Initialization Complete\n", tq->Unit.devName);
	dbg_Printf("%s:\n", tq->Unit.devName);
	dbg_Printf("%s: Step   Controller       Host\n", tq->Unit.devName);
	dbg_Printf("%s: ----   -------------    -------------\n", tq->Unit.devName);
	dbg_Printf("%s: #1     %06o (%04X)    %06o (%04X)\n",
		tq->Unit.devName, S1C, S1C, S1H, S1H);
	dbg_Printf("%s: #2     %06o (%04X)    %06o (%04X)\n",
		tq->Unit.devName, S2C, S2C, S2H, S2H);
	dbg_Printf("%s: #3     %06o (%04X)    %06o (%04X)\n",
		tq->Unit.devName, S3C, S3C, S3H, S3H);
	dbg_Printf("%s: #4     %06o (%04X)    %06o (%04X)\n",
	 	tq->Unit.devName, S4C, S4C, S4H, S4H);
	dbg_Printf("%s:\n", tq->Unit.devName);

	dbg_Printf("%s: Ringbase Address: %08o (hex %06X)\n",
		tq->Unit.devName, tq->rbAddr, tq->rbAddr);
	dbg_Printf("%s: Descriptors:      %d Command Slots, %d Response Slots\n",
		tq->Unit.devName, tq->cmdRing.szDesc, tq->resRing.szDesc);
	dbg_Printf("%s: Interrupt Vector: %03o (hex %03X), %s\n",
		tq->Unit.devName, tq->intVector, tq->intVector,
		((S1H & SA_S1H_IE) ? "Enabled" : "Disabled"));
}

void tq_DumpQueue(TQ_DRIVE *drv, TQ_PACKET *pktList, cchar *pktName)
{
	TQ_PACKET *pkt;
	char str[256], *p = str;
	int  cnt = 0;

	p += sprintf(p, "%s Queue: ", pktName);
	if (pkt = pktList) {
		while (pkt) {
			if (cnt > 0)
				p += sprintf(p, ", %d", pkt->idPacket);
			else
				p += sprintf(p, "%d", pkt->idPacket);
			pkt = pkt->Next;
			cnt++;
		}
	} else
		p += sprintf(p, "(Empty)");
	dbg_Printf("%s: %s\n", drv->Unit.devName, str);
}
#endif /* DEBUG */

// ********************************************************************

// Fatal error - Controller now is dead.
int tq_SetFatalError(TQ_DEVICE *tq, int error)
{
	tq->State = CST_DEAD;
	tq->errCode = error;
	SA = SA_ER | error;

	return UQ_ERROR;
}

// Find a free packet in internal queue.
TQ_PACKET *tq_GetFreePacket(TQ_DEVICE *tq)
{
	TQ_PACKET *newPacket = NULL;

	if (tq->pktFree) {
		newPacket   = tq->pktFree;
		tq->pktFree = newPacket->Next;
		tq->pktBusy++;
	}

	return newPacket;
}

void tq_Enqueue(TQ_PACKET **list, TQ_PACKET *pkt, int which)
{
	TQ_PACKET *ptr;

	if (pkt) {
		if (which == TQ_QHEAD) {
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

TQ_PACKET *tq_Dequeue(TQ_PACKET **list, int which)
{
	TQ_PACKET *pkt = NULL;

	if (*list) {
		pkt   = *list;
		*list = pkt->Next;
	}

	return pkt;
}

// Get a descriptor from host
int tq_GetDesc(TQ_DEVICE *tq, UQ_RING *ring, uint32 *desc)
{
	UQ_CALL *call = tq->Callback;
	uint32  addr  = ring->baseAddr + (ring->idxDesc << 2);

	if (call->ReadBlock(tq->System, addr, (uint8 *)desc, 4, 0))
		return tq_SetFatalError(tq, ER_QRE);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: (R) %s %d (%08o, hex %06X) => %08X\n",
			tq->Unit.devName, ring->Name, ring->idxDesc, addr, addr, *desc);
#endif /* DEBUG */

	return UQ_OK;
}

// Send a descriptor to host and send an interrupt if request
int tq_PutDesc(TQ_DEVICE *tq, UQ_RING *ring, uint32 desc)
{
	MAP_IO  *io     = &tq->ioMap;
	UQ_CALL *call   = tq->Callback;
	uint32  addr    = ring->baseAddr + (ring->idxDesc << 2);
	uint32  newDesc = (desc & ~UQ_DESC_OWN) | UQ_DESC_DONE;
	uint16  flag    = 1;

	if (call->WriteBlock(tq->System, addr, (uint8 *)&newDesc, 4, 0))
		return tq_SetFatalError(tq, ER_QWE);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: (W) %s %d (%08o, hex %06X) <= %08X\n",
			tq->Unit.devName, ring->Name, ring->idxDesc, addr, addr, newDesc);
#endif /* DEBUG */

	if (desc & UQ_DESC_INT) {
		if (ring->szDesc > 1) {
			uint32 prvAddr, prvDesc;

			prvAddr = ring->baseAddr +
				(((ring->idxDesc - 1) & ring->mskDesc) << 2);
			if (call->ReadBlock(tq->System, prvAddr, (uint8 *)&prvDesc, 4, 0))
				return tq_SetFatalError(tq, ER_QRE);
			if (prvDesc & UQ_DESC_OWN) {
				call->WriteBlock(tq->System, ring->intAddr, (uint8 *)&flag, 2, 0);
				TQ_DOINT;
			}
		} else {
			call->WriteBlock(tq->System, ring->intAddr, (uint8 *)&flag, 2, 0);
			TQ_DOINT;
		}
	}

	// Increment Descriptor Index
	ring->idxDesc = (ring->idxDesc + 1) & ring->mskDesc;

	return UQ_OK;
}

// Get a message packet from host
int tq_GetPacket(TQ_DEVICE *tq, TQ_PACKET **pkt)
{
	UQ_CALL *call = tq->Callback;
	uint32  desc;    // Current descriptor
	uint32  pktAddr; // Packet Address

	// Get a command descriptor from host
	if (tq_GetDesc(tq, &tq->cmdRing, &desc))
		return UQ_ERROR;

	// Check if descriptor belongs to us.
	if (desc & UQ_DESC_OWN) {
		if (*pkt = tq_GetFreePacket(tq)) {
			pktAddr = (desc & UQ_DESC_ADDR) + UQ_HDR_OFF;

			// Disable Host Timer.
			tq->hstTimer = 0;

#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: (R) Command Packet #%d (%08o, hex %06X)\n",
					tq->Unit.devName, (*pkt)->idPacket, pktAddr, pktAddr);
#endif /* DEBUG */

			// Get a command packet from host
			if (call->ReadBlock(tq->System, pktAddr, (uint8 *)&(*pkt)->Data, TQ_PKT_BSIZE, 0))
				return tq_SetFatalError(tq, ER_PRE);

#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				tq_DumpPacket(tq, *pkt, "Command");
#endif /* DEBUG */

			// Now release a command descriptor to host.
			tq_PutDesc(tq, &tq->cmdRing, desc);
		} else {
			// ** Fatal Error - No such free packets in internal queue.
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: *** Fatal Error - No such free packets\n",
					tq->Unit.devName);
#endif /* DEBUG */
			return tq_SetFatalError(tq, ER_NSR);
		}
	} else
		*pkt = NULL;

	return UQ_OK;
}

// Put a message packet into host
int tq_PutPacket(TQ_DEVICE *tq, TQ_PACKET *pkt, int wq)
{
	UQ_CALL *call = tq->Callback;
	uint32  desc, cr;
	uint32  pktAddr, pktLen;

	if (pkt == NULL)
		return UQ_OK;

	// Get a command descriptor from host
	if (tq_GetDesc(tq, &tq->resRing, &desc))
		return UQ_ERROR;

	if (desc & UQ_DESC_OWN) {
		pktAddr = (desc & UQ_DESC_ADDR) + UQ_HDR_OFF;
		pktLen  = pkt->Data[UQ_LNT] - UQ_HDR_OFF;

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: (R) Response Packet #%d (%08o, hex %06X)\n",
				tq->Unit.devName, pkt->idPacket, pktAddr, pktAddr);
#endif /* DEBUG */

		// Issue credits if any
		if ((UQ_GETP(pkt, UQ_CTC, TYP) == UQ_TYP_SEQ) &&
		    (UQ_GETP(pkt, CMD_OPC, OPC) & OP_END)) {
			cr = (tq->Credits > 14) ? 14 : tq->Credits;
			tq->Credits -= cr;
			pkt->Data[UQ_CTC] |= (++cr << UQ_CTC_P_CR);

#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: %d Credit%s Issued - Remaining %d Credits.\n",
					tq->Unit.devName, cr, (cr == 1 ? "s" : ""), tq->Credits);
#endif /* DEBUG */
		}

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			tq_DumpPacket(tq, pkt, "Response");
#endif /* DEBUG */

		// Now send a packet to host.
		if (call->WriteBlock(tq->System, pktAddr, (uint8 *)&pkt->Data, pktLen, 0))
			return tq_SetFatalError(tq, ER_PWE);

		// Release a old packet to free list.
		// Enable host timer if idle (no busy packets).
		tq_Enqueue(&tq->pktFree, pkt, TQ_QHEAD);
		if (--tq->pktBusy == 0)
			tq->hstTimer = tq->hstTimeout;

		// Release a response descriptor to host and return.
		return tq_PutDesc(tq, &tq->resRing, desc);
	} else {
		tq_Enqueue(&tq->pktResp, pkt, wq);
		ts10_SetTimer(&tq->queTimer);

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: Packet Enqueued.\n", tq->Unit.devName);
#endif /* DEBUG */
	}

	return UQ_OK;
}

TQ_DRIVE *tq_GetUnit(TQ_DRIVE *drv, uint32 lun)
{
	if ((lun < TQ_NDRVS) && (drv[lun].Flags & DF_EXIST))
		return &drv[lun];
	return NULL;
}

void tq_SetUnitFlags(TQ_DRIVE *drv, TQ_PACKET *pkt)
{
	// Update unit flags.
	drv->uFlags = (drv->uFlags & (UF_WPH|UF_RMV)) |
		(pkt->Data[ONL_UFLG] & UF_MASK);

	// Update software write lock enable.
	if ((pkt->Data[CMD_MOD] & MD_SWP) &&
		 (pkt->Data[ONL_UFLG] & UF_WPS))
		drv->uFlags |= UF_WPS;
}

// Set end flags for response packets.
uint32 tq_SetEndFlags(TQ_DRIVE *drv)
{
	uint32 flg = 0;

	if (drv->Flags & DF_POL)
		flg |= EF_POL; // Position Lost
	if (drv->Flags & DF_SXC)
		flg |= EF_SXC; // Serious Exception

	return flg;
}

void tq_SetResults(TQ_PACKET *pkt, uint32 cmd,
	uint32 flg, uint32 sts, uint32 lnt, uint32 typ)
{
	pkt->Data[UQ_LNT]  = lnt;
	pkt->Data[UQ_CTC]  = (typ << UQ_CTC_P_TYP) | (UQ_CID_TMSCP << UQ_CTC_P_CID);
	pkt->Data[RSP_OPF] = (cmd << RSP_OPF_P_OPC) | (flg << RSP_OPF_P_FLG);
	pkt->Data[RSP_STS] = sts;
}

// Data Transfer Error Log Packet
int tq_SendDataError(TQ_DRIVE *drv, TQ_PACKET *pkt, uint32 errCode)
{
	TQ_DEVICE *tq     = drv->tqDevice;
	TQ_DTYPE  *dtInfo = drv->dtInfo;
	TQ_PACKET *epkt; // Error packet
	uint32    lun;

	if (tq->cFlags & CF_THS)
		return TQ_OK;
	if ((epkt = tq_GetFreePacket(tq)) == NULL)
		return TQ_ERROR;
	lun = pkt->Data[CMD_UNIT];

	epkt->Data[ELP_REFL] = pkt->Data[CMD_REFL];
	epkt->Data[ELP_REFH] = pkt->Data[CMD_REFH];
	epkt->Data[ELP_UNIT] = lun;
	epkt->Data[ELP_SEQ]  = 0;
	epkt->Data[DTE_CIDA] = 0;
	epkt->Data[DTE_CIDB] = 0;
	epkt->Data[DTE_CIDC] = 0;
	epkt->Data[DTE_CIDD] = (TQ_CLASS << DTE_CIDD_P_CLS) |
		(tq->idCtlr << DTE_CIDD_P_MOD);
	epkt->Data[DTE_CVER]  = dtInfo->cVersion;
	epkt->Data[DTE_MLUN] = lun;
	epkt->Data[DTE_UIDA] = lun;
	epkt->Data[DTE_UIDB] = 0;
	epkt->Data[DTE_UIDC] = 0;
	epkt->Data[DTE_UIDD] = (CLS_TAPE << DTE_UIDD_P_CLS) |
		(dtInfo->idUnit << DTE_UIDD_P_MOD);
	epkt->Data[DTE_UVER] = dtInfo->uVersion;
	epkt->Data[DTE_FVER] = dtInfo->fVersion;
	UQ_PUTP32(epkt, DTE_POSL, drv->cntObject);

	tq_SetResults(epkt, FM_TAP, LF_SNR, errCode, DTE_LNT, UQ_TYP_DATA);
	return tq_PutPacket(tq, epkt, TQ_QTAIL);
}

// Host Bus Error Log Packet
int tq_SendHostError(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 ba)
{
	TQ_PACKET *epkt; // Error packet

	if (tq->cFlags & CF_THS)
		return TQ_OK;
	if ((epkt = tq_GetFreePacket(tq)) == NULL)
		return TQ_ERROR;

	epkt->Data[ELP_REFL] = pkt->Data[CMD_REFL];
	epkt->Data[ELP_REFH] = pkt->Data[CMD_REFH];
	epkt->Data[ELP_UNIT] = pkt->Data[CMD_UNIT];
	epkt->Data[ELP_SEQ]  = 0;
	epkt->Data[HBE_CIDA] = 0;
	epkt->Data[HBE_CIDB] = 0;
	epkt->Data[HBE_CIDC] = 0;
	epkt->Data[HBE_CIDD] = (TQ_CLASS << DTE_CIDD_P_CLS) |
		(tq->idCtlr << DTE_CIDD_P_MOD);
	epkt->Data[HBE_CVER] = tq->cVersion;
	epkt->Data[HBE_RSV]  = 0;
	UQ_PUTP32(epkt, HBE_BADL, ba);

	tq_SetResults(epkt, FM_BAD, LF_SNR, ST_HST|SB_HST_NXM, HBE_LNT, UQ_TYP_DATA);
	return tq_PutPacket(tq, epkt, TQ_QTAIL);
}

int tq_SendLastFail(TQ_DEVICE *tq, int errCode)
{
	TQ_PACKET *pkt;

	if (pkt = tq_GetFreePacket(tq)) {
		pkt->Data[ELP_REFL] = 0;
		pkt->Data[ELP_REFH] = 0;
		pkt->Data[ELP_UNIT] = 0;
		pkt->Data[ELP_SEQ]  = 0;

		pkt->Data[PLF_CIDA] = 0;
		pkt->Data[PLF_CIDB] = 0;
		pkt->Data[PLF_CIDC] = 0;
		pkt->Data[PLF_CIDD] = (TQ_CLASS << PLF_CIDD_P_CLS) |
			(tq->idCtlr << PLF_CIDD_P_MOD);
		pkt->Data[PLF_VER]  = tq->cVersion;
		pkt->Data[PLF_ERR]  = errCode;

		tq_SetResults(pkt, FM_CNT, LF_SNR, ST_CNT, PLF_LNT, UQ_TYP_DATA);
		pkt->Data[UQ_CTC] |= (UQ_CID_DIAG << UQ_CTC_P_CID);

		// Ok, Send a last fail packet to host.
		return tq_PutPacket(tq, pkt, TQ_QTAIL);
	} else
		return UQ_ERROR;
}

// Attention Message - Unit Now Available
int tq_MscpUnitNowAvailable(TQ_DEVICE *tq, TQ_DRIVE *drv)
{
	TQ_PACKET *pkt;

	if (pkt = tq_GetFreePacket(tq)) {
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
		pkt->Data[UNA_UIDD] = (CLS_TAPE << UNA_UIDD_P_CLS) |
			(drv->dtInfo->idUnit << UNA_UIDD_P_MOD);

		// Ok, Send an attention packet to host.
		tq_SetResults(pkt, OP_AVA, 0, 0, UNA_LNT, UQ_TYP_SEQ);
		return tq_PutPacket(tq, pkt, TQ_QTAIL);
	} else
		return UQ_ERROR;
}

// *******************************************************************

// Validity Checks
int tq_CheckValid(TQ_DRIVE *drv, TQ_PACKET *pkt, uint32 cmd)
{
	// Check unit for existant and attached.
	// if not, tell host that unit is available or offline.
	if (drv->Flags & DF_SXC)
		return ST_SXC;
	if ((drv->Flags & DF_ATT) == 0)
		return (ST_OFL | SB_OFL_NV);
	if ((drv->Flags & DF_ONL) == 0)
		return ST_AVL;

	// Check write protection aganist write accesses
	// If so, tell host that write access is denied.
	if ((cmd == OP_WR) || (cmd == OP_ERS)) {
		if (drv->uFlags & UF_WPS) {
			drv->Flags |= DF_SXC;
			return (ST_WPR | SB_WPR_SW);
		}
		if (drv->uFlags & UF_WPH) {
			drv->Flags |= DF_SXC;
			return (ST_WPR | SB_WPR_HW);
		}
	}

	// Won! Success! All data passed huge tests.
	return ST_SUC;
}

int tq_Results(TQ_DRIVE *drv, uint32 rc, int dir)
{ 
	switch (rc) {
		case MT_MARK:  // Tape Mark
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: *** Tape Mark ***\n", drv->Unit.devName);
#endif /* DEBUG */
			drv->cntObject += dir;
			drv->Flags     |= (DF_SXC|DF_TMK);
			return ST_TMK;

		case MT_EOM:   // End of Medium
		case MT_EOT:   // End of Tape
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: *** End of Tape ***\n", drv->Unit.devName);
#endif /* DEBUG */
			drv->Flags |= (DF_SXC|DF_POL);
			return ST_DAT;

		case MT_BOT:   // Bottom of Tape
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: *** Bottom of Tape ***\n", drv->Unit.devName);
#endif /* DEBUG */
			drv->Flags = (DF_SXC | (drv->Flags & ~DF_POL));
			return ST_BOT;

		case MT_ERROR: // Tape Error
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS)) {
				VMT_TAPE *vmt = &drv->dpTape;
				dbg_Printf("%s: *** Error: %d (%s) ***\n",
					drv->Unit.devName, vmt->errCode, strerror(vmt->errCode));
			}
#endif /* DEBUG */
		case MT_CRC:   // Bad Record
		default:
			drv->Flags |= (DF_SXC|DF_POL);
			return ST_DRV;
	}
}

int tq_Rewind(TQ_DRIVE *drv)
{
	VMT_TAPE   *vmt = &drv->dpTape;
	VMT_FORMAT *fmt = vmt->Format;

	fmt->Rewind(vmt);
	drv->cntObject = 0;
	drv->Flags &= ~(DF_TMK|DF_POL);

	// Successful Operation
	return ST_SUC;
}

int tq_Unload(TQ_DRIVE *drv)
{
	VMT_TAPE   *vmt = &drv->dpTape;
	VMT_FORMAT *fmt = vmt->Format;

	// Clear flags as detached status.
	drv->Flags  &= ~(DF_ATT|DF_WLK|DF_ONL|DF_ATP);
	drv->uFlags &= (UF_WPH|UF_RMV);
	vmt_CloseTape(vmt);

	// Clean up a mess.
	drv->fileName = NULL;
	if (vmt->fileName) {
		free(vmt->fileName);
		vmt->fileName = NULL;
	}
	vmt->fmtName = NULL;

	// Successful Operation
	return ST_SUC;
}

int tq_Space(TQ_DRIVE *drv, uint32 nRecs, uint32 *skRecs, uint32 skFlags)
{
	VMT_TAPE   *vmt = &drv->dpTape;
	VMT_FORMAT *fmt = vmt->Format;
	int rc;

	if (skFlags & MD_REV) {
		// Space Backward
		while (*skRecs < nRecs) {
			if ((rc = fmt->Skip(vmt, -1)) < -1)
				return tq_Results(drv, rc, -1);
			drv->cntObject--;
			if (rc == MT_MARK) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: *** Tape Mark ***\n", drv->Unit.devName);
#endif /* DEBUG */
				if (skFlags & TRUE)
					return ST_TMK;
			}
			(*skRecs)++;
		}
	} else {
		// Space Forward
		while (*skRecs < nRecs) {
			if ((rc = fmt->Skip(vmt, 1)) < -1)
				return tq_Results(drv, rc, 1);
			drv->cntObject++;
			if (rc == MT_MARK) {
				// Check the LEOT mark.
				if ((drv->pktWork->Data[CMD_MOD] & MD_DLE) &&
				    (drv->Flags & DF_TMK)) {
#ifdef DEBUG
					if (dbg_Check(DBG_IOREGS))
						dbg_Printf("%s: *** LEOT ***\n", drv->Unit.devName);
#endif /* DEBUG */
					fmt->Skip(vmt, -1); // Step back
					drv->Flags |= DF_SXC;
					return ST_LED;
				}
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: *** Tape Mark ***\n", drv->Unit.devName);
#endif /* DEBUG */
				drv->Flags |= DF_TMK;
				if (skFlags & TRUE)
					return ST_TMK;
			} else
				drv->Flags &= ~DF_TMK;
			(*skRecs)++;
		}
	}

	// Successful Operation
	return ST_SUC;
}

int tq_SkipFile(TQ_DRIVE *drv, uint32 nMarks, uint32 *skMarks, uint32 skFlags)
{
	VMT_TAPE   *vmt = &drv->dpTape;
	VMT_FORMAT *fmt = vmt->Format;
	uint32     skRecs = 0;
	int        sts;

	// Skip files (tape marks).
	while (*skMarks < nMarks) {
		sts = tq_Space(drv, -1, &skRecs, skFlags | TRUE);
		if (sts == ST_TMK)
			(*skMarks)++;
		else if (sts != ST_SUC)
			return sts;
	}

	// Successful Operation
	return ST_SUC;
}

int tq_Read(TQ_DRIVE *drv, uint8 *buf, uint32 *tbc, uint32 flags)
{
	VMT_TAPE   *vmt = &drv->dpTape;
	VMT_FORMAT *fmt = vmt->Format;
	int rbc;

	if ((rbc = fmt->Read(vmt, buf, TQ_MAXFR)) < 0)
		return tq_Results(drv, rbc, 1);
	drv->Flags &= ~DF_TMK;
	if (rbc > TQ_MAXFR) {
		drv->Flags |= (DF_SXC|DF_POL);
		return ST_FMT;
	}
	drv->cntObject++;
	*tbc = rbc;

	// Successful Operation
	return ST_SUC;
}

int tq_Write(TQ_DRIVE *drv, uint8 *buf, uint32 len)
{
	VMT_TAPE   *vmt = &drv->dpTape;
	VMT_FORMAT *fmt = vmt->Format;
	int        rc;

	if ((rc = fmt->Write(vmt, buf, len)) < 0)
		return tq_Results(drv, rc, 1);

	// Successful Operation
	return ST_SUC;
}

// Write Tape Mark
int tq_Mark(TQ_DRIVE *drv)
{
	VMT_TAPE   *vmt = &drv->dpTape;
	VMT_FORMAT *fmt = vmt->Format;
	int        rc;

	if ((rc = fmt->Mark(vmt)) < 0)
		return tq_Results(drv, rc, 1);
	drv->cntObject++;

	// Successful Operation
	return ST_SUC;
}

int tq_Erase(TQ_DRIVE *drv)
{
	VMT_TAPE   *vmt = &drv->dpTape;
	VMT_FORMAT *fmt = vmt->Format;
	int        rc;

	if ((rc = fmt->Erase(vmt)) < 0)
		return tq_Results(drv, rc, 1);
	fmt->Rewind(vmt);
	drv->cntObject = 0;
	drv->Flags &= ~(DF_TMK|DF_POL);

	// Successful Operation
	return ST_SUC;
}

// Read/Write Data Done
int tq_DataDone(TQ_DRIVE *drv, uint32 flg, uint32 sts, uint32 rsiz)
{
	TQ_DEVICE *tq  = drv->tqDevice;
	TQ_PACKET *pkt = drv->pktWork;
	uint32    cmd  = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32    lnt;

	drv->pktWork = NULL; // Done

	if (cmd == OP_ERG)      lnt = ERG_LNT;
	else if (cmd == OP_ERS) lnt = ERS_LNT;
	else if (cmd == OP_WTM) lnt = WTM_LNT;
	else if (cmd == OP_POS) lnt = POS_LNT;
	else                    lnt = RW_TAP_LNT;

	if (lnt > ERG_LNT) {
		UQ_PUTP32(pkt, RW_POSL, drv->cntObject);
		UQ_PUTP32(pkt, RW_RSZL, rsiz);
	}

	// Send a response packet back to host.
	flg |= tq_SetEndFlags(drv);
	tq_SetResults(pkt, cmd | OP_END, flg, sts, lnt, UQ_TYP_SEQ);
	if (tq_PutPacket(tq, pkt, TQ_QTAIL))
		return UQ_ERROR;

	// Any next packet?
	if (drv->pktQueue)
		ts10_SetTimer(&tq->queTimer);
	return UQ_OK;
}

// Data Error
void tq_DataError(TQ_DRIVE *drv, TQ_PACKET *pkt, uint32 rsiz)
{
	// Set serious exception flag
	drv->Flags = (drv->Flags & ~DF_TMK) | DF_SXC;

	// Send two packets (data error and response)
	// to host and return.
	if (tq_SendDataError(drv, pkt, ST_DRV))
		tq_DataDone(drv, EF_LOG, ST_DRV, rsiz);
}

// Read/Write Data Processing during queue timer
void tq_Service(void *dptr)
{
	TQ_DRIVE  *drv  = (TQ_DRIVE *)dptr;
	TQ_DEVICE *tq   = drv->tqDevice;
	TQ_PACKET *pkt  = drv->pktWork;
	UQ_CALL   *call = tq->Callback;
	uint8     *buf  = tq->bufData;
	uint32    cmd, mdf, ba, bc;
	uint32    nRecs,  skRecs;
	uint32    nMarks, skMarks;
	uint32    skFlags;
	uint32    rbc, wbc, tbc, sts, err;

	// Check if the packet is existing.
	if (pkt == NULL) {
#ifdef DEBUG
		dbg_Printf("%s: *** Non-existing packet on %s:\n",
			tq->Unit.devName, drv->Unit.devName);
#endif /* DEBUG */
		return;
	}

	if ((drv->Flags & DF_ATT) == 0) {
		// Unit not attached.
		tq_DataDone(drv, 0, ST_OFL | SB_OFL_NV, 0);
		return /* TS10_OK */;
	}

#ifdef DEBUG
//	dbg_Printf("%s: Executing Packet #%d\n",
//		drv->Unit.devName, pkt->idPacket);
#endif /* DEBUG */

	// Decode packet to process data.
	cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	mdf = pkt->Data[CMD_MOD];
	sts = ST_SUC; // Assume successful
	tbc = 0;      // Assume zero record

	// Check write protection aganist write access first
	if ((tq_CmdFlags[cmd] & CMF_WR) && 
		 (drv->uFlags & (UF_WPH|UF_WPS))) {
		drv->Flags |= DF_SXC;
		sts = ST_WPR | ((drv->uFlags & UF_WPH) ? SB_WPR_HW : SB_WPR_SW);
		tq_DataDone(drv, 0, sts, 0);
		return /* TS10_OK */;
	}

	switch (cmd) {
		case OP_WTM: // Write Tape Mark
			if (sts = tq_Mark(drv)) {
				tq_DataError(drv, pkt, 0);
				return;
			}
		case OP_ERG: // Erase Gap
			// Action: Test EOT
			drv->Flags &= ~DF_TMK;
			break;

		case OP_ERS:
			sts = tq_Erase(drv);
			break;

		case OP_POS: // Position
			nRecs   = UQ_GETP32(pkt, POS_RCL);   // # of records to skip
			nMarks  = UQ_GETP32(pkt, POS_TMCL);  // # of marks to skip
			skFlags = (mdf & MD_REV);
			skRecs  = skMarks = 0;

			if (mdf & MD_RWD)
				tq_Rewind(drv);
			if (mdf & MD_OBC)
				sts = tq_Space(drv, nRecs, &skRecs, skFlags);
			else {
				sts = tq_SkipFile(drv, nMarks, &skMarks, skFlags);
				if (sts == ST_SUC) {
					sts = tq_Space(drv, nRecs, &skRecs, skFlags | TRUE);
					if (sts == ST_TMK)
						skMarks++;
				}
			}

			// Return results of records and marks skipped.
			UQ_PUTP32(pkt, POS_RCL,  skRecs);  // #Records skipped.
			UQ_PUTP32(pkt, POS_TMCL, skMarks); // #Marks skipped.
			break;

		case OP_RD:  // Read Data
		case OP_ACC: // Access
		case OP_CHD: // Compare Data
			// Get bus address and byte count.
			ba = UQ_GETP32(pkt, RW_BAL);
			bc = UQ_GETP32(pkt, RW_BCL);

			sts = tq_Read(drv, buf, &tbc, (mdf & MD_REV));
			if (sts == ST_DRV) {
				UQ_PUTP32(pkt, RW_BCL, 0);
				tq_DataError(drv, pkt, tbc);
				return;
			} else if ((sts != ST_SUC) || (cmd == OP_ACC)) {
				UQ_PUTP32(pkt, RW_BCL, 0);
				break;
			}

			if (tbc > bc) {
				wbc = bc;
				sts = ST_RDT;
				drv->Flags |= DF_SXC;
			} else
				wbc = tbc;

			if (cmd == OP_RD) {
#ifdef DEBUG
//				dbg_Printf("%s: Transfer %d bytes to loc %08X\n",
//					drv->Unit.devName, wbc, ba);
#endif /* DEBUG */
				// Put a record into host system.
				if (rbc = call->WriteBlock(tq->System, ba, buf, wbc, 0)) {
					UQ_PUTP32(pkt, RW_BCL, (wbc - rbc));
					if (tq_SendHostError(tq, pkt, ba + (wbc - rbc)))
						tq_DataDone(drv, EF_LOG, ST_HST|SB_HST_NXM, tbc);
					return;
				}
			} else {
				// Compare Opcode
			}

			UQ_PUTP32(pkt, RW_BCL, wbc);
			break;
	
		case OP_WR: // Write Data
			// Get bus address and byte count.
			ba = UQ_GETP32(pkt, RW_BAL);
			bc = UQ_GETP32(pkt, RW_BCL);

			// Get a record from host.
			if (rbc = call->ReadBlock(tq->System, ba, buf, bc, 0)) {
				UQ_PUTP32(pkt, RW_BCL, 0);
				if (tq_SendHostError(tq, pkt, ba + (bc - rbc)))
					tq_DataDone(drv, EF_LOG, ST_HST|SB_HST_NXM, bc);
				return;
			}

			// Write a record to tape medium.
			if (tq_Write(drv, buf, bc)) {
				tq_DataError(drv, pkt, bc);
				return;
			}
			drv->cntObject++;
			drv->Flags &= ~DF_TMK;
			break;
	}

	tq_DataDone(drv, 0, sts, tbc);
	return /* TS10_OK */;
}

// ********************** TMSCP Commands **********************

// 01 - Abort Command
int tq_MscpAbort(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	TQ_DRIVE  *drv;
	TQ_PACKET *tpkt, *prv;
	uint32    lun = pkt->Data[CMD_UNIT];
	uint32    cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32    ref = UQ_GETP32(pkt, ABO_REFL);

	if (drv = tq_GetUnit(tq->Drives, lun)) {
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
			tq_SetResults(tpkt, tcmd | OP_END, 0, ST_ABO, RSP_LNT, UQ_TYP_SEQ);
			if (tq_PutPacket(tq, tpkt, TQ_QTAIL))
				return UQ_ERROR;
		}
	}

	// Ok, send a response packet back to host.
	tq_SetResults(pkt, cmd | OP_END, 0, ST_SUC, ABO_LNT, UQ_TYP_SEQ);
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 02 - Get Command Status Command
int tq_MscpGetCommandStatus(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	TQ_DRIVE  *drv;
	TQ_PACKET *tpkt;
	uint32    lun = pkt->Data[CMD_UNIT];
	uint32    cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32    ref = UQ_GETP32(pkt, GCS_REFL);

	if ((drv = tq_GetUnit(tq->Drives, lun)) &&
	    (tpkt = drv->pktWork) &&
		 (UQ_GETP32(tpkt, CMD_REFL) == ref) &&
		 (tq_CmdFlags[UQ_GETP(tpkt, CMD_OPC, OPC)] & CMF_RW)) {
		pkt->Data[GCS_STSL] = tpkt->Data[RW_BCL];
		pkt->Data[GCS_STSH] = tpkt->Data[RW_BCH];
	} else {
		pkt->Data[GCS_STSL] = 0;
		pkt->Data[GCS_STSH] = 0;
	}

	// Ok, Send a command status packet back to host.
	tq_SetResults(pkt, cmd | OP_END, 0, ST_SUC, GCS_LNT, UQ_TYP_SEQ);
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 03 - Get Unit Status Command
int tq_MscpGetUnitStatus(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 wq)
{
	TQ_DRIVE *drv;
	TQ_DTYPE *dtInfo;
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts, flg;

	// Check unit # as next unit or not.
	// If not, wrap back to unit #0.
	if (pkt->Data[CMD_MOD] & MD_NXU) {
		if (lun >= tq->nDrives) {
			lun = 0;
			pkt->Data[RSP_UNIT] = lun;
		}
	}

	if (drv = tq_GetUnit(tq->Drives, lun)) {
		dtInfo = drv->dtInfo;

		// Determine unit attributes and put results in status.
		if ((drv->Flags & DF_ATT) == 0)
			sts = ST_OFL | SB_OFL_NV; // No volume
		else if (drv->Flags & DF_ONL)
			sts = ST_SUC; // Online - Success
		else
			sts = ST_AVL; // Available - Success

		// Set up a new response packet.
		pkt->Data[GUS_MLUN] = lun;
		pkt->Data[GUS_UFLG] = drv->uFlags;
		pkt->Data[GUS_RSVL] = 0;
		pkt->Data[GUS_RSVH] = 0;
		pkt->Data[GUS_UIDA] = lun;
		pkt->Data[GUS_UIDB] = 0;
		pkt->Data[GUS_UIDC] = 0;
		pkt->Data[GUS_UIDD] = dtInfo->idUnit;
		UQ_PUTP32(pkt, GUS_MEDL, dtInfo->idMedia);
		pkt->Data[GUS_MENU] = dtInfo->Format;
		pkt->Data[GUS_CAP]  = 0;
		pkt->Data[GUS_FVER] = dtInfo->fVersion; // Formatter Version
		pkt->Data[GUS_UVER] = dtInfo->uVersion; // Unit Version

		flg = tq_SetEndFlags(drv);
	} else {
		sts = ST_OFL; // Offline
		flg = 0;
	}

	tq_SetResults(pkt, cmd | OP_END, flg, sts, GUS_LNT_T, UQ_TYP_SEQ);

	// Ok, Send a response packet back to host.
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 04 - Set Controller Characteristics Command
int tq_MscpSetCtlrChar(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 wq)
{
	uint32 sts, cmd, tmo;

	if (pkt->Data[SCC_MSV]) {
		sts = ST_CMD | I_VRSN;
		cmd = 0;
	} else {
		// Set up controller flags and host timeout.
		tq->cFlags = (tq->cFlags & CF_RPL) | pkt->Data[SCC_CFLG];
		if (tmo = pkt->Data[SCC_TMO])
			tq->hstTimeout = tmo + 2;

		// Set up a new response packet.
		pkt->Data[SCC_CFLG] = tq->cFlags;
		pkt->Data[SCC_TMO]  = TQ_DCTMO;
		pkt->Data[SCC_VER]  = tq->cVersion;
		pkt->Data[SCC_CIDA] = 0;
		pkt->Data[SCC_CIDB] = 0;
		pkt->Data[SCC_CIDC] = 0;
		pkt->Data[SCC_CIDD] = (TQ_CLASS << SCC_CIDD_P_CLS) |
			(tq->idCtlr << SCC_CIDD_P_MOD);
		UQ_PUTP32(pkt, SCC_MBCL, TQ_MAXFR);

		// Successful operation.
		sts = ST_SUC;
		cmd = UQ_GETP(pkt, CMD_OPC, OPC) | OP_END;
	}

	// Set up results.
	tq_SetResults(pkt, cmd, 0, sts, SCC_LNT, UQ_TYP_SEQ);

	// Ok, Send a new response packet back to host.
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 08 - Available Command
int tq_MscpAvailable(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC); // MSCP Opcode
	uint32   mdf = pkt->Data[CMD_MOD];         // Modifier Flags
	uint32   lun = pkt->Data[CMD_UNIT];        // Logical Unit
	uint32   sts, flg;
	TQ_DRIVE *drv;

	if (drv = tq_GetUnit(tq->Drives, lun)) {
		if ((drv->Flags & DF_SXC) == 0) {
			// Set this drive as offline and unload a tape at request.
			drv->Flags    &= ~(DF_ONL|DF_POL|DF_TMK);
			drv->uFlags   &= ~(UF_WPH|UF_RMV);
			drv->cntObject = 0;
			if (drv->Flags & DF_ATT) {
				tq_Rewind(drv);
				if (mdf & MD_UNL)
					tq_Unload(drv);
				sts = ST_SUC;
			} else
				sts = ST_OFL|SB_OFL_NV;
		} else
			sts = ST_SXC;
		flg = tq_SetEndFlags(drv);
	} else {
		sts = ST_OFL; // Offline
		flg = 0;
	}

	tq_SetResults(pkt, cmd | OP_END, flg, sts, AVL_LNT, UQ_TYP_SEQ);

	// Ok, Send a response packet back to host.
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 09 - Online Command
int tq_MscpOnline(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];        // Logical Unit
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC); // MSCP Opcode
	uint32   sts, flg;
	TQ_DRIVE *drv;

	if (drv = tq_GetUnit(tq->Drives, lun)) {
		if ((drv->Flags & DF_ATT) == 0)
			sts = ST_OFL | SB_OFL_NV;
		else if (drv->Flags & DF_ONL)
			sts = ST_SUC | SB_SUC_ON;
		else {
			tq_Rewind(drv);
			drv->Flags     = (drv->Flags & ~(DF_TMK|DF_POL)) | DF_ONL;
			drv->cntObject = 0;
			tq_SetUnitFlags(drv, pkt);
			sts = ST_SUC;
		}

		// Set up a new response packet.
		pkt->Data[ONL_MLUN] = lun;
		pkt->Data[ONL_UFLG] = drv->uFlags;
		pkt->Data[ONL_RSVL] = 0;
		pkt->Data[ONL_RSVH] = 0;
		pkt->Data[ONL_UIDA] = lun;
		pkt->Data[ONL_UIDB] = 0;
		pkt->Data[ONL_UIDC] = 0;
		pkt->Data[ONL_UIDD] = (CLS_TAPE << ONL_UIDD_P_CLS) |
			(drv->dtInfo->idUnit << ONL_UIDD_P_MOD);
		pkt->Data[ONL_VSNL] = 01000 + lun;
		pkt->Data[ONL_VSNH] = 0;
		UQ_PUTP32(pkt, ONL_MEDL, drv->dtInfo->idMedia);

		flg = tq_SetEndFlags(drv);
	} else {
		sts = ST_OFL; // Offline
		flg = 0;
	}

	tq_SetResults(pkt, cmd | OP_END, flg, sts, ONL_LNT, UQ_TYP_SEQ);

	// Ok, Send a response packet back to host.
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 0A - Set Unit Characteristics
int tq_MscpSetUnitChar(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	TQ_DRIVE *drv;
	TQ_DTYPE *dtInfo;
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts;

	if (drv = tq_GetUnit(tq->Drives, lun)) {
		if ((drv->Flags & DF_ATT) == 0)
			sts = ST_OFL | SB_OFL_NV;
		else {
			sts = ST_SUC;
			tq_SetUnitFlags(drv, pkt);
		}

		dtInfo = drv->dtInfo;
		pkt->Data[SUC_MLUN] = lun;
		pkt->Data[SUC_UFLG] = drv->uFlags;
		pkt->Data[SUC_RSVL] = 0;
		pkt->Data[SUC_RSVH] = 0;
		pkt->Data[SUC_UIDA] = lun;
		pkt->Data[SUC_UIDB] = 0;
		pkt->Data[SUC_UIDC] = 0;
		pkt->Data[SUC_UIDD] = (CLS_TAPE << SUC_UIDD_P_CLS) |
			(dtInfo->idUnit << SUC_UIDD_P_MOD);
		pkt->Data[SUC_FMT]  = dtInfo->Format;
		pkt->Data[SUC_SPD]  = 0;
		pkt->Data[SUC_NREC] = 0;
		pkt->Data[SUC_RSVE] = 0;
		UQ_PUTP32(pkt, SUC_MEDL, dtInfo->idMedia);
		UQ_PUTP32(pkt, SUC_MAXL, TQ_MAXFR);
	} else
		sts = ST_OFL; // Offline

	// Ok, send a response packet back to host.
	tq_SetResults(pkt, cmd | OP_END, 0, sts, SUC_LNT, UQ_TYP_SEQ);
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 12 - Erase Command
// 16 - Erase Gap Command
int tq_MscpErase(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts, flg;
	TQ_DRIVE *drv;
	
	if (drv = tq_GetUnit(tq->Drives, lun)) {
		if ((sts = tq_CheckValid(drv, pkt, cmd)) == 0) {
			// Upon all data met valid, set up a working
			// packet with initial working data.
			drv->pktWork = pkt;

			// Enter timer queue now
			ts10_SetTimer(&drv->xfrTimer);
			return UQ_OK;
		}
		flg = tq_SetEndFlags(drv);
	} else {
		sts = ST_OFL; // Offline
		flg = 0;
	}

	// Set up a new response packet.
	tq_SetResults(pkt, cmd | OP_END, flg, sts, WTM_LNT, UQ_TYP_SEQ);

	// Ok, send a response packet back to host and return.
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 13 - Flush Command
int tq_MscpFlush(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts, flg;
	TQ_DRIVE *drv;
	
	if (drv = tq_GetUnit(tq->Drives, lun)) {
		sts = tq_CheckValid(drv, pkt, cmd);
		flg = tq_SetEndFlags(drv);
	} else {
		sts = ST_OFL; // Offline
		flg = 0;
	}

	// Set up a new response packet.
	tq_SetResults(pkt, cmd | OP_END, flg, sts, FLU_LNT, UQ_TYP_SEQ);

	// Ok, send a response packet back to host and return.
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 24 - Write Tape Mark Command
int tq_MscpWriteMark(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts, flg;
	TQ_DRIVE *drv;
	
	if (drv = tq_GetUnit(tq->Drives, lun)) {
		if ((sts = tq_CheckValid(drv, pkt, cmd)) == 0) {
			// Upon all data met valid, set up a working
			// packet with initial working data.
			drv->pktWork = pkt;

			// Enter timer queue now
			ts10_SetTimer(&drv->xfrTimer);
			return UQ_OK;
		}
		flg = tq_SetEndFlags(drv);
	} else {
		sts = ST_OFL; // Offline
		flg = 0;
	}

	// Set up a new response packet.
	UQ_PUTP32(pkt, WTM_POSL, drv->cntObject);
	tq_SetResults(pkt, cmd | OP_END, flg, sts, WTM_LNT, UQ_TYP_SEQ);

	// Ok, send a response packet back to host and return.
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// 25 - Position Command
int tq_MscpPosition(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   sts, flg;
	TQ_DRIVE *drv;
	
	if (drv = tq_GetUnit(tq->Drives, lun)) {
		if ((sts = tq_CheckValid(drv, pkt, cmd)) == 0) {
			// When all data met valid, enqueue
			// data transfer requests to being
			// processed later.
			drv->pktWork = pkt;
			ts10_SetTimer(&drv->xfrTimer);
			return UQ_OK;
		}
		flg = tq_SetEndFlags(drv);
	} else {
		sts = ST_OFL; // Offline
		flg = 0;
	}

	// Set up a new response packet.
	UQ_PUTP32(pkt, POS_RCL,  0);
	UQ_PUTP32(pkt, POS_TMCL, 0);
	UQ_PUTP32(pkt, POS_POSL, drv->cntObject);
	tq_SetResults(pkt, cmd | OP_END, flg, sts, POS_LNT, UQ_TYP_SEQ);

	// Ok, send a response packet back to host and return.
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

// Read/Write Data Accesses
int tq_MscpData(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   bc  = UQ_GETP32(pkt, RW_BCL);
	uint32   sts, flg;
	TQ_DRIVE *drv;
	
	if (drv = tq_GetUnit(tq->Drives, lun)) {
		if ((sts = tq_CheckValid(drv, pkt, cmd)) == 0) {
			if ((bc == 0) || (bc > TQ_MAXFR)) {
				drv->Flags |= DF_SXC;
				sts = ST_CMD|I_BCNT;
			} else {
				// When all data met valid, enqueue
				// data transfer requests to being
				// processed later.
				drv->pktWork = pkt;
				ts10_SetTimer(&drv->xfrTimer);
				return UQ_OK;
			}
		}
		flg = tq_SetEndFlags(drv);
	} else {
		sts = ST_OFL; // Offline
		flg = 0;
	}

	// Set up a new response packet.
	UQ_PUTP32(pkt, RW_BCL,  0);
	UQ_PUTP32(pkt, RW_POSL, drv->cntObject);
	UQ_PUTP32(pkt, RW_RSZL, 0);
	tq_SetResults(pkt, cmd | OP_END, flg, sts, RW_TAP_LNT, UQ_TYP_SEQ);

	// Ok, send a response packet back to host and return.
	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

int tq_ProcessTMSCP(TQ_DEVICE *tq, TQ_PACKET *pkt, uint32 q)
{
	TQ_DRIVE *drv;
	uint32   cmd = UQ_GETP(pkt, CMD_OPC, OPC);
	uint32   flg = UQ_GETP(pkt, CMD_OPC, FLG);
	uint32   mdf = pkt->Data[CMD_MOD];
	uint32   lun = pkt->Data[CMD_UNIT];
	uint32   sts = ST_SUC;

	if ((cmd >= 64) || (tq_CmdFlags[cmd] == 0)) {
		cmd |= OP_END;        // Set End Flag
		sts  = ST_CMD|I_OPCD; // Invalid opcode
	} else if (flg) {
		cmd |= OP_END;        // Set End Flag
		sts  = ST_CMD|I_FLAG; // Illegal flags
	} else if (mdf & ~tq_CmdFlags[cmd]) {
		cmd |= OP_END;        // Set End Flag
		sts  = ST_CMD|I_MODF; // Illegal modifiers
	} else {
		if (drv = tq_GetUnit(tq->Drives, lun)) {
			// If drive is in use, enter this packet
			// into its queue to being processed later.
			if (q && (tq_CmdFlags[cmd] & CMF_SEQ)) {
				if (drv->pktWork || drv->pktQueue) {
					tq_Enqueue(&drv->pktQueue, pkt, TQ_QTAIL);
					return UQ_OK;
				}
			}
			// Clear Serious Exception
			if (tq_CmdFlags[cmd] & MD_CSE)
				drv->Flags &= ~DF_SXC;
		}

		switch (cmd) {
			case OP_ABO: // 01 - Abort Command
				return tq_MscpAbort(tq, pkt, q);
			case OP_GCS: // 02 - Get Command Status
				return tq_MscpGetCommandStatus(tq, pkt, q);
			case OP_GUS: // 03 - Get Unit Status
				return tq_MscpGetUnitStatus(tq, pkt, q);
			case OP_SCC: // 04 - Set Controller Characteristics
				return tq_MscpSetCtlrChar(tq, pkt, q);
			case OP_AVL: // 08 - Available Command
				return tq_MscpAvailable(tq, pkt, q);
			case OP_ONL: // 09 - Online Command
				return tq_MscpOnline(tq, pkt, q);
			case OP_SUC: // 0A - Set Unit Characteristics
				return tq_MscpSetUnitChar(tq, pkt, q);
			case OP_ERS: // 12 - Erase Command
			case OP_ERG: // 16 - Erase Gap Command
				return tq_MscpErase(tq, pkt, q);
			case OP_FLU: // 13 - Flush Command
				return tq_MscpFlush(tq, pkt, q);
			case OP_ACC: // 10 - Access Command
			case OP_CHD: // 20 - Compare Host Data Command
			case OP_RD:  // 21 - Read Command
			case OP_WR:  // 22 - Write Command
				return tq_MscpData(tq, pkt, q);
			case OP_WTM: // 24 - Write Tape Mark Command
				return tq_MscpWriteMark(tq, pkt, q);
			case OP_POS: // 25 - Position Command
				return tq_MscpPosition(tq, pkt, q);
			case OP_DAP: // 0B - Determine Access Paths
				cmd |= OP_END; // No Operations
				sts = ST_SUC;
				break;
			default:
				cmd = OP_END;           // Set End Flag
				sts = ST_CMD | I_OPCD;  // Invalid Opcode
		}
	}

	// Set up response packet
	tq_SetResults(pkt, cmd, 0, sts, RSP_LNT, UQ_TYP_SEQ);

	return tq_PutPacket(tq, pkt, TQ_QTAIL);
}

void tq_QueService(void *dptr)
{
	TQ_DEVICE *tq = (TQ_DEVICE *)dptr;
	TQ_DRIVE  *drv;
	TQ_PACKET *pkt = NULL;
	uint16    idConn;
	uint32    idx;

	// Initialization Steps here.

	// First, check and process any packets that had been queued.
	for (idx = 0; idx < tq->nDrives; idx++) {
		drv = &tq->Drives[idx];
		if (drv->Flags & DF_EXIST) {
			// Now process next MSCP commands if available.
			if (drv->pktWork || (drv->pktQueue == NULL))
				continue;
			pkt = tq_Dequeue(&drv->pktQueue, TQ_QHEAD);
			if (tq_ProcessTMSCP(tq, pkt, FALSE))
				return;
		}
	}

	// When all packet queues are finished, now poll
	// new MSCP packets to being processed or queued
	// for services.
	if ((pkt == NULL) && (tq->Flags & CFLG_POLL)) {
		if (tq_GetPacket(tq, &pkt))
			return;
		if (pkt) {
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA)) {
				dbg_Printf("%s: TMSCP Cmd=%04X Mode=%04X Unit=%d\n",
					tq->Unit.devName, pkt->Data[CMD_OPC], pkt->Data[CMD_MOD],
					pkt->Data[CMD_UNIT]);
			}
#endif /* DEBUG */

			// Make ensure that packet is sequence type. If not, die now.
			if (UQ_GETP(pkt, UQ_CTC, TYP) != UQ_TYP_SEQ) {
				tq_SetFatalError(tq, ER_PIE);
				return;
			}

			// Now process a packet at all time.
			idConn = UQ_GETP(pkt, UQ_CTC, CID);
			if (idConn == UQ_CID_TMSCP) {
				// TMSCP command processing
				if (tq_ProcessTMSCP(tq, pkt, TRUE))
					return;
			} else if (idConn == UQ_CID_DUP) {
				// DUP protocol is not supported at this time.
				tq_SetResults(pkt, OP_END, 0, ST_CMD|I_OPCD, RSP_LNT, UQ_TYP_SEQ);
				if (tq_PutPacket(tq, pkt, TQ_QTAIL))
					return;
			} else {
				// Other protocols are not supported, die now.
				tq_SetFatalError(tq, ER_ICI);
				return;
			}
		} else {
			// After all packets were processed,
			// discontinue poll state.
			tq->Flags &= ~CFLG_POLL;
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: End of Poll.\n", tq->Unit.devName);
#endif /* DEBUG */
		}
	}

	// Send next response that are remining in queues.
	if (tq->pktResp) {
		pkt = tq_Dequeue(&tq->pktResp, TQ_QHEAD);
		if (tq_PutPacket(tq, pkt, TQ_QHEAD))
			return;
	}
	
	if (pkt) ts10_SetTimer(&tq->queTimer);
}

// ********************************************************************

int tq_Timer(void *dptr)
{
	TQ_DEVICE *tq = (TQ_DEVICE *)dptr;

	// Host timeout? 
	if ((tq->hstTimer > 0) && (--tq->hstTimer == 0))
		tq_SetFatalError(tq, ER_HAT);

	return UQ_OK;
}

// *********************************************************************

// Reset entire controller
void tq_ResetDevice(TQ_DEVICE *tq)
{
	TQ_DRIVE  *drv;
	UQ_RING   *resRing = &tq->resRing;
	UQ_RING   *cmdRing = &tq->cmdRing;
	uint32    idx;

	// Put controller in reset state.
	tq->Flags |= CFLG_RESET;

	// Cancel all queue timers first.
	ts10_CancelTimer(&tq->queTimer);

	// Reset all drives first.
	for (idx = 0; idx < tq->nDrives; idx++) {
		drv = &tq->Drives[idx];
		ts10_CancelTimer(&drv->xfrTimer);
		drv->Flags    &= ~DF_ONL;
		if (drv->dtInfo)
			drv->uFlags = drv->dtInfo->uFlags;
		drv->pktWork  = NULL;
		drv->pktQueue = NULL;
	}

	// Clear all ring queues and free all packets.
	memset(resRing, 0, sizeof(UQ_RING));
	memset(cmdRing, 0, sizeof(UQ_RING));
	memset(&tq->pktList, 0, sizeof(TQ_PACKET) * TQ_NPKTS);

	resRing->Name = "Response Descriptor"; // Name of Response Descriptor
	cmdRing->Name = "Command Descriptor";  // Name of Command Descriptor

	// Initialize internal packet queue.
	for (idx = 0; idx < TQ_NPKTS; idx++) {
		tq->pktList[idx].Next =
			((idx + 1) < TQ_NPKTS) ? &tq->pktList[idx + 1] : NULL;
		tq->pktList[idx].idPacket = idx;
	}
	tq->pktFree = &tq->pktList[0];
	tq->pktResp = NULL;
	tq->pktBusy = 0;

	// Reset all controller data
	tq->cFlags     = CF_RPL;
	tq->Credits    = (TQ_NPKTS / 2) - 1;
	tq->xfrTimer   = TQ_XTIMER;
	tq->hstTimeout = TQ_DHTMO+1;
	tq->hstTimer   = tq->hstTimeout;

	// Return back to initialization step #1
	S1C = SA = (SA_S1|SA_S1C_QB|SA_S1C_DI|SA_S1C_MP);
	tq->State = CST_S1;

	// Finally reset all flags.
	tq->Flags = 0; 
}

// Transition to Intiialization Step #4
int tq_Step4(TQ_DEVICE *tq)
{
	MAP_IO   *io      = &tq->ioMap;
	UQ_CALL  *call    = tq->Callback; 
	UQ_RING  *resRing = &tq->resRing;
	UQ_RING  *cmdRing = &tq->cmdRing;
	uint32   base, len, idx;
	uint16   zero[CA_MAX];

	// Initialize Communication Region Area
	resRing->intAddr  = tq->rbAddr + CA_RI;
	resRing->baseAddr = tq->rbAddr;
	resRing->szDesc   = GET_RQ(S1H);
	resRing->mskDesc  = resRing->szDesc - 1;
	resRing->idxDesc  = 0;

	cmdRing->intAddr  = tq->rbAddr + CA_CI;
	cmdRing->baseAddr = tq->rbAddr + (resRing->szDesc << 2);
	cmdRing->szDesc   = GET_CQ(S1H);
	cmdRing->mskDesc  = cmdRing->szDesc - 1;
	cmdRing->idxDesc  = 0;

	base = tq->rbAddr + ((tq->Flags & CFLG_PI) ? CA_QQ : CA_CI);
	len  = ((resRing->szDesc + cmdRing->szDesc) << 2) + (tq->rbAddr - base);
	if (len > CA_MAX)
		len = CA_MAX;

	for (idx = 0; idx < (len >> 1); idx++)
		zero[idx] = 0;
	if (call->WriteBlock(tq->System, base, (uint8 *)zero, len, 0))
		return tq_SetFatalError(tq, ER_QWE);

	S4C = SA = SA_S4 | (tq->idPort << SA_S4C_P_MOD) |
		((tq->cVersion & 0xFF) << SA_S4C_P_VER);
	tq->State = CST_S4;
	TQ_DOINTI; // Send an interrupt if enabled
}

int tq_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 size)
{
	TQ_DEVICE *tq = (TQ_DEVICE *)dptr;
	uint32    reg = (pAddr - (tq->csrAddr & 0x1FFF)) >> 1;

	switch (reg) {
		case UQ_REG_IP: // Initialization and Poll Register
			if (tq->State == CST_S3_PPB)
				tq_Step4(tq);
			else if (tq->State == CST_UP) {
				tq->Flags |= CFLG_POLL;
				ts10_SetTimer(&tq->queTimer);
			}
			*data = tq->ip;
			break;

		case UQ_REG_SA: // Status, Address, and Purge Register
			*data = tq->sa;
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) Unknown Register %d - PA=%08o D=%06o (%04X)\n",
					tq->Unit.devName, reg, pAddr, *data, *data);
#endif /* DEBUG */
			return UQ_NXM;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%08o) => %06o (hex %04X)\n",
			tq->Unit.devName, regName[reg], pAddr, *data, *data);
#endif /* DEBUG */

	return UQ_OK;
}

int tq_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 size)
{
	TQ_DEVICE *tq = (TQ_DEVICE *)dptr;
	MAP_IO    *io = &tq->ioMap;
	uint32    reg = (pAddr - (tq->csrAddr & 0x1FFF)) >> 1;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS) && (reg < UQ_NREGS))
		dbg_Printf("%s: (W) %s (%08o) <= %06o (hex %04X)\n",
			tq->Unit.devName, regName[reg], pAddr, data, data);
#endif /* DEBUG */

	if (reg == UQ_REG_IP) {
		// Initialization and Poll Register
		tq_ResetDevice(tq);
	} else if (reg == UQ_REG_SA) {
		// Status, Address, and Purge Register

		switch (tq->State) {
			case CST_S1:
				// Initialization Step #1
				if (data & SA_S1H_VL) {
					if (data & SA_S1H_WR) {
						SA = data;
						tq->State = CST_S1_WRAP;
					} else {
						// Transition to Step #2
						S1H = data;
						S2C = SA = (SA_S2|SA_S2C_PT) | GET_S1H(S1H);
						tq->State = CST_S2;

						// Enable interrupts if request
						if (S1H & SA_S1H_IE)
							tq->Flags |= CFLG_IE;
						if (tq->intVector = (data & SA_S1H_VE) << 2)
							io->SetVector(io, tq->intVector, 0);
						TQ_DOINTI; // Send an interrupt if enabled
					}
				}
				break;

			case CST_S1_WRAP:
				// Wrap Mode
				SA = data;
				break;

			case CST_S2:
				// Initialization Step #2
				tq->rbAddr = data & SA_S2H_CLO; // Comm Address Low
				if (data & SA_S2H_PI)           // Purge Interrupt Request
					tq->Flags |= CFLG_PI;

				// Transition to Step #3
				S2H = data;
				S3C = SA = SA_S3 | GET_S1L(S1H);
				tq->State = CST_S3;
				TQ_DOINTI; // Send an interrupt if enabled
				break;

			case CST_S3:
				// Initialization Step #3
				tq->rbAddr |= (((uint32)(data & SA_S3H_CHI)) << 16);
				if (data & SA_S3H_PP) {
					SA = 0;
					tq->State = CST_S3_PPA;
				} else 
					tq_Step4(tq);
				break;

			case CST_S3_PPA:
				if (data)
					tq_SetFatalError(tq, ER_PPF);
				else
					tq->State = CST_S3_PPB;
				break;

			case CST_S4:
				// Initialization Step #4
				if (data & SA_S4H_GO) {
					S4H = data;
					SA  = 0;
					tq->State = CST_UP;

#ifdef DEBUG
					if (dbg_Check(DBG_IOREGS))
						tq_DumpInitInfo(tq);
#endif /* DEBUG */

					// If last error occured, send last fail packet.
					if ((data & SA_S4H_LF) && tq->errCode)
						tq_SendLastFail(tq, tq->errCode);
					tq->errCode = 0;
				}
				break;
		}
		
	} else {
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (W) Unknown Register %d - PA=%08o D=%06o (%04X)\n",
				tq->Unit.devName, reg, pAddr, data, data);
#endif /* DEBUG */
		return UQ_NXM;
	}

	return UQ_OK;
}

// Bus Initialization from Host.
void tq_ResetIO(void *dptr)
{
	tq_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// *********************************************************************

DEVICE tq_Device;

void *tq_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	TQ_DEVICE *tq = NULL;
	CLK_QUEUE *newTimer;
	TQ_DRIVE  *drv;
	MAP_IO    *io;
	uint32    idx;

	if (tq = (TQ_DEVICE *)calloc(1, sizeof(TQ_DEVICE))) {
		// First, set up its descriptions and
		// link it to its mapping devices.
		tq->Unit.devName    = newMap->devName;
		tq->Unit.keyName    = newMap->keyName;
		tq->Unit.emuName    = newMap->emuName;
		tq->Unit.emuVersion = newMap->emuVersion;
		tq->Device          = newMap->devParent->Device;
		tq->Callback        = newMap->devParent->Callback;
		tq->System          = newMap->devParent->sysDevice;

		// Controller Flags
		tq->Flags      = 0;
		tq->csrAddr    = TQ_CSRADR;
		tq->intVector  = 0;

		// Create data buffer for data transfers.
		tq->bufData = (uint8 *)calloc(1, TQ_MAXFR);

		// Set up the new queue timer.
		newTimer           = &tq->queTimer;
		newTimer->Name     = "TMSCP Queue Timer";
		newTimer->Next     = NULL;
		newTimer->Flags    = 0;
		newTimer->outTimer = TQ_QTIMER;
		newTimer->nxtTimer = TQ_QTIMER;
		newTimer->Device   = (void *)tq;
		newTimer->Execute  = tq_QueService;

		// Create drives on depending controller.
		tq->nDrives = TQ_NDRVS;
		tq->Drives  =
			(TQ_DRIVE *)calloc(tq->nDrives, sizeof(TQ_DRIVE));
		for (idx = 0; idx < tq->nDrives; idx++) {
			drv = &tq->Drives[idx];
			drv->uFlags   = 0;
			drv->idUnit   = idx;
			drv->tqDevice = tq;

			// Set up the new response timer.
			newTimer           = &drv->xfrTimer;
			newTimer->Name     = "TMSCP Transfer Timer";
			newTimer->Next     = NULL;
			newTimer->Flags    = 0;
			newTimer->outTimer = TQ_XTIMER;
			newTimer->nxtTimer = TQ_XTIMER;
			newTimer->Device   = drv;
			newTimer->Execute  = tq_Service;
		}

		// Set up I/O Entry
		io               = &tq->ioMap;
		io->devName      = tq->Unit.devName;
		io->keyName      = tq->Unit.keyName;
		io->emuName      = tq->Unit.emuName;
		io->emuVersion   = tq->Unit.emuVersion;
		io->Device       = tq;
		io->csrAddr      = tq->csrAddr;
		io->nRegs        = TQ_NREGS;
		io->intIPL       = TQ_IPL;
		io->nVectors     = TQ_NVECS;
		io->intVector[0] = 0;
		io->ReadIO       = tq_ReadIO;
		io->WriteIO      = tq_WriteIO;
		io->ResetIO      = tq_ResetIO;

		// Assign that registers to QBA's I/O Space.
		tq->Callback->SetMap(tq->Device, io);
		tq_ResetDevice(tq);

		// Finally, link it to its mapping device.
		newMap->Device = tq;
	}

	return tq;
}

void *tq_Configure(MAP_DEVICE *newMap, int argc, char **argv)
{
	TQ_DEVICE *tq  = (TQ_DEVICE *)newMap->devParent->Device;
	TQ_DRIVE  *drv = NULL;
	uint32    unit, idx;

	unit = GetDeviceUnit(newMap->devName);
	if (unit < tq->nDrives) {

		drv = &tq->Drives[unit];

		// Check if unit is existing or not first.
		if (drv->Flags & DF_EXIST) {
			printf("%s: Already created on %s:\n",
				newMap->devName, tq->Unit.devName);
			return NULL;
		}

		for (idx = 0; tq_Drives[idx].keyName; idx++)
			if (!strcasecmp(argv[2], tq_Drives[idx].keyName))
				break;

		if (tq_Drives[idx].keyName == NULL) {
			printf("%s: No such device type on %s:\n",
				argv[2], newMap->devName);
			return NULL;
		}

		if (tq->dtInfo == NULL) {
			// Set controller type for first time only.
			tq->dtInfo   = &tq_Drives[idx];
			tq->idPort   = tq_Drives[idx].idPort;
			tq->idCtlr   = tq_Drives[idx].idCtlr;
			tq->cVersion = tq_Drives[idx].cVersion;
		} else if (tq->dtInfo != &tq_Drives[idx]) {
			printf("%s: Invalid device type, must be %s\n",
				argv[2], tq->dtInfo->keyName);
			return NULL;
		}

		drv->Unit.devName = newMap->devName;
		drv->Unit.keyName = tq_Drives[idx].keyName;
		drv->Unit.emuName = tq_Drives[idx].devDesc;
		drv->idMedia      = tq_Drives[idx].idMedia;
		drv->Flags        = DF_EXIST;
		drv->uFlags       = tq_Drives[idx].uFlags;
		drv->dtInfo       = &tq_Drives[idx];

		newMap->idUnit     = drv->idUnit;
		newMap->keyName    = drv->Unit.keyName;
		newMap->emuName    = drv->Unit.emuName;
		newMap->emuVersion = "";
		newMap->devInfo    = &tq_Device;
		newMap->Device     = tq;
	} else {
		printf("%s: No such drive on %s: - (Only %d drives)\n",
			newMap->devName, tq->Unit.devName, tq->nDrives);
		return NULL;
	}

	// Send MSCP drive information.
	return drv;
}

int tq_Reset(void *dptr)
{
	TQ_DEVICE *tq = (TQ_DEVICE *)dptr;

	tq_ResetDevice(tq);
	return EMU_OK;
}

int tq_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	TQ_DEVICE *tq  = (TQ_DEVICE *)map->Device;
	TQ_DRIVE  *drv = &tq->Drives[map->idUnit];
	VMT_TAPE  *vmt = &drv->dpTape;
	uint32    mode = (drv->dtInfo->uFlags & UF_WPH) ? O_RDONLY : O_RDWR|O_CREAT;
	uint32    st;

	if (drv->Flags & DF_ATT) {
		printf("%s: Already attached. Please use 'detach %s:' first.\n",
			drv->Unit.devName, drv->Unit.devName);
		return EMU_OK;
	}

	// Set up a filename and format type.
	vmt->fileName = (char *)malloc(strlen(argv[2])+1);
	strcpy(vmt->fileName, argv[2]);
	vmt->fmtName  = NULL;

	// Debug Information Request
//	vmt->Flags    = VMT_DEBUG|VMT_DUMP;

	// Attempt to open a tape image/medium.
	if (st = vmt_OpenTape(vmt)) {
		printf("%s: File '%s' not attached, Reason: %d.\n",
			drv->Unit.devName, argv[2], st);
		free(vmt->fileName);
		vmt->fileName = NULL;
		return EMU_OPENERR;
	}

	// Set flags as attached status.
	drv->Flags |= DF_ATT;
	if (tq->State == CST_UP)
		drv->Flags |= DF_ATP;
	drv->uFlags = drv->dtInfo->uFlags;

	// Tell operator that and save it.
	printf("%s: File '%s' attached.\n", drv->Unit.devName, argv[2]);
	drv->fileName = vmt->fileName;

	return EMU_OK;
}

int tq_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	TQ_DEVICE *tq  = (TQ_DEVICE *)map->Device;
	TQ_DRIVE  *drv = &tq->Drives[map->idUnit];
	VMT_TAPE  *vmt = &drv->dpTape;

	if ((drv->Flags & DF_ATT) == 0) {
		printf("%s: Already detached.\n", drv->Unit.devName);
		return EMU_OK;
	}

	// Clear flags as detached status.
	drv->Flags  &= ~(DF_ATT|DF_WLK|DF_ONL|DF_ATP);
	drv->uFlags &= (UF_WPH|UF_RMV);
	vmt_CloseTape(vmt);

	// Tell operator that.
	printf("%s: File '%s' detached.\n", drv->Unit.devName,
		drv->fileName ? drv->fileName : "<Unknown filename>");

	// Clean up a mess.
	drv->fileName = NULL;
	if (vmt->fileName) {
		free(vmt->fileName);
		vmt->fileName = NULL;
	}
	vmt->fmtName = NULL;

	return EMU_OK;
}


void tq_InfoPacket(TQ_PACKET *pkt, cchar *pktName)
{
	char outBuffer[80], *str;
	int  idx1, idx2;

	printf("%s Packet #%d\n", pktName, pkt->idPacket);
	
	for (idx1 = 0; idx1 < TQ_PKT_WSIZE;) {
		str = ToRadix(outBuffer, idx1, 16, 4);
		*str++ = ' ';
		for (idx2 = 0; (idx1 < TQ_PKT_WSIZE) && (idx2 < 8); idx2++) {
			*str++ = ' ';
			str = ToRadix(str, (uint32)pkt->Data[idx1++], 16, 4);
		}
		*str++ = '\0';
		printf("%s\n", outBuffer);
	};
	printf("\n");
}

void tq_InfoQueue(TQ_PACKET *pktList, cchar *pktName, int32 flags)
{
	TQ_PACKET *pkt;
	int32     cnt = 0;

	if (pkt = pktList) {
		printf("%s Queue:\n", pktName);
		while (pkt) {
			if (flags) {
				tq_InfoPacket(pkt, pktName);
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
int tq_Info(MAP_DEVICE *map, int argc, char **argv)
{
	TQ_DEVICE *tq = (TQ_DEVICE *)map->Device;
	TQ_DRIVE  *drv;
	TQ_PACKET *pkt;
	int       idx, count = 0;

	// Information about MSCP controller
	printf("\nDevice:           %s  Type: %s  State: %s\n",
		map->devName, map->keyName, cstName[tq->State]);
	printf("CSR Address:      %06X (%08o)\n", tq->csrAddr, tq->csrAddr);
	printf("Interrupt Vector: %03X (%03o)\n", tq->intVector, tq->intVector);
	printf("Ringbase Address: %06X (%08o)\n", tq->rbAddr, tq->rbAddr);
	printf("Descriptors:      %d Command Slots, %d Response Slots\n\n",
		&tq->cmdRing.szDesc, &tq->resRing.szDesc);
	printf("Status:           %s\n\n",
		(tq->Flags & CFLG_POLL) ? "Polling in progress" : "Idle");

	// Information about MSCP drives
	for (idx = 0; idx < tq->nDrives; idx++) {
		drv = &tq->Drives[idx];
		if (drv->Flags & DF_EXIST) {
			if (count++ == 0) {
				printf(
					"Device   Type     Media ID Description\n"
					"------   ----     -------- -----------\n"
				);
			}
			printf("%-8s %-8s %-8s %s\n",
				drv->Unit.devName, drv->Unit.keyName,
				tq_GetMediaID(drv->idMedia), drv->Unit.emuName);
		}
	}

	if (count)
		printf("\nTotal %d drives.\n\n", count);
	else
		printf("\nNo drives.\n\n");

	printf("Packet Queues:\n\n");
	if (tq->State == CST_UP) {
		for (idx = 0; idx < tq->nDrives; idx++) {
			drv = &tq->Drives[idx];

			if ((drv->Flags & DF_EXIST) == 0)
				continue;
			if (drv->pktWork) {
				printf("%s: In Progress\n", drv->Unit.devName);
				tq_InfoPacket(drv->pktWork, "Current");
			} else
				printf("%s: Idle\n", drv->Unit.devName);
			tq_InfoQueue(drv->pktQueue, "Queue", 1);
		}

		// Information about response packet queues.
		tq_InfoQueue(tq->pktResp, "Response", 1);
		tq_InfoQueue(tq->pktFree, "Free",     0);
	} else
		printf("Controller is not initialized.\n");

	return EMU_OK;
}

DEVICE tq_Device =
{
	TQ_KEY,           // Device Type Name
	TQ_NAME,          // Emulator Name
	TQ_VERSION,       // Emulator Version
	NULL,             // Listing of Devices
	DF_SYSMAP,        // Device Flags
	DT_DEVICE,        // Device Type

	NULL, NULL, NULL,

	tq_Create,        // Create Routine
	tq_Configure,     // Configure Routine
	NULL,             // Delete Routine
	tq_Reset,         // Reset Routine
	tq_Attach,        // Attach Routine  - Not Used
	tq_Detach,        // Detach Routine  - Not Used
	tq_Info,          // Info Routine
	NULL,             // Boot Routine    - Not Used
	NULL,             // Execute Routine - Not Used
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
