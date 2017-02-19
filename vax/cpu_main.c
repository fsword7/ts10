// cpu_main.c - VAX CPU main routines
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
#include "emu/socket.h"

extern int32 ts10_ClkInterval;

// Instruction table from inst.c file.
extern INSTRUCTION vax_Instruction[];

#ifdef DEBUG
extern SOCKET *ts10_Stdout;
extern SOCKET *dbg_File;
int vax_Disasm(register VAX_CPU *, SOCKET *, uint32 *, uint32);
#endif /* DEBUG */

// Stop (Abort) Messages
static const char *abName[] =
{
	NULL,
	"HALT Instruction",
	"Breakpoint",
	"Illegal Vector",
	"In Interrupt/Exception",
	"Process PTE in P0/P1 Region",
	"Change Mode From IS",
	"Undefined IPL",
	"Unknown Reason"
};

// Access Mode Names
const char *vax_accNames[] = { "Kernel", "Executive", "Supervisor", "User" };

char *vax_DisplayConditions(uint32 cc)
{
	static char ascFlags[5];

	ascFlags[0] = (cc & CC_N) ? 'N' : '-';
	ascFlags[1] = (cc & CC_Z) ? 'Z' : '-';
	ascFlags[2] = (cc & CC_V) ? 'V' : '-';
	ascFlags[3] = (cc & CC_C) ? 'C' : '-';
	ascFlags[4] = '\0';

	return ascFlags;
}

#ifdef DEBUG
void vax_DisplayRegisters(register VAX_CPU *vax)
{
	dbg_Printf("OP=%08X %08X %08X %08X %08X %08X %08X %08X\n",
		OP0, OP1, OP2, OP3, OP4, OP5, OP6, OP7);
	dbg_Printf("R0=%08X %08X %08X %08X %08X %08X %08X %08X\n",
		R0, R1, R2, R3, R4, R5, R6, R7);
	dbg_Printf("R8=%08X %08X %08X %08X %08X %08X %08X %08X\n",
		R8, R9, R10, R11, AP, FP, SP, PC);
	dbg_Printf("PSL=%08X\n", PSL);
}
#endif /* DEBUG */

// Build executable opcode table by using CPU configuration settings
void vax_BuildCPU(VAX_CPU *cpu, uint32 config)
{
	int32   idxInst, idxOpnd;

	// Clear all instruction tables
	for (idxInst = 0; idxInst < NUM_INST; idxInst++)
		cpu->tblOpcode[idxInst] = vax_Opcode_Illegal;
	memset(&cpu->tblOperand, 0, sizeof(uint32) * (NUM_INST * (MAX_SPEC+1)));

	// Build instruction table
	for (idxInst = 0; vax_Instruction[idxInst].Name; idxInst++) {
		void   (*Execute)() = vax_Instruction[idxInst].Execute;
		uint32 opFlags      = vax_Instruction[idxInst].Flags;
		uint32 opExtend     = vax_Instruction[idxInst].Extended;
		uint32 opCode       = vax_Instruction[idxInst].Opcode;
		int32  nOpnds       = vax_Instruction[idxInst].nOperands;

		if (opExtend >= INST_EXTEND)
			opCode |= ((opExtend - (INST_EXTEND - 1)) << 8);

		// Build opcode entry
		if ((Execute == NULL) && ((opFlags & ISTR_EMULATE) == 0))
			cpu->tblOpcode[opCode] = vax_Opcode_Unimplemented;
		else {
			if (opFlags & ISTR_EMULATE)
				cpu->tblOpcode[opCode] = vax_Opcode_Emulate;
			else
				cpu->tblOpcode[opCode] = Execute;

			// Build operand entry
			cpu->tblOperand[opCode][0] = nOpnds;
			for (idxOpnd = 0; idxOpnd < nOpnds; idxOpnd++)
				cpu->tblOperand[opCode][idxOpnd+1] =
					vax_Instruction[idxInst].opMode[idxOpnd];
		}
	}
}

// Operand Decoder
//
// Operands and parsed and placed into operand queues.
//
//    r[bwl]    opRegs[idx]         value of operand
//    rq        opRegs[idx:idx+1]   value of operand
//    a[bwlq]   opRegs[idx]         address of operand
//    m[bwl]    opRegs[idx]         value of operand
//    mq        opRegs[idx:idx+1]   value of operand
//    w[bwlq]   opRegs[idx]         register/memory flag
//              opRegs[idx+1]       memory address

