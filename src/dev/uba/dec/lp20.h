// lp20.h - LP20/LP05/LP14 - Line Printer Emulation
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

#define LP20_KEY     "LP20"
#define LP20_NAME    "Line Printer Interface"
#define LP20_VERSION "v0.1 (Pre-Alpha)"

// Unibus Address Index
#define nLPCSRA (000 >> 1) // (R/W) Control and Status A      (CSRA)
#define nLPCSRB (002 >> 1) // (R/W) Control and Status B      (CSRB)
#define nLPBSAD (004 >> 1) // (R/W) DMA Bus Address Register  (BSAD)
#define nLPBCTR (006 >> 1) // (R/W) DMA Byte Count Register   (BCTR)
#define nLPPCTR (010 >> 1) // (R/W) Page Count Register       (PCTR)
#define nLPRAMD (012 >> 1) // (R/W) RAM Data Register         (RAMD)
#define nLPCCTR (014 >> 1) // (R/W) Column Count Register     (CCTR)
#define nLPCBUF (014 >> 1) // (R/W) Character Buffer Register (CBUF)
#define nLPCKSM (016 >> 1) // (R)   Checksum Register         (CKSM)
#define nLPTDAT (016 >> 1) // (R)   Printer Data Register     (TDAT)

// Unibus Address 775400
// LPCSRA - Control and Status Register #A
#define CSA_ERR     0100000 // (R)   Error  ("OR" of all errors)
#define CSA_PAG0    0040000 // (R)   Page Counter Incremented to Zero
#define CSA_ILCHR   0020000 // (R)   Illegal/Undefined Character
#define CSA_DVON    0010000 // (R)   DAVFU is loaded and ready
#define CSA_ONL     0004000 // (R)   Printer is ready and online
#define CSA_DEL     0002000 // (R/W) Last Character Received - Delimiter
#define CSA_INIT    0001000 // (W)   Local Initialization (Unibus Init)
#define CSA_RESET   0000400 // (W)   Reset Errors, Set Done, Reset Go
#define CSA_DONE    0000200 // (R)   Done
#define CSA_IE      0000100 // (R/W) Interrupt Enable
#define CSA_UAE     0000060 // (R/W) Unibus Address Extension
#define CSA_MODE    0000014 // (R/W) Mode (Function)
#define CSA_PAR     0000002 // (R/W) Parity Enable
#define CSA_GO      0000001 // (R/W) Go (Start DMA Transfers)

#define CSA_RW      (CSA_DEL|CSA_IE|CSA_UAE|CSA_MODE|CSA_PAR|CSA_GO)

#define CSA_GETMODE (((x) >> CSA_P_MODE) & MODE_MASK)
#define CSA_P_MODE  2

// Function/Mode Codes for CSA register
#define MODE_MASK   03  // Mode Mask
#define MODE_PRINT   0  // Print
#define MODE_TEST    1  // Test
#define MODE_DVU     2  // Load DAVFU
#define MODE_RAM     3  // Load Translation RAM

// Unibus Address 775402
// LPCSRB - Control and Status Register #B
#define CSB_VDATA   0100000 // (R/W) Valid Data Flag
#define CSB_LA180   0040000 // (R)   Set if LA180 Type Printer
#define CSB_NRDY    0020000 // (R)   Set if Printer is Not Ready
#define CSB_PAR     0010000 // (R)   Data Parity Bit
#define CSB_OVFU    0004000 // (R)   Optical VFU
#define CSB_TEST    0003400 // (R/W) Test bits
#define CSB_OFFL    0000200 // (R)   Printer is Offline
#define CSB_DVOF    0000100 // (R)   DAVFU Not Ready
#define CSB_LPE     0000040 // (R)   LPT Parity Error
#define CSB_MPE     0000020 // (R)   Memory Parity Error
#define CSB_RPE     0000010 // (R)   RAM Parity Error
#define CSB_MTE     0000004 // (R)   Master Sync Timeout Error
#define CSB_DTE     0000002 // (R)   Demand Timeout Error
#define CSB_GOE     0000001 // (R)   Go Set and Error Up or Demand Not Up

