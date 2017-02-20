// rl.c - RL11/RLV11: RL01/RL02 Disk Systems
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

#include "emu/defs.h"
#include "dec/rl.h"

#ifdef DEBUG
static cchar *regName[] = {
	"RLCS",  // Control and Status Register
	"RLBA",  // Bus Address Register
	"RLDA",  // Disk Address Register
	"RLMP",  // Multipurpose Register
	"RLBAE"  // Bus Address Extension Register (RLV11 only)
};

static cchar *fncName[] = {
	"No Operation",
	"Write Check",
	"Get Status",
	"Seek",
	"Read Header",
	"Write Data",
	"Read Data",
	"Read Data Without Header Check"
};
#endif /* DEBUG */

// Bus Initialization
void rl_ResetDevice(RL_DEVICE *rl)
{
	rl->rlcs  = RLCS_CRDY;
	rl->rlba  = 0;
	rl->rlda  = 0;
	rl->rlmp  = 0;
	rl->rlmp1 = 0;
	rl->rlmp2 = 0;
	rl->rlbae = 0;
}

void rl_SetDone(RL_DRIVE *drv, uint16 st)
{
	RL_DEVICE *rl = drv->Device;

	drv->rlcs |= RLCS_DRDY;
	rl->rlcs  |= (st | CSR_DONE);
	if (rl->rlcs & CSR_IE)
		rl->ioMap.SendInterrupt(&rl->ioMap, 0);
}

// Initialize DEC standard 044 bad block table.
int rl_InitBadBlocks(RL_DRIVE *drv)
{
	uint32 dskAddr = drv->totBytes - (RL_NSECS * RL_BSEC);
	uint16 buf[RL_WSEC];
	uint32 idx;

	buf[0]  = 0x1234;
	buf[1]  = 0x5678;
	buf[2]  = 0;
	buf[3]  = 0;

	for (idx = 4; idx < RL_WSEC; idx++)
		buf[idx] = 0177777u;

	lseek(drv->File, dskAddr, SEEK_SET);
	for (idx = 0; idx < RL_NSECS; idx++)
		write(drv->File, (char *)buf, RL_BSEC);
}

void rl_Service(void *dptr)
{
	RL_DEVICE *rl      = (RL_DEVICE *)dptr;
	RL_DRIVE  *drv     = &rl->Drives[GET_DS(rl->rlcs)];
	uint16    func     = GET_FUNC(rl->rlcs);
	uint8     *bufData = &rl->bufData[0];
	uint8     *bufComp;
	uint32    dskAddr, hstAddr;
	uint16    wCount, awCount;
	int16     xfrCount, maxCount;
	uint32    idx;
	int32     errCode = 0;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Function Code: %d, %s\n", drv->devName,
			func, (func < 8 ? fncName[func] : "(Unknown function)"));
#endif /* DEBUG */
	
	// Get Status
	if (func == FUNC_STATUS) {
		if (rl->rlda & RLDA_GS_CLR)
			drv->rlds &= ~RLDS_ERR;
		rl->rlmp = drv->rlds | (drv->track & RLDS_HD) |
			((drv->Flags & DFLG_ATTACHED) ? RLDS_ATT : RLDS_UNATT) |
			((drv->Flags & DFLG_RL02) ? RLDS_RL02 : 0) |
			((drv->Flags & DFLG_WLOCK) ? RLDS_WLK : 0);
		rl->rlmp2 = rl->rlmp1 = rl->rlmp;
		rl_SetDone(drv, 0);
		return;
	}
	
	// Check if disk cartridge is mounted or not.
	if ((drv->Flags & DFLG_ATTACHED) == 0) {
		drv->rlcs &= ~RLCS_DRDY;
		drv->rlds |= RLDS_SPE; // Spin Error
		rl_SetDone(drv, RLCS_ERR|RLCS_OPI);
		return;
	}

	switch (func) {
		case FUNC_SEEK:    // Seek
			rl_SetDone(drv, 0);
			return;

		case FUNC_RDHDR:   // Read Header
			rl->rlmp  = (drv->track & RLDA_TRACK) | GET_SECTOR(rl->rlda);
			rl->rlmp2 = rl->rlmp1 = 0;
			rl_SetDone(drv, 0);
			return;

		case FUNC_WRITE:   // Write Data
			// For write access, check write lock first.
			if (drv->Flags & DFLG_WLOCK) {
				drv->rlds |= RLDS_WGE;
				rl_SetDone(drv, RLCS_ERR|RLCS_DRE);
				return;
			}
			// Fall through...

		case FUNC_WRCHK:   // Write Check
		case FUNC_READ:    // Read Data
		case FUNC_RDNOHDR: // Read Without Header
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s:   Track: %d DA: %d  Sector: %d\n",
					drv->devName, GET_CYL(drv->track), GET_CYL(rl->rlda),
					GET_SECTOR(rl->rlda));
