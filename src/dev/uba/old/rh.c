// rh.c - RH11 Controller routines
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
#include "dev/rh.h"
#include "pdp10/proto.h"
#include "dev/proto.h"

extern DEVICE rm_Device;
extern DEVICE rp_Device;
extern DEVICE tm_Device;

#ifdef DEBUG
// Listing of name of the registers for debug facility
// Note: ^ = Mixed, * = Drive Register, space = Controller Register

char *rh_RegNames[] =
{
	"RHCS1", // (R/W) ^Control and Status Register #1
	"RHWC",  // (R/W)  Word Count Register
	"RHBA",  // (R/W)  Bus Address Register
	"RHFC",  // (R/W) *Frame Count Register
	"RHCS2", // (R/W)  Control and Status Register #2
	"RHDS",  // (R)   *Drive Status Register
	"RHER",  // (R)   *Error Register
	"RHAS",  // (R/W) *Attention Summary Register
	"RHCC",  // (R/W)  Character Check Register
	"RHDB",  // (R/W)  Data Buffer Register
	"RHMR",  // (R/W)  Maintenance Register
	"RHDT",  // (R)   *Drive Type Register
	"RHSN",  // (R)   *Serial Number Register
	"RH??",  // (R/W)
	"RHBAE", // (R/W)  Bus Address Extension Register  (RH70 Only)
	"RHCS3"  // (R/W)  Control and Status Register #3  (RH70 Only)
};
#endif /* DEBUG */

DTYPE rh_dTypes[] =
{
	{
		"RH11",        // Controller Name
		"RH11 - MASSBUS Controller",
		UNIT_DISABLE,  // Controller is disable
		0, 0, 0, 0,    // Not used
		000000,        // Drive Type ID - Not Used
		NULL,          // Slave Drive Types - Not Used

		RH11_CTLR,     // RH11 is a controller
		NULL           // Local RH functions
	},

	{
		"RH70",        // Controller Name
		"RH70 - MASSBUS Controller",
		UNIT_DISABLE,  // Controller is disable
		0, 0, 0, 0,    // Not used
		000000,        // Drive Type ID - Not Used
		NULL,          // Slave Drive Types - Not Used

		RH11_CTLR,     // RH11 is a controller
		NULL           // Not Used
	},

	{ NULL }
};

DEVICE *rh_Devices[] =
{
	&rm_Device, // RM - Disk Pack Drives
	&rp_Device, // RP - Disk Pack Drives
	&tm_Device  // TM - Tape Controllers
};

DEVICE rh_Device =
{
	"RH",            // Device Name
	"MASSBUS Controller Interface",
	"v0.9 (Beta)",   // Version
	rh_dTypes,       // Drive Type
	NULL,            // RH11 Units - up to 8 RH controllers
	rh_Devices,      // Listing of devices
	NULL,            // List of Commands
	NULL,            // List of Set Commands
	NULL,            // List of Show Commands

	0,               // Number of Devices
	RH_MAXUNITS,     // Number of Units

	rh_Initialize,   // Initialize Routine
	rh_Reset,        // Reset Routine
	rh_Create,       // Create Routine
	rh_Delete,       // Delete Routine
	NULL,            // Configure Routine
	NULL,            // Enable Routine
	NULL,            // Disable Routine
	NULL,            // Attach/Mount Routine - Not Used
	NULL,            // Detach/Unmount Routine - Not Used
	NULL,            // Format Routine - Not Used
	rh_ReadIO,       // Read I/O Routine
	rh_WriteIO,      // Write I/O Routine
	rh_Process,      // Process Routine
	NULL,            // Boot Routine - Not Used
	NULL,            // Execute Routine - Not Used

	NULL,             // SetUnit Routine - Not Used
	rh_SetATA,        // SetATA Routine
	rh_ClearATA,      // ClearATA Routine
	rh_ReadData18,    // ReadData (18-bit) Routine
	rh_ReadData36,    // ReadData (36-bit) Routine
	rh_WriteData18,   // WriteData (18-bit) Routine
	rh_WriteData36,   // WriteData (36-bit) Routine
	rh_Ready,         // Ready Rountine
	rh_CheckWordCount // CheckWordCount Routine
};

