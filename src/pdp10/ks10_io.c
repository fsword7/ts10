// ks10_io.c - KS10 Processor: I/O Routines
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

#include "pdp10/defs.h"
#include "pdp10/ks10.h"

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

//*********************************************************
//***************** DEC I/O Instructions ******************
//*********************************************************

// Opcode 700 series for DEC microcode
void (*KS10_io700dec[020])() =
{
	// APR Device Code (Arithmetic Processor System)
	p10_ksOpcode_APRID,  // Read APR Idenification
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	p10_ksOpcode_WRAPR,  // Write APR Register
	p10_ksOpcode_RDAPR,  // Read APR Register
	p10_ksOpcode_CZAPR,  // CONSZ APR,
	p10_ksOpcode_COAPR,  // CONSO APR,

	// PI Device Code (Priority Interrupt System)
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	p10_ksOpcode_WRPI,   // Write PI Register
	p10_ksOpcode_RDPI,   // Read PI Register
	p10_ksOpcode_CZPI,   // CONSZ PI,
	p10_ksOpcode_COPI    // CONSO PI,
};

// Opcode 701 series for DEC microcode
void (*KS10_io701dec[020])() =
{
	// PAG Device Code (Paging System)
	KS10_Opcode_UUO,
	p10_ksOpcode_RDUBR,   // Read User Base Register
	p10_ksOpcode_CLRPT,   // Clear Page Table (Cache Table)
	p10_ksOpcode_WRUBR,   // Write User Base Register
	p10_ksOpcode_WREBR,   // Write Executive Base Register
	p10_ksOpcode_RDEBR,   // Read Executive Base Register
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,

	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO
};

// Opcode 702 series for DEC microcode
void (*KS10_io702dec[020])() =
{
	p10_ksOpcode_RDSPB,  // Read Section Pointer Base Register
	p10_ksOpcode_RDCSB,  // Read Core Status Base Register
	p10_ksOpcode_RDPUR,  // Read Process-Use Register
	p10_ksOpcode_RDCSTM, // Read Core Status Mask Register
	p10_ksOpcode_RDTIM,  // Read Time Base Register
	p10_ksOpcode_RDINT,  // Read Interval Time Register
	p10_ksOpcode_RDHSB,  // Read Halt Status Base Register
	KS10_Opcode_UUO,

	p10_ksOpcode_WRSPB,  // Write Section Pointer Base Register
	p10_ksOpcode_WRCSB,  // Write Core Status Base Register
	p10_ksOpcode_WRPUR,  // Write Process-Use Register
	p10_ksOpcode_WRCSTM, // Write Core Status Mask Register
	p10_ksOpcode_WRTIM,  // Write Time Base Register
	p10_ksOpcode_WRINT,  // Write Interval Time Register
	p10_ksOpcode_WRHSB,  // Write Halt Status Base Register
	KS10_Opcode_UUO,
};

// Opcode 700 Series
void KS10_Opcode_IO700(void)
{
	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0)) {
		KS10_Opcode_UUO();
		return;
	}

	KS10_io700dec[opAC]();
}

// Opcode 701 Series
void KS10_Opcode_IO701(void)
{
	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0)) {
		KS10_Opcode_UUO();
		return;
	}

	KS10_io701dec[opAC]();
}

// Opcode 702 Series
void KS10_Opcode_IO702(void)
{
	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0)) {
		KS10_Opcode_UUO();
		return;
	}

	KS10_io702dec[opAC]();
}

// Calculate for effective I/O address
int36 KS10_GetIOAddr(int36 data)
{
	int32 indirect;
	int32 index;
	int32 eAddr;
	int36 XR; /* the contents of index register X */

	indirect = INST_GETI(data);
	index    = INST_GETX(data);
	eAddr    = INST_GETY(data);

	if (index) {
		XR = curAC[index];
		if (XR < 0)
			eAddr = VMA(eAddr + RH(XR));
		else
			eAddr = (eAddr + XR) & IOA_MASK;
	}

	if (indirect)
		eAddr = p10_vRead(VMA(eAddr), NOPXCT);

	return eAddr & WORD36_ONES;
}

