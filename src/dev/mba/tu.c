// tm.c - MASSBUS TM02/TM03 Tape Drive Emulation
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
#include "dev/mba/mba.h"
#include "dev/mba/tu.h"

uint8 tu_cBuffer[TM_XFER];

#ifdef DEBUG

char *tu_fncNames[] =
{
	"No Operation",          // 00 No operation
	"Unload",                // 01 Rewind, offline
	"Unknown",               // 02
	"Rewind",                // 03 Rewind
	"Drive Clear",           // 04 Drive Clear
	"Unknown",               // 05
	"Unknown",               // 06
	"Unknown",               // 07
	"Read-In Preset",        // 10 Read-In Preset
	"Unknown",               // 11
	"Erase",                 // 12 Erase
	"Write Tape Mark",       // 13 Write File Mark
	"Space Forward",         // 14 Space Forward
	"Space Reverse",         // 15 Space Reverse
	"Unknown",               // 16
	"Unknown",               // 17
	"Unknown",               // 20
	"Unknown",               // 21
	"Unknown",               // 22
	"Unknown",               // 23
	"Verify Forward",        // 24 Write Check Forward
	"Unknown",               // 25
	"Verify Reverse",        // 26 Write Check Reverse
	"Unknown",               // 27
	"Write Forward",         // 30 Write Forward
	"Unknown",               // 31
	"Unknown",               // 32
	"Unknown",               // 33
	"Read Forward",          // 34 Read Forward
	"Unknown",               // 35
	"Unknown",               // 36
	"Read Reverse"           // 37 Read Reverse
};

static const int nregNames = 10;
static cchar *regNames[] =
{
	"MTCS1",     // (R/W) Control and Status Register #1
	"MTDS",      // (R)   Drive Status Register
	"MTER",      // (R)   Error Register
	"MTMR",      // (R/W) Maintenance Register
	"MTAS",      // (R/W) Attention Summary Register
	"MTFC",      // (R/W) Frame Count Register
	"MTDT",      // (R)   Drive Type Register
	"MTCC",      // (R/W) Character Check Register
	"MTSN",      // (R)   Serial Number Register
	"MTTC",      // (R/W) Tape Control Register
};
#endif /* DEBUG */

// List of slave tape drives for TM02/03 tape controller
MBA_DTYPE tu_dTypes[] = {
	{
		"TU45",        // Device Name
		"TU45 - 800/1600 BPI Tape Drive",
		UNIT_REMOVABLE|UNIT_ATTABLE|UNIT_DISABLE,
		0, 0, 0, 0,    // Not Used
		000012,        // Device Type: TU45
		NULL,          // Not Used

		MBA_TAPE,      // RH11 Flags
		NULL,          // Not Used
	},

	{ NULL }
};

// List of tape controllers
MBA_DTYPE tm_dTypes[] = {
	{
		"TM02",        // Device Name
		"TM02 - Tape Controller",
		UNIT_DISABLE,
		0, 0, 0, 0,    // Not Used
		000010,        // Default Device Type
		tu_dTypes,     // Listing of slave tape drives

		MBA_CTLR,      // RH11 Flags - Controller
		NULL,          // Not used
	},

	{
		"TM03",        // Device Name
		"TM03 - Tape Controller",
		UNIT_DISABLE,
		0, 0, 0, 0,    // Not Used
		000050,        // Default Device Type
		tu_dTypes,     // Listing of slave tape drives

		MBA_CTLR,      // RH11 Flags - Controller
		NULL,          // Not Used
	},

	{ NULL }
};


