/*-----------------------------------------------------------------------*/
/* ATA control module                                     (C)ChaN, 2007  */
/*-----------------------------------------------------------------------*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "diskio.h"


/* ATA command */
#define CMD_RESET		0x08	/* DEVICE RESET */
#define CMD_READ		0x20	/* READ SECTOR(S) */
#define CMD_WRITE		0x30	/* WRITE SECTOR(S) */
#define CMD_IDENTIFY	0xEC	/* DEVICE IDENTIFY */
#define CMD_SETFEATURES	0xEF	/* SET FEATURES */

/* ATA register bit definitions */
#define	LBA				0x40
#define	BSY				0x80
#define	DRDY			0x40
#define	DF				0x20
#define	DRQ				0x08
#define	ERR				0x01
#define	SRST			0x40
#define	nIEN			0x20

/* Contorl Ports */
#define	CTRL_PORT		PORTA
#define	CTRL_DDR		DDRA
#define	DAT1_PORT		PORTC
#define	DAT1_DDR		DDRC
#define	DAT1_PIN		PINC
#define	DAT0_PORT		PORTD
#define	DAT0_DDR		DDRD
#define	DAT0_PIN		PIND

/* Bit definitions for Control Port */
#define	CTL_READ		0x20
#define	CTL_WRITE		0x40
#define	CTL_RESET		0x80
#define	REG_DATA		0xF0
#define	REG_ERROR		0xF1
#define	REG_FEATURES	0xF1
#define	REG_COUNT		0xF2
#define	REG_SECTOR		0xF3
#define	REG_CYLL		0xF4
#define	REG_CYLH		0xF5
#define	REG_DEV			0xF6
#define	REG_COMMAND		0xF7
#define	REG_STATUS		0xF7
#define	REG_DEVCTRL		0xEE
#define	REG_ALTSTAT		0xEE




/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/


static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
WORD Timer;			/* 100Hz decrement timer */



/*-----------------------------------------------------------------------*/
/* Read an ATA register                                                  */
/*-----------------------------------------------------------------------*/

static
BYTE read_ata (
	BYTE reg			/* Register to be read */
)
{
	BYTE rd;


	CTRL_PORT = reg;
	CTRL_PORT &= ~CTL_READ;
	CTRL_PORT &= ~CTL_READ;
	CTRL_PORT &= ~CTL_READ;
	rd = DAT0_PIN;
	CTRL_PORT |= CTL_READ;
	return rd;
}



/*-----------------------------------------------------------------------*/
/* Write a byte to an ATA register                                       */
/*-----------------------------------------------------------------------*/

static
void write_ata (
	BYTE reg,		/* Register to be written */
	BYTE dat		/* Data to be written */
)
{
	CTRL_PORT = reg;
	DAT0_PORT = dat;
	DAT0_DDR = 0xFF;
	CTRL_PORT &= ~CTL_WRITE;
	CTRL_PORT &= ~CTL_WRITE;
	CTRL_PORT |= CTL_WRITE;
	DAT0_PORT = 0xFF;
	DAT0_DDR = 0;
}



/*-----------------------------------------------------------------------*/
/* Read a part of data block                                             */
/*-----------------------------------------------------------------------*/

static
void read_part (
	BYTE *buff, 	/* Data buffer to store read data */
	BYTE ofs,		/* Offset of the part of data in unit of word */
	BYTE count		/* Number of word to pick up */
)
{
	BYTE c = 0, dl, dh;


	CTRL_PORT = REG_DATA;		/* Select Data register */
	do {
		CTRL_PORT &= ~CTL_READ;		/* IORD = L */
		CTRL_PORT &= ~CTL_READ;		/* delay */
		dl = DAT0_PIN;				/* Read even data */
		dh = DAT1_PIN;				/* Read odd data */
		CTRL_PORT |= CTL_READ;		/* IORD = H */
		if (count && (c >= ofs)) {	/* Pick up a part of block */
			*buff++ = dl;
			*buff++ = dh;
			count--;
		}
	} while (++c);
	read_ata(REG_ALTSTAT);
	read_ata(REG_STATUS);
}



