// ks10_sys.c - KS10 System Configuration
//
// Copyright (c) 2001-2002, Timothy M. Stark
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

#if 0 
// Look at complete information at bottom of this file.
DEVICE ks10_Processor;

void *ks10_Create(MAP_DEVICE *nmap, MAP_DEVICE *map, int argc, char **argv)
{
	KS10_DEVICE *ks10;
	DEVICE     *cptr;
	uint32      args[4];

	if (nmap == NULL) {
		if (ks10 = (KS10_DEVICE *)calloc(1, sizeof(KS10_DEVICE))) {
			map->dtName     = ks10_Processor.dtName;
			map->emuName    = ks10_Processor.emuName;
			map->emuVersion = ks10_Processor.emuVersion;
			map->devInfo    = &ks10_Processor;
			map->Device     = ks10;

			printf("Created %s on %s: - %s %s\n",
				map->dtName, map->devName, map->emuName, map->emuVersion);
		}
		return ks10;
	} else {
		if (cptr = GetDeviceInfo(map, argc, argv)) {
			ks10 = (KS10_DEVICE *)map->Device;

			ks10->uba = cptr->Create(NULL, nmap, argc, argv);

			args[0] = ks10->uba->dtName; // Key ID
//			args[1] = ks10->System;      // PDP-10 System
			args[2] = ks10;              // KS10 Processor
//			args[3] = ks10_Callback;     // Callback functions

			// Initialize UBA interface
			cptr->Setup(ks10->uba, &args);

			nmap->sysDevice = map->sysDevice;
			nmap->sysMap    = map->sysMap;

			return ks10->uba;
		}
		return NULL;
	}
}
#endif

int ks10_Reset(void *);

void *ks10_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	KS10_DEVICE *ks10 = NULL;
	P10_SYSTEM  *p10sys;

	if (ks10 = (KS10_DEVICE *)calloc(1, sizeof(KS10_DEVICE))) {
		// Set up its descriptions on new device.
		ks10->cpu.devName    = newMap->devName;
		ks10->cpu.keyName    = newMap->keyName;
		ks10->cpu.emuName    = newMap->emuName;
		ks10->cpu.emuVersion = newMap->emuVersion;

		// Link it to PDP-10 system device.
		p10sys            = (P10_SYSTEM *)newMap->sysDevice;
		ks10->cpu.System  = p10sys;
		ks10->cpu.Console = NULL;
		p10sys->Processor = (P10_CPU *)ks10;

		// Set KS10 configuration flag
		ks10->cpu.cnfFlags     = CNF_KS10;
		ks10->cpu.szBlock      = ACBLKSIZE;
		ks10->cpu.nBlocks      = NACBLOCKS;
		ks10->cpu.nAccumlators = (NACBLOCKS * ACBLKSIZE);
		ks10->cpu.acBlocks =
			(int36 *)calloc(ks10->cpu.nAccumlators, sizeof(int36));

		// Processor Initialization
		p10_InitMemory(1024 * 1024);
		p10_Initialize((P10_CPU *)ks10);
		ks10_Reset(ks10);

		newMap->Device = ks10;
	}

	return ks10;	
}

int ks10_Reset(void *dptr)
{
	KS10_DEVICE *ks10 = (KS10_DEVICE *)dptr;
 	P10_CPU     *cpu  = (P10_CPU *)ks10;
	int         ac;

	// Clear all ammulators
	for (ac = 0; ac < cpu->nAccumlators; ac++)
		cpu->acBlocks[ac] = 0;

	return EMU_OK;
}

extern DEVICE uba_Device;

DEVICE *ks10_Devices[] =
{
	&uba_Device,       // UBA Interface for KS10 Processor
	NULL
};

DEVICE ks10_Processor =
{
	KS10_KEY,          // Key Name (Device Type)
	KS10_NAME,         // Emulator Name
	KS10_VERSION,      // Emulator Version
	ks10_Devices,      // Lisiting of KS10 Devices
	DF_USE|DF_SYSMAP,  // Device Flags
	DT_PROCESSOR,      // Device Type

	NULL, NULL, NULL,

	ks10_Create,       // Create Routine
	NULL,              // Configure Routine
	NULL,              // Delete Routine
	ks10_Reset,        // Reset Routine
	NULL,              // Attach Routine
	NULL,              // Detach Routine
	NULL,              // Info Routine
	NULL,              // Boot Routine
	NULL,              // Execute Routine
#ifdef DEBUG
	NULL,              // Debug Routine
#endif /* DEBUG */
};

