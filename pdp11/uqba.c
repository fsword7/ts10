// uqba.c - PDP-11 Unibus/QBus Interface Support Routines
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

#include "pdp11/defs.h"
#include "pdp11/uqba.h"

void uq11_ResetIO(register UQ_IO *uq)
{
}

void uq11_ResetAll(register UQ_IO *uq)
{
	MAP_IO *io;

	if (uq->ioList == NULL) {
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: Unibus Initalization - No Devices.\n",
				uq->Unit.devName);
#endif /* DEBUG */
		return;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		dbg_Printf("%s: Unibus Initialization Begins:\n", uq->Unit.devName);
		for (io = uq->ioList; io; io = io->Next) {
			dbg_Printf("%s:   Reseting %s: (%s) ... ",
				uq->Unit.devName, io->devName, io->keyName);
			if (io->ResetIO) {
				io->ResetIO(io->Device);
				dbg_Printf("Done.\n");
			} else
				dbg_Printf("Skipped.\n");
		}
		dbg_Printf("%s: End of Unibus Initialization.\n", uq->Unit.devName);
	} else
#endif /* DEBUG */

	for (io = uq->ioList; io; io = io->Next)
		if (io->ResetIO)
			io->ResetIO(io->Device);
}

// Evaluate Interrupts
void uq11_EvalIRQ(register P11_CPU *p11, uint32 ipl)
{
	register UQ_IO *uq = p11->uqba;
	uint16 pirq = PIRQ;
	uint32 idx;

	for (idx = IPL_HLVL - 1; idx > ipl; idx--) {
		if (uq->intReqs[idx] || ((int16)pirq < 0)) {
			TIRQ |= TRAP_INT;
			return;
		}
		pirq <<= 1;
	}
	TIRQ &= ~TRAP_INT;
}

void uq11_ClearIRQ(register UQ_IO *uq)
{
	uint32 idx;

	// Clear All Interrupts
	for (idx = 0; idx < IPL_HLVL; idx++)
		uq->intReqs[idx] = 0;
}

// Acknowledge Interrupt and Get Vector
uint16 uq11_GetVector(register P11_CPU *p11, uint32 nipl)
{
	register UQ_IO *uq = p11->uqba;
	uint16 pirq = PIRQ;
	uint32 ipl, vec, lvl;
	MAP_IO *io;

	for (ipl = IPL_HLVL - 1; ipl > nipl; ipl--) {
		if (lvl = uq->intReqs[ipl]) {
			for (vec = 0; vec < IPL_NVECS; vec++) {
				if ((lvl >> vec) & 1) {
					uq->intReqs[ipl] &= ~(1u << vec);
//					if ((io = uq->intAck[ipl][vec]) && io->AckInterrupt)
//						return io->AckInterrupt(io->Device);
#ifdef DEBUG
					if (dbg_Check(DBG_INTERRUPT))
						dbg_Printf("%s: IRQ BR%d, Level %d, Vector %03o\n",
							uq->Unit.devName, ipl, vec, uq->intVecs[ipl][vec]);
#endif /* DEBUG */
					return uq->intVecs[ipl][vec];
				}
			}
		}
		if ((int16)pirq < 0)
			return VEC_PIRQ;
		pirq <<= 1;
	}

	// No interrupt requests.
	return 0;
}

void uq11_SetVector(MAP_IO *io, int newVector, int idx)
{
	UQ_IO  *uq = (UQ_IO *)io->uqDevice;
	uint32 ipl, lvl;

	if ((ipl = io->intIPL) && (lvl = io->intLevel[idx])) {
		uq->intVecs[ipl][lvl] = newVector;
		io->intVector[idx]    = newVector;
	}
}

void uq11_SendInterrupt(MAP_IO *io, int idx)
{
	UQ_IO   *uq  = (UQ_IO *)io->uqDevice;
	P11_CPU *p11 = uq->Processor;
	uint32  ipl, mask;

	if ((ipl = io->intIPL) && (mask = io->intMask[idx])) {
		uq->intReqs[ipl] |= mask;
		uq11_EvalIRQ(p11, GET_IPL(PSW));
#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Ring doorbell, IRQ BR%d, Level %d, Vector %03o\n",
				io->devName, ipl, io->intLevel[idx], io->intVector[idx]);
#endif /* DEBUG */
	}
}

