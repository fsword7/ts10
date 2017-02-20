// ts.c - TS11/TSV05 Tape Controller/Drive Emulator
//
// Copyright (c) 2002-2003, Timothy M. Stark
// Copyright (c) 1993-2003, Robert M. Supnik, see file 'Copyright'
// for more information.  Some derived works from SIMH emulator.
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
//
// -------------------------------------------------------------------------
//
// Modification History:
//
// 02/06/03  TMS  Updated some data access to follow new virtual
// 					tape routines in 'vtape.c' file.  Debugged and
// 					tested them.
// 02/06/03  TMS  Modification history started here.
//
// -------------------------------------------------------------------------

#include "emu/defs.h"
#include "dec/ts.h"

#ifdef DEBUG
static cchar *regNameR[] = {
	"TSBA",  // Bus Address Register
	"TSSR",  // Status Register
};

static cchar *regNameW[] = {
	"TSDB",  // Data Buffer Register
	"TSSR",  // Status Register
	"TSDBX", // Extended Data Buffer Register (TSV05 only)
};
#endif /* DEBUG */

static const uint16 msgAck[] = {
	MSG_ACK|MSG_CEND,           // TC0
	MSG_ACK|MSG_MATN|MSG_CATN,  // TC1
	MSG_ACK|MSG_CEND,           // TC2
	MSG_ACK|MSG_CFAIL,          // TC3
	MSG_ACK|MSG_CERR,           // TC4
	MSG_ACK|MSG_CERR,           // TC5
	MSG_ACK|MSG_CERR,           // TC6
	MSG_ACK|MSG_CERR            // TC7
};

inline void ts_UpdateTSSR(register TS_DEVICE *ts, uint16 sr)
{
	// Set 16/17 bit of current bus address.
	sr = (sr & ~TSSR_EMA) | ((TSBA >> (16 - TSSR_P_EMA)) & TSSR_EMA);

	// Check medium is online or not.  If not,
	// set OFL bit on TSSR.
	if (ts->Flags & TS_ATTACHED)
		sr &= ~TSSR_OFL;
	else
		sr |= TSSR_OFL;

	// Now update TSSR register.
	TSSR = sr & ~TSSR_MBZ;
}

inline void ts_UpdateXS0(register TS_DEVICE *ts, uint32 xs0)
{
	// Clear all old bits first.
	xs0 = (xs0 & ~(XS0_ONL|XS0_WLK|XS0_BOT|XS0_IE)) | XS0_PET;

	if (ts->Flags & TS_ATTACHED) {
		xs0 |= XS0_ONL; // Medium Online.
		if (ts->Flags & TS_WLOCK)
			xs0 |= XS0_WLK;
	} else
		xs0 &= ~XS0_EOT;

	// Check interrupt enable.  If so, set IE bit on
	// extended status #0 register.
	if (cmdHdr & CMD_IE)
		xs0 |= XS0_IE;

	// Now updata XS0 register.
	msgXSt0 = xs0;
}

// Bus Initialization
uint32 ts_Rewind(TS_DEVICE *);
void ts_ResetDevice(TS_DEVICE *ts)
{
	MAP_IO *io = &ts->ioMap;

	// Cancel all timers and interrupts.
	ts10_CancelTimer(&ts->svcTimer);
	io->CancelInterrupt(io, 0);
	if (ts->Flags & TS_ATTACHED)
		ts_Rewind(ts);

	// Reset all internal flags.
	ts->Flags &= ~(TS_BOOT|TS_QATN|TS_OWNCMD|TS_OWNMSG);

	// Reset all Unibus/Qbus registers.
	TSBA  = 0;
	TSDBX = 0;
	ts_UpdateTSSR(ts, TSSR_NBA|TSSR_SSR);

	// Initialize all packets
	memset(ts->pktCmd,  0, CMD_LEN);
	memset(ts->pktMsg,  0, MSG_LEN);
	memset(ts->pktChar, 0, WCH_LEN);
	ts_UpdateXS0(ts, XS0_VCK);
}

