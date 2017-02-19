// rl.h - RL11/RLV11: RL01/RL02 Disk Subsystem Emulator
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

#define RL_KEY     "RL11"
#define RLV_KEY    "RLV11"

#define RL_NAME    "RL01/RL02 Disk Subsystems Emulator"
#define RL_VERSION "v0.7 (Alpha)"

#define RL01_KEY   "RL01"
#define RL01_NAME  "5.2MB Disk Cartridge Drive"

#define RL02_KEY   "RL02"
#define RL02_NAME  "10.4MB Disk Cartridge Drive"

// Default Interrupt/Registers Settings
#define RL_IOADDR  0774400 // Default CSR address
#define RL_NREGS   4       // 4 RL11 Registers
#define RLV_NREGS  5       // 5 RLV11 Registers (includes RLBAE)
#define RL_NVECS   1       // 1 Interrupt Vector.
#define RL_IPL     UQ_BR5  // Default Interrupt Level BR5
#define RL_VEC     0160    // Default Vector Interrupt

// Unibus/Qbus Address Index (774400) (in 16-bit addressing)
#define RLCS  0 // (R/W) Control and Status Register
#define RLBA  1 // (R/W) Bus Address Register
#define RLDA  2 // (R/W) Disk Address Register
#define RLMP  3 // (R/W) Multipurpose Register
#define RLBAE 4 // (R/W) Bus Address Extension Registr (RLV11)

// RLCS - Control and Status Register
#define RLCS_ERR   0100000 // (R)   Comppsite Error
#define RLCS_DRE   0040000 // (R)   Drive Error
#define RLCS_NXM   0020000 // (R)   Non-Existant Memory
#define RLCS_HDE   0010000 // (R)   Header Error
#define RLCS_CRC   0004000 // (R)   CRC Error
#define RLCS_OPI   0002000 // (R)   Operation Incomplete
#define RLCS_DS    0001400 // (R/W) Drive Select
#define RLCS_CRDY  0000200 // (R/W) Controller Ready
#define RLCS_IE    0000100 // (R/W) Interrupt Enable
#define RLCS_BAE   0000030 // (R/W) Bus Address Extension (BA16/BA17)
#define RLCS_FUNC  0000016 // (R/W) Function Code
#define RLCS_DRDY  0000001 // (R)   Drive Ready

#define RLCS_ERRS  0176000 // All Error Bits
#define RLCS_RW    0001776 // Read/Write Access

#define RLCS_M_DS   3  // Drive Select Mask
#define RLCS_P_DS   8  // Drive Select Position
#define RLCS_M_BAE  3  // Bus Address Mask
#define RLCS_P_BAE  4  // Bus Address Position
#define RLCS_M_FUNC 7  // Function Code Mask
#define RLCS_P_FUNC 1  // Function Code Position

#define GET_DS(x)   (((x) >> RLCS_P_DS) & RLCS_M_DS)
#define GET_FUNC(x) (((x) >> RLCS_P_FUNC) & RLCS_M_FUNC)
#define PUT_BAE(x)  (((x) & RLCS_M_BAE) << RLCS_P_BAE)

// RLDA - Disk Address Register

#define RLDA_SK_HD    0000020 // Head Select
#define RLDA_GS_CLR   0000010 // Clear Errors
#define RLDA_SK_DIR   0000004 // Seek Direction

#define RLDA_M_SECTOR 077     // Sector Mask
#define RLDA_M_TRACK  01777   // Track Mask
#define RLDA_M_CYL    0777    // Cylinder Mask
#define RLDA_P_SECTOR 0       // Position of Sector Field
#define RLDA_P_TRACK  6       // Position of Track Field
#define RLDA_P_CYL    7       // Position of Sector Field

#define RLDA_SECTOR   (RLDA_M_SECTOR << RLDA_P_SECTOR)
#define RLDA_TRACK    (RLDA_M_TRACK << RLDA_P_TRACK)
#define RLDA_CYL      (RLDA_M_CYL << RLDA_P_CYL)
#define RLDA_HD0      (0 << RLDA_P_TRACK)
#define RLDA_HD1      (1u << RLDA_P_TRACK)

#define GET_SECTOR(x) (((x) >> RLDA_P_SECTOR) & RLDA_M_SECTOR)
#define GET_TRACK(x)  (((x) >> RLDA_P_TRACK) & RLDA_M_TRACK)
#define GET_CYL(x)    (((x) >> RLDA_P_CYL) & RLDA_M_CYL)
#define GET_DA(x)     ((GET_TRACK(x) * RL_NSECS) + GET_SECTOR(x))

#define PUT_SECTOR(x) (((x) & RLDA_M_SECTOR) << RLDA_P_SECTOR)
#define PUT_TRACK(x)  (((x) & RLDA_M_TRACK) << RLDA_P_TRACK)
#define PUT_CYL(x)    (((x) & RLDA_M_CYL) << RLDA_P_CYL)

// RLBA - Bus Address Register
#define RLBA_IMP   0177776 // (R/W) Bus Address 
#define RLBAE_IMP  0000077 // (R/W) Bus Address (RLV11 only)

