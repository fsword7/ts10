// tu.h - MASSBUS tape drive definitions
//
// Copyright (c) 2001-2002, Timothy M. Stark
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

// MASSBUS Register Index
#define nMTCS1  000 // (R/W) Control and Status Register
#define nMTDS   001 // (R/W) Drive Status Register
#define nMTER   002 // (R)   Error Register
#define nMTMR   003 // (R/W) Maintenance Register
#define nMTAS   004 // (R/W) Attention Summary Register
#define nMTFC   005 // (R/W) Frame Count Register
#define nMTDT   006 // (R)   Drive Type Register
#define nMTCC   007 // (R/W) Character Check Register
#define nMTSN   010 // (R)   Serial Number Register
#define nMTTC   011 // (R/W) Tape Control Register

#define MTCS1 MBA_REG(nMTCS1) // (R/W) Control and Status Register
#define MTDS  MBA_REG(nMTDS) // (R/W) Drive Status Register
#define MTER  MBA_REG(nMTER) // (R)   Error Register
#define MTMR  MBA_REG(nMTMR) // (R/W) Maintenance Register
#define MTAS  MBA_REG(nMTAS) // (R/W) Attention Summary Register
#define MTFC  MBA_REG(nMTFC) // (R/W) Frame Count Register
#define MTDT  MBA_REG(nMTDT) // (R)   Drive Type Register
#define MTCC  MBA_REG(nMTCC) // (R/W) Character Check Register
#define MTSN  MBA_REG(nMTSN) // (R)   Serial Number Register
#define MTTC  MBA_REG(nMTTC) // (R/W) Tape Control Register

// MTCS - Control and Status Register
#define MTCS1_SC    0100000 // (R)    Special Condition
#define MTCS1_TRE   0040000 // (R/W)  Transfer Error
#define MTCS1_MCPE  0020000 // (R)    MASSBUS Control Bus Parity Error
#define MTCS1_DVA   0004000 // (R)   *Drive Available
#define MTCS1_PSEL  0002000 // (R)    Not Used
#define MTCS1_A17   0001000 // (R/W)  A17 Unibus Bus Address
#define MTCS1_A16   0000400 // (R/W)  A16 Unibus Bus Address
#define MTCS1_RDY   0000200 // (R)    Ready
#define MTCS1_IE    0000100 // (R/W)  Interrupt Enable
#define MTCS1_FUNC  0000076 // (R/W) *Function Code (F0-F4)
#define MTCS1_GO    0000001 // (R/W) *Go

#define MTCS1_WR    0041577 // (W)    Write Mask

// MTFC - Frame Count Register
#define MTFC_MASK   0177777 // (R/W)  Frame Count

// MTDS - Drive Status Register
#define MTDS_ATA    0100000 // (R)    Attention Active
#define MTDS_ERR    0040000 // (R)    Error Summary
#define MTDS_PIP    0020000 // (R)    Positioning in Progress
#define MTDS_MOL    0010000 // (R)    Medium On-Line
#define MTDS_WRL    0004000 // (R)    Write Locked
#define MTDS_EOT    0002000 // (R)    End of Tape
#define MTDS_DPR    0000400 // (R)    Drive Present
#define MTDS_DRY    0000200 // (R)    Drive Ready
#define MTDS_SSC    0000100 // (R)    Slave Status Change
#define MTDS_PES    0000040 // (R)    Phase Encoded Status
#define MTDS_SDWN   0000020 // (R)    Slowing Down
#define MTDS_IDB    0000010 // (R)    Identification Burst
#define MTDS_TM     0000004 // (R)    Tape Mark
#define MTDS_BOT    0000002 // (R)    Beginning of Tape
#define MTDS_SLA    0000001 // (R)    Slave Attention

#define MTDS_SLAVE  0036076 // (R)    Slave drive bits

// MTER - Error Register
#define MTER_CRC    0100000 // (R)    CRC Error
#define MTER_COR    0100000 // (R)    Correctable Data Error
#define MTER_UNS    0040000 // (R)    Unsafe
#define MTER_OPI    0020000 // (R)    Operation Incomplete
#define MTER_DTE    0010000 // (R)    Drive Timing Error
#define MTER_NEF    0004000 // (R)    Non-executable Function
#define MTER_CS     0002000 // (R)    Correctable Skew
#define MTER_ITM    0002000 // (R)    Illegal Tape Mark
#define MTER_FCE    0001000 // (R)    Frame Count Error
#define MTER_NSG    0000400 // (R)    Non-Standard Gap
#define MTER_PEF    0000200 // (R)    PE Format Error
#define MTER_LRC    0000200 // (R)    LRC
#define MTER_INC    0000100 // (R)    Incorrectable Data
#define MTER_VPE    0000100 // (R)    Vertical Parity Error
#define MTER_DPAR   0000040 // (R)    Data Bus Parity Error
#define MTER_FMT    0000020 // (R)    Format Error
#define MTER_CPAR   0000010 // (R)    Control Bus Parity Error
#define MTER_RMR    0000004 // (R)    Register Modification Refused
#define MTER_ILR    0000002 // (R)    Illegal Register
#define MTER_ILF    0000001 // (R)    Illegal Function

