// cpu_move.c - VAX Move Instructions.
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

// MCOMB - Move Complemented Byte
// MCOMW - Move Complemented Word
// MCOML - Move COmplemented Long

DEF_INST(vax, MCOMB)
{
	register int8 src = OP0;
	register int8 dst;

	dst = ~src;
	BSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZP_B(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MCOMB: %02X -> %02X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MCOMW)
{
	register int16 src = OP0;
	register int16 dst;

	dst = ~src;
	WSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZP_W(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MCOMW: %04X -> %04X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MCOML)
{
	register int32 src = OP0;
	register int32 dst;

	dst = ~src;
	LSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZP_L(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MCOML: %08X -> %08X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}


// MNEGB - Move Negated Byte
// MNEGW - Move Negated Word
// MNEGL - Move Negated Long

DEF_INST(vax, MNEGB)
{
	register int8 src = OP0;
	register int8 dst;

	dst = -src;
	BSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_SUB_B(dst, src, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MNEGB: %02X -> %02X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MNEGW)
{
	register int16 src = OP0;
	register int16 dst;

	dst = -src;
	WSTORE(OP1, OP2, dst);

	CC_SUB_W(dst, src, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MNEGW: %04X -> %04X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MNEGL)
{
	register int32 src = OP0;
	register int32 dst;

	dst = -src;
	LSTORE(OP1, OP2, dst);

	CC_SUB_L(dst, src, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MNEGL: %08X -> %08X  CC: %s\n", src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

// Operand Registers:
//
//   For MOV[BWL] instruction:
//     src.rx: OP0 = value of operand
//     dst.wx: OP1 = memory/register flag
//             OP2 = memory address
//
//   For MOVQ instruction:
//     src.rq: OP0 = value of operand (left half)
//             OP1 = value of operand (right half)
//     dst.wx: OP2 = memory/register flag
//             OP3 = memory address
//
//   For MOVA[BWL] instruction:
//     src.ax: OP0 = address of operand
//     dst.wx: OP1 = memory/register flag
//             OP2 = memory address

DEF_INST(vax, MOVB)
{
	register int8 src = OP0;

	BSTORE(OP1, OP2, src);

	// Update condition codes
	CC_IIZP_B(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVB: Move %02X  CC: %s\n", src, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MOVW)
{
	register int16 src = OP0;

	WSTORE(OP1, OP2, src);

	// Update condition codes
	CC_IIZP_W(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVW: %04X  CC: %s\n", src, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MOVL)
{
	register int32 src = OP0;

	LSTORE(OP1, OP2, src);

	// Update condition codes
	CC_IIZP_L(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVL: Move %08X  CC: %s\n", src, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MOVQ)
{
	register int32 src1 = OP0;
	register int32 src2 = OP1;

	QSTORE(OP2, OP3, src1, src2);

	// Update condition codes
	CC_IIZP_Q(src1, src2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVQ: Move %08X %08X  CC: %s\n", src1, src2, CC_DSPL(CC));
#endif /* DEBUG */
}


// MOVZBW - 
// MOVZBL -
// MOVZWL -

DEF_INST(vax, MOVZBW)
{
	register int16 src = ZXTB(OP0);

	WSTORE(OP1, OP2, src);

	// Update condition codes
	CC_IIZP_W(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVZBW: Extend %02X to %04X  CC: %s\n",
			src, src, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MOVZBL)
{
	register int32 src = ZXTB(OP0);

	LSTORE(OP1, OP2, src);

	// Update condition codes
	CC_IIZP_L(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVZBL: Extend %02X to %08X\n",
			src, src, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MOVZWL)
{
	register int32 src = ZXTW(OP0);

	LSTORE(OP1, OP2, src);

	// Update condition codes
	CC_IIZP_L(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVZWL: Extend %04X to %08X  CC: %s\n",
			src, src, CC_DSPL(CC));
#endif /* DEBUG */
}
