// rh.c - RH11 MASSBUS Disk/Tape Controller
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

#include "dec/rh.h"

#ifdef DEBUG
// Listing of name of the registers for debug facility
// Note: ^ = Mixed, * = Drive Register, space = Controller Register

static cchar *regNameRH[] =
{
	"RHCS1",  // (R/W) ^Control and Status Register #1
	"RHWC",   // (R/W)  Word Count Register
	"RHBA",   // (R/W)  Bus Address Register
	"RHFC",   // (R/W) *Frame Count Register
	"RHCS2",  // (R/W)  Control and Status Register #2
	"RHDS",   // (R)   *Drive Status Register
	"RHER",   // (R)   *Error Register
	"RHAS",   // (R/C) #Attention Summary Register
	"RHCC",   // (R/W) *Character Check Register
	"RHDB",   // (R/W)  Data Buffer Register
	"RHMR",   // (R/W) *Maintenance Register
	"RHDT",   // (R/W) *Drive Type Register
	"RHSN",   // (R)   *Serial Number Register
	"RH??",   // (R/W)
	"RHBAE",  // (R/W)  Bus Address Extension Register (RH70 Only)
	"RHCS3",  // (R/W)  Control and Status Register #3 (RH70 Only)
};

static cchar *regNameRP[] =
{
	"RPCS1",  // (R/W) ^Control and Status Register #1
	"RPWC",   // (R/W)  Word Count Register
	"RPBA",   // (R/W)  Bus Address Register
	"RPDA",   // (R/W) *Disk Address Register
	"RPCS2",  // (R/W)  Control and Status Register #2
	"RPDS",   // (R)   *Drive Status Register
	"RPER1",  // (R)   *Error Register #1
	"RPAS",   // (R/C) #Attention Summary Register
	"RPLA",   // (R/W) *Look Ahead Register
	"RPDB",   // (R/W)  Data Buffer Register
	"RPMR",   // (R/W) *Maintenance Register
	"RPDT",   // (R/W) *Drive Type Register
	"RPSN",   // (R)   *Serial Number Register
	"RPOF",   // (R/W) *Offset Register
	"RPDC",   // (R/W) *Desired Cylinder Register
	"RPCC",   // (R)   *Current Cylinder Register
	"RPER2",  // (R)   *Error Register #2
	"RPER3",  // (R)   *Error Register #3
	"RPEC1",  // (R)   *ECC (Position) Register #1
	"RPEC2",  // (R)   *ECC (Pattern) Register #2
	"RPBAE",  // (R/W)  Bus Address Extension Register (RH70 Only)
	"RPCS3",  // (R/W)  Control and Status Register #3 (RH70 Only)
};

static cchar *regNameRM[] =
{
	"RMCS1",  // (R/W) ^Control and Status Register #1
	"RMWC",   // (R/W)  Word Count Register
	"RMBA",   // (R/W)  Bus Address Register
	"RMDA",   // (R/W) *Disk Address Register
	"RMCS2",  // (R/W)  Control and Status Register #2
	"RMDS",   // (R)   *Drive Status Register
	"RMER1",  // (R)   *Error Register #1
	"RMAS",   // (R/C) #Attention Summary Register
	"RMLA",   // (R/W) *Look Ahead Register
	"RMDB",   // (R/W)  Data Buffer Register
	"RMMR1",  // (R/W) *Maintenance Register #1
	"RMDT",   // (R/W) *Drive Type Register
	"RMSN",   // (R)   *Serial Number Register
	"RMOF",   // (R/W) *Offset Register
	"RMDC",   // (R/W) *Desired Cylinder Register
	"RMHR",   // (R)   *Current Cylinder Register
	"RMMR2",  // (R)   *Maintenace Register #2
	"RMER2",  // (R)   *Error Register #2
	"RMEC1",  // (R)   *ECC (Position) Register #1
	"RMEC2",  // (R)   *ECC (Pattern) Register #2
	"RMBAE",  // (R/W)  Bus Address Extension Register (RH70 Only)
	"RMCS3",  // (R/W)  Control and Status Register #3 (RH70 Only)
};

