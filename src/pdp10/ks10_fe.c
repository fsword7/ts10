// ks10_fe.c - Front-end 8080 communication routines for KS10 processor
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

#include "pdp10/defs.h"
#include "pdp10/ks10.h"
#include "pdp10/ks10_fe.h"
#include "emu/socket.h"
#include "emu/vdisk.h"
#include "emu/vtape.h"

#ifdef HAVE_SIGACTION
#include <sys/file.h>
#include <unistd.h>
#include <stropts.h>
#include <signal.h>
#endif /* HAVE_SIGACTION */

#if defined(linux)
#include <endian.h>
#endif /* linux */

#define CTY_PORT      5000 // Default CTY port
#define CTY_DELAY     200  // Delayed I/O Countdown
#define CTY_QDELAY    200  // Delayed Queue Countdown
#define CTY_ESCAPE    0x1C // ASCII Control-Backslash

#define KLU_PORT      5001 // Default KLU port
#define KLU_PASSWORD  "password" // KLINIK Password
#define KLU_PASSLEN   80   // Maximum Password Length
#define KLU_PASSTRY   3    // Number of tries allowed

// CTY/KLU Flag <20:27>
#define CTY_NOACT   0  // No active
#define CTY_PENDING 1  // Pending character
#define CTY_INITED  2  // Initialized/New call
#define CTY_HANGUP  2  // Hang up/Terminated
#define CTY_NOCAR   3  // Carrier Loss

#if BYTE_ORDER == LITTLE_ENDIAN
// For little endian machines
#define CTY_FLAG 1
#define CTY_CHAR 0
#else
// For big endian machines
#define CTY_FLAG 6
#define CTY_CHAR 7
#endif /* BYTE_ORDER */

struct p10_Console {
	// Simulation Timers
	CLK_QUEUE *Timer;    // Receive/Transmit Timer
	CLK_QUEUE *qTimer;   // Queue Timer

	// CTY Buffer
	uchar inBuffer[4096];
	char  outBuffer[4096];
	int   idxInQueue, idxOutQueue;
	int   idxOutBuffer;
	char  lastSeen;

	// KLINIK Buffer
	int   klu_Mode;
	int   klu_tryPassword;
	int   klu_idxPassword;
	char  klu_inPassword[KLU_PASSLEN+1];
	char  *klu_Password;

	int   klu_idxInQueue, klu_idxOutQueue;
	int   klu_idxOutBuffer;
	uchar klu_inBuffer[4096];
	char  klu_outBuffer[4096];
	char  klu_lastSeen;

	// Direct access to KS10 main memory
	// for better optimization.

	// Console TTY - Input Word
	int36 *ctyInWord;
	uchar *ctyInFlag;
	uchar *ctyInChar;

	// Console TTY - Output Word
	int36 *ctyOutWord;
	uchar *ctyOutFlag;
	uchar *ctyOutChar;

	// KLINIK - Input Word
	int36 *kluInWord;
	uchar *kluInFlag;
	uchar *kluInChar;

	// KLINIK - Output Word
	int36 *kluOutWord;
	uchar *kluOutFlag;
	uchar *kluOutChar;

	SOCKET *ctyServer; // CTY Listening Socket
	SOCKET *ctySocket; // CTY Socket
	SOCKET *kluServer; // KLINIK Listening Socket
	SOCKET *kluSocket; // KLINIK Socket
};

// Send initialization codes to make sure that telnet behave correctly.
static int  n_telnetInit = 15;
static char telnetInit[] =
{
	255, 251,  34, // IAC WILL LINEMODE
	255, 251,   3, // IAC WILL SGA
	255, 251,   1, // IAC WILL ECHO
	255, 251,   0, // IAC WILL BINARY
	255, 253,   0, // IAC DO BINARY
};

void p10_ctyAccept(SOCKET *);
void p10_ctyEof(SOCKET *, int, int);
void p10_ctyInput(SOCKET *, char *, int);

void p10_kluAccept(SOCKET *);
void p10_kluEof(SOCKET *, int, int);
void p10_kluPassword(SOCKET *, char *, int);
void p10_kluInput(SOCKET *, char *, int);

void p10_ConsoleOutput(P10_CONSOLE *);
void p10_ConsoleDone(void *);
void p10_ConsoleCheck(void *);

