/*
 * =====================================================================================
 *
 *            .d8888b  88888b.   .d88b.  .d8888b  888d888 8888b.  88888b.d88b.
 *            88K      888 "88b d8P  Y8b 88K      888P"      "88b 888 "888 "88b
 *            "Y8888b. 888  888 88888888 "Y8888b. 888    .d888888 888  888  888
 *                 X88 888  888 Y8b.          X88 888    888  888 888  888  888
 *             88888P' 888  888  "Y8888   88888P' 888    "Y888888 888  888  888
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



#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "uart.h"
#include "fifo.h"

volatile struct {
    uint8_t tmr_int:1;
    uint8_t adc_int:1;
    uint8_t rx_int:1;
} intflags;

/*
 *  * Last character read from the UART.
 *   
 */
volatile char rxbuff;


FILE uart_stdout = FDEV_SETUP_STREAM(uart_stream, NULL, _FDEV_SETUP_WRITE);

void uart_init(void)
{
    UCSR0A = _BV(U2X0);           /* improves baud rate error @ F_CPU = 1 MHz */
    UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0); /* tx/rx enable, rx complete
                                                 * intr */
    UBRR0L = (F_CPU / (8 * 115200UL)) - 1;       


}


ISR(USART0_RX_vect)
{
    uint8_t c;
    c = UDR0;
    if (bit_is_clear(UCSR0A, FE0)) {
        rxbuff = c;
        intflags.rx_int = 1;
    }
}


void uart_putc(uint8_t c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}


void uart_puts(const char *s)
{
    do {
        uart_putc(*s);
    }
    while (*s++);
}

void uart_puts_P(PGM_P s)
{
    while (1) {
        unsigned char c = pgm_read_byte(s);
        s++;
        if ('\0' == c)
            break;
        uart_putc(c);
    }
}

static int uart_stream(char c, FILE * stream)
{
    if (c == '\n')
        uart_putc('\r');
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}
