// defs.h - PDP-10 Series Definitions
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

// Memory size and limitation
#define VMA_SIZE  (1 << 18)      // Virtual address limit 
#define VMA_MASK  (VMA_SIZE - 1) // Virtual address mask
#define VMA(addr) ((addr) & VMA_MASK)

// Physcial/Extended memory size and limitation
#define PMA_SIZE  (1 << 30)      // Physcial address limit
#define PMA_MASK  (PMA_SIZE - 1) // Physcial address mask
#define PMA_LMASK (PMA_MASK & ~VMA_MASK) // Left halfword of address
#define PMA_RMASK VMA_MASK               // Right halfword of address
#define PMA(addr) ((addr) & PMA_MASK)

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
#define XLH(x)     ((x) & (WORD18L_ONES|WORD36_XSIGN))
#define LPC(x)     ((x) & PMA_LMASK)

#define LHSR(x)    (((x) >> 18) & WORD18R_ONES)
#define RHSL(x)    (((x) << 18) & WORD18L_ONES)
#define SR(x)      ((x) >> 18)
#define SL(x)      ((x) << 18)

#define LH18(x)    ((uint18)((x) >> 18) & WORD18_ONES)
#define RH18(x)    ((uint18)(x) & WORD18_ONES)
#define SWAP36(x)   (LHSR(x) | RHSL(x))

// Convert 18(32)-bit halfwords to 36(64)-bit word.
#define LH36(x)     (((uint36)(x) & WORD18_ONES) << 18)
#define RH36(x)     ((uint36)(x) & WORD18_ONES)
#define XWD36(x, y) (LH36(x) | RH36(y))

#define XWD(x, y)   (RHSL(x) | RH(y))

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
#endif /* PDP6 || KA10 */

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


#define INST_M_OP       0777        // Mask of Opcode
#define INST_M_DEV      0177        // Mask of Device Code
#define INST_M_FUNC     07          // Mask of Function Code
#define INST_M_AC       017         // Mask of Accumulator
#define INST_M_IND      01          // Mask of Indirect
#define INST_M_ADDR     0777777     // Mask of Address

#define INST_P_OP       27  // Position of Opcode
#define INST_P_DEV      26  // Position of Device Code
#define INST_P_FUNC     23  // Position of Function Code
#define INST_P_AC       23  // Position of Accumulator
#define INST_P_IND      22  // Position of Indirect
#define INST_P_XR       18  // Position of Index Register

#define INST_JFCL_AC    (INST_M_AC << INST_P_AC)
#define INST_XCT        0256

#define INST_GETOP(x)   ((uint32)((x) >> INST_P_OP) & INST_M_OP)
#define INST_GETDEV(x)  ((uint32)((x) >> INST_P_DEV) & INST_M_DEV)
#define INST_GETFUNC(x) (((uint32)(x) >> INST_P_FUNC) & INST_M_FUNC)
#define INST_GETAC(x)   (((uint32)(x) >> INST_P_AC) & INST_M_AC)
#define INST_GETI(x)    (((uint32)(x) >> INST_P_IND) & INST_M_IND)
#define INST_GETX(x)    (((uint32)(x) >> INST_P_XR) & INST_M_AC)
#define INST_GETY(x)    ((uint32)(x) & INST_M_ADDR)

// Local Indirect Word (IFIW)
#define LIW_P_IND      22          // Position of Indirect Bit
#define LIW_P_XR       18          // Position of Index Register
#define LIW_M_IND      01          // Mask of Indirect Bit
#define LIW_M_XR       017         // Mask of Index Register
#define LIW_M_ADDR     0777777     // Mask of Address

#define LIW_GETI(x)    (((x) >> LIW_P_IND) & LIW_M_IND)
#define LIW_GETX(x)    (((x) >> LIW_P_XR) & LIW_M_XR)
#define LIW_GETY(x)    ((x) & LIW_M_ADDR)

