// ka650_qba.c - CQBIC - Q22-Bus Interface for MicroVAX III Series
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
#include "vax/dev_cqbic.h"
#include "vax/ka650.h"

// Look at bottom of this file.
UQ_CALL cq_Callback;

// Master Memory Error - Non-Existant
void cq_errMaster(CQ_DEVICE *cq, uint32 pAddr)
{
	if (cq->dser & DSER_ERR)
		cq->dser |= DSER_LST;
	cq->dser |= DSER_MNX;
	cq->mear = (pAddr >> VA_P_VPN) & MEAR_MASK;
}

// Slave Memory Error - Non-Existant
void cq_errSlave(CQ_DEVICE *cq, uint32 pAddr)
{
	if (cq->dser & DSER_ERR)
		cq->dser |= DSER_LST;
	cq->dser |= DSER_SNX;
	cq->sear = (pAddr >> VA_P_VPN) & SEAR_MASK;
}

// ********************************************************************

// Q22 Bus Initialization Routine
void cq_ResetAll(CQ_DEVICE *cq)
{
	MAP_IO *io;

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: Q22 Bus Initialization Begins:\n", cq->devName);
#endif /* DEBUG */

	for (io = cq->ioList; io; io = io->Next) {
#ifdef DEBUG
//		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s:   Reseting %s: (%s) ... ",
				cq->devName, io->devName, io->keyName);
		if (io->ResetIO) {
			io->ResetIO(io->Device);
//			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("Done.\n");
		} else // if (dbg_Check(DBG_IOREGS))
			dbg_Printf("Skipped.\n");
#else /* DEBUG */
		if (io->ResetIO)
			io->ResetIO(io->Device);
#endif /* DEBUG */
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: End of Q22 Bus Initalization.\n", cq->devName);
#endif /* DEBUG */
}

uint32 cq_ReadBIC(CQ_DEVICE *cq, uint32 pAddr)
{
	int reg = (pAddr - CQBIC_BASE) >> 2;

	switch (reg) {
		case nSCR:
			return (cq->scr | SCR_POK) & SCR_MASK;
		case nDSER:
			return cq->dser & DSER_MASK;
		case nMEAR:
			return cq->mear & MEAR_MASK;
		case nSEAR:
			return cq->sear & SEAR_MASK;
		case nMBR:
			return cq->mbr & MBR_MASK;

		default:
			return 0;
	}
}

void cq_WriteBIC(CQ_DEVICE *cq, uint32 pAddr, uint32 data, uint32 size)
{
	int    reg = (pAddr - CQBIC_BASE) >> 2;
	uint32 ndata;

	if (size < LN_LONG) {
		uint32 tmp  = cq_ReadBIC(cq, pAddr);
		uint32 mask = (size == OP_WORD) ? WMASK : BMASK;
		uint32 sc   = (pAddr & 3) << 3;
		ndata = ((data & mask) << sc) | (tmp & ~(mask << sc));
		data  <<= sc;
	} else
		ndata = data;

	switch (reg) {
		case nSCR: // System Configuration Register
			cq->scr = ((ndata & SCR_RW) & SCR_MASK) |
				(cq->scr & ~SCR_RW);
			break;

		case nDSER:
			cq->dser = (cq->dser & ~data) & DSER_MASK;
			if (data & DSER_SME)
				cq->ipcReg[0] &= ~IPC_QME;
			break;

		case nMEAR:
		case nSEAR:
			{
				VAX_CPU *vax = cq->Processor;
				cq_errMaster(cq, pAddr);
				MACH_CHECK(MCHK_READ);
			}
			break;

		case nMBR:
			cq->mbr = ndata & MBR_MASK;
			break;
	}
}

// *************************************************************

uint32 cq_ReadIPC(CQ_DEVICE *cq, uint32 pAddr)
{
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) IPC (%08X) => %04X\n",
			cq->devName, pAddr, cq->ipcReg[0]);
#endif /* DEBUG */

	return cq->ipcReg[0] & IPC_MASK;
}

void cq_WriteIPC(CQ_DEVICE *cq, uint32 pAddr, uint32 data, uint32 size)
{
	if (size < LN_LONG)
		data <<= ((pAddr & 3) << 3);

	cq->ipcReg[0] &= ~(data & IPC_W1C);
	if ((pAddr & 3) == 0)
		cq->ipcReg[0] = (data & IPC_RW) | (cq->ipcReg[0] & ~IPC_RW);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) IPC (%08X) %04X <= %04X (Size: %d)\n",
			cq->devName, pAddr, cq->ipcReg[0], data, size);
