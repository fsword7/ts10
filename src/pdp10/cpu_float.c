// cpu_float.c - floating point instructions for all processors
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
// See README for copyright notice.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


// Instructions handled for all processors:
//
//   FADR   Floating Add and Round
//   FSBR   Floating Subtract and Round
//   FMPR   Floating Multiply and Round
//   FDVR   Floating Divide and Round
//   FAD    Floating Add
//   FSB    Floating Subtract
//   FMP    Floating Multiply
//   FDV    Floating Divide
//   FIX    Fix
//   FIXR   Fix and Round
//   FLTR   Float and Round
//   FSC    Floating Scale
//   
//
// Instructions handled for KI10, KL10, and KS10 processors:
//
//   DFAD   Double Floating Add
//   DFSB   Double Floating Subtract
//   DFMP   Double Floating Multiply
//   DFDV   Double Floating Divide
//
// Instructions handled for KL10 processor with UC v271 or later:
//
//   GFAD   G-Format Floating Add
//   GFSB   G-Format Floating Subtract
//   GFMP   G-Format Floating Multiply
//   GFDV   G-Format Floating Subtract
//   GFIX   G-Format Fix
//   GFIXR  G-Format Fix and Round
//   GFLTR  G-Format Float and Round
//   GDFIX  G-Format Double Fix
//   GDFIXR G-Format Double Fix and Round
//   DGFLTR Double G-Format Float and Round
//   GSNGL  G-Format to Single Precision
//   GDBLE  Single Precision to G-Format
//   GFSC   G-Format Floating Scale
//
//   DFN    Double Floating Negate
//   UFA    Unnormalized Floating Add
//   FADL   Floating Add Long
//   FSBL   FLoating Subtract Long
//   FMPL   Floating Multiply Long
//   FDVL   Floating Divide Long


// Single Precision Floating Point Word
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |S|    Exponent   |                    Fraction                         |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5

// Double Precision Floating Point Word
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |S|    Exponent   |           High-Order Fraction                       |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
// |0|                        Low-Order Fraction                           |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5

#include <pdp10/defs.h>
#include <pdp10/ks10.h>
#include <pdp10/proto.h>

#define SFP_DIFF    31
#define SFP_LEFT    4

#define SFP_FRAC    0x7FFFFFF0  // Fraction Field
#define SFP_SIGN    0x80000000  // Sign Bit
#define SFP_CARRY   0x80000000  // Carry Bit
#define SFP_NORM    0x40000000  // Normalization Bit
#define SFP_NORM64  0x20000000000000 // For FMP instruction series.
#define SFP_ROUND   0x00000008  // Round/Guard Bit
#define SFP_P_NORM  31          // Position of normalized bit

#define SFP_N_SIGN  1           // Number of sign bits
#define SFP_N_EXP   8           // Number of exponent bits
#define SFP_N_FRAC  27          // Number of fraction bits

#define UFP_ONES     0xFFFFFFFFFFFFFFFF
#define UFP_FRAC     0x7FFFFFFFFFFFFFFE
#define UFP_HIFRAC   0x7FFFFFF000000000
#define UFP_LOFRAC   0x0000000FFFFFFFFE
#define UFP_SIGN     0x8000000000000000
#define UFP_CARRY    0x8000000000000000
#define UFP_NORM     0x4000000000000000
#define UFP_SROUND   0x0000000800000000
#define UFP_DROUND   0x0000000000000001
#define UFP_MASK32   0xFFFFFFFF
#define UFP_P_LOFRAC 1
#define UFP_P_HIFRAC 36
#define UFP_P_NORM   63
#define UFP_DIFF64   63
#define UFP_DIFF128  127
#define UFP_SPM      (UFP_P_HIFRAC - (32 - SFP_N_FRAC - 1))

// Single Precision Floating Number
#define SFP_M_EXP   0377        // Exponent Mask
#define SFP_P_EXP   27          // Position of Exponent (35 - 8)
#define SFP_M_FRAC  0777777777  // Fraction Mask
#define SFP_BIAS    0200
#define SFP_EXP_MAX 0377        // Maximum number for exponent
#define SFP_EXP_MIN 0000        // Minimum number for exponent

// Double precision floating number
#define DFP_M_HIFRAC 0000777777777LL // High Fraction Mask
#define DFP_M_LOFRAC 0377777777777LL // Low Fraction Mask
#define DFP_M_EXP    0377            // Exponent Mask
#define DFP_P_EXP    27              // Position of Exponent (35 - 8)
#define DFP_BIAS     0200
#define DFP_EXP_MAX  0377            // Maximum number for exponent
#define DFP_EXP_MIN  0000            // Minimum number for exponent

#define FP_M_SIGN  0400000000000LL // Sign bit field
#define FP_M_EXP   0377000000000LL // Exponent field
#define FP_M_HFRAC 0000777777777LL // High Fraction Field
#define FP_M_LFRAC 0377777777777LL // Low Fraction Field
#define FP_N_HFRAC 27
#define FP_N_LFRAC 35

#define FP_FIT27    0777000000000LL // 27-Bit Fits?

#define FP_SGN_FRAC TRUE
#define FP_ABS_FRAC FALSE

// Unpacked floating number
typedef struct {
	int32  Sign;  // Sign:     1 = Negative, 0 = Positive
	int32  Exp;   // Exponent: 000 to 377 (-127 to 128).
	uint64 hFrac; // high left-justified fraction
	uint64 lFrac; // low left-justified fraction
} UFP;

static int36 *pData;

