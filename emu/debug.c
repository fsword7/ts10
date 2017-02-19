// debug.c - Debugging Facility
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

#ifdef DEBUG

#include "emu/defs.h"
#include "emu/socket.h"

extern MAP_DEVICE *ts10_Use;

DBG_MODES dbg_Modes[] = {
	{ "Console",     DBG_CONSOLE },
	{ "Data",        DBG_DATA  },
	{ "Interrupt",   DBG_INTERRUPT },
	{ "IOData",      DBG_IODATA },
	{ "IORegs",      DBG_IOREGS },
	{ "IPS",         DBG_IPS     },
	{ "MUUO",        DBG_MUUO    },
	{ "Operand",     DBG_OPERAND },
	{ "PageFault",   DBG_PAGEFAULT },
	{ "PCChange",    DBG_PCCHANGE },
	{ "Prefetch",    DBG_PREFETCH },
	{ "Register",    DBG_REGISTER },
	{ "Sockets",     DBG_SOCKETS },
	{ "SockErrors",  DBG_SOCKERR },
	{ "Table",       DBG_TABLE },
	{ "Timer",       DBG_TIMER },
	{ "Trace",       DBG_TRACE },
	{ "Translation", DBG_TRANSLATION },
	{ NULL,          0         }  // Null terminator
};

// History information for keep recent happens.
#define HST_FILENAME "history.log"
#define HST_ENABLE   1      // History Enable
#define HST_DEFLINES 10000  // Default number of lines

static int  flgHistory = 0;    // History flags
static int  maxHistory = 0;    // Maximum number of history lines.
static int  ptrHistory = 0;    // Current line of history data.
static char **History  = NULL; // Record history information.

// Debug log file
#define DBG_FILENAME "debug.log"

static FILE *debug = NULL;
SOCKET *dbg_File = NULL;
int32  dbg_Mode  = 0;

int OpenDebug(char *fileName)
{
	if (fileName == NULL)
		fileName = DBG_FILENAME;

	if (debug = fopen(fileName, "w"))
		dbg_File = sock_Open(fileName, fileno(debug), NET_FILE);
	else 
		perror(fileName);

	return TS10_OK;
}

int CloseDebug(void)
{
	if (debug) {
		sock_Close(dbg_File);
		fclose(debug);
	}
	debug    = NULL;
	dbg_File = NULL;

	return TS10_OK;
}

int dbg_Check(int mode)
{
	return ((dbg_Mode & mode) == mode);
}

void dbg_SetMode(int newMode)
{
	dbg_Mode |= newMode;
}

void dbg_ClearMode(int newMode)
{
	dbg_Mode &= ~newMode;
}

int dbg_GetMode(void)
{
	return dbg_Mode;
}

void dbg_PutMode(int newMode)
{
	dbg_Mode = newMode;
}

void dbg_Printf(cchar *Format, ...)
{
	char tmpBuffer[1024];
	va_list Args;
	int len;

	va_start(Args, Format);
	len = vsnprintf(tmpBuffer, 1023, Format, Args);
	tmpBuffer[1023] = 0;
	va_end(Args);

	// Record recent log information if enabled.
	if (flgHistory & HST_ENABLE) {
		if (History[ptrHistory])
			free(History[ptrHistory]);
		History[ptrHistory] = (char *)malloc(len);
		strcpy(History[ptrHistory], tmpBuffer);
		if (++ptrHistory == maxHistory)
			ptrHistory = 0;
	}

	if (debug) {
		fwrite(tmpBuffer, len, 1, debug);
		fflush(debug);
	}
}

void DumpHistory(void)
{
	int curHistory, lenHistory;
	int hstFile;

	if ((flgHistory & HST_ENABLE) == 0)
		return;

	if ((hstFile = open("history.log", O_WRONLY|O_CREAT, 0700)) < 0) {
		perror("history.log");
		return;
	}

	curHistory = ptrHistory;
	do {
		if (History[curHistory]) {
			// Write history line to a file.
			write(hstFile, History[curHistory], strlen(History[curHistory]));

			// Free histroy line.
			free(History[curHistory]);
			History[curHistory] = NULL;
		}

		// Go next history line.
		if (++curHistory == maxHistory)
			curHistory = 0;
	} while (curHistory == ptrHistory);

	close(hstFile);
}

void PrintDump(uint32 addr, uint8 *data, uint32 size)
{
	uchar  ascBuffer[17];
	uchar  ch, *pasc;
	int    idxAddr, idx;

	for(idxAddr = 0; idxAddr < size;) {
		SockPrintf(dbg_File, "%08X: ", addr + idxAddr);

		pasc = ascBuffer;
		for (idx = 0; (idx < 16) && (idxAddr < size); idx++) {
			ch = data[idxAddr++];
			SockPrintf(dbg_File, "%c%02X", (idx == 8) ? '-' : ' ', ch);
			*pasc++ = ((ch >= 32) && (ch < 127)) ? ch : '.';
		}

		for (; idx < 16; idx++)
			SockPrintf(dbg_File, "   ");
		*pasc = '\0';
		SockPrintf(dbg_File, "  |%-16s|\n", ascBuffer);
	}
}

