// cpu_main.c - PDP-11 Emulator: Main Routines/Basic Instructions
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

#include <pdp11/defs.h>
#include <pdp11/uqba.h>
#include <emu/socket.h>

extern int32     ts10_ClkInterval;
extern CLK_QUEUE *ts10_SimClock;

#ifdef DEBUG
extern SOCKET *ts10_Stdout;
extern SOCKET *dbg_File;
extern void   p11_Disasm(register P11_CPU *, SOCKET *, uint32 *, uint32);

static cchar *trapNames[] =
{
	"Red Stack",
	"Address Error",
	"Memory Management Error",
	"Non-Existant Memory",
	"Parity Error",
	"Privileged Instruction",
	"Illegal Instruction",
	"Breakpoint Instruction",
	"Input/Output Trap Instruction",
	"Emulator Trap Instruction",
	"Trap Instruction",
	"Trace (T-bit)",
	"Yellow Stack",
	"Power Failure",
	"Floating-Point Error",
	"Interrupts"
};
#endif /* DEBUG */

uint32 static trapClear[] = {
	TRAP_RED|TRAP_PAR|TRAP_YEL|TRAP_TRC|TRAP_ODD|TRAP_NXM,
	TRAP_ODD|TRAP_PAR|TRAP_YEL|TRAP_TRC,
	TRAP_MME|TRAP_PAR|TRAP_YEL|TRAP_TRC,
	TRAP_NXM|TRAP_PAR|TRAP_YEL|TRAP_TRC,
	TRAP_PAR|TRAP_TRC,
	TRAP_PRV|TRAP_TRC,
	TRAP_ILL|TRAP_TRC,
	TRAP_BPT|TRAP_TRC,
	TRAP_IOT|TRAP_TRC,
	TRAP_EMT|TRAP_TRC,
	TRAP_TRAP|TRAP_TRC,
	TRAP_TRC,
	TRAP_YEL,
	TRAP_PWRFL,
	TRAP_FPE
};

#define BRANCH(op) PC += (int8)op << 1

#define N (((CC & CC_N) >> 3) & 1)
#define Z (((CC & CC_Z) >> 2) & 1)
#define V (((CC & CC_V) >> 1) & 1)
#define C (CC & CC_C)

uint32 p11_dsMask[] = { MMR3_KDS, MMR3_SDS, 0, MMR3_UDS };

// Effective Address Calculation for Word Access
uint32 p11_GeteaW(register P11_CPU *p11, int32 spec)
{
	register uint32 reg = spec & 7;
	register uint32 ds  = (reg == 7) ? (VA_INST|ISPACE) : DSPACE;
	register uint16 adr;
	
	switch (spec >> 3) {
		default:
		case 1: // (Rn)
			return (UREGW(reg) | ds);
		case 2: // (Rn)+
			UREGW(reg) = (adr = UREGW(reg)) + 2;
			if (UpdateMM) SetMMR1(020 | reg);
			return (adr | ds);
		case 3: // @(Rn)+
			UREGW(reg) = (adr = UREGW(reg)) + 2;
			if (UpdateMM) SetMMR1(020 | reg);
			return ((uint16)ReadW(adr | ds) | DSPACE);
		case 4: // -(Rn)
			UREGW(reg) = (adr = (UREGW(reg) - 2));
			if (UpdateMM) SetMMR1(0360 | reg);
			if ((reg == 6) && (adr < STKLIM) && (PSW_GETCUR(PSW) == AM_KERNEL)) {
				SET_TRAP(TRAP_YEL);
				SET_CPUERR(CPUE_YEL);
			}
			return (adr | ds);
		case 5: // @-(Rn)
			UREGW(reg) = (adr = (UREGW(reg) - 2));
			if (UpdateMM) SetMMR1(0360 | reg);
			if ((reg == 6) && (adr < STKLIM) && (PSW_GETCUR(PSW) == AM_KERNEL)) {
				SET_TRAP(TRAP_YEL);
				SET_CPUERR(CPUE_YEL);
			}
			return ((uint16)ReadW(adr | ds) | DSPACE);
		case 6: // d(Rn)
			adr = ReadW(PC | (VA_INST|ISPACE)); PC += 2;
			return (((uint16)(UREGW(reg) + adr)) | DSPACE);
		case 7: // @d(Rn)
			adr = ReadW(PC | (VA_INST|ISPACE)); PC += 2;
			adr = ReadW(((uint16)(UREGW(reg) + adr)) | DSPACE);
			return (adr | DSPACE);
	}
}

// Effective Address Calculation for Byte Access
uint32 p11_GeteaB(register P11_CPU *p11, int32 spec)
{
	register uint32 reg = spec & 7;
	register uint32 ds  = (reg == 7) ? (VA_INST|ISPACE) : DSPACE;
	register uint16 adr, delta;
	
	switch (spec >> 3) {
		default:
		case 1: // (Rn)
			return (UREGW(reg) | ds);
		case 2: // (Rn)+
			delta = 1 + (reg >= 6);
			UREGW(reg) = (adr = UREGW(reg)) + delta;
			if (UpdateMM) SetMMR1((delta << 3) | reg);
			return (adr | ds);
		case 3: // @(Rn)+
			UREGW(reg) = (adr = UREGW(reg)) + 2;
			if (UpdateMM) SetMMR1(020 | reg);
			return ((uint16)ReadW(adr | ds) | DSPACE);
		case 4: // -(Rn)
			delta = 1 + (reg >= 6);
			UREGW(reg) = (adr = (UREGW(reg) - delta));
			if (UpdateMM) SetMMR1((((-delta) & 037) << 3) | reg);
			if ((reg == 6) && (adr < STKLIM) && (PSW_GETCUR(PSW) == AM_KERNEL)) {
				SET_TRAP(TRAP_YEL);
				SET_CPUERR(CPUE_YEL);
			}
			return (adr | ds);
		case 5: // @-(Rn)
			UREGW(reg) = (adr = (UREGW(reg) - 2));
			if (UpdateMM) SetMMR1(0360 | reg);
			if ((reg == 6) && (adr < STKLIM) && (PSW_GETCUR(PSW) == AM_KERNEL)) {
				SET_TRAP(TRAP_YEL);
				SET_CPUERR(CPUE_YEL);
			}
			return ((uint16)ReadW(adr | ds) | DSPACE);
		case 6: // d(Rn)
			adr = ReadW(PC | (VA_INST|ISPACE)); PC += 2;
			return (((uint16)(UREGW(reg) + adr)) | DSPACE);
		case 7: // @d(Rn)
			adr = ReadW(PC | (VA_INST|ISPACE)); PC += 2;
			adr = ReadW(((uint16)(UREGW(reg) + adr)) | DSPACE);
			return (adr | DSPACE);
	}
}

