// vtape.c - Virtual Tape Support Routines
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
#include "emu/vtape.h"

// Report tape drive error, Position might be lost.
#define MTOK(vmt, pos) \
	((vmt)->posFlag = (((pos) < 0) ? (pos) : MT_CUR), pos)
#define MTERR(vmt, err) \
	((vmt)->errCode = (err), MT_ERROR)

#ifdef DEBUG
static __inline__ int vmt_DispSuccess(register VMT_TAPE *vmt,
	int pos, char *reason)
{
	if (vmt->Flags & VMT_DEBUG)
		dbg_Printf("%s: *** %s ***\n", vmt->Format->Name, reason);
	return MTOK(vmt, pos);
}

static __inline__ int vmt_DispError(register VMT_TAPE *vmt, char *reason)
{
	if (vmt->Flags & VMT_DEBUG)
		dbg_Printf("%s: *** %s(%d) Error: %d (%s) ***\n",
			vmt->Format->Name, reason, vmt->mtAddr,
			errno, strerror(errno));
	return MTERR(vmt, errno);
}
#define MTOK2(vmt, pos, reason) vmt_DispSuccess(vmt, pos, reason)
#define MTERR2(vmt, reason)     vmt_DispError(vmt, reason)
#else  /* DEBUG */
#define MTOK2(vmt, pos, reason) MTOK(vmt, pos)
#define MTERR2(vmt, reason)     MTERR(vmt, errno)
#endif /* DEBUG */

// ********************************************************************

// TPS Format - Tape Data, SIMH Format

int tps_Read(VMT_TAPE *vmt, uint8 *data, int32 blksz)
{
	uint32 blksz32, tblksz32, wc;
	int    rbc, pos;

	vmt->errCode = 0;
	if (blksz < 0) {
		// Read Reverse
		if (vmt->mtAddr == 0)
			return MTOK2(vmt, MT_BOT, "Bottom of Tape");

		pos = vmt->mtAddr - sizeof(blksz32);
		lseek(vmt->dpFile, pos, SEEK_SET);
		rbc = read(vmt->dpFile, &blksz32, sizeof(blksz32));
		if (rbc < sizeof(blksz32)) {
			return MTERR2(vmt, "Read Reverse");
		} else if (blksz32 == MTR_TMK) {
			vmt->mtAddr = pos;
			return MTOK2(vmt, MT_MARK, "Tape Mark");
		} else if (blksz32 == MTR_EOM) {
			vmt->mtAddr = pos;
			return MTOK2(vmt, MT_EOM, "End of Medium");
		}

		pos -= (wc = (blksz32 + 1) & ~1); // Word-aligned length
		lseek(vmt->dpFile, pos, SEEK_SET);
		if ((rbc = read(vmt->dpFile, data, wc)) < 0)
			return MTERR2(vmt, "Read Reverse");
		else if (rbc < wc)
			return MTOK2(vmt, MT_BOT, "Bottom of Tape");
	
		pos -= sizeof(tblksz32);
		lseek(vmt->dpFile, pos, SEEK_SET);
		if ((rbc = read(vmt->dpFile, &tblksz32, sizeof(tblksz32))) < 0)
			return MTERR2(vmt, "Read Reverse");
		else if ((rbc != sizeof(tblksz32)) || (blksz32 != tblksz32)) {
#ifdef DEBUG
			dbg_Printf("%s: Read Reverse(%d): Mismatch block sizes: %d != %d\n",
				vmt->Format->Name, vmt->mtAddr, blksz32, tblksz32);
#endif /* DEBUG */
			return MTOK(vmt, MT_CRC);
		}
	} else {
		// Read Forward
		lseek(vmt->dpFile, vmt->mtAddr, SEEK_SET);
		rbc = read(vmt->dpFile, &blksz32, sizeof(blksz32));
		if (rbc < 0)
			return MTERR2(vmt, "Read Forward");
		else if (rbc < sizeof(blksz32))
			return MTOK2(vmt, MT_EOT, "End of Tape");
		else if (blksz32 == MTR_TMK) {
			vmt->mtAddr += sizeof(blksz32);
			return MTOK2(vmt, MT_MARK, "Tape Mark");
		} else if (blksz32 == MTR_EOM) {
			vmt->mtAddr += sizeof(blksz32);
			return MTOK2(vmt, MT_EOM, "End of Medium");
		}

		wc = (blksz32 + 1) & ~1; // Word-aligned length
		if ((rbc = read(vmt->dpFile, data, wc)) < 0)
			return MTERR2(vmt, "Read Forward");
		else if (rbc < wc)
			return MTOK2(vmt, MT_EOT, "End of Tape");

		if ((rbc = read(vmt->dpFile, &tblksz32, sizeof(tblksz32))) < 0)
			return MTERR2(vmt, "Read Forward");
		else if ((rbc != sizeof(tblksz32)) || (blksz32 != tblksz32)) {
#ifdef DEBUG
			dbg_Printf("%s: Read Forward(%d): Mismatch block sizes: %d != %d\n",
				vmt->Format->Name, vmt->mtAddr, blksz32, tblksz32);
#endif /* DEBUG */
			return MTOK(vmt, MT_CRC);
		}
		pos = vmt->mtAddr + wc + (sizeof(blksz32) * 2);
	}

#ifdef DEBUG
	if (vmt->Flags & VMT_DEBUG)
		dbg_Printf("%s: Read %s(%d): Read %d bytes of data.\n",
			vmt->Format->Name, (blksz < 0) ? "Reverse" : "Forward",
		  	vmt->mtAddr, blksz32);
	if (vmt->Flags & VMT_DUMP)
		PrintDump(0, data, wc);
#endif /* DEBUG */

	// Record is valid now.
	vmt->mtAddr = pos;
	return MTOK(vmt, blksz32);
}

