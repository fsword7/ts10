// k110_sys.c - KL10 System Configuration
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
#include "pdp10/kl10.h"
#include "pdp10/iodefs.h"

// ***************************************************************

// Initialize Processor and I/O Systems.
inline void kl10_Initialize(KL10_DEVICE *kl10)
{
//	kx10_InitIO(kl10);  // Initialize I/O System (Must be first)
	kl10_InitAPR(kl10); // Initialize Arithmetic Processor
	kl10_InitPI(kl10);  // Initialize Priority Interrupt
	kl10_InitPAG(kl10); // Initialize Pager
	kl10_InitCCA(kl10); // Initialize Cache
	kl10_InitTIM(kl10); // Initialize Timer
	kl10_InitMTR(kl10); // Initialize Meter
}

int kl10_Reset(void *);
void *kl10_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	KL10_DEVICE *kl10 = NULL;
	P10_SYSTEM  *p10sys;

	if (kl10 = (KL10_DEVICE *)calloc(1, sizeof(KL10_DEVICE))) {
		// Set up its descriptions on new device.
		kl10->cpu.devName    = newMap->devName;
		kl10->cpu.keyName    = newMap->keyName;
		kl10->cpu.emuName    = newMap->emuName;
		kl10->cpu.emuVersion = newMap->emuVersion;

		// Set KL10 configuration flag
		if (!strcmp(newMap->keyName, KL10A_KEY))
			kl10->cpu.cnfFlags = CNF_KL10;
#ifdef OPT_XADR
		else if (!strcmp(newMap->keyName, KL10B_KEY))
			kl10->cpu.cnfFlags = CNF_KL10|CNF_XADR;
#endif /* OPT_XADR */
#ifdef DEBUG
		else {
			printf("%s: *** Bug Check: Unknown device name - %s\n",
				newMap->devName, newMap->keyName);
			free(kl10);
			return NULL;
		}
#endif /* DEBUG */

		kl10->cpu.szBlock      = ACBLKSIZE;
		kl10->cpu.nBlocks      = NACBLOCKS;
		kl10->cpu.nAccumlators = (NACBLOCKS * ACBLKSIZE);
		kl10->cpu.acBlocks =
			(int36 *)calloc(kl10->cpu.nAccumlators, sizeof(int36));

		// Link it to PDP-10 system device.
		p10sys            = (P10_SYSTEM *)newMap->sysDevice;
		kl10->cpu.System  = p10sys;
		kl10->cpu.Console = NULL;
		p10sys->Processor = (P10_CPU *)kl10;

		// Processor Initialization
		p10_InitMemory(1024 * 1024);
		p10_Initialize((P10_CPU *)kl10);
		kl10_Initialize(kl10);
		kl10_Reset(kl10);

		newMap->Device = kl10;
	}

	return kl10;	
}

int kl10_Reset(void *dptr)
{
	KL10_DEVICE *kl10 = (KL10_DEVICE *)dptr;
 	P10_CPU     *cpu  = (P10_CPU *)kl10;
	int         ac;

	// Clear all ammulators
	for (ac = 0; ac < cpu->nAccumlators; ac++)
		cpu->acBlocks[ac] = 0;

	return EMU_OK;
}

int kl10_Info(MAP_DEVICE *map, int argc, char **argv)
{
	KL10_DEVICE *kl10 = (KL10_DEVICE *)map->Device;

	printf("KL10 I/O Systems:\n\n");
	kx10_Info();

	return TS10_OK;
}

// For more information, look in pdp10/dev/kxio.c file.
extern DEVICE *kx10_Devices[];

DEVICE kl10a_Processor =
{
	KL10A_KEY,         // Key Name (Device Type)
	KL10_NAME,         // Emulator Name
	KL10_VERSION,      // Emulator Version
	kx10_Devices,      // Lisiting of Kx10 Devices
	DF_USE|DF_SYSMAP,  // Device Flags
	DT_PROCESSOR,      // Device Type

	NULL, NULL, NULL,

	kl10_Create,       // Create Routine
	NULL,              // Configure Routine
	NULL,              // Delete Routine
	kl10_Reset,        // Reset Routine
	NULL,              // Attach Routine
	NULL,              // Detach Routine
	kl10_Info,         // Info Routine
	NULL,              // Boot Routine
	NULL,              // Execute Routine
#ifdef DEBUG
	NULL,              // Debug Routine
#endif /* DEBUG */
};

#ifdef OPT_XADR
DEVICE kl10b_Processor =
{
	KL10B_KEY,         // Key Name (Device Type)
	KL10_NAME,         // Emulator Name
	KL10_VERSION,      // Emulator Version
	kx10_Devices,      // Lisiting of Kx10 Devices
	DF_USE|DF_SYSMAP,  // Device Flags
	DT_PROCESSOR,      // Device Type

	NULL, NULL, NULL,

	kl10_Create,       // Create Routine
	NULL,              // Configure Routine
	NULL,              // Delete Routine
	kl10_Reset,        // Reset Routine
	NULL,              // Attach Routine
	NULL,              // Detach Routine
	kl10_Info,         // Info Routine
	NULL,              // Boot Routine
	NULL,              // Execute Routine
#ifdef DEBUG
	NULL,              // Debug Routine
#endif /* DEBUG */
};
#endif /* OPT_XADR */
