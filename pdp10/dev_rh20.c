// rh20.c - RH20 MASSBUS Interface Support Routines
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

// Device Name: RH20 (8 Channels)
// Device Code: 540,544,550,554,560,564,570,574

#include "pdp10/defs.h"
#include "pdp10/kl10.h"
#include "pdp10/proto.h"
#include "pdp10/rh20.h"

// Channel Logout Area in Executive Process Table.
extern int30 KL10_eptAddr;

void rh20_Initialize(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	rh20->srFlags = CI_CNR;
}

inline void rh20_ResetDevice(RH20_DEVICE *rh)
{
	int idx;

	rh->srFlags  = CI_CNR;
	rh->Flags    = 0;

	rh->Register = 0;
	rh->Drive    = 0;
	rh->Data     = 0;

	// Clear all internal registers
	rh->stcAddr  = 0;
	rh->stcFlags = 0;
	rh->ptcAddr  = 0;
	rh->ptcFlags = 0;
	rh->iviAddr  = 0;

	// Clear all existing MASSBUS drives.
	for (idx = 0; idx < RH20_MAXUNITS; idx++) {
		MBA_DRIVE *drv = &rh->mba.Drive[idx];
		if (drv->Flags < 0)
			drv->dInfo->ResetDrive(drv);
	}
	
	// Default first MASSBUS drive.
	rh->mbaDrive = &rh->mba.Drive[0];
}

void rh20_DoInterrupt(RH20_DEVICE *rh)
{
	if (rh->piLevel > 0) {
		int36 iopWord = IRQ_VEC | (rh->idChannel << IRQ_P_DEV) |
			rh->iviAddr;
#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: Interrupt Word: %06o,,%06o at PI level %d\n",
				rh->Unit.devName, LH18(iopWord), RH18(iopWord), rh->piLevel);
#endif /* DEBUG */

		// Tell KL10 Processor to process it.
		KL10pi_RequestIO(rh->piLevel, iopWord);
		rh->piRequest = 0;
		return;
	}

	rh->piRequest = 1;
#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: PI Level %d, No Interrupt Signal.\n",
			rh->Unit.devName, rh->piLevel);
#endif /* DEBUG */
}

// Check any interrupt bits, if so, send an
// interrupt signal to PDP-10 processor.
inline void rh20_CheckPI(RH20_DEVICE *rh)
{
	if ((rh->srFlags & CI_CMD) ||
		 ((rh->srFlags & CI_ATN) && (rh->srFlags & CI_AIE)))
		rh20_DoInterrupt(rh);
}

void rh20_SetAttention(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	rh20->srFlags |= CI_ATN;
	if (rh20->srFlags & CI_AIE) {
#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: Attention Interrupt\n", rh20->Unit.devName);
#endif /* DEBUG */
		rh20_DoInterrupt(rh20);
	} else
		rh20->piRequest = 1;
}

void rh20_SetReady(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	rh20->srFlags |= CI_CMD;
#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Command Done Interrupt\n", rh20->Unit.devName);
#endif /* DEBUG */
	rh20_DoInterrupt(rh20);
}

// Reset Command List Pointer
void rh20_ResetCCW(register RH20_DEVICE *rh)
{
	// Reset Command List Pointer
	rh->clpAddr = KL10_eptAddr + (rh->idChannel << 2) + CSICW;

	// Clear all CCW variables.
	rh->ccwStatus = 0;
	rh->ccwOpcode = 0;
	rh->ccwAddr   = 0;
	rh->ccwCount  = 0;
}

