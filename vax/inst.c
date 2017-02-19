// instruction.c - Complete VAX Instruction Table for all VAX series
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

#include "vax/defs.h"

INSTRUCTION vax_Instruction[] =
{
	{
		"HALT", "Halt",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x00,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, HALT) // Opcode Function
	},

	{
		"NOP", "No operation",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x01,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, NOP) // Opcode Function
	},

	{
		"REI", "Return from exception or interrupt",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x02,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, REI) // Opcode Function
	},

	{
		"BPT", "Break point trap",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x03,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BPT) // Opcode Function
	},

	{
		"RET", "Return from called procedure",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x04,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, RET) // Opcode Function
	},

	{
		"RSB", "Return from subroutine",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x05,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, RSB) // Opcode Function
	},

	{
		"LDPCTX", "Load program context",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x06,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, LDPCTX) // Opcode Function
	},

	{
		"SVPCTX", "Save program context",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x07,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SVPCTX) // Opcode Function
	},

	{
		"CVTPS", "Convert packed to leading separate",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x08,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, AB, RW, AB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTSP", "Convert leading separate to packed",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x09,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, AB, RW, AB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"INDEX", "Index calculation",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x0A,   // Opcode (Extended + Normal)
		6,            // Number of Operands
		{ RL, RL, RL, RL, RL, WL }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, INDEX) // Opcode Function
	},

	{
		"CRC", "Calcuate cyclic redundancy check",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x0B,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ AB, RL, RW, AB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"PROBER", "Probe read access",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x0C,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RW, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PROBER) // Opcode Function
	},

	{
		"PROBEW", "Probe write access",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x0D,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RW, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PROBEW) // Opcode Function
	},

	{
		"INSQUE", "Insert into queue",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x0E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AB, AB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, INSQUE) // Opcode Function
	},

	{
		"REMQUE", "Remove from queue",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x0F,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AB, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, REMQUE) // Opcode Function
	},

	{
		"BSBB", "Branch to subroutine with byte displacment",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x10,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BSBB) // Opcode Function
	},

	{
		"BRB", "Branch with byte displacement",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x11,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BRB) // Opcode Function
	},

	{
		"BNEQ", "Branch on not equal",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x12,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BNEQ) // Opcode Function
	},

	{
		"BNEQU", "Branch on not equal unsigned",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x12,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BNEQ) // Opcode Function
	},

	{
		"BEQL", "Branch on equal",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x13,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BEQL) // Opcode Function
	},

	{
		"BEQLU", "Branch on equal unsigned",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x13,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BEQL) // Opcode Function
	},

	{
		"BGTR", "Branch on greater",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x14,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BGTR) // Opcode Function
	},

	{
		"BLEQ", "Branch on less or equal",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x15,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BLEQ) // Opcode Function
	},

	{
		"JSB", "Jump to subroutine",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x16,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, JSB) // Opcode Function
	},

	{
		"JMP", "Jump",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x17,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, JMP) // Opcode Function
	},

	{
		"BGEQ", "Branch on greater or equal",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x18,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BGEQ) // Opcode Function
	},

	{
		"BLSS", "Branch on less",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x19,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BLSS) // Opcode Function
	},

	{
		"BGTRU", "Branch on greater unsigned",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x1A,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BGTRU) // Opcode Function
	},

	{
		"BLEQU", "Branch on less or equal unsigned",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x1B,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BLEQU) // Opcode Function
	},

	{
		"BVC", "Branch on overflow clear",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x1C,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BVC) // Opcode Function
	},

	{
		"BVS", "Branch on overflow set",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x1D,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BVS) // Opcode Function
	},

	{
		"BCC", "Branch on carry clear",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x1E,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BCC) // Opcode Function
	},

	{
		"BGEQU", "Branch on greater or equal unsigned",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x1E,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BCC) // Opcode Function
	},

	{
		"BCS", "Branch on carry set",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x1F,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BCS) // Opcode Function
	},

	{
		"BLSSU", "Branch on less unsigned",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x1F,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BCS) // Opcode Function
	},

	{
		"ADDP4", "Add packed 4 operand",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x20,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, AB, RW, AB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"ADDP6", "Add packed 6 operand",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x21,   // Opcode (Extended + Normal)
		6,            // Number of Operands
		{ RW, AB, RW, AB, RW, AB }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"SUBP4", "Subtract packed 4 operand",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x22,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, AB, RW, AB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"SUBP6", "Subtract packed 6 operand",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x23,   // Opcode (Extended + Normal)
		6,            // Number of Operands
		{ RW, AB, RW, AB, RW, AB }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTPT", "Convert packed to trailing",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x24,   // Opcode (Extended + Normal)
		5,            // Number of Operands
		{ RW, AB, AB, RW, AB, 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MULP6", "Multiply packed",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x25,   // Opcode (Extended + Normal)
		6,            // Number of Operands
		{ RW, AB, RW, AB, RW, AB }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTTP", "Convert trailing to packed",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x26,   // Opcode (Extended + Normal)
		5,            // Number of Operands
		{ RW, AB, AB, RW, AB, 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"DIVP6", "Divide packed",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x27,   // Opcode (Extended + Normal)
		6,            // Number of Operands
		{ RW, AB, RW, AB, RW, AB }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MOVC3", "Move character 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x28,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, AB, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVC3) // Opcode Function
	},

	{
		"CMPC3", "Compare character 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x29,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, AB, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPC3) // Opcode Function
	},

	{
		"SCANC", "Scan for character",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x2A,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, AB, AB, RB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SCANC) // Opcode Function
	},

	{
		"SPANC", "Span characters",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x2B,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, AB, AB, RB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SPANC) // Opcode Function
	},

	{
		"MOVC5", "Move character 5 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x2C,   // Opcode (Extended + Normal)
		5,            // Number of Operands
		{ RW, AB, RB, RW, AB, 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVC5) // Opcode Function
	},

	{
		"CMPC5", "Compare character 5 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x2D,   // Opcode (Extended + Normal)
		5,            // Number of Operands
		{ RW, AB, RB, RW, AB, 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPC5) // Opcode Function
	},

	{
		"MOVTC", "Move translated characters",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x2E,   // Opcode (Extended + Normal)
		6,            // Number of Operands
		{ RW, AB, RB, AB, RW, AB }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MOVTUC", "Move translated until character",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x2F,   // Opcode (Extended + Normal)
		6,            // Number of Operands
		{ RW, AB, RB, AB, RW, AB }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"BSBW", "Branch to subroutine with word displacment",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x30,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BSBW) // Opcode Function
	},

	{
		"BRW", "Branch with word displacement",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x31,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ BW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BRW) // Opcode Function
	},

	{
		"CVTWL", "Convert word to longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x32,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTWL) // Opcode Function
	},

	{
		"CVTWB", "Convert word to byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x33,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTWB) // Opcode Function
	},

	{
		"MOVP", "Move packed",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x34,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, AB, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CMPP3", "Compare packed 3 operand",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x35,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, AB, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTPL", "Convert packed to longword",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x36,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, AB, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CMPP4", "Compare packed 4 operand",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x37,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, AB, RW, AB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"EDITPC", "Edit packed to character",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x38,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, AB, AB, AB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MATCHC", "Match characters",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0x39,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, AB, RW, AB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"LOCC", "Locate character",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x3A,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RW, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, LOCC) // Opcode Function
	},

	{
		"SKPC", "Skip character",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x3B,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RW, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SKPC) // Opcode Function
	},

	{
		"MOVZWL", "Move zero-extended word to longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x3C,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVZWL) // Opcode Function
	},

	{
		"ACBW", "Add compare and branch word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x3D,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RW, RW, MW, BW, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ACBW) // Opcode Function
	},

	{
		"MOVAW", "Move address of word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x3E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AW, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"PUSHAW", "Push address of word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x3F,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"ADDF2", "Add F_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x40,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, MF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDF) // Opcode Function
	},

	{
		"ADDF3", "Add F_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x41,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RF, RF, WF, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDF) // Opcode Function
	},

	{
		"SUBF2", "Subtract F_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x42,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, MF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBF) // Opcode Function
	},

	{
		"SUBF3", "Subtract F_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x43,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RF, RF, WF, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBF) // Opcode Function
	},

	{
		"MULF2", "Multiply F_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x44,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, MF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULF) // Opcode Function
	},

	{
		"MULF3", "Multiply F_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x45,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RF, RF, WF, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULF) // Opcode Function
	},

	{
		"DIVF2", "Divide F_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x46,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, MF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVF) // Opcode Function
	},

	{
		"DIVF3", "Divide F_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x47,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RF, RF, WF, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVF) // Opcode Function
	},

	{
		"CVTFB", "Convert F_floating to byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x48,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, WB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTFB) // Opcode Function
	},

	{
		"CVTFW", "Convert F_floating to word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x49,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTFW) // Opcode Function
	},

	{
		"CVTFL", "Convert F_floating to longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x4A,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTFL) // Opcode Function
	},

	{
		"CVTRFL", "Convert rounded F_floating to longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x4B,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTRFL) // Opcode Function
	},

	{
		"CVTBF", "Convert byte to F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x4C,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTBF) // Opcode Function
	},

	{
		"CVTWF", "Convert word to F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x4D,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTWF) // Opcode Function
	},

	{
		"CVTLF", "Convert longword to F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x4E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, WF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTLF) // Opcode Function
	},

	{
		"ACBF", "Add compare and branch F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x4F,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RF, RF, MF, BW, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ACBF) // Opcode Function
	},

	{
		"MOVF", "Move F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x50,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, WF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVF) // Opcode Function
	},

	{
		"CMPF", "Compare F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x51,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, RF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPF) // Opcode Function
	},

	{
		"MNEGF", "Move negated F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x52,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, WF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MNEGF) // Opcode Function
	},

	{
		"TSTF", "Test F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x53,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RF, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, TSTF) // Opcode Function
	},

	{
		"EMODF", "Extended modulus F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x54,   // Opcode (Extended + Normal)
		5,            // Number of Operands
		{ RF, RB, RF, WL, WF, 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, EMODF) // Opcode Function
	},

	{
		"POLYF", "Evaluate polynomial F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x55,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RF, RW, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, POLYF) // Opcode Function
	},

	{
		"CVTFD", "Convert F_floating to D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x56,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, WD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTFD) // Opcode Function
	},

	{
		"ADAWI", "Add aligned word interlocked",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x58,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADAWI) // Opcode Function
	},

	{
		"INSQHI", "Insert into queue at head, interlocked",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x5C,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AB, AQ, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, INSQHI) // Opcode Function
	},

	{
		"INSQTI", "Insert into queue at tail, interlocked",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x5D,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AB, AQ, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, INSQTI) // Opcode Function
	},

	{
		"REMQHI", "Remove from queue at head, interlocked",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x5E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AQ, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, REMQHI) // Opcode Function
	},

	{
		"REMQTI", "Remove from queue at tail, interlocked",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x5F,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AQ, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, REMQTI) // Opcode Function
	},

	{
		"ADDD2", "Add D_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x60,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, MD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDD) // Opcode Function
	},

	{
		"ADDD3", "Add D_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x61,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RD, RD, WD, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDD) // Opcode Function
	},

	{
		"SUBD2", "Subtract D_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x62,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, MD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBD) // Opcode Function
	},

	{
		"SUBD3", "Subtract D_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x63,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RD, RD, WD, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBD) // Opcode Function
	},

	{
		"MULD2", "Multiply D_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x64,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, MD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULD) // Opcode Function
	},

	{
		"MULD3", "Multiply D_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x65,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RD, RD, WD, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULD) // Opcode Function
	},

	{
		"DIVD2", "Divide D_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x66,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, MD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVD) // Opcode Function
	},

	{
		"DIVD3", "Divide D_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x67,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RD, RD, WD, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVD) // Opcode Function
	},

	{
		"CVTDB", "Convert D_floating to byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x68,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, WB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTDB) // Opcode Function
	},

	{
		"CVTDW", "Convert D_floating to word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x69,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTDW) // Opcode Function
	},

	{
		"CVTDL", "Convert D_floating to longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x6A,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTDL) // Opcode Function
	},

	{
		"CVTRDL", "Convert rounded D_floating to longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x6B,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTRDL) // Opcode Function
	},

	{
		"CVTBD", "Convert byte to D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x6C,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTBD) // Opcode Function
	},

	{
		"CVTWD", "Convert word to D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x6D,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTWD) // Opcode Function
	},

	{
		"CVTLD", "Convert longword to D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x6E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, WD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTLD) // Opcode Function
	},

	{
		"ACBD", "Add compare and branch D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x6F,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RD, RD, MD, BW, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ACBD) // Opcode Function
	},

	{
		"MOVD", "Move D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x70,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, WD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVD) // Opcode Function
	},

	{
		"CMPD", "Compare D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x71,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, RD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPD) // Opcode Function
	},

	{
		"MNEGD", "Move negated D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x72,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, WD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MNEGD) // Opcode Function
	},

	{
		"TSTD", "Test D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x73,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RD, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, TSTD) // Opcode Function
	},

	{
		"EMODD", "Extended modulus D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x74,   // Opcode (Extended + Normal)
		5,            // Number of Operands
		{ RD, RB, RD, WL, WD, 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, EMODD) // Opcode Function
	},

	{
		"POLYD", "Evaluate polynomial D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x75,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RD, RW, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, POLYD) // Opcode Function
	},

	{
		"CVTDF", "Convert D_floating to F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x76,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, WF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTDF) // Opcode Function
	},

	{
		"ASHL", "Arithmetic shift longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x78,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RL, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ASHL) // Opcode Function
	},

	{
		"ASHQ", "Arithmetic shift quadword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x79,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RQ, WQ, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ASHQ) // Opcode Function
	},

	{
		"EMUL", "Extended multiply",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7A,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RL, RL, WQ, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, EMUL) // Opcode Function
	},

	{
		"EDIV", "Extended divide",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7B,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RQ, WL, WL, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, EDIV) // Opcode Function
	},

	{
		"CLRQ", "Clear quadword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7C,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ WQ, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CLRQ) // Opcode Function
	},

	{
		"CLRD", "Clear D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7C,   // Opcode (Extended + Normal)
		1,            // Number of Operands: 1
		{ WD, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CLRQ) // Opcode Function
	},

	{
		"CLRG", "Clear G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7C,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ WG, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CLRQ) // Opcode Function
	},

	{
		"MOVQ", "Move quadword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7D,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RQ, WQ, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVQ) // Opcode Function
	},

	{
		"MOVAQ", "Move address of quadword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AQ, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"MOVAD", "Move address of D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AD, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"MOVAG", "Move address of G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AG, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"PUSHAQ", "Push address of quadword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7F,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AQ, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"PUSHAD", "Push address of D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7F,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AD, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"PUSHAG", "Push address of G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x7F,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AG, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"ADDB2", "Add byte 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x80,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, MB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDB) // Opcode Function
	},

	{
		"ADDB3", "Add byte 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x81,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RB, WB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDB) // Opcode Function
	},

	{
		"SUBB2", "Subtract byte 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x82,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, MB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBB) // Opcode Function
	},

	{
		"SUBB3", "Subtract byte 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x83,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RB, WB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBB) // Opcode Function
	},

	{
		"MULB2", "Multiply byte 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x84,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, MB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULB) // Opcode Function
	},

	{
		"MULB3", "Multiply byte 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x85,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RB, WB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULB) // Opcode Function
	},

	{
		"DIVB2", "Divide byte 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x86,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, MB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVB) // Opcode Function
	},

	{
		"DIVB3", "Divide byte 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x87,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RB, WB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVB) // Opcode Function
	},

	{
		"BISB2", "Bit set byte 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x88,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, MB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BISB) // Opcode Function
	},

	{
		"BISB3", "Bit set byte 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x89,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RB, WB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BISB) // Opcode Function
	},

	{
		"BICB2", "Bit clear byte 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x8A,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, MB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BICB) // Opcode Function
	},

	{
		"BICB3", "Bit clear byte 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x8B,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RB, WB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BICB) // Opcode Function
	},

	{
		"XORB2", "Exclusive-OR byte 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x8C,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, MB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, XORB) // Opcode Function
	},

	{
		"XORB3", "Exclusive-OR byte 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x8D,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RB, WB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, XORB) // Opcode Function
	},

	{
		"MNEGB", "Move negated byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x8E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MNEGB) // Opcode Function
	},

	{
		"CASEB", "Case byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x8F,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RB, RB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CASEB) // Opcode Function
	},

	{
		"MOVB", "Move byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x90,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVB) // Opcode Function
	},

	{
		"CMPB", "Compare byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x91,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, RB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPB) // Opcode Function
	},

	{
		"MCOMB", "Move complemented byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x92,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MCOMB) // Opcode Function
	},

	{
		"BITB", "Bit test byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x93,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, RB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BITB) // Opcode Function
	},

	{
		"CLRB", "Clear byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x94,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ WB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CLRB) // Opcode Function
	},

	{
		"TSTB", "Test byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x95,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, TSTB) // Opcode Function
	},

	{
		"INCB", "Increment byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x96,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ MB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, INCB) // Opcode Function
	},

	{
		"DECB", "Decrement byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x97,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ MB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DECB) // Opcode Function
	},

	{
		"CVTBL", "Convert byte to longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x98,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTBL) // Opcode Function
	},

	{
		"CVTBW", "Convert byte to word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x99,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTBW) // Opcode Function
	},

	{
		"MOVZBL", "Move zero-extended byte to longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x9A,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVZBL) // Opcode Function
	},

	{
		"MOVZBW", "Move zero-extended byte to word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x9B,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVZBW) // Opcode Function
	},

	{
		"ROTL", "Rotate longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x9C,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RB, RL, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ROTL) // Opcode Function
	},

	{
		"ACBB", "Add compare and branch byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x9D,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RB, RB, MB, BW, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ACBB) // Opcode Function
	},

	{
		"MOVAB", "Move address of byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x9E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AB, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"PUSHAB", "Move address of byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0x9F,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AB, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"ADDW2", "Add word 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA0,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, MW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDW) // Opcode Function
	},

	{
		"ADDW3", "Add Word 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA1,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, RW, WW, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDW) // Opcode Function
	},

	{
		"SUBW2", "Subtract Word 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA2,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, MW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBW) // Opcode Function
	},

	{
		"SUBW3", "Subtract word 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA3,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, RW, WW, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBW) // Opcode Function
	},

	{
		"MULW2", "Multiply word 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA4,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, MW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULW) // Opcode Function
	},

	{
		"MULW3", "Multiply word 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA5,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, RW, WW, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULW) // Opcode Function
	},

	{
		"DIVW2", "Divide word 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA6,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, MW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVW) // Opcode Function
	},

	{
		"DIVW3", "Divide word 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA7,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, RW, WW, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVW) // Opcode Function
	},

	{
		"BISW2", "Bit set word 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA8,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, MW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BISW) // Opcode Function
	},

	{
		"BISW3", "Bit set word 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xA9,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, RW, WW, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BISW) // Opcode Function
	},

	{
		"BICW2", "Bit clear word 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xAA,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, MW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BICW) // Opcode Function
	},

	{
		"BICW3", "Bit clear word 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xAB,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, RW, WW, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BICW) // Opcode Function
	},

	{
		"XORW2", "Exclusive-OR word 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xAC,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, MW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, XORW) // Opcode Function
	},

	{
		"XORW3", "Exclusive-OR word 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xAD,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, RW, WW, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, XORW) // Opcode Function
	},

	{
		"MNEGW", "Move negated word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xAE,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MNEGW) // Opcode Function
	},

	{
		"CASEW", "Case word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xAF,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RW, RW, RW, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CASEW) // Opcode Function
	},

	{
		"MOVW", "Move word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB0,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVW) // Opcode Function
	},

	{
		"CMPW", "Compare word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB1,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, RW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPW) // Opcode Function
	},

	{
		"MCOMW", "Move complemented word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB2,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MCOMW) // Opcode Function
	},

	{
		"BITW", "Bit test word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB3,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, RW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BITW) // Opcode Function
	},

	{
		"CLRW", "Clear word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB4,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ WW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CLRW) // Opcode Function
	},

	{
		"TSTW", "Test word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB5,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, TSTW) // Opcode Function
	},

	{
		"INCW", "Increment word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB6,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ MW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, INCW) // Opcode Function
	},

	{
		"DECW", "Decrement word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB7,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ MW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DECW) // Opcode Function
	},

	{
		"BISPSW", "Bit set program status word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB8,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BISPSW) // Opcode Function
	},

	{
		"BICPSW", "Bit clear program status word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xB9,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BICPSW) // Opcode Function
	},

	{
		"POPR", "Pop registers",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xBA,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, POPR) // Opcode Function
	},

	{
		"PUSHR", "Push registers",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xBB,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PUSHR) // Opcode Function
	},

	{
		"CHMK", "Change mode to kernel",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xBC,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CHMK) // Opcode Function
	},

	{
		"CHME", "Change mode to executive",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xBD,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CHME) // Opcode Function
	},

	{
		"CHMS", "Change mode to supervisor",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xBE,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CHMS) // Opcode Function
	},

	{
		"CHMU", "Change mode to user",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xBF,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CHMU) // Opcode Function
	},

	{
		"ADDL2", "Add longword 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC0,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, ML, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDL) // Opcode Function
	},

	{
		"ADDL3", "Add longword 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC1,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, RL, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDL) // Opcode Function
	},

	{
		"SUBL2", "Subtract longword 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC2,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, ML, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBL) // Opcode Function
	},

	{
		"SUBL3", "Subtract longword 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC3,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, RL, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBL) // Opcode Function
	},

	{
		"MULL2", "Multiply longword 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC4,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, ML, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULL) // Opcode Function
	},

	{
		"MULL3", "Multiply longword 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC5,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, RL, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULL) // Opcode Function
	},

	{
		"DIVL2", "Divide longword 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC6,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, ML, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVL) // Opcode Function
	},

	{
		"DIVL3", "Divide longword 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC7,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, RL, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVL) // Opcode Function
	},

	{
		"BISL2", "Bit set longword 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC8,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, ML, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BISL) // Opcode Function
	},

	{
		"BISL3", "Bit set longword 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xC9,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, RL, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BISL) // Opcode Function
	},

	{
		"BICL2", "Bit clear longword 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xCA,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, ML, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BICL) // Opcode Function
	},

	{
		"BICL3", "Bit clear longword 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xCB,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, RL, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BICL) // Opcode Function
	},

	{
		"XORL2", "Exclusive-OR longword 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xCC,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, ML, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, XORL) // Opcode Function
	},

	{
		"XORL3", "Exclusive-OR longword 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xCD,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, RL, WL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, XORL) // Opcode Function
	},

	{
		"MNEGL", "Move negated longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xCE,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MNEGL) // Opcode Function
	},

	{
		"CASEL", "Case longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xCF,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, RL, RL, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CASEL) // Opcode Function
	},

	{
		"MOVL", "Move longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD0,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"CMPL", "Compare longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD1,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, RL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPL) // Opcode Function
	},

	{
		"MCOML", "Move complemented longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD2,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MCOML) // Opcode Function
	},

	{
		"BITL", "Bit test longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD3,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, RL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BITL) // Opcode Function
	},

	{
		"CLRL", "Clear longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD4,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ WL, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CLRL) // Opcode Function
	},

	{
		"CLRF", "Clear longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD4,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ WF, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CLRL) // Opcode Function
	},

	{
		"TSTL", "Test longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD5,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RL, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, TSTL) // Opcode Function
	},

	{
		"INCL", "Increment longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD6,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ ML, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, INCL) // Opcode Function
	},

	{
		"DECL", "Decrement longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD7,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ ML, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DECL) // Opcode Function
	},

	{
		"ADWC", "Add with carry",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD8,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, ML, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADWC) // Opcode Function
	},

	{
		"SBWC", "Subtract with carry",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xD9,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, ML, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SBWC) // Opcode Function
	},

	{
		"MTPR", "Move to processor register",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xDA,     // Opcode (Extended + Normal)
		2,              // Number of Operands
		{ RL, RL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,             // Profiling Data
		DEF_NAME(vax, MTPR) // Opcode Function
	},

	{
		"MFPR", "Move from processor register",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xDB,     // Opcode (Extended + Normal)
		2,              // Number of Operands
		{ RL, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,             // Profiling Data
		DEF_NAME(vax, MFPR) // Opcode Function
	},

	{
		"MOVPSL", "Move program status longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xDC,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ WL, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVPSL) // Opcode Function
	},

	{
		"PUSHL", "Push longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xDD,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RL, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"MOVAL", "Move address of longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xDE,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AL, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"MOVAF", "Move address of F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xDE,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AF, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"PUSHAL", "Push address of longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xDF,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AL, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"PUSHAF", "Push address of F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xDF,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AF, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"BBS", "Branch on bit set",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE0,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, VB, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BBS) // Opcode Function
	},

	{
		"BBC", "Branch on bit clear",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE1,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, VB, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BBC) // Opcode Function
	},

	{
		"BBSS", "Branch on bit set and set",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE2,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, VB, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BBSS) // Opcode Function
	},

	{
		"BBCS", "Branch on bit clear and set",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE3,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, VB, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BBCS) // Opcode Function
	},

	{
		"BBSC", "Branch on bit set and clear",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE4,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, VB, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BBSC) // Opcode Function
	},

	{
		"BBCC", "Branch on bit clear and clear",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE5,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, VB, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BBCC) // Opcode Function
	},

	{
		"BBSSI", "Branch on bit set and set interlocked",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE6,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, VB, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BBSS) // Opcode Function
	},

	{
		"BBCCI", "Branch on bit clear and clear interlocked",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE7,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, VB, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BBCC) // Opcode Function
	},

	{
		"BLBS", "Branch on low bit set",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE8,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, BB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BLBS) // Opcode Function
	},

	{
		"BLBC", "Branch on low bit clear",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xE9,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, BB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BLBC) // Opcode Function
	},

	{
		"FFS", "Find first set bit",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xEA,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RB, VB, WL, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, FFS) // Opcode Function
	},

	{
		"FFC", "Find first clear bit",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xEB,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RB, VB, WL, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, FFC) // Opcode Function
	},

	{
		"CMPV", "Compare field",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xEC,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RB, VB, RL, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPV) // Opcode Function
	},

	{
		"CMPZV", "Compare zero-extended field",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xED,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RB, VB, RL, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPZV) // Opcode Function
	},

	{
		"EXTV", "Extract field",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xEE,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RB, VB, WL, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, EXTV) // Opcode Function
	},

	{
		"EXTZV", "Extract zero-extended field",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xEF,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RB, VB, WL, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, EXTZV) // Opcode Function
	},

	{
		"INSV", "Insert field",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xF0,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RL, RB, VB, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, INSV) // Opcode Function
	},

	{
		"ACBL", "Add compare and branch longword",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xF1,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RL, RL, ML, BW, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ACBL) // Opcode Function
	},

	{
		"AOBLSS", "Add one and branch on less",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xF2,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, ML, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, AOBLSS) // Opcode Function
	},

	{
		"AOBLEQ", "Add one and branch on less or equal",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xF3,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, ML, BB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, AOBLEQ) // Opcode Function
	},

	{
		"SOBGEQ", "Subtract one and branch on greater or equal",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xF4,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ ML, BB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SOBGEQ) // Opcode Function
	},

	{
		"SOBGTR", "Subtract one and branch on greater",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xF5,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ ML, BB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SOBGTR) // Opcode Function
	},

	{
		"CVTLB", "Convert longword to byte",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xF6,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, WB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTLB) // Opcode Function
	},

	{
		"CVTLW", "Convert longword to word",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xF7,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTLW) // Opcode Function
	},

	{
		"ASHP", "Arithmetic shift and round packed",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0xF8,   // Opcode (Extended + Normal)
		6,            // Number of Operands
		{ RB, RW, AB, RB, RW, AB }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTLP", "Convert longword to packed",
		ISTR_EMULATE, // Instruction Flags
		0x00, 0xF9,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RL, RW, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CALLG", "Call with general argument list",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xFA,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AB, AB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CALLG) // Opcode Function
	},

	{
		"CALLS", "Call with stack",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xFB,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, AB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CALLS) // Opcode Function
	},

	{
		"XFC", "Extended function call",
		ISTR_NORMAL,  // Instruction Flags
		0x00, 0xFC,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, XFC) // Opcode Function
	},

	{
		"MFVP", "Move from vector processor",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x31,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTDH", "Convert D_floating to H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x32,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RD, WH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTGF", "Convert G_floating to F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x33,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, WF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTGF) // Opcode Function
	},

	{
		"VLDL", "Load longword vector from memory to vector register",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x34,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VGATHL", "Gather longword vector from memory to vector register",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x35,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VLDQ", "Load quadword vector from memory to vector register",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x36,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VGATHQ", "Gather quadword vector from memory to vector register",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x37,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"ADDG2", "Add G_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x40,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, MG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDG) // Opcode Function
	},

	{
		"ADDG3", "Add G_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x41,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RG, RG, WG, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ADDG) // Opcode Function
	},

	{
		"SUBG2", "Subtract G_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x42,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, MG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBG) // Opcode Function
	},

	{
		"SUBG3", "Subtract G_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x43,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RG, RG, WG, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, SUBG) // Opcode Function
	},

	{
		"MULG2", "Multiply G_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x44,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, MG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULG) // Opcode Function
	},

	{
		"MULG3", "Multiply G_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x45,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RG, RG, WG, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MULG) // Opcode Function
	},

	{
		"DIVG2", "Divide G_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x46,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, MG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVG) // Opcode Function
	},

	{
		"DIVG3", "Divide G_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x47,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RG, RG, WG, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, DIVG) // Opcode Function
	},

	{
		"CVTGB", "Convert G_floating to byte",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x48,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, WB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTGB) // Opcode Function
	},

	{
		"CVTGW", "Convert G_floating to word",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x49,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTGW) // Opcode Function
	},

	{
		"CVTGL", "Convert G_floating to longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x4A,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTGL) // Opcode Function
	},

	{
		"CVTRGL", "Convert rounded G_floating to longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x4B,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTRGL) // Opcode Function
	},

	{
		"CVTBG", "Convert byte to G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x4C,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTBG) // Opcode Function
	},

	{
		"CVTWG", "Convert word to G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x4D,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTWG) // Opcode Function
	},

	{
		"CVTLG", "Convert longword to G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x4E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, WG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTLG) // Opcode Function
	},

	{
		"ACBG", "Add compare and branch G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x4F,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RG, RG, WG, BW, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, ACBG) // Opcode Function
	},

	{
		"MOVG", "Move G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x50,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, WG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MOVG) // Opcode Function
	},

	{
		"CMPG", "Compare G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x51,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, RG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CMPG) // Opcode Function
	},

	{
		"MNEGG", "Move negated G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x52,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, WG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, MNEGG) // Opcode Function
	},

	{
		"TSTG", "Test G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x53,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RG, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, TSTG) // Opcode Function
	},

	{
		"EMODG", "Extended modulus G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x54,   // Opcode (Extended + Normal)
		5,            // Number of Operands
		{ RG, RW, RG, WL, WG, 0  },
		0L,           // Profiling Data
		DEF_NAME(vax, EMODG) // Opcode Function
	},

	{
		"POLYG", "Evaluate polynomial G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x55,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RG, RW, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, POLYG) // Opcode Function
	},

	{
		"CVTGH", "Convert G_floating to H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x56,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RG, WH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"ADDH2", "Add H_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x60,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, MH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"ADDH3", "Add H_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x61,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RH, RH, WH, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"SUBH2", "Subtract H_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x62,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, MH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"SUBH3", "Subtract H_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x63,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RH, RH, WH, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MULH2", "Multiply H_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x64,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, MH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MULH3", "Multiply H_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x65,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RH, RH, WH, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"DIVH2", "Divide H_floating 2 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x66,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, MH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"DIVH3", "Divide H_floating 3 operand",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x67,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RH, RH, WH, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTHB", "Convert H_floating to byte",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x68,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, WB, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTHW", "Convert H_floating to word",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x69,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, WW, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTHL", "Convert H_floating to longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x6A,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTRHL", "Convert rounded H_floating to longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x6B,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTBH", "Convert byte to H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x6C,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RB, WH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTWH", "Convert word to H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x6D,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RW, WH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTLH", "Convert longword to H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x6E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RL, WH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"ACBH", "Add compare and branch H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x6F,   // Opcode (Extended + Normal)
		4,            // Number of Operands
		{ RH, RH, WH, BW, 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MOVH", "Move H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x70,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, WH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CMPH", "Compare H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x71,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, RH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MNEGH", "Move negated H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x72,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, WH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"TSTH", "Test H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x73,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ RH, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"EMODH", "Extended modulus H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x74,   // Opcode (Extended + Normal)
		5,            // Number of Operands
		{ RH, RW, RH, WL, WH, 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"POLYH", "Evaluate polynomial H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x75,   // Opcode (Extended + Normal)
		3,            // Number of Operands
		{ RH, RW, AB, 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTHG", "Convert H_floating to G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x76,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, RG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CLRO", "Clear octaword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x7C,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ WO, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CLRH", "Clear H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x7C,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ WH, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MOVO", "Move octaword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x7D,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RO, WO, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MOVAO", "Move address of octaword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x7E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AO, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL,
//		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"MOVAH", "Move address of H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x7E,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ AH, WL, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL,
//		DEF_NAME(vax, MOVL) // Opcode Function
	},

	{
		"PUSHAO", "Push address of octaword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x7F,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AO, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
//		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"PUSHAH", "Push address of H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x7F,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ AH, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
//		DEF_NAME(vax, PUSHL) // Opcode Function
	},

	{
		"VVADDL", "Vector vector add longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x80,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSADDL", "Vector scalar add longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x81,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVADDG", "Vector vector add G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x82,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSADDG", "Vector scalar add G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x83,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVADDF", "Vector vector add F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x84,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSADDF", "Vector scalar add F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x85,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVADDD", "Vector vector add D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x86,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSADDD", "Vector scalar add D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x87,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVSUBL", "Vector vector subtract longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x88,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSSUBL", "Vector scalar subtract longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x89,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVSUBG", "Vector vector subtract G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x8A,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSSUBG", "Vector scalar subtract G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x8B,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVSUBF", "Vector vector subtract F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x8C,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSSUBF", "Vector scalar subtract F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x8D,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVSUBD", "Vector vector subtract D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x8E,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSSUBD", "Vector scalar subtract D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x8F,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTFH", "Convert F_floating to H_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x98,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, WH, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTFG", "Convert F_floating to G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x99,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RF, WG, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, CVTFG) // Opcode Function
	},

	{
		"VSTL", "Store longword vector from vector register to memory",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x9C,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSCATL", "Scatter longword vector from vector register to memory",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x9D,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSTQ", "Store quadword vector from vector register to memory",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x9E,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSCATQ", "Scatter quadword vector from vector register to memory",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0x9F,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVMULL", "Vector vector multiply longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA0,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSMULL", "Vector scalar multiply longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA1,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVMULG", "Vector vector multiply G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA2,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSMULG", "Vector scalar multiply G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA3,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVMULF", "Vector vector multiply F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA4,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSMULF", "Vector scalar multiply F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA5,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVMULD", "Vector vector multiply D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA6,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSMULD", "Vector scalar multiply D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA7,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSYNC", "Synchronize vector memory access",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA8,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"MTVP", "Move to vector processor",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xA9,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVDIVG", "Vector vector divide G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xAA,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSDIVG", "Vector scalar divide G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xAB,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVDIVF", "Vector vector divide F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xAC,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSDIVF", "Vector scalar divide F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xAD,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVDIVD", "Vector vector divide D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xAE,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSDIVD", "Vector scalar divide D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xAF,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVCMPL", "Vector vector compare longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC0,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSCMPL", "Vector scalar compare longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC1,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVCMPG", "Vector vector compare G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC2,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSCMPG", "Vector scalar compare G_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC3,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVCMPF", "Vector vector compare F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC4,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSCMPF", "Vector scalar compare F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC5,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVCMPD", "Vector vector compare D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC6,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSCMPD", "Vector scalar compare D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC7,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVBISL", "Vector vector bit set longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC8,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSBISL", "Vector scalar bit set longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xC9,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVBICL", "Vector vector bit clear longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xCC,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSBICL", "Vector scalar bit clear longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xCD,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVSRLL", "Vector vector shift right logical longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xE0,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSSRLL", "Vector scalar shift right logical longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xE1,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVSLLL", "Vector vector shift left logical longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xE4,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSSLLL", "Vector scalar shift left logical longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xE5,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVXORL", "Vector vector exclusive-OR longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xE8,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSXORL", "Vector scalar exclusive-OR longword",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xE9,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVCVT", "Vector convert",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xEC,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"IOTA", "Generate compressed iota vector",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xED,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VVMERGE", "Vector vector merge",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xEE,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"VSMERGE", "Vector scalar merge",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xEF,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ 0 , 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTHF", "Convert H_floating to F_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xF6,   // Opcode (Extended + Normal)
		2,            // Number of Operands
		{ RH, WF, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"CVTHD", "Convert H_floating to D_floating",
		ISTR_NORMAL,  // Instruction Flags
		0xFD, 0xF7,   // Opcode (Extended + Normal)
		0,            // Number of Operands
		{ RH, WD, 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		NULL          // Execute Routine
	},

	{
		"BUGL", "Bug Check Long",
		ISTR_NORMAL,  // Instruction Flags
		0xFF, 0xFD,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ IL, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BUGL) // Opcode Function
	},

	{
		"BUGW", "Bug Check Word",
		ISTR_NORMAL,  // Instruction Flags
		0xFF, 0xFE,   // Opcode (Extended + Normal)
		1,            // Number of Operands
		{ IW, 0 , 0 , 0 , 0 , 0  }, // Operand Scale/Mode
		0L,           // Profiling Data
		DEF_NAME(vax, BUGW) // Opcode Function
	},

	// Null terminator here
	{ NULL }
};
