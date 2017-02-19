// mscp.h - Mass Storage Control Protocol - Definitions
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
// See ReadMe for copyright notice.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// MSCP Flags
#define MDF_EXIST  0x80000000 // Device is existing
#define MDF_AVATTN 0x40000000 // Device is now available.

#define MDF_576    0x00000400 // Device has 576-byte blocks
#define MDF_RDONLY 0x00000200 // Device is read-only hardware
#define MDF_RMVBL  0x00000100 // Media is removable
#define MDF_FIXED  0x00000000 // Device is fixed
#define MDF_MEDIA  0x000000FF // up to 256 media types.

#define MSCP_OWN   0x80000000 // Own Flag
#define MSCP_INT   0x40000000 // Interrupt Flag
#define MSCP_DONE  0x40000000 // Done Flag

// MSCP Media Format Types
#define MDT_UNKNOWN  0  // Unknown media type
#define MDT_DISK     1  // Disk Drive
#define MDT_FLOPPY   2  // Floppy Drive
#define MDT_CD       3  // CD-ROM Drive
#define MDT_TAPE     4  // Tape Drive

// Control Message Opcodes
#define MOP_ABORT     0x01 // (001) Abort Command
#define MOP_GETCMDST  0x02 // (002) Get Command Status Command
#define MOP_GETUNITST 0x03 // (003) Get Unit Status Command
#define MOP_SETCTLRC  0x04 // (004) Set Controller Characteristics Command
#define MOP_SEREXC    0x07 // (007) Serious Exception End Message
#define MOP_AVAILABLE 0x08 // (010) Available Command
#define MOP_ONLINE    0x09 // (011) Online Command
#define MOP_SETUNITC  0x0A // (012) Set Unit Characteristics Command
#define MOP_DTACCPATH 0x0B // (013) Determine Access Paths Command
#define MOP_ACCESS    0x10 // (020) Access Command
#define MOP_CMPCD     0x11 // (021) Compare Controller Data Command
#define MOP_ERASE     0x12 // (022) Erase Command
#define MOP_FLUSH     0x13 // (023) Flush Command
#define MOP_REPLACE   0x14 // (024) Replace Command
#define MOP_CMPHD     0x20 // (040) Compare Host Data Command
#define MOP_READ      0x21 // (041) Read Command
#define MOP_WRITE     0x22 // (042) Write Command
#define MOP_WRMARK    0x24 // (044) Write Mark Command
#define MOP_POS       0x25 // (045) POsitioning Command
#define MOP_AVATTN    0x40 // (100) Available Attention Message
#define MOP_DUPUNIT   0x41 // (101) Duplicate Unit Number Attention Message
#define MOP_ACCPATH   0x42 // (102) Access Path Attention Message
#define MOP_END       0x80 // (200) End Message Flag
#define MOP_CMDMASK   0x7F // Command Opcode Mask

// MSCP Status Flags
#define MST_SUCCESS   0x00 // (000) Success
#define MST_INVALCMD  0x01 // (001) Invalid Command
#define MST_ABORTED   0x02 // (002) Command Aborted
#define MST_OFFLINE   0x03 // (003) Unit Offline
#define MST_AVAIL     0x04 // (004) Unit Available
#define MST_MDFMTERR  0x05 // (005) Media Format Error
#define MST_WRPROT    0x06 // (006) Write Protected
#define MST_CMPERR    0x07 // (007) Compare Error
#define MST_DATAERR   0x08 // (010) Data Error
#define MST_HSTBUFERR 0x09 // (011) Host Buffer Access Error
#define MST_CTLRERR   0x0A // (012) Controller Error
#define MST_DRVERR    0x0B // (013) Drive Error
#define MST_FMTERR    0x0C // (014) Formatter Error
#define MST_BOT       0x0D // (015) Beginning of Tape
#define MST_TAPEMARK  0x0E // (016) Tape Mark Encountered
#define MST_RDTRUNC   0x10 // (020) Record Data Truncated
#define MST_DIAG      0x1F // (037) Message from Internal Diagnostic
#define MST_MASK      0x1F // Status Mask for Modifier/Status Field

// MSCP Offline Status (Subcodes of MST_OFFLINE)
#define MOFF_INDIAGNOSTIC (8 << 5) // Disabled by Diagnostic 
#define MOFF_DUPLICATE    (4 << 5) // Duplicate Unit Number
#define MOFF_INOPER       (2 << 5) // Inoperative
#define MOFF_UNMOUNTED    (1 << 5) // Unmounted
#define MOFF_UNKNOWN      (0 << 5) // Unknown / Other Controller

