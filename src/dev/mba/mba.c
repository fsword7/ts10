// mba.c - MASSBUS Interface Controller Routines
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

extern MBA_DEVINFO rp_DriveInfo;
extern MBA_DEVINFO rm_DriveInfo;
extern MBA_DEVINFO tu_DriveInfo;

// Listing of Drive Information/Calls
MBA_DEVINFO *mba_DriveInfo[] =
{
	&rp_DriveInfo,    // RP - Disk Pack Drive Series
	&rm_DriveInfo,    // RM - Disk Pack Drive Series
	&tu_DriveInfo,    // TU - Tape Formattor/Slave Drive Series
	NULL
};

void mba_Create(void *dptr, MBA_DEVICE *mba, MBA_CALLBACK *mbaCall,
	int idDevice, int argc, char **argv)
{
	int idx;

	// Clear all contents first.
	memset(mba, 0, sizeof(MBA_DEVICE));

	// Create new MBA device;
	mba->idDevice = idDevice;
	mba->pDevice  = dptr;
	mba->Callback = mbaCall;
	for (idx = 0; idx < MBA_MAXDRIVES; idx++) {
		mba->Drive[idx].idDrive  = idx;
		mba->Drive[idx].wDevice  = mba;
		mba->Drive[idx].pDevice  = dptr;
		mba->Drive[idx].Callback = mbaCall;
	}
}

// Create MBA drive for desired MBA device.
void *mba_CreateDrive(MAP_DEVICE *newMap, MBA_DEVICE *mba,
	int argc, char **argv)
{
	MBA_DRIVE   *mbaDrive = NULL;
	MBA_DEVINFO *drvInfo  = NULL;
	MBA_DTYPE   *drvType  = NULL;
	int         drvSlot, idx;

	// Check the drive is existing or not.
	if ((drvSlot = GetDeviceUnit(newMap->devName)) < MBA_MAXDRIVES) {
		mbaDrive = &mba->Drive[drvSlot];
		if (mbaDrive->Flags < 0) {
			printf("%s: Already Created on %s:\n",
				newMap->devName, mba->Unit.devName);
			return NULL;
		}
	} else {
		printf("%s: Only %d drives on %s:\n",
			newMap->devName, MBA_MAXDRIVES, mba->Unit.devName);
		return NULL;
	}

	// Find a drive type like RP, RM, TM, etc.
	for (idx = 0; mba_DriveInfo[idx]->Name; idx++) {
		int len = strlen(mba_DriveInfo[idx]->Name);
		if (!strncasecmp(argv[2], mba_DriveInfo[idx]->Name, len)) {
			drvInfo = mba_DriveInfo[idx];
			break;
		}
	}
	if (drvInfo == NULL) {
		printf("%s: Drive not found.\n", newMap->devName);
		return NULL;
	}

	// Find a model like RP04, etc.
	for (idx = 0; drvInfo->dTypes[idx].Name; idx++) {
		if (!strcasecmp(argv[2], drvInfo->dTypes[idx].Name)) {
			drvType = &drvInfo->dTypes[idx];
			break;
		}
	}
	if (drvType == NULL) {
		printf("%s: Model number not found.\n", newMap->devName);
		return NULL;
	}

	// Now set up a new drive slot...
	mbaDrive->Unit.devName    = newMap->devName;
	mbaDrive->Unit.keyName    = drvType->Name;
	mbaDrive->Unit.emuName    = drvType->Desc;
	mbaDrive->Unit.emuVersion = "";

	mbaDrive->Flags = MBA_PRESENT;
	mbaDrive->dInfo = drvInfo;
	mbaDrive->dType = drvType;
	mbaDrive->dInfo->InitDrive(mbaDrive);

	printf("%s: Device %s (%s) - Created.\n",
		mbaDrive->Unit.devName, drvType->Name, drvInfo->Name);

	return mbaDrive;
}

// Attach Command
// Usage: attach <device> ...
int mba_Attach(MBA_DRIVE *mbaDrive, int argc, char **argv)
{
	if (mbaDrive->Flags < 0) {
		return mbaDrive->dInfo->Attach(mbaDrive, argv[2]);
	} else {
		printf("%s: No such drive %d.\n",
			mbaDrive->Unit.devName, mbaDrive->idDrive);
		return EMU_ATTACHED;
	}	
}

// Detach Command
// Usage: detach <device> ...
int mba_Detach(MBA_DRIVE *mbaDrive, int argc, char **argv)
{
	if (mbaDrive->Flags < 0) {
		return mbaDrive->dInfo->Detach(mbaDrive);
	} else {
		printf("%s: No such drive %d.\n",
			mbaDrive->Unit.devName, mbaDrive->idDrive);
		return EMU_ATTACHED;
	}	
}

// Write MASSBUS Registers
int mba_WriteIO(MBA_DRIVE *mbaDrive, int reg, uint16 data)
{
	int idx;

	if (reg == MBA_AS) {
		// Attention Summary Register (for all drives)
		MBA_DEVICE *mba = mbaDrive->wDevice;
		for (idx = 0; idx < MBA_MAXDRIVES; idx++) {
			if (data & (1u << idx)) {
				mbaDrive = &mba->Drive[idx];
				if (mbaDrive->Flags < 0)
					mbaDrive->dInfo->ClearAttention(mbaDrive);
			}
		}
		return MBA_OK;
	} else if (mbaDrive->Flags < 0) {
		mbaDrive->dInfo->WriteIO(mbaDrive, reg, data);
		return MBA_OK;
	}
	return MBA_NED;
}

// Read MASSBUS Registers
int mba_ReadIO(MBA_DRIVE *mbaDrive, int reg, uint16 *data)
{
	int idx;

	if (reg == MBA_AS) {
		// Attention Summary Register (for all drives)
		MBA_DEVICE *mba = mbaDrive->wDevice;
		*data = 0;
		for (idx = 0; idx < MBA_MAXDRIVES; idx++) {
			if (mba->Drive[idx].Regs[MBA_DS] & MBA_DS_ATN)
				*data |= (1u << idx);
		}
#ifdef DEBUG
		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("MBA: AS => %06o\n", *data);
#endif /* DEBUG */
		return MBA_OK;
	} else if (mbaDrive->Flags < 0) {
		mbaDrive->dInfo->ReadIO(mbaDrive, reg, data);
		return MBA_OK;
	}
	*data = 0;
	return MBA_NED;
}

// Info Command
int mba_Info(MBA_DEVICE *mba, int argc, char **argv)
{
	int cnt = 0;
	int idx;

	// Print each drive about information.
	for (idx = 0; idx < MBA_MAXDRIVES; idx++) {
		MBA_DRIVE *drv = &mba->Drive[idx];
		if (drv->Flags & MBA_PRESENT) {
			printf("%-8s %-8s %-20s\n",
				drv->Unit.devName, drv->Unit.keyName, drv->Unit.emuName);
			cnt++;
		}
	}

	// Print number of existing drives.
	if (cnt > 0)
		printf("\nTotal %d drive%s.\n", cnt, (cnt == 1 ? "" : "s"));
	else
		printf("No drives.\n");
}

// Boot Command
int mba_Boot(MBA_DRIVE *mbaDrive, int argc, char **argv)
{
	if (mbaDrive->Flags < 0) {
		return mbaDrive->dInfo->Boot(mbaDrive, argc, argv);
	} else {
		printf("%s: No such drive %d.\n",
			mbaDrive->Unit.devName, mbaDrive->idDrive);
		return EMU_ATTACHED;
	}	
}
