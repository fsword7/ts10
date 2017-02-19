// tm.c - TM MASSBUS-based tape drive routines
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
// See ReadMe for copyright notice.
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
#include "emu/tape.h"
#include "dev/rh.h"
#include "dev/tm.h"
#include "dev/proto.h"

char tm_cBuffer[65536];

#ifdef DEBUG

char *tm_fncNames[] =
{
	"No Operation",          // 00 No operation
	"Unload",                // 01 Rewind, offline
	"Unknown",               // 02
	"Rewind",                // 03 Rewind
	"Drive Clear",           // 04 Drive Clear
	"Unknown",               // 05
	"Unknown",               // 06
	"Unknown",               // 07
	"Read-In Preset",        // 10 Read-In Preset
	"Unknown",               // 11
	"Erase",                 // 12 Erase
	"Write Tape Mark",       // 13 Write File Mark
	"Space Forward",         // 14 Space Forward
	"Space Reverse",         // 15 Space Reverse
	"Unknown",               // 16
	"Unknown",               // 17
	"Unknown",               // 20
	"Unknown",               // 21
	"Unknown",               // 22
	"Unknown",               // 23
	"Verify Forward",        // 24 Write Check Forward
	"Unknown",               // 25
	"Verify Reverse",        // 26 Write Check Reverse
	"Unknown",               // 27
	"Write Forward",         // 30 Write Forward
	"Unknown",               // 31
	"Unknown",               // 32
	"Unknown",               // 33
	"Read Forward",          // 34 Read Forward
	"Unknown",               // 35
	"Unknown",               // 36
	"Read Reverse"           // 37 Read Reverse
};

// Listing of name of the registers for debug facility
// Note: ^ = Mixed, * = Drive Register, space = Controller Register

char *tm_RegNames[] =
{
	"MTCS1", // (R/W) ^Control and Status Register #1
	"MTWC",  // (R/W)  Word Count Register
	"MTBA",  // (R/W)  Bus Address Register
	"MTFC",  // (R/W) *Frame Count Register
	"MTCS2", // (R/W)  Control and Status Register #2
	"MTDS",  // (R)   *Drive Status Register
	"MTER",  // (R)   *Error Register
	"MTAS",  // (R/W) *Attention Summary Register
	"MTCC",  // (R/W)  Character Check Register
	"MTDB",  // (R/W)  Data Buffer Register
	"MTMR",  // (R/W)  Maintenance Register
	"MTDT",  // (R)   *Drive Type Register
	"MTSN",  // (R)   *Serial Number Register
	"MTTC",  // (R/W) *Tape Control Register
	"MTBAE", // (R/W)  Bus Address Extension Register  (RH70 Only)
	"MTCS3"  // (R/W)  Control and Status Register #3  (RH70 Only)
};
#endif /* DEBUG */

// List of slave tape drives for TM02/03 tape controller
DTYPE tu_dTypes[] = {
	{
		"TU45",        // Device Name
		"TU45 - 800/1600 BPI Tape Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		0, 0, 0, 0,    // Not Used
		000012,        // Device Type: TU45
		NULL,          // Not Used

		RH11_TAPE,     // RH11 Flags
		NULL,          // Not Used

		tape_Open,     // Low-level Open Routine
		tape_Close,    // Low-level Close Routine
		tape_Read,     // Low-level Read Routine
		tape_Write,    // Low-level Write Routine
		tape_Mark,     // Low-level Mark Routine
		tape_Rewind,   // Low-level Rewind Routine
		tape_SkipRec,  // Low-level Skip Routine
		NULL,          // Low-level Seek Routine
		NULL           // Low-level GetDiskAddr Routine - Not used
	},

	{ NULL }
};

// List of tape controllers
DTYPE tm_dTypes[] = {
	{
		"TM02",        // Device Name
		"TM02 - Tape Controller",
		UNIT_DISABLE,
		0, 0, 0, 0,    // Not Used
		000010,        // Default Device Type
		tu_dTypes,     // Listing of slave tape drives

		RH11_CTLR,     // RH11 Flags - Controller
		NULL,          // Not used

		// Routines below - Not used due to controller
		NULL,          // Low-level Open Routine
		NULL,          // Low-level Close Routine
		NULL,          // Low-level Read Routine
		NULL,          // Low-level Write Routine
		NULL,          // Low-level Rewind Routine
		NULL,          // Low-level Skip Routine
		NULL,          // Low-level Seek Routine
		NULL           // Low-level GetDiskAddr Routine
	},

	{
		"TM03",        // Device Name
		"TM03 - Tape Controller",
		UNIT_DISABLE,
		0, 0, 0, 0,    // Not Used
		000050,        // Default Device Type
		tu_dTypes,     // Listing of slave tape drives

		RH11_CTLR,     // RH11 Flags - Controller
		NULL,          // Not Used

		// Routines below - Not used due to controller
		NULL,          // Low-level Open Routine
		NULL,          // Low-level Close Routine
		NULL,          // Low-level Read Routine
		NULL,          // Low-level Write Routine
		NULL,          // Low-level Rewind Routine
		NULL,          // Low-level Skip Routine
		NULL,          // Low-level Seek Routine
		NULL           // Low-level GetDiskAddr Routine
	},

	{ NULL }
};

