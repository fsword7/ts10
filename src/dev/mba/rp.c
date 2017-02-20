// rp.c - RP MASSBUS-based disk drive routines
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

#include "emu/defs.h"
#include "emu/vdisk.h"
#include "dev/mba/mba.h"
#include "dev/mba/rm.h"
#include "dev/mba/rp.h"

// This controller supports many different disk drive types:
//
// Note: 16/18-bit words in sectors per surface
//
//
// Type     # Sectors/   # Tracks/   # Cylinders/    Total
//           Surface     Cyclinder       Drive
//
// RM02/03    32/30          5            823      =  67 MB
// RP04/05    22/20         19            411      =  88 MB
// RM80       31/30         14            559      = 124 MB
// RP06       22/20         19            815      = 176 MB
// RM05       32/30         19            823      = 256 MB
// RP07       50/43         32            630      = 516 MB

#define RM02 "67MB Disk Pack Drive"
#define RM03 "67MB Disk Pack Drive"
#define RM80 "124MB Disk Pack Drive"
#define RM05 "256MB Disk Pack Drive"
#define RP04 "88MB Disk Pack Drive"
#define RP05 "88MB Disk Pack Drive"
#define RP06 "176MB Disk Pack Drive"
#define RP07 "516MB Fixed Disk Drive"

#if 0
{
	{ "RM02", RM02, 32, 30,  5, 823, 020023 },
	{ "RM03", RM03, 32, 30,  5, 823, 020024 },
	{ "RP04", RP04, 22, 20, 19, 411, 020020 },
	{ "RP05", RP05, 22, 20, 19, 411, 020021 },
	{ "RM80", RM80, 31, 30, 14, 599, 020026 },
	{ "RP06", RP06, 22, 20, 19, 815, 020022 },
	{ "RM05", RM05, 32, 30, 19, 823, 020027 },
	{ "RP07", RP07, 50, 43, 32, 630, 020042 },
	{ NULL }
};
#endif

MBA_DTYPE rp_dTypes[] = {
	{
		"RP04",               // Device Name
		"RP04 - 88MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		20,                   // Number of Sectors
		14,                   // Number of Tracks
		411,                  // Number of Cylinders
		(20 * 14 * 411),      // Total Blocks
		020020,               // Device Type
		NULL,                 // Not Used

		MBA_PACK,            // RH11 Flags - Disk Pack Type
		NULL,                 // Not Used
	},

	{
		"RP05",               // Device Name
		"RP05 - 88MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		20,                   // Number of Sectors
		14,                   // Number of Tracks
		411,                  // Number of Cylinders
		(20 * 14 * 411),      // Total Blocks
		020021,               // Device Type
		NULL,                 // Not Used

		MBA_PACK,            // RH11 Flags - Disk Pack Type
		NULL,                 // Not Used
	},

	{
		"RP06",               // Device Name
		"RP06 - 176MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		20,                   // Number of Sectors
		19,                   // Number of Tracks
		815,                  // Number of Cylinders
		(20 * 19 * 815),      // Total Blocks
		020022,               // Device Type
		NULL,                 // Not Used

		MBA_PACK,            // RH11 Flags - Disk Pack Type
		NULL,                 // Not Used
	},

	{
		"RP07",               // Device Name
		"RP07 - 516MB Fixed Disk Drive",
		UNIT_ATTABLE|UNIT_DISABLE,
		43,                   // Number of Sectors
		32,                   // Number of Tracks
		630,                  // Number of Cylinders
		(43 * 32 * 630),      // Total Blocks
		020042,               // Device Type

		NULL,                 // Not Used

		MBA_FIXED,           // RH11 Flags - Fixed Disk Type
		NULL,                 // Not Used
	},

	{ NULL },
};

