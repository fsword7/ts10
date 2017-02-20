// func.h - Definitions of function codes for RH11 interface.
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

// Function code 
//
// Func  Fixed disk         Pack disk                Magnetic tape
// ----  ----------         ---------                -------------
// 000   No-op              No-op                    No-op
// 001                      Unload                   Rewind, offline
// 002   Seek               Seek
// 003   Recalibrate        Recalibrate              Rewind
// 004   Drive clear        Drive clear              Drive clear
// 005                      Release
// 006                      Offset
// 007                      Return to Clearline
// 010   Read-in Preset     Read-in Preset           Read-in Preset
// 011                      Pack Acknowledge
// 012                                               Erase
// 013                                               Write File Mark
// 014   Search             Search                   Space Forward
// 015                                               Space Reverse
// 024   Write Check Data   Write Check Data         Write Check forward
// 025                      Write Check Headr/Data
// 026                                               Write Check Reverse
// 030   Write data         Write data               Write Forward
// 031                      Write Header/Data
// 034   Read Data          Read Data                Read Forward
// 035                      Read Header/Data
// 037                                               Read Reverse

#define FNC_NOP         000 // (001) No operation
#define FNC_UNLOAD      001 // (003) Unload/Rewind and offline
#define FNC_SEEK        002 // (005) Seek
#define FNC_RECAL       003 // (007) Recalibrate
#define FNC_REWIND      003 // (007) Rewind
#define FNC_DCLR        004 // (011) Drive Clear
#define FNC_RELEASE     005 // (013) Release
#define FNC_OFFSET      006 // (015) Offset
#define FNC_RETURN      007 // (017) Return to clearline
#define FNC_PRESET      010 // (021) Read-in preset
#define FNC_PACK        011 // (023) Pack acknowledge
#define FNC_ERASE       012 // (025) Erase
#define FNC_WR_EOF      013 // (027) Write a file (tape) mark
#define FNC_SEARCH      014 // (031) Search
#define FNC_SP_FWD      014 // (031) Space Forward
#define FNC_SP_REV      015 // (033) Space Reverse
#define FNC_CHK_DATA    024 // (051) Write Check Data
#define FNC_CHK_FWD     024 // (051) Write Check Forward
#define FNC_CHK_HDR     025 // (053) Write Check Header/Data
#define FNC_CHK_REV     026 // (057) Write Check Reverse
#define FNC_WR_DATA     030 // (061) Write Data
#define FNC_WR_FWD      030 // (061) Write Forward
#define FNC_WR_HDR      031 // (063) Write Header/Data
#define FNC_RD_DATA     034 // (071) Read Data
#define FNC_RD_FWD      034 // (071) Read Forward
#define FNC_RD_HDR      036 // (075) Read Header/Data
#define FNC_RD_REV      037 // (077) Read Reverse
