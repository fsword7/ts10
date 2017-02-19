// bdv.c - BDV11 Boot/Diagnostics ROM Device
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
#include "dec/bdv.h"

// *********************** ROM Page 0 ************************

int bdv_ReadROM0(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	register BDV_ROM *bdv = (BDV_ROM *)dptr;
	register uint16  offAddr = bdv->Page0 | (pAddr & 0377);

	*data = (offAddr < bdv->romSize) ?
		((uint16 *)bdv->romImage)[offAddr >> 1] : 0;

	return UQ_OK;
}

int bdv_WriteROM0(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	register BDV_ROM *bdv = (BDV_ROM *)dptr;

	// Do nothing

	return UQ_OK;
}

// *********************** ROM Page 1 ************************

int bdv_ReadROM1(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	register BDV_ROM *bdv = (BDV_ROM *)dptr;
	register uint16  offAddr = bdv->Page1 | (pAddr & 0377);

	*data = (offAddr < bdv->romSize) ?
		((uint16 *)bdv->romImage)[offAddr >> 1] : 0;

	return UQ_OK;
}

int bdv_WriteROM1(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	register BDV_ROM *bdv = (BDV_ROM *)dptr;

	// Do nothing

	return UQ_OK;
}

// ********************** BDV11 Settings *********************

int bdv_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	register BDV_ROM *bdv = (BDV_ROM *)dptr;
	uint32  reg  = (pAddr >> 1) & 03;

	switch (reg) {
		case nPCR: // Page Control Register
			*data = (bdv->Flags & FLG_KDF11) ? 0101 : PCR;
			break;

		case nSPR: // Scratch Pad Register
			*data = SPR;
			break;

		case nOSR: // Option Select Register
			*data = OSR;
			break;

		default:   // Otherwise - Undefined Registers
			return UQ_NXM;
	}
		
	return UQ_OK;
}

int bdv_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	register BDV_ROM *bdv = (BDV_ROM *)dptr;
	uint32  reg  = (pAddr >> 1) & 03;

	switch (reg) {
		case nPCR: // Page Control Register
			if (acc == ACC_BYTE) // If access is byte, merge with PCR.
				data = (pAddr & 1) ? ((data << 8)   | (PCR & 0377)) :
						               ((data & 0377) | (PCR & ~0377));
			PCR = data & bdv->pcrMask;

			// Update page addresses.
			bdv->Page0 = (data & PCR_MASK) << 8;
			bdv->Page1 = (data & ~PCR_MASK);
			break;

		case nSPR: // Scratch Pad Register
			if (acc == ACC_BYTE) // If access is byte, merge with SPR.
				data = (pAddr & 1) ? ((data << 8)   | (SPR & 0377)) :
						               ((data & 0377) | (SPR & ~0377));
			SPR = data;
			break;

		case nDSP: // Display Register
			if (acc == ACC_BYTE) // If access is byte, merge with SPR.
				data = (pAddr & 1) ? ((data << 8)   | (DSP & 0377)) :
						               ((data & 0377) | (DSP & ~0377));
			DSP = data & 017;
			break;

		default:   // Otherwise - Undefined Registers
			return UQ_NXM;
	}

	return UQ_OK;
}

