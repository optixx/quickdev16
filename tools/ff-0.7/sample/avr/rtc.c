/*--------------------------------------------------------------------------*/
/*  RTC controls                                                            */

#include <avr/io.h>
#include <string.h>
#include "rtc.h"



#define SCL_LOW()	DDRE |=	0x04			/* SCL = LOW */
#define SCL_HIGH()	DDRE &=	0xFB			/* SCL = High-Z */
#define	SCL_VAL		((PINE & 0x04) ? 1 : 0)	/* SCL input level */
#define SDA_LOW()	DDRE |=	0x08			/* SDA = LOW */
#define SDA_HIGH()	DDRE &=	0xF7			/* SDA = High-Z */
#define	SDA_VAL		((PINE & 0x08) ? 1 : 0)	/* SDA input level */



static
void iic_delay (void)
{
	int n;
	BYTE d;

	for (n = 4; n; n--) d = PINE;
}


/* Generate start condition on the IIC bus */
static
void iic_start (void)
{
	SDA_HIGH();
	iic_delay();
	SCL_HIGH();
	iic_delay();
	SDA_LOW();
	iic_delay();
	SCL_LOW();
	iic_delay();
}


/* Generate stop condition on the IIC bus */
static
void iic_stop (void)
{
	SDA_LOW();
	iic_delay();
	SCL_HIGH();
	iic_delay();
	SDA_HIGH();
	iic_delay();
}


/* Send a byte to the IIC bus */
static
BOOL iic_send (BYTE dat)
{
	BYTE b = 0x80;
	BOOL ack;


	do {
		if (dat & b)	 {	/* SDA = Z/L */
			SDA_HIGH();
		} else {
			SDA_LOW();
		}
		iic_delay();
		SCL_HIGH();
		iic_delay();
		SCL_LOW();
		iic_delay();
	} while (b >>= 1);
	SDA_HIGH();
	iic_delay();
	SCL_HIGH();
	ack = SDA_VAL ? FALSE : TRUE;	/* Sample ACK */
	iic_delay();
	SCL_LOW();
	iic_delay();
	return ack;
}


/* Receive a byte from the IIC bus */
static
BYTE iic_rcvr (BOOL ack)
{
	UINT d = 1;


	do {
		d <<= 1;
		SCL_HIGH();
		if (SDA_VAL) d++;
		iic_delay();
		SCL_LOW();
		iic_delay();
	} while (d < 0x100);
	if (ack) {		/* SDA = ACK */
		SDA_LOW();
	} else {
		SDA_HIGH();
	}
	iic_delay();
	SCL_HIGH();
	iic_delay();
	SCL_LOW();
	SDA_HIGH();
	iic_delay();

	return (BYTE)d;
}




BOOL rtc_read (
	UINT adr,		/* Read start address */
	UINT cnt,		/* Read byte count */
	void* buff		/* Read data buffer */
)
{
	BYTE *rbuff = buff;
	int n;


	if (!cnt) return FALSE;

	n = 10;
	do {							/* Select DS1338 (0xD0) */
		iic_start();
	} while (!iic_send(0xD0) && --n);
	if (!n) return FALSE;

	if (iic_send((BYTE)adr)) {		/* Set start address */
		iic_start();				/* Reselect DS1338 in read mode (0xD1) */
		if (iic_send(0xD1)) {
			do {					/* Receive data */
				cnt--;
				*rbuff++ = iic_rcvr(cnt ? TRUE : FALSE);
			} while (cnt);
		}
	}

	iic_stop();						/* Deselect device */

	return cnt ? FALSE : TRUE;
}




BOOL rtc_write (
	UINT adr,			/* Write start address */
	UINT cnt,			/* Write byte count */
	const void* buff	/* Data to be written */
)
{
	const BYTE *wbuff = buff;
	int n;


	if (!cnt) return FALSE;

	n = 10;
	do {							/* Select DS1338 (0xD0) */
		iic_start();
	} while (!iic_send(0xD0) && --n);
	if (!n) return FALSE;

	if (iic_send((BYTE)adr)) {		/* Set start address */
		do {						/* Send data */
			if (!iic_send(*wbuff++)) break;
		} while (--cnt);
	}

	iic_stop();						/* Deselect device */

	return cnt ? FALSE : TRUE;
}




BOOL rtc_gettime (RTC *rtc)
{
	BYTE buf[8];


	if (!rtc_read(0, 7, buf)) return FALSE;

	rtc->sec = (buf[0] & 0x0F) + ((buf[0] >> 4) & 7) * 10;
	rtc->min = (buf[1] & 0x0F) + (buf[1] >> 4) * 10;
	rtc->hour = (buf[2] & 0x0F) + ((buf[2] >> 4) & 3) * 10;
	rtc->wday = (buf[2] & 0x07);
	rtc->mday = (buf[4] & 0x0F) + ((buf[4] >> 4) & 3) * 10;
	rtc->month = (buf[5] & 0x0F) + ((buf[5] >> 4) & 1) * 10;
	rtc->year = 2000 + (buf[6] & 0x0F) + (buf[6] >> 4) * 10;

	return TRUE;
}




BOOL rtc_settime (const RTC *rtc)
{

	BYTE buf[8];


	buf[0] = rtc->sec / 10 * 16 + rtc->sec % 10;
	buf[1] = rtc->min / 10 * 16 + rtc->min % 10;
	buf[2] = rtc->hour / 10 * 16 + rtc->hour % 10;
	buf[3] = rtc->wday & 7;
	buf[4] = rtc->mday / 10 * 16 + rtc->mday % 10;
	buf[5] = rtc->month / 10 * 16 + rtc->month % 10;
	buf[6] = (rtc->year - 2000) / 10 * 16 + (rtc->year - 2000) % 10;
	return rtc_write(0, 7, buf);
}




BOOL rtc_init (void)
{
	BYTE buf[8];	/* RTC R/W buffer */
	UINT adr;


	/* Read RTC registers */
	if (!rtc_read(0, 8, buf)) return FALSE;	/* IIC error */

	if (buf[7] & 0x20) {	/* When data has been volatiled, set default time */
		/* Clear nv-ram. Reg[8..63] */
		memset(buf, 0, 8);
		for (adr = 8; adr < 64; adr += 8)
			rtc_write(adr, 8, buf);
		/* Reset time to Jan 1, '08. Reg[0..7] */
		buf[4] = 1; buf[5] = 1; buf[6] = 8;
		rtc_write(0, 8, buf);
	}
	return TRUE;
}