// MTAS - Attention Summary
#define MTAS_MASK   0000377 // (R/C)  Attention Active Summary (7:0)
#define MTAS_MBZ    0177400 //        Must Be Zeros

// MTCC - Character Check
#define MTCC_MASK   0000777 // (R)    Check Character Read Track (CCD)
#define MTCC_MBZ    0177000 //        Must Be Zeros

// MTMR - Maintenance Register
#define MTMR_MDF    0177600 // (R/W)  Maintenance Data Field (8:15)
#define MTMR_BPICLK 0000100 // (R)    200 BPI Clock
#define MTMR_MC     0000040 // (R/W)  Maintenance Clock
#define MTMR_MF     0000036 // (R/W)  Maintenance Function
#define MTMR_MM     0000001 // (R/W)  Maintenance Mode

// MTDT - Drive Type
#define MTDT_NSA    0100000 // (R)    Not-Sector Addresses
#define MTDT_TAP    0040000 // (R)    Tape Drive
#define MTDT_MOH    0020000 // (R)    Moving Head
#define MTDT_7CH    0010000 // (R)    7 Channel
#define MTDT_DRQ    0004000 // (R)    Drive Request Required
#define MTDT_SPR    0002000 // (R)    Slave Present
#define MTDT_DT     0000777 // (R)    Drive Type (0:8)

// MTSN - Serial Number
#define MTSN_MASK   0177777 // (R)    Serial Number

// MTTC - Tape Control
#define MTTC_ACCL   0100000 // (R)    Acceleration
#define MTTC_FCS    0040000 // (R)    Frame Count Status
#define MTTC_TCW    0020000 // (R)    Tape Control Write
#define MTTC_EAODTE 0010000 // (R/W)  Enable Abort on Data Tranfer Errors
#define MTTC_DEN    0003400 // (R/W)  Density (0:2)
#define MTTC_FMTSEL 0000360 // (R/W)  Format Select
#define MTTC_EVPAR  0000010 // (R/W)  Even Parity
#define MTTC_SLAVE  0000007 // (R/W)  Slave Select (0:2)

#define MTTC_WR     0013777 // (W)    Write Mask
#define MTTC_MBZ    0004000 //        Must Be Zero

#define TM_MAXSLAVES 8  /* Maximum 8 slaves per tape controller */
#define TM_XFER      65536 // Maximum 65536 bytes each record.

#define TM_MOT    0 // Middle of Tape
#define TM_MARK   1 // Tape Mark (End of File)
#define TM_BOT    2 // Bottom of Tape
#define TM_EOT    3 // End of Tape

#define TM_TM03      0040 // TM03 Drive Type
#define TM_TU00      0010 // No Slave Present

// Density Code for MTCC Register
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
typedef struct tu_Drive  TU_DRIVE;
typedef struct tm_Device TM_DEVICE;

struct tu_Drive {
	UNIT      Unit;       // Unit Header
	UQ_BOOT   Boot;       // Boot Device
 
	VMT_TAPE  dpTape;     // Virtual Tape Descriptor
	MBA_DRIVE *mbaDrive;  // MASSBUS Drive
	TM_DEVICE *fmtDevice; // Formatter Device
	uint32    idUnit;     // Unit ID
	uint32    Flags;      // Tape Flags

	// Tape Unit Operations
	uint16 Position;      // Which position of tape
	uint16 Status;        // Drive status
	uint16 devType;       // Device Type
	uint16 Serial;        // Serial Number
};

struct tm_Device {
	UNIT     Unit;       // Unit Header

	MBA_DRIVE *mbaDrive; // MASSBUS Drive
	uint32    idUnit;    // Unit ID
	TU_DRIVE  *dsUnit;   // Drive Select

	// Tape Drives Slots
	TU_DRIVE Slaves[TM_MAXSLAVES];
};
