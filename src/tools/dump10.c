// dump.c - Tools: Dump Facility
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

#include "emu/defs.h"
#include "emu/vtape.h"
#include <getopt.h>

#define LH18(x) ((uint32)((x) >> 18) & 0777777)
#define RH18(x) ((uint32)(x) & 0777777)

void PrintDump10(uint32 addr, uint8 *data, uint32 size)
{
	int36  data36;
	uint8  text[80];
	uint8  text6[7];  // SIXBIT
	uint8  text7[6];  // 7-bit ASCII
	uint8  text8[5];  // 8-bit ASCII
	uint8  text50[7]; // RADIX-50
	uint32 srad50;
	uint8  rad50[6];
	int    i, p;
	int    idx;

	for (idx = 0; idx < size; idx += 5) {
		data36 = ((uint36)data[idx] << 28) | (data[idx+1] << 20) |
		         (data[idx+2] << 12)       | (data[idx+3] << 4)  |
		         (data[idx+4] & 0x0F);

		for (i = 30, p = 0; i >= 0; i -= 6, p++)
			text6[p] = ((data36 >> i) & 077) + 040;
		text6[p] = 0;

		for (i = 29, p = 0; i >= 1; i -= 7, p++) {
			text7[p] = (data36 >> i) & 0177;
			if ((text7[p] < 32) || (text7[p] > 126))
				text7[p] = ' ';
		}
		text7[p] = 0;

		// Decipher RADIX-50 string.
		srad50 = data36;
		for (i = 0; i < 6; i++) {
			rad50[i] = srad50 % 050;
			srad50 /= 050;
		}
		for (i = 5, p = 0; i >= 0; i--)
			text50[p++] = // what an array of string! :-)
				" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.$%"[rad50[i]];
		text50[p] = '\0';

		printf("%06o:  %06o,,%06o ('%s' '%s' '%s')\n",
			addr++, LH18(data36), RH18(data36), text6, text7, text50);
	}
}

void PrintDump(uint32 addr, uint8 *data, uint32 size)
{
	uchar  ascBuffer[17];
	uchar  ch, *pasc;
	int    idxAddr, idx;

	for(idxAddr = 0; idxAddr < size;) {
		printf("%04X: ", addr + idxAddr);

		pasc = ascBuffer;
		for (idx = 0; (idx < 16) && (idxAddr < size); idx++) {
			ch = data[idxAddr++];
			printf("%c%02X", (idx == 8) ? '-' : ' ', ch);
			*pasc++ = ((ch >= 32) && (ch < 127)) ? ch : '.';
		}

		for (; idx < 16; idx++)
			printf("   ");
		*pasc = '\0';
		printf("  |%-16s|\n", ascBuffer);
	}
}

void DumpTape(VMT_TAPE *vmt)
{
	VMT_FORMAT *fmt = vmt->Format;
	char       buf[32768];
	int        rc, noFile = 0, noBlock = 0;

	// Rewind a tape
	fmt->Rewind(vmt);

	for (;;) {
		// Read a first block
		if ((rc = fmt->Read(vmt, buf, 32768)) < 0) {
			switch (rc) {
				case MT_MARK:
					printf("** Tape Mark **\n\n");
					noBlock = 0;
					noFile++;
					continue;

				case MT_EOT:
					printf("** End of Tape **\n\n");
					break;

				case MT_ERROR:
					printf("Error: (Read) %s\n", strerror(vmt->errCode));
					break;
			}
			break;
		}
		
		// Dump block contents.
		printf("File: %d  Block: %d  Length: %d (%04X) Bytes\n\n",
			noFile, noBlock++, rc, rc);
		PrintDump10(0, buf, rc);
		printf("\n");
	}
}

VMT_TAPE *OpenTape(char *fileName)
{
	VMT_TAPE *vmt;
	int      rc;

	if ((vmt = (VMT_TAPE *)calloc(1, sizeof(VMT_TAPE))) == NULL)
		return NULL;
	vmt->fileName = (char *)malloc(strlen(fileName)+1);
	strcpy(vmt->fileName, fileName);
	if (rc = vmt_OpenTape(vmt)) {
		printf("Reason: %d\n", rc);
		if (vmt->fileName)
			free(vmt->fileName);
		free(vmt);
		return NULL;
	}

	return vmt;
}

void CloseTape(VMT_TAPE *vmt)
{
	// Close a tape image file.
	vmt_CloseTape(vmt);
	if (vmt->fileName)
		free(vmt->fileName);
	free(vmt);
}

int main(int argc, char **argv)
{
	VMT_TAPE *vmt;

	if (argc < 2) {
		printf("Usage: dump10 <tape file>");
		return 1;
	}

	vmt = OpenTape(argv[1]);
	DumpTape(vmt);
	CloseTape(vmt);
	return 0;
}
