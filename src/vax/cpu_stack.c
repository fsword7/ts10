// cpu_call.c - Procedure Call Instructions
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

//   CALLG  Call Procedure with General Argument List
//   CALLS  Call Procedure with Stack Argument List
//   RET    Return from Procedure
//
//   PUSHL  Push Longword
//
//   PUSHR  Push Registers into Stack
//   POPR   Pop Registers from Stack

#include "vax/defs.h"

// Procedure Call Stack Frame
//
// +----------------------------------------------------+
// |            Condition Handler (initially 0)         | :(FP)
// +---+-+-+-----------------+-+--------------+---------+
// |SPA|S|0|    Mask<11:0>   |Z|   PSW<14:5>  | 0 0 0 0 |
// +---+-+-+-----------------+-+--------------+---------+
// |                      saved AP                      |
// +----------------------------------------------------+
// |                      saved FP                      |
// +----------------------------------------------------+
// |                      saved PC                      |
// +----------------------------------------------------+
// |                    saved R0 (...)                  |
// +----------------------------------------------------+
//              :                         :
//              :                         :
// +----------------------------------------------------+
// |                    saved R11 (...)                 |
// +----------------------------------------------------+
//
// (0 to 3 bytes specified by SPA, Stack Pointer Alignment)
// S = Set if CALLS; clear if CALLG.
// Z = Always cleared by CALL.  Can be set by software
//     force a reserved operand fault on a RET.

// Procdure Entry Mask
//
//  15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |DV|IV| MBZ |           Registers               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

// Entry Mask definitions
#define CALL_DV   0x8000
#define CALL_IV   0x4000
#define CALL_MBZ  0x3000
#define CALL_MASK 0x0FFF

// Call Stack Frame definitions (2nd longword)
#define CALL_SPA    0x03
#define CALL_S      (1 << CALL_P_S)
#define CALL_PSW    0x7FE0

#define CALL_P_SPA  30
#define CALL_P_S    29
#define CALL_P_MASK 16

// PUSHR/POPR Instruction Definitions
#define STK_MASK    0x7FFF // Registers Mask

// Register count table from vax_rcnt.c file.
extern const uchar vax_rcntTable[];

#define RCNT(mask) vax_rcntTable[mask]

// CALLG  Call Procedure with General Argument List
// CALLS  Call Procedure with Stack Argument List
// RET    Return from Procedure

inline void vax_Call(register VAX_CPU *vax, int32 *opnd, int flgStack)
{
	int32 pcAddr = opnd[1];
	int32 mask, tSP;
	int32 idx, tmp;
	int32 stklen;

	mask = ReadV(pcAddr, OP_WORD, RA);
	if (mask & CALL_MBZ)
		RSVD_OPND_FAULT;

	// Write check for page fault routines
	stklen = RCNT(mask & CALL_MASK) + (flgStack ? 24 : 20);
	ReadV(SP - stklen, OP_BYTE, WA);

	if (flgStack) {
		WriteV(SP - OP_LONG, opnd[0], OP_LONG, WA);
		SP -= OP_LONG;
	}

	tmp = ((SP & CALL_SPA) << CALL_P_SPA) |
	      (flgStack << CALL_P_S) | (PSL & CALL_PSW) |
			((mask & CALL_MASK) << CALL_P_MASK);
	tSP = SP & ~CALL_SPA;
	for (idx = nR11; idx >= nR0; idx--) {
		if ((mask >> idx) & 1) {
			WriteV(tSP - OP_LONG, RN(idx), OP_LONG, WA);
			tSP -= OP_LONG;
		}
	}
	WriteV(tSP - 4,  PC, OP_LONG, WA);
	WriteV(tSP - 8,  FP, OP_LONG, WA);
	WriteV(tSP - 12, AP, OP_LONG, WA);
	WriteV(tSP - 16, tmp, OP_LONG, WA);
	WriteV(tSP - 20, 0, OP_LONG, WA);

	AP = flgStack ? SP : opnd[0];
	SP = FP = tSP - 20;
	SET_PC(pcAddr + OP_WORD);
	PSL = (PSL & ~(PSW_DV|PSW_FU|PSW_IV)) |
	      ((mask & CALL_DV) ? PSW_DV : 0) |
	      ((mask & CALL_IV) ? PSW_IV : 0);
}

