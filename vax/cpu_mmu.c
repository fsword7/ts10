// cpu_mmu.c - Memory Management Routine
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
//#include "vax/vm.h"

// TLB Access Table (Protection Bits)

#define KERN  ACC_MASK(AM_KERNEL)      // Kernel Mask
#define EXEC  ACC_MASK(AM_EXECUTIVE)   // Executive Mask
#define SUPER ACC_MASK(AM_SUPERVISOR)  // Supervisor Mask
#define USER  ACC_MASK(AM_USER)        // User Mask

#define TLB_ACCR(mask) ((mask) << TLB_P_RACC) // Read Access Field
#define TLB_ACCW(mask) ((mask) << TLB_P_WACC) // Write Access Field

static const int32 accTable[16] =
{
//	Write Access                     Read Access
//	------------                     -----------
	0                              | 0,
	0                              | 0,
	TLB_ACCW(KERN)                 | TLB_ACCR(KERN),
	0                              | TLB_ACCR(KERN),
	TLB_ACCW(KERN|EXEC|SUPER|USER) | TLB_ACCR(KERN|EXEC|SUPER|USER),
	TLB_ACCW(KERN|EXEC)            | TLB_ACCR(KERN|EXEC),
	TLB_ACCW(KERN)                 | TLB_ACCR(KERN|EXEC),
	0                              | TLB_ACCR(KERN|EXEC),
	TLB_ACCW(KERN|EXEC|SUPER)      | TLB_ACCR(KERN|EXEC|SUPER),
	TLB_ACCW(KERN|EXEC)            | TLB_ACCR(KERN|EXEC|SUPER),
	TLB_ACCW(KERN)                 | TLB_ACCR(KERN|EXEC|SUPER),
	0                              | TLB_ACCR(KERN|EXEC|SUPER),
	TLB_ACCW(KERN|EXEC|SUPER)      | TLB_ACCR(KERN|EXEC|SUPER|USER),
	TLB_ACCW(KERN|EXEC)            | TLB_ACCR(KERN|EXEC|SUPER|USER),
	TLB_ACCW(KERN)                 | TLB_ACCR(KERN|EXEC|SUPER|USER),
	0                              | TLB_ACCR(KERN|EXEC|SUPER|USER)
};

// Page Fault Trap Messages
static char *pftMessage[] =
{
	"Access Control Violation",
	"Length Not Valid",
	"Unknown Code 02",
	"PTE Length Not Valid",
	"Translation Not Valid",
	"Unknown Code 05",
	"PTE Translation Not Valid",
	"Successful"
};

TLBENT vax_PageFault(register VAX_CPU *vax,
	uint32 vAddr, int32 acc, int32 *status, int32 err)
{
	TLBENT pteZero = { 0, 0 };

	// Tell some instructions as PROBEx that page fault.
	if (status || (acc & CA)) {
		if (status)
			*status = err;
		return pteZero;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_PAGEFAULT))
		dbg_Printf("MMU: Page Fault on loc %08X at PC %08X (%s)\n",
			vAddr, faultPC, ((err < 8) ? pftMessage[err] : "Unknown"));
#endif /* DEBUG */

	// Page fault during interrupt or exception routine.
	// Go to Kernel Stack Not Valid Fault.
	if (IN_IE)
		ABORT(-SCB_KSNV);

	// Set parameters for page fault routines
	P1 = ((acc & TLB_WACC) ? MM_WRITE : 0) | (err & MM_EMASK);
	P2 = vAddr;

	// Go to 'Page Fault Trap' routines
	ABORT(((err & MM_TNV) ? -SCB_TNV : -SCB_ACV));
}

#define MM_ERR(err) \
	return vax_PageFault(vax, vAddr, acc, status, err)

TLBENT vax_Fill(register VAX_CPU *vax,
	uint32 vAddr, int32 lnt, int32 acc, int32 *status)
{
	uint32 ptIndex = (vAddr & VA_VPN) >> 7;
	uint32 ptAddr;
	uint32 vpn, tbi, pte, tlbpte;

	if (vAddr & VA_S0) {
		// System Region
		if (ptIndex >= (SLR << 2))
			MM_ERR(MM_LNV);
		ptAddr = SBR + ptIndex;
	} else {
		if (vAddr & VA_P1) {
			// P1 (Control) Region
			if (ptIndex < (P1LR << 2))
				MM_ERR(MM_LNV);
			ptAddr = P1BR + ptIndex;
		} else {
			// P0 (Program) Region
			if (ptIndex >= (P0LR << 2))
				MM_ERR(MM_LNV);
			ptAddr = P0BR + ptIndex;
		}

		// PPTE must be in System region
		if ((ptAddr & VA_S0) == 0)
			ABORT(STOP_PPTE);

		vpn = VA_GETVPN(ptAddr);
		tbi = VA_GETTBI(vpn);

		if (STLB[tbi].tag != vpn) {
			ptIndex = ((ptAddr & VA_VPN) >> 7);
			if (ptIndex >= (SLR << 2))
				MM_ERR(MM_PLNV);
			pte = ReadP(SBR + ptIndex, OP_LONG);
			if ((pte & PTE_V) == 0)
				MM_ERR(MM_PTNV);
			STLB[tbi].tag = vpn;
			STLB[tbi].pte = ((pte << VA_N_OFF) & TLB_PFN) |
				accTable[PTE_GETACC(pte)];
		}

		ptAddr = (STLB[tbi].pte & TLB_PFN) | VA_GETOFF(ptAddr);
	}

	pte = ReadP(ptAddr, OP_LONG);

	tlbpte = ((pte << VA_N_OFF) & TLB_PFN) | accTable[PTE_GETACC(pte)];

	if ((tlbpte & acc) == 0)
		MM_ERR(MM_ACV);
	if ((pte & PTE_V) == 0)
		MM_ERR(MM_TNV);
	if (acc & TLB_WACC) {
		if ((pte & PTE_M) == 0)
			WriteP(ptAddr, pte | PTE_M, OP_LONG);
		tlbpte |= TLB_M;
	}

	vpn = VA_GETVPN(vAddr);
	tbi = VA_GETTBI(vpn);

	if (vAddr & VA_S0) {
		// System Space
		STLB[tbi].tag = vpn;
		STLB[tbi].pte = tlbpte;
		return STLB[tbi];
	} else {
		// Process Space
		PTLB[tbi].tag = vpn;
		PTLB[tbi].pte = tlbpte;
		return PTLB[tbi];
	}
}

