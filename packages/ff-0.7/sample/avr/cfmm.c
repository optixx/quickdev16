/*-----------------------------------------------------------------------*/
/* CFC and MMC combo control module                       (C)ChaN, 2007  */
/*-----------------------------------------------------------------------*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "diskio.h"

#define CFC	0	/* Physical drive number for CompactFlash */
#define MMC	1	/* Physical drive number for MMC/SDSC/SDHC */


/*---------------------------------------------------*/
/* Definitions for CF card */

/* ATA command */
#define CMD_RESET		0x08	/* DEVICE RESET */
#define CMD_READ		0x20	/* READ SECTOR(S) */
#define CMD_WRITE		0x30	/* WRITE SECTOR(S) */
#define CMD_IDENTIFY	0xEC	/* DEVICE IDENTIFY */
#define CMD_SETFEATURES	0xEF	/* SET FEATURES */

/* ATA register bit definitions */
#define	LBA				0xE0
#define	BUSY			0x80
#define	DRDY			0x40
#define	DF				0x20
#define	DRQ				0x08
#define	ERR				0x01
#define	SRST			0x40
#define	nIEN			0x20

/* Contorl Ports */
#define	CTRL_PORT		PORTA
#define	CTRL_DDR		DDRA
#define	CF_SOCK_PORT	PORTC
#define	CF_SOCK_DDR		DDRC
#define	CF_SOCK_PIN		PINC
#define	DAT0_PORT		PORTD
#define	DAT0_DDR		DDRD
#define	DAT0_PIN		PIND
#define	CF_SOCKINS		0x03
#define	CF_SOCKPWR		0x04

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


/*---------------------------------------------------*/
/* Definitions for MMC */

/* MMC/SD command (in SPI) */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define	ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define ACMD13	(0xC0+13)	/* SD_STATUS (SDC) */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */


/* Control signals (Platform dependent) */
#define SELECT()	PORTB &= ~1		/* MMC CS = L */
#define	DESELECT()	PORTB |= 1		/* MMC CS = H */

#define MM_SOCKPORT	PINB			/* Socket contact port */
#define MM_SOCKWP	0x20			/* Write protect switch (PB5) */
#define MM_SOCKINS	0x10			/* Card detect switch (PB4) */

#define	FCLK_SLOW()					/* Set slow clock (100k-400k) */
#define	FCLK_FAST()					/* Set fast clock (depends on the CSD) */




/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/


static volatile
DSTATUS Stat[2] = { STA_NOINIT, STA_NOINIT };	/* Disk status {CFC, MMC}*/

static volatile
BYTE Timer1, Timer2;		/* 100Hz decrement timers */

static
BYTE CardType;




/*-----------------------------------------------------------------------*/
/* Read an ATA register  (Platform dependent)                            */
/*-----------------------------------------------------------------------*/

static
BYTE CF_read_ata (
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
/* Write a byte to an ATA register  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

static
void CF_write_ata (
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
/* Read a part of data block  (Platform dependent)                       */
/*-----------------------------------------------------------------------*/

static
void CF_read_part (
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
		dl = DAT0_PIN;				/* Read Even data */
		CTRL_PORT |= CTL_READ;		/* IORD = H */
		CTRL_PORT &= ~CTL_READ;		/* IORD = L */
		CTRL_PORT &= ~CTL_READ;		/* delay */
		dh = DAT0_PIN;				/* Read Odd data */
		CTRL_PORT |= CTL_READ;		/* IORD = H */
		if (count && (c >= ofs)) {	/* Pick up a part of block */
			*buff++ = dl;
			*buff++ = dh;
			count--;
		}
	} while (++c);
	CF_read_ata(REG_ALTSTAT);
	CF_read_ata(REG_STATUS);
}



/*-----------------------------------------------------------------------*/
/* Wait for Data Ready  (Platform dependent)                             */
/*-----------------------------------------------------------------------*/

