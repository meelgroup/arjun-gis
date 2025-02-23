/*
 Arjun

 Copyright (c) 2019, Mate Soos and Kuldeep S. Meel. 
               2022, Anna L.D. Latour.
 All rights reserved.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <cryptominisat5/solvertypesmini.h>

struct Config {
    int verb = 1;
    int seed = 0;
    int simp = 0;
    int fast_backw = 0;
    int distill = 1;
    int regularly_simplify = 0;
    int intree = 1;
    int guess = 0;
    int pre_simplify = 0;
    int incidence_sort = 1;
    int or_gate_based = 0;
    int xor_gates_based = 0;
    int ite_gate_based = 0;
    int irreg_gate_based = 0;
    int mirror_empty = 0;
    int empty_occs_based = 0;
    int probe_based = 1;
    int group_indep = 1;
    int forward = 0;
    int forward_group = 10;
    int backward = 1;
    int assign_fwd_val = 0;
    int gauss_jordan = 0;
    int backbone_simpl = 0;
    unsigned long backbone_simpl_max_confl = 10ULL*1000ULL;
    uint32_t backw_max_confl = 500;
    uint32_t guess_max_confl = 1000;
};

//ARJUN_CONFIG_H
#endif