void ts_CmdDone(TS_DEVICE *ts, uint16 tc, uint16 xs0, uint16 msg)
{
	UQ_CALL *cb = ts->Callback;
	MAP_IO  *io = &ts->ioMap;
	int     len, rc;

	// Update final message.
	ts_UpdateXS0(ts, msgXSt0 | xs0);
	if (wchXOpts & WCHX_HDS)
		msgXSt4 |= XS4_HDS;

	// Send a message packet to host if request.
	if (msg && !(TSSR & TSSR_NBA)) {
		TSBA   = (wchHAddr << 16) | wchLAddr;
		msgHdr = msg;
		msgLen = wchLen - 4;
		
		// Now send a message packet to host.
		len = (wchLen < MSG_LEN) ? wchLen : MSG_LEN;
		if (rc = cb->WriteBlock(ts->System, TSBA, msgPkt, len, 0)) {
			TSSR |= TSSR_NXM;
			tc = (tc & ~TSSR_TC) | TC4;
		}
		TSBA += len - rc;
	}

	// Update final TSSR, ring doorbell if enable, and return.
	// Host now is on its turn.
	ts_UpdateTSSR(ts, TSSR | TSSR_SSR | (tc | (tc ? TSSR_SC : 0)));
	if (cmdHdr & CMD_IE)
		io->SendInterrupt(io, 0);
	ts->Flags &= ~(TS_OWNCMD|TS_OWNMSG);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Status=%o TC=%o\n",
			ts->Unit.devName, msgXSt0, tc >> 1);
#endif /* DEBUG */
}

// Command done for two tape operations
void ts_CmdDone2(TS_DEVICE *ts, uint32 st0, uint32 st1)
{
	uint16 xs, sr, tc;

	// Break down two xtc values.
	xs = GET_XS(st0 | st1);
	sr = GET_SR(st0 | st1);
	tc = MAX_TC(GET_TC(st0), GET_TC(st1));

	// Send a message to host.
	ts_CmdDone(ts, sr | tc, xs, msgAck[tc >> 1]);
}

uint32 ts_ResultStatus(TS_DEVICE *ts, int rc)
{
	switch (rc) {
		case MT_MARK: // Tape Mark
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: ** Tape Mark **\n", ts->Unit.devName);
#endif /* DEBUG */
			msgXSt0 |= XS0_MOT; // Tape Motion
			return XTC(XS0_TMK|XS0_RLS, TC2);

		case MT_EOT: // End of Tape
//		case MT_EOM: // End of Medium
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: ** End of Medium **\n", ts->Unit.devName);
#endif /* DEBUG */
			msgXSt3 |= XS3_OPI; // Incomplete Operation
			return XTC(XS0_RLS, TC6);

#if 0
		case MT_EOT: // End of Tape
#ifdef DEBUG
//			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: ** End of Tape **\n", ts->Unit.devName);
#endif /* DEBUG */
			return XTC(XS0_EOT|XS0_RLS, TC2);
#endif

		case MT_BOT: // Begin of Tape
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("%s: ** Start of Tape **\n", ts->Unit.devName);
#endif /* DEBUG */
			msgXSt3 |= XS3_RIB; // Reverse to BOT
			return XTC(XS0_BOT|XS0_RLS, TC2);

		case MT_ERROR: // Tape Error
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA)) {
				VMT_TAPE *vmt = &ts->dpTape;
				dbg_Printf("%s: ** Error Code: %d (%s)\n",
					ts->Unit.devName, vmt->errCode, strerror(vmt->errCode));
			}
#endif /* DEBUG */
			msgXSt1 |= XS1_UCOR;
			return XTC(XS0_RLS, TC6);
	}
}

// Rewind and unload a tape.
uint32 ts_Unload(TS_DEVICE *ts)
{
	VMT_TAPE   *vmt  = &ts->dpTape;
	VMT_FORMAT *fmt  = vmt->Format;

	// First, rewind a tape.
	if (vmt->mtAddr > 0)
		msgXSt0 |= XS0_MOT; // Tape Motion
	fmt->Rewind(vmt);

	// Now unload a tape.
	vmt_CloseTape(vmt);
	if (vmt->fileName) {
		free(vmt->fileName);
		vmt->fileName = NULL;
	}
	ts->Flags &= ~TS_ATTACHED;

	return 0;
}

