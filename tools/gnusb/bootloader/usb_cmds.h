/*
 * =====================================================================================
 *
 *       Filename:  usb_cmds.h
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
// start firmware upload (sets page adress to 0)
#define SNESRAM_BOOT_CMD_START 	0xf1

// write one word to flash buffer, write page if filled up
#define SNESRAM_BOOT_CMD_WRITE 	0xf2

// write last page if there's anything left to write tpo flash
#define SNESRAM_BOOT_CMD_FINISH 	0xf3

// leave bootloader and start main application
#define SNESRAM_BOOT_CMD_LEAVE 	0xf4

// clear software jumper in EEPROM to prevent bootloader from starting 
#define SNESRAM_BOOT_CMD_CLEAR_FLAG 	0xf5

// get bootloader version
#define SNESRAM_BOOT_CMD_GET_VERSION 	0xf6

#define SNESRAM_BOOT_CMD_STATUS 	0xf7

#define SNESRAM_BOOT_CMD_ENTER 	0xf8