// Next Command List Pointer
int rh20_GetNextCCW(register RH20_DEVICE *rh)
{
	uint36 ccw;
	uint32 ccwOpcode;
	uint32 ccwCount;
	uint32 ccwAddr;

	for(;;) {
		ccw = p10_pRead(rh->clpAddr, 0);

		// Extract fields from a ccw word.
		ccwOpcode = (uint32)(ccw >> CWP_OPC) & CWM_OPC;
		ccwCount  = (uint32)(ccw >> CWP_WDC) & CWM_WDC;
		ccwAddr   = (uint32)ccw & CWM_ADR;

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("%s: %08o - Opcode: %o  Word Count: %04o  Address: %08o\n",
				rh->Unit.devName, rh->clpAddr, ccwOpcode, ccwCount, ccwAddr);
#endif /* DEBUG */

		switch(ccwOpcode) {
			case CWOP_HALT:
				return RH20_STOP; // Stop

			case CWOP_JUMP:
				rh->clpAddr = ccwAddr;
				continue;

			default:
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: Unimplemented CCW Opcode: %o\n",
						rh->Unit.devName, ccwOpcode);
#endif /* DEBUG */

#if 0
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: Bad CCW Opcode: %o\n",
						rh->Unit.devName, ccwOpcode);
#endif /* DEBUG */
				return RH20_ERROR;
#endif
		}

		rh->ccwOpcode = ccwOpcode;
		rh->ccwAddr   = ccwAddr;
		rh->ccwCount  = ccwCount;
		rh->clpAddr++;

		return RH20_OK;
	}
}

// Prepare/begin block transfers
int rh20_BeginIO(void *dptr)
{
	register RH20_DEVICE *rh = (RH20_DEVICE *)dptr;

	// Check if CCW word is good or not.
	if (rh20_GetNextCCW(rh)) {
		// No, bad CCW word or HALT opcode.
		rh->srFlags |= CI_CMD|CI_CER;
		rh20_DoInterrupt(rh);
		return MBA_ERROR;
	}

	// Now data channel is busy so that data transfers begin.
	rh->srFlags &= ~CI_CNR;

	return MBA_OK;
}

// Finalize block transfers
void rh20_EndIO(void *dptr)
{
	register RH20_DEVICE *rh = (RH20_DEVICE *)dptr;
	int30 clpAddr; // Command List Pointer Address
	int30 dbaAddr; // Data Buffer Address
	int36 clp;     // Status/CLP Word
	int36 ccw;

	// Update status/command list pointer
	clp = rh->clpAddr | (CLP_SET|CLP_NAE);
	if (rh->blkCount || rh->ccwCount) {
		clp |= (rh->ccwCount) ? (CLP_NW0|CLP_LWC) : CLP_SWC;
		rh->srFlags |= CI_CER; // Channel Error
	}

	// If store channel status request or any channel errors,
	// store channel status word into logout area.
	if ((rh->ptcFlags & TC_SCS) || (rh->srFlags & CI_CER)) {
		// Update current control word
		ccw = ((int36)(rh->ccwOpcode & CWM_OPC) << CWP_OPC) |
		      ((int36)(rh->ccwCount & CWM_WDC) << CWP_WDC) |
	   	   (int36)(rh->ccwAddr & CWM_ADR);

		clpAddr = KL10_eptAddr + CSCLP + (rh->idChannel << 2);
		dbaAddr = KL10_eptAddr + CSDBA + (rh->idChannel << 2);
		p10_pWrite(clpAddr, clp, 0);
		p10_pWrite(dbaAddr, ccw, 0);

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA)) {
			dbg_Printf("%s: Store Channel Status: %s\n", rh->Unit.devName,
				(rh->srFlags & CI_CER) ? "Channel Error" :
				(rh->ptcFlags & TC_SCS) ? "Requested" : "<Unknown>");
			dbg_Printf("%s: CSCLP (%08o) <= %06o,,%06o\n",
				rh->Unit.devName, clpAddr, LH18(clp), RH18(clp));
			dbg_Printf("%s: CSDBA (%08o) <= %06o,,%06o\n",
				rh->Unit.devName, dbaAddr, LH18(ccw), RH18(ccw));
		}
#endif /* DEBUG */
	}
	
	// Set command done flag and return.
	rh->srFlags &= ~CI_PCR;
	rh->srFlags |= CI_CMD|CI_CNR;
	rh20_DoInterrupt(rh);
}