MBA_DEVINFO tu_DriveInfo;
void tu_Service(void *);
void tu_InitDrive(MBA_DRIVE *mbaDrive)
{
	MBA_CALLBACK *mbaCall;
	TM_DEVICE    *tm;
	TU_DRIVE     *tu;
	UQ_BOOT      *bt;
	CLK_QUEUE    *newTimer;
	int          idx;

	mbaDrive->devInfo = (void *)&tu_DriveInfo;

	// Set new timer
	newTimer = (CLK_QUEUE *)calloc(1, sizeof(CLK_QUEUE));
	newTimer->Next      = NULL;
	newTimer->Flags     = 0;
	newTimer->outTimer  = 10;
	newTimer->nxtTimer  = 10;
	newTimer->Device    = mbaDrive;
	newTimer->Execute   = tu_Service;
	mbaDrive->Timer     = newTimer;

	// Initialize drive registers
	MTCS1 = MTCS1_DVA;         // DVA - Always On
	MTDS  = MTDS_DPR|MTDS_DRY; // DPR - Always On
	MTER  = 0;
	MTTC  = 0;

	if (tm = (TM_DEVICE *)calloc(1, sizeof(TM_DEVICE))) {
		tm->mbaDrive = mbaDrive;
		tm->idUnit   = mbaDrive->idDrive;
		mbaCall      = mbaDrive->Callback;

		// Initialize each slave drives
		for (idx = 0; idx < TM_MAXSLAVES; idx++) {
			tu            = &tm->Slaves[idx];
			tu->mbaDrive  = mbaDrive;
			tu->fmtDevice = tm;
			tu->Flags     = 0;
			tu->idUnit    = idx;
			tu->devType   = MTDT_NSA | MTDT_TAP | TM_TM03 | TM_TU00;
			tu->Serial    = ((tm->idUnit + 1) << 3) | idx;

			if (mbaCall->SetBoot) {
				// Initialize boot device.
				bt          = &tu->Boot;
				bt->Flags   = BT_TAPE|BT_SUPPORTED;
				bt->idDrive = (idx << 8) | mbaDrive->idDrive;
				mbaCall->SetBoot(mbaDrive->pDevice, bt);
			}
		}

		// Temporary: Configure slave drive #0 (at this time)
		tu = tm->dsUnit = &tm->Slaves[0];
		tu->Flags |= MBA_PRESENT;
		tu->devType =
			(MTDT_NSA|MTDT_TAP|MTDT_SPR|TM_TM03) | tu_dTypes[0].dType;
		
		// Initialize selected transport as default slave #0
		MTDS &= ~MTDS_SLAVE;
		MTDS |= tu->Status;
		MTDT  = tu->devType;
		MTSN  = tu->Serial;

		// Finally, link TU device to MBA drive's file reference.
		mbaDrive->FileRef = tm;
	}
}

void tu_ResetDrive(MBA_DRIVE *mbaDrive)
{
	TM_DEVICE *tm = (TM_DEVICE *)mbaDrive->FileRef;
	TU_DRIVE  *tu = tm->dsUnit;
	int idx;

	ts10_CancelTimer(mbaDrive->Timer);

	// Acknowledge the slave attention - reset it.
	tu->Status &= ~MTDS_SSC;

	// Reset others
	MTCS1 = MTCS1_DVA; // DVA - Always On
	MTDS  &= ~(MTDS_ATA|MTDS_SLAVE|MTDS_ERR|MTDS_SSC|MTDS_SLA);
	MTDS  |= tu->Status;
	MTER  = 0;
	MTTC  = 0;

	// Check any slave drive that still require slave attention
	for (idx = 0; idx < TM_MAXSLAVES; idx++)
		if (tm->Slaves[idx].Status & MTDS_SSC)
			MTDS |= MTDS_SSC;
}

#if 0
int tu_CreateSlave(UNIT *tmUnit, int32 slave, int argc, char **argv)
{
	MBA_DTYPE  *dTypes = tmUnit->dType->sTypes;
	UNIT   *sUnit  = &tmUnit->sUnits[slave];
	MTUNIT *sData  = (MTUNIT *)tmUnit->uData;
	int    idx, st;

	if (!(sUnit->Flags & UNIT_PRESENT)) {
		if (sUnit->Flags & UNIT_DISABLED)
			return EMU_DISABLED;

		if (!strcasecmp(argv[0], "off")) {
			sUnit->dType = NULL;
			sUnit->Flags = tmUnit->dType->Flags;
			sData->dTypes[slave] =
				(MTDT_NSA|MTDT_TAP) | tmUnit->dType->dType;
			return EMU_OK;
		}

		for (idx = 0; dTypes[idx].Name; idx++) {
			if (!strcasecmp(argv[0], dTypes[idx].Name))
				break;
		}

		if (dTypes[idx].Name) {
			*strchr(argv[1], ':') = '\0';
			if (st = unit_mapCreateDevice(argv[1], sUnit))
				return st;

			// This unit is a tape drive.
			sUnit->tFlags = UT_STORAGE;
			sUnit->dType  = &dTypes[idx];
			sUnit->Flags  = dTypes[idx].Flags | UNIT_PRESENT;
			sData->dTypes[slave] = (MTDT_NSA|MTDT_TAP|MTDT_SPR) |
				dTypes[idx].dType | (tmUnit->dType->dType & TM_TM03);

			ToUpper(argv[1]);
			printf("Device %s: had been created.\n", argv[1]);

			return EMU_OK;
		}
		return EMU_ARG;
	}
	return EMU_PRESENT;
}
#endif

