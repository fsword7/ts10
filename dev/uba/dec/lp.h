// dec_lp.h - LP11/LS11/LA11 Line Printer Emulation
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

#include "dec/defs.h"

#define LP_KEY     "LP11"
#define LS_KEY     "LS11"
#define LA_KEY     "LA11"

#define LP_NAME    "LP11 Series Line Printer"
#define LP_VERSION "v0.8a (Alpha)"

#define LP_IOADDR  0777514  // Default CSR Address
#define LP_NREGS   2        // Number of Registers
#define LP_NVECS   1        // Number of Vectors
#define LP_IPL     UQ_BR4   // Default Interrupt Level
#define LP_VEC     0200     // Default Vector Address
#define LP_TIMER   0

#ifdef DEBUG
static cchar *regName[] = { "LPCSR", "LPDB" };
#endif /* DEBUG */

#define nLPCSR   0
#define nLPDB    1

// Control and Status Register
#define CSR_ERR   0100000  // (R)   Error
#define CSR_DONE  0000200  // (R)   Ready/Done
#define CSR_IE    0000100  // (R/W) Interrupt Enable

#define CSR_RW    (CSR_IE) // Read/Write Access

// Data Buffer Register
#define DB_DATA   0000377  // (R/W) Data Buffer

typedef struct lp_Device LP_DEVICE;

struct lp_Device {
	// Device Identification
	char      *devName;
	char      *keyName;
	char      *emuName;
	char      *emuVersion;

	// LP11 Registers
	uint16    lpcsr;      // Control and Status Register
	uint16    lpdb;       // Data Buffer Register

	// File Descriptor
	int       File;       // File Descriptor
	char      *fileName;  // Filename

	// Device Configuration
	uint32    csrAddr;
	MAP_IO    ioMap;
	CLK_QUEUE svcTimer;
	void      *System;
	void      *Device;
	UQ_CALL   *Callback;
};