#endif /* DEBUG */
			if (((func != FUNC_RDNOHDR) &&
			    ((drv->track & RLDA_CYL) != (rl->rlda & RLDA_CYL))) ||
				 (GET_SECTOR(rl->rlda) >= RL_NSECS)) {
				rl_SetDone(drv, RLCS_ERR|RLCS_HDE|RLCS_OPI);
				return;
			}

			// Get disk address and memory address to
			// prepare for data transfers.
			dskAddr = GET_DA(rl->rlda) * RL_BSEC;
			hstAddr = (rl->rlbae << 16) + rl->rlba;

			// Get word count and maxize data transfer.
			wCount  = 0200000 - rl->rlmp;
			maxCount = (RL_NSECS - GET_SECTOR(rl->rlda)) * RL_WSEC;
			if (wCount > maxCount)
				wCount = maxCount;

			// Now perform data transfers.
			if (lseek(drv->File, dskAddr, SEEK_SET) < 0) {
				errCode = errno;
			} else {
				UQ_CALL *call = rl->Callback;

				if (func == FUNC_READ) {
					// Read Data
					if ((xfrCount = read(drv->File, bufData, (wCount << 1))) < 0)
						errCode = errno;
					for (; xfrCount < wCount; xfrCount++)
						bufData[xfrCount] = 0; // Fill buffer
					if (xfrCount = call->WriteBlock(rl->System, hstAddr, bufData, wCount << 1, 0)) {
						rl->rlcs |= (RLCS_ERR|RLCS_NXM);
						wCount -= xfrCount >> 1;
					}
#ifdef DEBUG
//					if (dbg_Debug(DBG_IODATA)) {
//						dbg_Printf("%s: Read Data\n", drv->devName);
//						PrintDump(0, bufData, wCount << 1);
//					}
#endif /* DEBUG */
				} else if (func == FUNC_WRITE) {
					// Write Data
					if (xfrCount = call->ReadBlock(rl->System, hstAddr, bufData, wCount << 1, 0)) {
						rl->rlcs |= (RLCS_ERR|RLCS_NXM);
						wCount -= xfrCount >> 1;
					}
					if (wCount) {
						uint16 awCount = (wCount + (RL_WSEC - 1)) & ~(RL_WSEC - 1);
						uint16 idx;

						for (idx = wCount; idx < awCount; idx++)
							bufData[idx] = 0; // Fill buffer
						if (write(drv->File, bufData, (wCount << 1)) < 0)
							errCode = errno;
					}
#ifdef DEBUG
//					if (dbg_Debug(DBG_IODATA)) {
//						dbg_Printf("%s: Write Data\n", drv->devName);
//						PrintDump(0, bufData, wCount << 1);
//					}
#endif /* DEBUG */
				} else if (func == FUNC_WRCHK) {
					// Write Check
					if ((xfrCount = read(drv->File, bufData, wCount << 1)) < 0)
						errCode = errno;
					for (; xfrCount < (wCount << 1); xfrCount++)
						bufData[xfrCount] = 0; // Fill buffer
					if (errCode > 0)
						break;
					bufComp = &rl->bufData[wCount << 1];
					if (xfrCount = call->ReadBlock(rl->System, hstAddr, bufComp, wCount << 1, 0)) {
						rl->rlcs |= (RLCS_ERR|RLCS_NXM);
						wCount -= xfrCount << 1;
					}
					if (memcmp(bufData, bufComp, wCount << 1))
						rl->rlcs |= (RLCS_ERR|RLCS_CRC);
#ifdef DEBUG
					if (dbg_Check(DBG_IODATA)) {
						dbg_Printf("%s:   Data on disk.\n", drv->devName);
						PrintDump(0, bufData, wCount << 1);
						dbg_Printf("%s:   Compare with that.\n", drv->devName);
						PrintDump(0, bufComp, wCount << 1);
						dbg_Printf("%s:   memcmp = %d\n", drv->devName,
							memcmp(bufData, bufComp, wCount << 1));
					}
#endif /* DEBUG */
				}
			}

