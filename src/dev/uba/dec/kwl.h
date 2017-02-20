// kwl.h - KW11L Line Clock Emulator
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

#include "emu/defs.h"
#include "dec/defs.h"

#define KWL_KEY      "KW11L"
#define KWL_NAME     "Line Clock Emulator"
#define KWL_VERSION  "v0.0 (Pre-Alpha)"

#define KWL_CSRADR 0777546
#define KWL_NREGS  1
#define KWL_NVECS  1
#define KWL_IPL    UQ_BR6
#define KWL_VEC    0100

#define KWL_50HZ   50 // 50Hz
#define KWL_60HZ   60 // 60Hz

#define CLKCSR       kwl->csrClock
#define CLKCSR_DONE  0000200
#define CLKCSR_IE    0000100
#define CLKCSR_RW    CLKCSR_IE

typedef struct kwl_Clock KWL_CLOCK;

struct kwl_Clock {
	UNIT      Unit;      // Unit Header Information
	void      *Device;   // I/O Interface
	void      *System;
	UQ_CALL   *Callback; // I/O Callback Routines
	MAP_IO    ioMap;     // I/O Mapping

	uint32    Flags;     // Internal Flags
	uint32    hzRate;    // 50/60 Hz Rate
	uint16    csrClock;  // Clock CSR Register

	CLK_QUEUE Timer;     // Tick Timer
};
