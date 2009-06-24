#include <avr/io.h>
#include <avr/wdt.h>
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
#include "crc.h"
#include "usb_bulk.h"

extern FILE uart_stdout;

uint8_t read_buffer[TRANSFER_BUFFER_SIZE];
uint32_t req_addr = 0;
uint32_t req_size;
uint8_t req_bank;
uint32_t req_bank_size;
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
        req_bank = 0;
        rx_remaining = 0;
        req_bank_size = 1 << rq->wValue.word;
        sync_errors = 0;
#if DEBUG_USB
        printf("USB_UPLOAD_INIT: bank_size=%li\n", req_bank_size);
#endif
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
#if DEBUG_USB
            printf
                ("USB_UPLOAD_ADDR: Out of sync addr=0x%lx remain=%i packet=%i sync_error=%i\n",
                 req_addr, rx_remaining, rq->wLength.word, sync_errors);
#endif
            ret_len = 0;
        }
        rx_remaining = rq->wLength.word;
        ret_len = USB_MAX_TRANS;
        if (req_addr && req_addr % req_bank_size == 0) {
#if DEBUG_USB
            printf("USB_UPLOAD_ADDR: req_bank=0x%02x addr=0x%08lx \n",
#endif
                   req_bank, req_addr);
            req_bank++;
        }
        ret_len = USB_MAX_TRANS;
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_DOWNLOAD_INIT) {
#if DEBUG_USB
        printf("USB_DOWNLOAD_INIT\n");
#endif
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_DOWNLOAD_ADDR) {
        printf("USB_DOWNLOAD_ADDR\n");
    } else if (rq->bRequest == USB_CRC) {
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
#if DEBUG_USB
        printf("USB_CRC: addr=0x%lx \n", req_addr);
#endif

#if USB_CRC_CHECK
        cli();
        crc_check_memory(req_addr,read_buffer);
        sei();
#endif
  
/*
 * -------------------------------------------------------------------------
 */
   } else if (rq->bRequest == USB_BULK_UPLOAD_INIT) {
        req_bank = 0;
        rx_remaining = 0;
        req_bank_size = 1 << rq->wValue.word;
        sync_errors = 0;
#if DEBUG_USB
        printf("USB_BULK_UPLOAD_INIT: bank_size=%li\n", req_bank_size);
#endif
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_BULK_UPLOAD_ADDR) {

        req_state = REQ_STATUS_BULK_UPLOAD;
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
#if DEBUG_USB
        printf("USB_BULK_UPLOAD_ADDR: req_bank=0x%02x addr=0x%08lx \n",req_bank,req_addr);
#endif
        ret_len = 0;
  
/*
 * -------------------------------------------------------------------------
 */
    } else if (rq->bRequest == USB_BULK_UPLOAD_NEXT) {

        if (rx_remaining) {
            sync_errors++;
#if DEBUG_USB
            printf
                ("USB_BULK_UPLOAD_NEXT: Out of sync addr=0x%lx remain=%i packet=%i sync_error=%i\n",
                 req_addr, rx_remaining, rq->wLength.word, sync_errors);
#endif
            ret_len = 0;
        }
        rx_remaining = rq->wLength.word;
        ret_len = USB_MAX_TRANS;
        if (req_addr && req_addr % req_bank_size == 0) {
#if DEBUG_USB
            printf("USB_BULK_UPLOAD_NEXT: req_bank=0x%02x addr= 0x%08lx \n",
#endif
                   req_bank, req_addr);
            req_bank++;
        }
        ret_len = USB_MAX_TRANS;
/*
 * -------------------------------------------------------------------------
 */

    } else if (rq->bRequest == USB_CRC_ADDR) {
        req_state = REQ_STATUS_CRC;
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
#if DEBUG_USB
        printf("USB_CRC_ADDR: addr=0x%lx size=%i\n", req_addr,
               rq->wLength.word);
#endif
        req_size = rq->wLength.word;
        req_size = req_size << 2;
        tx_remaining = 2;
#if DEBUG_USB
        printf("USB_CRC_ADDR: addr=0x%lx size=%li\n", req_addr, req_size);
#endif

        cli();
        crc = crc_check_memory_range(req_addr,req_size,read_buffer);
        tx_buffer[0] = crc & 0xff;
        tx_buffer[1] = (crc >> 8) & 0xff;
        sei();
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

int main(void)
{
    uint8_t i;
    uint32_t addr;

    uart_init();
    stdout = &uart_stdout;
    
    system_init();
    printf("Sytem Init\n");

    avr_bus_active();
    
    addr = 0x000000;
    i = 0;
    while (addr++ <= 0x00ffff){
        sram_write(addr,i++);
    }

    addr = 0x000000;
    while (addr++ <= 0x00ffff){
        printf("read addr=0x%08lx %x\n",addr,sram_read(addr));
    }
    
    usbInit();
    printf("USB Init\n");
    usbDeviceDisconnect();      /* enforce re-enumeration, do this while
                                 * interrupts are disabled! */
    cli();                             
    printf("USB disconnect\n");
    i = 10;
    while (--i) {               /* fake USB disconnect for > 250 ms */
        wdt_reset();
        led_on(); 
        _delay_ms(35);
        led_off();
        _delay_ms(65);
        
    }
    led_on();
    usbDeviceConnect();
    printf("USB connect\n");
    sei();
    printf("USB poll\n");
    for (;;) {                  /* main event loop */
        usbPoll();
    }
    return 0;
}

