// dl.h - DL11 Serial Line Unit Emulation
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
#include "emu/socket.h"
#include "dec/defs.h"

#define DL_KEY     "DL11"
#define DL_NAME    "Serial Line Emulator"
#define DL_VERSION "v0.8 (Alpha)"

// TTY Settings
#define TTY_PORT      5000 // Default CTY port
#define TTY_ESCAPE    0x1C // ASCII Control-Backslash
//#define TTY_RDELAY    200  // Default countdown - Receive
//#define TTY_TDELAY    200  // Default countdown - Transmit
//#define TTY_QDELAY    50   // Default countdown - Queue
#define TTY_RDELAY    0    // Default countdown - Receive
#define TTY_TDELAY    0    // Default countdown - Transmit
#define TTY_QDELAY    50   // Default countdown - Queue
#define TTY_BUFFER    8192 // Stream buffer in bytes

// Default Unibus/Qbus Address
#define DL_CSRADR  0777560
#define DL_NREGS   4

// Default Interrupt/Vector Settings
#define DL_IPL   UQ_BR4 // Interrupt Level BR4
#define DL_NVECS 2      // Number of Vectors
#define DL_RXVEC 0060   // Receive Vector
#define DL_TXVEC 0064   // Transmit Vector

// Interrupt Vector Index
#define RX 0 // Recevie Interrupt
#define TX 1 // Transmit Interrupt

// Unibus/QBus I/O Addresses
//
// 777560 RCSR  Receive Control/Status Register
// 777562 RBUF  Receive Data Buffer
// 777564 XCSR  Transmit Control/Status Register
// 777566 XBUF  Transmit Data Buffer

#define nRCSR 0 // Receive Control/Status Register
#define nRBUF 1 // Receive Data Buffer
#define nXCSR 2 // Transmit Control/Status Register
#define nXBUF 3 // Transmit Data Buffer

#define REG(n) dl->Regs[n]
#define RCSR   REG(nRCSR)
#define RBUF   REG(nRBUF)
#define XCSR   REG(nXCSR)
#define XBUF   REG(nXBUF)

// RXCS Receive Control and Status Register

#define RCSR_DSC   0100000  // (R)   Data Status Change
#define RCSR_RI    0040000  // (R)   Ring Indicator
#define RCSR_CTS   0020000  // (R)   Clear to Send
#define RCSR_CD    0010000  // (R)   Carrier Detector
#define RCSR_ACT   0004000  // (R)   Receive Action
#define RCSR_SRD   0002000  // (R)   Secondary Received Data
#define RCSR_RDY   0000200  // (R)   Receive Ready (Done)
#define RCSR_IE    0000100  // (R/W) Receive Interrupt Enable
#define RCSR_DIE   0000040  // (R/W) Dataset Interrupt Enable
#define RCSR_STD   0000010  // (R/W) Secondary Transmit Data
#define RCSR_RTS   0000004  // (R/W) Request to Clear
#define RCSR_DTR   0000002  // (R/W) Data Terminal Ready
#define RCSR_RDE   0000001  // (W)   Reader Enable
#define RCSR_RW    (RCSR_IE)

// RBUF Receive Data Buffer Register

#define RBUF_ERR   0100000  // (R)   Error
#define RBUF_OR    0040000  // (R)   Overrun Error
#define RBUF_FER   0020000  // (R)   Framing Error
#define RBUF_PER   0010000  // (R)   Data Parity Error
#define RBUF_DATA  0000377  // (R)   Data Buffer

// XCSR Transmit Control and Status Register

#define XCSR_RDY   0000200  // (R)   Transmit Ready/Done
#define XCSR_IE    0000100  // (R/W) Transmit Interrupt Enable
#define XCSR_MAINT 0000004  // (R/W) Maintenance
#define XCSR_BRK   0000001  // (R/W) Transmit Break
#define XCSR_RW    (XCSR_IE|XCSR_MAINT|XCSR_BRK)

// XBUF Transmit Data Buffer Register

#define XBUF_CHAR  0000377  // (W)   Data 

// ***************************************************

typedef struct dl_Line DL_TTY;

struct dl_Line {
	UNIT    Unit;
	void    *Device;
	void    *System;
	UQ_CALL *Callback;
	MAP_IO  ioMap;

	uint32  Flags;          // Internal Flags
	uint32  csrAddr;        // CSR Address
	uint16  Regs[DL_NREGS]; // DL11 Registers

	// Socket slot
	SOCKET  *Server; // CTY Listening Socket
	SOCKET  *Socket; // CTY Client Socket

	// Receive/Transmit Timers
	CLK_QUEUE rxTimer; // Receive Timer
	CLK_QUEUE txTimer; // Transmit Timer
	CLK_QUEUE qTimer;  // Queue Timer

	// TTY Buffer
	int     mCount; // Maintenance Test
	int     idxInQueue, idxOutQueue;
	int     idxOutBuffer;
	char    lastSeen;
	uchar   inBuffer[TTY_BUFFER];
	char    outBuffer[TTY_BUFFER];
};
