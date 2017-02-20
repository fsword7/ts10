// rp.h - RP MASSBUS-based disk drive definitions
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

/* Unibus Address Index */
/*   = register in controller */
/* ^ = register in both drive and controller */
/* * = register in drive */
#define RPCS1 (000 >> 1) /* (R/W) ^Control and Status Register #1 */
#define RPWC  (002 >> 1) /* (R/W)  Word Count Register */
#define RPBA  (004 >> 1) /* (R/W)  Bus Address Register */
#define RPDA  (006 >> 1) /* (R/W) *Desired Sector/Track Address Register */
#define RPCS2 (010 >> 1) /* (R/W)  Control and Status Register #2 */
#define RPDS  (012 >> 1) /* (R/W) *Drive Status Register */
#define RPER1 (014 >> 1) /* (R/W) *Error Register #1 */
#define RPAS  (016 >> 1) /* (R/W) *Attention Summary Register */
#define RPLA  (020 >> 1) /* (R)   *Look-Ahead Register */
#define RPDB  (022 >> 1) /* (R)    Data Buffer Register */
#define RPMR  (024 >> 1) /* (R/W) *Maintenance Register */
#define RPDT  (026 >> 1) /* (R)   *Drive Type Register */
#define RPSN  (030 >> 1) /* (R)   *Serial Number Register */
#define RPOF  (032 >> 1) /* (R/W) *Offset Register */
#define RPDC  (034 >> 1) /* (R/W) *Desired Cylinder Address Register */
#define RPCC  (036 >> 1) /* (R/W) *Current Cylinder Address Register */
#define RPER2 (040 >> 1) /* (R)   *Error Register #2 */
#define RPER3 (042 >> 1) /* (R/W) *Error Register #3 */
#define RPEC1 (044 >> 1) /* (R)   *ECC Position Register */
#define RPEC2 (046 >> 1) /* (R)   *ECC Pattern Register */
#define RPBAE (050 >> 1) /* (R/W)  Bus Address Extension (21:16) Register */
#define RPCS3 (052 >> 1) /* (R/W)  Control and Status Register #3 */

/* Massbus Address Index for drives */
#define MBA_RPCS1  000
#define MBA_RPDS   001
#define MBA_RPER1  002
#define MBA_RPMR1  003
#define MBA_RPAS   004
#define MBA_RPDA   005
#define MBA_RPDT   006
#define MBA_RPLA   007
#define MBA_RPSN   010
#define MBA_RPOF   011
#define MBA_RPDC   012
#define MBA_RPCC   013
#define MBA_RPER2  014
#define MBA_RMER3  015
#define MBA_RPEC1  016
#define MBA_RPEC2  017

/* RPCS1 - Control and Status 1 Register */
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

// RPWC - Word Count Register
#define RPWC_MASK    0177777 // (R/W)  Word Count

// RPBA - Bus Address Register
#define RPBA_MASK    0177776 // (R/W)  Bus Address
#define RPBA_MBZ     0000001 //        Must be Zeros

// RPDA - Desired Sector and Track Address Register
#define RPDA_TA      0017400 // (R/W)  Track Address
#define RPDA_SA      0000037 // (R/W)  Sector Address
#define RPDA_MASK    0017437 //        Track/Sector Mask
#define RPDA_MBZ     0000340 //        Must be Zeros

#define RPDA_P_TA    8       //        Track's Position

/* RPCS2 - Control and Status 2 Register */
#define RPCS2_DLT    0100000 /* (R)    Data Late */
#define RPCS2_WCE    0040000 /* (R)    Write Check Error */
#define RPCS2_UPE    0020000 /* (R)    Unibus Parity Error */
#define RPCS2_NED    0010000 /* (R)    Non-Existent Drive */
#define RPCS2_NEM    0004000 /* (R)    Non-Existent Memory */
#define RPCS2_PGE    0002000 /* (R)    Program Error */
#define RPCS2_MXF    0001000 /* (R)    Missed Transfer */
#define RPCS2_MDPE   0000400 /* (R)    Mass I/O Bus Data Parity Error */
#define RPCS2_OR     0000200 /* (R)    Output Ready */
#define RPCS2_IR     0000100 /* (R)    Input Ready */
#define RPCS2_CLR    0000040 /* (W)    Controller Clear */
#define RPCS2_PAT    0000020 /* (R/W)  Parity Test */
#define RPCS2_BAI    0000010 /* (R/W)  Unibus Address Increment Inhibit */
#define RPCS2_UNIT   0000007 /* (R/W)  U0-U2 Unit Select (2:0) */

#define RPCS2_RD     0177737 /* (R)    Read Mask */
#define RPCS2_WR     0000077 /* (W)    Write Mask */

