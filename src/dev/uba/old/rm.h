// rm.h - RM MASSBUS-based disk drive definitions
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

/* Unibus Address Index */
/*   = register in controller */
/* ^ = register in both drive and controller */
/* * = register in drive */
#define RMCS1 (000 >> 1) /* (R/W) ^Control and Status 1 Register */
#define RMWC  (002 >> 1) /* (R/W)  Word Count Register */
#define RMBA  (004 >> 1) /* (R/W)  Bus Address Register */
#define RMDA  (006 >> 1) /* (R/W) *Desired Sector/Track Address Register */
#define RMCS2 (010 >> 1) /* (R/W)  Control and Status 2 Register */
#define RMDS  (012 >> 1) /* (R/W) *Drive Status Register */
#define RMER1 (014 >> 1) /* (R/W) *Error 1 Register */
#define RMAS  (016 >> 1) /* (R/W) *Attention Summary Register */
#define RMLA  (020 >> 1) /* (R)   *Look-Ahead Register */
#define RMDB  (022 >> 1) /* (R)    Data Buffer Register */
#define RMMR1 (024 >> 1) /* (R/W) *Maintenance 1 Register */
#define RMDT  (026 >> 1) /* (R)   *Drive Type Register */
#define RMSN  (030 >> 1) /* (R)   *Serial Number Register */
#define RMOF  (032 >> 1) /* (R/W) *Offset Register */
#define RMDC  (034 >> 1) /* (R/W) *Desired Cyclinder */
#define RMHR  (036 >> 1) /* (R/W) *Holding (Not used) */
#define RMMR2 (040 >> 1) /* (R)   *Maintenance 2 Register */
#define RMER2 (042 >> 1) /* (R/W) *Error 2 Register */
#define RMEC1 (044 >> 1) /* (R)   *ECC Position Register */
#define RMEC2 (046 >> 1) /* (R)   *ECC Pattern Register */
#define RMBAE (050 >> 1) /* (R/W)  Bus Address Extension (21:16) Register */
#define RMCS3 (052 >> 1) /* (R/W)  Control and Status 3 Register */

/* Massbus Address Index for drives */
#define MBA_RMCS1  000
#define MBA_RMDS   001
#define MBA_RMER1  002
#define MBA_RMMR1  003
#define MBA_RMAS   004
#define MBA_RMDA   005
#define MBA_RMDT   006
#define MBA_RMLA   007
#define MBA_RMSN   010
#define MBA_RMOF   011
#define MBA_RMDC   012
#define MBA_RMHR   013
#define MBA_RMMR2  014
#define MBA_RMER2  015
#define MBA_RMEC1  016
#define MBA_RMEC2  017

/* RMCS1 - Control and Status 1 Register */
#define RMCS1_SC     0100000 /* (R)    Special Condition */
#define RMCS1_TRE    0040000 /* (R/W)  Transfer Error */
#define RMCS1_MCPE   0020000 /* (R)    Mass I/O Bus Control Parity Error */
#define RMCS1_X      0010000 /* (R)   *Reserved for use by the drive */
#define RMCS1_DVA    0004000 /* (R)   *Drive Available */
#define RMCS1_PSEL   0002000 /* (R/W)  Port Select */
#define RMCS1_A17    0001000 /* (R/W)  A17 Bus Address Extension Bit */
#define RMCS1_A16    0000400 /* (R/W)  A16 Bus Address Extension Bit */
#define RMCS1_RDY    0000200 /* (R)    Ready */
#define RMCS1_IE     0000100 /* (R/W)  Interrupt Enable */
#define RMCS1_F4     0000040 /* (R/W) *F4 Function (Command) Code */
#define RMCS1_F3     0000020 /* (R/W) *F3 Function (Command) Code */
#define RMCS1_F2     0000010 /* (R/W) *F2 Function (Command) Code */
#define RMCS1_F1     0000004 /* (R/W) *F1 Function (Command) Code */
#define RMCS1_F0     0000002 /* (R/W) *F0 Function (Command) Code */
#define RMCS1_GO     0000001 /* (R/W) *Go */

