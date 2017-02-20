// kl10.h - Definitions for the KL10 Processor emulation
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

// KL10 Processor - definitions
#define KL10A_KEY    "KL10A"
#define KL10B_KEY    "KL10B"
#define KL10_NAME    "DECsystem-10/20 Emulator"
#define KL10_VERSION "v0.4 (Alpha)"

// Memory Size and Limitation
#define MAXMEMSIZE  (1 << 23) // 23-bit Addressing   - 4096K words
#define MAXSECSIZE  037       // Maximum Section     - 37 sections
#define INIMEMSIZE  (1 << 20) // Initial memory size - 1024K words

#define NACBLOCKS 8   //  8 AC blocks
#define ACBLKSIZE 020 // 16 Accumulators

// Arithmetic Processor Identification
//
// ID Register - Read/Write Access
//
// |<-------- Left Halfword ---------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  Ucode Options  |  Ucode Version  | Hard Opts |     Serial Number     |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
//  Bits    Defintion
//  ----    ---------
//  0:8     Microcode Options
//   0        TOPS-20 Paging System (0 - T10, 1 - T20)
//   1        Extended Addressing
//   2        Exotic Microcode
//  9:17    Microcode Version
// 18:23    Hardware Options
//   18       50 Hz (0 - 60 Hz, 1 - 50 Hz)
//   19       Cache
//   20       Extended KL10 (0 - Single Section, 1 - Extended Section)
//   21       Master Osciiator
// 24:35    Serial Number

#define APRID_UC_OPTS    0777000000000LL // Microcode Options
#define APRID_UO_T20     0400000000000LL //    TOPS-20 Paging System
#define APRID_UO_XADR    0200000000000LL //    Extended Addressing
#define APRID_UO_EXOTIC  0100000000000LL //    Exotic Microcode
#define APRID_UC_VER     0000777000000LL // Microcode Version
#define APRID_HW_OPTS    0000000770000LL // Hardware Options
#define APRID_HO_50HZ    0000000400000LL //    50 Hz
#define APRID_HO_XKL     0000000200000LL //    Extended KL10
#define APRID_HO_MOSC    0000000100000LL //    Master OSC
#define APRID_SN         0000000007777LL // Serial Number

// Arithmetic Processor
//
// Status Register - Write Access
//
// |<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |0|R|       |Selected Flags |0| PIA |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5

#define APRSR_CLRIO   0200000 // (W)   Clear All In/Out Devices (Reset I/O)
#define APRSR_SWPBUSY 0200000 // (R)   Sweep Busy
#define APRSR_ENABLE  0100000 // (W)   Enable Flags
#define APRSR_DISABLE 0040000 // (W)   Disable Flags
#define APRSR_CLEAR   0020000 // (W)   Clear Flags
#define APRSR_SET     0010000 // (W)   Set Flags
#define APRSR_SBERR   0004000 // (R/W) Flag: S-BUS Error
#define APRSR_NXM     0002000 // (R/W) Flag: No Memory (NXM)
#define APRSR_IOPF    0001000 // (R/W) Flag: In/Out Page Failure
#define APRSR_MBPAR   0000400 // (R/W) Flag: MB Parity
#define APRSR_CDPAR   0000200 // (R/W) Flag: Cache Directory Parity
#define APRSR_ADRPAR  0000100 // (R/W) Flag: Address Parity
#define APRSR_PWRFAIL 0000040 // (R/W) Flag: Power Failure
#define APRSR_SWPDONE 0000020 // (R/W) Flag: Sweep Done
#define APRSR_IRQ     0000010 // (R)   Interrupt Request
#define APRSR_PIA     0000007 // (R/W) Priority Interrupt Assignment
#define APRSR_FLAGS   0007760 //       Flag Mask

// Debug Register - Read/Write Access
//
// |<-------- Left Halfword ---------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |0 0 0 0 0 0 0 0 0|F|R|W|U|               Break Address                 |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
//  Bit     Defintion
//  ---     ---------
//   9      Fetch |  
//  10      Read  | Reference Type
//  11      Write | 
//  12      User Space
//  13:35   Break Address

