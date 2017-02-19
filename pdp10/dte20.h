// dte20.h - DTE-20 Ten-Eleven Interface Definitions
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

#include "pdp10/iodefs.h"

#define DTE20_KEY     "DTE20"
#define DTE20_NAME    "Ten-Eleven Interface"
#define DTE20_VERSION "v0.5 (Alpha)"

// (CONxx) Condition Bits on Device DTE.
#define DTE_RM     0100000 // Restricted Mode 11
#define DTE_DEAD11 0040000 // Power Failure on PDP-11
#define DTE_TO11DB 0020000 // Ring Doorbell to PDP-11
#define DTE_CR11B  0010000 // Clear Reload PDP-11 Button
#define DTE_SR11B  0004000 // Set Reload PDP-11 Button
#define DTE_TO10DB 0001000 // Ring Doorbell to PDP-10
#define DTE_CL11PT 0001000 // Clear Doorbell for PDP-10
#define DTE_TO11ER 0000400 // To-11 Error
#define DTE_TO11DN 0000100 // To-11 Normal Termination Flag (Done)
#define DTE_CLTO11 0000100 // Clear To-11 Normal Termination Flag
#define DTE_TO10DN 0000040 // To-10 Normal Termination Flag (Done)
#define DTE_CLTO10 0000040 // Clear To-10 Normal Termination Flag
#define DTE_PILDEN 0000020 // Enable Load PIA
#define DTE_TO10ER 0000020 // To-10 Error
#define DTE_PIOENB 0000010 // PIO Enable
#define DTE_PIA    0000007 // PIA

#define DTE_CLR  (DTE_CL11PT|DTE_CLTO11|DTE_CLTO10)
#define DTE_DONE (DTE_TO10DB|DTE_TO10DN|DTE_TO10ER|DTE_TO11DN|DTE_TO11ER)

// (DATAx) Data Bits on Device DTE.
#define TO10IB 0010000 // Interrupt after xfer
#define TO10BC 0007777 // To-10 Byte Count Field

// DTE20 Hardware EPT Locations
#define DTEHBG 0140 // Beginning of DTE20 Hardware EPT Location
#define DTEEBP 0140 // To-11 Byte Pointer
#define DTETBP 0141 // To-10 Byte Pointer
#define DTEDII 0142 // Interrupt Location
//             0143 // Unused
#define DTEEPW 0144 // Examine Protection Word
#define DTEERW 0145 // Examine Relocation Word
#define DTEDPW 0146 // Deposit Protection Word
#define DTEDRW 0147 // Deposit Relocation Word
#define DTEHBE 0147 // End of DTE20 Hardware EPT Location

#define DTE_GETCB(id) (DTEHBG + ((id) * 8))

#define DTE_CB     dte->eptCommBase
#define DTE_CB11BP *(DTE_CB + (DTEEBP - DTEHBG))
#define DTE_CB10BP *(DTE_CB + (DTETBP - DTEHBG))
#define DTE_CBINT  *(DTE_CB + (DTEDII - DTEHBG))
#define DTE_CBEPW  *(DTE_CB + (DTEEPW - DTEHBG))
#define DTE_CBERW  *(DTE_CB + (DTEERW - DTEHBG))
#define DTE_CBDPW  *(DTE_CB + (DTEDPW - DTEHBG))
#define DTE_CBDRW  *(DTE_CB + (DTEDRW - DTEHBG))

// *************************************
// ****** Secondary Protocol Area ******
// *************************************

// DTE20 Secondary Protocol Locations
//   11 = PDP10 Rings PDP11 Doorbell
//   10 = PDP11 Rings PDP10 Doorbell

#define DTEFLG 0444 //      Operation Complete Flag
//             0445 //      |
//             0446 //      | Unused
//             0447 //      |
#define DTEF11 0450 //      Chracter from CTY to PDP-10
#define DTECMD 0451 // (11) PDP-10 to PDP-11 Command Word
//             0452 //      |
//             0453 //      | Unused
//             0454 //      |
#define DTEMTD 0455 // (10) CTY Output Complete Flag
#define DTEMTI 0456 // (10) CTY Input Flag

// DTE20 Secondary Protocol Command Field
#define CMD_MTO 010 // CTY Output
#define CMD_ESP 011 // Monitor Mode On  (Enter Secondary Protocol)
#define CMD_LSP 012 // Monitor Mode Off (Leave Secondary Protocol)
#define CMD_GDT 013 // Get Date/Time