// Global Indirect Word (EFIW)
#define GIW_P_IND      34          // Position of Indirect Bit
#define GIW_P_XR       30          // Position of Index Register
#define GIW_M_IND      01          // Mask of Indirect Bit
#define GIW_M_XR       017         // Mask of Index Register
#define GIW_M_ADDR     07777777777 // Mask of Address

#define GIW_GETI(x)    (((x) >> GIW_P_IND) & GIW_M_IND)
#define GIW_GETX(x)    (((x) >> GIW_P_XR) & GIW_M_XR)
#define GIW_GETY(x)    ((x) & GIW_M_ADDR)

// Byte Pointer
//
// One-word local byte pointer, where P <= 36.
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    Pos    |   Size    |0|I|   X   |                 Y                 |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Two-word global byte pointer, where P <= 36 for non-zero section
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    Pos    |   Size    |1|Reserved |       Available to user           |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
// |                               IFIW/EFIW                               |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// One-word global byte pointer, where P > 36.
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | Pos,Size  |                30-bit Global Address                      | 
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Bits       Label       Definitions
// ----       -----       -----------
// <0:5>       Pos        Position
// <6:11>      Size       Size
// <12>         G         Global flag for non-zero section
// <13>         I         Indirect
// <14:17>      X         Index Register
// <18:35>      Y         Address

#define BP_POS      0770000000000 // Position field
#define BP_SIZE     0007700000000 // Size field
#define BP_GLOBAL   0000040000000 // Global bit
#define BP_GADDR    0007777777777 // 30-bit Global Address
#define BP_LADDR    INST_M_ADDR   // 18-bit Local Address

#define BP_M_POS      077 // Mask of Position
#define BP_M_SIZE     077 // Mask of Size
#define BP_P_POS      30  // Position of Position
#define BP_P_SIZE     24  // Position of Size

#define BP_GETPOS(x)   (((x) >> BP_P_POS) & BP_M_POS)
#define BP_PUTPOS(x)   ((int36)((x) & BP_M_POS) << BP_P_POS)
#define BP_GETSIZE(x)  (((x) >> BP_P_SIZE) & BP_M_SIZE)
#define BP_PUTSIZE(x)  ((int36)((x) & BP_M_SIZE) << BP_P_SIZE)
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
//   8        1PROC       1-Proceed (Only used in KS10/ITS Processor)
//   9        TRAP2       Stack Trap
//  10        TRAP1       Arithmetic Trap
//  11        FXU         Floating Underflow
//  12        DCX         No Divide (Divide check)
//
// <12:0>     FLAGS       Flags (Mask)
// <17:13>    SECT        Section (Used in KL10 Processor)
// <35:18>    ADDR        Program Counter (Virtual Address)

#define FLG_AROV   0400000000000LL // Arithmetic overflow
#define FLG_PCP    0400000000000LL // Previous Context Public
#define FLG_CRY0   0200000000000LL // Carry 0
#define FLG_CRY1   0100000000000LL // Carry 1
#define FLG_FOV    0040000000000LL // Floating Overflow
#define FLG_FPD    0020000000000LL // First Part Done
#define FLG_USER   0010000000000LL // User Mode
#define FLG_USERIO 0004000000000LL // User In-Out Mode
#define FLG_PCU    0004000000000LL // Previous Context User
#define FLG_PUBLIC 0002000000000LL // Public
#define FLG_AFI    0001000000000LL // Address Failure Inhibit
#define FLG_1PROC  0001000000000LL // 1-Proceed Flag
#define FLG_TRAP3  0000600000000LL // Trap 3 (Not used by hardware)
#define FLG_TRAP2  0000400000000LL // Stack Trap (Trap 2)
#define FLG_TRAP1  0000200000000LL // Arithmetic Trap (Trap 1)
#define FLG_FXU    0000100000000LL // Floating Underflow
#define FLG_DCX    0000040000000LL // No Divide (Divide Check)
#define FLG_TRAPS  (FLG_TRAP1|FLG_TRAP2)
#define FLG_MASK   0777740 // Flags Mask

