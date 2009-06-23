#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>      /* for sei() */
#include <util/delay.h>         /* for _delay_ms() */
#include <stdlib.h>
#include <avr/pgmspace.h>       /* required by usbdrv.h */

#include "usbdrv.h"
#include "oddebug.h"            /* This is also an example for using debug
                                 * macros */
#include "requests.h"           /* The custom request numbers we use */
#include "uart.h"
#include "sram.h"
#include "debug.h"
#include "crc.h"


#define REQ_IDLE      0
#define REQ_UPLOAD    1
#define RES_CRC       2
#define BUFFER_SIZE 256

extern FILE uart_stdout;

uint8_t read_buffer[BUFFER_SIZE];
uint32_t req_addr = 0;
uint32_t req_size;
uint8_t req_bank;
uint32_t req_bank_size;
uint8_t req_state = REQ_IDLE;
uint8_t rx_remaining = 0;
uint8_t tx_remaining = 0;
uint16_t sync_errors = 0;
uint8_t tx_buffer[32];
uint8_t data_buffer[4];
uint32_t addr;

void crc_check_memory(uint32_t top_addr)
{
    uint16_t crc = 0;
    uint32_t addr;
    req_bank = 0;
    for (addr = 0x000000; addr < top_addr; addr += BUFFER_SIZE) {
        sram_read_buffer(addr, read_buffer, BUFFER_SIZE);
        crc = do_crc_update(crc, read_buffer, BUFFER_SIZE);
        if (addr && addr % 32768 == 0) {
            printf("crc_check_memory: req_bank: 0x%x Addr: 0x%lx CRC: %x\n",
                   req_bank, addr, crc);
            req_bank++;
            crc = 0;
        }
    }
}


void crc_check_memory_range(uint32_t start_addr, uint32_t size)
{
    uint16_t crc = 0;
    uint32_t addr;
    req_bank = 0;
    for (addr = start_addr; addr < start_addr + size; addr += BUFFER_SIZE) {
        sram_read_buffer(addr, read_buffer, BUFFER_SIZE);
        crc = do_crc_update(crc, read_buffer, BUFFER_SIZE);
    }
    tx_buffer[0] = crc & 0xff;
    tx_buffer[1] = (crc >> 8) & 0xff;
    printf("crc_check_memory_range: Addr: 0x%lx CRC: %x\n", addr, crc);
}


usbMsgLen_t usbFunctionSetup(uchar data[8])
{

    usbRequest_t *rq = (void *) data;
    uint8_t ret_len = 0;
    if (rq->bRequest == USB_UPLOAD_INIT) {
        req_bank = 0;
        rx_remaining = 0;
        req_bank_size = 1 << rq->wValue.word;
        sync_errors = 0;
        printf("USB_UPLOAD_INIT: bank size %li\n", req_bank_size);
    } else if (rq->bRequest == USB_UPLOAD_ADDR) {       /* echo -- used for
                                                         * reliability tests */
        req_state = REQ_UPLOAD;
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
        if (rx_remaining) {
            sync_errors++;
            printf
                ("USB_UPLOAD_ADDR: Out of sync Addr=0x%lx remain=%i packet=%i sync_error=%i\n",
                 req_addr, rx_remaining, rq->wLength.word, sync_errors);
            ret_len = 0;
        }
        rx_remaining = rq->wLength.word;
        ret_len = 0xff;
        if (req_addr && req_addr % req_bank_size == 0) {
            printf("USB_UPLOAD_ADDR: req_bank: 0x%x Addr: 0x%08lx \n",
                   req_bank, req_addr);
            req_bank++;
        }
        ret_len = 0xff;
    } else if (rq->bRequest == USB_DOWNLOAD_INIT) {
        printf("USB_DOWNLOAD_INIT\n");
    } else if (rq->bRequest == USB_DOWNLOAD_ADDR) {
        printf("USB_DOWNLOAD_ADDR\n");
    } else if (rq->bRequest == USB_CRC) {
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
        printf("USB_CRC: Addr 0x%lx \n", req_addr);
        cli();
        crc_check_memory(req_addr);
        sei();
    } else if (rq->bRequest == USB_CRC_ADDR) {
        req_addr = rq->wValue.word;
        req_addr = req_addr << 16;
        req_addr = req_addr | rq->wIndex.word;
        printf("USB_CRC_ADDR: Addr: 0x%lx Size: %i\n", req_addr,
               rq->wLength.word);
        req_size = rq->wLength.word;
        req_size = req_size << 2;
        tx_remaining = 2;
        printf("USB_CRC_ADDR: Addr: 0x%lx Size: %li\n", req_addr, req_size);
        cli();
        // crc_check_memory_range(req_addr,req_size);
        sei();
        ret_len = 2;
    }

    usbMsgPtr = data_buffer;
    return ret_len;             /* default for not implemented requests: return 
                                 * no data back to host */
}


uint8_t usbFunctionWrite(uint8_t * data, uint8_t len)
{
    if (len > rx_remaining) {
        printf("usbFunctionWrite more data than expected remain: %i len: %i\n",
               rx_remaining, len);
        len = rx_remaining;
    }
    if (req_state == REQ_UPLOAD) {

        rx_remaining -= len;
#if 1
        printf("usbFunctionWrite addr: 0x%08lx len: %i rx_remaining=%i\n",
               req_addr, len, rx_remaining);
#endif
        cli();
        sram_copy(req_addr, data, len);
        sei();
        req_addr += len;
    }
    return len;
}

uint8_t usbFunctionRead(uint8_t * data, uint8_t len)
{
    uint8_t i;
    if (len > tx_remaining)
        len = tx_remaining;
    tx_remaining -= len;
#if 1
    printf("usbFunctionRead len=%i tx_remaining=%i \n", len, tx_remaining);
#endif
    for (i = 0; i < len; i++) {
        *data = tx_buffer[len];
        data++;
    }
    return len;
}

/*
 * ------------------------------------------------------------------------- 
 */

int main(void)
{
    uint8_t i;
    wdt_enable(WDTO_1S);
    uart_init();
    stdout = &uart_stdout;
    sram_init();
    printf("SRAM Init\n");
    spi_init();
    printf("SPI Init\n");
    usbInit();
    printf("USB Init\n");
    usbDeviceDisconnect();      /* enforce re-enumeration, do this while
                                 * interrupts are disabled! */
    cli();                             
    printf("USB disconnect\n");
    i = 10;
    while (--i) {               /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(10);
    }
    usbDeviceConnect();
    printf("USB connect\n");
    sei();
    printf("USB poll\n");
    for (;;) {                  /* main event loop */
        wdt_reset();
        usbPoll();
    }
    return 0;
}

/*
 * ------------------------------------------------------------------------- 
 */
