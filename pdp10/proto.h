// proto.h - Prototypes for PDP10 emulation routines.
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

// pdp10/extend.c
int  p10_extOpcode_EUUO(void);
int  p10_extOpcode_CMPS(void);
int  p10_extOpcode_EDIT(void);
int  p10_extOpcode_CVTDB(void);
int  p10_extOpcode_CVTBD(void);
int  p10_extOpcode_MOVS(void);
void p10_Opcode_EXTEND(void);
int  KL10_extOpcode_XBLT(void);

// pdp10/cpu_main.c
int36 p10_CalcJumpAddr(int36, int);
int36 p10_ksCalcBPAddr(int36, int);
int18 p10_CalcEffAddr(int32);
int18 p10_bpCalcEffAddr(int32, int);
int36 p10_ksGetIOAddr(int36);
int   p10_CheckPXCT(int);
void  p10_Execute(int30, int);
void  p10_piExecute(int30);
void  p10_Initialize(P10_CPU *);
void  p10_ResetCPU(void);
int   p10_Go(MAP_DEVICE *);

void  KS10_Opcode_LUUO(void);
void  KS10_Opcode_UUO(void);


// pdp10/ks10_apr.c
void p10_ksOpcode_APRID(void);
void p10_ksOpcode_COAPR(void);
void p10_ksOpcode_CZAPR(void);
void p10_ksOpcode_RDAPR(void);
void p10_ksOpcode_WRAPR(void);

void p10_ResetAPR(void);
void p10_aprInterrupt(int);

int KS10_Trap_NoMemory(int30, int);

// pdp10/ks10_fe.c
P10_CONSOLE *p10_ConsoleInit(void);
void p10_ConsoleCleanup(P10_CONSOLE *);
void p10_ConsoleDone(void *);
void p10_ConsoleCheck(void *);
void p10_ConsoleOutput(P10_CONSOLE *);

// pdp10/ks10_io.c
void KS10_Opcode_IO700(void);
void KS10_Opcode_IO701(void);
void KS10_Opcode_IO702(void);

void KS10_Opcode_TIOE(void);
void KS10_Opcode_TION(void);
void KS10_Opcode_RDIO(void);
void KS10_Opcode_WRIO(void);
void KS10_Opcode_BSIO(void);
void KS10_Opcode_BCIO(void);
void KS10_Opcode_BLTBU(void);
void KS10_Opcode_BLTUB(void);

void KS10_Opcode_TIOEB(void);
void KS10_Opcode_TIONB(void);
void KS10_Opcode_RDIOB(void);
void KS10_Opcode_WRIOB(void);
void KS10_Opcode_BSIOB(void);
void KS10_Opcode_BCIOB(void);

// pdp10/ks10_pag.c
void p10_ksOpcode_RDSPB(void);
void p10_ksOpcode_RDCSB(void);
void p10_ksOpcode_RDPUR(void);
void p10_ksOpcode_RDCSTM(void);
void p10_ksOpcode_RDHSB(void);

void p10_ksOpcode_WRSPB(void);
void p10_ksOpcode_WRCSB(void);
void p10_ksOpcode_WRPUR(void);
void p10_ksOpcode_WRCSTM(void);
void p10_ksOpcode_WRHSB(void);

void p10_ksOpcode_RDUBR(void);
void p10_ksOpcode_CLRPT(void);
void p10_ksOpcode_WRUBR(void);
void p10_ksOpcode_WREBR(void);
void p10_ksOpcode_RDEBR(void);

int   KS10_PageTrap1(int);
void  KS10_PageTrap2(void);
int   KS10_PageRefill(uint30, uint30 *, int);
int36 KS10_GetMap(int36, int);
int   p10_CmdShowMap(char **);
void  p10_SetHaltStatus(void);
void  p10_ClearCache(void);
void  p10_ResetPager(void);

// pdp10/ks10_pi.c
void p10_ksOpcode_COPI(void);
void p10_ksOpcode_CZPI(void);
void p10_ksOpcode_RDPI(void);
void p10_ksOpcode_WRPI(void);

void p10_ResetPI(void);
void KS10_piRequestIO(int);
void KS10_piRequestAPR(int);
void KS10_piDismiss(void);
void KS10_piEvaluate(void);
void KS10_piProcess(void);

// pdp10/ks10_tim.c
void p10_ksOpcode_RDTIM(void);
void p10_ksOpcode_RDINT(void);
void p10_ksOpcode_WRTIM(void);
void p10_ksOpcode_WRINT(void);

void p10_HandleTimer(int);

// pdp10/asm.c
int p10_Assemble(int36 *, char *);

// pdp10/disasm.c
void p10_Disassemble(int30, int36, int);

// pdp10/alu.c
void p10_spAdd(int36 *, int36);
void p10_dpAdd(int36 *, int36 *, int36, int36);