DEVICE tm_Device =
{
	"TM",          // Device Name
	"MASSBUS Tape Controllers",
	"v0.8.2 (Alpha)",// Version
	tm_dTypes,     // Listing of RM devices
	NULL,          // Listing of Units
	NULL,          // Listing of Slave Devices - Not used
	NULL,          // Listing of Commands
	NULL,          // Listing of Set Commands
	NULL,          // Listing of Show Commands

	0,             // Number of Devices
	TM_MAXUNITS,   // Number of Units

	tm_Initialize, // Initialize Routine
	tm_Reset,      // Reset Routine
	tm_Create,     // Create Routine
	tm_Delete,     // Delete Routine
	NULL,          // Configure Routine
	tm_Enable,     // Enable Routine
	tm_Disable,    // Disable Routine
	tm_Attach,     // Attach Routine
	tm_Detach,     // Detach Routine
	NULL,          // Format Routine - Not Used
	tm_ReadIO,     // Read I/O Routine
	tm_WriteIO,    // Write I/O Routine
	tm_Process,    // Process Routine
	tm_Boot,       // Boot Routine
	NULL,          // Execute Routine - Not Used

	tm_SetUnit,    // SetUnit Routine
	tm_SetATA,     // SetATA Routine
	tm_ClearATA    // ClearATA Routine

};

// Power Up Initialization
void tm_Initialize(UNIT *tmUnit)
{
	MTUNIT *tmData = (MTUNIT *)tmUnit->uData;
	int    idx;

	tmData->mtcs1 = MTCS1_DVA;         // DVA - Always On
	tmData->mtds  = MTDS_DPR|MTDS_DRY; // DPR - Always On
	tmData->mter  = 0;
	tmData->mttc  = 0;

	// Initialize each slave drives
	for (idx = 0; idx < TM_MAXSLAVES; idx++) {
		tmData->dStatus[idx] = 0;
		tmData->Serial[idx]  = ((tmUnit->idUnit + 1) << 3) | idx;
	}
	
	// Initialize selected transport as default slave #0
	tmUnit->cUnit  = 0;
	tmUnit->pcUnit = &tmUnit->sUnits[0];
	tmData->mtds   &= ~MTDS_SLAVE;
	tmData->mtds   |= tmData->dStatus[tmUnit->cUnit];
	tmData->mtdt   = tmData->dTypes[0];
	tmData->mtsn   = tmData->Serial[0];
}

void tm_Reset(UNIT *tmUnit)
{
	MTUNIT *tmData  = (MTUNIT *)tmUnit->uData;
	int    idx;

	// Acknowledge the slave attention - reset it.
	tmData->dStatus[tmUnit->cUnit] &= ~MTDS_SSC;

	// Reset others
	tmData->mtcs1 = MTCS1_DVA; // DVA - Always On
	tmData->mtds  &= ~(MTDS_ATA|MTDS_SLAVE|MTDS_ERR|MTDS_SSC|MTDS_SLA);
	tmData->mtds  |= tmData->dStatus[tmUnit->cUnit];
	tmData->mter  = 0;
	tmData->mttc  = 0;

	// Check any slave drive that still require slave attention
	for (idx = 0; idx < TM_MAXSLAVES; idx++)
		if (tmData->dStatus[idx] & MTDS_SSC)
			tmData->mtds |= MTDS_SSC;
}

