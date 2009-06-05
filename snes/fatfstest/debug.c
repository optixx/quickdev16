#include <string.h>
#include <stdarg.h>
#include <fcntl.h>

#include "debug.h"
#include "data.h"
#include "pad.h"
#include "PPU.h"
#include "ressource.h"



word debug_colors[] = {
  0xff7f, 0x1f00, 0x1802, 0x9772, 0xb55a, 0xbf5a, 0x1f5b, 0x7b73,
  0x987f, 0x7f5f, 0xff03, 0xfc7f, 0xff7f, 0xff7f, 0xff7f, 0xff7f
};

word debugMap[0x400];
static char debug_buffer[255];
static char screen_buffer[255];


void debug_init(void) {
	word i;
	for(i=0; i<0x400; i++) {
		debugMap[i] = 0x00;
	}
    memset(debug_buffer,0,255);
}


void debug_enable(void){
	VRAMLoad((word) debugFont_pic, 0x5000, 2048);
	VRAMLoad((word) debugMap, 0x4000, 0x0800);
	setTileMapLocation(0x4000, (byte) 0x00, (byte) 0);
	setCharacterLocation(0x5000, (byte) 0);
	*(byte*) 0x2100 = 0x0f; // enable background

    // hex(24 << 10 | 24 << 5 | 24 )
    // '0x6318'

    *(byte*) 0x2121 = 0x00;
    *(byte*) 0x2122 = 0x18;
    *(byte*) 0x2122 = 0x63;


}

void clears(void) {
    word i,y;
    for(y=0; y<20; y++){
      waitForVBlank();
      for(i=0; i<32; i++){
        *(byte*)0x2115 = 0x80;
        *(word*)0x2116 = 0x4000+i+(y*0x20);
        *(byte*)0x2118 = 0;
      }
  }
}

void _print_char(word y,word x,  char c){
  waitForVBlank();
  VRAMByteWrite((byte) (c-32), (word) (0x4000+x+(y*0x20)));
}

void _print_screen(word y, char *buffer){
    char l;
    char x = 0;
    l = strlen(buffer);
    waitForVBlank();
    while(*buffer){
        if (*buffer == '\n' ) {
            while(x++<32){
              *(byte*)0x2115 = 0x80;
              *(word*)0x2116 = 0x4000+x+(y*0x20);
              *(byte*)0x2118 = 0;
            }
            x = 0;
            y++;
            buffer++;
            waitForVBlank();
            continue;
        }
        *(byte*)0x2115 = 0x80;
        *(word*)0x2116 = 0x4000+x+(y*0x20);
        *(byte*)0x2118 = *buffer-32;
        x++;
        buffer++;
        //waitForVBlank();
    }
}
void _print_console(const char *buffer){
    while(*buffer)
        *(byte*) 0x3000=*buffer++;
}

void printfc(char *fmt,...){
  va_list ap;
  va_start(ap,fmt);
  vsprintf(debug_buffer,fmt,ap);
  va_end(ap);
  _print_console(debug_buffer);
}

void printfs(word y,char *fmt,...){
  va_list ap;
  va_start(ap,fmt);
  vsprintf(screen_buffer,fmt,ap);
  va_end(ap);
  _print_screen(y,screen_buffer);
}


/* keep the linker happy */
int open(const char * _name, int _mode){
    _print_console("open called\n");
    return -1;
}

int close(int fd){
    _print_console("close called\n");
    return -1;
    
} 

size_t read(int fd, void * buff, size_t len){
    _print_console("read called\n");
    return 0;
} 

size_t write(int fd, void * buffer, size_t len){
    _print_console("write called\n");
    return 0;
}

long lseek(int fd, long off, int count){
    _print_console("lseek called\n");
    return 0;
} 

int unlink(const char * name){
    _print_console("unlink called\n");
    return -1;
}

int isatty(){
    _print_console("isatty called\n");
    return 1;
}

