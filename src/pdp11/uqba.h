// uqba.h - PDP-11 Unibus/QBus Interface Definitions
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

#define UQ_KEY     "UQBA"
#define UQ_NAME    "PDP-11 Unibus/QBus Interface"
#define UQ_VERSION "v0.7 (Alpha)"

#define IO_NREGS 4096  // 16-bit I/O registers

#define UQ_BME 1

#define IPL_HLVL  8    // Number of IPL Levels
#define IPL_NVECS 32   // Number of Vectors each IPL

#define UBM_CSRADR      0770200
#define UBM_NREGS       (UBM_SIZE << 1)

#define UBM_SIZE        32
#define UBM_P_PAGE      13
#define UBM_PAGE        037
#define UBM_BLKSZ       01000
#define UBM_BLKOFF      (UBM_BLKSZ - 1)
#define UBM_OFF         017777
#define UBM_GETPN(pn)   (((pn) >> UBM_P_PAGE) & UBM_PAGE)
#define UBM_GETOFF(off) ((off) & UBM_OFF)

typedef struct uq_Device UQ_IO;

struct uq_Device {
	UNIT   Unit;              // Unit Header Information

	P11_SYSTEM *System;
	P11_CPU    *Processor;
	UQ_CALL    *Callback;

	uint32 Flags;

	// Unibus Map
	uint32 map[UBM_SIZE];

	// Hardware Interrupts
	uint32 intRsvd[IPL_HLVL];
	uint32 intReqs[IPL_HLVL];
	uint32 intVecs[IPL_HLVL][IPL_NVECS];
	MAP_IO *intAck[IPL_HLVL][IPL_NVECS];

	// Internal CPU/MAP Registers
	MAP_IO ioUBM;   // Unibus MAP Registers
	MAP_IO ioCPU;   // CPU Registers
	MAP_IO ioSRMM;  // SR/MMR0-2 Registers
	MAP_IO ioMMR3;  // MMR3 Register
	MAP_IO ioAPR1;  // APR Registers (Kernel/Supervisor)
	MAP_IO ioAPR2;  // APR Registers (User)

	// I/O Page Map Area
	MAP_IO *ioList;
	MAP_IO *ioMap[IO_NREGS];
};

void   uq11_ResetIO(register UQ_IO *);
void   uq11_ResetAll(register UQ_IO *);
void   uq11_EvalIRQ(register P11_CPU *, uint32);
void   uq11_ClearIRQ(register UQ_IO *);
uint16 uq11_GetVector(register P11_CPU *, uint32);
int    uq11_ReadIO(register UQ_IO *, uint32, uint16 *, uint32);
int    uq11_WriteIO(register UQ_IO *, uint32, uint16, uint32);
