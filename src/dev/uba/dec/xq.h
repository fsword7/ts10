// xq.h - DEQNA/DELQA Ethernet Emulation Definitions
//
// Copyright (c) 2002-2003, Timothy M. Stark
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
// --------------------------------------------------------------------
//
// Modification History:
//
// 03/07/03  TMS  Merged xq.h and epp.h together.
// 03/07/03  TMS  Renamed qna.h to xq.h for PDP-11 style device names.
// 01/27/03  TMS  Removed all FIFO queues for new circular queues
//                for better performance.
// 01/21/03  TMS  Added DELQA-PLUS (Turbo).
// 01/13/03  TMS  Re-implemented the new circular packet queue management.
// 01/13/03  TMS  Removed all DESQA settings because I learned that DEQNA
//                and DELQA are compatible but not DESQA.
// 01/13/03  TMS  Modification History Starts Here.
//
// --------------------------------------------------------------------

#include "emu/defs.h"
#include "emu/socket.h"
#include "emu/ether.h"
#include "dec/defs.h"

#define QNA_KEY     "DEQNA"
#define LQA_KEY     "DELQA"
#define LQAT_KEY    "DELQA-PLUS"
#define QNA_NAME    "Qbus Ethernet Interface"
#define QNA_VERSION "v0.7 (Alpha)"

// DEQNA Register Map (I/O Base Address - 774440 or 774460)
//
// 774440 Station Address PROM (Read Only) (6 Words)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |        Reserved       |    Station Address    |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// 774440 Receive BDL Start Address (Write Only) (2 Words)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                Address (Low)                  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |            Reserved         | Address (High)  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// 774444 Transmit BDL Start Address (Write Only) (2 Words)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                Address (Low)                  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |            Reserved         | Address (High)  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// 774454 Vector Address Register (Read/Write)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |MS|OS|RS|S3|S2|S1|   Vector Address      |00|ID|
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// 774456 Control/Status Register (Read/Write)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |RI|PE|CA|OK|00|SE|EL|IL|XI|IE|RL|XL|BD|NI|SR|RE|
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
//
// DEQNA Registers for Read Access
//
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |        Reserved       |   Station Address #1  | Station Address PROM
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |        Reserved       |   Station Address #2  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |        Reserved       |   Station Address #3  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |        Reserved       |   Station Address #4  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |        Reserved       |   Station Address #5  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |        Reserved       |   Station Address #6  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |MS|OS|RS|S3|S2|S1|   Vector Address      |00|ID|
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |RI|00|CA|OK|00|SE|EL|IL|XI|IE|RL|XL|BD|NI|SR|RE| Control/Status Register
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
//
// DEQNA Registers for Write Access
//
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                   Reserved                    |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |                   Reserved                    |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |                Address (Low)                  | Receive BDL Address
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |            Reserved         | Address (High)  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |                Address (Low)                  | Transmit BDL Address
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |            Reserved         | Address (High)  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |MS|OS|RS|S3|S2|S1|   Vector Address      |00|ID|
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |RI|PE|CA|OK|00|SE|EL|IL|XI|IE|RL|XL|BD|NI|SR|RE| Control/Status Register
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
//
// Buffer Descriptor List
//
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//             :                       :
// |                  Status Word #2               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+ ---
// |                     Flag                      |  ^
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+  |
// |   Address Desc Bits         |  Address (High) |  |  One Buffer
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+  |  Descriptor List
// |                  Address (Low)                |  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+  |  ---
// |                  Buffer Length                |  |   ^
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+  |   |
// |                  Status Word #1               |  |   |  Omit if Chain
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+  |   |  Address
// |                  Status Word #2               |  v   v
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+ --- ---
// |                     Flag                      |
//             :                       :
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00


// Controller Flags
#define CFLG_ALLMULTI  0x00000010  // All Multicast Mode
#define CFLG_PROMISC   0x00000008  // Promiscuous Mode
#define CFLG_TURBO     0x00000004  // DELQA-PLUS Mode
#define CFLG_LQA       0x00000002  // DELQA Mode
#define CFLG_QNA       0x00000001  // DEQNA Mode

// CSR Base Address
#define QNA_IOADDR 0774440 // Default CSR Address
#define QNA_NREGS  8       // Number of Registers
#define QNA_NVECS  1       // Number of Vectors
#define QNA_IPL    UQ_BR5  // Default Interrupt Level

#define QNA_MAXBUF 2048
#define QNA_OK     0
#define QNA_NXM    1
#define QNA_ERR    2

// Delay timer
#define QNA_DELAY  500

