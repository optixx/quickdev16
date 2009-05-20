/*----------------------------------------------------------------------*/
/* FAT file system sample project for FatFs R0.06  (C)ChaN, 2008        */
/*----------------------------------------------------------------------*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "diskio.h"
#include "ff.h"



DWORD acc_size;				/* Work register for fs command */
WORD acc_files, acc_dirs;

FILINFO finfo;

BYTE line[120];				/* Console input buffer */
FATFS fatfs[2];				/* File system object for each logical drive */
BYTE Buff[1024];			/* Working buffer */

volatile WORD Timer;		/* 100Hz increment timer */



#if _MULTI_PARTITION != 0
const PARTITION Drives[] = { {0,0}, {0,1} };
#endif



char xatoi(char **str, long *ret){
  *ret = atoi(*str);
  //printf("'%s' '%li'\n",*str,*ret);
}


/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support a real time clock.          */
/* This is not required in read-only configuration.        */


DWORD get_fattime ()
{
  time_t rawtime;
  struct tm * ptm;
  time ( &rawtime );
  ptm = gmtime ( &rawtime );
	return	  ((DWORD)(ptm->tm_year - 80) << 25)
			| ((DWORD)(ptm->tm_mon +1) << 21)
			| ((DWORD)ptm->tm_mday << 16)
			| ((DWORD)ptm->tm_hour << 11)
			| ((DWORD)ptm->tm_min << 5)
			| ((DWORD)ptm->tm_sec >> 1);
}


/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */


static
void put_dump (const BYTE *buff, LONG ofs, BYTE cnt)
{
	BYTE n;


	printf("%08lX ", ofs);
	for(n = 0; n < cnt; n++)
		printf(" %02X", buff[n]);
	printf(" ");
	for(n = 0; n < cnt; n++) {
		if ((buff[n] < 0x20)||(buff[n] >= 0x7F))
			printf(".");
		else
			printf("%c",buff[n]);
	}
	printf("\n");
}


static
void get_line (char *buff, int len)
{
	char c;
	int idx = 0;

	for (;;) {
		c = getc(stdin);
		if (c == 0x0a) break;
		if ((c == '\b') && idx) {
			idx--;
		}
		if (((BYTE)c >= ' ') && (idx < len - 1)) {
				buff[idx++] = c;
		}
	}
    printf("return %s\n",buff);
	buff[idx] = 0;
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
    const char* str[] = {
		"OK",
        "DISK_ERR",
        "INT_ERR",
        "NOT_READY",
        "NO_FILE",
        "NO_PATH",
		"INVALID_NAME",
        "DENIED",
        "EXIST",
        "INVALID_OBJECT",
        "WRITE_PROTECTED",
		"INVALID_DRIVE",
        "NOT_ENABLED",
        "NO_FILE_SYSTEM",
        "MKFS_ABORTED",
        "TIMEOUT"
  };
  printf("rc=%u FR_%s\n", (WORD)rc, str[rc]);
}




static
void IoInit ()
{

}


/*-----------------------------------------------------------------------*/
/* Main                                                                  */