char *p11_GetCC(uint16 cc, char *str)
{
	static char ccStr[5];
	char        *pStr = str ? str : ccStr;

	// Display condition bits
	pStr[0] = (cc & CC_N) ? 'N' : '-';
	pStr[1] = (cc & CC_Z) ? 'Z' : '-';
	pStr[2] = (cc & CC_V) ? 'V' : '-';
	pStr[3] = (cc & CC_C) ? 'C' : '-';
	pStr[4] = '\0';

	return pStr;
}

inline void p11_FromTrap(register P11_CPU *p11, cchar *name)
{
	uint16 oldPSW  = PSW | CC;
	uint16 oldPC   = PC;
	uint16 oldMode = PSW_GETCUR(oldPSW);
	uint16 newPSW, newPC, newMode;
	int    idx;

	newPC   = ReadW(SP | DSPACE);
	newPSW  = ReadW((SP+2) | DSPACE);
	STKREG(oldMode) = (SP += 4);

	if (oldMode != AM_KERNEL)
		newPSW |= (oldPSW & (PSW_CM|PSW_PM|PSW_RS));
	newMode = PSW_GETCUR(newPSW);

	if ((oldPSW ^ newPSW) & PSW_RS) {
		uint32 oldSet = (oldPSW & PSW_RS) ? 1 : 0;
		uint32 newSet = (newPSW & PSW_RS) ? 1 : 0;

		// Exchange all R0-R5 with R0'-R5' registers
		for (idx = 0; idx < 6; idx++) {
			GPREG(idx, oldSet) = REGW(idx);
			REGW(idx) = GPREG(idx, newSet);
		}
	}

	ISPACE = GetISpace(newMode);
	DSPACE = GetDSpace(newMode);
	SP     = STKREG(newMode);
	PSW    = newPSW & ~CC_ALL;
	CC     = newPSW & CC_ALL;
	PC     = newPC;

	// Update Interrupt Requests
	uq11_EvalIRQ(p11, GET_IPL(PSW));

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("%s: (%s) Old PC %06o PSW %06o Mode: (%d,%d)\n",
			p11->Unit.devName, name, oldPC, oldPSW, oldMode, PSW_GETPRV(oldPSW));
		dbg_Printf("%s: (%s)   New PC %06o PSW %06o Mode: (%d,%d)\n",
			p11->Unit.devName, name, newPC, newPSW, newMode, PSW_GETPRV(newPSW));
	}
#endif /* DEBUG */
}

// Condition Instructions
// NOP, CCC, CLC, CLN, CLV, CLZ, SCC, SEC, SEN, SEV, and SEZ
// Opcode 000240-000257

INSDEF(p11, NOP)
{
	// Do nothing
}

// Cxx/Sxx/NOP Instruction
INSDEF(p11, XCC)
{
#ifdef DEBUG
	uint16 osr = CC;
#endif /* DEBUG */

	if (IR & CC_SET)
		CC |= (IR & CC_ALL);
	else
		CC &= ~(IR & CC_ALL);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		char oldCC[5], newCC[5];
		dbg_Printf("%s: (CC) %s => %s\n", p11->Unit.devName,
			p11_GetCC(osr, oldCC), p11_GetCC(CC, newCC));
	}
#endif /* DEBUG */
}

INSDEF(p11, ADC)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int16  dst, src = FetchMW(dstSpec, &pAddr);
	int16  cry      = CC & CC_C;

	// Add carry bit.
	dst = src + cry;

	// Update condition codes.
	CC_IIZZ_W(dst);
	if (cry) {
		if ((uint16)dst == 0100000) CC |= CC_V;
		if (dst == 0)               CC |= CC_C;
	}

	// Write results back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ADC) %06o + C => %06o: %s\n", p11->Unit.devName,
			(uint16)src, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, ADCB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int8   dst, src = FetchMB(dstSpec, &pAddr);
	int8   cry      = CC & CC_C;

	// Add carry bit.
	dst = src + cry;

	// Update condition codes.
	CC_IIZZ_B(dst);
	if (cry) {
		if ((uint8)dst == 0200) CC |= CC_V;
		if (dst == 0)           CC |= CC_C;
	}

	// Write results back.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ADCB) %03o + C => %03o: %s\n", p11->Unit.devName,
			(uint8)src, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, ADD)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	int16  src1, src2, dst;
	uint32 pAddr;

	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchMW(dstSpec, &pAddr);
		src1 = REGW(srcSpec);
	} else {
		src1 = FetchW(srcSpec);
		src2 = FetchMW(dstSpec, &pAddr);
	}

	// Perform Add Operation.
	dst = src2 + src1;

	// Update condition codes.
	CC_IIZZ_W(dst);
	if (((~src1 ^ src2) & (src1 ^ dst)) < 0) CC |= CC_V;
	if ((uint16)dst < (uint16)src1)          CC |= CC_C;

	// Write result back
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ADD) %06o + %06o => %06o: %s\n", p11->Unit.devName,
			(uint16)src2, (uint16)src1, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, ASL)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	uint16 dst, src = FetchMW(dstSpec, &pAddr);

	// Shift left by one.
	dst = src << 1;

	// Update condition codes.
	CC_IIZZ_W(dst);
	if ((int16)src < 0)  CC |= CC_C;
	if (N ^ C)           CC |= CC_V;

	// Write results back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ASL) %06o << 1 => %06o: %s\n", p11->Unit.devName,
			(uint16)src, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, ASLB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int8   dst, src = FetchMB(dstSpec, &pAddr);

	// Shift right by one.
	dst = src << 1;

	// Update condition codes.
	CC_IIZZ_B(dst);
	if ((int8)src < 0)  CC |= CC_C;
	if (N ^ C)          CC |= CC_V;

	// Write results back.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ASLB) %03o << 1 => %03o: %s\n", p11->Unit.devName,
			(uint8)src, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, ASR)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int16  dst, src = FetchMW(dstSpec, &pAddr);

	// Shift right by one.
	dst = src >> 1;

	// Update condition codes.
	CC_IIZZ_W(dst);
	if ((int16)src & 1)  CC |= CC_C;
	if (N ^ C)           CC |= CC_V;

	// Write results back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ASR) %06o >> 1 => %06o: %s\n", p11->Unit.devName,
			(uint16)src, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, ASRB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int8   dst, src = FetchMB(dstSpec, &pAddr);

	// Shift right by one.
	dst = src >> 1;

	// Update condition codes.
	CC_IIZZ_B(dst);
	if ((int8)src & 1)  CC |= CC_C;
	if (N ^ C)          CC |= CC_V;

	// Write results back.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ASRB) %03o >> 1 => %03o: %s\n", p11->Unit.devName,
			(uint8)src, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Branch Instructions with Condition Codes
