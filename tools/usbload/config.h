/* configuratino file for usbload */

/* after this timeout, the main application ist started, if no usb
 * connection is detected */
#define TIMEOUT 50

/* uncomment this if you need to define some other signature bytes */
//#define SIGNATURE_BYTES 0x23, 0x24, 0x25, 0

/* uncomment this if you want to catch the eeprom isp bytewise read/write
 * commands.  costs ~34 byte */
//#define ENABLE_CATCH_EEPROM_ISP

/* uncomment this if you want a usb echo function (for communication testing) */
//#define ENABLE_ECHO_FUNC

/* uncomment this for debug information via uart */
//#define DEBUG_UART
