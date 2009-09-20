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
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>         
#include <avr/pgmspace.h>       
#include <avr/eeprom.h>


#include "pwm.h"
#include "debug.h"
#include "info.h"
#include "sram.h"
#include "util.h"
#include "uart.h"
 
#define RECEIVE_BUF_LEN 40
 
uint8_t			recv_buf[RECEIVE_BUF_LEN];
volatile uint8_t	recv_counter = 0;
volatile uint8_t	cr = 0;

uint8_t *token_ptr;

uint8_t *get_token(void)
{
 	uint8_t *p = token_ptr;
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

uint8_t get_dec(uint32_t *decval)
{
 	const uint8_t *t;
 	t = get_token();
 	if (t != NULL) {
 		int x = util_sscandec(t);
 		if (x < 0)
 			return 0;
 		*decval = x;
 		return 1;
 	}
 	return 0;
 }

uint8_t parse_hex(const uint8_t *s, uint32_t *hexval)
{
 	uint32_t x = util_sscanhex(s);
 	if (x > 0xffffff)
 		return 0;
 	*hexval = (uint32_t) x;
 	return 1;
 }

uint8_t get_hex(uint32_t *hexval)
{
 	const uint8_t *t;
 	t = get_token();
 	if (t != NULL)
 		return parse_hex(t, hexval);
 	return 0;
 }

uint8_t get_hex_arg2(uint32_t *hexval1, uint32_t *hexval2)
{
 	return get_hex(hexval1) && get_hex(hexval2);
}

uint8_t get_hex_arg3(uint32_t *hexval1, uint32_t *hexval2, uint32_t *hexval3)
{
 	return get_hex(hexval1) && get_hex(hexval2) && get_hex(hexval3);
}

static uint8_t get_int32(uint32_t *val)
 {
 	if (!get_hex(val)){
 		printf("Invalid argument!\n");
        return 0;
    } else {
        return 1;
    }
 }

 static uint8_t get_int8(uint8_t *val)
 {
 	uint32_t ret;
 	if (!get_hex(&ret) ||ret > 0xff){
 		printf("Invalid argument!\n");
        return 0;
 	}else{
 		*val = (uint8_t)ret;
        return 1;
    }
 }

 static int get_bool(void)
 {
 	const uint8_t *t;
 	t = get_token();
 	if (t != NULL) {
 		int result = util_sscanbool(t);
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


void shellrun(void)
{
    uint8_t command_buf[RECEIVE_BUF_LEN];
	uint8_t *t;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    
	if (!cr)
        return;
    cr=0;
	strcpy((char*)command_buf, (char*)recv_buf);
	
	token_ptr = command_buf;
	t = get_token();
	if (t == NULL)
		return;

	util_strupper(t);

	if (strcmp((char*)t, "DUMP") == 0) {
        if (get_hex_arg2(&arg1,&arg2))
            dump_memory(arg1,arg2);
        else 
            printf("ERROR: arg parsing\n");
    }
}