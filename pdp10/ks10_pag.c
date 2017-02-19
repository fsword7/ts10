// ks10_pag.c - KS10 Processor: Pager System Routines
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

#include "pdp10/defs.h"
#include "pdp10/ks10.h"

extern int36 pdp10_MemorySize;
extern int36 *pdp10_Memory;

int   pager_On;  // Pager System On/Off
int   pager_T20; // 1 = TOPS-20 Paging, 0 = TOPS-10 Paging
int30 pager_PC;
int36 pager_Flags;
void  (*pager_Cleanup)() = NULL;

int PACB = 0; // Previous AC Block
int CACB = 0; // Current AC Block

int36 SPB;   // SPT Base Address
int36 CSB;   // CST Base Address
int36 CSTM;  // CST Mask
int36 PUR;   // Process Use Register
int36 EBR;   // Executive Base Register
int36 UBR;   // User Base Register

int30 sptAddr; // Shared page table base address
int30 eptAddr; // Executive page table base address
int30 uptAddr; // User page table base address
int30 cstAddr; // Core status table base address

int30 PTAE;  // Page Table - Executive
int30 PTAU;  // Page Table - User
int36 PFW;   // Page Fail Word
int18 lhPFW;
int30 rhPFW;

int36 HSB = 0376000;   /* Halt Status Base */

// Cache Entry Table
int32 p10_eptCache[01000];
int32 p10_uptCache[01000];

void p10_ResetPager(void)
{
	int30 tblAddr;

	KX10_Pager_On = 0; // Pager System On/Off
	pager_T20     = 0; // Paging Mode - TOPS-10 or TOPS-20

	SPB  = 0; // SPT Base Address
	CSB  = 0; // CST Base Address
	CSTM = 0; // CST Mask
	PUR  = 0; // Process Use Register
	EBR  = 0; // Executive Base Register
	UBR  = 0; // User Base Register

	sptAddr = 0; // Shared page table base address
	eptAddr = 0; // Executive page table base address
	uptAddr = 0; // User page table base address
	cstAddr = 0; // Core status table base address

	PTAE = 0;  // Page Table - Executive
	PTAU = 0;  // Page Table - User

	PACB = 0; // Previous AC Block
	CACB = 0; // Current AC Block

	for (tblAddr = 0; tblAddr <= 0777; tblAddr++) {
		p10_eptCache[tblAddr] = 0;
		p10_uptCache[tblAddr] = 0;
	}
}

inline void p10_ClearCache(void)
{
	int30 tblAddr;

	for (tblAddr = 0; tblAddr <= 0777; tblAddr++) {
		p10_eptCache[tblAddr] = 0;
		p10_uptCache[tblAddr] = 0;
	}
}

// ************ Pager Instructions for KS10 Processor ************

// 70120 WREBR - Write Executive Base Register
void p10_ksOpcode_WREBR(void)
{
	// E -> EBR

	EBR = eAddr;

	// Now decode EBR contents
	KX10_Pager_On = (EBR & PG_EBR_M_ENA_PAGER);
	pager_T20     = (EBR & PG_EBR_M_TOPS20_PAGING);
	eptAddr       = (EBR & PG_EBR_M_EXEC_BASE_ADDR) << 9;

	// Clear all cache entries
	p10_ClearCache();

#ifdef DEBUG
	if (dbg_Check(DBG_DATA)) {
		dbg_Printf("PAGER: EBR <- %06o, EPT <- %o,,%06o\n",
			RH18(EBR), LH18(eptAddr), RH18(eptAddr));
		dbg_Printf("PAGER: Pager Active: %s  TOPS-20 Paging: %s\n",
			KX10_Pager_On ? "On" : "Off",
			pager_T20 ? "On" : "Off");
	}
#endif /* DEBUG */
}

