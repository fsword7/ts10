// cpu_main.c - the KS10 Processor emulation routines
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

#include <time.h>

#include "pdp10/defs.h"
#include "pdp10/ks10.h"
#include "pdp10/iodefs.h"

#define IMM  ((int36)eAddr)
#define IMMS ((int36)eAddr << 18)

// Instruction/Opcode Table Defintions

extern INSTRUCTION pdp10_Instruction[];

P10_CONSOLE *p10_Console = NULL;
P10_CPU     *p10 = NULL;

void (*basOpcode[01000])(); // Basic Instruction Table
int  (*extOpcode[01000])(); // EXTEND Instruction Table
INSTRUCTION *pdp10_Opcode[01000];
INSTRUCTION *pdp10_OpcodeEXT[01000];
INSTRUCTION *pdp10_OpcodeIO[02000];
INSTRUCTION *pdp10_OpcodeDEV[0200];
INSTRUCTION *pdp10_OpcodeFUNC[010];

/* Instruction per second meter */
time_t  last, now;
int     cips = 0;
int     count_ints = 0;
jmp_buf p10_SetJump;
int     p10_State;
int     p10_pid;

// KS10 Processor Registers
int36 HR    = 0; // Instruction Register (36 bits)
int36 AR    = 0; // Arithmetic Register (36 bits)
int36 ARX   = 0; // Arithmetic Register (36 bits)
int36 BR    = 0; // Buffer Register (36 bits)
int36 BRX   = 0; // Buffer Register (36 bits)
int36 CR    = 0; // C Register (36 bits)
int36 CRX   = 0; // C Register (36 bits)
int30 PC    = 0; // Program Counter (18 bits) <18:35> of PC
int36 FLAGS = 0; // Status Register (18 buts) <0:17> of PC
int36 T0    = 0; // Temporary #0
int36 T1    = 0; // Temporary #1 (HALT code)

int   XCT   = 0; // XCT occurs

int     cpu_pFlags;    // Processor Flags
int32   KX10_IntrQ;    // IRQ (Interrupt Request)
int     KX10_IsGlobal; // E Global or Local (1 = Global, 0 = Local)
int30   p10_Section;

// PXCT switch variables
int srcMode;   // Source for BLT
int dstMode;   // Destination for BLT
int eaMode;    // Effective Address
int stackMode; // Stack Data
int dataMode;  // Memory Data
int byteMode;  // Byte Data

int savedMode;        // saved debug modes;
int savedModePFT;     // saved debug modes;

int36 eaFlags; // Flags for Effective Address (JRSTF and JEN)

int36 *pData;  // current address of AC/Memory for write test.

// Trap routines for desired processor
int30 (*KX10_CalcEffAddr)(int30, int30, int);
int   (*KX10_Trap_NoMemory)(int30, int) = NULL;
int   (*KX10_PageTrap1)(int);
void  (*KX10_PageTrap2)(void);
void  (*KX10_piEvaluate)(void);
void  (*KX10_piProcess)(void);

extern int (*KX10_PageRefill)(uint30, uint30 *, int);
extern int30 KL10_uptAddr, KL10_eptAddr;
extern int32 ts10_ClkInterval;

// Fields of Instruction Code
int18 opDevice;   // (I/O)   Device Code       (DEV)
int18 opFunction; // (I/O)   Function Code     (FUNC)
int18 opCode;     // (Basic) Opcode field      (OP)
int18 opAC;       // (Basic) Accumulator       (AC)
int18 opIndirect; // (Both)  Indirect          (I)
int18 opIndex;    // (Both)  Index Register    (X)
int18 opAddr;     // (Both)  Address           (Y)
int36 eAddr;      // (Both)  Effective Address (E)
//int30 eAddr;      // (Both)  Effective Address (E)

void p10_ResetCPU(void)
{
	p10_ResetPI();     // Priority Interrupt System
	p10_ResetAPR();    // Arithmetic Processor
	p10_ResetPager();  // Paging System
	p10_ResetMemory(); // Main Memory System

	cpu_pFlags = 0;
	KX10_IntrQ = 0;
	
	p10_Section = 0;
	FLAGS = 0;
	PC    = 0;
}

// Set Program Flags
void cpu_SetFlags(int36 newFlags)
{
	if (FLAGS & FLG_USER) {
		newFlags |= FLG_USER;
		if (!(FLAGS & FLG_USERIO))
			newFlags &= ~FLG_USERIO;
	}

	FLAGS = newFlags & PC_FLAGS;
}

// Calculate for regular effective address
// for other processors.
int18 KS10_CalcEffAddr(int30 dummy, int32 data, int eaMode)
{
	// I, XR, and ADDR fields clearly fit a 32-bit value.
	// It uses 32-bit values for much better optimization.

	register int32 Indirect;
	register int32 Index;
	register int18 eAddr;

	do {
		Indirect = INST_GETI(data);
		Index    = INST_GETX(data);
		eAddr    = INST_GETY(data);

		if (Index)
			eAddr = VMA(eAddr + (eaMode ? prvAC : curAC)[Index]);

		if (Indirect) {
			data = p10_vRead(eAddr, eaMode);
//			if (KX10_IntrQ)
//				emu_Abort(p10_SetJump, P10_INTERRUPT);
		}

	} while (Indirect);

	return eAddr;
}

// Calculate for JRSTF address
int36 p10_CalcJumpAddr(int36 jData, int eaMode)
{
	int18 Indirect;
	int18 Index;
	int36 jAddr;
	int36 jTemp;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_OPERAND))
		dbg_Printf("EA: Start: %06llo,,%06llo\n", LHSR(jData), RH(jData));
#endif /* DEBUG */

	do {
		Indirect = INST_GETI(jData);
		Index    = INST_GETX(jData);
		jAddr    = jData;
		
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_OPERAND))
			dbg_Printf("EA: I: %s XR: %02o Address: %06llo\n",
				(Indirect ? "Yes" : "No "), Index, RH(jAddr));
#endif /* DEBUG */

		if (Index) {
			jTemp = p10_vRead(Index, eaMode);

#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_OPERAND))
				dbg_Printf("EA:   XR[%d,%02o] => %06llo,,%06llo\n", 
					(eaMode ? PACB : CACB), Index, LHSR(jTemp), RH(jTemp));
#endif /* DEBUG */
			jAddr = VMA(jAddr) + jTemp;
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_OPERAND))
				dbg_Printf("EA:   + XR => %06llo,,%06llo\n",
					LHSR(jAddr), RH(jAddr));
#endif /* DEBUG */
		}

		if (Indirect) {
			jData = p10_vRead(VMA(jAddr), eaMode);
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_OPERAND))
				dbg_Printf("EA:   M[%06llo] => %06llo,,%06llo\n",
					VMA(jAddr), LHSR(jData), RH(jData));
#endif /* DEBUG */
//			if (KX10_IntrQ)
//				emu_Abort(p10_SetJump, P10_INTERRUPT);
		}

	} while (Indirect);
	
#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_OPERAND))
		dbg_Printf("EA: Result: %06llo,,%06llo\n", LHSR(jAddr), RH(jAddr));
#endif /* DEBUG */

	return jAddr;
}

int p10_CheckPXCT(int prvFlags)
{
	int newMode = 0;

	if (cpu_pFlags & prvFlags) {
		newMode = PTF_PREV;
		if (FLAGS & FLG_PCU)
			newMode |= PTF_USER;
	}

	return newMode;
}

// Execute interrupt instruction
void KL10_piExecute(int30 piAddr)
{
	HR = p10_pRead(piAddr, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE) || dbg_Check(DBG_INTERRUPT))
		p10_Disassemble(piAddr, HR, 0);
#endif /* DEBUG */

	// Count each instruction for measuring instructions per second
	cips++;

	opCode = INST_GETOP(HR);
	opAC   = INST_GETAC(HR);

	eAddr = KX10_CalcEffAddr(piAddr, HR, cpu_pFlags & PXCT_EA);

	basOpcode[opCode]();
}

// Execute instruction at specific address once.
void p10_Execute(int30 xAddr, int mode)
{
	// Execute trap instruction when TRAP1 or/and TRAP2
	// flags had been set in PC Flags and pager turned on.
	if (KX10_Pager_On && (FLAGS & FLG_TRAPS)) {
		int30 trapAddr;
		int   trapFlag;

		// Temp. for testing
		p10_Access(xAddr, 0);

		// Calcuate trap address to being executed
		trapFlag = (FLAGS & FLG_TRAPS) >> 25;
		if (ISCPU(CNF_KL10)) {
			trapAddr = ((FLAGS & FLG_USER) ?
				(KL10_uptAddr + UPT_TR_BASE) : (KL10_eptAddr + EPT_TR_BASE)) + trapFlag;
		} else {
			trapAddr = ((FLAGS & FLG_USER) ?
				(uptAddr + UPT_TR_BASE) : (eptAddr + EPT_TR_BASE)) + trapFlag;
		}

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("CPU: *** Trap %d at PC %06o\n", trapFlag, RH(PC - 1));
#endif /* DEBUG */

		// Save flags for page fail trap then clear all TRAP flags.
		// Set TRAP cycle flag on processor flags
		cpu_pFlags |= CPU_CYCLE_TRAP;
		pager_Flags = FLAGS;
		FLAGS &= ~FLG_TRAPS;

		// Now fetch the trap instruction from trap address.
		HR = p10_pRead(trapAddr, 0);

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE))
			p10_Disassemble(trapAddr, HR, 0);
#endif /* DEBUG */

	} else {
		HR = p10_vRead(xAddr, mode);

		if ((cpu_pFlags & (CPU_CYCLE_XCT|CPU_CYCLE_UUO)) == 0)
			PC = p10_Section | VMA(PC + 1);

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE))
			p10_Disassemble(xAddr, HR, 0);
#endif /* DEBUG */
	}

//	if (pdp10_Opcode[opCode]->Flags & OP_AC)
//		pdp10_Opcode[opCode]->AC[opAC]->useCount++;
//	else
//		pdp10_Opcode[opCode]->useCount++;

	// Count each instruction for measuring instructions per second
	cips++;

	opCode = INST_GETOP(HR);
	opAC   = INST_GETAC(HR);

	eAddr = KX10_CalcEffAddr(xAddr, HR, cpu_pFlags & PXCT_EA);

	basOpcode[opCode]();
}

/************* PDP10 Instructions **************/

// Opcode 000 Series

// Local UUO Instructions - Opcode 001-037
void KS10_Opcode_LUUO(void)
{
	// Set UUO word - Opcode, AC, and E.
	AR = (LH(HR) | RH(eAddr)) & LUUO_M_OPCODE;

	// Save UUO word at PC 40 and execute branch
	// instruction at PC 41 once.
	p10_vWrite(040, AR, 0);
	cpu_pFlags |= CPU_CYCLE_UUO;
	p10_Execute(041, 0);
}

// Monitor UUO Instructions - Opcode 000, 040-777
void KS10_Opcode_UUO(void)
{
//	printf("CPU: UUO code %03o at PC %06llo\r\n", opCode, RH(PC));

	if (EBR & PG_EBR_M_TOPS20_PAGING) {
		// For TOPS-20 operating system
		p10_pWrite(uptAddr + MUUO_T20_OPCODE,
			LH(FLAGS & ~FLG_TRAPS) | LHSR(HR & MUUO_M_OPCODE), 0);
		p10_pWrite(uptAddr + MUUO_T20_OLD_PC, PC, 0);
		p10_pWrite(uptAddr + MUUO_T20_OLD_E,  eAddr, 0);
		p10_pWrite(uptAddr + MUUO_T20_PCW,    UBR, 0);
	} else {
		// For TOPS-10 operating system
		p10_pWrite(uptAddr + MUUO_T10_OPCODE, HR & MUUO_M_OPCODE, 0);
		p10_pWrite(uptAddr + MUUO_T10_OLD_PC,
			LH(FLAGS & ~FLG_TRAPS) | RH(PC), 0);
		p10_pWrite(uptAddr + MUUO_T10_PCW,    UBR, 0);
	}

	// Jump into monitor for MUUO or Illegal Instructions
	BR = MUUO_NEW_PC_BASE;
	if (FLAGS & FLG_TRAPS)
		BR |= MUUO_M_TRAP; // Bump to TRAP area
	if (FLAGS & FLG_USER)
		BR |= MUUO_M_USER; // Bump to USER area
	BR = p10_pRead(uptAddr + BR, 0);

	// Allow user to load new Flags and PC
	FLAGS = LH(BR) | ((FLAGS & FLG_USER) ? FLG_PCU : 0);
	DO_JUMP(RH(BR));
}

// Opcode 100 Series

// 105 ADJSP - Adjust Statck Pointer
void p10_Opcode_ADJSP(void)
{
	// if L[PC] = 0 or (AC)[0,6:17] <= 0: (AC) + (+-)R[E],R[E] -> (AC)
	// if L[PC] != 0 and (AC)[0,6:17] > 0: (AC) + (+-)R[E] -> (AC)

	AR    = curAC[opAC];
	eAddr = SXT18(eAddr);
	if ((p10_Section > 0) && (XLH(AR) > 0))
		BR = AR + eAddr;
	else
		BR = LHSXT(AR + (eAddr << 18)) | RH(AR + eAddr);
	curAC[opAC] = BR;

	if (((BR ^ AR) & (~BR ^ (eAddr << 18))) & WORD36_SIGN)
		FLAGS |= FLG_TRAP2;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ADJSP: %06o,,%06o + %o,,%o = %06o,,%06o\n",
			LH18(AR), RH18(AR), RH18(eAddr), RH18(eAddr), LH18(BR), RH18(BR));
#endif /* DEBUG */
}

// *************** 72-bit Arithmetic ************

// 114 DADD - Double Add
// 115 DSUB - Double Subtract
// 116 DMUL - Double Multiply
// 117 DDIV - Double Divide

void p10_Opcode_DADD(void)
{
	// (AC,AC+1) + (E,E+1) -> (AC,AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	BRX = p10_vRead(VMA(eAddr + 1), dataMode);
	p10_dpAdd(&curAC[opAC], &curAC[AC(opAC + 1)], BR, BRX);
}

void p10_Opcode_DSUB(void)
{
	// (AC,AC+1) + (E,E+1) -> (AC,AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	BRX = p10_vRead(VMA(eAddr + 1), dataMode);
	p10_dpSubtract(&curAC[opAC], &curAC[AC(opAC + 1)], BR, BRX);
}

void p10_Opcode_DMUL(void)
{
	// (AC,AC+1) * (E,E+1) -> (AC,AC+1,AC+2,AC+3)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	BRX = p10_vRead(VMA(eAddr + 1), dataMode);
	p10_qpMultiply(&curAC[opAC], &curAC[AC(opAC + 1)],
		&curAC[AC(opAC + 2)], &curAC[AC(opAC + 3)], BR, BRX);
}

void p10_Opcode_DDIV(void)
{
	// (AC,AC+1,AC+2,AC+3) / (E,E+1) -> (AC,AC+1) R (AC+2,AC+3)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	BRX = p10_vRead(VMA(eAddr + 1), dataMode);
	p10_qpDivide(&curAC[opAC], &curAC[AC(opAC + 1)],
		&curAC[AC(opAC + 2)], &curAC[AC(opAC + 3)], BR, BRX);
}

// *************** 72-bit Move **************

// 120 DMOVE  - Double Move
// 121 DMOVN  - Double Move Negative
// 124 DMOVEM - Double Move to Memory
// 125 DMOVNM - Double Move Negative to Memory

void p10_Opcode_DMOVE(void)
{
	// (E,E+1) -> (AC,AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	ARX = p10_vRead(VMA(eAddr + 1), dataMode);
	curAC[opAC] = AR;
	curAC[AC(opAC+1)] = ARX;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		if (eAddr < 020)
			dbg_Printf("DMOVE: AC[%d,%02o]", dataMode ? PACB : CACB,
				eAddr);
		else
			dbg_Printf("DMOVE: %s E[%06llo]", dataMode ? "User" : "Exec",
				RH(eAddr));
		dbg_Printf(" -> AC[%d,%02o]: %06llo,,%06llo\n",
			CACB, opAC, LHSR(AR), RH(AR));

		if (RH(eAddr + 1) < 020)
			dbg_Printf("DMOVE: AC[%d,%02o]", dataMode ? PACB : CACB,
				RH(eAddr + 1));
		else
			dbg_Printf("DMOVE: %s E[%06llo]", dataMode ? "User" : "Exec",
				RH(eAddr + 1));
		dbg_Printf(" -> AC[%d,%02o]: %06llo,,%06llo\n",
			CACB, AC(opAC+1), LHSR(ARX), RH(ARX));
	}
#endif /* DEBUG */
}

void p10_Opcode_DMOVN(void)
{
	// -(E,E+1) -> (AC,AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	ARX = p10_vRead(VMA(eAddr + 1), dataMode);
	p10_dpNegate(&AR, &ARX);
	curAC[opAC] = AR;
	curAC[AC(opAC+1)] = ARX;
}

void p10_Opcode_DMOVEM(void)
{
	// (AC,AC+1) -> (E,E+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(VMA(eAddr + 1), dataMode | PTF_WRITE);
	AR  = curAC[opAC];
	ARX = curAC[AC(opAC+1)];
	p10_vWrite(eAddr, AR, dataMode);
	*pData = ARX;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("DMOVEM: AC[%d,%02o]", CACB, opAC);
		if (eAddr < 020)
			dbg_Printf(" -> AC[%d,%02o]", dataMode ? PACB : CACB, eAddr);
		else
			dbg_Printf(" -> %s E[%06llo]", dataMode ? "User" : "Exec", RH(eAddr));
		dbg_Printf(": %06llo,,%06llo\n", LHSR(AR), RH(AR));

		dbg_Printf("DMOVEM: AC[%d,%02o]", CACB, (opAC + 1) & 017);
		if (RH(eAddr + 1) < 020)
			dbg_Printf(" -> AC[%d,%02o]", dataMode ? PACB : CACB,
				RH(eAddr + 1));
		else
			dbg_Printf(" -> %s E[%06llo]", dataMode ? "User" : "Exec",
				RH(eAddr + 1));
		dbg_Printf(": %06llo,,%06llo\n", LHSR(ARX), RH(ARX));
	}
#endif /* DEBUG */
}

void p10_Opcode_DMOVNM(void)
{
	// -(AC,AC+1) -> (E,E+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(RH(eAddr + 1), dataMode | PTF_WRITE);
	AR  = curAC[opAC];
	ARX = curAC[AC(opAC + 1)];
	p10_dpNegate(&AR, &ARX);
	p10_vWrite(eAddr, AR, dataMode);
	*pData = ARX;
}

// ******************* Byte Pointer *************

#if 0
// Get a byte pointer
void klx_GetBP(void)
{
	int36 bp;
	uint32 Pos, Size;

	// Get a byte pointer with its content.
	bp = *(pBP = p10_Access(eAddr, dataMode | PTF_WRITE));

	// Extract position and size fields from a byte pointer.
	Pos  = BP_GETPOS(bp);
	Size = BP_GETSIZE(bp);

	if (Pos > 36) {
		// One-word global byte pointer

	} else if (p10_Section > 0) {
	}

		// One-word local byte pointer
	}
}
#endif

