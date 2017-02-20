// panel.c - Panel Control routines (screen area)
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

#include <termios.h>
#include <unistd.h>
#include "emu/defs.h"
#include "emu/socket.h"

#define Stdin  STDIN_FILENO
#define Stdout STDOUT_FILENO
#define Stderr STDERR_FILENO

int    ts10_pkbBuffer;
char   ts10_kbBuffer[256];
char   *ts10_Prompt;
SOCKET *ts10_Stdin;
SOCKET *ts10_Stdout;
SOCKET *ts10_Stderr;

void panel_Input(SOCKET *, char *, int);
void panel_Eof(SOCKET *, int, int);

// Initialize Panel Control Handler
void InitControlPanel(void)
{
	struct termios kbAttr;

	ts10_Prompt    = "TS10> ";
	ts10_pkbBuffer = 0;

	// Set up all standard I/O on socket slots.
	if (ts10_Stdin = sock_Open("<stdin>", Stdin, NET_STDIO)) {
		ts10_Stdin->Eof     = panel_Eof;
		ts10_Stdin->Process = panel_Input;

		// Turn edit and echo mode off for keyboard.
		tcgetattr(Stdin, &kbAttr);
		kbAttr.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(Stdin, TCSAFLUSH, &kbAttr);
	}

	ts10_Stdout = sock_Open("<stdout>", Stdout, NET_STDIO);
	ts10_Stderr = sock_Open("<stderr>", Stderr, NET_STDIO);
}

void CleanupControlPanel(void)
{
	struct termios kbAttr;

	tcgetattr(Stdin, &kbAttr);
	kbAttr.c_lflag |= (ICANON | ECHO);
	tcsetattr(Stdin, TCSANOW, &kbAttr);
}

void panel_Eof(SOCKET *Socket, int nBytes, int nError)
{
	// Do nothing
}

void panel_Input(SOCKET *Socket, char *inBuffer, int lenBuffer)
{
	int idx;

	for (idx = 0; idx < lenBuffer; idx++) {
		switch(inBuffer[idx]) {
			case '\b':   // Backspace key (ASCII BS)
			case '\x7F': // Delete key (ASCII DEL)
				if (ts10_pkbBuffer > 0) {
					ts10_pkbBuffer--;
					sock_Send(ts10_Stdout->idSocket, "\b \b", 3);
				}
				break;

			case '\n': // Linefeed Key (ASCII LF)
				sock_Send(ts10_Stdout->idSocket, "\r\n", 2);
				ts10_kbBuffer[ts10_pkbBuffer] = '\0';
				ExecutePrompt(ts10_kbBuffer);
				ts10_pkbBuffer = 0;
				sock_Send(ts10_Stdout->idSocket, ts10_Prompt, 0);
				break;

			case '?': // List current commands
				if (ts10_pkbBuffer == 0) {
					ListAllCommands();
					sock_Send(ts10_Stdout->idSocket, ts10_Prompt, 0);
					break;
				}

			default: // Append any character to keyboard buffer.
				if (ts10_pkbBuffer < 255) {
					ts10_kbBuffer[ts10_pkbBuffer++] = inBuffer[idx];
					sock_Send(ts10_Stdout->idSocket, &inBuffer[idx], 1);
				}
		}
	}
}

