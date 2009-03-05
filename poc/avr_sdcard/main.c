#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "uart.h"
#include "mmc.h"
#include "fat.h"

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


#define DEBUG_BUFFER_SIZE 128
#define READ_BUFFER_SIZE 512

uint8_t debug_buffer[DEBUG_BUFFER_SIZE];
uint8_t read_buffer[READ_BUFFER_SIZE];

void dprintf(const uint8_t * fmt, ...) {

	va_list args;
    va_start(args, fmt);
    vsprintf(debug_buffer, fmt, args);
    va_end(args);
    uart_puts(debug_buffer);
}


void dump_packet(uint32_t addr,uint32_t len,uint8_t *packet){
	uint16_t i,j;
	uint16_t sum =0;
	for (i=0;i<len;i+=16) {
		sum = 0;
		for (j=0;j<16;j++) {
			sum +=packet[i+j];
		}
		if (!sum)
			continue;

		dprintf("%08x:", addr + i);
		for (j=0;j<16;j++) {
			dprintf(" %02x", packet[i+j]);
		}
		dprintf(" |");
		for (j=0;j<16;j++) {
			if (packet[i+j]>=33 && packet[i+j]<=126 )
				dprintf("%c", packet[i+j]);
			else
				dprintf(".");
		}
		dprintf("|\n");
	}
}


void spi_init(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB |= ((1<<S_MOSI) | (1<<S_SCK) | (1<<S_LATCH));
	DDRB &= ~(1<<S_MISO);
	PORTB |= (1<<S_MISO);
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

uint8_t sram_read(uint32_t addr)
{
	uint8_t byte;

	DDRD=0x00;
	PORTD=0xff;

	PORTB |= (1<<R_RD);
	PORTB |= (1<<R_WR);

	spi_master_transmit((uint8_t)(addr>>16));
	spi_master_transmit((uint8_t)(addr>>8));
	spi_master_transmit((uint8_t)(addr>>0));

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

void sram_write(uint32_t addr, uint8_t data)
{
	DDRD=0xff;

	PORTB |= (1<<R_RD);
	PORTB |= (1<<R_WR);

	spi_master_transmit((uint8_t)(addr>>16));
	spi_master_transmit((uint8_t)(addr>>8));
	spi_master_transmit((uint8_t)(addr>>0));

	PORTB |= (1<<S_LATCH);
	PORTB &= ~(1<<S_LATCH);

	PORTB &= ~(1<<R_WR);

	PORTD=data;

	PORTB |= (1<<R_WR);

	DDRD=0x00;
	PORTD=0x00;
}

void  sram_init(void){
	DDRD=0x00;
	PORTD=0x00;

	DDRB |= ((1<<R_WR) | (1<<R_RD));
	PORTB |= (1<<R_RD);
	PORTB |= (1<<R_WR);

	DDRC |= (1<<D_LED0);
}


void sram_clear(uint32_t addr, uint32_t len){

	uint32_t i;
	for (i=addr; i<(addr + len);i++ )
		sram_write(i, 0x00);
}

void sram_copy(uint32_t addr,uint8_t *src, uint32_t len){

	uint32_t i;
	uint8_t *ptr = src;
	for (i=addr; i<(addr + len);i++ )
		sram_write(addr, *ptr++);
}

void sram_read_buffer(uint32_t addr,uint8_t *dst, uint32_t len){

	uint32_t i;
	uint8_t *ptr = dst;
	for (i=addr; i<(addr + len);i++ ){
		*ptr = sram_read(addr);
		ptr++;
	}
}

int main(void)
{

	uint16_t 	fat_cluster = 0;
    uint8_t 	fat_attrib = 0;
    uint32_t 	fat_size = 0;
    uint32_t 	rom_addr = 0;



    uart_init();

    sram_init();
	dprintf("sram_init\n");

	spi_init();
	dprintf("spi_init\n");

	sram_clear(0x000000, 0x400000);
	dprintf("sram_clear\n");

	while ( mmc_init() !=0) {
		dprintf("no sdcard..\n\r");
    }
    dprintf("mmc_init\n\r");

    fat_init(read_buffer);
    dprintf("fat_init\n\r");


    rom_addr = 0x000000;
    dprintf("look for sprite.smc\n\r");

    if (fat_search_file((uint8_t*)"sprite.smc",
						&fat_cluster,
						&fat_size,
						&fat_attrib,
						read_buffer) == 1) {

        for (uint16_t block_cnt=0; block_cnt<512; block_cnt++) {
        	fat_read_file (fat_cluster,read_buffer,block_cnt);
        	dprintf("Read Block %i addr 0x%06\n",block_cnt,rom_addr);
        	sram_copy(rom_addr,read_buffer,512);
			rom_addr += 512;
        }
	}

    rom_addr = 0x000000;
    for (uint16_t block_cnt=0; block_cnt<512; block_cnt++) {
    	sram_read_buffer(rom_addr,read_buffer,512);
    	dump_packet(rom_addr,512,read_buffer);
		rom_addr += 512;
    }


	while(1);

	return(0);
}

