#ifndef __CD64IO_H__
#define __CD64IO_H__

#ifdef CD64_USE_LIBIEEE1284
#include <ieee1284.h>
int cd64_open_ieee1284(struct cd64_t *cd64);
int cd64_close_ieee1284(struct cd64_t *cd64);
int cd64_xfer_ieee1284(struct cd64_t *cd64, uint8_t *wr, uint8_t *rd, int delayms);
#endif

#ifdef CD64_USE_PPDEV
#ifndef __linux__
#error ppdev can only be used on Linux
#endif
#include <sys/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
int cd64_open_ppdev(struct cd64_t *cd64);
int cd64_close_ppdev(struct cd64_t *cd64);
int cd64_xfer_ppdev(struct cd64_t *cd64, uint8_t *wr, uint8_t *rd, int delayms);
#endif

#ifdef CD64_USE_PORTDEV
#ifndef __linux__
#error portdev can only be used on Linux
#endif
int cd64_open_portdev(struct cd64_t *cd64);
int cd64_close_portdev(struct cd64_t *cd64);
int cd64_xfer_portdev(struct cd64_t *cd64, uint8_t *wr, uint8_t *rd, int delayms);
#endif

#ifdef CD64_USE_RAWIO
/* #define REALLY_SLOW_IO */
#if defined __linux__ && (defined __i386__ || defined __x86_64__)
#include <sys/io.h>
#endif
#ifdef __OpenBSD__
#include <sys/types.h>
#include <machine/sysarch.h>
#include <i386/pio.h>
/* pio.h defines several I/O functions & macros, including the macros inb() and
 * outb(). This shows that using a bit of inline assembly is not such a bad idea
 * at all. */
#undef inb
#define inb(port) __inb(port)
#undef outb
#define outb(data, port) __outb(port, data)
#endif
#ifdef __FreeBSD__
#include <fcntl.h>
#include <machine/cpufunc.h>
/* Almost the same story as under OpenBSD. cpufunc.h defines the macros inb()
 * and outb(). We redefine them. Be sure _POSIX_SOURCE is not defined before
 * including <machine/cpufunc.h>. */
#undef inb
#define inb(port) inbv(port)
#undef outb
#define outb(data, port) outbv(port, data)
#endif
#ifdef __BEOS__
#include <fcntl.h>
#endif
#ifdef _MSC_VER
#include <conio.h>                              /* inp() & outp() */
#pragma warning(push)
#pragma warning(disable: 4820) /* 'bytes' bytes padding added after construct 'member_name' */
#include <io.h>                                 /* access() */
#pragma warning(pop)
#define F_OK 0
#endif
#ifdef __MSDOS__
#include <pc.h>                                 /* inportb() & outportb() */
#endif
#if defined _WIN32 || defined __CYGWIN__
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4255) /* 'function' : no function prototype given: converting '()' to '(void)' */
#pragma warning(disable: 4668) /* 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives' */
#pragma warning(disable: 4820) /* 'bytes' bytes padding added after construct 'member_name' */
#endif
#include <windows.h>                            /* defines _WIN32 (checks for   */
#ifdef _MSC_VER                                 /*  __CYGWIN__ must come first) */
#pragma warning(pop)
#endif
#endif /* _WIN32 || __CYGWIN__ */

int cd64_open_rawio(struct cd64_t *cd64);
int cd64_close_rawio(struct cd64_t *cd64);
int cd64_xfer_rawio(struct cd64_t *cd64, uint8_t *wr, uint8_t *rd, int delayms);
#endif

#if defined _WIN32 && !defined __CYGWIN__
/* milliseconds */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4255) /* 'function' : no function prototype given: converting '()' to '(void)' */
#pragma warning(disable: 4668) /* 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives' */
#pragma warning(disable: 4820) /* 'bytes' bytes padding added after construct 'member_name' */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#define MSLEEP(x) Sleep(x)
#elif defined __MSDOS__
/* milliseconds */
#include <dos.h>
#define MSLEEP(x) delay(x)
#elif defined __BEOS__
/* microseconds */
#include <OS.h>
#define MSLEEP(x) snooze((x) * 1000)
#else                                           /* UNIX & Cygwin */
/* microseconds */
#include <unistd.h>
#define MSLEEP(x) usleep((x) * 1000)
#endif

#if defined __STDC_VERSION && __STDC_VERSION >= 19990L && !defined DEBUG
/* If DEBUG is defined the keyword inline is not recognised (syntax error). */
#define INLINE inline
#elif defined _MSC_VER
/* Visual C++ doesn't allow inline in C source code */
#define INLINE __inline
#else
#define INLINE
#endif

#endif