// DTECMD Command Word
#define CMD_M_CODE 017  // Mask of Command Code
#define CMD_P_CODE 8    // Position of Command Code
#define MTO_CHAR   0377 // MTO: ASCII Character
#define LSP_RCOM   01   // LSP: Reset Communication Region Area

// DTEMTD (Output Done Flag)
#define MTD_ODN 0x8000 // Terminal Output Done
#define MTD_CSH 0x0002 // Cache Enable by KLI
#define MTD_GDT 0x0001 // Zero if GDT is supported

// DTE20 Time of Day format for Secondary Protocol
// 
//     0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 0: |       Valid (Always -1)       |              Year             |
//    +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
// 1: |     Month     |      Day      |  Day of Week  |D|  Time Zone  |
//    +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
// 2: |     Seconds Since Midnight    |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0|
//    +-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-^-+-+-+-+
//     0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3 
//     4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5

#define TOD_VALID 0xFFFF0000 // 0 Valid - Always -1
#define TOD_YEAR  0x0000FFFF // 0 Year
#define TOD_MONTH 0xFF000000 // 1 Month
#define TOD_DAY   0x00FF0000 // 1 Day of Month
#define TOD_WDAY  0x0000FF00 // 1 Day of Week
#define TOD_DST   0x00000080 // 1 Day Saving Time Flag
#define TOD_TZ    0x0000007F // 1 Time Zone
#define TOD_SECS  0xFFFF0000 // 2 Seconds since midnight / 2

// *************************************
// ******* Primary Protocol Area *******
// *************************************

// Communication Region Area for each Processor
//
// COMBUF:  <Header words for processor #n, n-1, ...>
//          <Header word for processor #1>
//          <Header word for processor #0>
//
// COMBAS:  10's Own Communication Region
//            10's fixed   16 words
//            DTE0 to11:    8 words
//            DTE1 to11:    8 words
//            DTE2 to11:    8 words
//            DTE3 to11:    8 words
//
//          11#1's Own Comm Region
//            #1's fixed   16 words
//            DTE0 To11:    8 words
//          11#1's Own Comm Region
//            #1's fixed   16 words
//            DTE1 To11:    8 words
//          11#1's Own Comm Region
//            #1's fixed   16 words
//            DTE2 To11:    8 words
//          11#1's Own Comm Region
//            #1's fixed   16 words
//            DTE3 To11:    8 words

// COMBUF Header word at offset 0 in examine relocation area.
#define CMP_P_CPUN  24
#define CMP_CPUN    037
#define CMP_ADDR    0177777

// Obsolete
#define nPIDENT   000 // Processor Identification Word
#define nCNTPNT   001 // Pointer to Communication Area of Next Processor
#define nTOD      002 // Time of Day
#define nDATE     003 // Date
#define nKALIVE   004 // Keep Alive Counter
#define nEPTADR   015 // Address of Executive Process Table (EPT)
#define nLDADR    016 // Start Address of 10-Loader
#define nMONADR   017 // Start Address of 10-Monitor
#define nFORPRO   020 // For Processor Identification Word
#define nPROPRNT  021 // 
#define nSTATUS   022 // Status Word
#define nQSIZE    023 // Queue Size Word
#define nCTY0CW   024 // CTY0 Command Word
#define nCTY0RW   025 // CTY0 Response Word
#define nMISCW    030 // Misc Command Word
#define nMISRW    031 // Misc Response Word
#define nDATCW    032 // Data Command Word
#define nDATRW    033 // Data Response Word

// Word 0  PIDENT - Processor Identification Word
//
// |<---------- Left Halfword -------->|<--------- Right Halfword -------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |T|CVER |    PVER     |  NPRO     |CASZ |           PNAM                |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Label  Description
// -----  -----------
// T      Ten Indicator
// CVER   Communication Version
// PVER   Protocol Version
// NPRO   Number of Processors
// CASZ   Communication Area Size (In Multiplies of 8)
// PNAM   Procesor Name

#define PID_TIND 0400000000000 // Ten Processor Indicator
#define PID_CVER 0340000000000 // Communication Version
#define PID_PVER 0037600000000 // Protocol Version
#define PID_NPRO 0000176000000 // Number of Processors
#define PID_CASZ 0000001600000 // Block Size (In multiplies of 8)
#define PID_PNAM 0000000177777 // Processor Name

#define PID_M_CVER 03
#define PID_M_PVER 0177
#define PID_M_NPRO 077
#define PID_M_CASZ 07
#define PID_M_PNAM 0177777