P10_CONSOLE *p10_InitConsole(void)
{
	P10_CONSOLE *cty;
	CLK_QUEUE   *Clock;
	SOCKET      *newSocket;

	if ((cty = (P10_CONSOLE *)calloc(1, sizeof(P10_CONSOLE))) == NULL)
		return NULL;

	// Set up receive/transmit timer
	Clock = (CLK_QUEUE *)calloc(1, sizeof(CLK_QUEUE));
	Clock->Next     = NULL;
	Clock->Flags    = 0;
	Clock->outTimer = CTY_DELAY;
	Clock->nxtTimer = CTY_DELAY;
	Clock->Device   = (void *)cty;
	Clock->Execute  = p10_ConsoleDone;
	cty->Timer      = Clock;

	// Set up queue timer
	Clock = (CLK_QUEUE *)calloc(1, sizeof(CLK_QUEUE));
	Clock->Next     = NULL;
	Clock->Flags    = 0;
	Clock->outTimer = CTY_QDELAY;
	Clock->nxtTimer = CTY_QDELAY;
	Clock->Device   = (void *)cty;
	Clock->Execute  = p10_ConsoleCheck;
	cty->qTimer     = Clock;

	// Set up the listing socket for CTY device.
	newSocket = sock_Open("(ks10/cty)", CTY_PORT, NET_SERVER);
	if (newSocket != NULL) {
		newSocket->maxConns = 1;
		newSocket->nConns   = 0;
		newSocket->Accept   = p10_ctyAccept;
		newSocket->Eof      = NULL;
		newSocket->Process  = NULL;
		newSocket->Device   = (void *)cty;
		cty->ctyServer      = newSocket;

		// Now accept incoming connections;
		sock_Listen(newSocket, 5);

		// Tell operator that.
		printf("Activating console on TCP port %d.\n", CTY_PORT);
	}

	// Set up the listening socket for KLINIK device.
	newSocket = sock_Open("(ks10/klu)", KLU_PORT, NET_SERVER);
	if (newSocket != NULL) {
		newSocket->maxConns = 1;
		newSocket->nConns   = 0;
		newSocket->Accept   = p10_kluAccept;
		newSocket->Eof      = NULL;
		newSocket->Process  = NULL;
		newSocket->Device   = (void *)cty;
		cty->kluServer      = newSocket;

		cty->klu_Password   = KLU_PASSWORD;
		cty->klu_Mode       = 0; // Status: Not Connected

		// Now accept incoming connections;
		sock_Listen(newSocket, 5);

		// Tell operator that.
		printf("Activating KLINIK on TCP port %d.\n", KLU_PORT);
	}

	// Set up pointers from KS10 main memory
	// for optimization.

	// Set up CTYIWD/CTYOWD pointers
	cty->ctyInWord  = p10_pAccess(FE_CTYIWD);
	cty->ctyInFlag  = &((uchar *)cty->ctyInWord)[CTY_FLAG];
	cty->ctyInChar  = &((uchar *)cty->ctyInWord)[CTY_CHAR];

	cty->ctyOutWord = p10_pAccess(FE_CTYOWD);
	cty->ctyOutFlag = &((uchar *)cty->ctyOutWord)[CTY_FLAG];
	cty->ctyOutChar = &((uchar *)cty->ctyOutWord)[CTY_CHAR];

	// Set up KLUIWD/KLUOWD pointers
	cty->kluInWord  = p10_pAccess(FE_KLUIWD);
	cty->kluInFlag  = &((uchar *)cty->kluInWord)[CTY_FLAG];
	cty->kluInChar  = &((uchar *)cty->kluInWord)[CTY_CHAR];

	cty->kluOutWord = p10_pAccess(FE_KLUOWD);
	cty->kluOutFlag = &((uchar *)cty->kluOutWord)[CTY_FLAG];
	cty->kluOutChar = &((uchar *)cty->kluOutWord)[CTY_CHAR];

	return cty;
}

void p10_ConsoleCleanup(P10_CONSOLE *cty)
{
	sock_Close(cty->ctySocket);
	sock_Close(cty->ctyServer);
	sock_Close(cty->kluSocket);
	sock_Close(cty->kluServer);

	cty->ctySocket = NULL;
	cty->ctyServer = NULL;
	cty->kluSocket = NULL;
	cty->kluServer = NULL;
}