#define RMCS1_BAE    0001400 /* (R/W)  A16/A17 BAE bits */
#define RMCS1_FUNC   0000076 /* (R/W)  F0-F4 Function Code */

#define RMCS1_DRV    0014077 /* (R/W)  Drive Mask */
#define RMCS1_CTLR   0163700 /* (R/W)  Controller Mask */
#define RMCS1_MBZ    0010000 /*        Must Be Zeros */

/* RMWC - Word Count Register */
#define RMWC_MASK    0177777 /* (R/W)  Word Count */

/* RMBA - Bus Address Register */
#define RMBA_MASK    0177776 /* (R/W)  Bus Address */
#define RMBA_MBZ     0000001 /*        Must be Zeros */

/* RMDA - Desired Sector and Track Address Register */
#define RMDA_SP      0160340 /* (R/W)  Spare for future expansion */
#define RMDA_TA      0017400 /* (R/W)  Track Address */
#define RMDA_SA      0000037 /* (R/W)  Sector Address */

/* RMCS2 - Control and Status 2 Register */
#define RMCS2_DLT    0100000 /* (R)    Data Late */
#define RMCS2_WCE    0040000 /* (R)    Write Check Error */
#define RMCS2_UPE    0020000 /* (R)    Unibus Parity Error */
#define RMCS2_NED    0010000 /* (R)    Non-Existent Drive */
#define RMCS2_NEM    0004000 /* (R)    Non-Existent Memory */
#define RMCS2_PGE    0002000 /* (R)    Program Error */
#define RMCS2_MXF    0001000 /* (R)    Missed Transfer */
#define RMCS2_MDPE   0000400 /* (R)    Mass I/O Bus Data Parity Error */
#define RMCS2_OR     0000200 /* (R)    Output Ready */
#define RMCS2_IR     0000100 /* (R)    Input Ready */
#define RMCS2_CLR    0000040 /* (W)    Controller Clear */
#define RMCS2_PAT    0000020 /* (R/W)  Parity Test */
#define RMCS2_BAI    0000010 /* (R/W)  Unibus Address Increment Inhibit */
#define RMCS2_UNIT   0000007 /* (R/W)  U0-U2 Unit Select (2:0) */

#define RMCS2_RD     0177737 /* (R)    Read Mask */
#define RMCS2_WR     0000077 /* (W)    Write Mask */

/* RMDS - Drive Status Register */
#define RMDS_ATA     0100000 /* (R)    Attention Active */
#define RMDS_ERR     0040000 /* (R)    Error */
#define RMDS_PIP     0020000 /* (R)    Positioning in Progress */
#define RMDS_MOL     0010000 /* (R)    Medium On-Line */
#define RMDS_WRL     0004000 /* (R)    Write Lock */
#define RMDS_LBT     0002000 /* (R)    Last Block Transferred */
#define RMDS_PGM     0001000 /* (R)    Programmable */
#define RMDS_DPR     0000400 /* (R)    Drive Present */
#define RMDS_DRY     0000200 /* (R)    Drive Ready */
#define RMDS_VV      0000100 /* (R)    Volume Valid */
#define RMDS_SP      0000076 /* (R)    Spare for future expansions */
#define RMDS_OM      0000001 /* (R)    Offset Mode */

#define RMDS_MBZ     0000076 /*        Must Be Zeros */

/* RMER1 - Error Register #1 */
#define RMER1_DCK    0100000 /* (R)    Data Check Error */
#define RMER1_UNS    0040000 /* (R)    Drive */
#define RMER1_OPI    0020000 /* (R)    Operation Incomplete */
#define RMER1_DTE    0010000 /* (R)    Drive Timing Error */
#define RMER1_WLE    0004000 /* (R)    Write Lock Error */
#define RMER1_IAE    0002000 /* (R)    Invalid Address Error */
#define RMER1_AOE    0001000 /* (R)    Address Overflow Error */
#define RMER1_HCRC   0000400 /* (R)    Header CRC Error */
#define RMER1_HCE    0000200 /* (R)    Header Compare Error */
#define RMER1_ECH    0000100 /* (R)    ECC Hard Error */
#define RMER1_WCF    0000040 /* (R/W)  Write Clock Fail */
#define RMER1_FER    0000020 /* (R)    Format Error */
#define RMER1_PAR    0000010 /* (R)    Parity Error */
#define RMER1_RMR    0000004 /* (R/W)  Register Modification Refused */
#define RMER1_ILR    0000002 /* (R/W)  Illegal Register */
#define RMER1_ILF    0000001 /* (R/W)  Illegal Function */

