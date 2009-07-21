/*
 * =====================================================================================
 *
 *            .d8888b  88888b.   .d88b.  .d8888b  888d888 8888b.  88888b.d88b.
 *            88K      888 "88b d8P  Y8b 88K      888P"      "88b 888 "888 "88b
 *            "Y8888b. 888  888 88888888 "Y8888b. 888    .d888888 888  888  888
 *                 X88 888  888 Y8b.          X88 888    888  888 888  888  888
 *             88888P' 888  888  "Y8888   88888P' 888    "Y888888 888  888  888
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
#include <avr/interrupt.h>      /* for sei() */
#include <util/delay.h>         /* for _delay_ms() */
#include <stdlib.h>
#include <avr/pgmspace.h>       /* required by usbdrv.h */

#include "usbdrv.h"
#include "oddebug.h"            /* This is also an example for using debug
                                 * macros */
#include "config.h"
#include "requests.h"           /* The custom request numbers we use */
#include "uart.h"
#include "sram.h"
#include "debug.h"
#include "dump.h"
#include "crc.h"
#include "usb_bulk.h"
#include "timer.h"
#include "watchdog.h"

extern FILE uart_stdout;

uint8_t debug_level = ( DEBUG | DEBUG_USB | DEBUG_CRC );

uint8_t read_buffer[TRANSFER_BUFFER_SIZE];
uint32_t req_addr = 0;
uint32_t req_addr_end = 0;
uint32_t req_size;
uint8_t req_bank;
uint32_t req_bank_size;
uint16_t req_bank_cnt;
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
/*
 * -------------------------------------------------------------------------
 */
    if (rq->bRequest == USB_UPLOAD_INIT) {

        if (req_state != REQ_STATUS_IDLE){
            debug(DEBUG_USB,"USB_UPLOAD_INIT: ERROR state is not REQ_STATUS_IDLE\n");
            return 0;
        }

        req_bank = 0;
        rx_remaining = 0;
        req_bank_size = (uint32_t)1 << rq->wValue.word;
        sync_errors = 0;
        crc = 0;
        debug(DEBUG_USB,"USB_UPLOAD_INIT: bank_size=0x%08lx\n", req_bank_size);

/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_UPLOAD_ADDR) {       
                                                        
        req_state = REQ_STATUS_UPLOAD;
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
        if (rx_remaining) {
            sync_errors++;
            debug
              (DEBUG_USB,"USB_UPLOAD_ADDR: Out of sync addr=0x%lx remain=%i packet=%i sync_error=%i\n",
                 req_addr, rx_remaining, rq->wLength.word, sync_errors);
            ret_len = 0;
        }
        rx_remaining = rq->wLength.word;
        ret_len = USB_MAX_TRANS;

        
        if (req_addr && (req_addr % 0x1000) == 0) {
            debug(DEBUG_USB,"USB_UPLOAD_ADDR: bank=0x%02x addr=0x%08lx crc=%04x\n",
                req_bank, req_addr,crc_check_bulk_memory(req_addr - 0x1000,req_addr,req_bank_size));
        
        }
        if (req_addr && req_addr % req_bank_size == 0) {
            debug(DEBUG_USB,"USB_UPLOAD_ADDR: req_bank=0x%02x addr=0x%08lx\n",
                   req_bank, req_addr);

            req_bank++;
        }
        ret_len = USB_MAX_TRANS;
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_DOWNLOAD_INIT) {
        debug(DEBUG_USB,"USB_DOWNLOAD_INIT\n");

/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_DOWNLOAD_ADDR) {
        debug(DEBUG_USB,"USB_DOWNLOAD_ADDR\n");
/*
 * -------------------------------------------------------------------------
 */
   } else if (rq->bRequest == USB_BULK_UPLOAD_INIT) {

        req_bank = 0;
        rx_remaining = 0;
        debug(DEBUG_USB,"USB_BULK_UPLOAD_INIT: %i %i\n",rq->wValue.word,  rq->wIndex.word);    
        req_bank_size = (uint32_t)(1L << rq->wValue.word);
        req_bank_cnt = rq->wIndex.word;
        req_addr_end =  (uint32_t)req_bank_size * req_bank_cnt;
        
        sync_errors = 0;
        debug(DEBUG_USB,"USB_BULK_UPLOAD_INIT: bank_size=0x%08lx bank_cnt=0x%x end_addr=0x%08lx\n", 
                req_bank_size, req_bank_cnt, req_addr_end);
        
        if (req_addr == 0x000000){
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
        sram_bulk_write_start(req_addr);
        rx_remaining = rq->wLength.word;
            
        if (req_addr && req_addr % req_bank_size == 0) {
            debug(DEBUG_USB,"USB_BULK_UPLOAD_ADDR: req_bank=0x%02x addr=0x%08lx time=%.4f\n",
                   req_bank, req_addr,timer_stop());
            req_bank++;
            timer_start();
            
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
#if 0
        if (req_addr && (req_addr % 0x1000) == 0) {
            debug(DEBUG_USB,"USB_BULK_UPLOAD_NEXT: bank=0x%02x addr=0x%08lx crc=%04x\n",
                req_bank, req_addr,crc_check_bulk_memory(req_addr - 0x1000,req_addr,req_bank_size));
        
        }
        sram_bulk_write_start(req_addr);
#endif
        if (req_addr && ( req_addr % req_bank_size) == 0) {
            debug(DEBUG_USB,"USB_BULK_UPLOAD_NEXT: req_bank=0x%02x addr=0x%08lx time=%.4f\n",
                   req_bank, req_addr,timer_stop());
            req_bank++;
            timer_start();
            
        }
        ret_len = USB_MAX_TRANS;
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_BULK_UPLOAD_END) {
        if (req_state != REQ_STATUS_BULK_UPLOAD){
            debug(DEBUG_USB,"USB_BULK_UPLOAD_END: ERROR state is not REQ_STATUS_BULK_UPLOAD\n");
            return 0;
        }
        debug(DEBUG_USB,"USB_BULK_UPLOAD_END:\n");
        req_state = REQ_STATUS_IDLE;
        sram_bulk_write_end();
        ret_len = 0;
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_CRC) {
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
        debug(DEBUG_USB,"USB_CRC: addr=0x%08lx \n", req_addr);
        crc_check_bulk_memory(0x000000, req_addr, req_bank_size);
        ret_len = 0;
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_MODE_SNES) {
        req_state = REQ_STATUS_SNES;
        debug(DEBUG_USB,"USB_MODE_SNES:\n");
        ret_len = 0;
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_MODE_AVR) {
        req_state = REQ_STATUS_AVR;
        debug(DEBUG_USB,"USB_MODE_AVR:\n");
        ret_len = 0;
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_AVR_RESET) {
        debug(DEBUG_USB,"USB_AVR_RESET:\n");
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
        debug(DEBUG_USB,"USB_CRC_ADDR: addr=0x%lx size=%i\n", req_addr,
               rq->wLength.word);
        req_size = rq->wLength.word;
        req_size = req_size << 2;
        tx_remaining = 2;
        debug(DEBUG_USB,"USB_CRC_ADDR: addr=0x%lx size=%li\n", req_addr, req_size);

        crc = crc_check_memory_range(req_addr, req_size, read_buffer);
        tx_buffer[0] = crc & 0xff;
        tx_buffer[1] = (crc >> 8) & 0xff;
        ret_len = 2;
        req_state = REQ_STATUS_IDLE;
    }

    usbMsgPtr = data_buffer;
    return ret_len;             /* default for not implemented requests: return 
                                 * no data back to host */
}


/*
 * ------------------------------------------------------------------------- 
 */

void test_read_write(){
    
    uint8_t i;
    uint32_t addr;
    avr_bus_active();
    addr = 0x000000;
    i = 1;
    while (addr++ <= 0x0000ff){
        sram_write(addr,i++);
    }
    addr = 0x000000;
    while (addr++ <= 0x0000ff){
        printf("read addr=0x%08lx %x\n",addr,sram_read(addr));
    }
}



void test_bulk_read_write(){
    
    uint8_t i;
    uint32_t addr;
    avr_bus_active();
    addr = 0x000000;
    i = 0;
    sram_bulk_write_start(addr);
    while (addr++ <= 0x8000){
        sram_bulk_write(i++);
        sram_bulk_write_next();
    }
    sram_bulk_write_end();

    addr = 0x000000;
    sram_bulk_read_start(addr);
    while (addr <= 0x8000){
        printf("addr=0x%08lx %x\n",addr,sram_bulk_read());
        sram_bulk_read_next();
        addr++;
    }
    sram_bulk_read_end();
}


void test_non_zero_memory(uint32_t bottom_addr,uint32_t top_addr)
{
    uint32_t addr = 0;
    uint8_t c;
    sram_bulk_read_start(bottom_addr);
    for (addr = bottom_addr; addr < top_addr; addr++) {
        c = sram_bulk_read();
        if (c!=0xff)
            printf("addr=0x%08lx c=0x%x\n",addr,c);
        sram_bulk_read_next();
    }
    sram_bulk_read_end();
}



void test_crc(){
    printf("test_crc: clear\n");
    avr_bus_active();
    sram_bulk_set(0x000000,0x10000,0xff);
    printf("test_crc: crc\n");
    crc_check_bulk_memory(0x000000,0x10000,0x8000);
    printf("test_crc: check\n");
    test_non_zero_memory(0x000000,0x10000);
}

int main(void)
{
    uint8_t i;
    uint8_t c;
    
    uart_init();
    stdout = &uart_stdout;

    
    system_init();
    printf("Sytem start\n");

#if 0
    wdt_init();
    printf("Watchdog init\n");
#endif

#if 0
    test_read_write();
    test_bulk_read_write();
    test_crc();
    while(1);
#endif
    
    usbInit();
    printf("USB init\n");
    usbDeviceDisconnect();      /* enforce re-enumeration, do this while
                                 * interrupts are disabled! */
    cli();                             
    printf("USB disconnect\n");
    i = 10;
    while (--i) {               /* fake USB disconnect for > 250 ms */
        led_on(); 
        _delay_ms(35);
        led_off();
        _delay_ms(65);
    }
    led_on();
    
    usbDeviceConnect();
    printf("USB connect\n");

    
    while (1){
        avr_bus_active();
        printf("Activate AVR bus\n");

        printf("IRQ off\n");
        snes_irq_lo();
        snes_irq_off();

        printf("Set Snes lowrom\n");
        snes_lorom();

        printf("Disable snes WR\n");
        snes_wr_disable(); 

        sei();
        printf("USB poll\n");
        while (req_state != REQ_STATUS_SNES){
            usbPoll();
        }
        printf("USB poll done\n");
    

    #if 0    
        crc_check_bulk_memory(0x000000, req_addr_end, req_bank_size);
    #endif

        snes_reset_hi();
        snes_reset_off();

        snes_irq_lo();
        snes_irq_off();
    
        printf("IRQ off\n");
        if (req_bank_size == 0x8000){
            snes_lorom();
            printf("Set Snes lowrom \n");
        } else {
            snes_hirom();
            printf("Set Snes hirom \n");
        }
        snes_wr_disable(); 
        printf("Disable snes WR\n");
        
        snes_bus_active();
        printf("Activate Snes bus\n");

        _delay_ms(100);
        printf("Reset Snes\n");
        snes_reset_on();
        snes_reset_lo();
        _delay_ms(2);
        snes_reset_hi();
        snes_reset_off();

        printf("Poll\n");
        while (req_state != REQ_STATUS_AVR){
            usbPoll();
#if 0          
            i = 20;
            while (--i) {               /* fake USB disconnect for > 250 ms */
                _delay_ms(500);
                printf("Wait to switch to avr mode %i\n", i);
            }
            printf("Send IRQ\n");
            //snes_irq_lo();
            //snes_irq_on();
            //_delay_ms(1);

            avr_bus_active();
            
            sram_bulk_read_start(0x003000);
            c = sram_bulk_read();
            i = 5;
            while (--i) {               /* fake USB disconnect for > 250 ms */
                _delay_ms(500);
                printf("Wait to switch to snes mode %i\n", i);
            }
            
            if (req_bank_size == 0x8000){
                snes_lorom();
                printf("Set Snes lowrom \n");
            } else {
                snes_hirom();
                printf("Set Snes hirom \n");
            }
            snes_wr_disable(); 
            printf("Disable snes WR\n");

            snes_bus_active();
            printf("Activate Snes bus\n");
            
            printf("Read 0x3000=%c\n",c);
#endif
        }
    }
     
    return 0;
}

