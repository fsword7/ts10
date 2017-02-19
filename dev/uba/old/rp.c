// rp.c - RP MASSBUS-based disk drive routines
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS-10 Emulator.
// See README for copyright notice.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "emu/defs.h"
#include "dev/rh.h"
#include "dev/rp.h"
#include "dev/proto.h"

// This controller supports many different disk drive types:
//
// Note: 16/18-bit words in sectors per surface
//
// Type     # Sectors/   # Tracks/   # Cylinders/    Total
//           Surface     Cyclinder       Drive
//
// RP04/05    22/20         19            411      =  88 MB
// RP06       22/20         19            815      = 176 MB
// RP07       50/43         32            630      = 516 MB

DTYPE rp_dTypes[] = {
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

		RH11_PACK,            // RH11 Flags - Disk Pack Type
		NULL,                 // Not Used

		disk_Open,            // Low-level Open Routine
		disk_Close,           // Low-level Close Routine
		disk_Read,            // Low-level Read Routine
		disk_Write,           // Low-level Write Routine
		NULL,                 // Low-level Mark Routine
		NULL,                 // Low-level Rewind Routine
		NULL,                 // Low-level Skip Routine
		disk_Seek,            // Low-level Seek Routine
		disk_GetDiskAddr      // Low-level GetDiskAddr Routine
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

		RH11_PACK,            // RH11 Flags - Disk Pack Type
		NULL,                 // Not Used

		disk_Open,            // Low-level Open Routine
		disk_Close,           // Low-level Close Routine
		disk_Read,            // Low-level Read Routine
		disk_Write,           // Low-level Write Routine
		NULL,                 // Low-level Mark Routine
		NULL,                 // Low-level Rewind Routine
		NULL,                 // Low-level Skip Routine
		disk_Seek,            // Low-level Seek Routine
		disk_GetDiskAddr      // Low-level GetDiskAddr Routine
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

		RH11_PACK,            // RH11 Flags - Disk Pack Type
		NULL,                 // Not Used

		disk_Open,            // Low-level Open Routine
		disk_Close,           // Low-level Close Routine
		disk_Read,            // Low-level Read Routine
		disk_Write,           // Low-level Write Routine
		NULL,                 // Low-level Mark Routine
		NULL,                 // Low-level Rewind Routine
		NULL,                 // Low-level Skip Routine
		disk_Seek,            // Low-level Seek Routine
		disk_GetDiskAddr      // Low-level GetDiskAddr Routine
	},

	{
		"RP07",               // Device Name
		"RP07 - 516MB Fixed Disk Drive",
		UNIT_ATTABLE|UNIT_DISABLE,
		43,                   // Number of Sectors
		32,                   // Number of Tracks
		630,                  // Number of Cylinders
		(43 * 32 * 630),      // Total Blocks
		020024,               // Device Type

		NULL,                 // Not Used

		RH11_FIXED,           // RH11 Flags - Fixed Disk Type
		NULL,                 // Not Used

		disk_Open,            // Low-level Open Routine
		disk_Close,           // Low-level Close Routine
		disk_Read,            // Low-level Read Routine
		disk_Write,           // Low-level Write Routine
		NULL,                 // Low-level Mark Routine
		NULL,                 // Low-level Rewind Routine
		NULL,                 // Low-level Skip Routine
		disk_Seek,            // Low-level Seek Routine
		disk_GetDiskAddr      // Low-level GetDiskAddr Routine
	},

	{ NULL },
};

