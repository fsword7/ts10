// tq.h - TQ Series - TMSCP Tape Controller/Drive
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
#include "dec/uqssp.h" // Unibus/Qbus Storage Systems Port
#include "dec/mscp.h"  // Mass Storage Control Protocol
#include "emu/vtape.h"     // Virtual Tape Support Routines.

#define TQ_KEY     "TQ"
#define TQ_NAME    "TMSCP Tape Subsystem"
#define TQ_VERSION "v0.0 (Pre-Alpha)"

#define TQ_CLASS    CLS_CNTL   // Controller Class
#define TQ_DHTMO    0          // Default Host Timeout
#define TQ_DCTMO    120        // Default Controller Timeout
#define TQ_CSRADR   0774500    // TMSCP CSR Address
#define TQ_NREGS    2          // Number of Registers
#define TQ_NVECS    1          // Number of Vectors per Device.
#define TQ_IPL      UQ_BR5     // Interrupt Level BR5
#define TQ_NDRVS    4          // Number of Drives
#define TQ_XTIMER   500        // Data Transfers Timer
#define TQ_QTIMER   200        // Queue Timer for MSCP Commands
#define TQ_BLKSZ    512        // Block Size - 512 bytes
#define TQ_MAXFR    (1u << 16) // Maximum Data Transfer

#define TQ_OK       1          // For DTE and HBE packets only
#define TQ_ERROR    0

// Interrupt Requests
#define TQ_IE     (S1H & SA_S1H_IE)
#define TQ_VEC    (tq->intVector)
#define TQ_DOINTI if (TQ_IE && TQ_VEC) io->SendInterrupt(io, 0)
#define TQ_DOINT  if (TQ_VEC)          io->SendInterrupt(io, 0)

// Controller Flags
#define CFLG_RESET  0x8000 // Reset State
#define CFLG_PI     0x0004 // Purge Interrupt Request 
#define CFLG_IE     0x0002 // Interrupt Enable 
#define CFLG_POLL   0x0001 // Poll Request

// Drive Flags
#define DF_EXIST 0x8000 // Existant Drive
#define DF_ATT   0x4000 // Attached Medium
#define DF_WLK   0x2000 // Write Lock
#define DF_SXC   0x0010 // Serious Exception
#define DF_POL   0x0008 // Position Lost
#define DF_TMK   0x0004 // Tape Mark
#define DF_ATP   0x0002 // Attention Pending
#define DF_ONL   0x0001 // Online

#define TQ_NPKTS     32  // Number of Packets
#define TQ_PKT_WSIZE 32  // Packet Size in 8/16-bit Words
#define TQ_PKT_BSIZE (TQ_PKT_WSIZE << 1)
#define TQ_QHEAD     1   // Queue Head
#define TQ_QTAIL     0   // Queue Tail

typedef struct tq_Controller TQ_DEVICE;
typedef struct tq_Drive      TQ_DRIVE;
typedef struct tq_DriveType  TQ_DTYPE;
typedef struct tq_Packet     TQ_PACKET;

// Internal Packet Management
struct tq_Packet {
	TQ_PACKET *Next;              // Linked-List (NULL = End of packet)
	uint32    idPacket;           // Packet ID
	uint16    Data[TQ_PKT_WSIZE]; // Message Packet
};

struct tq_DriveType {
	char   *keyName;   // Drive Type Name
	char   *devDesc;   // Description

	// Tape Characteristic Information
	uint32 idMedia;    // Media ID
	uint32 uFlags;     // Unit Flags
	uint32 Format;     // Format Menu
	uint32 capSize;    // Capacity Size
	uint32 idPort;     // Unibus/Qbus Port ID
	uint32 idCtlr;     // Controller ID
	uint32 idUnit;     // Unit ID
	uint32 cVersion;   // Controller Version
	uint32 fVersion;   // Formatter Version
	uint32 uVersion;   // Unit Version
};

struct tq_Drive {
	UNIT      Unit;        // Unit Header Information

	char      *fileName;   // File Name
	VMT_TAPE  dpTape;      // Virtual Tape Descriptor
	uint32    cntObject;   // Object Count
	int32     Flags;       // Drive Flags
	int32     idUnit;      // Unit Identification
	int32     idMedia;     // Media Identification
	TQ_DEVICE *tqDevice;   // Parent Unit (Controller Device)
	TQ_DTYPE  *dtInfo;     // Drive Type
	uint16    uFlags;      // MSCP Unit Flags
	TQ_PACKET *pktWork;    // Working (Current) Packet
	TQ_PACKET *pktQueue;   // Packet Queue
	CLK_QUEUE xfrTimer;    // Data Transfer Timer
};

struct tq_Controller {
	UNIT      Unit;        // Unit Header Information

	uint32    Flags;       // Internal Controller Flags
	uint32    State;       // Internal Controller State
	uint32    csrAddr;     // CSR Address
	uint32    intVector;   // Interrupt Vector Address

	TQ_DTYPE  *dtInfo;     // Controller Device Type
	uint32    idPort;      // UQ Port ID
	uint32    idCtlr;      // Controller ID
	uint32    cVersion;    // Controller Version

	uint16    ip;          // UQSSP Initialization and Poll Register
	uint16    sa;          // UQSSP Status, Address, and Purge Register
	uint16    iData[8];    // Initialization Data Information
	uint16    cFlags;      // Controller Flags
	uint16    Credits;     // Credits
	uint16    errCode;     // Error Code
	uint32    hstTimeout;  // Host Timeout
	uint32    hstTimer;    // Host Timer (Countdown)
	uint32    xfrTimer;    // Data Transfer Time
	CLK_QUEUE Timer;       // Timer
	CLK_QUEUE queTimer;    // Queue Timer for MSCP Commands

	// Communication Region Management
	uint32    rbAddr;      // Ringbase Address
	UQ_RING   cmdRing;     // Command Ring Table
	UQ_RING   resRing;     // Response Ring Table

	// Internal Packet Management
	uint32    pktBusy;     // Number of Busy Packets
	TQ_PACKET *pktResp;    // Response List
	TQ_PACKET *pktFree;    // Free List
	TQ_PACKET pktList[TQ_NPKTS]; // Packet List

	// Disk/Tape Device Units
	int       nDrives;     // Number of disk/tape drives
	TQ_DRIVE  *Drives;
	uint8     *bufData;    // Data Transfers

	// Unibus/Q22-Bus I/O Table
	MAP_IO    ioMap;

	// Device Mapping Table for MSCP Device Units
	void     *System;
	void     *Device;
	UQ_CALL  *Callback;
};

// UQSSP Initialization Definitions

#define TQ(p) tq->p
#define IP    TQ(ip)       // Initialization and Poll Register
#define SA    TQ(sa)       // Status, Address, and Purge Register

#define SX(x) TQ(iData[x])
#define S1C   SX(0)        // Init Step 1 - Controller to Host
#define S1H   SX(1)        // Init Step 1 - Host to Controller
#define S2C   SX(2)        // Init Step 2 - Controller to Host
#define S2H   SX(3)        // Init Step 2 - Host to Controller
#define S3C   SX(4)        // Init Step 3 - Controller to Host
#define S3H   SX(5)        // Init Step 3 - Host to Controller
#define S4C   SX(6)        // Init Step 4 - Controller to Host
#define S4H   SX(7)        // Init Step 4 - Host to Controller 
