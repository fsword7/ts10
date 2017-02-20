// misc.c - Misc support routines
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

#include "emu/defs.h"
#include <time.h>

void RemoveSpaces(register char *String)
{
	register char *curString, *endString;

	if (String && *String) {
		// Remove trailing spaces
		endString = String + strlen(String) - 1;
		curString = endString;
		while(isspace(*curString) && (curString >= String))
			curString--;
		if (curString != endString)
			*(curString + 1) = 0;

		// Remove leading spaces
		curString = String;
		while(isspace(*curString) && *curString)
			curString++;
		if (curString != String)
			strcpy(String, curString);
	}
}

char *SplitChar(register char **curString, register char divider)
{
	register char *ptr, *wordString;

	if (curString) {
		ptr = *curString;
		for (wordString = ptr; *ptr && (*ptr != divider); ptr++);
		if (*ptr == divider) {
			*ptr++ = 0;
			*curString = ptr;
			return wordString;
		}
	}
	return NULL;
}

char *SplitWord(register char **curString)
{
	register char *ptr, *wordString;

	if (!curString)
		return *curString = "";
	for (ptr = *curString; isspace(*ptr); ptr++);
	for (wordString = ptr; *ptr && !isspace(*ptr); ptr++);
	if (*ptr)
		*ptr++ = 0;
	*curString = ptr;
	return wordString;
}

char *SplitQuote(register char **curString)
{
	register char *ptr, *quoteString;

	if (!curString)
		return *curString = "";
	for (ptr = *curString; isspace(*ptr); ptr++);
	if (*ptr == '"') {
		ptr++;
		for (quoteString = ptr; *ptr && (*ptr != '"'); ptr++);
		if (*ptr != '"')
			return NULL;
	} else 
		for (quoteString = ptr; !isspace(*ptr); ptr++);
	if (*ptr)
		*ptr++ = 0;
	*curString = ptr;
	return quoteString;
}

// Convert a string into a number by using desired radix.
int GetInteger(char *cptr, int radix, int max, int *status)
{
	char *tptr;
	int  val;

	val = ToInteger(cptr, &tptr, radix);
	if ((cptr == tptr) || (val > max))
		*status = EMU_ARG;
	else
		*status = EMU_OK;
	return val;
}

// Convert a string into a number by using desired radix.
int ToInteger(char *cptr, char **eptr, int radix)
{
	int c;
	int digit, nodigit;
	int val;

	*eptr = cptr;
	if ((radix < 2) || (radix > 36))
		return 0;

	while (isspace(*cptr))
		cptr++;

	val = 0;
	nodigit = 1;
	while (isalnum(c = *cptr)) {
		if (islower(c))
			c = toupper(c);
		if (isdigit(c))
			digit = c - '0';
		else
			digit = c + 10 - 'A';
		if (digit >= radix)
			return 0;
		val = (val * radix) + digit;
		cptr++;
		nodigit = 0;
	}
	if (nodigit)
		return 0;
	*eptr = cptr;
	return val;
}

#if 0
// Convert a number into a string using base 10.
// Not used - ToRadix should suffice !? - /DH
char *ToBase10(uint32 value)
{
	static char outBuffer[32];
	int idx = 31;

	outBuffer[31] = '\0';
	if (!value) {
		outBuffer[30] = '0';
		return outBuffer+30;
	}
	while (value) {
		idx--;
		outBuffer[idx] = '0' + (value % 10);
		value /= 10;
	}
	return outBuffer + idx;
}
#endif

char *ToRadix(char *str, uint32 value, int radix, int zeros)
{
	char ascValue[32];
	int  digit, idx = 0;

	// Break a value into digits.
	while (value) {
		digit = value % radix;
		ascValue[idx++] = (digit > 9 ? 'A' - 10 : '0') + digit;
		value /= radix;
	}

	// Set least one on number of leading zeros
	// And fill leading zeros with least one fill. 
	if (zeros < 1) zeros = 1;
	while (idx < zeros)
		ascValue[idx++] = '0';

	// Build a string of number.
	while (idx)
		*str++ = ascValue[--idx];

	return str;
}

void ToUpper(register char *curString)
{
	register int idx;

	for (idx = 0; curString[idx]; idx++)
		curString[idx] = toupper(curString[idx]);
}

// DEC Core-Dump file.
// Format for storing 36-bits into 5 disk/tape frames.
//
//    DEC Core-Dump Mode          ANSI Compatible Mode
// |00 01 02 03 04 05 06|07     |__ 00 01 02 03 04 05 06|
//  08 09 10 11 12 13|14 15     |__ 07 08 09 10 11 12 13|
//  16 17 18 19 20|21 22 23     |__ 14 15 16 17 18 19 20|
//  24 25 26 27|28 29 30 31     |__ 21 22 23 24 25 26 27|
//  __ __ __ __ 32 33 34|35|    |35|28 29 30 31 32 33 34|
//
// Note: "|" separate the 7-bit bytes,
// "__" are unused bits (which is zeros).

