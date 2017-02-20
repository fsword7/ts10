/*
  ether.c - Ethernet Support Routines

  Copyright (c) 2003, Timothy M. Stark

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

  01/13/03  TMS  Started Ethernet support routines.

  -------------------------------------------------------------------------
*/

#include "emu/defs.h"
#include "emu/ether.h"

ETH_TYPE EtherTypes[] = {
	{ "tun", "TAP/TUN Connection" },
	{ "tap", "TAP/TUN Connection" },
	{ NULL } // Null Terminator
};


// Get Ethernet Address in Printable Format
char *eth_FormatAddress(ETH_MAC *mac, char *str)
{
	static char tmp[18];
	char *p = str ? str : tmp;
	uint8 *m = (uint8 *)mac;

	sprintf(p, "%02X:%02X:%02X:%02X:%02X:%02X",
		m[0], m[1], m[2], m[3], m[4], m[5], m[6]);

	return p;
}

#ifdef DEBUG
void TraceEtherPacket(ETH_PACKET *pkt)
{
	char dstAddr[18];
	char srcAddr[18];
	uint32 idxAddr, idx;

	eth_FormatAddress((ETH_MAC *)&pkt->msg[0], dstAddr);
	eth_FormatAddress((ETH_MAC *)&pkt->msg[6], srcAddr);
	dbg_Printf("eth: %s to %s  Type: %04X Length: %d bytes\n",
		dstAddr, srcAddr, &pkt->msg[12], pkt->len);

	for (idxAddr = 0; idxAddr < pkt->len;) {
		uchar outDump[80], outAscii[17];
		uchar *pdump, *pasc;
		uchar ch;

		pdump = outDump;
		pasc  = outAscii;

		pdump += sprintf(pdump, "%04X: ", idxAddr);

		for (idx = 0; (idx < 16) && (idxAddr < pkt->len); idx++) {
			ch = pkt->msg[idxAddr++];
			pdump += sprintf(pdump, "%c%02X", (idx == 8 ? '-' : ' '), ch);
			*pasc++ = ((ch >= 32) && (ch < 127)) ? ch : '.';
		}

		*pdump = '\0';
		*pasc  = '\0';
	
		dbg_Printf("eth: %-64s |%-16s|\n", outDump, outAscii);
	}
}
#endif /* DEBUG */

// Open Ethernet Connection
ETH_DEVICE *OpenEther(char *name)
{
	ETH_DEVICE *eth;

	return eth;
}

// Close Ethernet Connection
void CloseEther(ETH_DEVICE *eth)
{
}