MBA_DTYPE rm_dTypes[] = {
	{
		"RM02",          // Device Name
		"RM02 - 67MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		32,              // Number of Sectors
		5,               // Number of Surfaces
		823,             // Number of Cylinders
		(32 * 5 * 823),  // Total Blocks
		020023,          // Device Type
		NULL,            // Not Used

		MBA_PACK,        // RH11 Flags - Disk Pack Type
		NULL,            // Not Used
	},

	{
		"RM03",          // Device Name
		"RM03 - 67MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		32,              // Number of Sectors
		5,               // Number of Surfaces
		823,             // Number of Cylinders
		(32 * 5 * 823),  // Total Blocks
		020024,          // Device Type
		NULL,            // Not Used

		MBA_PACK,        // RH11 Flags - Disk Pack Type
		NULL,            // Not Used
	},

	{
		"RM80",          // Device Name
		"RM80 - 124MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		31,              // Number of Sectors
		19,              // Number of Surfaces
		559,             // Number of Cylinders
		(31 * 19 * 559), // Total Blocks
		020026,          // Device Type
		NULL,            // Not Used

		MBA_PACK,        // RH11 Flags - Disk Pack Type
		NULL,            // Not Used
	},

	{
		"RM05",          // Device Name
		"RM05 - 256MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		32,              // Number of Sectors
		19,              // Number of Surfaces
		823,             // Number of Cylinders
		(32 * 19 * 823), // Total Blocks
		020027,          // Device Type
		NULL,            // Not Used

		MBA_PACK,        // RH11 Flags - Disk Pack Type
		NULL,            // Not Used
	},

	{ NULL },
};

#ifdef DEBUG
static char *regNames[] =
{
	"RPCS1", // (R/W) Control and Status Register #1
	"RPDS",  // (R)   Drive Status Register
	"RPER1", // (R)   Error Register #1
	"RPMR",  // (R/W) Maintenance Register
	"RPAS",  // (R/W) Attention Summary Register
	"RPDA",  // (R/W) Desired Sector/Track Address Register
	"RPDT",  // (R)   Drive Type Register
	"RPLA",  // (R)   Look-Ahead Register
	"RPSN",  // (R)   Serial Number Register
	"RPOF",  // (R/W) Offset Register
	"RPDC",  // (R/W) Desired Cylinder Address Register
	"RPCC",  // (R/W) Current Cylinder Address Register
	"RPER2", // (R)   Error Register #2
	"RPER3", // (R/W) Error Register #3
	"RPEC1", // (R)   ECC Position Register
	"RPEC2", // (R)   ECC Pattern Register
};

static char *fncNames[] =
{
	"No Operation",          // 00
	"Unload",                // 01
	"Seek",                  // 02
	"Recalibrate",           // 03
	"Drive Clear",           // 04
	"Port Release",          // 05
	"Offset",                // 06
	"Return to Clearline",   // 07
	"Read-In Preset",        // 10
	"Pack Acknowledge",      // 11
	"*Unknown",              // 12
	"*Unknown",              // 13
	"Search",                // 14
	"*Unknown",              // 15
	"*Unknown",              // 16
	"*Unknown",              // 17
	"*Unknown",              // 20
	"*Unknown",              // 21
	"*Unknown",              // 22
	"*Unknown",              // 23
	"Verify Data",           // 24 Write Check Data
	"Verify Header",         // 25 Write Check Header
	"*Unknown",              // 26
	"*Unknown",              // 27
	"Write Data",            // 30
	"Write Header",          // 31
	"*Unknown",              // 32
	"*Unknown",              // 33
	"Read Data",             // 34
	"Read Header",           // 35
	"*Unknown",              // 36
	"*Unknown"               // 37 
};

#endif /* DEBUG */

// Valid register for access
static char rp_regValid[] = {
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
};
 
static char rp_regWrite[] = {
	1, 0, 0, 1,  1, 1, 0, 0,  0, 1, 1, 1,  0, 1, 0, 0,
	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
};

// ************************************************************

void rp_ResetDrive(MBA_DRIVE *mbaDrive)
{
	ts10_CancelTimer(mbaDrive->Timer);

	RPCS1 = RPCS1_DVA; // DVA - Always On.
	RPDA  = 0;
	RPER1 = 0;
	RPER2 = 0;
	RPER3 = 0;
}

void rp_SetAttention(MBA_DRIVE *mbaDrive)
{
	// Set attention bit on drive status register and
	// inform its controller that attention is needed.
	RPDS |= RPDS_ATA;
	mbaDrive->Callback->SetAttention(mbaDrive->pDevice);
}