static cchar *regNameTM[] =
{
	"MTCS1", // (R/W) ^Control and Status Register #1
	"MTWC",  // (R/W)  Word Count Register
	"MTBA",  // (R/W)  Bus Address Register
	"MTFC",  // (R/W) *Frame Count Register
	"MTCS2", // (R/W)  Control and Status Register #2
	"MTDS",  // (R)   *Drive Status Register
	"MTER",  // (R)   *Error Register
	"MTAS",  // (R/W) #Attention Summary Register
	"MTCC",  // (R/W) *Character Check Register
	"MTDB",  // (R/W)  Data Buffer Register
	"MTMR",  // (R/W) *Maintenance Register
	"MTDT",  // (R)   *Drive Type Register
	"MTSN",  // (R)   *Serial Number Register
	"MTTC",  // (R/W) *Tape Control Register
	"MTBAE", // (R/W)  Bus Address Extension Register  (RH70 Only)
	"MTCS3"  // (R/W)  Control and Status Register #3  (RH70 Only)
};
#endif /* DEBUG */

// Unibus to MASSBUS Register # Conversion.
// Note: -1 = Controller Register, # = MASSBUS Register.
static const int regDrive[] = {
	0,  -1, -1,  5,  -1,  1,  2,  4,   7, -1,  3,  6,   8,  9, 10, 11,
	12, 13, 14, 15,  -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1
};

static const int IsDrive[] = {
	0, 0, 0, 1,  0, 1, 1, 1,  1, 0, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0
};

// Bus Initialization
void rh_ResetDevice(RH_DEVICE *rh)
{
	int idx;

	rh->rhcs1 = RHCS1_RDY;
	rh->rhcs2 = RHCS2_IR | RHCS2_OR;
	rh->rhba  = 0;

	// Reset all existing MASSBUS drives.
	for (idx = 0; idx < RH_MAXUNITS; idx++) {
		MBA_DRIVE *drv = &rh->mba.Drive[idx];
		if (drv->Flags < 0)
			drv->dInfo->ResetDrive(drv);
	}
	
	rh->mbaDrive = &rh->mba.Drive[0];
}

int rh_SetBoot(void *dptr, UQ_BOOT *bt)
{
	RH_DEVICE *rh = (RH_DEVICE *)dptr;

	// Set up CSR address with interface number
	// and system function call.
	bt->idUnit  = rh->ioMap.idUnit;
	bt->csrAddr = rh->ioMap.csrAddr;
	bt->Execute = rh->Callback->Boot;

	return UQ_OK;
}

void rh_SetDone(void *dptr)
{
	RH_DEVICE *rh = (RH_DEVICE *)dptr;

	// Set ready (done) bit on CS1 register.
	rh->rhcs1 |= RHCS1_RDY;
	if (rh->rhcs1 & RHCS1_IE) {
		rh->ioMap.SendInterrupt(&rh->ioMap, 0);
#ifdef DEBUG
		if (dbg_Check(DBG_INTERRUPT))
			dbg_Printf("%s: Ringing Host...\n", rh->Unit.devName);
#endif /* DEBUG */
	}
}

int rh_ReadBlock(void *dptr, uint8 *data, uint32 szWords, uint32 mode)
{
	RH_DEVICE *rh     = (RH_DEVICE *)dptr;
	UQ_CALL   *call   = rh->Callback;
	uint32    addr    = rh->rhba | ((rh->rhcs1 & RHCS1_BAE) << 8);
	uint32    wc      = 0200000 - rh->rhwc;
	uint32    szBytes = szWords << 1;
	uint32    ubm     = UB | ((mode & MBA_18B) ? B18 : 0);
	int       bai     = (mode & MBA_NOBAI) == 0 && (rh->rhcs2 & RHCS2_BAI);
	uint32    tbc;

	// Check Word Count first.
	if (wc == 0)
		return 0;
	if (wc < szWords) {
		szWords = wc;
		szBytes = wc << 1;
	}

//	dbg_Printf("%s: Address %06o (Size: %d bytes)\n",
//		rh->Unit.devName, addr, szBytes);

	// Now transfer data blocks.
	if (tbc = call->ReadBlock(rh->System, addr, data, szBytes, ubm))
		rh->rhcs2 |= RHCS2_NEM;
	tbc   = szBytes - tbc;
	wc   -= tbc >> 1;
	addr += tbc;

//	dbg_Printf("%s:   Address %06o (Transferred: %d bytes)\n",
//		rh->Unit.devName, addr, tbc);

	// Update RH11 registers
	if (!bai) {
		rh->rhba  = addr;
		rh->rhcs1 = ((addr >> 8) & RHCS1_BAE) | (rh->rhcs1 & ~RHCS1_BAE);
	}
	rh->rhwc = ~wc + 1;

	return tbc ? (rh->rhwc ? UQ_CONT : UQ_OK) : UQ_ERROR;
}

