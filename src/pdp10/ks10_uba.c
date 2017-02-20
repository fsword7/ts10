// ks10_uba.c - KS10 Processor: UBA Interface (Unibus)
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

#include "pdp10/defs.h"
#include "pdp10/ks10.h"

// Unibus Initialization Routine
void ks10uba_ResetAll(KS10UBA_IF *uba)
{
	MAP_IO *io;

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: Unibus Initialization Begins:\n", uba->Unit.devName);
#endif /* DEBUG */

	for (io = uba->ioList; io; io = io->Next) {
#ifdef DEBUG
//		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s:   Reseting %s: (%s) ... ",
				uba->Unit.devName, io->devName, io->keyName);
		if (io->ResetIO) {
			io->ResetIO(io->Device);
//			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("Done.\n");
		} else // if (dbg_Check(DBG_IOREGS))
			dbg_Printf("Skipped.\n");
#else /* DEBUG */
		if (io->ResetIO)
			io->ResetIO(io->Device);
#endif /* DEBUG */
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: End of Unibus Initalization.\n", uba->Unit.devName);
#endif /* DEBUG */
}


int32 uba_GetVector(KS10UBA_DEVICE *uba, int32 chan, int32 *unit)
{
	KS10UBA_IF *uif;
	int32      dev, ipl, irq = 0;

	for (dev = 1; dev < UBA_MAX; dev += 2) {
		uif = &uba->Slots[dev];
		if (chan == ((uif->sr & SR_PIH) >> 3)) {
			irq = uif->intRequests;
			break;
		}
		if (chan == (uif->sr & SR_PIL)) {
			irq = uif->intRequests;
			break;
		}
	}

	if (irq) {
		// Find highest priority to execute interrupt vector.
		for (ipl = 0; ipl < 32; ipl++) {
			if ((irq >> ipl) & 1) {
				// Acknowledge IPL mask and return
				// UBA number and vector to PI routine.
				uif->intRequests &= ~(1u << ipl);
				*unit = dev; // Device (UBA) Number
#ifdef DEBUG
				if (dbg_Check(DBG_INTERRUPT))
					dbg_Printf("UBA(%d): Channel %d  IPL %d  Vector %03o\n",
						dev, chan, ipl, uif->intVector[ipl]);
#endif /* DEBUG */
				return uif->intVector[ipl];
			}
		}
	}
	
	// Regular interrupt as default;
	return 0;
}

void ks10uba_PageFailTrap(KS10UBA_DEVICE *uba, int30 ioAddr, int mode)
{
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (%c) Unknown I/O Address: %o,,%06o\n",
			uba->devName, ((mode & PTF_WRITE) ? 'W' : 'R'),
			LH18(ioAddr), RH18(ioAddr));
#endif /* DEBUG */

	// Create a page fail word for bad I/O address
	// and perform 'page fail trap' routine.
	PFW = ((FLAGS & FLG_USER) ? PFW1_USER : 0) |
		((mode & PTF_IOBYTE) ? PFW1_BYTE : 0) |
		PFW1_HARD | PFW1_PAGED | PFW1_IO | ioAddr;
	PC = RH(PC - 1);
	emu_Abort(p10_SetJump, PAGE_FAIL);
}

// ****************************************************************

int ks10uba_ReadMAP(void *dptr, uint32 ioAddr, uint32 *data, uint32 acc)
{
	register KS10UBA_IF *uba = (KS10UBA_IF *)dptr;
	register uint32     reg  = ioAddr & MAP_MASK;

	*data = uba->map[reg];

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) MAP %02o => %06o,,%06o\n",
			uba->Unit.devName, reg, LH18(*data), RH18(*data));
#endif /* DEBUG */

	return UQ_OK;
}

int ks10uba_WriteMAP(void *dptr, uint32 ioAddr, uint32 data, uint32 acc)
{
	register KS10UBA_IF *uba = (KS10UBA_IF *)dptr;
	register uint32     reg  = ioAddr & MAP_MASK;

#ifdef DEBUG
	register uint32 data36 =
#endif /* DEBUG */
	uba->map[reg] =
		((data & MAPW_FLAGS) << 13) | ((data & MAPW_PAGE) << 9);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) MAP %02o <= %06o,,%06o (%06o)\n",
			uba->Unit.devName, reg, LH18(data36), RH18(data36), data);
#endif /* DEBUG */

	return UQ_OK;
}

// *****************************************************************

