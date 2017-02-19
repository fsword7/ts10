// system.c - TS10 Emulator System Configurations
//
// Copyright (c) 2000-2002, Timothy M. Stark
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
#include "emu/version.h"

extern COMMAND ts10_Commands;
extern COMMAND ts10_SetCommands;
extern COMMAND ts10_ShowCommands;

extern DEVICE p10_System;   // PDP-10 System
extern DEVICE p11_System;   // PDP-11 System
extern DEVICE vax_System;   // VAX-11 System

// Emulator Modules
DEVICE *ts10_Systems[] =
{
	&p10_System,   // DECsystem-10/20 Series Emulator Module
	&p11_System,   // PDP-11 Series Emulator Module
	&vax_System,   // VAX-11 Series Emulator Module
	NULL
};

DEVICE ts10_MainSystem =
{
	TS10_DTNAME,        // Device Name
	TS10_NAME,          // System Name
	TS10_VERSION,       // System Version
	ts10_Systems,       // Systems List
	0,                  // Device Flags
	DT_ROOT,            // Device Type

	&ts10_Commands,     // System Commands
	&ts10_SetCommands,  // System Set Commands
	&ts10_ShowCommands, // System Show Commands

	// No function calls for this root device.
};


MAP_DEVICE *ts10_Root   = NULL; // Root System
MAP_DEVICE *ts10_System = NULL; // Desired System
MAP_DEVICE *ts10_Use    = NULL; // Current Device for System

// TS10 Emulator Systems Initialization
void InitSystem(void)
{
	MAP_DEVICE *map;

	// Create a new root mapping device first.
	if (map = CreateDevice(TS10_DEVNAME)) {
		// Set up family tree system.
		map->mapNext    = NULL; // Sidling map devices
		map->mapParent  = NULL; // Root map device
		map->mapChild   = NULL; // Child map devices

		map->keyName    = ts10_MainSystem.dtName;
		map->emuName    = ts10_MainSystem.emuName;
		map->emuVersion = ts10_MainSystem.emuVersion;

		map->devInfo    = &ts10_MainSystem;
		map->sysMap     = map;
		ts10_Root       = map;

		// Welcome Message
		printf("%s %s\n\n", map->emuName, map->emuVersion);
		return;
	}

	printf("*** Can't initialize the system. Aborting. ***\n");
	exit(1);
}

// Create a new mapping device.
MAP_DEVICE *CreateDevice(char *devName)
{
	MAP_DEVICE *newMap;

	if (newMap = (MAP_DEVICE *)calloc(1, sizeof(MAP_DEVICE))) {
		// Copy device name into mapping device's name.
		if (newMap->devName = (char *)malloc(strlen(devName)+1))
			strcpy(newMap->devName, devName);

		// If successful, return an new mapping device.
		// Otherwise, destroy incomplete mapping device
		// and return nothing.
		if (newMap && newMap->devName)
			return newMap;
		DestroyDevice(newMap);
	}

	return NULL;
}

MAP_DEVICE *CreateDevice2(MAP_DEVICE *map, char *devName, int argc, char **argv)
{
	MAP_DEVICE *newMap;

	if ((newMap = (MAP_DEVICE *)calloc(1, sizeof(MAP_DEVICE))) == NULL)
		return NULL;

	return newMap;
}

// Destroy an old mapping device.
void DestroyDevice(MAP_DEVICE *oldMap)
{
	if (oldMap) {
		if (oldMap->devName)
			free(oldMap->devName);
		free(oldMap);
	}
}

