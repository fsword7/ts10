// cpu_queue.c - VAX Queue Instructions
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

// Due to lack of documentation about inner workings of queue 
// instructions so that I had quoted them from Bob's simh VAX
// emulator to pass all HCORE tests.  When I find complete
// specs about queue instructions, I will write my own code
// here.  That's why VAX Architecture Reference Handbook (First
// Edition) did not give much information.   I apologize for that.

DEF_INST(vax, INSQHI)
{
	int32 entry  = OP0;
	int32 header = OP1;
	int32 a, t;
	int32 at;

	// Header must be quadword aligned.
	// header and Entry must not be equal.
	// If so, go to reserved operand fault.
	if ((entry == header) || ((entry | header) & 0x07))
		RSVD_OPND_FAULT;

	ReadV(entry, OP_BYTE, WA);
	a = ReadV(header, OP_LONG, WA);

	if (a & 06)
		RSVD_OPND_FAULT;
	if (a & 01) {
		CC = CC_C;
	} else {
		WriteV(header, a | 1, OP_LONG, WA);
		a += header;
		if (TestV(a, WA, &t) < 0)
			WriteV(header, a - header, OP_LONG, WA);
		WriteV(a + 4, entry - a, OP_LONG, WA);
		WriteV(entry, a - entry, OP_LONG, WA);
		WriteV(entry + 4, header - entry, OP_LONG, WA);
		WriteV(header, entry - header, OP_LONG, WA);

		CC = (a == header) ? CC_Z : 0;
	}
}

DEF_INST(vax, INSQTI)
{
	int32 entry  = OP0;
	int32 header = OP1;
	int32 a, c, t;

	// Header must be quadword aligned.
	// header and Entry must not be equal.
	// If so, go to reserved operand fault.
	if ((header == entry) || ((header | entry) & 07))
		RSVD_OPND_FAULT;

	ReadV(entry, OP_BYTE, WA);
	a = ReadV(header, OP_LONG, WA);
	if (a == 0) {
		// Treat as INSQHI instruction.
		DEF_NAME(vax, INSQHI)(vax);
		return;
	}
	if (a & 06)
		RSVD_OPND_FAULT;
	if (a & 01) {
		// Busy, set CC as 0001 (---C)
		CC = CC_C;
	} else {
		WriteV(header, a | 1, OP_LONG, WA);
		c = ReadV(header + 4, OP_LONG, RA) + header;
		if (c & 07) {
			WriteV(header, a, OP_LONG, WA);
			RSVD_OPND_FAULT;
		}
		if (TestV(c, WA, &t) < 0)
			WriteV(header, a, OP_LONG, WA);
		WriteV(c, entry - c, OP_LONG, WA);
		WriteV(entry, header - entry, OP_LONG, WA);
		WriteV(entry + 4, c - entry, OP_LONG, WA);
		WriteV(header + 4, entry - header, OP_LONG, WA);
		WriteV(header, a, OP_LONG, WA);

		// Clear all condition codes
		CC = 0;
	}
}

// INSQUE - Insert Entry in Queue
//
// Format:
//   opcode entry.ab, pred.ab
//
// Operation:
//   if {all memory accesses can be completed} then
//     begin
//       (entry) <- (pred);       ! forward link of entry
//       (entry + 4) <- pred;     ! backward link of entry
//       ((pred) + 4) <- entry;   ! backward link of successor
//       (pred) <- entry;         ! forward link of predecessor
//     end

DEF_INST(vax, INSQUE)
{
	register int32 entry = OP0;
	register int32 pred  = OP1;
	register int32 succ;

	// Check memory accesses first
	succ = ReadV(pred, OP_LONG, WA);
	ReadV(succ + 4, OP_LONG, WA);
	ReadV(entry + 4, OP_LONG, WA);
	WriteV(entry, succ, OP_LONG, WA);
	WriteV(entry + 4, pred, OP_LONG, WA);
	WriteV(succ + 4, entry, OP_LONG, WA);
	WriteV(pred, entry, OP_LONG, WA);

	// Update condition codes
	CC_CMP_L(succ, pred);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("INSQUE: Enqueue %08X to %08X with next %08X\n",
			entry, pred, succ);
#endif /* DEBUG */
}

