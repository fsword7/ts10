// kl10_pi.c - PI (Priority Interrupt System)
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

// Device Name: PI
// Device Code: 004

#include "pdp10/defs.h"
#include "pdp10/kl10.h"
#include "pdp10/iodefs.h"

// Select highest priority number from levels
static int toLevel[128] = {
	0, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static int toMask[8] = {
	PI_INT0, // Interrupt Level #0 - No Interrupts
	PI_INT1, // Interrupt Level #1 - Highest priority
	PI_INT2, // Interrupt Level #2
	PI_INT3, // Interrupt Level #3
	PI_INT4, // Interrupt Level #4
	PI_INT5, // Interrupt Level #5
	PI_INT6, // Interrupt Level #6
	PI_INT7, // Interrupt Level #7 - Lowest priority
};

#ifdef DEBUG
static char *iopNames[] = {
	"Internal Device",
	"Standard Interrupt",
	"Vector Interrupt",
	"Increment",
	"Examine Data",
	"Deposit Data",
	"Byte Transfer",
	"(Unknown)"
};
#endif /* DEBUG */

// IRQ flag from cpu_main.c
extern int32 KX10_IntrQ;

// APR flags from kl10_apr.c.
extern int32 KL10apr_srFlags;
extern int32 KL10apr_srEnables;
extern int32 KL10apr_srLevel;
extern int30 KL10_eptAddr;

int32 KL10pi_On;        // PI System On/Off  (On = 1, Off = 0)
int32 KL10pi_Flags;     // PI Flags
int32 KL10pi_aprReqs;   // Interrupt Requests from APR.
int32 KL10pi_ioReqs;    // Interrupt Requests from I/O devices.
int32 KL10pi_pgmReqs;   // Interrupt Requests.
int32 KL10pi_Actives;   // Interrupt Actives (In Progress)
int32 KL10pi_Enables;   // Interrupt Enables
int36 KL10_iopWords[8]; // Interrupt Function Word.
int36 *KL10_iopWord;    // Current Interrupt Function Word (AC 3, Block 7)

// *******************************************************

void KL10pi_Reset(void)
{
	KL10pi_On      = 0;
	KL10pi_Flags   = 0;
	KL10pi_aprReqs = 0;
	KL10pi_ioReqs  = 0;
	KL10pi_pgmReqs = 0;
	KL10pi_Actives = 0;
	KL10pi_Enables = 0;
	KL10_iopWord = &p10_ACB[7][3];
}

// Evaluate priority interrupts
void KL10pi_Evaluate(void)
{
	int actlvl, reqlvl;

	KX10_IntrQ = 0;
	if (KL10pi_On) {
		KL10pi_aprReqs = (KL10apr_srFlags & KL10apr_srEnables)
			? toMask[KL10apr_srLevel] : 0;
		reqlvl = toLevel[(KL10pi_aprReqs | KL10pi_ioReqs | KL10pi_pgmReqs)
			& KL10pi_Enables];
		actlvl = toLevel[KL10pi_Actives];
		if ((actlvl == 0) || (reqlvl < actlvl)) {
			KL10pi_ioReqs &= ~toMask[reqlvl];
			KX10_IntrQ = reqlvl;
		}
	}
}

void KL10pi_RequestAPR(int pi)
{
	KL10pi_aprReqs |= toMask[pi];

#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT))
		dbg_Printf("PI: Channel %d had been requested for APR.\n", pi);
#endif /* DEBUG */
}

// Request an interrupt with interrupt function word
void KL10pi_RequestIO(int pi, int36 iop)
{
	KL10pi_ioReqs |= toMask[pi];
	KL10_iopWords[pi] = iop;
	KL10pi_Evaluate();

#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT)) {
		dbg_Printf("PI: Channel %d had been requested for I/O.\n", pi);
		dbg_Printf("PI: Interrupt Function Word = %06o,,%06o\n",
			LH18(iop), RH18(iop));
	}
#endif /* DEBUG */
}

void KL10pi_Dismiss(void)
{
#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT))
		dbg_Printf("KL10(PI): Channel %d dismissed.\n",
			toLevel[KL10pi_Actives]);
#endif /* DEBUG */

	// Dismiss highest priority level.
	KL10pi_Actives &= ~toMask[toLevel[KL10pi_Actives]];
	KL10pi_Evaluate();
}

// Priority Interrupt Process during an interrupt.
void KL10pi_Process(void)
{
	int36 iopWord;
	int32 iopOpc;
	int32 iopDev;
	int30 iopAddr;

	if (cpu_pFlags & CPU_CYCLE_PI) {
		// Second part of interrupt instruction
		iopAddr = KL10_eptAddr + EPT_PI_BASE2 + (KX10_IntrQ << 1);
		KL10_piExecute(iopAddr);

		// Always get out of PI cycle after 2nd interrupt instruction
		cpu_pFlags &= ~CPU_CYCLE_PI;
	} else {
		// First part of interrupt instruction
		cpu_pFlags |= CPU_CYCLE_PI;
		iopWord = KL10_iopWords[KX10_IntrQ];
		iopOpc  = (iopWord >> IRQ_P_FNC) & IRQ_M_FNC;

		// Save IOP word into AC 3 in block 7.
		*KL10_iopWord = SXT36(iopWord);

#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("KL10(PI): Reason: %s Device: %o Address: %08o\n",
				iopNames[iopOpc], (int32)((iopWord >> IRQ_M_DEV) & IRQ_M_DEV),
				(iopWord & IRQ_M_ADR));
#endif /* DEBUG */

		switch (iopOpc) {
			case IRQ_FNC_INT: // Internal Device
			case IRQ_FNC_STD: // Standard Interrupt
				iopAddr = KL10_eptAddr + EPT_PI_BASE1 + (KX10_IntrQ << 1);
				KL10_piExecute(iopAddr);
				break;

			case IRQ_FNC_VEC: // Vector Interrupt
				iopDev = ((int32)iopWord >> IRQ_P_DEV) & IRQ_M_DEV;
				switch (iopDev >> 2) {
					case 0: case 1:
						iopAddr = KL10_eptAddr + ((int30)iopWord & IRQ_M_ADR);
						KL10_piExecute(iopAddr);
						break;

					case 2:
						iopAddr = KL10_eptAddr + 0142;
						KL10_piExecute(iopAddr);
						break;
				}
				break;
		}
	}

	if (pager_PC != PC) {
		KL10_iopWords[KX10_IntrQ] = 0;
		KL10pi_Actives |= toMask[KX10_IntrQ];
		KL10pi_Evaluate();

		// Get out of PI cycle
		cpu_pFlags &= ~CPU_CYCLE_PI;
	}
}

