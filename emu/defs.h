// defs.h - Definitions for the main emulator routines
//
// Copyright (c) 2000-2002, Timothy M. Stark
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

#include "emu/version.h"

typedef char               int8;
typedef short              int12;
typedef short              int16;
typedef int                int18;
typedef int                int30;
typedef int                int32;
typedef long long          int36;
typedef long long          int64;

typedef unsigned char      uint8;
typedef unsigned short     uint12;
typedef unsigned short     uint16;
typedef unsigned int       uint18;
typedef unsigned int       uint30;
typedef unsigned int       uint32;
typedef unsigned long long uint36;
typedef unsigned long long uint64;

typedef const char    cchar;
typedef unsigned char uchar;

// Pairs - little endian only at this time.
typedef union {
	struct { uint8  l, h1, h2, h3; } b; // Byte pairs
	struct { uint16 l, h; }          w; // Word pair
	uint32                           d; // Longword
} pair32;

typedef union {
	struct { uint8 l, h; } b; // Byte pairs
	uint16                 w; // Word
} pair16;

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <setjmp.h>
#include <errno.h>

#include "emu/byteswap.h"

#define TRUE  1
#define FALSE 0

#ifdef USE_ATTR_REGPARM
#define ATTR_REGPARM(n) __attribute__((regparm(n)))
#else /* USE_ATTR_REGPARM */
#define ATTR_REGPARM(n)
#endif /* USE_ATTR_REGPARM */

// Switch mask (-a to -z)
#define SWMASK(char) (1u << ((uint32)(char) - (uint32)'a'))

// Maximum number of arguments.
#define TS10_MAXARGS 256
#define TS10_CMDLEN  32

#define TS10_OK          0  // Successful Operation
#define TS10_ERROR       1  // Operation Failure - Error
#define TS10_MEMERR      EMU_MEMERR
#define TS10_OPENERR     EMU_OPENERR

#define EMU_OK           0               // Normal
#define EMU_BASE         64              // Base for Messages
#define EMU_FATAL        (EMU_BASE +  0) // Fatal Error
#define EMU_NXM          (EMU_BASE +  1) // Non-existant Memory
#define EMU_MEMERR       (EMU_BASE +  2) // Memory Error
#define EMU_OPENERR      (EMU_BASE +  3) // Open Error
#define EMU_IOERR        (EMU_BASE +  4) // I/O Error
#define EMU_PRESENT      (EMU_BASE +  5) // Unit is already present
#define EMU_NPRESENT     (EMU_BASE +  6) // Unit is not present
#define EMU_DISABLED     (EMU_BASE +  7) // Unit is disabled
#define EMU_NOATT        (EMU_BASE +  8) // Unit is not attachable
#define EMU_ATTACHED     (EMU_BASE +  9) // Unit is already attached
#define EMU_ARG          (EMU_BASE + 10) // Bad Argument
#define EMU_UNKNOWN      (EMU_BASE + 11) // Unknown Command
#define EMU_NOTFOUND     (EMU_BASE + 12) // Not Found
#define EMU_CONFLICT     (EMU_BASE + 13) // Conflict
#define EMU_NOTSUPPORTED (EMU_BASE + 14) // Not supported
#define EMU_NOTBOOTABLE  (EMU_BASE + 15) // Not bootable

#define EMU_ILR      (EMU_BASE + 128)  // Illegal Register
#define EMU_ZWC      (EMU_BASE + 129)  // Zero Word Count
#define EMU_PFAIL    (EMU_BASE + 130)  // Page Failure

#define EMU_QUIT     0
#define EMU_CONSOLE  1
#define EMU_RUN      2
#define EMU_HALT     3
#define EMU_SWHALT   4

#define EMU_MAXARGS  16

#define emu_Abort(e, x) longjmp(e, x)

extern int  emu_State;
extern int  emu_logFile;

typedef struct ClockQueue    CLK_QUEUE;
typedef struct Command       COMMAND;
typedef struct User          USER;
typedef struct Help          HELP;
typedef struct MapDevice     MAP_DEVICE;
typedef struct BreakEntry    DBG_BREAK;
typedef struct BreakSystem   DBG_BRKSYS;

