
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "uart.h"
#include "mmc.h"
#include "fat.h"

// Debug
#define debug(x, fmt) printf("%s:%u: %s=" fmt, __FILE__, __LINE__, #x, x)
extern FILE uart_stdout;

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

//#define FILENAME	"sprite.raw" //ok
//#define FILENAME	"ascii.smc"  //ok
//#define FILENAME	"rom.smc"    //ok
//#define FILENAME	"supert.smc" //ok
//#define FILENAME	"vortex.smc" //failed
//#define FILENAME	"mrdo.smc"   //failed
//#define FILENAME	"spacei.smc" //ok ntsc
//#define FILENAME	"bank01.smc" //ok
//#define FILENAME	"bank02.smc" //ok
//#define FILENAME	"bank03.smc" //ok
//#define FILENAME	"bank04.smc" //ok
//#define FILENAME	"bank05.smc" //ok 
//#define FILENAME	"bank06.smc" //ok
//#define FILENAME	"bank07.smc" //ok
//#define FILENAME	"banklo.smc" //ok
//#define FILENAME	"bankhi.smc" //ok
//#define FILENAME	"vram2.smc"  //ok
//#define FILENAME	"super02.smc" //ok
//#define FILENAME	"super01.smc"//ok
#define FILENAME	"crc.smc"    //ok
//#define FILENAME	"banks.smc"  //ok
//#define FILENAME	"hungry.smc" //ok
//#define FILENAME	"arkanoid.smc"//ok  
//#define FILENAME	"eric.smc"  
//#define FILENAME	"super01.smc"  

#define ROMSIZE      2              // 4 == 4mbit == 512kb
                                    // 2 == 2mbit == 256kb
#define DUMPNAME	"dump256.smc"
#define BUFFER_SIZE 512
#define BLOCKS 		(ROMSIZE << 8)
#define MEMSIZE 	0x80000

uint8_t read_buffer[BUFFER_SIZE];


uint16_t crc_xmodem_update (uint16_t crc, uint8_t data)
{
    int i;
    crc = crc ^ ((uint16_t)data << 8);
    for (i=0; i<8; i++)
    {
        if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
    }

    return crc;
}

uint16_t do_crc(uint8_t * data,uint16_t size)
{
	uint16_t crc =0;
	uint16_t i;
	for (i=0; i<size; i++){
		crc = crc_xmodem_update(crc,data[i]);
		//printf("%x : %x\n",crc,data[i]);	
	}
  	return crc;
}


uint16_t do_crc_update(uint16_t crc,uint8_t * data,uint16_t size)
{
  uint16_t i;
  for (i=0; i<size; i++)
	crc = crc_xmodem_update(crc,data[i]);
  return crc;
}


