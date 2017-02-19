// uba.c - KS10 Processor: Unibus emulation routines
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

#include "pdp10/defs.h"
#include "pdp10/uba.h"
#include "pdp10/proto.h"
#include "dev/rh.h"
#include "dev/proto.h"

// Look at uba_Devices in dev/uba.c file
// for a listing of Unibus/Q-Bus devices.
extern DEVICE *uba_Devices[];

DEVICE p10_ubaDevice =
{
	"UBA",              /* Device Name */
	"KS10 - Unibus System",
	"v0.8 (Alpha)",  /* Version */
	NULL,            /* Drive Type - Not Used */
	NULL,            /* Unit table - Not used */
	uba_Devices,     // Listing of devices
	NULL,            // Listing of Commands
	NULL,            // Listing of Set Commands
	NULL,            // Listing of Show Commands

	0,               /* Number of Devices */
	0,               /* Number of Units */

	p10_ubaInitialize, /* Initialize Routine */
	p10_ubaReset,      /* Reset Routine */
	p10_ubaCreate,     /* Create Routine */
	p10_ubaDelete,     /* Delete Routine */
	p10_ubaConfigure,  /* Configure Routine */
	NULL,               /* Enable Routine */
	NULL,               /* Disable Routine */
	NULL,               /* Attach/Mount Routine - Not Used */
	NULL,               /* Detach/Unmount Routine - Not Used */
	NULL,               /* Format Routine */
	NULL,               /* Read I/O Routine */
	NULL,               /* Write I/O Routine */
	NULL,               /* Process Routine */
	NULL,               /* Boot Routine */
	NULL,               /* Execute Routine */

	NULL                /* SetUnit Routine - Not Used */
};

// Unibus Adapters
UBADEVICE *p10_ioDevice = NULL;

void p10_ubaInitialize(UNIT *ubaUnits, int32 maxUnits)
{
	int idx;

	for (idx = 0; idx < maxUnits; idx++) {
		ubaUnits[idx].Device = &p10_ubaDevice;
		ubaUnits[idx].idUnit = idx;
	}
}

void p10_ubaReset(UNIT *ubaUnit)
{
	UBAUNIT *ubaData = (UBAUNIT *)ubaUnit->uData;
	UNIT    *sUnits  = ubaUnit->sUnits;
	int     idx;

	// Initialize all UBA registers
	for (idx = 0; idx < 077; idx++)
		ubaData->Map[idx] = 0;
	ubaData->sr = 0;
	ubaData->mr = 0;

	// Initialize all devices and controllers.
	for (idx = 0; idx < ubaUnit->nUnits; idx++) {
		if (sUnits[idx].Flags & UNIT_PRESENT) {
#ifdef DEBUG
			dbg_Printf("UBA %d: Initializing unit #%d\n", ubaUnit->idUnit, idx);
#endif DEBUG
			sUnits[idx].Device->Reset(&sUnits[idx]);
		}
	}
}