#ifdef DEBUG
			if (dbg_Check(DBG_IODATA) && (errCode > 0))
//			if (errCode > 0)
				dbg_Printf("%s:   I/O Error Code: %d, %s\n", drv->devName,
					errCode, strerror(errCode));
#endif /* DEBUG */

			// Done, update registers.
			hstAddr  += (wCount << 1);
			rl->rlda += ((wCount + (RL_WSEC - 1)) / RL_WSEC);
			rl->rlmp += wCount;
			if (rl->rlmp != 0)
				rl->rlcs |= (RLCS_ERR|RLCS_OPI);
			rl->rlba  = hstAddr;
			rl->rlbae = hstAddr >> 16;
			rl->rlcs  = PUT_BAE(rl->rlbae) | (rl->rlcs & ~RLCS_BAE);
			rl_SetDone(drv, 0); // Ring host.
			break;

		default:  // Illegal Function
			break;
	}
}

inline void rl_Execute(RL_DEVICE *rl)
{
	RL_DRIVE *drv = &rl->Drives[GET_DS(rl->rlcs)];

	// First, clear interrupts, errors, etc.
	rl->ioMap.CancelInterrupt(&rl->ioMap, 0);
	rl->rlcs  &= ~RLCS_ERRS; // Clear all error bits
	drv->rlcs &= ~RLCS_DRDY;

	switch (GET_FUNC(rl->rlcs)) {
		case FUNC_NOP:  // No Operation
			rl_SetDone(drv, 0);
			break;

		case FUNC_SEEK: // Seek Operation
			// RL11/RLV11 manual said that must set up registers
			// first before it can start perform seek operation.
			{
				uint16 ccyl = GET_CYL(drv->track);
				uint16 off  = GET_CYL(rl->rlda);
				int16  ncyl;

				// Move head to desired cylinder.
				if (rl->rlda & RLDA_SK_DIR) {
					// Move head outward
					ncyl = ccyl + off;
					if (ncyl >= drv->totCyls)
						ncyl = drv->totCyls - 1;
				} else {
					// Move head inward
					ncyl = ccyl - off;
					if (ncyl < 0) ncyl = 0;
				}

				// Set up new track for seek operation.
				drv->track = PUT_CYL(ncyl) |
					((rl->rlda & RLDA_SK_HD) ? RLDA_HD1 : RLDA_HD0);
			}
			// Fall through...

		default:
			rl_Service(rl);
//			ts10_SetTimer(&rl->svcTimer);
	}
}

int rl_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	RL_DEVICE *rl = (RL_DEVICE *)dptr;
	uint32    reg = (pAddr - (rl->csrAddr & 0x1FFF)) >> 1;

	switch(reg) {
		case RLCS:  // Control and Status Register
			*data = rl->rlcs | rl->Drives[GET_DS(rl->rlcs)].rlcs;
			break;

		case RLBA:  // Bus Address Register
			*data = rl->rlba;
			break;

		case RLDA:  // Disk Address Register
			*data = rl->rlda;
			break;

		case RLMP:  // Multipurpose Register
			*data     = rl->rlmp;
			rl->rlmp  = rl->rlmp1;
			rl->rlmp1 = rl->rlmp2;
			break;

		case RLBAE: // Bus Address Extension Register (RLV11 only)
			if (rl->Flags & CFLG_RLV11) {
				*data = rl->rlbae;
				break;
			}
			// Fall through...

#ifdef DEBUG
		default:
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				rl->devName, reg, pAddr & 0x1FFF, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%o) => %06o (%04X) (Size: %d bytes)\n",
			rl->devName, regName[reg], pAddr, *data, *data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