int32 rh_ata[] = {
	RHAS_ATA0, // Attention for unit #0
	RHAS_ATA1, // Attention for unit #1
	RHAS_ATA2, // Attention for unit #2
	RHAS_ATA3, // Attention for unit #3
	RHAS_ATA4, // Attention for unit #4
	RHAS_ATA5, // Attention for unit #5
	RHAS_ATA6, // Attention for unit #6
	RHAS_ATA7  // Attention for unit #7
};

// Create RH11 controller
int rh_Create(UNIT *pUnit, char *devName, int argc, char **argv)
{
	int unit;
	int len, st;
	int idx1, idx2;

	len = strlen(rh_Device.Name);
	unit = toupper(devName[len++]) - 'A';

	if (unit < RH_MAXUNITS) {
		UNIT *uptr;

		uptr = &pUnit->sUnits[unit];

		if (devName[len] != ':') {
			DEVICE *dptr;
			char   *pdevName = (char *)strchr(devName, ':')+1;
			int    unit;
			int    st;

//			if ((dptr = unit_FindDevice(rh_Devices, pdevName)) == NULL)
//				return EMU_ARG;

			unit = devName[len] - '0';
			if (st = dptr->Create(&uptr->sUnits[unit], pdevName, argc, argv))
				return;

			return EMU_OK;
		}

		if (!(uptr->Flags & UNIT_PRESENT)) {
			UNIT     *rhUnits = (UNIT *)calloc(sizeof(UNIT), RH_MAXUNITS);
			RH11UNIT *rhData  = (RH11UNIT *)calloc(sizeof(RH11UNIT), 1);

			if (rhUnits && rhData) {
				char  *pAddress, *epAddress;
				char  *pMask,    *epMask;
				char  *pIntIPL,  *epIntIPL;
				char  *pIntVec,  *epIntVec;
				int32 nAddress, nMask;
				int32 nIntIPL, nIntVec;
				int idx;

				for (idx1 = 0; rh_dTypes[idx1].Name; idx1++) {
					if (!strcasecmp(argv[0], rh_dTypes[idx1].Name))
						break;
				}

				if (!rh_dTypes[idx1].Name) {
					free(rhUnits);
					free(rhData);
					return EMU_ARG;
				}

				pAddress = argv[1];
				pMask    = argv[2];
				pIntIPL  = argv[3];
				pIntVec  = argv[4];

				nAddress = ToInteger(pAddress, &epAddress, 8);
				nMask    = ToInteger(pMask, &epMask, 8);
				nIntIPL  = ToInteger(pIntIPL, &epIntIPL, 10);
				nIntVec  = ToInteger(pIntVec, &epIntVec, 8);

				if ((pAddress == epAddress) || (pMask == epMask) ||
				    (pIntIPL == epIntIPL) || (pIntVec == epIntVec)) {
					free(rhUnits);
					free(rhData);
					return EMU_ARG;
				}
				
				if (pUnit) {
					int st;

					if (st = pUnit->Device->Configure(pUnit, NULL, nAddress, nMask, nIntIPL, nIntVec)) {
						free(rhUnits);
						free(rhData);
						return st;
					}
				}

				*strchr(argv[5], ':') = '\0';
//				if (st = unit_mapCreateDevice(argv[5], uptr)) {
//					free(rhUnits);
//					free(rhData);
//					return st;
//				}

				printf("Addr=%06o Mask=%06o IPL=%d Vector=%03o\n",
					nAddress, nMask, nIntIPL, nIntVec);

				rhData->Address    = nAddress;
				rhData->Mask       = nMask;
				rhData->IntIPL     = nIntIPL;
				rhData->IntVec     = nIntVec;

				for (idx2 = 0; idx2 < RH_MAXUNITS; idx2++) {
					rhUnits[idx2].idUnit = idx2;
					rhUnits[idx2].pUnit  = uptr;
				}

				// This unit is a RH11 controller
				uptr->tFlags = UT_CONTROLLER;
				uptr->dType  = &rh_dTypes[idx1];
				uptr->Device = &rh_Device;
				uptr->sUnits = rhUnits;
				uptr->nUnits = RH_MAXUNITS;
				uptr->Flags  = rh_dTypes[idx1].Flags | UNIT_PRESENT;
				uptr->pUnit  = pUnit;
				uptr->uData  = (void *)rhData;

				if (pUnit)
					pUnit->Device->Configure(pUnit, uptr, nAddress, nMask, nIntIPL, nIntVec);
			} else
				return EMU_MEMERR;
		} else
			return EMU_ARG;
	} else
		return EMU_ARG;
	return EMU_OK;
}

