// ==============================================================================
// gnusb.h
// globals and utilities for gnusb - OPEN SOURCE USB SENSOR BOX
//
// License:
// The project is built with AVR USB driver by Objective Development, which is
// published under an own licence based on the GNU General Public License (GPL).
// gnusb is also distributed under this enhanced licence. See Documentation.
//
// target-cpu: ATMega16 @ 12MHz
// created 2007-01-28 Michael Egger me@anyma.ch
//
// ==============================================================================




// ==============================================================================
// includes
// ------------------------------------------------------------------------------
// AVR Libc (see http://www.nongnu.org/avr-libc/)
#include <avr/io.h>				// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>		// include interrupt support
#include <avr/pgmspace.h>
#include <avr/wdt.h>			// include watchdog timer support
#include <avr/sleep.h>			// include cpu sleep support

// USB driver by Objective Development (see http://www.obdev.at/products/avrusb/index.html)
#include "usbdrv.h"

// local includes
#include "../common/gnusb_cmds.h"		// USB command and error constants
										// common between client and host software

// ==============================================================================
// Constants
// ------------------------------------------------------------------------------

// Software jumper to initiate firmware updates via built in bootloader
#define UBOOT_SOFTJUMPER_ADDRESS	0x05
#define UBOOT_SOFTJUMPER			0xd9

#define STATUS_LED_YELLOW			5
#define	STATUS_LED_GREEN			6

#define F_CPU       		 12000000               		// 12MHz processor

// ==============================================================================
// UTILITY FUNCTIONS

// ------------------------------------------------------------------------------
// - Write to EEPROM
// ------------------------------------------------------------------------------
extern  void eepromWrite(unsigned char addr, unsigned char val);

// ------------------------------------------------------------------------------
// - Read EEPROM
// ------------------------------------------------------------------------------
extern uchar eepromRead(uchar addr);

// ------------------------------------------------------------------------------
// - Status Leds
// ------------------------------------------------------------------------------
extern void ledOn(uchar led);
extern void ledOff(uchar led);
extern void ledToggle(uchar led);

// ------------------------------------------------------------------------------
// - ADC Utilities
// ------------------------------------------------------------------------------

extern int ad_ConversionComplete (void);
extern int ad_Read10bit (void);
extern int ad_Read8bit (void);
extern void ad_SetChannel (uchar mux);
extern void ad_StartConversion ();

// ==============================================================================
// CORE FUNCTIONS

// ------------------------------------------------------------------------------
// - Init core hardware
// ------------------------------------------------------------------------------
// Sets DDR for USB, Led and jumper pins (PORTD on gnusbCore hardware)
// Starts Interrupts for Sleep mode and initializes USB port
extern  void initCoreHardware(void);

// ------------------------------------------------------------------------------
// - Start Bootloader
// ------------------------------------------------------------------------------
extern  void startBootloader(void);

// ------------------------------------------------------------------------------
// - sleepIfIdle
// ------------------------------------------------------------------------------
// call this function regularly to check if there is still activity on the USB bus
// puts the device to sleep if necessary.
extern void sleepIfIdle();

// ------------------------------------------------------------------------------
// - USB Reset
// ------------------------------------------------------------------------------
// Set USB- and USB+ Pins to output and pull them low for more than 10ms
// Will force host to reevaluate the device
 extern void usbReset(void);


// ==============================================================================
// Additional types
// ------------------------------------------------------------------------------
typedef unsigned char  u08;
typedef   signed char  s08;
typedef unsigned short u16;
typedef   signed short s16;


// convenience macros (from Pascal Stangs avrlib)
#ifndef BV
	#define BV(bit)			(1<<(bit))
#endif
#ifndef cbi
	#define cbi(reg,bit)	reg &= ~(BV(bit))
#endif
#ifndef sbi
	#define sbi(reg,bit)	reg |= (BV(bit))
#endif

// ==============================================================================
// From AVRLIB by Pascal Stang 
// ------------------------------------------------------------------------------

// A2D clock prescaler select
//		*selects how much the CPU clock frequency is divided
//		to create the A2D clock frequency
//		*lower division ratios make conversion go faster
//		*higher division ratios make conversions more accurate
#define ADC_PRESCALE_DIV2		0x00	///< 0x01,0x00 -> CPU clk/2
#define ADC_PRESCALE_DIV4		0x02	///< 0x02 -> CPU clk/4
#define ADC_PRESCALE_DIV8		0x03	///< 0x03 -> CPU clk/8
#define ADC_PRESCALE_DIV16		0x04	///< 0x04 -> CPU clk/16
#define ADC_PRESCALE_DIV32		0x05	///< 0x05 -> CPU clk/32
#define ADC_PRESCALE_DIV64		0x06	///< 0x06 -> CPU clk/64
#define ADC_PRESCALE_DIV128		0x07	///< 0x07 -> CPU clk/128
// default value
#define ADC_PRESCALE			ADC_PRESCALE_DIV64
// do not change the mask value
#define ADC_PRESCALE_MASK		0x07


// bit mask for A2D channel multiplexer
#define ADC_MUX_MASK			0x1F