// 710 TIOE - Test IO Equal
void KS10_Opcode_TIOE(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	if ((ks10uba_ReadIO(uba, eAddr, PTF_IOWORD) & curAC[opAC]) == 0)
		DO_SKIP;
}

// 711 TION - Test IO Not Equal
void KS10_Opcode_TION(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	if ((ks10uba_ReadIO(uba, eAddr, PTF_IOWORD) & curAC[opAC]) != 0)
		DO_SKIP;
}

// 712 RDIO - Read IO
void KS10_Opcode_RDIO(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	curAC[opAC] = ks10uba_ReadIO(uba, eAddr, PTF_IOWORD);
}

// 713 WRIO - Write IO
void KS10_Opcode_WRIO(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	ks10uba_WriteIO(uba, eAddr, curAC[opAC], PTF_IOWORD);
}

// 714 BSIO - Bit Set IO
void KS10_Opcode_BSIO(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	AR = ks10uba_ReadIO(uba, eAddr, PTF_IOWORD) | curAC[opAC];
	ks10uba_WriteIO(uba, eAddr, AR, PTF_IOWORD);
}

// 715 BCIO - Bit Clear IO
void KS10_Opcode_BCIO(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	AR = ks10uba_ReadIO(uba, eAddr, PTF_IOWORD) & ~curAC[opAC];
	ks10uba_WriteIO(uba, eAddr, AR, PTF_IOWORD);
}

// 720 TIOEB - Test IO Equal, Byte
void KS10_Opcode_TIOEB(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	if (((ks10uba_ReadIO(uba, eAddr, PTF_IOBYTE) & curAC[opAC]) & 0377) == 0)
		DO_SKIP;
}

// 721 TIONB - Test IO Not Equal, Byte
void KS10_Opcode_TIONB(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	if (((ks10uba_ReadIO(uba, eAddr, PTF_IOBYTE) & curAC[opAC]) & 0377) != 0)
		DO_SKIP;
}

// 722 RDIOB - Read IO Byte
void KS10_Opcode_RDIOB(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	curAC[opAC] = ks10uba_ReadIO(uba, eAddr, PTF_IOBYTE) & 0377;
}

// 723 WRIOB - Write IO Byte
void KS10_Opcode_WRIOB(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	ks10uba_WriteIO(uba, eAddr, curAC[opAC] & 0377, PTF_IOBYTE);
}

// 724 BSIOB - Bit Set IO Byte
void KS10_Opcode_BSIOB(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	AR = ks10uba_ReadIO(uba, eAddr, PTF_IOBYTE) | (curAC[opAC] & 0377);
	ks10uba_WriteIO(uba, eAddr, AR, PTF_IOBYTE);
}

// 725 BCIOB - Bit Clear IO Byte
void KS10_Opcode_BCIOB(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	eAddr = KS10_GetIOAddr(HR);
	AR = ks10uba_ReadIO(uba, eAddr, PTF_IOBYTE) & ~(curAC[opAC] & 0377);
	ks10uba_WriteIO(uba, eAddr, AR, PTF_IOBYTE);
}

#ifdef OPT_ITS

//*********************************************************
//***************** ITS I/O Instructions ******************
//*********************************************************

// Opcode 700 series for ITS microcode
void (*KS10_io700its[020])() =
{
	// APR Device Code (Arithmetic Processor System)
	p10_ksOpcode_APRID,  // Read APR Idenification
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	p10_ksOpcode_WRAPR,  // Write APR Register
	p10_ksOpcode_RDAPR,  // Read APR Register
	p10_ksOpcode_CZAPR,  // CONSZ APR,
	p10_ksOpcode_COAPR,  // CONSO APR,

	// PI Device Code (Priority Interrupt System)
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	p10_ksOpcode_WRPI,   // Write PI Register
	p10_ksOpcode_RDPI,   // Read PI Register
	p10_ksOpcode_CZPI,   // CONSZ PI,
	p10_ksOpcode_COPI    // CONSO PI,
};

