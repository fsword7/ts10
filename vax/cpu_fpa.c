// cpu_fpa.c - VAX Floating Point Accelerator 
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

// Based on SIMH VAX, copyright (c) 1993-2003, Robert M. Supnik, see
// enclosed file 'Copyright' in main directory for license terms.

#include "vax/defs.h"

typedef struct {
	int32  Sign;      // Sign Field
	int32  Exponent;  // Exponent Field
	uint64 Fraction;  // Fraction Field
} UFP;

// Add/Subtract Flag

#define OP_ADD    0  // Add Flag
#define OP_SUB    1  // Subtract Flag

// Left-justified floating defintions

#define UFP_M32   0x00000000FFFFFFFF // 32-bit Mask
#define UFP_ONES  0xFFFFFFFFFFFFFFFF // 64-bit Ones
#define UFP_NORM  0x8000000000000000 // Normalized
#define UFP_FMASK 0xFFFFFF0000000000 // F Fraction
#define UFP_FRND  0x0000008000000000 // F Round
#define UFP_DMASK 0xFFFFFFFFFFFFFF00 // D Fraction
#define UFP_DRND  0x0000000000000080 // D Round
#define UFP_GMASK 0xFFFFFFFFFFFFF800 // G Fraction
#define UFP_GRND  0x0000000000000400 // G Round

#define UFP_P_NORM 63
#define UFP_P_DFHI 40
#define UFP_P_DFLO (UFP_P_DFHI - 32)
#define UFP_P_GHI  43
#define UFP_P_GLO  (UFP_P_GHI - 32)

#define FP_SIGN  0x00008000 // Sign bit in word-swapped floating-point.

#define SWAP32(x)    (((uint32)(x) >> 16) | ((uint32)(x) << 16))
#define SWAP64(h,l)  (((uint64)SWAP32(h)) << 32) | SWAP32(l)

// F/D_Floating Definitions

#define FD_N_EXP     8
#define FD_P_EXP     7
#define FD_M_EXP     ((1 << FD_N_EXP) - 1)
#define FD_EXP       (FD_M_EXP << FD_P_EXP)
#define FD_BIAS      (1 << (FD_N_EXP - 1))
#define FD_HB        (1 << FD_P_EXP)
#define FD_SIGN      FP_SIGN
#define FD_FRACW     (0xFFFF & ~(FD_EXP | FD_SIGN))
#define FD_FRACL     (FD_FRACW | 0xFFFF0000)
#define FD_GETEXP(x) (((x) >> FD_P_EXP) & FD_M_EXP)
#define FD_PUTEXP(x) ((x) << FD_P_EXP)
#define FD_PUTHI(x)  (int32)((((x) >> (16 + UFP_P_DFHI)) & FD_FRACW) | \
                     (((x) >> (UFP_P_DFHI - 16)) & ~0xFFFF))
#define FD_PUTLO(x)  (int32)((((x) >> (16 + UFP_P_DFLO)) & 0xFFFF) | \
                     (((x) << (16 - UFP_P_DFLO)) & ~0xFFFF))
#define FD_GETHI(x)  ((uint64)(((((uint32)(x) & FD_FRACW) | FD_HB) << 16) | \
                     ((uint32)(x) >> 16)) << UFP_P_DFHI)
#define FD_GETLO(x)  ((uint64)(((uint32)(x) >> 16) | \
                     ((uint32)(x) << 16)) << UFP_P_DFLO)

// G_Floating Definitions

#define G_N_EXP      11
#define G_P_EXP      4
#define G_M_EXP      ((1 << G_N_EXP) - 1)
#define G_EXP        (G_M_EXP << G_P_EXP)
#define G_BIAS       (1 << (G_N_EXP - 1))
#define G_HB         (1 << G_P_EXP)
#define G_SIGN       FP_SIGN
#define G_FRACW      (0xFFFF & ~(G_EXP | G_SIGN))
#define G_FRACL      (G_FRACW | 0xFFFF0000)
#define G_GETEXP(x)  (((x) >> G_P_EXP) & G_M_EXP)
#define G_PUTEXP(x)  ((x) << G_P_EXP)
#define G_PUTHI(x)   (int32)((((x) >> (16 + UFP_P_GHI)) & G_FRACW) | \
                     (((x) >> (UFP_P_GHI - 16)) & ~0xFFFF))
#define G_PUTLO(x)   (int32)((((x) >> (16 + UFP_P_GLO)) & 0xFFFF) | \
                     (((x) << (16 - UFP_P_GLO)) & ~0xFFFF))
#define G_GETHI(x)   ((uint64)(((((uint32)(x) & G_FRACW) | G_HB) << 16) | \
                     ((uint32)(x) >> 16)) << UFP_P_GHI)
#define G_GETLO(x)   ((uint64)(((uint32)(x) >> 16) | \
                     ((uint32)(x) << 16)) << UFP_P_GLO)

static uint64 maxv[3] = { 0x7F, 0x7FFF, 0x7FFFFFFF };

#define FP_BYTE 0
#define FP_WORD 1
#define FP_LONG 2

// Unpack F_floating numbers
inline void vax_Unpackf(register VAX_CPU *vax, uint32 wd, UFP *ufp)
{
	ufp->Sign     = wd & FP_SIGN;
	ufp->Exponent = FD_GETEXP(wd);
	if (ufp->Exponent == 0) {
		if (ufp->Sign)
			RSVD_OPND_FAULT;
		ufp->Fraction = 0;
		return;
	}
	ufp->Fraction = FD_GETHI(wd);
}

// Unpack D_floating numbers
inline void vax_Unpackd(register VAX_CPU *vax, uint32 hi, uint32 lo, UFP *ufp)
{
	ufp->Sign     = hi & FP_SIGN;
	ufp->Exponent = FD_GETEXP(hi);
	if (ufp->Exponent == 0) {
		if (ufp->Sign)
			RSVD_OPND_FAULT;
		ufp->Fraction = 0;
		return;
	}
	ufp->Fraction = FD_GETHI(hi) | FD_GETLO(lo);
}

// Unpack G_floating numbers
inline void vax_Unpackg(register VAX_CPU *vax, uint32 hi, uint32 lo, UFP *ufp)
{
	ufp->Sign     = hi & FP_SIGN;
	ufp->Exponent = G_GETEXP(hi);
	if (ufp->Exponent == 0) {
		if (ufp->Sign)
			RSVD_OPND_FAULT;
		ufp->Fraction = 0;
		return;
	}
	ufp->Fraction = G_GETHI(hi) | G_GETLO(lo);
}