void p10_ConsoleDone(void *dev)
{
	P10_CONSOLE *cty = (P10_CONSOLE *)dev;

	// Send a done interrupt to the KS10 Processor.
	p10_aprInterrupt(APRSR_F_CON_INT);
}

void p10_ctyAccept(SOCKET *srvSocket)
{
	P10_CONSOLE *cty = (P10_CONSOLE *)srvSocket->Device;
	SOCKET      *newSocket;

	if (newSocket = sock_Accept(srvSocket)) {
		// First, check if CTY connection already was taken.
		// If so, tell operator that.
		if (cty->ctySocket != NULL) {
			sock_Print(newSocket, "Console (CTY) connection already was taken.\r\n", 0);
			sock_Print(newSocket, "Check other terminal which has that connection.\r\n", 0);
			sock_Print(newSocket, "\r\nTerminated.\r\n", 0);
			sock_Close(newSocket);

			return;
		}
 
		// Set up the CTY socket connection.
		newSocket->Accept  = NULL;
		newSocket->Eof     = p10_ctyEof;
		newSocket->Process = p10_ctyInput;
		newSocket->Device  = (void *)cty;
		cty->ctySocket     = newSocket;

		// Reset all buffer for this new connection.
		cty->idxInQueue   = 0;
		cty->idxOutQueue  = 0;
		cty->idxOutBuffer = 0;

		// Send initialization codes and welcome messages
		sock_Print(newSocket, telnetInit, n_telnetInit);
		sock_Print(newSocket, "Welcome to KS10 Emulator\r\n\r\n", 0);

		// Check if output buffer is waiting for terminal ready.
		if (*cty->ctyOutWord)
			p10_ConsoleOutput(cty);
	}
}

void p10_ctyEof(SOCKET *Socket, int rc, int nError)
{
	P10_CONSOLE *cty = (P10_CONSOLE *)Socket->Device;

	sock_Close(Socket);
	cty->ctySocket = NULL;
}

void p10_ctyInput(SOCKET *Socket, char *keyBuffer, int len)
{
	P10_CONSOLE *cty   = (P10_CONSOLE *)Socket->Device;
	int         okSend = FALSE;
	int         idx;

	// Process telnet codes and filter them out of data stream.
	if (len > 1) {
		if ((len = sock_ProcessTelnet((uchar *)keyBuffer, len)) == 0)
			return;
	}

	for (idx = 0; idx < len; idx++) {
		uchar ch = keyBuffer[idx];

		// Press ^\ twice to disconnect.
		if (ch == CTY_ESCAPE) {
			if (cty->lastSeen == CTY_ESCAPE) {
				p10_ctyEof(Socket, 0, 0);
				return;
			}
			cty->lastSeen = ch;
			continue;
		}

		// Convert CR NL to CR line.
		if ((ch == 012) && (cty->lastSeen == 015))
			continue;
		cty->lastSeen = ch;

		okSend = TRUE;
		cty->inBuffer[cty->idxInQueue] = ch;
		if (++cty->idxInQueue == 4096)
			cty->idxInQueue = 0;
		if (cty->idxInQueue == cty->idxOutQueue) {
#ifdef DEBUG
			dbg_Printf("CTY: Error - Overrun!!\n");
#endif /* DEBUG */
			break;
		}
	}

	if (okSend && (*cty->ctyInFlag == 0)) {
		*cty->ctyInChar = cty->inBuffer[cty->idxOutQueue];
		*cty->ctyInFlag = 1;
		if (++cty->idxOutQueue == 4096)
			cty->idxOutQueue = 0;
		p10_aprInterrupt(APRSR_F_CON_INT);

		if (cty->idxInQueue != cty->idxOutQueue)
			ts10_SetTimer(cty->qTimer);
	}
}

