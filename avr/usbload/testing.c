

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
#include <util/delay.h>

#include "shared_memory.h"
#include "config.h"
#include "sram.h"
#include "debug.h"
#include "crc.h"
#include "info.h"
#include "dump.h"

void test_read_write()
{

    uint8_t i;
    uint32_t addr;
    avr_bus_active();
    addr = 0x000000;
    i = 1;
    while (addr++ <= 0x0000ff) {
        sram_write(addr, i++);
    }
    addr = 0x000000;
    while (addr++ <= 0x0000ff) {
        info_P(PSTR("read addr=0x%08lx %x\n"), addr, sram_read(addr));
    }
}



void test_bulk_read_write()
{

    uint8_t i;
    uint32_t addr;
    avr_bus_active();
    addr = 0x000000;
    i = 0;
    sram_bulk_write_start(addr);
    while (addr++ <= 0x8000) {
        sram_bulk_write(i++);
        sram_bulk_write_next();
    }
    sram_bulk_write_end();

    addr = 0x000000;
    sram_bulk_read_start(addr);
    while (addr <= 0x8000) {
        info_P(PSTR("addr=0x%08lx %x\n"), addr, sram_bulk_read());
        sram_bulk_read_next();
        addr++;
    }
    sram_bulk_read_end();
}


void test_non_zero_memory(uint32_t bottom_addr, uint32_t top_addr)
{
    uint32_t addr = 0;
    uint8_t c;
    sram_bulk_read_start(bottom_addr);
    for (addr = bottom_addr; addr < top_addr; addr++) {
        c = sram_bulk_read();
        if (c != 0xff)
            info_P(PSTR("addr=0x%08lx c=0x%x\n"), addr, c);
        sram_bulk_read_next();
    }
    sram_bulk_read_end();
}


void test_memory_pattern(uint32_t bottom_addr, uint32_t top_addr,
                         uint32_t bank_size)
{
    uint32_t addr = 0;
    uint8_t pattern = 0x55;
    info_P(PSTR("test_memory_pattern: bottom_addr=0x%08lx top_addr=0x%08lx\n"),
           bottom_addr, top_addr);
    sram_bulk_write_start(bottom_addr);
    for (addr = bottom_addr; addr < top_addr; addr++) {
        if (addr % bank_size == 0) {
            pattern++;
            info_P(PSTR
                   ("test_memory_pattern: write addr=0x%08lx pattern=0x%08lx\n"),
                   addr, pattern);
        }
        sram_bulk_write(pattern);
    }
    sram_bulk_write_end();


    for (addr = bottom_addr; addr < top_addr; addr += bank_size) {
        info_P(PSTR
               ("test_memory_pattern: dump bottom_addr=0x%08lx top_addr=0x%08lx\n"),
               addr, addr + bank_size);
        dump_memory(addr, addr + bank_size);
        info_P(PSTR
               ("----------------------------------------------------------------\n"));
    }

    crc_check_bulk_memory((uint32_t) bottom_addr, top_addr, bank_size);
}

void test_crc()
{
    info_P(PSTR("test_crc: clear\n"));
    avr_bus_active();
    sram_bulk_set(0x000000, 0x10000, 0xff);
    info_P(PSTR("test_crc: crc\n"));
    crc_check_bulk_memory(0x000000, 0x10000, 0x8000);
    info_P(PSTR("test_crc: check\n"));
    test_non_zero_memory(0x000000, 0x10000);
}
