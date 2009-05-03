#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <stdlib.h>
#include <avr/pgmspace.h>   /* required by usbdrv.h */

#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */
#include "uart.h"
#include "sram.h"
#include "debug.h"
#include "crc.h"


#define STATE_IDLE      0
#define STATE_UPLOAD    1
#define BUFFER_SIZE 256

extern FILE uart_stdout;

uint32_t    rom_addr;
uint32_t    addr;
uint8_t     bytes_remaining = 0;
uint16_t    sync_errors = 0;
uint8_t     read_buffer[BUFFER_SIZE];
uint8_t     dataBuffer[4];  /* buffer must stay valid when usbFunctionSetup returns */
uint8_t     state = STATE_IDLE;
uint8_t     bank;  /* buffer must stay valid when usbFunctionSetup returns */

usbMsgLen_t usbFunctionSetup(uchar data[8]){
    
    usbRequest_t    *rq = (void *)data;
 	uint16_t crc = 0;
    uint8_t len = 0;
    if(rq->bRequest == USB_UPLOAD_INIT){
        printf("USB_UPLOAD_INIT: reset values\n");
        bank=0;
        bytes_remaining=0;
        crc=0;
        sync_errors=0;
    }else if(rq->bRequest == USB_UPLOAD_ADDR){ /* echo -- used for reliability tests */
        state = STATE_UPLOAD;
        rom_addr = rq->wValue.word;
        rom_addr = rom_addr << 16;
        rom_addr = rom_addr | rq->wIndex.word;
        if (bytes_remaining){
            sync_errors++;
            printf("USB_UPLOAD_ADDR: Out of sync Addr=0x%lx remain=%i packet=%i sync_error=%i\n",rom_addr,bytes_remaining,rq->wLength.word,sync_errors );  
            len=0;
        }    
        bytes_remaining  = rq->wLength.word;
        len = 0xff;
        if (rom_addr && rom_addr%32768 == 0){
		    printf("USB_UPLOAD_ADDR: Bank: 0x%x Addr: 0x%08lx \n",bank,rom_addr);
            bank++;
		}
        len=0xff;
    }else if(rq->bRequest == USB_DOWNLOAD_INIT){
        printf("USB_DOWNLOAD_INIT\n");
    }else if(rq->bRequest == USB_DOWNLOAD_ADDR){
        printf("USB_DOWNLOAD_ADDR\n");
    }else if(rq->bRequest ==USB_CRC_CHECK){
        rom_addr = rq->wValue.word;
        rom_addr = rom_addr << 16;
        rom_addr = rom_addr | rq->wIndex.word;
        bank = 0;
       	crc = 0;
        printf("USB_CRC_CHECK: Addr 0x%lx \n", rom_addr);  
        cli();
        for (addr=0x000000; addr<rom_addr; addr+=BUFFER_SIZE) {
        	sram_read_buffer(addr,read_buffer,BUFFER_SIZE);
    		crc = do_crc_update(crc,read_buffer,BUFFER_SIZE);
    		if (addr && addr%32768 == 0){
    		    printf("USB_CRC_CHECK: Bank: 0x%x Addr: 0x%lx CRC: %x\n",bank,addr,crc);
                bank++;
    			crc = 0;
    		}
        }
        sei();
        
    }
    usbMsgPtr = dataBuffer;
    return len;   /* default for not implemented requests: return no data back to host */
}


uint8_t usbFunctionWrite(uint8_t *data, uint8_t len)
{
    if (len > bytes_remaining){
         printf("usbFunctionWrite more data than expected remain: %i len: %i\n",bytes_remaining,len);
         len = bytes_remaining;
     }
     if (state==STATE_UPLOAD){
    
        bytes_remaining -= len;
        #if 0    
        printf("Addr: 0x%08lx Len: %i\n",rom_addr,len);
        #endif    
        cli();
        sram_copy(rom_addr,data,len);
        sei();
        rom_addr +=len;
    }
    return len;
}

uint8_t usbFunctionRead(uint8_t *data, uint8_t len)
{
    if(len > bytes_remaining)
        len = bytes_remaining;
    bytes_remaining -= len;

    for (uint8_t i = 0; i < len; i++) {
        if(request == USBASP_FUNC_READEEPROM)
            *data = eeprom_read_byte((void *)flash_address.word);
        else
            *data = pgm_read_byte_near((void *)flash_address.word);
        data++;
        flash_address.word++;
    }

    /* flash led on activity */
    DLED_TGL;

    return len;
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    uint8_t i;
    wdt_enable(WDTO_1S);
    uart_init();
    stdout = &uart_stdout;
    sram_init();
 	printf("SRAM Init\n");
 	spi_init();
 	printf("SPI Init\n");
    usbInit();
 	printf("USB Init\n");
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
 	printf("USB disconnect\n");
    i = 10;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
 	printf("USB connect\n");
    sei();
 	printf("USB poll\n");
    for(;;){                /* main event loop */
        wdt_reset();
        usbPoll();
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
