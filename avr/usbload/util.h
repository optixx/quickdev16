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

#ifndef __UTIL_H__
#define __UTIL_H__

 uint8_t *util_strupper(uint8_t *s);
uint8_t *util_strlower(uint8_t *s);
void util_chomp(uint8_t *s);
void util_trim(uint8_t *s);
uint32_t util_sscandec(const uint8_t *s);
uint32_t util_sscanhex(const uint8_t *s);
uint8_t util_sscanbool(const uint8_t *s);

#endif
