// proto.h - Prototypes for all .c files.
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

// Instruction Function Calls
DEF_INST(vax, ACBB);
DEF_INST(vax, ACBD);
DEF_INST(vax, ACBF);
DEF_INST(vax, ACBG);
DEF_INST(vax, ACBL);
DEF_INST(vax, ACBW);
DEF_INST(vax, ADAWI);
DEF_INST(vax, ADDB);
DEF_INST(vax, ADDD);
DEF_INST(vax, ADDF);
DEF_INST(vax, ADDG);
DEF_INST(vax, ADDL);
DEF_INST(vax, ADDW);
DEF_INST(vax, ADWC);
DEF_INST(vax, AOBLEQ);
DEF_INST(vax, AOBLSS);
DEF_INST(vax, ASHL);
DEF_INST(vax, ASHQ);
DEF_INST(vax, BBC);
DEF_INST(vax, BBCC);
DEF_INST(vax, BBCS);
DEF_INST(vax, BBS);
DEF_INST(vax, BBSC);
DEF_INST(vax, BBSS);
DEF_INST(vax, BCC);
DEF_INST(vax, BCS);
DEF_INST(vax, BEQL);
DEF_INST(vax, BGEQ);
DEF_INST(vax, BGTR);
DEF_INST(vax, BGTRU);
DEF_INST(vax, BICB);
DEF_INST(vax, BICL);
DEF_INST(vax, BICPSW);
DEF_INST(vax, BICW);
DEF_INST(vax, BISB);
DEF_INST(vax, BISL);
DEF_INST(vax, BISPSW);
DEF_INST(vax, BISW);
DEF_INST(vax, BITB);
DEF_INST(vax, BITL);
DEF_INST(vax, BITW);
DEF_INST(vax, BLBC);
DEF_INST(vax, BLBS);
DEF_INST(vax, BLEQ);
DEF_INST(vax, BLEQU);
DEF_INST(vax, BLSS);
DEF_INST(vax, BNEQ);
DEF_INST(vax, BPT);
DEF_INST(vax, BRB);
DEF_INST(vax, BRW);
DEF_INST(vax, BSBB);
DEF_INST(vax, BSBW);
DEF_INST(vax, BUGL);
DEF_INST(vax, BUGW);
DEF_INST(vax, BVC);
DEF_INST(vax, BVS);
DEF_INST(vax, CALLG);
DEF_INST(vax, CALLS);
DEF_INST(vax, CASEB);
DEF_INST(vax, CASEL);
DEF_INST(vax, CASEW);
DEF_INST(vax, CHME);
DEF_INST(vax, CHMK);
DEF_INST(vax, CHMS);
DEF_INST(vax, CHMU);
DEF_INST(vax, CLRB);
DEF_INST(vax, CLRL);
DEF_INST(vax, CLRQ);
DEF_INST(vax, CLRW);
DEF_INST(vax, CMPB);
DEF_INST(vax, CMPC3);
DEF_INST(vax, CMPC5);
DEF_INST(vax, CMPD);
DEF_INST(vax, CMPF);
DEF_INST(vax, CMPG);
DEF_INST(vax, CMPL);
DEF_INST(vax, CMPV);
DEF_INST(vax, CMPW);
DEF_INST(vax, CMPZV);
DEF_INST(vax, CVTBD);
DEF_INST(vax, CVTBF);
DEF_INST(vax, CVTBG);
DEF_INST(vax, CVTBL);
DEF_INST(vax, CVTBW);
DEF_INST(vax, CVTDB);
DEF_INST(vax, CVTDF);
DEF_INST(vax, CVTDL);
DEF_INST(vax, CVTDW);
DEF_INST(vax, CVTFB);
DEF_INST(vax, CVTFD);
DEF_INST(vax, CVTFG);
DEF_INST(vax, CVTFL);
DEF_INST(vax, CVTFW);
DEF_INST(vax, CVTGB);
DEF_INST(vax, CVTGF);
DEF_INST(vax, CVTGL);
DEF_INST(vax, CVTGW);
DEF_INST(vax, CVTLB);
DEF_INST(vax, CVTLD);
DEF_INST(vax, CVTLF);
DEF_INST(vax, CVTLG);
DEF_INST(vax, CVTLW);
DEF_INST(vax, CVTRDL);
DEF_INST(vax, CVTRFL);
DEF_INST(vax, CVTRGL);
DEF_INST(vax, CVTWB);
DEF_INST(vax, CVTWD);
DEF_INST(vax, CVTWF);
DEF_INST(vax, CVTWG);
DEF_INST(vax, CVTWL);
DEF_INST(vax, DECB);
DEF_INST(vax, DECL);
DEF_INST(vax, DECW);
DEF_INST(vax, DIVB);
DEF_INST(vax, DIVD);
DEF_INST(vax, DIVF);
DEF_INST(vax, DIVG);
DEF_INST(vax, DIVL);
DEF_INST(vax, DIVW);
DEF_INST(vax, EDIV);
DEF_INST(vax, EMODD);
DEF_INST(vax, EMODF);
DEF_INST(vax, EMODG);
DEF_INST(vax, EMUL);
DEF_INST(vax, EXTV);
DEF_INST(vax, EXTZV);
DEF_INST(vax, FFC);
DEF_INST(vax, FFS);
DEF_INST(vax, HALT);
DEF_INST(vax, INCB);
DEF_INST(vax, INCL);
DEF_INST(vax, INCW);
DEF_INST(vax, INDEX);
DEF_INST(vax, INSQHI);
DEF_INST(vax, INSQTI);
DEF_INST(vax, INSQUE);
DEF_INST(vax, INSV);
DEF_INST(vax, JMP);
DEF_INST(vax, JSB);
DEF_INST(vax, LDPCTX);
DEF_INST(vax, LOCC);
DEF_INST(vax, MCOMB);
DEF_INST(vax, MCOML);
DEF_INST(vax, MCOMW);
DEF_INST(vax, MFPR);
DEF_INST(vax, MNEGB);
DEF_INST(vax, MNEGD);
DEF_INST(vax, MNEGF);
DEF_INST(vax, MNEGG);
DEF_INST(vax, MNEGL);
DEF_INST(vax, MNEGW);
DEF_INST(vax, MOVB);
DEF_INST(vax, MOVC3);
DEF_INST(vax, MOVC5);
DEF_INST(vax, MOVD);
DEF_INST(vax, MOVF);
DEF_INST(vax, MOVG);
DEF_INST(vax, MOVL);
DEF_INST(vax, MOVPSL);
DEF_INST(vax, MOVQ);
DEF_INST(vax, MOVW);
DEF_INST(vax, MOVZBL);
DEF_INST(vax, MOVZBW);
DEF_INST(vax, MOVZWL);
DEF_INST(vax, MTPR);
DEF_INST(vax, MULB);
DEF_INST(vax, MULD);
DEF_INST(vax, MULF);
DEF_INST(vax, MULG);
DEF_INST(vax, MULL);
DEF_INST(vax, MULW);
DEF_INST(vax, NOP);
DEF_INST(vax, POLYD);
DEF_INST(vax, POLYF);
DEF_INST(vax, POLYG);
DEF_INST(vax, POPR);
DEF_INST(vax, PROBER);
DEF_INST(vax, PROBEW);
DEF_INST(vax, PUSHL);
DEF_INST(vax, PUSHR);
DEF_INST(vax, REI);
DEF_INST(vax, REMQHI);
DEF_INST(vax, REMQTI);
DEF_INST(vax, REMQUE);
DEF_INST(vax, RET);
DEF_INST(vax, ROTL);
DEF_INST(vax, RSB);
DEF_INST(vax, SBWC);
DEF_INST(vax, SCANC);
DEF_INST(vax, SKPC);
DEF_INST(vax, SOBGEQ);
DEF_INST(vax, SOBGTR);
DEF_INST(vax, SPANC);
DEF_INST(vax, SUBB);
DEF_INST(vax, SUBD);
DEF_INST(vax, SUBF);
DEF_INST(vax, SUBG);
DEF_INST(vax, SUBL);
DEF_INST(vax, SUBW);
DEF_INST(vax, SVPCTX);
DEF_INST(vax, TSTB);
DEF_INST(vax, TSTD);
DEF_INST(vax, TSTF);
DEF_INST(vax, TSTG);
DEF_INST(vax, TSTL);
DEF_INST(vax, TSTW);
DEF_INST(vax, XFC);
DEF_INST(vax, XORB);
DEF_INST(vax, XORL);
DEF_INST(vax, XORW);
DEF_INST(vax, Emulate);
DEF_INST(vax, Illegal);
DEF_INST(vax, Unimplemented);