// Create UBA (Unibus Adapter) controller
int p10_ubaCreate(UNIT *pUnit, char *devName, int argc, char **argv)
{
	int  unit;
	int  len, idx1, idx2;
	int  nUnits;
	char *tptr;

	len = strlen(p10_ubaDevice.Name);

	if (devName[len] == ':') {
		if (pUnit->sUnits) {
			printf("Error: Device %s already created.\n",
				p10_ubaDevice.Name);
			return EMU_ARG;
		}

		if (argc != 1) {
			printf("Error: wrong number of arguments.\n");
			printf("Usage: create %s <units>\n", p10_ubaDevice.Name);
			return EMU_ARG;
		}

		nUnits = strtol(argv[0], &tptr, 10);
		if (argv[0] == tptr) {
			printf("Error: argument must be value.\n");
			printf("Usage: create %s <units>\n", p10_ubaDevice.Name);
			return EMU_ARG;
		}

		p10_ioDevice = (UBADEVICE *)calloc(1, sizeof(UBADEVICE));
		uba_Units    = (UNIT *)calloc(nUnits, sizeof(UNIT));
		if (p10_ioDevice && uba_Units) {
			p10_ubaInitialize(uba_Units, nUnits);
			if (pUnit) {
				pUnit->sUnits = uba_Units;
				pUnit->nUnits = nUnits;
			}
			p10_ioDevice->nUnits   = nUnits;
			p10_ioDevice->ubaUnits = uba_Units;
			printf("Unit %s had been created.\n", devName);
		} else {
			if (p10_ioDevice) {
				free(p10_ioDevice);
				p10_ioDevice = NULL;
			}
			if (uba_Units)
				free(uba_Units);
			return EMU_MEMERR;
		}

		return EMU_OK;
	}

	unit = toupper(devName[len]) - '0';

//	printf("Device: %s Unit: %d\n", devName, unit);

	// Configure and enable specific UBA unit.
	// UBA0:, UBA1:, UBA2:, etc.. 
	//
	// Usage: create cpu0>uba1: <units>

	if (uba_Units && (unit < pUnit->nUnits)) {
		UNIT *uptr = &uba_Units[unit];

		if (!(uptr->Flags & UNIT_PRESENT)) {
			UNIT    *ubaUnits;
			UBAUNIT *ubaData;

			if (argc != 1) {
				printf("Error: wrong number of arguments.\n");
				printf("Usage: create %s <units>\n", p10_ubaDevice.Name);
				return EMU_ARG;
			}

			nUnits = strtol(argv[0], &tptr, 10);
			if (argv[0] == tptr) {
				printf("Error: argument must be value.\n");
				printf("Usage: create %s <units>\n", p10_ubaDevice.Name);
				return EMU_ARG;
			}

			ubaUnits = (UNIT *)calloc(nUnits, sizeof(UNIT));
			ubaData  = (UBAUNIT *)calloc(1, sizeof(UBAUNIT));

			if (ubaUnits && ubaData) {
				int idx;

				// Initialize internal Unibus I/O page map
				for (idx = UBA_INT_SADDR; idx <= UBA_INT_EADDR; idx += 2) {
					int32 idx2 = (idx & 017777) >> 1;

					ubaData->IOPage[idx2].pUnit = uptr;
					switch(idx) {
						case UBA_SR_ADDR:
							// Status Register (763100)
							ubaData->IOPage[idx2].ReadIO  = p10_ubaReadSR;
							ubaData->IOPage[idx2].WriteIO = p10_ubaWriteSR;
							break;

						case UBA_MR_ADDR:
							// Maint Register (763101)
							ubaData->IOPage[idx2].ReadIO  = p10_ubaReadMR;
							ubaData->IOPage[idx2].WriteIO = p10_ubaWriteMR;
							break;

						default:
							// Map Registers (763000 to 763777)
							ubaData->IOPage[idx2].ReadIO  = p10_ubaReadMAP;
							ubaData->IOPage[idx2].WriteIO = p10_ubaWriteMAP;
					}
				}

				// This unit is a Unibus interface.
				uptr->tFlags = UT_INTERFACE;
				uptr->dType  = NULL;
				uptr->Device = &p10_ubaDevice;
				uptr->sUnits = ubaUnits;
				uptr->nUnits = nUnits;
				uptr->Flags  = UNIT_PRESENT;
				uptr->uData  = (void *)ubaData;
			} else
				return EMU_MEMERR;
		} else {
			DEVICE *dptr;
			char   *pdevName = StrChar(devName, ':')+1;

			if ((dptr = unit_FindDevice(uba_Devices, pdevName)) == NULL)
				return EMU_ARG;

			return dptr->Create(uptr, pdevName, argc, argv);
		}
	} else
		return EMU_ARG;
	return EMU_OK;
}

// Delete the desired UBA controller
int p10_ubaDelete(int32 Unit)
{
	if (Unit < UBA_MAX) {
		UNIT *uptr = &uba_Units[Unit];

		if (uptr->Flags & UNIT_DISABLED)
			return EMU_ARG;

		free(uptr->sUnits);
		free(uptr->uData);
		uptr->Flags |= UNIT_DISABLED;
	} else
		return EMU_ARG;
	return EMU_OK;
}

