// cpu_branch.c - VAX Branch Instructions
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

// *******************************************************
// *********** Condition branch instructions *************
// *******************************************************

DEF_INST(vax, ACBB)
{
	int8 limit = OP0;
	int8 add   = OP1;
	int8 idx   = OP2;
	int8 ridx; // Result of Index
#ifdef DEBUG
	int32 old_PC = PC;
#endif /* DEBUG */
	
	ridx = idx + add;
	BSTORE(OP3, OP4, ridx);

	// Update condition codes
	CC_IIZP_B(ridx);
	V_ADD_B(ridx, add, idx);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ACBB: %02X + %02X => %02X  CC: %s\n",
			idx, add, ridx, CC_DSPL(CC));
#endif /* DEBUG */

	if ((add < 0) ? (ridx >= limit) : (ridx <= limit)) {
		SET_PC(PC + SXTW(vax->brDisp));
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("ACBB: PC %08X + %04X => %08X\n",
				old_PC, vax->brDisp, PC);
			dbg_Printf("ACBB: Jump into location %08X\n", PC);
		}
#endif /* DEBUG */
	}
}

DEF_INST(vax, ACBW)
{
	int16 limit = OP0;
	int16 add   = OP1;
	int16 idx   = OP2;
	int16 ridx; // Result of Index
#ifdef DEBUG
	int32 old_PC = PC;
#endif /* DEBUG */
	
	ridx = idx + add;
	WSTORE(OP3, OP4, ridx);

	// Update condition codes
	CC_IIZP_W(ridx);
	V_ADD_W(ridx, add, idx);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ACBW: %04X + %04X => %04X  CC: %s\n",
			idx, add, ridx, CC_DSPL(CC));
#endif /* DEBUG */

	if ((add < 0) ? (ridx >= limit) : (ridx <= limit)) {
		SET_PC(PC + SXTW(vax->brDisp));
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("ACBW: PC %08X + %04X => %08X\n",
				old_PC, vax->brDisp, PC);
			dbg_Printf("ACBW: Jump into location %08X\n", PC);
		}
#endif /* DEBUG */
	}
}

DEF_INST(vax, ACBL)
{
	int32 limit = OP0;
	int32 add   = OP1;
	int32 idx   = OP2;
	int32 ridx; // Result of Index
#ifdef DEBUG
	int32 old_PC = PC;
#endif /* DEBUG */
	
	ridx = idx + add;
	LSTORE(OP3, OP4, ridx);

	// Update condition codes
	CC_IIZP_L(ridx);
	V_ADD_L(ridx, add, idx);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ACBL: %08X + %08X => %08X  CC: %s\n",
			idx, add, ridx, CC_DSPL(CC));
#endif /* DEBUG */

	if ((add < 0) ? (ridx >= limit) : (ridx <= limit)) {
		SET_PC(PC + SXTW(vax->brDisp));
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("ACBL: PC %08X + %04X => %08X\n",
				old_PC, vax->brDisp, PC);
			dbg_Printf("ACBL: Jump into location %08X\n", PC);
		}
#endif /* DEBUG */
	}
}

DEF_INST(vax, AOBLEQ)
{
	register int32 limit = OP0;
	register int32 index = OP1;
	register int32 res;
#ifdef DEBUG
	int32 old_PC = PC;
#endif /* DEBUG */

	res = index + 1;
	LSTORE(OP2, OP3, res);

	// Update condition codes
	CC_IIZP_L(res);
	V_ADD_L(res, 1, index);

#ifdef DEBUG
	if (res <= limit) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("AOBLEQ: PC %08X + %02X => %08X\n",
				old_PC, vax->brDisp, PC);
			dbg_Printf("AOBLEQ: Jump into location %08X\n", PC);
		}
	}
#else
	if (res <= limit)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, AOBLSS)
{
	register int32 limit = OP0;
	register int32 index = OP1;
	register int32 res;
#ifdef DEBUG
	int32 old_PC = PC;
#endif /* DEBUG */

	res = index + 1;
	LSTORE(OP2, OP3, res);

	// Update condition codes
	CC_IIZP_L(res);
	V_ADD_L(res, 1, index);

#ifdef DEBUG
	if (res < limit) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("AOBLSS: PC %08X + %02X => %08X\n",
				old_PC, vax->brDisp, PC);
			dbg_Printf("AOBLSS: Jump into location %08X\n", PC);
		}
	}