// Left Halfword PC Flags (New)
#define PCF_ARO   0400000 // Arithmetic overflow
#define PCF_PCP   0400000 // Previous Context Public
#define PCF_CR0   0200000 // Carry 0
#define PCF_CR1   0100000 // Carry 1
#define PCF_FOV   0040000 // Floating Overflow
#define PCF_FPD   0020000 // First Part Done
#define PCF_USR   0010000 // User Mode
#define PCF_UIO   0004000 // User In-Out Mode
#define PCF_PCU   0004000 // Previous Context User
#define PCF_PUB   0002000 // Public
#define PCF_AFI   0001000 // Address Failure Inhibit
#define PCF_1PR   0001000 // 1-Proceed Flag
#define PCF_TR3   0000600 // Trap 3 (Not used by hardware)
#define PCF_TR2   0000400 // Stack Trap (Trap 2)
#define PCF_TR1   0000200 // Arithmetic Trap (Trap 1)
#define PCF_FXU   0000100 // Floating Underflow
#define PCF_DIV   0000040 // No Divide (Divide Check)
#define PCF_TRAPS (PCF_TRAP1|PCF_TRAP2)
#define PCF_MASK  0777740 // Flags Mask

#define PC_FLAGS  0777740000000LL // Flags Mask
#define PC_SECT   0000037000000LL // Section - Extended Address
#define PC_ADDR   0000000777777LL // Virtual Address Mask

#define VA_GETSECT(va)  (((va) >> 18) & 037)
#define VA_GETSECTF(va) ((va) & 037000000)
#define VA_ISLOCAL(va)  (!KX10_IsGlobal)
#define VA_ISGLOBAL(va) (KX10_IsGlobal)
#define PCS             p10->prvSection

#define CPU_CYCLE_PI    0200 // PI cycle flag
#define CPU_CYCLE_TRAP  0100 // TRAP cycle flag
#define CPU_CYCLE_UUO   0040 // UUO cycle flag
#define CPU_CYCLE_XCT   0020 // XCT cycle flag
#define CPU_FLAGS_PXCT  0017 // PXCT switch flags

// System Configuration Settings
#define CNF_ITS      0x200   // ITS Mode (0 = DEC, 1 = MIT)
#define CNF_XADR     0x100   // Extended Addressing Mode
#define CNF_CPUID    0x0FF   // CPU Identification Code

// CPU Identification Code - Exclusive Option
#define CNF_KS10     0x010   // KS10 Processor
#define CNF_KL10     0x008   // KL10 Processor
#define CNF_KI10     0x004   // KI10 Processor
#define CNF_KA10     0x002   // KA10 Processor
#define CNF_PDP6     0x001   // PDP6 Processor

#define CNF_KL10A (CNF_KL10)          // Model KL10-A
#define CNF_KL10B (CNF_KL10|CNF_XADR) // Model KL10-B

#define ISCPU(cpuid) (p10->cnfFlags & (cpuid))

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

// Skip (IntraSection)
#define DO_SKIP \
	PC = LPC(PC) | VMA(PC + 1)

// Jump (IntraSection)
#define DO_JUMP(newAddr) \
	PC = p10_Section | VMA(newAddr)

// Jump (InterSection)
#define DO_XJUMP(newAddr) \
	PC = PMA(newAddr); p10_Section = LPC(PC)

// Effective Address Math with Global/Local Flag
#define ADDEA(Addr, Add) \
	(KX10_IsGlobal ? PMA(Addr + Add) : LPC(Addr) | VMA(Addr + Add))
#define SUBEA(Addr, Sub) \
	(KX10_IsGlobal ? PMA(Addr - Sub) : LPC(Addr) | VMA(Addr - Sub))
#define INCEA(Addr) \
	Addr = (KX10_IsGlobal ? PMA(Addr + 1) : LPC(Addr) | VMA(Addr + 1))
#define DECEA(Addr) \
	Addr = (KX10_IsGlobal ? PMA(Addr - 1) : LPC(Addr) | VMA(Addr - 1))

extern int (*extOpcode[01000])();

extern jmp_buf p10_SetJump;

// APR - Arithmetic Processor System
extern int apr_Enables; // Interrupt Enables
extern int apr_Flags;   // System Flags
extern int apr_Level;   // PI Channel Level

