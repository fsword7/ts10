/*
  socket.h - Socket definitions

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

  01/15/03  TMS  Added SCK_OWNIO that allows socket to handle its own
                 I/O processes via OwnIO routine.
  01/15/03  TMS  Modification history starts here now.

  -------------------------------------------------------------------------
*/

#ifndef _SOCKET_H
#define _SOCKET_H

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stropts.h>
#include <errno.h>

// TUN/TAP connections
#include <linux/if_tun.h>

#define NET_MAXSOCKETS  256  // Maximum number of open sockets.
#define NET_MAXBUF      2048  // Manimum number of bytes of buffer.

typedef struct sockaddr    SOCKADDR;
typedef struct sockaddr_in SOCKADDRIN;

typedef struct Socket     SOCKET;
typedef struct SocketType SOCKTYPE;

struct SocketType {
	char *Name;
	// Callback functions
	void (*Accept)(SOCKET *);
	void (*Eof)(SOCKET *, int, int);
	void (*Process)(SOCKET *, char *, int);
	void (*Status)(SOCKET *);
};

struct Socket {
	// Internal variables
	SOCKET      *Next;    // Next Socket     (Forward)
	SOCKET      *Back;    // Previous Socket (Backward)
	char        *Name;    // Socket Name
	int         idSocket; // Socket ID (FD number)
	int         Flags;    // Socket Flags
	SOCKET      *Server;  // Parent socket.
	int         maxConns; // Maximum number of opened connections.
	int         nConns;   // Current number of opened connections.
	SOCKADDRIN  locAddr;  // Local Internet Address/Port
	SOCKADDRIN  remAddr;  // Remote Internet Address/Port
	SOCKTYPE    *Type;    // Socket Type

	// User-defined variables;
	void        *Device;  // User-defined Device
	int         uPort;    // User-defined Port/Line
	void        *uData;   // User-defined Data

	// Callback functions
	void (*Accept)(SOCKET *);
	void (*Eof)(SOCKET *, int, int);
	void (*Process)(SOCKET *, char *, int);
	void (*OwnIO)(SOCKET *);
};

// Flag definitions for Socket variable
#define SCK_OPENED    0x80000000 // Socket is opened or closed.
#define SCK_SERVER    0x40000000 // Socket is server or client.
#define SCK_LISTEN    0x20000000 // Socket is listening.
#define SCK_CONNECT   0x10000000 // Socket is connecting (incoming).
#define SCK_SOCKET    0x08000000 // Socket is real socket.
#define SCK_STDIO     0x04000000 // Socket is standard I/O (TTY)
#define SCK_FILE      0x02000000 // Socket is file I/O
#define SCK_PACKET    0x01000000 // Socket is packet type.
#define SCK_OWNIO     0x00800000 // Own I/O process

// Socket mode defintions - Server, Client, or Connected.
#define NET_SERVER   1 // Socket's Server role
#define NET_CLIENT   2 // Socket's Client role
#define NET_CONNECT  3 // Incoming socket connection
#define NET_STDIO    4 // Standard I/O (TTY)
#define NET_FILE     5 // File I/O
#define NET_TUN      6 // TUN/TAP connection

// Socket error definitions
#define NET_OK            0 // Operation successful
#define NET_OPENERR       1 // Open Error
#define NET_BINDERR       2 // Bind Error
#define NET_FULLSOCKETS   3 // Sockets are full
#define NET_UNKNOWNMODE   4 // Unknown Mode
#define NET_SOCKERR       5 // Socket Error
#define NET_NOTVALID      6 // Not valid socket

// Telnet code definitions
#define TEL_SE    240 // End of subnegotiation parameters
#define TEL_NOP   241 // No operation.
#define TEL_DM    242 // Data Mark
#define TEL_BRK   243 // Break
#define TEL_IP    244 // Interrupt Process
#define TEL_AO    245 // Abort Output
#define TEL_AYT   246 // Are You There?
#define TEL_EC    247 // Erase Character
#define TEL_EL    248 // Erase Line
#define TEL_GA    249 // Go Ahead
#define TEL_SB    250 // Subnegotiation
#define TEL_WILL  251 // Will (Option code)
#define TEL_WONT  252 // Will Not (Option Code)
#define TEL_DO    253 // Do (Option Code)
#define TEL_DONT  254 // Don't (Option Code)
#define TEL_IAC   255 // Interpret as Command

// Protoype definitions
void   InitSockets(void);
void   sock_Cleanup(void);
#ifdef DEBUG
void   sock_Dump(int, uchar *, int, char *);
#endif /* DEBUG */
SOCKET *sock_Open(char *, int, int);
int    sock_Close(SOCKET *);
void   sock_CloseAll(char *);
int    sock_Listen(SOCKET *, int);
SOCKET *sock_Accept(SOCKET *);
int    sock_Send(int, char *, int);
int    sock_Print(SOCKET *, char *, int);
void   SockGetEtherAddr(char *, uint8 *);
int    SockSendPacket(SOCKET *, uint8 *, uint32);
int    SockPrintf(SOCKET *, cchar *, ...);
int    sock_ProcessTelnet(uchar *, int);
void   SocketHandler(int);
void   sock_ShowList(void);

#endif /* _SOCKET_H */