// Disable the desired RH11 controller
int rh_Delete(UNIT *rhUnit)
{
	if (rhUnit->Flags & UNIT_DISABLED)
		return EMU_ARG;

	free(rhUnit->sUnits);
	free(rhUnit->uData);
	rhUnit->Flags |= UNIT_DISABLED;

	return EMU_OK;
}

void rh_Initialize(UNIT *rhUnit)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;
	int      idx;

	rhData->rhcs1 = RHCS1_RDY;
	rhData->rhba  = 0;
	rhData->rhcs2 = 0;

	for (idx = 0; idx < RH_MAXUNITS; idx++) {
		UNIT   *sUnit   = &rhUnit->sUnits[idx];
		DEVICE *sDevice = sUnit->Device;
			
		if (sDevice)
			sDevice->Initialize(sUnit);
	}
}

void rh_Reset(UNIT *rhUnit)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;
	int      idx;

	rhData->rhcs1 = RHCS1_RDY;
	rhData->rhba  = 0;
	rhData->rhcs2 = 0;

	for (idx = 0; idx < RH_MAXUNITS; idx++) {
		UNIT   *sUnit   = &rhUnit->sUnits[idx];
		DEVICE *sDevice = sUnit->Device;
			
		if (sDevice)
			sDevice->Reset(sUnit);
	}
}

void rh_DoInterrupt(UNIT *rhUnit)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;

	ks10uba_DoInterrupt(rhUnit->pUnit, rhData->IntIPL, rhData->IntVec);
}

void rh_SetATA(UNIT *rhUnit, int32 unit)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;

	rhData->rhas  |= rh_ata[unit];
	rhData->rhcs1 |= (RHCS1_SC|RHCS1_RDY);
	if (rhData->rhcs1 & RHCS1_IE)
		rh_DoInterrupt(rhUnit);
}

void rh_ClearATA(UNIT *rhUnit, int32 unit)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;

	rhData->rhas  &= ~rh_ata[unit];
	rhData->rhcs1 &= ~RHCS1_SC;
}

void rh_UpdateCS1(UNIT *rhUnit, int32 data)
{
	UNIT     *sUnits = rhUnit->sUnits;
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;

	if (data & RHCS1_TRE)
		rhData->rhcs1 &= ~RHCS1_TRE;

	if (rhData->rhcs2 & RHCS2_ERR)
		rhData->rhcs1 |= RHCS1_TRE;

	if ((rhData->rhcs1 & (RHCS1_TRE | RHCS1_MCPE)) || rhData->rhas)
		rhData->rhcs1 |= RHCS1_SC;

	if ((data & (RHCS1_RDY|RHCS1_IE)) == (RHCS1_RDY|RHCS1_IE))
		rh_DoInterrupt(rhUnit);
}

void rh_Ready(UNIT *rhUnit)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;

	rhData->rhcs1 |= RHCS1_RDY;
	if (rhData->rhcs1 & RHCS1_IE)
		rh_DoInterrupt(rhUnit);
}

int rh_CheckWordCount(UNIT *rhUnit)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;

	if (rhData->rhwc == 0)
		return EMU_ZWC;

	return EMU_OK;
}

