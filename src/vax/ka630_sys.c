// ka630.c - KA630 Bus System Configuration (MicroVAX II series)
//
// Copyright (c) 2002, Timothy M. Stark
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

// KA630 - MicroVAX II Memory Map
//
// 00000000 +-------------------------------------+
//          |           Main Memory               |
//          |- - - - - - - - - - - - - - - - - - -|
//          |        Up to 16 MB Memory           |
// 01000000 +-------------------------------------+
//          |             Reserved                |
// 20000000 +-------------------------------------+
//          |         Q22-Bus I/O space           |
// 20002000 +-------------------------------------+
//          |             Reserved                |
// 20040000 +-------------------------------------+
//          |               ROM                   |
// 20080000 +-------------------------------------+
//          |      Local I/O Register Area        |
// 200C0000 +-------------------------------------+
//          |             Reserved                |
// 30000000 +-------------------------------------+
//          |        Q22-Bus Memory Space         |
// 303FFFFF +-------------------------------------+
//          |             Reserved                |
// 3FFFFFFF +-------------------------------------+
//
// Local Memory Area:
//
//   Main Memory       - 00000000-00FFFFFF (up to 16 MB memory)
//
// Local I/O Register Area:
//
//   CPU Registers     - 20080000-2008000F (4 16-bit CPU Registers)
//   Clock Registers   - 200B8000-200B807F (64 8-bit Clock Registers)
//
//   Q22 I/O Page      - 20000000-20001FFF (8 KB I/O Space)
//   Q22 Map Registers - 20088000-2008FFFF (8096 32-bit Map Registers)
//   Q22 Memory Space  - 30000000-3003FFFF (4 MB Memory Space)

#include "vax/defs.h"
#include "dev/uba/dec/defs.h"
#include "vax/ka630.h"
#include "vax/mtpr.h"

// **********************************************************
// ***************** Privileged Registers *******************
// **********************************************************

#ifdef DEBUG
static int  regLength = 0x40;
static char *regNames[] = {
	"KSP",     // (R/W) 00 Kernel Stack Pointer
	"ESP",     // (R/W) 01 Executive Stack Pointer
	"SSP",     // (R/W) 02 Supervisor Stack Pointer
	"USP",     // (R/W) 03 User Stack Pointer
	"ISP",     // (R/W) 04 Interrupt Stack Pointer
	NULL,      //       05 Unknown
	NULL,      //       06 Unknown
	NULL,      //       07 Unknown
	"P0BR",    // (R/W) 08 P0 Base Register
	"P0LR",    // (R/W) 09 P0 Length Register
	"P1BR",    // (R/W) 0A P1 Base Register
	"P1LR",    // (R/W) 0B P1 Length Register
	"SBR",     // (R/W) 0C System Base Register
	"SLR",     // (R/W) 0D System Length Register
	NULL,      //       0E Unknown
	NULL,      //       0F Unknown
	"PCBB",    // (R/W) 10 Process Control Block Base 
	"SCBB",    // (R/W) 11 System Control Block Base
	"IPL",     // (R/W) 12 Interrupt Priority Level
	"ASTLVL",  // (R/W) 13 AST Level
	"SIRR",    // (W)   14 Software Interrupt Request
	"SISR",    // (R/W) 15 Software Interrupt Summary
	NULL,      //       16 Unknown
	NULL,      //       17 Unknown
	"ICCS",    // (R/W) 18 Interval Clock Control
	"NICR",    // (W)   19 Next Interval Count
	"ICR",     // (R)   1A Interval Count
	"TODR",    // (R/W) 1B Time of Year
	NULL,      //       1C Unknown
	NULL,      //       1D Unknown
	NULL,      //       1E Unknown
	NULL,      //       1F Unknown
	"RXCS",    // (R/W) 20 Console Receiver Status
	"RXDB",    // (R)   21 Console Receiver Data Buffer
	"TXCS",    // (R/W) 22 Console Transmit Status
	"TXDB",    // (W)   23 Console Transmit Data Buffer
	NULL,      //       24 Unknown
	NULL,      //       25 Unknown
	NULL,      //       26 Unknown
	NULL,      //       27 Unknown
	"ACCS",    //       28 Accelerator Control and Status
	"SAVISP",  //       29 Console Saved ISP (Interrupt Stack Pointer)
	"SAVPC",   //       2A Console Saved PC  (Program Counter)
	"SAVPSL",  //       2B Console Saved PSL (Processor Status Longword)
	NULL,      //       2C Unknown
	NULL,      //       2D Unknown
	NULL,      //       2E Unknown
	NULL,      //       2F Unknown
	"SBIFS",   //       30 SBI Fault Status
	"SBIS",    //       31 SBI silo
	"SBISC",   //       32 SBI silo comparator
	"SBIMT",   //       33 SBI Maintenance
	"SBIER",   //       34 SBI Error
	"SBITA",   //       35 SBI Timeout Address
	"SBIQC",   //       36 SBI Quadword Clear
	"IORESET", //       37 I/O Reset
	"MAPEN",   // (R/W) 38 Memory Management Enable
	"TBIA",    // (W)   39 Translation Buffer Invalidate All
	"TBIS",    // (W)   3A Translation Buffer Invalidate Single
	NULL,      //       3B Unknown
	"MBRK",    //       3C Microprogram Breakpoint
	"PME",     // (R/W) 3D Performance Monitor Enable
	"SID",     // (R)   3E System Identification
	"TBCHK",   // (W)   3F Translation Buffer Check
};
#endif /* DEBUG */

