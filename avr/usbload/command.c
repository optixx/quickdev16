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

#include <avr/io.h>
#include <avr/interrupt.h>      
#include <util/delay.h>         
#include <stdlib.h>

#include "config.h"
#include "requests.h"           
#include "sram.h"
#include "info.h"
#include "irq.h"
#include "usbdrv.h"
#include "rle.h"
#include "loader.h"

extern usb_transaction_t usb_trans;

extern const char _rom[] PROGMEM;

void usb_connect()
{
    uint8_t i = 0;
    info_P(PSTR("USB init\n"));
    usbDeviceDisconnect();      /* enforce re-enumeration, do this while */
    cli();
    info_P(PSTR("USB disconnect\n"));
    i = 10;
    while (--i) {               /* fake USB disconnect for > 250 ms */
        _delay_ms(50);
    }
    led_on();
    usbDeviceConnect();
    info_P(PSTR("USB connect\n"));
}



void send_reset()
{
    info_P(PSTR("Reset SNES\n"));
    cli();
    snes_reset_on();
    snes_reset_lo();
    _delay_ms(2);
    snes_reset_hi();
    snes_reset_off();
    sei();
}

void send_irq()
{
    snes_irq_on();
    snes_irq_lo();
    _delay_us(20);
    snes_irq_hi();
    snes_irq_off();
}

void set_rom_mode()
{
    if (usb_trans.req_bank_size == 0x8000) {
        snes_lorom();
        info_P(PSTR("Set SNES lowrom \n"));
    } else {
        snes_hirom();
        info_P(PSTR("Set SNES hirom \n"));
    }
}


void boot_startup_rom(uint16_t init_delay)
{
    info_P(PSTR("Fetch loader rom\n"));
    info_P(PSTR("Activate AVR bus\n"));
    avr_bus_active();
    info_P(PSTR("IRQ off\n"));
    snes_irq_lo();
    snes_irq_off();
    snes_lorom();
    rle_decode(&_rom, ROM_BUFFER_SIZE, 0x000000);
    info_P(PSTR("\n"));

#if DO_CRC_CHECK_LOADER     
    dump_memory(0x010000 - 0x100, 0x010000);
    uint16_t crc;
    crc = crc_check_bulk_memory((uint32_t)0x000000,0x010000, 0x010000);
    info(PSTR("crc=%x\n"),crc);
#endif

    snes_irq_lo();
    snes_irq_off();
    snes_hirom();
    snes_wr_disable();
    
    snes_bus_active();
    info_P(PSTR("Activate SNES bus\n"));
    send_reset();
    _delay_ms(init_delay);
}

void banner(){
    uint8_t i;
    for (i=0;i<40;i++)
        info_P(PSTR("\n"));
    info_P(PSTR(" ________        .__        __    ________               ____  ________\n"));
    info_P(PSTR(" \\_____  \\  __ __|__| ____ |  | __\\______ \\   _______  _/_   |/  _____/\n"));
    info_P(PSTR("  /  / \\  \\|  |  \\  |/ ___\\|  |/ / |    |  \\_/ __ \\  \\/ /|   /   __  \\ \n"));
    info_P(PSTR(" /   \\_/.  \\  |  /  \\  \\___|    <  |    `   \\  ___/\\   / |   \\  |__\\  \\ \n"));
    info_P(PSTR(" \\_____\\ \\_/____/|__|\\___  >__|_ \\/_______  /\\___  >\\_/  |___|\\_____  / \n"));
    info_P(PSTR("        \\__>             \\/     \\/        \\/     \\/                 \\/ \n"));
    info_P(PSTR("\n"));
    info_P(PSTR("                               www.optixx.org\n"));
    info_P(PSTR("\n"));
    info_P(PSTR("System Hw: %s Sw: %s\n"),HW_VERSION,SW_VERSION);
    
}

void transaction_status(){
    info_P(PSTR("\nAddr           0x%06lx\n"),usb_trans.req_addr);
    info_P(PSTR("Bank           0x%02x\n"),usb_trans.req_bank);
    info_P(PSTR("Banksize       0x%06lx\n"),usb_trans.req_bank_size);
    info_P(PSTR("Bankcount      0x%02x\n"),usb_trans.req_bank_cnt);
    info_P(PSTR("Status         0x%02x\n"),usb_trans.req_state);
    info_P(PSTR("Percent        %02i\n"),usb_trans.req_percent);
    info_P(PSTR("TX buffer      %02i\n"),usb_trans.tx_remaining);
    info_P(PSTR("RX buffer      %02i\n"),usb_trans.rx_remaining);
    info_P(PSTR("Syncerr        %02i\n"),usb_trans.sync_errors);
}