// Enable the TM02/03 tape controller
int tm_Create(UNIT *uptr, char *devName, int argc, char **argv)
{
	int len = strlen(tm_Device.Name);

	if (devName[len] == ':') {
		if (!(uptr->Flags & UNIT_PRESENT)) {
			UNIT   *sUnits = (UNIT *)calloc(sizeof(UNIT), TM_MAXSLAVES);
			MTUNIT *tmData = (MTUNIT *)calloc(sizeof(MTUNIT), 1);

			if (sUnits && tmData) {
				int idx1, idx2;
				int st;

				for (idx1 = 0; tm_dTypes[idx1].Name; idx1++)
					if (!strcasecmp(argv[0], tm_dTypes[idx1].Name))
						break;

				if (!tm_dTypes[idx1].Name) {
					free(sUnits);
					free(tmData);
					return EMU_ARG;
				}

				*strchr(argv[1], ':') = '\0';
//				if (st = unit_mapCreateDevice(argv[1], uptr)) {
//					free(sUnits);
//					free(tmData);
//					return st;
//				}

				for (idx2 = 0; idx2 < TM_MAXSLAVES; idx2++) {
					sUnits[idx2].idUnit   = idx2;
					sUnits[idx2].Device   = &tm_Device;
					sUnits[idx2].pUnit    = uptr;
					sUnits[idx2].Flags    = tm_dTypes[idx1].Flags;
					sUnits[idx2].uData    = (void *)tmData;
					tmData->dStatus[idx2] = 0;
					tmData->dTypes[idx2]  =
						(MTDT_NSA|MTDT_TAP) | tm_dTypes[idx1].dType;
				}

				// This unit is a tape formatter (controller).
				uptr->tFlags = UT_CONTROLLER;
				uptr->dType  = &tm_dTypes[idx1];
				uptr->Device = &tm_Device;
				uptr->sUnits = sUnits;
				uptr->nUnits = TM_MAXUNITS;
				uptr->cUnit  = 0;
				uptr->pcUnit = &sUnits[0];
				uptr->Flags  = tm_dTypes[idx1].Flags | UNIT_PRESENT;
				uptr->uData  = (void *)tmData;

				ToUpper(argv[1]);
				printf("Device %s: had been created.\n", argv[1]);
			} else
				return EMU_MEMERR;
		} else
			return EMU_ARG;
	} else {
		int slave = devName[len] - '0';

		if (slave < TM_MAXSLAVES) {
			return tm_CreateSlave(uptr, slave, argc, argv);
		} else
			return EMU_ARG;
	}
	return EMU_OK;
}

// Disable the desired TM02/03 tape controller
int tm_Delete(UNIT *tmUnits, int32 unit)
{
	if (unit < TM_MAXUNITS) {
		UNIT *uptr = &tmUnits[unit];

		if (uptr->Flags & UNIT_DISABLED)
			return EMU_ARG;

		free(uptr->sUnits);
		free(uptr->uData);
		uptr->Flags |= UNIT_DISABLED;
	} else
		return EMU_ARG;
	return EMU_OK;
}

int tm_CreateSlave(UNIT *tmUnit, int32 slave, int argc, char **argv)
{
	DTYPE  *dTypes = tmUnit->dType->sTypes;
	UNIT   *sUnit  = &tmUnit->sUnits[slave];
	MTUNIT *sData  = (MTUNIT *)tmUnit->uData;
	int    idx, st;

	if (!(sUnit->Flags & UNIT_PRESENT)) {
		if (sUnit->Flags & UNIT_DISABLED)
			return EMU_DISABLED;

		if (!strcasecmp(argv[0], "off")) {
			sUnit->dType = NULL;
			sUnit->Flags = tmUnit->dType->Flags;
			sData->dTypes[slave] =
				(MTDT_NSA|MTDT_TAP) | tmUnit->dType->dType;
			return EMU_OK;
		}

		for (idx = 0; dTypes[idx].Name; idx++) {
			if (!strcasecmp(argv[0], dTypes[idx].Name))
				break;
		}

		if (dTypes[idx].Name) {
			*strchr(argv[1], ':') = '\0';
//			if (st = unit_mapCreateDevice(argv[1], sUnit))
//				return st;

			// This unit is a tape drive.
			sUnit->tFlags = UT_STORAGE;
			sUnit->dType  = &dTypes[idx];
			sUnit->Flags  = dTypes[idx].Flags | UNIT_PRESENT;
			sData->dTypes[slave] = (MTDT_NSA|MTDT_TAP|MTDT_SPR) |
				dTypes[idx].dType | (tmUnit->dType->dType & TM_TM03);

			ToUpper(argv[1]);
			printf("Device %s: had been created.\n", argv[1]);

			return EMU_OK;
		}
		return EMU_ARG;
	}
	return EMU_PRESENT;
}

int tm_Enable(UNIT *tmUnit, int32 slave)
{
	UNIT *uptr = &tmUnit->sUnits[slave];

	if (!(uptr->Flags & UNIT_DISABLED)) {
		uptr->Flags |= UNIT_DISABLED;
		return EMU_OK;
	}
	return EMU_ARG;
}

