// uqssp.h - Unibus/Qbus Storage Systems Port Defintions File
//
// Copyright (c) 2001-2003, Timothy M. Stark
// Copyright (c) 2001-2003, Robert M. Supnik
// Derived from work by Stephen F. Shirron
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

// UQSSP - Unibus/Qbus Storage Systems Port by Digital/Compaq
//   Provide the initializing information for all MSCP
//   controllers (RQDX3, UDA50, HSC50, etc..)
//
// IP Register (772150) - Initialization and Polling
//
//     Read Access  - Let controller polls command queue from host.
//     Write Access - Let controller re-initializes.
//
// SA Register (772152) - Status, Address, and Purge
//
//     Read Access  - Read data and error information from controller.
//     Write Access - Set up controller to startup.

#define UQ_NREGS  2 // Two UQSSP registers
#define UQ_REG_IP 0 // (172150) Initialization and Poll Register
#define UQ_REG_SA 1 // (172152) Status, Address, and Purge Register

#define UQ_IOADDR 0772150 // Default 18-bit CSR address

#ifdef DEBUG
static const char *regName[] = { "IP", "SA" };
#endif /* DEBUG */

// Generic SA Register (Read Access)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER|S4|S3|S2|S1|          Data Area             |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_S1 0x0800 // Init Step #1
#define SA_S2 0x1000 // Init Step #2
#define SA_S3 0x2000 // Init Step #3
#define SA_S4 0x4000 // Init Step #4
#define SA_ER 0x8000 // Error Flag

// *** INITIALIZATION STEP 1 ***
//
// Read Access (Controller to Host)
//
//    |  Step #1  | 
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER| 0| 0| 0| 1|NV|Q2|DI|OD|MP|SM|CN|    Rsvd   |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_S1C_NV  0x0400 // Fixed Interrupt Vector
#define SA_S1C_QB  0x0200 // Q22 Device
#define SA_S1C_DI  0x0100 // External Diagnosics
#define SA_S1C_OD  0x0080 // Odd Address
#define SA_S1C_MP  0x0040 // Mapping
#define SA_S1C_SM  0x0020 // Special Functions
#define SA_S1C_CN  0x0010 // Node Name

// Write Access (Host to Controller)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |VL|WR|   CQ   |   RQ   |IE|         VE         |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_S1H_VL  0x8000 // Valid
#define SA_S1H_WR  0x4000 // Wrap Mode
#define SA_S1H_CQ  0x3800 // Command Length  (1 << x)
#define SA_S1H_RQ  0x0700 // Response Length (1 << x)
#define SA_S1H_IE  0x0080 // Interrupt Enable
#define SA_S1H_VE  0x007F // Interrupt Vector

#define SA_S1H_P_CQ 11   // Command Length Position
#define SA_S1H_M_CQ 0x7  // Command Length Mask
#define SA_S1H_P_RQ 8    // Response Length Position
#define SA_S1H_M_RQ 0x7  // Response Length Mask

// Extract fields to get command/response queue size
#define GET_CQ(x) (1u << (((x) >> SA_S1H_P_CQ) & SA_S1H_M_CQ))
#define GET_RQ(x) (1u << (((x) >> SA_S1H_P_RQ) & SA_S1H_M_RQ))

// *** INITIALIZATION STEP 2 ***
//
// Read Access (Controller to Host)
//
//    |  Step #2  |        |<--- Echo from S1H --->|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER| 0| 0| 1| 0|  Rsvd  |VL|WR|   CQ   |   RQ   |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_S2C_PT  0x0000 // Port Type
#define SA_S2C_EC  0x00FF // Echo Information from S1H high
#define  SA_S2C_VL 0x0080 // Valid
#define  SA_S2C_WR 0x0040 // Wrap Mode
#define  SA_S2C_CQ 0x0038 // Command Length  (1 << x)
#define  SA_S2C_RQ 0x0007 // Response Length (1 << x)

#define SA_S2C_P_EC 8    // Echo Position (S1H high)
#define SA_S2C_M_EC 0xFF // Echo Mask
#define GET_S1H(x)  (((x) >> SA_S2C_P_EC) & SA_S2C_M_EC)