int main (void)
{
	char *ptr, *ptr2;
	DWORD p1, p2, p3;
	BYTE res, b1;
	WORD w1;
	UINT s1, s2, cnt;
	DWORD ofs, sect = 0;
    time_t rawtime;
    struct tm * ptm;

	FATFS *fs;
	DIR dir;				/* Directory object */
	FIL file1, file2;		/* File object */


	IoInit();


	printf("FatFs module test monitor\n");

	for (;;) {
		printf(">");
		get_line(line, sizeof(line));
		ptr = line;
		switch (*ptr++) {
		case 'd' :
			switch (*ptr++) {
			case 'd' :	/* dd <phy_drv#> [<sector>] - Dump secrtor */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) p2 = sect;
				res = disk_read((BYTE)p1, Buff, p2, 1);
				if (res) { printf("rc=%d\n", (WORD)res); break; }
				sect = p2 + 1;
				printf("Sector:%lu\n", p2);
				for (ptr=Buff, ofs = 0; ofs < 0x200; ptr+=16, ofs+=16)
					put_dump(ptr, ofs, 16);
				break;

			case 'i' :	/* di <phy_drv#> - Initialize disk */
				if (!xatoi(&ptr, &p1)) break;
				printf("rc=%d\n", (WORD)disk_initialize((BYTE)p1));
				break;

			case 's' :	/* ds <phy_drv#> - Show disk status */
				if (!xatoi(&ptr, &p1)) break;
				if (disk_ioctl((BYTE)p1, GET_SECTOR_COUNT, &p2) == RES_OK)
					{ printf("Drive size: %lu sectors\n", p2); }
				if (disk_ioctl((BYTE)p1, GET_SECTOR_SIZE, &w1) == RES_OK)
					{ printf("Sector size: %u\n", w1); }
				if (disk_ioctl((BYTE)p1, GET_BLOCK_SIZE, &p2) == RES_OK)
					{ printf("Erase block size: %lu sectors\n", p2); }
				if (disk_ioctl((BYTE)p1, MMC_GET_TYPE, &b1) == RES_OK)
					{ printf("Card type: %u\n", b1); }
				if (disk_ioctl((BYTE)p1, MMC_GET_CSD, Buff) == RES_OK)
					{ printf("CSD:\n"); put_dump(Buff, 0, 16); }
				if (disk_ioctl((BYTE)p1, MMC_GET_CID, Buff) == RES_OK)
					{ printf("CID:\n"); put_dump(Buff, 0, 16); }
				if (disk_ioctl((BYTE)p1, MMC_GET_OCR, Buff) == RES_OK)
					{ printf("OCR:\n"); put_dump(Buff, 0, 4); }
				if (disk_ioctl((BYTE)p1, MMC_GET_SDSTAT, Buff) == RES_OK) {
					printf("SD Status:\n");
					for (s1 = 0; s1 < 64; s1 += 16) put_dump(Buff+s1, s1, 16);
				}
				if (disk_ioctl((BYTE)p1, ATA_GET_MODEL, line) == RES_OK)
					{ line[40] = '\0'; printf("Model: %s\n", line); }
				if (disk_ioctl((BYTE)p1, ATA_GET_SN, line) == RES_OK)
					{ line[20] = '\0'; printf("S/N: %s\n", line); }
				break;
			}
			break;

		case 'b' :
			switch (*ptr++) {
			case 'd' :	/* bd <addr> - Dump R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				for (ptr=&Buff[p1], ofs = p1, cnt = 32; cnt; cnt--, ptr+=16, ofs+=16)
					put_dump(ptr, ofs, 16);
				break;

			case 'e' :	/* be <addr> [<data>] ... - Edit R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				if (xatoi(&ptr, &p2)) {
					do {
						Buff[p1++] = (BYTE)p2;
					} while (xatoi(&ptr, &p2));
					break;
				}
				for (;;) {
					printf("%04X %02X-", (WORD)(p1), (WORD)Buff[p1]);
					get_line(line, sizeof(line));
					ptr = line;
					if (*ptr == '.') break;
					if (*ptr < ' ') { p1++; continue; }
					if (xatoi(&ptr, &p2))
						Buff[p1++] = (BYTE)p2;
					else
						printf("???\n");
				}
				break;

			case 'r' :	/* br <phy_drv#> <sector> [<n>] - Read disk into R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) break;
				if (!xatoi(&ptr, &p3)) p3 = 1;
				printf("rc=%u\n", (WORD)disk_read((BYTE)p1, Buff, p2, p3));
				break;

			case 'w' :	/* bw <phy_drv#> <sector> [<n>] - Write R/W buffer into disk */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) break;
				if (!xatoi(&ptr, &p3)) p3 = 1;
				printf("rc=%u\n", (WORD)disk_write((BYTE)p1, Buff, p2, p3));
				break;

			case 'f' :	/* bf <n> - Fill working buffer */
				if (!xatoi(&ptr, &p1)) break;
				memset(Buff, (BYTE)p1, sizeof(Buff));
				break;

			}
			break;

		case 'f' :
			switch (*ptr++) {

			case 'i' :	/* fi <log drv#> - Initialize logical drive */
				if (!xatoi(&ptr, &p1)) break;
				put_rc(f_mount((BYTE)p1, &fatfs[p1]));
				break;

			case 's' :	/* fs [<path>] - Show logical drive status */
				while (*ptr == ' ') ptr++;
				res = f_getfree(ptr, &p2, &fs);
				if (res) { put_rc(res); break; }
				printf("FAT type = %u\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
							 "Root DIR entries = %u\nSectors/FAT = %lu\nNumber of clusters = %lu\n"
							 "FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\nData start (lba) = %lu\n",
						(WORD)fs->fs_type, (DWORD)fs->csize * 512, (WORD)fs->n_fats,
						fs->n_rootdir, (DWORD)fs->sects_fat, (DWORD)fs->max_clust - 2,
						fs->fatbase, fs->dirbase, fs->database
				);
				acc_size = acc_files = acc_dirs = 0;
				res = scan_files(ptr);
				if (res) { put_rc(res); break; }
				printf("%u files, %lu bytes.\n%u folders.\n"
							 "%lu KB total disk space.\n%lu KB available.\n",
						acc_files, acc_size, acc_dirs,
						(fs->max_clust - 2) * (fs->csize / 2), p2 * (fs->csize / 2)
				);
				break;

			case 'l' :	/* fl [<path>] - Directory listing */
				while (*ptr == ' ') ptr++;
				res = f_opendir(&dir, ptr);
				if (res) { put_rc(res); break; }
				p1 = s1 = s2 = 0;
				for(;;) {
					res = f_readdir(&dir, &finfo);
					if ((res != FR_OK) || !finfo.fname[0]) break;
					if (finfo.fattrib & AM_DIR) {
						s2++;
					} else {
						s1++; p1 += finfo.fsize;
					}
					printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s\n",
								(finfo.fattrib & AM_DIR) ? 'D' : '-',
								(finfo.fattrib & AM_RDO) ? 'R' : '-',
								(finfo.fattrib & AM_HID) ? 'H' : '-',
								(finfo.fattrib & AM_SYS) ? 'S' : '-',
								(finfo.fattrib & AM_ARC) ? 'A' : '-',
								(finfo.fdate >> 9) + 1980, (finfo.fdate >> 5) & 15, finfo.fdate & 31,
								(finfo.ftime >> 11), (finfo.ftime >> 5) & 63,
								finfo.fsize, &(finfo.fname[0]));
				}
				printf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
				if (f_getfree(ptr, &p1, &fs) == FR_OK)
					printf(", %10luK bytes free\n", p1 * fs->csize / 2);
				break;

			case 'o' :	/* fo <mode> <name> - Open a file */
				if (!(&ptr, &p1)) break;
				while (*ptr == ' ') ptr++;
				put_rc(f_open(&file1, ptr, (BYTE)p1));
				break;

			case 'c' :	/* fc - Close a file */
				put_rc(f_close(&file1));
				break;

			case 'e' :	/* fe - Seek file pointer */
				if (!xatoi(&ptr, &p1)) break;
				res = f_lseek(&file1, p1);
				put_rc(res);
				if (res == FR_OK)
					printf("fptr = %lu(0x%lX)\n", file1.fptr, file1.fptr);
				break;

			case 'r' :	/* fr <len> - read file */
				if (!xatoi(&ptr, &p1)) break;
				p2 = 0;
				Timer = 0;
				while (p1) {
					if (p1 >= sizeof(Buff))	{ cnt = sizeof(Buff); p1 -= sizeof(Buff); }
					else 			{ cnt = (WORD)p1; p1 = 0; }
					res = f_read(&file1, Buff, cnt, &s2);
					if (res != FR_OK) { put_rc(res); break; }
					p2 += s2;
					if (cnt != s2) break;
				}
				s2 = Timer;
				printf("%lu bytes read with %lu bytes/sec.\n", p2, p2 * 100 / s2);
				break;

			case 'd' :	/* fd <len> - read and dump file from current fp */
				if (!xatoi(&ptr, &p1)) break;
				ofs = file1.fptr;
				while (p1) {
					if (p1 >= 16)	{ cnt = 16; p1 -= 16; }
					else 			{ cnt = (WORD)p1; p1 = 0; }
					res = f_read(&file1, Buff, cnt, &cnt);
					if (res != FR_OK) { put_rc(res); break; }
					if (!cnt) break;
					put_dump(Buff, ofs, cnt);
					ofs += 16;
				}
				break;

			case 'w' :	/* fw <len> <val> - write file */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				memset(Buff, (BYTE)p2, sizeof(Buff));
				p2 = 0;
				Timer = 0;
				while (p1) {
					if (p1 >= sizeof(Buff))	{ cnt = sizeof(Buff); p1 -= sizeof(Buff); }
					else 			{ cnt = (WORD)p1; p1 = 0; }
					res = f_write(&file1, Buff, cnt, &s2);
					if (res != FR_OK) { put_rc(res); break; }
					p2 += s2;
					if (cnt != s2) break;
				}
				s2 = Timer;
				printf("%lu bytes written with %lu bytes/sec.\n", p2, p2 * 100 / s2);
				break;

			case 'v' :	/* fv - Truncate file */
				put_rc(f_truncate(&file1));
				break;

			case 'n' :	/* fn <old_name> <new_name> - Change file/dir name */
				while (*ptr == ' ') ptr++;
				ptr2 = strchr(ptr, ' ');
				if (!ptr2) break;
				*ptr2++ = 0;
				while (*ptr2 == ' ') ptr2++;
				put_rc(f_rename(ptr, ptr2));
				break;

			case 'u' :	/* fu <name> - Unlink a file or dir */
				while (*ptr == ' ') ptr++;
				put_rc(f_unlink(ptr));
				break;

			case 'k' :	/* fk <name> - Create a directory */
				while (*ptr == ' ') ptr++;
				put_rc(f_mkdir(ptr));
				break;

			case 'a' :	/* fa <atrr> <mask> <name> - Change file/dir attribute */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				while (*ptr == ' ') ptr++;
				put_rc(f_chmod(ptr, p1, p2));
				break;

			case 't' :	/* ft <year> <month> <day> <hour> <min> <sec> <name> */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				finfo.fdate = ((p1 - 1980) << 9) | ((p2 & 15) << 5) | (p3 & 31);
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				finfo.ftime = ((p1 & 31) << 11) | ((p1 & 63) << 5) | ((p1 >> 1) & 31);
				put_rc(f_utime(ptr, &finfo));
				break;

			case 'x' : /* fx <src_name> <dst_name> - Copy file */
				while (*ptr == ' ') ptr++;
				ptr2 = strchr(ptr, ' ');
				if (!ptr2) break;
				*ptr2++ = 0;
				printf("Opening \"%s\"", ptr);
				res = f_open(&file1, ptr, FA_OPEN_EXISTING | FA_READ);
				if (res) {
					put_rc(res);
					break;
				}
				printf("\nCreating \"%s\"", ptr2);
				res = f_open(&file2, ptr2, FA_CREATE_ALWAYS | FA_WRITE);
				if (res) {
					put_rc(res);
					f_close(&file1);
					break;
				}
				printf("\nCopying...");
				p1 = 0;
				for (;;) {
					res = f_read(&file1, Buff, sizeof(Buff), &s1);
					if (res || s1 == 0) break;   /* error or eof */
					res = f_write(&file2, Buff, s1, &s2);
					p1 += s2;
					if (res || s2 < s1) break;   /* error or disk full */
				}
				if (res) put_rc(res);
				printf("\n%lu bytes copied.\n", p1);
				f_close(&file1);
				f_close(&file2);
				break;
#if _USE_MKFS
			case 'm' :	/* fm <logi drv#> <part type> <bytes/clust> - Create file system */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				printf("The drive %u will be formatted. Are you sure? (Y/n)=", (WORD)p1);
				get_line(ptr, sizeof(line));
				if (*ptr == 'Y') put_rc(f_mkfs((BYTE)p1, (BYTE)p2, (WORD)p3));
				break;
#endif
			}
			break;

		case 't' :	/* t [<year> <mon> <mday> <hour> <min> <sec>] */
			if (xatoi(&ptr, &p1)) {
                time ( &rawtime );
                ptm = gmtime ( &rawtime );
                ptm->tm_year = (WORD)p1;
				xatoi(&ptr, &p1); ptm->tm_mon = (BYTE)p1;
				xatoi(&ptr, &p1); ptm->tm_mday = (BYTE)p1;
				xatoi(&ptr, &p1); ptm->tm_hour = (BYTE)p1;
				xatoi(&ptr, &p1); ptm->tm_min = (BYTE)p1;
				if (!xatoi(&ptr, &p1)) break;
				ptm->tm_sec = (BYTE)p1;
				//rtc_settime(&rtc);
			}
            time ( &rawtime );
            ptm = gmtime ( &rawtime );
			printf("%u/%u/%u %02u:%02u:%02u\n", 1900 + ptm->tm_year, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
			break;
		}
	}

}

