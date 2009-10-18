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




#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>      /* for sei() */
#include <avr/wdt.h>


#include "usbdrv.h"
#include "oddebug.h"            /* This is also an example for using debug
                                 * macros */
#include "debug.h" 
#include "info.h"
#include "sram.h"
#include "system.h"

extern system_t system;
  
void (*jump_to_app) (void) = 0x0000;
  
void irq_init(){
    cli();
    PCMSK3 |=(1<<PCINT27);
    PCICR  |= (1<<PCIE3);
    sei();
    system.reset_irq = RESET_IRQ_ON;
} 

void irq_stop(){
    cli();
    PCMSK3 &=~(1<<PCINT27);
    sei();
    system.reset_irq = RESET_IRQ_OFF;
} 

void leave_application(void)
{
    cli();
    usbDeviceDisconnect();
    system.avr_reset_count++;
    wdt_enable(WDTO_15MS);
    while (1);

}

ISR (SIG_PIN_CHANGE3)
{
    if (snes_reset_test()){
        info_P(PSTR("Catch SNES reset button\n"));
        info_P(PSTR("Set watchdog...\n"));
        leave_application();
    }    
}
 