int tm_Disable(UNIT *tmUnit, int32 slave)
{
	UNIT *uptr = &tmUnit->sUnits[slave];
	if (uptr->Flags & UNIT_DISABLED) {
		uptr->Flags &= ~UNIT_DISABLED;
		return EMU_OK;
	}
	return EMU_ARG;
}

int tm_Attach(UNIT *tmUnit, char *fileName)
{
	UNIT   *pUnit  = tmUnit->pUnit;
	MTUNIT *tmData = (MTUNIT *)pUnit->uData;
	int  st;

	if (tmUnit->Flags & UNIT_DISABLED)
		return EMU_DISABLED;

	if (tmUnit->Flags & UNIT_ATTACHED)
		return EMU_ATTACHED;

	// Attach a file to that unit
	if (st = tape_Open(tmUnit, fileName, 0))
		return st;

	tmUnit->Flags |= UNIT_ATTACHED;

	// Now let's set some registers for medium on-line
	tmData->dStatus[tmUnit->idUnit] = (MTDS_MOL|MTDS_SSC|MTDS_BOT);
	if (pUnit->cUnit == tmUnit->idUnit)
		tmData->mtds = (tmData->mtds & ~MTDS_SLAVE) |
			tmData->dStatus[tmUnit->idUnit] | MTDS_SLA;
	tmData->mtds |= MTDS_SSC;
	tm_SetATA(tmUnit->pUnit);

	return EMU_OK;
}

int tm_Detach(UNIT *tmUnit)
{
	UNIT   *pUnit  = tmUnit->pUnit;
	MTUNIT *tmData = (MTUNIT *)pUnit->uData;
	int st;

	if (tmUnit->Flags & UNIT_DISABLED)
		return EMU_DISABLED;

	if (!(tmUnit->Flags & UNIT_ATTACHED))
		return EMU_ARG;

	// Detach a file from that unit.
	if (st = tape_Close(tmUnit))
		return st;

	tmUnit->Flags &= ~(UNIT_ATTACHED|UNIT_WRLOCKED);

	// Now let's set some registers for medium off-line

	tmData->dStatus[tmUnit->idUnit] = MTDS_SSC;
	if (tmUnit->idUnit == pUnit->cUnit) {
		tmData->mtds &= ~MTDS_SLAVE;
	}
	tmData->mtds |= MTDS_SSC;
	tm_SetATA(tmUnit->pUnit);

	return EMU_OK;
}

UNIT *tm_SetUnit(UNIT *tmUnits, int32 unit)
{
	if (unit < TM_MAXUNITS) {
		if (!(tmUnits[unit].Flags & UNIT_DISABLED))
			return &tmUnits[unit];
	}
	return NULL;
}

void tm_SetATA(UNIT *tmUnit)
{
	MTUNIT *tmData  = (MTUNIT *)tmUnit->uData;
	UNIT   *pUnit   = tmUnit->pUnit;
	DEVICE *pDevice = pUnit->Device;

	tmData->mtds |= MTDS_ATA;
	pDevice->SetATA(pUnit, tmUnit->idUnit);
}

void tm_ClearATA(UNIT *tmUnit)
{
	MTUNIT *tmData  = (MTUNIT *)tmUnit->uData;
	UNIT   *pUnit   = tmUnit->pUnit;
	DEVICE *pDevice = pUnit->Device;

	tmData->mtds &= ~MTDS_ATA;
	pDevice->ClearATA(pUnit, tmUnit->idUnit);
}

void tm_Ready(UNIT *tmUnit)
{
	MTUNIT *tmData  = (MTUNIT *)tmUnit->uData;
	UNIT   *pUnit   = tmUnit->pUnit;
	DEVICE *pDevice = pUnit->Device;

	tmData->mtds |= MTDS_DRY;
	pDevice->Ready(pUnit);
}

void tm_SetStatus(UNIT *tmUnit, int16 data)
{
	UNIT   *pUnit  = tmUnit->pUnit;
	MTUNIT *tmData = (MTUNIT *)pUnit->uData;

	tmData->dStatus[tmUnit->idUnit] |= data;
	if (tmUnit->idUnit == pUnit->cUnit)
		tmData->mtds = (tmData->mtds & ~MTDS_SLAVE) |
			tmData->dStatus[tmUnit->idUnit];
}

void tm_ClearStatus(UNIT *tmUnit, int16 data)
{
	UNIT   *pUnit  = tmUnit->pUnit;
	MTUNIT *tmData = (MTUNIT *)pUnit->uData;

	tmData->dStatus[tmUnit->idUnit] &= ~data;
	if (tmUnit->idUnit == pUnit->cUnit)
		tmData->mtds = (tmData->mtds & ~MTDS_SLAVE) |
			tmData->dStatus[tmUnit->idUnit];
}

