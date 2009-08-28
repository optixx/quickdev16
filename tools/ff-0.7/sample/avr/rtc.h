#include "integer.h"

typedef struct {
	WORD	year;	/* 2000..2099 */
	BYTE	month;	/* 1..12 */
	BYTE	mday;	/* 1.. 31 */
	BYTE	wday;	/* 1..7 */
	BYTE	hour;	/* 0..23 */
	BYTE	min;	/* 0..59 */
	BYTE	sec;	/* 0..59 */
} RTC;

BOOL rtc_init (void);						/* Initialize RTC */
BOOL rtc_gettime (RTC*);					/* Get time */
BOOL rtc_settime (const RTC*);				/* Set time */
BOOL rtc_write (UINT, UINT, const void*);	/* Write RTC regs */
BOOL rtc_read (UINT, UINT, void*);			/* Read RTC regs */

