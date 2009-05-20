#ifndef _COMMFUNC
#define _COMMFUNC

#include "integer.h"

void uart0_init (void);
int uart0_test (void);
void uart0_put (BYTE);
BYTE uart0_get (void);

#endif

