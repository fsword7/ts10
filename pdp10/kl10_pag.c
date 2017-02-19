// kl10_pag.c - PAG (Pager System)
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

// Device Name: PAG
// Device Code: 010

#include "pdp10/defs.h"
#include "pdp10/kl10.h"
#include "pdp10/iodefs.h"

// KL10 Pager System
int32 KL10_Pager_On;  // Pager On/Off (On = 1, Off = 0)
int32 KL10_Pager_T20; // TOPS-20 Paging Mode (T20 = 1, T10 = 0)
int32 KL10_eFlags;    // Executive Base Register
int32 KL10_uFlags;    // User Base Register
int32 KL10_curACB;    // Current AC Block
int32 KL10_prvACB;    // Previous Context AC Block
int30 KL10_eptAddr;   // EPT Base Address
int30 KL10_uptAddr;   // UPT Base Address
int36 *KL10_cstMask;  // CST Mask Word    (AC 0, Block 6)
int36 *KL10_cstData;  // CST Data Word    (AC 1, Block 6)
int36 *KL10_cstAddr;  // CST Base Address (AC 2, Block 6)
int36 *KL10_sptAddr;  // SPT Base Address (AC 3, Block 6)

int18 KL10_pfFlags;   // Page Fail Word - Flags
int30 KL10_pfAddr;    // Page Fail Word - Address
int36 KL10_pfWord;    // Page Fail Word.

// KL10 Cache System
int32 KL10_Cache_On;  // Cache On/Off

//******************************************************************

// Pager System Initialization.
void KL10_InitPager(void)
{
	// Set pointers for accumulators (AC 0-3, Block 6)
	KL10_cstMask = &p10_ACB[6][0];
	KL10_cstData = &p10_ACB[6][1];
	KL10_cstAddr = &p10_ACB[6][2];
	KL10_sptAddr = &p10_ACB[6][3];

	// Default current/previous AC blocks.
	KL10_curACB  = 0;
	KL10_prvACB  = 0;
	PCS = 0;

	curAC = &p10_ACB[0][0];
	prvAC = &p10_ACB[0][0];

	// Default pager settings
	KX10_Pager_On  = 0;
	KL10_Pager_T20 = 0;
	KL10_eFlags    = 0;
	KL10_uFlags    = 0;
	KL10_eptAddr   = 0;
	KL10_uptAddr   = 0;

	KL10_Cache_On = 0;
}

//******************************************************************

void KL10_ioOpcode_CLRPT(void *uptr)
{
}

void KL10_ioOpcode_WRUBR(void *uptr)
{
	int36 ubreg = p10_vRead(eAddr, PXCT_CUR);

	// Loading Current and Previous Context AC Blocks
	if (ubreg & UBR_SELACB) {
		// Load new AC Block numbers
		KL10_uFlags = (KL10_uFlags & ~UBR_ACBS) | ((int32)ubreg & UBR_ACBS);
		KL10_curACB = (KL10_uFlags >> UBR_P_CACB) & UBR_M_ACB;
		KL10_prvACB = (KL10_uFlags >> UBR_P_PACB) & UBR_M_ACB;

		// Set new AC block pointers for KL10 processor.
		curAC = &p10_ACB[KL10_curACB][0];
		prvAC = &p10_ACB[KL10_prvACB][0];

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("KL10(PAG): (WRUBR) Loaded Cur ACB: %d  Prev ACB: %d\n",
				KL10_curACB, KL10_prvACB);
#endif /* DEBUG */
	}

	// Loading Previous Context Section
	if (ubreg & UBR_SELPCS) {
		// Now load previous context section
		KL10_uFlags  = (KL10_uFlags & ~UBR_PCS) | ((int32)ubreg & UBR_PCS);
		PCS          = (KL10_uFlags >> UBR_P_PCS) & UBR_M_PCS;

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("KL10(PAG): (WRUBR) Loaded Prev Section: %02o\n", PCS);
#endif /* DEBUG */
	}

	// Loading User Base Address
	if (ubreg & UBR_LDUPT) {
		KL10_uFlags  = (KL10_uFlags & ~UBR_ADDR) | ((int32)ubreg & UBR_ADDR);
		KL10_uptAddr = (KL10_uFlags & UBR_ADDR) << 9;

#ifdef DEBUG
		if (dbg_Check(DBG_TRACE|DBG_DATA))
			dbg_Printf("KL10(PAG): (WRUBR) Loaded UPT Address: %02o,,%07o\n",
				SR(KL10_uptAddr), RH(KL10_uptAddr));
#endif /* DEBUG */
	}

	// ACTION: Implement the updating the accounts.
}