int tps_Write(VMT_TAPE *vmt, uint8 *data, int32 blksz)
{
	int wc, idx, rc;

	vmt->errCode = 0;

	if (vmt->Flags & VMT_WRLOCK) {
		vmt->errCode = EACCES;
		return MT_ERROR;
	}

	if (blksz < 0) {
#ifdef DEBUG
		dbg_Printf("%s: Reverse Not Supported.\n",
			vmt->Format->Name);
#endif /* DEBUG */
		vmt->errCode = -1;
	} else if ((rc = lseek(vmt->dpFile, vmt->mtAddr, SEEK_SET)) >= 0) {
		if ((rc = write(vmt->dpFile, &blksz, sizeof(blksz))) == sizeof(blksz)) {
			wc = (blksz + 1) & ~1; // Word-aligned length
			if (blksz < wc) data[blksz] = '\0';
			if ((rc = write(vmt->dpFile, data, wc)) == wc) {
				rc = write(vmt->dpFile, &blksz, sizeof(blksz));
				if (rc == sizeof(blksz)) {
#ifdef DEBUG
	if (vmt->Flags & VMT_DEBUG)
		dbg_Printf("%s: Write Forward(%d): Write %d bytes of data.\n",
			vmt->Format->Name, vmt->mtAddr, blksz);
	if (vmt->Flags & VMT_DUMP)
		PrintDump(0, data, wc);
#endif /* DEBUG */
					// Record is valid now.
					vmt->mtAddr  += wc + (sizeof(blksz) * 2);
					vmt->posFlag  = MT_CUR;
					vmt->errCode  = 0;
					return blksz;
				}
			}
		}
	} else {
		vmt->errCode = errno;
#ifdef DEBUG
		if (vmt->Flags & VMT_DEBUG)
			dbg_Printf("%s: (W) Seek(%s): %s\n",
				vmt->Format->Name, vmt->mtAddr, strerror(errno));
#endif /* DEBUG */
	}

	// if no space left on device means the end of tape.
	if (vmt->errCode == ENOSPC) {
		vmt->errCode = 0;
		vmt->posFlag = MT_EOT;
		return MT_EOT;
	}

	// Return with I/O error.
	return MT_ERROR;
}

int tps_Mark(VMT_TAPE *vmt)
{
	uint32 tmk = MTR_TMK; // Block length
	int    rc;

	if (vmt->Flags & VMT_WRLOCK) {
		vmt->errCode = EACCES;
		return MT_ERROR;
	}

	if ((rc = write(vmt->dpFile, &tmk, sizeof(tmk))) < 0)
		return MTERR2(vmt, "Tape Mark");
	if (rc < sizeof(tmk))
		return MTOK2(vmt, MT_EOT, "End of Tape");

	// Successful Operation
	vmt->mtAddr += sizeof(tmk);
	return MTOK(vmt, MT_OK);
}

int tps_Erase(VMT_TAPE *vmt)
{
	uint32 eom = MTR_EOM; // Block length
	int    rc;

	if (vmt->Flags & VMT_WRLOCK) {
		vmt->errCode = EACCES;
		return MT_ERROR;
	}

	if ((rc = write(vmt->dpFile, &eom, sizeof(eom))) < 0)
		return MTERR2(vmt, "Erase");
	if (rc < sizeof(eom))
		return MTOK2(vmt, MT_EOT, "End of Tape");

	// Successful Operation
	vmt->mtAddr += sizeof(eom);
	return MTOK(vmt, MT_OK);
}

