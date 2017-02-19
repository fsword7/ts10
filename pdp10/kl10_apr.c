// kl10_apr.c - APR (Arithmetic Processor)
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

// Device Name: APR
// Device Code: 000

#include "pdp10/defs.h"
#include "pdp10/kl10.h"
#include "pdp10/iodefs.h"

#define NOPXCT 0

// External function from kl10_cca.c.
void KL10_ioOpcode_WRFIL(void *);

// Control/Status and Debug Registers
int32 KL10apr_srEnables; // (Status) Enables to interrupt
int32 KL10apr_srFlags;   // (Status) Control/Status Register
int32 KL10apr_srLevel;   // (Status) Priority Interrupt Assignment
int32 KL10apr_drFlags;   // (Debug)  Reference Flags
int32 KL10apr_drBreak;   // (Debug)  Address Break

// 70000 APRID (BLKI APR,) Instruction
// Read APR Identification
void KL10_ioOpcode_APRID(void *uptr)
{
	int36 aprid = p10_Serial;

	p10_vWrite(eAddr, aprid, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("KL10(APR): (BI,R) ID => %06llo,,%06llo\n",
			LHSR(aprid), RH(aprid));
	}
#endif /* DEBUG */
}

// 70004 RDDBG (DATAI APR,) Instruction
// Read Address Debugging
void KL10_ioOpcode_RDDBG(void *uptr)
{
	p10_vWrite(eAddr, KL10apr_drFlags, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(APR): (DI,R) DR => %06o,,%06o\n",
			LHSR(KL10apr_drFlags), RH(KL10apr_drFlags));
#endif /* DEBUG */
}

// 70014 WRDBG (DATAO APR,) Instruction
// Write Address Debugging
void KL10_ioOpcode_WRDBG(void *uptr)
{
	int32 aprdr;

	aprdr = p10_vRead(eAddr, NOPXCT);
	KL10apr_drFlags = aprdr & APRDR_FLAGS;
	KL10apr_drBreak = aprdr & APRDR_BREAK;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(APR): (DO,W) DR <= %06o,,%06o\n",
			SR(aprdr), RH(aprdr));
#endif /* DEBUG */
}

// 70020 WRAPR (CONO APR,) Instruction
// Write APR Status Register
void KL10_ioOpcode_WRAPR(void *uptr)
{
	// Reset all I/O devices.
	if (eAddr & APRSR_CLRIO) {
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("KL10(APR): (CO,W) Reset All I/O Devices.\n");
#endif /* DEBUG */

		// Reset all I/O devices through DIA interface.
		KX10_ResetAllDevices();
	}

	// Set system flags and priority interrupt assignment.
	if (eAddr & APRSR_ENABLE)  KL10apr_srEnables |=  (eAddr & APRSR_FLAGS);
	if (eAddr & APRSR_DISABLE) KL10apr_srEnables &= ~(eAddr & APRSR_FLAGS);
	if (eAddr & APRSR_SET)     KL10apr_srFlags   |=  (eAddr & APRSR_FLAGS);
	if (eAddr & APRSR_CLEAR)   KL10apr_srFlags   &= ~(eAddr & APRSR_FLAGS);
	KL10apr_srLevel = eAddr & APRSR_PIA;

	// ACTION: Need timer to count a few cycle to happen.
	KL10pi_Evaluate();

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int32 aprsr = KL10apr_srFlags | KL10apr_srLevel |
			((KL10apr_srEnables & KL10apr_srFlags) ? APRSR_IRQ : 0);
		dbg_Printf("KL10(APR): (CO,W) SR %06o,,%06o <= %06o\n",
			KL10apr_srEnables, aprsr, eAddr);
	}
#endif /* DEBUG */
}

// 70024 RDAPR (CONI APR,) Instruction
// Read APR Status Register
void KL10_ioOpcode_RDAPR(void *uptr)
{
	int32 aprsr = ((KL10apr_srEnables & KL10apr_srFlags) ? APRSR_IRQ : 0) |
		SL(KL10apr_srEnables) | KL10apr_srFlags | KL10apr_srLevel;

	p10_vWrite(eAddr, aprsr, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("KL10(APR): (CI,R) SR => %06o,,%06o\n",
			SR(aprsr), RH(aprsr));
	}
#endif /* DEBUG */
}

// 70030 CZAPR (CONSZ APR,) Instruction
// Test APR Status Register - Skip if Zero.
void KL10_ioOpcode_CZAPR(void *uptr)
{
	int32 aprsr = ((KL10apr_srEnables & KL10apr_srFlags) ? APRSR_IRQ : 0) |
		KL10apr_srFlags | KL10apr_srLevel;

	if ((aprsr & eAddr) == 0)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int32 result = aprsr & eAddr;
		dbg_Printf("KL10(APR): (CZ,T) SR => %06o & %06llo = %06o : %s\n",
			aprsr, eAddr, result, (result == 0) ? "Skip" : "Continue");

	}
#endif /* DEBUG */
}

// 70034 COAPR (CONSO APR,) Instruction
// Test APR Status Register - Skip if one
void KL10_ioOpcode_COAPR(void *uptr)
{
	int32 aprsr = ((KL10apr_srEnables & KL10apr_srFlags) ? APRSR_IRQ : 0) |
		KL10apr_srFlags | KL10apr_srLevel;

	if (aprsr & eAddr)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int32 result = aprsr & eAddr;
		dbg_Printf("KL10(APR): (CO,T) SR => %06o & %06llo = %06o : %s\n",
			aprsr, eAddr, result, result ? "Skip" : "Continue");
	}
#endif /* DEBUG */
}

void kl10_InitAPR(KL10_DEVICE *kl10)
{
	P10_IOMAP *io;

	if (io = (P10_IOMAP *)calloc(1, sizeof(P10_IOMAP))) {
		io->devName    = "APR";
		io->keyName    = "APR";
		io->emuName    = "KL10: Arithmetic Processor";
		io->emuVersion = "(Internal)";
		io->idDevice   = KL10_APR;
		io->ResetIO    = NULL;

		// Set APR instructions for KL10 processor
		io->Function[IOF_BLKI]  = KL10_ioOpcode_APRID;
		io->Function[IOF_DATAI] = KL10_ioOpcode_RDDBG;
		io->Function[IOF_BLKO]  = KL10_ioOpcode_WRFIL;
		io->Function[IOF_DATAO] = KL10_ioOpcode_WRDBG;
		io->Function[IOF_CONO]  = KL10_ioOpcode_WRAPR;
		io->Function[IOF_CONI]  = KL10_ioOpcode_RDAPR;
		io->Function[IOF_CONSZ] = KL10_ioOpcode_CZAPR;
		io->Function[IOF_CONSO] = KL10_ioOpcode_COAPR;
		
		// Assign APR device to I/O mapping
		kx10_SetMap(io);
	}
}

// Non-Existant Memory Trap 
int KL10_Trap_NoMemory(int30 pAddr, int mode)
{
	KL10apr_srFlags |= APRSR_NXM;

	// ACTION: Implement a page fail trap call.
}