void p10_kluAccept(SOCKET *srvSocket)
{
	P10_CONSOLE *cty = (P10_CONSOLE *)srvSocket->Device;
	SOCKET      *newSocket;

	if (newSocket = sock_Accept(srvSocket)) {
		// First, check if KLU connection already was taken.
		// If so, tell operator that.
		if ((cty->kluSocket != NULL) || (cty->klu_Mode > 0)) {

			sock_Print(newSocket, "Console (KLU) connection already was taken.\r\n", 0);
			sock_Print(newSocket, "Check other terminal which has that connection.\r\n", 0);
			sock_Print(newSocket, "\r\nTerminated.\r\n", 0);
			sock_Close(newSocket);

			return;
		}
		cty->klu_Mode = 1; // State: Password Prompt
 
		// Set up the CTY socket connection.
		newSocket->Accept  = NULL;
		newSocket->Eof     = p10_kluEof;
		newSocket->Process = p10_kluPassword;
		newSocket->Device  = (void *)cty;

		// Send initialization codes and welcome messages
		sock_Print(newSocket, telnetInit, n_telnetInit);
		sock_Print(newSocket, "Welcome to KS10 Emulator\r\n\r\n", 0);

		if (cty->klu_Password == NULL) {
			sock_Print(newSocket, "No Access. Terminated.\r\n", 0);
			p10_kluEof(newSocket, 0, 0);
			return;
		}

		sock_Print(newSocket, "Password: ", 0);

		cty->klu_idxPassword = 0;
		cty->klu_tryPassword = 0;

#ifdef DEBUG
		if (dbg_Check(DBG_CONSOLE)) {
			dbg_Printf("KLU: Opened on <date/time>\n");
		}
#endif /* DEBUG */
	}
}

void p10_kluEof(SOCKET *Socket, int rc, int nError)
{
	P10_CONSOLE *cty = (P10_CONSOLE *)Socket->Device;

	// Send a CARRIER LOSS signal to KS10 Processor.
	// Let system know a carrier loss signal on KLINIK
	// connection.

	if (cty->klu_Mode >= 2) {
		*cty->kluInFlag = CTY_NOCAR;
		*cty->kluInChar = 0;
		p10_aprInterrupt(APRSR_F_CON_INT);
	}

	sock_Close(Socket);

	cty->klu_Mode  = 0;
	cty->kluSocket = NULL;

#ifdef DEBUG
	if (dbg_Check(DBG_CONSOLE)) {
		if (rc < 0) {
			dbg_Printf("KLU: Socket Error: %s\n", strerror(nError));
			dbg_Printf("KLU: *** Carrier Loss ***\n");
		}
		dbg_Printf("KLU: Closed on <date/time>\n");
	}
#endif /* DEBUG */
}

void p10_kluPassword(SOCKET *Socket, char *keyBuffer, int len)
{
	P10_CONSOLE *cty = (P10_CONSOLE *)Socket->Device;
	int         idx;

	// Process telnet codes and filter them out of data stream.
	if (len > 1) {
		if ((len = sock_ProcessTelnet((uchar *)keyBuffer, len)) == 0)
			return;
	}

	for (idx = 0; idx < len; idx++) {
		uchar ch = keyBuffer[idx];

		// Press ^\ twice to disconnect.
		if (ch == CTY_ESCAPE) {
			if (cty->klu_lastSeen == CTY_ESCAPE) {
				p10_kluEof(Socket, 0, 0);
				return;
			}
			cty->klu_lastSeen = ch;
			continue;
		}

		// Convert CR NL to CR line.
		if ((ch == 012) && (cty->klu_lastSeen == 015))
			continue;
		cty->klu_lastSeen = ch;

		// Now check a password.
		if (ch == 015) {
			cty->klu_inPassword[cty->klu_idxPassword] = '\0';
			if (strcmp(cty->klu_inPassword, cty->klu_Password)) {
				// Sorry, wrong password.
				if (++cty->klu_tryPassword == KLU_PASSTRY) {
#ifdef DEBUG
					if (dbg_Check(DBG_CONSOLE))
						dbg_Printf("KLU: *** Wrong Password! Access Denied. ***\n");
#endif /* DEBUG */
					sock_Print(Socket, "Invalid Password.\r\n", 0);
					p10_kluEof(Socket, 0, 0);
					return;
				}
#ifdef DEBUG
				if (dbg_Check(DBG_CONSOLE))
					dbg_Printf("KLU: *** Wrong Password! Try Again. ***\n");
#endif /* DEBUG */
				sock_Print(Socket, "Invalid Password.\r\n", 0);
				sock_Print(Socket, "Password: ", 0);
				cty->klu_idxPassword = 0;
				return;
			}
			sock_Print(Socket, "Password accepted.\r\n", 0);
			break;
		}

		if (cty->klu_idxPassword < KLU_PASSLEN)
			cty->klu_inPassword[cty->klu_idxPassword++] = ch;
		return;
	}

	// Reset all buffer for this new connection.
	cty->klu_idxInQueue   = 0;
	cty->klu_idxOutQueue  = 0;
	cty->klu_idxOutBuffer = 0;
	cty->klu_Mode         = 2; // State: Connected.

	cty->kluSocket = Socket;
	cty->kluSocket->Process = p10_kluInput;

	// Check if KLINIK buffer is waiting for terminal ready.
	if (*cty->kluOutWord)
		p10_ConsoleOutput(cty);

	// Let system know that KLINIK is inited.
	*cty->kluInFlag = CTY_INITED;
	*cty->kluInChar = 0;
	p10_aprInterrupt(APRSR_F_CON_INT);
}

