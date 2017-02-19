// mrv.c - MRV11 Boot ROM Device
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
#include "dec/mrv.h"

// ****************** ROM Area #1 (765000) ********************

int mrv_ReadROM65(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	register MRV_ROM *mrv    = (MRV_ROM *)dptr;
	register uint16  offAddr = mrv->Page65 | (pAddr & ROM_PAGE);

	if (CSR & CSR_DIS65) {
		*data = 0;
		return UQ_OK;
	}
	if (CSR & CSR_ROM3) {
		*data = 0;
		return UQ_OK;
	}

	*data = (offAddr < mrv->romSize) ?
		((uint16 *)mrv->romImage)[offAddr >> 1] : 0;

	return UQ_OK;
}

int mrv_WriteROM65(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	register MRV_ROM *mrv = (MRV_ROM *)dptr;

	if (CSR & CSR_DIS65)
		return UQ_OK;
	// Do nothing
	return UQ_OK;
}

// ****************** ROM Area #2 (773000) ********************

int mrv_ReadROM73(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	register MRV_ROM *mrv    = (MRV_ROM *)dptr;
	register uint16  offAddr = mrv->Page73 | (pAddr & ROM_PAGE);

	if (CSR & CSR_DIS73) {
		*data = 0;
		return UQ_OK;
	}

	*data = (offAddr < mrv->romSize) ?
		((uint16 *)mrv->romImage)[offAddr >> 1] : 0;

	return UQ_OK;
}

int mrv_WriteROM73(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	register MRV_ROM *mrv = (MRV_ROM *)dptr;

	if (CSR & CSR_DIS73)
		return UQ_OK;
	// Do nothing
	return UQ_OK;
}

// ******************** MRV Settings *******************

int mrv_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	MRV_ROM *mrv = (MRV_ROM *)dptr;
	uint32  reg  = (pAddr >> 1) & 03;

	switch (reg) {
		case nCSR: // Control/Status Register
			*data = CSR;
			break;

		case nPCR: // Page Control Register
			*data = PCR;
			break;

		case nCR: // Configuration Register
			*data = CR;
			break;

		default:   // Otherwise - Undefined Registers
			return UQ_NXM;
	}
		
	return UQ_OK;
}

int mrv_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	MRV_ROM *mrv = (MRV_ROM *)dptr;
	uint32  reg  = (pAddr >> 1) & 03;

	switch (reg) {
		case nCSR: // Control/Status Register
			if (acc == ACC_BYTE) // If access is byte, merge with CSR.
				data = (pAddr & 1) ? ((data << 8)   | (CSR & 0377)) :
						               ((data & 0377) | (CSR & ~0377));
			CSR = data & CSR_RW;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) CSR <= %06o\n",
					mrv->Unit.devName, CSR);
#endif /* DEBUG */
			break;

		case nPCR: // Page Control Register
			if (acc == ACC_BYTE) // If access is byte, merge with PCR.
				data = (pAddr & 1) ? ((data << 8)   | (PCR & 0377)) :
						               ((data & 0377) | (PCR & ~0377));
			PCR = data & PCR_RW;

			// Set page addresses for ROM access.
			mrv->Page65 = (uint32)(data & PCR_ROM65) << 8;
			mrv->Page73 = (uint32)(data & PCR_ROM73);
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) PCR <= %06o (ROM65=%06o ROM73=%06o)\n",
					mrv->Unit.devName, PCR, mrv->Page65, mrv->Page73);
#endif /* DEBUG */
			break;

		case nDR: // Display Register
			if (acc == ACC_BYTE) // If access is byte, merge with SPR.
				data = (pAddr & 1) ? ((data << 8)   | (DR & 0377)) :
						               ((data & 0377) | (DR & ~0377));
			DR = data & 017;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) DR <= %06o\n",
					mrv->Unit.devName, DR);
#endif /* DEBUG */
			break;

		default:   // Otherwise - Undefined Registers
			return UQ_NXM;
	}

	return UQ_OK;
}

// ****************** MRV11 System Configuration *****************

// Create/Initialize MRV11 device.
// Usage: create <device> <MRV11> ...
void *mrv_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	MRV_ROM *mrv = NULL;
	MAP_IO  *io;

	if (mrv = (MRV_ROM *)calloc(1, sizeof(MRV_ROM))) {
		// First, set up its description,
		mrv->Unit.devName    = newMap->devName;
		mrv->Unit.keyName    = newMap->keyName;
		mrv->Unit.emuName    = newMap->emuName;
		mrv->Unit.emuVersion = newMap->emuVersion;
		mrv->Device          = newMap->devParent->Device;
		mrv->Callback        = newMap->devParent->Callback;
		mrv->System          = newMap->devParent->sysDevice;

		// Set up I/O map settings
		io               = &mrv->ioMap;
		io->devName      = mrv->Unit.devName;
		io->keyName      = mrv->Unit.keyName;
		io->emuName      = mrv->Unit.emuName;
		io->emuVersion   = mrv->Unit.emuVersion;
		io->Device       = mrv;
		io->csrAddr      = MRV_CSRADR;
		io->nRegs        = MRV_NREGS;
		io->ReadIO       = mrv_ReadIO;
		io->WriteIO      = mrv_WriteIO;

		// Assign that registers to QBA's I/O space.
		mrv->Callback->SetMap(mrv->Device, io);

		// Set up I/O map settings for ROM area #1
		io               = &mrv->ioPage65;
		io->devName      = mrv->Unit.devName;
		io->keyName      = mrv->Unit.keyName;
		io->emuName      = mrv->Unit.emuName;
		io->emuVersion   = mrv->Unit.emuVersion;
		io->Device       = mrv;
		io->csrAddr      = ROM65_PAGE;
		io->nRegs        = ROM65_WSIZE;
		io->ReadIO       = mrv_ReadROM65;
		io->WriteIO      = mrv_WriteROM65;

		// Assign that registers to QBA's I/O space.
		mrv->Callback->SetMap(mrv->Device, io);

		// Set up I/O map settings for ROM area #2
		io               = &mrv->ioPage73;
		io->devName      = mrv->Unit.devName;
		io->keyName      = mrv->Unit.keyName;
		io->emuName      = mrv->Unit.emuName;
		io->emuVersion   = mrv->Unit.emuVersion;
		io->Device       = mrv;
		io->csrAddr      = ROM73_PAGE;
		io->nRegs        = ROM73_WSIZE;
		io->ReadIO       = mrv_ReadROM73;
		io->WriteIO      = mrv_WriteROM73;

		// Assign that registers to QBA's I/O space.
		mrv->Callback->SetMap(mrv->Device, io);
		// Finally, link it to its mapping device.
		newMap->Device   = mrv;
		newMap->Callback = mrv->Callback;
	}

	return mrv;
}