// Unpacking a single-precision floating number.
inline void p10_sfpUnpack(int36 fpWord, UFP *fp, int sign)
{
	fp->Sign  = S36(fpWord) ? 1 : 0;
	fp->Exp   = ((int32)(fpWord >> SFP_P_EXP)) & SFP_M_EXP;
	fp->hFrac = (fpWord & SFP_M_FRAC) << UFP_P_HIFRAC;
	if (fp->Sign) {
		fp->Exp ^= SFP_M_EXP;
		if (sign)
			fp->hFrac |= UFP_SIGN;
		else {
			if (fp->hFrac)
				fp->hFrac = (-fp->hFrac) & UFP_HIFRAC;
			else {
				fp->Exp++;
				fp->hFrac = UFP_NORM;
			}
		}
	}
}
 
// Packing a single-precision floating number.
inline int36 p10_sfpPack(UFP *fp, int fdvneg)
{
	int36 fpWord;

	// Check exponent for out of range (0 - 377).
	// If so, set system flags.
	if (fp->Exp > SFP_EXP_MAX)
		FLAGS |= (FLG_TRAP1 | FLG_AROV | FLG_FOV);
	else if (fp->Exp < SFP_EXP_MIN)
		FLAGS |= (FLG_TRAP1 | FLG_AROV | FLG_FOV | FLG_FXU);
	
	// Packing floating number.
	fpWord = (fp->hFrac >> UFP_P_HIFRAC) & SFP_M_FRAC;
	fpWord |= ((int36)(fp->Exp & SFP_M_EXP)) << SFP_P_EXP;
	if (fp->Sign)
		fpWord = -fpWord - fdvneg;

	return fpWord;
}

// Normalize a single-precision floating number.
inline void p10_sfpNorm(UFP *fp, int rnd)
{
	// If fraction is zero, clear all and return.
	if (fp->hFrac == 0) {
		fp->Sign = 0;
		fp->Exp  = 0;
		return;
	}

	// Normalize fraction by shift first bit to normalization bit.
	while ((fp->hFrac & UFP_NORM) == 0) {
		fp->hFrac <<= 1;
		fp->Exp--;
	}

	// Optionally round floating number.
	if (rnd) {
		fp->hFrac += UFP_SROUND;
		if (fp->hFrac & UFP_CARRY) {
			fp->hFrac >>= 1;
			fp->Exp++;
		}
	}
}

// Unpacking a double-precision floating number.
inline
void p10_dfpUnpack(int36 fpWord1, int36 fpWord2, UFP *fp, int sign)
{
	fp->Sign  = S36(fpWord1) ? 1 : 0;
	fp->Exp   = ((int32)(fpWord1 >> DFP_P_EXP)) & DFP_M_EXP;
	fp->hFrac = ((fpWord1 & DFP_M_HIFRAC) << UFP_P_HIFRAC) |
	            ((fpWord2 & DFP_M_LOFRAC) << UFP_P_LOFRAC);
	fp->lFrac = 0;

	if (fp->Sign) {
		fp->Exp ^= DFP_M_EXP;
		if (sign)
			fp->hFrac |= UFP_SIGN;
		else {
			if (fp->hFrac)
				fp->hFrac = (-fp->hFrac) & UFP_FRAC;
			else {
				fp->Exp++;
				fp->hFrac = UFP_NORM;
			}
		}
	}
}

// Packing a double-precision floating number.
inline void p10_dfpPack(UFP *fp, int36 *fpWord1, int36 *fpWord2)
{
	// Check exponent for out of range (0 - 377).
	// If so, set system flags.
	if (fp->Exp > DFP_EXP_MAX)
		FLAGS |= (FLG_TRAP1 | FLG_AROV | FLG_FOV);
	else if (fp->Exp < DFP_EXP_MIN)
		FLAGS |= (FLG_TRAP1 | FLG_AROV | FLG_FOV | FLG_FXU);

	// Packing floating number.
	*fpWord1 = (fp->hFrac >> UFP_P_HIFRAC) & DFP_M_HIFRAC;
	*fpWord2 = (fp->hFrac >> UFP_P_LOFRAC) & DFP_M_LOFRAC;
	*fpWord1 |= ((int36)(fp->Exp & DFP_M_EXP)) << DFP_P_EXP;
	if (fp->Sign) {
		*fpWord2 = -*fpWord2 & WORD36_MAXP;
		*fpWord1 = ~*fpWord1 + (*fpWord2 == 0);
	}
}

// Normalize a double-precision floating number.
inline void p10_dfpNorm64(UFP *fp, int rnd)
{
	// If fraction is zero, clear all and return.
	if (fp->hFrac == 0) {
		fp->Sign = 0;
		fp->Exp = 0;
		return;
	}

	// Normalize fraction by shift first bit to normalization bit.
	while ((fp->hFrac & UFP_NORM) == 0) {
		fp->hFrac <<= 1;
		fp->Exp--;
	}

	// Optionally round floating number.
	if (rnd) {
		fp->hFrac += UFP_DROUND;
		if (fp->hFrac & UFP_CARRY) {
			fp->hFrac >>= 1;
			fp->Exp++;
		}
	}
}

// Normalize a double-precision floating number.
inline void p10_dfpNorm128(UFP *fp, int rnd)
{
	// If fraction is zero, clear all and return.
	if ((fp->hFrac | fp->lFrac) == 0) {
		fp->Sign = 0;
		fp->Exp  = 0;
		return;
	}

	// Normalize fraction by shift first bit to normalization bit.
	while ((fp->hFrac & UFP_NORM) == 0) {
		fp->hFrac = (fp->hFrac << 1) | (fp->lFrac >> 63);
		fp->lFrac <<= 1;
		fp->Exp--;
	}

	// Optionally round floating number.
	if (rnd) {
		fp->hFrac += UFP_DROUND;
		if (fp->hFrac & UFP_CARRY) {
			fp->hFrac >>= 1;
			fp->Exp++;
		}
	}
}

