// epp_tun.c - Ethernet Protocol Program for TUN/TAP interface
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

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "epp.h"

typedef unsigned char uchar;

void Usage(char *pgmName)
{
	printf("Usage: %s <tap|tun>\n", pgmName);
}

void DumpPacket(uchar *pkt, int len)
{
	uchar ascBuffer[17];
	uchar ch, *pasc;
	int   idxAddr, idx;

	// Display Ethernet Head Information
	printf("Ethernet %02X:%02X:%02X:%02X:%02X:%02X to %02X:%02X:%02X:%02X:%02X:%02X, type %04X\n",
		pkt[6], pkt[7], pkt[8], pkt[9], pkt[10], pkt[11], // Source
		pkt[0], pkt[1], pkt[2], pkt[3], pkt[4], pkt[5],   // Destination
		(pkt[12] << 8) | pkt[13]);                        // Type

	// Display Ethernet Data Information
	for (idxAddr = 14; idxAddr < len;) {
		printf("%04X: ", idxAddr - 14);

		pasc = ascBuffer;
		for (idx = 0; (idx < 16) && (idxAddr < len); idx++) {
			ch = pkt[idxAddr++];
			printf("%c%02X", (idx == 8) ? '-' : ' ', ch);
			*pasc++ = ((ch >= 32) && (ch < 127)) ? ch : '.';
		}

		for (; idx < 16; idx++)
			printf("   ");
		*pasc = '\0';
		printf(" |%-16s|\n", ascBuffer);
	}
}

int OpenTUN(char *tunName)
{
	struct ifreq ifr; // Interface Requests
	int  tunFlags;    // Interface Flags
	int  tun;         // TUN/TAP Socket
	int  err;         // Error Flag

	if (!strcmp(tunName, "tap"))
		tunFlags = IFF_TAP;
	else if (!strcmp(tunName, "tun"))
		tunFlags = IFF_TUN;
	else
		return EPP_TUN_INVALID;

	if ((tun = open("/dev/net/tun", O_RDWR)) < 0)
		return tun;

	// Set up interface flags
	memset(&ifr, 0, sizeof(ifr));
	sprintf(ifr.ifr_name, "%s%%d", tunName);
	ifr.ifr_flags = tunFlags|IFF_NO_PI;

	// Send interface requests to TUN/TAP driver.
	if ((err = ioctl(tun, TUNSETIFF, &ifr)) < 0)
		return err;
	strcpy(tunName, ifr.ifr_name);

	return tun;
}


int main(int argc, char **argv)
{
	char tunName[40];
	int  tun;

	uchar frame[2048];
	int   len = 2048;
	int   rc;

	strcpy(tunName, argv[1]);
	tun = OpenTUN(tunName);
	switch(tun) {
		case EPP_TUN_IOERROR:
			printf("%s: Can't open %s: %s\n",
				argv[0], tunName, strerror(errno));
			return 1;

		case EPP_TUN_INVALID:
			Usage(argv[0]);
			return 2;

		default:
			printf("Successful! %s\n", tunName);
			break;
	}


	// Test TUN/TAP interface (temp)
	printf("Ok, listening...\n");
	while(1) {
		if ((rc = read(tun, frame, len)) < 0) {
			printf("Error: %s\n", strerror(errno));
			exit(1);
		}
		DumpPacket(frame, rc);
	}
}
