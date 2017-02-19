// system.c - VAX System Configurations
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

#include "vax/defs.h"

#define VAX_KEY     "VAX"
#define VAX_NAME    "VAX-11 Series Emulator"
#define VAX_VERSION "v0.8.2 (Late Alpha)"


// VAX System Table
extern DEVICE vax_System_KA630;
extern DEVICE vax_System_KA650;
extern DEVICE vax_System_KA655;
extern DEVICE vax_System_KA780;
extern DEVICE vax_System_KA785;

// VAX commands in commands.c file.
extern COMMAND vax_Commands[];
extern COMMAND vax_SetCommands[];
extern COMMAND vax_ShowCommands[];

DEVICE *vax_Systems[] =
{
	&vax_System_KA630,  // MicroVAX II Series
	&vax_System_KA650,  // CVAX Series
	&vax_System_KA655,  // MicroVAX 3800/3900 Series
	&vax_System_KA780,  // VAX-11/780 Series
	&vax_System_KA785,  // VAX-11/780 Series
	NULL
};

DEVICE vax_System; // Look at bottom of this file.

// Load system executable file into VAX memory
int vax_LoadFile(VAX_CPU *vax, char *fn, uint32 sAddr, uint32 *eAddr)
{
	int exeFile;
	int idx;
	int st = 1;

	if ((exeFile = open(fn, O_RDONLY)) < 0)
		return EMU_OPENERR;

	for (idx = sAddr; st > 0; idx += 512) {
		st = read(exeFile, &vax->RAM[idx], 512);
	}
	if (eAddr) *eAddr = idx;

	close(exeFile);

	// Tell operator that file had been loaded.
	printf("File '%s' loaded from %08X to %08X\n", fn, sAddr, idx);

	return VAX_OK;
}

// Load ROM image into VAX memory
int vax_LoadROM(VAX_CPU *vax, char *fn, uint32 sAddr)
{
	struct stat st;
	int romFile;
	int baseAddr, lenAddr;
	int idx;

	if ((romFile = open(fn, O_RDONLY)) < 0)
		return EMU_OPENERR;

	// Set up starting address and ROM image size.
	fstat(romFile, &st);
	baseAddr = sAddr;
	lenAddr  = st.st_size;

	vax->ROM     = (uint8 *)malloc(lenAddr);
	vax->baseROM = baseAddr;
	vax->endROM  = (baseAddr + lenAddr) - 1;
	vax->sizeROM = lenAddr;
	vax->maskROM = lenAddr - 1;

	for (idx = 0; idx < lenAddr; idx += 32768)
		read(romFile, &vax->ROM[idx], 32768);

	close(romFile);

	PC = baseAddr;

	return VAX_OK;
}

// Load NVRAM image into VAX memory
int vax_LoadNVRAM(char *fn)
{
}

extern INSTRUCTION vax_Instruction[];
void vax_CheckInstructions(void)
{
	char outBuffer[80];
	int  done, total;
	int  idx;

	outBuffer[0] = 0;
	total = 0;
	done  = 0;

	printf("Following instructions that need to be implemented:\n");
	for (idx = 0; vax_Instruction[idx].Name; idx++) {
		total++;
		if (vax_Instruction[idx].Execute == NULL) {
			if (strlen(outBuffer) > 70) {
				outBuffer[strlen(outBuffer)] = '\0';
				printf("%s\n", outBuffer);
				outBuffer[0] = '\0';
			}

			if (outBuffer[0])
				strcat(outBuffer, ", ");
			strcat(outBuffer, vax_Instruction[idx].Name);
		} else
			done++;
	}

	if (outBuffer[0]) {
		outBuffer[strlen(outBuffer)] = '\0';
		printf("%s\n", outBuffer);
	}

	printf("\n%d of %d instructions had been implemented.\n", done, total);
}

// ****************************************************************

void *vax_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	VAX_SYSTEM *vax;

	if (vax = (VAX_SYSTEM *)calloc(1, sizeof(VAX_SYSTEM))) {
#ifdef DEBUG
		vax_InitDisasm();
#endif /* DEBUG */

		// Finally, set up its description and
		// link it to its mapping device.
		vax->devName       = newMap->devName;
		vax->keyName       = newMap->keyName;
		vax->emuName       = newMap->emuName;
		vax->emuVersion    = newMap->emuVersion;
		newMap->sysMap     = newMap;
		newMap->sysDevice  = vax;
		newMap->Device     = vax;
	}

	return vax;
}

DEVICE vax_System =
{
	VAX_KEY,      // System Device Name
	VAX_NAME,     // Emulator Name
	VAX_VERSION,  // Emulator Version
	vax_Systems,  // Listing of VAX Systems
	DF_SELECT,    // Device Flags
	DT_SYSTEM,    // Device Type

	// Command Table for VAX system
	vax_Commands,     // Commands
	vax_SetCommands,  // Set Commands
	vax_ShowCommands, // Show Commands

	vax_Create,       // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	NULL,             // Reset Routine
	NULL,             // Attach Routine (Not Used)
	NULL,             // Detach Routine (Not Used)
	NULL,             // Info Routine
	NULL,             // Boot Routine
	vax_Execute,      // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