// Write Access (Host to Controller)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |         Communication Address Low          |PI|
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_S2H_CLO 0xFFFE // Communication Address Low (Ring Base)
#define SA_S2H_PI  0x0001 // Adaptor Purge Interrupts

// *** INITIALIZATION STEP 3 ***
//
// Read Access (Controller to Host)
//
//    |  Step #3  |        |<--- Echo from S1H --->|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER| 0| 1| 0| 0|  Rsvd  |IE|         VE         |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_S3C_EC  0x00FF // Echo information from S1H low
#define  SA_S3C_IE 0x0080 // Interrupt Enable 
#define  SA_S3C_VE 0x007F // Interrupt Vector

#define SA_S3C_P_EC 0    // Echo Position (S1H low)
#define SA_S3C_M_EC 0xFF // Echo Mask
#define GET_S1L(x)  (((x) >> SA_S3C_P_EC) & SA_S3C_M_EC)

// Write Access (Host to Controller)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |PP|        Communication Address High          |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_S3H_PP  0x8000 // Purge/Poll Test
#define SA_S3H_CHI 0xEFFF // Communication Address High (Ring Base)

// *** INITIALIZATION STEP 4 ***
//
// Read Access (Controller to Host)
//
//    |  Step #4  | 
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |ER| 1| 0| 0| 0|    Model Number    |  Version  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_S4C_MOD 0x07F0 // Model (Adaptor) Number
#define SA_S4C_VER 0x000F // Version Number

#define SA_S4C_P_MOD 4 // Model Number Position
#define SA_S4C_P_VER 0 // Version Number Position

// Write Access (Host to Controller)
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |              |CS|NN|SF|                 |LF|GO|
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00

#define SA_S4H_CS 0x0400 // Host Scratchpad
#define SA_S4H_NN 0x0200 // Node Name
#define SA_S4H_SF 0x0100 // Special Functions
#define SA_S4H_LF 0x0002 // Send Last Fail Packet
#define SA_S4H_GO 0x0001 // Go

// Controller State

#define CST_IDLE    0  // Power-Up (Initially 0)
#define CST_S1      1  // Init Step #1
#define CST_S2      2  // Init Step #2
#define CST_S3      3  // Init Step #3
#define CST_S4      4  // Init Step #4
#define CST_UP      5  // Ready (Up)
#define CST_DEAD    6  // Dead

#define CST_S1_WRAP 7  // Wrap Mode         (during Init Step #1)
#define CST_S3_PPA  8  // Purge/Poll Test A (during Init Step #3)
#define CST_S3_PPB  9  // Purge/Poll Test B (during Init Step #3)

static const char *cstName[] =
{
	"Idle",               // 0 Power-Up (Initially 0)
	"Init Step #1",       // 1 Init Step #1
	"Init Step #2",       // 2 Init Step #2
	"Init Step #3",       // 3 Init Step #3
	"Init Step #4",       // 4 Init Step #4
	"Ready (Up)",         // 5 Ready (Up)
	"Dead",               // 6 Dead (Error Mode)
	"Wrap Mode",          // 7 Wrap Mode
	"Purge/Poll Test A",  // 8 Purge/Poll Test A
	"Purge/Poll Test B"   // 9 Purge/Poll Test B
};

// Controller Error Codes

#define ER_PRE 1    // Packet Read Error
#define ER_PWE 2    // Packet Write Error
#define ER_QRE 6    // Queue Read Error
#define ER_QWE 7    // Queue Write Error
#define ER_HAT 9    // Host Access Timeout
#define ER_ICI 14   // Invalid Connection Identification
#define ER_PIE 20   // Protocol Incompatible Error
#define ER_PPF 21   // Purge/Poll Error
#define ER_MRE 22   // Map Register Read Error
#define ER_T11 475  // T11 Processor Error
#define ER_SND 476  // T11 Send Error
#define ER_RCV 477  // T11 Receive Error
#define ER_NSR 478  // T11 No such RSRC

