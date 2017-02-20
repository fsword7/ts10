// timer.c - Real/Simulation Timers Support Routines
//
// Copyright (c) 2001-2003, Timothy M. Stark
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
// 02/04/03  TMS   Added global time.
// 02/04/03  TMS   Modification history started here.
//
// -------------------------------------------------------------------------

#include <sys/time.h>
#include <signal.h>

#include "emu/defs.h"

#ifdef HAVE_SIGACTION
struct sigaction sigTimer;
#endif /* HAVE_SIGACTION */

static struct itimerval startTimer =
{ 
	{ 0, 10000 },
	{ 0, 10000 }
};

static struct itimerval stopTimer =
{ 
	{ 0, 0 },
	{ 0, 0 }
};

static struct itimerval oldTimer;

#define  NOQUEUE_WAIT 10000

uint32    ts10_GlobalTime  = 0;
int32     ts10_ClkInterval = 0;
int32     ts10_NoqueueTime = 0;
CLK_QUEUE *ts10_SimClock   = NULL; // Simulation Clock Queue
CLK_QUEUE *ts10_RealClock  = NULL; // Real Clock Queue

static __inline__ void UpdateTime(uint32 *tmr)
{
	ts10_GlobalTime += (*tmr - ts10_ClkInterval);
	*tmr = ts10_ClkInterval;
}

int ts10_StartTimer(void)
{
	setitimer(ITIMER_REAL, &startTimer, &oldTimer);
}

int ts10_StopTimer(void)
{
	setitimer(ITIMER_REAL, &stopTimer, &oldTimer);
}

int ts10_SetAlarm(void (*handler)(int))
{
#ifdef HAVE_SIGACTION
	sigTimer.sa_handler = handler;
	sigTimer.sa_flags   = 0;
	sigaction(SIGALRM, &sigTimer, NULL);
#else
	signal(SIGALRM, handler);
#endif /* HAVE_SIGACTION */
	return EMU_OK;
}

void ts10_SetRealTimer(CLK_QUEUE *qptr)
{
	// Enter a timer entry into real clock queue.
	qptr->Next     = ts10_RealClock;
	ts10_RealClock = qptr;
}

void ts10_CancelRealTimer(CLK_QUEUE *qptr)
{
	CLK_QUEUE *cptr;

	// Remove that entry from the real timer queue.
	if (ts10_RealClock == qptr) {
		ts10_RealClock = qptr->Next;
		return;
	}
	
	for (cptr = ts10_RealClock; cptr; cptr = cptr->Next) {
		if (cptr->Next == qptr) {
			cptr->Next = qptr->Next;
			return;
		}
	}
}

void ts10_TickRealTimer(int sig)
{
	CLK_QUEUE *qptr, *pptr;

	if (qptr = ts10_RealClock) {
		do {
			if ((qptr->Flags & CLK_ENABLE) &&
			    ((qptr->outTimer -= CLK_TICK) <= 0)) {
				// Update clock queue
				if (qptr->Flags & CLK_REACTIVE) {
					qptr->outTimer = qptr->nxtTimer;
				} else {
					if (qptr == ts10_RealClock)
						ts10_RealClock = qptr->Next;
					else
						pptr = qptr->Next;
				}
				// Execute a tick routine.
				qptr->Execute(qptr->Device);
			}
			pptr = qptr;
		} while (qptr = qptr->Next);
	}
}

//****************************************************************
//****************************************************************
//****************************************************************

uint32 ts10_GetGlobalTime(void)
{
	// Update current global time.
	if (ts10_SimClock)
		UpdateTime(&ts10_SimClock->outTimer);
	else
		UpdateTime(&ts10_NoqueueTime);

	// Return latest global time.
	return ts10_GlobalTime;
}

void ts10_InitTimer(void)
{
	ts10_SimClock    = NULL;
	ts10_GlobalTime  = 0;
	ts10_ClkInterval = 0;
	ts10_NoqueueTime = 0;
}

