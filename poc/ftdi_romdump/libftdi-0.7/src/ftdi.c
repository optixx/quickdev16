/***************************************************************************
                          ftdi.c  -  description
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

#include <usb.h>
#include <string.h>

#include "ftdi.h"

#define ftdi_error_return(code, str) do {  \
        ftdi->error_str = str;             \
        return code;                       \
   } while(0);                 


/* ftdi_init

  Initializes a ftdi_context.

  Return codes:
   0: All fine
  -1: Couldn't allocate read buffer
*/
int ftdi_init(struct ftdi_context *ftdi)
{
    ftdi->usb_dev = NULL;
    ftdi->usb_read_timeout = 5000;
    ftdi->usb_write_timeout = 5000;

    ftdi->type = TYPE_BM;    /* chip type */
    ftdi->baudrate = -1;
    ftdi->bitbang_enabled = 0;

    ftdi->readbuffer = NULL;
    ftdi->readbuffer_offset = 0;
    ftdi->readbuffer_remaining = 0;
    ftdi->writebuffer_chunksize = 4096;

    ftdi->interface = 0;
    ftdi->index = 0;
    ftdi->in_ep = 0x02;
    ftdi->out_ep = 0x81;
    ftdi->bitbang_mode = 1; /* 1: Normal bitbang mode, 2: SPI bitbang mode */

    ftdi->error_str = NULL;

    /* All fine. Now allocate the readbuffer */
    return ftdi_read_data_set_chunksize(ftdi, 4096);
}

/* ftdi_set_interface
   
   Call after ftdi_init
   
   Open selected channels on a chip, otherwise use first channel
    0: all fine
   -1: unknown interface
*/
int ftdi_set_interface(struct ftdi_context *ftdi, enum ftdi_interface interface)
{
    switch (interface) {
    case INTERFACE_ANY:
    case INTERFACE_A:
        /* ftdi_usb_open_desc cares to set the right index, depending on the found chip */
        break;
    case INTERFACE_B:
        ftdi->interface = 1;
        ftdi->index     = INTERFACE_B;
        ftdi->in_ep     = 0x04;
        ftdi->out_ep    = 0x83;
        break;
    default:
        ftdi_error_return(-1, "Unknown interface");
    }
    return 0;
}

/* ftdi_deinit

   Deinitializes a ftdi_context.
*/
void ftdi_deinit(struct ftdi_context *ftdi)
{
    if (ftdi->readbuffer != NULL) {
        free(ftdi->readbuffer);
        ftdi->readbuffer = NULL;
    }
}

/* ftdi_set_usbdev
 
   Use an already open device.
*/
void ftdi_set_usbdev (struct ftdi_context *ftdi, usb_dev_handle *usb)
{
    ftdi->usb_dev = usb;
}


/* ftdi_usb_find_all
 
   Finds all ftdi devices on the usb bus. Creates a new ftdi_device_list which
   needs to be deallocated by ftdi_list_free after use.

   Return codes:
    >0: number of devices found
    -1: usb_find_busses() failed
    -2: usb_find_devices() failed
    -3: out of memory
*/
int ftdi_usb_find_all(struct ftdi_context *ftdi, struct ftdi_device_list **devlist, int vendor, int product) 
{
    struct ftdi_device_list **curdev;
    struct usb_bus *bus;
    struct usb_device *dev;
    int count = 0;
    
    usb_init();
    if (usb_find_busses() < 0)
        ftdi_error_return(-1, "usb_find_busses() failed");
    if (usb_find_devices() < 0)
        ftdi_error_return(-2, "usb_find_devices() failed");

    curdev = devlist;
    for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == vendor
                    && dev->descriptor.idProduct == product)
            {
                *curdev = (struct ftdi_device_list*)malloc(sizeof(struct ftdi_device_list));
                if (!*curdev)
                    ftdi_error_return(-3, "out of memory");
                
                (*curdev)->next = NULL;
                (*curdev)->dev = dev;

                curdev = &(*curdev)->next;
                count++;
            }
        }
    }
    
    return count;
}

