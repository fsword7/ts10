// mscp.c -  MSCP Device Emulator
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

#include "emu/defs.h"
#include "dev/defs.h"
#include "dev/mscp.h" // MSCP Defintions (Specification)

// Look device type table in mscp_dev.c file.
extern MSCP_DTYPE mscp_dtList[];

#define MSCP_DTNAME  "MSCP"
#define MSCP_NAME    "MSCP Device Emulator"
#define MSCP_VERSION "v0.5 (Alpha)"

#define UDA_IOADDR     017772150  // Default 22-bit I/O Address Space
#define UDA_INT        20         // IPL 14 (BR4)

#define UDA_UVER    0x133 // Model 19, Rev 3

#define STATE_NONE   0 // Power Up - Initially Zero
#define STATE_STEP1  1 // Initialization Step 1
#define STATE_STEP2  2 // Initialization Step 2
#define STATE_STEP3  3 // Initialization Step 3
#define STATE_STEP4  4 // Initialization Step 4
#define STATE_READY  5 // Initialization Step 5
#define STATE_WRAP   6 // Wrap Mode

#define UDA_SENDALL 1

static const char *stateName[] =
{
	"Idle",      // Idle State
	"Step 1",    // Initialization Step 1
	"Step 2",    // Initialization Step 2
	"Step 3",    // Initialization Step 3
	"Step 4",    // Initialization Step 4
	"Ready",     // Ready
	"Wrap Mode"  // Wrap Mode (Diagnostics)
};

#ifdef DEBUG
static const char *regName[] = { "IP", "SA" };

static const char *opName[] =
{
	"\001Abort Command",
	"\002Get Command Status Command",
	"\003Get Unit Status Command",
	"\004Set Controller Characteristics Command",
	"\007Serious Exception End Message",
	"\010Available Command",
	"\011Online Command",
	"\012Set Unit Characteristics Command",
	"\013Determine Access Paths Command",
	"\020Access Command",
	"\021Compare Controller Data Command",
	"\022Erase Command",
	"\023Flush Command",
	"\024Replace Command",
	"\040Compare Host Data Command",
	"\041Read Command",
	"\042Write Command",
	"\044Write Mark Command",
	"\045Positioning Command",
	"\100Available Attention Message",
	"\101Duplicate Unit Number Attention Message",
	"\102Access Path Attention Message",
	"\200End Message Flag",
	NULL
};

static const char *msgStatus[] =
{
	"Success",
	"Invalid command",
	"Command aborted",
	"Unit offline",
	"Unit available",
	"Media format error",
	"Write protected",
	"Compare error",
	"Data error",
	"Host buffer access error",
	"Controller error",
	"Drive error",
	"Formatter error",
	"Beginning-of-tape",
	"Tape mark encountered",
	NULL,
	"Record data truncated",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Message from an internal diagnostic"
};
#endif /* DEBUG */

// MSCP Controller Series - Register Table
//
// 0000 (R/W) Initialization and Polling Register
// 0002 (R/W) Status, Address and Purge Register

#define MSCP_IP 0 // Initialization and Polling Register
#define MSCP_SA 1 // Status, Address and Purge Register

// 0002 SA (R/W) Status, Address and Purge Register
//
// MQER Field Format
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER|S4|S3|S2|S1|         Values Vary            |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// Initilaization Step 1
// ---------------------
//
// MQSA1 (R0) - Read from SA
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER| 0| 0| 0| 1|NV|QB|DI|       Reserved        |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// MQSA1 (W0) - Response
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// | 1|WR| CRING  | RRING  |IE| Int Vector Addr/4  |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// Initialization Step 2
// ---------------------
//
// MQSA2 (R0) - Read from SA
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER| 0| 0| 1| 0|Rserved | 1| 0| CRING  | RRING  |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// MQSA2 (W0) - Response
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |          Ring Base Address (Low)           |PI|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// Initialization Step 3
// ---------------------
//
// MQSA3 (R0) - Read from SA
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER| 0| 1| 0| 0|Rserved |IE| Int Vector Addr/4  |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// MQSA3 (W0) - Response
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |PP|          Ring Base Address (High)          |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// Initialization Step 4
// ---------------------
//
// MQSA4 (R0) - Read from SA
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER| 1| 0| 0| 0|    Model Number    | Revision  |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// MQSA4 (W0) - Response
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |       Reserved        |     Burst       |LF|GO|
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_ER     0100000 // Error 
#define SA_S4     0040000 // Initialization - Step 4
#define SA_S3     0020000 // Initialization - Step 3
#define SA_S2     0010000 // Initialization - Step 2
#define SA_S1     0004000 // Initialization - Step 1

#define SA_NV     0002000  // (R1) !Host-Settable Interrupt Vector Address 
#define SA_QB     0001000  // (R1) Controller is a QBus device
#define SA_DI     0000400  // (R1) Enhanced Diagnostics

#define SA_WRAP   0040000 // (W1) Diagnostic Wrap Mode
#define SA_CL     0034000 // (W1) Command Ring Length
#define SA_RL     0003400 // (W1) Response Ring Length
#define SA_IE     0000200 // (W1) Interrupt Enable
#define SA_VECTOR 0000177 // (W1) Interrupt Vector Address

#define SA_LEN    07
#define SA_P_CLEN 11
#define SA_P_RLEN 8

#define SA_CL    0000070 // (R2) Command Ring Length
#define SA_RL    0000007 // (R2) Response Ring Length

#define SA_PP     0100000 // (W3) Purge and Poll Tests
#define SA_RBLOW  0077777 // (W3) Ringbase Address Low
#define SA_LF     0000002 // (W4) Last Fail Packet Request
#define SA_PI     0000001 // (W2) Purge Interrupts Request
#define SA_GO     0000001 // (W4) Go

#define SA_BURST   077
#define SA_P_BURST 2

#define SA_STEP  0074000 // Initialization Step Mask

typedef struct uda_Device MSCP_DEVICE;
typedef struct mscpType   MSCP_TYPE;
typedef struct uda_Unit   MSCP_UNIT;