int rp_CheckAttention(MBA_DRIVE *mbaDrive)
{
	return (RPDS & RPDS_ATA) ? 1 : 0;
}

void rp_ClearAttention(MBA_DRIVE *mbaDrive)
{
	RPDS &= ~RPDS_ATA;
}

void rp_SetReady(MBA_DRIVE *mbaDrive)
{
	// Set ready/done bit on drive status register and
	// inform its controller that command or drive is
	// ready or done. 
	RPDS |= RPDS_DRY;
	mbaDrive->Callback->SetReady(mbaDrive->pDevice);
}

int rp_WriteBlock(MBA_DRIVE *mbaDrive)
{
	RP_DRIVE *rp  = (RP_DRIVE *)mbaDrive->FileRef;
	uint18   blkData[RP_BLKSZ]; // 16/18-bit data buffer
	int      rc;

	// Get data block from host.
	rc = mbaDrive->Callback->ReadBlock(mbaDrive->pDevice,
		(uint8 *)blkData, RP_BLKSZ, MBA_18B);
	if (rc != MBA_ERROR) {
		if (vdk_WriteDisk(&rp->dpDisk, (uint8 *)blkData, VDK_18B))
			return MBA_ERROR;
	}

	return rc;
}

int rp_CheckBlock(MBA_DRIVE *mbaDrive)
{
	RP_DRIVE *rp  = (RP_DRIVE *)mbaDrive->FileRef;
	uint18   blkData1[RP_BLKSZ]; // 16/18-bit buffer
	uint18   blkData2[RP_BLKSZ]; // 16/18-bit buffer
	int      idx, rc;

	if (vdk_ReadDisk(&rp->dpDisk, (uint8 *)blkData1, VDK_18B))
		return MBA_ERROR;

	// Get data block from host.
	rc = mbaDrive->Callback->ReadBlock(mbaDrive->pDevice,
		(uint8 *)blkData2, RP_BLKSZ, MBA_18B);

	if (rc != MBA_ERROR) {
		for (idx = 0; idx < RP_BLKSZ; idx++)
			if (blkData1[idx] != blkData2[idx])
				return -2;

	}

	return rc;
}

int rp_ReadBlock(MBA_DRIVE *mbaDrive)
{
	RP_DRIVE *rp  = (RP_DRIVE *)mbaDrive->FileRef;
	uint18   blkData[RP_BLKSZ]; // 16/18-bit data buffer

	if (vdk_ReadDisk(&rp->dpDisk, (uint8 *)blkData, VDK_18B))
		return MBA_ERROR;
	return mbaDrive->Callback->WriteBlock(mbaDrive->pDevice,
		(uint8 *)blkData, RP_BLKSZ, MBA_18B);
}

void rp_Process(void *dptr)
{
	MBA_DRIVE    *mbaDrive = (MBA_DRIVE *)dptr;
	MBA_DTYPE    *mbaType  = mbaDrive->dType;
	MBA_CALLBACK *mbaCall  = mbaDrive->Callback;
	RP_DRIVE     *rp       = (RP_DRIVE *)mbaDrive->FileRef;
	char   *diskName = mbaType->Name;
	int    fnc       = (RPCS1 & RPCS1_FUNC) >> 1;
	int    diskAddr;
	int    temp, st;
	int    savedMode = 0; // temp.

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Drive %d - Process: %s\n",
			diskName, mbaDrive->idDrive, fncNames[fnc]);
