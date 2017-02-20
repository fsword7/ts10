// cpu_extend.c - EXTEND Instruction for KL10 and KS10 Processor
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


// Instructions handled on this module for KL10 and KS10 Processor:
//
//   MOVSLJ  Move String Left Justified
//   MOVSO   Move String Offset
//   MOVST   Move String Translated
//   MOVSRJ  Move String Right Justified
//   CMPSL   Compare Strings, Skip on Less
//   CMPSE   Compare Strings, Skip on Equal
//   CMPSLE  Compare Strings, Skip on Less or Equal
//   CMPGE   Compare Strings, Skip on Greater or Equal
//   CMPSN   Compare Strings, Skip on Not Equal
//   CMPSG   Compare Strings, Skip on Greater
//   CVTBDO  Convert Binary to Decimal Offset
//   CVTBDT  Convert Binary to Decimal Translated
//   CVTDBO  Convert Decimal to Binary Offset
//   CVTDBT  Convert Decimal to Binary Translated
//   EDIT    Edit String
//
// Instructions handled on this module for KL10 Processor only:
//
//   XBLT    Extended Block Transfer (020)
//   GFIX    G-Format Fix (024)
//   GFIXR   G-Format Fix and Round (026)
//   GFLTR   G-Format Float and Round (030)
//   GDFIX   G-Format Double Fix (023)
//   GDFIXR  G-Format Double Fix and Round (025)
//   DGFLTR  Double G-Format Float and Round (027)
//   GSNGL   G-Format to Single Precision (021)
//   GDBLE   Single Precision to G-Format (022)
//   GFSC    G-Format Floating Scale (031)

#include "pdp10/defs.h"
#include "pdp10/ks10.h"
#include "pdp10/proto.h"

#define EXT_CMPSL  0001 // Compare Strs and Skip if Less than
#define EXT_CMPSE  0002 // Compare Strs and Skip if Equal to
#define EXT_CMPSLE 0003 // Compare Strs and Skip if Less than or Equal to
#define EXT_EDIT   0004 // Edit String
#define EXT_CMPSGE 0005 // Compare Strs and Skip if Greater than or Equal to
#define EXT_CMPSN  0006 // Compare Strs and Skip if Not Equal to
#define EXT_CMPSG  0007 // Compare Strs and Skip if Greater than
#define EXT_CVTDBO 0010 // Convert Decimal to Binary Offset
#define EXT_CVTDBT 0011 // Convert Decimal to Binary Translated
#define EXT_CVTBDO 0012 // Convert Binary to Decimal Offset
#define EXT_CVTBDT 0013 // Convert Binary to Decimal Translated
#define EXT_MOVSO  0014 // Move String Offset
#define EXT_MOVST  0015 // Move String Translated
#define EXT_MOVSLJ 0016 // Move String Left Justified
#define EXT_MOVSRJ 0017 // Move String Right Justified

#define EXT_NOSKIP 0
#define EXT_SKIP   1
#define EXT_MUUO   2

#define EXT_EDDST  000002 // Destination
#define EXT_EDSRC  000001 // Source 

#define STR_FLG_LEAD  0400000000000LL // Leading Space Flag (L flag)
#define STR_FLG_SIGN  0400000000000LL // Significance Flag  (S flag)
#define STR_FLG_NZERO 0200000000000LL // Non-Zero Flag      (N flag)
#define STR_FLG_MINUS 0100000000000LL // Minus Flag         (M flag)
#define STR_FLG_MASK  0700000000000LL // Flag Mask
#define STR_FLG_XMASK (WORD36_XSIGN|STR_FLG_MASK)
#define STR_LEN_MASK  0000777777777LL // String Length Mask
#define STR_MBZ       0777000000000LL // Must be zero

// Accumulator Table (AC#0 to AC#5) for EDIT Instruction
//
// MSB                                                                     LSB
//  |<-------- Left halfword ---------->|<-------- Right halfword --------->|
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |S N M 0|PBN|                 Pattern String Address                    |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  |                                                                       |
//  +-+-+-^-+-+-^-+-+-^-+-+ Source String Byte Pointer -+-^-+-+-^-+-+-^-+-+-+
//  |                                                                       |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  |0 0 0 0 0 0|                      Mark Address                         |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  |                                                                       |
//  +-+-+-^-+-+-^-+-+-^- Destination String Byte Pointer -^-+-+-^-+-+-^-+-+-+
//  |                                                                       |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//   0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// PBN = Pattern Byte Number

#define EDIT_PBN_MASK    03  // Pattern Byte Number - Mask
#define EDIT_PBN_POS     30  // Pattern Byte Number - Position
#define EDIT_MBZ         0047777000000LL // Must be zeros
#define EDIT_PBYTEN(x)   (((x) >> EDIT_PBN_POS) & EDIT_PBN_MASK)
#define EDIT_PBYTE(x, y) ((x) >> (27 - ((y) * 9)) & EDIT_PAT_MASK)

#define EDIT_PAT_MASK   0777 // Pattern Byte Mask
#define EDIT_OP_MASK    0700 // Opcode Mask
#define EDIT_NUM_MASK   0077 // Number Mask

// Pattern opcodes
#define EDIT_STOP     0000 // Stop Edit
#define EDIT_SELECT   0001 // Select Next Source Byte
#define EDIT_SIGST    0002 // Start Significance
#define EDIT_FLDSEP   0003 // Separate Fields
#define EDIT_EXCHMD   0004 // Exchange Mark and Destination Pointers
#define EDIT_NOP      0005 // No-operation
#define EDIT_MESSAG   0100 // Insert Message Character
#define EDIT_SKPM     0500 // Skip on M flag
#define EDIT_SKPN     0600 // Skip on N flag
#define EDIT_SKPA     0700 // Skip Always