inline void tu_SetAttention(MBA_DRIVE *mbaDrive)
{
	MTDS |= MTDS_ATA;
	mbaDrive->Callback->SetAttention(mbaDrive->pDevice);
}

int tu_CheckAttention(MBA_DRIVE *mbaDrive)
{
	return (MTDS & MTDS_ATA) ? 1 : 0;
}

void tu_ClearAttention(MBA_DRIVE *mbaDrive)
{
	MTDS &= ~MTDS_ATA;
}

inline void tu_SetReady(MBA_DRIVE *mbaDrive)
{
	MTDS |= MTDS_DRY;
	mbaDrive->Callback->SetAttention(mbaDrive->pDevice);
}

int tu_Attach(MBA_DRIVE *mbaDrive, char *fileName)
{
	TM_DEVICE *tm  = (TM_DEVICE *)mbaDrive->FileRef;
	TU_DRIVE  *tu  = tm->dsUnit;
	VMT_TAPE  *vmt = &tu->dpTape;
	int       st;

	if (tu->Flags & MBA_ATTACHED)
		return EMU_ATTACHED;

	vmt->fileName = (char *)malloc(strlen(fileName)+1);
	strcpy(vmt->fileName, fileName);

	// Attach a file to that unit
	if (st = vmt_OpenTape(vmt)) {
		printf("Reason: %d\n", st);
		return st;
	}

	tu->Flags |= MBA_ATTACHED;

	// Enable boot device.
	if (tu->Boot.Flags & BT_SUPPORTED) {
		tu->Boot.ioDevice  = vmt;
		tu->Boot.Flags    |= BT_VALID;
	}

	// Now let's set some registers for medium on-line
	tu->Status = (MTDS_MOL|MTDS_SSC|MTDS_BOT);
	if (tm->dsUnit && (tm->dsUnit == tu))
		MTDS = (MTDS & ~MTDS_SLAVE) | tu->Status | MTDS_SLA;
	MTDS |= MTDS_SSC;
	tu_SetAttention(mbaDrive);

	return EMU_OK;
}

int tu_Detach(MBA_DRIVE *mbaDrive)
{
	TM_DEVICE *tm  = (TM_DEVICE *)mbaDrive->FileRef;
	TU_DRIVE  *tu  = tm->dsUnit;
	VMT_TAPE  *vmt = &tu->dpTape;
	int       rc;

	if ((tu->Flags & MBA_ATTACHED) == 0)
		return EMU_ARG;

	// Disable boot device
	if (tu->Boot.Flags & BT_SUPPORTED) {
		tu->Boot.Flags    &= ~BT_VALID;
		tu->Boot.ioDevice  = NULL;
	}

	// Detach a file from that unit.
	if (rc = vmt_CloseTape(vmt))
		return rc;
	if (vmt->fileName) {
		free(vmt->fileName);
		vmt->fileName = NULL;
	}

	tu->Flags &= ~(MBA_ATTACHED|MBA_WRLOCKED);

	// Now let's set some registers for medium off-line

	tu->Status = MTDS_SSC;
//		tmData->mtds &= ~MTDS_SLAVE;
	MTDS |= MTDS_SSC;
	tu_SetAttention(mbaDrive);

	return EMU_OK;
}

inline void tu_SetStatus(TU_DRIVE *tu, int16 data)
{
	MBA_DRIVE *mbaDrive = tu->mbaDrive;

	tu->Status |= data;
	if (tu->fmtDevice->dsUnit == tu)
		MTDS = (MTDS & ~MTDS_SLAVE) | tu->Status;
}

