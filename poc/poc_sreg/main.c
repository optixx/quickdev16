#include <avr/io.h>
#define F_CPU 8000000
#include <util/delay.h>
#include <stdlib.h>

#include "uart.h"

//SREG defines
#define S_MOSI 	PB3
#define S_SCK  	PB5
#define S_LATCH	PB2

//DEBUG defines
#define D_LED0	PB0

//SRAM defines
#define R_WR	PB1
#define R_RD	PB7
#define R_DATA	PORTD
#define R_DIR	DDRD


//Software UART defines
//	SUART_RX PB6


void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB |= ((1<<S_MOSI) | (1<<S_SCK) | (1<<S_LATCH) );

	/* Enable SPI, Master, set clock rate fck */
	SPCR = ((1<<SPE)|(1<<MSTR));
}

void SPI_MasterTransmit(unsigned char cData)
{
	/* Start transmission */
	SPDR = cData;
	
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
}

uint8_t SRAM_Read(uint32_t addr)
{
	uint8_t byte;

	DDRD=0x00;
	PORTD=0x00;
	
	PORTB |= (1<<R_RD);
	PORTB |= (1<<R_WR);


	SPI_MasterTransmit((uint8_t)(addr>>8));	
	SPI_MasterTransmit((uint8_t)(addr>>4));
	SPI_MasterTransmit((uint8_t)(addr>>0));

	PORTB |= (1<<S_LATCH);  
    	PORTB &= ~(1<<S_LATCH);

	PORTB &= ~(1<<R_RD);

	asm volatile ("nop");
	asm volatile ("nop");
	
	byte = PIND;
	
	PORTB |= (1<<R_RD);

	return byte;
}

void SRAM_Write(uint32_t addr, uint8_t data)
{
	DDRD=0xff;

	PORTB |= (1<<R_RD);
	PORTB |= (1<<R_WR);

	SPI_MasterTransmit((uint8_t)(addr>>8));	
	SPI_MasterTransmit((uint8_t)(addr>>4));
	SPI_MasterTransmit((uint8_t)(addr>>0));

	PORTB |= (1<<S_LATCH);  
    	PORTB &= ~(1<<S_LATCH);

	PORTB &= ~(1<<R_WR);
	
	PORTD=data;

	PORTB |= (1<<R_WR);
}



int main(void)
{

	uint8_t read, buf[2], i;
	DDRB |= ((1<<D_LED0) | (1<<R_WR) | (1<<R_RD));    	
	PORTB |= (1<<R_RD);
	PORTB |= (1<<R_WR);

	SPI_MasterInit();

	uart_init();



	SRAM_Write(0x00000000,0x23);
	SRAM_Write(0x00000001, 0x42);

	while(1){
		i++;

		PORTB ^= ((1<<D_LED0));

		read = SRAM_Read(0x00000000);
		itoa(read,buf,16);
		uart_puts("dump: ");
		uart_puts(buf);
		uart_putc(0x20);

		read = SRAM_Read(0x00000001);
		itoa(read,buf,16);
		uart_puts(buf);
		uart_putc(0x20);

		read = SRAM_Read(0x00000002);
		itoa(read,buf,16);
		uart_puts(buf);
		uart_putc(0x20);

		read = SRAM_Read(0x00000003);
		itoa(read,buf,16);
		uart_puts(buf);
		uart_putc(0x20);

		read = SRAM_Read(0x00000004);
		itoa(read,buf,16);
		uart_puts(buf);
		uart_putc(0x20);

		if(i==10){
			SRAM_Write(0x00000002,0xBE);
			SRAM_Write(0x00000003,0xEF);
		}
		if(i==11)
			while(1);

		uart_putc(10);
		uart_putc(13);
	}

	return(0);
}

