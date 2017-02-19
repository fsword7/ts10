// cpu_ksio.c - KS10 Processor: I/O Routines
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
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

#include "pdp10/defs.h"
#include "pdp10/proto.h"

//             DEC Microcode              ITS Microcode
//  AC#    700      701      702      700      701      702
// +----+--------+--------+--------+--------+--------+--------+
// | 00 | APRID  |        | RDSPB  | APRID  |        | SDBR1  |
// | 01 |        | RDUBR  | RDCSB  |        | RDUBR  | SDBR2  |
// | 02 |        | CLRPT  | RDPUR  |        | CLRPT  | SBBR3  |
// | 03 |        | WRUBR  | RDCSTM |        | WRUBR  | SDBR4  |
// | 04 | WRAPR  | WREBR  | RDTIM  | WRAPR  | WREBR  | RDTIM  |
// | 05 | RDAPR  | RDEBR  | RDINT  | RDAPR  | RDEBR  | RDINT  |
// | 06 | CZAPR  |        | RDHSB  | CZAPR  |        | RDHSB  |
// | 07 | COAPR  |        |        | COAPR  |        | SPM    |
// | 10 |        |        | WRSPB  |        |        | LDBR1  |
// | 11 |        |        | WRCSB  |        | RDPCST | LDBR2  |
// | 12 |        |        | WRPUR  |        |        | LDBR3  |
// | 13 |        |        | WRCSTM |        | WRPCST | LDBR4  |
// | 14 | WRPI   |        | WRTIM  | WRPI   |        | WRTIM  |
// | 15 | RDPI   |        | WRINT  | RDPI   |        | WRINT  |
// | 16 | CZPI   |        | WRHSB  | CZPI   |        | WRHSB  |
// | 17 | COPI   |        |        | COPI   |        | LPMR   |
// +----+--------+--------+--------+--------+--------+--------+
//
// NOTE: ITS microcode is not implemented yet.

// Opcode 700 series for DEC microcode
void (*p10_io700dec[020])() =
{
	// APR Device Code (Arithmetic Processor System)
	p10_ksOpcode_APRID,  // Read APR Idenification
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_WRAPR,  // Write APR Register
	p10_ksOpcode_RDAPR,  // Read APR Register
	p10_ksOpcode_CZAPR,  // CONSZ APR,
	p10_ksOpcode_COAPR,  // CONSO APR,

	// PI Device Code (Priority Interrupt System)
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_WRPI,   // Write PI Register
	p10_ksOpcode_RDPI,   // Read PI Register
	p10_ksOpcode_CZPI,   // CONSZ PI,
	p10_ksOpcode_COPI    // CONSO PI,
};

// Opcode 701 series for DEC microcode
void (*p10_io701dec[020])() =
{
	// PAG Device Code (Paging System)
	p10_ksOpcode_UUO,
	p10_ksOpcode_RDUBR,   // Read User Base Register
	p10_ksOpcode_CLRPT,   // Clear Page Table (Cache Table)
	p10_ksOpcode_WRUBR,   // Write User Base Register
	p10_ksOpcode_WREBR,   // Write Executive Base Register
	p10_ksOpcode_RDEBR,   // Read Executive Base Register
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,

	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO,
	p10_ksOpcode_UUO
};

// Opcode 702 series for DEC microcode
void (*p10_io702dec[020])() =
{
	p10_ksOpcode_RDSPB,  // Read Section Pointer Base Register
	p10_ksOpcode_RDCSB,  // Read Core Status Base Register
	p10_ksOpcode_RDPUR,  // Read Process-Use Register
	p10_ksOpcode_RDCSTM, // Read Core Status Mask Register
	p10_ksOpcode_RDTIM,  // Read Time Base Register
	p10_ksOpcode_RDINT,  // Read Interval Time Register
	p10_ksOpcode_RDHSB,  // Read Halt Status Base Register
	p10_ksOpcode_UUO,

	p10_ksOpcode_WRSPB,  // Write Section Pointer Base Register
	p10_ksOpcode_WRCSB,  // Write Core Status Base Register
	p10_ksOpcode_WRPUR,  // Write Process-Use Register
	p10_ksOpcode_WRCSTM, // Write Core Status Mask Register
	p10_ksOpcode_WRTIM,  // Write Time Base Register
	p10_ksOpcode_WRINT,  // Write Interval Time Register
	p10_ksOpcode_WRHSB,  // Write Halt Status Base Register
	p10_ksOpcode_UUO,
};