// Accumulator Table (AC#0 to AC#5) for CMPS Instruction
//
// MSB                                                                     LSB
//  |<-------- Left halfword ---------->|<-------- Right halfword --------->|
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |0 0 0 0 0 0 0 0 0|                  String 1 Length                    |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  |                                                                       |
//  +-+-+-^-+-+-^-+-+-^-+-+-^ String 1 Byte Pointer ^-+-+-^-+-+-^-+-+-^-+-+-+
//  |                                                                       |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  |0 0 0 0 0 0 0 0 0|                  String 2 Length                    |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  |                                                                       |
//  +-+-+-^-+-+-^-+-+-^-+-+-^ String 2 Byte Pointer ^-+-+-^-+-+-^-+-+-^-+-+-+
//  |                                                                       |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//   0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Translation Table for MOVST instruction
//
//     Translation function for even B     Translation function for odd B
//
// MSB                                                                     LSB
//  |<-------- Left halfword ---------->|<-------- Right halfword --------->|
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | OP  |0 0 0|  Substitute for Byte  | OP  |0 0 0|  Substitude for Byte  |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//   0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Translation Table for CVTDBT/CVTBDT instruction
//
//     Translation function for even B     Translation function for odd B
//
// MSB                                                                     LSB
//  |<-------- Left halfword ---------->|<-------- Right halfword --------->|
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | OP  |0 0 0 0 0 0 0 0 0 0 0| Digit | OP  |0 0 0 0 0 0 0 0 0 0 0| Digit |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//   0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Location: E1 + B/2

#define XLATE_M_CODE     0700000
#define XLATE_M_BYTE     0007777
#define XLATE_M_DIGIT    0000017
#define XLATE_P_CODE     (17 - 2)

#define XLATE_OPCODE(x) (((x) & XLATE_M_CODE) >> XLATE_P_CODE)

static const int36 ext_Powers10[23][2] = {
	{           0LL,           0LL }, // 10**0
	{           0LL,           1LL }, // 10**1
	{           0LL,          10LL }, // 10**2
	{           0LL,         100LL }, // 10**3
	{           0LL,        1000LL }, // 10**4
	{           0LL,       10000LL }, // 10**5
	{           0LL,      100000LL }, // 10**6
	{           0LL,     1000000LL }, // 10**7
	{           0LL,    10000000LL }, // 10**8
	{           0LL,   100000000LL }, // 10**9
	{           0LL,  1000000000LL }, // 10**10
	{           0LL, 10000000000LL }, // 10**11
	{           2LL, 31280523264LL }, // 10**12
	{          29LL,  3567587328LL }, // 10**13
	{         291LL,  1316134912LL }, // 10**14
	{        2910LL, 13161349120LL }, // 10**15
	{       29103LL, 28534276096LL }, // 10**16
	{      291038LL, 10464854016LL }, // 10**17
	{     2910383LL,  1569325056LL }, // 10**18
	{    29103830LL, 15693250560LL }, // 10**19
	{   291038304LL, 19493552128LL }, // 10**20
	{  2910383045LL, 23136829440LL }, // 10**21
	{ 29103830456LL, 25209864192LL }, // 10**22
};

static int36 ext_Inst;    // EXTEND Instruction Code
static int   ext_opCode;  //   Opcode field
static int   ext_opAC;    //   Accumulator field
static int   ext_pfFlags; // Flags for page faults

// PXCT switches for all EXTEND instructions
static int xsrc_dataMode;
static int xsrc_eaMode;
static int xdst_dataMode;
static int xdst_eaMode;

static int36 *ac0, *ac1, *ac2;
static int36 *ac3, *ac4, *ac5;
static int30 e0, e1;
static int36 d0, d1;
static int36 f1, f2;
static int36 b1, b2;
static int36 s2;

static int18 ext_Offset; // Offset Register

static int dataMode;

extern int30 (*KX10_CalcEffAddr)(int30, int30, int);

// Increment a byte pointer
void ext_IncByte(int36 *bp)
{
	int36 P; // Position field
	int36 S; // Size field

	// Extract position and size fields from a byte pointer.
	P = BP_GETPOS(*bp);
	S = BP_GETSIZE(*bp);

	// Increment a byte pointer
	if ((P -= S) < 0) {
		*bp = LH(*bp) | RH(*bp + 1);
		P = (36 - S) & BP_M_POS;
	}

	*bp = (P << BP_P_POS) | (*bp & ~BP_POS);
	*bp = SXT36(*bp);
}

// Decrement a byte pointer (page fail trap routines)
void ext_DecByte(int36 *bp)
{
	int36 P; // Position field
	int36 S; // Size field

	// Extract position and size fields from a byte pointer.
	P = BP_GETPOS(*bp);
	S = BP_GETSIZE(*bp);

	// Move its position back
	if ((P += S) > 36) {
		*bp = LH(*bp) | RH(*bp - 1);
		P = S;
	}

	// Update a byte pointer
	*bp = (P << BP_P_POS) | (*bp & ~BP_POS);
	*bp = SXT36(*bp);
}

// Increment and load byte
void ext_IncLoadByte(int36 *bp, int36 *data)
{
	int36 cbp = *bp; // Current Byte Pointer
	int36 P;         // Position field
	int36 S;         // Size field
	int30 eAddr;
	int36 data36;

	// Extract position and size fields from a byte pointer.
	P = BP_GETPOS(cbp);
	S = BP_GETSIZE(cbp);

	// Increment a byte pointer
	if ((P -= S) < 0) {
		cbp = LH(cbp) | RH(cbp + 1);
		P = (36 - S) & BP_M_POS;
	}
	cbp = (P << BP_P_POS) | (cbp & ~BP_POS);
	*bp = SXT36(cbp);

	// Get a 36-bit word where a byte pointer points to.
	eAddr = KX10_CalcEffAddr(PC, cbp, xsrc_eaMode);
	data36 = p10_vRead(eAddr, xsrc_dataMode);
	
	// Extract a byte from a 36-bit word.
	*data = ((data36 & WORD36_ONES) >> P) & ((1LL << (S > 36 ? 36 : S)) - 1);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("STR: %06o/ %012llo => %llo (Pos = %Ld Size = %Ld)\n",
			eAddr, data36 & WORD36_ONES, *data & WORD36_ONES, P, S);
#endif /* DEBUG */
}

