// ks10.h - Definitions for the KS10 Processor emulation
//
// Copyright (c) 2001-2003, Timothy M. Stark
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
//
// -------------------------------------------------------------------------
//
// Modification History:
//
// XX/XX/XX  TMS  Comments Here
//
// -------------------------------------------------------------------------

#define KS10_KEY     "KS10"
#define KS10_NAME    "KS10 Emulator"
#define KS10_VERSION "v0.8.9 (Late Alpha)"

// KS10 Processor - definitions

// Memory Configuration for KS10 Processor

// Memory Size and Limitation
#define MAXMEMSIZE  (1 << 20) // KS10 - 20-bit Addressing - 1024K words

// Number of AC Blocks
#define NACBLOCKS 8   //  8 AC blocks
#define ACBLKSIZE 020 // 16 accumlators

// APR - KS10 Arithmetic Processor 
//
// APRID Instruction - Opcode 70000
// 
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
// | Microcode Opts  | Microcode Ver # |     |        Serial Number        |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// MSB                                                                   LSB
//
// Bit 0-8   - Microcode Options
// Bit 9-17  - Microcode Version Number
// Bit 18-20 - Hardware Options
// Bit 21-35 - Processor Serial Number

#define APRID_M_MC_OPTS  0777000000000LL
#define APRID_M_MC_VER   0000777000000LL
#define APRID_M_HW_OPTS  0000000700000LL
#define APRID_M_PROC_SN  0000000077777LL

#define APRID_V_MC_OPTS  (35 - 8)
#define APRID_V_MC_VER   (35 - 17)
#define APRID_V_HW_OPTS  (35 - 20)
#define APRID_V_PROC_SN  (35 - 35)

#define KS10_MC_NCU      0600000000000LL
#define KS10_MC_UBABLT   0040000000000LL
#define KS10_MC_KIP      0020000000000LL
#define KS10_MC_KLP      0010000000000LL

#define KS10_MC_OPTS (KS10_MC_KLP|KS10_MC_NCU)
#define KS10_MC_VER  0130
#define KS10_HW_OPTS 0
#define KS10_SN      4096 // Default S/N number.

// APR Status Register
//
// WRAPR Instruction - Opcode 70020
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
// |          Unused                       |E|D|C|S|  System Flags | | PIA |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// MSB                                                                   LSB
//
// Bit 20-23: Selected Flags
//   E = Enable
//   D = Disable
//   C = Clear
//   S = Set
//
// Bit 24-31: System Flags
//   Bit 24 - Flag 24
//   Bit 25 - Interrupt Console
//   Bit 26 - Power Failure
//   Bit 27 - No Memory (NXM)
//   Bit 28 - Bad Memory Data
//   Bit 29 - Corrected Memory Data
//   Bit 30 - Interval Done
//   Bit 31 - Console Interrupt
//
// Bit 33-35: Priority Interrupt Assignment

#define APR_M_SEL_FLAGS     0000000170000LL
#define APR_M_SYS_FLAGS     0000000007760LL 

#define APR_V_SEL_FLAGS     (35 - 23)
#define APR_V_SYS_FLAGS     (35 - 31)

#define APR_F_SEL_ENA       0000000100000LL // Select Enable Flag
#define APR_F_SEL_DIS       0000000040000LL // Select Disable Flag
#define APR_F_SEL_CLR       0000000020000LL // Select Clear Flag
#define APR_F_SEL_SET       0000000010000LL // Select Set Flag
#define APR_F_FLAG24        0000000004000LL // Flag 24
#define APR_F_INT_CON       0000000002000LL // Interrupt Console
#define APR_F_POWER_FAIL    0000000001000LL // Power Failure
#define APR_F_NO_MEM        0000000000400LL // No Memory
#define APR_F_BAD_MEM_DATA  0000000000200LL // Bad Memory Data
#define APR_F_COR_MEM_DATA  0000000000100LL // Corrected Memory Data
#define APR_F_INT_DONE      0000000000040LL // Interval Done
#define APR_F_CON_INT       0000000000020LL // Console Interrupt
#define APR_M_PI_CHAN       0000000000007LL // Priority Interrupt Assignments

