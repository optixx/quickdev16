/*
 * Name: set-led.c Project: custom-class, a basic USB example Author:
 * Christian Starkjohann Creation Date: 2008-04-10 Tabsize: 4 Copyright: (c)
 * 2008 by OBJECTIVE DEVELOPMENT Software GmbH License: GNU GPL v2 (see
 * License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt) This
 * Revision: $Id: set-led.c 692 2008-11-07 15:07:40Z cs $
 */

/*
 * General Description: This is the host-side driver for the custom-class
 * example device. It searches the USB for the LEDControl device and sends the
 * requests understood by this device. This program must be linked with libusb
 * on Unix and libusb-win32 on Windows. See http://libusb.sourceforge.net/ or
 * http://libusb-win32.sourceforge.net/ respectively. 
 */


#define READ_BUFFER_SIZE    (1024 * 32)
#define SEND_BUFFER_SIZE    128
#define BANK_SIZE           (1<<15)
#define BANK_SIZE_SHIFT     15

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <usb.h>                /* this is libusb */
#include "opendevice.h"         /* common code moved to separate module */

#include "../requests.h"        /* custom request numbers */
#include "../usbconfig.h"       /* device's VID/PID and names */



void dump_packet(uint32_t addr, uint32_t len, uint8_t * packet)
{
    uint16_t i,
     j;
    uint16_t sum = 0;
    uint8_t clear = 0;

    for (i = 0; i < len; i += 16) {

        sum = 0;
        for (j = 0; j < 16; j++) {
            sum += packet[i + j];
        }
        if (!sum) {
            clear = 1;
            continue;
        }
        if (clear) {
            printf("*\n");
            clear = 0;
        }
        printf("%08x:", addr + i);
        for (j = 0; j < 16; j++) {
            printf(" %02x", packet[i + j]);
        }
        printf(" |");
        for (j = 0; j < 16; j++) {
            if (packet[i + j] >= 33 && packet[i + j] <= 126)
                printf("%c", packet[i + j]);
            else
                printf(".");
        }
        printf("|\n");
    }
}


uint16_t crc_xmodem_update(uint16_t crc, uint8_t data)
{
    int i;
    crc = crc ^ ((uint16_t) data << 8);
    for (i = 0; i < 8; i++) {
        if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
    }

    return crc;
}

uint16_t do_crc(uint8_t * data, uint16_t size)
{
    uint16_t crc = 0;
    uint16_t i;
    for (i = 0; i < size; i++) {
        crc = crc_xmodem_update(crc, data[i]);
    }
    return crc;
}


uint16_t do_crc_update(uint16_t crc, uint8_t * data, uint16_t size)
{
    uint16_t i;
    for (i = 0; i < size; i++)
        crc = crc_xmodem_update(crc, data[i]);
    return crc;
}



static void usage(char *name)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s upload filename.. upload\n", name);
}


