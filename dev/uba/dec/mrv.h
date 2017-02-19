// mrv.h - MRV11 Boot ROM Device Definitions
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

#define MRV_KEY     "MRV11"
#define MRV_NAME    "Boot ROM Device"
#define MRV_VERSION "v0.7 (Alpha)"

#include "dec/defs.h"

// BDV11/KDF11 Unibus/QBus I/O Address:
//
// 765xxx ROM - (R)   ROM Page 1 Area (256 words window)
// 773xxx ROM - (R)   ROM Page 0 Area (256 words window)
// 777520 CSR - (R/W) Control/Status Register
// 777522 PCR - (R/W) Page Control Register
// 777524 CR  - (R)   Configuration Register
// 777524 DR  - (W)   Display Register

#define nCSR  0  // Control/Status Register
#define nPCR  1  // Page Control Register
#define nCR   2  // Configuration Register
#define nDR   2  // Display Register

#define CSR  mrv->csr
#define PCR  mrv->pcr
#define CR   mrv->cr
#define DR   mrv->dr

// 777520 - CSR (Control/Status Register)
#define CSR_BBRE    0100000 // (R/W) Battery Backup Reboot Enable
#define CSR_PULSE   0040000 // (R/W) Reboot Pulse
#define CSR_FRCLCIE 0020000 // (R/W) Force Line Clock Interrupt Enable
#define CSR_DISLKS  0010000 // (R/W) Disable Line Clock Status Register
#define CSR_CLKSEL  0006000 // (R/W) Clock Select
#define CSR_ENBHOB  0001000 // (R/W) Enable Halt-on-Break
#define CSR_SAMODE  0000400 // (R/W) Stand-alone Mode
#define CSR_DIS73   0000200 // (R/W) Disable 773000 ROM page
#define CSR_DIS65   0000100 // (R/W) Disable 765000 ROM page
#define CSR_ROM3    0000040 // (R/W) ROM Socket #3 Enable
#define CSR_WE3     0000020 // (R/W) ROM Socket #3 Write Enable
#define CSR_PMG     0000007 // (R/W) Processor Mastership Grant Control

#define CSR_RW      0177767 // Read/Write Accessible Mask

// 777522 - PCR (Page Control Register)
#define PCR_ROM73   0077000 // (R/W) Page Address at 773000 page.
#define PCR_ROM65   0000176 // (R/W) Page Address at 765000 page.

#define PCR_RW      (PCR_ROM73|PCR_ROM65)

// 777524 - CR (Configuration Register)
// 777524 - DR (Display Register)

#define MRV_CSRADR  0777520    // CSR Device Address
#define MRV_NREGS   3          // Number of Registers
#define ROM65_PAGE  0765000    // ROM Page 1 Window Address
#define ROM65_WSIZE 256        // Window Size
#define ROM73_PAGE  0773000    // ROM Page 0 Window Address
#define ROM73_WSIZE 256        // Window Size
#define ROM_PAGE    0777       // Offset Address Field
#define ROM_SIZE    (1u << 16) // Maximum 64K ROM Size

typedef struct mrv_Device MRV_ROM;

struct mrv_Device {
	UNIT    Unit;         // Unit Header Information
	void    *System;      // System device
	void    *Device;      // Unibus/QBus Interface
	UQ_CALL *Callback;    // Callback functions

	// I/O Map Area
	MAP_IO  ioMap;        // MRV11 Register Area
	MAP_IO  ioPage65;     // ROM Window Area
	MAP_IO  ioPage73;     // ROM Window Area

	// Unibus/Qbus Registers
	uint16  csr;          // Control/Status Register
	uint16  pcr;          // Page Control Register
	uint16  cr;           // Configuration Register
	uint16  dr;           // Display Register

	// Internal Registers
	uint32  Page65;        // ROM Area #1
	uint32  Page73;        // ROM Area #2

	// ROM Image Information
	char    *romFile;     // ROM Filename
	uint8   *romImage;    // ROM Image
	uint32  romSize;      // ROM Size
};