#define APR_ENABLE    0000000100000LL // (W)   Enable Flags
#define APR_DISABLE   0000000040000LL // (W)   Disable Flags
#define APR_CLEAR     0000000020000LL // (W)   Clear Flags
#define APR_SET       0000000010000LL // (W)   Set Flags
#define APR_FLAGS     0000000007760LL // (R/W) System Flags Mask
#define APR_IRQ       0000000000010LL // (R)   Interrupt Request
#define APR_LEVEL     0000000000007LL // (R/W) PI Channel Level

#define APR_V_SEL_ENA       (35 - 20)
#define APR_V_SEL_DIS       (35 - 21)
#define APR_V_SEL_CLR       (35 - 22)
#define APR_V_SEL_SET       (35 - 23)
#define APR_V_FLAG24        (35 - 24)
#define APR_V_INT_CON       (35 - 25)
#define APR_V_POWER_FAIL    (35 - 26)
#define APR_V_NO_MEM        (35 - 27)
#define APR_V_BAD_MEM_DATA  (35 - 28)
#define APR_V_COR_MEM_DATA  (35 - 29)
#define APR_V_INT_DONE      (35 - 30)
#define APR_V_CON_INT       (35 - 31)
#define APR_V_PI_CHAN       (35 - 35)

// RDAPR Instruction - Opcode 70024
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
// |  Unused   | Flags Enabled |     Unused        |                 | PIA |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// MSB                                                                   LSB
//
// Bit 6-13: Flags Enabled to Interrupt
//   Bit 6 - Flag 24
//   Bit 7 - Interrupt Console
//   Bit 8 - Power Failure
//   Bit 9 - No Memory (NXM)
//   Bit 10 - Bad Memory Data
//   Bit 11 - Corrected Memory Data
//   Bit 12 - Interval Done
//   Bit 13 - Console Interrupt
//
// Bit 24-32: Interrupt Flags
//   Bit 24 - Flag 24
//   Bit 25 - Interrupt Console
//   Bit 26 - Power Failure
//   Bit 27 - No Memory (NXM)
//   Bit 28 - Bad Memory Data
//   Bit 29 - Corrected Memory Data
//   Bit 30 - Interval Done
//   Bit 31 - Console Interrupt
//
// Bit 32: Interrupt Requested
//
// Bit 33-35: Priority Interrupt Assignements

#define APRSR_F_ENA_FLAG24         0004000000000LL
#define APRSR_F_ENA_INT_CON        0002000000000LL
#define APRSR_F_ENA_POWER_FAIL     0001000000000LL
#define APRSR_F_ENA_NO_MEMORY      0000400000000LL
#define APRSR_F_ENA_BAD_MEM_DATA   0000200000000LL
#define APRSR_F_ENA_COR_MEM_DATA   0000100000000LL
#define APRSR_F_ENA_INT_DONE       0000040000000LL
#define APRSR_F_ENA_CON_INT        0000020000000LL

#define APRSR_V_ENA_FLAG24         (35 - 6)
#define APRSR_V_ENA_INT_CON        (35 - 7)
#define APRSR_V_ENA_POWER_FAIL     (35 - 8)
#define APRSR_V_ENA_NO_MEMORY      (35 - 9)
#define APRSR_V_ENA_BAD_MEM_DATA   (35 - 10)
#define APRSR_V_ENA_COR_MEM_DATA   (35 - 11)
#define APRSR_V_ENA_INT_DONE       (35 - 12)
#define APRSR_V_ENA_CON_INT        (35 - 13)

#define APRSR_F_FLAG24             0000000004000LL
#define APRSR_F_INT_CON            0000000002000LL
#define APRSR_F_POWER_FAIL         0000000001000LL
#define APRSR_F_NO_MEMORY          0000000000400LL
#define APRSR_F_BAD_MEM_DATA       0000000000200LL
#define APRSR_F_COR_MEM_DATA       0000000000100LL
#define APRSR_F_INT_DONE           0000000000040LL
#define APRSR_F_CON_INT            0000000000020LL
#define APRSR_F_INT_REQ            0000000000010LL
#define APRSR_M_PI_CHAN            0000000000007LL

#define APRSR_V_FLAG24             (35 - 24)
#define APRSR_V_INT_CON            (35 - 25)
#define APRSR_V_POWER_FAIL         (35 - 26)
#define APRSR_V_NO_MEMORY          (35 - 27)
#define APRSR_V_BAD_MEM_DATA       (35 - 28)
#define APRSR_V_COR_MEM_DATA       (35 - 29)
#define APRSR_V_INT_DONE           (35 - 30)
#define APRSR_V_CON_INT            (35 - 31)
#define APRSR_V_INT_REQ            (35 - 32)
#define APRSR_V_PI_CHAN            (35 - 35)