// Insert a mapping device into its parent's device table.
MAP_DEVICE *InsertDevice(MAP_DEVICE *map, MAP_DEVICE *newMap)
{
	MAP_DEVICE *cptr, *pptr = NULL;
	int idx;

	if (map) {
		// Order by ascending
		for (cptr = map->sysMap->devList; cptr; cptr = cptr->devNext) {
			if (strcmp(newMap->devName, cptr->devName) < 0)
				break;
			pptr = cptr;
		}

		// Insert a node into linked list (devTable).
		// Also, link a node to its parent device.
		if (cptr == map->sysMap->devList) {
			newMap->devNext      = map->sysMap->devList;
			map->sysMap->devList = newMap;
		} else {
			newMap->devNext = pptr->devNext;
			pptr->devNext   = newMap;
		}
		newMap->devParent = map;

		return newMap;
	}

	return NULL;
}

MAP_DEVICE *FindDevice(MAP_DEVICE *map, char *fndName)
{
	MAP_DEVICE *cptr;

	for (cptr = map->devList; cptr; cptr = cptr->devNext) {
		if (cptr->devName && !strcasecmp(fndName, cptr->devName))
			break;
	}

	return cptr;
}

MAP_DEVICE *GetDevice(MAP_DEVICE *map, int argc, char **argv)
{
	MAP_DEVICE *dptr;

	if (argc < 2) {
		printf("%s: Too few arguments.\n", argv[0]);
		printf("Usage: %s <device> ...\n", argv[0]);
		return NULL;
	}

	RemoveSpaces(argv[1]);
	*StrChar(argv[1], ':') = '\0';
	ToUpper(argv[1]);
	if ((dptr = FindDevice(map, argv[1])) == NULL) {
		printf("%s: Device not found\n", argv[1]);
		return NULL;
	}

	return dptr;
}

DEVICE *GetDeviceInfo(MAP_DEVICE *map, int argc, char **argv)
{
	DEVICE *devInfo = map->devInfo;
	DEVICE *cptr;
	int    idx;

	if (argc < 3)
		printf("Usage: %s %s: <device type> ...\n\n", argv[0], argv[1]);
	else if (devInfo->Devices) {
		for (idx = 0; cptr = devInfo->Devices[idx]; idx++)
			if (!strcasecmp(argv[2], cptr->dtName))
				return cptr;

		ToUpper(argv[2]);
		printf("%s: No Such Device on %s:\n\n", argv[2], map->devName);

		printf("Device List:\n\n");
		for (idx = 0; cptr = devInfo->Devices[idx]; idx++) {
			printf("%-8s %s %s\n",
				cptr->dtName, cptr->emuName, cptr->emuVersion);
		}
		printf("\n");
	} else
		printf("No such device lists on %s: (%s)\n",
			map->devName, map->keyName);

	return NULL;
}

// Extract unit number from device name.
int GetDeviceUnit(char *devName)
{
	int unit = 0; // Assume zero as default.

	// Skip all characters until first digit letter.
	while (*devName && !isdigit(*devName))
		devName++;

	// Convert ASCII to integer for unit number.
	while (*devName && isdigit(*devName)) {
		unit *= 10;
		unit += *devName++ - '0';
	}

	return unit;
}

// ********************************************************

// Usage: boot <device> ...
int CmdBoot(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map;

	if (map = GetDevice(ts10_System, argc, argv)) {
		if (map->devInfo->Boot) {
			// Call Device's Info routine to complete operation.
			return map->devInfo->Boot(map, argc, argv);
		} else {
			printf("%s: Not Supported.\n", argv[0]);
		}
	}

	return EMU_OK;
}

#ifdef DEBUG
// Usage: debug2 <device> ...
int CmdDebug2(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map;

	if (map = GetDevice(ts10_System, argc, argv)) {
		if (map->devInfo->Debug) {
			// Call Device's Info routine to complete operation.
			return map->devInfo->Debug(map, argc, argv);
		} else {
			printf("%s: Not Supported.\n", argv[0]);
		}
	}

	return EMU_OK;
}
#endif /* DEBUG */