void uq11_CancelInterrupt(MAP_IO *io, int idx)
{
	UQ_IO   *uq  = (UQ_IO *)io->uqDevice;
	P11_CPU *p11 = uq->Processor;
	uint32  ipl, mask;

	if ((ipl = io->intIPL) && (mask = io->intMask[idx])) {
		uq->intReqs[ipl] &= ~mask;
		uq11_EvalIRQ(p11, GET_IPL(PSW));
#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Cancel doorbell, IRQ BR%d, Level %d, Vector %03o\n",
				io->devName, ipl, io->intLevel[idx], io->intVector[idx]);
#endif /* DEBUG */
	}
}

// Read 8/16-bit data from a 32-bit map register.
int uq11_ReadMAP(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	UQ_IO *uq = (UQ_IO *)dptr;

	// Get 16-bit data from anywhere within a 32-bit map register.
	*data = uq->map[(pAddr >> 2) & UBM_PAGE] >> ((pAddr & 2) ? 16 : 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		uint32 pn = (pAddr >> 2) & UBM_PAGE;
		dbg_Printf("%s: (R) MAP %02o (%o) => %08o (%06o)\n",
			uq->Unit.devName, pn, pAddr, uq->map[pn], *data);
	}
#endif /* DEBUG */

	return UQ_OK;
}

// Write 8/16-bit data to a 32-bit map register.
int uq11_WriteMAP(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{ 
	UQ_IO  *uq = (UQ_IO *)dptr;
	uint32 sc, pn = (pAddr >> 2) & UBM_PAGE;

	// Put 8/16-bit data into anywhere within a 32-bit map register.
	if (acc == ACC_BYTE) {
		sc = (pAddr & 3) << 3;
		uq->map[pn] = ((data & 0377) << sc) |
			(uq->map[pn] & ~(0377 << sc));
	} else {
		sc = (pAddr & 2) << 3;
		uq->map[pn] = ((uint32)data << sc) |
			(uq->map[pn] & ~(0177777 << sc));
	}

	uq->map[pn] &= 017777776;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) MAP %02o (%o) <= %08o (%06o)\n",
			uq->Unit.devName, pn, pAddr, uq->map[pn], data);
#endif /* DEBUG */

	return UQ_OK;
}

inline uint32 uq11_MapAddr(register UQ_IO *uq, uint32 ioAddr)
{
	uint32 pn, mapAddr;

	if (uq->Flags & UQ_BME) {
		uint32 pn = UBM_GETPN(ioAddr);
		return ((pn < UBM_PAGE) ? uq->map[pn] : IO_PAGEBASE)
			+ UBM_GETOFF(ioAddr);
	}
	return ioAddr;
}

uint32 uq11_ReadBlock(void *dptr, uint32 ioAddr, uint8 *data,
	uint32 szBytes, uint32 mode)
{
	P11_CPU *p11 = (P11_CPU *)dptr;
	UQ_IO   *uq  = p11->uqba;
	uint32  mapAddr, idxAddr = 0;
	uint32  cntBytes;

	// Align data with starting
//	ioAddr  &= ~01;
//	szBytes &= ~01;

	cntBytes = UBM_BLKSZ - (ioAddr & UBM_BLKOFF);
	if (szBytes < cntBytes)
		cntBytes = szBytes;

	while (szBytes) {
		mapAddr = uq11_MapAddr(uq, ioAddr);
		if (mapAddr >= p11->ramSize)
			return szBytes;

		memcpy(&data[idxAddr], &((uint8 *)p11->ramData)[mapAddr], cntBytes);

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA)) {
			dbg_Printf("I/O Address: %08o  Host Address: %08o  Size: %d bytes\n",
				ioAddr, mapAddr, cntBytes);
			PrintDump(idxAddr, &data[idxAddr], cntBytes);
		}
#endif /* DEBUG */

		// Increment them by 512-bytes block.
		ioAddr   += cntBytes;
		idxAddr  += cntBytes;
		szBytes  -= cntBytes;
		cntBytes  = (szBytes > UBM_BLKSZ) ? UBM_BLKSZ : szBytes;
	}

	return szBytes;
}