// 133 IBP   - Increment Byte Pointer  AC #0
// 133 ADJBP - Adjust Byte Pointer     AC #1-17
void p10_Opcode_IBP(void)
{
 	// if AC = 0:  go IBP instruction
	// if AC != 0: go ADJBP instruction

	int36 Pos;  // Position field of the byte pointer
	int36 Size; // Size field of the byte pointer
	int36 *pBP; // Address of byte pointer

	// Get PXCT switch for Memory data
	dataMode = p10_CheckPXCT(PXCT_DATA);

	if (opAC) {
		/* ADJBP Instruction (AC != 0) */
		int36 adjByte, adjWord;
		int36 leftPos, bpWord, newByte;
		int36 ac, PTR;

		/* Array operations on pointer in E or E,E+1 */
		/* Let A = Remainder ( (36 - P) / S ) */
		/* if S > 36 - A: 1 -> NO DIVIDE */
		/* if S = 0: (E) -> (AC) or (E,E+1) -> (AC,AC+1) */
		/* if 0 < S < 36-A: make copy C of (E) or (E,E+1) */
		/*    Complete (AC) + ((36-P)/S) = Q * Bytes/Word + R */
		/*    1 <= R <= [Bytes/Word = ((36-P)/S)+(P/S)] */
		/*    Y[C] + Q -> Y[C] */
		/*    36 - (R*S) - A -> P[C] */
		/*    C -> (AC) or (AC,AC+1) */

		// Get a byte pointer with its content.
		// AR = *(pBP = p10_Access(eAddr, dataMode | PTF_WRITE));
		AR = p10_vRead(eAddr, dataMode);

		/* Extract position and size fields from a byte pointer */
		Pos  = BP_GETPOS(AR);
		Size = BP_GETSIZE(AR);
		PTR  = BP_GETY(AR);

		ac = curAC[opAC];

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("ADJBP: Instruction at PC %06o\n", pager_PC);
			dbg_Printf("ADJBP: AC = %c%012llo E = %c%012llo BP = %c%012llo\n",
				(ac < 0 ? '-' : '+'), ac & WORD36_ONES,
				(eAddr < 0 ? '-' : '+'), eAddr & WORD36_ONES,
				(AR < 0 ? '-' : '+'), AR & WORD36_ONES);
			dbg_Printf("ADJBP: Old P = %lld PTR = %06llo ADJ = %lld\n",
				Pos, PTR, ac);
		}
#endif /* DEBUG */

		if (Size) {
			leftPos = (36 - Pos) / Size;
			bpWord  = leftPos + (Pos / Size);
			if (bpWord == 0) {
				FLAGS |= (FLG_AROV|FLG_TRAP1|FLG_DCX);
				return;
			}
			newByte = ac + leftPos;
			adjByte = (newByte >= 0) ? newByte % bpWord : -(-newByte % bpWord);
			adjWord = newByte / bpWord;
			if (adjByte <= 0) {
				adjWord -= 1;
				adjByte += bpWord;
			}
			PTR += adjWord;
			Pos = (36 - (adjByte * Size)) + ((36 - Pos) % Size);

			/* Replaced position field in a pointer */
			AR = (AR & ~(BP_POS|BP_LADDR))
				| ((Pos & BP_M_POS) << BP_P_POS) | (PTR & BP_LADDR);

#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA)) {
				dbg_Printf("ADJBP: New P = %lld PTR = %06llo\n", Pos, PTR);
				dbg_Printf("ADJBP: New BP = %c%012llo\n",
					(AR < 0 ? '-' : '+'), AR & WORD36_ONES);
			}
#endif /* DEBUG */

		}
		curAC[opAC] = SXT36(AR);
	} else {
		// IBP Instruction (AC = 0)

		// Linear operations on pointer in E or E,E+1
		// if P - S >= 0: P - S -> P
		// if P - S < 0: Y + 1 = Y, 36 - S -> P

		// Get a byte pointer with its content.
		AR = *(pBP = p10_Access(eAddr, dataMode | PTF_WRITE));

		// Extract position and size fields from a byte pointer
		Pos  = BP_GETPOS(AR);
		Size = BP_GETSIZE(AR);

		// Increment a byte pointer
		if ((Pos -= Size) < 0) {
			AR = LH(AR) | RH(AR+1);
			Pos = (36 - Size) & BP_M_POS;
		}

		// Update a byte pointer for new position
		AR = (Pos << BP_P_POS) | (AR & ~BP_POS);
		*pBP = SXT36(AR);
	}
}

// 134 ILDB - Increment Pointer and Load Byte
void p10_Opcode_ILDB(void)
{
	// Increment a byte pointer.
	// Byte in ((E)) -> (AC)

	int36 P;    // Position field of the byte pointer
	int36 S;    // Size field of the byte pointer
	int36 Mask; // Bit mask where position and size are
	int36 *pBP; // Address of byte pointer

	// Get PXCT switches for Memory and Byte data
	dataMode = p10_CheckPXCT(PXCT_DATA);
	byteMode = p10_CheckPXCT(PXCT_BP_DATA);
	eaMode   = p10_CheckPXCT(PXCT_BP_EA);

	// Get a byte pointer with its content.
	AR = *(pBP = p10_Access(eAddr, dataMode | PTF_WRITE));

	// Extract position and size fields from a byte pointer.
	P = BP_GETPOS(AR);
	S = BP_GETSIZE(AR);

	if ((FLAGS & FLG_FPD) == 0) {

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("BP: Old - Pos: %lld Size: %lld Address: %06llo\n",
				P, S, RH(AR));
#endif /* DEBUG */

#ifdef DEBUG
		if (P > 36)
			dbg_Printf("BP: One-Word Global Byte Pointer at PC %o,,%06o.\n",
				LH18(pager_PC), RH18(pager_PC));
		if (AR & BP_GLOBAL)
			dbg_Printf("BP: Two-Word Global Byte Pointer at PC %o,,%06o.\n",
				LH18(pager_PC), RH18(pager_PC));
#endif /* DEBUG */

		// Increment a byte pointer
		if ((P -= S) < 0) {
			AR = LH(AR) | RH(AR+1);
			P = (36 - S) & BP_M_POS;
		}

		// Update a byte pointer for new position.
		AR = (P << BP_P_POS) | (AR & ~BP_POS);
		*pBP = SXT36(AR);

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("BP: New - Pos: %lld Size: %lld Address: %06llo\n",
				P, S, RH(AR));
#endif /* DEBUG */

		// Set FPD (First Part Done) flag to system flags
		FLAGS |= FLG_FPD;
	}

	// Get a 36-bit word where a byte pointer points to.
	BR = KX10_CalcEffAddr(eAddr, AR, eaMode);
	BRX = p10_vRead(BR, byteMode);

	// Extract a byte from a 36-bit word.
	AR = ((BRX & WORD36_ONES) >> P) & ((1LL << (S > 36 ? 36 : S)) - 1);
	curAC[opAC] = SXT36(AR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BP: %c%012llo -> %c%012llo\n",
			((BRX < 0) ? '-' : '+'), BRX & WORD36_ONES,
			((AR < 0) ? '-' : '+'), AR & WORD36_ONES);
#endif /* DEBUG */

	// When process is done, reset FPD flag on system flags.
	FLAGS &= ~FLG_FPD;
}

// 135 LDB - Load Byte
void p10_Opcode_LDB(void)
{
	// Byte in ((E)) -> (AC)

	int36 P;    // Position field of the byte pointer
	int36 S;    // Size field of the byte pointer
	int36 Mask; // Bit mask where position and size are

	// Get PXCT switches for Memory and Byte data.
	dataMode = p10_CheckPXCT(PXCT_DATA);
	byteMode = p10_CheckPXCT(PXCT_BP_DATA);
	eaMode   = p10_CheckPXCT(PXCT_BP_EA);

	// Get a byte pointer.
	AR = p10_vRead(eAddr, dataMode);

	// Extract position and size fields from a byte pointer.
	P = BP_GETPOS(AR);
	S = BP_GETSIZE(AR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BP: Current - Pos: %lld Size: %lld Address: %06llo\n",
			P, S, RH(AR));
#endif /* DEBUG */

#ifdef DEBUG
	if (P > 36)
		dbg_Printf("BP: One-Word Global Byte Pointer at PC %o,,%06o.\n",
			LH18(pager_PC), RH18(pager_PC));
	if (AR & BP_GLOBAL)
		dbg_Printf("BP: Two-Word Global Byte Pointer at PC %o,,%06o.\n",
			LH18(pager_PC), RH18(pager_PC));
#endif /* DEBUG */

	// Get a 36-bit word where a byte pointer points to.
	BR = KX10_CalcEffAddr(eAddr, AR, eaMode);
	BRX = p10_vRead(BR, byteMode);

	// Extract a byte from a 36-bit word.
	AR = ((BRX & WORD36_ONES) >> P) & ((1LL << (S > 36 ? 36 : S)) - 1);
	curAC[opAC] = SXT36(AR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BP: %c%012llo -> %c%012llo\n",
			((BRX < 0) ? '-' : '+'), BRX & WORD36_ONES,
			((AR < 0) ? '-' : '+'), AR & WORD36_ONES);
#endif /* DEBUG */
}

// 136 IDPB - Increment Pointer and Deposit Byte
void p10_Opcode_IDPB(void)
{
	// Increment a byte pointer.
	// Byte in (AC) -> Byte in ((E))

	int36 P;    // Position field of the byte pointer
	int36 S;    // Size field of the byte pointer
	int36 Mask; // Bit mask where position and size are
	int36 *pBP; // Address of byte pointer

	// Get PXCT switches for Memory and Byte data
	dataMode = p10_CheckPXCT(PXCT_DATA);
	byteMode = p10_CheckPXCT(PXCT_BP_DATA);
	eaMode   = p10_CheckPXCT(PXCT_BP_EA);

	// Get a byte pointer with its content.
	AR = *(pBP = p10_Access(eAddr, dataMode | PTF_WRITE));

	// Extract position and size fields from a byte pointer
	P = BP_GETPOS(AR);
	S = BP_GETSIZE(AR);

	if ((FLAGS & FLG_FPD) == 0) {

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("BP: Old - Pos: %lld Size: %lld Address: %06llo\n",
				P, S, RH(AR));
#endif /* DEBUG */

#ifdef DEBUG
		if (P > 36)
			dbg_Printf("BP: One-Word Global Byte Pointer at PC %o,,%06o.\n",
				LH18(pager_PC), RH18(pager_PC));
		if (AR & BP_GLOBAL)
			dbg_Printf("BP: Two-Word Global Byte Pointer at PC %o,,%06o.\n",
				LH18(pager_PC), RH18(pager_PC));
#endif /* DEBUG */

		// Increment a byte pointer
		if ((P -= S) < 0) {
			AR = LH(AR) | RH(AR+1);
			P = (36 - S) & BP_M_POS;
		}

		// Update a byte pointer for new position.
		AR = (P << BP_P_POS) | (AR & ~BP_POS);
		*pBP = SXT36(AR);

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("BP: New - Pos: %lld Size: %lld Address: %06llo\n",
				P, S, RH(AR));
#endif /* DEBUG */

		// Set FPD (First Part Done) flag to system flags
		FLAGS |= FLG_FPD;
	}

	// Get a 36-bit word where a byte pointer points to.
	BR = KX10_CalcEffAddr(eAddr, AR, eaMode);

	AR = p10_vRead(BR, byteMode);
	ARX = curAC[opAC];

	// Deposit a byte into a 36-bit word.
	Mask = ((1LL << (S > 36 ? 36 : S)) - 1) << P;
	AR = (AR & ~Mask) | ((ARX << P) & Mask);
	AR = SXT36(AR);

	// Update a new 36-bit word back to memory.
	p10_vWrite(BR, AR, byteMode);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BP: %c%012llo <- %c%012llo\n",
			((AR < 0) ? '-' : '+'), AR & WORD36_ONES,
			((ARX < 0) ? '-' : '+'), ARX & WORD36_ONES);
#endif /* DEBUG */

	// When process is done, reset FPD flag on system flags.
	FLAGS &= ~FLG_FPD;
}

// 137 DPB - Deposit Byte
void p10_Opcode_DPB(void)
{
	// Byte in (AC) -> Byte in ((E))

	int36 P;    // Position field of the byte pointer
	int36 S;    // Size field of the byte pointer
	int36 Mask; // Bit mask

	// Get PXCT switches for Memory and Byte data
	dataMode = p10_CheckPXCT(PXCT_DATA);
	byteMode = p10_CheckPXCT(PXCT_BP_DATA);
	eaMode   = p10_CheckPXCT(PXCT_BP_EA);

	// Get a byte pointer
	AR = p10_vRead(eAddr, dataMode);

	// Extract position and size fields from a byte pointer
	P = BP_GETPOS(AR);
	S = BP_GETSIZE(AR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BP: Current - Pos: %lld Size: %lld Address: %06llo\n",
			P, S, RH(AR));
#endif /* DEBUG */

#ifdef DEBUG
	if (P > 36)
		dbg_Printf("BP: One-Word Global Byte Pointer at PC %o,,%06o.\n",
			LH18(pager_PC), RH18(pager_PC));
	if (AR & BP_GLOBAL)
		dbg_Printf("BP: Two-Word Global Byte Pointer at PC %o,,%06o.\n",
			LH18(pager_PC), RH18(pager_PC));
#endif /* DEBUG */

	// Get a 36-bit word where a byte pointer points to.
	BR = KX10_CalcEffAddr(eAddr, AR, eaMode);

	AR = p10_vRead(BR, byteMode);
	ARX = curAC[opAC];

	// Deposit a byte into a 36-bit word.
	Mask = ((1LL << (S > 36 ? 36 : S)) - 1) << P;
	AR = (AR & ~Mask) | ((ARX << P) & Mask);
	AR = SXT36(AR);

	// Update a new 36-bit word back to memory.
	p10_vWrite(BR, AR, byteMode);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BP: %c%012llo <- %c%012llo\n",
			((AR < 0) ? '-' : '+'), AR & WORD36_ONES,
			((ARX < 0) ? '-' : '+'), ARX & WORD36_ONES);
#endif /* DEBUG */
}

/* Opcode 200 Series */

// ************** Move *************

// 200 MOVE  - Move 
// 201 MOVEI - Move Immediate
// 202 MOVEM - Move to Memory
// 203 MOVES - Move to Self

void p10_Opcode_MOVE(void)
{
	// (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	curAC[opAC] = p10_vRead(eAddr, dataMode);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA)) {
		if (eAddr < 020)
			dbg_Printf("MOVE: AC[%d,%02o]", dataMode ? PACB : CACB, eAddr);
		else
			dbg_Printf("MOVE: %s E[%06llo]", dataMode ? "User" : "Exec", RH(eAddr));
		dbg_Printf(" -> AC[%d,%02o]: %06llo,,%06llo\n",
			CACB, opAC, LHSR(curAC[opAC]), RH(curAC[opAC]));
	}
#endif /* DEBUG */
}

void p10_Opcode_MOVEI(void)
{
	// 0,,E -> (AC)

	curAC[opAC] = RH(eAddr);
}

void p10_Opcode_MOVEM(void)
{
	// (AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	p10_vWrite(eAddr, curAC[opAC], dataMode);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA)) {
		dbg_Printf("MOVEM: AC[%d,%02o]", CACB, opAC);
		if (eAddr < 020)
			dbg_Printf(" -> AC[%d,%02o]", dataMode ? PACB : CACB, eAddr);
		else
			dbg_Printf(" -> %s E[%06llo]", dataMode ? "User" : "Exec", RH(eAddr));
		dbg_Printf(": %06llo,,%06llo\n", LHSR(curAC[opAC]), RH(curAC[opAC]));
	}
#endif /* DEBUG */
}

void p10_Opcode_MOVES(void)
{
	// (E) -> (E)
	// if AC is non-zero: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}

// ************** Move Swapped *************

// 204 MOVS  - Move Swapped
// 205 MOVSI - Move Swapped Immediate
// 206 MOVSM - Move Swapped to Memory
// 207 MOVSS - Move Swapped to Self

void p10_Opcode_MOVS(void)
{
	// S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	BR = SWAP36(AR);
	BR = SXT36(BR);
	curAC[opAC] = BR;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVS: %c%06llo,,%06llo => %c%06llo,,%06llo\n",
			((AR < 0) ? '-' : '+'), LHSR(AR), RH(AR),
			((BR < 0) ? '-' : '+'), LHSR(BR), RH(BR));
#endif /* DEBUG */
}

void p10_Opcode_MOVSI(void)
{
	// E,,0 -> (AC)

	AR = RH(eAddr);
	BR = SWAP36(AR);
	curAC[opAC] = SXT36(BR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVS: %c%06llo,,%06llo => %c%06llo,,%06llo\n",
			((AR < 0) ? '-' : '+'), LHSR(AR), RH(AR),
			((BR < 0) ? '-' : '+'), LHSR(BR), RH(BR));
#endif /* DEBUG */
}

void p10_Opcode_MOVSM(void)
{
	// S(AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

//	p10_vWrite(eAddr, SWAP36(curAC[opAC]), dataMode);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR = curAC[opAC];
	BR = SWAP36(AR);
	BR = SXT36(BR);
	*pData = BR;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVS: %c%06llo,,%06llo => %c%06llo,,%06llo\n",
			((AR < 0) ? '-' : '+'), LHSR(AR), RH(AR),
			((BR < 0) ? '-' : '+'), LHSR(BR), RH(BR));
#endif /* DEBUG */
}

void p10_Opcode_MOVSS(void)
{
	// S(E) -> (E)
	// if AC is non-zero: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR = *pData;
	BR = SWAP36(AR);
	BR = SXT36(BR);
	*pData = BR;
	if (opAC)
		curAC[opAC] = BR;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVS: %c%06llo,,%06llo => %c%06llo,,%06llo\n",
			((AR < 0) ? '-' : '+'), LHSR(AR), RH(AR),
			((BR < 0) ? '-' : '+'), LHSR(BR), RH(BR));
#endif /* DEBUG */
}

// ************** Move Negative *************

// 210 MOVN  - Move Negative
// 211 MOVNI - Move Negative Immediate
// 212 MOVNM - Move Negative to Memory
// 213 MOVNS - Move Negative to Self

void p10_Opcode_MOVN(void)
{
	// -(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	curAC[opAC] = p10_vRead(eAddr, dataMode);
	p10_spNegate(&curAC[opAC]);
}

void p10_Opcode_MOVNI(void)
{
	// -(0,,E) -> (AC)

	curAC[opAC] = RH(eAddr);
	p10_spNegate(&curAC[opAC]);
}