inline void tu_ClearStatus(TU_DRIVE *tu, int16 data)
{
	MBA_DRIVE *mbaDrive = tu->mbaDrive;

	tu->Status &= ~data;
	if (tu->fmtDevice->dsUnit == tu)
		MTDS = (MTDS & ~MTDS_SLAVE) | tu->Status;
}

inline int16 tu_CheckStatus(TU_DRIVE *tu, int16 data)
{
	return tu->Status & data;
}

int tu_ReadData(TU_DRIVE *tu, int fc)
{
	MBA_DRIVE *mbaDrive = tu->mbaDrive;
	VMT_TAPE  *vmt      = &tu->dpTape;
	int       fmt       = (MTTC & MTTC_FMTSEL) >> 4;
	uint32    blkData[TM_XFER/2];
	uint8     *blk;
	int       idx;

	switch (fmt) {
		case FMT10_COREDUMP:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: PDP-10 Format - Coredump\n", tu->Unit.devName);
			dbg_Printf("READ: %d bytes being read from memory.\n", fc);
#endif /* DEBUG */

			mbaDrive->Callback->ReadBlock(mbaDrive->pDevice,
				(uint8 *)blkData, (fc / 5) * 4, MBA_18B);

			blk = tu_cBuffer;
			for (idx = 0; idx < fc; idx += 5) {
				*blk++ = blkData[idx] >> 10;
				*blk++ = blkData[idx] >> 2;
				*blk++ = (blkData[idx] << 6) | (blkData[idx+1] >> 12);
				*blk++ = blkData[idx+1] >> 4;
				*blk++ = blkData[idx+1] & 0xF;
			}
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: Format not supported - code %02o\n",
					tu->Unit.devName, fmt);
#endif /* DEBUG */
			break;
	}

	return idx;
}

void tu_WriteData(TU_DRIVE *tu, int fc)
{
	MBA_DRIVE *mbaDrive = tu->mbaDrive;
	VMT_TAPE  *vmt      = &tu->dpTape;
	int       fmt       = (MTTC & MTTC_FMTSEL) >> 4;
	uint32    blkData[TM_XFER/2], *wds;
	int       idx;

	switch (fmt) {
		case FMT10_COREDUMP:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: PDP-10 Format - Coredump\n", tu->Unit.devName);
#endif /* DEBUG */

			wds = blkData;
			for (idx = 0; idx < fc; idx += 5) {
				*wds++ = (tu_cBuffer[idx] << 10)   | (tu_cBuffer[idx+1] << 2) |
					(tu_cBuffer[idx+2] >> 6);
				*wds++ = (tu_cBuffer[idx+2] << 12) | (tu_cBuffer[idx+3] << 4) |
					(tu_cBuffer[idx+4] & 0xF);
			}
			mbaDrive->Callback->WriteBlock(mbaDrive->pDevice,
				(uint8 *)blkData, (fc / 5) * 4, MBA_18B);
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: Format not supported - code %02o\n",
					tu->Unit.devName, fmt);
#endif /* DEBUG */
			break;
	}
}

void tu_ReadIO(MBA_DRIVE *mbaDrive, int reg, uint16 *data)
{

	switch (reg) {
		case nMTCS1:
			*data = MTCS1;
			break;

		case nMTFC:
			*data = MTFC;
			break;

		case nMTDS:
			*data = MTDS;
			break;

		case nMTAS:
			{
				MBA_DEVICE *mba = mbaDrive->wDevice;
				int idx, ata = 0;

				for (idx = 0; idx < MBA_MAXDRIVES; idx++) {
					if (mba->Drive[idx].Regs[nMTDS] & MTDS_ATA)
						ata |= (1 << idx);
				}
				MTAS = ata;
			}
			*data = MTAS;
			break;

		case nMTER:
			*data = MTER;
			break;

		case nMTCC:
			*data = MTCC;
			break;

		case nMTMR:
			*data = MTMR;
			break;

		case nMTDT:
			*data = MTDT;
			break;

		case nMTSN:
			*data = MTSN;
			break;

		case nMTTC:
			*data = MTTC;
			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (R) Unknown MASSBUS Register %o\n",
					mbaDrive->Unit.devName, reg);
#endif /* DEBUG */
			MTER |= MTER_ILR;
			return /* EMU_ILR */;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		if ((reg >> 1) == nMTFC) // Frame Count Register
			dbg_Printf("%s: %s (%02o) => %06o (%d Bytes)\n",
				mbaDrive->Unit.devName, regNames[reg], reg, *data,
				-((*data & 0x8000) ? *data | 0xFFFF0000 : *data));
		else
			dbg_Printf("%s: %s (%02o) => %06o\n",
				mbaDrive->Unit.devName, regNames[reg], reg, *data);
	}