#endif /* DEBUG */

	if (RPER1 || RPER2 || RPER3) {
		RPDS |= RPDS_ERR;
		if (RPER2 || RPER3)
			RPER1 |= RPER1_UNS;
	}

	switch (fnc) {
		case FNC_RECAL:
		case FNC_SEEK:
		case FNC_SEARCH:
			diskAddr = GetDiskAddr(RPDC, RPDA, mbaType);

			st = vdk_SeekDisk(&rp->dpDisk, diskAddr);
#ifdef DEBUG
//			if (dbg_Check(DBG_IODATA) && st == VDK_OK)
//				dbg_Printf("%s: Unit %d - Move to disk address %08X\n",
//					diskName, mbaDrive->idDrive, 0);
#endif /* DEBUG */
			RPCC = RPDC;
			RPDS &= ~RPDS_PIP;
			rp_SetAttention(mbaDrive);
			break;

		case FNC_OFFSET:
		case FNC_RETURN:
			RPDS &= ~RPDS_PIP;
			rp_SetAttention(mbaDrive);
			break;

		case FNC_WR_DATA:
			if (RPDS & RPDS_WRL) {
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: Unit %d - Write Lock Error\n",
						diskName, mbaDrive->idDrive);
#endif /* DEBUG */
				RPER1 |= RPER1_WLE;
				break;
			}

		case FNC_CHK_DATA:
		case FNC_RD_DATA:
			// Tell disk controller that transfers begin.
			if (mbaCall->BeginIO)
				mbaCall->BeginIO(mbaDrive->pDevice);

			diskAddr = GetDiskAddr(RPDC, RPDA, mbaType);
			do {
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: Disk Address %d (Cyl %d Trk %d Sec %d)\n",
						diskName, diskAddr, RPDC, GetTrack(RPDA), GetSector(RPDA));
#endif /* DEBUG */

				if (diskAddr >= mbaType->Blocks) {
					RPER1 |= RPER1_AOE;
					break;
				}

				if ((st = vdk_SeekDisk(&rp->dpDisk, diskAddr)) == VDK_OK) {
					switch (fnc) {
						case FNC_WR_DATA:
							st = rp_WriteBlock(mbaDrive);
							break;

						case FNC_CHK_DATA:
							st = rp_CheckBlock(mbaDrive);
							break;

						case FNC_RD_DATA:
							st = rp_ReadBlock(mbaDrive);
							break;
					}
					if (st == MBA_ERROR)
						break;

					// Update RPDA and RPDC registers
					diskAddr++;
					temp = diskAddr % mbaType->Sectors;
					if (temp == (mbaType->Sectors - 1))
						RPDS |= RPDS_LST;
					else
						RPDS &= ~RPDS_LST;
					RPDA = temp;
					temp = diskAddr / mbaType->Sectors;
					RPDA |= (temp % mbaType->Tracks) << RPDA_P_TA;
					RPDC = temp / mbaType->Tracks;
				}
			} while (st == MBA_CONT);

			// Tell disk controller that transfers are completed
			// so that controller can update status registers.
			if (mbaCall->EndIO)
				mbaCall->EndIO(mbaDrive->pDevice);
			break;
	}

	// Check any errors. If so, set attention bit.
	if (RPER1 || RPER2 || RPER3) {
		RPDS |= RPDS_ERR;
		rp_SetAttention(mbaDrive);
	}

	// When process is done, GO bit had been turned off.
	// Also, inform MASSBUS controller this drive is ready now.
	RPCS1 &= ~RPCS1_GO;
	rp_SetReady(mbaDrive);
}

void rp_Execute(MBA_DRIVE *mbaDrive)
{
	MBA_DTYPE  *mbaType  = mbaDrive->dType;
	char   *diskName = mbaType->Name;
	int    fnc       = (RPCS1 & RPCS1_FUNC) >> 1;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: Drive %d - Execute: %s\n",
			diskName, mbaDrive->idDrive, fncNames[fnc]);
#endif /* DEBUG */

	if (fnc != FNC_DCLR) {
		if (RPDS & RPDS_ERR) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d - Function isn't accetped when DS_ERR is set.\n",
					diskName, mbaDrive->idDrive);
#endif /* DEBUG */
			return;
		}

		if (!(RPDS & RPDS_MOL)) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d - Medium is not on-line!\n",
					diskName, mbaDrive->idDrive);