// Register List (Index from CSR Base in 16-bit Words)
#define nPROM0   0 // Station Address PROM #0
#define nPROM1   1 // Station Address PROM #1
#define nPROM2   2 // Station Address PROM #2
#define nPROM3   3 // Station Address PROM #3
#define nPROM4   4 // Station Address PROM #4
#define nPROM5   5 // Station Address PROM #5
#define nRBADR0  2 // Receive BDL Start Address #0 (Low)
#define nRBADR1  3 // Receive BDL Start Address #1 (High)
#define nTBADR0  4 // Transmit BDL Start Address #0 (Low)
#define nTBADR1  5 // Transmit BDL Start Address #1 (High)
#define nVAR     6 // Vector Address Register
#define nCSR     7 // Control/Status Register

#define WMASK   0xFFFF // Word Mask
#define WSIZE   2      // Word Size

// CSR - Control and Status Register
#define CSR_RI  0100000 // (R/C) Receive Interrupt Request
#define CSR_PE  0040000 // (R)   Parity Error in Memory (DELQA only)
#define CSR_CA  0020000 // (R)   Carrier
#define CSR_OK  0010000 // (R)   Fuse OK
#define CSR_SE  0002000 // (R/W) Sanity Timer Enable
#define CSR_EL  0001000 // (R/W) External Loopback
#define CSR_IL  0000400 // (R/W) Internal Loopback
#define CSR_XI  0000200 // (R/C) Transmit Interrupt Request
#define CSR_IE  0000100 // (R/W) Interrupt Enable
#define CSR_RL  0000040 // (R)   Receive List Invalid
#define CSR_XL  0000020 // (R)   Transmit List Invalid
#define CSR_BD  0000010 // (R/W) Boot/Diagnostic ROM (PDP-11 Only)
#define CSR_NI  0000004 // (R)   Non-existent Memory Interrupt
#define CSR_SR  0000002 // (R/W) Software Reset
#define CSR_RE  0000001 // (R/W) Receiver Enable

#define CSR_RW  (CSR_SE|CSR_EL|CSR_IL|CSR_IE|CSR_BD|CSR_SR|CSR_RE)
#define CSR_W1C (CSR_RI|CSR_XI)

#define PROM_MBO   0xFF00 // Must be ones (high-order byte)

// Vector Address Register
#define VAR_MS  0100000 // (R/W) Mode Select       (DELQA only)
#define VAR_OS  0040000 // (R)   Option Switch     (DELQA only)
#define VAR_RS  0020000 // (R/W) Request Self-Test (DELQA only)
#define VAR_ST  0016000 // (R)   Self-Test Status  (DELQA only)
#define VAR_IV  0001774 // (R/W) Interrupt Vector Address
#define VAR_ID  0000001 // (R/W) Identity Test Bit (DELQA only)

#define VAR_QNA_RW  (VAR_IV)
#define VAR_LQA_LRW (VAR_MS|VAR_IV|VAR_ID)
#define VAR_LQA_QRW (VAR_IV|VAR_ID)
#define LQA_MODE    VAR_MS  // 0 = QNA mode, 1 = LQA mode

// Timeout Peroid (Sanity Timeout) in milliseconds
#define TIMEOUT0     250 // 0 (000)  1/4 second
#define TIMEOUT1    1000 // 1 (001)   1  second
#define TIMEOUT2    4000 // 2 (010)   4  seconds
#define TIMEOUT3   16000 // 3 (011)  16  seconds
#define TIMEOUT4   60000 // 4 (100)   1  minute
#define TIMEOUT5  240000 // 5 (101)   4  minutes
#define TIMEOUT6  960000 // 6 (110)  16  minutes
#define TIMEOUT7 3840000 // 7 (111)  64  minutes

// Setup Length Bits
#define SETUP_PROMISC    2  // Promiscuous Mode Bit
#define SETUP_ALLMULTI   1  // All Multicast Mode Bit

// Buffer Descriptor List
#define BDL_SIZE    12       // BDL Descriptor Length
#define BDL_STATUS1 8        // Offset to Status Word 1.
#define BDL_NEXT    BDL_SIZE // Next BDL Descriptor (offset)
#define BDL_USED    0040000  // BDL Flag - Used

// Address Descriptor Bits
#define BDL_VALID   0100000 // (R/T) Valid Bit
#define BDL_CHAIN   0040000 // (R/T) Address Chain
#define BDL_EOM     0020000 // (T)   End of Message
#define BDL_SETUP   0010000 // (T)   Set-Up
#define BDL_LBYTE   0000200 // (T)   Low Byte Only Termination
#define BDL_HBYTE   0000100 // (T)   High Byte Only Start