// MSCP Device Unit
struct uda_Unit {
	char       *devName;   // Device Name (ie DUA0, MUA0, etc.)
	char       *dtName;    // Device Type Name (ie RDD40, RD53, etc.)
	char       *dtDescrip; // Device Type Description
	char       *fileName;  // File Name
	int        File;       // File Descriptor
	int32      Flags;      // Drive Flags
	int32      idUnit;     // Unit Identification
	int32      idMedia;    // Media Identification
	MSCP_DEVICE *udaDevice; // Parent Unit (Controller Device)
	MSCP_DTYPE *dtInfo;    // Device Type
};

// MSCP Type (Controller)
struct mscpType {
	char  *dtName;   // Controller Name
	int32 idModel;   // Model Number
	int32 nDrives;   // Number of Supported Drives.
};

// MSCP Device
struct uda_Device {
	char        *devName;     // Device Name
	char        *emuName;     // Emulator Name
	char        *emuVersion;  // Emulator Version

	uint32 Flags;     // Controller Flags
	uint32 State;     // Controller State
	uint32 rbAddr;    // Ringbase Address
	uint32 csrAddr;   // CSR Address
	uint32 intVector; // Interrupt Vector Address

	uint16 cFlags;    // Host-settable Controller Flags
	uint16 saFlags;
	uint16 cLen, rLen;
	uint16 curRes;     // Current Response Descriptor
	uint16 burst;
	uint16 lf;
	uint16 go;

	uint16 ip; // Initialization and Polling Register
	uint16 sa; // Status, Address, and Purge Register

	// Disk/Tape Device Units
	int       nUnits;  // Number of MSCP Device Units 
	MSCP_UNIT  *Units;

	// Unibus/Q22-Bus I/O Table
	MAP_IO ioMap;

	// Device Mapping Table for MSCP Device Units
	void        *System;
	void        *Device;
	QBA_CALL    *Callback;
};

// ************************************************************

void mscp_SendPacket(MSCP_DEVICE *uda, void *msgPacket, int msgSize)
{
	MAP_IO   *io     = &uda->ioMap;
	QBA_CALL *call   = uda->Callback;
	uint16   *rptr16 = (uint16 *)call->GetHostAddr(uda->System, uda->rbAddr);
	uint32   *rptr32 = (uint32 *)rptr16;
	int      resSize = 1u << uda->rLen; // Get number of slots
	uint32   ioAddr  = NULL;
	int      idx;

	// Find the available response descriptor.
	for (idx = 0; idx < resSize; idx++) {
		if (rptr32[uda->curRes] & MSCP_OWN) {
			ioAddr = (rptr32[uda->curRes] & ~(MSCP_OWN|MSCP_INT)) - 4;
			break;
		}
		if (++uda->curRes == resSize)
			uda->curRes = 0;
	}

	if (ioAddr) {
#ifdef DEBUG
		dbg_Printf("UDA: Response Dscriptor: %08X\n", rptr32[uda->curRes]);
#endif /* DEBUG */

		// Send an message packet now.
		call->WriteBlock(uda->System, ioAddr, msgPacket, msgSize + 4);

		// Set done flag and send an interrupt if I bit was set.
		if (rptr32[uda->curRes] & MSCP_INT)
			io->SendInterrupt(io);
		rptr32[uda->curRes] = (rptr32[uda->curRes] | MSCP_DONE) & ~MSCP_OWN;
		if (++uda->curRes == resSize)
			uda->curRes = 0;

		// Incrment response interrupt count
		rptr16[-1]++;
	}
}

void mscp_SendLastFail(MSCP_DEVICE *uda, uint16 cdError)
{
	MSCP_LASTFAIL msgLastFail;

	// Clear all message packet.
	memset(&msgLastFail, 0, sizeof(MSCP_LASTFAIL));

	// Set up the message header
	msgLastFail.hdr.msgSize = MSCP_SIZE(msgLastFail);
	msgLastFail.hdr.msgType = 0x10; // Datagram
	msgLastFail.hdr.vcid    = 0xFF; // Diagnostics

	// Set up the Last Fail message packet.
	msgLastFail.val400    = 0400;
	msgLastFail.val12     = 0012;
	msgLastFail.idCtlr[0] = 0x00000000;
	msgLastFail.idCtlr[1] = 0x00130000;
	msgLastFail.swUnitRev = 0;
	msgLastFail.hwUnitRev = 3;
	msgLastFail.cdError   = cdError;

	// Now send it.
	mscp_SendPacket(uda, &msgLastFail, msgLastFail.hdr.msgSize);
}

// Send Available Attention Message
void mscp_SendAvailAttn(MSCP_UNIT *drv)
{
	MSCP_AVATTN mp;

	memset(&mp, 0, sizeof(MSCP_AVATTN));

	// Set MSCP header
	mp.hdr.msgSize = MSCP_SIZE(mp);
	mp.hdr.msgType = 0;
	mp.hdr.vcid    = 0;

	// Set Available Attention Message
	mp.opCode = MOP_AVATTN;
	mp.idUnit = drv->idUnit;

	// Now send it.
	mscp_SendPacket(drv->udaDevice, &mp, mp.hdr.msgSize);

	drv->Flags &= ~MDF_AVATTN;
}

// Media ID Format
//
// |<---- Port ------->|<---------------- Drive ------------------>|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         |         |         |         |         |    Number   |
// +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0

char *mscp_GetMediaID(uint32 idMedia)
{
	static char ascMedia[10];
	char *str = ascMedia;
	char ch;
	int  idx;

	for (idx = 4; idx >= 0; idx--) {
		ch = (idMedia >> ((idx * 5) + 7)) & 037;
		*str++ = (ch ? (ch + '@') : ' ');
	}
	str = ToRadix(str, idMedia & 0x7F, 10, 2);
	*str++ = '\0';

	return ascMedia;
}

