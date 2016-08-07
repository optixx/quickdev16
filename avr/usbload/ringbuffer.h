#ifndef _ringbuffer_h
#define _ringbuffer_h

int __mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

#define ring_buffer_typedef(T, NAME) \
  typedef struct { \
    int size; \
    int start; \
    int end; \
    T* elems; \
  } NAME

#define buffer_init(BUF, S, T) \
  BUF.size = S+1; \
  BUF.start = 0; \
  BUF.end = 0; \
  BUF.elems = (T*)calloc(BUF.size, sizeof(T))


#define buffer_destroy(BUF) free(BUF->elems)
#define nex_start_index(BUF) ((BUF->start + 1) % BUF->size)
#define is_buffer_empty(BUF) (BUF->end == BUF->start)
#define buffer_get(BUF, INDEX) (BUF->elems[__mod(BUF->end - INDEX, BUF->size)])

#define bufferWrite(BUF, ELEM) \
  BUF->elems[BUF->end] = ELEM; \
  BUF->end = (BUF->end + 1) % BUF->size; \
  if (is_buffer_empty(BUF)) { \
    BUF->start = nex_start_index(BUF); \
  }

#define bufferRead(BUF, ELEM) \
    ELEM = BUF->elems[BUF->start]; \
    BUF->start = nex_start_index(BUF);

#endif

