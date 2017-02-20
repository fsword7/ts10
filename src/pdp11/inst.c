// inst.c - PDP-11 Instruction Table
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

// List of PDP-11 Instruction
//
// SINGLE-OPERAND INSTRUCTION
// --------------------------
//
// General
//
//   Mnemonic Instruction                Op Code
//   -------- -----------                -------
//   CLR(B)   Clear Destination          B050DD
//   COM(B)   Complement Destination     B051DD
//   INC(B)   Increment Destination      B052DD
//   DEC(B)   Decrement Destination      B053DD
//   NEG(B)   Negate Destination         B054DD
//   TST(B)   Test Destination           B057DD
//
// Shift and Rotate
//
//   Mnemonic Instruction                Op Code
//   -------- -----------                -------
//   ASR(B)   Arithmetic Shift Right     B062DD
//   ASL(B)   Arithmetic Shift Left      B063DD
//   ROR(B)   Rotate Right               B060DD
//   ROL(B)   Rotate Left                B061DD
//   SWAB     Swap Bytes                 0003DD
//
// Multiple-Precision
//
//   Mnemonic Instruction                Op Code
//   -------- -----------                -------
//   ADC(B)   Add Carry                  B055DD
//   SBC(B)   Subtract Carry             B056DD
//   SXT      Sign Extend                0067DD
//
// Processor Status Word Operators
//
//   Mnemonic Instruction                Op Code
//   -------- -----------                -------
//   MFPS     Move Byte from PS          1067DD
//   MTPS     Move Byte to PS            1064SS
//
// DOUBLE-OPERAND INSTRUCTION
// --------------------------
//
// General
//
//   Mnemonic Instruction                    Op Code
//   -------- -----------                    -------
//   MOV(B)   Move Source to Destination     B1SSDD
//   CMP(B)   Compare Source to Destination  B2SSDD
//   ADD      Add Source to Destination      06SSDD
//   SUB      Subtract Source to Destination 16SSDD
//
// Logical
//
//   Mnemonic Instruction                    Op Code
//   -------- -----------                    -------
//   BIT(B)   Bit Test                       B3SSDD
//   BIC(B)   Bit Clear                      B4SSDD
//   BIS(B)   Bit Set                        B5SSDD
//   XOR      Exclusive OR                   074RDD
//
// PROGRAM CONTROL INSTRUCTION
// ---------------------------
//
// Branch
//
//   Mnemonic Instruction                    Op Code
//   -------- -----------                    -------
//   BR       Branch (Unconditional)         000400
//   BNE      Branch if Not Equal            001000
//   BEQ      Branch if Equal                001400
//   BPL      Branch if Plus                 100000
//   BMI      Branch if Minus                100400
//   BVC      Branch if Overflow Clear       102000
//   BVS      Branch if Overflow Set         102400
//   BCC      Branch if Carry Clear          103000
//   BCS      Branch if Carry Set            103400
//
// Signed Conditional Branch
//
//   Mnemonic Instruction                     Op Code
//   -------- -----------                     -------
//   BGE      Branch if Greater Than or Equal 002000
//   BLT      Branch if Less Than             002400
//   BGT      Branch if Greater Than          003000
//   BLE      Branch if Less Than or Equal    003400
//
// Unsigned Conditional Branch
//
//   Mnemonic Instruction                     Op Code
//   -------- -----------                     -------
//   BHI      Branch if Higher                101000
//   BLOS     Branch if Lower or Same         101400
//   BHIS     Branch if Higher or Same        103000
//   BLO      Branch if Lower                 103400
//
// Jump and Subroutine
//
//   Mnemonic Instruction                     Op Code
//   -------- -----------                     -------
//   JMP      Jump                            101000
//   JSR      Jump to Subroutine              004RDD
//   RTS      Return from Subroutine          00020R
//   SOB      Subtract One and Branch         077R00
//
// Trap and Interrupt
//
//   Mnemonic Instruction                     Op Code
//   -------- -----------                     -------
//   EMT      Emulator Trap                   104000-104377
//   TRAP     Trap                            104400-104777
//   BPT      Breakpoint Trap                 000003
//   IOT      Input/Output Trap               000004
//   RTI      Return from Interrupt           000002
//   RTT      Return from Interrupt           000006
//
// MISCELLANEOUS INSTRUCTION
// -------------------------
//
//   Mnemonic Instruction                     Op Code
//   -------- -----------                     -------
//   HALT     Halt                            000000
//   WAIT     Wait                            000001
//   RESET    Reset External Bus              000005
//   MFPT     Move Processor Type             000007
//
// CONDITION CODE OPERATORS
// ------------------------
//
//   Mnemonic Instruction                     Op Code
//   -------- -----------                     -------
//   CLC      Clear C (Carry)                 000241
//   CLV      Clear V (Overflow)              000242
//   CLZ      Clear Z (Zero)                  000244
//   CLN      Clear N (Negate)                000250
//   CCC      Clear All CC Bits               000257
//   SEC      Set C (Carry)                   000261
//   SEV      Set V (Overflow)                000262
//   SEZ      Set Z (Zero)                    000264
//   SEN      Set N (Negate)                  000270
//   SCC      Set All CC Bits                 000277
//   NOP      No Operation                    000240

