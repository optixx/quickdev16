/*
chksum.c - miscellaneous checksum functions

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
#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#ifdef HAVE_BYTESWAP_H
//#include <byteswap.h>
//#else
#include "bswap.h"
//#endif
#ifdef  USE_ZLIB
#include <zlib.h>
#include "unzip.h"
#endif
#include "misc.h"
#include "chksum.h"


#if     (!defined TRUE || !defined FALSE)
#define FALSE 0
#define TRUE (!FALSE)
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

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


#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif // MAXBUFSIZE


#define ROTL32(x,n) (((x) << (n)) | ((x) >> (32 - (n))))

#define SHA1_BLOCK_SIZE  64
#define SHA1_DIGEST_SIZE 20
#define SHA2_GOOD         0
#define SHA2_BAD          1
#define SHA1_MASK (SHA1_BLOCK_SIZE - 1)


void
sha1_compile (s_sha1_ctx_t ctx[1])
{
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define PARITY(x,y,z) ((x) ^ (y) ^ (z))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

  uint32_t w[80], i, a, b, c, d, e, t;

  /*
    note that words are compiled from the buffer into 32-bit
    words in big-endian order so an order reversal is needed
    here on little endian machines
  */
  for (i = 0; i < SHA1_BLOCK_SIZE / 4; ++i)
    w[i] = me2be_32 (ctx->wbuf[i]);

  for (i = SHA1_BLOCK_SIZE / 4; i < 80; ++i)
    w[i] = ROTL32 (w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

  a = ctx->hash[0];
  b = ctx->hash[1];
  c = ctx->hash[2];
  d = ctx->hash[3];
  e = ctx->hash[4];

  for (i = 0; i < 80; i++)
    {
      t = a;
      a = ROTL32 (a, 5) + e + w[i];
      if (i < 20)
        a += CH (b, c, d) + 0x5a827999;
      else if (i < 40)
        a += PARITY (b, c, d) + 0x6ed9eba1;
      else if (i < 60)
        a += MAJ (b, c, d) + 0x8f1bbcdc;
      else if (i < 80)
        a += PARITY (b, c, d) + 0xca62c1d6;
      e = d;
      d = c;
      c = ROTL32 (b, 30);
      b = t;
    }

  ctx->hash[0] += a;
  ctx->hash[1] += b;
  ctx->hash[2] += c;
  ctx->hash[3] += d;
  ctx->hash[4] += e;
}


void
sha1_begin (s_sha1_ctx_t ctx[1])
{
  ctx->count[0] = ctx->count[1] = 0;
  ctx->hash[0] = 0x67452301;
  ctx->hash[1] = 0xefcdab89;
  ctx->hash[2] = 0x98badcfe;
  ctx->hash[3] = 0x10325476;
  ctx->hash[4] = 0xc3d2e1f0;
}


void
sha1 (s_sha1_ctx_t ctx[1], const unsigned char data[], unsigned int len)
{
  uint32_t pos = (uint32_t) (ctx->count[0] & SHA1_MASK),
           space = SHA1_BLOCK_SIZE - pos;
  const unsigned char *sp = data;

  if ((ctx->count[0] += len) < len)
    ++(ctx->count[1]);

  while (len >= space)                  // transfer whole blocks while possible
    {
      memcpy (((unsigned char *) ctx->wbuf) + pos, sp, space);
      sp += space;
      len -= space;
      space = SHA1_BLOCK_SIZE;
      pos = 0;
      sha1_compile (ctx);
    }

  memcpy (((unsigned char *) ctx->wbuf) + pos, sp, len);
}