int CmdDebug(void *dev, int argc, char **argv)
{
	int  idx;

	if (argc == 1) {
		for (idx = 0; dbg_Modes[idx].Name; idx++)
			printf("Debug %s:\t%s\n", dbg_Modes[idx].Name,
				(dbg_Mode & dbg_Modes[idx].Mode) ? "on" : "off");
	} else {
		for (idx = 0; dbg_Modes[idx].Name; idx++)
			if (!strcasecmp(argv[1], dbg_Modes[idx].Name))
				break;

		if (dbg_Modes[idx].Name) {
			if (argc == 2)
				printf("Debug %s: %s\n", dbg_Modes[idx].Name,
					(dbg_Mode & dbg_Modes[idx].Mode) ? "on" : "off");
			else {
				if (argc == 3) {
					RemoveSpaces(argv[2]);
					if (!strcasecmp(argv[2], "on")) {
						dbg_Mode |= dbg_Modes[idx].Mode;
						printf("Debug %s had been turned on.\n",
							dbg_Modes[idx].Name);
					} else if (!strcasecmp(argv[2], "off")) {
						dbg_Mode &= ~dbg_Modes[idx].Mode;
						printf("Debug %s had been turned off.\n",
							dbg_Modes[idx].Name);
					}
				} else
					printf("Usage: debug [mode] [on|off]\n");
			}
		} else
			printf("Unknown debug mode: %s\n", argv[1]);
	}

	return EMU_OK;
}


// Usage: deposit [switches] <address[-address]>
int CmdDeposit(void *dev, int argc, char **argv)
{
}

// Usage: examine [switches] <address[-address]>
int CmdExamine(void *dev, int argc, char **argv)
{

}

int CmdHistory(void *dev, int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: history <on|off>\n");
		return TS10_OK;
	} else if (!strcasecmp(argv[1], "on")) {
		if (History == NULL) {
			History    = (char **)calloc(sizeof(int), HST_DEFLINES);
			if (History == NULL) {
				printf("Can't create history data - Not enough memory.\n");
				return TS10_OK;
			}
			printf("History was turned on with new %d history lines.\n",
				HST_DEFLINES);
			maxHistory = HST_DEFLINES;
		} else
			printf("History was turned on.\n");
		flgHistory = HST_ENABLE;
	} else if (!strcasecmp(argv[1], "off")) {
		flgHistory &= ~HST_ENABLE;
		printf("History turned off.\n");
	}
}

int CmdTrace(void *dev, int argc, char **argv)
{
	if (argc == 1)
		printf("Trace: %s\n", (dbg_Mode & DBG_TRACE) ? "on" : "off");
	else {
		RemoveSpaces(argv[1]);
		if (!strcasecmp(argv[1], "on")) {
			dbg_Mode |= DBG_TRACE;
			printf("Trace had been turned on.\n");
		} else if (!strcasecmp(argv[1], "off")) {
			dbg_Mode &= ~DBG_TRACE;
			printf("Trace had been turned off.\n");
		} else
			printf("Usage: trace [on|off]\n");
	}
	return EMU_OK;
}

// *****************
// Breakpoint System
// *****************

// Find breakpoint entry in sorted breakpoint table
DBG_BREAK *dbg_FindBreak(DBG_BRKSYS *brksys, uint32 loc)
{
	register DBG_BREAK *brk;

	if (brksys->List == NULL)
		return NULL;

	// Get current break entry
//	if ((brk = brksys->cBreak) == NULL)
//		brk = brksys->List;

	if (loc < brk->Addr) {
		do {
			if (brk->Back)
				brk = brk->Back;
			else {
//				brksys->cBreak = brk;
				return NULL;
			}
		} while (loc < brk->Addr);
	} else if (loc > brk->Addr) {

	}

	// Found a break and check for countdown.
	if (loc == brk->Addr) {
		if (--brk->Count <= 0) {
			brk->Count = 0;
			return brk;
		}
		return NULL;
	}

	// Set current break
//	brksys->cBreak = brk;
	return NULL;
}

DBG_BREAK *dbg_CheckBreak(DBG_BRKSYS *brk, uint32 loc, uint32 sw)
{
	return NULL;
}

int CmdBreak(void *dptr, int argc, char **argv)
{
	DBG_BRKSYS *brkSystem;
	DBG_BREAK  *newBreak;
	MAP_DEVICE *map;
	int        newArgc;
	void       **newArgv;
	int        ok = FALSE;

	if (argc < 2)
		printf("Usage: %s [switch] <address> [command line]\n", argv[0]);
	else {
		// Check device for requirments first
		if ((map = ts10_Use) == NULL) {
			printf("Enter 'USE <device>' first.\n");
			return TS10_OK;
		}
		if ((brkSystem = map->Breaks) == NULL) {
			printf("%s(%s): Breakpoint system is not supported.\n",
				map->devName, map->keyName);
			return TS10_OK;
		}

//		sscanf(argv[2], "%x", &newAddr);
//		if (newBreak = (DBG_BREAK *)calloc(1, sizeof(BREAK))) {
//			if ((newArgc = argc - 3) == 0) {
//				// No command line
//				newArgv == NULL;
//				ok = TRUE;
//			} else if (newArgv = (void *)malloc(sizeof(void) * newArgc)) {
//				pa = &argv[3];
//				for (idx = 0; idx < newArgc; idx++) {
//					newArgv[idx] = (void *)malloc(strlen(*pa)+1);
//					strcpy(newArgv[idx], *pa++);
//				}
//				ok = TRUE;
//			}
//		}
	}

//	if (ok) {
//		// Set up new breakpoint entry
//		newBreak->Next    = io->Breaks;
//		newBreak->brkAddr = newAddr;
//		newBreak->cntDown = newCountdown;
//		newBreak->argc    = newArgc;
//		newBreak->argv    = newArgv;
//		io->Breaks        = newBreak;
//	} else {
//		printf("Can't set breakpoint.\n");
//	}

	return EMU_OK;
}

int CmdNoBreak(void *dptr, int argc, char **argv)
{
	return EMU_OK;
}

#endif /* DEBUG */