// Create/Initialize BDV11/KDF11 device.
// Usage: create <device> <BDV11|KDF11> ...
void *bdv_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	BDV_ROM *bdv = NULL;
	MAP_IO  *io;

	if (bdv = (BDV_ROM *)calloc(1, sizeof(BDV_ROM))) {
		// First, set up its description,
		bdv->Unit.devName    = newMap->devName;
		bdv->Unit.keyName    = newMap->keyName;
		bdv->Unit.emuName    = newMap->emuName;
		bdv->Unit.emuVersion = newMap->emuVersion;

		// Recongize which model - BDV11 or KDF11.
		if (!strcmp(bdv->Unit.keyName, BDV_KEY)) {
			bdv->Flags    = FLG_BDV11;
			bdv->maxSize  = BDV_ROMSZ;
			bdv->pcrMask  = BDV_PCRMASK;
			bdv->pcrWidth = BDV_PCRSZ;
			bdv->osrWidth = BDV_OSRSZ;
		} else if (!strcmp(bdv->Unit.keyName, KDF_KEY)) {
			bdv->Flags    = FLG_KDF11;
			bdv->maxSize  = KDF_ROMSZ;
			bdv->pcrMask  = KDF_PCRMASK;
			bdv->pcrWidth = KDF_PCRSZ;
			bdv->osrWidth = KDF_OSRSZ;
		} else {
			printf("%s: *** Bug Check: Unknown device name - %s\n",
				bdv->Unit.devName, bdv->Unit.keyName);
			free(bdv);
			return NULL;
		}

		bdv->Device     = newMap->devParent->Device;
		bdv->Callback   = newMap->devParent->Callback;
		bdv->System     = newMap->devParent->sysDevice;

		// Set up I/O map settings for BDV11 device.
		io               = &bdv->ioMap;
		io->devName      = bdv->Unit.devName;
		io->keyName      = bdv->Unit.keyName;
		io->emuName      = bdv->Unit.emuName;
		io->emuVersion   = bdv->Unit.emuVersion;
		io->Device       = bdv;
		io->csrAddr      = BDV_CSRADR;
		io->nRegs        = BDV_NREGS;
		io->ReadIO       = bdv_ReadIO;
		io->WriteIO      = bdv_WriteIO;

		// Assign that registers to QBA's I/O space.
		bdv->Callback->SetMap(bdv->Device, io);

		// Set up I/O map settings for ROM Page 0
		io               = &bdv->ioPage0;
		io->devName      = bdv->Unit.devName;
		io->keyName      = bdv->Unit.keyName;
		io->emuName      = bdv->Unit.emuName;
		io->emuVersion   = bdv->Unit.emuVersion;
		io->Device       = bdv;
		io->csrAddr      = ROM_PAGE0;
		io->nRegs        = ROM_WSIZE;
		io->ReadIO       = bdv_ReadROM0;
		io->WriteIO      = bdv_WriteROM0;

		// Assign that registers to QBA's I/O space.
		bdv->Callback->SetMap(bdv->Device, io);

		// Set up I/O map settings for ROM Page 1
		io               = &bdv->ioPage1;
		io->devName      = bdv->Unit.devName;
		io->keyName      = bdv->Unit.keyName;
		io->emuName      = bdv->Unit.emuName;
		io->emuVersion   = bdv->Unit.emuVersion;
		io->Device       = bdv;
		io->csrAddr      = ROM_PAGE1;
		io->nRegs        = ROM_WSIZE;
		io->ReadIO       = bdv_ReadROM1;
		io->WriteIO      = bdv_WriteROM1;

		// Assign that registers to QBA's I/O space.
		bdv->Callback->SetMap(bdv->Device, io);

		// Finally, link it to its mapping device.
		newMap->Device   = bdv;
		newMap->Callback = bdv->Callback;
	}

	return bdv;
}

int bdv_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	BDV_ROM *bdv = (BDV_ROM *)map->Device;
	int     romFile;
	int     szImage;
	int     rc;

	if (bdv->romImage) {
		printf("%s: Already attached.  Please use DETACH first.\n",
			bdv->Unit.devName);
		return EMU_OPENERR;
	}

	if ((romFile = open(argv[2], O_RDONLY)) < 0) {
		printf("%s: File '%s': %s\n",
			bdv->Unit.devName, argv[2], strerror(errno));
		return EMU_OPENERR;
	}

	if ((szImage = lseek(romFile, 0, SEEK_END)) < 0) {
		printf("%s: File 's': %s\n",
			bdv->Unit.devName, argv[2], strerror(errno));
		close(romFile);
		return EMU_OPENERR;
	}
	
	if (szImage > bdv->maxSize) {
		printf("%s: File '%s': Too large ROM image (%d bytes > %d max bytes)\n",
			bdv->Unit.devName, argv[2], szImage, bdv->maxSize);
		close(romFile);
		return EMU_OPENERR;
	}

	bdv->romFile  = (char *)malloc(strlen(argv[2])+1);
	strcpy(bdv->romFile, argv[2]);

	bdv->romImage = (uint8 *)malloc(szImage);
	lseek(romFile, 0, SEEK_SET);
	if ((rc = read(romFile, bdv->romImage, szImage)) < szImage) {
		printf("%s: File '%s': %s\n", bdv->Unit.devName, argv[2],
			(rc < 0) ? strerror(errno) : "Too Short Image - Prematured EOF");
		if (rc >= 0) printf("%s:   %d bytes read (%d bytes expected).\n",
				bdv->Unit.devName, rc, szImage);

		// Release ROM and filename space.
		close(romFile);
		free(bdv->romFile);
		free(bdv->romImage);
		bdv->romFile  = NULL;
		bdv->romImage = NULL;

		return EMU_OPENERR;
	}
	bdv->romSize  = szImage;
	close(romFile);
	OSR = 07773;

	printf("%s: ROM File '%s' had been loaded.\n",
		bdv->Unit.devName, argv[2]);

	return EMU_OK;
}