/* RMLA - Look-Ahead Register */
#define RMLA_SC      0003700 /* (R)    Sector Count */

/* RMDB - Data Buffer Register */
#define RMDB_MASK    0177777 /* (R/W)  Data Buffer */

/* RMMR1 - Maintenance Register #1 - Read Only */
#define RMMR1_RD_OCC  0100000 /* (R) Occupied */
#define RMMR1_RD_RG   0040000 /* (R) Run and Go */
#define RMMR1_RD_EBL  0020000 /* (R) End of Block */
#define RMMR1_RD_REX  0010000 /* (R) Execption */
#define RMMR1_RD_ESRC 0004000 /* (R) Enable Search */
#define RMMR1_RD_LFS  0002000 /* (R) Looking for Sync */
#define RMMR1_RD_ECRC 0001000 /* (R) Enable CRC Out */
#define RMMR1_RD_PDA  0000400 /* (R) Data Area */
#define RMMR1_RD_PHA  0000200 /* (R) Header Area */
#define RMMR1_RD_CONT 0000100 /* (R) Continue */
#define RMMR1_RD_WC   0000040 /* (R) PROM Strobe */
#define RMMR1_RD_EECC 0000020 /* (R) Enable ECC Out */
#define RMMR1_RD_WE   0000010 /* (R) Write Data */
#define RMMR1_RD_LS   0000004 /* (R) Last Sector */
#define RMMR1_RD_LST  0000002 /* (R) Last Sector and Track */
#define RMMR1_RD_DMD  0000001 /* (R) Diagnostic Mode */

/* RMMR1 - Maintenance Register #1 - Write Only */
#define RMMR1_WR_DBCK 0100000 /* (W) Debug Clock */
#define RMMR1_WR_DBEN 0040000 /* (W) Debug Clock Enable */
#define RMMR1_WR_DEBL 0020000 /* (W) Diagnostic EBL */
#define RMMR1_WR_MSEN 0010000 /* (W) Search Timeout Disable */
#define RMMR1_WR_MCLK 0004000 /* (W) Maintenance Clock */
#define RMMR1_WR_MRD  0002000 /* (W) Read Data */
#define RMMR1_WR_MUR  0001000 /* (W) Unit Ready */
#define RMMR1_WR_MDC  0000400 /* (W) On Cyclinder */
#define RMMR1_WR_MSER 0000200 /* (W) Seek Error */
#define RMMR1_WR_MDF  0000100 /* (W) Drive Fault */
#define RMMR1_WR_MS   0000040 /* (W) Sector Pulse */
#define RMMR1_WR_X    0000020 /*     Not used */
#define RMMR1_WR_MWP  0000010 /* (W) Write Protection */
#define RMMR1_WR_MI   0000004 /* (W) Index Pulse */
#define RMMR1_WR_MSC  0000002 /* (W) Sector Compare */
#define RMMR1_WR_DMD  0000001 /* (W) Diagnostic Mode */

/* RMDT - Drive Type Register */
#define RMDR_MOH     0020000 /* (R)    Moving Head */
#define RMDT_DRQ     0004000 /* (R)    Drive Request Required */
#define RMDT_DT      0000777 /* (R)    Drive Type */

/* RMSN - Serial Number Register */
#define RMSN_MASK    0177777 /* (R)    Serial Number */

/* RMOF - Offset Register */
#define RMOF_MASK    0177777 /* (R)    Offset */

