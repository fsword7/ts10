// ka780_mba.c - KA780 Processor Series, Massbus Interface.
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

#include "vax/defs.h"
#include "vax/dev_mba.h"

#if 0
void ka780_ReadMBA(MBA_DEVICE *mba, uint32 pAddr, uint32 *data)
{
	pAddr &= SBI_SIZE - 1;
	switch (pAddr >> 10) {
		case 0:  // 0000 - 03FF - Nexus/Massbus Registers
			break;

		case 1:  // 0400 - 07FF - Drive Registers
			drv = (pAddr >> MBA_PDRV) & MBA_NDRV;
			reg = (pAddr >> MBA_PREG) & MBA_NREG;
			mba_ReadIO(mba, drv, reg, &data16);
			*data = data16;
			break;

		case 2:  // 0800 - 0BFF - Map Registers
			*data = mba->map[pAddr >> MBA_PREG];
			break;

		default: // 0C00 - 1FFF - Unused Area
			*data = 0;
	}
}

void ka780_WriteMBA(MBA_DEVICE *mba, uint32 pAddr, uint32 data)
{
	switch ((paddr & (SBI_SIZE-1)) >> 10) {
		case 0:  // 0000 - 03FF - Nexus/Massbus Registers
			break;

		case 1:  // 0400 - 07FF - Drive Registers
			drv = (pAddr >> MBA_PDRV) & MBA_NDRV;
			reg = (pAddr >> MBA_PREG) & MBA_NREG;
			mba_WriteIO(mba, drv, reg, data);
			break;

		case 2:  // 0800 - 0BFF - Map Registers
			mba->map[pAddr >> MBA_PREG] = data;
			break;

		default: // 0C00 - 1FFF - Unused Area
			// Do nothing here
	}
}
#endif
