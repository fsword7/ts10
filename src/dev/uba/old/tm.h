// tm.h - TM MASSBUS-based tape drive definitions
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

#define MTCS1  (000 >> 1) /* (R/W) ^Control and Status Register #1 */
#define MTWC   (002 >> 1) /* (R/W)  Word Count Register */
#define MTBA   (004 >> 1) /* (R/W)  Bus Address Register */
#define MTFC   (006 >> 1) /* (R/W) *Frame Count Register */
#define MTCS2  (010 >> 1) /* (R/W)  Control and Status Register #2 */
#define MTDS   (012 >> 1) /* (R)   *Drive Status Register */
#define MTER   (014 >> 1) /* (R)   *Error Register */
#define MTAS   (016 >> 1) /* (R/W) *Attention Summary Puesdo-Register */
#define MTCC   (020 >> 1) /* (R/W)  Character Check Register */
#define MTDB   (022 >> 1) /* (R/W)  Data Buffer Register */
#define MTMR   (024 >> 1) /* (R/W)  Maintenance Register */
#define MTDT   (026 >> 1) /* (R)   *Drive Type Register */
#define MTSN   (030 >> 1) /* (R)   *Serial Number Register */
#define MTTC   (032 >> 1) /* (R/W) *Tape Control Register */
#define MTBAE  (034 >> 1) /* (R/W)  Bus Address Extension Register */
#define MTCS3  (036 >> 1) /* (R/W)  Control and Status Register #3 */

/* MTCS1 - Control and Status Register #1 */
#define MTCS1_SC    0100000 /* (R)    Special Condition */
#define MTCS1_TRE   0040000 /* (R/W)  Transfer Error */
#define MTCS1_MCPE  0020000 /* (R)    MASSBUS Control Bus Parity Error */
#define MTCS1_DVA   0004000 /* (R)   *Drive Available */
#define MTCS1_PSEL  0002000 /* (R)    Not Used */
#define MTCS1_A17   0001000 /* (R/W)  A17 Unibus Bus Address */
#define MTCS1_A16   0000400 /* (R/W)  A16 Unibus Bus Address */
#define MTCS1_RDY   0000200 /* (R)    Ready */
#define MTCS1_IE    0000100 /* (R/W)  Interrupt Enable */
#define MTCS1_FUNC  0000076 /* (R/W) *Function Code (F0-F4) */
#define MTCS1_GO    0000001 /* (R/W) *Go */

#define MTCS1_WR    0041577 /* (W)    Write Mask */

/* MTWC - Word Count Register */
#define MTWC_MASK   0177777 /* (R/W)  Word Count */

/* MTBA - Bus Address Register */
#define MTBA_MASK   0177776 /* (R/W)  Bus Address */
#define MTBA_MBZ    0000001 /*        Must Be Zero */

/* MTFC - Frame Count Register */
#define MTFC_MASK   0177777 /* (R/W)  Frame Count */

/* MTCS2 - Control and Status Register #2 */
#define MTCS2_DLT   0100000 /* (R)    Date Late */
#define MTCS2_WCE   0040000 /* (R)    Write Check Error */
#define MTCS2_PE    0020000 /* (R/W)  Parity Error */
#define MTCS2_NED   0010000 /* (R)    Non-existent Drive */
#define MTCS2_NEM   0004000 /* (R)    Non-existent Memory */
#define MTCS2_PGE   0002000 /* (R)    Program Error */
#define MTCS2_MXF   0001000 /* (R/W)  Missed Transfer */
#define MTCS2_MDPE  0000400 /* (R)    MASSBUS Data Bus Parity Error */
#define MTCS2_OR    0000200 /* (R)    Output Ready */
#define MTCS2_IR    0000100 /* (R)    Input Ready */
#define MTCS2_CLR   0000040 /* (W)    Controller Clear */
#define MTCS2_PAT   0000020 /* (R/W)  Parity Test */
#define MTCS2_BAI   0000010 /* (R/W)  Unibus Address Incremented Inhibit */
#define MTCS2_UNIT  0000007 /* (R/W)  Unit Select */