INSDEF(p11, BR)   { BRANCH(IR); }
INSDEF(p11, BCC)  { if ((CC & CC_C) == 0)        BRANCH(IR); }
INSDEF(p11, BCS)  { if (CC & CC_C)               BRANCH(IR); }
INSDEF(p11, BEQ)  { if (CC & CC_Z)               BRANCH(IR); }
INSDEF(p11, BGE)  { if ((N ^ V) == 0)            BRANCH(IR); }
INSDEF(p11, BGT)  { if ((Z | (N ^ V)) == 0)      BRANCH(IR); }
INSDEF(p11, BHI)  { if ((CC & (CC_Z|CC_C)) == 0) BRANCH(IR); }
INSDEF(p11, BLE)  { if (Z | (N ^ V))             BRANCH(IR); }
INSDEF(p11, BLT)  { if (N ^ V)                   BRANCH(IR); }
INSDEF(p11, BLOS) { if (CC & (CC_Z|CC_C))        BRANCH(IR); }
INSDEF(p11, BMI)  { if (CC & CC_N)               BRANCH(IR); }
INSDEF(p11, BNE)  { if ((CC & CC_Z) == 0)        BRANCH(IR); }
INSDEF(p11, BPL)  { if ((CC & CC_N) == 0)        BRANCH(IR); }
INSDEF(p11, BVC)  { if ((CC & CC_V) == 0)        BRANCH(IR); }
INSDEF(p11, BVS)  { if (CC & CC_V)               BRANCH(IR); }