uint32 uq11_WriteBlock(void *dptr, uint32 ioAddr, uint8 *data,
	uint32 szBytes, uint32 mode)
{
	P11_CPU *p11 = (P11_CPU *)dptr;
	UQ_IO   *uq  = p11->uqba;
	uint32  mapAddr, idxAddr = 0;
	uint32  cntBytes;

	// Align data with starting
//	ioAddr  &= ~01;
//	szBytes &= ~01;

	cntBytes = UBM_BLKSZ - (ioAddr & UBM_BLKOFF);
	if (szBytes < cntBytes)
		cntBytes = szBytes;

	while (szBytes) {
		mapAddr = uq11_MapAddr(uq, ioAddr);
		if (mapAddr >= p11->ramSize)
			return szBytes;

		memcpy(&((uint8 *)p11->ramData)[mapAddr], &data[idxAddr], cntBytes);

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA)) {
			dbg_Printf("I/O Address: %08o  Host Address: %08o  Size: %d bytes\n",
				ioAddr, mapAddr, cntBytes);
			PrintDump(idxAddr, &data[idxAddr], cntBytes);
		}
#endif /* DEBUG */

		// Increment them by 512-bytes block.
		ioAddr   += cntBytes;
		idxAddr  += cntBytes;
		szBytes  -= cntBytes;
		cntBytes  = (szBytes > UBM_BLKSZ) ? UBM_BLKSZ : szBytes;
	}

	return szBytes;
}

int uq11_ReadIO(register UQ_IO *uq, uint32 pAddr, uint16 *data, uint32 size)
{
	uint32 ioAddr = pAddr & IO_MASK;
	MAP_IO *io;

	if (io = uq->ioMap[ioAddr >> 1]) {
		int rc = io->ReadIO(io->Device, ioAddr, data, size);
		if (size == ACC_BYTE)
			*data = (pAddr & 1) ? (*data >> 8) : (*data & 0377);
		return rc;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		P11_CPU *p11 = uq->Processor;
		dbg_Printf("%s: (R) Non-Existant Device (%o) at PC %06o\n",
			uq->Unit.devName, pAddr, MMR2);
	}
#endif /* DEBUG */

	*data = 0;
	return UQ_NXM;
}

int uq11_WriteIO(register UQ_IO *uq, uint32 pAddr, uint16 data, uint32 size)
{
	uint32 ioAddr = pAddr & IO_MASK;
	MAP_IO *io;

	if (io = uq->ioMap[ioAddr >> 1])
		return io->WriteIO(io->Device, ioAddr, data, size);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		P11_CPU *p11 = uq->Processor;
		dbg_Printf("%s: (W) Non-Existant Device (%o) at PC %06o\n",
			uq->Unit.devName, pAddr, MMR2);
	}
#endif /* DEBUG */

	return UQ_NXM;
}

int uq11_SetMap(void *dptr, MAP_IO *io)
{
	UQ_IO   *uq = (UQ_IO *)dptr;
	P11_CPU *p11 = uq->Processor;
	MAP_IO  *cptr, *pptr = NULL;
	int     csrAddr = (io->csrAddr & 017777) >> 1;
	int     idx, ipl, lvl;

	// Check free area first. If any reserved area,
	// Report that any or all area is already reserved.
	for (idx = 0; idx < io->nRegs; idx++)
		if (uq->ioMap[csrAddr + idx])
			return UQ_RESERVED;

	// Check free vector slots first.  If reserved area is
	// near full or full, report that.
	if (ipl = io->intIPL) {
		for (idx = 0; idx < io->nVectors; idx++) {
			// Find a first vacant slot.
			for (lvl = 0; lvl < IPL_NVECS; lvl++)
				if (((uq->intRsvd[ipl] >> lvl) & 1) == 0)
					break;
			// Check any space is enough to fit.
			if ((lvl + io->nVectors) > IPL_NVECS)
				return UQ_RSVDVEC;
		}
	}

	// Convert 16/18/22-bit address to 18-bit address.
	io->csrAddr  = (io->csrAddr & 017777) | 0760000;
	io->uqDevice = uq;

	// Set up callback functions for this Unibus/QBus interface.
	io->SetVector       = uq11_SetVector;
	io->SendInterrupt   = uq11_SendInterrupt;
	io->CancelInterrupt = uq11_CancelInterrupt;

	// Now assign device registers to call.
	for (idx = 0; idx < io->nRegs; idx++)
		uq->ioMap[csrAddr + idx] = io;

	// Now assign interrupt vectors to interrupt table.
	if (ipl && io->nVectors) {
		for (idx = 0; idx < io->nVectors; idx++, lvl++) {
			// Assign vector slot to device.
			uq->intRsvd[ipl]      |= (1u << lvl);
			uq->intVecs[ipl][lvl]  = io->intVector[idx];
			io->intLevel[idx]      = lvl;
			io->intMask[idx]       = (1u << lvl);
		}
	}

	// Insert a I/O node into a device list.
	if (uq->ioList == NULL) {
		io->Next   = NULL;
		uq->ioList = io;
	} else {
		// First, check an I/O map already was linked.
		// If so, do nothing and return.
		for (cptr = uq->ioList; cptr; cptr = cptr->Next)
			if (cptr == io) return UQ_OK;

		// Find space by ascending I/O address.
		for (cptr = uq->ioList; cptr; cptr = cptr->Next) {
			if (io->csrAddr < cptr->csrAddr)
				break;
			pptr = cptr;
		}

		// Link an I/O map to a list.
		if (cptr == uq->ioList) {
			io->Next = uq->ioList;
			uq->ioList = io;
		} else {
			io->Next = pptr->Next;
			pptr->Next = io;
		}
	}

	return UQ_OK;
}

