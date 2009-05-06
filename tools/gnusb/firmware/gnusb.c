// ==============================================================================
// gnusbcore.c
// globals and utilities for gnusbCore - OPEN SOURCE USB SENSOR BOX
//
// License:
// The project is built with AVR USB driver by Objective Development, which is
// published under an own licence based on the GNU General Public License (GPL).
// usb2dmx is also distributed under this enhanced licence. See Documentation.
//
// target-cpu: ATMega16 @ 12MHz
// created 2007-01-28 Michael Egger me@anyma.ch
//

// ==============================================================================
// includes
// ------------------------------------------------------------------------------
// AVR Libc (see http://www.nongnu.org/avr-libc/)
#include <avr/io.h>			// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>	// include interrupt support
#include <avr/pgmspace.h> 	
#include <avr/wdt.h>		// include watchdog timer support
#include <avr/sleep.h>		// include cpu sleep support

// USB driver by Objective Development (see http://www.obdev.at/products/avrusb/index.html)
#include "usbdrv.h"

// local includes
#include "gnusb.h"		// gnusb setup and utility functions 


// ==============================================================================
// - sleepIfIdle
// ------------------------------------------------------------------------------
void sleepIfIdle()
{
	if(TIFR & BV(TOV1)) {
		cli(); 
		if(!(GIFR & BV(INTF1))) {
			// no activity on INT1 pin for >3ms => suspend:
						
			// - reconfigure INT1 to level-triggered and enable for wake-up
			cbi(MCUCR, ISC10);
			sbi(GICR, INT1);
			// -------------- go to sleep
				
			//cbi(ADCSRA, ADIE);				// disable ADC interrupts
//			cbi(ADCSRA, ADEN);				// disable ADC (turn off ADC power)

			PORTA = 0;						// pull all pins low
			PORTB = 0;
			PORTC = 0;
			PORTD = 0x60;					// except LEDs - they light if pin is low...

			wdt_disable();
			sleep_enable();
			sei();
			sleep_cpu();
			
			// -------------- wake up
			sleep_disable();
			// - reconfigure INT1 to any edge for SE0-detection
			cbi(GICR, INT1);
			sbi(MCUCR, ISC10);
			// - re-enable watchdog
			wdt_reset();
			wdt_enable(WDTO_1S);
			
	//		sbi(ADCSRA, ADIE);				// enable ADC interrupts
	//		sbi(ADCSRA, ADEN);				// enable ADC
			
			PORTD = 0x70; 	// set Pullup for Bootloader Jumper, no pullups on USB pins -> 0111 0000
			ledOn(STATUS_LED_GREEN);

			
		}
		sei();
		// clear INT1 flag
		sbi(GIFR, INTF1);
		// reload timer and clear overflow
		TCCR1B = 1;
		TCNT1 = 25000;		// max ca. 3ms between SE0
		sbi(TIFR, TOV1);
	}
}


// ------------------------------------------------------------------------------
// - INT1_vec (dummy for wake-up)
// ------------------------------------------------------------------------------
ISR(INT1_vect) {}



// ------------------------------------------------------------------------------
// - Write to EEPROM
// ------------------------------------------------------------------------------
// from PowerSwitch by Objective Development
 void eepromWrite(unsigned char addr, unsigned char val)
{
    while(EECR & (1 << EEWE));
    EEARL = addr;
    EEDR = val;
    cli();
    EECR |= 1 << EEMWE;
    EECR |= 1 << EEWE;  /* must follow within a couple of cycles -- therefore cli() */
    sei();
}


// ------------------------------------------------------------------------------
// - Read EEPROM
// ------------------------------------------------------------------------------
// from PowerSwitch by Objective Development
uchar eepromRead(uchar addr)
{
    while(EECR & (1 << EEWE));
    EEARL = addr;
    EECR |= 1 << EERE;
    return EEDR;
}

// ------------------------------------------------------------------------------
// - Status Leds
// ------------------------------------------------------------------------------
// 							(on means  set to 0 as we sink the LEDs )
void ledOff(uchar led) {
	PORTD |=  1 << led;
}

void ledOn(uchar led){
	PORTD &= ~(1 << led);
}