#endif /* DEBUG */
}

int cq_ReadDBL(void *dptr, uint32 pAddr, uint16 *data, uint32 size)
{
	CQ_DEVICE *cq = (CQ_DEVICE *)dptr;

	if (size == LN_BYTE)
		*data = cq->ipcReg[0] >> ((pAddr & 1) ? 8 : 0) & BMASK;
	else
		*data = cq->ipcReg[0];

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) IPC (%08X) %04X => %04X (Size: %d)\n",
			cq->devName, pAddr, cq->ipcReg[0], *data, size);
#endif /* DEBUG */

	return UQ_OK;
}

int cq_WriteDBL(void *dptr, uint32 pAddr, uint16 data, uint32 size)
{
	CQ_DEVICE *cq = (CQ_DEVICE *)dptr;
	MAP_IO    *io = &cq->ipcr;

	if ((size == LN_BYTE) && (pAddr & 1))
		data = (data << 8) | (cq->ipcReg[0] & (~IPC_W1C & BMASK));

	cq->ipcReg[0] =   (data & IPC_RW) | (cq->ipcReg[0] & ~IPC_RW);
	cq->ipcReg[0] &= ~(data & IPC_W1C);
	if ((data & (IPC_IE|IPC_IRQ)) == (IPC_IE|IPC_IRQ))
		io->SendInterrupt(io, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) IPC (%08X) %04X <= %04X (Size: %d)\n",
			cq->devName, pAddr, cq->ipcReg[0], data, size);
#endif /* DEBUG */

	return UQ_OK;
}

// *************************************************************

int cq_MapAddr(CQ_DEVICE *cq, uint32 qAddr, uint32 *mAddr)
{
	VAX_CPU *vax = (VAX_CPU *)cq->Processor;
	uint32 meAddr = cq->mbr + (((qAddr >> VA_P_VPN) << 2) & CQMEM_MASK);

	if (IN_RAM(meAddr)) {
		uint32 mapEntry = LMEM(meAddr >> 2);
		if (mapEntry & MAP_VALID) {
			*mAddr = ((mapEntry & MAP_PAGE) << VA_P_VPN) + VA_GETOFF(qAddr);
			if (IN_RAM(*mAddr))
				return 1; // We win!

			// Invalid Memory
			cq_errSlave(cq, *mAddr);
			return 0;
		}
		// Invalid Map Entry
		cq_errMaster(cq, qAddr);
		return 0;
	}

	// Invalid Memory
	cq_errSlave(cq, 0);
	return 0;
}


uint32 cq_ReadMAP(CQ_DEVICE *cq, uint32 pAddr)
{
	VAX_CPU *vax = (VAX_CPU *)cq->Processor;
	uint32  mAddr = (pAddr & CQMAP_MASK) + cq->mbr;

#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (R) Map Base %08X + %08X => %08X\n",
				cq->devName, cq->mbr, pAddr & CQMAP_MASK, mAddr);
#endif /* DEBUG */

	if (IN_RAM(mAddr)) {
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (R) Map Register (%08X) => %08X\n",
				cq->devName, pAddr, LMEM(mAddr >> 2));
#endif /* DEBUG */
		return LMEM(mAddr >> 2);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) Map Register (%08X) - Non-Existant\n",
			cq->devName, mAddr);
#endif /* DEBUG */

	// Non-existant Memory
	cq_errSlave(cq, mAddr);
	MACH_CHECK(MCHK_READ);
	return 0;
}

void cq_WriteMAP(CQ_DEVICE *cq, uint32 pAddr, uint32 data, uint32 size)
{
	VAX_CPU *vax = (VAX_CPU *)cq->Processor;
	uint32  mAddr = (pAddr & CQMAP_MASK) + cq->mbr;

#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (W) Map Base %08X + %08X => %08X\n",
				cq->devName, cq->mbr, pAddr & CQMAP_MASK, mAddr);
#endif /* DEBUG */

	if (IN_RAM(mAddr)) {
		if (size >= LN_LONG)
			LMEM(mAddr >> 2) = data;
		else if (size == LN_WORD)
			WMEM(mAddr >> 1) = data;
		else
			BMEM(mAddr) = data;

#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (W) Map Register (%08X) %08X <= %08X (size: %d bytes)\n",
				cq->devName, pAddr, LMEM(mAddr >> 2), data, size);
