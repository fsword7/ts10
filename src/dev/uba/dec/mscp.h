// mscp.h - Mass Storage Control Protocol (MSCP/TMSCP)
//
// Copyright (c) 2001-2003, Timothy M. Stark
// Copyright (c) 2001-2003, Robert M. Sunpik
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
//
// -----------------------------------------------------------------------
//
// Modification History:
//
// 01/28/03  TMS  Added Command Flags.
// 01/13/03  TMS  Added RW_TAP_LNT since I learned that Bob Supnik's
//                posting in alt.sys.* newsgroup.  Also renamed RW_LNT
//                to RW_DSK_LNT.
// 01/13/03  TMS  Modification history starts here for better
//                understanding code.
//
// -----------------------------------------------------------------------

// Command Flags

#define CMF_IMM  0x80000000 // Immediate
#define CMF_SEQ  0x40000000 // Sequential
#define CMF_WR   0x20000000 // Write
#define CMF_RW   0x10000000 // Response to GCS
#define CMF_MOD  0x0000FFFF // Valid Modifier Flags

// Opcodes (* = TMSCP only, + = MSCP only)

#define OP_ABO 0x01 // 001 Abort
#define OP_GCS 0x02 // 002 Get Command Status
#define OP_GUS 0x03 // 003 Get Unit Status
#define OP_SCC 0x04 // 004 Set Controller Characteristics
#define OP_SEX 0x07 // 007 Serious Exception 
#define OP_AVL 0x08 // 010 Available
#define OP_ONL 0x09 // 011 Online
#define OP_SUC 0x0A // 012 Set Unit Characteristics
#define OP_DAP 0x0B // 013 Determine Access Paths
#define OP_ACC 0x10 // 020 Access
#define OP_CCD 0x11 // 021 +Compare Controller Data
#define OP_ERS 0x12 // 022 Erase
#define OP_FLU 0x13 // 023 +Flush
#define OP_RPL 0x14 // 024 Replace
#define OP_ERG 0x16 // 026 *Erase Gap
#define OP_CHD 0x20 // 040 Compare Host Data
#define OP_RD  0x21 // 041 Read
#define OP_WR  0x22 // 042 Write
#define OP_WTM 0x24 // 044 *Write Tape Mark
#define OP_POS 0x25 // 045 *Positioning
#define OP_FMT 0x2F // 057 +Format
#define OP_AVA 0x40 // 100 Unit Now Available    (Attention Message)
#define OP_DUN 0x41 // 101 Duplicate Unit Number (Attention Message)
#define OP_ACP 0x42 // 102 Access Path           (Attention Message)
#define OP_END 0x80 // 200 End Flag

// Modifiers

#define MD_EXP 0x8000 // +Express Request
#define MD_CMP 0x4000 // Compare
#define MD_CSE 0x2000 // Clear Serious Exception
#define MD_ERR 0x1000 // +Force Error
#define MD_CDL 0x1000 // *Controller Data Lost
#define MD_SCH 0x0800 // *Suppress Caching (High Speed)
#define MD_SCL 0x0400 // Suppress Caching (Low Speed)
#define MD_SEC 0x0200 // Suppress Error Correction
#define MD_SER 0x0100 // Suppress Error Recovery
#define MD_DLE 0x0080 // *Detect LEOT
#define MD_SHW 0x0080 // Suppress Shadowing
#define MD_IMM 0x0040 // *Immediate
#define MD_WBN 0x0040 // Write Back (Non-volatile)
#define MD_WBV 0x0020 // Write Back (Volatile)
#define MD_EXA 0x0020 // +Exclude Access
#define MD_WSS 0x0010 // Write Shadow Set One Unit at a time
#define MD_UNL 0x0010 // *AVL: Unload
#define MD_SHD 0x0010 // Shadowing
#define MD_ERW 0x0008 // *WR: Enable Rewrite
#define MD_REV 0x0008 // *RD, POS: Reverse
#define MD_SWP 0x0004 // SUC: Set Write Protection
#define MD_OBC 0x0004 // *POS: Object Count
#define MD_IMF 0x0002 // ONL: Ignore Media Format Error
#define MD_RWD 0x0002 // *POS: Rewind
#define MD_ACL 0x0002 // AVL: All Class
#define MD_RIP 0x0001 // +ONL: Allow Self-Destruction
#define MD_NXU 0x0001 // GUS: Next Unit?

