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
 #include <string.h>
 #include <avr/io.h>
 #include <avr/interrupt.h>

 #include "pwm.h"
 #include "debug.h"
 #include "info.h"
 #include "sram.h"
 
 #define RECEIVE_BUF_LEN 40
 
 uint8_t			recv_buf[RECEIVE_BUF_LEN];
 volatile uint8_t	recv_counter = 0;
 volatile uint8_t	cr = 0;
 
ISR(USART0_RX_vect)		//	Interrupt for UART Byte received
{
  UCSR0B &= (255 - (1<<RXCIE0));//	Interrupts disable for RxD
  sei();
  if(recv_counter == (sizeof(recv_buf)-1)) {
    cr=1;
    recv_buf[recv_counter]='\0';
    recv_counter=0;
    uart_putc('\n');
    uart_putc(':');
    uart_putc('>');
  }
  recv_buf[recv_counter] = UDR0;
  uart_putc(recv_buf[recv_counter]); /* do a echo, maybe should reside not in interrupt */
  if (recv_buf[recv_counter] == 0x0d) {
    /* recv_buf[recv_counter] = 0; */
    cr = 1;				//	found a CR, so the application should do something
    recv_buf[++recv_counter]='\0';  // terminate string
    recv_counter = 0;
    uart_putc('\n');
  } else {
    // we accept backspace or delete
    if ((recv_buf[recv_counter] == 0x08 || recv_buf[recv_counter] == 0x7f) && recv_counter > 0) {
      recv_counter--;
    } else {
      recv_counter++;
    }
  }
  UCSR0B |= (1<<RXCIE0);//	Interrupts enable for RxD
}