int p10_ubaConfigure(UNIT *ubaUnit, UNIT *Unit, int32 sAddress, int32 Mask)
{
	UNIT    *sUnits  = ubaUnit->sUnits;
	UBAUNIT *ubaData = (UBAUNIT *)ubaUnit->uData;
	UBAMAP  *ubaMap  = ubaData->IOPage;
	int32   eAddress = sAddress + (Mask + 1);
	int     idx;

	switch ((int)Unit) {
		case UBA_DELETE:
			for (idx = sAddress; idx < eAddress; idx += 2) {
				int32 idx2 = (idx & 017777) >> 1;

				ubaMap[idx2].pUnit   = NULL;
				ubaMap[idx2].ReadIO  = NULL;
				ubaMap[idx2].WriteIO = NULL;
			}
			break;

		case UBA_CHECK:
			for (idx = sAddress; idx < eAddress; idx += 2) {
				int32 idx2 = (idx & 017777) >> 1;

				if (ubaMap[idx2].pUnit)
					return EMU_UBA_BADADDR;
			}
			break;

		default:
			for (idx = sAddress; idx < eAddress; idx += 2) {
				int32 idx2 = (idx & 017777) >> 1;

				ubaMap[idx2].pUnit   = Unit;
				ubaMap[idx2].ReadIO  = Unit->Device->ReadIO;
				ubaMap[idx2].WriteIO = Unit->Device->WriteIO;
			}
			break;
	}

	return EMU_OK;
}

int18 ks10uba_ReadData18(UNIT *ubaUnit, int18 vaddr)
{
	UBAUNIT *ubaData = (UBAUNIT *)ubaUnit->uData;
	int36 paddr = (ubaData->Map[(vaddr >> 11) & 077] & UBA_MAP_PAGE) |
		((vaddr >> 2) & 0777);
	int36 data = p10_pRead(paddr, 0);

	return (vaddr & 2) ? RH(data) : LHSR(data);
}

