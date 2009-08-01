/*
 * =====================================================================================
 *
 * ________        .__        __    ________               ____  ________
 * \_____  \  __ __|__| ____ |  | __\______ \   _______  _/_   |/  _____/
 *  /  / \  \|  |  \  |/ ___\|  |/ / |    |  \_/ __ \  \/ /|   /   __  \
 * /   \_/.  \  |  /  \  \___|    <  |    `   \  ___/\   / |   \  |__\  \
 * \_____\ \_/____/|__|\___  >__|_ \/_______  /\___  >\_/  |___|\_____  /
 *        \__>             \/     \/        \/     \/                 \/
 *
 *                                  www.optixx.org
 *
 *
 *        Version:  1.0
 *        Created:  07/21/2009 03:32:16 PM
 *         Author:  david@optixx.org
 *
 * =====================================================================================
 */




#include <stdint.h>
#include <stdio.h>
#include <avr/io.h> 
#include <avr/io.h>
#include <avr/interrupt.h>      /* for sei() */

#include "debug.h" 
 
#ifndef OCR1A
    #define OCR1A OCR1  // 2313 support
#endif
 
#ifndef WGM12
    #define WGM12 CTC1  // 2313 support
#endif

//#define XTAL      11059201L   // nominal value
#define XTAL        20000000UL 

#define DEBOUNCE    500L        // debounce clock (256Hz = 4msec)
 
#define uint8_t unsigned char
#define uint unsigned int
 
uint16_t prescaler;
uint16_t volatile second;          // count seconds
 
 
ISR (SIG_OUTPUT_COMPARE1A)
{
 
#if XTAL % DEBOUNCE                     // bei rest
    OCR1A = 20000000UL / DEBOUNCE - 1;      // compare DEBOUNCE - 1 times
#endif
  if( --prescaler == 0 ){
    prescaler = (uint16_t)DEBOUNCE;
    second++;               // exact one second over
#if XTAL % DEBOUNCE         // handle remainder
    OCR1A = XTAL / DEBOUNCE + XTAL % DEBOUNCE - 1; // compare once per second
#endif
  }
}
 
void timer_start( void )
{
  TCCR1B = (1<<WGM12) | (1<<CS10);      // divide by 1
                                        // clear on compare
  OCR1A = XTAL / DEBOUNCE - 1UL;          // Output Compare Register
  TCNT1 = 0;                            // Timmer startet mit 0
  second = 0;
  prescaler = (uint16_t)DEBOUNCE;          //software teiler
  TIMSK1 = 1<<OCIE1A;                    // beim Vergleichswertes Compare Match                    
                                        // Interrupt (SIG_OUTPUT_COMPARE1A)
  sei();

}

uint16_t timer_stop_int(void)
{
    uint16_t t = ((DEBOUNCE - prescaler) / DEBOUNCE )  + second;
    return t;
}



