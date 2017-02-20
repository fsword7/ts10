// dec_dh.c - DH11 Terminal Emulation System
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

#include "dec/defs.h"
#include "emu/socket.h"

#define DHU_KEY   "DHU11"
#define DHV_KEY   "DHV11"
#define DHQ_KEY   "DHQ11"

#define DH_NAME    "Terminal Server"
#define DH_VERSION "v0.5 (Alpha)"

// * = Multiple Registers (per line)
#define DH_CSR      0 // (R/W) Control/Status Register
#define DH_RBUF     1 // (R)   Receive Buffer Register
#define DH_TXCHAR   1 // (W)   *Transmit Character
#define DH_LPR      2 // (R/W) *Line Parameter Register
#define DH_STAT     3 // (R)   *Line Status
#define DH_LNCTRL   4 // (R/W) *Line Control
#define DH_TBUFFAD1 5 // (R/W) *Transmit Buffer Address 1
#define DH_TBUFFAD2 6 // (R/W) *Transmit Buffer Address 2
#define DH_TBUFFCT  7 // (R/W) *Transmit Buffer Count

// Register Macros
#define MREG(idx) tty->regs[idx]
#define CSR       dh->csr
#define RBUF      dh->rbuf
#define TBUF      dh->tbuf
#define TXCH      MREG(DH_TXCHAR)
#define LPR       MREG(DH_LPR)
#define STAT      MREG(DH_STAT)
#define LNC       MREG(DH_LNCTRL)
#define TBAD1     MREG(DH_TBUFFAD1)
#define TBAD2     MREG(DH_TBUFFAD2)
#define TBCNT     MREG(DH_TBUFFCT)

// CSR - Control and Status Register  (Base)
#define CSR_TXACT     0100000 // (R)   Transmitter Action
#define CSR_TXIE      0040000 // (R/W) Transmit Interrupt Enable
#define CSR_DIAGFAIL  0020000 // (R)   Diagnostic Fail
#define CSR_TXERR     0010000 // (R)   Transmit DMA Error
#define CSR_TXLINE    0007400 // (R)   Transmit Line Number
#define CSR_RXAVAIL   0000200 // (R)   Received Data Available
#define CSR_RXIE      0000100 // (R/W) Received Interrupt Enable
#define CSR_RESET     0000040 // (R/W) Master Reset
#define CSR_LINE      0000017 // (R/W) Indirect Address Register

#define CSR_RW        (CSR_TXIE|CSR_RXIE|CSR_RESET|CSR_LINE)

// RBUF - Receive Buffer   (Base + 2)
#define RBUF_VALID    0100000 // (R)   Data Valid
#define RBUF_OVRRUN   0040000 // (R)   Overrun Error
#define RBUF_FRERR    0020000 // (R)   Framing Error
#define RBUF_PAERR    0010000 // (R)   Parity Error
#define RBUF_STATUS   0070000 // (R)   Status Flag
#define RBUF_RXLINE   0007400 // (R)   Receive Line Number
#define RBUF_RXCHAR   0000377 // (R)   Received Character

// TXCHAR - (M) Transmit Character Register (Base + 2)
#define TXCH_VALID    0100000 // (W)   Data Valid
#define TXCH_DATA     0000377 // (W)   Transmit Character (RXCHAR) 

#define TXCH_RW       0100377 // Writable Bits

// LPR - (M) Line Parameter Register (Base + 4)
#define LPR_TXSPEED   0170000 // (R/W) Transmitted Data Rate
#define LPR_RXSPEED   0007400 // (R/W) Received Data Rate
#define LPR_STOP      0000200 // (R/W) Stop Code
#define LPR_EVEN      0000100 // (R/W) Even Parity
#define LPR_PEN       0000040 // (R/W) Parity Enable
#define LPR_LEN       0000030 // (R/W) Character Length
#define LPR_DIAG      0000006 // (R/W) Diagnostic Code

#define LPR_RW        0177776 // Readable/Writable Bits

#define BDMASK 017
#define LNMASK 03

#define PUT_TXSP(baud) ((baud & BDMASK) << 12)
#define PUT_RXSP(baud) ((baud & BDMASK) << 8)
#define PUT_CHLEN(len) ((len & LNMASK) << 3)

// STAT - (M) Line Status Register (Base + 6)
#define STAT_DSR      0100000 // (R)   Data Set Ready
#define STAT_RI       0020000 // (R)   Ring Indicator
#define STAT_DCD      0010000 // (R)   Data Carrier Detected
#define STAT_CTS      0004000 // (R)   Clear to Send
#define STAT_TYPE     0000400 // (R)   DHU11 = 1, DHV11 = 0

// LNCTRL - (M) Line Control Register (Base + 10)
#define LNC_RTS       0010000 // (R/W) Request to Send
#define LNC_DTR       0001000 // (R/W) Data Terminal Ready
#define LNC_LINK      0000400 // (R/W) Link Type
#define LNC_MAINT     0000300 // (R/W) Maintenance Mode
#define LNC_FORCE     0000040 // (R/W) Force X-OFF
#define LNC_OAUTO     0000020 // (R/W) Outgoing Auto Flow
#define LNC_BREAK     0000010 // (R/W) Break Control
#define LNC_RXEN      0000004 // (R/W) Receiver Enable
#define LNC_IAUTO     0000002 // (R/W) Incoming Auto Flow
#define LNC_ABORT     0000001 // (R/W) Transmit DMA Abort

#define LNC_RW        0011777 // (R/W) Accessible Bits

// TBUFFAD1 - (M) Transmit Buffer Address Register #1 (Base + 12)
#define TBAD1_ADR     0177777 // (R/W) Transmit Buffer Address (Low)

