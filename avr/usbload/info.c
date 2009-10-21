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

#include "info.h"
#include "uart.h"
#include "config.h"




extern FILE uart_stdout;


#ifndef NO_INFO
    uint8_t buffer_info[FORMAT_BUFFER_LEN];
#endif

#if defined(NO_INFO) && defined(__GNUC__)

#define info(format, args...) ((void)0)

#else
void info_P(PGM_P format, ...) {
#ifdef NO_INFO

#else
    strlcpy_P((char*)buffer_info,format,FORMAT_BUFFER_LEN);
    va_list args;
    va_start(args, format);
    vprintf((char*)buffer_info, args);
    va_end(args);
#endif
}
#endif
   
