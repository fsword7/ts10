// memory.c - PDP-11 Memory System
//
// Copyright (c) 2002, Timothy M. Stark
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

#include "pdp11/defs.h"
#include "pdp11/uqba.h"

uint16 *p11_InitMemory(register P11_CPU *p11, uint32 Size)
{
	if (p11->ramData = (int16 *)calloc(1, Size)) {
		p11->ramSize = Size;
		return (uint16 *)p11->ramData;
	}
	return NULL;
}

void p11_FreeMemory(register P11_CPU *p11)
{
	if (p11->ramData) {
		free(p11->ramData);
		p11->ramData = NULL;
		p11->ramSize = 0;
	}
}

// ************** Physical Addressing *************

inline uint16 p11_ReadIO(register P11_CPU *p11,
	uint32 pAddr, uint32 size)
{
	uint16 data;
	int    rc;

	if (rc = uq11_ReadIO(p11->uqba, pAddr, &data, size)) {
		if (rc == UQ_ADRERR) {
			SET_CPUERR(CPUE_ODD);
			ABORT(TRAP_ODD);
		} else {
			SET_CPUERR(CPUE_TMO);
			ABORT(TRAP_NXM);
		}
	}

	return data;
}

inline void p11_WriteIO(register P11_CPU *p11,
	uint32 pAddr, uint16 data, uint32 size)
{
	int    rc;

	if (rc = uq11_WriteIO(p11->uqba, pAddr, data, size)) {
		if (rc == UQ_ADRERR) {
			SET_CPUERR(CPUE_ODD);
			ABORT(TRAP_ODD);
		} else {
			SET_CPUERR(CPUE_TMO);
			ABORT(TRAP_NXM);
		}
	}
}

uint16 p11_ReadPW(register P11_CPU *p11, uint32 pAddr)
{
	if (pAddr < p11->ramSize)
		return p11->ramData[pAddr >> 1];

	// I/O Page Area
	if (pAddr >= IO_PAGEBASE)
		return p11_ReadIO(p11, pAddr, ACC_WORD);

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
	return 0;
}

uint16 p11_ReadPB(register P11_CPU *p11, uint32 pAddr)
{
	if (pAddr < p11->ramSize)
		return ((uint8 *)p11->ramData)[pAddr];

	// I/O Page Area
	if (pAddr >= IO_PAGEBASE)
		return p11_ReadIO(p11, pAddr, ACC_BYTE);

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
	return 0;
}

void p11_WritePW(register P11_CPU *p11, uint32 pAddr, uint16 data)
{
	if (pAddr < p11->ramSize) {
		p11->ramData[pAddr >> 1] = data;
		return;
	}

	// I/O Page Area
	if (pAddr >= IO_PAGEBASE) {
		p11_WriteIO(p11, pAddr, data, ACC_WORD);
		return;
	}

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
}

void p11_WritePB(register P11_CPU *p11, uint32 pAddr, uint16 data)
{
	if (pAddr < p11->ramSize) {
		((uint8 *)p11->ramData)[pAddr] = data;
		return;
	}

	// I/O Page Area
	if (pAddr >= IO_PAGEBASE) {
		p11_WriteIO(p11, pAddr, data, ACC_BYTE);
		return;
	}

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
}

// ************** Virtual Addressing *************

#define ODDTRAP(vAddr) p11_OddAddr(p11, vAddr);
inline void p11_OddAddr(register P11_CPU *p11, uint16 vAddr)
{
#ifdef DEBUG
	if (dbg_Check(DBG_INTERRUPT))
		dbg_Printf("%s: *** Odd Address: %06o at PC %06o\n",
			p11->Unit.devName, vAddr, PC);
#endif /* DEBUG */

	SET_CPUERR(CPUE_ODD);
	ABORT(TRAP_ODD);
}

#define MMTRAP(vAddr, apridx, abort) \
	p11_PageFault(p11, vAddr, apridx, abort);
