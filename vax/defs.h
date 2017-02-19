// defs.h - definitions for VAX Processor
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

#include "emu/defs.h"
#include <setjmp.h>

// Phsyical Memory defintions
#define PASIZE   30
#define PAMASK   ((1 << PASIZE) - 1)

// Memory Management
#include "vax/vm.h"

// Byte macro definitions
#define BMASK    0x000000FF
#define BSIGN    0x00000080
#define BMAX     127
#define BMIN     -128

#define WMASK    0x0000FFFF
#define WSIGN    0x00008000
#define WMAX     32767
#define WMIN     -32768
#define WALIGN   0xFFFFFFFE

#define LMASK    0xFFFFFFFF
#define LSIGN    0x80000000
#define LMAX     0x7FFFFFFF
#define LMIN     0x80000000
#define LALIGN   0xFFFFFFFC

#define FPSIGN   0x00008000

#define REGMASK  0xF

// Signed/Zero Extenstion defintions
//#define SXTB(x)  (((x) & BSIGN) ? ((x) | ~BMASK) : (x))
//#define SXTW(x)  (((x) & WSIGN) ? ((x) | ~WMASK) : (x))
//#define ZXTB(x)  ((x) & BSIGN)
//#define ZXTW(x)  ((x) & WSIGN)

#define SXTB(x)  ((int8)(x))
#define SXTW(x)  ((int16)(x))
#define SXTL(x)  ((int32)(x))

#define ZXTB(x)  ((uint8)(x))
#define ZXTW(x)  ((uint16)(x))
#define ZXTL(x)  ((uint32)(x))

// Result code definitions
#define VAX_OK       EMU_OK
#define VAX_NXM      EMU_NXM
#define VAX_RUN      EMU_RUN
#define VAX_HALT     EMU_HALT
#define VAX_SWHALT   EMU_SWHALT

#define MAX_OPREGS    9 //    9 Operand Registers
#define MAX_GREGS    16 //   16 General Registers
#define MAX_PREGS   256 //  256 Processor (Privileged) Registers
#define NUM_INST   1024 // 1024 Instructions Set Table
#define MAX_SPEC      6 //    6 Specificers

// Instruction definitons

#define ISTR_NORMAL  0x00000000 // Normal Instruction
#define ISTR_EMULATE 0x80000000 // Instruction is emulatable
#define ISTR_STRING  0x00000080 // String Instructions
#define ISTR_PACKED  0x00000040 // Packed Instructions
#define ISTR_VECTOR  0x00000020 // Vector Instructions
#define ISTR_FLOAT   0x00000010 // Floating Instructions
#define ISTR_OCTA    0x00000008 // Octaword/H_Floating Type
#define ISTR_QUAD    0x00000004 // Quadword/G_Floating Type
#define ISTR_WORD    0x00000002 // Word/D_Floating Type
#define ISTR_BYTE    0x00000001 // Byte/F_Floating Type

#define INST_EXTEND 0xFD // For two-bytes Instructions

// Operand type definitions
#define OP_BYTE    1 // Operand is a byte      (1 byte)
#define OP_WORD    2 // Operand is a word      (2 bytes)
#define OP_LONG    4 // Operand is a longword  (4 bytes)
#define OP_QUAD    8 // Operand is a quadword  (8 bytes)
#define OP_OCTA   16 // Operand is a octaword  (16 bytes)

#define OP_SCALE    0x000FF // Scale Mask
#define OP_FLOAT    0x00F00 // Operand is floating point (otherwise - integer)
#define OP_FFLOAT   0x00100 // Operand is F_floating type
#define OP_DFLOAT   0x00200 // Operand is D_floating type
#define OP_GFLOAT   0x00400 // Operand is G_floating type
#define OP_HFLOAT   0x00800 // Operand is H_floating type
#define OP_WRITE    0x01000 // Operand is writable (otherwise - readable)
#define OP_MODIFIED 0x02000 // Operand is modifiable
#define OP_IMMED    0x04000 // Operand is immediate
#define OP_ADDR     0x08000 // Operand is address
#define OP_VADDR    0x10000 // Operand is variable-length address
#define OP_BRANCH   0x20000 // Operand is a branch displacement

// Read-Only Integer Operands
#define RB    (OP_BYTE)
#define RW    (OP_WORD)
#define RL    (OP_LONG)
#define RQ    (OP_QUAD)
#define RO    (OP_OCTA)

// Read/Write Integer Operands
#define MB    (OP_MODIFIED|OP_BYTE)
#define MW    (OP_MODIFIED|OP_WORD)
#define ML    (OP_MODIFIED|OP_LONG)
#define MQ    (OP_MODIFIED|OP_QUAD)
#define MO    (OP_MODIFIED|OP_OCTA)

// Write-Only Integer Operands
#define WB    (OP_WRITE|OP_BYTE)
#define WW    (OP_WRITE|OP_WORD)
#define WL    (OP_WRITE|OP_LONG)
#define WQ    (OP_WRITE|OP_QUAD)
#define WO    (OP_WRITE|OP_OCTA)

// Read-Only Floating Operands
#define RF    (OP_FFLOAT|OP_LONG)
#define RD    (OP_DFLOAT|OP_QUAD)
#define RG    (OP_GFLOAT|OP_QUAD)
#define RH    (OP_HFLOAT|OP_OCTA)

