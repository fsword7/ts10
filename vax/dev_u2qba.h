// dev_u2qba.h - Q22-Bus I/O Interface for MicroVAX II Series
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

#include "dev/uba/dec/defs.h"

#define QBA_KEY     "U2QBA"
#define QBA_NAME    "Q22-Bus Interface for MicroVAX II Series"
#define QBA_VERSION "v0.6 (Alpha)"

#define QBA_N_IPCR   4    // Number of IPC Registers
#define QBA_N_IOREG  4096 // Number of I/O Registers
#define QBA_N_MAP    8192 // Number of Map Registers

#define BYTE OP_BYTE
#define WORD OP_WORD
#define LONG OP_LONG

// 1F40 IPCR - Interprocess Communication Registers
// (Up to 4 IPCR registers - 1F40 to 1F48)
// Note: It supports up to 4 processors on the same Q22 bus.

#define IPCR_ADDR0 0x1F40 // IPCR Register #0
#define IPCR_ADDR1 0x1F42 // IPCR Register #1
#define IPCR_ADDR2 0x1F44 // IPCR Register #2
#define IPCR_ADDR3 0x1F46 // IPCR Register #3

#define IPCR_CSR   0777500 // Base address of IPCR registers
#define IPCR_VEC   0004    // Interrupt Vector
//#define IPCR_NREGS 4     // Number of Registers
#define IPCR_NREGS 1       // Number of Registers
#define IPCR_NVECS 1       // Number of Vectors
#define IPCR_IPL   UQ_BR4  // Interrupt Level
#define IPCR_INT   24      // IPL 14

#define IPCR_QPE   0x8000 // (R/?) DMA Q22-Bus Address Space Parity Error
#define IPCR_HLT   0x0100 // (R/?) Auxiliary Halt
#define IPCR_IE    0x0040 // (R/W) Doorbell Interrupt Enable
#define IPCR_LME   0x0020 // (R/W) Local Memory External Access Enable
#define IPCR_IRQ   0x0001 // (R/C) Doorbell Interrupt Request

#define IPCR_WMASK 0x0060 // Write Mask
#define IPCR_CMASK 0x0001 // Clear Mask
#define IPCR_ZMASK 0x7E9E // MBZ Mask

// Map Registers

#define MAP_V     0x80000000 // Valid bit
#define MAP_ADR   0x00007FFF // Page Frame Number (512 bytes a page)
#define MAP_WMASK 0x80007FFF // Write Mask
#define MAP_N_OFF 9
#define MAP_OFF   ((1u << MAP_N_OFF) - 1)
#define MAP_PAGSZ (1u << MAP_N_OFF)


typedef struct u2qba_Device U2QBA_IO;
typedef struct u2qba_Device QBA_DEVICE;

struct u2qba_Device {
	char        *devName;    // Device Name (QBA:)
	char        *keyName;    // Key (Device Type) Name
	char        *emuName;    // Emulator Name
	char        *emuVersion; // Emulator Version
	int         idDevice;    // Device Identification

	VAX_SYSTEM  *System;     // VAX System
	VAX_CPU     *Processor;  // VAX Processor
	UQ_CALL     *Callback;   // Callback Functions

	MAP_IO      ioIPCR;      // Interprocessor Control Register

	// I/O Page Table
	MAP_IO      *ioMap[QBA_N_IOREG];
	MAP_IO      *ioList;

	// Interrupt for IPCR registers
	uint32      intLevel;   // Interrupt Level (IPL)
	uint32      intMask;    // Interrupt Level Mask
	uint32      intVector;  // Interrupt Vector

	// Interrupt Priority Levels
	UQ_IPL      iplList[UQ_HLVL];

	// Q22 Bus Registers
	uint16      ipcReg[QBA_N_IPCR];
	uint32      mapRegs[QBA_N_MAP];
};