int mrv_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	MRV_ROM *mrv = (MRV_ROM *)map->Device;
	int     romFile;
	int     szImage;
	int     rc;

	if (mrv->romImage) {
		printf("%s: Already attached.  Please use DETACH first.\n",
			mrv->Unit.devName);
		return EMU_OPENERR;
	}

	if ((romFile = open(argv[2], O_RDONLY)) < 0) {
		printf("%s: File '%s': %s\n",
			mrv->Unit.devName, argv[2], strerror(errno));
		return EMU_OPENERR;
	}

	if ((szImage = lseek(romFile, 0, SEEK_END)) < 0) {
		printf("%s: File 's': %s\n",
			mrv->Unit.devName, argv[2], strerror(errno));
		close(romFile);
		return EMU_OPENERR;
	}
	
	if (szImage > ROM_SIZE) {
		printf("%s: File '%s': Too large ROM image (%d bytes > %d max bytes)\n",
			mrv->Unit.devName, argv[2], szImage, ROM_SIZE);
		close(romFile);
		return EMU_OPENERR;
	}

	mrv->romFile  = (char *)malloc(strlen(argv[2])+1);
	strcpy(mrv->romFile, argv[2]);

	mrv->romImage = (uint8 *)malloc(szImage);
	lseek(romFile, 0, SEEK_SET);
	if ((rc = read(romFile, mrv->romImage, szImage)) < szImage) {
		printf("%s: File '%s': %s\n", mrv->Unit.devName, argv[2],
			(rc < 0) ? strerror(errno) : "Too Short Image - Prematured EOF");
		if (rc >= 0) printf("%s:   %d bytes read (%d bytes expected).\n",
				mrv->Unit.devName, rc, szImage);

		// Release ROM and filename space.
		close(romFile);
		free(mrv->romFile);
		free(mrv->romImage);
		mrv->romFile  = NULL;
		mrv->romImage = NULL;

		return EMU_OPENERR;
	}
	mrv->romSize  = szImage;
	close(romFile);
//	CR = 07773;
	CSR = 0;
	PCR = 0;
	DR  = 0;
	CR  = 0;

	mrv->Page65 = 0;
	mrv->Page73 = 0;

	printf("%s: ROM File '%s' had been loaded.\n",
		mrv->Unit.devName, argv[2]);

	return EMU_OK;
}

int mrv_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	MRV_ROM *mrv = (MRV_ROM *)map->Device;

	if (mrv->romImage) {
		// Remove ROM image from MRV11 Boot ROM device.
		free(mrv->romImage);
		mrv->romImage = NULL;
		mrv->romSize  = 0;

		// Tell operator that ROM image had been removed.
		printf("%s: ROM File '%s' had been removed.\n",
			mrv->Unit.devName, mrv->romFile);

		// Finally, delete ROM filename and return.
		free(mrv->romFile);
		mrv->romFile = NULL;

		return EMU_OK;
	}

	printf("%s: Already detached.\n", mrv->Unit.devName);
	return EMU_OK;
}

int mrv_Info(MAP_DEVICE *map, int argc, char **argv)
{
	MRV_ROM *mrv = (MRV_ROM *)map->Device;

	printf("\nDevice:       %s  Type: %s\n",
		mrv->Unit.devName, mrv->Unit.keyName);
	printf("ROM File:     %s\n",
		mrv->romImage ? mrv->romFile : "(Not Loaded)");
	printf("ROM Size:     %d of %d bytes\n",
		mrv->romSize, ROM_SIZE);
	printf("ROM Page 65:  %06o\n", mrv->Page65);
	printf("ROM Page 73:  %06o\n", mrv->Page73);

	return EMU_OK;
}

// MRV11 ROM Device
DEVICE mrv_Device =
{
	MRV_KEY,      // Device Type (Key) Name
	MRV_NAME,     // Emulator Name
	MRV_VERSION,  // Emulator Version

	NULL,         // Listing of devices
	DF_SYSMAP,    // Device Flags
	DT_NETWORK,   // Device Type

	NULL,         // Commands
	NULL,         // Set Commands
	NULL,         // Show Commands

	mrv_Create,   // Create Routine
	NULL,         // Configure Routine
	NULL,         // Delete Routine
	NULL,         // Reset Routine
	mrv_Attach,   // Attach Routine
	mrv_Detach,   // Detach Routine
	mrv_Info,     // Info Routine
	NULL,         // Boot Routine
	NULL,         // Execute Routine
#ifdef DEBUG
	NULL,         // Debug Routine
#endif /* DEBUG */
};
