// rm.c - RM MASSBUS-based disk drive routines
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
// See ReadMe for copyright notice.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "emu/defs.h"
#include "dev/mba/mba.h"
#include "dev/mba/rm.h"

// This controller supports many different disk drive types:
//
// Type     # Sectors/  # Surfaces/  # Cyclinders/    Total
//           Surface     Cyclinder       Drive
//
// RM02/03     32            5            823      =  67 MB
// RM80        31           14            559      = 124 MB
// RM05        32           19            823      = 256 MB

MBA_DTYPE rm_dTypes[] = {
	{
		"RM02",          // Device Name
		"RM02 - 67MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		32,              // Number of Sectors
		5,               // Number of Surfaces
		823,             // Number of Cylinders
		(32 * 5 * 823),  // Total Blocks
		020023,          // Device Type
		NULL,            // Not Used

		RH11_PACK,       // RH11 Flags - Disk Pack Type
		NULL,            // Not Used

		disk_Open,       // Low-level Open Routine 
		disk_Close,      // Low-level Close Routine
		disk_Read,       // Low-level Read Routine
		disk_Write,      // Low-level Write Routine
		NULL,            // Low-level Mark Routine
		NULL,            // Low-level Rewind Routine
		NULL,            // Low-level Skip Routine
		disk_Seek,       // Low-level Seek Routine
		mba_GetDiskAddr // Low-level GetDiskAddr Routine
	},

	{
		"RM03",          // Device Name
		"RM03 - 67MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		32,              // Number of Sectors
		5,               // Number of Surfaces
		823,             // Number of Cylinders
		(32 * 5 * 823),  // Total Blocks
		020024,          // Device Type
		NULL,            // Not Used

		RH11_PACK,       // RH11 Flags - Disk Pack Type
		NULL,            // Not Used

		disk_Open,       // Low-level Open Routine 
		disk_Close,      // Low-level Close Routine
		disk_Read,       // Low-level Read Routine
		disk_Write,      // Low-level Write Routine
		NULL,            // Low-level Mark Routine
		NULL,            // Low-level Rewind Routine
		NULL,            // Low-level Skip Routine
		disk_Seek,       // Low-level Seek Routine
		mba_GetDiskAddr // Low-level GetDiskAddr Routine
	},

	{
		"RM80",          // Device Name
		"RM80 - 124MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		31,              // Number of Sectors
		19,              // Number of Surfaces
		559,             // Number of Cylinders
		(31 * 19 * 559), // Total Blocks
		020026,          // Device Type
		NULL,            // Not Used

		RH11_PACK,       // RH11 Flags - Disk Pack Type
		NULL,            // Not Used

		disk_Open,       // Low-level Open Routine 
		disk_Close,      // Low-level Close Routine
		disk_Read,       // Low-level Read Routine
		disk_Write,      // Low-level Write Routine
		NULL,            // Low-level Mark Routine
		NULL,            // Low-level Rewind Routine
		NULL,            // Low-level Skip Routine
		disk_Seek,       // Low-level Seek Routine
		mba_GetDiskAddr // Low-level GetDiskAddr Routine
	},

	{
		"RM05",          // Device Name
		"RM05 - 256MB Disk Pack Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		32,              // Number of Sectors
		19,              // Number of Surfaces
		823,             // Number of Cylinders
		(32 * 19 * 823), // Total Blocks
		020027,          // Device Type
		NULL,            // Not Used

		RH11_PACK,       // RH11 Flags - Disk Pack Type
		NULL,            // Not Used

		disk_Open,       // Low-level Open Routine 
		disk_Close,      // Low-level Close Routine
		disk_Read,       // Low-level Read Routine
		disk_Write,      // Low-level Write Routine
		NULL,            // Low-level Mark Routine
		NULL,            // Low-level Rewind Routine
		NULL,            // Low-level Skip Routine
		disk_Seek,       // Low-level Seek Routine
		mba_GetDiskAddr // Low-level GetDiskAddr Routine
	},

	{ NULL },
};