// Round, Renormalize, and Pack F/D_Floating numbers
inline uint32 vax_Packfd(register VAX_CPU *vax, UFP *ufp, uint32 *rh)
{
	// If fraction is zero, return all zeros.
	if (ufp->Fraction == 0) {
		if (rh)
			*rh = 0;
		return 0;
	}

	// Round and renormalize it.
	ufp->Fraction += rh ? UFP_DRND : UFP_FRND;
	if ((ufp->Fraction & UFP_NORM) == 0) {
		ufp->Fraction >>= 1;
		ufp->Exponent++;
	}

	// Check it for overflow/underflow faults
	if (ufp->Exponent > FD_M_EXP)
		FLT_OVFL_FAULT;
	if (ufp->Exponent <= 0) {
		if (PSW & PSW_FU)
			FLT_UNFL_FAULT;
		else {
			if (rh)
				*rh = 0;
			return 0;
		}
	}

	// Now pack it and return.
	if (rh)
		*rh = FD_PUTLO(ufp->Fraction);
	return ufp->Sign | FD_PUTEXP(ufp->Exponent) | FD_PUTHI(ufp->Fraction);
}

// Round, Renormalize, and Pack G_Floating numbers
inline uint32 vax_Packg(register VAX_CPU *vax, UFP *ufp, uint32 *rh)
{
	// If fraction is zero, return all zeros.
	if (ufp->Fraction == 0) {
		*rh = 0;
		return 0;
	}

	// Round and renormalize it.
	ufp->Fraction += UFP_GRND;
	if ((ufp->Fraction & UFP_NORM) == 0) {
		ufp->Fraction >>= 1;
		ufp->Exponent++;
	}

	// Check it for overflow/underflow faults
	if (ufp->Exponent > G_M_EXP)
		FLT_OVFL_FAULT;
	if (ufp->Exponent <= 0) {
		if (PSW & PSW_FU)
			FLT_UNFL_FAULT;
		else {
			*rh = 0;
			return 0;
		}
	}

	// Now pack it and return.
	*rh = G_PUTLO(ufp->Fraction);
	return ufp->Sign | G_PUTEXP(ufp->Exponent) | G_PUTHI(ufp->Fraction);
}

// Normalize floating number.
inline void vax_Normalize(UFP *ufp)
{
	if (ufp->Fraction == 0) {
		ufp->Sign = 0;
		ufp->Exponent = 0;
	} else {
		while ((ufp->Fraction & UFP_NORM) == 0) {
			ufp->Fraction <<= 1;
			ufp->Exponent--;
		}
	}
}

// *********************** Conversion ************************

// Integer to F/D/G_Floating Convert
inline uint32 vax_ConvertInteger(register VAX_CPU *vax,
	int32 val, uint32 *rh, int32 bias)
{
	UFP ufp;

	if (val) {
		// Convert to floating number.
		if (val < 0) {
			ufp.Sign = FP_SIGN;
			val = -val;
		} else
			ufp.Sign = 0;
		ufp.Exponent = 32 + bias;
		ufp.Fraction = (uint64)val << (UFP_P_NORM - 31);	

		// Now normalize and pack it and return.
		vax_Normalize(&ufp);
		if (bias == FD_BIAS)
			return vax_Packfd(vax, &ufp, rh);
		else
			return vax_Packg(vax, &ufp, rh);
	} else {
		// Value is zero.
		if (rh)
			*rh = 0;
		return 0;
	}
}

int32 vax_fpCvtf(register VAX_CPU *vax,
	int32 *opnd, int32 *flg, int32 lnt, int32 rnd)
{
	UFP   fp;
	int32 ubexp;

	*flg = 0;
	vax_Unpackf(vax, opnd[0], &fp);
	ubexp = fp.Exponent - FD_BIAS;

	if ((fp.Exponent == 0) || (ubexp < 0))
		return 0;
	if (ubexp <= UFP_P_NORM) {
		fp.Fraction = ((fp.Fraction >> (UFP_P_NORM - ubexp)) + rnd) >> 1;
		if (fp.Fraction > (maxv[lnt] + (fp.Sign != 0)))
			*flg = CC_V;
	} else {
		if (ubexp > (UFP_P_NORM + 32))
			return 0;
		fp.Fraction <<= (ubexp - UFP_P_NORM - 1);
		*flg = CC_V;
	}

	return (fp.Sign ? (fp.Fraction ^ UFP_M32) + 1 : fp.Fraction);
}

int32 vax_fpCvtd(register VAX_CPU *vax,
	int32 *opnd, int32 *flg, int32 lnt, int32 rnd)
{
	UFP   fp;
	int32 ubexp;

	*flg = 0;
	vax_Unpackd(vax, opnd[0], opnd[1], &fp);
	ubexp = fp.Exponent - FD_BIAS;

	if ((fp.Exponent == 0) || (ubexp < 0))
		return 0;
	if (ubexp <= UFP_P_NORM) {
		fp.Fraction = ((fp.Fraction >> (UFP_P_NORM - ubexp)) + rnd) >> 1;
		if (fp.Fraction > (maxv[lnt] + (fp.Sign != 0)))
			*flg = CC_V;
	} else {
		if (ubexp > (UFP_P_NORM + 32))
			return 0;
		fp.Fraction <<= (ubexp - UFP_P_NORM - 1);
		*flg = CC_V;
	}

	return (fp.Sign ? (fp.Fraction ^ UFP_M32) + 1 : fp.Fraction);
}

int32 vax_fpCvtg(register VAX_CPU *vax,
	int32 *opnd, int32 *flg, int32 lnt, int32 rnd)
{
	UFP   fp;
	int32 ubexp;

	*flg = 0;
	vax_Unpackg(vax, opnd[0], opnd[1], &fp);
	ubexp = fp.Exponent - G_BIAS;

	if ((fp.Exponent == 0) || (ubexp < 0))
		return 0;
	if (ubexp <= UFP_P_NORM) {
		fp.Fraction = ((fp.Fraction >> (UFP_P_NORM - ubexp)) + rnd) >> 1;
		if (fp.Fraction > (maxv[lnt] + (fp.Sign != 0)))
			*flg = CC_V;
	} else {
		if (ubexp > (UFP_P_NORM + 32))
			return 0;
		fp.Fraction <<= (ubexp - UFP_P_NORM - 1);
		*flg = CC_V;
	}

	return (fp.Sign ? (fp.Fraction ^ UFP_M32) + 1 : fp.Fraction);
}

int32 vax_fpCvtfg(register VAX_CPU *vax, int32 *opnd, uint32 *rh)
{
	UFP fp;

	vax_Unpackf(vax, opnd[0], &fp);
	fp.Exponent = fp.Exponent - FD_BIAS + G_BIAS;
	return vax_Packg(vax, &fp, rh);
}

int32 vax_fpCvtgf(register VAX_CPU *vax, int32 *opnd)
{
	UFP fp;

	vax_Unpackg(vax, opnd[0], opnd[1], &fp);
	fp.Exponent = fp.Exponent - G_BIAS + FD_BIAS;
	return vax_Packfd(vax, &fp, NULL);
}