INSDEF(p11, BIC)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	int16  src1, src2, dst;
	uint32 pAddr;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchMW(dstSpec, &pAddr);
		src1 = REGW(srcSpec);
	} else {
		src1 = FetchW(srcSpec);
		src2 = FetchMW(dstSpec, &pAddr);
	}

	// Clear any bits.
	dst = src2 & ~src1;

	// Update condition codes.
	CC_IIZP_W(dst);

	// Write result back
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (BIC) %06o & ~%06o => %06o: %s\n", p11->Unit.devName,
			(uint16)src2, (uint16)src1, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, BICB)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	int8   src1, src2, dst;
	uint32 pAddr;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchMB(dstSpec, &pAddr);
		src1 = REGB(srcSpec);
	} else {
		src1 = FetchB(srcSpec);
		src2 = FetchMB(dstSpec, &pAddr);
	}

	// Clear any bits.
	dst = src2 & ~src1;

	// Update condition codes.
	CC_IIZP_B(dst);

	// Write result back
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (BICB) %03o & ~%03o => %03o: %s\n", p11->Unit.devName,
			(uint8)src2, (uint8)src1, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, BIS)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	int16  src1, src2, dst;
	uint32 pAddr;

	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchMW(dstSpec, &pAddr);
		src1 = REGW(srcSpec);
	} else {
		src1 = FetchW(srcSpec);
		src2 = FetchMW(dstSpec, &pAddr);
	}

	// Set any bits.
	dst = src2 | src1;

	// Update condition codes.
	CC_IIZP_W(dst);

	// Write result back
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (BIS) %06o | %06o => %06o: %s\n", p11->Unit.devName,
			(uint16)src2, (uint16)src1, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, BISB)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	int8   src1, src2, dst;
	uint32 pAddr;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchMB(dstSpec, &pAddr);
		src1 = REGB(srcSpec);
	} else {
		src1 = FetchB(srcSpec);
		src2 = FetchMB(dstSpec, &pAddr);
	}

	// Set any bits.
	dst = src2 | src1;

	// Update condition codes.
	CC_IIZP_B(dst);

	// Write result back
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (BISB) %03o | %03o => %03o: %s\n", p11->Unit.devName,
			(uint8)src2, (uint8)src1, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, BIT)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	int16  src1, src2, dst;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchW(dstSpec);
		src1 = REGW(srcSpec);
	} else {
		src1 = FetchW(srcSpec);
		src2 = FetchW(dstSpec);
	}

	// Test bits.
	dst = src2 & src1;

	// Update condition codes.
	CC_IIZP_W(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (BIT) %06o & %06o => %06o: %s\n", p11->Unit.devName,
			(uint16)src2, (uint16)src1, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, BITB)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	int8   src1, src2, dst;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchB(dstSpec);
		src1 = REGB(srcSpec);
	} else {
		src1 = FetchB(srcSpec);
		src2 = FetchB(dstSpec);
	}

	// Test bits.
	dst = src2 & src1;

	// Update condition codes.
	CC_IIZP_B(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (BITB) %03o & %03o => %03o: %s\n", p11->Unit.devName,
			(uint8)src2, (uint8)src1, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Breakpoint Trap
INSDEF(p11, BPT)
{
	SET_TRAP(TRAP_BPT);
}

INSDEF(p11, CLR)
{
	uint16 dstSpec = (IR & 077);

	// Update condition codes.
	CC_Z1ZZ;

	// Clear register or memory in word size.
	StoreW(dstSpec, 0);
}

INSDEF(p11, CLRB)
{
	uint16 dstSpec = (IR & 077);

	// Update condition codes.
	CC_Z1ZZ;

	// Clear register or memory in byte size.
	StoreB(dstSpec, 0);
}

INSDEF(p11, CMP)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	int16  src1, src2, dst;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchW(dstSpec);
		src1 = REGW(srcSpec);
	} else {
		src1 = FetchW(srcSpec);
		src2 = FetchW(dstSpec);
	}

	// Compare two values.
	dst = src1 - src2;

	// Update condition codes.
	CC_IIZZ_W(dst);
	if (((src1 ^ src2) & (~src2 ^ dst)) < 0) CC |= CC_V;
	if ((uint16)src1 < (uint16)src2)         CC |= CC_C;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (CMP) Compare %o with %o: %s\n", p11->Unit.devName,
			(uint16)src1, (uint16)src2, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, CMPB)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	int8   src1, src2, dst;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchB(dstSpec);
		src1 = REGB(srcSpec);
	} else {
		src1 = FetchB(srcSpec);
		src2 = FetchB(dstSpec);
	}

	// Compare two values.
	dst = src1 - src2;

	// Update condition codes.
	CC_IIZZ_B(dst);
	if (((src1 ^ src2) & (~src2 ^ dst)) < 0) CC |= CC_V;
	if ((uint8)src1 < (uint8)src2)           CC |= CC_C;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (CMPB) Compare %o with %o: %s\n", p11->Unit.devName,
			(uint8)src1, (uint8)src2, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, COM)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int16  dst, src = FetchMW(dstSpec, &pAddr);
	
	// Complement bits.
	dst = src ^ 0177777;

	// Update condition codes.
	CC_IIZ1_W(dst);

	// Write results back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (COM) %06o => %06o: %s\n", p11->Unit.devName,
			(uint16)src, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, COMB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int8   dst, src = FetchMB(dstSpec, &pAddr);

	// Complement bits.
	dst = src ^ 0377;

	// Update condition codes.
	CC_IIZ1_B(dst);

	// Write results back.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (COMB) %03o => %03o: %s\n", p11->Unit.devName,
			(uint8)src, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, DEC)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int16  dst, src = FetchMW(dstSpec, &pAddr);

	// Decrement by one.
	dst = src - 1;

	// Update condition codes.
	CC_IIZP_W(dst);
	if (dst == 077777) CC |= CC_V;

	// Write results back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (DEC) %06o - 1 => %06o: %s\n", p11->Unit.devName,
			(uint16)src, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, DECB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int8   dst, src = FetchMB(dstSpec, &pAddr);

	// Decrement by one.
	dst = src - 1;

	// Update condition codes.
	CC_IIZP_B(dst);
	if (dst == 0177) CC |= CC_V;

	// Write results back.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (DECB) %03o - 1 => %03o: %s\n", p11->Unit.devName,
			(uint8)src, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Emulator Trap
INSDEF(p11, EMT)
{
	SET_TRAP(TRAP_EMT);
}

INSDEF(p11, INC)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int16  dst, src = FetchMW(dstSpec, &pAddr);

	// Increment by one.
	dst = src + 1;

	// Update condition codes.
	CC_IIZP_W(dst);
	if ((uint16)dst == 0100000) CC |= CC_V;

	// Write results back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (INC) %06o + 1 => %06o: %s\n", p11->Unit.devName,
			(uint16)src, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, INCB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int8   dst, src = FetchMB(dstSpec, &pAddr);

	// Increment by one.
	dst = src + 1;

	// Update condition codes.
	CC_IIZP_B(dst);
	if ((uint8)dst == 0200) CC |= CC_V;

	// Write results back.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (INCB) %03o + 1 => %03o: %s\n", p11->Unit.devName,
			(uint8)src, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Input/Output Trap
INSDEF(p11, IOT)
{
	SET_TRAP(TRAP_IOT);
}

INSDEF(p11, JMP)
{
	uint16 dstSpec = IR & 077;

	if (dstSpec <= 7) {
		SET_TRAP(TRAP_ILL);
		return;
	}

	PC = GeteaW(dstSpec);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (JMP) Jump into %06o\n",
			p11->Unit.devName, PC);
#endif /* DEBUG */
}

INSDEF(p11, JSR)
{
	uint16 srcSpec = (IR >> 6) & 7;
	uint16 dstSpec = IR & 077;
	uint16 dst;

	if (dstSpec <= 7) {
		SET_TRAP(TRAP_ILL);
		return;
	}
	dst = GeteaW(dstSpec);
	SP -= 2;
	if (UpdateMM) SetMMR1(0366);
	WriteW(SP | DSPACE, REGW(srcSpec));
	if (((uint16)SP < STKLIM) && (PSW_GETCUR(PSW) == AM_KERNEL)) {
		SET_TRAP(TRAP_YEL);
		SET_CPUERR(CPUE_YEL);
	}
	REGW(srcSpec) = PC;
	PC = dst;
}

// Halt Processor
INSDEF(p11, HALT)
{
	if (((PSW & PSW_CM) == 0) && ((MAINT & CPU_HALT) == 0)) {
		emu_State = EMU_HALT;
	} else {
		SET_TRAP(TRAP_PRV);
		SET_CPUERR(CPUE_HALT);
	}
}

INSDEF(p11, MARK)
{
	uint16 mark  = IR & 077;
	uint16 newSP = (PC + (mark << 1));

	PC = R5;
	R5 = ReadW(newSP | DSPACE);
	SP = newSP + 2;
}