DEVICE rm_Device =
{
	"RM",          // Device Name
	"MASSBUS Disk Pack Drives",
	"v0.0 (Alpha)",// Version
	rm_dTypes,     // Listing of RM devices
	NULL,          // Listing of Units
	NULL,          // Listing of Slave Devices - Not used
	NULL,          // Listing of Commands
	NULL,          // Listing of Set Commands
	NULL,          // Listing of Show Commands

	0,             // Number of Devices
	RM_MAXUNITS,   // Number of Units

	rm_Initialize, // Initialize Routine
	rm_Reset,      // Reset Routine
	rm_Create,     // Create Routine
	rm_Delete,     // Delete Routine
	NULL,          // Configure Routine - Not Used
	rm_Enable,     // Enable Routine
	rm_Disable,    // Disable Routine
	rm_Attach,     // Attach Routine
	rm_Detach,     // Detach Routine
	NULL,          // Format Routine - Not Used
	rm_ReadIO,     // Read I/O Routine
	rm_WriteIO,    // Write I/O Routine
	rm_Process,    // Process Routine
	rm_Boot,       // Boot Routine
	NULL,          // Execute Routine - Not Used

	rm_SetUnit,    // SetUnit Routine
	rm_SetATA,     // SetATA Routine
	rm_ClearATA    // ClearATA Routine
};

#ifdef DEBUG
// Listing of name of the registers for debug facility
// Note: ^ = Mixed, * = Drive Register, space = Controller Register

char *rm_RegNames[] =
{
	"RMCS1", // (R/W) ^Control and Status Register #1
	"RMWC",  // (R/W)  Word Count Register
	"RMBA",  // (R/W)  Bus Address Register
	"RMDA",  // (R/W) *Desired Sector/Track Address Register
	"RMCS2", // (R/W)  Control and Status Register #2
	"RMDS",  // (R)   *Drive Status Register
	"RMER1", // (R)   *Error Register #1
	"RMAS",  // (R/W) *Attention Summary Register
	"RMLA",  // (R)   *Look-Ahead Register
	"RMDB",  // (R/W)  Data Buffer Register
	"RMMR1", // (R/W)  Maintenance Register #1
	"RMDT",  // (R)   *Drive Type Register
	"RMSN",  // (R)   *Serial Number Register
	"RMOF",  // (R/W) *Offset Register
	"RMDC",  // (R/W) *Desired Cylinder Address Register
	"RMHR",  // (R/W) *Holding Register (Not Used)
	"RMMR2", // (R)   *Maintenance Register #2
	"RMER2", // (R/W) *Error Register #2
	"RMEC1", // (R)   *ECC Position Register
	"RMEC2", // (R)   *ECC Pattern Register
	"RMBAE", // (R/W)  Bus Address Extension Register  (RH70 Only)
	"RMCS3"  // (R/W)  Control and Status Register #3  (RH70 Only)
};
#endif /* DEBUG */

// Power Up Initialization
void rm_Initialize(UNIT *rmUnit)
{
	RMUNIT *rmData = (RMUNIT *)rmUnit->uData;
}

void rm_Reset(UNIT *rmUnit)
{
	RMUNIT *rmData  = (RMUNIT *)rmUnit->uData;
}

// Create the desired RM disk drive
int rm_Create(UNIT *uptr, char *devName, int argc, char **argv)
{
	if (!(uptr->Flags & UNIT_PRESENT)) {
		RMUNIT *rmData = (RMUNIT *)calloc(sizeof(RMUNIT), 1);

		if (rmData) {
			int idx;
			int st;

			for (idx = 0; rm_dTypes[idx].Name; idx++) {
				if (!strcasecmp(argv[0], rm_dTypes[idx].Name)) {
					printf("Found %s\n", rm_dTypes[idx].Name);
					break;
				}
			}

			if (!rm_dTypes[idx].Name) {
				free(rmData);
				return EMU_ARG;
			}

			*strchr(argv[1], ':') = '\0';
			if (st = unit_mapCreateDevice(argv[1], uptr)) {
				free(rmData);
				return st;
			}

			uptr->dType  = &rm_dTypes[idx];
			uptr->Device = &rm_Device;
			uptr->Flags  = rm_dTypes[idx].Flags | UNIT_PRESENT;
			uptr->uData  = (void *)rmData;
		} else
			return EMU_MEMERR;
	} else
		return EMU_ARG;
	return EMU_OK;
}