int32 vax_fpCvtdf(register VAX_CPU *vax, int32 *opnd)
{
	UFP fp;

	vax_Unpackd(vax, opnd[0], opnd[1], &fp);
	return vax_Packfd(vax, &fp, NULL);
}

// ******************** Floating Compare *************************

int32 vax_fpCmpf(register VAX_CPU *vax, int32 fp1, int32 fp2)
{
	uint32 n1, n2;

	if ((fp1 & FD_EXP) == 0) {
		if (fp1 & FP_SIGN)
			RSVD_OPND_FAULT;
		fp1 = 0;
	}

	if ((fp2 & FD_EXP) == 0) {
		if (fp2 & FP_SIGN)
			RSVD_OPND_FAULT;
		fp2 = 0;
	}

	if ((fp1 ^ fp2) & FP_SIGN)
		return ((fp1 & FP_SIGN) ? CC_N : 0);

	n1 = SWAP32(fp1);
	n2 = SWAP32(fp2);

	if (n1 == n2)
		return CC_Z;

	return (((n1 < n2) ^ ((fp1 & FP_SIGN) != 0)) ? CC_N : 0);
}

int32 vax_fpCmpd(register VAX_CPU *vax,
	int32 hi1, int32 lo1, int32 hi2, int32 lo2)
{
	uint64 n1, n2;

	if ((hi1 & FD_EXP) == 0) {
		if (hi1 & FP_SIGN)
			RSVD_OPND_FAULT;
		hi1 = 0;
	}

	if ((hi2 & FD_EXP) == 0) {
		if (hi2 & FP_SIGN)
			RSVD_OPND_FAULT;
		hi2 = 0;
	}

	if ((hi1 ^ hi2) & FP_SIGN)
		return ((hi1 & FP_SIGN) ? CC_N : 0);

	n1 = SWAP64(hi1, lo1);
	n2 = SWAP64(hi2, lo2);
	if (n1 == n2)
		return CC_Z;

	return (((n1 < n2) ^ ((hi1 & FP_SIGN) != 0)) ? CC_N : 0);
}

int32 vax_fpCmpg(register VAX_CPU *vax,
	int32 hi1, int32 lo1, int32 hi2, int32 lo2)
{
	uint64 n1, n2;

	if ((hi1 & G_EXP) == 0) {
		if (hi1 & FP_SIGN)
			RSVD_OPND_FAULT;
		hi1 = 0;
	}

	if ((hi2 & G_EXP) == 0) {
		if (hi2 & FP_SIGN)
			RSVD_OPND_FAULT;
		hi2 = 0;
	}

	if ((hi1 ^ hi2) & FP_SIGN)
		return ((hi1 & FP_SIGN) ? CC_N : 0);

	n1 = SWAP64(hi1, lo1);
	n2 = SWAP64(hi2, lo2);
	if (n1 == n2)
		return CC_Z;

	return (((n1 < n2) ^ ((hi1 & FP_SIGN) != 0)) ? CC_N : 0);
}


// *************** Floating Add/Subtract Operation ***************

void vax_fpAdd(UFP *a, UFP *b, int64 mask)
{
	int32 diff;
	UFP   tmp;

	// Check either exponent is zero first,
	// If so, do nothing with add operation.
	if (a->Exponent == 0) {
		*a = *b;
		return;
	}
	if (b->Exponent == 0)
		return;

	if (a->Exponent < b->Exponent) {
		tmp = *a;
		*a = *b;
		*b = tmp;
	}

	diff = a->Exponent - b->Exponent;
	if (a->Sign ^ b->Sign) {
		if (diff) {
			b->Fraction = (diff > 63) ? UFP_ONES :
				((-((int64)b->Fraction) >> diff) |
				(UFP_ONES << (64 - diff)));
			a->Fraction += b->Fraction;
		} else {
			if (a->Fraction < b->Fraction) {
				a->Fraction = b->Fraction - a->Fraction;
				a->Sign     = b->Sign;
			} else
				a->Fraction -= b->Fraction;
		}
		a->Fraction &= ~mask;
		vax_Normalize(a);
	} else {
		if (diff >= 64)
			b->Fraction = 0;
		else
			b->Fraction >>= diff;
		a->Fraction += b->Fraction;
		if (a->Fraction < b->Fraction) {
			a->Fraction = UFP_NORM | (a->Fraction >> 1);
			a->Exponent++;
		}
		a->Fraction &= ~mask;
	}
}

int32 vax_fpAddf(register VAX_CPU *vax, int32 *opnd, int32 op)
{
	UFP a, b;

	// Unpack F_floating numbers.
	vax_Unpackf(vax, opnd[0], &a);
	vax_Unpackf(vax, opnd[1], &b);

	// if subtract, negate it.
	if (op == OP_SUB) a.Sign ^= FP_SIGN;

	// Do add math now.
	vax_fpAdd(&a, &b, 0);

	// Return F_floating results.
	return vax_Packfd(vax, &a, NULL);
}

int32 vax_fpAddd(register VAX_CPU *vax, int32 *opnd, uint32 *rh, int32 op)
{
	UFP a, b;

	// Unpack D_floating numbers.
	vax_Unpackd(vax, opnd[0], opnd[1], &a);
	vax_Unpackd(vax, opnd[2], opnd[3], &b);

	// if subtract, negate it.
	if (op == OP_SUB) a.Sign ^= FP_SIGN;

	// Do add math now.
	vax_fpAdd(&a, &b, 0);

	// Return D_floating results.
	return vax_Packfd(vax, &a, rh);
}

int32 vax_fpAddg(register VAX_CPU *vax, int32 *opnd, uint32 *rh, int32 op)
{
	UFP a, b;

	// Unpack G_floating numbers.
	vax_Unpackg(vax, opnd[0], opnd[1], &a);
	vax_Unpackg(vax, opnd[2], opnd[3], &b);

	// if subtract, negate it.
	if (op == OP_SUB) a.Sign ^= FP_SIGN;

	// Do add math now.
	vax_fpAdd(&a, &b, 0);

	// Return G_floating results.
	return vax_Packg(vax, &a, rh);
}

// ****************** Floating Multiply Operation *****************

void vax_fpMultiply(UFP *a, UFP *b, int32 prec, int32 bias, uint64 mask)
{
	uint64 ah, bh, al, bl, rhi, rlo, rmid1, rmid2;

	// If either exponent is zero, return zero as result.
	if ((a->Exponent == 0) || (b->Exponent == 0)) {
		a->Sign     = 0;
		a->Exponent = 0;
		a->Fraction = 0;
		return;
	}

	// Do 32/64-bit Multiply Math
	a->Sign     = a->Sign ^ b->Sign;
	a->Exponent = a->Exponent + b->Exponent - bias;
	ah  = (a->Fraction >> 32) & UFP_M32;
	bh  = (b->Fraction >> 32) & UFP_M32;
	rhi = ah * bh;
	if (prec > 32) {
		al    = a->Fraction & UFP_M32;
		bl    = b->Fraction & UFP_M32;
		rmid1 = ah * bl;
		rmid2 = al * bh;
		rlo   = al * bl;
		rhi  += ((rmid1 >> 32) & UFP_M32) + ((rmid2 >> 32) & UFP_M32);
		if ((rmid1 = rlo + (rmid1 << 32)) < rlo)
			rhi++;
		if ((rmid2 = rmid1 + (rmid2 << 32)) < rmid1)
			rhi++;
	}

	a->Fraction = rhi & ~mask;
	vax_Normalize(a);
}