// End Flags

#define EF_LOG 0x0020 // Error Log
#define EF_SXC 0x0010 // Serious Exception
#define EF_EOT 0x0008 // *End of Tape
#define EF_POL 0x0004 // *Position Lost

// Status Codes

#define ST_SUC   0   // Successful
#define ST_CMD   1   // Invalid Command
#define ST_ABO   2   // Aborted Command
#define ST_OFL   3   // Unit Offline
#define ST_AVL   4   // Unit Available
#define ST_MFE   5   // Media Format Error
#define ST_WPR   6   // Write Protection Error
#define ST_CMP   7   // Compare Error
#define ST_DAT   8   // Data Error
#define ST_HST   9   // Host Access Error
#define ST_CNT   10  // Controller Error
#define ST_DRV   11  // Drive Error
#define ST_FMT   12  // *Formatter Error
#define ST_BOT   13  // *BOT Encountered
#define ST_TMK   14  // *Tape Mark
#define ST_RDT   16  // *Record Truncation
#define ST_POL   17  // *Position Lost
#define ST_SXC   18  // Serious Exception
#define ST_LED   19  // *LEOT Detected
#define ST_BBR   20  // +Bad Block
#define ST_DIA   31  // Diagnostic
#define ST_P_SUB 5   // Subcode
#define ST_P_INV 8   // Invalid Operation

// Subcodes

#define SB_SUC_IGN  (1 << ST_P_SUB)   // *Unload Ignored
#define SB_SUC_ON   (8 << ST_P_SUB)   // Already Online
#define SB_SUC_EOT  (32 << ST_P_SUB)  // *EOT Encountered
#define SB_SUC_RO   (128 << ST_P_SUB) // Read Only
#define SB_OFL_NV   (1 << ST_P_SUB)   // No Volume
#define SB_OFL_INOP (2 << ST_P_SUB)   // *Inoperative
#define SB_AVL_INU  (32 << ST_P_SUB)  // In Use
#define SB_WPR_SW   (128 << ST_P_SUB) // Software Write Lock
#define SB_WPR_HW   (256 << ST_P_SUB) // Hardware Write Lock
#define SB_HST_OA   (1 << ST_P_SUB)   // Odd Address
#define SB_HST_OC   (2 << ST_P_SUB)   // +Odd Count
#define SB_HST_NXM  (3 << ST_P_SUB)   // Non-existant Memory
#define SB_HST_PTE  (5 << ST_P_SUB)   // Mapping Error
#define SB_DAT_RDE  (7 << ST_P_SUB)   // *Read Error

// Status Invalid Subcodes

#define I_OPCD  (8 << ST_P_INV)   // Invalid Opcode
#define I_FLAG  (9 << ST_P_INV)   // Invalid Flags
#define I_MODF  (10 << ST_P_INV)  // Invalid Modifier
#define I_BCNT  (12 << ST_P_INV)  // Invalid Byte Count
#define I_LBN   (28 << ST_P_INV)  // Invalid Logical Block Number
#define I_VRSN  (12 << ST_P_INV)  // Invalid Version
#define I_FMTI  (28 << ST_P_INV)  // Invalid Format

// Controller Flags

#define CF_RPL  0x8000 // Controller Bad Block Replacement
#define CF_ATN  0x0080 // Enable Attention Messages
#define CF_MSC  0x0040 // Enable Miscellaneous Error Log Messages
#define CF_OTH  0x0020 // Enable Other Host's Error Log Messages
#define CF_THS  0x0010 // Enable This Host's Error Log Messages
#define CF_MHS  0x0004 // Multi-Host
#define CF_SHW  0x0002 // Shadowing
#define CF_576  0x0001 // 576-byte sectors (16/18-bit data mode)
#define CF_MASK (CF_ATN|CF_MSC|CF_OTH|CF_THS)

// Unit Flags

