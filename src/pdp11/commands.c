// commands.c - Commands for PDP-11 Emulator
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

#include "pdp11/defs.h"
#include "emu/socket.h"

extern SOCKET *ts10_Stdout;

#ifdef DEBUG

extern void p11_Disasm(register P11_CPU *, SOCKET *, uint32 *, uint32);
extern void p11_Dump(P11_CPU *, SOCKET *, uint32 *, uint32, uint32);

int p11_CmdAsm(void *dptr, int argc, char **argv)
{
	P11_CPU *p11 = (P11_CPU *)dptr;
	printf("Not implemented yet.\n");
	return EMU_OK;
}

int p11_CmdDisasm(void *dptr, int argc, char **argv)
{
	P11_CPU *p11 = (P11_CPU *)dptr;
	char    *strAddr;
	static  uint32 sAddr = 0; // Start address (Initially 0)
	uint32  eAddr = -1;
	int32   count = -1;

	if (argc > 1) {
		sscanf(argv[1], "%o", &sAddr);
		if (strAddr = strchr(argv[1], '-'))
			sscanf(strAddr+1, "%o", &eAddr);
	}

	if (argc > 2)
		sscanf(argv[2], "%d", &count);
	else if (eAddr == -1)
		count = 20;

	// Display disassembly listing.
	if (count > 0) {
		while (count--)
			p11_Disasm(p11, ts10_Stdout, &sAddr, SWMASK('v'));
	} else {
		while (sAddr < eAddr)
			p11_Disasm(p11, ts10_Stdout, &sAddr, SWMASK('v'));
	}

	// Allow this command to be repeated with advancing addresses.
	strcpy(ts10_LastCmd, argv[0]);
	ts10_LastCmdFlag = 1;

	return EMU_OK;
}

int p11_CmdDump(void *dptr, int argc, char **argv)
{
	P11_CPU       *p11 = (P11_CPU *)dptr;
	static uint32 sAddr = 0;  // Start address (Initially 0)
	uint32        eAddr = -1; // End Address
	char          *strAddr;
//	static uint32 sw = SWMASK('v'); // Memory Access Mode (Initially Virtual)
	static uint32 sw = 0;

	if (argc > 1) {
		sscanf(argv[1], "%o", &sAddr);
		if (strAddr = strchr(argv[1], '-'))
			sscanf(strAddr+1, "%o", &eAddr);
	}

	if (argc > 2) {
		sscanf(argv[2], "%o", &eAddr);
		eAddr = sAddr + eAddr - 1;
	} else if (eAddr == -1)
		eAddr = sAddr + 0200 - 1;

	p11_Dump(p11, ts10_Stdout, &sAddr, eAddr, sw);

	// Allow this command to be repeated with advancing addresses.
	strcpy(ts10_LastCmd, argv[0]);
	ts10_LastCmdFlag = 1;

	return EMU_OK;
}

#endif /* DEBUG */

// Stop the VAX emulator
int p11_CmdStop(void *dptr, int argc, char **argv)
{
	P11_CPU *p11 = (P11_CPU *)dptr;

	// Hardware halt action
	// Signal the VAX emulator to stop by force.
	emu_State = P11_HALT;
	
	return EMU_OK;
}

int p11_CmdShowMemory(void *dptr, int argc, char **argv)
{
	P11_CPU *p11 = (P11_CPU *)dptr;
	int     idx;

	printf("MMR0 = %06o\n", MMR0);
	printf("MMR1 = %06o\n", MMR1);
	printf("MMR2 = %06o\n", MMR2);
	printf("MMR3 = %06o\n", MMR3);

	printf("\nKernel, I/D-Space:\n\n");
	for (idx = APR_KERNEL; idx < (APR_KERNEL + APR_NMODE); idx++)
		printf("  APR %02o = %06o %06o, %s\n", idx, PAR(idx), PDR(idx));

	printf("\nSupervisor, I/D-Space:\n\n");
	for (idx = APR_SUPER; idx < (APR_SUPER + APR_NMODE); idx++)
		printf("  APR %02o = %06o %06o\n", idx, PAR(idx), PDR(idx));

	printf("\nUser, I/D-Space:\n\n");
	for (idx = APR_USER; idx < (APR_USER + APR_NMODE); idx++)
		printf("  APR %02o = %06o %06o\n", idx, PAR(idx), PDR(idx));
}

COMMAND p11_Commands[] = {
#ifdef DEBUG
	{ "asm",     "{Not Implemented Yet}",   p11_CmdAsm     },
	{ "disasm",  "[start[-end]] [count]",   p11_CmdDisasm  },
	{ "dump",    "[srart[-end]] [length]",  p11_CmdDump    },
#endif /* DEBUG */
//	{ "halt",    "",                        p11_CmdHalt    },
//	{ "load",    "<filename> [address]",    p11_CmdLoad    },
//	{ "rom",     "<filename> [address]",    p11_CmdLoadROM },
	{ "stop",    "",                        p11_CmdStop    },
	{ NULL,      NULL,                      NULL           },
};

COMMAND p11_SetCommands[] = {
	{ NULL, NULL, NULL }
};

COMMAND p11_ShowCommands[] = {
	{ "memory", "", p11_CmdShowMemory },
	{ NULL, NULL, NULL }
};
