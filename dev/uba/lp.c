// lp.c - LP11/LS11/LA11 Line Printer Emulation
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

#include "emu/defs.h"
#include "dec/lp.h"

void lp_Service(void *dptr)
{
	LP_DEVICE *lp = (LP_DEVICE *)dptr;
	MAP_IO    *io = &lp->ioMap;
	uchar     ch;
	int       err;

	// Print a character.
	if (lp->File) {
		ch = lp->lpdb & 0177;
		if ((err = write(lp->File, &ch, 1)) <= 0) {
			lp->lpcsr |= CSR_ERR;
			if (err < 0)
				printf("%s: *** Error: %s\n", lp->devName, strerror(errno));
		}
	}

	// Update CSR flags and ring its host.
	lp->lpcsr |= CSR_DONE;
	if (lp->lpcsr & CSR_IE) {
		io->SendInterrupt(io, 0);
#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Ring host\n", lp->devName);
#endif /* DEBUG */
	}
}

int lp_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 size)
{
	LP_DEVICE *lp = (LP_DEVICE *)dptr;
	uint32    reg = (pAddr - (lp->csrAddr & 0x1FFF)) >> 1;

	switch (reg) {
		case nLPCSR:
			*data = lp->lpcsr;
			break;

		case nLPDB:
			*data = lp->lpdb;
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) Undefined Register (%06)\n",
					lp->keyName, pAddr & 0x1FFF);
#endif /* DEBUG */
			*data = 0;
			return UQ_NXM;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%06o) => %06o\n",
			lp->keyName, regName[reg], pAddr & 0x1FFF, *data);
#endif /* DEBUG */

	return UQ_OK;
}

int lp_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 size)
{
	LP_DEVICE *lp = (LP_DEVICE *)dptr;
	MAP_IO    *io = &lp->ioMap;
	uint32    reg = (pAddr - (lp->csrAddr & 0x1FFF)) >> 1;

	switch (reg) {
		case nLPCSR:
			if (pAddr & 1)
				break;

			// Update interrupt flags.
			if ((data & CSR_IE) == 0)
				io->CancelInterrupt(io, 0);
			else if ((lp->lpcsr & (CSR_DONE|CSR_IE)) == CSR_DONE) {
				io->SendInterrupt(io, 0);
#ifdef DEBUG
				if (dbg_Check(DBG_INTERRUPT))
					dbg_Printf("%s: Ring Doorbell\n", lp->devName);
#endif /* DEBUG */
			}

			lp->lpcsr = (data & CSR_RW) | (lp->lpcsr & ~CSR_RW);
			break;

		case nLPDB:
			if ((pAddr & 1) == 0) {
				lp->lpdb   = data & DB_DATA;
				lp->lpcsr &= ~CSR_DONE;
				io->CancelInterrupt(io, 0);
				ts10_SetTimer(&lp->svcTimer);
			}
			break;

#ifdef DEBUG
		default:
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) Undefined Register (%06)\n",
					lp->keyName, pAddr & 0x1FFF);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) %s (%06o) <= %06o\n",
			lp->keyName, regName[reg], pAddr & 0x1FFF, data);
#endif /* DEBUG */

	return UQ_OK;
}

// ******************************************************************

int lp_Reset(void *);
void *lp_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	LP_DEVICE *lp = NULL;
	CLK_QUEUE *newTimer;
	MAP_IO    *io;

	if (lp = (LP_DEVICE *)calloc(1, sizeof(LP_DEVICE))) {
		lp->csrAddr    = LP_IOADDR;

		// Set up service timer for transmit
		// characters to the printer.
		newTimer           = &lp->svcTimer;
		newTimer->Next     = NULL;
		newTimer->Flags    = 0;
		newTimer->outTimer = LP_TIMER;
		newTimer->nxtTimer = LP_TIMER;
		newTimer->Device   = (void *)lp;
		newTimer->Execute  = lp_Service;

		// Set up I/O Entry
		io               = &lp->ioMap;
		io->devName      = newMap->devName;
		io->keyName      = newMap->keyName;
		io->emuName      = newMap->emuName;
		io->emuVersion   = newMap->emuVersion;
		io->Device       = lp;
		io->csrAddr      = lp->csrAddr;
		io->nRegs        = LP_NREGS;
		io->nVectors     = LP_NVECS;
		io->intIPL       = LP_IPL;
		io->intVector[0] = LP_VEC;
		io->ReadIO       = lp_ReadIO;
		io->WriteIO      = lp_WriteIO;

		lp->Device   = newMap->devParent->Device;
		lp->Callback = newMap->devParent->Callback;
		lp->System   = newMap->sysDevice;

		// Assign that registers to QBA's I/O Space.
		lp->Callback->SetMap(lp->Device, io);

		lp_Reset(lp);

		// Finally, set up its descriptions and
		// link it to its mapping device.
		lp->devName    = newMap->devName;
		lp->keyName    = newMap->keyName;
		lp->emuName    = newMap->emuName;
		lp->emuVersion = newMap->emuVersion;
		newMap->Device = lp;
	}

	return lp;
}

int lp_Reset(void *dptr)
{
	LP_DEVICE *lp = (LP_DEVICE *)dptr;

	ts10_CancelTimer(&lp->svcTimer);

	lp->lpcsr = CSR_DONE | ((lp->File == 0) ? CSR_ERR : 0);
	lp->lpdb  = 0;

	return EMU_OK;
}

int lp_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	LP_DEVICE *lp  = (LP_DEVICE *)map->Device;

	if ((lp->File = open(argv[2], O_WRONLY, 0700)) < 0) {
		printf("%s: file '%s' not attached - %s.\n",
			lp->devName, argv[2], strerror(errno));
		return EMU_OPENERR;
	}

	// Tell operator that and save it.
	printf("%s: file '%s' attached.\n", lp->devName, argv[2]);
	if (lp->fileName = (char *)malloc(strlen(argv[2])+1)) {
		// Put filename on file and update CSR flags.
		strcpy(lp->fileName, argv[2]);
		lp->lpcsr &= ~CSR_ERR;
	} else {
		printf("%s: Can't assign the name of '%s' - Not enough memory.\n",
			map->devName, argv[2]);
	}

	return EMU_OK;
}

int lp_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	LP_DEVICE *lp = (LP_DEVICE *)map->Device;

	if (lp->File) {
		close(lp->File);
		lp->File = 0;

		// Tell operator that.
		printf("%s: file '%s' detached.\n", lp->devName,
			lp->fileName ? lp->fileName : "<Unknown filename>");

		// Free the allocation of filename if available.
		if (lp->fileName)
			free(lp->fileName);
		lp->fileName = NULL;

		// Update CSR flags
		lp->lpcsr |= CSR_ERR;
	} else
		printf("%s: Already detached.\n", lp->devName);

	return EMU_OK;
}

DEVICE lp_Device =
{
	LP_KEY,        // Device Type Name
	LP_NAME,       // Emulator Name
	LP_VERSION,    // Emulator Version
	NULL,          // Listing of Devices
	DF_SYSMAP,     // Device Flags
	DT_DEVICE,     // Device Type

	NULL, NULL, NULL,

	lp_Create,     // Create Routine
	NULL,          // Configure Routine
	NULL,          // Delete Routine
	lp_Reset,      // Reset Routine
	lp_Attach,     // Attach Routine
	lp_Detach,     // Detach Routine
	NULL,          // Info Routine
	NULL,          // Boot Routine    - Not Used
	NULL,          // Execute Routine - Not Used
#ifdef DEBUG
	NULL,          // Debug Routine
#endif /* DEBUG */
};