DEVICE rp_Device =
{
	"RP",          // Device Name
	"MASSBUS Disk Pack Drives",
	"v0.9 (Beta)", // Version
	rp_dTypes,     // Listing of RP devices
	NULL,          // Listing of Units
	NULL,          // Listing of Slave Devices - Not used
	NULL,          // Listing of Commands
	NULL,          // Listing of Set Commands
	NULL,          // Listing of Show Commands

	0,             // Number of Devices
	RP_MAXUNITS,   // Number of Units

	rp_Initialize, // Initialize Routine
	rp_Reset,      // Reset Routine
	rp_Create,     // Create Routine
	rp_Delete,     // Delete Routine
	NULL,          // Configure Routine - Not Used
	rp_Enable,     // Enable Routine
	rp_Disable,    // Disable Routine
	rp_Attach,     // Attach Routine
	rp_Detach,     // Detach Routine
	rp_Format,     // Format Routine
	rp_ReadIO,     // Read I/O Routine
	rp_WriteIO,    // Write I/O Routine
	rp_Process,    // Process Routine
	rp_Boot,       // Boot Routine
	NULL,          // Execute Routine - Not Used

	rp_SetUnit,    // SetUnit Routine
	rp_SetATA,     // SetATA Routine
	rp_ClearATA    // ClearATA Routine
};

#ifdef DEBUG
// Listing of name of the registers for debug facility
// Note: ^ = Mixed, * = Drive Register, space = Controller Register

char *rp_regNames[] =
{
	"RPCS1", // (R/W) ^Control and Status Register #1
	"RPWC",  // (R/W)  Word Count Register
	"RPBA",  // (R/W)  Bus Address Register
	"RPDA",  // (R/W) *Desired Sector/Track Address Register
	"RPCS2", // (R/W)  Control and Status Register #2
	"RPDS",  // (R)   *Drive Status Register
	"RPER1", // (R)   *Error Register #1
	"RPAS",  // (R/W) *Attention Summary Register
	"RPLA",  // (R)   *Look-Ahead Register
	"RPDB",  // (R/W)  Data Buffer Register
	"RPMR",  // (R/W) *Maintenance Register
	"RPDT",  // (R)   *Drive Type Register
	"RPSN",  // (R)   *Serial Number Register
	"RPOF",  // (R/W) *Offset Register
	"RPDC",  // (R/W) *Desired Cylinder Address Register
	"RPCC",  // (R/W) *Current Cylinder Address Register
	"RPER2", // (R)   *Error Register #2
	"RPER3", // (R/W) *Error Register #3
	"RPEC1", // (R)   *ECC Position Register
	"RPEC2", // (R)   *ECC Pattern Register
	"RPBAE", // (R/W)  Bus Address Extension Register  (RH70 Only)
	"RPCS3"  // (R/W)  Control and Status Register #3  (RH70 Only)
};

