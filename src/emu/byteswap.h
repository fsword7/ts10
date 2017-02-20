// byteswap.h - Byte Swapping Support Routines

#ifndef _BYTESWAP_H

// Override #include <byteswap.h> for my own routines.
#define _BYTESWAP_H
#define _BITS_BYTESWAP_H

// Override any pervious __bswap definitions.
#undef __bswap_16
#undef __bswap_32
#undef __bswap_64

#define USE_ASM_BYTESWAP
#ifdef USE_ASM_BYTESWAP

static __inline__ uint16 __bswap_16(uint16 x)
{
	__asm__( "xchgb %b0,%h0" : "=q" (x) : "0" (x) );
	return x;
}

static __inline__ uint32 __bswap_32(uint32 x)
{
	__asm__( "bswap %0" : "=q" (x) : "0" (x) );
	return x;
}

static __inline__ uint64 __bswap_64(uint64 x)
{
	union {
		uint64 q;
		uint32 l[2];	
	} r, w;

	w.q = (x);
	r.l[0] = __bswap_32(w.l[1]);
	r.l[1] = __bswap_32(w.l[0]);
	return r.q;
}

#else /* USE_ASM_BYTESWAP */

// generic 16-bit byteswap
#define __bswap_16(x) \
	( (((x) & 0xFF00) >> 8) \
	| (((x) & 0x00FF) << 8) )

// generic 32-bit byteswap
#define __bswap_32(x) \
	( (((x) & 0xFF000000) >> 24) \
	| (((x) & 0x00FF0000) >> 8)  \
	| (((x) & 0x0000FF00) << 8)  \
	| (((x) & 0x000000FF) << 24) ) 

// generic 64-bit byteswap
#define __bswap_64(x) \
	( (((x) & 0xFF00000000000000ULL) >> 56) \
	| (((x) & 0x00FF000000000000ULL) >> 40) \
	| (((x) & 0x0000FF0000000000ULL) >> 24) \
	| (((x) & 0x000000FF00000000ULL) >> 8)  \
	| (((x) & 0x00000000FF000000ULL) << 8)  \
	| (((x) & 0x0000000000FF0000ULL) << 24) \
	| (((x) & 0x000000000000FF00ULL) << 40) \
	| (((x) & 0x00000000000000FFULL) << 56) )

#endif /* USE_ASM_BYTESWAP */

// Override any pervious bswap definitions
#undef bswap_16
#undef bswap_32
#undef bswap_64

#define bswap_16(x) __bswap_16(x)
#define bswap_32(x) __bswap_32(x)
#define bswap_64(x) __bswap_64(x)

#endif /* _BYTESWAP_H */
