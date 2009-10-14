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
#include <string.h>

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
#include "irq.h"
#include "pwm.h"
#include "testing.h"
#include "shell.h"
#include "system.h"



extern FILE uart_stdout;
extern system_t system;

uint8_t debug_level = (DEBUG | DEBUG_USB | DEBUG_CRC | DEBUG_SHM );


usb_transaction_t usb_trans;

usbMsgLen_t usbFunctionSetup(uchar data[8])
{

    usbRequest_t *rq = (void *) data;
    uint8_t ret_len = 0;

    if (rq->bRequest == USB_BULK_UPLOAD_INIT) {

        usb_trans.req_bank = 0;
        usb_trans.rx_remaining = 0;
        debug_P(DEBUG_USB, PSTR("USB_BULK_UPLOAD_INIT: %i %i\n"), rq->wValue.word,
              rq->wIndex.word);
        usb_trans.req_bank_size = (uint32_t) (1L << rq->wValue.word);
        usb_trans.req_bank_cnt = rq->wIndex.word;
        usb_trans.req_addr_end = (uint32_t) usb_trans.req_bank_size * usb_trans.req_bank_cnt;
        usb_trans.req_percent = 0;
        usb_trans.req_percent_last = 0;
        usb_trans.sync_errors = 0;
        debug_P(DEBUG_USB,
              PSTR("USB_BULK_UPLOAD_INIT: bank_size=0x%08lx bank_cnt=0x%x end_addr=0x%08lx\n"),
              usb_trans.req_bank_size, usb_trans.req_bank_cnt, usb_trans.req_addr_end);

        shared_memory_write(SHARED_MEM_TX_CMD_UPLOAD_START, 0);
        shared_memory_write(SHARED_MEM_TX_CMD_BANK_COUNT, usb_trans.req_bank_cnt);
#if DO_TIMER
        if (usb_trans.req_addr == 0x000000) {
            timer_start();
        }
#endif       
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_BULK_UPLOAD_ADDR) {

        usb_trans.req_state = REQ_STATUS_BULK_UPLOAD;
        usb_trans.req_addr = rq->wValue.word;
        usb_trans.req_addr = usb_trans.req_addr << 16;
        usb_trans.req_addr = usb_trans.req_addr | rq->wIndex.word;
        usb_trans.rx_remaining = rq->wLength.word;

        

        if (usb_trans.req_addr && usb_trans.req_addr % usb_trans.req_bank_size == 0) {
#if DO_TIMER

#ifdef FLT_DEBUG
            debug_P(DEBUG_USB,
                  PSTR("USB_BULK_UPLOAD_ADDR: req_bank=0x%02x addr=0x%08lx time=%.4f\n"),
                  usb_trans.req_bank, usb_trans.req_addr, timer_stop());
#else
            debug_P(DEBUG_USB,
                  PSTR("USB_BULK_UPLOAD_ADDR: req_bank=0x%02x addr=0x%08lx time=%i\n"),
                  usb_trans.req_bank, usb_trans.req_addr, timer_stop_int());
#endif
              timer_start();
#endif
            usb_trans.req_bank++;

        } else {
            sram_bulk_write_start(usb_trans.req_addr);
        }
        ret_len = USB_MAX_TRANS;

        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_BULK_UPLOAD_NEXT) {
        usb_trans.req_state = REQ_STATUS_BULK_UPLOAD;
        usb_trans.req_addr = rq->wValue.word;
        usb_trans.req_addr = usb_trans.req_addr << 16;
        usb_trans.req_addr = usb_trans.req_addr | rq->wIndex.word;
        usb_trans.rx_remaining = rq->wLength.word;

#if DO_SHM
        usb_trans.req_percent = (uint32_t)( 100 * usb_trans.req_addr )  / usb_trans.req_addr_end;
        if (usb_trans.req_percent!=usb_trans.req_percent_last){
            shared_memory_write(SHARED_MEM_TX_CMD_UPLOAD_PROGESS, usb_trans.req_percent);
        }
        usb_trans.req_percent_last = usb_trans.req_percent;
        shared_memory_scratchpad_region_save_helper(usb_trans.req_addr);
#endif        
        if (usb_trans.req_addr && (usb_trans.req_addr % usb_trans.req_bank_size) == 0) {
#if DO_TIMER

#ifdef FLT_DEBUG
            debug_P(DEBUG_USB,
                  PSTR("USB_BULK_UPLOAD_NEXT: req_bank=0x%02x addr=0x%08lx time=%.4f\n"),
                  usb_trans.req_bank, usb_trans.req_addr, timer_stop());
#else
            debug_P(DEBUG_USB,
                  PSTR("USB_BULK_UPLOAD_NEXT: req_bank=0x%02x addr=0x%08lx time=%i\n"),
                  usb_trans.req_bank, usb_trans.req_addr, timer_stop_int());
#endif
              timer_start();
#endif
            usb_trans.req_bank++;
#if DO_SHM
            shared_memory_write(SHARED_MEM_TX_CMD_BANK_CURRENT, usb_trans.req_bank);
#endif
        }
        ret_len = USB_MAX_TRANS;
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_BULK_UPLOAD_END) {
        if (usb_trans.req_state != REQ_STATUS_BULK_UPLOAD) {
            debug_P(DEBUG_USB,
                  PSTR("USB_BULK_UPLOAD_END: ERROR state is not REQ_STATUS_BULK_UPLOAD\n"));
            return 0;
        }
        debug_P(DEBUG_USB, PSTR("USB_BULK_UPLOAD_END:\n"));
        usb_trans.req_state = REQ_STATUS_IDLE;
        sram_bulk_write_end();
#if DO_SHM
        shared_memory_write(SHARED_MEM_TX_CMD_UPLOAD_END, 0);
#endif
        ret_len = 0;

        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_CRC) {
        usb_trans.req_addr = rq->wValue.word;
        usb_trans.req_addr = usb_trans.req_addr << 16;
        usb_trans.req_addr = usb_trans.req_addr | rq->wIndex.word;
        debug_P(DEBUG_USB, PSTR("USB_CRC: addr=0x%08lx \n"), usb_trans.req_addr);
        crc_check_bulk_memory(0x000000, usb_trans.req_addr, usb_trans.req_bank_size);
        ret_len = 0;
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_MODE_SNES) {
        usb_trans.req_state = REQ_STATUS_SNES;
        debug_P(DEBUG_USB, PSTR("USB_MODE_SNES:\n"));
        ret_len = 0;
        
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_MODE_AVR) {
        usb_trans.req_state = REQ_STATUS_AVR;
        debug_P(DEBUG_USB, PSTR("USB_MODE_AVR:\n"));
        ret_len = 0;
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_AVR_RESET) {
        debug_P(DEBUG_USB, PSTR("USB_AVR_RESET:\n"));
        soft_reset();
        ret_len = 0;
        /*
         * -------------------------------------------------------------------------
         */
    } else if (rq->bRequest == USB_SET_LAODER) {
        usb_trans.loader_enabled = rq->wValue.word;
        ret_len = 0;
    }
    usbMsgPtr = usb_trans.rx_buffer;
    return ret_len;             
}

