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



#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <avr/pgmspace.h>

#if defined(NO_DEBUG) && defined(__GNUC__)
/*
 * gcc's cpp has extensions; it allows for macros with a variable number of arguments. We use this extension here to preprocess pmesg away. 
 */
#define debug(level, format, args...) ((void)0)
#else
void debug(int level, char *format, ...);
/*
 * print a message, if it is considered significant enough. Adapted from [K&R2], p. 174 
 */
#endif



#if defined(NO_DEBUG) && defined(__GNUC__)
/*
 * gcc's cpp has extensions; it allows for macros with a variable number of arguments. We use this extension here to preprocess pmesg away. 
 */
#define debug_P(level, format, args...) ((void)0)
#else
void debug_P(int level, PGM_P format, ...);
/*
 * print a message, if it is considered significant enough. Adapted from [K&R2], p. 174 
 */
#endif


#endif                          /* DEBUG_H */
