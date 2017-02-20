// mtpr.h - VAX Privleged Registers Definition
//
// Copyright (c) 2001, Timothy M. Stark
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
//    MTPR  Move to Privileged Register
//    MFPR  Move from Privileged Register

#define PR_KSP     0   // Kernel Stack Pointer
#define PR_ESP     1   // Executive Stack Pointer
#define PR_SSP     2   // Supervisor Stack Pointer
#define PR_USP     3   // User Stack Pointer
#define PR_ISP     4   // Interrupt Stack Pointer

#define PR_P0BR    8   // P0 Base Register
#define PR_P0LR    9   // P0 Length Register
#define PR_P1BR    10  // P1 Base Register
#define PR_P1LR    11  // P1 Length Register
#define PR_SBR     12  // System Base Register
#define PR_SLR     13  // System Length Register
#define PR_PCBB    16  // Process Control Block Base
#define PR_SCBB    17  // System Control Block Base
#define PR_IPL     18  // Interrupt Priority Level
#define PR_ASTLVL  19  // AST Level
#define PR_SIRR    20  // Software Interrupt Request
#define PR_SISR    21  // Software Interrupt Summary
#define PR_IPIR    22  // KA820 Interprocessor Register
#define PR_MCSR    23  // Machine Check Status Register (VAX-11/750)
#define PR_ICCS    24  // Interval Clock Control
#define PR_NICR    25  // Next Interval Count
#define PR_ICR     26  // Interval Count
#define PR_TODR    27  // Time of Year (Optional)
#define PR_CSRS    28  // Console Storage Receiver Control Status
#define PR_CSRD    29  // Console Storage Receiver Data Buffer
#define PR_CSTS    30  // Console Storage Transmit Control Status
#define PR_CSTD    31  // Console Storage Transmit Data Buffer
#define PR_RXCS    32  // Console Terminal Receiver Control Status
#define PR_RXDB    33  // Console Terminal Receiver Data Buffer
#define PR_TXCS    34  // Console Terminal Transmit Control Status
#define PR_TXDB    35  // Console Terminal Transmit Data Buffer
#define PR_MSER    39  // Memory System Error Register
#define PR_ACCS    40  // Accelerator Control Register
#define PR_SAVISP  41  // Console Saved ISP
#define PR_SAVPC   42  // Console Saved PC
#define PR_CONPC   42  // Console Saved PC
#define PR_SAVPSL  43  // Console Saved PSL
#define PR_CONPSL  43  // Console Saved PSL
#define PR_WCSA    44  // WCS Address
#define PR_WCSB    45  // WCS Data
#define PR_SBIFS   48  // SBI Fault/Status
#define PR_SBIS    49  // SBI Silo
#define PR_SBISC   50  // SBI Silo Comparator
#define PR_SBIMT   51  // SBI Silo Maintenance
#define PR_SBIER   52  // SBI Error Register
#define PR_SBITA   53  // SBI Timeout Address Register
#define PR_SBIQC   54  // SBI Quadword Clear
#define PR_IORESET 55  // Initialize Unibus Register
#define PR_MAPEN   56  // Memory Management Enable
#define PR_TBIA    57  // Translation Buffer Invalidate All
#define PR_TBIS    58  // Translation Buffer Invalidate Single
#define PR_TBDATA  59  // Translation Buffer Data
#define PR_MBRK    60  // Microprogram Break
#define PR_PMR     61  // Performance Monitor Enable
#define PR_SID     62  // System ID Register
#define PR_TBCHK   63  // Translation Buffer Check

// KA750 Processor - Privileged Registers Definition
#define PR_TBDR    36  // Translation Buffer Diasble Regiser
#define PR_CADR    37  // Cache Disable Register
#define PR_MCESR   38  // Machine Check Error Summary Register
#define PR_CAER    39  // Cache Error Register

// KA86 Processor - Privileged Registers Definition
#define PR_PAMACC  64  // Physical Address Memory Map Access
#define PR_PAMLOC  65  // Physical Address Memory Map Location
#define PR_CSWP    66  // Cache Sweep
#define PR_MDECC   67  // MBOX Data Ecc Register
#define PR_MENA    68  // MBOX Error Enable Register
#define PR_MDCTL   69  // MBOX Data Control Register
#define PR_MCCTL   70  // MBOX Mcc Control Register
#define PR_MERG    71  // MBOX Error Generator Register
#define PR_CRBT    72  // Console Reboot
#define PR_DFI     73  // Diagnostic Fault Insertion Register
#define PR_EHSR    74  // Error Handling Status Register
#define PR_STXCS   76  // Console Storage C/S
#define PR_STXDB   77  // Console Storage D/B
#define PR_ESPA    78  // EBOX Scratchpad Address
#define PR_ESPD    79  // EBOX Scratchpad Data

// KA820 Processor - Privileged Registers Definition
#define PR_RXCS1   80  // Serial-Line Unit 1 Receive CSR
#define PR_RXDB1   81  // Serial-Line Unit 1 Receive Data Buffer
#define PR_TXCS1   82  // Serial-Line Unit 1 Transmit CSR
#define PR_TXDB1   83  // Serial-Line Unit 1 Transmit Data Buffer
#define PR_RXCS2   84  // Serial-Line Unit 2 Receive CSR
#define PR_RXDB2   85  // Serial-Line Unit 2 Receive Data Buffer
#define PR_TXCS2   86  // Serial-Line Unit 2 Transmit CSR
#define PR_TXDB2   87  // Serial-Line Unit 2 Transmit Data Buffer
#define PR_RXCS3   88  // Serial-Line Unit 3 Receive CSR
#define PR_RXDB3   89  // Serial-Line Unit 3 Receive Data Buffer
#define PR_TXCS3   90  // Serial-Line Unit 3 Transmit CSR
#define PR_TXDB3   91  // Serial-Line Unit 3 Transmit Data Buffer
#define PR_RXCD    92  // Receive Console Data from another cpu
#define PR_CACHEX  93  // Cache invalidate Register
#define PR_BINID   94  // VAXBI node ID Register
#define PR_BISTOP  95  // VAXBI Stop Register

// KA670 Processor - Privileged Registers Definition
#define PR_BCBTS   113 // Backup Cache Tag Store
#define PR_BCP1TS  114 // Primary Tag Store 1st half
#define PR_BCP2TS  115 // Primary Tag Store 2st half
#define PR_BCRFR   116 // Refresh Register
#define PR_BCIDX   117 // Index Register
#define PR_BCSTS   118 // Status
#define PR_BCCTL   119 // Control Register
#define PR_BCERR   120 // Error Address
#define PR_BCFBTS  121 // Flush backup tag store
#define PR_BCFPTS  122 // Flush primary tag store

// KA43/KA46 Processor - Privileged Registers Definition
#define PR_VINTSR  123 // vector i/f error status
#define PR_PCTAG   124 // primary cache tag store
#define PR_PCIDX   125 // primary cache index
#define PR_PCERR   126 // primary cache error address
#define PR_PCSTS   127 // primary cache status