int16 tm_CheckStatus(UNIT *tmUnit, int16 data)
{
	UNIT   *pUnit  = tmUnit->pUnit;
	MTUNIT *tmData = (MTUNIT *)pUnit->uData;

	return tmData->dStatus[tmUnit->idUnit] & data;
}

int tm_ReadData(UNIT *tmUnit, int fc)
{
	MTUNIT *tmData   = (MTUNIT *)tmUnit->uData;
	UNIT   *rhUnit   = tmUnit->pUnit;
	DEVICE *rhDevice = rhUnit->Device;
	int    format    = (tmData->mttc & MTTC_FMTSEL) >> 4;
	int36  data36;
	int    st;
	uint   idx = -1;

	switch (format) {
		case FMT10_COREDUMP:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: PDP-10 Format - Coredump\n",
					tmUnit->dType->Name);
#endif /* DEBUG */

			fc = 0200000 - fc;
//			dbg_Printf("READ: %d bytes being read from memory.\n", fc);

			for (idx = 0; idx < fc; idx += 5) {
				if (st = rhDevice->CheckWordCount(rhUnit))
					break;
				rhDevice->ReadData36(rhUnit, &data36, TRUE);
				Convert36to8(data36, &tm_cBuffer[idx]);
			}
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: Format not supported - code %02o\n",
					tmUnit->dType->Name, format);
#endif /* DEBUG */
			break;
	}

	return idx;
}

void tm_WriteData(UNIT *tmUnit, int fc)
{
	MTUNIT *tmData   = (MTUNIT *)tmUnit->uData;
	UNIT   *rhUnit   = tmUnit->pUnit;
	DEVICE *rhDevice = rhUnit->Device;
	int    format    = (tmData->mttc & MTTC_FMTSEL) >> 4;
	int36  data36;
	int    idx;

	switch (format) {
		case FMT10_COREDUMP:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: PDP-10 Format - Coredump\n",
					tmUnit->dType->Name);
#endif /* DEBUG */

//			fc = 0200000 - fc;
			for (idx = 0; idx < fc; idx += 5) {
				data36 = Convert8to36(&tm_cBuffer[idx]);
				rhDevice->WriteData36(rhUnit, data36, TRUE);
			}
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: Format not supported - code %02o\n",
					tmUnit->dType->Name, format);
#endif /* DEBUG */
			break;
	}
}

int32 tm_ReadIO(UNIT *tmUnit, int32 reg)
{
	MTUNIT *tmData = (MTUNIT *)tmUnit->uData;
	int32  data;

	switch (reg >> 1) {
		case MTCS1:
			data = tmData->mtcs1;
			break;

		case MTFC:
			data = tmData->mtfc;
			break;

		case MTDS:
			data = tmData->mtds;
			break;

		case MTER:
			data = tmData->mter;
			break;

		case MTCC:
			data = tmData->mtcc;
			break;

		case MTMR:
			data = tmData->mtmr;
			break;

		case MTDT:
			data = tmData->mtdt;
			break;

		case MTSN:
			data = tmData->mtsn;
			break;

		case MTTC:
			data = tmData->mttc;
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) Unknown MASSBUS Register %o\n",
					tmUnit->dType->Name, reg);
#endif /* DEBUG */
			tmData->mter |= MTER_ILR;
			return 0;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		if ((reg >> 1) == MTFC) // Frame Count Register
			dbg_Printf("%s: %s (%02o) => %06o (%d Bytes)\n",
				tmUnit->dType->Name, tm_RegNames[reg >> 1], reg, data,
				-((data & 0x8000) ? data | 0xFFFF0000 : data));
		else
			dbg_Printf("%s: %s (%02o) => %06o\n",
				tmUnit->dType->Name, tm_RegNames[reg >> 1], reg, data);
	}
#endif /* DEBUG */
	data &= 0xFFFF;

	return data;
}

void tm_WriteIO(UNIT *tmUnit, int32 reg, int32 data)
{
	MTUNIT *tmData = (MTUNIT *)tmUnit->uData;
	int    newSlave;
	int    idx;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		if ((reg >> 1) == MTFC) // Frame Count Register
			dbg_Printf("%s: %s (%02o) <= %06o (%d Bytes)\n",
				tmUnit->dType->Name, tm_RegNames[reg >> 1], reg, data,
				-((data & 0x8000) ? data | 0xFFFF0000 : data));
		else
			dbg_Printf("%s: %s (%02o) <= %06o\n",
				tmUnit->dType->Name, tm_RegNames[reg >> 1], reg, data);
	}
