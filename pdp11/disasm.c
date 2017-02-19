// disasm.c - PDP11 Disassembler
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
#include "pdp11/defs.h"
#include "emu/socket.h"

static cchar *regNames[] =
	{ "R0", "R1", "R2", "R3", "R4", "R5", "SP", "PC" };
static cchar *fpNames[] =
	{ "F0", "F1", "F2", "F3", "F4", "R5", "?6", "?7" };

int p11_GetSpec(register P11_CPU *p11, char *str, uint32 *Addr,
	uint32 dSpace, uint16 opr, uint16 fpFlag)
{
	uint32 reg = opr & 07;
	uint16 val;

	switch(opr >> 3) {
		case 0: // Rn
			return sprintf(str, "%s", fpFlag ? fpNames[reg] : regNames[reg]);

		case 1: // (Rn)
			return sprintf(str, "(%s)", regNames[reg]);

		case 2: // (Rn)+ or #n
			if (reg == 7) {
				val   = ReadC(*Addr | dSpace, OP_WORD);
				*Addr = (*Addr + OP_WORD) & PA_MASK16;
				return sprintf(str, "#%o", val);
			}
			return sprintf(str, "(%s)+", regNames[reg]);

		case 3: // @(Rn)+ or @#n
			if (reg == 7) {
				val   = ReadC(*Addr | dSpace, OP_WORD);
				*Addr = (*Addr + OP_WORD) & PA_MASK16;
				return sprintf(str, "@#%o", val);
			}
			return sprintf(str, "@(%s)+", regNames[reg]);

		case 4: // -(Rn)
			return sprintf(str, "-(%s)", regNames[reg]);

		case 5: // @-(Rn)
			return sprintf(str, "@-(%s)", regNames[reg]);

		case 6: // n(Rn) or n
			val   = ReadC(*Addr | dSpace, OP_WORD);
			*Addr = (*Addr + OP_WORD) & PA_MASK16;
			if (reg == 7)
				return sprintf(str, "%-o", (*Addr + val) & PA_MASK16);
			else
				return sprintf(str, "%-o(%s)", val, regNames[reg]);

		case 7:  // @n(Rn) or @n
			val   = ReadC(*Addr | dSpace, OP_WORD);
			*Addr = (*Addr + OP_WORD) & PA_MASK16;
			if (reg == 7)
				return sprintf(str, "@%-o", (*Addr + val) & PA_MASK16);
			else
				return sprintf(str, "@%-o(%s)", val, regNames[reg]);
	}
}

char *p11_GetOpers(register P11_CPU *p11, uint32 *Addr,
	uint32 dSpace, P11_INST *tblCode, uint16 opCode)
{
	static char strOper[80];
	char        *str = strOper;
	int16       brDisp;

	switch (tblCode->opFlags & OP_TYPE) {
		case OP_NPN: // No Operands
		case OP_SCC: // Sxx Instructions
		case OP_CCC: // Cxx Instructions
			break;

		case OP_BR:  // Branch Displacement
			brDisp = (int8)opCode << 1;
			str += sprintf(str, "%06o", (*Addr + brDisp) & PA_MASK16);
			break;

		case OP_3BIT: // 3-bit Operand
			str += sprintf(str, "%-o", opCode & 07);
			break;

		case OP_6BIT: // 6-bit Operand
			str += sprintf(str, "%-o", opCode & 077);
			break;

		case OP_8BIT: // 8-bit Operand
			str += sprintf(str, "%-o", opCode & 0377);
			break;

		case OP_REG: // Register Operand
			str += sprintf(str, "%s", regNames[opCode & 07]);
			break;

		case OP_SOP: // Single-Operand Instruction
			str += p11_GetSpec(p11, str, Addr, dSpace,
				(opCode & 077), tblCode->opFlags & OP_FLOAT);
			break;

		case OP_RSOP: // Specifier, Register Operand
			str += sprintf(str, "%s,", regNames[(opCode >> 6) & 7]);
			str += p11_GetSpec(p11, str, Addr, dSpace,
				(opCode & 077), tblCode->opFlags & OP_FLOAT);
			break;

		case OP_SOB: // SOB Instruction
			str += sprintf(str, "%s,%06o",
				regNames[(opCode >> 6) & 7], *Addr - ((opCode & 077) << 1));
			break;

		case OP_DOP: // Double-Operand Instruction
			str += p11_GetSpec(p11, str, Addr, dSpace,
				((opCode >> 6) & 077), tblCode->opFlags & OP_FLOAT);
			*str++ = ',';
			str += p11_GetSpec(p11, str, Addr, dSpace,
				(opCode & 077), tblCode->opFlags & OP_FLOAT);
			break;
	}

	*str = '\0';
	return strOper;
}

void p11_Disasm(register P11_CPU *p11, SOCKET *dbg, uint32 *Addr, uint32 dSpace)
{
	P11_SYSTEM *p11sys = p11->System;
	P11_INST   *tblCode;
	uint16     opCode;
	uint16     bAddr;
	char       *strOper;

	// Fetch instruction from PDP-11 memory.
	bAddr   = *Addr;
	opCode  = p11_ReadC(p11, *Addr | dSpace, OP_WORD);
	tblCode = &p11sys->tblOpcodes[opCode];
	*Addr  += OP_WORD;

	// Decode Operands
	strOper = p11_GetOpers(p11, Addr, dSpace, tblCode, opCode);

	// Finally, print assembly line.
	SockPrintf(dbg, "%06o %06o %-8s %s\n",
		bAddr, opCode, tblCode->Name, strOper);
}

extern P11_INST pdp11_Inst[];
void p11_InitDisasm(register P11_SYSTEM *p11sys)
{
	P11_INST *tblOpcodes;
	uint32   idxInst, idxOpnd;

	// Create an instruction table for disassembler
	if((tblOpcodes = (P11_INST *)calloc(NUM_INST, sizeof(P11_INST))) == NULL)
		return;

	// Build an instruction table.
	for (idxInst = 0; pdp11_Inst[idxInst].Name; idxInst++) {
		uint16 opCode = pdp11_Inst[idxInst].opCode;
		uint16 opReg  = pdp11_Inst[idxInst].opReg;

		for (idxOpnd = opCode; idxOpnd < (opCode + opReg + 1); idxOpnd++)
			tblOpcodes[idxOpnd] = pdp11_Inst[idxInst];
	}

	// Link a table to PDP11 system device and return.
	p11sys->tblOpcodes = tblOpcodes;
}

void p11_CleanupDisasm(register P11_SYSTEM *p11sys)
{
	if (p11sys->tblOpcodes)
		free(p11sys->tblOpcodes);
	p11sys->tblOpcodes = NULL;
}

// Dump contents into terminal from memory area
void p11_Dump(P11_CPU *p11, SOCKET *dbg,
	uint32 *Addr, uint32 eAddr, uint32 sw)
{
	int     idx;
	char    ascBuffer[9];
	char    *pasc;
	uint32  dspace;
	uint16  data;

	dspace = p11_GetSpace(p11, sw);
	while (*Addr <= eAddr) {
		SockPrintf(dbg, "%06o: ", *Addr);
		pasc = ascBuffer;
		for (idx = 0; (idx < 8) && (*Addr <= eAddr); idx++) {
			data = p11_ReadC(p11, (*Addr)++ | dspace, 1);
			SockPrintf(dbg, "%03o%c", data, (idx == 3) ? '-' : ' ');
			*pasc++ = ((data >= 32) && (data < 127)) ? data : '.';
		}
		*pasc = '\0';
		SockPrintf(dbg, " |%-8s|\n", ascBuffer);
	}
}

#endif /* DEBUG */