// Increment and store byte
void ext_IncStoreByte(int36 *bp, int36 data)
{
	int36 cbp = *bp; // Current Byte Pointer
	int36 P;         // Position field
	int36 S;         // Size field
	int36 Mask;
	int30 eAddr;
	int36 data36;

	// Extract position and size fields from a byte pointer.
	P = BP_GETPOS(cbp);
	S = BP_GETSIZE(cbp);

	// Increment a byte pointer
	if ((P -= S) < 0) {
		cbp = LH(cbp) | RH(cbp + 1);
		P = (36 - S) & BP_M_POS;
	}
	cbp = (P << BP_P_POS) | (cbp & ~BP_POS);
	*bp = SXT36(cbp);

	// Get a 36-bit word where a byte pointer points to.
	eAddr = KX10_CalcEffAddr(PC, cbp, xdst_eaMode);
	data36 = p10_vRead(eAddr, xdst_dataMode);
	
	// Deposit a byte into a 36-bit word.
	Mask   = ((1LL << (S > 36 ? 36 : S)) - 1) << P;
	data36 = ((data << P) & Mask) | (data36 & ~Mask);

	// Update a 36-bit word back to its location.
	p10_vWrite(eAddr, data36, xdst_dataMode);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("STR: %06o/ %012llo <= %llo (Pos = %Ld Size = %Ld)\n",
			eAddr, data36 & WORD36_ONES, data, P, S);
#endif /* DEBUG */
}

// Fill space
void ext_FillSpace(int36 *ac0, int36 *ac1, int36 fill, int36 count)
{
	while (count--) {
		ext_IncStoreByte(ac1, fill);
		*ac0 = (*ac0 & STR_FLG_XMASK) | ((*ac0 - 1) & STR_LEN_MASK);
	}
}

// Translate byte by using a translation table
int36 ext_Translate(int30 tblAddr, int36 xData, int36 *xFlags)
{
	int36 tblEntry;
	int18 tblCode;

	tblEntry = p10_vRead(tblAddr + (xData >> 1), PXCT_CUR);
	tblEntry = (xData & 1) ? RH(tblEntry) : LHSR(tblEntry);
	tblCode  = XLATE_OPCODE(tblEntry);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("XLATE: Code = %d Data = %06llo\n",
			tblCode, tblEntry & XLATE_M_BYTE);
	}
#endif /* DEBUG */

	switch(tblCode) {
		case 00:
			return (*xFlags & STR_FLG_SIGN) ? tblEntry : xData;

		case 01:
			return -1;

		case 02:
			*xFlags &= ~STR_FLG_MINUS;
			return (*xFlags & STR_FLG_SIGN) ? tblEntry : xData;

		case 03:
			*xFlags |= STR_FLG_MINUS;
			return (*xFlags & STR_FLG_SIGN) ? tblEntry : xData;

		case 04:
			*xFlags |= STR_FLG_SIGN|STR_FLG_NZERO;
			return tblEntry;

		case 05:
			*xFlags |= STR_FLG_NZERO;
			return -1;

		case 06:
			*xFlags = (*xFlags | STR_FLG_SIGN|STR_FLG_NZERO) & ~STR_FLG_MINUS;
			return tblEntry;

		case 07:
			*xFlags |= STR_FLG_SIGN|STR_FLG_NZERO|STR_FLG_MINUS;
			return tblEntry;
	}
}

// During page fault routines, accumulators need being cleaned up.
void ext_Cleanup(void)
{
	// Move a source byte pointer back.
	if (ext_pfFlags & EXT_EDSRC)
		ext_DecByte(ac1);

	// Move a destination byte pointer back.
	if (ext_pfFlags & EXT_EDDST)
		ext_DecByte(ac4);
}

// 000 EUUO   - EUUO Instruction

int p10_extOpcode_EUUO(void)
{
	return EXT_MUUO;
}

// 001 CMPSL  - Compare Strings and Skip if less than
// 002 CMPSE  - Compare Strings and Skip if equal to
// 003 CMPSLE - Compare Strings and Skip if less than or equal to
// 005 CMPSGE - Compare Strings and Skip if greater than or equal to
// 006 CMPSN  - Compare Strings and Skip if not equal to
// 007 CMPSG  - Compare Strings and Skip if greater than

int p10_extOpcode_CMPS(void)
{
	int rc;

	// AC field must be zero. Otherwise,
	// execute an UUO instruction instead.
	if (ext_opAC)
		return EXT_MUUO;

	// Get string lengths from AC #0 and AC #3
	ac0 = &curAC[opAC];
	ac3 = &curAC[AC(opAC + 3)];

	if ((*ac0 | *ac3) & STR_MBZ)
		return EXT_MUUO;

	// Get byte pointers
	ac1 = &curAC[AC(opAC + 1)];
	ac4 = &curAC[AC(opAC + 4)];

	// Get two fills after CMPSx instruction
	f1 = p10_vRead((e0 + 1) & VMA_MASK, PXCT_CUR);
	f2 = p10_vRead((e0 + 2) & VMA_MASK, PXCT_CUR);
	f1 &= (1LL << BP_GETSIZE(*ac1)) - 1;
	f2 &= (1LL << BP_GETSIZE(*ac4)) - 1;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("CMPS: Instruction (Opcode %03o) at PC %06o\n",
			ext_opCode, pager_PC);
		dbg_Printf("CMPS: AC0 = %c%012llo  AC1 = %c%012llo\n",
			((*ac0 < 0) ? '-' : '+'), *ac0 & WORD36_ONES,
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES);
		dbg_Printf("CMPS: AC3 = %c%012llo  AC4 = %c%012llo\n",
			((*ac3 < 0) ? '-' : '+'), *ac3 & WORD36_ONES,
			((*ac4 < 0) ? '-' : '+'), *ac4 & WORD36_ONES);
		dbg_Printf("CMPS: F1 = %012llo, F2 = %012llo\n", f1, f2);
	}