inline void p11_PageFault(register P11_CPU *p11,
	uint16 vAddr, uint16 apridx, uint16 abort)
{
	if (UpdateMM)
		MMR0 = (apridx << MMR0_P_PAGE) | (MMR0 & ~MMR0_M_PAGE);
	MMR0 |= abort;

#ifdef DEBUG
	if (dbg_Check(DBG_PAGEFAULT)) {
		dbg_Printf("%s: *** Page Fault: %06o at PC %06o (Reason: %s)\n",
			p11->Unit.devName, vAddr, MMR2, abort == MMR0_NR ?
			"Non-Resident" : (abort == MMR0_PL) ? "Page Length" : "Read-Only");
		dbg_Printf("%s:     APR %03o: %06o %06o\n", p11->Unit.devName,
			apridx, APR(apridx) >> 16, APR(apridx) & 0177777);
	}
#endif /* DEBUG */

	ABORT(TRAP_MME);
}

inline uint32 p11_RelocR(register P11_CPU *p11, uint32 vAddr)
{
	uint32 pAddr;

#ifdef DEBUG
//	if ((int32)vAddr < 0)
//		dbg_Printf("%s: Bad Address: %o\n", p11->Unit.devName, vAddr);
#endif /* DEBUG */

	if (MMR0 & MMR0_MME) {
		// Break virtual address down
		uint32 apridx = (vAddr >> VA_P_APF) & 077;
		uint32 apr    = APR(apridx);
		uint32 plf    = (apr & PDR_PLF) >> 2;
		uint32 dbn    = vAddr & VA_BN;

		// Check Page Read Access
		if ((apr & PDR_PRD) == 0) 
			MMTRAP(vAddr, apridx, MMR0_NR);

		// Check Page Length
		if ((apr & PDR_ED) ? dbn < plf : dbn > plf)
			MMTRAP(vAddr, apridx, MMR0_PL);

		// Convert 16-bit virtual to 22-bit physical
		pAddr = ((vAddr & VA_DF) + ((apr >> 10) & 017777700)) & PA_MASK22;

		// Extending 18-bit to 22-bit physical addressing
		if ((MMR3 & MMR3_M22E) == 0) {
			pAddr &= PA_MASK18;
			if (pAddr >= PA_IOPAGE18)
				pAddr |= IO_PAGE18;
		}
	} else {
		// Extending 16-bit to 22-bit physical addressing
		pAddr = vAddr & VA_MASK;
		if (pAddr >= PA_IOPAGE16)
			pAddr |= IO_PAGE16;
	}
	return pAddr;
}

inline uint32 p11_RelocW(register P11_CPU *p11, uint32 vAddr)
{
	uint32 pAddr;

#ifdef DEBUG
//	if ((int32)vAddr < 0)
//		dbg_Printf("%s: Bad Address: %o\n", p11->Unit.devName, vAddr);
#endif /* DEBUG */

	if (MMR0 & MMR0_MME) {
		// Break virtual address down
		uint32 apridx = (vAddr >> VA_P_APF) & 077;
		uint32 apr    = APR(apridx);
		uint32 plf    = (apr & PDR_PLF) >> 2;
		uint32 dbn    = vAddr & VA_BN;

		// Check Page Read Access
		if ((apr & PDR_PRD) == 0)
			MMTRAP(vAddr, apridx, MMR0_NR);

		// Check Page Length
		if ((apr & PDR_ED) ? dbn < plf : dbn > plf)
			MMTRAP(vAddr, apridx, MMR0_PL);

		// Check Page Write Access
		if ((apr & PDR_PWR) == 0)
			MMTRAP(vAddr, apridx, MMR0_RO);

		// Mark that page as written
		APR(apridx) = apr | PDR_W;

		// Convert 16-bit virtual to 22-bit physical
		pAddr = ((vAddr & VA_DF) + ((apr >> 10) & 017777700)) & PA_MASK22;

		// Extending 18-bit to 22-bit physical addressing
		if ((MMR3 & MMR3_M22E) == 0) {
			pAddr &= PA_MASK18;
			if (pAddr >= PA_IOPAGE18)
				pAddr |= IO_PAGE18;
		}
	} else {
		// Extending 16-bit to 22-bit physical addressing
		pAddr = vAddr & VA_MASK;
		if (pAddr >= PA_IOPAGE16)
			pAddr |= IO_PAGE16;
	}
	return pAddr;
}