// Usage: create <device> <type> ...
int CmdCreate(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map, *newMap;
	void       *newDevice;
	DEVICE     *dptr;
	char       *devName;

	// Make ensure least two arguments 
	if (argc < 3) {
		printf("Usage: %s <device> <type> ...\n\n", argv[0]);
		return EMU_OK;
	}

	// Make device name to captions
	RemoveSpaces((char *)argv[1]);
	*StrChar(argv[1], ':') = '\0';
	ToUpper(argv[1]);

	// Find which system to start...
	map = ts10_System ? ts10_System : ts10_Root;
//	if (map->sysDevice == NULL) {
//		printf("%s: Can't create device on %s:\n", argv[1], map->devName);
//		return EMU_OK;
//	}

	// Check if device name taken or already created first.
	if (FindDevice(map, argv[1])) {
		printf("%s: Device name taken or already created.\n", argv[1]);
		return EMU_OK;
	}

	// Use the device for its system if available.
	if (map->useDevice)
		map = map->useDevice;

	// Get device information
	if ((dptr = GetDeviceInfo(map, argc, argv)) == NULL)
		return EMU_OK;

	// First, set up the new mapping device.
	if ((newMap = CreateDevice(argv[1])) == NULL) {
		printf("%s: Can't create mapping device - memory full\n", argv[1]);
		return EMU_OK;
	}

	newMap->devParent  = map;
	newMap->keyName    = dptr->dtName;
	newMap->emuName    = dptr->emuName;
	newMap->emuVersion = dptr->emuVersion;
	newMap->devInfo    = dptr;
	if (dptr->Flags & DF_SYSMAP) {
		newMap->sysDevice = map->sysDevice;
		newMap->sysMap    = map->sysMap;
	}

	if (newDevice = dptr->Create(newMap, argc, argv)) {
		printf("Created %s on %s: - %s %s\n",
			newMap->keyName,  newMap->devName,
			newMap->emuName, newMap->emuVersion);
		if (map = InsertDevice(map, newMap)) {
			if (dptr->Flags & DF_SELECT)
				ts10_System = map;
			if (dptr->Flags & DF_USE) {
				ts10_System->useDevice = map;
				ts10_Use = map;
			}
		}
	} else {
		printf("%s: *** Can't create %s: (%s): %s\n",
			newMap->devName, newMap->keyName, strerror(errno));
		DestroyDevice(newMap);
	}

	return EMU_OK;
}

// Usage: configure <device> ...
int CmdConfigure(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map, *newMap;
	void       *newDevice;
	DEVICE     *dptr;
	char       *devName;

	// Make ensure least one argument
	if (argc < 2) {
		printf("Usage: %s <device> ...\n\n", argv[0]);
		return EMU_OK;
	}

	// Make device name to captions
	RemoveSpaces((char *)argv[1]);
	*StrChar(argv[1], ':') = '\0';
	ToUpper(argv[1]);

	// Find which device to start...
	if ((map = ts10_Use) == NULL) {
		printf("%s: Use 'USE <device>' first.\n", argv[1]);
		return EMU_OK;
	}

	// Check if device name is existing or not.
	// if not, let's create device name.
	if ((newMap = FindDevice(map, argv[1])) == NULL) {
		if ((newMap = CreateDevice(argv[1])) == NULL) {
			printf("%s: Can't create mapping device - memory full\n", argv[1]);
			return EMU_OK;
		}
		newMap->devParent = map;
	}
	dptr = map->devInfo;

	if (dptr->Configure == NULL) {
		printf("%s: Not supported on %s: (%s)\n",
			newMap->devName, map->devName, map->keyName);
		DestroyDevice(newMap);
	} else if (newDevice = dptr->Configure(newMap, argc, argv)) {
		printf("Configured %s on %s: - %s %s\n",
			newMap->keyName, newMap->devName,
			newMap->emuName, newMap->emuVersion);
		map = InsertDevice(map, newMap);
	} else {
		printf("%s: *** Can't configure %s: %s\n",
			newMap->devName, newMap->keyName, strerror(errno));
		DestroyDevice(newMap);
	}

	return EMU_OK;
}

