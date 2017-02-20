// ts.h - TS11/TSV05 Tape Controller/Drive Defintions
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

#include "emu/vtape.h"
#include "dec/defs.h"

#define TS11_KEY    "TS11"
#define TSV05_KEY   "TSV05"
#define TS_NAME     "Magtape Subsystem Emulator"
#define TS_VERSION  "v0.7 (Alpha)"

// Default CSR/interrupt settings
#define TS_CSRADR 0772520 // Default CSR Address
#define TS_NREGS  2       // Number of Registers
#define TS_NVECS  1       // Number of Vectors per Device.
#define TS_IPL    UQ_BR5  // Default Interrupt Level BR5
#define TS_VEC    0224    // Default Vector

#define TS_DELAY  0       // Delay Timer (per instruction)

// TS11/TSV05 Internal Flags
#define TS_ATTACHED 0x80000000 // Tape Medium is attached.
#define TS_WLOCK    0x40000000 // Tape Medium is write-locked.
#define TS_BOOT     0x20000000 // Boot Request
#define TS_QATN     0x10000000 // Pending Attention
#define TS_OWNCMD   0x08000000 // Ownership of Command Packet
#define TS_OWNMSG   0x04000000 // Ownership of Message Packet
#define ID_TSV05    0x00000002 // Is a TSV05 device.
#define ID_TS11     0x00000001 // Is a TS11 device.

// TS11/TSV05 Unibus Registers
//
// 772520 TSBA  - (R) Bus Address Register
// 772520 TSDB  - (W) Data Buffer Register
// 772522 TSSR  - (R) Status Register           (Write to Reset)
// 772523 TSDBX - (W) Extended Address Register (TSV05 Only)

#define nTSBA  0
#define nTSDB  0
#define nTSSR  1
#define nTSBDX 1

#define TS11   ts
#define TSBA   TS11->tsba
#define TSDB   TS11->tsdb
#define TSSR   TS11->tssr
#define TSDBX  TS11->tsdbx

// TSSR - Status Register (Read-Only, Write to Reset)

#define TSSR_SC  0100000 // Special Condition
#define TSSR_RMR 0010000 // Register Modification Refused
#define TSSR_NXM 0004000 // Non-Existant Memory
#define TSSR_NBA 0002000 // Need Bus Address
#define TSSR_EMA 0001400 // Extended Memory Address
#define TSSR_SSR 0000200 // Subsystem Ready
#define TSSR_OFL 0000100 // Offline
#define TSSR_TC  0000016 // Terminal Class

#define TSSR_P_EMA 8
#define TSSR_MBZ   0060060 // Must Be Zeroes.

// TSDBX - Extended Data Buffer Register (TSV05 only)
#define TSDBX_BOOT 0200
#define TSDBX_MASK 017

// Terminal Class
#define TC0 (0 << 1) // Normal Termination
#define TC1 (1 << 1) // Attention Condition
#define TC2 (2 << 1) // Tape Status Alert
#define TC3 (3 << 1) // Function Reject
#define TC4 (4 << 1) // Recoverable Error, Tape Moved
#define TC5 (5 << 1) // Recoverable Error
#define TC6 (6 << 1) // Unrecoverable Error, Position Lost
#define TC7 (7 << 1) // Fatal Controller Error

// Command Packet (Offsets)
//
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// |ACK|CVC|OPP|SWP|     Mode      |  Format   |     Command       |
// +---^---+---+---^---+---+---^---+---+---^---+---+---^---+---+---+
// | <---------------- Low Order Buffer Address -----------------> |
// +---^---+---+---^---+---+---^---+---+---^---+---+---^---+---+---+
// | <--------------- High Order Buffer Address -----------------> |
// +---^---+---+---^---+---+---^---+---+---^---+---+---^---+---+---+
// | <--------------- Buffer Extent (Byte Count) ----------------> |
// +---^---+---+---^---+---+---^---+---+---^---+---+---^---+---+---+
//  15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0

#define CMD_LEN  (4 << 1)  // Length in bytes
#define cmdPkt   (uint8 *)&TS11->pktCmd[0]
#define cmdHdr   TS11->pktCmd[0]
#define cmdLAddr TS11->pktCmd[1]
#define cmdHAddr TS11->pktCmd[2]
#define cmdCount TS11->pktCmd[3]

#define CMD_ACK  0100000 // Acknowledge
#define CMD_CVC  0040000 // Clear Volume Check
#define CMD_OPP  0020000 // Opposite
#define CMD_SWP  0010000 // Swap Bytes
#define CMD_MODE 0007400 // Mode
#define CMD_FMT  0000340 // Format
#define CMD_IE   0000200 // Interrupt Enable
#define CMD_FNC  0000037 // Command Code

#define CMD_M_MODE 017
#define CMD_P_MODE 8

#define GET_FNC(x)  ((x) & CMD_FNC)
#define GET_MODE(x) (((x) >> CMD_P_MODE) & CMD_M_MODE)

