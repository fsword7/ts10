// cpu_intexc.c - VAX Interrupt/Exception routines
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

#include "vax/defs.h"

#if 0
// Hardware Interrupt Table
static const uint32 hw_IntMasks[IPL_HMAX] =
{
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 00
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 01
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 02
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 03
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 04
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 05
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 06
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 07
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 08
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 09
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 0A
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 0B
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 0C
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 0D
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 0E
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 0F
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 10
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 11
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 12
	INT_BR4|INT_BR5|INT_BR6|INT_BR7|INT_CLK, // IPL 13
	INT_BR5|INT_BR6|INT_BR7|INT_CLK,         // IPL 14
	INT_BR6|INT_BR7|INT_CLK,                 // IPL 15
	INT_BR7|INT_CLK,                         // IPL 16
	INT_CLK,                                 // IPL 17
};
#endif

// Software Interrupt Table
static const uint32 sw_IntMasks[IPL_SMAX] =
{
	0xFFFE, 0xFFFC, 0xFFF8, 0xFFF0, // IPL 00 - 03
	0xFFE0, 0xFFC0, 0xFF80, 0xFF00, // IPL 04 - 07
	0xFE00, 0xFC00, 0xF800, 0xF000, // IPL 08 - 0B
	0xE000, 0xC000, 0x8000          // IPL 0C - 0E
};

int32 vax_EvaluateIRQ(register VAX_CPU *vax)
{
	uint32 ipl = PSL_GETIPL(PSL);
	int32  idx;

	// Evaluate Clock Interrupt Requests
	// Depending on individual implementations
//	if (vax->CheckTimer && ((idx = vax->CheckTimer(ipl)) > 0))
//		return idx;

	if ((ipl < IPL_MEMERR) && MEMERR)
		return IPL_MEMERR;

	// Evaluate Hardware Interrupt Requests
	if (ipl >= IPL_HMAX)
		return 0;
	if (idx = vax->CheckIRQ(vax, ipl))
		return idx;

	// Evaluate Software Interrupt Requests
	if (ipl >= IPL_SMAX)
		return 0;
	if (SISR & sw_IntMasks[ipl]) {
		for (idx = IPL_SMAX; idx > 0; idx--)
			if (SISR & sw_IntMasks[idx - 1])
				return idx;
	}

	// No such interrupt pending
	return 0;
}

#ifdef DEBUG

// Exception/Interrupt Names
static char *ieNames[] =
	{ "Severe Exception", "Normal Exception", "Interrupt" };

// Exception/Interrupt Types
static char *ieTypes[] =
	{ "SVE", "EXC", "INT" };

// Access Mode Names
static char *accNames[] =
	{ "Kernel", "Executive", "Supervisor", "User", "Interrupt" };

#define DSPL_CUR(mode) accNames[PSL_GETCUR(mode)]
#define DSPL_PRV(mode) accNames[PSL_GETPRV(mode)]

#endif /* DEBUG */

