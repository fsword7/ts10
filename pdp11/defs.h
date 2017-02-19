// defs.h - PDP-11 Series Configuration
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

#define PDP11_KEY     "PDP11"
#define PDP11_NAME    "PDP-11 Emulator System"
#define PDP11_VERSION "v0.7 (Alpha)"

#include "emu/defs.h"

#define CNF_ODDTRAP 0x00000001 // Optional Odd Address Trap (J11 Only)

#define NUM_INST 65536   // Number of Instructions
#define OPC_BYTE 0100000 // Byte Bit

#define MAX_POS  ((1 << 15) - 1)
#define MAX_NEG  -(1 << 15)

#define OP_CIS    04000  // Commerical Instruction Set
#define OP_FIS    02000  // Floating Instruction Set
#define OP_EIS    01000  // Extended Instruction Set
#define OP_FLOAT  00400  // Floating-Point Type
#define OP_TYPE   00377  // Operand Type

#define OP_AFOP   15
#define OP_ASMD   14
#define OP_ASOP   13
#define OP_FOP    12
#define OP_CCC    11    // Cxx Instruction
#define OP_SCC    10    // Sxx Instruction
#define OP_REG    9
#define OP_8BIT   8
#define OP_6BIT   7
#define OP_3BIT   6
#define OP_SOB    5     // SOB Instruction
#define OP_BR     4     // Branch Instruction
#define OP_RSOP   3
#define OP_DOP    2     // Instruction with two operands
#define OP_SOP    1     // Instruction with one operand
#define OP_NPN    0     // Instruction with no operands

#define OP_FOP_F   OP_FOP
#define OP_FOP_D   OP_FOP
#define OP_AFOP_F  OP_AFOP
#define OP_AFOP_D  OP_AFOP
#define OP_ASMD_F  OP_ASMD
#define OP_ASMD_D  OP_ASMD
#define OP_ASMD_FL OP_ASMD
#define OP_ASMD_DL OP_ASMD

#define OP_BYTE   1
#define OP_WORD   2

#define P11_OK     EMU_OK
#define P11_NXM    EMU_NXM
#define P11_RUN    EMU_RUN
#define P11_HALT   EMU_HALT
#define P11_SWHALT EMU_SWHALT

// Processor Type for MTPT Instruction
#define PID_1144 1 // PDP-11/44
#define PID_UNK  2
#define PID_F11  3 // DCF11/KDF11
#define PID_T11  4 // DCT11
#define PID_J11  5 // DCJ11/KDJ11

// MAINT - Maintenance Register
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |  USER ADDRESS   |UB|FO|     ID    |HO|PWUP |OK|
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define CPU_USER  0176000 // User Start Address (Boot Address)
#define CPU_UB    0001000 // Unibus Adaptor (0 = QB, 1 = UB)
#define CPU_FPA   0000400 // Floating Point Adaptor Option
#define CPU_SID   0000360 // System Type (ID)
#define CPU_HALT  0000010 // Halt Option
#define CPU_PWUP  0000006 // Power Up Option
#define CPU_PWOK  0000001 // Power Ok

// Power-Up Options
#define PUO_MASK  3 // Power-Up Option Mask
#define PUO_POS   1 // Power-Up Option Position

#define PUO_LOC24 0 // Load PC from loc 24, and PSW from loc 26.
#define PUO_ODT   1 // Enter Micro-ODT Mode (NI)
#define PUO_BOOT  2 // Load PC as 173000 and PSW as 340.
#define PUO_USER  3 // Load PC from MAINT register and PSW as 340.

// MAINT - System ID Field 
#define SID_MASK 017   // SID Mask
#define SID_POS  4     // SID Position

#define SID_KDJ11A  1  // KDJ11A for PDP-11/73
#define SID_KDJ11B  2  // KDJ11B for PDP-11/73,83,84
#define SID_KDJ11X  3  // KDJ11X for Co-processor
#define SID_KDJ11D  4  // KDJ11D for PDP-11/53
#define SID_KDJ11E  5  // KDJ11E for PDP-11/93,94