int rh_WriteBlock(void *dptr, uint8 *data, uint32 szWords, uint32 mode)
{
	RH_DEVICE *rh     = (RH_DEVICE *)dptr;
	UQ_CALL   *call   = rh->Callback;
	uint32    addr    = rh->rhba | ((rh->rhcs1 & RHCS1_BAE) << 8);
	uint32    wc      = 0200000 - rh->rhwc;
	uint32    szBytes = szWords << 1;
	uint32    ubm     = UB | ((mode & MBA_18B) ? B18 : 0);
	int       bai     = (mode & MBA_NOBAI) == 0 && (rh->rhcs2 & RHCS2_BAI);
	uint32    tbc;

	// Check Word Count first.
	if (wc == 0)
		return 0;
	if (wc < szWords) {
		szWords = wc;
		szBytes = wc << 1;
	}
	
//	dbg_Printf("%s: Address %06o (Size: %d bytes)\n",
//		rh->Unit.devName, addr, szBytes);

	// Now transfer data blocks.
	if (tbc = call->WriteBlock(rh->System, addr, data, szBytes, ubm))
		rh->rhcs2 |= RHCS2_NEM;
	tbc   = szBytes - tbc;
	wc   -= tbc >> 1;
	addr += tbc;

//	dbg_Printf("%s:   Address %06o (Transferred: %d bytes)\n",
//		rh->Unit.devName, addr, tbc);

	// Update RH11 registers
	if (!bai) {
		rh->rhba  = addr;
		rh->rhcs1 = ((addr >> 8) & RHCS1_BAE) | (rh->rhcs1 & ~RHCS1_BAE);
	}
	rh->rhwc = ~wc + 1;

	return tbc ? (rh->rhwc ? UQ_CONT : UQ_OK) : UQ_ERROR;
}

int rh_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	RH_DEVICE *rh  = (RH_DEVICE *)dptr;
	uint32    reg  = (pAddr & rh->mskAddr) >> 1;
	MBA_DRIVE *drv;
	uint16    csr;
	int       rc;

	switch(reg) {
		case RHCS1:
			if ((drv = rh->mbaDrive) && (drv->Flags < 0)) {
				drv->dInfo->ReadIO(drv, 0, &csr);
				*data = rh->rhcs1 | (csr & RHCS1_DRV);
			} else {
				rh->rhcs1 |= RHCS1_MCPE;
				rh->rhcs2 |= RHCS2_NED;
				*data = rh->rhcs1;
			}

#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) RHCS1 => %06o\n", rh->Unit.devName, *data);
#endif /* DEBUG */
			break;

		case RHWC:
			*data = rh->rhwc;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) RHWC => %06o (%d 16/18-bit words)\n",
					rh->Unit.devName, *data, 0200000 - *data);
#endif /* DEBUG */
			break;

		case RHBA:
			*data = rh->rhba;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) RHBA => %06o\n", rh->Unit.devName, *data);
#endif /* DEBUG */
			break;

		case RHCS2:
			*data = rh->rhcs2;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) RHCS2 => %06o\n", rh->Unit.devName, *data);
#endif /* DEBUG */
			break;

		case RHDB:
			*data = rh->rhdb;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) RHDB => %06o\n", rh->Unit.devName, *data);
#endif /* DEBUG */
			break;

		default:
//			dbg_Printf("%s: Drive Register #%d -> MASSBUS Drive #%d\n",
//				rh->Unit.devName, reg, regDrive[reg]);

			if (regDrive[reg] >= 0) {
				rc = mba_ReadIO(rh->mbaDrive, regDrive[reg], data);
				if (rc == MBA_NED) {
					rh->rhcs1 |= RHCS1_MCPE;
					rh->rhcs2 |= RHCS2_NED;
				}
				break;
			}

#ifdef DEBUG
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				rh->Unit.devName, reg, pAddr & 0x1FFF, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
//		dbg_Printf("%s: (R) %s (%o) => %06o (%04X) (Size: %d bytes)\n",
//			rh->Unit.devName, regNameRH[reg], pAddr, *data, *data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

