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


#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#define USB_UPLOAD_INIT         0
#define USB_UPLOAD_ADDR         1

#define USB_DOWNLOAD_INIT       2
#define USB_DOWNLOAD_ADDR       3

#define USB_CRC                 4
#define USB_CRC_ADDR            5

#define USB_BULK_UPLOAD_INIT    6
#define USB_BULK_UPLOAD_ADDR    7
#define USB_BULK_UPLOAD_NEXT    8
#define USB_BULK_UPLOAD_END     9
#define USB_MODE_SNES           10
#define USB_MODE_AVR            11
#define USB_AVR_RESET           12
#define USB_SET_LAODER          13

typedef struct usb_transaction_t {
    uint32_t req_addr;
    uint32_t req_addr_end;
    uint8_t req_bank;
    uint32_t req_bank_size;
    uint16_t req_bank_cnt;
    uint8_t req_percent;
    uint8_t req_percent_last;
    uint8_t req_state;
    uint8_t rx_remaining;
    uint8_t tx_remaining;
    uint16_t sync_errors;
    uint8_t tx_buffer[32];
    uint8_t rx_buffer[8];
    uint8_t loader_enabled;
} usb_transaction_t;

#endif                          /* __REQUESTS_H_INCLUDED__ */