char *rp_fncNames[] =
{
	"No Operation",          // 00
	"Unload",                // 01
	"Seek",                  // 02
	"Recalibrate",           // 03
	"Drive Clear",           // 04
	"Release",               // 05
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

uint8 rp_cBuffer[RP_BLKSZ18];

// Power Up Initialization
void rp_Initialize(UNIT *rpUnit)
{
	RPUNIT *rpData = (RPUNIT *)rpUnit->uData;

	rpData->rpcs1 = RPCS1_DVA; // DVA - Always On.
	rpData->rpda  = 0;
	rpData->rpds  = RPDS_DPR;  // DPR - Always On.
	rpData->rper1 = 0;
	rpData->rper2 = 0;
	rpData->rper3 = 0;
	rpData->rpdt  = rpUnit->dType->dType;
	rpData->rpsn  = 01000 + rpUnit->idUnit;
}

void rp_Reset(UNIT *rpUnit)
{
	RPUNIT *rpData  = (RPUNIT *)rpUnit->uData;

	rpData->rpcs1 = RPCS1_DVA; // DVA - Always On.
	rpData->rpda  = 0;
	rpData->rper1 = 0;
	rpData->rper2 = 0;
	rpData->rper3 = 0;
}

// Create the desired RP disk drive
int rp_Create(UNIT *uptr, char *devName, int argc, char **argv)
{
	if (!(uptr->Flags & UNIT_PRESENT)) {
		RPUNIT *rpData = (RPUNIT *)calloc(sizeof(RPUNIT), 1);

		if (rpData) {
			int idx;
			int st;

			for (idx = 0; rp_dTypes[idx].Name; idx++)
				if (!strcasecmp(argv[0], rp_dTypes[idx].Name))
					break;

			if (!rp_dTypes[idx].Name) {
				free(rpData);
				return EMU_ARG;
			}

			*strchr(argv[1], ':') = '\0';
//			if (st = unit_mapCreateDevice(argv[1], uptr)) {
//				free(rpData);
//				return st;
//			}

			// This unit is a disk/pack drive.
			uptr->tFlags  = UT_STORAGE;
			uptr->dType   = &rp_dTypes[idx];
			uptr->Device  = &rp_Device;
			uptr->Flags   = rp_dTypes[idx].Flags | UNIT_PRESENT;
			uptr->uData   = (void *)rpData;

			uptr->Blocks  = rp_dTypes[idx].Blocks;
			uptr->szBlock = RP_BLKSZ18;

			ToUpper(argv[1]);
			printf("Device %s: had been created.\n", argv[1]);
		} else
			return EMU_MEMERR;
	} else
		return EMU_ARG;
	return EMU_OK;
}

// Delete the desired RP disk drive
int rp_Delete(UNIT *rpUnit)
{
	if (rpUnit->Flags & UNIT_PRESENT) {
		free(rpUnit->uData);
		rpUnit->Flags &= ~UNIT_PRESENT;

		return EMU_OK;
	}
	return EMU_NPRESENT;
}

int rp_Enable(UNIT *rpUnit)
{
	return EMU_OK;
}

int rp_Disable(UNIT *rpUnit)
{
	return EMU_OK;
}

int rp_Attach(UNIT *rpUnit, char *fileName)
{
	RPUNIT *rpData = (RPUNIT *)rpUnit->uData;
	int    st;

	if (rpUnit->Flags & UNIT_DISABLED)
		return EMU_DISABLED;

	if (rpUnit->Flags & UNIT_ATTACHED)
		return EMU_ATTACHED;

	// Attach a file to that unit
	st = disk_Open(&rpUnit->FileRef, fileName, 0,
		rpUnit->Blocks, rpUnit->szBlock);
	if (st) return st;

	rpUnit->Flags |= UNIT_ATTACHED;

	// Now let's set some registers for medium on-line
	rpData->rpds |= (RPDS_MOL|RPDS_DRY|RPDS_VV);
	rp_SetATA(rpUnit);

	return EMU_OK;
}

int rp_Detach(UNIT *rpUnit)
{
	RPUNIT *rpData = (RPUNIT *)rpUnit->uData;
	int st;

	if (rpUnit->Flags & UNIT_DISABLED)
		return EMU_DISABLED;

	if (!(rpUnit->Flags & UNIT_ATTACHED))
		return EMU_ARG;

	// Detach a file from that unit.
	if (st = disk_Close(&rpUnit->FileRef))
		return st;

	rpUnit->Flags &= ~(UNIT_ATTACHED|UNIT_WRLOCKED);

	// Now let's set some registers for medium off-line
	rpData->rpds &= ~(RPDS_MOL|RPDS_WRL|RPDS_DRY|RPDS_VV);

	return EMU_OK;
}

int36 rp_Packed(uchar *str)
{
	int36 data = 0;

	data =  ((int36)(str[0] & 0177)) << (35 - 17);
	data |= ((int36)(str[1] & 0177)) << (35 - 9);
	data |= ((int36)(str[2] & 0177)) << (35 - 35);
	data |= ((int36)(str[3] & 0177)) << (35 - 27);

	return data;
}

// Do not use that format routine at this time.  My advise
// is using TOPS-10 operating system (startup: prompt) to
// DESTROYing to create new generated RP06 pack file.

int rp_Format(UNIT *rpUnit)
{
	int36 homBlock[128];
	int36 batBlock[128];
	int   idx1, idx2;

	// Format for TOPS-10 operating system

	// Initialize HOM Block
	memset(&homBlock, 0, sizeof(homBlock));
	homBlock[0000] = PackedASCII6("HOM");
	homBlock[0001] = PackedASCII6("TIM000");
	homBlock[0002] = FORMWORD36(FORMCHS18(0,0,0), FORMCHS18(0,0,10));
	homBlock[0026] = -1LL;
	homBlock[0173] = rp_Packed("TOPS");
	homBlock[0174] = rp_Packed("-10 ");
	homBlock[0175] = rp_Packed(" \0\0\0");
	homBlock[0176] = FORMWORD36(0, 0707070);
	homBlock[0177] = FORMWORD36(0, 0);

	for (idx1 = 0, idx2 = 0; idx2 < RP_BLKSZ18; idx1++, idx2 += 5) {
		Convert36to8(homBlock[idx1], &rp_cBuffer[idx2]);
	}
	disk_Seek(rpUnit->FileRef, 0, SEEK_SET);
	disk_Write(rpUnit->FileRef, rp_cBuffer);

	homBlock[0177] = FORMWORD36(0, 10);

	for (idx1 = 0, idx2 = 0; idx2 < RP_BLKSZ18; idx1++, idx2 += 5) {
		Convert36to8(homBlock[idx1], &rp_cBuffer[idx2]);
	}
	disk_Seek(rpUnit->FileRef, 10, SEEK_SET);
	disk_Write(rpUnit->FileRef, rp_cBuffer);

	// Initialize BAT Block
	memset(&batBlock, 0, sizeof(batBlock));
	batBlock[0000] = PackedASCII6("BAT");
	batBlock[0001] = FORMWORD36(-174, 2);
	batBlock[0176] = FORMWORD36(0, 0606060);
	batBlock[0177] = FORMWORD36(0, 1);

	for (idx1 = 0, idx2 = 0; idx2 < RP_BLKSZ18; idx1++, idx2 += 5) {
		Convert36to8(batBlock[idx1], &rp_cBuffer[idx2]);
	}
	disk_Seek(rpUnit->FileRef, 1, SEEK_SET);
	disk_Write(rpUnit->FileRef, rp_cBuffer);

	batBlock[0177] = FORMWORD36(0, 11);

	for (idx1 = 0, idx2 = 0; idx2 < RP_BLKSZ18; idx1++, idx2 += 5) {
		Convert36to8(batBlock[idx1], &rp_cBuffer[idx2]);
	}
	disk_Seek(rpUnit->FileRef, 11, SEEK_SET);
	disk_Write(rpUnit->FileRef, rp_cBuffer);

	return EMU_OK;
}

UNIT *rp_SetUnit(UNIT *rpUnit)
{
	return rpUnit;
}

void rp_SetATA(UNIT *rpUnit)
{
	RPUNIT *rpData  = (RPUNIT *)rpUnit->uData;
	UNIT   *pUnit   = rpUnit->pUnit;
	DEVICE *pDevice = pUnit->Device;

	rpData->rpds |= (RPDS_ATA|RPDS_DRY);
	pDevice->SetATA(pUnit, rpUnit->idUnit);
}

void rp_ClearATA(UNIT *rpUnit)
{
	RPUNIT *rpData  = (RPUNIT *)rpUnit->uData;
	UNIT   *pUnit   = rpUnit->pUnit;
	DEVICE *pDevice = pUnit->Device;

	rpData->rpds &= ~RPDS_ATA;
	pDevice->ClearATA(pUnit, rpUnit->idUnit);
}

void rp_Ready(UNIT *rpUnit)
{
	RPUNIT *rpData  = (RPUNIT *)rpUnit->uData;
	UNIT   *pUnit   = rpUnit->pUnit;
	DEVICE *pDevice = pUnit->Device;

	rpData->rpds |= RPDS_DRY;
	pDevice->Ready(pUnit);
}

int rp_WriteBlock(UNIT *rpUnit)
{
	RPUNIT *rpData   = (RPUNIT *)rpUnit->uData;
	UNIT   *rhUnit   = rpUnit->pUnit;
	DEVICE *rhDevice = rhUnit->Device;
	int36  data36;
	int    idx;
	int    st = EMU_OK;

	for (idx = 0; idx < RP_BLKSZ18; idx += 5) {
		if (st = rhDevice->CheckWordCount(rhUnit))
			break;
		rhDevice->ReadData36(rhUnit, &data36, FALSE);
		Convert36to8(data36, &rp_cBuffer[idx]);
	}

	if (idx)
		return disk_Write(rpUnit->FileRef, rp_cBuffer);
	else
		return st;
}

int rp_CheckBlock(UNIT *rpUnit)
{
	RPUNIT *rpData   = (RPUNIT *)rpUnit->uData;
	UNIT   *rhUnit   = rpUnit->pUnit;
	DEVICE *rhDevice = rhUnit->Device;
	int36  data1, data2;
	int    idx;
	int    st = EMU_OK;

	if ((st = disk_Read(rpUnit->FileRef, rp_cBuffer)) >= 0) {
		for (idx = 0; idx < RP_BLKSZ18; idx += 5) {
			data1 = Convert8to36(&rp_cBuffer[idx]);
			rhDevice->ReadData36(rhUnit, &data2, FALSE);
			if (data1 != data2)
				return -2;
			if (st = rhDevice->CheckWordCount(rhUnit))
				return st;
		}
	}

	return st;
}

int rp_ReadBlock(UNIT *rpUnit)
{
	RPUNIT *rpData   = (RPUNIT *)rpUnit->uData;
	UNIT   *rhUnit   = rpUnit->pUnit;
	DEVICE *rhDevice = rhUnit->Device;
	int36  data36;
	int    idx;
	int    st = EMU_OK;

	if ((st = disk_Read(rpUnit->FileRef, rp_cBuffer)) >= 0) {
		for (idx = 0; idx < RP_BLKSZ18; idx += 5) {
			data36 = Convert8to36(&rp_cBuffer[idx]);
			rhDevice->WriteData36(rhUnit, data36, FALSE);
			if (st = rhDevice->CheckWordCount(rhUnit))
				return st;
		}
	}

	return st;
}

int32 rp_ReadIO(UNIT *rpUnit, int32 reg)
{
	RPUNIT *rpData   = (RPUNIT *)rpUnit->uData;
	char   *diskName = rpUnit->dType->Name;
	int32  data;

	switch (reg >> 1) {
		case RPCS1:
			data = rpData->rpcs1;
			break;

		case RPDA:
			data = rpData->rpda;
			break;

		case RPDS:
			data = rpData->rpds;
			break;

		case RPER1:
			data = rpData->rper1;
			break;

		case RPLA:
			// Increase sector count by one
			rpData->rpla += (1 << RPLA_P_SC);
			if ((rpData->rpla >> RPLA_P_SC) == rpUnit->dType->Sectors)
				rpData->rpla = 0;

			data = rpData->rpla;
			break;

		case RPMR:
			data = rpData->rpmr;
			break;

		case RPDT:
			data = rpData->rpdt;
			break;

		case RPSN:
			data = rpData->rpsn;
			break;

		case RPOF:
			data = rpData->rpof;
			break;

		case RPDC:
			data = rpData->rpdc;
			break;

		case RPCC:
			data = rpData->rpcc;
			break;

		case RPER2:
			data = rpData->rper2;
			break;

		case RPER3:
			data = rpData->rper3;
			break;

		case RPEC1:
			data = rpData->rpec1;
			break;

		case RPEC2:
			data = rpData->rpec2;
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) Unknown MASSBUS Register %02o\n",
					diskName, reg);
#endif /* DEBUG */
			rpData->rper1 |= RPER1_ILR;
			rpData->rpds  |= RPDS_ERR;
			return 0;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: %s (%02o) => %06o\n",
			diskName, rp_regNames[reg >> 1], reg, data);