int ks10uba_ReadSR(void *dptr, uint32 ioAddr, uint32 *data, uint32 acc)
{
	register KS10UBA_IF *uba = (KS10UBA_IF *)dptr;

	*data = uba->sr;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) SR => %06o\n", uba->Unit.devName, uba->sr);
#endif /* DEBUG */

	return UQ_OK;
}

int ks10uba_WriteSR(void *dptr, uint32 ioAddr, uint32 data, uint32 acc)
{
	register KS10UBA_IF *uba = (KS10UBA_IF *)dptr;

	// Update Control and Status Register
	if (data & SR_UINIT) {
		ks10uba_ResetAll(uba);
		uba->sr = data & SR_DXFR;
	} else {
		uba->sr = data & SR_RW;
//		uba->sr &= ~(data & SR_W1C);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) SR <= %06o (Now: %06o)\n",
			uba->Unit.devName, data, uba->sr);
#endif /* DEBUG */

	return UQ_OK;
}

// *****************************************************************

int ks10uba_ReadMR(void *dptr, uint32 ioAddr, uint32 *data, uint32 acc)
{
	register KS10UBA_IF *uba = (KS10UBA_IF *)dptr;

	*data = uba->mr;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) MR => %06o\n", uba->Unit.devName, *data);
#endif /* DEBUG */

	return UQ_OK;
}

int ks10uba_WriteMR(void *dptr, uint32 ioAddr, uint32 data, uint32 acc)
{
	register KS10UBA_IF *uba = (KS10UBA_IF *)dptr;

	uba->mr = data;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) MR <= %06o\n", uba->Unit.devName, data);
#endif /* DEBUG */

	return UQ_OK;
}

// *****************************************************************

uint32 ks10uba_ReadIO(KS10UBA_DEVICE *uba, int30 addr, int mode)
{
	uint32     ioUnit = IO_CONTROLLER(addr);
	uint32     ioAddr = IO_REG_ADDR(addr);
	uint32     data   = 0;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s%d: (R) UBA Address: %06o\n",
			uba->devName, ioUnit, ioAddr);
#endif /* DEBUG */

	if ((ioUnit < UBA_MAX) && (uba->Slots[ioUnit].Flags & UIF_EXIST)) {
		int        acc  = (mode & PTF_IOBYTE) ? 1 : 2;
		KS10UBA_IF *uif = &uba->Slots[ioUnit];
		MAP_IO     *io;

		if (io = uif->ioMap[(ioAddr & 017777) >> 1]) {
			if (io->ReadIO) {
				if (io->ReadIO(io->Device, ioAddr, (uint16 *)&data, 2) == UQ_OK)
					return data;
			} else if (io->ReadIO32) {
				if (io->ReadIO32(io->Device, ioAddr, &data, 2) == UQ_OK)
					return data;
			}
		}
	}

	if ((ioUnit == 0) && (ioAddr == 0100000))
		return 0;

	ks10uba_PageFailTrap(uba, ioAddr, mode | PTF_READ);
	return 0;
}

void ks10uba_WriteIO(KS10UBA_DEVICE *uba, int30 addr, uint32 data, int mode)
{
	uint32     ioUnit = IO_CONTROLLER(addr);
	uint32     ioAddr = IO_REG_ADDR(addr);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s%d: (W) UBA Address: %06o\n",
			uba->devName, ioUnit, ioAddr);
#endif /* DEBUG */

	if ((ioUnit < UBA_MAX) && (uba->Slots[ioUnit].Flags & UIF_EXIST)) {
		int        acc  = (mode & PTF_IOBYTE) ? 1 : 2;
		KS10UBA_IF *uif = &uba->Slots[ioUnit];
		MAP_IO     *io;

		if (io = uif->ioMap[(ioAddr & 017777) >> 1]) {
			if (io->WriteIO) {
				if (io->WriteIO(io->Device, ioAddr, data, 2) == UQ_OK)
					return;
			} else if (io->WriteIO32) {
				if (io->WriteIO32(io->Device, ioAddr, data, 2) == UQ_OK)
					return;
			}
		}
	}

	if ((ioUnit == 0) && (ioAddr == 0100000))
		return;

	ks10uba_PageFailTrap(uba, ioAddr, mode | PTF_WRITE);
}

// Set Software Interrupt Vector (New)
void ks10uba_SetVector(MAP_IO *io, int newVector, int idx)
{
	KS10UBA_IF *uba = (KS10UBA_IF *)io->uqDevice;

	if (idx < 2) {
		uba->intVector[io->intLevel[idx]] = newVector;
		io->intVector[idx]                = newVector;
	}
}

