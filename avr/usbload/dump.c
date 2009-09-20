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
#include <stdint.h>

#include "debug.h"
#include "info.h"
#include "uart.h"
#include "sram.h"
#include "dump.h"


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
            info_P(PSTR("*\n"));
            clear = 0;
        }
        info_P(PSTR("%08lx:"), addr + i);
        for (j = 0; j < 16; j++) {
            info_P(PSTR(" %02x"), packet[i + j]);
        }
        info_P(PSTR(" |"));
        for (j = 0; j < 16; j++) {
            if (packet[i + j] >= 33 && packet[i + j] <= 126)
                info_P(PSTR("%c"), packet[i + j]);
            else
                info_P(PSTR("."));
        }
        info_P(PSTR("|\n"));
    }
}

void dump_memory(uint32_t bottom_addr, uint32_t top_addr)
{
    uint32_t addr;
    uint8_t byte;
    sram_bulk_read_start(bottom_addr);
    for ( addr = bottom_addr; addr < top_addr; addr++) {
        if (addr%0x10 == 0)
            info_P(PSTR("\n%08lx:"), addr);
        byte = sram_bulk_read();
        sram_bulk_read_next();
        info_P(PSTR(" %02x"), byte);
    }
    info_P(PSTR("\n"));
    sram_bulk_read_end();
}