// Rewind a tape.
uint32 ts_Rewind(TS_DEVICE *ts)
{
	VMT_TAPE   *vmt  = &ts->dpTape;
	VMT_FORMAT *fmt  = vmt->Format;

	if (vmt->mtAddr > 0)
		msgXSt0 |= XS0_MOT; // Tape Motion
	fmt->Rewind(vmt);

	return 0;
}

// Space Forward/Reverse
uint32 ts_Space(TS_DEVICE *ts, int32 fc)
{
	VMT_TAPE   *vmt  = &ts->dpTape;
	VMT_FORMAT *fmt  = vmt->Format;
	uint32     xtc   = XTC_NORMAL;
	int        dir   = (fc < 0) ? -1 : 1;
	int        count = 0;
	int        rc;
	
	if (fc < 0) {
		fc = -fc;
		msgXSt3 |= XS3_REV;
	}

	if (fc == 0) {
		// Continue until first tape mark or error.
		while((rc = fmt->Skip(vmt, dir)) == 0) {
			msgXSt0 |= XS0_MOT; // Tape Motion.
			count++;
		}
		xtc = ts_ResultStatus(ts, rc);
	} else {
		do {
			fc--;
			if ((rc = fmt->Skip(vmt, dir)) < 0) {
				xtc = ts_ResultStatus(ts, rc);
				break; // Get out of a loop.
			}
			msgXSt0 |= XS0_MOT; // Tape Motion.
			count++;
		} while (fc > 0);
		msgFrame = fc;
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: %d record%s skipped.\n",
			ts->Unit.devName, count, (count != 1 ? "s" : ""));
#endif /* DEBUG */

	return xtc;
}

// Skip Files Forward/Reverse
uint32 ts_Skip(TS_DEVICE *ts, uint32 fc)
{
	VMT_TAPE   *vmt = &ts->dpTape;
	VMT_FORMAT *fmt = vmt->Format;
	uint32     ptmk = FALSE;
	uint32     ppos, xtc;

	msgFrame = fc;
	if ((wchOpts & WCH_ENB))
		ptmk = TRUE;

	do {
		xtc = ts_Space(ts, 0);
		if (GET_XS(xtc) & MT_MARK) {
			msgFrame--;
			// ACTION: Check Long EOT (ESS)
			ptmk = TRUE;
		} else if (xtc)
			return xtc;
		ptmk = FALSE;
	} while (msgFrame > 0);

	return 0;
}

// Read Forward/Reverse
uint32 ts_Read(TS_DEVICE *ts, uint8 *buf, int32 fc)
{
	VMT_TAPE   *vmt  = &ts->dpTape;
	VMT_FORMAT *fmt  = vmt->Format;
	UQ_CALL    *call = ts->Callback;
	int32      tbc, wbc; // Word count
	uint32     idx, rc;

	// Set up frame count first.
	if (fc < 0) {
		msgXSt3 |= XS3_REV;
		msgFrame = -fc;
	} else
		msgFrame = fc;

	// Start I/O action here.
	if ((tbc = fmt->Read(vmt, buf, fc)) <= 0)
		return ts_ResultStatus(ts, tbc);

	// Successful.
	msgXSt0 |= XS0_MOT; // Tape Motion
	TSBA = (cmdHAddr << 16) | cmdLAddr;	
	wbc  = (tbc > fc) ? fc : tbc;

	// Fill 0's into rest of good record.
	// then transfer entire record to host.
	for (idx = tbc; idx < wbc; idx++)
		buf[idx] = 0;
	if (rc = call->WriteBlock(ts->System, TSBA, (uint8 *)buf, wbc, 0)) {
		msgFrame -= rc;
		ts_UpdateTSSR(ts, TSSR | TSSR_NXM);
		return XTC(XS0_RLS, TC4);
	}
	msgFrame -= wbc;

	if (msgFrame)  // Too short record
		return XTC(XS0_RLS, TC2);
	if (tbc > wbc) // Too long record
		return XTC(XS0_RLL, TC2);
	return XTC_NORMAL;
}

