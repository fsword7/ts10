// mba_rm.h - MASSBUS disk drive definitions
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

// Massbus Address Index 
#define nRMCS1  000 // (R/W) Control and Status Register #1
#define nRMDS   001 // (R/W) Drive Status Register
#define nRMER1  002 // (R/W) Error Register #1
#define nRMMR1  003 // (R/W) Maintenance Register #1
#define nRMAS   004 // (R/W) Attention Summary Register
#define nRMDA   005 // (R/W) Desired Sector/Track Address Register
#define nRMDT   006 // (R)   Drive Type Register
#define nRMLA   007 // (R)   Look-Ahead Register
#define nRMSN   010 // (R)   Serial Number Register
#define nRMOF   011 // (R/W) Offset Register
#define nRMDC   012 // (R/W) Desired Cylinder Register
#define nRMHR   013 // (R/W) Holding Register
#define nRMMR2  014 // (R)   Maintenance Register #2
#define nRMER2  015 // (R/W) Error Register #2
#define nRMEC1  016 // (R)   ECC Register #1
#define nRMEC2  017 // (R)   ECC Register #2

#define RMCS1  MBA_REG(nRMCS1) // (R/W) Control and Status Register #1
#define RMDS   MBA_REG(nRMDS)  // (R/W) Drive Status Register
#define RMER1  MBA_REG(nRMER1) // (R/W) Error Register #1
#define RMMR1  MBA_REG(nRMMR1) // (R/W) Maintenance Register #1
#define RMAS   MBA_REG(nRMAS)  // (R/W) Attention Summary Register
#define RMDA   MBA_REG(nRMDA)  // (R/W) Desired Sector/Track Address Register
#define RMDT   MBA_REG(nRMDT)  // (R)   Drive Type Register
#define RMLA   MBA_REG(nRMLA)  // (R)   Look-Ahead Register
#define RMSN   MBA_REG(nRMSN)  // (R)   Serial Number Register
#define RMOF   MBA_REG(nRMOF)  // (R/W) Offset Register
#define RMDC   MBA_REG(nRMDC)  // (R/W) Desired Cylinder Register
#define RMHR   MBA_REG(nRMHR)  // (R/W) Holding Register
#define RMMR2  MBA_REG(nRMMR2) // (R)   Maintenance Register #2
#define RMER2  MBA_REG(nRMER2) // (R/W) Error Register #2
#define RMEC1  MBA_REG(nRMEC1) // (R)   ECC Register #1
#define RMEC2  MBA_REG(nRMEC2) // (R)   ECC Register #2

// RMCS1 - Control and Status 1 Register
#define RMCS1_SC     0100000 // (R)    Special Condition
#define RMCS1_TRE    0040000 // (R/W)  Transfer Error
#define RMCS1_MCPE   0020000 // (R)    Mass I/O Bus Control Parity Error
#define RMCS1_X      0010000 // (R)   *Reserved for use by the drive
#define RMCS1_DVA    0004000 // (R)   *Drive Available
#define RMCS1_PSEL   0002000 // (R/W)  Port Select
#define RMCS1_BAE    0001400 // (R/W)  A16/A17 BAE bits 
#define RMCS1_RDY    0000200 // (R)    Ready
#define RMCS1_IE     0000100 // (R/W)  Interrupt Enable
#define RMCS1_FUNC   0000076 // (R/W) *Function (Command) Code
#define RMCS1_GO     0000001 // (R/W) *Go

#define RMCS1_DRV    0014077 // (R/W)  Drive Mask
#define RMCS1_CTLR   0163700 // (R/W)  Controller Mask
#define RMCS1_MBZ    0010000 //        Must Be Zeros

// RMDA - Desired Sector and Track Address Register
#define RMDA_SP      0160340 // (R/W)  Spare for future expansion
#define RMDA_TA      0017400 // (R/W)  Track Address
#define RMDA_SA      0000037 // (R/W)  Sector Address

// RMDS - Drive Status Register
#define RMDS_ATA     0100000 // (R)    Attention Active
#define RMDS_ERR     0040000 // (R)    Error
#define RMDS_PIP     0020000 // (R)    Positioning in Progress
#define RMDS_MOL     0010000 // (R)    Medium On-Line
#define RMDS_WRL     0004000 // (R)    Write Lock
#define RMDS_LBT     0002000 // (R)    Last Block Transferred
#define RMDS_PGM     0001000 // (R)    Programmable
#define RMDS_DPR     0000400 // (R)    Drive Present
#define RMDS_DRY     0000200 // (R)    Drive Ready
#define RMDS_VV      0000100 // (R)    Volume Valid
#define RMDS_OM      0000001 // (R)    Offset Mode

