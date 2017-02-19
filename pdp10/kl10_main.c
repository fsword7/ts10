// kl10_main.c - Main routines for KL10 processor.
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
#include "pdp10/kl10.h"

extern int KX10_IsGlobal;
extern int30   p10_Section;
extern int32   KL10_Pager_T20;
extern int30   KL10_uptAddr;

extern void (*basOpcode[01000])(); // Basic Instruction Table
extern int  (*extOpcode[01000])(); // EXTEND Instruction Table

// Local UUO Instructions - Opcode 001-037
void KL10_Opcode_LUUO(void)
{
	// Set UUO word - Opcode, AC, and E.
	AR = (LH(HR) | RH(eAddr)) & LUUO_OPCODE;

	// Save UUO word at PC 40 and execute branch
	// instruction at PC 41 once.
	p10_vWrite(040, AR, 0);
	cpu_pFlags |= CPU_CYCLE_UUO;
	p10_Execute(041, 0);
}

// Monitor UUO Instructions - Opcode 000, 040-777
void KL10_Opcode_UUO(void)
{
	int30 uuoAddr; // Address of new PC word
	int36 newAddr; // New PC word

#ifdef DEBUG
	if (dbg_Check(DBG_MUUO) && !dbg_Check(DBG_TRACE))
		p10_Disassemble(pager_PC, HR, 0);
#endif /* DEBUG */

	if (KL10_Pager_T20) {
		// For TOPS-20 operating system
		if (ISCPU(CNF_XADR)) {
			// Extended KL10 processor
			int36 opWord = LH(FLAGS & ~FLG_TRAPS) | LHSR(HR & MUUO_OPCODE) |
				((FLAGS & FLG_USER) ? 0 : PCS);
			p10_pWrite(KL10_uptAddr + T20_XUUO_OPCODE, opWord, 0);
			p10_pWrite(KL10_uptAddr + T20_XUUO_OLDPC,  PC, 0);
			p10_pWrite(KL10_uptAddr + T20_XUUO_EADDR,  eAddr, 0);
			p10_pWrite(KL10_uptAddr + T20_XUUO_PCWORD, KL10_uptAddr, 0);
		} else {
			// Single-section KL10 processor
			p10_pWrite(KL10_uptAddr + T20_MUUO_OPCODE, HR & MUUO_OPCODE, 0);
			p10_pWrite(KL10_uptAddr + T20_MUUO_OLDPC,
				LH(FLAGS & ~FLG_TRAPS) | RH(PC), 0);
			p10_pWrite(KL10_uptAddr + T20_MUUO_PCWORD, KL10_uptAddr, 0);
		}
	} else {
		// For TOPS-10 operating system
		p10_pWrite(KL10_uptAddr + T10_MUUO_OPCODE, HR & MUUO_OPCODE, 0);
		p10_pWrite(KL10_uptAddr + T10_MUUO_OLDPC,
			LH(FLAGS & ~FLG_TRAPS) | RH(PC), 0);
		p10_pWrite(KL10_uptAddr + T10_MUUO_PCWORD, KL10_uptAddr, 0);
	}

	// Jump into monitor for MUUO or Illegal Instructions
	uuoAddr = UPT_MUUO_NEWPC;
	if (FLAGS & FLG_USER)
		uuoAddr += MUUO_USER;   // Bump to USER area
	if (FLAGS & FLG_PUBLIC)
		uuoAddr += MUUO_PUBLIC; // Bump to PUBLIC area
	if (FLAGS & FLG_TRAPS)
		uuoAddr += MUUO_TRAP;   // Bump to TRAP area
	newAddr = p10_pRead(KL10_uptAddr + uuoAddr, 0);

	// Load new Flags and PC for system calls.
	if (ISCPU(CNF_XADR)) {
		PCS   = VA_GETSECT(PC);
		FLAGS = ((FLAGS & FLG_USER) ? FLG_PCU : 0) |
		        ((FLAGS & FLG_PUBLIC) ? FLG_PCP : 0);
		DO_XJUMP(newAddr);
	} else {
		FLAGS = LH(newAddr) | ((FLAGS & FLG_USER) ? FLG_PCU : 0) |
		        ((FLAGS & FLG_PUBLIC) ? FLG_PCP : 0);
		DO_JUMP(newAddr);
	}
}

