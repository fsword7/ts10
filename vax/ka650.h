// ka650.h - KA650/KA655 System Definitions
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


// System Information

#define CVAX_SID   (10 << 24) // Processor Type
#define CVAX_REV   6          // Microcode Revision
#define CON_PWRUP  (3 << 8)   // Power-up State

// TODR - Time of Day Register
// Note: Increment by one each 10 msec.

#define TODR_BASE   (1u << 28)
#define TODR_SEC    100

// ICCS - Interval Count Control Status
#define ICCS_IE     0x00000040 // Interrupt Enable
#define ICCS_WMASK  0x00000040 // Write Mask
#define ICCS_SECOND 100        // Tick each second
#define ICCS_TICK   10         // Tick each 10 microseconds
#define ICCS_NVECS  1          // Number of Vectors
#define ICCS_IPL    UQ_BR6     // Interrupt Level BR6
#define ICCS_VEC    SCB_TIMER  // System Vector

// Programmable Timer #1 - Interrupts
#define TMR0_NVECS  1
#define TMR0_IPL    UQ_BR4
#define TMR0_PRIO   255

// Programmable Timer #2 - Interrupts
#define TMR1_NVECS  1
#define TMR1_IPL    UQ_BR4
#define TMR1_PRIO   255

// CADR (IPR 37) - Cache Disable Register

#define CADR_S1E  0x00000040 // Set 1 Cache Enable
#define CADR_ISE  0x00000020 // I-Stream Cache Enable
#define CADR_DSE  0x00000010 // D-Stream Cache Enable
#define CADR_MBO  0x0000000C // Unused - Always 1s
#define CADR_WW   0x00000002 // Write Wrong Parity
#define CADR_DIA  0x00000001 // Diagnostic Mode
#define CADR_RW   0x000000F3 // Read/Write Mask

// MSER (IPR 39) - Memory System Error Register

#define MSER_HM  0x00000080 // Hit/Miss 
#define MSER_DSL 0x00000040 // DAL Parity Error
#define MSER_MCD 0x00000020 // Machine Check DAL Parity Error
#define MSER_MCC 0x00000010 // Machine Check Cache Parity Error
#define MSER_DAT 0x00000002 // Data Parity Error
#define MSER_TAG 0x00000001 // Tag Parity Error

// CONPSL (IPR 42) - Saved PSL Register

#define CONPSL_MASK    0xFFFF00FF // Processor Status Mask
#define CONPSL_MAPEN   0x00008000 // MAPEN<0> Bit
#define CONPSL_INVALID 0x00004000 // Invalid PSL
#define CONPSL_RESTART 0x00003F00 // Restart Code

// Machine Check Codes
#define MCHK_TBM_P0  0x05 // PPTE in P0
#define MCHK_TBM_P1  0x06 // PPTE in P1
#define MCHK_M0_P0   0x07 // PPTE in P0
#define MCHK_M0_P1   0x08 // PPTE in P1
#define MCHK_INTIPL  0x09 // Invalid Interrupt Request
#define MCHK_READ    0x80 // Read Check
#define MCHK_WRITE   0x82 // Write Check

// Halt Action Codes

#define HALT_EXTERN  0x02 // External Halt - HALT Button Depressed
#define HALT_PON     0x03 // Initial Power On
#define HALT_ISNV    0x04 // Interrupt Stack Not Valid during Exception
#define HALT_MCHK    0x05 // Machine Check during Normal Exception
#define HALT_INST    0x06 // HALT Instruction in Kernel Mode
#define HALT_SCB11   0x07 // SCB Vector <1:0> = 11
#define HALT_SCB10   0x08 // SCB Vector <1:0> = 10
#define HALT_CHMFIS  0x0A // CHMx executed while on interrupt stack
#define HALT_CHMTIS  0x0B // CHMx executed to the interrupt stack
#define HALT_AMCHK   0x10 // ACT/TNV During Machine Check
#define HALT_AKSNV   0x11 // ACV/TNV During Kernel Stack Not Valid
#define HALT_DMCHK   0x12 // Machine Check During Machine Check
#define HALT_MKSNV   0x13 // Machine Check During Kernel Stack Not Valid
#define HALT_INIE0   0x19 // PSL<26:24> = 101 during Interrupt/Exception
#define HALT_INIE1   0x1A // PSL<26:24> = 110 during Interrupt/Exception
#define HALT_INIE2   0x1B // PSL<26:24> = 111 during Interrupt/Exception
#define HALT_REI0    0x1D // PSL<26:24> = 101 during REI instruction
#define HALT_REI1    0x1E // PSL<26:24> = 110 during REI instruction
#define HALT_REI2    0x1F // PSL<26:24> = 111 during REI instruction

