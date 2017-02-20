// dev_mba.h - KA700 Processor Series, Massbus Interface
//
// Copyright (c) 2003, Timothy M. Stark
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
//
// -------------------------------------------------------------------------
//
// Modification History:
//
// XX/XX/XX  TMS  Comments Here
//
// -------------------------------------------------------------------------

#include "dev/mba/mba.h"
#include "vax/nexus.h"

// VAX Massbus Map (Nexus Slot)
//
//      +------------------------------------------------+
// 0000 | Massbus Controller Registers                   |
// 03FF | (First 6 32-bit registers)                     |
//      +------------------------------------------------+
// 0400 | Massbus Drive #0 (32 registers or 128 bytes)   |
//      +------------------------------------------------+
// 0480 | Massbus Drive #1                               |
//      +------------------------------------------------+
// 0500 | Massbus Drive #2                               |
//      +------------------------------------------------+
// 0580 | Massbus Drive #3                               |
//      +------------------------------------------------+
// 0600 | Massbus Drive #4                               |
//      +------------------------------------------------+
// 0680 | Massbus Drive #5                               |
//      +------------------------------------------------+
// 0700 | Massbus Drive #6                               |
//      +------------------------------------------------+
// 0780 | Massbus Drive #7                               |
//      +------------------------------------------------+
// 0800 | I/O Space Map                                  |
//      +------------------------------------------------+
// 0C00 | Reserved Area                                  |
// 1FFF |                                                |
//      +------------------------------------------------+

// 0000 MBCSR - Configuration Register (NEXUS CSR Register)

#define MBCSR_TYPE 0xFF // Nexus Type

// 0004 MBCR  - Control Register

#define MBCR_IE    0x04 // Enable MBA Interrupts
#define MBCR_INIT  0x01 // Initialize MBA Interface

// 0008 MBSR  - Status Register

#define MBSR_DTBUSY  0x80000000 // Data Transfer Busy
#define MBSR_NRCONF  0x40000000 // No Response Confirmation
#define MBSR_CRD     0x20000000 // Corrected Read Data
#define MBSR_CBHUNG  0x00800000 // Control Bus Hung
#define MBSR_PGE     0x00080000 // Programming Error
#define MBSR_NED     0x00040000 // Non-Existant Drive
#define MBSR_MCPE    0x00020000 // Massbus Control Parity Error
#define MBSR_ATTN    0x00010000 // Attention from Massbus
#define MBSR_SPE     0x00004000 // SILO Parity Error
#define MBSR_DTCMP   0x00002000 // Data Transfer Completed
#define MBSR_DTABT   0x00001000 // Data Transfer Aborted
#define MBSR_DLT     0x00000800 // Data Late
#define MBSR_WCKUP   0x00000400 // Write Check Upper
#define MBSR_WCKLWR  0x00000200 // Write Check Lower
#define MBSR_MXF     0x00000100 // Miss Transfer Error
#define MBSR_MBEXC   0x00000080 // Massbus Exception
#define MBSR_MDPE    0x00000040 // Massbus Data Parity Error
#define MBSR_MAPPE   0x00000020 // Page Frame Map Parity Error
#define MBSR_INVMAP  0x00000010 // Invalid Map
#define MBSR_ERRCONF 0x00000008 // Error Confirmation
#define MBSR_RDS     0x00000004 // Read Data Substitute
#define MBSR_ISTIMO  0x00000002 // Interface Sequence Timeout
#define MBSR_RDTIMO  0x00000001 // Read Data Timeout

// 000A MBVAR - Virtual Address Register
// 000C MBBCR - Byte Count Register
// 0010 MBDR  - ??

typedef struct {
	char  *Name;        // Device Name
	int16 Type;         // Device Type - Massbus Adaptor
	MBA_DEVICE *Device; // Massbus Drives

	// Massbus Controller Registers
	uint32 cfgr;        // Nexus/Configuration Register
	uint32 cr;          // Control Register
	uint32 sr;          // Status Register
	uint32 var;         // Virtual Address Register
	uint32 bcr;         // Byte Count Register
	uint32 dr;          // Diagnostics Register ??
	uint32 map[512];    // Map Registers
} VAXMBA_DEVICE;
