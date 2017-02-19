// ka630_qba.c - Q22 Bus Interface for MicroVAX II series
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
#include "vax/dev_u2qba.h"
#include "vax/ka630.h"

// Look at bottom of this file.
UQ_CALL u2qba_Callback;

int qba_ReadIPCR(void *dptr, uint32 pAddr, uint16 *data, uint32 size)
{
	QBA_DEVICE *qba = (QBA_DEVICE *)dptr;

	if (size == BYTE)
		*data = qba->ipcReg[0] >> ((pAddr & 1) ? 8 : 0) & BMASK;
	else
		*data = qba->ipcReg[0];

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) IPCR (%08X) %04X => %04X (Size: %d)\n",
			qba->devName, pAddr, qba->ipcReg[0], *data, size);
#endif /* DEBUG */

	return UQ_OK;
}

void u2qba_SendInterrupt(MAP_IO *, int);
int qba_WriteIPCR(void *dptr, uint32 pAddr, uint16 data, uint32 size)
{
	QBA_DEVICE *qba = (QBA_DEVICE *)dptr;

	if ((size == BYTE) && (pAddr & 1))
		data <<= 8;

	qba->ipcReg[0] = (qba->ipcReg[0] & ~IPCR_WMASK) | (data & IPCR_WMASK);
	qba->ipcReg[0] &= ~(data & IPCR_CMASK);
	if ((data & (IPCR_IE|IPCR_IRQ)) == (IPCR_IE|IPCR_IRQ)) {
		qba->ipcReg[0] |= IPCR_IRQ;
		u2qba_SendInterrupt(&qba->ioIPCR, 0);
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) IPCR (%08X) %04X <= %04X (Size: %d)\n",
			qba->devName, pAddr, qba->ipcReg[0], data, size);
#endif /* DEBUG */

	return UQ_OK;
}

// *************************************************************

// Read Longword Aligned (Q22-Bus Map Registers)
int32 u2qba_ReadMAP(QBA_DEVICE *qba, uint32 pAddr)
{
	uint32 reg  = (pAddr & 0x7FFF) >> 2;
	uint32 data = qba->mapRegs[reg];

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		VAX_CPU *vax = qba->Processor;
		dbg_Printf("QBA: (R) Map Register %d (%04X) => %08X at PC %08X\n",
			reg, pAddr & 0x7FFF, data, faultPC);
	}
#endif /* DEBUG */

	return data;
}

// Write Longword Aligned (Map Registers)
void u2qba_WriteMAP(QBA_DEVICE *qba, uint32 pAddr, uint32 data)
{
	uint32 reg = (pAddr & 0x7FFF) >> 2;

	qba->mapRegs[reg] = data & MAP_WMASK;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		VAX_CPU *vax = qba->Processor;
		dbg_Printf("QBA: (W) Map Register %d (%04X) <= %08X at PC %08X\n",
			reg, pAddr & 0x7FFF, data, faultPC);
	}
#endif /* DEBUG */
}

// Read data aligned to a Q22 I/O device.
uint32 u2qba_ReadMEM(void *dptr, uint32 pAddr, uint32 size)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)dptr;
	VAX_CPU      *vax   = (VAX_CPU *)ka630;
	QBA_DEVICE   *qba   = ka630->qba;
	uint32 mAddr;

	// Translate to I/O Address from low/high CPU address
	// High CPU Address - 30000000 to 303FFFFF
	pAddr &= 0x3FFFFF;

	if (qba->mapRegs[pAddr >> MAP_N_OFF] & MAP_V) {
		mAddr = (qba->mapRegs[pAddr >> MAP_N_OFF] << MAP_N_OFF) |
			(pAddr & MAP_OFF);

#ifdef TEST_PARITY
		// "This memory location is bad" Test
		if ((MSER & MSER_PEN) &&
	 	    ((mAddr == ka630->q22Addr1) || (mAddr == ka630->q22Addr2))) {
#ifdef DEBUG
			dbg_Printf("PAR: Parity Error Trap on loc %08X at PC %08X\n",
				mAddr, faultPC);
#endif /* DEBUG */
			MSER |= (MSER_CD|MSER_QPE);
			if (ka630->q22State++ < 1)
				CEAR = pAddr >> 9;
			else
				MSER |= MSER_LEB;

			// Go to machine check exception
			P1 = 0;
			P2 = pAddr;
			P3 = MCHK_RBP;
			ABORT(-SCB_MCHK);
		}
#endif /* TEST_PARITY */

		if (size >= LONG)
			return LMEM(mAddr >> 2);
		else if (size == WORD)
			return WMEM(mAddr >> 1);
		else
			return BMEM(mAddr);

	}

	// Non-Existant Memory Error
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("QBA: (R) Non-Existant Memory at PC %08X\n", faultPC);
#endif /* DEBUG */

	MSER |= (MSER_CD|MSER_NXM);

	// Go to machine check exception
	P1 = 0;
	P2 = pAddr;
	P3 = MCHK_RBP;
	ABORT(-SCB_MCHK);
}