// Word 22  STATUS - Status Word
//
// |<---------- Left Halfword -------->|<--------- Right Halfword -------->|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |P|L|I|V|F|N|0 0 0 0 0 0 0|Q|0 0 0|W|I|X|    TO10IC     |    TO11IC     |
// +-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-^-+-+-+
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//
// Label    Description
// -----    -----------
// P        Power Fail Indicator
// L        Load -11 Code Indicator
// I        Initialize (QP2)
// V        Valid Examine Test
// F        CTY In Use Indicator
// N        CTY In Use Number
// Q        Queue'd In Use
// W        16-Bit Mode
// I        Indirect Data
// X        Processor is transferring queue
// TO10IC   TO10 Interrupt Count
// TO11IC   TO11 Interrupt Count

#define ST_POWERF  0400000000000 // Power Fail Indicator
#define ST_LOAD11  0200000000000 // Load -11 Code Indicator
#define ST_INIT    0100000000000 // Initialize (QP2)
#define ST_VALID   0040000000000 // Valid Examine (Must be one)
#define ST_CTYIUF  0020000000000 // CTY In Use Indicator
#define ST_CTYIUN  0010000000000 // CTY In Use Number
#define ST_QPIU    0000020000000 // Queue'd Protocol In Use
#define ST_16BIT   0000001000000 // Transfer: 16-Bit Mode
#define ST_IND     0000000400000 // Transfer: Indirect Data
#define ST_TOIT    0000000200000 // Processor is transferring queue
#define ST_RCV     0000000200000 // Received (QP2)
#define ST_TO10IC  0000000177600 // TO-10 Interrupt Count
#define ST_TO11IC  0000000000377 // TO-11 Interrupt Count

#define ST_P_TO10IC 8
#define ST_M_IC     0377

// TO10Q/TO11Q Message Field - Function Codes
#define FNC_LCI 001 // TO-11 Initial Message
#define FNC_ALS 002 // TO-10 Initial Message
#define FNC_STR 003 // String Data
#define FNC_LNC 004 // Line-Char, Line-Char
#define FNC_RDS 005 // Request Device Status
#define FNC_HDS 007 // Here is Device Status
#define FNC_RDT 011 // Request Date and Time
#define FNC_HDT 012 // Here is Date and Time
#define FNC_FLO 013 // Flush Output (^O)
#define FNC_SNA 014 // Send All (All terminal lines)
#define FNC_DSC 015 // Dataset Connected
#define FNC_HUD 016 // Hang Up Dataset
#define FNC_ACK 017 // Acknowledge (Device Line Allocation Free)
#define FNC_XOF 020 // XOFF TTY Line
#define FNC_XON 021 // XON TTY Line
#define FNC_HLS 022 // Here are line speeds for TTYs
#define FNC_HLA 023 // Here are allocations
#define FNC_RBI 024 // TO-10 Reboot Information
#define FNC_AKA 025 // Acknowledge All
#define FNC_TDO 026 // Turn Device (On/Off)
#define FNC_EDR 027 // Enable/Disable Remotes
#define FNC_LDR 030 // Load LP RAM
#define FNC_LDV 031 // Load LP VFU
#define FNC_D6D 032 // DAS60 Data
#define FNC_KPS 033 // KLINIK Parameter Status
#define FNC_AXF 034 // Enable/Disable Auto-XOFF
#define FNC_BTC 035 // TO-11 Break Through Character Data
#define FNC_DBN 036 // TO-11 Turn On Debug Mode (KL Mode)
#define FNC_DBF 037 // TO-11 Turn Off Debug Mode

#define DEV_CTY  1  // CTY device on DL11-C device.
#define DEV_DL1  2  // DL11-C on the master
#define DEV_DH1  3  // DH11 terminal lines
#define DEV_DLS  4  // Generic Data Lines
#define DEV_LPT  5  // Line Printer
#define DEV_CDR  6  // Card Reader
#define DEV_CLK  7  // Clock
#define DEV_FE   8  // Software Front End Device

#define DLS_CTY  0  // DLS Line #0 for CTY

// Here is Device Status Defintions

#define HDS_CFG 0004000 // Configuration Information
#define HDS_LIN 0002000 // Lost Interrupt Enable
#define HDS_HNG 0001000 // Hung Device
#define HDS_F11 0000400 // FILES-11
#define HDS_FER 0000200 // Fatal Error
#define HDS_ELR 0000100 // Error Logging Request
#define HDS_EOF 0000040 // End of File
#define HDS_IOP 0000020 // I/O In Progress
#define HDS_SER 0000010 // Soft Error
#define HDS_HER 0000004 // Hard Error
#define HDS_OFL 0000002 // Off-Line Flag
#define HDS_NXD 0000001 // Non-Existant Device