uint16 p11_ReadW(register P11_CPU *p11, uint32 vAddr)
{
	uint32 pAddr;

	if ((vAddr & 1) && (p11->Flags & CNF_ODDTRAP))
		ODDTRAP(vAddr);

	pAddr = p11_RelocR(p11, vAddr);
	if (pAddr < p11->ramSize)
		return p11->ramData[pAddr >> 1];

	// I/O Page Area
	if (pAddr >= IO_PAGEBASE)
		return p11_ReadIO(p11, pAddr,
			ACC_WORD | ((vAddr & VA_INST) ? ACC_INST : 0));

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
	return 0;
}

uint8 p11_ReadB(register P11_CPU *p11, uint32 vAddr)
{
	uint32 pAddr;

	pAddr = p11_RelocR(p11, vAddr);
	if (pAddr < p11->ramSize)
		return ((int8 *)p11->ramData)[pAddr];

	// I/O Page Area
	if (pAddr >= IO_PAGEBASE)
		return p11_ReadIO(p11, pAddr, ACC_BYTE);

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
	return 0;
}

uint16 p11_ReadMW(register P11_CPU *p11, uint32 vAddr, uint32 *pAddr)
{
	if ((vAddr & 1) && (p11->Flags & CNF_ODDTRAP))
		ODDTRAP(vAddr);

	*pAddr = p11_RelocW(p11, vAddr);
	if (*pAddr < p11->ramSize)
		return p11->ramData[*pAddr >> 1];

	// I/O Page Area
	if (*pAddr >= IO_PAGEBASE)
		return p11_ReadIO(p11, *pAddr, ACC_WORD);

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
	return 0;
}

uint8 p11_ReadMB(register P11_CPU *p11, uint32 vAddr, uint32 *pAddr)
{
	*pAddr = p11_RelocW(p11, vAddr);
	if (*pAddr < p11->ramSize)
		return ((int8 *)p11->ramData)[*pAddr];

	// I/O Page Area
	if (*pAddr >= IO_PAGEBASE)
		return p11_ReadIO(p11, *pAddr, ACC_BYTE);

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
}

void p11_WriteW(register P11_CPU *p11, uint32 vAddr, uint16 data)
{
	uint32 pAddr;

	if ((vAddr & 1) && (p11->Flags & CNF_ODDTRAP))
		ODDTRAP(vAddr);

	pAddr = p11_RelocW(p11, vAddr);
	if (pAddr < p11->ramSize) {
		p11->ramData[pAddr >> 1] = data;
		return;
	}

	// I/O Page Area
	if (pAddr >= IO_PAGEBASE) {
		p11_WriteIO(p11, pAddr, data, ACC_WORD);
		return;
	}

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
}

void p11_WriteB(register P11_CPU *p11, uint32 vAddr, uint8 data)
{
	uint32 pAddr;

	pAddr = p11_RelocW(p11, vAddr);
	if (pAddr < p11->ramSize) {
		((int8 *)p11->ramData)[pAddr] = data;
		return;
	}

	// I/O Page Area
	if (pAddr >= IO_PAGEBASE) {
		p11_WriteIO(p11, pAddr, data, ACC_BYTE);
		return;
	}

	// Non-existant Memory
	SET_CPUERR(CPUE_NXM);
	ABORT(TRAP_NXM);
}

// ******************* Console Read/write Access *****************