// cpu_intexc.c
int32 vax_EvaluateIRQ(register VAX_CPU *);
int32 vax_GetVector(void);
void  vax_DoIntexc(register VAX_CPU *, int32, int32, int32, ...);
void  vax_Emulate(register VAX_CPU *, int32);

// cpu_main.c
void  vax_BuildCPU(VAX_CPU *, uint32);
char *vax_DisplayConditions(uint32);
//void vax_DecodeOperand(INSTRUCTION *, int32 *);
//void  vax_DecodeOperand(uint32 *, int32 *);
int   vax_Execute(MAP_DEVICE *);

// cpu_mmu.c
TLBENT  vax_Fill(register VAX_CPU *, uint32, int32, int32, int32 *);
void    vax_ClearTBTable(register VAX_CPU *, int);
void    vax_ClearTBEntry(register VAX_CPU *, uint32);
int     vax_CheckTBEntry(register VAX_CPU *, uint32);
int     vax_ShowTLB(void *, int, char **);

#ifdef DEBUG
// disasm.c
void vax_InitDisasm(void);
//int  vax_Disasm(register VAX_CPU *, SOCKET *, uint32 *, uint32);
//int  vax_Dump(VAX_CPU *, SOCKET *, uint32 *, uint32, uint32);
#endif /* DEBUG */