//***********************************************************
//************ Single Precision Floating Math ***************
//***********************************************************

inline int36 p10_sfpAdd(int36 ac, int36 e, int rnd)
{
	UFP fpAC, fpE;
	int diff;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("FAD%s: %c%012llo + %c%012llo = ", (rnd ? "R" : ""),
			((ac < 0) ? '-' : '+'), ac & WORD36_ONES,
			((e < 0)  ? '-' : '+'), e & WORD36_ONES);
#endif /* DEBUG */

	if (ac == 0)
		p10_sfpUnpack(e, &fpAC, FP_ABS_FRAC);
	else if (e == 0)
		p10_sfpUnpack(ac, &fpAC, FP_ABS_FRAC);
	else {
		// Now unpack floating numbers in order to do math.
		p10_sfpUnpack(ac, &fpAC, FP_SGN_FRAC);
		p10_sfpUnpack(e, &fpE, FP_SGN_FRAC);

		// Align exponent
		if (fpAC.Exp < fpE.Exp) {
			diff = -(fpAC.Exp - fpE.Exp);
			fpAC.Exp += diff;
			fpAC.hFrac = (int64)fpAC.hFrac 
				>> ((diff > UFP_DIFF64) ? UFP_DIFF64 : diff);
		} else {
			diff = fpAC.Exp - fpE.Exp;
			fpE.Exp += diff;
			fpE.hFrac = (int64)fpE.hFrac
				>> ((diff > UFP_DIFF64) ? UFP_DIFF64 : diff);
		}

		// Do floating add math
		fpAC.hFrac += fpE.hFrac;
		if (fpAC.Sign ^ fpE.Sign) {
			if (fpAC.hFrac & UFP_SIGN) {
				fpAC.hFrac = -fpAC.hFrac;
				fpAC.Sign = 1;
			} else
				fpAC.Sign = 0;
		} else {
			if (fpAC.Sign)
				fpAC.hFrac = -fpAC.hFrac;
			if (fpAC.hFrac & UFP_CARRY) {
				fpAC.hFrac >>= 1;
				fpAC.Exp++;
			}
		}
	}

	p10_sfpNorm(&fpAC, rnd);
	ac = p10_sfpPack(&fpAC, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo\n",
			((ac < 0) ? '-' : '+'), ac & WORD36_ONES);
#endif /* DEBUG */

	return ac;
}

inline int36 p10_sfpSubtract(int36 ac, int36 e, int rnd)
{
	UFP fpAC, fpE;
	int diff;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("FSB%s: %c%012llo - %c%012llo = ", (rnd ? "R" : ""),
			((ac < 0) ? '-' : '+'), ac & WORD36_ONES,
			((e < 0)  ? '-' : '+'), e & WORD36_ONES);
#endif /* DEBUG */

	e = NEG(e);
	if (ac == 0)
		p10_sfpUnpack(e, &fpAC, FP_ABS_FRAC);
	else if (e == 0)
		p10_sfpUnpack(ac, &fpAC, FP_ABS_FRAC);
	else {
		// Now unpack floating numbers in order to do math.
		p10_sfpUnpack(ac, &fpAC, FP_SGN_FRAC);
		p10_sfpUnpack(e, &fpE, FP_SGN_FRAC);

		// Align exponent
		if (fpAC.Exp < fpE.Exp) {
			diff = -(fpAC.Exp - fpE.Exp);
			fpAC.Exp += diff;
			fpAC.hFrac = (int64)fpAC.hFrac
				>> ((diff > UFP_DIFF64) ? UFP_DIFF64 : diff);
		} else {
			diff = fpAC.Exp - fpE.Exp;
			fpE.Exp += diff;
			fpE.hFrac = (int64)fpE.hFrac
				>> ((diff > UFP_DIFF64) ? UFP_DIFF64 : diff);
		}

		// Do floating add math
		fpAC.hFrac += fpE.hFrac;
		if (fpAC.Sign ^ fpE.Sign) {
			if (fpAC.hFrac & UFP_SIGN) {
				fpAC.hFrac = -fpAC.hFrac;
				fpAC.Sign = 1;
			} else
				fpAC.Sign = 0;
		} else {
			if (fpAC.Sign)
				fpAC.hFrac = -fpAC.hFrac;
			if (fpAC.hFrac & UFP_CARRY) {
				fpAC.hFrac >>= 1;
				fpAC.Exp++;
			}
		}
	}

	p10_sfpNorm(&fpAC, rnd);
	ac = p10_sfpPack(&fpAC, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo\n",
			((ac < 0) ? '-' : '+'), ac & WORD36_ONES);
#endif /* DEBUG */

	return ac;
}