void p10_spSubtract(int36 *, int36);
void p10_dpSubtract(int36 *, int36 *, int36, int36);

void p10_spMultiply(int36 *, int36);
void p10_dpMultiply(int36 *, int36 *, int36);
void p10_qpMultiply(int36 *, int36 *, int36 *, int36 *, int36, int36);

int  p10_spDivide(int36 *, int36 *, int36);
int  p10_dpDivide(int36 *, int36 *, int36);
int  p10_qpDivide(int36 *, int36 *, int36 *, int36 *, int36, int36);

void p10_spMagnitude(int36 *);
void p10_spNegate(int36 *);
void p10_dpNegate(int36 *, int36 *);

void p10_spInc(int36 *);
void p10_spDec(int36 *);

void p10_spAShift(int36 *, int36);
void p10_dpAShift(int36 *, int36 *, int36);

void p10_spLShift(int36 *, int36);
void p10_dpLShift(int36 *, int36 *, int36);

void p10_spRotate(int36 *, int36);
void p10_dpRotate(int36 *, int36 *, int36);

// pdp10/fpu.c

void p10_Opcode_DFAD(void);
void p10_Opcode_DFSB(void);
void p10_Opcode_DFMP(void);
void p10_Opcode_DFDV(void);

void p10_Opcode_FIX(void);
void p10_Opcode_FIXR(void);
void p10_Opcode_FLTR(void);
void p10_Opcode_FSC(void);

void p10_Opcode_FAD(void);
void p10_Opcode_FADM(void);
void p10_Opcode_FADB(void);

void p10_Opcode_FADR(void);
void p10_Opcode_FADRI(void);
void p10_Opcode_FADRM(void);
void p10_Opcode_FADRB(void);

void p10_Opcode_FSB(void);
void p10_Opcode_FSBM(void);
void p10_Opcode_FSBB(void);

void p10_Opcode_FSBR(void);
void p10_Opcode_FSBRI(void);
void p10_Opcode_FSBRM(void);
void p10_Opcode_FSBRB(void);

void p10_Opcode_FMP(void);
void p10_Opcode_FMPM(void);
void p10_Opcode_FMPB(void);

void p10_Opcode_FMPR(void);
void p10_Opcode_FMPRI(void);
void p10_Opcode_FMPRM(void);
void p10_Opcode_FMPRB(void);

void p10_Opcode_FDV(void);
void p10_Opcode_FDVM(void);
void p10_Opcode_FDVB(void);

void p10_Opcode_FDVR(void);
void p10_Opcode_FDVRI(void);
void p10_Opcode_FDVRM(void);
void p10_Opcode_FDVRB(void);

// pdp10/kl10_apr.c
int KL10_Trap_NoMemory(int30, int);

// pdp10/kl10_main.c
int30 KL10_extCalcEffAddr(int30, int30, int);
void  KL10_piExecute(int30);
void  KL10_Opcode_LUUO(void);
void  KL10_Opcode_UUO(void);
void  KL10_Opcode_JRST(void);
void  KL10_Opcode_MAP(void);
void  KL10_Opcode_XCT(void);

// pdp10/kl10_mtr.c
void  KL10_ClockCount(void);

// pdp10/kl10_pag.c
void  KL10_InitPager(void);
int   KL10_PageTrap1(int);
void  KL10_PageTrap2(void);
int   KL10_PageRefill(uint30, uint30 *, int);
int36 KL10_GetMap(int36, int);

// pdp10/kl10_pi.c
void KL10pi_RequestIO(int, int36);
void KL10pi_Process(void);
void KL10pi_Dismiss(void);
void KL10pi_Reset(void);
void KL10pi_Evaluate(void);

// pdp10/memory.c
void  p10_InitMemory(int32);
void  p10_ResetMemory(void);
void  p10_ReleaseMemory(void);
// int   p10_DoNXM(uint30, int);
int   p10_CheckNXM(uint30, int);

int36 p10_eRead(uint30);
int36 p10_pRead(uint30, int);
int36 p10_vRead(uint30, int);
void  p10_eWrite(uint30, int36);
void  p10_pWrite(uint30, int36, int);
void  p10_vWrite(uint30, int36, int);
int36 *p10_pAccess(uint30);
int36 *p10_Access(uint30, int);

// pdp10/symbols.c
void   p10_BuildSymbols(void);
char   *p10_FindSymbol(int36);

// pdp10/system.c
char  *pdp10_DisplayData(int36);
int36  pdp10_Convert8to36(uchar *);
uchar *pdp10_Convert36to8(int36);
int    exe_GetWord(int, int36 *);
int    pdp10_LoadRimFile(char *);
int    pdp10_LoadExeFile(char *);
P10_CONSOLE *p10_InitConsole(void);
