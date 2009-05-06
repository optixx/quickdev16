/*
 * =====================================================================================
 *
 *       Filename:  snesram_bootuploader.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  05/06/2009 03:06:26 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  David Voswinkel (DV), david@optixx.org
 *        Company:  Optixx
 *
 * =====================================================================================
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <usb.h>

#include "../usb_cmds.h"

#define USBDEV_SHARED_VENDOR    0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   0x05DC  /* Obdev's free shared PID */
#define USB_VENDORSTRING        "www.optixx.org"
#define USB_BOOTLOADERNAME      "gnusboot"
#define USB_PRODUCTNAME         "gnusb"
#define RETRIES                 6

usb_dev_handle *handle = NULL;
unsigned char buffer[8];
int nBytes;
int tries;

static int usbGetStringAscii(usb_dev_handle * dev, int index, int langid, char *buf, int buflen)
{
    char buffer[256];
    int rval, i;

    if ((rval =
         usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
                         (USB_DT_STRING << 8) + index, langid, buffer, sizeof(buffer), 1000)) < 0)
        return rval;
    if (buffer[1] != USB_DT_STRING)
        return 0;
    if ((unsigned char)buffer[0] < rval)
        rval = (unsigned char)buffer[0];
    rval /= 2;
    /*
     * lossy conversion to ISO Latin1 
     */
    for (i = 1; i < rval; i++) {
        if (i > buflen)         /* destination buffer overflow */
            break;
        buf[i - 1] = buffer[2 * i];
        if (buffer[2 * i + 1] != 0)     /* outside of ISO Latin1 range */
            buf[i - 1] = '?';
    }
    buf[i - 1] = 0;
    return i - 1;
}

static usb_dev_handle *findDevice(char *thedevice)
{
    struct usb_bus *bus;
    struct usb_device *dev;
    usb_dev_handle *handle = 0;
    tries++;
    usb_find_busses();
    usb_find_devices();
    for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == USBDEV_SHARED_VENDOR && dev->descriptor.idProduct == USBDEV_SHARED_PRODUCT) {
                char string[256];
                int len;
                handle = usb_open(dev); /* we need to open the device in
                                         * order to query strings */
                if (!handle) {
                    fprintf(stderr, "Warning: cannot open USB device: %s\n", usb_strerror());
                    continue;
                }
                /*
                 * now find out whether the device actually is obdev's
                 * Remote Sensor: 
                 */
                len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, 0x0409, string, sizeof(string));
                if (len < 0) {
                    fprintf(stderr, "warning: cannot query manufacturer for device: %s\n", usb_strerror());
                    goto skipDevice;
                }
                /*
                 * fprintf(stderr, "seen device from vendor ->%s<-\n",
                 * string); 
                 */
                if (strcmp(string, USB_VENDORSTRING) != 0)
                    goto skipDevice;
                len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
                if (len < 0) {
                    fprintf(stderr, "warning: cannot query product for device: %s\n", usb_strerror());
                    goto skipDevice;
                }
                /*
                 * fprintf(stderr, "seen product ->%s<-\n", string); 
                 */
                if (strcmp(string, thedevice) == 0)
                    break;
 skipDevice:
                usb_close(handle);
                handle = NULL;
            }
        }
        if (handle)
            break;
    }
    if (!handle) {
        if (tries < RETRIES) {
            handle = findDevice(thedevice);
        } else {
            fprintf(stderr, "Could not find USB device %s/%s\n", USB_VENDORSTRING, thedevice);
        }
    }
    return handle;
}

static int hextobin(unsigned char c)
{
    /*
     * http://www.scit.wlv.ac.uk/~jphb/sst/c/cgisd.html Function to
     * convert hex character to binary. Returns value or -1 if there is
     * any error. 
     */
    if (isdigit(c))
        return c - '0';
    else if (isxdigit(c))
        return tolower(c) - 'a' + 10;
    else
        return -1;
}

static void usage(char *name)
{
    fprintf(stderr, "\nusage:\n");
    fprintf(stderr, "  %s -upload file_to_upload.hex    upload hex file\n", name);
    fprintf(stderr,
            "  %s -version          print version number of both the bootloader and the command line tool\n", name);
    fprintf(stderr, "  %s -leave            leave bootloader and start application\n", name);
    fprintf(stderr, "  %s -clear_flag       clear softjumper\n", name);
    fprintf(stderr, "\n");

}