/*
 * WRPI Intruction - Opcode 70060
 *
 * |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 * |                                         | | | | | | | | |             | 
 * +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
 *  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 * MSB                                                                   LSB
 *
 * Bit 22: Drop Program Requests On Selected Levels
 * Bit 23: Clear PI System
 * Bit 24: Initiate Interrupts On
 * Bit 25: Turn On for Selected Levels
 * Bit 26: Turn Off for Selected Levels
 * Bit 27: Turn On PI System
 * Bit 28: Turn Off PI System
 * Bit 29-35: Select Interrupt Levels
 */

#define PI_M_DROP_PROG_REQ 0000000020000LL
#define PI_M_CLR_PI_SYS    0000000010000LL
#define PI_M_INI_INT_ON    0000000004000LL
#define PI_M_LEVELS_ON     0000000002000LL
#define PI_M_LEVELS_OFF    0000000001000LL
#define PI_M_PI_SYS_OFF    0000000000400LL
#define PI_M_PI_SYS_ON     0000000000200LL
#define PI_M_LEVELS        0000000000177LL

/*
 * RDPI Instruction - Opcode 70064
 *
 * |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 * |0 0 0 0 0 0 0 0 0 0 0| PI Request  |0 0 0|   PI Held   |A|    PI On    |
 * +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
 *  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 * MSB                                                                   LSB
 *
 * Bit 11-17: Program Requests On Levels
 * Bit 21-27: Interrupt In Progress On Levels
 * Bit 28:    PI System On
 * Bit 29-35: Interrupt Levels On
 */

#define PISR_M_PI_REQUEST   0000177000000LL
#define PISR_M_PI_HOLD      0000000077400LL
#define PISR_M_PI_SYS_ON    0000000000200LL
#define PISR_M_PI_ON        0000000000177LL

#define PISR_P_PI_REQUEST   (35 - 17)
#define PISR_P_PI_HOLD      (35 - 27)
#define PISR_P_PI_SYS_ON    (35 - 28)
#define PISR_P_PI_ON        (35 - 35)

/* PI Interrupt Mask */
#define PI_M_INT0 0000 /* Interrupt Level #0 - No Interrupts */
#define PI_M_INT1 0100 /* Interrupt Level #1 - Highest Priority */
#define PI_M_INT2 0040 /* Interrupt Level #2 */
#define PI_M_INT3 0020 /* Interrupt Level #3 */
#define PI_M_INT4 0010 /* Interrupt Level #4 */
#define PI_M_INT5 0004 /* Interrupt Level #5 */
#define PI_M_INT6 0002 /* Interrupt Level #6 */
#define PI_M_INT7 0001 /* Interrupt Level #7 - Lowest Priority */

// Time Base
#define TB_MASK    037777777777777777777LL
#define TB_JIFFY   010
#define TB_HW_BITS 12

// Previous Context Flags (PXCT)
//
// Bit   Reference Mode in Previous Context if Bit is 1.
// ---   -----------------------------------------------
//  9    Effective address calculation of instruction
//       Both instruction words in EXTEND
//
// 10    Memory operands specified by E, fetch or store
//       Byte pointer
//       Second instruction word in EXTEND
//
// 11    Effective address caluclation of byte pointer
//       source in EXTEND
//       Effective address calculation of EXTEND source
//          pointer if bit 9 is 1.
//
// 12    Byte data
//       Stack in PUSH or POP
//       Source in BLT
//       Destination in EXTEND
//       Effective calculation of EXTEND destination
//          pointer if bit 9 is 1.

#define PXCT_DST_EA   010 // Effective Address Destination
#define PXCT_DST_DATA 004 // Data Destination
#define PXCT_SRC_EA   002 // Effective Address Source
#define PXCT_SRC_DATA 001 // Data Source
#define PXCT_CUR      000 // Current (No PXCT switches)
#define NOPXCT        000 // No PXCT Switches