#ifdef DEBUG
void mscp_DumpPacket(MSCP_DEVICE *uda, uint32 mscpAddr)
{
	QBA_CALL *call = uda->Callback;
	uint16   *rptr = (uint16 *)call->GetHostAddr(uda->System, mscpAddr);
	char     *pName;
	uint8    opCode, Flags;
	int32    idx;

	dbg_Printf("  MSCP Packet (Address %06X)\n", mscpAddr);
	dbg_Printf("   -4 Message Length          %04X\n", rptr[-2]);
	dbg_Printf("   -2 ConnId, MsgTyp, Credits %04X\n", rptr[-1]);
	dbg_Printf("    0 Command Ref (Low)       %04X\n", rptr[0]);
	dbg_Printf("    2 Command Ref (High)      %04X\n", rptr[1]);
	dbg_Printf("    4 Unit                    %04X\n", rptr[2]);
	dbg_Printf("    6 Sequence                %04X\n", rptr[3]);
	dbg_Printf("    8 Flags, Opcode           %04X\n", rptr[4]);
	dbg_Printf("   10 Modifier                %04X\n", rptr[5]);
	for(idx = 6; idx < rptr[-2]/2; idx++)
		dbg_Printf("  %3d Data                    %04X\n", idx << 1, rptr[idx]);

	opCode = rptr[4] & 0xFF;
	Flags  = ((rptr[4] >> 8) & 0x1F);

	pName = "Unknown Opcode"; // Assume unknown opcode
	for (idx = 0; opName[idx]; idx++) {
		if (opCode == *opName[idx]) {
			pName = opName[idx]+1;
			break;
		}
	}

	dbg_Printf("  MSCP Packet (Address %06X)\n", mscpAddr);
	dbg_Printf("    Unit:   %d  Sequence: %d\n", rptr[2], rptr[3]);
	dbg_Printf("    Opcode: %02X - %s\n", opCode, pName);
	dbg_Printf("    Flags:  %02X - %s\n", Flags, msgStatus[Flags]);
}

void mscp_DumpPacket2(MSCP_PACKET *mp, uint32 mscpAddr)
{
	uint8    opCode, opFlags;
	char     *pName;
	int32    idx;

	opCode  = mp->opCode & MOP_CMDMASK;
	opFlags = mp->opFlags;
	pName   = "Unknown Opcode"; // Assume unknown opcode
	for (idx = 0; opName[idx]; idx++) {
		if (opCode == *opName[idx]) {
			pName = opName[idx]+1;
			break;
		}
	}

	dbg_Printf("  MSCP Packet (Address %06X)\n", mscpAddr);
	dbg_Printf("    VCID: %d  Type: %02X  Length: %d bytes\n",
		mp->vcid, mp->msgType, mp->msgLength);
	dbg_Printf("    Cmd Ref: %08X\n", mp->cmdRef);
	dbg_Printf("    Unit:    %d  Sequence: %d\n", mp->idUnit, mp->Sequence);
	dbg_Printf("    Opcode:  %02X - %s%s\n",
		opCode, pName, (mp->opCode & MOP_END ? " (End Message Flag)" : ""));
	dbg_Printf("    Flags:   %02X - %s\n", opFlags, msgStatus[opFlags]);

	switch (mp->opCode) {
		case MOP_GETUNITST:
			// Get Unit Status Command (No data)
			break;

		case MOP_GETUNITST|MOP_END:
			// Get Unit Status End
			dbg_Printf("    Multi-Unit Code:     %d\n",    mp->u.guse.muCode);
			dbg_Printf("    Unit Flags:          %04X\n",  mp->u.guse.untFlags);
			dbg_Printf("    Host ID:             %d\n",    mp->u.guse.idHost);
			dbg_Printf("    Unit ID:             %04X %04X %04X %04X\n",
				mp->u.guse.idUnit[0], mp->u.guse.idUnit[1],
				mp->u.guse.idUnit[2], mp->u.guse.idUnit[3]);
			dbg_Printf("    Media ID:            %s\n", 
				mscp_GetMediaID(mp->u.guse.idMedia));
			dbg_Printf("    Shadow Unit:         %d\n",    mp->u.guse.shwUnit);
			dbg_Printf("    Shadow Status:       %04X\n",  mp->u.guse.shwStatus);
			dbg_Printf("    Sectors Per Track:   %d\n",    mp->u.guse.trkSectors);
			dbg_Printf("    Track Group Size:    %d\n",    mp->u.guse.szGroup);
			dbg_Printf("    Groups Per Cylinder: %d\n",    mp->u.guse.cylGroups);
			dbg_Printf("    RCT Size:            %d\n",    mp->u.guse.rctsize);
			dbg_Printf("    RBNs Per Track:      %d\n",    mp->u.guse.nrpt);
			dbg_Printf("    Number of RCTs:      %d\n",    mp->u.guse.nrct);
			break;

		case MOP_SETCTLRC:
			// Set Controller Characteristics Command
			dbg_Printf("    MSCP Version:       %d\n",     mp->u.sccc.Version);
			dbg_Printf("    Controller Flags:   %04X\n",   mp->u.sccc.Flags);
			dbg_Printf("    Host Timeout:       %d\n",     mp->u.sccc.Timeout);
			dbg_Printf("    Use Fraction:       %04X\n",   mp->u.sccc.useFrac);
			dbg_Printf("    Time/Date:          %016LX\n", mp->u.sccc.Time);
			dbg_Printf("    Error Log Flags:    %04X\n",   mp->u.sccc.errFlags);
			dbg_Printf("    Copy Speed:         %04X\n",   mp->u.sccc.cpySpeed);
			break;

		case MOP_SETCTLRC|MOP_END:
			// Set Controller Characteristics Commad - End Message
			dbg_Printf("    MSCP Version:       %d\n",     mp->u.scce.Version);
			dbg_Printf("    Controller Flags:   %04X\n",   mp->u.scce.Flags);
			dbg_Printf("    Controller Timeout: %d\n",     mp->u.scce.Timeout);
			dbg_Printf("    Controller ???:     %04X\n",   mp->u.scce.Command);
			dbg_Printf("    Controller Options: %04X %04X %04X %04X\n",
				mp->u.scce.idCtlr[0], mp->u.scce.idCtlr[1],
				mp->u.scce.idCtlr[2], mp->u.scce.idCtlr[3]);
			dbg_Printf("    Volume Serial #:    %04X\n",   mp->u.scce.snVolume);
			break;

		case MOP_ONLINE:
			// Online Command
			dbg_Printf("    Unit Flags:         %04X\n", mp->u.onlc.untFlags);
			dbg_Printf("    Device Parmeters:   %08X\n", mp->u.onlc.dParmeters);
			dbg_Printf("    Shadow Unit:        %04X\n", mp->u.onlc.shwUnit);
			dbg_Printf("    Copy Speed:         %04X\n", mp->u.onlc.cpySpeed);
			break;

		case MOP_ONLINE|MOP_END:
			// Online Command - End Message
			dbg_Printf("    Unit Flags:         %04X\n",   mp->u.onle.untFlags);
			dbg_Printf("    Unit ID:            %016LX\n", mp->u.onle.idUnit);
			dbg_Printf("    Media ID:           %s\n",
				mscp_GetMediaID(mp->u.onle.idMedia));
			dbg_Printf("    Shadow Unit:        %04X\n",   mp->u.onle.shwUnit);
			dbg_Printf("    Shadow Status:      %04X\n",   mp->u.onle.shwStatus);
			dbg_Printf("    Unit Size:          %d\n",     mp->u.onle.szUnit);
			dbg_Printf("    Volume Serial #:    %d\n",     mp->u.onle.snVolume);
			break;

		case MOP_READ:
		case MOP_WRITE:
			dbg_Printf("    Byte Count:         %08X (%d bytes)\n",
				mp->u.xfer.cntBytes, mp->u.xfer.cntBytes);
			dbg_Printf("    Buffer #1:          %08X\n", mp->u.xfer.buffer[0]);
			dbg_Printf("    Buffer #2:          %08X\n", mp->u.xfer.buffer[1]);
			dbg_Printf("    Buffer #3:          %08X\n", mp->u.xfer.buffer[2]);
			dbg_Printf("    Logical Block #:    %d\n",   mp->u.xfer.lbnBlock);
			break;

		default:
			for(idx = 6; idx < ((mp->msgLength - 4) / 2); idx++)
				dbg_Printf("  %3d Data %04X\n", idx << 1, mp->u.Data16[idx - 6]);
	};
}

