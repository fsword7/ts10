// kl10_mtr.c - KL10 Processor: Meter and Timer routines
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

// Device Name: TIM, MTR
// Device Code: 020, 024

#include "pdp10/defs.h"
#include "pdp10/kl10.h"
#include "pdp10/iodefs.h"

// *******************************************************

#define ON  1
#define OFF 0

extern int30 KL10_eptAddr; // Exec Process Table Address
extern int30 KL10_uptAddr; // User Process Table Address

// Accouting Meters
static int     mtrEnable;  // Meter System - On/Off
static uint18  mtrFlags;   // Meter Flags

// Time Base Counter
static int     timEnable;  // Time Base Counter - On/Off
static uint18  timFlags;   // Time Base Flags

// Interval Counter
static int     intEnable;  // Interval Counter - On/Off
static uint18  intFlags;   // Interval Flags (On, Done, and Overflow flags)
static uint16  intCount;   // Interval Count
static uint16  intPeriod;  // Interval Period

CLK_QUEUE kl10_Timer;

void KL10_TimeReset(void *uptr)
{
	// Clear all meters
	mtrEnable = OFF;
	mtrFlags  = 0;

	// Clear all time base
	timEnable = OFF;
	timFlags  = 0;

	// Clear all interval counter.
	intEnable = OFF;
	intFlags  = 0;
	intCount  = 0;
	intPeriod = 0;
}

void KL10_TimeBaseCount(void)
{
	// Get address of EPT time base doubleword.
	int36 *tb1 = p10_pAccess(KL10_eptAddr + EPT_TIMEBASE);
	int36 *tb2 = p10_pAccess(KL10_eptAddr + EPT_TIMEBASE + 1);

	// Update time base doubleword.
	*tb2 += TIM_TICK;
	if (*tb2 > WORD36_MAXP) {
		(*tb1)++;
		*tb1 = SXT36(*tb1);
		*tb2 = 0;
	}
}

void KL10_IntervalCount(void)
{
	intCount += 1000;

	// If period matches or overflow occurs, perform an interrupt.
	if ((intCount >= intPeriod) || (intCount >= INT_COUNT)) {
		int pi = timFlags & INT_PIA;

		// Set interval flags and
		// check counter for overflow.
		intFlags |= INT_DONE;
		if (intCount >= INT_COUNT)
			intFlags |= INT_OVF;
		intCount = 0;

		if (pi > 0) {
#ifdef DEBUG
			if (dbg_Check(DBG_INTERRUPT))
				dbg_Printf("KL10(TIM): Interval Interrupt\n");
#endif /* DEBUG */

			// Perform an interval interrupt
			KL10pi_RequestIO(pi, IRQ_VEC | EPT_INTERVAL);
		}
	}
}

#if 0
void KL10_ClockCount(void)
{
	static int intWait = 0;

	// Count at 1 MHz rate
	if (timEnable)
		KL10_TimeBaseCount();

	// Count at 100 KHz rate
	if (intWait++ < 10) {
		intWait = 0;
		if (intEnable)
			KL10_IntervalCount();
	}
}
#endif

// Execute once each 10ms.
void kl10_TickCount(void *dptr)
{
	if (timEnable)
		KL10_TimeBaseCount();
	if (intEnable)
		KL10_IntervalCount();
}

// ****************** Accounting/Timing ********************

// 70204 RDTIME (DATAI TIM,) Instruction
// Read Time Base (Data In, Timing)
void KL10_ioOpcode_RDTIME(void *uptr)
{
	int36 tb1, tb2;

	tb1 = p10_pRead(KL10_eptAddr + EPT_TIMEBASE, 0);
	tb2 = p10_pRead(KL10_eptAddr + EPT_TIMEBASE + 1, 0);

	p10_vWrite(eAddr, tb1, 0);
	p10_vWrite(eAddr+1, tb2, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(MTR): TIME BASE => %c%06o,,%06o %c%06o,,%06o\n",
			(tb1 < 0 ? '-' : '+'), LH18(tb1), RH18(tb1),
			(tb2 < 0 ? '-' : '+'), LH18(tb2), RH18(tb2));
#endif /* DEBUG */
}

// 70240 RDMACT (BLKI MTR,) Instruction
// Read Memory Account
void KL10_ioOpcode_RDMACT(void *uptr)
{
	int36 mc1, mc2;

	mc1 = p10_pRead(KL10_uptAddr + UPT_MEMCOUNT, 0);
	mc2 = p10_pRead(KL10_uptAddr + UPT_MEMCOUNT + 1, 0);

	p10_vWrite(eAddr, mc1, 0);
	p10_vWrite(eAddr+1, mc2, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(MTR): MACT => %c%06o,,%06o %c%06o,,%06o\n",
			(mc1 < 0 ? '-' : '+'), LH18(mc1), RH18(mc1),
			(mc2 < 0 ? '-' : '+'), LH18(mc2), RH18(mc2));
#endif /* DEBUG */
}