static void save_send_word(unsigned int to_send)
{
    unsigned int result;
    unsigned int ok;

    ok = 0;

    while (!ok) {
      nBytes =
            usb_control_msg(handle,
                            USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                            USB_ENDPOINT_IN, SNESRAM_BOOT_CMD_STATUS, 0, 0, (char *)buffer, sizeof(buffer), 100);
        if (nBytes == 1)
            ok = buffer[0];
        printf(".");
    }
    buffer[0] = 0;
    buffer[1] = 0;
    ok = 0;

    // while(!ok) {
    nBytes =
        usb_control_msg(handle,
                        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                        USB_ENDPOINT_IN, SNESRAM_BOOT_CMD_WRITE, to_send, 0, (char *)buffer, sizeof(buffer), 5000);
    if (nBytes == 2) {
        result = buffer[0] | (buffer[1] << 8);
        if (result != to_send)
            printf("COMMUNICATION ERROR: Sent %i but received %i\n", to_send, result);
    } else
        printf("COMMUNICATION ERROR: Only %i bytes received\n", nBytes);
    // }
    return;
}

static void send_file(char *filepath)
{
    int char_count, i, send_bytes = 0;
    int c;                      // Character read from the file.
    unsigned int sendWord = 0;  // word to send to uDMX
    FILE *fp;                   // Pointer to the file. 

    // Open the file 
    if ((fp = fopen(filepath, "rb")) == NULL) {
        printf("File %s open failed!\n", filepath);
        exit(1);
    };

    nBytes =
        usb_control_msg(handle,
                        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                        USB_ENDPOINT_OUT, SNESRAM_BOOT_CMD_START, 0, 0, (char *)buffer, sizeof(buffer), 100);

    i = 0;
    // Read one character at a time
    while ((c = fgetc(fp)) != EOF) {
        // hex file lines start with ':'
        if (c == ':') {

            char_count = 0;
            send_bytes = 0;

        } else {
            char_count++;
            printf("%c", c);

            // first two characters hold length of data 
            if (char_count < 3) {
                send_bytes += hextobin(c) << ((2 - char_count) * 4);
            }
            // check if line holds data (record type = 00)
            if (char_count == 8) {
                if (c != '0')
                    send_bytes = 0;
                printf(" ");
            }
            // ignore adress part of every line in hex file
            if (char_count > 8) {
                if (send_bytes) {
                    sendWord += hextobin(c) << ((3 - i) * 4);

                    if (i == 3) {

                        i = -1;
                        send_bytes -= 2;
                        printf(" ");
                        save_send_word(sendWord);       // send word over
                        // USB
                        sendWord = 0;
                    }

                    i++;
                }
            }
        }
    }

    // write last page to flash if there is anything left over
    nBytes =
        usb_control_msg(handle,
                        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                        USB_ENDPOINT_OUT, SNESRAM_BOOT_CMD_FINISH, 0, 0, (char *)buffer, sizeof(buffer), 100);

    fclose(fp);                 /* Close the file.  */
}

int main(int argc, char **argv)
{

    if (argc < 2) {
        usage(argv[0]);
        exit(1);
    }

    char *productName;

    // usb_set_debug(1);

    usb_init();
    productName = USB_PRODUCTNAME;
    if (argc > 2) {
        if (strcmp(argv[2], "-device") == 0)
            productName = argv[3];
    }

    if (strcmp(argv[1], "-upload") == 0) {

        if ((handle = findDevice(USB_BOOTLOADERNAME)) != NULL)
            send_file(argv[2]);

    } else if (strcmp(argv[1], "-version") == 0) {
        printf("Not implemented\n");

    } else if (strcmp(argv[1], "-leave") == 0) {
        if ((handle = findDevice(USB_BOOTLOADERNAME)) != NULL) {
            nBytes =
                usb_control_msg(handle,
                                USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                                USB_ENDPOINT_OUT, SNESRAM_BOOT_CMD_LEAVE, 0, 0, (char *)buffer, sizeof(buffer), 100);
        }
    } else if (strcmp(argv[1], "-enter") == 0) {
        if ((handle = findDevice(productName)) != NULL)
            nBytes =
                usb_control_msg(handle,
                                USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                                USB_ENDPOINT_OUT, SNESRAM_BOOT_CMD_ENTER, 0, 0, (char *)buffer, sizeof(buffer), 100);
    } else if (strcmp(argv[1], "-clear_flag") == 0) {
        if ((handle = findDevice(USB_BOOTLOADERNAME)) != NULL)
            nBytes =
                usb_control_msg(handle,
                                USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                                USB_ENDPOINT_OUT,
                                SNESRAM_BOOT_CMD_CLEAR_FLAG, 0, 0, (char *)buffer, sizeof(buffer), 100);
    } else
        usage(argv[0]);

    usb_close(handle);
    return 0;
}