inline int36 p10_sfpMultiply(int36 ac, int36 e, int rnd)
{
	UFP    fpAC, fpE;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("FMP%s: %c%012llo * %c%012llo = ", (rnd ? "R" : ""),
			((ac < 0) ? '-' : '+'), ac & WORD36_ONES,
			((e < 0)  ? '-' : '+'), e & WORD36_ONES);
#endif /* DEBUG */

	// Get unpacked floating point numbers.
	p10_sfpUnpack(ac, &fpAC, FP_ABS_FRAC);
	p10_sfpUnpack(e, &fpE, FP_ABS_FRAC);

	if ((fpAC.hFrac != 0) && (fpE.hFrac != 0)) {
		// Do floating multiply math

		fpAC.Sign  = fpAC.Sign ^ fpE.Sign;
		fpAC.Exp   = fpAC.Exp + fpE.Exp - SFP_BIAS + 1;
		fpAC.hFrac = (fpAC.hFrac >> UFP_SPM) * (fpE.hFrac >> UFP_SPM);

		// Normalize and optionally round result and return it.
		p10_sfpNorm(&fpAC, rnd);
		ac = p10_sfpPack(&fpAC, 0);
	} else
		ac = 0;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo\n",
			((ac < 0) ? '-' : '+'), ac & WORD36_ONES);
#endif /* DEBUG */

	return ac;
}

inline int36 p10_sfpDivide(int36 ac, int36 e, int rnd)
{
	UFP    fpAC, fpE;
	uint64 quo, dvd, dvr;
	int    rem; // Remainder
	int    count;

	// Get unpacked floating point numbers.
	p10_sfpUnpack(ac, &fpAC, FP_ABS_FRAC);
	p10_sfpUnpack(e, &fpE, FP_ABS_FRAC);

	if (fpAC.hFrac >= (fpE.hFrac << 1)) {
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("FDV%s: *** FP Error: %s\n", (rnd ? "R" : ""),
				(fpE.hFrac == 0) ? "Divide By Zero" : "Overflow");
			dbg_Printf("FDV%s: AC=%c%06llo,,%06llo  E=%c%06llo,,%06llo\n",
				(rnd ? "R" : ""),
				((ac < 0) ? '-' : '+'), LHSR(ac), RH(ac),
				((e < 0) ? '-' : '+'), LHSR(e), RH(e));
		}
#endif /* DEBUG */
		FLAGS |= (FLG_TRAP1 | FLG_AROV | FLG_FOV | FLG_DCX);
		emu_Abort(p10_SetJump, P10_ABORT);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("FDV%s: %c%012llo / %c%012llo = ", (rnd ? "R" : ""),
			((ac < 0) ? '-' : '+'), ac & WORD36_ONES,
			((e < 0)  ? '-' : '+'), e & WORD36_ONES);
#endif /* DEBUG */

	// Do single-precision floating divide math
	if (fpAC.hFrac) {
		fpAC.Sign  = fpAC.Sign ^ fpE.Sign;
		fpAC.Exp   = fpAC.Exp - fpE.Exp + SFP_BIAS + 1;

		// ACTION: This alogrithm need more working for optimization.
		// I tried to implement optimizing divide math but it did not
		// work so well.  I had to implement step-by-step divide math
		// (like do divide math by using your hands) at this time.

		quo = 0;
		dvd = fpAC.hFrac;
		dvr = fpE.hFrac;
		if (dvd < dvr) {
			dvd <<= 1;
			fpAC.Exp--;
		}
		count = UFP_P_NORM;
		while (count--) {
			quo <<= 1;
			if (dvd >= dvr) {
				dvd -= dvr;
				quo++;
			}
			dvd <<= 1;
		}
		fpAC.hFrac = quo;
		rem = (fpAC.Sign && (dvd != 0));
	}

	// Normalize and optionally round result and return it.
	p10_sfpNorm(&fpAC, rnd);
	ac = p10_sfpPack(&fpAC, rem);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo\n",
			((ac < 0) ? '-' : '+'), ac & WORD36_ONES);
#endif /* DEBUG */

	return ac;
}

inline void p10_sfpScale(int36 *ac, int36 e)
{
	int sc = LIT8(e);
	UFP fpAC;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("FSC: %c%012llo S %c%012llo = ",
			((*ac < 0) ? '-' : '+'), *ac & WORD36_ONES,
			((e < 0)  ? '-' : '+'), e & WORD36_ONES);
#endif /* DEBUG */

	if (*ac) {
		p10_sfpUnpack(*ac, &fpAC, FP_ABS_FRAC);

		fpAC.Exp += sc;

		p10_sfpNorm(&fpAC, FALSE);
		*ac = p10_sfpPack(&fpAC, 0);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo\n",
			((*ac < 0) ? '-' : '+'), *ac & WORD36_ONES);
#endif /* DEBUG */
}

// Convert floating to fixed
inline void p10_sfpFix(int36 *ac, int36 e, int rnd)
{
	UFP    fpAC;
	int32  sc;
	uint64 so;

	p10_sfpUnpack(e, &fpAC, FP_ABS_FRAC);

	if (fpAC.Exp > (SFP_BIAS + 35)) {
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("FIX%s: ** Expoent = %d - exceeding 35.\n",
				(rnd ? "R" : ""), fpAC.Exp);
			dbg_Printf("FIX%s: ** Too big number - aborted.\n",
				(rnd ? "R" : ""));
		}
#endif /* DEBUG */
		FLAGS |= FLG_TRAP1|FLG_AROV;
		return;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("FIX%s: Convert %c%012llo (F) -> ", (rnd ? "R" : ""),
			((e < 0)  ? '-' : '+'), e & WORD36_ONES);
#endif /* DEBUG */

	if (fpAC.Exp >= SFP_BIAS) {
		// Convert to fixed number.
		sc  = UFP_P_NORM - (fpAC.Exp - SFP_BIAS);
		*ac = fpAC.hFrac >> sc;

		// Optionally round fixed number.
		if (rnd) {
			so = fpAC.hFrac << (64 - sc);
			if (so >= (UFP_SIGN + fpAC.Sign))
				(*ac)++;
		}

		// If sign bit is set, negate it.
		if (fpAC.Sign)
			*ac = NEG(*ac);
	} else
		*ac = 0;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo (I)\n",
			((*ac < 0) ? '-' : '+'), *ac & WORD36_ONES);