#define UF_RPL  0x8000 // +Controller Bad Block Replacement
#define UF_CAC  0x8000 // *Cache Write Back
#define UF_INA  0x4000 // Inactive Shadow Set Unit
#define UF_SCH  0x8000 // Suppress Caching (High-Speed)
#define UF_SCL  0x4000 // Suppress Caching (Low-Speed)
#define UF_WPH  0x2000 // Write Protection Hardware
#define UF_WPS  0x1000 // Write Protection Software
#define UF_WPD  0x0100 // Write Protection Data
#define UF_RMV  0x0080 // Removable Media
#define UF_WBN  0x0040 // *Write Back Non-Volatile
#define UF_VSS  0x0020 // *Suppress Variable Speed
#define UF_576  0x0004 // +576-byte sector (16/18-bit data mode)
#define UF_CMW  0x0002 // Compare Writes
#define UF_CMR  0x0001 // Compare Reads
#define UF_MASK (UF_CMW|UF_CMR) // Write Mask

// Tape Format Flags

#define TF_9TK     0x0100 // 9 Track
#define TF_9TK_NRZ 0x0001 //   800 bpi
#define TF_9TK_PE  0x0002 //   1600 bpi
#define TF_9TK_GRP 0x0004 //   6250 bpi
#define TF_CTP     0x0200 // TK50
#define TF_CTP_LO  0x0001 //   Low density
#define TF_CTP_HI  0x0002 //   High denisty
#define TF_3480    0x0300 // 3480
#define TF_WOD     0x0400 // RV80

// Error Log Flags

#define LF_SUC 0x0080 // Successful
#define LF_CON 0x0040 // Continuing
#define LF_BBR 0x0020 // Bad Block Replacement (NI)
#define LF_RCT 0x0010 // Error in Replacement (NI)
#define LF_SNR 0x0001 // Sequence Number Reset

// Error Log Formats

#define FM_CNT 0 // Port Last Fail Error
#define FM_BAD 1 // Bad Host Address
#define FM_DSK 2 // Disk Transfer
#define FM_SDI 3 // SDI Error
#define FM_SDE 4 // SM Disk Error
#define FM_TAP 5 // *Tape Error
#define FM_RPL 9 // Bad Block Replacement

// Class Numbers

#define CLS_CNTL 1  // Controller Class
#define CLS_DISK 2  // Disk Class
#define CLS_TAPE 3  // Tape Class


// Message Packet Format
// Note: All packet lengths must be multiplies of 4 bytes

// Command Packet

#define CMD_REFL 2 // Reference Number (Low)
#define CMD_REFH 3 // Reference Number (High)
#define CMD_UNIT 4 // Unit Number
//               5 // Reserved Area
#define CMD_OPC  6 // Opcode
#define CMD_MOD  7 // Modifier

#define CMD_OPC_P_OPC 0     // Opcode Field
#define CMD_OPC_M_OPC 0xFF  // Opcode Mask
#define CMD_OPC_P_CCA 8     // Cache Field
#define CMD_OPC_M_CCA 0xFF  // Cache Mask
#define CMD_OPC_P_FLG 8     // Flag Field
#define CMD_OPC_M_FLG 0xFF  // Flag Mask


// Response Packet

#define RSP_LNT  12 // Response Packet Length
#define RSP_REFL 2  // Reference Number (Low)
#define RSP_REFH 3  // Reference Number (High)
#define RSP_UNIT 4  // Unit Number
#define RSP_RSV  5  // Reserved Area
#define RSP_OPF  6  // Opcode, Flag
#define RSP_STS  7  // Status

#define RSP_OPF_P_OPC 0 // Opcode Field
#define RSP_OPF_P_FLG 8 // Flag Field

// Abort Packet
// MSCP Opcode: 01

#define ABO_LNT  16  // Packet Length
#define ABO_REFL 8   // Reference Number (Low)
#define ABO_REFH 9   // Reference Number (High)


// Get Command Status Packet
// MSCP Opcode: 02

#define GCS_LNT  20  // Packet Length
#define GCS_REFL 8   // Reference Number (Low)
#define GCS_REFH 9   // Reference Number (High)
#define GCS_STSL 10  // Status (Low)
#define GCS_STSH 11  // Status (High)

