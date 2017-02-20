// cpu_fp11.c - FP-11 Floating Point Accerlator
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

// CFCC    0170000 NPN
// SETF    0170001 NPN
// SETI    0170002 NPN
// SETD    0170011 NPN
// SETL    0170012 NPN
// LDFPS   0170100 SOP
// STFPS   0170200 SOP
// STST    0170300 SOP
// CLRF    0170400 FOP
// CLRD    0170400 FOP+D
// TSTF    0170500 FOP
// TSTD    0170500 FOP+D
// ABSF    0170600 FOP
// ABSD    0170600 FOP+D
// NEGF    0170700 FOP
// NEGD    0170700 FOP+D
// MULF    0171000 AFOP
// MULD    0171000 AFOP+D
// MODF    0171400 AFOP
// MODD    0171400 AFOP+D
// ADDF    0172000 AFOP
// ADDD    0172000 AFOP+D
// LDF     0172400 AFOP
// LDD     0172400 AFOP+D
// SUBF    0173000 AFOP
// SUBD    0173000 AFOP+D
// CMPF    0173400 AFOP
// CMPD    0173400 AFOP+D
// STF     0174000 AFOP
// STD     0174000 AFOP+D
// DIVF    0174400 AFOP
// DIVD    0174400 AFOP+D
// STEXP   0175000 ASOP
// STCFI   0175400 ASMD
// STCDI   0175400 ASMD+D
// STCFL   0175400 ASMD+L
// STCDL   0175400 ASMD+D+L
// STCFD   0176000 AFOP
// STCDF   0176000 AFOP+D
// LDEXP   0176400 ASOP
// LDCFI   0177000 ASMD
// LDCDI   0177000 ASMD+D
// LDCFL   0177000 ASMD+L
// LDCDL   0177000 ASMD+D+L
// LDCFD   0177400 AFOP
// LDCDF   0177400 AFOP+D
