// ka650.c - CVAX Series System Configurations and Routines
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

// CVAX Series - KA650/KA655 Processors
//   KA650 Processor - MicroVAX III Series (3500/3600)
//   KA655 Processor - MicroVAX III+ Series (3800/3900)
//
// Some derviated works from Simh VAX emulator due to lack of
// KA655 technical specification (incomplete).
//
// -------------------------------------------------------------------------
//
// Modification History:
//
// 03/11/03  TMS  Added command tables to device tables.
//
// -------------------------------------------------------------------------

#include <time.h>
#include "vax/defs.h"
#include "dev/uba/dec/defs.h"
#include "vax/ka650.h"
#include "vax/mtpr.h"

#define KA650_KEY     "KA650" // Device Type Name
#define KA650_NAME    "MicroVAX III Emulator"

#define KA655_KEY     "KA655" // Device Type Name
#define KA655_NAME    "MicroVAX III+ Emulator"

#define KA65X_VERSION "v0.8 (Late Alpha)"

// KA650/KA655 - CVAX Memory Map
//
// 00000000 +-------------------------------------+
//          |           Main Memory               |
//          |- - - - - - - - - - - - - - - - - - -|
//          |        Up to 64 MB Memory           |
// 01000000 +-------------------------------------+
//          |             Reserved                |
// 20000000 +-------------------------------------+
//          |           QBus I/O space            |
// 20002000 +-------------------------------------+
//          |             Reserved                |
// 20040000 +-------------------------------------+
//          |               ROM                   |
// 20080000 +-------------------------------------+
//          |      Local I/O Register Area        |
// 20200000 +-------------------------------------+
//          |             Reserved                |
// 30000000 +-------------------------------------+
//          |          QBus Memory Space          |
// 303FFFFF +-------------------------------------+
//          |             Reserved                |
// 3FFFFFFF +-------------------------------------+
//
// Systen Memory Map
//
//   0000 0000 - 03FF FFFF  Main Memory (Up to 64 MB)
//   1000 0000 - 13FF FFFF  Secondary Cache Diagnostic Space
//
//   2000 0000 - 2000 1FFF  Qbus I/O Page
//   2004 0000 - 2005 FFFF  ROM Space, Halt Protected
//   2006 0000 - 2007 FFFF  ROM Space, Halt Unprotected
//   2008 0000 - 201F FFFF  Local Register Space
//   3000 0000 - 303F FFFF  Qbus Memory Space

#ifdef DEBUG
// CMCTL Register Names
static char *cmctlNames[] = {
	"CMCNF00",  // Configuration Register - Bank 0
	"CMCNF01",  // Configuration Register - Bank 1
	"CMCNF02",  // Configuration Register - Bank 2
	"CMCNF03",  // Configuration Register - Bank 3
	"CMCNF04",  // Configuration Register - Bank 4
	"CMCNF05",  // Configuration Register - Bank 5
	"CMCNF06",  // Configuration Register - Bank 6
	"CMCNF07",  // Configuration Register - Bank 7
	"CMCNF08",  // Configuration Register - Bank 8
	"CMCNF09",  // Configuration Register - Bank 9
	"CMCNF10",  // Configuration Register - Bank 10
	"CMCNF11",  // Configuration Register - Bank 11
	"CMCNF12",  // Configuration Register - Bank 12
	"CMCNF13",  // Configuration Register - Bank 13
	"CMCNF14",  // Configuration Register - Bank 14
	"CMCNF15",  // Configuration Register - Bank 15
	"CMERR",    // Error Register
	"CMCSR",    // Control/Status Register
};
#endif /* DEBUG */

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
	"CADR",    //       25 Cache Disable Register
	NULL,      //       26 Unknown
	"MSER",    //       27 Memory System Error Register
	"ACCS",    //       28 *Accelerator Control and Status
	NULL,      //       29 *Accelerator Maintenance
	"CONPC",   //       2A Console PC
	"CONPSL",  //       2B Console PSL
	NULL,      //       2C *Writable-Control-Store Address
	NULL,      //       2D *Writable-Control-Store Data
	NULL,      //       2E Unknown
	NULL,      //       2F Unknown
	"SBIFS",   //       30 *SBI Fault Status
	"SBIS",    //       31 *SBI silo
	"SBISC",   //       32 *SBI silo comparator
	"SBIMT",   //       33 *SBI Maintenance
	"SBIER",   //       34 *SBI Error
	"SBITA",   //       35 *SBI Timeout Address
	"SBIQC",   //       36 *SBI Quadword Clear
	NULL,      //       37 Unknown
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

#define SCBB_MBZ   0x000001FF // Must be zeros.
#define BR_MASK    0xFFFFFFFC // xxBR Mask
#define LR_MASK    0x003FFFFF // xxLR Mask

// #define CADR_RW    0xF3
// #define CADR_MBO   0x0C

// #define MSER_HM    0x80
#define MSER_CPE   0x40
#define MSER_CPM   0x20

#define CADR      PRN(PR_CADR)
#define MSER      PRN(PR_MSER)
#define CONPC     PRN(PR_CONPC)
#define CONPSL    PRN(PR_CONPSL)