static
BOOL CF_wait_data (void)
{
	BYTE s;


	Timer1 = 100;	/* Time out = 1 sec */
	do {
		if (!Timer1) return FALSE;		/* Abort when timeout occured */
		s = CF_read_ata(REG_STATUS);	/* Get status */
	} while ((s & (BUSY|DRQ)) != DRQ);	/* Wait for BUSY goes low and DRQ goes high */

	CF_read_ata(REG_ALTSTAT);
	return TRUE;
}



/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI (Platform dependent)                   */
/*-----------------------------------------------------------------------*/

#define MM_xmit_spi(dat) 	SPDR=(dat); loop_until_bit_is_set(SPSR,SPIF)


/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

static
BYTE MM_rcvr_spi (void)
{
	SPDR = 0xFF;
	loop_until_bit_is_set(SPSR, SPIF);
	return SPDR;
}

/* Alternative macro to receive data fast */
#define MM_rcvr_spi_m(dst)	SPDR=0xFF; loop_until_bit_is_set(SPSR,SPIF); *(dst)=SPDR


/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
BYTE MM_wait_ready (void)
{
	BYTE res;


	Timer2 = 50;	/* Wait for ready in timeout of 500ms */
	MM_rcvr_spi();
	do
		res = MM_rcvr_spi();
	while ((res != 0xFF) && Timer2);

	return res;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void MM_release_spi (void)
{
	DESELECT();
	MM_rcvr_spi();
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
BOOL MM_rcvr_datablock (
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be multiple of 4) */
)
{
	BYTE token;


	Timer1 = 20;
	do {							/* Wait for data packet in timeout of 200ms */
		token = MM_rcvr_spi();
	} while ((token == 0xFF) && Timer1);
	if(token != 0xFE) return FALSE;	/* If not valid data token, retutn with error */

	do {							/* Receive the data block into buffer */
		MM_rcvr_spi_m(buff++);
		MM_rcvr_spi_m(buff++);
		MM_rcvr_spi_m(buff++);
		MM_rcvr_spi_m(buff++);
	} while (btr -= 4);
	MM_rcvr_spi();						/* Discard CRC */
	MM_rcvr_spi();

	return TRUE;					/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static
BOOL MM_xmit_datablock (
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	BYTE resp, wc = 0;


	if (MM_wait_ready() != 0xFF) return FALSE;

	MM_xmit_spi(token);					/* Xmit data token */
	if (token != 0xFD) {	/* Is data token */
		do {							/* Xmit the 512 byte data block to MMC */
			MM_xmit_spi(*buff++);
			MM_xmit_spi(*buff++);
		} while (--wc);
		MM_xmit_spi(0xFF);					/* CRC (Dummy) */
		MM_xmit_spi(0xFF);
		resp = MM_rcvr_spi();			/* Reveive data response */
		if ((resp & 0x1F) != 0x05)		/* If not accepted, return with error */
			return FALSE;
	}

	return TRUE;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE MM_send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = MM_send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	DESELECT();
	SELECT();
	if (MM_wait_ready() != 0xFF) return 0xFF;

	/* Send command packet */
	MM_xmit_spi(cmd);					/* Start + Command index */
	MM_xmit_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
	MM_xmit_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
	MM_xmit_spi((BYTE)(arg >> 8));		/* Argument[15..8] */
	MM_xmit_spi((BYTE)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	MM_xmit_spi(n);

	/* Receive command response */
	if (cmd == CMD12) MM_rcvr_spi();	/* Skip a stuff byte when stop reading */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = MM_rcvr_spi();
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}




/*-----------------------------------------------------------------------*/
/* Power control (Platform dependent)                                    */
/*-----------------------------------------------------------------------*/

static
void MM_power_off (void)
{
	SELECT();
	MM_wait_ready();
	MM_release_spi();

	SPCR = 0;				/* Disable SPI function */
	DDRB  = 0b11000000;		/* Disable drivers */
	PORTB = 0b10110000;
	PORTE |=  0x80;			/* Socket power OFF */
	Stat[1] |= STA_NOINIT;
}


static
void MM_power_on (void)
{
	PORTE &= ~0x80;				/* Socke power (PE7) */
	for (Timer1 = 3; Timer1; );	/* Wait for 30ms */
	PORTB = 0b10110101;			/* Enable drivers */
	DDRB  = 0b11000111;
	SPCR = 0b01010000;			/* Initialize SPI port (Mode 0) */
	SPSR = 0b00000001;
}


static
void CF_power_off()
{
	CF_SOCK_PORT = 0xFF;
	CF_SOCK_DDR = CF_SOCKPWR;
	DAT0_PORT = 0;
	CTRL_DDR = 0;
}


static
void CF_power_on(void)
{
	CF_SOCK_PORT &= ~CF_SOCKPWR;			/* Power ON */
	for (Timer1 = 1; Timer1; );				/* 10ms */
	CTRL_PORT = CTL_READ | CTL_WRITE;		/* Enable control signals */
	CTRL_DDR = 0xFF;
	DAT0_PORT = 0xFF;						/* Pull-up D0-D7 */
	for (Timer1 = 5; Timer1; );				/* 50ms */
	CTRL_PORT |= CTL_RESET;					/* RESET = H */
	for (Timer1 = 5; Timer1; );				/* 50ms */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

static
DSTATUS CF_disk_initialize (void)
{
	Stat[0] |= STA_NOINIT;

	CF_power_off();								/* Power OFF */
	for (Timer1 = 10; Timer1; );				/* 100ms */

	if (Stat[0] & STA_NODISK) return Stat[0];	/* Exit when socket is empty */

	CF_power_on();								/* Power ON */
	CF_write_ata(REG_DEV, LBA);					/* Select Device 0 */
	Timer1 = 200;
	do {										/* Wait for card goes ready */
		if (!Timer1) return Stat[0];
	} while (CF_read_ata(REG_STATUS) & BUSY);

	CF_write_ata(REG_DEVCTRL, SRST | nIEN);		/* Software reset */
	for (Timer1 = 2; Timer1; );					/* 20ms */
	CF_write_ata(REG_DEVCTRL, nIEN);			/* Release software reset */
	for (Timer1 = 2; Timer1; );					/* 20ms */
	Timer1 = 200;
	do {										/* Wait for card goes ready */
		if (!Timer1) return Stat[0];
	} while ((CF_read_ata(REG_STATUS) & (DRDY|BUSY)) != DRDY);

	CF_write_ata(REG_FEATURES, 0x01);			/* Select 8-bit PIO transfer mode */
	CF_write_ata(REG_COMMAND, CMD_SETFEATURES);
	Timer1 = 100;
	do {
		if (!Timer1) return Stat[0];
	} while (CF_read_ata(REG_STATUS) & BUSY);

	Stat[0] &= ~STA_NOINIT;						/* When device goes ready, clear STA_NOINIT */

	return Stat[0];
}


static
DSTATUS MM_disk_initialize (void)
{
	BYTE n, ty, cmd, ocr[4];


	if (Stat[1] & STA_NODISK)			/* No card in the socket */
		return Stat[1];

	MM_power_on();						/* Force socket power ON */
	FCLK_SLOW();
	for (n = 10; n; n--) MM_rcvr_spi();	/* 80 dummy clocks */

	ty = 0;
	if (MM_send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		Timer1 = 100;							/* Initialization timeout of 1000 msec */
		if (MM_send_cmd(CMD8, 0x1AA) == 1) {	/* SDC ver 2.00 */
			for (n = 0; n < 4; n++) ocr[n] = MM_rcvr_spi();
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {	/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && MM_send_cmd(ACMD41, 1UL << 30));	/* ACMD41 with HCS bit */
				if (Timer1 && MM_send_cmd(CMD58, 0) == 0) {	/* Check CCS bit */
					for (n = 0; n < 4; n++) ocr[n] = MM_rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;
				}
			}
		} else {							/* SDC ver 1.XX or MMC */
			if (MM_send_cmd(ACMD41, 0) <= 1) {
				ty = CT_SD1; cmd = ACMD41;	/* SDC ver 1.XX */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMC */
			}
			while (Timer1 && MM_send_cmd(cmd, 0));	/* Wait for leaving idle state */
			if (!Timer1 || MM_send_cmd(CMD16, 512) != 0)	/* Select R/W block length */
				ty = 0;
		}
	}
	CardType = ty;
	MM_release_spi();

	if (ty) {			/* Initialization succeded */
		Stat[1] &= ~STA_NOINIT;		/* Clear STA_NOINIT */
		FCLK_FAST();
	} else {			/* Initialization failed */
		MM_power_off();
	}

	return Stat[1];
}



DSTATUS disk_initialize (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	switch (drv) {
	case CFC :
		return CF_disk_initialize();
	case MMC :
		return MM_disk_initialize();
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber */
)
{
	switch (drv) {
	case CFC :
		return Stat[0];
	case MMC :
		return Stat[1];
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static
DRESULT CF_disk_read (
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector number (LBA) */
	BYTE count		/* Sector count (1..255) */
)
{
	BYTE c, iord_l, iord_h;


	if (!count) return RES_PARERR;
	if (Stat[0] & STA_NOINIT) return RES_NOTRDY;

	/* Issue Read Setor(s) command */
	CF_write_ata(REG_COUNT, count);
	CF_write_ata(REG_SECTOR, (BYTE)sector);
	CF_write_ata(REG_CYLL, (BYTE)(sector >> 8));
	CF_write_ata(REG_CYLH, (BYTE)(sector >> 16));
	CF_write_ata(REG_DEV, ((BYTE)(sector >> 24) & 0x0F) | LBA);
	CF_write_ata(REG_COMMAND, CMD_READ);

	do {
		if (!CF_wait_data()) return RES_ERROR;	/* Wait data ready */
		CTRL_PORT = REG_DATA;
		iord_h = REG_DATA;
		iord_l = REG_DATA & ~CTL_READ;
		c = 0;
		do {
			CTRL_PORT = iord_l;		/* IORD = L */
			CTRL_PORT = iord_l;		/* delay */
			CTRL_PORT = iord_l;		/* delay */
			*buff++ = DAT0_PIN;		/* Get even data */
			CTRL_PORT = iord_h;		/* IORD = H */
			CTRL_PORT = iord_l;		/* IORD = L */
			CTRL_PORT = iord_l;		/* delay */
			CTRL_PORT = iord_l;		/* delay */
			*buff++ = DAT0_PIN;		/* Get Odd data */
			CTRL_PORT = iord_h;		/* IORD = H */
		} while (--c);
	} while (--count);

	CF_read_ata(REG_ALTSTAT);
	CF_read_ata(REG_STATUS);

	return RES_OK;
}



static
DRESULT MM_disk_read (
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (!count) return RES_PARERR;
	if (Stat[1] & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert LBA to byte address if needed */

	if (count == 1) {	/* Single block read */
		if ((MM_send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& MM_rcvr_datablock(buff, 512))
			count = 0;
	}
	else {				/* Multiple block read */
		if (MM_send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!MM_rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			MM_send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	MM_release_spi();

	return count ? RES_ERROR : RES_OK;
}



DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector number (LBA) */
	BYTE count		/* Sector count (1..255) */
)
{
	switch (drv) {
	case CFC :
		return CF_disk_read(buff, sector, count);
	case MMC :
		return MM_disk_read(buff, sector, count);
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static
DRESULT CF_disk_write (
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	BYTE s, c, iowr_l, iowr_h;


	if (!count) return RES_PARERR;
	if (Stat[0] & STA_NOINIT) return RES_NOTRDY;

	/* Issue Write Setor(s) command */
	CF_write_ata(REG_COUNT, count);
	CF_write_ata(REG_SECTOR, (BYTE)sector);
	CF_write_ata(REG_CYLL, (BYTE)(sector >> 8));
	CF_write_ata(REG_CYLH, (BYTE)(sector >> 16));
	CF_write_ata(REG_DEV, ((BYTE)(sector >> 24) & 0x0F) | LBA);
	CF_write_ata(REG_COMMAND, CMD_WRITE);

	do {
		if (!CF_wait_data()) return RES_ERROR;
		CTRL_PORT = REG_DATA;
		iowr_h = REG_DATA;
		iowr_l = REG_DATA & ~CTL_WRITE;
		DAT0_DDR = 0xFF;		/* Set D0-D7 as output */
		c = 0;
		do {
			DAT0_PORT = *buff++;	/* Set even data */
			CTRL_PORT = iowr_l;		/* IOWR = L */
			CTRL_PORT = iowr_h;		/* IOWR = H */
			DAT0_PORT = *buff++;	/* Set odd data */
			CTRL_PORT = iowr_l;		/* IOWR = L */
			CTRL_PORT = iowr_h;		/* IOWR = H */
		} while (--c);
		DAT0_PORT = 0xFF;		/* Set D0-D7 as input */
		DAT0_DDR = 0;
	} while (--count);

	Timer1 = 100;
	do {
		if (!Timer1) return RES_ERROR;
		s = CF_read_ata(REG_STATUS);
	} while (s & BUSY);
	if (s & ERR) return RES_ERROR;

	CF_read_ata(REG_ALTSTAT);
	CF_read_ata(REG_STATUS);

	return RES_OK;
}


static
DRESULT MM_disk_write (
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (!count) return RES_PARERR;
	if (Stat[1] & STA_NOINIT) return RES_NOTRDY;
	if (Stat[1] & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert LBA to byte address if needed */

	if (count == 1) {	/* Single block write */
		if ((MM_send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& MM_xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) {
			MM_send_cmd(CMD55, 0); MM_send_cmd(CMD23, count);	/* ACMD23 */
		}
		if (MM_send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!MM_xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!MM_xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	MM_release_spi();

	return count ? RES_ERROR : RES_OK;
}


DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	switch (drv) {
	case CFC :
		return CF_disk_write(buff, sector, count);
	case MMC :
		return MM_disk_write(buff, sector, count);
	}
	return RES_PARERR;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0
static
DRESULT CF_disk_ioctl (
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	BYTE n, w, ofs, dl, dh, *ptr = buff;


	if (Stat[0] & STA_NOINIT) return RES_NOTRDY;

	switch (ctrl) {
		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			ofs = 60; w = 2; n = 0;
			break;

		case GET_SECTOR_SIZE :	/* Get sectors on the disk (WORD) */
			*(WORD*)buff = 512;
			return RES_OK;

		case GET_BLOCK_SIZE :	/* Get erase block size in sectors (DWORD) */
			*(DWORD*)buff = 32;
			return RES_OK;

		case CTRL_SYNC :		/* Nothing to do */
			return RES_OK;

		case ATA_GET_REV :		/* Get firmware revision (8 chars) */
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

	CF_write_ata(REG_COMMAND, CMD_IDENTIFY);
	if (!CF_wait_data()) return RES_ERROR;
	CF_read_part(ptr, ofs, w);
	while (n--) {
		dl = *ptr; dh = *(ptr+1);
		*ptr++ = dh; *ptr++ = dl; 
	}

	return RES_OK;
}


static
DRESULT MM_disk_ioctl (
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	WORD csize;


	if (Stat[1] & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;
	switch (ctrl) {
	case CTRL_SYNC :	/* Make sure that pending write process has been finished */
		SELECT();
		if (MM_wait_ready() == 0xFF)
			res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
		if ((MM_send_cmd(CMD9, 0) == 0) && MM_rcvr_datablock(csd, 16)) {
			if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
				csize = csd[9] + ((WORD)csd[8] << 8) + 1;
				*(DWORD*)buff = (DWORD)csize << 10;
			} else {					/* MMC or SDC ver 1.XX */
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(DWORD*)buff = (DWORD)csize << (n - 9);
			}
			res = RES_OK;
		}
		break;

	case GET_SECTOR_SIZE :	/* Get sectors on the disk (WORD) */
		*(WORD*)buff = 512;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */
		if (CardType & CT_SD2) {	/* SDC ver 2.00 */
			if (MM_send_cmd(ACMD13, 0) == 0) {	/* Read SD status */
				MM_rcvr_spi();
				if (MM_rcvr_datablock(csd, 16)) {				/* Read partial block */
					for (n = 64 - 16; n; n--) MM_rcvr_spi();	/* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDC ver 1.XX or MMC */
			if ((MM_send_cmd(CMD9, 0) == 0) && MM_rcvr_datablock(csd, 16)) {	/* Read CSD */
				if (CardType & CT_SD1) {	/* SDC ver 1.XX */
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMC */
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
		if (MM_send_cmd(CMD9, 0) == 0		/* READ_CSD */
			&& MM_rcvr_datablock(ptr, 16))
			res = RES_OK;
		break;

	case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
		if (MM_send_cmd(CMD10, 0) == 0		/* READ_CID */
			&& MM_rcvr_datablock(ptr, 16))
			res = RES_OK;
		break;

	case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
		if (MM_send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
			for (n = 0; n < 4; n++)
				*ptr++ = MM_rcvr_spi();
			res = RES_OK;
		}
		break;

	case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
		if (MM_send_cmd(ACMD13, 0) == 0) {		/* SD_STATUS */
			MM_rcvr_spi();
			if (MM_rcvr_datablock(ptr, 64))
				res = RES_OK;
		}
		break;

	default:
		res = RES_PARERR;
	}

	MM_release_spi();

	return res;
}


DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	switch (drv) {
	case CFC :
		return CF_disk_ioctl(ctrl, buff);
	case MMC :
		return MM_disk_ioctl(ctrl, buff);
	}
	return RES_PARERR;
}
#endif /* _USE_IOCTL != 0 */



/*-----------------------------------------------------------------------*/
/* Device timer interrupt procedure                                      */
/*-----------------------------------------------------------------------*/
/* This must be called in period of 10ms                                 */

void disk_timerproc (void)
{
	static BYTE pvc, pvm;
	BYTE n;
	DSTATUS s;


	n = Timer1;						/* 100Hz decrement timer */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;

	/* CF control */
	n = pvc;
	pvc = CF_SOCK_PIN & CF_SOCKINS;	/* Sapmle socket switch */

	if (n == pvc) {					/* Have contacts stabled? */
		if (pvc & CF_SOCKINS) {		/* CD1 or CD2 is high (Socket empty) */
			Stat[0] |= (STA_NODISK | STA_NOINIT);
			DAT0_DDR = 0; DAT0_PORT = 0;			/* Float D0-D7 */
			CTRL_DDR = CTL_RESET; CTRL_PORT = 0;	/* Assert RESET# */
			CF_SOCK_PORT |= CF_SOCKPWR;				/* Power OFF */
		} else {					/* CD1 and CD2 are low (Card inserted) */
			Stat[0] &= ~STA_NODISK;
		}
	}

	/* MMC control */
	n = pvm;
	pvm = MM_SOCKPORT & (MM_SOCKWP | MM_SOCKINS);	/* Sample socket switch */

	if (n == pvm) {					/* Have contacts stabled? */
		s = Stat[1];

		if (pvm & MM_SOCKWP)		/* WP is H (write protected) */
			s |= STA_PROTECT;
		else						/* WP is L (write enabled) */
			s &= ~STA_PROTECT;

		if (pvm & MM_SOCKINS)		/* INS = H (Socket empty) */
			s |= (STA_NODISK | STA_NOINIT);
		else						/* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat[1] = s;
	}

}