#endif /* DEBUG */
	*data &= 0xFFFF;

/*	return EMU_OK; */
}

void tu_Execute(MBA_DRIVE *);
void tu_WriteIO(MBA_DRIVE *mbaDrive, int reg, uint16 data)
{
	TM_DEVICE *tm = (TM_DEVICE *)mbaDrive->FileRef;
	TU_DRIVE  *tu = tm->dsUnit;
	int    newSlave;
	int    st = EMU_OK;
	int    idx;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		if ((reg >> 1) == nMTFC) // Frame Count Register
			dbg_Printf("%s: %s (%02o) <= %06o (%d Bytes)\n",
				mbaDrive->Unit.devName, regNames[reg], reg, data,
				-((data & 0x8000) ? data | 0xFFFF0000 : data));
		else
			dbg_Printf("%s: %s (%02o) <= %06o\n",
				mbaDrive->Unit.devName, regNames[reg], reg, data);
	}
#endif /* DEBUG */

	switch (reg) {
		case nMTCS1:
			MTCS1 = (MTCS1 & ~MTCS1_WR) | (data & MTCS1_WR);
			if (MTCS1 & MTCS1_GO) {
				MTDS &= ~MTDS_ATA;
				tu_Execute(mbaDrive);
			}
			break;

		case nMTFC:
			if ((MTCS1 & MTCS1_GO) == 0) {
				MTFC = data;
				MTTC |= MTTC_FCS;
			}
			break;

		case nMTAS:
			{
				MBA_DEVICE *mba = mbaDrive->wDevice;
				int idx;

				for (idx = 0; idx < MBA_MAXDRIVES; idx++) {
					if (data & 1)
						mba->Drive[idx].Regs[nMTDS] &= ~MTDS_ATA;
					data >>= 1;
				}
			}
			break;

		case nMTTC:
			tu = tm->dsUnit = &tm->Slaves[data & MTTC_SLAVE];
			MTDS = (MTDS & ~MTDS_SLAVE) | tu->Status;
			MTDT = tu->devType;
			MTSN = (tu->Flags & MBA_PRESENT) ? tu->Serial : 0;
			MTTC = (MTTC & ~MTTC_WR) | (data & MTTC_WR);

			// Check any slave drive that still require slave attention
			for (idx = 0; idx < TM_MAXSLAVES; idx++)
				if (tm->Slaves[idx].Status & MTDS_SSC)
					MTDS |= MTDS_SSC;

			break;

		default:
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: (W) Unknown MASSBUS Register %o\n",
					mbaDrive->Unit.devName, reg);
#endif /* DEBUG */
			MTER |= MTER_ILR;
			st = EMU_ILR;
			break;
	}

	if (MTER)
		MTDS |= MTDS_ERR;

/*	return st; */
}

void tu_Execute(MBA_DRIVE *mbaDrive)
{
	TM_DEVICE *tm  = (TM_DEVICE *)mbaDrive->FileRef;
	TU_DRIVE  *tu  = tm->dsUnit;
	VMT_TAPE  *vmt = &tu->dpTape;
	int       fnc  = (MTCS1 & MTCS1_FUNC) >> 1;

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: Execute Function: %s\n",
			tu->Unit.devName, tu_fncNames[fnc]);
#endif /* DEBUG */

	if (fnc != FNC_DCLR) {
		if (MTDS & MTDS_ERR) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Function - Not accetped when DS_ERR is set.\n",
					tu->Unit.devName);
