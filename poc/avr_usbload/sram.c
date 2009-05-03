#include <stdlib.h>
#include <stdint.h> 
#include <avr/io.h>

#include "sram.h"
#include "uart.h"
#include "debug.h"

void spi_init(void)
{
	/* Set MOSI and SCK output, all others input */
	SPI_DIR |= ((1<<S_MOSI) | (1<<S_SCK) | (1<<S_LATCH));
	SPI_DIR &= ~(1<<S_MISO);
	SPI_PORT |= (1<<S_MISO);
	/* Enable SPI, Master*/
	SPCR = ((1<<SPE) | (1<<MSTR));
}

void spi_master_transmit(unsigned char cData)
{
	/* Start transmission */
	SPDR = cData;

	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
}


void sram_set_addr(uint32_t addr)
{
	spi_master_transmit((uint8_t)(addr>>16));
	spi_master_transmit((uint8_t)(addr>>8));
	spi_master_transmit((uint8_t)(addr>>0));

	LATCH_PORT |= (1<<S_LATCH);
    LATCH_PORT &= ~(1<<S_LATCH);
}

uint8_t sram_read(uint32_t addr)
{
	uint8_t byte;

	RAM_DIR = 0x00;
	RAM_PORT = 0xff;

	CTRL_PORT |= (1<<R_RD);
	CTRL_PORT |= (1<<R_WR);

	spi_master_transmit((uint8_t)(addr>>16));
	spi_master_transmit((uint8_t)(addr>>8));
	spi_master_transmit((uint8_t)(addr>>0));

	LATCH_PORT |= (1<<S_LATCH);
    LATCH_PORT &= ~(1<<S_LATCH);
    CTRL_PORT &= ~(1<<R_RD);
	
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");

	byte = RAM_REG;
	CTRL_PORT |= (1<<R_RD);
	RAM_DIR =0x00;
	RAM_PORT =0x00;
	return byte;
}

void sram_write(uint32_t addr, uint8_t data)
{
	RAM_DIR = 0xff;

	CTRL_PORT |= (1<<R_RD);
	CTRL_PORT |= (1<<R_WR);

	spi_master_transmit((uint8_t)(addr>>16));
	spi_master_transmit((uint8_t)(addr>>8));
	spi_master_transmit((uint8_t)(addr>>0));

	LATCH_PORT |= (1<<S_LATCH);
	LATCH_PORT &= ~(1<<S_LATCH);
	

	CTRL_PORT &= ~(1<<R_WR);

	RAM_PORT = data;
	CTRL_PORT |= (1<<R_WR);

	RAM_DIR = 0x00;
	RAM_PORT = 0x00;
}

void  sram_init(void){
	
	RAM_DIR  = 0x00;
	RAM_PORT = 0x00;

	CTRL_DIR 	|= ((1<<R_WR) | (1<<R_RD));
	CTRL_PORT  	|= (1<<R_RD);
	CTRL_PORT 	|= (1<<R_WR);

	LED_PORT |= (1<<D_LED0);
}

void sram_snes_mode01(void){
	CTRL_PORT |= (1<<R_WR);
    CTRL_PORT &= ~(1<<R_RD);
}

void sram_snes_mode02(void){
    CTRL_DIR  |= (1<<R_WR);
	CTRL_PORT |= (1<<R_WR);
    //CTRL_PORT &= ~(1<<R_RD);
    CTRL_DIR  &= ~(1<<R_RD);
    CTRL_PORT &= ~(1<<R_RD);
    
}


void sram_clear(uint32_t addr, uint32_t len){

	uint32_t i;
	for (i=addr; i<(addr + len);i++ ){
		if (0==i%0xfff)
	    	printf("sram_clear %lx\n\r",i);
		sram_write(i, 0x00);
	}
}

void sram_copy(uint32_t addr,uint8_t *src, uint32_t len){

	uint32_t i;
	uint8_t *ptr = src;
	for (i=addr; i<(addr + len);i++ )
		sram_write(i, *ptr++);
}

void sram_read_buffer(uint32_t addr,uint8_t *dst, uint32_t len){

	uint32_t i;
	uint8_t *ptr = dst;
	for (i=addr; i<(addr + len);i++ ){
		*ptr = sram_read(i);
		ptr++;
	}
}
uint8_t sram_check(uint8_t *buffer, uint32_t len){
    uint16_t cnt;
	for (cnt=0; cnt<len; cnt++) 
		if (buffer[cnt])
			return 1;
	return 0;
}
