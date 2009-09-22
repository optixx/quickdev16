/*
 *  neginf_priv.h
 *  neginf -- embedded inflate lib
 *
 *  internal header file
 */

#ifndef NEGINF_PRIV_H
#define NEGINF_PRIV_H

typedef struct neginf_state_s neginf_state;
struct neginf_state_s {
    ntiny queue_size; // 0 .. 24
    ntiny mode;
    nbool last_block;
#ifdef NEGINF_POS_TRACKING
    nsize output_pos;
#endif
    // can be left uninitialized
    nbuf input_queue; // three input bytes
    
    ntiny raw_size;
    ntiny tcode;
    nint code;
    nint match_len;
    
    nint order;
    ntiny torder;
    nint hlit;
    ntiny hdist;
    ntiny hclen;
    
    ntiny lit_len_lengths[288];
    nint lit_len_begins[14];
    
    ntiny dist_lengths[32];
    nint dist_begins[14];
    
    ntiny hc_lengths[19];
    nint hc_begins[14];
    // what could be saved by limiting this to 7
    // will be lost due to the extra code i guess
    
    
}
#ifdef NEGINF_PACKED_STATE
__attribute__((__packed__))
#endif
;

enum neginf_mode {
    mode_await_block = 0,
    mode_raw_block_begin,
    mode_raw_block_begin2,
    mode_raw_block,
    mode_fixed_block_begin,
    mode_huff_block,
    mode_huff_len_addbits,
    mode_huff_dist,
    mode_huff_dist_addbits,
    mode_dynamic_block_begin,
    mode_dynamic_read_lc,
    mode_dynamic_read_lit_len,
    mode_dynamic_read_dist,
    mode_count
};

static void await_block();
static void raw_block_begin();
static void raw_block_begin2();
static void raw_block();
static void fixed_block_begin();
static void huff_block();
static void huff_len_addbits();
static void huff_dist();
static void huff_dist_addbits();
static void dynamic_block_begin();
static void dynamic_read_lc();
static void dynamic_read_lit_len();
static void dynamic_read_dist();

static void compute_begins();
static void compute_begin(ntiny * lengths, nint * begins, nint size);
static nint lit_len_read();
static nint dist_read();
static nint huff_read(ntiny * lengths, nint * begins, nint size);
static ntiny lc_read(ntiny * lengths);

static nint lookahead();
static void consume(ntiny amount);

#ifndef NEGINF_USE_SEQ_WRITES
static void neginf_cb_seq_byte(nbyte byte);
#endif

#ifndef NEGINF_USE_REL_COPY
void neginf_cb_rel_copy(nint distance, nint length);
#endif

#endif


