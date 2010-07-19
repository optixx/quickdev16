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


volatile char rxbuff;



static int uart_stream(char c, FILE * stream);

FILE uart_stdout = FDEV_SETUP_STREAM(uart_stream, NULL, _FDEV_SETUP_WRITE);

void uart_init(void)
{
    UCSR0A = _BV(U2X0);         /* improves baud rate error @ F_CPU = 1 MHz */
    UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0);     /* tx/rx enable, rx complete * intr */
    UBRR0L = (F_CPU / (8 * 115200UL)) - 1;


}

/*
 * ISR(USART0_RX_vect) { uint8_t c; c = UDR0; if (bit_is_clear(UCSR0A, FE0)) { rxbuff = c; intflags.rx_int = 1; } } 
 */

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