#endif /* DEBUG */
			RPER1 |= RPER1_UNS;
			RPDS  |= RPDS_ERR;
			RPCS1 &= ~RPCS1_GO;
			rp_SetAttention(mbaDrive);
			return;
		}
	}

	RPDS &= ~RPDS_LST;

	switch (fnc) {
		case FNC_NOP:
		case FNC_RELEASE:
			RPCS1 &= ~RPCS1_GO;
			break;

		case FNC_RECAL:
			RPDC = 0;
		case FNC_SEEK:
		case FNC_SEARCH:
			if ((GetCylinder(RPDC) >= mbaType->Cylinders) ||
			    (GetTrack(RPDA)    >= mbaType->Tracks)    ||
			    (GetSector(RPDA)   >= mbaType->Sectors)) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS)) {
					dbg_Printf("%s: Unit %d - Invalid Address Error\n",
						diskName, mbaDrive->idDrive);
					dbg_Printf("%s: C %d, T %d, S %d (Max C %d, T %d, S %d)\n",
						diskName, GetCylinder(RPDC), GetTrack(RPDA),
						GetSector(RPDA), mbaType->Cylinders,
						mbaType->Tracks, mbaType->Sectors);
				}
#endif /* DEBUG */
				RPER1 |= RPER1_IAE;
				break;
			}
			RPDS = (RPDS & ~RPDS_DRY) | RPDS_PIP;
			ts10_SetTimer(mbaDrive->Timer);
//			rp_Process(mbaDrive);
			break;

		case FNC_DCLR:
			rp_ResetDrive(mbaDrive);
			RPCS1 &= ~RPCS1_GO;
			break;

		case FNC_OFFSET:
		case FNC_RETURN:
			RPDS = (RPDS & ~RPDS_DRY) | RPDS_PIP;
			ts10_SetTimer(mbaDrive->Timer);
//			rp_Process(mbaDrive);
			break;

		case FNC_PRESET:
			RPDA = 0;
			RPOF = 0;
			RPDC = 0;
			RPCS1 &= ~RPCS1_GO;
			break;

		case FNC_PACK:
			RPDS |= RPDS_VV;
			RPCS1 &= ~RPCS1_GO;
			break;

		case FNC_WR_DATA:
		case FNC_CHK_DATA:
		case FNC_RD_DATA:
			if ((GetCylinder(RPDC) >= mbaType->Cylinders) ||
			    (GetTrack(RPDA)    >= mbaType->Tracks)    ||
			    (GetSector(RPDA)   >= mbaType->Sectors)) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS)) {
					dbg_Printf("%s: Unit %d - Invalid Address Error\n",
						diskName, mbaDrive->idDrive);
					dbg_Printf("%s: C %d, T %d, S %d (Max C %d, T %d, S %d)\n",
						diskName, GetCylinder(RPDC), GetTrack(RPDA),
						GetSector(RPDA), mbaType->Cylinders,
						mbaType->Tracks, mbaType->Sectors);
				}
#endif /* DEBUG */
				RPER1 |= RPER1_IAE;
				rp_SetAttention(mbaDrive);
				break;
			}
			RPDS &= ~RPDS_DRY;
			ts10_SetTimer(mbaDrive->Timer);
//			rp_Process(mbaDrive);
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d - Unknown function code %02o\n", diskName,
					mbaDrive->idDrive, (RPCS1 & (RPCS1_FUNC|RPCS1_GO)));
#endif /* DEBUG */
			RPCS1 &= ~RPCS1_GO;
			RPER1 |= RPER1_ILF;
			rp_SetAttention(mbaDrive);
			break;
	}
}

void rp_ReadIO(MBA_DRIVE *mbaDrive, int reg, uint16 *data)
{
	char *diskName = mbaDrive->dType->Name;

	if (rp_regValid[reg]) {

		if (reg == nRPLA) {
			// Increase sector count by one
			RPLA += (1 << RPLA_P_SC);
			if ((RPLA >> RPLA_P_SC) == mbaDrive->dType->Sectors)
				RPLA = 0;
		}
		if (reg == nRPAS) {
			MBA_DEVICE *mba = mbaDrive->wDevice;
			int idx, ata = 0;

			for (idx = 0; idx < MBA_MAXDRIVES; idx++) {
				if (mba->Drive[idx].Regs[nRPDS] & RPDS_ATA)
					ata |= (1 << idx);
			}
			RPAS = ata;
		}
		*data = MBA_REG(reg);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: %s (%02o) => %06o\n",
			diskName, regNames[reg], reg, *data);
#endif /* DEBUG */
	} else {
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (R) Unknown MASSBUS Register %02o\n",
				diskName, reg);