// MSCP Controller Flags
#define MCF_ATTN     0x80  // Enable Attention Messages
#define MCF_MISC     0x40  // Enable Miscellaneous Error Log Messages
#define MCF_OTHER    0x20  // Enable Other Host's Error Log Messages
#define MCF_THIS     0x10  // Enable This Host's Error Log Messages
#define MCF_MULTI    0x04  // Multi-Host
#define MCF_SHADOW   0x02  // Shadowing
#define MCF_576      0x01  // 576-byte Sectors

// MSCP Unit Flags
#define MUF_SUPCHH    0x8000 // Suppress caching for high speed
#define MUF_SUPCHL    0x4000 // Suppress caching for low speed
#define MUF_REPLC     0x8000 // Controller initiated bad block replacement
#define MUF_INACT     0x4000 // Inactive shadow set unit
#define MUF_WRPRTH    0x2000 // Write Protect (Hardware)
#define MUF_WRPRTS    0x1000 // Write Protect (Software/Volume)
#define MUF_RMVBL     0x0080 // Media Removable
#define MUF_WRBKNV    0x0040 // Write Back Non-Volatile
#define MUF_576       0x0004 // 576-bytes Sectors
#define MUF_CMPWR     0x0002 // Compare Writes
#define MUF_CMPRD     0x0001 // Compare Reads

// Packet length (excludes MSCP header)
#define MSCP_SIZE(mp) (sizeof(mp) - sizeof(MSCP_HEADER))

typedef struct mscpDeviceType  MSCP_DTYPE;
typedef struct mscpHeader      MSCP_HEADER;
typedef struct mscpPacket      MSCP_PACKET;
typedef struct mscpGetUnitEnd  GUSE_PACKET;
typedef struct mscpCtlrCharCmd SCCC_PACKET;
typedef struct mscpCtlrCharEnd SCCE_PACKET;
typedef struct mscpOnlineCmd   ONLC_PACKET;
typedef struct mscpOnlineEnd   ONLE_PACKET;
typedef struct mscpDataXfer    XFER_PACKET;
typedef struct mscpOpcode      MSCP_OPCODE;
typedef struct mscpLastFail    MSCP_LASTFAIL;
typedef struct mscpAvailAttn   MSCP_AVATTN;

struct mscpDeviceType {
	char       *dtName;    // Drive Type Name
	char       *dtDescrip; // Description
	uint32     Flags;      // Devie Type Flags
	uint32     idMedia;    // Media ID
	uint32     Interleave; // Sector Interleave;
	uint32     secSize;    // Bytes per Sector
	uint32     Sectors;    // Sectors per LBN (Logical Block)
	uint32     Blocks;     // LBNs per Track
	uint32     Tracks;     // Tracks per Group (Surface)
	uint32     Groups;     // Groups per Cylinder
	uint32     Cylinders;  // Cylinders per Unit
	uint32     lbnBlocks;  // Number of Total Logical Blocks
	uint32     lbnUser;    // Number of User Logical Blocks
	uint32     xbnBlocks;  // Number of Transfer Blocks
	uint32     dbnBlocks;  // Number of Diagnostics Blocks
	uint32     rbnBlocks;  // Number of Replacement Blocks
	uint32     rctBlocks;  // Number of RCT Blocks
	uint32     rctCopies;  // Copies of RCT Tables
};

// Get Unit Status Command Variant

struct mscpGetUnitEnd {
	uint16  muCode;     // Multi-Unit Code
	uint16  untFlags;   // Unit Flags
	uint32  idHost;     // Host ID
	uint16  idUnit[4];  // Unit ID (Drive Type in Unit ID #3)
	uint32  idMedia;    // Media ID
	uint16  shwUnit;    // Shadow Unit
	uint16  shwStatus;  // Shadow Status
	uint16  trkSectors; // Sectors per Track
	uint16  szGroup;    // Track Group Size
	uint16  cylGroups;  // Groups per Cylinder
	uint16  rsvd;       // Reserved
	uint16  rctsize;    // RCT Size
	uint8   nrpt;       // RBNs per track
	uint8   nrct;       // Number of RCTs.
};

// Set Controller Characteristics Command Variant

