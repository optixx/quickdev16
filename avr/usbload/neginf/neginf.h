/*
 *  neginf.h 
 *  neginf -- embedded inflate lib
 *
 *  public header file
 */

#ifndef NEGINF_H
#define NEGINF_H

#include "neginf_conf.h"

#if defined(NEGINF_USE_SEQ_WRITES) && defined(NEGINF_USE_REL_COPY)
#else
#ifndef NEGINF_POS_TRACKING
#define NEGINF_POS_TRACKING
#endif
#endif

void neginf_init(nsize start_pos);
void neginf_process_byte(nbyte byte);

#ifdef NEGINF_POS_TRACKING
nsize neginf_output_position();
#endif

// callbacks

#ifdef NEGINF_USE_SEQ_WRITES
void neginf_cb_seq_byte(nbyte byte);
#else
void neginf_cb_byte(nsize pos, nbyte byte);
#endif

#ifdef NEGINF_USE_REL_COPY
void neginf_cb_rel_copy(nint distance, nint length);
#else
void neginf_cb_copy(nsize from, nsize to, nint length);
#endif

void neginf_cb_completed();

#endif
