/*
chksum.h - miscellaneous checksum functions
           SHA1, MD5, CRC16 and CRC32

Copyright (c) 1999 - 2004 NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2004 dbjh

sha1 - Copyright (c) 2002, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


MD5  - Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.
       License to copy and use this software is granted provided that
       it is identified as the "RSA Data Security, Inc. MD5 Message
       Digest Algorithm" in all material mentioning or referencing this
       software or this function.
*/
#ifndef MISC_CHKSUM_H
#define MISC_CHKSUM_H

#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif
#ifdef  __cplusplus
extern "C" {
#endif
#ifdef  HAVE_INTTYPES_H
#include <inttypes.h>
#else                                           // __MSDOS__, _WIN32 (VC++)
#include "itypes.h"
#endif


/*
  s_sha1_ctx_t
  sha1_begin() start sha1
  sha1()       process data
  sha1_end()   stop sha1

  s_md5_ctx_t
  md5_init()   start md5
  md5_update() process data
  md5_final()  stop md5

  chksum_crc16()

  crc32()      a crc32() clone (if no ZLIB is used)
                 use zlib's crc32() if USE_ZLIB is defined...
                 ... but make it possible to link against a library
                 that uses zlib while this code does not use it
*/
typedef struct
{
  uint32_t count[2];
  uint32_t hash[5];
  uint32_t wbuf[16];
} s_sha1_ctx_t;

extern void sha1_begin (s_sha1_ctx_t ctx[1]);
extern void sha1 (s_sha1_ctx_t ctx[1], const unsigned char data[], unsigned int len);
extern void sha1_end (unsigned char hval[], s_sha1_ctx_t ctx[1]);


// data structure for MD5 (Message Digest) computation
typedef struct
{
  uint32_t i[2];                        // number of _bits_ handled mod 2^64
  uint32_t buf[4];                      // scratch buffer
  unsigned char in[64];                 // input buffer
  unsigned char digest[16];             // actual digest after md5_final call
} s_md5_ctx_t;

extern void md5_init (s_md5_ctx_t *mdContext, unsigned long pseudoRandomNumber);
extern void md5_update (s_md5_ctx_t *mdContext, unsigned char *inBuf, unsigned int inLen);
extern void md5_final (s_md5_ctx_t *mdContext);


#ifdef  WITH_CRC16
extern unsigned short chksum_crc16 (unsigned short crc, const void *buffer, unsigned int size);
#endif

#ifndef  USE_ZLIB
extern unsigned int crc32 (unsigned int crc, const void *buffer, unsigned int size);
#endif


#ifdef  __cplusplus
}
#endif

#endif // MISC_CHKSUM_H
