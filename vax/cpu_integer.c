// cpu_integer.c - Integer/Logical Arithmetic Instructions
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

DEF_INST(vax, ADAWI)
{
	register int16 src = OP0;
	register int16 tmp, dst;

	if (OP1 >= 0) {
		tmp = RN(OP1);
		dst = RN(OP1) = ZXTW(src + tmp);
	} else {
		if (OP2 & 1)
			RSVD_OPND_FAULT;
		tmp = ReadV(OP2, OP_WORD, WA);
		dst = src + tmp;
		WriteV(OP2, dst, OP_WORD, WA);
	}

	// Update condition codes
	CC_ADD_W(dst, src, tmp);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ADAWI: %04X + %04X => %04X  CC: %s\n",
			src, tmp, dst, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

// ADD  Add
//
// Opcodes:
//
// Operand Registers:
//
//   opcode w/2 operands
//     add.rx   OP0 = value of operand
//     sum.mx   OP1 = value of operand
//              OP2 = register/memory flag
//              OP3 = memory address
//
//   opcode w/3 operands
//     add1.rx  OP0 = value of operand
//     add2.rx  OP1 = value of operand
//     sum.wx   OP2 = register/memory flag
//              OP3 = memory address

DEF_INST(vax, ADDB)
{
	register int8 add1 = OP0;
	register int8 add2 = OP1;
	register int8 sum;

	sum = add2 + add1;
	BSTORE(OP2, OP3, sum);

	// Update condition codes
	CC_ADD_B(sum, add1, add2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ADDB: %02X + %02X => %02X  CC: %s\n",
			add1, add2, sum, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

DEF_INST(vax, ADDW)
{
	register int16 add1 = OP0;
	register int16 add2 = OP1;
	register int16 sum;

	sum = add2 + add1;
	WSTORE(OP2, OP3, sum);

	// Update condition codes
	CC_ADD_W(sum, add1, add2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ADDW: %04X + %04X => %04X  CC: %s\n",
			add1, add2, sum, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

DEF_INST(vax, ADDL)
{
	register int32 add1 = OP0;
	register int32 add2 = OP1;
	register int32 sum;

	sum = add2 + add1;
	LSTORE(OP2, OP3, sum);

	// Update condition codes
	CC_ADD_L(sum, add1, add2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ADDL: %08X + %08X => %08X  CC: %s\n",
			add1, add2, sum, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

// ADWC  Add With Carry
//
// Opcode:
//
//   D8  ADWC  Add With Carry
//
// Operand Registers:
//
//   add.rl   OP0 = value of operand
//   sum.rl   OP1 = value of operand
//   sum.wl   OP2 = register/memory flag
//            OP3 = memory address

DEF_INST(vax, ADWC)
{
	register int32 add1 = OP0;
	register int32 add2 = OP1;
	register int32 sum;
#ifdef DEBUG
	int32 carry = CC & CC_C;
#endif /* DEBUG */

	sum = add1 + add2 + (CC & CC_C);
	LSTORE(OP2, OP3, sum);

	// Update condition codes
	CC_ADD_L(sum, add1, add2);
	if ((sum == add2) && add1)
		CC |= CC_C;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ADWC: %08X + %08X + %d => %08X  CC: %s\n",
			add1, add2, sum, carry, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

// ASH  Arithmetic Shift
//
// Opcodes:
//
//   78  ASHL  Arithmetic Shift Long
//   79  ASHQ  Arithmetic Shift Quad
//
// Operand Registers:
//
//   cnt.rb    OP0 = value of operand
//   src.rx    OP1 = value of operand
//   dst.wx    OP2 = register/memory flag
//             OP3 = memory address

DEF_INST(vax, ASHL)
{
	register int8  cnt = OP0;
	register int32 src = OP1;
	register int32 dst, ovflg = 0;

	// Do 32-bit arithmetic shift
	if (cnt == 0) {
		dst = src;
		ovflg = 0;
	} else {
		if (cnt < 0) {
			if (cnt > -32)
				dst = src >> -cnt;
			else 
				dst = (src < 0) ? -1 : 0;
		} else {
			if (cnt < 32) {
				dst   = (uint32)src << cnt;
				ovflg = (src != (dst >> cnt));
			} else {
				dst   = 0;
				ovflg = (src != 0);
			}
		}
	}

	// Write results back.
	LSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZZ_L(dst);
	if (ovflg) V_INTOVF;

	// If overflow occurs, initiate integer overflow trap.
//	if (ovflg) {
//		CC |= CC_V;
//		if (PSW & PSW_IV)
//			SET_TRAP(TRAP_INTOVF);
//	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ASHL: %08X %s %d => %08X  CC: %s\n",
			src, ((cnt < 0) ? ">>" : "<<"), abs(cnt), dst, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

DEF_INST(vax, ASHQ)
{
	int8  cnt = OP0;
	int64 src = ((uint64)OP2 << 32) | (uint32)OP1;
	int64 dst;
	int   ovflg;

	// Do 64-bit arithmetic shift
	if (cnt == 0) {
		dst = src;
		ovflg = 0;
	} else {
		if (cnt < 0) {
			// Shift right
			if (cnt > -64)
				dst = src >> -cnt;
			else 
				dst = (src < 0) ? -1LL : 0LL;
			ovflg = 0;
		} else {
			// Shift left
			if (cnt < 64) {
				dst   = src << cnt;
				ovflg = (src != (dst >> cnt));
			} else {
				dst = 0LL;
				ovflg = (src != 0);
			}
		}
	}

	// Write results back.
	QSTORE(OP3, OP4, (int32)dst, (int32)(dst >> 32));
 
	// Update condition codes
	CC_IIZZ_I(dst);
	if (ovflg) V_INTOVF;

	// If overflow occurs, initiate integer overflow trap.
//	if (ovflg) {
//		CC |= CC_V;
//		if (PSW & PSW_IV)
//			SET_TRAP(TRAP_INTOVF);
//	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ASHQ: %016llX %s %d => %016llX  CC: %s\n",
			src, ((cnt < 0) ? ">>" : "<<"), abs(cnt), dst, CC_DSPL(CC));
	}
#endif /* DEBUG */
}


DEF_INST(vax, BICB)
{
	register int8 mask = OP0;
	register int8 src  = OP1;
	register int8 dst;

	dst = src & ~mask;
	BSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_B(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BICB: %02X & ~%02X => %02X  CC: %s\n",
			src, mask, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, BICW)
{
	register int16 mask = OP0;
	register int16 src  = OP1;
	register int16 dst;

	dst = src & ~mask;
	WSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_W(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BICW: %04X & ~%04X => %04X  CC: %s\n",
			src, mask, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, BICL)
{
	register int32 mask = OP0;
	register int32 src  = OP1;
	register int32 dst;

	dst = src & ~mask;
	LSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_L(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BICL: %08X & ~%08X => %08X  CC: %s\n",
			src, mask, dst, CC_DSPL(CC));
#endif /* DEBUG */
}


DEF_INST(vax, BISB)
{
	register int8 mask = OP0;
	register int8 src  = OP1;
	register int8 dst;

	dst = src | mask;
	BSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_B(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BISB: %02X | %02X => %02X  CC: %s\n",
			src, mask, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, BISW)
{
	register int16 mask = OP0;
	register int16 src  = OP1;
	register int16 dst;

	dst = src | mask;
	WSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_W(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BISW: %04X | %04X => %04X  CC: %s\n",
			src, mask, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, BISL)
{
	register int32 mask = OP0;
	register int32 src  = OP1;
	register int32 dst;

	dst = src | mask;
	LSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_L(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BISL: %08X | %08X => %08X  CC: %s\n",
			src, mask, dst, CC_DSPL(CC));
#endif /* DEBUG */
}


DEF_INST(vax, BITB)
{
	register int8 mask = OP0;
	register int8 src  = OP1;
	register int8 tmp;

	tmp = src & mask;

	// Update condition codes
	CC_IIZP_B(tmp);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BITB: %02X & %02X => %02X  CC: %s\n",
			src, mask, tmp, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, BITW)
{
	register int16 mask = OP0;
	register int16 src  = OP1;
	register int16 tmp;

	tmp = src & mask;

	// Update condition codes
	CC_IIZP_W(tmp);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BITW: %04X & %04X => %04X  CC: %s\n",
			src, mask, tmp, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, BITL)
{
	register int32 mask = OP0;
	register int32 src  = OP1;
	register int32 tmp;

	tmp = src & mask;

	// Update condition codes
	CC_IIZP_L(tmp);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BITL: %08X & %08X => %08X  CC: %s\n",
			src, mask, tmp, CC_DSPL(CC));
#endif /* DEBUG */
}

// CLR  Clear
//
// Opcodes:
//   94    CLRB  Clear Byte
//   B4    CLRW  Clear Word
//   D4    CLRL  Clear Long
//   7C    CLRQ  Clear Quad
//   7CFD  CLRO  Clear Octa
//
// Operand Registers:
//   dst.wx   OP0 = register/memory flag
//            OP1 = memory address

DEF_INST(vax, CLRB)
{
	BSTORE(OP0, OP1, 0);
	CC_Z1ZP; // Update condition codes
}

DEF_INST(vax, CLRW)
{
	WSTORE(OP0, OP1, 0);
	CC_Z1ZP; // Update condition codes
}

DEF_INST(vax, CLRL)
{
	LSTORE(OP0, OP1, 0);
	CC_Z1ZP; // Update condition codes
}

DEF_INST(vax, CLRQ)
{
	QSTORE(OP0, OP1, 0, 0);
	CC_Z1ZP; // Update condition codes
}

// **********************************************************
// ******************** DEC  Decrement **********************
// **********************************************************

DEF_INST(vax, DECB)
{
	register int8 src = OP0;
	register int8 dst;

	dst = src - 1;
	BSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_SUB_B(dst, 1, src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DECB: %02X => %02X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, DECW)
{
	register int16 src = OP0;
	register int16 dst;

	dst = src - 1;
	WSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_SUB_W(dst, 1, src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DECW: %04X => %04X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, DECL)
{
	register int32 src = OP0;
	register int32 dst;

	dst = src - 1;
	LSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_SUB_L(dst, 1, src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DECL: %08X => %08X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

// **********************************************************
// ********************* DIV  Divide ************************
// **********************************************************

DEF_INST(vax, DIVB)
{
	register int8 divr = OP0;
	register int8 divd = OP1;
	register int8 quo;
	int flg;

	if (divr == 0) {
		quo = divd;
		flg = CC_V;
		SET_TRAP(TRAP_INTDIV);
	} else if ((divr == -1) && (divd == -128)) {
		quo = divd;
		flg = CC_V;
		INTOVF;
	} else {
		quo = divd / divr;
		flg = 0;
	}

	BSTORE(OP2, OP3, quo);

	// Update condition codes
	CC_IIZZ_B(quo);
	CC |= flg;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DIVB: %02X / %02X => %02X  CC: %s\n",
			divd, divr, quo, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, DIVW)
{
	register int16 divr = OP0;
	register int16 divd = OP1;
	register int16 quo;
	int flg;

	if (divr == 0) {
		quo = divd;
		flg == CC_V;
		SET_TRAP(TRAP_INTDIV);
	} else if ((divr == -1) && (divd == -32768)) {
		quo = divd;
		flg = CC_V;
		INTOVF;
	} else {
		quo = divd / divr;
		flg = 0;
	}

	WSTORE(OP2, OP3, quo);

	// Update condition codes
	CC_IIZZ_W(quo);
	CC |= flg;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DIVW: %04X / %04X => %04X  CC: %s\n",
			divd, divr, quo, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, DIVL)
{
	register int32 divr = OP0;
	register int32 divd = OP1;
	register int32 quo;
	int flg;

	if (divr == 0) {
		quo = divd;
		flg = CC_V;
		SET_TRAP(TRAP_INTDIV);
	} else if ((divr == LMASK) && (divd == LSIGN)) {
		quo = divd;
		flg = CC_V;
		INTOVF;
	} else {
		quo = divd / divr;
		flg = 0;
	}

	LSTORE(OP2, OP3, quo);

	CC_IIZZ_L(quo);
	CC |= flg;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DIVL: %08X / %08X => %08X  CC: %s\n",
			divd, divr, quo, CC_DSPL(CC));
#endif /* DEBUG */
}

// **********************************************************
// ********* EMUL/EDIV - Extended Multiply/Divide ***********
// **********************************************************

DEF_INST(vax, EDIV)
{
	int64   dvr = ((OP0 < 0) ? -OP0 : OP0) & LMASK;
	int64   dvd = ((uint64)OP2 << 32) | (uint32)OP1;
	int32   quo, rem;
	int     ovflg = 0;

	if (dvr == 0)
		ovflg = TRAP_INTDIV;
	else {
		if (dvd < 0)
			dvd = -dvd;
		if (((dvd >> 32) & LMASK) >= dvr) {
			ovflg = TRAP_INTOVF;
		} else {
			quo = dvd / dvr;
			rem = dvd % dvr;
		}
		if ((OP0 ^ OP2) & LSIGN) {
			quo = -quo;
			if (quo && ((quo & LSIGN) == 0))
				ovflg = TRAP_INTOVF;
		} else if (quo < 0)
			ovflg = TRAP_INTOVF;
		if (OP2 < 0)
			rem = -rem;
	}

	if (ovflg) {
		quo = OP1;
		rem = 0;
		if (ovflg == TRAP_INTDIV)
			SET_TRAP(TRAP_INTDIV);
		else if (PSW & PSW_IV)
			SET_TRAP(TRAP_INTOVF);
		ovflg = CC_V;
	}

	LSTORE(OP3, OP4, quo);
	LSTORE(OP5, OP6, rem);

	// Update condition codes
	CC_IIZZ_L(quo);
	CC |= ovflg;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("EDIV: %016llX / %08X => %08X R %08X  CC: %s\n",
			dvd, (int32)dvr, quo, rem, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, EMUL)
{
	int64 mulr = OP0;
	int64 muld = OP1;
	int32 add  = OP2;
	int64 prod;

	prod = (muld * mulr) + add;

	QSTORE(OP3, OP4, (int32)prod, (int32)(prod >> 32));

	// Update condition codes
	CC_IIZZ_I(prod);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("EMUL: (%08X * %08X) + %08X => %016llX  CC: %s\n",
			(int32)muld, (int32)mulr, add, prod, CC_DSPL(CC));
#endif /* DEBUG */
}

// **********************************************************
// ******************** INC  Increment **********************
// **********************************************************

DEF_INST(vax, INCB)
{
	register int8 src = OP0;
	register int8 dst;

	dst = src + 1;
	BSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_ADD_B(dst, 1, src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("INCB: %02X => %02X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, INCW)
{
	register int16 src = OP0;
	register int16 dst;

	dst = src + 1;
	WSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_ADD_W(dst, 1, src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("INCW: %04X => %04X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, INCL)
{
	register int32 src = OP0;
	register int32 dst;

	dst = src + 1;
	LSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_ADD_L(dst, 1, src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("INCL: %08X => %08X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

// **********************************************************
// ******************** MUL  Multiply ***********************
// **********************************************************

DEF_INST(vax, MULB)
{
	register int8  mulr = OP0;
	register int8  muld = OP1;
	register int32 prod;

	prod = mulr * muld;
	BSTORE(OP2, OP3, prod);

	// Update condition codes
	CC_IIZZ_B((int8)prod);

	if ((prod > 127) || (prod < -128)) {
		CC |= CC_V;
		if (PSW & PSW_IV)
			SET_TRAP(TRAP_INTOVF);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MULB: %02X * %02X => %02X  CC: %s\n",
			mulr, muld, prod, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MULW)
{
	int16 mulr = OP0;
	int16 muld = OP1;
	int32 prod;

	prod = mulr * muld;
	WSTORE(OP2, OP3, prod);

	// Update condition codes
	CC_IIZZ_W((int16)prod);

	if ((prod > 32767) || (prod < -32768)) {
		CC |= CC_V;
		if (PSW & PSW_IV)
			SET_TRAP(TRAP_INTOVF);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MULW: %04X * %04X => %04X  CC: %s\n",
			mulr, muld, prod, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MULL)
{
	int64 mulr = OP0;
	int64 muld = OP1;
	int64 prod;

	prod = mulr * muld;
	LSTORE(OP2, OP3, prod);

	// Update condition codes
	CC_IIZZ_L((int32)prod);

	if ((int32)(prod >> 32) != (((int32)prod & LSIGN) ? -1 : 0)) {
		CC |= CC_V;
		if (PSW & PSW_IV)
			SET_TRAP(TRAP_INTOVF);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MULL: %08X * %08X => %08X  CC: %s\n",
			(int32)mulr, (int32)muld, (int32)prod, CC_DSPL(CC));
#endif /* DEBUG */
}

// ROTL  Rotate Long
//
// Opcode:
//   9C  ROTL  Rotate Long
//
// Operand Registers:
//   cnt.rb   OP0 = value of operand
//   src.rl   OP1 = value of operand
//   dst.wl   OP2 = register/memory flag
//            OP3 = memory address

DEF_INST(vax, ROTL)
{
	register int8   cnt = (OP0 & BMASK) % 32;
	register uint32 src = OP1;
	register int32  dst;

	// Do 32-bit rotate
//	if (cnt == 0) {
//		dst = src;
//	} else {
//		if (cnt < 0) {
//			// Rotate right
//			cnt = -cnt;
//			dst = (src >> cnt) | (src << (32 - cnt));
//		} else {
//			// Rotate left
//			dst = (src << cnt) | (src >> (32 - cnt));
//		}
//	}

	dst = cnt ? ((src << cnt) | (src >> (32 - cnt))) : src;

	// Write result back
	LSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_L(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ROTL: %08X %s %d (%d) => %08X  CC: %s\n",
			src, ((cnt < 0) ? ">>" : "<<"), abs(cnt)%32, abs(cnt), dst,
			CC_DSPL(CC));
#endif /* DEBUG */
}

// SBWC  Subtract With Carry
//
// Opcode:
//   D9  SBWC  Add With Carry
//
// Operand Registers:
//   sub.rl   OP0 = value of operand
//   dif.rl   OP1 = value of operand
//   dif.wl   OP2 = register/memory flag
//            OP3 = memory address

DEF_INST(vax, SBWC)
{
	register int32 sub = OP0;
	register int32 min = OP1;
	register int32 dif;
#ifdef DEBUG
	int32 carry = CC & CC_C;
#endif /* DEBUG */

	dif = min - sub - (CC & CC_C);
	LSTORE(OP2, OP3, dif);

	// Update condition codes
	CC_SUB_L(dif, sub, min);
	if ((sub == min) && dif)
		CC |= CC_C;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("SBWC: %08X - %08X - %d => %08X  CC: %s\n",
			min, sub, dif, carry, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

// SUB  Subtract
//
// Opcodes:
//
//   82  SUBB2  Subtract Byte 2 Operand
//   83  SUBB3  Subtract Byte 3 Operand
//   A2  SUBW2  Subtract Word 2 Operand
//   A3  SUBW3  Subtract Word 3 Operand
//   C2  SUBL2  Subtract Long 2 Operand
//   C3  SUBL3  Subtract Long 3 Operand
//
// Operand Registers:
//
//   2 Operand opcode
//     sub.rx   OP0 = value of operand
//     min.mx   OP1 = value of operand
//              OP2 = register/memory flag
//              OP3 = memory address
//
//   3 Operand opcode
//     sub.rx   OP0 = value of operand
//     min.rx   OP1 = value of operand
//     dif.wx   OP2 = register/memory flag
//              OP3 = memory address

DEF_INST(vax, SUBB)
{
	register int8 sub = OP0;
	register int8 min = OP1;
	register int8 dif;

	dif = min - sub;
	BSTORE(OP2, OP3, dif);

	// Update condition codes
	CC_SUB_B(dif, sub, min);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("SUBB: %02X - %02X => %02X  CC: %s\n",
			min, sub, dif, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

DEF_INST(vax, SUBW)
{
	register int16 sub = OP0;
	register int16 min = OP1;
	register int16 dif;

	dif = min - sub;
	WSTORE(OP2, OP3, dif);

	// Update condition codes
	CC_SUB_W(dif, sub, min);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("SUBW: %04X - %04X => %04X  CC: %s\n",
			min, sub, dif, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

DEF_INST(vax, SUBL)
{
	register int32 sub = OP0;
	register int32 min = OP1;
	register int32 dif;

	dif = min - sub;
	LSTORE(OP2, OP3, dif);

	// Update condition codes
	CC_SUB_L(dif, sub, min);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("SUBL: %08X - %08X => %08X  CC: %s\n",
			min, sub, dif, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

// **********************************************************
// ******************* XOR  Exclusive-OR ********************
// **********************************************************

DEF_INST(vax, XORB)
{
	register int8 mask = OP0;
	register int8 src  = OP1;
	register int8 dst;

	dst = src ^ mask;
	BSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_B(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("XORB: %02X XOR %02X => %02X  CC: %s\n",
			src, mask, dst, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

DEF_INST(vax, XORW)
{
	register int16 mask = OP0;
	register int16 src  = OP1;
	register int16 dst;

	dst = src ^ mask;
	WSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_W(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("XORW: %04X XOR %04X => %04X  CC: %s\n",
			src, mask, dst, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

DEF_INST(vax, XORL)
{
	register int32 mask = OP0;
	register int32 src  = OP1;
	register int32 dst;

	dst = src ^ mask;
	LSTORE(OP2, OP3, dst);

	// Update condition codes
	CC_IIZP_L(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("XORL: %08X XOR %08X => %08X  CC: %s\n",
			src, mask, dst, CC_DSPL(CC));
	}
#endif /* DEBUG */
}