int tps_Skip(VMT_TAPE *vmt, int32 dir)
{
	uint32 blksz;
	int count = 0;
	int pos, rc;

	vmt->posFlag = MT_CUR;
	if (dir < 0) {
		// Check tape position as BOT or not.
		if (vmt->mtAddr == 0) {
			vmt->posFlag = MT_BOT;
			return vmt->posFlag;
		}
		pos = vmt->mtAddr - sizeof(blksz);
		lseek(vmt->dpFile, pos, SEEK_SET);
		if ((rc = read(vmt->dpFile, &blksz, sizeof(blksz))) < sizeof(blksz)) {
			if (rc < 0) {
				// I/O Error
				vmt->errCode = errno;
				vmt->posFlag = MT_ERROR;
			} else
				// BOT/EOM Encounter
				vmt->posFlag = MT_BOT;
			vmt->mtAddr = 0;
			return vmt->posFlag;
		}
#ifdef DEBUG
		if (vmt->Flags & VMT_DEBUG)
			dbg_Printf("%s: (Backward) Skipping %d (%04X at %08X) bytes of record.\n",
				vmt->Format->Name, blksz, blksz, vmt->mtAddr);
#endif /* DEBUG */
		if (blksz == 0) {
			vmt->mtAddr  = pos;
			vmt->posFlag = MT_MARK;
		} else
			vmt->mtAddr -= (blksz + (sizeof(blksz) * 2));
	} else {
		lseek(vmt->dpFile, vmt->mtAddr, SEEK_SET);
		rc = read(vmt->dpFile, &blksz, sizeof(blksz));
		if (rc == sizeof(blksz)) {
#ifdef DEBUG
			if (vmt->Flags & VMT_DEBUG)
				dbg_Printf("%s: (Forward) Skipping %d (%04X at %08X) bytes of record.\n",
					vmt->Format->Name, blksz, blksz, vmt->mtAddr);
#endif /* DEBUG */
			if (blksz == 0) {
				vmt->mtAddr += sizeof(blksz);
				vmt->posFlag = MT_MARK;
			} else
				vmt->mtAddr += (blksz + (sizeof(blksz) * 2));
		} else if (rc == 0)
			vmt->posFlag = MT_EOT;
	}

	return vmt->posFlag;
}

int tps_Rewind(VMT_TAPE *vmt)
{
	int32 pos;

	if ((pos = lseek(vmt->dpFile, 0, SEEK_SET)) < 0) {
		vmt->errCode = errno;
		return MT_ERROR;
	}

#ifdef DEBUG
	if (vmt->Flags & VMT_DEBUG)
		dbg_Printf("%s: Rewinding.\n", vmt->Format->Name);
#endif /* DEBUG */

	vmt->mtAddr  = pos;
	vmt->posFlag = MT_BOT;

	return MT_OK;
}

VMT_FORMAT vmt_Formats[] =
{
//	{ "tpc", "Tape data with Counts",   VMT_FMT_TPC },
	{ "tap", "Tape data, SIMH Format",  VMT_FMT_TPS,
		tps_Read, tps_Write, tps_Mark, tps_Erase, tps_Skip, tps_Rewind },
	{ NULL }  // Null Terminator
};

// ********************************************************************

VMT_FORMAT *vmt_GetFormat(char *fmtName)
{
	int idx;

	// Looking for disk format by name.
	for (idx = 0; vmt_Formats[idx].Name; idx++)
		if (!strcasecmp(fmtName, vmt_Formats[idx].Name))
			return &vmt_Formats[idx];

	// Format not found - return nothing.
	return NULL;
}

int vmt_OpenTape(VMT_TAPE *vmt)
{
	VMT_FORMAT *fmt;
	int        umode;

	// Check any errors first.
	if (vmt == NULL)
		return VMT_NODESC;
	if (vmt->fileName == NULL)
		return VMT_NONAME;
	if (vmt->fmtName == NULL) {
		char *p = strrchr(vmt->fileName, '.');
		if (p == NULL)
			return VMT_NOFMT;
		vmt->fmtName = p + 1;
	}
	if ((fmt = vmt_GetFormat(vmt->fmtName)) == NULL)
		return VMT_NOFMT;
	vmt->Format = fmt;

	// Attempt to open a tape file.
	umode = (vmt->Flags & VMT_WRLOCK) ? O_RDONLY : O_RDWR|O_CREAT;
	if ((vmt->dpFile = open(vmt->fileName, umode, 0700)) < 0) {
		vmt->errCode = errno;
		return VMT_OPENERR;
	}
	vmt->Flags |= VMT_OPENED;

	return VMT_OK;
}

int vmt_CloseTape(VMT_TAPE *vmt)
{
	if (vmt == NULL)
		return VMT_NODESC;

	close(vmt->dpFile);
	vmt->Flags  &= ~VMT_OPENED;
	vmt->dpFile  = 0;

	return VMT_OK;
}
