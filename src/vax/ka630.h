// ka630.h - KA630 Processor (MicroVAX II series) Definitions
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

// KA630 - MicroVAX II Memory Map
//
// 00000000 +-------------------------------------+
//          |           Main Memory               |
//          |- - - - - - - - - - - - - - - - - - -|
//          |        Up to 16 MB Memory           |
// 01000000 +-------------------------------------+
//          |             Reserved                |
// 20000000 +-------------------------------------+
//          |         Q22-Bus I/O space           |
// 20002000 +-------------------------------------+
//          |             Reserved                |
// 20040000 +-------------------------------------+
//          |               ROM                   |
// 20080000 +-------------------------------------+
//          |      Local I/O Register Area        |
// 200C0000 +-------------------------------------+
//          |             Reserved                |
// 30000000 +-------------------------------------+
//          |        Q22-Bus Memory Space         |
// 303FFFFF +-------------------------------------+
//          |             Reserved                |
// 3FFFFFFF +-------------------------------------+
//
// Local Memory Area:
//
//   Main Memory       - 00000000-00FFFFFF (up to 16 MB memory)
//
// Local I/O Register Area:
//
//   CPU Registers     - 20080000-2008000F (4 16-bit CPU Registers)
//   Clock Registers   - 200B8000-2008007F (64 8-bit Clock Registers)
//
//   Q22 I/O Page      - 20000000-20001FFF (8 KB I/O Space)
//   Q22 Map Registers - 20088000-2008FFFF (8096 32-bit Map Registers)
//   Q22 Memory Space  - 30000000-3003FFFF (4 MB Memory Space)

#define KA630_DTNAME  "KA630"
#define KA630_NAME    "KA630 (MicroVAX II) Emulator"
#define KA630_VERSION "v0.8 (Late Alpha)"

// KA630's Q22-Bus Inferface
extern DEVICE qba_Device;

// ***************************************
// ******** KA630 CPU Registers **********
// ***************************************

#define CPUREG(reg) ka630->cpuRegs[reg]

#define nCPUREG 4 // Number of CPU Registers

#define nBDR    0 // Boot Diagnostics Register
#define nMSER   1
#define nCEAR   2
#define nDEAR   3

#define BDR  CPUREG(nBDR)
#define MSER CPUREG(nMSER)
#define CEAR CPUREG(nCEAR)
#define DEAR CPUREG(nDEAR)

#ifdef DEBUG
static char *cpuNames[] = { "BDR", "MSER", "CEAR", "DEAR" };
#endif /* DEBUG */

// 20080000 - KA630_BDR - Boot Diagnostics Register

#define BDR_PWROK  0x8000 // (R)   Power Ok
#define BDR_HLTENB 0x4000 // (R)   Halt Enable Switch
#define BDR_CPU    0x0C00 // (R)   CPU Arbitration ID
#define BDR_BDG    0x0300 // (R)   Language, Test, and Loopback Switch
#define BDR_DSPL   0x000F // (R/W) LED Display (0-F)

#define BDR_WMASK  0x000F // Write Mask

// 20080004 - KA630_MSER - Memory System Error Register

#define MSER_CD    0x0300 // (R)   Memory Code
#define MSER_NXM   0x0080 // (R/C) CPU Non-Existant Memory
#define MSER_LPE   0x0040 // (R/C) CPU Local Address Space Parity Error
#define MSER_QPE   0x0020 // (R/C) CPU Q22-Bus Address Space Parity Error
#define MSER_DQPE  0x0010 // (R/C) DMA Q22-Bus Address Space Parity Error
#define MSER_LEB   0x0008 // (R/C) Memory System Lost Error Bit
#define MSER_WWP   0x0002 // (R/W) Write Wrong Parity (1 = Bad, 0 = Good)
#define MSER_PEN   0x0001 // (R/W) Parity Enable

#define MSER_CMASK 0x00F8 // Clear Mask
#define MSER_WMASK 0x0003 // Write Mask

// Memory Code for MSER_CD field

#define MCD_Q22   0x00 // Q-22 Memory or Device
#define MCD_MEM   0x01 // KA630 Memory On-Board
#define MCD_ME1   0x02 // Memory Expansion Module 1
#define MCD_ME2   0x03 // Memory Expansion Module 2

// 20080008 - KA630_CEAR - CPU Error Address Register
// Valid if CPU LPE or CPU QPE are set in MSER register.

#define CEAR_PAGE  0x7FFF // (R) Local Memory Address Bits <23:9>

// 2008000C - KA630_DEAR - DMA Error Address Register
// Valid if DMA QPE is set in MSER register.

#define DEAR_PAGE  0x7FFF // (R) Local Memory Address Bits <23:9>

// ***********************************************
// ******** KA630 TOY (Clock) Registers **********
// ***********************************************

#define CLKREG(reg) ka630->clkRegs[reg << 1]

#define nCLKREG 64 // Number of Clock registers in bytes

