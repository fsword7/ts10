// mba_rp.h - MASSBUS disk pack drive definitions
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
// See README for copyright notice.
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

// Massbus Address Index for drives
#define nRPCS1 000 // (R/W) Control and Status Register #1
#define nRPDS  001 // (R/W) Drive Status Register
#define nRPER1 002 // (R/W) Error Register #1
#define nRPMR1 003 // (R/W) Maintenance Register #1
#define nRPAS  004 // (R/W) Attention Summary Register (Pesudo)
#define nRPDA  005 // (R/W) Desired Sector/Track Address Register
#define nRPDT  006 // (R)   Drive Type Register
#define nRPLA  007 // (R)   Look Ahead Register
#define nRPSN  010 // (R)   Serial Number Register
#define nRPOF  011 // (R/W) Offset Register
#define nRPDC  012 // (R/W) Desired Cylinder Address Register
#define nRPCC  013 // (R)   Current Cylinder Address Register
#define nRPER2 014 // (R)   Error Register #2
#define nRPER3 015 // (R/W) Error Register #3
#define nRPEC1 016 // (R)   ECC Position Register
#define nRPEC2 017 // (R)   ECC Pattern Register

// Massbus Address Index for drives
#define RPCS1  MBA_REG(nRPCS1) // (R/W) Control and Status Register #1
#define RPDS   MBA_REG(nRPDS)  // (R/W) Drive Status Register
#define RPER1  MBA_REG(nRPER1) // (R/W) Error Register #1
#define RPMR1  MBA_REG(nRPMR1) // (R/W) Maintenance Register #1
#define RPAS   MBA_REG(nRPAS)  // (R/W) Attention Summary Register (Pesudo)
#define RPDA   MBA_REG(nRPDA)  // (R/W) Desired Sector/Track Address Register
#define RPDT   MBA_REG(nRPDT)  // (R)   Drive Type Register
#define RPLA   MBA_REG(nRPLA)  // (R)   Look Ahead Register
#define RPSN   MBA_REG(nRPSN)  // (R)   Serial Number Register
#define RPOF   MBA_REG(nRPOF)  // (R/W) Offset Register
#define RPDC   MBA_REG(nRPDC)  // (R/W) Desired Cylinder Address Register
#define RPCC   MBA_REG(nRPCC)  // (R)   Current Cylinder Address Register
#define RPER2  MBA_REG(nRPER2) // (R)   Error Register #2
#define RPER3  MBA_REG(nRPER3) // (R/W) Error Register #3
#define RPEC1  MBA_REG(nRPEC1) // (R)   ECC Position Register
#define RPEC2  MBA_REG(nRPEC2) // (R)   ECC Pattern Register

// RPCS1 - Control and Status 1 Register
#define RPCS1_SC     0100000 /* (R)    Special Condition */
#define RPCS1_TRE    0040000 /* (R/W)  Transfer Error */
#define RPCS1_MCPE   0020000 /* (R)    Mass I/O Bus Control Parity Error */
#define RPCS1_X      0010000 /* (R)   *Reserved for use by the drive */
#define RPCS1_DVA    0004000 /* (R)   *Drive Available */
#define RPCS1_PSEL   0002000 /* (R/W)  Port Select */
#define RPCS1_A17    0001000 /* (R/W)  A17 Bus Address Extension Bit */
#define RPCS1_A16    0000400 /* (R/W)  A16 Bus Address Extension Bit */
#define RPCS1_RDY    0000200 /* (R)    Ready */
#define RPCS1_IE     0000100 /* (R/W)  Interrupt Enable */
#define RPCS1_FUNC   0000076 /* (R/W) *Function Code (F0:F4) */
#define RPCS1_GO     0000001 /* (R/W) *Go */

#define RPCS1_BAE    0001400 /* (R/W)  A16/A17 BAE bits */

#define RPCS1_DRV    0014077 /* (R/W)  Drive Mask */
#define RPCS1_CTLR   0163700 /* (R/W)  Controller Mask */
#define RPCS1_MBZ    0010000 /*        Must Be Zeros */
#define RPCS1_WR     0000077 // (W)    Write Mask

// RPDA - Desired Sector and Track Address Register
#define RPDA_TA      0037400 // (R/W)  Track Address
#define RPDA_SA      0000077 // (R/W)  Sector Address
#define RPDA_MASK    0037477 //        Track/Sector Mask
#define RPDA_MBZ     0000340 //        Must be Zeros

#define RPDA_P_TA    8       //        Track's Position

// RPDS - Drive Status Register
#define RPDS_ATA     0100000 // (R)    Attention Active
#define RPDS_ERR     0040000 // (R)    Error
#define RPDS_PIP     0020000 // (R)    Positioning in Progress
#define RPDS_MOL     0010000 // (R)    Medium On-Line
#define RPDS_WRL     0004000 // (R)    Write Lock
#define RPDS_LST     0002000 // (R)    Last Sector Transferred
#define RPDS_PGM     0001000 // (R)    Programmable
#define RPDS_DPR     0000400 // (R)    Drive Present
#define RPDS_DRY     0000200 // (R)    Drive Ready
#define RPDS_VV      0000100 // (R)    Volume Valid
#define RPDS_DE1     0000040 // (R)    Difference Equals 1
#define RPDS_DL64    0000020 // (R)    Difference Less Than 64
#define RPDS_GRV     0000010 // (R)    Go Reverse
#define RPDS_DIGB    0000004 // (R)    Drive to Inner Guard Band
#define RPDS_DF20    0000002 // (R)    Drive Forward 20 inch/sec
#define RPDS_DF5     0000001 // (R)    Drive Forward 5 inch/sec