void mscp_DumpRing(MSCP_DEVICE *uda)
{
	QBA_CALL *call = uda->Callback;
	uint16 *rptr   = (uint16 *)call->GetHostAddr(uda->System, uda->rbAddr);
	int32 resSize  = 1 << uda->rLen;
	int32 cmdSize  = 1 << uda->cLen;
	int32 mscpAddr, pktAddr;
	int32 idx;

	dbg_Printf("Ringbase Address: %08X\n", uda->rbAddr);
	dbg_Printf("  (Host Address: %08X)\n", rptr);
	dbg_Printf("  -8  Reserved   %04X\n", rptr[-4]);
	dbg_Printf("  -6  Adp Chan   %04X\n", rptr[-3]);
	dbg_Printf("  -4  Cmd Int    %04X\n", rptr[-2]);
	dbg_Printf("  -2  Res Int    %04X\n", rptr[-1]);

	for (idx = 0; idx < resSize; idx++) {
		mscpAddr = (rptr[(idx << 1)+1] << 16) | rptr[idx << 1];
		dbg_Printf(" %3d Response Descriptor  %08X\n", idx << 2, mscpAddr);
//		if (pktAddr = mscpAddr & ~(MSCP_OWN|MSCP_INT))
//			mscp_DumpPacket(uda, pktAddr);
	}

	for (idx = resSize; idx < (resSize + cmdSize); idx++) {
		mscpAddr = (rptr[(idx << 1)+1] << 16) | rptr[idx << 1];
		dbg_Printf(" %3d Command Descriptor  %08X\n", idx << 2, mscpAddr);
		if (mscpAddr & MSCP_OWN) {
			pktAddr = mscpAddr & ~(MSCP_OWN|MSCP_INT);
			mscp_DumpPacket(uda, pktAddr);
		}
	}
}

#endif /* DEBUG */

// ******************** MSCP Opcode Commands *******************

// 03 (003) - Get Unit Status Command
void mscp_CmdGetUnitStatus(void *dptr, MSCP_PACKET *mp)
{
	MSCP_DEVICE *uda  = (MSCP_DEVICE *)dptr;
	uint32      idUnit = mp->idUnit;
	MSCP_UNIT   *uptr;
	MSCP_DTYPE  *dtInfo;

	if ((idUnit < uda->nUnits) && (uda->Units[idUnit].Flags & MDF_EXIST)) {
		uptr   = &uda->Units[idUnit];
		dtInfo = uptr->dtInfo;

		// Load drive characteristics
		mp->u.guse.idMedia    = dtInfo->idMedia;
		mp->u.guse.idUnit[2]  = dtInfo->idMedia & 0x7F;
		mp->u.guse.shwUnit    = 0;
		mp->u.guse.shwStatus  = 0;

		// Load drive geometries
		mp->u.guse.trkSectors = dtInfo->Blocks;
		mp->u.guse.szGroup    = dtInfo->Tracks;
		mp->u.guse.cylGroups  = dtInfo->Groups;
		mp->u.guse.rctsize    = dtInfo->rctBlocks;
		mp->u.guse.nrpt       = dtInfo->rbnBlocks;
		mp->u.guse.nrct       = dtInfo->rctCopies;

		// Load drive flags
		if (dtInfo->Flags & MDF_RDONLY)
			mp->u.guse.untFlags |= MUF_WRPRTH;
		if (dtInfo->Flags & MDF_RMVBL)
			mp->u.guse.untFlags |= MUF_RMVBL;

	} else {
		// Non-existant Unit
#ifdef DEBUG
		dbg_Printf("MSCP: Drive %d: Non-Existant\n", mp->idUnit);
#endif /* DEBUG */
		mp->Status = MST_OFFLINE;
		return;
	}

	// Done - Set End Message Flag
	mp->opCode |= MOP_END;
	mp->Status =  MST_SUCCESS;

}

