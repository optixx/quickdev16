
#include "data.h";
#include "pad.h";
#include "event.h";
#include "myEvents.h";
#include "ressource.h";
#include "PPU.h"
#include "debug.h"
#include "integer.h"
#include "ff.h"


#include <time.h>
#include <stdlib.h>
#include <string.h>

padStatus pad1;

DWORD acc_size;             /* Work register for fs command */
WORD acc_files, acc_dirs;

FILINFO finfo;
FATFS fatfs[2];             /* File system object for each logical drive */
BYTE Buff[512];            /* Working buffer */

DWORD p1, p2, p3;
BYTE res;
WORD w1;
UINT s1, s2, cnt;

FATFS *fs;
DIR dir;                /* Directory object */
FIL file1, file2;       /* File object */



void initInternalRegisters(void) {
	characterLocation[0] = 0x0000;
	characterLocation[1] = 0x0000;
	characterLocation[2] = 0x0000;
	characterLocation[3] = 0x0000;
	debug_init();
}

void preInit(void) {
	// For testing purpose ... 
	// Insert code here to be executed before register init
}

DWORD get_fattime ()
{
  time_t rawtime;
  struct tm * ptm;
  //time ( &rawtime );
  ptm = gmtime ( &rawtime );

  return      ((DWORD)(ptm->tm_year - 80) << 25)
            | ((DWORD)(ptm->tm_mon +1) << 21)
            | ((DWORD)ptm->tm_mday << 16)
            | ((DWORD)ptm->tm_hour << 11)
            | ((DWORD)ptm->tm_min << 5)
            | ((DWORD)ptm->tm_sec >> 1);
}



void put_rc (FRESULT rc){
    const char *p;
    static const char str[] =
        "OK\0" "NOT_READY\0" "NO_FILE\0" "FR_NO_PATH\0" "INVALID_NAME\0" "INVALID_DRIVE\0"
        "DENIED\0" "EXIST\0" "RW_ERROR\0" "WRITE_PROTECTED\0" "NOT_ENABLED\0"
        "NO_FILESYSTEM\0" "INVALID_OBJECT\0" "MKFS_ABORTED\0";
    FRESULT i;

    for (p = str, i = 0; i != rc && *p; i++) {
        while(*p++);
    }
    printfc("rc=%u FR_%s\n", (WORD)rc, p);
}


FRESULT scan_files (char* path){
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

void main(void) {
	word i,j;
	word crc01;
	word crc02;
	padStatus pad1;
	char line_header[32] = "OK2";
	initInternalRegisters();

	*(byte*) 0x2105 = 0x01;	// MODE 1 value
	*(byte*) 0x212c = 0x01; // Plane 0 (bit one) enable register
	*(byte*) 0x212d = 0x00;	// All subPlane disable
	*(byte*) 0x2100 = 0x0f; // enable background

    debug_enable();
    printfs(0,"FATFS ");

    printfc("Try to init disk\n");
    put_rc(f_mount(0, &fatfs[p1]));

    res = f_getfree("/", &p2, &fs);
    if (res)
        put_rc(res);

    printfc("FAT type = %u\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
            "Root DIR entries = %u\nSectors/FAT = %lu\nNumber of clusters = %lu\n"
            "FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\nData start (lba) = %lu\n",
            (WORD)fs->fs_type, (DWORD)fs->csize * 512, (WORD)fs->n_fats,
            fs->n_rootdir, (DWORD)fs->sects_fat, (DWORD)fs->max_clust - 2,
            fs->fatbase, fs->dirbase, fs->database
    );
    acc_size = acc_files = acc_dirs = 0;
    res = scan_files("/");
    if (res)
        put_rc(res);
    printfc("%u files, %lu bytes.\n%u folders.\n"
           "%lu KB total disk space.\n%lu KB available.\n",
           acc_files, acc_size, acc_dirs,
           (fs->max_clust - 2) * (fs->csize / 2), p2 * (fs->csize / 2)
    );

  
	while(1){
		while(!pad1.start) {
			waitForVBlank();
			pad1 = readPad((byte) 0);
		}
	}	
	while(1);
}

void IRQHandler(void) {
}

void NMIHandler(void) {
	//processEvents();
}