/*
IPR_TABLE ka650_iprTable[] =
{
	{ PR_KSP,  vax_ReadKSP,  vax_WriteKSP  }, // Kernel Stack Pointer
	{ PR_ESP,  vax_ReadESP,  vax_WriteESP  }, // Executive Stack Pointer
	{ PR_SSP,  vax_ReadSSP,  vax_WriteSSP  }, // Supervisor Stack Pointer
	{ PR_USP,  vax_ReadUSP,  vax_WriteUSP  }, // User Stack Pointer
	{ PR_ISP,  vax_ReadISP,  vax_WriteISP  }, // Interrupt Stack Pointer

	// VAX Memory Management Registers
	{ PR_P0BR, vax_ReadP0BR, vax_WriteP0BR }, // P0 Base Register
	{ PR_P0LR, vax_ReadP0LR, vax_WriteP0LR }, // P0 Length Register
	{ PR_P1BR, vax_ReadP1BR, vax_WriteP1BR }, // P1 Base Register
	{ PR_P1LR, vax_ReadP1LR, vax_WriteP1LR }, // P1 Length Register
	{ PR_SBR,  vax_ReadSBR,  vax_WriteSBR  }, // System Base Register
	{ PR_SLR,  vax_ReadSLR,  vax_WriteSLR  }, // System Length Register

	{ NULL }
};
*/

// Read Privileged Register
int32 ka650_ReadRegister(register VAX_CPU *vax, int32 pReg)
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
		dbg_Printf("KA650: (R) %s (%02X) => %08X\n", name, pReg, data);
	}
#endif /* DEBUG */

	return data;
}

void ka650_WriteRegister(register VAX_CPU *vax, int32 pReg, int32 data)
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
#ifdef DEBUG
			if (dbg_Check(DBG_INTERRUPT))
				dbg_Printf("KA650: Clock Interrupt Enable: %s\n",
					(data & ICCS_IE) ? "On" : "Off");
#endif /* DEBUG */
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

		case PR_CADR:
			PRN(pReg) = (data & CADR_RW) | CADR_MBO;
			break;

		case PR_MSER:
			PRN(pReg) &= MSER_HM;
			break;

		case PR_CONPC:
		case PR_CONPSL:
			PRN(pReg) = data;
			break;

		case PR_IORESET:
			cq_ResetAll(((KA650_DEVICE *)vax)->qba);
			break;

		case PR_MAPEN:
			MAPEN = data & 1;
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
		dbg_Printf("KA650: (W) %s (%02X) <= %08X\n", name, pReg, data);
	}
#endif /* DEBUG */
}

// ********************************************************************

inline void ka650_ResetCPU(KA650_DEVICE *ka650)
{
	VAX_CPU *vax = (VAX_CPU *)ka650;

	SID = (CVAX_SID | CVAX_REV);
}

#define NVRAM(reg) ka650->sscRAM[reg]

uint32 ka650_ReadKA(KA650_DEVICE *ka650, uint32 pAddr)
{
	int reg = (pAddr - KA_BASE) >> 2;

	switch (reg) {
		case nCACR:
			return ka650->cacr;
		case nBDR:
			return ka650->bdr;
		default:
			return 0;
	}
}

void ka650_WriteKA(KA650_DEVICE *ka650, uint32 pAddr, uint32 data, uint32 size)
{
	int reg = (pAddr - KA_BASE) >> 2;

	if ((reg == 0) && ((pAddr & 3) == 0)) {
		ka650->cacr = (ka650->cacr & ~(data & CACR_W1C)) | CACR_FIXED;
		ka650->cacr = (data & CACR_RW) | (ka650->cacr & ~CACR_RW);
	}
}

// Read Non-volatile RAM
uint32 ka650_ReadNVR(KA650_DEVICE *ka650, uint32 pAddr)
{
	int reg = (pAddr - NVR_BASE) >> 2;

	return NVRAM(reg);
}

// Write Non-volatile RAM
void ka650_WriteNVR(KA650_DEVICE *ka650, uint32 pAddr, uint32 data, uint32 size)
{
	int reg = (pAddr - NVR_BASE) >> 2;

	if (size < OP_LONG) {
		uint32 mask = (size == OP_WORD) ? WMASK : BMASK;
		uint32 sc   = (pAddr & 3) << 3;
		NVRAM(reg)  = ((data & mask) << sc) | (NVRAM(reg) & ~(mask << sc));
	} else
		NVRAM(reg) = data;
}

