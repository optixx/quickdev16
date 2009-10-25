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
 *        Created:  07/21/2009 03:32:16 PM
 *         Author:  david@optixx.org
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>

#include "neginf/neginf.h"
#include "inflate.h"
#include "loader_test.h"

extern const char _rom[];
extern char inflate_done;

void main(int argc, char **argv)
{
    
    int j;
    char c;
    inflate_init();
    for (j=0; j< ROM_ZIP_SIZE; j++){
        neginf_process_byte(_rom[j]);
    }
    while(!inflate_done)
        neginf_process_byte(0x00);
    inflate_flush();
}