int rh20_ReadBlock(void *dptr, uint8 *data, uint32 blkSize, uint32 mode)
{
	register RH20_DEVICE *rh = (RH20_DEVICE *)dptr;
	uint32  *blkData = (uint32 *)data;
	int36   data36; // Data buffer
	int     idx;

	for (idx = 0; idx < blkSize; idx += 2) {
		if (rh->ccwCount == 0) {
			if (rh20_GetNextCCW(rh))
				break;

			// Throw-away CCW
			if (rh->ccwAddr == 0) {
				idx += (rh->ccwCount << 1);
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: (R) Throw-Away CCW - %04o (%d) words skipped.\n",
						rh->Unit.devName, rh->ccwCount, rh->ccwCount);
#endif /* DEBUG */
				rh->ccwCount = 0;
				continue;
			}
		}

		data36 = p10_pRead(rh->ccwAddr, 0);

		if (rh->ccwOpcode & CWB_REV) {
			// Reverse Data
			blkData[idx+1] = LH18(data36);
			blkData[idx]   = RH18(data36);
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: (R) %08o (%04o) => %06o,,%06o\n",
					rh->Unit.devName, rh->ccwAddr, idx,
					blkData[blkSize - (idx + 1)], blkData[blkSize + idx]);
#endif /* DEBUG */
		} else {
			// Forward Data
			blkData[idx]   = LH18(data36);
			blkData[idx+1] = RH18(data36);
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: (R) %08o (%04o) => %s\n",
					rh->Unit.devName, rh->ccwAddr, idx, pdp10_DisplayData(data36));
#endif /* DEBUG */
		}

		rh->ccwAddr++;
		rh->ccwCount--;
	}
	rh->blkCount--;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: (R) Blocks Left: %d Word Count Left: %04o (%d)\n",
			rh->Unit.devName, rh->blkCount, rh->ccwCount, rh->ccwCount);
#endif /* DEBUG */

	return (rh->blkCount | rh->ccwCount) ? MBA_CONT : MBA_OK;
}

// Transfer a 18-bit block to the PDP-10 main memory directly
int rh20_WriteBlock(void *dptr, uint8 *data, uint32 blkSize, uint32 mode)
{
	register RH20_DEVICE *rh = (RH20_DEVICE *)dptr;
	uint32  *blkData = (uint32 *)data;
	int36   data36; // Data buffer
	int     idx;

	for (idx = 0; idx < blkSize; idx += 2) {
		if (rh->ccwCount == 0) {
			if (rh20_GetNextCCW(rh))
				break;

			// Throw-away CCW
			if (rh->ccwAddr == 0) {
				idx += (rh->ccwCount << 1);
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: (W) Throw-Away CCW - %04o (%d) words skipped.\n",
						rh->Unit.devName, rh->ccwCount, rh->ccwCount);
#endif /* DEBUG */
				rh->ccwCount = 0;
				continue;
			}
		}

		if (rh->ccwOpcode & CWB_REV) {
			// Reverse Data
			data36 = blkData[blkSize - idx] |
				SL((int36)blkData[blkSize - (idx + 1)]);
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: (W) %08o (%04o) <= %06o,,%06o\n",
					rh->Unit.devName, rh->ccwAddr, idx,
					blkData[blkSize - (idx + 1)], blkData[blkSize + idx]);
#endif /* DEBUG */
		} else {
			// Forward Data
			data36 = SL((int36)blkData[idx]) | blkData[idx+1];
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: (W) %08o (%04o) <= %s\n",
					rh->Unit.devName, rh->ccwAddr, idx, pdp10_DisplayData(data36));
#endif /* DEBUG */
		}

		p10_pWrite(rh->ccwAddr++, SXT36(data36), 0);
		rh->ccwCount--;
	}
	rh->blkCount--;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: (W) Blocks Left: %d Word Count Left: %04o (%d)\n",
			rh->Unit.devName, rh->blkCount, rh->ccwCount, rh->ccwCount);
#endif /* DEBUG */

	return (rh->blkCount | rh->ccwCount) ? MBA_CONT : MBA_OK;
}

