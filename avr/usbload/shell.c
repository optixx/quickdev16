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
#include "dump.h"
#include "irq.h"
#include "config.h"
#include "crc.h"
#include "command.h"
#include "shared_memory.h"

 

 
uint8_t command_buf[RECEIVE_BUF_LEN];
uint8_t	recv_buf[RECEIVE_BUF_LEN];

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
 		if (*token_ptr == ' ' || *token_ptr == '\n' || *token_ptr == '\r') {
 			*token_ptr++ = '\0';
 			break;
 		}
 	} while (*token_ptr != ' ' && *token_ptr != '\n' && *token_ptr != '\r');
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
 		info_P(PSTR("Invalid argument!\n"));
        return 0;
    } else {
        return 1;
    }
 }

 static uint8_t get_int8(uint8_t *val)
 {
 	uint32_t ret;
 	if (!get_hex(&ret) ||ret > 0xff){
 		info_P(PSTR("Invalid argument!\n"));
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
 	info_P(PSTR("Invalid argument (should be 0 or 1)!\n"));
 	return -1;
 } 
 void prompt(void){
     
     uart_putc('\r');
     uart_putc('\n');
     uart_putc('>');
     
 }
 
ISR(USART0_RX_vect)		//	Interrupt for UART Byte received
{
  UCSR0B &= (255 - (1<<RXCIE0));//	Interrupts disable for RxD
  sei();
  if(recv_counter == (sizeof(recv_buf)-1)) {
    cr=1;
    recv_buf[recv_counter]='\0';
    recv_counter=0;
    prompt();
  }
  recv_buf[recv_counter] = UDR0;
  uart_putc(recv_buf[recv_counter]); /* do a echo, maybe should reside not in interrupt */
  if (recv_buf[recv_counter] == 0x0d) {
    /* recv_buf[recv_counter] = 0; */
    cr = 1;				//	found a CR, so the application should do something
    recv_buf[++recv_counter]='\0';  // terminate string
    recv_counter = 0;
    prompt();
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

enum cmds { CMD_DUMP,
            CMD_CRC,
            CMD_EXIT,
            CMD_RESET,
            CMD_IRQ,
            CMD_AVR,
            CMD_SNES,
            CMD_LOROM,
            CMD_HIROM,
            CMD_WR,
            CMD_SHMWR,
            CMD_SHMSAVE,
            CMD_SHMRESTORE,
            CMD_LOADER,
            CMD_RECONNECT,
            CMD_STATUS,
            CMD_HELP
};

uint8_t cmdlist[][CMD_HELP] PROGMEM = {
            {"DUMP"},
            {"CRC"},
            {"EXIT"},
            {"RESET"},
            {"IRQ"},
            {"AVR"},
            {"SNES"},
            {"LOROM"},
            {"HIROM"},
            {"WR"},
            {"SHMWR"},
            {"SHMSAVE"},
            {"SHMRESTORE"},
            {"LOADER"},
            {"RECONNECT"},
            {"STATUS"},
            {"HELP"},
        };


void shell_help(void){
	uint8_t i;
    info_P(PSTR("\n"));
    for (i=CMD_DUMP; i<CMD_HELP; i++){
        info_P((PGM_P)cmdlist[i]);
        info_P(PSTR("\n"));
        
    }
}

void shell_run(void)
{
	uint8_t *t;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
	uint16_t crc;
    
	if (!cr)
        return;
    cr=0;
	strcpy((char*)command_buf, (char*)recv_buf);
	
	token_ptr = command_buf;
	t = get_token();
	
	if (t == NULL)
        shell_help();

	util_strupper(t);

	if (strcmp_P((const char*)t,(PGM_P)cmdlist[CMD_DUMP]) == 0) {
        if (get_hex_arg2(&arg1,&arg2))
            dump_memory(arg1,arg2);
        else 
            info_P(PSTR("DUMP <start addr> <end addr>\n"));
    
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_CRC]) == 0) {
        if (get_hex_arg2(&arg1,&arg2)){
            crc = crc_check_bulk_memory(arg1,arg2,0x8000);
            info_P(PSTR("0x%06lx - 0x%06lx crc=0x%04x\n"),arg1,arg2,crc);
        } else 
            info_P(PSTR("CRC <start addr> <end addr>\n"));
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_EXIT]) == 0) {
        leave_application();
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_RESET]) == 0) {
        send_reset();
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_IRQ]) == 0) {
        info_P(PSTR("Send IRQ\n"));
        snes_irq_on();
        snes_irq_lo();
        _delay_us(20);
        snes_irq_hi();
        snes_irq_off();
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_AVR]) == 0) {
         info_P(PSTR("Activate AVR bus\n"));
         avr_bus_active();
         snes_irq_lo();
         snes_irq_off();
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_SNES]) == 0) {
        info_P(PSTR("Activate SNES bus\n"));
        snes_irq_lo();
        snes_irq_off();
        snes_wr_disable();
        snes_bus_active();
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_LOROM]) == 0) {
        info_P(PSTR("Set LOROM\n"));
        snes_lorom();
        snes_wr_disable();
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_HIROM]) == 0) {
        info_P(PSTR("Set HIROM\n"));
        snes_hirom();
        snes_wr_disable();
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_WR]) == 0) {
        arg1 = get_bool();
        if(arg1==1){
            info_P(PSTR("Set WR enable"));
            snes_wr_enable();
        }else if (arg1==0){
            info_P(PSTR("Set WR disable"));
            snes_wr_disable();
        }
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_SHMWR]) == 0) {
        if (get_hex_arg2(&arg1,&arg2))
            shared_memory_write((uint8_t)arg1, (uint8_t)arg1);
        else 
            info_P(PSTR("SHMWR <command> <value>\n"));
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_SHMSAVE]) == 0) {
        shared_memory_scratchpad_region_tx_save();
        shared_memory_scratchpad_region_rx_save();
        info_P(PSTR("Save scratchpad\n"));
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_SHMRESTORE]) == 0) {
        shared_memory_scratchpad_region_tx_restore();
        shared_memory_scratchpad_region_rx_restore();
        info_P(PSTR("Restore scratchpad\n"));
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_LOADER]) == 0) {
        boot_startup_rom(500);    
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_RECONNECT]) == 0) {
        usb_connect();
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_STATUS]) == 0) {
            transaction_status();
    }else if (strcmp_P((char*)t, (PGM_P)cmdlist[CMD_HELP]) == 0) {
        shell_help();
    }    
    prompt();
    /*
    dias
    set irq vector
    set reset vector
    dump cart header
    */
}