// Read/Write Floating Operands
#define MF    (OP_FFLOAT|OP_MODIFIED|OP_LONG)
#define MD    (OP_DFLOAT|OP_MODIFIED|OP_QUAD)
#define MG    (OP_GFLOAT|OP_MODIFIED|OP_QUAD)
#define MH    (OP_HFLOAT|OP_MODIFIED|OP_OCTA)

// Write-Only Floating Operands
#define WF    (OP_FFLOAT|OP_WRITE|OP_LONG)
#define WD    (OP_DFLOAT|OP_WRITE|OP_QUAD)
#define WG    (OP_GFLOAT|OP_WRITE|OP_QUAD)
#define WH    (OP_HFLOAT|OP_WRITE|OP_OCTA)

// Address Operands
#define AB    (OP_ADDR|OP_BYTE)
#define AW    (OP_ADDR|OP_WORD)
#define AL    (OP_ADDR|OP_LONG)
#define AQ    (OP_ADDR|OP_QUAD)
#define AO    (OP_ADDR|OP_OCTA)
#define AF    (OP_ADDR|OP_FFLOAT|OP_LONG)
#define AD    (OP_ADDR|OP_DFLOAT|OP_QUAD)
#define AG    (OP_ADDR|OP_GFLOAT|OP_QUAD)
#define AH    (OP_ADDR|OP_HFLOAT|OP_OCTA)

// Branch/Misc Operands
#define BB    (OP_BRANCH|OP_BYTE)
#define BW    (OP_BRANCH|OP_WORD)
#define VB    (OP_VADDR|OP_BYTE)

// Immediate Operands
#define IB    (OP_IMMED|OP_BYTE)
#define IW    (OP_IMMED|OP_WORD)
#define IL    (OP_IMMED|OP_LONG)

// Operand mode definitions
#define OP_MMASK     0xF0
#define OP_RMASK     0x0F
#define OP_MEM       -1

#define LIT0  0x00 // Short Literal
#define LIT1  0x10 
#define LIT2  0x20 
#define LIT3  0x30 
#define IDX   0x40 // Indexed
#define REG   0x50 // Register
#define REGD  0x60 // Register Deferred
#define ADEC  0x70 // Autodecrement
#define AINC  0x80 // Autoincrement
#define AINCD 0x90 // Autoincrement Deferred
#define BDP   0xA0 // Byte Displacement
#define BDPD  0xB0 // Byte Displacement Deferred
#define WDP   0xC0 // Word Displacement
#define WDPD  0xD0 // Word Displacement Deferred
#define LDP   0xE0 // Longword Displacement
#define LDPD  0xF0 // Longword Displacement Deferred

// Access Mode Definitions for PSL, page tables, etc..
#define AM_KERNEL     0 // Kernel Mode
#define AM_EXECUTIVE  1 // Executive Mode
#define AM_SUPERVISOR 2 // Supervisor Mode
#define AM_USER       3 // User Mode
#define AM_INTERRUPT  4 // Interrupt Mode

// System Control Block Vectors
#define SCB_PASSIVE    0x00  // Passive Release
#define SCB_MCHK       0x04  // Machine Check
#define SCB_KSNV       0x08  // Kernel Stack Not Valid
#define SCB_POWER      0x0C  // Power Fail
#define SCB_RESIN      0x10  // Reserved or Privileged Instruction
#define SCB_XFC        0x14  // Customer Reserved Instruction (XFC)
#define SCB_RESOP      0x18  // Reserved Operand
#define SCB_RESAD      0x1C  // Reserved Address Mode
#define SCB_ACV        0x20  // Access-Control Violation
#define SCB_TNV        0x24  // Translation Not Valid
#define SCB_TP         0x28  // Trace Pending
#define SCB_BPT        0x2C  // Breakpoint Instruction
#define SCB_COMPAT     0x30  // Compatilbility
#define SCB_ARITH      0x34  // Arithmetic
#define SCB_CHMK       0x40  // Change Mode to Kernel
#define SCB_CHME       0x44  // Change Mode to Executive
#define SCB_CHMS       0x48  // Change Mode to Supervisor
#define SCB_CHMU       0x4C  // Change Mode to User
#define SCB_CRDERR     0x54  // CRD Error Interrupt
#define SCB_MEMERR     0x60  // Memory Error Interrupt
#define SCB_IPLSOFT    0x80  // Software Interrupt Base
#define SCB_SOFTWARE1  0x84  // Software Level 1
#define SCB_SOFTWARE2  0x88  // Software Level 2
#define SCB_SOFTWARE3  0x8C  // Software Level 3
#define SCB_SOFTWARE4  0x90  // Software Level 4
#define SCB_SOFTWARE5  0x94  // Software Level 5
#define SCB_SOFTWARE6  0x98  // Software Level 6
#define SCB_SOFTWARE7  0x9C  // Software Level 7
#define SCB_SOFTWARE8  0xA0  // Software Level 8
#define SCB_SOFTWARE9  0xA4  // Software Level 9
#define SCB_SOFTWARE10 0xA8  // Software Level A
#define SCB_SOFTWARE11 0xAC  // Software Level B
#define SCB_SOFTWARE12 0xB0  // Software Level C
#define SCB_SOFTWARE13 0xB4  // Software Level D
#define SCB_SOFTWARE14 0xB8  // Software Level E
#define SCB_SOFTWARE15 0xBC  // Software Level F
#define SCB_TIMER      0xC0  // Interval Timer
#define SCB_EMULATE    0xC8  // Subset Emulation
#define SCB_EMULFPD    0xCC  // Subset Emulation with FPD flag
#define SCB_CSREAD     0xF0  // Console Storage Read
#define SCB_CSWRITE    0xF4  // Console Storage Write
#define SCB_CTYIN      0xF8  // Console Terminal Read
#define SCB_CTYOUT     0xFC  // COnsole Terminal Write

