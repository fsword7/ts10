// j11.h - KDJ11 (J11) Series Definitions
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

#define J11_KEY     "KDJ11"
#define J11_NAME    "MicroPDP Emulator"
#define J11_VERSION "v0.6 (Alpha)"

#include "pdp11/uqba.h"

// Standard Devices (partial of CPU area)
// KW11L - Line Time Clock

#define LTC_KEY    "KW11L"  // Device Type
#define LTC_CSRADR 0777546  // CSR Address
#define LTC_VEC    0100     // Vector Address
#define LTC_IPL    UQ_BR6   // Interrupt Level
#define LTC_NREGS  1        // Number of Registers
#define LTC_NVECS  1        // Number of Vectors
#define LTC_TICK   16       // 60Hz Rate

#define LTC_DONE   0000200 // Done (Set by each tick)
#define LTC_IE     0000100 // Enable Interrupt
#define LTC_RW     LTC_IE

#define LTC        j11->clkcsr

typedef struct j11_Processor J11_CPU;

struct j11_Processor {
	P11_CPU    cpu;

	// Standard Devices
	// KW11L Line Time Clock
	MAP_IO     ioClock;
	CLK_QUEUE  clkTimer;
	uint32     clkCount;
	uint16     clkcsr;
};