int rl_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	RL_DEVICE *rl = (RL_DEVICE *)dptr;
	uint32    reg = (pAddr - (rl->csrAddr & 0x1FFF)) >> 1;

	switch(reg) {
		case RLCS:  // Control and Status Register
			if (acc == ACC_BYTE) // If access is byte, merge with RLDA.
				data = (pAddr & 1) ? ((data << 8)   | (rl->rlcs & 0377)) :
				                     ((data & 0377) | (rl->rlcs & ~0377));

			// Update CSR bits
			rl->rlcs = (data & RLCS_RW) | (rl->rlcs & ~RLCS_RW);
			if (rl->Flags & CFLG_RLV11)
				rl->rlbae = ((data >> RLCS_P_BAE) & RLCS_M_BAE) |
					(rl->rlbae & ~RLCS_M_BAE);

			// Check interrupt enable
			if (data & CSR_DONE) {
				if ((data & CSR_IE) == 0)
					rl->ioMap.CancelInterrupt(&rl->ioMap, 0);
				else if ((rl->rlcs & (CSR_DONE|CSR_IE)) == CSR_DONE)
					rl->ioMap.SendInterrupt(&rl->ioMap, 0);
				break;
			}

			// Now execute function codes...
			rl_Execute(rl);
			break;

		case RLBA:  // Bus Address Register
			if (acc == ACC_BYTE) // If access is byte, merge with RLDA.
				data = (pAddr & 1) ? ((data << 8)   | (rl->rlba & 0377)) :
				                     ((data & 0377) | (rl->rlba & ~0377));
			rl->rlba = data & RLBA_IMP;
			break;

		case RLDA:  // Disk Address Register
			if (acc == ACC_BYTE) // If access is byte, merge with RLDA.
				data = (pAddr & 1) ? ((data << 8)   | (rl->rlda & 0377)) :
				                     ((data & 0377) | (rl->rlda & ~0377));
			rl->rlda = data;
			break;

		case RLMP:  // Multipurpose Register
			if (acc == ACC_BYTE) // If access is byte, merge with RLDA.
				data = (pAddr & 1) ? ((data << 8)   | (rl->rlmp & 0377)) :
				                     ((data & 0377) | (rl->rlmp & ~0377));
			rl->rlmp  = data;
			rl->rlmp1 = data;
			rl->rlmp2 = data;
			break;

		case RLBAE: // Bus Address Extension Register
			if (rl->Flags & CFLG_RLV11) {
				if (pAddr & 1)
					break;
				rl->rlbae = data & RLBAE_IMP;
				rl->rlcs  = ((data & RLCS_M_BAE) << RLCS_P_BAE) |
					(rl->rlcs & ~ RLCS_BAE);
				break;
			}
			// Fall through...

#ifdef DEBUG
		default:
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				rl->devName, reg, pAddr & 0x1FFF, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) %s (%o) <= %06o (%04X) (Size: %d bytes)\n",
			rl->devName, regName[reg], pAddr, data, data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

