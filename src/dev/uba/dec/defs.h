// defs.h - I/O Device Definitions
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

#define UQ_CONT    -1  // Successful - Continue
#define UQ_OK       0  // Successful - Ok
#define UQ_ERROR    1  // Error
#define UQ_NXM      2  // Non-Existant Device/Memory
#define UQ_ADRERR   3  // Address Error
#define UQ_RESERVED 4  // Reserved Area
#define UQ_RSVDVEC  5  // Reserved Area

// Interrupt Priority Levels (BR4 to BR7) for I/O devices.
#define UQ_HLVL     4   // Number of Interrupt Levels (BR4-BR7)
#define UQ_NVECS    32  // Number of vectors per BR level.
#define UQ_BR7      7   // Interrupt Level BR7
#define UQ_BR6      6   // Interrupt Level BR6
#define UQ_BR5      5   // Interrupt Level BR5
#define UQ_BR4      4   // Interrupt Level BR4

#define ACC_BYTE    1   // Byte Access
#define ACC_WORD    2   // Word Access
#define ACC_INST    256 // Instruction Access Flag

#define BT_VALID     0x80000000  // Boot device is valid
#define BT_SUPPORTED 0x40000000  // Boot device is supported
#define BT_MODE      0x000000FF  // Boot Mode
#define BT_UNKNOWN   0           //   Unknown
#define BT_DISK      1           //   Disk
#define BT_TAPE      2           //   Tape

// WriteBlock/ReadBlock Modes.
#define QB   0x00000004 // Qbus mode
#define UB   0x00000002 // Unibus mode
#define B18  0x00000001 // 18-bit data mode (Use 'uint32' for data)
#define B16  0x00000000 // 16-bit data mode (Use 'uint8' for data)

// Common CSR bit settings
#define CSR_ERR  0100000 // Error
#define CSR_BUSY 0004000 // Busy
#define CSR_DONE 0000200 // Done
#define CSR_IE   0000100 // Interrupt Enable
#define CSR_GO   0000001 // Go Ahead

// I/O Device Mapping Table for Unibus/Q22-Bus Interface
typedef struct ioEntry      MAP_IO;
typedef struct ioCallback   UQ_CALL;
typedef struct ioBootDevice UQ_BOOT; 
typedef struct ioInterrupt  UQ_IPL;

struct ioEntry {
	MAP_IO *Next;        // Linked List
	char   *devName;     // Device Name
	char   *keyName;     // Key (Device Type) Name
	char   *emuName;     // Emulator Name
	char   *emuVersion;  // Emulator Version
	void   *uqDevice;    // Unibus/Qbus Device
	void   *Device;      // I/O Device

	// Device Settings
	uint32 idUnit;       // Interface number
	uint32 csrAddr;      // Base address
	uint32 nRegs;        // Number of Registers
	uint32 nVectors;     // Number of Vectors
	uint32 intVector[2]; // Interrupt Vector
	uint32 intLevel[2];  // Interrupt Level
	uint32 intMask[2];   // Interrupt Mask
	uint32 intIPL;       // Interrupt Priority Level (BRx)
	uint32 Priority;     // Priority (0 = Highest Priority)

	// Interrupt/Vector Functions
	void (*SetVector)(MAP_IO *, int, int);
	void (*SendInterrupt)(MAP_IO *, int);
	void (*CancelInterrupt)(MAP_IO *, int);

	// Read/Write Access Functions
	int  (*ReadIO)(void *, uint32, uint16 *, uint32);
	int  (*WriteIO)(void *, uint32, uint16, uint32);
	int  (*ReadIO32)(void *, uint32, uint32 *, uint32);
	int  (*WriteIO32)(void *, uint32, uint32, uint32);
	void (*ResetIO)(void *);
	int  (*Boot)();
};

struct ioCallback {
	// Read/Write Data I/O Functions
	uint32 (*ReadData)(void *, uint32, uint32);
	void   (*WriteData)(void *, uint32, uint32, uint32);

	// Read/Write Block I/O Functions
	uint32 (*ReadBlock)(void *, uint32, uint8 *, uint32, uint32);
	uint32 (*WriteBlock)(void *, uint32, uint8 *, uint32, uint32);
	void   *(*GetHostAddr)(void *, uint32);

	// QBA/UBA Interface Routines
	int    (*SetMap)(void *, MAP_IO *);      // SetMap Routine
	int    (*Boot)(UQ_BOOT *, int, char **); // Boot Routine
};

struct ioBootDevice {
	UNIT   Unit;      // Header information
	void   *ioDevice; // I/O Low-level Device
	uint32 Flags;     // Boot Flags
	uint32 csrAddr;   // CSR Address
	uint32 idUnit;    // Inferface Number
	uint32 idDrive;   // Drive Number

	// Function calls
	int (*Execute)(UQ_BOOT *, int, char **);
};

struct ioInterrupt {
	uint32 intRsvd;            // Reserved (Set by SetMap function call)
	uint32 intReqs;            // Requests (IRQ)
	uint32 intVecs[UQ_NVECS];  // Interrupt Vectors
	MAP_IO *ioMap[UQ_NVECS];   // I/O Mapping
};

extern DEVICE *uq_Devices[];

// External function calls
int uq_GetAddr(UNIT *, uint32 *, uint32 *, uint32 *, uint32 *, int, char **);