/* RPDS - Drive Status Register */
#define RPDS_ATA     0100000 /* (R)    Attention Active */
#define RPDS_ERR     0040000 /* (R)    Error */
#define RPDS_PIP     0020000 /* (R)    Positioning in Progress */
#define RPDS_MOL     0010000 /* (R)    Medium On-Line */
#define RPDS_WRL     0004000 /* (R)    Write Lock */
#define RPDS_LST     0002000 /* (R)    Last Sector Transferred */
#define RPDS_PGM     0001000 /* (R)    Programmable */
#define RPDS_DPR     0000400 /* (R)    Drive Present */
#define RPDS_DRY     0000200 /* (R)    Drive Ready */
#define RPDS_VV      0000100 /* (R)    Volume Valid */
#define RPDS_DE1     0000040 /* (R)    Difference Equals 1 */
#define RPDS_DL64    0000020 /* (R)    Difference Less Than 64 */
#define RPDS_GRV     0000010 /* (R)    Go Reverse */
#define RPDS_DIGB    0000004 /* (R)    Drive to Inner Guard Band */
#define RPDS_DF20    0000002 /* (R)    Drive Forward 20 inch/sec */
#define RPDS_DF5     0000001 /* (R)    Drive Forward 5 inch/sec */

/* RPER1 - Error Register #1 */
#define RPER1_DCK    0100000 /* (R)    Data Check Error */
#define RPER1_UNS    0040000 /* (R)    Unsafe */
#define RPER1_OPI    0020000 /* (R)    Operation Incomplete */
#define RPER1_DTE    0010000 /* (R)    Drive Timing Error */
#define RPER1_WLE    0004000 /* (R)    Write Lock Error */
#define RPER1_IAE    0002000 /* (R)    Invalid Address Error */
#define RPER1_AOE    0001000 /* (R)    Address Overflow Error */
#define RPER1_HCRC   0000400 /* (R)    Header CRC Error */
#define RPER1_HCE    0000200 /* (R)    Header Compare Error */
#define RPER1_ECH    0000100 /* (R)    ECC Hard Error */
#define RPER1_WCF    0000040 /* (R/W)  Write Clock Fail */
#define RPER1_FER    0000020 /* (R)    Format Error */
#define RPER1_PAR    0000010 /* (R)    Parity Error */
#define RPER1_RMR    0000004 /* (R/W)  Register Modification Refused */
#define RPER1_ILR    0000002 /* (R/W)  Illegal Register */
#define RPER1_ILF    0000001 /* (R/W)  Illegal Function */

/* RPLA - Look-Ahead Register */
#define RPLA_SC      0003700 /* (R)    Sector Count */
#define RPLA_EXT     0000060 /* (R)    Encoded Extension Field */

#define RPLA_P_SC    6

/* RPDB - Data Buffer Register */
#define RPDB_MASK    0177777 /* (R/W)  Data Buffer */

/* RPMR - Maintenance Register */

/* RPDT - Drive Type Register */
#define RPDT_MOH    0020000 /* (R)    Moving Head */
#define RPDT_DRQ    0004000 /* (R)    Drive Request Required */
#define RPDT_DT     0000777 /* (R)    Drive Type */

/* RPSN - Serial Number Register */
#define RPSN_MASK   0177777 /* (R)    Serial Number */

// RPOF - Offset Register
#define RPOF_SCG    0100000 // (R/W)  Sign Change
#define RPOF_FMT22  0010000 // (R/W)  Format 22 (16/18-bit Word)
#define RPOF_ECI    0004000 // (R/W)  Error Correction Code Inhibit
#define RPOF_HCI    0002000 // (R/W)  Header Compare Inhibit
#define RPOF_OFS    0000377 // (R/W)  Offset Information

// RPDC - Desired Cylinder Register
#define RPDC_MASK   0001777 // (R/W)  Desired Cyclinder (for RP04/RP05)
#define RPDC_MBZ    0176000 //        Must be Zeros

// RPCC - Current Cylinder Register
#define RPCC_MASK   0001777 // (R)    Current Cylinder (for RP04/RP05)
#define RPCC_MBZ    0176000 //        Must be Zeros

/* RPER2 - Error Register #2 */
#define RPER2_ACU   0100000 /* (R/W)  AC Unsafe */
#define RPER2_X     0040000 /*        Not Used */
#define RPER2_PLU   0020000 /* (R/W)  PLO Unsafe */
#define RPER2_30VU  0010000 /* (R/W)  30 Volts Unsafe */
#define RPER2_IXE   0004000 /* (R/W)  Index Error */
#define RPER2_NHS   0002000 /* (R/W)  No Head Selection */
#define RPER2_MHS   0001000 /* (R/W)  Multiple Head Selection */
#define RPER2_WRU   0000400 /* (R/W)  Write Ready Unsafe */
#define RPER2_FEN   0000200 /* (R/W)  Failsafe Enabled */
#define RPER2_TUF   0000100 /* (R/W)  Transitions Unsafe */
#define RPER2_TDF   0000040 /* (R/W)  Transitions Detector Failure */
#define RPER2_MSE   0000020 /* (R/W)  Motor Sequence Error */
#define RPER2_CSU   0000010 /* (R/W)  Current Switch Unsafe */
#define RPER2_WSU   0000004 /* (R/W)  Write Select Unsafe */
#define RPER2_CSF   0000002 /* (R/W)  Current Sink Failure */
#define RPER2_WCU   0000001 /* (R/W)  Write Current Unsafe */

