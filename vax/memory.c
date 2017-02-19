// memory.c - Memory Configurations
//
// Copyright (c) 2002-2003, Timothy M. Stark
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
//
// -------------------------------------------------------------------------
//
// Modification History:
//
// 03/11/03  TMS  Added 'vax_SetMemory' and renamed 'vax_FreeMemory' to
//                'vax_ReleaseMemory' for new 'set ram <bytes>' command.
//
// -------------------------------------------------------------------------

#include "vax/defs.h"

uint32 vax_SetMemory(VAX_CPU *vax, char *reqSize)
{
	uint32 ramSize;
	uint32 ramScale;
	char   ramType; // K, M, or G type.

	sscanf(reqSize, "%d%c", &ramSize, &ramType);

	ramScale = 0;
	if ((ramType == 'k') || (ramType == 'K'))
		ramScale = 1024;
	else if ((ramType == 'm') || (ramType == 'M'))
		ramScale = 1024 * 1024;
	else if ((ramType == 'g') || (ramType == 'G'))
		ramScale = 1024 * 1024 * 1024;

	ramSize *= ramScale;
	if (ramSize <= vax->ramMaxSize) {
		// if memory already was allocated, release
		// memory allocation back to host first.
		if (vax->RAM)
			vax_ReleaseMemory(vax);

		// Create physical memory
		if (vax->RAM = (uint8 *)calloc(1, ramSize)) {
			vax->baseRAM = 0L;
			vax->endRAM  = ramSize - 1;
			vax->sizeRAM = ramSize;
			return ramSize;
		}
	}
	return 0;
}

// That now is obsolete function call. Do not use that.
int vax_InitMemory(VAX_CPU *vax, int reqSize)
{
	// Create physical memory
	vax->RAM     = (uint8 *)malloc(reqSize);
	vax->baseRAM = 0L;
	vax->endRAM  = reqSize - 1;
	vax->sizeRAM = reqSize;

	// Clear all physical memory
	memset(vax->RAM, 0, reqSize);

	return VAX_OK;
}

int vax_ReleaseMemory(VAX_CPU *vax)
{
	// Free physical memory back to host system.
	if (vax->RAM)
		free(vax->RAM);

	// Reset all memory values;
	vax->RAM     = NULL;
	vax->baseRAM = -1L;
	vax->endRAM  = -1L;
	vax->sizeRAM = 0;

	return VAX_OK;
}

// **************************************
// New Memory Routines for more effective
// **************************************

#define IN_RAM(addr) \
	((addr) < vax->sizeRAM)

// Aligned Memory Access
// Note: It only works for little endian machines at this time.
#define BMEM(addr)  ((uint8 *)vax->RAM)[addr]
#define WMEM(addr)  ((uint16 *)vax->RAM)[addr]
#define LMEM(addr)  ((uint32 *)vax->RAM)[addr]

// Unaligned Memory Access
// Note: It only works for little endian machines at this time.
#define BMEMU(addr) *(uint8 *)(&vax->RAM[addr])
#define WMEMU(addr) *(uint16 *)(&vax->RAM[addr])
#define LMEMU(addr) *(uint32 *)(&vax->RAM[addr])

static const uint32 align[]  =
	{ 0x00FFFFFF, 0x00FFFFFF, 0x0000FFFF, 0x000000FF };
static const uint32 insert[] =
	{ 0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF };

// Physical Read Aligned
uint32 vax_ReadAligned(register VAX_CPU *vax, uint32 pAddr, int32 len)
{
	if (IN_RAM(pAddr)) {
#ifdef TEST_PARITY
		if (vax->TestParity)
			vax->TestParity(vax, pAddr);
#endif /* TEST_PARITY */
		if (len >= LN_LONG)
			return LMEM(pAddr >> 2);
		if (len == LN_WORD)
			return WMEM(pAddr >> 1);
		return BMEM(pAddr);
	} else {
		// I/O Device Access
		MCHK_REF = REF_V;
		return vax->ReadAligned(vax, pAddr, len);
	}
}

