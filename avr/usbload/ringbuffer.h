
#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_


#define ringbuffer_size 8

extern int rb_count;

#define rb_free() (ringbuffer_size - rb_count)
#define rb_isfull() (rb_count == ringbuffer_size)
#define rb_isempty() (rb_count == 0)

void rb_init(void);
void rb_put(char el);
char rb_get(void);
void rb_flush(void);

#endif