inline void vax_DecodeOperand(register VAX_CPU *vax, uint32 *Operand, int32 *pc)
{
	int   opmode, scale;
	uint8 optype;
	uint8 mode, reg;
	int   idx1, idx2, idx3;
	int32 iAddr;
	int32 t1, t2;

#ifdef DEBUG
	// Reset all operand registers first for debug use.
	for (idx1 = 0; idx1 < MAX_OPREGS; idx1++)
		OPN(idx1) = 0;
#endif /* DEBUG */
	RQPTR = 0;

	for (idx1 = 0, idx2 = 0; idx1 < Operand[0]; idx1++) {
		opmode = Operand[idx1+1];
		scale  = opmode & OP_SCALE;

		// For BUGW/BUGL, Do not decode their operands!
		if (opmode & OP_IMMED)
			return;

		if (opmode & OP_BRANCH) {
			vax->brDisp = ReadI(scale);

			// Get out of the operand decoder routine.
			return;
		}	

		optype = ReadI(OP_BYTE);
		mode = optype & OP_MMASK;
		reg  = optype & OP_RMASK;

		t1 = t2 = 0;

		switch (mode) {
			case LIT0: case LIT1: // Short Literal
			case LIT2: case LIT3:
				if (opmode & (OP_VADDR|OP_ADDR|OP_MODIFIED|OP_WRITE))
					RSVD_ADDR_FAULT;
				if (opmode & OP_FLOAT) {
					if (opmode & (OP_FFLOAT|OP_DFLOAT))
						OPN(idx2++) = 0x4000 | (optype << 4);
					else
						OPN(idx2++) = 0x4000 | (optype << 1);
				} else
					OPN(idx2++) = optype;
				if (scale > OP_LONG)
					OPN(idx2++) = 0;
				continue;

			case REG: // Register
				if (reg >= (nPC - (scale > OP_LONG)))
					RSVD_ADDR_FAULT;
				if (opmode & OP_ADDR)
					RSVD_ADDR_FAULT;
				if (opmode & (OP_VADDR|OP_WRITE)) {
					OPN(idx2++) = reg;
					OPN(idx2++) = RN(reg);
				} else {
					if (scale <= OP_LONG)
						OPN(idx2++) = RN(reg);
					else {
						OPN(idx2++) = RN0(reg);
						OPN(idx2++) = RN1(reg);
					}
					if (opmode & OP_MODIFIED)
						OPN(idx2++) = reg;
				}
				continue;

			case ADEC: // Autodecrement
				RN(reg) -= scale;
				RQOP(RQPTR++) = reg | (scale << 4);

			case REGD: // Register Deferred
				if (reg == nPC)
					RSVD_ADDR_FAULT;
				iAddr = RN(reg);
				break;

			case AINC: // Autoincrement or Immediate
				if (reg == nPC) {
					// Immediate
					if (opmode & (OP_VADDR|OP_ADDR|OP_WRITE)) {
						if (opmode & (OP_VADDR|OP_WRITE))
							OPN(idx2++) = OP_MEM;
						OPN(idx2++) = RN(reg);
						for (idx3 = 0; idx3 < scale; idx3 += OP_LONG)
							ReadI(MAX(scale, OP_LONG));
					} else {
						for (idx3 = 0; idx3 < scale; idx3 += OP_LONG)
							OPN(idx2++) = ReadI(MAX(scale, OP_LONG));
					}
					continue;
				} else {
					// Autoincrement
					iAddr = RN(reg);
					RN(reg) += scale;
					RQOP(RQPTR++) = 0x8000 | reg | (scale << 4);
				}
				break;

			case AINCD: // Autoincrement Deferred or Absolute
				if (reg == nPC) {
					iAddr = ReadI(OP_LONG);
				} else {
					iAddr = ReadV(RN(reg), OP_LONG, RA);
					RN(reg) += OP_LONG;
					RQOP(RQPTR++) = 0x8000 | reg | (OP_LONG << 4);
				}
				break;

			case BDP: // Byte Displacement
				iAddr = RN(reg) + SXTB(ReadI(OP_BYTE));
				break;

			case BDPD: // Byte Displacement Deferred
				iAddr = RN(reg) + SXTB(ReadI(OP_BYTE));
				iAddr = ReadV(iAddr, OP_LONG, RA);
				break;

			case WDP: // Word Displacement
				iAddr = RN(reg) + SXTW(ReadI(OP_WORD));
				break;

			case WDPD: // Word Displacement Deferred
				iAddr = RN(reg) + SXTW(ReadI(OP_WORD));
				iAddr = ReadV(iAddr, OP_LONG, RA);
				break;

			case LDP: // Longword Displacement
				iAddr = RN(reg) + ReadI(OP_LONG);
				break;

			case LDPD: // Longword Displacement Deferred
				iAddr = RN(reg) + ReadI(OP_LONG);
				iAddr = ReadV(iAddr, OP_LONG, RA);
				break;

			case IDX: // Indexed
				if (reg == nPC)
					RSVD_ADDR_FAULT;
				iAddr = RN(reg) * scale;

				optype = ReadI(OP_BYTE);
				mode = optype & OP_MMASK;
				reg  = optype & OP_RMASK;

				switch(mode) {
					case ADEC: // Autodecrement
						RN(reg) -= scale;
						RQOP(RQPTR++) = reg | (scale << 4);

					case REGD: // Register Deferred
						if (reg == nPC)
							RSVD_ADDR_FAULT;
						iAddr += RN(reg);
						break;

					case AINC: // Autoincrement
						if (reg == nPC)
							RSVD_ADDR_FAULT;
						iAddr += RN(reg);
						RN(reg) += scale;
						RQOP(RQPTR++) = 0x8000 | reg | (scale << 4);
						break;

					case AINCD: // Autoincrement Deferred or Absolute
						if (reg == nPC)
							iAddr += ReadI(OP_LONG);
						else {
							iAddr += ReadV(RN(reg), OP_LONG, RA);
							RN(reg) += OP_LONG;
							RQOP(RQPTR++) = 0x8000 | reg | (OP_LONG << 4);
						}
						break;

					case BDP: // Byte Displacement
						iAddr += RN(reg) + SXTB(ReadI(OP_BYTE));
						break;

					case BDPD: // Byte Displacement Deferred
						t1 = RN(reg) + SXTB(ReadI(OP_BYTE));
						iAddr += ReadV(t1, OP_LONG, RA);
						break;

					case WDP: // Word Displacement
						iAddr += RN(reg) + SXTW(ReadI(OP_WORD));
						break;

					case WDPD: // Word Displacement Deferred
						t1 = RN(reg) + SXTW(ReadI(OP_WORD));
						iAddr += ReadV(t1, OP_LONG, RA);
						break;

					case LDP: // Longword Displacement
						iAddr += RN(reg) + ReadI(OP_LONG);
						break;

					case LDPD: // Longword Displacement Deferred
						t1 = RN(reg) + ReadI(OP_LONG);
						iAddr += ReadV(t1, OP_LONG, RA);
						break;

					default:
						RSVD_ADDR_FAULT;
				}
		}

		// Write operand registers
		if (opmode & (OP_VADDR|OP_ADDR|OP_WRITE)) {
			if (opmode & (OP_VADDR|OP_WRITE))
				OPN(idx2++) = OP_MEM;
			OPN(idx2++) = iAddr;
		} else {
//			for (idx3 = 0; idx3 < scale; idx3 += OP_LONG)
//				OPN(idx2++) = ReadV(iAddr + idx3, scale, RA);
			if (scale <= OP_LONG)
				OPN(idx2++) = ReadV(iAddr, scale, RA);
			else {
				OPN(idx2++) = ReadV(iAddr, OP_LONG, RA);
				OPN(idx2++) = ReadV(iAddr + OP_LONG, OP_LONG, RA);
			}
			if (opmode & OP_MODIFIED) {
				OPN(idx2++) = OP_MEM;
				OPN(idx2++) = iAddr;
			}
		}
	}
}