#endif /* DEBUG */

	b1 = b2 = 0;
	while ((*ac0 | *ac3) && (b1 == b2)) {
		ext_pfFlags = 0;

		// Get a byte from 1st string pointer.
		if (*ac0) {
			ext_pfFlags |= EXT_EDSRC;
			ext_IncLoadByte(ac1, &b1);
		} else
			b1 = f1;

		// Get a byte from 2nd string pointer.
		if (*ac3) {
			ext_pfFlags |= EXT_EDDST;
			ext_IncLoadByte(ac4, &b2);
		} else
			b2 = f2;

		// Decrement both string length
		if (*ac0) (*ac0)--;
		if (*ac3) (*ac3)--;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("STR: B1 = %012llo  B2 = %012llo\n", b1, b2);
#endif /* DEBUG */

	switch (ext_opCode) {
		case EXT_CMPSL:  return (b1 < b2)  ? EXT_SKIP : EXT_NOSKIP;
		case EXT_CMPSE:  return (b1 == b2) ? EXT_SKIP : EXT_NOSKIP;
		case EXT_CMPSLE: return (b1 <= b2) ? EXT_SKIP : EXT_NOSKIP;
		case EXT_CMPSGE: return (b1 >= b2) ? EXT_SKIP : EXT_NOSKIP;
		case EXT_CMPSN:  return (b1 != b2) ? EXT_SKIP : EXT_NOSKIP;
		case EXT_CMPSG:  return (b1 > b2)  ? EXT_SKIP : EXT_NOSKIP;
	}

	return EXT_MUUO;
}

// 004 EDIT   - Edit String

int p10_extOpcode_EDIT(void)
{
	int36 ppi, pat;
	int30 tblAddr;
	int36 tblEntry;
	int18 tblCode;
	int36 ext_Flags;
	int   rc;

	// AC field must be zero. Otherwise,
	// execute an UUO instruction instead.
	if (ext_opAC)
		return EXT_MUUO;

	// Get Accumulator #0
	ac0 = &curAC[opAC];        // Pattern String Address

	if (*ac0 & EDIT_MBZ)
		return EXT_MUUO;

	// Get rest of accumulators
	ac1 = &curAC[AC(opAC + 1)]; // Source Byte Pointer
	ac3 = &curAC[AC(opAC + 3)]; // Mark Address
	ac4 = &curAC[AC(opAC + 4)]; // Destination Byte Pointer

	// Get the translation address and flags
	e1 = KX10_CalcEffAddr(PC, ext_Inst, cpu_pFlags & PXCT_EA);
	ext_Flags = *ac0 & STR_FLG_MASK;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("EDIT: Instruction (Opcode %03o) at PC %06o\n",
			ext_opCode, pager_PC);
		dbg_Printf("EDIT: AC0 = %c%012llo  AC1 = %c%012llo\n",
			((*ac0 < 0) ? '-' : '+'), *ac0 & WORD36_ONES,
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES);
		dbg_Printf("EDIT: AC3 = %c%012llo  AC4 = %c%012llo\n",
			((*ac3 < 0) ? '-' : '+'), *ac3 & WORD36_ONES,
			((*ac4 < 0) ? '-' : '+'), *ac4 & WORD36_ONES);
		dbg_Printf("EDIT: Translation Address = %06llo\n", e1);
	}
#endif /* DEBUG */

	rc = -1;
	while (rc < 0) {
		ext_pfFlags = 0;
		b1 = p10_vRead(VMA(*ac0), dataMode);
		ppi = EDIT_PBYTEN(*ac0);
		pat = EDIT_PBYTE(b1, ppi);

		switch ((pat < 0100) ? pat : pat & EDIT_OP_MASK) {
			case EDIT_STOP:      // 000 - Stop Edit
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA))
					dbg_Printf("EDIT: STOP   - Opcode %03llo at PC %06llo (%Ld)\n",
						pat, VMA(*ac0), ppi);
#endif /* DEBUG */
				rc = EXT_SKIP;
				break;

			case EDIT_SELECT:    // 001 - Select Next Source Byte
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA))
					dbg_Printf("EDIT: SELECT - Opcode %03llo at PC %06llo (%Ld)\n",
						pat, VMA(*ac0), ppi);
#endif /* DEBUG */
				ext_pfFlags |= EXT_EDSRC;
				ext_IncLoadByte(ac1, &b1);

				f1 = p10_vRead(e1 + (b1 >> 1), PXCT_CUR);
				tblEntry = (b1 & 1) ? RH(f1) : LHSR(f1);
				tblCode  = XLATE_OPCODE(tblEntry);

#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA)) {
					dbg_Printf("XLATE: Code = %d Data = %06llo\n",
					tblCode, tblEntry & XLATE_M_BYTE);
				}
#endif /* DEBUG */

				if (tblCode & 2)
					ext_Flags = (tblCode & 1) ?
						(ext_Flags | STR_FLG_MINUS) :
						(ext_Flags & ~STR_FLG_MINUS);

				switch (tblCode) {
					case 0:
					case 2:
					case 3:
						if ((ext_Flags & STR_FLG_SIGN) == 0) {
							f1 = p10_vRead(VMA(e0 + 1), dataMode);
							if (f1 == 0)
								break;
							tblEntry = f1;
						}
						tblEntry &= XLATE_M_BYTE;
						ext_pfFlags |= EXT_EDDST;
						ext_IncStoreByte(ac4, tblEntry);
						break;

					case 1:
						rc = EXT_NOSKIP;
						break;

					case 4:
					case 6:
					case 7:
						ext_Flags |= STR_FLG_NZERO;
						if ((ext_Flags & STR_FLG_SIGN) == 0) {
							ext_Flags |= STR_FLG_SIGN;
							f2 = p10_vRead(VMA(e0 + 2), dataMode);
							p10_vWrite(*ac3, *ac4, dataMode);
							if (f2) {
								ext_pfFlags |= EXT_EDDST;
								ext_IncStoreByte(ac4, f2);
							}
						}
						tblEntry &= XLATE_M_BYTE;
						ext_pfFlags |= EXT_EDDST;
						ext_IncStoreByte(ac4, tblEntry);
						break;

					case 5:
						ext_Flags |= STR_FLG_NZERO;
						rc = EXT_NOSKIP;
						break;
				}
				break;

			case EDIT_SIGST:     // 002 - Start Significance
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA))
					dbg_Printf("EDIT: SIGST  - Opcode %03llo at PC %06llo (%Ld)\n",
						pat, VMA(*ac0), ppi);
