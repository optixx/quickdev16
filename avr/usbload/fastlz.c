
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
int fastlz_decompress(const void* input, int length, void* output);

#define MAX_DISTANCE  256


#include <avr/pgmspace.h>       /* required by usbdrv.h */
#include <util/delay.h>         /* for _delay_ms() */
#include <avr/interrupt.h>      /* for sei() */

#include "sram.h"
#include "debug.h"
#include "info.h"
#include "ringbuffer.h"

//#define log2(NUM) printf("%i op=%i(%x) ip=%i(%x) ref=%i(%x) dist=%i buf->end=%i len=%i ctrl=%i ofs=%i(%i) limit=%i\n",NUM, output_index,output[output_index], input_index,input[input_index],ref_index,input[ref_index],output_index - ref_index,ref_buffer_ptr->end, len, ctrl, ofs, ofs>>6,input_index < ip_limit); 

#define log2(NUM) info_P(PSTR("%i op=%i ip=%i ref=%i dist=%i buf->end=%i len=%i ctrl=%i ofs=%i(%i) limit=%i\n"),NUM, output_index, input_index,ref_index,output_index - ref_index,ref_buffer_ptr->end, len, ctrl, ofs, ofs>>6,input_index < ip_limit); 

#define OUTPUT_INC(B)  do { \
    __b = B;\
    sram_bulk_write(__b);\
    sram_bulk_write_next();\
    bufferWrite(ref_buffer_ptr, __b);\
    output_index++;\
} while (0)

#define OUTPUT_INC_FROM_REFINC()  do { \
    __dist = (output_index-ref_index); \
    __c = buffer_get(ref_buffer_ptr, __dist); \
    info_P(PSTR("output_index=%i ref_index=%i(%x) dist=%i(%x) buf->end=%i buf->size=%i position=%i\n"), output_index, ref_index, __c, __dist, __c, ref_buffer_ptr->end, ref_buffer_ptr->size, __mod(ref_buffer_ptr->end - __dist, ref_buffer_ptr->size)); \
    sram_bulk_write(__c);\
    sram_bulk_write_next();\
    output_index++;\
    bufferWrite(ref_buffer_ptr, __c);\
    ref_index++;\
} while (0)


#define FROM_REF(OUT)  do { \
    flzuint16 __dist = (output_index-ref_index+1); \
    OUT = buffer_get(ref_buffer_ptr, __dist); \
} while (0)

#define OUTBYTE(OUT) do { \
    sram_bulk_write(OUT);\
    sram_bulk_write_next();\
    output_index++;\
} while(0)

#define INBYTE(IN) do { \
    cli();\
    IN = pgm_read_byte((PGM_VOID_P)input_index++);  \
    sei();\
} while(0)

#define INPUT_INC(IN)  do { \
    cli();\
    if (input_index<32768) { \
        IN = pgm_read_byte((PGM_VOID_P)input_index++);  \
    } else { \
        IN = pgm_read_byte((PGM_VOID_P)input_index-32768);  \
        input_index++; \
    }\
    sei();\
} while (0)

ring_buffer_typedef(unsigned char, byte_buffer);

int fastlz_decompress2(unsigned char* input1, unsigned char* input2, int length)
{
  flzuint32 input_index = 0;
  flzuint32 ip_limit = length;
  flzuint32 output_index = 0;
  flzuint32 ref_index = 0;
  //flzuint32 ctrl = (input[input_index++]) & 31;
  flzuint32 ctrl;
  INPUT_INC(ctrl);
  ctrl = ctrl & 31;



  int loop = 1;

  byte_buffer ref_buffer;
  buffer_init(ref_buffer, MAX_DISTANCE, unsigned char);
  byte_buffer* ref_buffer_ptr;
  ref_buffer_ptr = &ref_buffer;

  do
  {
    flzuint8 __b;
    flzuint16 __dist;
    flzuint8 __c; 
    flzuint8 tmp;

    ref_index = output_index;
    flzuint32 len = ctrl >> 5;
    flzuint32 ofs = (ctrl & 31) << 6;
    log2(1)
    if(ctrl >= 32)
    {
      len--;
      ref_index -= ofs;
      if (len == 7-1){
        INPUT_INC(tmp);
        len += tmp;
        //len += input[input_index++];
      }
      INPUT_INC(tmp);
      ref_index -= tmp;
      //ref_index -= input[input_index++];
      
      log2(1)

      if(FASTLZ_EXPECT_CONDITIONAL( input_index < ip_limit))
        INPUT_INC(ctrl);
        //ctrl = input[input_index++];
      else
        loop = 0;
      
      log2(1)

      if(ref_index == output_index)
      {
        log2(2)
        //flzuint8 b = output[ref_index-1];
        flzuint8 b;
        FROM_REF(b);
        

        OUTPUT_INC(b);
        OUTPUT_INC(b);
        OUTPUT_INC(b);
        for(; len; --len)
            OUTPUT_INC(b);
      }
      else
      {
        log2(3)
        ref_index--;
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
      INPUT_INC(tmp);
      OUTPUT_INC(tmp);
      //OUTPUT_INC(input[input_index++]);
      for(--ctrl; ctrl; ctrl--){
        log2(5)
        INPUT_INC(tmp);
        OUTPUT_INC(tmp);
        //OUTPUT_INC(input[input_index++]);
    } 

      loop = FASTLZ_EXPECT_CONDITIONAL(input_index < ip_limit);
      if (loop){
        INPUT_INC(ctrl);
        //ctrl = input[input_index++];
     }
      log2(6)
    }
  }
  while(FASTLZ_EXPECT_CONDITIONAL(loop));
  buffer_destroy(ref_buffer_ptr);
  return 0;
}