#endif /* DEBUG */
			return;
		}

		if ((tu->Status & MTDS_MOL) == 0) {
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Medium is not online!\n", tu->Unit.devName);
#endif /* DEBUG */
			MTER  |= MTER_UNS;
			MTDS  |= MTDS_ERR;
			MTCS1 &= ~MTCS1_GO;
			tu_SetReady(mbaDrive);
			return;
		}
	}

	switch (fnc) {
		case FNC_NOP:          // No Operation
			MTCS1 &= ~MTCS1_GO;
			tu_SetReady(mbaDrive);
			break;

		case FNC_UNLOAD:       // Unload (Rewind, offline)
		case FNC_REWIND:       // Rewind
			tu_SetStatus(tu, MTDS_PIP);
			tu_Service(mbaDrive);
			break;

		case FNC_DCLR:         // Drive Clear
			tu_ResetDrive(mbaDrive);
			MTCS1 &= ~MTCS1_GO;
			tu_SetReady(mbaDrive);
			break;

		case FNC_SP_FWD:       // Space Forward
			if ((MTTC & MTTC_FCS) == 0) {
				MTER |= MTER_NEF;
				break;
			}

			tu_SetStatus(tu, MTDS_PIP);
			tu_Service(mbaDrive);
			break;

		case FNC_SP_REV:       // Space Reverse
			if ((tu->Position == TM_BOT) || ((MTTC & MTTC_FCS) == 0)) {
				MTER |= MTER_NEF;
				break;
			}

			tu_SetStatus(tu, MTDS_PIP);
			tu_Service(mbaDrive);
			break;

		case FNC_RD_REV:       // Read Reverse
		case FNC_CHK_REV:      // Verify Reverse
			if (tu->Position == TM_BOT) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Error - Already Bottom of Tape\n",
						tu->Unit.devName);
#endif /* DEBUG */
				MTER |= MTER_NEF;
				break;
			}
			break;

		case FNC_WR_FWD:       // Write Forward
			if ((MTTC & MTTC_FCS) == 0) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Error - Zero Frame Count\n",
						tu->Unit.devName);
#endif /* DEBUG */
				MTER |= MTER_NEF;
				break;
			}
		case FNC_WR_EOF:       // Write Tape Mark
		case FNC_ERASE:        // Erase Tape
			if (tu->Status & MTDS_WRL) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Error - Write Locked\n", tu->Unit.devName);
#endif /* DEBUG */
				MTER |= MTER_NEF;
				break;
			}
		case FNC_CHK_FWD:      // Verify Forward
		case FNC_RD_FWD:       // Read Forward
			if ((tu->Flags & MBA_ATTACHED) == 0) {
#ifdef DEBUG
				if (dbg_Check(DBG_IOREGS))
					dbg_Printf("%s: Error - Medium not on tape drive\n",
						tu->Unit.devName);
#endif /* DEBUG */
				MTER |= MTER_UNS;
				break;
			}
			tu_Service(mbaDrive);
			break;

		default:                // Illegal Function
#ifdef DEBUG
			if (dbg_Check(DBG_IOREGS))
				dbg_Printf("%s: Unknown function code %02o\n",
					tu->Unit.devName, (MTCS1 & (MTCS1_FUNC|MTCS1_GO)));
#endif /* DEBUG */
			MTCS1 &= ~MTCS1_GO;
			MTER  |= MTER_ILF;
			tu_SetReady(mbaDrive);
			break;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS) && MTER) {
		if (MTER & MTER_UNS)
			dbg_Printf("%s: Unsafe Operation - Aborted.\n",
				tu->Unit.devName);
		if (MTER & MTER_NEF)
			dbg_Printf("%s: Non-Executable Function - Aborted.\n",
				tu->Unit.devName);
	}
#endif /* DEBUG */
}

inline void tu_ResultStatus(TU_DRIVE *tu, int rc)
{
	switch (rc) {
		case MT_MARK:
			tu->Position = TM_MARK; // Tape Mark
			tu_SetStatus(tu, MTDS_TM);
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: ** Tape Mark **\n", tu->Unit.devName);
#endif /* DEBUG */
			break;

		case MT_EOT:
			tu->Position = TM_EOT; // End of Tape
			tu_SetStatus(tu, MTDS_EOT);
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: ** End of Tape **\n", tu->Unit.devName);
#endif /* DEBUG */
			break;

		case MT_BOT:
			tu->Position = TM_BOT; // Bottom of Tape
			tu_SetStatus(tu, MTDS_BOT);
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: ** Bottom of Tape **\n", tu->Unit.devName);
#endif /* DEBUG */
			break;

#ifdef DEBUG
		case MT_ERROR:
			{
				VMT_TAPE *vmt = &tu->dpTape;

				dbg_Printf("%s: ** Error Code: %d (%s)\n",
					tu->Unit.devName, vmt->errCode, strerror(vmt->errCode));
			}
			break;
#endif /* DEBUG */
	}
}