// 04 (004) - Set Controller Characteristics Command
void mscp_CmdSetCtlrFlags(void *dptr, MSCP_PACKET *mp)
{
	MSCP_DEVICE *uda = (MSCP_DEVICE *)dptr;

	// Set Controller Flags
	uda->cFlags = mp->u.sccc.Flags;
//	if (uda->cFlags & MCF_ATTN)
//		// Send all available messages for existing drives.
//		uda->Flags |= UDA_SENDALL;

	// Return controller characteristics information
	mp->u.scce.Timeout   = 120;
	mp->u.scce.idCtlr[0] = 0;
	mp->u.scce.idCtlr[1] = 0;
	mp->u.scce.idCtlr[2] = 0;
	mp->u.scce.idCtlr[3] = 19; // RQDX3 Model #
	mp->u.scce.snVolume  = 0;

	// Done - Set End Message Flag
	mp->msgType = 15;
	mp->opCode  |= MOP_END;
	mp->Status  =  MST_SUCCESS;
}

// 09 (011) - Online Command
void mscp_CmdOnline(void *dptr, MSCP_PACKET *mp)
{
	MSCP_DEVICE *uda   = (MSCP_DEVICE *)dptr;
	uint32     idUnit = mp->idUnit;
	MSCP_UNIT   *uptr;
	MSCP_DTYPE *dtInfo;

	if ((idUnit < uda->nUnits) && (uda->Units[idUnit].Flags & MDF_EXIST)) {
		uptr   = &uda->Units[idUnit];
		dtInfo = uptr->dtInfo;

		// Put some information on online command packet.
		mp->u.onle.idMedia = dtInfo->idMedia;
		mp->u.onle.szUnit  = dtInfo->lbnUser;
		if (dtInfo->Flags & MDF_RDONLY)
			mp->u.onle.untFlags |= MUF_WRPRTH;
		if (dtInfo->Flags & MDF_RMVBL)
			mp->u.onle.untFlags |= MUF_RMVBL;

	} else {
		// Non-existant Unit
#ifdef DEBUG
		dbg_Printf("MSCP: Drive %d: Non-Existant\n", mp->idUnit);
#endif /* DEBUG */
		mp->Status = MST_OFFLINE;
		return;
	}

	// Done - Set End Message Flag
	mp->opCode |= MOP_END;
	mp->Status =  MST_SUCCESS;
}

// 21 (041) - Read Command
void mscp_CmdRead(void *dptr, MSCP_PACKET *mp)
{
	MSCP_DEVICE *uda   = (MSCP_DEVICE *)dptr;
	QBA_CALL   *call  = uda->Callback;
	uint32     idUnit = mp->idUnit;
	MSCP_UNIT   *drv;
	int32      lbnBlock, ioAddr, cntBytes, szRead;
	uint8      buf[512];

	// Check drive is existing or not first.
	if ((idUnit >= uda->nUnits) &&
	    ((uda->Units[idUnit].Flags & MDF_EXIST) == 0)) {
		// Non-Existant Device

#ifdef DEBUG
		dbg_Printf("MSCP: Drive %d: Non-Existant\n", mp->idUnit);
#endif /* DEBUG */

		mp->Status = MST_OFFLINE;
		mp->opCode = MOP_END;
	}

	// Initially Data Transfer..
	drv      = &uda->Units[idUnit];
	lbnBlock = mp->u.xfer.lbnBlock;
	ioAddr   = mp->u.xfer.buffer[0];
	cntBytes = mp->u.xfer.cntBytes;
	while (cntBytes) {
		szRead = cntBytes > 512 ? 512 : cntBytes;

#ifdef DEBUG
		dbg_Printf("MSCP: Drive %d - Logical Block: %d  Size: %d\n",
			drv->idUnit, lbnBlock, szRead);
#endif /* DEBUG */

		lseek(drv->File, lbnBlock * 512, SEEK_SET);
		read(drv->File, buf, szRead);
		call->WriteBlock(uda->System, ioAddr, buf, szRead);

		lbnBlock++;
		ioAddr   += szRead;
		cntBytes -= szRead;
	}

	// Command Done - Set End Message Flag
	mp->Status = MST_SUCCESS;
	mp->opCode |= MOP_END;
}

// 22 (042) - Write Command
void mscp_CmdWrite(void *dptr, MSCP_PACKET *mp)
{
	MSCP_DEVICE *uda = (MSCP_DEVICE *)dptr;
	uint32     idUnit = mp->idUnit;
	MSCP_UNIT   *uptr;

	// Check drive is existing or not first.
	if ((idUnit >= uda->nUnits) &&
	    ((uda->Units[idUnit].Flags & MDF_EXIST) == 0)) {
		// Non-Existant Device

#ifdef DEBUG
		dbg_Printf("MSCP: Drive %d: Non-Existant\n", mp->idUnit);
#endif /* DEBUG */

		mp->Status = MST_OFFLINE;
		mp->opCode = MOP_END;
	} else if (uptr->Flags & (MUF_WRPRTH|MUF_WRPRTS)) {
		// Check device is write-protected or not.
		mp->Status = MST_WRPROT;
		mp->opCode = MOP_END;
	} else {

		// Command Done - Set End Message Flag
		mp->Status = MST_SUCCESS;
		mp->opCode |= MOP_END;
	}
}

// *************************************************************

MSCP_OPCODE mscpOpcodes[];

