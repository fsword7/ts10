// defs.h - PDP-10 Series Definitions
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
// See 'ReadMe' for copyright notice.
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

#include "emu/defs.h"

// Memory size and limitation
#define VMA_SIZE  (1 << 18)      // Virtual address limit 
#define VMA_MASK  (VMA_SIZE - 1) // Virtual address mask
#define VMA(addr) ((addr) & VMA_MASK)

// 36-Bit Word Definitions (Word)
#define WORD36_ONES   0777777777777LL // Word - All ones
#define WORD36_XONES  -1LL            // Word - Signed all ones
#define WORD36_CARRY 01000000000000LL // Word - Carry bit
#define WORD36_SIGN   0400000000000LL // Word - Sign bit
#define WORD36_XSIGN  ~WORD36_MAXP    // Word - Extended sign bit
#define WORD36_MAXP   0377777777777LL // Word - Maximum postive number
#define WORD36_MAXN   ~WORD36_MAXP    // Word - Maximum negative number

// 18-Bit Word Definitions (Halfword)
#define WORD18_ONES   0777777         // Halfword - All ones
#define WORD18_XONES  -1LL            // Halfword - Signed all ones
#define WORD18_CARRY 01000000         // Halfword - Carry bit
#define WORD18_SIGN   0400000         // Halfword - Sign bit
#define WORD18_XSIGN  ~WORD18_MAXP    // Halfword - Extended sign bit
#define WORD18_MAXP   0377777         // Halfword - Maximum postive number
#define WORD18_MAXN   ~WORD18_MAXP    // Halfword - Maximum negative number

// Left Halfword Definitions
#define WORD18L_ONES  0777777000000LL // Left Halfword - All ones
#define WORD18L_SIGN  0400000000000LL // Left Halfword - Sign bit
#define WORD18L_XONES (WORD18L_ONES|WORD36_XSIGN) // With Extended Sign

// Right Halfword Definitions
#define WORD18R_ONES  0000000777777LL // Right Halfword - All ones
#define WORD18R_SIGN  0000000400000LL // Right Halfword - Sign bit
#define WORD18R_XONES (WORD18R_ONES|WORD36_XSIGN) // With Extended Sign

// 8-Bit Byte Definitions
#define BYTE8_ONES  0377 // 8-bit Byte - All ones
#define BYTE8_SIGN  0200 // 8-bit Byte - Sign bit
#define ONES8  0377
#define SIGN8  0200

// Macro definitions - Sign bit functions
#define S36(x) ((x) & WORD36_SIGN)
#define S18(x) ((x) & WORD18_SIGN)
#define S8(x)  ((x) & BYTE8_SIGN)

#define SXT36(x) (S36(x) ? ((x) | ~WORD36_ONES) : ((x) & WORD36_ONES))
#define SXT18(x) (S18(x) ? ((x) | ~WORD18_ONES) : ((x) & WORD18_ONES))
#define SXT8(x)  (S8(x) ? ((x) | ~BYTE8_ONES) : ((x) & BYTE8_ONES))
#define LIT8(x)  (S18(x) ? \
	(((x) & ONES8) ? ((x) | ~ONES8) : ~ONES8) : ((x) & ONES8))

#define LH(x)      ((x) & WORD18L_ONES)
#define RH(x)      ((x) & WORD18R_ONES)

#define LHSXT(x)   ((x) & (WORD18L_ONES|WORD36_XSIGN))

#define LHSR(x)    (((x) >> 18) & WORD18R_ONES)
#define RHSL(x)    (((x) << 18) & WORD18L_ONES)

#define SWAP36(x) (LHSR(x) | RHSL(x))

#define XWD(x, y) (RHSL(x) | RH(y))

#define NEG(x) -(x)
#define ABS(x) (((x) < 0) ? -(x) : (x))

#define AC(x)     ((x) & 017)
#define ACX(x, y) ((x + y) & 017)

// Add one to both halfwords
#if defined(PDP6) || defined(KA10)
#define AOB(x) ((x) + 01000001)
#define SOB(x) ((x) - 01000001)
#else
#define AOB(x) (LH((x) + 01000000) | RH((x) + 1))
#define SOB(x) (LH((x) - 01000000) | RH((x) - 1))
#endif