void tu_Service(void *dptr)
{
	MBA_DRIVE  *mbaDrive = (MBA_DRIVE *)dptr;
	TM_DEVICE  *tm  = (TM_DEVICE *)mbaDrive->FileRef;
	TU_DRIVE   *tu  = tm->dsUnit;
	MBA_CALLBACK *mbaCall = mbaDrive->Callback;
	VMT_TAPE   *vmt = &tu->dpTape;
	VMT_FORMAT *fmt = vmt->Format;
	int        fnc  = (MTCS1 & MTCS1_FUNC) >> 1;
	int        count    = 0; // number of bytes had been transferred
	int        len, rc;

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Process: %s\n",
			tu->Unit.devName, tu_fncNames[fnc]);
#endif /* DEBUG */

	switch (fnc) {
		case FNC_UNLOAD: // Unload (Rewind, offline)
		case FNC_REWIND: // Rewind
			rc = fmt->Rewind(vmt);
			if (fnc == FNC_UNLOAD)
				tu_Detach(mbaDrive);
			else {
				tu->Position = TM_BOT;
				tu_ClearStatus(tu, MTDS_PIP);
				tu_SetStatus(tu, MTDS_SSC|MTDS_BOT);
				tu_SetAttention(mbaDrive);
			}
			break;

		case FNC_SP_FWD: // Space Forward
		case FNC_SP_REV: // Space Reverse
			do {
				MTFC++;

				tu_ClearStatus(tu, MTDS_EOT|MTDS_TM|MTDS_BOT);
				if ((rc = fmt->Skip(vmt, (fnc == FNC_SP_FWD) ? 1 : -1)) < 0) {
					tu_ResultStatus(tu, rc);
					break; // Get out of loop.
				}

				tu->Position = TM_MOT; // Middle of Tape
				count++;
			} while (MTFC);

#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: %d record%s had been skipped\n",
					tu->Unit.devName, count, ((count > 1) ? "s" : ""));
#endif /* DEBUG */

			if (MTFC == 0)
				MTTC &= ~MTTC_FCS;
			tu_ClearStatus(tu, MTDS_PIP);
			tu_SetAttention(mbaDrive);

			break;

		case FNC_WR_EOF: // Write Tape Mark (EOF)
			fmt->Mark(vmt);
		case FNC_ERASE:  // Erase Tape
			tu_SetAttention(mbaDrive);
			break;

		case FNC_WR_FWD: // Write Forward
			if (tu_CheckStatus(tu, MTDS_WRL)) {
#ifdef DEBUG
				if (dbg_Check(DBG_IODATA))
					dbg_Printf("%s: - Write Lock Error\n", tu->Unit.devName);
#endif /* DEBUG */
				MTER |= MTER_NEF;
				break;
			}

			if (mbaCall->BeginIO)
				mbaCall->BeginIO(mbaDrive->pDevice);
			if ((len = tu_ReadData(tu, 0200000 - MTFC)) <= 0)
				break;

			tu_ClearStatus(tu, MTDS_EOT|MTDS_TM|MTDS_BOT);
			if ((rc = fmt->Write(vmt, tu_cBuffer, len)) <= 0)
				tu_ResultStatus(tu, rc);
			else {
				tu->Position = TM_MOT; // Middle of Tape

#ifdef DEBUG
				if (dbg_Check(DBG_IODATA)) {
					if (MTFC == 0)
						dbg_Printf("%s: Wrote %d bytes of data.\n",
							tu->Unit.devName, rc);
					else
						dbg_Printf("%s: Wrote %d of %d bytes of data.\n",
							tu->Unit.devName, rc, -MTFC);
				}
#endif /* DEBUG */
				count = rc;
			}
			if (mbaCall->EndIO)
				mbaCall->EndIO(mbaDrive->pDevice);
			break;

		case FNC_CHK_FWD: // Verify Forward
		case FNC_RD_FWD:  // Read Forward
			if (mbaCall->BeginIO)
				mbaCall->BeginIO(mbaDrive->pDevice);
			tu_ClearStatus(tu, MTDS_EOT|MTDS_TM|MTDS_BOT);
			if ((rc = fmt->Read(vmt, tu_cBuffer, 0200000 - MTFC)) <= 0)
				tu_ResultStatus(tu, rc);
			else {
				tu->Position = TM_MOT; // Middle of Tape

#ifdef DEBUG
				if (dbg_Check(DBG_IODATA)) {
					if (MTFC == 0)
						dbg_Printf("%s: Read %d bytes of data.\n",
							tu->Unit.devName, rc);
					else
						dbg_Printf("%s: Read %d of %d bytes of data.\n",
							tu->Unit.devName, rc, -MTFC);
				}
#endif /* DEBUG */
				count = rc;

				if (fnc == FNC_RD_FWD)
					tu_WriteData(tu, count);
			}
			if (mbaCall->EndIO)
				mbaCall->EndIO(mbaDrive->pDevice);
			break;
	}

	// Update drive registers
	if ((count > 0) && (fnc >= 024)) {
		MTFC += count;
		if (MTFC == 0)
			MTTC &= ~MTTC_FCS;
	}

	// Any errors? If so, set ATA bit.
	if (MTER)
		tu_SetAttention(mbaDrive);

	// Inform RH11 controller that tape formatter is ready now
	MTCS1 &= ~MTCS1_GO;
	tu_SetReady(mbaDrive);