// TBUFFAD2 - (M) Transmit Buffer Address Register #2 (Base + 14)
#define TBAD2_TXEN    0100000 // (R/W) Transmit Enable
#define TBAD2_START   0000200 // (R/W) Transmit DMA Start
#define TBAD2_ADDR    0000077 // (R/W) Transmit Buffer Address (High)

#define TBAD2_RW      0100277 // (R/W) Read/Write Bits

// TBUFFCT - (M) Transmit DMA Buffer Counter (Base + 16)
#define TBCNT_COUNT   0177777 // (R/W0 Transmit Character Count

// Baud Rate (Speed)
#define BAUD_50      000 //    50   Bits/Second  A
#define BAUD_75      001 //    75   Bits/Second    B
#define BAUD_110     002 //   110   Bits/Second  A B
#define BAUD_135     003 //   134.5 Bits/Second  A B
#define BAUD_150     004 //   150   Bits/Second    B
#define BAUD_300     005 //   300   Bits/Second  A B
#define BAUD_600     006 //   600   Bits/Second  A B
#define BAUD_1200    007 //  1200   Bits/Second  A B
#define BAUD_1800    010 //  1800   Bits/Second    B
#define BAUD_2000    011 //  2000   Bits/Second    B
#define BAUD_2400    012 //  2400   Bits/Second  A B
#define BAUD_4800    013 //  4800   Bits/Second  A B
#define BAUD_7200    014 //  7200   Bits/Second  A
#define BAUD_9600    015 //  9600   Bits/Second  A B
#define BAUD_19200   016 // 19200   Bits/Second    B
#define BAUD_38400   017 // 38400   Bits/Second  A

// Character Width
#define BIT_5 00 // 5 Bits
#define BIT_6 01 // 6 Bits
#define BIT_7 02 // 7 Bits
#define BIT_8 03 // 8 Bits

#define DH_CSRADDR  0760440  // CSR Base Address
#define DH_IPL      UQ_BR4   // Interrupt Level BR4
#define DH_NVECS    2        // Number of vectors per device.
#define DH_RXVEC    0320     // Receive Vector Interrupt
#define DH_TXVEC    0324     // Transmit Vector Interrupt
#define DH_NREGS    8        // Number of Registers
#define LEN_RBUF    256      // FIFO Receive Buffer Length
#define LEN_TBUF    256      // FIFO Transmit Buffer Length

#define DH_NPORTS   16       // Up to 16 communication ports

#define DHV_PORTS   8        // For DHV11,  8 comm ports.
#define DHU_PORTS   16       // For DHU11, 16 comm ports.
#define DHQ_PORTS   8        // For DHQ11,  8 comm ports.

#define RXINT 0
#define TXINT 1

// Controller Flags
#define CFLG_DHV 0x00000002
#define CFLG_DHU 0x00000001
#define CFLG_DHQ (CFLG_DHV|CFLG_DHU)
#define CFLG_DH  (CFLG_DHV|CFLG_DHU)

// Console TTY defintions
#define TTY_PORT      5010 // Default TTY port
#define TTY_ESCAPE    0x1C // ASCII Control-Backslash
#define TTY_RDELAY    200  // Default countdown - Receive
#define TTY_TXDELAY   200  // Default countdown - Transmit
#define TTY_TBDELAY   500  // Default countdown - Transmit DMA
#define TTY_QDELAY    200  // Default countdown - Queue
#define TTY_BUFFER    8192 // Stream buffer in bytes

typedef struct dh_Port   DH_TTY;
typedef struct dh_Device DH_DEVICE;

struct dh_Port {
	char   *devName;    // Device Name
	char   *keyName;    // Key (Device Type) Name
	char   *emuName;    // Emulator Name
	char   *emuVersion; // Emulator Version

	int       idPort;   // Line ID
	DH_DEVICE *Device;  // Whose MUX controller.
	CLK_QUEUE dmaTimer; // Transmit DMA Timer
	CLK_QUEUE txTimer;  // Transmit Timer
	CLK_QUEUE qTimer;   // Queue Timer

	// DHx-11 Indexed Registers
	uint16 regs[8];

	// Socket slot
	SOCKET  *Server; // CTY Listening Socket
	SOCKET  *Socket; // CTY Client Socket

	// TTY Buffer
	uchar   inBuffer[TTY_BUFFER];
	char    outBuffer[TTY_BUFFER];
	int     idxInQueue, idxOutQueue;
	int     idxOutBuffer;
	char    lastSeen;
};

struct dh_Device {
	UNIT   Unit; // Unit Header

//	char   *devName;    // Device Name
//	char   *keyName;    // Key (Device Type) Name
//	char   *emuName;    // Emulator Name
//	char   *emuVersion; // Emulator Version

	void    *Device;
	void    *System;
	UQ_CALL *Callback;

	// Controller Flags
	uint32  Flags;
	uint32  csrAddr;
	MAP_IO  ioMap;

	// TTY Line Handler
//	CLK_QUEUE *rxTimer; // Receive Timer
//	CLK_QUEUE *txTimer; // Transmit Timer
//	CLK_QUEUE *qTimer;  // Queue Timer

	SOCKET  *Server; // TTY Listening Socket

	// DHx-11 Registers
	uint16  csr;               // Control and Status Register
	uint16  idxIn, idxOut;     // Pointers for Receive Buffer.
	uint16  rbuf[LEN_RBUF];    // Receive Buffer Register
	uint16  idxTxIn, idxTxOut; // Pointers for Transmit Buffer.
	uint16  tbuf[LEN_TBUF];    // Receive Transmit Register

	// Communication Ports
	uint32  nPorts;           // Number of supported ports
	DH_TTY  Ports[DH_NPORTS]; // up to 16 communication ports
};