void ledToggle(uchar led){
	PORTD ^= 1 << led;
}

// ------------------------------------------------------------------------------
// - ADC Utilities
// ------------------------------------------------------------------------------
int ad_ConversionComplete (void) {
	return (!(ADCSRA & (1 << ADSC)));
}

 int ad_Read10bit (void) {
 	return (ADCL | ADCH << 8);
 }
 
 int ad_Read8bit (void) {
 	return ad_Read10bit() >> 2;
 }
 
 void ad_SetChannel (uchar mux) {
 	ADMUX = (ADMUX & ~ADC_MUX_MASK) | (mux & ADC_MUX_MASK);		// set channel
 }
 
 void ad_StartConversion () {
 			ADCSRA |= (1 << ADIF);			// clear hardware "conversion complete" flag 
			ADCSRA |= (1 << ADSC);			// start conversion
}

// ------------------------------------------------------------------------------
// - USB Reset
// ------------------------------------------------------------------------------
// Set USB- and USB+ Pins to output and pull them low for more than 10ms

 void usbReset(void) {
 	u08  i, j;
 	
	USBOUT &= ~USBMASK;	// make sure USB pins are pulled low 	
 	USBDDR |= USBMASK;	// set USB pins to output -> SE0
 	
    j = 0;
    while(--j) {       
        i = 0;
        while(--i);     // delay >10ms for USB reset
    }
 
    USBDDR &= ~USBMASK; // set USB pins to input
}

// ==============================================================================
// - Init hardware
// ------------------------------------------------------------------------------
void initCoreHardware(void)
{
	// --------------------- Init AD Converter

	sbi(ADCSRA, ADEN);				// enable ADC (turn on ADC power)
	cbi(ADCSRA, ADATE);				// default to single sample convert mode
									// Set ADC-Prescaler (-> precision vs. speed)
	ADCSRA = ((ADCSRA & ~ADC_PRESCALE_MASK) | ADC_PRESCALE_DIV64);
	sbi(ADMUX,REFS0);cbi(ADMUX,REFS1);			// Set ADC Reference Voltage to AVCC
				
	cbi(ADCSRA, ADLAR);				// set to right-adjusted result
//	sbi(ADCSRA, ADIE);				// enable ADC interrupts
	cbi(ADCSRA, ADIE);				// disable ADC interrupts

	// --------------------- Init USB
	
	// set PORT D Directions -> 1110 0000, output 0 on unconnected PD7
	DDRD = 0xe0; 	// 1110 0000 -> set PD0..PD4 to inputs -> USB pins
	PORTD = 0x70; 	// set Pullup for Bootloader Jumper, no pullups on USB pins -> 0111 0000

	usbDeviceConnect();
	wdt_enable(WDTO_1S);	// enable watchdog timer
	usbReset();
    usbInit();

	// --------------------- Init Sleep
	
	// init Timer 1  and Interrupt 1 for usb activity detection:
	// - set INT1 to any edge (polled by sleepIfIdle())
	cbi(MCUCR, ISC11);
	sbi(MCUCR, ISC10);
	
	// set sleep mode to full power-down for minimal consumption
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	// - set Timer 1 prescaler to 64 and restart timer
	TCCR1B = 3;
	TCNT1 = 0;
	sbi(TIFR, TOV1);

	sei();		// turn on interrupts
}

// ------------------------------------------------------------------------------
// - Start Bootloader
// ------------------------------------------------------------------------------
// dummy function doing the jump to bootloader section (Adress 1C00 on Atmega16)
void (*jump_to_bootloader)(void) = 0x1C00; __attribute__ ((unused))

void startBootloader(void) {
		
		eepromWrite(UBOOT_SOFTJUMPER_ADDRESS,UBOOT_SOFTJUMPER); 	// set software jumper

		cli();							// turn off interrupts
		wdt_disable();					// disable watchdog timer
		usbDeviceDisconnect(); 			// disconnect gnusb from USB bus
		
		cbi(ADCSRA, ADIE);				// disable ADC interrupts
		cbi(ADCSRA, ADEN);				// disable ADC (turn off ADC power)

		PORTA = 0;						// pull all pins low
		PORTB = 0;
		PORTC = 0;

		jump_to_bootloader();
}