#define SCB_ADDR       0xFFFFFFFC // SCB Address Mask
#define SCB_VECTOR     0xFFFFFFFC // SCB Vector Mask

// Exception/Interrupt Type
#define IE_SVE -1 // Severe Exception
#define IE_EXC 0  // Normal Exception
#define IE_INT 1  // Interrupt

// Arithmetic Exception Type Codes
#define TRAP_INTOVF   1 // Integer Overflow Trap
#define TRAP_INTDIV   2 // Integer Divide-By-Zero Trap
#define TRAP_FLTOVF   3 // Floating Overflow Trap
#define TRAP_FLTDIV   4 // Floating or Decimal Divide-By-Zero Trap
#define TRAP_FLTUND   5 // Floating Underflow Trap
#define TRAP_DECOVF   6 // Decimal Overflow Trap
#define TRAP_SUBRNG   7 // Subscript Range Trap

#define FAULT_FLTOVF  8 // Floating Overflow Fault
#define FAULT_FLTDIV  9 // Floating Divide-By-Zero Fault
#define FAULT_FLTUND 10 // Floating Underflow Fault

#define STOP_HALT     1 // HALT Instruction
#define STOP_BRKPT    2 // Breakpoint
#define STOP_ILLVEC   3 // Illegal Vector in SCB
#define STOP_INIE     4 // In Exception/Interrupt Routine
#define STOP_PPTE     5 // Process PTE In P0/P1 Region
#define STOP_CHMIS    6 // Change Mode From IS
#define STOP_UIPL     7 // Undefined IPL
#define STOP_UNKNOWN  8 // Unknown Reason
#define STOP_NOCTY    9 // No console TTY device

#define HALT_BUTTON  0x02 // Halt button depressed
#define HALT_PWRUP   0x03 // Initial power on
#define HALT_INIE    0x04 // Interrupt stack not valid during exception
#define HALT_MCHK    0x05 // Machine check during machine check
#define HALT_KSNV    0x05 // Kernel stack not valid during execption
#define HALT_INST    0x06 // HALT instruction while in kernel mode
#define HALT_SCB11   0x07 // SCB vector <1:0> = 11
#define HALT_SCB10   0x08 // SCB vector <1:0> = 10
#define HALT_CHMIS   0x0A // CHMx instruction while on interrupt stack
#define HALT_PFMCHK  0x10 // ACV/TNV during machine check
#define HALT_PFKSNV  0x11 // ACV/TNV during kernel stack not valid

// Interrupt Priority Level
// Interrupt Assignments, Priority is right to left
//
// Field      Interrupt
// -----      ---------
// <0>        Clock (IPL 22 or 24)
// <3:1>      BR7   (IPL 17)
// <7:4>      BR6   (IPL 16)
// <19:8>     BR5   (IPL 15)
// <30:20>    BR4   (IPL 14)

#define INT_CLK 0x00000001 // Clock Interrupt
#define INT_BR7 0x0000000E // BR7 Interrupt
#define INT_BR6 0x000000F0 // BR6 Interrupt
#define INT_BR5 0x000FFF00 // BR5 Interrupt
#define INT_BR4 0x7FF00000 // BR4 Interrupt

#define IPL_MEMERR 0x1D // Memory Error Interrupt Level
#define IPL_CRDERR 0x1A // CRD Error Interrupt Level
#define IPL_HMAX   0x17 // Highest Hardware Interrupt Level
#define IPL_HMIN   0x14 // Lowest Hardware Interrupt Level
#define IPL_SMAX   0x0F // Highest Software Interrupt Level

#define IN_IE     vax->intFlag      // During execption routine
#define TIR       vax->intRequest
#define HIRQ      vax->intHardware
#define INTVEC    vax->intVector
#ifdef DEBUG
#define DBGIPL    vax->intModes
#endif /* DEBUG */

// Trap and Interrupt Requests

#define TIR_TRAP   0x00E0
#define TIR_IPL    0x001F
#define TIR_M_IPL  0x1F
#define TIR_M_TRAP 0x7
#define TIR_P_IPL  0
#define TIR_P_TRAP 5

// SISR - Software Interrupt Summary Register
#define SISR_MASK  0xFFFE
#define SISR_2     (1 << 2)

// General Registers Definition for VAX Processor

#define RN(n)  vax->gRegs[n]

#define RN0(n) RN(n)
#define RN1(n) RN((n+1) & REGMASK)