#endif /* DEBUG */

	return data;
}

void rp_WriteIO(UNIT *rpUnit, int32 reg, int32 data)
{
	RPUNIT *rpData = (RPUNIT *)rpUnit->uData;
	char   *diskName = rpUnit->dType->Name;

	switch (reg >> 1) {
		case RPCS1:
			rpData->rpcs1 = (rpData->rpcs1 & ~RPCS1_WR) | (data & RPCS1_WR);
			if (rpData->rpcs1 & RPCS1_GO) {
				rp_ClearATA(rpUnit);
				rp_Go(rpUnit);
			}
			break;

		case RPDA:
			rpData->rpda = data;
			break;

		case RPMR:
			rpData->rpmr = data;
			break;

		case RPOF:
			rpData->rpof = data;
			break;

		case RPDC:
			rpData->rpdc = data & RPDC_MASK;
			break;

		case RPDS:
		case RPLA:
		case RPDT:
		case RPSN:
		case RPCC:
		case RPER2:
		case RPER3:
		case RPEC1:
		case RPEC2:
			// Write to that registers had been ignored - No Action
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) Unknown MASSBUS Register %o\n",
					rpUnit->dType->Name, reg);
#endif /* DEBUG */
			rpData->rper1 |= RPER1_ILR;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS) && (rpData->rper1 & RPER1_ILR))
		dbg_Printf("%s: %s (%02o) <= %06o\n",
			diskName, rp_regNames[reg >> 1], reg, data);