#define PXCT_EA       PXCT_DST_EA
#define PXCT_DATA     PXCT_DST_DATA
#define PXCT_BLT_DST  PXCT_DST_DATA
#define PXCT_BLT_SRC  PXCT_SRC_DATA
#define PXCT_BP_EA    PXCT_SRC_EA
#define PXCT_BP_DATA  PXCT_SRC_DATA
#define PXCT_EXT_EA   PXCT_SRC_EA
#define PXCT_STACK    PXCT_SRC_DATA
#define PXCT_XSRC     PXCT_SRC_EA
#define PXCT_XDST     PXCT_SRC_DATA

/*
 * KS10 Memory Management
 *
 * WREBR Instruction - Write Executive Base Register - Opcode 70120
 * RDEBR Instruction - Read Executive Base Register  - Opcode 70124
 *
 * |<-------- Right Halfword --------->|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0 0 0|2|P|0 0|  Exec Base Address  |
 * +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
 *  1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3
 *  8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 * MSB                               LSB
 *
 * Bit 21 (2) - TOPS-20 Paging
 * Bit 22 (P) - Enable Pager
 * Bit 25-35  - Executive Base Address
 *
 */

#define PG_EBR_M_TOPS20_PAGING   0040000LL
#define PG_EBR_M_ENA_PAGER       0020000LL
#define PG_EBR_M_EXEC_BASE_ADDR  0003777LL

#define PG_EBR_P_TOPS20_PAGING   (35 - 21)
#define PG_EBR_P_ENA_PAGER       (35 - 22)
#define PG_EBR_P_EXEC_BASE_ADDR  (35 - 35)

/*
 * WRUBR Instruction - Write User Base Register - Opcode 70114
 * RDUBR Instruction - Read User Base Register  - Opcode 70104
 *
 * |<--------- Left Halfword --------->|<--------- Right Halfword -------->|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |S|0|L|0 0 0| CAC | PAC |0 0 0 0 0 0|0 0 0 0 0 0 0|  User Base Address  |
 * +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
 *  0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 * MSB                                                                   LSB
 *
 * Bit 0 (S)  - Select AC Blocks
 * Bit 1 (L)  - Load User Base Address
 * Bit 6-8    - Current AC Block
 * Bit 9-11   - Pervious AC Block
 * Bit 25-35  - User Base Address
 *
 */

#define PG_UBR_M_SEL_AC_BLOCKS        0400000000000LL
#define PG_UBR_M_LD_USER_BASE_ADDR    0100000000000LL
#define PG_UBR_M_CUR_AC_BLOCK         0007000000000LL
#define PG_UBR_M_PREV_AC_BLOCK        0000700000000LL
#define PG_UBR_M_USER_BASE_ADDR       0000000003777LL

#define PG_UBR_P_SEL_AC_BLOCKS        (35 - 0)
#define PG_UBR_P_LD_USER_BASE_ADDR    (35 - 2)
#define PG_UBR_P_CUR_AC_BLOCK         (35 - 8)
#define PG_UBR_P_PREV_AC_BLOCK        (35 - 11)
#define PG_UBR_P_USER_BASE_ADDR       (35 - 35)

#define PG_UBR_M_ACB    0407700000000LL // AC Blocks Mask
#define PG_UBR_M_UADDR  0100000017777LL // User Base Address Mask
#define PG_UBR_MASK     0507700017777LL

// TOPS-10 Paging System
//
// Page Table Entry (Executive/User)
//
// |<--- Data for even virtual page -->|<--- Data for odd virtual page --->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |A|0|W|S|C|0 0 0| Physical Page Adr |A|0|W|S|C|0 0 0| Physical Page Adr |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// A - Access Allowed
// P - Public   (Not used for KS10 Processor)
// W - Writable (Not write-protected)
// S - Software (Not interpreted by hardware)
// C - Cacheable

#define PTE_T10_M_EVEN_PAGE   0777777000000LL
#define PTE_T10_M_ODD_PAGE    0000000777777LL

#define PTE_T10_P_EVEN_PAGE   (35 - 17)
#define PTE_T10_P_ODD_PAGE    (35 - 35)

// Even/odd page format
#define PTE_T10_ACCESS   0400000 // Accessible
#define PTE_T10_PUBLIC   0200000 // Public - Not Used
#define PTE_T10_WRITABLE 0100000 // Writable
#define PTE_T10_SOFTWARE 0040000 // Software
#define PTE_T10_CACHE    0020000 // Cachable
#define PTE_T10_PADDR    0001777 // Page Address