// Read SSC Registers
uint32 ka650_ReadSSC(KA650_DEVICE *ka650, uint32 pAddr)
{
	int reg = (pAddr - SSC_BASE) >> 2;

	switch (reg) {
		case nSSC_BASE:  // SSC Base Register
			return ka650->sscBase;
		case nSSC_CR:    // SSC Configuration Register
			return ka650->sscConfig;
		case nSSC_BTO:   // SSC Bus Timeout Register
			return ka650->sscTimeout;
		case nSSC_DLEDR: // SSC Display LED Register
			return ka650->sscLED;

		case nSSC_TCR0:  // Timer #0 - Configuration Register
			return ka650->tcr[0];
		case nSSC_TIR0:  // Timer #0 - Interval Register
			return ka650->tir[0];
		case nSSC_TNIR0: // Timer #0 - Next Interval Register
			return ka650->tnir[0];
		case nSSC_TIVR0: // Timer #0 - Interrupt Vector Register
			return ka650->tivr[0];
		case nSSC_TCR1:  // Timer #1 - Configuration Register
			return ka650->tcr[1];
		case nSSC_TIR1:  // Timer #1 - Interval Register
			return ka650->tir[1];
		case nSSC_TNIR1: // Timer #1 - Next Interval Register
			return ka650->tnir[1];
		case nSSC_TIVR1: // Timer #1 - Interrupt Vector Register
			return ka650->tivr[1];
		case nSSC_AD0MAT:
			return ka650->admat[0];
		case nSSC_AD0MSK:
			return ka650->admsk[0];
		case nSSC_AD1MAT:
			return ka650->admat[1];
		case nSSC_AD1MSK:
			return ka650->admsk[1];
		default:
			return 0;
	}
}

void ka650_WriteTCR(KA650_DEVICE *, int, uint32);

// Write SSC Registers
void ka650_WriteSSC(KA650_DEVICE *ka650, uint32 pAddr, uint32 data, uint32 size)
{
	int    reg = (pAddr - SSC_BASE) >> 2;
	MAP_IO *io;

	// If operand size is byte or word, merge it with current data.
	if (size < OP_LONG) {
		uint32 tmp  = ka650_ReadSSC(ka650, pAddr);
		uint32 mask = (size == OP_WORD) ? WMASK : BMASK;
		uint32 sc   = (pAddr & 3) << 3;
		data = ((data & mask) << sc) | (tmp & ~(mask << sc));
	}

	switch (reg) {
		case nSSC_BASE:
			ka650->sscBase = (data & SSCBASE_RW) | SSCBASE_MBO;
			break;

		case nSSC_CR:
			ka650->sscConfig &= ~(data & SSCCNF_W1C);
			ka650->sscConfig =  (data & SSCCNF_RW) |
				(ka650->sscConfig & ~SSCCNF_RW);
			break;

		case nSSC_BTO:
			ka650->sscTimeout &= ~(data & SSCBTO_W1C);
			ka650->sscTimeout =  (data & SSCBTO_RW) |
				(ka650->sscTimeout & ~SSCBTO_RW);
			break;

		case nSSC_DLEDR:
			ka650->sscLED = data & SSCLED_MASK;
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Display LED Countdown: %X\n",
					ka650->cpu.devName, (ka650->sscLED ^ SSCLED_MASK));
#endif /* DEBUG */
			break;

		case nSSC_TCR0:   // Timer #0 - Configuration Register
			ka650_WriteTCR(ka650, 0, data);
			break;
		case nSSC_TNIR0:  // Timer #0 - Next Interval Register
			ka650->tnir[0] = data;
			break;
		case nSSC_TIVR0:  // Timer #0 - Interrupt Vector Register
			ka650->tivr[0] = data & TIVR_VEC;
			io = &ka650->ioTimer[0];
			io->SetVector(io, data & TIVR_VEC, 0);
			break;
		case nSSC_TCR1:   // Timer #1 - Configuration Register
			ka650_WriteTCR(ka650, 1, data);
			break;
		case nSSC_TNIR1:  // Timer #1 - Next Interval Register
			ka650->tnir[1] = data;
			break;
		case nSSC_TIVR1:  // Timer #1 - Interrupt Vector Register
			ka650->tivr[1] = data & TIVR_VEC;
			io = &ka650->ioTimer[1];
			io->SetVector(io, data & TIVR_VEC, 0);
			break;
		case nSSC_AD0MAT:
			ka650->admat[0] = data & ADS_MASK;
			break;
		case nSSC_AD0MSK:
			ka650->admsk[0] = data & ADS_MASK;
			break;
		case nSSC_AD1MAT:
			ka650->admat[1] = data & ADS_MASK;
			break;
		case nSSC_AD1MSK:
			ka650->admsk[1] = data & ADS_MASK;
			break;
	}
}

uint32 ka650_ReadCMCTL(KA650_DEVICE *ka650, uint32 pAddr)
{
	int reg = (pAddr - CMCTL_BASE) >> 2;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %08X: %s (%d) => %08X\n", ka650->cpu.devName,
			pAddr, cmctlNames[reg], reg, ka650->cmctl[reg]);
#endif /* DEBUG */

	switch(reg) {
		case nCMERR:
			return ka650->cmctl[nCMERR];
		case nCMCSR:
			return ka650->cmctl[nCMCSR] & CMCSR_MASK;
		default:
			return ka650->cmctl[reg] & CMCNF_MASK;
	}
}