#endif /* DEBUG */

	if (rpData->rper1 || rpData->rper2 || rpData->rper3) {
		rpData->rpds |= RPDS_ERR;
		if (rpData->rper2 || rpData->rper3)
			rpData->rper1 |= RPER1_UNS;
	}
}

void rp_Go(UNIT *rpUnit)
{
	RPUNIT *rpData   = (RPUNIT *)rpUnit->uData;
	DTYPE  *rpType   = rpUnit->dType;
	char   *diskName = rpType->Name;
	int    function  = (rpData->rpcs1 & RPCS1_FUNC) >> 1;

	if (function != FNC_DCLR) {
		if (rpData->rpds & RPDS_ERR) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d - Function isn't accetped when DS_ERR is set.\n",
					diskName, rpUnit->idUnit);
#endif /* DEBUG */
			return;
		}

		if (!(rpData->rpds & RPDS_MOL)) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d - Medium is not on-line!\n",
					diskName, rpUnit->idUnit);
#endif /* DEBUG */
			rpData->rper1 |= RPER1_UNS;
			rpData->rpds  |= RPDS_ERR;
			rpData->rpcs1 &= ~RPCS1_GO;
			rp_SetATA(rpUnit);
			return;
		}
	}

	rpData->rpds &= ~RPDS_LST;

	switch (function) {
		case FNC_NOP:
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: Unit %d - No Operation\n",
				diskName, rpUnit->idUnit);