#else
	if (res < limit)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

// Bcc - Branch on (condition)
//
// Format:
//   opcode displ.bb
//
// Operation:
//   if condition then PC <- PC + SEXT(displ);
//
// Condition Codes:
//   None affected.
//
// Expections:
//   None
//
// Opcodes: Condition
//   12  Z EQL 0         BNEQ  Branch on Not Equal (signed)
//   12  Z EQL 0         BNEQU Branch on Not Equal Unsigned
//   13  Z EQL 1         BEQL  Branch on Equal (signed)
//   13  Z EQL 1         BEQLU Branch on Equal Unsigned
//   14  (N OR Z) EQL 0  BGTR  Branch on Greater Than (signed)
//   15  (N OR Z) EQL 1  BLEQ  Branch on Less Than or Equal (signed)
//   18  N EQL 0         BGEQ  Branch on Greater Than or Equal (signed)
//   19  N EQL 1         BLSS  Branch on Less Than (signed)
//   1A  (C OR Z) EQL 0  BGTRU Branch on Greater Than Unsigned
//   1B  (C OR Z) EQL 1  BLEQU Branch on Less Than or Equal Unsigned
//   1C  V EQL 0         BVC   Branch on Overflow Clear
//   1D  V EQL 1         BVS   Branch on Overflow Set
//   1E  C EQL 0         BGEQU Branch on Greater Than or Equal Unsigned
//   1E  C EQL 0         BCC   Branch on Carry Clear
//   1F  C EQL 1         BLSSU Branch on Less Than Unsigned
//   1F  C EQL 1         BCS   Branch on Carry Set

DEF_INST(vax, BGTR)
{
#ifdef DEBUG
	int old_pc = PC;

	if ((CC & (CC_N|CC_Z)) == 0) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BGTR: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if ((CC & (CC_N|CC_Z)) == 0)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BLEQ)
{
#ifdef DEBUG
	int old_pc = PC;

	if (CC & (CC_N|CC_Z)) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BLEQ: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if (CC & (CC_N|CC_Z))
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BNEQ)
{
#ifdef DEBUG
	int old_pc = PC;

	if ((CC & CC_Z) == 0) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BNEQ: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if ((CC & CC_Z) == 0)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BEQL)
{
#ifdef DEBUG
	int old_pc = PC;

	if (CC & CC_Z) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BEQL: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if (CC & CC_Z)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BGEQ)
{
#ifdef DEBUG
	int old_pc = PC;

	if ((CC & CC_N) == 0) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BGEQ: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if ((CC & CC_N) == 0)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BLSS)
{
#ifdef DEBUG
	int old_pc = PC;

	if (CC & CC_N) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BLSS: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if (CC & CC_N)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BGTRU)
{
#ifdef DEBUG
	int old_pc = PC;

	if ((CC & (CC_C|CC_Z)) == 0) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BGTRU: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if ((CC & (CC_C|CC_Z)) == 0)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BLEQU)
{
#ifdef DEBUG
	int old_pc = PC;

	if (CC & (CC_C|CC_Z)) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BLEQU: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if (CC & (CC_C|CC_Z))
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BVC)
{
#ifdef DEBUG
	int old_pc = PC;

	if ((CC & CC_V) == 0) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BVC: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if ((CC & CC_V) == 0)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BVS)
{
#ifdef DEBUG
	int old_pc = PC;

	if (CC & CC_V) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BVS: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if (CC & CC_V)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BCC)
{
#ifdef DEBUG
	int old_pc = PC;

	if ((CC & CC_C) == 0) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BCC: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if ((CC & CC_C) == 0)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, BCS)
{
#ifdef DEBUG
	int old_pc = PC;

	if (CC & CC_C) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("BCS: PC %08X + %02X => %08X\n",
				old_pc, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}

#else
	if (CC & CC_C)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

// BB - Branch on Bit
//
// Opcodes:
//   E0  BBS  Branch on Bit Set
//   E1  BBC  Branch on Bit Clear
//
// Operand Registers:
//   pos.rl       OP0 = value of operand
//   base.vb      OP1 = register/memory flag
//                OP2 = content/memory address
//   displ.bb

inline int vax_GetBit(register VAX_CPU *vax, int32 *opnd)
{
	int32  pos  = opnd[0];
	int32  nReg = opnd[1];
	uint32 ea, src;

	if (nReg >= 0) {
		if ((uint32)pos > 31)
			RSVD_OPND_FAULT;
		src = RN(nReg);
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("BB: R%d %08X<%d> => %d\n",
				nReg, src, pos, (src >> pos) & 1);
#endif /* DEBUG */
	} else {
		ea  = opnd[2] + (pos >> 3);
		src = ReadV(ea, OP_BYTE, RA);
		pos &= 0x07;
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("BB: %08X<%d> => (%08X) %02X<%d> => %d\n",
				opnd[2], opnd[0], ea, src, pos, (src >> pos) & 1);
#endif /* DEBUG */
	}

	return (src >> pos) & 1;
}

DEF_INST(vax, BBS)
{
	if (vax_GetBit(vax, &OP0))
		SET_PC(PC + SXTB(vax->brDisp));
}

DEF_INST(vax, BBC)
{
	if (vax_GetBit(vax, &OP0) == 0)
		SET_PC(PC + SXTB(vax->brDisp));
}

// BB - Branch on Bit (and modify without Interlock)
//
// Opcodes:
//   E2  BBSS  Branch on Bit Set and Set
//   E3  BBCS  Branch on Bit Clear and Set
//   E4  BBSC  Branch on Bit Set and Clear
//   E5  BBCC  Branch on Bit Clear and Clear
//
// Operand Registers:
//   pos.rl       OP0 = value of operand
//   base.vb      OP1 = register/memory flag
//                OP2 = content/memory address
//   displ.bb

inline int vax_SetBit(register VAX_CPU *vax, int32 *opnd, int32 setBit)
{
	int32  Pos  = opnd[0];
	int32  nReg = opnd[1];
	int32  eAddr;
	uint8  curBit, newBit;
	int    oldBit;

	if (nReg >= 0) {
		if ((uint32)Pos > 31)
			RSVD_OPND_FAULT;
		oldBit = (RN(nReg) >> Pos) & 1;
		RN(nReg) = setBit ? (RN(nReg) | (1 << Pos)) :
		                    (RN(nReg) & ~(1 << Pos));
	} else {
		eAddr = opnd[2] + (Pos >> 3);
		Pos &= 0x07;
		curBit = ReadV(eAddr, OP_BYTE, WA);
		oldBit = (curBit >> Pos) & 1;
		newBit = setBit ? (curBit | (1 << Pos)) :
		                  (curBit & ~(1 << Pos));
		WriteV(eAddr, newBit, OP_BYTE, WA);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BB: %08X<%d> (%08X) %01X & %01X => %01X\n",
			opnd[2], Pos, eAddr, curBit, setBit, newBit);
#endif /* DEBUG */
	}

	return oldBit;
}