// System ID Longword
//
//  31   24                                          0
// +-------+------------------------------------------+
// |   8   |               Reserved                   |
// +-------+------------------------------------------+

#define KA630_SYSID    0x08000000

// Privileged Register for MicroVAX II system
/*
#define nKSP     0x00 // Kernel Stack Pointer
#define nESP     0x01 // Executive Stack Pointer
#define nSSP     0x02 // Supervisor Stack Pointer
#define nUSP     0x03 // User Stack Pointer
#define nISP     0x04 // Interrupt Stack Pointer

#define nP0BR    0x08 // P0 Base Register
#define nP0LR    0x09 // P0 Length Register
#define nP1BR    0x0A // P1 Base Register
#define nP1LR    0x0B // P1 Length Register
#define nSBR     0x0C // System Base Register
#define nSLR     0x0D // System Length Register

#define nPCBB    0x10 // Process Control Block Base
#define nSCBB    0x11 // System Control Block Base
#define nIPL     0x12 // Interrupt Priority Level
#define nASTLVL  0x13 // AST Level
#define nSIRR    0x14 // Software Interrupt Request
#define nSISR    0x15 // Software Interrupt Summary

#define nICCS    0x18 // Interval Clock Control Status Register

#define nRXCS    0x20 // Console Receive Control and Status Register
#define nRXDB    0x21 // Console Receive Data Buffer Register
#define nTXCS    0x22 // Console Transmit Control and Status Register
#define nTXDB    0x23 // Console Transmit Data Buffer Register

#define nSAVISP  0x29 // Console Saved ISP
#define nSAVPC   0x2A // Console Saved PC
#define nSAVPSL  0x2B // Console Saved PSL

#define nIORESET 0x37 // I/O Reset
#define nMAPEN   0x38 // Map Enable
#define nTBIA    0x39 // TB Clear All Entries
#define nTBIS    0x3A // TB Clear Single Entry
#define nSID     0x3E // System Identification
#define nTBCHK   0x3F // TB Check
*/

#define SCBB_MBZ   0x000001FF // Must be zeros.
#define BR_MASK    0xFFFFFFFC // xxBR Mask
#define LR_MASK    0x003FFFFF // xxLR Mask

// Independent Implmentations for KA630 Processor

#define ACCS     PRN(PR_ACCS)
// #define ICCS     PRN(PR_ICCS)
#define SAVISP   PRN(PR_SAVISP)
#define SAVPC    PRN(PR_SAVPC)
#define SAVPSL   PRN(PR_SAVPSL)
#define IORESET  PRN(PR_IORESET)

// Interval Clock Control Status Registers (Subset)

#define ICCS_IE     0x00000040  // Interrupt Enable
#define ICCS_WMASK  0x00000040  // Write Mask
#define ICCS_TICK   10000       // Each tick in usecs (10 msecs).
#define ICCS_SECOND 100         // Each second tick
#define ICCS_NVECS  1           // Number of Vectors
#define ICCS_IPL    UQ_BR6      // Interrupt Level BR6
#define ICCS_VEC    SCB_TIMER   // System Timer Vector

// Console Terminal Registers

// RXCS - Console Receive Control and Status Register
#define RXCS_MBZ   0xFFFFFF3F // Must be zeros.
#define RXCS_RDY   0x00000080 // (R)   Ready
#define RXCS_IE    0x00000040 // (R/W) Interrupt Enable

// RXDB - Console Receive Data Buffer Register
#define RXDB_ERROR 0x00008000 // (R)   Error
#define RXDB_ID    0x00000F00 // (R)   Identification
#define RXDB_DATA  0x000000FF // (R)   Data Received

// TXCS - Console Terminal Control and Status Register
#define TXCS_MBZ   0xFFFFFF3F // Must be zeros.
#define TXCS_RDY   0x00000080 // (R)   Ready
#define TXCS_IE    0x00000040 // (R/W) Interrupt Enable

