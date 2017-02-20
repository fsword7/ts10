// cpu_eis.c - Extended Instruction Set
//
// Copyright (c) 2002, Timothy M. Stark
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

// ASH  - Arithmetic Shift
// ASHC - Arithmetic Shift Combined
// MUL  - Multiply
// DIV  - Divide

#include "pdp11/defs.h"

INSDEF(p11, ASH)
{
	uint16 srcSpec   = (IR >> 6) & 7;
	uint16 dstSpec   = IR & 077;
	int32  sc        = FetchW(dstSpec) & 077;
	int32  dst, src  = REGW(srcSpec);
	
	CC = 0;
	if (sc == 0) {
		dst = src;
	} else if (sc <= 31) {
		// Shift Left
		dst = src << sc;
		if ((dst >> 16) & 1)
			CC |= CC_C;
		if ((uint16)(dst >> 16) != (uint16)((int16)dst >> 15))
			CC |= CC_V;
	} else if (sc == 32) {
		dst = (src < 0) ? -1 : 0;
		if (src < 0) CC |= CC_C;
	} else {
		// Shift Right (Negative Shift Count)
		dst = src >> (63 - sc);
		if (dst & 1) CC |= CC_C;
		dst >>= 1;
	}
	
	// Update condition codes for N and Z bits.
	if ((int16)dst < 0)  CC |= CC_N;
	if ((int16)dst == 0) CC |= CC_Z;

	// Write results back.
	REGW(srcSpec) = dst;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ASH) %06o %s %d => %06o : %s\n",
			p11->Unit.devName, (uint16)src, (sc < 32) ? "<<" : ">>",
		  	(sc < 32) ? sc : (64 - sc), (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, ASHC)
{
	uint16 srcSpec   = (IR >> 6) & 7;
	uint16 dstSpec   = IR & 077;
	int32  sc        = FetchW(dstSpec) & 077;
	int32  dst, src  = (UREGW(srcSpec) << 16) | UREGW(srcSpec | 1);
	int32  cv; // Carry/Overflow Test
	
	CC = 0;
	if (sc == 0) {
		dst = src;
	} else if (sc <= 31) {
		// Shift Left
		dst = ((uint32)src) << sc;
		cv  = src >> (32 - sc);
		if (cv != ((dst < 0) ? -1 : 0))
			CC |= CC_V;
		if (cv & 1) CC |= CC_C;
//	} else if (sc == 32) {
//		dst = (src < 0) ? -1 : 0;
//		if (src < 0) CC |= CC_C;
	} else {
		// Shift Right (Negative Shift Count)
		dst = src >> (63 - sc);
		if (dst & 1) CC |= CC_C;
		dst >>= 1;
	}
	
	// Update condition codes for N and Z bits.
	if (dst < 0)  CC |= CC_N;
	if (dst == 0) CC |= CC_Z;

	// Write results back.
	REGW(srcSpec)     = dst >> 16;
	REGW(srcSpec | 1) = dst;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (ASHC) %06o %06o %s %d => %06o %06o : %s\n",
			p11->Unit.devName, (uint16)(src >> 16), (uint16)src,
			(sc < 32) ? "<<" : ">>", (sc < 32) ? sc : (64 - sc),
			(uint16)(dst >> 16), (uint16)dst, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, MUL)
{
	uint16 srcSpec = (IR >> 6) & 7;
	uint16 dstSpec = IR & 077;
	int32  mpr     = FetchW(dstSpec);
	int32  mpc     = REGW(srcSpec);
	int32  prod;

	// Do 32-bit multiply operation.
	prod = mpc * mpr;

	// Update conition codes
	CC_IIZZ_I(prod);
	if ((prod < MAX_NEG) || (prod > MAX_POS))
		CC |= CC_C;

	// Write results back
	UREGW(srcSpec)     = prod >> 16;
	UREGW(srcSpec | 1) = prod;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (MUL) %06o * %06o => %06o %06o : %s\n",
			p11->Unit.devName, (uint16)mpc, (uint16)mpr,
			(uint16)(prod >> 16), (uint16)prod, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}

INSDEF(p11, DIV)
{
	uint16 srcSpec = (IR >> 6) & 7;
	uint16 dstSpec = IR & 077;
	int32  dvr     = FetchW(dstSpec);
	int32  dvd     = (UREGW(srcSpec) << 16) | UREGW(srcSpec | 1);
	int32  quo, rem;

	if (dvr == 0) {
		quo = rem = 0;
		CC  = (CC_Z|CC_V|CC_C);
	} else if (abs(dvd >> 16) > abs(dvr)) {
		quo = rem = 0;
		CC  = CC_V | (((dvd < 0) ^ (dvr < 0)) ? CC_N : 0);
	} else {
		// Do 32-bit divide operation.
		quo = dvd / dvr;
		rem = dvd % dvr;

		// Update condition codes for N, Z, and V bits
		// If V bit is set, do not write them back.
		if ((quo > MAX_POS) || (quo < MAX_NEG)) {
			CC  = CC_V | ((quo < 0) ? CC_N : 0);
			quo = rem = 0;
		} else {
			CC_IIZZ_I(quo);
			// Write results back
			UREGW(srcSpec)     = quo;
			UREGW(srcSpec | 1) = rem;
		}
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%s: (DIV) %06o %06o / %06o => %06o R %06o : %s\n",
			p11->Unit.devName, (uint16)(dvd >> 16), (uint16)dvd,
			(uint16)dvr, (uint16)quo, (uint16)rem, p11_GetCC(CC, NULL));
#endif /* DEBUG */
}