DEF_INST(vax, BBCC)
{
	if (vax_SetBit(vax, &OP0, 0) == 0)
		SET_PC(PC + SXTB(vax->brDisp));
}

DEF_INST(vax, BBCS)
{
	if (vax_SetBit(vax, &OP0, 1) == 0)
		SET_PC(PC + SXTB(vax->brDisp));
}

DEF_INST(vax, BBSC)
{
	if (vax_SetBit(vax, &OP0, 0))
		SET_PC(PC + SXTB(vax->brDisp));
}

DEF_INST(vax, BBSS)
{
	if (vax_SetBit(vax, &OP0, 1))
		SET_PC(PC + SXTB(vax->brDisp));
}

// BLB  Branch on Low Bit
//
// Opcodes:
//   E8  BLBS  Branch on Low Bit Set
//   E9  BLBC  Branch on Low Bit Clear
//
// Operand Registers:
//   src.rl    OP0 = value of operand
//   displ.bb

DEF_INST(vax, BLBS)
{
	if (OP0 & 1)
		SET_PC(PC + SXTB(vax->brDisp));

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BLBS: %08X & 1 => %d\n", OP0, OP0 & 1);
#endif /* DEBUG */
}

DEF_INST(vax, BLBC)
{
	if ((OP0 & 1) == 0)
		SET_PC(PC + SXTB(vax->brDisp));

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BLBC: %08X & 1 => %d\n", OP0, OP0 & 1);
#endif /* DEBUG */
}

// CASE  Case
//
// Opcodes:
//
//   8F  CASEB  Case Byte
//   AF  CASEW  Case Word
//   CF  CASEL  Case Long
//
// Operand Registers:
//
//   selctor.rx   OP0 = value of operand
//   base.rx      OP1 = value of operand
//   limit.rx     OP2 = value of operand