#define PTE_T10_EVEN(x) (((x) & PTE_T10_M_EVEN_PAGE) >> PTE_T10_P_EVEN_PAGE)
#define PTE_T10_ODD(x)  ((x) & PTE_T10_M_ODD_PAGE)

// TOPS-20 Paging System 
//
// Page Table Entry (Executive/User for both Section/Map pointers)
//
// No Access
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  0  |            Available to Software (No Access)                    |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// MSB                                                                   LSB
//
// Immediate Pointer
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  1  |0|W|0|C|0 0 0 0 0|  Storage  |0 0 0 0 0|  Pager Num of Page Map  |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// MSB                                                                   LSB
//
// Shared Pointer
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  2  |0|W|0|C|0 0 0 0 0 0 0 0 0 0 0|       Index to SPT Location       |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// MSB                                                                   LSB
//
// Indirect Pointer
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  3  |0|W|0|C|0 0| Sec Table Index |       Index to SPT Location       |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// MSB                                                                   LSB
//
// W - Writable (Not write-protected)
// C - Cachable

// Section/map pointers
#define PTE_T20_ACCESS 0700000000000LL // Access Mode
#define PTE_T20_WRITE  0020000000000LL // Writable Bit
#define PTE_T20_CACHE  0004000000000LL // Cachable Bit
#define PTE_T20_STM    0000077000000LL // Storage Medium

#define PTE_T20_PIDX   0000777000000LL // Page Index
#define PTE_T20_SIDX   0000000777777LL // SPT Index
#define PTE_T20_PNUM   0000000017777LL // Page Number

// Position for shift functions
#define PTE_T20_P_ACCESS (35 - 2)
#define PTE_T20_P_PIDX   (35 - 17)

// Access Mode
#define PTE_T20_NOA 0 // No access
#define PTE_T20_IMM 1 // Immediate Pointer
#define PTE_T20_SHR 2 // Shared Pointer
#define PTE_T20_IND 3 // Indirect Pointer

// Core Status Format for CST entries
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   State Code    |                    Reserved                       |M|
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// MSB                                                                   LSB
//
// Bit    Label
// ---    -----
// <0:5>  Page Age
// <0:8>  State Code
// <35>   Page Modified

#define CST_AGE      0770000000000LL
#define CST_STATE    0777000000000LL
#define CST_MODIFIED 0000000000001LL

// Page fail word for paging system (Left halfword)

#define PFW_USER         0400000 // User Mode Reference
#define PFW_HARD         0200000 // Hardware Failure Reference
#define PFW_T10_ACCESS   0100000 // T10: PTE Access (A) Bit
#define PFW_T10_WRITE    0040000 // T10: PTE Write (W) Bit
#define PFW_T10_SOFTWARE 0020000 // T10: PTE Software (S) Bit
#define PFW_T20_DONE     0100000 // T20: Evaluation Done
#define PFW_T20_MODIFIED 0040000 // T20: Modified Reference
#define PFW_T20_WRITE    0020000 // T20: PTE Write (W) bit
#define PFW_WRITE        0010000 // Write Reference
#define PFW_PUBLIC       0004000 // Public Reference (Not used)
#define PFW_CACHE        0002000 // Cache Reference
#define PFW_PAGED        0001000 // Paged (Virtual) Reference
#define PFW_IO           0000200 // I/O Reference
#define PFW_BYTE         0000020 // I/O Byte Reference
#define PFW_PHYSICAL     0160000 // Physical Address Reference

#define PFW_BIO          0200000 // I/O Address Error
#define PFW_UNM          0360000 // Uncorrectable Memory Error
#define PFW_NXM          0370000 // Non-Existent Memory Error

// Temp. for UBA page fail trap
#define PFW1_USER  0400000000000LL // User Mode Reference
#define PFW1_HARD  0200000000000LL // Hardware Failure Reference
#define PFW1_PAGED 0001000000000LL // Paged (Virtual) Reference
#define PFW1_IO    0000200000000LL // I/O Reference
#define PFW1_BYTE  0000020000000LL // I/O Byte Reference

// User/Executive Process Table

