#ifndef __UART_H__
#define __UART_H__

#define CR "\r\n"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>

void uart_init(void);
void uart_putc(const uint8_t);
void uart_puts(const char *s);
void uart_puts_P(PGM_P s);
static int uart_stream(char c, FILE *stream);

#endif