/*-----------------------------------------------------------------------*/
/* Wait for Data Ready                                                   */
/*-----------------------------------------------------------------------*/

static
BOOL wait_data (void)
{
	WORD w;
	BYTE s;


	cli(); Timer = 1000; sei();	/* Time out = 10 sec */
	do {
		cli(); w = Timer; sei();
		if (!w) return FALSE;			/* Abort when timeout occured */
		s = read_ata(REG_STATUS);		/* Get status */
	} while ((s & (BSY|DRQ)) != DRQ);	/* Wait for BSY goes low and DRQ goes high */

	read_ata(REG_ALTSTAT);
	return TRUE;
}




/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	WORD w;


	if (drv) return STA_NOINIT;		/* Supports only single drive */
	Stat |= STA_NOINIT;

	/* Initialize the ATA control port */
	DAT0_PORT = 0xFF;
	DAT1_PORT = 0xFF;
	DAT0_DDR = 0;
	DAT1_DDR = 0;
	CTRL_PORT = CTL_READ | CTL_WRITE;		/* Assert RESET */
	CTRL_DDR = 0xFF;

	for (Timer = 2; Timer; );				/* 20ms */
	CTRL_PORT |= CTL_RESET;					/* Deassert RESET */
	for (Timer = 1; Timer; );				/* 10ms */
	write_ata(REG_DEV, 0);					/* Select Device 0 */

	cli(); Timer = 1000; sei();
	do {
		cli(); w = Timer; sei();
		if (!w) return Stat;
	} while (!(read_ata(REG_STATUS) & (BSY | DRQ)));

	write_ata(REG_DEVCTRL, SRST | nIEN);	/* Software reset */
	for (Timer = 2; Timer; );
	write_ata(REG_DEVCTRL, nIEN);
	for (Timer = 2; Timer; );

	cli(); Timer = 1000; sei();
	do {
		cli(); w = Timer; sei();
		if (!w) return Stat;
	} while ((read_ata(REG_STATUS) & (DRDY|BSY)) != DRDY);

	write_ata(REG_FEATURES, 3);				/* Select PIO default mode without IORDY */
	write_ata(REG_COUNT, 1);
	write_ata(REG_COMMAND, CMD_SETFEATURES);
	Timer = 100;
	do {
		if (!Timer) return Stat;
	} while (read_ata(REG_STATUS) & BSY);


	Stat &= ~STA_NOINIT;		/* When device goes ready, clear STA_NOINIT */

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector number (LBA) */
	BYTE count		/* Sector count (1..255) */
)
{
	BYTE c, iord_l, iord_h;


	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	/* Issue Read Setor(s) command */
	write_ata(REG_COUNT, count);
	write_ata(REG_SECTOR, (BYTE)sector);
	write_ata(REG_CYLL, (BYTE)(sector >> 8));
	write_ata(REG_CYLH, (BYTE)(sector >> 16));
	write_ata(REG_DEV, ((BYTE)(sector >> 24) & 0x0F) | LBA);
	write_ata(REG_COMMAND, CMD_READ);

	do {
		if (!wait_data()) return RES_ERROR;	/* Wait data ready */
		CTRL_PORT = REG_DATA;
		iord_h = REG_DATA;
		iord_l = REG_DATA & ~CTL_READ;
		c = 128;
		do {
			CTRL_PORT = iord_l;		/* IORD = L */
			CTRL_PORT = iord_l;		/* delay */
			*buff++ = DAT0_PIN;		/* Get even data */
			*buff++ = DAT1_PIN;		/* Get odd data */
			CTRL_PORT = iord_h;		/* IORD = H */
			CTRL_PORT = iord_l;		/* IORD = L */
			CTRL_PORT = iord_l;		/* delay */
			*buff++ = DAT0_PIN;		/* Get even data */
			*buff++ = DAT1_PIN;		/* Get odd data */
			CTRL_PORT = iord_h;		/* IORD = H */
		} while (--c);
	} while (--count);

	read_ata(REG_ALTSTAT);
	read_ata(REG_STATUS);

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	BYTE s, c, iowr_l, iowr_h;


	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	/* Issue Write Setor(s) command */
	write_ata(REG_COUNT, count);
	write_ata(REG_SECTOR, (BYTE)sector);
	write_ata(REG_CYLL, (BYTE)(sector >> 8));
	write_ata(REG_CYLH, (BYTE)(sector >> 16));
	write_ata(REG_DEV, ((BYTE)(sector >> 24) & 0x0F) | LBA);
	write_ata(REG_COMMAND, CMD_WRITE);

	do {
		if (!wait_data()) return RES_ERROR;
		CTRL_PORT = REG_DATA;
		iowr_h = REG_DATA;
		iowr_l = REG_DATA & ~CTL_WRITE;
		DAT0_DDR = 0xFF;	/* Set D0-D15 as output */
		DAT1_DDR = 0xFF;
		c = 128;
		do {
			DAT0_PORT = *buff++;	/* Set even data */
			DAT1_PORT = *buff++;	/* Set odd data */
			CTRL_PORT = iowr_l;		/* IOWR = L */
			CTRL_PORT = iowr_h;		/* IOWR = H */
			DAT0_PORT = *buff++;	/* Set even data */
			DAT1_PORT = *buff++;	/* Set odd data */
			CTRL_PORT = iowr_l;		/* IOWR = L */
			CTRL_PORT = iowr_h;		/* IOWR = H */
		} while (--c);
		DAT0_PORT = 0xFF;	/* Set D0-D15 as input */
		DAT1_PORT = 0xFF;
		DAT0_DDR = 0;
		DAT1_DDR = 0;
	} while (--count);

	Timer = 100;
	do {
		if (!Timer) return RES_ERROR;
		s = read_ata(REG_STATUS);
	} while (s & BSY);
	if (s & ERR) return RES_ERROR;

	read_ata(REG_ALTSTAT);
	read_ata(REG_STATUS);

	return RES_OK;
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	BYTE n, dl, dh, ofs, w, *ptr = buff;


	if (drv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	switch (ctrl) {
		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			ofs = 60; w = 2; n = 0;
			break;

		case GET_SECTOR_SIZE :	/* Get sectors on the disk (WORD) */
			*(WORD*)buff = 512;
			return RES_OK;

		case GET_BLOCK_SIZE :	/* Get erase block size in sectors (DWORD) */
			*(DWORD*)buff = 1;
			return RES_OK;

		case CTRL_SYNC :	/* Nothing to do */
			return RES_OK;

		case ATA_GET_REV :	/* Get firmware revision (8 chars) */
			ofs = 23; w = 4; n = 4;
			break;

		case ATA_GET_MODEL :	/* Get model name (40 chars) */
			ofs = 27; w = 20; n = 20;
			break;

		case ATA_GET_SN :	/* Get serial number (20 chars) */
			ofs = 10; w = 10; n = 10;
			break;

		default:
			return RES_PARERR;
	}

	write_ata(REG_COMMAND, CMD_IDENTIFY);
	if (!wait_data()) return RES_ERROR;
	read_part(ptr, ofs, w);
	while (n--) {
		dl = *ptr; dh = *(ptr+1);
		*ptr++ = dh; *ptr++ = dl; 
	}

	return RES_OK;
}
#endif /*  _USE_IOCTL != 0 */



/*-----------------------------------------------------------------------*/
/* Device timer interrupt procedure                                      */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms */

void disk_timerproc (void)
{
	WORD n;


	n = Timer;					/* 100Hz decrement timer */
	if (n) Timer = --n;
}

