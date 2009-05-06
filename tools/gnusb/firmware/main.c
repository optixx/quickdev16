// ==============================================================================
// main.c
// firmware for a device based on the gnusb - OPEN SOURCE USB SENSOR BOX
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

#include "gnusb.h"				// the gnusb library: setup and utility functions 

// ==============================================================================
// Constants
// ------------------------------------------------------------------------------
#define ADC_PAUSE 		10		// number of passes before we sample the next AD channel
#define LED_KEEP_ALIVE	100  	// number of passes before usb status led turns off
#define USB_REPLY_PORTB	8		// Values of portb gets stored into 9th byte of usb_reply (counting from 0)
#define USB_REPLY_PORTC	9	


// ==============================================================================
// Globals
// ------------------------------------------------------------------------------

static u08		ad_mux;			// current ad input channel
static u16		ad_values[8];	// sampled ad input values
static u08		ad_smoothing;	// smoothing level of ad samples (0 -  15)
static u08		ad_samplepause;	// counts up to ADC_PAUSE between samples


static u08		usb_reply[12];	// 8 bytes AD Values (8 most significant bits)
								// 1 byte PORTB
								// 1 byte PORTC
								// 2 bytes least significant bits of AD values



// ------------------------------------------------------------------------------
// - usbFunctionSetup
// ------------------------------------------------------------------------------
// this function gets called when the usb driver receives a non standard request
// that is: our own requests defined in ../common/gnusb_cmds.h
// here's where the magic happens...

uchar usbFunctionSetup(uchar data[8])
{
	
	switch (data[1]) {
	// 								----------------------------  get all values		
		case GNUSB_CMD_POLL:    
		
			usbMsgPtr = usb_reply;
	        return sizeof(usb_reply);
    		break;
    		
	// 								----------------------------  set smoothing
		case GNUSB_CMD_SET_SMOOTHING:		
			
			if (data[2] > 15) ad_smoothing = 15;
			else ad_smoothing = data[2];
			break;
	// 								----------------------------  output one byte on PORTB
		case GNUSB_CMD_SET_PORTB:					
		
			DDRB = 0xff;							// set PORTB to output
			PORTB = data[2];						// output values
			usb_reply[USB_REPLY_PORTB] = data[2];	// mirror data in next poll
			break;

	// 								----------------------------  output one byte on  PORTC	    		
		case GNUSB_CMD_SET_PORTC:					
		
			DDRC = 0xff;							// set PORTC to output
			PORTC = data[2];						// output values
			usb_reply[USB_REPLY_PORTC] = data[2];	// mirror data in next poll
			break;

	// 								----------------------------  set PORTB to input  		
		case GNUSB_CMD_INPUT_PORTB:
		
			DDRB = 0x00;
			break;
	// 								----------------------------  set PORTC to input  		
		case GNUSB_CMD_INPUT_PORTC:
		
			DDRC = 0x00;
			break;
			
			
	// 								----------------------------   Start Bootloader for reprogramming the gnusb    		
		case GNUSB_CMD_START_BOOTLOADER:

			startBootloader();
			break;
			
		default:
			break;
				
	} 
	
	return 0;
}



// ------------------------------------------------------------------------------
// - Check ADC and update ad_values
// ------------------------------------------------------------------------------

void checkAnlogPorts (void) {
	unsigned int temp,replymask,replyshift,replybyte;
	
	if (ad_samplepause != 0xff) {													
		if (ad_samplepause < ADC_PAUSE) {
			ad_samplepause++;								// advance pause counter
		} else {
			ad_StartConversion();							// start a new conversion
			ad_samplepause = 0xff;							// indicate we're waiting for a result now
		}
		
	} else {

		if ( ad_ConversionComplete() ) {								// see if AD-Conversion is complete
				
			temp = ad_Read10bit();										// read ADC (10 bits);		
			
			// basic low pass filter
			ad_values[ad_mux] = (ad_values[ad_mux] * ad_smoothing + temp) / (ad_smoothing + 1);
			
			usb_reply[ad_mux] = ad_values[ad_mux] >> 2;			// copy 8 most significant bits to usb reply 
	
			// if you don't need 10bit precision you can leave out the following stuff 		
				replybyte = 10 + (ad_mux / 4);				// are we writing to byte 10 or 11?
				replyshift = ((ad_mux % 4) * 2);			// how much to shift the bits
				replymask = (3 << replyshift);				// create bitmask
															// write bits to the right place
				usb_reply[replybyte] =	
					(usb_reply[replybyte] & ~replymask) | (replymask & (ad_values[ad_mux] << replyshift));
	
			ad_mux = (ad_mux + 1) % 8;									// advance multiplexer index
			ad_SetChannel(ad_mux);										// set mutliplexer channel
			ad_samplepause = 0;											// start counting up to ADC_PAUSE in order to let the input settle a bit 
		}
	}
}

// ------------------------------------------------------------------------------
// - Check PORT B and PORT C
// ------------------------------------------------------------------------------

void checkDigitalPorts(void) {
	// copy state of pins to usb reply, only if the port is configured as an input
	if (DDRB == 0x00) usb_reply[USB_REPLY_PORTB] = PINB;
	if (DDRC == 0x00) usb_reply[USB_REPLY_PORTC] = PINC;	
}




// ==============================================================================
// - main
// ------------------------------------------------------------------------------
int main(void)
{
	// ------------------------- Initialize Hardware
	
	// PORTA: AD Converter
	DDRA 	= 0x00;		// set all pins to input
	PORTA 	= 0x00;		// make sure pull-up resistors are turned off

	// PORTB: Default Input
	DDRB 	= 0x00;		// set all pins to input
	PORTB 	= 0xff;		// make sure pull-up resistors are turned ON

	// PORTC: Default output
	DDRC 	= 0xff;		// set all pins to output
	PORTC 	= 0xff;		// turn off all leds
	
	// PORTD: gnusbCore stuff: USB, status leds, jumper
	initCoreHardware();
	ledOn(STATUS_LED_GREEN);

	// ------------------------- Main Loop
	while(1) {
        wdt_reset();		// reset Watchdog timer - otherwise Watchdog will reset gnusb
        sleepIfIdle();		// go to low power mode if host computer is sleeping
		usbPoll();			// see if there's something going on on the usb bus
	
		checkAnlogPorts();		// see if we've finished an analog-digital conversion
		checkDigitalPorts();	// have a look at PORTB and PORTC
		
	}
	return 0;
}