int32 vax_fpMulf(register VAX_CPU *vax, int32 *opnd)
{
	UFP a, b;

	vax_Unpackf(vax, opnd[0], &a);
	vax_Unpackf(vax, opnd[1], &b);
	vax_fpMultiply(&a, &b, 24, FD_BIAS, 0);
	return vax_Packfd(vax, &a, NULL);
}

int32 vax_fpMuld(register VAX_CPU *vax, int32 *opnd, uint32 *rh)
{
	UFP a, b;

	vax_Unpackd(vax, opnd[0], opnd[1], &a);
	vax_Unpackd(vax, opnd[2], opnd[3], &b);
	vax_fpMultiply(&a, &b, 56, FD_BIAS, 0);
	return vax_Packfd(vax, &a, rh);
}

int32 vax_fpMulg(register VAX_CPU *vax, int32 *opnd, uint32 *rh)
{
	UFP a, b;

	vax_Unpackg(vax, opnd[0], opnd[1], &a);
	vax_Unpackg(vax, opnd[2], opnd[3], &b);
	vax_fpMultiply(&a, &b, 53, G_BIAS, 0);
	return vax_Packg(vax, &a, rh);
}


// ****************** Floating Divide Operation *******************

void vax_fpDivide(register VAX_CPU *vax,
	UFP *dvr, UFP *dvd, int32 prec, int32 bias)
{
	uint64 quo;	
	int32  i;

	// dvd = Dividend
	// dvr = Divsor

	if (dvr->Exponent == 0)
		FLT_DZRO_FAULT;
	if (dvd->Exponent == 0)
		return;

	dvd->Sign     = dvd->Sign ^ dvr->Sign;
	dvd->Exponent = dvd->Exponent - dvr->Exponent + bias + 1;
	dvr->Fraction >>= 1;
	dvd->Fraction >>= 1;

	quo = 0;
	for (i = 0; (i < prec) && dvd->Fraction; i++) {
		quo <<= 1;
		if (dvd->Fraction >= dvr->Fraction) {
			dvd->Fraction -= dvr->Fraction;
			quo++;
		}
		dvd->Fraction <<= 1;
	}
	dvd->Fraction = quo << (UFP_P_NORM - i + 1);
	vax_Normalize(dvd);
}

int32 vax_fpDivf(register VAX_CPU *vax, int32 *opnd)
{
	UFP dvr, dvd; // Divsor and Dividend

	vax_Unpackf(vax, opnd[0], &dvr);
	vax_Unpackf(vax, opnd[1], &dvd);
	vax_fpDivide(vax, &dvr, &dvd, 26, FD_BIAS);
	return vax_Packfd(vax, &dvd, NULL);
}

int32 vax_fpDivd(register VAX_CPU *vax, int32 *opnd, uint32 *rh)
{
	UFP dvr, dvd; // Divsor and Dividend

	vax_Unpackd(vax, opnd[0], opnd[1], &dvr);
	vax_Unpackd(vax, opnd[2], opnd[3], &dvd);
	vax_fpDivide(vax, &dvr, &dvd, 58, FD_BIAS);
	return vax_Packfd(vax, &dvd, rh);
}

int32 vax_fpDivg(register VAX_CPU *vax, int32 *opnd, uint32 *rh)
{
	UFP dvr, dvd; // Divsor and Dividend

	vax_Unpackg(vax, opnd[0], opnd[1], &dvr);
	vax_Unpackg(vax, opnd[2], opnd[3], &dvd);
	vax_fpDivide(vax, &dvr, &dvd, 55, G_BIAS);
	return vax_Packg(vax, &dvd, rh);
}


// ****************** Floating Modularize Operation *********************

void vax_fpModularize(UFP *a, int32 bias, int32 *intgr, int32 *flg)
{
	if (a->Exponent <= bias)
		*intgr = 0;
	else if (a->Exponent <= (bias + 64)) {
		*intgr = (int32)(a->Fraction >> (64 - (a->Exponent - bias)));
		a->Fraction <<= (a->Exponent - bias);
	} else {
		*intgr = 0;
		a->Fraction = 0;
	}

	if (a->Sign)
		*intgr = -*intgr;

	if ((a->Exponent >= (bias + 32)) && (*intgr != 0x80000000))
		*flg = CC_V;
	else
		*flg = 0;

	a->Exponent = bias;

	vax_Normalize(a);
}

int32 vax_fpModf(register VAX_CPU *vax, int32 *opnd, int32 *intgr, int32 *flg)
{
	UFP a, b;

	vax_Unpackf(vax, opnd[0], &a);
	vax_Unpackf(vax, opnd[2], &b);
	a.Fraction |= (((uint64)ZXTB(opnd[1])) << 32);
	vax_fpMultiply(&a, &b, 32, FD_BIAS, UFP_M32);
	vax_fpModularize(&a, FD_BIAS, intgr, flg);
	return vax_Packfd(vax, &a, NULL);
}

int32 vax_fpModd(register VAX_CPU *vax,
	int32 *opnd, uint32 *flo, int32 *intgr, int32 *flg)
{
	UFP a, b;

	vax_Unpackd(vax, opnd[0], opnd[1], &a);
	vax_Unpackd(vax, opnd[3], opnd[4], &b);
	a.Fraction |= ZXTW(opnd[2]);
	vax_fpMultiply(&a, &b, 64, FD_BIAS, 0);
	vax_fpModularize(&a, FD_BIAS, intgr, flg);
	return vax_Packfd(vax, &a, flo);
}

int32 vax_fpModg(register VAX_CPU *vax,
	int32 *opnd, uint32 *flo, int32 *intgr, int32 *flg)
{
	UFP a, b;

	vax_Unpackg(vax, opnd[0], opnd[1], &a);
	vax_Unpackg(vax, opnd[3], opnd[4], &b);
	a.Fraction |= ZXTW(opnd[2]) >> 5;
	vax_fpMultiply(&a, &b, 64, G_BIAS, 0);
	vax_fpModularize(&a, G_BIAS, intgr, flg);
	return vax_Packg(vax, &a, flo);
}

// ***************** Polynomial Evaluation ***********************