#endif /* DEBUG */
}

// Convert fixed number to floating number.
inline void p10_sfpFloat(int36 *ac, int36 e)
{
	UFP fpAC;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("FLTR: Convert %c%012llo (I) -> ",
			((e < 0)  ? '-' : '+'), e & WORD36_ONES);
#endif /* DEBUG */

	if (e < 0) {
		fpAC.Sign = 1;
		e = NEG(e);
	} else
		fpAC.Sign = 0;

	fpAC.Exp   = SFP_BIAS + 36;
	fpAC.hFrac = e << (UFP_P_NORM - 36);

	// Normalize and round a floating number.
	p10_sfpNorm(&fpAC, TRUE);
	*ac = p10_sfpPack(&fpAC, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo (FP)\n",
			((*ac < 0) ? '-' : '+'), *ac & WORD36_ONES);
#endif /* DEBUG */
}

//***********************************************************
//************ Double Precision Floating Math ***************
//***********************************************************

inline
void p10_dfpAdd(int36 *ac1, int36 *ac2, int36 e1, int36 e2)
{
	UFP fpAC, fpE;
	int diff;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DFAD: %c%012llo %c%012llo + %c%012llo %c%012llo = ",
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES,
			((*ac2 < 0) ? '-' : '+'), *ac2 & WORD36_ONES,
			((e1 < 0)  ? '-' : '+'), e1 & WORD36_ONES,
			((e2 < 0)  ? '-' : '+'), e2 & WORD36_ONES);
#endif /* DEBUG */

	if ((*ac1 | *ac2) == 0)
		p10_dfpUnpack(e1, e2, &fpAC, FP_ABS_FRAC);
	else if ((e1 | e2) == 0)
		p10_dfpUnpack(*ac1, *ac2, &fpAC, FP_ABS_FRAC);
	else {
		// Now unpack floating numbers in order to do math.
		p10_dfpUnpack(*ac1, *ac2, &fpAC, FP_SGN_FRAC);
		p10_dfpUnpack(e1, e2, &fpE, FP_SGN_FRAC);

		// Align floating numbers first.
		if (fpAC.Exp < fpE.Exp) {
			diff = -(fpAC.Exp - fpE.Exp);
			fpAC.Exp += diff;
			if (diff > UFP_DIFF128)
				diff = UFP_DIFF128;
			if (diff > 63) {
				fpAC.lFrac = (int64)fpAC.hFrac >> (diff - 64);
				fpAC.hFrac = fpAC.Sign ? UFP_ONES : 0;
			} else if (diff) {
				fpAC.lFrac = fpAC.hFrac << (64 - diff);
				fpAC.hFrac = (int64)fpAC.hFrac >> diff;
			}
		} else {
			diff = fpAC.Exp - fpE.Exp;
			fpE.Exp += diff;
			if (diff > UFP_DIFF128)
				diff = UFP_DIFF128;
			if (diff > 63) {
				fpE.lFrac = (int64)fpE.hFrac >> (diff - 64);
				fpE.hFrac = fpE.Sign ? UFP_ONES : 0;
			} else if (diff) {
				fpE.lFrac = fpE.hFrac << (64 - diff);
				fpE.hFrac = (int64)fpE.hFrac >> diff;
			}
		}

		// Do floating add math
		fpAC.hFrac += fpE.hFrac;
		if (fpAC.Sign ^ fpE.Sign) {
			if (fpAC.hFrac & UFP_SIGN) {
				fpAC.lFrac = -fpAC.lFrac;
				fpAC.hFrac = ~fpAC.hFrac + (fpAC.lFrac == 0);
				fpAC.Sign = 1;
			} else
				fpAC.Sign = 0;
		} else {
			if (fpAC.Sign) {
				fpAC.lFrac = -fpAC.lFrac;
				fpAC.hFrac = ~fpAC.hFrac + (fpAC.lFrac == 0);
			}
			if (fpAC.hFrac & UFP_CARRY) {
				fpAC.hFrac >>= 1;
				fpAC.Exp++;
			}
		}
	}

	// Normalize and pack a floating number.
	p10_dfpNorm128(&fpAC, TRUE);
	p10_dfpPack(&fpAC, ac1, ac2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES,
			((*ac2 < 0) ? '-' : '+'), *ac2 & WORD36_ONES);
#endif /* DEBUG */
}

inline
void p10_dfpSubtract(int36 *ac1, int36 *ac2, int36 e1, int36 e2)
{
	UFP fpAC, fpE;
	int diff;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DFSB: %c%012llo %c%012llo - %c%012llo %c%012llo = ",
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES,
			((*ac2 < 0) ? '-' : '+'), *ac2 & WORD36_ONES,
			((e1 < 0)  ? '-' : '+'), e1 & WORD36_ONES,
			((e2 < 0)  ? '-' : '+'), e2 & WORD36_ONES);