// 254 JRST - Jump and Restore Flags
void KL10_Opcode_JRST(void)
{
	int36 eaFlags;
	int36 pcFlags;

	switch(opAC) {
		// 25400 JRST - Jump                      JRST 0,
		case 000:
			// E -> (PC)

			DO_XJUMP(eAddr);

#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA))
				dbg_Printf("JRST: Jump to %o,,%06o\n",
					LH18(eAddr), RH18(eAddr));
#endif /* DEBUG */
			break;

		// 25404 PORTAL - Portal                  JRST 1,
		case 001:
			// 0 -> PUBLIC, E -> (PC)

			FLAGS &= ~FLG_PUBLIC;
			DO_XJUMP(eAddr);

#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA))
				dbg_Printf("JRST: Jump to %o,,%06o\n",
					LH18(eAddr), RH18(eAddr));
#endif /* DEBUG */
			break;

		// 25410 JRSTF - Restore Flags and Jump    JRST 2,
		case 02:
			// L(X) or L(Y) -> flags, E -> (PC)

			if (p10_Section > 0) {
				// JRSTF should not be used in non-zero
				// section area.  Use XJRSTF instead.
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA))
					dbg_Printf(
						"JRSTF: Must be in zero section area. Use XJRSTF instead.\n"
					);
#endif /* DEBUG */
				KL10_Opcode_UUO();
				return;
			}

			eaMode  = p10_CheckPXCT(PXCT_EA);
			eaFlags = p10_CalcJumpAddr(HR, eaMode);
			if (FLAGS & FLG_USER) {
				eaFlags |= FLG_USER;
				if (!(FLAGS & FLG_USERIO))
					eaFlags &= ~FLG_USERIO;
			}
			if ((FLAGS & FLG_PUBLIC) && !(eaFlags & FLG_PUBLIC)) {
				if ((FLAGS & FLG_USER) || !(eaFlags & FLG_USER))
					eaFlags |= FLG_PUBLIC;
			}
			FLAGS = eaFlags & PC_FLAGS;
			DO_JUMP(eAddr);
			break;

		// 25420 HALT - Halt                       JRST 4,
		case 04:
			// E -> (PC), Stop

			if (FLAGS & FLG_USER)
				KL10_Opcode_UUO();
			else {
				printf("CPU: HALT (Reason Code: %06llo) at PC %06llo\r\n",
					eAddr, RH(PC-1));
				DO_XJUMP(eAddr);
				p10_State = EMU_HALT;
			}
			break;

		// 25424 XJRSTF -  ??                      JRST 5,
		case 05:
			// L(E) -> flags, (E+1) -> (PC)

			AR = p10_vRead(eAddr++, PXCT_CUR);
			BR = p10_vRead(eAddr, PXCT_CUR);
			if (FLAGS & FLG_USER) {
				AR |= FLG_USER;
				if (!(FLAGS & FLG_USERIO))
					AR &= ~FLG_USERIO;
			}
			if ((FLAGS & FLG_PUBLIC) && !(AR & FLG_PUBLIC)) {
				if ((FLAGS & FLG_USER) || !(AR & FLG_USER))
					AR |= FLG_PUBLIC;
			}
#ifdef OPT_XADR
			if (ISCPU(CNF_XADR) && !(FLAGS & FLG_USER))
				PCS = AR & 037;