//	return EMU_OK;
}

int tu_Boot(MBA_DRIVE *mbaDrive, int argc, char **argv)
{
	TM_DEVICE *tm   = (TM_DEVICE *)mbaDrive->FileRef;
	TU_DRIVE  *tu   = tm->dsUnit;
	UQ_BOOT   *bt   = &tu->Boot;

	if ((bt->Flags & BT_SUPPORTED) == 0)
		return EMU_NOTSUPPORTED;
	if (bt->Flags & BT_VALID) {
		// Execute boot device and return.
		return bt->Execute(bt, argc, argv);
	}
	return EMU_NOTSUPPORTED;
}

// ****************************************************************

#if 0
DEVICE tu_Device =
{
	"TM",          // Device Name
	"MASSBUS Tape Controllers",
	"v0.8.2 (Alpha)",// Version
	tu_dTypes,     // Listing of RM devices
	NULL,          // Listing of Units
	NULL,          // Listing of Slave Devices - Not used
	NULL,          // Listing of Commands
	NULL,          // Listing of Set Commands
	NULL,          // Listing of Show Commands

	0,             // Number of Devices
	TM_MAXUNITS,   // Number of Units

	tu_Initialize, // Initialize Routine
	tu_Reset,      // Reset Routine
	tu_Create,     // Create Routine
	tu_Delete,     // Delete Routine
	NULL,          // Configure Routine
	tu_Enable,     // Enable Routine
	tu_Disable,    // Disable Routine
	tu_Attach,     // Attach Routine
	tu_Detach,     // Detach Routine
	NULL,          // Format Routine - Not Used
	tu_ReadIO,     // Read I/O Routine
	tu_WriteIO,    // Write I/O Routine
	tu_Process,    // Process Routine
	tu_Boot,       // Boot Routine
	NULL,          // Execute Routine - Not Used

	tu_SetUnit,    // SetUnit Routine
	tu_SetATA,     // SetATA Routine
	tu_ClearATA    // ClearATA Routine
};
#endif

MBA_DEVINFO tu_DriveInfo =
{
	"TM",                // Device Name
	"Tape Drive Series", // Description
	"0.2 (Alpha)",       // Version
	tm_dTypes,           // Listing of RP devices

	tu_InitDrive,      // InitDrive Routine
	tu_ResetDrive,     // ResetDrive Routine
	tu_Attach,         // Mount/Attach Routine
	tu_Detach,         // Dismount/Detach Routine
	tu_Boot,           // Boot Routine
	tu_ReadIO,         // Read I/O Routine
	tu_WriteIO,        // Write I/O Routine
	tu_CheckAttention, // Check Attention for AS register
	tu_ClearAttention, // Clear Attention for AS register
};
