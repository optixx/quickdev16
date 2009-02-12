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
	uint8_t read, buf[10], i=0;
 	uint8_t Buffer[512];
	uint16_t Clustervar;	

        uint8_t Dir_Attrib = 0;
        uint32_t Size = 0;	


	DDRD=0x00;
	PORTD=0x00;
	
	DDRB |= ((1<<R_WR) | (1<<R_RD));    	
	PORTB |= (1<<R_RD);
	PORTB |= (1<<R_WR);

	DDRC |= (1<<D_LED0);

	uart_init();
	
	SPI_MasterInit();
	uart_puts("\n\r\n\rSPI_init!\n\r");

	SRAM_Write(0x00000000,0x23);
	SRAM_Write(0x00000001,0x42);
	SRAM_Write(0x00000003,0xee);


	//Initialisierung der MMC/SD-Karte ANFANG:
	while ( mmc_init() !=0) //ist der Rückgabewert ungleich NULL ist ein Fehler aufgetreten
        {
        	uart_puts("** Keine MMC/SD Karte gefunden!! **\n\r");
        }
        uart_puts("Karte gefunden!!\n\r");

	fat_init(Buffer);       //laden Cluster OFFSET und Size

        //Initialisierung der MMC/SD-Karte ENDE!
        
	mmc_read_csd (Buffer);

        for (uint16_t tmp = 0;tmp<16;tmp++)
        {
                itoa(Buffer[tmp],buf,16);
                uart_puts(buf);
        }
	
	Clustervar = 0;//suche im Root Verzeichnis
        if (fat_search_file((unsigned char *)"rom.txt",&Clustervar,&Size,&Dir_Attrib,Buffer) == 1)
                {
		uart_puts("\r\n\r\n");
                uart_puts("rom.txt:print and write to RAM:\r\n");
                //Lese File und gibt es auf der seriellen Schnittstelle aus und schreibt es ins RAM
                for (uint8_t b = 0;b<1;b++)
		{
                	fat_read_file (Clustervar,Buffer,b);
                	for (uint8_t a = 0;a<50;a++)
                        {
				SRAM_Write(0x00023420+a,Buffer[a]);
                                uart_putc(Buffer[a]);
                        }
               	}
	}
	
	for (uint8_t b=0;b<1;b++)
	{
		for(uint8_t a=0; a<50;a++)
		{
			uart_puts("\r\nRead RAM Addr 0x");
			ltoa(0x00023420+a,buf,16);			
			uart_puts(buf);
			uart_puts(": ");
			uart_putc(SRAM_Read(0x00023420+a));
		}
	}

	uart_puts("\r\n\r\n");
	while(1){
		i++;

		uart_puts("\r\nDump RAM Addr 0x00000-0x00004: ");

		read = SRAM_Read(0x00000000);
		itoa(read,buf,16);
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

		if(i==2){
			uart_puts("\r\nWriting 0xBE,0xEF to 0x00002-0x00003..");
			SRAM_Write(0x00000002,0xBE);
			SRAM_Write(0x00000003,0xEF);
		}
		if(i==3){
			while(1);
		}

	}

	return(0);
}

