// cpu_convert.c - CVT Instruction Series
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

// 99 CVTBW - Convert Byte to Word
DEF_INST(vax, CVTBW)
{
	register int8  src = OP0;
	register int16 dst = src;

	WSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZZ_W(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTBW: Byte %02X => Word %04X  CC: %s\n",
			src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

// 98 CVTBL - Convert Byte to Long
DEF_INST(vax, CVTBL)
{
	register int8  src = OP0;
	register int32 dst = src;

	LSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZZ_L(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTBL: Byte %02X => Long %08X  CC: %s\n",
			src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

// 32 CVTWB - Convert Word to Byte
DEF_INST(vax, CVTWB)
{
	register int16 src = OP0;
	register int8  dst = src;

	BSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZZ_B(dst);
	if ((src > 127) || (src < -128))
		V_INTOVF;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTWB: Word %04X => Byte %02X  CC: %s\n",
			src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

// 32 CVTWL - Convert Word to Long
DEF_INST(vax, CVTWL)
{
	register int16 src = OP0;
	register int32 dst = src;

	LSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZZ_L(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTWL: Word %04X => Long %08X  CC: %s\n",
			src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

// F6 CVTLB - Convert Long to Byte
DEF_INST(vax, CVTLB)
{
	register int32 src = OP0;
	register int8  dst = src;

	BSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZZ_B(dst);
	if ((src > 127) || (src < -128))
		V_INTOVF;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTLB: Long %08X => Byte %02X  CC: %s\n",
			src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

// F7 CVTLW - Convert Long to Word
DEF_INST(vax, CVTLW)
{
	register int32 src = OP0;
	register int16 dst = src;

	WSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZZ_W(dst);
	if ((src > 32767) || (src < -32768))
		V_INTOVF;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTLW: Long %08X => Word %04X  CC: %s\n",
			src, dst, CC_DSPL(CC));
#endif /* DEBUG */
}