void
sha1_end (unsigned char hval[], s_sha1_ctx_t ctx[1])
{
#ifdef  WORDS_BIGENDIAN
  const uint32_t mask[4] = { 0x00000000, 0xff000000, 0xffff0000, 0xffffff00 };
  const uint32_t bits[4] = { 0x80000000, 0x00800000, 0x00008000, 0x00000080 };
#else
  const uint32_t mask[4] = { 0x00000000, 0x000000ff, 0x0000ffff, 0x00ffffff };
  const uint32_t bits[4] = { 0x00000080, 0x00008000, 0x00800000, 0x80000000 };
#endif
  uint32_t i = (uint32_t) (ctx->count[0] & SHA1_MASK);

  /*
    mask out the rest of any partial 32-bit word and then set
    the next byte to 0x80. On big-endian machines any bytes in
    the buffer will be at the top end of 32 bit words, on little
    endian machines they will be at the bottom. Hence the AND
    and OR masks above are reversed for little endian systems
    Note that we can always add the first padding byte at this
    because the buffer always contains at least one empty slot
  */
  ctx->wbuf[i >> 2] = (ctx->wbuf[i >> 2] & mask[i & 3]) | bits[i & 3];

  /*
    we need 9 or more empty positions, one for the padding byte
    (above) and eight for the length count.  If there is not
    enough space pad and empty the buffer
  */
  if (i > SHA1_BLOCK_SIZE - 9)
    {
      if (i < 60)
        ctx->wbuf[15] = 0;
      sha1_compile (ctx);
      i = 0;
    }
  else                                  // compute a word index for the empty buffer positions
    i = (i >> 2) + 1;

  while (i < 14)                        // and zero pad all but last two positions
    ctx->wbuf[i++] = 0;

  // assemble the eight byte counter in big-endian format
  ctx->wbuf[14] = me2be_32 ((ctx->count[1] << 3) | (ctx->count[0] >> 29));
  ctx->wbuf[15] = me2be_32 (ctx->count[0] << 3);

  sha1_compile (ctx);

  // extract the hash value as bytes in case the hash buffer is
  // misaligned for 32-bit words
  for (i = 0; i < SHA1_DIGEST_SIZE; ++i)
    hval[i] = (unsigned char) (ctx->hash[i >> 2] >> 8 * (~i & 3));
}


// MD5
static void md5_transform (uint32_t *buf, uint32_t *in);


