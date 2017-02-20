// lp20.c - LP20 Line Printer Interface for KS10 machines
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
#include "dec/lp20.h"

#ifdef DEBUG
static cchar *regName[] = {
	"LPCSA",  "LPCSA",   // Control and Status Register #A
	"LPCSB",  "LPCSB",   // Control and Status Register #B
	"LPBA",   "LPBA",    // DMA Bus Address Register
	"LPBC",   "LPBC",    // DMA Byte Counter Register
	"LPPAGC", "LPPAGC",  // Page Count Register
	"LPRDTR", "LPRDTR",  // RAM Data Reguster
	"LPCBUF",            // Character Buffer Register (Low byte)
	          "LPCOLC",  // Column Count Register     (High byte)
	"LPPDAT",            // Printer Data Register     (Low byte)
	          "LPCSUM",  // Checksum Register         (High byte)
};
#endif /* DEBUG */

// Bus Initialization
void lp20_ResetDevice(LP20_DEVICE *lp20)
{
	// Discontinue print/DMA processing
	ts10_CancelTimer(&lp20->svcTimer);

	// Clear all LP20 registers.
	LPCSRA = CSA_DONE;
	LPCSRB = 0;
	LPBSAD = 0;
	LPBCTR = 0;
	LPPCTR = 0;
	LPRAMD = 0;
	LPCCTR = 0;
	LPCBUF = 0;
	LPCKSM = 0;
	LPTDAT = 0;

	lp20->dvptr = 0;
}

int lp20_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	LP20_DEVICE *lp20 = (LP20_DEVICE *)dptr;
	uint32       reg  = pAddr & ((LP20_NREGS * 2) - 1);

	switch(reg >> 1) {
		case nLPCSRA: *data = LPCSRA;                 break;
		case nLPCSRB: *data = LPCSRB;                 break;
		case nLPBSAD: *data = LPBSAD;                 break;
		case nLPBCTR: *data = LPBCTR;                 break;
		case nLPPCTR: *data = LPPCTR;                 break;
		case nLPRAMD: *data = LPRAMD;                 break;
		case nLPCCTR: *data = (LPCCTR << 8) | LPCBUF; break;
		case nLPCKSM: *data = (LPCKSM << 8) | LPTDAT; break;

#ifdef DEBUG
		default:
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				lp20->Unit.devName, reg >> 1, pAddr & 0x1FFF, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%o) => %06o (%04X) (Size: %d bytes)\n",
			lp20->Unit.devName, regName[reg], pAddr, *data, *data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