// PI - Priority Interrupt System

// PAG - Pager System
extern int   KX10_Pager_On;
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
extern int p10_Serial;

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

extern int30 p10_Section;

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

//extern int36 p10_ACB[NACBLOCKS][020]; // AC blocks
extern int36 p10_ACB[8][020]; // AC blocks
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

extern int  (*KX10_Trap_NoMemory)(int30, int);
extern int  (*KX10_PageTrap1)(int);
extern void (*KX10_PageTrap2)(void);

// Interrupt Services
#define PAGE_FAIL     -1
#define P10_INTERRUPT -2
#define P10_ABORT     -3

#define PAGE_FAIL_TRAP(mode) \
	return KX10_PageTrap1(mode)

// Memory (Page Table Fill) definitions
#define PTF_BLT     04000 // BLT Instruction
#define PTF_CONSOLE 02000 // Console - No PFT/NXM traps
#define PTF_IOWORD  00000 // I/O Word Access
#define PTF_IOBYTE  01000 // I/O Byte Access
#define PTF_INHIBIT 00400 // PC Inhibit
#define PTF_PAGING  00200 // Paging In Progress
#define PTF_NOTRAP  00100 // No Page Fault Trap
#define PTF_NONXM   00040 // No Non-existent Memory Trap
#define PTF_CUR     00000 // Current AC Block Memory
#define PTF_PREV    00020 // Previous AC Block Memory
#define PTF_USER    00010 // User Virtual Address Memory
#define PTF_EXEC    00004 // Exec Virtual Address Memory
#define PTF_MAP     00002 // MAP Instruction
#define PTF_WRITE   00001 // Write Access \ Note: Access Bit
#define PTF_READ    00000 // Read Access  / Write = 1, Read = 0

// Cache Page Defintions
#define PTE_M    0x80000000 // Page is modified (must be a sign bit)
#define PTE_U    0x40000000 // Page is user  (1=User, 0=Executive)
#define PTE_V    0x20000000 // Page is valid
#define PTE_ADDR 0x001FFFFF // 21-bit Page address

// *****************************************

typedef struct p10_Console   P10_CONSOLE;
typedef struct p10_Processor P10_CPU;
typedef struct p10_System    P10_SYSTEM;

struct p10_Processor {
	UNIT        Unit;          // Unit Header Information
	P10_CPU     *Next;         // Next Processor
	P10_SYSTEM  *System;       // System (Parnet)
	P10_CONSOLE *Console;      // Console TTY line

	char        *devName;      // Device Name
	char        *keyName;      // Key (Device Type) Name
	char        *emuName;      // Emulator Name
	char        *emuVersion;   // Emulator Version

	uint32 cnfFlags;    // Configuration Flags
	void   *Processor;  // Model-specific device (Obsolete)

	// Program Counter - Flags, Section, and Address
	uint32 prvSection; // Previous Context Section (PCS)
	uint32 pcFlags;    // PC Flags
	uint32 pcSection;  // PC Section
	uint32 pcAddr;     // PC Address

	// Accumulators (Registers)
	// Each block contains 16 accumulators
	int32 szBlock;        // Size of AC Block
	int32 nBlocks;        // Number of AC Blocks
	int32 nAccumlators;   // Number of Accumlators
	int36 *acBlocks;      // AC Blocks
	int36 *prvAC, *curAC; // Current AC blocks

};
extern P10_CPU *p10;

struct p10_System {
	char        *devName;      // Device Name
	char        *keyName;      // Key (Device Type) Name
	char        *emuName;      // Emulator Name
	char        *emuVersion;   // Emulator Version

	// PDP-10 Processor
	P10_CPU *Processor;

	// Main Memory System for single or multiple processing system
	void  *RAM;     // Main Memory
	int32 baseRAM;  // Starting Address of Main Memory
	int32 endRAM;   // Ending Address of Main Memory
	int32 sizeRAM;  // Size of Memory in Bytes
};

#include "pdp10/proto.h"
