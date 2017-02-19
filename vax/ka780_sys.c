// ka780_sys.c - VAX-11/780 System Configurations and Routines
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

// VAX-11/780 Memory Map
//
//            +-----------------------------------------------------+
// 0000 0000: |                  Installed Memory                   |
//     :      |- - - - - - - - - - - - - - - - - - - - - - - - - - -|
// 1FFF FFFF: |                 Up to 512MB Memory                  |
//            +-----------------------------------------------------+   
// 2000 0000: |                  TR0 Adapter Space                  |
//            +-----------------------------------------------------+
// 2000 2000: |                  TR1 Adapter Space                  |
//            |                          :                          |
// 2001 E000: |                  TR15 Adapter Space                 |
//            +-----------------------------------------------------+
// 2002 0000: |                                                     |
//     :      |                      Reserved                       |
// 200F FFFF: |                                                     |
//            +-----------------------------------------------------+
// 2010 0000: |                UNIBUS 0 Address Space               |
//            +-----------------------------------------------------+
// 2014 0000: |                UNIBUS 1 Address Space               |
//            +-----------------------------------------------------+
// 2018 0000: |                UNIBUS 2 Address Space               |
//            +-----------------------------------------------------+
// 201C 0000: |                UNIBUS 3 Address Space               |
//            +-----------------------------------------------------+
// 2020 0000: |                                                     |
//     :      |                      Reserved                       |
// 3FFF FFFF: |                                                     |
//            +-----------------------------------------------------+

#include <time.h>
#include "vax/ka780.h"

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
	"ACCS",    //       28 *Accelerator Control and Status
	"ACCR",    //       29 *Accelerator Maintenance
	NULL,      //       2A Unknown
	NULL,      //       2B Unknown
	"WCSA",    //       2C *Writable-Control-Store Address
	"WCSD",    //       2D *Writable-Control-Store Data
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

// ***************************************************************

// Read Privileged Register
int32 ka780_ReadRegister(register VAX_CPU *vax, int32 pReg)
{
	int32 data;

	switch (pReg) {
		case nKSP:
			data = (PSL & PSL_IS) ? KSP : SP;
			break;

		case nISP:
			data = (PSL & PSL_IS) ? SP : ISP;
			break;

		case nIPL:
			IPL = data = PSL_GETIPL(PSL);
			break;

		case nRXCS:
			data = vax_ReadRXCS(vax);
			break;

		case nRXDB:
			data = vax_ReadRXDB(vax);
			break;

		case nTXCS:
			data = vax_ReadTXCS(vax);
			break;

//		case nTXDB:
//			data = vax_ReadTXDB(vax);
//			break;

		default:
			data = PRN(pReg);
			break;

/*
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA)) {
				dbg_Printf("KA780: (R) Unknown privileged register: %08X (%d)\n",
					pReg, pReg);
			}
#endif // DEBUG
			RSVD_OPND_FAULT;
*/
	}

#ifdef DEBUG
//	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
	{
		char *name = "Undefined Register";
		if ((pReg < regLength) && regNames[pReg])
			name = regNames[pReg];
		dbg_Printf("KA780: (R) %s (%02X) => %08X\n", name, pReg, data);
	}
#endif /* DEBUG */

	return data;
}