// Padding
static unsigned char md5_padding[64] =
  {
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

// MD5_F, MD5_G and MD5_H are basic MD5 functions: selection, majority, parity
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

// MD5_FF, MD5_GG, MD5_HH, and MD5_II transformations for rounds 1, 2, 3, and 4
// Rotation is separate from addition to prevent recomputation
#define MD5_FF(a, b, c, d, x, s, ac) {(a) += MD5_F ((b), (c), (d)) + (x) + (uint32_t)(ac); (a) = ROTL32 ((a), (s)); (a) += (b); }
#define MD5_GG(a, b, c, d, x, s, ac) {(a) += MD5_G ((b), (c), (d)) + (x) + (uint32_t)(ac); (a) = ROTL32 ((a), (s)); (a) += (b); }
#define MD5_HH(a, b, c, d, x, s, ac) {(a) += MD5_H ((b), (c), (d)) + (x) + (uint32_t)(ac); (a) = ROTL32 ((a), (s)); (a) += (b); }
#define MD5_II(a, b, c, d, x, s, ac) {(a) += MD5_I ((b), (c), (d)) + (x) + (uint32_t)(ac); (a) = ROTL32 ((a), (s)); (a) += (b); }

// constants for transformation
#define MD5_S11 7                               // round 1
#define MD5_S12 12
#define MD5_S13 17
#define MD5_S14 22
#define MD5_S21 5                               // round 2
#define MD5_S22 9
#define MD5_S23 14
#define MD5_S24 20
#define MD5_S31 4                               // round 3
#define MD5_S32 11
#define MD5_S33 16
#define MD5_S34 23
#define MD5_S41 6                               // round 4
#define MD5_S42 10
#define MD5_S43 15
#define MD5_S44 21


// basic MD5 step, md5_transform buf based on in
static void
md5_transform (uint32_t *buf, uint32_t *in)
{
  uint32_t a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  // round 1
  MD5_FF (a, b, c, d, in[0], MD5_S11, (uint32_t) 3614090360u);  // 1
  MD5_FF (d, a, b, c, in[1], MD5_S12, (uint32_t) 3905402710u);  // 2
  MD5_FF (c, d, a, b, in[2], MD5_S13, (uint32_t) 606105819u);   // 3
  MD5_FF (b, c, d, a, in[3], MD5_S14, (uint32_t) 3250441966u);  // 4
  MD5_FF (a, b, c, d, in[4], MD5_S11, (uint32_t) 4118548399u);  // 5
  MD5_FF (d, a, b, c, in[5], MD5_S12, (uint32_t) 1200080426u);  // 6
  MD5_FF (c, d, a, b, in[6], MD5_S13, (uint32_t) 2821735955u);  // 7
  MD5_FF (b, c, d, a, in[7], MD5_S14, (uint32_t) 4249261313u);  // 8
  MD5_FF (a, b, c, d, in[8], MD5_S11, (uint32_t) 1770035416u);  // 9
  MD5_FF (d, a, b, c, in[9], MD5_S12, (uint32_t) 2336552879u);  // 10
  MD5_FF (c, d, a, b, in[10], MD5_S13, (uint32_t) 4294925233u); // 11
  MD5_FF (b, c, d, a, in[11], MD5_S14, (uint32_t) 2304563134u); // 12
  MD5_FF (a, b, c, d, in[12], MD5_S11, (uint32_t) 1804603682u); // 13
  MD5_FF (d, a, b, c, in[13], MD5_S12, (uint32_t) 4254626195u); // 14
  MD5_FF (c, d, a, b, in[14], MD5_S13, (uint32_t) 2792965006u); // 15
  MD5_FF (b, c, d, a, in[15], MD5_S14, (uint32_t) 1236535329u); // 16

  // round 2
  MD5_GG (a, b, c, d, in[1], MD5_S21, (uint32_t) 4129170786u);  // 17
  MD5_GG (d, a, b, c, in[6], MD5_S22, (uint32_t) 3225465664u);  // 18
  MD5_GG (c, d, a, b, in[11], MD5_S23, (uint32_t) 643717713u);  // 19
  MD5_GG (b, c, d, a, in[0], MD5_S24, (uint32_t) 3921069994u);  // 20
  MD5_GG (a, b, c, d, in[5], MD5_S21, (uint32_t) 3593408605u);  // 21
  MD5_GG (d, a, b, c, in[10], MD5_S22, (uint32_t) 38016083u);   // 22
  MD5_GG (c, d, a, b, in[15], MD5_S23, (uint32_t) 3634488961u); // 23
  MD5_GG (b, c, d, a, in[4], MD5_S24, (uint32_t) 3889429448u);  // 24
  MD5_GG (a, b, c, d, in[9], MD5_S21, (uint32_t) 568446438u);   // 25
  MD5_GG (d, a, b, c, in[14], MD5_S22, (uint32_t) 3275163606u); // 26
  MD5_GG (c, d, a, b, in[3], MD5_S23, (uint32_t) 4107603335u);  // 27
  MD5_GG (b, c, d, a, in[8], MD5_S24, (uint32_t) 1163531501u);  // 28
  MD5_GG (a, b, c, d, in[13], MD5_S21, (uint32_t) 2850285829u); // 29
  MD5_GG (d, a, b, c, in[2], MD5_S22, (uint32_t) 4243563512u);  // 30
  MD5_GG (c, d, a, b, in[7], MD5_S23, (uint32_t) 1735328473u);  // 31
  MD5_GG (b, c, d, a, in[12], MD5_S24, (uint32_t) 2368359562u); // 32

  // round 3
  MD5_HH (a, b, c, d, in[5], MD5_S31, (uint32_t) 4294588738u);  // 33
  MD5_HH (d, a, b, c, in[8], MD5_S32, (uint32_t) 2272392833u);  // 34
  MD5_HH (c, d, a, b, in[11], MD5_S33, (uint32_t) 1839030562u); // 35
  MD5_HH (b, c, d, a, in[14], MD5_S34, (uint32_t) 4259657740u); // 36
  MD5_HH (a, b, c, d, in[1], MD5_S31, (uint32_t) 2763975236u);  // 37
  MD5_HH (d, a, b, c, in[4], MD5_S32, (uint32_t) 1272893353u);  // 38
  MD5_HH (c, d, a, b, in[7], MD5_S33, (uint32_t) 4139469664u);  // 39
  MD5_HH (b, c, d, a, in[10], MD5_S34, (uint32_t) 3200236656u); // 40
  MD5_HH (a, b, c, d, in[13], MD5_S31, (uint32_t) 681279174u);  // 41
  MD5_HH (d, a, b, c, in[0], MD5_S32, (uint32_t) 3936430074u);  // 42
  MD5_HH (c, d, a, b, in[3], MD5_S33, (uint32_t) 3572445317u);  // 43
  MD5_HH (b, c, d, a, in[6], MD5_S34, (uint32_t) 76029189u);    // 44
  MD5_HH (a, b, c, d, in[9], MD5_S31, (uint32_t) 3654602809u);  // 45
  MD5_HH (d, a, b, c, in[12], MD5_S32, (uint32_t) 3873151461u); // 46
  MD5_HH (c, d, a, b, in[15], MD5_S33, (uint32_t) 530742520u);  // 47
  MD5_HH (b, c, d, a, in[2], MD5_S34, (uint32_t) 3299628645u);  // 48

  // round 4
  MD5_II (a, b, c, d, in[0], MD5_S41, (uint32_t) 4096336452u);  // 49
  MD5_II (d, a, b, c, in[7], MD5_S42, (uint32_t) 1126891415u);  // 50
  MD5_II (c, d, a, b, in[14], MD5_S43, (uint32_t) 2878612391u); // 51
  MD5_II (b, c, d, a, in[5], MD5_S44, (uint32_t) 4237533241u);  // 52
  MD5_II (a, b, c, d, in[12], MD5_S41, (uint32_t) 1700485571u); // 53
  MD5_II (d, a, b, c, in[3], MD5_S42, (uint32_t) 2399980690u);  // 54
  MD5_II (c, d, a, b, in[10], MD5_S43, (uint32_t) 4293915773u); // 55
  MD5_II (b, c, d, a, in[1], MD5_S44, (uint32_t) 2240044497u);  // 56
  MD5_II (a, b, c, d, in[8], MD5_S41, (uint32_t) 1873313359u);  // 57
  MD5_II (d, a, b, c, in[15], MD5_S42, (uint32_t) 4264355552u); // 58
  MD5_II (c, d, a, b, in[6], MD5_S43, (uint32_t) 2734768916u);  // 59
  MD5_II (b, c, d, a, in[13], MD5_S44, (uint32_t) 1309151649u); // 60
  MD5_II (a, b, c, d, in[4], MD5_S41, (uint32_t) 4149444226u);  // 61
  MD5_II (d, a, b, c, in[11], MD5_S42, (uint32_t) 3174756917u); // 62
  MD5_II (c, d, a, b, in[2], MD5_S43, (uint32_t) 718787259u);   // 63
  MD5_II (b, c, d, a, in[9], MD5_S44, (uint32_t) 3951481745u);  // 64

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}


// set pseudoRandomNumber to zero for RFC MD5 implementation
void
md5_init (s_md5_ctx_t *mdContext, unsigned long pseudoRandomNumber)
{
  mdContext->i[0] = mdContext->i[1] = (uint32_t) 0;

  // Load magic initialization constants
  mdContext->buf[0] = (uint32_t) 0x67452301 + (pseudoRandomNumber * 11);
  mdContext->buf[1] = (uint32_t) 0xefcdab89 + (pseudoRandomNumber * 71);
  mdContext->buf[2] = (uint32_t) 0x98badcfe + (pseudoRandomNumber * 37);
  mdContext->buf[3] = (uint32_t) 0x10325476 + (pseudoRandomNumber * 97);
}


void
md5_update (s_md5_ctx_t *mdContext, unsigned char *inBuf, unsigned int inLen)
{
  uint32_t in[16];
  int mdi = 0;
  unsigned int i = 0, ii = 0;

  // Compute number of bytes mod 64
  mdi = (int) ((mdContext->i[0] >> 3) & 0x3F);

  // Update number of bits
  if ((mdContext->i[0] + ((uint32_t) inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((uint32_t) inLen << 3);
  mdContext->i[1] += ((uint32_t) inLen >> 29);

  while (inLen--)
    {
      // Add new character to buffer, increment mdi
      mdContext->in[mdi++] = *inBuf++;

      // Transform if necessary
      if (mdi == 0x40)
        {
          for (i = 0, ii = 0; i < 16; i++, ii += 4)
            in[i] = (((uint32_t) mdContext->in[ii + 3]) << 24) |
              (((uint32_t) mdContext->in[ii + 2]) << 16) |
              (((uint32_t) mdContext->in[ii + 1]) << 8) |
              ((uint32_t) mdContext->in[ii]);

          md5_transform (mdContext->buf, in);
          mdi = 0;
        }
    }
}


void
md5_final (s_md5_ctx_t *mdContext)
{
  uint32_t in[16];
  int mdi = 0;
  unsigned int i = 0, ii = 0, padLen = 0;

  // Save number of bits
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  // Compute number of bytes mod 64
  mdi = (int) ((mdContext->i[0] >> 3) & 0x3F);

  // Pad out to 56 mod 64
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  md5_update (mdContext, md5_padding, padLen);

  // Append length in bits and transform
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((uint32_t) mdContext->in[ii + 3]) << 24) |
      (((uint32_t) mdContext->in[ii + 2]) << 16) |
      (((uint32_t) mdContext->in[ii + 1]) << 8) | ((uint32_t) mdContext->in[ii]);

  md5_transform (mdContext->buf, in);

  // Store buffer in digest
  for (i = 0, ii = 0; i < 4; i++, ii += 4)
    {
      mdContext->digest[ii] = (unsigned char) (mdContext->buf[i] & 0xFF);
      mdContext->digest[ii + 1] =
        (unsigned char) ((mdContext->buf[i] >> 8) & 0xFF);
      mdContext->digest[ii + 2] =
        (unsigned char) ((mdContext->buf[i] >> 16) & 0xFF);
      mdContext->digest[ii + 3] =
        (unsigned char) ((mdContext->buf[i] >> 24) & 0xFF);
    }
}


// CRC16
//#define WITH_CRC16                            // currently not used

#if     !defined USE_ZLIB || defined WITH_CRC16

#define CRC16_POLYNOMIAL 0xa001
#define CRC32_POLYNOMIAL 0xedb88320


void
init_crc_table (void *table, unsigned int polynomial)
// works for crc16 and crc32
{
  unsigned int crc, i, j;

  for (i = 0; i < 256; i++)
    {
      crc = i;
      for (j = 8; j > 0; j--)
        if (crc & 1)
          crc = (crc >> 1) ^ polynomial;
        else
          crc >>= 1;

      if (polynomial == CRC32_POLYNOMIAL)
        ((unsigned int *) table)[i] = crc;
      else
        ((unsigned short *) table)[i] = (unsigned short) crc;
    }
}
#endif


#ifdef  WITH_CRC16

static unsigned short *crc16_table = NULL;


static void
free_crc16_table (void)
{
  free (crc16_table);
  crc16_table = NULL;
}


unsigned short
chksum_crc16 (unsigned short crc, const void *buffer, unsigned int size)
{
  unsigned char *p = (unsigned char *) buffer;

  if (!crc16_table)
    {
      crc16_table = (unsigned short *) malloc (256 * 2);
      register_func (free_crc16_table);
      init_crc_table (crc16_table, CRC16_POLYNOMIAL);
    }

  crc = ~crc;
  while (size--)
    crc = (crc >> 8) ^ crc16_table[(crc ^ *p++) & 0xff];
  return ~crc;
}
#endif


// CRC32
#ifndef USE_ZLIB

static unsigned int *crc32_table = NULL;


static void
free_crc32_table (void)
{
  free (crc32_table);
  crc32_table = NULL;
}


unsigned int
crc32 (unsigned int crc, const void *buffer, unsigned int size)
{
  unsigned char *p = (unsigned char *) buffer;

  if (!crc32_table)
    {
      crc32_table = (unsigned int *) malloc (256 * 4);
      register_func (free_crc32_table);
      init_crc_table (crc32_table, CRC32_POLYNOMIAL);
    }

  crc = ~crc;
  while (size--)
    crc = (crc >> 8) ^ crc32_table[(crc ^ *p++) & 0xff];
  return ~crc;
}
#endif
