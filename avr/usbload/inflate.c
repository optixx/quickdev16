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
char *mem_ref;

int addr_ref = 0;

int cnt_hit = 0;
int cnt = 0;


void inflate_init() 
{
    neginf_init(0);
    mem_ref = (char*)malloc(2<<15);
    addr_ref = 0;
    rb_init();
}

void inflate_flush() 
{    

    rb_flush();
    FILE *file;
    printf("write out_ref.smc\n");
    file = fopen("out_ref.smc","w");
    fwrite(mem_ref,2<<15,1,file);
    fclose(file);
    printf("cnt=%i cnt_hit=%i\n",cnt,cnt_hit);
}

void neginf_cb_completed()
{
    inflate_done = 1;
}

void neginf_cb_seq_byte(nbyte byte)
{
    mem_ref[addr_ref++] = byte; 
    rb_put(byte);
}

void neginf_cb_copy(nsize from, nsize to, nint length)
{
    int i;
    cnt++;
    if ((to - from) < ( 1024 * 2 ) ){
        cnt_hit++;
    }
    printf("neginf_cb_copy from=0x%06x to=0x%06x dist=%i len=%i\n",from, to, (to - from), length);
    for (i=0; i<length;i++){
        mem_ref[to+i] = mem_ref[from+i];
    }
    addr_ref = to + length;
}