// TXDB - Console Terminal Data Buffer Register
#define TXDB_MBZ   0xFFFFF000 // Must be zeros
#define TXDB_ID    0x00000F00 // (R/W) Identification
#define TXDB_DATA  0x000000FF // (R/W) Data Transmitted

// Read Privileged Register
int32 ka630_ReadRegister(register VAX_CPU *vax, int32 pReg)
{
	int32 data;

	switch (pReg) {
		case PR_KSP:
			data = (PSL & PSL_IS) ? KSP : SP;
			break;

		case PR_ISP:
			data = (PSL & PSL_IS) ? SP : ISP;
			break;

		case PR_IPL:
			IPL = data = PSL_GETIPL(PSL);
			break;

		case PR_RXCS:
			data = vax_ReadRXCS(vax);
			break;

		case PR_RXDB:
			data = vax_ReadRXDB(vax);
			break;

		case PR_TXCS:
			data = vax_ReadTXCS(vax);
			break;

//		case PR_TXDB:
//			data = vax_ReadTXDB(vax);
//			break;

		default:
			data = PRN(pReg);
			break;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		char *name = "Undefined Register";
		if ((pReg < regLength) && regNames[pReg])
			name = regNames[pReg];
		dbg_Printf("KA630: (R) %s (%02X) => %08X\n", name, pReg, data);
	}
#endif /* DEBUG */

	return data;
}

void ka630_WriteRegister(register VAX_CPU *vax, int32 pReg, int32 data)
{
	switch (pReg) {
		case PR_KSP: // Kernel Stack Pointer
			((PSL & PSL_IS) ? KSP : SP) = data;
			break;

		case PR_ESP: // Executive Stack Pointer
		case PR_SSP: // Supervisor Stack Pointer
		case PR_USP: // User Stack Pointer
			PRN(pReg) = data;
			break;

		case PR_ISP: // Interrupt Stack Pointer
			((PSL & PSL_IS) ? SP : ISP) = data;
			break;

		case PR_P0BR:
		case PR_P1BR:
		case PR_SBR:
			PRN(pReg) = data & BR_MASK;
			vax_ClearTBTable(vax, pReg == PR_SBR);
			break;

		case PR_P0LR:
		case PR_P1LR:
		case PR_SLR:
			PRN(pReg) = data & LR_MASK;
			vax_ClearTBTable(vax, pReg == PR_SLR);
			break;
		
		case PR_PCBB:
		case PR_SCBB:
			PRN(pReg) = data & LALIGN;
			break;

		case PR_IPL:
			IPL = data & PSL_M_IPL;
			PSL = PSL_PUTIPL(IPL) | (PSL & ~PSL_IPL);
			break;

		case PR_SIRR:
			if ((data > 0xF) || (data == 0))
				RSVD_OPND_FAULT;
			SISR |= (1 << data);
			break;

		case PR_SISR:
			SISR = data & SISR_MASK;
			break;

		case PR_ASTLVL:
			if ((uint32)data > AST_MAX)
				RSVD_OPND_FAULT;
			ASTLVL = data;
			break;

		case PR_ICCS:
			// Subset implementation in MicroVAX series
			ICCS = data & ICCS_WMASK;
			break;

		case PR_RXCS:
//			if (data & RXCS_MBZ)
//				RSVD_OPND_FAULT;
			vax_WriteRXCS(vax, data);
			break;

		case PR_RXDB:
//			vax_WriteRXDB(vax, data);
			break;

		case PR_TXCS:
//			if (data & TXCS_MBZ)
//				RSVD_OPND_FAULT;
			vax_WriteTXCS(vax, data);
			break;

		case PR_TXDB:
			vax_WriteTXDB(vax, data);
			break;

		case PR_SAVISP:
		case PR_SAVPC:
		case PR_SAVPSL:
			PRN(pReg) = data;
			break;

		case PR_IORESET:
			qba_ResetAll(((KA630_DEVICE *)vax)->qba);
			break;

		case PR_MAPEN:
			MAPEN = data & 1;
			break;

		case PR_TBIA:
			vax_ClearTBTable(vax, 1);
			break;

		case PR_TBIS:
			vax_ClearTBEntry(vax, data);
			break;

		case PR_TBCHK:
			if (vax_CheckTBEntry(vax, data))
				CC |= CC_V;
			break;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
		char *name = "Undefined Register";
		if ((pReg < regLength) && regNames[pReg])
			name = regNames[pReg];
		dbg_Printf("KA630: (W) %s (%02X) <= %08X\n", name, pReg, data);
	}
#endif /* DEBUG */
}

// ***********************************************************
// *************** KA630 System Configuration ****************
// ***********************************************************