#endif /* DEBUG */
		RPER1 |= RPER1_ILR;
		RPDS  |= RPDS_ERR;
		rp_SetAttention(mbaDrive);
	}
}

void rp_WriteIO(MBA_DRIVE *mbaDrive, int reg, uint16 data)
{
	char *diskName = mbaDrive->dType->Name;

	if (rp_regValid[reg]) {

		// Update registers
		if (reg == nRPCS1) {
			RPCS1 = (RPCS1 & ~RPCS1_WR) | (data & RPCS1_WR);
			if (RPCS1 & RPCS1_GO) {
				RPDS &= ~RPDS_ATA;
				rp_Execute(mbaDrive);
			}
		} else if (reg == nRPAS) {
			MBA_DEVICE *mba = mbaDrive->wDevice;
			int idx;

			for (idx = 0; idx < MBA_MAXDRIVES; idx++) {
				if (data & 1)
					mba->Drive[idx].Regs[nRPDS] &= ~RPDS_ATA;
				data >>= 1;
			}
		} else if (rp_regWrite[reg])
			MBA_REG(reg) = data;

#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: %s (%02o) <= %06o\n",
				diskName, regNames[reg], reg, data);
#endif /* DEBUG */

	} else {
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (W) Unknown MASSBUS Register %o\n",
				diskName, reg);
#endif /* DEBUG */
		RPER1 |= RPER1_ILR;
	}

	if (RPER1 || RPER2 || RPER3) {
		RPDS |= RPDS_ERR;
		if (RPER2 || RPER3)
			RPER1 |= RPER1_UNS;
		rp_SetAttention(mbaDrive);
	}
}

// ****************************************************************

DEVICE  rp_DeviceInfo;

void rp_InitDrive(MBA_DRIVE *mbaDrive)
{
	RP_DRIVE  *rp;
	UQ_BOOT   *bt;
	CLK_QUEUE *newTimer;

	mbaDrive->devInfo = &rp_DeviceInfo;

	// Set new timer
	newTimer = (CLK_QUEUE *)calloc(1, sizeof(CLK_QUEUE));
	newTimer->Next     = NULL;
	newTimer->Flags    = 0;
	newTimer->outTimer = 10;
	newTimer->nxtTimer = 10;
	newTimer->Device   = (void *)mbaDrive;
	newTimer->Execute  = rp_Process;
	mbaDrive->Timer    = newTimer;

	// Initialize drive registers
	RPCS1 = RPCS1_DVA; // DVA - Always On.
	RPDS  = RPDS_DPR;  // DPR - Always On.
	RPDT  = mbaDrive->dType->dType;
	RPSN  = 01000 + mbaDrive->idDrive;
	RPDA  = 0;
	RPER1 = 0;
	RPER2 = 0;
	RPER3 = 0;

	if (rp = (RP_DRIVE *)calloc(1, sizeof(RP_DRIVE))) {
		rp->Flags    = 0;
		rp->mbaDrive = mbaDrive;
		rp->idUnit   = mbaDrive->idDrive;

		if (mbaDrive->Callback->SetBoot) {
			// Set up boot device.
			bt          = &rp->Boot;
			bt->Flags   = BT_DISK|BT_SUPPORTED;
			bt->idDrive = mbaDrive->idDrive;
			mbaDrive->Callback->SetBoot(mbaDrive->pDevice, bt);
		}

		mbaDrive->FileRef = rp;
	}
}

