/*
  ether.h - Ethernet Definition

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

#define ETH_MIN  60    // Minimum Ethernet Packet Size
#define ETH_MAX  1536  // Maximum Ethernet Packet Size
#define ETH_SIZE ETH_MAX

typedef uint8 ETH_MAC[6];              // MAC Address (6 bytes)
typedef struct EtherPacket ETH_PACKET; // Ethernet Packet Message
typedef struct EtherType   ETH_TYPE;   // Ethernet Type
typedef struct EtherDevice ETH_DEVICE; // Ethernet Device

struct EtherPacket {
	uint32 len;
	uint8  msg[ETH_MAX];
};

struct EtherType {
	char   *name;   // Ethernet Type Name
	char   *desc;   //   Description
};

struct EtherDevice {
};