// SID -  System Identification
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      Type     |            Reserved           |   Microcode   |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//

#define SID_CVAX   0x0A000000 // CVAX Series Processor Type

// System Type Register on location 20040004 (in KA650/KA655 ROM Firmware)
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   Sys Type    |  Rev Level    | Sys Sub Type  |   Reserved    | 
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0

#define STR_ADDR   0x20040004 // Address of System Type Register
#define STR_SYSTYP 0xFF000000 // System Type Code
#define STR_REVLVL 0x00FF0000 // Revision Code (KA650/KA655 Firmware)
#define STR_SYSSUB 0x0000FF00 // System Subtype Code

// Memory Space (Up to 64MB main memory)

#define RAM_BASE  0
#define RAM_WIDTH 26 
#define RAM_BITS  RAM_WIDTH
#define RAM_SIZE  (1u << RAM_WIDTH)
#define RAM_MASK  (1 - RAM_SIZE)
#define RAM_END   (RAM_BASE + RAM_SIZE)

// ROM Space

#define ROM_BASE  0x20040000
#define ROM_WIDTH 17
#define ROM_SIZE  (1u << ROM_WIDTH)
#define ROM_MASK  (ROM_SIZE - 1)
#define ROM_END   (ROM_BASE + (ROM_SIZE << 2))

// Local Register Space

#define REG_BASE  0x20080000          // Register Address Base
#define REG_WIDTH 19                  // Register Address Length
#define REG_SIZE  (1u << REG_WIDTH)   // Register Address Size

// CMCTL - Memory Controller

#define CMCTL_BASE (REG_BASE + 0x100)        // CMCTL Address Base
#define CMCTL_NREG 18                        // 18 CMCTL Registers
#define CMCTL_SIZE (CMCTL_NREG << 2)         // CMCTL Address Length
#define CMCTL_END  (CMCTL_BASE + CMCTL_SIZE) // CMCTL Address End

#define nCMERR     16 // CMCTL Error Register
#define nCMCSR     17 // CMCTL Control/Status Register

// CMCTL Configuration Registers

#define CMCNF_VLD   0x80000000  // Address Valid
#define CMCNF_BA    0x1FF00000  // Base Address
#define CMCNF_LOCK  0x00000040  // Lock
#define CMCNF_SRQ   0x00000020  // Signature Request
#define CMCNF_SIG   0x0000001F  // Signature

#define CMCNF_RW    (CMCNF_VLD | CMCNF_BA) // Read/Write Access
#define CMCNF_MASK  (CMCNF_RW | CMCNF_SIG)

#define MEM_BANK    (1 << 22)   // Bank Size - 4 MB
#define MEM_SIG     0x17        // ECC, 4 x 4 MB

// CMCTL Error Register

#define CMERR_RDS 0x80000000 // Uncorrected Error
#define CMERR_FRQ 0x40000000 // 2nd RDS
#define CMERR_CRD 0x20000000 // CRD Error
#define CMERR_PAG 0x1FFFFC00 // Page Address
#define CMERR_DMA 0x00000100 // DMA Error
#define CMERR_BUS 0x00000080 // Bus Error
#define CMERR_SYN 0x0000007F // Syndrome

#define CMERR_W1C (CMERR_RDS | CMERR_FRQ | CMERR_CRD | \
                   CMERR_DMA | CMERR_BUS)