void ka780_WriteRegister(register VAX_CPU *vax, int32 pReg, int32 data)
{
#ifdef DEBUG
//	if (dbg_Check(DBG_TRACE|DBG_DATA)) {
	{
		char *name = "Undefined Register";
		if ((pReg < regLength) && regNames[pReg])
			name = regNames[pReg];
		dbg_Printf("KA780: (W) %s (%02X) <= %08X\n", name, pReg, data);
	}
#endif /* DEBUG */

	switch (pReg) {
		case nKSP: // Kernel Stack Pointer
			((PSL & PSL_IS) ? KSP : SP) = data;
			break;

		case nESP: // Executive Stack Pointer
		case nSSP: // Supervisor Stack Pointer
		case nUSP: // User Stack Pointer
			PRN(pReg) = data;
			break;

		case nISP: // Interrupt Stack Pointer
			((PSL & PSL_IS) ? SP : ISP) = data;
			break;

		case nP0BR:
		case nP1BR:
		case nSBR:
			PRN(pReg) = data & BR_MASK;
			vax_ClearTBTable(vax, pReg == nSBR);
			break;

		case nP0LR:
		case nP1LR:
		case nSLR:
			PRN(pReg) = data & LR_MASK;
			vax_ClearTBTable(vax, pReg == nSLR);
			break;
		
		case nPCBB:
		case nSCBB:
			PRN(pReg) = data & LALIGN;
			break;

		case nIPL:
			IPL = data & PSL_M_IPL;
			PSL = PSL_PUTIPL(IPL) | (PSL & ~PSL_IPL);
			break;

		case nSIRR:
			if ((data > 0xF) || (data == 0))
				RSVD_OPND_FAULT;
			SISR |= (1 << data);
			break;

		case nSISR:
			SISR = data & SISR_MASK;
			break;

		case nASTLVL:
			if ((uint32)data > AST_MAX)
				RSVD_OPND_FAULT;
			ASTLVL = data;
			break;

/*
		case nICCS:
			if (((ICCS & ICCS_RUN) == 0) && (data & ICCS_STEP))
				ICR++
			if (data & ICCS_XFER)
				ICR = NICR;
			ICCS = (ICCS & ~ICCS_WMASK) | data & ICCS_WMASK;
			ICCS &= ~(data & ICCS_CMASK);
			break;

		case nNICR:
		case nICR:
			PRN(pReg) = data;
			break;
*/

		case nRXCS:
//			if (data & RXCS_MBZ)
//				RSVD_OPND_FAULT;
			vax_WriteRXCS(vax, data);
			break;

//		case nRXDB:
//			vax_WriteRXDB(vax, data);
//			break;

		case nTXCS:
//			if (data & TXCS_MBZ)
//				RSVD_OPND_FAULT;
			vax_WriteTXCS(vax, data);
			break;

		case nTXDB:
			vax_WriteTXDB(vax, data);
			break;

		case nMAPEN:
			MAPEN = data & 1;
			break;

		case nTBIA:
			vax_ClearTBTable(vax, 1);
			break;

		case nTBIS:
			vax_ClearTBEntry(vax, data);
			break;

		case nTBCHK:
			if (vax_CheckTBEntry(vax, data))
				PSL |= CC_V;
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_TRACE|DBG_DATA)) {
				dbg_Printf("KA780: (W) Unknown privileged register: %08X (%d)\n",
					pReg, pReg);
			}
#endif /* DEBUG */
//			RSVD_OPND_FAULT;
	}
}

// *****************************************************************

#if 0
static IOMAP ioMap[] =
{
	{ NEX_BASE, NEX_END, 0, ka780_ReadNexus, ka780_WriteNexus },
	{ UBA_BASE, UBA_END, 0, ka780_ReadUBA,   ka780_WriteUBA   },
	{ 0 }; // Null Terminator
};
#endif

uint32 ka780_ReadIO(VAX_CPU *vax, int32 pAddr, int32 size)
{
	KA780_DEVICE *ka780 = (KA780_DEVICE *)vax;

#if 0
	for (io = &ioMap[0]; io->loAddr != 0; io++) {
		if ((pAddr >= io->loAddr) && (pAddr < io->hiAddr) && io->Read) {
			data = io->Read(ka780, pAddr);
			if (size >= LN_LONG)
				return data;
			else if (size == LN_WORD)
				return (data >> ((pAddr & 2) ? 16 : 0)) & WMASK;
			else
				return (data >> ((pAddr & 3) << 3)) & BMASK;
		}
	}
#endif

	// Non-existant memory
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) CPU Non-Existant Memory (%08X) at PC %08X\n",
			ka780->cpu.devName, pAddr, faultPC);
#endif /* DEBUG */
	return 0;
}

void ka780_WriteIO(VAX_CPU *vax, int32 pAddr, uint32 data, int32 size)
{
	KA780_DEVICE *ka780 = (KA780_DEVICE *)vax;

#if 0
	for (io = &ioMap[0]; io->loAddr != 0; io++) {
		if ((pAddr >= io->loAddr) && (pAddr < io->hiAddr) && io->Write) {
			io->Write(ka780, pAddr, data, size);
			return;
		}
	}
#endif

	// Non-existant memory
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (W) CPU Non-Existant Memory (%08X) at PC %08X\n",
			ka780->cpu.devName, pAddr, faultPC);
#endif /* DEBUG */
}

// ******************************************************
// ************** Console Read/Write Access *************
// ******************************************************

