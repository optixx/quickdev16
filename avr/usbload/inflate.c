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

#include "neginf/neginf.h"

#include "inflate.h"
#include "sram.h"



char inflate_done = 0;

void inflate_init(){
    neginf_init(0);
    sram_bulk_write_start(0x000000);
}

void neginf_cb_completed()
{
    inflate_done = 1;
}

void neginf_cb_seq_byte(nbyte byte)
{
    sram_bulk_write(byte);
}

uint8_t buffer[512];

void neginf_cb_copy(nsize from, nsize to, nint length)
{
    uint32_t addr;
    uint8_t c;

/*
    sram_bulk_addr_save();
    for (addr=from; addr<from+length; addr++){
        c = sram_read(addr);
        sram_write(addr,c);
    }
    sram_bulk_addr_restore();
*/

    sram_bulk_addr_save();
    sram_bulk_copy_into_buffer(from, buffer, length);
    sram_bulk_copy_from_buffer(to, buffer, length);
    sram_bulk_addr_restore();

}