// Delete the desired RM disk drive
int rm_Delete(UNIT *rmUnit)
{
	if (rmUnit->Flags & UNIT_PRESENT) {
		free(rmUnit->uData);
		rmUnit->Flags &= ~UNIT_PRESENT;

		return EMU_OK;
	}
	return EMU_NPRESENT;
}

int rm_Enable(UNIT *rmUnit)
{
	return EMU_OK;
}

int rm_Disable(UNIT *rmUnit)
{
	return EMU_OK;
}

int rm_Attach(UNIT *rmUnit, char *fileName)
{
	RMUNIT *rmData = (RMUNIT *)rmUnit->pUnit->uData;
	int    st;

	if (rmUnit->Flags & UNIT_DISABLED)
		return EMU_DISABLED;

	if (rmUnit->Flags & UNIT_ATTACHED)
		return EMU_ATTACHED;

	// Attach a file to that unit
	if (st = disk_Open(rmUnit, fileName, 0))
		return st;

	rmUnit->Flags |= UNIT_ATTACHED|UNIT_WRLOCKED;

	// Now let's set some registers for medium on-line
	rmData->rmds |= 0;
	rm_SetATA(rmUnit->pUnit);

	return EMU_OK;
}

int rm_Detach(UNIT *rmUnit)
{
	RMUNIT *rmData = (RMUNIT *)rmUnit->pUnit->uData;
	int st;

	if (rmUnit->Flags & UNIT_DISABLED)
		return EMU_DISABLED;

	if (!(rmUnit->Flags & UNIT_ATTACHED))
		return EMU_ARG;

	// Detach a file from that unit.
	if (st = disk_Close(rmUnit))
		return st;

	rmUnit->Flags &= ~(UNIT_ATTACHED|UNIT_WRLOCKED);

	// Now let's set some registers for medium off-line
	rmData->rmds &= ~0;

	return EMU_OK;
}

UNIT *rm_SetUnit(UNIT *rmUnit)
{
	return rmUnit;
}

void rm_SetATA(UNIT *rmUnit)
{
	RMUNIT *rmData  = (RMUNIT *)rmUnit->uData;
	UNIT   *pUnit   = rmUnit->pUnit;
	DEVICE *pDevice = pUnit->Device;

	rmData->rmds |= RMDS_ATA;
	pDevice->SetATA(pUnit, rmUnit->idUnit);
}

void rm_ClearATA(UNIT *rmUnit)
{
	RMUNIT *rmData  = (RMUNIT *)rmUnit->uData;
	UNIT   *pUnit   = rmUnit->pUnit;
	DEVICE *pDevice = pUnit->Device;

	rmData->rmds &= ~RMDS_ATA;
	pDevice->ClearATA(pUnit, rmUnit->idUnit);
}

void rm_Ready(UNIT *rmUnit)
{
	UNIT   *pUnit   = rmUnit->pUnit;
	DEVICE *pDevice = pUnit->Device;

	pDevice->Ready(pUnit);
}

void rm_WriteData(UNIT *rmUnit)
{
}

int rm_ReadIO(UNIT *rmUnit, int32 reg, int32 *data)
{
	RMUNIT *rmData = (RMUNIT *)rmUnit->uData;

	return EMU_OK;
}

int rm_WriteIO(UNIT *rmUnit, int32 reg, int32 data)
{
	RMUNIT *rmData = (RMUNIT *)rmUnit->uData;

	return EMU_OK;
}

void rm_Go(UNIT *rmUnit)
{
}

int rm_Process(UNIT *rmUnit)
{
	return EMU_OK;
}

int rm_Boot(UNIT *rmUnit, int argc, char **argv)
{
	printf("Not supported yet.\n");
	return EMU_NOTSUPPORTED;
}