// Write data aligned from a Q22 I/O Device
void u2qba_WriteMEM(void *dptr, uint32 pAddr, uint32 data, uint32 size)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)dptr;
	VAX_CPU      *vax   = (VAX_CPU *)ka630;
	QBA_DEVICE   *qba   = ka630->qba;
	uint32 mAddr;

	// Translate to I/O Address from CPU address
	// CPU Address - 30000000 to 303FFFFF
	pAddr &= 0x3FFFFF;

	if (qba->mapRegs[pAddr >> MAP_N_OFF] & MAP_V) {
		mAddr = (qba->mapRegs[pAddr >> MAP_N_OFF] << MAP_N_OFF) |
			(pAddr & MAP_OFF);

#ifdef TEST_PARITY
		if (MSER & MSER_PEN) {
			if (MSER & MSER_WWP) {
				ka630->cpuAddr = mAddr;
				if (ka630->q22Addr1 == -1)
					ka630->q22Addr1 = mAddr;
				else
					ka630->q22Addr2 = mAddr;
#ifdef DEBUG
				dbg_Printf("PAR: Set Wrong Parity on loc %08X at PC %08X\n",
					mAddr, faultPC);
#endif /* DEBUG */
			} else if (ka630->cpuAddr == mAddr) {
				ka630->cpuAddr = -1;
				if (ka630->q22Addr1 == mAddr)
					ka630->q22Addr1 = -1;
				else if (ka630->q22Addr2 == mAddr)
					ka630->q22Addr2 = -1;
#ifdef DEBUG
				dbg_Printf("PAR: Clear Wrong Parity on loc %08X at PC %08X\n",
					mAddr, faultPC);
#endif /* DEBUG */
			}
		}
#endif /* TEST_PARITY */

		if (size >= LONG)
			LMEM(mAddr >> 2) = data;
		else if (size == WORD)
			WMEM(mAddr >> 1) = data;
		else
			BMEM(mAddr) = data;
		return;
	}

	// Non-Existant Memory Error
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("QBA: (W) Non-Existant Memory at PC %08X\n", faultPC);
#endif /* DEBUG */

	MSER |= (MSER_CD|MSER_NXM);

	// Go to machine check exception
	P1 = 0;
	P2 = pAddr;
	P3 = MCHK_WBP;
	ABORT(-SCB_MCHK);
}


// Read block to a Q22 I/O device
uint32 u2qba_ReadBlockIO(void *dptr, uint32 ioAddr,
	uint8 *data, uint32 szBytes, uint32 mode)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)dptr;
	QBA_DEVICE   *qba   = ka630->qba;
//	VAX_CPU      *vax   = ka630->Processor;
	uint32       mapAddr, idxAddr = 0;
	uint32       cntBytes;

	cntBytes = MAP_PAGSZ - (ioAddr & MAP_OFF);
	if (szBytes < cntBytes)
		cntBytes = szBytes;

	while (szBytes) {
		if ((qba->mapRegs[ioAddr >> MAP_N_OFF] & MAP_V) == 0) {
			// Non-Existant Memory Error
			MSER |= (MSER_CD|MSER_NXM);

// 		Go to machine check exception
//			P1 = 0;
//			P2 = ioAddr;
//			P3 = MCHK_RBP;
//			ABORT(-SCB_MCHK);

			return szBytes;
		}
			
		mapAddr = (qba->mapRegs[ioAddr >> MAP_N_OFF] << MAP_N_OFF) |
			(ioAddr & MAP_OFF);

		memcpy(&data[idxAddr], &ka630->cpu.RAM[mapAddr], cntBytes);

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
		cntBytes =  (szBytes > MAP_PAGSZ) ? MAP_PAGSZ : szBytes;
	}

	return szBytes;
}