// RMER1 - Error Register #1
#define RMER1_DCK    0100000 // (R)    Data Check Error
#define RMER1_UNS    0040000 // (R)    Drive
#define RMER1_OPI    0020000 // (R)    Operation Incomplete
#define RMER1_DTE    0010000 // (R)    Drive Timing Error
#define RMER1_WLE    0004000 // (R)    Write Lock Error
#define RMER1_IAE    0002000 // (R)    Invalid Address Error
#define RMER1_AOE    0001000 // (R)    Address Overflow Error
#define RMER1_HCRC   0000400 // (R)    Header CRC Error
#define RMER1_HCE    0000200 // (R)    Header Compare Error
#define RMER1_ECH    0000100 // (R)    ECC Hard Error
#define RMER1_WCF    0000040 // (R/W)  Write Clock Fail
#define RMER1_FER    0000020 // (R)    Format Error
#define RMER1_PAR    0000010 // (R)    Parity Error
#define RMER1_RMR    0000004 // (R/W)  Register Modification Refused
#define RMER1_ILR    0000002 // (R/W)  Illegal Register
#define RMER1_ILF    0000001 // (R/W)  Illegal Function

// RMLA - Look-Ahead Register
#define RMLA_SC      0003700 // (R)    Sector Count

// RMMR1 - Maintenance Register #1 - Read Only
#define RMMR1_RD_OCC  0100000 // (R) Occupied
#define RMMR1_RD_RG   0040000 // (R) Run and Go
#define RMMR1_RD_EBL  0020000 // (R) End of Block
#define RMMR1_RD_REX  0010000 // (R) Execption
#define RMMR1_RD_ESRC 0004000 // (R) Enable Search
#define RMMR1_RD_LFS  0002000 // (R) Looking for Sync
#define RMMR1_RD_ECRC 0001000 // (R) Enable CRC Out
#define RMMR1_RD_PDA  0000400 // (R) Data Area
#define RMMR1_RD_PHA  0000200 // (R) Header Area
#define RMMR1_RD_CONT 0000100 // (R) Continue
#define RMMR1_RD_WC   0000040 // (R) PROM Strobe
#define RMMR1_RD_EECC 0000020 // (R) Enable ECC Out
#define RMMR1_RD_WE   0000010 // (R) Write Data
#define RMMR1_RD_LS   0000004 // (R) Last Sector
#define RMMR1_RD_LST  0000002 // (R) Last Sector and Track
#define RMMR1_RD_DMD  0000001 // (R) Diagnostic Mode

// RMMR1 - Maintenance Register #1 - Write Only
#define RMMR1_WR_DBCK 0100000 // (W) Debug Clock
#define RMMR1_WR_DBEN 0040000 // (W) Debug Clock Enable
#define RMMR1_WR_DEBL 0020000 // (W) Diagnostic EBL
#define RMMR1_WR_MSEN 0010000 // (W) Search Timeout Disable
#define RMMR1_WR_MCLK 0004000 // (W) Maintenance Clock
#define RMMR1_WR_MRD  0002000 // (W) Read Data
#define RMMR1_WR_MUR  0001000 // (W) Unit Ready
#define RMMR1_WR_MDC  0000400 // (W) On Cyclinder
#define RMMR1_WR_MSER 0000200 // (W) Seek Error
#define RMMR1_WR_MDF  0000100 // (W) Drive Fault
#define RMMR1_WR_MS   0000040 // (W) Sector Pulse
#define RMMR1_WR_X    0000020 //     Not used
#define RMMR1_WR_MWP  0000010 // (W) Write Protection
#define RMMR1_WR_MI   0000004 // (W) Index Pulse
#define RMMR1_WR_MSC  0000002 // (W) Sector Compare
#define RMMR1_WR_DMD  0000001 // (W) Diagnostic Mode

// RMDT - Drive Type Register
#define RMDR_MOH     0020000 // (R)    Moving Head
#define RMDT_DRQ     0004000 // (R)    Drive Request Required
#define RMDT_DT      0000777 // (R)    Drive Type

// RMSN - Serial Number Register
#define RMSN_MASK    0177777 // (R)    Serial Number

// RMOF - Offset Register
#define RMOF_MASK    0177777 // (R)    Offset

// RMDC - Desired Cyclinder Register
#define RMDC_MASK    0177777 // (R/W)  Desired Cyclinder

// RMHR - Holding Register (Not Used)

// RMMR2 - Maintenace Register #2
#define RMMR2_REQA   0100000 // (R)    Request A
#define RMMR2_REQB   0040000 // (R)    Request B
#define RMMR2_TAG    0020000 // (R)    Tag
#define RMMR2_TB     0010000 // (R)    Test Bit
#define RMMR2_CCYL   0004000 // (R)    Control or Cylinder Select
#define RMMR2_CHD    0002000 // (R)    Control or Head Select
#define RMMR2_BB     0001777 // (R)    Bus Line (9:0)

// RMER2 - Error Register #2
#define RMER2_BSE    0100000 // (R)    Bad Sector Error
#define RMER2_SKI    0040000 // (R)    Seek Incomplete
#define RMER2_OPE    0020000 // (R)    Operator Plug Error
#define RMER2_IVC    0010000 // (R)    Invalid Command
#define RMER2_LSC    0004000 // (R)    Loss of System Clock
#define RMER2_LBC    0002000 // (R)    Loss of Bit Check
#define RMER2_DVC    0000200 // (R)    Device Check
#define RMER2_DPE    0000010 // (R)    Data Parity Error

// RMEC1 - ECC Register #1

// RMEC2 - ECC Register #2
