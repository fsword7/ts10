// dz.h - DZ11/DZV11 Terminal communications
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

#include "dec/defs.h"
#include "emu/socket.h"

#define  DZ_KEY      "DZ11"      // Key Name for Unibus  (8 lines)
#define  DZV_KEY     "DZV11"     // Key Name for Q22-Bus (4 lines)

#define  DZ_NAME     "Terminal Communication"
#define  DZ_VERSION  "v0.1 (Pre-Alpha)"

// Unibus Address Index
#define DZ_CSR  (000 >> 1) // (R/W)  Control and Status   (CSR)
#define DZ_RBUF (002 >> 1) // (R)    Receiver Buffer      (RBUF)
#define DZ_LPR  (002 >> 1) // (W)    Line Parameter       (LPR)
#define DZ_TCR  (004 >> 1) // (R/W)  Transmit Control     (TCR)
#define DZ_MSR  (006 >> 1) // (R)    Modem Status         (MSR)
#define DZ_TDR  (006 >> 1) // (W)    Transmit Data        (TDR)

// DZCSR - Control and Status Register
#define CSR_TRDY        0100000 // (R)   Transmit Ready
#define CSR_TIE         0040000 // (R/W) Transmit Interrupt Enable
#define CSR_SA          0020000 // (R)   Silo Alarm
#define CSR_SAE         0010000 // (R/W) Silo Alarm Enable
#define CSR_TLINE       0003400 // (R)   Transmit Line
#define CSR_RDONE       0000200 // (R)   Receiver Done
#define CSR_RIE         0000100 // (R/W) Receiver Interrupt Enable
#define CSR_MSE         0000040 // (R/W) Master Scan Enable
#define CSR_CLR         0000020 // (R/W) Clear
#define CSR_MAINT       0000010 // (R/W) Maintenance
#define CSR_RW          0050170 // (W)   Write Mask

// DZRBUF - Receiver Buffer
#define RBUF_DATA_VALID 0100000 // (R)   Data Valid
#define RBUF_OVRN       0040000 // (R)   Overrun
#define RBUF_FRAM_ERR   0020000 // (R)   Framing Error
#define RBUF_PAR_ERR    0010000 // (R)   Parity Error
#define RBUF_RX_LINE    0003400 // (R)   Line Number
#define RBUF_RBUF       0000377 // (R)   Received Character
#define RBUF_RDMASK     0173777 // (R)   Read Mask

// DZLPR - Line Parameter Register
#define LPR_RX_ON       0010000 // (W)   Receiver On
#define LPR_FREQ        0007400 // (W)   Speed Select
#define LPR_ODD_PAR     0000200 // (W)   Odd Parity
#define LPR_PAR_EN      0000100 // (W)   Parity Enable
#define LPR_STOP_CODE   0000040 // (W)   Stop Code
#define LPR_CHAR_LEN    0000030 // (W)   Character Length
#define LPR_LINE        0000007 // (W)   Line Number
#define LPR_WRMASK      0017777 // (W)   Write Mask

// DZTCR - Transmit Control Register
#define TCR_LEN  0    // Position of Line Enable
#define TCR_DTR  8    // Position of Datat Terminal Ready

// DZMSR - Modem Status Register
#define MSR_RI   0    // Position of Ring Indicator
#define MSR_CD   8    // Position of Carrier Detect

// DZTDR - Transmit Data Register
#define TDR_TBUF 0377 // Transmit Buffer Data
#define TDR_BRK  8    // Position of Breaks

//  Baud Rate Selection Chart
// Bits: 11 10 09 08  Baud Rate
//        0  0  0  0      50
//        0  0  0  1      75
//        0  0  1  0     110
//        0  0  1  1     134.5
//        0  1  0  0     150
//        0  1  0  1     300
//        0  1  1  0     600
//        0  1  1  1    1200
//        1  0  0  0    1800
//        1  0  0  1    2000
//        1  0  1  0    2400
//        1  0  1  1    3600
//        1  1  0  0    4800
//        1  1  0  1    7200
//        1  1  1  0    9600
//        1  1  1  1   Not used

#define DZ_B50    0
#define DZ_B75    1
#define DZ_B110   2
#define DZ_B134   3
#define DZ_B150   4
#define DZ_B300   5
#define DZ_B600   6
#define DZ_B1200  7
#define DZ_B1800  8
#define DZ_B2000  9
#define DZ_B2400 10
#define DZ_B3600 11
#define DZ_B4800 12
#define DZ_B7200 13
#define DZ_B9600 14

// Character Length Chart
//
//   Character   Bits
//    Length    04  03
//    ------    ------
//    5 bits     0   0
//    6 bits     0   1
//    7 bits     1   0
//    8 bits     1   1