void ka630_ResetCPU(KA630_DEVICE *ka630)
{
	VAX_CPU *vax = (VAX_CPU *)ka630;
	int32   idx;

#ifdef TEST_PARITY
	ka630->cpuState = ka630->q22State = 0;
	ka630->cpuAddr  = -1;
	ka630->q22Addr1 = ka630->q22Addr2 = -1;
#endif /* TEST_PARITY */

	// Show my identification that I am MicroVAX II system.

	SID = KA630_SYSID;

	// Clear all CPU and clock registers first
	for (idx = 0; idx < nCPUREG; idx++)
		ka630->cpuRegs[idx] = 0;
	for (idx = 0; idx < nCLKREG; idx++)
		ka630->clkRegs[idx] = 0;
//	for (idx = 0; idx < 8096; idx++)
//		ka630->mapRegs[idx] = 0;
}

// *********** Clock Timer System *************

// Tick each 10 milliseconds
// for ICCS register and RTC (TOY) chip
void ka630_ClockTimer(void *device)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)device;
	VAX_CPU *vax = (VAX_CPU *)ka630;
	MAP_IO  *io  = &ka630->ioClock;

	// Generate IPL 22 signal each 10 milliseconds
	if (ICCS & ICCS_IE)
		io->SendInterrupt(io, 0);

	if (ka630->TickCount++ == ICCS_SECOND) {
		ka630->TickCount = 0;

#ifdef DEBUG
		// Instructions Per Second Meter
		if (dbg_Check(DBG_IPS))
			dbg_Printf("KA630: %d ips\n", vax->ips);
#else
		printf("KA630: %d ips\n", vax->ips);
#endif /* DEBUG */
		vax->ips = 0;
	}
}

// Enable Clock Timer
void ka630_StartTimer(void *device)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)device;

	ka630->TickCount = 0;
	ka630->Timer.Flags |= CLK_ENABLE;
}

// Disable Clock Timer
void ka630_StopTimer(void *device)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)device;

	ka630->Timer.Flags &= ~CLK_ENABLE;
}

int32 ka630_CheckTimer(uint32 ipl)
{
//	if (ipl > ICCS_IPL)
//		return -1;
//	if (HIRQ & (1u << ICCS_IPL))
//		return ICCS_IPL;
//	return 0;
}

void ka630_InitIO(VAX_CPU *vax)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)vax;
	UQ_CALL      *cb    = ka630->Callback;
	MAP_IO       *io;

	// Set up clock interrupts
	io               = &ka630->ioClock;
	io->devName      = "CLK";
	io->keyName      = "CLK";
	io->emuName      = "KA630: Clock Interrupt";
	io->emuVersion   = NULL;
	io->nVectors     = ICCS_NVECS;
	io->intIPL       = ICCS_IPL;
	io->intVector[0] = ICCS_VEC;
	cb->SetMap(ka630->qba, io);

	if (ka630->cpu.Console)
		vax_ConsoleInit2(ka630->cpu.Console, cb);
}

//static const align[]  = { 0x00FFFFFF, 0x00FFFFFF, 0x0000FFFF, 0x000000FF };
//static const insert[] = { 0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF };

// Halt Action
void ka630_HaltAction(register VAX_CPU *vax, uint32 code)
{
	// Save ISP, PC, and PSL into SAVISP, SAVPC, SAVPSL.
	SAVISP = ISP;
	SAVPC  = PC;
	SAVPSL = PSL | CC | (code << 8) | (MAPEN ? 0x8000 : 0);

#ifdef DEBUG
	dbg_Printf("VAX: Halt Action - Reason %d\n", code);
#endif /* DEBUG */

	// Reset Processor Registers
	PC     = 0x20040000;
	PSL    = 0x041F0000;
	CC     = 0;
	MAPEN  = 0;
	SISR   = 0; 
	ASTLVL = 4;

	// Go Next Instruction
	FLUSH_ISTR;
	ABORT(NEXT);
}

// Machine-Check Exception
void ka630_MachineCheck(register VAX_CPU *vax)
{
	vax_DoIntexc(vax, SCB_MCHK, 0, IE_SVE);
	IN_IE = 1;
	WriteV(SP - 16, 0xC, LONG, WA);
	WriteV(SP - 12, P3, LONG, WA);
	WriteV(SP - 8,  P2, LONG, WA);
	WriteV(SP - 4,  P1, LONG, WA);
	SP -= 16;
	IN_IE = 0;

#ifdef DEBUG
	DumpHistory();
#endif /* DEBUG */
}

