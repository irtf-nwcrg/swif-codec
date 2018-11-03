/**
 * SWiF Codec: an open-source sliding window FEC codec in C
 * https://github.com/irtf-nwcrg/swif-codec
 */

/*
 * PRNG coming from RLC FEC Scheme, version 10, I-D.
 * https://datatracker.ietf.org/doc/draft-ietf-tsvwg-rlc-fec-scheme/
 */

 /**
  * Tiny Mersenne Twister only 127 bit internal state
  *
  * Authors : Mutsuo Saito (Hiroshima University)
  *           Makoto Matsumoto (University of Tokyo)
  *
  * Copyright (c) 2011, 2013 Mutsuo Saito, Makoto Matsumoto,
  * Hiroshima University and The University of Tokyo.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  *
  *   - Redistributions of source code must retain the above copyright
  *     notice, this list of conditions and the following disclaimer.
  *   - Redistributions in binary form must reproduce the above
  *     copyright notice, this list of conditions and the following
  *     disclaimer in the documentation and/or other materials
  *     provided with the distribution.
  *   - Neither the name of the Hiroshima University nor the names of
  *     its contributors may be used to endorse or promote products
  *     derived from this software without specific prior written
  *     permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
  * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
  * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
  * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  */

 #include "swif_includes.h"


 /**
  * tinymt32 internal state vector and parameters
  */
 typedef struct {
     uint32_t status[4];
     uint32_t mat1;
     uint32_t mat2;
     uint32_t tmat;
 } tinymt32_t;

 static void tinymt32_next_state (tinymt32_t * s);
 static uint32_t tinymt32_temper (tinymt32_t * s);
 static uint32_t tinymt32_generate_uint32 (tinymt32_t * random);

 /**
  * Parameter set to use for the IETF RLC FEC Schemes specification.
  * Do not change.
  * This parameter set is the first entry of the precalculated parameter
  * sets in file tinymt32dc.0.1048576.txt, by Kenji Rikitake, available
  * at: https://github.com/jj1bdx/tinymtdc-longbatch/blob/master/
  *     tinymt32dc/tinymt32dc.0.1048576.txt
  * It is also the parameter set used:
  *    Rikitake, K., "TinyMT Pseudo Random Number Generator for
  *    Erlang", ACM 11th SIGPLAN Erlang Workshop (Erlang'12),
  *    September, 2012.
  */
 const uint32_t  TINYMT32_MAT1_PARAM = UINT32_C(0x8f7011ee);
 const uint32_t  TINYMT32_MAT2_PARAM = UINT32_C(0xfc78ff1f);
 const uint32_t  TINYMT32_TMAT_PARAM = UINT32_C(0x3793fdff);


 /**
  * This function initializes the internal state array with a 32-bit
  * unsigned integer seed.
  * @param s pointer to tinymt internal state.
  * @param seed  a 32-bit unsigned integer used as a seed.
  */
 void tinymt32_init (tinymt32_t * s, uint32_t seed)
 {
     const uint32_t    MIN_LOOP = 8;
     const uint32_t    PRE_LOOP = 8;
     s->status[0] = seed;
     s->status[1] = s->mat1 = TINYMT32_MAT1_PARAM;
     s->status[2] = s->mat2 = TINYMT32_MAT2_PARAM;
     s->status[3] = s->tmat = TINYMT32_TMAT_PARAM;
     for (int i = 1; i < MIN_LOOP; i++) {
     s->status[i & 3] ^= i + UINT32_C(1812433253)
         * (s->status[(i - 1) & 3]
            ^ (s->status[(i - 1) & 3] >> 30));
     }
     for (int i = 0; i < PRE_LOOP; i++) {
     tinymt32_next_state(s);
     }
 }

 /**
  * This function outputs an integer in the [0 .. maxv-1] range.
  * theJPster answer in:
  * https://stackoverflow.com/questions/2509679/
  *      how-to-generate-a-random-integer-number-from-within-a-range
  *
  * @param s pointer to tinymt internal state.
  * @return  32-bit unsigned integer between 0 and maxv-1 inclusive.
  */
 uint32_t tinymt32_rand (tinymt32_t * s, uint32_t maxv)
 {
     uint32_t    r;
     uint32_t    buckets = 0xffffffff / maxv;
     uint32_t    limit = buckets * maxv;
     /* Create equal size buckets all in a row, then fire randomly
      * towards the buckets until you land in one of them. All buckets
      * are equally likely. If you land off the end of the line of
      * buckets, try again. */
     do {
         r = tinymt32_generate_uint32(s);
     } while (r >= limit);
     return (r / buckets);
 }

 /**
  * Internal tinymt32 constants and functions.
  * Users should not call these functions directly.
  */
 const uint32_t  TINYMT32_SH0 = 1;
 const uint32_t  TINYMT32_SH1 = 10;
 const uint32_t  TINYMT32_SH8 = 8;
 const uint32_t  TINYMT32_MASK = UINT32_C(0x7fffffff);

 /**
  * This function changes internal state of tinymt32.
  * @param s pointer to tinymt internal state.
  */
 static void tinymt32_next_state (tinymt32_t * s)
 {
     uint32_t x;
     uint32_t y;

     y = s->status[3];
     x = (s->status[0] & TINYMT32_MASK)
         ^ s->status[1]
         ^ s->status[2];
     x ^= (x << TINYMT32_SH0);
     y ^= (y >> TINYMT32_SH0) ^ x;
     s->status[0] = s->status[1];
     s->status[1] = s->status[2];
     s->status[2] = x ^ (y << TINYMT32_SH1);
     s->status[3] = y;
     s->status[1] ^= -((int32_t)(y & 1)) & s->mat1;
     s->status[2] ^= -((int32_t)(y & 1)) & s->mat2;
 }

 /**
  * This function outputs 32-bit unsigned integer from internal state.
  * @param s pointer to tinymt internal state.
  * @return  32-bit unsigned pseudos number
  */
 static uint32_t tinymt32_temper (tinymt32_t * s)
 {
     uint32_t t0, t1;
     t0 = s->status[3];
     t1 = s->status[0] + (s->status[2] >> TINYMT32_SH8);
     t0 ^= t1;
     t0 ^= -((int32_t)(t1 & 1)) & s->tmat;
     return t0;
 }

 /**
  * This function outputs 32-bit unsigned integer from internal state.
  * @param s pointer to tinymt internal state.
  * @return  32-bit unsigned integer r (0 <= r < 2^32)
  */
 static uint32_t tinymt32_generate_uint32 (tinymt32_t * random) {
     tinymt32_next_state(random);
     return tinymt32_temper(random);
 }