void KL10_ioOpcode_RDUBR(void *uptr)
{
	int36 ubreg = UBR_SELS | KL10_uFlags | (PCS << 18);

	p10_vWrite(eAddr, ubreg | WORD36_XSIGN, PXCT_CUR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(PAG): (RDUBR) UBR => %06llo,,%06llo\n",
			SR(ubreg), RH(ubreg));
#endif /* DEBUG */
}

void KL10_ioOpcode_WREBR(void *uptr)
{
	KL10_eFlags = eAddr;

	// Load individual flags.
	KX10_Pager_On  = eAddr & EBR_ENABLE;
	KL10_Pager_T20 = eAddr & EBR_T20;
	KL10_Cache_On  = eAddr & EBR_CACHE;
	KL10_eptAddr   = (eAddr & EBR_ADDR) << 9;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("KL10(PAG): (WREBR) EBR <= %06o\n", KL10_eFlags);
		dbg_Printf("KL10(PAG): (WREBR) System: %s Paging: %s EPT: %02o,,%06o\n",
			KX10_Pager_On  ? "On" : "Off",
			KL10_Pager_T20 ? "TOPS-20" : "TOPS-10",
			SR(KL10_eptAddr), RH(KL10_eptAddr));
	}
#endif /* DEBUG */
}

void KL10_ioOpcode_RDEBR(void *uptr)
{
	p10_vWrite(eAddr, KL10_eFlags, PXCT_CUR);

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("KL10(PAG): (RDEBR) EBR => %06o\n", KL10_eFlags);
#endif /* DEBUG */
}

void kl10_InitPAG(KL10_DEVICE *kl10)
{
	P10_IOMAP *io;

	if (io = (P10_IOMAP *)calloc(1, sizeof(P10_IOMAP))) {
		io->devName    = "PAG";
		io->keyName    = "PAG";
		io->emuName    = "KL10: Pager";
		io->emuVersion = "(Internal)";
		io->idDevice   = KL10_PAG;
		io->ResetIO    = NULL;

		// Set APR instructions for KL10 processor
		io->Function[IOF_BLKI]  = NULL;
		io->Function[IOF_DATAI] = KL10_ioOpcode_RDUBR;
		io->Function[IOF_BLKO]  = KL10_ioOpcode_CLRPT;
		io->Function[IOF_DATAO] = KL10_ioOpcode_WRUBR;
		io->Function[IOF_CONO]  = KL10_ioOpcode_WREBR;
		io->Function[IOF_CONI]  = KL10_ioOpcode_RDEBR;
		io->Function[IOF_CONSZ] = NULL;
		io->Function[IOF_CONSO] = NULL;
		
		// Assign APR device to I/O mapping
		kx10_SetMap(io);
	}
}

//*********************************************************

// Page Fail Trap - Phase I
int KL10_PageTrap1(int mode)
{
	KL10_pfWord = ((int36)KL10_pfFlags << 18) | KL10_pfAddr;

	if (mode & (PTF_CONSOLE|PTF_MAP|PTF_BLT|PTF_NOTRAP))
		return EMU_PFAIL;

	// Cleanup routines for some instructions
	if (pager_Cleanup) {
		pager_Cleanup();
		pager_Cleanup = NULL;
	}

	emu_Abort(p10_SetJump, PAGE_FAIL);
}

// Page Fail Trap - Phase II
void KL10_PageTrap2(void)
{
#ifdef DEBUG
	if (dbg_Check(DBG_PAGEFAULT)) {
		dbg_Printf("PAGER: *** Page Fail Trap: %06o,,%06o at PC %06o (%06o)\n",
			LH18(KL10_pfWord), RH18(KL10_pfWord), pager_PC, PC);
		if (!dbg_Check(DBG_TRACE))
			p10_Disassemble(pager_PC, HR, 0);
	}
#endif /* DEBUG */

	if (cpu_pFlags & CPU_CYCLE_TRAP)
		FLAGS = pager_Flags;

	if (KL10_Pager_T20) {
		if (ISCPU(CNF_XADR)) {
			p10_pWrite(KL10_uptAddr + T20_XPF_WORD, KL10_pfWord, 0);
			p10_pWrite(KL10_uptAddr + T20_XPF_FLAGS, FLAGS, 0);
			p10_pWrite(KL10_uptAddr + T20_XPF_OLDPC, pager_PC, 0);
			BR = p10_pRead(KL10_uptAddr + T20_XPF_NEWPC, 0);
		} else {
			p10_pWrite(KL10_uptAddr + T20_PF_WORD, KL10_pfWord, 0);
			p10_pWrite(KL10_uptAddr + T20_PF_OLDPC, FLAGS | pager_PC, 0);
			BR = p10_pRead(KL10_uptAddr + T20_PF_NEWPC, 0);
		}
	} else {
		p10_pWrite(KL10_uptAddr + T10_PF_WORD, KL10_pfWord, 0);
		p10_pWrite(KL10_uptAddr + T10_PF_OLDPC, FLAGS | pager_PC, 0);
		BR = p10_pRead(KL10_uptAddr + T10_PF_NEWPC, 0);
	}

	// Load new PC and system flags.
	if (ISCPU(CNF_XADR)) {
		FLAGS = 0;
		DO_XJUMP(BR);
	} else {
		FLAGS = BR & PC_FLAGS;
		DO_XJUMP(VMA(BR));
	}
}