void rh20_DoTransfer(register RH20_DEVICE *rh)
{
	int fncDrive;

	// Transfer secondary registers to primary registers
	// in order that being processed for transfers.
	rh->srFlags  = (rh->srFlags | CI_PCR) & ~CI_SCR;
	rh->ptcFlags = rh->stcFlags;
	rh->ptcAddr  = rh->stcAddr;

	rh->blkCount = TC_MAXCNT - (rh->ptcFlags >> TC_P_CNT) & TC_M_CNT;
	fncDrive     = rh->ptcFlags & TC_M_FNC;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		dbg_Printf("%s: Primary Transfer Control\n", rh->Unit.devName);
		dbg_Printf("%s: Block Count = %04o (%d) Massbus Function = %02o\n",
			rh->Unit.devName, rh->blkCount, rh->blkCount, fncDrive);
		dbg_Printf("%s: TC: %06o,,%06o BAR: %06o,,%06o\n",
			rh->Unit.devName, LH18(rh->ptcFlags), RH18(rh->ptcFlags),
			LH18(rh->ptcAddr), RH18(rh->ptcAddr));
	}
#endif /* DEBUG */

	// Reset Command List Pointer
	if (rh->ptcFlags & TC_RCP)
		rh20_ResetCCW(rh);

	// Set Bus Address/Frame Count Massbus Register
	if (rh->Flags & RH20_SBAR) {
		rh->Flags &= ~RH20_SBAR;

	}

	// Now execute Massbus function...
	mba_WriteIO(rh->mbaDrive, 0, fncDrive);
}

// ***************************************************************

void rh20_ResetIO(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	rh20_ResetDevice(dptr);
}

void rh20_Opcode_DATAI(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;
	int36 data;
	int rc;

	if (rh20->Register < RH20_XREG) {
		mba_ReadIO(rh20->mbaDrive, rh20->Register, &rh20->Data);
		data = rh20->Data;
	} else {
		switch(rh20->Register) {
			case RH20_SBAR: // Secondary Bus Address Register
				data = rh20->stcAddr & RH_DATA;
				break;

			case RH20_STCR: // Secondary Transfer Control Register
				data = rh20->stcFlags & RH_DATA;
				break;

			case RH20_PBAR: // Primary Bus Address Register
				data = rh20->ptcAddr & RH_DATA;
				break;

			case RH20_PTCR: // Primary Transfer Control Register
				data = rh20->ptcFlags & RH_DATA;
				break;

			case RH20_IVIR:
				// Load Interrupt Vector Index Register
				data = rh20->iviAddr & RH_DATA;
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Interrupt Vector Address %03o\n",
						rh20->Unit.devName, rh20->iviAddr);
#endif /* DEBUG */
				break;

			default:
				// Unknown or Illegal Register
				rh20->srFlags |= CI_RAE;
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Unknown Register %02o\n",
						rh20->Unit.devName, rh20->Register);
#endif /* DEBUG */
		}
	}

	data |= SL((int36)(rh20->erFlags & ~D_DRE));

	p10_vWrite(eAddr, data, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (DI) ER => %06o,,%06o\n",
			rh20->Unit.devName, LH18(data), RH18(data));
#endif /* DEBUG */
}

void rh20_Opcode_DATAO(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;
	int36 data = p10_vRead(eAddr, 0);
	int rc;

	rh20->erFlags  = LH18(data);
	rh20->Register = (data >> D_P_REG) & D_M_REG;
	rh20->Drive    = (data >> D_P_DRV) & D_M_DRV;
	rh20->Data     = data & ER_DATA;
	rh20->mbaDrive = &rh20->mba.Drive[rh20->Drive];

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (DO) ER <= %06o,,%06o\n",
			rh20->Unit.devName, LH18(data), RH18(data));
