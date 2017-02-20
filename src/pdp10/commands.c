// commands.c - PDP-10 commands
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
// See README for copyright notice.
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

#include "pdp10/defs.h"
#include "pdp10/ks10.h"
#include "pdp10/ks10_fe.h"
#include "pdp10/proto.h"

int p10_CmdAsm(void *dptr, int argc, char **argv)
{
	printf("Not implemented yet.\n");
	return EMU_OK;
}

int p10_CmdHalt(void *dptr, int argc, char **argv)
{
//	char yes[10];

//	printf("Are you really sure? (Y/N): ");
//	fgets(yes, 10, stdin);
//	RemoveSpaces(yes);

//	if (yes[0] == 'y' || yes[0] == 'Y') {
		// Write ones on location 30 to signal system to
		// being shut down gracely.  That is for
		// KS10 Processor only at this time.

		printf("KS10 System is being halted... ");
		p10_pWrite(FE_HALTSW, -1LL, PTF_CONSOLE);
		printf("Done.\n");
//	}

	return EMU_OK;
}

int p10_CmdDeposit(void *dptr, int argc, char **argv)
{
	static int30 dAddr = 0; // Deposit address (Initially 0)
	char  inbuf[80];
	int36 value;

	if (argc > 1)
		sscanf(argv[1], "%o", &dAddr);

	do {
		value = p10_pRead(dAddr, PTF_CONSOLE);
		printf("%06o/ %012llo - ", dAddr, value);
		fgets(inbuf, 80, stdin);
		RemoveSpaces(inbuf);
		if (inbuf[0]) {
			sscanf(inbuf, "%llo", &value);
			p10_pWrite(dAddr, value, PTF_CONSOLE);
		}
	} while (inbuf[0]);

	return EMU_OK;
}

#ifdef DEBUG

int p10_CmdDisasm(void *dptr, int argc, char **argv)
{
	static int30 sAddr = 0; // Start address (Initially 0)
	static int30 eAddr = 0; // End address (Initially 0)
	int36 opCode;

	if (argc > 1)
		sscanf(argv[1], "%o", &sAddr);
	if (argc > 2) {
		sscanf(argv[2], "%o", &eAddr);
		eAddr += sAddr;
	} else
		eAddr = sAddr + 20;

	while (sAddr < eAddr) {
		opCode = p10_pRead(sAddr, PTF_CONSOLE);
		p10_Disassemble(sAddr++, opCode, 0);
	}

	// Allow this command to being repeated with
	// advancing addresses.
	strcpy(ts10_LastCmd, argv[0]);
	ts10_LastCmdFlag = 1;

	return EMU_OK;
}

#endif /* DEBUG */

int p10_CmdDump(void *dptr, int argc, char **argv)
{
	static int30 sAddr = 0; // Start address (Initially 0)
	static int30 eAddr = 0; // End address (Initially 0)
	int36 data;

	if (argc > 1)
		sscanf(argv[1], "%o", &sAddr);
	if (argc > 2) {
		sscanf(argv[2], "%o", &eAddr);
		eAddr += sAddr;
	} else
		eAddr = sAddr + 20;

	while (sAddr < eAddr) {
		data = p10_pRead(sAddr, PTF_CONSOLE);
		printf("%06o %s\n", sAddr, pdp10_DisplayData(data));
		sAddr++;
	}

	// Allow this command to being repeated with
	// advancing addresses.
	strcpy(ts10_LastCmd, argv[0]);
	ts10_LastCmdFlag = 1;

	return EMU_OK;
}

int p10_CmdLoad(void *dptr, int argc, char **argv)
{
	int st = EMU_OK;

	if (argc != 2)
		return EMU_ARG;
	else {
		RemoveSpaces(argv[1]);
		if ((st = pdp10_LoadExeFile(argv[1])) == EMU_OK)
			printf("File '%s' had been loaded.\n", argv[1]);
	}
	return st;
}

int p10_CmdRim(void *dptr, int argc, char **argv)
{
	int st = EMU_OK;

	if (argc != 2)
		return EMU_ARG;
	else {
		RemoveSpaces(argv[1]);
		if ((st = pdp10_LoadRimFile(argv[1])) == EMU_OK)
			printf("File '%s' had been loaded.\n", argv[1]);
	}
	return st;
}

COMMAND p10_Commands[] = {
	{ "asm",      "{Not Implemented Yet}",  p10_CmdAsm      },
	{ "halt",     "",                       p10_CmdHalt     },
	{ "deposit",  "[addr]",                 p10_CmdDeposit  },
#ifdef DEBUG
	{ "disasm",   "[addr] [count]",         p10_CmdDisasm   } ,
#endif /* DEBUG */
	{ "dump",     "[addr] [count]",         p10_CmdDump     },
	{ "load",     "<filename>",             p10_CmdLoad     },
	{ "rim",      "<filename>",             p10_CmdRim      },
	{ NULL,       NULL,                     NULL            },
};

COMMAND p10_SetCommands[] = {
	{ NULL, NULL, NULL },
};

COMMAND p10_ShowCommands[] = {
	{ NULL, NULL, NULL },
};