// INSTRUCTION FORMATS
// -------------------
//
// Single Operand Instruction
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |         Opcode              |     Src/Dst     |
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// Double Operand Instruction
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |  Opcode   |       Src       |       Dst       |
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// Branch Instruction
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |       Opcode          |        Offset         |
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// Jump to Subroutine (JSR) Instruction
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |         004        |   Reg  |       Dst       |
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// Operate Instruction
//
// HALT, MFPT, RESET, RIT, RTT, and WAIT
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                      Opcode                   | 
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// Condition Code Operator Instruction
//
// NOP, CCC, CLC, CLN, CLV, CLZ, SCC, SEC, SEN, SEV, and SEZ
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |              00024             |S |N |Z |V |C | 
// +--^--+--+--^--+--+--^--+--+--^--+--+--^--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//
// S = (Set = 1, Clear = 0)
// N = Negate Bit
// Z = Zero Bit
// V = Overflow Bit
// C = Carry Bit

#include "pdp11/defs.h"

P11_INST pdp11_Inst[] =
{
	{
		"ADC", "Add Carry Word", "0055DD",
		OP_SOP,            // Flags
		0005500, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, ADC),  // Opcode Function Call
	},

	{
		"ADCB", "Add Carry Byte", "1055DD",
		OP_SOP,            // Flags
		0105500, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, ADCB), // Opcode Function Call
	},

	{
		"ADD", "Add Word", "06SSDD",
		OP_DOP,            // Flags
		0060000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, ADD),  // Opcode Function Call
	},

	{
		"ASH", "Arithmetic Shift", "072RSS",
		OP_RSOP|OP_EIS,    // Flags
		0072000, 0000777,  // Opcode, Operand Mask
		INSNAM(p11, ASH),  // Opcode Function Call
	},

	{
		"ASHC", "Arithmetic Shift Combined", "073RSS",
		OP_RSOP|OP_EIS,    // Flags
		0073000, 0000777,  // Opcode, Operand Mask
		INSNAM(p11, ASHC), // Opcode Function Call
	},

	{
		"ASL", "Arithmetic Shift Left Word", "0063DD",
		OP_SOP,            // Flags
		0006300, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, ASL),  // Opcode Function Call
	},

	{
		"ASLB", "Arithmetic Shift Left Byte", "1063DD",
		OP_SOP,            // Flags
		0106300, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, ASLB), // Opcode Function Call
	},

	{
		"ASR", "Arithmetic Shift Right Word", "0062DD",
		OP_SOP,            // Flags
		0006200, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, ASR),  // Opcode Function Call
	},

	{
		"ASRB", "Arithmetic Shift Right Byte", "1062DD",
		OP_SOP,            // Flags
		0106200, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, ASRB), // Opcode Function Call
	},

	{
		"BCC", "Branch if Carry Clear", "103000",
		OP_BR,             // Flags
		0103000, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BCC),  // Opcode Function Call
	},

	{
		"BCS", "Branch if Carry Set", "103400",
		OP_BR,             // Flags
		0103400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BCS),  // Opcode Function Call
	},

	{
		"BEQ", "Branch if Not Equal", "001400",
		OP_BR,             // Flags
		0001400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BEQ),  // Opcode Function Call
	},

	{
		"BGE", "Branch if Greater Than or Equal", "002000",
		OP_BR,             // Flags
		0002000, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BGE),  // Opcode Function Call
	},

	{
		"BGT", "Branch if Greater Than", "003000",
		OP_BR,             // Flags
		0003000, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BGT),  // Opcode Function Call
	},

	{
		"BHI", "Branch if Higher", "101000",
		OP_BR,             // Flags
		0101000, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BHI),  // Opcode Function Call
	},

	{  // Note: BHIS is same as BCC opcode
		"BHIS", "Branch if Higher or Same", "103000",
		OP_BR,             // Flags
		0103000, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BCC),  // Opcode Function Call
	},

	{
		"BIC", "Bit Clear Word", "04SSDD",
		OP_DOP,            // Flags
		0040000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, BIC),  // Opcode Function Call
	},

	{
		"BICB", "Bit Clear Byte", "14SSDD",
		OP_DOP,            // Flags
		0140000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, BICB), // Opcode Function Call
	},

	{
		"BIS", "Bit Set Word", "05SSDD",
		OP_DOP,            // Flags
		0050000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, BIS),  // Opcode Function Call
	},

	{
		"BISB", "Bit Set Byte", "15SSDD",
		OP_DOP,            // Flags
		0150000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, BISB), // Opcode Function Call
	},

	{
		"BIT", "Bit Test Word", "03SSDD",
		OP_DOP,            // Flags
		0030000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, BIT),  // Opcode Function Call
	},

	{
		"BITB", "Bit Test Byte", "13SSDD",
		OP_DOP,            // Flags
		0130000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, BITB), // Opcode Function Call
	},

	{
		"BLE", "Branch if Less Than or Equal", "003400",
		OP_BR,             // Flags
		0003400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BLE),  // Opcode Function Call
	},

	{  // Note: BLO is same as BCS opcode
		"BLO", "Branch if Lower", "103400",
		OP_BR,             // Flags
		0103400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BCS),  // Opcode Function Call
	},

	{
		"BLOS", "Branch if Lower or Same", "101400",
		OP_BR,             // Flags
		0101400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BLOS), // Opcode Function Call
	},

	{
		"BLT", "Branch if Less Than", "002400",
		OP_BR,             // Flags
		0002400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BLT),  // Opcode Function Call
	},

	{
		"BMI", "Branch if Minus", "100400",
		OP_BR,             // Flags
		0100400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BMI),  // Opcode Function Call
	},

	{
		"BNE", "Branch if Not Equal", "001000",
		OP_BR,             // Flags
		0001000, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BNE),  // Opcode Function Call
	},

	{
		"BPL", "Branch if Plus", "100000",
		OP_BR,             // Flags
		0100000, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BPL),  // Opcode Function Call
	},

	{
		"BPT", "Breakpoint Trap", "000003",
		OP_NPN,            // Flags
		0000003, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, BPT),  // Opcode Function Call
	},

	{
		"BR", "Branch", "000400",
		OP_BR,             // Flags
		0000400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BR),   // Opcode Function Call
	},

	{
		"BVC", "Branch if Overflow Clear", "102000",
		OP_BR,             // Flags
		0102000, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BVC),  // Opcode Function Call
	},

	{
		"BVS", "Branch if Overflow Set", "102400",
		OP_BR,             // Flags
		0102400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, BVS),  // Opcode Function Call
	},

	{
		"CALL", "Call Subroutine", "0047DD",
		OP_SOP,            // Flags
		0004700, 0000077,  // Opcode, Operand Mask
		NULL,              // Opcode Function Call
	},

	{
		"Cxx", "Clear Condition Codes", "00024N",
		OP_CCC,            // Flags
		0000240, 0000017,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"CCC", "Clear All CC Bits", "000257",
		OP_NPN,            // Flags
		0000257, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"CLC", "Clear Carry Bit (C)", "000241",
		OP_NPN,            // Flags
		0000241, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"CLN", "Clear Negate Bit (N)", "000250",
		OP_NPN,            // Flags
		0000250, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"CLR", "Clear Word", "0050DD",
		OP_SOP,            // Flags
		0005000, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, CLR),  // Opcode Function Call
	},

	{
		"CLRB", "Clear Byte", "1050DD",
		OP_SOP,            // Flags
		0105000, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, CLRB), // Opcode Function Call
	},

	{
		"CLV", "Clear Overflow Bit (V)", "000242",
		OP_NPN,            // Flags
		0000242, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"CLZ", "Clear Zero Bit (Z)", "000244",
		OP_NPN,            // Flags
		0000244, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"CMP", "Compare Word", "02SSDD",
		OP_DOP,            // Flags
		0020000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, CMP),  // Opcode Function Call
	},

	{
		"CMPB", "Compare Byte", "12SSDD",
		OP_DOP,            // Flags
		0120000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, CMPB), // Opcode Function Call
	},

	{
		"COM", "Complement Word", "0051DD",
		OP_SOP,            // Flags
		0005100, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, COM),  // Opcode Function Call
	},

	{
		"COMB", "Complement Byte", "1051DD",
		OP_SOP,            // Flags
		0105100, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, COMB), // Opcode Function Call
	},

	{
		"DEC", "Decrement Word", "0053DD",
		OP_SOP,            // Flags
		0005300, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, DEC),  // Opcode Function Call
	},

	{
		"DECB", "Decrement Byte", "1053DD",
		OP_SOP,            // Flags
		0105300, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, DECB), // Opcode Function Call
	},

	{
		"DIV", "Divide", "071RSS",
		OP_RSOP|OP_EIS,    // Flags
		0071000, 0000777,  // Opcode, Operand Mask
		INSNAM(p11, DIV),  // Opcode Function Call
	},

	{
		"EMT", "Emulator Trap", "104000-104377",
		OP_8BIT,           // Flags
		0104000, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, EMT),  // Opcode Function Call
	},

	{
		"FADD", "Floating Add", "07500R",
		OP_REG|OP_FIS,     // Flags
		0075000, 0000007,  // Opcode, Operand Mask
		NULL,              // Opcode Function Call
	},

	{
		"FDIV", "Floating Divide", "07503R",
		OP_REG|OP_FIS,     // Flags
		0075030, 0000007,  // Opcode, Operand Mask
		NULL,              // Opcode Function Call
	},

	{
		"FMUL", "Floating Multiply", "07502R",
		OP_REG|OP_FIS,     // Flags
		0075020, 0000007,  // Opcode, Operand Mask
		NULL,              // Opcode Function Call
	},

	{
		"FSUB", "Floating Subtract", "07501R",
		OP_REG|OP_FIS,     // Flags
		0075010, 0000007,  // Opcode, Operand Mask
		NULL,              // Opcode Function Call
	},

	{
		"INC", "Increment Word", "0052DD",
		OP_SOP,            // Flags
		0005200, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, INC),  // Opcode Function Call
	},

	{
		"INCB", "Increment Byte", "1052DD",
		OP_SOP,            // Flags
		0105200, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, INCB), // Opcode Function Call
	},

	{
		"IOT", "Input/Output Trap", "000004",
		OP_NPN,            // Flags
		0000004, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, IOT),  // Opcode Function Call
	},

	{
		"JMP", "Jump", "0001DD",
		OP_SOP,            // Flags
		0000100, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, JMP),  // Opcode Function Call
	},

	{
		"JSR", "Jump to Subroutine", "004RDD",
		OP_RSOP,           // Flags
		0004000, 0000777,  // Opcode, Operand Mask
		INSNAM(p11, JSR),  // Opcode Function Call
	},

	{
		"HALT", "Halt Processor", "000000",
		OP_NPN,            // Flags
		0000000, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, HALT), // Opcode Function Call
	},

	{
		"MARK", "Mark Stack", "0064NN",
		OP_6BIT,           // Flags
		0006400, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, MARK), // Opcode Function Call
	},

	{
		"MFPD", "Move From Previous Data", "1065SS",
		OP_SOP,            // Flags
		0106500, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, MFPD), // Opcode Function Call
	},

	{
		"MFPI", "Move From Previous Instruction", "0065SS",
		OP_SOP,            // Flags
		0006500, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, MFPI), // Opcode Function Call
	},

	{
		"MFPS", "Move From Processor Status", "1067DD",
		OP_SOP,            // Flags
		0106700, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, MFPS), // Opcode Function Call
	},

	{
		"MFPT", "Move From Processor Type", "000007",
		OP_NPN,            // Flags
		0000007, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, MFPT), // Opcode Function Call
	},

	{
		"MOV", "Move Word", "01SSDD",
		OP_DOP,            // Flags
		0010000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, MOV),  // Opcode Function Call
	},

	{
		"MOVB", "Move Byte", "11SSDD",
		OP_DOP,            // Flags
		0110000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, MOVB), // Opcode Function Call
	},

	{
		"MTPD", "Move To Previous Data", "1066DD",
		OP_SOP,            // Flags
		0106600, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, MTPD), // Opcode Function Call
	},

	{
		"MTPI", "Move To Previous Instruction", "0066DD",
		OP_SOP,            // Flags
		0006600, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, MTPI), // Opcode Function Call
	},

	{
		"MTPS", "Move To Processor Status", "1064SS",
		OP_SOP,            // Flags
		0106400, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, MTPS), // Opcode Function Call
	},

	{
		"MUL", "Multiply", "070RSS",
		OP_RSOP|OP_EIS,    // Flags
		0070000, 0000777,  // Opcode, Operand Mask
		INSNAM(p11, MUL),  // Opcode Function Call
	},

	{
		"NEG", "Negate Word", "0054DD",
		OP_SOP,            // Flags
		0005400, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, NEG),  // Opcode Function Call
	},

	{
		"NEGB", "Negate Byte", "1054DD",
		OP_SOP,            // Flags
		0105400, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, NEGB), // Opcode Function Call
	},

	{
		"NOP", "No Operation", "000240",
		OP_NPN,            // Flags
		0000240, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, NOP),  // Opcode Function Call
	},

	{
		"RESET", "Reset External Bus", "000005",
		OP_NPN,             // Flags
		0000005, 0000000,   // Opcode, Operand Mask
		INSNAM(p11, RESET), // Opcode Function Call
	},

	{
		"RETURN", "Return from Subroutine", "000207",
		OP_NPN,             // Flags
		0000207, 0000000,   // Opcode, Operand Mask
		NULL,               // Opcode Function Call
	},

	{
		"ROL", "Rotate Left Word", "0061DD",
		OP_SOP,             // Flags
		0006100, 0000077,   // Opcode, Operand Mask
		INSNAM(p11, ROL),   // Opcode Function Call
	},

	{
		"ROLB", "Rotate Left Byte", "1061DD",
		OP_SOP,             // Flags
		0106100, 0000077,   // Opcode, Operand Mask
		INSNAM(p11, ROLB),  // Opcode Function Call
	},

	{
		"ROR", "Rotate Right Word", "0060DD",
		OP_SOP,             // Flags
		0006000, 0000077,   // Opcode, Operand Mask
		INSNAM(p11, ROR),   // Opcode Function Call
	},

	{
		"RORB", "Rotate Right Byte", "1060DD",
		OP_SOP,             // Flags
		0106000, 0000077,   // Opcode, Operand Mask
		INSNAM(p11, RORB),  // Opcode Function Call
	},

	{
		"RTI", "Return from Interrupt", "000002",
		OP_NPN,             // Flags
		0000002, 0000000,   // Opcode, Operand Mask
		INSNAM(p11, RTI),   // Opcode Function Call
	},

	{
		"RTS", "Return from Subroutine", "00020R",
		OP_REG,            // Flags
		0000200, 0000007,  // Opcode, Operand Mask
		INSNAM(p11, RTS),  // Opcode Function Call
	},

	{
		"RTT", "Return from Trap", "000006",
		OP_NPN,            // Flags
		0000006, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, RTT),  // Opcode Function Call
	},

	{
		"SBC", "Subtract Carry Word", "0056DD",
		OP_SOP,            // Flags
		0005600, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, SBC),  // Opcode Function Call
	},

	{
		"SBCB", "Subtract Carry Byte", "1056DD",
		OP_SOP,            // Flags
		0105600, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, SBCB), // Opcode Function Call
	},

	{
		"Sxx", "Set Condition Codes", "00026N",
		OP_SCC,            // Flags
		0000260, 0000017,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"SCC", "Set All CC Bits", "000277",
		OP_NPN,            // Flags
		0000277, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"SEC", "Set Carry Bit (C)", "000261",
		OP_NPN,            // Flags
		0000261, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"SEN", "Set Negate Bit (N)", "000270",
		OP_NPN,            // Flags
		0000270, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"SEV", "Set Overflow Bit (V)", "000262",
		OP_NPN,            // Flags
		0000262, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"SEZ", "Set Zero Bit (Z)", "000264",
		OP_NPN,            // Flags
		0000264, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, XCC),  // Opcode Function Call
	},

	{
		"SOB", "Subtract One and Branch", "077RNN",
		OP_SOB,            // Flags
		0077000, 0000777,  // Opcode, Operand Mask
		INSNAM(p11, SOB),  // Opcode Function Call
	},

	{
		"SPL", "Set Priority Level", "00023N",
		OP_3BIT,           // Flags
		0000230, 0000007,  // Opcode, Operand Mask
		INSNAM(p11, SPL),  // Opcode Function Call
	},

	{
		"SUB", "Subtract Word", "16SSDD",
		OP_DOP,            // Flags
		0160000, 0007777,  // Opcode, Operand Mask
		INSNAM(p11, SUB),  // Opcode Function Call
	},

	{
		"SWAB", "Swap Bytes", "0003DD",
		OP_SOP,            // Flags
		0000300, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, SWAB), // Opcode Function Call
	},

	{
		"SXT", "Sign Extend", "0067DD",
		OP_SOP,            // Flags
		0006700, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, SXT),  // Opcode Function Call
	},

	{
		"TRAP", "Trap", "104400-104777",
		OP_8BIT,           // Flags
		0104400, 0000377,  // Opcode, Operand Mask
		INSNAM(p11, TRAP), // Opcode Function Call
	},

	{
		"TST", "Test Word", "0057DD",
		OP_SOP,            // Flags
		0005700, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, TST),  // Opcode Function Call
	},

	{
		"TSTB", "Test Byte", "1057DD",
		OP_SOP,            // Flags
		0105700, 0000077,  // Opcode, Operand Mask
		INSNAM(p11, TSTB), // Opcode Function Call
	},

	{
		"TSTSET", "Test and Set", "0072DD",
		OP_SOP,              // Flags
		0007200, 0000077,    // Opcode, Operand Mask
		INSNAM(p11, TSTSET), // Opcode Function Call
	},

	{
		"WAIT", "Wait", "000001",
		OP_NPN,            // Flags
		0000001, 0000000,  // Opcode, Operand Mask
		INSNAM(p11, WAIT), // Opcode Function Call
	},

	{
		"WRTLCK", "Write and Lock", "0073DD",
		OP_SOP,              // Flags
		0007300, 0000077,    // Opcode, Operand Mask
		INSNAM(p11, WRTLCK), // Opcode Function Call
	},

	{
		"XOR", "Exclusive OR", "074RDD",
		OP_RSOP,           // Flags
		0074000, 0000777,  // Opcode, Operand Mask
		INSNAM(p11, XOR),  // Opcode Function Call
	},

	{ NULL } // Null Terminator
};