void vax_fpPolyf(register VAX_CPU *vax, int32 *opnd)
{
	UFP   r, a, c;
	int32 deg = opnd[1];
	int32 ptr = opnd[2];
	int32 i, wd, res;

	if (deg > 31)
		RSVD_OPND_FAULT;
	vax_Unpackf(vax, opnd[0], &a);
	wd = ReadV(ptr, OP_LONG, RA);
	ptr += 4;
	vax_Unpackf(vax, wd, &r);
	res = vax_Packfd(vax, &r, NULL);
	for (i = 0; (i < deg) && a.Exponent; i++) {
		vax_Unpackf(vax, res, &r);
		vax_fpMultiply(&r, &a, 32, FD_BIAS, UFP_M32);
		wd = ReadV(ptr, OP_LONG, RA);
		ptr += 4;
		vax_Unpackf(vax, wd, &c);
		vax_fpAdd(&r, &c, UFP_M32);
		res = vax_Packfd(vax, &r, NULL);
	}

	R0 = res;
	R1 = 0;
	R2 = 0;
	R3 = opnd[2] + 4 + (opnd[1] << 2);
}

void vax_fpPolyd(register VAX_CPU *vax, int32 *opnd)
{
	UFP    r, a, c;
	int32  deg = opnd[2];
	int32  ptr = opnd[3];
	int32  i, wd, wd1, res;
	uint32 resh;

	if (deg > 31)
		RSVD_OPND_FAULT;
	vax_Unpackd(vax, opnd[0], opnd[1], &a);
	wd = ReadV(ptr, OP_LONG, RA);
	wd1 = ReadV(ptr + 4, OP_LONG, RA);
	ptr += 8;
	vax_Unpackd(vax, wd, wd1, &r);
	res = vax_Packfd(vax, &r, &resh);
	for (i = 0; (i < deg) && a.Exponent; i++) {
		vax_Unpackd(vax, res, resh, &r);
		vax_fpMultiply(&r, &a, 32, FD_BIAS, 0);
		wd = ReadV(ptr, OP_LONG, RA);
		wd1 = ReadV(ptr + 4, OP_LONG, RA);
		ptr += 8;
		vax_Unpackd(vax, wd, wd1, &c);
		vax_fpAdd(&r, &c, 0);
		res = vax_Packfd(vax, &r, &resh);
	}

	R0 = res;
	R1 = resh;
	R2 = 0;
	R3 = opnd[3] + 4 + (opnd[2] << 2);
}

void vax_fpPolyg(register VAX_CPU *vax, int32 *opnd)
{
	UFP    r, a, c;
	int32  deg = opnd[2];
	int32  ptr = opnd[3];
	int32  i, wd, wd1, res;
	uint32 resh;

	if (deg > 31)
		RSVD_OPND_FAULT;
	vax_Unpackg(vax, opnd[0], opnd[1], &a);
	wd = ReadV(ptr, OP_LONG, RA);
	wd1 = ReadV(ptr + 4, OP_LONG, RA);
	ptr += 8;
	vax_Unpackg(vax, wd, wd1, &r);
	res = vax_Packg(vax, &r, &resh);
	for (i = 0; (i < deg) && a.Exponent; i++) {
		vax_Unpackg(vax, res, resh, &r);
		vax_fpMultiply(&r, &a, 32, G_BIAS, 0);
		wd = ReadV(ptr, OP_LONG, RA);
		wd1 = ReadV(ptr + 4, OP_LONG, RA);
		ptr += 8;
		vax_Unpackg(vax, wd, wd1, &c);
		vax_fpAdd(&r, &c, 0);
		res = vax_Packg(vax, &r, &resh);
	}

	R0 = res;
	R1 = resh;
	R2 = 0;
	R3 = opnd[3] + 4 + (opnd[2] << 2);
}

// ****************************************************************

// ACBF - Add, Compare and Branch F_floating
// ACBD - Add, Compare and Branch D_floating
// ACBG - Add, Compare and Branch G_floating

DEF_INST(vax, ACBF)
{
	int32 r, tmp;

	r = vax_fpAddf(vax, &OP1, OP_ADD);
	tmp = vax_fpCmpf(vax, r, OP0);
	LSTORE(OP3, OP4, r);
	CC_IIZP_FP(r);
	if ((tmp & CC_Z) || ((OP1 & FP_SIGN) ? !(tmp & CC_N) : (tmp & CC_N)))
		SET_PC(PC + SXTW(vax->brDisp));

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("ACBF: Add %08X + %08X => %08X\n", OP2, OP1, r);
		dbg_Printf("ACBF: Compare %08X with %08X  CC: %s\n",
			r, OP0, CC_DSPL(tmp));
	}
#endif /* DEBUG */
}

DEF_INST(vax, ACBD)
{
	int32  r, tmp;
	uint32 rh;

	r = vax_fpAddd(vax, &OP2, &rh, OP_ADD);
	tmp = vax_fpCmpd(vax, r, rh, OP0, OP1);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZP_FP(r);
	if ((tmp & CC_Z) || ((OP2 & FP_SIGN) ? !(tmp & CC_N) : (tmp & CC_N)))
		SET_PC(PC + SXTW(vax->brDisp));
}

DEF_INST(vax, ACBG)
{
	int32  r, tmp;
	uint32 rh;

	r = vax_fpAddg(vax, &OP2, &rh, OP_ADD);
	tmp = vax_fpCmpg(vax, r, rh, OP0, OP1);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZP_FP(r);
	if ((tmp & CC_Z) || ((OP2 & FP_SIGN) ? !(tmp & CC_N) : (tmp & CC_N)))
		SET_PC(PC + SXTW(vax->brDisp));
}


// ADDF2 - Add F_floating 2 Operands
// ADDF3 - Add F_floating 3 Operands
// ADDD2 - Add D_floating 2 Operands
// ADDD3 - Add D_floating 3 Operands
// ADDG2 - Add G_floating 2 Operands
// ADDG3 - Add G_floating 3 Operands

DEF_INST(vax, ADDF)
{
	int32 r;

	r = vax_fpAddf(vax, &OP0, OP_ADD);
	LSTORE(OP2, OP3, r);
	CC_IIZZ_FP(r);
}

DEF_INST(vax, ADDD)
{
	int32  r;
	uint32 rh;

	r = vax_fpAddd(vax, &OP0, &rh, OP_ADD);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZZ_FP(r);
}

DEF_INST(vax, ADDG)
{
	int32  r;
	uint32 rh;

	r = vax_fpAddg(vax, &OP0, &rh, OP_ADD);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZZ_FP(r);
}

// CMPF - Compare F_floating
// CMPD - Compare D_floating
// CMPG - Compare G_floating

