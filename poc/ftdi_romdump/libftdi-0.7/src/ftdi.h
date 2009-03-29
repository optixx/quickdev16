/***************************************************************************
                          ftdi.h  -  description
                             -------------------
    begin                : Fri Apr 4 2003
    copyright            : (C) 2003 by Intra2net AG
    email                : opensource@intra2net.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation;             *
 *                                                                         *
 ***************************************************************************/

#ifndef __libftdi_h__
#define __libftdi_h__

#include <usb.h>

enum ftdi_chip_type { TYPE_AM=0, TYPE_BM=1, TYPE_2232C=2 };
enum ftdi_parity_type { NONE=0, ODD=1, EVEN=2, MARK=3, SPACE=4 };
enum ftdi_stopbits_type { STOP_BIT_1=0, STOP_BIT_15=1, STOP_BIT_2=2 };
enum ftdi_bits_type { BITS_7=7, BITS_8=8 };

enum ftdi_mpsse_mode {
    BITMODE_RESET  = 0x00,
    BITMODE_BITBANG= 0x01,
    BITMODE_MPSSE  = 0x02,
    BITMODE_SYNCBB = 0x04,
    BITMODE_MCU    = 0x08,
    BITMODE_OPTO   = 0x10
};

/* Port interface code for FT2232C */
enum ftdi_interface {
    INTERFACE_ANY = 0,
    INTERFACE_A   = 1,
    INTERFACE_B   = 2
};

/* Shifting commands IN MPSSE Mode*/
#define MPSSE_WRITE_NEG 0x01   /* Write TDI/DO on negative TCK/SK edge*/
#define MPSSE_BITMODE   0x02   /* Write bits, not bytes */
#define MPSSE_READ_NEG  0x04   /* Sample TDO/DI on negative TCK/SK edge */
#define MPSSE_LSB       0x08   /* LSB first */
#define MPSSE_DO_WRITE  0x10   /* Write TDI/DO */
#define MPSSE_DO_READ   0x20   /* Read TDO/DI */
#define MPSSE_WRITE_TMS 0x40   /* Write TMS/CS */

/* FTDI MPSSE commands */
#define SET_BITS_LOW   0x80
/*BYTE DATA*/
/*BYTE Direction*/
#define SET_BITS_HIGH  0x82
/*BYTE DATA*/
/*BYTE Direction*/
#define GET_BITS_LOW   0x81
#define GET_BITS_HIGH  0x83
#define LOOPBACK_START 0x84
#define LOOPBACK_END   0x85
#define TCK_DIVISOR    0x86
/* Value Low */
/* Value HIGH */ /*rate is 12000000/((1+value)*2) */
#define DIV_VALUE(rate) (rate > 6000000)?0:((6000000/rate -1) > 0xffff)? 0xffff: (6000000/rate -1)

/* Commands in MPSSE and Host Emulation Mode */
#define SEND_IMMEDIATE 0x87 
#define WAIT_ON_HIGH   0x88
#define WAIT_ON_LOW    0x89

/* Commands in Host Emulation Mode */
#define READ_SHORT     0x90
/* Address_Low */
#define READ_EXTENDED  0x91
/* Address High */
/* Address Low  */
#define WRITE_SHORT    0x92
/* Address_Low */
#define WRITE_EXTENDED 0x93
/* Address High */
/* Address Low  */

struct ftdi_context {
    // USB specific
    struct usb_dev_handle *usb_dev;
    int usb_read_timeout;
    int usb_write_timeout;

    // FTDI specific
    enum ftdi_chip_type type;
    int baudrate;
    unsigned char bitbang_enabled;
    unsigned char *readbuffer;
    unsigned int readbuffer_offset;
    unsigned int readbuffer_remaining;
    unsigned int readbuffer_chunksize;
    unsigned int writebuffer_chunksize;

    // FTDI FT2232C requirecments
    int interface;   // 0 or 1
    int index;       // 1 or 2
    // Endpoints
    int in_ep;
    int out_ep;      // 1 or 2

