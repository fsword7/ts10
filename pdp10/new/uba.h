// uba.h - KS10 Processor: Unibus emulation routines
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

#define UBA_MAX       5       // Numbers of Unibus adapters

// I/O Addresses for Unibus registers
#define UBA_BASE      0760000 // Beginning of I/O Page Map
#define UBA_INT_SADDR 0763000 // Start of internal registers
#define UBA_INT_EADDR 0763101 // End of internal registers
#define UBA_MAP_ADDR  0763000 // Map Registers
#define UBA_MAP_MASK  0000077 // Map Register Area Mask
#define UBA_SR_ADDR   0763100 // Status Register
#define UBA_MR_ADDR   0763101 // Maintenance Register

// I/O Address 7630xx - Paging Map Registers

// For write access
#define UBAW_MAP_FLAGS 0740000 // Flags Mask
#define UBAW_MAP_FRPW  0400000 // Force Read/Pause/Write
#define UBAW_MAP_EN16  0200000 // Disable upper 2 bits on xfers
#define UBAW_MAP_FME   0100000 // Fast Mode Enable
#define UBAW_MAP_VALID 0040000 // Page is valid
#define UBAW_MAP_PAGE  0003777 // Page Number

// For read access
#define UBA_MAP_RAMP   0020000000000LL // RAM Parity
#define UBA_MAP_FRPW   0010000000000LL // Force Read/Pause/Write
#define UBA_MAP_EN16   0004000000000LL // Disable upper 2 bits on xfers
#define UBA_MAP_FME    0002000000000LL // Fast Mode Enable
#define UBA_MAP_VALID  0001000000000LL // Page is valid
#define UBA_MAP_RPV    0000400000000LL // Paging RAM Parity - valid
#define UBA_MAP_PAGE   0000003777000LL // Page Number

// I/O Address 763100 - Status Register

#define UBA_SR_TIM    0400000 // (R/C) Unibus Arbitrator Timeout
#define UBA_SR_BAD    0200000 // (R/C) Bad memory data on NPR transfer
#define UBA_SR_PAR    0100000 // (R/C) KS10 Bus Parity Error
#define UBA_SR_NED    0040000 // (R/C) CPU Addressed - Non-existant device
#define UBA_SR_INTH   0004000 // (R)   Interrupt Request on BR6/BR7
#define UBA_SR_INTL   0002000 // (R)   Interrupt Request on BR4/BR5
#define UBA_SR_INT    0006000 // (R)   Interrupt Requests on BR4-BR7
#define UBA_SR_PWRL   0001000 // (R)   AC/DC Low - Clear on write access
#define UBA_SR_DXFR   0000200 // (R/W) Disable transfer on bad data
#define UBA_SR_UINIT  0000100 // (W)   Issue Unibus Initization
#define UBA_SR_PIH    0000070 // (R/W) PI Level of BR6,BR7
#define UBA_SR_PIL    0000007 // (R/W) PI Level of BR4,BR5

#define UBA_SR_CLMASK 0740000 // (C)   Clear Mask
#define UBA_SR_WRMASK 0000377 // (W)   Write Mask
#define UBA_SR_RDMASK 0747277 // (R)   Read Mask

// I/O Address 763101 - Maintenance Register

#define UBA_MR_SMB    0000002 // (W)   Spare Maintenance Bit
#define UBA_MR_CNA    0000001 // (W)   Change NPR Address

#define UBA_MR_WRMASK 0000003 // (W)   Write Mask
#define UBA_MR_RDMASK 0000000 // (R)   Read Mask

// PDP-11 Style Interrupt System

#define UBINT_BR7  0x000000FF // IPL <7:0>
#define UBINT_BR6  0x0000FF00 // IPL <15:8>
#define UBINT_BR5  0x00FF0000 // IPL <23:16>
#define UBINT_BR4  0x3F000000 // IPL <30:24>
#define UBINT_BR76 (UBINT_BR7|UBINT_BR6)
#define UBINT_BR54 (UBINT_BR5|UBINT_BR4)

// Definitions for UBA Map and Configure routine
#define UBA_CHECK     NULL
#define UBA_DELETE    -1
#define UBA_INTERNAL  -1

#define EMU_UBA_BADADDR 1
#define EMU_UBA_FULL    2

// UBA I/O Map definition - Starting 760000
typedef struct {
	UNIT  *pUnit;
	int32 (*ReadIO)(UNIT *, int32);
	void  (*WriteIO)(UNIT *, int32, int32);
} UBAMAP;

// UBA Unit definition
typedef struct {
	UBAMAP IOPage[4096]; // I/O Page Map -         76xxxx
	int32  Map[0100];    // Map registers -        7630xx
	int18  sr;           // Status Register -      763100
	int18  mr;           // Maintenance Register - 763101
	int32  inth_Vector;  // High Interrupt - Vector
	int32  inth_Channel; // High Interrupt - Channel
	int32  intl_Vector;  // Low Interrupt - Vector
	int32  intl_Channel; // Low Interrupt - Channel
} UBAUNIT;

// Unibus Interface definition
typedef struct {
	int     nUnits;     // Number of UBA units;
	UNIT    *ubaUnits;  // Array of UBA units;
	uint32  intReq;     // Interrupt Requests Pending (IPL)
	uint32  intVec[32]; // Listing of Interrupt Vector Map
} UBADEVICE;