// CMCTL Control/Status Register

#define CMCSR_PMI 0x00002000 // PMI Speed
#define CMCSR_CRD 0x00001000 // Enable CRD Interrupt
#define CMCSR_FRF 0x00000800 // Force Reference
#define CMCSR_DET 0x00000400 // Dis Error
#define CMCSR_FDT 0x00000200 // Fast Diagnostic
#define CMCSR_DCM 0x00000080 // Diagnostic Mode
#define CMCSR_SYN 0x0000007F // Syndrome

#define CMCSR_MASK (CMCSR_PMI | CMCSR_CRD | CMCSR_DET | \
                    CMCSR_FDT | CMCSR_DCM | CMCSR_SYN)

// KA650 Board Registers

#define KA_BASE   (REG_BASE + 0x4000) // KA650 Address Base
#define KA_WIDTH  3                   // KA650 Address Width
#define KA_SIZE   (1u << KA_WIDTH)    // KA650 Address Size
#define KA_END    (KA_BASE + KA_SIZE)

#define nCACR     0
#define nBDR      1

// KA650 Cache Control Register

#define CACR_DRO     0x00FFFF00 // Diagnostics Bits
#define CACR_P_DPAR  24
#define CACR_FIXED   0x00000040 // Fixed Bits
#define CACR_CPE     0x00000020 // Parity Error
#define CACR_CEN     0x00000010 // Cache Enable
#define CACR_DPE     0x00000004 // Disable Parity
#define CACR_WWP     0x00000002 // Write Wrong Parity
#define CACR_DIAG    0x00000001 // Diagnostic Mode

#define CACR_W1C     (CACR_CPE)
#define CACR_RW      (CACR_CEN|CACR_DPE|CACR_WWP|CACR_DIAG)

// SSC Registers

#define SSC_BASE     0x20140000 // SSC Address Base
#define SSC_SIZE     0x150      // SSC Address Size
#define SSC_END      (SSC_BASE + SSC_SIZE)

#define nSSC_BASE    0x00 // Base Address
#define nSSC_CR      0x04 // Configuration Register
#define nSSC_BTO     0x08 // CDAL
#define nSSC_DLEDR   0x0C // Display LED Register
#define nSSC_TODR    0x1B // Time of Day Register
#define nSSC_CSRS    0x1C
#define nSSC_CSRD    0x1D
#define nSSC_CSTS    0x1E
#define nSSC_CSTD    0x1F
#define nSSC_RXCS    0x20 // Receive Control/Status Register
#define nSSC_RXDB    0x21 // Receive Data Buffer
#define nSSC_TXCS    0x22 // Transmit Control/Status Register
#define nSSC_TXDB    0x23 // Transmit Data Buffer
#define nSSC_TCR0    0x40 // Timer 0 Control/Status Register
#define nSSC_TIR0    0x41 // Timer 0 Interval Register
#define nSSC_TNIR0   0x42 // Timer 0 Next Interval Register
#define nSSC_TIVR0   0x43 // Timer 0 Interrupt Vector Register
#define nSSC_TCR1    0x44 // Timer 1 Control/Status Register
#define nSSC_TIR1    0x45 // Timer 1 Interval Register
#define nSSC_TNIR1   0x46 // Timer 1 Next Interval Register
#define nSSC_TIVR1   0x47 // Timer 1 Interrupt Vector Register
#define nSSC_AD0MAT  0x4C
#define nSSC_AD0MSK  0x4D
#define nSSC_AD1MAT  0x50
#define nSSC_AD1MSK  0x51

// SSC Base Register
#define SSCBASE_ADDR 0x1FFFFC00 // Base Address

#define SSCBASE_RW   0x1FFFFC00 // Read/Write Access
#define SSCBASE_MBO  0x20000000 // Must Be One

