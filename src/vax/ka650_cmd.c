// ka650_cmd.c - KA650 Series, CPU Commands.
//
// Copyright (c) 2003, Timothy M. Stark
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
//
// -------------------------------------------------------------------------
//
// Modification History:
//
// XX/XX/XX  TMS  Comments Here
//
// -------------------------------------------------------------------------

#include "vax/defs.h"
#include "dev/uba/dec/defs.h"
#include "vax/ka650.h"

int ka650_CmdSetMemory(void *dptr, int argc, char **argv)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)dptr;
	uint32        ramSize;

	if (ramSize = vax_SetMemory((VAX_CPU *)ka650, argv[2])) {
		printf("%s: Set memory space to %s (%d) bytes.\n",
			ka650->cpu.devName, argv[2], ramSize);
	}
	return VAX_OK;
}

// Command Table
COMMAND ka650_Commands[] =
{
	{ NULL } // Null Terminator
};

// Set Command Table
COMMAND ka650_SetCommands[] =
{
	{ "ram", "<bytes>", ka650_CmdSetMemory },
	{ NULL } // Null Terminator
};

// Show Command Table
COMMAND ka650_ShowCommands[] =
{
	{ NULL } // Null Terminator
};
