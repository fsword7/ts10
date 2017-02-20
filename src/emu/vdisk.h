// vdisk.h - Virtual Disk Support Routines
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

// Virtual Disk Flags
#define VDK_OPENED   0x80000000  // File is opened and accesible.
#define VDK_WRLOCK   0x40000000  // Write-locked (1 = Locked, 0 = Unlocked)
#define VDK_18B      0x00000001  // 18-bit Mode - Use format conversion

// Virtual Disk Error Codes
#define VDK_OK       0  // Successful - Normal Operation
#define VDK_IOERROR  1  // I/O Error
#define VDK_OPENERR  2  // Open Error
#define VDK_MEMERR   3  // Memory Error
#define VDK_NONAME   4  // No Filename
#define VDK_NODESC   5  // No Descriptor
#define VDK_NOFMT    6  // No Format
#define VDK_ADRERR   7  // Address Error
#define VDK_WRPROT   8  // Write Protection Violation

typedef struct vdk_Format VDK_FORMAT;
typedef struct vdk_Disk   VDK_DISK;

struct vdk_Format {
	char   *Name;    // Format Name
	char   *Desc;    // Description
	uint32 Flags;    // Format Flags
	int    nBytes;   // Number of Bytes for format.
	int    nWords;   // Number of 18-bit Words.

	// Conversion Function Calls
	void (*To)(uint18 *, int, uint8 *);    // Convert to disk format
	void (*From)(uint8 *, uint18 *, int);  // Convert from disk format
};

struct vdk_Disk {
	char       *fileName; // File Name
	char       *fmtName;  // Format Name
	char       *devType;  // Device Type like 'RP06', etc.

	VDK_FORMAT *Format;   // Disk Format
	uint32     Flags;     // Disk Flags
	int        dpFile;    // File Descriptor for host operating system.
	uint32     dskAddr;   // Current Disk Address in Blocks.
	int        errCode;   // Error Code (errno)

	// Disk Geometries
	uint32     Cylinders; // Number of Cylinders
	uint32     Tracks;    // Number of Tracks
	uint32     Sectors;   // Number of Sectors
	uint32     Blocks;    // Total Blocks
	uint32     vsBlock;   // Virtual Block Size
	uint32     szBlock;   // Physical Block Size
	uint32     szImage;   // Image Size in Bytes
};

// External function calls.
int vdk_OpenDisk(VDK_DISK *);
int vdk_CloseDisk(VDK_DISK *);
int vdk_SeekDisk(VDK_DISK *, uint32);
int vdk_ReadDisk(VDK_DISK *, uint8 *, uint32);
int vdk_WriteDisk(VDK_DISK *, uint8 *, uint32);
uint32 vdk_GetDiskAddr(VDK_DISK *, uint32, uint32, uint32);
