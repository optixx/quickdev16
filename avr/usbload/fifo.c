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



#include "fifo.h"

void fifo_init(fifo_t * f, uint8_t * buffer, const uint8_t size)
{
    f->count = 0;
    f->pread = f->pwrite = buffer;
    f->read2end = f->write2end = f->size = size;
}

uint8_t fifo_put(fifo_t * f, const uint8_t data)
{
    return _inline_fifo_put(f, data);
}

uint8_t fifo_get_wait(fifo_t * f)
{
    while (!f->count);

    return _inline_fifo_get(f);
}

int fifo_get_nowait(fifo_t * f)
{
    if (!f->count)
        return -1;

    return (int) _inline_fifo_get(f);
}
