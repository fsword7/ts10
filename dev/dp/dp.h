// dp.h - Device Process Support Definitions
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

#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

// Device Process - Communication Area
//
// +----------------------------------+
// |     DPC - Communication Area     |
// +----------------------------------+
// |     DPX - To Device Doorbell     |
// +----------------------------------+
// |     DPX - From Device Doorbell   |
// +----------------------------------+
// |        User-defined Area         |
// +----------------------------------+
// |         Transmit Buffer          |
// +----------------------------------+
// |         Receive Buffer           |
// +----------------------------------+

typedef struct dp  DP;   // Device Process - Handler
typedef struct dpc DPC;  // Device Process - Communication Area
typedef struct dpx DPX;  // Device Process - Doorbell/Transfer Area

// Device Process - Doorbell/Transfer Area
struct dpx {
	uint32 Command;   // Command
	uint32 Result;    // Result from device
	uint32 Flags;     // Doorbell Flags
	uint32 Ready;     // Ready/Done or Busy Flag

	uint32 Type;      // Communication Type
	uint32 Signal;    // Signal Number
	uint32 ParentPID; // Parent Process Identification

	void   *Buffer;   // Buffer Area
	uint32 Offset;    // Offset from DPC address base
	uint32 Size;      // Buffer Length
};

// Device Process - Communication Area
struct dpc {
	uint32 Version;     // Device Process Version
	uint32 Offset;      // Buffer Area, Offset from base.

	DPX    toDevice;    // Transmit data/results to device.
	DPX    fromDevice;  // Receive data/results from device.
};

struct dp {
	uint32 Type;      // Communication Type
	DPC    *dpc;      // Communication Area (in shared memory area)
	uint32 ShMemID;   // Shared Memory Identification
	uint32 ChildPID;  // Chlid Process Identification
};

#define DP_VERSION 0  // (Pre-Alpha)

#define DP_OK     0   // Successful
#define DP_NOSHM  1   // No Shared Memory
#define DP_NOADDR 2   // No Shared Address
#define DP_NOFORK 3   // Fork failure

