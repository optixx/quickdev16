

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
#include "dump.h"
#include "info.h"
#include "crc.h"


uint8_t irq_addr_lo;
uint8_t irq_addr_hi;

uint8_t scratchpad_state;
uint8_t scratchpad_cmd;
uint8_t scratchpad_payload;

uint8_t scratchpad_region_rx[SHARED_MEM_RX_LOC_SIZE];
uint8_t scratchpad_region_tx[SHARED_MEM_TX_LOC_SIZE];

uint8_t scratchpad_locked_rx = 1;
uint8_t scratchpad_locked_tx = 1;


uint8_t shared_memory_scratchpad_region_save_helper(uint32_t addr){
    
    if(addr > (SHARED_MEM_TX_LOC_STATE +  SHARED_MEM_TX_LOC_SIZE) &&  scratchpad_locked_tx){
        debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_save_helper: open tx addr=0x%06lx\n"),addr);
        shared_memory_scratchpad_region_tx_save();
        return 0;
    }
    if(addr > (SHARED_MEM_RX_LOC_STATE +  SHARED_MEM_RX_LOC_SIZE) &&  scratchpad_locked_rx){
        debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_save_helper: open rx addr=0x%06lx\n"),addr);
        shared_memory_scratchpad_region_rx_save();
        return 0;
    }
    return 1;
}


void shared_memory_scratchpad_region_tx_save()
{
#if 0 
    uint16_t crc;
    crc = crc_check_bulk_memory((uint32_t)SHARED_MEM_TX_LOC_STATE, 
        (uint32_t)(SHARED_MEM_TX_LOC_STATE + SHARED_MEM_TX_LOC_SIZE), 0x8000);
    debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_tx_save: crc=%x\n"),crc);
#endif

    debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_tx_save: unlock\n"));
    sram_bulk_copy_into_buffer((uint32_t)SHARED_MEM_TX_LOC_STATE,scratchpad_region_tx,
        (uint32_t)SHARED_MEM_TX_LOC_SIZE);
    scratchpad_locked_tx = 0;

#if 0 
    dump_packet(SHARED_MEM_TX_LOC_STATE, SHARED_MEM_TX_LOC_SIZE, scratchpad_region_tx);
    dump_memory(SHARED_MEM_TX_LOC_STATE, SHARED_MEM_TX_LOC_STATE + SHARED_MEM_TX_LOC_SIZE);
#endif
    

}

void shared_memory_scratchpad_region_rx_save()
{
#if 0    
    uint16_t crc;
    crc = crc_check_bulk_memory((uint32_t)SHARED_MEM_RX_LOC_STATE, 
        (uint32_t)(SHARED_MEM_RX_LOC_STATE + SHARED_MEM_RX_LOC_SIZE), 0x8000);
    debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_tx_save: crc=%x\n"),crc);
#endif

    debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_rx_save: unlock\n"));
    sram_bulk_copy_into_buffer((uint32_t)SHARED_MEM_RX_LOC_STATE,scratchpad_region_rx,
        (uint32_t)SHARED_MEM_RX_LOC_SIZE);
    scratchpad_locked_rx = 0;

#if 0    
    dump_packet(SHARED_MEM_RX_LOC_STATE, SHARED_MEM_RX_LOC_SIZE, scratchpad_region_tx);
    dump_memory(SHARED_MEM_RX_LOC_STATE, SHARED_MEM_RX_LOC_STATE + SHARED_MEM_RX_LOC_SIZE);
#endif
}

void shared_memory_scratchpad_region_tx_restore()
{
    if (scratchpad_locked_tx)
        return;

    debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_tx_restore: lock\n"));
    sram_bulk_copy_from_buffer((uint32_t)SHARED_MEM_TX_LOC_STATE,scratchpad_region_tx,
        (uint32_t)SHARED_MEM_TX_LOC_SIZE);
    scratchpad_locked_tx = 1;
#if 0    
    dump_packet(SHARED_MEM_TX_LOC_STATE, SHARED_MEM_TX_LOC_SIZE, scratchpad_region_tx);
    dump_memory(SHARED_MEM_TX_LOC_STATE, SHARED_MEM_TX_LOC_STATE + SHARED_MEM_TX_LOC_SIZE);
#endif

#if 0    
    uint16_t crc;
    crc = crc_check_bulk_memory((uint32_t)SHARED_MEM_TX_LOC_STATE, 
        (uint32_t)(SHARED_MEM_TX_LOC_STATE + SHARED_MEM_TX_LOC_SIZE), 0x8000);
    debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_tx_restore: crc=%x\n"),crc);
#endif
}