#endif /* DEBUG */

	switch (reg >> 1) {
		case MTCS1:
			tmData->mtcs1 = (tmData->mtcs1 & ~MTCS1_WR) | (data & MTCS1_WR);
			if (tmData->mtcs1 & MTCS1_GO) {
				tm_ClearATA(tmUnit);
				tm_Go(tmUnit);
			}
			break;

		case MTFC:
			if (!(tmData->mtcs1 & MTCS1_GO)) {
				tmData->mtfc = data;
				tmData->mttc |= MTTC_FCS;
			}
			break;

		case MTTC:
			newSlave       = (data & MTTC_SLAVE);
			tmUnit->cUnit  = newSlave;
			tmUnit->pcUnit = &tmUnit->sUnits[newSlave];
			tmData->mtds   = (tmData->mtds & ~MTDS_SLAVE) |
				tmData->dStatus[newSlave];
			tmData->mtdt   = tmData->dTypes[newSlave];
			if (tmUnit->pcUnit->Flags & UNIT_PRESENT)
				tmData->mtsn   = tmData->Serial[newSlave];
			else
				tmData->mtsn   = 0;
			tmData->mttc   = (tmData->mttc & ~MTTC_WR) | (data & MTTC_WR);

			// Check any slave drive that still require slave attention
			for (idx = 0; idx < TM_MAXSLAVES; idx++)
				if (tmData->dStatus[idx] & MTDS_SSC)
					tmData->mtds |= MTDS_SSC;

			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) Unknown MASSBUS Register %o\n",
					tmUnit->dType->Name, reg);
#endif /* DEBUG */
			tmData->mter |= MTER_ILR;
			break;
	}

	if (tmData->mter)
		tmData->mtds |= MTDS_ERR;
}

void tm_Go(UNIT *tmUnit)
{
	MTUNIT *tmData   = (MTUNIT *)tmUnit->uData;
	UNIT   *sUnit    = tmUnit->pcUnit;
	int    function  = (tmData->mtcs1 & MTCS1_FUNC) >> 1;
	char   *tapeName = tmUnit->dType->Name;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: Unit %d (Slave %d) - Function: %s\n", tapeName,
			tmUnit->idUnit, tmUnit->cUnit, tm_fncNames[function]);
#endif /* DEBUG */

	if (function != FNC_DCLR) {
		if (tmData->mtds & MTDS_ERR) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Function - Not accetped when DS_ERR is set.\n",
					tmUnit->dType->Name);
#endif /* DEBUG */
			return;
		}

		if (!(tmData->dStatus[sUnit->idUnit] & MTDS_MOL)) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Medium is not online!\n", tapeName);
#endif /* DEBUG */
			tmData->mter  |= MTER_UNS;
			tmData->mtds  |= MTDS_ERR;
			tmData->mtcs1 &= ~MTCS1_GO;
			tm_Ready(tmUnit);
			return;
		}
	}

	switch (function) {
		case FNC_NOP:          // No Operation
			tmData->mtcs1 &= ~MTCS1_GO;
			tm_Ready(tmUnit);
			break;

		case FNC_UNLOAD:       // Unload (Rewind, offline)
		case FNC_REWIND:       // Rewind
			tm_SetStatus(sUnit, MTDS_PIP);
			tm_Process(tmUnit);
			break;

		case FNC_DCLR:         // Drive Clear
			tm_Reset(tmUnit);
			tmData->mtcs1 &= ~MTCS1_GO;
			tm_Ready(tmUnit);
			break;

		case FNC_SP_FWD:       // Space Forward
			if ((tmData->mttc & MTTC_FCS) == 0) {
				tmData->mter |= MTER_NEF;
				break;
			}

			tm_SetStatus(sUnit, MTDS_PIP);
			tm_Process(tmUnit);
			break;

		case FNC_SP_REV:       // Space Reverse
			if ((tmData->Position[tmUnit->cUnit] == TM_BOT) ||
			    ((tmData->mttc & MTTC_FCS) == 0)) {
				tmData->mter |= MTER_NEF;
				break;
			}

			tm_SetStatus(sUnit, MTDS_PIP);
			tm_Process(tmUnit);
			break;

		case FNC_RD_REV:       // Read Reverse
		case FNC_CHK_REV:      // Verify Reverse
			if (tmData->Position[tmUnit->cUnit] == TM_BOT) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Error - Already Bottom of Tape\n", tapeName);
#endif /* DEBUG */
				tmData->mter |= MTER_NEF;
				break;
			}
			break;

		case FNC_WR_FWD:       // Write Forward
			if ((tmData->mttc & MTTC_FCS) == 0) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Error - Zero Frame Count\n", tapeName);