void p10_kluInput(SOCKET *Socket, char *keyBuffer, int len)
{
	P10_CONSOLE *cty = (P10_CONSOLE *)Socket->Device;
	int         okSend = FALSE;
	int         idx;

	// Process telnet codes and filter them out of data stream.
	if (len > 1) {
		if ((len = sock_ProcessTelnet((uchar *)keyBuffer, len)) == 0)
			return;
	}

	for (idx = 0; idx < len; idx++) {
		uchar ch = keyBuffer[idx];

		// Press ^\ twice to disconnect.
		if (ch == CTY_ESCAPE) {
			if (cty->klu_lastSeen == CTY_ESCAPE) {
				p10_kluEof(cty->kluSocket, 0, 0);
				return;
			}
			cty->klu_lastSeen = ch;
			continue;
		}

		// Convert CR NL to CR line.
		if ((ch == 012) && (cty->klu_lastSeen == 015))
			continue;
		cty->klu_lastSeen = ch;

		okSend = TRUE;
		cty->klu_inBuffer[cty->klu_idxInQueue] = ch;
		if (++cty->klu_idxInQueue == 4096)
			cty->klu_idxInQueue = 0;
		if (cty->klu_idxInQueue == cty->klu_idxOutQueue) {
#ifdef DEBUG
			dbg_Printf("CTY: Error - Overrun!!\n");
#endif /* DEBUG */
			break;
		}
	}

	if (okSend && ((*cty->kluInFlag == CTY_NOACT) ||
	    (*cty->kluInFlag == CTY_INITED))) {
		*cty->kluInChar = cty->klu_inBuffer[cty->klu_idxOutQueue];
		*cty->kluInFlag = 1;
		if (++cty->klu_idxOutQueue == 4096)
			cty->klu_idxOutQueue = 0;
		p10_aprInterrupt(APRSR_F_CON_INT);

		if (cty->klu_idxInQueue != cty->klu_idxOutQueue)
			ts10_SetTimer(cty->qTimer);
	}
}

void p10_ConsoleCheck(void *dev)
{
	P10_CONSOLE *cty = (P10_CONSOLE *)dev;
	int         pi;

	// Update CTYIWD queue.
	if (cty->idxOutQueue != cty->idxInQueue) {
		if (*cty->ctyInFlag == CTY_NOACT) {
			*cty->ctyInChar = cty->inBuffer[cty->idxOutQueue];
			*cty->ctyInFlag = CTY_PENDING;
			if (++cty->idxOutQueue == 4096)
				cty->idxOutQueue = 0;
			p10_aprInterrupt(APRSR_F_CON_INT);
		}
		if (cty->ctySocket)
			p10_ConsoleOutput(cty);
	}

	// Update KLUIWD queue.
	if (cty->klu_idxOutQueue != cty->klu_idxInQueue) {
		if (*cty->kluInFlag == CTY_NOACT) {
			*cty->kluInChar = cty->klu_inBuffer[cty->klu_idxOutQueue];
			*cty->kluInFlag = CTY_PENDING;
			if (++cty->klu_idxOutQueue == 4096)
				cty->klu_idxOutQueue = 0;
			p10_aprInterrupt(APRSR_F_CON_INT);
		}
		if (cty->kluSocket)
			p10_ConsoleOutput(cty);
	}

	// Reactivate queue timer if any characters are remaining
	// in either input queues. (CTY and KLU)
	if ((cty->idxOutQueue != cty->idxInQueue) ||
	    (cty->klu_idxOutQueue != cty->klu_idxInQueue))
		ts10_SetTimer(cty->qTimer);
}

