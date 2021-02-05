// This file is public domain; see md5.c for details

#ifndef MD5_H
#define MD5_H

#include "Api.h"

#ifdef __alpha
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif

struct RAINMAN_API MD5Context {
        uint32 buf[4];
        uint32 bits[2];
        unsigned char in[64];
};

//! Create a new MD5 stream
/*!
	\author Colin Plumb
	\date 1993
	\param[in] ctx Pointer to the MD5 stream to be init-ed
*/
RAINMAN_API void MD5Init(struct MD5Context *ctx);

//! Create a new MD5 stream from an ascii string
/*!
	Equivalent to \code
	MD5Init(ctx);
	MD5Update(ctx, s, strlen(s)); \endcode
	\param[in] ctx Pointer to the MD5 stream to be init-ed
	\param[in] s The ASCII string
*/
RAINMAN_API void MD5InitKey(struct MD5Context *ctx, const char* s);

//! Update an MD5 stream
/*!
	\author Colin Plumb
	\date 1993
	\param[in] ctx Pointer to the existing MD5 stream
	\param[in] buf Pointer to the data to 'append' to the hash
	\param[in] len The length of the data
*/
RAINMAN_API void MD5Update(struct MD5Context *ctx, const unsigned char *buf, unsigned len);

//! End an MD5 stream
/*!
	\author Colin Plumb
	\date 1993
	\param[in] ctx Pointer to the existing MD5 stream
	\param[in] digest String into which the hash is dumped
	\note Will reset the stream after dumping it
*/
RAINMAN_API void MD5Final(unsigned char digest[16], struct MD5Context *ctx);

//! Transform an MD5 stream
/*!
	The core of the MD5 algorithm, this alters an existing
	MD5 hash to reflect the addition of 16 longwords of new
	data. MD5Update() blocks the data and converts bytes
	into longwords for this routine.
	\author Colin Plumb
	\date 1993
*/
RAINMAN_API void MD5Transform(uint32 buf[4], uint32 in[16]);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct MD5Context MD5_CTX;

#endif /* !MD5_H */
