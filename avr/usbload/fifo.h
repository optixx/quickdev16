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



#ifndef __FIFO_H__
#define __FIFO_H__

#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct {
    uint8_t volatile count;     // # Zeichen im Puffer
    uint8_t size;               // Puffer-Größe
    uint8_t *pread;             // Lesezeiger
    uint8_t *pwrite;            // Schreibzeiger
    uint8_t read2end, write2end;        // # Zeichen bis zum Überlauf Lese-/Schreibzeiger
} fifo_t;

extern void fifo_init(fifo_t *, uint8_t * buf, const uint8_t size);
extern uint8_t fifo_put(fifo_t *, const uint8_t data);
extern uint8_t fifo_get_wait(fifo_t *);
extern int fifo_get_nowait(fifo_t *);

static inline uint8_t _inline_fifo_put(fifo_t * f, const uint8_t data)
{
    if (f->count >= f->size)
        return 0;

    uint8_t *pwrite = f->pwrite;

    *(pwrite++) = data;

    uint8_t write2end = f->write2end;

    if (--write2end == 0) {
        write2end = f->size;
        pwrite -= write2end;
    }

    f->write2end = write2end;
    f->pwrite = pwrite;

    uint8_t sreg = SREG;
    cli();
    f->count++;
    SREG = sreg;

    return 1;
}

static inline uint8_t _inline_fifo_get(fifo_t * f)
{
    uint8_t *pread = f->pread;
    uint8_t data = *(pread++);
    uint8_t read2end = f->read2end;

    if (--read2end == 0) {
        read2end = f->size;
        pread -= read2end;
    }

    f->pread = pread;
    f->read2end = read2end;

    uint8_t sreg = SREG;
    cli();
    f->count--;
    SREG = sreg;

    return data;
}

#endif                          /* _FIFO_H_ */