#endif /* DEBUG */
				if ((ext_Flags & STR_FLG_SIGN) == 0) {
					ext_Flags |= STR_FLG_SIGN;
					p10_vWrite(*ac3, *ac4, dataMode);
					f2 = p10_vRead(e0+2, dataMode);
					if (f2) {
						ext_pfFlags |= EXT_EDDST;
						ext_IncStoreByte(ac4, f2);
					}
				}
				break;

			case EDIT_FLDSEP:    // 003 - Separate Fields
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA)) {
					dbg_Printf("EDIT: FLDSEP - Opcode %03llo at PC %06llo (%lld)\n",
						pat, VMA(*ac0), ppi);
					dbg_Printf("EDIT:   All flags are clear.\n");
				}
#endif /* DEBUG */
				ext_Flags = 0;
				break;

			case EDIT_EXCHMD:    // 004 - Exchange Mark and Destination Pointers
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA))
					dbg_Printf("EDIT: EXCHMD - Opcode %03llo at PC %06llo (%lld)\n",
						pat, VMA(*ac0), ppi);
#endif /* DEBUG */
				f2 = p10_vRead(*ac3, dataMode);
				p10_vWrite(*ac3, *ac4, dataMode);
				*ac4 = f2;
				break;

			case EDIT_NOP:			// 005 - No-operation - Do nothing
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA))
					dbg_Printf("EDIT: NOP    - Opcode %03llo at PC %06llo (%lld)\n",
						pat, VMA(*ac0), ppi);
#endif /* DEBUG */
				break;

			case EDIT_MESSAG:    // 1XX - Insert Message Character
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA))
					dbg_Printf("EDIT: MESSAG - Opcode %03llo at PC %06llo (%lld)\n",
						pat, VMA(*ac0), ppi);
#endif /* DEBUG */
				if (ext_Flags & STR_FLG_SIGN)
					f1 = p10_vRead(e0 + (pat & EDIT_NUM_MASK) + 1, dataMode);
				else {
					f1 = p10_vRead(e0 + 1, dataMode);
					if (f1 == 0)
						break;
				}
				ext_pfFlags |= EXT_EDDST;
				ext_IncStoreByte(ac4, f1);
				break;

			case EDIT_SKPM:      // 5XX - Skip on M flag.
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA)) {
					dbg_Printf("EDIT: SKPM   - Opcode %03llo at PC %06llo (%lld)\n",
						pat, VMA(*ac0), ppi);
					if (ext_Flags & STR_FLG_MINUS)
						dbg_Printf("EDIT:   M flag is set - %d Opcode(s) skipped.\n",
							(pat & EDIT_NUM_MASK) + 1);
					else
						dbg_Printf("EDIT:   M flag is clear - Not skipped.\n");
				}
#endif /* DEBUG */
				if (ext_Flags & STR_FLG_MINUS)
					ppi += (pat & EDIT_NUM_MASK) + 1;
				break;

			case EDIT_SKPN:      // 6XX - Skip on N flag.
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA)) {
					dbg_Printf("EDIT: SKPN   - Opcode %03llo at PC %06llo (%lld)\n",
						pat, VMA(*ac0), ppi);
					if (ext_Flags & STR_FLG_NZERO)
						dbg_Printf("EDIT:   N flag is set - %d Opcode(s) skipped.\n",
							(pat & EDIT_NUM_MASK) + 1);
					else
						dbg_Printf("EDIT:   N flag is clear - Not skipped.\n");
				}
#endif /* DEBUG */
				if (ext_Flags & STR_FLG_NZERO)
					ppi += (pat & EDIT_NUM_MASK) + 1;
				break;

			case EDIT_SKPA:      // 7XX - Skip Always
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA)) {
					dbg_Printf("EDIT: SKPA   - Opcode %03llo at PC %06llo (%lld)\n",
						pat, VMA(*ac0), ppi);
					dbg_Printf("EDIT:   %d Opcode(s) skipped.\n",
						(pat & EDIT_NUM_MASK) + 1);
				}
#endif /* DEBUG */
				ppi += (pat & EDIT_NUM_MASK) + 1;
				break;

			default:             // XXX - Undefined opcode - Do nothing
#ifdef DEBUG
				if (dbg_Check(DBG_TRACE|DBG_DATA))
					dbg_Printf("EDIT: Undefined opcode %03llo at PC %06llo (%lld)\n",
						pat, VMA(*ac0), ppi);
#endif /* DEBUG */
				break;
		}

		// Update pattern byte address and flags
		ppi++;
		*ac0 = VMA(*ac0 + (ppi >> 2));
		*ac0 |= ext_Flags | ((ppi & EDIT_PBN_MASK) << EDIT_PBN_POS);
		*ac0 = SXT36(*ac0);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("EDIT: Done - %sSKIP was executed.\n",
			(rc == EXT_NOSKIP) ? "No " : "");
#endif /* DEBUG */

	return rc;
}

// 010 CVTDBO - Convert Decimal to Binary Offset
// 011 CVTDBT - Convert Decimal to Binary Translated
// 012 CVTBDO - Convert Binary to Decimal Offset
// 013 CVTBDT - Convert Binary to Decimal Translated

