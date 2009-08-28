
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
#include "integer.h"



typedef void (*FUNC) (void);

padStatus pad1;

void initInternalRegisters(void)
{
    characterLocation[0] = 0x0000;
    characterLocation[1] = 0x0000;
    characterLocation[2] = 0x0000;
    characterLocation[3] = 0x0000;
    debug_init();
}

void preInit(void)
{

    // For testing purpose ... 
    // Insert code here to be executed before register init
} 

void halt(void)
{
    while (1);
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

void boot(DWORD addr)
{
    FUNC fn;
    //printfc("SNES::boot addr=%lx\n", addr);
    fn = (FUNC) addr;
    fn();

} 

unsigned char i;
unsigned char j;

void main(void)
{
    initInternalRegisters();
    *(byte *) 0x2105 = 0x01;    // MODE 1 value
    *(byte *) 0x212c = 0x01;    // Plane 0 (bit one) enable register
    *(byte *) 0x212d = 0x00;    // All subPlane disable
    *(byte *) 0x2100 = 0x0f;    // enable background

    debug_enable();
    i=0;
    j=0;
    while (1) {
        printfs(0,"IRQ COUNT  %i", i);
        printfs(1,"NMI COUNT  %i", j++);
        waitForVBlank();
    }
}

void IRQHandler(void)
{
    i = i + 1;
}

void NMIHandler(void)
{

    // processEvents();
}
