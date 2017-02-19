// symbols.c - PDP-10 Symbol Table routines
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

#include "pdp10/defs.h"

#define SYM_FLAGS  0740000000000 // Symbol Flags
#define SYM_NAME   0037777777777 // Symbol Name in RADIX-50 format

#define SYM_M_FLAGS 074 // Mask of symbol flags
#define SYM_P_FLAGS 30  // Position of symbol flags

// Symbol flags
#define SYM_PNAME  074 // Program Name
#define SYM_GLOBAL 004 // Global Symbol
#define SYM_LOCAL  010 // Local Symbol
#define SYM_DELI   020 // Delete Input
#define SYM_DELO   040 // Delete Output

#define JOB_JBSYM  0116
#define JOB_JBUSY  0117

#define SYCNT  0
#define SYSTB  1
#define SYTYP  0
#define SYADR  1
#define SYSTL  3

typedef struct tblSymbol SYMBOL;
struct tblSymbol {
	char   ascName[6];
	uint32 r50Name;
	int32  Flags;
	int36  Value;
};

SYMBOL *p10_dSymbols   = NULL; // Defined Symbol Table
int    p10_dSymbolSize = 0;    // Length of Symbol Table
SYMBOL *p10_uSymbols   = NULL; // Undefined Symbol Table
int    p10_uSymbolSize = 0;    // Length of Symbol Table

static int36   dSymbol;     // Defined Symbol Table
static int36   uSymbol;     // Undefined Symbol Table

static int32   symCount;    // Length of Symbol Table Header
static int32   symAddr;     // Address of Symbol Table
static int32   symAddr2;    // Address of Symbol Subtable
static int32   symLength;   // Length of Symbol Table
static int36   symType;     // Type of Symbol Table
static int32   symFlags;    // Symbol Table Flags
static int32   symTable;    // Address of Subtable
static int     symExtended; // Single section or Extended.

// Decipher a RADIX-50 word into a string.
char *p10_GetSymName(uint32 symName)
{
	static char ascName[7];
	char r50Name[6];
	int idx, p;

	// Decipher RADIX-50 string.
	for (idx = 0; idx < 6; idx++) {
		r50Name[idx] = symName % 050;
		symName /= 050;
	}
	for (idx = 5, p = 0; idx >= 0; idx--)
		ascName[p++] = // what an array of string! :-)
			" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.$%"[r50Name[idx]];
	ascName[p] = '\0';

	// Remove spaces from the
	// beginning of symbol name.
	for (idx = 0; idx < 6; idx++)
		if (ascName[idx] != ' ')
			break;

	return &ascName[idx];
}

#ifdef DEBUG
void p10_DisplaySymbols(int32 symAddr, int32 symLength)
{
	int36 symWord;  // Flags, Name in RADIX-50 format
	int36 symValue; // Value/Address
	int32 symFlags; // Symbol flags
	int32 symName;  // Symbol Name

	dbg_Printf("SYM: Symbol Table at %08o\n", symAddr);
	dbg_Printf("SYM:   Name    Flags  Value\n");
	dbg_Printf("SYM:   ------  -----  --------------\n");

	while (symLength > 0) {
		symWord   = p10_pRead(symAddr, 0);
		symValue  = p10_pRead(symAddr+1, 0);
		symFlags  = (symWord >> SYM_P_FLAGS) & SYM_M_FLAGS;
		symName   = symWord;
		symAddr   += 2;
		symLength -= 2;

		dbg_Printf("SYM:   %-6s  %02o     %06o,,%06o\n",
			p10_GetSymName(symName), symFlags, LH18(symValue), RH18(symValue));
	}
}
#endif /* DEBUG */

SYMBOL *p10_CreateSymbolTable(int30 symAddr, int30 symLength)
{
	SYMBOL *newSymbol; // New Symbol Table
	int36  symWord;    // Flags, Name in RADIX-50 format
	int    idx;

	newSymbol = (SYMBOL *)calloc(symLength >> 1, sizeof(SYMBOL));
	for (idx = 0; symLength > 0; idx++) {
		symWord = p10_pRead(symAddr, 0);
		strncpy(newSymbol[idx].ascName, p10_GetSymName(symWord), 6);
		newSymbol[idx].r50Name = symWord;
		newSymbol[idx].Flags   = (symWord >> SYM_P_FLAGS) & SYM_M_FLAGS;
		newSymbol[idx].Value   = p10_pRead(symAddr+1, 0);

		symAddr   += 2;
		symLength -= 2;
	}

	printf("Symbol table: %d symbols\n", idx);

	return newSymbol;
}

void p10_BuildSymbols(void)
{
	dSymbol = p10_pRead(JOB_JBSYM, 0);

	if (dSymbol == 0) {
#ifdef DEBUG
		dbg_Printf("SYM: No Symbol Table\n");
#endif /* DEBUG */
		return;
	}

	if (dSymbol < 0) {
		uSymbol = p10_pRead(JOB_JBUSY, 0);
		symAddr   = RH(dSymbol);
		symLength = -(dSymbol >> 18);
#ifdef DEBUG
		dbg_Printf("SYM: Symbol Table - Single Section\n");
		dbg_Printf("SYM: Table Address: %06o  Length: %06o\n",
			symAddr, symLength);
#endif /* DEBUG */
	} else {
		uSymbol = dSymbol;
		symAddr = dSymbol;

		symCount = p10_pRead(symAddr, 0);
		if (symCount < (SYSTB + (SYSTL * 2))) {
#ifdef DEBUG
			dbg_Printf("SYM: Symbol Table Vector Too Short - Aborted.\n");
#endif /* DEBUG */
			return;
		}

#ifdef DEBUG
		dbg_Printf("SYM: Symbol Table - Extended Addressing\n");
#endif /* DEBUG */

		symTable = symAddr + SYSTB;
		while (symTable < (symAddr + symCount)) {
			symType   = p10_pRead(symTable + SYTYP, 0);
			symAddr2  = p10_pRead(symTable + SYADR, 0);
			symLength = symType & 07777777777;
			symFlags  = (symType >> 30) & 077;
			symTable  += SYSTL;

#ifdef DEBUG
			dbg_Printf("SYM: Subtable Address: %o,,%06o Length: %o,,%06o Flags: %02o\n",
				SR(symAddr2), symAddr2 & 0777777,
				SR(symLength), symLength & 0777777, symFlags);

			p10_DisplaySymbols(symAddr2, symLength);
#endif /* DEBUG */

			if (symFlags == 1) {
				p10_dSymbols = p10_CreateSymbolTable(symAddr2, symLength);
				p10_dSymbolSize = symLength >> 1;
			}
		}
	}
}

char *p10_FindSymbol(int36 symValue)
{
	SYMBOL *cptr;
	int    idx;

	if (p10_dSymbols == NULL)
		return NULL;

	for (idx = 0; idx < p10_dSymbolSize; idx++) {
		cptr = &p10_dSymbols[idx];
		if (!(cptr->Flags & SYM_DELO) && (cptr->Value == symValue))
			return cptr->ascName;
	}
	return NULL;
}
