// vm.h - VAX Virtual Memory Handler
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
// See ReadMe for copyright notice.
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

// Page Table Entry Field
// ----------------------
//
// For Central Processor Unit (CPU):
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |V| PROT  |M|0|OWN|0 0|                 PFN                     |
// +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//
// For I/O Devices:
//
// PTE with Valid Page Frame Number
//   PTE<31,26,22> = 1xx
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |1| PROT  |M|0|OWN|0 0|                 PFN                     |
// +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//
// PTE with Valid Page Frame Number
//   PTE<31,26,22> = 000
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |0| PROT  |0|0|OWN|0|S|                 PFN                     |
// +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//
// Global Page Table Entry Index
//   PTE<31,26,22> = 001
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |0| PROT  |0|0|OWN|1|                  GPTX                     |
// +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//
// Invalid PTE, I/O Abort.
//   PTE<31,26,22> = 01x
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |0| PROT  |1|0|OWN|   |      Reserved for Software Use          |
// +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//
//
//  Label    Description
//  -----    -----------
//  V        Valid Bit
//  PROT     Protection Code
//  M        Modified Bit
//  OWN      Owner
//  PFN      Page Frame Number

#define PTE_V       0x80000000 // Valid Bit
#define PTE_PROT    0x78000000 // Protection Code
#define PTE_M       0x04000000 // Modified Bit
#define PTE_OWN     0x01800000 // Owner
#define PTE_GLOBAL  0x00400000 // Global Page Table Entry
#define PTE_PFN     0x0001FFFF // Page Frame Number

#define PTE_M_PROT  0xF        // Protection Code Mask
#define PTE_M_OWN   0x3        // Owner Mask
#define PTE_M_PFN   0x1FFFF    // Page Frame Number Mask

#define PTE_P_PROT  27         // Position of Protection Code
#define PTE_P_OWN   23         // Position of Owner

#define PTE_GETACC(x) (((x) >> PTE_P_PROT) & PTE_M_PROT)

#define VMA_PFN     8          // Position of Page Frame Number
#define VMA_PAGE    0xFF       // Page Byte Mask

// PTE Protection Code Table
//
//                          Accessibility
// Name      Code    Kernel  Exec    Super   User
// ----      ----    ------  ----    -----   ----
// NA        0       None    None    None    None
// Reserved  1               UNPREDICTABLE
// KW        2       Write   None    None    None
// KR        3       Read    None    None    None
// UW        4       Write   Write   Write   Write
// EW        5       Write   Write   None    None
// ERKW      6       Write   Read    None    None
// ER        7       Read    Read    None    None
// SW        8       Write   Write   Write   None
// SREW      9       Write   Write   Read    None
// SRKW      10      Write   Read    Read    None
// SR        11      Read    Read    Read    None
// URSW      12      Write   Write   Write   Read
// UREW      13      Write   Write   Read    Read
// URKW      14      Write   Read    Read    Read
// UR        15      Read    Read    Read    Read

#define PTE_NA   0x0 // No Access
#define PTE_KW   0x2 // Kernel Write
#define PTE_KR   0x3 // Kernel Read
#define PTE_UW   0x4 // User Write
#define PTE_EW   0x5 // Exec Write
#define PTE_ERKW 0x6 // Exec Read, Kernel Write
#define PTE_ER   0x7 // Exec Read
#define PTE_SW   0x8 // Super Write
#define PTE_SREW 0x9 // Super Read, Exec Write
#define PTE_SRKW 0xA // Super Read, Kernel Write
#define PTE_SR   0xB // Super Read
#define PTE_URSW 0xC // User Read, Super Write
#define PTE_UREW 0xD // User Read, Exec Write
#define PTE_URKW 0xE // User Read, Kernel Write
#define PTE_UR   0xF // User Read

// Virtual Address Definition
//
//           +-----------------------------------------+
// 0000 0000 |   |        P0 (Program) Region      |   |
//           |- -|- - - - - - P0 Length - - - - - -|- -|
// 3FFF FFFF |   V   P0 Region Growth Direction    V   |
//           +-----------------------------------------+
// 4000 0000 |   A   P1 Region Growth Direction    A   |
//           |- -|- - - - - - P1 Length - - - - - -|- -|
// 7FFF FFFF |   |        P1 (Control) Region      |   |
//           +-----------------------------------------+ 
// 8000 0000 |   |        S0 (System) Region       |   |
//           |- -|- - - - - - S0 Length - - - - - -|- -|
// BFFF FFFF |   V   S0 Region Growth Direction    V   |
//           +-----------------------------------------+
// C000 0000 |                                         |
//           |            Reserved Region              |
// FFFF FFFF |                                         |
//           +-----------------------------------------+