int36 ks10uba_ReadData36(UNIT *ubaUnit, int18 vaddr)
{
	UBAUNIT *ubaData = (UBAUNIT *)ubaUnit->uData;
	int36 paddr = (ubaData->Map[(vaddr >> 11) & 077] & UBA_MAP_PAGE) |
		((vaddr >> 2) & 0777);
	int36 data;

	data = p10_pRead(paddr, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("UBA %d: %06o (%06llo) => %s\n",
			ubaUnit->idUnit, vaddr, paddr, pdp10_DisplayData(data));
#endif DEBUG

	return data;
}

void ks10uba_WriteData18(UNIT *ubaUnit, int18 vaddr, int18 data)
{
	UBAUNIT *ubaData = (UBAUNIT *)ubaUnit->uData;
	int36 paddr = (ubaData->Map[(vaddr >> 11) & 077] & UBA_MAP_PAGE) |
		((vaddr >> 2) & 0777);
	int36 *pData = p10_pAccess(paddr);

	if (vaddr & 2)
		*pData = LH(*pData) | RH(data);
	else
		*pData = RHSL((int36)data) | RH(*pData);
	*pData = SXT36(*pData);
}

void ks10uba_WriteData36(UNIT *ubaUnit, int18 vaddr, int36 data)
{
	UBAUNIT *ubaData = (UBAUNIT *)ubaUnit->uData;
	int36 paddr = (ubaData->Map[(vaddr >> 11) & 077] & UBA_MAP_PAGE) |
		((vaddr >> 2) & 0777);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("UBA %d: %06o (%06llo) <= %s\n",
			ubaUnit->idUnit, vaddr, paddr, pdp10_DisplayData(SXT36(data)));
#endif DEBUG

	p10_pWrite(paddr, SXT36(data), 0);
}

void ks10uba_DoInterrupt(UNIT *ubaUnit, int32 IntBR, int32 IntVec)
{
	UBAUNIT *ubaData = (UBAUNIT *)ubaUnit->uData;

#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT))
		dbg_Printf("UBA: Interrupt - Unit %d, BR %d, Vector %04o\n",
			ubaUnit->idUnit, IntBR, IntVec);
#endif DEBUG

	if ((IntBR == 6) || (IntBR == 7)) {
		ubaData->sr |= UBA_SR_INTH;
		if (ubaData->sr & UBA_SR_PIH) {
#ifdef DEBUG
			if (dbg_Check(DBG_INTERRUPT))
				dbg_Printf("UBA: ** High Interrupt **\n");
#endif DEBUG
			ubaData->inth_Vector  = IntVec;
			ubaData->inth_Channel = (ubaData->sr & UBA_SR_PIH) >> 3;
			p10_piRequestIO(ubaData->inth_Channel);
		}
	}

	if ((IntBR == 4) || (IntBR == 5)) {
		ubaData->sr |= UBA_SR_INTL;
		if (ubaData->sr & UBA_SR_PIL) {
#ifdef DEBUG
			if (dbg_Check(DBG_INTERRUPT))
				dbg_Printf("UBA: ** Low Interrupt **\n");
#endif DEBUG
			ubaData->intl_Vector  = IntVec;
			ubaData->intl_Channel = ubaData->sr & UBA_SR_PIL;
			p10_piRequestIO(ubaData->intl_Channel);
		}
	}
}

void ks10uba_DisableInterrupt(UNIT *ubaUnit, int32 IntBR)
{
	UBAUNIT *ubaData = (UBAUNIT *)ubaUnit->uData;

	if ((IntBR == 6) || (IntBR == 7)) {
		ubaData->sr &= ~UBA_SR_INTH;
	}

	if ((IntBR == 4) || (IntBR == 5)) {
		ubaData->sr &= ~UBA_SR_INTL;
	}
}

int32 ks10uba_CheckInterrupt(int32 *Vector, int32 Channel)
{
	int Device;
	int Unit;

	for (Unit = 1; Unit < UBA_MAX; Unit += 2) {
		UBAUNIT *ubaData = (UBAUNIT *)uba_Units[Unit].uData;

		if (ubaData->sr & UBA_SR_INT) {
			if ((ubaData->sr & UBA_SR_INTH) &&
			    (ubaData->inth_Channel == Channel)) {
				Device = Unit;
				*Vector = ubaData->inth_Vector;
				ubaData->inth_Channel = 0;
				return Device;
			} else if ((ubaData->sr & UBA_SR_INTL) &&
			           (ubaData->intl_Channel == Channel)) {
				Device = Unit;
				*Vector = ubaData->intl_Vector;
				ubaData->intl_Channel = 0;
				return Device;
			}
		}
	}
	return 0;
}

void p10_ubaPageFailTrap(int30 ioAddr, int mode)
{

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UBA: (%c) Unknown I/O Address: %o,,%06o\n",
			((mode & PTF_WRITE) ? 'W' : 'R'), LHSR(ioAddr), RH(ioAddr));
#endif DEBUG

	// Create a page fail word for bad I/O address
	// and perform 'page fail trap' routine.
	PFW = ((FLAGS & PC_USER) ? PFW1_USER : 0) |
		((mode & PTF_IOBYTE) ? PFW1_BYTE : 0) |
		PFW1_HARD | PFW1_PAGED | PFW1_IO | ioAddr;
	PC = RH(PC - 1);
	emu_Abort(p10_SetJump, PAGE_FAIL);
}

// Read data from one of map (page) registers.
int32 p10_ubaReadMAP(UNIT *uptr, int32 ioAddr)
{
	UBAUNIT *ubaData = (UBAUNIT *)uptr->uData;
	int32   data     = ubaData->Map[ioAddr & UBA_MAP_MASK];

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UBA %d: MAP %02o => %012o\n",
			uptr->idUnit, ioAddr & UBA_MAP_MASK, data);
#endif DEBUG

	return data;
}

// Read data from status register.
int32 p10_ubaReadSR(UNIT *uptr, int32 ioAddr)
{
	UBAUNIT *ubaData = (UBAUNIT *)uptr->uData;
	int32   data     = (ubaData->sr & UBA_SR_RDMASK);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UBA %d: SR => %06o\n", uptr->idUnit, data);
#endif DEBUG

	return data;
}