#endif /* DEBUG */
				tmData->mter |= MTER_NEF;
				break;
			}
		case FNC_WR_EOF:       // Write Tape Mark
		case FNC_ERASE:        // Erase Tape
			if (tmData->mtds & MTDS_WRL) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Error - Write Locked\n", tapeName);
#endif /* DEBUG */
				tmData->mter |= MTER_NEF;
				break;
			}
		case FNC_CHK_FWD:      // Verify Forward
		case FNC_RD_FWD:       // Read Forward
			if ((sUnit->Flags & UNIT_ATTACHED) == 0) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Error - Medium not on tape drive\n", tapeName);
#endif /* DEBUG */
				tmData->mter |= MTER_UNS;
				break;
			}
			tm_Process(tmUnit);
			break;

		default:                // Illegal Function
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unit %d (Slave %d) - Unknown function code %02o\n",
					tmUnit->dType->Name, tmUnit->idUnit, tmUnit->cUnit,
					(tmData->mtcs1 & (MTCS1_FUNC|MTCS1_GO)));
#endif /* DEBUG */
			tmData->mtcs1 &= ~MTCS1_GO;
			tmData->mter |= MTER_ILF;
			tm_Ready(tmUnit);
			break;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS) && tmData->mter) {
		if (tmData->mter & MTER_UNS)
			dbg_Printf("%s: Unsafe Operation - Aborted.\n", tapeName);
		if (tmData->mter & MTER_NEF)
			dbg_Printf("%s: Non-Executable Function - Aborted.\n", tapeName);
	}
#endif /* DEBUG */
}

int tm_Process(UNIT *tmUnit)
{
	MTUNIT *tmData   = (MTUNIT *)tmUnit->uData;
	UNIT   *sUnit    = tmUnit->pcUnit;
	MTAPE  *mtape    = (MTAPE *)sUnit->FileRef;
	int    slave     = tmUnit->cUnit;
	int    function  = (tmData->mtcs1 & MTCS1_FUNC) >> 1;
	char   *tapeName = sUnit->dType->Name;
	int    count     = 0; // number of bytes had been transferred
	int    len, rc;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Unit %d (Slave %d) - Process: %s\n", tapeName,
			tmUnit->idUnit, tmUnit->cUnit, tm_fncNames[function]);
#endif /* DEBUG */

	switch (function) {
		case FNC_UNLOAD: // Unload (Rewind, offline)
		case FNC_REWIND: // Rewind
			rc = tape_Rewind(sUnit);
			if (function == FNC_UNLOAD)
				tm_Detach(sUnit);
			else {
				tmData->Position[slave] = TM_BOT;
				tm_ClearStatus(sUnit, MTDS_PIP);
				tm_SetStatus(sUnit, MTDS_SSC|MTDS_BOT);
				tm_SetATA(tmUnit);
			}
			break;

		case FNC_SP_FWD: // Space Forward
		case FNC_SP_REV: // Space Reverse
			do {
				tmData->mtfc++;

				rc = tape_SkipRec(sUnit, (function == FNC_SP_FWD) ? 1 : -1);
				tm_ClearStatus(sUnit, MTDS_EOT|MTDS_TM|MTDS_BOT);
				if (rc <= 0) {
					switch (rc) {
						case MT_TMARK:
							tmData->Position[slave] = TM_MARK; // Tape Mark
							tm_SetStatus(sUnit, MTDS_TM);
#ifdef DEBUG
							if (dbg_Check(DBG_IODATA))
								dbg_Printf("%s: ** Tape Mark **\n", tapeName);
#endif /* DEBUG */
							break;

						case MT_EOT:
							tmData->Position[slave] = TM_EOT; // End of Tape
							tm_SetStatus(sUnit, MTDS_EOT);
#ifdef DEBUG
							if (dbg_Check(DBG_IODATA))
								dbg_Printf("%s: ** End of Tape **\n", tapeName);
#endif /* DEBUG */
							break;

						case MT_BOT:
							tmData->Position[slave] = TM_BOT; // Bottom of Tape
							tm_SetStatus(sUnit, MTDS_BOT);
#ifdef DEBUG
							if (dbg_Check(DBG_IODATA))
								dbg_Printf("%s: ** Bottom of Tape **\n", tapeName);
#endif /* DEBUG */
							break;

						case MT_ERROR:
#ifdef DEBUG
							dbg_Printf("%s: ** Error Code: %d (%s)\n",
								tapeName, mtape->error, strerror(mtape->error));
#endif /* DEBUG */
							break;
					}

					// Get out of loop.
					break;
				} else {
					tmData->Position[slave] = TM_MOT; // Middle of Tape
					count++;
				}

			} while (tmData->mtfc);

#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: %d record%s had been skipped\n",
					tapeName, count, ((count > 1) ? "s" : ""));
