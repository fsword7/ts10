/*
  socket.c - Socket Support Routines

  Copyright (c) 2001-2003, Timothy M. Stark

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  TIMOTHY M STARK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  Except as contained in this notice, the name of Timothy M Stark shall not
  be used in advertising or otherwise to promote the sale, use or other 
  dealings in this Software without prior written authorization from
  Timothy M Stark.

  -------------------------------------------------------------------------

  Modification History:

  01/15/03  TMS  Added SCK_OWNIO option that allow any socket to handle
                 own I/O process instead of default I/O process here.
  01/15/03  TMS  Modification history starts here now.

  -------------------------------------------------------------------------
*/

#include "emu/defs.h"
#include "emu/socket.h"

static SOCKET Sockets[NET_MAXSOCKETS];

static fd_set fdsRead;
static fd_set fdsWrite;

static int sock_Error = NET_OK;

extern void (*emu_IOTrap)();

// Default unconfigured functions for each new socket.
SOCKTYPE SocketDefault;
void SocketAccept(SOCKET *);
void SocketEof(SOCKET *, int, int);
void SocketProcess(SOCKET *, char *, int);
void SocketStatus(SOCKET *);

// Initialize Socket Handler
void InitSockets(void)
{
	// Initialize select table.
	FD_ZERO(&fdsRead);
	FD_ZERO(&fdsWrite);

	// Initialize socket table.
	memset(&Sockets, 0, sizeof(SOCKET) * NET_MAXSOCKETS);

	emu_IOTrap = SocketHandler;
}

void sock_Cleanup(void)
{
	emu_IOTrap = NULL;
}

#ifdef DEBUG
void sock_Dump(int idSocket, uchar *data, int len, char *method)
{
	char cDump[17], *pcDump;
	int  idx1, idx2;
	uchar ch;

	dbg_Printf("SOCKET: Socket ID: %d  Method: %s\n",
		idSocket, method);
	dbg_Printf("SOCKET:\n");

	for (idx1 = 0; idx1 < len;) {
		dbg_Printf("SOCKET: %04X  ", idx1);
		pcDump = cDump;
		for (idx2 = 0; (idx2 < 16) && (idx1 < len); idx2++) {
			ch = data[idx1++];
			dbg_Printf("%02X%c", ch, (idx2 == 7) ? '-' : ' ');
			*pcDump++ = ((ch >= 32) && (ch < 127)) ? ch : '.';
		}
		for (; idx2 < 16; idx2++)
			dbg_Printf("   ");
		*pcDump = '\0';
		dbg_Printf(" |%-16s|\n", cDump);
	}
}
#endif /* DEBUG */

