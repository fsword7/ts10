// cpu_string.c - VAX String Instructions
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

DEF_INST(vax, CMPC3)
{
	uint16 len    = OP0;
	int32  s1addr = OP1;
	int32  s2addr = OP2;
	int32  cc;
	int8   s1, s2;

	if (len > 0) {
		while (len > 0) {
			s1 = ReadV(s1addr, OP_BYTE, RA);
			s2 = ReadV(s2addr, OP_BYTE, RA);

			if (s1 != s2)
				break;

			len--;
			s1addr++;
			s2addr++;
		}

		cc =  (s1 < s2) ? CC_N : 0;
		cc |= (s1 == s2) ? CC_Z : 0;
		cc |= ((uint8)s1 < (uint8)s2) ? CC_C : 0;
	} else
		cc = CC_Z;

	// Update condition bits
	CC = cc;

	// Update registers
	R0 = len;
	R1 = s1addr;
	R2 = R0;
	R3 = s2addr;
}

DEF_INST(vax, CMPC5)
{
	uint16 s1len  = OP0;
	int32  s1addr = OP1;
	int8   fill   = OP2;
	uint16 s2len  = OP3;
	int32  s2addr = OP4;
	int32  cc;
	int8   s1, s2;

	if ((s1len | s2len) > 0) {
		while ((s1len | s2len) > 0) {
			s1 = (s1len > 0) ? ReadV(s1addr, OP_BYTE, RA) : fill;
			s2 = (s2len > 0) ? ReadV(s2addr, OP_BYTE, RA) : fill;

			if (s1 != s2)
				break;

			if (s1len > 0) {
				s1len--;
				s1addr++;
			}
			if (s2len > 0) {
				s2len--;
				s2addr++;
			}
		}

		cc =  (s1 < s2) ? CC_N : 0;
		cc |= (s1 == s2) ? CC_Z : 0;
		cc |= ((uint8)s1 < (uint8)s2) ? CC_C : 0;
	} else
		cc = CC_Z;

	// Update condition bits
	CC = cc;

	// Update registers
	R0 = s1len;
	R1 = s1addr;
	R2 = s2len;
	R3 = s2addr;
}

// 3A LOCC - Locate Character
DEF_INST(vax, LOCC)
{
	uint8  ch   = OP0;
	uint16 len  = OP1;
	int32  addr = OP2;
	uint8  fch;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("LOCC: Char=%02X (%c)  Addr=%08X  Len=%04X (%d)\n",
			ch, ((ch > ' ' && ch < 127) ? ch : '.'), addr, len, len);
#endif /* DEBUG */

	// Find a character in that string.
	while (len > 0) {
		fch = ReadV(addr, OP_BYTE, RA) & BMASK;
		if (ch == fch) {
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA))
				dbg_Printf("LOCC: Found at location %08X\n", addr);
#endif /* DEBUG */
			break;
		}
		len--;
		addr++;
	}

	// Update registers
	R0 = len;
	R1 = addr;

	// If string has a zero length, set Z bit.
	CC = (len ? 0 : CC_Z);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("LOCC: Len=%04X (%d)  Addr=%08X  CC: %s\n",
			len, len, addr,  CC_DSPL(CC));
#endif /* DEBUG */
}

// 28 MOVC3 - Move Character 3 Operand
DEF_INST(vax, MOVC3)
{
	uint16 len     = OP0;
	uint32 srcAddr = OP1;
	uint32 dstAddr = OP2;
	uint16 svlen;
	uint8  ch;

	if (srcAddr > dstAddr) {
		while (len > 0) {
			ch = ReadV(srcAddr, OP_BYTE, RA);
			WriteV(dstAddr, ch, OP_BYTE, WA);
			len--;
			srcAddr++;
			dstAddr++;
		}
		R1 = srcAddr;
		R3 = dstAddr;
	} else {
		svlen = len;
		srcAddr += len;
		dstAddr += len;
		while (len > 0) {
			len--;
			srcAddr--;
			dstAddr--;
			ch = ReadV(srcAddr, OP_BYTE, RA);
			WriteV(dstAddr, ch, OP_BYTE, WA);
		}
		R1 = srcAddr + svlen;
		R3 = dstAddr + svlen;
	}
	
	// Clear rest of registers
	R0 = 0;
	R2 = 0;
	R4 = 0;
	R5 = 0;

	// Update condition bits
	CC = CC_Z;
}