void p10_ConsoleOutput(P10_CONSOLE *cty)
{
	char ch;

	if (cty->ctySocket && *cty->ctyOutWord) {
		if (*cty->ctyOutFlag == CTY_PENDING) {
			ch = *cty->ctyOutChar & 0177;
			sock_Print(cty->ctySocket, &ch, 1);

			// Log a character into a log file.
			if (ch == '\n') {
				cty->outBuffer[cty->idxOutBuffer++] = ch;
				cty->outBuffer[cty->idxOutBuffer++] = '\0';
#ifdef DEBUG
				if (dbg_Check(DBG_CONSOLE))
					dbg_Printf("CTY: %s", cty->outBuffer);
#endif /* DEBUG */
				if (emu_logFile >= 0)
					write(emu_logFile, cty->outBuffer, cty->idxOutBuffer);
				cty->idxOutBuffer = 0;
			} else {
				if (ch == '\b' || ch == 127) {
					if (cty->idxOutBuffer > 0)
						cty->idxOutBuffer--;
				} else if (ch != '\r' && ch != '\0') {
					if (cty->idxOutBuffer < 4095)
						cty->outBuffer[cty->idxOutBuffer++] = ch;
				}
			}

			*cty->ctyOutWord = 0;
			ts10_SetTimer(cty->Timer);
		}
	}

	// Write a CTY character to the terminal
	if (cty->kluSocket && *cty->kluOutWord) {
		switch (*cty->kluOutFlag) {
			case CTY_PENDING:
				// Print a character on terminal
				ch = *cty->kluOutChar & 0177;
				sock_Print(cty->kluSocket, &ch, 1);

				// Log a character into a log file.
				if (ch == '\n') {
					cty->klu_outBuffer[cty->klu_idxOutBuffer++] = ch;
					cty->klu_outBuffer[cty->klu_idxOutBuffer++] = '\0';
#ifdef DEBUG
					if (dbg_Check(DBG_CONSOLE))
						dbg_Printf("KLU: %s", cty->klu_outBuffer);
#endif /* DEBUG */
					if (emu_logFile >= 0)
						write(emu_logFile, cty->klu_outBuffer, cty->klu_idxOutBuffer);
					cty->klu_idxOutBuffer = 0;
				} else {
					if (ch == '\b' || ch == 127) {
						if (cty->klu_idxOutBuffer > 0)
							cty->klu_idxOutBuffer--;
					} else if (ch != '\r' && ch != '\0') {
						if (cty->klu_idxOutBuffer < 4095)
							cty->klu_outBuffer[cty->klu_idxOutBuffer++] = ch;
					}
				}
				break;

			case CTY_HANGUP:
				// Send a carrier loss to KS10 Processor
#ifdef DEBUG
				if (dbg_Check(DBG_CONSOLE))
					dbg_Printf("KLU: *** Hangup Request ***\n");
#endif /* DEBUG */
				// Disconnected now.
				p10_kluEof(cty->kluSocket, 0, 0);
				break;

			default:
#ifdef DEBUG
				if (dbg_Check(DBG_CONSOLE))
					dbg_Printf("KLU: Unknown flag = %03o\n", *cty->kluOutFlag);
#endif /* DEBUG */
		}

		*cty->kluOutWord = 0;
		ts10_SetTimer(cty->Timer);
	}
}

// ********************************************************************

int ks10_BootTape(UQ_BOOT *bt, int argc, char **argv)
{
	VMT_TAPE   *vmt = (VMT_TAPE *)bt->ioDevice;
	VMT_FORMAT *fmt = vmt->Format;
	uint8      bufData[32768];
	uint18     pAddr;
	uint36     data36;
	int        rqBoot = 1;
	int        file, count;
	int        idx, rc;

	printf("Booting %s...\n", vmt->fileName);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("BOOT: Boot Device CSR address %o,,%06o, Drive %d, Slave %d.\n",
			bt->idUnit, bt->csrAddr, (bt->idDrive & 0xFF),
			(bt->idDrive >> 8));
	}
#endif /* DEBUG */

	// Must rewind a tape to recalibrate
	// position to the bottom of tape.
	fmt->Rewind(vmt);
