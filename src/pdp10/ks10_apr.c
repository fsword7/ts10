// ks10_apr.c - KS10 Processor: APR (Arithmetic Processor) routines
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

extern P10_CONSOLE *p10_Console;

int apr_Enables; // Interrupt Enables
int apr_Flags;   // System Flags
int apr_Level;   // PI Channel Level
int p10_Serial;  // Processor Serial Number

// Initialize/Reset APR registers
void p10_ResetAPR(void)
{
	apr_Enables = 0; // Interrupt Enables
	apr_Flags   = 0; // System Flags
	apr_Level   = 0; // PI Channel Level
}

// Request an interrupt
inline void p10_aprInterrupt(int flag)
{
	apr_Flags |= flag;
	if (apr_Enables & flag) {
		KS10_piRequestAPR(apr_Level);
		KS10_piEvaluate();
	}
}

// Do an non-existant memory trap
int KS10_Trap_NoMemory(int30 pAddr, int mode)
{
	// Request an interrupt for non-existing memory encounter.
	p10_aprInterrupt(APRSR_F_NO_MEMORY);

	// If pager system is on, go page fail trap.
	if (KX10_Pager_On) {
		lhPFW = PFW_NXM|PFW_PAGED;
		rhPFW = pAddr;
		PAGE_FAIL_TRAP(mode);
	}
}

// 70000 APRID  Get APR Identification
void p10_ksOpcode_APRID(void)
{
	BR = KS10_MC_OPTS & APRID_M_MC_OPTS;
	BR |= (KS10_MC_VER << APRID_V_MC_VER) & APRID_M_MC_VER;
	BR |= KS10_HW_OPTS & APRID_M_HW_OPTS;
	BR |= p10_Serial & APRID_M_PROC_SN;
	p10_vWrite(eAddr, BR, NOPXCT);
}

// 70020 WRAPR  Write APR register
void p10_ksOpcode_WRAPR(void)
{
	int flags = eAddr & APR_FLAGS;

	apr_Level = eAddr & APR_LEVEL;
	if (eAddr & APR_ENABLE)  apr_Enables |= flags;
	if (eAddr & APR_DISABLE) apr_Enables &= ~flags;
	if (eAddr & APR_SET)     apr_Flags   |= flags;
	if (eAddr & APR_CLEAR)   apr_Flags   &= ~flags;

	// Process CTY device when an interrupt console had been set.
	if (apr_Flags & APRSR_F_INT_CON) {
		p10_ConsoleOutput(p10_Console);
		apr_Flags &= ~APRSR_F_INT_CON;
	}

	KS10_piEvaluate();
}

// 70024 RDAPR  Read APR register
void p10_ksOpcode_RDAPR(void)
{
	int36 apr_sr;

	apr_sr = ((apr_Enables & apr_Flags) ? APR_IRQ : 0) |
	         (apr_Enables << 18) | apr_Flags | apr_Level;

	p10_vWrite(eAddr, apr_sr, NOPXCT);
}

// 70030 CONSO APR, Instruction
void p10_ksOpcode_COAPR(void)
{
	int18 apr_sr;

	apr_sr = ((apr_Enables & apr_Flags) ? APR_IRQ : 0) |
	         apr_Flags | apr_Level;

	if (apr_sr & eAddr)
		DO_SKIP;
}

// 70034 CONSZ APR, Instruction
void p10_ksOpcode_CZAPR(void)
{
	int18 apr_sr;

	apr_sr = ((apr_Enables & apr_Flags) ? APR_IRQ : 0) |
	         apr_Flags | apr_Level;

	if ((apr_sr & eAddr) == 0)
		DO_SKIP;
}