SOCKET *sock_Open(char *sockName, int newPort, int mode)
{
	SOCKET *Socket;
	SOCKADDRIN locAddr = {0};
	SOCKADDRIN remAddr = {0};
	struct ifreq ifr;
	int newSocket;
	int flags;
	int idx;
	int on = 1;

	sock_Error = NET_OK; // Assume successfull.

	// Find a free socket slot.
	Socket = NULL;
	for (idx = 0; idx < NET_MAXSOCKETS; idx++) {
		if ((Sockets[idx].Flags & SCK_OPENED) == 0) {
			Socket = &Sockets[idx];
			break;
		}
	}

	// No, all slots are occupied.
	if (Socket == NULL) {
		sock_Error = NET_FULLSOCKETS;
		return NULL;
	}

	switch (mode) {
		case NET_STDIO:
			// Extend fd to socket functions like keyboard, screen.
			newSocket = newPort;

			// Now set I/O async for socket stream.
			flags = fcntl(newSocket, F_GETFL, 0);
			fcntl(newSocket, F_SETFL, flags | FASYNC|FNDELAY);
			fcntl(newSocket, F_SETOWN, getpid());

			flags = SCK_STDIO;
			break;

		case NET_FILE:
			newSocket = newPort;
			flags     = SCK_FILE;
			break;

		case NET_TUN:
			// Make sure it is valid name as 'tun' or 'tap'. Otherwise,
			// tell operator that its name is invalid and return.
			if (!strcmp(sockName, "tap"))
				flags = IFF_TAP;
			else if (!strcmp(sockName, "tun"))
				flags = IFF_TUN;
			else {
				printf("TUN: Invalid Name - %s\n", sockName);
				sock_Error = NET_OPENERR;
				return NULL;
			}

			// Open a TUN connection for Ethernet connection.
			if ((newSocket = open("/dev/net/tun", O_RDWR)) < 0) {
				perror("TUN: Error (Open)");
				sock_Error = NET_OPENERR;
				return NULL;
			}

			// Set up interface request.
			memset(&ifr, 0, sizeof(ifr));
			sprintf(ifr.ifr_name, "%s%%d", sockName);
			ifr.ifr_flags = flags | IFF_NO_PI;

			// Send interface requests to TUN/TAP driver for
			// settings and new TUN/TAP name.
			if (ioctl(newSocket, TUNSETIFF, &ifr) < 0) {
				perror("TUN: Error (ioctl)");
				sock_Error = NET_OPENERR;
				close(newSocket);
				return NULL;
			}
			strcpy(sockName, ifr.ifr_name);

			// Now set I/O async for TUN/TAP stream.
			flags = fcntl(newSocket, F_GETFL, 0);
			fcntl(newSocket, F_SETFL, flags | FASYNC|FNDELAY);
			fcntl(newSocket, F_SETOWN, getpid());

			// Set flags for Ethernet packets.
			flags = SCK_PACKET;
			break;

		case NET_SERVER:
			// Open a socket for Internet (TCP/IP) connection.
			if ((newSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
				perror("Socket Error (Open)");
				sock_Error = NET_OPENERR;
				return NULL;
			}

			// Now set I/O async for socket stream.
			flags = fcntl(newSocket, F_GETFL, 0);
			fcntl(newSocket, F_SETFL, flags | FASYNC|FNDELAY);
			fcntl(newSocket, F_SETOWN, getpid());
			setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

			// Give socket a local name;
			locAddr.sin_family      = AF_INET;
			locAddr.sin_port        = htons(newPort);
			locAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(newSocket, (SOCKADDR *)&locAddr, sizeof(locAddr)) < 0) {
//				perror("Socket Error (Bind)");
				close(newSocket);
				sock_Error = NET_BINDERR;
				return NULL;
			}

			// Set flags for socket table.
			flags = SCK_SERVER|SCK_SOCKET;
			break;

		default:
			printf("Open: Socket error: Unknown mode\n");
			sock_Error = NET_UNKNOWNMODE;
			return NULL;
	}

	// Initialize a socket slot for new socket.
	if (sockName) {
		Socket->Name = malloc(strlen(sockName)+1);
		strcpy(Socket->Name, sockName);
	}
	Socket->idSocket = newSocket;
	Socket->locAddr  = locAddr;
	Socket->remAddr  = remAddr;
	Socket->Flags    |= (flags | SCK_OPENED); // Now occupied.

	// Set up unconfigured functions so that TS10 will not
	// crash with segmentation violation error.  Also, those
	// remind TS10 developers about that in debug log file.
	Socket->Type    = &SocketDefault;
	Socket->Accept  = SocketAccept;
	Socket->Eof     = SocketEof;
	Socket->Process = SocketProcess;

	if (mode != NET_FILE)
		FD_SET(newSocket, &fdsRead);

	return Socket;
}

int sock_Close(SOCKET *Socket)
{
	SOCKET *srvSocket = Socket->Server;
	int oldSocket     = Socket->idSocket;

	if (Socket->Flags & SCK_SOCKET)
		shutdown(oldSocket, SHUT_RDWR);
	FD_CLR(oldSocket, &fdsRead);
	FD_CLR(oldSocket, &fdsWrite);
	close(oldSocket);

	// Free a string of socket name.
	if (Socket->Name)
		free(Socket->Name);

	// Decrement a number of opened connections by one.
	if (srvSocket && (srvSocket->nConns > 0))
		srvSocket->nConns--;

	// Clear all slot.
	memset(Socket, 0, sizeof(SOCKET));
}

void sock_CloseAll(char *outReason)
{
	int idx;

	for (idx = 0; idx < NET_MAXSOCKETS; idx++) {
		if ((Sockets[idx].Flags & SCK_OPENED) &&
		    ((Sockets[idx].Flags & SCK_STDIO) == 0)) {
				if ((Sockets[idx].Flags & SCK_SERVER) == 0)
					sock_Send(Sockets[idx].idSocket, outReason, 0);
				sock_Close(&Sockets[idx]);
		}
	}
}

int sock_Listen(SOCKET *Socket, int backlog)
{
	if ((Socket->Flags & SCK_OPENED) == 0)
		return NET_NOTVALID;

	if ((backlog == 0) || (backlog > SOMAXCONN))
		backlog = SOMAXCONN;

	// Tell socket to listen incoming connections.
	if (listen(Socket->idSocket, backlog) < 0) {
		perror("Socket Error (Listen)");
		sock_Error = errno;
		return NET_SOCKERR;
	}
	Socket->Flags |= SCK_LISTEN;

	return NET_OK;
}

