// proto.h - Protype Definition
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

INSDEF(p11, ADC);     // Add Carry Word
INSDEF(p11, ADCB);    // Add Carry Byte
INSDEF(p11, ADD);     // Add Word
INSDEF(p11, ASH);     // Arithmetic Shift (EIS)
INSDEF(p11, ASHC);    // Arithmetic Shift Combined (EIS)
INSDEF(p11, ASL);     // Arithmetic Shift Left Word
INSDEF(p11, ASLB);    // Arithmetic Shift Left Byte
INSDEF(p11, ASR);     // Arithmetic Shift Right Word
INSDEF(p11, ASRB);    // Arithmetic Shift Right Byte
INSDEF(p11, BCC);     // Branch if Carry Clear
INSDEF(p11, BCS);     // Branch if Carry Set
INSDEF(p11, BEQ);     // Branch if Equal
INSDEF(p11, BGE);     // Branch if Greater or Equal
INSDEF(p11, BGT);     // Branch if Greater Than
INSDEF(p11, BHI);     // Branch if Higher
//INSDEF(p11, BHIS);  // Branch if Higher or Same
INSDEF(p11, BIC);     // Bit Clear Word
INSDEF(p11, BICB);    // Bit Clear Byte
INSDEF(p11, BIS);     // Bit Set Word
INSDEF(p11, BISB);    // Bit Set Byte
INSDEF(p11, BIT);     // Bit Test Word
INSDEF(p11, BITB);    // Bit Test Byte
INSDEF(p11, BLE);     // Branch if Less or Equal
INSDEF(p11, BLT);     // Branch if Less Than
//INSDEF(p11, BLO);   // Branch if Lower
INSDEF(p11, BLOS);    // Branch if Lower or Same
INSDEF(p11, BMI);     // Branch if Minus
INSDEF(p11, BNE);     // Branch if Not Equal
INSDEF(p11, BPL);     // Branch if Plus
INSDEF(p11, BPT);     // Breakpoint Trap
INSDEF(p11, BR);      // Branch
INSDEF(p11, BVC);     // Branch if Overflow Clear
INSDEF(p11, BVS);     // Branch if Overflow Set
INSDEF(p11, XCC);     // Set/Clear Condition Bits
INSDEF(p11, CLR);     // Clear Word
INSDEF(p11, CLRB);    // Clear Byte
INSDEF(p11, CMP);     // Compare Word
INSDEF(p11, CMPB);    // Compare Byte
INSDEF(p11, COM);     // Complent Word
INSDEF(p11, COMB);    // Complent Byte
INSDEF(p11, DEC);     // Decrement Word
INSDEF(p11, DECB);    // Decrement Byte
INSDEF(p11, DIV);     // Divide (EIS)
INSDEF(p11, EMT);     // Emulator Trap
INSDEF(p11, HALT);    // Halt Processor
INSDEF(p11, ILL);     // Illegal Instruction
INSDEF(p11, INC);     // Increment Word
INSDEF(p11, INCB);    // Increment Byte
INSDEF(p11, IOT);     // Input/Output Trap
INSDEF(p11, JMP);     // Jump
INSDEF(p11, JSR);     // Jump to Subroutine
INSDEF(p11, MARK);    // Mark Stack
INSDEF(p11, MFPD);    // Move From Previous Data
INSDEF(p11, MFPI);    // Move From Previous Instruction
INSDEF(p11, MFPT);    // Move From Processor Type
INSDEF(p11, MFPS);    // Move From Processor Status
INSDEF(p11, MOV);     // Move Word
INSDEF(p11, MOVB);    // Move Byte
INSDEF(p11, MTPD);    // Move To Previous Data
INSDEF(p11, MTPI);    // Move To Previous Instruction
INSDEF(p11, MTPS);    // Move To Processor Status
INSDEF(p11, MUL);     // Multiply (EIS)
INSDEF(p11, NEG);     // Negate Word
INSDEF(p11, NEGB);    // Negate Byte
INSDEF(p11, NOP);     // No Operation
INSDEF(p11, RESET);   // Reset External Bus
INSDEF(p11, ROL);     // Rotate Left Word
INSDEF(p11, ROLB);    // Rotate Left Byte
INSDEF(p11, ROR);     // Rotate Right Word
INSDEF(p11, RORB);    // Rotate Right Byte
INSDEF(p11, RTI);     // Return from Interrupt
INSDEF(p11, RTS);     // Return from Subroutine
INSDEF(p11, RTT);     // Return from Trap
INSDEF(p11, SBC);     // Subtract Carry Word
INSDEF(p11, SBCB);    // Subtract Carry Byte
INSDEF(p11, SOB);     // Subtract One and Branch
INSDEF(p11, SPL);     // Set Priority Level
INSDEF(p11, SUB);     // Subtract Word
INSDEF(p11, SWAB);    // Swap Bytes
INSDEF(p11, SXT);     // Sign Extend
INSDEF(p11, TRAP);    // Trap
INSDEF(p11, TST);     // Test Word
INSDEF(p11, TSTB);    // Test Byte
INSDEF(p11, TSTSET);  // Test/Set
INSDEF(p11, WAIT);    // Wait
INSDEF(p11, WRTLCK);  // Write Lock
INSDEF(p11, XOR);     // Exclusive OR

#ifdef DEBUG
// disasm.c
//void p11_Disasm(register P11_CPU *, SOCKET *, uint32 *, uint32);
void p11_InitDisasm(register P11_SYSTEM *);
void p11_CleanupDisasm(register P11_SYSTEM *);
//void p11_Dump(P11_CPU *, SOCKET *, uint32 *, uint32, uint32);
#endif /* DEBUG */

// cpu_main.c
char   *p11_GetCC(uint16, char *);
uint32  p11_GeteaW(register P11_CPU *, int32);
uint32  p11_GeteaB(register P11_CPU *, int32);
void    p11_Execute(register P11_CPU *);

// memory.c
uint16 *p11_InitMemory(register P11_CPU *, uint32);
void    p11_FreeMemory(register P11_CPU *);
uint16  p11_ReadPW(register P11_CPU *, uint32);
uint16  p11_ReadPB(register P11_CPU *, uint32);
void    p11_WritePW(register P11_CPU *, uint32, uint16);
void    p11_WritePB(register P11_CPU *, uint32, uint16);
uint16  p11_ReadW(register P11_CPU *, uint32);
uint8   p11_ReadB(register P11_CPU *, uint32);
uint16  p11_ReadMW(register P11_CPU *, uint32, uint32 *);
uint8   p11_ReadMB(register P11_CPU *, uint32, uint32 *);
void    p11_WriteW(register P11_CPU *, uint32, uint16);
void    p11_WriteB(register P11_CPU *, uint32, uint8);
uint32  p11_GetSpace(register P11_CPU *, uint32);
uint16  p11_ReadCP(register P11_CPU *, uint32, uint32);
void    p11_WriteCP(register P11_CPU *, uint32, uint16, uint32);
uint16  p11_ReadC(register P11_CPU *, uint32, uint32);
void    p11_WriteC(register P11_CPU *, uint32, uint16, uint32);