#endif /* DEBUG */
         rpData->rpcs1 &= ~RPCS1_GO;
			break;

		case FNC_RECAL:
			rpData->rpdc = 0;

		case FNC_SEEK:
		case FNC_SEARCH:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS)) {
				if (function == FNC_RECAL)
					dbg_Printf("%s: Unit %d - Recalibrate\n",
						diskName, rpUnit->idUnit);
				else if (function == FNC_SEEK)
					dbg_Printf("%s: Unit %d - Seek\n",
						diskName, rpUnit->idUnit);
				else if (function == FNC_SEARCH)
					dbg_Printf("%s: Unit %d - Search\n",
						diskName, rpUnit->idUnit);
			}
#endif /* DEBUG */
			if ((GetCylinder(rpData->rpdc) >= rpType->Cylinders) ||
			    (GetTrack(rpData->rpda)    >= rpType->Tracks)    ||
			    (GetSector(rpData->rpda)   >= rpType->Sectors)) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS)) {
					dbg_Printf("%s: Unit %d - Invalid Address Error\n",
						diskName, rpUnit->idUnit);
					dbg_Printf("%s: C %d, T %d, S %d (Max C %d, T %d, S %d)\n",
						diskName, GetCylinder(rpData->rpdc), GetTrack(rpData->rpda),
						GetSector(rpData->rpda), rpType->Cylinders,
						rpType->Tracks, rpType->Sectors);
				}
#endif /* DEBUG */
				rpData->rper1 |= RPER1_IAE;
				break;
			}
			rpData->rpds = (rpData->rpds & ~RPDS_DRY) | RPDS_PIP;
			rp_Process(rpUnit);
			break;

		case FNC_DCLR:
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: Unit %d - Drive Clear\n",
				diskName, rpUnit->idUnit);