void ka650_WriteCMCTL(KA650_DEVICE *ka650,
	uint32 pAddr, uint32 data, uint32 size)
{
	int reg = (pAddr - CMCTL_BASE) >> 2;

	if (size < LN_LONG)
		data <<= ((pAddr & 3) << 3);

	switch(reg) {
		case nCMERR:
			ka650->cmctl[nCMERR] &= ~(data & CMERR_W1C);
			break;

		case nCMCSR:
			ka650->cmctl[nCMCSR] = data & CMCSR_MASK;
			break;

		default:
			if (data & CMCNF_SRQ) {
				VAX_CPU *vax = (VAX_CPU *)ka650;
				uint32  idx;

				for (idx = reg; idx < (reg + 4); idx++) {
					ka650->cmctl[idx] &= ~CMCNF_SIG;
					if (IN_RAM(idx * MEM_BANK))
						ka650->cmctl[idx] |= MEM_SIG;
				}
			}
			ka650->cmctl[reg] = (data & CMCNF_RW) |
				(ka650->cmctl[reg] & ~CMCNF_RW);
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) %08X: %s (%d) %08X <= %08X\n", ka650->cpu.devName,
			pAddr, cmctlNames[reg], reg, ka650->cmctl[reg], data);
#endif /* DEBUG */
}

// Cache Diagnostic Space
uint32 ka650_ReadCDG(KA650_DEVICE *ka650, uint32 pAddr)
{
	uint32 row  = CDG_GETROW(pAddr);
	uint32 tag  = CDG_GETTAG(pAddr);
	uint32 data = ka650->cdgData[row];

	ka650->cacr &= ~CACR_DRO; // Clear Diagnostic
	ka650->cacr |= (Parity((data >> 24) & 0xFF, 1) << (CACR_P_DPAR + 3)) |
	               (Parity((data >> 16) & 0xFF, 0) << (CACR_P_DPAR + 2)) |
	               (Parity((data >> 8)  & 0xFF, 1) << (CACR_P_DPAR + 1)) |
	               (Parity(data & 0xFF, 0) << CACR_P_DPAR);

	return data;
}

void ka650_WriteCDG(KA650_DEVICE *ka650, uint32 pAddr, uint32 data, uint32 size)
{
	uint32 row = CDG_GETROW(pAddr);

	// Merge it with existing contents of cache data.
	if (size < LN_LONG) {
		uint32 tmp  = ka650->cdgData[row];
		uint32 mask = (size == OP_WORD) ? WMASK : BMASK;
		uint32 sc   = (pAddr & 3) << 3;
		data = ((data & mask) << sc) | (tmp & ~(mask << sc));
	}

	// Store it.
	ka650->cdgData[row] = data;
}

// ********************************************************************

extern uint32 cq_ReadBIC(void *, uint32);
extern void cq_WriteBIC(void *, uint32, uint32, uint32);
extern uint32 cq_ReadMAP(void *, uint32);
extern void cq_WriteMAP(void *, uint32, uint32, uint32);
extern uint32 cq_ReadMEM(void *, uint32);
extern void cq_WriteMEM(void *, uint32, uint32, uint32);

// I/O Memory Table
static IOMAP ioMap[] =
{
	{ CQMAP_BASE, CQMAP_END, 1, cq_ReadMAP,      cq_WriteMAP      },
	{ NVR_BASE,   NVR_END,   0, ka650_ReadNVR,   ka650_WriteNVR   },
	{ CMCTL_BASE, CMCTL_END, 0, ka650_ReadCMCTL, ka650_WriteCMCTL },
	{ SSC_BASE,   SSC_END,   0, ka650_ReadSSC,   ka650_WriteSSC   },
	{ KA_BASE,    KA_END,    0, ka650_ReadKA,    ka650_WriteKA    },
	{ CQBIC_BASE, CQBIC_END, 1, cq_ReadBIC,      cq_WriteBIC      },
	{ CQMEM_BASE, CQMEM_END, 1, cq_ReadMEM,      cq_WriteMEM      },
	{ CDG_BASE,   CDG_END,   0, ka650_ReadCDG,   ka650_WriteCDG   },
	{ 0 } // Terminator
};

uint32 ka650_ReadIO(VAX_CPU *vax, int32 pAddr, int32 size)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)vax;
	IOMAP        *io;
	uint32       data;

	if (IN_ROM(pAddr)) {
		pAddr &= vax->maskROM;
		if (size >= LN_LONG)
			return LROM(pAddr >> 2);
		else if (size == LN_WORD)
			return WROM(pAddr >> 1);
		else
			return BROM(pAddr);
	}

	if (IN_IO(pAddr))
		return cq_ReadIO(ka650->qba, pAddr, size);

	for (io = &ioMap[0]; io->loAddr != 0; io++) {
		if ((pAddr >= io->loAddr) && (pAddr < io->hiAddr) && io->Read) {
			data = io->Read(io->Type ? ka650->qba : ka650, pAddr);
			if (size < LN_LONG) {
				if (size == LN_BYTE)
					return (data >> ((pAddr & 3) << 3)) & BMASK;
				return (data >> ((pAddr & 2) ? 16 : 0)) & WMASK;
			}
			return data;
		}
	}

	// Non-Existant Memory Exception
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) CPU Non-Existant Memory (%08X) at PC %08X\n",
			ka650->cpu.devName, pAddr, faultPC);
#endif /* DEBUG */

	ka650->sscTimeout |= (SSCBTO_BTO|SSCBTO_RWT);

	// Go to machine check exception
	MACH_CHECK(MCHK_READ);

	return 0;
}