DEF_INST(vax, CASEB)
{
	int8  selector = OP0;
	int8  base     = OP1;
	int8  limit    = OP2;
	int8  tmp      = selector - base;
	int16 disp;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("CASEB: (%02X - %02X) => %02X, %02X <= %02X\n",
			selector, base, tmp, tmp, limit);
		if (tmp <= limit)
			dbg_Printf("CASEB: %08X + %02X => %08X\n",
				PC, tmp << 1, PC + (tmp << 1));
		else
			dbg_Printf("CASEB: %08X + %02X + 2 => %08X\n",
				PC, limit << 1, PC + (limit << 1) + 2);
	}
#endif /* DEBUG */

	// Update condition codes
	CC_CMP_B(tmp, limit);

	if ((uint8)tmp <= (uint8)limit) {
		disp = ReadV(PC + ((uint8)tmp << 1), OP_WORD, RA);
		SET_PC(PC + disp);
	} else
		SET_PC(PC + ((uint8)limit << 1) + 2);
}

DEF_INST(vax, CASEW)
{
	int16 selector = OP0;
	int16 base     = OP1;
	int16 limit    = OP2;
	int16 tmp      = selector - base;
	int16 disp;

	// Update condition codes
	CC_CMP_W(tmp, limit);

	if ((uint16)tmp <= (uint16)limit) {
		disp = ReadV(PC + ((uint16)tmp << 1), OP_WORD, RA);
		SET_PC(PC + disp);
	} else
		SET_PC(PC + ((uint16)limit << 1) + 2);
}

DEF_INST(vax, CASEL)
{
	int32 selector = OP0;
	int32 base     = OP1;
	int32 limit    = OP2;
	int32 tmp      = selector - base;
	int16 disp;

	// Update condition codes
	CC_CMP_L(tmp, limit);

	if ((uint32)tmp <= (uint32)limit) {
		disp = ReadV(PC + ((uint32)tmp << 1), OP_WORD, RA);
		SET_PC(PC + disp);
	} else
		SET_PC(PC + ((uint32)limit << 1) + 2);
}

// *******************************************************
// **************** Branch instructions ******************
// *******************************************************

// BR - Branch
//
// Format:
//   opcode displ.bx
//
// Operation:
//   PC <- PC + SEXT(displ);
//
// Conditions:
//   None affected.
//
// Expections:
//   None
//
// Opcodes:
//   11  BRB  Branch With Byte Displacement
//   31  BRW  Branch With Word Displacement

DEF_INST(vax, BRB)
{
#ifdef DEBUG
	int old_pc = PC;
#endif /* DEBUG */

	SET_PC(PC + SXTB(vax->brDisp));

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("BRB: PC %08X + %02X => %08X\n",
			old_pc, vax->brDisp, PC);
		dbg_Printf("VAX: Jump into location %08X\n", PC);
	}
#endif /* DEBUG */
}

DEF_INST(vax, BRW)
{
#ifdef DEBUG
	int old_pc = PC;
#endif /* DEBUG */

	SET_PC(PC + SXTW(vax->brDisp));

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("BRW: PC %08X + %04X => %08X\n",
			old_pc, vax->brDisp, PC);
		dbg_Printf("VAX: Jump into location %08X\n", PC);
	}
#endif /* DEBUG */
}

// JMP - Jump
//
// Format:
//   opcode dst.ab
//
// Operation:
//   PC <- dst
//
// Condition Codes:
//   None affected.
//
// Expections:
//   None
//
// Opcode:
//   17  JMP  Jump
//
// Description:
//   PC is replaced by the destination operand.

DEF_INST(vax, JMP)
{
	SET_PC(OP0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("JMP: PC <= %08X\n", OP0);
		dbg_Printf("VAX: Jump into location %08X\n", PC);
	}
#endif /* DEBUG */
}

// *******************************************************
// ************** Subroutine instructions ****************
// *******************************************************

// BSB - Branch to Subroutine
//
// Format:
//   opcode displ.bx
//
// Operation:
//   -(SP) <- PC;
//   PC <- PC + SEXT(displ);
//
// Condition Codes:
//   None affected.
//
// Execptions:
//   None
//
// Opcodes:
//   10  BSBB  Branch to Subroutine With Byte Displacement
//   30  BSBW  Branch to Subroutine With Word Displacement