inline void vax_DoFault(register VAX_CPU *vax, int32 vec)
{
	uint32 pc;
	int    idx;
#ifdef DEBUG
	char   *resName;
#endif /* DEBUG */

	// Reset registers which had been incremented or decremented.
	if ((PSL & PSL_FPD) == 0) {
		for (idx = 0; idx < RQPTR; idx++) {
			if (RQOP(idx) < 0)
				RN(RQOP(idx) & 0xF) -= (RQOP(idx) >> 4) & 0xFF;
			else
				RN(RQOP(idx) & 0xF) += (RQOP(idx) >> 4) & 0xFF;
		}
	}

	if (vec == 1)
		return;

#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT))
		dbg_Printf("VAX: ** Fault **  Fault Code: %02X\n", vec & SCB_VECTOR);
#endif /* DEBUG */

	// Reset PC for the faulting instruction
	SET_PC(faultPC);

	switch(vec) {
		// Instruction Decoder
		case SCB_RESIN|NOPRIV: // Privileged Instruction
			vec &= ~NOPRIV;
		case SCB_RESAD:        // Reserved Address Mode
		case SCB_RESOP:        // Reserved Operand
#ifdef DEBUG
			if (vec == SCB_RESIN)
				resName = "Privileged Instruction";
			else if (vec == SCB_RESAD)
				resName = "Reserved Address Mode";
			else if (vec == SCB_RESOP)
				resName = "Reserved Operand";

			pc = faultPC;
			dbg_Printf(">>CPU: *** %s at PC %08X\n", resName, pc);
			vax_Disasm(vax, dbg_File, &pc, SWMASK('v'));
#endif /* DEBUG */

		case SCB_RESIN: // Reserved Instruction
			if (IN_IE) {
				if (vax->HaltAction)
					vax->HaltAction(vax, HALT_INIE); // HLT_INIE
				ABORT(STOP_INIE);
			}
			vax_DoIntexc(vax, vec, 0, IE_EXC);
			break;

		// Memory Mangement - Page Fault Results
		case SCB_ACV: // Access-Control Violation
		case SCB_TNV: // Translation Not Valid
			vax_DoIntexc(vax, vec, 0, IE_EXC);
			IN_IE = 1;
			WriteV(SP - 8, P1, OP_LONG, WA);
			WriteV(SP - 4, P2, OP_LONG, WA);
			SP -= 8;
			IN_IE = 0;
			break;

		case SCB_KSNV: // Kernel Stack Not Valid
			if (PSL & PSL_IS) {
				if (vax->HaltAction)
					vax->HaltAction(vax, HALT_KSNV);
				ABORT(STOP_INIE);
			}
			vax_DoIntexc(vax, vec, 0, IE_SVE);
			break;

		case SCB_ARITH:
			if (IN_IE) {
				if (vax->HaltAction)
					vax->HaltAction(vax, HALT_INIE);
				ABORT(STOP_INIE);
			}
			vax_DoIntexc(vax, vec, 0, IE_EXC);
			IN_IE = 1;
			WriteV(SP - 4, P1, OP_LONG, WA);
			SP -= 4;
			IN_IE = 0;
			break;

		case SCB_MCHK:
			vax->MachineCheck(vax);
			break;

		default:
			printf("VAX: Unknown Fault (SCB) Code %02X\n", vec);
	}
}