uint32 ts_Write(TS_DEVICE *ts, uint8 *buf, int32 fc)
{
	VMT_TAPE   *vmt  = &ts->dpTape;
	VMT_FORMAT *fmt  = vmt->Format;
	UQ_CALL    *call = ts->Callback;
	int32      tbc; // Transferred byte count

	// Prepare for I/O action.
	msgFrame = fc;
	TSBA = (cmdHAddr << 16) | cmdLAddr;

	// Transfer data from host.
	if (tbc = call->ReadBlock(ts->System, TSBA, (uint8 *)buf, fc, 0)) {
		msgFrame -= tbc;
		ts_UpdateTSSR(ts, TSSR | TSSR_NXM);
		return TC5;
	}

	// Write a record to a tape medium.
	if ((tbc = fmt->Write(vmt, buf, fc)) < 0)
		return ts_ResultStatus(ts, tbc);
	msgXSt0 |= XS0_MOT; // Tape Motion.
	msgFrame = 0;

	// Return successful.
	return XTC_NORMAL;
}

// Write a tape mark (EOF)
uint32 ts_Mark(TS_DEVICE *ts)
{
	VMT_TAPE   *vmt  = &ts->dpTape;
	VMT_FORMAT *fmt  = vmt->Format;

	// Write a tape mark on the tape medium.
	if (fmt->Mark(vmt))
		return TC6;
	msgXSt0 |= XS0_MOT; // Tape Motion.

	// Return successful with tape mark bit.
	return XTC(XS0_TMK, TC0);
}

void ts_StartIO(TS_DEVICE *ts)
{
	MAP_IO  *io = &ts->ioMap;
	UQ_CALL *cb = ts->Callback;
	uint32  rc;

	// Initialize that for preparation.
	io->CancelInterrupt(io, 0);
	ts_UpdateTSSR(ts, TSSR & TSSR_NBA);
	ts_UpdateXS0(ts, msgXSt0 & ~XS0_CLR);
	msgFrame = 0;
	msgXSt1  = 0;
	msgXSt2  = 0;
	msgXSt3  = 0;
	msgXSt4  = 0;

	// Get command packet from host.
	if (rc = cb->ReadBlock(ts->System, TSBA, cmdPkt, CMD_LEN, 0)) {
		TSBA += (CMD_LEN - rc);
		ts_CmdDone(ts, TSSR_NXM|TC5, 0, MSG_ACK|MSG_MNEF|MSG_CFAIL);
		return;
	}
	TSBA += CMD_LEN;
	
	// Set ownership on command and message packet,
	// then start/spawn service for I/O process.
	ts->Flags |= TS_OWNCMD|TS_OWNMSG;
	ts10_SetTimer(&ts->svcTimer);
}