// New PDP-11 Instruction Table
P11_INST2 p11_Inst[] =
{
	{ "HALT",   OP_NPN,     0000000, 0000000, INSNAM(p11, HALT)   },
	{ "WAIT",   OP_NPN,     0000001, 0000000, INSNAM(p11, WAIT)   },
	{ "RTI",    OP_NPN,     0000002, 0000000, INSNAM(p11, RTI)    },
	{ "BPT",    OP_NPN,     0000003, 0000000, INSNAM(p11, BPT)    },
	{ "IOT",    OP_NPN,     0000004, 0000000, INSNAM(p11, IOT)    },
	{ "RESET",  OP_NPN,     0000005, 0000000, INSNAM(p11, RESET)  },
	{ "RTT",    OP_NPN,     0000006, 0000000, INSNAM(p11, RTT)    },
	{ "MFPT",   OP_NPN,     0000007, 0000000, INSNAM(p11, MFPT)   },
	{ "JMP",    OP_SOP,     0000100, 0000077, INSNAM(p11, JMP)    },
	{ "RTS",    OP_REG,     0000200, 0000007, INSNAM(p11, RTS)    },
	{ "SPL",    OP_3BIT,    0000230, 0000007, INSNAM(p11, SPL)    },
	{ "Cxx",    OP_CCC,     0000240, 0000017, INSNAM(p11, XCC)    },
	{ "NOP",    OP_CCC,     0000240, 0000000, INSNAM(p11, NOP)    },
	{ "CLC",    OP_CCC,     0000241, 0000000, INSNAM(p11, XCC)    },
	{ "CLV",    OP_CCC,     0000242, 0000000, INSNAM(p11, XCC)    },
	{ "CLZ",    OP_CCC,     0000244, 0000000, INSNAM(p11, XCC)    },
	{ "CLN",    OP_CCC,     0000250, 0000000, INSNAM(p11, XCC)    },
	{ "CCC",    OP_CCC,     0000257, 0000000, INSNAM(p11, XCC)    },
	{ "Sxx",    OP_SCC,     0000260, 0000017, INSNAM(p11, XCC)    },
	{ "NOP",    OP_SCC,     0000260, 0000000, INSNAM(p11, NOP)    },
	{ "SEC",    OP_SCC,     0000261, 0000000, INSNAM(p11, XCC)    },
	{ "SEV",    OP_SCC,     0000262, 0000000, INSNAM(p11, XCC)    },
	{ "SEZ",    OP_SCC,     0000264, 0000000, INSNAM(p11, XCC)    },
	{ "SEN",    OP_SCC,     0000270, 0000000, INSNAM(p11, XCC)    },
	{ "SCC",    OP_SCC,     0000277, 0000000, INSNAM(p11, XCC)    },
	{ "SWAB",   OP_SOP,     0000300, 0000077, INSNAM(p11, SWAB)   },
	{ "BR",     OP_BR,      0000400, 0000377, INSNAM(p11, BR)     },
	{ "BNE",    OP_BR,      0001000, 0000377, INSNAM(p11, BNE)    },
	{ "BEQ",    OP_BR,      0001400, 0000377, INSNAM(p11, BEQ)    },
	{ "BGE",    OP_BR,      0002000, 0000377, INSNAM(p11, BGE)    },
	{ "BLT",    OP_BR,      0002400, 0000377, INSNAM(p11, BLT)    },
	{ "BGT",    OP_BR,      0003000, 0000377, INSNAM(p11, BGT)    },
	{ "BLE",    OP_BR,      0003400, 0000377, INSNAM(p11, BLE)    },
	{ "JSR",    OP_RSOP,    0004000, 0000777, INSNAM(p11, JSR)    },
	{ "CLR",    OP_SOP,     0005000, 0000077, INSNAM(p11, CLR)    },
	{ "COM",    OP_SOP,     0005100, 0000077, INSNAM(p11, COM)    },
	{ "INC",    OP_SOP,     0005200, 0000077, INSNAM(p11, INC)    },
	{ "DEC",    OP_SOP,     0005300, 0000077, INSNAM(p11, DEC)    },
	{ "NEG",    OP_SOP,     0005400, 0000077, INSNAM(p11, NEG)    },
	{ "ADC",    OP_SOP,     0005500, 0000077, INSNAM(p11, ADC)    },
	{ "SBC",    OP_SOP,     0005600, 0000077, INSNAM(p11, SBC)    },
	{ "TST",    OP_SOP,     0005700, 0000077, INSNAM(p11, TST)    },
	{ "ROR",    OP_SOP,     0006000, 0000077, INSNAM(p11, ROR)    },
	{ "ROL",    OP_SOP,     0006100, 0000077, INSNAM(p11, ROL)    },
	{ "ASR",    OP_SOP,     0006200, 0000077, INSNAM(p11, ASR)    },
	{ "ASL",    OP_SOP,     0006300, 0000077, INSNAM(p11, ASL)    },
	{ "MARK",   OP_SOP,     0006400, 0000077, INSNAM(p11, MARK)   },
	{ "MFPI",   OP_SOP,     0006500, 0000077, INSNAM(p11, MFPI)   },
	{ "MTPI",   OP_SOP,     0006600, 0000077, INSNAM(p11, MTPI)   },
	{ "SXT",    OP_SOP,     0006700, 0000077, INSNAM(p11, SXT)    },
	{ "CSM",    OP_SOP,     0007100, 0000077, NULL                },
	{ "TSTSET", OP_SOP,     0007200, 0000077, INSNAM(p11, TSTSET) },
	{ "WRTLCK", OP_SOP,     0007300, 0000077, INSNAM(p11, WRTLCK) },
	{ "MOV",    OP_DOP,     0010000, 0007777, INSNAM(p11, MOV)    },
	{ "CMP",    OP_DOP,     0020000, 0007777, INSNAM(p11, CMP)    },
	{ "BIT",    OP_DOP,     0030000, 0007777, INSNAM(p11, BIT)    },
	{ "BIC",    OP_DOP,     0040000, 0007777, INSNAM(p11, BIC)    },
	{ "BIS",    OP_DOP,     0050000, 0007777, INSNAM(p11, BIS)    },
	{ "ADD",    OP_DOP,     0060000, 0007777, INSNAM(p11, ADD)    },
	{ "MUL",    OP_RSOP,    0070000, 0000777, INSNAM(p11, MUL)    },
	{ "DIV",    OP_RSOP,    0071000, 0000777, INSNAM(p11, DIV)    },
	{ "ASH",    OP_RSOP,    0072000, 0000777, INSNAM(p11, ASH)    },
	{ "ASHC",   OP_RSOP,    0073000, 0000777, INSNAM(p11, ASHC)   },
	{ "XOR",    OP_RSOP,    0074000, 0000777, INSNAM(p11, XOR)    },
	{ "FADD",   OP_REG,     0075000, 0000007, NULL                },
	{ "FSUB",   OP_REG,     0075010, 0000007, NULL                },
	{ "FMUL",   OP_REG,     0075020, 0000007, NULL                },
	{ "FDIV",   OP_REG,     0075030, 0000007, NULL                },
	{ "L2DR",   OP_REG,     0076020, 0000007, NULL                },
	{ "MOVC",   OP_NPN,     0076030, 0000000, NULL                },
	{ "MOVRC",  OP_NPN,     0076031, 0000000, NULL                },
	{ "MOVTC",  OP_NPN,     0076032, 0000000, NULL                },
	{ "LOCC",   OP_NPN,     0076040, 0000000, NULL                },
	{ "SKPC",   OP_NPN,     0076041, 0000000, NULL                },
	{ "SCANC",  OP_NPN,     0076042, 0000000, NULL                },
	{ "SPANC",  OP_NPN,     0076043, 0000000, NULL                },
	{ "CMPC",   OP_NPN,     0076044, 0000000, NULL                },
	{ "MATC",   OP_NPN,     0076045, 0000000, NULL                },
	{ "ADDN",   OP_NPN,     0076050, 0000000, NULL                },
	{ "SUBN",   OP_NPN,     0076051, 0000000, NULL                },
	{ "CMPN",   OP_NPN,     0076052, 0000000, NULL                },
	{ "CVTNL",  OP_NPN,     0076053, 0000000, NULL                },
	{ "CVTPN",  OP_NPN,     0076054, 0000000, NULL                },
	{ "CVTNP",  OP_NPN,     0076055, 0000000, NULL                },
	{ "ASHN",   OP_NPN,     0076056, 0000000, NULL                },
	{ "CVTLN",  OP_NPN,     0076057, 0000000, NULL                },
	{ "L3DR",   OP_REG,     0076060, 0000007, NULL                },
	{ "ADDP",   OP_NPN,     0076070, 0000000, NULL                },
	{ "SUBP",   OP_NPN,     0076071, 0000000, NULL                },
	{ "CMPP",   OP_NPN,     0076072, 0000000, NULL                },
	{ "CVTPL",  OP_NPN,     0076073, 0000000, NULL                },
	{ "MULP",   OP_NPN,     0076074, 0000000, NULL                },
	{ "DIVP",   OP_NPN,     0076075, 0000000, NULL                },
	{ "ASHP",   OP_NPN,     0076076, 0000000, NULL                },
	{ "CVTLP",  OP_NPN,     0076077, 0000000, NULL                },
	{ "MOVCI",  OP_NPN,     0076130, 0000000, NULL                },
	{ "MOVRCI", OP_NPN,     0076131, 0000000, NULL                },
	{ "MOVTCI", OP_NPN,     0076132, 0000000, NULL                },
	{ "LOCCI",  OP_NPN,     0076140, 0000000, NULL                },
	{ "SKPCI",  OP_NPN,     0076141, 0000000, NULL                },
	{ "SCANCI", OP_NPN,     0076142, 0000000, NULL                },
	{ "SPANCI", OP_NPN,     0076143, 0000000, NULL                },
	{ "CMPCI",  OP_NPN,     0076144, 0000000, NULL                },
	{ "MATCI",  OP_NPN,     0076145, 0000000, NULL                },
	{ "ADDNI",  OP_NPN,     0076150, 0000000, NULL                },
	{ "SUBNI",  OP_NPN,     0076151, 0000000, NULL                },
	{ "CMPNI",  OP_NPN,     0076152, 0000000, NULL                },
	{ "CVTNLI", OP_NPN,     0076153, 0000000, NULL                },
	{ "CVTPNI", OP_NPN,     0076154, 0000000, NULL                },
	{ "CVTNPI", OP_NPN,     0076155, 0000000, NULL                },
	{ "ASHNI",  OP_NPN,     0076156, 0000000, NULL                },
	{ "CVTLNI", OP_NPN,     0076157, 0000000, NULL                },
	{ "ADDPI",  OP_NPN,     0076170, 0000000, NULL                },
	{ "SUBPI",  OP_NPN,     0076171, 0000000, NULL                },
	{ "CMPPI",  OP_NPN,     0076172, 0000000, NULL                },
	{ "CVTPLI", OP_NPN,     0076173, 0000000, NULL                },
	{ "MULPI",  OP_NPN,     0076174, 0000000, NULL                },
	{ "DIVPI",  OP_NPN,     0076175, 0000000, NULL                },
	{ "ASHPI",  OP_NPN,     0076176, 0000000, NULL                },
	{ "CVTLPI", OP_NPN,     0076177, 0000000, NULL                },
	{ "SOB",    OP_SOP,     0077000, 0000777, INSNAM(p11, SOB)    },
	{ "BMI",    OP_BR,      0100400, 0000377, INSNAM(p11, BMI)    },
	{ "BHI",    OP_BR,      0101000, 0000377, INSNAM(p11, BHI)    },
	{ "BLOS",   OP_BR,      0101400, 0000377, INSNAM(p11, BLOS)   },
	{ "BVC",    OP_BR,      0102000, 0000377, INSNAM(p11, BVC)    },
	{ "BVS",    OP_BR,      0102400, 0000377, INSNAM(p11, BVS)    },
	{ "BCC",    OP_BR,      0103000, 0000377, INSNAM(p11, BCC)    },
	{ "BCS",    OP_BR,      0103400, 0000377, INSNAM(p11, BCS)    },
	{ "BHIS",   OP_BR,      0103000, 0000377, INSNAM(p11, BCC)    },
	{ "BLO",    OP_BR,      0103400, 0000377, INSNAM(p11, BCS)    },
	{ "EMT",    OP_8BIT,    0104000, 0000377, INSNAM(p11, EMT)    },
	{ "TRAP",   OP_8BIT,    0104400, 0000377, INSNAM(p11, TRAP)   },
	{ "CLRB",   OP_SOP,     0105000, 0000077, INSNAM(p11, CLRB)   },
	{ "COMB",   OP_SOP,     0105100, 0000077, INSNAM(p11, COMB)   },
	{ "INCB",   OP_SOP,     0105200, 0000077, INSNAM(p11, INCB)   },
	{ "DECB",   OP_SOP,     0105300, 0000077, INSNAM(p11, DECB)   },
	{ "NEGB",   OP_SOP,     0105400, 0000077, INSNAM(p11, NEGB)   },
	{ "ADCB",   OP_SOP,     0105500, 0000077, INSNAM(p11, ADCB)   },
	{ "SBCB",   OP_SOP,     0105600, 0000077, INSNAM(p11, SBCB)   },
	{ "TSTB",   OP_SOP,     0105700, 0000077, INSNAM(p11, TSTB)   },
	{ "RORB",   OP_SOP,     0106000, 0000077, INSNAM(p11, RORB)   },
	{ "ROLB",   OP_SOP,     0106100, 0000077, INSNAM(p11, ROLB)   },
	{ "ASRB",   OP_SOP,     0106200, 0000077, INSNAM(p11, ASRB)   },
	{ "ASLB",   OP_SOP,     0106300, 0000077, INSNAM(p11, ASLB)   },
	{ "MTPS",   OP_SOP,     0106400, 0000077, INSNAM(p11, MTPS)   },
	{ "MFPD",   OP_SOP,     0106500, 0000077, INSNAM(p11, MFPD)   },
	{ "MTPD",   OP_SOP,     0106600, 0000077, INSNAM(p11, MTPD)   },
	{ "MFPS",   OP_SOP,     0106700, 0000077, INSNAM(p11, MFPS)   },
	{ "MOVB",   OP_DOP,     0110000, 0007777, INSNAM(p11, MOVB)   },
	{ "CMPB",   OP_DOP,     0120000, 0007777, INSNAM(p11, CMPB)   },
	{ "BITB",   OP_DOP,     0130000, 0007777, INSNAM(p11, BITB)   },
	{ "BICB",   OP_DOP,     0140000, 0007777, INSNAM(p11, BICB)   },
	{ "BISB",   OP_DOP,     0150000, 0007777, INSNAM(p11, BISB)   },
	{ "SUB",    OP_DOP,     0160000, 0007777, INSNAM(p11, SUB)    },
	{ "CFCC",   OP_NPN,     0170000, 0000000, NULL                },
	{ "SETF",   OP_NPN,     0170001, 0000000, NULL                },
	{ "SETI",   OP_NPN,     0170002, 0000000, NULL                },
	{ "SETD",   OP_NPN,     0170011, 0000000, NULL                },
	{ "SETL",   OP_NPN,     0170012, 0000000, NULL                },
	{ "LDFPS",  OP_SOP,     0170100, 0000077, NULL                },
	{ "STFPS",  OP_SOP,     0170200, 0000077, NULL                },
	{ "STST",   OP_SOP,     0170300, 0000077, NULL                },
	{ "CLRF",   OP_FOP_F,   0170400, 0000000, NULL                },
	{ "CLRD",   OP_FOP_D,   0170400, 0000000, NULL                },
	{ "TSTF",   OP_FOP_F,   0170500, 0000000, NULL                },
	{ "TSTD",   OP_FOP_D,   0170500, 0000000, NULL                },
	{ "ABSF",   OP_FOP_F,   0170600, 0000000, NULL                },
	{ "ABSD",   OP_FOP_D,   0170600, 0000000, NULL                },
	{ "NEGF",   OP_FOP_F,   0170700, 0000000, NULL                },
	{ "NEGD",   OP_FOP_D,   0170700, 0000000, NULL                },
	{ "MULF",   OP_AFOP_F,  0171000, 0000000, NULL                },
	{ "MULD",   OP_AFOP_D,  0171000, 0000000, NULL                },
	{ "MODF",   OP_AFOP_F,  0171400, 0000000, NULL                },
	{ "MODD",   OP_AFOP_D,  0171400, 0000000, NULL                },
	{ "ADDF",   OP_AFOP_F,  0172000, 0000000, NULL                },
	{ "ADDD",   OP_AFOP_D,  0172000, 0000000, NULL                },
	{ "LDF",    OP_AFOP_F,  0172400, 0000000, NULL                },
	{ "LDD",    OP_AFOP_D,  0172400, 0000000, NULL                },
	{ "SUBF",   OP_AFOP_F,  0173000, 0000000, NULL                },
	{ "SUBD",   OP_AFOP_D,  0173000, 0000000, NULL                },
	{ "CMPF",   OP_AFOP_F,  0173400, 0000000, NULL                },
	{ "CMPD",   OP_AFOP_D,  0173400, 0000000, NULL                },
	{ "STF",    OP_AFOP_F,  0174000, 0000000, NULL                },
	{ "STD",    OP_AFOP_D,  0174000, 0000000, NULL                },
	{ "DIVF",   OP_AFOP_F,  0174400, 0000000, NULL                },
	{ "DIVD",   OP_AFOP_D,  0174400, 0000000, NULL                },
	{ "STEXP",  OP_ASOP,    0175000, 0000000, NULL                },
	{ "STCFI",  OP_ASMD_F,  0175400, 0000000, NULL                },
	{ "STCDI",  OP_ASMD_D,  0175400, 0000000, NULL                },
	{ "STCFL",  OP_ASMD_FL, 0175400, 0000000, NULL                },
	{ "STCDL",  OP_ASMD_DL, 0175400, 0000000, NULL                },
	{ "STCFD",  OP_AFOP_F,  0176000, 0000000, NULL                },
	{ "STCDF",  OP_AFOP_D,  0176000, 0000000, NULL                },
	{ "LDEXP",  OP_ASOP,    0176400, 0000000, NULL                },
	{ "LDCIF",  OP_ASMD_F,  0177000, 0000000, NULL                },
	{ "LDCID",  OP_ASMD_D,  0177000, 0000000, NULL                },
	{ "LDCLF",  OP_ASMD_FL, 0177000, 0000000, NULL                },
	{ "LDCLD",  OP_ASMD_DL, 0177000, 0000000, NULL                },
	{ "LDCFD",  OP_AFOP_F,  0177400, 0000000, NULL                },
	{ "LDCDF",  OP_AFOP_D,  0177400, 0000000, NULL                },

	{ NULL } // Null Terminator
};
