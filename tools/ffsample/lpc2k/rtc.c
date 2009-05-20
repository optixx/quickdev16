/*--------------------------------------------------------------------------*/
/*  RTC controls                                                            */

#include "LPC2300.h"
#include "rtc.h"



BOOL rtc_gettime (RTC *rtc)
{
	DWORD d, t;


	do {
		t = RTC_CTIME0;
		d = RTC_CTIME1;
	} while (t != RTC_CTIME0 || d != RTC_CTIME1);

	rtc->sec = t & 63;
	rtc->min = (t >> 8) & 63;
	rtc->hour = (t >> 16) & 31;
	rtc->wday = (t >> 24) & 7;
	rtc->mday = d & 31;
	rtc->month = (d >> 8) & 15;
	rtc->year = (d >> 16) & 4095;

	return TRUE;
}




BOOL rtc_settime (const RTC *rtc)
{
	/* Stop RTC */
	RTC_CCR = 0x12;

	/* Update RTC registers */
	RTC_SEC = rtc->sec;
	RTC_MIN = rtc->min;
	RTC_HOUR = rtc->hour;
	RTC_DOW = rtc->wday;
	RTC_DOM = rtc->mday;
	RTC_MONTH = rtc->month;
	RTC_YEAR = rtc->year;

	/* Start RTC with external XTAL */
	RTC_CCR = 0x11;

	return TRUE;
}