void dump_packet(uint32_t addr,uint32_t len,uint8_t *packet){
	uint16_t i,j;
	uint16_t sum = 0;
	uint8_t clear=0;
	
	for (i=0;i<len;i+=16) {
		
		sum = 0;
		for (j=0;j<16;j++) {
			sum +=packet[i+j];
		}
		if (!sum){
			clear=1;
			continue;
		}
		if (clear){
			printf("*\n");
			clear = 0;
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

    /* deactive RD and WR on ram  */
	CTRL_PORT |= (1<<R_RD);   
	CTRL_PORT |= (1<<R_WR);

    /* setup  address */
	spi_master_transmit((uint8_t)(addr>>16));
	spi_master_transmit((uint8_t)(addr>>8));
	spi_master_transmit((uint8_t)(addr>>0));

	/* passthru address in sreg */
	LATCH_PORT |= (1<<S_LATCH);
	LATCH_PORT &= ~(1<<S_LATCH);

    /* write enable */ 
	CTRL_PORT &= ~(1<<R_WR);

    /* busdriver toggle */


    /* write data */ 
	RAM_PORT = data;
	
    /* disable write */ 
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

	for (uint16_t cnt=0; cnt<len; cnt++) 
		if (buffer[cnt])
			return 1;
	return 0;
}
int main(void)
{

	uint16_t 	fat_cluster = 0;
    uint8_t 	fat_attrib = 0;
    uint32_t 	fat_size = 0;
    uint32_t 	rom_addr = 0;
    uint8_t 	bank_cnt = 0;
 	uint16_t 	crc = 0;
	uint16_t 	block_cnt;
    uart_init();
    stdout = &uart_stdout;
	
    sram_init();
	printf("SRAM Init\n");

	spi_init();
	printf("SPI Init\n");

#if 0                 
    uint8_t t[] = "david";
	printf("Test CRC %x\n",do_crc(t,5));
	while(1);
#endif


#if 0                 
	sram_clear(0x000000, 0x80000);
	printf("sram_clear\n");
#endif

#if 0                 
	printf("read 0x0f0f\n");
	sram_read(0x0f0f);
	printf("write 0x0f0f\n");
	sram_write(0x0f0f,0xaa);
#endif
	
#if 0	
	rom_addr = 0x4aaaa;
	printf("write %lx\n",rom_addr);
	sram_set_addr(rom_addr);
	while(1);
#endif
	
	
	while ( mmc_init() !=0) {
		printf("No sdcard...\n");
    }
    printf("MMC Init done\n");
    fat_init(read_buffer);
    printf("FAT Init done.\n");
    rom_addr = 0x000000;
    printf("Look for %s\n",FILENAME);

    if (fat_search_file((uint8_t*)FILENAME,
						&fat_cluster,
						&fat_size,
						&fat_attrib,
						read_buffer) == 1) {
	   

        for (block_cnt=0; block_cnt<BLOCKS; block_cnt++) {
        	fat_read_file (fat_cluster,read_buffer,block_cnt);
			
			if (block_cnt && block_cnt % 64 == 0){
				printf("Write Ram Bank: 0x%x Addr: 0x%lx Block: %x CRC: %x\n",bank_cnt,rom_addr,block_cnt,crc);
				bank_cnt++;
				crc = 0;
			}
			crc = do_crc_update(crc,read_buffer,512);
			sram_copy(rom_addr,read_buffer,512);
			rom_addr += 512;
        }
		printf("Write Ram Bank: 0x%x Addr: 0x%lx Block: %x CRC: %x\n",bank_cnt,rom_addr,block_cnt,crc);
		printf("Done\n");
	}


    printf("Dump Headern\r");
    rom_addr = 0x8000-512;
    sram_read_buffer(rom_addr,read_buffer,512);
	dump_packet(rom_addr,512,read_buffer);

#if 0
    printf("Dump Memory\n\r");
    rom_addr = 0x000000;
    for (uint16_t block_cnt=0; block_cnt < 64; block_cnt++) {
    	sram_read_buffer(rom_addr,read_buffer,512);
    	dump_packet(rom_addr,512,read_buffer);
		rom_addr += 512;
    }
	printf("\nDone 0x%lx\n",rom_addr);
#endif	

#if 1
	block_cnt = 0;
	crc = 0;
    bank_cnt=0x00;
	rom_addr = 0x000000;
    for (block_cnt=0; block_cnt<BLOCKS; block_cnt++) {
    	sram_read_buffer(rom_addr,read_buffer,512);
		if (block_cnt && block_cnt % 64 == 0){
			printf("Read Ram Bank: 0x%x Addr: 0x%lx Block: %x CRC: %x\n",bank_cnt,rom_addr,block_cnt,crc);
			bank_cnt++;
			crc = 0;
		}
		crc = do_crc_update(crc,read_buffer,512);
		rom_addr += 512;
    }
	printf("Read Ram Bank: 0x%x Addr: 0x%lx Block: %x CRC: %x\n",bank_cnt,rom_addr,block_cnt,crc);
#endif	


#if 0
	
    printf("Look for %s\n",DUMPNAME);

	fat_cluster = 0;
    fat_attrib = 0;
    fat_size = 0;
	
    if (fat_search_file((uint8_t*)DUMPNAME,
						&fat_cluster,
						&fat_size,
						&fat_attrib,
						read_buffer) == 1) {
	   

		printf("Found %s\n",DUMPNAME);
		rom_addr = 0x000000;
		bank_cnt =0;
        for (uint16_t block_cnt=0; block_cnt<BLOCKS; block_cnt++) {
			printf("Write 1");
	    	sram_read_buffer(rom_addr,read_buffer,512);
			printf("Write 2");
			fat_write_file (fat_cluster,read_buffer,block_cnt);
			if (block_cnt % 64 == 0){
				bank_cnt++;
			}
			printf("Write File Bank: 0x%x Addr: 0x%lx Skipped: %li\n",bank_cnt,rom_addr,skip_block);
			rom_addr += 512;
        }
		printf("Done 0x%lx Skipped %li\n",rom_addr,skip_block);
	}
#endif	
	
#if 0
	sram_snes_mode01();
	printf("\nEnter Snes mode 02\n");
#endif	
#if 0
	sram_snes_mode02();
	printf("\nEnter Snes mode 02\n");
#endif	
	
	printf("\nUpload done.\n");
	
	while(1);
	return 0 ;
	
}





