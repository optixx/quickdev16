/*
 *  neginf.c
 *  neginf -- embedded inflate lib
 *
 *  inflate routines
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "neginf.h"
#include "neginf_priv.h"

typedef void(*mode_fun)() ;

static neginf_state state;
static const mode_fun mode_tab[mode_count] = {
    &await_block,
    &raw_block_begin,
    &raw_block_begin2,
    &raw_block,
    &fixed_block_begin,
    &huff_block,
    &huff_len_addbits,
    &huff_dist,
    &huff_dist_addbits,
    &dynamic_block_begin,
    &dynamic_read_lc,
    &dynamic_read_lit_len,
    &dynamic_read_dist
};

void neginf_init(nsize start_pos)
{
    state.queue_size = 0;
    state.mode = mode_await_block;
    state.last_block = 0;
#ifdef NEGINF_POS_TRACKING
    state.output_pos = start_pos;
#endif
}


void neginf_process_byte(nbyte byte)
{
    assert(state.queue_size <= 16);
    state.input_queue |= (byte << state.queue_size);
    state.queue_size += 8;
    
    while(state.queue_size >= 16)
    {
        //printf("qsize=%i mode=%i\n",state.queue_size,state.mode);
        mode_tab[state.mode]();
    }
}

#ifdef NEGINF_POS_TRACKING
nsize neginf_output_position()
{
    return state.output_pos;
}
#endif

nint lookahead()
{
    //printf("lookahead\n");
    return state.input_queue;
}

void consume(ntiny amount)
{
    //printf("consume %i %i\n",state.queue_size,amount);
    assert(state.queue_size > amount);
    state.input_queue >>= amount;
    state.queue_size -= amount;
}

void await_block()
{
    //printf("wait block\n");
    if(state.last_block)
    {
        neginf_cb_completed();
        consume(16);
    }
    else
    {
        nint la = lookahead();
        state.last_block = la & 1;
        consume(3);
        switch(la & 6)
        {
            case 0: // 00 uncompressed
                consume((state.queue_size) & 7); // align to byte
                state.mode = mode_raw_block_begin;
                break;
            case 2: // 01 fixed huffman
                state.mode = mode_fixed_block_begin;
                break;
            case 4: // 10 dynamic huffman
                state.mode = mode_dynamic_block_begin;
                break;
            default:
                assert(0);
        }
    }
}

void raw_block_begin()
{
    //printf("raw block begin\n");
    state.raw_size = lookahead() & 0xFFFF; // size of raw block
    consume(16);
    state.mode = mode_raw_block_begin2;
}

void raw_block_begin2()
{
    //printf("raw block begin2\n");
    consume(16); // we ignore the inverted size
    state.mode = mode_raw_block;
}

void raw_block()
{
    //printf("raw block\n");
    if(state.raw_size == 0)
    {
        state.mode = mode_await_block;
    }
    else
    {
        state.raw_size--;
        neginf_cb_seq_byte(lookahead() & 0xFF);
#ifdef NEGINF_POS_TRACKING
        state.output_pos++;
#endif
        consume(8);
    }
}

void fixed_block_begin()
{
    //printf("fixed block begin\n");
    
    nint i = 0;
    for(; i < 144; i++)
        state.lit_len_lengths[i] = 8;
    for(; i < 256; i++)
        state.lit_len_lengths[i] = 9;
    for(; i < 280; i++)
        state.lit_len_lengths[i] = 7;
    for(; i < 288; i++)
        state.lit_len_lengths[i] = 8;
    
    ntiny j;
    for(j = 0; i < 32; i++)
        state.dist_lengths[i] = 5;
    
    compute_begins();
    state.mode = mode_huff_block;
}

void huff_block()
{
    //printf("huff block\n");
    nint code = lit_len_read();
    if(code == 256)
    {
        state.mode = mode_await_block;
    }
    else if(code < 256)
    {
        neginf_cb_seq_byte(code);
#ifdef NEGINF_POS_TRACKING
        state.output_pos++;
#endif
    }
    else
    {
        state.code = code;
        state.mode = mode_huff_len_addbits;
    }
}

void huff_len_addbits()
{
    //printf("huff len addbits\n");
    nint len;
    nint code = state.code;
    nint la = lookahead();
    if(code < 265)
        len = code - 257 + 3;
    else if(code < 269)
    {
        len = (code - 265) * 2 + 11 + (la & 1);
        consume(1);
    }
    else if(code < 273)
    {
        len = (code - 269) * 4 + 19 + (la & 3);
        consume(2);
    }
    else if(code < 277)
    {
        len = (code - 273) * 8 + 35 + (la & 7);
        consume(3);
    }
    else if(code < 281)
    {
        len = (code - 277) * 16 + 67 + (la & 15);
        consume(4);
    }
    else if(code < 285)
    {
        len = (code - 281) * 32 + 131 + (la & 31);
        consume(5);
    }
    else
    {
        len = 258;
    }
    state.match_len = len;
    state.mode = mode_huff_dist;
}

void huff_dist()
{
    //printf("huff dist\n");
    state.tcode = dist_read();
    state.mode = mode_huff_dist_addbits;
}

void huff_dist_addbits()
{
    //printf("huff addbits\n");
    nint dist;
    ntiny code = state.tcode;
    
    if(code < 4)
    {
        dist = code+1;
    }
    else if(code > 29)
    {
        assert(0);
    }
    else
    {
        nint la = lookahead();
        ntiny len = (code - 2) / 2;
        dist = ((2 + (code & 1)) << len) + 1 + (((1 << len) - 1) & la);
        consume(len);
    }
    neginf_cb_rel_copy(dist, state.match_len);
#ifdef NEGINF_POS_TRACKING
    state.output_pos += state.match_len;
#endif
    
    state.mode = mode_huff_block;
}

void dynamic_block_begin()
{
    nint j;
    ntiny i;
    //printf("dynamic block begin\n");
    for(j = 0; j < 288; j++)
        state.lit_len_lengths[j] = 0;
    for(i = 0; i < 32; i++)
        state.dist_lengths[i] = 0;
    for(i = 0; i < 19; i++)
        state.hc_lengths[i] = 0;
    
    nint la = lookahead();
    state.hlit = (la & 31) + 257;
    state.hdist = ((la >> 5) & 31) + 1;
    state.hclen = ((la >> 10) & 15) + 4;
    state.torder = 0;
    consume(5+5+4);
    state.mode = mode_dynamic_read_lc;
}

void dynamic_read_lc()
{
    //printf("dynamic read lc\n");
    if(state.hclen == 0)
    {
        compute_begin(state.hc_lengths, state.hc_begins, 19);
        state.mode = mode_dynamic_read_lit_len;
        state.order = 0;
    }
    else
    {
        static const ntiny order[19] = {
            16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
        };
        ntiny i = lookahead() & 7;
        state.hc_lengths[order[state.torder]] = i;
        consume(3);
        
        state.torder++;
        state.hclen--;
    }
}

void dynamic_read_lit_len()
{
    //printf("dynamic read lit len\n");
    if(state.hlit == 0)
    {
        state.mode = mode_dynamic_read_dist;
        state.order = 0;
    }
    else
    {
        state.hlit -= lc_read(state.lit_len_lengths);
    }
}

void dynamic_read_dist()
{
    //printf("dynamic read dist\n");
    if(state.hdist == 0)
    {
        compute_begins();
        state.mode = mode_huff_block;
    }
    else
    {
        state.hdist -= lc_read(state.dist_lengths);
    }
}

ntiny lc_read(ntiny * lenghts)
{
    //printf("read lc\n");
    ntiny code = huff_read(state.hc_lengths, state.hc_begins, 19);
    // this reads 7 bits max so we still have 9 bits left in the buffer
    if(code < 16)
    {
        lenghts[state.order] = code;
        state.order++;
        return 1;
    }
    else if(code == 16)
    {
        ntiny i;
        ntiny copy = (lookahead() & 3) + 3;
        consume(2);
        for(i = 0; i < copy; i++)
            lenghts[state.order + i] = lenghts[state.order - 1];
        state.order += copy;
        return copy;
    }
    else
    {
        ntiny fill;
        ntiny i;
        
        if(code == 17)
        {
            fill = (lookahead() & 7) + 3;
            consume(3);
        }
        else
        {
            fill = (lookahead() & 127) + 11;
            consume(7);
        }
        for(i = 0; i < fill; i++)
        {
            lenghts[state.order] = 0;
            state.order++;
        }
        return fill;
    }    
}


void compute_begins()
{
    //printf("compute begins\n");
    compute_begin(state.lit_len_lengths, state.lit_len_begins, 288);
    compute_begin(state.dist_lengths, state.dist_begins, 32);
}


void compute_begin(ntiny * lengths, nint * begins, nint size)
{
    ntiny j;
    nint i;
    //printf("compute begin\n");
    for(j = 0; j < 14; j++)
        begins[j] = 0;
    for(i = 0; i < size; i++)
    {
        nint len = lengths[i];
        if(len != 0 && len != 15)
            begins[len-1] += 1 << (15 - len);
    }
    nint acc = 0;
    for(j = 0; j < 14; j++)
    {
        nint val = begins[j];
        acc += val;
        begins[j] = acc;
    }    
}

nint lit_len_read()
{
    //printf("lit len read\n");
    
    return huff_read(state.lit_len_lengths, state.lit_len_begins, 288);
}

nint dist_read()
{
    //printf("dist read\n");
    return huff_read(state.dist_lengths, state.dist_begins, 32);
}

nint huff_read(ntiny * lenghts, nint * begins, nint size)
{
    //printf("huff read\n");
    nint code = 0;
    ntiny i;
    for(i = 1; i < 16; i++)
    {
        
        code |= (lookahead() & 1) << (15-i);
        consume(1);
        if(i == 15 || code < begins[i-1])
            break;
    }
    code -= begins[i-2];
    code >>= (15-i);
    nint j;
    for(j = 0; j < size; j++)
    {
        if(lenghts[j] == i)
        {
            if(code == 0)
                return j;
            code--;
        }
    }
    //assert(0);
    return 0; // silent warning
}

#ifndef NEGINF_USE_SEQ_WRITES
void neginf_cb_seq_byte(nbyte byte)
{
    neginf_cb_byte(state.output_pos, byte);
}
#endif

#ifndef NEGINF_USE_REL_COPY
void neginf_cb_rel_copy(nint distance, nint length)
{
    neginf_cb_copy(state.output_pos - distance, state.output_pos, length);
}
#endif