void ts_Service(void *dptr)
{
	TS_DEVICE  *ts  = (TS_DEVICE *)dptr;
	UQ_CALL    *cb  = ts->Callback;
	MAP_IO     *io  = &ts->ioMap;
	uint8      buf[65536];
	uint32     fnc, mode;
	int32      fc, len;
	uint32     st0, st1, rc;

	// Load and run a 512-byte bootstrap
	// per request from host.
	if (ts->Flags & TS_BOOT) {
		ts->Flags &= ~TS_BOOT;

		if (ts->Flags & TS_ATTACHED) {
//			uint8 buf[512]; // Bootstrap buffer

			// Rewind a tape then skip to record #1 of tape
			// and finally read a 512-byte record (bootstrap)
			// from a tape.
			ts_Rewind(ts);
			ts_Space(ts, 1);
			ts_Read(ts, buf, 512);

			// Done. Magtape drive is now ready. 
			ts_UpdateTSSR(ts, TSSR | TSSR_SSR);
		} else 
			ts_UpdateTSSR(ts, TSSR | (TSSR_SSR|TC3));
		
		// Ring a doorbell and return.
		if (cmdHdr & CMD_IE)
			io->SendInterrupt(io, 0);
		return;
	}

	// If no acknowledge, set magtape drive
	// is ready and host now is on its turn.
	if ((cmdHdr & CMD_ACK) == 0) {
		ts_UpdateTSSR(ts, TSSR | TSSR_SSR);
		if (cmdHdr & CMD_IE)
			io->SendInterrupt(io, 0);
		ts->Flags &= ~(TS_OWNCMD|TS_OWNMSG);
		return;
	}

	// Get function code and mode.
	fnc  = GET_FNC(cmdHdr);
	mode = GET_MODE(cmdHdr);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("%s: Cmd=%o Mode=%o Addr=%o Len=%o\n",
			ts->Unit.devName, fnc, mode, cmdLAddr, cmdCount);
#endif /* DEBUG */

	// Requires a write characteristics for this operation.
	if ((fnc != FNC_WCHR) && (TSSR & TSSR_NBA)) {
		ts_CmdDone(ts, TC3, 0, 0);
		return;
	}

	// Check if attention is pending, let host know that immediately.
	// Ring doorbell and return.
	if ((ts->Flags & TS_QATN) && (wchOpts & WCH_EAI)) {
		ts_CmdDone(ts, TC1, 0, MSG_MATN|MSG_CATN);
		ts->Flags &= ~TS_QATN;
		io->SendInterrupt(io, 0);
		return;
	}

	if (cmdHdr & CMD_CVC)
		msgXSt0 &= ~XS0_VCK;
	
	
	// Now execute function codes.
	st0 = st1 = 0;
	switch (fnc) {
		case FNC_INIT: // Initialization
			ts_Rewind(ts);
		case FNC_WSSM: // Write Memory
		case FNC_GSTA: // Get Status
			ts_CmdDone(ts, TC0, 0, MSG_ACK|MSG_CEND);
			return;

		case FNC_WCHR: // Write Characteristics
			if ((cmdLAddr & 1) && (cmdCount < 6)) {
				ts_CmdDone(ts, TSSR_NBA|TC3, XS0_ILA, 0);
				return;
			}
			TSBA = (cmdHAddr << 16) | cmdLAddr;

			// Get a write characteristics packet from host.
			len = (cmdCount < WCH_LEN) ? cmdCount : WCH_LEN;
			if (rc = cb->ReadBlock(ts->System, TSBA, wchPkt, len, 0)) {
				TSBA += (len - rc);
				ts_CmdDone(ts, TSSR_NBA|TSSR_NXM|TC5, 0, 0);
				return;
			}
			TSBA += len;

			// Check for valid bits first.
			if ((wchLen < (MSG_LEN - 2)) || (wchHAddr & 0177700) || (wchLAddr & 1)) {
				ts_CmdDone(ts, TSSR_NBA|TC3, 0, 0);
				return;
			}

			// Update status registers and return.
			msgXSt2 |= (XS2_XTF | 1);
			ts_UpdateTSSR(ts, TSSR & ~TSSR_NBA);
			ts_CmdDone(ts, TC0, 0, MSG_ACK|MSG_CEND);
			return;

		case FNC_CTL:  // Control
			switch(mode) {
				case 0: // Message Buffer Release
					ts_UpdateTSSR(ts, TSSR | TSSR_SSR);
					if (wchOpts & WCH_ERI)
						io->SendInterrupt(io, 0);
					ts->Flags = (ts->Flags & ~TS_OWNCMD) | TS_OWNMSG;
					break;

				case 1: // Rewind and Unload
					ts_Unload(ts);
					ts_CmdDone(ts, TC0, 0, MSG_ACK|MSG_CEND);
					break;

				case 2: // Clean
					ts_CmdDone(ts, TC0, 0, MSG_ACK|MSG_CEND);
					break;

				case 3: // Undefined
					ts_CmdDone(ts, TC3, XS0_ILC, MSG_ACK|MSG_MILL|MSG_CFAIL);
					break;

				case 4: // Rewind
					ts_Rewind(ts);
					ts_CmdDone(ts, TC0, XS0_BOT, MSG_ACK|MSG_CEND);
					break;

				default:
					goto error;
			}
			return;

		case FNC_POS:  // Positoning
			fc = (cmdLAddr == 0) ? 0200000 : cmdLAddr;
			switch (mode) {
				case 0: // Space Forward
					st0 = ts_Space(ts, fc);
					break;

				case 1: // Space Reverse
					st0 = ts_Space(ts, -fc);
					break;

				case 2: // Space File Forward
					st0 = ts_Skip(ts, fc);
					break;

				case 3: // Space File Reverse
					st0 = ts_Skip(ts, -fc);
					break;

				case 4: // Rewind
					ts_Rewind(ts);
					break;

				default:
					goto error;
			}
			ts_CmdDone2(ts, st0, 0);
			return;

		case FNC_FMT:  // Format
			switch (mode) {
				case 0: // Write Tape Mark
					st0 = ts_Mark(ts);
					break;

				case 1: // Erase
					break;

				case 2: // Re-write Tape Mark
					st0 = ts_Space(ts, -1);
					st1 = ts_Mark(ts);
					break;

				default:
					goto error;
			}
			ts_CmdDone2(ts, st0, st1);
			return;

		case FNC_READ: // Read Data
			fc = (cmdCount == 0) ? 0200000 : cmdCount;
			switch (mode) {
				case 0: // Read Forward
					st0 = ts_Read(ts, buf, fc);
					break;

				case 1: // Read Reverse
					st0 = ts_Read(ts, buf, -fc);
					break;

				case 2: // Re-read Forward
					if (cmdHdr & CMD_OPP) {
						st0 = ts_Read(ts, buf, -fc);
						st1 = ts_Space(ts, 1);
					} else {
						st0 = ts_Space(ts, -1);
						st1 = ts_Read(ts, buf, fc);
					}
					break;

				case 3: // Re-read Reverse
					if (cmdHdr & CMD_OPP) {
						st0 = ts_Read(ts, buf, fc);
						st1 = ts_Space(ts, -1);
					} else {
						st1 = ts_Space(ts, 1);
						st0 = ts_Read(ts, buf, -fc);
					}
					break;

				default:
					goto error;
			}
			ts_CmdDone2(ts, st0, st1);
			return;
		
		case FNC_WRITE: // Write Data
			fc = (cmdCount == 0) ? 0200000 : cmdCount;
			switch (mode) {
				case 0: // Write Forward
					st0 = ts_Write(ts, buf, fc);
					break;

				case 1: // Re-write Forward
					st0 = ts_Space(ts, -1);
					st1 = ts_Write(ts, buf, fc);
					break;

				default:
					goto error;
			}
			ts_CmdDone2(ts, st0, st1);
			return;
	}

error:
	ts_CmdDone(ts, TC3, XS0_ILC, MSG_ACK|MSG_MILL|MSG_CFAIL);
}

