#include <avr/wdt.h>

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__


void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

#define soft_reset()        \
do                          \
{                           \
    wdt_enable(WDTO_500MS );\
    for(;;)                 \
    {                       \
    }                       \
} while(0)

#endif