#define R0  RN(0)  // General Register #0
#define R1  RN(1)  // General Register #1
#define R2  RN(2)  // General Register #2
#define R3  RN(3)  // General Register #3
#define R4  RN(4)  // General Register #4
#define R5  RN(5)  // General Register #5
#define R6  RN(6)  // General Register #6
#define R7  RN(7)  // General Register #7
#define R8  RN(8)  // General Register #8
#define R9  RN(9)  // General Register #9
#define R10 RN(10) // General Register #10
#define R11 RN(11) // General Register #11
#define AP  RN(12) // Argument Pointer Register (R12)
#define FP  RN(13) // Frame Pointer Register    (R13)
#define SP  RN(14) // Stack Pointer Register    (R14)
#define PC  RN(15) // Program Counter Register  (R15)

#define nR0  0  // Index of General Register #0
#define nR11 11 // Index of General Register #11
#define nSP  14 // Index of Stack Pointer
#define nPC  15 // Index of Program Counter Register

#define faultPC vax->fault_PC

// Operand Registers for up to 8 operands

#define OPN(n) vax->opRegs[n]

#define OP0 OPN(0) // Operand Register #0
#define OP1 OPN(1) // Operand Register #1
#define OP2 OPN(2) // Operand Register #2
#define OP3 OPN(3) // Operand Register #3
#define OP4 OPN(4) // Operand Register #4
#define OP5 OPN(5) // Operand Register #5
#define OP6 OPN(6) // Operand Register #6
#define OP7 OPN(7) // Operand Register #7
#define OP8 OPN(8) // Operand Register #8

// Processor (Privileged) Registers Definition.
// Standard implementation for VAX architecture system.

#define PRN(n) vax->pRegs[n]   // Processor Register #n

#define KSP    PRN(0)   // (R/W) Kernel Stack Pointer
#define ESP    PRN(1)   // (R/W) Executive Stack Pointer
#define SSP    PRN(2)   // (R/W) Supervisor Stack Pointer
#define USP    PRN(3)   // (R/W) User Stack Pointer
#define ISP    PRN(4)   // (R/W) Interrupt Stack Pointer

#define P0BR   PRN(8)   // (R/W) P0 Base Register
#define P0LR   PRN(9)   // (R/W) P0 Length Register
#define P1BR   PRN(10)  // (R/W) P1 Base Register
#define P1LR   PRN(11)  // (R/W) P1 Length Register
#define SBR    PRN(12)  // (R/W) System Base Register
#define SLR    PRN(13)  // (R/W) System Limit Register

#define PCBB   PRN(16)  // (R/W) Process Control Block Base
#define SCBB   PRN(17)  // (R/W) System Control Block Base
#define IPL    PRN(18)  // (R/W) Interrupt Priority Level
#define ASTLVL PRN(19)  // (R/W) AST Level
#define SIRR   PRN(20)  // (W)   Software Interrupt Request Register
#define SISR   PRN(21)  // (R/W) Software Interrupt Summary Register

#define ICCS   PRN(24)  // (R/W) Internal Clock Control Status
#define NICR   PRN(25)  // (W)   Next Interval Count Register
#define ICR    PRN(26)  // (R)   Interval Count Register
#define TODR   PRN(27)  // (R/W) Time of Year Register

#define RXCS   PRN(32)  // (R/W) Console Receiver Status
#define RXDB   PRN(33)  // (R)   Console Receiver Data Buffer
#define TXCS   PRN(34)  // (R/W) Console Transmit Status
#define TXDB   PRN(35)  // (W)   Console Transmit Data Buffer

#define MAPEN  PRN(56)  // (R/W) Map Enable
#define TBIA   PRN(57)  // (W)   Translation Buffer Invalidate All
#define TBIS   PRN(58)  // (W)   Trabslation Buffer Invalidate Single

#define PME    PRN(61)  // (R/W) Performance Monitor Enable
#define SID    PRN(62)  // (R)   System Identification
#define TBCHK  PRN(63)  // (W)   Translation Buffer Check

// Processor Status Register Definitions
//
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// |CM |TP | 0 | 0 |FPD|IS |CUR_MOD|PRV_MOD| 0 |        IPL        |
// +---+---+---+---^---+---+---+---^---+---+---+---^---+---+---+---+
//   3   3   2   2   2   2   2   2   2   2   2   2   1   1   1   1
//   1   0   9   8   7   6   5   4   3   2   1   0   9   8   7   6
//
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |DV |FU |IV | T | N | Z | V | C |
// +---+---+---+---^---+---+---+---^---+---+---+---^---+---+---+---+
//   1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   0
//   5   4   3   2   1   0   9   8   7   6   5   4   3   2   1   0

// Processor Status macro definitions
#define PSL vax->stReg
#define PSW vax->stReg
#define CC  vax->ccReg

// Processor Status Long Register (32-bit word)
#define PSL_MBZ  0x3020FF00 // Must Be Zeros
#define PSL_CM   0x80000000 // Compatibility Mode (PDP-11 Mode)
#define PSL_TP   0x40000000 // Trace Pending
#define PSL_FPD  0x08000000 // First Part Done
#define PSL_IS   0x04000000 // Interrupt Stack
#define PSL_CUR  0x03000000 // Current Mode
#define PSL_PRV  0x00C00000 // Previous Mode
#define PSL_IPL  0x001F0000 // Interrupt Priority Level

