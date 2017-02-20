// system.c - system routines for PDP10 emulation
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

#define PDP10_KEY      "PDP10"
#define PDP10_NAME     "DECsystem-10/20 Emulator"
#define PDP10_VERSION  "v0.8.7 (Alpha)"

// **************************************************************

char *pdp10_DisplayData(int36 data)
{
	static char text[80];
	char  text6[7];  // SIXBIT
	char  text7[6];  // 7-bit ASCII
	char  text50[7]; // RADIX-50
	uint32 srad50;
	char  rad50[6];
	int i, p;

	data &= WORD36_ONES;

	for (i = 30, p = 0; i >= 0; i -= 6, p++)
		text6[p] = ((data >> i) & 077) + 040;
	text6[p] = 0;

	for (i = 29, p = 0; i >= 1; i -= 7, p++) {
		text7[p] = (data >> i) & 0177;
		if ((text7[p] < 32) || (text7[p] > 126))
			text7[p] = ' ';
	}
	text7[p] = 0;

	// Decipher RADIX-50 string.
	srad50 = data;
	for (i = 0; i < 6; i++) {
		rad50[i] = srad50 % 050;
		srad50 /= 050;
	}
	for (i = 5, p = 0; i >= 0; i--)
		text50[p++] = // what an array of string! :-)
			" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.$%"[rad50[i]];
	text50[p] = '\0';

	sprintf(text, "%06llo,,%06llo ('%s' '%s' '%s')",
		LHSR(data), RH(data), text6, text7, text50);

	return text;
}

/*
 * DEC Core-Dump file.
 * Format for storing 36-bits into 5 tape frames.
 *
 *    DEC Core-Dump Mode          ANSI Compatible Mode
 * |00 01 02 03 04 05 06|07     |__ 00 01 02 03 04 05 06|
 *  08 09 10 11 12 13|14 15     |__ 07 08 09 10 11 12 13|
 *  16 17 18 19 20|21 22 23     |__ 14 15 16 17 18 19 20|
 *  24 25 26 27|28 29 30 31     |__ 21 22 23 24 25 26 27|
 *  __ __ __ __ 32 33 34|35|    |35|28 29 30 31 32 33 34|
 *
 * Note: "|" separate the 7-bit bytes,
 * "__" are unused bits (which is zeros).
 *
 */

int36 pdp10_Convert8to36(uchar *data8)
{
	int36 data36;

	data36 = data8[0];
	data36 = (data36 << 8) | data8[1];
	data36 = (data36 << 8) | data8[2];
	data36 = (data36 << 8) | data8[3];
	data36 = (data36 << 4) | data8[4];

	return data36;
}

uchar *pdp10_Convert36to8(int36 data36)
{
	static uchar data8[6];

	data8[0] = (data36 >> 28) & 0xFF;
	data8[1] = (data36 >> 20) & 0xFF;
	data8[2] = (data36 >> 12) & 0xFF;
	data8[3] = (data36 >>  4) & 0xFF;
	data8[4] = data36 & 0xF;
	data8[5] = '\0';

	return data8;
}

int exe_GetWord(int inFile, int36 *data)
{
	char inBuffer[5];
	int  st;

	if ((st = read(inFile, &inBuffer, 5)) > 0) {
		*data = inBuffer[0] & 0377;
		*data = (inBuffer[1] & 0377) | (*data << 8);
		*data = (inBuffer[2] & 0377) | (*data << 8);
		*data = (inBuffer[3] & 0377) | (*data << 8);
		*data = (inBuffer[4] & 0017) | (*data << 4);
	}

	return st;
}

