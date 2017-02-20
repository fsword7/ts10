// nexus.h - VAX Nexus Slot Routines
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
// XX/XX/XX  TMS  Comments Here
//
// -------------------------------------------------------------------------

#define NEX790_NSBI 16  // VAX-8600    - 16 Nexus Slots
#define NEX780_NSBI 16  // VAX-11/780  - 16 NEXUS Slots
#define NEX750_NSBI 16  // VAX-11/750  - 16 NEXUS Slots
#define NEX730_NSBI 16  // VAX-11/730  - 16 NEXUS Slots

// Nexus Slot
#define SBI_WIDTH  13
#define SBI_BITS   SBI_WIDTH
#define SBI_SIZE   (1u << SBI_WIDTH)
#define SBI_MASK   (SBI_SIZE - 1)

// SBI/Nexus Area (Up to 16 Nexus Slots)
#define NEX_SLOTS  16
#define NEX_WIDTH  (SBI_WIDTH + 4)
#define NEX_BITS   NEX_WIDTH
#define NEX_SIZE   (1u << NEX_WIDTH)
#define NEX_MASK   (NEX_SIZE - 1)

// NEXCSR - Configuration Register

#define NEX_CFGFLT  0xFC000000 // Configuration Fault
#define NEX_TYPE    0x000000FF // Nexus Adaptor Type

#define SBI_PARFLT  0x80000000 // SBI Parity Fault
#define SBI_WSQFLT  0x40000000 // Write Sequence Fault
#define SBI_URDFLT  0x20000000 // Unexpected Read Data Fault
#define SBI_ISQFLT  0x10000000 // Interlock Sequence Fault
#define SBI_MXTFLT  0x08000000 // Multiple Transmitter Fault
#define SBI_XMTFLT  0x04000000 // Transmit Fault

#define NEX_APD     0x00800000 // Adaptor Power Down
#define NEX_APU     0x00400000 // Adaptor Power Up

#define MBA_OT      0x00200000 // Massbus Over-Temperature

#define UBA_UBINIT  0x00040000 // Unibus Initialization
#define UBA_UBPDN   0x00020000 // Unibus Power Down
#define UBA_UBIC    0x00010000 // Unibus Initialization Complete

// NEXUS Adaptor Types

#define NEX_ANY       0
#define NEX_MEM4      0x08 // 4K Chips, Non-interleaved Memory
#define NEX_MEM4I     0x09 // 4K Chips, Interleaved Memory
#define NEX_MEM16     0x10 // 16K Chips, Non-interleaved Memory
#define NEX_MEM16I    0x11 // 16K Chips, Interleaved Memory
#define NEX_MBA       0x20 // Massbus Adaptor
#define NEX_UBA0      0x28 // Unibus Adaptor #0
#define NEX_UBA1      0x29 // Unibus Adaptor #1
#define NEX_UBA2      0x2A // Unibus Adaptor #2
#define NEX_UBA3      0x2B // Unibus Adaptor #3
#define NEX_DR32      0x30 // DR32 User Interface to SBI
#define NEX_CI        0x38 // CI Adaptor
#define NEX_MPM0      0x40 // Multi-Port Memory #0
#define NEX_MPM1      0x41 // Multi-Port Memory #1
#define NEX_MPM2      0x42 // Multi-Port Memory #2
#define NEX_MPM3      0x43 // Multi-Port Memory #3
#define NEX_MEM64L    0x68 // 64K Chips, Non-interleaved, Lower
#define NEX_MEM64LI   0x69 // 64K Chips, Ext-Interleaved, Lower
#define NEX_MEM64U    0x6A // 64K Chips, Non-Interleaved, Upper
#define NEX_MEM64UI   0x6B // 64K Chips, Ext-Interleaved, Upper
#define NEX_MEM64I    0x6C // 64K Chips, Interleaved
#define NEX_MEM256L   0x70 // 256K Chips, Non-interleaved, Lower
#define NEX_MEM256LI  0x71 // 256K Chips, Ext-Interleaved, Lower
#define NEX_MEM256U   0x72 // 256K Chips, Non-Interleaved, Upper
#define NEX_MEM256UI  0x73 // 256K Chips, Ext-Interleaved, Upper
#define NEX_MEM256I   0x74 // 256K Chips, Interleaved

typedef struct {
	char   *devName;
	char   *keyName;
	char   *emuName;
	char   *emuVersion;

	uint32 idUnit; // Unit Identification
	uint16 Type;   // Device Type
	uint32 sAddr;  // Starting Address
} NEX_DEVICE;
