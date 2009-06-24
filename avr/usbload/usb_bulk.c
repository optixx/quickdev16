
#include <avr/io.h>
#include <avr/pgmspace.h>       /* required by usbdrv.h */
#include <avr/interrupt.h>      /* for sei() */
#include <util/delay.h>         /* for _delay_ms() */

#include <stdlib.h>

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

extern uint8_t read_buffer[TRANSFER_BUFFER_SIZE];
extern uint32_t req_addr;
extern uint32_t req_size;
extern uint8_t req_bank;
extern uint32_t req_bank_size;
extern uint8_t req_state;
extern uint8_t rx_remaining; 
extern uint8_t tx_remaining;
extern  uint8_t tx_buffer[32];

uint8_t usbFunctionWrite(uint8_t * data, uint8_t len)
{
    uint8_t *ptr;
    uint8_t  i;
    if (len > rx_remaining) {
        printf("usbFunctionWrite more data than expected remain: %i len: %i\n",
               rx_remaining, len);
        len = rx_remaining;
    }
    if (req_state == REQ_STATUS_UPLOAD) {

        rx_remaining -= len;
#if DEBUG_USB_RAW
        printf("usbFunctionWrite REQ_STATUS_UPLOAD addr: 0x%08lx len: %i rx_remaining=%i\n",
               req_addr, len, rx_remaining);
#endif
        //cli();
        sram_copy(req_addr, data, len);
        //sei();
        req_addr += len;
    } else if (req_state == REQ_STATUS_BULK_UPLOAD) {

        rx_remaining -= len;
#if DEBUG_USB_RAW
        printf("usbFunctionWrite REQ_STATUS_BULK_UPLOAD addr: 0x%08lx len: %i rx_remaining=%i\n",
               req_addr, len, rx_remaining);
#endif
        ptr = data;
        i = len;
        cli();
        while(i--){
            sram_bulk_write(*ptr++);
            counter_up();
        }
        sei();
    }
    return len;
}

uint8_t usbFunctionRead(uint8_t * data, uint8_t len)
{
    uint8_t i;
    if (len > tx_remaining)
        len = tx_remaining;
    tx_remaining -= len;
#if DEBUG_USB_RAW
    printf("usbFunctionRead len=%i tx_remaining=%i \n", len, tx_remaining);
#endif
    for (i = 0; i < len; i++) {
        *data = tx_buffer[len];
        data++;
    }
    return len;
}