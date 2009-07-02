#include <io.h>
#include <interrupt.h>
#include <signal.h>
 
 
#ifndef OCR1A
    #define OCR1A OCR1  // 2313 support
#endif
 
#ifndef WGM12
    #define WGM12 CTC1  // 2313 support
#endif

//#define XTAL      11059201L   // nominal value
#define XTAL        20000000L 

 
#define DEBOUNCE    256L        // debounce clock (256Hz = 4msec)
 
#define uint8_t unsigned char
#define uint unsigned int
 
uint8_t prescaler;
uint8_t volatile second;          // count seconds
 
 
SIGNAL (SIG_OUTPUT_COMPARE1A)
{
 
#if XTAL % DEBOUNCE                     // bei rest
  OCR1A = XTAL / DEBOUNCE - 1;      // compare DEBOUNCE - 1 times
#endif
  if( --prescaler == 0 ){
    prescaler = (uint8_t)DEBOUNCE;
    second++;               // exact one second over
#if XTAL % DEBOUNCE         // handle remainder
    OCR1A = XTAL / DEBOUNCE + XTAL % DEBOUNCE - 1; // compare once per second
#endif
  }
}
 
 
uint16_t timer_start( void )
{
  TCCR1B = (1<<WGM12) | (1<<CS10);      // divide by 1
                                        // clear on compare
  OCR1A = XTAL / DEBOUNCE - 1;          // Output Compare Register
  TCNT1 = 0;                            // Timmer startet mit 0
  second = 0;
  prescaler = (uint8_t)DEBOUNCE;          //software teiler
  TIMSK = 1<<OCIE1A;                    // beim Vergleichswertes Compare Match                    
                                        // Interrupt (SIG_OUTPUT_COMPARE1A)
  sei();
}

uint16_t timer_stop(void)
{
    cli():
    return second
}



