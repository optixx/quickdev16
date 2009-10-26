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

#include "neginf/neginf.h"
#include "inflate.h"

char inflate_done = 0;
char *mem;
int addr = 0;


void inflate_init() 
{
    neginf_init(0);
    mem = (char*)malloc(2<<16);
    addr = 0;
}

void inflate_flush() 
{
    FILE *file;
    file = fopen("out.smc","w");
    fwrite(mem,2<<16,1,file); 
    fclose(file); 

}

void neginf_cb_completed()
{
    inflate_done = 1;
}

void neginf_cb_seq_byte(nbyte byte)
{
    mem[addr] = byte;
    addr++;
}

void neginf_cb_copy(nsize from, nsize to, nint length)
{
    int i;
    printf("neginf_cb_copy from=0x%06x to=0x%06x len=%i\n",from, to, length);
    for (i=0; i<length;i++)
        mem[to+i] = mem[from+i];
    
    addr = to + length;
}


