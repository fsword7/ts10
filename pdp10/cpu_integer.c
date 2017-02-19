// cpu_integer.c - PDP-6/PDP-10 integer routines
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

#include "pdp10/defs.h"
#include "pdp10/proto.h"

#define FIT32 (WORD36_ONES - 0xFFFFFFFF)

// ********************* Single Precision Math ********************

// Integer Add Table
//
//  AC + E = Result Flags
//  +    +    +     None
//  +    +    -     AROV + CRY1
//  +    -    +     CRY0 + CRY1
//  +    -    -     None
//  -    +    +     CRY0 + CRY1
//  -    +    -     None
//  -    -    +     AROV + CRY0
//  -    -    -     CRY0 + CRY1

inline void p10_spAdd(int36 *ac, int36 e)
{
	int36 r;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (S) %c%012llo + %c%012llo => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES,
			(e < 0) ? '-' : '+',   e & WORD36_ONES);
#endif /* DEBUG */

	r = *ac + e;

	// Correct sign bit of result.
	if (r > WORD36_MAXP)
		r |= WORD36_XSIGN;
	else if (r < WORD36_MAXN)
		r &= WORD36_MAXP;

	// Check sign bits and set system flags.
	if ((*ac & e) & WORD36_SIGN) {
		if (r & WORD36_SIGN)
			FLAGS |= FLG_CRY0|FLG_CRY1;
		else
			FLAGS |= FLG_TRAP1|FLG_AROV|FLG_CRY0;
	} else if (!((*ac | e) & WORD36_SIGN)) {
		if (r & WORD36_SIGN)
			FLAGS |= FLG_TRAP1|FLG_AROV;		
	} else {
		if (!(r & WORD36_SIGN))
			FLAGS |= FLG_CRY0|FLG_CRY1;
	}

	*ac = r;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo ST: %012llo\n",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES, FLAGS);
#endif /* DEBUG */
}

inline void p10_spSubtract(int36 *ac, int36 e)
{
	int36 r;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (S) %c%012llo - %c%012llo => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES,
			(e < 0) ? '-' : '+',   e & WORD36_ONES);