// Usage: attach <device> ...
int CmdAttach(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map;

	if (map = GetDevice(ts10_System, argc, argv)) {
		if (map->devInfo->Attach) {
			// Call Device's Attach routine to complete operation.
			return map->devInfo->Attach(map, argc, argv);
		} else {
			printf("%s: Not supported.\n", argv[0]);
		}
	}

	return EMU_OK;
}

// Usage: detach <device> ...
int CmdDetach(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map;

	if (map = GetDevice(ts10_System, argc, argv)) {
		if (map->devInfo->Detach) {
			// Call Device's Detach routine to complete operation.
			return map->devInfo->Detach(map, argc, argv);
		} else {
			printf("%s: Not Supported.\n", argv[0]);
		}
	}

	return EMU_OK;
}

// Usage: info <device> ...
int CmdInfo(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map;

	if (map = GetDevice(ts10_System, argc, argv)) {
		if (map->devInfo->Info) {
			// Call Device's Info routine to complete operation.
			return map->devInfo->Info(map, argc, argv);
		} else {
			printf("%s: Not Supported.\n", argv[0]);
		}
	}

	return EMU_OK;
}

// Usage: set <device> <subcommand> ...
int CmdSet2(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map;
	COMMAND    *cmd;
	int        idx;

	if (map = GetDevice(ts10_System, argc, argv)) {
		if (cmd = map->devInfo->SetCommands) {
			for (idx = 0; cmd[idx].Name; idx++) {
				if (!strncasecmp(cmd[idx].Name, argv[2], strlen(argv[2])))
					return cmd[idx].Execute(map->Device, argc, argv);
			}
		}
		printf("%s: Unknown Subcommand: %s\n", map->devName, argv[2]);
	}

	return EMU_OK;
}

// Usage: show <device> [subcommand] ...
int CmdShow2(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map;
	COMMAND    *cmd;
	int        idx;

	if (map = GetDevice(ts10_System, argc, argv)) {
		if (argv[2] && (cmd = map->devInfo->ShowCommands)) {
			for (idx = 0; cmd[idx].Name; idx++) {
				if (!strncasecmp(cmd[idx].Name, argv[2], strlen(argv[2])))
					return cmd[idx].Execute(map->Device, argc, argv);
			}
			printf("%s: Unknown Subcommand: %s\n", map->devName, argv[2]);
		} else if (map->devInfo->Info) 
			return map->devInfo->Info(map, argc, argv);
		else
			printf("%s: No Information.\n", map->devName);
	}

	return EMU_OK;
}

// Usage: select [system|none]
int CmdSelect(void *dev, int argc, char **argv)
{
	MAP_DEVICE *map;

	if (argc < 2) {
		// Display current system selected
		if (ts10_System) {
			printf("%s: (%s) - %s %s\n",
				ts10_System->devName, ts10_System->keyName,
				ts10_System->emuName, ts10_System->emuVersion);
			if (ts10_Use)
				printf("  Using %s: (%s) - %s %s\n",
					ts10_Use->devName, ts10_Use->keyName,
					ts10_Use->emuName, ts10_Use->emuVersion);
		} else
			printf("No System Selected.\n");
	} else {
		*StrChar(argv[1],':') = '\0';
		ToUpper(argv[1]);
		if (!strcasecmp(argv[1], "none")) {
			// Release system.
			printf("Released %s: (%s) - %s %s\n",
				ts10_System->devName, ts10_System->keyName,
				ts10_System->emuName, ts10_System->emuVersion);
			if (ts10_Use)
				printf("  Using %s: (%s) - %s %s\n",
					ts10_Use->devName, ts10_Use->keyName,
					ts10_Use->emuName, ts10_Use->emuVersion);
			ts10_System = NULL;
			ts10_Use    = NULL;
		} else if (map = FindDevice(ts10_Root, argv[1])) {
			// Select system.
			ts10_System = map;
			ts10_Use    = map->useDevice;
			printf("Selected %s: (%s) - %s %s\n",
				ts10_System->devName, ts10_System->keyName,
				ts10_System->emuName, ts10_System->emuVersion);
			if (ts10_Use)
				printf("  Using %s: (%s) - %s %s\n",
					ts10_Use->devName, ts10_Use->keyName,
					ts10_Use->emuName, ts10_Use->emuVersion);
		} else
			printf("%s: No Such System\n", argv[1]);
	}

	return EMU_OK;
}