void ka650_WriteIO(VAX_CPU *vax, int32 pAddr, uint32 data, int32 size)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)vax;
	IOMAP        *io;

	if (IN_IO(pAddr)) {
		cq_WriteIO(ka650->qba, pAddr, data, size);
		return;
	}

	for (io = &ioMap[0]; io->loAddr != 0; io++) {
		if ((pAddr >= io->loAddr) && (pAddr < io->hiAddr) && io->Write) {
			io->Write(io->Type ? ka650->qba : ka650, pAddr, data, size);
			return;
		}
	}

	// Non-Existant Memory Exception
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) CPU Non-Existant Memory (%08X) at PC %08X\n",
			ka650->cpu.devName, pAddr, faultPC);
#endif /* DEBUG */

	ka650->sscTimeout |= (SSCBTO_BTO|SSCBTO_RWT);

	// Go to machine check exception
	MACH_CHECK(MCHK_WRITE);
}

// *******************************************************
// ************* Console Read/Write Access ***************
// *******************************************************

// Console Read Aligned
int ka650_ReadC(VAX_CPU *vax, int32 pAddr, uint32 *data, int32 size)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)vax;
	IOMAP        *io;

	if (IN_RAM(pAddr)) {
		if (size >= LN_LONG)
			*data = LMEM(pAddr >> 2);
		else if (size == LN_WORD)
			*data = WMEM(pAddr >> 1);
		else
			*data = BMEM(pAddr);
		return MM_OK;
	} else if (IN_ROM(pAddr)) {
		pAddr &= vax->maskROM;
		if (size >= LN_LONG)
			*data = LROM(pAddr >> 2);
		else if (size == LN_WORD)
			*data = WROM(pAddr >> 1);
		else
			*data = BROM(pAddr);
		return MM_OK;
	}

	for (io = &ioMap[0]; io->loAddr != 0; io++) {
		if ((pAddr >= io->loAddr) && (pAddr < io->hiAddr) && io->Read) {
			*data = io->Read(ka650, pAddr);
			return MM_OK;
		}
	}

	*data = 0;

	return MM_NXM;
}

// Console Write Aligned
int ka650_WriteC(register VAX_CPU *vax, int32 pAddr, uint32 data, int32 size)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)vax;
	IOMAP        *io;

	if (IN_RAM(pAddr)) {
		if (size >= LN_LONG)
			LMEM(pAddr >> 2) = data;
		else if (size == LN_WORD)
			WMEM(pAddr >> 1) = data;
		else
			BMEM(pAddr) = data;
		return MM_OK;
	}

	for (io = &ioMap[0]; io->loAddr != 0; io++) {
		if ((pAddr >= io->loAddr) && (pAddr < io->hiAddr) && io->Write) {
			io->Write(ka650, pAddr, data, size);
			return MM_OK;
		}
	}

	return MM_NXM;
}

// *********************************************************************

void ka650_MachineCheck(register VAX_CPU *vax)
{
	uint32 p1,  p2;
	uint32 st1, st2;
	uint32 idx, hsir = 0;

	// Find highest software interrupt
	for (idx = 0; idx < 16; idx++)
		if ((SISR >> idx) & 1) hsir = idx;

	// Gather faulting information
	p1  = P1 | ((P1 & 0x80) ? MCHK_REF : 0);
	p2  = MCHK_ADDR + 4;
	st1 = ((OPC & 0xFF) << 24) | (hsir << 16) |
		   ((CADR & 0xFF) << 8) | (MSER & 0xFF);
	st2 = 0x00C07000 + ((faultPC - PC) & 0xFF);

	vax_DoIntexc(vax, SCB_MCHK, 0, IE_SVE);
	IN_IE = 1;
	SP -= 20;
	WriteV(SP,      16,  LN_LONG, WA);
	WriteV(SP + 4,  p1,  LN_LONG, WA);
	WriteV(SP + 8,  p2,  LN_LONG, WA);
	WriteV(SP + 12, st1, LN_LONG, WA);
	WriteV(SP + 16, st2, LN_LONG, WA);
	IN_IE = 0;
}

void ka650_HaltAction(register VAX_CPU *vax, uint32 reason)
{
	// Get IS'CUR value from PSL status
	register uint32 cur = PSL_GETISCUR(PSL);

#ifdef DEBUG
	dbg_Printf("KA650: Halt Action - Reason %d\n", reason);
#endif /* DEBUG */

	// Save current PC and PSL
	CONPC  = PC;
	CONPSL = (PSL | CC) | (reason << 8) |
		(MAPEN ? CONPSL_MAPEN : 0);

	if (cur > 4) CONPSL |= CONPSL_INVALID;
	else         PRN(cur) = SP;
	
	// Reset Processor Registers
	SP    = ISP;
	PC    = ROM_BASE;
	PSL   = PSL_IS | PSL_IPL1F;
	CC    = 0;
	MAPEN = 0;
	
	// Go Next Instruction
	FLUSH_ISTR;
	ABORT(NEXT);
}

// ******************************************************
// ************** CVAX Series Timer System **************
// ******************************************************
//
// The SCC timers, which with increment at 1 Mhz rate, cannot
// be accurate due to host-specific hardware limitation.
//
// Interval register contains value which is less than 10 ms,
// use instruction counter.  Otherwise, use 100 Hz real clock
// tick until less than 10 ms.

