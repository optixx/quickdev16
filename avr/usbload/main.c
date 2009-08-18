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
#include <avr/pgmspace.h>       
#include <avr/eeprom.h>

#include "usbdrv.h"
#include "oddebug.h"            
#include "config.h"
#include "requests.h"           
#include "uart.h"
#include "sram.h"
#include "debug.h"
#include "info.h"
#include "dump.h"
#include "crc.h"
#include "usb_bulk.h"
#include "timer.h"
#include "watchdog.h"
#include "rle.h"
#include "loader.h"
#include "command.h"
#include "shared_memory.h"
#include "testing.h"



extern const char _rom[] PROGMEM;
extern FILE uart_stdout;

uint8_t debug_level = (DEBUG | DEBUG_USB | DEBUG_CRC | DEBUG_FAT);

uint8_t read_buffer[TRANSFER_BUFFER_SIZE];
uint32_t req_addr = 0;
uint32_t req_addr_end = 0;
uint32_t req_size;
uint8_t req_bank;
uint32_t req_bank_size;
uint16_t req_bank_cnt;
uint8_t req_percent;
uint8_t req_percent_last;
uint8_t req_state = REQ_STATUS_IDLE;
uint8_t rx_remaining = 0;
uint8_t tx_remaining = 0;
uint16_t sync_errors = 0;
uint8_t tx_buffer[32];
uint8_t data_buffer[4];
uint32_t addr;
uint16_t crc = 0;




usbMsgLen_t usbFunctionSetup(uchar data[8])
{

    usbRequest_t *rq = (void *) data;
    uint8_t ret_len = 0;

    if (rq->bRequest == USB_BULK_UPLOAD_INIT) {

        req_bank = 0;
        rx_remaining = 0;
        debug(DEBUG_USB, "USB_BULK_UPLOAD_INIT: %i %i\n", rq->wValue.word,
              rq->wIndex.word);
        req_bank_size = (uint32_t) (1L << rq->wValue.word);
        req_bank_cnt = rq->wIndex.word;
        req_addr_end = (uint32_t) req_bank_size *req_bank_cnt;
        req_percent = 0;
        req_percent_last = 0;
        sync_errors = 0;
        debug(DEBUG_USB,
              "USB_BULK_UPLOAD_INIT: bank_size=0x%08lx bank_cnt=0x%x end_addr=0x%08lx\n",
              req_bank_size, req_bank_cnt, req_addr_end);

        shared_memory_write(SHARED_MEM_TX_CMD_BANK_COUNT, req_bank_cnt);
        if (req_addr == 0x000000) {
            timer_start();
        }
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_BULK_UPLOAD_ADDR) {

        req_state = REQ_STATUS_BULK_UPLOAD;
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
        rx_remaining = rq->wLength.word;


        if (req_addr && req_addr % req_bank_size == 0) {
#ifdef FLT_DEBUG
            debug(DEBUG_USB,
                  "USB_BULK_UPLOAD_ADDR: req_bank=0x%02x addr=0x%08lx time=%.4f\n",
                  req_bank, req_addr, timer_stop());
#else
            debug(DEBUG_USB,
                  "USB_BULK_UPLOAD_ADDR: req_bank=0x%02x addr=0x%08lx time=%i\n",
                  req_bank, req_addr, timer_stop_int());
#endif
            req_bank++;
            shared_memory_write(SHARED_MEM_TX_CMD_UPLOAD_PROGESS, req_bank);
            sram_bulk_write_start(req_addr);
            timer_start();

        } else {
            sram_bulk_write_start(req_addr);
        }
        ret_len = USB_MAX_TRANS;

        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_BULK_UPLOAD_NEXT) {

        req_state = REQ_STATUS_BULK_UPLOAD;
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
        rx_remaining = rq->wLength.word;

        req_percent = (uint32_t)( 100 * req_addr )  / req_addr_end;
        if (req_percent!=req_percent_last){
            //debug(DEBUG_USB,
            //    "USB_BULK_UPLOAD_ADDR: precent=%i\n",  req_percent);
            shared_memory_write(SHARED_MEM_TX_CMD_UPLOAD_PROGESS, req_percent);
            sram_bulk_write_start(req_addr);
        }
        req_percent_last = req_percent;

#if 0
        if (req_addr && (req_addr % 0x1000) == 0) {
            debug(DEBUG_USB,
                  "USB_BULK_UPLOAD_NEXT: bank=0x%02x addr=0x%08lx crc=%04x\n",
                  req_bank, req_addr, crc_check_bulk_memory(req_addr - 0x1000,
                                                            req_addr,
                                                            req_bank_size));

        }
        sram_bulk_write_start(req_addr);
#endif
        if (req_addr && (req_addr % req_bank_size) == 0) {
#ifdef FLT_DEBUG
            debug(DEBUG_USB,
                  "USB_BULK_UPLOAD_NEXT: req_bank=0x%02x addr=0x%08lx time=%.4f\n",
                  req_bank, req_addr, timer_stop());
#else
            debug(DEBUG_USB,
                  "USB_BULK_UPLOAD_NEXT: req_bank=0x%02x addr=0x%08lx time=%i\n",
                  req_bank, req_addr, timer_stop_int());
#endif
            req_bank++;
            timer_start();
            shared_memory_write(SHARED_MEM_TX_CMD_BANK_CURRENT, req_bank);
            sram_bulk_write_start(req_addr);

        }
        ret_len = USB_MAX_TRANS;
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_BULK_UPLOAD_END) {
        if (req_state != REQ_STATUS_BULK_UPLOAD) {
            debug(DEBUG_USB,
                  "USB_BULK_UPLOAD_END: ERROR state is not REQ_STATUS_BULK_UPLOAD\n");
            return 0;
        }
        debug(DEBUG_USB, "USB_BULK_UPLOAD_END:\n");
        req_state = REQ_STATUS_IDLE;
        sram_bulk_write_end();
        shared_memory_write(SHARED_MEM_TX_CMD_UPLOAD_END, 0);
        ret_len = 0;

        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_CRC) {
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
        debug(DEBUG_USB, "USB_CRC: addr=0x%08lx \n", req_addr);
        crc_check_bulk_memory(0x000000, req_addr, req_bank_size);
        ret_len = 0;
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_MODE_SNES) {
        req_state = REQ_STATUS_SNES;
        debug(DEBUG_USB, "USB_MODE_SNES:\n");
        ret_len = 0;
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_MODE_AVR) {
        req_state = REQ_STATUS_AVR;
        debug(DEBUG_USB, "USB_MODE_AVR:\n");
        ret_len = 0;
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_AVR_RESET) {
        debug(DEBUG_USB, "USB_AVR_RESET:\n");
        soft_reset();
        ret_len = 0;
        /*
         * -------------------------------------------------------------------------
         */

    } else if (rq->bRequest == USB_CRC_ADDR) {
        req_state = REQ_STATUS_CRC;
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
        debug(DEBUG_USB, "USB_CRC_ADDR: addr=0x%lx size=%i\n", req_addr,
              rq->wLength.word);
        req_size = rq->wLength.word;
        req_size = req_size << 2;
        tx_remaining = 2;
        debug(DEBUG_USB, "USB_CRC_ADDR: addr=0x%lx size=%li\n", req_addr,
              req_size);

        crc = crc_check_memory_range(req_addr, req_size, read_buffer);
        tx_buffer[0] = crc & 0xff;
        tx_buffer[1] = (crc >> 8) & 0xff;
        ret_len = 2;
        req_state = REQ_STATUS_IDLE;
    }

    usbMsgPtr = data_buffer;
    return ret_len;             /* default for not implemented requests: return no data back to host */
}