// Processor Status Word Register (16-bit word)
#define PSW_MASK 0xFFFF // PSW Mask
#define PSW_MBZ  0xFF00 // Must Be Zeros
#define PSW_DV   0x0080 // Decimal Overflow Enable
#define PSW_FU   0x0040 // Floating Underflow Enable
#define PSW_IV   0x0020 // Integer Overflow Enable
#define PSW_T    0x0010 // Trace Enable
#define PSW_CC   0x000F // Condition Codes
#define PSW_N    0x0008 // Negative Result
#define PSW_Z    0x0004 // Zero Result
#define PSW_V    0x0002 // Overflow Result
#define PSW_C    0x0001 // Carry Bit Result

// Condition Bits
#define CC_N    PSW_N // Nagative Result
#define CC_Z    PSW_Z // Zero Result
#define CC_V    PSW_V // Overflow Result
#define CC_C    PSW_C // Carry Bit Result
#define CC_MASK (CC_N|CC_Z|CC_V|CC_C)

#define PSL_M_MODE  0x3
#define PSL_M_ISCUR 0x7
#define PSL_M_IPL   0x1F        
#define PSL_P_CUR   24
#define PSL_P_PRV   22
#define PSL_P_IPL   16
#define PSL_IPL01   0x00010000
#define PSL_IPL1F   0x001F0000

#define PSL_GETCUR(psl)   (((psl) >> PSL_P_CUR) & PSL_M_MODE)
#define PSL_GETISCUR(psl) (((psl) >> PSL_P_CUR) & PSL_M_ISCUR)
#define PSL_PUTCUR(cur)   ((cur) << PSL_P_CUR)
#define PSL_GETPRV(psl)   (((psl) >> PSL_P_PRV) & PSL_M_MODE)
#define PSL_PUTPRV(prv)   ((prv) << PSL_P_PRV)
#define PSL_PUTIPL(ipl)   ((ipl) << PSL_P_IPL)
#define PSL_GETIPL(psl)   (((psl) >> PSL_P_IPL) & PSL_M_IPL)

// Instruction Data Structure Definitions
// Instruction Table for Assembler, Disassembler, and Execution

typedef struct {
	char   *Name;            // Name of the Instruction
	char   *Desc;            // Description of the Instruction
	uint32 Flags;            // Instruction Flags
	uint8  Extended;         // MSB of Instruction (Normally Zero)
	uint8  Opcode;           // Opcode Value
	uint8  nOperands;        // Number of Operands
	uint32 opMode[6];        // Attributes/Scales for Each Operand
	int    useCount;         // Instruction Profile Data
	void   (*Execute)(); // Execute Routine
} INSTRUCTION;

// I/O Memory Map
typedef struct {
	uint32 loAddr;      // Address Low
	uint32 hiAddr;      // Address High
	uint32 Type;        // Type (Local or I/O)
	uint32 (*Read)();   // Read Access
	void   (*Write)();  // Write Access
} IOMAP;

typedef struct vax_System    VAX_SYSTEM;
typedef struct vax_Processor VAX_CPU;
typedef struct vax_Console   VAX_CONSOLE;

struct vax_System {
	// System Identification
	char *devName;    // System Device
	char *keyName;    // Key (Device Type) Name
	char *emuName;    // Emulator Name
	char *emuVersion; // Emulator Version

	// Symmetry Multi-Processors
	int     mProcessor; // Maximum Number of Processors
	int     nProcessor; // Current Number of Processors
	VAX_CPU *Processor; // Listing of Processors
};

struct vax_Processor {
	// CPU Processor Identification
	char  *devName;       // Device Name
	char  *keyName;       // Key (Device Type) Name
	char  *emuName;       // Emulator Name
	char  *emuVersion;    // Emulator Version

	VAX_SYSTEM  *System;    // Parent System
	VAX_CONSOLE *Console;   // Console TTY (OPA0:)
	void        *Callback;  // Unibus/QBus Callback
	void        *uqba;      // Unibus/QBus Device
	jmp_buf     SetJump;    // For ABORT function.

	// Instruction Table for this processor
	void    (*tblOpcode[NUM_INST])();
	uint32  tblOperand[NUM_INST][MAX_SPEC+1];
	int     ips; // Instructions Per Second Meter

	// Internal Processor Register Table
	uint32  (*ReadIPR[MAX_PREGS])(uint8, uint32 *);
	uint32  (*WriteIPR[MAX_PREGS])(uint8, uint32);

	// Prefetch Instruction Buffer (Look-Ahead Buffer)
	uint8   ibBuffer[OP_LONG * 2]; // Instruction Buffer (Two longwords)
	int32   ibCount;  // Current Buffer Size
	int32   ibIndex;  // Current Index of Instruction Buffer
	int32   ibPPC;    // Current Physical Program Counter

