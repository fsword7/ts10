// There is template code for new Unibus/Qbus devices.
//
// To replace TEMPL and templ with desired device type name on vi editor:
//
//    1,$s/TEMPL/DEV-NAME/g
//    1,$s/templ/dev-name/g

#include "dev/defs.h"

// Bus Initialization
void templ_ResetDevice(TEMPL_DEVICE *templ)
{
}

int templ_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	TEMPL_DEVICE *templ = (TEMPL_DEVICE *)dptr;
	uint32       reg    = (pAddr - (templ->csrAddr & 0x1FFF)) >> 1;

	switch(reg) {
#ifdef DEBUG
		default:
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				templ->devName, reg, pAddr & 0x1FFF, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%o) => %06o (%04X) (Size: %d bytes)\n",
			templ->devName, regNameR[reg], pAddr, *data, *data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

int templ_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	TEMPL_DEVICE *templ = (TEMPL_DEVICE *)dptr;
	uint32       reg    = (pAddr - (templ->csrAddr & 0x1FFF)) >> 1;


	switch(reg) {
#ifdef DEBUG
		default:
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				templ->devName, reg, pAddr & 0x1FFF, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) %s (%o) <= %06o (%04X) (Size: %d bytes)\n",
			templ->devName, regNameW[reg], pAddr, data, data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

// Bus Initialization
void templ_ResetIO(void *dptr)
{
	templ_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// **************************************************************

void *templ_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	TEMPL_DEVICE *templ = NULL;
	MAP_IO       *io;

	if (templ = (TEMPL_DEVICE *)calloc(1, sizeof(TEMPL_DEVICE))) {
		// First, set up its description and
		// link it to its parent device.
		templ->Unit.devName    = newMap->devName;
		templ->Unit.keyName    = newMap->keyName;
		templ->Unit.emuName    = newMap->emuName;
		templ->Unit.emuVersion = newMap->emuVersion;
		templ->Device          = newMap->devParent->Device;
		templ->Callback        = newMap->devParent->Callback;
		templ->System          = newMap->devParent->sysDevice;

		// Set up an I/O space.
		io               = &templ->ioMap;
		io->devName      = templ->Unit.devName;
		io->keyName      = templ->Unit.keyName;
		io->emuName      = templ->Unit.emuName;
		io->emuVersion   = templ->Unit.emuVersion;
		io->Device       = templ;
		io->csrAddr      = templ->csrAddr;
		io->nRegs        = TEMPL_NREGS;
		io->intLevel[0]  = TEMPL_RXINT;
		io->intMask[0]   = (1u << TEMPL_RXINT);
		io->intVector[0] = TEMPL_RXVEC;
		io->intLevel[1]  = TEMPL_TXINT;
		io->intMask[1]   = (1u << TEMPL_TXINT);
		io->intVector[1] = TEMPL_TXVEC;
		io->ReadIO       = templ_ReadIO;
		io->WriteIO      = templ_WriteIO;
		io->ResetIO      = templ_ResetIO;

		// Assign that registers to QBA's I/O space.
		templ->Callback->SetMap(templ->Device, io);

		// Finally, link it to its mapping device and return.
		newMap->Device = templ;
	}

	return templ;
}

//int templ_Reset(MAP_DEVICE *map)
int templ_Reset(void *dptr)
{
//	TEMPL_DEVICE *templ = (TEMPL_DEVICE *)map->Device;
	TEMPL_DEVICE *templ = (TEMPL_DEVICE *)dptr;

	templ_ResetDevice(templ);
}

DEVICE templ_Device =
{
	TEMPL_KEY,        // Key Name
	TEMPL_NAME,       // Emulator Name
	TEMPL_VERSION,    // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_TERMINAL,      // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	templ_Create,     // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	templ_Reset,      // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
