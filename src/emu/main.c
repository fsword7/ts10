// main.c - main routines
//
// Copyright (c) 2001-2002, Timothy M. Stark
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

#include <stdio.h>
#include <signal.h>

#ifdef HAVE_SIGACTION
#include <unistd.h>
#include <stropts.h>
#endif /* HAVE_SIGACTION */

#include "emu/defs.h"
#include "emu/socket.h"

void (*emu_IOTrap)(void) = NULL;
int  emu_State;
int  emu_logFile = -1;

extern char   *ts10_Prompt;
extern SOCKET *ts10_Stdout;
extern MAP_DEVICE *ts10_System;

#ifdef HAVE_SIGACTION
struct sigaction sigInt;   // ^E Exit
struct sigaction sigSegV;  // Bad memory
struct sigaction sigQuit;  // ^T Trace
struct sigaction sigIO;    // Keyboard activity
#endif /* HAVE_SIGACTION */

void emu_Interrupt(int s)
{
	emu_State = EMU_HALT;

#ifndef HAVE_SIGACTION
	signal(SIGINT, emu_Interrupt);
#endif /* HAVE_SIGACTION */
}

void emu_Trace(int s)
{
#ifdef DEBUG
	if (dbg_Check(DBG_TRACE)) {
		dbg_ClearMode(DBG_TRACE|DBG_DATA);
		printf("[Trace off]\r\n");
	} else {
		dbg_SetMode(DBG_TRACE|DBG_DATA);
		printf("[Trace on]\r\n");
	}
#endif /* DEBUG */

#ifndef HAVE_SIGACTION
	signal(SIGQUIT, emu_Trace);
#endif /* HAVE_SIGACTION */
}

void emu_BadMemory(int s)
{
	ts10_StopTimer();

#if defined(HAVE_SIGACTION) && !defined(linux)
	if (ioctl(STDIN_FILENO, I_SETSIG, 0) < 0) {
		perror("IOCTL I_SETSIG");
	}
#endif /* HAVE_SIGACTION && !linux */

	ts10_Exit("Segmentation Failure");
}

void emu_IO(int s)
{
	if (emu_IOTrap)
		emu_IOTrap();

#ifndef HAVE_SIGACTION
	signal(SIGIO,  emu_IO);
	signal(SIGURG, emu_IO);
#endif /* HAVE_SIGACTION */
}

void ts10_Exit(char *Reason)
{
	char outReason[1024];

	sprintf(outReason, "\r\nTS10 Emulator Exit (Reason: %s)\r\n", Reason);
	sock_Send(1, outReason, 0);
	sock_CloseAll(outReason);
	CleanupControlPanel();

#ifdef DEBUG
	// Flush and Close debug file.
	CloseDebug();
#endif /* DEBUG */

	exit(0);
}

int main(int argc, char **argv)
{
	int idx;

#ifdef HAVE_SIGACTION
	sigInt.sa_handler = emu_Interrupt;
	sigInt.sa_flags   = 0;
	sigaction(SIGINT, &sigInt, NULL);

	sigQuit.sa_handler = emu_Trace;
	sigQuit.sa_flags   = 0;
	sigaction(SIGQUIT, &sigQuit, NULL);

	sigSegV.sa_handler = emu_BadMemory;
	sigSegV.sa_flags   = 0;
//	sigaction(SIGSEGV, &sigSegV, NULL);

	sigIO.sa_handler = emu_IO;
	sigIO.sa_flags   = 0;
	sigaction(SIGIO, &sigIO, NULL);
	sigaction(SIGURG, &sigIO, NULL);
#else
	signal(SIGINT,  emu_Interrupt);
	signal(SIGQUIT, emu_Trace);
//	signal(SIGSEGV, emu_BadMemory);
	signal(SIGIO,   emu_IO);
	signal(SIGURG,  emu_IO);
#endif /* HAVE_SIGACTION */

	InitSystem();       // Initialize Emulator System
	InitSockets();      // Initialize Socket Handler
	InitControlPanel(); // Initialize Panel Control Handler

#ifdef DEBUG
	OpenDebug(NULL);
#endif /* DEBUG */

	emu_State = EMU_CONSOLE;

	for (idx = 1; idx < argc; idx++) {
		if (argv[idx][0] == '-') {
			switch (argv[idx][1]) {
				case 'f': // Configuration file
					ExecuteFile(argv[++idx]);
					break;

				default:
					printf("Unknown option: %s\n", argv[idx]);
			}
		}
	}
	
	// Tell operator that it is ready now.
	sock_Send(ts10_Stdout->idSocket, ts10_Prompt, 0);

	while (emu_State) {
		if (emu_State == EMU_RUN) {
			if (ts10_System) {
				DEVICE *devInfo = ts10_System->devInfo;
				if (devInfo->Execute)
					devInfo->Execute(ts10_System);
				else {
					printf("%s: Execute Not Supported.\n",
						ts10_System->devName);
				}
			} else
				printf("Please type 'select <device>' first.\n");
			emu_State = EMU_CONSOLE;
		}
		pause();
	}

	ts10_Exit("Exit");
}
