/******************************************************************************
 
 efsl Demo-Application for Philips LPC2138 ARM7TDMI-S
 
 Copyright (c) 2005
 Martin Thomas, Kaiserslautern, Germany <mthomas@rhrk.uni-kl.de>
 
 *****************************************************************************/

#include <string.h>

#include "lpc214x.h"
#include "lpc_config.h"

#include "uart.h"

#include "startup.h"
#include "efs.h"
#include "ls.h"
#include "mkfs.h"
#include "types.h"
#include "debug.h"
#include "interfaces/efsl_dbg_printf_arm.h"

#define rprintf efsl_debug_printf_arm

#define BAUD 115200

#define BIT(x) ((unsigned long)1<<x)

#define LED1PIN  	10
#define LED1BIT     BIT(LED1PIN)
#define LEDDIR      IODIR0
#define LEDSET      IOSET0
#define LEDCLR      IOCLR0
static char rom_filename[] = "SPRITE.SMC";

static void gpioInit(void)
{
    LEDSET = BIT(LED1PIN);      // set Bit = LED off (active low)
    LEDDIR |= BIT(LED1PIN);     // define LED-Pin as output
} static void ledToggle(void)
{
    static unsigned char state = 0;
    state = !state;
    if (state)
        LEDCLR = BIT(LED1PIN);  // set Bit = LED on
    else
        LEDSET = BIT(LED1PIN);  // set Bit = LED off (active low)
}

EmbeddedFileSystem efs;
EmbeddedFile filer, filew;
DirList list;
unsigned short e;
unsigned char buf[513];

void cleanup_name(uint8_t * filename,uint8_t *sfn){
	
	while(*filename != '\0');
	
	
}

void list_roms(){
	uint8_t cnt = 0;	
    rprintf("Directory of 'root':\n");
    ls_openDir(&list, &(efs.myFs), "/");
	while (ls_getNext(&list) == 0) {
		cnt++;
        list.currentEntry.FileName[LIST_MAXLENFILENAME - 1] = '\0';
        rprintf("[%li] %s ( %li bytes )\n",cnt, list.currentEntry.FileName, list.currentEntry.FileSize);
	}	
}

uint8_t * get_filename(uint8_t idx){
	uint8_t cnt = 0;	
    if (idx<1 || idx>9)
		return NULL;
	ls_openDir(&list, &(efs.myFs), "/");
	while (ls_getNext(&list) == 0) {
		cnt++;
        //list.currentEntry.FileName[LIST_MAXLENFILENAME - 1] = '\0';
        if (cnt==idx)
			return list.currentEntry.FileName;
	}
	return NULL;	
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
		DBG((TXT("%08x:"), addr + i));
		for (j=0;j<16;j++) {
			DBG((TXT(" %02x"), packet[i+j]));
		}
		DBG((TXT(" |")));
		for (j=0;j<16;j++) {
			if (packet[i+j]>=33 && packet[i+j]<=126 )
				DBG((TXT("%c"), packet[i+j]));
			else
				DBG((TXT(".")));
		}
		DBG((TXT("|\n")));
		
	}
}

void dump_filename(uint8_t * filename){
	uint32_t cnt = 0;
    if (file_fopen(&filer, &efs.myFs, filename, 'r') == 0) {
        rprintf("File %s open. Content:\n", filename);
        while ((e = file_read(&filer, 512, buf)) != 0) {
			dump_packet(cnt,e,buf);
			cnt+=e;
        } 
		DBG((TXT("Len %08x(%li)\n"), cnt,cnt));
		rprintf("\n");
        file_fclose(&filer);
	} else {
		rprintf("Failed to open %s\n",filename);
	}
}




int main(void)
{
    int ch;
    int8_t res;
    uint8_t * filename;
    uint8_t fatfilename[12];

    Initialize();
    gpioInit();
    uart0Init(UART_BAUD(BAUD), UART_8N1, UART_FIFO_8);  // setup the UART
    uart0Puts("\r\nMMC/SD Card Filesystem Test (P:LPC2148 L:EFSL)\r\n");

    /* init efsl debug-output */
    efsl_debug_devopen_arm(uart0Putch);
    ledToggle();
    rprintf("CARD init...");
    if ((res = efs_init(&efs, 0)) != 0) {
        rprintf("failed with %i\n", res);
    }
	list_roms(&efs);
	rprintf("Select File:\n");
    while (1) {
        if ((ch = uart0Getch()) >= 0) {
            uart0Puts("You pressed : ");
            uart0Putch(ch);
            uart0Puts("\r\n");
			if (ch >='1' && ch <='9'){
				
				filename = get_filename(ch - 48);
				file_normalToFatName(filename,fatfilename);
				rprintf("Filename: '%s'\n",filename);
				dump_filename(filename);
				rprintf("Fatfilename: '%s'\n",fatfilename);
				dump_filename(fatfilename);
				file_normalToFatName("sprite.smc",fatfilename);
				rprintf("Fatfilename: '%s'\n",fatfilename);
				dump_filename(fatfilename);
				file_normalToFatName("sprite .smc",fatfilename);
				rprintf("Fatfilename: '%s'\n",fatfilename);
				dump_filename(fatfilename);
				dump_filename(rom_filename);
			}
			  
            ledToggle();
        }
    }
    return 0;                   /* never reached */
}