// ***********************************************************

UQ_CALL uq11_Callback;
void *uq11_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	UQ_IO   *uq = NULL;
	P11_CPU *p11;
	MAP_IO  *io;

	if (uq = (UQ_IO *)calloc(1, sizeof(UQ_IO))) {
		uq->Unit.devName    = newMap->devName;
		uq->Unit.keyName    = newMap->keyName;
		uq->Unit.emuName    = newMap->emuName;
		uq->Unit.emuVersion = newMap->emuVersion;

		// Set up UQ11 and P11 devices and
		// link them together.
		uq->System    = newMap->sysDevice;
		uq->Processor = newMap->devParent->Device;
		uq->Callback  = &uq11_Callback;
		p11           = (P11_CPU *)uq->Processor;
		p11->uqba     = uq;

		// Set Map Registers
		io             = &uq->ioUBM;
		io->devName    = uq->Unit.devName;
		io->keyName    = uq->Unit.keyName;
		io->emuName    = "Unibus Map Registers";
		io->emuVersion = NULL;
		io->Device     = uq;
		io->csrAddr    = UBM_CSRADR;
		io->nRegs      = UBM_NREGS;
		io->ReadIO     = uq11_ReadMAP;
		io->WriteIO    = uq11_WriteMAP;
		
		// Assign that registers to UQBA's I/O space.
		uq->Callback->SetMap(uq, io);

		// Assign CPU Registers to I/O Space.
		p11->InitRegs(p11);

		// Initialize UQ11 device.
		uq11_ResetIO(uq);

		newMap->Device    = uq;
		newMap->Callback  = uq->Callback;
		newMap->sysDevice = uq->Processor;
	}

	return uq;
}

int uq11_Reset(void *dptr)
{
	uq11_ResetIO(dptr);
}

int uq11_Info(MAP_DEVICE *map, int argc, char **argv)
{
	UQ_IO  *uq = (UQ_IO *)map->Device;
	MAP_IO *io;

	// Display a listing of I/O device.
	if (uq->ioList) {
		for (io = uq->ioList; io; io = io->Next) {
			printf("%-20s %06o %03o %03o %3d\n",
				io->emuName, io->csrAddr,
				io->intVector[0], io->intVector[1], io->nRegs);	
		}
	} else
		printf("\nNo I/O devices.\n\n");

	return UQ_OK;
}

UQ_CALL uq11_Callback =
{
	NULL,              // Read Data I/O
	NULL,              // Write Data I/O
	uq11_ReadBlock,    // Read Block I/O
	uq11_WriteBlock,   // Write Block I/O
	NULL,              // Get Host Address

	uq11_SetMap,       // SetMap Routine
};

// PDP-11 Unibus/QBus Device
DEVICE uq11_Device =
{
	// Description Information
	UQ_KEY,              // Device Type Name
	UQ_NAME,             // Emulator Name
	UQ_VERSION,          // Emulator Version

	uq_Devices,          // Unibus/Qbus Device Listings
	DF_USE|DF_SYSMAP,    // Device Flags
	DT_DEVICE,           // Device Type

	NULL, NULL, NULL,    // Commands

	uq11_Create,         // Create Routine
	NULL,                // Configure Routine
	NULL,                // Delete Routine
	uq11_Reset,          // Reset Routine
	NULL,                // Attach Routine
	NULL,                // Detach Routine
	uq11_Info,           // Info Routine
	NULL,                // Boot Routine
	NULL,                // Execute Routine
#ifdef DEBUG
	NULL,                // Debug Routine
#endif /* DEBUG */
};
