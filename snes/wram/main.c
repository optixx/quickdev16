
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
void main(void)
{
    unsigned char i,j;
    unsigned long addr;
    initInternalRegisters();
    *(byte *) 0x2105 = 0x01;    // MODE 1 value
    *(byte *) 0x212c = 0x01;    // Plane 0 (bit one) enable register
    *(byte *) 0x212d = 0x00;    // All subPlane disable
    *(byte *) 0x2100 = 0x0f;    // enable background

    debug_enable();
#if 0    
    addr = 0x008000;
    for (i=0;i<28;i++){
        printfs(i,"ROW %02i %02X %08li %06lX",i,i,addr,addr);
        printfc("ROW %02i %02X %08li %06lX\n",i,i,addr,addr);
        addr += 0x10000;
    }
#endif

#define START_ADDR 0x020000
#define END_ADDR   0x7f2000
    addr = 0x000000;
    i=0;
    j=0;
    for (addr=START_ADDR; addr < END_ADDR; addr+=0x1000){
        if (addr > START_ADDR && addr%0x2000==0){
            //printfc("hit %06lx\n",addr);
            addr += 0x10000 - 0x2000;
            j++;
            if (j>26)
                j=0;
        }
        i++;
        printfc("fill mem=%06lx tag=%02x tag=%i \n", addr, i, i);
        *(byte *) (addr) = i;
    }
    i=0;
    j=0;
    for (addr=START_ADDR; addr < END_ADDR; addr+=0x1000){
        if (addr > START_ADDR && addr%0x2000==0){
            addr += 0x10000 - 0x2000;
            i++;
            j++;
            if (j>26)
                j=0;
        }
        printfs(j,"DUMP %06lX ",addr);
        printc_packet(addr,16,(byte *) (addr));
    }
    
    
    while (1) {
        wait();
    }
}

void IRQHandler(void)
{

}

void NMIHandler(void)
{

    // processEvents();
}