#define APRDR_FETCH 0000400000000 // Reference - Fetch
#define APRDR_READ  0000200000000 // Reference - Read
#define APRDR_WRITE 0000100000000 // Reference - Write
#define APRDR_USER  0000040000000 // Reference - User Space
#define APRDR_FLAGS 0000740000000 // Access Flags
#define APRDR_BREAK 0000037777777 // Break Address

// Priority Interrupt System
//
// Status Register - Write Access
//
// |<------------------ Right Halfword ----------------->|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |PA|PD|PR|00|DR|CS|II|I+|I-|S+|S-|   Selected Levels  |
// +--+--+--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
//
//  Bit     Defintion
//  ---     ---------
//  18      (PA) Addresss   |     
//  19      (PD) Data       | Write Even Parity
//  20      (PR) Directory  |
//  22      (DR) Drop Program Requests (on selected Levels)
//  23      (CS) Clear PI System
//  24      (II) Initial Interrupts On |  
//  25      (I+) Turn Interrupts On    | (on selected Levels)
//  26      (I-) Turn Interrupts Off   |
//  27      (S+) Turn PI System On
//  28      (S-) Turn PI System Off
//  29:35   Selected Levels
//
// Status Register - Read Access
//
// |<------------------ Left Halfword ------------------>|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |00 00 00 00 00 00 00 00 00 00 00|  Program Requests  |
// +--+--+--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17
//
// |<------------------ Right Halfword ----------------->|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |PA|PD|PR|    In Progress     |On|     Levels On      |
// +--+--+--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
//
//  Bit     Defintion
//  ---     ---------
//  11:17   Program Requests on Levels
//  18      (PA) Addresss   |
//  19      (PD) Data       | Write Even Parity
//  20      (PR) Directory  |
//  21:27   Interrupt in Progress on Levels
//  28      PI System On
//  29:35   Levels On

#define PISR_PADR    0400000 // (R/W) Address   |
#define PISR_PDATA   0200000 // (R/W) Data      | Write Even Parity
#define PISR_PDIR    0100000 // (R/W) Directory |
#define PISR_DROP    0020000 // (W)   Drop Program Requests
#define PISR_CLEAR   0010000 // (W)   Clear PI System
#define PISR_REQ     0004000 // (W)   Initial Interrupts On (Requests)
#define PISR_ION     0002000 // (W)   Turn Interrupts On
#define PISR_IOFF    0001000 // (W)   Turn Interrupts Off 
#define PISR_OFF     0000400 // (W)   Turn PI System Off
#define PISR_ON      0000200 // (R/W) Turn PI System On
#define PISR_LEVELS  0000177 // (R/W) Interrupt Levels Mask
#define PISR_PFLAGS  0700000 //       Parity Flags Mask

#define PISR_REQS    0000177000000 // Program Requests
#define PISR_PROGS   0000000077400 // Interrupts in Progress
#define PISR_ENABLES 0000000000177 // Interrupts On

// PI Interrupt Mask
#define PI_INT0 0000 // Interrupt Level #0 - No Interrupts
#define PI_INT1 0100 // Interrupt Level #1 - Highest Priority
#define PI_INT2 0040 // Interrupt Level #2
#define PI_INT3 0020 // Interrupt Level #3
#define PI_INT4 0010 // Interrupt Level #4
#define PI_INT5 0004 // Interrupt Level #5
#define PI_INT6 0002 // Interrupt Level #6
#define PI_INT7 0001 // Interrupt Level #7 - Lowest Priority