/* MTDS - Drive Status Register */
#define MTDS_ATA    0100000 /* (R)    Attention Active */
#define MTDS_ERR    0040000 /* (R)    Error Summary */
#define MTDS_PIP    0020000 /* (R)    Positioning in Progress */
#define MTDS_MOL    0010000 /* (R)    Medium On-Line */
#define MTDS_WRL    0004000 /* (R)    Write Locked */
#define MTDS_EOT    0002000 /* (R)    End of Tape */
#define MTDS_DPR    0000400 /* (R)    Drive Present */
#define MTDS_DRY    0000200 /* (R)    Drive Ready */
#define MTDS_SSC    0000100 /* (R)    Slave Status Change */
#define MTDS_PES    0000040 /* (R)    Phase Encoded Status */
#define MTDS_SDWN   0000020 /* (R)    Slowing Down */
#define MTDS_IDB    0000010 /* (R)    Identification Burst */
#define MTDS_TM     0000004 /* (R)    Tape Mark */
#define MTDS_BOT    0000002 /* (R)    Beginning of Tape */
#define MTDS_SLA    0000001 /* (R)    Slave Attention */

#define MTDS_SLAVE  0036076 // (R)    Slave drive bits

/* MTER - Error Register */
#define MTER_CRC    0100000 /* (R)    CRC Error */
#define MTER_COR    0100000 /* (R)    Correctable Data Error */
#define MTER_UNS    0040000 /* (R)    Unsafe */
#define MTER_OPI    0020000 /* (R)    Operation Incomplete */
#define MTER_DTE    0010000 /* (R)    Drive Timing Error */
#define MTER_NEF    0004000 /* (R)    Non-executable Function */
#define MTER_CS     0002000 /* (R)    Correctable Skew */
#define MTER_ITM    0002000 /* (R)    Illegal Tape Mark */
#define MTER_FCE    0001000 /* (R)    Frame Count Error */
#define MTER_NSG    0000400 /* (R)    Non-Standard Gap */
#define MTER_PEF    0000200 /* (R)    PE Format Error */
#define MTER_LRC    0000200 /* (R)    LRC */
#define MTER_INC    0000100 /* (R)    Incorrectable Data */
#define MTER_VPE    0000100 /* (R)    Vertical Parity Error */
#define MTER_DPAR   0000040 /* (R)    Data Bus Parity Error */
#define MTER_FMT    0000020 /* (R)    Format Error */
#define MTER_CPAR   0000010 /* (R)    Control Bus Parity Error */
#define MTER_RMR    0000004 /* (R)    Register Modification Refused */
#define MTER_ILR    0000002 /* (R)    Illegal Register */
#define MTER_ILF    0000001 /* (R)    Illegal Function */

/* MTAS - Attention Summary */
#define MTAS_MASK   0000377 /* (R/C)  Attention Active Summary (7:0) */
#define MTAS_MBZ    0177400 /*        Must Be Zeros */

/* MTCC - Character Check */
#define MTCC_MASK   0000777 /* (R)    Check Character Read Track (CCD) */
#define MTCC_MBZ    0177000 /         Must Be Zeros */

/* MTDB - Data Buffer */
#define MTDB_MASK   0177777 /* (R/W)  Data Buffer */

/* MTMR - Maintenance Register */
#define MTMR_MDF    0177600 /* (R/W)  Maintenance Data Field (8:15) */
#define MTMR_BPICLK 0000100 /* (R)    200 BPI Clock */
#define MTMR_MC     0000040 /* (R/W)  Maintenance Clock */
#define MTMR_MF     0000036 /* (R/W)  Maintenance Function */
#define MTMR_MM     0000001 /* (R/W)  Maintenance Mode */

/* MTDT - Drive Type */
#define MTDT_NSA    0100000 /* (R)    Not-Sector Addresses */
#define MTDT_TAP    0040000 /* (R)    Tape Drive */
#define MTDT_MOH    0020000 /* (R)    Moving Head */
#define MTDT_7CH    0010000 /* (R)    7 Channel */
#define MTDT_DRQ    0004000 /* (R)    Drive Request Required */
#define MTDT_SPR    0002000 /* (R)    Slave Present */
#define MTDT_DT     0000777 /* (R)    Drive Type (0:8) */

