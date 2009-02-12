#ifndef _UART_H_
#define _UART_H_

#define SUART_TXD
#define SUART_RXD

#include <avr/io.h>
#include <stdio.h>

void uart_init();

#ifdef SUART_TXD
    void uart_putc(uint8_t byte);
    void uart_puts(uint8_t *buf);
#endif // SUART_RXD

#ifdef SUART_RXD
    uint8_t uart_getc_wait();
    uint8_t uart_getc_nowait();
#endif // SUART_RXD

#endif /* _UART_H_ */

