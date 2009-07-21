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



#include <stdlib.h>
#include <stdint.h>

#include "debug.h"
#include "uart.h"
#include "sram.h"


extern FILE uart_stdout;

void dump_packet(uint32_t addr, uint32_t len, uint8_t * packet)
{
    uint16_t i,j;
    uint16_t sum = 0;
    uint8_t clear = 0;

    for (i = 0; i < len; i += 16) {

        sum = 0;
        for (j = 0; j < 16; j++) {
            sum += packet[i + j];
        }
        if (!sum) {
            clear = 1;
            continue;
        }
        if (clear) {
            printf("*\n");
            clear = 0;
        }
        printf("%08lx:", addr + i);
        for (j = 0; j < 16; j++) {
            printf(" %02x", packet[i + j]);
        }
        printf(" |");
        for (j = 0; j < 16; j++) {
            if (packet[i + j] >= 33 && packet[i + j] <= 126)
                printf("%c", packet[i + j]);
            else
                printf(".");
        }
        printf("|\n");
    }
}

void dump_memory(uint32_t bottom_addr, uint32_t top_addr)
{
    uint32_t addr;
    uint8_t byte;
    sram_bulk_read_start(bottom_addr);
    for ( addr = bottom_addr; addr < top_addr; addr++) {
        if (addr%0x10 == 0)
            printf("\n%08lx:", addr);
        byte = sram_bulk_read();
        sram_bulk_read_next();
        printf(" %02x", byte);
    }
    printf("\n");
    sram_bulk_read_end();
}
