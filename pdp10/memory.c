// memory.c - PDP10 memory routines
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

#include <malloc.h>

#include "pdp10/defs.h"
#include "pdp10/ks10.h"
#include "pdp10/proto.h"

extern int30 eptKeepAlive;
extern int   KX10_IsGlobal;

int36 p10_MemorySize;
int36 *p10_Memory;
int32 p10_CacheMisses;
int32 p10_CacheHits;

int KX10_Pager_On;
int (*KX10_PageRefill)(uint30, uint30 *, int);

// Fast memory for AC blocks
int36 p10_ACB[NACBLOCKS][020];
int36 *curAC, *prvAC; // Current/Previous AC block

void p10_InitMemory(int32 size)
{
	int i, j;

	// Initialize main memory
	p10_MemorySize = size;
	p10_Memory = (int36 *)calloc(size, sizeof(int36));

	// Initialize AC blocks
	curAC = &p10_ACB[CACB = 0][0];
	prvAC = &p10_ACB[PACB = 0][0];
	for (i = 0; i < NACBLOCKS; i++)
		for (j = 0; j < 020; j++)
			p10_ACB[i][j] = 0;

	KX10_Pager_On = 0;
}

void p10_ResetMemory(void)
{
	int i, j;

	// Clear all memory.
	memset(p10_Memory, 0, p10_MemorySize * sizeof(int36));

	// Clear all AC blocks
	for (i = 0; i < NACBLOCKS; i++)
		for (j = 0; j < 020; j++)
			p10_ACB[i][j] = 0;

	KX10_Pager_On = 0;
}

void p10_ReleaseMemory(void)
{
	free(p10_Memory);
}

/*****************************************************/

// Check Non-existing memory area by using physical address 
//   Return TRUE  if memory is not existing 
//   Return FALSE if memory is existing 

/*
int p10_DoNXM(uint30 pAddr, int mode)
{
#ifdef DEBUG
	dbg_Printf("MEM: Non-existing Memory at address %06llo,,%06llo\n",
		LHSR(pAddr), RH(pAddr));
#endif // DEBUG

	// Call APR routine for desired processor.
	if (KX10_Trap_NoMemory)
		KX10_Trap_NoMemory(pAddr, mode);

	return EMU_NXM;
}

inline int p10_CheckNXM(uint30 pAddr, int mode)
{
	if (pAddr >= p10_MemorySize)
		return p10_DoNXM(pAddr, mode);
	return EMU_OK;
}
*/

inline int p10_CheckNXM(uint30 pAddr, int mode)
{
	if (pAddr >= p10_MemorySize) {
#ifdef DEBUG
//		dbg_Printf("MEM: Non-existing Memory at address %06llo,,%06llo\n",
//			LHSR(pAddr), RH(pAddr));
#endif /* DEBUG */

		// Call APR routine for desired processor.
		if (KX10_Trap_NoMemory)
			KX10_Trap_NoMemory(pAddr, mode);

		return EMU_NXM;
	}
	return EMU_OK;
}

#ifdef OPT_XADR
inline int p10_IsAC(uint30 vAddr)
{
	if (VMA(vAddr) < 020) {
		if (p10_Section < 2)         // Global AC address
			return TRUE;
		if (KX10_IsGlobal == FALSE)  // Local AC address
			return TRUE;
	}
	return FALSE;
}
#else  /* OPT_XADR */
#define p10_IsAC(vAddr) (vAddr < 020)
#endif /* OPT_XADR */

//**************  New Memory Routines *************

// Read Executive Memory
int36 p10_eRead(uint30 vAddr)
{
	if (p10_IsAC(vAddr))
		return curAC[AC(vAddr)];
	else {
		uint30 pAddr;
		int32  pte;

		// Translate virtual address into physical address.
		if (KX10_Pager_On) {
#ifdef CACHE
			if ((pte = p10_eptCache[vAddr >> 9]) == 0) {
//				p10_CacheMisses++;
				KX10_PageRefill(vAddr, &pAddr, 0);
			} else {
//				p10_CacheHits++;
				pAddr = ((pte & PTE_ADDR) << 9) | (vAddr & 0777);
			}
#else
			KX10_PageRefill(vAddr, &pAddr, 0);
#endif /* CACHE */
		} else
			pAddr = vAddr;
		if (p10_CheckNXM(pAddr, 0))
			return 0;

		return p10_Memory[pAddr];
	}
}

// Read Physical Memory
int36 p10_pRead(uint30 pAddr, int mode)
{
	if (p10_CheckNXM(pAddr, mode))
		return 0;
	return p10_Memory[pAddr];
}

