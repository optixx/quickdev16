// AT90USB/ringbuffer.c
// Simple Ring-Buffer (FIFO) for Elements of type char
// S. Salewski, 19-MAR-2007

/*
t-> o
    o <-w
    x 
    x <-r
b-> x
*/

#include "ringbuffer.h"




static char buf[ringbuffer_size];
int rb_count;

#define t &buf[ringbuffer_size - 1]
#define b &buf[0]

//char *t = &buf[ringbuffer_size - 1];
//char *b = &buf[0];

char *r; // position from where we can read (if rb_count > 0)
char *w; // next free position (if rb_count < ringbuffer_size))

void rb_init(void)
{
  r = b;
  w = b;
  rb_count = 0;
  memset(buf,0,ringbuffer_size);
}

char rb_get(void)
{
  rb_count--;
  if (r > t) r = b;
  return *r++;
}


char rb_read(void)
{

  if (r > t) r = b;
  return *r++;
}


void rb_put(char el)
{
  rb_count++;
  if (w > t){
    w = b;
    printf("wrap around\n");
   }
  *w++ = el;
}
