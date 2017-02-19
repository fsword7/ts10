// vtape.h - Virtual Tape Support Routines
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

// Virtual Tape Flags
#define VMT_OPENED  0x80000000  // Tape file is Opened.
#define VMT_WRLOCK  0x40000000  // Write-locked (1 = Locked, 0 = Unlocked)
#define VMT_DEBUG   0x20000000  // Debug Information Enable
#define VMT_DUMP    0x10000000  // Dump Information Enable

// Virtual Disk Error Codes
#define VMT_OK       0  // Successful - Normal Operation
#define VMT_IOERROR  1  // I/O Error
#define VMT_OPENERR  2  // Open Error
#define VMT_MEMERR   3  // Memory Error
#define VMT_NONAME   4  // No Filename
#define VMT_NODESC   5  // No Descriptor
#define VMT_NOFMT    6  // No Format
#define VMT_ADRERR   7  // Address Error
#define VMT_WRPROT   8  // Write Protection Violation

// Virtual Tape Format
#define VMT_FMT_UNK  0  // Unknown Format
#define VMT_FMT_REAL 0  // Real Tape Drive
#define VMT_FMT_TPC  1  // Tape Archives
#define VMT_FMT_TPS  2  // Tape data, SIMH format

// Tape Data, SIMH Format
#define MTR_TMK  0x00000000        // Tape Mark
#define MTR_EOM  0xFFFFFFFF        // End of Medium
#define MTR_ERF  0x80000000        // Error Flag
#define MTRF(x)  ((x) & MTR_ERF)   // Record Error Flag
#define MTRL(x)  ((x) & ~MTR_ERF)  // Record Length

// Tape Operation Result Codes.
#define MT_CUR    0  // Middle of Tape (current position)
#define MT_OK     0  // Successful Normal Operation
#define MT_MARK  -1  // Tape Mark
#define MT_BOT   -2  // Bottom of Tape
#define MT_EOM   -3  // End of Medium
#define MT_EOT   -4  // End of Tape
#define MT_CRC   -5  // Bad CRC/Checksum
#define MT_ERROR -6  // I/O Error

typedef struct vmt_Format VMT_FORMAT;
typedef struct vmt_Tape   VMT_TAPE;

struct vmt_Format {
	char *Name;    // Format Name (File extension)
	char *Desc;    // Description
	int  Type;     // Format Type (TPS, TPC, etc.)

	// Function Calls
	int (*Read)(VMT_TAPE *, uint8 *, int32);
	int (*Write)(VMT_TAPE *, uint8 *, int32);
	int (*Mark)(VMT_TAPE *);
	int (*Erase)(VMT_TAPE *);
	int (*Skip)(VMT_TAPE *, int32);
	int (*Rewind)(VMT_TAPE *);
};

struct vmt_Tape {
	char       *fileName; // File Name
	int        dpFile;    // File Descriptor for host operating system.
	char       *devType;  // Device Type like 'TU45', etc.
	char       *fmtName;  // Format Name
	VMT_FORMAT *Format;   // Tape Format
	uint32     Flags;     // Tape Flags
	uint32     mtAddr;    // Current Tape Address in Bytes.
	int        posFlag;   // Position Flag - BOT, EOF, or EOT.
	int        errCode;   // Error Code

};

// External function calls
VMT_FORMAT *vmt_GetFormat(char *fmtName);
int vmt_OpenTape(VMT_TAPE *vmt);
int vmt_CloseTape(VMT_TAPE *vmt);