#endif /* DEBUG */
			rp_Reset(rpUnit);
         rpData->rpcs1 &= ~RPCS1_GO;
			break;

		case FNC_OFFSET:
		case FNC_RETURN:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d - Offset\n",
					diskName, rpUnit->idUnit);
#endif /* DEBUG */
			rpData->rpds = (rpData->rpds & ~RPDS_DRY) | RPDS_PIP;
			rp_Process(rpUnit);
			break;

		case FNC_PRESET:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d - Read-In Preset\n",
					diskName, rpUnit->idUnit);
#endif /* DEBUG */
			rpData->rpda = 0;
			rpData->rpof = 0;
			rpData->rpdc = 0;
			rpData->rpcs1 &= ~RPCS1_GO;
			break;

		case FNC_PACK:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d - Pack Acknowledge\n",
					diskName, rpUnit->idUnit);
#endif /* DEBUG */
			rpData->rpds |= RPDS_VV;
			rpData->rpcs1 &= ~RPCS1_GO;
			break;

		case FNC_WR_DATA:
		case FNC_CHK_DATA:
		case FNC_RD_DATA:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS)) {
				if (function == FNC_WR_DATA)
					dbg_Printf("%s: Unit %d - Write Data Forward\n",
						diskName, rpUnit->idUnit);
				else if (function == FNC_CHK_DATA)
					dbg_Printf("%s: Unit %d - Check Data Forward\n",
						diskName, rpUnit->idUnit);
				else if (function == FNC_RD_DATA)
					dbg_Printf("%s: Unit %d - Read Data Forward\n",
						diskName, rpUnit->idUnit);
			}
#endif /* DEBUG */
			if ((GetCylinder(rpData->rpdc) >= rpType->Cylinders) ||
			    (GetTrack(rpData->rpda)    >= rpType->Tracks)    ||
			    (GetSector(rpData->rpda)   >= rpType->Sectors)) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS)) {
					dbg_Printf("%s: Unit %d - Invalid Address Error\n",
						diskName, rpUnit->idUnit);
					dbg_Printf("%s: C %d, T %d, S %d (Max C %d, T %d, S %d)\n",
						diskName, GetCylinder(rpData->rpdc), GetTrack(rpData->rpda),
						GetSector(rpData->rpda), rpType->Cylinders,
						rpType->Tracks, rpType->Sectors);
				}
#endif /* DEBUG */
				rpData->rper1 |= RPER1_IAE;
				rp_SetATA(rpUnit);
				break;
			}
			rpData->rpds &= ~RPDS_DRY;
			rp_Process(rpUnit);
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d - Unknown function code %02o\n", diskName,
					rpUnit->idUnit, (rpData->rpcs1 & (RPCS1_FUNC|RPCS1_GO)));
#endif /* DEBUG */
         rpData->rpcs1 &= ~RPCS1_GO;
         rpData->rper1 |= RPER1_ILF;
         rp_SetATA(rpUnit);
			break;
	}
}

int rp_Process(UNIT *rpUnit)
{
	RPUNIT *rpData   = (RPUNIT *)rpUnit->uData;
	DTYPE  *rpType   = rpUnit->dType;
	UNIT   *rhUnit   = rpUnit->pUnit;
	DEVICE *rhDevice = rhUnit->Device;
	char   *diskName = rpType->Name;
	int    function  = (rpData->rpcs1 & RPCS1_FUNC) >> 1;
	int    diskAddr;
	int    temp, st;
	int    savedMode = 0; // temp.

	if (rpData->rper1 || rpData->rper2 || rpData->rper3) {
		rpData->rpds |= RPDS_ERR;
		if (rpData->rper2 || rpData->rper3)
			rpData->rper1 |= RPER1_UNS;
	}

	switch (function) {
		case FNC_RECAL:
		case FNC_SEEK:
		case FNC_SEARCH:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA)) {
				if (function == FNC_RECAL)
					dbg_Printf("%s: Unit %d - Recalibrate\n",
						diskName, rpUnit->idUnit);
				else if (function == FNC_SEEK)
					dbg_Printf("%s: Unit %d - Seek\n",
						diskName, rpUnit->idUnit);
				else if (function == FNC_SEARCH)
					dbg_Printf("%s: Unit %d - Search\n",
						diskName, rpUnit->idUnit);
			}
