// proto.h - Prototypes for the emulation routines
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS10 Emulator.
// See ReadMe for copyright notice.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

void ts10_Exit(char *);

// Console Commands - console.c
int  Execute(char *);
void ExecuteFile(char *);
void ExecutePrompt(char *);
void ListAllCommands(void);

int CmdConfigure(void *, int, char **);
int CmdCreate(void *, int, char **);
int CmdCreate2(void *, int, char **);
int CmdDelete(void *, int, char **);
int CmdAttach(void *, int, char **);
int CmdDetach(void *, int, char **);
int CmdFormat(void *, int, char **);
int CmdInit(void *, int, char **);
int CmdUse(void *, int, char **);
int CmdBoot(void *, int, char **);
int CmdDebug2(void *, int, char **);

// Timer - emu/timer.h
int    ts10_StartTimer(void);
int    ts10_StopTimer(void);
int    ts10_SetAlarm(void (*)(int));
void   ts10_TickRealTimer(int);
void   ts10_SetRealTimer(CLK_QUEUE *);
uint32 ts10_GetGlobalTime(void);
void   ts10_InitTimer(void);
void   ts10_SetTimer(CLK_QUEUE *);
void   ts10_CancelRealTimer(CLK_QUEUE *);
void   ts10_CancelTimer(CLK_QUEUE *);
void   ts10_ExecuteTimer(void);

// panel.c
void       InitControlPanel(void);
void       CleanupControlPanel(void);

// system.c
void       InitSystem(void);
MAP_DEVICE *CreateDevice(char *);
void       DestroyDevice(MAP_DEVICE *);
MAP_DEVICE *InsertDevice(MAP_DEVICE *, MAP_DEVICE *);
MAP_DEVICE *FindDevice(MAP_DEVICE *, char *);
MAP_DEVICE *GetDevice(MAP_DEVICE *, int, char **);
DEVICE     *GetDeviceInfo(MAP_DEVICE *, int, char **);
int        GetDeviceUnit(char *);

int   CmdCreate(void *, int, char **);
int   CmdAttach(void *, int, char **);
int   CmdDetach(void *, int, char **);
int   CmdInfo(void *, int, char **);
int   CmdSelect(void *, int, char **);
int   CmdUse(void *, int, char **);
int   CmdSet2(void *, int, char **);
int   CmdShow2(void *, int, char **);

int   CmdShowDevice(void *, int, char **);
int   CmdListDevice(void *, int, char **);

// Utilities - emu/utils.c
void  RemoveSpaces(register char *);
char  *SplitChar(register char **, register char);
char  *SplitWord(register char **);
char  *SplitQuote(register char **);
void  ToUpper(register char *);
int   GetInteger(char *, int, int, int *);
int   ToInteger(char *, char **, int);
// char  *ToBase10(uint32);
char  *ToRadix(char *, uint32, int, int);
int36 Convert8to36(uchar *);
void  Convert36to8(int36, uchar *);
int36 PackedASCII6(uchar *);
int36 PackedASCII7(uchar *);
char  *NowTime(cchar *);
void  Printf(cchar *, ...);
char  *StrChar(char *, char);
int32 GetSwitches(char *, uint32 *, uint32 *, uint32 *);

#ifdef DEBUG
// Debugging Facility - emu/debug.c
int     dbg_Check(int);
void    dbg_SetMode(int);
void    dbg_ClearMode(int);
int     dbg_GetMode(void);
void    dbg_PutMode(int);
void    dbg_Printf(cchar *, ...);
int     CmdDebug(void *, int, char **);
int     CmdTrace(void *, int, char **);
int     CmdAsm(void *, int, char **);
int     CmdDisasm(void *, int, char **);
int     CmdHistory(void *, int, char **);
int     CmdBreak(void *, int, char **);
int     CmdNoBreak(void *, int, char **);
int     OpenDebug(char *);
int     CloseDebug(void);
void    PrintDump(uint32, uint8 *, uint32);
#endif /* DEBUG */