/* RPER3 - Error Register #3 */
#define RPER3_OCYL  0100000 /* (R/W)  Off Cylinder */
#define RPER3_SKI   0040000 /* (R/W)  Seek Incomplete */
#define RPER3_DCL   0000100 /* (R/W)  DC Low */
#define RPER3_ACL   0000040 /* (R/W)  AC Low */
#define RPER3_UWR   0000010 /* (R/W)  Any Unsafe Except Read/Write */
#define RPER3_VUF   0000002 /* (R/W)  Velocity Unsafe */
#define RPER3_PSU   0000001 /* (R/W)  Pack Speed Unsafe */
#define RPER3_MBZ   0037624 /*        Must Be Zeros */

/* RPEC1 - ECC Register #1 */

/* RPEC2 - ECC Register #2 */

/* RPBAE - Bus Address Extension Register */
#define RPBAE_MASK  0000077 /* (R/W)  Bus Address Extension (A21-A16) */
#define RPBAE_MBZ   0177700 /*        Must be Zeros */

/* RPCS3 - Control and Status 3 Register */
#define RPCS3_APE   0100000 /* (R)    Address Parity Error */
#define RPCS3_DFEOW 0040000 /* (R)    Data Parity Error - Odd Word */
#define RPCS3_DFEEW 0020000 /* (R)    Data Parity Error - Even Word */
#define RPCS3_WCEOW 0010000 /* (R)    Write Check Error - Odd Word */
#define RPCS3_WCEEW 0004000 /* (R)    Write Check Error - Even Word */
#define RPCS3_DBL   0002000 /* (R)    Double Word */
#define RPCS3_IE    0000100 /* (R/W)  Interrupt Enable */
#define RPCS3_IPCK  0000017 /* (R/W)  Invert Parity Check (3:0) */

#define RPCS3_RD    0176117 /* (R)    Read Mask */
#define RPCS3_WR    0000117 /* (W)    Write Mask */
#define RPCS3_MBZ   0001660 /*        Must Be Zeros */

// Maximum number of units per disk controller
#define RP_MAXUNITS 8

#define RP_BLKSZ16  512 // 256 16-Bit Words per Block (in bytes)
//#define RP_BLKSZ18  576 // 256 18-Bit Words per Block (in bytes)
#define RP_BLKSZ18  (128 * 5) // 128 36-Bit Words per Block

// Get C/T/S values from one of drive registers
#define GetCylinder(c) (c)
#define GetTrack(t)    (((t) & RPDA_TA) >> 8)
#define GetSector(s)   ((s) & RPDA_SA)
#define GetDiskAddr(c, ts, d) \
	((((GetCylinder(c) * (d)->Tracks) + \
	    GetTrack(ts)) * (d)->Sectors) + \
	    GetSector(ts))

// TOPS-10 HOM/BAT Blocks
#define FORMWORD36(lh, rh) (((lh & 0777777) << 18) | (rh & 0777777))
#define FORMCHS18(c, h, s) (((c & 0377) << 10) | ((h & 037) << 5) | (s & 037))

// RP Disk Drive Unit
typedef struct {
	uint16 rpcs1; // Control and Status Register #1
//	uint16 rpwc;  // Word Count Register
//	uint16 rpba;  // Bus Address Register
	uint16 rpda;  // Desired Sector/Track Address Register
//	uint16 rpcs2; // Control and Status Register #2
	uint16 rpds;  // Drive Status Register
	uint16 rper1; // Error Register #1
//	uint16 rpas;  // Attention Summary Register
	uint16 rpla;  // Look-Ahead Register
//	uint16 rpdb;  // Data Buffer Register
	uint16 rpmr;  // Maintenance Register
	uint16 rpdt;  // Drive Type Register
	uint16 rpsn;  // Serial Number Register
	uint16 rpof;  // Offset Register
	uint16 rpdc;  // Desired Cylinder Address Register
	uint16 rpcc;  // Current Cylinder Address Register
	uint16 rper2; // Error Register #2
	uint16 rper3; // Error Register #3
	uint16 rpec1; // ECC Position Register
	uint16 rpec2; // ECC Pattern Register
//	uint16 rpbae; // Bus Address Extension Register
//	uint16 rpcs3; // Control and Status Register #3
} RPUNIT;
