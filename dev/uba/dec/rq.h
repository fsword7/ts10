// rq.h - RQ Series - MSCP Disk Controller/Drive
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
#include "dec/uqssp.h" // Unibus/QBus Storage Systems Port
#include "dec/mscp.h"  // Mass Storage Control Protocol

#define RQ_KEY     "RQDX3"
#define RQ_NAME    "MSCP Disk Controller"
#define RQ_VERSION "v0.8.1 (Alpha)"

#define RQ_CLASS    CLS_CNTL  // Controller Class
#define RQ_MODEL    19        // Adaptor Number (Model #)
#define RQ_HVER     1         // Hardware Version
#define RQ_SVER     3         // Software Version
#define RQ_DHTMO    60        // Default Host Timeout
#define RQ_DCTMO    120       // Default Controller Timeout
#define RQ_NREGS    2         // Number of Registers
#define RQ_IPL      UQ_BR5    // Interrupt Level BR5
#define RQ_NVECS    1         // Number of Vectors per Device.
#define RQ_NDRVS    4         // Number of Drives
#define RQ_XTIMER   500       // Data Transfers Timer
#define RQ_QTIMER   250       // Queue Timer for MSCP Commands
#define RQ_BLKSZ    512       // Block Size - 512 bytes
#define RQ_MAXFR    (1 << 16) // Maximum Data Transfer

#define RQ_OK       1         // For DTE and HBE packets
#define RQ_ERROR    0

// Interrupt Requests
#define RQ_IE     (S1H & SA_S1H_IE)
#define RQ_VEC    (rq->intVector)
#define RQ_DOINTI if (RQ_IE && RQ_VEC) io->SendInterrupt(io, 0)
#define RQ_DOINT  if (RQ_VEC)          io->SendInterrupt(io, 0)

// Controller Flags
#define CFLG_RESET  0x8000 // Reset State
#define CFLG_PI     0x0004 // Purge Interrupt Request 
#define CFLG_IE     0x0002 // Interrupt Enable 
#define CFLG_POLL   0x0001 // Poll Request

// Drive Flags
#define DFL_EXIST    0x8000 // Existant Drive
#define DFL_ATTACHED 0x4000 // Attached Medium
#define DFL_ATNPEND  0x0002 // Attention Pending
#define DFL_ONLINE   0x0001 // Online

#define RQ_NPKTS     32  // Number of Packets
#define RQ_PKT_WSIZE 32  // Packet Size in 8/16-bit Words
#define RQ_PKT_BSIZE (RQ_PKT_WSIZE << 1)
#define RQ_QHEAD     1   // Queue Head
#define RQ_QTAIL     0   // Queue Tail

typedef struct rq_Controller RQ_DEVICE;
typedef struct rq_Drive      RQ_DRIVE;
typedef struct rq_DriveType  RQ_DTYPE;
typedef struct rq_Packet     RQ_PACKET;

// Internal Packet Management
struct rq_Packet {
	RQ_PACKET *Next;              // Linked-List (NULL = End of packet)
	uint32    idPacket;           // Packet ID
	uint16    Data[RQ_PKT_WSIZE]; // Message Packet
};

struct rq_DriveType {
	char   *dtName;    // Drive Type Name
	char   *dtDescrip; // Description
	uint32 idModel;    // Model Identification
	uint32 idMedia;    // Media Identification
	uint32 Interface;  // Interface Type
	uint32 Flags;      // Drive Flags

	// Disk Geometry Information
	uint32 sec;        // Number of Sectors
	uint32 surf;       // Number of Surfaces
	uint32 cyl;        // Number of Cylinders;
	uint32 tpg;        // Tracks per Group;
	uint32 gpc;        // Groups per Cylinder;
	uint32 xbn;        // Extended Blocks
	uint32 dbn;        // Diagnostic Blocks
	uint32 lbn;        // Logical Blocks
	uint32 rcts;       // RCT Blocks
	uint32 rctc;       // RCT Copies
	uint32 rbn;        // Replacement Blocks
};

struct rq_Drive {
	char      *devName;    // Device Name (ie DUA0, MUA0, etc.)
	char      *dtName;     // Device Type Name (ie RDD40, RD53, etc.)
	char      *dtDescrip;  // Device Type Description
	char      *fileName;   // File Name
	int       File;        // File Descriptor
	int32     Flags;       // Drive Flags
	int32     idUnit;      // Unit Identification
	int32     idMedia;     // Media Identification
	RQ_DEVICE *rqDevice;   // Parent Unit (Controller Device)
	RQ_DTYPE  *dtInfo;     // Drive Type
	uint16    uFlags;      // MSCP Unit Flags
	RQ_PACKET *pktWork;    // Working (Current) Packet
	RQ_PACKET *pktQueue;   // Packet Queue
	CLK_QUEUE xfrTimer;    // Data Transfer Timer
};

struct rq_Controller {
	char      *devName;    // Device Name
	char      *keyName;    // Key (Device Type) Name
	char      *emuName;    // Emulator Name
	char      *emuVersion; // Emulator Version

	uint32    Flags;       // Internal Controller Flags
	uint32    State;       // Internal Controller State
	uint32    csrAddr;     // CSR Address
	uint32    intVector;   // Interrupt Vector Address

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
	RQ_PACKET *pktResp;    // Response List
	RQ_PACKET *pktFree;    // Free List
	RQ_PACKET pktList[RQ_NPKTS]; // Packet List

	// Disk/Tape Device Units
	int       nDrives;     // Number of disk/tape drives
	RQ_DRIVE  *Drives;
	uint8     *bufData;    // Data Transfers

	// Unibus/Q22-Bus I/O Table
	MAP_IO    ioMap;

	// Device Mapping Table for MSCP Device Units
	void     *System;
	void     *Device;
	UQ_CALL  *Callback;
};

// UQSSP Definitions

#define RQ(p) rq->p
#define IP    RQ(ip)       // Initialization and Poll Register
#define SA    RQ(sa)       // Status, Address, and Purge Register

#define SX(x) RQ(iData[x])
#define S1C   SX(0)        // Init Step 1 - Controller to Host
#define S1H   SX(1)        // Init Step 1 - Host to Controller
#define S2C   SX(2)        // Init Step 2 - Controller to Host
#define S2H   SX(3)        // Init Step 2 - Host to Controller
#define S3C   SX(4)        // Init Step 3 - Controller to Host
#define S3H   SX(5)        // Init Step 3 - Host to Controller
#define S4C   SX(6)        // Init Step 4 - Controller to Host
#define S4H   SX(7)        // Init Step 4 - Host to Controller 