// Get Unit Status Packet
// MSCP Opcode: 03

#define GUS_LNT   48  // Disk Packet Length
#define GUS_LNT_T 44  // Tape Packet Length
#define GUS_MLUN  8   // Logical Unit Number
#define GUS_UFLG  9   // Unit Flags
#define GUS_RSVL  10  // Reserved Area
#define GUS_RSVH  11  // Reserved Area
#define GUS_UIDA  12  // Unit ID A
#define GUS_UIDB  13  // Unit ID B
#define GUS_UIDC  14  // Unit ID C
#define GUS_UIDD  15  // Unit ID D
#define GUS_MEDL  16  // Media ID Low
#define GUS_MEDH  17  // Media ID High

// Disk Specific Parameters
#define GUS_SHUN  18  // Shadow Unit
#define GUS_SHST  19  // Shadow Status
#define GUS_TRK   20  // Track
#define GUS_GRP   21  // Group
#define GUS_CYL   22  // Cylinder
#define GUS_UVER  23  // Unit Version
#define GUS_RCTS  24  // RCT Size
#define GUS_RBSC  25  // RBNs, Copies

// Tape Specific Parameters
#define GUS_FMT   18  // Format
#define GUS_SPEED 19  // Speed
#define GUS_MENU  20  // Menu
#define GUS_CAP   21  // Capacity
#define GUS_FVER  22  // Formatter Version

#define GUS_UIDD_P_MOD 0 // Model Number
#define GUS_UIDD_P_CLS 8 // Class Number
#define GUS_RB_P_RBNS  0 // RBNs/track
#define GUS_RB_P_RCTC  8 // RCT copies

// Set Controller Characteristics Packet
// MSCP Opcode: 04

#define SCC_LNT  32  // Packet Length
#define SCC_MSV  8   // MSCP Version
#define SCC_CFLG 9   // Controller Flags
#define SCC_TMO  10  // Timeout
#define SCC_VER  11  // Controller Version
#define SCC_CIDA 12  // Controller ID A
#define SCC_CIDB 13  // Controller ID B
#define SCC_CIDC 14  // Controller ID C
#define SCC_CIDD 15  // Controller ID D
#define SCC_MBCL 16  // Maximum Bytes Count Low
#define SCC_MBCH 17  // Maximum Bytes Count High

#define SCC_VER_P_SVER 0 // Software Version
#define SCC_VER_P_HVER 8 // Hardware Version
#define SCC_CIDD_P_MOD 0 // Model Number
#define SCC_CIDD_P_CLS 8 // Class Number

// Available Packet
// MSCP Opcode: 08

#define AVL_LNT  12 // Packet Length

// Online Packet
// MSCP Opcode: 09

#define ONL_LNT  44  // Packet Length
#define ONL_MLUN 8   // Logical Unit Number
#define ONL_UFLG 9   // Unit Flags
#define ONL_RSVL 10  // Reserved Area
#define ONL_RSVH 11  // Reserved Area
#define ONL_UIDA 12  // Unit ID A
#define ONL_UIDB 13  // Unit ID B
#define ONL_UIDC 14  // Unit ID C
#define ONL_UIDD 15  // Unit ID D
#define ONL_MEDL 16  // Media ID Low
#define ONL_MEDH 17  // Media ID High

// Disk Specific Status
#define ONL_SHUN 18  // Shadow Unit
#define ONL_SHST 19  // Shadow Status
#define ONL_SIZL 20  // LBN Size Low
#define ONL_SIZH 21  // LBN Size High
#define ONL_VSNL 22  // Volume Serial Number Low
#define ONL_VSNH 23  // Volume Serial Number High

// Tape Specific Status
#define ONL_FMT  18  // Format Type
#define ONL_SPD  19  // Speed
#define ONL_MAXL 20  // Maximum Record Length Low
#define ONL_MAXH 21  // Maximum Record Length High
#define ONL_NREC 22  // Noise Record
#define ONL_RSVE 23  // Reserved

#define ONL_UIDD_P_MOD 0 // Model Number
#define ONL_UIDD_P_CLS 8 // Class Number

// Set Unit Characteristics Packet
// MSCP Opcode: 0A