// Physical Write Aligned
void vax_WriteAligned(register VAX_CPU *vax,
	uint32 pAddr, uint32 data, int32 len)
{
	if (IN_RAM(pAddr)) {
#ifdef TEST_PARITY
		if (vax->WriteParity)
			vax->WriteParity(vax, pAddr);
#endif /* TEST_PARITY */
		if (len >= LN_LONG)
			LMEM(pAddr >> 2) = data;
		else if (len == LN_WORD)
			WMEM(pAddr >> 1) = data;
		else
			BMEM(pAddr) = data;
	} else {
		// I/O Device Access
		MCHK_REF = REF_V;
		vax->WriteAligned(vax, pAddr, data, len);
	}
}

// Physical Read Longword Aligned (Optimized)
inline uint32 vax_ReadLP(register VAX_CPU *vax, uint32 pAddr)
{
	if (IN_RAM(pAddr))
		return LMEM(pAddr >> 2);
	else {
		MCHK_ADDR = pAddr;
		MCHK_REF  = REF_P;
		return vax->ReadAligned(vax, pAddr, LN_LONG);
	}
}

// Physical Write Longword Aligned (Optimized)
inline void vax_WriteLP(register VAX_CPU *vax, uint32 pAddr, uint32 data)
{
	if (IN_RAM(pAddr))
		LMEM(pAddr >> 2) = data;
	else {
		MCHK_ADDR = pAddr;
		MCHK_REF  = REF_P;
		vax->WriteAligned(vax, pAddr, data, LN_LONG);
	}
}

// Virtual Read Longword Aligned (Optimized)
inline uint32 vax_ReadL(register VAX_CPU *vax, uint32 pAddr)
{
	if (IN_RAM(pAddr))
		return LMEM(pAddr >> 2);
	else {
		MCHK_REF = REF_V;
		return vax->ReadAligned(vax, pAddr, LN_LONG);
	}
}

// Virtual Write Longword Aligned (Optimized)
inline void vax_WriteL(register VAX_CPU *vax, uint32 pAddr, uint32 data)
{
	if (IN_RAM(pAddr))
		LMEM(pAddr >> 2) = data;
	else {
		MCHK_REF = REF_V;
		vax->WriteAligned(vax, pAddr, data, LN_LONG);
	}
}

// Virtual Read
uint32 vax_Read(register VAX_CPU *vax, uint32 vAddr, int32 lint, int32 acc)
{
	uint32 vpn, off, tbi;
	TLBENT xpte;
	uint32 pAddr, pAddr1;
	uint32 wl, wh, sc;

	MCHK_ADDR = vAddr;
	if (MAPEN) {
		// Translate virtual to physcial address
		vpn = VA_GETVPN(vAddr); // Get Virtual Page Number
		off = VA_GETOFF(vAddr); // Get Byte Offset
		tbi = VA_GETTBI(vpn);   // Get TLB Index

		xpte = (vAddr & VA_S0) ? STLB[tbi] : PTLB[tbi];

		if (((xpte.pte & acc) == 0) || (xpte.tag != vpn) ||
		    ((acc & TLB_WACC) && ((xpte.pte & TLB_M) == 0)))
			xpte = vax_Fill(vax, vAddr, lint, acc, NULL);
		pAddr = (xpte.pte & TLB_PFN) | off;
	} else
		pAddr = vAddr & PAMASK;

	// Check address is aligned first.
	if ((pAddr & (lint - 1)) == 0)
		return vax_ReadAligned(vax, pAddr, lint);

	// No, address is unaligned.
	if (MAPEN && ((off + lint) >= VA_PAGESIZE)) {
		// Address is crossing the page.
		vpn = VA_GETVPN(vAddr + lint);
		tbi = VA_GETTBI(vpn);
		
		xpte = (vAddr & VA_S0) ? STLB[tbi] : PTLB[tbi];

		if (((xpte.pte & acc) == 0) || (xpte.tag != vpn) ||
		    ((acc & TLB_WACC) && ((xpte.pte & TLB_M) == 0)))
			xpte = vax_Fill(vax, vAddr + lint, lint, acc, NULL);
		pAddr1 = (xpte.pte & TLB_PFN) | VA_GETOFF(vAddr + lint);
	} else
		pAddr1 = (pAddr + lint) & PAMASK;

	// Read 16/32-bit data in unaligned address
	// for crossing a page, etc.
	if (lint < LN_LONG) {
		if ((pAddr & 03) == 1)
			return ZXTW(vax_ReadL(vax, pAddr) >> 8);
		else {
			wl = vax_ReadL(vax, pAddr);
			wh = vax_ReadL(vax, pAddr1);
			return ZXTW((wl >> 24) | (wh << 8));
		}
	} else {
		wl = vax_ReadL(vax, pAddr);
		wh = vax_ReadL(vax, pAddr1);
		sc = (pAddr & 03) << 3;
		return (wl >> sc) | (wh << (32 - sc));
	}
}

