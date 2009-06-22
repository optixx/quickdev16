
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>


//SREG defines
#define S_MOSI 	PB3
#define S_MISO	PB4
#define S_SCK  	PB5
#define S_LATCH	PB2

//DEBUG defines
#define D_LED0	PC5

//SRAM defines
#define R_WR	PB6
#define R_RD	PB7
#define R_DATA	PORTD
#define R_DIR	DDRD



void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB |= ((1<<S_MOSI) | (1<<S_SCK) | (1<<S_LATCH));
	DDRB &= ~(1<<S_MISO);
	PORTB |= (1<<S_MISO);

	/* Enable SPI, Master*/
	SPCR = ((1<<SPE) | (1<<MSTR));
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
	PORTD=0xff;
	
	PORTB |= (1<<R_RD);
	PORTB |= (1<<R_WR);

	SPI_MasterTransmit((uint8_t)(addr>>16));	
	SPI_MasterTransmit((uint8_t)(addr>>8));
	SPI_MasterTransmit((uint8_t)(addr>>0));

	PORTB |= (1<<S_LATCH);  
    	PORTB &= ~(1<<S_LATCH);

	PORTB &= ~(1<<R_RD);

	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	
	byte = PIND;
	
	PORTB |= (1<<R_RD);

	DDRD=0x00;
	PORTD=0x00;

	return byte;
}

void SRAM_Write(uint32_t addr, uint8_t data)
{
	DDRD=0xff;

	PORTB |= (1<<R_RD);
	PORTB |= (1<<R_WR);

	SPI_MasterTransmit((uint8_t)(addr>>16));	
	SPI_MasterTransmit((uint8_t)(addr>>8));
	SPI_MasterTransmit((uint8_t)(addr>>0));

	PORTB |= (1<<S_LATCH);  
	PORTB &= ~(1<<S_LATCH);

	PORTB &= ~(1<<R_WR);
	
	PORTD=data;

	PORTB |= (1<<R_WR);

	DDRD=0x00;	
	PORTD=0x00;
}



int main(void)
{

    while(1);
	return(0);
}