// Opcode 701 series for ITS microcode
void (*KS10_io701its[020])() =
{
	// PAG Device Code (Paging System)
	KS10_Opcode_UUO,
	p10_ksOpcode_RDUBR,   // Read User Base Register
	p10_ksOpcode_CLRPT,   // Clear Page Table (Cache Table)
	p10_ksOpcode_WRUBR,   // Write User Base Register
	p10_ksOpcode_WREBR,   // Write Executive Base Register
	p10_ksOpcode_RDEBR,   // Read Executive Base Register
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,

	KS10_Opcode_UUO,
	ks10_Opcode_RDPCST,
	KS10_Opcode_UUO,
	ks10_Opcode_WRPCST,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO,
	KS10_Opcode_UUO
};

// Opcode 702 series for ITS microcode
void (*KS10_io702its[020])() =
{
	ks10_Opcode_SDBR1,   //
	ks10_Opcode_SDBR2,   //
	ks10_Opcode_SDBR3,   //
	ks10_Opcode_SDBR4,   // 
	p10_ksOpcode_RDTIM,  // Read Time Base Register
	p10_ksOpcode_RDINT,  // Read Interval Time Register
	p10_ksOpcode_RDHSB,  // Read Halt Status Base Register
	ks10_Opcode_SPM,

	ks10_Opcode_LDBR1,   //
	ks10_Opcode_LDBR2,   //
	ks10_Opcode_LDBR3,   //
	ks10_Opcode_LDBR4,   //
	p10_ksOpcode_WRTIM,  // Write Time Base Register
	p10_ksOpcode_WRINT,  // Write Interval Time Register
	p10_ksOpcode_WRHSB,  // Write Halt Status Base Register
	ks10_Opcode_LPMR,
};

// Opcode 700 Series
void KS10_Opcode_IO700its(void)
{
	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0)) {
		KS10_Opcode_UUO();
		return;
	}

	KS10_io700its[opAC]();
}

// Opcode 701 Series
void KS10_Opcode_IO701its(void)
{
	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0)) {
		KS10_Opcode_UUO();
		return;
	}

	KS10_io701its[opAC]();
}

// Opcode 702 Series
void KS10_Opcode_IO702its(void)
{
	if ((FLAGS & FLG_USER) && ((FLAGS & FLG_USERIO) == 0)) {
		KS10_Opcode_UUO();
		return;
	}

	KS10_io702its[opAC]();
}

// IORDI - Read Word from Unibus 3
void KS10_Opcode_IORDI(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	curAC[opAC] = ks10uba_ReadIO(uba, IO_UBA3 | eAddr, PTF_IOWORD);
}

// IORDQ - Read Word from Unibus 1
void KS10_Opcode_IORDQ(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	curAC[opAC] = ks10uba_ReadIO(uba, IO_UBA1 | eAddr, PTF_IOWORD);
}

// IORD - Read I/O Word, I/O Address in M[ea]
void KS10_Opcode_IORD(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;
	int dataMode = p10_CheckPXCT(PXCT_DATA);

	eAddr = p10_Read(eAddr, dataMode);
	curAC[opAC] = ks10uba_ReadIO(uba, eAddr, PTF_IOWORD);
}

// IOWR - Write I/O Word, I/O Address in M[ea]
void KS10_Opcode_IOWR(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;
	int dataMode = p10_CheckPXCT(PXCT_DATA);
	
	eAddr = p10_Read(eAddr, dataMode);
	ks10uba_WriteIO(uba, eAddr, curAC[opAC], PTF_IOWORD);
}

// IOWRI - Write Word to Unibus 3
void KS10_Opcode_IOWRI(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	ks10uba_WriteIO(uba, IO_UBA3 | eAddr, curAC[opAC], PTF_IOWORD);
}