// Virtual Write
void vax_Write(register VAX_CPU *vax,
	uint32 vAddr, uint32 data, int32 lint, int32 acc)
{
	uint32 vpn, off, tbi;
	TLBENT xpte;
	uint32 pAddr, pAddr1;
	uint32 wl, wh, bo, sc;

	MCHK_ADDR = vAddr;
	if (MAPEN) {
		// Translate virtual to physcial address
		vpn = VA_GETVPN(vAddr); // Get Virtual Page Number
		off = VA_GETOFF(vAddr); // Get Byte Offset
		tbi = VA_GETTBI(vpn);   // Get TLB Index

		xpte = (vAddr & VA_S0) ? STLB[tbi] : PTLB[tbi];

		if (((xpte.pte & acc) == 0) || (xpte.tag != vpn) ||
		    ((xpte.pte & TLB_M) == 0))
			xpte = vax_Fill(vax, vAddr, lint, acc, NULL);
		pAddr = (xpte.pte & TLB_PFN) | off;
	} else
		pAddr = vAddr & PAMASK;

	// Check address is aligned first.
	if ((pAddr & (lint - 1)) == 0)
		return vax_WriteAligned(vax, pAddr, data, lint);

	// No, address is unaligned.
	if (MAPEN && ((off + lint) >= VA_PAGESIZE)) {
		// Address is crossing the page.
		vpn = VA_GETVPN(vAddr + lint);
		tbi = VA_GETTBI(vpn);
		
		xpte = (vAddr & VA_S0) ? STLB[tbi] : PTLB[tbi];

		if (((xpte.pte & acc) == 0) || (xpte.tag != vpn) ||
		    ((xpte.pte & TLB_M) == 0))
			xpte = vax_Fill(vax, vAddr + lint, lint, acc, NULL);
		pAddr1 = (xpte.pte & TLB_PFN) | VA_GETOFF(vAddr + lint);
	} else
		pAddr1 = (pAddr + lint) & PAMASK;

	// Write 16/32-bit data in unaligned address
	// for crossing a page, etc.
	bo = pAddr & 03;
	wl = vax_ReadL(vax, pAddr);
	if (lint < LN_LONG) {
		// Unaligned word (16-bit words)
		if (bo == 1) {
			wl = (wl & 0xFF0000FF) | (ZXTW(data) << 8);
			vax_WriteL(vax, pAddr, wl);
		} else {
			wh = vax_ReadL(vax, pAddr1);
			wl = (wl & 0x00FFFFFF) | (data << 24);
			wh = (wh & 0xFFFFFF00) | ZXTB(data >> 8);
			vax_WriteL(vax, pAddr, wl);
			vax_WriteL(vax, pAddr1, wh);
		}
	} else {
		// Unaligned longword (32-bit words)
		wh = vax_ReadL(vax, pAddr1);
		sc = bo << 3;
		wl = (wl & insert[bo])  | (data << sc);
		wh = (wh & ~insert[bo]) | (data >> (32 - sc));
		vax_WriteL(vax, pAddr, wl);
		vax_WriteL(vax, pAddr1, wh);
	}
}

// Virtual Access Check
int32 vax_Test(register VAX_CPU *vax, uint32 vAddr, int32 acc, int32 *status)
{
	uint32 vpn, off, tbi;
	TLBENT xpte;

	*status = MM_OK; // Assume Ok.
	if (MAPEN) {
		vpn = VA_GETVPN(vAddr); // Get Virtual Page Number
		off = VA_GETOFF(vAddr); // Get Byte Offset
		tbi = VA_GETTBI(vpn);   // Get TLB Index

		xpte = (vAddr & VA_S0) ? STLB[tbi] : PTLB[tbi];

		if (((xpte.pte & acc) == 0) || (xpte.tag != vpn))
			xpte = vax_Fill(vax, vAddr, LN_BYTE, acc, status);
		return (*status == MM_OK) ? ((xpte.pte & TLB_PFN) | off) : -1;
	} else
		return vAddr & PAMASK;
}