#define PRIMARY   0  // Primary Protocol
#define SECONDARY 1  // Secondary Protocol

// DTM Message Packet
#define DTM_LEN   (QMH_SIZ + 8)
#define DTM_DST   0x80
#define DTM_VALID 0xFFFF

#define DTE_MAXUNITS  4     // Maximum number of DTEs.
#define DTE_BASE      0200  // Default Device Code
#define DTE_NPKTS     32    // Number of Message Packets

#define HEAD 1
#define TAIL 0

#define DTE_OK     0 // Successful
#define DTE_ERROR  1 // Error

#define QMH_SIZ 10     // Message Header Size
#define QMH_IND 0x8000 // High bit of indirect

#define SND_HDR 0 // Sending header packet
#define SND_DAT 1 // Sending data packet     (bytes)
#define SND_IND 2 // Sending indirect packet (words)

// TO11 Message Flags
#define TO11_16BIT   0x00000004 // 16-bit data transfers
#define TO11_GOTHDR  0x00000002 // Got message header flag
#define TO11_INDIR   0x00000001 // Indirect Data Request

#define DTE_DTMSENT  1 // Day/Time Message Sent Flag

#define CTY_PORT   5000 // Default CTY port
#define CTY_ESCAPE 0x1C // Default ASCII code

typedef struct dte_Device DTE_DEVICE;
typedef struct dte_Packet DTE_PACKET;

struct dte_Packet {
	DTE_PACKET *Next;    // Next Packet
	uint32     idPacket; // Packet ID
	uint32     Flags;    // Packet Flags
	uint32     xfrCount; // Transferred Bytes Count

	// Packet Data
	uint16 cnt;       // Byte/Word Count
	uint16 fnc;       // Function Code
	uint16 dev;       // Device Code
	uint16 wd1;       // First word
	uint8  Data[256]; // Data (up to 256 bytes)
};

struct dte_Device {
	UNIT      Unit;   // Unit Header Information
	P10_IOMAP ioMap;  // I/O Mapping

	int32  idChannel;    // Channel Identification
	int32  idDevice;     // Device Identification
	int32  Protocol;     // Protocol Flag (Primary = 0, Secondary = 1)
	uint16 srFlags;      // Status Register
	uint16 mtdFlags;     // Done Register
	int32  t10bCount;    // TO10 - Byte Count/Interrupt after xfer

	// Communication Base Area (in EPT block)
	int36  *eptCommBase;   // Communication Base Pointer
	uint32  eptOffset;     // Offset from EPT Base Address

	int36  *eptExamine;    // Examine Pointer
	uint32  eptExaAddr;    // Examine Address
	uint32  eptExaSize;    // Examine Size
	
	int36  *eptDeposit;    // Deposit Pointer
	uint32  eptDepAddr;    // Deposit Address
	uint32  eptDepSize;    // Deposit Size
	
	// Communication Region Area (from Examine/Deposit Area)
	uint32  cmbProc;       // Processor #
	uint32  cmbAddr;       // Communication Base Address
	uint32  depAddr;       // Deposit Address
	uint32  ec10Addr;      // Examine To10 Comm Base
	uint32  et10Addr;      // Examine To10 Address
	uint32  dt10Addr;      // Deposit To10 Address
	uint32  et11Addr;      // Examine To11 Address

	uint32     Flags;    // Day/Time Message Sent Flag
	int36      to10st;   // TO10 Status Word
	uint8      to11qc;   // TO11 Queue Count
	uint32     t11Flags; // TO11 Flags
	DTE_PACKET pktMsg;   // TO11 Message

	// Message Packet Queues
	DTE_PACKET *pktQueue;          // Queue packet (Head)
	DTE_PACKET *pktTail;           // Queue packet (Tail)
	DTE_PACKET *pktFree;           // Free packets
	DTE_PACKET pktData[DTE_NPKTS]; // Message Packets

	// Send Data
	uint32  sndState;    // Sending State (Header, Data, and Indirect)

	CLK_QUEUE kacTimer;  // Keep Alive Counter Timer

	SOCKET    *ctyServer;   // CTY Listening Socket
	SOCKET    *ctySocket;   // CTY Socket
	CLK_QUEUE cinTimer;     // CTY Input Queue Timer
	CLK_QUEUE ackTimer;     // ACK Delay Timer

	// Console TTY Buffer
	uchar inBuffer[4096];
	char  outBuffer[4096];
	int   idxInQueue, idxOutQueue;
	int   idxOutBuffer;
	char  lastSeen;
};