/* RMDC - Desired Cyclinder Register */
#define RMDC_MASK    0177777 /* (R/W)  Desired Cyclinder */

/* RMHR - Holding Register */
/* Not used */

/* RMMR2 - Maintenace Register #2 */
#define RMMR2_REQA   0100000 /* (R)    Request A */
#define RMMR2_REQB   0040000 /* (R)    Request B */
#define RMMR2_TAG    0020000 /* (R)    Tag */
#define RMMR2_TB     0010000 /* (R)    Test Bit */
#define RMMR2_CCYL   0004000 /* (R)    Control or Cylinder Select */
#define RMMR2_CHD    0002000 /* (R)    Control or Head Select */
#define RMMR2_BB     0001777 /* (R)    Bus Line (9:0) */

/* RMER2 - Error Register #2 */
#define RMER2_BSE    0100000 /* (R)    Bad Sector Error */
#define RMER2_SKI    0040000 /* (R)    Seek Incomplete */
#define RMER2_OPE    0020000 /* (R)    Operator Plug Error */
#define RMER2_IVC    0010000 /* (R)    Invalid Command */
#define RMER2_LSC    0004000 /* (R)    Loss of System Clock */
#define RMER2_LBC    0002000 /* (R)    Loss of Bit Check */
#define RMER2_DVC    0000200 /* (R)    Device Check */
#define RMER2_DPE    0000010 /* (R)    Data Parity Error */

/* RMEC1 - ECC Register #1 */

/* RMEC2 - ECC Register #2 */

/* RMBAE - Bus Address Extension Register */
#define RMBAE_MASK   0000077 /* (R/W)  Bus Address Extension (A21-A16) */
#define RMBAE_MBZ    0177700 /*        Must be Zeros */

/* RMCS3 - Control and Status 3 Register */
#define RMCS3_APE    0100000 /* (R)    Address Parity Error */
#define RMCS3_DFE_OW 0040000 /* (R)    Data Parity Error - Odd Word */
#define RMCS3_DFE_EW 0020000 /* (R)    Data Parity Error - Even Word */
#define RMCS3_WCE_OW 0010000 /* (R)    Write Check Error - Odd Word */
#define RMCS3_WCE_EW 0004000 /* (R)    Write Check Error - Even Word */
#define RMCS3_DBL    0002000 /* (R)    Double Word */
#define RMCS3_IE     0000100 /* (R/W)  Interrupt Enable */
#define RMCS3_IPCK   0000017 /* (R/W)  Invert Parity Check (3:0) */

#define RMCS3_RD     0176117 /* (R)    Read Mask */
#define RMCS3_WR     0000117 /* (W)    Write Mask */
#define RMCS3_MBZ    0001660 /*        Must Be Zeros */

// Maximum number of units per disk controller
#define RM_MAXUNITS 8

// RM Disk Drive Unit
typedef struct {
	uint16 rmcs1; // Control and Status Register #1
//	uint16 rmwc;  // Word Count Register
//	uint16 rmba;  // Bus Address Register
	uint16 rmda;  // Desired Sector/Track Address Register
//	uint16 rmcs2; // Control and Status Register #2
	uint16 rmds;  // Drive Status Register
	uint16 rmer1; // Error Register #1
//	uint16 rmas;  // Attention Summary Register
	uint16 rmla;  // Look-Ahead Register
//	uint16 rmdb;  // Data Buffer Register
	uint16 rmmr;  // Maintenance Register #1
	uint16 rmdt;  // Drive Type Register
	uint16 rmsn;  // Serial Number Register
	uint16 rmof;  // Offset Register
	uint16 rmdc;  // Desired Cylinder Address Register
	uint16 rmhr;  // Holding Register (Not Used)
	uint16 rmmr2; // Maintenance Register #2
	uint16 rmer2; // Error Register #2
	uint16 rmec1; // ECC Position Register
	uint16 rmec2; // ECC Pattern Register
//	uint16 rmbae; // Bus Address Extension Register
//	uint16 rmcs3; // Control and Status Register #3
} RMUNIT;