// Write block from a Q22 I/O device
uint32 u2qba_WriteBlockIO(void *dptr, uint32 ioAddr,
	uint8 *data, uint32 szBytes, uint32 mode)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)dptr;
	QBA_DEVICE   *qba   = ka630->qba;
//	VAX_CPU      *vax   = ka630->Processor;
	uint32       mapAddr, idxAddr = 0;
	uint32       cntBytes;

	cntBytes = MAP_PAGSZ - (ioAddr & MAP_OFF);
	if (szBytes < cntBytes)
		cntBytes = szBytes;

	while (szBytes) {
		if ((qba->mapRegs[ioAddr >> MAP_N_OFF] & MAP_V) == 0) {
			// Non-Existant Memory Error
			MSER |= (MSER_CD|MSER_NXM);

// 		Go to machine check exception
//			P1 = 0;
//			P2 = ioAddr;
//			P3 = MCHK_RBP;
//			ABORT(-SCB_MCHK);

			return szBytes;
		}
			
		mapAddr = (qba->mapRegs[ioAddr >> MAP_N_OFF] << MAP_N_OFF) |
			(ioAddr & MAP_OFF);

		memcpy(&ka630->cpu.RAM[mapAddr], &data[idxAddr], cntBytes);

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
		cntBytes =  (szBytes > MAP_PAGSZ) ? MAP_PAGSZ : szBytes;
	}

	return szBytes;
}

// *************************************************************
 
// Read Word Aligned (Q22 I/O Space)
int qba_ReadIO(QBA_DEVICE *qba, uint32 ioAddr, uint16 *data, uint32 size)
{
	MAP_IO *io;

	// Access a desired I/O device
	if (io = qba->ioMap[ioAddr >> 1])
		return io->ReadIO(io->Device, ioAddr, data, size);

	return UQ_NXM;
}

// Write Word Aligned (Q22 I/O Space)
int qba_WriteIO(QBA_DEVICE *qba, uint32 ioAddr, uint32 data, uint32 size)
{
	MAP_IO *io;

	// Access a desired I/O device
	if (io = qba->ioMap[ioAddr >> 1])
		return io->WriteIO(io->Device, ioAddr, data, size);

	return UQ_NXM;
}

// Q22 Bus Initialization Routine
void qba_ResetAll(QBA_DEVICE *qba)
{
	VAX_CPU *vax = qba->Processor;
	MAP_IO  *io;

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: Q22 Bus Initialization Begins at PC %08X:\n",
			qba->devName, faultPC);
#endif /* DEBUG */

	for (io = qba->ioList; io; io = io->Next) {
#ifdef DEBUG
//		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s:   Reseting %s: (%s) ... ",
				qba->devName, io->devName, io->keyName);
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
		dbg_Printf("%s: End of Q22 Bus Initalization.\n", qba->devName);
#endif /* DEBUG */
}

// ********************* Hardware Interrupts ********************

// Evaluate Interrupts
uint32 u2qba_CheckIRQ(VAX_CPU *vax, uint32 nipl)
{
	QBA_DEVICE *qba = (QBA_DEVICE *)vax->uqba;
	int32      idx;

	for (idx = UQ_HLVL-1; idx >= 0; idx--) {
		if ((idx + IPL_HMIN) <= nipl)
			return 0;
		if (qba->iplList[idx].intReqs)
			return idx + IPL_HMIN;
	}
	return 0;
}