#define SUC_LNT  44 // Packet Length
#define SUC_MLUN 8   // Logical Unit Number
#define SUC_UFLG 9   // Unit Flags
#define SUC_RSVL 10  // Reserved Area
#define SUC_RSVH 11  // Reserved Area
#define SUC_UIDA 12  // Unit ID A
#define SUC_UIDB 13  // Unit ID B
#define SUC_UIDC 14  // Unit ID C
#define SUC_UIDD 15  // Unit ID D
#define SUC_MEDL 16  // Media ID Low
#define SUC_MEDH 17  // Media ID High

// Disk Specific Status
#define SUC_SHUN 18  // Shadow Unit
#define SUC_SHST 19  // Shadow Status
#define SUC_SIZL 20  // LBN Size Low
#define SUC_SIZH 21  // LBN Size High
#define SUC_VSNL 22  // Volume Serial Number Low
#define SUC_VSNH 23  // Volume Serial Number High

// Tape Specific Status
#define SUC_FMT  18  // Format Type
#define SUC_SPD  19  // Speed
#define SUC_MAXL 20  // Maximum Record Length Low
#define SUC_MAXH 21  // Maximum Record Length High
#define SUC_NREC 22  // Noise Record
#define SUC_RSVE 23  // Reserved

#define SUC_UIDD_P_MOD 0 // Model Number
#define SUC_UIDD_P_CLS 8 // Class Number

// Erase Packet
// MSCP Opcode: 12

#define ERS_LNT  12 // Packet Length

// Flush Packet
// MSCP Opcode: 13

#define FLU_LNT  32   // Packet Length
//               8-15 // Reserved Area
#define FLU_POSL 16   // Position Low
#define FLU_POSH 17   // POsition High

// Erase Gap Packet
// MSCP Opcode: 16

#define ERG_LNT  12 // Packet Length

// Write Tape Mark Packet
// MSCP Opcode: 24 

#define WTM_LNT  32   // Packet Length
//               8-15 // Reserved Area
#define WTM_POSL 16   // Position Low
#define WTM_POSH 17   // POsition High

// Position Packet
// MSCP Opcode: 25

#define POS_LNT  32    // Packet Length
#define POS_RCL  8     // Record Count Low
#define POS_RCH  9     // Record Count High
#define POS_TMCL 10    // Tape Mark Count Low
#define POS_TMCH 11    // Tape Mark Count High
//               12-15 // Reserved Area
#define POS_POSL 16    // Position Low
#define POS_POSH 17    // POsition High

// Format Packet
// MSCP Code: 2F

#define FMT_LNT  12    // Packet Length
#define FMT_IH   17    // Magic Bit

// Data Transfer Packet

#define RW_DSK_LNT 32 // Packet Length for Disk
#define RW_TAP_LNT 36 // Packet Length for Tape (TMSCP)
#define RW_BCL      8 // Byte Count Low
#define RW_BCH      9 // Byte Count High
#define RW_BAL     10 // Buffer Descriptor Low
#define RW_BAH     11 // Buffer Descriptor High
#define RW_MAPL    12 // Map Table Low
#define RW_MAPH    13 // Map Table High
//                 14 // Reserved Area
//                 15 // Reserved Area

// Disk Specific Parameters
#define RW_LBNL  16 // Logical Block Number Low
#define RW_LBNH  17 // Logical Block Number High
#define RW_WBCL  18 // Working Byte Count Low
#define RW_WBCH  19 // Working Byte Count High
#define RW_WBAL  20 // Working Buffer Address Low
#define RW_WBAH  21 // Working Buffer Address High
#define RW_WLBNL 22 // Working Logical Block Number Low
#define RW_WLBNH 23 // Working Logical Block Number High

// Tape Specific Parameters
#define RW_POSL  16 // Position Low
#define RW_POSH  17 // Position High
#define RW_RSZL  18 // Record Size Low
#define RW_RSZH  19 // Record Size High

// Unit Now Available Packet

#define UNA_LNT  32  // Packet Length
#define UNA_MLUN 8   // Logical Unit Number
#define UNA_UFLG 9   // Unit Flags
#define UNA_RSVL 10  // Reserved Area
#define UNA_RSVH 11  // Reserved Area
#define UNA_UIDA 12  // Unit ID A
#define UNA_UIDB 13  // Unit ID B
#define UNA_UIDC 14  // Unit ID C
#define UNA_UIDD 15  // Unit ID D