UNIT *p10_ioUnits;

// Opcode 700 Series
void p10_ksOpcode_IO700(void)
{
	if ((FLAGS & PC_USER) && ((FLAGS & PC_USERIO) == 0)) {
		p10_ksOpcode_UUO();
		return;
	}

	p10_io700dec[opAC]();
}

// Opcode 701 Series
void p10_ksOpcode_IO701(void)
{
	if ((FLAGS & PC_USER) && ((FLAGS & PC_USERIO) == 0)) {
		p10_ksOpcode_UUO();
		return;
	}

	p10_io701dec[opAC]();
}

// Opcode 702 Series
void p10_ksOpcode_IO702(void)
{
	if ((FLAGS & PC_USER) && ((FLAGS & PC_USERIO) == 0)) {
		p10_ksOpcode_UUO();
		return;
	}

	p10_io702dec[opAC]();
}


// Calculate for effective I/O address
int36 p10_ksGetIOAddr(int36 data)
{
	int36 indirect;
	int36 index;
	int36 eAddr;
	int36 XR; /* the contents of index register X */

	indirect = INST_GETI(data);
	index    = INST_GETX(data);
	eAddr    = INST_GETY(data);

	if (index) {
		mem_vRead36(index, &XR, NOPXCT);
		if (XR < 0)
			eAddr = (eAddr + RH(XR)) & VMA_MASK;
		else
			eAddr = (eAddr + XR) & IOA_MASK;
	}

	if (indirect)
		mem_vRead36(eAddr & VMA_MASK, &eAddr, NOPXCT);

	return eAddr & WORD36_ONES;
}

// 710 TIOE - Test IO Equal
void p10_ksOpcode_TIOE(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	if ((p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOWORD) & curAC[opAC]) == 0)
		DO_SKIP;
}

// 711 TION - Test IO Not Equal
void p10_ksOpcode_TION(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	if ((p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOWORD) & curAC[opAC]) != 0)
		DO_SKIP;
}

// 712 RDIO - Read IO
void p10_ksOpcode_RDIO(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	curAC[opAC] = p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOWORD);
}

// 713 WRIO - Write IO
void p10_ksOpcode_WRIO(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	p10_ubaWriteIO(p10_ioUnits, eAddr, curAC[opAC], PTF_IOWORD);
}

// 714 BSIO - Bit Set IO
void p10_ksOpcode_BSIO(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	AR = p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOWORD) | curAC[opAC];
	p10_ubaWriteIO(p10_ioUnits, eAddr, AR, PTF_IOWORD);
}

// 715 BCIO - Bit Clear IO
void p10_ksOpcode_BCIO(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	AR = p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOWORD) & ~curAC[opAC];
	p10_ubaWriteIO(p10_ioUnits, eAddr, AR, PTF_IOWORD);
}

// 720 TIOEB - Test IO Equal, Byte
void p10_ksOpcode_TIOEB(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	if (((p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOBYTE) & curAC[opAC]) & 0377) == 0)
		DO_SKIP;
}

// 721 TIONB - Test IO Not Equal, Byte
void p10_ksOpcode_TIONB(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	if (((p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOBYTE) & curAC[opAC]) & 0377) != 0)
		DO_SKIP;
}

// 722 RDIOB - Read IO Byte
void p10_ksOpcode_RDIOB(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	curAC[opAC] = p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOBYTE) & 0377;
}

// 723 WRIOB - Write IO Byte
void p10_ksOpcode_WRIOB(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	p10_ubaWriteIO(p10_ioUnits, eAddr, curAC[opAC] & 0377, PTF_IOBYTE);
}

// 724 BSIOB - Bit Set IO Byte
void p10_ksOpcode_BSIOB(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	AR = p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOBYTE) | (curAC[opAC] & 0377);
	p10_ubaWriteIO(p10_ioUnits, eAddr, AR, PTF_IOBYTE);
}

// 725 BCIOB - Bit Clear IO Byte
void p10_ksOpcode_BCIOB(void)
{
	eAddr = p10_ksGetIOAddr(HR);
	AR = p10_ubaReadIO(p10_ioUnits, eAddr, PTF_IOBYTE) & ~(curAC[opAC] & 0377);
	p10_ubaWriteIO(p10_ioUnits, eAddr, AR, PTF_IOBYTE);
}

