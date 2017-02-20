// cpu_field.c - Variable-length Bit Field Instructions
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

uint32 vax_ByteMask[33] = {
	0x00000000,
	0x00000001, 0x00000003, 0x00000007, 0x0000000F,
	0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
	0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
	0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
	0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
	0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
	0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
};

uint32 vax_ByteSign[33] = {
	0x00000000,
	0x00000001, 0x00000002, 0x00000004, 0x00000008,
	0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x00000100, 0x00000200, 0x00000400, 0x00000800,
	0x00001000, 0x00002000, 0x00004000, 0x00008000,
	0x00010000, 0x00020000, 0x00040000, 0x00080000,
	0x00100000, 0x00200000, 0x00400000, 0x00800000,
	0x01000000, 0x02000000, 0x04000000, 0x08000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000
};

// Operand Registers:
//   pos.rl   OP0 = value of operand
//   size.rb  OP1 = value of operand
//   base.vb  OP2 = register/memory flag
//            OP3 = content/memory address

int32 vax_GetField(register VAX_CPU *vax, int32 *opnd, int32 sign)
{
	int32  Pos   = opnd[0];
	uint8  Size  = opnd[1];
	int32  nReg  = opnd[2];
	uint32 Word1 = opnd[3];
	uint32 Word2, Base;

	// If size is zero, do nothing but return zero.
	if (Size == 0)
		return 0;

	// If size is more than 32,
	// initiate reserved operand fault.
	if (Size > 32)
		RSVD_OPND_FAULT;

	// Extract a field from one or two longwords.
	if (nReg >= 0) {
		if ((uint32)Pos > 31)
			RSVD_OPND_FAULT;
		if (Pos)
			Word1 = (Word1 >> Pos) | (RN1(nReg) << (32 - Pos));
	} else {
		Base = Word1 + (Pos >> 3);
		Pos = (Pos & 0x07) | ((Base & 0x03) << 3);
		Base &= ~0x03;
		Word1 = ReadV(Base, OP_LONG, RA);
		if ((Pos + Size) > 32)
			Word2 = ReadV(Base+4, OP_LONG, RA);
		if (Pos)
			Word1 = (Word1 >> Pos) | (Word2 << (32 - Pos));
	}

	// Zero or Sign Extension
	Word1  &= vax_ByteMask[Size];
	if (sign && (Word1 & vax_ByteSign[Size]))
		Word1 |= ~vax_ByteMask[Size];

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("FIELD: Pos=%d  Size=%d  Data=%08X\n", Pos, Size, Word1); 
#endif /* DEBUG */

	return Word1;
}

// Operand Registers:
//   pos.rl   OP0 = value of operand
//   size.rb  OP1 = value of operand
//   base.vb  OP2 = register/memory flag
//            OP3 = content/memory address
//   src.rl   OP4 = value of operand

DEF_INST(vax, CMPV)
{
	register int32 tmp = vax_GetField(vax, &OP0, 1);
	register int32 src = OP4;

	// Update condition codes
	CC_CMP_L(tmp, src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CMPV: Compare %08X with %08X  CC: %s\n",
			tmp, src, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CMPZV)
{
	register int32 tmp = vax_GetField(vax, &OP0, 0);
	register int32 src = OP4;

	// Update condition codes
	CC_CMP_L(tmp, src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CMPV: Compare %08X with %08X  CC: %s\n",
			tmp, src, CC_DSPL(CC));
#endif /* DEBUG */
}

// Operand Registers:
//   pos.rl   OP0 = value of operand
//   size.rb  OP1 = value of operand
//   base.vb  OP2 = register/memory flag
//            OP3 = content/memory address
//   src.wl   OP4 = register/memory flag
//            OP5 = content/memory address

DEF_INST(vax, EXTV)
{
	register int32 dst = vax_GetField(vax, &OP0, 1);

	LSTORE(OP4, OP5, dst);

	// Update condition codes
	CC_IIZP_L(dst);
}

DEF_INST(vax, EXTZV)
{
	register int32 dst = vax_GetField(vax, &OP0, 0);

	LSTORE(OP4, OP5, dst);

	// Update condition codes
	CC_IIZP_L(dst);
}

DEF_INST(vax, FFC)
{
	uint32  startpos = OP0;
	uint8   size     = OP1;
	uint32  idx, findpos, src = 0;
	uint32  tmp;

	if (size > 0) {
		tmp = src = vax_GetField(vax, &OP0, 0) ^ vax_ByteMask[size];
		for (idx = 0; idx < size; idx++) {
			if (tmp & 1) break;
			tmp >>= 1;
		}
		findpos = startpos + idx;
	} else
		findpos = startpos;

	LSTORE(OP4, OP5, findpos);

	// Update condition codes
	CC = (src ? 0 : CC_Z);
}

DEF_INST(vax, FFS)
{
	uint32  startpos = OP0;
	uint8   size     = OP1;
	uint32  idx, findpos, src = 0;
	uint32  tmp;

	if (size > 0) {
		tmp = src = vax_GetField(vax, &OP0, 0);
		for (idx = 0; idx < size; idx++) {
			if (tmp & 1) break;
			tmp >>= 1;
		}
		findpos = startpos + idx;
	} else
		findpos = startpos;

	LSTORE(OP4, OP5, findpos);

	// Update condition codes
	CC = (src ? 0 : CC_Z);
}

DEF_INST(vax, INSV)
{
	uint32 src  = OP0;
	int32  pos  = OP1;
	uint8  size = OP2;
	int32  nReg = OP3;
	uint32 base, word1, word2;
	uint32 mask;

	if (size > 0) {
		if (size > 32)
			RSVD_OPND_FAULT;
		if (nReg >= 0) {
			if ((uint32)pos > 31)
				RSVD_OPND_FAULT;
			if ((pos + size) > 32) {
				if (nReg >= nSP)
					RSVD_OPND_FAULT;
				mask = vax_ByteMask[pos + size - 32];
				RN1(nReg) = (RN1(nReg) & ~mask) | ((src >> (32 - pos)) & mask);
			}
			mask = vax_ByteMask[size] << pos;
			RN0(nReg) = (RN0(nReg) & ~mask) | ((src << pos) & mask);
		} else {
			base = OP4 + (pos >> 3);
			pos  = (pos & 7) | ((base & 3) << 3);
			base &= ~3;
			word1 = ReadV(base, OP_LONG, RA);
			if ((pos + size) > 32) {
				word2 = ReadV(base + OP_LONG, OP_LONG, RA);
				mask = vax_ByteMask[pos + size - 32];
				word2 = (word2 & ~mask) | ((src >> (32 - pos)) & mask);
				WriteV(base + OP_LONG, word2, OP_LONG, WA);
			}
			mask = vax_ByteMask[size] << pos;
			word1 = (word1 & ~mask) | ((src << pos) & mask);
			WriteV(base, word1, OP_LONG, WA);
		}
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("INSV: Pos=%d  Size=%d  Data=%08X\n", pos, size, src); 
#endif /* DEBUG */
}