/*
 * ------------------------------------------------------------------------- 
 */


void usb_connect()
{
    uint8_t i = 0;
    info("USB init\n");
    usbDeviceDisconnect();      /* enforce re-enumeration, do this while */
    cli();
    info("USB disconnect\n");
    i = 10;
    while (--i) {               /* fake USB disconnect for > 250 ms */
        led_on();
        _delay_ms(35);
        led_off();
        _delay_ms(65);
    }
    led_on();
    usbDeviceConnect();
    info("USB connect\n");
}


void boot_startup_rom()
{

    info("Activate AVR bus\n");
    avr_bus_active();
    info("IRQ off\n");
    snes_irq_lo();
    snes_irq_off();
    snes_lorom();
    info("Set Snes lowrom \n");
    rle_decode(&_rom, ROM_BUFFER_SIZE, 0x000000);
    dump_memory(0x10000 - 0x100, 0x10000);
    snes_reset_hi();
    snes_reset_off();
    snes_irq_lo();
    snes_irq_off();
    info("IRQ off\n");
    snes_hirom();
    snes_wr_disable();
    info("Disable snes WR\n");
    snes_bus_active();
    info("Activate Snes bus\n");
    _delay_ms(20);
    send_reset();
    _delay_ms(200);
}


int main(void)
{

    uart_init();
    stdout = &uart_stdout;


    info("Sytem start\n");
    system_init();


#if 1
    avr_bus_active();
    info("Activate AVR bus\n");
    info("IRQ off\n");
    snes_irq_lo();
    snes_irq_off();
    info("Set Snes lowrom\n");
    snes_lorom();
    info("Disable snes WR\n");
    snes_wr_disable();
    test_sdcard();
#endif


    info("Boot startup rom\n");
    boot_startup_rom();


    usbInit();
    usb_connect();

    while (1) {
        
        avr_bus_active();
        info("Activate AVR bus\n");
        info("IRQ off\n");
        snes_irq_lo();
        snes_irq_off();
        info("Set Snes lowrom\n");
        snes_lorom();
        info("Disable snes WR\n");
        snes_wr_disable();
        sei();
        info("USB poll\n");
        while (req_state != REQ_STATUS_SNES) {
            usbPoll();
        }
        shared_memory_write(SHARED_MEM_TX_CMD_TERMINATE, 0);
        info("USB poll done\n");
        set_rom_mode();
        snes_wr_disable();
        info("Disable snes WR\n");
        snes_bus_active();
        info("Activate Snes bus\n");
        _delay_ms(100);
        send_reset();

        info("Poll\n");
        while (req_state != REQ_STATUS_AVR) {
            usbPoll();

#ifdef DO_IRQ
            uint8_t i;
            uint16_t irq_count = 0;
            i = 10;
            while (--i) {
                _delay_ms(100);
            }
            info("Send IRQ %i\n", ++irq_count);
            send_irq();
#endif

#ifdef DO_BUS_STEALING
            avr_bus_active();
            sram_bulk_read_start(0x003000);
            c = sram_bulk_read();
            i = 5;
            while (--i) {
                _delay_ms(500);
                info("Wait to switch to snes mode %i\n", i);
            }

            if (req_bank_size == 0x8000) {
                snes_lorom();
                info("Set Snes lowrom \n");
            } else {
                snes_hirom();
                info("Set Snes hirom \n");
            }
            snes_wr_disable();
            info("Disable snes WR\n");
            snes_bus_active();
            info("Activate Snes bus\n");
            info("Read 0x3000=%c\n", c);
#endif
        }
        info("Boot startup rom\n");
        boot_startup_rom();
    }

    return 0;
}