// Send Interrupt to Host (New)
void ks10uba_SendInterrupt(MAP_IO *io, int idx)
{
	KS10UBA_IF *uba = (KS10UBA_IF *)io->uqDevice;

#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT))
		dbg_Printf("%s: Interrupt - Vector %03o IPL %d\n",
			uba->Unit.devName, io->intVector[idx],  io->intLevel[idx]);
#endif /* DEBUG */

	uba->intRequests |= io->intMask[idx];

	if (io->intMask[idx] & INT_BR67) {
		if (uba->sr & SR_PIH) {
			KS10_piRequestIO((uba->sr & SR_PIH) >> 3);
#ifdef DEBUG
			if (dbg_Check(DBG_INTERRUPT))
				dbg_Printf("%s:   High Interrupt (PI %d)\n",
					uba->Unit.devName, (uba->sr & SR_PIH) >> 3);
#endif /* DEBUG */
		}
	}

	if (io->intMask[idx] & INT_BR45) {
		if (uba->sr & SR_PIL) {
			KS10_piRequestIO(uba->sr & SR_PIL);
#ifdef DEBUG
			if (dbg_Check(DBG_INTERRUPT))
				dbg_Printf("%s:   Low Interrupt (PI %d)\n",
					uba->Unit.devName, uba->sr & SR_PIL);
#endif /* DEBUG */
		}
	}
}

inline int ks10uba_MapAddr(KS10UBA_IF *uba, uint32 ioAddr, uint32 *mapAddr)
{
	register uint32 mapData = uba->map[(ioAddr >> 11) & 077];

	if (mapData & MAP_VALID) {
		*mapAddr = (mapData & MAP_PAGE) | ((ioAddr >> 2) & 0777);
		return 0;
	}
	return 1;
}

// Read block to an Unibus I/O device
uint32 ks10uba_ReadBlock(void *dptr, uint32 ioAddr,
	uint8 *data, uint32 szBytes, uint32 mode)
{
	KS10UBA_IF  *uba     = (KS10UBA_IF *)dptr;
	KS10_DEVICE *ks10    = uba->Processor;
	uint32      *blkData = (uint32 *)data;
	uint32      mapAddr, idxAddr = 0;
	uint32      cntBytes, wc36;
	int36       *pData;

	cntBytes = 01000 - (ioAddr & 0777);
	if (szBytes < cntBytes)
		cntBytes = szBytes;
	
	while (szBytes > 0) {
		if (ks10uba_MapAddr(uba, ioAddr, &mapAddr))
			return szBytes;
		pData = p10_pAccess(mapAddr);

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA)) {
			dbg_Printf("%s: (R) Address: %08o (%08o)  Size: %d words\n",
				uba->Unit.devName, ioAddr, mapAddr, cntBytes >> 2);
//			PrintDump18(idxAddr, &blkData[idxAddr], cntBytes);
		}
#endif /* DEBUG */

		for (wc36 = (cntBytes >> 2); wc36 > 0; wc36--) {
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s:   %06o (%06o) => %s\n", uba->Unit.devName,
					ioAddr, mapAddr++, pdp10_DisplayData(*pData));
#endif /* DEBUG */
			blkData[idxAddr++] = LH18(*pData);
			blkData[idxAddr++] = RH18(*pData++);
			ioAddr += 4;
		}
	
		szBytes  -= cntBytes;
		cntBytes  = (szBytes > 01000) ? 01000 : szBytes;
	}

	return szBytes;
}

// Write block from an Unibus I/O device
uint32 ks10uba_WriteBlock(void *dptr, uint32 ioAddr,
	uint8 *data, uint32 szBytes, uint32 mode)
{
	KS10UBA_IF  *uba     = (KS10UBA_IF *)dptr;
	KS10_DEVICE *ks10    = uba->Processor;
	uint32      *blkData = (uint32 *)data;
	uint32      mapAddr, idxAddr = 0;
	uint32      cntBytes, wc36;
	int36       *pData;

	cntBytes = 01000 - (ioAddr & 0777);
	if (szBytes < cntBytes)
		cntBytes = szBytes;

	while (szBytes > 0) {
		if (ks10uba_MapAddr(uba, ioAddr, &mapAddr))
			return szBytes;
		pData = p10_pAccess(mapAddr);

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA)) {
			dbg_Printf("%s: (W) Address: %08o (%08o)  Size: %d words\n",
				uba->Unit.devName, ioAddr, mapAddr, cntBytes >> 2);
//			PrintDump18(idxAddr, &data[idxAddr], cntBytes);
		}
