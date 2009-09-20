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
 
 
 static char *token_ptr;

 static char *get_token(void)
 {
 	char *p = token_ptr;
 	while (*p == ' ')
 		p++;
 	if (*p == '\0')
 		return NULL;
 	token_ptr = p;
 	do {
 		token_ptr++;
 		if (*token_ptr == ' ') {
 			*token_ptr++ = '\0';
 			break;
 		}
 	} while (*token_ptr != '\0');
 	return p;
 }

 static int get_dec(int *decval)
 {
 	const char *t;
 	t = get_token();
 	if (t != NULL) {
 		int x = Util_sscandec(t);
 		if (x < 0)
 			return FALSE;
 		*decval = x;
 		return TRUE;
 	}
 	return FALSE;
 }

 static int parse_hex(const char *s, UWORD *hexval)
 {
 	int x = Util_sscanhex(s);
 #ifdef MONITOR_HINTS
 	int y = find_label_value(s);
 	if (y >= 0) {
 		if (x < 0 || x > 0xffff || x == y) {
 			*hexval = (UWORD) y;
 			return TRUE;
 		}
 		/* s can be a hex number or a label name */
 		printf("%s is ambiguous. Use 0%X or %X instead.\n", s, x, y);
 		return FALSE;
 	}
 #endif
 	if (x < 0 || x > 0xffff)
 		return FALSE;
 	*hexval = (UWORD) x;
 	return TRUE;
 }

 static int get_hex(UWORD *hexval)
 {
 	const char *t;
 	t = get_token();
 	if (t != NULL)
 		return parse_hex(t, hexval);
 	return FALSE;
 }

 static int get_hex2(UWORD *hexval1, UWORD *hexval2)
 {
 	return get_hex(hexval1) && get_hex(hexval2);
 }

 static int get_hex3(UWORD *hexval1, UWORD *hexval2, UWORD *hexval3)
 {
 	return get_hex(hexval1) && get_hex(hexval2) && get_hex(hexval3);
 }

 static void get_uword(UWORD *val)
 {
 	if (!get_hex(val))
 		printf("Invalid argument!\n");
 }

 static void get_ubyte(UBYTE *val)
 {
 	UWORD uword;
 	if (!get_hex(&uword) || uword > 0xff)
 		printf("Invalid argument!\n");
 	else
 		*val = (UBYTE) uword;
 }

 static int get_bool(void)
 {
 	const char *t;
 	t = get_token();
 	if (t != NULL) {
 		int result = Util_sscanbool(t);
 		if (result >= 0)
 			return result;
 	}
 	printf("Invalid argument (should be 0 or 1)!\n");
 	return -1;
 } 
 
ISR(USART0_RX_vect)		//	Interrupt for UART Byte received
{
  UCSR0B &= (255 - (1<<RXCIE0));//	Interrupts disable for RxD
  sei();
  if(recv_counter == (sizeof(recv_buf)-1)) {
    cr=1;
    recv_buf[recv_counter]='\0';
    recv_counter=0;
    uart_putc('\r');
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
    uart_putc('\r');
    uart_putc('\n');
    uart_putc(':');
    uart_putc('>');
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