DEF_INST(vax, CMPF)
{
	CC = vax_fpCmpf(vax, OP0, OP1);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CMPF: Compare %08X with %08X, CC: %s\n",
			OP1, OP0, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CMPD)
{
	CC = vax_fpCmpd(vax, OP0, OP1, OP2, OP3);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CMPD: Compare %08X %08X with %08X %08X, CC: %s\n",
			OP2, OP3, OP0, OP1, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CMPG)
{
	CC = vax_fpCmpg(vax, OP0, OP1, OP2, OP3);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CMPG: Compare %08X %08X with %08X %08X, CC: %s\n",
			OP2, OP3, OP0, OP1, CC_DSPL(CC));
#endif /* DEBUG */
}


// CVTBF  - Convert byte to F_floating
// CVTBD  - Convert byte to D_floating
// CVTBG  - Convert byte to G_floating
// CVTWF  - Convert word to F_floating
// CVTWD  - Convert word to D_floating
// CVTWG  - Convert word to G_floating
// CVTLF  - Convert longword to F_floating
// CVTLD  - Convert longword to D_floating
// CVTLG  - Convert longword to G_floating
// CVTFB  - Convert F_floating to byte
// CVTFW  - Convert F_floating to word
// CVTFL  - Convert F_floating to longword
// CVTRFL - Convert rounded F_floating to longword
// CVTFD  - Convert F_floating to D_floating
// CVTFG  - Convert F_floating to G_floating
// CVTDB  - Convert D_floating to byte
// CVTDW  - Convert D_floating to word
// CVTDL  - Convert D_floating to longword
// CVTRDL - Convert rounded D_floating to longword
// CVTDF  - Convert D_floating to F_floating
// CVTGB  - Convert G_floating to byte
// CVTGW  - Convert G_floating to word
// CVTGL  - Convert G_floating to longword
// CVTRGL - Convert rounded G_floating to longword
// CVTGF  - Convert G_floating to F_floating

