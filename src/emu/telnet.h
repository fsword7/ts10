// telnet.h - Telnet Tokens Definition
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

#define BINARY          0 // Binary Transmission
#define IS              0 // Used by terminal-type negotiation
#define SEND            1 // Used by terminal-type negotiation
#define ECHO_OPTION     1 // Echo Option
#define ECHO            1 // Echo Option
#define SUPPRESS_GA     3 // Suppress Go-Ahead Option
#define SGA             3 // Suppress Go-Ahead Option
#define TIMING_MARK     6 // Timing Mark Option
#define TERMINAL_TYPE  24 // Terminal Type Option
#define EOR            25 // End of Record Option
#define LINEMODE       34 // Line Mode
#define EOR_MARK      239 // End of Record Marker
#define SE            240 // End of Subnegotiation parameters
#define NOP           241 // No Operation
#define DATA_MARK     242 // 
#define BRK           243 // Break Character
#define IP            244 // Interrupt Process
#define AO            245 // Abort Output
#define AYT           246 // Are You There?
#define EC            247 // Erase Character
#define EL            248 // Erase Line
#define GA            249 // Go Ahead
#define SB            250 // Beginning of Subnegotiation
#define WILL          251 //
#define WONT          252 //
#define DO            253 //
#define DONT          254 // 
#define IAC           255 // Interpret as Command