#endif /* DEBUG */

	r = *ac - e;

	// Correct sign bit of result.
	if (r > WORD36_MAXP)
		r |= WORD36_XSIGN;
	else if (r < WORD36_MAXN)
		r &= WORD36_MAXP;

	// Check sign bits and set system flags.
	if ((*ac & ~e) & WORD36_SIGN) {
		if (r & WORD36_SIGN)
			FLAGS |= FLG_CRY0|FLG_CRY1;
		else
			FLAGS |= FLG_TRAP1|FLG_AROV|FLG_CRY0;
	} else if (!((*ac | ~e) & WORD36_SIGN)) {
		if (r & WORD36_SIGN)
			FLAGS |= FLG_TRAP1|FLG_AROV|FLG_CRY0;		
	} else {
		if (!(r & WORD36_SIGN))
			FLAGS |= FLG_CRY0|FLG_CRY1;
	}

	*ac = r;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("%c%012llo\n", (*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_spMultiply(int36 *ac, int36 e)
{
	uint64 mch, mcl, mrh, mrl;
	uint36 mpc, mpr;
	uint36 p1, p2;

	if ((*ac == WORD36_MAXN) && (e == WORD36_MAXN)) {
		FLAGS |= (FLG_TRAP1 | FLG_AROV);
		*ac = WORD36_MAXN;
		return;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("ALU: (S) %c%012llo * %c%012llo => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES,
			(e < 0) ? '-' : '+',   e & WORD36_ONES);
#endif /* DEBUG */

	mpc = ABS(*ac);
	mpr = ABS(e);

	if ((mpc == 0) || (mpr == 0)) {
		*ac = 0;
	} else {
		p1 = p2 = 0;

		if ((mpc & FIT32) || (mpr & FIT32)) {
			mch = mpc >> 18;
			mcl = RH(mpc);
			mrh = mpr >> 18;
			mrl = RH(mpr);

			p2 = (mcl * mrl) + (((mch * mrl) + (mcl * mrh)) << 18);
			p1 = ((mch * mrh) << 1) + (p2 >> 35);
			p2 &= WORD36_MAXP;
		} else {
			p2 = mpr * mpc;
			p1 = p2 >> 35;
			p2 &= WORD36_MAXP;
		}

		// Correct sign bits first.
		if ((*ac ^ e) & WORD36_SIGN) {
			p2 = -p2 | WORD36_XSIGN;
			p1 = ~p1 + (p2 == WORD36_MAXN);
		}

		// Check if product is too big to fit.
		// If so, set system flags to TRAP1 and AROV.
		if (p1 && (p1 != -1LL))
			FLAGS |= (FLG_TRAP1|FLG_AROV);

		*ac = p2;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("%c%012llo\n", (*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */
}

inline int p10_spDivide(int36 *ac0, int36 *ac1, int36 e)
{
	int36 dvd = ABS(*ac0);
	int36 dvr = ABS(e);
	int36 q, r;

	if (dvr == 0) {
		// Set Trap 1, Overflow, and No Divide flags
		FLAGS |= (FLG_TRAP1|FLG_AROV|FLG_DCX);
		return FALSE;
	}
	
#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (S) %c%012llo / %c%012llo => ",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(e < 0) ? '-' : '+',   e & WORD36_ONES);
#endif /* DEBUG */

	q = dvd / dvr; // Result of the quotient
	r = dvd % dvr; // Result of the remainder

	// Correct sign bits
	if ((*ac0 ^ e) & WORD36_SIGN)
		q = NEG(q);
	if (*ac0 & WORD36_SIGN)
		r = NEG(r);

	// Return results.
	*ac0 = q;
	*ac1 = r;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo R %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */

	return TRUE;
}

inline void p10_spMagnitude(int36 *ac)
{
#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (S) MAG %c%012llo => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */

	*ac = ABS(*ac);

	if (*ac > WORD36_MAXP) {
		*ac |= WORD36_XSIGN;
		FLAGS |= (FLG_TRAP1|FLG_AROV|FLG_CRY1);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo\n", (*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_spNegate(int36 *ac)
{
#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (S) NEG %c%012llo => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */

	*ac = NEG(*ac);

	if (*ac > WORD36_MAXP) {
		*ac |= WORD36_XSIGN;
		FLAGS |= (FLG_TRAP1|FLG_AROV|FLG_CRY1);
	} else if (*ac == 0)
		FLAGS |= (FLG_CRY0|FLG_CRY1);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo\n", (*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_spInc(int36 *ac)
{
#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("ALU: (S) INC %c%012llo => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */

	(*ac)++;

	if (*ac > WORD36_MAXP) {
		*ac |= WORD36_XSIGN;
		FLAGS |= (FLG_TRAP1|FLG_AROV|FLG_CRY1);	
	} else if (*ac == 0)
		FLAGS |= (FLG_CRY0|FLG_CRY1);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("%c%012llo\n", (*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_spDec(int36 *ac)
{
#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("ALU: (S) DEC %c%012llo => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */

	(*ac)--;

	if (*ac < WORD36_MAXN) {
		*ac &= WORD36_MAXP;
		FLAGS |= (FLG_TRAP1|FLG_AROV|FLG_CRY1);
	} else if (*ac != -1)
		FLAGS |= (FLG_CRY0|FLG_CRY1);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("%c%012llo\n", (*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_spAShift(int36 *ac, int36 e)
{
	int   count = SXT18(e) % 256;
	int36 sign  = *ac & WORD36_XSIGN;
	int36 lost;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("ALU: (S) ASH %c%012llo %s %d => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES,
			(count >= 0) ? "<<" : ">>",
			(count >= 0) ? count : -count);
#endif /* DEBUG */

	// If count is zero, do nothing with shift operation.
	if (count == 0)
		return;

	if (count < 0) {
		// Shift Right
		if (count < -35)
			count = -35;
		*ac >>= -count;
	} else {
		// Shift Left
		if (count > 35)
			count = 35;
		// Check any lost bits. If so, set AROV and TRAP1 flags.
		lost = (*ac & WORD36_ONES) >> (35 - count);
		if (lost != (sign ? ((1LL << (count + 1)) - 1) : 0))
			FLAGS |= FLG_TRAP1|FLG_AROV;
		*ac = ((*ac << count) & WORD36_MAXP) | sign;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("%c%012llo\n", (*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_spLShift(int36 *ac, int36 e)
{
	int count = SXT18(e) % 256;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("ALU: (S) LSH %c%012llo %s %d => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES,
			(count >= 0) ? "<<" : ">>",
			(count >= 0) ? count : -count);
#endif /* DEBUG */

	// Shifting value in either direction.
	if (ABS(count) > 35)
		*ac = 0;
	else {
		*ac &= WORD36_ONES;
		if (count < 0)
			*ac >>= -count; // Shift Right
		else
			*ac <<= count;  // Shift Left
		*ac = SXT36(*ac);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("%c%012llo\n", (*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_spRotate(int36 *ac, int36 e)
{
	int count = SXT18(e);

	count = ((count == WORD18_MAXN) ? -256 : (count % 256)) % 36;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("ALU: (S) ROT %c%012llo %s %d => ",
			(*ac < 0) ? '-' : '+', *ac & WORD36_ONES,
			(count >= 0) ? "<<" : ">>",
			(count >= 0) ? count : -count);
#endif /* DEBUG */

	*ac &= WORD36_ONES;

	// Rotating value in either direction.
	if (count < 0) 
		count = 36 - -count;
	*ac = (*ac << count) | (*ac >> (36 - count));

	*ac = SXT36(*ac);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("%c%012llo\n", (*ac < 0) ? '-' : '+', *ac & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_dpMultiply(int36 *ac0, int36 *ac1, int36 e)
{
	uint64 mch, mcl, mrh, mrl;
	uint36 mpc, mpr;
	uint36 p1, p2;

	if ((*ac0 == WORD36_MAXN) && (e == WORD36_MAXN)) {
		FLAGS |= (FLG_TRAP1 | FLG_AROV);
		*ac0 = *ac1 = WORD36_MAXN;
		return;
	}

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("ALU: (S) %c%012llo * %c%012llo = ",
				(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
				(e < 0) ? '-' : '+', e & WORD36_ONES);
#endif /* DEBUG */

	mpc = ABS(*ac0);
	mpr = ABS(e);

	if ((mpc == 0) || (mpr == 0)) {
		*ac0 = *ac1 = 0;
	} else {
		p1 = p2 = 0;

		if ((mpc & FIT32) || (mpr & FIT32)) {
			mch = mpc >> 18;
			mcl = RH(mpc);
			mrh = mpr >> 18;
			mrl = RH(mpr);

			p2 = (mcl * mrl) + (((mch * mrl) + (mcl * mrh)) << 18);
			p1 = ((mch * mrh) << 1) + (p2 >> 35);
			p2 &= WORD36_MAXP;
		} else {
			p2 = mpc * mpr;
			p1 = p2 >> 35;
			p2 &= WORD36_MAXP;
		}

		// Correct sign bits and return results of product.
		if ((*ac0 ^ e) & WORD36_SIGN) {
			*ac1 = -p2 | WORD36_XSIGN;
			*ac0 = ~p1 + (*ac1 == WORD36_MAXN);
		} else {
			// Check if product is too big to fit.
			// If so, set system flags to TRAP1 and AROV.
			if (p1 > WORD36_MAXP) {
				FLAGS |= FLG_TRAP1|FLG_AROV;
				p2 |= WORD36_XSIGN;
				p1 |= WORD36_XSIGN;
			}

			*ac1 = p2;
			*ac0 = p1;
		}
	}

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("%c%012llo %c%012llo\n",
				(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
				(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */
}

inline int p10_dpDivide(int36 *ac0, int36 *ac1, int36 e)
{
	int64 q, r;       // Quotient and Remainder
	uint36 dvd1, dvd2; // Dividend
	uint36 dvr;
	int    i;

	// Prepare for 72-bit divide math
	dvr  = ABS(e);
	if (*ac0 & WORD36_SIGN) {
		dvd2 = (-*ac1) & WORD36_MAXP;
		dvd1 = (~*ac0 + (dvd2 == 0)) & WORD36_MAXP;
	} else {
		dvd2 = *ac1 & WORD36_MAXP;
		dvd1 = *ac0 & WORD36_MAXP;
	}

	// Check like divide by zero, etc.
	if (dvd1 >= dvr) {
		// Set Trap 1, Overflow, and No Divide flags
		FLAGS |= (FLG_TRAP1|FLG_AROV|FLG_DCX);
		return FALSE;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (S) %c%012llo %c%012llo / %c%012llo = ",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES,
			(e < 0) ? '-' : '+', e & WORD36_ONES);
#endif /* DEBUG */

	// Do 70-bit divide math
	for (i = 0, q = 0; i < 35; i++) {
		dvd1 = (dvd1 << 1) | ((dvd2 >> 34) & 1);
		dvd2 = (dvd2 << 1) & WORD36_MAXP;
		q <<= 1;
		if (dvd1 >= dvr) {
			dvd1 -= dvr;
			q++;
		}
	}
	r = dvd1;

	// Restore sign bits for both quotient and remainder.
	if ((*ac0 ^ e) & WORD36_SIGN)
		q = NEG(q);
	if (*ac0 & WORD36_SIGN)
		r = NEG(r);

	// Return results of quotient and remainder
	*ac0 = q; // Quotient
	*ac1 = r; // Remainder

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo R %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */

	return TRUE;
}

// ************ Double Precision Arthimetic ************

inline void p10_dpAdd(int36 *ac0, int36 *ac1, int36 e0, int36 e1)
{
	int36 r;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (D) %c%012llo %c%012llo + %c%012llo %c%012llo = ",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES,
			(e0 < 0) ? '-' : '+', e0 & WORD36_ONES,
			(e1 < 0) ? '-' : '+', e1 & WORD36_ONES);
#endif /* DEBUG */

	*ac1 = (*ac1 & WORD36_MAXP) + (e1 & WORD36_MAXP);
	r = *ac0 + e0 + ((*ac1 & WORD36_SIGN) ? 1 : 0);

	// Correct sign bit of result.
	if (r > WORD36_MAXP)
		r |= WORD36_XSIGN;
	else if (r < WORD36_MAXN)
		r &= WORD36_MAXP;
	*ac1 = (r & WORD36_SIGN) ?
		(*ac1 | WORD36_XSIGN) : (*ac1 & WORD36_MAXP);

	// Check sign bits and set system flags.
	if ((*ac0 & e0) & WORD36_SIGN) {
		if (r & WORD36_SIGN)
			FLAGS |= FLG_CRY0|FLG_CRY1;
		else
			FLAGS |= FLG_TRAP1|FLG_AROV|FLG_CRY0;
	} else if (!((*ac0 | e0) & WORD36_SIGN)) {
		if (r & WORD36_SIGN)
			FLAGS |= FLG_TRAP1|FLG_AROV|FLG_CRY1;		
	} else {
		if (!(r & WORD36_SIGN))
			FLAGS |= FLG_CRY0|FLG_CRY1;
	}

	*ac0 = r;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_dpSubtract(int36 *ac0, int36 *ac1, int36 e0, int36 e1)
{
	int36 r;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (D) %c%012llo %c%012llo - %c%012llo %c%012llo = ",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES,
			(e0 < 0) ? '-' : '+', e0 & WORD36_ONES,
			(e1 < 0) ? '-' : '+', e1 & WORD36_ONES);
#endif /* DEBUG */

	*ac1 = (*ac1 & WORD36_MAXP) - (e1 & WORD36_MAXP);
	r = (*ac0 - e0) - ((*ac1 & WORD36_SIGN) ? 1 : 0);

	// Correct sign bit of result.
	if (r > WORD36_MAXP)
		r |= WORD36_XSIGN;
	else if (r < WORD36_MAXN)
		r &= WORD36_MAXP;
	*ac1 = (r & WORD36_SIGN) ?
		(*ac1 | WORD36_XSIGN) : (*ac1 & WORD36_MAXP);

	// Check sign bits and set system flags.
	if ((*ac0 & ~e0) & WORD36_SIGN) {
		if (r & WORD36_SIGN)
			FLAGS |= FLG_CRY0|FLG_CRY1;
		else
			FLAGS |= FLG_TRAP1|FLG_AROV|FLG_CRY0;
	} else if (!((*ac0 | ~e0) & WORD36_SIGN)) {
		if (r & WORD36_SIGN)
			FLAGS |= FLG_TRAP1|FLG_AROV|FLG_CRY1;		
	} else {
		if (!(r & WORD36_SIGN))
			FLAGS |= FLG_CRY0|FLG_CRY1;
	}

	*ac0 = r;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_dpNegate(int36 *ac0, int36 *ac1)
{
#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (D) NEG %c%012llo %c%012llo => ",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */

	// Negate double word
	*ac1 = -*ac1 & WORD36_MAXP;
	*ac0 = ~*ac0 + (*ac1 == 0);

	// Correct sign bits and set system flags
	if (*ac1 == 0) {
		if (*ac0 > WORD36_MAXP) {
			*ac0 |= WORD36_XSIGN;
			FLAGS |= (FLG_TRAP1|FLG_AROV|FLG_CRY1);
		} else if (*ac0 == 0)
			FLAGS |= (FLG_CRY0|FLG_CRY1);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_dpAShift(int36 *ac0, int36 *ac1, int36 e)
{
	int   count = SXT18(e) % 256;
	int36 sign  = *ac0 & WORD36_XSIGN;
	int36 fill  = (sign ? WORD36_XONES : 0);
	int36 lost;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("ALU: (D) ASH %c%012llo %c%012llo %s %d => ",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES,
			(count >= 0) ? "<<" : ">>",
			(count >= 0) ? count : -count);
#endif /* DEBUG */

	// If count is zero, do nothing with shift operation.
	if (count == 0)
		return;

	// Clear extended sign bits
	*ac0 &= WORD36_MAXP;
	*ac1 &= WORD36_MAXP;

	if (count < 0) {
		count = -count;
		if (count > 70)
			count = 70;
		if (count >= 35) {
			*ac1 = (*ac0 >> (count - 35)) | (fill << (70 - count));
			*ac0 = fill;
		} else {
			*ac1 = sign |
				(((*ac1 >> count) | (*ac0 << (35 - count))) & WORD36_MAXP);
			*ac0 = (*ac0 >> count) | (fill << (35 - count));
		}
	} else {
		if (count > 70)
			count = 70;
		if (count >= 35) {
			// Check any lost bits. If so, set AROV and TRAP1 flags.
			lost = *ac1 >> (70 - count);
			if ((lost != (sign ? ((1LL << count) - 1) : 0)) ||
			    (*ac0 != (sign ? WORD36_MAXP : 0)))
				FLAGS |= FLG_TRAP1|FLG_AROV;

			*ac0 = sign | ((*ac1 << (count - 35)) & WORD36_MAXP);
			*ac1 = sign;
		} else {
			// Check any lost bits. If so, set AROV and TRAP1 flags.
			lost = *ac0 >> (35 - count);
			if (lost != (sign ? ((1LL << count) - 1) : 0))
				FLAGS |= FLG_TRAP1|FLG_AROV;

			*ac0 = sign |
				(((*ac0 << count) | (*ac1 >> (35 - count))) & WORD36_MAXP);
			*ac1 = sign | ((*ac1 << count) & WORD36_MAXP);
		}
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_dpLShift(int36 *ac0, int36 *ac1, int36 e)
{
	int count = SXT18(e) % 256;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("ALU: (D) LSH %c%012llo %c%012llo %s %d => ",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES,
			(count >= 0) ? "<<" : ">>",
			(count >= 0) ? count : -count);
#endif /* DEBUG */

	// Shifting value in either direction.
	if (ABS(count) > 71) {
		*ac0 = 0;
		*ac1 = 0;
	} else {
		*ac0 &= WORD36_ONES;
		*ac1 &= WORD36_ONES;
		if (count < 0) {
			count = -count;
			if (count > 35) {
				*ac1 = *ac0 >> (count - 36);
				*ac0 = 0;
			} else {
				*ac1 = (*ac1 >> count) | (*ac0 << (36 - count));
				*ac0 >>= count;
			}
		} else {
			if (count > 35) {
				*ac0 = *ac1 << (count - 36);
				*ac1 = 0;
			} else {
				*ac0 = (*ac0 << count) | (*ac1 >> (36 - count));
				*ac1 <<= count;
			}
		}
		*ac0 = SXT36(*ac0);
		*ac1 = SXT36(*ac1);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_dpRotate(int36 *ac0, int36 *ac1, int36 e)
{
	int count = SXT18(e);
	int36 temp;

	count = ((count == WORD18_MAXN) ? -256 : (count % 256)) % 72;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("ALU: (D) ROT %c%012llo %c%012llo %s %d => ",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES,
			(count >= 0) ? "<<" : ">>",
			(count >= 0) ? count : -count);
#endif /* DEBUG */

	*ac0 &= WORD36_ONES;
	*ac1 &= WORD36_ONES;

	if (count < 0)
		count = 72 - -count;
	temp = *ac0;
	if (count > 35) {
		*ac0 = (*ac1 << (count - 36)) | (temp >> (72 - count));
		*ac1 = (temp << (count - 36)) | (*ac1 >> (72 - count));
	} else {
		*ac0 = (temp << count) | (*ac1 >> (36 - count));
		*ac1 = (*ac1 << count) | (temp >> (36 - count));
	}
		
	*ac0 = SXT36(*ac0);
	*ac1 = SXT36(*ac1);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("%c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
#endif /* DEBUG */
}

inline void p10_qpMultiply(int36 *ac0, int36 *ac1, int36 *ac2, int36 *ac3,
	int36 e0, int36 e1)
{
	uint36 mpc1, mpc2;      // Multiplicand
	uint36 mpr1, mpr2;      // Multiplier
	uint36 p1, p2, p3, p4;  // Product
	int   count;           // Loop control

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ALU: Double Precision Multiply (DMUL)\n");
		dbg_Printf("ALU: Multiplicand: %c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
		dbg_Printf("ALU: Multiplier:   %c%012llo %c%012llo\n",
			(e0 < 0) ? '-' : '+', e0 & WORD36_ONES,
			(e1 < 0) ? '-' : '+', e1 & WORD36_ONES);
	}
#endif /* DEBUG */

	// Prepare for 70-bit multiply math.

	if (*ac0 & WORD36_SIGN) {
		mpc2 = (-*ac1) & WORD36_MAXP;
		mpc1 = (~*ac0 + (mpc2 == 0)) & WORD36_ONES;
	} else {
		mpc2 = *ac1 & WORD36_MAXP;
		mpc1 = *ac0 & WORD36_MAXP;
	}

	if (e0 & WORD36_SIGN) {
		mpr2 = (-e1) & WORD36_MAXP;
		mpr1 = (~e0 + (mpr2 == 0)) & WORD36_ONES;
	} else {
		mpr2 = e1 & WORD36_MAXP;
		mpr1 = e0 & WORD36_MAXP;
	}

	if (((mpr1 | mpr2) == 0) || ((mpc1 | mpc2) == 0)) {
		// Check either is zero first.
		// If either is zero, return a result of zero.
		*ac3 = *ac2 = *ac1 = *ac0 = 0;
	} else {
		// Do 140-bit multiply math
		//
		// Product = Multiplicand * Multiplier

		p1 = p2 = p3 = p4 = 0;

		for (count = 0; count < 71; count++) {
			if (count) {
				// Shift right by one

				p4 = (p4 >> 1) | ((p3 & 1) << 34);
				p3 = (p3 >> 1) | ((p2 & 1) << 34);
				p2 = (p2 >> 1) | ((p1 & 1) << 34);
				p1 >>= 1;

				mpr2 = (mpr2 >> 1) | ((mpr1 & 1) << 34);
				mpr1 >>= 1;
			}

			if (mpr2 & 1) {
				p2 += mpc2;
				p1 += mpc1 + (p2 > WORD36_MAXP);
				p2 &= WORD36_MAXP;
			}
		}

		// Restore sign bits and return results of product.
		if ((*ac0 ^ e0) & WORD36_SIGN) {
			*ac3 = -p4 & WORD36_MAXP;
			*ac2 = (~p3 + (*ac3 == 0)) & WORD36_MAXP;
			*ac1 = (~p2 + (*ac2 == 0)) & WORD36_MAXP;
			*ac0 = ~p1 + (*ac1 == 0);
		} else {
			// Check if product is too big to fit.
			// If so, set system flags to TRAP1 and AROV.
			if (p1 > WORD36_MAXP) {
				FLAGS |= FLG_TRAP1|FLG_AROV;
				p1 |= WORD36_XSIGN;
			}

			*ac3 = p4;
			*ac2 = p3;
			*ac1 = p2;
			*ac0 = p1;
		}

		// Fix sign bits on a quadword (140-bit word).
		if (*ac0 < 0) {
			*ac3 |= WORD36_XSIGN;
			*ac2 |= WORD36_XSIGN;
			*ac1 |= WORD36_XSIGN;
		}
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ALU: Product: %c%012llo %c%012llo %c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES,
			(*ac2 < 0) ? '-' : '+', *ac2 & WORD36_ONES,
			(*ac3 < 0) ? '-' : '+', *ac3 & WORD36_ONES);
	}
#endif /* DEBUG */
}

inline int p10_qpDivide(int36 *ac0, int36 *ac1, int36 *ac2, int36 *ac3,
	int36 e0, int36 e1)
{
	int36 q1, q2;     // Quotient
	uint36 dvd1, dvd2; // Dividend
	uint36 dvd3, dvd4; 
	uint36 dvr1, dvr2; // Divisor
	int   count;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ALU: Double Precision Divide (DDIV)\n");
		dbg_Printf("ALU: Dividend:  %c%012llo %c%012llo %c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES,
			(*ac2 < 0) ? '-' : '+', *ac2 & WORD36_ONES,
			(*ac3 < 0) ? '-' : '+', *ac3 & WORD36_ONES);
		dbg_Printf("ALU: Divisor:   %c%012llo %c%012llo\n",
			(e0 < 0) ? '-' : '+', e0 & WORD36_ONES,
			(e1 < 0) ? '-' : '+', e1 & WORD36_ONES);
	}
#endif /* DEBUG */

	// Prepare for 70-bit divide math

	if (*ac0 & WORD36_SIGN) {
		int36 cryin;

		dvd4 = (-*ac3) & WORD36_MAXP;
		cryin = (dvd4 == 0);
		dvd3 = (~*ac2 + cryin) & WORD36_MAXP;
		cryin = cryin & (dvd3 == 0);
		dvd2 = (~*ac1 + cryin) & WORD36_MAXP;
		cryin = cryin & (dvd2 == 0);
		dvd1 = (~*ac0 + cryin) & WORD36_ONES;
	} else {
		dvd4 = *ac3 & WORD36_MAXP;
		dvd3 = *ac2 & WORD36_MAXP;
		dvd2 = *ac1 & WORD36_MAXP;
		dvd1 = *ac0 & WORD36_MAXP;
	}

	if (e0 & WORD36_SIGN) {
		dvr2 = (-e1) & WORD36_MAXP;
		dvr1 = (~e0 + (dvr2 == 0)) & WORD36_ONES;
	} else {
		dvr2 = e1 & WORD36_MAXP;
		dvr1 = e0 & WORD36_MAXP;
	}

	// Check like divide by zero, etc.
	if ((dvd1 > dvr1) || ((dvd1 == dvr1) && (dvd2 >= dvr2))) {
		// Set Trap 1, Overflow, and No Divide flags
#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("ALU: Program Error: Dividend >= Divisor\n");
#endif /* DEBUG */
		FLAGS |= (FLG_TRAP1|FLG_AROV|FLG_DCX);
		return FALSE;
	}

	// Do 140-bit divide math
	//
	// Dividend
	// -------- = Quotient R Remainder
	// Divisor

	q1 = 0;
	q2 = 0;
	for (count = 0; count < 70; count++) {
		// Shift left by one
		dvd1 = ((dvd1 << 1) | ((dvd2 >> 34) & 1)) & WORD36_ONES;
		dvd2 = ((dvd2 << 1) | ((dvd3 >> 34) & 1)) & WORD36_MAXP;
		dvd3 = ((dvd3 << 1) | ((dvd4 >> 34) & 1)) & WORD36_MAXP;
		dvd4 = (dvd4 << 1) & WORD36_MAXP;

		q1 = (q1 << 1) | ((q2 >> 34) & 1);
		q2 = (q2 << 1) & WORD36_MAXP;

		if ((dvd1 > dvr1) || ((dvd1 == dvr1) && (dvd2 >= dvr2))) {
			dvd1 = dvd1 - dvr1 - (dvd2 < dvr2);
			dvd2 = (dvd2 - dvr2) & WORD36_MAXP;
			q2++;
		}
	}

	// Restore sign bits for both quotient and remainder.
	if (((*ac0 ^ e0) & WORD36_SIGN) && (q1 | q2)) {
		q2 = -q2 | WORD36_XSIGN;
		q1 = ~q1 + (q2 == WORD36_MAXN);
	}
	if ((*ac0 & WORD36_SIGN) && (dvd1 | dvd2)) {
		dvd2 = -dvd2 | WORD36_XSIGN;
		dvd1 = ~dvd1 + (dvd2 == WORD36_MAXN);
	}

	// Return results of quotient and remainder
	*ac0 = q1;   // Quotient
	*ac1 = q2;
	*ac2 = dvd1; // Remainder
	*ac3 = dvd2;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ALU: Quotient:  %c%012llo %c%012llo\n",
			(*ac0 < 0) ? '-' : '+', *ac0 & WORD36_ONES,
			(*ac1 < 0) ? '-' : '+', *ac1 & WORD36_ONES);
		dbg_Printf("ALU: Remainder: %c%012llo %c%012llo\n",
			(*ac2 < 0) ? '-' : '+', *ac2 & WORD36_ONES,
			(*ac3 < 0) ? '-' : '+', *ac3 & WORD36_ONES);
	}
#endif /* DEBUG */

	return TRUE;
}
