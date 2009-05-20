#include "integer.h"
#include "comm.h"
#define xgetc()		(char)uart0_get()

int xatoi (char**, long*);
void xputc (char);
void xputs (const char*);
void xitoa (long, int, int);
void xprintf (const char*, ...);
void put_dump (const BYTE*, DWORD ofs, int cnt);
void get_line (char*, int len);