// Acknowledge Interrupt and Get Vector
uint16 u2qba_GetVector(VAX_CPU *vax, uint32 nipl)
{
	QBA_DEVICE *qba = (QBA_DEVICE *)vax->uqba;
	UQ_IPL     *ipl = &qba->iplList[nipl];
	int        lvl, vec;
	MAP_IO     *io;

	lvl = ipl->intReqs;
	for (vec = 0; vec < UQ_NVECS; vec++) {
		if ((lvl >> vec) & 1) {
			ipl->intReqs &= ~(1u << vec);
//			if ((io = ipl->ioMap[vec]) && io->AckInterrupt)
//				return io->AckInterrupt(io->Device);
#ifdef DEBUG
			if (dbg_Check(DBG_INTERRUPT))
				dbg_Printf("%s: IRQ BR%d, Level %d, Vector %03o\n",
					qba->devName, nipl + UQ_BR4, vec, ipl->intVecs[vec]);
#endif /* DEBUG */
			return ipl->intVecs[vec];
		}
	}

	// No interrupt requests.
	return 0;
}

void u2qba_SetVector(MAP_IO *io, int newVector, int idx)
{
	QBA_DEVICE *qba = (QBA_DEVICE *)io->uqDevice;
	uint32     nipl;

	if ((nipl = io->intIPL) && io->intMask[idx]) {
		UQ_IPL *ipl = &qba->iplList[nipl - UQ_BR4];
		
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

void u2qba_SendInterrupt(MAP_IO *io, int idx)
{
	QBA_DEVICE *qba = (QBA_DEVICE *)io->uqDevice;
	VAX_CPU    *vax = qba->Processor;
	uint32     nipl, mask;

	if ((nipl = io->intIPL) && (mask = io->intMask[idx])) {
		qba->iplList[nipl - UQ_BR4].intReqs |= mask;
		SET_IRQ; // Evaluate IRQs.

#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Ring IRQ BR%d, Level %d, Vector %03o (%03X)\n",
				io->devName, nipl, io->intLevel[idx],
				io->intVector[idx], io->intVector[idx]);
#endif /* DEBUG */
	}
}

void u2qba_CancelInterrupt(MAP_IO *io, int idx)
{
	QBA_DEVICE *qba = (QBA_DEVICE *)io->uqDevice;
	VAX_CPU    *vax = qba->Processor;
	uint32     nipl, mask;

	if ((nipl = io->intIPL) && (mask = io->intMask[idx])) {
		qba->iplList[nipl-UQ_BR4].intReqs &= ~mask;
		SET_IRQ; // Evaluate IRQs.

#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Cancel IRQ BR%d, Level %d, Vector %03o (%03X)\n",
				io->devName, nipl, io->intLevel[idx],
				io->intVector[idx], io->intVector[idx]);
#endif /* DEBUG */
	}
}

// *************************************************************

