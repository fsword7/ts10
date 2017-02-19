// rh20.h - RH20 MASSBUS Interface Support Routines.
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

#define RH20_KEY     "RH20"
#define RH20_NAME    "MASSBUS Interface"
#define RH20_VERSION "v0.5 (Alpha)"

#include "pdp10/iodefs.h"
#include "dev/mba/mba.h"

// DATAI/DATAO Register definitions

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | Register  |L|               |Drive|                                   |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5

// Common Data for All Registers (left halfword)
#define D_REG   0770000 // (R/W) Register Select Field
#define D_LDR   0004000 // (R/W) Load Register
#define D_DRE   0000400 // (W)   Disable Register Access Error
#define D_DRV   0000007 // (R/W) Drive Select Field

#define D_M_REG  077
#define D_P_REG  30
#define D_M_DRV  07
#define D_P_DRV  18

#define RH_DATA 07777777 // Drive Select and Data Field

// External Register Data
#define ER_CPE  0001000000000 // (R)   Control Bus Parity Error
#define ER_TRA  0000200000000 // (R)   Transfer Received
#define ER_CEP  0000000400000 // (W)   Control Bus Even Parity
#define ER_CPA  0000000200000 // (R)   Control Bus Parity Bit
#define ER_DATA 0000000177777 // (R/W) External Register Data

// Primary/Secondary Block Address Register
#define BA_ADR  0000000177777 // (R/W) Block Address Register

// Primary/Secondary Transfer Control Register
#define TC_RCP  0002000000000 // (R/W) Reset Command List Pointer
#define TC_SCS  0000200000000 // (R/W) Store Channel Status
#define TC_DTE  0000000200000 // (R/W) Disable Transfer Error Stop
#define TC_CNT  0000000177700 // (R/W) Negative Block Count
#define TC_FNC  0000000000077 // (R/W) Massbus Function Code

#define TC_MAXCNT 02000  // Maximum Block Count
#define TC_M_CNT  01777  // Mask of Block Count
#define TC_P_CNT  6      // Position of Block Count
#define TC_M_FNC  077    // Mask of Massbus Function

// Interrupt Vector Index Data Register
#define IVI_ADR 0000000000777 // (R/W) Interrupt Vector Address

// Specific Register definitions
#define RH20_XREG 040 // (R/W) External Data Register (00 - 37)
#define RH20_SBAR 070 // (R/W) Secondary Block Address Register
#define RH20_STCR 071 // (R/W) Secondary Transfer Control Register
#define RH20_PBAR 072 // (R)   Primary Block Address Register
#define RH20_PTCR 073 // (R)   Primary Transfer Control Register
#define RH20_IVIR 074 // (R/W) Interrupt Vector Index Register
#define RH20_RR   075 // (R)   Read Register (Diagnostics Only)
#define RH20_WR   076 // (W)   Write Register (Diagnostics Only)
#define RH20_DCR  077 // (W)   Diagnostic Control Register

// CONI/CONO/CONSZ/CONSO Register definitions

#define CI_DBP  0400000 // Data Bus Parity Error
#define CI_EXC  0200000 // Exception
#define CI_LWC  0100000 // Long Word Count
#define CI_SWC  0040000 // Short Word Count
#define CI_CER  0020000 // Channel Error
#define CI_DRE  0010000 // Drive Response Error
#define CI_RAE  0004000 // Register Access Error
#define CI_CNR  0002000 // Channel Ready
#define CI_OVR  0001000 // Data Overun
#define CI_MBE  0000400 // Massbus Enable
#define CI_ATN  0000200 // Attention
#define CI_SCR  0000100 // SCR Full (Loaded)
#define CI_AIE  0000040 // Attention Interrupt Enable
#define CI_PCR  0000020 // PCR Full (Loaded)
#define CI_CMD  0000010 // Command Done
#define CI_PIA  0000007 // Priority Interrupt Assignment

#define CO_RAE  0004000 // Clear Register Access Error
#define CO_MBI  0002000 // Massbus Initialization
#define CO_TEC  0001000 // Transfer Error Clear
#define CO_MBE  0000400 // Massbus Enable
#define CO_RCP  0000200 // Reset Command List Pointer
#define CO_SCR  0000100 // Delete SCR
#define CO_AIE  0000040 // Attention Interrupt Enable
#define CO_STP  0000020 // Stop Transfer
#define CO_CCD  0000010 // Clear Command Done
#define CO_PIA  0000007 // Priority Interrupt Assignment