// 2C MOVC5 - Move Character 5 Operand
DEF_INST(vax, MOVC5)
{
	uint16 srcLen  = OP0;
	uint32 srcAddr = OP1;
	uint8  fill    = OP2;
	uint16 dstLen  = OP3;
	uint32 dstAddr = OP4;
	uint16 srcLen2 = srcLen;
	uint16 dstLen2 = dstLen;
	uint16 sv_srcLen, sv_dstLen;
	uint8  ch;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("MOVC5: SA %08X (%04X) => DA %08X (%04X)\n",
			srcAddr, srcLen, dstAddr, dstLen);
		dbg_Printf("MOVC5: Fill = %02X\n", fill);
	}
#endif /* DEBUG */


	if (srcAddr > dstAddr) {
		while ((srcLen > 0) && (dstLen > 0)) {
			ch = ReadV(srcAddr, OP_BYTE, RA);
			WriteV(dstAddr, ch, OP_BYTE, WA);
			srcLen--;
			srcAddr++;
			dstLen--;
			dstAddr++;
		}
		while (dstLen > 0) {
			WriteV(dstAddr, fill, OP_BYTE, WA);
			dstLen--;
			dstAddr++;
		}
		R1 = srcAddr;
		R3 = dstAddr;
	} else {
		sv_srcLen = (srcLen < dstLen) ? srcLen : dstLen;
		sv_dstLen = dstLen;
		srcAddr   += sv_srcLen;
		dstAddr   += sv_dstLen;
		while (dstLen > srcLen) {
			dstLen--;
			dstAddr--;
			WriteV(dstAddr, fill, OP_BYTE, WA);
		}
		while (dstLen > 0) {
			srcLen--;
			srcAddr--;
			dstLen--;
			dstAddr--;
			ch = ReadV(srcAddr, OP_BYTE, RA);
			WriteV(dstAddr, ch, OP_BYTE, WA);
		}
		R1 = srcAddr + sv_srcLen;
		R3 = dstAddr + sv_dstLen;
	}

	R0 = srcLen;
	R2 = 0;
	R4 = 0;
	R5 = 0;

	// Update condition codes
	CC_CMP_W((int16)srcLen2, (int16)dstLen2);
}

DEF_INST(vax, SCANC)
{
	uint16 len     = OP0;
	int32  addr    = OP1;
	int32  tbladdr = OP2;
	uint8  mask    = OP3;
	uint8  ch, tmp;

	// Scan Characters
	while (len > 0) {
		ch  = ReadV(addr, OP_BYTE, RA);
		tmp = ReadV(tbladdr + ch, OP_BYTE, RA);
		if (tmp & mask)
			break;
		len--;
		addr++;
	}

	// Update condition bits
	CC = (len == 0) ? CC_Z : 0;

	// Update registers
	R0 = len;
	R1 = addr;
	R2 = 0;
	R3 = tbladdr;
}

DEF_INST(vax, SKPC)
{
	uint8  ch   = OP0;
	uint16 len  = OP1;
	int32  addr = OP2;

	// Skip Character(s)
	while (len > 0) {
		if ((ReadV(addr, OP_BYTE, RA) & BMASK) != ch)
			break;
		len--;
		addr++;
	}

	// Update condition bits
	CC = (len == 0) ? CC_Z : 0;

	// Update registers
	R0 = len;
	R1 = addr;
}

DEF_INST(vax, SPANC)
{
	uint16 len     = OP0;
	int32  addr    = OP1;
	int32  tbladdr = OP2;
	uint8  mask    = OP3;
	uint8  ch, tmp;

	// Span Characters
	while (len > 0) {
		ch  = ReadV(addr, OP_BYTE, RA);
		tmp = ReadV(tbladdr + ch, OP_BYTE, RA);
		if ((tmp & mask) == 0)
			break;
		len--;
		addr++;
	}

	// Update condition bits
	CC = (len == 0) ? CC_Z : 0;

	// Update registers
	R0 = len;
	R1 = addr;
	R2 = 0;
	R3 = tbladdr;
}