/* ftdi_list_free

   Frees a created device list.
*/
void ftdi_list_free(struct ftdi_device_list **devlist) 
{
    struct ftdi_device_list **curdev;
    for (; *devlist == NULL; devlist = curdev) {
        curdev = &(*devlist)->next;
        free(*devlist);
    }

    devlist = NULL;
}

/* ftdi_usb_open_dev 

   Opens a ftdi device given by a usb_device.
   
   Return codes:
     0: all fine
    -4: unable to open device
    -5: unable to claim device
    -6: reset failed
    -7: set baudrate failed
*/
int ftdi_usb_open_dev(struct ftdi_context *ftdi, struct usb_device *dev)
{
    if (!(ftdi->usb_dev = usb_open(dev)))
        ftdi_error_return(-4, "usb_open() failed");
    
    if (usb_claim_interface(ftdi->usb_dev, ftdi->interface) != 0) {
        usb_close (ftdi->usb_dev);
        ftdi_error_return(-5, "unable to claim usb device. Make sure ftdi_sio is unloaded!");
    }

    if (ftdi_usb_reset (ftdi) != 0) {
        usb_close (ftdi->usb_dev);
        ftdi_error_return(-6, "ftdi_usb_reset failed");
    }

    if (ftdi_set_baudrate (ftdi, 9600) != 0) {
        usb_close (ftdi->usb_dev);
        ftdi_error_return(-7, "set baudrate failed");
    }

    // Try to guess chip type
    // Bug in the BM type chips: bcdDevice is 0x200 for serial == 0
    if (dev->descriptor.bcdDevice == 0x400 || (dev->descriptor.bcdDevice == 0x200
            && dev->descriptor.iSerialNumber == 0))
        ftdi->type = TYPE_BM;
    else if (dev->descriptor.bcdDevice == 0x200)
        ftdi->type = TYPE_AM;
    else if (dev->descriptor.bcdDevice == 0x500) {
        ftdi->type = TYPE_2232C;
        if (!ftdi->index)
            ftdi->index = INTERFACE_A;
    }

    ftdi_error_return(0, "all fine");
}

/* ftdi_usb_open
   
   Opens the first device with a given vendor and product ids.
   
   Return codes:
   See ftdi_usb_open_desc()
*/  
int ftdi_usb_open(struct ftdi_context *ftdi, int vendor, int product)
{
    return ftdi_usb_open_desc(ftdi, vendor, product, NULL, NULL);
}

/* ftdi_usb_open_desc

   Opens the first device with a given, vendor id, product id,
   description and serial.
   
   Return codes:
     0: all fine
    -1: usb_find_busses() failed
    -2: usb_find_devices() failed
    -3: usb device not found
    -4: unable to open device
    -5: unable to claim device
    -6: reset failed
    -7: set baudrate failed
    -8: get product description failed
    -9: get serial number failed
    -10: unable to close device
*/
int ftdi_usb_open_desc(struct ftdi_context *ftdi, int vendor, int product,
                       const char* description, const char* serial)
{
    struct usb_bus *bus;
    struct usb_device *dev;
    char string[256];

    usb_init();

    if (usb_find_busses() < 0)
        ftdi_error_return(-1, "usb_find_busses() failed");
    if (usb_find_devices() < 0)
        ftdi_error_return(-2, "usb_find_devices() failed");

    for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == vendor
                    && dev->descriptor.idProduct == product) {
                if (!(ftdi->usb_dev = usb_open(dev)))
                    ftdi_error_return(-4, "usb_open() failed");

                if (description != NULL) {
                    if (usb_get_string_simple(ftdi->usb_dev, dev->descriptor.iProduct, string, sizeof(string)) <= 0) {
                        usb_close (ftdi->usb_dev);
                        ftdi_error_return(-8, "unable to fetch product description");
                    }
                    if (strncmp(string, description, sizeof(string)) != 0) {
                        if (usb_close (ftdi->usb_dev) != 0)
                            ftdi_error_return(-10, "unable to close device");
                        continue;
                    }
                }
                if (serial != NULL) {
                    if (usb_get_string_simple(ftdi->usb_dev, dev->descriptor.iSerialNumber, string, sizeof(string)) <= 0) {
                        usb_close (ftdi->usb_dev);
                        ftdi_error_return(-9, "unable to fetch serial number");
                    }
                    if (strncmp(string, serial, sizeof(string)) != 0) {
                        if (usb_close (ftdi->usb_dev) != 0)
                            ftdi_error_return(-10, "unable to close device");
                        continue;
                    }
                }

                if (usb_close (ftdi->usb_dev) != 0)
                    ftdi_error_return(-10, "unable to close device");
                
                return ftdi_usb_open_dev(ftdi, dev);
            }
        }
    }

    // device not found
    ftdi_error_return(-3, "device not found");
}