#define CSB_ECLR    (CSB_GOE|CSB_DTE|CSB_MTE|CSB_RPE|CSB_MPE|CSB_LPE)
#define CSB_ERR     (CSB_ECLR|CSB_DVOF|CSB_OFFL)
#define CSB_RW      CSB_TEST

// Unibus Address 775404
// LPBSAD - DMA Bus Address Register
#define BSAD_ADR     0177777 // (R/W) Address of Printer Data Buffer

// Unibus Address 775406
// LPBCTR - Byte Count Register
#define BCTR_MASK    0007777 // (R/W) Byte Count (0 = Done & Interrupt)

// Unibus Address 775410
// LPPCTR - Page Count Register
#define PCTR_MASK    0007777 // (R/W) Page Count Limit

// Unibus Address 775412
// LPRAMD - RAM Data Register
#define RDAT_RPAR    0010000 // (R)   RAM Parity Error
#define RDAT_RINT    0004000 // (R/W) Interrupt
#define RDAT_RDEL    0002000 // (R/W) Delimiter
#define RDAT_RTRN    0001000 // (R/W) Translation
#define RDAT_RPI     0000400 // (R/W) Paper Instruction
#define RDAT_ADR     0000377 // (R/W) RAM Data Addr (Bit 13 is R-Only)
#define RDAT_MASK    0007777 // (R/W) RAM Data

// Unibus Address 775414
// LPCCTR - Column Count Register     (High Byte)
// LPCBUF - Character Buffer Register (Low Byte)

// Unibus Address 775416
// LPCKSM - Checksum Register         (High Byte)
// LPTDAT - Printer Data Register     (Low Byte)

// ************************************************************

// LP20 Register Definitions
#define LPREG(reg) lp20->reg
#define LPCSRA     LPREG(lpcsra)
#define LPCSRB     LPREG(lpcsra)
#define LPBSAD     LPREG(lpbsad)
#define LPBCTR     LPREG(lpbctr)
#define LPPCTR     LPREG(lppctr)
#define LPRAMD     LPREG(lpramd)
#define LPCBUF     LPREG(lpcbuf)
#define LPCCTR     LPREG(lpcctr)
#define LPTDAT     LPREG(lptdat)
#define LPCKSM     LPREG(lpcksm)

#define TXRAM(adr) LPREG(txram[adr])
#define DAVFU(adr) LPREG(davfu[adr])

// I/O Mapping Settings
#define LP20_NREGS   8
#define LP20_NVECS   1
#define LP20_CSRADDR 0775400
#define LP20_INT     0
#define LP20_VEC     0

#define LP_WIDTH     132  // Line Printer Width Size

// Translation RAM
#define TX_SIZE      256
#define TX_AMASK     (TX_SIZE - 1)
#define TX_DMASK     07777

// DA Vertical Format Unit RAM
#define DV_SIZE      143  // DAVFU Size
#define DV_DMASK     077  // Data Mask per Byte
#define DV_TOF       0    // Top of Form Channel
#define DV_MAX       11   // Max Channel Number

typedef struct lp20_Device LP20_DEVICE;
struct lp20_Device {
	UNIT Unit; // Unit Header Information

	void      *Device;
	void      *System;
	UQ_CALL   *Callback;
	MAP_IO    ioMap;
	CLK_QUEUE svcTimer;

	// Internal LP20 Area
	uint16 txram[TX_SIZE];  // Translation Memory
	uint16 davfu[DV_SIZE];  // DAVFU Memory
	uint16 dvptr;           // DAVFU Pointer
	uint16 dvlen;           // DAVFU Length

	// LP20 Registers
	uint16 lpcsra; // Control and Status Register A
	uint16 lpcsrb; // Control and Status Register B
	uint16 lpbsad; // DMA Bus Address Register
	uint16 lpbctr; // DMA Byte Counter Register
	uint16 lppctr; // Page Counter Register
	uint16 lpramd; // RAM Data Register
	uint8  lpcbuf; // Character Buffer Register (Low Byte)
	uint8  lpcctr; // Column Counter Register   (High Byte)
	uint8  lptdat; // Printer Data Register     (Low Byte)
	uint8  lpcksm; // Checksum Register         (High Byte)
};