int lp20_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	LP20_DEVICE *lp20 = (LP20_DEVICE *)dptr;
	uint32       reg  = pAddr & ((LP20_NREGS * 2) - 1);

	switch(reg >> 1) {
		case nLPCSRA: // Control and Status Register A
			// If byte access, merge data with CSRA.
			if (acc == ACC_BYTE) data = (pAddr & 1) ?
				((LPCSRA & 0377)  | (data << 8)) :
				((LPCSRA & ~0377) | (data & 0377));

			// Clear all errors.
			if (data & CSA_RESET) {
				ts10_CancelTimer(&lp20->svcTimer);
				LPCSRA  = (LPCSRA & CSA_DONE) & ~CSA_GO;
				LPCSRB &= ~CSB_ECLR;
			}

			// Initialize LP20 interface.
			if (data & CSA_INIT)
				lp20_ResetDevice(lp20);

			// Start printing now. (Start DMA)
			if (data & CSA_GO) {
				if ((LPCSRA & CSA_GO) == 0) {
					if (LPCSRB & CSB_ERR)
						LPCSRB |= CSB_GOE;
					LPCKSM = 0; // Clear checksum.
					ts10_SetTimer(&lp20->svcTimer);
				}
			} else {
				// Stop printing now. (Stop DMA)
				ts10_CancelTimer(&lp20->svcTimer);
			}

			// Finally, load data into CSRA register.
			LPCSRA = (data & CSA_RW) | (LPCSRA & ~CSA_RW);
			break;

		case nLPCSRB: // Control and Status Register B
			break;

		case nLPBSAD: // DMA Bus Address Register
			// If byte access, merge data with LPBSAD.
			if (acc == ACC_BYTE) data = (pAddr & 1) ?
				((LPBSAD & 0377)  | (data << 8)) :
				((LPBSAD & ~0377) | (data & 0377));
			LPBSAD = data;
			break;

		case nLPBCTR: // DMA Byte Counter Register
			// If byte access, merge data with LPBCTR.
			if (acc == ACC_BYTE) data = (pAddr & 1) ?
				((LPBCTR & 0377)  | (data << 8)) :
				((LPBCTR & ~0377) | (data & 0377));
			LPBCTR = data & BCTR_MASK;
			LPCSRA &= ~CSA_DONE;
			break;

		case nLPPCTR: // Page Counter Register
			// If byte access, merge data with LPPCTR.
			if (acc == ACC_BYTE) data = (pAddr & 1) ?
				((LPPCTR & 0377)  | (data << 8)) :
				((LPPCTR & ~0377) | (data & 0377));
			LPPCTR = data & PCTR_MASK;
			break;

		case nLPRAMD: // RAM Data Register
			// If byte access, merge data with LPRAMD.
			if (acc == ACC_BYTE) data = (pAddr & 1) ?
				((LPRAMD & 0377)  | (data << 8)) :
				((LPRAMD & ~0377) | (data & 0377));
			LPRAMD = data & RDAT_MASK;

			// Load RAM data
			TXRAM(LPCBUF & TX_AMASK) = LPRAMD;
			break;

		case nLPCCTR: // LPCCTR/LPCBUF Register
			if (acc == ACC_BYTE) {
				// Byte Access
				if (pAddr & 1) LPCCTR = data;
				else           LPCBUF = data;
			} else {
				// Word Access
				LPCCTR = data >> 8;
				LPCBUF = data;
			}
			break;

		case nLPCKSM:
			// Do nothing - Read Only Register
			break;

#ifdef DEBUG
		default:
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				lp20->Unit.devName, reg >> 1, pAddr & 0x1FFF, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) %s (%o) <= %06o (%04X) (Size: %d bytes)\n",
			lp20->Unit.devName, regName[reg], pAddr, data, data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

// Bus Initialization
void lp20_ResetIO(void *dptr)
{
	lp20_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// **************************************************************

void *lp20_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	LP20_DEVICE *lp20 = NULL;
	MAP_IO      *io;

	if (lp20 = (LP20_DEVICE *)calloc(1, sizeof(LP20_DEVICE))) {
		// First, set up its description and
		// link it to its parent device.
		lp20->Unit.devName    = newMap->devName;
		lp20->Unit.keyName    = newMap->keyName;
		lp20->Unit.emuName    = newMap->emuName;
		lp20->Unit.emuVersion = newMap->emuVersion;
		lp20->Device          = newMap->devParent->Device;
		lp20->Callback        = newMap->devParent->Callback;
		lp20->System          = newMap->devParent->sysDevice;

		// Set up an I/O space.
		io               = &lp20->ioMap;
		io->devName      = lp20->Unit.devName;
		io->keyName      = lp20->Unit.keyName;
		io->emuName      = lp20->Unit.emuName;
		io->emuVersion   = lp20->Unit.emuVersion;
		io->Device       = lp20;
		io->csrAddr      = LP20_CSRADDR; /* lp20->csrAddr; */
		io->nRegs        = LP20_NREGS;
		io->nVectors     = LP20_NVECS;
		io->intIPL       = LP20_INT;
		io->intVector[0] = LP20_VEC;
		io->ReadIO       = lp20_ReadIO;
		io->WriteIO      = lp20_WriteIO;
		io->ResetIO      = lp20_ResetIO;

		// Assign that registers to QBA's I/O space.
		lp20->Callback->SetMap(lp20->Device, io);

		// Finally, link it to its mapping device and return.
		newMap->Device = lp20;
	}

	return lp20;
}

//int lp20_Reset(MAP_DEVICE *map)
int lp20_Reset(void *dptr)
{
//	LP20_DEVICE *lp20 = (LP20_DEVICE *)map->Device;
	LP20_DEVICE *lp20 = (LP20_DEVICE *)dptr;

	lp20_ResetDevice(lp20);
}

DEVICE lp20_Device =
{
	LP20_KEY,         // Key Name
	LP20_NAME,        // Emulator Name
	LP20_VERSION,     // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	lp20_Create,      // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	lp20_Reset,       // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