void mscp_Command(MSCP_DEVICE *uda, uint32 *cdp, uint32 *rdp)
{
	QBA_CALL    *call = uda->Callback;
	MAP_IO      *io   = &uda->ioMap;
	MSCP_PACKET mp;
	uint32      cptr, rptr;
	uint32      size;
	int         idx;

	dbg_Printf("UDA: Command Descriptor  = %08X\n", *cdp);

	cptr = (*cdp & ~(MSCP_OWN|MSCP_INT)) - 4;

	size = call->ReadData(uda->System, cptr, 2);
	call->ReadBlock(uda->System, cptr, (uint8 *)&mp, size + 4);
	if (*cdp & MSCP_INT)
		io->SendInterrupt(io);
	*cdp = (*cdp | MSCP_DONE) & ~MSCP_OWN;
	mscp_DumpPacket2(&mp, cptr + 4);

	for (idx = 0; mscpOpcodes[idx].opCode; idx++) {
		if (mp.opCode == mscpOpcodes[idx].opCode) {
			mscpOpcodes[idx].Execute(uda, &mp);
			break;
		}
	}

	if (mscpOpcodes[idx].opCode == 0) {
		// Invalid Command Status
#ifdef DEBUG
		dbg_Printf("Unknown MSCP Opcode - %02X (%03o)\n", mp.opCode, mp.opCode);
#endif /* DEBUG */
		mp.opCode = MOP_END;
		mp.Status = MST_INVALCMD;
//		uda->sa   |= SA_ER;
	}

	// Send results (message packet) back to host.
	mscp_DumpPacket2(&mp, rptr + 4);
	mscp_SendPacket(uda, &mp, mp.msgLength);

	// Send any available messages if request from host
	if (uda->Flags & UDA_SENDALL) {
		uda->Flags &= ~UDA_SENDALL;
		for (idx = 0; idx < uda->nUnits; idx++) {
			MSCP_UNIT *drv = &uda->Units[idx];
			if (drv->Flags & MDF_AVATTN)
				mscp_SendAvailAttn(drv);
		}
	}
}

void mscp_PollIO(MSCP_DEVICE *uda)
{
	QBA_CALL *call   = uda->Callback;
	uint16   *rptr16 = (uint16 *)call->GetHostAddr(uda->System, uda->rbAddr);
	uint32   *rptr32 = (uint32 *)rptr16;
	int32    resSize = 1 << uda->rLen;
	int32    cmdSize = 1 << uda->cLen;
	uint32   idx1, idx2;

	// Process MSCP packets...
	mscp_DumpRing(uda);

	// Find an available response packet
	for (idx1 = 0; idx1 < resSize; idx1++)
		if (rptr32[idx1] & MSCP_OWN)
			break;			

	// Find a command buffer to process
	for (idx2 = resSize; idx2 < (resSize + cmdSize); idx2++) {
		if (rptr32[idx2] & MSCP_OWN) {
			if (rptr32[idx2] & MSCP_INT)
				rptr16[-2]++;
			mscp_Command(uda, &rptr32[idx2], &rptr32[idx1]);
		}
	}
}

// ************************************************************

int uda_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 size)
{
	MSCP_DEVICE *uda = (MSCP_DEVICE *)dptr; 
	MAP_IO      *io  = &uda->ioMap;
	uint32      reg   = (pAddr - (uda->csrAddr & 0x1FFF)) >> 1;

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UDA: (W) %s (%08o) <= %06o\n", regName[reg], pAddr, data);
#endif /* DEBUG */

	switch (reg) {
		case MSCP_IP: // Initialization and Polling Register
			// Start Initialization
			uda->saFlags   = 0;
			uda->ip        = 0;
			uda->sa        = SA_S1|SA_QB|SA_DI;
			uda->State     = STATE_STEP1;
			uda->intVector = 0;
			break;

		case MSCP_SA: // Status, Address, and Purge Register
			switch (uda->State) {
				case STATE_NONE:
					uda->sa = data;
					break;

				case STATE_WRAP: // Wrap Mode
					uda->sa = data;

					// Send an interrupt if vector is set.
					if (uda->intVector)
						io->SendInterrupt(io);
					break;

				case STATE_STEP1: // Initialization Step 1
					uda->saFlags   = (data & (SA_WRAP|SA_IE));
					uda->cLen      = (data >> SA_P_CLEN) & SA_LEN;
					uda->rLen      = (data >> SA_P_RLEN) & SA_LEN;
					uda->intVector = (data & SA_VECTOR) << 2;

#ifdef DEBUG
//					if (dbg_Check(DBG_IOREGS)) {
						dbg_Printf("UDA: Initialization Step 1\n");
						dbg_Printf("UDA: Wrap Mode: %s  Interrupt: %s  Vector: %04X (%03o)\n",
							((uda->saFlags & SA_WRAP) ? "Yes" : "No"),
							((uda->saFlags & SA_IE) ? "Yes" : "No"),
							uda->intVector, uda->intVector);
						dbg_Printf("UDA: Descriptor: Command: %d (%d slots)  Response: %d (%d slots)\n",
							uda->cLen, (1u << uda->cLen),
							uda->rLen, (1u << uda->rLen));
//					}
#endif /* DEBUG */

					if (uda->saFlags & SA_WRAP) {
						uda->sa    = data;
						uda->State = STATE_WRAP;
					} else {
						uda->sa    = (SA_S2|SA_IE) | (uda->cLen << 3) | uda->rLen;
						uda->State = STATE_STEP2;
					}

					// Set new vector and send an interrupt.
					if (uda->intVector) {
						dbg_SetMode(DBG_TRACE|DBG_DATA|DBG_INTERRUPT|DBG_PAGEFAULT);
						io->SetVector(io, uda->intVector);
						io->SendInterrupt(io);
					}
					break;

				case STATE_STEP2: // Initialization Step 2
					uda->saFlags |= (data & SA_PI);
					uda->rbAddr  = data & ~1;
					uda->sa = SA_S3 | (uda->saFlags & SA_IE) |
						(uda->intVector >> 2);
					uda->State = STATE_STEP3;

#ifdef DEBUG
//					if (dbg_Check(DBG_IOREGS)) {
						dbg_Printf("UDA: Initialization Step 2\n");
						dbg_Printf("UDA: PI: %s  Ringbase Address (low): %04X\n",
							((data & SA_PI) ? "Yes" : "No"), uda->rbAddr);

//					}
#endif /* DEBUG */

					// Send Interrupt if Vector is set.
					if (uda->intVector)
						io->SendInterrupt(io);
					break;

				case STATE_STEP3: // Initialization Step 3
					uda->saFlags |= (data & SA_PP);
					uda->rbAddr  |= ((data & 077777) << 16);
					uda->sa = SA_S4 | UDA_UVER;
					uda->State = STATE_STEP4;

#ifdef DEBUG
//					if (dbg_Check(DBG_IOREGS)) {
						dbg_Printf("UDA: Initialization Step 3\n");
						dbg_Printf("UDA: PP: %s  Ringbase Addr (high): %04X\n",
							((data & SA_PP) ? "Yes" : "No"), uda->rbAddr >> 16);
//					}
#endif /* DEBUG */

					// Send Interrupt if Vector is set.
					if (uda->intVector)
						io->SendInterrupt(io);
					break;

				case STATE_STEP4: // Initialization Step 4
					uda->burst = (data >> 2) & 077;
					uda->lf    = (data & 2);
					uda->go    = (data & 1);

					// if GO bit was set, controller is now ready.
					if (data & SA_GO) {
						uda->curRes = 0; // Starting Response #0
						uda->sa     = 0;
						uda->State  = STATE_READY;
					}

					// If LF bit was set, send a Last Fail message.
//					if (data & SA_LF)
//						mscp_SendLastFail(uda, 0);

#ifdef DEBUG
//					if (dbg_Check(DBG_IOREGS)) {
						dbg_Printf("UDA: Initialization Step 4\n");
						dbg_Printf("UDA: GO: %s  LF: %s  Burst: %02X (%02o)\n",
							((data & SA_GO) ? "Yes" : "No"),
							((data & SA_LF) ? "Yes" : "No"),
							uda->burst);

						if (data & SA_GO) {
							dbg_Printf("UDA: All setup done.  Ready.\n");
							dbg_Printf("UDA: Ring=%08X C=%d R=%d Vector=%o\n",
								uda->rbAddr, uda->cLen, uda->rLen, uda->intVector);
						}

//					}
#endif /* DEBUG */

					// Send Interrupt if Vector is set.
					if (uda->intVector)
						io->SendInterrupt(io);
					break;

				default:
#ifdef DEBUG
//					if (dbg_Check(DBG_IOREGS))
						dbg_Printf("UDA: (W) Undefined State=%d PA=%08o Data=%06o\n",
							uda->State, pAddr, data);
#endif /* DEBUG */
			}
	}

	return QBA_OK;
}

