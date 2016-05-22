/*  
  FastLZ - lightning-fast lossless compression library

  Copyright (C) 2007 Ariya Hidayat (ariya@kde.org)
  Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
  Copyright (C) 2005 Ariya Hidayat (ariya@kde.org)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/*
 * Give hints to the compiler for branch prediction optimization.
 */
#if defined(__GNUC__) && (__GNUC__ > 2)
#define FASTLZ_EXPECT_CONDITIONAL(c)    (__builtin_expect((c), 1))
#define FASTLZ_UNEXPECT_CONDITIONAL(c)  (__builtin_expect((c), 0))
#else
#define FASTLZ_EXPECT_CONDITIONAL(c)    (c)
#define FASTLZ_UNEXPECT_CONDITIONAL(c)  (c)
#endif

/*
 * Use inlined functions for supported systems.
 */
#if defined(__GNUC__) || defined(__DMC__) || defined(__POCC__) || defined(__WATCOMC__) || defined(__SUNPRO_C)
#define FASTLZ_INLINE inline
#elif defined(__BORLANDC__) || defined(_MSC_VER) || defined(__LCC__)
#define FASTLZ_INLINE __inline
#else 
#define FASTLZ_INLINE
#endif

typedef unsigned char  flzuint8;
typedef unsigned short flzuint16;
typedef unsigned int   flzuint32;

/* prototypes */
int fastlz_compress(const void* input, int length, void* output);
int fastlz_decompress(const void* input, int length, void* output);

#define MAX_COPY       32
#define MAX_LEN       264  /* 256 + 8 */
#define MAX_DISTANCE  256

#define FASTLZ_READU16(p) ((p)[0] | (p)[1]<<8)

#define HASH_LOG  13
#define HASH_SIZE (1<< HASH_LOG)
#define HASH_MASK  (HASH_SIZE-1)
#define HASH_FUNCTION(v,p) { v = FASTLZ_READU16(p); v ^= FASTLZ_READU16(p+1)^(v>>(16-HASH_LOG));v &= HASH_MASK; }