#ifdef DEBUG
static char *trapNames[] =
{
	NULL,
	"Integer Overflow Trap",
	"Integer Divide-By-Zero Trap",
	"Floating Overflow Trap",
	"Floating or Decimal Divide-By-Zero Trap",
	"Floating Underflow Trap",
	"Decimal Overflow Trap",
	"Subscript Range Trap",
	"Floating Overflow Fault",
	"Floating Divide-By-Zero Fault",
	"Floating Underflow Fault"
};
#endif /* DEBUG */

inline void vax_Interrupt(register VAX_CPU *vax)
{
	int32 newTrap;  // New Trap
	int32 newIPL;   // New Interrupt Priority Level
	int32 vec;      // SCB Vector
	int32 idx;

	if (newTrap = GET_TRAP(TIR)) {
#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("VAX: ** Trap **  Reason: %s\n", trapNames[newTrap]);
#endif /* DEBUG */
		vax_DoIntexc(vax, SCB_ARITH, 0, IE_EXC);
		IN_IE = 1;
		WriteV(SP - 4, newTrap, OP_LONG, WA);
		SP -= 4;
		IN_IE = 0;
	} else if (newIPL = GET_IRQ(TIR)) {
#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("VAX: ** Interrupt **  IPL = %d\n", newIPL);
//		if ((newIPL == 24) || (newIPL == 8)) {
//			DBGIPL[newIPL] = dbg_GetMode() | DBG_FLAG;
//			dbg_ClearMode(DBG_TRACE|DBG_DATA);
//		}
#endif /* DEBUG */

		if (newIPL > IPL_HMAX) {
			if (newIPL == IPL_MEMERR) {
				vec    = SCB_MEMERR;
				MEMERR = 0;
			} else
				// Undefined Interrupt
				ABORT(STOP_UIPL);
		} else if (newIPL >= IPL_HMIN) {
			// Hardware Interrupts
//			if (newIPL == IPL_HMAX) {
//				vec = SCB_TIMER;
//			} else if ((vec = vax_GetVector()) == SCB_PASSIVE)
//				return;
			vec = vax->GetVector(vax, newIPL - IPL_HMIN);
			if (vec == SCB_PASSIVE)
				return;
		} else if (newIPL <= IPL_SMAX) {
			// Software Interrupts
			SISR &= ~(1u << newIPL);
			vec = SCB_IPLSOFT + (newIPL << 2);
		} else
			// Undefined Interrupt
			ABORT(STOP_UIPL);

		vax_DoIntexc(vax, vec, newIPL, IE_INT);
	} else
		TIR = 0;
}