#endif /* DEBUG */
			diskAddr = GetDiskAddr(rpData->rpdc, rpData->rpda, rpType);

			st = disk_Seek(rpUnit->FileRef, diskAddr, SEEK_SET);
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA) && st >= 0)
				dbg_Printf("%s: Unit %d - Move to disk address %08X\n",
					diskName, rpUnit->idUnit, st);
#endif /* DEBUG */
			rpData->rpcc = rpData->rpdc;
			rpData->rpds &= ~RPDS_PIP;
			rp_SetATA(rpUnit);
			break;

		case FNC_OFFSET:
		case FNC_RETURN:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA)) {
				dbg_Printf("%s: Unit %d - Process: Offset\n",
					diskName, rpUnit->idUnit);
			}
#endif /* DEBUG */
			rpData->rpds &= ~RPDS_PIP;
			rp_SetATA(rpUnit);
			break;

		case FNC_WR_DATA:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: Unit %d - Process: Write Data Forward\n",
					diskName, rpUnit->idUnit);
#endif /* DEBUG */
			if (rpData->rpds & RPDS_WRL) {
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: Unit %d - Write Lock Error\n",
						diskName, rpUnit->idUnit);
#endif /* DEBUG */
				rpData->rper1 |= RPER1_WLE;
				break;
			}

		case FNC_CHK_DATA:
		case FNC_RD_DATA:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA)) {
				if (function == FNC_CHK_DATA)
					dbg_Printf("%s: Unit %d - Process: Check Data Forward\n",
						diskName, rpUnit->idUnit);
				else if (function == FNC_RD_DATA)
					dbg_Printf("%s: Unit %d - Process: Read Data Forward\n",
						diskName, rpUnit->idUnit);
			}
#endif /* DEBUG */
			
			diskAddr = GetDiskAddr(rpData->rpdc, rpData->rpda, rpType);

			do {
				if (diskAddr >= rpType->Blocks) {
					rpData->rper1 |= RPER1_AOE;
					break;
				}

				st = disk_Seek(rpUnit->FileRef, diskAddr, SEEK_SET);
				if (st >= 0) {
					switch (function) {
						case FNC_WR_DATA:
							st = rp_WriteBlock(rpUnit);
							break;

						case FNC_CHK_DATA:
							st = rp_CheckBlock(rpUnit);
							break;

						case FNC_RD_DATA:
							st = rp_ReadBlock(rpUnit);
							break;
					}

					// Update RPDA and RPDC registers
					diskAddr++;
					temp = diskAddr % rpType->Sectors;
					if (temp == (rpType->Sectors - 1))
						rpData->rpds |= RPDS_LST;
					else
						rpData->rpds &= ~RPDS_LST;
					rpData->rpda = temp;
					temp = diskAddr / rpType->Sectors;
					rpData->rpda |= (temp % rpType->Tracks) << RPDA_P_TA;
					rpData->rpdc = temp / rpType->Tracks;
				}
			} while (rhDevice->CheckWordCount(rhUnit) == EMU_OK);

			break;
	}

	// Check any errors. If so, set ATA bit.
	if (rpData->rper1 || rpData->rper2 || rpData->rper3)
		rp_SetATA(rpUnit);

	// When process is done, GO bit had been turned off.
	// Also, inform RH11 controller this unit is ready now.
	rpData->rpcs1 &= ~RPCS1_GO;
	rp_Ready(rpUnit);

	return EMU_OK;
}

int rp_Boot(UNIT *rpUnit, int argc, char **argv)
{
	return pdp10_Boot(rpUnit, BOOT_DISK, argc, argv);
}