#endif /* DEBUG */

	// Negate floating number.
	e2 = -e2 & WORD36_MAXP;
	e1 = ~e1 + (e2 == 0);

	if ((*ac1 | *ac2) == 0)
		p10_dfpUnpack(e1, e2, &fpAC, FP_ABS_FRAC);
	else if ((e1 | e2) == 0)
		p10_dfpUnpack(*ac1, *ac2, &fpAC, FP_ABS_FRAC);
	else {
		// Now unpack floating numbers in order to do math.
		p10_dfpUnpack(*ac1, *ac2, &fpAC, FP_SGN_FRAC);
		p10_dfpUnpack(e1, e2, &fpE, FP_SGN_FRAC);

		// Align floating numbers first.
		if (fpAC.Exp < fpE.Exp) {
			diff = -(fpAC.Exp - fpE.Exp);
			fpAC.Exp += diff;
			if (diff > UFP_DIFF128)
				diff = UFP_DIFF128;
			if (diff > 63) {
				fpAC.lFrac = (int64)fpAC.hFrac >> (diff - 64);
				fpAC.hFrac = fpAC.Sign ? UFP_ONES : 0;
			} else if (diff) {
				fpAC.lFrac = fpAC.hFrac << (64 - diff);
				fpAC.hFrac = (int64)fpAC.hFrac >> diff;
			}
		} else {
			diff = fpAC.Exp - fpE.Exp;
			fpE.Exp += diff;
			if (diff > UFP_DIFF128)
				diff = UFP_DIFF128;
			if (diff > 63) {
				fpE.lFrac = (int64)fpE.hFrac >> (diff - 64);
				fpE.hFrac = fpE.Sign ? UFP_ONES : 0;
			} else if (diff) {
				fpE.lFrac = fpE.hFrac << (64 - diff);
				fpE.hFrac = (int64)fpE.hFrac >> diff;
			}
		}

		// Do floating subtract math
		fpAC.hFrac += fpE.hFrac;
		if (fpAC.Sign ^ fpE.Sign) {
			if (fpAC.hFrac & UFP_SIGN) {
				fpAC.lFrac = -fpAC.lFrac;
				fpAC.hFrac = ~fpAC.hFrac + (fpAC.lFrac == 0);
				fpAC.Sign = 1;
			} else
				fpAC.Sign = 0;
		} else {
			if (fpAC.Sign) {
				fpAC.lFrac = -fpAC.lFrac;
				fpAC.hFrac = ~fpAC.hFrac + (fpAC.lFrac == 0);
			}
			if (fpAC.hFrac & UFP_CARRY) {
				fpAC.hFrac >>= 1;
				fpAC.Exp++;
			}
		}
	}

	// Normalize and pack a floating number.
	p10_dfpNorm128(&fpAC, TRUE);
	p10_dfpPack(&fpAC, ac1, ac2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES,
			((*ac2 < 0) ? '-' : '+'), *ac2 & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_dfpMultiply(int36 *ac1, int36 *ac2, int36 e1, int36 e2)
{
	UFP    fpAC, fpE;
	uint64 xh, xl, yh, yl, mid;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DFMP: %c%012llo %c%012llo * %c%012llo %c%012llo = ",
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES,
			((*ac2 < 0) ? '-' : '+'), *ac2 & WORD36_ONES,
			((e1 < 0)  ? '-' : '+'), e1 & WORD36_ONES,
			((e2 < 0)  ? '-' : '+'), e2 & WORD36_ONES);
#endif /* DEBUG */

	// Get unpacked floating point numbers.
	p10_dfpUnpack(*ac1, *ac2, &fpAC, FP_ABS_FRAC);
	p10_dfpUnpack(e1, e2, &fpE, FP_ABS_FRAC);

	if ((fpAC.hFrac != 0) && (fpE.hFrac != 0)) {

		fpAC.Sign = fpAC.Sign ^ fpE.Sign;
		fpAC.Exp = fpAC.Exp + fpE.Exp - DFP_BIAS + 1;

		xh = fpAC.hFrac >> 32;
		xl = fpAC.hFrac & UFP_MASK32;
		yh = fpE.hFrac >> 32;
		yl = fpE.hFrac & UFP_MASK32;
		fpAC.hFrac = xh * yh;
		fpAC.lFrac = xl * yl;
		mid = (xh * yl) + (yh * xl);
		fpAC.lFrac += (mid << 32);
		fpAC.hFrac += (mid >> 32) + (fpAC.lFrac < (mid << 32));
		
		p10_dfpNorm128(&fpAC, TRUE);
		p10_dfpPack(&fpAC, ac1, ac2);
	} else
		*ac1 = *ac2 = 0;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES,
			((*ac2 < 0) ? '-' : '+'), *ac2 & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_dfpDivide(int36 *ac1, int36 *ac2, int36 e1, int36 e2)
{
	UFP    fpAC, fpE;
	uint64 quo, dvd, dvr;
	int    count;

	// Get unpacked floating point numbers.
	p10_dfpUnpack(*ac1, *ac2, &fpAC, FP_ABS_FRAC);
	p10_dfpUnpack(e1, e2, &fpE, FP_ABS_FRAC);

	if (fpAC.hFrac >= (fpE.hFrac << 1)) {
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA)) {
			dbg_Printf("DFDV: *** Error: %s\n",
				(fpE.hFrac == 0) ? "Divide By Zero" : "Overflow");
			dbg_Printf("DFDV: AC1=%c%06llo,,%06llo  AC2=%c%06llo,,%06llo\n",
				((*ac1 < 0) ? '-' : '+'), LHSR(*ac1), RH(*ac1),
				((*ac2 < 0) ? '-' : '+'), LHSR(*ac2), RH(*ac2));
			dbg_Printf("DFDV: E1=%c%06llo,,%06llo  E2=%c%06llo,,%06llo\n",
				((e1 < 0) ? '-' : '+'), LHSR(e1), RH(e1),
				((e2 < 0) ? '-' : '+'), LHSR(e2), RH(e2));
		}
#endif /* DEBUG */
		FLAGS |= (FLG_TRAP1 | FLG_AROV | FLG_FOV | FLG_DCX);\
		return;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DFDV: %c%012llo %c%012llo / %c%012llo %c%012llo = ",
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES,
			((*ac2 < 0) ? '-' : '+'), *ac2 & WORD36_ONES,
			((e1 < 0)  ? '-' : '+'), e1 & WORD36_ONES,
			((e2 < 0)  ? '-' : '+'), e2 & WORD36_ONES);