int uda_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 size)
{
	MSCP_DEVICE  *uda = (MSCP_DEVICE *)dptr; 
	uint32      reg  = (pAddr - (uda->csrAddr & 0x1FFF)) >> 1;

	switch (reg) {
		case MSCP_IP: // Initialization and Polling Register
			if (uda->State == STATE_READY)
				mscp_PollIO(uda);
			*data = uda->ip;
			break;

		case MSCP_SA: // Status, Address, and Purge Register
			*data = uda->sa;
			break;

		default:
#ifdef DEBUG
//			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("UDA: (R) Undefined Register %d - PA=%08o Data=%06o\n",
					reg, pAddr, *data);
#endif /* DEBUG */
			return QBA_NXM;
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("UDA: (R) %s (%08o) => %06o\n", regName[reg], pAddr, *data);
#endif /* DEBUG */

	return QBA_OK;
}

// ************************************************************

DEVINFO uda_Device;

void *uda_Create(MAP_DEVICE *nmap, MAP_DEVICE *map, int argc, char **argv)
{
	MSCP_DEVICE *uda = NULL;
	MSCP_UNIT   *uptr;
	MAP_IO      *io;
	uint32      nDevices;
	int         unit, idx;

	if (nmap == NULL) {
		if (uda = (MSCP_DEVICE *)calloc(1, sizeof(MSCP_DEVICE))) {
			uda->devName    = map->devName;
			uda->emuName    = MSCP_NAME;
			uda->emuVersion = MSCP_VERSION;
			// Controller Flags
			uda->Flags     = 0;
			uda->State     = STATE_NONE; // Power-Up - Initially Zero
			uda->csrAddr   = UDA_IOADDR;
			uda->intVector = 0;

			// Create drives on depending controller.
			uda->nUnits = 4;
			uda->Units  =
				(MSCP_UNIT *)calloc(uda->nUnits, sizeof(MSCP_UNIT));
			for (idx = 0; idx < uda->nUnits; idx++) {
				uptr = &uda->Units[idx];
				uptr->idUnit    = idx;
				uptr->udaDevice = uda;
			}

			// Device Registers
			uda->cFlags = 0;
			uda->ip     = 0;
			uda->sa     = 0;

			map->dtName     = uda_Device.dtName;
			map->emuName    = uda_Device.emuName;
			map->emuVersion = uda_Device.emuVersion;
			map->devInfo    = &uda_Device;
			map->Device     = uda;

			// Set up I/O Entry
			io = &uda->ioMap;
			io->devName   = uda->devName;
			io->Device    = uda;
			io->Descrip   = "MSCP Controller";
			io->csrAddr   = UDA_IOADDR;
			io->nRegs     = 2;
			io->intLevel  = UDA_INT;
			io->intMask   = (1u << UDA_INT);
			io->intVector = 0;
			io->ReadIO    = uda_ReadIO;
			io->WriteIO   = uda_WriteIO;
		}

		if (uda) {
			return uda;
		} else {
			printf("UDA: *** Can't create MSCP controller ***\n");
			if (uda)
				free(uda);
			return NULL;
		}
	} else {
		uda = (MSCP_DEVICE *)map->Device;

		unit = GetDeviceUnit(nmap->devName);
		if (unit < uda->nUnits) {

			uptr = &uda->Units[unit];

			// Check if unit is existing or not first.
			if (uptr->Flags & MDF_EXIST) {
				printf("%s: Already created on %s:\n",
					nmap->devName, map->devName);
				return NULL;
			}

			for (idx = 0; mscp_dtList[idx].dtName; idx++)
				if (!strcasecmp(argv[2], mscp_dtList[idx].dtName))
					break;

			if (mscp_dtList[idx].dtName == NULL) {
				printf("%s: No such device type on %s:\n",
					argv[2], nmap->devName);
				return NULL;
			}

			uptr->devName   = nmap->devName;
			uptr->dtName    = mscp_dtList[idx].dtName;
			uptr->dtDescrip = mscp_dtList[idx].dtDescrip;
			uptr->idMedia   = mscp_dtList[idx].idMedia;
			uptr->Flags     = mscp_dtList[idx].Flags | MDF_EXIST;
			uptr->dtInfo    = &mscp_dtList[idx];

			nmap->idUnit     = uptr->idUnit;
			nmap->dtName     = uptr->dtName;
			nmap->emuName    = uptr->dtDescrip;
			nmap->emuVersion = "";
			nmap->devInfo    = &uda_Device;
			nmap->Device     = uda;

			// Successful - Inform operator that.
			printf("Created %s on %s: - %s %s\n",
				nmap->dtName, nmap->devName, nmap->emuName, nmap->emuVersion);
		} else {
			printf("%s: No such drive on %s: - (Only %d drives)\n",
				nmap->devName, map->devName, uda->nUnits);
			return NULL;
		}

		// Send MSCP drive information.
		return uptr;
	}
}

