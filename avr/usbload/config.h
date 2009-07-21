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




#ifndef __CONFIH_H__
#define __CONFIH_H__

#define DEBUG                       1
#define DEBUG_USB                   2
#define DEBUG_USB_TRANS             4
#define DEBUG_SRAM                  8
#define DEBUG_SRAM_RAW              16
#define DEBUG_SREG                  32
#define DEBUG_CRC                   64

#define REQ_STATUS_IDLE             0x01
#define REQ_STATUS_UPLOAD           0x02
#define REQ_STATUS_BULK_UPLOAD      0x03
#define REQ_STATUS_BULK_NEXT        0x04
#define REQ_STATUS_CRC              0x05
#define REQ_STATUS_SNES             0x06
#define REQ_STATUS_AVR              0x07

#define USB_MAX_TRANS               0xff
#define USB_CRC_CHECK               0x01

#define TRANSFER_BUFFER_SIZE        0x200


#endif 