#endif /* DEBUG */

	// Load Register if LDR bit set
	if ((rh20->erFlags & D_LDR) == 0)
  		return;

	if (rh20->Register < RH20_XREG) {
		mba_WriteIO(rh20->mbaDrive, rh20->Register, rh20->Data);
		return;
	}

	switch(rh20->Register) {
		case RH20_SBAR: // (RW) Secondary Bus Address Register
			rh20->stcAddr = data;
			rh20->Flags  |= RH20_SBAR;
			break;

		case RH20_STCR: // (RW) Secondary Transfer Control Register
			// Begin I/O transfers.
			rh20->stcFlags = data;
			rh20_DoTransfer(rh20);
			break;

		case RH20_PBAR: // (RO) Primary Bus Address Register
		case RH20_PTCR: // (RO) Primary Transfer Control Register
			// Do nothing - Read-Only Registers
			break;

		case RH20_IVIR: // (RW) Interrupt Vector Index Register
			rh20->iviAddr = data & IVI_ADR;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Interrupt Vector Address %03o\n",
					rh20->Unit.devName, rh20->iviAddr);
#endif /* DEBUG */
			break;

		default:
			// Unknown or Illegal Register
			rh20->srFlags |= CI_RAE;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unknown Register %02o\n",
					rh20->Unit.devName, rh20->Register);
#endif /* DEBUG */
	}
}

void rh20_Opcode_CONO(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	if (eAddr & CO_MBI) {
		// Massbus Initialization
		rh20_ResetDevice(rh20);
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (CO) Massbus Initialization\n", rh20->Unit.devName);
#endif /* DEBUG */
	}
 
	// Write bits to clear bits
	rh20->srFlags &= ~(eAddr & CO_CMASK);

	// Write bits to enable/clear bits
	rh20->srFlags = (rh20->srFlags & ~CO_WMASK) | (eAddr & CO_WMASK);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (CO) SR %06o <= %06o\n",
			rh20->Unit.devName, rh20->srFlags, RH18(eAddr));
#endif /* DEBUG */

	// Also, Change Priority Interrupt Assignment
	// and check any interrupt bits. If so, send
	// an interrupt signal to PDP-10.
	rh20->piLevel = (eAddr & CO_PIA);
	if ((rh20->piLevel > 0) && rh20->piRequest)
		rh20_CheckPI(rh20);
}

void rh20_Opcode_CONI(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	p10_vWrite(eAddr, rh20->srFlags, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (CI) SR => %06o\n",
			rh20->Unit.devName, rh20->srFlags);
#endif /* DEBUG */
}

