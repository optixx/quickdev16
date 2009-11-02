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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "ringbuffer.h"
#define memory_size         65536
#define t &buf[ringbuffer_size - 1]
#define b &buf[0]

char buf[ringbuffer_size];
int rb_count;
char *memory;
int pos_mem;
int pos_head;


//char *t = &buf[ringbuffer_size - 1];
//char *b = &buf[0];

char *r; // position from where we can read (if rb_count > 0)
char *w; // next free position (if rb_count < ringbuffer_size))
char *o; // output pointer

void rb_init()
{
    r = b;
    w = b;
    o = b;
    rb_count = 0;
    memory = (char*)malloc(memory_size);
    pos_mem = 0;
    pos_head = 0;
}


void rb_dump()
{
    int i;
    printf("b=0x%02x t=0x%02x w=0x%02x o=0x%02x\n",*b,*t,*w,*o);
    for (i=0; i<ringbuffer_size; i++)
        printf("%02i 0x%02x\n",i, buf[i]);

}
void rb_flush(){
    FILE *file;
    while(!rb_isempty()){
        memory[pos_mem++] = rb_get();
    }
    printf("write out.smc\n");
    file = fopen("out.smc","w");
    fwrite(memory,memory_size,1,file);
    fclose(file);
}

char rb_get(void)
{
    rb_count--;
    if (r > t)
        r = b;
    return *r++;
}


char rb_read(int pos)
{
    char *p;
    printf("rb_read: pos_mem=%06i pos_head=%06i pos=%06i\n",
    	pos_mem, pos_head,pos);
    
    if ( pos_head - pos > ringbuffer_size){
        printf("rb_read: memory[%i]=0x%02x \n", 
            pos,
            memory[pos]);
        return memory[pos];
    }
    if (w - index >= b)
        p = w - index;
    else
        p = b + (b - ( w - index ));
    return *p;
}

void rb_copy(int from,int to,int len){
    int i;
    char c;
    for (i = from; i< to; i++){
        c = rb_read(i);
        rb_put(c);
    }
}





void rb_put(char el)
{
    pos_head++;
    rb_count++;

    if ( rb_count > ringbuffer_size){
        rb_dump();
        memory[pos_mem++]=*o++;
        if (o > t){
            o = b;
        }
    }
    printf("rb_count=%i pos_head=0x%06x add_mem=0x%06x\n",rb_count, pos_head,pos_mem);

    if (w > t){
        w = b;
    }
    *w++ = el;
}