int vax_Execute(MAP_DEVICE *map)
{
	register VAX_CPU *vax;
	uint16 opcode;
	int    abValue;
	int    idxInst, idxOpnd;

	void   (*tblOpcode[NUM_INST])();
	uint32 tblOperand[NUM_INST][MAX_SPEC+1];

#ifdef DEBUG
	DBG_BRKSYS *Break;
#endif /* DEBUG */

	vax = ((VAX_SYSTEM *)map->Device)->Processor;

	// Console TTY device must be set.
	if (vax->Console == NULL) {
		printf("No console TTY device - Aborted.\n");
		return STOP_NOCTY;
	}

#ifdef DEBUG
	Break = &vax->Breaks;
#endif /* DEBUG */

//	tblOpcode  = &vax->tblOpcode;
//	tblOperand = &vax->tblOperand;

	for (idxInst = 0; idxInst < NUM_INST; idxInst++) {
		tblOpcode[idxInst] = vax->tblOpcode[idxInst];
		for (idxOpnd = 0; idxOpnd < MAX_SPEC+1; idxOpnd++)
			tblOperand[idxInst][idxOpnd] = vax->tblOperand[idxInst][idxOpnd];
	}

	// Set up host timer system and
	// reset clock (Time of Day).
	ts10_SetAlarm(ts10_TickRealTimer);
	ts10_StartTimer();
	vax->StartTimer(vax);
	if (vax->ResetClock)
		vax->ResetClock(vax);
	vax->ips = 0;

//	PC = goAddr;

	SET_ACCESS; // Reset access mode for memory management
	FLUSH_ISTR; // Reset prefetch instruction buffer first

	// Set up fault trap
	abValue = setjmp(vax->SetJump);
	if (abValue > 0) {
		uint32 pc = faultPC;

		// Return back to TS10 Emulator
		printf("\n\nVAX: %s at PC %08X\n", 
			abName[abValue], faultPC);
#ifdef DEBUG
		vax_Disasm(vax, ts10_Stdout, &pc, SWMASK('v'));
#endif /* DEBUG */
		vax->StopTimer(vax);
		ts10_StopTimer();
		return VAX_HALT;
	} else if (abValue < 0)
		vax_DoFault(vax, -abValue);

	// Main loop for every instruction execution.
	while (emu_State == VAX_RUN) {
		// Save current PC for fault/interrupt use.
		faultPC = PC;
		RQPTR   = 0;

		// Check Simulation Interval Timer
		if (ts10_ClkInterval-- <= 0)
			ts10_ExecuteTimer();

		// Check Trap and Interrupt Requests First
		if (TIR) {
			vax_Interrupt(vax); // Execute Interrupts
			SET_IRQ;            // Evaluate Interrupts
			continue;
		}

		// Check trace pending for debugging purposes.
		// If Trace bit is set, set Trace-Pending bit.
		if (PSL & PSL_TP) {
			PSL &= ~PSL_TP;
			vax_DoIntexc(vax, SCB_TP, 0, IE_EXC);
			continue;
		}
		if (PSW & PSW_T)
			PSL |= PSL_TP;

#ifdef DEBUG
		// Breakpoints here
		if (Break->Switch && dbg_CheckBreak(Break, faultPC, SWMASK('e')))
			ABORT(STOP_BRKPT);

		if (dbg_Check(DBG_TRACE)) {
			int32 pc = PC;
			vax_Disasm(vax, dbg_File, &pc, SWMASK('v'));
		}
#endif /* DEBUG */

		// Fetch instruction opcode from current Program Counter.
		// If opcode is greater than or equal to 0xFD, then
		// opcode is extended opcode.  Fetch another instruction
		// opcode from next byte location.

		vax->ips++;

		opcode = ZXTB(ReadI(OP_BYTE));
		if (opcode >= INST_EXTEND) {
			opcode = (opcode - (INST_EXTEND - 1)) << 8;
			opcode |= ZXTB(ReadI(OP_BYTE));
		}

		OPC = opcode;
		if (tblOperand[opcode][0])
			vax_DecodeOperand(vax, &tblOperand[opcode][0], &PC);
		tblOpcode[opcode](vax);

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_REGISTER))
			vax_DisplayRegisters(vax);
#endif /* DEBUG */
	}

	if (emu_State == VAX_SWHALT) {
		if (vax->HaltAction) {
			// Signal VAX system that halt button was pressed.
			emu_State = VAX_RUN;
			vax->HaltAction(vax, 2);
		}
	}

	printf("VAX: Stopped at PC %08X\n", PC);

	return VAX_OK;
}