void rh20_Opcode_CONSZ(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	if ((rh20->srFlags & RH18(eAddr)) == 0)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		int18 result = rh20->srFlags & RH18(eAddr);
		dbg_Printf("%s: (SZ) SR => %06o & %06o = %06o : %s\n",
			rh20->Unit.devName, rh20->srFlags, RH18(eAddr), result,
			(result == 0 ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

void rh20_Opcode_CONSO(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	if (rh20->srFlags & RH18(eAddr))
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		int18 result = rh20->srFlags & RH18(eAddr);
		dbg_Printf("%s: (SO) SR => %06o & %06o = %06o : %s\n",
			rh20->Unit.devName, rh20->srFlags, RH18(eAddr), result,
			(result ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

void *rh20_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	RH20_DEVICE *rh20 = NULL;
	P10_IOMAP   *io;
	CLK_QUEUE   *timer;
	uint32      idUnit;
	uint32      devAddr;

	if (rh20 = (RH20_DEVICE *)calloc(1, sizeof(RH20_DEVICE))) {
		// Set up its descriptions on new device.
		rh20->Unit.devName    = newMap->devName;
		rh20->Unit.keyName    = newMap->keyName;
		rh20->Unit.emuName    = newMap->emuName;
		rh20->Unit.emuVersion = newMap->emuVersion;

		idUnit = GetDeviceUnit(newMap->devName);
		if (idUnit < RH20_MAXUNITS) {
			rh20->idChannel = idUnit;
			devAddr         = RH20_BASE + (idUnit << 2);
		}
		printf("%s: Unit: %d Address: %03o\n",
			newMap->devName, idUnit, devAddr);

		// Initialize MASSBUS Controller
		mba_Create(rh20, &rh20->mba, &rh20->mbaCall, 0, argc, argv);

		// Set MBA Callback Functions
		rh20->mbaCall.SetAttention = rh20_SetAttention;
		rh20->mbaCall.SetReady     = rh20_SetReady;
		rh20->mbaCall.BeginIO      = rh20_BeginIO;
		rh20->mbaCall.EndIO        = rh20_EndIO;
		rh20->mbaCall.ReadBlock    = rh20_ReadBlock;
		rh20->mbaCall.WriteBlock   = rh20_WriteBlock;
//		rh20->mbaCall.SetBoot      = rh20_SetBoot;

		// Set up I/O mapping
		io             = &rh20->ioMap;
		io->devName    = rh20->Unit.devName;
		io->keyName    = rh20->Unit.keyName;
		io->emuName    = rh20->Unit.emuName;
		io->emuVersion = rh20->Unit.emuVersion;
		io->Device     = rh20;
		io->idDevice   = devAddr;
		io->ResetIO    = rh20_ResetIO;

		// Set up I/O instructions
		io->Function[IOF_BLKI]  = NULL;
		io->Function[IOF_DATAI] = rh20_Opcode_DATAI;
		io->Function[IOF_BLKO]  = NULL;
		io->Function[IOF_DATAO] = rh20_Opcode_DATAO;
		io->Function[IOF_CONO]  = rh20_Opcode_CONO;
		io->Function[IOF_CONI]  = rh20_Opcode_CONI;
		io->Function[IOF_CONSZ] = rh20_Opcode_CONSZ;
		io->Function[IOF_CONSO] = rh20_Opcode_CONSO;

		// Assign I/O map to PDP-10 device table
		kx10_SetMap(io);
	
		// Finally, link its device to mapping
		// device and return.
		newMap->Device = rh20;
	}

	return rh20;	
}

void *rh20_Configure(MAP_DEVICE *newMap, int argc, char **argv)
{
	RH20_DEVICE *rh20 = newMap->devParent->Device;
	MBA_DRIVE   *mbaDrive;

	mbaDrive = mba_CreateDrive(newMap, &rh20->mba, argc, argv);
	if (mbaDrive) {
		newMap->idUnit     = mbaDrive->idDrive;
		newMap->keyName    = mbaDrive->Unit.keyName;
		newMap->emuName    = mbaDrive->Unit.emuName;
		newMap->emuVersion = mbaDrive->Unit.emuVersion;
		newMap->devInfo    = newMap->devParent->devInfo;
		newMap->Device     = newMap->devParent->Device;
	}

	return mbaDrive;
}

int rh20_Delete(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	return 0;
}

int rh20_Reset(void *dptr)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)dptr;

	rh20_ResetDevice(rh20);
}

int rh20_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)map->Device;
	MBA_DRIVE   *drv  = &rh20->mba.Drive[map->idUnit];

	return mba_Attach(drv, argc, argv);
}

int rh20_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)map->Device;
	MBA_DRIVE   *drv  = &rh20->mba.Drive[map->idUnit];

	return mba_Detach(drv, argc, argv);
}

int rh20_Info(MAP_DEVICE *map, int argc, char **argv)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)map->Device;

	mba_Info(&rh20->mba, argc, argv);
}

int rh20_Boot(MAP_DEVICE *map, int argc, char **argv)
{
	RH20_DEVICE *rh20 = (RH20_DEVICE *)map->Device;
	MBA_DRIVE   *drv  = &rh20->mba.Drive[map->idUnit];

	return mba_Boot(drv, argc, argv);
}

DEVICE rh20_Device =
{
	RH20_KEY,         // Key Name (Device Type)
	RH20_NAME,        // Emulator Name
	RH20_VERSION,     // Emulator Version
	NULL,             // Lisiting of KL10 Devices
	DF_SYSMAP,        // Device Flags
	DT_DEVICE,        // Device Type

	NULL, NULL, NULL,

	rh20_Create,      // Create Routine
	rh20_Configure,   // Configure Routine
	rh20_Delete,      // Delete Routine
	rh20_Reset,       // Reset Routine
	rh20_Attach,      // Attach Routine
	rh20_Detach,      // Detach Routine
	rh20_Info,        // Info Routine
	rh20_Boot,        // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
