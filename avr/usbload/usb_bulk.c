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
extern uint16_t crc;

uint8_t usbFunctionWrite(uint8_t * data, uint8_t len)
{
    uint8_t *ptr;
    uint8_t  i;
    
    if (len > rx_remaining) {
        printf("ERROR:usbFunctionWrite more data than expected remain: %i len: %i\n",
               rx_remaining, len);
        len = rx_remaining;
    }
    if (req_state == REQ_STATUS_UPLOAD) {

        rx_remaining -= len;
        debug(DEBUG_USB_TRANS,"usbFunctionWrite REQ_STATUS_UPLOAD addr: 0x%08lx len: %i rx_remaining=%i\n",
               req_addr, len, rx_remaining);
        debug(DEBUG_USB_TRANS,"usbFunctionWrite %02x %02x %02x %02x %02x %02x %02x %x\n",
        data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);         
        sram_copy(req_addr, data, len);
        req_addr += len;
    } else if (req_state == REQ_STATUS_BULK_UPLOAD) {

        rx_remaining -= len;
        debug(DEBUG_USB_TRANS,"usbFunctionWrite REQ_STATUS_BULK_UPLOAD addr: 0x%08lx len: %i rx_remaining=%i\n",
               req_addr, len, rx_remaining);
        ptr = data;
        i = len;
        while(i--){
            sram_bulk_write(*ptr++);
            sram_bulk_write_next();
        }
    }
    /* test this */
    //return rx_remaining == 0
    return len;
}

uint8_t usbFunctionRead(uint8_t * data, uint8_t len)
{
    uint8_t i;
    if (len > tx_remaining)
        len = tx_remaining;
    tx_remaining -= len;
    debug(DEBUG_USB_TRANS,"usbFunctionRead len=%i tx_remaining=%i \n", len, tx_remaining);

    for (i = 0; i < len; i++) {
        *data = tx_buffer[len];
        data++;
    }
    return len;
}