#endif /* OPT_XADR */
			FLAGS = AR & PC_FLAGS;
			DO_XJUMP(BR);
			break;

		// 25430 XJEN - ??                         JRST 6,
		case 06:
			// Dismiss PI, L(E) -> Flags, (E+1) -> (PC)

			if (FLAGS & FLG_USER)
				KL10_Opcode_UUO();
			else {
				AR = p10_vRead(eAddr++, PXCT_CUR);
				BR = p10_vRead(eAddr, PXCT_CUR);
				AR &= PC_FLAGS;
				if (FLAGS & FLG_USER) {
					AR |= FLG_USER;
					if (!(FLAGS & FLG_USERIO))
						AR &= ~FLG_USERIO;
				}
				if ((FLAGS & FLG_PUBLIC) && !(AR & FLG_PUBLIC)) {
					if ((FLAGS & FLG_USER) || !(AR & FLG_USER))
						AR |= FLG_PUBLIC;
				}
#ifdef OPT_XADR
				if (ISCPU(CNF_XADR) && !(FLAGS & FLG_USER))
					PCS = AR & 037;
#endif /* OPT_XADR */
				FLAGS = AR;
				DO_XJUMP(BR);
				KL10pi_Dismiss();
			}
			break;

		// 25434 XPCW - ??                         JRST 7,
		case 07:
			// Flags,,0 -> (E), PC+1 -> (E+1), L(E+2) -> Flags, (E+3) -> (PC)

			if ((FLAGS & FLG_USER) && ((cpu_pFlags & CPU_CYCLE_PI) == 0))
				KL10_Opcode_UUO();
			else {
				pcFlags = FLAGS & PC_FLAGS;
#ifdef OPT_XADR
				if (ISCPU(CNF_XADR) && !(FLAGS & FLG_USER))
					pcFlags |= PCS;
#endif /* OP_XADR */
				p10_eWrite(eAddr++, pcFlags);

				p10_eWrite(eAddr++, PC);
				AR = p10_eRead(eAddr++);
				BR = p10_eRead(eAddr);
				AR &= PC_FLAGS;
//				if (FLAGS & FLG_USER) {
//					AR |= FLG_USER;
//					if (!(FLAGS & FLG_USERIO))
//						AR &= ~FLG_USERIO;
//				}
//				if ((FLAGS & FLG_PUBLIC) && !(AR & FLG_PUBLIC)) {
//					if ((FLAGS & FLG_USER) || !(AR & FLG_USER))
//						AR |= FLG_PUBLIC;
//				}
				FLAGS = AR;
				DO_XJUMP(BR);
			}
			break;

		// 25440                                   JRST 10,
		case 010:
			if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0))
				KL10_Opcode_UUO();
			else {
				DO_XJUMP(eAddr);
				KL10pi_Dismiss();
			}
			break;

		// 25450 JEN - Jump and Enable             JRST 12,
		case 012:
			// Dismiss PI, L(X) or L(Y) -> Flags, E -> (PC)

			if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0))
				KL10_Opcode_UUO();
			else {
				eaMode = p10_CheckPXCT(PXCT_EA);
				eaFlags = p10_CalcJumpAddr(HR, eaMode);
				if (FLAGS & FLG_USER) {
					eaFlags |= FLG_USER;
					if (!(FLAGS & FLG_USERIO))
						eaFlags &= ~FLG_USERIO;
				}
				if ((FLAGS & FLG_PUBLIC) && !(eaFlags & FLG_PUBLIC)) {
					if ((FLAGS & FLG_USER) || !(eaFlags & FLG_USER))
						eaFlags |= FLG_PUBLIC;
				}
				FLAGS = eaFlags & PC_FLAGS;
				DO_XJUMP(eAddr);
				KL10pi_Dismiss();
			}
			break;

		// 25460 XSFM - Store Flags to Memory      JRST 14,
		case 014:
			pcFlags = FLAGS & PC_FLAGS;
#ifdef OPT_XADR
			if (ISCPU(CNF_XADR) && !(FLAGS & FLG_USER))
				pcFlags |= PCS;
#endif /* OP_XADR */
			p10_vWrite(eAddr, pcFlags, PXCT_CUR);
			break;

		// 25464 XJRST - Extended Jump            JRST 15,
		case 015:
			PC = PMA(p10_vRead(eAddr, PXCT_CUR));
			p10_Section = LPC(PC);
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA))
				dbg_Printf("XJRST: Jump to %o,,%06o\n",
					LH18(PC), RH18(PC));
#endif /* DEBUG */
			break;

		// Otherwise - Illegal Instruction
		default:
			KL10_Opcode_UUO();
	}
}

// 257 MAP - Map
void KL10_Opcode_MAP(void)
{
	// Physical Map Data -> (AC)

	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0))
		KL10_Opcode_UUO();
	else {
		dataMode = p10_CheckPXCT(PXCT_DATA);
		AR = KL10_GetMap(eAddr, dataMode);
		curAC[opAC] = SXT36(AR);
#ifdef DEBUG
		if (dbg_Check(DBG_DATA))
			dbg_Printf("MAP: %06o,,%06o -> %06o,,%06o\n",
				LH18(eAddr), RH18(eAddr), LH18(AR), RH18(AR));
#endif /* DEBUG */
	}
}