#endif /* DEBUG */

		return;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) Map Register (%08X) - Non-Existant\n",
			cq->devName, mAddr);
#endif /* DEBUG */

	// Non-existant Memory
	cq_errSlave(cq, mAddr);
	MEMERR = 1;
}

uint32 cq_ReadMEM(CQ_DEVICE *cq, uint32 pAddr)
{
	VAX_CPU *vax = (VAX_CPU *)cq->Processor;
	uint32  mAddr;

	if (cq_MapAddr(cq, pAddr & CQMEM_MASK, &mAddr))
		return LMEM(mAddr >> 2);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) Non-Existant Memory (%08X) at PC %08X\n",
			cq->devName, pAddr, faultPC);
#endif /* DEBUG */

	// Non-existant Memory
	MACH_CHECK(MCHK_READ);
	return 0;
}

void cq_WriteMEM(CQ_DEVICE *cq, uint32 pAddr, uint32 data, uint32 size)
{
	VAX_CPU *vax = (VAX_CPU *)cq->Processor;
	uint32  mAddr;

	if (cq_MapAddr(cq, pAddr & CQMEM_MASK, &mAddr)) {
		if (size >= LN_LONG)
			LMEM(mAddr >> 2) = data;
		else if (size == LN_WORD)
			WMEM(mAddr >> 1) = data;
		else
			BMEM(mAddr) = data;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) Non-Existant Memory (%08X) at PC %08X\n",
			cq->devName, pAddr, faultPC);
#endif /* DEBUG */

	// Non-existant Memory
	MEMERR = 1;
}

// *************************************************************
 
inline uint32 cq_ReadQB(CQ_DEVICE *cq, uint32 pAddr, uint32 size)
{
	uint32  ioAddr = pAddr - IO_BASE;
	VAX_CPU *vax;
	MAP_IO  *io;
	uint16  data;

	// Access a desired I/O device
	if (io = cq->ioMap[ioAddr >> 1]) {
		io->ReadIO(io->Device, ioAddr, &data, size);
		return data;
	}

	vax = cq->Processor;
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) Non-Existant Device (%06o) at PC %08X\n",
			cq->devName, ioAddr, faultPC);
#endif /* DEBUG */

	// Non-existant Device
	cq_errMaster(cq, pAddr);
	MACH_CHECK(MCHK_READ);
	return 0;
}

inline void cq_WriteQB(CQ_DEVICE *cq, uint32 pAddr, uint32 data, uint32 size)
{
	uint32  ioAddr = pAddr - IO_BASE;
	VAX_CPU *vax;
	MAP_IO  *io;

	// Access a desired I/O device
	if (io = cq->ioMap[ioAddr >> 1]) {
		io->WriteIO(io->Device, ioAddr, data, size);
		return;
	}

	vax = cq->Processor;
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) Non-Existant Device (%06o) at PC %08X\n",
			cq->devName, ioAddr, faultPC);
#endif /* DEBUG */

	// Non-existant device
	cq_errMaster(cq, pAddr);
	MEMERR = 1;
}

uint32 cq_ReadIO(CQ_DEVICE *cq, uint32 pAddr, uint32 size)
{
	uint32 data;

	// Read 8/16/32-bit data from I/O device.
	data = cq_ReadQB(cq, pAddr, size);
	if (size >= LN_LONG)
		data |= cq_ReadQB(cq, pAddr + 2, size) << 16;
	else if ((size == LN_BYTE) && (pAddr & 1))
		data >>= 8;
	
//	else 
//		data <<= ((pAddr & 2) ? 16 : 0);

	return data;
}

void cq_WriteIO(CQ_DEVICE *cq, uint32 pAddr, uint32 data, uint32 size)
{
	MAP_IO *io;

	// Write 8/16/32-bit data to I/O device.
	cq_WriteQB(cq, pAddr, data, size);
	if (size >= LN_LONG)
		cq_WriteQB(cq, pAddr + 2, data >> 16, size);
}

// ****************************************************************

// Evaluate Interrupts
uint32 cq_CheckIRQ(VAX_CPU *vax, uint32 nipl)
{
	CQ_DEVICE *cq  = (CQ_DEVICE *)vax->uqba;
	int32     idx;

	for (idx = UQ_HLVL-1; idx >= 0; idx--) {
		if ((idx + IPL_HMIN) <= nipl)
			return 0;
		if (cq->iplList[idx].intReqs)
			return idx + IPL_HMIN;
	}
	return 0;
}

