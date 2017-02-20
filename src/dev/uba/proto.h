// proto.h - Prototypes for the device routines
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

// RH Interface - rh.c
int   rh_Create(UNIT *, char *, int, char **);
int   rh_Delete(UNIT *);
void  rh_Initialize(UNIT *);
void  rh_Reset(UNIT *);

void  rh_SetATA(UNIT *, int32);
void  rh_ClearATA(UNIT *, int32);
void  rh_Ready(UNIT *);
int   rh_CheckWordCount(UNIT *);
int32 rh_ReadIO(UNIT *, int32);
void  rh_WriteIO(UNIT *, int32, int32);
int   rh_ReadData18(UNIT *, int18 *, boolean);
int   rh_ReadData36(UNIT *, int36 *, boolean);
int   rh_WriteData18(UNIT *, int18, boolean);
int   rh_WriteData36(UNIT *, int36, boolean);
int   rh_Process(UNIT *);

// RM Interface - rm.c
int   rm_Create(UNIT *, char *, int, char **);
int   rm_Delete(UNIT *);
int   rm_Enable(UNIT *);
int   rm_Disable(UNIT *);
int   rm_Attach();
int   rm_Detach();

void  rm_Initialize(UNIT *);
void  rm_Reset(UNIT *);
UNIT  *rm_SetUnit(UNIT *);
void  rm_SetATA(UNIT *);
void  rm_ClearATA(UNIT *);
int32 rm_ReadIO(UNIT *, int32);
void  rm_WriteIO(UNIT *, int32, int32);
void  rm_Go(UNIT *);
int   rm_Process(UNIT *);
int   rm_Boot(UNIT *, int, char **);

// RP Interface - rp.c
int   rp_Create(UNIT *, char *, int, char **);
int   rp_Delete(UNIT *);
int   rp_Enable(UNIT *);
int   rp_Disable(UNIT *);
int   rp_Attach();
int   rp_Detach();

void  rp_Initialize(UNIT *);
void  rp_Reset(UNIT *);
int   rp_GetDiskAddr(UNIT *, int, int, int);
int   rp_Format(UNIT *);
UNIT  *rp_SetUnit(UNIT *);
void  rp_SetATA(UNIT *);
void  rp_ClearATA(UNIT *);
int32 rp_ReadIO(UNIT *, int32);
void  rp_WriteIO(UNIT *, int32, int32);
void  rp_Go(UNIT *);
int   rp_Process(UNIT *);
int   rp_Boot(UNIT *, int, char **);

// TCU150 Interface - tcu.c
void  tcu_Initialize(UNIT *);
void  tcu_Reset(UNIT *);
int   tcu_Create(UNIT *, char *, int, char **);
int   tcu_Delete(UNIT *);
int32 tcu_ReadIO(UNIT *, int32);
void  tcu_WriteIO(UNIT *, int32, int32);

// TM02/TM03 Interface - tm.c
int   tm_Create(UNIT *, char *, int, char **);
int   tm_CreateSlave(UNIT *, int32, int, char **);
int   tm_Delete(UNIT *, int32);
int   tm_Enable(UNIT *, int32);
int   tm_Disable(UNIT *, int32);
int   tm_Attach();
int   tm_Detach();

void  tm_Initialize(UNIT *);
void  tm_Reset(UNIT *);
void  tm_SetATA(UNIT *);
void  tm_ClearATA(UNIT *);
void  tm_SetStatus(UNIT *, int16);
void  tm_ClearStatus(UNIT *, int16);
int32 tm_ReadIO(UNIT *, int32);
void  tm_WriteIO(UNIT *, int32, int32);
void  tm_Go(UNIT *);
int   tm_Process(UNIT *);
int   tm_Boot(UNIT *, int, char **);
UNIT  *tm_SetUnit(UNIT *, int32);