#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("BOOT: Rewind\n");
#endif /* DEBUG */

	// Move to a file from beginning
	// includes a microcode file.
	for (file = 0; file < rqBoot; file++) {
		count = 0;
		while ((rc = fmt->Skip(vmt, 1)) == MT_OK)
			count++;
#ifdef DEBUG
		if (dbg_Check(DBG_IODATA) && count)
			dbg_Printf("BOOT: File %d - Skip %d records.\n", file, count);
#endif /* DEBUG */
	}
	
	// Load first RDI block into KS10
	// memory starting location 1000.
	if ((rc = fmt->Read(vmt, bufData, 32768)) > 0) {
#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("BOOT: File %d - Read %d bytes from tape.\n", file, rc);
#endif /* DEBUG */

		// Convert 40-bit tape words to native words.
		pAddr = 01000;
		for (idx = 0; idx < rc; idx += 5) {
			data36 = ((uint36)bufData[idx] << 28) | (bufData[idx+1] << 20) |
				      (bufData[idx+2] << 12)       | (bufData[idx+3] << 4)  |
				      (bufData[idx+4] & 017);
#ifdef DEBUG
			if (dbg_Check(DBG_IODATA))
				dbg_Printf("BOOT:   %06o <= %s\n", pAddr, pdp10_DisplayData(data36));
#endif /* DEBUG */
			p10_pWrite(pAddr++, data36, 0);
		}
		
#ifdef DEBUG
		if (dbg_Check(DBG_IODATA))
			dbg_Printf("BOOT: RDI file had been loaded successfully.\n");
#endif /* DEBUG */

		// Set boot device information for RDI loader.
		p10_pWrite(FE_BRH11BA, ((bt->idUnit << 18) | bt->csrAddr), 0);
		p10_pWrite(FE_BDRVNUM, (bt->idDrive & 0xFF), 0);
		p10_pWrite(FE_MTBFSN,  (bt->idDrive >> 8), 0);
		
		// Set PC to execute the RDI loader.
		PC = 01000;

		printf("Running...\n");
		emu_State = EMU_RUN;
	
		return EMU_OK;
	}
}

int ks10_BootDisk(UQ_BOOT *bt, int argc, char **argv)
{
	VDK_DISK *vdk   = (VDK_DISK *)bt->ioDevice;
	int      rqBoot = 2; // Monitor Preboot Page
	uint30   pAddr;
	uint18   inBlock[1024];
	uint36   idHome = PackedASCII6("HOM");
	uint36   dskAddr36;
	uint32   dskCylinder, dskTrack, dskSector, dskAddr;
	int      idx;

	printf("Booting %s...\n", vdk->fileName);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA))
		dbg_Printf("BOOT: Boot Device CSR address %o,,%06o, Drive %d.\n",
			bt->idUnit, bt->csrAddr, bt->idDrive);
#endif /* DEBUG */

	// Read first HOME block (logical block #1)
	printf("Reading first HOME block...\n");
	vdk_SeekDisk(vdk, 1);
	vdk_ReadDisk(vdk, (uint8 *)inBlock, VDK_18B);
#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("BOOT: First Home Block.\n");
		for (idx = 0, pAddr = 0; idx < 256; idx += 2)
			dbg_Printf("BOOT:   %06o: %s\n",
				pAddr++, pdp10_DisplayData(XWD36(inBlock[idx], inBlock[idx+1])));
	}
