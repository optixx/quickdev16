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
#include "irq.h"



typedef struct system_t {
    enum bus_mode_e   { MODE_AVR, MODE_SNES } bus_mode;
    enum rom_mode_e   { LOROM, HOROM } rom_mode;
    enum reset_line_e { RESET_OFF, RESET_ON } reset_line;
    enum irq_line_e   { IRQ_ON, IRQ_OFF } irq_line;
    enum wr_line_e    { WR_DISABLE, WR_ENABLE } wr_line;

    uint8_t reset_count;
} system_t;

system_t system;

void system_init(void)
{
    snes_reset_hi();
    snes_reset_off();
    system.reset_line = RESET_OFF;

    snes_irq_hi();
    snes_irq_off();
    system.irq_line = IRQ_OFF;

    system.reset_count = 0;

    snes_wr_disable();
    system.wr_line = WR_DISABLE;

    avr_bus_active();
    system.bus_mode = MODE_AVR;


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
    system.reset_count++;
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
    avr_bus_active();
    info_P(PSTR("Activate AVR bus\n"));
    system.bus_mode = MODE_AVR;
    snes_wr_disable();
    system.wr_line = WR_DISABLE;
    info_P(PSTR("Disable SNES WR\n"));
}

void system_bus_snes()
{
    snes_wr_disable();
    system.wr_line = WR_DISABLE;
    info_P(PSTR("Disable SNES WR\n"));
    snes_bus_active();
    system.bus_mode = MODE_SNES;
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