// Page Table Refill routine
int KL10_PageRefill(uint30 vAddr, uint30 *pAddr, int mode)
{
	uint30 ptPageAddr;    // Page Address (vAddr >> 9)
	uint30 ptSection;     // Section Index
	uint30 ptBase;        // Page Table Base Address
	uint30 ptIndex;       // Page Table Index Address
	uint36 ptPage;        // Page Table Entry
	uint30 ptCSTAddr;     // CST Address
	uint36 ptCSTData;     // CST Data
	uint32 ptIndirect;    // Indirect flag

	KL10_pfFlags = PFW_PAGED;
	KL10_pfAddr = vAddr;
	if (mode & PTF_USER)
		KL10_pfFlags |= PFW_USER;
	if (mode & PTF_WRITE)
		KL10_pfFlags |= PFW_WRITE;

	if (KL10_Pager_T20) {
		int36 ptAccess = (P20_PUBLIC|P20_WRITE|P20_CACHE);

		ptSection = LH(vAddr);
		ptIndex = ((KL10_pfFlags & PFW_USER) ? KL10_uptAddr : KL10_eptAddr) +
			T20_SECTION + SR(ptSection);
		ptPage = p10_pRead(ptIndex, mode);

		// Phase I - Process a section pointer first.

		do {
			ptIndirect = 0;
			ptAccess   &= ptPage; // Check access bits
			switch ((int32)(ptPage >> P20_P_TYPE) & P20_M_TYPE) {
				case P20_IMM: // Immediate pointer
					// Now final pointer here - Exit this loop.
					break;

				case P20_SHR: // Shared pointer
					ptIndex = *KL10_sptAddr + (ptPage & P20_SPTIDX);
					ptPage = p10_pRead(ptSection | ptIndex, mode);
					// Now final pointer here - Exit this loop.
					break;

				case P20_IND: // Indirect pointer
					ptSection = ptPage & P20_TBLIDX;
					ptIndex = *KL10_sptAddr + (ptPage & P20_SPTIDX);
					ptPage = p10_pRead(ptSection | ptIndex, mode);
					if (ptPage & P20_STM)
						PAGE_FAIL_TRAP(mode);
					ptIndex = ((ptPage & P20_PAGNUM) << 9) | (ptSection >> 18);
					ptPage = p10_pRead(ptSection | ptIndex, mode);
					ptIndirect = 1;
					break;

				default: // No access or bad pointer
					PAGE_FAIL_TRAP(mode);
			}
		} while (ptIndirect);

		ptBase = ((ptPage & P20_PAGNUM) << 9) | ((vAddr >> 9) & 0777);

		// Phase II - Now process a map pointer.

		do {
			// Now have first final pointer
			if (ptPage & P20_STM)
				PAGE_FAIL_TRAP(mode);

			// Now update CST Data
			if (*KL10_cstAddr) {
				ptCSTAddr = *KL10_cstAddr + (ptPage & P20_PAGNUM);
				ptCSTData = p10_pRead(ptCSTAddr, mode);
				if ((ptCSTData & CST_AGE) == 0)
					PAGE_FAIL_TRAP(mode);
				ptCSTData = (ptCSTData & *KL10_cstMask) | *KL10_cstData;
				p10_pWrite(ptCSTAddr, ptCSTData, mode);
			}

			ptPage = p10_pRead(ptBase, mode);

			ptIndirect = 0;
			ptAccess &= ptPage;
			switch ((ptPage & P20_TYPE) >> P20_P_TYPE) {
				case P20_IMM: // Immediate pointer
					// Now final pointer here - Exit this loop.
					break;

				case P20_SHR: // Shared pointer
					ptIndex = *KL10_sptAddr + (ptPage & P20_SPTIDX);
					ptPage = p10_pRead(ptSection | ptIndex, mode);
					// Now final pointer here - Exit this loop.
					break;

				case P20_IND: // Indirect pointer
					ptIndex = (ptPage >> P20_P_TBLIDX) & P20_M_TBLIDX;
					ptBase = *KL10_sptAddr + (ptPage & P20_SPTIDX);
//					ptPage = p10_pRead(ptSection | ptBase, mode);
					ptPage = p10_pRead(ptBase, mode);
					ptBase = (ptPage << 9) | ptIndex;
					ptIndirect = 1;
					break;

				default: // No access or bad pointer
					PAGE_FAIL_TRAP(mode);
			}
		} while (ptIndirect);

		// Phase III - Now final pointer here,

		if (ptPage & P20_STM)
			PAGE_FAIL_TRAP(mode);

		// Now update CST Data
		if (*KL10_cstAddr) {
			ptCSTAddr = *KL10_cstAddr + (ptPage & P20_PAGNUM);
			ptCSTData = p10_pRead(ptCSTAddr, mode);
			if ((ptCSTData & CST_AGE) == 0)
				PAGE_FAIL_TRAP(mode);
			ptCSTData &= *KL10_cstMask;
		}

		// Set eval done flag to PF word.
		KL10_pfFlags |= PFW_T20_A; // PFW_T20_DONE

		// Check for Write Access
		if (ptAccess & P20_WRITE) {
			KL10_pfFlags |= PFW_T20_W;
			if (*KL10_cstAddr && (KL10_pfFlags & PFW_WRITE)) {
				KL10_pfFlags |= PFW_T20_M;
				ptCSTData |= CST_MODIFIED;
			}
		} else {
			if (KL10_pfFlags & PFW_WRITE)
				PAGE_FAIL_TRAP(mode);
		}

		if (*KL10_cstAddr)
			// Store new CST Data
			p10_pWrite(ptCSTAddr, ptCSTData | *KL10_cstData, mode);

		if (ptAccess & P20_CACHE)
			// Page is cacheable
			KL10_pfFlags |= PFW_CACHE;
	} else {
		// TOPS-10 Paging Translation (KI10 Paging System)
		ptPageAddr = (vAddr >> 9) & 0777;
		if (KL10_pfFlags & PFW_USER) {
			ptBase = KL10_uptAddr;
		} else {
			if (ptPageAddr < 0340)
				ptBase = KL10_eptAddr + 0600; // For 000-340 pages
			else if (ptPageAddr < 0400)
				ptBase = KL10_uptAddr + 0220; // For 340-377 pages
			else
				ptBase = KL10_eptAddr;        // For 400-777 pages
		}
		ptPage = p10_pRead(ptBase + (ptPageAddr >> 1), mode);
		ptPage = (ptPageAddr & 1) ? RH(ptPage) : LHSR(ptPage);

		// Now we have a page map entry
		if (ptPage & P10_ACCESS) {
			KL10_pfFlags |= PFW_T10_A;

			if (ptPage & P10_PUBLIC)
				KL10_pfFlags |= PFW_PUBLIC;

			if (ptPage & P10_CACHE)
				KL10_pfFlags |= PFW_CACHE;

			if (ptPage & P10_SOFTWARE)
				KL10_pfFlags |= PFW_T10_S;

			if (ptPage & P10_WRITE) {
				// Page is writable
				KL10_pfFlags |= PFW_T10_W;
			} else {
				if (KL10_pfFlags & PFW_WRITE) {
					// Write Failure
					PAGE_FAIL_TRAP(mode);
				}
			}
		} else
			PAGE_FAIL_TRAP(mode);
	}

	// Green signal here....
	KL10_pfAddr = ((ptPage & P20_PAGNUM) << 9) | (vAddr & 0777);

	if (mode & PTF_MAP)
		KL10_pfWord = ((int36)KL10_pfFlags << 18) | KL10_pfAddr;
	else {
#ifdef CACHE
		if (KL10_pfFlags & PFW_CACHE) {
			int32 cPage;

			// Set cache page entry
			cPage = PTE_V;
			if (KL10_pfFlags & PFW_USER)
				cPage |= PTE_U;
			if (KL10_pfFlags & PFW_WRITE)
				cPage |= PTE_M;
			cPage |= (KL10_pfAddr >> 9) & PTE_ADDR;

			((KL10_pfFlags & PFW_USER) ? p10_uptCache : p10_eptCache)[vAddr >> 9] = cPage;
		}
#endif /* CACHE */
		*pAddr = KL10_pfAddr;
	}

	return EMU_OK;
}

int36 KL10_GetMap(int36 vAddr, int mode)
{
	uint30 pAddr;

	if (KX10_Pager_On)
		KL10_PageRefill(vAddr, &pAddr, mode | PTF_MAP);
	else
		KL10_pfWord = vAddr | (PFW_PHYSICAL << 18);

	return KL10_pfWord;
}