	// Current Instruction Execution
	int32   State;              // Current State
	int32   prvAccessMode;      // Previous Access Mode
	int32   curAccessMode;      // Current Access Mode
	uint32  fault_PC;           // Address of Faulting Instruction
	uint16  opCode;             // Opcode
	int32   gRegs[MAX_GREGS];   // General Registers
	uint32  stReg;              // Status Register
	uint32  ccReg;              //   Condition Code
	int32   pRegs[MAX_PREGS];   // Processor (Privileged) Registers
	int32   opRegs[MAX_OPREGS]; // Operand Registers
	int32   brDisp;             // Branch Displacement Register
	int32   rqPtr;              // Recovery Queue Pointer
	int16   rqOpnd[MAX_OPREGS]; // Recovery Queue Operands

	uint32  mchkAddr;  // Machine Check - Address
	uint32  mchkRef;   // Machine Check - Reference
	uint32  memError;  // Memory Error Flag


	// Interrupt/Exception Services
	int     intFlag;            // During Interrupt/Exception Routine Flag
	int32   intRequest;         // Trap and Interrupt Requests
	int32   intHardware;        // Hardware Interrupts
	int32   intVector[32];      // Hardware Interrupt Vectors
#ifdef DEBUG
	uint32  intModes[32];       // Modes for Debugging
#endif /* DEBUG */

	// Memory Management
	uint32 p1, p2, p3;           // Page Fault Results
	TLBENT stlbTable[VA_TBSIZE]; // System TLB Table
	TLBENT ptlbTable[VA_TBSIZE]; // Process TLB Table

	// RAM (Physical Memory) for VAX series
	uint8 *RAM;       // Address of physical memory
	int32 baseRAM;    // Starting address of physical memory
	int32 endRAM;     // Ending address of physical memory
	int32 sizeRAM;    // Size of physical memory
	int32 ramMaxSize; // Maximum memory size.

	// ROM Image for MicroVAX series
	uint8 *ROM;
	int32 baseROM;      // Starting Address of ROM
	int32 endROM;       // Ending Address of ROM
	int32 sizeROM;      // Size of ROM Area
	int32 maskROM;      // ROM Address Mask

	// NVRAM Image for MicroVAX series
	uint8 *NVRAM;       // Address of physical memory
	int32 baseNVRAM;    // Beginning of NVRAM Area
	int32 endNVRAM;     // End of NVRAM Area
	int32 sizeNVRAM;    // Size of NVRAM Area

#ifdef DEBUG
	// Debug Facility Area
	DBG_BRKSYS Breaks;  // Breakpoint System
#endif /* DEBUG */

	// Console Instruction Read Access (Look-Ahead)
	uint32  cvAddr;    // Current Virtual Address
	uint8   cibBuffer[OP_LONG * 2]; // Instruction Buffer (Two longwords)
	int32   cibCount;  // Current Buffer Size
	int32   cibIndex;  // Current Index of Instruction Buffer
	int32   cibPPC;    // Current Physical Program Counter

	int32  (*ReadRegister)(register VAX_CPU *, int32);
	void   (*WriteRegister)(register VAX_CPU *, int32, int32);
	uint32 (*ReadAligned)(VAX_CPU *, int32, int32);
	void   (*WriteAligned)(VAX_CPU *, int32, uint32, int32);
	uint32 (*ReadUnaligned)(VAX_CPU *, int32, int32);
	void   (*WriteUnaligned)(VAX_CPU *, int32, uint32, int32);
	int    (*ReadCA)(VAX_CPU *, int32, uint32 *, int32);
	int    (*WriteCA)(VAX_CPU *, int32, uint32, int32);
	int    (*ReadCU)(VAX_CPU *, int32, uint32 *, int32);
	int    (*WriteCU)(VAX_CPU *, int32, uint32, int32);
	void   (*TestParity)(VAX_CPU *, uint32);
	void   (*WriteParity)(VAX_CPU *, uint32);
	void   (*MachineCheck)(register VAX_CPU *);
	void   (*HaltAction)(register VAX_CPU *, uint32);
	int32  (*CheckTimer)(uint32);
	void   (*ResetClock)(VAX_CPU *);
	void   (*StartTimer)(void *);
	void   (*StopTimer)(void *);

	// Unibus/Qbus Function Calls
	void   (*InitIO)(VAX_CPU *);
	uint32 (*CheckIRQ)(VAX_CPU *, uint32);
	uint16 (*GetVector)(VAX_CPU *, uint32);
};

#define NOPRIV    1 // No privilege flag
#define ABORT(x)  longjmp(vax->SetJump, (x))
#define CONTINUE  ABORT(0)

#define RSVD_INST_FAULT ABORT(-SCB_RESIN) // Reserved Instruction
#define RSVD_ADDR_FAULT ABORT(-SCB_RESAD) // Reserved Address Mode
#define RSVD_OPND_FAULT ABORT(-SCB_RESOP) // Reserved Operand
#define PRIV_INST_FAULT ABORT(-(SCB_RESIN|NOPRIV)) // Privileged Instruction
#define FLT_OVFL_FAULT  P1=FAULT_FLTOVF, ABORT(-SCB_ARITH)
#define FLT_DZRO_FAULT  P1=FAULT_FLTDIV, ABORT(-SCB_ARITH)
#define FLT_UNFL_FAULT  P1=FAULT_FLTUND, ABORT(-SCB_ARITH)
#define NEXT -1