// Load RIM file into memory
int pdp10_LoadRimFile(char *rimFilename)
{
	int   rimFile;
	char  inBuffer[5];
	int18 len, addr;
	int36 data36;

	if ((rimFile = open(rimFilename, O_RDONLY)) < 0) {
		perror(rimFilename);
		return EMU_OK;
	}

	while (exe_GetWord(rimFile, &data36) > 0) {
		len = LHSR(data36);
		len = SXT18(len);
		addr = RH(data36);

		if (len < 0) {
			printf("RIM: Address: %06o at %06o words\n", addr+1, -len);
			while (len++) {
				if (exe_GetWord(rimFile, &data36) <= 0)
					break;
				p10_pWrite(++addr, data36, 0);
			}
		}
	}

	close(rimFile);

	// Set PC to execute that program.
	data36 = p10_pRead(0120, 0);
	PC = RH(data36);
	printf("RIM: Start Address: %06llo\n", PC);

	return EMU_OK;
}

// Load EXE file into memory
int pdp10_LoadExeFile(char *exefilename)
{
	int exefile;
	int idx, idx1, len, st;
	uchar cBuffer[SV_BLK_SIZE * 5];
	int36 data36;
	int36 ndir;      // number of directory entries
	int36 dir[128];  // directory entries for page information
	int36 id, flags, fpage, ppage, paddr, count;

	if ((exefile = open(exefilename, O_RDONLY)) < 0) {
		perror(exefilename);
		return EMU_OK;
	}

	// Now read EXE header information
	do {
		st = read(exefile, cBuffer, 5);
		data36 = pdp10_Convert8to36(cBuffer);
		id = (data36 >> 18) & 0777777;
		len = (data36 & 0777777) - 1;

		switch (id) {
			case SV_ID_DIRECTORY:
				ndir = len;
				if (ndir >= 128) {
					printf("Error: %s has too long directory entry.\n",
						exefilename);
#ifdef DEBUG
//					dbg_Printf("EXE: %s has too long directory entry.\n",
//						exefilename);
#endif /* DEBUG */
					return EMU_OK;
				}
				st = read(exefile, cBuffer, ndir * 5);
				idx = 0;
				while (idx < ndir) {
					dir[idx] = pdp10_Convert8to36(&cBuffer[idx*5]);
					idx++;
				}

#ifdef DEBUG
//				dbg_Printf("EXE: Directory Entries:\n");
#endif /* DEBUG */
				idx = 0;
				while (idx < ndir) {
					flags = dir[idx];
#ifdef DEBUG
//					dbg_Printf("EXE: Flags: %c%c%c%c%c%c Repeat: %06llo File Page: %06llo CPU Page: %06llo\n",
//						((flags & SV_M_HIGH_SEG)  ? 'H' : '-'),
//						((flags & SV_M_SHARABLE)  ? 'S' : '-'),
//						((flags & SV_M_WRITABLE)  ? 'W' : '-'),
//						((flags & SV_M_CONCEALED) ? 'C' : '-'),
//						((flags & SV_M_SYM_TABLE) ? 'T' : '-'),
//						((flags & SV_M_ALLOCATED) ? 'A' : '-'),
//						((dir[idx+1] >> 18) & 0777777),
//						(dir[idx] & 0777777),
//						(dir[idx+1] & 0777777));
#endif /* DEBUG */
					idx += 2;
				}

				break;

//			case SV_ID_ENTRY_VECTOR:
//#ifdef DEBUG
//				dbg_Printf("EXE: Entry Vector\n");
//#endif /* DEBUG */
//				break;

			case SV_ID_END_BLOCK:
#ifdef DEBUG
//				dbg_Printf("EXE: End of block, now loading data\n");
#endif /* DEBUG */
				break;

			default:
#ifdef DEBUG
//				dbg_Printf("EXE: Unknown ID code %06llo\n", id);
#endif /* DEBUG */
				lseek(exefile, len * 5, SEEK_CUR); /* Skip data */
				break;
		}
	} while (id != SV_ID_END_BLOCK);

	// Seek to the starting file page 1
	st = lseek(exefile, SV_BLK_SIZE * 5, SEEK_SET);
#ifdef DEBUG
//	dbg_Printf("EXE: Starting data block at %d\n", st);
#endif /* DEBUG */

	// Now start to load data block.
	idx = 0;
	while (idx < ndir) {
		flags = (dir[idx] >> 18) & 0777777;
		fpage = dir[idx] & 0777777;
		count = (dir[idx+1] >> 18) & 0777777;
		ppage = dir[idx+1] & 0777777;
		paddr = ppage << 9;

#ifdef DEBUG
//		dbg_Printf("EXE: Starting address at %06llo (Page %06llo)\n",
//			paddr, ppage);
#endif /* DEBUG */
		while (count >= 0) {
			if (fpage) {
#ifdef DEBUG
//				dbg_Printf("EXE:   Fetch file page %06llo\n", fpage);
#endif /* DEBUG */
				st = read(exefile, cBuffer, SV_BLK_SIZE * 5);
				fpage++;
			}
#ifdef DEBUG
//			dbg_Printf("EXE:   Load data block into address %06llo\n", paddr);
#endif /* DEBUG */
			if (fpage) {
				idx1 = 0;
				while (idx1 < SV_BLK_SIZE) {
					data36 = pdp10_Convert8to36(&cBuffer[(idx1++)*5]);
					p10_pWrite(paddr, SXT36(data36), 0);
#ifdef DEBUG
//					dbg_Printf("EXE:     %06llo: %s\n", 
//						paddr, pdp10_DisplayData(data36));
#endif /* DEBUG */
					paddr++;
				}
			} else {
				while (idx1++ < SV_BLK_SIZE)
					p10_pWrite(paddr++, 0, 0);
			}

			ppage++;
			count -= SV_BLK_SIZE;
		}
		idx += 2;
	}

	close(exefile);

	// Set PC to execute that program.
	data36 = p10_pRead(0120, 0);

	PC = RH(data36);
	printf("Start Address: %06llo\n", PC);
#ifdef DEBUG
//	dbg_Printf("EXE: Start Address: %06llo\n", PC);
#endif /* DEBUG */

	// Try to build symbol tables.
	p10_BuildSymbols();

	return EMU_OK;
}