    /* 1: (default) Normal bitbang mode, 2: FT2232C SPI bitbang mode */
    unsigned char bitbang_mode;

    // misc
    char *error_str;
};

struct ftdi_device_list {
    struct ftdi_device_list *next;
    struct usb_device *dev;
};

struct ftdi_eeprom {
    int vendor_id;
    int product_id;

    int self_powered;
    int remote_wakeup;
    int BM_type_chip;

    int in_is_isochronous;
    int out_is_isochronous;
    int suspend_pull_downs;

    int use_serial;
    int change_usb_version;
    int usb_version;
    int max_power;

    char *manufacturer;
    char *product;
    char *serial;
};

#ifdef __cplusplus
extern "C" {
#endif

    int ftdi_init(struct ftdi_context *ftdi);
    int ftdi_set_interface(struct ftdi_context *ftdi, enum ftdi_interface interface);

    void ftdi_deinit(struct ftdi_context *ftdi);
    void ftdi_set_usbdev (struct ftdi_context *ftdi, usb_dev_handle *usbdev);
    
    int ftdi_usb_find_all(struct ftdi_context *ftdi, struct ftdi_device_list **devlist,
                          int vendor, int product);
    void ftdi_list_free(struct ftdi_device_list **devlist);
    
    int ftdi_usb_open(struct ftdi_context *ftdi, int vendor, int product);
    int ftdi_usb_open_desc(struct ftdi_context *ftdi, int vendor, int product,
                           const char* description, const char* serial);
    int ftdi_usb_open_dev(struct ftdi_context *ftdi, struct usb_device *dev);
    
    int ftdi_usb_close(struct ftdi_context *ftdi);
    int ftdi_usb_reset(struct ftdi_context *ftdi);
    int ftdi_usb_purge_buffers(struct ftdi_context *ftdi);

    int ftdi_set_baudrate(struct ftdi_context *ftdi, int baudrate);
    int ftdi_set_line_property(struct ftdi_context *ftdi, enum ftdi_bits_type bits,
                               enum ftdi_stopbits_type sbit, enum ftdi_parity_type parity);

    int ftdi_read_data(struct ftdi_context *ftdi, unsigned char *buf, int size);
    int ftdi_read_data_set_chunksize(struct ftdi_context *ftdi, unsigned int chunksize);
    int ftdi_read_data_get_chunksize(struct ftdi_context *ftdi, unsigned int *chunksize);

    int ftdi_write_data(struct ftdi_context *ftdi, unsigned char *buf, int size);
    int ftdi_write_data_set_chunksize(struct ftdi_context *ftdi, unsigned int chunksize);
    int ftdi_write_data_get_chunksize(struct ftdi_context *ftdi, unsigned int *chunksize);

    int ftdi_enable_bitbang(struct ftdi_context *ftdi, unsigned char bitmask);
    int ftdi_disable_bitbang(struct ftdi_context *ftdi);
    int ftdi_set_bitmode(struct ftdi_context *ftdi, unsigned char bitmask, unsigned char mode);
    int ftdi_read_pins(struct ftdi_context *ftdi, unsigned char *pins);

    int ftdi_set_latency_timer(struct ftdi_context *ftdi, unsigned char latency);
    int ftdi_get_latency_timer(struct ftdi_context *ftdi, unsigned char *latency);

    // init and build eeprom from ftdi_eeprom structure
    void ftdi_eeprom_initdefaults(struct ftdi_eeprom *eeprom);
    int  ftdi_eeprom_build(struct ftdi_eeprom *eeprom, unsigned char *output);

    // "eeprom" needs to be valid 128 byte eeprom (generated by the eeprom generator)
    // the checksum of the eeprom is valided
    int ftdi_read_eeprom(struct ftdi_context *ftdi, unsigned char *eeprom);
    int ftdi_write_eeprom(struct ftdi_context *ftdi, unsigned char *eeprom);
    int ftdi_erase_eeprom(struct ftdi_context *ftdi);

    char *ftdi_get_error_string(struct ftdi_context *ftdi);

#ifdef __cplusplus
}
#endif

#endif /* __libftdi_h__ */
