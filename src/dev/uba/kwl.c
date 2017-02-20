// kwl.c - KW11L Line Clock Emulator
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

#include "dec/kwl.h"

void kwl_ResetClock(KWL_CLOCK *kwl)
{
	MAP_IO *io = &kwl->ioMap;

	CLKCSR = CLKCSR_DONE;
	io->CancelInterrupt(io, 0);
}

int kwl_Tick(void *dptr)
{
	KWL_CLOCK *kwl = (KWL_CLOCK *)dptr;
	MAP_IO    *io  = &kwl->ioMap;

	CLKCSR |= CLKCSR_DONE;
	if (CLKCSR & CLKCSR_IE)
		io->SendInterrupt(io, 0);

	return TS10_OK;
}

int kwl_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	KWL_CLOCK *kwl = (KWL_CLOCK *)dptr;

	*data = CLKCSR;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) CLKCSR (%o) => %06o\n",
			kwl->Unit.devName, pAddr, *data);
#endif /* DEBUG */

	return UQ_OK;
}

int kwl_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	KWL_CLOCK *kwl = (KWL_CLOCK *)dptr;
	MAP_IO    *io  = &kwl->ioMap;

	if (pAddr & 1)
		return UQ_OK;

	// Update IE or DONE bit on Clock CSR register.
	CLKCSR = (data & CLKCSR_RW) | (CLKCSR & ~CLKCSR_RW);
	if (data & CLKCSR_DONE)
		CLKCSR &= ~CLKCSR_DONE;

	// Unless DONE+IE set, clear interrupt request.
	if (((CLKCSR & CLKCSR_DONE) == 0) || ((CLKCSR & CLKCSR_IE) == 0))
		io->CancelInterrupt(io, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) CLKCSR (%o) <= %06o (Now: %06o)\n",
			kwl->Unit.devName, pAddr, data, CLKCSR);
#endif /* DEBUG */

	return UQ_OK;
}

// Create/Initialize KW11L device.
// Usage: create <device> <KW11L> ...
void *kwl_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	KWL_CLOCK *kwl = NULL;
	MAP_IO    *io;

	if (kwl = (KWL_CLOCK *)calloc(1, sizeof(KWL_CLOCK))) {
		// First, set up its description,
		kwl->Unit.devName    = newMap->devName;
		kwl->Unit.keyName    = newMap->keyName;
		kwl->Unit.emuName    = newMap->emuName;
		kwl->Unit.emuVersion = newMap->emuVersion;
		kwl->Device          = newMap->devParent->Device;
		kwl->Callback        = newMap->devParent->Callback;
		kwl->System          = newMap->devParent->sysDevice;

		// Set up I/O map settings
		io               = &kwl->ioMap;
		io->devName      = kwl->Unit.devName;
		io->keyName      = kwl->Unit.keyName;
		io->emuName      = kwl->Unit.emuName;
		io->emuVersion   = kwl->Unit.emuVersion;
		io->Device       = kwl;
		io->csrAddr      = KWL_CSRADR;
		io->nRegs        = KWL_NREGS;
		io->nVectors     = KWL_NVECS;
		io->intIPL       = KWL_IPL;
		io->intVector[0] = KWL_VEC;
		io->ReadIO       = kwl_ReadIO;
		io->WriteIO      = kwl_WriteIO;

		// Assign that registers to QBA's I/O space.
		kwl->Callback->SetMap(kwl->Device, io);

		// Finally, link it to its mapping device.
		newMap->Device   = kwl;
		newMap->Callback = kwl->Callback;
	}

	return kwl;
}

DEVICE kwl_Device =
{
	KWL_KEY,      // Device Type (Key) Name
	KWL_NAME,     // Emulator Name
	KWL_VERSION,  // Emulator Version

	NULL,         // Listing of devices
	DF_SYSMAP,    // Device Flags
	DT_CLOCK,     // Device Type

	NULL,         // Commands
	NULL,         // Set Commands
	NULL,         // Show Commands

	kwl_Create,   // Create Routine
	NULL,         // Configure Routine
	NULL,         // Delete Routine
	NULL,         // Reset Routine
	NULL,         // Attach Routine
	NULL,         // Detach Routine
	NULL,         // Info Routine
	NULL,         // Boot Routine
	NULL,         // Execute Routine
#ifdef DEBUG
	NULL,         // Debug Routine
#endif /* DEBUG */
};