// Prefetch instruction buffer
// Warning: This routine only work for little endian machines

#define BIBUF(idx) *((uint8 *)&IBUF[idx])
#define WIBUF(idx) *((uint16 *)&IBUF[idx])
#define LIBUF(idx) *((uint32 *)&IBUF[idx])

int32 vax_ReadInst(register VAX_CPU *vax, int32 size)
{
	uint32 data;
	int32  t;

	// If buffer is short, fetch data from main memory.
	// To flush instruction buffer, must set IBCNT as 0
	// and IBPPC as -1 to refill instruction buffer 
	// for branch and call instructions.

	if ((IBIDX + size) > IBCNT) {
		// Refill instruction buffer after flush.
		if ((IBPPC < 0) || (VA_GETOFF(IBPPC) == 0))  {
			IBPPC = TestV((PC + IBCNT) & ~03, RA, &t);
			if (t != MM_OK)
				ReadV((PC + IBCNT), LN_LONG, RA); // Force page fault trap
			IBIDX = PC & 03;
		}
		LIBUF(IBCNT) = vax_ReadLP(vax, IBPPC);
		IBPPC += LN_LONG;
		IBCNT += LN_LONG;
	}

	// Extract data from the instruction buffer
	if (size == LN_BYTE)
		data = BIBUF(IBIDX);
	else if (size == LN_WORD)
		data = WIBUF(IBIDX);
	else
		data = LIBUF(IBIDX);

	// Increment Program Counter (PC)
	PC    += size;
	IBIDX += size;
	if (IBIDX >= LN_LONG) {
		LIBUF(0) = LIBUF(LN_LONG);
		IBIDX -= LN_LONG;
		IBCNT -= LN_LONG;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_PREFETCH)) {
		dbg_Printf("PRE: Data=%08X PC=%08X PPC=%08X IDX=%d CNT=%d SIZE=%d\n",
			data, PC, IBPPC, IBIDX, IBCNT, size);
		if (IBCNT > LN_LONG)
			dbg_Printf("PRE: Buffer=%08X %08X\n", LIBUF(0), LIBUF(4));
		else
			dbg_Printf("PRE: Buffer=%08X\n", LIBUF(0));
	}
#endif /* DEBUG */

	return data;
}

// ******************* Console Read/Write Access *********************

// Console Read Access
int32 vax_ReadC(register VAX_CPU *vax,
	uint32 vAddr, uint32 *data, int32 size, uint32 sw)
{
	int32  pAddr, pAddr1; // Physical Address
	uint32 wl, wh, sc;    // Data Low and High
	int32  sts = MM_OK;   // Access Status - Assume Ok.

	if (MAPEN && (sw & SWMASK('v'))) {
		if ((pAddr = TestV(vAddr, CRA, &sts)) < 0)
			return sts;
	} else
		pAddr = vAddr & PAMASK;

	// Check address is aligned first.
	if ((pAddr & (size - 1)) == 0)
		return vax->ReadCA(vax, pAddr, data, size);

	if (MAPEN && (sw & SWMASK('v'))) {
		if ((VA_GETOFF(vAddr) + size) >= VA_PAGESIZE) {
			if ((pAddr1 = TestV(vAddr + size, CRA, &sts)) < 0)
				return sts;
		}
	} else
		pAddr1 = (vAddr + size) + PAMASK;

	// Fetch two words for crossing a page.
	if ((sts = vax->ReadCA(vax, pAddr1, &wh, size)) != MM_OK)
		return sts;
	if ((sts = vax->ReadCA(vax, pAddr, &wl, size)) != MM_OK)
		return sts;

	// Extract unaligned data.
	if (size == LN_WORD) {
		*data = (wl >> 8) | (wh << 8);
	} else {
		sc = (pAddr & 03) << 3;
		*data = (wl >> sc) | (wh << (32 - sc));
	}

	return sts;
}

