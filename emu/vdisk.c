// vdisk.c - Virtual Disk Support Routines
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

#include "emu/defs.h"
#include "emu/vdisk.h"

// ********************************************************************

// Disk Big-Endian Double Format (2 36-bit words or 4 18-bit words)

void dbd9_To(register uint18 *wds, register int szWords, register uint8 *fmt)
{
	register int idx;

	for (idx = 0; idx < szWords; idx += 4) {
		*fmt++ = wds[idx] >> 10;
		*fmt++ = wds[idx] >> 2;
		*fmt++ = (wds[idx] << 6) | (wds[idx+1] >> 12);
		*fmt++ = wds[idx+1] >> 4;
		*fmt++ = (wds[idx+1] << 4) | (wds[idx+2] >> 14);
		*fmt++ = wds[idx+2] >> 6;
		*fmt++ = (wds[idx+2] << 2) | (wds[idx+3] >> 16);
		*fmt++ = wds[idx+3] >> 8;
		*fmt++ = wds[idx+3];
	}
}

void dbd9_From(register uint8 *fmt, register uint18 *wds, register int szWords)
{
	register int idx;

	for (idx = 0; szWords > 0; idx += 9, szWords -= 4) {
		*wds++ = (fmt[idx] << 10)   | (fmt[idx+1] << 2) | (fmt[idx+2] >> 6);
		*wds++ = (fmt[idx+2] << 12) | (fmt[idx+3] << 4) | (fmt[idx+4] >> 4);
		*wds++ = (fmt[idx+4] << 14) | (fmt[idx+5] << 6) | (fmt[idx+6] >> 2);
		*wds++ = (fmt[idx+6] << 16) | (fmt[idx+7] << 8) | fmt[idx+8];
	}
}

// Disk Big-Endian Single Format (5-bytes per 36-bit word)

void dbs5_To(register uint18 *wds, register int szWords, register uint8 *fmt)
{
	register int idx;

	for (idx = 0; idx < szWords; idx += 2) {
		*fmt++ = wds[idx] >> 10;
		*fmt++ = wds[idx] >> 2;
		*fmt++ = (wds[idx] << 6) | (wds[idx+1] >> 12);
		*fmt++ = wds[idx+1] >> 4;
		*fmt++ = wds[idx+1] & 0xF;
	}
}

void dbs5_From(register uint8 *fmt, register uint18 *wds, register int szWords)
{
	register int idx;

	for (idx = 0; szWords > 0; idx += 5, szWords -= 2) {
		*wds++ = (fmt[idx] << 10)   | (fmt[idx+1] << 2) | (fmt[idx+2] >> 6);
		*wds++ = (fmt[idx+2] << 12) | (fmt[idx+3] << 4) | (fmt[idx+4] & 0xF);
	}
}

VDK_FORMAT vdk_Formats[] =
{
	{ "dsk",  "Disk, No Conversion",     0, 2, 1, NULL,     NULL       },
	{ "dbd9", "Disk Big-Endian Double",  0, 9, 4, dbd9_To,  dbd9_From  },
	{ "dbs5", "Disk Big-Endian Single",  0, 5, 2, dbs5_To,  dbs5_From  },
	{ NULL }  // Null Terminator
};

// ********************************************************************

VDK_FORMAT *vdk_GetFormat(char *fmtName)
{
	int idx;

	// Looking for disk format by name.
	for (idx = 0; vdk_Formats[idx].Name; idx++)
		if (!strcasecmp(fmtName, vdk_Formats[idx].Name))
			return &vdk_Formats[idx];

	// Format not found - return nothing.
	return NULL;
}