int p10_extOpcode_CVTDB(void)
{
	int36 ext_Flags;
	int   rc = EXT_SKIP;

	// AC field must be zero. Otherwise,
	// execute an UUO instruction instead.
	if (ext_opAC)
		return EXT_MUUO;

	ac0 = &curAC[opAC];
	ac1 = &curAC[AC(opAC + 1)];
	ac3 = &curAC[AC(opAC + 3)];
	ac4 = &curAC[AC(opAC + 4)];

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("CVTDB: Instruction (Opcode %03o) at PC %06o\n",
			ext_opCode, pager_PC);
		dbg_Printf("CVTDB: AC0 = %c%012llo  AC1 = %c%012llo\n",
			((*ac0 < 0) ? '-' : '+'), *ac0 & WORD36_ONES,
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES);
		dbg_Printf("CVTDB: AC3 = %c%012llo  AC4 = %c%012llo\n",
			((*ac3 < 0) ? '-' : '+'), *ac3 & WORD36_ONES,
			((*ac4 < 0) ? '-' : '+'), *ac4 & WORD36_ONES);
	}
#endif /* DEBUG */

	// If SIGN flag is clear, clear AC3 and AC4.
	if ((*ac0 & STR_FLG_SIGN) == 0)
		*ac3 = *ac4 = 0;
		
	// Either get a 18-bit signed offset if CVTDBO instruction
	// or get an address of translation table.
	e1 = KX10_CalcEffAddr(PC, ext_Inst, cpu_pFlags & PXCT_EA);
	if (ext_opCode == EXT_CVTDBO) {
		*ac0 |= STR_FLG_SIGN;
		*ac0 = SXT36(*ac0);
		ext_Offset = SXT18(e1);
	}

	ext_Flags = (*ac0 & STR_FLG_MASK);
	*ac4 &= WORD36_MAXP;
	while (*ac0 & STR_LEN_MASK) {
		ext_pfFlags |= EXT_EDSRC;
		ext_IncLoadByte(ac1, &b1);

		if (ext_opCode == EXT_CVTDBO)
			// With using offset.
			b1 = (b1 + ext_Offset) & WORD36_ONES;
		else {
			// With using translation table.
			if ((b1 = ext_Translate(e1, b1, &ext_Flags)) >= 0)
				b1 = (ext_Flags & STR_FLG_SIGN) ? (b1 & XLATE_M_DIGIT) : 0;
		}

		// Decrement AC0 by one and update AC1 for a byte pointer.
		*ac0 = ext_Flags | ((*ac0 - 1) & STR_LEN_MASK);
		*ac0 = SXT36(*ac0);

		// Check digit first if it is valid between 0 and 9.
		if ((b1 < 0) || (b1 > 9))
			rc = EXT_NOSKIP;

		if (rc == EXT_NOSKIP)
			break;

		// Append a digit to a double binary integer.
		*ac4 = (*ac4 * 10) + b1;
		*ac3 = ((*ac3 * 10) + (*ac4 >> 35)) & WORD36_ONES;
		*ac4 &= WORD36_MAXP;
	}

	if (rc == EXT_SKIP) {
		// If M flag is set, negate it.
		if (*ac0 & STR_FLG_MINUS) {
			*ac4 = -*ac4 & WORD36_MAXP;
			*ac3 = ~*ac3 + (*ac4 == 0);
		}
	}

	// Fix sign bits on accumulator #3 and #4
	if (*ac3 & WORD36_SIGN) {
		*ac3 |= WORD36_XSIGN;
		*ac4 |= WORD36_XSIGN;
	} else {
		*ac3 &= WORD36_MAXP;
		*ac4 &= WORD36_MAXP;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("CVTDB: -- Final --\n");
		dbg_Printf("CVTDB: AC0 = %c%012llo  AC1 = %c%012llo\n",
			((*ac0 < 0) ? '-' : '+'), *ac0 & WORD36_ONES,
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES);
		dbg_Printf("CVTDB: AC3 = %c%012llo  AC4 = %c%012llo\n",
			((*ac3 < 0) ? '-' : '+'), *ac3 & WORD36_ONES,
			((*ac4 < 0) ? '-' : '+'), *ac4 & WORD36_ONES);
		dbg_Printf("CVTDB: Next instruction: %s.\n",
			(rc == EXT_SKIP) ? "Skipped" : "Not skipped");
	}
#endif /* DEBUG */

	return rc;
}

int p10_extOpcode_CVTBD(void)
{
	int   nDigits;

	// AC field must be zero. Otherwise,
	// execute an UUO instruction instead.
	if (ext_opAC)
		return EXT_MUUO;

	// Either get a 18-bit signed offset if CVTDBO instruction
	// or get an address of translation table.
	e1 = KX10_CalcEffAddr(PC, ext_Inst, cpu_pFlags & PXCT_EA);
	if (ext_opCode == EXT_CVTDBO)
		ext_Offset = SXT18(e1);

	// Get a double binary integer from AC and AC+1.
	ac0 = &curAC[opAC];
	ac1 = &curAC[AC(opAC + 1)];

	// Get string length and flags from AC+3 and
	// a byte pointer from AC+4.
	ac3 = &curAC[AC(opAC + 3)];
	ac4 = &curAC[AC(opAC + 4)];

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("CVTBD: AC0 = %c%012llo  AC1 = %c%012llo\n",
			((*ac0 < 0) ? '-' : '+'), *ac0 & WORD36_ONES,
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES);
		dbg_Printf("CVTBD: AC3 = %c%012llo  AC4 = %c%012llo\n",
			((*ac3 < 0) ? '-' : '+'), *ac3 & WORD36_ONES,
			((*ac4 < 0) ? '-' : '+'), *ac4 & WORD36_ONES);
		if (ext_opCode == EXT_CVTBDO)
			dbg_Printf("CVTBD: With using offset %c%06o.\n",
				((ext_Offset < 0) ? '-' : '+'), ext_Offset & WORD18_ONES);
		else
			dbg_Printf("CVTBD: With using the translation table.\n");
	}