// CPU - Error Register
#define CPUE_HALT   0000200 // Illegal HALT (Not Kernel or HTRAP option)
#define CPUE_ODD    0000100 // Address Error
#define CPUE_NXM    0000040 // Memory Non-Existant
#define CPUE_TMO    0000020 // I/O Page NXM
#define CPUE_YEL    0000010 // Yellow Stack Error
#define CPUE_RED    0000004 // Red Stack Error

// PIRQ - Program Interrupt Request Register (777772)
#define PIRQ_PIR7   0100000 // Program Interrupt Request #7
#define PIRQ_PIR6   0040000 // Program Interrupt Request #6
#define PIRQ_PIR5   0020000 // Program Interrupt Request #5
#define PIRQ_PIR4   0010000 // Program Interrupt Request #4
#define PIRQ_PIR3   0004000 // Program Interrupt Request #3
#define PIRQ_PIR2   0002000 // Program Interrupt Request #2
#define PIRQ_PIR1   0001000 // Program Interrupt Request #1
#define PIRQ_RW     0177000 // Read/Write Bits

// MMR0 - Memory Management Register #0
#define MMR0_FREEZE 0160000 // No updates if any error bit set
#define MMR0_NR     0100000 // Non-Resident Error
#define MMR0_PL     0040000 // Page Length Error
#define MMR0_RO     0020000 // Read-Only Error
#define MMR0_MAI    0000400 // Maintenance (Not Implemented)
#define MMR0_MODE   0000140 // Mode from PSW at during abort
#define MMR0_SPACE  0000020 // Address Space during abort
#define MMR0_PAGE   0000016 // Aborted Page Number
#define MMR0_MME    0000001 // Memory Management Enable
#define MMR0_RW     0160001 // Writable Bits

#define MMR0_M_PAGE (MMR0_MODE|MMR0_SPACE|MMR0_PAGE)
#define MMR0_P_PAGE 1

// MMR3 - Memory Management Register #3
#define MMR3_BME    0000040 // DMA Bus Map Enable
#define MMR3_M22E   0000020 // 22-bit Memory Management Enable
#define MMR3_CSM    0000010 // CSM Instruction Enable
#define MMR3_KDS    0000004 // Kernel Data Space Enable
#define MMR3_SDS    0000002 // Supervisor Data Space Enable
#define MMR3_UDS    0000001 // User Data Space Enable
#define MMR3_RW     0000077 // Writable Bits

// PAR - Page Address Register
// PDR - Page Descriptor Register
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |             Page Address Register             |
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
// |BY|       PLF          | 0|W | 0  0|ED| ACF | 0|
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define PDR_BY      0100000 // Bypass Cache Bit
#define PDR_PLF     0077400 // Page Length Field
#define PDR_W       0000100 // Written Flag
#define PDR_ED      0000010 // Expansion Direction
#define PDR_ACF     0000006 // Access Control Field
#define PDR_PWR     0000004 // Page Writable
#define PDR_PRD     0000002 // Page Readable
#define PDR_RW      0177416 // Writable Bits

// PA - Physical Address
#define PA_SIZE16   (1u << 16)      // 16-bit physical addressing
#define PA_MASK16   (PA_SIZE16 - 1)
#define PA_IOPAGE16 (PA_MASK16 ^ IO_MASK)
#define PA_SIZE18   (1u << 18)      // 18-bit physical addressing
#define PA_MASK18   (PA_SIZE18 - 1)
#define PA_IOPAGE18 (PA_MASK18 ^ IO_MASK)
#define PA_SIZE22   (1u << 22)      // 22-bit physical addressing
#define PA_MASK22   (PA_SIZE22 - 1)
#define PA_IOPAGE22 (PA_MASK22 ^ IO_MASK)

#define IO_SIZE     (1u << 13)      // 13-bit I/O addressing
#define IO_MASK     (IO_SIZE - 1)
#define IO_PAGEBASE PA_IOPAGE22
#define IO_PAGE16   017600000
#define IO_PAGE18   017000000

// VA - Virtual Address
#define VA_SIZE     (1u << 16)      // 16-bit virtual addressing
#define VA_MASK     (VA_SIZE - 1)

#define VA_DF       0017777
#define VA_BN       0017700
#define VA_DS       (1u << VA_P_DS)
#define VA_INST     (1u << VA_P_INST) // Instruction Fetch Access
#define VA_P_APF    13
#define VA_P_DS     16
#define VA_P_MODE   17
#define VA_P_INST   19

