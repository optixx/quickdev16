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

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "config.h"
#include "requests.h"
#include "sram.h"
#include "info.h"
#include "irq.h"
#include "usbdrv.h"

#if defined(BOOT_COMPRESS_RLE) 
#include "rle.h"
#endif 

#if defined(BOOT_COMPRESS_ZIP) 
#include "neginf/neginf.h"
#include "inflate.h"
#endif 

#if defined(BOOT_COMPRESS_FASTLZ) 
#include "fastlz.h"
#endif 

#include "loader.h"
#include "system.h"


extern usb_transaction_t usb_trans;
extern system_t my_system;

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

#if defined(BOOT_COMPRESS_RLE) 
    uint32_t addr = 0x000000;
#endif 

#if defined(BOOT_COMPRESS_ZIP) 
    uint8_t c;
    uint16_t j;
    PGM_VOID_P p_addr;
#endif 
    
    info_P(PSTR("Fetch Loader: %s from AVR PROGMEM\n"), LOADER_NAME);

    system_set_bus_avr();
    // snes_irq_lo();
    // system_snes_irq_off();
    system_set_rom_lorom();

    // info_P(PSTR("Activate AVR bus\n"));
    // avr_bus_active();
    // info_P(PSTR("IRQ off\n"));
    // snes_irq_lo();
    // snes_irq_off();
    // snes_lorom();

    info_P(PSTR("Unpack Loader with using method: %s\n"), LOADER_COMPRESS);
#if defined(BOOT_COMPRESS_RLE) 
    for (i = 0; i < ROM_BUFFER_CNT; i++) {
        addr += rle_decode(_rom[i], _rom_size[i], addr);
    }
#endif 

#if defined(BOOT_COMPRESS_ZIP) 
    //inflate_init();
    neginf_init(0);
    for (i=0; i<ROM_BUFFER_CNT; i++){
        p_addr = _rom[i];
        info_P(PSTR("idx=%i addr=%lx %s\n"), i, p_addr);
        for (j=0; j<_rom_size[i]; j++){
            c = pgm_read_byte((PGM_VOID_P)p_addr++); 
            neginf_process_byte(c);
            
        }
    }
#endif 

#if defined(BOOT_COMPRESS_FASTLZ) 
    uint32_t rlen = _rom_size[0]+ _rom_size[1];
    fastlz_decompress2(_rom[0], _rom[0], rlen);
#endif 

    info_P(PSTR("\n"));
    

#if DO_CRC_CHECK_LOADER
    dump_memory(0x010000 - 0x100, 0x010000);
    uint16_t crc;
    crc = crc_check_bulk_memory((uint32_t) 0x000000, 0x010000, 0x010000);
    info(PSTR("crc=%x\n"), crc);
#endif

    // snes_irq_lo();
    // snes_irq_off();
    // snes_hirom();
    // snes_wr_disable();

    // system_set_bus_snes();
    // system_set_rom_hirom();
    // system_set_wr_disable();
    // system_snes_irq_off();

    snes_irq_lo();
    system_snes_irq_off();
    system_set_rom_hirom();
    system_set_wr_disable();
    system_set_bus_snes();


    system_send_snes_reset();
    info_P(PSTR("Move Loader to wram"));
    for (i = 0; i < 30; i++) {
        _delay_ms(20);
        info_P(PSTR("."));
    }
    info_P(PSTR("\n"));
}

void banner()
{
    uint8_t i;
    for (i = 0; i < 40; i++)
        info_P(PSTR("\n"));
    info_P(PSTR
           (" ________        .__        __    ________               ____  ________\n"));
    info_P(PSTR
           (" \\_____  \\  __ __|__| ____ |  | __\\______ \\   _______  _/_   |/  _____/\n"));
    info_P(PSTR
           ("  /  / \\  \\|  |  \\  |/ ___\\|  |/ / |    |  \\_/ __ \\  \\/ /|   /   __  \\ \n"));
    info_P(PSTR
           (" /   \\_/.  \\  |  /  \\  \\___|    <  |    `   \\  ___/\\   / |   \\  |__\\  \\ \n"));
    info_P(PSTR
           (" \\_____\\ \\_/____/|__|\\___  >__|_ \\/_______  /\\___  >\\_/  |___|\\_____  / \n"));
    info_P(PSTR
           ("        \\__>             \\/     \\/        \\/     \\/                 \\/ \n"));
    info_P(PSTR("\n"));
    info_P(PSTR("                               www.optixx.org\n"));
    info_P(PSTR("\n"));
    info_P(PSTR("Hardware Version: %s Software Version: %s Build Date: %s \n"), HW_VERSION, SW_VERSION, __DATE__ );

}

void transaction_status()
{
    info_P(PSTR("\nAddr           0x%06lx\n"), usb_trans.req_addr);
    info_P(PSTR("Bank           0x%02x\n"), usb_trans.req_bank);
    info_P(PSTR("Banksize       0x%06lx\n"), usb_trans.req_bank_size);
    info_P(PSTR("Bankcount      0x%02x\n"), usb_trans.req_bank_cnt);
    info_P(PSTR("Status         0x%02x\n"), usb_trans.req_state);
    info_P(PSTR("Percent        %02i\n"), usb_trans.req_percent);
    info_P(PSTR("TX buffer      %02i\n"), usb_trans.tx_remaining);
    info_P(PSTR("RX buffer      %02i\n"), usb_trans.rx_remaining);
    info_P(PSTR("Syncerr        %02i\n"), usb_trans.sync_errors);
}
