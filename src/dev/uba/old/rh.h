// rh.h - RH11 Controller Definitions
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

/* RH11 Controller - Registers */

/* Unibus Address Index */
#define RHCS1 (000 >> 1) /* (R/W) ^Control and Status 1 Register */
#define RHWC  (002 >> 1) /* (R/W)  Word Count Register */
#define RHBA  (004 >> 1) /* (R/W)  Bus Address Register */
#define RHCS2 (010 >> 1) /* (R/W)  Control and Status 2 Register */
#define RHAS  (016 >> 1) /* (R/W) *Attention Summary Pseudo-Register */
#define RHDB  (022 >> 1) /* (R/W)  Data Buffer Register */

/* RHCS1 - Control and Status 1 Register */
#define RHCS1_SC    0100000 /* (R)    Special Condition */
#define RHCS1_TRE   0040000 /* (R/C)  Transfer Error */
#define RHCS1_MCPE  0020000 /* (R)    Mass I/O Bus Control Parity Error */
#define RHCS1_X     0010000 /* (R)   *Reserved for use by the drive */
#define RHCS1_DVA   0004000 /* (R)   *Drive Available */
#define RHCS1_A17   0001000 /* (R/W)  A17 Bus Address Extension Bit */
#define RHCS1_A16   0000400 /* (R/W)  A16 Bus Address Extension Bit */
#define RHCS1_RDY   0000200 /* (R)    Ready */
#define RHCS1_IE    0000100 /* (R/W)  Interrupt Enable */
#define RHCS1_FUNC  0000076 /* (R/W) *Function Code (F0:F4) */
#define RHCS1_GO    0000001 /* (R/W) *Go */

#define RHCS1_BAE   0001400 /* (R/W)  A16/A17 BAE bits */

#define RHCS1_DRV   0014077 /* (R/W)  Drive Mask */
#define RHCS1_CTLR  0163700 /* (R/W)  Controller Mask */
#define RHCS1_MBZ   0012000 /*        Must Be Zeros */

/* RHWC - Word Count Register */
#define RHWC_MASK   0177777 /* (R/W)  Word Count */

/* RHBA - Bus Address Register */
#define RHBA_MASK   0177776 /* (R/W)  Bus Address */
#define RHBA_MBZ    0000001 /*        Must be Zeros */

/* RHDB - Data Buffer Register */
#define RHDB_MASK   0177777 /* (R/W)  Data Buffer */

/* RHCS2 - Control and Status 2 Register */
#define RHCS2_DLT   0100000 /* (R)    Data Late */
#define RHCS2_WCE   0040000 /* (R)    Write Check Error */
#define RHCS2_PE    0020000 /* (R)    Parity Error */
#define RHCS2_NED   0010000 /* (R)    Non-Existent Drive */
#define RHCS2_NEM   0004000 /* (R)    Non-Existent Memory */
#define RHCS2_PGE   0002000 /* (R)    Program Error */
#define RHCS2_MXF   0001000 /* (R)    Missed Transfer */
#define RHCS2_MDPE  0000400 /* (R)    Mass I/O Bus Data Parity Error */
#define RHCS2_OR    0000200 /* (R)    Output Ready */
#define RHCS2_IR    0000100 /* (R)    Input Ready */
#define RHCS2_CLR   0000040 /* (W)    Controller Clear */
#define RHCS2_PAT   0000020 /* (R/W)  Parity Test */
#define RHCS2_BAI   0000010 /* (R/W)  Unibus Address Increment Inhibit */
#define RHCS2_UNIT  0000007 /* (R/W)  Unit Select (U0:U2) */

#define RHCS2_ERR   0177400 /*        Set TRE on CS1 if any errors */
#define RHCS2_RD    0177737 /* (R)    Read Mask */
#define RHCS2_WR    0000077 /* (W)    Write Mask */

/* RHAS - Attention Summary Pseudo-Register */
#define RHAS_ATA7   0000200 /* (R/C)  Attention Active #7 */
#define RHAS_ATA6   0000100 /* (R/C)  Attention Active #6 */
#define RHAS_ATA5   0000040 /* (R/C)  Attention Active #5 */
#define RHAS_ATA4   0000020 /* (R/C)  Attention Active #4 */
#define RHAS_ATA3   0000010 /* (R/C)  Attention Active #3 */
#define RHAS_ATA2   0000004 /* (R/C)  Attention Active #2 */
#define RHAS_ATA1   0000002 /* (R/C)  Attention Active #1 */
#define RHAS_ATA0   0000001 /* (R/C)  Attention Active #0 */

#define RHAS_MASK   0000377 /* (R/C)  Attention Summary (ATA0:ATA7) */

/* User-definable flags for UNIT variables */
#define UNIT_18B     0002000  /* 18-Bit Data Width */
#define UNIT_WRLOCK  0001000  /* Write Lock */
#define UNIT_DTYPE   0000777  /* Drive Type */

/* RH Flags */
#define RH11_CTLR    0  /* Controller */
#define RH11_TAPE    1  /* Tape drive */
#define RH11_PACK    2  /* Pack drive */
#define RH11_FIXED   3  /* Fixed-disk drive */

#define RH_MAXUNITS 8  /* Up to 8 MASSBUS drives per controller */

/* RH11 Controller Unit */
typedef struct {
	int32  Controller; // Unibus: Unit #
	int32  Address;    // Unibus: Address
	int32  Mask;       // Unibus: Address Mask

	int32  IntIPL;     // Interrupt: IPL Level
	int32  IntVec;     // Interrupt: Vector Address

	// RH-based controller registers
	uint16 rhcs1;      // RH Control and Status Register #1
	uint16 rhwc;       // RH Word Counter Register
	uint16 rhba;       // RH Bus Address Register
	uint16 rhcs2;      // RH Control and Status Register #2
	uint16 rhas;       // RH Attention Summary Register
	uint16 rhdb;       // RH Data Buffer Register
} RH11UNIT;

/* RH Functions */
typedef struct {
	void (*Initialize)();
	void (*Reset)();
	int  (*ReadIO)();
	int  (*WriteIO)();
	int  (*Process)();
} RHFUNCS;

#include "dev/func.h"