// INSTRUCTION FORMAT
//
// Basic Instruction Format for all Kx10 Processors.
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   Opcode/Mode   |   AC  |I|   X   |                 Y                 |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Bits      Label    Definitions
// ----      -----    -----------
// <0:8>     OP       Instruction Code (Opcode)
// <9:12>    AC       Accumulator
// <13>      I        Indirect
// <14:17>   X        Index Register
// <18:35>   Y        Address
//
// In-Out Instruction Format for KA10/KI10/KL10 Processors only
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |1 1 1|   Device    |Func |I|   X   |                 Y                 |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Bits      Label    Definitions
// ----      -----    -----------
// <0:2>     OP       Opcode - 700 Series
// <3:9>     Device   Device Code
// <10:12>   Func     Function Code
// <13>      I        Indirect
// <14:17>   X        Index Register
// <18:35>   Y        Address
//
// Local Indirect Word
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |1|       Reserved        |I|   X   |                 Y                 |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Global Indirect Word
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |0|I|   X   |                           Y                               |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5


#define INST_M_OP       0777      // Mask of Opcode
#define INST_M_DEV      0177      // Mask of Device Code
#define INST_M_FUNC     07        // Mask of Function Code
#define INST_M_AC       017       // Mask of Accumulator
#define INST_M_IND      01        // Mask of Indirect
#define INST_M_ADDR     0777777   // Mask of Address

#define INST_P_OP       27  // Position of Opcode
#define INST_P_DEV      26  // Position of Device Code
#define INST_P_FUNC     23  // Position of Function Code
#define INST_P_AC       23  // Position of Accumulator
#define INST_P_IND      22  // Position of Indirect
#define INST_P_XR       18  // Position of Index Register

#define INST_JFCL_AC    (INST_M_AC << INST_P_AC)

#define INST_GETOP(x)   ((uint32)((x) >> INST_P_OP) & INST_M_OP)
#define INST_GETDEV(x)  ((uint32)((x) >> INST_P_DEV) & INST_M_DEV)
#define INST_GETFUNC(x) (((uint32)(x) >> INST_P_FUNC) & INST_M_FUNC)
#define INST_GETAC(x)   (((uint32)(x) >> INST_P_AC) & INST_M_AC)
#define INST_GETI(x)    (((uint32)(x) >> INST_P_IND) & INST_M_IND)
#define INST_GETX(x)    (((uint32)(x) >> INST_P_XR) & INST_M_AC)
#define INST_GETY(x)    ((uint32)(x) & INST_M_ADDR)

// Byte Pointer
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    Pos    |   Size    |0|I|   X   |                 Y                 |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Bits       Label       Definitions
// ----       -----       -----------
// <0:5>       Pos        Position
// <6:11>      Size       Size
// <12>        N/A        Unused
// <13>         I         Indirect
// <14:17>      X         Index Register
// <18:35>      Y         Address

#define BP_M_POS      0770000000000LL // Mask of Position
#define BP_M_SIZE     0007700000000LL // Mask of Size
#define BP_M_ADDR     INST_M_ADDR

#define BP_P_POS      30  // Position of Position
#define BP_P_SIZE     24  // Position of Size

#define BP_GETPOS(x)   (((x) & BP_M_POS) >> BP_P_POS)
#define BP_GETSIZE(x)  (((x) & BP_M_SIZE) >> BP_P_SIZE)
#define BP_GETI(x)     INST_GETI(x)
#define BP_GETX(x)     INST_GETX(x)
#define BP_GETY(x)     INST_GETY(x)

// PC/Status Word
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          Flags          | Section |          Program Counter          |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Bits       Label       Definitions
// ----       -----       -----------
//   0        AROV        Arithmetic Overflow
//   1        CRY0        Carry 0
//   2        CRY1        Carry 1
//   3        FOV         Floating Overflow
//   4        FPD         First Part Done
//   5        USER        User Mode
//   6        USERIO      User In-Out Mode
//   7        PUBLIC      Public (Not used in KS10 Processor)
//   8        AFI         Address Failure Inhibit
//   9        TRAP2       Stack Trap
//  10        TRAP1       Arithmetic Trap
//  11        FXU         Floating Underflow
//  12        DCX         No Divide (Divide check)
//
// <12:0>     FLAGS       Flags
// <17:13>    SECT        Section (Used in KL10 Processor)
// <35:18>    ADDR        Program Counter (Virtual Address)

