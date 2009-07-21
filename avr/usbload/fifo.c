/*
 * =====================================================================================
 *
 *            .d8888b  88888b.   .d88b.  .d8888b  888d888 8888b.  88888b.d88b.
 *            88K      888 "88b d8P  Y8b 88K      888P"      "88b 888 "888 "88b
 *            "Y8888b. 888  888 88888888 "Y8888b. 888    .d888888 888  888  888
 *                 X88 888  888 Y8b.          X88 888    888  888 888  888  888
 *             88888P' 888  888  "Y8888   88888P' 888    "Y888888 888  888  888
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