int vdk_OpenDisk(VDK_DISK *vdk)
{
	VDK_FORMAT *fmt;
	int        umode;

	// Check any errors first.
	if (vdk == NULL)
		return VDK_NODESC;
	if (vdk->fileName == NULL)
		return VDK_NONAME;
	if (vdk->fmtName == NULL) {
		char *p = strrchr(vdk->fileName, '.');
		if (p == NULL)
			return VDK_NOFMT;
		vdk->fmtName = p + 1;
	}

	// Set up format conversion for 18-bit words if request.
	fmt = vdk_GetFormat(vdk->fmtName);
	if (fmt && fmt->To && fmt->From) {
		vdk->Format  = fmt;
		vdk->szBlock = (vdk->vsBlock / (fmt->nWords * 2)) * fmt->nBytes;
	} else {
//		vdk->Format  = &vdk_Formats[0]; // Default - No Conversion
		vdk->szBlock = vdk->vsBlock;
	}
	vdk->Blocks  = vdk->Cylinders * vdk->Tracks * vdk->Sectors;
	vdk->szImage = vdk->Blocks * vdk->szBlock;

	// Attempt to open a disk file.
	umode = (vdk->Flags & VDK_WRLOCK) ? O_RDONLY : O_RDWR|O_CREAT;
	if ((vdk->dpFile = open(vdk->fileName, umode, 0700)) < 0) {
		vdk->errCode = errno;
		return VDK_OPENERR;
	}
	vdk->Flags |= VDK_OPENED;
	
	return VDK_OK;
}

int vdk_CloseDisk(VDK_DISK *vdk)
{
	if (vdk == NULL)
		return VDK_NODESC;

	close(vdk->dpFile);
	vdk->Flags  &= ~VDK_OPENED;
	vdk->dpFile  = 0;

	return VDK_OK;
}

int vdk_SeekDisk(VDK_DISK *vdk, uint32 dskAddr)
{
	int32 pos;

	if (vdk == NULL)
		return VDK_NODESC;
	if (dskAddr >= vdk->Blocks)
		return VDK_ADRERR;

	if ((pos = lseek(vdk->dpFile, dskAddr * vdk->szBlock, SEEK_SET)) < 0) {
		vdk->errCode = errno;
		return VDK_IOERROR;
	} else {
		vdk->dskAddr = pos / vdk->szBlock;
	}

	return VDK_OK;
}

int vdk_ReadDisk(VDK_DISK *vdk, uint8 *data, uint32 mode)
{
	int rc;

	if (vdk == NULL)
		return VDK_NODESC;
	if (vdk->Flags & (mode & VDK_18B)) {
		uint8 fmt[vdk->szBlock];
		VDK_FORMAT *cvt;

		if ((rc = read(vdk->dpFile, fmt, vdk->szBlock)) == vdk->szBlock) {
			vdk->dskAddr++;
			if (cvt = vdk->Format)
				cvt->From(fmt, (uint18 *)data, 256);
		}
	} else {
		rc = read(vdk->dpFile, data, 512);
	}

	if (rc < 0) {
		vdk->errCode = errno;
		return VDK_IOERROR;
	}

	return VDK_OK;
}

int vdk_WriteDisk(VDK_DISK *vdk, uint8 *data, uint32 mode)
{
	int rc;

	if (vdk == NULL)
		return VDK_NODESC;
	if (vdk->Flags & VDK_WRLOCK)
		return VDK_WRPROT;

	if (vdk->Flags & (mode & VDK_18B)) {
		uint8 fmt[vdk->szBlock];
		VDK_FORMAT *cvt;

		if (cvt = vdk->Format)
			cvt->To((uint18 *)data, 256, fmt);
		if ((rc = write(vdk->dpFile, fmt, vdk->szBlock)) == vdk->szBlock)
			vdk->dskAddr++;
	} else {
		rc = write(vdk->dpFile, data, 512);
	}

	if (rc < 0) {
		vdk->errCode = errno;
		return VDK_IOERROR;
	}

	return VDK_OK;
}

uint32 vdk_GetDiskAddr(VDK_DISK *vdk,
	uint32 Cylinder, uint32 Track, uint32 Sector)
{
	return (((Cylinder * vdk->Tracks) + Track) * vdk->Sectors) + Sector;
}