// New (Official Standard for TS10 Emulator System)
typedef struct DeviceList    DEVLIST;  // Device Names (Aliases).
typedef struct DeviceInfo    DEVICE;   // Static Device
typedef struct UnitHeader    UNIT;     // Dynamic Device/Unit

// Timer Queue Table
struct ClockQueue {
	CLK_QUEUE *Next;              // Next queue
	char      *Name;              // Name of Timer
	int32     Flags;              // Flags
	int32     outTimer;           // Timeout Timer
	int32     nxtTimer;           // Next Timer if available
	void      *Device;            // User-defined device unit
	void      (*Execute)(void *); // Execute (Action) Routine
};

// Command table
struct Command {
	char  *Name;   // Name of Command
	char  *Usage;  // Usage information
	int   (*Execute)(void *, int, char **);
};

/*
// User table
struct User {
	SOCKET *io;    // Terminal I/O socket
	uint32 sw;     // Switches
	int    argc;   // Number of Arguments
	char   **argv; // Argument List
};
*/

// Help table
struct Help {
	char *Option; // Usage/Option information
	char *Desc;   // Complete description
};

// Device Information Table for I/O commands
// Listing of Device Type (Key) Names
struct DeviceList {
	uint32    idType;       // Device Type
	char      *keyName;     // Device Type (Key) Name
	char      *emuName;     // Emulator Name
	char      *emuVersion;  // Emulator Version
};

// Static Tree Data Structure.
struct DeviceInfo {
	char      *dtName;       // Device Type Name (Key)
	char      *emuName;      // Emulator Name
	char      *emuVersion;   // Version
	DEVICE    **Devices;     // Child Device Table
	uint32    Flags;         // Device Flags
	uint32    Type;          // Device Type

	COMMAND   *Commands;     // Command Table
	COMMAND   *SetCommands;  // Set Command Table
	COMMAND   *ShowCommands; // Show Command Table

	// Command Table
	void *(*Create)(MAP_DEVICE *, int, char **);
	void *(*Configure)(MAP_DEVICE *, int, char **);
	int  (*Delete)(void *);
	int  (*Reset)(void *);
	int  (*Attach)(MAP_DEVICE *, int, char **);
	int  (*Detach)(MAP_DEVICE *, int, char **);
	int  (*Info)(MAP_DEVICE *, int, char **);
	int  (*Boot)(MAP_DEVICE *, int, char **);
	int  (*Execute)(MAP_DEVICE *);
#ifdef DEBUG
	int  (*Debug)(MAP_DEVICE *, int, char **);
#endif /* DEBUG */

	DEVLIST *Names;
};

// Mapping Device List
struct MapDevice {
	// Data Structure: Family Tree
	MAP_DEVICE  *mapNext;    // Next Mapping Device
	MAP_DEVICE  *mapParent;  // Parent Mapping Device
	MAP_DEVICE  *mapChild;   // Child Mapping Devices

	// Data Structure: User-defined system.
	MAP_DEVICE  *devNext;    // Next Mapping Device
	MAP_DEVICE  *devParent;  // Parent Mapping Device
	MAP_DEVICE  *devList;    // Child Mapping Devices

	// Device Names
	char        *devName;    // Device Name
	char        *keyName;    // Key (Device Type) Name
	char        *emuName;    // Emulator Name
	char        *emuVersion; // Emulator Version

	// Device Information
	int         idUnit;      // Unit Identification
	void        *Device;     // User-defined Device
	void        *Callback;   // User-defined Callback Routines
	void        *sysDevice;  // System Device
	MAP_DEVICE  *sysMap;     // System Mapping Device
	DEVICE      *devInfo;    // Device Information (Commands)
	MAP_DEVICE  *useDevice;  // Using Current Device

#ifdef DEBUG
	// Debug Facility
	DBG_BRKSYS  *Breaks;     // Breakpoint System
#endif /* DEBUG */
};

// Unit Header (New)
struct UnitHeader {
	// Unit Description
	char   *devName;    // Device Name
	char   *keyName;    // Key (Device Type) Name
	char   *emuName;    // Emulator Name
	char   *emuVersion; // Emulator Version

	// Unit Flags
	uint32 Flags;       // Unit Flags
};

// Timer Queue flags
#define CLK_PENDING  0x80000000 // Entry is pending
#define CLK_ENABLE   0x00000002 // Entry is enabled
#define CLK_REACTIVE 0x00000001 // Entry is re-activable
#define CLK_TICK     10000      // 10ms Tick (Linux)

