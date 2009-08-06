

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
#include "info.h"

uint8_t irq_addr_lo;
uint8_t irq_addr_hi;

uint8_t scratchpad_state;
uint8_t scratchpad_cmd;
uint8_t scratchpad_payload;

void shared_memory_scratchpad_tx_save()
{
    scratchpad_state = sram_read(SHARED_MEM_TX_LOC_STATE);
    scratchpad_cmd = sram_read(SHARED_MEM_TX_LOC_CMD);
    scratchpad_payload = sram_read(SHARED_MEM_TX_LOC_PAYLOAD);
}

void shared_memory_scratchpad_tx_restore()
{
    sram_write(SHARED_MEM_TX_LOC_STATE, scratchpad_state);
    sram_write(SHARED_MEM_TX_LOC_CMD, scratchpad_cmd);
    sram_write(SHARED_MEM_TX_LOC_PAYLOAD, scratchpad_payload);
}

void shared_memory_irq_hook()
{
    irq_addr_lo = sram_read(SHARED_IRQ_LOC_LO);
    irq_addr_hi = sram_read(SHARED_IRQ_LOC_HI);
    sram_write(SHARED_IRQ_HANDLER_LO, 0);
    sram_write(SHARED_IRQ_HANDLER_HI, 0);
}

void shared_memory_irq_restore()
{
    sram_write(SHARED_IRQ_LOC_LO, irq_addr_lo);
    sram_write(SHARED_IRQ_LOC_HI, irq_addr_hi);
}

void shared_memory_write(uint8_t cmd, uint8_t value)
{

    info("Write shared memory 0x%04x=0x%02x 0x%04x=0x%02x \n",
         SHARED_MEM_TX_LOC_CMD, cmd, SHARED_MEM_TX_LOC_PAYLOAD, value);

    shared_memory_scratchpad_tx_save();
    shared_memory_irq_hook();

    sram_write(SHARED_MEM_TX_LOC_STATE, SHARED_MEM_TX_SNES_ACK);
    sram_write(SHARED_MEM_TX_LOC_CMD, cmd);
    sram_write(SHARED_MEM_TX_LOC_PAYLOAD, value);

    snes_hirom();
    snes_wr_disable();
    snes_bus_active();
    _delay_ms(50);


    avr_bus_active();
    snes_irq_lo();
    snes_irq_off();
    snes_lorom();
    snes_wr_disable();

    shared_memory_scratchpad_tx_restore();
    shared_memory_irq_restore();

}