int bdv_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	BDV_ROM *bdv = (BDV_ROM *)map->Device;

	if (bdv->romImage) {
		// Remove ROM image from BDV11/KDF11 Boot ROM device.
		free(bdv->romImage);
		bdv->romImage = NULL;
		bdv->romSize  = 0;

		// Tell operator that ROM image had been removed.
		printf("%s: ROM File '%s' had been removed.\n",
			bdv->Unit.devName, bdv->romFile);

		// Finally, delete ROM filename and return.
		free(bdv->romFile);
		bdv->romFile = NULL;

		return EMU_OK;
	}

	printf("%s: Already detached.\n", bdv->Unit.devName);
	return EMU_OK;
}

int bdv_Info(MAP_DEVICE *map, int argc, char **argv)
{
	BDV_ROM *bdv = (BDV_ROM *)map->Device;

	printf("\nDevice:       %s  Type: %s\n",
		bdv->Unit.devName, bdv->Unit.keyName);
	printf("ROM File:     %s\n",
		bdv->romImage ? bdv->romFile : "(Not Loaded)");
	printf("ROM Size:     %d of %d bytes\n",
		bdv->romSize, bdv->maxSize);

	return EMU_OK;
}

// BDV11 ROM Device
DEVICE bdv_Device =
{
	BDV_KEY,      // Device Type (Key) Name
	BDV_NAME,     // Emulator Name
	BDV_VERSION,  // Emulator Version

	NULL,         // Listing of devices
	DF_SYSMAP,    // Device Flags
	DT_NETWORK,   // Device Type

	NULL,         // Commands
	NULL,         // Set Commands
	NULL,         // Show Commands

	bdv_Create,   // Create Routine
	NULL,         // Configure Routine
	NULL,         // Delete Routine
	NULL,         // Reset Routine
	bdv_Attach,   // Attach Routine
	bdv_Detach,   // Detach Routine
	bdv_Info,     // Info Routine
	NULL,         // Boot Routine
	NULL,         // Execute Routine
#ifdef DEBUG
	NULL,         // Debug Routine
#endif /* DEBUG */
};

// KDF11 ROM Device for F11 processor
DEVICE kdf_Device =
{
	KDF_KEY,      // Device Type (Key) Name
	BDV_NAME,     // Emulator Name
	BDV_VERSION,  // Emulator Version

	NULL,         // Listing of devices
	DF_SYSMAP,    // Device Flags
	DT_NETWORK,   // Device Type

	NULL,         // Commands
	NULL,         // Set Commands
	NULL,         // Show Commands

	bdv_Create,   // Create Routine
	NULL,         // Configure Routine
	NULL,         // Delete Routine
	NULL,         // Reset Routine
	bdv_Attach,   // Attach Routine
	bdv_Detach,   // Detach Routine
	bdv_Info,     // Info Routine
	NULL,         // Boot Routine
	NULL,         // Execute Routine
#ifdef DEBUG
	NULL,         // Debug Routine
#endif /* DEBUG */
};