// Acknowledge Interrupt and Get Vector
uint16 cq_GetVector(VAX_CPU *vax, uint32 nipl)
{
	CQ_DEVICE *cq  = (CQ_DEVICE *)vax->uqba;
	UQ_IPL    *ipl = &cq->iplList[nipl];
	int       lvl, vec;
	MAP_IO    *io;

	lvl = ipl->intReqs;
	for (vec = 0; vec < UQ_NVECS; vec++) {
		if ((lvl >> vec) & 1) {
			ipl->intReqs &= ~(1u << vec);
//			if ((io = ipl->ioMap[vec]) && io->AckInterrupt)
//				return io->AckInterrupt(io->Device);
#ifdef DEBUG
			if (dbg_Check(DBG_INTERRUPT))
				dbg_Printf("%s: IRQ BR%d, Level %d, Vector %03o\n",
					cq->devName, nipl + UQ_BR4, vec, ipl->intVecs[vec]);
#endif /* DEBUG */
			return ipl->intVecs[vec];
		}
	}

	// No interrupt requests.
	return 0;
}

void cq_SetVector(MAP_IO *io, int newVector, int idx)
{
	CQ_DEVICE *cq  = (CQ_DEVICE *)io->uqDevice;
	uint32    nipl;

	if ((nipl = io->intIPL) && io->intMask[idx]) {
		UQ_IPL *ipl = &cq->iplList[nipl - UQ_BR4];
		
		io->intVector[idx] = newVector;
		ipl->intVecs[io->intLevel[idx]]  =
			newVector | (io->csrAddr ? 0x200 : 0);

#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: Set Vector %03o (%03X) on BR%d, Level %d\n",
				io->devName, newVector, newVector, nipl, io->intLevel[idx]);
#endif /* DEBUG */
	}
}

void cq_SendInterrupt(MAP_IO *io, int idx)
{
	CQ_DEVICE *cq  = (CQ_DEVICE *)io->uqDevice;
	VAX_CPU   *vax = cq->Processor;
	uint32    nipl, mask;

	if ((nipl = io->intIPL) && (mask = io->intMask[idx])) {
		cq->iplList[nipl - UQ_BR4].intReqs |= mask;
		SET_IRQ; // Evaluate IRQs.

#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Ring IRQ BR%d, Level %d, Vector %03o (%03X)\n",
				io->devName, nipl, io->intLevel[idx],
				io->intVector[idx], io->intVector[idx]);
#endif /* DEBUG */
	}
}

void cq_CancelInterrupt(MAP_IO *io, int idx)
{
	CQ_DEVICE *cq = (CQ_DEVICE *)io->uqDevice;
	VAX_CPU   *vax = cq->Processor;
	uint32    nipl, mask;

	if ((nipl = io->intIPL) && (mask = io->intMask[idx])) {
		cq->iplList[nipl-UQ_BR4].intReqs &= ~mask;
		SET_IRQ; // Evaluate IRQs.

#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Cancel IRQ BR%d, Level %d, Vector %03o (%03X)\n",
				io->devName, nipl, io->intLevel[idx],
				io->intVector[idx], io->intVector[idx]);
#endif /* DEBUG */
	}
}

// Read block to a Q22 I/O device
uint32 cq_ReadBlock(void *dptr, uint32 ioAddr, uint8 *data, uint32 szBytes, uint32 mode)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)dptr;
	CQ_DEVICE    *cq    = ka650->qba;
//	VAX_CPU      *vax   = ka650->Processor;
	uint32       mapAddr, idxAddr = 0;
	uint32       cntBytes;

	cntBytes = VA_PAGESIZE - VA_GETOFF(ioAddr);
	if (szBytes < cntBytes)
		cntBytes = szBytes;

	while (szBytes) {
		if (cq_MapAddr(cq, ioAddr & CQMEM_MASK, &mapAddr) == 0)
			return szBytes;
			
		memcpy(&data[idxAddr], &ka650->cpu.RAM[mapAddr], cntBytes);

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA)) {
			dbg_Printf("I/O Address: %08X  Host Address: %08X  Size: %d bytes\n",
				ioAddr, mapAddr, cntBytes);
			PrintDump(idxAddr, &data[idxAddr], cntBytes);
		}
#endif /* DEBUG */

		// Increment them by 512-bytes page.
		ioAddr   += cntBytes;
		idxAddr  += cntBytes;
		szBytes  -= cntBytes;
		cntBytes =  (szBytes > VA_PAGESIZE) ? VA_PAGESIZE : szBytes;
	}

	return szBytes;
}