void shared_memory_scratchpad_region_rx_restore()
{
    if (scratchpad_locked_rx)
        return;
    debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_tx_save: lock\n"));
    sram_bulk_copy_from_buffer((uint32_t)SHARED_MEM_RX_LOC_STATE,scratchpad_region_rx,
        (uint32_t)SHARED_MEM_RX_LOC_SIZE);
    scratchpad_locked_rx = 1;

#if 0    
    dump_packet(SHARED_MEM_RX_LOC_STATE, SHARED_MEM_TX_LOC_SIZE, scratchpad_region_rx);
    dump_memory(SHARED_MEM_RX_LOC_STATE, SHARED_MEM_TX_LOC_STATE + SHARED_MEM_RX_LOC_SIZE);
#endif

#if 1    
    uint16_t crc;
    crc = crc_check_bulk_memory((uint32_t)SHARED_MEM_RX_LOC_STATE, 
        (uint32_t)(SHARED_MEM_RX_LOC_STATE + SHARED_MEM_RX_LOC_SIZE), 0x8000);
    debug_P(DEBUG_SHM, PSTR("shared_memory_scratchpad_region_rx_restore: crc=%x\n"),crc);
#endif
}


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

    if (scratchpad_locked_tx) 
        debug_P(DEBUG_SHM, PSTR("shared_memory_write:  locked_tx\n"));
    
    debug_P(DEBUG_SHM, PSTR("shared_memory_write:  0x%04x=0x%02x 0x%04x=0x%02x \n"),
         SHARED_MEM_TX_LOC_CMD, cmd, SHARED_MEM_TX_LOC_PAYLOAD, value);

    sram_bulk_addr_save();
    shared_memory_scratchpad_tx_save();
    shared_memory_irq_hook();

    sram_write(SHARED_MEM_TX_LOC_STATE, SHARED_MEM_TX_SNES_ACK);
    sram_write(SHARED_MEM_TX_LOC_CMD, cmd);
    sram_write(SHARED_MEM_TX_LOC_PAYLOAD, value);

    snes_hirom();
    snes_wr_disable();
    snes_bus_active();

#if SHARED_MEM_SWITCH_IRQ    
    snes_irq_on();
    snes_irq_lo();
    _delay_us(20);
    snes_irq_hi();
    snes_irq_off();
#else
    _delay_ms(SHARED_MEM_SWITCH_DELAY);
#endif

    avr_bus_active();
    snes_irq_lo();
    snes_irq_off();
    snes_lorom();
    snes_wr_disable();

    shared_memory_scratchpad_tx_restore();
    shared_memory_irq_restore();
    //sram_bulk_addr_restore();

}

void shared_memory_yield()
{

    snes_hirom();
    snes_wr_disable();
    snes_bus_active();
    _delay_ms(SHARED_MEM_SWITCH_DELAY);
    avr_bus_active();
    snes_lorom();
    snes_wr_disable();

}


int shared_memory_read(uint8_t *cmd, uint8_t *len,uint8_t *buffer)
{
    uint8_t state;
    
    if (scratchpad_locked_rx) 
        debug_P(DEBUG_SHM, PSTR("shared_memory_write:  locked_tx\n"));


    state = sram_read(SHARED_MEM_RX_LOC_STATE);
    if (state != SHARED_MEM_RX_AVR_ACK){
        return 1;
    }
    sram_bulk_addr_save();

    *cmd = sram_read(SHARED_MEM_RX_LOC_CMD);
    *len = sram_read(SHARED_MEM_RX_LOC_LEN);
    debug_P(DEBUG_SHM, PSTR("shared_memory_read: 0x%04x=0x%02x 0x%04x=0x%02x \n"),
         SHARED_MEM_RX_LOC_CMD, *cmd, SHARED_MEM_RX_LOC_LEN, *len);

    sram_bulk_copy_into_buffer(SHARED_MEM_RX_LOC_PAYLOAD,buffer, *len);
    sram_write(SHARED_MEM_RX_LOC_STATE, SHARED_MEM_RX_AVR_RTS);

    snes_hirom();
    snes_wr_disable();
    snes_bus_active();

#if SHARED_MEM_SWITCH_IRQ    
    snes_irq_on();
    snes_irq_lo();
    _delay_us(20);
    snes_irq_hi();
    snes_irq_off();
#else
    _delay_ms(SHARED_MEM_SWITCH_DELAY);
#endif

    avr_bus_active();
    snes_lorom();
    snes_wr_disable();
    sram_bulk_addr_restore();
    return 0;
}
