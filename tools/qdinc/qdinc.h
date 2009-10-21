#ifndef __QDINC_H__
#define __QDINC_H__

#define QDINC_RANGE 96
#define QDINC_SIZE 254

/* quickdev protocol defines */

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
#define USB_SET_LOADER          13

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
#define USB_CFG_DEVICE_NAME     'Q', 'U', 'I', 'C', 'K', 'D', 'E', 'V', '1', '6'
#define USB_CFG_DEVICE_NAME_LEN 10

/* end of quickdev protocol defines */

#endif