#define STKLIM      0400 // Stack Limit

// Access Mode (Protection Codes)
#define AM_KERNEL  0  // Kernel Mode
#define AM_SUPER   1  // Supervisor Mode
#define AM_ILLEGAL 2  // Illegal Mode
#define AM_USER    3  // User Mode

// PSW - Processor Status Register
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// | CM  | PM  |RS|0  0 |SI|   IPL  |T |N |Z |V |C |
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define PSW_CM  0140000 // Current Mode
#define PSW_PM  0030000 // Previous Mode
#define PSW_RS  0004000 // General Register Set     (J11 only)
#define PSW_SI  0000400 // Supsended Information    (F11 only)
#define PSW_IPL 0000340 // Interrupt Priority Level
#define PSW_T   0000020 // Trace/Trap Mode
#define PSW_N   0000010 // Negative
#define PSW_Z   0000004 // Zero
#define PSW_V   0000002 // Overflow
#define PSW_C   0000001 // Carry
#define PSW_RW  0174357 // Writable Bits for J11

#define PSW_CC  (PSW_N|PSW_Z|PSW_V|PSW_C)

#define PSW_P_CM 14
#define PSW_P_PM 12
#define PSW_MODE 03

#define PSW_GETCUR(psw) ((psw >> PSW_P_CM) & PSW_MODE)
#define PSW_GETPRV(psw) ((psw >> PSW_P_PM) & PSW_MODE)

#define IPL_MASK     7
#define IPL_POS      5
#define SET_IPL(ipl) PSW = ((ipl & IPL_MASK) << IPL_POS) | (PSW & ~PSW_IPL)
#define GET_IPL(psw) (((psw) >> IPL_POS) & IPL_MASK)

// Condition Code Flags
#define CC_SET 0000020 // Set bit in CC instructions
#define CC_N   PSW_N   // Negative Flag
#define CC_Z   PSW_Z   // Negative Flag
#define CC_V   PSW_V   // Overflow Flag
#define CC_C   PSW_C   // Carry Flag
#define CC_ALL (CC_N|CC_Z|CC_V|CC_C)

// Trap Mask - Descending priority order according to J11.
#define TRAP_P_RED    0 // Red Stack
#define TRAP_P_ODD    1 // Odd Address
#define TRAP_P_MME    2 // Memory Management Error
#define TRAP_P_NXM    3 // Non-existant Memory
#define TRAP_P_PAR    4 // Parity Error
#define TRAP_P_PRV    5 // Privileged Instruction
#define TRAP_P_ILL    6 // Illegal Instruction
#define TRAP_P_BPT    7 // Breakpoint Intruction
#define TRAP_P_IOT    8 // I/O Instruction
#define TRAP_P_EMT    9 // Emulator Instruction
#define TRAP_P_TRAP  10 // Trap Instruction
#define TRAP_P_TRC   11 // Trace (T-bit)
#define TRAP_P_YEL   12 // Yellow Stack
#define TRAP_P_PWRFL 13 // Power Failure
#define TRAP_P_FPE   14 // Floating-Point Error
#define TRAP_P_INT   15 // Interrupt Requests

#define TRAP_RED   (1u << TRAP_P_RED)   // Red Stack
#define TRAP_ODD   (1u << TRAP_P_ODD)   // Odd Address
#define TRAP_MME   (1u << TRAP_P_MME)   // Memory Management Error
#define TRAP_NXM   (1u << TRAP_P_NXM)   // Non-existant Memory
#define TRAP_PAR   (1u << TRAP_P_PAR)   // Parity Error
#define TRAP_PRV   (1u << TRAP_P_PRV)   // Privileged Instruction
#define TRAP_ILL   (1u << TRAP_P_ILL)   // Illegal Instruction
#define TRAP_BPT   (1u << TRAP_P_BPT)   // BPT Instruction
#define TRAP_IOT   (1u << TRAP_P_IOT)   // IOT Instruction
#define TRAP_EMT   (1u << TRAP_P_EMT)   // EMT Instruction
#define TRAP_TRAP  (1u << TRAP_P_TRAP)  // TRAP Instruction
#define TRAP_TRC   (1u << TRAP_P_TRC)   // Trace (T-bit)
#define TRAP_YEL   (1u << TRAP_P_YEL)   // Yellow Stack
#define TRAP_PWRFL (1u << TRAP_P_PWRFL) // Power Failure
#define TRAP_FPE   (1u << TRAP_P_FPE)   // Floating Point Error
#define TRAP_INT   (1u << TRAP_P_INT)   // Interrupt Requests