int uda_Reset(void *);
int uda_Setup(void *dptr, void *dev, void *sys, QBA_CALL *call)
{
	MSCP_DEVICE *uda = (MSCP_DEVICE *)dptr;

	uda->System      = sys;
	uda->Device      = dev;
	uda->Callback    = call;

	// Assign that registers to QBA's I/O Space.
	call->SetMap(uda->Device, &uda->ioMap);

	uda_Reset(dptr);
}

int uda_Reset(void *dptr)
{
	MSCP_DEVICE *uda = (MSCP_DEVICE *)dptr;

	uda->saFlags = 0;
	uda->ip      = 0;
	uda->sa      = SA_S1|SA_QB|SA_DI;
	uda->State   = STATE_STEP1;

	return EMU_OK;
}

int uda_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	MSCP_DEVICE *uda  = (MSCP_DEVICE *)map->Device;
	MSCP_UNIT   *drv = &uda->Units[map->idUnit];

	if ((drv->File = open(argv[2], O_RDONLY)) < 0) {
		printf("%s: file '%s' not attached - %s.\n",
			drv->devName, argv[2], strerror(errno));
		return EMU_OPENERR;
	}

	drv->Flags |= MDF_AVATTN;

	// Tell operator that and save it.
	printf("%s: file '%s' attached.\n", drv->devName, argv[2]);
	if (drv->fileName = (char *)malloc(strlen(argv[2])+1))
		strcpy(drv->fileName, argv[2]);
	else {
		printf("%s: Can't assign the name of '%s' - Not enough memory.\n",
			map->devName, argv[2]);
	}

	return EMU_OK;
}

int uda_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	MSCP_DEVICE *uda  = (MSCP_DEVICE *)map->Device;
	MSCP_UNIT   *drv = &uda->Units[map->idUnit];

	if (drv->File) {
		close(drv->File);
		drv->Flags |= MDF_AVATTN;
		drv->File  =  0;

		// Tell operator that.
		printf("%s: file '%s' detached.\n", drv->devName,
			drv->fileName ? drv->fileName : "<Unknown filename>");

		// Free the allocation of filename if available.
		if (drv->fileName)
			free(drv->fileName);
		drv->fileName = NULL;
	} else
		printf("%s: Already detached.\n", drv->devName);

	return EMU_OK;
}

// Show information about MSCP controller and drives
int uda_Info(MAP_DEVICE *map, int argc, char **argv)
{
	MSCP_DEVICE *uda = (MSCP_DEVICE *)map->Device;
	MSCP_UNIT   *uptr;
	int        idx, count = 0;

	// Information about MSCP controller
	printf("\nDevice:           %s  Type: %s  State: %s\n",
		map->devName, map->dtName, stateName[uda->State]);
	printf("CSR Address:      %06X (%08o)\n", uda->csrAddr, uda->csrAddr);
	printf("Interrupt Vector: %03X (%03o)\n", uda->intVector, uda->intVector);
	printf("Ringbase Address: %06X (%08o)\n", uda->rbAddr, uda->rbAddr);
	printf("Descriptors:      %d Command Slots, %d Response Slots\n\n",
		(1u << uda->cLen), (1u << uda->rLen));

	// Information about MSCP drives
	for (idx = 0; idx < uda->nUnits; idx++) {
		uptr = &uda->Units[idx];
		if (uptr->Flags & MDF_EXIST) {
			if (count++ == 0) {
				printf(
					"Device   Type     Media ID Description\n"
					"------   ----     -------- -----------\n"
				);
			}
			printf("%-8s %-8s %-8s %s\n",
				uptr->devName, uptr->dtName, mscp_GetMediaID(uptr->idMedia),
				uptr->dtDescrip);
		}
	}

	if (count)
		printf("\nTotal %d MSCP drives.\n\n", count);
	else
		printf("\nNo MSCP drives.\n\n");

	return EMU_OK;
}

DEVINFO uda_Device =
{
	MSCP_DTNAME,   // Device Type Name
	MSCP_NAME,     // Emulator Name
	MSCP_VERSION,  // Emulator Version
	NULL,          // Listing of Devices
	DF_USE,        // Device Flags
	DT_DEVICE,     // Device Type

	NULL, NULL, NULL,

	uda_Create,    // Create Routine
	NULL,          // Delete Routine
	uda_Setup,     // Setup Routine
	uda_Reset,     // Reset Routine
	uda_Attach,    // Attach Routine  - Not Used
	uda_Detach,    // Detach Routine  - Not Used
	uda_Info,      // Info Routine
	NULL           // Execute Routine - Not Used
};

// Listing of MSCP Controllers
MSCP_TYPE   mscpTypes[] =
{
	{ "RQDX3",  19,  4 },
	{ NULL }
};

MSCP_OPCODE mscpOpcodes[] =
{
	{ MOP_GETUNITST, mscp_CmdGetUnitStatus }, // Get Unit Status
	{ MOP_SETCTLRC,  mscp_CmdSetCtlrFlags  }, // Set Controller Characteristics
	{ MOP_ONLINE,    mscp_CmdOnline        }, // Online Command
	{ MOP_READ,      mscp_CmdRead          }, // Read Command
	{ 0,             NULL }
};