#define CO_CMASK (CO_RAE|CO_TEC|CO_CCD)  // Clear bits mask
#define CO_WMASK (CO_MBE|CO_AIE|CO_PIA)  // Write bits mask

// Channel Logout Area - Defintions

#define CSICW  0   // Initial Control Word
#define CSCLP  1   // Status/Command List Pointer
#define CSDBA  2   // Current CCW
#define CSIVI  3   // Interrupt Vector Instruction

// CSICW - Initial Control Word
#define ICW_OPC  0700000000000 // Opcode
#define ICW_XFR  0400000000000 // Opcode - Data Transfer if Set
#define ICW_HLT  0200000000000 // Opcode - Halt Bit for Data Transfers
#define ICW_REV  0100000000000 // Opcode - Reverse Bit for Data Transfers
#define ICW_WDC  0077760000000 // Word Count
#define ICW_ADR  0000017777777 // Address

#define CWM_OPC 07        // Opcode Mask
#define CWP_OPC 33        // Position of Opcode Field
#define CWB_XFR 04        // Data Tranfer if Set
#define CWB_HLT 02        // Halt Bit for Data Transfers
#define CWB_REV 01        // Reverse Bit for Data Transfers
#define CWM_WDC 03777     // Word Count Mask
#define CWP_WDC 22        // Position of Word Count Field
#define CWM_ADR 017777777 // Address Mask

#define CWOP_HALT 00      // HALT Opcode
#define CWOP_JUMP 02      // JUMP Opcode
#define CWOP_RDF  04      // Read Data Forward (Do not halt)
#define CWOP_RDR  05      // Read Data Reverse (Do not halt)
#define CWOP_HRDF 06      // Read Data Forward (Halt)
#define CWOP_HRDR 07      // Read Data Reverse (Halt)

// CSCLP - Status/Command List Pointer
#define CLP_SET  0400000000000LL // Always Set One
#define CLP_MPE  0200000000000LL // Memory Parity Error
#define CLP_NAE  0100000000000LL // Not Address Parity Error
#define CLP_NW0  0040000000000LL // Not Zero Word Count
#define CLP_NXM  0020000000000LL // Non-existant Memory
#define CLP_LTE  0000400000000LL // Late Transfer Error
#define CLP_RHE  0000200000000LL // RH20 Error
#define CLP_LWC  0000100000000LL // Long Word Count
#define CLP_SWC  0000040000000LL // Short Word Count
#define CLP_OVR  0000020000000LL // Overrun
#define CLP_ADR  0000017777777LL // Command List Pointer

// CSDBA - Current Command Word
#define DBA_OPC  0700000000000 // Opcode
#define DBA_CWC  0077760000000 // Word Count
#define DBA_ADR  0000017777777 // Data Buffer Address

#define RH20_MAXUNITS  8      // Maximum number of RH20s.
#define RH20_BASE      0540   // Base Device Address

#define RH20_ERROR -1
#define RH20_OK     0
#define RH20_STOP   1

typedef struct rh20_Device RH20_DEVICE;

struct rh20_Device {
	UNIT      Unit;  // Unit header information
	P10_IOMAP ioMap; // I/O Mapping Information

	// RH20 Settings
	int       idChannel;   // Channel Number Identification
	uint32    Flags;       // Controller Flags
	int18     srFlags;     // Status Register
	int18     erFlags;     // External Register
	int32     piLevel;     // Priority Interrupt Level
	int32     piRequest;   // Pending Interrupts
	uint32    blkCount;    // Blocks Count
	uint16    Register;    // Current Register Number
	uint16    Drive;       // Current Drive Number
	uint16    Data;        // Current Data
	MBA_DRIVE *mbaDrive;   // Current Drive

	uint32 regPrep;   // Preparation Register (LH field)

	// Internal Registers (DATAI/DATAO Instructions)
	uint32 stcAddr;   // (R/W) Secondary Bus Address Register
	uint32 stcFlags;  // (R/W) Secondary Transfer Control Register
	uint32 ptcAddr;   // (R)   Primary Bus Address Register
	uint32 ptcFlags;  // (R)   Primary Transfer Control Register
	uint32 iviAddr;   // (R/W) Interrupt Vector Index

	// Data Channel Area
	uint32 clpAddr;   // CLP: Address
	uint32 ccwOpcode; // CCW: Opcode
	uint32 ccwAddr;   // CCW: Buffer Address
	uint32 ccwCount;  // CCW: Word Count
	uint36 ccwStatus; // CCW: Data Channel Status

	// MASSBUS Controller System
	MBA_CALLBACK mbaCall;
	MBA_DEVICE   mba;
};