// Move From Previous Data
INSDEF(p11, MFPD)
{
	uint16 dstSpec = IR & 077;
	uint16 curMode = PSW_GETCUR(PSW);
	uint16 prvMode = PSW_GETPRV(PSW);
	int16  dst;

	// Get data from previous data space.
	if (dstSpec <= 7) {
		dst = ((dstSpec == 6) && (curMode != prvMode))
			? STKREG(prvMode) : REGW(dstSpec);
	} else
		dst = ReadW(GeteaW(dstSpec) | GetDSpace(prvMode));

	// Update condition codes
	CC_IIZP_W(dst);

	// Push data into stack
	SP -= OP_WORD;
	if (UpdateMM) SetMMR1(0366);
	WriteW(SP | DSPACE, dst);
	if ((SP < STKLIM) && (curMode == AM_KERNEL)) {
		SET_TRAP(TRAP_YEL);
		SET_CPUERR(CPUE_YEL);
	}
}

// Move From Previous Instruction
INSDEF(p11, MFPI)
{
	uint16 dstSpec = IR & 077;
	uint16 curMode = PSW_GETCUR(PSW);
	uint16 prvMode = PSW_GETPRV(PSW);
	int16  dst;

	// Get data from previous instruction space.
	if (dstSpec <= 7) {
		dst = ((dstSpec == 6) && (curMode != prvMode))
			? STKREG(prvMode) : REGW(dstSpec);
	} else {
		uint32 dspace = ((curMode == AM_USER) && (curMode == prvMode))
			? GetDSpace(prvMode) : GetISpace(prvMode);
		dst = ReadW(GeteaW(dstSpec) | dspace);
	}

	// Update condition codes
	CC_IIZP_W(dst);

	// Push data into stack
	SP -= OP_WORD;
	if (UpdateMM) SetMMR1(0366);
	WriteW(SP | DSPACE, dst);
	if ((SP < STKLIM) && (curMode == AM_KERNEL)) {
		SET_TRAP(TRAP_YEL);
		SET_CPUERR(CPUE_YEL);
	}
}

