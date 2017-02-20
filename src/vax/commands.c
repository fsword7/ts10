// commands.c - VAX commands
//
// Copyright (c) 2001-2002, Timothy M. Stark
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

#include "vax/defs.h"
#include "emu/socket.h"

extern SOCKET *ts10_Stdout;

#ifdef DEBUG
int vax_Disasm(register VAX_CPU *, SOCKET *, uint32 *, uint32);
int vax_Dump(VAX_CPU *, SOCKET *, uint32 *, uint32, uint32);

int vax_CmdAsm(void *dptr, int argc, char **argv)
{
	VAX_SYSTEM *vax = (VAX_SYSTEM *)dptr;
	printf("Not implemented yet.\n");
	return EMU_OK;
}

int vax_CmdDisasm(void *dptr, int argc, char **argv)
{
	VAX_CPU *vax = ((VAX_SYSTEM *)dptr)->Processor;
	char    *strAddr;
	static  uint32 sAddr = 0; // Start address (Initially 0)
	uint32  eAddr = -1;
	int32   count = -1;


	if (argc > 1) {
		sscanf(argv[1], "%x", &sAddr);
		if (strAddr = strchr(argv[1], '-'))
			sscanf(strAddr+1, "%x", &eAddr);
	}

	if (argc > 2)
		sscanf(argv[2], "%d", &count);
	else if (eAddr == -1)
		count = 20;

	// Display disassembly listing.
	if (count > 0) {
		while (count--)
			vax_Disasm(vax, ts10_Stdout, &sAddr, SWMASK('v'));
	} else {
		while (sAddr < eAddr)
			vax_Disasm(vax, ts10_Stdout, &sAddr, SWMASK('v'));
	}

	// Allow this command to be repeated with advancing addresses.
	strcpy(ts10_LastCmd, argv[0]);
	ts10_LastCmdFlag = 1;

	return EMU_OK;
}

int vax_CmdDump(void *dptr, int argc, char **argv)
{
	VAX_CPU       *vax = ((VAX_SYSTEM *)dptr)->Processor;
	static uint32 sAddr = 0;  // Start address (Initially 0)
	uint32        eAddr = -1; // End Address
	char          *strAddr;
	static uint32 sw = SWMASK('v'); // Memory Access Mode (Initially Virtual)


	if (argc > 1) {
		sscanf(argv[1], "%x", &sAddr);
		if (strAddr = strchr(argv[1], '-'))
			sscanf(strAddr+1, "%x", &eAddr);
	}

	if (argc > 2) {
		sscanf(argv[2], "%x", &eAddr);
		eAddr = sAddr + eAddr - 1;
	} else if (eAddr == -1)
		eAddr = sAddr + 0x140 - 1;

	vax_Dump(vax, ts10_Stdout, &sAddr, eAddr, sw);

	// Allow this command to be repeated with advancing addresses.
	strcpy(ts10_LastCmd, argv[0]);
	ts10_LastCmdFlag = 1;

	return EMU_OK;
}

#endif /* DEBUG */

int vax_CmdHalt(void *dptr, int argc, char **argv)
{
	VAX_SYSTEM *vax = (VAX_SYSTEM *)dptr;
	VAX_CPU    *cpu = vax->Processor;

	// Software halt action
	if (cpu->HaltAction)
		emu_State = VAX_SWHALT;
	else
		printf("Halt is not supported.\n");

	return EMU_OK;
}

int vax_CmdLoad(void *dptr, int argc, char **argv)
{
	VAX_SYSTEM *vax = (VAX_SYSTEM *)dptr;
	VAX_CPU    *cpu = vax->Processor;
	uint32     sAddr = 0;
	int        st = EMU_OK;

	if (argc < 2)
		return EMU_ARG;
	else {
		RemoveSpaces(argv[1]);
		if (argc >= 3)
			sscanf(argv[2],"%x", &sAddr);
		if ((st = vax_LoadFile(cpu, argv[1], sAddr, NULL)) == EMU_OK)
			printf("File '%s' had been loaded.\n", argv[1]);
	}
	return st;
}

int vax_CmdLoadROM(void *dptr, int argc, char **argv)
{
	VAX_SYSTEM *vax  = (VAX_SYSTEM *)dptr;
	VAX_CPU    *cpu  = vax->Processor;
	uint32     sAddr = 0x20040000; // Default ROM address
	int        st    = EMU_OK;

	if (argc != 2)
		return EMU_ARG;
	else {
		RemoveSpaces(argv[1]);
		if (argc >= 3)
			sscanf(argv[2],"%x", &sAddr);
		if ((st = vax_LoadROM(cpu, argv[1], sAddr)) == EMU_OK) {
			printf("File '%s' had been loaded.\n", argv[1]);
			printf("  ROM Base = %08X  Length = %d bytes\n",
				cpu->baseROM, cpu->sizeROM);
		}
	}
	return st;
}

// Stop the VAX emulator
int vax_CmdStop(void *dptr, int argc, char **argv)
{
	VAX_SYSTEM *vax = (VAX_SYSTEM *)dptr;

	// Hardware halt action
	// Signal the VAX emulator to stop by force.
	emu_State = VAX_HALT;
	
	return EMU_OK;
}

COMMAND vax_Commands[] = {
#ifdef DEBUG
	{ "asm",     "{Not Implemented Yet}",   vax_CmdAsm     },
	{ "disasm",  "[start[-end]] [count]",   vax_CmdDisasm  },
	{ "dump",    "[srart[-end]] [length]",  vax_CmdDump    },
#endif /* DEBUG */
	{ "halt",    "",                        vax_CmdHalt    },
	{ "load",    "<filename> [address]",    vax_CmdLoad    },
	{ "rom",     "<filename> [address]",    vax_CmdLoadROM },
	{ "stop",    "",                        vax_CmdStop    },
	{ NULL,      NULL,                      NULL           },
};

COMMAND vax_SetCommands[] = {
	{ NULL, NULL, NULL }
};

COMMAND vax_ShowCommands[] = {
	{ "tlb",  "",   vax_ShowTLB },
	{ NULL,   NULL, NULL        }
};