#endif /* DEBUG */

	d0 = *ac0;
	d1 = *ac1;

	if ((FLAGS & FLG_FPD) == 0) {
		if (d0 & WORD36_SIGN) {
			d0 = -d0;
			d1 = ~d1 + (d0 == 0);
		}

		// Find a number of digits by using the power of 10 table.
		for (nDigits = 22; nDigits > 1; nDigits--)
			if ((d0 > ext_Powers10[nDigits][0]) ||
			    ((d0 == ext_Powers10[nDigits][0]) &&
			     (d1 >= ext_Powers10[nDigits][1])))
				break;

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("CVTBD: Actual digits = %d, Requested digits = %d\n",
				nDigits, *ac3 & STR_LEN_MASK);
#endif /* DEBUG */

		// Check number of digits with string length in AC+3 first.
		if (nDigits > (*ac3 & STR_LEN_MASK)) {
			if (ISCPU(CNF_KL10)) {
				// For KL10 Processor, N and M flags must be set first in
				// order that processor can be identified as KL10 or KS10.
				*ac3 |= (STR_FLG_NZERO|STR_FLG_MINUS);
			}
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA))
				dbg_Printf("CVTBD: Too big binary integer - Aborted.\n");
#endif /* DEBUG */
			// Too large binary integer - Aborted.
			return EXT_NOSKIP;
		}

		// Check string length for smaller binary integer than expected.
		if ((nDigits < (*ac3 & STR_LEN_MASK)) && (*ac3 & STR_FLG_LEAD)) {
			// Fill leading spaces in left of string now.
			f1 = p10_vRead(RH(eAddr + 1), dataMode);
			ext_pfFlags |= EXT_EDDST;
			ext_FillSpace(ac3, ac4, f1, (*ac3 & STR_LEN_MASK) - nDigits);
		} else {
			// Update new string length (number of digits) in AC+3.
			*ac3 = (*ac3 & STR_FLG_XMASK) | (nDigits & STR_LEN_MASK);
		}

		// If a double integer is not zero, set N flag.
		if (*ac0 | *ac1)
			*ac3 |= STR_FLG_NZERO;

		// If a double integer is negative, set M flag.
		if (*ac0 & WORD36_SIGN)
			*ac3 |= STR_FLG_MINUS;

		// Update them to accumlators and set FPD flag to
		// avoid updates twice or more due to page faults.
		*ac0 = d0;
		*ac1 = d1;
		FLAGS |= FLG_FPD;
	}

	// Now convert from binary to decimal.
	while (*ac3 & STR_LEN_MASK) {
		int36 digit;

		// Get a number of digits and
		// set limit to maximum 22 digits.
		nDigits = *ac3 & STR_LEN_MASK;
		if (nDigits > 22)
			nDigits = 22;

		for (digit = 0; digit < 10; digit++) {
			if ((d0 > ext_Powers10[nDigits][0]) ||
			    ((d0 == ext_Powers10[nDigits][0]) &&
			     (d1 >= ext_Powers10[nDigits][1]))) {
				d0 = d0 - ext_Powers10[nDigits][0] -
					(d1 < ext_Powers10[nDigits][1]);
				d1 = (d1 - ext_Powers10[nDigits][1]) & WORD36_MAXP;	
			} else
				break;
		}

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("CVTBD: AC0 = %012llo AC1 = %012llo Digit = %lld\n",
				d0 & WORD36_ONES, d1 & WORD36_ONES, digit);
#endif /* DEBUG */

		if (ext_opCode == EXT_CVTBDO) {
			// CVTBDO Instruction - Offset
			digit = (digit + ext_Offset) & WORD36_ONES;
		} else {
			// CVTBDT Instruction - Translation Table
			f1 = p10_vRead(e1 + digit, dataMode);
			digit = (((nDigits == 1) && (*ac3 & STR_FLG_LEAD)) ? f1 >> 18 : f1)
				& WORD18_ONES;
		}

		ext_pfFlags |= EXT_EDDST;
		ext_IncStoreByte(ac4, digit);

		*ac0 = d0;
		*ac1 = d1;
		*ac3 = (*ac3 & STR_FLG_XMASK) | ((*ac3 - 1) & STR_LEN_MASK);
	}

	// When the conversion was finished, clear FPD (First Part Done) flag.
	FLAGS &= ~FLG_FPD;
	return EXT_SKIP;
}

// 0014 MOVSO  - Move String Offset
// 0015 MOVST  - Move String Translated
// 0016 MOVSLJ - Move String Left Justified
// 0017 MOVSRJ - Move String Right Justified

int p10_extOpcode_MOVS(void)
{
	int36 ext_Flags = 0;

	// AC field must be zero. Otherwise,
	// execute an UUO instruction instead.
	if (ext_opAC)
		return EXT_MUUO;

	// Get all accumulators.
	ac0 = &curAC[opAC];
	ac1 = &curAC[AC(opAC + 1)];
	ac3 = &curAC[AC(opAC + 3)];
	ac4 = &curAC[AC(opAC + 4)];

	// Make sure AC3 must have MBZ bits.
	// If not, execute as an UUO instruction.
	if (*ac3 & STR_MBZ)
		return EXT_MUUO;

	f1 = p10_vRead(RH(e0 + 1), dataMode);
	f1 &= WORD36_ONES;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("MOVS: Instruction (Opcode %03o) at PC %06o\n",
			ext_opCode, pager_PC);
		dbg_Printf("MOVS: AC0 = %c%012llo  AC1 = %c%012llo\n",
			((*ac0 < 0) ? '-' : '+'), *ac0 & WORD36_ONES,
			((*ac1 < 0) ? '-' : '+'), *ac1 & WORD36_ONES);
		dbg_Printf("MOVS: AC3 = %c%012llo  AC4 = %c%012llo\n",
			((*ac3 < 0) ? '-' : '+'), *ac3 & WORD36_ONES,
			((*ac4 < 0) ? '-' : '+'), *ac4 & WORD36_ONES);
		dbg_Printf("MOVS: F1  = %012llo\n", f1);
	}
