// defs.h - PDP-10 Device Support Defintions.
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

#define IO_NDEV   128  // Number of I/O devices per PDP-10 processor
#define IO_NFNC   8    // Number of I/O functions per device.

// Function Code Definitions
#define IOF_BLKI   0 // (00) Block In
#define IOF_DATAI  1 // (04) Data In
#define IOF_BLKO   2 // (10) Block Out
#define IOF_DATAO  3 // (14) Data Out
#define IOF_CONO   4 // (20) Conditions Out
#define IOF_CONI   5 // (24) Conditions In
#define IOF_CONSZ  6 // (30) Conditions Skip Zero
#define IOF_CONSO  7 // (34) Conditiion Skip Ones

typedef struct p10_IOSys  P10_IOSYS;
typedef struct p10_IOMap  P10_IOMAP;

struct p10_IOSys {

	// I/O Device Table
	P10_IOMAP *ioMap[IO_NDEV];
};

// I/O Device Table
struct p10_IOMap {
	char *devName;    // Device Name
	char *keyName;    // Devce Type (Key) Name
	char *emuName;    // Emulator Name
	char *emuVersion; // Emulator Version

	void *Device;     // Individual Device Data
	int  idDevice;    // I/O Device Code

	void (*ResetIO)(void *);

	// I/O Instruction Calls
	void (*Function[IO_NFNC])(void *);
};


// kx10.c
void KL10_InitDevices(void);
void KX10_ResetAllDevices(void);
void KX10_Opcode_IO(void);
void kx10_Info(void);
void kx10_SetMap(P10_IOMAP *);
