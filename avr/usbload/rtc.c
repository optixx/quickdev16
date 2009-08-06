/*--------------------------------------------------------------------------*/
/*  RTC controls                                                            */

#include <avr/io.h>
#include "rtc.h"




BOOL rtc_gettime (RTC *rtc)
{
	BYTE buf[8];


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
	return 1;
}