void p10_Opcode_MOVNM(void)
{
	// -(AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = curAC[opAC];
	p10_spNegate(&AR);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_MOVNS(void)
{
	// -(E) -> (E)
	// if AC is non-zero: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	p10_spNegate(&AR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}

// ************** Move Magnitude *************

// 214 MOVM  - Move Magnitude
// 215 MOVMI - Move Magnitude Immediate
// 216 MOVMM - Move Magnitude to Memory
// 217 MOVMS - Move Magnitude to Self

void p10_Opcode_MOVM(void)
{
	// |(E)| -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	curAC[opAC] = p10_vRead(eAddr, dataMode);
	p10_spMagnitude(&curAC[opAC]);
}

void p10_Opcode_MOVMI(void)
{
	// 0,,E -> (AC)

	curAC[opAC] = RH(eAddr);
	p10_spMagnitude(&curAC[opAC]);
}

void p10_Opcode_MOVMM(void)
{
	// |(AC)| -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData  = p10_Access(eAddr, dataMode | PTF_WRITE);
	*pData = curAC[opAC];
	p10_spMagnitude(pData);
}

void p10_Opcode_MOVMS(void)
{
	// |(E)| -> (E)
	// if AC is nonzero: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	p10_spMagnitude(pData);
	if (opAC)
		curAC[opAC] = *pData;
}

// ************** Integer Multiply *************

// 220 IMUL  - Integer Multiply to AC from Memory
// 221 IMULI - Integer Multiply to AC from Immediate
// 222 IMULM - Integer Multiply to Memory from AC
// 223 IMULB - Integer Multiply to Both from Memory

void p10_Opcode_IMUL(void)
{
	// (AC) * (E) -> (AC)
	// Note: the high word of the product is discarded.

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	p10_spMultiply(&curAC[opAC], BR);
}

void p10_Opcode_IMULI(void)
{
	// (AC) * 0,,E -> (AC)
	// Note: the high word of the product is discarded.

	p10_spMultiply(&curAC[opAC], RH(eAddr));
}

void p10_Opcode_IMULM(void)
{
	// (AC) * (E) -> (E)
	// Note: the high word of the product is discarded.

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR = curAC[opAC];
	p10_spMultiply(&AR, *pData);
	*pData = AR;
}

void p10_Opcode_IMULB(void)
{
	// (AC) * (E) -> (AC)(E)
	// Note: the high word of the product is discarded.

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR = curAC[opAC];
	p10_spMultiply(&AR, *pData);
	curAC[opAC] = *pData = AR;
}

// ************** Multiply *************

// 224 MUL  - Multiply to AC from Memory
// 225 MULI - Multiply to AC from Immediate
// 226 MULM - Multiply to Memory from AC
// 227 MULB - Multiply to Both from Memory

void p10_Opcode_MUL(void)
{
	// (AC) * (E) -> (AC,AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	p10_dpMultiply(&curAC[opAC], &curAC[AC(opAC + 1)], BR);
}

void p10_Opcode_MULI(void)
{
	// (AC) * 0,,E -> (AC,AC+1)

	p10_dpMultiply(&curAC[opAC], &curAC[AC(opAC + 1)], RH(eAddr));
}

void p10_Opcode_MULM(void)
{
	// (AC) * (E) -> (E)
	// Note: The low word of the product is discarded

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR = curAC[opAC];
	p10_dpMultiply(&AR, &ARX, *pData);
	*pData = AR;
}

void p10_Opcode_MULB(void)
{
	// (AC) * (E) -> (AC,AC+1)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR  = curAC[opAC];
	ARX = curAC[AC(opAC+1)];
	p10_dpMultiply(&AR, &ARX, *pData);
	curAC[opAC] = *pData = AR;
	curAC[AC(opAC+1)] = ARX;
}

// ************** Integer Divide *************

// 230 IDIV  - Integer Divide to AC from Memory
// 231 IDIVI - Integer Divide to AC from Immediate
// 232 IDIVM - Integer Divide to Memory from AC
// 233 IDIVB - Integer Divide to Both from Memory

void p10_Opcode_IDIV(void)
{
	// (AC) / (E) -> (AC), Remainder -> (AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	p10_spDivide(&curAC[opAC], &curAC[AC(opAC + 1)], BR);
}

void p10_Opcode_IDIVI(void)
{
	// (AC) / 0,,E -> (AC), Remainder -> (AC+1)

	p10_spDivide(&curAC[opAC], &curAC[AC(opAC + 1)], RH(eAddr));
}

void p10_Opcode_IDIVM(void)
{
	// (AC) / (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR = curAC[opAC];
	if (p10_spDivide(&AR, &ARX, *pData))
		*pData = AR;
}

void p10_Opcode_IDIVB(void)
{
	// (AC) / (E) -> (AC)(E), Remainder -> (AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR    = curAC[opAC];
	ARX   = curAC[AC(opAC+1)];
	if (p10_spDivide(&AR, &ARX, *pData)) {
		curAC[opAC] = *pData = AR;;
		curAC[AC(opAC+1)] = ARX;
	}
}

// ************** Divide *************

// 234 DIV  - Multiply to AC from Memory
// 235 DIVI - Multiply to AC from Immediate
// 236 DIVM - Multiply to Memory from AC
// 237 DIVB - Multiply to Both from Memory

void p10_Opcode_DIV(void)
{
	// (AC,AC+1) / (E) -> (AC), Remainder -> (AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	p10_dpDivide(&curAC[opAC], &curAC[AC(opAC + 1)], BR);
}

void p10_Opcode_DIVI(void)
{
	// (AC,AC+1) / 0,,E -> (AC), Remainder -> (AC+1)

	p10_dpDivide(&curAC[opAC], &curAC[AC(opAC + 1)], RH(eAddr));
}

void p10_Opcode_DIVM(void)
{
	// (AC,AC+1) / (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR    = curAC[opAC];
	ARX   = curAC[AC(opAC + 1)];
	if (p10_dpDivide(&AR, &ARX, *pData))
		*pData = AR;
}

void p10_Opcode_DIVB(void)
{
	// (AC,AC+1) / (E) -> (AC)(E), Remainder -> (AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	AR    = curAC[opAC];
	ARX   = curAC[AC(opAC+1)];
	if (p10_dpDivide(&AR, &ARX, *pData)) {
		curAC[opAC] = *pData = AR;
		curAC[AC(opAC+1)] = ARX;
	}
}

// ************ Shift/Rotate/Jump *************

// 240 ASH  - Arithmetic Shift
// 241 ROT  - Rotate
// 242 LSH  - Logical Shift
// 243 JFFO - Jump if Find First One
// 244 ASHC - Arithmetic Shift Combined
// 245 ROTC - Rotate Combined
// 246 LSHC - Logical Shift Combined

void p10_Opcode_ASH(void)
{
	// (AC) * 2^E -> (AC)

	p10_spAShift(&curAC[opAC], eAddr);
}

void p10_Opcode_ROT(void)
{
	// Rotate (AC) E places 

	p10_spRotate(&curAC[opAC], eAddr);
}

void p10_Opcode_LSH(void)
{
	// Shift (AC) E places

	p10_spLShift(&curAC[opAC], eAddr);
}

void p10_Opcode_JFFO(void)
{
	// if (AC) = 0:  0 -> (AC+1)
	// if (AC) != 0: E -> (PC)

	AR = curAC[opAC];
	BR = 0;

	if (AR) {
		while (!(AR & WORD36_SIGN)) {
			AR <<= 1;
			BR++;
		}
		DO_XJUMP(eAddr);
	}

	curAC[AC(opAC + 1)] = SXT36(BR);
}

void p10_Opcode_ASHC(void)
{
	// (AC,AC+1) * 2^E -> (AC,AC+1)

	p10_dpAShift(&curAC[opAC], &curAC[AC(opAC + 1)], eAddr);
}

void p10_Opcode_ROTC(void)
{
	// Rotate (AC,AC+1) E places

	p10_dpRotate(&curAC[opAC], &curAC[AC(opAC + 1)], eAddr);
}

void p10_Opcode_LSHC(void)
{
	// Shift (AC,AC+1) E places 

	p10_dpLShift(&curAC[opAC], &curAC[AC(opAC + 1)], eAddr);
}

// ********************************************

// 250 EXCH - Exchange
void p10_Opcode_EXCH(void)
{
	// (E) <> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	BR = *pData;
	*pData = curAC[opAC];
	curAC[opAC] = BR;

#ifdef DEBUG
//	if (dbg_Check(DBG_TRACE|DBG_DATA))
//		dbg_Printf("MOVS: %c%06llo,,%06llo <=> %c%06llo,,%06llo\n",
//			((AR < 0) ? '-' : '+'), LHSR(AR), RH(AR),
//			((BR < 0) ? '-' : '+'), LHSR(BR), RH(BR));
#endif /* DEBUG */
}

static int30 srcAddr, dstAddr;
static int30 endAddr, lenAddr;
static int36 *srcData, *dstData;

void KX10_bltCleanup(void)
{
	int36 ac;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("BLT: *** Page Fail Trap at PC %06o\n", pager_PC);
		dbg_Printf("BLT: Current SA %06o DA %06o\n", srcAddr, dstAddr);
	}
#endif /* DEBUG */

	// On Page Fail Trap, save current source and destination
	// pointers for restarted instruction.
	ac = XWD((int36)srcAddr, (int36)dstAddr);
	curAC[opAC] = SXT36(ac);
}

// 251 BLT - Block Transfer
void KX10_Opcode_BLT(void)
{
	// Move R[E]-R(AC)+1 words starting with (L(AC))->(R(AC))

	int30 Section = LPC(eAddr);

	srcAddr = LHSR(curAC[opAC]);
	dstAddr = RH(curAC[opAC]);
	endAddr = RH(eAddr);
	lenAddr = endAddr - dstAddr + 1;

#ifdef DEBUG
	if (endAddr < dstAddr) {
		dbg_Printf("BLT: *** Program Error at PC %06o ***\n", pager_PC);
		dbg_Printf("BLT: E = %06o is less than %06o.\n", endAddr, dstAddr);
	}
#endif /* DEBUG */

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("BLT: Section: %o  E: %s  Method: %s\n",
			Section, KX10_IsGlobal ? "Global" : "Local",
			((srcAddr + 1) == dstAddr) ? "Spray" : "Transfer");
		dbg_Printf("BLT: Start SA %06o DA %06o at %06o Words\n",
			srcAddr, dstAddr, (lenAddr < 1 ? 1 : lenAddr));
	}
#endif /* DEBUG */

	if (ISCPU(CNF_KL10|CNF_KS10)) {
		// Must update SA/DA on AC frst before start transfer
		// for KL/KS Processors.
		AR = XWD((int36)(srcAddr + lenAddr), (int36)(dstAddr + lenAddr));
		curAC[opAC] = SXT36(AR);
	}

	// Get PXCT switches for source and destination.
	srcMode = p10_CheckPXCT(PXCT_BLT_SRC);
	dstMode = p10_CheckPXCT(PXCT_BLT_DST);
	pager_Cleanup = KX10_bltCleanup;

	if ((srcAddr + 1) == dstAddr) {
		// Method: Spray
		BR = p10_vRead(Section | VMA(srcAddr), srcMode);
		do {
			p10_vWrite(Section | VMA(dstAddr), BR, dstMode);

			srcAddr++;
			dstAddr++;
		} while (dstAddr <= endAddr);
	} else {
		// Method: Transfer
		do {
			BR = p10_vRead(Section | VMA(srcAddr), srcMode);
			p10_vWrite(Section | VMA(dstAddr), BR, dstMode);
	
			srcAddr++;
			dstAddr++;
		} while (dstAddr <= endAddr);
	}

	pager_Cleanup = NULL;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BLT: Final SA %06o DA %06o\n", srcAddr, dstAddr);
#endif /* DEBUG */
}

// 252 AOBJP - Add One to Both Halves of AC and Jump if Positive
void p10_Opcode_AOBJP(void)
{
	// (AC)+1,,1 -> (AC), if (AC) >= 0: E -> (PC)
	
	AR = curAC[opAC];
	AR = AOB(AR);
	AR = SXT36(AR);
	curAC[opAC] = AR; 

	if (AR >= 0) {
		DO_XJUMP(eAddr);
	}
}

// 253 AOBJN - Add One to Both Halves of AC and Jump if Negative
void p10_Opcode_AOBJN(void)
{
	// (AC)+1,,1 -> (AC), if (AC) < 0: E -> (PC)

	AR = curAC[opAC];
	AR = AOB(AR);
	AR = SXT36(AR);
	curAC[opAC] = AR; 

	if (AR < 0) {
		DO_XJUMP(eAddr);
	}
}

// 254 JRST - Jump and Restore Flags
// This is for KS10 processor only
void KS10_Opcode_JRST(void)
{
	switch(opAC) {
		// 25400 JRST - Jump                      JRST 0,
		// 25404 PORTAL - Portal                  JRST 1,
		//   Public flag is not supported by KS10 Processor 
		//   Jump - like JRST 0,
		case 000:
		case 001:
			// E -> (PC)

			DO_JUMP(eAddr);
			break;

		// 25410 JRSTF - Restore Flags and Jump    JRST 2,
		case 02:
			// L(X) or L(Y) -> flags, E -> (PC)

			eaMode = p10_CheckPXCT(PXCT_EA);
			eaFlags = p10_CalcJumpAddr(HR, eaMode);
			if (FLAGS & FLG_USER) {
				eaFlags |= FLG_USER;
				if (!(FLAGS & FLG_USERIO))
					eaFlags &= ~FLG_USERIO;
			}
			FLAGS = eaFlags & PC_FLAGS;
			DO_JUMP(eAddr);
			break;

		// 25420 HALT - Halt                       JRST 4,
		case 04:
			// E -> (PC), Stop

			if (FLAGS & FLG_USER)
				KS10_Opcode_UUO();
			else {
				printf("CPU: HALT (Reason Code: %06llo) at PC %06llo\r\n",
					eAddr, RH(PC-1));
				DO_JUMP(eAddr);
				T1 = 1; /* Halt Code 1 - Halt Instruction */
				p10_SetHaltStatus(); // Store info on starting HSB address
				p10_State = EMU_HALT;
			}
			break;

		// 25424 XJRSTF -  ??                      JRST 5,
		case 05:
			// L(E) -> flags, (E+1) -> (PC)

			AR = p10_vRead(eAddr++, NOPXCT);
			BR = p10_vRead(eAddr, NOPXCT);
			AR &= PC_FLAGS;
			if (FLAGS & FLG_USER) {
				AR |= FLG_USER;
				if (!(FLAGS & FLG_USERIO))
					AR &= ~FLG_USERIO;
			}
			FLAGS = AR;
			DO_JUMP(BR);

#ifdef DEBUG
//			if (savedModePFT < 0)
//				dbg_SetMode(~savedModePFT);
#endif /* DEBUG */

			break;

		// 25430 XJEN - ??                         JRST 6,
		case 06:
			// Dismiss PI, L(E) -> Flags, (E+1) -> (PC)

			if (FLAGS & FLG_USER)
				KS10_Opcode_UUO();
			else {
				AR = p10_vRead(eAddr++, NOPXCT);
				BR = p10_vRead(eAddr, NOPXCT);
				AR &= PC_FLAGS;
				if (FLAGS & FLG_USER) {
					AR |= FLG_USER;
					if (!(FLAGS & FLG_USERIO))
						AR &= ~FLG_USERIO;
				}
				FLAGS = AR;
				DO_JUMP(BR);
				KS10_piDismiss();

#ifdef DEBUG
//				if (savedMode < 0)
//					dbg_SetMode(~savedMode);
#endif /* DEBUG */
			}
			break;

		// 25434 XPCW - ??                         JRST 7,
		case 07:
			// Flags,,0 -> (E), PC+1 -> (E+1), L(E+2) -> Flags, (E+3) -> (PC)

			if (FLAGS & FLG_USER)
				KS10_Opcode_UUO();
			else {
				p10_vWrite(eAddr++, FLAGS, NOPXCT);
				p10_vWrite(eAddr++, PC, NOPXCT);
				AR = p10_vRead(eAddr++, NOPXCT);
				BR = p10_vRead(eAddr, NOPXCT);
				AR &= PC_FLAGS;
				if (FLAGS & FLG_USER) {
					AR |= FLG_USER;
					if (!(FLAGS & FLG_USERIO))
						AR &= ~FLG_USERIO;
				}
				FLAGS = AR;
				DO_JUMP(BR);
			}
			break;

		// 25440                                   JRST 10,
		case 010:
			if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0))
				KS10_Opcode_UUO();
			else {
				KS10_piDismiss();
				DO_JUMP(eAddr);
			}
			break;

		// 25450 JEN - Jump and Enable             JRST 12,
		case 012:
			// Dismiss PI, L(X) or L(Y) -> Flags, E -> (PC)

			if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0))
				KS10_Opcode_UUO();
			else {
				eaMode = p10_CheckPXCT(PXCT_EA);
				eaFlags = p10_CalcJumpAddr(HR, eaMode);
				if (FLAGS & FLG_USER) {
					eaFlags |= FLG_USER;
					if (!(FLAGS & FLG_USERIO))
						eaFlags &= ~FLG_USERIO;
				}
				FLAGS = eaFlags & PC_FLAGS;
				DO_JUMP(eAddr);
				KS10_piDismiss();
			}
			break;

		// 25460 SFM - Store Flags to Memory       JRST 14,
		case 014:
			// Flags,,0 -> (E)

			if (FLAGS & FLG_USER)
				KS10_Opcode_UUO();
			else
				p10_vWrite(eAddr, FLAGS & PC_FLAGS, NOPXCT);
			break;

		// Otherwise - Illegal Instruction
		default:
			KS10_Opcode_UUO();
	}
}

// 255 JFCL - Jump on Flag and Clear

// 25500 JFCL  - No-op                       JFCL 0,
// 25504 JFOV  - Jump on Floating Overflow   JFCL 1,
// 25510 JCRY1 - Jump on Carry 1             JFCL 2,
// 25520 JCRY0 - Jump on Carry 0             JFCL 4,
// 25530 JCRY  - Jump on Carry 0 or 1        JFCL 6,
// 25540 JOV   - Jump on Overflow            JFCL 10,

void p10_Opcode_JFCL(void)
{
	// AC and Flags != 0: E -> (PC), Flags AND ~AC -> Flags

	AR = (HR & INST_JFCL_AC) << 9;
	if (FLAGS & AR) {
		FLAGS &= ~AR;
		DO_XJUMP(eAddr);
	}
}

// 256 XCT - Execute
void p10_Opcode_XCT(void)
{
	// Execute (E)

//	dataMode = p10_CheckPXCT(PXCT_DATA);

	if (opAC && ((FLAGS & FLG_USER) == 0))
			cpu_pFlags |= opAC;

	cpu_pFlags |= CPU_CYCLE_XCT;
//	p10_Execute(eAddr, dataMode);
	p10_Execute(eAddr, 0);
}

// 257 MAP - Map
// This is for KS10 processor only
void KS10_Opcode_MAP(void)
{
	// Physical Map Data -> (AC)

	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0))
		KS10_Opcode_UUO();
	else {
		dataMode = p10_CheckPXCT(PXCT_DATA);
		AR = KS10_GetMap(eAddr, dataMode);
		curAC[opAC] = SXT36(AR);
#ifdef DEBUG
		if (dbg_Check(DBG_DATA))
			dbg_Printf("MAP: %06o,,%06o -> %06o,,%06o\n",
				LH18(eAddr), RH18(eAddr), LH18(AR), RH18(AR));
#endif /* DEBUG */
	}
}

// **************** Stack Operations ******************

// 260 PUSHJ - Push and Jump
// 261 PUSH  - Push
// 262 POP   - Pop
// 263 POPJ  - Pop and Jump

void p10_Opcode_PUSHJ(void)
{
	// if L(PC) = 0:
	//     (AC)+1,1 -> (AC), Flags,PC+1 ->(R(AC))
	// if L(PC) != 0 and (AC)0,6-17 <= 0:
	//     (AC)+1,1 -> (AC), PC+1 ->(R(AC))
	// if L(PC) != 0 and (AC)0,6-17 > 0:
	//     (AC)+1 -> (AC), PC+1 -> ((AC))
	// E -> (PC)

	stackMode = p10_CheckPXCT(PXCT_STACK); // Stack Data

	AR = curAC[opAC];
	if (p10_Section > 0) {
		if (XLH(AR) > 0) {
			AR++;
			p10_vWrite(AR, PC, stackMode);
		} else {
			AR = AOB(AR);
			p10_vWrite(RH(AR), PC, stackMode);
		}
		BR = PC;
	} else {
		AR = AOB(AR);
		BR = FLAGS | RH(PC);
		p10_vWrite(RH(AR), BR, stackMode);
	}
	curAC[opAC] = SXT36(AR);

	FLAGS &= ~(FLG_FPD|FLG_AFI|FLG_TRAPS);
	if (LH(AR) == 0)
		FLAGS |= FLG_TRAP2;

	DO_XJUMP(eAddr);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("PUSHJ: Stack: %06o,,%06o <- %06o,,%06o\n",
			LH18(AR), RH18(AR), LH18(BR), RH18(BR));
#endif /* DEBUG */
}

void p10_Opcode_PUSH(void)
{
	// if L(PC) = 0 or (AC)0,6-17 <= 0:
	//     (AC)+1,1 -> (AC), (E) -> (R(AC))
	// if L(PC) != 0 and (AC)0,6-17 > 0:
	//     (AC)+1 -> (AC), (E) -> ((AC))

	stackMode = p10_CheckPXCT(PXCT_STACK); // Stack Data
	dataMode = p10_CheckPXCT(PXCT_DATA);   // Memory Data

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC];
	if ((p10_Section > 0) && (XLH(AR) > 0)) {
		AR++;
		p10_vWrite(AR, BR, stackMode);
	} else {
		AR = AOB(AR);
		p10_vWrite(RH(AR), BR, stackMode);
	}
	curAC[opAC] = SXT36(AR);

	if (LH(AR) == 0)
		FLAGS |= FLG_TRAP2;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("PUSH: Stack: %06o,,%06o <- %06o,,%06o\n",
			LH18(AR), RH18(AR), LH18(BR), RH18(BR));
#endif /* DEBUG */
}

void p10_Opcode_POP(void)
{
	// if L(PC) = 0 or (AC)0,6-17 <= 0:
	//     (R(AC)) -> (E), (AC)-1,1 -> (AC)
	// if L(PC) != 0 and (AC)0,6-17 > 0:
	//     ((AC)) -> (E), (AC)-1 -> (AC)

	stackMode = p10_CheckPXCT(PXCT_STACK); // Stack Data
	dataMode = p10_CheckPXCT(PXCT_DATA);   // Memory Data

	ARX = AR = curAC[opAC];
	if ((p10_Section > 0) && (XLH(AR) > 0)) {
		BR = p10_vRead(AR, stackMode);
		AR--;
	} else {
		BR = p10_vRead(RH(AR), stackMode);
		AR = SOB(AR);
	}
	p10_vWrite(eAddr, BR, dataMode);
	curAC[opAC] = SXT36(AR);

	if (LH(AR) == WORD18L_ONES)
		FLAGS |= FLG_TRAP2;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("POP: Stack: %06o,,%06o -> %06o,,%06o\n",
			LH18(ARX), RH18(ARX), LH18(BR), RH18(BR));
#endif /* DEBUG */
}