#define PC_AROV   0400000000000LL // Arithmetic overflow
#define PC_PCP    0400000000000LL // Previous Context Public
#define PC_CRY0   0200000000000LL // Carry 0
#define PC_CRY1   0100000000000LL // Carry 1
#define PC_FOV    0040000000000LL // Floating Overflow
#define PC_FPD    0020000000000LL // First Part Done
#define PC_USER   0010000000000LL // User Mode
#define PC_USERIO 0004000000000LL // User In-Out Mode
#define PC_PCU    0004000000000LL // Previous Context User
#define PC_PUBLIC 0002000000000LL // Public
#define PC_AFI    0001000000000LL // Address Failure Inhibit
#define PC_TRAP3  0000600000000LL // Trap 3 (Not used by hardware)
#define PC_TRAP2  0000400000000LL // Stack Trap (Trap 2)
#define PC_TRAP1  0000200000000LL // Arithmetic Trap (Trap 1)
#define PC_FXU    0000100000000LL // Floating Underflow
#define PC_DCX    0000040000000LL // No Divide (Divide Check)
#define PC_TRAPS  (PC_TRAP1|PC_TRAP2)
#define PC_FLAGS  0777740000000LL // Flags Mask
#define PC_SECT   0000037000000LL // Section - Extended Address
#define PC_ADDR   0000000777777LL // Virtual Address Mask

#define CPU_CYCLE_TRAP  0100 // TRAP cycle flag
#define CPU_CYCLE_UUO   0040 // UUO cycle flag
#define CPU_CYCLE_XCT   0020 // XCT cycle flag
#define CPU_FLAGS_PXCT  0017 // PXCT switch flags

// PDP-6/PDP-10 Instruction Table Definitions
typedef struct Instruction {
	char  *Name;            // Name of the Instruction
	char  *Desc;            // Description of the Instruction
	int   Flags;            // Flags
	int32 Extended;         // Extended Opcode (Normally Zero)
	int32 Opcode;           // Opcode Value
	int   useCount;         // Profile Data
	void  (*Execute[10])(); // Execute Routine
	struct Instruction *AC[020];
} INSTRUCTION;

// Instruction Table Definitions
#define OP_P6   040000000 // PDP-6 - Processor Type 166
#define OP_KA   020000000 // KA10 Processor
#define OP_KI   010000000 // KI10 Processor
#define OP_KL   004000000 // KL10 Processor
#define OP_KS   002000000 // KS10 Processor
#define OP_ALL  (OP_KA|OP_KI|OP_KL|OP_KS) // All Prcoessors

#define OP_ITS  000100000 // ITS Instructions
#define OP_T20  000040000 // TOPS-20 Instructions
#define OP_T10  000020000 // TOPS-10 Instructions
#define OP_IMM  000010000 // Immediate
#define OP_DEV  000004000 // I/O Instruction - Device Codes
#define OP_FUNC 000002000 // I/O Instruction - Function Codes
#define OP_IO   000001000 // I/O Instruction
#define OP_E271 000000400 // KL10 Microcode v271
#define OP_EXT  000000200 // Extended Instruction
#define OP_AC   000000100 // AC-based Instruction

// AC-Based Instruction Definitions
#define OP_Z    000000020 // AC# Zero (00) 
#define OP_NZ   000000040 // AC# Non-Zero (01-17)
#define OP_00   000000060 // AC# 00
#define OP_01   000000061 // AC# 01
#define OP_02   000000062 // AC# 02
#define OP_03   000000063 // AC# 03
#define OP_04   000000064 // AC# 04
#define OP_05   000000065 // AC# 05
#define OP_06   000000066 // AC# 06
#define OP_07   000000067 // AC# 07
#define OP_10   000000070 // AC# 10
#define OP_11   000000071 // AC# 11
#define OP_12   000000072 // AC# 12
#define OP_13   000000073 // AC# 13
#define OP_14   000000074 // AC# 14
#define OP_15   000000075 // AC# 15
#define OP_16   000000076 // AC# 16
#define OP_17   000000077 // AC# 17

