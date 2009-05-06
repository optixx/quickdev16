/*
 * =====================================================================================
 *
 *       Filename:  snesram_bootloader.c 
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

 *      inspired by
 *      AVRUSBBoot - USB bootloader for Atmel AVR controllers
 *      Thomas Fischl <tfischl@gmx.de>
 *
 * =====================================================================================
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/boot.h>

#include "usbdrv.h"
#include "../usb_cmds.h"        

#define SNESRAM_BOOT_SOFTJUMPER_ADDRESS	0x05
#define SNESRAM_BOOT_SOFTJUMPER			0xd9


static uchar replyBuffer[8];    // reply buffer for USB
static unsigned int page;       // address of page we're currently writing to
static unsigned int byte_counter;       // address of next byte to write to
static uchar ready;

void (*jump_to_app) (void) = 0x0000;

void leaveBootloader()
{
    cli();
    boot_rww_enable();
    GICR = (1 << IVCE);         /* enable change of interrupt vectors */
    GICR = (0 << IVSEL);        /* move interrupts to application flash section */
    jump_to_app();
}

void writePage(void)
{
    PORTD ^= (1 << 5);          // Toggle yellow led

    eeprom_busy_wait();

    cli();
    boot_page_erase(page);      // erase page
    boot_spm_busy_wait();       // wait until page is erased
    boot_page_write(page);      // Store buffer in flash page.
    boot_spm_busy_wait();       // Wait until the memory is written.
    sei();

    byte_counter = 0;
    page += SPM_PAGESIZE;
}

uchar usbFunctionSetup(uchar data[8])
{
    uchar len = 0;

    usbMsgPtr = replyBuffer;

    if (data[1] == SNESRAM_BOOT_CMD_LEAVE) {
        usbDeviceDisconnect();
        leaveBootloader();

    } else if (data[1] == SNESRAM_BOOT_CMD_START) {
        page = 0;
        byte_counter = 0;
        ready = 1;
    } else if (data[1] == SNESRAM_BOOT_CMD_STATUS) {

        if (byte_counter >= SPM_PAGESIZE) {
            writePage();
        }
        replyBuffer[0] = ready;
        len = 1;

    } else if (data[1] == SNESRAM_BOOT_CMD_WRITE) {

        replyBuffer[0] = data[2];
        replyBuffer[1] = data[3];
        len = 2;

        cli();
        boot_page_fill(page + byte_counter, data[3] | (data[2] << 8));
        sei();

        byte_counter += 2;

    } else if (data[1] == SNESRAM_BOOT_CMD_FINISH) {
        if (byte_counter) {
            writePage();
        }
        ready = 0;
        PORTD &= ~(1 << 5);     // Light yellow Led 

    } else if (data[1] == SNESRAM_BOOT_CMD_CLEAR_FLAG) {

        while (EECR & (1 << EEWE)) ;
        // write 00 to eeprom
        EEARL = SNESRAM_BOOT_SOFTJUMPER_ADDRESS;
        EEDR = 0x00;
        cli();
        EECR |= 1 << EEMWE;
        EECR |= 1 << EEWE;      // must follow within a couple of cycles -- therefore cli() 
        sei();

    }

    return len;
}

int main(void)
{
    // set PORT D Directions -> 1110 0000, output 0 on USB pullup PD7
    DDRD = 0xe0;                // 1110 0000 -> set PD0..PD4 to inputs -> USB pins
    PORTD = 0x70;               // 0111 0000 -> set Pullup for Bootloader Jumper, no pullups on USB pins

    // see if hardware jumper is set
    if (!((PIND & (1 << 4)) == 0)) {

        // no jumper, let's see if we have softjumper flag in EEPROM            
        while (EECR & (1 << EEWE)) ;
        EEARL = SNESRAM_BOOT_SOFTJUMPER_ADDRESS;
        EECR |= 1 << EERE;
        if (EEDR != SNESRAM_BOOT_SOFTJUMPER)
            leaveBootloader();
    }

    GICR = (1 << IVCE);         // enable change of interrupt vectors 
    GICR = (1 << IVSEL);        // move interrupts to boot flash section 

    uchar i, j;
    PORTD = 0xd0;               // 1101 0000 -> pull up pd7 for device connect, pullup on jumper, light yellow led to show we're in bootloader
    DDRD = 0xe3;                // 1110 0011 -> set USB pins to output -> SE0
    j = 0;
    while (--j) {
        i = 0;
        while (--i) ;           // delay >10ms for USB reset
    }

    DDRD = 0xe0;                // 1110 0000 -> set USB pins to input

    usbInit();
    sei();                      // turn on interrupts

    while (1) {                 // main event loop
        usbPoll();
    }
    return 0;
}