#define TRAP_MAX   TRAP_P_INT
#define TRAP_ALL   ((1u << TRAP_MAX) - 1) // All Trap Requests

#define VEC_RED    0004
#define VEC_ODD    0004
#define VEC_MME    0250
#define VEC_NXM    0004
#define VEC_PAR    0114
#define VEC_PRV    0004
#define VEC_ILL    0010
#define VEC_BPT    0014
#define VEC_IOT    0020
#define VEC_EMT    0030
#define VEC_TRAP   0034
#define VEC_TRC    0014
#define VEC_YEL    0004
#define VEC_PWRFL  0024
#define VEC_FPE    0244
#define VEC_PIRQ   0240

#define SET_CPUERR(cpue) CPUERR |= (cpue)
#define CLR_CPUERR(cpue) CPUERR &= ~(cpue)
#define SET_TRAP(tirq)   TIRQ |= (tirq)
#define CLR_TRAP(tirq)   TIRQ &= ~(tirq)
#define ABORT(why)       longjmp(p11->SetJump, why)

// General Register Defintions

// Working Registers (Signed/Unsigned Word/Byte)
#define REGW(rn)     (int16)p11->wkRegs[rn]
#define REGB(rn)     *((int8 *)(&p11->wkRegs[rn]))
#define UREGW(rn)    (uint16)p11->wkRegs[rn]
#define UREGB(rn)    *((uint8 *)(&p11->wkRegs[rn]))

#define GPREG(rn,rs) p11->gpRegs[rn][rs] // General Purpose Registers
#define STKREG(rn)   p11->stkRegs[rn]    // Stack Registers (KSP, SSP, and USP)

#define R0 UREGW(0) // R0 General Register
#define R1 UREGW(1) // R1 General Register
#define R2 UREGW(2) // R2 General Register
#define R3 UREGW(3) // R3 General Register
#define R4 UREGW(4) // R4 General Register
#define R5 UREGW(5) // R5 General Register
#define R6 UREGW(6) // R6 General Register
#define R7 UREGW(7) // R7 General Register
#define SP UREGW(6) // Stack Pointer Register   (R6)
#define PC UREGW(7) // Program Counter Register (R7)

#define faultPC p11->FaultPC   // Faulting PC address
#define IDLE    p11->Wait      // Current Wait State
#define PIRQ    p11->pgmReqs   // Program Interrupt Requests
#define TIRQ    p11->intReqs   // Interrupt/Trap Requests
#define TADR    p11->intAddr   // Interrupt/Trap Vector Address
#define IR      p11->opCode    // Instruction Register
#define PSW     p11->pswFlags  // Processor Status Register
#define CC      p11->ccFlags   //   Condition Codes
#define MAINT   p11->cpuMaint  // CPU Maintenance Register
#define CPUID   p11->cpuType   // Processor Type for MFPT Instruction
#define CPUERR  p11->cpuError  // CPU Error Register
#define MEMERR  p11->memError  // Memory Error Register
#define CCR     p11->cacReg    // Cache Control Register
#define HMR     p11->cacHit    // Hit/Miss Register
#define SR      p11->swReg     // Switch Register
#define DR      p11->dpReg     // Display Register
#define CIPS    p11->opMeter   // Instruction Speedometer

#define MMR0    p11->mmr0   // MMR0 - Status
#define MMR1    p11->mmr1   // MMR1 - R+/-R Register Recovery
#define MMR2    p11->mmr2   // MRR2 - Saved PC
#define MMR3    p11->mmr3   // MMR3 - 22-bit Status

#define APR(idx) p11->aprFile[idx]
#define PAR(idx) ((uint16)(APR(idx) >> 16))
#define PDR(idx) ((uint16)APR(idx))

