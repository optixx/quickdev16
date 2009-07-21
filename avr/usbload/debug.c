/*
 * =====================================================================================
 *
 *            .d8888b  88888b.   .d88b.  .d8888b  888d888 8888b.  88888b.d88b.
 *            88K      888 "88b d8P  Y8b 88K      888P"      "88b 888 "888 "88b
 *            "Y8888b. 888  888 88888888 "Y8888b. 888    .d888888 888  888  888
 *                 X88 888  888 Y8b.          X88 888    888  888 888  888  888
 *             88888P' 888  888  "Y8888   88888P' 888    "Y888888 888  888  888
 *
 *                                  www.optixx.org
 *
 *
 *        Version:  1.0
 *        Created:  07/21/2009 03:32:16 PM
 *         Author:  david@optixx.org
 *
 * =====================================================================================
 */



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
#ifdef NO_DEBUG
    /* Empty body, so a good compiler will optimise calls
       to pmesg away */
#else
    va_list args;
    if (!(debug_level & level))
        return;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif /* NDEBUG */
#endif /* NDEBUG && __GNUC__ */
}

   