#endif /* DEBUG */

	// Do double-precision floating divide math
	if (fpAC.hFrac) {
		fpAC.Sign = fpAC.Sign ^ fpE.Sign;
		fpAC.Exp  = fpAC.Exp - fpE.Exp + DFP_BIAS + 1;

		quo = 0;
		dvd = fpAC.hFrac;
		dvr = fpE.hFrac;
		if (dvd < dvr) {
			dvd <<= 1;
			fpAC.Exp--;
		}
		count = UFP_P_NORM;
		while (count--) {
			quo <<= 1;
			if (dvd >= dvr) {
				dvd -= dvr;
				quo++;
			}
			dvd <<= 1;
		}
		fpAC.hFrac = quo;
	}

	// Normalize and optionally round result and return it.
	p10_dfpNorm64(&fpAC, TRUE);
	p10_dfpPack(&fpAC, ac1, ac2);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES,
			((*ac2 < 0) ? '-' : '+'), *ac2 & WORD36_ONES);
#endif /* DEBUG */
}

//***********************************************************
//************** PDP-10 Instruction Emulation ***************
//***********************************************************

// 110 DFAD - Double Floating Add

void p10_Opcode_DFAD(void)
{
	// (AC,AC+1) + (E,E+1) -> (AC,AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR  = p10_vRead(eAddr, dataMode);
	ARX = p10_vRead(VMA(eAddr+1), dataMode);
	p10_dfpAdd(&curAC[opAC], &curAC[AC(opAC+1)], AR, ARX);
}

// 111 DFSB - Double Floating Subtract

void p10_Opcode_DFSB(void)
{
	// (AC,AC+1) - (E,E+1) -> (AC,AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR  = p10_vRead(eAddr, dataMode);
	ARX = p10_vRead(VMA(eAddr+1), dataMode);
	p10_dfpSubtract(&curAC[opAC], &curAC[AC(opAC+1)], AR, ARX);
}

// 112 DFMP - Double Floating Multiply

void p10_Opcode_DFMP(void)
{
	// (AC,AC+1) * (E,E+1) -> (AC,AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR  = p10_vRead(eAddr, dataMode);
	ARX = p10_vRead(VMA(eAddr+1), dataMode);
	p10_dfpMultiply(&curAC[opAC], &curAC[AC(opAC+1)], AR, ARX);
}

// 113 DFDV - Double Floating Divide

void p10_Opcode_DFDV(void)
{
	// (AC,AC+1) / (E,E+1) -> (AC,AC+1)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	AR  = p10_vRead(eAddr, dataMode);
	ARX = p10_vRead(VMA(eAddr+1), dataMode);
	p10_dfpDivide(&curAC[opAC], &curAC[AC(opAC+1)], AR, ARX);
}

// 122 FIXR - Fix

void p10_Opcode_FIX(void)
{
	// (E) fixed -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	p10_sfpFix(&curAC[opAC], *pData, FALSE);
}

// 126 FIXR - Fix and Round

void p10_Opcode_FIXR(void)
{
	// (E) fixed, rounded -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	p10_sfpFix(&curAC[opAC], *pData, TRUE);
}

// 127 FLTR - Float Integer and Round

void p10_Opcode_FLTR(void)
{
	// (E) floated, rounded -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	p10_sfpFloat(&curAC[opAC], *pData);
}

// 132 FSC  - Floating Scale

void p10_Opcode_FSC(void)
{
	// (AC) * 2^E -> (AC)

	p10_sfpScale(&curAC[opAC], SXT18(eAddr));
}

// ************* Floating Add *************

// 140 FAD  - Floating Add
// 141 FADL - Floating Add Long
// 142 FADM - Floating Add to Memory
// 143 FADB - Floating Add to Both

void p10_Opcode_FAD(void)
{
	// (AC) + (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	curAC[opAC] = p10_sfpAdd(curAC[opAC], *pData, FALSE);
}

void p10_Opcode_FADM(void)
{
	// (AC) + (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	*pData = p10_sfpAdd(curAC[opAC], *pData, FALSE);
}

void p10_Opcode_FADB(void)
{
	// (AC) + (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	curAC[opAC] = *pData = p10_sfpAdd(curAC[opAC], *pData, FALSE);
}

// ************* Floating Add and Round *************

// 144 FADR  - Floating Add and Round
// 145 FADRI - Floating Add and Round Immediate
// 146 FADRM - Floating Add and Round to Memory
// 147 FADRB - Floating Add and Round to Both

void p10_Opcode_FADR(void)
{
	// (AC) + (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	curAC[opAC] = p10_sfpAdd(curAC[opAC], *pData, TRUE);
}

void p10_Opcode_FADRI(void)
{
	// (AC) + E,0 -> (AC)

	curAC[opAC] = p10_sfpAdd(curAC[opAC], RHSL(eAddr), TRUE);
}

void p10_Opcode_FADRM(void)
{
	// (AC) + (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	*pData = p10_sfpAdd(curAC[opAC], *pData, TRUE);
}

void p10_Opcode_FADRB(void)
{
	// (AC) + (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	curAC[opAC] = *pData = p10_sfpAdd(curAC[opAC], *pData, TRUE);
}

