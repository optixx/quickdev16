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

extern uint32_t req_bank_size;


void send_reset()
{
    info("Reset SNES\n");
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
    if (req_bank_size == 0x8000) {
        snes_lorom();
        info("Set SNES lowrom \n");
    } else {
        snes_hirom();
        info("Set SNES hirom \n");
    }
}