int rh_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	RH_DEVICE *rh  = (RH_DEVICE *)dptr;
	uint32    reg  = (pAddr & rh->mskAddr) >> 1;
	MBA_DRIVE *drv;
	int       rc;

	switch(reg) {
		case RHCS1:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) RHCS1 <= %06o\n", rh->Unit.devName, data);
#endif /* DEBUG */
			// Set bits on controller register first.
			if (data & RHCS1_TRE)
				rh->rhcs1 &= ~RHCS1_TRE;
			rh->rhcs1 &= ~(RHCS1_BAE|RHCS1_IE);
			rh->rhcs1 |= data & (RHCS1_BAE|RHCS1_IE);
//			rh->rhcs1 = (data & RHCS1_RW) | (rh->rhcs1 & ~RHCS1_RW);

			// Check if drive is existing or not.
			if ((drv = rh->mbaDrive) && (drv->Flags < 0)) {
				int fnc = data & RHCS1_CMD;
				if ((fnc >= 050) && (fnc & RHCS1_GO)) {
					if (rh->rhcs1 & RHCS1_RDY) {
						rh->rhcs1 &= ~(RHCS1_TRE|RHCS1_RDY);
						rh->rhcs2 &= ~RHCS2_ERR;
					} else {
						rh->rhcs2 |= RHCS2_PGE;
						data      &= ~RHCS1_CMD;
					}
				}
//					rh->rhcs1 |= RHCS1_SC;

				if (data & RHCS1_CMD)
					drv->dInfo->WriteIO(drv, 0, data & RHCS1_CMD);
				// ACTION: Implement error handling for drive registers.
			} else
				rh->rhcs2 |= RHCS2_NED;
			break;

		case RHWC:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) RHWC <= %06o (%d 16/18-bit words)\n",
					rh->Unit.devName, data, 0200000 - data);
#endif /* DEBUG */
			rh->rhwc = data;
			break;

		case RHBA:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) RHBA <= %06o\n", rh->Unit.devName, data);
#endif /* DEBUG */
			rh->rhba = data;
			break;

		case RHCS2:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) RHCS2 <= %06o\n", rh->Unit.devName, data);
#endif /* DEBUG */

			if (data & RHCS2_CLR)
				rh_ResetDevice(rh);

			// Now update RHCS2 register
			rh->rhcs2 = (rh->rhcs2 & ~RHCS2_WR) | (data & RHCS2_WR);

			// Select desired MASSBUS drive.
			drv = rh->mbaDrive = &rh->mba.Drive[data & RHCS2_UNIT];
			if (drv->Flags < 0)
				rh->rhcs2 &= ~RHCS2_NED;
			else
				rh->rhcs2 |= RHCS2_NED;
			break;

		case RHDB:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) RHDB <= %06o\n", rh->Unit.devName, data);
#endif /* DEBUG */
			rh->rhdb = data;
			break;

		default:
			if (regDrive[reg] >= 0) {
				rc = mba_WriteIO(rh->mbaDrive, regDrive[reg], data);
				if (rc == MBA_NED) {
					rh->rhcs1 |= RHCS1_MCPE;
					rh->rhcs2 |= RHCS2_NED;
				}
				break;
			}

#ifdef DEBUG
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				rh->Unit.devName, reg, pAddr & 0x1FFF, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
//		dbg_Printf("%s: (W) %s (%o) <= %06o (%04X) (Size: %d bytes)\n",
//			rh->Unit.devName, regNameRH[reg], pAddr, data, data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

