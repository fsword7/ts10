// kl10_cca.c - CCA (Cache System)
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

// Device Name: CCA
// Device Code: 014

#include "pdp10/defs.h"
#include "pdp10/kl10.h"
#include "pdp10/iodefs.h"

#define NOPXCT 0

extern int KL10apr_srFlags;

// 70010 WRFIL (BLKO APR,)
// Write Refill Table
void KL10_ioOpcode_WRFIL(void *uptr)
{
}

// 70144 SWPIA (DATAI CCA,)
// Sweep Cache, Invalidate All Pages
void KL10_ioOpcode_SWPIA(void *uptr)
{
	// Set sweep busy bit in APR register.
	KL10apr_srFlags |= APRSR_SWPBUSY;

	// Clear all cache entries.

	// Clear sweep busy bit and set sweep done bit in APR register.
	// Also, request an interrupt.
	KL10apr_srFlags = (KL10apr_srFlags & ~APRSR_SWPBUSY) | APRSR_SWPDONE;
}

// 70150 SWPVA (BLKO CCA,)
// Sweep Cache, Validate All Pages
void KL10_ioOpcode_SWPVA(void *uptr)
{
	// Set sweep busy bit in APR register.
	KL10apr_srFlags |= APRSR_SWPBUSY;

	// Write into storage all cached words whose written bits are set.
	// Clear all written but but do not change the validity of any
	// entries.

	// Clear sweep busy bit and set sweep done bit in APR register.
	// Also, request an interrupt.
	KL10apr_srFlags = (KL10apr_srFlags & ~APRSR_SWPBUSY) | APRSR_SWPDONE;
}

// 70154 SWPUA (DATAO CCA,)
// Sweep Cache, Unload All Pages
void KL10_ioOpcode_SWPUA(void *uptr)
{
	// Set sweep busy bit in APR register.
	KL10apr_srFlags |= APRSR_SWPBUSY;

	// Write into storage all cached words whose written bits are set.
	// Invalidate the entire cache (clear all valid and written bits).

	// Clear sweep busy bit and set sweep done bit in APR register.
	// Also, request an interrupt.
	KL10apr_srFlags = (KL10apr_srFlags & ~APRSR_SWPBUSY) | APRSR_SWPDONE;
}

// 70164 SWPIO (CONI CCA,)
// Sweep Cache, Invalidate One Page
void KL10_ioOpcode_SWPIO(void *uptr)
{
	// Set sweep busy bit in APR register.
	KL10apr_srFlags |= APRSR_SWPBUSY;

	// Clear all cache entries.

	// Clear sweep busy bit and set sweep done bit in APR register.
	// Also, request an interrupt.
	KL10apr_srFlags = (KL10apr_srFlags & ~APRSR_SWPBUSY) | APRSR_SWPDONE;
}

// 70170 SWPVO (CONSZ CCA,)
// Sweep Cache, Validate One Page
void KL10_ioOpcode_SWPVO(void *uptr)
{
	// Set sweep busy bit in APR register.
	KL10apr_srFlags |= APRSR_SWPBUSY;

	// Write into storage all cached words whose written bits are set.
	// Clear all written but but do not change the validity of any
	// entries.

	// Clear sweep busy bit and set sweep done bit in APR register.
	// Also, request an interrupt.
	KL10apr_srFlags = (KL10apr_srFlags & ~APRSR_SWPBUSY) | APRSR_SWPDONE;
}

// 70174 SWPUO (CONSO CCA,)
// Sweep Cache, Unload One Page
void KL10_ioOpcode_SWPUO(void *uptr)
{
	// Set sweep busy bit in APR register.
	KL10apr_srFlags |= APRSR_SWPBUSY;

	// Write into storage all cached words whose written bits are set.
	// Invalidate the entire cache (clear all valid and written bits).

	// Clear sweep busy bit and set sweep done bit in APR register.
	// Also, request an interrupt.
	KL10apr_srFlags = (KL10apr_srFlags & ~APRSR_SWPBUSY) | APRSR_SWPDONE;
}

void kl10_InitCCA(KL10_DEVICE *kl10)
{
	P10_IOMAP *io;

	if (io = (P10_IOMAP *)calloc(1, sizeof(P10_IOMAP))) {
		io->devName    = "CCA";
		io->keyName    = "CCA";
		io->emuName    = "KL10: Cache";
		io->emuVersion = "(Internal)";
		io->idDevice   = KL10_CCA;
		io->ResetIO    = NULL;

		// Set APR instructions for KL10 processor
		io->Function[IOF_BLKI]  = NULL;
		io->Function[IOF_DATAI] = KL10_ioOpcode_SWPIA;
		io->Function[IOF_BLKO]  = KL10_ioOpcode_SWPVA;
		io->Function[IOF_DATAO] = KL10_ioOpcode_SWPUA;
		io->Function[IOF_CONO]  = NULL;
		io->Function[IOF_CONI]  = KL10_ioOpcode_SWPIO;
		io->Function[IOF_CONSZ] = KL10_ioOpcode_SWPVO;
		io->Function[IOF_CONSO] = KL10_ioOpcode_SWPUO;
		
		// Assign APR device to I/O mapping
		kx10_SetMap(io);
	}
}