//*********************************************************
//************** Block Transfer Instructions **************
//*********************************************************

// Packing and Unpacking bytes to/from Unibus format
//
// Byte Format
//
// |<---------- Left Halfword -------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     Byte 0    |     Byte 1    |     Byte 2    |     Byte 3    |0|0|0|0|
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Unibus Format
//
// |<---------- Left Halfword -------->|<-------- Right Halfword --------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |0|0|     Byte 1    |     Byte 0    |0|0|     Byte 3    |     Byte 2    |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5

// BLTU - Byte Format Definitions
#define BLTB_BYTE0 0776000000000LL
#define BLTB_BYTE1 0001774000000LL
#define BLTB_BYTE2 0000003770000LL
#define BLTB_BYTE3 0000000007760LL

// BLTU - Unibus Format Definitions
#define BLTU_BYTE0 0177400000000LL
#define BLTU_BYTE1 0000377000000LL
#define BLTU_BYTE2 0000000177400LL
#define BLTU_BYTE3 0000000000377LL

static int30 srcAddr, dstAddr;
static int30 endAddr, lenAddr;
static int36 *srcData, *dstData;

void p10_bltuCleanup(void)
{
	int36 ac;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("BLTU: *** Page Fail Trap at PC %06o\n", PC);
		dbg_Printf("BLTU: Current SA=%06o DA=%06o\n", srcAddr, dstAddr);
	}
#endif DEBUG

	// On Page Fail Trap, save current source and destination
	// pointers for restarted instruction.
	ac = XWD((int36)srcAddr, (int36)dstAddr);
	curAC[opAC] = SXT36(ac);
}

inline void p10_BlockTransfer(boolean dir)
{
	srcAddr = LHSR(curAC[opAC]);
	dstAddr = RH(curAC[opAC]);
	endAddr = RH(eAddr);
	lenAddr = endAddr - dstAddr + 1;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BLTU: Transfer SA=%06o to DA=%06o at %o Words.\n",
			srcAddr, dstAddr, (lenAddr < 1) ? 1 : lenAddr);
#endif DEBUG

	// Must update SA/DA on AC first before start transfer
	// for KS10 Processor.
	curAC[opAC] = XWD((int36)(srcAddr + lenAddr), (int36)(dstAddr + lenAddr));
	curAC[opAC] = SXT36(curAC[opAC]);

	// Get previous context modes.
	srcMode = p10_CheckPXCT(PXCT_BLT_SRC);
	dstMode = p10_CheckPXCT(PXCT_BLT_DST);

	pager_Cleanup = p10_bltuCleanup;

	do {
		dstData = p10_Access(dstAddr, dstMode | PTF_WRITE);
		srcData = p10_Access(srcAddr, srcMode);

		// Convert to byte or unibus format
		if (dir) {
			// Byte to Unibus Convert
			*dstData = ((*srcData >> 10) & BLTU_BYTE0) |
			           ((*srcData << 6)  & BLTU_BYTE1) |
			           ((*srcData >> 12) & BLTU_BYTE2) |
			           ((*srcData << 4)  & BLTU_BYTE3);
		} else {
			// Unibus to Byte Convert
			*dstData = ((*srcData << 10) & BLTB_BYTE0) |
			           ((*srcData >> 6)  & BLTB_BYTE1) |
			           ((*srcData << 12) & BLTB_BYTE2) |
			           ((*srcData >> 4)  & BLTB_BYTE3);
			*dstData = SXT36(*dstData);
		}

		// Increase source and destination addresses.
		srcAddr++;
		dstAddr++;
	} while (dstAddr <= endAddr);

	pager_Cleanup = NULL;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BLTU: Final SA=%06o DA=%06o\n", srcAddr, dstAddr);
#endif DEBUG
}

// 716 BLTBU - Block Transfer from Byte to Unibus
void p10_ksOpcode_BLTBU(void)
{
	p10_BlockTransfer(TRUE);
}


// 717 BLTUB - Block Transfer from Unibus to Byte
void p10_ksOpcode_BLTUB(void)
{
	p10_BlockTransfer(FALSE);
}
