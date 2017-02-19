// cpu_compare.c - Compare instructions.
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

// CMPB - Compare Byte
// CMPW - Compare Word
// CMPL - Compare Long

DEF_INST(vax, CMPB)
{
	register int8 src1 = OP0;
	register int8 src2 = OP1;

	// Update condition codes
	CC_CMP_B(src1, src2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CMPB: Compare %02X with %02X  CC: %s\n",
			src1, src2, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CMPW)
{
	register int16 src1 = OP0;
	register int16 src2 = OP1;

	// Update condition codes
	CC_CMP_W(src1, src2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CMPW: Compare %04X with %04X  CC: %s\n",
			src1, src2, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CMPL)
{
	register int32 src1 = OP0;
	register int32 src2 = OP1;

	// Update condition codes
	CC_CMP_L(src1, src2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CMPL: Compare %08X with %08X  CC: %s\n",
			src1, src2, CC_DSPL(CC));
#endif /* DEBUG */
}


// TSTB - Test Byte
// TSTW - Test Word
// TSTL - Test Long

DEF_INST(vax, TSTB)
{
	register int8 src = OP0;

	// Update condition codes
	CC_IIZZ_B(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TSTB: Test %02X  CC: %s\n", src, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, TSTW)
{
	register int16 src = OP0;

	// Update condition codes
	CC_IIZZ_W(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TSTW: Test %04X  CC: %s\n", src, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, TSTL)
{
	register int32 src = OP0;

	// Update condition codes
	CC_IIZZ_L(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TSTL: Test %08X  CC: %s\n", src, CC_DSPL(CC));
#endif /* DEBUG */
}
