/*
snesram.h - Snesram support for uCON64

Copyright (c) 2009 david@optixx.org


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef SNESRAM_H
#define SNESRAM_H

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

/* -------------------------- Device Description --------------------------- */

#define  USB_CFG_VENDOR_ID       0xc0, 0x16
/* USB vendor ID for the device, low byte first. If you have registered your
 * own Vendor ID, define it here. Otherwise you use one of obdev's free shared
 * VID/PID pairs. Be sure to read USBID-License.txt for rules!
 */
#define  USB_CFG_DEVICE_ID       0xdd, 0x05
/* This is the ID of the product, low byte first. It is interpreted in the
 * scope of the vendor ID. If you have registered your own VID with usb.org
 * or if you have licensed a PID from somebody else, define it here. Otherwise
 * you use obdev's free shared VID/PID pair. Be sure to read the rules in
 * USBID-License.txt!
 */
#define USB_CFG_DEVICE_VERSION  0x00, 0x01
/* Version number of the device: Minor number first, then major number.
 */
#define USB_CFG_VENDOR_NAME     'o', 'p', 't', 'i', 'x', 'x', '.', 'o', 'r', 'g'
#define USB_CFG_VENDOR_NAME_LEN 10
/* These two values define the vendor name returned by the USB device. The name
 * must be given as a list of characters under single quotes. The characters
 * are interpreted as Unicode (UTF-16) entities.
 * If you don't want a vendor name string, undefine these macros.
 * ALWAYS define a vendor name containing your Internet domain name if you use
 * obdev's free shared VID/PID pair. See the file USBID-License.txt for
 * details.
 */
#define USB_CFG_DEVICE_NAME     'S', 'N', 'E', 'S', 'R', 'A', 'M'
#define USB_CFG_DEVICE_NAME_LEN 7
/* Same as above for the device name. If you don't want a device name, undefine
 * the macros. See the file USBID-License.txt before you assign a name if you
 * use a shared VID/PID.
 */

extern const st_getopt2_t snesram_usage[];

#ifdef USE_USB
extern int snesram_write_rom (const char *filename);
#endif

#endif // SNESRAM_H
