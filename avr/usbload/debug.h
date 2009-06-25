#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>


#if defined(NO_DEBUG) && defined(__GNUC__)
/* gcc's cpp has extensions; it allows for macros with a variable number of
   arguments. We use this extension here to preprocess pmesg away. */
#define debug(level, format, args...) ((void)0)
#else
void debug(int level, char *format, ...);
/* print a message, if it is considered significant enough.
      Adapted from [K&R2], p. 174 */
#endif

void dump_packet(uint32_t addr,uint32_t len,uint8_t *packet);

#endif /* DEBUG_H */
        