#define nSEC       0 // Second
#define nSECLARM   1 // Second Alarm
#define nMINUTE    2 // Minute
#define nMINALRM   3 // Minute Alarm
#define nHOUR      4 // Hour
#define nHOURALRM  5 // Hour Alarm
#define nDAYWK     6 // Day of Week
#define nDAYMON    7 // Day of Month
#define nMONTH     8 // Month
#define nYEAR      9 // Year
#define nCSRA     10 // Control and Status Register #0
#define nCSRB     11 // Control and Status Register #1
#define nCSRC     12 // Control and Status Register #2
#define nCSRD     13 // Control and Status Register #3
#define nCPMBX    14 // Control Panel Mailbox

#define SECOND   CLKREG(nSECOND)
#define SECALRM  CLKREG(nSECALRM)
#define MINUTE   CLKREG(nMINUTE)
#define MINALRN  CLKREG(nMINALRM)
#define HOUR     CLKREG(nHOUR)
#define HOURALRM CLKREG(nHOURALRM)
#define DAYWK    CLKREG(nDAYWK)
#define DAYMON   CLKREG(nDAYMON)
#define MONTH    CLKREG(nMONTH)
#define YEAR     CLKREG(nYEAR)
#define CSRA     CLKREG(nCSRA)
#define CSRB     CLKREG(nCSRB)
#define CSRC     CLKREG(nCSRC)
#define CSRD     CLKREG(nCSRD)
#define CPMBX    CLKREG(nCPMBX)

// 200B8000 - TOY Clock Chip (64 8-bit Registers with scratch pad RAM)
//
//                     Address
// Reg Function        Offset   Comment
//  0  Second          00       0-59 Seconds
//  1  Second Alarm    02       0-59 Seconds
//  2  Minute          04       0-59 Minutes
//  3  Minute Alarm    06       0-59 Minutes
//  4  Hour            08       0-23 Hours
//  5  Hour Alarm      0A       0-23 Hours
//  6  Day of Week     0C       1-7  Day of Week
//  7  Day of Month    0E       1-31 Day of Month
//  8  Month           10       1-12 Months
//  9  Year            12       0-99 Years
// 10  CSRA            14
// 11  CSRB            16
// 12  CSRC            18
// 13  CSRD            1A
// 14  RAM Byte 1      1C       Scratch Pad RAM  (CPMBX)
// ...............     ..       ...........
// 63  RAM Byte 50     7E       Last byte

// TOY Register 10 - CSRA   Control and Status Register #0

#define CSRA_UIP    0x80 // (R)   Update in Progress Flag
#define CSRA_DV     0x70 // (R/W) DV0-DV2 ??
#define CSRA_RS     0x0F // (R/W) Not used??

// TOY Register 11 - CSRB   Control and Status Register #1

#define CSRB_SET    0x80 // Set Time (0 = Normal, 1 = Stop to update)
#define CSRB_PIE    0x40 // Peroidic Interrupt Enable
#define CSRB_AIE    0x20 // Alarm Interrupt Enable
#define CSRB_UIE    0x10 // Update Interrupt Enable
#define CSRB_SQWE   0x08 // Square-Wave Enable
#define CSRB_DM     0x04 // Data Mode  (0 = Binary, 1 = BCD)
#define CSRB_24     0x02 // 12/24 Hour (0 = 12, 1 = 24)
#define CSRB_DS     0x01 // Daylight Saving Enable

// TOY Register 12 - CSRC   Control and Status Register #2

// TOY Register 13 - CSRD   Control and Status Register #3

// TOY Register 14 - CPMBX  Control Panel Mailbox
// (First byte in scratch pad RAM)

#define CPMBX_LANG  0xF0 // Language
#define CPMBX_RIP   0x08 // Restart in Progress Flag
#define CPMBX_BIP   0x04 // Bootstrap in Progress Flag
#define CPMBX_HALT  0x03 // Halt Actions

// Console Message          Halt Actions
// Text Language
//
// 7 6 5 4  Langauge        1 0  Halt Action
// -------  --------        ---  -----------
// 0 0 0 1  German          0 0  Use Halt Enable (BDR 14 bit)
// 0 0 1 0  English         0 1  Restart, if that fails, Halt
// 0 0 1 1  Spanish         1 0  Reboot, if that fails, Halt
// 0 1 0 0  French          1 1  Halt
// 0 1 0 1  Italian
// 0 1 1 0  Danish
// 0 1 1 1  Dutch
// 1 0 0 0  Finish
// 1 0 0 1  Norwegian
// 1 0 1 0  Swedish
// 1 0 1 1  Portuguese 

// *******************************************
// ******* KA630 Machine Check Code **********
// *******************************************

#define MCHK_FSD 0x01 // Impossible Microcode State (FSD)
#define MCHK_SSD 0x02 // Impossible Microcode State (SSD)
#define MCHK_FP0 0x03 // Undefined FPU Error Code 0
#define MCHK_FP7 0x04 // Undefined FPU Error Code 7
#define MCHK_TBM 0x05 // Undefined Memory Management Status (TB Miss)
#define MCHK_MZ  0x06 // Undefined Memory Management Status (M = 0)
#define MCHK_P0  0x07 // Process PTE in P0 Space
#define MCHK_P1  0x08 // Process PTE in P1 Space
#define MCHK_IPL 0x09 // Undefined Interrupt ID Code
#define MCHK_RBV 0x80 // Read Bus Error, Address Parameter is Virtual
#define MCHK_RBP 0x81 // Read Bus Error, Address Parameter is Physical
#define MCHK_WBV 0x82 // Write Bus Error, Address Parameter is Virtual
#define MCHK_WBP 0x83 // Wirte Bus Error, Address Parameter is Physical

