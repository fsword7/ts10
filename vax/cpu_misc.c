// cpu_misc.c - VAX Misc Instructions.
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

// BICPSW - Bit Clear PSW
// BISPSW - Bit Set PSW

DEF_INST(vax, BICPSW)
{
	register int32 mask = ZXTW(OP0);

	if (mask & PSW_MBZ)
		RSVD_OPND_FAULT;

	// Clear all or any bits with mask bits.
	PSW &= ~mask;
	CC  &= ~mask;
}

DEF_INST(vax, BISPSW)
{
	register int32 mask = ZXTW(OP0);

	if (mask & PSW_MBZ)
		RSVD_OPND_FAULT;

	// Set all or any bits with mask bits.
	PSW |= (mask & ~PSW_CC);
	CC  |= (mask & PSW_CC);
}

// BPT  Breakpoint

DEF_INST(vax, BPT)
{
	SET_PC(faultPC);
	vax_DoIntexc(vax, SCB_BPT, 0, IE_EXC);
}

// BUG

DEF_INST(vax, BUGL)
{
#ifdef DEBUG
	uint32 msgError = ReadV(PC, OP_LONG, RA);
	dbg_Printf("BUGL: Bug Check #%08X at PC %08X\n", msgError, faultPC);

	if (msgError & 1) { // If Bugcheck is fatal, turn trace off.
		printf("[Bugcheck #%08X - Trace off]\n", msgError);
		dbg_ClearMode(DBG_TRACE|DBG_DATA|DBG_INTERRUPT);
	}
#endif /* DEBUG */

	RSVD_INST_FAULT;
}

DEF_INST(vax, BUGW)
{
#ifdef DEBUG
	uint16 msgError = ReadV(PC, OP_WORD, RA);
	dbg_Printf("BUGW: Bug Check #%04X at PC %08X\n", msgError, faultPC);

	if (msgError & 1) { // If Bugcheck is fatal, turn trace off.
		printf("[Bugcheck #%04X - Trace off]\n", msgError);
		dbg_ClearMode(DBG_TRACE|DBG_DATA|DBG_INTERRUPT);
	}
#endif /* DEBUG */

	RSVD_INST_FAULT;
}

// HALT - Halt

DEF_INST(vax, HALT)
{
	// If current mode in PSL is not kernel,
	// privileged instruction fault occurs.
	if (PSL & PSL_CUR)
		PRIV_INST_FAULT;

	// Return back to console prompt
	if (vax->HaltAction)
		vax->HaltAction(vax, HALT_INST);
	ABORT(STOP_HALT);
}

// INDEX - Compute Index
//
// Operand Registers:
//   subscript.rl   OP0 = value of operand
//   low.rl         OP1 = value of operand
//   high.rl        OP2 = value of operand
//   size.rl        OP3 = value of operand
//   indexin.rl     OP4 = value of operand
//   indexout.wl    OP5 = memory/register flag
//                  OP6 = memory address

DEF_INST(vax, INDEX)
{
	register int32 idx;

	// Check subscript for out of range first.
	// If so, initiate subscript range trap.
	if ((OP0 < OP1) || (OP0 > OP2))
		SET_TRAP(TRAP_SUBRNG);

	// Compute Index
 	idx = (OP0 + OP4) * OP3;

	// Write result back to indexout.
	LSTORE(OP5, OP6, idx);

	// Update condition flags
	CC_IIZZ_L(idx);
}

// MOVPSL - Move from PSL
//
// Operand Registers:
//   dst.wl: OP0 = memory/register flag
//           OP1 = memory address

DEF_INST(vax, MOVPSL)
{
	LSTORE(OP0, OP1, (PSL | CC));
}

// NOP - No Operation
//
// Operand Register
//   None

DEF_INST(vax, NOP)
{
	// Do nothing here...
}

// XFC - Extended Function Call
//
// Operand Register
//   None

DEF_INST(vax, XFC)
{
	SET_PC(faultPC);
	vax_DoIntexc(vax, SCB_XFC, 0, IE_EXC);
}


DEF_INST(vax, MFPR)
{
	register int32 dst;

	if (PSL & PSL_CUR)
		PRIV_INST_FAULT;
//	dst = vax->ReadIPR(vax, OP0, &CC);
	dst = vax->ReadRegister(vax, OP0);

	LSTORE(OP1, OP2, dst);

	CC_IIZP_I(dst);
}

DEF_INST(vax, MTPR)
{
	register int32 src = OP0;

	if (PSL & PSL_CUR)
		PRIV_INST_FAULT;

	CC_IIZP_I(src);

//	vax->WriteIPR(vax, OP1, src, &CC);
	vax->WriteRegister(vax, OP1, src);
	SET_IRQ;
}


DEF_INST(vax, Illegal)
{
#ifdef DEBUG
	uint8 extend = (OPC >= BMASK) ? ((OPC >> 8) + (INST_EXTEND - 1)) : 0;
	uint8 opcode = OPC;
	char  strOpcode[10];

	if (extend)
		sprintf(strOpcode, "%02X,%02X", extend, opcode);
	else
		sprintf(strOpcode, "%02X", opcode);

	dbg_Printf("CPU: Illegal Instruction (Opcode: %s) at PC %08X\n",
		strOpcode, faultPC);
#endif /* DEBUG */

	RSVD_INST_FAULT;
}

DEF_INST(vax, Unimplemented)
{
#ifdef DEBUG
	uint8 extend = (OPC >= BMASK) ? ((OPC >> 8) + (INST_EXTEND - 1)) : 0;
	uint8 opcode = OPC;
	char  strOpcode[10];

	if (extend)
		sprintf(strOpcode, "%02X,%02X", extend, opcode);
	else
		sprintf(strOpcode, "%02X", opcode);

	dbg_Printf("CPU: Unimplemented Instruction (Opcode: %s) at PC %08X\n",
		strOpcode, faultPC);
#endif /* DEBUG */

	RSVD_INST_FAULT;
}

DEF_INST(vax, Emulate)
{
	uint8 extend = (OPC >= BMASK) ? ((OPC >> 8) + (INST_EXTEND - 1)) : 0;
	uint8 opcode = OPC;

	vax_Emulate(vax, opcode);
}