DEF_INST(vax, CVTBF)
{
	register uint32 dst;

	dst = vax_ConvertInteger(vax, SXTB(OP0), NULL, FD_BIAS);
	LSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZZ_FP(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTBF: Integer %-02X to Floating %08X  CC: %s\n",
			SXTB(OP0), dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CVTWF)
{
	register uint32 dst;

	dst = vax_ConvertInteger(vax, SXTW(OP0), NULL, FD_BIAS);
	LSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZZ_FP(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTWF: Integer %-04X to Floating %08X  CC: %s\n",
			SXTW(OP0), dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CVTLF)
{
	register uint32 dst;

	dst = vax_ConvertInteger(vax, OP0, NULL, FD_BIAS);
	LSTORE(OP1, OP2, dst);

	// Update condition codes
	CC_IIZZ_FP(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTLF: Integer %-08X to Floating %08X  CC: %s\n",
			OP0, dst, CC_DSPL(CC));
#endif /* DEBUG */
}



DEF_INST(vax, CVTBD)
{
	register uint32 dstl;
	uint32 dsth;

	dstl = vax_ConvertInteger(vax, SXTB(OP0), &dsth, FD_BIAS);
	QSTORE(OP1, OP2, dstl, dsth);

	// Update condition codes
	CC_IIZZ_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTBD: Integer %-02X to Floating %08X %08X CC: %s\n",
			SXTB(OP0), dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CVTWD)
{
	register uint32 dstl;
	uint32 dsth;

	dstl = vax_ConvertInteger(vax, SXTW(OP0), &dsth, FD_BIAS);
	QSTORE(OP1, OP2, dstl, dsth);

	// Update condition codes
	CC_IIZZ_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTWD: Integer %-04X to Floating %08X %08X CC: %s\n",
			SXTW(OP0), dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CVTLD)
{
	register uint32 dstl;
	uint32 dsth;

	dstl = vax_ConvertInteger(vax, OP0, &dsth, FD_BIAS);
	QSTORE(OP1, OP2, dstl, dsth);

	// Update condition codes
	CC_IIZZ_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTLD: Integer %-08X to Floating %08X %08X CC: %s\n",
			OP0, dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CVTBG)
{
	register uint32 dstl;
	uint32 dsth;

	dstl = vax_ConvertInteger(vax, SXTB(OP0), &dsth, G_BIAS);
	QSTORE(OP1, OP2, dstl, dsth);

	// Update condition codes
	CC_IIZZ_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTBG: Integer %-02X to Floating %08X %08X CC: %s\n",
			SXTB(OP0), dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CVTWG)
{
	register uint32 dstl;
	uint32 dsth;

	dstl = vax_ConvertInteger(vax, SXTW(OP0), &dsth, G_BIAS);
	QSTORE(OP1, OP2, dstl, dsth);

	// Update condition codes
	CC_IIZZ_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTWG: Integer %-04X to Floating %08X %08X CC: %s\n",
			SXTW(OP0), dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CVTLG)
{
	register uint32 dstl;
	uint32 dsth;

	dstl = vax_ConvertInteger(vax, OP0, &dsth, G_BIAS);
	QSTORE(OP1, OP2, dstl, dsth);

	// Update condition codes
	CC_IIZZ_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTLG: Integer %-08X to Floating %08X %08X CC: %s\n",
			OP0, dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, CVTFB)
{
	register int8  res;
	int32 flg;

	res = vax_fpCvtf(vax, &OP0, &flg, FP_BYTE, 0); 
	BSTORE(OP1, OP2, res);
	CC_IIZZ_B(res);
	CC |= flg;
}

DEF_INST(vax, CVTFW)
{
	register int16 res;
	int32 flg;

	res = vax_fpCvtf(vax, &OP0, &flg, FP_WORD, 0); 
	WSTORE(OP1, OP2, res);
	CC_IIZZ_W(res);
	CC |= flg;
}

DEF_INST(vax, CVTFL)
{
	register int32 res;
	int32 flg;

	res = vax_fpCvtf(vax, &OP0, &flg, FP_LONG, 0); 
	LSTORE(OP1, OP2, res);
	CC_IIZZ_L(res);
	CC |= flg;
}

DEF_INST(vax, CVTRFL)
{
	register int32 res;
	int32 flg;

	res = vax_fpCvtf(vax, &OP0, &flg, FP_LONG, 1); 
	LSTORE(OP1, OP2, res);
	CC_IIZZ_L(res);
	CC |= flg;
}

DEF_INST(vax, CVTFD)
{
	register int32 fp = OP0;

	if ((fp & FD_EXP) == 0) {
		if (fp & FP_SIGN)
			RSVD_OPND_FAULT;
		fp = 0;
	}
	QSTORE(OP1, OP2, fp, 0);
	CC_IIZZ_FP(fp);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTFD: Convert %08X to %08X %08X\n", OP0, fp, 0);
#endif /* DEBUG */
}

DEF_INST(vax, CVTFG)
{
	int32  r;
	uint32 rh;

	r = vax_fpCvtfg(vax, &OP0, &rh);
	QSTORE(OP1, OP2, r, rh);
	CC_IIZZ_FP(r);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTFG: Convert %08X to %08X %08X\n", OP0, r, rh);
#endif /* DEBUG */
}


DEF_INST(vax, CVTDB)
{
	register int8 res;
	int32 flg;

	res = vax_fpCvtd(vax, &OP0, &flg, FP_BYTE, 0); 
	BSTORE(OP2, OP3, res);
	CC_IIZZ_B(res);
	CC |= flg;
}

DEF_INST(vax, CVTDW)
{
	register int16 res;
	int32 flg;

	res = vax_fpCvtd(vax, &OP0, &flg, FP_WORD, 0); 
	WSTORE(OP2, OP3, res);
	CC_IIZZ_W(res);
	CC |= flg;
}

DEF_INST(vax, CVTDL)
{
	register int32 res;
	int32 flg;

	res = vax_fpCvtd(vax, &OP0, &flg, FP_LONG, 0); 
	LSTORE(OP2, OP3, res);
	CC_IIZZ_L(res);
	CC |= flg;
}

DEF_INST(vax, CVTRDL)
{
	register int32 res;
	int32 flg;

	res = vax_fpCvtd(vax, &OP0, &flg, FP_LONG, 1); 
	LSTORE(OP2, OP3, res);
	CC_IIZZ_L(res);
	CC |= flg;
}

DEF_INST(vax, CVTDF)
{
	register int32 r;

	r = vax_fpCvtdf(vax, &OP0);
	LSTORE(OP2, OP3, r);
	CC_IIZZ_FP(r);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTDF: Convert %08X %08X to %08X\n", OP0, OP1, r);
#endif /* DEBUG */
}


DEF_INST(vax, CVTGB)
{
	register int8 res;
	int32 flg;

	res = vax_fpCvtg(vax, &OP0, &flg, FP_BYTE, 0); 
	BSTORE(OP2, OP3, res);
	CC_IIZZ_B(res);
	CC |= flg;
}

DEF_INST(vax, CVTGW)
{
	register int16 res;
	int32 flg;

	res = vax_fpCvtg(vax, &OP0, &flg, FP_WORD, 0); 
	WSTORE(OP2, OP3, res);
	CC_IIZZ_W(res);
	CC |= flg;
}

DEF_INST(vax, CVTGL)
{
	register int32 res;
	int32 flg;

	res = vax_fpCvtg(vax, &OP0, &flg, FP_LONG, 0); 
	LSTORE(OP2, OP3, res);
	CC_IIZZ_L(res);
	CC |= flg;
}

DEF_INST(vax, CVTRGL)
{
	register int32 res;
	int32 flg;

	res = vax_fpCvtg(vax, &OP0, &flg, FP_LONG, 1); 
	LSTORE(OP2, OP3, res);
	CC_IIZZ_L(res);
	CC |= flg;
}

DEF_INST(vax, CVTGF)
{
	register int32 r;

	r = vax_fpCvtgf(vax, &OP0);
	LSTORE(OP2, OP3, r);
	CC_IIZZ_FP(r);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("CVTGF: Convert %08X %08X to %08X\n", OP0, OP1, r);
#endif /* DEBUG */
}


// DIVF2 - Divide F_floating 2 Operands
// DIVF3 - Divide F_floating 3 Operands
// DIVD2 - Divide D_floating 2 Operands
// DIVD3 - Divide D_floating 3 Operands
// DIVG2 - Divide G_floating 2 Operands
// DIVG3 - Divide G_floating 3 Operands

DEF_INST(vax, DIVF)
{
	int32 r;

	r = vax_fpDivf(vax, &OP0);
	LSTORE(OP2, OP3, r);
	CC_IIZZ_FP(r);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DIVF: %08X / %08X => %08X  CC: %s\n",
			OP1, OP0, r, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, DIVD)
{
	int32  r;
	uint32 rh;

	r = vax_fpDivd(vax, &OP0, &rh);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZZ_FP(r);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DIVD: %08X %08X / %08X %08X => %08X %08X  CC: %s\n",
			OP2, OP3, OP0, OP1, r, rh, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, DIVG)
{
	int32  r;
	uint32 rh;

	r = vax_fpDivg(vax, &OP0, &rh);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZZ_FP(r);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("DIVG: %08X %08X / %08X %08X => %08X %08X  CC: %s\n",
			OP2, OP3, OP0, OP1, r, rh, CC_DSPL(CC));
#endif /* DEBUG */
}


// EMODF - Extended Modulus F_floating
// EMODD - Extended Modulus D_floating
// EMODG - Extended Modulus G_floating
//
// EMODF Operands:
//   op0     = Multiplier
//   op1     = Extension
//   op2     = Multiplicand
//   op3:op4 = Integer Destination (int.wl)
//   op5:op6 = Floating Destination (flt.wl)
//
// EMODD/EMODG Operands:
//   op0:op1 = Multiplier
//   op2     = Extension
//   op3:op4 = Multiplicand
//   op5:op6 = Integer Destination (int.wl)
//   op7:op8 = Floating Destination (flt.wq)

DEF_INST(vax, EMODF)
{
	int32 r, tmp, flg;

	r = vax_fpModf(vax, &OP0, &tmp, &flg);
	if (OP5 < 0)
		ReadV(OP6, OP_LONG, WA);
	LSTORE(OP3, OP4, tmp);
	LSTORE(OP5, OP6, r);
	CC_IIZZ_FP(r);
	CC |= flg;
	if (flg && (PSW & PSW_IV))
		SET_TRAP(TRAP_INTOVF);

//	dbg_Printf("Result: I=%08X  F=%08X  CC=%02X\n", tmp, r, flg);
}

DEF_INST(vax, EMODD)
{
	int32  r, tmp, flg;
	uint32 rh;

	r = vax_fpModd(vax, &OP0, &rh, &tmp, &flg);
	if (OP7 < 0)
		ReadV(OP8, OP_LONG, WA);
	LSTORE(OP5, OP6, tmp);
	QSTORE(OP7, OP8, r, rh);
	CC_IIZZ_FP(r);
	CC |= flg;
	if (flg && (PSW & PSW_IV))
		SET_TRAP(TRAP_INTOVF);
}

DEF_INST(vax, EMODG)
{
	int32  r, tmp, flg;
	uint32 rh;

	r = vax_fpModg(vax, &OP0, &rh, &tmp, &flg);
	if (OP7 < 0)
		ReadV(OP8, OP_LONG, WA);
	LSTORE(OP5, OP6, tmp);
	QSTORE(OP7, OP8, r, rh);
	CC_IIZZ_FP(r);
	CC |= flg;
	if (flg && (PSW & PSW_IV))
		SET_TRAP(TRAP_INTOVF);
}


// MNEGF - Move Ngeated F_floating
// MNEGD - Move Negated D_floating
// MNEGG - Move Negated G_floating

DEF_INST(vax, MNEGF)
{
	register int32 dst;

	if (OP0 & FD_EXP)
		dst = OP0 ^ FP_SIGN;
	else {
		if (OP0 & FP_SIGN)
			RSVD_OPND_FAULT;
		dst = 0;
	}
	LSTORE(OP1, OP2, dst);

	// Set condition bits
	CC_IIZZ_FP(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MNEGF: %08X => %08X  CC: %s\n",
			OP0, dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MNEGD)
{
	register int32 dstl, dsth;

	if (OP0 & FD_EXP) {
		dstl = OP0 ^ FP_SIGN;
		dsth = OP1;
	} else {
		if (OP0 & FP_SIGN)
			RSVD_OPND_FAULT;
		dstl = dsth = 0;
	}
	QSTORE(OP2, OP3, dstl, dsth);

	// Set condition bits
	CC_IIZZ_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MNEGD: %08X %08X => %08X %08X  CC: %s\n",
			OP0, OP1, dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MNEGG)
{
	register int32 dstl, dsth;

	if (OP0 & G_EXP) {
		dstl = OP0 ^ FP_SIGN;
		dsth = OP1;
	} else {
		if (OP0 & FP_SIGN)
			RSVD_OPND_FAULT;
		dstl = dsth = 0;
	}
	QSTORE(OP2, OP3, dstl, dsth);

	// Set condition bits
	CC_IIZZ_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MNEGG: %08X %08X => %08X %08X  CC: %s\n",
			OP0, OP1, dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}


// MOVF - Move F_floating
// MOVD - Move D_floating
// MOVG - Move G_floating

DEF_INST(vax, MOVF)
{
	register uint32 dst = OP0;

	if ((dst & FD_EXP) == 0) {
		if (dst & FP_SIGN)
			RSVD_OPND_FAULT;
		dst = 0;
	}
	LSTORE(OP1, OP2, dst);

	// Set condition bits
	CC_IIZP_FP(dst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVF: %08X  CC: %s\n", dst, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MOVD)
{
	register uint32 dstl = OP0;
	register uint32 dsth = OP1;

	if ((dstl & FD_EXP) == 0) {
		if (dstl & FP_SIGN)
			RSVD_OPND_FAULT;
		dstl = dsth = 0;
	}
	QSTORE(OP2, OP3, dstl, dsth);

	// Set condition bits
	CC_IIZP_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVD: %08X %08X  CC: %s\n", dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, MOVG)
{
	register uint32 dstl = OP0;
	register uint32 dsth = OP1;

	if ((dstl & G_EXP) == 0) {
		if (dstl & FP_SIGN)
			RSVD_OPND_FAULT;
		dstl = dsth = 0;
	}
	QSTORE(OP2, OP3, dstl, dsth);

	// Set condition bits
	CC_IIZP_FP(dstl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("MOVG: %08X %08X  CC: %s\n", dstl, dsth, CC_DSPL(CC));
#endif /* DEBUG */
}


// MULF2 - Multiply F_floating 2 Operands
// MULF3 - Multiply F_floating 3 Operands
// MULD2 - Multiply D_floating 2 Operands
// MULD3 - Multiply D_floating 3 Operands
// MULG2 - Multiply G_floating 2 Operands
// MULG3 - Multiply G_floating 3 Operands

DEF_INST(vax, MULF)
{
	int32 r;

	r = vax_fpMulf(vax, &OP0);
	LSTORE(OP2, OP3, r);
	CC_IIZZ_FP(r);
}

DEF_INST(vax, MULD)
{
	int32  r;
	uint32 rh;

	r = vax_fpMuld(vax, &OP0, &rh);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZZ_FP(r);
}

DEF_INST(vax, MULG)
{
	int32  r;
	uint32 rh;

	r = vax_fpMulg(vax, &OP0, &rh);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZZ_FP(r);
}

// POLYF - Polynomial F_floating
// POLYD - Polynomial D_floating
// POLYG - Polynomial G_floating

DEF_INST(vax, POLYF)
{
	vax_fpPolyf(vax, &OP0);
	CC_IIZZ_FP(R0);
}

DEF_INST(vax, POLYD)
{
	vax_fpPolyd(vax, &OP0);
	CC_IIZZ_FP(R0);
}

DEF_INST(vax, POLYG)
{
	vax_fpPolyg(vax, &OP0);
	CC_IIZZ_FP(R0);
}


// SUBF2 - Subtract F_floating 2 Operands
// SUBF3 - Subtract F_floating 3 Operands
// SUBD2 - Subtract D_floating 2 Operands
// SUBD3 - Subtract D_floating 3 Operands
// SUBG2 - Subtract G_floating 2 Operands
// SUBG3 - Subtract G_floating 3 Operands

DEF_INST(vax, SUBF)
{
	int32 r;

	r = vax_fpAddf(vax, &OP0, OP_SUB);
	LSTORE(OP2, OP3, r);
	CC_IIZZ_FP(r);
}

DEF_INST(vax, SUBD)
{
	int32  r;
	uint32 rh;

	r = vax_fpAddd(vax, &OP0, &rh, OP_SUB);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZZ_FP(r);
}

DEF_INST(vax, SUBG)
{
	int32  r;
	uint32 rh;

	r = vax_fpAddg(vax, &OP0, &rh, OP_SUB);
	QSTORE(OP4, OP5, r, rh);
	CC_IIZZ_FP(r);
}


// TSTF - Test F_floating
// TSTD - Test D_floating
// TSTG - Test G_floating

DEF_INST(vax, TSTF)
{
	register uint32 src = OP0;

	if ((src & FD_EXP) == 0) {
		if (src & FP_SIGN)
			RSVD_OPND_FAULT;
		src = 0;
	}

	// Set condition bits
	CC_IIZZ_FP(src);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TSTF: Testing %08X  CC: %s\n", OP0, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, TSTD)
{
	register uint32 srcl = OP0;
	register uint32 srch = OP1;

	if ((srcl & FD_EXP) == 0) {
		if (srcl & FP_SIGN)
			RSVD_OPND_FAULT;
		srcl = srch = 0;
	}

	// Set condition bits
	CC_IIZZ_FP(srcl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TSTD: Testing %08X %08X  CC: %s\n", OP0, OP1, CC_DSPL(CC));
#endif /* DEBUG */
}

DEF_INST(vax, TSTG)
{
	register uint32 srcl = OP0;
	register uint32 srch = OP1;

	if ((srcl & G_EXP) == 0) {
		if (srcl & FP_SIGN)
			RSVD_OPND_FAULT;
		srcl = srch = 0;
	}

	// Set condition bits
	CC_IIZZ_FP(srcl);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("TSTG: Testing %08X %08X  CC: %s\n", OP0, OP1, CC_DSPL(CC));
#endif /* DEBUG */
}