// SSC Configuration Register
#define SSCCNF_BLO   0x80000000 // Battery Low
#define SSCCNF_IVD   0x08000000 // 
#define SSCCNF_IPL   0x03000000 //
#define SSCCNF_ROM   0x00F70000 // ROM Parameters
#define SSCCNF_CTLP  0x00008000 // Ctrl-P Enable
#define SSCCNF_BAUD  0x00007700 // Baud Rates
#define SSCCNF_ADS   0x00000077 // Address

#define SSCCNF_W1C   (SSCCNF_BLO) // Clear Mask
#define SSCCNF_RW    0x0BF7F777   // Read/Write Mask

// SSC Bus Timeout Register
#define SSCBTO_BTO   0x80000000   // Timeout
#define SSCBTO_RWT   0x40000000   // Read/Write
#define SSCBTO_INTV  0x00FFFFFF

#define SSCBTO_W1C   (SSCBTO_BTO|SSCBTO_RWT)
#define SSCBTO_RW    (SSCBTO_INTV)

// SSC_DLEDR - Display LED Register

#define SSCLED_MASK  0x0000000F

// SSC Timer

#define TCR_ERROR    0x80000000  // Timer Error
#define TCR_DONE     0x00000080  // Done
#define TCR_IE       0x00000040  // Interrupt Enable
#define TCR_SINGLE   0x00000020  // Single
#define TCR_TRANSFER 0x00000010  // Transfer
#define TCR_STOP     0x00000004  // Stop
#define TCR_RUN      0x00000001  // Run
#define TCR_W1C      (TCR_ERROR|TCR_DONE)
#define TCR_RW       (TCR_IE|TCR_STOP|TCR_RUN)

#define TMR_TICK     1000      // Each 1 kHz
#define TMR_HZ       1000      // 1000 Hz

#define TIVR_VEC   0x000003FC  // Interrupt Vector

// SCC Address Strobes

#define ADS_MASK     0x3FFFFFFC // Match or mask

// Non-volatile RAM - 1024 bytes

#define NVR_BASE     (SSC_BASE + 0x400)    // NVR Address Base
#define NVR_WIDTH    10                    // NVR Address Width
#define NVR_SIZE     (1u << NVR_WIDTH)     // NVR Address Length
#define NVR_MASK     (NVR_SIZE - 1)        // NVR Address Mask
#define NVR_END      (NVR_BASE + NVR_SIZE) // NVR Address End

#define IN_NVR(x)    (((x) >= NVR_BASE) && ((x) < NVR_END))

// Cache Diagnostic Space

#define CDA_WIDTH     16                    // Cache Data Address Width
#define CDA_SIZE      (1u << CDA_WIDTH)     // Cache Data Address Length
#define CDA_MASK      (CDA_SIZE - 1)        // Cache Data Address Mask

#define CTA_WIDTH     10                    // Cache Tag Address Width
#define CTA_SIZE      (1u << CTA_WIDTH)     // Cache Tag Address Length
#define CTA_MASK      (CTA_SIZE - 1)        // Cache Tag Address Mask

#define CDG_BASE      0x10000000            // Cache Diag Address Base
#define CDG_SIZE      (CDA_SIZE * CTA_SIZE) // Cache Diag Address Length
#define CDG_END       (CDG_BASE + CDG_SIZE) // Cache Diag Address End

#define CTG_V         (1u << (CTA_WIDTH + 0)) // Valid Bit
#define CTG_WP        (1u << (CTA_WIDTH + 1)) // Wrong Parity Bit

#define CDG_GETROW(x) (((x) & CDA_MASK) >> 2)
#define CDG_GETTAG(x) (((x) >> CDA_WIDTH) & CTA_MASK)

#define IN_CDG(x)     (((x) >= CDG_BASE) && ((x) < CDG_END))

// CQBIC Q22-Bus I/O Page Area

#define IO_BASE       0x20000000          // I/O Address Base
#define IO_WIDTH      13                  // I/O Address Width
#define IO_SIZE       (1u << IO_WIDTH)    // I/O Address Length
#define IO_MASK       (IO_SIZE - 1)       // I/O Address Mask
#define IO_END        (IO_BASE + IO_SIZE) // I/O Address End