// Do an interrupt or exception.
void vax_DoIntexc(register VAX_CPU *vax, int32 vec, int32 ipl, int32 ie, ...)
{
	int32  newPC;
	int32  newPSL, oldPSL = PSL | CC;
	int32  prvMode = PSL_GETCUR(PSL);
#ifdef DEBUG
	uint32 oldSP  = SP;
#endif /* DEBUG */

	IN_IE = 1;

	CLR_TRAPS; // Clear all traps
	newPC = ReadP(SCBB + vec, OP_LONG);
	if (ie < 0) newPC |= 1;

#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT))
		dbg_Printf(">>%s: SCB Vector: %04X  New PC: %08X(%1X)  Type: %s\n",
			ieTypes[ie+1], vec, newPC & ~03, newPC & 03, ieNames[ie+1]);
#endif /* DEBUG */

	if (newPC & 2) {
		if (vax->HaltAction)
			vax->HaltAction(vax, (newPC & 1) ? HALT_SCB11 : HALT_SCB10);
		ABORT(STOP_ILLVEC);
	}

	if (oldPSL & PSL_IS)
		newPSL = PSL_IS;
	else {
		PRN(prvMode) = SP;
		if (newPC & 1) {
			newPSL = PSL_IS;
			SP = ISP;
		} else {
			newPSL = 0;
			SP = KSP;
		}
	}

	// Update new IPL (Interrupt Priority Level)
	if (ie == IE_INT) {
		// For Interrupts
		PSL = newPSL | PSL_PUTIPL(ipl);
	} else {
		// For Exceptions
		PSL = newPSL | ((newPC & 1) ? PSL_IPL1F : (oldPSL & PSL_IPL)) |
			PSL_PUTPRV(prvMode);
	}

//	newPSL |= (ie > 0) ? PSL_PUTIPL(ipl) :
//		((newPC & 1) ? PSL_IPL1F : (oldPSL & PSL_IPL));
//	PSL = newPSL | ((ie < 1) ? 
//		((oldPSL & PSL_CUR) >> (PSL_P_CUR - PSL_P_PRV)) : 0);

	CC  = 0;

	// Save current PSL and PC into new stack in kernel mode.
	ACC  = ACC_MASK(AM_KERNEL);
	WriteV(SP - 8, PC,  OP_LONG, WA);
	WriteV(SP - 4, oldPSL, OP_LONG, WA);
	SP -= 8;
	
	// Jump to the Interrupt/Exception Service Routine
	SET_PC(newPC & SCB_ADDR);

	// Update new access mode
	SET_ACCESS;

	IN_IE = 0;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA) || dbg_Check(DBG_PCCHANGE)) {
		dbg_Printf(">>%s: Old PC=%08X PSL=%08X SP=%08X  Access: %s, %s\n",
			ieTypes[ie+1], faultPC, oldPSL, oldSP,
			DSPL_CUR(oldPSL), DSPL_PRV(oldPSL));
		dbg_Printf(">>%s:   New PC=%08X PSL=%08X SP=%08X  Access: %s, %s\n",
			ieTypes[ie+1], PC, (PSL | CC), SP,
			DSPL_CUR(PSL), DSPL_PRV(PSL));
	}
#endif /* DEBUG */
}

void vax_Emulate(register VAX_CPU *vax, int32 opCode)
{
	int32 vec;
	int32 newPC;

	if (PSL & PSL_FPD) {
		vec = (SCBB + SCB_EMULFPD) & PAMASK;
		WriteV(SP - 8, faultPC, OP_LONG, WA);
		WriteV(SP - 4, PSL | CC, OP_LONG, WA);
		SP -= 8;
	} else {
		// Temprarory - Hack for HCORE tests (CVTPL)
		if (opCode == 0x36)
			OP2 = (OP2 >= 0) ? ~OP2 : OP3;

		vec = (SCBB + SCB_EMULATE) & PAMASK;
		WriteV(SP - 48, opCode, OP_LONG, WA);
		WriteV(SP - 44, faultPC, OP_LONG, WA);
		WriteV(SP - 40, OP0, OP_LONG, WA);
		WriteV(SP - 36, OP1, OP_LONG, WA);
		WriteV(SP - 32, OP2, OP_LONG, WA);
		WriteV(SP - 28, OP3, OP_LONG, WA);
		WriteV(SP - 24, OP4, OP_LONG, WA);
		WriteV(SP - 20, OP5, OP_LONG, WA);
		WriteV(SP - 8,  PC, OP_LONG, WA);
		WriteV(SP - 4,  PSL | CC, OP_LONG, WA);
		SP -= 48;
	}

	// Jump to the Emulation Routine
	newPC = ReadP(vec, OP_LONG);
	PSL  &= ~(PSL_TP|PSL_FPD|PSW_DV|PSW_FU|PSW_IV|PSW_T);
	CC    = 0;
	SET_PC(newPC & SCB_ADDR);
}