#define UNA_UIDD_P_MOD 0 // Model Number
#define UNA_UIDD_P_CLS 8 // Class Number

// Error Log Packet Header

#define ELP_REFL 2  // Reference Low
#define ELP_REFH 3  // Reference High
#define ELP_UNIT 4  // Unit Number
#define ELP_SEQ  5  // Sequence Number
#define ELP_FF   6  // Format, Flag
#define ELP_EVT  7  // Event

#define ELP_EV_P_FMT 0 // Format Field
#define ELP_EV_P_FLG 8 // Flag Field

// Port Last Fail Packet

#define PLF_LNT  24 // Packet Length
#define PLF_CIDA 8  // Controller ID #A
#define PLF_CIDB 9  // Controller ID #B
#define PLF_CIDC 10 // Controller ID #C
#define PLF_CIDD 11 // Controller ID #D
#define PLF_VER  12 // Controller Version
#define PLF_ERR  13 // Error Code

#define PLF_CIDD_P_MOD 0 // Model Number
#define PLF_CIDD_P_CLS 8 // Class Number
#define PLF_VER_P_SVER 0 // Software Version
#define PLF_VER_P_HVER 8 // Hardware Version

// Disk Transfer Error Log Packet

#define DTE_LNT  48 // Packet Length
#define DTE_CIDA 8  // Controller ID #A
#define DTE_CIDB 9  // Controller ID #B
#define DTE_CIDC 10 // Controller ID #C
#define DTE_CIDD 11 // Controller ID #D
#define DTE_CVER 12 // Controller Version
#define DTE_MLUN 13 // Multi-Code, Logical Unit
#define DTE_UIDA 14 // Unit ID #A
#define DTE_UIDB 15 // Unit ID #B
#define DTE_UIDC 16 // Unit ID #C
#define DTE_UIDD 17 // Unit ID #D
#define DTE_UVER 18 // Unit Version
#define DTE_D2   23 // Device Parameters #2
#define DTE_D3   24 // Device Parameters #3
#define DTE_D4   25 // Device Parameters #4

// Disk Specific Status
#define DTE_SCYL 19 // Cylinder
#define DTE_VSNL 20 // Volume Serial Number Low
#define DTE_VSNH 21 // Volume Serial Number High
#define DTE_D1   22 // Device Parameters #1

// Tape Specific Status
#define DTE_RETR 19 // Retry
#define DTE_POSL 20 // Position Low
#define DTE_POSH 21 // Position High
#define DTE_FVER 22 // Formatter Version

#define DTE_CIDD_P_MOD  0 // Controller - Model Number
#define DTE_CIDD_P_CLS  8 // Controller - Class Number
#define DTE_CVER_P_SVER 0 // Controller - Software Version
#define DTE_CVER_P_HVER 8 // Controller - Hardware Version
#define DTE_UIDD_P_MOD  0 // Unit - Model Number
#define DTE_UIDD_P_CLS  8 // Unit - Class Number
#define DTE_D2_P_SECT   8 // Sector Number
#define DTE_D3_P_SURF   0 // Surfrace Number
#define DTE_D3_P_CYL    8 // Cylinder Number

// Host Bus Error Log Packet

#define HBE_LNT   28 // Packet Length
#define HBE_CIDA  8  // Controller ID #A
#define HBE_CIDB  9  // Controller ID #B
#define HBE_CIDC  10 // Controller ID #C
#define HBE_CIDD  11 // Controller ID #D
#define HBE_CVER  12 // Controller Version
#define HBE_RSV   13 // Reserved
#define HBE_BADL  14 // Bad Address Low
#define HBE_BADH  15 // Bad Address High

#define HBE_CIDD_P_MOD 0 // Controller - Model Number
#define HBE_CIDD_P_CLS 8 // Controller - Class Number
#define HBE_VER_P_SVER 0 // Controller - Software Version
#define HBR_VER_P_HVER 8 // Controller - Hardware Version