// Interrupt Function Word
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |Space|Func |Q|Device |0 0|             Interrupt Address               |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
//  Field      Description           Bits
//  -----      --------------------  -------
//  Space      Address Space         <0:2>
//  Func       Function to Perform   <3:5>
//  Q          Function Qualifier    <6>
//  Device     Physical Device       <7:10>
//  Address    Interrupt Address     <13:35>
//
//  Address Space
//  -------------
//  0 = EPT (Executive Process Table) Address
//  1 = Executive Virtual Address
//  4 = Physical Address
//
//  Function
//  --------
//  0 = Internal Device
//      Zero word.  For interval counter perform a vector interrupt.
//      Otherwise, perform a standard interrupt.
//  1 = Standard Interrupt
//      Excute an instruction in the location 40 + 2N of the executive
//      process table (EPT)
//  2 = Vector Interrupt
//      Interval Counter - execute the instruction in the location 514
//        of the executive process table (EPT).
//      DTE20 - execute the instruction in the location 2 of the DTE20
//        control block
//      Channel - execute the instruction in the location of EPT by
//        27-35 bit of interrupt function word.
//      DIA20 - dispatch interrupt; execute the instruction in the
//        location of executive virtual address (13-35).
//  3 = Increment - add or subtract one in the location of the
//        executive virtual address. If Q = 0, add one, Q = 1,
//        subtract one.

//  Device
//  ------
//  0-7   = RH20 Channel 0-7
//  10-13 = DTE20 Channel 10-13
//  17    = DIA20 Channel 17

// Interrupt Function Word Field
#define IRQ_SPC  0700000000000 // Address Space
#define IRQ_FNC  0070000000000 // Function to Perform
#define IRQ_QUAL 0004000000000 // Qualifier
#define IRQ_DEV  0003600000000 // Physical Device
#define IRQ_ADR  0000037777777 // Interrupt Address

#define IRQ_VEC  0020000000000 // Function: Vector Interrupt

// Mask of fields
#define IRQ_M_SPC  07          // Address Space
#define IRQ_M_FNC  07          // Function to Perform
#define IRQ_M_QUAL 01          // Qualifier
#define IRQ_M_DEV  017         // Physical Device
#define IRQ_M_ADR  037777777   // Interrupt Address

// Position of fields
#define IRQ_P_SPC  33          // Address Space
#define IRQ_P_FNC  30          // Function to Perform
#define IRQ_P_QUAL 29          // Qualifier
#define IRQ_P_DEV  25          // Physical Device

// Address Space
#define IRQ_SPC_EPT  0  // Executive Process Table
#define IRQ_SPC_EXEC 1  // Executive Virtual Address
#define IRQ_SPC_PHY  4  // Physical Address

// Functon to Perform
#define IRQ_FNC_INT  0  // Internal Device
#define IRQ_FNC_STD  1  // Standard Interrupt
#define IRQ_FNC_VEC  2  // Vector Interrupt
#define IRQ_FNC_INC  3  // Increment
#define IRQ_FNC_EXA  4  // Examine
#define IRQ_FNC_DEP  5  // Deposit
#define IRQ_FNC_TRA  6  // Byte Transfer

// Device Code
#define IRQ_DEV_CH0  000 // RH20 Channel 0
#define IRQ_DEV_CH1  001 // RH20 Channel 1
#define IRQ_DEV_CH2  002 // RH20 Channel 2
#define IRQ_DEV_CH3  003 // RH20 Channel 3
#define IRQ_DEV_CH4  004 // RH20 Channel 4
#define IRQ_DEV_CH5  005 // RH20 Channel 5
#define IRQ_DEV_CH6  006 // RH20 Channel 6
#define IRQ_DEV_CH7  007 // RH20 Channel 7
#define IRQ_DEV_DTE0 010 // DTE20 Channel 10
#define IRQ_DEV_DTE1 011 // DTE20 Channel 11
#define IRQ_DEV_DTE2 012 // DTE20 Channel 12
#define IRQ_DEV_DTE3 013 // DTE20 Channel 13
#define IRQ_DEV_DIA  017 // DIA20 Channel 17

// Pager
// Cache Strategy
//
//   0x = Disable the cache.
//   10 = Look for all references, but do not load physical references.
//   11 = Make complete of the cache for physical references.
//   1x = Virtual preferences act as directed by cache bit in the mapping
//        for the page.