SOCKET *sock_Accept(SOCKET *srvSocket)
{
	SOCKET *Socket;
	SOCKADDRIN remAddr;
	uint32 lenAddr = sizeof(remAddr);
	int newSocket;
	int flags, idx;

	if (srvSocket == NULL)
		return NULL;

	// Get a new socket.
	newSocket = accept(srvSocket->idSocket, (SOCKADDR *)&remAddr, &lenAddr);
	if (newSocket < 0) {
		perror("Socket Error (Accept)");
		sock_Error = errno;
		return NULL;
	}

	// Now set I/O async for socket stream.
	flags = fcntl(newSocket, F_GETFL, 0);
	fcntl(newSocket, F_SETFL, flags | FASYNC|FNDELAY);
	fcntl(newSocket, F_SETOWN, getpid());
	FD_SET(newSocket, &fdsRead);

	// Find a empty slot for the incoming connection.
	Socket = NULL; // Assume that slots are full.
	if ((srvSocket->maxConns == 0) ||
	    (srvSocket->nConns < srvSocket->maxConns)) {
		for (idx = 0; idx < NET_MAXSOCKETS; idx++) {
			if (Sockets[idx].Flags >= 0) {
				srvSocket->nConns++;
				Socket = &Sockets[idx];
				break;
			}
		}
	}

	// If sockets are full, inform user that all circuits are busy.
	if (Socket == NULL) {
		printf("Socket Error (Accept): Sockets are full.\n");
		sock_Send(newSocket, "All connections busy. Try again later.\r\n", 0);
		sock_Send(newSocket, "\r\nTerminated.\r\n", 0);

		FD_CLR(newSocket, &fdsRead);
		close(newSocket);

		return NULL;
	}

	// Set up a new socket slot.
	Socket->Server   = srvSocket;
	Socket->idSocket = newSocket;
	Socket->Flags    = SCK_OPENED|SCK_CONNECT|SCK_SOCKET;
	Socket->locAddr  = srvSocket->locAddr;
	Socket->remAddr  = remAddr;

	return Socket;
}

int sock_Send(int idSocket, char *str, int len)
{
	if (str && *str) {
		if (len == 0)
			len = strlen(str);
#ifdef DEBUG
		if (dbg_Check(DBG_SOCKETS))
			sock_Dump(idSocket, (uchar *)str, len, "Output");
#endif /* DEBUG */
		return write(idSocket, str, len);
	}
	return 0;
}

int SockSendPacket(SOCKET *Socket, uint8 *pkt, uint32 len)
{
	if (pkt && (len > 0))
		return write(Socket->idSocket, pkt, len);
	return 0;
}

int sock_Print(SOCKET *Socket, char *str, int len)
{
	if (str && *str) {
		if (len == 0)
			len = strlen(str);
#ifdef DEBUG
		if (dbg_Check(DBG_SOCKETS))
			sock_Dump(Socket->idSocket, (uchar *)str, len, "Output");
#endif /* DEBUG */
		return write(Socket->idSocket, str, len);
	}
	return 0;
}

int SockPrintf(SOCKET *Socket, cchar *Format, ...)
{
	char tmpBuffer[1024];
	va_list Args;
	int len, sts;

	va_start(Args, Format);
	len = vsnprintf(tmpBuffer, 1023, Format, Args);
	tmpBuffer[1023] = 0;
	va_end(Args);

	// Send it away and return.
	return write(Socket->idSocket, tmpBuffer, len);
}

// Process telnet codes and filter them out of data stream.
int sock_ProcessTelnet(uchar *str, int len)
{
	int state = 0;
	int perform = 0;
	int idx1, idx2 = 0;

	for (idx1 = 0; idx1 < len; idx1++) {
		switch (state) {
			case 0:
				if (str[idx1] == TEL_IAC) {
					state++;
					break;
				}
				str[idx2++] = str[idx1];
				break;

			case 1:
				switch (str[idx1]) {
					case TEL_IAC:
						str[idx2++] = str[idx1];
						state = 0;
						break;

					case TEL_WILL:
					case TEL_WONT:
					case TEL_DO:
					case TEL_DONT:
						perform = str[idx1];
						state++;
						break;

					default:
						state = 0;
						break;
				}
				break;

			case 2:
				perform = 0;
				state = 0;
				break;
		}
	}

	return idx2;
}

// Get Current Ethernet Address
void SockGetEtherAddr(char *ifName, uint8 *ethAddr)
{
	struct ifreq ifr;
	int eth;

	if ((eth = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name));
	if (ioctl(eth, SIOCGIFHWADDR, &ifr) < 0)
		return;

	// Get Ethernet address.
	memcpy(ethAddr, &ifr.ifr_addr.sa_data[0], 6);

	// Done, close temporary socket.
	close(eth);
}

// SIGIO Handler routine to process sockets.

