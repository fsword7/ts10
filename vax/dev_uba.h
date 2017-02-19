// dev_uba.h - VAX Unibus Interface
//
// Copyright (c) 2003, Timothy M. Stark
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

#include "vax/nexus.h"

// 0000 UBA_CSR    Configuration Register (NEX_CSR)
// 0004 UBA_CR     Control Register
// 0008 UBA_SR     Status Register
// 000C UBA_DCR    Diagnostic Control Register
// 0010 UBA_FMER   Failed Map Entry Register
// 0014 UBA_FUBAR  Failed UNIBUS Address Register
// 0018 Reserved
// 001C Reserved
// 0020 UBA_BRSVR  Send Vector Registers (4)
// 0030 UBA_BRRVR  Receive Vector Registers (4)
// 0040 UBA_DPR    Buffered Data Path Registers (16)
// 0080 Reserved
// 0800 UBA_MAP    Unibus Map Registers (496)
// 0FC0 Reserved

// 0004 UBA_CR - Control Register

#define UBCR_MRD      0x7C000000 // Map Register Disable bit 4-0
#define UBCR_IFS      0x00000040 // Interrupt Field Switch
#define UBCR_BRIE     0x00000020 // BR Interrupt Enable
#define UBCR_USEFIE   0x00000010 // UNIBUS to SBI Error Field IE
#define UBCR_SUEFIE   0x00000008 // SBI to UNIBUS Error Field IE
#define UBCR_CNFIE    0x00000004 // Configuration Interrupt Enable
#define UBCR_UPF      0x00000002 // UNIBUS Power Fail
#define UBCR_ADINIT   0x00000001 // Adapter Initialization

// 0008 UBA_SR - Status Register

#define UBSR_BR7FULL  0x08000000 // BR7 Receive Vector Register Full
#define UBSR_BR6FULL  0x04000000 // BR6 Receive Vector Register Full
#define UBSR_BR5FULL  0x02000000 // BR5 Receive Vector Register Full
#define UBSR_BR4FULL  0x01000000 // BR4 Receive Vector Register Full
#define UBSR_RDTO     0x00000400 // UNIBUS to SBI Read Data Timeout
#define UBSR_RDS      0x00000200 // Read Data Substitute
#define UBSR_CRD      0x00000100 // Corrected Read Data
#define UBSR_CXTER    0x00000080 // Command Transmit Error
#define UBSR_CXTMO    0x00000040 // Command Transmit Timeout
#define UBSR_DPPE     0x00000020 // Data Path Parity Error
#define UBSR_IVMR     0x00000010 // Invalid Map Register
#define UBSR_MRPF     0x00000008 // Map Register Parity Failure
#define UBSR_LEB      0x00000004 // Lost Error
#define UBSR_UBSTO    0x00000002 // UNIBUS Select Timeout
#define UBSR_UBBSYNTO 0x00000001 // UNIBUS Slave Sync Timeout

// UBA_BRRVR

#define UBBRRVR_AIRI  0x80000000 // Adapter Interrupt Request
#define UBBRRVR_DIV   0x0000FFFF // Device Interrupt Vector Field

// UBA_DPR

#define UBDPR_BNE     0x80000000 // Buffer Not Empty - Purge
#define UBDPR_BTE     0x40000000 // Buffer Transfer Error
#define UBDPR_DPF     0x20000000 // DP Function (RO)
#define UBDPR_BS      0x007F0000 // Buffer State Field
#define UBDPR_BUBA    0x0000FFFF // Buffered Unibus Address

#define UBDPR_ERROR   0x80000000 // Error Occured
#define UBDPR_NXM     0x40000000 // Non-Existant Memory
#define UBDPR_UCE     0x20000000 // Uncorrectable Error
#define UBDPR_PURGE   0x00000001 // Purge BDP

// UBA_MR - Unibus Map Register
#define UBMR_MRV      0x80000000 // Map Register Valid
#define UBMR_BO       0x02000000 // Byte Offset Bit
#define UBMR_DPDB     0x01E00000 // Data Path Designator Field
#define UBMR_SBIPFN   0x000FFFFF // SBI Page Address Field

#define UBMR_P_DPDB   21         // Position of Data Shift Desinator