#define DO_SKIP \
	PC = (PC + 1) & VMA_MASK

#define DO_JUMP(newAddr) \
	PC = newAddr

extern int (*extOpcode[01000])();

extern jmp_buf p10_SetJump;

// APR - Arithmetic Processor System
extern int apr_Enables; // Interrupt Enables
extern int apr_Flags;   // System Flags
extern int apr_Level;   // PI Channel Level

// PI - Priority Interrupt System

// PAG - Pager System
extern int   pager_On;
extern int30 pager_PC;
extern int36 pager_Flags;
extern void  (*pager_Cleanup)();

extern int30 eptAddr; // EPT base address
extern int30 uptAddr; // UPT base address
extern int36 PFW;

// PXCT switch variables
extern int srcMode;   // Source for BLT
extern int dstMode;   // Destination for BLT
extern int eaMode;    // Effective Address
extern int stackMode; // Stack Data
extern int dataMode;  // Memory Data
extern int byteMode;  // Byte Data

extern int savedMode;
extern int savedModePFT;
extern int cpu_pFlags;
extern int cpu_pInterrupt;

// KS10 Processor Registers
extern int36 HR;    // Instruction Register (36 bits)
extern int36 AR;    // Arithmetic Register (36 bits)
extern int36 ARX;   // Arithmetic Register (36 bits)
extern int36 BR;    // Buffer Register (36 bits)
extern int36 BRX;   // Buffer Register (36 bits)
extern int30 PC;    // Program Counter (18 bits) <18:35> of PC
extern int36 FLAGS; // Status Register (18 bits) <0:17> of PC
extern int36 EBR;   // Executive Base Register
extern int36 UBR;   // User Base Register
extern int36 T0;    // Temparatory #0
extern int36 T1;    // Temparatory #1

// Fields of Instruction Code
extern int18 opDevice;   // (I/O)   Device Code       (DEV)
extern int18 opFunction; // (I/O)   Function Code     (FUNC)
extern int18 opCode;     // (Basic) Opcode            (OP)
extern int18 opAC;       // (Basic) Accumulator       (AC)
extern int18 opIndirect; // (Both)  Indirect          (I)
extern int18 opIndex;    // (Both)  Index Register    (X)
extern int18 opAddr;     // (Both)  18-bit Address    (Y)

// Calculation for Effective Address from I, X, and Y fields
extern int36 eAddr;      // (Both)  Effective Address (E)
//extern int30 eAddr;      // (Both)  Effective Address (E)

extern int PACB;  // Previous Context AC Block
extern int CACB;  // Current Context AC Block

extern IOCOMMAND p10_Commands[];

// EXE Format Specifications

// EXE header blocks
#define SV_ID_ENTRY_VECTOR 01775
#define SV_ID_DIRECTORY    01776
#define SV_ID_END_BLOCK    01777

// Page flags 
#define SV_M_HIGH_SEG  0400000000000LL // Page is part of high segement
#define SV_M_SHARABLE  0200000000000LL // Page is sharable
#define SV_M_WRITABLE  0100000000000LL // Page is writable
#define SV_M_CONCEALED 0040000000000LL // Page is concealed
#define SV_M_SYM_TABLE 0020000000000LL // Page is part of symbol table
#define SV_M_ALLOCATED 0010000000000LL // Page is allocated but zero

#define SV_BLK_SIZE 512 // 512 words per block (777 words in octal)

#include "pdp10/ks10.h"

extern int36 p10_ACB[NACBLOCKS][020]; // AC blocks
extern int36 *curAC;    // Current AC block
extern int36 *prvAC;    // Previous AC block

extern int cips;
extern int jiffy;

extern int p10_ctyCountdown;
extern int p10_State;

// Cache entry table for KS10/KL10 Processor
extern int30 p10_eptCache[];
extern int30 p10_uptCache[];
extern int32 p10_CacheMisses;
extern int32 p10_CacheHits;
extern UNIT  *p10_ioUnits;