DEF_INST(vax, CALLG)
{
	vax_Call(vax, &OP0, 0);
	CC = 0;
}

DEF_INST(vax, CALLS)
{
	vax_Call(vax, &OP0, 1);
	CC = 0;
}

DEF_INST(vax, RET)
{
	int32 tSP = FP;
	int32 newPC, mask;
	int32 idx, tmp;
	int32 stklen;

	// Get stack entry mask
	mask = ReadV(tSP+4, OP_LONG, RA);
	if (mask & PSW_MBZ)
		RSVD_OPND_FAULT;

	// Access Check for Page Fault
	stklen = RCNT((mask >> CALL_P_MASK) & CALL_MASK) +
		((mask & CALL_S) ? 23 : 19);
#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int32 rmask = (mask >> CALL_P_MASK) & CALL_MASK;
		dbg_Printf("RET: Mask = %02X (Stack Length = %d)\n",
			rmask, RCNT(rmask));
		dbg_Printf("RET: Total Stack Length = %d (SP = %08X)\n",
			stklen, tSP + stklen);
	}
#endif /* DEBUG */
	ReadV(tSP + stklen, OP_BYTE, RA);

	// Restore PC, FP, AP, and SP registers from stack
	AP = ReadV(tSP+8, OP_LONG, RA);
	FP = ReadV(tSP+12, OP_LONG, RA);
	newPC = ReadV(tSP+16, OP_LONG, RA);
	tSP += 20;

	// Restore specific registers from stack
	tmp = mask >> CALL_P_MASK;
	for (idx = nR0; idx <= nR11; idx++) {
		if ((tmp >> idx) & 1) {
			RN(idx) = ReadV(tSP, OP_LONG, RA);
			tSP += OP_LONG;
		}
	}

	// Dealign stack pointer
	SP = tSP + ((mask >> CALL_P_SPA) & CALL_SPA);

	// Pop old argument list if CALLS is used.
	if (mask & CALL_S) {
		tmp = ReadV(SP, OP_LONG, RA);
		SP += ((tmp & BMASK) << 2) + 4;
	}

	// Reset PSW bits
	PSW = (PSW & ~PSW_MASK) | (mask & (PSW_MASK & ~PSW_CC));
	CC  = mask & PSW_CC;
	SET_PC(newPC);
}

// PUSHL  Push Longword

DEF_INST(vax, PUSHL)
{
	WriteV(SP - OP_LONG, OP0, OP_LONG, WA);
	SP -= OP_LONG;

	// Update condition codes
	CC_IIZP_L(OP0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("PUSHL: Stack %08X <= %08X\n", SP, OP0);
#endif /* DEBUG */
}

// PUSHR - Push Registers into Stack
// POPR  - Pop Registers from Stack

DEF_INST(vax, PUSHR)
{
	register int32 mask = OP0 & STK_MASK;
	register int32 idx, tsp = SP;

	// Access check for page fault traps
	ReadV(SP - RCNT(mask), OP_BYTE, WA);

	// Push registers into stack
	for (idx = nSP; idx >= nR0; idx--) {
		if ((mask >> idx) & 1) {
			WriteV(tsp - OP_LONG, RN(idx), OP_LONG, WA);
			tsp -= OP_LONG;
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA))
				dbg_Printf("PUSHR: R%d %08X => (SP) %08X\n", idx, RN(idx), tsp);
#endif /* DEBUG */
		}
	}
	SP = tsp;
}

DEF_INST(vax, POPR)
{
	register int32 mask = OP0 & STK_MASK;
	register int32 idx;

	// Access check for page fault traps
	ReadV(SP + RCNT(mask) - 1, OP_BYTE, RA);

	for (idx = nR0; idx <= nSP; idx++) {
		if ((mask >> idx) & 1) {
			RN(idx) = ReadV(SP, OP_LONG, RA);
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA))
				dbg_Printf("POPR: R%d %08X <= (SP) %08X\n", idx, RN(idx), SP);
#endif /* DEBUG */
			if (idx < nSP)
				SP += OP_LONG; // If pop SR, no increment.
		}
	}
}