void rh_WriteIO(UNIT *rhUnit, int32 addr, int32 data)
{
	RH11UNIT *rhData   = (RH11UNIT *)rhUnit->uData;
	UNIT     *sUnit    = &rhUnit->sUnits[rhUnit->cUnit];
	DEVICE   *sDevice  = sUnit->Device;
	int32    reg       = (addr & rhData->Mask);
	int      newUnit;
	int      idx;

	data &= 0xFFFF;
	switch (reg >> 1) {
		case RHCS1:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHCS1 (00) <= %06o\n", data);
#endif /* DEBUG */

			// Have to set BAE and IE before process data on drive's side to
			// avoid major system crash.
			rhData->rhcs1 = (rhData->rhcs1 & ~(RHCS1_BAE|RHCS1_IE)) |
				(data & (RHCS1_BAE|RHCS1_IE));

			if ((data & RHCS1_DRV) && sDevice) {
				int function = data & (RHCS1_FUNC|RHCS1_GO);

				if ((function >= 050) && (function & RHCS1_GO)) {
					if (rhData->rhcs1 & RHCS1_RDY) {
						rhData->rhcs1 &= ~(RHCS1_RDY|RHCS1_TRE);
						rhData->rhcs2 &= ~RHCS2_ERR;
					} else {
						rhData->rhcs2 |= RHCS2_PGE;
						data &= ~(RHCS1_FUNC|RHCS1_GO);
					}
				} else
					// Requires action when non-transfer functions completed
					rhData->rhcs1 |= RHCS1_SC;

				if (data & RHCS1_DRV)
					sDevice->WriteIO(sUnit, reg, data & RHCS1_DRV);
			}

			// Now update rest of CS1 register.
			rh_UpdateCS1(rhUnit, data);
			break;

		case RHWC:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHWC (02) <= %06o (%d 16/18-Bit Words)\n",
					data, -((data & 0x8000) ? data | 0xFFFF0000 : data));
#endif /* DEBUG */
			rhData->rhwc = data;
			break;

		case RHBA:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHBA (04) <= %06o\n", data);
#endif /* DEBUG */
			rhData->rhba = data;
			break;

		case RHCS2:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHCS2 (10) <= %06o\n", data);
#endif /* DEBUG */
			if (data & RHCS2_CLR)
				rh_Reset(rhUnit);

			newUnit = data & RHCS2_UNIT;
			rhUnit->cUnit = newUnit;
			sUnit   = &rhUnit->sUnits[newUnit];
			sDevice = sUnit->Device;

			if ((sUnit->Flags & UNIT_PRESENT) &&
			    !(sUnit->Flags & UNIT_DISABLED)) {
				rhUnit->pcUnit = sUnit;
			} else {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS)) {
					if (sUnit->Flags & UNIT_DISABLED)
						dbg_Printf("RH11: Unit %d is disabled.\n", newUnit);
					else
						dbg_Printf("RH11: Unit %d is not present.\n", newUnit);
				}
#endif /* DEBUG */
				data |= RHCS2_NED;
				rhData->rhcs1 |= (RHCS1_SC | RHCS1_TRE);
				rhUnit->pcUnit = NULL;
			}

			rhData->rhcs2 = data;
			break;

		case RHAS:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHAS (16) <= %06o\n", data);
#endif /* DEBUG */
			for (idx = 0; idx < RH_MAXUNITS; idx++) {
				UNIT   *sUnit   = &rhUnit->sUnits[idx];
				DEVICE *sDevice = sUnit->Device;

				if ((data & rh_ata[idx]) && sDevice)
					sDevice->ClearATA(sUnit, idx);
			}
			break;

		case RHDB:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHDB (22) <= %06o\n", data);
#endif /* DEBUG */
			rhData->rhdb = data;
			break;

		default: /* Drive register or Illegal Register */
			if (sDevice) {
				sDevice->WriteIO(sUnit, reg, data);
				break;
			}
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: Illegal Register: %02o (%06o)\n", reg, addr);
#endif /* DEBUG */
			rhData->rhcs2 |= RHCS2_NED;
			rhData->rhcs1 |= (RHCS1_SC | RHCS1_TRE);
			break;
	}
}

int32 rh_ReadIO(UNIT *rhUnit, int32 addr)
{
	RH11UNIT *rhData   = (RH11UNIT *)rhUnit->uData;
	UNIT     *sUnit    = &rhUnit->sUnits[rhUnit->cUnit];
	DEVICE   *sDevice  = sUnit->Device;
	int32    reg       = (addr & rhData->Mask);
	int32    data;

	switch (reg >> 1) {
		case RHCS1:
			if (sDevice)
				data = sDevice->ReadIO(sUnit, reg) | rhData->rhcs1;
			else
				data = rhData->rhcs1;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHCS1 (00) => %06o\n", data);
#endif /* DEBUG */
			break;

		case RHWC:
			data = rhData->rhwc;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHWC (02) => %06o (%d 16/18-Bit Words)\n",
					data, -((data & 0x8000) ? data | 0xFFFF0000 : data));
#endif /* DEBUG */
			break;

		case RHBA:
			data = rhData->rhba;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHBA (04) => %06o\n", data);
#endif /* DEBUG */
			break;

		case RHCS2:
			data = rhData->rhcs2;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHCS2 (10) => %06o\n", data);
#endif /* DEBUG */
			break;

		case RHAS:
			data = rhData->rhas;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHAS (16) => %06o\n", data);
#endif /* DEBUG */
			break;

		case RHDB:
			data = rhData->rhdb;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: RHDB (22) => %06o\n", data);
#endif /* DEBUG */
			break;

		default: // Drive register or Illegal register
			if (sDevice) {
				data = sDevice->ReadIO(sUnit, reg);
				break;
			}
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("RH11: Illegal Register: %02o (%06o)\n", reg, addr);
#endif /* DEBUG */
			data = 0;
			rhData->rhcs2 |= RHCS2_NED;
			rhData->rhcs1 |= (RHCS1_SC | RHCS1_TRE);
			break;
	}

	return data;
}

