// ka780.h - VAX-11/780 Series System Configuration Definitions
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

// VAX-11/780 Memory Map
//
//            +-----------------------------------------------------+
// 0000 0000: |                  Installed Memory                   |
//     :      |- - - - - - - - - - - - - - - - - - - - - - - - - - -|
// 1FFF FFFF: |                 Up to 512MB Memory                  |
//            +-----------------------------------------------------+   
// 2000 0000: |                  TR0 Adapter Space                  |
//            +-----------------------------------------------------+
// 2000 2000: |                  TR1 Adapter Space                  |
//            |                          :                          |
// 2001 E000: |                  TR15 Adapter Space                 |
//            +-----------------------------------------------------+
// 2002 0000: |                                                     |
//     :      |                      Reserved                       |
// 200F FFFF: |                                                     |
//            +-----------------------------------------------------+
// 2010 0000: |                UNIBUS 0 Address Space               |
//            +-----------------------------------------------------+
// 2014 0000: |                UNIBUS 1 Address Space               |
//            +-----------------------------------------------------+
// 2018 0000: |                UNIBUS 2 Address Space               |
//            +-----------------------------------------------------+
// 201C 0000: |                UNIBUS 3 Address Space               |
//            +-----------------------------------------------------+
// 2020 0000: |                                                     |
//     :      |                      Reserved                       |
// 3FFF FFFF: |                                                     |
//            +-----------------------------------------------------+

#include "vax/defs.h"
#include "vax/nexus.h"

#define KA780_KEY     "KA780"
#define KA785_KEY     "KA785"

#define KA78X_NAME    "VAX-11/780 Emulator"
#define KA78X_VERSION "v0.1 (Pre-Alpha)"

// System ID Longword
//
//  31   24 23 22          15 14   12 11             0
// +-------+--+--------------+-------+----------------+
// |   1   |  |   ECO Level  | Plant |  Serial Number |
// +-------+--+--------------+-------+----------------+
//          |
//          +- 0 = VAX-11/780
//             1 = VAX-11/785

#define KA780_ECO      0
#define KA780_PLANT    0
#define KA780_SN       1

#define KA780_SYSID    0x01000000
#define KA780_785      0x00800000

#define KA780_P_ECO    15
#define KA780_P_PLANT  12

// Privileged Register for VAX-11/780 system
#define nKSP    0x00 // Kernel Stack Pointer
#define nESP    0x01 // Executive Stack Pointer
#define nSSP    0x02 // Supervisor Stack Pointer
#define nUSP    0x03 // User Stack Pointer
#define nISP    0x04 // Interrupt Stack Pointer

#define nP0BR   0x08 // P0 Base Register
#define nP0LR   0x09 // P0 Length Register
#define nP1BR   0x0A // P1 Base Register
#define nP1LR   0x0B // P1 Length Register
#define nSBR    0x0C // System Base Register
#define nSLR    0x0D // System Length Register

#define nPCBB   0x10 // Process Control Block Base
#define nSCBB   0x11 // System Control Block Base
#define nIPL    0x12 // Interrupt Priority Level
#define nASTLVL 0x13 // AST Level
#define nSIRR   0x14 // Software Interrupt Request
#define nSISR   0x15 // Software Interrupt Summary

#define nRXCS   0x20 // Console Receive Control and Status Register
#define nRXDB   0x21 // Console Receive Data Buffer Register
#define nTXCS   0x22 // Console Transmit Control and Status Register
#define nTXDB   0x23 // Console Transmit Data Buffer Register

#define nMAPEN  0x38 // Map Enable
#define nTBIA   0x39 // TB Clear All Entries
#define nTBIS   0x3A // TB Clear Single Entry
#define nSID    0x3E // System Identification
#define nTBCHK  0x3F // TB Check

#define SCBB_MBZ   0x000001FF // Must be zeros.
#define BR_MASK    0xFFFFFFFC // xxBR Mask
#define LR_MASK    0x003FFFFF // xxLR Mask

// TODR - Time of Day Register
// Note: Increment by one each 10 msec.

#define TODR_BASE (1u << 28)
#define TODR_SEC  100

// ICCS - Interval Clock Control Status Registers

#define ICCS_ERR    0x80000000  // Timer Error
#define ICCS_DONE   0x00000080  // Ready/Done (Interrupt)
#define ICCS_IE     0x00000040  // Interrupt Enable
#define ICCS_STEP   0x00000020  // Single Step
#define ICCS_XFER   0x00000010  // Transfer
#define ICCS_RUN    0x00000001  // Run

#define ICCS_CMASK  0x80000001  // Clear Mask
#define ICCS_WMASK  0x00000090  // Write Mask
#define ICCS_MBZ    0x7FFFFF0E  // Must Be Zeros

#define ICCS_SECOND 100         // Ticks each second
#define ICCS_TICK   10          // Tick each 10 microseconds

#define nICCS_INT   4
#define ICCS_INT    (1u << nICCS_INT)

// Aligned RAM Access
#define IN_RAM(addr) ((addr) < vax->sizeRAM)
#define BMEM(addr)   ((uint8 *)vax->RAM)[addr]
#define WMEM(addr)   ((uint16 *)vax->RAM)[addr]
#define LMEM(addr)   ((uint32 *)vax->RAM)[addr]

typedef struct {
	char *devName;     // Device Name
	char *keyName;     // Key (Device Type) Name
	char *emuName;     // Emulator Name
	char *emuVersion;  // Emulator Version

	uint32 sAddr;      // Starting Address
} SBI780_DEVICE;

typedef struct {
	// Generic processor header.
	VAX_CPU   cpu;

	// System Timer
	CLK_QUEUE ClockTimer; // Clock Timer (10 msecs)
	uint32    TickCount;

	// SBI Interface (Nexus Slots)
	SBI780_DEVICE *sbi[NEX780_NSBI];
} KA780_DEVICE;