// Read Virtual Memory
int36 p10_vRead(uint30 vAddr, int mode)
{
	if (p10_IsAC(vAddr))
		return ((mode & PTF_PREV) ? prvAC : curAC)[AC(vAddr)];
	else {
		uint30 pAddr;
		int32 pte;

		// Translate virtual address into physical address.
		if (KX10_Pager_On) {
			mode |= (FLAGS & FLG_USER) ? PTF_USER : 0;
#ifdef CACHE
			pte = ((mode & PTF_USER) ? p10_uptCache : p10_eptCache)[vAddr >> 9];
			if (pte == 0) {
//				p10_CacheMisses++;
				KX10_PageRefill(vAddr, &pAddr, mode);
			} else {
//				p10_CacheHits++;
				pAddr = ((pte & PTE_ADDR) << 9) | (vAddr & 0777);
			}
#else
			KX10_PageRefill(vAddr, &pAddr, mode);
#endif /* CACHE */
		} else
			pAddr = vAddr;
		if (p10_CheckNXM(pAddr, mode))
			return 0;
		return p10_Memory[pAddr];
	}
}

// Write Executive Memory
void p10_eWrite(uint30 vAddr, int36 data)
{
	if (p10_IsAC(vAddr))
		curAC[VMA(vAddr)] = SXT36(data);
	else {
		uint30 pAddr;
		int32 pte;

		// Translate virtual address into physical address.
		if (KX10_Pager_On) {
#ifdef CACHE
			if ((pte = p10_eptCache[vAddr >> 9]) >= 0) {
//				p10_CacheMisses++;
				KX10_PageRefill(vAddr, &pAddr, PTF_WRITE);
			} else {
//				p10_CacheHits++;
				pAddr = ((pte & PTE_ADDR) << 9) | (vAddr & 0777);
			}
#else
			KX10_PageRefill(vAddr, &pAddr, PTF_WRITE);
#endif /* CACHE */
		} else
			pAddr = vAddr;
		if (p10_CheckNXM(pAddr, 0))
			return;

		p10_Memory[pAddr] = SXT36(data);
	}
}

// Write Physical Memory
void p10_pWrite(uint30 pAddr, int36 data, int mode)
{
	if (p10_CheckNXM(pAddr, mode))
		return;

	p10_Memory[pAddr] = SXT36(data);
}

//  Write Virtual Memory
void p10_vWrite(uint30 vAddr, int36 data, int mode)
{
	if (p10_IsAC(vAddr))
		((mode & PTF_PREV) ? prvAC : curAC)[AC(vAddr)] = SXT36(data);
	else {
		uint30 pAddr;
		int32 pte;

#ifdef DEBUG
		if (vAddr == 0314021)
			dbg_Printf("Write DEVISN %o,,%o at PC %o,,%o\n",
				LH18(data), RH18(data), LH18(pager_PC), RH18(pager_PC));
#endif /* DEBUG */

		// Translate virtual address into physical address.
		if (KX10_Pager_On) {
			mode |= ((FLAGS & FLG_USER) ? PTF_USER : 0) | PTF_WRITE;
#ifdef CACHE
			pte = ((mode & PTF_USER) ? p10_uptCache : p10_eptCache)[vAddr >> 9];
			if (pte >= 0) {
//				p10_CacheMisses++;
				KX10_PageRefill(vAddr, &pAddr, mode);
			} else {
//				p10_CacheHits++;
				pAddr = ((pte & PTE_ADDR) << 9) | (vAddr & 0777);
			}
#else
			KX10_PageRefill(vAddr, &pAddr, mode);
#endif /* CACHE */
		} else
			pAddr = vAddr;

		if (p10_CheckNXM(pAddr, mode))
			return;

		p10_Memory[pAddr] = SXT36(data);
	}
}

// Provide direct access to physical memory.
// For Unibus routines, etc.
int36 *p10_pAccess(uint30 pAddr)
{
	if (p10_CheckNXM(pAddr, 0))
		return NULL;

	return &p10_Memory[pAddr];
}

// Provide direct access to virtual memory.
// Also implies read/write access test.
int36 *p10_Access(uint30 vAddr, int mode)
{

	if (p10_IsAC(vAddr))
		return &((mode & PTF_PREV) ? prvAC : curAC)[AC(vAddr)];
	else {
		uint30 pAddr;
		int32 pte;

		// Translate virtual address into physical address.
		if (KX10_Pager_On) {
			mode |= (FLAGS & FLG_USER) ? PTF_USER : 0;
#ifdef CACHE
			pte = ((mode & PTF_USER) ? p10_uptCache : p10_eptCache)[vAddr >> 9];
			if ((pte == 0) || ((pte >= 0) && (mode & PTF_WRITE))) {
//				p10_CacheMisses++;
				KX10_PageRefill(vAddr, &pAddr, mode);
			} else {
//				p10_CacheHits++;
				pAddr = ((pte & PTE_ADDR) << 9) | (vAddr & 0777);
			}
#else
			KX10_PageRefill(vAddr, &pAddr, mode);
#endif /* CACHE */
		} else
			pAddr = vAddr;
		if (p10_CheckNXM(pAddr, mode))
			return NULL;

		return &p10_Memory[pAddr];
	}
}