// Executive Base Register
#define EBR_LOOK   0400000 // Look | Cache Strategy
#define EBR_LOAD   0200000 // Load |
#define EBR_CACHE  0600000 // Cache Strategy Flags Mask
#define EBR_T20    0040000 // TOPS-20 Paging Mode (T20 = 1, T10 = 0)
#define EBR_ENABLE 0020000 // Pager System Enable (On = 1, Off = 0)
#define EBR_ADDR   0017777 // Executive Base Address (EPT)

// User Base Register
#define UBR_SELACB 0400000000000LL // Select AC Blocks
#define UBR_SELPCS 0200000000000LL // Select Previous Context Section
#define UBR_LDUPT  0100000000000LL // Load User Base Address (UPT)
#define UBR_CACB   0007000000000LL // Current AC Block
#define UBR_PACB   0000700000000LL // Previous Context AC Block
#define UBR_ACBS   0007700000000LL // Both AC Blocks
#define UBR_PCS    0000037000000LL // Prevous Context Section
#define UBR_NOACT  0000000400000LL // Do Not Update Accounts.
#define UBR_ADDR   0000000017777LL // User Base Address (UPT)

#define UBR_SELS   (UBR_SELACB|UBR_SELPCS|UBR_LDUPT)
#define UBR_P_CACB 27 // Current AC Block Position
#define UBR_P_PACB 24 // Previous AC Block Position
#define UBR_P_PCS  18 // Previous Section Position
#define UBR_M_ACB  007 // AC Block Mask
#define UBR_M_PCS  037 // Section Mask

// TOP-10 Paging
//
// |<- Data for Even Virtual Address ->|<- Data for Odd Virtual Address -->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |A|P|W|S|C|  Physcial Page Address  |A|P|W|S|C|  Physcial Page Address  |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5

#define P10_ACCESS   0400000 // Page is accessible
#define P10_PUBLIC   0200000 // Page is public
#define P10_WRITE    0100000 // Page is writable
#define P10_SOFTWARE 0040000 // Page is software
#define P10_CACHE    0020000 // Page is cacheable
#define P10_PAGNUM   0017777 // Page Number

// TOPS-20 Paging
//
// Section/Map Pointer
//
// No Access
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  0  |                    Available to Software                        |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Immediate Pointer
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  1  |P|W|0|C|0 0 0 0 0|  Storage  |0 0 0 0 0|       Page Number       |  
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Shared Pointer
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  2  |P|W|0|C|0 0 0 0 0 0 0 0 0 0 0|       Index to SPT Location       |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Indirect Pointer
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  3  |P|W|0|C|0 0|   Table Index   |       Index to SPT Location       | 
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5

#define P20_TYPE   0700000000000LL // Access Mode Field
#define P20_PUBLIC 0040000000000LL // Page is public
#define P20_WRITE  0020000000000LL // Page is writable
#define P20_CACHE  0004000000000LL // Page is cacheable
#define P20_TBLIDX 0000777000000LL // Section/Page Table Index
#define P20_STM    0000077000000LL // Storage Medium
#define P20_SPTIDX 0000000777777LL // SPT Location Index
#define P20_PAGNUM 0000000017777LL // Page Number

#define P20_M_TYPE   07   // Mask of Access Type Field
#define P20_P_TYPE   33   // Position of Access Type Field
#define P20_M_TBLIDX 0777 // Mask of Table Index Field
#define P20_P_TBLIDX 18   // Position of Table Index Field

#define P20_NOA 0 // No Access
#define P20_IMM 1 // Immediate
#define P20_SHR 2 // Shared
#define P20_IND 3 // Indirect

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

// Page Fail Word (18-bit left halfword)

#define PFW_USER     0400000 // User Reference
#define PFW_HARD     0200000 // Hardware Failure Reference
#define PFW_T10_A    0100000 // T10: PTE A (Access) bit
#define PFW_T10_W    0040000 // T10: PTE W (Writable) bit
#define PFW_T10_S    0020000 // T10: PTE S (Software) bit
#define PFW_T20_A    0100000 // T20: PTE A (Access) bit
#define PFW_T20_M    0040000 // T20: PTE M (Modified) bit
#define PFW_T20_W    0020000 // T20: PTE W (Writable) bit
#define PFW_WRITE    0010000 // Write Reference
#define PFW_PUBLIC   0004000 // Public Reference
#define PFW_CACHE    0002000 // Cache Reference
#define PFW_PAGED    0001000 // Paged Reference (Virtual)
#define PFW_PHYSICAL 0160000 // Physical Reference