// Read CPU Register (Aligned Longword)
inline uint32 ka630_ReadCPU(KA630_DEVICE *ka630, uint32 pAddr, uint32 size)
{
	uint32 reg, data;

	reg = (pAddr & 0x7FFF) >> 2;
	if (reg < nCPUREG) {
		if ((size <= WORD) && (pAddr & 2))
			data = 0;
		else if (size == BYTE)
			data = (CPUREG(reg) >> ((pAddr & 1) ? 8 : 0)) & BMASK;
		else
			data = CPUREG(reg);
	} else {
		// Non-Existant Memory Exception
		return 0;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%08X) %08X => %08X (Size: %d)\n",
			ka630->cpu.devName, cpuNames[reg], pAddr, CPUREG(reg), data, size);
#endif /* DEBUG */

	return data;
}

void ka630_TestParity(VAX_CPU *, uint32);
void ka630_WriteParity(VAX_CPU *, uint32);

// Write CPU Register (Aligned Longword)
inline void ka630_WriteCPU(KA630_DEVICE *ka630, uint32 pAddr, uint32 data, uint32 size)
{
	uint32 reg = (pAddr & 0x7FFF) >> 2;

	switch (reg) {
		case nBDR: // Boot and Diagnostics Register
			if ((size <= WORD) && (pAddr & 2))
				break;
			if ((size == BYTE) && (pAddr & 1))
				break;
			BDR = (BDR & ~BDR_WMASK) | (data & BDR_WMASK);	

#ifdef TEST_PARITY
			if ((BDR & BDR_DSPL) == 0) {
				ka630->cpu.TestParity  = NULL;
				ka630->cpu.WriteParity = NULL;
			} else {
				ka630->cpu.TestParity  = ka630_TestParity;
				ka630->cpu.WriteParity = ka630_WriteParity;
			}
#endif /* TEST_PARITY */
			break;

		case nMSER: // Memory System Error Register
			if ((size <= WORD) && (pAddr & 2))
				break;
			if ((size == BYTE) && (pAddr & 1))
				break;
			MSER = (MSER & ~MSER_WMASK) | (data & MSER_WMASK);
			MSER &= ~(data & MSER_CMASK);
#ifdef TEST_PARITY
			if ((data & MSER_PEN) == 0) {
				ka630->cpuState = ka630->q22State = 0;
				ka630->cpuAddr  = -1;
				ka630->q22Addr1 = ka630->q22Addr2 = -1;
			}
#endif /* TEST_PARITY */
			break;

		case nCEAR: // CPU Error Address Register
		case nDEAR: // DMA Error Address Register
			// Read-Only Registers - Do nothing
			break;

		default:
			// Non-Existant Memory Exception
			return;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) %s (%08X) %08X <= %08X (Size: %d)\n",
			ka630->cpu.devName, cpuNames[reg], pAddr, CPUREG(reg), data, size);
#endif /* DEBUG */
}

inline uint32 ka630_ReadClock(KA630_DEVICE *ka630, uint32 pAddr, uint32 size)
{
	uint32 reg = pAddr & 0x7FFF;
	uint32 data;

	if (reg < (nCLKREG << 1)) {
		if (size >= LONG)
			data = LREGU(ka630->clkRegs, reg);
		else if (size == WORD)
			data = WREGU(ka630->clkRegs, reg);
		else
			data = BREGU(ka630->clkRegs, reg);
	} else
		return 0;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		VAX_CPU *vax = (VAX_CPU *)ka630;
		dbg_Printf("%s: (R) Clock Register %d => %04X (Size %d) at PC %08X\n",
			ka630->cpu.devName, reg >> 1, data, size, faultPC);
	}
#endif /* DEBUG */

	return data;
}

inline void ka630_WriteClock(KA630_DEVICE *ka630, uint32 pAddr, uint32 data, uint32 size)
{
	uint32 reg = pAddr & 0x7FFF;

	if (reg < (nCLKREG << 1)) {
		data &= (pAddr & 1) ? 0xFF00FF00 : 0x00FF00FF;
		if (size >= LONG)
			LREGU(ka630->clkRegs, reg) = data;
		else if (size == WORD)
			WREGU(ka630->clkRegs, reg) = data;
		else
			BREGU(ka630->clkRegs, reg) = data;
	} else
		return;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		VAX_CPU *vax = (VAX_CPU *)ka630;
		dbg_Printf("%s: (W) Clock Register %d <= %04X (Size %d) at PC %08X\n",
			ka630->cpu.devName, reg >> 1, data, size, faultPC);
	}
#endif /* DEBUG */
}