inline uint32 ka650_TimerWait(KA650_DEVICE *ka650, int tmr)
{
	// If interval is so small, then count instructions
	// Otherwise, continue until next tick...
	if (ka650->tir[tmr] > (LMASK - TMR_TICK))
		return (ka650->tir[tmr] ^ LMASK) + 1;
	return ka650->tmrTick; // Next tick
}

#if 0
uint32 ka650_TimerWait2(KA650_DEVICE *ka650, int tmr)
{
	// If interval is so small (less than 10ms), then
	// count instructions. Otherwise, continue
	// until next tick...
	if (ka650->tir[tmr] > (LMASK - TMR_TICK))
		return ~ks650->tir[tmr] + 1;
	return ka650->tmrTick;
}

int32 ka650_ReadTIR(KA650_DEVICE *ka650, int tmr, int flag)
{
	uint32 delta;

	if (flag || (ka650->tcsr[tmr] & TMR_CSR_RUN)) {
		delta = ts10_GetGlobalTime() - ka650->tmr_sav[tmr];
		if (delta >= ka650->tmr_inc[tmr])
			delta = ka650->tmr_inc[tmr] - 1;
		return ka650->tir[tmr] + delta;
	}
	return ka650->tir[tmr];
}
#endif

void ka650_TimerCount(KA650_DEVICE *ka650, int tmr, uint32 count)
{
	VAX_CPU *vax = (VAX_CPU *)ka650;
	MAP_IO  *io  = &ka650->ioTimer[tmr];
	uint32 newCount = ka650->tir[tmr] + count;

	if (newCount < ka650->tir[tmr]) {
		// Set done bit.
		ka650->tir[tmr] = 0;
		ka650->tcr[tmr] |=
			(ka650->tcr[tmr] & TCR_DONE) ? TCR_ERROR : TCR_DONE;

		// Reset interval timer unless stop request.
		if (ka650->tcr[tmr] & TCR_STOP)
			ka650->tcr[tmr] &= ~TCR_RUN;
		else {
			ka650->tir[tmr]    = ka650->tnir[tmr];
			ka650->Timers[tmr].nxtTimer = ka650_TimerWait(ka650, tmr);
			ts10_SetTimer(&ka650->Timers[tmr]);
		}

		// If interrupt enable is set, do timer interrupt.
		if (ka650->tcr[tmr] & TCR_IE)
			io->SendInterrupt(io, 0);
	} else {
		ka650->tir[tmr] = newCount;
		if (ka650->tcr[tmr] & TCR_RUN) {
			ka650->Timers[tmr].nxtTimer = ka650_TimerWait(ka650, tmr);
			ts10_SetTimer(&ka650->Timers[tmr]);
		}
	}
}

void  ka650_Timer0(void *dptr)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)dptr;

	ka650_TimerCount(ka650, 0, TMR_TICK);
}

void  ka650_Timer1(void *dptr)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)dptr;

	ka650_TimerCount(ka650, 1, TMR_TICK);
}

void ka650_WriteTCR(KA650_DEVICE *ka650, int tmr, uint32 data)
{
	MAP_IO *io = &ka650->ioTimer[tmr];

	ka650->tcr[tmr] &= ~(data & TCR_W1C);
	ka650->tcr[tmr] = (data & TCR_RW) | (ka650->tcr[tmr] & ~TCR_RW);

//	if (tmr && (data == 0x80000090))
//		dbg_SetMode(DBG_TRACE|DBG_DATA|DBG_INTERRUPT);

	if (data & TCR_TRANSFER)
		ka650->tir[tmr] = ka650->tnir[tmr];
	else if ((data & TCR_SINGLE) && !(data & TCR_RUN)) {
		ka650_TimerCount(ka650, tmr, 1);
		if (ka650->tir[tmr] == 0)
			ka650->tir[tmr] = ka650->tnir[tmr];
	}

	// If run bit is set, set up timers to count ticks.
	if (ka650->tcr[tmr] & TCR_RUN) {
		if (data & TCR_TRANSFER)
			ts10_CancelTimer(&ka650->Timers[tmr]);
		ka650->Timers[tmr].nxtTimer = ka650_TimerWait(ka650, tmr);
		ts10_SetTimer(&ka650->Timers[tmr]);
	} else
		ts10_CancelTimer(&ka650->Timers[tmr]);

	if ((ka650->tcr[tmr] & (TCR_IE|TCR_DONE)) != (TCR_IE|TCR_DONE))
		io->CancelInterrupt(io, 0);

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) TCR%d %08X <= %08X\n",
			ka650->cpu.devName, tmr, ka650->tcr[tmr], data);
#endif /* DEBUG */

#ifdef DEBUG
//	dbg_Printf("Timer %d: TCR = %08X TIR = %08X TNIR = %08X TIVR = %08X\n", tmr,
//		ka650->tcr[tmr], ka650->tir[tmr], ka650->tnir[tmr], ka650->tivr[tmr]);
#endif /* DEBUG */
}

// *****************************************************************

void ka650_ResetTODR(VAX_CPU *vax)
{
	time_t now    = time(NULL);
	struct tm *tm = localtime(&now);
	uint32 yrsec;

	// Calcuate seconds since January 1 of this year.
	yrsec = (((((tm->tm_yday * 24) +
	             tm->tm_hour) * 60) +
	             tm->tm_min) * 60) +
	             tm->tm_sec;

	// Finally, set up TODR register.
	TODR = (yrsec * TODR_SEC) + TODR_BASE;
}