#endif /* DEBUG */

		for (wc36 = (cntBytes >> 2); wc36 > 0; wc36--) {
			*pData = SL((int36)blkData[idxAddr]) | blkData[idxAddr+1];
			*pData = SXT36(*pData);
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s:   %06o (%06o) <= %s\n", uba->Unit.devName,
					ioAddr, mapAddr++, pdp10_DisplayData(*pData));
#endif /* DEBUG */
			pData++;
			ioAddr  += 4;
			idxAddr += 2;
		}

		szBytes  -= cntBytes;
		cntBytes  = (szBytes > 01000) ? 01000 : szBytes;
	}

	return szBytes;
}

// ***************************************************************

UQ_CALL ks10uba_Callback;

int ks10uba_SetMap(void *dptr, MAP_IO *io)
{
	KS10UBA_IF *uba = (KS10UBA_IF *)dptr;
	MAP_IO     *cptr, *pptr = NULL;
	int        csrAddr = (io->csrAddr & 017777) >> 1;
	int        idx;

	// Check free area first. If any reserved area,
	// Report that any or all area is already reserved.
	for (idx = 0; idx < io->nRegs; idx++)
		if (uba->ioMap[csrAddr + idx])
			return UQ_RESERVED;

	// Convert 16/18/22-bit address to 18-bit address.
	io->csrAddr = (io->csrAddr & 017777) | 0760000;
	io->idUnit  = uba->idSlot;

	// Set up callback functions for this Q22 interface.
	io->SetVector     = ks10uba_SetVector;
	io->SendInterrupt = ks10uba_SendInterrupt;

	// Now assign device registers to call.
	for (idx = 0; idx < io->nRegs; idx++)
		uba->ioMap[csrAddr + idx] = io;

	// Now assign vector address to host.
	for (idx = 0; idx < 2; idx++)
		if (io->intVector[idx] && io->intLevel[idx])
			uba->intVector[io->intLevel[idx]] = io->intVector[idx];

	// Insert a I/O node into a device list.
	if (uba->ioList == NULL) {
		io->Next    = NULL;
		uba->ioList = io;
	} else {
		for (cptr = uba->ioList; cptr; cptr = cptr->Next) {
			if (io->csrAddr < cptr->csrAddr)
				break;
			pptr = cptr;
		}
		if (cptr == uba->ioList) {
			io->Next = uba->ioList;
			uba->ioList = io;
		} else {
			io->Next = pptr->Next;
			pptr->Next = io;
		}
	}

	return UQ_OK;
}

void *ks10uba_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	KS10UBA_DEVICE *uba = NULL;
	KS10UBA_IF     *uif;
	KS10_DEVICE    *ks10;
	uint32         idx;

	if (uba = (KS10UBA_DEVICE *)calloc(1, sizeof(KS10UBA_DEVICE))) {
		// Set up its descriptions on new device.
		uba->devName     = newMap->devName;
		uba->keyName     = newMap->keyName;
		uba->emuName     = newMap->emuName;
		uba->emuVersion  = newMap->emuVersion;

		// Set up UBA slots
		uba->nSlots = UBA_MAX;
		uba->Slots  =
			(KS10UBA_IF *)calloc(UBA_MAX, sizeof(KS10UBA_IF));
		if (uba->Slots == NULL) {
			free(uba);
			return NULL;
		}
		for (idx = 0; idx < UBA_MAX; idx++) {
			uif = &uba->Slots[idx];
			uif->idSlot = idx;
			uif->uba    = uba;
		}

		// Link them together.
		uba->System       = newMap->sysDevice;
		uba->Processor    = newMap->devParent->Device;
		uba->Callback     = &ks10uba_Callback;
		ks10              = (KS10_DEVICE *)uba->Processor;
		ks10->uba         = uba;
		newMap->Device    = uba;
		newMap->sysDevice = uba->Processor;
		newMap->Callback  = uba->Callback;
	}

	return uba;
}

