// disasm.c - VAX Symbolic Disassembler
//
// Copyright (c) 2001, Timothy M. Stark
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

#include "vax/defs.h"
#include "emu/socket.h"

// Instruction table from inst.c file.
extern INSTRUCTION vax_Instruction[];

static INSTRUCTION *disasm_basOpcode[256];
static INSTRUCTION *disasm_extOpcode[256];
static INSTRUCTION *disasm_bugOpcode[256];

static char *regNames[] = {
	"R0",  "R1",  "R2",  "R3",  "R4",  "R5",  "R6",  "R7",
	"R8",  "R9",  "R10", "R11", "AP",  "FP",  "SP",  "PC"
};

static char Comment[80];
static int  nWords = 0;

void vax_DisasmAddDest(int32 data)
{
	char aschex[10];

	if (!Comment[0])
		strcat(Comment, " ; ");
	else
		strcat(Comment, ", ");

	sprintf(aschex, "%08X", data);
	strcat(Comment, aschex);
}

int vax_DisasmOperand(register VAX_CPU *vax, INSTRUCTION *opCode, int opCount,
	uint32 *pc, char *disasm, int indexed, uint32 sw)
{
	int    access = opCode->opMode[opCount];
	int    scale  = access & 0x00FF;
	uint8  opType, mode, reg;
	uint32 data = 0;
	char   fmt[64];
	char   strReg[64];

	if ((opCount > 0) && !indexed)
		strcat(disasm, ",");

	if (access & OP_IMMED) {
		vax_ReadCI(vax, *pc, &data, scale, sw);
		*pc += scale;

		sprintf(fmt, "#%%0%dX", scale * 2);
		sprintf(strReg, fmt, data);
		strcat(disasm, strReg);

		return VAX_OK;
	}

	if (access & OP_BRANCH) {
		vax_ReadCI(vax, *pc, &data, scale, sw);
		*pc += scale;

		data = (scale == 1) ? (int8)data :
				 (scale == 2) ? (int16)data: data;
	
//		if (scale == 1) {
//			data = (data & 0x0080) ? (data | 0xFFFFFF00) : (data & 0x007F);
//		} else if (scale == 2) {
//			data = (data & 0x8000) ? (data | 0xFFFF0000) : (data & 0x7FFF);
//		}

		sprintf(strReg, "%08X", *pc + (int32)data);
		strcat(disasm, strReg);

		return VAX_OK;
	}

	vax_ReadCI(vax, (*pc)++, (uint32 *)&opType, OP_BYTE, sw);
	mode = (opType >> 4) & 0x0F;
	reg  = opType & 0x0F;

	if ((mode >= 8) && (reg == 0x0F)) {
		switch (mode) {
			case 0x08: // Immediate
				vax_ReadCI(vax, *pc, &data, scale, sw);
				switch (scale) {
					case 1:
						sprintf(strReg, "I^#%02X", data);
						break;

					case 2:
						sprintf(strReg, "I^#%04X", data);
						break;

					case 4:
					case 8:
						if (access & OP_FLOAT)
							sprintf(strReg, "I^#%f", 0.0);
						else
							sprintf(strReg, "I^#%08X", data);
						break;
				}
				*pc += scale;
				strcat(disasm, strReg);

				// CASEx instruction here
				if (opCount == 2 && opCode->Extended == 0) {
					uint8 op = opCode->Opcode;
					if (op == 0x8F || op == 0xAF || op == 0xCF)
						nWords = data;
				}
				break;

			case 0x09: // Absolute
				vax_ReadCI(vax, *pc, &data, OP_LONG, sw);
				*pc += 4;
				sprintf(strReg, "@#%08X", data);
				strcat(disasm, strReg);
				break;

			case 0x0A: // Byte Relative
				vax_ReadCI(vax, (*pc)++, &data, OP_BYTE, sw);
				vax_DisasmAddDest(*pc + (int8)data);
				sprintf(strReg, "B^%02X", data);
				strcat(disasm, strReg);
				break;

			case 0x0B: // Deferred Byte Relative
				vax_ReadCI(vax, (*pc)++, &data, OP_BYTE, sw);
				vax_DisasmAddDest(*pc + (int8)data);
				sprintf(strReg, "@B^%02X", data);
				strcat(disasm, strReg);
				break;

			case 0x0C: // Word Relative
				vax_ReadCI(vax, *pc, &data, OP_WORD, sw);
				*pc += 2;
				vax_DisasmAddDest(*pc + (int16)data);
				sprintf(strReg, "W^%04X", data);
				strcat(disasm, strReg);
				break;

			case 0x0D: // Deferred Word Relative
				vax_ReadCI(vax, *pc, &data, OP_WORD, sw);
				*pc += 2;
				vax_DisasmAddDest(*pc + (int16)data);
				sprintf(strReg, "@W^%04X", data);
				strcat(disasm, strReg);
				break;

			case 0x0E: // Longword Relative
				vax_ReadCI(vax, *pc, &data, OP_LONG, sw);
				*pc += 4;
				vax_DisasmAddDest(*pc + (int32)data);
				sprintf(strReg, "L^%08X", data);
				strcat(disasm, strReg);
				break;

			case 0x0F: // Deferred Longword Relative
				vax_ReadCI(vax, *pc, &data, OP_LONG, sw);
				*pc += 4;
				vax_DisasmAddDest(*pc + (int32)data);
				sprintf(strReg, "@L^%08X", data);
				strcat(disasm, strReg);
				break;
		}

		return VAX_OK;
	}

	switch (mode) {
		case 0x00: // Literal
		case 0x01:
		case 0x02:
		case 0x03:
			if (access & OP_FLOAT)
				sprintf(strReg, "S^#%f", 0.0);
			else
				sprintf(strReg, "S^#%02X", opType);
			strcat(disasm, strReg);

			// CASEx instruction here
			if (opCount == 2 && opCode->Extended == 0) {
				uint8 op = opCode->Opcode;
				if (op == 0x8F || op == 0xAF || op == 0xCF)
					nWords = opType;
			}
			break;

		case 0x04: // Indexed
			vax_DisasmOperand(vax, opCode, opCount, pc, disasm, 1, sw);
			sprintf(strReg, "[%s]", regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x05: // Register
			strcat(disasm, regNames[reg]);
			break;

		case 0x06: // Register Deferred
			sprintf(strReg, "(%s)", regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x07: // Autodecrement
			sprintf(strReg, "-(%s)", regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x08: // Autoincrement
			sprintf(strReg, "(%s)+", regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x09: // Autoincrement Deferred 
			sprintf(strReg, "@(%s)+", regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x0A: // Byte Displacement
			vax_ReadCI(vax, (*pc)++, &data, OP_BYTE, sw);
			sprintf(strReg, "B^%02X(%s)", data, regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x0B: // Deferred Byte Displacement
			vax_ReadCI(vax, (*pc)++, &data, OP_BYTE, sw);
			sprintf(strReg, "@B^%02X(%s)", data, regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x0C: // Word Displacement
			vax_ReadCI(vax, *pc, &data, OP_WORD, sw);
			*pc += 2;
			sprintf(strReg, "W^%04X(%s)", data, regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x0D: // Deferred Word Displacement
			vax_ReadCI(vax, *pc, &data, OP_WORD, sw);
			*pc += 2;
			sprintf(strReg, "@W^%04X(%s)", data, regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x0E: // Longword Displacement
			vax_ReadCI(vax, *pc, &data, OP_LONG, sw);
			*pc += 4;
			sprintf(strReg, "L^%08X(%s)", data, regNames[reg]);
			strcat(disasm, strReg);
			break;

		case 0x0F: // Deferred Longword Displacement
			vax_ReadCI(vax, *pc, &data, OP_LONG, sw);
			*pc += 4;
			sprintf(strReg, "@L^%08X(%s)", data, regNames[reg]);
			strcat(disasm, strReg);
			break;
	}

	return VAX_OK;
}

int vax_Disasm(register VAX_CPU *vax, SOCKET *dbg, uint32 *pc, uint32 sw)
{
	INSTRUCTION *opCode;
	char   disasm[256], str[256];
	uint32 extended, opcode;
	int    idx;

	sprintf(disasm, "%08X ", *pc);

	extended = 0;
	vax_ReadCI(vax, (*pc)++, &opcode, OP_BYTE, sw);
	if (opcode >= 0xFD) {
		extended = opcode;
		vax_ReadCI(vax, (*pc)++, &opcode, OP_BYTE, sw);
	}

	switch (extended) {
		case 0x00:
			opCode = disasm_basOpcode[opcode];
			break;

		case 0xFD:
			opCode = disasm_extOpcode[opcode];
			break;

		case 0xFF:
			opCode = disasm_bugOpcode[opcode];
			break;

		default:
			opCode = NULL;
	}

	if (opCode) {
		Comment[0] = 0;
		sprintf(str, "%-8s ", opCode->Name);
		strcat(disasm, str);

		if (opCode->nOperands) {
			for (idx = 0; idx < opCode->nOperands; idx++)
				vax_DisasmOperand(vax, opCode, idx, pc, disasm, 0, sw);
		}

		if (Comment[0])
			strcat(disasm, Comment);
	} else {
		if (extended)
			sprintf(str, ".BYTE %02X,%02X", extended, opcode);
		else
			sprintf(str, ".BYTE %02X", opcode);
		strcat(disasm, str);
	}

	SockPrintf(dbg, "%s\n", disasm);

	// Following CASE instruction.
	if (nWords) {
		int32  base_pc = *pc;
		uint32 data;
		for (idx = 0; idx <= nWords; idx++) {
			vax_ReadCI(vax, *pc, &data, OP_WORD, sw);
			SockPrintf(dbg, "%08X .WORD    %08X\n", *pc, base_pc + SXTW(data));	
			*pc += OP_WORD;
		}
		nWords = 0;
	}

	return VAX_OK;
}

// Build the instruction table for disassembler routines
void vax_InitDisasm(void)
{
	uint8 extended, opcode;
	int   idx;

	// Clear all instruction tables first.
	for(idx = 0; idx < 256; idx++) {
		disasm_basOpcode[idx] = NULL;
		disasm_extOpcode[idx] = NULL;
		disasm_bugOpcode[idx] = NULL;
	}

	// Build new instruction tables now.
	for(idx = 0; vax_Instruction[idx].Name; idx++) {
		extended = vax_Instruction[idx].Extended;
		opcode   = vax_Instruction[idx].Opcode;

//		printf("Name: %-8s  Extended: %02X Opcode %02X\n",
//			vax_Instruction[idx].Name, extended, opcode);

		switch(extended) {
			case 0x00: // Normal function
				if (disasm_basOpcode[opcode] == NULL)
					disasm_basOpcode[opcode] = &vax_Instruction[idx];
				break;

			case 0xFD: // Extended function
				if (disasm_extOpcode[opcode] == NULL)
					disasm_extOpcode[opcode] = &vax_Instruction[idx];
				break;

			case 0xFF: // Bug Check instructions
				if (disasm_bugOpcode[opcode] == NULL)
					disasm_bugOpcode[opcode] = &vax_Instruction[idx];
				break;
		}
	}
}

// Dump contents into terminal from memory area
int vax_Dump(VAX_CPU *vax, SOCKET *dbg, uint32 *Addr, uint32 eAddr, uint32 sw)
{
	int     idx;
	char    ascBuffer[16];
	char    *pasc;
	uint32  data;

	while (*Addr <= eAddr) {
		SockPrintf(dbg, "%08X: ", *Addr);
		pasc = ascBuffer;
		for (idx = 0; (idx < 16) && (*Addr <= eAddr); idx++) {
			vax_ReadC(vax, (*Addr)++, &data, OP_BYTE, sw);
			SockPrintf(dbg, "%02X%c", data, (idx == 7) ? '-' : ' ');
			*pasc++ = ((data >= 32) && (data < 127)) ? data : '.';
		}
		*pasc = '\0';
		SockPrintf(dbg, " |%-16s|\n", ascBuffer);
	}

	return VAX_OK;
}

#endif /* DEBUG */
