/*
 * =====================================================================================
 *
 * ________        .__        __    ________               ____  ________
 * \_____  \  __ __|__| ____ |  | __\______ \   _______  _/_   |/  _____/
 *  /  / \  \|  |  \  |/ ___\|  |/ / |    |  \_/ __ \  \/ /|   /   __  \
 * /   \_/.  \  |  /  \  \___|    <  |    `   \  ___/\   / |   \  |__\  \
 * \_____\ \_/____/|__|\___  >__|_ \/_______  /\___  >\_/  |___|\_____  /
 *        \__>             \/     \/        \/     \/                 \/
 *              ___.
 *  __ __  _____\_ |__
 * |  |  \/  ___/| __ \
 * |  |  /\___ \ | \_\ \
 * |____//____  >|___  /
 *            \/     \/
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

   