#define APR_NREGS  64    // Number of PAR/PDR Registers
#define APR_NMODE  16    // Number of PAR/PDR Registers Each Mode
#define APR_KERNEL 0     // 16 PAR/PDR Registers for Kernel Mode
#define APR_SUPER  16    // 16 PAR/PDR Registers for Supervisor Mode
#define APR_UNUSED 32    // 16 PAR/PDR Registers for Unused mode
#define APR_USER   48    // 16 PAR/PDR Registers for User Mode

#define ISPACE  p11->iSpace  // Instruction Space (Enable)
#define DSPACE  p11->dSpace  // Data Space        (Enable)

#define GetISpace(mode) ((mode) << VA_P_MODE)
#define GetDSpace(mode) \
	(GetISpace(mode) | ((MMR3 & p11_dsMask[mode]) ? VA_DS : 0))
#define UpdateMM        ((MMR0 & MMR0_FREEZE) == 0)
#define SetMMR1(reg)    MMR1 = MMR1 ? (((reg) << 8) | MMR1) : (reg)

// Virtual Memory Access
#define ReadW(addr)         p11_ReadW(p11, addr)
#define ReadB(addr)         p11_ReadB(p11, addr)
#define ReadMW(addr, paddr) p11_ReadMW(p11, addr, paddr)
#define ReadMB(addr, paddr) p11_ReadMB(p11, addr, paddr)
#define WriteW(addr, data)  p11_WriteW(p11, addr, data)
#define WriteB(addr, data)  p11_WriteB(p11, addr, data)

// Physical Memory Access
#define ReadPW(addr)        p11_ReadPW(p11, addr)
#define ReadPB(addr)        p11_ReadPB(p11, addr)
#define WritePW(addr, data) p11_WritePW(p11, addr, data)
#define WritePB(addr, data) p11_WritePB(p11, addr, data)

// Console Memory Access
#define ReadCP(addr, size)        p11_ReadCP(p11, addr, size)
#define WriteCP(addr, data, size) p11_WriteCP(p11, addr, data, size)
#define ReadC(addr, size)         p11_ReadC(p11, addr, size)
#define WriteC(addr, data, size)  p11_WriteC(p11, addr, data, size)

// Fetch/Store Access for Instruction Use
#define GeteaW(spec)      p11_GeteaW(p11, spec)
#define GeteaB(spec)      p11_GeteaB(p11, spec)

#define FetchW(spec) \
	((spec <= 7) ? REGW(spec) : (int16)ReadW(GeteaW(spec)))
#define FetchB(spec) \
	((spec <= 7) ? REGB(spec) : (int8)ReadB(GeteaB(spec)))
#define FetchMW(spec, pa) \
	((spec <= 7) ? REGW(spec) : (int16)ReadMW(GeteaW(spec), pa))
#define FetchMB(spec, pa) \
	((spec <= 7) ? REGB(spec) : (int8)ReadMB(GeteaB(spec), pa))

#define StoreW(spec, data) \
	if (spec <= 7) REGW(spec) = data; \
	else           WriteW(GeteaW(spec), data);

#define StoreB(spec, data) \
	if (spec <= 7) REGB(spec) = data; \
	else           WriteB(GeteaB(spec), data);

#define StoreMW(spec, pa, data) \
	if (spec <= 7) REGW(spec) = data; \
	else           WritePW(pa, data);

#define StoreMB(spec, pa, data) \
	if (spec <= 7) REGB(spec) = data; \
	else           WritePB(pa, data);

// Condition Code Macros

#define CC_Z1ZZ \
	CC = CC_Z;

#define CC_IIZZ_I(r) \
	if ((r) < 0)       CC = CC_N; \
	else if ((r) == 0) CC = CC_Z; \
	else               CC = 0;

#define CC_IIZZ_B(r) CC_IIZZ_I((int8)(r))
#define CC_IIZZ_W(r) CC_IIZZ_I((int16)(r))

#define CC_IIZP_I(r) \
	if ((r) < 0)       CC = CC_N | (CC & CC_C); \
	else if ((r) == 0) CC = CC_Z | (CC & CC_C); \
	else               CC =        (CC & CC_C);

#define CC_IIZP_B(r) CC_IIZP_I((int8)(r))
#define CC_IIZP_W(r) CC_IIZP_I((int16)(r))

#define CC_IIZ1_I(r) \
	if ((r) < 0)       CC = CC_N | CC_C; \
	else if ((r) == 0) CC = CC_Z | CC_C; \
	else               CC =        CC_C;