// IOWRQ - Write Word to Unibus 1
void KS10_Opcode_IOWRQ(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	ks10uba_WriteIO(uba, IO_UBA1 | eAddr, curAC[opAC], PTF_IOWORD);
}

// IORDBI - Read Byte from Unibus 3
void KS10_Opcode-IORDBI(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	curAC[opAC] = ks10uba_ReadIO(uba, IO_UBA3 | eAddr, PTF_IOBYTE) & 0377;
}

// IORDBQ - Read Byte from Unibus 1
void KS10_Opcode_IORDBQ(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	curAC[opAC] = ks10uba_ReadIO(uba, IO_UBA1 | eAddr, PTF_IOBYTE) & 0377;
}

// IORDB - Read I/O Byte, I/O Address in M[ea]
void KS10_Opcode_IORDB(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;
	int dataMode = p10_CheckPXCT(PXCT_DATA);

	eAddr = p10_Read(eAddr, dataMode);
	curAC[opAC] = ks10uba_ReadIO(uba, eAddr, PTF_IOBYTE) & 0377;
}

// IOWRB - Write I/O Byte, I/O Address in M[ea]
void KS10_Opcode_IOWRB(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;
	int dataMode = p10_CheckPXCT(PXCT_DATA);

	eAddr = p10_Read(eAddr, dataMode);
	ks10uba_WriteIO(uba, eAddr, curAC[opAC] & 0377, PTF_IOBYTE);
}

// IOWRBI - Write Byte to Unibus 3
void KS10_Opcode_IOWRBI(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	ks10uba_WriteIO(uba, IO_UBA3 | eAddr, curAC[opAC] & 0377, PTF_IOBYTE);
}

// IOWRBQ - Write Byte to Unibus 1
void KS10_Opcode_IOWRBQ(void)
{
	KS10UBA_DEVICE *uba = ((KS10_CPU *)p10)->uba;

	ks10uba_WriteIO(uba, IO_UBA1 | eAddr, curAC[opAC] & 0377, PTF_IOBYTE);
}

#endif /* OPT_ITS */

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

void KS10_bltuCleanup(void)
{
	int36 ac;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		dbg_Printf("BLTU: *** Page Fail Trap at PC %06o\n", PC);
		dbg_Printf("BLTU: Current SA=%06o DA=%06o\n", srcAddr, dstAddr);
	}
#endif /* DEBUG */

	// On Page Fail Trap, save current source and destination
	// pointers for restarted instruction.
	ac = XWD((int36)srcAddr, (int36)dstAddr);
	curAC[opAC] = SXT36(ac);
}

void KS10_BlockTransfer(int dir)
{
	srcAddr = LHSR(curAC[opAC]);
	dstAddr = RH(curAC[opAC]);
	endAddr = RH(eAddr);
	lenAddr = endAddr - dstAddr + 1;

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA))
		dbg_Printf("BLTU: Transfer SA=%06o to DA=%06o at %o Words.\n",
			srcAddr, dstAddr, (lenAddr < 1) ? 1 : lenAddr);
#endif /* DEBUG */

	// Must update SA/DA on AC first before start transfer
	// for KS10 Processor.
	curAC[opAC] = XWD((int36)(srcAddr + lenAddr), (int36)(dstAddr + lenAddr));
	curAC[opAC] = SXT36(curAC[opAC]);

	// Get previous context modes.
	srcMode = p10_CheckPXCT(PXCT_BLT_SRC);
	dstMode = p10_CheckPXCT(PXCT_BLT_DST);

	pager_Cleanup = KS10_bltuCleanup;

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
#endif /* DEBUG */
}

// 716 BLTBU - Block Transfer from Byte to Unibus
void KS10_Opcode_BLTBU(void)
{
	KS10_BlockTransfer(TRUE);
}


// 717 BLTUB - Block Transfer from Unibus to Byte
void KS10_Opcode_BLTUB(void)
{
	KS10_BlockTransfer(FALSE);
}