DEF_INST(vax, BSBB)
{
#ifdef DEBUG
	int oldPC = PC;
#endif /* DEBUG */

	WriteV(SP - OP_LONG, PC, OP_LONG, WA);
	SP -= OP_LONG;
	SET_PC(PC + SXTB(vax->brDisp));

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("BSBB: PC %08X + %02X => %08X\n",
			oldPC, vax->brDisp, PC);
		dbg_Printf("BSBB: -(SP %08X) <= %08X <= PC, PC <= %08X\n",
			SP, oldPC, PC);
		dbg_Printf("VAX: Subroutine into location %08X\n", PC);
	}
#endif /* DEBUG */
}

DEF_INST(vax, BSBW)
{
#ifdef DEBUG
	int oldPC = PC;
#endif /* DEBUG */

	WriteV(SP - OP_LONG, PC, OP_LONG, WA);
	SP -= OP_LONG;
	SET_PC(PC + SXTW(vax->brDisp));

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("BSBW: PC %08X + %04X => %08X\n",
			oldPC, vax->brDisp, PC);
		dbg_Printf("BSBW: -(SP %08X) <= %08X <= PC, PC <= %08X\n",
			SP, oldPC, PC);
		dbg_Printf("VAX: Subroutine into location %08X\n", PC);
	}
#endif /* DEBUG */
}

// JSB - Jump to Subroutine
//
// Format:
//   opcode dst.ab
//
// Operations:
//   -(SP) <- PC;
//   PC <- dst;
//
// Condition Codes:
//   None affected.
//
// Exceptions:
//   None
//
// Opcodes:
//   16  JSB  Jump to Subroutine

DEF_INST(vax, JSB)
{
#ifdef DEBUG
	int32 oldPC = PC;
#endif /* DEBUG */

	WriteV(SP - OP_LONG, PC, OP_LONG, WA);
	SP -= OP_LONG;
	SET_PC(OP0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("JSB: -(SP %08X) <= %08X <= PC, PC <= %08X\n",
			SP, oldPC, OP0);
		dbg_Printf("VAX: Subroutine into location %08X\n", PC);
	}
#endif /* DEBUG */
}

// RSB - Return from Subroutine
//
// Format:
//   opcode
//
// Operations:
//   PC <- (SP)+;
//
// Condition Codes:
//   None affected.
//
// Execptions:
//   None
//
// Opcodes:
//   05  RSB  Return From Subroutine

DEF_INST(vax, RSB)
{
	SET_PC(ReadV(SP, OP_LONG, RA));

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("RSB: (SP %08X)+ => %08X => PC\n", SP, PC);
		dbg_Printf("VAX: Return from subroutine %08X\n", faultPC);
	}
#endif /* DEBUG */

	SP += OP_LONG;
}

DEF_INST(vax, SOBGEQ)
{
	register int32 index = OP0;
	register int32 res;
#ifdef DEBUG
	int32 old_PC = PC;
#endif /* DEBUG */

	res = index - 1;
	LSTORE(OP1, OP2, res);

	// Update condition codes
	CC_IIZP_L(res);
	V_SUB_L(res, 1, index);

#ifdef DEBUG
	if (res >= 0) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("SOBGEQ: PC %08X + %02X => %08X\n",
				old_PC, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	}
#else
	if (res >= 0)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}

DEF_INST(vax, SOBGTR)
{
	register int32 index = OP0;
	register int32 res;
#ifdef DEBUG
	int32 old_PC = PC;
#endif /* DEBUG */

	res = index - 1;
	LSTORE(OP1, OP2, res);

	// Update condition codes
	CC_IIZP_L(res);
	V_SUB_L(res, 1, index);

#ifdef DEBUG
	if (res > 0) {
		SET_PC(PC + SXTB(vax->brDisp));
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("SOBGTR: Countdown %08X - Jump.\n", res);
			dbg_Printf("SOBGTR: PC %08X + %02X => %08X\n",
				old_PC, vax->brDisp, PC);
			dbg_Printf("VAX: Jump into location %08X\n", PC);
		}
	} else if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("SOBGTR: Countdown %08X - Done.\n", res);
#else
	if (res > 0)
		SET_PC(PC + SXTB(vax->brDisp));
#endif /* DEBUG */
}