// Read Word Aligned (Q22 I/O Space)
uint32 ka630_ReadIO(KA630_DEVICE *ka630, uint32 pAddr, uint32 size)
{
	uint32  ioAddr = pAddr & 0x1FFF;
	uint16  data;
	VAX_CPU *vax;
	void    *qba;

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS)) {
	{
		uint32 addr = ioAddr + 0x3FE000;
		dbg_Printf("QBA: (R) %08X => %06X (%08o) at PC %08X\n",
			pAddr, addr, addr, ka630->cpu.fault_PC);
	}
#endif /* DEBUG */

	// Read I/O Register from Q22-Bus I/O Device...
	qba = ka630->qba;
	if (qba && (qba_ReadIO(qba, ioAddr, &data, size) == UQ_OK))
		return data;

	// Non-Existant Device Exception
	vax = (VAX_CPU *)ka630;
#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("QBA: (R) Non-Existant Device at PC %08X\n",
			ka630->cpu.fault_PC);
#endif /* DEBUG */

	MSER = (MSER & ~MSER_CD) | MSER_NXM;

	// Go to machine check exception
	P1 = 0;
	P2 = pAddr;
	P3 = MCHK_RBP;
	ABORT(-SCB_MCHK);
}

// Write Word Aligned (Q22 I/O Space)
void ka630_WriteIO(KA630_DEVICE *ka630, uint32 pAddr, uint32 data, uint32 size)
{
	uint32  ioAddr = pAddr & 0x1FFF;
	VAX_CPU *vax;
	void    *qba;

#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS)) {
	{
		int addr = ioAddr + 0x3FE000;
		dbg_Printf("QBA: (W) %08X => %06X (%08o) at PC %08X\n",
			pAddr, addr, addr, ka630->cpu.fault_PC);
	}
#endif /* DEBUG */

	// Write I/O Register to Q22-Bus I/O Device...
	qba = ka630->qba;
	if (qba && (qba_WriteIO(qba, ioAddr, data, size) == UQ_OK))
		return;

	// Non-Existant Device Exception
	vax = (VAX_CPU *)ka630;
#ifdef DEBUG
//	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("QBA: (W) Non-Existant Device at PC %08X\n",
			ka630->cpu.fault_PC);
#endif /* DEBUG */

	MSER = (MSER & ~MSER_CD) | MSER_NXM;

	// Go to machine check exception
	P1 = 0;
	P2 = pAddr;
	P3 = MCHK_WBP;
	ABORT(-SCB_MCHK);
}

// *******************************************************
// *******************************************************
// *******************************************************

#ifdef TEST_PARITY

// Test Parity
void ka630_TestParity(VAX_CPU *vax, uint32 pAddr)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)vax;

	// This routine is stub for test #3
	if ((pAddr == ka630->cpuAddr) && (MSER & MSER_PEN)) {
#ifdef DEBUG
		dbg_Printf("PAR: Parity Error Trap on loc %08X at PC %08X\n",
			pAddr, faultPC);
#endif /* DEBUG */
		MSER |= 0x300 | MSER_LPE;
		if (ka630->cpuState++ < 1)
			CEAR = pAddr >> 9;
		else
			MSER |= MSER_LEB;
	
		// Go to machine check exception
		P1 = 0;
		P2 = pAddr;
		P3 = MCHK_RBP;
		ABORT(-SCB_MCHK);
	}
}

// Write Parity
void ka630_WriteParity(VAX_CPU *vax, uint32 pAddr)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)vax;

	if (MSER & MSER_PEN) {
		if (MSER & MSER_WWP) {
			ka630->cpuAddr = pAddr;
#ifdef DEBUG
			dbg_Printf("PAR: Set Wrong Parity on loc %08X at PC %08X\n",
				pAddr, faultPC);
#endif /* DEBUG */
		} else if (ka630->cpuAddr == pAddr) {
			ka630->cpuAddr = -1;
#ifdef DEBUG
			dbg_Printf("PAR: Clear Wrong Parity on loc %08X at PC %08X\n",
				pAddr, faultPC);
#endif /* DEBUG */
		}
	}
}

#endif /* TEST_PARITY */

// Physical Read Aligned
uint32 ka630_ReadAligned(VAX_CPU *vax, int32 pAddr, int32 size)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)vax;

	if (IN_ROM(pAddr)) {
		pAddr &= vax->maskROM;
		if (size >= LONG)
			return LROM(pAddr >> 2);
		else if (size == WORD)
			return WROM(pAddr >> 1);
		else
			return BROM(pAddr);
	} else if (IN_CPUREG(pAddr))
			return ka630_ReadCPU(ka630, pAddr, size);
	else if (IN_CLKREG(pAddr))
		return ka630_ReadClock(ka630, pAddr, size);
	else if (IN_Q22MAP(pAddr))
		return u2qba_ReadMAP(ka630->qba, pAddr);
	else if (IN_Q22IO(pAddr))
		return ka630_ReadIO(ka630, pAddr, size);
	else if (IN_Q22MEM(pAddr))
		return u2qba_ReadMEM(ka630, pAddr, size);
	else {
		// Non-Existant Memory Exception
#ifdef DEBUG
//		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (R) CPU Non-Existant Memory (%08X) at PC %08X\n",
				vax->devName, pAddr, faultPC);