INSDEF(p11, MFPS)
{
	uint16 dstSpec = (IR & 077);
	int8   dst     = PSW | CC;

	// Update condition codes
	CC_IIZP_B(dst);
	
	// Write value to destination
	if (dstSpec <= 7) REGW(dstSpec) = dst;
	else WriteB(p11_GeteaB(p11, dstSpec), dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (MFPS) PSW => %03o: %s\n",
			p11->Unit.devName, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Move From Processor Type
INSDEF(p11, MFPT)
{
	R0 = CPUID;
}

INSDEF(p11, MOV)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	uint32 eAddr;
	int16  dst;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		eAddr = GeteaW(dstSpec);
		dst   = REGW(srcSpec);
	} else {
		dst = FetchW(srcSpec);
		if (dstSpec > 7)
			eAddr = GeteaW(dstSpec);
	}

	// Update condition codes.
	CC_IIZP_W(dst);

	// Write value to destination
	if (dstSpec <= 7) REGW(dstSpec) = dst;
	else WriteW(eAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: Move %o: %s\n", p11->Unit.devName,
			(uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, MOVB)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	uint32 eAddr;
	int8   dst;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		eAddr = GeteaB(dstSpec);
		dst   = REGB(srcSpec);
	} else {
		dst = FetchB(srcSpec);
		if (dstSpec > 7)
			eAddr = GeteaB(dstSpec);
	}

	// Update condition codes.
	CC_IIZP_B(dst);

	// Write value to destination
	if (dstSpec <= 7) REGW(dstSpec) = dst;
	else WriteB(eAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: Move %o: %s\n", p11->Unit.devName,
			(uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Move To Previous Data
INSDEF(p11, MTPD)
{
	uint16 dstSpec = IR & 077;
	uint16 curMode = PSW_GETCUR(PSW);
	uint16 prvMode = PSW_GETPRV(PSW);
	int16  dst;

	// Pop data from stack
	dst = ReadW(SP | DSPACE); SP += 2;
	if (UpdateMM) SetMMR1(026);

	// Update condition codes
	CC_IIZP_W(dst);

	// Put data into previous data space.
	if (dstSpec <= 7) {
		if ((dstSpec == 6) && (curMode != prvMode))
			STKREG(prvMode) = dst;
		else
			REGW(dstSpec) = dst;
	} else
		WriteW(GeteaW(dstSpec) | GetDSpace(prvMode), dst);
}

// Move To Previous Instruction
INSDEF(p11, MTPI)
{
	uint16 dstSpec = IR & 077;
	uint16 curMode = PSW_GETCUR(PSW);
	uint16 prvMode = PSW_GETPRV(PSW);
	int16  dst;

	// Pop data from stack
	dst = ReadW(SP | DSPACE); SP += 2;
	if (UpdateMM) SetMMR1(026);

	// Update condition codes
	CC_IIZP_W(dst);

	// Put data into previous instruction space.
	if (dstSpec <= 7) {
		if ((dstSpec == 6) && (curMode != prvMode))
			STKREG(prvMode) = dst;
		else
			REGW(dstSpec) = dst;
	} else
		WriteW(GeteaW(dstSpec) | GetISpace(prvMode), dst);
}

// Move To Processor Status
INSDEF(p11, MTPS)
{
	uint32 dstSpec = (IR & 077);
	uint8  dst     = FetchB(dstSpec);
	
	// Write low byte of PSW.
	if (PSW_GETCUR(PSW) == AM_KERNEL)
		PSW = (dst & PSW_IPL) | (PSW & ~PSW_IPL);
	CC = dst & CC_ALL;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (MTPS) PSW <= %03o (Now: %06o): %s\n",
			p11->Unit.devName, dst, PSW|CC, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, NEG)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int16  dst, src = FetchMW(dstSpec, &pAddr);
	
	// Negate bits.
	dst = -src;

	// Update condition codes.
	CC &= ~CC_ALL;
	if (dst < 0)  CC |= CC_N;
	if (dst == 0) CC |= CC_Z; else CC |= CC_C;
	if ((uint16)dst == 0100000) CC |= CC_V;

	// Write results back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (NEG) Negate %06o => %06o: %s\n", p11->Unit.devName,
			(uint16)src, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, NEGB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int8   dst, src = FetchMB(dstSpec, &pAddr);

	// Negate bits.
	dst = -src;

	// Update condition codes.
	CC &= ~CC_ALL;
	if (dst < 0)  CC |= CC_N;
	if (dst == 0) CC |= CC_Z; else CC |= CC_C;
	if ((uint8)dst == 0200) CC |= CC_V;

	// Write results back.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (NEGB) Negate %03o => %03o: %s\n", p11->Unit.devName,
			(uint8)src, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Reset External Bus
INSDEF(p11, RESET)
{
	if (PSW & PSW_CM) {
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("%s: (RESET) Non-Kernel Mode - Do nothing.\n",
				p11->Unit.devName);
#endif /* DEBUG */
		return;
	}

	// Reset all I/O devices and interrupts.
	uq11_ResetAll(p11->uqba);
	uq11_ClearIRQ(p11->uqba);

	// Reset Memory Management and Interrupts.
	PIRQ   = 0;
	TIRQ  &= ~TRAP_INT;
	MMR0  &= ~(MMR0_FREEZE|MMR0_MME);
	MMR3   = 0;
	DSPACE = GetDSpace(PSW_GETCUR(PSW));	
}

INSDEF(p11, ROL)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	uint16 dst, src = FetchMW(dstSpec, &pAddr);

	// Rotate left with carry bit.
	dst = (src << 1) | (CC & CC_C);

	// Update condition codes.
	CC_IIZZ_W(dst);
	if ((int16)src < 0) CC |= CC_C;
	if (N ^ C)          CC |= CC_V;

	// Write result backs.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ROL) %06o << 1 => %06o: %s\n", p11->Unit.devName,
			src, dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, ROLB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	uint8  dst, src = FetchMB(dstSpec, &pAddr);

	// Rotate left with carry bit.
	dst = (src << 1) | (CC & CC_C);

	// Update condition codes.
	CC_IIZZ_B(dst);
	if ((int8)src < 0) CC |= CC_C;
	if (N ^ C)         CC |= CC_V;

	// Write result backs.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ROLB) %03o << 1 => %03o: %s\n", p11->Unit.devName,
			src, dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, ROR)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	uint16 dst, src = FetchMW(dstSpec, &pAddr);

	// Rotate left with carry bit.
	dst = (src >> 1) | ((CC & CC_C) << 15);

	// Update condition codes.
	CC_IIZZ_W(dst);
	if (src & 1) CC |= CC_C;
	if (N ^ C)   CC |= CC_V;

	// Write results back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ROR) %06o >> 1 => %06o: %s\n", p11->Unit.devName,
			src, dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, RORB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	uint8  dst, src = FetchMB(dstSpec, &pAddr);

	// Rotate left with carry bit.
	dst = (src >> 1) | ((CC & CC_C) << 7);

	// Update conditon codes.
	CC_IIZZ_B(dst);
	if (src & 1) CC |= CC_C;
	if (N ^ C)   CC |= CC_V;

	// Write results back.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (RORB) %03o << 1 => %03o: %s\n", p11->Unit.devName,
			src, dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Return From Interrupt
INSDEF(p11, RTI)
{
	p11_FromTrap(p11, "RTI");
	if (PSW & PSW_T)
		SET_TRAP(TRAP_TRC);
}

INSDEF(p11, RTS)
{
	uint32 dstSpec = IR & 7;

	PC = REGW(dstSpec);
	REGW(dstSpec) = ReadW(SP | DSPACE);
	if (dstSpec != 6) SP += 2;
}

// Return From Trap
INSDEF(p11, RTT)
{
	p11_FromTrap(p11, "RTT");
}

INSDEF(p11, SBC)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int16  dst, src = FetchMW(dstSpec, &pAddr);
	int16  cry      = (CC & CC_C);

	// Subtract carry bit.
	dst = src - cry;

	// Update condition codes.
	CC_IIZZ_W(dst);
	if (cry) {
		if (dst == 0077777)         CC |= CC_V;
		if ((uint16)dst == 0177777) CC |= CC_C;
	}

	// Write results back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (SBC) %06o - C => %06o: %s\n", p11->Unit.devName,
			(uint16)src, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, SBCB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	int8   dst, src = FetchMB(dstSpec, &pAddr);
	int16  cry      = (CC & CC_C);

	// Subtract carry bit.
	dst = src - cry;

	// Update condition codes.
	CC_IIZZ_B(dst);
	if (cry) {
		if (dst == 0177)        CC |= CC_V;
		if ((uint8)dst == 0377) CC |= CC_C;
	}

	// Write results back.
	StoreMB(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (SBCB) %03o - C => %03o: %s\n", p11->Unit.devName,
			(uint8)src, (uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Subtract One and Branch
INSDEF(p11, SOB)
{
	register int dstSpec = (IR >> 6) & 7;

	if (--REGW(dstSpec))
		PC -= (IR & 077) << 1;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("%s: (SOB) %06o: ",
			p11->Unit.devName, UREGW(dstSpec));
		if (UREGW(dstSpec))
			dbg_Printf("Jump back to %06o.\n", PC);
		else
			dbg_Printf("Done - Continue.\n");
	}
#endif /* DEBUG */
}

// Set Interrupt Priority Level
INSDEF(p11, SPL)
{
	uint32 newIPL;

	// Check if current mode is kernel.
	// Otherwise, do nothing like NOP.
	if (PSW & PSW_CM) {
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("%s: (SPL) Non-Kernel Mode - Do nothing.\n",
				p11->Unit.devName);
#endif /* DEBUG */
		return;
	}

	// Set Interrupt Priority Level
	newIPL = IR & 7;
	SET_IPL(newIPL);
	uq11_EvalIRQ(p11, newIPL);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (SPL) Set IPL to %o\n",
			p11->Unit.devName, newIPL);
#endif /* DEBUG */
}

INSDEF(p11, SUB)
{
	uint16 srcSpec = ((IR >> 6) & 077);
	uint16 dstSpec = (IR & 077);
	uint32 pAddr;
	int16  src1, src2, dst;

	// Decode two operands.
	if ((srcSpec <= 7) && (dstSpec > 7)) {
		src2 = FetchMW(dstSpec, &pAddr);
		src1 = REGW(srcSpec);
	} else {
		src1 = FetchW(srcSpec);
		src2 = FetchMW(dstSpec, &pAddr);
	}

	// Do subtract operation.
	dst = src2 - src1;

	// Update condition codes.
	CC_IIZZ_W(dst);
	if (((src1 ^ src2) & (~src1 ^ dst)) < 0) CC |= CC_V;
	if ((uint16)src2 < (uint16)src1)         CC |= CC_C;

	// Write result back
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (SUB) %06o - %06o => %06o: %s\n", p11->Unit.devName,
			(uint16)src2, (uint16)src1, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, SWAB)
{
	uint32 pAddr;
	uint16 dstSpec  = (IR & 077);
	uint16 dst, src = FetchMW(dstSpec, &pAddr);

	// Swap two bytes.
	dst = (src >> 8) | (src << 8);

	// Update condition codes.
	CC_IIZZ_B(dst);

	// Write result back.
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (SWAB) Swap %06o => %06o: %s\n", p11->Unit.devName,
			src, dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Sign Extend
INSDEF(p11, SXT)
{
	uint16 dstSpec = IR & 077;
	int16  dst     = (CC & CC_N) ? -1 : 0;

	// Update condition codes
	CC &= ~(CC_Z|CC_V);
	if (dst == 0) CC |= CC_Z;

	// Write results back.
	StoreW(dstSpec, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (SXT) Extend Sign %06o: %s\n",
			p11->Unit.devName, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Trap
INSDEF(p11, TRAP)
{
	SET_TRAP(TRAP_TRAP);
}

INSDEF(p11, TST)
{
	uint16 dstSpec = (IR & 077);
	int16  dst     = FetchW(dstSpec);

	// Update condition codes.
	CC_IIZZ_W(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (TST) Test %o: %s\n", p11->Unit.devName,
			(uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, TSTB)
{
	uint16 dstSpec = (IR & 077);
	int8   dst     = FetchB(dstSpec);

	// Update condition codes.
	CC_IIZZ_B(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (TSTB) Test %o: %s\n", p11->Unit.devName,
			(uint8)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, TSTSET)
{
	uint16 dstSpec = (IR & 077);
	int16  dst;
	uint32 pAddr;

	if (dstSpec <= 7) {
		SET_TRAP(TRAP_ILL);
		return;
	}

	dst = FetchMW(dstSpec, &pAddr);

	// Update condition codes.
	CC_IIZZ_W(dst);
	if (dst & 1)  CC |= CC_C;
	
	// Write results back.
	R0 = dst;
	StoreMW(dstSpec, pAddr, (dst | 1));
}

INSDEF(p11, XOR)
{
	uint32 pAddr;
	uint16 srcSpec   = ((IR >> 6) & 07);
	uint16 dstSpec   = (IR & 077);
	int16  dst, src2 = FetchMW(dstSpec, &pAddr);
	int16  src1      = REGW(srcSpec);

	// Do exclusive OR gate.
	dst = src2 ^ src1;

	// Update condition codes.
	CC_IIZP_W(dst);

	// Write result back
	StoreMW(dstSpec, pAddr, dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (XOR) %06o ^ %06o => %06o: %s\n", p11->Unit.devName,
			(uint16)src2, (uint16)src1, (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

// Wait
INSDEF(p11, WAIT)
{
	if (PSW_GETCUR(PSW) == AM_KERNEL) {
		IDLE = 1;
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA) || dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Entered Wait State at PC %06o.\n",
				p11->Unit.devName, PC);
#endif /* DEBUG */
	}
#ifdef DEBUG
	else {
		if (dbg_Check(DBG_TRACE|DBG_DATA) || dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: No Wait State (No Kernel Mode) at PC %06o.\n",
				p11->Unit.devName, PC);
	}
#endif /* DEBUG */
}

INSDEF(p11, WRTLCK)
{
	uint16 dstSpec = (IR & 077);
	int16  dst;
	uint32 pAddr;

	if (dstSpec <= 7) {
		SET_TRAP(TRAP_ILL);
		return;
	}

	// Get data from register R0.
	dst = R0;

	// Update condition codes.
	CC_IIZP_W(dst);
	
	// Write results back.
	WriteW(GeteaW(dstSpec), dst);
}

// Illegal Instruction (Undefined Instruction)
INSDEF(p11, ILL)
{
#ifdef DEBUG
	dbg_Printf("%s: *** Illegal Instruction at PC %06o\n",
		p11->Unit.devName, faultPC);
	if (!dbg_Check(DBG_TRACE)) {
		uint32 pc = faultPC;
		p11_Disasm(p11, dbg_File, &pc, ISPACE);
//		dbg_SetMode(DBG_TRACE|DBG_DATA);
	}
#endif /* DEBUG */

	SET_TRAP(TRAP_ILL);
}

// *************************************************
// ****************** Main CPU Loop ****************
// *************************************************

void p11_DoTraps(register P11_CPU *p11)
{
	uint32 tAddr;
	uint16 tirq, tnum;
	uint16 oldPSW, oldPC, oldMode;
	uint16 newPSW, newPC, newMode;

	tAddr = 0;
	if (tirq = (TIRQ & TRAP_ALL)) {
		for (tnum = 0; tnum < TRAP_MAX; tnum++) {
			if ((tirq >> tnum) & 1) {
				tAddr = p11->intVec[tnum];
				TIRQ &= ~trapClear[tnum]; 
				break;
			}
		}
	} else {
		// Interrupts from hardware
		tAddr = uq11_GetVector(p11, GET_IPL(PSW));
		tnum  = TRAP_P_INT;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT))
		dbg_Printf("%s: *** Trap occured at PC %06o: %s\n",
			p11->Unit.devName, (uint16)PC, trapNames[tnum]);
#endif /* DEBUG */

	IDLE    = 0; // Exit Wait State
	TADR    = tAddr;
	oldPC   = PC;
	oldPSW  = PSW | CC;
	oldMode = PSW_GETCUR(PSW);
	STKREG(oldMode) = SP;

	// Get new PC and PSW from vector entry.
	newPC   = ReadW(tAddr       | GetDSpace(AM_KERNEL));
	newPSW  = ReadW((tAddr + 2) | GetDSpace(AM_KERNEL));

#ifdef DEBUG
	if ((newPC == 0) && (newPSW == 0)) {
		dbg_Printf("%s: *** Zero Entry Detection - Aborted.\n",
			p11->Unit.devName);
		ABORT(-P11_HALT);
	}
#endif /* DEBUG */

	// Save current PSW and PC.
	TADR = ~(newMode = PSW_GETCUR(newPSW));
	WriteW((STKREG(newMode) - 2) | GetDSpace(newMode), oldPSW);
	WriteW((STKREG(newMode) - 4) | GetDSpace(newMode), oldPC);
	TADR = 0;

	if ((newPSW ^ oldPSW) & PSW_RS) {
		uint32 oldSet = (oldPSW & PSW_RS) ? 1 : 0;
		uint32 newSet = (newPSW & PSW_RS) ? 1 : 0;
		int    idx;

		// Exchange all R0-R5 with R0'-R5' registers
		for (idx = 0; idx < 6; idx++) {
			GPREG(idx, oldSet) = REGW(idx);
			REGW(idx) = GPREG(idx, newSet);
		}
	}
	SP = STKREG(newMode) - 4;

	// Load new PC and PSW and continue.
	PSW = ((oldMode << PSW_P_PM) | (newPSW & ~PSW_PM)) & ~CC_ALL;
	CC  = newPSW & CC_ALL;
	PC  = newPC;

	// Set Instruction/Data Space for Memory Mangement
	ISPACE = GetISpace(newMode);
	DSPACE = GetDSpace(newMode);

	// Check stack limit for kernel mode, if exceeding
	// stack limit, alert yellow stack status.
	if ((SP < STKLIM) && (newMode == AM_KERNEL) &&
	    (tnum != TRAP_P_RED) && (tnum != TRAP_P_YEL)) {
		SET_TRAP(TRAP_YEL);
		SET_CPUERR(CPUE_YEL);
	}

	// Update Interrupt Requests
	uq11_EvalIRQ(p11, GET_IPL(PSW));

#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT)) {
		dbg_Printf("%s: (Trap) Old PC %06o PSW %06o Mode: (%d,%d)\n",
			p11->Unit.devName, oldPC, oldPSW, oldMode, PSW_GETPRV(oldPSW));
		dbg_Printf("%s: (Trap)   New PC %06o PSW %06o Mode: (%d,%d)\n",
			p11->Unit.devName, newPC, newPSW, newMode, PSW_GETPRV(newPSW));
	}
#endif /* DEBUG */
}

#ifdef DEBUG
void p11_DumpRegisters(register P11_CPU *p11)
{
	dbg_Printf("%s: %06o %06o %06o %06o %06o %06o %06o %06o\n",
		p11->Unit.devName, (uint16)R0, (uint16)R1, (uint16)R2, (uint16)R3,
	  	(uint16)R4, (uint16)R5, (uint16)SP, (uint16)PC);
}
#endif /* DEBUG */

void p11_Execute(register P11_CPU *p11)
{
	int    abValue;
	uint16 opCode;

	// Initialize Real Timer.
	ts10_SetAlarm(ts10_TickRealTimer);
	ts10_StartTimer();
	p11->StartTimer(p11);

	abValue = setjmp(p11->SetJump);
	if (abValue < 0) {
		uint32 pc = PC;
#ifdef DEBUG
		p11_Disasm(p11, ts10_Stdout, &pc, ISPACE);
#endif /* DEBUG */
		emu_State = -abValue;
	} else if (abValue > 0) {
		// Page Fault Traps, etc.
		SET_TRAP(abValue);

		// Kernel Stack Abort Error
		if (TADR == (uint16)~AM_KERNEL) {
			SET_TRAP(TRAP_RED);
			SET_CPUERR(CPUE_RED);
			STKREG(AM_KERNEL) = 4;
			if (PSW_GETCUR(PSW) == AM_KERNEL)
				SP = 4;
		}
	}

	while (emu_State == P11_RUN) {
		if (ts10_ClkInterval-- <= 0)
			ts10_ExecuteTimer();

		// If any interrupt/trap requests,
		// do them right now.
		if (TIRQ) {
			p11_DoTraps(p11);
			continue;
		}
		if (PSW & PSW_T)
			SET_TRAP(TRAP_TRC);

		// Current wait state
		if (IDLE) {
			if (ts10_SimClock != NULL)
				ts10_ClkInterval = 0;
			continue;
		}

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE)) {
			uint32 pc = PC;
			p11_Disasm(p11, dbg_File, &pc, ISPACE);
		}
#endif /* DEBUG */

		// Fetch instruction from memory.
		faultPC = PC;
		if (UpdateMM) {
			MMR1 = 0;
			MMR2 = PC;
		}
		IR = opCode = ReadW(PC | (VA_INST|ISPACE));
		PC += 2;

		// Count and execute instruction.
		CIPS++;
		p11->tblOpcode[opCode](p11);

#ifdef DEBUG
		if (dbg_Check(DBG_REGISTER))
			p11_DumpRegisters(p11);
#endif /* DEBUG */
	}

	// Stop real timer.
	p11->StopTimer(p11);
	ts10_StopTimer();

	printf("%s: Stopped at PC %06o\n", p11->Unit.devName, (uint16)PC);
}
