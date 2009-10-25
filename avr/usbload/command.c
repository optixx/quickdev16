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
#include "system.h"

#include "neginf/neginf.h"
#include "inflate.h"


extern usb_transaction_t usb_trans;
extern system_t system;

extern const char *_rom[];
extern const char _rom01[];
extern const int _rom_size[];

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



void boot_startup_rom(uint16_t init_delay)
{
    uint8_t i;
    uint8_t c;
    uint16_t j;
    uint32_t addr = 0x000000;
    PGM_VOID_P p_addr;
    
    info_P(PSTR("Fetch loader rom\n"));
    
    system_set_bus_avr();
    snes_irq_lo();
    system_snes_irq_off();
    system_set_rom_lorom();
    
    
    inflate_init();
    for (i=0; i<ROM_BUFFER_CNT; i++){
        p_addr = _rom[i];
        printf("idx: %i %lx\n",i,p_addr);
        for (j=0; j<_rom_size[i]; j++){
            //rle_decode(_rom[i], _rom_size[i], addr);
            c = pgm_read_byte((PGM_VOID_P)p_addr++); 
            printf("%02x ",c);
            neginf_process_byte(c);
            
        }
    }
    info_P(PSTR("\n"));

#if DO_CRC_CHECK_LOADER     
    dump_memory(0x010000 - 0x100, 0x010000);
    uint16_t crc;
    crc = crc_check_bulk_memory((uint32_t)0x000000,0x010000, 0x010000);
    info(PSTR("crc=%x\n"),crc);
#endif

 
    snes_irq_lo();
    system_snes_irq_off();
    system_set_rom_hirom();
    system_set_wr_disable();
    system_set_bus_snes();
    
    
    system_send_snes_reset();
    _delay_ms(init_delay);
}

void banner(){
    
}

void transaction_status(){
}