int main(int argc, char **argv)
{
    usb_dev_handle *handle = NULL;
    const unsigned char rawVid[2] = { USB_CFG_VENDOR_ID }, rawPid[2] = {
    USB_CFG_DEVICE_ID};
    char vendor[] = { USB_CFG_VENDOR_NAME, 0 }, product[] = {
    USB_CFG_DEVICE_NAME, 0};
    int cnt, vid, pid;
    uint8_t *read_buffer;
    uint8_t *crc_buffer;
    uint8_t *ptr;
    
    uint32_t addr = 0;
    uint16_t addr_lo = 0;
    uint16_t addr_hi = 0;
    uint16_t step = 0;
    uint16_t crc = 0;
    uint8_t bank = 0;
    
    FILE *fp;
    usb_init();
    if (argc < 2) {             /* we need at least one argument */
        usage(argv[0]);
        exit(1);
    }
    /*
     * compute VID/PID from usbconfig.h so that there is a central source
     * of information
     */
    vid = rawVid[1] * 256 + rawVid[0];
    pid = rawPid[1] * 256 + rawPid[0];
    /*
     * The following function is in opendevice.c: 
     */
    if (usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) !=
        0) {
        fprintf(stderr,
                "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n",
                product, vid, pid);
        exit(1);
    }
    printf("Open USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid,
           pid);
    if (strcasecmp(argv[1], "upload") == 0) {
        if (argc < 3) {         /* we need at least one argument */
            usage(argv[0]);
            exit(1);
        }
        fp = fopen(argv[2], "r");
        if (fp == NULL) {
            fprintf(stderr, "Cannot open file %s ", argv[2]);
            exit(1);
        }
        read_buffer = (unsigned char *) malloc(READ_BUFFER_SIZE);
        crc_buffer = (unsigned char *) malloc(0x1000);
        memset(crc_buffer, 0, 0x1000);
        addr = 0x000000;
        cnt = usb_control_msg(handle,
                        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                        USB_BULK_UPLOAD_INIT, BANK_SIZE_SHIFT , 0, NULL, 0, 5000);

        if (cnt < 0) {
            fprintf(stderr, "USB error: %s\n", usb_strerror());
            usb_close(handle);
            exit(-1);
        } 
        ptr = crc_buffer;
        while ((cnt = fread(read_buffer, READ_BUFFER_SIZE, 1, fp)) > 0) {
            ptr = crc_buffer;
            for (step = 0; step < READ_BUFFER_SIZE; step += SEND_BUFFER_SIZE) {
                
                
                addr_lo = addr & 0xffff;
                addr_hi = (addr >> 16) & 0x00ff;

                cnt = usb_control_msg(handle,
                                USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                                USB_ENDPOINT_OUT, USB_BULK_UPLOAD_ADDR, addr_hi,
                                addr_lo, (char *) read_buffer + step,
                                SEND_BUFFER_SIZE, 5000);
                
                if (cnt < 0) {
                    fprintf(stderr, "USB error: %s\n", usb_strerror());
                    usb_close(handle);
                    exit(-1);
                } 
                
                memcpy(ptr, read_buffer + step, SEND_BUFFER_SIZE);
                addr += SEND_BUFFER_SIZE;
                ptr += SEND_BUFFER_SIZE;
                if ( addr % 0x1000 == 0){
                    crc = do_crc(crc_buffer, 0x1000);
                    printf ("bank=0x%02x addr=0x%08x addr=0x%08x crc=0x%04x\n", bank, addr - 0x1000, addr, crc);
                    ptr = crc_buffer;
                    if ( addr % 0x8000 == 0) {
                        bank++;
                        
                    }
                }                
            }
        }
        bank = 0;
        
        fseek(fp, 0, SEEK_SET);
        while ((cnt = fread(read_buffer, READ_BUFFER_SIZE, 1, fp)) > 0) {
            printf ("bank=0x%02x crc=0x%04x\n", bank++, 
                do_crc(read_buffer, READ_BUFFER_SIZE));
        }
        fclose(fp);
        
        cnt = usb_control_msg(handle,
                            USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                            USB_ENDPOINT_OUT, USB_SNES_BOOT, 0, 0, NULL,
                            0, 5000);
        

        if (cnt < 1) {
            if (cnt < 0) {
                fprintf(stderr, "USB error: %s\n", usb_strerror());
            } else {
                fprintf(stderr, "only %d bytes received.\n", cnt);
            }
        }
    } else if (strcasecmp(argv[1], "crc") == 0) {
        /*
         * if(argc < 2){ usage(argv[0]); exit(1); } 
         */
        addr = 0x000000;
        addr_lo = addr & 0xffff;
        addr_hi = (addr >> 16) & 0xff;
        printf("Request CRC for Addr: 0x%06x\n", addr);

        cnt = usb_control_msg(handle,
                              USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                              USB_ENDPOINT_OUT, USB_CRC_ADDR, addr_hi, addr_lo,
                              NULL, (1 << 15) / 4, 5000);

        if (cnt < 1) {
            if (cnt < 0) {
                fprintf(stderr, "USB error: %s\n", usb_strerror());
            } else {
                fprintf(stderr, "only %d bytes received.\n", cnt);
            }
        }
    } else {
        usage(argv[0]);
        exit(1);
    }
    usb_close(handle);
    return 0;
}