#endif /* DEBUG */
		MSER |= MSER_NXM;

		// Go to machine check exception
		P1 = 0;
		P2 = pAddr;
		P3 = MCHK_RBP;
		ABORT(-SCB_MCHK);
	}

	return 0;
}

// Physical Write Aligned
void ka630_WriteAligned(VAX_CPU *vax, int32 pAddr, uint32 data, int32 size)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)vax;

	if (IN_CPUREG(pAddr))
		ka630_WriteCPU(ka630, pAddr, data, size);
	else if (IN_CLKREG(pAddr))
		ka630_WriteClock(ka630, pAddr, data, size);
	else if (IN_Q22MAP(pAddr))
		u2qba_WriteMAP(ka630->qba, pAddr, data);
	else if (IN_Q22IO(pAddr))
		ka630_WriteIO(ka630, pAddr, data, size);
	else if (IN_Q22MEM(pAddr))
		u2qba_WriteMEM(ka630, pAddr, data, size);
	else {
		// Non-Existant Memory Exception
#ifdef DEBUG
//		if (dbg_Check(DBG_IOREGS))
			dbg_Printf("%s: (W) CPU Non-Existant Memory (%08X) at PC %08X\n",
				vax->devName, pAddr, faultPC);
#endif /* DEBUG */
		MSER |= MSER_NXM;

		// Go to machine check exception
		P1 = 0;
		P2 = pAddr;
		P3 = MCHK_WBP;
		ABORT(-SCB_MCHK);
	}
}

// *******************************************************
// ************* Console Read/Write Access ***************
// *******************************************************

// Console Read Aligned
int ka630_ReadC(VAX_CPU *vax, int32 pAddr, uint32 *data, int32 size)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)vax;

	if (IN_RAM(pAddr)) {
		if (size >= LONG)
			*data = LMEM(pAddr >> 2);
		else if (size == WORD)
			*data = WMEM(pAddr >> 1);
		else
			*data = BMEM(pAddr);
		return MM_OK;
	} else if (IN_ROM(pAddr)) {
		pAddr &= vax->maskROM;
		if (size >= LONG)
			*data = LROM(pAddr >> 2);
		else if (size == WORD)
			*data = WROM(pAddr >> 1);
		else
			*data = BROM(pAddr);
		return MM_OK;
	} else if (IN_CPUREG(pAddr))
		return ka630_ReadCPU(ka630, pAddr, size);
	else if (IN_CLKREG(pAddr))
		return ka630_ReadClock(ka630, pAddr, size);
	else if (IN_Q22MAP(pAddr))
		return u2qba_ReadMAP(ka630->qba, pAddr);
	else if (IN_Q22IO(pAddr))
		return ka630_ReadIO(ka630, pAddr, size);
	else if (IN_Q22MEM(pAddr))
		return u2qba_ReadMEM(ka630, pAddr, size);

	return MM_NXM;
}

// Console Write Aligned
int ka630_WriteC(register VAX_CPU *vax, int32 pAddr, uint32 data, int32 size)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)vax;

	if (IN_RAM(pAddr)) {
		if (size >= LONG)
			LMEM(pAddr >> 2) = data;
		else if (size == WORD)
			WMEM(pAddr >> 1) = data;
		else
			BMEM(pAddr) = data;
	} else if (IN_CPUREG(pAddr))
		ka630_WriteCPU(ka630, pAddr, data, size);
	else if (IN_CLKREG(pAddr))
		ka630_WriteClock(ka630, pAddr, data, size);
	else if (IN_Q22MAP(pAddr))
		u2qba_WriteMAP(ka630->qba, pAddr, data);
	else if (IN_Q22IO(pAddr))
		ka630_WriteIO(ka630, pAddr, data, size);
	else if (IN_Q22MEM(pAddr))
		u2qba_WriteMEM(ka630, pAddr, data, size);
	else
		return MM_NXM;

	return MM_OK;
}

// **********************************************************
// ************ MicroVAX II System Configuration ************
// **********************************************************