// RLDS - Drive Status Register for RLMP
#define RLDS_WDE   0100000 // (R)   Write Data Error
#define RLDS_CHE   0040000 // (R)   Current Head Error
#define RLDS_WLK   0020000 // (R)   Write Lock
#define RLDS_SKTO  0010000 // (R)   Seek Time Out
#define RLDS_SPE   0004000 // (R)   Spin Error
#define RLDS_WGE   0002000 // (R)   Write Gate Error
#define RLDS_VC    0001000 // (R)   Volume Check
#define RLDS_DSE   0000400 // (R)   Drive Select Error
#define RLDS_DT    0000200 // (R)   Drive Type, RL01 = 0, RL02 = 1
#define RLDS_HD    0000100 // (R)   Head Select
#define RLDS_CO    0000040 // (R)   Cover Open
#define RLDS_HO    0000020 // (R)   Heads Out
#define RLDS_BH    0000010 // (R)   Brush Home
#define RLDS_ST    0000007 // (R)   State

#define RLDS_LOAD  0
#define RLDS_LOCK  5

#define RLDS_RL01  0
#define RLDS_RL02  RLDS_DT
#define RLDS_ATT   (RLDS_HO|RLDS_BH|RLDS_LOCK)
#define RLDS_UNATT (RLDS_CO|RLDS_LOAD)
#define RLDS_ERR \
	(RLDS_WDE|RLDS_CHE|RLDS_SKTO|RLDS_SPE|RLDS_WGE|RLDS_VC|RLDS_DSE)

// RL11 Function Codes
#define FUNC_NOP     0  // No Operation
#define FUNC_MAINT   0  // Maintenance (RLV11 only)
#define FUNC_WRCHK   1  // Write Check
#define FUNC_STATUS  2  // Get Status
#define FUNC_SEEK    3  // Seek
#define FUNC_RDHDR   4  // Read Header
#define FUNC_WRITE   5  // Write Data
#define FUNC_READ    6  // Read Data
#define FUNC_RDNOHDR 7  // Read Data Without Header Check

// RL01/RL02 Disk Geometries
#define RL01_SEC   40 // 40  Sectors (per track)
#define RL01_HEAD   2 // 2   Heads   (per cylinder)
#define RL01_CYL  256 // 256 Cylinders

#define RL02_SEC   40 // 40  Sectors (per track)
#define RL02_HEAD   2 // 2   Heads   (per cylinder)
#define RL02_CYL  512 // 512 Cylinders

#define RL_WSEC   128 // 16-bits words/sector
#define RL_8SEC   170 // 12-bits words/sector
#define RL_BSEC   256 // 8-bits bytes/sector
#define RL_NSECS   40 // Sectors per Track
#define RL01_TRKS 256 // Tracks per surface (RL01)
#define RL02_TRKS 512 // Tracks per surface (RL02)

#define RL01_SIZE (RL01_CYL * RL01_HEAD * RL01_SEC * RL_BSEC)
#define RL02_SIZE (RL02_CYL * RL02_HEAD * RL02_SEC * RL_BSEC)

#define RL_NDRVS    4 // Up to 4 disk pack drives.
#define RL_MAXFR    (1u << 16)

// RL11/RLV11 Controller Flags
#define CFLG_RLV11    0x00000001 // RLV11 Device Type
#define CFLG_RL11     0x00000000 // RL11 Device Type

// RL01/RL02 Drive Flags
#define DFLG_EXIST    0x80000000 // Drive is existing
#define DFLG_WLOCK    0x00000004 // Drive is write-locked
#define DFLG_ATTACHED 0x00000002 // Drive is attached (medium online)
#define DFLG_RL02     0x00000001 // Drive is RL02 device type
#define DFLG_RL01     0x00000000 // Drive is RL01 device type

typedef struct rl_Controller RL_DEVICE;
typedef struct rl_Drive      RL_DRIVE;

struct rl_Drive {
	char *devName;
	char *keyName;
	char *emuName;
	char *emuVersion;

	// Drive Flags
	RL_DEVICE *Device;   // Parent Device (Controller)
	int       idUnit;    // Unit/Drive Identification
	int       File;      // File Description
	char      *fileName; // Filename
	uint32    Flags;     // Drive Flags
	uint16    rlcs;      // Control and Status Register
	uint16    rlds;      // Drive Status Register
	uint16    track;     // Current Track/Sector

	uint32    totCyls;   // Total cylinders of disk file.
	uint32    totBlocks; // Total blocks of disk file.
	uint32    totBytes;  // Total bytes of disk file.
};

struct rl_Controller {
	// Its descriptions
	char *devName;
	char *keyName;
	char *emuName;
	char *emuVersion;

	void    *Device;
	void    *System;
	UQ_CALL *Callback;

	uint32 Flags;    // Controller Flags
	uint32 csrAddr;  // CSR Address

	// RL01/RL02 Disk Pack Drives (up to 4 drives)
	RL_DRIVE Drives[RL_NDRVS];

	// I/O Space Assignment
	MAP_IO ioMap;

	// Registers
	uint16 rlcs;   // Control and Status Register
	uint16 rlba;   // Bus Address Register
	uint16 rlda;   // Disk Address Register
	uint16 rlmp;   // Multipurpose Register #1
	uint16 rlmp1;  // Multipurpose Register #2
	uint16 rlmp2;  // Multipurpose Register #3
	uint16 rlbae;  // Bus Address Extension Register (RLV11 only)

	// Data Transfer Buffer
	uint8  bufData[RL_MAXFR];
};
