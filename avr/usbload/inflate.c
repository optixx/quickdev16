/*
 * =====================================================================================
 *
 * ________        .__        __    ________               ____  ________
 * \_____  \  __ __|__| ____ |  | __\______ \   _______  _/_   |/  _____/
 *  /  / \  \|  |  \  |/ ___\|  |/ / |    |  \_/ __ \  \/ /|   /   __  \
 * /   \_/.  \  |  /  \  \___|    <  |    `   \  ___/\   / |   \  |__\  \
 * \_____\ \_/____/|__|\___  >__|_ \/_______  /\___  >\_/  |___|\_____  /
 *        \__>             \/     \/        \/     \/                 \/
 *
 *                                  www.optixx.org
 *
 *
 *        Version:  1.0
 *        Created:  09/22/2009
 *         Author:  jannis@harderweb.de
 *
 * =====================================================================================
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "neginf/neginf.h"
#include "inflate.h"
#include "assert.h"
#include "ringbuffer.h"

char inflate_done = 0;
char *mem;
char *mem_ref;

int addr = 0;
int addr_ref = 0;


void inflate_init() 
{
    neginf_init(0);
    mem = (char*)malloc(2<<15);
    mem_ref = (char*)malloc(2<<15);
    addr_ref = 0;
    addr = 0;
    rb_init();
}

void inflate_flush() 
{    
    while(!rb_isempty()){

        mem[addr++] = rb_get();
        printf("final fill addr=0x%06x size=%i\n",addr,rb_free());
    }
    printf("write out.smc\n");
    FILE *file;
    file = fopen("out.smc","w");
    fwrite(mem,2<<15,1,file);
    fclose(file); 

    printf("write out_ref.smc\n");
    file = fopen("out_ref.smc","w");
    fwrite(mem_ref,2<<15,1,file);
    fclose(file);

}

void neginf_cb_completed()
{
    inflate_done = 1;
}

void neginf_cb_seq_byte(nbyte byte)
{

    mem_ref[addr_ref++] = byte; 


    if (rb_isfull())
        mem[addr++] = rb_get();
    
    printf("addr=%x byte=%i size=%i\n",addr,byte, rb_free());
    rb_put(byte);
    //assert(!rb_isfull());
}

void neginf_cb_copy(nsize from, nsize to, nint length)
{
    int i;

    printf("neginf_cb_copy addr=0x%06x from=0x%06x to=0x%06x len=%i\n",addr,from, to, length);

    for (i=0; i<length;i++){
        mem_ref[to+i] = mem_ref[from+i];
    }

    for (i=0; i<length;i++){ 
        mem[to+i] = mem_ref[from+i];
    }
    addr = to + length;
    addr_ref = to + length;
}


