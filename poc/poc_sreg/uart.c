#include <avr/io.h>

#include <avr/signal.h>

#include <avr/interrupt.h>

#include <uart.h>



#define BAUDRATE 9600
#define F_CPU 8000000


#define nop() __asm volatile ("nop")



#define SUART_TXD_PORT PORTB

#define SUART_TXD_DDR  DDRB

#define SUART_TXD_BIT  PB6



static volatile uint16_t outframe;



void uart_init()

{

    uint8_t sreg = SREG, tifr = 0;

    cli();

   

    // Mode #4 für Timer1

    // und volle MCU clock

    // IC Noise Cancel

    // IC on Falling Edge

    TCCR1A = 0;

    TCCR1B = (1 << WGM12) | (1 << CS10) | (0 << ICES1) | (1 << ICNC1);



    // PoutputCompare für gewünschte Timer1 Frequenz

    OCR1A = (uint16_t) ((uint32_t) F_CPU/BAUDRATE);

   

    tifr |= (1 << OCF1A);

    SUART_TXD_PORT |= (1 << SUART_TXD_BIT);

    SUART_TXD_DDR  |= (1 << SUART_TXD_BIT);

    outframe = 0;

   

    TIFR = tifr;

   

    SREG = sreg;

}



//#### TXD PART ##############################################



void uart_putc (const char c)

{

    do

    {   

        sei(); nop(); cli(); // yield();

    } while (outframe);



    // frame = *.P.7.6.5.4.3.2.1.0.S   S=Start(0), P=Stop(1), *=Endemarke(1)

    outframe = (3 << 9) | (((uint8_t) c) << 1);

   

    TIMSK |= (1 << OCIE1A);

    TIFR   = (1 << OCF1A);

   

    sei();

}



SIGNAL (SIG_OUTPUT_COMPARE1A)

{

    uint16_t data = outframe;

   

    if (data & 1)      SUART_TXD_PORT |=  (1 << SUART_TXD_BIT);

    else               SUART_TXD_PORT &= ~(1 << SUART_TXD_BIT);

   

    if (1 == data)

    {

        TIMSK &= ~(1 << OCIE1A);

    }   

   

    outframe = data >> 1;

}



void uart_puts( uint8_t *txt )			// send string

{

  while( *txt )

    uart_putc ( *txt++ );

}