// Bus Initialization
void rl_ResetIO(void *dptr)
{
	rl_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// **************************************************************

void *rl_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	RL_DEVICE *rl = NULL;
	MAP_IO    *io;
	uint32    idx;

	if (rl = (RL_DEVICE *)calloc(1, sizeof(RL_DEVICE))) {
		// First, set up its description and
		// link it to its parent device.
		rl->devName    = newMap->devName;
		rl->keyName    = newMap->keyName;
		rl->emuName    = newMap->emuName;
		rl->emuVersion = newMap->emuVersion;
		rl->Device     = newMap->devParent->Device;
		rl->Callback   = newMap->devParent->Callback;
		rl->System     = newMap->devParent->sysDevice;

		if (!strcmp(rl->keyName, RL_KEY))
			rl->Flags = CFLG_RL11;
		else if (!strcmp(rl->keyName, RLV_KEY))
			rl->Flags = CFLG_RLV11;
		else {
			printf("%s: *** Bug Check: Unknown device name - %s\n",
				rl->devName, rl->keyName);
			free(rl);
			return NULL;
		}

		rl->csrAddr = RL_IOADDR;

		// Set up an I/O space.
		io               = &rl->ioMap;
		io->devName      = rl->devName;
		io->keyName      = rl->keyName;
		io->emuName      = rl->emuName;
		io->emuVersion   = rl->emuVersion;
		io->Device       = rl;
		io->csrAddr      = rl->csrAddr;
		io->nRegs        =
			(rl->Flags & CFLG_RLV11) ? RLV_NREGS : RL_NREGS;
		io->nVectors     = RL_NVECS;
		io->intIPL       = RL_IPL;
		io->intVector[0] = RL_VEC;
		io->ReadIO       = rl_ReadIO;
		io->WriteIO      = rl_WriteIO;
		io->ResetIO      = rl_ResetIO;

		// Assign that registers to QBA's I/O space.
		rl->Callback->SetMap(rl->Device, io);

		// Initialize RL01/RL02 Drives
		for (idx = 0; idx < RL_NDRVS; idx++) {
			RL_DRIVE *drv = &rl->Drives[idx];

			drv->idUnit = idx; // Unit/Drive identification
			drv->Device = rl;  // Link this to parent device.
		}

		// Finally, link it to its mapping device and return.
		newMap->Device = rl;
	}

	return rl;
}

void *rl_Configure(MAP_DEVICE *newMap, int argc, char **argv)
{
	RL_DEVICE *rl = (RL_DEVICE *)newMap->devParent->Device;
	RL_DRIVE  *drv;
	uint32    idUnit, idx;

	idUnit = GetDeviceUnit(newMap->devName);
	if (idUnit < RL_NDRVS) {
		drv = &rl->Drives[idUnit];

		// Check if unit is existing or not.
		if (drv->Flags & DFLG_EXIST) {
			printf("%s: Already created on %s:\n",
				newMap->devName, rl->devName);
			return NULL;
		}

		if (!strcasecmp(argv[2], RL01_KEY)) {
			drv->keyName    = RL01_KEY;
			drv->emuName    = RL01_NAME;
			drv->emuVersion = "";
			drv->Flags      = DFLG_RL01;
		} else if (!strcasecmp(argv[2], RL02_KEY)) {
			drv->keyName    = RL02_KEY;
			drv->emuName    = RL02_NAME;
			drv->emuVersion = "";
			drv->Flags      = DFLG_RL02;
		} else {
			printf("%s: No such device type on %s:\n",
				argv[2], newMap->devName);
			return NULL;
		}

		drv->devName    =  newMap->devName;
		drv->Flags      |= DFLG_EXIST;
		
		newMap->idUnit     = drv->idUnit;
		newMap->keyName    = drv->keyName;
		newMap->emuName    = drv->emuName;
		newMap->emuVersion = drv->emuVersion;
		newMap->devInfo    = newMap->devParent->devInfo;
		newMap->Device     = newMap->devParent->Device;
	} else {
		printf("%s: No such drive on %s: - (Only %d drives)\n",
			newMap->devName, rl->devName, RL_NDRVS);
		return NULL;
	}

	return drv;
}