void *ka630_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	KA630_DEVICE *ka630 = NULL;
	VAX_SYSTEM   *vaxsys;
	VAX_CPU      *vax;
	CLK_QUEUE    *Clock;

	if (ka630 = (KA630_DEVICE *)calloc(1, sizeof(KA630_DEVICE))) {
		// First, link it to VAX system device.
		vaxsys             = (VAX_SYSTEM *)newMap->sysDevice;
		ka630->cpu.System  = vaxsys;
		ka630->cpu.Console = vax_ConsoleInit((VAX_CPU *)ka630);
		vaxsys->Processor  = (VAX_CPU *)ka630;
	
		// Build instruction table
		vax_BuildCPU((VAX_CPU *)ka630, 0);
		ka630_ResetCPU(ka630);

		// ACTION: Will implment a command later.
		vax_InitMemory((VAX_CPU *)ka630, 16384 * 1024);

		// Reload current time/date into clock registers
		// Load saved NVRAM data into TOY chip.
		// ACTION: Implement that.

		// Set up clock timer
		Clock = &ka630->Timer;
		Clock->Next     = NULL;
		Clock->Flags    = CLK_REACTIVE;
		Clock->outTimer = ICCS_TICK;
		Clock->nxtTimer = ICCS_TICK;
		Clock->Device   = (void *)ka630;
		Clock->Execute  = ka630_ClockTimer;

		// Now enable clock timer (on host timer)
		vax = (VAX_CPU *)ka630;
		ts10_SetRealTimer(Clock);

		// Set up functions for KA630 CPU (MicroVAX II Series)
		ka630->cpu.ReadRegister   = ka630_ReadRegister;
		ka630->cpu.WriteRegister  = ka630_WriteRegister;
		ka630->cpu.ReadAligned    = ka630_ReadAligned;
		ka630->cpu.WriteAligned   = ka630_WriteAligned;
		ka630->cpu.ReadCA         = ka630_ReadC;
		ka630->cpu.WriteCA        = ka630_WriteC;
		ka630->cpu.TestParity     = ka630_TestParity;
		ka630->cpu.WriteParity    = ka630_WriteParity;
		ka630->cpu.MachineCheck   = ka630_MachineCheck;
		ka630->cpu.HaltAction     = ka630_HaltAction;
		ka630->cpu.CheckTimer     = ka630_CheckTimer;
		ka630->cpu.StartTimer     = ka630_StartTimer;
		ka630->cpu.StopTimer      = ka630_StopTimer;
		ka630->cpu.InitIO         = ka630_InitIO;

		// Finally, Set up its descriptions and
		// link it to mapping device.
		ka630->cpu.devName    = newMap->devName;
		ka630->cpu.keyName    = newMap->keyName;
		ka630->cpu.emuName    = newMap->emuName;
		ka630->cpu.emuVersion = newMap->emuVersion;
		newMap->Device        = ka630;
#ifdef DEBUG
		newMap->Breaks        = &ka630->cpu.Breaks;
#endif /* DEBUG */
	}

	return ka630;
}

// Delete CPU Device
int ka630_Delete(void *dptr)
{
}

int ka630_Reset(void *dptr)
{
	ka630_ResetCPU(dptr);
	return VAX_OK;
}

int ka630_Boot(MAP_DEVICE *map, int argc, char **argv)
{
	KA630_DEVICE *ka630 = (KA630_DEVICE *)map->Device;
	VAX_CPU      *vax   = (VAX_CPU *)ka630;

	// Tell operator that processor is being booted.
	printf("Booting %s: (%s)... ", ka630->cpu.devName, ka630->cpu.keyName);

	// Initialize KA630 Processor
	vax->State = VAX_HALT;
	ka630_Reset(ka630);

	// Set BDR register as power-up state
	BDR = BDR_PWROK | BDR_HLTENB | 0xF;

	// Must load 0x300 (Restart Code - Power On) into SAVPSL in order that
	// the console program perform properly.

	// Initial Power On
	MAPEN  = 0;
	SAVPSL = HLT_PON << 8;
	PC     = 0x20040000;
	PSL    = 0x041F0000;
	CC     = 0;
	SISR   = 0; 
	ASTLVL = 4;

	// Enable KA630 processor to run.
	vax->State = VAX_RUN;
	emu_State  = VAX_RUN;

	// Tell operator that it was done.
	printf("done.\n");
}

DEVICE *ka630_Devices[] =
{
	&qba_Device, // Q22-Bus Interface
	NULL         // Terminator
};

// MicroVAX II Device Information Table
DEVICE vax_System_KA630 =
{
	KA630_DTNAME,     // Device Type Name
	KA630_NAME,       // Emulator Name
	KA630_VERSION,    // Emulator Version
	ka630_Devices,    // Listing of Devices
	DF_USE|DF_SYSMAP, // Device Flags
	DT_PROCESSOR,     // Device Type

	NULL, NULL, NULL,

	ka630_Create,     // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	ka630_Reset,      // Reset Routine
	NULL,             // Attach Routine
	NULL,             // Detach Routine
	NULL,             // Info Routine
	ka630_Boot,       // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};