// Communication Region Area
// 
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                   Unused                      | -8 
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |                Purge Interrupt                | -6
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |            Command Interrupt Count            | -4
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |           Response Interrupt Count            | -2
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |                                               |  0 (Ring Base Address)
// +--         Response Descriptor #1            --+
// |                                               |  2
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//           :                       :
//           :                       :
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |                                               |
// +--         Response Descriptor #n            --+  :
// |                                               |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |                                               |
// +--         Command Descriptor #0             --+  :
// |                                               |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
//           :                       :
//           :                       :
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |                                               |
// +--         Command Descriptor #n             --+
// |                                               |  n
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+

#define CA_QQ  -8  // Unused
#define CA_PI  -6  // Purge Interrupt
#define CA_CI  -4  // Command Interrupt Count
#define CA_RI  -2  // Response Interrupt Count

// Maximum Communication Region Size
#define CA_MAX ((4 << SA_S1H_M_CQ) + (4 << SA_S1H_M_RQ) + -CA_QQ)

// Ring Descriptor Entry
//
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |             Packet Address Low             | 0|
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// | O| I|                       |  Pkt Addr High  |
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+

#define UQ_DESC_OWN  0x80000000 // Ownership Flag (1 = Controller, 0 = Host)
#define UQ_DESC_INT  0x40000000 // Interrupt Flag
#define UQ_DESC_DONE 0x40000000 // Done Flag
#define UQ_DESC_ADDR 0x003FFFFE // Unibus/Qbus Memory Address

// Generic MSCP Packet
// 
//  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                 Packet Length                 | -4
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |       Conn ID         |    Type   |  Credits  | -2
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+
// |                                               | 0
// +--                                           --+
// |          :                        :           |
// +--        :    Message Packet      :         --+ 
// |          :                        :           |
// +--                                           --+
// |                                               | n
// +--+--+--+--^--+--+--+--^--+--+--+--^--+--+--+--+

#define UQ_HDR_OFF    -4     // Offset to Packet Header
#define UQ_LNT        0      // Message Packet Size (Length)
#define UQ_CTC        1      // Credits, Type, and Connection ID
#define UQ_CTC_CID    0xFF00 // Connection ID
#define UQ_CTC_TYP    0x00F0 // Type
#define UQ_CTC_CR     0x000F // Credits

#define UQ_CTC_P_CR   0      // Credits Position
#define UQ_CTC_M_CR   0xF    // Credits Mask
#define UQ_CTC_P_TYP  4      // Type Position
#define UQ_CTC_M_TYP  0xF    // Type Mask
#define  UQ_TYP_SEQ   0      //   Sequence
#define  UQ_TYP_DATA  1      //   Datagram
#define UQ_CTC_P_CID  8      // Conn ID Position
#define UQ_CTC_M_CID  0xFF   // Conn ID Mask
#define  UQ_CID_MSCP  0      //   MSCP Standard  (Disk Drives)
#define  UQ_CID_TMSCP 1      //   TMSCP Standard (Tape Drives)
#define  UQ_CID_DUP   2      //   DUP
#define  UQ_CID_DIAG  0xFF   //   Diagnostic

// Macro definitions
#define UQ_GETP(p, w, f) \
	(((p)->Data[w] >> w##_P_##f) & w##_M_##f)
#define UQ_GETP32(p, w) \
	((uint32)(p)->Data[(w)+1] << 16) | ((uint32)(p)->Data[w])
#define UQ_PUTP32(p, w, x) \
	p->Data[w] = (uint16)(x); p->Data[(w)+1] = (uint16)((x) >> 16)

// Command/Response Rings Table

typedef struct uq_RingTable UQ_RING;
struct uq_RingTable {
	char   *Name;     // Name of Descriptor
	uint32 intAddr;   // Interrupt Address
	uint32 baseAddr;  // Descriptor Address
	uint32 szDesc;    // Descriptor Length (Size)
	uint32 mskDesc;   // Discriptor Length (Mask)
	uint32 idxDesc;   // Current Index
};

// Interface type definitions

#define UQI_UNKNOWN  0  // Unknown Interface
#define UQI_ESDI     1  // ESDI Interface for RQ controllers
#define UQI_SDI      2  // SDI Interface for UDA50 controller
#define UQI_SCSI     3  // SCSI Interface