/* ftdi_usb_reset

   Resets the ftdi device.
   
   Return codes:
     0: all fine
    -1: FTDI reset failed
*/
int ftdi_usb_reset(struct ftdi_context *ftdi)
{
    if (usb_control_msg(ftdi->usb_dev, 0x40, 0, 0, ftdi->index, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return(-1,"FTDI reset failed");

    // Invalidate data in the readbuffer
    ftdi->readbuffer_offset = 0;
    ftdi->readbuffer_remaining = 0;

    return 0;
}

/* ftdi_usb_purge_buffers

   Cleans the buffers of the ftdi device.
   
   Return codes:
     0: all fine
    -1: write buffer purge failed
    -2: read buffer purge failed
*/
int ftdi_usb_purge_buffers(struct ftdi_context *ftdi)
{
    if (usb_control_msg(ftdi->usb_dev, 0x40, 0, 1, ftdi->index, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return(-1, "FTDI purge of RX buffer failed");

    // Invalidate data in the readbuffer
    ftdi->readbuffer_offset = 0;
    ftdi->readbuffer_remaining = 0;

    if (usb_control_msg(ftdi->usb_dev, 0x40, 0, 2, ftdi->index, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return(-2, "FTDI purge of TX buffer failed");

    return 0;
}

/* ftdi_usb_close
   
   Closes the ftdi device.
   
   Return codes:
     0: all fine
    -1: usb_release failed
    -2: usb_close failed
*/
int ftdi_usb_close(struct ftdi_context *ftdi)
{
    int rtn = 0;

    if (usb_release_interface(ftdi->usb_dev, ftdi->interface) != 0)
        rtn = -1;

    if (usb_close (ftdi->usb_dev) != 0)
        rtn = -2;

    return rtn;
}


/*
    ftdi_convert_baudrate returns nearest supported baud rate to that requested.
    Function is only used internally
*/
static int ftdi_convert_baudrate(int baudrate, struct ftdi_context *ftdi,
                                 unsigned short *value, unsigned short *index)
{
    static const char am_adjust_up[8] = {0, 0, 0, 1, 0, 3, 2, 1};
    static const char am_adjust_dn[8] = {0, 0, 0, 1, 0, 1, 2, 3};
    static const char frac_code[8] = {0, 3, 2, 4, 1, 5, 6, 7};
    int divisor, best_divisor, best_baud, best_baud_diff;
    unsigned long encoded_divisor;
    int i;

    if (baudrate <= 0) {
        // Return error
        return -1;
    }

    divisor = 24000000 / baudrate;

    if (ftdi->type == TYPE_AM) {
        // Round down to supported fraction (AM only)
        divisor -= am_adjust_dn[divisor & 7];
    }

    // Try this divisor and the one above it (because division rounds down)
    best_divisor = 0;
    best_baud = 0;
    best_baud_diff = 0;
    for (i = 0; i < 2; i++) {
        int try_divisor = divisor + i;
        int baud_estimate;
        int baud_diff;

        // Round up to supported divisor value
        if (try_divisor <= 8) {
            // Round up to minimum supported divisor
            try_divisor = 8;
        } else if (ftdi->type != TYPE_AM && try_divisor < 12) {
            // BM doesn't support divisors 9 through 11 inclusive
            try_divisor = 12;
        } else if (divisor < 16) {
            // AM doesn't support divisors 9 through 15 inclusive
            try_divisor = 16;
        } else {
            if (ftdi->type == TYPE_AM) {
                // Round up to supported fraction (AM only)
                try_divisor += am_adjust_up[try_divisor & 7];
                if (try_divisor > 0x1FFF8) {
                    // Round down to maximum supported divisor value (for AM)
                    try_divisor = 0x1FFF8;
                }
            } else {
                if (try_divisor > 0x1FFFF) {
                    // Round down to maximum supported divisor value (for BM)
                    try_divisor = 0x1FFFF;
                }
            }
        }
        // Get estimated baud rate (to nearest integer)
        baud_estimate = (24000000 + (try_divisor / 2)) / try_divisor;
        // Get absolute difference from requested baud rate
        if (baud_estimate < baudrate) {
            baud_diff = baudrate - baud_estimate;
        } else {
            baud_diff = baud_estimate - baudrate;
        }
        if (i == 0 || baud_diff < best_baud_diff) {
            // Closest to requested baud rate so far
            best_divisor = try_divisor;
            best_baud = baud_estimate;
            best_baud_diff = baud_diff;
            if (baud_diff == 0) {
                // Spot on! No point trying
                break;
            }
        }
    }
    // Encode the best divisor value
    encoded_divisor = (best_divisor >> 3) | (frac_code[best_divisor & 7] << 14);
    // Deal with special cases for encoded value
    if (encoded_divisor == 1) {
        encoded_divisor = 0;    // 3000000 baud
    } else if (encoded_divisor == 0x4001) {
        encoded_divisor = 1;    // 2000000 baud (BM only)
    }
    // Split into "value" and "index" values
    *value = (unsigned short)(encoded_divisor & 0xFFFF);
    if(ftdi->type == TYPE_2232C) {
        *index = (unsigned short)(encoded_divisor >> 8);
        *index &= 0xFF00;
        *index |= ftdi->index;
    }
    else
        *index = (unsigned short)(encoded_divisor >> 16);

    // Return the nearest baud rate
    return best_baud;
}

/*
    ftdi_set_baudrate
    
    Sets the chip baudrate
    
    Return codes:
     0: all fine
    -1: invalid baudrate
    -2: setting baudrate failed
*/
int ftdi_set_baudrate(struct ftdi_context *ftdi, int baudrate)
{
    unsigned short value, index;
    int actual_baudrate;

    if (ftdi->bitbang_enabled) {
        baudrate = baudrate*4;
    }

    actual_baudrate = ftdi_convert_baudrate(baudrate, ftdi, &value, &index);
    if (actual_baudrate <= 0)
        ftdi_error_return (-1, "Silly baudrate <= 0.");

    // Check within tolerance (about 5%)
    if ((actual_baudrate * 2 < baudrate /* Catch overflows */ )
            || ((actual_baudrate < baudrate)
                ? (actual_baudrate * 21 < baudrate * 20)
                : (baudrate * 21 < actual_baudrate * 20)))
        ftdi_error_return (-1, "Unsupported baudrate. Note: bitbang baudrates are automatically multiplied by 4");

    if (usb_control_msg(ftdi->usb_dev, 0x40, 3, value, index, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return (-2, "Setting new baudrate failed");

    ftdi->baudrate = baudrate;
    return 0;
}

/*
    ftdi_set_line_property

    set (RS232) line characteristics by Alain Abbas
    
    Return codes:
     0: all fine
    -1: Setting line property failed
*/
int ftdi_set_line_property(struct ftdi_context *ftdi, enum ftdi_bits_type bits,
                    enum ftdi_stopbits_type sbit, enum ftdi_parity_type parity)
{
    unsigned short value = bits;

    switch(parity) {
    case NONE:
        value |= (0x00 << 8);
        break;
    case ODD:
        value |= (0x01 << 8);
        break;
    case EVEN:
        value |= (0x02 << 8);
        break;
    case MARK:
        value |= (0x03 << 8);
        break;
    case SPACE:
        value |= (0x04 << 8);
        break;
    }
    
    switch(sbit) {
    case STOP_BIT_1:
        value |= (0x00 << 11);
        break;
    case STOP_BIT_15:
        value |= (0x01 << 11);
        break;
    case STOP_BIT_2:
        value |= (0x02 << 11);
        break;
    }
    
    if (usb_control_msg(ftdi->usb_dev, 0x40, 0x04, value, ftdi->index, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return (-1, "Setting new line property failed");
    
    return 0;
}

int ftdi_write_data(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    int ret;
    int offset = 0;
    int total_written = 0;

    while (offset < size) {
        int write_size = ftdi->writebuffer_chunksize;

        if (offset+write_size > size)
            write_size = size-offset;

        ret = usb_bulk_write(ftdi->usb_dev, ftdi->in_ep, buf+offset, write_size, ftdi->usb_write_timeout);
        if (ret < 0)
            ftdi_error_return(ret, "usb bulk write failed");

        total_written += ret;
        offset += write_size;
    }

    return total_written;
}


int ftdi_write_data_set_chunksize(struct ftdi_context *ftdi, unsigned int chunksize)
{
    ftdi->writebuffer_chunksize = chunksize;
    return 0;
}


int ftdi_write_data_get_chunksize(struct ftdi_context *ftdi, unsigned int *chunksize)
{
    *chunksize = ftdi->writebuffer_chunksize;
    return 0;
}


int ftdi_read_data(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    int offset = 0, ret = 1, i, num_of_chunks, chunk_remains;

    // everything we want is still in the readbuffer?
    if (size <= ftdi->readbuffer_remaining) {
        memcpy (buf, ftdi->readbuffer+ftdi->readbuffer_offset, size);

        // Fix offsets
        ftdi->readbuffer_remaining -= size;
        ftdi->readbuffer_offset += size;

        /* printf("Returning bytes from buffer: %d - remaining: %d\n", size, ftdi->readbuffer_remaining); */

        return size;
    }
    // something still in the readbuffer, but not enough to satisfy 'size'?
    if (ftdi->readbuffer_remaining != 0) {
        memcpy (buf, ftdi->readbuffer+ftdi->readbuffer_offset, ftdi->readbuffer_remaining);

        // Fix offset
        offset += ftdi->readbuffer_remaining;
    }
    // do the actual USB read
    while (offset < size && ret > 0) {
        ftdi->readbuffer_remaining = 0;
        ftdi->readbuffer_offset = 0;
        /* returns how much received */
        ret = usb_bulk_read (ftdi->usb_dev, ftdi->out_ep, ftdi->readbuffer, ftdi->readbuffer_chunksize, ftdi->usb_read_timeout);
        if (ret < 0)
            ftdi_error_return(ret, "usb bulk read failed");

        if (ret > 2) {
            // skip FTDI status bytes.
            // Maybe stored in the future to enable modem use
            num_of_chunks = ret / 64;
            chunk_remains = ret % 64;
            //printf("ret = %X, num_of_chunks = %X, chunk_remains = %X, readbuffer_offset = %X\n", ret, num_of_chunks, chunk_remains, ftdi->readbuffer_offset);

            ftdi->readbuffer_offset += 2;
            ret -= 2;

            if (ret > 62) {
                for (i = 1; i < num_of_chunks; i++)
                    memmove (ftdi->readbuffer+ftdi->readbuffer_offset+62*i,
                             ftdi->readbuffer+ftdi->readbuffer_offset+64*i,
                             62);
                if (chunk_remains > 2) {
                    memmove (ftdi->readbuffer+ftdi->readbuffer_offset+62*i,
                             ftdi->readbuffer+ftdi->readbuffer_offset+64*i,
                             chunk_remains-2);
                    ret -= 2*num_of_chunks;
                } else
                    ret -= 2*(num_of_chunks-1)+chunk_remains;
            }
        } else if (ret <= 2) {
            // no more data to read?
            return offset;
        }
        if (ret > 0) {
            // data still fits in buf?
            if (offset+ret <= size) {
                memcpy (buf+offset, ftdi->readbuffer+ftdi->readbuffer_offset, ret);
                //printf("buf[0] = %X, buf[1] = %X\n", buf[0], buf[1]);
                offset += ret;

                /* Did we read exactly the right amount of bytes? */
                if (offset == size)
                    //printf("read_data exact rem %d offset %d\n",
                    //ftdi->readbuffer_remaining, offset);
                    return offset;
            } else {
                // only copy part of the data or size <= readbuffer_chunksize
                int part_size = size-offset;
                memcpy (buf+offset, ftdi->readbuffer+ftdi->readbuffer_offset, part_size);

                ftdi->readbuffer_offset += part_size;
                ftdi->readbuffer_remaining = ret-part_size;
                offset += part_size;

                /* printf("Returning part: %d - size: %d - offset: %d - ret: %d - remaining: %d\n",
                part_size, size, offset, ret, ftdi->readbuffer_remaining); */

                return offset;
            }
        }
    }
    // never reached
    return -127;
}


int ftdi_read_data_set_chunksize(struct ftdi_context *ftdi, unsigned int chunksize)
{
    unsigned char *new_buf;

    // Invalidate all remaining data
    ftdi->readbuffer_offset = 0;
    ftdi->readbuffer_remaining = 0;

    if ((new_buf = (unsigned char *)realloc(ftdi->readbuffer, chunksize)) == NULL)
        ftdi_error_return(-1, "out of memory for readbuffer");

    ftdi->readbuffer = new_buf;
    ftdi->readbuffer_chunksize = chunksize;

    return 0;
}


int ftdi_read_data_get_chunksize(struct ftdi_context *ftdi, unsigned int *chunksize)
{
    *chunksize = ftdi->readbuffer_chunksize;
    return 0;
}



int ftdi_enable_bitbang(struct ftdi_context *ftdi, unsigned char bitmask)
{
    unsigned short usb_val;

    usb_val = bitmask; // low byte: bitmask
    /* FT2232C: Set bitbang_mode to 2 to enable SPI */
    usb_val |= (ftdi->bitbang_mode << 8);

    if (usb_control_msg(ftdi->usb_dev, 0x40, 0x0B, usb_val, ftdi->index, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return(-1, "unable to enter bitbang mode. Perhaps not a BM type chip?");

    ftdi->bitbang_enabled = 1;
    return 0;
}


int ftdi_disable_bitbang(struct ftdi_context *ftdi)
{
    if (usb_control_msg(ftdi->usb_dev, 0x40, 0x0B, 0, ftdi->index, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return(-1, "unable to leave bitbang mode. Perhaps not a BM type chip?");

    ftdi->bitbang_enabled = 0;
    return 0;
}


int ftdi_set_bitmode(struct ftdi_context *ftdi, unsigned char bitmask, unsigned char mode)
{
    unsigned short usb_val;

    usb_val = bitmask; // low byte: bitmask
    usb_val |= (mode << 8);
    if (usb_control_msg(ftdi->usb_dev, 0x40, 0x0B, usb_val, ftdi->index, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return(-1, "unable to configure bitbang mode. Perhaps not a 2232C type chip?");

    ftdi->bitbang_mode = mode;
    ftdi->bitbang_enabled = (mode == BITMODE_BITBANG || mode == BITMODE_SYNCBB)?1:0;
    return 0;
}

int ftdi_read_pins(struct ftdi_context *ftdi, unsigned char *pins)
{
    unsigned short usb_val;
    if (usb_control_msg(ftdi->usb_dev, 0xC0, 0x0C, 0, ftdi->index, (char *)&usb_val, 1, ftdi->usb_read_timeout) != 1)
        ftdi_error_return(-1, "read pins failed");

    *pins = (unsigned char)usb_val;
    return 0;
}


int ftdi_set_latency_timer(struct ftdi_context *ftdi, unsigned char latency)
{
    unsigned short usb_val;

    if (latency < 1)
        ftdi_error_return(-1, "latency out of range. Only valid for 1-255");

    usb_val = latency;
    if (usb_control_msg(ftdi->usb_dev, 0x40, 0x09, usb_val, ftdi->index, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return(-2, "unable to set latency timer");

    return 0;
}


int ftdi_get_latency_timer(struct ftdi_context *ftdi, unsigned char *latency)
{
    unsigned short usb_val;
    if (usb_control_msg(ftdi->usb_dev, 0xC0, 0x0A, 0, ftdi->index, (char *)&usb_val, 1, ftdi->usb_read_timeout) != 1)
        ftdi_error_return(-1, "reading latency timer failed");

    *latency = (unsigned char)usb_val;
    return 0;
}


void ftdi_eeprom_initdefaults(struct ftdi_eeprom *eeprom)
{
    eeprom->vendor_id = 0x0403;
    eeprom->product_id = 0x6001;

    eeprom->self_powered = 1;
    eeprom->remote_wakeup = 1;
    eeprom->BM_type_chip = 1;

    eeprom->in_is_isochronous = 0;
    eeprom->out_is_isochronous = 0;
    eeprom->suspend_pull_downs = 0;

    eeprom->use_serial = 0;
    eeprom->change_usb_version = 0;
    eeprom->usb_version = 0x0200;
    eeprom->max_power = 0;

    eeprom->manufacturer = NULL;
    eeprom->product = NULL;
    eeprom->serial = NULL;
}


/*
    ftdi_eeprom_build
    
    Build binary output from ftdi_eeprom structure.
    Output is suitable for ftdi_write_eeprom.
    
    Return codes:
    positive value: used eeprom size
    -1: eeprom size (128 bytes) exceeded by custom strings
*/
int ftdi_eeprom_build(struct ftdi_eeprom *eeprom, unsigned char *output)
{
    unsigned char i, j;
    unsigned short checksum, value;
    unsigned char manufacturer_size = 0, product_size = 0, serial_size = 0;
    int size_check;

    if (eeprom->manufacturer != NULL)
        manufacturer_size = strlen(eeprom->manufacturer);
    if (eeprom->product != NULL)
        product_size = strlen(eeprom->product);
    if (eeprom->serial != NULL)
        serial_size = strlen(eeprom->serial);

    size_check = 128; // eeprom is 128 bytes
    size_check -= 28; // 28 are always in use (fixed)
    size_check -= manufacturer_size*2;
    size_check -= product_size*2;
    size_check -= serial_size*2;

    // eeprom size exceeded?
    if (size_check < 0)
        return (-1);

    // empty eeprom
    memset (output, 0, 128);

    // Addr 00: Stay 00 00
    // Addr 02: Vendor ID
    output[0x02] = eeprom->vendor_id;
    output[0x03] = eeprom->vendor_id >> 8;

    // Addr 04: Product ID
    output[0x04] = eeprom->product_id;
    output[0x05] = eeprom->product_id >> 8;

    // Addr 06: Device release number (0400h for BM features)
    output[0x06] = 0x00;

    if (eeprom->BM_type_chip == 1)
        output[0x07] = 0x04;
    else
        output[0x07] = 0x02;

    // Addr 08: Config descriptor
    // Bit 1: remote wakeup if 1
    // Bit 0: self powered if 1
    //
    j = 0;
    if (eeprom->self_powered == 1)
        j = j | 1;
    if (eeprom->remote_wakeup == 1)
        j = j | 2;
    output[0x08] = j;

    // Addr 09: Max power consumption: max power = value * 2 mA
    output[0x09] = eeprom->max_power;
    ;

    // Addr 0A: Chip configuration
    // Bit 7: 0 - reserved
    // Bit 6: 0 - reserved
    // Bit 5: 0 - reserved
    // Bit 4: 1 - Change USB version
    // Bit 3: 1 - Use the serial number string
    // Bit 2: 1 - Enable suspend pull downs for lower power
    // Bit 1: 1 - Out EndPoint is Isochronous
    // Bit 0: 1 - In EndPoint is Isochronous
    //
    j = 0;
    if (eeprom->in_is_isochronous == 1)
        j = j | 1;
    if (eeprom->out_is_isochronous == 1)
        j = j | 2;
    if (eeprom->suspend_pull_downs == 1)
        j = j | 4;
    if (eeprom->use_serial == 1)
        j = j | 8;
    if (eeprom->change_usb_version == 1)
        j = j | 16;
    output[0x0A] = j;

    // Addr 0B: reserved
    output[0x0B] = 0x00;

    // Addr 0C: USB version low byte when 0x0A bit 4 is set
    // Addr 0D: USB version high byte when 0x0A bit 4 is set
    if (eeprom->change_usb_version == 1) {
        output[0x0C] = eeprom->usb_version;
        output[0x0D] = eeprom->usb_version >> 8;
    }


    // Addr 0E: Offset of the manufacturer string + 0x80
    output[0x0E] = 0x14 + 0x80;

    // Addr 0F: Length of manufacturer string
    output[0x0F] = manufacturer_size*2 + 2;

    // Addr 10: Offset of the product string + 0x80, calculated later
    // Addr 11: Length of product string
    output[0x11] = product_size*2 + 2;

    // Addr 12: Offset of the serial string + 0x80, calculated later
    // Addr 13: Length of serial string
    output[0x13] = serial_size*2 + 2;

    // Dynamic content
    output[0x14] = manufacturer_size*2 + 2;
    output[0x15] = 0x03; // type: string

    i = 0x16, j = 0;

    // Output manufacturer
    for (j = 0; j < manufacturer_size; j++) {
        output[i] = eeprom->manufacturer[j], i++;
        output[i] = 0x00, i++;
    }

    // Output product name
    output[0x10] = i + 0x80;  // calculate offset
    output[i] = product_size*2 + 2, i++;
    output[i] = 0x03, i++;
    for (j = 0; j < product_size; j++) {
        output[i] = eeprom->product[j], i++;
        output[i] = 0x00, i++;
    }

    // Output serial
    output[0x12] = i + 0x80; // calculate offset
    output[i] = serial_size*2 + 2, i++;
    output[i] = 0x03, i++;
    for (j = 0; j < serial_size; j++) {
        output[i] = eeprom->serial[j], i++;
        output[i] = 0x00, i++;
    }

    // calculate checksum
    checksum = 0xAAAA;

    for (i = 0; i < 63; i++) {
        value = output[i*2];
        value += output[(i*2)+1] << 8;

        checksum = value^checksum;
        checksum = (checksum << 1) | (checksum >> 15);
    }

    output[0x7E] = checksum;
    output[0x7F] = checksum >> 8;

    return size_check;
}


int ftdi_read_eeprom(struct ftdi_context *ftdi, unsigned char *eeprom)
{
    int i;

    for (i = 0; i < 128; i++) {
        if (usb_control_msg(ftdi->usb_dev, 0xC0, 0x90, 0, i, eeprom+(i*2), 2, ftdi->usb_read_timeout) != 2)
            ftdi_error_return(-1, "reading eeprom failed");
    }

    return 0;
}


int ftdi_write_eeprom(struct ftdi_context *ftdi, unsigned char *eeprom)
{
    unsigned short usb_val;
    int i;

    for (i = 0; i < 64; i++) {
        usb_val = eeprom[i*2];
        usb_val += eeprom[(i*2)+1] << 8;
        if (usb_control_msg(ftdi->usb_dev, 0x40, 0x91, usb_val, i, NULL, 0, ftdi->usb_write_timeout) != 0)
            ftdi_error_return(-1, "unable to write eeprom");
    }

    return 0;
}


int ftdi_erase_eeprom(struct ftdi_context *ftdi)
{
    if (usb_control_msg(ftdi->usb_dev, 0x40, 0x92, 0, 0, NULL, 0, ftdi->usb_write_timeout) != 0)
        ftdi_error_return(-1, "unable to erase eeprom");

    return 0;
}


char *ftdi_get_error_string (struct ftdi_context *ftdi)
{
    return ftdi->error_str;
}