/*
 * ------------------------------------------------------------------------- 
 */


void globals_init(){
    memset(&usb_trans,0,sizeof(usb_transaction_t));
    
    usb_trans.req_addr = 0;
    usb_trans.req_addr_end = 0;
    usb_trans.req_state = REQ_STATUS_IDLE;
    usb_trans.rx_remaining = 0;
    usb_trans.tx_remaining = 0;
    usb_trans.sync_errors = 0;
    usb_trans.loader_enabled = 1;
}

int main(void)
{

    uart_init();
    stdout = &uart_stdout;
    banner();
    shared_memory_init();
    system_init();
    sram_init();
    pwm_init();
    irq_init();
    
    boot_startup_rom(50);
    
    globals_init();
    pwm_stop();
    
    usbInit();
    usb_connect();
    sei();
    
    while (1) {
        avr_bus_active();
        info_P(PSTR("Activate AVR bus\n"));
        //snes_lorom();
        info_P(PSTR("Disable SNES WR\n"));
        snes_wr_disable();
        info_P(PSTR("USB poll\n"));
        while (usb_trans.req_state != REQ_STATUS_SNES) {
            usbPoll();
            shell_run();
        }
        
#if DO_SHM
        shared_memory_write(SHARED_MEM_TX_CMD_TERMINATE, 0);
#endif

#if DO_SHM_SCRATCHPAD
        shared_memory_scratchpad_region_tx_restore();
        shared_memory_scratchpad_region_rx_restore();
#endif

#if DO_CRC_CHECK     
        info_P(PSTR("-->CRC Check\n"));
        crc_check_bulk_memory(0x000000, usb_trans.req_bank_size * usb_trans.req_bank_cnt, usb_trans.req_bank_size);
#endif        
        
        info_P(PSTR("-->Switch TO SNES\n"));
        set_rom_mode();
        snes_wr_disable();
        info_P(PSTR("Disable SNES WR\n"));
        snes_bus_active();
        info_P(PSTR("Activate SNES bus\n"));
        irq_stop();
        send_reset();
        info_P(PSTR("Poll USB\n"));
        while ((usb_trans.req_state != REQ_STATUS_AVR)) {
            usbPoll();
            shell_run();
        }
        info_P(PSTR("-->Switch TO AVR\n"));
        shared_memory_init();
        irq_init();
        if(usb_trans.loader_enabled)
            boot_startup_rom(500);
        else
        {
            avr_bus_active();
            send_reset();
        }
    }
    return 0;
}