int rl_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	RL_DEVICE  *rl  = (RL_DEVICE *)map->Device;
	RL_DRIVE   *drv = &rl->Drives[map->idUnit];
	int32      off; // Offset from beginning of file.

	if (drv->File) {
		printf("%s: Already attached. Please use DETACH first.\n",
			drv->devName);
		return EMU_OPENERR;
	}

	if ((drv->File = open(argv[2], O_RDWR|O_CREAT, 0700)) < 0) {
		printf("%s: file '%s' not attached - %s.\n",
			drv->devName, argv[2], strerror(errno));
		return EMU_OPENERR;
	}

	if ((off = lseek(drv->File, 0, SEEK_END)) < 0) {
		printf("%s: file 's': %s\n",
			drv->devName, argv[2], strerror(errno));
		close (drv->File);
		drv->File = 0;
		return EMU_OPENERR;
	}
  
	// Test file sizes...
	printf("File size = %d bytes (RL01: %d bytes, RL02: %d bytes)\n",
		off, RL01_SIZE, RL02_SIZE);

	if (off == 0) {
		// Empty file - new disk cartridge
		printf("%s: Initializing file '%s'...\n",
			drv->devName, argv[2]);

		// Recognize RL01 or RL02 device type and
		// initialize bad blocks.
		drv->totBytes = (drv->Flags & DFLG_RL02) ? RL02_SIZE : RL01_SIZE;
		drv->totCyls  = (drv->Flags & DFLG_RL02) ? RL02_CYL : RL01_CYL;
		rl_InitBadBlocks(drv);
		drv->rlds &= ~RLDS_UNATT;
	} else if (off <= RL01_SIZE) {
		printf("%s: file '%s' is a RL01 disk file.\n",
			drv->devName, argv[2]);
		drv->totBytes = off;
		drv->totCyls  = RL01_CYL;
		drv->rlds &= ~RLDS_UNATT;
		drv->rlds |= RLDS_ATT|RLDS_RL01|RLDS_VC;
	} else if (off <= RL02_SIZE) {
		printf("%s: file '%s' is a RL02 disk file.\n",
			drv->devName, argv[2]);
		drv->totBytes = off;
		drv->totCyls  = RL02_CYL;
		drv->rlds &= ~RLDS_UNATT;
		drv->rlds |= RLDS_ATT|RLDS_RL02|RLDS_VC;
	} else {
		// No, this file is not RLO1 or RL02 disk file.
		printf("%s: file '%s' is not RL01 or RL02 disk file.\n",
			drv->devName, argv[2]);
		close (drv->File);
		drv->File = 0;
		return EMU_OPENERR;
	}

	// Set up drive flags
	drv->Flags   |= DFLG_ATTACHED;
	drv->rlcs    |= RLCS_DRDY;
	drv->rlds    |= RLDS_VC; // New volume
	drv->track    = 0;

	// Tell operator that and save filename.
	printf("%s: file '%s' attached.\n", drv->devName, argv[2]);
	if (drv->fileName = (char *)malloc(strlen(argv[2])+1))
		strcpy(drv->fileName, argv[2]);
	else {
		printf("%s: Can't assign the name of '%s' - Not enough memory.\n",
			drv->fileName, argv[2]);
	}

	return EMU_OK;
}

int rl_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	RL_DEVICE  *rl  = (RL_DEVICE *)map->Device;
	RL_DRIVE   *drv = &rl->Drives[map->idUnit];

	if (drv->File) {
		// First, cancel all pending disk operations.
//		ts10_CancelTimer(&drv->svcTimer);

		// Clear all flags.
		drv->rlcs    &= ~RLCS_DRDY;
		drv->Flags   &= ~(DFLG_ATTACHED|DFLG_WLOCK);
		drv->track    = 0;

		// Now close disk file.
		close(drv->File);
		drv->File = 0;

		// Tell operator that.
		printf("%s: file '%s' detached.\n", drv->devName,
			drv->fileName ? drv->fileName : "<Unknown>");

		// Free the allocation of filename if available.
		if (drv->fileName) {
			free(drv->fileName);
			drv->fileName = NULL;
		}
	} else
		printf("%s: Already detached.\n", drv->devName);
	
	return EMU_OK;
}

//int rl_Reset(MAP_DEVICE *map)
int rl_Reset(void *dptr)
{
//	RL_DEVICE *rl = (RL_DEVICE *)map->Device;
	RL_DEVICE *rl = (RL_DEVICE *)dptr;

	rl_ResetDevice(rl);
}

DEVICE rl_Device =
{
	RL_KEY,           // Key Name
	RL_NAME,          // Emulator Name
	RL_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	rl_Create,        // Create Routine
	rl_Configure,     // Configure Routine
	NULL,             // Delete Routine
	rl_Reset,         // Reset Routine
	rl_Attach,        // Attach Routine
	rl_Detach,        // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};

DEVICE rlv_Device =
{
	RLV_KEY,          // Key Name
	RL_NAME,          // Emulator Name
	RL_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	rl_Create,        // Create Routine
	rl_Configure,     // Configure Routine
	NULL,             // Delete Routine
	rl_Reset,         // Reset Routine
	rl_Attach,        // Attach Routine
	rl_Detach,        // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
