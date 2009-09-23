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
#include <avr/io.h>
#include <util/delay.h>         /* for _delay_ms() */
#include <avr/interrupt.h>      


#include "config.h"
#include "sram.h"
#include "system.h"
#include "uart.h"
#include "debug.h"
#include "info.h"
#include "requests.h"


void system_init(void)
{
    snes_reset_hi();
    snes_reset_off();
    snes_irq_hi();
    snes_irq_off();

}   

void system_reset()
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

void system_send_irq()
{
    snes_irq_on();
    snes_irq_lo();
    _delay_us(20);
    snes_irq_hi();
    snes_irq_off();
}

void system_bus_avr()
{
    info_P(PSTR("Activate AVR bus\n"));
    avr_bus_active();
    info_P(PSTR("Disable SNES WR\n"));
    snes_wr_disable();
}

void system_bus_snes()
{
    snes_wr_disable();
    info_P(PSTR("Disable SNES WR\n"));
    snes_bus_active();
    info_P(PSTR("Activate SNES bus\n"));
    irq_stop();
}

void system_rom_mode(usb_transaction_t *usb_trans)
{
    if (usb_trans->req_bank_size == 0x8000) {
        snes_lorom();
        info_P(PSTR("Set SNES lowrom \n"));
    } else {
        snes_hirom();
        info_P(PSTR("Set SNES hirom \n"));
    }
}

