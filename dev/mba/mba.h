// mba.h - MASSBUS interface definitions
//
// Copyright (c) 2001-2003, Timothy M. Stark
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

#include "dev/uba/dec/defs.h"

#define MBA_MAXDRIVES  8   // Maximum number of drives per controller
#define MBA_MAXREGS    32  // Maximum number of registers per drive

typedef struct tblCallback  MBA_CALLBACK;
typedef struct tblDriveInfo MBA_DTYPE;
typedef struct tblInfo      MBA_DEVINFO;
typedef struct tblDrive     MBA_DRIVE;
typedef struct tblDevice    MBA_DEVICE;

struct tblCallback {
	void (*SetAttention)(void *);
	void (*SetReady)(void *);
	int  (*BeginIO)(void *);
	void (*EndIO)(void *);
	int  (*ReadBlock)(void *, uint8 *, uint32, uint32);
	int  (*WriteBlock)(void *, uint8 *, uint32, uint32);
	int  (*SetBoot)(void *, UQ_BOOT *);
};

// Drive type table
struct tblDriveInfo {
	char  *Name;      /* Device Name */
	char  *Desc;      /* Description */
	int32 Flags;      /* Device flags */
	int32 Sectors;    /* Number of Sectors */
	int32 Tracks;     /* Number of Tracks (Heads) */
	int32 Cylinders;  /* Number of Cylinders */
	int32 Blocks;     /* Number of Blocks */
	int32 dType;      /* Device Type ID */
	MBA_DTYPE *sTypes; /* Slave Drive Types */

	int32 uFlags;     /* User-definable Flags */
	void  *uData;     /* User-definable Data */

#if 0
	// Low-level Routines for disk, tape, ethernet, etc..
	int    (*Open)();        // low-level Open Routine
	int    (*Close)();       // low-level Close Routine
	int    (*Read)();        // low-level Read Routine
	int    (*Write)();       // low-level Write Routine
	int    (*Mark)();        // low-level Mark Routine
	int    (*Rewind)();      // low-level Rewind Routine
	int    (*Skip)();        // low-level Skip Routine
	int    (*Seek)();        // low-level Seek Routine
	int    (*GetDiskAddr)(); // low-level GetDiskAddr Routine
#endif 
};

struct tblInfo {
	char      *Name;    // Device Name
	char      *Desc;    // Description
	char      *Version; // Version
	MBA_DTYPE *dTypes;  // Specific Drive Type

	// Desired Function Calls
	void (*InitDrive)(MBA_DRIVE *);
	void (*ResetDrive)(MBA_DRIVE *);
	int  (*Attach)(MBA_DRIVE *, char *);
	int  (*Detach)(MBA_DRIVE *);
	int  (*Boot)(MBA_DRIVE *, int, char **);
	void (*ReadIO)(MBA_DRIVE *, int, uint16 *);
	void (*WriteIO)(MBA_DRIVE *, int, uint16);
	int  (*CheckAttention)(MBA_DRIVE *);
	void (*ClearAttention)(MBA_DRIVE *);
};

struct tblDrive {
	UNIT         Unit;       // Unit Header

	DEVICE       *devInfo;   // Device Information
	char         *Name;      // User-defined Device Name
	int          idDrive;    // Drive Number Identification
	int          Flags;      // Drive Flags
	MBA_DEVINFO  *dInfo;     // Drive Information/Calls
	MBA_DTYPE    *dType;     // Drive Type Information
	MBA_DEVICE   *wDevice;   // Which/Whose Device Controller
	void         *pDevice;   // Massbus controller (Parent device)
	MBA_CALLBACK *Callback;  //   Callback functions
	void         *FileRef;   // File/Slave Reference
	CLK_QUEUE    *Timer;     // Timer Alarm
	uint16       Regs[MBA_MAXREGS];
};

struct tblDevice {
	UNIT         Unit;       // Unit Header

	int          idDevice;  // Controller Number Identification
	void         *pDevice;  // Massbus controller (Parent device)
	MBA_CALLBACK *Callback; //   Callback functions
	MBA_DRIVE    Drive[MBA_MAXDRIVES];
};

#define MBA_REG(reg) mbaDrive->Regs[reg]