#define CC_IIZ1_B(r) CC_IIZ1_I((int8)(r))
#define CC_IIZ1_W(r) CC_IIZ1_I((int16)(r))

#define INSNAM(cpu, opcode) cpu##_Opcode_##opcode
#define INSDEF(cpu, opcode) \
void INSNAM(cpu, opcode)(register P11_CPU *p11)

typedef struct p11_System    P11_SYSTEM;
typedef struct p11_Processor P11_CPU;
typedef struct p11_InstTable P11_INST;
typedef struct p11_InstTable2 P11_INST2;

struct p11_System {
	UNIT      Unit;       // Unit Header Information
	P11_CPU  *cpu;        // CPU Module
	P11_INST *tblOpcodes; // CPU Instruction Table
};

struct p11_Processor {
	UNIT       Unit;     // Unit Header Information
	P11_CPU    *Next;    // Next CPU Module (Multi-Processor)
	P11_SYSTEM *System;  // PDP-11 System
	int        State;    // Processor State (Stop, Run, etc.)
	void       *uqba;    // Unibus/QBus Adaptor
	jmp_buf    SetJump;


	uint16 FaultPC;      // Faulting PC address for debugging purpose.
	uint32 Flags;        // Internal Flags (Configuration)
	uint16 wkRegs[8];    // Working Registers (R0-R7, SP, and PC)
	uint16 gpRegs[6][2]; // Register Set      (R0-R5, R0'-R5')
	uint16 stkRegs[4];   // Stack Pointers    (KSP, SSP, and USP)

	uint16 intReqs;    // Interrupt/Trap Requests
	uint16 intVec[16]; // Interrupt/Trap Vectors;
	uint16 intAddr;    // Current Trap Vector Address

	uint16 Wait;       // Wait State for WAIT instruction
	uint16 opCode;     // Instruction Register       (IR)
	uint16 pswFlags;   // Processor Status Register  (PSW)
	uint16 ccFlags;    //   Condition Codes
	uint16 pgmReqs;    // Program Interrupt Requests (PIRQ)
	uint16 cpuMaint;   // CPU Maintenance Register   (MAINT)
	uint16 cpuType;    // CPU Identification         (CPU ID)
	uint16 cpuError;   // CPU Error Register         (CPUER)
	uint16 memError;   // Memory Error Register      (MEMERR)
	uint16 cacReg;     // Cache Control Register     (CCR)
	uint16 cacHit;     // Hit/Miss Register          (HITMISS)
	uint16 swReg;      // Switch Register            (SR)
	uint16 dpReg;      // Display Register           (DR)

	uint32 opMeter;    // Instruction Speedometer

	// Memory Management
	uint16 mmr0; // Memory Management Register #0
	uint16 mmr1; // Memory Management Register #1
	uint16 mmr2; // Memory Management Register #2
	uint16 mmr3; // Memory Management Register #3

	uint32 aprFile[APR_NREGS];

	uint32 iSpace;    // Instruction Space
	uint32 dSpace;    // Data Space

	int16  *ramData;  // Main Memory (RAM) Area
	uint32 ramSize;   // Size of RAM Area

	// Function Calls
	int  (*InitRegs)(P11_CPU *);
	int  (*InitTimer)(P11_CPU *);
	void (*StartTimer)(P11_CPU *);
	void (*StopTimer)(P11_CPU *);
	void (*ResetTimer)(P11_CPU *);

	// Instruction Table for Specific Processor
	void (*tblOpcode[NUM_INST])(register P11_CPU *);
};

struct p11_InstTable {
	char    *Name;    // Opcode Name
	char    *Descrip; // Description
	char    *Opcode;  // Opcode
	uint32  opFlags;  // Flags
	uint16  opCode;   // Opcode
	uint16  opReg;    // Operand Mask

	void (*Execute)(register P11_CPU *);
};

struct p11_InstTable2 {
	char    *Name;    // Opcode Name
	uint32  opFlags;  // Flags
	uint16  opCode;   // Opcode
	uint16  opReg;    // Operand Mask

	void (*Execute)(register P11_CPU *);
};

// External/Protype definitions
#include "pdp11/proto.h"