// Bus Initialization
void rh_ResetIO(void *dptr)
{
	rh_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// **************************************************************

void *rh_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	RH_DEVICE *rh = NULL;
	MAP_IO    *io;
	uint32    csrAddr, mskAddr, vecAddr, ipl;

	if (rh = (RH_DEVICE *)calloc(1, sizeof(RH_DEVICE))) {
		// First, set up its description and
		// link it to its parent device.
		rh->Unit.devName    = newMap->devName;
		rh->Unit.keyName    = newMap->keyName;
		rh->Unit.emuName    = newMap->emuName;
		rh->Unit.emuVersion = newMap->emuVersion;
		rh->Device          = newMap->devParent->Device;
		rh->Callback        = newMap->devParent->Callback;
		rh->System          = newMap->devParent->sysDevice;

		// Get CSR, Vector, and IPL from its command.
		if (uq_GetAddr((UNIT *)rh, &csrAddr, &mskAddr, &vecAddr, &ipl, argc-3, &argv[3])) {
			free(rh);
			return NULL;
		}
		rh->csrAddr = csrAddr;
		rh->mskAddr = mskAddr;

		// Initialize MASSBUS Controller
		mba_Create(rh, &rh->mba, &rh->mbaCall, 0, argc, argv);

		// Set callback functions
		rh->mbaCall.SetAttention = rh_SetDone;
		rh->mbaCall.SetReady     = rh_SetDone;
		rh->mbaCall.ReadBlock    = rh_ReadBlock;
		rh->mbaCall.WriteBlock   = rh_WriteBlock;
		rh->mbaCall.SetBoot      = rh_SetBoot;

		// Set up an I/O space.
		io               = &rh->ioMap;
		io->devName      = rh->Unit.devName;
		io->keyName      = rh->Unit.keyName;
		io->emuName      = rh->Unit.emuName;
		io->emuVersion   = rh->Unit.emuVersion;
		io->uqDevice     = rh->Device;
		io->Device       = rh;
		io->csrAddr      = csrAddr;
		io->nRegs        = (mskAddr + 1) >> 1;
		io->nVectors     = RH_NVECS;
		io->intIPL       = ipl;
		io->intVector[0] = vecAddr;
		io->ReadIO       = rh_ReadIO;
		io->WriteIO      = rh_WriteIO;
		io->ResetIO      = rh_ResetIO;

		// Assign that registers to QBA's I/O space.
		rh->Callback->SetMap(rh->Device, io);

		// Finally, link it to its mapping device and return.
		newMap->Device = rh;
	}

	return rh;
}

void *rh_Configure(MAP_DEVICE *newMap, int argc, char **argv)
{
	RH_DEVICE *rh = newMap->devParent->Device;
	MBA_DRIVE *mbaDrive;

	mbaDrive = mba_CreateDrive(newMap, &rh->mba, argc, argv);
	if (mbaDrive) {	
		newMap->idUnit     = mbaDrive->idDrive;
		newMap->keyName    = mbaDrive->Unit.keyName;
		newMap->emuName    = mbaDrive->Unit.emuName;
		newMap->emuVersion = mbaDrive->Unit.emuVersion;
		newMap->devInfo    = newMap->devParent->devInfo;
		newMap->Device     = newMap->devParent->Device;
	}

	return mbaDrive;
}

//int rh_Reset(MAP_DEVICE *map)
int rh_Reset(void *dptr)
{
//	RH_DEVICE *rh = (RH_DEVICE *)map->Device;
	RH_DEVICE *rh = (RH_DEVICE *)dptr;

	rh_ResetDevice(rh);
}

int rh_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	RH_DEVICE *rh  = (RH_DEVICE *)map->Device;
	MBA_DRIVE *drv = &rh->mba.Drive[map->idUnit];

	return mba_Attach(drv, argc, argv);
}

int rh_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	RH_DEVICE *rh  = (RH_DEVICE *)map->Device;
	MBA_DRIVE *drv = &rh->mba.Drive[map->idUnit];

	return mba_Detach(drv, argc, argv);
}

int rh_Info(MAP_DEVICE *map, int argc, char **argv)
{
	RH_DEVICE *rh = (RH_DEVICE *)map->Device;

	mba_Info(&rh->mba, argc, argv);
}

int rh_Boot(MAP_DEVICE *map, int argc, char **argv)
{
	RH_DEVICE *rh = (RH_DEVICE *)map->Device;
	MBA_DRIVE *drv = &rh->mba.Drive[map->idUnit];

	return mba_Boot(drv, argc, argv);
}

DEVICE rh_Device =
{
	RH_KEY,           // Key Name
	RH_NAME,          // Emulator Name
	RH_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	rh_Create,        // Create Routine
	rh_Configure,     // Configure Routine
	NULL,             // Delete Routine
	rh_Reset,         // Reset Routine
	rh_Attach,        // Attach Routine
	rh_Detach,        // Detach Routine
	rh_Info,          // Info Device
	rh_Boot,          // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
