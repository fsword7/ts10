// tcu150.c - TCU routines.
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

#include <time.h>
#include "emu/defs.h"
#include "dec/defs.h"

#define TCU_KEY     "TCU150"
#define TCU_NAME    "Timing Control Unit"
#define TCU_VERSION "v0.7 (Alpha)"

// Article posting from alt.sys.pdp10 by Bob Supnik.
//
// From KSSER.MAC,
//
// Base address = 760770 on Unibus adapter 3.
//
// Register 0
//   15:9   years since 1900 (0-127)
//    8:5   month (1-12)
//    4:0   day (1-31)
//
// Register 2
//   12:8   hour (0-24, with midnight represented as 24:00,
//          but the KS10 will accept 00:00)
//    5:0   minute (0-59)
//
// Register 4
//    5:0   second (0-59)
//
// I have no idea what happens if you write, I can't find any references
// to that.  Implementation is easy, a tm struct has all the fields one
// needs.
//
// Of course, this gives the KS10 a year 2028 problem...

// TCU150 Registers on 760770.
#define TCU_DATE  0 // (R) Date Entry (Year/Month/Day)
#define TCU_TIME1 1 // (R) Time Entry (Hour/Minute)
#define TCU_TIME2 2 // (R) Time Entry (Second)
#define TCU_SR    3 // (R) Status Register

// 760770 - Date Register
#define TCU_DATE_YEAR    0177000 // (R) Year   (0-127)
#define TCU_DATE_MONTH   0000740 // (R) Month  (1-12)
#define TCU_DATE_DAY     0000037 // (R) Day    (1-31)

// 760772 - Time 1 Register
#define TCU_TIME1_HOUR   0017400 // (R) Hour   (0-24)
#define TCU_TIME1_MINUTE 0000077 // (R) Minute (0-59)

// 760774 - Time 2 Register
#define TCU_TIME2_SECOND 0000077 // (R) Second (0-59)

// 760776 - Status Register
#define TCU_SR_READY     0000200 // (R) Ready

#define TCU_CSRADDR      0760770 // Default CSR Address
#define TCU_NREGS        4       // Number of Registers
#define TCU_INT          0       // Interrupt Level (Not Used)
#define TCU_VECTOR       0       // Vector Address  (Not Used)

#ifdef DEBUG
static cchar *regNames[] = {
	"TCU_DATE",
	"TCU_TIME1",
	"TCU_TIME2",
	"TCU_SR",
};
#endif /* DEBUG */

typedef struct tcu_Device TCU_DEVICE;
struct tcu_Device {
	UNIT    Unit; // Unit Header
	void    *Device;
	void    *System;
	UQ_CALL *Callback;
	MAP_IO  ioMap;
};

// ******************************************************************

int tcu_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	TCU_DEVICE *tcu = (TCU_DEVICE *)dptr;
	int32      reg  = pAddr & 07;
	struct tm  *tm;
	time_t     now;

	// Get time clock data
	now = time(NULL);
	tm  = localtime(&now);

	switch (reg >> 1) {
		case TCU_DATE:
			// Return current year/month/day.
			*data = ((tm->tm_year & 0177) << 9) |
			        (((tm->tm_mon + 1) & 017)  << 5) |
			        (tm->tm_mday & 037);
			break;

		case TCU_TIME1:
			// Return current hours and minutes
			*data = ((tm->tm_hour & 037) << 8) |
			        (tm->tm_min & 077);
			break;

		case TCU_TIME2:
			// Return current seconds.
			*data = tm->tm_sec & 077;
			break;

		case TCU_SR:
			// Always tell it is ready.
			*data = TCU_SR_READY;
			break;

#ifdef DEBUG
		default:
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) Illegal Register: %02o (%06o)\n",
					tcu->Unit.devName, reg, pAddr);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%02o) => %06o\n", tcu->Unit.devName,
			regNames[reg >> 1], reg, *data);
#endif /* DEBUG */

	return UQ_OK;
}

int tcu_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	TCU_DEVICE *tcu = (TCU_DEVICE *)dptr;
	int32      reg  = pAddr & 07;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) Illegal Register: %02o (%06o)\n",
			tcu->Unit.devName, reg, pAddr);
#endif /* DEBUG */

	return UQ_OK;
}

// ************************************************************

void *tcu_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	TCU_DEVICE *tcu = NULL;
	MAP_IO     *io;

	if (tcu = (TCU_DEVICE *)calloc(1, sizeof(TCU_DEVICE))) {
		// Set up its descriptions on this new device.
		tcu->Unit.devName      = newMap->devName;
		tcu->Unit.keyName      = newMap->keyName;
		tcu->Unit.emuName      = newMap->emuName;
		tcu->Unit.emuVersion   = newMap->emuVersion;

		// Link it to its parent device.
		tcu->Device       = newMap->devParent->Device;
		tcu->System       = newMap->devParent->sysDevice;
		tcu->Callback     = newMap->devParent->Callback;

		// Set up an I/O space.
		io                = &tcu->ioMap;
		io->devName       = tcu->Unit.devName;
		io->keyName       = tcu->Unit.keyName;
		io->emuName       = tcu->Unit.emuName;
		io->emuVersion    = tcu->Unit.emuVersion;
		io->Device        = tcu;
		io->csrAddr       = TCU_CSRADDR;
		io->nRegs         = TCU_NREGS;
		io->intIPL        = TCU_INT;
		io->intVector[0]  = TCU_VECTOR;
		io->ReadIO        = tcu_ReadIO;
		io->WriteIO       = tcu_WriteIO;

		// Assign that registers to Unibus/Q22's I/O space.
		tcu->Callback->SetMap(tcu->Device, io);

		// Finally, link it to this mapping device and return.
		newMap->Device = tcu;
	}

	return tcu;
}

DEVICE tcu_Device =
{
	// Descriptions, etc.
	TCU_KEY,               // Key Name
	TCU_NAME,              // Emulator Name
	TCU_VERSION,           // Emulator Version
	NULL,                  // Listing of slave devices
	DF_SYSMAP,             // Device Flags
	DT_CLOCK,              // Device Type

	// Device Commands (Not Used)
	NULL,                  // Commands
	NULL,                  // Set Commands
	NULL,                  // Show Commands

	// Function Calls
	tcu_Create,            // Create Routine
	NULL,                  // Configure Routine
	NULL,                  // Delete Routine
	NULL,                  // Reset Routine
	NULL,                  // Attach Routine
	NULL,                  // Detach Routine
	NULL,                  // Info Routine
	NULL,                  // Boot Routine
	NULL,                  // Execute Routine
#ifdef DEBUG
	NULL,                  // Debug Routine
#endif /* DEBUG */
};