// Transmit Status Word #1 Bits
#define TSW_LASTNOT 0100000 // 
#define TSW_ERROR   0040000 // Error
#define TSW_USED    0040000 // Used Buffer
#define TSW_LOSS    0010000 // Loss of Carrier
#define TSW_NOCAR   0004000 // Mo Carrier
#define TSW_STE16   0002000 // 4-Minute Sanity Timer
#define TSW_ABORT   0001000 // Excessive Collisions (Abort)
#define TSW_FAIL    0000400 // Heartbeat Failure    (Fail)
#define TSW_COUNT   0000360 // Collision Count

// Transmit Status Word #2 Bits
#define TSW_TDR     0377777 // Time Domain Reflectometer

// Receive Status Words
#define RSW_LASTNOT 0100000 // 
#define RSW_ERROR   0040000 // Error
#define RSW_USED    0040000 // Used Buffer
#define RSW_ESETUP  0020000 // Loopback Setup
#define RSW_DISCARD 0010000 // 
#define RSW_RUNT    0004000 // Remains of a packet
#define RSW_RBLH    0003400 // Received Byte Length (High)
#define RSW_RBLL    0000377 // Received Byte Length (Low) (RSW #2)
#define RSW_SHORT   0000010 // Short Frame Error
#define RSW_FRAME   0000004 // Framing Alignment Error
#define RSW_CRCERR  0000002 // CRC Error Detection
#define RSW_OVF     0000001 // Ethernet Protocol Processor (EPP) Overflow

// FIFO Queue
#define FIFO_SIZE    8192    // 8K FIFO buffer
#define QNA_NPKTS    512     // Hold up to 512 ethernet packets

// Ethernet packet type for the circular queue.
#define PKT_INVALID  0  // Invalid Packet
#define PKT_NORMAL   1  // Normal Packet (Incoming)
#define PKT_LOOPBACK 2  // Loopback Packet
#define PKT_SETUP    3  // Setup Packet

// OpenTUN function - result code
#define EPP_TUN_IOERROR -1
#define EPP_TUN_INVALID -2

#define forever for(;;)

typedef struct qna_Buffer QNA_BDL;
typedef struct qna_Packet QNA_PACKET;
typedef struct qna_Device QNA_DEVICE;

// EPP Commands
enum {
	EPP_INIT = 1, // Initialization (First time)
	EPP_SETUP,    // Setup Frame
	EPP_TRANSMIT, // Transmit Ethernet Frame
	EPP_ILOOP,    // Internal Loopback
	EPP_ELOOP,    // External Loopback
	EPP_EILOOP,   // Extended Internal Loopback
	EPP_XSTATUS,  // Transmit Status Results
	EPP_RECEIVE,  // Receive Ethernet Frame
	EPP_RSTATUS,  // Receive Status Results
};

// BDL - Buffer Descriptor List
struct qna_Buffer {
	uint16  Flag;      // Flagword (Reserved)
	uint16  hiAddr;    // Descriptor Flags & Address (High)
	uint16  loAddr;    // Address (Low)
	int16   szBuffer;  // Buffer Length
	uint16  Status1;   // Status Word #1
	uint16  Status2;   // Status Word #2
};

struct qna_Packet {
	uint32     idPacket;
	uint32     Type;
	uint16     Status[2];
	ETH_PACKET Data;
};

struct qna_Device {
	// Unit Header Information
	UNIT      Unit;

	// Device Mapping Table
	void      *System;
	void      *Device;
	UQ_CALL   *Callback;

	// Unibus/Qbus I/O Table
	MAP_IO    ioMap;

//	DP        dp;          // Device Process Area
	SOCKET    *World;      // Gateway to the world
	uint32    Flags;       // Controller Flags
	uint32    csrAddr;     // CSR Base Address
	uint16    intVector;   // Interrupt Vector Address
	int       LED;         // LED Display
	CLK_QUEUE rxTimer;     // Receive Delay Timer
	CLK_QUEUE txTimer;     // Transmit Delay Timer

	// DEQNA Registers
	uint8     tunAddr[8];      // TUN/TAP Ethernet Address
	uint8     ownAddr[8];      // PROM Ethernet Address w/Checksum
	uint32    nAddrs;          // # of Ethernet Addresses
	uint8     ethAddr[14][8];  // Ethernet Addresses
	uint32    rxBDLAddr;       // Receive BDL Address
	uint32    txBDLAddr;       // Transmit BDL Address
	uint16    var;             // Vector Address Register
	uint16    csr;             // Control/Status Register

	// Circular Packet Queue Management
	int        pktHead;   // Current Queue Head
	int        pktTail;   // Current Queue Tail
	int        pktCount;  // Packet Queue Count
	int        pktLoss;   // Packet Loss
	QNA_PACKET pktList[QNA_NPKTS];
};