/* MTSN - Serial Number */
#define MTSN_MASK   0177777 /* (R)    Serial Number */

/* MTTC - Tape Control */
#define MTTC_ACCL   0100000 /* (R)    Acceleration */
#define MTTC_FCS    0040000 /* (R)    Frame Count Status */
#define MTTC_TCW    0020000 /* (R)    Tape Control Write */
#define MTTC_EAODTE 0010000 /* (R/W)  Enable Abort on Data Tranfer Errors */
#define MTTC_DEN    0003400 /* (R/W)  Density (0:2) */
#define MTTC_FMTSEL 0000360 /* (R/W)  Format Select */
#define MTTC_EVPAR  0000010 /* (R/W)  Even Parity */
#define MTTC_SLAVE  0000007 /* (R/W)  Slave Select (0:2) */

#define MTTC_WR     0013777 /* (W)    Write Mask */
#define MTTC_MBZ    0004000 /*        Must Be Zero */

/* MTBAE - Bus Address External Register */
#define MTBAE_MASK  0000037 /* (R/W)  Bus Address (A16:A21) */
#define MTBAE_MBZ   0177740 /*        Must Be Zeros */

/* MTCS3 - Control and Status Register #3 */
#define MTCS3_APE   0100000 /* (R)    Address Parity Error */
#define MTCS3_DEPOW 0040000 /* (R)    Data Parity Error - Odd Word */
#define MTCS3_DEPEW 0020000 /* (R)    Data Parity Error - Even Word */
#define MTCS3_WCEOW 0010000 /* (R)    Write Check Error - Odd Word */
#define MTCS3_WCEEW 0004000 /* (R)    Write Check Error - Even Word */
#define MTCS3_DBL   0002000 /* (R)    Double Word */
#define MTCS3_IE    0000100 /* (R/W)  Interrupt Enable */
#define MTCS3_IPCK  0000017 /* (R/W)  Invert Parity Check (0:3) */

#define TM_MAXUNITS  8  /* Maximum 8 units per MASSBUS controller */
#define TM_MAXSLAVES 8  /* Maximum 8 slaves per tape controller */

#define TM_BOT       0 // Beginning of Tape
#define TM_MOT       1 // Middle of Tape
#define TM_MARK      2 // Tape Mark
#define TM_EOT       3 // End of Tape

#define TM_TM03      0040 /* TM03 Drive Type */

/* Density Code for MTCC Register */
#define DEN200   00
#define DEN556   01
#define DEN800   02
#define DEN1600  04
#define DEN6250  07

// Format Code for MTCC register
#define FMT10_COREDUMP   000 // PDP-10 Core Dump
#define FMT10_7TRACK     001 // PDP-10 7-Track
#define FMT10_ASCII      002 // PDP-10 ASCII
#define FMT10_COMPATIBLE 003 // PDP-10 Compatible
#define FMT11_NORMAL     014 // PDP-11 Normal
#define FMT11_COREDUMP   015 // PDP-11 Core Dump
#define FMT11_15NORMAL   016 // PDP-11 15 Normal

// TM 02/03 Controller Unit
typedef struct {
	int16  Position[TM_MAXSLAVES];  // Which position of tape
	int16  dStatus[TM_MAXSLAVES];   // Slave Status
	int16  dTypes[TM_MAXSLAVES];    // Tape Drive Type
	int16  Serial[TM_MAXSLAVES];    // Serial Number

	uint16 mtcs1; // Control and Status Register #1
//	uint16 mtwc;  // Word Count Register
//	uint16 mtba;  // Bus Address Register
	uint16 mtfc;  // Frame Count Register
//	uint16 mtcs2; // Control and Status Register #2
	uint16 mtds;  // Drive Status Register
	uint16 mter;  // Error Register
//	uint16 mtas;  // Attention Summary Register
	uint16 mtcc;  // Character Check Register
//	uint16 mtdb;  // Data Buffer Register
	uint16 mtmr;  // Maintenance Register
	uint16 mtdt;  // Drive Type Register
	uint16 mtsn;  // Serial Number Register
	uint16 mttc;  // Tape Control Register
//	uint16 mtbae; // Bus Address Extension Register
//	uint16 mtcs3; // Control and Status Register #3
} MTUNIT;