int36 Convert8to36(uchar *data8)
{
	int36 data36;

	data36 = data8[0];
	data36 = (data36 << 8) | data8[1];
	data36 = (data36 << 8) | data8[2];
	data36 = (data36 << 8) | data8[3];
	data36 = (data36 << 4) | data8[4];

	return data36;
}

void Convert36to8(int36 data36, uchar *data8)
{
	data8[0] = (data36 >> 28) & 0xFF;
	data8[1] = (data36 >> 20) & 0xFF;
	data8[2] = (data36 >> 12) & 0xFF;
	data8[3] = (data36 >>  4) & 0xFF;
	data8[4] = data36 & 0xF;
}

int36 PackedASCII6(uchar *data8)
{
	int36 data36;
	int   idx;

	// Return zero for empty string or null
	if ((data8 == NULL) || (data8[0] == '\0'))
		return 0;

	// Fill first six characters
	data36 = 0;
	for (idx = 0; data8[idx] && (idx < 6); idx++)
		data36 = (data36 << 6) | ((data8[idx] - 040) & 077);

	// Fill zeros rest of 36-bit word
	if (idx < 6)
		data36 <<= (6 - idx) * 6;

	return data36;
}

int36 PackedASCII7(uchar *data8)
{
	int36 data36;
	int   idx;

	// Return zero for empty string or null
	if ((data8 == NULL) || (data8[0] == '\0'))
		return 0;

	// Fill first six characters
	data36 = 0;
	for (idx = 0; data8[idx] && (idx < 5); idx++)
		data36 = (data36 << 7) | (data8[idx] & 0177);

	// Fill zeros rest of 36-bit word
	if (idx < 5)
		data36 <<= (5 - idx) * 7;
	data36 <<= 1;

	return data36;
}

char *nowTime(cchar *Format)
{
	time_t now = time(NULL);
	
	if (Format == NULL)
		return ctime(&now);
	else {
		static char tmpTime[1024];
		struct tm   *tmTime;

		tmTime = localtime(&now);
		strftime(tmpTime, 1023, Format, tmTime);

		return tmpTime;
	}
}

void Printf(cchar *Format, ...)
{
	char tmpBuffer[1024];
	va_list Args;
	int len;

	va_start(Args, Format);
	len = vsnprintf(tmpBuffer, 1023, Format, Args);
	tmpBuffer[1023] = 0;
	va_end(Args);

	printf(tmpBuffer);
	if (emu_logFile >= 0)
		write(emu_logFile, tmpBuffer, len);
}

char *StrChar(char *str, char delim)
{
	while ((*str != delim) && (*str != '\0'))
		str++;

	return str;
}

// Evaluate parity for that data.
uint32 Parity(uint32 data, uint32 odd)
{
	for (; data != 0; data >>= 1)
		if (data & 1) odd ^= 1;
	return odd;
}

// GetSwitches - Convert ASCII to switch masks in upper, lower, or/and
//               digit cases.
//
// Input:
//   str = a pointer to a string of ASCII switches.
//
// Output:
//   usw = a pointer to 32-bit upper-case switch-mask integer.
//   lsw = a pointer to 32-bit lower-case switch-mask integer.
//   dsw = a pointer to 32-bit digit-case switch-mask integer.
//
// Return:
//   32-bit total switch summary integer.
//   Note: 0 = no switches, -1 = error.
//
//
// Switch Mask Format:
//
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// usw |           |Z|Y|X|W|U|V|T|S|R|Q|P|O|N|M|L|K|J|I|H|G|F|E|D|C|B|A|
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// lsw |           |z|y|x|w|u|v|t|s|r|q|p|o|n|m|l|k|j|i|h|g|f|e|d|c|b|a|
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// dsw |                                           |9|8|7|6|5|4|3|2|1|0|
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//      3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
//      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0

int32 GetSwitches(char *str, uint32 *usw, uint32 *lsw, uint32 *dsw)
{
	// Check if this string is a switch or not.
	// First character must be a dash (-) or no switches.
	if (*str++ != '-')
		return 0; // No switches

	for (; (isspace(*str) == 0) || (*str != '\0'); str++) {
		if (usw && isupper(*str))
			*usw |= SWMASK(*str); // Upper-case switches.
		else if (lsw && islower(*str))
			*lsw |= SWMASK(*str); // Lower-case switches.
		else if (dsw && isdigit(*str))
			*dsw |= SWMASK(*str); // Digit-case switches
		else
			return -1; // Invalid switch argument.
	}

	// Generate total switch summary and return
	return ((usw ? *usw : 0) | (lsw ? *lsw : 0) | (dsw ? *dsw : 0));
}
