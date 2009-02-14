#include "lpc214x.h"
#include "startup.h"
#include "console.h"

#define BAUD_RATE 115200 


void delay_ms (int count){
    int i;
    count *= 3000;
    for (i = 0; i < count; i++) {
        asm volatile ("nop");
    }
}

int main(void)
{
    unsigned int i;
    Initialize();
    ConsoleInit(60000000 / (16 * BAUD_RATE));
    puts("Init done\n");
    IODIR0 |= 1 << 10;          // P0.10 is an output
    IODIR0 |= 1 << 11;          // P0.10 is an output
    IOSET0 = 1 << 10;           //LED off
    IOSET0 = 1 << 11;           //LED off

    while (1) {
        delay_ms(1000);
        IOSET0 = 1 << 10;       //LED off
        IOCLR0 = 1 << 11;       //LED on
        puts("led1: off  led2: on\n");
        delay_ms(1000);
        IOCLR0 = 1 << 10;       //LED on
        IOSET0 = 1 << 11;       //LED off
        puts("led1: on   led2: off\n");
    }
}