DEF_INST(vax, REMQHI)
{
	int32 header = OP0;
	int32 ar, a, b, t;

	// Header must be quadword aligned.
	// Also check address for write access and
	// must not be equal to header.
	if (header & 07)
		RSVD_OPND_FAULT;
	if (OP1 < 0) {
		if (OP2 == header)
			RSVD_OPND_FAULT;
		ReadV(OP2, OP_LONG, WA);
	}

	ar = ReadV(header, OP_LONG, WA);
	if (ar & 06)
		RSVD_OPND_FAULT;
	if (ar & 01) {
		CC = CC_V|CC_C;
	} else {
		a = ar + header;
		if (ar) {
			WriteV(header, ar | 1, OP_LONG, WA);
			if (TestV(a, RA, &t) < 0)
				WriteV(header, ar, OP_LONG, WA);
			b = ReadV(a, OP_LONG, RA) + a;
			if (b & 07) {
				WriteV(header, ar, OP_LONG, WA);
				RSVD_OPND_FAULT;
			}
			if (TestV(b, RA, &t) < 0)
				WriteV(header, ar, OP_LONG, WA);
			WriteV(b + OP_LONG, header - b, OP_LONG, WA);
			WriteV(header, b - header, OP_LONG, WA);
		}

		LSTORE(OP1, OP2, a);

		// Update condition codes
		if (ar == 0)
			CC = CC_Z|CC_V;
		else if (b == header)
			CC = CC_Z;
		else
			CC = 0;
	}
}

DEF_INST(vax, REMQTI)
{
	int32 header = OP0;
	int32 ar, b, c, t;

	// Header must be quadword aligned.
	// Also check address for write access and
	// must not be equal to header.
	if (header & 07)
		RSVD_OPND_FAULT;
	if (OP1 < 0) {
		if (header == OP2)
			RSVD_OPND_FAULT;
		ReadV(OP2, OP_LONG, WA);
	}

	ar = ReadV(header, OP_LONG, WA);
	if (ar & 06)
		RSVD_OPND_FAULT;
	if (ar & 01) {
		// Busy, set CC as 0011 (--VC)
		CC = CC_V|CC_C;
	} else {
		if (ar) {
			WriteV(header, ar | 1, OP_LONG, WA);
			c = ReadV(header + 4, OP_LONG, RA);
			if (ar == c) {
				WriteV(header, ar, OP_LONG, WA);
				DEF_NAME(vax, REMQHI)(vax);
				return;
			}
			if (c & 07) {
				WriteV(header, ar, OP_LONG, WA);
				RSVD_OPND_FAULT;
			}
			c += header;
			if (TestV(c + 4, RA, &t) < 0)
				WriteV(header, ar, OP_LONG, WA);
			b = ReadV(c + 4, OP_LONG, RA) + c;
			if (b & 07) {
				WriteV(header, ar, OP_LONG, WA);
				RSVD_OPND_FAULT;
			}
			if (TestV(b, WA, &t) < 0)
				WriteV(header, ar, OP_LONG, WA);
			WriteV(b, header - b, OP_LONG, WA);
			WriteV(header + 4, b - header, OP_LONG, WA);
			WriteV(header, ar, OP_LONG, WA);
		} else
			c = header;

		LSTORE(OP1, OP2, c);

		CC = (ar == 0) ? (CC_Z|CC_V) : 0;
	}
}

DEF_INST(vax, REMQUE)
{
	int32 entry = OP0;
	int32 succ, pred;

	succ = ReadV(entry, OP_LONG, RA);
	pred = ReadV(entry + 4, OP_LONG, RA);

	CC_CMP_L(succ, pred);
	if (entry != pred) {
		ReadV(succ + 4, OP_LONG, WA);
		if (OP1 < 0)
			ReadV(OP2, OP_LONG, WA);
		WriteV(pred, succ, OP_LONG, WA);
		WriteV(succ + 4, pred, OP_LONG, WA);
	} else
		CC |= CC_V;

	LSTORE(OP1, OP2, entry);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("REMQUE: Dequeue %08X from previous %08X and next %08X\n",
			entry, pred, succ);
#endif /* DEBUG */
}