// Console Write Access
int32 vax_WriteC(register VAX_CPU *vax,
	uint32 vAddr, uint32 data, int32 size, uint32 sw)
{
	int32  pAddr, pAddr1;  // Physical Address
	uint32 wl, wh, bo, sc; // Unaligned Data
	int32  sts = MM_OK;    // Access Status - Assume Ok.

	if (MAPEN && (sw & SWMASK('v'))) {
		if ((pAddr = TestV(vAddr, CRA, &sts)) < 0)
			return sts;
	} else
		pAddr = vAddr & PAMASK;

	// Check address is aligned first.
	if ((pAddr & (size - 1)) == 0)
		return vax->WriteCA(vax, pAddr, data, size);

	if (MAPEN && (sw & SWMASK('v'))) {
		if ((VA_GETOFF(vAddr) + size) >= VA_PAGESIZE) {
			if ((pAddr1 = TestV(vAddr + size, CRA, &sts)) < 0)
				return sts;
		}
	} else
		pAddr1 = (vAddr + size) & PAMASK;

	// Fetch two words for crossing a page.
	if ((sts = vax->ReadCA(vax, pAddr1, &wh, size)) != MM_OK)
		return sts;
	if ((sts = vax->ReadCA(vax, pAddr, &wl, size)) != MM_OK)
		return sts;

	// Insert unaligned data.
	if (size == LN_WORD) {
		wl = (wl & 0x00FF) | (data << 8);
		wh = (wh & 0xFF00) | (data >> 8);
	} else {
		bo = pAddr & 03;
		sc = bo << 3;
		wl = (wl & insert[bo])  | (data << sc);
		wh = (wh & ~insert[bo]) | (data >> (32 - sc));
	}

	// Send them back to memory.
	if ((sts = vax->WriteCA(vax, pAddr1, wh, size)) != MM_OK)
		return sts;
	if ((sts = vax->WriteCA(vax, pAddr, wl, size)) != MM_OK)
		return sts;

	return sts;
}

// Console Read Access with Instruction Buffer 
// Warning: This routine only work for little endian machines

#define BCIBUF(idx) *(uint8 *)(&CIBUF[idx])
#define WCIBUF(idx) *(uint16 *)(&CIBUF[idx])
#define LCIBUF(idx) *(uint32 *)(&CIBUF[idx])

int32 vax_ReadCI(register VAX_CPU *vax,
	uint32 vAddr, uint32 *data, int32 size, uint32 sw)
{
	int32 sts; // Memory Access Status

	// If buffer is short, fetch data from main memory.
	// Use different virutal address rather than
	// following address to refill instruction buffer.

	if (vAddr != CVADDR) {
		CVADDR = vAddr;
		CIBCNT = 0;
		CIBPPC = -1;
	}

	if ((CIBIDX + size) > CIBCNT) {
		// Refill instruction buffer after flush.
		if ((CIBPPC < 0) || (VA_GETOFF(CIBPPC) == 0))  {
			CIBPPC = TestV((vAddr + CIBCNT) & ~03, RA, &sts);
			if (sts != MM_OK)
				return sts;
			CIBIDX = vAddr & 03;
		}
		vax->ReadCA(vax, CIBPPC, (uint32 *)&LCIBUF(CIBCNT), LN_LONG);
		CIBPPC += LN_LONG;
		CIBCNT += LN_LONG;
	}

	// Extract data from the instruction buffer
	if (size == LN_BYTE)
		*data = BCIBUF(CIBIDX);
	else if (size == LN_WORD)
		*data = WCIBUF(CIBIDX);
	else
		*data = LCIBUF(CIBIDX);

#ifdef DEBUG
	if (dbg_Check(DBG_PREFETCH)) {
		dbg_Printf("PRE: Data=%08X PC=%08X PPC=%08X IDX=%d CNT=%d SIZE=%d\n",
			*data, CVADDR, CIBPPC, CIBIDX, CIBCNT, size);
		if (CIBCNT > LN_LONG)
			dbg_Printf("PRE: Buffer=%08X %08X\n", LCIBUF(0), LCIBUF(4));
		else
			dbg_Printf("PRE: Buffer=%08X\n", LCIBUF(0));
	}
#endif /* DEBUG */

	// Increment Program Counter (PC)
	CVADDR += size;
	CIBIDX += size;
	if (CIBIDX >= LN_LONG) {
		LCIBUF(0) = LCIBUF(LN_LONG);
		CIBIDX -= LN_LONG;
		CIBCNT -= LN_LONG;
	}

	return sts;
}
