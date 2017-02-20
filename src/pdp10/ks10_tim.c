// ks10_tim.c - KS10 Processor: Timer System Routines
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

#include "pdp10/defs.h"
#include "pdp10/ks10.h"

#ifndef HAVE_SIGACTION
#include <signal.h>
#endif /* HAVE_SIGACTION */

int64 timebase;  // time base (59 (71-12) bits)
int36 period;    // Initial period for interval timer
int36 ttg;       // "Time to go" for interval timer
int   jiffy = 0; // 100 jiffies per second

// System Timing

void p10_HandleTimer(int x)
{
	int pi;

#ifndef HAVE_SIGACTION
	// [AAK] Note to Tim - Is this handler reentrant?
	signal(SIGALRM, SIG_IGN);
#endif /* HAVE_SIGACTION */

	// Increase time base by 10 because 10 millisecond
	// (100 jiffies per second) limitation in
	// Red Hat Linux operating system
	timebase = (timebase + TB_JIFFY) & TB_MASK;

	if (period) {
		ttg -= TB_JIFFY;

		// Set "Interval Done" flag on APR and request an interrupt.
		// Reset TTG (Time To Go) by using initial Period
		if (ttg <= 0) {
			ttg = period;
			p10_aprInterrupt(APRSR_F_INT_DONE);
		}
	}

	// Execute real timer entries each 10ms.
	ts10_TickRealTimer(1);

	if (jiffy++ == 100) {
		jiffy = 0;
#ifdef DBEUG
		if (dbg_Check(DBG_IPS))
			dbg_Printf("CPU: %d instructions per second.\n", cips);
#else
//		fprintf(debug, "CPU: %d instructions per second.\n", cips);
#endif /* DEBUG */
		cips = 0;
	}

#ifndef HAVE_SIGACTION
	signal(SIGALRM, p10_HandleTimer);
#endif /* HAVE_SIGACTION */
}


// 70260 WRTIM - Write Time Base Register
void p10_ksOpcode_WRTIM(void)
{
	// (E,E+1[0:23]) -> (Time Base)
	// 0 -> (Time Base[24:35])

	AR = p10_vRead(eAddr++, NOPXCT);
	ARX = p10_vRead(eAddr, NOPXCT);
	timebase = (AR << (35 - TB_HW_BITS));
	timebase |= (ARX & WORD36_MAXP) >> TB_HW_BITS;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TIME: Time Base <- %012llo %012llo\n", AR, ARX);
#endif /* DEBUG */
}

// 70220 RDTIM - Read Time Base Register
void p10_ksOpcode_RDTIM(void)
{
	// (Time Base) -> (E,E+1)

	AR  = timebase >> (35 - TB_HW_BITS);
	ARX = (timebase << TB_HW_BITS) & WORD36_MAXP;
	p10_vWrite(eAddr, AR, NOPXCT);
	p10_vWrite((eAddr + 1) & VMA_MASK, ARX, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TIME: Time Base -> %012llo %012llo\n", AR, ARX);
#endif /* DEBUG */
}

// 70264 WRINT - Write Interval Register
void p10_ksOpcode_WRINT(void)
{
	// (E) -> (Interval)

	AR = p10_vRead(eAddr, NOPXCT);
	period = AR >> TB_HW_BITS;
	ttg    = period;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TIME: Interval <- %012llo\n", AR);
#endif /* DEBUG */
}

// 70224 RDINT - Read Interval Register
void p10_ksOpcode_RDINT(void)
{
	// (Interval) -> (E)

	AR = period << TB_HW_BITS;
	p10_vWrite(eAddr, AR, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TIME: Interval -> %012llo\n", AR);
#endif /* DEBUG */
}