// Hardware failure error w/PFW_HARD set.
#define PFW_PROPVIO 0210000 // Proprietary Violation
#define PFW_ADRFAIL 0230000 // Address Failure
#define PFW_ILLIND  0240000 // Illegal Indirect
#define PFW_PTERR   0250000 // Page Table Parity Error
#define PFW_ILLADR  0270000 // Illegal Address
#define PFW_ARERR   0360000 // AR Parity Error
#define PFW_ARXERR  0370000 // ARX Parity Error

// Timing and Accouting Section
//
// Time Base (510 and 511 EPT locations)
//
// |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          High Order of Count                          |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
// |0| Low Order of Count  |        Counter        |       Reserved        |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//

// Meter Status Register
//
//             |Accounting |Time Base  |
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// |SAC| 0 | 0 |PI |NPI|ON |OFF|ON |CLR|                       |     PI    |
// +---+---+---^---+---+---^---+---+---^---+---+---^---+---+---^---+---+---+
//  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35

#define MTR_SET   0400000 // Set Up Accounts
#define MTR_PI    0040000 // Accounting: Executive PI Account
#define MTR_NPI   0020000 // Accounting: Executive Non-PI Account
#define MTR_ON    0010000 // Accounting: Turn On
#define TIM_OFF   0004000 // Time Base: Turn Off
#define TIM_ON    0002000 // Time Base: Turn On
#define TIM_CLR   0001000 // Time Base: Clear
#define INT_PIA   0000007 // Interval Counter: PI Assignment

#define MTR_FLAGS (MTR_PI|MTR_NPI|MTR_ON)

//#define TIM_TICK  010000  // Tick count
#define TIM_TICK (10 * 010000)

// Interval Counter
//
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// |CLR| 0 | 0 |ON |CIF| 0 |               Interval Period                 |
// +---+---+---^---+---+---^---+---+---^---+---+---^---+---+---^---+---+---+
//  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35
//
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// | 0   0   0   0   0   0 |               Interval Count                  |
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// | 0   0   0 |ON |DON|OVF|               Interval Period                 |
// +---+---+---^---+---+---^---+---+---^---+---+---^---+---+---^---+---+---+
//  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35

#define INT_CLR    0400000 // Clear Interval Counter
#define INT_ON     0040000 // Turn On Interval Counter
#define INT_CFLG   0020000 // Clear Interval Flags
#define INT_DONE   0020000 // Done Flag
#define INT_OVF    0010000 // Overflow Flag
#define INT_COUNT  0007777 // Interval Count
#define INT_PERIOD 0007777 // Interval Period

// KL10 Process Table Configuration for TOPS-10/TOPS-20

#define EPT_PI_BASE1    0040 // Priority Interrupt Base 1
#define EPT_PI_BASE2    0041 // Priority Interrupt Base 2
#define UPT_LUUO_ADR    0420 // Address of LUUO Block
#define EPT_TR_BASE     0420 // Executive Trap Base
#define EPT_TR_AROV     0421 // Trap 1: Arithmetic Overflow Trap Instruction
#define EPT_TR_STACK    0422 // Trap 2: Stack Overflow Trap Instruction
#define EPT_TR_TRAP3    0423 // Trap 3: Trap 3 Trap Instruction

// Single KL10 for TOPS-10 (UPT)
#define T10_MUUO_OPCODE 0424 // MMUO Stored
#define T10_MUUO_OLDPC  0425 // MUUO Old PC
#define T10_MUUO_PCWORD 0426 // MUUO Process Context Word

// Single KL10 for TOPS-20 (UPT)
#define T20_MUUO_OPCODE 0425 // MMUO Stored
#define T20_MUUO_OLDPC  0426 // MUUO Old PC
#define T20_MUUO_PCWORD 0427 // MUUO Process Context Word