// Clear Translation Buffer Table
void vax_ClearTBTable(register VAX_CPU *vax, int stlb)
{
	uint32 idx;

	for (idx = 0; idx < VA_TBSIZE; idx++) {
		PTLB[idx].tag = PTLB[idx].pte = -1;
		if (stlb)
			STLB[idx].tag = STLB[idx].pte = -1;
	}
}

// Clear TB Entry with virtual address
void vax_ClearTBEntry(register VAX_CPU *vax, uint32 vAddr)
{
	uint32 tbi = VA_GETTBI(VA_GETVPN(vAddr));

	if (vAddr & VA_S0)
		STLB[tbi].tag = STLB[tbi].pte = -1;
	else
		PTLB[tbi].tag = PTLB[tbi].pte = -1;
}

// Check TB Entry with virtual address
int vax_CheckTBEntry(register VAX_CPU *vax, uint32 vAddr)
{
	uint32 vpn = VA_GETVPN(vAddr);
	uint32 tbi = VA_GETTBI(vpn);

	return ((vAddr & VA_S0) ? STLB : PTLB)[tbi].tag == vpn;
}

// PROBER/PROBEW Instruction
//
//   opnd[0] = Mode
//   opnd[1] = Length
//   opnd[2] = Base Address

int vax_Probe(register VAX_CPU *vax, int32 *opnd, int rw)
{
	uint8  mode = opnd[0] & PSL_M_MODE;
	uint16 len  = opnd[1];
	uint32 base = opnd[2];
	uint8  prv  = PSL_GETPRV(PSL);
	int32  sts1, sts2;
	int32  acc, cc = 0;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("PROBE%c: Address: %08X  Length: %04X  Mode: %d/%d\n",
			(rw ? 'W' : 'R'), base, len, mode, prv);
#endif /* DEBUG */

	// Set up access with desired mode and write/read access.
	if (prv > mode) mode = prv;
	acc = ACC_MASK(mode) << (rw ? TLB_P_WACC : 0);

	TestV(base, acc, &sts1);
#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("PROBE%c:   Address: %08X  Status: %d (%s)\n",
			(rw ? 'W' : 'R'), base, sts1,
			((sts1 < 8) ? pftMessage[sts1] : "Unknown"));
#endif /* DEBUG */

	switch (sts1) {
		case MM_PTNV: // PTE Translation Not Valid
			P1 = (rw ? 4 : 0) | (sts1 & 03);
			P2 = base;
			ABORT(-SCB_TNV); // Force Page Fault

		case MM_TNV: // Translation Not Valid
		case MM_OK:  // Normal - Win!
			break;

		default:     // Other Page Faults - Lose
			return CC_Z;
	}

	TestV(base + len - 1, acc, &sts2);
#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("PROBE%c:   Address: %08X  Status: %d (%s)\n",
			(rw ? 'W' : 'R'), base + len - 1, sts2,
			((sts2 < 8) ? pftMessage[sts2] : "Unknown"));
#endif /* DEBUG */

	switch (sts2) {
		case MM_PTNV: // PTE Translation Not Valid
			P1 = (rw ? 4 : 0) | (sts2 & 03);
			P2 = base + len - 1;
			ABORT(-SCB_TNV); // Force Page Fault

		case MM_TNV: // Translation Not Valid
		case MM_OK:  // Normal - Win!
			return 0;

		default:     // Other Page Faults - Lose
			return CC_Z;
	}
}

DEF_INST(vax, PROBER)
{
	CC = vax_Probe(vax, &OP0, 0) | (CC & CC_C);
}

DEF_INST(vax, PROBEW)
{
	CC = vax_Probe(vax, &OP0, 1) | (CC & CC_C);
}

//****************************************************
//****************** Show Commands *******************
//****************************************************

int vax_ShowTLB(void *dptr, int argc, char **argv)
{
	VAX_CPU *vax = (VAX_CPU *)dptr;
	TLBENT  spte, ppte;
	int32   idx;

	printf("System Translation Buffer\n");

	printf("       System Space        Process Space\n");
	printf("\n");
	printf("       Virtual  Physical   Virtual  Physical\n");
	printf("Idx    Address  Address    Address  Address\n");
	printf("----   -------- --------   -------- --------\n");

	for (idx = 0; idx < VA_TBSIZE; idx++) {
		if (STLB[idx].tag == -1)
			continue;
		spte = STLB[idx];
		ppte = PTLB[idx];

		printf("%04X:   %08X %08X   %08X %08X\n", idx,
			spte.tag << VA_N_OFF, spte.pte & TLB_PFN,
			ppte.tag << VA_N_OFF, ppte.pte & TLB_PFN);
	}

	return VAX_OK;
}