// 70244 RDEACT (DATAI MTR,) Instruction
// Read Execution Account
void KL10_ioOpcode_RDEACT(void *uptr)
{
	int36 pt1, pt2;

	pt1 = p10_pRead(KL10_uptAddr + UPT_PROCTIME, 0);
	pt2 = p10_pRead(KL10_uptAddr + UPT_PROCTIME + 1, 0);

	p10_vWrite(eAddr, pt1, 0);
	p10_vWrite(eAddr+1, pt2, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(MTR): EACT => %c%06o,,%06o %c%06o,,%06o\n",
			(pt1 < 0 ? '-' : '+'), LH18(pt1), RH18(pt1),
			(pt2 < 0 ? '-' : '+'), LH18(pt2), RH18(pt2));
#endif /* DEBUG */
}

// 70260 WRTIME (CONO MTR,) Instruction
// Conditions Out, Meters
void KL10_ioOpcode_WRTIME(void *uptr)
{
	// Set up accounts for meters
	if (eAddr & MTR_SET) {
		mtrEnable = (eAddr & MTR_ON) ? ON : OFF;
		mtrFlags  = (eAddr & MTR_FLAGS);
	}

	// Turn on time base count
	if (eAddr & TIM_ON) {
		timEnable = ON;
		timFlags |= TIM_ON;
	}

	// Turn off time base count
	if (eAddr & TIM_OFF) {
		timEnable = OFF;
		timFlags &= ~TIM_ON;
	}

	timFlags = (timFlags & ~INT_PIA) | (eAddr & INT_PIA);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("KL10(MTR): MR %06o <= %06o\n",
			mtrFlags | timFlags, (int18)eAddr);
		dbg_Printf("KL10(MTR): Meter: %s Time: %s PI: %o\n",
			(mtrEnable ? "On" : "Off"), (timEnable ? "On" : "Off"),
			timFlags & INT_PIA);
	}
#endif /* DEBUG */
}