#endif /* DEBUG */

	if (XWD36(inBlock[0], inBlock[1]) != idHome) {
		// Try second HOME block (logical block #10)
		printf("Reading second HOME block...\n");
		vdk_SeekDisk(vdk, 10);
		vdk_ReadDisk(vdk, (uint8 *)inBlock, VDK_18B);
#ifdef DEBUG
		if (dbg_Check(DBG_IODATA)) {
			dbg_Printf("BOOT: Second Home Block.\n");
			for (idx = 0, pAddr = 0; idx < 256; idx += 2)
				dbg_Printf("BOOT:   %06o: %s\n",
					pAddr++, pdp10_DisplayData(XWD36(inBlock[idx], inBlock[idx+1])));
		}
#endif /* DEBUG */
		if (XWD36(inBlock[0], inBlock[1]) != idHome) {
			printf("Both HOME blocks not found - aborted boot.\n");
			return EMU_OK;
		}
	}
	
	// Good HOME block found, check if disk is bootable...
	dskAddr36 = XWD36(inBlock[FE_BT_8080*2], inBlock[(FE_BT_8080*2)+1]);
	if (dskAddr36 == 0) {
		printf("Disk %s is not bootable - aborted boot.\n", vdk->fileName);
		return EMU_OK;
	}

	// Extract cylinder, track, and sector fields
	// from disk address word.
	dskCylinder = FE_DA_CYL(dskAddr36);
	dskTrack    = FE_DA_TRK(dskAddr36);
	dskSector   = FE_DA_SEC(dskAddr36);
	dskAddr     = vdk_GetDiskAddr(vdk, dskCylinder, dskTrack, dskSector);

	// Read FE-FILE page 0 (4 blocks).
	printf("Reading FE-FILE Page 0 at block %d (Cyl %d Trk %d Sec %d)...\n",
		dskAddr, dskCylinder, dskTrack, dskSector);

	vdk_SeekDisk(vdk, dskAddr);
	for (idx = 0; idx < 4; idx++)
		vdk_ReadDisk(vdk, (uint8 *)&inBlock[idx * 256], VDK_18B);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("BOOT: FE-FILE Page 0 at block %d (Cyl %d Trk %d Sec %d).\n",
			dskAddr, dskCylinder, dskTrack, dskSector);
		for (idx = 0, pAddr = 0; idx < 1024; idx += 2)
			dbg_Printf("BOOT:   %06o: %s\n",
				pAddr++, pdp10_DisplayData(XWD36(inBlock[idx], inBlock[idx+1])));
	}
#endif /* DEBUG */

	// Now get the pre-boot loader page from disk.
	rqBoot <<= 2;
	if ((dskAddr36 = XWD36(inBlock[rqBoot], inBlock[rqBoot+1])) == 0) {
		printf("Pre-boot loader not available - Boot aborted.\n");
		return EMU_NOTBOOTABLE;
	}

	// Extract cylinder, track, and sector fields
	// from disk address word.
	dskCylinder = FE_DA_CYL(dskAddr36);
	dskTrack    = FE_DA_TRK(dskAddr36);
	dskSector   = FE_DA_SEC(dskAddr36);
	dskAddr     = vdk_GetDiskAddr(vdk, dskCylinder, dskTrack, dskSector);
		
	// Read pre-boot loader (4 blocks)
	printf("Reading Pre-boot loader at block %d (Cyl %d Trk %d Sec %d)...\n",
		dskAddr, dskCylinder, dskTrack, dskSector);

	vdk_SeekDisk(vdk, dskAddr);
	for (idx = 0; idx < 4; idx++)
		vdk_ReadDisk(vdk, (uint8 *)&inBlock[idx * 256], VDK_18B);

#ifdef DEBUG
	if (dbg_Check(DBG_IODATA)) {
		dbg_Printf("BOOT: RDI Pre-boot Loader at block %d (Cyl %d Trk %d Sec %d).\n",
			dskAddr, dskCylinder, dskTrack, dskSector);
		for (idx = 0, pAddr = 0; idx < 1024; idx += 2)
			dbg_Printf("BOOT:   %06o: %s\n",
				pAddr++, pdp10_DisplayData(XWD36(inBlock[idx], inBlock[idx+1])));
	}
#endif /* DEBUG */

	// Now load the RDI pre-boot loader into location 1000;
	for (idx = 0, pAddr = 01000; idx < 0777; idx += 2)
		p10_pWrite(pAddr++, XWD36(inBlock[idx], inBlock[idx+1]), 0);

	// Finally, transfer its control to the RDI pre-boot loader to
	// continue booting system.
	p10_pWrite(FE_BRH11BA, ((bt->idUnit << 18) | bt->csrAddr), 0);
	p10_pWrite(FE_BDRVNUM, bt->idDrive, 0);
	PC = 01000;

	printf("Running...\n");
	emu_State = EMU_RUN;
	
	return EMU_OK;
}

int ks10_BootDevice(UQ_BOOT *bt, int argc, char **argv)
{
	switch (bt->Flags & BT_MODE) {
		case BT_TAPE:
			return ks10_BootTape(bt, argc, argv);
		case BT_DISK:
			return ks10_BootDisk(bt, argc, argv);
	}
	return EMU_NOTSUPPORTED;
}
