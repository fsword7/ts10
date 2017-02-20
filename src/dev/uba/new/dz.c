// dz.c - DZ11 Communication Systems with socket interface
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
#include "emu/socket.h"
#include "dev/dz.h"

#ifdef DEBUG
// Read I/O Registers
char *dz_rdRegName[] =
{
	"DZCSR",  // (R) Control and Status Register  (CSR)
	"DZRBUF", // (R) Receiver Buffer Register     (RBUF)
	"DZTCR",  // (R) Transmit Control Register    (TCR)
	"DZMSR"   // (R) Modem Status Register        (MSR)
};

// Write I/O Registers
char *dz_wrRegName[] =
{
	"DZCSR",  // (W) Control and Status Register  (CSR)
	"DZLPR",  // (W) Line Parameter Register      (LPR)
	"DZTCR",  // (W) Transmit Control Register    (TCR)
	"DZTDR"   // (W) Transmit Data Register       (TDR)
};
#endif DEBUG

DTYPE dz_dTypes[] =
{
	{
		"DZ11",        // Terminal Device Name
		"Communication Systems",
		UNIT_DISABLE,  // Controller is disable
		0, 0, 0, 0,    // Not used
		000000,        // Drive Type ID - Not Used
		NULL,          // Slave Drive Types - Not Used

		UT_TERMINAL,   // DZ11 is a terminal server
		NULL           // Local DZ functions
	},

	{ NULL }
};

DEVICE dz_Device =
{
	"DZ",            // Device Name
	"DZ Communication Systems",
	"v0.1 (Alpha)",  // Version
	dz_dTypes,       // Drive Type
	NULL,            // Units
	NULL,            // Listing of devices
	NULL,            // List of Commands
	NULL,            // List of Set Commands
	NULL,            // List of Show Commands

	0,               // Number of Devices
	0,               // Number of Units

	dz_Initialize,   // Initialize Routine
	dz_Reset,        // Reset Routine
	dz_Create,       // Create Routine
	dz_Delete,       // Delete Routine
	NULL,            // Configure Routine
	NULL,            // Enable Routine
	NULL,            // Disable Routine
	NULL,            // Attach/Mount Routine
	NULL,            // Detach/Unmount Routine
	NULL,            // Format Routine
	dz_ReadIO,       // Read I/O Routine
	dz_WriteIO,      // Write I/O Routine
	dz_Process,      // Process Routine
	NULL,            // Boot Routine
	NULL,            // Execute Routine
};

// Create the DZ terminal server
int dz_Create(UNIT *pUnit, char *devName, int argc, char **argv)
{
	UNIT *uptr;
	int unit;
	int len, st;
	int idx1, idx2;

	len = strlen(rh_Device.Name);
	unit = toupper(devName[len++]) - 'A';
	uptr = &pUnit->sUnits[unit];

	if (!(uptr->Flags & UNIT_PRESENT)) {
		UNIT     *rhUnits = (UNIT *)calloc(sizeof(UNIT), RH_MAXUNITS);

		if (rhUnits && rhData) {
			char  *pAddress, *epAddress;
			char  *pMask,    *epMask;
			char  *pIntBR,   *epIntBR;
			char  *pIntVec,  *epIntVec;
			int32 nAddress, nMask;
			int32 nIntBR, nIntVec;
			int idx;

			for (idx1 = 0; rh_dTypes[idx1].Name; idx1++) {
				if (!strcasecmp(argv[0], rh_dTypes[idx1].Name))
					break;
			}

			if (!rh_dTypes[idx1].Name) {
				free(rhUnits);
				free(rhData);
				return EMU_ARG;
			}

			pAddress = argv[1];
			pMask    = argv[2];
			pIntBR   = argv[3];
			pIntVec  = argv[4];

			nAddress = ToInteger(pAddress, &epAddress, 8);
			nMask    = ToInteger(pMask, &epMask, 8);
			nIntBR   = ToInteger(pIntBR, &epIntBR, 8);
			nIntVec  = ToInteger(pIntVec, &epIntVec, 8);

			if ((pAddress == epAddress) || (pMask == epMask) ||
			    (pIntBR == epIntBR) || (pIntVec == epIntVec)) {
				free(rhUnits);
				free(rhData);
				return EMU_ARG;
			}
				
			if (pUnit) {
				int st;

				if (st = pUnit->Device->Configure(pUnit, NULL, nAddress, nMask)) {
					free(rhUnits);
					free(rhData);
					return st;
				}
			}

			*strchr(argv[5], ':') = '\0';
			if (st = unit_mapCreateDevice(argv[5], uptr)) {
				free(rhUnits);
				free(rhData);
				return st;
			}

			printf("Addr=%06o Mask=%06o IntBR=%d IntVec=%03o\n",
				nAddress, nMask, nIntBR, nIntVec);

			rhData->Address    = nAddress;
			rhData->Mask       = nMask;
			rhData->IntBR      = nIntBR;
			rhData->IntVec     = nIntVec;

			for (idx2 = 0; idx2 < RH_MAXUNITS; idx2++) {
				rhUnits[idx2].idUnit = idx2;
				rhUnits[idx2].pUnit  = uptr;
			}

			// This unit is a RH11 controller
			uptr->tFlags = UT_CONTROLLER;
			uptr->dType  = &rh_dTypes[idx1];
			uptr->Device = &rh_Device;
			uptr->sUnits = rhUnits;
			uptr->nUnits = RH_MAXUNITS;
			uptr->Flags  = rh_dTypes[idx1].Flags | UNIT_PRESENT;
			uptr->pUnit  = pUnit;
			uptr->uData  = (void *)rhData;

			if (pUnit)
				pUnit->Device->Configure(pUnit, uptr, nAddress, nMask);
		} else
			return EMU_MEMERR;
	} else
		return EMU_ARG;
	return EMU_OK;
}