// Boot type for devices
#define BOOT_UNKNOWN  0
#define BOOT_DISK     1
#define BOOT_MAGTAPE  2
#define BOOT_NETWORK  3

// Device Flags
#define DF_SYSMAP    4 // Use System Map
#define DF_SELECT    2 // Set Select Device
#define DF_USE       1 // Set Using Device

// Device Type
#define DT_ANY       0
#define DT_ROOT      1
#define DT_SYSTEM    2
#define DT_PROCESSOR 3
#define DT_DEVICE    4
#define DT_NETWORK   5
#define DT_TERMINAL  6
#define DT_CLOCK     7
#define DT_VIDEO     8

// Unit type for devices
#define UT_UNKNOWN    0 // Unit is unknown
#define UT_PROCESSOR  1 // Unit is CPU/Processor
#define UT_MEMORY     2 // Unit is memory
#define UT_INTERFACE  3 // Unit is bus interface
#define UT_CONTROLLER 4 // Unit is controller
#define UT_NETWORK    5 // Unit is network
#define UT_STORAGE    6 // Unit is storage medium
#define UT_CLOCK      7 // Unit is time clock

// Unit flags for devices
#define UNIT_WRLOCKED   010000 // Device is write-locked
#define UNIT_DISABLE    004000 // Device is disable
#define UNIT_DISABLED   002000 // Device is disabled
#define UNIT_REMOVABLE  001000 // Device is removable
#define UNIT_ATTABLE    000400 // Device is attachable
#define UNIT_ATTACHED   000200 // Device is attached
#define UNIT_BUFABLE    000100 // Device is bufferable
#define UNIT_BUFREQ     000040 // Device must have buffer
#define UNIT_BUFFERED   000020 // Device is buffered
#define UNIT_FIXED      000010 // Device capacity is fixed
#define UNIT_SEQ        000004 // Device is sequential
#define UNIT_RO         000002 // Device allows read access only
#define UNIT_PRESENT    000001 // Device is present

#ifdef DEBUG

// Sorted Breakpoint System

struct BreakEntry {
	// Double-Linked List
	DBG_BREAK *Next;    // Next Node
	DBG_BREAK *Back;    // Back Node

	// Break Address Information 
	uint32    Addr;     // Break address
	uint32    Count;    // Break Countdown
	uint32    Switch;   // Switch Flag

	// Action Being Executed
	int       argc;     // Number of Arguments
	void      **argv;   // Array of Arguments (Command Line)
};

struct BreakSystem {
	uint32     Types;    // Break Types Supported
	uint32     Switch;   // Switch Summary
	DBG_BREAK  *List;    // Breakpoint List (Head end)
	DBG_BREAK  *Tail;    // Tail end of breakpoint list
};

typedef struct {
	char *Name;
	int  Mode;
} DBG_MODES;

// Debugging Facility Definitions
#define DBG_FLAG        0x80000000 // Debug Flag
#define DBG_MUUO        0400000  // Monitor UUO Instructions (PDP-10 Only)
#define DBG_SOCKERR     0200000  // Socket Errors
#define DBG_PCCHANGE    0100000  // PC Changes
#define DBG_TRANSLATION 0040000  // Page Translation
#define DBG_PAGEFAULT   0020000  // Page Faults
#define DBG_PREFETCH    0010000  // Prefetch Instruction Buffer
#define DBG_TIMER       0004000  // Timer Alarm
#define DBG_IPS         0002000  // Instructions Per Second Meter
#define DBG_SOCKETS     0001000  // Sockets
#define DBG_INTERRUPT   0000400  // Interrupt
#define DBG_CONSOLE     0000200  // I/O Console
#define DBG_IODATA      0000100  // I/O Data Transfers
#define DBG_IOREGS      0000040  // I/O Registers
#define DBG_TABLE       0000020  // Instruction Table
#define DBG_DATA        0000010  // Data Watch
#define DBG_OPERAND     0000004  // Operand Watch
#define DBG_REGISTER    0000002  // Registers Watch
#define DBG_TRACE       0000001  // Execution Trace

#endif /* DEBUG */

// Function definitions (Prototype)
#include "emu/extern.h"
#include "emu/proto.h"
