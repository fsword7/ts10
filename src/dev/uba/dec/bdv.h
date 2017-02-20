// bdv.h - BDV11/KDF11 Boot ROM Device Definitions
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

#define BDV_KEY     "BDV11"
#define KDF_KEY     "KDF11"
#define BDV_NAME    "Boot ROM Device"
#define BDV_VERSION "v0.7 (Alpha)"

#include "dec/defs.h"

// BDV11/KDF11 Unibus/QBus I/O Address:
//
// 773xxx ROM - (R)   ROM Area (256 bytes window)
// 777520 PCR - (R/W) Page Control Register  (Write Only on KDF11 device)
// 777522 SPR - (R/W) Scratch Pad Register
// 777524 OSR - (R)   Option Select Register (8b on KDF11, 12b on BDV11)
// 777524 DSP - (W)   Display LEDs (4b)

#define nPCR  0  // Page Control Register
#define nSPR  1  // Scratch Pad Register
#define nOSR  2  // Option Select Register
#define nDSP  2  // Display LED Register

#define PCR  bdv->pcr
#define SPR  bdv->spr
#define OSR  bdv->osr
#define DSP  bdv->dsp

#define BDV_CSRADR 0777520   // CSR Device Address
#define BDV_NREGS  3         // Number of Registers

#define ROM_PAGE0  0773000   // ROM Page 0 Address
#define ROM_PAGE1  0773400   // ROM Page 1 Address
#define ROM_WSIZE  128       // 256-byte Window Size
#define PCR_MASK   0377      // Page Offset Address

// BDV11/KDF11 Device Flags
#define FLG_KDF11   0x00000001  // KDF11 Mode
#define FLG_BDV11   0x00000000  // BDV11 Mode

// KDF11 Settings
#define KDF_ROMSZ   16384     // ROM Size
#define KDF_PCRMASK 0037477   // PCR Mask
#define KDF_PCRSZ   6         // PCR Width
#define KDF_OSRSZ   8         // OSR Width

// BDV11 Settings
#define BDV_ROMSZ   65536     // ROM Size
#define BDV_PCRMASK 0177777   // PCR Mask
#define BDV_PCRSZ   8         // PCR Width
#define BDV_OSRSZ   12        // OSR Width

typedef struct bdv_Device BDV_ROM;

struct bdv_Device {
	UNIT    Unit;         // Unit Header Information
	void    *System;      // System device
	void    *Device;      // Unibus/QBus Interface
	UQ_CALL *Callback;    // Callback functions

	// ROM Device Information
	uint32  Flags;        // BDV11/KDF11 ROM Flags
	uint32  maxSize;      // ROM Size
	uint32  pcrMask;      // PCR Mask
	uint32  pcrWidth;     // PCR Width
	uint32  osrWidth;     // OSR Width

	// I/O Map Area
	MAP_IO  ioMap;        // BDV11/KDF11 Register Area
	MAP_IO  ioPage0;      // ROM Page 0 Window Area
	MAP_IO  ioPage1;      // ROM Page 1 Window Area

	// Unibus/Qbus Registers
	uint16  pcr;          // Page Control Register
	uint16  spr;          // Scratch Pad Register
	uint16  osr;          // Option Select Register
	uint16  dsp;          // Display Register

	// ROM Page Addresses
	uint32  Page0;        // Page 0 (173000 to 173377)
	uint32  Page1;        // Page 1 (173400 to 173777)

	// ROM Image Information
	char    *romFile;     // ROM Filename
	uint8   *romImage;    // ROM Image
	uint32  romSize;      // ROM Size
};