int u2qba_SetMap(void *dptr, MAP_IO *io)
{
	QBA_DEVICE *qba = (QBA_DEVICE *)dptr;
	MAP_IO     *cptr, *pptr = NULL;
	UQ_IPL     *iplList     = NULL;
	uint32     csrAddr;
	int        idx, ipl, lvl;

	// Check free area first. If any reserved area,
	// Report that any or all area is already reserved.
	if (io->csrAddr) {
		csrAddr = (io->csrAddr & 017777) >> 1;
		for (idx = 0; idx < io->nRegs; idx++)
			if (qba->ioMap[csrAddr + idx])
				return UQ_RESERVED;
	}

	// Check free vector slots first.  If reserved area is
	// near full or full, report that.
	if (ipl = io->intIPL) {
		iplList = &qba->iplList[ipl - UQ_BR4];
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
			qba->ioMap[csrAddr + idx] = io;
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
	io->uqDevice        = qba;
	io->SetVector       = u2qba_SetVector;
	io->SendInterrupt   = u2qba_SendInterrupt;
	io->CancelInterrupt = u2qba_CancelInterrupt;

	// Insert a I/O node into a device list.
	if (qba->ioList == NULL) {
		io->Next   = NULL;
		qba->ioList = io;
	} else {
		for (cptr = qba->ioList; cptr; cptr = cptr->Next) {
			if (io->csrAddr < cptr->csrAddr)
				break;
			pptr = cptr;
		}
		if (cptr == qba->ioList) {
			io->Next = qba->ioList;
			qba->ioList = io;
		} else {
			io->Next = pptr->Next;
			pptr->Next = io;
		}
	}

	return UQ_OK;
}

// nmap->sysDevice = map->Device;
// nmap->sysMap    = map->sysMap;

// Usage: create <device> u2qba [<ipl> <vector>]
void *qba_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	QBA_DEVICE   *qba = NULL;
	MAP_IO       *ipcr;
	KA630_DEVICE *ka630;

	if (qba = (QBA_DEVICE *)calloc(1, sizeof(QBA_DEVICE))) {
		qba->devName      = newMap->devName;
		qba->keyName      = newMap->keyName;
		qba->emuName      = newMap->emuName;
		qba->emuVersion   = newMap->emuVersion;

		// Set up U2QBA and KA630 devices and
		// link them together.
		qba->idDevice   = 0;
		qba->System     = newMap->sysDevice;
		qba->Processor  = newMap->devParent->Device;
		qba->Callback   = &u2qba_Callback;
		ka630           = (KA630_DEVICE *)qba->Processor;
		ka630->qba      = qba;
		ka630->Callback = qba->Callback;
		ka630->cpu.uqba = qba;
		ka630->cpu.Callback = qba->Callback;	

		ipcr               = &qba->ioIPCR;
		ipcr->Next         = NULL;
		ipcr->devName      = NULL;
		ipcr->keyName      = NULL;
		ipcr->emuName      = "IPCR Registers";
		ipcr->emuVersion   = NULL;
		ipcr->Device       = qba;
		ipcr->csrAddr      = IPCR_CSR;
		ipcr->nRegs        = IPCR_NREGS;
		ipcr->nVectors     = IPCR_NVECS;
		ipcr->intIPL       = IPCR_IPL;
		ipcr->intVector[0] = IPCR_VEC;
		ipcr->ReadIO       = qba_ReadIPCR;
		ipcr->WriteIO      = qba_WriteIPCR;
		u2qba_SetMap(qba, ipcr);

		// Set up Unibus/Qbus function calls.
		ka630->cpu.CheckIRQ  = u2qba_CheckIRQ;
		ka630->cpu.GetVector = u2qba_GetVector;

		ka630->cpu.InitIO((VAX_CPU *)ka630);
	
		// Finally, set up its descriptions and
		// Link it to mapping device.
		newMap->Device    = qba;
		newMap->Callback  = qba->Callback;
		newMap->sysDevice = qba->Processor;
	}

	return qba;
}

int qba_Info(MAP_DEVICE *map, int argc, char **argv)
{
	QBA_DEVICE *qba = (QBA_DEVICE *)map->Device;
	MAP_IO     *io;

	// Display a listing of I/O device.
	if (qba->ioList) {
		for (io = qba->ioList; io; io = io->Next) {
			printf("%-20s %06o (%05X) %03o (%03X) %3d\n",
				io->emuName, io->csrAddr, io->csrAddr,
				io->intVector[0], io->intVector[0], io->nRegs);	
		}
	} else
		printf("\nNo I/O devices.\n\n");

	return UQ_OK;
}

UQ_CALL u2qba_Callback =
{
	NULL,                // Read Data I/O
	NULL,                // Write Data I/O
	u2qba_ReadBlockIO,   // Read Block I/O
	u2qba_WriteBlockIO,  // Write Block I/O
	NULL,                // Get Host Address

	u2qba_SetMap,        // SetMap Routine
};

DEVICE qba_Device =
{
	QBA_KEY,          // Key Name
	QBA_NAME,         // Emulator Name
	QBA_VERSION,      // Emulator Version
	uq_Devices,       // Listing of Unibus/Q22-Bus Devices
	DF_USE|DF_SYSMAP, // Device Flags
	DT_DEVICE,        // Device Type

	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	qba_Create,       // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	NULL,             // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	qba_Info,         // Info Routine
	NULL,             // Boot Routine    (Not Used)
	NULL,             // Execute Routine (Not Used)
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
