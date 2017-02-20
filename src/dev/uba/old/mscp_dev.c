// mscp_dev.c - MSCP Device Types Table
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

#include "emu/defs.h"
#include "dev/mscp.h"

MSCP_DTYPE mscp_dtList[] =
{
	{
		"RD51",                     // Device Type
		"11MB Hard Disk Drive",     // Description
		MDT_DISK|MDF_FIXED,         // Device Flags
		0x25644033,                 // Media ID
		2,                          // Sector Interleave
		512,                        // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		18,                         // Blocks per Track
		1,                          // Tracks per Group (Surface)
		4,                          // Groups per Cylinder
		306,                        // Cylinders per Unit
		22032,                      // Total LBNs per Unit
		21600,                      // User LBNs per Unit
		144,                        // Number of Replacement Blocks (RBN)
		87,                         // Number of Diagnostics Blocks (DBN)
		57,                         // Number of Transfer Blocks    (XBN)
		36,                         // RCT Size (Blocks)
		4,                          // RCT Copies
	},

	{
		"RD52",                     // Device Type
		"70MB Hard Disk Drive",     // Description
		MDT_DISK|MDF_FIXED,         // Device Flags
		0x25644034,                 // Media ID
		1,                          // Sector Interleave
		512,                        // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		17,                         // Blocks per Track
		8,                          // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		512,                        // Cylinders per Unit
		69632,                      // Total LBNs per Unit
		60480,                      // User LBNs per Unit
		168,                        // Number of Replacement Blocks (RBN)
		82,                         // Number of Diagnostics Blocks (DBN)
		54,                         // Number of Transfer Blocks    (XBN)
		4,                          // RCT Size (Blocks)
		8,                          // RCT Copies
	},

	{
		"RD52",                     // Device Type
		"70MB Hard Disk Drive",     // Description
		MDT_DISK|MDF_FIXED,         // Device Flags
		0x25644034,                 // Media ID
		1,                          // Sector Interleave
		512,                        // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		17,                         // Blocks per Track
		7,                          // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		645,                        // Cylinders per Unit
		76755,                      // Total LBNs per Unit
		60480,                      // User LBNs per Unit
		168,                        // Number of Replacement Blocks (RBN)
		65,                         // Number of Diagnostics Blocks (DBN)
		54,                         // Number of Transfer Blocks    (XBN)
		4,                          // RCT Size (Blocks)
		8,                          // RCT Copies
	},

	{
		"RD53",                     // Device Type
		"70MB Hard Disk Drive",     // Description
		MDT_DISK|MDF_FIXED,         // Device Flags
		0x25644035,                 // Media ID
		1,                          // Sector Interleave
		512,                        // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		17,                         // Blocks per Track
		8,                          // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		1024,                       // Cylinders per Unit
		139264,                     // Total LBNs per Unit
		138672,                     // User LBNs per Unit
		280,                        // Number of Replacement Blocks (RBN)
		82,                         // Number of Diagnostics Blocks (DBN)
		54,                         // Number of Transfer Blocks    (XBN)
		5,                          // RCT Size (Blocks)
		8,                          // RCT Copies
	},

	{
		"RRD40",                    // Device Type
		"DEC RRD40 CD-ROM Drive",   // Description
		MDT_CD|MDF_RMVBL|MDF_RDONLY, // Device Flags
		0x25652228,                 // Media ID
		1,                          // Sector Interleave
		2048,                       // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		32,                         // Blocks per Track
		8,                          // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		1024,                       // Cylinders per Unit
		332800,                     // Total LBNs per Unit
		332800,                     // User LBNs per Unit
		0,                          // Number of Replacement Blocks (RBN)
		0,                          // Number of Diagnostics Blocks (DBN)
		0,                          // Number of Transfer Blocks    (XBN)
		0,                          // RCT Size (Blocks)
		0,                          // RCT Copies
	},

	{
		"RRD50",                    // Device Type
		"DEC RRD50 CD-ROM Drive",   // Description
		MDT_CD|MDF_RMVBL|MDF_RDONLY, // Device Flags
		0x25652228,                 // Media ID
		1,                          // Sector Interleave
		2048,                       // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		32,                         // Blocks per Track
		8,                          // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		1024,                       // Cylinders per Unit
		332800,                     // Total LBNs per Unit
		332800,                     // User LBNs per Unit
		0,                          // Number of Replacement Blocks (RBN)
		0,                          // Number of Diagnostics Blocks (DBN)
		0,                          // Number of Transfer Blocks    (XBN)
		0,                          // RCT Size (Blocks)
		0,                          // RCT Copies
	},

	{
		"RX01",                     // Device Type
		"250KB RX11 Floppy Drive",  // Description
		MDT_FLOPPY|MDF_RMVBL,       // Device Flags
		0x25658001,                 // Media ID
		1,                          // Sector Interleave
		128,                        // Bytes per Sector (Sector Size)
		4,                          // Sectors per LBN (Logical Block)
		26,                         // Blocks per Track
		77,                         // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		1,                          // Cylinders per Unit
		2002,                       // Total LBNs per Unit
		2002,                       // User LBNs per Unit
		0,                          // Number of Replacement Blocks (RBN)
		0,                          // Number of Diagnostics Blocks (DBN)
		0,                          // Number of Transfer Blocks    (XBN)
		0,                          // RCT Size (Blocks)
		0,                          // RCT Copies
	},

	{
		"RX02",                     // Device Type
		"500KB RX21 Floppy Drive",  // Description
		MDT_FLOPPY|MDF_RMVBL,       // Device Flags
		0x25658002,                 // Media ID
		1,                          // Sector Interleave
		256,                        // Bytes per Sector (Sector Size)
		2,                          // Sectors per LBN (Logical Block)
		26,                         // Blocks per Track
		77,                         // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		1,                          // Cylinders per Unit
		2002,                       // Total LBNs per Unit
		2002,                       // User LBNs per Unit
		0,                          // Number of Replacement Blocks (RBN)
		0,                          // Number of Diagnostics Blocks (DBN)
		0,                          // Number of Transfer Blocks    (XBN)
		0,                          // RCT Size (Blocks)
		0,                          // RCT Copies
	},

	{
		"RX23",                     // Device Type
		"1.4MB 3.5 Floppy Drive",   // Description
		MDT_FLOPPY|MDF_RMVBL,       // Device Flags
		0x25658017,                 // Media ID
		1,                          // Sector Interleave
		512,                        // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		18,                         // Blocks per Track
		160,                        // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		1,                          // Cylinders per Unit
		2880,                       // Total LBNs per Unit
		2880,                       // User LBNs per Unit
		0,                          // Number of Replacement Blocks (RBN)
		0,                          // Number of Diagnostics Blocks (DBN)
		0,                          // Number of Transfer Blocks    (XBN)
		0,                          // RCT Size (Blocks)
		0,                          // RCT Copies
	},

	{
		"RX26",                     // Device Type
		"2.8MB 3.5 Floppy Drive",   // Description
		MDT_FLOPPY|MDF_RMVBL,       // Device Flags
		0x2565801A,                 // Media ID
		1,                          // Sector Interleave
		512,                        // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		36,                         // Blocks per Track
		160,                        // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		1,                          // Cylinders per Unit
		5760,                       // Total LBNs per Unit
		5760,                       // User LBNs per Unit
		0,                          // Number of Replacement Blocks (RBN)
		0,                          // Number of Diagnostics Blocks (DBN)
		0,                          // Number of Transfer Blocks    (XBN)
		0,                          // RCT Size (Blocks)
		0,                          // RCT Copies
	},

	{
		"RX33",                     // Device Type
		"1.2MB 5.25 Floppy Drive",  // Description
		MDT_FLOPPY|MDF_RMVBL,       // Device Flags
		0x25658021,                 // Media ID
		1,                          // Sector Interleave
		512,                        // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		15,                         // Blocks per Track
		160,                        // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		1,                          // Cylinders per Unit
		2400,                       // Total LBNs per Unit
		2400,                       // User LBNs per Unit
		0,                          // Number of Replacement Blocks (RBN)
		0,                          // Number of Diagnostics Blocks (DBN)
		0,                          // Number of Transfer Blocks    (XBN)
		0,                          // RCT Size (Blocks)
		0,                          // RCT Copies
	},

	{
		"RX50",                     // Device Type
		"400KB 5.25 Floppy Drive",  // Description
		MDT_FLOPPY|MDF_RMVBL,       // Device Flags
		0x25658032,                 // Media ID
		2,                          // Sector Interleave
		512,                        // Bytes per Sector (Sector Size)
		1,                          // Sectors per LBN (Logical Block)
		10,                         // Blocks per Track
		80,                         // Tracks per Group (Surface)
		1,                          // Groups per Cylinder
		1,                          // Cylinders per Unit
		800,                        // Total LBNs per Unit
		800,                        // User LBNs per Unit
		0,                          // Number of Replacement Blocks (RBN)
		0,                          // Number of Diagnostics Blocks (DBN)
		0,                          // Number of Transfer Blocks    (XBN)
		0,                          // RCT Size (Blocks)
		0,                          // RCT Copies
	},

	{ NULL }, // Terminator
};