// Function Codes
#define FNC_READ   0001 // Read
#define FNC_WCHR   0004 // Write Char
#define FNC_WRITE  0005 // Write 
#define FNC_WSSM   0006 // Write Memory
#define FNC_POS    0010 // Position
#define FNC_FMT    0011 // Format
#define FNC_CTL    0012 // Control
#define FNC_INIT   0013 // Initialize
#define FNC_GSTA   0017 // Get Status

// Message Packet (Offsets)

#define MSG_LEN  (8 << 1) // Length in bytes
#define msgPkt   (uint8 *)&TS11->pktMsg[0]
#define msgHdr   TS11->pktMsg[0] // Header
#define msgLen   TS11->pktMsg[1] // Length
#define msgFrame TS11->pktMsg[2] // Residual Frame
#define msgXSt0  TS11->pktMsg[3] // Extended Status 0
#define msgXSt1  TS11->pktMsg[4] // Extended Status 1
#define msgXSt2  TS11->pktMsg[5] // Extended Status 2
#define msgXSt3  TS11->pktMsg[6] // Extended Status 3
#define msgXSt4  TS11->pktMsg[7] // Extended Status 4

// Message Header
#define MSG_ACK   0100000 // Acknowledge
#define MSG_MATN  0000000 // Attention
#define MSG_MILL  0000400 // Illegal
#define MSG_MNEF  0001000 // Non-executable Function
#define MSG_CEND  0000020 // End
#define MSG_CFAIL 0000021 // Fail
#define MSG_CERR  0000022 // Error
#define MSG_CATN  0000023 // Attention

// Extended Status Register #0
#define XS0_TMK  0100000 // Tape Mark
#define XS0_RLS  0040000 // Record Length Short
#define XS0_LET  0020000 // Long End Tape
#define XS0_RLL  0010000 // Record Length Long
#define XS0_WLE  0004000 // Write Lock Error
#define XS0_NEF  0002000 // Non-executable Function
#define XS0_ILC  0001000 // Illegal Command
#define XS0_ILA  0000400 // Illegal Address
#define XS0_MOT  0000200 // Tape has moved
#define XS0_ONL  0000100 // Online
#define XS0_IE   0000040 // Interrupt Enable
#define XS0_VCK  0000020 // Volume Check
#define XS0_PET  0000010 // 1600 bpi Density
#define XS0_WLK  0000004 // Write Lock
#define XS0_BOT  0000002 // Beginning of Tape
#define XS0_EOT  0000001 // End of Tape
#define XS0_CLR  0177600 // Clear All to Start Transfers

// Extended Status Register #1
#define XS1_UCOR 0000002 // Uncorrectable Error

// Extended Status Register #2
#define XS2_XTF  0000200 // Extended Features

// Extended Status Register #3
#define XS3_OPI  0000100 // Incomplete Operation
#define XS3_REV  0000040 // Reverse
#define XS3_RIB  0000001 // Reverse to BOT

// Extended Status Register #4
#define XS4_HDS  0100000 // High Density

// Write Characteristics Packet
#define WCH_LEN   (5 << 1)
#define wchPkt    (uint8 *)&TS11->pktChar[0]
#define wchLAddr  TS11->pktChar[0]
#define wchHAddr  TS11->pktChar[1]
#define wchLen    TS11->pktChar[2]
#define wchOpts   TS11->pktChar[3]
#define wchXOpts  TS11->pktChar[4]

// Write Characteristics Options
#define WCH_ESS  0000200 // Stop Double Tape Marks
#define WCH_ENB  0000100 // BOT = Tape Mark
#define WCH_EAI  0000040 // Enable Attention Interrupt
#define WCH_ERI  0000020 // Enable MRLS Interrupt

// Write Characteristics Extended Options
#define WCHX_HDS 0000040 // High Density

// Return results of tape operation
#define XTC_NORMAL  0
#define XTC(xs, tc) (((xs) << 16) | (tc))

#define GET_XS(xtc)      ((uint16)((xtc) >> 16))
#define GET_SR(xtc)      ((uint16)((xtc) & ~TSSR_TC))
#define GET_TC(xtc)      ((uint16)((xtc) & TSSR_TC))
#define MAX_TC(tc0, tc1) (((tc0) < (tc1)) ? (tc1) : (tc0))

typedef struct ts_Device TS_DEVICE;
struct ts_Device {
	UNIT    Unit;      // Unit Header Information
	void    *System;
	void    *Device;
	UQ_CALL *Callback;
	MAP_IO  ioMap;     // I/O Mapping

	// Internal Flags/Settings
	uint32    Flags;    // Internal Flags
	uint32    csrAddr;  // CSR Address
	CLK_QUEUE svcTimer; // Service Timer

	// TS11/TSV05 Unibus Registers
	uint32 tsba;    // Bus Address Register (22-bit addressing)
	uint16 tsdb;    // Data Buffer Register
	uint16 tssr;    // Status Register
	uint16 tsdbx;   // Extended Data Buffer Register

	// Command, Message, and Characteristics Packets
	uint16 pktCmd[CMD_LEN];   // Command Packet
	uint16 pktMsg[MSG_LEN];   // Message Packet
	uint16 pktChar[WCH_LEN];  // Write Characteristics Packet

	// Magtape Drive
	uint32   dpFlags;
	VMT_TAPE dpTape;
};