// 70264 CONI MTR, Instruction
// Conditions In, Meters
void KL10_ioOpcode_RDMTR(void *uptr)
{
	p10_vWrite(eAddr, mtrFlags | timFlags, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(MTR): MR => %06o\n", mtrFlags | timFlags);
#endif /* DEBUG */
}

// 70270 CONSZ MTR, Instruction
// Conditions In and Skip if Zero, Meters 
void KL10_ioOpcode_CZMTR(void *uptr)
{
	int18 flags = mtrFlags | timFlags;

	if ((flags & (int18)eAddr) == 0)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int result = flags & (int18)eAddr;
		dbg_Printf("KL10(MTR): MR => %06o & %06o = %06o : %s\n",
			flags, (int18)eAddr, result, (result == 0 ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

// 70274 CONSZ MTR, Instruction
// Conditions In and Skip if Ones, Meters 
void KL10_ioOpcode_COMTR(void *uptr)
{
	int18 flags = mtrFlags | timFlags;

	if (flags & (int18)eAddr)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int result = flags & (int18)eAddr;
		dbg_Printf("KL10(MTR): MR => %06o & %06o = %06o : %s\n",
			flags, (int18)eAddr, result, (result ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

// **************** Interval Counter ********************

// 70220 WRINT (CONO TIM,) Instruction
// Conditions Out, Interval Counter
void KL10_ioOpcode_WRINT(void *uptr)
{
	// Turn on or off the interval counter
	if (eAddr & INT_ON) {
		intEnable = ON;
		intFlags |= INT_ON;
	} else {
		intEnable = OFF;
		intFlags &= ~INT_ON;
	}

	// Clear the interval count
	if (eAddr & INT_CLR)
		intCount = 0;

	// Clear interval flags
	if (eAddr & INT_CFLG)
		intFlags &= ~(INT_DONE|INT_OVF);

	// Load the interval period
	intPeriod = eAddr & INT_PERIOD;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("KL10(TIM): ICR %06o,,%06o <= %06o\n",
			intCount, intFlags | intPeriod, (int18)eAddr);
		dbg_Printf("KL10(TIM): Enable: %s Flags: %c%c%c Period: %04o Count: %04o\n",
			(intEnable ? "On" : "Off"),
			((intFlags & INT_ON)   ? 'o' : '-'),
			((intFlags & INT_DONE) ? 'd' : '-'),
			((intFlags & INT_OVF)  ? 'v' : '-'),
			intPeriod, intCount);
	}
#endif /* DEBUG */
}

// 70224 RDINT (CONI TIM,) Instruction
// Conditions In, Interval Counter
void KL10_ioOpcode_RDINT(void *uptr)
{
	int36 intcnt = ((int36)intCount << 18) | intFlags | intPeriod;

	p10_vWrite(eAddr, intcnt, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(TIM): ICR => %06o,,%06o\n",
			intCount, intFlags | intPeriod);
#endif /* DEBUG */
}

// 70230 CZINT (CONSZ TIM,) Instruction
// Conditions In and Skip if Zero, Interval Counter
void KL10_ioOpcode_CZINT(void *uptr)
{
	int18 flags = intFlags | intPeriod;

	if ((flags & (int18)eAddr) == 0)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int18 result = flags & (int18)eAddr;
		dbg_Printf("KL10(TIM): ICR => %06o & %06o = %06o : %s\n",
			flags, (int18)eAddr, result, (result == 0 ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

// 70234 CZINT (CONSZ TIM,) Instruction
// Conditions In and Skip if Zero, Interval Counter
void KL10_ioOpcode_COINT(void *uptr)
{
	int18 flags = intFlags | intPeriod;

	if (flags & (int18)eAddr)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int18 result = flags & (int18)eAddr;
		dbg_Printf("KL10(TIM): ICR => %06o & %06o = %06o : %s\n",
			flags, (int18)eAddr, result, (result ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

// ************ Performance Analysis Count **************

// 70200 RDPERF (BLKI TIM,)
// Read Performance Analysis Count
void KL10_ioOpcode_RDPERF(void *uptr)
{
	int36 pac1, pac2;

	pac1 = p10_pRead(KL10_eptAddr + EPT_PACOUNT, 0);
	pac2 = p10_pRead(KL10_eptAddr + EPT_PACOUNT + 1, 0);

	p10_vWrite(eAddr, pac1, 0);
	p10_vWrite(eAddr+1, pac2, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(MTR): PA Count => %c%06o,,%06o %c%06o,,%06o\n",
			(pac1 < 0 ? '-' : '+'), LH18(pac1), RH18(pac1),
			(pac2 < 0 ? '-' : '+'), LH18(pac2), RH18(pac2));
#endif /* DEBUG */
}

// 70210 WRPAE (BLKO TIM,)
// Write Performance Analysis Enables
void KL10_ioOpcode_WRPAE(void *uptr)
{
}

// *****************************************************

void kl10_InitTIM(KL10_DEVICE *kl10)
{
	P10_IOMAP *io;
	CLK_QUEUE *timer;

	if (io = (P10_IOMAP *)calloc(1, sizeof(P10_IOMAP))) {
		io->devName    = "TIM";
		io->keyName    = "TIM";
		io->emuName    = "KL10: Timer";
		io->emuVersion = "(Internal)";
		io->idDevice   = KL10_TIM;
		io->ResetIO    = NULL;

		// Set APR instructions for KL10 processor
		io->Function[IOF_BLKI]  = KL10_ioOpcode_RDPERF;
		io->Function[IOF_DATAI] = KL10_ioOpcode_RDTIME;
		io->Function[IOF_BLKO]  = KL10_ioOpcode_WRPAE;
		io->Function[IOF_DATAO] = NULL;
		io->Function[IOF_CONO]  = KL10_ioOpcode_WRINT;
		io->Function[IOF_CONI]  = KL10_ioOpcode_RDINT;
		io->Function[IOF_CONSZ] = KL10_ioOpcode_CZINT;
		io->Function[IOF_CONSO] = KL10_ioOpcode_COINT;
		
		// Assign APR device to I/O mapping
		kx10_SetMap(io);

		// Set up KL10 Tick Timer
		timer = &kl10_Timer;
		timer->Next      = NULL;
		timer->Name      = "KL10 Tick Timer";
		timer->Flags     = CLK_ENABLE|CLK_REACTIVE;
		timer->outTimer  = 10000;
		timer->nxtTimer  = 10000;
		timer->Device    = NULL;
		timer->Execute   = kl10_TickCount;
	}
}

void kl10_InitMTR(KL10_DEVICE *kl10)
{
	P10_IOMAP *io;

	if (io = (P10_IOMAP *)calloc(1, sizeof(P10_IOMAP))) {
		io->devName    = "MTR";
		io->keyName    = "MTR";
		io->emuName    = "KL10: Meter";
		io->emuVersion = "(Internal)";
		io->idDevice   = KL10_MTR;
		io->ResetIO    = NULL;

		// Set APR instructions for KL10 processor
		io->Function[IOF_BLKI]  = KL10_ioOpcode_RDMACT;
		io->Function[IOF_DATAI] = KL10_ioOpcode_RDEACT;
		io->Function[IOF_BLKO]  = NULL;
		io->Function[IOF_DATAO] = NULL;
		io->Function[IOF_CONO]  = KL10_ioOpcode_WRTIME;
		io->Function[IOF_CONI]  = KL10_ioOpcode_RDMTR;
		io->Function[IOF_CONSZ] = KL10_ioOpcode_CZMTR;
		io->Function[IOF_CONSO] = KL10_ioOpcode_COMTR;
		
		// Assign APR device to I/O mapping
		kx10_SetMap(io);
	}
}
