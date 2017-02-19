// disasm.c - PDP-6/PDP-10 Disassembler
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

#ifdef DEBUG

#include "pdp10/defs.h"

extern INSTRUCTION *pdp10_Opcode[];
extern INSTRUCTION *pdp10_OpcodeEXT[];
extern INSTRUCTION *pdp10_OpcodeIO[];
extern INSTRUCTION *pdp10_OpcodeDEV[];
extern INSTRUCTION *pdp10_OpcodeFUNC[];

void p10_Disassemble(int30 Addr, int36 Inst, int mode)
{
	// Fields of Instruction Code
	int18 opDevice;   // (I/O)   Device Code       (DEV)
	int18 opFunction; // (I/O)   Function Code     (FUNC)
	int18 opCode;     // (Basic) Opcode field      (OP)
	int18 opAC;       // (Basic) Accumulator       (AC)
	int18 opIndirect; // (Both)  Indirect          (I)
	int18 opIndex;    // (Both)  Index Register    (X)
	int18 opAddr;     // (Both)  Address           (Y)

	char  *Name;
	char  *Symbol = NULL;
	char  *locSymbol;// Symbol of location
	char  *acSymbol; // Symbol of accumlator
	char  *xrSymbol; // Symbol of index accumlator
	char  *eaSymbol; // Symbol of effective address

	opCode     = INST_GETOP(Inst);
	opDevice   = INST_GETDEV(Inst);
	opFunction = INST_GETFUNC(Inst);
	opIndirect = INST_GETI(Inst);
	opIndex    = INST_GETX(Inst);
	opAddr     = INST_GETY(Inst);
	opAC       = INST_GETAC(Inst);

	if (mode & OP_EXT) {
		Name = pdp10_OpcodeEXT[opCode]->Name;
	} else {
		if (opCode >= 0700) {
			// I/O Instruction Format
			if (pdp10_Opcode[opCode] != NULL) {
				Name = pdp10_Opcode[opCode]->Name;
			} else if (pdp10_OpcodeIO[(Inst >> 23) & 01777]) {
				Name = pdp10_OpcodeIO[(Inst >> 23) & 01777]->Name;
				opAC = 0;
			} else {
				Name = pdp10_OpcodeFUNC[opFunction]->Name;
				if (pdp10_OpcodeDEV[opDevice])
					Symbol = pdp10_OpcodeDEV[opDevice]->Name;
//				if (p10_kxDevices[opDevice])
//					Symbol = p10_kxDevices[opDevice]->Name;
				opAC = opDevice << 2;
			}
		} else {
			// Basic Instruction Format
			if (pdp10_Opcode[opCode]->Flags & OP_AC) {
				if (pdp10_Opcode[opCode]->AC[opAC]) {
					Name = pdp10_Opcode[opCode]->AC[opAC]->Name;
					opAC = 0;
				} else
					Name = pdp10_Opcode[opCode]->Name;
			} else
				Name = pdp10_Opcode[opCode]->Name;
		}
	}

	dbg_Printf("CPU: %06o %06o,,%06o %-6s ",
		Addr, LH18(Inst), RH18(Inst), Name);

	if (opAC)
		dbg_Printf("%o,", opAC);
	else if (Symbol)
		dbg_Printf("%s,", Symbol);
	if (opIndirect)
		dbg_Printf("@");
	if (opAddr)
		dbg_Printf("%o", opAddr);
	if (opIndex)
		dbg_Printf("(%o)", opIndex);

	dbg_Printf("\n");
}

#endif /* DEBUG */
