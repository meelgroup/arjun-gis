/*
 Arjun

 Copyright (c) 2019, Mate Soos and Kuldeep S. Meel. All rights reserved.

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

#ifndef ARJUN_COMMON_H
#define ARJUN_COMMON_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <string>
#include <cryptominisat5/cryptominisat.h>
#include "cryptominisat5/dimacsparser.h"

#include "time_mem.h"
#include "config.h"

using namespace CMSat;
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::set;
using std::string;
using std::vector;

struct Common
{
    Common() {
        sampling_set = &sampling_set_tmp1;
        other_sampling_set = &sampling_set_tmp2;
        interrupt_asap = false;
        set_up_solver();
    }

    ~Common()
    {
        delete solver;
    }

    Config conf;
    CMSat::SATSolver* solver = NULL;
    vector<uint32_t> sampling_set_tmp1;
    vector<uint32_t> sampling_set_tmp2;
    vector<uint32_t>* sampling_set = NULL;
    std::atomic<bool> interrupt_asap;

    vector<Lit> tmp;
    vector<char> seen;
    uint32_t orig_num_vars = std::numeric_limits<uint32_t>::max();
    uint32_t total_eq_removed = 0;
    uint32_t total_set_removed = 0;
    uint32_t mult_or_invers_var;
    enum ModeType {one_mode, many_mode};

    //assert indic[var] to FASLE to force var==var+orig_num_vars
    vector<uint32_t> var_to_indic; //maps an ORIG VAR to an INDICATOR VAR
    vector<uint32_t> indic_to_var; //maps an INDICATOR VAR to ORIG VAR


    vector<uint32_t>* other_sampling_set = NULL;
    map<uint32_t, vector<uint32_t>> global_assump_to_testvars;

    //Incidence as counted by clauses it's appeared together with other variables
    vector<uint32_t> incidence;

    //Incidence as counted by probing
    vector<uint32_t> incidence_probing;

    //maps var->commpart. If it doesn't belong anywhere, it's -1
    vector<int> commpart;

    //maps variable -> number of communities it's connected to via clauses
    vector<set<int>> var_to_num_communities;

    //total incidence in a commpart. Maps commpart->maxinc
    vector<uint32_t> commpart_incs;

    vector<double> vsids_scores;
    vector<Lit> dont_elim;
    vector<Lit> tmp_implied_by;

    void update_sampling_set(
        const vector<uint32_t>& unknown,
        const vector<char>& unknown_set,
        const vector<uint32_t>& indep
    );
    bool preproc_and_duplicate();
    void add_fixed_clauses();
    void print_orig_sampling_set();
    void start_with_clean_sampling_set();
    void duplicate_problem();
    void get_incidence();
    void calc_community_parts();
    void set_up_solver();


    //guess
    std::mt19937 random_source = std::mt19937(0);
    uint32_t guess_div = 10;
    void run_guess();
    void fill_assumptions_guess(
        vector<Lit>& assumptions,
        const vector<uint32_t>& indep,
        const vector<uint32_t>& unknown,
        const vector<char>& unknown_set,
        uint32_t group,
        uint32_t offs,
        uint32_t index,
        vector<char>& dontremove_vars);
    void guess_round(
        uint32_t group,
        bool reverse = false,
        bool shuffle = false,
        uint32_t offset = 0);
    uint32_t guess_remove_and_update_ass(
        vector<Lit>& assumptions,
        vector<char>& unknown_set,
        vector<char>& dontremove_vars);

    //simp
    vector<uint32_t> toClear;
    bool simp();
    void remove_definabile_by_xor();
    void remove_definable_by_gates();
    void remove_zero_assigned_literals(bool print = true);
    void remove_eq_literals(bool print = true);
    bool probe_all();

    //forward
    void set_guess_forward_round(
        const vector<uint32_t>& indep,
        vector<uint32_t>& unknown,
        vector<char>& unknown_set,
        uint32_t group,
        uint32_t offs,
        vector<char>& guess_set);
    void fill_assumptions_forward(
        vector<Lit>& assumptions,
        const vector<uint32_t>& indep,
        vector<uint32_t>& unknown,
        uint32_t group,
        uint32_t offs,
        vector<char>& guess_set);
    bool forward_round(
        uint32_t max_iters = std::numeric_limits<uint32_t>::max(),
        uint32_t group = 1,
        int offset = 0);

    //backward
    void fill_assumptions_backward(
        vector<Lit>& assumptions,
        vector<uint32_t>& unknown,
        const vector<char>& unknown_set,
        const vector<uint32_t>& indep);
    void backward_round();

    //Sorting
    template<class T> void sort_unknown(T& unknown);

};

inline double stats_line_percent(double num, double total)
{
    if (total == 0) {
        return 0;
    } else {
        return num/total*100.0;
    }
}

template<class T>
struct IncidenceSorter
{
    IncidenceSorter(const vector<T>& _inc) :
        inc(_inc)
    {}

    bool operator()(const T a, const T b) {
        if (inc[a] != inc[b]) {
            return inc[a] > inc[b];
        }
        return a < b;
    }

    const vector<T>& inc;
};

template<class T>
struct IncidenceSorter2
{
    IncidenceSorter2(const vector<T>& _inc, const vector<T>& _inc2) :
        inc(_inc),
        inc2(_inc2)
    {}

    bool operator()(const T a, const T b) {
        if (inc[a] != inc[b]) {
            return inc[a] > inc[b];
        }
        if (inc2[a] != inc2[b]) {
            return inc2[a] > inc2[b];
        }
        return a < b;
    }

    const vector<T>& inc;
    const vector<T>& inc2;
};

struct IncidenceSorterCommPart
{
    IncidenceSorterCommPart(const Common* _comm) :
        comm(_comm)
    {}

    bool operator()(const uint32_t a, const uint32_t b) {
        assert(a < comm->orig_num_vars);
        assert(b < comm->orig_num_vars);

        auto part_a = comm->commpart.at(a);
        auto part_b = comm->commpart.at(b);

        if (part_a == -1 && part_b == -1 ) {
            return false;
        }

        //If not in "part", put at the end
        if (part_a == -1) {
            return false;
        }
        if (part_b == -1) {
            return true;
        }

        //Put parts with smaller MAX incidence first
        auto part_a_inc = comm->commpart_incs.at(part_a);
        auto part_b_inc = comm->commpart_incs.at(part_b);
        if (part_a_inc != part_b_inc) {
            return part_a_inc < part_b_inc;
        }
        return false;

        auto a_inc = comm->incidence[a];
        auto b_inc = comm->incidence[b];
        if (a_inc != b_inc) {
            return a_inc > b_inc; //"a" has larger incidence -> return TRUE
        }
        return a < b;
    }

    const Common* comm;
};



struct IncidenceSorterCommPartToOtherComm
{
    IncidenceSorterCommPartToOtherComm(const Common* _comm) :
        comm(_comm)
    {}

    bool operator()(const uint32_t a, const uint32_t b) {
        assert(a < comm->orig_num_vars);
        assert(b < comm->orig_num_vars);

        auto part_a = comm->var_to_num_communities.at(a).size();
        auto part_b = comm->var_to_num_communities.at(b).size();

        if (part_a != part_b) {
            return part_a < part_b; //"a" is connected to LESS communities -> return TRUE
        }

        auto a_inc = comm->incidence_probing[a];
        auto b_inc = comm->incidence_probing[b];
        if (a_inc != b_inc) {
            return a_inc > b_inc; //"a" has LARGER incidence -> return TRUE
        }

        return a < b;
    }

    const Common* comm;
};

template<class T>
void Common::sort_unknown(T& unknown)
{
    if (conf.incidence_sort == 1) {
        std::sort(unknown.begin(), unknown.end(), IncidenceSorter<uint32_t>(incidence));
    } else if (conf.incidence_sort == 2) {
        std::sort(unknown.begin(), unknown.end(), IncidenceSorter2<uint32_t>(incidence, incidence_probing));
    } else if (conf.incidence_sort == 3) {
        std::sort(unknown.begin(), unknown.end(), IncidenceSorter<uint32_t>(incidence_probing));
    } else if (conf.incidence_sort == 4) {
        std::sort(unknown.begin(), unknown.end(), IncidenceSorterCommPart(this));
    } else if (conf.incidence_sort == 5) {
        std::sort(unknown.begin(), unknown.end(), IncidenceSorterCommPartToOtherComm(this));
    } else if (conf.incidence_sort == 6) {
        std::shuffle(unknown.begin(), unknown.end(), random_source);
    } else {
        cout << "ERROR: wrong sorting mechanism given" << endl;
        exit(-1);
    }
}

//ARJUN_COMMON_H
#endif
