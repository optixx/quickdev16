/*
 *  neginf_conf.h
 *  neginf -- embedded inflate lib
 *
 *  configuration header file
 */

#ifndef NEGINF_CONF_H
#define NEGINF_CONF_H

#include <stddef.h>
#include <stdint.h>


#define NEGINF_USE_SEQ_WRITES
//#define NEGINF_USE_REL_COPY
//#define NEGINF_POS_TRACKING

//#define NEGINF_8BIT
#define NEGINF_PACKED_STATE


#ifdef NEGINF_8BIT

typedef char nbool;
typedef uint8_t nbyte;
typedef uint8_t ntiny;
typedef uint16_t nint;
typedef uint32_t nbuf;

typedef uint32_t nsize;

#else

typedef int nbool; // boolean
typedef uint8_t nbyte; // has to be exaclty 8 bit, unsigned
typedef unsigned int ntiny; // has to be at least 8 bit, unsigned
typedef unsigned int nint; // has to be at least 16 bit, unsigned
typedef unsigned int nbuf; // has to be at least 24 bit, unsigned


typedef size_t nsize; // has be at least 24 bit, unsigned

#endif



#endif
