#include "data.h"

void debug_init(void);
void debug_enable(void);
void printfs(word y,char* fmt,...);
void printfc(char* fmt,...);
void clears(void);
void printc_packet(unsigned long addr,unsigned int len,byte *packet);