int CmdUse(void *dptr, int argc, char **argv)
{
	MAP_DEVICE *map;

	if (argc < 2) {
		// Display Current Device
		if (ts10_Use) {
			printf("%s: (%s) on %s: - %s %s\n",
				ts10_Use->devName, ts10_Use->keyName, ts10_System->devName,
				ts10_Use->emuName, ts10_Use->emuVersion);
		} else
			printf("None.\n");
	} else {
		if (ts10_System == NULL) {
			printf("Use 'SELECT <system>' first.\n");
			return EMU_OK;
		}

		*StrChar(argv[1], ':') = '\0';
		if (!strcasecmp(argv[1], "none")) {
			printf("Released %s: (%s) on %s: - %s %s\n",
				ts10_Use->devName, ts10_Use->keyName, ts10_System->devName,
				ts10_Use->emuName, ts10_Use->emuVersion);
			ts10_System->useDevice = NULL;
			ts10_Use = NULL;
	   } else if (map = FindDevice(ts10_System, argv[1])) {
			ts10_System->useDevice = map;
			ts10_Use = map;
			printf("Selected %s: (%s) on %s: - %s %s\n",
				ts10_Use->devName, ts10_Use->keyName, ts10_System->devName,
				ts10_Use->emuName, ts10_Use->emuVersion);
		} else {
			ToUpper(argv[1]);
			printf("%s: No Such Device on %s: (%s)\n",
				argv[1], ts10_System->devName, ts10_System->keyName);
		}
	}

	return EMU_OK;
}

int CmdShowDevice(void *dev, int argc, char **argv)
{
	MAP_DEVICE  *mapInfo, *map;
	DEVICE      *devInfo;
	char        devName[16];
	int         idx, count = 0;

	mapInfo = ts10_System ? ts10_System : ts10_Root;

//	if ((map = ts10_System->devList) == NULL) {
//		printf("Can't display devices on %s:\n", mapInfo->devName);
//		return EMU_OK;
//	}

	// List devices that system has
	for (map = mapInfo->devList; map; map = map->devNext) {
		if (count == 0) {
			printf(
				"\n"
				"Device   Type     Description\n"
				"------   ----     -----------\n"
			);
		}
		sprintf(devName, "%s:", map->devName);
		printf("%-8s %-8s %s %s\n",
			devName, map->keyName, map->emuName, map->emuVersion);
		count++;
	}

	// Tell how many devices that system has
	if (count == 0)
		printf("\n\nNo devices.\n\n");
	else {
		printf("\nTotal %d device%s.\n\n",
			count, (count == 1 ? "" : "s"));
	}

	return EMU_OK;
}

void ListDevices(DEVICE *devInfo, int ident, int flags)
{
	DEVICE *cptr;
	char   keyName[80], *kptr;
	int    idx;

	for (idx = 0; idx < ident; idx++)
		keyName[idx] = ' ';
	kptr = &keyName[idx];

	for (idx = 0; cptr = devInfo->Devices[idx]; idx++) {
		strcpy(kptr, cptr->dtName);
		printf("%-12s %s %s\n", keyName, cptr->emuName, cptr->emuVersion);
		if (flags && cptr->Devices)
			ListDevices(cptr, ident+1, flags-1);
	}
}

int CmdListDevice(void *dev, int argc, char **argv)
{
	printf("Key          Emulator\n");
	printf("---          --------\n");
	ListDevices(ts10_Root->devInfo, 0, 16);

	return EMU_OK;
}
