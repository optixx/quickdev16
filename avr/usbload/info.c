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

#include "info.h"
#include "uart.h"



extern FILE uart_stdout;


#if defined(NO_INFO) && defined(__GNUC__)

#define info(format, args...) ((void)0)

#else
void info(char* format, ...) {
#ifdef NO_INFO

#else
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif 
}
#endif 

   