// Each tick = 10 microseconds.
void ka650_ClockTimer(void *dptr)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)dptr;
	VAX_CPU      *vax   = (VAX_CPU *)ka650;
	MAP_IO       *io    = &ka650->ioClock;
	static int   bips   = 0;

	// Increment Time of Day Register by one each 10 microseconds.
	TODR++;

	// Clock interrupt each tick if enabled.
	if (ICCS & ICCS_IE)
		io->SendInterrupt(io, 0);

	ka650->tmrTick = (vax->ips - bips) / 10;
	bips = vax->ips;

	// Each second
	if (ka650->TickCount++ >= ICCS_SECOND) {
		ka650->TickCount = 0;
#ifdef DEBUG
		if (dbg_Check(DBG_IPS))
			dbg_Printf("%s: %d ips\n", ka650->cpu.devName, vax->ips);
#else 
		printf("%s: %d ips\n", ka650->cpu.devName, vax->ips);
#endif /* DEBUG */
//		ka650->tmrTick = vax->ips / TMR_HZ;
		vax->ips = 0;
		bips     = 0;
	}
}

// Enable System Timer
void ka650_StartTimer(void *dptr)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)dptr;

	ka650->TickCount         = 0;
	ka650->ClockTimer.Flags |= CLK_ENABLE;
}

// Disable System Timer
void ka650_StopTimer(void *dptr)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)dptr;

	ka650->ClockTimer.Flags &= ~CLK_ENABLE;
}

void ka650_InitIO(VAX_CPU *vax)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)vax;
	UQ_CALL      *cb    = ka650->Callback;
	MAP_IO       *io;

	// Set up clock interrupts
	io               = &ka650->ioClock;
	io->devName      = "CLK";
	io->keyName      = "CLK";
	io->emuName      = "KA650: Clock Timer";
	io->emuVersion   = NULL;
	io->nVectors     = ICCS_NVECS;
	io->intIPL       = ICCS_IPL;
	io->intVector[0] = ICCS_VEC;
	cb->SetMap(ka650->qba, io);

	// Set up DL11 interrupts (console terminal)
	if (ka650->cpu.Console)
		vax_ConsoleInit2(ka650->cpu.Console, cb);

	// Set up timer #1 interrupts
	io               = &ka650->ioTimer[0];
	io->devName      = "TMR0";
	io->keyName      = "TMR0";
	io->emuName      = "KA650: Prog Timer #1";
	io->emuVersion   = NULL;
	io->nVectors     = TMR0_NVECS;
	io->intIPL       = TMR0_IPL;
	io->Priority     = TMR0_PRIO;
	cb->SetMap(ka650->qba, io);

	// Set up timer #2 interrupts
	io               = &ka650->ioTimer[1];
	io->devName      = "TMR1";
	io->keyName      = "TMR1";
	io->emuName      = "KA650: Prog Timer #2";
	io->emuVersion   = NULL;
	io->nVectors     = TMR1_NVECS;
	io->intIPL       = TMR1_IPL;
	io->Priority     = TMR1_PRIO;
	cb->SetMap(ka650->qba, io);
}

// **************************************************************
// ************** CVAX Series System Configuration **************
// **************************************************************

void *ka650_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	KA650_DEVICE *ka650 = NULL;
	VAX_SYSTEM   *vaxsys;
	VAX_CPU      *vax;
	CLK_QUEUE    *Timer;

	if (ka650 = (KA650_DEVICE *)calloc(1, sizeof(KA650_DEVICE))) {
		// First, link it to VAX system device.
		vaxsys             = (VAX_SYSTEM *)newMap->sysDevice;
		ka650->cpu.System  = vaxsys;
		ka650->cpu.Console = vax_ConsoleInit((VAX_CPU *)ka650);
		vaxsys->Processor  = (VAX_CPU *)ka650;

		// Build instruction table
		vax_BuildCPU((VAX_CPU *)ka650, 0);
		ka650_ResetCPU(ka650);

		// Set maximum memory space (Up to 64MB RAM).
		ka650->cpu.ramMaxSize = RAM_SIZE;

		// Set up system timer
		Timer           = &ka650->ClockTimer;
		Timer->Next     = NULL;
		Timer->Flags    = CLK_REACTIVE;
		Timer->outTimer = ICCS_TICK;
		Timer->nxtTimer = ICCS_TICK;
		Timer->Device   = ka650;
		Timer->Execute  = ka650_ClockTimer;

		// Now enable clock timer (on host timer)
		vax = (VAX_CPU *)ka650;
		ts10_SetRealTimer(Timer);
	
		// Set up programmable timers
		Timer           = &ka650->Timers[0];
		Timer->Next     = NULL;
		Timer->Flags    = 0;
		Timer->outTimer = 0;
		Timer->nxtTimer = 0;
		Timer->Device   = ka650;
		Timer->Execute  = ka650_Timer0;

		Timer           = &ka650->Timers[1];
		Timer->Next     = NULL;
		Timer->Flags    = 0;
		Timer->outTimer = 0;
		Timer->nxtTimer = 0;
		Timer->Device   = ka650;
		Timer->Execute  = ka650_Timer1;

		// Set up functions for KA650/KA655 Processor
		ka650->cpu.ReadRegister   = ka650_ReadRegister;
		ka650->cpu.WriteRegister  = ka650_WriteRegister;
		ka650->cpu.ReadAligned    = ka650_ReadIO;
		ka650->cpu.WriteAligned   = ka650_WriteIO;
		ka650->cpu.ReadCA         = ka650_ReadC;
		ka650->cpu.WriteCA        = ka650_WriteC;
		ka650->cpu.HaltAction     = ka650_HaltAction;
		ka650->cpu.MachineCheck   = ka650_MachineCheck;
		ka650->cpu.ResetClock     = ka650_ResetTODR;
		ka650->cpu.StartTimer     = ka650_StartTimer;
		ka650->cpu.StopTimer      = ka650_StopTimer;

		// Set up Unibus/Qbus function calls
		ka650->cpu.InitIO         = ka650_InitIO;

		// Finally, Set up its descriptions and
		// link it to its mapping device.
		ka650->cpu.devName    = newMap->devName;
		ka650->cpu.keyName    = newMap->keyName;
		ka650->cpu.emuName    = newMap->emuName;
		ka650->cpu.emuVersion = newMap->emuVersion;
		newMap->Device        = ka650;
#ifdef DEBUG
		newMap->Breaks        = &ka650->cpu.Breaks;
#endif /* DEBUG */
	}

	return (void *)ka650;
}

