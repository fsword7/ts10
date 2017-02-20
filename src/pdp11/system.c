// system.c - PDP-11 Emulator System
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

void *p11_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	P11_SYSTEM *p11 = NULL;

	if (p11 = (P11_SYSTEM *)calloc(1, sizeof(P11_SYSTEM))) {
		// Set up its descriptions on new device.
		p11->Unit.devName    = newMap->devName;
		p11->Unit.keyName    = newMap->keyName;
		p11->Unit.emuName    = newMap->emuName;
		p11->Unit.emuVersion = newMap->emuVersion;

		newMap->Device    = p11;
		newMap->sysDevice = p11;
		newMap->sysMap    = newMap;

#ifdef DEBUG
		// Initialize PDP-11 disassembler
		p11_InitDisasm(p11);
#endif /* DEBUG */
	}
	return p11;
}

int p11_ExecSystem(MAP_DEVICE *map)
{
	P11_SYSTEM *p11sys = (P11_SYSTEM *)map->Device;

	// Run PDP-11 Processor.
	p11_Execute(p11sys->cpu);

	return P11_OK;
}

extern DEVICE f11_System;
extern DEVICE j11_System;

DEVICE *p11_Systems[] =
{
	&f11_System, // KDF11 Series System
	&j11_System, // KDJ11 Series System
	NULL         // Terminator
};

DEVICE p11_System =
{
	PDP11_KEY,         // Type Name
	PDP11_NAME,        // Emulator Name
	PDP11_VERSION,     // Emulator Version
	p11_Systems,       // System Listings
	DF_SELECT,         // Device Flags
	DT_SYSTEM,         // Device Type

#if 0
	p10_Commands,      // Commands
	p10_SetCommands,   // Set Commands
	p10_ShowCommands,  // Show Commands
#endif
	NULL, NULL, NULL,

	p11_Create,        // Create Routine
	NULL,              // Configure Routine
	NULL,              // Delete Routine
	NULL,              // Reset Routine
	NULL,              // Attach Routine (Not Used)
	NULL,              // Detach Routine (Not Used)
	NULL,              // Info Routine
	NULL,              // Boot Routine
	p11_ExecSystem,    // Execute Routine
#ifdef DEBUG
	NULL,              // Debug Routine
#endif /* DEBUG */
};
