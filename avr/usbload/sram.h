#include <stdlib.h>
#include <stdint.h> 
#include <avr/io.h>


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
#define CTRL_DIR    DDRB
#define LATCH_PORT	PORTB
#define LATCH_DIR	DDRB

#define SPI_PORT	PORTB
#define SPI_DIR		DDRB

#define LED_PORT	PORTD
#define LED_DIR		DDRD

#define ROMSIZE      4
#define BLOCKS 		(ROMSIZE << 8)
#define MEMSIZE 	0x80000



void spi_init(void);
void spi_master_transmit(unsigned char cData);
void sram_set_addr(uint32_t addr);
uint8_t sram_read(uint32_t addr);
void sram_write(uint32_t addr, uint8_t data);
void sram_init(void);
void sram_snes_mode01(void);
void sram_snes_mode02(void);
void sram_clear(uint32_t addr, uint32_t len);
void sram_copy(uint32_t addr,uint8_t *src, uint32_t len);
void sram_read_buffer(uint32_t addr,uint8_t *dst, uint32_t len);
uint8_t sram_check(uint8_t *buffer, uint32_t len);
