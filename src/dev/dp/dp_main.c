// dp_main.c - Device Process, main (support) routines
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

#include "emu/defs.h"
#include "dev/dp/dp.h"

// Initialize DP doorbell/transfer area
void dp_InitDoorbell(DPC *dpc, DPX *dpx,
	uint32 Type, uint32 Signal, uint32 Offset, uint32 Size)
{
	dpx->Type      = Type;
	dpx->Signal    = Signal;
	dpx->ParentPID = getpid();

	// Set up buffer area
	dpx->Buffer    = dpc + Offset;
	dpx->Offset    = Offset;
	dpx->Size      = Size;
}

// Create Device Process Communication Area
int dp_Create(DP *dp, uint32 dpcSize,
	uint32 outType, uint32 outSignal, uint32 outSize,
	uint32 inType,  uint32 inSignal,  uint32 inSize)
{
	DPC    *dpc;
	uint32 totSize   = dpcSize + inSize + outSize;
	uint32 outOffset = dpcSize;
	uint32 inOffset  = dpcSize + outSize;
	int32  shmid;

	// Initialize DP variables
	dp->Type     = 0;
	dp->dpc      = NULL;
	dp->ShMemID  = -1;
	dp->ChildPID = 0;

	// Create shared memory for DP communication area
	if ((shmid = shmget(IPC_PRIVATE, totSize, 0600)) < 0) {
		printf("DP: Can't create shared memory - %s\n", strerror(errno));
		return DP_NOSHM;
	}
	
	if ((dpc = (DPC *)shmat(shmid, NULL, SHM_RND)) == NULL) {
		printf("DP: Can't create shared memory - %s\n", strerror(errno));
		shmctl(shmid, IPC_RMID, NULL);
		return DP_NOADDR;
	}

	memset(dpc, 0, totSize);
	dpc->Version = DP_VERSION;
	dpc->Offset  = dpcSize;

	// Initialize Doorbell/Transfer Area
	dp_InitDoorbell(dpc, &dpc->toDevice,
		outType, outSignal, outOffset, outSize);
//	dp_InitDoorbell(dpc, &dpc->fromDevice,
//		inType,  inSignal,  inOffset,  inSize);

	dp->dpc     = dpc;
	dp->ShMemID = shmid;

	return DP_OK;
}

int dp_Start(DPC *dpc)
{
	int32 pid;

	if ((pid = fork()) < 0) {
		printf("DP: Can't fork - %s\n", strerror(errno));
		return DP_NOFORK;
	} else if (pid == 0) {
		// Child Process Here

		printf("Hey, I am child process - %d\n", getpid());

		_exit(1);
	}

	return DP_OK;
}