int ts_ReadIO(void *dptr, uint32 pAddr, uint16 *data, uint32 acc)
{
	TS_DEVICE *ts = (TS_DEVICE *)dptr;
	uint32    reg = (pAddr & 03) >> 1;

	switch(reg) {
		case nTSBA: // Bus Address Register
			*data = TSBA;
			break;

		case nTSSR: // Status Register
			*data = TSSR;
			break;

#ifdef DEBUG
		default:
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				ts->Unit.devName, reg, pAddr, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("%s: (R) %s (%o) => %06o (%04X) (Size: %d bytes)\n",
			ts->Unit.devName, regNameR[reg], pAddr, *data, *data, acc);
#endif /* DEBUG */

	return UQ_OK;
}

int ts_WriteIO(void *dptr, uint32 pAddr, uint16 data, uint32 acc)
{
	TS_DEVICE *ts = (TS_DEVICE *)dptr;
	uint32    reg = (pAddr & 03) >> 1;

	switch(reg) {
		case nTSDB: // Data Buffer Register
			// If drive is not ready, do not accept
			// that and return.
			if ((TSSR & TSSR_SSR) == 0) {
				TSSR |= TSSR_RMR;
				break;
			}

			// Set new command packet address.
			TSBA  = ((TSDBX & TSDBX_MASK) << 18) |
				((data & 03) << 16) | (data & 0177774);
			TSDBX = 0;
			
			// Start I/O action.
			ts_StartIO(ts);
			break;

		case nTSSR: // Status Register
			if ((ts->Flags & ID_TSV05) && (pAddr & 1)) {
				// Extended Data Buffer Register
				if (TSSR & TSSR_SSR) {
					TSDBX = data;
					if (data & TSDBX_BOOT) {
						ts->Flags |= TS_BOOT;
						ts10_SetTimer(&ts->svcTimer);
					}
				} else {
					// Register Modification Refused.
					TSSR |= TSSR_RMR;
				}
			} else if (acc == ACC_WORD) {
				// Status Register - Write to reset.
				ts_ResetDevice(ts);
			}
			break;

#ifdef DEBUG
		default:
			dbg_Printf("%s: *** Undefined Register %d (%06o) at line %d in file %s\n",
				ts->Unit.devName, reg, pAddr, __LINE__, __FILE__);
			return UQ_NXM;
#endif /* DEBUG */
	}

#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS)) {
		if (((pAddr & 3) == 3) && (ts->Flags & ID_TSV05)) reg++;
		dbg_Printf("%s: (W) %s (%o) <= %06o (%04X) (Size: %d bytes)\n",
			ts->Unit.devName, regNameW[reg], pAddr, data, data, acc);
	}
#endif /* DEBUG */

	return UQ_OK;
}

