#include <stdlib.h>
#include <stdint.h>

#include "debug.h"
#include "uart.h"


extern FILE uart_stdout;

extern int debug_level; /* the higher, the more messages... */

#if defined(NO_DEBUG) && defined(__GNUC__)
/* Nothing. debug has been "defined away" in debug.h already. */
#else
void debug(int level, char* format, ...) {
#ifdef NDEBUG
    /* Empty body, so a good compiler will optimise calls
       to pmesg away */
#else
    va_list args;
    if (!(debug_level & level))
        return;
    va_start(args, format);
    printf(format, args);
    va_end(args);
#endif /* NDEBUG */
#endif /* NDEBUG && __GNUC__ */
}
   