#define SET_TRAP(trap)  TIR = (TIR & TIR_IPL) | ((trap) << TIR_P_TRAP)
#define CLR_TRAPS       TIR &= ~TIR_TRAP
#define SET_IRQ         TIR = (TIR & TIR_TRAP) | vax_EvaluateIRQ(vax)
#define GET_TRAP(x)     (((x) >> TIR_P_TRAP) & TIR_M_TRAP)
#define GET_IRQ(x)      (((x) >> TIR_P_IPL) & TIR_M_IPL)

// Register Recovery Queue
#define RQPTR           vax->rqPtr
#define RQOP(n)         vax->rqOpnd[n]

#define AST_MASK 7
#define AST_MAX  4

#define BR_MASK 0xFFFFFFFC
#define LR_MASK 0x003FFFFF

#define FAULT   ((void *)-2) // Reserved Fault Flag
#define EMULATE ((void *)-1) // Emulation Flag

// Prefetch Instruction Buffer
#define IBUF    vax->ibBuffer
#define IBCNT   vax->ibCount
#define IBIDX   vax->ibIndex
#define IBPPC   vax->ibPPC

// Console Instruction Buffer
#define CVADDR  vax->cvAddr
#define CIBUF   vax->cibBuffer
#define CIBCNT  vax->cibCount
#define CIBIDX  vax->cibIndex
#define CIBPPC  vax->cibPPC

#define OPC     vax->opCode

#define SET_PC(addr)  PC = (addr), FLUSH_ISTR
#define FLUSH_ISTR    IBCNT = 0, IBPPC = -1
#define MAX(val,max)  ((val) > (max) ? (max) : (val))

// Memory Access - Write and Read
#define PACC        vax->prvAccessMode  // Previous Access Mode
#define ACC         vax->curAccessMode  // Current Access Mode
#define CA          0x80000000          // Console Access
#define RA          ACC                 // Read Access
#define WA          (ACC << 4)          // Write Access
#define CRA         (RA|CA)             // Console Read Access
#define CWA         (WA|CA)             // Console Write Access
#define ACC_MASK(m) (1 << (m))          // Access Mask
#define REF_V       0                   // Virtual Address Reference
#define REF_P       1                   // Physical Address Reference

// Machine Check Trap
#define MCHK_ADDR vax->mchkAddr  // Machine Check - Address
#define MCHK_REF  vax->mchkRef   // Machine Check - Reference
#define MACH_CHECK(code) P1=code, ABORT(-SCB_MCHK)
#define MEMERR    vax->memError  // Memory Error Flag (Trap)

#define LN_BYTE     OP_BYTE // Byte Access
#define LN_WORD     OP_WORD // Word Access
#define LN_LONG     OP_LONG // Longword Access

#define P1   vax->p1
#define P2   vax->p2
#define P3   vax->p3
#define STLB vax->stlbTable
#define PTLB vax->ptlbTable

// Update Access Mode Routine
#define SET_ACCESS ACC = ACC_MASK(PSL_GETCUR(PSL))

#if 0
// That is obsolete routine
#ifdef DEBUG
#define SET_ACCESS \
	{ \
		ACC = ACC_MASK(PSL_GETCUR(PSL)); \
		if (dbg_Check(DBG_TRACE|DBG_DATA)) \
			dbg_Printf("VAX: Access Mode: %s (was: %s)\n", \
				vax_accNames[PSL_GETCUR(PSL)], vax_accNames[PACC]); \
		PACC = PSL_GETCUR(PSL); \
	}
#else /* DEBUG */
#define SET_ACCESS ACC = ACC_MASK(PSL_GETCUR(PSL))
#endif /* DEBUG */
#endif /* 0 */

// Instruction Opcode Definition with optional register-passing attribute
#define DEF_NAME(cpu, name) cpu##_Opcode_##name
#define DEF_INST(cpu, name) \
ATTR_REGPARM(1) void DEF_NAME(cpu, name)(VAX_CPU *vax)

// Memory Access Definitions
#define ReadP(pa, len)             vax_ReadAligned(vax, pa, len)
#define ReadV(va, len, acc)        vax_Read(vax, va, len, acc)
#define ReadI(len)                 vax_ReadInst(vax, len)
#define WriteP(pa, data, len)      vax_WriteAligned(vax, pa, data, len)
#define WriteV(va, data, len, acc) vax_Write(vax, va, data, len, acc)
#define TestV(va, acc, st)         vax_Test(vax, va, acc, st)

// Store/Load Register/Memory
// Note: This macro only works for little endian at this time.

#define BSTORE(op0, op1, wd) \
	if (op0 >= 0) *((uint8 *)&RN(op0)) = wd; \
	else          WriteV(op1, (uint8)wd, OP_BYTE, WA);

#define WSTORE(op0, op1, wd) \
	if (op0 >= 0) *((uint16 *)&RN(op0)) = wd; \
	else          WriteV(op1, (uint16)wd, OP_WORD, WA);

#define LSTORE(op0, op1, wd) \
	if (op0 >= 0) RN(op0) = wd; \
	else          WriteV(op1, wd, OP_LONG, WA);

#define QSTORE(op0, op1, wl, wh) \
	if (op0 >= 0) { \
		RN0(op0) = wl; \
		RN1(op0) = wh; \
	} else { \
		WriteV(op1+OP_LONG, wh, OP_LONG, WA); \
		WriteV(op1, wl, OP_LONG, WA); \
	}