#define MBA_PRESENT  0x80000000 // Device is present.
#define MBA_ATTABLE  0x00000001 // Device is attachable.
#define MBA_ATTACHED 0x00000002 // Device is attached.
#define MBA_WRLOCKED 0x00000004 // Device is write-locked.

#define MBA_UNKNOWN 0
#define MBA_CTLR    1
#define MBA_FIXED   2
#define MBA_PACK    3
#define MBA_TAPE    4

#define MBA_OK  0  // Successful Operation
#define MBA_RAE 1  // Register Access Error
#define MBA_ATN 2  // Attention Signal
#define MBA_NED 3  // Non-existing Device

#define MBA_18B   2 // 18/36-bit data transfers
#define MBA_NOBAI 1 // BAI Force

#define MBA_CONT  UQ_CONT
#define MBA_ERROR UQ_ERROR

// Attention Summary Register for all drives.
#define MBA_DS     1       // Drive Status Register
#define MBA_DS_ATN 0100000 //   Attention Bit
#define MBA_AS     4       // Attention Summary Register

// Function code 
//
// Func  Fixed disk         Pack disk                Magnetic tape
// ----  ----------         ---------                -------------
// 000   No-op              No-op                    No-op
// 001                      Unload                   Rewind, offline
// 002   Seek               Seek
// 003   Recalibrate        Recalibrate              Rewind
// 004   Drive clear        Drive clear              Drive clear
// 005                      Port Release
// 006                      Offset
// 007                      Return to Clearline
// 010   Read-in Preset     Read-in Preset           Read-in Preset
// 011                      Pack Acknowledge
// 012                                               Erase
// 013                                               Write File Mark
// 014   Search             Search                   Space Forward
// 015                                               Space Reverse
// 024   Write Check Data   Write Check Data         Write Check forward
// 025                      Write Check Headr/Data
// 026                                               Write Check Reverse
// 030   Write data         Write data               Write Forward
// 031                      Write Header/Data
// 034   Read Data          Read Data                Read Forward
// 035                      Read Header/Data
// 037                                               Read Reverse

#define FNC_NOP         000 // (001) No operation
#define FNC_UNLOAD      001 // (003) Unload/Rewind and offline
#define FNC_SEEK        002 // (005) Seek
#define FNC_RECAL       003 // (007) Recalibrate
#define FNC_REWIND      003 // (007) Rewind
#define FNC_DCLR        004 // (011) Drive Clear
#define FNC_RELEASE     005 // (013) Port Release
#define FNC_OFFSET      006 // (015) Offset
#define FNC_RETURN      007 // (017) Return to clearline
#define FNC_PRESET      010 // (021) Read-in preset
#define FNC_PACK        011 // (023) Pack acknowledge
#define FNC_ERASE       012 // (025) Erase
#define FNC_WR_EOF      013 // (027) Write a file (tape) mark
#define FNC_SEARCH      014 // (031) Search
#define FNC_SP_FWD      014 // (031) Space Forward
#define FNC_SP_REV      015 // (033) Space Reverse
#define FNC_CHK_DATA    024 // (051) Write Check Data
#define FNC_CHK_FWD     024 // (051) Write Check Forward
#define FNC_CHK_HDR     025 // (053) Write Check Header/Data
#define FNC_CHK_REV     026 // (057) Write Check Reverse
#define FNC_WR_DATA     030 // (061) Write Data
#define FNC_WR_FWD      030 // (061) Write Forward
#define FNC_WR_HDR      031 // (063) Write Header/Data
#define FNC_RD_DATA     034 // (071) Read Data
#define FNC_RD_FWD      034 // (071) Read Forward
#define FNC_RD_HDR      036 // (075) Read Header/Data
#define FNC_RD_REV      037 // (077) Read Reverse

// Protype defintions
void mba_Create(void *, MBA_DEVICE *, MBA_CALLBACK *, int, int, char **);
void *mba_CreateDrive(MAP_DEVICE *, MBA_DEVICE *, int, char **);
int  mba_Attach(MBA_DRIVE *, int, char **);
int  mba_Detach(MBA_DRIVE *, int, char **);
int  mba_WriteIO(MBA_DRIVE *, int, uint16);
int  mba_ReadIO(MBA_DRIVE *, int, uint16 *);
int  mba_Info(MBA_DEVICE *, int, char **);
int  mba_Boot(MBA_DRIVE *, int, char **);