void ka650_Reset(void *dptr)
{
	ka650_ResetCPU(dptr);
}

int ka650_Boot(MAP_DEVICE *map, int argc, char **argv)
{
	KA650_DEVICE *ka650 = (KA650_DEVICE *)map->Device;
	VAX_CPU      *vax   = (VAX_CPU *)ka650;

	// Tell operator that processor is being booted.
	printf("Booting %s: (%s)... ", ka650->cpu.devName, ka650->cpu.keyName);

	// Initialize KA650 Processor
	vax->State = VAX_HALT;
	ka650_Reset(ka650);

	// Initial Power On
	SP      = 0x00000000;
	PSL     = PSL_IS | PSL_IPL1F;
	PC      = ROM_BASE;
	CONPC   = 0;
	CONPSL  = PSL_IS | PSL_IPL1F | CON_PWRUP;
	CC      = 0;
	MAPEN   = 0;
	ICCS    = 0;
	MSER    = 0;
	CADR    = 0;
	SISR    = 0;
	ASTLVL  = 0;

	ka650->bdr    = 0x80;
	ka650->sscLED = 0xF; // Power-On Status

	// Enable KA650 Processor to run.
	vax->State = VAX_RUN;
	emu_State  = VAX_RUN;

	// Tell operator that it was done.
	printf("done.\n");

	return TS10_OK;
}

extern DEVICE cq_Device;
extern COMMAND ka650_Commands[];
extern COMMAND ka650_SetCommands[];
extern COMMAND ka650_ShowCommands[];

DEVICE *ka650_Devices[] =
{
	&cq_Device, // CQBIC Q22-Bus Interface
	NULL        // Terminator
};

// CVAX Series System Definition
DEVICE vax_System_KA650 =
{
	// Description Information
	KA650_KEY,             // Device Type Name
	KA650_NAME,            // Emulator Name
	KA65X_VERSION,         // Emulator Version

	ka650_Devices,         // KA650 Device List
	DF_USE|DF_SYSMAP,      // Device Flags
	DT_PROCESSOR,          // Device Type

	ka650_Commands,        // Command Table
	ka650_SetCommands,     // Set Command Table
	ka650_ShowCommands,    // Show Command Table

	ka650_Create,          // Create Routine
	NULL,                  // Configure Routine
	NULL,                  // Delete Routine
	NULL,                  // Reset Routine
	NULL,                  // Attach Routine
	NULL,                  // Detach Routine
	NULL,                  // Info Routine
	ka650_Boot,            // Boot Routine
	NULL,                  // Execute Routine
#ifdef DEBUG
	NULL,                  // Debug Routine
#endif /* DEBUG */
};

DEVICE vax_System_KA655 =
{
	// Description Information
	KA655_KEY,             // Device Type Name
	KA655_NAME,            // Emulator Name
	KA65X_VERSION,         // Emulator Version

	ka650_Devices,         // KA655 Device List
	DF_USE|DF_SYSMAP,      // Device Flags
	DT_PROCESSOR,          // Device Type

	ka650_Commands,        // Command Table
	ka650_SetCommands,     // Set Command Table
	ka650_ShowCommands,    // Show Command Table

	ka650_Create,          // Create Routine
	NULL,                  // Configure Routine
	NULL,                  // Delete Routine
	NULL,                  // Reset Routine
	NULL,                  // Attach Routine
	NULL,                  // Detach Routine
	NULL,                  // Info Routine
	ka650_Boot,            // Boot Routine
	NULL,                  // Execute Routine
#ifdef DEBUG
	NULL,                  // Debug Routine
#endif /* DEBUG */
};