int rp_Attach(MBA_DRIVE *mbaDrive, char *fileName)
{
	RP_DRIVE  *rp    = (RP_DRIVE *)mbaDrive->FileRef;
	VDK_DISK  *vdk   = &rp->dpDisk;
	MBA_DTYPE *dType = mbaDrive->dType;
	int Blocks;
	int rc;

	if (mbaDrive->Flags & MBA_ATTACHED)
		return EMU_ATTACHED;

	memset(vdk, 0, sizeof(VDK_DISK));
	vdk->fileName = (char *)malloc(strlen(fileName)+1);
	strcpy(vdk->fileName, fileName);

	vdk->Flags      = VDK_18B;
	vdk->devType    = dType->Name;
	vdk->Cylinders  = dType->Cylinders;
	vdk->Tracks     = dType->Tracks;
	vdk->Sectors    = dType->Sectors;
	vdk->vsBlock    = RP_BLKSZ * 2;

	if (rc = vdk_OpenDisk(vdk)) {
		if (vdk->fileName)
			free(vdk->fileName);
		return TS10_OPENERR;
	}

	mbaDrive->Flags |= MBA_ATTACHED;

	// Enable boot device for boot command
	if (rp->Boot.Flags & BT_SUPPORTED) {
		rp->Boot.ioDevice  = vdk;
		rp->Boot.Flags    |= BT_VALID;
	}

	// Now let's set some registers for medium on-line
	RPDS |= (RPDS_ATA|RPDS_MOL|RPDS_DRY|RPDS_VV);
	rp_SetAttention(mbaDrive);
	rp_SetReady(mbaDrive);

	return EMU_OK;
}

int rp_Detach(MBA_DRIVE *mbaDrive)
{
	RP_DRIVE *rp  = (RP_DRIVE *)mbaDrive->FileRef;
	VDK_DISK *vdk = &rp->dpDisk;
	int      rc;

	if ((mbaDrive->Flags & MBA_ATTACHED) == 0)
		return EMU_ATTACHED;

	// Now let's set some registers for medium off-line
	mbaDrive->Flags &= ~MBA_ATTACHED;
	RPDS &= ~(RPDS_MOL|RPDS_WRL|RPDS_DRY|RPDS_VV);

	// Disable boot device.
	if (rp->Boot.Flags & BT_SUPPORTED) {
		rp->Boot.Flags    &= ~BT_VALID;
		rp->Boot.ioDevice  = NULL;
	}

	// Detach a file from that drive slot.
	rc = vdk_CloseDisk(vdk);
	if (vdk->fileName) {
		free(vdk->fileName);
		vdk->fileName == NULL;
	}

	return rc;
}

int rp_Boot(MBA_DRIVE *mbaDrive, int argc, char **argv)
{
	RP_DRIVE  *rp = (RP_DRIVE *)mbaDrive->FileRef;
	UQ_BOOT   *bt = &rp->Boot;

	if ((bt->Flags * BT_SUPPORTED) == 0)
		return EMU_NOTSUPPORTED;
	if (bt->Flags & BT_VALID)
		return bt->Execute(bt, argc, argv);
	return EMU_NOTSUPPORTED;
}

// ****************************************************************

MBA_DEVINFO rp_DriveInfo =
{
	"RP",                     // Device Name
	"Disk Pack Drive Series", // Description
	"0.2 (Alpha)",            // Version
	rp_dTypes,                // Listing of RP devices

	rp_InitDrive,      // InitDrive Routine
	rp_ResetDrive,     // ResetDrive Routine
	rp_Attach,         // Mount/Attach Routine
	rp_Detach,         // Dismount/Detach Routine
	rp_Boot,           // Boot Routine
	rp_ReadIO,         // Read I/O Routine
	rp_WriteIO,        // Write I/O Routine
	rp_CheckAttention, // Check Attention for AS register
	rp_ClearAttention, // Clear Attention for AS register
};

MBA_DEVINFO rm_DriveInfo =
{
	"RM",                     // Device Name
	"Disk Pack Drive Series", // Description
	"0.2 (Alpha)",            // Version
	rm_dTypes,                // Listing of RP devices

	rp_InitDrive,      // InitDrive Routine
	rp_ResetDrive,     // ResetDrive Routine
	rp_Attach,         // Mount/Attach Routine
	rp_Detach,         // Dismount/Detach Routine
	rp_Boot,           // Boot Routine
	rp_ReadIO,         // Read I/O Routine
	rp_WriteIO,        // Write I/O Routine
	rp_CheckAttention, // Check Attention for AS register
	rp_ClearAttention, // Clear Attention for AS register
};
