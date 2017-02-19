// kx10_io.c - All Processors: I/O access routines.
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

#include "pdp10/defs.h"

#define PXCT_CUR 0

// Function Code Definitions
#define IOF_BLKI   0 // (00) Block In
#define IOF_DATAI  1 // (04) Data In
#define IOF_BLKO   2 // (10) Block Out
#define IOF_DATAO  3 // (14) Data Out
#define IOF_CONO   4 // (20) Conditions Out
#define IOF_CONI   5 // (24) Conditions In
#define IOF_CONSZ  6 // (30) Conditions Skip Zero
#define IOF_CONSO  7 // (34) Conditiion Skip Ones

// I/O Device Table
typedef struct {
	char      *Name;   // User-defined device name
	P10DEVICE *Device; // Device
	void      *Data;   // Data
} P10_IODEVICE;

// Up to 128 devices per CPU.
void         *p10_kxData[0200];
P10DEVICE    *p10_kxDevices[0200];
P10_IODEVICE p10_ioDevices[0200];

extern P10DEVICE KL10_Device_APR;
extern P10DEVICE KL10_Device_PI;
extern P10DEVICE KL10_Device_PAG;
extern P10DEVICE KL10_Device_CCA;
extern P10DEVICE KL10_Device_TIM;
extern P10DEVICE KL10_Device_MTR;

extern P10DEVICE KX10_Device_DTE20;
extern P10DEVICE KX10_Device_RH20;

P10DEVICE *p10_xxDevices[] =
{
	&KX10_Device_DTE20, // DTE20 front end for KL10 Processor
	&KX10_Device_RH20,  // RH20 Massbus disk/tape controller
	NULL
};

#ifdef DEBUG
char *p10_fncNames[] = {
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

#if 0
// Create Kx10 I/O devices from script file
void *kxio_Create(char *devName, int argc, char **argv)
{
	// Usage: create ... <device> <address> <channel>

	P10DEVICE *dptr = NULL;
	int       ioAddr, ioChan;
	int       idx;

	// If there is a pointer, find it and call deeper Create routine.
	if (*StrChar(devName, '>') == '>') {
		char *fndName = devName;

		// Get next device name.
		devName = StrChar(devName, '>');
		*devName++ = '\0';

		if (*devName == '\0') {
			printf("%s: Undefined Device Pointer.\n", fndName);
			return NULL;
		}

		// Find a parent device first
		for (idx = 0; idx < 0200; idx++) {
			if (p10_ioDevices[idx].Name &&
			    !strcasecmp(fndName, p10_ioDevices[idx].Name)) {
				dptr = p10_kxDevices[idx];
				break;
			}
		}
		if (dptr == NULL) {
			printf("%s: No such device", fndName);
			return NULL;
		}

		return dptr->Create(p10_kxData[idx], devName, argc, argv);
	}

	// Remove a colon from end of device name if there is.
	*StrChar(devName, ':') = '\0';

	// Find a PDP-10 device to be set up.
	for (idx = 0; p10_xxDevices[idx]; idx++) {
		if (!strcasecmp(argv[0], p10_xxDevices[idx]->Name)) {
			dptr = p10_xxDevices[idx];
			break;
		}
	}

	// If a device is not found, aborted.
	if (dptr == NULL) {
		printf("%s: Device %s not found - Aborted.\n", devName, argv[0]);
		return NULL;
	}

	sscanf(argv[1], "%o", &ioAddr);
	sscanf(argv[2], "%o", &ioChan);
	ioAddr >>= 2;

	if (p10_kxDevices[ioAddr]) {
		printf("%s: Already created - Aborted.\n", devName);
		return NULL;
	}

	// Now set up a new device on empty slot.
	p10_kxDevices[ioAddr] = dptr;
	if (dptr->Create)
		p10_kxData[ioAddr] = dptr->Create(NULL, devName, ioChan, NULL);
	p10_ioDevices[ioAddr].Name = (char *)malloc(strlen(devName)+1);
	strcpy(p10_ioDevices[ioAddr].Name, devName);

	return p10_kxData[ioAddr];
}
#endif

// Initialize internal devices for KL10 Processor
void KL10_InitDevices(void)
{
	int idx;

	for (idx = 0; idx < 0200; idx++) {
		p10_kxData[idx] = NULL;
		p10_kxDevices[idx] = NULL;
	}

	// KL10 Processor - 6 internal devices
	p10_kxDevices[0] = &KL10_Device_APR;
	p10_kxDevices[1] = &KL10_Device_PI;
	p10_kxDevices[2] = &KL10_Device_PAG;
	p10_kxDevices[3] = &KL10_Device_CCA;
	p10_kxDevices[4] = &KL10_Device_TIM;
	p10_kxDevices[5] = &KL10_Device_MTR;

	// Initialize all devices.
	for (idx = 0; idx < 0200; idx++)
		if (p10_kxDevices[idx] && p10_kxDevices[idx]->Init)
			p10_kxDevices[idx]->Init(p10_kxData[idx]);
}

// Reset all I/O devices for Kx10 Processor.
void KX10_ResetAllDevices(void)
{
	int idx;

	for (idx = 0; idx < 0200; idx++) {
		if (p10_kxDevices[idx] && p10_kxDevices[idx]->Reset) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("KX10: Reset device %s...\n",
					p10_kxDevices[idx]->Name);
#endif /* DEBUG */

			// Reset that device.
			p10_kxDevices[idx]->Reset(p10_kxData[idx]);
		}
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("KX10: All I/O Devices had been reset.  Done.\n");
#endif /* DEBUG */
}

void KX10_CleanupAllDevices(void)
{
	int idx;

	// Clean up and free all devices.
	for (idx = 0; idx < 0200; idx++) {
		if (p10_kxDevices[idx]) {
			if (p10_kxDevices[idx]->Cleanup)
				p10_kxDevices[idx]->Cleanup(p10_kxData[idx]);
			p10_kxDevices[idx] = NULL;
			p10_kxData[idx] = NULL;
		}
	}
}

void KX10_Opcode_IO(void)
{
	int  opDev  = INST_GETDEV(HR);
	int  opFunc = INST_GETFUNC(HR);
	void (*DoFunction)(void *);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		char *devName = p10_kxDevices[opDev] ?
			p10_kxDevices[opDev]->Name : "(Unknown)";
		char *fncName = p10_fncNames[opFunc];

		dbg_Printf("KXIO: Device: %s (%03o) Function: %s (%02o)\n",
			devName, (opDev << 2), fncName, (opFunc << 2));
	}
#endif /* DEBUG */

	if (p10_kxDevices[opDev]) {
		if (DoFunction = p10_kxDevices[opDev]->Function[opFunc]) {
			DoFunction(p10_kxData[opDev]);
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

// ****************************************************************
