// fe.h - KS10 Processor: front-end 8080 routines
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS-10 Emulator.
// See README for copyright notice.
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

// Front-End Console - 8080 Communications
// For KS10 Processor (DECsystem-20 Model 2020)
//
// Location 031 - Keep Alive and Status Word
//
//    Bit     Definition
//    ---     ----------
//    4       Reload
//    5       Examine Keep Alive
//    6       KLINIK Active
//    7       Parity Error detect enabled
//    8       CRAM Parity Error detect enabled
//    9       DP Parity Error detect enabled
//    10      Cache Enabled
//    11      1 microsec enabled
//    12      TRAPS - Enabled
//    20:27   Keep Alive
//    32      Boot Switch Boot
//    33      Power Fail
//    34      Forced Reload
//    35      Keep Alive failed to change
//
// Location 032 - KS10 CTY Input Word (from 8080)
//
//    Bit     Definition
//    ---     ----------
//    20:27   0 = No action, 1 = CTY Character Pending
//    28:35   CTY Character
//
// Location 033 - KS10 CTY Output Word (to 8080)
//
//    Bit     Definition
//    ---     ----------
//    20:27   0 = No action, 1 = CTY Character Pending
//    28:35   CTY Character
//
// Location 034 - KS10 KLINIK User Input Word (from 8080)
//
//    Bit     Definition
//    ---     ----------
//    20:27   0 = No action, 1 = KLINIK Character,
//            2 = KLINIK Active, 3 = KLINIK Carrier Loss 
//    28:35   KLINIK Character
//
// Location 035 - KS10 KLINIK User Output Word (to 8080)
//
//    Bit     Definition
//    ---     ----------
//    20:27   0 = No action, 1 = KLINIK Character,
//            2 = Hangup request
//    28:35   KLINIK Character
//
// Input Process 8080 -> KS10
//    8080 gets interrupted "TTY-Char Available", 8080 gets character, and
//    delivers into CTYIWD (32) with flag(s) and set KS-10 interrupt.
//
// Output Process KS10 -> 8080
//    Load character and flag into CTYOWD (33), set 8080 interrupt, 8080
//    Examines CTYOWD and get it, clears interrupt, sends character
//    hardware, clears CTYOWD, and set KS-10 interrupt.

#define FE_HALTSW  030 // Halt Switch
#define FE_KASWD   031 // Keep Alive and Status Word
#define FE_CTYIWD  032 // Console TTY Input Word
#define FE_CTYOWD  033 // Console TTY Output Word
#define FE_KLUIWD  034 // KLINIK User Input Word
#define FE_KLUOWD  035 // KLINIK User Output Word
#define FE_BRH11BA 036 // Boot RH-11 Base Address
#define FE_BDRVNUM 037 // Boot Drive Number
#define FE_MTBFSN  040 // Magtape Boot Format and Slave Number

// 8080 Disk Address for bootstrap
//
// MSB                                                                     LSB
//  |<--------- Left Halfword --------->|<-------- Right Halfword --------->|
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |0 0 0|     Cylinder    |0 0 0 0 0 0 0 0 0 0 0|  Track  |0 0 0| Sector  |
//  +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//   0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
//   Bits     Defintions
//  -------   ----------
//  <3:11>    Cylinder
//  <23:27>   Track
//  <31:35>   Sector

// Locations at HOM block for bootstrap
#define FE_BT_DADDR  0101 // Disk address of FE-FILE
#define FE_BT_LEN    0102 // Number of sectors
#define FE_BT_8080   0103 // 8080 Cylinder/Track/Sector

// Masks in 8080 disk address word
#define FE_M_DA_CYL  0077700000000LL // Cylinder field  (0 - 511)
#define FE_M_DA_TRK  0000000017600LL // Track field     (0 - 31)
#define FE_M_DA_SEC  0000000000037LL // Sector field    (0 - 31)
#define FE_M_DA_MBZ  0077700017637LL // Must be zeros

// Positions in 8080 disk address word
#define FE_P_DA_CYL  (35 - 11)
#define FE_P_DA_TRK  (35 - 27)
#define FE_P_DA_SEC  (35 - 35)

// Extract fields from 8080 disk address word
#define FE_DA_CYL(x) (((x) & FE_M_DA_CYL) >> FE_P_DA_CYL)
#define FE_DA_TRK(x) (((x) & FE_M_DA_TRK) >> FE_P_DA_TRK)
#define FE_DA_SEC(x) ((x) & FE_M_DA_SEC)

// FE-FILE Page 0 table
#define FE_P0_FREESPACE            000 // Pointer to Free Space
#define FE_P0_MICROCODE            002 // Pointer to Microcode
#define FE_P0_MONITOR_PREBOOT      004 // Pointer to Monitor Pre-Boot
#define FE_P0_DIAGNOSTIC_PREBOOT   006 // Pointer to Diagnostic Pre-Boot
#define FE_P0_BOOTCHECK1_MICROCODE 010 // Pointer to Boot Check 1 Microcode
#define FE_P0_BOOTCHECK2_PREBOOT   012 // Pointer to Boot Check 2 Pre-Boot
#define FE_P0_MONITOR_BOOT         014 // Pointer to Monitor Boot
#define FE_P0_DIAGNOSTIC_BOOT      016 // Pointer to Diagnostic Boot
#define FE_P0_BOOTCHECK2           020 // Pointer to Boot Check 2
#define FE_P0_INDIRECT_FILE        022 // Pointer to Indirect File #0