inline void vax_ChangeMode(register VAX_CPU *vax, int newMode)
{
	int32 code = SXTW(OP0);
	int32 vec, prvMode;
	int32 newPC, newSP;
	int32 sts;

	static int prvCode = 0;

#ifdef DEBUG
	char   accType[5] = "KESU";
	int    mode       = newMode;
	uint32 oldPSL     = PSL | CC;
	uint32 oldSP      = SP;

	if (dbg_Check(DBG_PCCHANGE)) {
		if ((prvCode == 0x23) && (code == 0x32)) {
			dbg_SetMode(DBG_TRACE|DBG_DATA);
			printf("[Trace On]\n");
		}
		prvCode = code;
	}
#endif /* DEBUG */

	// Illegal from Interrupt Stack
	if (PSL & PSL_IS) {
		if (vax->HaltAction)
			vax->HaltAction(vax, HALT_CHMIS);
		ABORT(STOP_CHMIS);
	}

	// Get a vector entry from SCBB (System Control Block)
	vec   = (SCBB + SCB_CHMK + (newMode << 2)) & PAMASK;
	newPC = ReadP(vec, OP_LONG);

	// Maximize privilege
	prvMode = PSL_GETCUR(PSL);
	if (prvMode < newMode) newMode = prvMode;

	// Switch stack pointer between modes.
	PRN(prvMode) = SP;
	newSP = PRN(newMode);

	// Set Access Mode for Memory Access
	ACC  = ACC_MASK(newMode);

	// Check valid pages here. (PROBEW)
	if (TestV(newSP - 1, WA, &sts) < 0) {
		P1 = MM_WRITE | (sts & MM_EMASK);
		P2 = newSP - 1;
		ABORT((sts & MM_TNV) ? -SCB_TNV : -SCB_ACV);
	}

	if (TestV(newSP - 12, WA, &sts) < 0) {
		P1 = MM_WRITE | (sts & MM_EMASK);
		P2 = newSP - 12;
		ABORT((sts & MM_TNV) ? -SCB_TNV : -SCB_ACV);
	}

	// Push current code, PC and PSL into new stack
	WriteV(newSP - 12, code, OP_LONG, WA);
	WriteV(newSP - 8,  PC, OP_LONG, WA);
	WriteV(newSP - 4,  PSL | CC, OP_LONG, WA);
	SP = newSP - 12;

	// Load new PC and PSL in new access mode.
	PSL = (PSL & PSL_IPL) | PSL_PUTCUR(newMode) | PSL_PUTPRV(prvMode);
	CC  = 0;
	SET_PC(newPC & SCB_ADDR);

	// Set new access mode and check for software interrupts
	SET_ACCESS; // Update new access mode
	SET_IRQ;    // Evaluate Interrupts

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA) || dbg_Check(DBG_PCCHANGE)) {
		dbg_Printf(">>CHM%c(%04X): Old PC=%08X PSL=%08X SP=%08X  Access: %s, %s\n",
			accType[mode], code, faultPC, oldPSL, oldSP,
			DSPL_CUR(oldPSL), DSPL_PRV(oldPSL));
		dbg_Printf(">>CHM%c(%04X):   New PC=%08X PSL=%08X SP=%08X  Access: %s, %s\n",
			accType[mode], code, PC, PSL, SP, DSPL_CUR(PSL), DSPL_PRV(PSL));
	}
#endif /* DEBUG */
}

DEF_INST(vax, CHMK)
{
	vax_ChangeMode(vax, AM_KERNEL);
}

DEF_INST(vax, CHME)
{
	vax_ChangeMode(vax, AM_EXECUTIVE);
}

DEF_INST(vax, CHMS)
{
	vax_ChangeMode(vax, AM_SUPERVISOR);
}

DEF_INST(vax, CHMU)
{
	vax_ChangeMode(vax, AM_USER);
}

