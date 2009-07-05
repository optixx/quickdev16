#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>

#include "debug.h"
#include "data.h"
#include "pad.h"
#include "PPU.h"
#include "ressource.h"




#define DEBUG_BUFFER_SIZE 256

word debug_map[0x400];
char debug_buffer[DEBUG_BUFFER_SIZE];
//char screen_buffer[DEBUG_BUFFER_SIZE];



void debug_init(void)
{
    
    memset(debug_map, 0, 0x800);
    memset(debug_buffer, 0,DEBUG_BUFFER_SIZE);

#if 0    
    memset(screen_buffer, 0, DEBUG_BUFFER_SIZE);
    printfc("screen_buffer=%p\n",screen_buffer);
    printfc("debug_buffer=%p\n",debug_buffer);
    printfc("debug_map=%p\n",debug_map);
#endif
    
}


void debug_enable(void)
{
    VRAMLoad((word) debugFont_pic, 0x5000, 2048);
    VRAMLoad((word) debug_map, 0x4000, 0x0800);
    setTileMapLocation(0x4000, (byte) 0x00, (byte) 0);
    setCharacterLocation(0x5000, (byte) 0);
    *(byte *) 0x2100 = 0x0f;    // enable background

    // Font Color
    // hex(24 << 10 | 24 << 5 | 24 ) = '0x6318'
    *(byte *) 0x2121 = 0x02;
    *(byte *) 0x2122 = 0xff;
    *(byte *) 0x2122 = 0x7f;

    // Font Border Color
    *(byte *) 0x2121 = 0x00;
    *(byte *) 0x2122 = 0x00;
    *(byte *) 0x2122 = 0x00;

    // Background Color
    *(byte *) 0x2121 = 0x01;
    *(byte *) 0x2122 = 0x05;
    *(byte *) 0x2122 = 0x29;


}

void clears(void)
{
    word i, y;
    for (y = 0; y < 20; y++) {
        waitForVBlank();
        for (i = 0; i < 32; i++) {
            *(byte *) 0x2115 = 0x80;
            *(word *) 0x2116 = 0x4000 + i + (y * 0x20);
            *(byte *) 0x2118 = 0;
        }
    }
}

void _print_char(word y, word x, char c)
{
    waitForVBlank();
    VRAMByteWrite((byte) (c - 32), (word) (0x4000 + x + (y * 0x20)));
}


void _print_screen(word y, char *buffer)
{
    char l;
    unsigned int x; 
    x = y * 0x20;
    l = strlen(buffer);
    waitForVBlank();
    while (*buffer) {
        if (*buffer == '\n') {
            while (x++ < 32) {
                *(byte *) 0x2115 = 0x80;
                *(word *) 0x2116 = 0x4000 + x + (y * 0x20);
                *(byte *) 0x2118 = 0;
            }
            y ++;
            x = y * 0x20;
            buffer++;
            waitForVBlank();
            continue;
        }
        *(byte *) 0x2115 = 0x80;
        *(word *) 0x2116 = 0x4000 + x;
        *(byte *) 0x2118 = *buffer - 32;
        x++;
        buffer++;
#if 0
        waitForVBlank();
#endif
    }
}

void _print_console(const char *buffer)
{
    while (*buffer)
        *(byte *) 0x3000 = *buffer++;
}




void printfc(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf(debug_buffer, fmt, ap);
    va_end(ap);
    _print_console(debug_buffer);
    //memset(debug_buffer, 0, DEBUG_BUFFER_SIZE);

}

void printfs(word y, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf(debug_buffer, fmt, ap);
    va_end(ap);
    _print_screen(y, debug_buffer);
    //memset(debug_buffer, 0, DEBUG_BUFFER_SIZE);
    //memset(screen_buffer, 0, DEBUG_BUFFER_SIZE);
}

void printc_packet(unsigned long addr, unsigned int len, byte * packet)
{
    unsigned int i, j;
    unsigned int sum = 0;
    unsigned int clear = 0;

    for (i = 0; i < len; i += 16) {

        sum = 0;
        for (j = 0; j < 16; j++) {
            sum += packet[i + j];
        }
        if (!sum) {
            clear = 1;
            continue;
        }
        if (clear) {
            printfc("*\n");
            clear = 0;
        }
        printfc("%06lX:", addr + i);
        for (j = 0; j < 16; j++) {
            printfc(" %x", packet[i + j]);
        }
        printfc(" |");
        for (j = 0; j < 16; j++) {
            if (packet[i + j] >= 33 && packet[i + j] <= 126)
                printfc("%c", packet[i + j]);
            else
                printfc(".");
        }
        printfc("|\n");
    }
}

/*
 * keep the linker happy 
 */
int open(const char *_name, int _mode)
{
    _print_console("open called\n");
    return -1;
}

int close(int fd)
{
    _print_console("close called\n");
    return -1;

}

size_t read(int fd, void *buff, size_t len)
{
    _print_console("read called\n");
    return 0;
}

size_t write(int fd, void *buffer, size_t len)
{
    _print_console("write called\n");
    return 0;
}

long lseek(int fd, long off, int count)
{
    _print_console("lseek called\n");
    return 0;
}

int unlink(const char *name)
{
    _print_console("unlink called\n");
    return -1;
}

int isatty()
{
    _print_console("isatty called\n");
    return 1;
}