void SocketHandler(int sig)
{
	SOCKET *pSocket;
	uchar  inBuffer[NET_MAXBUF+1];
	int    nBytes;
	fd_set fdtRead;
	struct timeval tv = { 0, 0 };
	int    idx, reqs, maxfd = 0;

	do {
		// Get a highest file description number.
		for (idx = 0; idx < NET_MAXSOCKETS; idx++)
			if (Sockets[idx].idSocket > maxfd)
				maxfd = Sockets[idx].idSocket;

		// Find which open sockets that have requests for you.
		fdtRead = fdsRead;
		if ((reqs = select(maxfd+1, &fdtRead, NULL, NULL, &tv)) <= 0) {
#ifdef DEBUG
			if ((reqs < 0) && dbg_Check(DBG_SOCKERR))
				dbg_Printf("SCK: *** Error (select): %s\n", strerror(errno));
#endif /* DEBUG */
			return;
		}

		// Process open sockets to perform activities.
		for (idx = 0; idx < NET_MAXSOCKETS; idx++) {
			pSocket = &Sockets[idx];

			if ((pSocket->Flags & SCK_OPENED) == 0)
				continue;
			if (!FD_ISSET(pSocket->idSocket, &fdtRead))
		  		continue;
			if (pSocket->Flags & SCK_LISTEN) {
				pSocket->Accept(pSocket);
				continue;
			}
			if (pSocket->Flags & SCK_OWNIO) {
				pSocket->OwnIO(pSocket);
				continue;
			}

			// Socket successfully is connected.
			if (pSocket->Flags & SCK_CONNECT)
				pSocket->Flags &= ~SCK_CONNECT;

			// Attempt to read a packet from Internet.
			nBytes = read(pSocket->idSocket, inBuffer, NET_MAXBUF);
			if (nBytes < 0) {
				// Socket Error
				if (errno == EAGAIN)
					break;
				pSocket->Eof(pSocket, nBytes, errno);
#ifdef DEBUG
				if (dbg_Check(DBG_SOCKERR))
					dbg_Printf("SCK: *** Error (read): %s\n", strerror(errno));
#endif /* DEBUG */
			} else if (nBytes == 0) {
				// End-of-File - Close a socket normally.
//				printf("Socket %d - EOF Read\n", pSocket->idSocket);
				pSocket->Eof(pSocket, nBytes, errno);
			} else {
				// Incoming Data
#ifdef DEBUG
				if (dbg_Check(DBG_SOCKETS))
					sock_Dump(pSocket->idSocket, inBuffer, nBytes, "Input");
#endif /* DEBUG */
				pSocket->Process(pSocket, (char *)inBuffer, nBytes);
			}
		}
	} while (reqs > 0);
}

// **********************************************************

// Default unconfigured functions for new socket slots.

void SocketAccept(SOCKET *Socket)
{
#ifdef DEBUG
	if (dbg_Check(DBG_SOCKETS))
		dbg_Printf("Socket(%d): Unconfigured Accept\n", Socket->idSocket);
#endif /* DEBUG */
}

void SocketEof(SOCKET *Socket, int nBytes, int nError)
{
#ifdef DEBUG
	if (dbg_Check(DBG_SOCKETS)) {
		if (nBytes < 0)
			dbg_Printf("Socket(%d): Unconfigured Eof - %s (%d)\n",
				Socket->idSocket, strerror(nError), nError);
		else
			dbg_Printf("Socket(%d): Unconfigured Eof - Closing Socket\n",
				Socket->idSocket);
	}
#endif /* DEBUG */
}

void SocketProcess(SOCKET *Socket, char *inBuffer, int nBytes)
{
#ifdef DEBUG
	if (dbg_Check(DBG_SOCKETS))
		dbg_Printf("Socket(%d): Unconfigured Process\n", Socket->idSocket);
#endif /* DEBUG */
}

void SocketStatus(SOCKET *Socket)
{
#ifdef DEBUG
	if (dbg_Check(DBG_SOCKETS))
		dbg_Printf("Socket(%d): Unconfigured Status\n", Socket->idSocket);
#endif /* DEBUG */
}

// **********************************************************

void sock_ShowList(void)
{
	int idx;

	for (idx = 0; idx < NET_MAXSOCKETS; idx++) {
		if (Sockets[idx].Flags < 0) {
			printf("%3d %5d %08X/%-5d %08X/%-5d\n",
				idx, Sockets[idx].idSocket,
				ntohl(Sockets[idx].locAddr.sin_addr.s_addr),
				ntohs(Sockets[idx].locAddr.sin_port),
				ntohl(Sockets[idx].remAddr.sin_addr.s_addr),
				ntohs(Sockets[idx].remAddr.sin_port));
		}
	}
}

// **********************************************************

SOCKTYPE SocketDefault =
{
	"Unconfigured Socket",

	// Callback functions
	SocketAccept,            // Accept function
	SocketEof,               // End-of-file function
	SocketProcess,           // Process function
	SocketStatus             // Status function
};