// Bus Initialization
void ts_ResetIO(void *dptr)
{
	ts_ResetDevice(dptr);
#ifdef DEBUG
	if (dbg_Check(DBG_IOREGS))
		dbg_Printf("Done.\n");
#endif /* DEBUG */
}

// **************************************************************

void *ts_Create(MAP_DEVICE *newMap, int argc, char **argv)
{
	TS_DEVICE *ts = NULL;
	CLK_QUEUE *timer;
	MAP_IO    *io;

	if (ts = (TS_DEVICE *)calloc(1, sizeof(TS_DEVICE))) {
		// First, set up its description and
		// link it to its parent device.
		ts->Unit.devName    = newMap->devName;
		ts->Unit.keyName    = newMap->keyName;
		ts->Unit.emuName    = newMap->emuName;
		ts->Unit.emuVersion = newMap->emuVersion;
		ts->Device          = newMap->devParent->Device;
		ts->Callback        = newMap->devParent->Callback;
		ts->System          = newMap->devParent->sysDevice;

		// Recongize which device type - TS11 or TSV05.
		if (!strcmp(ts->Unit.keyName, TS11_KEY))
			ts->Flags = ID_TS11;
		else if (!strcmp(ts->Unit.keyName, TSV05_KEY))
			ts->Flags = ID_TSV05;
		else {
			printf("%s: Bug Check: Unknown device name - %s\n",
				ts->Unit.devName, ts->Unit.keyName);
			free(ts);
			return NULL;
		}

		// Set up an I/O space.
		io               = &ts->ioMap;
		io->devName      = ts->Unit.devName;
		io->keyName      = ts->Unit.keyName;
		io->emuName      = ts->Unit.emuName;
		io->emuVersion   = ts->Unit.emuVersion;
		io->Device       = ts;
		io->csrAddr      = TS_CSRADR;
		io->nRegs        = TS_NREGS;
		io->nVectors     = TS_NVECS;
		io->intIPL       = TS_IPL;
		io->intVector[0] = TS_VEC;
		io->ReadIO       = ts_ReadIO;
		io->WriteIO      = ts_WriteIO;
		io->ResetIO      = ts_ResetIO;

		// Assign that registers to QBA's I/O space.
		ts->Callback->SetMap(ts->Device, io);

		// Set up service timer
		timer           = &ts->svcTimer;
		timer->Next     = NULL;
		timer->Flags    = 0;
		timer->outTimer = TS_DELAY;
		timer->nxtTimer = TS_DELAY;
		timer->Device   = ts;
		timer->Execute  = ts_Service;

		// Finally, link it to its mapping device and return.
		newMap->Device = ts;
	}

	return ts;
}

//int ts_Reset(MAP_DEVICE *map)
int ts_Reset(void *dptr)
{
//	TS_DEVICE *ts = (TS_DEVICE *)map->Device;
	TS_DEVICE *ts = (TS_DEVICE *)dptr;

	ts_ResetDevice(ts);
}