int pdp10_Start(char *strAddr)
{
	sscanf(strAddr, "%o", &PC);
	printf("CPU: Start Address: %06o\n", PC);
	return EMU_OK;
}

// ***********************************************************

void *p10_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	P10_SYSTEM *p10 = NULL;

	if (p10 = (P10_SYSTEM *)calloc(1, sizeof(P10_SYSTEM))) {
		// Set up its descriptions on new device.
		p10->devName      = newMap->devName;
		p10->keyName      = newMap->keyName;
		p10->emuName      = newMap->emuName;
		p10->emuVersion   = newMap->emuVersion;

		newMap->Device    = p10;
		newMap->sysDevice = p10;
		newMap->sysMap    = newMap;
	}
	return p10;
}

extern COMMAND p10_Commands[];
extern COMMAND p10_SetCommands[];
extern COMMAND p10_ShowCommands[];

extern DEVICE ks10_Processor;
extern DEVICE kl10a_Processor;
#ifdef OPT_XADR
extern DEVICE kl10b_Processor;
#endif /* OPT_XADR */

DEVICE *p10_Systems[] =
{
	&ks10_Processor,  // KS10 Processor
	&kl10a_Processor, // KL10A Processor w/o extended addressing
#ifdef OPT_XADR
	&kl10b_Processor, // KL10B Processor w/extended addressing
#endif /* OPT_XADR */
	NULL
};

DEVICE p10_System =
{
	PDP10_KEY,         // Type Name
	PDP10_NAME,        // Emulator Name
	PDP10_VERSION,     // Emulator Version
	p10_Systems,       // System Listings
	DF_SELECT,         // Device Flags
	DT_SYSTEM,         // Device Type

	p10_Commands,      // Commands
	p10_SetCommands,   // Set Commands
	p10_ShowCommands,  // Show Commands

	p10_Create,        // Create Routine
	NULL,              // Configure Routine
	NULL,              // Delete Routine
	NULL,              // Reset Routine
	NULL,              // Attach Routine (Not Used)
	NULL,              // Detach Routine (Not Used)
	NULL,              // Info Routine
	NULL,              // Boot Routine
	p10_Go,            // Execute Routine
#ifdef DEBUG
	NULL,              // Debug Routine
#endif /* DEBUG */
};
