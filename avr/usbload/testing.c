

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
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>      
#include <util/delay.h>         
#include <avr/pgmspace.h>       
#include <avr/eeprom.h>

#include "usbdrv.h"
#include "oddebug.h"            
#include "config.h"
#include "requests.h"           
#include "uart.h"
#include "sram.h"
#include "debug.h"
#include "info.h"
#include "dump.h"
#include "crc.h"
#include "usb_bulk.h"
#include "timer.h"
#include "watchdog.h"
#include "rle.h"
#include "loader.h"
#include "command.h"
#include "shared_memory.h"
#include "testing.h"

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






/*----------------------------------------------------------------------*/
/* FAT file system sample project for FatFs R0.06  (C)ChaN, 2008        */
/*----------------------------------------------------------------------*/

#include "ff.h"
#include "diskio.h"
#include "rtc.h"



DWORD acc_size;				/* Work register for fs command */
WORD acc_files, acc_dirs;
FILINFO finfo;


FATFS fatfs[2];				/* File system object for each logical drive */
BYTE Buff[1024];			/* Working buffer */

volatile WORD Timer;		/* 100Hz increment timer */



#if _MULTI_PARTITION != 0
    const PARTITION Drives[] = { {0,0}, {0,1} };
#endif

/*
ISR(TIMER2_COMP_vect)
{
	Timer++;			
	disk_timerproc();	
}
*/

DWORD get_fattime ()
{
	RTC rtc;


	//rtc_gettime(&rtc);

	return	  ((DWORD)(rtc.year - 1980) << 25)
			| ((DWORD)rtc.month << 21)
			| ((DWORD)rtc.mday << 16)
			| ((DWORD)rtc.hour << 11)
			| ((DWORD)rtc.min << 5)
			| ((DWORD)rtc.sec >> 1);
}



static
FRESULT scan_files (char* path)
{
	DIR dirs;
	FRESULT res;
	int i;

	if ((res = f_opendir(&dirs, path)) == FR_OK) {
		i = strlen(path);
		while (((res = f_readdir(&dirs, &finfo)) == FR_OK) && finfo.fname[0]) {
			if (finfo.fattrib & AM_DIR) {
				acc_dirs++;
				*(path+i) = '/'; strcpy(path+i+1, &finfo.fname[0]);
				res = scan_files(path);
				*(path+i) = '\0';
				if (res != FR_OK) break;
			} else {
				acc_files++;
				acc_size += finfo.fsize;
			}
		}
	}

	return res;
}


static
void put_rc (FRESULT rc)
{
	const prog_char *p;
	static const prog_char str[] =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0";
	FRESULT i;

	for (p = str, i = 0; i != rc && pgm_read_byte_near(p); i++) {
		while(pgm_read_byte_near(p++));
	}
	printf("rc=%u FR_%s\n", (WORD)rc, p);
}




void test_sdcard (void)
{
	char *ptr, *ptr2;
	DWORD p1, p2, p3;
	BYTE res, b1;
	WORD w1;
	UINT s1, s2, cnt;
	DWORD ofs, sect = 0;
	RTC rtc;
	FATFS *fs;
	DIR dir;				/* Directory object */
	FIL file1, file2;		/* File object */


    printf("Try to init disk\n");
    put_rc(f_mount((BYTE) 0, &fatfs[0]));
    res = f_getfree("", &p2, &fs);
    if (res)
        put_rc(res);

    printf(     "FAT TYPE = %u\nBYTES/CLUSTER = %lu\nNUMBER OF FATS = %u\n"
                "ROOT DIR ENTRIES = %u\nSECTORS/FAT = %lu\nNUMBER OF CLUSTERS = %lu\n"
                "FAT START = %lu\nDIR START LBA,CLUSTER = %lu\nDATA START LBA = %lu\n",
                (WORD) fs->fs_type, (DWORD) fs->csize * 512,
                (WORD) fs->n_fats, fs->n_rootdir, (DWORD) fs->sects_fat,
                (DWORD) fs->max_clust - 2, fs->fatbase, fs->dirbase, fs->database);
    acc_size = acc_files = acc_dirs = 0;

    printf("scan files\n");
    res = scan_files("");
    if (res)
        put_rc(res);
    printf("%u FILES, %lu BYTES\n%u FOLDERS\n"
                "%lu KB TOTAK DISK SPACE\n%lu KB AVAILABLE\n", acc_files,
                acc_size, acc_dirs, (fs->max_clust - 2) * (fs->csize / 2),
                p2 * (fs->csize / 2));

}