// Console Read Aligned
int ka780_ReadC(VAX_CPU *vax, int32 pAddr, uint32 *data, int32 size)
{
	if (IN_RAM(pAddr)) {
		if (size >= OP_LONG)
			*data = LMEM(pAddr >> 2);
		else if (size == OP_WORD)
			*data = WMEM(pAddr >> 1);
		else
			*data = BMEM(pAddr);
		return MM_OK;
	}
	*data = 0;
	return MM_NXM;
}

// Console Write Aligned
int ka780_WriteC(VAX_CPU *vax, int32 pAddr, uint32 data, int32 size)
{
	if (IN_RAM(pAddr)) {
		if (size >= OP_LONG)
			LMEM(pAddr >> 2) = data;
		else if (size == OP_WORD)
			WMEM(pAddr >> 1) = data;
		else
			BMEM(pAddr) = data;
		return MM_OK;
	}
	return MM_NXM;
}

// ******************************************************
// ************** VAX-11/780 Timer System ***************
// ******************************************************

void ka780_ResetTODR(VAX_CPU *vax)
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
void ka780_ClockTimer(void *dptr)
{
	KA780_DEVICE *ka780 = (KA780_DEVICE *)dptr;
	VAX_CPU      *vax   = (VAX_CPU *)ka780;

	// Increment Time of Day Register by one each 10 microseconds.
	TODR++;

	// Clock interrupt each tick if enabled.
	if (ICCS & ICCS_IE) {
		HIRQ |= ICCS_INT;
		SET_IRQ;
	}

	// Each second
	if (ka780->TickCount++ >= ICCS_SECOND) {
		ka780->TickCount = 0;
#ifdef DEBUG
		if (dbg_Check(DBG_IPS))
			dbg_Printf("%s: %d ips\n", ka780->cpu.devName, vax->ips);
#else 
		printf("%s: %d ips\n", ka780->cpu.devName, vax->ips);
#endif /* DEBUG */
		vax->ips = 0;
	}
}

// Enable System Timer
void ka780_StartTimer(void *dptr)
{
	KA780_DEVICE *ka780 = (KA780_DEVICE *)dptr;

	ka780->TickCount         = 0;
	ka780->ClockTimer.Flags |= CLK_ENABLE;
}

// Disable System Timer
void ka780_StopTimer(void *dptr)
{
	KA780_DEVICE *ka780 = (KA780_DEVICE *)dptr;

	ka780->ClockTimer.Flags &= ~CLK_ENABLE;
}

// **************************************************************
// ************** VAX-11/780 System Configuration ***************
// **************************************************************

void ka780_ResetCPU(KA780_DEVICE *ka780)
{
	VAX_CPU *vax = (VAX_CPU *)ka780;

//	TXCS = TXCS_RDY;
	SID  = KA780_SYSID;
}

void *ka780_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	KA780_DEVICE *ka780 = NULL;
	VAX_SYSTEM   *vaxsys;
	VAX_CPU      *vax;
	CLK_QUEUE    *Timer;

	if (ka780 = (KA780_DEVICE *)calloc(1, sizeof(KA780_DEVICE))) {
		// Set up its descriptions.
		ka780->cpu.devName    = newMap->devName;
		ka780->cpu.keyName    = newMap->keyName;
		ka780->cpu.emuName    = newMap->emuName;
		ka780->cpu.emuVersion = newMap->emuVersion;

		// Link it to VAX system device.
		vaxsys             = (VAX_SYSTEM *)newMap->sysDevice;
		ka780->cpu.System  = vaxsys;
		ka780->cpu.Console = vax_ConsoleInit((VAX_CPU *)ka780);
		vaxsys->Processor  = (VAX_CPU *)ka780;

		// Build instruction table
		vax_BuildCPU((VAX_CPU *)ka780, 0);
		ka780_ResetCPU(ka780);

		// ACTION: Implement a command later.
		vax_InitMemory((VAX_CPU *)ka780, 32768 * 1024);

		// Set up system timer
		Timer           = &ka780->ClockTimer;
		Timer->Next     = NULL;
		Timer->Flags    = CLK_REACTIVE;
		Timer->outTimer = ICCS_TICK;
		Timer->nxtTimer = ICCS_TICK;
		Timer->Device   = ka780;
		Timer->Execute  = ka780_ClockTimer;

		// Now enable clock timer (on host timer)
		vax = (VAX_CPU *)ka780;
		INTVEC[nICCS_INT] = SCB_TIMER;
		ts10_SetRealTimer(Timer);

		// Set up functions for KA780 CPU (VAX-11/780)
		ka780->cpu.ReadRegister   = ka780_ReadRegister;
		ka780->cpu.WriteRegister  = ka780_WriteRegister;
		ka780->cpu.ReadAligned    = ka780_ReadIO;
		ka780->cpu.WriteAligned   = ka780_WriteIO;
		ka780->cpu.ReadCA         = ka780_ReadC;
		ka780->cpu.WriteCA        = ka780_WriteC;
		ka780->cpu.ResetClock     = ka780_ResetTODR;
		ka780->cpu.StartTimer     = ka780_StartTimer;
		ka780->cpu.StopTimer      = ka780_StopTimer;

		// Finally, link it to mapping device and return.
		newMap->Device = ka780;
#ifdef DEBUG
		newMap->Breaks = &ka780->cpu.Breaks;
#endif /* DEBUG */
	}

	return ka780;
}