void p10_Opcode_POPJ(void)
{
	// if L(PC) = 0:
	//     R(R(AC)) -> (PC), (AC)-1,1 -> (AC)
	// if L(PC) != 0 and (AC)0,6-17 <= 0:
	//     (R(AC) -> (PC), (AC)-1,1 -> (AC)
	// if L(PC) != 0 and (AC)0,6-17 > 0:
	//     ((AC)) -> (PC), (AC)-1 -> (AC)

	stackMode = p10_CheckPXCT(PXCT_STACK); // Stack Data

	ARX = AR = curAC[opAC];
	if (p10_Section > 0) {
		if (XLH(AR) > 0) {
			BR = p10_vRead(AR, stackMode);
			AR--;
		} else {
			BR = p10_vRead(RH(AR), stackMode);
			eAddr = SXT18(eAddr);
			AR = SOB(AR);
		}
	} else {
		BR = RH(p10_vRead(RH(AR), stackMode));
		AR = SOB(AR);
	}
	curAC[opAC] = SXT36(AR);

	if (LH(AR) == WORD18L_ONES)
		FLAGS |= FLG_TRAP2;

	DO_XJUMP(BR);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("POPJ: Stack: %06o,,%06o -> %06o,,%06o\n",
			LH18(ARX), RH18(ARX), LH18(BR), RH18(BR));
#endif /* DEBUG */
}

// ******************** Jump ************************

// 264 JSR - Jump to Subroutine
// 265 JSP - Jump and Save PC
// 266 JSA - Jump and Save AC
// 267 JRA - Jump and Restore AC

void p10_Opcode_JSR(void)
{
	// if L(PC) = 0:
	//     Flags,,R(PC)+1 -> (E), E+1 -> (PC)
	// if L(PC) != 0:
	//     PC+1 -> (E), E+1 -> (PC)

	if (p10_Section > 0)
		p10_vWrite(eAddr, PC, NOPXCT);
	else 
		p10_vWrite(eAddr, (FLAGS & PC_FLAGS) | RH(PC), NOPXCT);
	DO_XJUMP(eAddr + 1);
	FLAGS &= ~(FLG_FPD|FLG_AFI|FLG_TRAP1|FLG_TRAP2);
}

void p10_Opcode_JSP(void)
{
	// if L(PC) = 0:
	//     Flags,,R(PC)+1 -> (AC), E -> (PC)
	// if L(PC) != 0:
	//     PC+1 -> (AC), E -> (PC)

	AR = (p10_Section > 0) ? PC : (FLAGS | RH(PC));
	curAC[opAC] = SXT36(AR);
	DO_XJUMP(eAddr);
	FLAGS &= ~(FLG_FPD|FLG_AFI|FLG_TRAP1|FLG_TRAP2);
}

void p10_Opcode_JSA(void)
{
	// (AC) -> (E), R(E),,R(PC)+1 -> (AC), E+1 -> (PC)

	p10_vWrite(eAddr, curAC[opAC], NOPXCT);
	AR = RHSL(eAddr) | RH(PC);
	curAC[opAC] = SXT36(AR);
	DO_JUMP(eAddr + 1);
}

void p10_Opcode_JRA(void)
{
	// (L(AC)) -> (AC), E -> (PC)

	AR  = curAC[opAC];
	ARX = p10_vRead(LHSR(AR), NOPXCT);
	curAC[opAC] = SXT36(ARX);
	DO_JUMP(eAddr);
}

// ************** Add *************

// 270 ADD  - Add
// 271 ADDI - Add Immediate
// 272 ADDM - Add to Memory
// 273 ADDB - Add to Both

void p10_Opcode_ADD(void)
{
	// (AC) + (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	p10_spAdd(&curAC[opAC], BR);
}

void p10_Opcode_ADDI(void)
{
	// (AC) + 0,,E -> (AC)

	p10_spAdd(&curAC[opAC], RH(eAddr));
}

void p10_Opcode_ADDM(void)
{
	// (AC) + (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	BR = *pData;
	AR = curAC[opAC];
	p10_spAdd(&AR, BR);
	*pData = AR;
}

void p10_Opcode_ADDB(void)
{
	// (AC) + (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	BR = *pData;
	AR = curAC[opAC];
	p10_spAdd(&AR, BR);
	*pData = AR;
	curAC[opAC] = AR;
}

// ************** Subtract *************

// 274 SUB  - Subtract
// 275 SUBI - Subtract Immediate
// 276 SUBM - Subtract to Memory
// 277 SUBB - Subtract to Both

void p10_Opcode_SUB(void)
{
	// (AC) - (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	p10_spSubtract(&curAC[opAC], BR);
}

void p10_Opcode_SUBI(void)
{
	// (AC) - 0,,E -> (AC)

	p10_spSubtract(&curAC[opAC], RH(eAddr));
}

void p10_Opcode_SUBM(void)
{
	// (AC) - (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	BR = *pData;
	AR = curAC[opAC];
	p10_spSubtract(&AR, BR);
	*pData = AR;
}

void p10_Opcode_SUBB(void)
{
	// (AC) - (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	BR = *pData;
	AR = curAC[opAC];
	p10_spSubtract(&AR, BR);
	*pData = AR;
	curAC[opAC] = AR;
}


// Opcode 300 Series - Arithmetic Testing

// Compare AC with Immediate and Skip if Condition Satisfied

// 300 CAI    - Compare AC with Immediate but Never Skip
// 301 CAIL   - Compare AC with Immediate and Skip if Less
// 302 CAIE   - Compare AC with Immediate and Skip if Equal
// 303 CAILE  - Compare AC with Immediate and Skip if Less or Equal
// 304 CAIA   - Compare AC with Immediate and Always Skip
// 305 CAIGE  - Compare AC with Immediate and Skip if Greater or Equal
// 306 CAIN   - Compare AC with Immediate and Skip if Not Equal
// 307 CAIG   - Compare AC with Immediate and Skip if Greater

void p10_Opcode_CAI(void)
{
	AR = curAC[opAC];

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("CAI: AC = %c%06o,,%06o E = %06o\n",
			((AR < 0) ? '-' : '+'), (int18)LHSR(AR), (int18)RH(AR),
			(int18)RH(eAddr));
	}
#endif /* DEBUG */

	switch(opCode & 7) {
		case 0:                               break;
		case 1: if (AR < RH(eAddr))  DO_SKIP; break;
		case 2: if (AR == RH(eAddr)) DO_SKIP; break;
		case 3: if (AR <= RH(eAddr)) DO_SKIP; break;
		case 4:                      DO_SKIP; break;
		case 5: if (AR >= RH(eAddr)) DO_SKIP; break;
		case 6: if (AR != RH(eAddr)) DO_SKIP; break;
		case 7: if (AR > RH(eAddr))  DO_SKIP; break;
	}
}


// Compare AC with Memory and Skip if Condition Satisfied

// 310 CAM    - Compare AC with Memory but Never Skip
// 311 CAML   - Compare AC with Memory and Skip if Less
// 312 CAME   - Compare AC with Memory and Skip if Equal
// 313 CAMLE  - Compare AC with Memory and Skip if Less or Equal
// 314 CAMA   - Compare AC with Memory and Always Skip
// 315 CAMGE  - Compare AC with Memory and Skip if Greater or Equal
// 316 CAMN   - Compare AC with Memory and Skip if Not Equal
// 317 CAMG   - Compare AC with Memory and Skip if Greater

void p10_Opcode_CAM(void)
{
	dataMode = p10_CheckPXCT(PXCT_DATA);
	
	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC];

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CAM: AC = %c%06llo,,%06llo E = %c%06llo,,%06llo\n",
			(AR < 0 ? '-' : '+'), LHSR(AR), RH(AR),
			(BR < 0 ? '-' : '+'), LHSR(BR), RH(BR));
#endif /* DEBUG */

	switch(opCode & 7) {
		case 0:                        break;
		case 1: if (AR <  BR) DO_SKIP; break;
		case 2: if (AR == BR) DO_SKIP; break;
		case 3: if (AR <= BR) DO_SKIP; break;
		case 4:               DO_SKIP; break;
		case 5: if (AR >= BR) DO_SKIP; break;
		case 6: if (AR != BR) DO_SKIP; break;
		case 7: if (AR >  BR) DO_SKIP; break;
	}
}


// Jump if AC Condition Satisfied

// 320 JUMP   - Never Jump
// 321 JUMPL  - Jump if AC Less than Zero
// 322 JUMPE  - Jump if AC Equal to Zero
// 323 JUMPLE - Jump if AC Less than or Equal to Zero
// 324 JUMPA  - Always Jump
// 325 JUMPGE - Jump if AC Greater than or Equal to Zero
// 326 JUMPN  - Jump if AC Not Equal to Zero
// 327 JUMPG  - Jump if AC Greater than Zero

void p10_Opcode_JUMP(void)
{
	AR = curAC[opAC];
	AR = SXT36(AR);

	switch(opCode & 7) {
		case 0:                               break;
		case 1: if (AR <  0) DO_XJUMP(eAddr); break;
		case 2: if (AR == 0) DO_XJUMP(eAddr); break;
		case 3: if (AR <= 0) DO_XJUMP(eAddr); break;
		case 4:              DO_XJUMP(eAddr); break;
		case 5: if (AR >= 0) DO_XJUMP(eAddr); break;
		case 6: if (AR != 0) DO_XJUMP(eAddr); break;
		case 7: if (AR >  0) DO_XJUMP(eAddr); break;
	}
}


// Skip if Memory Condition Satisfied

// 330 SKIP   - Never Skip
// 331 SKIPL  - Skip if Memory Less than Zero
// 332 SKIPE  - Skip if Memory Equal to Zero
// 333 SKIPLE - Skip if Memory Less than or Equal to Zero
// 334 SKIPA  - Always Skip
// 335 SKIPGE - Skip if Memory Greater than or Equal to Zero
// 336 SKIPN  - Skip if Memory Not Equal to Zero
// 337 SKIPG  - Skip if Memory Greater than Zero

void p10_Opcode_SKIP(void)
{
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	AR = SXT36(AR);
	if (opAC)
		curAC[opAC] = AR;

	switch(opCode & 7) {
		case 0:                       break;
		case 1: if (AR < 0)  DO_SKIP; break;
		case 2: if (AR == 0) DO_SKIP; break;
		case 3: if (AR <= 0) DO_SKIP; break;
		case 4:              DO_SKIP; break;
		case 5: if (AR >= 0) DO_SKIP; break;
		case 6: if (AR != 0) DO_SKIP; break;
		case 7: if (AR > 0)  DO_SKIP; break;
	}
}

#ifdef IDLE
static void idle (void)
{
	pause();
}

#define EMPTY_LOOP(PC, E) (((PC - 1) & VMA_MASK) == (E & VMA_MASK))
#endif /* IDLE */

// Add One to AC and Jump if Condition Satisfied

// 340 AOJ   - Add One to AC but Never Jump
// 341 AOJL  - Add One to AC and Jump if Less than Zero
// 342 AOJE  - Add One to AC and Jump if Equal to Zero
// 343 AOJLE - Add One to AC and Jump if Less than or Equal to Zero
// 344 AOJA  - Add One to AC and Always Jump
// 345 AOJGE - Add One to AC and Jump if Greater than or Equal to Zero
// 346 AOJN  - Add One to AC and Jump if Not Equal to Zero
// 347 AOJG  - Add One to AC and Jump if Greater than Zero

void p10_Opcode_AOJ(void)
{
	// (AC) + 1 -> (AC)

	p10_spInc(&curAC[opAC]);
	AR = curAC[opAC];

	switch(opCode & 7) {
		case 0:                              break;
		case 1: if (AR <  0) DO_XJUMP(eAddr); break;
		case 2: if (AR == 0) DO_XJUMP(eAddr); break;
		case 3: if (AR <= 0) DO_XJUMP(eAddr); break;
#ifdef IDLE
		case 4:
			if (EMPTY_LOOP (PC, eAddr)) {
				idle ();
			} else
				DO_XJUMP (eAddr);
			break;
#else
		case 4:              DO_XJUMP(eAddr); break;
#endif /* IDLE */
		case 5: if (AR >= 0) DO_XJUMP(eAddr); break;
		case 6: if (AR != 0) DO_XJUMP(eAddr); break;
		case 7: if (AR >  0) DO_XJUMP(eAddr); break;
	}
}


// Add One to Memory and Skip if Condition Satisfied

// 350 AOS   - Add One to Memory but Never Skip
// 351 AOSL  - Add One to Memory and Skip if Less than Zero
// 352 AOSE  - Add One to Memory and Skip if Equal to Zero
// 353 AOSLE - Add One to Memory and Skip if Less than or Equal to Zero
// 354 AOSA  - Add One to Memory and Always Skip
// 355 AOSGE - Add One to Memory and Skip if Greater than or Equal to Zero
// 356 AOSN  - Add One to Memory and Skip if Not Equal to Zero
// 357 AOSG  - Add One to Memory and Skip if Greater than Zero

void p10_Opcode_AOS(void)
{
	// (E) + 1 -> (E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	p10_spInc(&AR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;

	switch(opCode & 7) {
		case 0:                       break;
		case 1: if (AR < 0)  DO_SKIP; break;
		case 2: if (AR == 0) DO_SKIP; break;
		case 3: if (AR <= 0) DO_SKIP; break;
		case 4:              DO_SKIP; break;
		case 5: if (AR >= 0) DO_SKIP; break;
		case 6: if (AR != 0) DO_SKIP; break;
		case 7: if (AR > 0)  DO_SKIP; break;
	}
}

#ifdef IDLE
// Return 1 if the instruction is safe in an idle loop, 0 otherwise.
static int idleInstruction (int36 insn, int18 AC, int30 *nextPC)
{
	int dataMode;
	int36 E, AR;

	*nextPC = (*nextPC + 1) & VMA_MASK;

	switch (INST_GETOP (insn)) {
		case 0201: // MOVEI
			// A MOVEI into the SOJG AC is safe.
			return INST_GETAC (insn) == AC;
		case 0254: // JRST
			*nextPC = insn & 0777777;
			// Don't bother if the address is complex.
			return ((insn >> 18) & 0777) == 0;
		case 0332: // SKIPE
		case 0336: // SKIPN
			E = KX10_CalcEffAddr(PC, insn, cpu_pFlags & PXCT_EA);
			dataMode = p10_CheckPXCT(PXCT_DATA);
			AR = p10_vRead(E, dataMode);
			AR = WORD36_SXT(AR);
			if ((INST_GETOP (insn) == 0332 && AR == 0)
			    || (INST_GETOP (insn) == 0336 && AR != 0))
				*nextPC = (*nextPC + 1) & VMA_MASK;
			// Don't allow instructions that change AC.
			return INST_GETAC (insn) == 0;
		case 0344: // AOJA
			if (INST_GETAC (insn) != 0)
				return 0;
			// FIXME: this is kinda lame.
			p10_spInc(&curAC[INST_GETAC (insn)]);
			*nextPC = insn & 0777777;
			// Don't bother if the address is complex.
			return ((insn >> 18) & 037) == 0;
		case 0367: // SOJG
			// A SOJG jumping to itself is safe.
			return INST_GETAC (insn) == AC
			  && INST_GETY (insn) == ((*nextPC - 1) & VMA_MASK);
		case 0612: // TDNE
			E = KX10_CalcEffAddr(PC, insn, cpu_pFlags & PXCT_EA);
			dataMode = p10_CheckPXCT(PXCT_DATA);
			AR = p10_vRead(E, dataMode);
			if ((curAC[INST_GETAC (insn)] & AR) == 0)
				*nextPC = (*nextPC + 1) & VMA_MASK;
			return 1;
		default:
			return 0;
	}
}

// Return 1 if there is an idle loop at address PC, 0 otherwise.
// AC is the SOJG accumulator.
static int idleLoop (int30 PC, int18 AC)
{
	int30 startPC = PC;
	int30 i = PC;
	int36 insn;
	int n = 0;

	insn = p10_vRead(i, 0);
	while (idleInstruction (insn, AC, &i) && n < 20) {
		if (i == startPC)
			return 1;
		n++;
		insn = p10_vRead(i, 0);
	}

	return 0;
}
#endif /* IDLE */

// Subtract One from AC and Jump if Condition Satisfied

// 360 SOJ   - Subtract One from AC but Never Jump
// 361 SOJL  - Subtract One from AC and Jump if Less than Zero
// 362 SOJE  - Subtract One from AC and Jump if Equal to Zero
// 363 SOJLE - Subtract One from AC and Jump if Less than or Equal to Zero
// 364 SOJA  - Subtract One from AC and Always Jump
// 365 SOJGE - Subtract One from AC and Jump if Greater than or Equal to Zero
// 366 SOJN  - Subtract One from AC and Jump if Not Equal to Zero
// 367 SOJG  - Subtract One from AC and Jump if Greater than Zero

void p10_Opcode_SOJ(void)
{
	// (AC) - 1 -> (AC)

	p10_spDec(&curAC[opAC]);
	AR = curAC[opAC];

	switch(opCode & 7) {
		case 0:                               break;
		case 1: if (AR < 0)  DO_XJUMP(eAddr); break;
		case 2: if (AR == 0) DO_XJUMP(eAddr); break;
		case 3: if (AR <= 0) DO_XJUMP(eAddr); break;
		case 4:              DO_XJUMP(eAddr); break;
		case 5: if (AR >= 0) DO_XJUMP(eAddr); break;
		case 6: if (AR != 0) DO_XJUMP(eAddr); break;
#ifdef IDLE
		case 7:
			if (AR > 0) {
				if (EMPTY_LOOP (PC, eAddr)) {
					AR = curAC[opAC] = 0;
					if (idleLoop (PC & VMA_MASK, opAC))
						idle ();
				} else
					DO_XJUMP (eAddr);
			}
#else
		case 7: if (AR > 0)  DO_XJUMP(eAddr); break;
#endif /* IDLE */
	}
}


// Subtract One from Memory and Skip if Condition Satisfied

// 370 SOS   Subtract One from Memory but Never Skip
// 371 SOSL  Subtract One from Memory and Skip if Less than Zero
// 372 SOSE  Subtract One from Memory and Skip if Equal to Zero
// 373 SOSLE Subtract One from Memory and Skip if Less than or Equal to Zero
// 374 SOSA  Subtract One from Memory and Always Skip
// 375 SOSGE Subtract One from Memory and Skip if Greater than or Equal to Zero
// 376 SOSN  Subtract One from Memory and Skip if Not Equal to Zero
// 377 SOSG  Subtract One from Memory and Skip if Greater than Zero

void p10_Opcode_SOS(void)
{
	// (E) - 1 -> (E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	p10_spDec(&AR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;

	switch(opCode & 7) {
		case 0:                       break;
		case 1: if (AR < 0)  DO_SKIP; break;
		case 2: if (AR == 0) DO_SKIP; break;
		case 3: if (AR <= 0) DO_SKIP; break;
		case 4:              DO_SKIP; break;
		case 5: if (AR >= 0) DO_SKIP; break;
		case 6: if (AR != 0) DO_SKIP; break;
		case 7: if (AR > 0)  DO_SKIP; break;
	}
}


// Opcode 400 Series - Boolean

// Set to Zeros

// 400 SETZ   Set to Zeros
// 401 SETZI  Set to Zeros Immediate
// 402 SETZM  Set to Zeros Memory
// 403 SETZB  Set to Zeros Both

void p10_Opcode_SETZ(void)
{
	// 0 -> (AC)

	curAC[opAC] = 0;
}

void p10_Opcode_SETZI(void)
{
	// 0 -> (AC)

	curAC[opAC] = 0;
}

void p10_Opcode_SETZM(void)
{
	// 0 -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	p10_vWrite(eAddr, 0, dataMode);
}

void p10_Opcode_SETZB(void)
{
	// 0 -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	p10_vWrite(eAddr, 0, dataMode);
	curAC[opAC] = 0;
}


// And with AC

// 404 AND   And
// 405 ANDI  And Immediate
// 406 ANDM  And to Memory
// 407 ANDB  And to Both

