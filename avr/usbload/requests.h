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

#endif /* __REQUESTS_H_INCLUDED__ */