void ka780_Cleanup(VAX_CPU *vax)
{
}


int ka780_Reset(void *dptr)
{
	ka780_ResetCPU(dptr);
	return VAX_OK;
}

int ka780_Boot(MAP_DEVICE *map, int argc, char **argv)
{
	KA780_DEVICE *ka780 = (KA780_DEVICE *)map->Device;
	VAX_CPU      *vax   = (VAX_CPU *)ka780;
	char         *fileName;
	uint32       sAddr = 0x1000, eAddr;

	// Tell operator that processor is being booted.
	printf("Booting %s: (%s)...",
		ka780->cpu.devName, ka780->cpu.keyName);

	// Load system file (VMB.EXE) into main memory if request.
//	if (argc > 2) {
//		RemoveSpaces(argv[2]);
//
//		if (argc == 4)
//			sscanf(argv[3], "%x", &sAddr);
//
//		if (vax_LoadFile(vax, argv[2], sAddr, &eAddr)) {
//			printf("failed.\n");
//			return VAX_OK;
//		}
//	}

	// Reset CPU system first
	vax->State = VAX_HALT;
	ka780_ResetCPU(ka780);

	// Initialize KA780 Processor
	R0     = 0;
	R1     = 0;
	R2     = 0;
	R3     = 0;
	R4     = 0;
	R5     = 0;
	R6     = 0;
	R7     = 0;
	R8     = 0;
	R9     = 0;
	R10    = 0;
	R11    = 0;
	AP     = 0;
	FP     = 0;
	SP     = sAddr;
	PC     = sAddr;

	IPL    = 31;
	ASTLVL = 4;
	PSL    = 0x04150000;

	// Enable KA780 Processor to execute.
	vax->State = VAX_RUN;
	emu_State  = VAX_RUN;

	// Tell operator that it was done.
	printf("done.\n");

	return VAX_OK;
}

DEVICE *ka780_Devices[] =
{
	NULL // Terminator
};

// VAX-11/780 System Definition
DEVICE vax_System_KA780 =
{
	KA780_KEY,         // Key Name
	KA78X_NAME,        // Emulator Name
	KA78X_VERSION,     // Emulator Version

	ka780_Devices,     // KA780 Device List
	DF_USE|DF_SYSMAP,  // Device Flags
	DT_PROCESSOR,      // Device Type

	NULL, NULL, NULL,

	ka780_Create,      // Create Routine
	NULL,              // Configure Routine
	NULL,              // Delete Routine
	ka780_Reset,       // Reset Routine
	NULL,              // Attach Routine
	NULL,              // Detach Routine
	NULL,              // Info Routine
	ka780_Boot,        // Boot Routine
	NULL,              // Execute Routine
#ifdef DEBUG
	NULL,              // Debug Routine
#endif /* DEBUG */
};

// VAX-11/785 System Definition
DEVICE vax_System_KA785 =
{
	KA785_KEY,         // Key Name
	KA78X_NAME,        // Emulator Name
	KA78X_VERSION,     // Emulator Version

	ka780_Devices,     // KA780 Device List
	DF_USE|DF_SYSMAP,  // Device Flags
	DT_PROCESSOR,      // Device Type

	NULL, NULL, NULL,

	ka780_Create,      // Create Routine
	NULL,              // Configure Routine
	NULL,              // Delete Routine
	ka780_Reset,       // Reset Routine
	NULL,              // Attach Routine
	NULL,              // Detach Routine
	NULL,              // Info Routine
	ka780_Boot,        // Boot Routine
	NULL,              // Execute Routine
#ifdef DEBUG
	NULL,              // Debug Routine
#endif /* DEBUG */
};
