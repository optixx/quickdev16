/*
 * =====================================================================================
 *
 * ________        .__        __    ________               ____  ________
 * \_____  \  __ __|__| ____ |  | __\______ \   _______  _/_   |/  _____/
 *  /  / \  \|  |  \  |/ ___\|  |/ / |    |  \_/ __ \  \/ /|   /   __  \
 * /   \_/.  \  |  /  \  \___|    <  |    `   \  ___/\   / |   \  |__\  \
 * \_____\ \_/____/|__|\___  >__|_ \/_______  /\___  >\_/  |___|\_____  /
 *        \__>             \/     \/        \/     \/                 \/
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
#include <avr/pgmspace.h>

#include "debug.h"
#include "uart.h"
#include "config.h"

extern FILE uart_stdout;

extern int debug_level; /* the higher, the more messages... */


#ifndef NO_INFO
    uint8_t buffer_debug[FORMAT_BUFFER_LEN];
#endif
   
#if defined(NO_DEBUG) && defined(__GNUC__)
#else
void debug_P(int level, PGM_P format, ...) {
#ifdef NO_DEBUG

#else
    va_list args;
    if (!(debug_level & level))
        return;
    strlcpy_P((char*)buffer_debug,format,FORMAT_BUFFER_LEN);
    va_start(args, format);
    vprintf((char*)buffer_debug, args);
    va_end(args);
#endif
}
#endif 