// *******************************************************

// 70060 WRPI (CONO PI,) Instruction
// Write PI Status Register
void KL10_ioOpcode_WRPI(void *uptr)
{
	// Clear priority interrupt system completely.
	if (eAddr & PISR_CLEAR) {
		KL10pi_On      = 0;
		KL10pi_pgmReqs = 0;
		KL10pi_Actives = 0;
		KL10pi_Enables = 0;
	}

	// Set/clear write even parity flags
	KL10pi_Flags = (KL10pi_Flags & ~PISR_PFLAGS) | (eAddr & PISR_PFLAGS);

	// Turn on/off priority interrupt system.
	if (eAddr & PISR_ON)   KL10pi_Flags   |= PISR_ON;
	if (eAddr & PISR_OFF)  KL10pi_Flags   &= ~PISR_ON;
	KL10pi_On = KL10pi_Flags & PISR_ON;

	// Set/clear priority interrupt levels.
	if (eAddr & PISR_REQ)  KL10pi_pgmReqs |=  (eAddr & PISR_LEVELS);
	if (eAddr & PISR_DROP) KL10pi_pgmReqs &= ~(eAddr & PISR_LEVELS);
	if (eAddr & PISR_ION)  KL10pi_Enables |=  (eAddr & PISR_LEVELS);
	if (eAddr & PISR_IOFF) KL10pi_Enables &= ~(eAddr & PISR_LEVELS);

	// Evaluate priority interrupts
	KL10pi_Evaluate();

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int32 pisr = (KL10pi_Actives << 8) | KL10pi_Enables | KL10pi_Flags;
		dbg_Printf("KL10(PI): (CO,W) SR %06o,,%06o <= %06o\n",
			KL10pi_pgmReqs, pisr, eAddr);
		dbg_Printf("KL10(PI): System: %s Req: %03o Active: %03o Level: %03o\n",
			(KL10pi_On ? "On" : "Off"), KL10pi_pgmReqs, KL10pi_Actives,
			KL10pi_Enables);
	}
#endif /* DEBUG */
}

// 70064 RDPI (CONI PI,) Instruction
// Read PI Status Register
void KL10_ioOpcode_RDPI(void *uptr)
{
	int32 pisr = SL(KL10pi_pgmReqs) | (KL10pi_Actives << 8)
		| KL10pi_Enables | KL10pi_Flags;

	p10_vWrite(eAddr, pisr, PXCT_CUR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(PI): (CI,R) SR => %06o,,%06o\n",
			KL10pi_pgmReqs, RH(pisr));
#endif /* DEBUG */
}

// 70070 CZPI (CONSZ PI,) Instruction
// Test Conditions and Skip if Zero
void KL10_ioOpcode_CZPI(void *uptr)
{
	int32 pisr = (KL10pi_Actives << 8) | KL10pi_Enables | KL10pi_Flags;

	if ((pisr & eAddr) == 0)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int32 result = pisr & eAddr;
		dbg_Printf("KL10(PI): (CSZ,T) SR => %06o & %06llo = %06o : %s\n",
			pisr, eAddr, result, (result == 0) ? "Skip" : "Continue");
	}
#endif /* DEBUG */
}

// 70074 COPI (CONSO PI,) Instruction
// Test Conditions and Skip if One
void KL10_ioOpcode_COPI(void *uptr)
{
	int32 pisr = (KL10pi_Actives << 8) | KL10pi_Enables | KL10pi_Flags;

	if (pisr & eAddr)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int32 result = pisr & eAddr;
		dbg_Printf("KL10(PI): (CSO,T) SR => %06o & %06llo = %06o : %s\n",
			pisr, eAddr, result, result ? "Skip" : "Continue");
	}
#endif /* DEBUG */
}

void kl10_InitPI(KL10_DEVICE *kl10)
{
	P10_IOMAP *io;

	if (io = (P10_IOMAP *)calloc(1, sizeof(P10_IOMAP))) {
		io->devName    = "PI";
		io->keyName    = "PI";
		io->emuName    = "KL10: Priority Interrupt";
		io->emuVersion = "(Internal)";
		io->idDevice   = KL10_PI;
		io->ResetIO    = NULL;

		// Set APR instructions for KL10 processor
		io->Function[IOF_BLKI]  = NULL;
		io->Function[IOF_DATAI] = NULL;
		io->Function[IOF_BLKO]  = NULL;
		io->Function[IOF_DATAO] = NULL;
		io->Function[IOF_CONO]  = KL10_ioOpcode_WRPI;
		io->Function[IOF_CONI]  = KL10_ioOpcode_RDPI;
		io->Function[IOF_CONSZ] = KL10_ioOpcode_CZPI;
		io->Function[IOF_CONSO] = KL10_ioOpcode_COPI;
		
		// Assign APR device to I/O mapping
		kx10_SetMap(io);
	}
}