#endif /* DEBUG */

	switch(ext_opCode) {
		case EXT_MOVSO:
			ext_Offset = KX10_CalcEffAddr(PC, ext_Inst, cpu_pFlags & PXCT_EA);
			ext_Offset = SXT18(ext_Offset);
			s2 = BP_GETSIZE(*ac4);
			if (s2 > 36)
				s2 = 36;
			*ac0 &= STR_LEN_MASK;
			break;

		case EXT_MOVST:
			e1 = KX10_CalcEffAddr(PC, ext_Inst, cpu_pFlags & PXCT_EA);
			ext_Flags = *ac0 & STR_FLG_MASK;
			break;

		case EXT_MOVSLJ:
			*ac0 &= STR_LEN_MASK;
			break;

		case EXT_MOVSRJ:
			*ac0 &= STR_LEN_MASK;
			if (*ac3 == 0)
				return *ac0 ? EXT_NOSKIP : EXT_SKIP;

			// Make a destination string right justified.
			if (*ac0 > *ac3) {
				while (*ac0 > *ac3) {
					ext_IncByte(ac1);
					*ac0 = (*ac0 - 1) & STR_LEN_MASK;
				}
			} else if (*ac0 < *ac3) {
				ext_pfFlags |= EXT_EDDST;
				ext_FillSpace(ac3, ac4, f1, *ac3 - *ac0);
			}
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA))
				dbg_Printf("MOVS: Unknown opcode %03o at PC %06o\n",
					ext_opCode, pager_PC);
#endif /* DEBUG */
			return EXT_MUUO;
	}

	if (*ac3) {
		while (*ac3 & STR_LEN_MASK) {
			ext_pfFlags = 0;
			if (*ac0 & STR_LEN_MASK) {
				ext_pfFlags |= EXT_EDSRC;
				ext_IncLoadByte(ac1, &b1);

				if (ext_opCode == EXT_MOVSO) {
					b1 = (b1 + ext_Offset) & WORD36_ONES;
					if (b1 & ~((1LL << s2) - 1)) {
						*ac0 = ext_Flags | ((*ac0 - 1) & STR_LEN_MASK);
						*ac0 = SXT36(*ac0);
						return EXT_NOSKIP;
					}
				} else if (ext_opCode == EXT_MOVST) {
					if ((b1 = ext_Translate(e1, b1, &ext_Flags)) < 0) {
						*ac0 = ext_Flags | ((*ac0 - 1) & STR_LEN_MASK);
						*ac0 = SXT36(*ac0);
						return EXT_NOSKIP;
					}
					b1 = (ext_Flags & STR_FLG_SIGN) ? (b1 & XLATE_M_BYTE) : -1;
				}
			} else
				b1 = f1;

			if (b1 >= 0) {
				ext_pfFlags |= EXT_EDDST;
				ext_IncStoreByte(ac4, b1);

				*ac3 = (*ac3 - 1) & STR_LEN_MASK;
			}

			if (*ac0 & STR_LEN_MASK) {
				*ac0 = ext_Flags | ((*ac0 - 1) & STR_LEN_MASK);
				*ac0 = SXT36(*ac0);
			}
		}
	}

	return (*ac0 & STR_LEN_MASK) ? EXT_NOSKIP : EXT_SKIP;
}

// 0123 EXTEND - Extended Instruction

void p10_Opcode_EXTEND(void)
{
	int rc;

	dataMode = p10_CheckPXCT(PXCT_DATA);
	if (xsrc_dataMode = p10_CheckPXCT(PXCT_XSRC))
		xsrc_eaMode = p10_CheckPXCT(PXCT_EA);
	if (xdst_dataMode = p10_CheckPXCT(PXCT_XDST))
		xdst_eaMode = p10_CheckPXCT(PXCT_EA);

	ext_Inst = p10_vRead(eAddr, dataMode);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE))
		p10_Disassemble(eAddr, ext_Inst, OP_EXT);
#endif /* DEBUG */

	ext_opCode = INST_GETOP(ext_Inst);
	ext_opAC   = INST_GETAC(ext_Inst);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE))
		dbg_Printf("EXTEND: %03o at PC %06o\n", ext_opCode, PC);
#endif /* DEBUG */

	// Set callback pointer to execute
	// ext_Cleanup when page fault occurs.
	pager_Cleanup = ext_Cleanup;
	ext_pfFlags = 0;

	e0 = eAddr;
	rc = extOpcode[ext_opCode]();

	// Reset callback pointer.
	pager_Cleanup = NULL;

	switch (rc) {
		case EXT_MUUO:
			if (ISCPU(CNF_KS10))
				KS10_Opcode_UUO();
			if (ISCPU(CNF_KL10))
				KL10_Opcode_UUO();
			break;

		case EXT_SKIP:
			DO_SKIP;
			break;
	}
		
}

// **************** KL10 Processor **************

extern int KX10_IsGlobal;

// XBLT - Extended Block Transfer
// According to KLX microcode source codes, XBLT instruction is allowed
// in section zero area on KLX edition v262 or above.

int KL10_extOpcode_XBLT(void)
{
	// Get all accumulators.
	ac0 = &curAC[opAC];
	ac1 = &curAC[AC(opAC + 1)];
	ac2 = &curAC[AC(opAC + 2)];

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("XBLT: Instruction (Opcode %03o) at PC %06o\n",
			ext_opCode, pager_PC);
		dbg_Printf("XBLT: Start SA: %010llo -> DA: %010llo at %llo words by %s\n",
			PMA(*ac1), PMA(*ac2), ABS(*ac0), 
			(*ac0 < 0 ? "descending" : "ascending"));
	}
#endif /* DEBUG */

	KX10_IsGlobal = TRUE;

	if (*ac0 < 0) {
		// Descending (backward) transferring
		*ac1 += -*ac0;
		*ac2 += -*ac0;
		while (*ac0) {
			(*ac1)--;
			(*ac2)--;
			AR = p10_vRead(*ac1, PXCT_CUR);
			p10_vWrite(*ac2, AR, PXCT_CUR);
			(*ac0)++;
		}
	} else {
		// Ascending (forward) transferring
		while (*ac0) {
			AR = p10_vRead(*ac1, PXCT_CUR);
			p10_vWrite(*ac2, AR, PXCT_CUR);
			(*ac1)++;
			(*ac2)++;
			(*ac0)--;
		}
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("XBLT: Final SA: %010llo -> DA: %010llo\n",
			PMA(*ac1), PMA(*ac2));
#endif /* DEBUG */
	return EXT_NOSKIP;
}
