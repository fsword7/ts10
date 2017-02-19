// uba.c - Complete listing of Unibus devices
//
// Copyright (c) 2003, Timothy M. Stark
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
// 01/21/03  TMS  Added DEUNA/DELUA (Unibus Ethernet) entries.
// 01/21/03  TMS  Added DELQA-PLUS (Turbo) entry.
// 01/21/03  TMS  Modification history started here.
//
// -------------------------------------------------------------------------

#include "emu/defs.h"
#include "dec/defs.h"

// External device structure from other files

extern DEVICE bdv_Device;
extern DEVICE dhu_Device;
extern DEVICE dhv_Device;
extern DEVICE dhq_Device;
extern DEVICE dl_Device;
extern DEVICE dz_Device;
extern DEVICE dzv_Device;
extern DEVICE kdf_Device;
extern DEVICE kwl_Device;
extern DEVICE mrv_Device;
extern DEVICE qna_Device;
//extern DEVICE lua_Device;
extern DEVICE lqa_Device;
extern DEVICE lqat_Device;
//extern DEVICE sqa_Device;
extern DEVICE rh_Device;
extern DEVICE rl_Device;
extern DEVICE rlv_Device;
extern DEVICE rq_Device;
extern DEVICE lp_Device;
extern DEVICE lp20_Device;
extern DEVICE tcu_Device;
extern DEVICE tq_Device;
extern DEVICE ts_Device;
extern DEVICE tsv_Device;
//extern DEVICE una_Device;

// Unibus/Qbus Devices for KS10, PDP11 and VAX series.
DEVICE *uq_Devices[] = {
	&bdv_Device,  // BDV11   - Boot ROM Device
	&dhq_Device,  // DHQ11   - DHV/DHU Terminal Interface
	&dhu_Device,  // DHU11   - Unibus Terminal Interface
	&dhv_Device,  // DHV11   - Qbus Terminal Interface
	&dl_Device,   // DL11    - Serial Line Interface (Console TTY)
	&dz_Device,   // DZ11    - Unibus Terminal Interface
	&dzv_Device,  // DZV11   - Qbus Terminal Interface
	&kdf_Device,  // KDF11   - Boot ROM Device
	&kwl_Device,  // KW11L   - Line Clock
	&mrv_Device,  // MRV11   - Boot ROM Device
	&qna_Device,  // DEQNA   - QBus Ethernet Interface
//	&lua_Device,  // DELUA   - Unibus Ethernet Interface
	&lqa_Device,  // DELQA   - QBus Ethernet Interface
	&lqat_Device, // DELQA-T - QBus Ethernet Interface
//	&sqa_Device,  // DESQA   - QBus Ethernet Interface
	&lp_Device,   // LP11    - Line Printer Interface
	&lp20_Device, // LP20    - Line Printer Interface for KS10 machines
	&rh_Device,   // RH11    - MASSBUS Disk/Tape Controller
	&rl_Device,   // RL11    - Unibus RL01/RL02 Disk Subsystems
	&rlv_Device,  // RLV11   - Qbus RL01/RL02 Disk Subsystems
	&rq_Device,   // RQDX3   - MSCP Disk Controller
	&tcu_Device,  // TCU     - Time Clock Unit
	&tq_Device,   // TQK50   - TMSCP Tape Controller
	&ts_Device,   // TS11    - Magtape Drive
	&tsv_Device,  // TSV05   - Magtape Drive
//	&una_Device,  // DEUNA   - Unibus Ethernet Interface
	NULL          // Terminator
};

int uq_GetAddr(UNIT *uptr, uint32 *csrAddr, uint32 *mskAddr,
	uint32 *vecAddr, uint32 *ipl, int argc, char **argv)
{
	if (argc < 4) {
		printf("%s: Usage: create %s: %s ... <csr> <mask> <vector> <ipl>\n",
			uptr->devName, uptr->devName, uptr->keyName);
		return UQ_ERROR;
	}

	sscanf(argv[0], "%o", csrAddr);
	sscanf(argv[1], "%o", mskAddr);
	sscanf(argv[2], "%o", vecAddr);
	sscanf(argv[3], "%d", ipl);

	printf("%s: CSR = %06o, MASK = %02o, VEC = %03o, IPL = %d\n",
		uptr->devName, *csrAddr, *mskAddr, *vecAddr, *ipl);

	return UQ_OK;
}

#if 0
int uq_SetMap(UQ_IPL *ioList, MAP_IO *io)
{
	MAP_IO    *cptr, *pptr = NULL;
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
#endif
