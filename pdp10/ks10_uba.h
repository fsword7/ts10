// uba.h - KS10 Processor: UBA (Unibus) Inferface Definitions
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

#include "dev/uba/dec/defs.h"

#define UBA_KEY        "UBA"
#define UBA_NAME       "Unibus Interface"
#define UBA_VERSION    "v0.1 (Pre-Alpha)"

#define UBA_MAX       4       // Numbers of Unibus adapters
#define IO_NREGS      4096

// I/O Addresses for Unibus registers
#define UBA_BASE  0760000 // Beginning of I/O Page Map
#define UBA_SADDR 0763000 // Start of internal registers
#define UBA_EADDR 0763101 // End of internal registers
#define MAP_ADDR  0763000 // Map Registers
#define MAP_MASK  0000077 //   Address Mask
#define MAP_NREG  32      //   Number of Registers
#define SR_ADDR   0763100 // Status Register
#define SR_NREG   1       //   Number of Registers
#define MR_ADDR   0763101 // Maintenance Register
#define MR_NREG   1       //   Number of Registers

// I/O Address 7630xx - Paging Map Registers

// For write access (18-bit registers)
#define MAPW_FLAGS 0740000 // Flags Mask
#define MAPW_FRPW  0400000 // Force Read/Pause/Write
#define MAPW_EN16  0200000 // Disable upper 2 bits on xfers
#define MAPW_FME   0100000 // Fast Mode Enable
#define MAPW_VALID 0040000 // Page is valid
#define MAPW_PAGE  0003777 // Page Number

// For read access (36-bit registers)
#define MAP_RAMP   020000000000 // RAM Parity
#define MAP_FRPW   010000000000 // Force Read/Pause/Write
#define MAP_EN16   004000000000 // Disable upper 2 bits on xfers
#define MAP_FME    002000000000 // Fast Mode Enable
#define MAP_VALID  001000000000 // Page is valid
#define MAP_RPV    000400000000 // Paging RAM Parity - valid
#define MAP_PAGE   000003777000 // Page Number

// I/O Address 763100 - Status Register

#define SR_TIM    0400000 // (R/C) Unibus Arbitrator Timeout
#define SR_BAD    0200000 // (R/C) Bad memory data on NPR transfer
#define SR_PAR    0100000 // (R/C) KS10 Bus Parity Error
#define SR_NED    0040000 // (R/C) CPU Addressed - Non-existant device
#define SR_INTH   0004000 // (R)   Interrupt Request on BR6/BR7
#define SR_INTL   0002000 // (R)   Interrupt Request on BR4/BR5
#define SR_INT    0006000 // (R)   Interrupt Requests on BR4-BR7
#define SR_PWRL   0001000 // (R/C) AC/DC Low - Clear on write access
#define SR_DXFR   0000200 // (R/W) Disable transfer on bad data
#define SR_UINIT  0000100 // (W)   Issue Unibus Initization
#define SR_PIH    0000070 // (R/W) PI Level of BR6,BR7
#define SR_PIL    0000007 // (R/W) PI Level of BR4,BR5

#define SR_W1C    0741000 // (C)   Clear Mask
#define SR_RW     0000277 // (W)   Write Mask

// I/O Address 763101 - Maintenance Register

#define MR_SMB    0000002 // (W)   Spare Maintenance Bit
#define MR_CNA    0000001 // (W)   Change NPR Address

#define MR_WRMASK 0000003 // (W)   Write Mask
#define MR_RDMASK 0000000 // (R)   Read Mask

// PDP-11 Style Interrupt System

#define INT_BR7  0x000000FF // IPL<7:0>
#define INT_BR6  0x0000FF00 // IPL<15:8>
#define INT_BR5  0x00FF0000 // IPL<23:16>
#define INT_BR4  0x3F000000 // IPL<30:24>

#define INT_BR67 (INT_BR6|INT_BR7)
#define INT_BR45 (INT_BR4|INT_BR5)

// Definitions for UBA Map and Configure routine
#define UBA_CHECK     0
#define UBA_DELETE    -1
#define UBA_INTERNAL  -1

#define EMU_UBA_BADADDR 1 // Reserved I/O Address (Already Assigned)
#define EMU_UBA_FULL    2
#define EMU_UBA_RSVDIPL 3 // Reserved IPL Slot (Already Assigned)

// UBA Adaptor Flags
#define UIF_EXIST   0x80000000 // Device is existing

typedef struct ks10uba_Interface KS10UBA_IF;
typedef struct ks10uba_Device    KS10UBA_DEVICE;

struct ks10uba_Interface {
	UNIT Unit; // Unit Header

	uint32         idSlot;
	uint32         Flags;
	KS10UBA_DEVICE *uba;

	// Processor/System Device
	void     *System;
	void     *Processor;
	UQ_CALL  *Callback;

	// I/O Page Table
	MAP_IO *ioMap[IO_NREGS];
	MAP_IO *ioList;

	// Internal Registers
	MAP_IO mapMap; // Map Registers
	MAP_IO srMap;  // Status Register
	MAP_IO mrMap;  // Maintenance Register
	
	int32  map[0100];     // Map registers -        7630xx
	int18  sr;            // Status Register -      763100
	int18  mr;            // Maintenance Register - 763101
	UQ_IPL intList[UQ_HLVL]; // Pending I/O Interrupts List
	int32  intRequests;   // Pending Interrupt Requests
	int32  intIPL;        // IPL Levels
	int32  intVector[32]; // Interrupt Vectors
};

struct ks10uba_Device {
	// Descriptions
	char     *devName;     // Device Name
	char     *keyName;     // Device Type (Key) Name
	char     *emuName;     // Emulator Name
	char     *emuVersion;  // Emulator Version

	// Processor/System Device
	void     *System;
	void     *Processor;
	UQ_CALL  *Callback;

	// UBA Interface Slots
	uint32     nSlots;
	KS10UBA_IF *Slots;
};

// Prototype Definitions
int32  uba_GetVector(KS10UBA_DEVICE *, int32, int32 *);
uint32 ks10uba_ReadIO(KS10UBA_DEVICE *, int30, int);
void   ks10uba_WriteIO(KS10UBA_DEVICE *, int30, uint32, int);