void ts10_SetTimer(CLK_QUEUE *qptr)
{
	CLK_QUEUE *cptr, *pptr = NULL;
	int adjTimer = 0;

	// First, check if this unit already is activated.
	if (qptr->Flags & CLK_PENDING) {
#ifdef DEBUG
		if (dbg_Check(DBG_TIMER))
			dbg_Printf("TIMER: Entry: %s - Already Pending.\n",
				qptr->Name ? qptr->Name : "(None)");
#endif /* DEBUG */
		return;
	}

	if ((qptr->outTimer < 0) || (qptr->nxtTimer < 0)) {
#ifdef DEBUG
		if (dbg_Check(DBG_TIMER)) {
			dbg_Printf("TIMER: Entry: %s - Negative Countdown.\n",
				qptr->Name ? qptr->Name : "(None)");
			dbg_Printf("TIMER:    Current %d   Next %d\n",
				qptr->outTimer, qptr->nxtTimer);
		}
#endif /* DEBUG */
		return;
	}

	if (ts10_ClkInterval < 0) {
#ifdef DEBUG
		dbg_Printf("TIMER: *** Negative Clock Interval: %d\n",
			ts10_ClkInterval);
#endif /* DEBUG */
		ts10_ClkInterval = 0;
	}

	qptr->outTimer =  qptr->nxtTimer;
	qptr->Flags    |= CLK_PENDING;

	if (ts10_SimClock == NULL) {
		// Put that entry on the empty clock queue.
		UpdateTime(&ts10_NoqueueTime);
		qptr->Next    = NULL;
		ts10_SimClock = qptr;
	} else {
		// Update first entry for latest interval count.
		UpdateTime(&ts10_SimClock->outTimer);
		ts10_SimClock->outTimer = ts10_ClkInterval;

		// Find that entry between their timer by ascending.
		for (cptr = ts10_SimClock; cptr; cptr = cptr->Next) {
			if (qptr->outTimer < (cptr->outTimer + adjTimer))
				break;
			adjTimer += cptr->outTimer;
			pptr = cptr;
		}

		// Insert that entry on elsewhere within the clock queue.
		qptr->outTimer -= adjTimer;
		if (cptr == ts10_SimClock) {
			qptr->Next = ts10_SimClock;
			ts10_SimClock = qptr;
		} else {
			qptr->Next =  pptr->Next;
			pptr->Next =  qptr;
		}
	}

	// Set latest next interval count
	ts10_ClkInterval = ts10_SimClock->outTimer;

#ifdef DEBUG
	if (dbg_Check(DBG_TIMER)) {
		CLK_QUEUE *nptr = ts10_SimClock;
		dbg_Printf("TIMER: That Entry: %s  Interval Count: %d\n",
			qptr->Name ? qptr->Name : "(None)", qptr->outTimer + adjTimer);
		if (qptr != nptr) {
			dbg_Printf("TIMER: Next Entry: %s  Interval Count: %d\n",
				nptr->Name ? nptr->Name : "(None)", nptr->outTimer);
		}
	}
#endif /* DEBUG */
}

void ts10_CancelTimer(CLK_QUEUE *qptr)
{
	CLK_QUEUE *cptr, *nptr = NULL;

	if (ts10_SimClock == NULL)
		return;
	UpdateTime(&ts10_SimClock->outTimer);

	// Remove that entry from the timer queue.
	if (ts10_SimClock == qptr) {
		nptr = ts10_SimClock = qptr->Next;
	} else {
		for (cptr = ts10_SimClock; cptr; cptr = cptr->Next) {
			if (cptr->Next == qptr) {
				nptr = cptr->Next = qptr->Next;
				break;
			}
		}
	}

	// Update timer alarm.
	qptr->Flags &= ~CLK_PENDING;
	if (nptr != NULL)
		nptr->outTimer += qptr->outTimer;

//	ts10_ClkInterval = ts10_SimClock
//		? ts10_SimClock->outTimer : NOQUEUE_WAIT;
	if (ts10_SimClock) {
		ts10_ClkInterval = ts10_SimClock->outTimer;
	} else {
		ts10_ClkInterval = NOQUEUE_WAIT;
		ts10_NoqueueTime = NOQUEUE_WAIT;
	}

	qptr->Next = NULL;
}

void ts10_ExecuteTimer(void)
{
	CLK_QUEUE *qptr;

	if (ts10_SimClock == NULL) {
		UpdateTime(&ts10_NoqueueTime);
		ts10_NoqueueTime = NOQUEUE_WAIT;
		ts10_ClkInterval = NOQUEUE_WAIT;
	} else {
//		do {
			// Remove that entry from the queue.
			qptr = ts10_SimClock;
			ts10_SimClock = qptr->Next;

#ifdef DEBUG
			if (dbg_Check(DBG_TIMER)) {
				CLK_QUEUE *nptr;
				dbg_Printf("TIMER: Entry: %s - Now Executing.\n",
					qptr->Name ? qptr->Name : "(None)");
				if (nptr = qptr->Next) {
					dbg_Printf("TIMER: Next Entry: %s  Interval Count: %d\n",
						nptr->Name ? nptr->Name : "(None)", nptr->outTimer);
				}
			}
#endif /* DEBUG */

			// Reset that entry or put it back to the queue.
			qptr->Flags &= ~CLK_PENDING;
			if (qptr->Flags & CLK_REACTIVE)
				ts10_SetTimer(qptr);
			else
				qptr->Next = NULL;

			// Update timer alarm.
//			ts10_ClkInterval = ts10_SimClock
//				? ts10_SimClock->outTimer : NOQUEUE_WAIT;
			if (ts10_SimClock) {
				ts10_ClkInterval = ts10_SimClock->outTimer;
			} else {
				ts10_ClkInterval = NOQUEUE_WAIT;
				ts10_NoqueueTime = NOQUEUE_WAIT;
			}

			// Now execute that entry.
			if (qptr->Execute != NULL)
				qptr->Execute(qptr->Device);
//		} while (ts10_ClkInterval == 0);
	}
}