// Write block from a Q22 I/O device
uint32 cq_WriteBlock(void *dptr, uint32 ioAddr, uint8 *data, uint32 szBytes, uint32 mode)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)dptr;
	CQ_DEVICE    *cq    = ka650->qba;
//	VAX_CPU      *vax   = ka650->Processor;
	uint32       mapAddr, idxAddr = 0;
	uint32       cntBytes;

	cntBytes = VA_PAGESIZE - VA_GETOFF(ioAddr);
	if (szBytes < cntBytes)
		cntBytes = szBytes;

	while (szBytes) {
		if (cq_MapAddr(cq, ioAddr & CQMEM_MASK, &mapAddr) == 0)
			return szBytes;

		memcpy(&ka650->cpu.RAM[mapAddr], &data[idxAddr], cntBytes);

#ifdef DEBUG
		if (dbg_Check(DBG_IODATA)) {
			dbg_Printf("I/O Address: %08X  Host Address: %08X  Size: %d bytes\n",
				ioAddr, mapAddr, cntBytes);
			PrintDump(idxAddr, &data[idxAddr], cntBytes);
		}
#endif /* DEBUG */

		// Increment them by 512-bytes page.
		ioAddr   += cntBytes;
		idxAddr  += cntBytes;
		szBytes  -= cntBytes;
		cntBytes =  (szBytes > VA_PAGESIZE) ? VA_PAGESIZE : szBytes;
	}

	return szBytes;
}

// *************************************************************

int cq_SetMap(void *dptr, MAP_IO *io)
{
	CQ_DEVICE *cq     = (CQ_DEVICE *)dptr;
	VAX_CPU   *vax    = cq->Processor;
	MAP_IO    *cptr, *pptr = NULL;
	UQ_IPL    *iplList     = NULL;
	uint32    csrAddr;
	int       idx, ipl, lvl;

	// Check free area first. If any reserved area,
	// Report that any or all area is already reserved.
	if (io->csrAddr) {
		csrAddr = (io->csrAddr & 017777) >> 1;
		for (idx = 0; idx < io->nRegs; idx++)
			if (cq->ioMap[csrAddr + idx])
				return UQ_RESERVED;
	}

	// Check free vector slots first.  If reserved area is
	// near full or full, report that.
	if (ipl = io->intIPL) {
		iplList = &cq->iplList[ipl - UQ_BR4];
		for (idx = 0; idx < io->nVectors; idx++) {
			// Find a first vacant slot.
			for (lvl = 0; lvl < UQ_NVECS; lvl++)
				if (((iplList->intRsvd >> lvl) & 1) == 0)
					break;
			// Check any space is enough to fit.
			if ((lvl + io->nVectors) > UQ_NVECS)
				return UQ_RSVDVEC;
		}
	}

	// Now assign device registers to call.
	if (io->csrAddr) {
		// Convert 16/18/22-bit address to 18-bit address.
		io->csrAddr = (io->csrAddr & 017777) | 0760000;
		for (idx = 0; idx < io->nRegs; idx++)
			cq->ioMap[csrAddr + idx] = io;
	}

	// Now assign interrupt vectors to interrupt table.
	if (iplList && io->nVectors) {
		for (idx = 0; idx < io->nVectors; idx++, lvl++) {
			// Assign vector slot to device.
			iplList->intRsvd      |= (1u << lvl);
			iplList->intVecs[lvl]  =
				 io->intVector[idx] | (io->csrAddr ? 0x200 : 0);
			iplList->ioMap[lvl]    = io;
			io->intLevel[idx]      = lvl;
			io->intMask[idx]       = (1u << lvl);
		}
	}

	// Set up callback functions for this Q22 interface.
	io->uqDevice        = cq;
	io->SetVector       = cq_SetVector;
	io->SendInterrupt   = cq_SendInterrupt;
	io->CancelInterrupt = cq_CancelInterrupt;

	// Insert a I/O node into a device list.
	if (cq->ioList == NULL) {
		io->Next   = NULL;
		cq->ioList = io;
	} else {
		for (cptr = cq->ioList; cptr; cptr = cptr->Next) {
			if (io->csrAddr < cptr->csrAddr)
				break;
			pptr = cptr;
		}
		if (cptr == cq->ioList) {
			io->Next = cq->ioList;
			cq->ioList = io;
		} else {
			io->Next = pptr->Next;
			pptr->Next = io;
		}
	}

	return UQ_OK;
}


