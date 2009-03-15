
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "uart.h"
#include "mmc.h"
#include "fat.h"

//SREG defines
#define S_MOSI 	PB5
#define S_MISO	PB6
#define S_SCK  	PB7
#define S_LATCH	PB4

//DEBUG defines
#define D_LED0	PD6

//SRAM defines
#define R_WR		PB1
#define R_RD		PB0

#define RAM_PORT	PORTA
#define RAM_DIR		DDRA
#define RAM_REG		PINA


#define CTRL_PORT	PORTB
#define CTR_DIR		DDRB


#define LATCH_PORT	PORTB
#define LATCH_DIR	DDRB

#define SPI_PORT	PORTB
#define SPI_DIR		DDRB


#define LED_PORT	PORTD
#define LED_DIR		DDRD


#define READ_BUFFER_SIZE 512
#define BLOCKS 512

#define debug(x, fmt) printf("%s:%u: %s=" fmt, __FILE__, __LINE__, #x, x)

extern FILE uart_stdout;

uint8_t read_buffer[READ_BUFFER_SIZE];

void dump_packet(uint32_t addr,uint32_t len,uint8_t *packet){
	uint16_t i,j;
	uint16_t sum =0;
	for (i=0;i<len;i+=16) {
		sum = 0;
		for (j=0;j<16;j++) {
			sum +=packet[i+j];
		}
		if (!sum){
			//printf(".");
			continue;
		}
		printf("%08lx:", addr + i);
		for (j=0;j<16;j++) {
			printf(" %02x", packet[i+j]);
		}
		printf(" |");
		for (j=0;j<16;j++) {
			if (packet[i+j]>=33 && packet[i+j]<=126 )
				printf("%c", packet[i+j]);
			else
				printf(".");
		}
		printf("|\n");
	}
}


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

	CTR_DIR 	|= ((1<<R_WR) | (1<<R_RD));
	CTRL_PORT  	|= (1<<R_RD);
	CTRL_PORT 	|= (1<<R_WR);

	LED_PORT |= (1<<D_LED0);
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

int main(void)
{

	uint16_t 	fat_cluster = 0;
    uint8_t 	fat_attrib = 0;
    uint32_t 	fat_size = 0;
    uint32_t 	rom_addr = 0;



    uart_init();
    stdout = &uart_stdout;
	
    sram_init();
	printf("sram_init\n");

	spi_init();
	printf("spi_init\n");
/*
	sram_clear(0x000000, 0x400000);
	printf("sram_clear\n");
*/

	//printf("read 0x0f0f\n");
	//sram_read(0x0f0f);
	//printf("write 0x0f0f\n");
	//sram_write(0x0f0f,0xaa);
	//while(1);

	while ( mmc_init() !=0) {
		printf("no sdcard..\n");
    }
    printf("mmc_init\n");

    fat_init(read_buffer);
    printf("fat_init\n");


    rom_addr = 0x000000;
    printf("look for sprite.smc\n");

    if (fat_search_file((uint8_t*)"sprite.smc",
						&fat_cluster,
						&fat_size,
						&fat_attrib,
						read_buffer) == 1) {

        for (uint16_t block_cnt=0; block_cnt<BLOCKS; block_cnt++) {
        	fat_read_file (fat_cluster,read_buffer,block_cnt);
        	printf("Read Block %i addr 0x%06lx\n",block_cnt,rom_addr);
	    	//dump_packet(rom_addr,512,read_buffer);
        	sram_copy(rom_addr,read_buffer,512);
			rom_addr += 512;
        }
	}

    rom_addr = 0x000000;
    for (uint16_t block_cnt=0; block_cnt<BLOCKS; block_cnt++) {
    	sram_read_buffer(rom_addr,read_buffer,512);
		printf("Block %i\n",block_cnt);
    	dump_packet(rom_addr,512,read_buffer);
		rom_addr += 512;
    }


	while(1);

	return(0);
}