// dev_cty.c
VAX_CONSOLE *vax_ConsoleInit(VAX_CPU *);
void vax_ConsoleCleanup(VAX_CONSOLE *);
uint32 vax_ReadRXCS(VAX_CPU *);
uint32 vax_ReadRXDB(VAX_CPU *);
uint32 vax_ReadTXCS(VAX_CPU *);
uint32 vax_ReadTXDB(VAX_CPU *);
void   vax_WriteRXCS(VAX_CPU *, uint32);
void   vax_WriteRXDB(VAX_CPU *, uint32);
void   vax_WriteTXCS(VAX_CPU *, uint32);
void   vax_WriteTXDB(VAX_CPU *, uint32);

// memory.c
int    vax_InitMemory(VAX_CPU *, int);
int    vax_FreeMemory(VAX_CPU *);
int32  vax_ReadInst(register VAX_CPU *, int32);
uint32 vax_ReadAligned(register VAX_CPU *, uint32, int32);
uint32 vax_Read(register VAX_CPU *, uint32, int32, int32);
void   vax_WriteAligned(register VAX_CPU *, uint32, uint32, int32);
void   vax_Write(register VAX_CPU *, uint32, uint32, int32, int32);
int32  vax_Test(register VAX_CPU *, uint32, int32, int32 *);

int32  vax_ReadC(register VAX_CPU *, uint32, uint32 *, int32, uint32);
int32  vax_WriteC(register VAX_CPU *, uint32, uint32, int32, uint32);
int32  vax_ReadCI(register VAX_CPU *, uint32, uint32 *, int32, uint32);

// system.c
int  vax_LoadFile(VAX_CPU *, char *, uint32, uint32 *);
int  vax_LoadROM(VAX_CPU *, char *, uint32);
void vax_CheckInstructions(void);

// ka780.c (KA780 System Configurations)
int32 ka780_prRead(int32);
void  ka780_prWrite(int32, int32);
void  ka780_Initialize(VAX_CPU *);