// Executive Process Table
#define EPT_PI_BASE     0040 // Priority Interrupt Instruction Table
#define EPT_UBA_BASE    0100 // UBA Vector Interrupt Base
#define EPT_UBA_1       0101 // Unibus Adapter 3: Table Pointer
#define EPT_UBA_3       0103 // Unibus Adapter 3: Table Pointer

#define EPT_TR_BASE     0420 // Executive Trap Base
#define EPT_TR_AROV     0421 // Trap 1: Arithmetic Overflow Trap Instruction
#define EPT_TR_STACK    0422 // Trap 2: Stack Overflow Trap Instruction
#define EPT_TR_TRAP3    0423 // Trap 3: Software Trap Instruction

#define EPT_T20_SECTION 0540 // TOPS-20: Section 0 Pointer

// Executive Process Table for TOPS-20 operating system
#define T20_EPT_PI_INST      0042 // Priority Interrupt Instructions
#define T20_EPT_ADAPTER_1    0101 // Adapter 1 Interrupt Table Pointer
#define T20_EPT_ADAPTER_3    0103 // Adapter 3 Interrupt Table Pointer
#define T20_EPT_TRAP_BASE    0420 // Executive Trap Base
#define T20_EPT_AROV_TRAP    0421 // Arithmetic Overflow Trap Instruction
#define T20_EPT_STACK_TRAP   0422 // Stack Overflow Trap Instruction
#define T20_EPT_TRAP_3       0423 // Trap 3 Trap Instruction
#define T20_EPT_SECTION      0540 // Section 0 Pointer

// User Process Table
#define UPT_TR_BASE       0420 // Executive Trap Base
#define UPT_TR_AROV       0421 // Trap 1: Arithmetic Overflow Trap Instruction
#define UPT_TR_STACK      0422 // Trap 2: Stack Overflow Trap Instruction
#define UPT_TR_TRAP3      0423 // Trap 3: Software Trap Instruction

#define UPT_T10_PF_WORD   0500 // TOPS-10: Page Fail Word
#define UPT_T10_PF_OLD_PC 0501 // TOPS-10: Page Fail - Old PC Word
#define UPT_T10_PF_NEW_PC 0502 // TOPS-10: Page Fail - New PC Word

#define UPT_T20_PF_WORD   0500 // TOPS-20: Page Fail Word
#define UPT_T20_PF_FLAGS  0501 // TOPS-20: Page Fail - PC Flags
#define UPT_T20_PF_OLD_PC 0502 // TOPS-20: Page Fail - Old PC Word
#define UPT_T20_PF_NEW_PC 0503 // TOPS-20: Page Fail - New PC Word

#define UPT_T20_SECTION   0540 // TOPS-20: Section 0 Pointer

// User Process Table for TOPS-10 operating system
#define T10_UPT_PF_WORD      0500 // Page Fail Word
#define T10_UPT_PF_OLD_PC    0501 // Page Fail - Old PC Word
#define T10_UPT_PF_NEW_PC    0502 // Page Fail - New PC Word

// User Process Table for TOPS-20 operating system
#define T20_UPT_TRAP_BASE    0420 // User Trap Base
#define T20_UPT_AROV_TRAP    0421 // Arithmetic Overflow Trap Instruction
#define T20_UPT_STACK_TRAP   0422 // Stack Overflow Trap Instruction
#define T20_UPT_TRAP_3       0423 // Trap 3 Trap Instruction
#define T20_UPT_MUUO         0424 // MUUO Flags and Opcode, A
#define T20_UPT_MUUO_OLD_PC  0425 // MUUO Old PC Word
#define T20_UPT_MUUO_OLD_E   0426 // MUUO Old E Address
#define T20_UPT_MUUO_NEW_PC  0427 // MUUO Process Context Word
#define T20_UPT_MUUO_EXEC_NO_TRAP 0428 // New PC Word
#define T20_UPT_MUUO_EXEC_TRAP    0429 // New PC Word
#define T20_UPT_MUUO_USER_NO_TRAP 0434 // New PC Word
#define T20_UPT_MUUO_USER_TRAP    0435 // New PC Word
#define T20_UPT_PF_WORD      0500 // Page Fail Word
#define T20_UPT_PF_FLAGS     0501 // Page Fail - PC Flags
#define T20_UPT_PF_OLD_PC    0502 // Page Fail - Old PC Word
#define T20_UPT_PF_NEW_PC    0503 // Page Fail - New PC Word
#define T20_UPT_SECTION      0540 // Section 0 Pointer