#define VA_P0             // P0 (Program) Region Area
#define VA_P1 (1u << 30)  // P1 (Control) Region Area
#define VA_S0 (1u << 31)  // S0 (System) Region Area

// Page Offset (Each page hold 512 bytes)
#define VA_N_OFF    9                     // Offset Size
#define VA_M_OFF    ((1 << VA_N_OFF) - 1) // Offset Mask
#define VA_PAGESIZE (1 << VA_N_OFF)       // Page Size

// Virtual Page Number
#define VA_N_VPN    (31 - VA_N_OFF)       // VPN Size
#define VA_P_VPN    VA_N_OFF              // VPN Position
#define VA_M_VPN    ((1 << VA_N_VPN) - 1) // VPN Mask
#define VA_VPN      ((VA_M_VPN << VA_N_OFF) & PAMASK) // VPN Field

// Translation Buffer Index
#define VA_N_TBI    12                    // TB Index Size
#define VA_M_TBI    ((1 << VA_N_TBI) - 1) // TB Index Mask
#define VA_TBSIZE   (1 << VA_N_TBI)       // TB Size

// Virtual Address Field Extraction
#define VA_GETOFF(va)  ((va) & VA_M_OFF)
#define VA_GETBASE(va) ((va) & ~VA_M_OFF)
#define VA_GETVPN(va)  (((va) >> VA_P_VPN) & VA_M_VPN)
#define VA_GETTBI(vpn) ((vpn) & VA_M_TBI)

// Translation Buffer Entry
//
//                                               |   Access Bits   |
//                                               | | Write | Read  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           Physical Frame Number             |M|U|S|E|K|U|S|E|K| 
// +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0

#define TLB_N_ACC   4
#define TLB_M_ACC   ((1 << TLB_N_ACC) - 1)
#define TLB_P_RACC  0
#define TLB_P_WACC  TLB_N_ACC
#define TLB_RACC    (TLB_M_ACC << TLB_P_RACC)
#define TLB_WACC    (TLB_M_ACC << TLB_P_WACC)
#define TLB_P_M     (TLB_N_ACC << 1)
#define TLB_M       (1 << TLB_P_M)
#define TLB_N_PFN   (PASIZE - VA_N_OFF)
#define TLB_M_PFN   ((1 << TLB_N_PFN) - 1)
#define TLB_PFN     (TLB_M_PFN << VA_N_OFF)

// TLB Access - Write and Read

#define WA_USER   ((1 << (AM_USER)) << TLB_WACC)
#define WA_SUPER  ((1 << (AM_SUPERVISOR)) << TLB_WACC)
#define WA_EXEC   ((1 << (AM_EXECUTIVE)) << TLB_WACC)
#define WA_KERN   ((1 << (AM_KERNEL)) << TLB_WACC)

#define RA_USER   ((1 << (AM_USER)) << TLB_RACC)
#define RA_SUPER  ((1 << (AM_SUPERVISOR)) << TLB_RACC)
#define RA_EXEC   ((1 << (AM_EXECUTIVE)) << TLB_RACC)
#define RA_KERN   ((1 << (AM_KERNEL)) << TLB_RACC)

typedef struct {
	int32 tag; // Tag Information
	int32 pte; // Process Table Entry
} TLBENT;

// Memory Management Fault Codes and Probe Results for exception
//
//  31                                    2 1 0
// +-------------------------------------+-+-+-+
// |                  0                  |M|P|L| :(SP)
// +-------------------------------------+-+-+-+
// | Some virtual address in the faulting page | 
// +-------------------------------------------+
// |        PC of faulting instruction         |
// +-------------------------------------------+
// |                  PSL                      |
// +-------------------------------------------+

#define MM_WRITE 4  // Write Access
#define MM_EMASK 3  // Mask aganist probe

#define MM_ACV   0  // Access-Control Violation
#define MM_LNV   1  // Length Not Valid
#define MM_PLNV  3  // PTE Length Not Valid
#define MM_TNV   4  // Translation Not Valid
#define MM_PTNV  6  // PTE Translation Not Valid
#define MM_OK    7  // Ok
#define MM_NXM   8  // Console Non-existant Memory