// Extended KL10 for TOPS-20 (UPT)
#define T20_XUUO_OPCODE 0424 // MUUO Flags, Opcode w/AC.
#define T20_XUUO_OLDPC  0425 // MMUO Old PC
#define T20_XUUO_EADDR  0426 // MUUO Effective Address
#define T20_XUUO_PCWORD 0427 // MUUO Process Context Word

// MUUO Trap Vectors (New PC words)
#define UPT_MUUO_NEWPC  0430 // MUUO New PC Word Base
#define UPT_KERN_NOTRAP 0430 // MUUO New PC Word - Kernel No Trap
#define UPT_KERN_TRAP   0431 // MUUO New PC Word - Kernel Trap
#define UPT_SUPR_NOTRAP 0432 // MUUO New PC Word - Supervisor No Trap
#define UPT_SUPR_TRAP   0433 // MUUO New PC Word - Supervisor Trap
#define UPT_CONC_NOTRAP 0434 // MUUO New PC Word - Concealed No Trap
#define UPT_CONC_TRAP   0435 // MUUO New PC Word - Concealed Trap
#define UPT_PUBL_NOTRAP 0436 // MUUO New PC Word - Public No Trap
#define UPT_PUBL_TRAP   0437 // MUUO New PC Word - Public Trap

#define LUUO_OPCODE     0777740777777LL // Clear @ and XR fields
#define MUUO_OPCODE     0777740777777LL // Clear @ and XR fields
#define MUUO_USER       0004
#define MUUO_PUBLIC     0002
#define MUUO_TRAP       0001

// Single KL10 for TOPS-10 (UPT)
#define T10_PF_WORD     0500 // Page Fail Word
#define T10_PF_OLDPC    0501 // Page Old PC Word
#define T10_PF_NEWPC    0502 // Page New PC Word

// Single KL10 for TOPS-20 (UPT)
#define T20_PF_WORD     0501 // Page Fail Word
#define T20_PF_OLDPC    0502 // Page Fail Old PC Word
#define T20_PF_NEWPC    0503 // Page Fail New PC Word

// Extended KL10 for TOPS-20 (UPT)
#define T20_XPF_WORD    0500 // Page Fail Word
#define T20_XPF_FLAGS   0501 // Page Fail Flags
#define T20_XPF_OLDPC   0502 // Page Fail Old PC Word
#define T20_XPF_NEWPC   0503 // Page Fail New PC Word

#define UPT_PROCTIME    0504 // User Process Execution Time (2 words)
#define UPT_MEMCOUNT    0506 // User Memory Reference Count (2 words)
#define EPT_TIMEBASE    0510 // Time Base (2 words)
#define EPT_PACOUNT     0512 // Performance Analysis Count (2 words)
#define EPT_INTERVAL    0514 // Interval Counter Interrupt Instruction
#define T20_SECTION     0540 // User/Exec Section Pointer (0-37)

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

// I/O Device Codes for KL10 Processor
#define KL10_APR (0 << 2) // (00) Arithmetic Processor
#define KL10_PI  (1 << 2) // (04) Priority Interrupt
#define KL10_PAG (2 << 2) // (10) Pager
#define KL10_CCA (3 << 2) // (14) Cache
#define KL10_TIM (4 << 2) // (20) Timer
#define KL10_MTR (5 << 2) // (24) Meter

typedef struct kl10_Device KL10_DEVICE;
typedef struct kl10_Device KL10_CPU;

struct kl10_Device {
	P10_CPU cpu;   // PDP-10 CPU Header
};

void kl10_InitAPR(KL10_DEVICE *); // Initialize Arithmetic Processor
void kl10_InitPI(KL10_DEVICE *);  // Initialize Priority Interrupt
void kl10_InitPAG(KL10_DEVICE *); // Initialize Pager
void kl10_InitCCA(KL10_DEVICE *); // Initialize Cache
void kl10_InitTIM(KL10_DEVICE *); // Initialize Timer
void kl10_InitMTR(KL10_DEVICE *); // Initialize Meter