FASTLZ_INLINE int fastlz_compress(const void* input, int length, void* output)
{
  const flzuint8* ip = (const flzuint8*) input;
  const flzuint8* ip_bound = ip + length - 2;
  const flzuint8* ip_limit = ip + length - 12;
  flzuint8* op = (flzuint8*) output;

  const flzuint8* htab[HASH_SIZE];
  const flzuint8** hslot;
  flzuint32 hval;

  flzuint32 copy;

  /* sanity check */
  if(FASTLZ_UNEXPECT_CONDITIONAL(length < 4))
  {
    if(length)
    {
      /* create literal copy only */
      *op++ = length-1;
      ip_bound++;
      while(ip <= ip_bound)
        *op++ = *ip++;
      return length+1;
    }
    else
      return 0;
  }

  /* initializes hash table */
  for (hslot = htab; hslot < htab + HASH_SIZE; hslot++)
    *hslot = ip;

  /* we start with literal copy */
  copy = 2;
  *op++ = MAX_COPY-1;
  *op++ = *ip++;
  *op++ = *ip++;

  /* main loop */
  while(FASTLZ_EXPECT_CONDITIONAL(ip < ip_limit))
  {
    const flzuint8* ref;
    flzuint32 distance;

    /* minimum match length */
    flzuint32 len = 3;

    /* comparison starting-point */
    const flzuint8* anchor = ip;

    /* check for a run */

    /* find potential match */
    HASH_FUNCTION(hval,ip);
    hslot = htab + hval;
    ref = htab[hval];

    /* calculate distance to the match */
    distance = anchor - ref;

    /* update hash table */
    *hslot = anchor;

    /* is this a match? check the first 3 bytes */
    if(distance==0 || 
    (distance >= MAX_DISTANCE) ||
    *ref++ != *ip++ || *ref++!=*ip++ || *ref++!=*ip++)
      goto literal;


    /* last matched byte */
    ip = anchor + len;

    /* distance is biased */
    distance--;

    if(!distance)
    {
      /* zero distance means a run */
      flzuint8 x = ip[-1];
      while(ip < ip_bound)
        if(*ref++ != x) break; else ip++;
    }
    else
    for(;;)
    {
      /* safe because the outer check against ip limit */
      if(*ref++ != *ip++) break;
      if(*ref++ != *ip++) break;
      if(*ref++ != *ip++) break;
      if(*ref++ != *ip++) break;
      if(*ref++ != *ip++) break;
      if(*ref++ != *ip++) break;
      if(*ref++ != *ip++) break;
      if(*ref++ != *ip++) break;
      while(ip < ip_bound)
        if(*ref++ != *ip++) break;
      break;
    }

    /* if we have copied something, adjust the copy count */
    if(copy)
      /* copy is biased, '0' means 1 byte copy */
      *(op-copy-1) = copy-1;
    else
      /* back, to overwrite the copy count */
      op--;

    /* reset literal counter */
    copy = 0;

    /* length is biased, '1' means a match of 3 bytes */
    ip -= 3;
    len = ip - anchor;

    /* encode the match */
    if(FASTLZ_UNEXPECT_CONDITIONAL(len > MAX_LEN-2))
      while(len > MAX_LEN-2)
      {
        *op++ = (7 << 5) + (distance >> 8);
        *op++ = MAX_LEN - 2 - 7 -2; 
        *op++ = (distance & 255);
        len -= MAX_LEN-2;
      }

    if(len < 7)
    {
      *op++ = (len << 5) + (distance >> 8);
      *op++ = (distance & 255);
    }
    else
    {
      *op++ = (7 << 5) + (distance >> 8);
      *op++ = len - 7;
      *op++ = (distance & 255);
    }

    /* update the hash at match boundary */
    HASH_FUNCTION(hval,ip);
    htab[hval] = ip++;
    HASH_FUNCTION(hval,ip);
    htab[hval] = ip++;

    /* assuming literal copy */
    *op++ = MAX_COPY-1;

    continue;

    literal:
      *op++ = *anchor++;
      ip = anchor;
      copy++;
      if(FASTLZ_UNEXPECT_CONDITIONAL(copy == MAX_COPY))
      {
        copy = 0;
        *op++ = MAX_COPY-1;
      }
  }

  /* left-over as literal copy */
  ip_bound++;
  while(ip <= ip_bound)
  {
    *op++ = *ip++;
    copy++;
    if(copy == MAX_COPY)
    {
      copy = 0;
      *op++ = MAX_COPY-1;
    }
  }

  /* if we have copied something, adjust the copy length */
  if(copy)
    *(op-copy-1) = copy-1;
  else
    op--;

  return op - (flzuint8*)output;
}


#include <stdio.h>
#define log1(NUM) printf("%i op=%i(%x) ip=%i(%x) len=%i ctrl=%i ofs=%i(%i) limit=%i\n",NUM,  (((int)op - (int)output)), *op, (((int)ip - (int)input)),*ip,  len, ctrl, ofs, (ofs >> 8),ip < ip_limit);

int fastlz_decompress(const void* input, int length, void* output)
{
  const flzuint8* ip = (const flzuint8*) input;
  const flzuint8* ip_limit  = ip + length;
  flzuint8* op = (flzuint8*) output;
  flzuint32 ctrl = (*ip++) & 31;
  int loop = 1;
  do
  {
    const flzuint8* ref = op;
    flzuint32 len = ctrl >> 5;
    flzuint32 ofs = (ctrl & 31) << 8;
    printf("-------------------\n");
    log1(1)
    if(ctrl >= 32)
    {
      len--;
      ref -= ofs;
      if (len == 7-1)
        len += *ip++;
      ref -= *ip++;
      

      log1(1)
      if(FASTLZ_EXPECT_CONDITIONAL(ip < ip_limit))
        ctrl = *ip++;
      else
        loop = 0;
      log1(1)

      if(ref == op)
      {
        log1(2)
        /* optimize copy for a run */
        flzuint8 b = ref[-1];
        *op++ = b;
        *op++ = b;
        *op++ = b;
        for(; len; --len)
          *op++ = b;
      }
      else
      {
        log1(3)
        /* copy from reference */
        ref--;
        *op++ = *ref++;
        *op++ = *ref++;
        *op++ = *ref++;

        for(; len; --len)
          *op++ = *ref++;
      }
    }
    else
    {
      ctrl++;
      log1(4)
      *op++ = *ip++; 
      for(--ctrl; ctrl; ctrl--){
        log1(5)
        *op++ = *ip++;
     }

      loop = FASTLZ_EXPECT_CONDITIONAL(ip < ip_limit);
      if(loop){
        ctrl = *ip++;
      }
      log1(6)
    }
  }
  while(FASTLZ_EXPECT_CONDITIONAL(loop));

  return op - (flzuint8*)output;

}

