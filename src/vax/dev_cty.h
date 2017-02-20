// dev_cty.h - Console Terminal Definitions
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

#include "dev/uba/dec/defs.h"
#include "emu/socket.h"

// Console TTY defintions (OPA0:)
#define CTY_PORT      5000 // Default CTY port
#define CTY_ESCAPE    0x1C // ASCII Control-Backslash
#define CTY_HALT      0x10 // ASCII Control-P
//#define CTY_RDELAY    200  // Default countdown - Receive
//#define CTY_TDELAY    200  // Default countdown - Transmit
//#define CTY_QDELAY    50   // Default countdown - Queue
#define CTY_RDELAY    0  // Default countdown - Receive
#define CTY_TDELAY    0  // Default countdown - Transmit
#define CTY_QDELAY    0   // Default countdown - Queue
#define CTY_BUFFER    65536 // Stream buffer in bytes

#define CTY_IPL       UQ_BR4     // Interrupt Level BR4
#define CTY_NVECS     2          // Number of Vectors (RX and TX)
#define CTY_RXVEC     SCB_CTYIN  // Input Interrupts
#define CTY_TXVEC     SCB_CTYOUT // Output Interrupts

#define RX 0
#define TX 1

// ********************************************************
// ********** IPR Registers for MicroVAX Series ***********
// ********************************************************

// RXCS - Console Receive Control and Status Register

#define RXCS_ACT   0x00000800 // (R)   Receive Active
#define RXCS_RDY   0x00000080 // (R)   Ready/Done
#define RXCS_IE    0x00000040 // (R/W) Interrupt Enable
#define RXCS_WMASK 0x00000040 // Write Mask

// RXDB - Console Receive Data Buffer Register

#define RXDB_ERR   0x00008000 // (R)   Error
#define RXDB_OVR   0x00004000 // (R)   Overrun Error
#define RXDB_FRM   0x00002000 // (R)   Framing Error
#define RXDB_BRK   0x00000800 // (R)   Receive Break
#define RXDB_CHAR  0x000000FF // (R)   Data 

// TXCS - Console Transmit Control and Status Register

#define TXCS_RDY   0x00000080 // (R)   Ready/Done
#define TXCS_IE    0x00000040 // (R/W) Interrupt Enable
#define TXCS_MAINT 0x00000004 // (R/W) Maintenance
#define TXCS_BRK   0x00000001 // (R/W) Transmit Break

#define TXCS_WMASK 0x00000045 // Write Mask

// TXDB - Console Transmit Data Buffer Register

#define TXDB_CHAR  0x000000FF // (W)   Data 

// ***************************************************

//typedef struct Console VAX_CONSOLE;

struct vax_Console {
	// Belong to its processor.
	VAX_CPU *Processor;

	// Unibus/QBus Interrupts
	MAP_IO ioMap;

	// Delayed I/O per a few instructions (50 to 100)
	// KA630 VMB rom complaint about instanteous I/O
	// access during test #3.
	CLK_QUEUE *rxTimer; // Receive Timer
	CLK_QUEUE *txTimer; // Transmit Timer
	CLK_QUEUE *qTimer;  // Queue Timer

	// Socket slot
	SOCKET  *Server; // CTY Listening Socket
	SOCKET  *Socket; // CTY Client Socket

	// Console TTY Buffer
	int     mCount; // Maintenance Test
	uchar   inBuffer[CTY_BUFFER];
	char    outBuffer[CTY_BUFFER];
	int     idxInQueue, idxOutQueue;
	int     idxOutBuffer;
	char    lastSeen;
};
