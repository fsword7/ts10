// dev_cqbic.h - CQBIC Q22-Bus Interface
//
// Copyright (c) 2001, Timothy M. Stark
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

// CQBIC Definitions

#include "dev/uba/dec/defs.h"

#define CQ_KEY       "CQBIC"
#define CQ_NAME      "CQBIC Q22-Bus Emulator"
#define CQ_VERSION   "v0.7 (Alpha)"

// IPC - Interprocessor Communication Register

// 1F40 IPCR - Interprocess Communication Registers
// (Up to 4 IPCR registers - 1F40 to 1F48)
// Note: It supports up to 4 processors on the same Q22 bus.

#define IPC_ADDR0 0x1F40 // IPCR Register #0
#define IPC_ADDR1 0x1F42 // IPCR Register #1
#define IPC_ADDR2 0x1F44 // IPCR Register #2
#define IPC_ADDR3 0x1F46 // IPCR Register #3

#define IPC_CSR   017500 // Base address of IPCR registers
#define IPC_NREGS 1      // Number of Registers
#define IPC_NVECS 1      // Number of Vectors
#define IPC_IPL   UQ_BR4 // Interrupt Level BR4
#define IPC_VEC   0x004  // Interrupt Vector

#define IPC_QME   0x8000 // (R/C) QBus Read NXM
#define IPC_INV   0x4000 // (R/?) CAM Invalid
#define IPC_HLT   0x0100 // (R/W) Auxiliary Halt
#define IPC_IE    0x0040 // (R/W) Doorbell Interrupt Enable
#define IPC_LME   0x0020 // (R/W) Local Memory External Access Enable
#define IPC_IRQ   0x0001 // (R/W) Doorbell Interrupt Request

#define IPC_W1C   (IPC_QME)
#define IPC_RW    (IPC_HLT|IPC_IE|IPC_LME|IPC_IRQ)
#define IPC_MASK  (IPC_RW|IPC_QME)

#define IO_NREGS  4096 // 16-bit I/O Registers

// Map Registers

#define MAP_VALID 0x80000000 // Valid Entry
#define MAP_PAGE  0x000FFFFF // Memory Page Address
#define MAP_WMASK (MAP_VALID|MAP_PAGE)

#define MAP_N_OFF 9
#define MAP_OFF   ((1u << MAP_N_OFF) - 1)

// CQBIC System Configuration Register

#define SCR_POK   0x00008000 // Power Ok
#define SCR_BHL   0x00004000 // BHALT Enable
#define SCR_AUX   0x00000400 // Aux Mode
#define SCR_DBO   0x0000000C // Offset
#define SCR_RW    (SCR_BHL | SCR_DBO)
#define SCR_MASK  (SCR_RW | SCR_POK | SCR_AUX)

// DSER - DMA System Error Register

#define DSER_BHL  0x00008000 // BHALT
#define DSER_DCN  0x00004000 // DC ~OK
#define DSER_MNX  0x00000080 // Master NXM
#define DSER_MPE  0x00000020 // Master Parity Error
#define DSER_SME  0x00000010 // Slave Memory Error
#define DSER_LST  0x00000008 // Lost Error
#define DSER_TMO  0x00000004 // No Grant
#define DSER_SNX  0x00000001 // Slave NXM

#define DSER_ERR  (DSER_MNX | DSER_MPE | DSER_TMO | DSER_SNX)
#define DSER_MASK 0x0000C0BD

// MEAR - Master Error Address Register

#define MEAR_MASK 0x00001FFF // QBus Page Address

// SEAR - Slave Error Address Register

#define SEAR_MASK 0x000FFFFF // Memory Address

// MBR - Map Base Register

#define MBR_MASK  0x1FFF8000 // 32KB-Aligned Map Base Address

// Register Index
#define nSCR   0
#define nDSER  1
#define nMEAR  2
#define nSEAR  3
#define nMBR   4

typedef struct cqDevice CQ_DEVICE;

struct cqDevice {
	char     *devName;     // Device Name
	char     *keyName;     // Device Type Name
	char     *emuName;     // Emulator Name
	char     *emuVersion;  // Emulator Version
	int32    idDevice;     // Device Identification

	VAX_SYSTEM *System;
	VAX_CPU    *Processor;
	UQ_CALL    *Callback;

	// CQBIC Registers
	uint32   scr;  // System Configuration Register
	uint32   dser; // DMA System Error Register
	uint32   mear; // Master Memory Error Register
	uint32   sear; // Slave Memory Error Register
	uint32   mbr;  // Map Base Register

	// IPC Rgeisters
	MAP_IO   ipcr;
	uint16   ipcReg[IPC_NREGS];

	// Interrupt Priority Levels
	UQ_IPL   iplList[UQ_HLVL];

	// I/O Page Table
	MAP_IO   *ioMap[IO_NREGS];
	MAP_IO   *ioList;
};
