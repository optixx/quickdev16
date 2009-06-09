#include "ff.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data.h";
#include "pad.h";
#include "event.h";
#include "myEvents.h";
#include "ressource.h";
#include "PPU.h"
#include "debug.h"
#include "crc.h"

/*

o relocate main code
o exec loaded file

o debug STA global
o optimize internal transfer buffer
o direct writeto mempage


*/


#define ROM_NAME        "MRDO.SMC"
#define BLOCK_SIZE      512
#define BANK_SIZE       32768L
#define BANK_COUNT      8
#define BASE_ADDR       0x008000

padStatus pad1;
DWORD acc_size;                 /* Work register for fs command */
WORD acc_files, acc_dirs;
FILINFO finfo;
FATFS fatfs[2];                 /* File system object for each logical * drive */
//BYTE Buff[512];                 /* Working buffer */
DWORD p1, p2, p3;
DWORD addr;
DWORD crc_addr;
UINT crc;
BYTE res, bank;
WORD w1;
UINT s1, s2, cnt;
FATFS *fs;
DIR dir;                        /* Directory object */
FIL file1, file2;
void initInternalRegisters(void)
{
    characterLocation[0] = 0x0000;
    characterLocation[1] = 0x0000;
    characterLocation[2] = 0x0000;
    characterLocation[3] = 0x0000;
    debug_init();
} void preInit(void)
{

    // For testing purpose ... 
    // Insert code here to be executed before register init
} DWORD get_fattime()
{
    time_t rawtime;
    struct tm *ptm;

    // time ( &rawtime );
    ptm = gmtime(&rawtime);
    return ((DWORD) (ptm->tm_year - 80) << 25)
        | ((DWORD) (ptm->tm_mon + 1) << 21)
        | ((DWORD) ptm->tm_mday << 16)
        | ((DWORD) ptm->tm_hour << 11)
        | ((DWORD) ptm->tm_min << 5) | ((DWORD) ptm->tm_sec >> 1);
}

void halt(void)
{
    while (1);
}

void put_rc(FRESULT rc)
{
    const char *p;
    static const char str[] =
        "OK\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0" "INVALID_NAME\0"
        "INVALID_DRIVE\0" "DENIED\0" "EXIST\0" "RW_ERROR\0"
        "WRITE_PROTECTED\0" "NOT_ENABLED\0" "NO_FILESYSTEM\0"
        "INVALID_OBJECT\0" "MKFS_ABORTED\0";
    FRESULT i;
    for (p = str, i = 0; i != rc && *p; i++) {
        while (*p++);
    }
    printfc("rc=%u FR_%s\n", (WORD) rc, p);
}

FRESULT scan_files(char *path)
{
    DIR dirs;
    FRESULT res;
    int i;
    if ((res = f_opendir(&dirs, path)) == FR_OK) {
        i = strlen(path);
        while (((res = f_readdir(&dirs, &finfo)) == FR_OK)
               && finfo.fname[0]) {
            if (finfo.fattrib & AM_DIR) {
                acc_dirs++;
                *(path + i) = '/';
                strcpy(path + i + 1, &finfo.fname[0]);
                res = scan_files(path);
                *(path + i) = '\0';
                if (res != FR_OK)
                    break;
            } else {
                acc_files++;
                acc_size += finfo.fsize;
            }
        }
    }
    return res;
}

void wait(void)
{
    printfc("SNES::wait: press A to continue\n");
    enablePad();
    pad1 = readPad((byte) 0);
    while (!pad1.A) {
        waitForVBlank();
        pad1 = readPad((byte) 0);
    }
    printfc("SNES::wait: done\n");
}

