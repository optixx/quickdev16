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

#ifndef __CONFIH_H__
#define __CONFIH_H__


#define DEBUG                       1
#define DEBUG_USB                   2
#define DEBUG_USB_TRANS             4
#define DEBUG_SRAM                  8
#define DEBUG_SRAM_RAW              16
#define DEBUG_SREG                  32
#define DEBUG_CRC                   64
#define DEBUG_SHM                   128

#define REQ_STATUS_IDLE             0x01
#define REQ_STATUS_UPLOAD           0x02
#define REQ_STATUS_BULK_UPLOAD      0x03
#define REQ_STATUS_BULK_NEXT        0x04
#define REQ_STATUS_CRC              0x05
#define REQ_STATUS_SNES             0x06
#define REQ_STATUS_AVR              0x07

#define USB_MAX_TRANS               0xff
#define USB_CRC_CHECK               0x01

#define TRANSFER_BUFFER_SIZE        0x000
#define FORMAT_BUFFER_LEN           0x080
#define RECEIVE_BUF_LEN             0x030
#define HW_VERSION                  "2.6"
#define SW_VERSION                  "1.0"

#define DO_CRC_CHECK_LOADER         0
#define DO_CRC_CHECK                0
#define DO_SHM_SCRATCHPAD           0
#define DO_SHM                      0
#define DO_TIMER                    0

#endif 