#define DZ_5BIT  0
#define DZ_6BIT  1
#define DZ_7BIT  2
#define DZ_8BIT  3

#define DZ_BUFLEN 256

// **************************************************************

#define CFLG_DZV11  0x00000002
#define CFLG_DZ11   0x00000001

#define TTY_EXIST   0x80000000 // Existant Flag
#define TTY_RXON    0x00000001 // Receive Enable

#define MUXREG(reg) (mux->reg)
#define CSR   MUXREG(csr)
#define RBUF  MUXREG(rbuf)
#define LPR   MUXREG(lpr)
#define TCR   MUXREG(tcr)
#define MSR   MUXREG(msr)
#define TDR   MUXREG(tdr)

// Line Position Field
#define LINE(pos) (1u << (tty->idPort + pos))

#define RX 0  // Recevice Interrupt Index
#define TX 1  // Transmit Interrupt Index

// DZ11 Controller Definitions
#define DZ_NREGS  4  // Number of DZ11 Registers
#define DZ_MUXES  4  // Maximum # of Muxes
#define DZ_PORTS  8  // Lines per Mux for DZ11
#define DZV_PORTS 4  // Lines per Mux for DZV11

// Total TTY Lines
#define DZ_LINES   (DZ_MUXES * DZ_PORTS)
#define DZ_MUXMASK 07

// Default Settings
#define DZ_CSRADR   0760100
#define DZ_NREGS    4
#define DZ_NVECS    2
#define DZ_IPL      UQ_BR5
#define DZ_RXVEC    0310
#define DZ_TXVEC    0314

// TTY Definitions
#define TTY_PORT   5020 // Default TTY Port
#define TTY_ESCAPE 0x1C // ASCII Control-Backslash
#define TTY_RDELAY 200  // Default Countdown - Receive
#define TTY_TDELAY 200  // Default Countdown - Transmit
#define TTY_QDELAY 200  // Default Countdown - Queue
#define TTY_BUFFER 8192 // Stream buffer in bytes

typedef struct dz_Device DZ_DEVICE;
typedef struct dz_Mux    DZ_MUX;
typedef struct dz_Line   DZ_TTY;

struct dz_Line {
	UNIT      Unit;    // Unit Header Information

	DZ_DEVICE *Device; // DZ11 Controller (Parent)
	DZ_MUX    *Mux;    // DZ11 Mux Data

	int       Flags;   // Line flags
	int       idMux;   // Mux Identification
	int       idPort;  // Port Identification
	int       idLine;  // Line Identification

	CLK_QUEUE txTimer;
	CLK_QUEUE qTimer;

	// Sockets
	SOCKET  *Server; // TTY Listening Socket
	SOCKET  *Socket; // TTY Client Socket

	// TTY Buffer
	uchar     inBuffer[TTY_BUFFER];
	char      outBuffer[TTY_BUFFER];
	int       idxInQueue, idxOutQueue;
	int       idxOutBuffer;
	char      lastSeen;
};

// DZ11 Mux Data
struct dz_Mux {
	DZ_DEVICE *Device; // DZ11 Controller (Parent)
	uint32    idMux;   // Mux Identification
	MAP_IO    ioMap;

	// Unibus DZ11 registers
	uint16    csr;  // Control and Status Register
	uint16    rbuf; // Receiver Buffer Register
	uint16    lpr;  // Line Parameter Register
	uint16    tcr;  // Transmit Control Register
	uint16    msr;  // Modem Status Register
	uint16    tdr;  // Transmit Data Register

	// Local TTY lines
	uint32    nLines;          // Number of existing lines
	DZ_TTY    Lines[DZ_PORTS]; // Local TTY lines
};

// DZ11 Controller Data
struct dz_Device {
	UNIT   Unit;      // Unit Header Information

	void    *Device;
	void    *System;
	UQ_CALL *Callback;

	uint32 Flags;     // Controller Flags
	int18  csrAddr;   // CSR Base Address
	int32  nRegs;     // Number of Registers
	int32  IntIPL;    // Interrupt Priority Level
	int32  IntVec;    // Interrupt Vector

	SOCKET  *Server;  // Listening Socket for DZ terminal controller.
	int     maxConns; // Maximum number of opened connections.
	int     nConns;   // Current number of opened connections.

	int     nMuxes;   // Number of Existing Muxes.
	int     nPorts;   // Number of Existing Ports per Mux.
	int     nLines;   // Number of Existing Lines.

	DZ_MUX  Muxes[DZ_MUXES];  // Number of Muxes
	DZ_TTY  *Lines[DZ_LINES]; // Number of Total Lines
};
