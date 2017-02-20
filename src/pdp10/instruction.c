// instruction.c - A listing of PDP-6/PDP-10 instructions
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

#include "pdp10/defs.h"

INSTRUCTION pdp10_Instruction[] =
{
	{
		"UUO",       // Name of the Instruction
		"Undefined Opcode",
		OP_ALL,      // Opcode Flags
		0000, 0000,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		{            // Execute Routines
			NULL,     // PDP6 Processor
			NULL,     // KA10 Processor
			NULL,     // KI10 Processor
			NULL,     // KL10 Prcoessor
			NULL,     // KS10 Processor
		}
	},

	{
		"GFAD",      // Name of the Instruction
		"G-Format Floating Add",
		OP_KL|OP_E271, // Opcode Flags
		0000, 0102,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GFSB",      // Name of the Instruction
		"G-Format Floating Subtract",
		OP_KL|OP_E271, // Opcode Flags
		0000, 0103,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ADJSP",     // Name of the Instruction
		"Adjust Stack Pointer",
		OP_KL|OP_KS, // Opcode Flags
		0000, 0105,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GFMP",     // Name of the Instruction
		"G-Format Floating Multiply",
		OP_KL|OP_E271, // Opcode Flags
		0000, 0106,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GFDV",     // Name of the Instruction
		"G-Format Floating Divide",
		OP_KL|OP_E271, // Opcode Flags
		0000, 0107,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DFAD",     // Name of the Instruction
		"Double Floating Add",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0110,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DFSB",     // Name of the Instruction
		"Double Floating Subtract",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0111,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DFMP",     // Name of the Instruction
		"Double Floating Multiply",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0112,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DFDV",     // Name of the Instruction
		"Double Floating Divide",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0113,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DADD",      // Name of the Instruction
		"Double Add",
		OP_KL|OP_KS, // Opcode Flags
		0000, 0114,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DSUB",      // Name of the Instruction
		"Double Subtract",
		OP_KL|OP_KS, // Opcode Flags
		0000, 0115,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DMUL",      // Name of the Instruction
		"Double Multiply",
		OP_KL|OP_KS, // Opcode Flags
		0000, 0116,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DDIV",      // Name of the Instruction
		"Double Divide",
		OP_KL|OP_KS, // Opcode Flags
		0000, 0117,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DMOVE",     // Name of the Instruction
		"Double Move to AC",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0120,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DMOVN",     // Name of the Instruction
		"Double Move Negative to AC",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0121,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FIX",       // Name of the Instruction
		"Fix",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0122,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"EXTEND",    // Name of the Instruction
		"Extended Function",
		OP_KL|OP_KS, // Opcode Flags
		0000, 0123,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DMOVEM",    // Name of the Instruction
		"Double Move to Memory",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0124,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DMOVNM",    // Name of the Instruction
		"Double Move Negative to Memory",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0125,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FIXR",     // Name of the Instruction
		"Fix and Round",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0126,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FLTR",     // Name of the Instruction
		"Float and Round",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0127,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"UFA",     // Name of the Instruction
		"Unnormalized Floating Add",
		OP_KA|OP_KI|OP_KL, // Opcode Flags
		0000, 0130,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DFN",     // Name of the Instruction
		"Double Floating Negate",
		OP_KA|OP_KI|OP_KL, // Opcode Flags
		0000, 0131,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FSC",       // Name of the Instruction
		"Floating Scale",
		OP_ALL,      // Opcode Flags
		0000, 0132,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IBP",       // Name of the Instruction
		"Increment Byte Pointer",
		OP_ALL|OP_AC|OP_Z, // Opcode Flags
		0000, 0133,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ADJBP",       // Name of the Instruction
		"Adjust Byte Pointer",
		OP_ALL|OP_AC|OP_NZ|OP_IMM, // Opcode Flags
		0000, 0133,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ILDB",       // Name of the Instruction
		"Increment Pointer and Load Byte into AC",
		OP_ALL,      // Opcode Flags
		0000, 0134,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"LDB",       // Name of the Instruction
		"Load Byte into AC",
		OP_ALL,      // Opcode Flags
		0000, 0135,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IDPB",       // Name of the Instruction
		"Increment Pointer and Deposit Byte in Memory",
		OP_ALL,      // Opcode Flags
		0000, 0136,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DPB",       // Name of the Instruction
		"Deposit Byte in Memory",
		OP_ALL,      // Opcode Flags
		0000, 0137,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FAD",       // Name of the Instruction
		"Floating Add",
		OP_ALL,      // Opcode Flags
		0000, 0140,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FADL",       // Name of the Instruction
		"Floating Add Long",
		OP_KA|OP_KI|OP_KL, // Opcode Flags
		0000, 0141,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FADM",      // Name of the Instruction
		"Floating Add to Memory",
		OP_ALL,      // Opcode Flags
		0000, 0142,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FADB",      // Name of the Instruction
		"Floating Add to Both (AC and Memory)",
		OP_ALL,      // Opcode Flags
		0000, 0143,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FADR",      // Name of the Instruction
		"Floating Add and Round",
		OP_ALL,      // Opcode Flags
		0000, 0144,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FADRI",      // Name of the Instruction
		"Floating Add and Round Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0145,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FADRM",     // Name of the Instruction
		"Floating Add and Round to Memory",
		OP_ALL,      // Opcode Flags
		0000, 0146,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FADRB",     // Name of the Instruction
		"Floating Add and Round to Both (AC and Memory)",
		OP_ALL,      // Opcode Flags
		0000, 0147,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FSB",       // Name of the Instruction
		"Floating Subtract",
		OP_ALL,      // Opcode Flags
		0000, 0150,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FSBL",       // Name of the Instruction
		"Floating Subtract Long",
		OP_KA|OP_KI|OP_KL, // Opcode Flags
		0000, 0151,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FSBM",      // Name of the Instruction
		"Floating Subtract to Memory",
		OP_ALL,      // Opcode Flags
		0000, 0152,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FSBB",      // Name of the Instruction
		"Floating Subtract to Both (AC and Memory)",
		OP_ALL,      // Opcode Flags
		0000, 0153,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FSBR",      // Name of the Instruction
		"Floating Subtract and Round",
		OP_ALL,      // Opcode Flags
		0000, 0154,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FSBRI",      // Name of the Instruction
		"Floating Subtract and Round Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0155,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FSBRM",     // Name of the Instruction
		"Floating Subtract and Round to Memory",
		OP_ALL,      // Opcode Flags
		0000, 0156,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FSBRB",     // Name of the Instruction
		"Floating Subtract and Round to Both (AC and Memory)",
		OP_ALL,      // Opcode Flags
		0000, 0157,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FMP",       // Name of the Instruction
		"Floating Multiply",
		OP_ALL,      // Opcode Flags
		0000, 0160,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FMPL",       // Name of the Instruction
		"Floating Multiply Long",
		OP_KA|OP_KI|OP_KL, // Opcode Flags
		0000, 0161,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FMPM",      // Name of the Instruction
		"Floating Multiply to Memory",
		OP_ALL,      // Opcode Flags
		0000, 0162,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FMPB",      // Name of the Instruction
		"Floating Multiply to Both (AC and Memory)",
		OP_ALL,      // Opcode Flags
		0000, 0163,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FMPR",      // Name of the Instruction
		"Floating Multiply and Round",
		OP_ALL,      // Opcode Flags
		0000, 0164,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FMPRI",      // Name of the Instruction
		"Floating Multiply and Round Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0165,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FMPRM",     // Name of the Instruction
		"Floating Multiply and Round to Memory",
		OP_ALL,      // Opcode Flags
		0000, 0166,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FMPRB",     // Name of the Instruction
		"Floating Multiply and Round to Both (AC and Memory)",
		OP_ALL,      // Opcode Flags
		0000, 0167,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FDV",       // Name of the Instruction
		"Floating Divide",
		OP_ALL,      // Opcode Flags
		0000, 0170,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FDVL",       // Name of the Instruction
		"Floating Divide Long",
		OP_KA|OP_KI|OP_KL, // Opcode Flags
		0000, 0171,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FDVM",      // Name of the Instruction
		"Floating Divide to Memory",
		OP_ALL,      // Opcode Flags
		0000, 0172,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FDVB",      // Name of the Instruction
		"Floating Divide to Both (AC and Memory)",
		OP_ALL,      // Opcode Flags
		0000, 0173,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FDVR",      // Name of the Instruction
		"Floating Divide and Round",
		OP_ALL,      // Opcode Flags
		0000, 0174,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FDVRI",      // Name of the Instruction
		"Floating Divide and Round Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0175,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FDVRM",     // Name of the Instruction
		"Floating Divide and Round to Memory",
		OP_ALL,      // Opcode Flags
		0000, 0176,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"FDVRB",     // Name of the Instruction
		"Floating Divide and Round to Both (AC and Memory)",
		OP_ALL,      // Opcode Flags
		0000, 0177,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVE",     // Name of the Instruction
		"Move to AC",
		OP_ALL,      // Opcode Flags
		0000, 0200,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVEI",     // Name of the Instruction
		"Move Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0201,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVEM",     // Name of the Instruction
		"Move to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0202,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVES",     // Name of the Instruction
		"Move to Self",
		OP_ALL,       // Opcode Flags
		0000, 0203,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVS",     // Name of the Instruction
		"Move Swapped to AC",
		OP_ALL,      // Opcode Flags
		0000, 0204,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVSI",     // Name of the Instruction
		"Move Swapped Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0205,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVSM",     // Name of the Instruction
		"Move Swapped to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0206,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVSS",     // Name of the Instruction
		"Move Swapped to Self",
		OP_ALL,       // Opcode Flags
		0000, 0207,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVN",     // Name of the Instruction
		"Move Negated to AC",
		OP_ALL,      // Opcode Flags
		0000, 0210,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVNI",     // Name of the Instruction
		"Move Negated Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0211,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVNM",     // Name of the Instruction
		"Move Negated to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0212,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVNS",     // Name of the Instruction
		"Move Negated to Self",
		OP_ALL,       // Opcode Flags
		0000, 0213,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVM",     // Name of the Instruction
		"Move Magnitude to AC",
		OP_ALL,      // Opcode Flags
		0000, 0214,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVMI",     // Name of the Instruction
		"Move Magnitude Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0215,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVMM",     // Name of the Instruction
		"Move Magnitude to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0216,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVMS",     // Name of the Instruction
		"Move Magnitude to Self",
		OP_ALL,       // Opcode Flags
		0000, 0217,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IMUL",     // Name of the Instruction
		"Integer Multiply to AC",
		OP_ALL,      // Opcode Flags
		0000, 0220,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IMULI",     // Name of the Instruction
		"Integer Multiply Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0221,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IMULM",     // Name of the Instruction
		"Integer Multiply to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0222,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IMULB",     // Name of the Instruction
		"Integer Multiply to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0223,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MUL",       // Name of the Instruction
		"Multiply to AC",
		OP_ALL,       // Opcode Flags
		0000, 0224,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MULI",      // Name of the Instruction
		"Multiply Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0225,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MULM",      // Name of the Instruction
		"Multiply to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0226,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MULB",      // Name of the Instruction
		"Multiply to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0227,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IDIV",     // Name of the Instruction
		"Integer Divide to AC",
		OP_ALL,      // Opcode Flags
		0000, 0230,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IDIVI",     // Name of the Instruction
		"Integer Divide Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0231,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IDIVM",     // Name of the Instruction
		"Integer Divide to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0232,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IDIVB",     // Name of the Instruction
		"Integer Divide to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0233,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DIV",       // Name of the Instruction
		"Divide to AC",
		OP_ALL,       // Opcode Flags
		0000, 0234,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DIVI",      // Name of the Instruction
		"Divide Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0235,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DIVM",      // Name of the Instruction
		"Divide to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0236,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DIVB",      // Name of the Instruction
		"Divide to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0237,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ASH",       // Name of the Instruction
		"Arithmetic Shift",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0240,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ROT",       // Name of the Instruction
		"Rotate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0241,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"LSH",       // Name of the Instruction
		"Logical Shift",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0242,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JFFO",      // Name of the Instruction
		"Jump if Find First One",
		OP_ALL,       // Opcode Flags
		0000, 0243,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ASHC",      // Name of the Instruction
		"Arithmetic Shift Combined",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0244,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ROTC",      // Name of the Instruction
		"Rotate Combined",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0245,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"LSHC",      // Name of the Instruction
		"Logical Shift Combined",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0246,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"EXCH",      // Name of the Instruction
		"Exchange AC and Memory",
		OP_ALL,       // Opcode Flags
		0000, 0250,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"BLT",       // Name of the Instruction
		"Block Transfer",
		OP_ALL,       // Opcode Flags
		0000, 0251,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOBJP",     // Name of the Instruction
		"Add one to both halves of AC and Jump if positive",
		OP_ALL,       // Opcode Flags
		0000, 0252,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOBJN",     // Name of the Instruction
		"Add one to both halves of AC and Jump if negative",
		OP_ALL,       // Opcode Flags
		0000, 0253,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JRST",      // Name of the Instruction
		"Jump and Restore",
		OP_ALL|OP_AC,// Opcode Flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"PORTAL",    // Name of the Instruction
		"Portal",
		OP_ALL|OP_AC|OP_01,// Opcode Flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JRSTF",      // Name of the Instruction
		"Jump and Restore Flags",
		OP_ALL|OP_AC|OP_02,// Opcode Flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HALT",      // Name of the Instruction
		"Halt",
		OP_ALL|OP_AC|OP_04,// Opcode Flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XJRSTF",    // Name of the Instruction
		"Extended Jump and Restore Flags",
		OP_KL|OP_KS|OP_AC|OP_05,// Opcode Flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XJEN",      // Name of the Instruction
		"Extended Jump and Enable PI Channel",
		OP_KL|OP_KS|OP_AC|OP_06,// Opcode Flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XPCW",      // Name of the Instruction
		"??",
		OP_KL|OP_KS|OP_AC|OP_07,// Opcode Flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JEN",      // Name of the Instruction
		"Jump and Enable PI Channel",
		OP_KL|OP_KS|OP_AC|OP_12,// Opcode Flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XSFM",      // Name of the Instruction
		"??",
		OP_KL|OP_KS|OP_AC|OP_14,// Opcode Flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XJRST",     // Name of the Instruction
		"Extended Jump and Restore Flags",
		OP_KL|OP_AC|OP_15, // Opcode flags
		0000, 0254,  // Extended+Opcode (Normally Zero)
		0L,          // Profile data
		NULL         // Execute Routine
	},

	{
		"JFCL",      // Name of the Instruction
		"Jump on Flag and Clear",
		OP_ALL|OP_AC,// Opcode Flags
		0000, 0255,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JFOV",      // Name of the Instruction
		"Jump on Floating Overflow",
		OP_ALL|OP_AC|OP_01,// Opcode Flags
		0000, 0255,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JCRY1",     // Name of the Instruction
		"Jump on Carry 1",
		OP_ALL|OP_AC|OP_02,// Opcode Flags
		0000, 0255,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JCRY0",     // Name of the Instruction
		"Jump on Carry 0",
		OP_ALL|OP_AC|OP_04,// Opcode Flags
		0000, 0255,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JCRY",      // Name of the Instruction
		"Jump on Carry",
		OP_ALL|OP_AC|OP_06,// Opcode Flags
		0000, 0255,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JOV",       // Name of the Instruction
		"Jump on Overflow",
		OP_ALL|OP_AC|OP_10,// Opcode Flags
		0000, 0255,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XCT",       // Name of the Instruction
		"Execute",
		OP_ALL|OP_AC|OP_Z,// Opcode Flags
		0000, 0256,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"PXCT",      // Name of the Instruction
		"Previous Context Execute",
		OP_ALL|OP_AC|OP_NZ, // Opcode Flags
		0000, 0256,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MAP",       // Name of the Instruction
		"Map to Physcial Address",
		OP_KI|OP_KL|OP_KS, // Opcode Flags
		0000, 0257,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"PUSHJ",     // Name of the Instruction
		"Push and Jump",
		OP_ALL,       // Opcode Flags
		0000, 0260,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"PUSH",      // Name of the Instruction
		"Push",
		OP_ALL,       // Opcode Flags
		0000, 0261,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"POP",       // Name of the Instruction
		"Pop",
		OP_ALL,       // Opcode Flags
		0000, 0262,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"POPJ",      // Name of the Instruction
		"Pop and Jump",
		OP_ALL,       // Opcode Flags
		0000, 0263,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JSR",       // Name of the Instruction
		"Jump to Subroutine",
		OP_ALL,       // Opcode Flags
		0000, 0264,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JSP",       // Name of the Instruction
		"Jump and Save PC",
		OP_ALL,       // Opcode Flags
		0000, 0265,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JSA",       // Name of the Instruction
		"Jump and Save AC",
		OP_ALL,       // Opcode Flags
		0000, 0266,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JRA",       // Name of the Instruction
		"Jump and Restore AC",
		OP_ALL,       // Opcode Flags
		0000, 0267,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ADD",       // Name of the Instruction
		"Add to AC",
		OP_ALL,       // Opcode Flags
		0000, 0270,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ADDI",      // Name of the Instruction
		"Add Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0271,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ADDM",      // Name of the Instruction
		"Add to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0272,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ADDB",      // Name of the Instruction
		"Add to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0273,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SUB",       // Name of the Instruction
		"Subtract to AC",
		OP_ALL,       // Opcode Flags
		0000, 0274,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SUBI",      // Name of the Instruction
		"Subtract Immediate to AC",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0275,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SUBM",      // Name of the Instruction
		"Subract to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0276,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SUBB",      // Name of the Instruction
		"Subtract to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0277,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAI",       // Name of the Instruction
		"Compare AC with Immediate and Skip Never",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0300,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAIL",      // Name of the Instruction
		"Compare AC with Immediate and Skip on Less",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0301,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAIE",      // Name of the Instruction
		"Compare AC with Immediate and Skip on Equal",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0302,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAILE",     // Name of the Instruction
		"Compare AC with Immediate and Skip on Less or Equal",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0303,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAIA",      // Name of the Instruction
		"Compare AC with Immediate and Skip Always",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0304,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAIGE",     // Name of the Instruction
		"Compare AC with Immediate and Skip on Greater or Equal",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0305,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAIN",      // Name of the Instruction
		"Compare AC with Immediate and Skip on not Equal",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0306,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAIG",      // Name of the Instruction
		"Compare AC with Immediate and Skip on Greater",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0307,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAM",       // Name of the Instruction
		"Compare AC with Memory and Skip Never",
		OP_ALL,       // Opcode Flags
		0000, 0310,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAML",      // Name of the Instruction
		"Compare AC with Memory and Skip on Less",
		OP_ALL,       // Opcode Flags
		0000, 0311,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAME",      // Name of the Instruction
		"Compare AC with Memory and Skip on Equal",
		OP_ALL,       // Opcode Flags
		0000, 0312,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAMLE",     // Name of the Instruction
		"Compare AC with Memory and Skip on Less or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0313,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAMA",      // Name of the Instruction
		"Compare AC with Memory and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0314,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAMGE",     // Name of the Instruction
		"Compare AC with Memory and Skip on Greater or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0315,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAMN",      // Name of the Instruction
		"Compare AC with Memory and Skip on not Equal",
		OP_ALL,       // Opcode Flags
		0000, 0316,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CAMG",      // Name of the Instruction
		"Compare AC with Memory and Skip on Greater",
		OP_ALL,       // Opcode Flags
		0000, 0317,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JUMP",      // Name of the Instruction
		"Jump Never",
		OP_ALL,       // Opcode Flags
		0000, 0320,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JUMPL",     // Name of the Instruction
		"Jump on Less",
		OP_ALL,       // Opcode Flags
		0000, 0321,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JUMPE",      // Name of the Instruction
		"Jump on Equal",
		OP_ALL,       // Opcode Flags
		0000, 0322,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JUMPLE",    // Name of the Instruction
		"Jump on Less or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0323,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JUMPA",     // Name of the Instruction
		"Jump Always",
		OP_ALL,       // Opcode Flags
		0000, 0324,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JUMPGE",    // Name of the Instruction
		"Jump on Greater or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0325,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JUMPN",     // Name of the Instruction
		"Jump on not Equal",
		OP_ALL,       // Opcode Flags
		0000, 0326,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JUMPG",     // Name of the Instruction
		"Jump on Greater",
		OP_ALL,       // Opcode Flags
		0000, 0327,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SKIP",      // Name of the Instruction
		"Skip Never",
		OP_ALL,       // Opcode Flags
		0000, 0330,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SKIPL",     // Name of the Instruction
		"Skip on Less",
		OP_ALL,       // Opcode Flags
		0000, 0331,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SKIPE",      // Name of the Instruction
		"Skip on Equal",
		OP_ALL,       // Opcode Flags
		0000, 0332,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SKIPLE",    // Name of the Instruction
		"Skip on Less or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0333,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SKIPA",     // Name of the Instruction
		"Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0334,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SKIPGE",    // Name of the Instruction
		"Skip on Greater or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0335,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SKIPN",     // Name of the Instruction
		"Skip on not Equal",
		OP_ALL,       // Opcode Flags
		0000, 0336,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SKIPG",     // Name of the Instruction
		"Skip on Greater",
		OP_ALL,       // Opcode Flags
		0000, 0337,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOJ",      // Name of the Instruction
		"Add one to AC and Jump Never",
		OP_ALL,       // Opcode Flags
		0000, 0340,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOJL",     // Name of the Instruction
		"Add one to AC and Jump on Less",
		OP_ALL,       // Opcode Flags
		0000, 0341,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOJE",      // Name of the Instruction
		"Add one to AC and Jump on Equal",
		OP_ALL,       // Opcode Flags
		0000, 0342,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOJLE",    // Name of the Instruction
		"Add one to AC and Jump on Less or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0343,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOJA",     // Name of the Instruction
		"Add one to AC and Jump Always",
		OP_ALL,       // Opcode Flags
		0000, 0344,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOJGE",    // Name of the Instruction
		"Add one to AC and Jump on Greater or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0345,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOJN",     // Name of the Instruction
		"Add one to AC and Jump on not Equal",
		OP_ALL,       // Opcode Flags
		0000, 0346,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOJG",     // Name of the Instruction
		"Add one to AC and Jump on Greater",
		OP_ALL,       // Opcode Flags
		0000, 0347,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOS",      // Name of the Instruction
		"Add one to Memory and Skip Never",
		OP_ALL,       // Opcode Flags
		0000, 0350,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOSL",     // Name of the Instruction
		"Add one to Memory and Skip on Less",
		OP_ALL,       // Opcode Flags
		0000, 0351,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOSE",      // Name of the Instruction
		"Add one to Memory and Skip on Equal",
		OP_ALL,       // Opcode Flags
		0000, 0352,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOSLE",    // Name of the Instruction
		"Add one to Memory and Skip on Less or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0353,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOSA",     // Name of the Instruction
		"Add one to Memory and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0354,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOSGE",    // Name of the Instruction
		"Add one to Memory and Skip on Greater or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0355,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOSN",     // Name of the Instruction
		"Add one to Memory and Skip on not Equal",
		OP_ALL,       // Opcode Flags
		0000, 0356,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AOSG",     // Name of the Instruction
		"Add one to Memory and Skip on Greater",
		OP_ALL,       // Opcode Flags
		0000, 0357,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOJ",      // Name of the Instruction
		"Subtract one from AC and Jump Never",
		OP_ALL,       // Opcode Flags
		0000, 0360,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOJL",     // Name of the Instruction
		"Subtract one from AC and Jump on Less",
		OP_ALL,       // Opcode Flags
		0000, 0361,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOJE",      // Name of the Instruction
		"Subtract one from AC and Jump on Equal",
		OP_ALL,       // Opcode Flags
		0000, 0362,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOJLE",    // Name of the Instruction
		"Subtract one from AC and Jump on Less or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0363,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOJA",     // Name of the Instruction
		"Subtract one from AC and Jump Always",
		OP_ALL,       // Opcode Flags
		0000, 0364,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOJGE",    // Name of the Instruction
		"Subtract one from AC and Jump on Greater or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0365,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOJN",     // Name of the Instruction
		"Subtract one from AC and Jump on not Equal",
		OP_ALL,       // Opcode Flags
		0000, 0366,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOJG",     // Name of the Instruction
		"Subtract one from AC and Jump on Greater",
		OP_ALL,       // Opcode Flags
		0000, 0367,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOS",      // Name of the Instruction
		"Subtract one from Memory and Skip Never",
		OP_ALL,       // Opcode Flags
		0000, 0370,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOSL",     // Name of the Instruction
		"Subtract one from Memory and Skip on Less",
		OP_ALL,       // Opcode Flags
		0000, 0371,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOSE",      // Name of the Instruction
		"Subtract one from Memory and Skip on Equal",
		OP_ALL,       // Opcode Flags
		0000, 0372,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOSLE",    // Name of the Instruction
		"Subtract one from Memory and Skip on Less or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0373,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOSA",     // Name of the Instruction
		"Subtract one from Memory and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0374,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOSGE",    // Name of the Instruction
		"Subtract one from Memory and Skip on Greater or Equal",
		OP_ALL,       // Opcode Flags
		0000, 0375,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOSN",     // Name of the Instruction
		"Subtract one from Memory and Skip on not Equal",
		OP_ALL,       // Opcode Flags
		0000, 0376,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SOSG",     // Name of the Instruction
		"Subtract one from Memory and Skip on Greater",
		OP_ALL,       // Opcode Flags
		0000, 0377,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETZ",      // Name of the Instruction
		"Set Zeros to AC",
		OP_ALL,       // Opcode Flags
		0000, 0400,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETZI",     // Name of the Instruction
		"Set Zeros Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0401,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETZM",     // Name of the Instruction
		"Set Zeros to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0402,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETZB",     // Name of the Instruction
		"Set Zeros to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0403,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"AND",       // Name of the Instruction
		"AND to AC",
		OP_ALL,       // Opcode Flags
		0000, 0404,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDI",      // Name of the Instruction
		"AND Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0405,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDM",      // Name of the Instruction
		"AND to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0406,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDB",      // Name of the Instruction
		"AND to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0407,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCA",     // Name of the Instruction
		"AND with complement of AC to AC",
		OP_ALL,       // Opcode Flags
		0000, 0410,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCAI",    // Name of the Instruction
		"AND with complement of AC Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0411,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCAM",    // Name of the Instruction
		"AND with complement of AC to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0412,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCAB",    // Name of the Instruction
		"AND with complement of AC to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0413,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETM",      // Name of the Instruction
		"Set Memory to AC",
		OP_ALL,       // Opcode Flags
		0000, 0414,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETMI",     // Name of the Instruction
		"Set Memory Immediate",
		OP_KA|OP_KI|OP_IMM, // Opcode Flags
		0000, 0415,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XMOVEI",    // Name of the Instruction
		"Extended Move Imemdiate",
		OP_KL|OP_KS|OP_IMM, // Opcode Flags
		0000, 0415,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETMM",     // Name of the Instruction
		"Set Memory to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0416,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETMB",     // Name of the Instruction
		"Set Memory to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0417,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCM",     // Name of the Instruction
		"AND with complement of Memory to AC",
		OP_ALL,       // Opcode Flags
		0000, 0420,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCMI",    // Name of the Instruction
		"AND with complement of Memory Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0421,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCMM",    // Name of the Instruction
		"AND with complement of Memory to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0422,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCMB",    // Name of the Instruction
		"AND with complement of Memory to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0423,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETA",      // Name of the Instruction
		"Set AC to AC",
		OP_ALL,       // Opcode Flags
		0000, 0424,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETAI",     // Name of the Instruction
		"Set AC Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0425,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETAM",     // Name of the Instruction
		"Set AC to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0426,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETAB",     // Name of the Instruction
		"Set AC to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0427,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XOR",       // Name of the Instruction
		"Exclusive OR to AC",
		OP_ALL,       // Opcode Flags
		0000, 0430,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XORI",      // Name of the Instruction
		"Exclusive OR Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0431,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XORM",      // Name of the Instruction
		"Exclusive OR to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0432,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XORB",      // Name of the Instruction
		"Exclusive OR to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0433,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IOR",       // Name of the Instruction
		"Inclusive OR to AC",
		OP_ALL,       // Opcode Flags
		0000, 0434,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IORI",      // Name of the Instruction
		"Inclusive OR Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0435,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IORM",      // Name of the Instruction
		"Inclusive OR to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0436,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IORB",      // Name of the Instruction
		"Inclusive OR to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0437,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"OR",       // Name of the Instruction
		"OR to AC",
		OP_ALL,       // Opcode Flags
		0000, 0434,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORI",      // Name of the Instruction
		"OR Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0435,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORM",      // Name of the Instruction
		"OR to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0436,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORB",      // Name of the Instruction
		"OR to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0437,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCB",     // Name of the Instruction
		"AND with complement of Both to AC",
		OP_ALL,       // Opcode Flags
		0000, 0440,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCBI",    // Name of the Instruction
		"AND with complement of Both Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0441,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCBM",    // Name of the Instruction
		"AND with complement of Both to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0442,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ANDCBB",    // Name of the Instruction
		"AND with complement of Both to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0443,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"EQV",       // Name of the Instruction
		"Equivalence to AC",
		OP_ALL,       // Opcode Flags
		0000, 0444,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"EQVI",      // Name of the Instruction
		"Equivalence Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0445,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"EQVM",      // Name of the Instruction
		"Equivalence to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0446,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"EQVB",      // Name of the Instruction
		"Equivalence to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0447,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETCA",     // Name of the Instruction
		"Set Complement of AC to AC",
		OP_ALL,       // Opcode Flags
		0000, 0450,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETCAI",    // Name of the Instruction
		"Set Complement of AC Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0451,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETCAM",    // Name of the Instruction
		"Set Complement of AC to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0452,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETCAB",    // Name of the Instruction
		"Set Complement of AC to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0453,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCA",       // Name of the Instruction
		"OR with Complement of AC to AC",
		OP_ALL,       // Opcode Flags
		0000, 0454,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCAI",      // Name of the Instruction
		"OR with Complement of AC Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0455,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCAM",      // Name of the Instruction
		"OR with Complement of AC to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0456,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCAB",      // Name of the Instruction
		"OR with Complement of AC to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0457,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETCM",     // Name of the Instruction
		"Set Complement of Memory to AC",
		OP_ALL,       // Opcode Flags
		0000, 0460,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETCMI",    // Name of the Instruction
		"Set Complement of Memory Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0461,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETCMM",    // Name of the Instruction
		"Set Complement of Memory to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0462,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETCMB",    // Name of the Instruction
		"Set Complement of Memory to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0463,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCM",       // Name of the Instruction
		"OR with Complement of Memory to AC",
		OP_ALL,       // Opcode Flags
		0000, 0464,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCMI",      // Name of the Instruction
		"OR with Complement of Memory Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0465,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCMM",      // Name of the Instruction
		"OR with Complement of Memory to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0466,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCMB",      // Name of the Instruction
		"OR with Complement of Memory to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0467,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCB",       // Name of the Instruction
		"OR with Complement of Both to AC",
		OP_ALL,       // Opcode Flags
		0000, 0470,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCBI",      // Name of the Instruction
		"OR with Complement of Both Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0471,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCBM",      // Name of the Instruction
		"OR with Complement of Both to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0472,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ORCBB",      // Name of the Instruction
		"OR with Complement of Both to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0473,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETO",     // Name of the Instruction
		"Set Ones to AC",
		OP_ALL,       // Opcode Flags
		0000, 0474,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETOI",    // Name of the Instruction
		"Set Ones Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0475,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETOM",    // Name of the Instruction
		"Set Ones to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0476,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETOB",    // Name of the Instruction
		"Set Ones to Both (AC and Memory)",
		OP_ALL,       // Opcode Flags
		0000, 0477,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLL",       // Name of the Instruction
		"Halfword Move, Left to Left, Hold Right, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0500,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLI",       // Name of the Instruction
		"Halfword Move, Left to Left, Hold Right, Immediate",
		OP_KA|OP_KI|OP_IMM, // Opcode Flags
		0000, 0501,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XHLLI",     // Name of the Instruction
		"Extended Halfword Move, Left to Left, Immediate",
		OP_KL|OP_KS|OP_IMM, // Opcode Flags
		0000, 0501,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLM",      // Name of the Instruction
		"Halfword Move, Left to Left, Hold Right, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0502,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLS",      // Name of the Instruction
		"Halfword Move, Left to Left, Hold Right, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0503,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRL",       // Name of the Instruction
		"Halfword Move, Right to Left, Hold Right, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0504,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLI",       // Name of the Instruction
		"Halfword Move, Right to Left, Hold Right, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0505,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLM",      // Name of the Instruction
		"Halfword Move, Right to Left, Hold Right, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0506,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLS",      // Name of the Instruction
		"Halfword Move, Right to Left, Hold Right, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0507,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLZ",       // Name of the Instruction
		"Halfword Move, Left to Left, Zeroes Right, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0510,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLZI",       // Name of the Instruction
		"Halfword Move, Left to Left, Zeroes Right, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0511,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLZM",      // Name of the Instruction
		"Halfword Move, Left to Left, Zeroes Right, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0512,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLZS",      // Name of the Instruction
		"Halfword Move, Left to Left, Zeroes Right, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0513,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLZ",       // Name of the Instruction
		"Halfword Move, Right to Left, Zeroes Right, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0514,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLZI",       // Name of the Instruction
		"Halfword Move, Right to Left, Zeroes Right, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0515,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLZM",      // Name of the Instruction
		"Halfword Move, Right to Left, Zeroes Right, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0516,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLZS",      // Name of the Instruction
		"Halfword Move, Right to Left, Zeroes Right, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0517,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLO",       // Name of the Instruction
		"Halfword Move, Left to Left, Ones Right, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0520,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLOI",       // Name of the Instruction
		"Halfword Move, Left to Left, Ones Right, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0521,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLOM",      // Name of the Instruction
		"Halfword Move, Left to Left, Ones Right, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0522,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLOS",      // Name of the Instruction
		"Halfword Move, Left to Left, Ones Right, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0523,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLO",       // Name of the Instruction
		"Halfword Move, Right to Left, Ones Right, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0524,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLOI",       // Name of the Instruction
		"Halfword Move, Right to Left, Ones Right, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0525,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLOM",      // Name of the Instruction
		"Halfword Move, Right to Left, Ones Right, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0526,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLOS",      // Name of the Instruction
		"Halfword Move, Right to Left, Ones Right, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0527,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLE",       // Name of the Instruction
		"Halfword Move, Left to Left, Extended Right, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0530,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLEI",       // Name of the Instruction
		"Halfword Move, Left to Left, Extended Right, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0531,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLEM",      // Name of the Instruction
		"Halfword Move, Left to Left, Extended Right, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0532,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLLES",      // Name of the Instruction
		"Halfword Move, Left to Left, Extended Right, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0533,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLE",       // Name of the Instruction
		"Halfword Move, Right to Left, Extended Right, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0534,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLEI",       // Name of the Instruction
		"Halfword Move, Right to Left, Extended Right, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0535,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLEM",      // Name of the Instruction
		"Halfword Move, Right to Left, Extended Right, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0536,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRLES",      // Name of the Instruction
		"Halfword Move, Right to Left, Extended Right, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0537,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRR",       // Name of the Instruction
		"Halfword Move, Right to Right, Hold Left, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0540,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRI",       // Name of the Instruction
		"Halfword Move, Right to Right, Hold Left, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0541,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRM",      // Name of the Instruction
		"Halfword Move, Right to Right, Hold Left, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0542,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRS",      // Name of the Instruction
		"Halfword Move, Right to Right, Hold Left, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0543,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLR",       // Name of the Instruction
		"Halfword Move, Left to Right, Hold Left, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0544,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRI",       // Name of the Instruction
		"Halfword Move, Left to Right, Hold Left, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0545,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRM",      // Name of the Instruction
		"Halfword Move, Left to Right, Hold Left, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0546,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRS",      // Name of the Instruction
		"Halfword Move, Left to Right, Hold Left, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0547,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRZ",      // Name of the Instruction
		"Halfword Move, Right to Right, Zeroes Left, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0550,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRZI",     // Name of the Instruction
		"Halfword Move, Right to Right, Zeroes Left, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0551,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRZM",     // Name of the Instruction
		"Halfword Move, Right to Right, Zeroes Left, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0552,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRZS",     // Name of the Instruction
		"Halfword Move, Right to Right, Zeroes Left, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0553,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRZ",      // Name of the Instruction
		"Halfword Move, Left to Right, Zeroes Left, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0554,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRZI",     // Name of the Instruction
		"Halfword Move, Left to Right, Zeroes Left, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0555,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRZM",     // Name of the Instruction
		"Halfword Move, Left to Right, Zeroes Left, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0556,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRZS",     // Name of the Instruction
		"Halfword Move, Left to Right, Zeroes Left, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0557,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRO",      // Name of the Instruction
		"Halfword Move, Right to Right, Ones Left, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0560,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRROI",     // Name of the Instruction
		"Halfword Move, Right to Right, Ones Left, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0561,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRROM",     // Name of the Instruction
		"Halfword Move, Right to Right, Ones Left, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0562,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRROS",     // Name of the Instruction
		"Halfword Move, Right to Right, Ones Left, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0563,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRO",      // Name of the Instruction
		"Halfword Move, Left to Right, Ones Left, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0564,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLROI",     // Name of the Instruction
		"Halfword Move, Left to Right, Ones Left, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0565,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLROM",     // Name of the Instruction
		"Halfword Move, Left to Right, Ones Left, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0566,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLROS",     // Name of the Instruction
		"Halfword Move, Left to Right, Ones Left, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0567,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRE",      // Name of the Instruction
		"Halfword Move, Right to Right, Extended Left, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0570,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRREI",     // Name of the Instruction
		"Halfword Move, Right to Right, Extended Left, Immediate",
		OP_ALL|OP_IMM, // Opcode Flags
		0000, 0571,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRREM",     // Name of the Instruction
		"Halfword Move, Right to Right, Extended Left, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0572,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HRRES",     // Name of the Instruction
		"Halfword Move, Right to Right, Extended Left, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0573,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRE",      // Name of the Instruction
		"Halfword Move, Left to Right, Extended Left, to AC",
		OP_ALL,       // Opcode Flags
		0000, 0574,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLREI",     // Name of the Instruction
		"Halfword Move, Left to Right, Extended Left, Immediate",
		OP_ALL|OP_IMM,// Opcode Flags
		0000, 0575,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLREM",     // Name of the Instruction
		"Halfword Move, Left to Right, Extended Left, to Memory",
		OP_ALL,       // Opcode Flags
		0000, 0576,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"HLRES",     // Name of the Instruction
		"Halfword Move, Left to Right, Extended Left, to Self",
		OP_ALL,       // Opcode Flags
		0000, 0577,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRN",       // Name of the Instruction
		"Test AC Right with E, No modification, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0600,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLN",       // Name of the Instruction
		"Test AC Left with E, No modification, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0601,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRNE",      // Name of the Instruction
		"Test AC Right with E, No modification, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0602,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLNE",      // Name of the Instruction
		"Test AC Left with E, No modification, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0603,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRNA",      // Name of the Instruction
		"Test AC Right with E, No modification, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0604,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLNA",      // Name of the Instruction
		"Test AC Left with E, No modification, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0605,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRNN",      // Name of the Instruction
		"Test AC Right with E, No modification, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0606,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLNN",      // Name of the Instruction
		"Test AC Left with E, No modification, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0607,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDN",       // Name of the Instruction
		"Test AC with direct mask, No modification, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0610,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSN",       // Name of the Instruction
		"Test AC with swapped mask, No modification, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0611,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDNE",      // Name of the Instruction
		"Test AC with direct mask, No modification, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0612,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSNE",      // Name of the Instruction
		"Test AC with swapped mask, No modification, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0613,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDNA",      // Name of the Instruction
		"Test AC with direct mask, No modification, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0614,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSNA",      // Name of the Instruction
		"Test AC with swapped mask, No modification, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0615,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDNN",      // Name of the Instruction
		"Test AC with direct mask, No modification, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0616,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSNN",      // Name of the Instruction
		"Test AC with swapped mask, No modification, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0617,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRZ",       // Name of the Instruction
		"Test AC Right with E, Set masked bits to zeros, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0620,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLZ",       // Name of the Instruction
		"Test AC Left with E, Set masked bits to zeros, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0621,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRZE",      // Name of the Instruction
		"Test AC Right with E, Set masked bits to zeros, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0622,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLZE",      // Name of the Instruction
		"Test AC Left with E, Set masked bits to zeros, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0623,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRZA",      // Name of the Instruction
		"Test AC Right with E, Set masked bits to zeros, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0624,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLZA",      // Name of the Instruction
		"Test AC Left with E, Set masked bits to zeros, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0625,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRZN",      // Name of the Instruction
		"Test AC Right with E, Set masked bits to zeros, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0626,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLZN",      // Name of the Instruction
		"Test AC Left with E, Set masked bits to zeros, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0627,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDZ",       // Name of the Instruction
		"Test AC with direct mask, Set masked bits to zeros, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0630,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSZ",       // Name of the Instruction
		"Test AC with swapped mask, Set masked bits to zeros, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0631,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDZE",      // Name of the Instruction
		"Test AC with direct mask, Set masked bits to zeros, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0632,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSZE",      // Name of the Instruction
		"Test AC with swapped mask, Set masked bits to zeros, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0633,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDZA",      // Name of the Instruction
		"Test AC with direct mask, Set masked bits to zeros, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0634,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSZA",      // Name of the Instruction
		"Test AC with swapped mask, Set masked bits to zeros, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0635,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDZN",      // Name of the Instruction
		"Test AC with direct mask, Set masked bits to zeros, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0636,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSZN",      // Name of the Instruction
		"Test AC with swapped mask, Set masked bits to zeros, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0637,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRC",       // Name of the Instruction
		"Test AC Right with E, Complement masked bits, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0640,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLC",       // Name of the Instruction
		"Test AC Left with E, Complement masked bits, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0641,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRCE",      // Name of the Instruction
		"Test AC Right with E, Complement masked bits, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0642,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLCE",      // Name of the Instruction
		"Test AC Left with E, Complement masked bits, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0643,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRCA",      // Name of the Instruction
		"Test AC Right with E, Complement masked bits, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0644,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLCA",      // Name of the Instruction
		"Test AC Left with E, Complement masked bits, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0645,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRCN",      // Name of the Instruction
		"Test AC Right with E, Complement masked bits, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0646,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLCN",      // Name of the Instruction
		"Test AC Left with E, Complement masked bits, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0647,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDC",       // Name of the Instruction
		"Test AC with direct mask, Complement masked bits, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0650,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSC",       // Name of the Instruction
		"Test AC with swapped mask, Complement masked bits, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0651,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDCE",      // Name of the Instruction
		"Test AC with direct mask, Complement masked bits, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0652,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSCE",      // Name of the Instruction
		"Test AC with swapped mask, Complement masked bits, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0653,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDCA",      // Name of the Instruction
		"Test AC with direct mask, Complement masked bits, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0654,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSCA",      // Name of the Instruction
		"Test AC with swapped mask, Complement masked bits, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0655,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDCN",      // Name of the Instruction
		"Test AC with direct mask, Complement masked bits, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0656,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSCN",      // Name of the Instruction
		"Test AC with swapped mask, Complement masked bits, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0657,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRO",       // Name of the Instruction
		"Test AC Right with E, Set masked bits to ones, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0660,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLO",       // Name of the Instruction
		"Test AC Left with E, Set masked bits to ones, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0661,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TROE",      // Name of the Instruction
		"Test AC Right with E, Set masked bits to ones, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0662,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLOE",      // Name of the Instruction
		"Test AC Left with E, Set masked bits to ones, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0663,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TROA",      // Name of the Instruction
		"Test AC Right with E, Set masked bits to ones, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0664,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLOA",      // Name of the Instruction
		"Test AC Left with E, Set masked bits to ones, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0665,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TRON",      // Name of the Instruction
		"Test AC Right with E, Set masked bits to ones, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0666,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TLON",      // Name of the Instruction
		"Test AC Left with E, Set masked bits to ones, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0667,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDO",       // Name of the Instruction
		"Test AC with direct mask, Set masked bits to ones, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0670,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSO",       // Name of the Instruction
		"Test AC with swapped mask, Set masked bits to ones, and Skip never",
		OP_ALL,       // Opcode Flags
		0000, 0671,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDOE",      // Name of the Instruction
		"Test AC with direct mask, Set masked bits to ones, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0672,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSOE",      // Name of the Instruction
		"Test AC with swapped mask, Set masked bits to ones, and Skip on all masked equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0673,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDOA",      // Name of the Instruction
		"Test AC with direct mask, Set masked bits to ones, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0674,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSOA",      // Name of the Instruction
		"Test AC with swapped mask, Set masked bits to ones, and Skip Always",
		OP_ALL,       // Opcode Flags
		0000, 0675,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TDON",      // Name of the Instruction
		"Test AC with direct mask, Set masked bits to ones, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0676,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TSON",      // Name of the Instruction
		"Test AC with swapped mask, Set masked bits to ones, and Skip on not all masked bits equal zero",
		OP_ALL,       // Opcode Flags
		0000, 0677,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

// Extended Instructions for Opcode 0123 (EXTEND)

	{
		"CMPSL",     // Name of the Instruction
		"Compare Strings and Skip on Less",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0001,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CMPSE",     // Name of the Instruction
		"Compare Strings and Skip on Equal",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0002,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CMPSLE",    // Name of the Instruction
		"Compare Strings and Skip on Less or Equal",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0003,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"EDIT",      // Name of the Instruction
		"Edit String",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0004,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CMPSGE",    // Name of the Instruction
		"Compare Strings and Skip on Greater or Equal",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0005,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CMPSN",     // Name of the Instruction
		"Compare Strings and Skip on not Equal",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0006,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CMPSG",     // Name of the Instruction
		"Compare Strings and Skip on Greater",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0007,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CVTDBO",    // Name of the Instruction
		"Convert Decimal to Binary Offset",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0010,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CVTDBT",    // Name of the Instruction
		"Convert Decimal to Binary Translated",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0011,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CVTBDO",    // Name of the Instruction
		"Convert Binary to Decimal Offset",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0012,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CVTBDT",    // Name of the Instruction
		"Convert Binary to Decimal Translated",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0013,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVSO",     // Name of the Instruction
		"Move String Offset",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0014,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVST",     // Name of the Instruction
		"Move String Translated",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0015,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVSLJ",    // Name of the Instruction
		"Move String Left Justified",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0016,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MOVSRJ",    // Name of the Instruction
		"Move String Right Justified",
		OP_KL|OP_KS|OP_EXT, // Opcode Flags
		0123, 0017,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"XBLT",      // Name of the Instruction
		"Extended Block Transfer",
		OP_KL|OP_EXT, // Opcode Flags
		0123, 0020,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GSNGL",     // Name of the Instruction
		"G-Format to Single Precision",
		OP_KL|OP_EXT|OP_E271, // Opcode Flags
		0123, 0021,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GDBLE",     // Name of the Instruction
		"Single Precision to G-Format Double Precision",
		OP_KL|OP_EXT|OP_E271, // Opcode Flags
		0123, 0022,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GDFIX",     // Name of the Instruction
		"G-Format Double Fix",
		OP_KL|OP_EXT|OP_E271, // Opcode Flags
		0123, 0023,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GFIX",      // Name of the Instruction
		"G-Format Fix",
		OP_KL|OP_EXT|OP_E271, // Opcode Flags
		0123, 0024,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GDFIXR",    // Name of the Instruction
		"G-Format Double Fix and Round",
		OP_KL|OP_EXT|OP_E271, // Opcode Flags
		0123, 0025,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GFIXR",     // Name of the Instruction
		"G-Format Fix and Round",
		OP_KL|OP_EXT|OP_E271, // Opcode Flags
		0123, 0026,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"DGFLTR",    // Name of the Instruction
		"Double G-Format Float and Round",
		OP_KL|OP_EXT|OP_E271, // Opcode Flags
		0123, 0027,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GFSC",      // Name of the Instruction
		"G-Format Floating Scale",
		OP_KL|OP_EXT|OP_E271, // Opcode Flags
		0123, 0031,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

// TOPS-10/20 Instructions

	{
		"CALL",      // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0040,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"INIT",      // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0041,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CALLI",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0047,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"OPEN",      // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0050,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"TTCALL",    // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0051,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"RENAME",    // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0055,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"IN",        // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0056,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"OUT",       // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0057,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"SETSTS",    // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0060,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"STATO",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0061,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"GETSTS",    // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0062,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"STATUS",    // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0062,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"STATZ",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0063,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"INBUF",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0064,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"OUTBUF",    // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0065,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"INPUT",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0066,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"OUTPUT",    // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0067,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"CLOSE",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0070,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"RELEAS",    // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0071,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"MTAPE",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0072,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"UGETF",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0073,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"USETI",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0074,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"USETO",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0075,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"LOOKUP",    // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0076,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"ENTER",     // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0077,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"UJEN",      // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0100,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

	{
		"JSYS",      // Name of the Instruction
		"",
		OP_ALL|OP_T10,// Opcode Flags
		0000, 0104,  // Extended+Opcode (Normally Zero)
		0L,          // Profile Data
		NULL         // Execute Routine
	},

// I/O Instructions - Opcode 7xx Series

	{
		"APRID",       // Name of the Instruction
		"Arithmetic Processor Identification",
		OP_KL|OP_KS|OP_IO,   // Opcode Flags
		0000, 070000,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"CLRPT",       // Name of the Instruction
		"Clear Page Table Entry",
		OP_KL|OP_KS|OP_IO,   // Opcode Flags
		0000, 070110,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RSW",         // Name of the Instruction
		"",
		OP_KA|OP_KI|OP_IO, // Opcode Flags
		0000, 070004,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

// I/O Instructions for KL10 Processor

	{
		"WRFIL",       // Name of the Instruction
		"Write Refill Table",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070010,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDERA",       // Name of the Instruction
		"Read Error Address Register",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070040,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"SBDIAG",      // Name of the Instruction
		"S Bus Diagnostic Function",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070050,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"SWPIA",       // Name of the Instruction
		"Sweep Cache, Invalidate All Pages",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070144,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"SWPVA",       // Name of the Instruction
		"Sweep Cache, Validate All Pages",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070150,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"SWPUA",       // Name of the Instruction
		"Sweep Cache, Unload All Pages",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070154,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"SWPIO",       // Name of the Instruction
		"Sweep Cache, Invalidate One Page",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070164,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"SWPVO",       // Name of the Instruction
		"Sweep Cache, Validate One Page",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070170,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"SWPUO",       // Name of the Instruction
		"Sweep Cache, Unload One Page",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070174,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDPERF",      // Name of the Instruction
		"Read Performance Analysis Count",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070200,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDTIME",      // Name of the Instruction
		"Read Time Base",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070204,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRPAE",       // Name of the Instruction
		"Write Performance Analysis Enables",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070210,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDMACT",      // Name of the Instruction
		"Read Memory Account",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070240,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDEACT",      // Name of the Instruction
		"Read Execution Account",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070244,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRTIME",      // Name of the Instruction
		"Write Time Base",
		OP_KL|OP_IO,   // Opcode Flags
		0000, 070260,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

// I/O Instructions for KS10 Processor

	{
		"UMOVE",       // Name of the Instruction
		"User Move to AC",
		OP_KS,         // Opcode Flags
		0000, 0704,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"UMOVEM",      // Name of the Instruction
		"User Move to Memory",
		OP_KS,         // Opcode Flags
		0000, 0705,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"TIOE",        // Name of the Instruction
		"Test In-Out Equal",
		OP_KS,         // Opcode Flags
		0000, 0710,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"TION",        // Name of the Instruction
		"Test In-Out Not Equal",
		OP_KS,         // Opcode Flags
		0000, 0711,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDIO",        // Name of the Instruction
		"Read In-Out",
		OP_KS,         // Opcode Flags
		0000, 0712,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRIO",        // Name of the Instruction
		"Write In-Out",
		OP_KS,         // Opcode Flags
		0000, 0713,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"BSIO",        // Name of the Instruction
		"Bit Set In-Out",
		OP_KS,         // Opcode Flags
		0000, 0714,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"BCIO",        // Name of the Instruction
		"Bit Clear In-Out",
		OP_KS,         // Opcode Flags
		0000, 0715,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"TIOEB",       // Name of the Instruction
		"Test In-Out Equal Byte",
		OP_KS,         // Opcode Flags
		0000, 0720,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"TIONB",       // Name of the Instruction
		"Test In-Out Not Equal Byte",
		OP_KS,         // Opcode Flags
		0000, 0721,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDIOB",       // Name of the Instruction
		"Read In-Out Byte",
		OP_KS,         // Opcode Flags
		0000, 0722,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRIOB",       // Name of the Instruction
		"Write In-Out Byte",
		OP_KS,         // Opcode Flags
		0000, 0723,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"BSIOB",       // Name of the Instruction
		"Bit Set In-Out Byte",
		OP_KS,         // Opcode Flags
		0000, 0724,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"BCIOB",       // Name of the Instruction
		"Bit Clear In-Out Byte",
		OP_KS,         // Opcode Flags
		0000, 0725,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRAPR",       // Name of the Instruction
		"Write Arithmetic Register",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070020,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDAPR",       // Name of the Instruction
		"Read Arithmetic Register",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070024,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRPI",        // Name of the Instruction
		"Write Priority Interrupt",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070060,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDPI",        // Name of the Instruction
		"Read Priority Interrupt",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070064,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDUBR",       // Name of the Instruction
		"Read User Base Register",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070104,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRUBR",       // Name of the Instruction
		"Write User Base Register",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070114,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WREBR",       // Name of the Instruction
		"Write Executive Base Register",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070120,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDEBR",       // Name of the Instruction
		"Read Executive Base Register",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070124,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDSPB",       // Name of the Instruction
		"Read Shared Page Table",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070200,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDCSB",       // Name of the Instruction
		"Read Core Status Base",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070204,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDPUR",       // Name of the Instruction
		"Read Process-Use Register",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070210,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDCSTM",      // Name of the Instruction
		"Read Core Status Mask",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070214,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDTIM",       // Name of the Instruction
		"Read Timer",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070220,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDINT",       // Name of the Instruction
		"Read Interval Timer",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070224,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"RDHSB",       // Name of the Instruction
		"Read Halt Status Base",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070230,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRSPB",       // Name of the Instruction
		"Write Shared Page Table",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070240,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRCSB",       // Name of the Instruction
		"Write Core Status Base",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070244,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRPUR",       // Name of the Instruction
		"Write Process-Use Register",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070250,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRCSTM",      // Name of the Instruction
		"Write Core Status Mask",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070254,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRTIM",       // Name of the Instruction
		"Write Timer",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070260,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRINT",       // Name of the Instruction
		"Write Interval Timer",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070264,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"WRHSB",       // Name of the Instruction
		"Write Halt Status Base",
		OP_KS|OP_IO,   // Opcode Flags
		0000, 070270,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

// Function codes for I/O Instructions

	{
		"BLKI",        // Name of the Instruction
		"Block In",
		OP_ALL|OP_FUNC,// Opcode Flags
		0000, 070000,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"DATAI",       // Name of the Instruction
		"Data In",
		OP_ALL|OP_FUNC,// Opcode Flags
		0000, 070004,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"BLKO",        // Name of the Instruction
		"Block Out",
		OP_ALL|OP_FUNC,// Opcode Flags
		0000, 070010,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"DATAO",       // Name of the Instruction
		"Data Out",
		OP_ALL|OP_FUNC,// Opcode Flags
		0000, 070014,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"CONO",        // Name of the Instruction
		"Conditions Out",
		OP_ALL|OP_FUNC,// Opcode Flags
		0000, 070020,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"CONI",        // Name of the Instruction
		"Conditions In",
		OP_ALL|OP_FUNC,// Opcode Flags
		0000, 070024,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"CONSZ",       // Name of the Instruction
		"Conditions In and Skip on all masked bits zero",
		OP_ALL|OP_FUNC,// Opcode Flags
		0000, 070030,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"CONSO",       // Name of the Instruction
		"Conditions In and Skip on some masked bit one",
		OP_ALL|OP_FUNC,// Opcode Flags
		0000, 070034,  // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

// Device Codes for I/O Instruction

	{
		"APR",         // Name of the Instruction
		"Arithmetic Processor Register",
		OP_ALL|OP_DEV, // Opcode Flags
		0000, 0000,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"PI",          // Name of the Instruction
		"Priority Interrupt",
		OP_ALL|OP_DEV, // Opcode Flags
		0000, 0004,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"PAG",         // Name of the Instruction
		"Pager",
		OP_KI|OP_KL|OP_KS|OP_DEV, // Opcode Flags
		0000, 0010,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"CCA",         // Name of the Instruction
		"Cache",
		OP_KL|OP_DEV,   // Opcode Flags
		0000, 0014,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"TIM",         // Name of the Instruction
		"Timer",
		OP_KL|OP_DEV,   // Opcode Flags
		0000, 0020,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{
		"MTR",         // Name of the Instruction
		"Meter",
		OP_KL|OP_DEV,   // Opcode Flags
		0000, 0024,    // Extended+Opcode (Normally Zero)
		0L,            // Profile Data
		NULL           // Execute Routine
	},

	{ NULL, NULL, 0, 0, 0, 0, NULL }
};
