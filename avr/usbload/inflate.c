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
 *        Created:  09/22/2009
 *         Author:  jannis@harderweb.de
 *
 * =====================================================================================
 */

#include "neginf/neginf.h"

#include "inflate.h"

char inflate_done = 0;

void neginf_cb_completed()
{
    inflate_done = 1;
}

void neginf_cb_seq_byte(nbyte byte)
{
    // TODO: implement this
}

void neginf_cb_copy(nsize from, nsize to, nint length)
{
    // TODO: implement this
}