DEF_INST(vax, LDPCTX)
{
	int32 pcbAddr;
	int32 newPC, newPSL;

	// Must be kernel
	if (PSL & PSL_CUR)
		PRIV_INST_FAULT;

	// Clear Process Translation Buffer Table
	vax_ClearTBTable(vax, 0);

	pcbAddr = PCBB & PAMASK;

	// Restore all registers from PCBB area
	KSP    = ReadP(pcbAddr, OP_LONG);
	ESP    = ReadP(pcbAddr + 4, OP_LONG);
	SSP    = ReadP(pcbAddr + 8, OP_LONG);
	USP    = ReadP(pcbAddr + 12, OP_LONG);
	R0     = ReadP(pcbAddr + 16, OP_LONG);
	R1     = ReadP(pcbAddr + 20, OP_LONG);
	R2     = ReadP(pcbAddr + 24, OP_LONG);
	R3     = ReadP(pcbAddr + 28, OP_LONG);
	R4     = ReadP(pcbAddr + 32, OP_LONG);
	R5     = ReadP(pcbAddr + 36, OP_LONG);
	R6     = ReadP(pcbAddr + 40, OP_LONG);
	R7     = ReadP(pcbAddr + 44, OP_LONG);
	R8     = ReadP(pcbAddr + 48, OP_LONG);
	R9     = ReadP(pcbAddr + 52, OP_LONG);
	R10    = ReadP(pcbAddr + 56, OP_LONG);
	R11    = ReadP(pcbAddr + 60, OP_LONG);
	AP     = ReadP(pcbAddr + 64, OP_LONG);
	FP     = ReadP(pcbAddr + 68, OP_LONG);
	newPC  = ReadP(pcbAddr + 72, OP_LONG);
	newPSL = ReadP(pcbAddr + 76, OP_LONG);
	P0BR   = ReadP(pcbAddr + 80, OP_LONG);
	P0LR   = ReadP(pcbAddr + 84, OP_LONG);
	P1BR   = ReadP(pcbAddr + 88, OP_LONG);
	P1LR   = ReadP(pcbAddr + 92, OP_LONG);

	ASTLVL = (P0LR >> 24) & AST_MASK;
	P0BR   &= BR_MASK;
	P0LR   &= LR_MASK;
	P1BR   &= BR_MASK;
	P1LR   &= LR_MASK;
	// Set up memory management

	if (PSL & PSL_IS) {
		ISP = SP;
		PSL &= ~PSL_IS;
	}
	SP = KSP - 8;

	WriteV(SP, newPC, OP_LONG, WA);
	WriteV(SP + 4, newPSL, OP_LONG, WA);
}

DEF_INST(vax, SVPCTX)
{
	int32 pcbAddr;
	int32 savedPC, savedPSL;

	// Must be kernel
	if (PSL & PSL_CUR)
		PRIV_INST_FAULT;

	savedPC  = ReadV(SP, OP_LONG, RA);
	savedPSL = ReadV(SP + 4, OP_LONG, RA);
	if (PSL & PSL_IS)
		SP += 8;
	else {
		KSP = SP + 8;
		SP = ISP;
		if ((PSL & PSL_IPL) == 0)
			PSL |= PSL_IPL01;
		PSL |= PSL_IS;
	}
	
	pcbAddr = PCBB & PAMASK;

	// Save all registers into PCBB area
	WriteP(pcbAddr, KSP, OP_LONG);
	WriteP(pcbAddr + 4, ESP, OP_LONG);
	WriteP(pcbAddr + 8, SSP, OP_LONG);
	WriteP(pcbAddr + 12, USP, OP_LONG);
	WriteP(pcbAddr + 16, R0, OP_LONG);
	WriteP(pcbAddr + 20, R1, OP_LONG);
	WriteP(pcbAddr + 24, R2, OP_LONG);
	WriteP(pcbAddr + 28, R3, OP_LONG);
	WriteP(pcbAddr + 32, R4, OP_LONG);
	WriteP(pcbAddr + 36, R5, OP_LONG);
	WriteP(pcbAddr + 40, R6, OP_LONG);
	WriteP(pcbAddr + 44, R7, OP_LONG);
	WriteP(pcbAddr + 48, R8, OP_LONG);
	WriteP(pcbAddr + 52, R9, OP_LONG);
	WriteP(pcbAddr + 56, R10, OP_LONG);
	WriteP(pcbAddr + 60, R11, OP_LONG);
	WriteP(pcbAddr + 64, AP, OP_LONG);
	WriteP(pcbAddr + 68, FP, OP_LONG);
	WriteP(pcbAddr + 72, savedPC, OP_LONG);
	WriteP(pcbAddr + 76, savedPSL, OP_LONG);
}