// Read data from maintenance register.
int32 p10_ubaReadMR(UNIT *uptr, int32 ioAddr)
{
	UBAUNIT *ubaData = (UBAUNIT *)uptr->uData;
	int32   data     = (ubaData->mr & UBA_MR_RDMASK);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UBA %d: MR => %06o\n", uptr->idUnit, data);
#endif DEBUG

	return data;
}

// Write data into one of map (page) registers.
void p10_ubaWriteMAP(UNIT *uptr, int32 ioAddr, int18 data)
{
	UBAUNIT *ubaData = (UBAUNIT *)uptr->uData;
	int     reg      = ioAddr & UBA_MAP_MASK;

	ubaData->Map[reg] = (data & UBAW_MAP_PAGE) << 9;
	ubaData->Map[reg] |= (data & UBAW_MAP_FLAGS) << 13;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UBA %d: MAP %02o <= %06o (now: %012o)\n",
			uptr->idUnit, reg, data, ubaData->Map[reg]);
#endif DEBUG
}

// Read data from status register.
void p10_ubaWriteSR(UNIT *uptr, int32 ioAddr, int18 data)
{
	UBAUNIT *ubaData = (UBAUNIT *)uptr->uData;

	if (data & UBA_SR_UINIT) {
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("UBA %d: Unibus Initialization\n", uptr->idUnit);
#endif DEBUG
		p10_ubaReset(&uba_Units[uptr->idUnit]);
	}
	ubaData->sr = (data & UBA_SR_WRMASK) | (ubaData->sr & ~UBA_SR_WRMASK);
	ubaData->sr &= ~(data & UBA_SR_CLMASK);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UBA %d: SR <= %06o (now: %06o)\n",
			uptr->idUnit, data, ubaData->sr);
#endif DEBUG
}

// Read data from maintenance register.
void p10_ubaWriteMR(UNIT *uptr, int32 ioAddr, int18 data)
{
	UBAUNIT *ubaData = (UBAUNIT *)uptr->uData;

	ubaData->mr = data;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UBA %d: MR <= %06o\n", uptr->idUnit, data);
#endif DEBUG
}

// **************************************************************

int36 p10_ubaReadIO(UNIT *uptr, int30 ioAddr, int mode)
{
	int32 Unit = IO_CONTROLLER(ioAddr);
	int32 Addr = IO_REG_ADDR(ioAddr);
	int32 data32;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UBA: (R) Controller: %o  Register Address: %06o\n",
			Unit, Addr);
#endif DEBUG

	if ((Unit < nUnits) && uptr[Unit]->uData) {
		UBAUNIT *ubaData = (UBAUNIT *)uptr[Unit]->uData;
		UBAMAP  *ubaMap  = &ubaData->IOPage[(Addr & 017777) >> 1];

		if (ubaMap->pUnit && ubaMap->ReadIO)
			return ubaMap->ReadIO(ubaMap->pUnit, Addr);
		ubaData->sr |= (UBA_SR_TIM|UBA_SR_NED);
	}
 
	if ((Unit == 0) && (Addr == 0100000))
		return 0;

	p10_ubaPageFailTrap(ioAddr, mode | PTF_READ);

	return 0;
}

void p10_ubaWriteIO(int30 ioAddr, int36 Data, int mode)
{
	int32 Unit = IO_CONTROLLER(ioAddr);
	int32 Addr = IO_REG_ADDR(ioAddr);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UBA: (W) Controller: %o  Register Address: %06o\n",
			Unit, Addr);
#endif DEBUG

	if ((Unit < UBA_MAX) && uba_Units[Unit].uData) {
		UBAUNIT *ubaData = (UBAUNIT *)uba_Units[Unit].uData;
		UBAMAP  *ubaMap  = &ubaData->IOPage[(Addr & 017777) >> 1];

		if (ubaMap->pUnit && ubaMap->WriteIO) {
			ubaMap->WriteIO(ubaMap->pUnit, Addr, Data);
			return;
		}
		ubaData->sr |= (UBA_SR_TIM|UBA_SR_NED);
	} 

	if ((Unit == 0) && (Addr == 0100000))
		return;

	p10_ubaPageFailTrap(ioAddr, mode | PTF_WRITE);
}