#define T20_SECTION          0540 // Section 0 Pointer

// Cache Entry Table Definitons
#define C_VALID    0x80000000 // Valid bit
#define C_USER     0x40000000 // User/Executive bit
#define C_MODIFIED 0x20000000 // Modified bit
#define C_ADDRESS  0x001FFFFF // 21-bit Address mask
#define C_MBZ      0xE01FFFFF // Must be zeros

// LUUO/MUUO Instructions
#define LUUO_M_OPCODE 0777740777777LL // Clear @ and index bits
#define MUUO_M_OPCODE 0777740777777LL // Clear @ and index bits

// For TOSP-10 Operating System
#define MUUO_T10_OPCODE   0424 // MMUO Instruction
#define MUUO_T10_OLD_PC   0425 // MUUO Old PC Word w/Flags
#define MUUO_T10_PCW      0426 // MUUO Previous-Context Word

// For TOPS-20 Operating System
#define MUUO_T20_OPCODE   0424 // MMUO Flags and Opcode, A
#define MUUO_T20_OLD_PC   0425 // MUUO Old PC Word
#define MUUO_T20_OLD_E    0426 // MUUO Old E Word
#define MUUO_T20_PCW      0427 // MUUO Previous-Context Word

// For KS10 Processor MUUO traps
#define MUUO_NEW_PC_BASE  0430
#define MUUO_EXEC_NO_TRAP 0430
#define MUUO_EXEC_TRAP    0431
#define MUUO_USER_NO_TRAP 0434
#define MUUO_USER_TRAP    0435

#define MUUO_M_TRAP       0001
#define MUUO_M_USER       0004

/*
 * KS10 Processor - I/O Address
 *
 * |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0 0 0 0 0 0 0 0 0 0 0 0 0 0|   C   |           Register Address        |
 * +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
 *  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 * MSB                                                                   LSB
 *
 * Note: C - the controller number
 *
 *   Controller   Register Address   Specifies
 *   ----------   ----------------   ---------
 *       0        100000             Memory Status
 *       0        200000             Console (microcode only)
 *       1        400000-777777      Adapter 1 Unibus registers
 *       3        400000-777777      Adapter 3 Unibus registers
 *
 */

#define IO_M_CONTROLLER 017000000 // Controller #
#define IO_M_REG_ADDR   000777777 // 18-Bit Register Address

#define IO_P_CONTROLLER 18

// Macros to extract fields from I/O address
#define IO_CONTROLLER(x)  (((x) & IO_M_CONTROLLER) >> IO_P_CONTROLLER)
#define IO_REG_ADDR(x)    ((x) & IO_M_REG_ADDR)

// I/O Address mask for effective addreess (E)
#define IOA_SIZE (1 << 22)      // Limit I/O Address
#define IOA_MASK (IOA_SIZE - 1) // I/O Address Mask

// HSB Location
#define HSB_L_MAG   (HSB + 000)
#define HSB_L_PC    (HSB + 001)
#define HSB_L_HR    (HSB + 002)
#define HSB_L_AR    (HSB + 003)
#define HSB_L_ARX   (HSB + 004)
#define HSB_L_BR    (HSB + 005)
#define HSB_L_BRX   (HSB + 006)
#define HSB_L_ONE   (HSB + 007)
#define HSB_L_EBR   (HSB + 010)
#define HSB_L_UBR   (HSB + 011)
#define HSB_L_MASK  (HSB + 012)
#define HSB_L_FLG   (HSB + 013)
#define HSB_L_PI    (HSB + 014)
#define HSB_L_XWD1  (HSB + 015)
#define HSB_L_T0    (HSB + 016)
#define HSB_L_T1    (HSB + 017)
#define HSB_L_VMA   (HSB + 020)

// Halt Status Base
#define HSB_M_STORAGE   0400000000000LL

// Unibus Interface definitions
#include "pdp10/ks10_uba.h"

extern int18 lhPFW;
extern int30 rhPFW;

typedef struct ks10_Device KS10_DEVICE;
typedef struct ks10_Device KS10_CPU;
struct ks10_Device {
	P10_CPU        cpu;   // PDP-10 CPU Header
	KS10UBA_DEVICE *uba;
};