// 02 REI - Return from Exception or Interrupt
DEF_INST(vax, REI)
{
	uint32 newPC, newPSL, newSP;
	int32  newMode, oldMode;
	int32  newIPL, oldIPL;

#ifdef DEBUG
	uint32 oldPC  = faultPC;
	uint32 oldPSL = PSL | CC;
	uint32 oldSP  = SP;
#endif /* DEBUG */

	// Get PC and PSL data from stack
	newPC  = ReadV(SP, OP_LONG, RA);
	newPSL = ReadV(SP + 4, OP_LONG, RA);

	newMode = PSL_GETCUR(newPSL);
	oldMode = PSL_GETCUR(PSL);
	oldIPL  = PSL_GETIPL(PSL);

	if ((newPSL & PSL_MBZ) || (newMode < oldMode))
		RSVD_OPND_FAULT;
	if (newMode) {
		if ((newPSL & (PSL_IS|PSL_IPL)) || (newMode > PSL_GETPRV(newPSL)))
			RSVD_OPND_FAULT;
	} else {
		newIPL = PSL_GETIPL(newPSL);
		if ((newPSL & PSL_IS) && (((PSL & PSL_IS) == 0) || (newIPL == 0)))
			RSVD_OPND_FAULT;
		if (newIPL > PSL_GETIPL(PSL))
			RSVD_OPND_FAULT;
	}
	if (newPSL & PSL_CM)
		RSVD_OPND_FAULT;

	SP += 8;

	// Check Compatibility Mode here

	// Save stack pointer for old mode.
	// Also, pass TP bit to new PSL.
//	((PSL & PSL_IS) ? ISP : PRN(oldMode)) = SP;
	if (PSL & PSL_IS)
		ISP = SP;
	else
		PRN(oldMode) = SP;
	PSL = (PSL & PSL_TP) | (newPSL & ~PSW_CC);
	CC  = newPSL & PSW_CC;
	SET_PC(newPC);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA) || dbg_Check(DBG_PCCHANGE)) {
		dbg_Printf(">>REI: Old PC=%08X PSL=%08X SP=%08X  Access: %s, %s\n",
			oldPC, oldPSL, oldSP, DSPL_CUR(oldPSL), DSPL_PRV(oldPSL));
		dbg_Printf(">>REI:   New PC=%08X PSL=%08X SP=%08X  Access: %s, %s\n",
			PC, (PSL | CC), ((PSL & PSL_IS) ? ISP : PRN(newMode)),
			DSPL_CUR(newPSL), DSPL_PRV(newPSL));
	}
#endif /* DEBUG */

	// Load stack pointer for new mode
	// Also, check AST delivery.
	if ((PSL & PSL_IS) == 0) {
		SP = PRN(newMode);
		if (newMode >= ASTLVL) {
			// Request AST delivery (Software IPL 2).
			SISR |= SISR_2;
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA) || dbg_Check(DBG_INTERRUPT))
				dbg_Printf(">>REI: AST Delivered. (%d >= %d)\n", newMode, ASTLVL);
#endif /* DEBUG */
		}
	}

	// Clear instruction look-ahead

	// Set new access mode and check for software interrupts
	SET_ACCESS; // Update new access mode
	SET_IRQ;    // Evaluate Interrupts

#ifdef DEBUG
	if (DBGIPL[oldIPL] & DBG_FLAG) {
		dbg_PutMode(DBGIPL[oldIPL] & ~DBG_FLAG);
		DBGIPL[oldIPL] = 0;
	}
#endif /* DEBUG */
}