// RPER1 - Error Register #1
#define RPER1_DCK    0100000 // (R)    Data Check Error
#define RPER1_UNS    0040000 // (R)    Unsafe
#define RPER1_OPI    0020000 // (R)    Operation Incomplete
#define RPER1_DTE    0010000 // (R)    Drive Timing Error
#define RPER1_WLE    0004000 // (R)    Write Lock Error
#define RPER1_IAE    0002000 // (R)    Invalid Address Error
#define RPER1_AOE    0001000 // (R)    Address Overflow Error
#define RPER1_HCRC   0000400 // (R)    Header CRC Error
#define RPER1_HCE    0000200 // (R)    Header Compare Error
#define RPER1_ECH    0000100 // (R)    ECC Hard Error
#define RPER1_WCF    0000040 // (R/W)  Write Clock Fail
#define RPER1_FER    0000020 // (R)    Format Error
#define RPER1_PAR    0000010 // (R)    Parity Error
#define RPER1_RMR    0000004 // (R/W)  Register Modification Refused
#define RPER1_ILR    0000002 // (R/W)  Illegal Register
#define RPER1_ILF    0000001 // (R/W)  Illegal Function

// RPLA - Look-Ahead Register
#define RPLA_SC      0003700 // (R)    Sector Count
#define RPLA_EXT     0000060 // (R)    Encoded Extension Field

#define RPLA_P_SC    6

// RPMR - Maintenance Register

// RPDT - Drive Type Register
#define RPDT_MOH    0020000 // (R)    Moving Head
#define RPDT_DRQ    0004000 // (R)    Drive Request Required
#define RPDT_DT     0000777 // (R)    Drive Type

// RPSN - Serial Number Register
#define RPSN_MASK   0177777 // (R)    Serial Number

// RPOF - Offset Register
#define RPOF_SCG    0100000 // (R/W)  Sign Change
#define RPOF_FMT22  0010000 // (R/W)  Format 22 (16/18-bit Word)
#define RPOF_ECI    0004000 // (R/W)  Error Correction Code Inhibit
#define RPOF_HCI    0002000 // (R/W)  Header Compare Inhibit
#define RPOF_OFS    0000377 // (R/W)  Offset Information

// RPDC - Desired Cylinder Register
#define RPDC_MASK   0001777 // (R/W)  Desired Cyclinder
#define RPDC_MBZ    0176000 //        Must be Zeros

// RPCC - Current Cylinder Register
#define RPCC_MASK   0001777 // (R)    Current Cylinder
#define RPCC_MBZ    0176000 //        Must be Zeros

// RPER2 - Error Register #2
#define RPER2_ACU   0100000 // (R/W)  AC Unsafe
#define RPER2_X     0040000 //        Not Used
#define RPER2_PLU   0020000 // (R/W)  PLO Unsafe
#define RPER2_30VU  0010000 // (R/W)  30 Volts Unsafe
#define RPER2_IXE   0004000 // (R/W)  Index Error
#define RPER2_NHS   0002000 // (R/W)  No Head Selection
#define RPER2_MHS   0001000 // (R/W)  Multiple Head Selection
#define RPER2_WRU   0000400 // (R/W)  Write Ready Unsafe
#define RPER2_FEN   0000200 // (R/W)  Failsafe Enabled
#define RPER2_TUF   0000100 // (R/W)  Transitions Unsafe
#define RPER2_TDF   0000040 // (R/W)  Transitions Detector Failure
#define RPER2_MSE   0000020 // (R/W)  Motor Sequence Error
#define RPER2_CSU   0000010 // (R/W)  Current Switch Unsafe
#define RPER2_WSU   0000004 // (R/W)  Write Select Unsafe
#define RPER2_CSF   0000002 // (R/W)  Current Sink Failure
#define RPER2_WCU   0000001 // (R/W)  Write Current Unsafe

// RPER3 - Error Register #3
#define RPER3_OCYL  0100000 // (R/W)  Off Cylinder
#define RPER3_SKI   0040000 // (R/W)  Seek Incomplete
#define RPER3_DCL   0000100 // (R/W)  DC Low
#define RPER3_ACL   0000040 // (R/W)  AC Low
#define RPER3_UWR   0000010 // (R/W)  Any Unsafe Except Read/Write
#define RPER3_VUF   0000002 // (R/W)  Velocity Unsafe
#define RPER3_PSU   0000001 // (R/W)  Pack Speed Unsafe
#define RPER3_MBZ   0037624 //        Must Be Zeros

// RPEC1 - ECC Register #1

// RPEC2 - ECC Register #2

#define RP_BLKSZ16  512 // 256 16-Bit Words per Block (in bytes)
//#define RP_BLKSZ18  576 // 256 18-Bit Words per Block (in bytes)
#define RP_BLKSZ18  (128 * 5) // 128 36-Bit Words per Block
#define RP_BLKSZ    256 // 256 16/18-bit Words per Block (in words)

// Get C/T/S values from one of drive registers
#define GetCylinder(c) (c)
#define GetTrack(t)    (((t) & RPDA_TA) >> 8)
#define GetSector(s)   ((s) & RPDA_SA)
#define GetDiskAddr(c, ts, d) \
	((((GetCylinder(c) * (d)->Tracks) + \
	    GetTrack(ts)) * (d)->Sectors) + \
	    GetSector(ts))


typedef struct rp_Drive RP_DRIVE;

struct rp_Drive {
	UNIT      Unit;      // Unit Header Information

	UQ_BOOT   Boot;      // Boot device
	VDK_DISK  dpDisk;    // Disk Descriptor
	uint32    Flags;     // Drive Flags
	MBA_DRIVE *mbaDrive; // MASSBUS Drive
	uint32    idUnit;    // Unit ID
};