// ***************************************************************

// Calculate for extended effective address
inline int30 KL10_extCalcEffAddr(int30 pcSection, int32 LocalIW, int eaMode)
{
	int36   XR, GlobalIW;
	int32   Index, Indirect;
	int30   Section = LPC(pcSection);
	int30   eAddr;

	// Reset global/local flag to local as default.
	KX10_IsGlobal = FALSE;

	do {
		Indirect = LIW_GETI(LocalIW);
		Index    = LIW_GETX(LocalIW);
		eAddr    = LIW_GETY(LocalIW);

		if (Index) {
			XR = (eaMode ? prvAC : curAC)[Index];
			if (Section && (XLH(XR) > 0)) {
				eAddr = PMA(XR + SXT18(eAddr));    // Global Index Word
				KX10_IsGlobal = TRUE;
			} else
				eAddr = Section | VMA(XR + eAddr); // Local Index Word
		} else
			eAddr |= Section;

		if (Indirect) {
			do {
				GlobalIW = p10_vRead(eAddr, eaMode);
				Section  = LPC(eAddr);

				if (Section && (GlobalIW >= 0)) {
					// Global Indirect Word

					Indirect = GIW_GETI(GlobalIW);
					Index    = GIW_GETX(GlobalIW);
					eAddr    = GIW_GETY(GlobalIW);

					KX10_IsGlobal = TRUE;

					if (Index)
						eAddr = PMA(eAddr + (eaMode ? prvAC : curAC)[Index]);
				} else {
					// Local Indirect Word
					// If IW<0:1> = 11, go to page fail trap.
					// Otherwise, get out of inner while.
					KX10_IsGlobal = FALSE;
					LocalIW = GlobalIW;
					break; 
				}
			} while (Indirect);
		}
	} while (Indirect);

	return eAddr;
}

// Extended Effective Address Calculation for PXCT Instruction
inline int30 KL10_PrvCalcEffAddr(int30 sect, int32 iw, int pxct)
{
	uint32 eAddr;

	if (pxct & PXCT_EA)
		return KL10_extCalcEffAddr((PCS << 18), iw, PXCT_EA);

	eAddr = KL10_extCalcEffAddr(sect, iw, PXCT_CUR);
	if ((pxct & PXCT_DATA) && ((PCS == 0) || VA_ISLOCAL(eAddr)))
		eAddr = (PCS << 18) | VMA(eAddr);

	return eAddr;
}

// 256 XCT - Execute
void KL10_Opcode_XCT(void)
{
	// Execute (E)
	
	for (;;) {
		HR = p10_vRead(eAddr, PXCT_CUR);

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE))
			p10_Disassemble(eAddr, HR, 0);
#endif /* DEBUG */

		cips++;

		if (opAC && ((FLAGS & FLG_USER) == 0))
			cpu_pFlags |= opAC;
		opCode = INST_GETOP(HR);
		opAC   = INST_GETAC(HR);

		if (cpu_pFlags & (PXCT_EA|PXCT_DATA))
			eAddr = KL10_PrvCalcEffAddr(eAddr, HR, cpu_pFlags);
		else
			eAddr = KL10_extCalcEffAddr(eAddr, HR, PXCT_CUR);

		if (opCode != INST_XCT) {
			basOpcode[opCode]();
			break;
		}
	}
}

#if 0
// Execute instruction at specific address once.
inline void kl10_Execute(int30 xAddr, int mode)
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
		trapAddr = ((FLAGS & FLG_USER) ?
			(KL10_uptAddr + UPT_TR_BASE) : (KL10_eptAddr + EPT_TR_BASE)) + trapFlag;

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

	eAddr = KL10_CalcEffAddr(xAddr, HR, cpu_pFlags & PXCT_EA);

	basOpcode[opCode]();
}

extern CLK_QUEUE kl10_Timer;

int kl10_Execute(MAP_DEVICE *map)
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

//		if (PC == 0407010) {
//			dbg_SetMode(DBG_TRACE|DBG_DATA|DBG_PAGEFAULT);
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
#endif
