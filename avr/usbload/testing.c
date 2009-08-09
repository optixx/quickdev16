

/*
 * =====================================================================================
 *
 * ________        .__        __    ________               ____  ________
 * \_____  \  __ __|__| ____ |  | __\______ \   _______  _/_   |/  _____/
 *  /  / \  \|  |  \  |/ ___\|  |/ / |    |  \_/ __ \  \/ /|   /   __  \
 * /   \_/.  \  |  /  \  \___|    <  |    `   \  ___/\   / |   \  |__\  \
 * \_____\ \_/____/|__|\___  >__|_ \/_______  /\___  >\_/  |___|\_____  /
 *        \__>             \/     \/        \/     \/                 \/
 *
 *                                  www.optixx.org
 *
 *
 *        Version:  1.0
 *        Created:  07/21/2009 03:32:16 PM
 *         Author:  david@optixx.org
 *
 * =====================================================================================
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <util/delay.h>

#include "shared_memory.h"
#include "config.h"
#include "sram.h"
#include "debug.h"
#include "crc.h"
#include "info.h"

#include "mmc.h"
#include "fat.h"
#include "file.h"





void test_read_write()
{

    uint8_t i;
    uint32_t addr;
    avr_bus_active();
    addr = 0x000000;
    i = 1;
    while (addr++ <= 0x0000ff) {
        sram_write(addr, i++);
    }
    addr = 0x000000;
    while (addr++ <= 0x0000ff) {
        info("read addr=0x%08lx %x\n", addr, sram_read(addr));
    }
}



void test_bulk_read_write()
{

    uint8_t i;
    uint32_t addr;
    avr_bus_active();
    addr = 0x000000;
    i = 0;
    sram_bulk_write_start(addr);
    while (addr++ <= 0x8000) {
        sram_bulk_write(i++);
        sram_bulk_write_next();
    }
    sram_bulk_write_end();

    addr = 0x000000;
    sram_bulk_read_start(addr);
    while (addr <= 0x8000) {
        info("addr=0x%08lx %x\n", addr, sram_bulk_read());
        sram_bulk_read_next();
        addr++;
    }
    sram_bulk_read_end();
}


void test_non_zero_memory(uint32_t bottom_addr, uint32_t top_addr)
{
    uint32_t addr = 0;
    uint8_t c;
    sram_bulk_read_start(bottom_addr);
    for (addr = bottom_addr; addr < top_addr; addr++) {
        c = sram_bulk_read();
        if (c != 0xff)
            info("addr=0x%08lx c=0x%x\n", addr, c);
        sram_bulk_read_next();
    }
    sram_bulk_read_end();
}



void test_crc()
{
    info("test_crc: clear\n");
    avr_bus_active();
    sram_bulk_set(0x000000, 0x10000, 0xff);
    info("test_crc: crc\n");
    crc_check_bulk_memory(0x000000, 0x10000, 0x8000);
    info("test_crc: check\n");
    test_non_zero_memory(0x000000, 0x10000);
}


void test_sdcard(void){


    while (mmc_init() !=0){ 		//ist der Rückgabewert ungleich NULL ist ein Fehler aufgetreten	
	    printf("no sdcard\n");	
    }  
	
 
    if (fat_initfat()==0){	
        printf("fatinit ok\n");
    } else {
        printf("fatinit failed\n");
      return;
    }

    char datei[12]="test.txt";		// hier muss platz für 11 zeichen sein (8.3), da fat_str diesen string benutzt !!
    fat_str(datei);						// wandelt "test.txt" in das fat format 8.3 der form: "TEST    TXT" muss immer dieses Format haben, auch ordner !!

    // 0.) ______________löschen von dateien/ordnern (ordner rekursiv)____________________________________________
    ffrm( datei );								// löschen der datei/ordner falls vorhanden

    // 1.) ______________anlegen und schreiben____________________________________________________________________
    /* 	öffnet datei, wenn nicht vorhanden, legt ffopen datei an (rückgabewert = 1 datei existiert, also nur öffnen, 2 = angelegt). */  
    ffopen( datei );						

    /* schreibt string  */
    ffwrites((char*)"Hallo Datei :)");
    // neue zeile in der datei
    ffwrite(0x0D);
    ffwrite(0x0A);

    /* schließt datei */
    ffclose();

    // 2.)________________ändern von vorhandenen daten in dateien__________________________________________________
    ffopen( datei );		// siehe oben...
    ffseek(12);				// spult in datei auf position 12 vor (fängt immer bei 0 an zu zählen !)
    ffwrite(';');			// schreibt dann ab position 12 (überschreibt daten der datei, hier nur 1 zeichen)
    ffclose();				// schließt datei  

    // 3.)________________lesen von dateien_________________________________________________________________________
    ffopen( datei );							// siehe oben...
    unsigned long int seek=file.length;	// eine variable setzen und runterzählen bis 0 geht am schnellsten !
    do{
  	 printf(ffread());							// liest ein zeichen und gibt es über uart aus !
  	 }while(--seek);							// liest solange bytes da sind (von datei länge bis 0)
    ffclose();									// schließt datei


}