// 70124 RDEBR - Read Executive Base Register
void p10_ksOpcode_RDEBR(void)
{
	// EBR -> (E)

	p10_vWrite(eAddr, EBR, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("PAGER: EBR -> %06o\n", RH18(EBR));
#endif /* DEBUG */
}

// 70114 WRUBR - Write User Base Register
void p10_ksOpcode_WRUBR(void)
{
	// (E) -> UBR

	BR = p10_vRead(eAddr, NOPXCT);
#ifdef DEBUG
		if (dbg_Check(DBG_DATA))
			dbg_Printf("PAGER: UBR <- %06o,,%06o\n", LH18(BR), RH18(BR));
#endif /* DEBUG */

	// Now decode UBR contents
	if (BR & PG_UBR_M_SEL_AC_BLOCKS) {
		UBR = (UBR & ~PG_UBR_M_ACB) | (BR & PG_UBR_M_ACB);
		PACB = (UBR & PG_UBR_M_PREV_AC_BLOCK) >> PG_UBR_P_PREV_AC_BLOCK;
		CACB = (UBR & PG_UBR_M_CUR_AC_BLOCK) >> PG_UBR_P_CUR_AC_BLOCK;
		prvAC = &p10_ACB[PACB][0];
		curAC = &p10_ACB[CACB][0];
#ifdef DEBUG
		if (dbg_Check(DBG_DATA)) {
			dbg_Printf("PAGER: Previous AC Block <- %o\n", PACB);
			dbg_Printf("PAGER: Current AC Block  <- %o\n", CACB);
		}
#endif /* DEBUG */
	}

	if (BR & PG_UBR_M_LD_USER_BASE_ADDR) {
		UBR = (UBR & ~PG_UBR_M_UADDR) | (BR & PG_UBR_M_UADDR);
		uptAddr = (UBR & PG_UBR_M_USER_BASE_ADDR) << 9;

		// Clear all cache entries
		p10_ClearCache();

#ifdef DEBUG
		if (dbg_Check(DBG_DATA))
			dbg_Printf("PAGER: UPT <- %o,,%06o\n",
				LH18(uptAddr), RH18(uptAddr));
#endif /* DEBUG */
	}
}

// 70104 RDUBR - Read User Base Register
void p10_ksOpcode_RDUBR(void)
{
	// UBR -> (E)

	p10_vWrite(eAddr, UBR, NOPXCT);

#ifdef DEBUG
		if (dbg_Check(DBG_DATA))
			dbg_Printf("PAGER: UBR -> %06o,,%06o\n", LH18(UBR), RH18(UBR));
#endif /* DEBUG */
}

// 70110 CLRPT - Clear Page Table
void p10_ksOpcode_CLRPT(void)
{
	// Invalidate page table line E<18:26>
	// Invalidate cache

	int30 tblAddr = (eAddr >> 9) & 0777;

	PTAE = 0; // Clear PT Executive
	PTAU = 0; // Clear PT User

	// Clear cache entry
	p10_eptCache[tblAddr] = 0;
	p10_uptCache[tblAddr] = 0;
}

// 70240 WRSPB - Write SPB
void p10_ksOpcode_WRSPB(void)
{
	// (E) -> SPT Base

	SPB = p10_vRead(eAddr, NOPXCT);
	sptAddr = SPB;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("WRSPB: SPB <- %06o,,%06o\n", LH18(SPB), RH18(SPB));
#endif /* DEBUG */
}

// 70200 RDSPB - Read SPB
void p10_ksOpcode_RDSPB(void)
{
	// SPT Base -> (E)

	p10_vWrite(eAddr, SPB, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("RDSPB: SPB -> %06o,,%06o\n", LH18(SPB), RH18(SPB));
#endif /* DEBUG */
}

// 70244 WRCSB - Write CSB
void p10_ksOpcode_WRCSB(void)
{
	// (E) -> CST Base

	CSB = p10_vRead(eAddr, NOPXCT);
	cstAddr = CSB;

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("WRCSB: CSB <- %06o,,%06o\n", LH18(CSB), RH18(CSB));
#endif /* DEBUG */
}

// 70204 RDCSB - Read CSB
void p10_ksOpcode_RDCSB(void)
{
	// CST Base -> (E)

	p10_vWrite(eAddr, CSB, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("RDCSB: CSB -> %06o,,%06o\n", LH18(CSB), RH18(CSB));
#endif /* DEBUG */
}

// 70254 WRCSTM - Write CST Mask
void p10_ksOpcode_WRCSTM(void)
{
	// (E) -> CST Mask

	// There is a known bug in MTBOOT.RDI (TOPS-20 v4.1)
	// Here is workaround to override that bug.
	if (eAddr == 040127)
		CSTM = 0770000000000LL;
	else
		CSTM = p10_vRead(eAddr, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("WRCSTM: CSTM <- %06o,,%06o\n", LH18(CSTM), RH18(CSTM));
#endif /* DEBUG */
}

// 70214 RDCSTM - Read CST Mask
void p10_ksOpcode_RDCSTM(void)
{
	// CST Mask -> (E)

	p10_vWrite(eAddr, CSTM, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("RDCSTM: CSTM -> %06o,,%06o\n", LH18(CSTM), RH18(CSTM));
#endif /* DEBUG */
}

// 70250 WRPUR - Write Process Use Register
void p10_ksOpcode_WRPUR(void)
{
	// (E) -> Process Use

	PUR = p10_vRead(eAddr, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("WRPUR: PUR <- %06o,,%06o\n", LH18(PUR), RH18(PUR));
#endif /* DEBUG */
}

// 70210 RDPUR - Read Process Use Register
void p10_ksOpcode_RDPUR(void)
{
	// Process Use -> (E)

	p10_vWrite(eAddr, PUR, NOPXCT);

#ifdef DEBUG
	if (dbg_Check(DBG_DATA))
		dbg_Printf("RDPUR: PUR -> %06o,,%06o\n", LH18(PUR), RH18(PUR));
#endif /* DEBUG */
}

// Halt Status

// 70270 WRHSB - Write Halt Status Base Register
void p10_ksOpcode_WRHSB(void)
{
	// (E) -> (Halt Status Base)
	// if (E[0]) = 0: Disable status storage

	HSB = p10_vRead(eAddr, NOPXCT);
}

// 70230 RDHSB - Read Halt Status Base Register
void p10_ksOpcode_RDHSB(void)
{
	// (Halt Status Base) -> (E)

	p10_vWrite(eAddr, HSB, NOPXCT);
}

void p10_SetHaltStatus(void)
{
/*
	mem_pwWrite(0, PC);
	if (!(HSB & HSB_M_STORAGE)) {
		mem_pwWrite(HSB_L_MAG,  0777777777777LL);
		mem_pwWrite(HSB_L_PC,   PC);
		mem_pwWrite(HSB_L_HR,   HR);
		mem_pwWrite(HSB_L_AR,   AR);
		mem_pwWrite(HSB_L_ARX,  ARX);
		mem_pwWrite(HSB_L_BR,   BR);
		mem_pwWrite(HSB_L_BRX,  BRX);
		mem_pwWrite(HSB_L_ONE,  0000000000001LL);
		mem_pwWrite(HSB_L_EBR,  EBR);
		mem_pwWrite(HSB_L_UBR,  UBR);
		mem_pwWrite(HSB_L_MASK, 0777777777777LL);
		mem_pwWrite(HSB_L_FLG,  FLAGS);
		mem_pwWrite(HSB_L_PI,   PISR);
		mem_pwWrite(HSB_L_XWD1, 0000001000001LL);
		mem_pwWrite(HSB_L_T0,   T0);
		mem_pwWrite(HSB_L_T1,   T1);
	}
*/
}

/****************************************************************/

// Page Fail Trap - Phase I
int KS10_PageTrap1(int mode)
{
	PFW = ((int36)lhPFW << 18) | rhPFW;

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
void KS10_PageTrap2(void)
{
#ifdef DEBUG
	if (dbg_Check(DBG_PAGEFAULT)) {
		dbg_Printf("PAGER: *** Page Fail Trap: %06o,,%06o at PC %06o (%06o)\n",
			LH18(PFW), RH18(PFW), pager_PC, PC);
		if (!dbg_Check(DBG_TRACE))
			p10_Disassemble(pager_PC, HR, 0);
	}
#endif /* DEBUG */

	if (cpu_pFlags & CPU_CYCLE_TRAP)
		FLAGS = pager_Flags;

	if (pager_T20) {
		p10_pWrite(uptAddr + T20_UPT_PF_WORD, PFW, 0);
		p10_pWrite(uptAddr + T20_UPT_PF_FLAGS, FLAGS, 0);
		p10_pWrite(uptAddr + T20_UPT_PF_OLD_PC, pager_PC, 0);
		BR = p10_pRead(uptAddr + T20_UPT_PF_NEW_PC, 0);
	} else {
		p10_pWrite(uptAddr + T10_UPT_PF_WORD, PFW, 0);
		p10_pWrite(uptAddr + T10_UPT_PF_OLD_PC, FLAGS | pager_PC, 0);
		BR = p10_pRead(uptAddr + T10_UPT_PF_NEW_PC, 0);
	}

	// Load new PC and system flags.
	FLAGS = BR & PC_FLAGS;
	PC    = VMA(BR);
}

// Page Table Fill routine
inline int KS10_PageRefill(uint30 vAddr, uint30 *pAddr, int mode)
{
	uint30 ptPageAddr;    // Page Address (vAddr >> 9)
	uint30 ptSection;     // Section Index
	uint30 ptBase;        // Page Table Base Address
	uint30 ptIndex;       // Page Table Index Address
	uint36 ptPage;        // Page Table Entry
	uint30 ptCSTAddr;     // CST Address
	uint36 ptCSTData;     // CST Data
	uint32 ptIndirect;    // Indirect flag

	lhPFW = PFW_PAGED;
	rhPFW = vAddr;
	if (mode & PTF_USER)
		lhPFW |= PFW_USER;
	if (mode & PTF_WRITE)
		lhPFW |= PFW_WRITE;

	if (pager_T20) {
		int36 ptAccess = PTE_T20_WRITE | PTE_T20_CACHE;

		// For KS10 only, Section index always is initially zero.
		ptSection = 0;

		ptIndex = ((lhPFW & PFW_USER) ? uptAddr : eptAddr) +
			T20_SECTION + (ptSection >> 18);
		ptPage = p10_pRead(ptIndex, mode);

		// Phase I - Process a section pointer first.

		do {
			ptIndirect = 0;
			ptAccess   &= ptPage; // Check access bits
			switch ((ptPage & PTE_T20_ACCESS) >> PTE_T20_P_ACCESS) {
				case PTE_T20_IMM: // Immediate pointer
					// Now final pointer here - Exit this loop.
					break;

				case PTE_T20_SHR: // Shared pointer
					ptIndex = sptAddr + (ptPage & PTE_T20_SIDX);
					ptPage = p10_pRead(ptSection | ptIndex, mode);
					// Now final pointer here - Exit this loop.
					break;

				case PTE_T20_IND: // Indirect pointer
					ptSection = ptPage & PTE_T20_PIDX;
					ptIndex = sptAddr + (ptPage & PTE_T20_SIDX);
					ptPage = p10_pRead(ptSection | ptIndex, mode);
					if (ptPage & PTE_T20_STM)
						PAGE_FAIL_TRAP(mode);
					ptIndex = ((ptPage & PTE_T20_PNUM) << 9) | (ptSection >> 18);
					ptPage = p10_pRead(ptSection | ptIndex, mode);
					ptIndirect = 1;
					break;

				default: // No access or bad pointer
					PAGE_FAIL_TRAP(mode);
			}
		} while (ptIndirect);

		ptBase = ((ptPage & PTE_T20_PNUM) << 9) | ((vAddr >> 9) & 0777);

		// Phase II - Now process a map pointer.

		do {
			// Now have first final pointer
			if (ptPage & PTE_T20_STM)
				PAGE_FAIL_TRAP(mode);

			// Now update CST Data
			if (cstAddr) {
				ptCSTAddr = cstAddr + (ptPage & PTE_T20_PNUM);
				ptCSTData = p10_pRead(ptCSTAddr, mode);
				if ((ptCSTData & CST_AGE) == 0)
					PAGE_FAIL_TRAP(mode);
				ptCSTData = (ptCSTData & CSTM) | PUR;
				p10_pWrite(ptCSTAddr, ptCSTData, mode);
			}

			ptPage = p10_pRead(ptBase, mode);

			ptIndirect = 0;
			ptAccess &= ptPage;
			switch ((ptPage & PTE_T20_ACCESS) >> PTE_T20_P_ACCESS) {
				case PTE_T20_IMM: // Immediate pointer
					// Now final pointer here - Exit this loop.
					break;

				case PTE_T20_SHR: // Shared pointer
					ptIndex = sptAddr + (ptPage & PTE_T20_SIDX);
					ptPage = p10_pRead(ptSection | ptIndex, mode);
					// Now final pointer here - Exit this loop.
					break;

				case PTE_T20_IND: // Indirect pointer
					ptIndex = (ptPage & PTE_T20_PIDX) >> PTE_T20_P_PIDX;
					ptBase = sptAddr + (ptPage & PTE_T20_SIDX);
					ptPage = p10_pRead(ptSection | ptBase, mode);
					ptBase = (ptPage << 9) | ptIndex;
					ptIndirect = 1;
					break;

				default: // No access or bad pointer
					PAGE_FAIL_TRAP(mode);
			}
		} while (ptIndirect);

		// Phase III - Now final pointer here,

		if (ptPage & PTE_T20_STM)
			PAGE_FAIL_TRAP(mode);

		// Now update CST Data
		if (cstAddr) {
			ptCSTAddr = cstAddr + (ptPage & PTE_T20_PNUM);
			ptCSTData = p10_pRead(ptCSTAddr, mode);
			if ((ptCSTData & CST_AGE) == 0)
				PAGE_FAIL_TRAP(mode);
			ptCSTData &= CSTM;
		}

		// Set eval done flag to PF word.
		lhPFW |= PFW_T20_DONE;

		// Check for Write Access
		if (ptAccess & PTE_T20_WRITE) {
			lhPFW |= PFW_T20_WRITE;
			if (cstAddr && (lhPFW & PFW_WRITE)) {
				lhPFW |= PFW_T20_MODIFIED;
				ptCSTData |= CST_MODIFIED;
			}
		} else {
			if (lhPFW & PFW_WRITE)
				PAGE_FAIL_TRAP(mode);
		}

		if (cstAddr)
			// Store new CST Data
			p10_pWrite(ptCSTAddr, ptCSTData | PUR, mode);

		if (ptAccess & PTE_T20_CACHE)
			// Page is cachable
			lhPFW |= PFW_CACHE;
	} else {
		// TOPS-10 Paging Translation (KI10 Paging System)
		ptPageAddr = (vAddr >> 9) & 0777;
		if (lhPFW & PFW_USER) {
			ptBase = uptAddr;
		} else {
			if (ptPageAddr < 0340)
				ptBase = eptAddr + 0600; // For 000-340 pages
			else if (ptPageAddr < 0400)
				ptBase = uptAddr + 0220; // For 340-377 pages
			else
				ptBase = eptAddr;        // For 400-777 pages
		}
		ptPage = p10_pRead(ptBase + (ptPageAddr >> 1), mode);
		ptPage = (ptPageAddr & 1) ? PTE_T10_ODD(ptPage) : PTE_T10_EVEN(ptPage);

		// Now we have a page map entry
		if (ptPage & PTE_T10_ACCESS) {
			lhPFW |= PFW_T10_ACCESS;

			if (ptPage & PTE_T10_CACHE)
				lhPFW |= PFW_CACHE;

			if (ptPage & PTE_T10_SOFTWARE)
				lhPFW |= PFW_T10_SOFTWARE;

			if (ptPage & PTE_T10_WRITABLE) {
				// Page is writable
				lhPFW |= PFW_T10_WRITE;
			} else {
				if (lhPFW & PFW_WRITE) {
					// Write Failure
					PAGE_FAIL_TRAP(mode);
				}
			}
		} else
			PAGE_FAIL_TRAP(mode);
	}

	// Green signal here....
	rhPFW = ((ptPage & 03777) << 9) | (vAddr & 0777);

	if (mode & PTF_MAP)
		PFW = ((int36)lhPFW << 18) | rhPFW;
	else {
#ifdef CACHE
		if (lhPFW & PFW_CACHE) {
			int32 cPage;

			// Set cache page entry
			cPage = PTE_V;
			if (lhPFW & PFW_USER)
				cPage |= PTE_U;
			if (lhPFW & PFW_WRITE)
				cPage |= PTE_M;
			cPage |= (rhPFW >> 9) & PTE_ADDR;

			((lhPFW & PFW_USER) ? p10_uptCache : p10_eptCache)[vAddr >> 9] = cPage;
		}
#endif /* CACHE */
		*pAddr = rhPFW;
	}

	return EMU_OK;
}

int36 KS10_GetMap(int36 vAddr, int mode)
{
	uint30 pAddr;

	if (KX10_Pager_On)
		KS10_PageRefill(vAddr, &pAddr, mode | PTF_MAP);
	else
		PFW = vAddr | (PFW_PHYSICAL << 18);

	return PFW;
}

int p10_CmdShowMap(char **args)
{
	int36 ptAddr;
	int36 data;
	int   idx;

	ptAddr = p10_pRead(eptAddr + T20_EPT_SECTION, 0);

	printf("Executive Base: %06o\n", eptAddr);
	ptAddr = (ptAddr & 0777) << 9;
	for (idx = 0; idx <= 0777; idx++) {
		data = p10_pRead(ptAddr + idx, 0);
		printf("%012llo: %012llo\n", ptAddr + idx, data);
	}
}