#endif /* DEBUG */

			if (tmData->mtfc)
				tmData->mter |= MTER_FCE;
			else
				tmData->mttc &= ~MTTC_FCS;

			tm_ClearStatus(sUnit, MTDS_PIP);
			tm_SetATA(tmUnit);

			break;

		case FNC_WR_EOF: // Write Tape Mark (EOF)
			tape_Mark(sUnit);
		case FNC_ERASE:  // Erase Tape
			tm_SetATA(tmUnit);
			break;

		case FNC_WR_FWD: // Write Forward
			if (tm_CheckStatus(sUnit, MTDS_WRL)) {
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA)) {
					dbg_Printf("%s: Unit %d (Slave %d) - Write Lock Error\n",
						tapeName, tmUnit->idUnit, tmUnit->cUnit);
				}
#endif /* DEBUG */
				tmData->mter |= MTER_NEF;
				break;
			}

			if ((len = tm_ReadData(tmUnit, tmData->mtfc)) <= 0)
				break;

			tm_ClearStatus(sUnit, MTDS_EOT|MTDS_TM|MTDS_BOT);
			if ((rc = tape_Write(sUnit, tm_cBuffer, len)) <= 0) {
				switch (rc) {
					case MT_EOT:
						tmData->Position[slave] = TM_EOT; // End of Tape
						tm_SetStatus(sUnit, MTDS_EOT);
#ifdef DEBUG
						if (dbg_Check(DBG_IODATA))
							dbg_Printf("%s: ** End of Tape **\n", tapeName);
#endif /* DEBUG */

						break;

					case MT_ERROR:
#ifdef DEBUG
						dbg_Printf("%s: ** Error Code: %d (%s)\n",
							tapeName, mtape->error, strerror(mtape->error));
#endif /* DEBUG */
						break;
				}
			} else {
				tmData->Position[slave] = TM_MOT; // Middle of Tape

#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: %d of %d bytes had been written\n",
						tapeName, rc, -tmData->mtfc);
#endif /* DEBUG */
				count = rc;
			}
			break;

		case FNC_CHK_FWD: // Verify Forward
		case FNC_RD_FWD:  // Read Forward
			tm_ClearStatus(sUnit, MTDS_EOT|MTDS_TM|MTDS_BOT);
			if ((rc = tape_Read(sUnit, tm_cBuffer, -tmData->mtfc)) <= 0) {
				switch (rc) {
					case MT_TMARK:
						tmData->Position[slave] = TM_MARK; // Tape Mark
						tm_SetStatus(sUnit, MTDS_TM);
#ifdef DEBUG
						if (dbg_Check(DBG_IODATA))
							dbg_Printf("%s: ** Tape Mark **\n", tapeName);
#endif /* DEBUG */
						break;

					case MT_EOT:
						tmData->Position[slave] = TM_EOT; // End of Tape
						tm_SetStatus(sUnit, MTDS_EOT);
#ifdef DEBUG
						if (dbg_Check(DBG_IODATA))
							dbg_Printf("%s: ** End of Tape **\n", tapeName);
#endif /* DEBUG */

						break;

					case MT_ERROR:
#ifdef DEBUG
						dbg_Printf("%s: ** Error Code: %d (%s)\n",
							tapeName, mtape->error, strerror(mtape->error));
#endif /* DEBUG */
						break;
				}
			} else {
				tmData->Position[slave] = TM_MOT; // Middle of Tape

#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: %d of %d bytes had been read\n",
						tapeName, rc, -tmData->mtfc);
#endif /* DEBUG */
				count = rc;

				if (function == FNC_RD_FWD)
					tm_WriteData(tmUnit, count);
			}
			break;
	}

	// Update drive registers
	if ((count > 0) && (function >= 024)) {
		tmData->mtfc += count;
		if (tmData->mtfc == 0)
			tmData->mttc &= ~MTTC_FCS;
		else
			tmData->mttc |= MTTC_FCS;
	}

	// Any errors? If so, set ATA bit.
	if (tmData->mter)
		tm_SetATA(sUnit);

	// Inform RH11 controller that tape formatter is ready now
	tmData->mtcs1 &= ~MTCS1_GO;
	tm_Ready(tmUnit);

	return EMU_OK;
}

int tm_Boot(UNIT *tmUnit, int argc, char **argv)
{
	pdp10_Boot(tmUnit, BOOT_MAGTAPE, argc, argv);
	return EMU_OK;
}
