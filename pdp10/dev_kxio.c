// kxio.c - All Processors: I/O Access Support Routines
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

#include "pdp10/defs.h"
#include "pdp10/iodefs.h"
#include "pdp10/proto.h"

#define PXCT_CUR 0

extern DEVICE dte20_Device;
extern DEVICE rh20_Device;

DEVICE *kx10_Devices[] =
{
	&dte20_Device, // DTE20 front end for KL10 Processor
	&rh20_Device,  // RH20 Massbus disk/tape controller
	NULL
};

#ifdef DEBUG
static cchar *fncNames[] = {
	"BLKI",  // (00) Block In
	"DATAI", // (04) Data In
	"BLKO",  // (10) Block Out
	"DATAO", // (14) Data Out
	"CONO",  // (20) Condition Out
	"CONI",  // (24) Condition In
	"CONSZ", // (30) Condition Skip Zero
	"CONSO", // (34) Condition Skip One
};
#endif /* DEBUG */

P10_IOSYS p10_ioSystem;

// Initialize internal devices for KL10 Processor
void KL10_InitDevices(void)
{
	int idx;

	// Initialize PDP-10 I/O system.
	memset(&p10_ioSystem, 0, sizeof(P10_IOSYS));

	// Initialize all devices.
//	for (idx = 0; idx < 0200; idx++)
//		if (p10_kxDevices[idx] && p10_kxDevices[idx]->Init)
//			p10_kxDevices[idx]->Init(p10_kxData[idx]);
}

// Reset all I/O devices for Kx10 Processor.
void KX10_ResetAllDevices(void)
{
	int idx;

	for (idx = 0; idx < IO_NDEV; idx++) {
		P10_IOMAP *io = p10_ioSystem.ioMap[idx];

		if (io && io->ResetIO) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("KX10: Reset device %s...\n",
					io->keyName);
#endif /* DEBUG */

			// Reset that device.
			io->ResetIO(io->Device);
		}
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("KX10: All I/O Devices had been reset.  Done.\n");
#endif /* DEBUG */
}

void KX10_CleanupAllDevices(void)
{
}

void kx10_SetMap(P10_IOMAP *io)
{
	p10_ioSystem.ioMap[io->idDevice >> 2] = io;
}

void KX10_Opcode_IO(void)
{
	int       opDev  = INST_GETDEV(HR);
	int       opFunc = INST_GETFUNC(HR);
	P10_IOMAP *io    = p10_ioSystem.ioMap[opDev];
	static int count = 0;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("KXIO: Device: %s (%03o) Function: %s (%02o)\n",
			(io ? io->keyName : "(Unknown)"), (opDev << 2),
			fncNames[opFunc], (opFunc << 2));
#endif /* DEBUG */

	if (io != NULL) {
		if (io->Function[opFunc]) {
			io->Function[opFunc](io->Device);
			return;
		}
	}

	// Default function for undefined devices.
	// DATAO, CONO, and CONSO are an no-op.
	// DATAI, and CONI clear the E address as zero.
	// CONSZ is an absolute skip.

	switch (opFunc) {
		case IOF_BLKI:
		case IOF_BLKO:
			AR = p10_vRead(eAddr, PXCT_CUR);
			if (opFunc == IOF_BLKI)
				p10_vWrite(p10_Section | RH(AR), 0, PXCT_CUR);
			if (XLH(AR) >= 0)
				DO_SKIP;
			p10_vWrite(eAddr, AOB(AR), PXCT_CUR);
			break;

		case IOF_DATAI:
		case IOF_CONI:
			p10_vWrite(eAddr, 0, PXCT_CUR);
			break;

		case IOF_CONSZ:
			DO_SKIP;
			break;
	}
}

void kx10_Info(void)
{
	int idx;

	for (idx = 0; idx < 128; idx++) {
		P10_IOMAP *io = p10_ioSystem.ioMap[idx];
		if (io != NULL) {
			printf("%03o %-6s %s %s\n",
				io->idDevice, io->keyName, io->emuName, io->emuVersion);
		}
	}
}