//	nmap->sysDevice = map->Device;
//	nmap->sysMap    = map->sysMap;

// Usage: create <device> cqbic [<ipl> <vector>]
void *cq_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	CQ_DEVICE    *cq = NULL;
	MAP_IO       *io;
	KA650_DEVICE *ka650;

	if (cq = (CQ_DEVICE *)calloc(1, sizeof(CQ_DEVICE))) {
		cq->devName       = newMap->devName;
		cq->keyName       = newMap->keyName;
		cq->emuName       = newMap->emuName;
		cq->emuVersion    = newMap->emuVersion;
		newMap->Device    = cq;
		newMap->Callback  = &cq_Callback;
		newMap->sysDevice = newMap->devParent->Device;

		// Set up an IPCR register.
		io               = &cq->ipcr;
		io->Next         = NULL;
		io->devName      = NULL;
		io->keyName      = NULL;
		io->emuName      = "IPC Registers";
		io->emuVersion   = NULL;
		io->Device       = cq;
		io->csrAddr      = IPC_CSR;
		io->nRegs        = IPC_NREGS;
		io->nVectors     = IPC_NVECS;
		io->intIPL       = IPC_IPL;
		io->intVector[0] = IPC_VEC;
		io->ReadIO       = cq_ReadDBL;
		io->WriteIO      = cq_WriteDBL;
		cq_SetMap(cq, io);

		// Set up CQ and KA650 devices and
		// link them together.
		cq->idDevice    = 0;
		cq->System      = newMap->sysDevice;
		cq->Processor   = newMap->devParent->Device;
		cq->Callback    = &cq_Callback;
		ka650           = (KA650_DEVICE *)cq->Processor;
		ka650->qba      = cq;
		ka650->Callback = &cq_Callback;
		ka650->cpu.uqba = cq;
		ka650->cpu.Callback = &cq_Callback;

		// Set up Unibus/Qbus Function Calls.
		ka650->cpu.CheckIRQ   = cq_CheckIRQ;
		ka650->cpu.GetVector  = cq_GetVector;
	
		// Initialize Internal Devices
		ka650->cpu.InitIO((VAX_CPU *)ka650);
	}

	return cq;
}

int cq_Info(MAP_DEVICE *map, int argc, char **argv)
{
	CQ_DEVICE *cq = (CQ_DEVICE *)map->Device;
	UQ_IPL    *iplList;
	MAP_IO    *io;
	int       ipl, lvl;

	if (cq->ioList == NULL) {
		printf("\nNo I/O devices.\n\n");
		return UQ_OK;
	}

	// Display a listing of I/O device.
	for (io = cq->ioList; io; io = io->Next) {
		printf("%-20.20s %06o (%05X) %03o (%03X) %3d\n",
			io->emuName, io->csrAddr, io->csrAddr,
			io->intVector[0], io->intVector[1], io->nRegs);	
	}

	for (ipl = UQ_HLVL - 1; ipl >= 0; ipl--) {
		printf("Interrupt Level BR%d:\n\n", ipl + UQ_BR4);
		iplList = &cq->iplList[ipl];
		for (lvl = 0; lvl < UQ_NVECS; lvl++) {
			if (io = iplList->ioMap[lvl])
				printf("%-20.20s %06o (%05X) %03o (%03X) %3d\n",
					io->emuName, io->csrAddr, io->csrAddr,
					io->intVector[0], io->intVector[1], io->nRegs);	
		}
		printf("\n");
	}

	return UQ_OK;
}

// Callback functions for Q22-Bus interface device
UQ_CALL cq_Callback =
{
	NULL,           // Read Data I/O
	NULL,           // Write Data I/O
	cq_ReadBlock,   // Read Block I/O
	cq_WriteBlock,  // Write Block I/O
	NULL,           // Get Host Address

	cq_SetMap,      // SetMap Routine
};

DEVICE cq_Device =
{
	CQ_KEY,           // Key Name
	CQ_NAME,          //  Emulator Name
	CQ_VERSION,       // Emulator Version
	uq_Devices,       // Listing of Unibus/Q22-Bus Devices
	DF_USE|DF_SYSMAP, // Device Flags
	DT_DEVICE,        // Device Type

	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	cq_Create,        // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	NULL,             // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	cq_Info,          // Info Routine
	NULL,             // Boot Routine    (Not Used)
	NULL,             // Execute Routine (Not Used)
#ifdef DEBUG
	NULL,				   // Debug Routine
#endif /* DEBUG */
};
