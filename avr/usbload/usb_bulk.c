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
#include "info.h"

#include "crc.h"
#include "usb_bulk.h"


extern usb_transaction_t usb_trans;

uint8_t usbFunctionWrite(uint8_t * data, uint8_t len)
{
    uint8_t *ptr;
    uint8_t  i;
    
    if (len > usb_trans.rx_remaining) {
        info_P(PSTR("ERROR:usbFunctionWrite more data than expected remain: %i len: %i\n"),
                usb_trans.rx_remaining, len);
        len =  usb_trans.rx_remaining;
    }
    if (usb_trans.req_state == REQ_STATUS_BULK_UPLOAD) {

        usb_trans.rx_remaining -= len;
        debug_P(DEBUG_USB_TRANS, PSTR("usbFunctionWrite REQ_STATUS_BULK_UPLOAD addr: 0x%08lx len: %i rx_remaining=%i\n"),
                usb_trans.req_addr, len,  usb_trans.rx_remaining);
        ptr = data;
        i = len;
        while(i--){
            sram_bulk_write(*ptr++);
            sram_bulk_write_next();
        }
    }
    return len;
}

uint8_t usbFunctionRead(uint8_t * data, uint8_t len)
{
    uint8_t i;
    if (len >  usb_trans.tx_remaining)
        len =  usb_trans.tx_remaining;
     usb_trans.tx_remaining -= len;
    debug_P(DEBUG_USB_TRANS, PSTR("usbFunctionRead len=%i tx_remaining=%i \n"), len, usb_trans.tx_remaining);

    for (i = 0; i < len; i++) {
        *data =  usb_trans.tx_buffer[len];
        data++;
    }
    return len;
}