// Condition Code Macros

#define CC_DSPL(cc) vax_DisplayConditions(cc)

#define CC_Z1ZP \
	CC = CC_Z | (CC & CC_C);

#define CC_IIZZ_I(r) \
	if ((r) < 0)       CC = CC_N; \
	else if ((r) == 0) CC = CC_Z; \
	else               CC = 0;

#define CC_IIZZ_B(r) CC_IIZZ_I(r)
#define CC_IIZZ_W(r) CC_IIZZ_I(r)
#define CC_IIZZ_L(r) CC_IIZZ_I(r)

#define CC_IIZZ_Q(rl,rh) \
	if ((rh) < 0)              CC = CC_N; \
	else if (((rl)|(rh)) == 0) CC = CC_Z; \
	else                       CC = 0;

#define CC_IIZZ_FP(r) \
	if ((r) & FPSIGN)  CC = CC_N; \
	else if ((r) == 0) CC = CC_Z; \
	else               CC = 0;


#define CC_IIZP_I(r) \
	if ((r) < 0)       CC = CC_N | (CC & CC_C); \
	else if ((r) == 0) CC = CC_Z | (CC & CC_C); \
	else               CC = CC & CC_C;

#define CC_IIZP_B(r) CC_IIZP_I(r)
#define CC_IIZP_W(r) CC_IIZP_I(r)
#define CC_IIZP_L(r) CC_IIZP_I(r)

#define CC_IIZP_Q(rl,rh) \
	if ((rh) < 0)              CC = CC_N | (CC & CC_C); \
	else if (((rl)|(rh)) == 0) CC = CC_Z | (CC & CC_C); \
	else                       CC = CC & CC_C;

#define CC_IIZP_FP(r) \
	if ((r) & FPSIGN)  CC = CC_N | (CC & CC_C); \
	else if ((r) == 0) CC = CC_Z | (CC & CC_C); \
	else               CC = CC & CC_C;

#define INTOVF    if (PSW & PSW_IV) SET_TRAP(TRAP_INTOVF)
#define V_INTOVF  { CC |= CC_V; INTOVF; }

#define V_ADD_B(r, s1, s2) \
	if (((~(s1) ^ (s2)) & ((s1) ^ (r))) & BSIGN) V_INTOVF;
#define V_ADD_W(r, s1, s2) \
	if (((~(s1) ^ (s2)) & ((s1) ^ (r))) & WSIGN) V_INTOVF;
#define V_ADD_L(r, s1, s2) \
	if (((~(s1) ^ (s2)) & ((s1) ^ (r))) & LSIGN) V_INTOVF;
#define C_ADD(r, s1, s2) \
	if (((uint32)(r)) < ((uint32)(s2)))          CC |= CC_C;

#define CC_ADD_B(r, s1, s2) \
	CC_IIZZ_B(r); \
	V_ADD_B(r, s1, s2); \
	C_ADD(r, s1, s2);

#define CC_ADD_W(r, s1, s2) \
	CC_IIZZ_W(r); \
	V_ADD_W(r, s1, s2); \
	C_ADD(r, s1, s2);

#define CC_ADD_L(r, s1, s2) \
	CC_IIZZ_L(r); \
	V_ADD_L(r, s1, s2); \
	C_ADD(r, s1, s2);


#define V_SUB_B(r, s1, s2) \
	if ((((s1) ^ (s2)) & (~(s1) ^ (r))) & BSIGN) V_INTOVF;
#define V_SUB_W(r, s1, s2) \
	if ((((s1) ^ (s2)) & (~(s1) ^ (r))) & WSIGN) V_INTOVF;
#define V_SUB_L(r, s1, s2) \
	if ((((s1) ^ (s2)) & (~(s1) ^ (r))) & LSIGN) V_INTOVF;
#define C_SUB(r, s1, s2) \
	if (((uint32)(s2)) < ((uint32)(s1)))         CC |= CC_C;

#define CC_SUB_B(r, s1, s2) \
	CC_IIZZ_B(r); \
	V_SUB_B(r, s1, s2); \
	C_SUB(r, s1, s2);

#define CC_SUB_W(r, s1, s2) \
	CC_IIZZ_W(r); \
	V_SUB_W(r, s1, s2); \
	C_SUB(r, s1, s2);

#define CC_SUB_L(r, s1, s2) \
	CC_IIZZ_L(r); \
	V_SUB_L(r, s1, s2); \
	C_SUB(r, s1, s2);

#define CC_CMP_I(s1, s2) \
	if ((s1) < (s2))                 CC = CC_N; \
	else if ((s1) == (s2))           CC = CC_Z; \
	else                             CC = 0; \
	if ((uint32)(s1) < (uint32)(s2)) CC |= CC_C;

#define CC_CMP_B(s1, s2) CC_CMP_I(s1, s2)
#define CC_CMP_W(s1, s2) CC_CMP_I(s1, s2)
#define CC_CMP_L(s1, s2) CC_CMP_I(s1, s2)

#define TEST_PARITY

// Procedure/Function prototype definitions
#include "vax/proto.h"