// *******************************************
// ************* KA630 Halt Code *************
// *******************************************

#define HLT_SWITCH 0x02 // HALT L Asserted (Halt Button)
#define HLT_PON    0x03 // Initial Power On
#define HLT_ISNV   0x04 // Interrupt Stack Not Valid During Exception
#define HLT_MCHK   0x05 // Machine-Check During Machine-Check/KSNV
#define HLT_INST   0x06 // HALT Instruction Executed in Kernel Mode
#define HLT_SCB11  0x07 // SCB Vector <1:0> = 11
#define HLT_SCB10  0x08 // SCB Vector <1:0> = 10
#define HLT_CHM    0x0A // CHMx Executed While On Interrupt Stack
#define HLT_MMCHK  0x10 // ACV/TNV During Machine-Check Exception
#define HLT_MKSNV  0x11 // ACV/TNV During Kernel-Stack-Not-Valid Exception

// Note: It only works for little endian machines at this time.

#define IN_RAM(addr) \
	((addr) < vax->sizeRAM)

#define IN_ROM(addr) \
	(((addr) >= 0x20040000) && ((addr) < 0x20080000))
#define IN_CPUREG(addr) \
	(((addr) >= 0x20080000) && ((addr) < 0x20088000))
#define IN_CLKREG(addr) \
	(((addr) >= 0x200B8000) && ((addr) < 0x200C0000))

#define IN_Q22IO(addr) \
	(((addr) >= 0x20000000) && ((addr) < 0x20001FFF))
#define IN_Q22MAP(addr) \
	(((addr) >= 0x20008800) && ((addr) < 0x2008FFFF))
#define IN_Q22MEM(addr) \
	(((addr) >= 0x30000000) && ((addr) < 0x303FFFFF))

// Aligned RAM Access
#define BMEM(addr)  ((uint8 *)vax->RAM)[addr]
#define WMEM(addr)  ((uint16 *)vax->RAM)[addr]
#define LMEM(addr)  ((uint32 *)vax->RAM)[addr]

// Unaligned RAM Access
#define BMEMU(addr) *(uint8 *)(&vax->RAM[addr])
#define WMEMU(addr) *(uint16 *)(&vax->RAM[addr])
#define LMEMU(addr) *(uint32 *)(&vax->RAM[addr])

// Aligned ROM Access
#define BROM(addr) ((uint8 *)vax->ROM)[addr]
#define WROM(addr) ((uint16 *)vax->ROM)[addr]
#define LROM(addr) ((uint32 *)vax->ROM)[addr]

// Unaligned ROM Access
#define BROMU(addr) *(uint8 *)(&vax->ROM[addr])
#define WROMU(addr) *(uint16 *)(&vax->ROM[addr])
#define LROMU(addr) *(uint32 *)(&vax->ROM[addr])

// Aligned Register Access
#define BREG(x, addr) ((uint8 *)(x))[addr]
#define WREG(x, addr) ((uint16 *)(x))[addr]
#define LREG(x, addr) ((uint32 *)(x))[addr]

// Unaligned Register Access
#define BREGU(x, addr) *(uint8 *)(&(x)[addr])
#define WREGU(x, addr) *(uint16 *)(&(x)[addr])
#define LREGU(x, addr) *(uint32 *)(&(x)[addr])

#define LONG OP_LONG
#define WORD OP_WORD
#define BYTE OP_BYTE

typedef struct {
	VAX_CPU    cpu;        // Generic VAX Processor

	MAP_IO     ioClock;    // Clock Interrupt
	CLK_QUEUE  Timer;      // Clock Timer
	int32      TickCount;  // Count each tick
	UQ_CALL    *Callback;  // Unibus/Qbus
	void       *qba;       // QBA Interface for KA630.

#ifdef TEST_PARITY
	// That is stub for required test #3 on KA630 firmware
	uint32     cpuAddr;    // CPU Parity Check Test
	uint32     cpuState;   // CPU Test State

	uint32     q22Addr1;   // Q22 Parity Check Test #1
	uint32     q22Addr2;   // Q22 Parity Check Test #2
	uint32     q22State;   // Q22 Test State
#endif /* TEST_PARITY */

	// Local I/O Registers Area (in bytes)
	uint16 cpuRegs[4];   // KA630 CPU Registers
	uint8  clkRegs[128]; // KA630 Clock Registers (TOY) w/scratch pad RAM
	uint32 clkZero;

	// Map Registers for Q22-Bus Interface
//	uint32 mapRegs[8192]; // 8192 Map Registers
} KA630_DEVICE;