void boot(void)
{

#asm
    jsl $008000
#endasm
} void main(void)
{
    word i, j;
    BYTE res;
    initInternalRegisters();
    *(byte *) 0x2105 = 0x01;    // MODE 1 value
    *(byte *) 0x212c = 0x01;    // Plane 0 (bit one) enable register
    *(byte *) 0x212d = 0x00;    // All subPlane disable
    *(byte *) 0x2100 = 0x0f;    // enable background

    debug_enable();
    printfc("SNES::main: Try to init disk\n");
    put_rc(f_mount((BYTE)0, &fatfs[0]));

#if 0
    printfs(0, "FATFS OPTIXX.ORG ");
    printfc("SNES::main: Try to get free\n");
    res = f_getfree("", &p2, &fs);
    if (res)
        put_rc(res);
    printfc("SNES::main: printf fs results\n");
    printfs(1,
            "FAT TYPE = %u\nBYTES/CLUSTER = %lu\nNUMBER OF FATS = %u\n"
            "ROOT DIR ENTRIES = %u\nSECTORS/FAT = %lu\nNUMBER OF CLUSTERS = %lu\n"
            "FAT START = %lu\nDIR START LBA,CLUSTER = %lu\nDATA START LBA = %lu\n",
            (WORD) fs->fs_type, (DWORD) fs->csize * 512,
            (WORD) fs->n_fats, fs->n_rootdir, (DWORD) fs->sects_fat,
            (DWORD) fs->max_clust - 2, fs->fatbase, fs->dirbase, fs->database);
    acc_size = acc_files = acc_dirs = 0;
    printfc("SNES::main: scan files\n");
    res = scan_files("");
    if (res)
        put_rc(res);
    printfs(12, "%u FILES, %lu BYTES\n%u FOLDERS\n"
            "%lu KB TOTAK DISK SPACE\n%lu KB AVAILABLE\n", acc_files,
            acc_size, acc_dirs, (fs->max_clust - 2) * (fs->csize / 2),
            p2 * (fs->csize / 2));
    res = f_opendir(&dir, "");
    if (res)
        put_rc(res);
    p1 = s1 = s2 = 0;
    cnt = 0;
    wait();
    clears();
    printfc("SNES::main: read dir\n");
    for (;;) {
        res = f_readdir(&dir, &finfo);
        if ((res != FR_OK) || !finfo.fname[0])
            break;
        if (finfo.fattrib & AM_DIR) {
            s2++;
        } else {
            s1++;
            p1 += finfo.fsize;
        }
        ;
        printfs(cnt, "%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu\n%s\n",
                (finfo.fattrib & AM_DIR) ? 'D' : '-',
                (finfo.fattrib & AM_RDO) ? 'R' : '-',
                (finfo.fattrib & AM_HID) ? 'H' : '-',
                (finfo.fattrib & AM_SYS) ? 'S' : '-',
                (finfo.fattrib & AM_ARC) ? 'A' : '-',
                (finfo.fdate >> 9) + 1980, (finfo.fdate >> 5) & 15,
                finfo.fdate & 31, (finfo.ftime >> 11),
                (finfo.ftime >> 5) & 63, finfo.fsize, &(finfo.fname[0]));
        cnt += 2;

        /*
         *  printfs(cnt,"%u/%02u/%02u %02u:%02u %9lu\n%s\n", (finfo.fdate >> 9) + 1980, (finfo.fdate >> 5) & 15, finfo.fdate & 31,
         * (finfo.ftime >> 11), (finfo.ftime >> 5) & 63, finfo.fsize, &(finfo.fname[0])); 
         */
        if (cnt && cnt == 20) {
            cnt = 0;
            wait();
            clears();
        }
    }
    printfs(20, "%4u FILES\n%10lu BYTES TOTAL\n%4u DIRS", s1, p1, s2);
    if (f_getfree("", &p1, &fs) == FR_OK)
        printfs(23, "%10luK BYTES FREE\n", p1 * fs->csize / 2);
    wait();
    clears();

#endif                          /* */

    printfc("SNES::main: open %s \n", ROM_NAME);
    printfs(0, "OPEN %s", ROM_NAME);
    put_rc(f_open(&file1, ROM_NAME, (BYTE)FA_READ));
    p1 = BANK_SIZE * BANK_COUNT;
    p2 = 0;
    p3 = 0;
    cnt = 0;
    bank = 0;
    addr = BASE_ADDR;
    crc_addr = BASE_ADDR;
    while (p1) {
        cnt = BLOCK_SIZE;
        p1 -= BLOCK_SIZE;
        res = f_read(&file1, (byte *) (addr), cnt, &s2);
        printfc("SNES::main: read cnt=%i p1=%li p2=%li s2=%i\n", cnt,
                p1, p2, s2);


       if (res != FR_OK) {
            printfc("SNES::main: read failed\n");
            put_rc(res);
            break;
        }
        p2 += s2;
        if (cnt != s2) {
            printfc("SNES::main: read cnt=%i s2=%i\n", cnt, s2);
            break;
        }
        printfs(1 + bank, "CRC 0000 BANK %X  ADDR %lX",bank, addr);

#if 0
        printc_packet(addr, 512, (byte *) (addr));
        wait();
#endif
        
        addr += s2;
        if (addr % 0x10000 == 0) {
            crc = crc_update_mem(crc_addr,0x8000);
            printfc("addr=%lx crc=%x\n",crc_addr,crc);
            printfs(1 + bank, "CRC %X BANK %X  ADDR %LX",crc, bank, addr);
            addr += 0x8000;
            crc_addr+=0x8000;
            bank++;
        }
    }
    put_rc(f_close(&file1));
    boot();
    while (1) {
        wait();
    }
    while (1);
}

void IRQHandler(void)
{
} void NMIHandler(void)
{

    // processEvents();
}