void p10_Opcode_AND(void)
{
	// (AC) AND (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ANDI(void)
{
	// (AC) AND 0,,E -> (AC)

	AR = curAC[opAC] & RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ANDM(void)
{
	// (AC) AND (E) -> (E)
 
	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_ANDB(void)
{
	// (AC) AND (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & BR;
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// And with Complement of AC

// 410 ANDCA   And with Complement of AC
// 411 ANDCAI  And with Complement of AC Immediate
// 412 ANDCAM  And with Complement of AC to Memory
// 413 ANDCAB  And with Complement of AC to Both

void p10_Opcode_ANDCA(void)
{
	// ~(AC) AND (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] & BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ANDCAI(void)
{
	// ~(AC) AND 0,,E -> (AC)

	AR = ~curAC[opAC] & RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ANDCAM(void)
{
	// ~(AC) AND (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] & BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_ANDCAB(void)
{
	// ~(AC) AND (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	ARX = ~(AR = curAC[opAC]) & BR;
	p10_vWrite(eAddr, ARX, dataMode);
	curAC[opAC] = SXT36(ARX);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ANDCAB: %c%012llo AND NOT %c%12llo =>  %c%12llo\n",
			(AR < 0 ? '-' : '+'), AR & WORD36_ONES,
			(BR < 0 ? '-' : '+'), BR & WORD36_ONES,
			(ARX < 0 ? '-' : '+'), ARX & WORD36_ONES);
#endif /* DEBUG */
}


// Set to Memory

// 414 SETM   Set to Memory
// 415 SETMI  Set to Memory Immediate
// 416 SETMM  Set to Memory Memory
// 417 SETMB  Set to Memory Both

// 415 XMOVEI Extended Move Immediate (KL10 Processor)

void p10_Opcode_SETM(void)
{
	// (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	curAC[opAC] = p10_vRead(eAddr, dataMode);
}

void p10_Opcode_SETMI(void)
{
	// For XMOVEI Instruction: (Extended addressing only)
	// If not local AC reference: E -> (AC)
	// if local AC reference:     SECTION,,E -> (AC)

	// For SETMI Instruction
	// 0,,E -> (AC)

	curAC[opAC] = eAddr;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		if (ISCPU(CNF_XADR)) {
			dbg_Printf("XMOVEI: AC[%d,%02o] <= %06llo,,%06llo\n",
				CACB, opAC, LHSR(eAddr), RH(eAddr));
		} else {
			dbg_Printf("SETMI: AC[%d,%02o] <= %06llo\n",
				CACB, opAC, RH(eAddr));
		}
	}
#endif /* DEBUG */
}

void p10_Opcode_SETMM(void)
{
	// (E) -> (E)  (No-op)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_SETMB(void)
{
	// (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = AR;
}


// And Complement of Memory with AC

// 420 ANDCM   And Complement of Memory
// 421 ANDCMI  And Complement of Memory Immediate
// 422 ANDCMM  And Complement of Memory to Memory
// 423 ANDCMB  And Complement of Memory to Both

void p10_Opcode_ANDCM(void)
{
	// (AC) AND ~(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & ~BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ANDCMI(void)
{
	// (AC) AND ~[0,,E] -> (AC)

	AR = curAC[opAC] & ~RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ANDCMM(void)
{
	// (AC) AND ~(E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & ~BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_ANDCMB(void)
{
	// (AC) AND ~(E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & ~BR;
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// Set to AC

// 424 SETA   Set to AC
// 425 SETAI  Set to AC Immediate
// 426 SETAM  Set to AC Memory
// 427 SETAB  Set to AC Both

// Note: SETA and SETAI acts like no-op instruction
//       because they write AC themselves.
// Note: SETAM and SETAB acts like MOVEM instruction.

void p10_Opcode_SETA(void)
{
	// (AC) -> (AC)  (No-op)

	curAC[opAC] = curAC[opAC];
}

void p10_Opcode_SETAI(void)
{
	// (AC) -> (AC)  (No-op)

	curAC[opAC] = curAC[opAC];
}

void p10_Opcode_SETAM(void)
{
	// (AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	p10_vWrite(eAddr, curAC[opAC], dataMode);
}

void p10_Opcode_SETAB(void)
{
	// (AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	p10_vWrite(eAddr, curAC[opAC], dataMode);
}


// Exclusive Or with AC

// 430 XOR   Exclusive Or 
// 431 XORI  Exclusive Or Immediate
// 432 XORM  Exclusive Or to Memory
// 433 XORB  Exclusive Or to Both

void p10_Opcode_XOR(void)
{
	// (AC) ^ (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] ^ BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_XORI(void)
{
	// (AC) ^ 0,,E -> (AC)

	AR = curAC[opAC] ^ RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_XORM(void)
{
	// (AC) ^ (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] ^ BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_XORB(void)
{
	// (AC) ^ (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] ^ BR;
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// Inclusive Or with AC

// 434 IOR   Inclusive Or 
// 435 IORI  Inclusive Or Immediate
// 436 IORM  Inclusive Or to Memory
// 437 IORB  Inclusive Or to Both

void p10_Opcode_IOR(void)
{
	// (AC) | (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_IORI(void)
{
	// (AC) | 0,,E -> (AC)

	AR = curAC[opAC] | RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_IORM(void)
{
	// (AC) | (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_IORB(void)
{
	// (AC) | (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | BR;
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// And Complements of Both

// 440 ANDCB   And Complements of Both
// 441 ANDCBI  And Complements of Both Immediate
// 442 ANDCBM  And Complements of Both to Memory
// 443 ANDCBB  And Complements of Both to Both

void p10_Opcode_ANDCB(void)
{
	// ~(AC) AND ~(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] & ~BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ANDCBI(void)
{
	// ~(AC) AND ~[0,,E] -> (AC)

	AR = ~curAC[opAC] & ~RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ANDCBM(void)
{
	// ~(AC) AND ~(E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] & ~BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_ANDCBB(void)
{
	// ~(AC) AND ~(E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] & ~BR;
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// Equivalence with AC

// 444 EQV   Equivalence
// 445 EQVI  Equivalence Immediate
// 446 EQVM  Equivalence to Memory
// 447 EQVB  Equivalence to Both

void p10_Opcode_EQV(void)
{
	// ~[(AC) ^ (E)] -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~(curAC[opAC] ^ BR);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_EQVI(void)
{
	// ~[(AC) ^ 0,,E] -> (AC)

	AR = ~(curAC[opAC] ^ RH(eAddr));
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_EQVM(void)
{
	// ~[(AC) ^ (E)] -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~(curAC[opAC] ^ BR);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_EQVB(void)
{
	// ~[(AC) ^ (E)] -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~(curAC[opAC] ^ BR);
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// Set to Complement of AC

// 450 SETCA   Set to Complement of AC
// 451 SETCAI  Set to Complement of AC Immediate
// 452 SETCAM  Set to Complement of AC Memory
// 453 SETCAB  Set to Complement of AC Both

void p10_Opcode_SETCA(void)
{
	// ~(AC) -> (AC)

	AR = ~curAC[opAC];
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_SETCAI(void)
{
	// ~(AC) -> (AC)

	AR = ~curAC[opAC];
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_SETCAM(void)
{
	// ~(AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = ~curAC[opAC];
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_SETCAB(void)
{
	// ~(AC) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = ~curAC[opAC];
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// Inclusive Or with Complement of AC

// 454 ORCA  Or with Complement of AC
// 455 ORCAI Or with Complement of AC Immediate
// 456 ORCAM Or with Complement of AC to Memory
// 457 ORCAB Or with Complement of AC to Both

void p10_Opcode_ORCA(void)
{
	// ~(AC) | (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] | BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ORCAI(void)
{
	// ~(AC) | 0,,E -> (AC)

	AR = ~curAC[opAC] | RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ORCAM(void)
{
	// ~(AC) | (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] | BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_ORCAB(void)
{
	// ~(AC) | (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] | BR;
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// Set to Complement of Memory

// 460 SETCM   Set to Complement of Memory
// 461 SETCMI  Set to Complement of Memory Immediate
// 462 SETCMM  Set to Complement of Memory Memory
// 463 SETCMB  Set to Complement of Memory Both

void p10_Opcode_SETCM(void)
{
	// ~(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_SETCMI(void)
{
	// ~[0,,E] -> (AC)

	AR = ~RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_SETCMM(void)
{
	// ~(E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_SETCMB(void)
{
	// ~(E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~BR;
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// Inclusive Or Complement of Memory with AC

// 464 ORCM   Or Complement of Memory
// 465 ORCMI  Or Complement of Memory Immediate
// 466 ORCMM  Or Complement of Memory to Memory
// 467 ORCMB  Or Complement of Memory to Both

void p10_Opcode_ORCM(void)
{
	// (AC) | ~(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | ~BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ORCMI(void)
{
	// (AC) | ~[0,,E] -> (AC)

	AR = curAC[opAC] | ~RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ORCMM(void)
{
	// (AC) | ~(E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | ~BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_ORCMB(void)
{
	// (AC) | ~(E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | ~BR;
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// Inclusive Or Complements of Both

// 470 ORCB   Or Complements of Both
// 471 ORCBI  Or Complements of Both Immediate
// 472 ORCBM  Or Complements of Both to Memory
// 473 ORCBB  Or Complements of Both to Both

void p10_Opcode_ORCB(void)
{
	// ~(AC) | ~(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] | ~BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ORCBI(void)
{
	// ~(AC) | ~[0,,E] -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = ~curAC[opAC] | ~RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_ORCBM(void)
{
	// ~(AC) | ~(E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] | ~BR;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_ORCBB(void)
{
	// ~(AC) | ~(E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = ~curAC[opAC] | ~BR;
	p10_vWrite(eAddr, AR, dataMode);
	curAC[opAC] = SXT36(AR);
}


// Set to Ones

// 474 SETO   Set to Ones
// 475 SETOI  Set to Ones Immediate
// 476 SETOM  Set to Ones Memory
// 477 SETOB  Set to Ones Both

void p10_Opcode_SETO(void)
{
	// 777777,,777777 -> (AC)

	curAC[opAC] = WORD36_XONES;
}

void p10_Opcode_SETOI(void)
{
	// 777777,,777777 -> (AC)

	curAC[opAC] = WORD36_XONES;
}

void p10_Opcode_SETOM(void)
{
	// 777777,,777777 -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	p10_vWrite(eAddr, WORD36_XONES, dataMode);
}

void p10_Opcode_SETOB(void)
{
	// 777777,,777777 -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	p10_vWrite(eAddr, WORD36_XONES, dataMode);
	curAC[opAC] = WORD36_XONES;
}


// Opcode 500 Series - Halfword Data Transmission

// Halfword Move Instruction Format
//
// +-----------+---+-------+---+-------+--....--+
// | 1   0   0 | D |  M2   | S |  M1   |        |
// +---^---^---^---^---^---^---^---^---^--....--+
//   0   1   2   3   4   5   6   7   8   9    35
//
//  Bits      Labels       Definition
//  ----      ------       ----------
//   3          D          Destination  - Left or Right
//  4-5         M2         Modification - Nothing, Zero, Ones, or Extend
//   6          S          Source       - Left or Right
//  7-8         M1         Mode         - Basic, Immediate, Memory, or Self

// Halfword Move, Left to Left

// 500 HLL   Halfword Move, Left to Left
// 501 HLLI  Halfword Move, Left to Left, Immediate
// 502 HLLM  Halfword Move, Left to Left, Memory
// 503 HLLS  Halfword Move, Left to Left, Self

// 501 XHLLI Extended Halfword Move, Left to Left, Immediate

void p10_Opcode_HLL(void)
{
	// L(E) -> L(AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC];
	curAC[opAC] = LHSXT(BR) | RH(AR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("HLL: %c%06o,,%06o + %06o => %c%06o,,%06o\n",
			((AR < 0) ? '-' : '+'), (int18)LHSR(AR), (int18)RH(AR),
			(int18)LHSR(BR), ((curAC[opAC] < 0) ? '-' : '+'),
			(int18)LHSR(curAC[opAC]), (int18)RH(curAC[opAC]));
#endif /* DEBUG */
}

void p10_Opcode_HLLI(void)
{
	// HLLI:  0    -> L(AC)
	// XHLLI: L[E] -> L(AC)

	curAC[opAC] = LH(eAddr) | RH(curAC[opAC]);
}

void p10_Opcode_HLLM(void)
{
	// L(AC) -> L(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	AR = LHSXT(curAC[opAC]) | RH(AR);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HLLS(void)
{
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	if (opAC)
		curAC[opAC] = BR;
}


// Halfword Move, Right to Left

// 504 HRL   Halfword Move, Right to Left
// 505 HRLI  Halfword Move, Right to Left, Immediate
// 506 HRLM  Halfword Move, Right to Left, Memory
// 507 HRLS  Halfword Move, Right to Left, Self

void p10_Opcode_HRL(void)
{
	// R(E) -> L(AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RHSL(BR) | RH(curAC[opAC]);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRLI(void)
{
	// E -> L(AC)

	AR = RHSL(eAddr) | RH(curAC[opAC]);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRLM(void)
{
	// R(AC) -> L(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RHSL(curAC[opAC]) | RH(BR);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HRLS(void)
{
	// R(E) -> L(E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RHSL(BR) | RH(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = SXT36(AR);
}


// Halfword Move, Left to Left, Zeros

// 510 HLLZ   Halfword Move, Left to Left, Zeros
// 511 HLLZI  Halfword Move, Left to Left, Zeros, Immediate
// 512 HLLZM  Halfword Move, Left to Left, Zeros, Memory
// 513 HLLZS  Halfword Move, Left to Left, Zeros, Self

void p10_Opcode_HLLZ(void)
{
	// L(E),,0 -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	curAC[opAC] = LHSXT(BR);
}

void p10_Opcode_HLLZI(void)
{
	// 0 -> (AC)

	curAC[opAC] = 0;
}

void p10_Opcode_HLLZM(void)
{
	// L(AC),,0 -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = LHSXT(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HLLZS(void)
{
	// 0 -> R(E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = LHSXT(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}


// Halfword Move, Right to Left, Zeros

// 514 HRLZ   Halfword Move, Right to Left, Zeros
// 515 HRLZI  Halfword Move, Right to Left, Zeros, Immediate
// 516 HRLZM  Halfword Move, Right to Left, Zeros, Memory
// 517 HRLZS  Halfword Move, Right to Left, Zeros, Self

void p10_Opcode_HRLZ(void)
{
	// R(E),,0 -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RHSL(BR);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRLZI(void)
{
	// E,,0 -> (AC)

	AR = RHSL(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRLZM(void)
{
	// R(AC),,0 -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = RHSL(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HRLZS(void)
{
	// R(E),,0 -> (E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RHSL(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = SXT36(AR);
}


// Halfword Move, Left to Left, Ones

// 520 HLLO   Halfword Move, Left to Left, Ones
// 521 HLLOI  Halfword Move, Left to Left, Ones, Immediate
// 522 HLLOM  Halfword Move, Left to Left, Ones, Memory
// 523 HLLOS  Halfword Move, Left to Left, Ones, Self

void p10_Opcode_HLLO(void)
{
	// L(E),,777777 -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	curAC[opAC] = LHSXT(BR) | WORD18R_ONES;
}

void p10_Opcode_HLLOI(void)
{
	// 0,,777777 -> (AC)

	curAC[opAC] = WORD18R_ONES;
}

void p10_Opcode_HLLOM(void)
{
	// L(AC),,777777 -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = LHSXT(curAC[opAC]) | WORD18R_ONES;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HLLOS(void)
{
	// 777777 -> R(E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = LHSXT(BR) | WORD18R_ONES;
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}


// Halfword Move, Right to Left, Ones

// 524 HRLO   Halfword Move, Right to Left, Ones
// 525 HRLOI  Halfword Move, Right to Left, Ones, Immediate
// 526 HRLOM  Halfword Move, Right to Left, Ones, Memory
// 527 HRLOS  Halfword Move, Right to Left, Ones, Self

void p10_Opcode_HRLO(void)
{
	// R(E),,777777 -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RHSL(BR) | WORD18R_ONES;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRLOI(void)
{
	// E,,777777 -> (AC)

	AR = RHSL(eAddr) | WORD18R_ONES;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRLOM(void)
{
	// R(AC),,777777 -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = RHSL(curAC[opAC]) | WORD18R_ONES;
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HRLOS(void)
{
	// R(E),,777777 -> (E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RHSL(BR) | WORD18R_ONES;
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = SXT36(AR);
}


// Halfword Move, Left to Left, Extend

// 530 HLLE   Halfword Move, Left to Left, Extend
// 531 HLLEI  Halfword Move, Left to Left, Extend, Immediate
// 532 HLLEM  Halfword Move, Left to Left, Extend, Memory
// 533 HLLES  Halfword Move, Left to Left, Extend, Self

void p10_Opcode_HLLE(void)
{
	// L(E),,[(E)0 * 777777] -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	curAC[opAC] = LHSXT(BR) | (S36(BR) ? WORD18R_ONES : 0);
}

void p10_Opcode_HLLEI(void)
{
	// 0 -> (AC)

	curAC[opAC] = 0;
}

void p10_Opcode_HLLEM(void)
{
	// L(AC),,[(AC)0 * 777777] -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = LHSXT(curAC[opAC]) | (S36(curAC[opAC]) ? WORD18R_ONES : 0);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HLLES(void)
{
	// (E)0 * 777777 -> R(E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = LHSXT(BR) | (S36(BR) ? WORD18R_ONES : 0);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}


// Halfword Move, Right to Left, Extend

// 534 HRLE   Halfword Move, Right to Left, Extend
// 535 HRLEI  Halfword Move, Right to Left, Extend, Immediate
// 536 HRLEM  Halfword Move, Right to Left, Extend, Memory
// 537 HRLES  Halfword Move, Right to Left, Extend, Self

void p10_Opcode_HRLE(void)
{
	// R(E),,[(E)18 x 777777] -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RHSL(BR) | (S18(BR) ? WORD18R_ONES : 0);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRLEI(void)
{
	// E,,[(E)18 x 777777] -> (AC)

	AR = RHSL(eAddr) | (S18(eAddr) ? WORD18R_ONES : 0);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRLEM(void)
{
	// R(AC),,[(AC)18 x 777777] -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = RHSL(curAC[opAC]) | (S18(curAC[opAC]) ? WORD18R_ONES : 0);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HRLES(void)
{
	// R(E),,[(E)18 x 777777] -> (E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RHSL(BR) | (S18(BR) ? WORD18R_ONES : 0);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = SXT36(AR);
}


// Halfword Move, Right to Right

// 540 HRR   Halfword Move, Right to Right
// 541 HRRI  Halfword Move, Right to Right, Immediate
// 542 HRRM  Halfword Move, Right to Right, Memory
// 543 HRRS  Halfword Move, Right to Right, Self

void p10_Opcode_HRR(void)
{
	// R(E) -> R(AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = LH(curAC[opAC]) | RH(BR);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRRI(void)
{
	// E -> R(AC)

	AR = LH(curAC[opAC]) | RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_HRRM(void)
{
	// R(AC) -> R(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = LH(BR) | RH(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HRRS(void)
{
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = p10_vRead(eAddr, dataMode);
	if (opAC)
		curAC[opAC] = SXT36(AR);
}

// Halfword Move, Left to Right 

// 544 HLR   Halfword Move, Left to Right
// 545 HLRI  Halfword Move, Left to Right, Immediate
// 546 HLRM  Halfword Move, Left to Right, Memory
// 547 HLRS  Halfword Move, Left to Right, Self

void p10_Opcode_HLR(void)
{
	// L(E) -> R(AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	curAC[opAC] = LHSXT(curAC[opAC]) | LHSR(BR);
}

void p10_Opcode_HLRI(void)
{
	// 0 -> R(AC)

	curAC[opAC] = LHSXT(curAC[opAC]);
}

void p10_Opcode_HLRM(void)
{
	// L(AC) -> R(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = LHSXT(BR) | LHSR(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HLRS(void)
{
	// L(E) -> R(E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = LHSXT(BR) | LHSR(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}


// Halfword Move, Right to Right, Zeros

// 550 HRRZ   Halfword Move, Right to Right, Zeros
// 551 HRRZI  Halfword Move, Right to Right, Zeros, Immediate
// 552 HRRZM  Halfword Move, Right to Right, Zeros, Memory
// 553 HRRZS  Halfword Move, Right to Right, Zeros, Self

void p10_Opcode_HRRZ(void)
{
	// 0,,R(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	curAC[opAC] = RH(BR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("HRRZ: AC[%02o] <= E[%o,,%o]: 000000,,%06o\n",
			opAC, LH18(eAddr), RH18(eAddr), RH18(BR));

#endif /* DEBUG */
}

void p10_Opcode_HRRZI(void)
{
	// 0,,E -> (AC)

	curAC[opAC] = RH(eAddr);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("HRRZI: AC[%02o] <= 000000,,%06o\n", opAC, RH18(eAddr));
#endif /* DEBUG */
}

void p10_Opcode_HRRZM(void)
{
	// 0,,R(AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = RH(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("HRRZM: E[%o,,%o] <= AC[%02o]: 000000,,%06o\n",
			LH18(eAddr), RH18(eAddr), opAC, RH18(AR));
#endif /* DEBUG */
}

void p10_Opcode_HRRZS(void)
{
	// 0 -> L(E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = RH(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("HRRZS: E[%o,,%o] <= AC[%02o]: 000000,,%06o\n",
			LH18(eAddr), RH18(eAddr), opAC, RH18(AR));
		if (opAC)
			dbg_Printf("HRRZS: AC[%02o] <= AC[%02o]: 000000,,%06o\n",
				opAC, opAC, RH18(AR));
	}
#endif /* DEBUG */
}


// Halfword Move, Left to Right, Zeros

// 554 HLRZ   Halfword Move, Left to Right, Zeros
// 555 HLRZI  Halfword Move, Left to Right, Zeros, Immediate
// 556 HLRZM  Halfword Move, Left to Right, Zeros, Memory
// 557 HLRZS  Halfword Move, Left to Right, Zeros, Self

void p10_Opcode_HLRZ(void)
{
	// 0,,L(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	curAC[opAC] = LHSR(BR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("HLRZ: %06o,,%06o -> %06o,,%06o\n",
			LH18(BR), RH18(BR), LH18(curAC[opAC]), RH18(curAC[opAC]));
	}
#endif /* DEBUG */
}

void p10_Opcode_HLRZI(void)
{
	// 0 -> (AC)

	curAC[opAC] = 0;
}

void p10_Opcode_HLRZM(void)
{
	// 0,,L(AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = LHSR(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HLRZS(void)
{
	// 0,,L(E) -> (E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = LHSR(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}


// Halfword Move, Right to Right, Ones

// 560 HRRO   Halfword Move, Right to Right, Ones
// 561 HRROI  Halfword Move, Right to Right, Ones, Immediate
// 562 HRROM  Halfword Move, Right to Right, Ones, Memory
// 563 HRROS  Halfword Move, Right to Right, Ones, Self

void p10_Opcode_HRRO(void)
{
	// 777777,,R(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = WORD18L_XONES | RH(BR);
	curAC[opAC] = AR;
}

void p10_Opcode_HRROI(void)
{
	// 777777,,E -> (AC)

	AR = WORD18L_XONES | RH(eAddr);
	curAC[opAC] = AR;
}

void p10_Opcode_HRROM(void)
{
	// 777777,,R(AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = WORD18L_XONES | RH(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HRROS(void)
{
	// 777777 -> L(E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = WORD18L_XONES | RH(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}


// Halfword Move, Left to Right, Ones

// 564 HLRO   Halfword Move, Left to Right, Ones
// 565 HLROI  Halfword Move, Left to Right, Ones, Immediate
// 566 HLROM  Halfword Move, Left to Right, Ones, Memory
// 567 HLROS  Halfword Move, Left to Right, Ones, Self

void p10_Opcode_HLRO(void)
{
	// 777777,,L(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = WORD18L_XONES | LHSR(BR);
	curAC[opAC] = AR;
}

void p10_Opcode_HLROI(void)
{
	// 777777,,0 -> (AC)

	curAC[opAC] = WORD18L_XONES;
}

void p10_Opcode_HLROM(void)
{
	// 777777,,L(AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = WORD18L_XONES | LHSR(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HLROS(void)
{
	// 777777,,L(E) -> (E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = WORD18L_XONES | LHSR(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}


// Halfword Move, Right to Right, Extend

// 570 HRRE   Halfword Move, Right to Right, Extend
// 571 HRREI  Halfword Move, Right to Right, Extend, Immediate
// 572 HRREM  Halfword Move, Right to Right, Extend, Memory
// 573 HRRES  Halfword Move, Right to Right, Extend, Self

void p10_Opcode_HRRE(void)
{
	// [18(E) * 777777],,R(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = (S18(BR) ? WORD18L_XONES : 0) | RH(BR);
	curAC[opAC] = AR;
}

void p10_Opcode_HRREI(void)
{
	// [18[E] * 777777],,E -> (AC)

	AR = (S18(eAddr) ? WORD18L_XONES : 0) | RH(eAddr);
	curAC[opAC] = AR;
}

void p10_Opcode_HRREM(void)
{
	// [18(AC) * 777777],,R(AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = (S18(curAC[opAC]) ? WORD18L_XONES : 0) | RH(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HRRES(void)
{
	// 18(E) * 777777 -> L(E)
	// if AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = (S18(BR) ? WORD18L_XONES : 0) | RH(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}


// Halfword Move, Left to Right, Extend

// 574 HLRE   Halfword Move, Left to Right, Extend
// 575 HLREI  Halfword Move, Left to Right, Extend, Immediate
// 576 HLREM  Halfword Move, Left to Right, Extend, Memory
// 577 HLRES  Halfword Move, Left to Right, Extend, Self

void p10_Opcode_HLRE(void)
{
	// [0(E) * 777777],,L(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = (S36(BR) ? WORD18L_XONES : 0) | LHSR(BR);
	curAC[opAC] = AR;
}

void p10_Opcode_HLREI(void)
{
	// 0 -> (AC)

	curAC[opAC] = 0;
}

void p10_Opcode_HLREM(void)
{
	// [0(AC) * 777777],,L(AC) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR = (S36(curAC[opAC]) ? WORD18L_XONES : 0) | LHSR(curAC[opAC]);
	p10_vWrite(eAddr, AR, dataMode);
}

void p10_Opcode_HLRES(void)
{
	// [0(E) * 777777],,L(E) -> (E)
	// AC != 0: (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = (S36(BR) ? WORD18L_XONES : 0) | LHSR(BR);
	p10_vWrite(eAddr, AR, dataMode);
	if (opAC)
		curAC[opAC] = AR;
}


// Opcode 600 Series - Logical Testing and Modification

// Test Right, No modifications, Skip if Condition Satisfied
// 600 TRN   Test Right, No Mods, Never Skip
// 602 TRNE  Test Right, No Mods, All Masked Bits Equal to Zero
// 604 TRNA  Test Right, No Mods, Always Skip
// 606 TRNN  Test Right, No Mods, Not All Masked Bits Equal to Zero

// Test Left, No modifications, Skip if Condition Satisfied
// 601 TLN   Test Left, No Mods, Never Skip
// 603 TLNE  Test Left, No Mods, All Masked Bits Equal to Zero
// 605 TLNA  Test Left, No Mods, Always Skip
// 607 TLNN  Test left, No Mods, Not All Masked Bits Equal to Zero

void p10_Opcode_TRN(void)
{
	// No-op
}

void p10_Opcode_TLN(void)
{
	// No-op
}

void p10_Opcode_TRNE(void)
{
	// if R(AC) AND E = 0: Skip

	if ((curAC[opAC] & RH(eAddr)) == 0)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int32 result = (curAC[opAC] & RH(eAddr));
		dbg_Printf("TRNE: %06llo & %06llo = %06o : %s\n",
			RH(curAC[opAC]), RH(eAddr), result,
			(result == 0 ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

void p10_Opcode_TLNE(void)
{
	// if L(AC) AND E = 0: Skip

	if ((curAC[opAC] & RHSL(eAddr)) == 0)
		DO_SKIP;
}

void p10_Opcode_TRNA(void)
{
	// Skip

	DO_SKIP;
}

void p10_Opcode_TLNA(void)
{
	// Skip

	DO_SKIP;
}

void p10_Opcode_TRNN(void)
{
	// if R(AC) AND E != 0: Skip

	if ((curAC[opAC] & RH(eAddr)) != 0)
		DO_SKIP;
}

void p10_Opcode_TLNN(void)
{
	// if L(AC) AND E != 0: Skip

	if ((curAC[opAC] & RHSL(eAddr)) != 0)
		DO_SKIP;
}


// Test Direct, No modifications, Skip if Condition Satisfied
// 610 TDN   Test Direct, No Mods, Never Skip
// 612 TDNE  Test Direct, No Mods, All Masked Bits Equal to Zero
// 614 TDNA  Test Direct, No Mods, Always Skip
// 616 TDNN  Test Direct, No Mods, Not All Masked Bits Equal to Zero

// Test Swapped, No modifications, Skip if Condition Satisfied
// 611 TSN   Test Swapped, No Mods, Never Skip
// 613 TSNE  Test Swapped, No Mods, All Masked Bits Equal to Zero
// 615 TSNA  Test Swapped, No Mods, Always Skip
// 617 TSNN  Test Swapped, No Mods, Not All Masked Bits Equal to Zero


void p10_Opcode_TDN(void)
{
	// No-op
}

void p10_Opcode_TSN(void)
{
	// No-op
}

void p10_Opcode_TDNE(void)
{
	// if (AC) AND (E) = 0: Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);

	if ((curAC[opAC] & BR) == 0)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int36 result = curAC[opAC] & BR;
		dbg_Printf("TDNE: %06o,,%06o & %06o,,%06o = %06o,,%06o : %s\n",
			LH18(curAC[opAC]),RH18(curAC[opAC]), LH18(BR), RH18(BR),
			LH18(result), RH18(result), (result == 0 ? "Skip" : "Continue"));
	}
#endif /* DEBUG */
}

void p10_Opcode_TSNE(void)
{
	// if (AC) AND S(E) = 0: Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);

	if ((curAC[opAC] & SWAP36(BR)) == 0)
		DO_SKIP;
}

void p10_Opcode_TDNA(void)
{
	// Skip

	DO_SKIP;
}

void p10_Opcode_TSNA(void)
{
	// Skip

	DO_SKIP;
}

void p10_Opcode_TDNN(void)
{
	// if (AC) AND (E) != 0: Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	if ((curAC[opAC] & BR) != 0)
		DO_SKIP;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		int36 res = curAC[opAC] & BR;
		AR = curAC[opAC];
		dbg_Printf("TDNN: %c%06o,,%06o & %c%06o,,%06o => %c%06o,,%06o : %s\n",
			((AR < 0) ? '-' : '+'), (int18)LHSR(AR), (int18)RH(AR),
			((BR < 0) ? '-' : '+'), (int18)LHSR(BR), (int18)RH(BR),
			((res < 0) ? '-' : '+'), (int18)LHSR(res), (int18)RH(res),
			(res != 0) ? "Skip" : "Continue");	
	}
#endif /* DEBUG */
}

void p10_Opcode_TSNN(void)
{
	// if (AC) AND S(E) != 0: Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);

	if ((curAC[opAC] & SWAP36(BR)) != 0)
		DO_SKIP;
}


// Test Right, Zeros, Skip if Condition Satisfied
// 620 TRZ   Test Right, Zeros, Never Skip
// 622 TRZE  Test Right, Zeros, All Masked Bits Equal to Zero
// 624 TRZA  Test Right, Zeros, Always Skip
// 626 TRZN  Test Right, Zeros, Not All Masked Bits Equal to Zero

// Test Left, Zeros, Skip if Condition Satisfied
// 621 TLZ   Test Left, Zeros, Never Skip
// 623 TLZE  Test Left, Zeros, All Masked Bits Equal to Zero
// 625 TLZA  Test Left, Zeros, Always Skip
// 627 TLZN  Test left, Zeros, Not All Masked Bits Equal to Zero

void p10_Opcode_TRZ(void)
{
	// R(AC) AND ~E -> R(AC)

	AR = LH(curAC[opAC]) | (RH(curAC[opAC]) & ~RH(eAddr));
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TLZ(void)
{
	// L(AC) AND ~E -> L(AC)

	AR = (LH(curAC[opAC]) & ~RHSL(eAddr)) | RH(curAC[opAC]);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TRZE(void)
{
	// if R(AC) AND E = 0: Skip
	// R(AC) AND ~E -> R(AC)

	AR = curAC[opAC];
	BR = RH(eAddr);
	ARX = LH(AR) | (RH(AR) & ~BR);
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TLZE(void)
{
	// if L(AC) AND E = 0: Skip
	// L(AC) AND ~E -> L(AC)

	AR = curAC[opAC];
	BR = RHSL(eAddr);
	ARX = (LH(AR) & ~BR) | RH(AR);
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TRZA(void)
{
	// R(AC) AND ~E -> R(AC)
	// Skip

	AR = curAC[opAC];
	AR = LH(AR) | (RH(AR) & ~RH(eAddr));
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TLZA(void)
{
	// L(AC) AND ~E -> L(AC)
	// Skip

	AR = curAC[opAC];
	AR = (LH(AR) & ~RHSL(eAddr)) | RH(AR);
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TRZN(void)
{
	// if R(AC) AND E != 0: Skip
	// R(AC) AND ~E -> R(AC)

	AR = curAC[opAC];
	BR = RH(eAddr);
	ARX = LH(AR) | (RH(AR) & ~BR);
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}

void p10_Opcode_TLZN(void)
{
	// if L(AC) AND E != 0: Skip
	// L(AC) AND ~E -> L(AC)

	AR = curAC[opAC];
	BR = RHSL(eAddr);
	ARX = (LH(AR) & ~BR) | RH(AR);
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}


// Test Right, Zeros, Skip if Condition Satisfied
// 630 TDZ   Test Direct, Zeros, Never Skip
// 632 TDZE  Test Direct, Zeros, All Masked Bits Equal to Zero
// 634 TDZA  Test Direct, Zeros, Always Skip
// 636 TDZN  Test Direct, Zeros, Not All Masked Bits Equal to Zero

// Test Left, Zeros, Skip if Condition Satisfied
// 631 TSZ   Test Swapped, Zeros, Never Skip
// 633 TSZE  Test Swapped, Zeros, All Masked Bits Equal to Zero
// 635 TSZA  Test Swapped, Zeros, Always Skip
// 637 TSZN  Test Swapped, Zeros, Not All Masked Bits Equal to Zero

void p10_Opcode_TDZ(void)
{
	// (AC) AND ~(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & ~BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TSZ(void)
{
	// (AC) AND ~S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & ~SWAP36(BR);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TDZE(void)
{
	// if (AC) AND (E) = 0: Skip 
	// (AC) AND ~(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	ARX = AR & ~BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TSZE(void)
{
	// if (AC) AND S(E) = 0: Skip
	// (AC) AND ~S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	BR  = SWAP36(BR);
	ARX = AR & ~BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TDZA(void)
{
	// (AC) AND ~(E) -> (AC)
	// Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & ~BR;
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TSZA(void)
{
	// (AC) AND ~S(E) -> (AC)
	// Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] & ~SWAP36(BR);
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TDZN(void)
{
	// if (AC) AND (E) != 0: Skip
	// (AC) AND ~(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	ARX = AR & ~BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}

void p10_Opcode_TSZN(void)
{
	// if (AC) AND S(E) != 0: Skip
	// (AC) AND ~S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	BR  = SWAP36(BR);
	ARX = AR & ~BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}


// Test Right, Complment, Skip if Condition Satisfied
// 640 TRC   Test Right, Complement, Never Skip
// 642 TRCE  Test Right, Complement, All Masked Bits Equal to Zero
// 644 TRCA  Test Right, Complement, Always Skip
// 646 TRCN  Test Right, Complement, Not All Masked Bits Equal to Zero

// Test Left, Complement, Skip if Condition Satisfied
// 641 TLC   Test Left, Complement, Never Skip
// 643 TLCE  Test Left, Complement, All Masked Bits Equal to Zero
// 645 TLCA  Test Left, Complement, Always Skip
// 647 TLCN  Test Left, Complement, Not All Masked Bits Equal to Zero

void p10_Opcode_TRC(void)
{
	// R(AC) XOR E -> R(AC)

	AR = LH(curAC[opAC]) | (RH(curAC[opAC]) ^ RH(eAddr));
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TLC(void)
{
	// L(AC) XOR E -> L(AC)

	AR = (LH(curAC[opAC]) ^ RHSL(eAddr)) | RH(curAC[opAC]);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TRCE(void)
{
	// if R(AC) AND E = 0: Skip
	// R(AC) XOR E -> R(AC)

	AR  = curAC[opAC];
	BR  = RH(eAddr);
	ARX = LH(AR) | (RH(AR) ^ BR);
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TLCE(void)
{
	// if L(AC) AND E = 0: Skip
	// L(AC) XOR E -> L(AC)

	AR  = curAC[opAC];
	BR  = RHSL(eAddr);
	ARX = (LH(AR) ^ BR) | RH(AR);
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TRCA(void)
{
	// R(AC) XOR E -> R(AC)
	// Skip

	AR = LH(curAC[opAC]) | (RH(curAC[opAC]) ^ RH(eAddr));
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TLCA(void)
{
	// L(AC) XOR E -> L(AC)
	// Skip

	AR = (LH(curAC[opAC]) ^ RHSL(eAddr)) | RH(curAC[opAC]);
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TRCN(void)
{
	// if R(AC) AND E != 0: Skip
	// R(AC) XOR E -> R(AC)

	AR  = curAC[opAC];
	BR  = RH(eAddr);
	ARX = LH(AR) | (RH(AR) ^ BR);
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}

void p10_Opcode_TLCN(void)
{
	// if L(AC) AND E != 0: Skip
	// L(AC) XOR E -> L(AC)

	AR  = curAC[opAC];
	BR  = RHSL(eAddr);
	ARX = (LH(AR) ^ BR) | RH(AR);
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}


// Test Direct, Complment, Skip if Condition Satisfied
// 650 TDC   Test Direct, Complement, Never Skip
// 652 TDCE  Test Direct, Complement, All Masked Bits Equal to Zero
// 654 TDCA  Test Direct, Complement, Always Skip
// 656 TDCN  Test Direct, Complement, Not All Masked Bits Equal to Zero

// Test Swapped, Complement, Skip if Condition Satisfied
// 651 TSC   Test Swapped, Complement, Never Skip
// 653 TSCE  Test Swapped, Complement, All Masked Bits Equal to Zero
// 655 TSCA  Test Swapped, Complement, Always Skip
// 657 TSCN  Test Swapped, Complement, Not All Masked Bits Equal to Zero

void p10_Opcode_TDC(void)
{
	// (AC) XOR (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] ^ BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TSC(void)
{
	// (AC) XOR S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] ^ SWAP36(BR);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TDCE(void)
{
	// if (AC) AND (E) = 0: Skip
	// (AC) XOR (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	ARX = AR ^ BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TSCE(void)
{
	// if (AC) AND S(E) = 0: Skip
	// (AC) XOR S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	BR  = SWAP36(BR);
	ARX = AR ^ BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TDCA(void)
{
	// (AC) XOR (E) -> (AC)
	// Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] ^ BR;
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TSCA(void)
{
	// (AC) XOR S(E) -> (AC)
	// Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] ^ SWAP36(BR);
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TDCN(void)
{
	// if (AC) AND (E) != 0: Skip
	// (AC) XOR (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	ARX = AR ^ BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}

void p10_Opcode_TSCN(void)
{
	// if (AC) AND S(E) != 0: Skip
	// (AC) XOR S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	BR  = SWAP36(BR);
	ARX = AR ^ BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}


// Test Right, Ones, Skip if Condition Satisfied
// 660 TRO   Test Right, Ones, Never Skip
// 662 TROE  Test Right, Ones, All Masked Bits Equal to Zero
// 664 TROA  Test Right, Ones, Always Skip
// 666 TRON  Test Right, Ones, Not All Masked Bits Equal to Zero

// Test Left, Ones, Skip if Condition Satisfied
// 661 TLO   Test Left, Ones, Never Skip
// 663 TLOE  Test Left, Ones, All Masked Bits Equal to Zero
// 665 TLOA  Test Left, Ones, Always Skip
// 667 TLON  Test left, Ones, Not All Masked Bits Equal to Zero

void p10_Opcode_TRO(void)
{
	// R(AC) OR E -> R(AC)

	AR = curAC[opAC] | RH(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TLO(void)
{
	// L(AC) OR E -> L(AC)

	AR = curAC[opAC] | RHSL(eAddr);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TROE(void)
{
	// if R(AC) AND E = 0: Skip
	// R(AC) OR E -> R(AC)

	AR  = curAC[opAC];
	BR  = RH(eAddr);
	ARX = AR | BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TLOE(void)
{
	// if L(AC) AND E = 0: Skip
	// L(AC) OR E -> L(AC)

	AR  = curAC[opAC];
	BR  = RHSL(eAddr);
	ARX = AR | BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TROA(void)
{
	// R(AC) OR E -> R(AC)
	// Skip

	AR = curAC[opAC] | RH(eAddr);
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TLOA(void)
{
	// L(AC) OR E -> L(AC)
	// Skip 

	AR = curAC[opAC] | RHSL(eAddr);
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TRON(void)
{
	// if R(AC) AND E != 0: Skip
	// R(AC) OR E -> R(AC)

	AR  = curAC[opAC];
	BR  = RH(eAddr);
	ARX = AR | BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}

void p10_Opcode_TLON(void)
{
	// if L(AC) AND E != 0: Skip
	// L(AC) OR E -> L(AC)

	AR  = curAC[opAC];
	BR  = RHSL(eAddr);
	ARX = AR | BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}


// Test Direct, Ones, Skip if Condition Satisfied
// 670 TDO   Test Direct, Ones, Never Skip
// 672 TDOE  Test Direct, Ones, All Masked Bits Equal to Zero
// 674 TDOA  Test Direct, Ones, Always Skip
// 676 TDON  Test Direct, Ones, Not All Masked Bits Equal to Zero

// Test Swapped, Ones, Skip if Condition Satisfied
// 671 TSO   Test Swapped, Ones, Never Skip
// 673 TSOE  Test Swapped, Ones, All Masked Bits Equal to Zero
// 675 TSOA  Test Swapped, Ones, Always Skip
// 677 TSON  Test Swapped, Ones, Not All Masked Bits Equal to Zero

void p10_Opcode_TDO(void)
{
	// (AC) OR (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | BR;
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TSO(void)
{
	// (AC) OR S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | SWAP36(BR);
	curAC[opAC] = SXT36(AR);
}

void p10_Opcode_TDOE(void)
{
	// if (AC) AND (E) = 0: Skip
	// (AC) OR (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	ARX = AR | BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TSOE(void)
{
	// if (AC) AND S(E) = 0: Skip
	// (AC) OR S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	BR  = SWAP36(BR);
	ARX = AR | BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) == 0)
		DO_SKIP;
}

void p10_Opcode_TDOA(void)
{
	// (AC) OR (E) -> (AC)
	// Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | BR;
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TSOA(void)
{
	// (AC) OR S(E) -> (AC)
	// Skip

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR = curAC[opAC] | SWAP36(BR);
	curAC[opAC] = SXT36(AR);

	DO_SKIP;
}

void p10_Opcode_TDON(void)
{
	// if (AC) AND (E) != 0: Skip
	// (AC) OR (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	ARX = AR | BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}

void p10_Opcode_TSON(void)
{
	// if (AC) AND S(E) != 0: Skip
	// (AC) OR S(E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	BR = p10_vRead(eAddr, dataMode);
	AR  = curAC[opAC];
	BR  = SWAP36(BR);
	ARX = AR | BR;
	curAC[opAC] = SXT36(ARX);

	if ((AR & BR) != 0)
		DO_SKIP;
}


// Opcode 700 Series - IO instructions

// 704 UMOVE  User Move 
// This is for KS10 processor only
void KS10_Opcode_UMOVE(void)
{
	// PXCT 4,[MOVE A,E]

	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0))
		KS10_Opcode_UUO();
	else {
		AR = p10_vRead(eAddr, PTF_PREV|PTF_USER);
		curAC[opAC] = AR;

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			if (eAddr < 020)
				dbg_Printf("UMOVE: AC[%d,%02o]", PACB, eAddr);
			else
				dbg_Printf("UMOVE: E[%06llo]", RH(eAddr));
			dbg_Printf(" -> AC[%d,%02o]: %06llo,,%06llo\n",
				CACB, opAC, LHSR(AR), RH(AR));
		}
#endif /* DEBUG */

	}
}

// 705 UMOVEM  User Move to Memory
// This is for KS10 processor only
void KS10_Opcode_UMOVEM(void)
{
	// PXCT 4,[MOVEM A,E]

	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0))
		KS10_Opcode_UUO();
	else {
		AR = curAC[opAC];
		p10_vWrite(eAddr, AR, PTF_PREV|PTF_USER);

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("UMOVEM: AC[%d,%02o]", CACB, opAC);
			if (eAddr < 020)
				dbg_Printf(" -> AC[%d,%02o]", PACB, eAddr);
			else
				dbg_Printf(" -> E[%06llo]", RH(eAddr));
			dbg_Printf(": %06llo,,%06llo\n", LHSR(AR), RH(AR));
		}
#endif /* DEBUG */

	}
}

/***************************************************/

extern CLK_QUEUE kl10_Timer;

int p10_Go(MAP_DEVICE *map)
{
	int abValue; // Abort value
	int pid;     // Child process ID

	// Initialize IPS meter
	cips  = 0;
	jiffy = 0;
	savedMode = 0;
	KX10_IsGlobal = FALSE;
	KX10_IntrQ = 0;
	p10_State = EMU_RUN;
	p10_CacheMisses = 0;
	p10_CacheHits = 0;

	// Switch to run state for TTY console.
	ts10_StartTimer();
	ts10_SetAlarm(p10_HandleTimer);

	// Enable KL10 real timer
	if (ISCPU(CNF_KL10))
		ts10_SetRealTimer(&kl10_Timer);

	abValue = setjmp(p10_SetJump);
	if (abValue > 0) {
		ts10_StopTimer();
		return abValue;
	} else if (abValue < 0) {
		switch (abValue) {
			case P10_INTERRUPT:
			case P10_ABORT:
				// Interrupted/Aborted out of instruction
				break;

			default:
				KX10_PageTrap2();
		}
	}

	while (p10_State == EMU_RUN) {
		pager_PC = PC; // Save base address for trap.
		cpu_pFlags &= CPU_CYCLE_PI;  // Reset process flags.
		KX10_IsGlobal = FALSE; // Instruction fetches always are local.

//		if (PC == 0247) {
//			dbg_SetMode(DBG_TRACE|DBG_DATA|DBG_IOREGS|DBG_IODATA|DBG_PAGEFAULT);
//			printf("[Trace/Data On]\n");
//		}

		// ACTION: Need that to being set by timer.

		if (ts10_ClkInterval-- <= 0)
			ts10_ExecuteTimer();

		if (KX10_IntrQ) {
			KX10_piProcess();
			continue;
		}

		p10_Execute(PC, 0);
	}

	// Switch back to command state.
	ts10_StopTimer();
	return EMU_OK;
}

void p10_Initialize(P10_CPU *cpu)
{
	int i, j;

	p10 = cpu;

	printf("CPU Flags = %08X\n", p10->cnfFlags);

	// Basic Instruction Table Initialization

	// Opcode 000 Series

	if (ISCPU(CNF_KS10)) {
		// Opcode 000 - Illegal Instruction
		basOpcode[0] = KS10_Opcode_UUO;

		// Initialize LUUO opcodes
		for (i = 0001; i <= 0037; i++)
			basOpcode[i] = KS10_Opcode_LUUO;

		// Initialize MUUO opcodes
		for (i = 0040; i <= 0177; i++)
			basOpcode[i] = KS10_Opcode_UUO;
	}

	if (ISCPU(CNF_KL10)) {
		// Opcode 000 - Illegal Instruction
		basOpcode[0] = KL10_Opcode_UUO;

		// Initialize LUUO opcodes
		for (i = 0001; i <= 0037; i++)
			basOpcode[i] = KL10_Opcode_LUUO;

		// Initialize MUUO opcodes
		for (i = 0040; i <= 0177; i++)
			basOpcode[i] = KL10_Opcode_UUO;
	}

	// Opcode 100 Series
	basOpcode[0105] = p10_Opcode_ADJSP;

	basOpcode[0110] = p10_Opcode_DFAD;
	basOpcode[0111] = p10_Opcode_DFSB;
	basOpcode[0112] = p10_Opcode_DFMP;
	basOpcode[0113] = p10_Opcode_DFDV;

	basOpcode[0114] = p10_Opcode_DADD;
	basOpcode[0115] = p10_Opcode_DSUB;
	basOpcode[0116] = p10_Opcode_DMUL;
	basOpcode[0117] = p10_Opcode_DDIV;

	basOpcode[0120] = p10_Opcode_DMOVE;
	basOpcode[0121] = p10_Opcode_DMOVN;
	basOpcode[0122] = p10_Opcode_FIX;
	basOpcode[0123] = p10_Opcode_EXTEND;
	basOpcode[0124] = p10_Opcode_DMOVEM;
	basOpcode[0125] = p10_Opcode_DMOVNM;
	basOpcode[0126] = p10_Opcode_FIXR;
	basOpcode[0127] = p10_Opcode_FLTR;

	basOpcode[0132] = p10_Opcode_FSC;
	basOpcode[0133] = p10_Opcode_IBP;
	basOpcode[0134] = p10_Opcode_ILDB;
	basOpcode[0135] = p10_Opcode_LDB;
	basOpcode[0136] = p10_Opcode_IDPB;
	basOpcode[0137] = p10_Opcode_DPB;

	basOpcode[0140] = p10_Opcode_FAD;
	basOpcode[0142] = p10_Opcode_FADM;
	basOpcode[0143] = p10_Opcode_FADB;
	basOpcode[0144] = p10_Opcode_FADR;
	basOpcode[0145] = p10_Opcode_FADRI;
	basOpcode[0146] = p10_Opcode_FADRM;
	basOpcode[0147] = p10_Opcode_FADRB;

	basOpcode[0150] = p10_Opcode_FSB;
	basOpcode[0152] = p10_Opcode_FSBM;
	basOpcode[0153] = p10_Opcode_FSBB;
	basOpcode[0154] = p10_Opcode_FSBR;
	basOpcode[0155] = p10_Opcode_FSBRI;
	basOpcode[0156] = p10_Opcode_FSBRM;
	basOpcode[0157] = p10_Opcode_FSBRB;

	basOpcode[0160] = p10_Opcode_FMP;
	basOpcode[0162] = p10_Opcode_FMPM;
	basOpcode[0163] = p10_Opcode_FMPB;
	basOpcode[0164] = p10_Opcode_FMPR;
	basOpcode[0165] = p10_Opcode_FMPRI;
	basOpcode[0166] = p10_Opcode_FMPRM;
	basOpcode[0167] = p10_Opcode_FMPRB;

	basOpcode[0170] = p10_Opcode_FDV;
	basOpcode[0172] = p10_Opcode_FDVM;
	basOpcode[0173] = p10_Opcode_FDVB;
	basOpcode[0174] = p10_Opcode_FDVR;
	basOpcode[0175] = p10_Opcode_FDVRI;
	basOpcode[0176] = p10_Opcode_FDVRM;
	basOpcode[0177] = p10_Opcode_FDVRB;

	// Opcode 200 Series 
	basOpcode[0200] = p10_Opcode_MOVE;
	basOpcode[0201] = p10_Opcode_MOVEI;
	basOpcode[0202] = p10_Opcode_MOVEM;
	basOpcode[0203] = p10_Opcode_MOVES;

	basOpcode[0204] = p10_Opcode_MOVS;
	basOpcode[0205] = p10_Opcode_MOVSI;
	basOpcode[0206] = p10_Opcode_MOVSM;
	basOpcode[0207] = p10_Opcode_MOVSS;

	basOpcode[0210] = p10_Opcode_MOVN;
	basOpcode[0211] = p10_Opcode_MOVNI;
	basOpcode[0212] = p10_Opcode_MOVNM;
	basOpcode[0213] = p10_Opcode_MOVNS;

	basOpcode[0214] = p10_Opcode_MOVM;
	basOpcode[0215] = p10_Opcode_MOVMI;
	basOpcode[0216] = p10_Opcode_MOVMM;
	basOpcode[0217] = p10_Opcode_MOVMS;

	basOpcode[0220] = p10_Opcode_IMUL;
	basOpcode[0221] = p10_Opcode_IMULI;
	basOpcode[0222] = p10_Opcode_IMULM;
	basOpcode[0223] = p10_Opcode_IMULB;

	basOpcode[0224] = p10_Opcode_MUL;
	basOpcode[0225] = p10_Opcode_MULI;
	basOpcode[0226] = p10_Opcode_MULM;
	basOpcode[0227] = p10_Opcode_MULB;

	basOpcode[0230] = p10_Opcode_IDIV;
	basOpcode[0231] = p10_Opcode_IDIVI;
	basOpcode[0232] = p10_Opcode_IDIVM;
	basOpcode[0233] = p10_Opcode_IDIVB;

	basOpcode[0234] = p10_Opcode_DIV;
	basOpcode[0235] = p10_Opcode_DIVI;
	basOpcode[0236] = p10_Opcode_DIVM;
	basOpcode[0237] = p10_Opcode_DIVB;

	basOpcode[0240] = p10_Opcode_ASH;
	basOpcode[0241] = p10_Opcode_ROT;
	basOpcode[0242] = p10_Opcode_LSH;
	basOpcode[0243] = p10_Opcode_JFFO;

	basOpcode[0244] = p10_Opcode_ASHC;
	basOpcode[0245] = p10_Opcode_ROTC;
	basOpcode[0246] = p10_Opcode_LSHC;
	if (ISCPU(CNF_KS10))
		basOpcode[0247] = KS10_Opcode_UUO; // Opcode 247 - Unused
	if (ISCPU(CNF_KL10))
		basOpcode[0247] = KL10_Opcode_UUO; // Opcode 247 - Unused

	basOpcode[0250] = p10_Opcode_EXCH;
	basOpcode[0251] = KX10_Opcode_BLT;
	basOpcode[0252] = p10_Opcode_AOBJP;
	basOpcode[0253] = p10_Opcode_AOBJN;

	basOpcode[0254] = KS10_Opcode_JRST;
	basOpcode[0255] = p10_Opcode_JFCL;
	if (ISCPU(CNF_KS10)) {
		basOpcode[0256] = p10_Opcode_XCT;
		basOpcode[0257] = KS10_Opcode_MAP;
	}
	if (ISCPU(CNF_KL10)) {
		basOpcode[0256] = KL10_Opcode_XCT;
		basOpcode[0257] = KL10_Opcode_MAP;
	}

	basOpcode[0260] = p10_Opcode_PUSHJ;
	basOpcode[0261] = p10_Opcode_PUSH;
	basOpcode[0262] = p10_Opcode_POP;
	basOpcode[0263] = p10_Opcode_POPJ;

	basOpcode[0264] = p10_Opcode_JSR;
	basOpcode[0265] = p10_Opcode_JSP;
	basOpcode[0266] = p10_Opcode_JSA;
	basOpcode[0267] = p10_Opcode_JRA;

	basOpcode[0270] = p10_Opcode_ADD;
	basOpcode[0271] = p10_Opcode_ADDI;
	basOpcode[0272] = p10_Opcode_ADDM;
	basOpcode[0273] = p10_Opcode_ADDB;

	basOpcode[0274] = p10_Opcode_SUB;
	basOpcode[0275] = p10_Opcode_SUBI;
	basOpcode[0276] = p10_Opcode_SUBM;
	basOpcode[0277] = p10_Opcode_SUBB;

	// Opcode 300 Series
	basOpcode[0300] = p10_Opcode_CAI; // CAI
	basOpcode[0301] = p10_Opcode_CAI; // CAIL
	basOpcode[0302] = p10_Opcode_CAI; // CAIE
	basOpcode[0303] = p10_Opcode_CAI; // CAILE
	basOpcode[0304] = p10_Opcode_CAI; // CAIA
	basOpcode[0305] = p10_Opcode_CAI; // CAIGE
	basOpcode[0306] = p10_Opcode_CAI; // CAIN
	basOpcode[0307] = p10_Opcode_CAI; // CAIG

	basOpcode[0310] = p10_Opcode_CAM; // CAM
	basOpcode[0311] = p10_Opcode_CAM; // CAML
	basOpcode[0312] = p10_Opcode_CAM; // CAME
	basOpcode[0313] = p10_Opcode_CAM; // CAMLE
	basOpcode[0314] = p10_Opcode_CAM; // CAMA
	basOpcode[0315] = p10_Opcode_CAM; // CAMGE
	basOpcode[0316] = p10_Opcode_CAM; // CAMN
	basOpcode[0317] = p10_Opcode_CAM; // CAMG

	basOpcode[0320] = p10_Opcode_JUMP; // JUMP
	basOpcode[0321] = p10_Opcode_JUMP; // JUMPL
	basOpcode[0322] = p10_Opcode_JUMP; // JUMPE
	basOpcode[0323] = p10_Opcode_JUMP; // JUMPLE
	basOpcode[0324] = p10_Opcode_JUMP; // JUMPA
	basOpcode[0325] = p10_Opcode_JUMP; // JUMPGE
	basOpcode[0326] = p10_Opcode_JUMP; // JUMPN
	basOpcode[0327] = p10_Opcode_JUMP; // JUMPG

	basOpcode[0330] = p10_Opcode_SKIP; // SKIP
	basOpcode[0331] = p10_Opcode_SKIP; // SKIPL
	basOpcode[0332] = p10_Opcode_SKIP; // SKIPE
	basOpcode[0333] = p10_Opcode_SKIP; // SKIPLE
	basOpcode[0334] = p10_Opcode_SKIP; // SKIPA
	basOpcode[0335] = p10_Opcode_SKIP; // SKIPGE
	basOpcode[0336] = p10_Opcode_SKIP; // SKIPN
	basOpcode[0337] = p10_Opcode_SKIP; // SKIPG

	basOpcode[0340] = p10_Opcode_AOJ; // AOJ
	basOpcode[0341] = p10_Opcode_AOJ; // AOJL
	basOpcode[0342] = p10_Opcode_AOJ; // AOJE
	basOpcode[0343] = p10_Opcode_AOJ; // AOJLE
	basOpcode[0344] = p10_Opcode_AOJ; // AOJA
	basOpcode[0345] = p10_Opcode_AOJ; // AOJGE
	basOpcode[0346] = p10_Opcode_AOJ; // AOJN
	basOpcode[0347] = p10_Opcode_AOJ; // AOJG

	basOpcode[0350] = p10_Opcode_AOS; // AOS
	basOpcode[0351] = p10_Opcode_AOS; // AOSL
	basOpcode[0352] = p10_Opcode_AOS; // AOSE
	basOpcode[0353] = p10_Opcode_AOS; // AOSLE
	basOpcode[0354] = p10_Opcode_AOS; // AOSA
	basOpcode[0355] = p10_Opcode_AOS; // AOSGE
	basOpcode[0356] = p10_Opcode_AOS; // AOSN
	basOpcode[0357] = p10_Opcode_AOS; // AOSG

	basOpcode[0360] = p10_Opcode_SOJ; // SOJ
	basOpcode[0361] = p10_Opcode_SOJ; // SOJL
	basOpcode[0362] = p10_Opcode_SOJ; // SOJE
	basOpcode[0363] = p10_Opcode_SOJ; // SOJLE
	basOpcode[0364] = p10_Opcode_SOJ; // SOJA
	basOpcode[0365] = p10_Opcode_SOJ; // SOJGE
	basOpcode[0366] = p10_Opcode_SOJ; // SOJN
	basOpcode[0367] = p10_Opcode_SOJ; // SOJG

	basOpcode[0370] = p10_Opcode_SOS; // SOS
	basOpcode[0371] = p10_Opcode_SOS; // SOSL
	basOpcode[0372] = p10_Opcode_SOS; // SOSE
	basOpcode[0373] = p10_Opcode_SOS; // SOSLE
	basOpcode[0374] = p10_Opcode_SOS; // SOSA
	basOpcode[0375] = p10_Opcode_SOS; // SOSGE
	basOpcode[0376] = p10_Opcode_SOS; // SOSN
	basOpcode[0377] = p10_Opcode_SOS; // SOSG

	// Opcode 400 Series
	basOpcode[0400] = p10_Opcode_SETZ;
	basOpcode[0401] = p10_Opcode_SETZI;
	basOpcode[0402] = p10_Opcode_SETZM;
	basOpcode[0403] = p10_Opcode_SETZB;

	basOpcode[0404] = p10_Opcode_AND;
	basOpcode[0405] = p10_Opcode_ANDI;
	basOpcode[0406] = p10_Opcode_ANDM;
	basOpcode[0407] = p10_Opcode_ANDB;

	basOpcode[0410] = p10_Opcode_ANDCA;
	basOpcode[0411] = p10_Opcode_ANDCAI;
	basOpcode[0412] = p10_Opcode_ANDCAM;
	basOpcode[0413] = p10_Opcode_ANDCAB;

	basOpcode[0414] = p10_Opcode_SETM;
	basOpcode[0415] = p10_Opcode_SETMI;
	basOpcode[0416] = p10_Opcode_SETMM;
	basOpcode[0417] = p10_Opcode_SETMB;

	basOpcode[0420] = p10_Opcode_ANDCM;
	basOpcode[0421] = p10_Opcode_ANDCMI;
	basOpcode[0422] = p10_Opcode_ANDCMM;
	basOpcode[0423] = p10_Opcode_ANDCMB;

	basOpcode[0424] = p10_Opcode_SETA;
	basOpcode[0425] = p10_Opcode_SETAI;
	basOpcode[0426] = p10_Opcode_SETAM;
	basOpcode[0427] = p10_Opcode_SETAB;

	basOpcode[0430] = p10_Opcode_XOR;
	basOpcode[0431] = p10_Opcode_XORI;
	basOpcode[0432] = p10_Opcode_XORM;
	basOpcode[0433] = p10_Opcode_XORB;

	basOpcode[0434] = p10_Opcode_IOR;
	basOpcode[0435] = p10_Opcode_IORI;
	basOpcode[0436] = p10_Opcode_IORM;
	basOpcode[0437] = p10_Opcode_IORB;

	basOpcode[0440] = p10_Opcode_ANDCB;
	basOpcode[0441] = p10_Opcode_ANDCBI;
	basOpcode[0442] = p10_Opcode_ANDCBM;
	basOpcode[0443] = p10_Opcode_ANDCBB;

	basOpcode[0444] = p10_Opcode_EQV;
	basOpcode[0445] = p10_Opcode_EQVI;
	basOpcode[0446] = p10_Opcode_EQVM;
	basOpcode[0447] = p10_Opcode_EQVB;

	basOpcode[0450] = p10_Opcode_SETCA;
	basOpcode[0451] = p10_Opcode_SETCAI;
	basOpcode[0452] = p10_Opcode_SETCAM;
	basOpcode[0453] = p10_Opcode_SETCAB;

	basOpcode[0454] = p10_Opcode_ORCA;
	basOpcode[0455] = p10_Opcode_ORCAI;
	basOpcode[0456] = p10_Opcode_ORCAM;
	basOpcode[0457] = p10_Opcode_ORCAB;

	basOpcode[0460] = p10_Opcode_SETCM;
	basOpcode[0461] = p10_Opcode_SETCMI;
	basOpcode[0462] = p10_Opcode_SETCMM;
	basOpcode[0463] = p10_Opcode_SETCMB;

	basOpcode[0464] = p10_Opcode_ORCM;
	basOpcode[0465] = p10_Opcode_ORCMI;
	basOpcode[0466] = p10_Opcode_ORCMM;
	basOpcode[0467] = p10_Opcode_ORCMB;

	basOpcode[0470] = p10_Opcode_ORCB;
	basOpcode[0471] = p10_Opcode_ORCBI;
	basOpcode[0472] = p10_Opcode_ORCBM;
	basOpcode[0473] = p10_Opcode_ORCBB;

	basOpcode[0474] = p10_Opcode_SETO;
	basOpcode[0475] = p10_Opcode_SETOI;
	basOpcode[0476] = p10_Opcode_SETOM;
	basOpcode[0477] = p10_Opcode_SETOB;

	// Opcode 500 Series
	basOpcode[0500] = p10_Opcode_HLL;
	basOpcode[0501] = p10_Opcode_HLLI;
	basOpcode[0502] = p10_Opcode_HLLM;
	basOpcode[0503] = p10_Opcode_HLLS;

	basOpcode[0504] = p10_Opcode_HRL;
	basOpcode[0505] = p10_Opcode_HRLI;
	basOpcode[0506] = p10_Opcode_HRLM;
	basOpcode[0507] = p10_Opcode_HRLS;

	basOpcode[0510] = p10_Opcode_HLLZ;
	basOpcode[0511] = p10_Opcode_HLLZI;
	basOpcode[0512] = p10_Opcode_HLLZM;
	basOpcode[0513] = p10_Opcode_HLLZS;

	basOpcode[0514] = p10_Opcode_HRLZ;
	basOpcode[0515] = p10_Opcode_HRLZI;
	basOpcode[0516] = p10_Opcode_HRLZM;
	basOpcode[0517] = p10_Opcode_HRLZS;

	basOpcode[0520] = p10_Opcode_HLLO;
	basOpcode[0521] = p10_Opcode_HLLOI;
	basOpcode[0522] = p10_Opcode_HLLOM;
	basOpcode[0523] = p10_Opcode_HLLOS;

	basOpcode[0524] = p10_Opcode_HRLO;
	basOpcode[0525] = p10_Opcode_HRLOI;
	basOpcode[0526] = p10_Opcode_HRLOM;
	basOpcode[0527] = p10_Opcode_HRLOS;

	basOpcode[0530] = p10_Opcode_HLLE;
	basOpcode[0531] = p10_Opcode_HLLEI;
	basOpcode[0532] = p10_Opcode_HLLEM;
	basOpcode[0533] = p10_Opcode_HLLES;

	basOpcode[0534] = p10_Opcode_HRLE;
	basOpcode[0535] = p10_Opcode_HRLEI;
	basOpcode[0536] = p10_Opcode_HRLEM;
	basOpcode[0537] = p10_Opcode_HRLES;

	basOpcode[0540] = p10_Opcode_HRR;
	basOpcode[0541] = p10_Opcode_HRRI;
	basOpcode[0542] = p10_Opcode_HRRM;
	basOpcode[0543] = p10_Opcode_HRRS;

	basOpcode[0544] = p10_Opcode_HLR;
	basOpcode[0545] = p10_Opcode_HLRI;
	basOpcode[0546] = p10_Opcode_HLRM;
	basOpcode[0547] = p10_Opcode_HLRS;

	basOpcode[0550] = p10_Opcode_HRRZ;
	basOpcode[0551] = p10_Opcode_HRRZI;
	basOpcode[0552] = p10_Opcode_HRRZM;
	basOpcode[0553] = p10_Opcode_HRRZS;

	basOpcode[0554] = p10_Opcode_HLRZ;
	basOpcode[0555] = p10_Opcode_HLRZI;
	basOpcode[0556] = p10_Opcode_HLRZM;
	basOpcode[0557] = p10_Opcode_HLRZS;

	basOpcode[0560] = p10_Opcode_HRRO;
	basOpcode[0561] = p10_Opcode_HRROI;
	basOpcode[0562] = p10_Opcode_HRROM;
	basOpcode[0563] = p10_Opcode_HRROS;

	basOpcode[0564] = p10_Opcode_HLRO;
	basOpcode[0565] = p10_Opcode_HLROI;
	basOpcode[0566] = p10_Opcode_HLROM;
	basOpcode[0567] = p10_Opcode_HLROS;

	basOpcode[0570] = p10_Opcode_HRRE;
	basOpcode[0571] = p10_Opcode_HRREI;
	basOpcode[0572] = p10_Opcode_HRREM;
	basOpcode[0573] = p10_Opcode_HRRES;

	basOpcode[0574] = p10_Opcode_HLRE;
	basOpcode[0575] = p10_Opcode_HLREI;
	basOpcode[0576] = p10_Opcode_HLREM;
	basOpcode[0577] = p10_Opcode_HLRES;
	
	// Opcode 600 Series
	basOpcode[0600] = p10_Opcode_TRN;
	basOpcode[0601] = p10_Opcode_TLN;
	basOpcode[0602] = p10_Opcode_TRNE;
	basOpcode[0603] = p10_Opcode_TLNE;
	basOpcode[0604] = p10_Opcode_TRNA;
	basOpcode[0605] = p10_Opcode_TLNA;
	basOpcode[0606] = p10_Opcode_TRNN;
	basOpcode[0607] = p10_Opcode_TLNN;

	basOpcode[0610] = p10_Opcode_TDN;
	basOpcode[0611] = p10_Opcode_TSN;
	basOpcode[0612] = p10_Opcode_TDNE;
	basOpcode[0613] = p10_Opcode_TSNE;
	basOpcode[0614] = p10_Opcode_TDNA;
	basOpcode[0615] = p10_Opcode_TSNA;
	basOpcode[0616] = p10_Opcode_TDNN;
	basOpcode[0617] = p10_Opcode_TSNN;

	basOpcode[0620] = p10_Opcode_TRZ;
	basOpcode[0621] = p10_Opcode_TLZ;
	basOpcode[0622] = p10_Opcode_TRZE;
	basOpcode[0623] = p10_Opcode_TLZE;
	basOpcode[0624] = p10_Opcode_TRZA;
	basOpcode[0625] = p10_Opcode_TLZA;
	basOpcode[0626] = p10_Opcode_TRZN;
	basOpcode[0627] = p10_Opcode_TLZN;

	basOpcode[0630] = p10_Opcode_TDZ;
	basOpcode[0631] = p10_Opcode_TSZ;
	basOpcode[0632] = p10_Opcode_TDZE;
	basOpcode[0633] = p10_Opcode_TSZE;
	basOpcode[0634] = p10_Opcode_TDZA;
	basOpcode[0635] = p10_Opcode_TSZA;
	basOpcode[0636] = p10_Opcode_TDZN;
	basOpcode[0637] = p10_Opcode_TSZN;

	basOpcode[0640] = p10_Opcode_TRC;
	basOpcode[0641] = p10_Opcode_TLC;
	basOpcode[0642] = p10_Opcode_TRCE;
	basOpcode[0643] = p10_Opcode_TLCE;
	basOpcode[0644] = p10_Opcode_TRCA;
	basOpcode[0645] = p10_Opcode_TLCA;
	basOpcode[0646] = p10_Opcode_TRCN;
	basOpcode[0647] = p10_Opcode_TLCN;

	basOpcode[0650] = p10_Opcode_TDC;
	basOpcode[0651] = p10_Opcode_TSC;
	basOpcode[0652] = p10_Opcode_TDCE;
	basOpcode[0653] = p10_Opcode_TSCE;
	basOpcode[0654] = p10_Opcode_TDCA;
	basOpcode[0655] = p10_Opcode_TSCA;
	basOpcode[0656] = p10_Opcode_TDCN;
	basOpcode[0657] = p10_Opcode_TSCN;

	basOpcode[0660] = p10_Opcode_TRO;
	basOpcode[0661] = p10_Opcode_TLO;
	basOpcode[0662] = p10_Opcode_TROE;
	basOpcode[0663] = p10_Opcode_TLOE;
	basOpcode[0664] = p10_Opcode_TROA;
	basOpcode[0665] = p10_Opcode_TLOA;
	basOpcode[0666] = p10_Opcode_TRON;
	basOpcode[0667] = p10_Opcode_TLON;

	basOpcode[0670] = p10_Opcode_TDO;
	basOpcode[0671] = p10_Opcode_TSO;
	basOpcode[0672] = p10_Opcode_TDOE;
	basOpcode[0673] = p10_Opcode_TSOE;
	basOpcode[0674] = p10_Opcode_TDOA;
	basOpcode[0675] = p10_Opcode_TSOA;
	basOpcode[0676] = p10_Opcode_TDON;
	basOpcode[0677] = p10_Opcode_TSON;

	// Opcode 700 Series
	if (ISCPU(CNF_KS10)) {
		for (i = 0700; i <= 0777; i++)
			basOpcode[i] = KS10_Opcode_UUO;

		basOpcode[0700] = KS10_Opcode_IO700;
		basOpcode[0701] = KS10_Opcode_IO701;
		basOpcode[0702] = KS10_Opcode_IO702;

		basOpcode[0704] = KS10_Opcode_UMOVE;
		basOpcode[0705] = KS10_Opcode_UMOVEM;

		basOpcode[0710] = KS10_Opcode_TIOE;
		basOpcode[0711] = KS10_Opcode_TION;
		basOpcode[0712] = KS10_Opcode_RDIO;
		basOpcode[0713] = KS10_Opcode_WRIO;
		basOpcode[0714] = KS10_Opcode_BSIO;
		basOpcode[0715] = KS10_Opcode_BCIO;
		basOpcode[0716] = KS10_Opcode_BLTBU;
		basOpcode[0717] = KS10_Opcode_BLTUB;

		basOpcode[0720] = KS10_Opcode_TIOEB;
		basOpcode[0721] = KS10_Opcode_TIONB;
		basOpcode[0722] = KS10_Opcode_RDIOB;
		basOpcode[0723] = KS10_Opcode_WRIOB;
		basOpcode[0724] = KS10_Opcode_BSIOB;
		basOpcode[0725] = KS10_Opcode_BCIOB;
	}

	if (ISCPU(CNF_KL10)) {
		for (i = 0700; i <= 0777; i++)
			basOpcode[i] = KX10_Opcode_IO;
	}

	// EXTEND Instruction Table Initialization

	// Initialize default MUUO Instruction.
	for (i = 0; i < 01000; i++)
		extOpcode[i] = p10_extOpcode_EUUO;

	// CMPS and EDIT instructions
	extOpcode[0001] = p10_extOpcode_CMPS;
	extOpcode[0002] = p10_extOpcode_CMPS;
	extOpcode[0003] = p10_extOpcode_CMPS;
	extOpcode[0004] = p10_extOpcode_EDIT;
	extOpcode[0005] = p10_extOpcode_CMPS;
	extOpcode[0006] = p10_extOpcode_CMPS;
	extOpcode[0007] = p10_extOpcode_CMPS;

	// CVTDB and CVTBD instructions
	extOpcode[0010] = p10_extOpcode_CVTDB;
	extOpcode[0011] = p10_extOpcode_CVTDB;
	extOpcode[0012] = p10_extOpcode_CVTBD;
	extOpcode[0013] = p10_extOpcode_CVTBD;

	// MOVSx instructions
	extOpcode[0014] = p10_extOpcode_MOVS;
	extOpcode[0015] = p10_extOpcode_MOVS;
	extOpcode[0016] = p10_extOpcode_MOVS;
	extOpcode[0017] = p10_extOpcode_MOVS;

	// Initialize CPU registers
	p10_ResetCPU();

	// Now build instruction table for assembler/disassembler

	for (i = 0; i < 0700; i++)
		pdp10_Opcode[i] = &pdp10_Instruction[0];

	for (i = 0; i < 01000; i++)
		pdp10_OpcodeEXT[i] = &pdp10_Instruction[0];

	for (i = 0; i < 02000; i++)
		pdp10_OpcodeIO[i] = NULL;

	for (i = 0; i < 0200; i++)
		pdp10_OpcodeDEV[i] = NULL;

	for (i = 0; i < 010; i++)
		pdp10_OpcodeFUNC[i] = NULL;

	for (i = 1; pdp10_Instruction[i].Name; i++) {
		int32 Flags  = pdp10_Instruction[i].Flags;
		int32 Opcode = pdp10_Instruction[i].Opcode;
		int   cpuid;

		if (ISCPU(CNF_KS10))
			cpuid = OP_KS;

		if (ISCPU(CNF_KL10))
			cpuid = OP_KL;

		if (Flags & cpuid) {
			if (!(Flags & (OP_EXT|OP_IO|OP_DEV|OP_FUNC))) {
				if ((pdp10_Opcode[Opcode] == NULL) ||
				    (pdp10_Opcode[Opcode]->Opcode == 0))
					pdp10_Opcode[Opcode] = &pdp10_Instruction[i];

				if (Flags & OP_AC) {
					switch ((Flags & 060) >> 4) {
						case 0: 
							for (j = 0; j < 020; j++) 
								pdp10_Opcode[Opcode]->AC[j] = NULL;
							break;

						case 1:
							pdp10_Opcode[Opcode]->AC[0] =
								&pdp10_Instruction[i];
							break;

						case 2:	
							for (j = 1; j < 020; j++) 
								pdp10_Opcode[Opcode]->AC[j] =
									&pdp10_Instruction[i];
							break;

						case 3:
							pdp10_Opcode[Opcode]->AC[Flags & 017] =
								&pdp10_Instruction[i];
							break;
					}
				}
			} else if (Flags & OP_EXT) {
				pdp10_OpcodeEXT[Opcode] = &pdp10_Instruction[i];
			} else if (Flags & OP_IO) {
				Opcode = (pdp10_Instruction[i].Opcode >> 2) & 01777;
				pdp10_OpcodeIO[Opcode] = &pdp10_Instruction[i];
			} else if (Flags & OP_DEV) {
				Opcode = pdp10_Instruction[i].Opcode >> 2;
				pdp10_OpcodeDEV[Opcode] = &pdp10_Instruction[i];
			} else if (Flags & OP_FUNC) {
				Opcode = (pdp10_Instruction[i].Opcode >> 2) & 07;
				pdp10_OpcodeFUNC[Opcode] = &pdp10_Instruction[i];
			}
		}
	}

#ifdef DEBUG

	if (dbg_Check(DBG_TABLE)) {

		// Dump Instruction Table for Debugging Purpose
		for (i = 0; i < 01000; i++) {
			dbg_Printf("Opcode %03o: %s\n", i,
				pdp10_Opcode[i]->Name);
			if (pdp10_Opcode[i]->Flags & OP_AC) {
				for (j = 0; j < 020; j++) {
					if (pdp10_Opcode[i]->AC[j])
						dbg_Printf("  AC %02o: %s\n", j,
							pdp10_Opcode[i]->AC[j]->Name);
				}
			}
			dbg_Printf("\n");
		}

		// I/O Instrctions - Specific functions
		for (i = 0; i < 02000; i++) {
			if (pdp10_OpcodeIO[i]) {
				dbg_Printf("Opcode %05o: %s\n", (i << 2) | 070000,
					pdp10_OpcodeIO[i]->Name);
			}
		}
		dbg_Printf("\n");

		// I/O Instructions - Device Codes
		for (i = 0; i < 0200; i++) {
			if (pdp10_OpcodeDEV[i]) {
				dbg_Printf("Opcode %05o: %s\n", (i << 2),
					pdp10_OpcodeDEV[i]->Name);
			}
		}
		dbg_Printf("\n");

		// I/O Instruction - Function Codes
		for (i = 0; i < 010; i++) {
			if (pdp10_OpcodeFUNC[i]) {
				dbg_Printf("Opcode %05o: %s\n", (i << 2) | 070000,
					pdp10_OpcodeFUNC[i]->Name);
			}
		}
		dbg_Printf("\n");
	}

#endif /* DEBUG */

	switch (p10->cnfFlags & CNF_CPUID) {
		case CNF_KL10:
			KL10_InitDevices();
			KL10_InitPager();
			KL10pi_Reset();
			KX10_CalcEffAddr   = KL10_extCalcEffAddr;
			KX10_PageRefill    = KL10_PageRefill;
			KX10_PageTrap1     = KL10_PageTrap1;
			KX10_PageTrap2     = KL10_PageTrap2;
			KX10_Trap_NoMemory = KL10_Trap_NoMemory;
			KX10_piProcess     = KL10pi_Process;
			// Instructions with extended addressing feature.
			basOpcode[0254]    = KL10_Opcode_JRST;
			extOpcode[0020]    = KL10_extOpcode_XBLT;
			break;

		case CNF_KS10:
			p10_Console = p10_InitConsole();
			KX10_CalcEffAddr   = KS10_CalcEffAddr;
			KX10_PageRefill    = KS10_PageRefill;
			KX10_PageTrap1     = KS10_PageTrap1;
			KX10_PageTrap2     = KS10_PageTrap2;
			KX10_Trap_NoMemory = KS10_Trap_NoMemory;
			KX10_piProcess     = KS10_piProcess;
			break;
	}
	
}
