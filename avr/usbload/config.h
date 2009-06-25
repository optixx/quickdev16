
#ifndef __config_h__
#define __config_h__

#define DEBUG                       1
#define DEBUG_USB                   2
#define DEBUG_USB_RAW               4
#define DEBUG_SRAM                  8
#define DEBUG_SRAM_RAW              16
#define DEBUG_SREG                  32

#define REQ_STATUS_IDLE             0x01
#define REQ_STATUS_UPLOAD           0x02
#define REQ_STATUS_BULK_UPLOAD      0x03
#define REQ_STATUS_BULK_NEXT        0x04
#define REQ_STATUS_CRC              0x05
#define REQ_STATUS_BOOT             0x06

#define USB_MAX_TRANS               0xff
#define USB_CRC_CHECK               0x01

#define TRANSFER_BUFFER_SIZE        0x200


#endif 
