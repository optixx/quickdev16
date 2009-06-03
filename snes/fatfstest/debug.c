#include <string.h>
#include <stdarg.h>
#include <fcntl.h>

#include "data.h"
#include "pad.h"
#include "PPU.h"
#include "ressource.h"


word debugMap[0x400];
static char debug_buffer[255];


void debug_init(void) {
	word i;
	for(i=0; i<0x400; i++) {
		debugMap[i] = 0x00;
	}
    memset(debug_buffer,0,255);
}


void debug_enable(void){
	VRAMLoad((word) debugFont_pic, 0x5000, 2048);
	CGRAMLoad((word) debugFont_pal, (byte) 0x00, (word) 16);
	VRAMLoad((word) debugMap, 0x4000, 0x0800);
	setTileMapLocation(0x4000, (byte) 0x00, (byte) 0);
	setCharacterLocation(0x5000, (byte) 0);
	*(byte*) 0x2100 = 0x0f; // enable background
}


void _print_char(word y,word x, unsigned char c){
  VRAMByteWrite((byte) (c-32), (word) (0x4000+x+(y*0x20)));
}

void _print_screen(word y, char *buffer){
    char l;
    char x = 0;
    l = strlen(buffer);
    waitForVBlank();
    while(*buffer){
        if (*buffer == '\n' ) {
          while(x++<32)
              _print_char(y,x,' ');
          x = 0;
          y++;
      }
      _print_char(y,x,*buffer);
      x++;
      buffer++;
    }
}

void _print_console(const char *buffer){
	while(*buffer)
	    *(byte*) 0x3000=*buffer++;
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
  vsprintf(debug_buffer,fmt,ap);
  va_end(ap);
  _print_screen(y,debug_buffer);
}