// Usage: attach <device> <filename>
int ts_Attach(MAP_DEVICE *map, int argc, char **argv)
{
	TS_DEVICE *ts  = (TS_DEVICE *)map->Device;
	VMT_TAPE  *vmt = &ts->dpTape;
	int       st;

	if (ts->Flags & TS_ATTACHED)
		return EMU_ATTACHED;

	vmt->fileName = (char *)malloc(strlen(argv[2])+1);
	strcpy(vmt->fileName, argv[2]);
	vmt->fmtName  = NULL;

	// Debug Information Request
	vmt->Flags = VMT_DEBUG|VMT_DUMP;

	// Attach a file to that unit
	if (st = vmt_OpenTape(vmt)) {
		printf("Reason: %d\n", st);
		return st;
	}

	// Update TS11/TSV05 registers
	ts->Flags |= TS_ATTACHED;
	TSSR &= ~TSSR_OFL;
	if (!(TSSR & TSSR_NBA) && (wchOpts & WCH_EAI)) {
		if (ts->Flags & TS_OWNMSG) {
			ts_CmdDone(ts, TC1, 0, MSG_MATN|MSG_CATN);
			ts->Flags &= ~TS_QATN;
			ts->ioMap.SendInterrupt(&ts->ioMap, 0);
		} else
			ts->Flags |= TS_QATN;
	}

	printf("%s: File '%s' attached.\n", ts->Unit.devName, argv[2]);

#if 0
	// Enable boot device.
	if (tu->Boot.Flags & BT_SUPPORTED) {
		tu->Boot.ioDevice  = vmt;
		tu->Boot.Flags    |= BT_VALID;
	}
#endif

	return EMU_OK;
}

// Usage: detach <device>
int ts_Detach(MAP_DEVICE *map, int argc, char **argv)
{
	TS_DEVICE *ts  = (TS_DEVICE *)map->Device;
	VMT_TAPE  *vmt = &ts->dpTape;
	int       rc;

	if ((ts->Flags & TS_ATTACHED) == 0)
		return EMU_ARG;

#if 0
	// Disable boot device
	if (tu->Boot.Flags & BT_SUPPORTED) {
		tu->Boot.Flags    &= ~BT_VALID;
		tu->Boot.ioDevice  = NULL;
	}
#endif

	// Detach a file from that unit.
	if (rc = vmt_CloseTape(vmt))
		return rc;
	if (vmt->fileName) {
		free(vmt->fileName);
		vmt->fileName = NULL;
	}
	vmt->fmtName = NULL;

	// Update TS11/TSV05 registers
	ts->Flags &= ~TS_ATTACHED;
	TSSR |= TSSR_OFL;
	if (!(TSSR & TSSR_NBA) && (wchOpts & WCH_EAI)) {
		if (ts->Flags & TS_OWNMSG) {
			ts_CmdDone(ts, TC1, 0, MSG_MATN|MSG_CATN);
			ts->Flags &= ~TS_QATN;
			ts->ioMap.SendInterrupt(&ts->ioMap, 0);
		} else
			ts->Flags |= TS_QATN;
	}

	return EMU_OK;
}

DEVICE ts_Device =
{
	TS11_KEY,         // Key Name
	TS_NAME,          // Emulator Name
	TS_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_DEVICE,        // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	ts_Create,        // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	ts_Reset,         // Reset Routine
	ts_Attach,        // Attach Routine
	ts_Detach,        // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};

DEVICE tsv_Device =
{
	TSV05_KEY,        // Key Name
	TS_NAME,          // Emulator Name
	TS_VERSION,       // Emulator Version
	NULL,             // Listing of slave devices
	DF_SYSMAP,        // Device Flags
	DT_DEVICE,        // Device Type

	// Device Commands
	NULL,             // Commands
	NULL,             // Set Commands
	NULL,             // Show Commands

	// Function Calls
	ts_Create,        // Create Routine
	NULL,             // Configure Routine
	NULL,             // Delete Routine
	ts_Reset,         // Reset Routine
	ts_Attach,        // Attach Routine
	ts_Detach,        // Detach Routine
	NULL,             // Info Device
	NULL,             // Boot Routine
	NULL,             // Execute Routine
#ifdef DEBUG
	NULL,             // Debug Routine
#endif /* DEBUG */
};