// ************* Floating Subtract *************

// 150 FSB  - Floating Subtract
// 151 FSBL - Floating Subtract Long
// 152 FSBM - Floating Subtract to Memory
// 153 FSBB - Floating Subtract to Both

void p10_Opcode_FSB(void)
{
	// (AC) - (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	curAC[opAC] = p10_sfpSubtract(curAC[opAC], *pData, FALSE);
}

void p10_Opcode_FSBM(void)
{
	// (AC) - (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	*pData = p10_sfpSubtract(curAC[opAC], *pData, FALSE);
}

void p10_Opcode_FSBB(void)
{
	// (AC) - (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	curAC[opAC] = *pData = p10_sfpSubtract(curAC[opAC], *pData, FALSE);
}

// ************* Floating Subtract and Round *************

// 154 FSBR  - Floating Subtract and Round
// 155 FSBRI - Floating Subtract and Round Immediate
// 156 FSBRM - Floating Subtract and Round to Memory
// 157 FSBRB - Floating Subtract and Round to Both

void p10_Opcode_FSBR(void)
{
	// (AC) - (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	curAC[opAC] = p10_sfpSubtract(curAC[opAC], *pData, TRUE);
}

void p10_Opcode_FSBRI(void)
{
	// (AC) - E,0 -> (AC)

	curAC[opAC] = p10_sfpSubtract(curAC[opAC], RHSL(eAddr), TRUE);
}

void p10_Opcode_FSBRM(void)
{
	// (AC) - (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	*pData = p10_sfpSubtract(curAC[opAC], *pData, TRUE);
}

void p10_Opcode_FSBRB(void)
{
	// (AC) - (E) -> (AC)(E)
	
	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	curAC[opAC] = *pData = p10_sfpSubtract(curAC[opAC], *pData, TRUE);
}

// ************* Floating Multiply *************

// 160 FMP  - Floating Multiply
// 161 FMPL - Floating Multiply Long
// 162 FMPM - Floating Multiply to Memory
// 163 FMPB - Floating Multiply to Both

void p10_Opcode_FMP(void)
{
	// (AC) * (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	curAC[opAC] = p10_sfpMultiply(curAC[opAC], *pData, FALSE);
}

void p10_Opcode_FMPM(void)
{
	// (AC) * (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	*pData = p10_sfpMultiply(curAC[opAC], *pData, FALSE);
}

void p10_Opcode_FMPB(void)
{
	// (AC) * (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	curAC[opAC] = *pData = p10_sfpMultiply(curAC[opAC], *pData, FALSE);
}

// ************* Floating Multiply and Round *************

// 164 FMPR  - Floating Multiply and Round
// 165 FMPRI - Floating Multiply and Round Immediate
// 166 FMPRM - Floating Multiply and Round to Memory
// 167 FMPRB - Floating Multiply and Round to Both

void p10_Opcode_FMPR(void)
{
	// (AC) * (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	curAC[opAC] = p10_sfpMultiply(curAC[opAC], *pData, TRUE);
}

void p10_Opcode_FMPRI(void)
{
	// (AC) * E,0 -> (AC)

	curAC[opAC] = p10_sfpMultiply(curAC[opAC], RHSL(eAddr), TRUE);
}

void p10_Opcode_FMPRM(void)
{
	// (AC) * (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	*pData = p10_sfpMultiply(curAC[opAC], *pData, TRUE);
}

void p10_Opcode_FMPRB(void)
{
	// (AC) * (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	curAC[opAC] = *pData = p10_sfpMultiply(curAC[opAC], *pData, TRUE);
}

// ************* Floating Divide *************

// 170 FDV  - Floating Divide
// 171 FDVL - Floating Divide Long
// 172 FDVM - Floating Divide to Memory
// 173 FDVB - Floating Divide to Both

void p10_Opcode_FDV(void)
{
	// (AC) / (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	curAC[opAC] = p10_sfpDivide(curAC[opAC], *pData, FALSE);
}

void p10_Opcode_FDVM(void)
{
	// (AC) / (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	*pData = p10_sfpDivide(curAC[opAC], *pData, FALSE);
}

void p10_Opcode_FDVB(void)
{
	// (AC) / (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	curAC[opAC] = *pData = p10_sfpDivide(curAC[opAC], *pData, FALSE);
}

// ************* Floating Divide and Round *************

// 174 FDVR  - Floating Divide and Round
// 175 FDVRI - Floating Divide and Round Immediate
// 176 FDVRM - Floating Divide and Round to Memory
// 177 FDVRB - Floating Divide and Round to Both

void p10_Opcode_FDVR(void)
{
	// (AC) / (E) -> (AC)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode);
	curAC[opAC] = p10_sfpDivide(curAC[opAC], *pData, TRUE);
}

void p10_Opcode_FDVRI(void)
{
	// (AC) / E,0 -> (AC)

	curAC[opAC] = p10_sfpDivide(curAC[opAC], RHSL(eAddr), TRUE);
}

void p10_Opcode_FDVRM(void)
{
	// (AC) / (E) -> (E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	*pData = p10_sfpDivide(curAC[opAC], *pData, TRUE);
}

void p10_Opcode_FDVRB(void)
{
	// (AC) / (E) -> (AC)(E)

	dataMode = p10_CheckPXCT(PXCT_DATA);

	pData = p10_Access(eAddr, dataMode | PTF_WRITE);
	curAC[opAC] = *pData = p10_sfpDivide(curAC[opAC], *pData, TRUE);
}