extern uint32 p11_dsMask[];
uint32 p11_GetSpace(register P11_CPU *p11, uint32 sw)
{
	uint32 mode;

	if (SWMASK('k'))      mode = AM_KERNEL;       // Kernel Mode
	else if (SWMASK('s')) mode = AM_SUPER;        // Supervisor Mode
	else if (SWMASK('u')) mode = AM_USER;         // User Mode
	else if (SWMASK('p')) mode = PSW_GETPRV(PSW); // Previous Mode
	else                  mode = PSW_GETCUR(PSW); // Current Mode

	return SWMASK('d') ? GetDSpace(mode) : GetISpace(mode);
}

inline int32 p11_RelocC(register P11_CPU *p11, uint32 vAddr)
{
	uint32 pAddr;

	if (MMR0 & MMR0_MME) {
		// Break virtual address down
		uint32 apridx = (vAddr >> VA_P_APF) & 077;
		uint32 apr    = APR(apridx);
		uint32 plf    = (apr & PDR_PLF) >> 2;
		uint32 dbn    = vAddr & VA_BN;

		// Check Page Read Access
		if ((apr & PDR_PRD) == 0)
			return -1;

		// Check Page Length
		if ((apr & PDR_ED) ? dbn < plf : dbn > plf)
			return -2;

		// Convert 16-bit virtual to 22-bit physical
		pAddr = ((vAddr & VA_DF) + ((apr >> 10) & 017777700)) & PA_MASK22;

		// Extending 18-bit to 22-bit physical addressing
		if ((MMR3 & MMR3_M22E) == 0) {
			pAddr &= PA_MASK18;
			if (pAddr >= PA_IOPAGE18)
				pAddr |= IO_PAGE18;
		}
	} else {
		// Extending 16-bit to 22-bit physical addressing
		pAddr = vAddr & VA_MASK;
		if (pAddr >= PA_IOPAGE16)
			pAddr |= IO_PAGE16;
	}
	return pAddr;
}

uint16 p11_ReadCP(register P11_CPU *p11, uint32 pAddr, uint32 size)
{
	if (pAddr < p11->ramSize) {
		if (size == 2)
			return p11->ramData[pAddr >> 1];
		else
			return ((uint8 *)p11->ramData)[pAddr];
	} else if (pAddr >= IO_PAGEBASE) {
		uint16 data;
		uq11_ReadIO(p11->uqba, pAddr, &data, 2);
		return data;
	}

	// Non-existant Memory
	return 0;
}

void p11_WriteCP(register P11_CPU *p11, uint32 pAddr, uint16 data, uint32 size)
{
	if (pAddr < p11->ramSize) {
		if (size == 2)
			p11->ramData[pAddr >> 1] = data;
		else
			((uint8 *)p11->ramData)[pAddr] = data;
	} else if (pAddr >= IO_PAGEBASE)
		uq11_WriteIO(p11->uqba, pAddr, data, size);
}

uint16 p11_ReadC(register P11_CPU *p11, uint32 vAddr, uint32 size)
{
	uint32 pAddr;

	if ((int32)(pAddr = p11_RelocC(p11, vAddr)) < 0)
		return 0;
	if (pAddr < p11->ramSize) {
		if (size == 2)
			return p11->ramData[pAddr >> 1];
		else
			return ((uint8 *)p11->ramData)[pAddr];
	} else if (pAddr >= IO_PAGEBASE) {
		uint16 data;
		uq11_ReadIO(p11->uqba, pAddr, &data, size);
		return data;
	}

	// Non-existant Memory
	return 0;
}

void p11_WriteC(register P11_CPU *p11, uint32 vAddr, uint16 data, uint32 size)
{
	uint32 pAddr;

	if ((int32)(pAddr = p11_RelocC(p11, vAddr)) < 0)
		return;
	if (pAddr < p11->ramSize) {
		if (size == 2)
			p11->ramData[pAddr >> 1] = data;
		else
			((uint8 *)p11->ramData)[pAddr] = data;
	} else if (pAddr >= IO_PAGEBASE)
		uq11_WriteIO(p11->uqba, pAddr, data, size);
}