int rh_WriteData18(UNIT *rhUnit, int18 data, boolean force)
{
	RH11UNIT *rhData  = (RH11UNIT *)rhUnit->uData;
	UNIT     *ubaUnit = rhUnit->pUnit;

	if (rhData->rhwc) {
		int32 addr = rhData->rhba | ((rhData->rhcs1 & RHCS1_BAE) << 8);

		ks10uba_WriteData18(ubaUnit, addr, data);
		rhData->rhwc++;
		addr += 2;
		if (force || (rhData->rhcs2 & RHCS2_BAI) == 0) {
			rhData->rhba  = addr;
			rhData->rhcs1 = (rhData->rhcs1 & ~RHCS1_BAE) |
				((addr >> 8) & RHCS1_BAE);
		}
		return EMU_OK;
	}
	return EMU_ZWC;
}

int rh_WriteData36(UNIT *rhUnit, int36 data, boolean force)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;
	UNIT     *ubaUnit = rhUnit->pUnit;

	if (rhData->rhwc) {
		int32 addr = rhData->rhba | ((rhData->rhcs1 & RHCS1_BAE) << 8);

		ks10uba_WriteData36(ubaUnit, addr, data);
		rhData->rhwc += 2;
		addr += 4;
		if (force || (rhData->rhcs2 & RHCS2_BAI) == 0) {
			rhData->rhba  = addr;
			rhData->rhcs1 = (rhData->rhcs1 & ~RHCS1_BAE) |
				((addr >> 8) & RHCS1_BAE);
		}
		return EMU_OK;
	}
	return EMU_ZWC;
}

int rh_ReadData18(UNIT *rhUnit, int18 *data, boolean force)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;
	UNIT     *ubaUnit = rhUnit->pUnit;

	if (rhData->rhwc) {
		int32 addr = rhData->rhba | ((rhData->rhcs1 & RHCS1_BAE) << 8);

		*data = ks10uba_ReadData18(ubaUnit, addr);
		rhData->rhwc++;
		addr += 2;
		if (force || (rhData->rhcs2 & RHCS2_BAI) == 0) {
			rhData->rhba  = addr;
			rhData->rhcs1 = (rhData->rhcs1 & ~RHCS1_BAE) |
				((addr >> 8) & RHCS1_BAE);
		}
		return EMU_OK;
	}
	return EMU_ZWC;
}

int rh_ReadData36(UNIT *rhUnit, int36 *data, boolean force)
{
	RH11UNIT *rhData = (RH11UNIT *)rhUnit->uData;
	UNIT     *ubaUnit = rhUnit->pUnit;

	if (rhData->rhwc) {
		int32 addr = rhData->rhba | ((rhData->rhcs1 & RHCS1_BAE) << 8);

		*data = ks10uba_ReadData36(ubaUnit, addr);
		rhData->rhwc += 2;
		addr += 4;
		if (force || (rhData->rhcs2 & RHCS2_BAI) == 0) {
			rhData->rhba  = addr;
			rhData->rhcs1 = (rhData->rhcs1 & ~RHCS1_BAE) |
				((addr >> 8) & RHCS1_BAE);
		}
		return EMU_OK;
	}
	return EMU_ZWC;
}

int rh_Process(UNIT *rhUnit)
{
	DEVICE   *rhDevice = rhUnit->Device;
	RH11UNIT *rhData   = (RH11UNIT *)rhUnit->uData;

	return EMU_OK;
}