DEVICE uba_Device;
void *ks10uba_Configure(MAP_DEVICE *newMap, int argc, char **argv)
{
	KS10UBA_DEVICE *uba = (KS10UBA_DEVICE *)newMap->devParent->Device;
	KS10UBA_IF     *uif = NULL;
	MAP_IO         *io;
	int            unit;

	unit = GetDeviceUnit(newMap->devName);
	if (unit < uba->nSlots) {
		uif = &uba->Slots[unit];

		// Check unit is existing or not first
		if (uif->Flags & UIF_EXIST) {
			printf("%s: Already created on %s:\n",
				newMap->devName, uba->devName);
			return NULL;
		}

		// UBA - Map Registers
		io               = &uif->mapMap;
		io->devName      = newMap->devName;
		io->keyName      = UBA_KEY;
		io->emuName      = "UBA - Map Registers";
		io->emuVersion   = "";
		io->uqDevice     = uif;
		io->Device       = uif;
		io->csrAddr      = MAP_ADDR;
		io->nRegs        = MAP_NREG;
		io->ReadIO32     = ks10uba_ReadMAP;
		io->WriteIO32    = ks10uba_WriteMAP;
		ks10uba_SetMap(uif, io);

		// UBA - Status Register
		io = &uif->srMap;
		io->devName      = newMap->devName;
		io->keyName      = UBA_KEY;
		io->emuName      = "UBA - Status Register";
		io->emuVersion   = "";
		io->uqDevice     = uif;
		io->Device       = uif;
		io->csrAddr      = SR_ADDR;
		io->nRegs        = SR_NREG;
		io->ReadIO32     = ks10uba_ReadSR;
		io->WriteIO32    = ks10uba_WriteSR;
		ks10uba_SetMap(uif, io);
		
		// UBA - Maintenace Register
		io = &uif->mrMap;
		io->devName      = newMap->devName;
		io->keyName      = UBA_KEY;
		io->emuName      = "UBA - Maintenance Register";
		io->emuVersion   = "";
		io->uqDevice     = uif;
		io->Device       = uif;
		io->csrAddr      = MR_ADDR;
		io->nRegs        = MR_NREG;
		io->ReadIO32     = ks10uba_ReadMR;
		io->WriteIO32    = ks10uba_WriteMR;
		ks10uba_SetMap(uif, io);
	
		// Finally, set up descriptions and return.
		uif->Unit.devName    = newMap->devName;
		uif->Unit.keyName    = UBA_KEY;
		uif->Unit.emuName    = UBA_NAME;
		uif->Unit.emuVersion = UBA_VERSION;
		uif->Flags           = UIF_EXIST;
		newMap->idUnit       = uif->idSlot;
		newMap->keyName      = uif->Unit.keyName;
		newMap->emuName      = uif->Unit.emuName;
		newMap->emuVersion   = uif->Unit.emuVersion;
		newMap->devInfo      = &uba_Device;
		newMap->Device       = uif;
		newMap->sysDevice    = uif;
//		newMap->sysDevice    = newMap->devParent->sysDevice;
		newMap->sysMap       = newMap->devParent->sysMap;
		newMap->Callback     = newMap->devParent->Callback;
	}
	return uif;
}

int ks10uba_Info(MAP_DEVICE *map, int argc, char **argv)
{
	KS10UBA_IF *uba = (KS10UBA_IF *)map->Device;
	MAP_IO     *io;

	// Display a listing of I/O device.
	if (uba->ioList) {
		for (io = uba->ioList; io; io = io->Next) {
			printf("%-20s %06o (%05X) %03o (%03X) %3d\n",
				io->emuName, io->csrAddr, io->csrAddr,
				io->intVector[0], io->intVector[0], io->nRegs);	
		}
	} else
		printf("\nNo I/O devices.\n\n");

	return UQ_OK;
}

extern int ks10_BootDevice(UQ_BOOT *, int, char **);

UQ_CALL ks10uba_Callback =
{
	NULL,               // Read Data I/O
	NULL,               // Write Data I/O
	ks10uba_ReadBlock,  // Read Block I/O
	ks10uba_WriteBlock, // Write Block I/O
	NULL,               // Get Host Address

	ks10uba_SetMap,     // SetMap Routine
	ks10_BootDevice,    // Boot Device Routine
};

DEVICE uba_Device =
{
	UBA_KEY,           // Device Type Name
	UBA_NAME,          // Interface Name
	UBA_VERSION,       // Interface Version
	uq_Devices,        // Listing of Unibus/Qbus Devices
	DF_USE|DF_SYSMAP,  // Device Flags
	DT_DEVICE,         // Device Type

	NULL, NULL, NULL,  // Commands

	ks10uba_Create,    // Create Routine
	ks10uba_Configure, // Configure Routine
	NULL,              // Delete Routine
	NULL,              // Reset Routine
	NULL,              // Attach Routine
	NULL,              // Detach Routine
	ks10uba_Info,      // Info Routine
	NULL,              // Boot Routine
	NULL,              // Execute Routine
#ifdef DEBUG
	NULL,              // Debug Routine
#endif /* DEBUG */
};
