/*
bswap.h - bswap functions

Copyright (c) 1999 - 2004 NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2004 dbjh


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
*/
#ifndef MISC_BSWAP_H
#define MISC_BSWAP_H

#ifdef  HAVE_CONFIG_H
#include "config.h"                             // HAVE_BYTESWAP_H
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
  bswap_16()        12             21
  bswap_32()        1234         4321
  bswap_64()        12345678 87654321

  wswap_32()        1234         3412
  wswap_64()        12345678 78563412
*/

#ifdef  _MSC_VER
// Visual C++ doesn't allow inline in C source code
#define inline __inline
#endif

#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#else


static inline uint16_t
bswap_16 (uint16_t x)
{
#if 1
  uint8_t *ptr = (uint8_t *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[1];
  ptr[1] = tmp;
  return x;
#else
  return ((((x) & 0xff00) >> 8) |
          (((x) & 0x00ff) << 8));
#endif
}


static inline uint32_t
bswap_32 (uint32_t x)
{
#if 1
  uint8_t *ptr = (uint8_t *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[3];
  ptr[3] = tmp;
  tmp = ptr[1];
  ptr[1] = ptr[2];
  ptr[2] = tmp;
  return x;
#else
  return ((((x) & 0xff000000) >> 24) |
          (((x) & 0x00ff0000) >>  8) |
          (((x) & 0x0000ff00) <<  8) |
          (((x) & 0x000000ff) << 24));
#endif
}


static inline uint64_t
bswap_64 (uint64_t x)
{
#if 1
  uint8_t *ptr = (uint8_t *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[7];
  ptr[7] = tmp;
  tmp = ptr[1];
  ptr[1] = ptr[6];
  ptr[6] = tmp;
  tmp = ptr[2];
  ptr[2] = ptr[5];
  ptr[5] = tmp;
  tmp = ptr[3];
  ptr[3] = ptr[4];
  ptr[4] = tmp;
  return x;
#else
  return ((((x) & 0xff00000000000000ull) >> 56) |
          (((x) & 0x00ff000000000000ull) >> 40) |
          (((x) & 0x0000ff0000000000ull) >> 24) |
          (((x) & 0x000000ff00000000ull) >>  8) |
          (((x) & 0x00000000ff000000ull) <<  8) |
          (((x) & 0x0000000000ff0000ull) << 24) |
          (((x) & 0x000000000000ff00ull) << 40) |
          (((x) & 0x00000000000000ffull) << 56));
#endif
}
#endif // HAVE_BYTESWAP_H


#ifdef  WORDS_BIGENDIAN
#undef  WORDS_BIGENDIAN
#endif

#if     defined _LIBC || defined __GLIBC__
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#endif
#elif   defined AMIGA || defined __sparc__ || defined __BIG_ENDIAN__ || \
        defined __APPLE__
#define WORDS_BIGENDIAN 1
#endif

#ifdef  WORDS_BIGENDIAN
#define me2be_16
#define me2be_32
#define me2be_64
#define me2le_16 bswap_16
#define me2le_32 bswap_32
#define me2le_64 bswap_64
#else
#define me2be_16 bswap_16
#define me2be_32 bswap_32
#define me2be_64 bswap_64
#define me2le_16
#define me2le_32
#define me2le_64
#endif
#define be2me_16 me2be_16
#define be2me_32 me2be_32
#define be2me_64 me2be_64
#define le2me_16 me2le_16
#define le2me_32 me2le_32
#define le2me_64 me2le_64


static inline uint32_t
wswap_32 (uint32_t x)
{
#if 0
  uint16_t *ptr = (uint16_t *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[1];
  ptr[1] = tmp;
  return x;
#else
  return ((((x) & 0xffff0000) >> 16) |
          (((x) & 0x0000ffff) << 16));
#endif
}


#if 0
static inline uint64_t
wswap_64 (uint64_t x)
{
#if 1
  uint16_t *ptr = (uint16_t *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[3];
  ptr[3] = tmp;
  tmp = ptr[1];
  ptr[1] = ptr[2];
  ptr[2] = tmp;
  return x;
#else
  return ((((x) & 0xffff000000000000ull) >> 48) |
          (((x) & 0x0000ffff00000000ull) >> 16) |
          (((x) & 0x00000000ffff0000ull) << 16) |
          (((x) & 0x000000000000ffffull) << 48));
#endif
}
#endif


#ifdef  __cplusplus
}
#endif
#endif // MISC_BSWAP_H