struct mscpCtlrCharCmd {
	uint16  Version;    // MSCP Version Number
	uint16  Flags;      // Controller Flags
	uint16  Timeout;    // Host Timeout
	uint16  useFrac;    // Use Fraction
	int64   Time;       // Time and Date
	int32   errFlags;   // Error Log Flags
	int16   dummy;      // (Reserved)
	int16   cpySpeed;   // Copy Speed
};

struct mscpCtlrCharEnd {
	uint16  Version;    // MSCP Version Number
	uint16  Flags;      // Controller Flags
	uint16  Timeout;    // Controller Timeout
	uint16  Command;    // Controller ???
	uint16  idCtlr[3];  // Controller Options
	int16   dummy[3];   // (Reserved)
	int16   snVolume;   // Volume Serial Number
};

// Online Command Variant

struct mscpOnlineCmd {
	uint16  dummy1;     // Reserved Area
	uint16  untFlags;   // Unit Flags
	uint16  dummy2[6];  // Reserved Area
	uint32  dParmeters; // Device Dependent Parameters
	uint16  shwUnit;    // Shadow Unit
	uint16  cpySpeed;   // Copy Speed
};

struct mscpOnlineEnd {
	uint16 muCode;      // Multi-Unit Code
	uint16 untFlags;    // Unit Flags;
	uint32 dummy1;      // Reserved Area
	uint64 idUnit;      // Unit Identification
	uint32 idMedia;     // Media Identification
	uint16 shwUnit;     // Shadow Unit;
	uint16 shwStatus;   // Shadow Status;
	uint32 szUnit;      // Unit Size
	uint32 snVolume;    // Volume Serial Number
};

// Read/Write Command Variant

struct mscpDataXfer {
	uint32     cntBytes;   // Byte Count
	uint32     buffer[3];  // Buffer Descriptor
	uint32     lbnBlock;   // Logical Block Number
};

struct mscpHeader {
	// Message Header
	uint16   msgSize;  // Message Packet Length
	uint8    msgType;  // Message Packet Type
	uint8    vcid;     // Virtual Circuit ID#
};

struct mscpPacket {
	// Message Header
	uint16   msgLength;  // -4 Message Length
	uint8    msgType;    // -2 Message Type
	uint8    vcid;       // -1 Virtual Circuit ID#

	// Message Packet
	uint32   cmdRef;     //  0 Command Refernce
	uint16   idUnit;     //  4 Unit #
	uint16   Sequence;   //  6 Sequence #
	uint8    opCode;     //  8 Opcode
	uint8    opFlags;    //  9 Flags
	uint16   Status;     // 10 Modifiers/Status

	// Data Area
	union {                    // 12 Data Area (50 bytes)
		uint8       Data8[54];  // 8-bit data area
		uint16      Data16[27]; // 16-bit data area
		GUSE_PACKET guse;       // Get Unit Status End
		SCCC_PACKET sccc;       // Set Controller Flags Command
		SCCE_PACKET scce;       // Set Controller Flags End
		ONLC_PACKET onlc;       // Online Command
		ONLE_PACKET onle;       // Online End
		XFER_PACKET xfer;       // Read/Write Command
	} u;
};

// Last Fail Message Packet
// ------------------------
//
// Controller sends a Last Fail message packet if an error is detected
// since the last initialization and the LF bit was set in step #4 of
// the current initialization.

struct mscpLastFail {
	MSCP_HEADER hdr;        // MSCP Header (Size, Type, and Credits)
	uint32      Zeros[2];   // All Zeros
	uint16      val400;     // ??? 400 (8)
	uint16      val12;      // ??? 12 (8)
	uint32      idCtlr[2];  // Controller Identification
	uint8       swUnitRev;  // Unit Software Revision
	uint8       hwUnitRev;  // Unit Hardware Revision
	uint16      cdError;    // Error Code
};

// Available Attention Message
// ---------------------------

struct mscpAvailAttn {
	MSCP_HEADER hdr;      // MSCP Header (Size, Type, and Credits)
	uint32      cmdRef;   // Command Refernce
	uint16      idUnit;   // Unit #
	uint16      Sequence; // Sequence #
	uint8       opCode;   // Opcode
	uint8       opFlags;  // Flags
	uint16      Status;   // Modifiers/Status
};

struct mscpOpcode {
	uint8 opCode;
	void  (*Execute)(void *, MSCP_PACKET *);
};