#define IN_IO(x)      (((x) >= IO_BASE) && ((x) < IO_END))

// CQBIC Q22-Bus Register Area

#define CQBIC_BASE    0x20080000 // CQBIC Address - Base
#define CQBIC_SIZE    (5 << 2)   // CQBIC - 5 Registers
#define CQBIC_END     (CQBIC_BASE + CQBIC_SIZE)

// CQBIC Q22-Bus Map Area

#define CQMAP_BASE    (REG_BASE + 0x8000)       // CQMAP Address Base
#define CQMAP_WIDTH   15                        // CQMAP Address Width
#define CQMAP_SIZE    (1u << CQMAP_WIDTH)       // CQMAP Address Length
#define CQMAP_MASK    (CQMAP_SIZE - 1)          // CQMAP Address Mask
#define CQMAP_END     (CQMAP_BASE + CQMAP_SIZE) // CQMAP Address End

// CQBIC Q22-Bus Memory Area

#define CQMEM_BASE    0x30000000                // CQMEM Address Base
#define CQMEM_WIDTH   22                        // CQMEM Address Width
#define CQMEM_SIZE    (1u << CQMEM_WIDTH)       // CQMEM Address Length
#define CQMEM_MASK    (CQMEM_SIZE - 1)          // CQMEM Address Mask
#define CQMEM_END     (CQMEM_BASE + CQMEM_SIZE) // CQMEM Address End

// Note: It only works for little endian machines at this time.

#define IN_RAM(addr) \
	((addr) < vax->sizeRAM)
#define IN_ROM(addr) \
	(((addr) >= 0x20040000) && ((addr) < 0x20080000))

// Aligned RAM Access
#define BMEM(addr)  ((uint8 *)vax->RAM)[addr]
#define WMEM(addr)  ((uint16 *)vax->RAM)[addr]
#define LMEM(addr)  ((uint32 *)vax->RAM)[addr]

// Aligned ROM Access
#define BROM(addr) ((uint8 *)vax->ROM)[addr]
#define WROM(addr) ((uint16 *)vax->ROM)[addr]
#define LROM(addr) ((uint32 *)vax->ROM)[addr]

// Aligned Register Access
#define BREG(x, addr) ((uint8 *)(x))[addr]
#define WREG(x, addr) ((uint16 *)(x))[addr]
#define LREG(x, addr) ((uint32 *)(x))[addr]

typedef struct {
	VAX_CPU   cpu;

	MAP_IO    ioClock;     // Clock - Interrupts
	CLK_QUEUE ClockTimer;  // Clock Timer (10 msecs)
	uint32    TickCount;
	UQ_CALL   *Callback;
	void      *qba;

	// KA Registers
	uint32    cacr;     // Cache Register
	uint32    bdr;      // Boot and Diagnostic Register

	// SSC Registers
	uint32    sscBase;    // Base Register
	uint32    sscConfig;  // Configuration Register
	uint32    sscTimeout; // Bus Timerout Register
	uint32    sscLED;     // Display LED Register
	uint32    sscRAM[NVR_SIZE >> 2]; // Non-Volatile RAM

	// SSC Timer Registers
	MAP_IO    ioTimer[2];  // Interrupts
	CLK_QUEUE Timers[2];   // Programmable Timers
	uint32    tmrTick;     // Calibrated Tick
	uint32    tcr[2];      // Configuration Register
	uint32    tir[2];      // Interval Register
	uint32    tnir[2];     // Next Interval Register
	uint32    tivr[2];     // Interrupt Vector Register
	uint32    tmr_inc[2];
	uint32    tmr_sav[2];

	// SSC Address strobes
	uint32    admat[2];    // Address Match
	uint32    admsk[2];    // Address Mask

	// CMCTL Memory Registers
	uint32    cmctl[CMCTL_NREG];

	// CDG Cache Registers
	uint32    cdgData[CDA_SIZE >> 2];

} KA650_DEVICE;