#include <stdlib.h>
#include <assert.h>
#include "ringbuffer.h"

#define log2(NUM) printf("%i op=%i(%x) ip=%i(%x) ref=%i(%x) dist=%i buf->end=%i len=%i ctrl=%i ofs=%i(%i) limit=%i\n",NUM, output_addr,output[output_addr], input_addr,input[input_addr],ref_addr,input[ref_addr],output_addr - ref_addr,buffer_ptr->end, len, ctrl, ofs, ofs>>6,input_addr < ip_limit); 

#define OUTPUT_INC(B)  do { \
    flzuint8 __b = B;\
    output[output_addr] = __b;\
    bufferWrite(buffer_ptr, __b);\
    output_addr++;\
} while (0)

#define OUTPUT_INC_FROM_REFINC()  do { \
    flzuint8 __b = output[ref_addr]; \
    flzuint16 __dist = (output_addr-ref_addr); \
    flzuint8 __c = bufferGet(buffer_ptr, __dist); \
    printf("output_addr=%i ref_addr=%i(%x) dist=%i(%x) buf->end=%i buf->size=%i position=%i\n", output_addr, ref_addr, __b, __dist, __c, buffer_ptr->end, buffer_ptr->size, __mod(buffer_ptr->end - __dist, buffer_ptr->size)); \
    assert(__c == __b); \
    output[output_addr] = __c;\
    bufferWrite(buffer_ptr, __c);\
    output_addr++;\
    ref_addr++;\
} while (0)


ringBuffer_typedef(unsigned char, charBuffer);

int fastlz_decompress2(unsigned char* input, int length, unsigned char* output)
{
  flzuint32 input_addr = 0;
  flzuint32 ip_limit = length;
  flzuint32 output_addr = 0;
  flzuint32 ref_addr = 0;
  flzuint32 ctrl = (input[input_addr++]) & 31;
  int loop = 1;

  charBuffer buffer;
  bufferInit(buffer, MAX_DISTANCE, unsigned char);
  charBuffer* buffer_ptr;
  buffer_ptr = &buffer;

  do
  {
    ref_addr = output_addr;
    flzuint32 len = ctrl >> 5;
    flzuint32 ofs = (ctrl & 31) << 6;
    printf("-------------------\n");
    log2(1)
    if(ctrl >= 32)
    {
      len--;
      ref_addr -= ofs;
      if (len == 7-1)
        len += input[input_addr++];
      ref_addr -= input[input_addr++];
      
      log2(1)

      if(FASTLZ_EXPECT_CONDITIONAL( input_addr < ip_limit))
        ctrl = input[input_addr++];
      else
        loop = 0;
      
      log2(1)

      if(ref_addr == output_addr)
      {
        log2(2)
        flzuint8 b = output[ref_addr-1];
        OUTPUT_INC(b);
        OUTPUT_INC(b);
        OUTPUT_INC(b);
        for(; len; --len)
            OUTPUT_INC(b);
      }
      else
      {
        log2(3)
        ref_addr--;
        OUTPUT_INC_FROM_REFINC();
        OUTPUT_INC_FROM_REFINC();
        OUTPUT_INC_FROM_REFINC();
        for(; len; --len)
            OUTPUT_INC_FROM_REFINC();
      }
    }
    else
    {
      ctrl++;
      log2(4)
      OUTPUT_INC(input[input_addr++]);
      for(--ctrl; ctrl; ctrl--){
        log2(5)
        OUTPUT_INC(input[input_addr++]);
    } 

      loop = FASTLZ_EXPECT_CONDITIONAL(input_addr < ip_limit);
      if (loop){
        ctrl = input[input_addr++];
     }
      log2(6)
    }
  }
  while(FASTLZ_EXPECT_CONDITIONAL(loop));

  return 0;
}