// Delete the desired DZ terminal server
int dz_Delete(UNIT *dzUnit)
{
	return EMU_OK;
}

int dz_Setup(UNIT *dzUnit)
{
	DZMUNIT *dzData;
	SOCKET  *dzServer;

	dzData = (DZMUNIT *)calloc(1, sizeof(DZMUNIT));
	if (dzData) {
		if (dzServer = sock_Open(5001, NET_SERVER)) {
			dzServer->maxConns = 0;
			dzServer->nConns   = 0;
			dzServer->Accept   = dz_Accept;
			dzServer->Eof      = NULL;
			dzServer->Process  = NULL;

			// Now accept incoming connections.
			sock_Listen(dzServer, 5);

			dzData->dzServer = dzServer;
			dzServer->uData  = (void *)dzData;
		}

		dzData->nPorts = 0;

		dzUnit->uData = (void *)dzData;
	} else
		return EMU_MEMERR;

	return EMU_OK;
}

void dz_Accept(SOCKET *dzServer)
{
	DZMUNIT *dzData = (DZMUNIT *)dzServer->uData;
	SOCKET  *newSocket;

	if (newSocket = sock_Accept(dzServer)) {
		// First, check if any port is available for this socket.
		if (dzData->nConns >= dzData->maxConns) {
			// No, tell user that all connections are busy.
			sock_Send(newSocket->idSocket,
				"Welcome to TS10 Emulator\r\n\r\n"
				"Sorry, All connections are busy. Try later.\r\n"
				"Terminated\r\n", 0);
			sock_Close(newSocket);
			return;
		}

		// Yes, assign a new port to this socket...
		newSocket->uPort = newPort;
		newSocket->uData = (void *)dzData;

		// Send telnet initialization codes and welcome message.
		sock_Send(newSocket->idSocket, dz_telnetInit, dzn_telnetInit);
		sock_Send(newSocket->idSocket, "Welcome to TS10 Emulator\r\n\r\n", 0);
	}
}

void dz_Initialize(UNIT *dzUnit)
{
	DZSUNIT *dzData = (DZSUNIT *)dzUnit->uData;

	dzData->dzcsr = DZCSR_RDONE;
}

void dz_Reset(UNIT *dzUnit)
{
	DZSUNIT *dzData = (DZSUNIT *)dzUnit->uData;

	dzData->dzcsr = DZCSR_RDONE;
}

// ACTION: Need re-write for IPL handling
void dz_DoInterrupt(UNIT *dzUnit)
{
	DZSUNIT *dzData = (DZSUNIT *)dzUnit->uData;

	ks10uba_DoInterrupt(dzUnit->pUnit, dzData->IntIPL, dzData->IntVec);
}

void dz_WriteIO(UNIT *dzUnit, int32 ioAddr, int32 data)
{
	DZSUNIT *dzData  = (DZSUNIT *)dzUnit->uData;
	int32   reg      = ioAddr - dzData->CSR;

	switch (reg >> 1) {
		case DZCSR:
			dzData->dzcsr = (dzData->dzcsr & ~DZCSR_WRMASK) |
				(data & DZCSR_WRMASK);

			// Reset DZ terminal controller.
			if (data & DZCSR_CLR)
				dz_Reset(dzUnit);

			break;

		default: // Unknown Register
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unknown Register: %02o (%06o)\n",
					dzUnit->dType->Name, reg, addr);
#endif DEBUG
			return;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: %s (%02o) <= %06o (%04X)\n",
			dzUnit->dType->Name, dz_wrRegName[reg >> 1], reg, data, data);
#endif DEBUG
}

int32 dz_ReadIO(UNIT *dzUnit, int32 ioAddr)
{
	DZSUNIT *rhData  = (DZSUNIT *)dzUnit->uData;
	int32  reg       = ioAddr - dzData->CSR;
	int32  data;

	switch (reg >> 1) {
		case DZCSR:
			data = dzData->dzcsr;
			break;

		default: // Unknown register
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unknown Register: %02o (%06o)\n",
					dzUnit->dType->Name, reg, ioAddr);
#endif DEBUG
			data = 0;
			return data;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: %s (%02o) => %06o (%04X)\n",
			dzUnit->dType->Name, dz_rdRegName[reg >> 1], reg, data, data);
#endif DEBUG
	return data;
}

int dz_Process(UNIT *dzUnit)
{
	return EMU_OK;
}
