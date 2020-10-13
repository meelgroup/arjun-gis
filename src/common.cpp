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

#include "common.h"
#include "louvain_communities/louvain_communities.h"

using std::pair;
using std::make_pair;


void Common::update_sampling_set(
    const vector<uint32_t>& unknown,
    const vector<char>& unknown_set,
    const vector<uint32_t>& indep
)
{
    other_sampling_set->clear();
    for(const auto& var: unknown) {
        if (unknown_set[var]) {
            other_sampling_set->push_back(var);

        }
    }
    for(const auto& var: indep) {
        other_sampling_set->push_back(var);
    }
    //TODO: atomic swap
    std::swap(sampling_set, other_sampling_set);

}

void Common::start_with_clean_sampling_set()
{
    seen.clear();
    seen.resize(solver->nVars(), 0);

    sampling_set->clear();
    vector<Lit> already_assigned = solver->get_zero_assigned_lits();
    for (Lit l: already_assigned) {
        seen[l.var()] = 1;
    }

    //Only set up for sampling if it's not already set
    for (size_t i = 0; i < solver->nVars(); i++) {
        if (seen[i] == 0) {
            sampling_set->push_back(i);
        }
    }

    //Clear seen
    for (Lit l: already_assigned) {
        seen[l.var()] = 0;
    }
}


void Common::print_orig_sampling_set()
{
    if (sampling_set->size() > 100) {
        cout
        << "c [mis] Sampling var set contains over 100 variables, not displaying"
        << endl;
    } else {
        cout << "c [mis] Sampling set: ";
        for (auto v: *sampling_set) {
            cout << v+1 << ", ";
        }
        cout << endl;
    }
    cout << "c [mis] Orig size         : " << sampling_set->size() << endl;
}

void Common::add_fixed_clauses()
{
    dont_elim.clear();
    var_to_indic.clear();
    var_to_indic.resize(orig_num_vars, var_Undef);
    indic_to_var.clear();
    indic_to_var.resize(solver->nVars(), var_Undef);

    //Indicator variable is TRUE when they are NOT equal
    for(uint32_t var: *sampling_set) {
        //(a=b) = !f
        //a  V -b V  f
        //-a V  b V  f
        //a  V  b V -f
        //-a V -b V -f
        solver->new_var();
        uint32_t this_indic = solver->nVars()-1;
        //torem_orig.push_back(Lit(this_indic, false));
        var_to_indic[var] = this_indic;
        dont_elim.push_back(Lit(this_indic, false));
        indic_to_var.resize(this_indic+1, var_Undef);
        indic_to_var[this_indic] = var;

        tmp.clear();
        tmp.push_back(Lit(var,               false));
        tmp.push_back(Lit(var+orig_num_vars, true));
        tmp.push_back(Lit(this_indic,      false));
        solver->add_clause(tmp);

        tmp.clear();
        tmp.push_back(Lit(var,               true));
        tmp.push_back(Lit(var+orig_num_vars, false));
        tmp.push_back(Lit(this_indic,      false));
        solver->add_clause(tmp);

        tmp.clear();
        tmp.push_back(Lit(var,               false));
        tmp.push_back(Lit(var+orig_num_vars, false));
        tmp.push_back(Lit(this_indic,      true));
        solver->add_clause(tmp);

        tmp.clear();
        tmp.push_back(Lit(var,               true));
        tmp.push_back(Lit(var+orig_num_vars, true));
        tmp.push_back(Lit(this_indic,      true));
        solver->add_clause(tmp);
    }

    if (false) {
        //This is a set of clauses where k1..kN are new indicators:
        // a1 V -k1
        //-b1 V -k1
        // --> i.e. if a1=False or b1=True --> k1 is False
        // --> one of the ks must be TRUE
        //k1 V k2 V ... kN
        vector<Lit> tmp2;
        for(uint32_t var: *sampling_set) {
            solver->new_var();
            uint32_t k = solver->nVars()-1;
            dont_elim.push_back(Lit(k, false));

            tmp.clear();
            tmp.push_back(Lit(var, false));
            tmp.push_back(Lit(k, true));
            solver->add_clause(tmp);

            tmp.clear();
            tmp.push_back(Lit(var+orig_num_vars, true));
            tmp.push_back(Lit(k, true));
            solver->add_clause(tmp);

            tmp2.push_back(Lit(k, false));
        }
        solver->add_clause(tmp2);
    }

    //Don't eliminate the sampling variables
    for(uint32_t var: *sampling_set) {
        dont_elim.push_back(Lit(var, false));
        dont_elim.push_back(Lit(var+orig_num_vars, false));
    }
}

void Common::duplicate_problem()
{
    vector<vector<Lit>> cnf;
    solver->start_getting_small_clauses(
        std::numeric_limits<uint32_t>::max(),
        std::numeric_limits<uint32_t>::max(),
        false);

    bool ret = true;
    vector<Lit> clause;
    while(ret) {
        ret = solver->get_next_small_clause(clause);
        if (ret) {
            cnf.push_back(clause);
        }
    }
    solver->end_getting_small_clauses();
    solver->new_vars(orig_num_vars);

    for(auto& cl: cnf) {
        for(auto& l: cl) {
            l = Lit(l.var()+orig_num_vars, l.sign());
        }
        solver->add_clause(cl);
    }
}

void Common::get_incidence()
{
    incidence.resize(orig_num_vars, 0);
    incidence_probing.resize(orig_num_vars, 0);
//     incidence = solver->get_var_incidence();
    vector<uint32_t> inc = solver->get_lit_incidence();
    for(uint32_t i = 0; i < orig_num_vars; i++) {
        Lit l = Lit(i, true);
        incidence[l.var()] = std::min(inc[l.toInt()],inc[(~l).toInt()]);
        //incidence[l.var()] = inc[(~l).toInt()];
    }
}

void Common::set_up_solver()
{
    assert(solver == NULL);
    solver = new SATSolver(NULL, &interrupt_asap);
    solver->set_up_for_arjun();
    solver->set_bve(0);
    solver->set_verbosity(0);
    solver->set_intree_probe(conf.intree);
    solver->set_distill(conf.distill);
    solver->set_sls(0);
    if (conf.polarmode == 0 || conf.polarmode == 1) {
        solver->set_default_polarity(conf.polarmode);
    } else {
        assert(conf.polarmode == 2);
        solver->set_polarity_auto();
    }
}

void Common::preproc_and_duplicate()
{
    orig_num_vars = solver->nVars();
    seen.clear();
    seen.resize(solver->nVars(), 0);
    get_incidence();
    calc_community_parts();

    //Solve problem to SAT
    double solve_time = cpuTime();
    cout << "c [mis] Solving problem once..." << endl;
    solver->set_max_confl(50000);
    auto ret = solver->solve();
    //solver->print_stats();
    if (ret == l_False) {
        cout << "c [mis] CNF is unsatisfiable. Exiting." << endl;
        exit(0);
    }
    cout << "c [mis] Solved problem to " << ret << " T: " << (cpuTime()-solve_time) << endl;

    //Simplify problem
    simp();
    //incidence = solver->get_var_incidence(); //NOTE: makes it slower
    solver->set_verbosity(std::max<int>(conf.verb-2, 0));

    //Read in file again, with offset
    cout << "c [mis] Duplicating CNF..." << endl;
    double dupl_time = cpuTime();
    duplicate_problem();
    cout << "c [mis] Duplicated CNF. T:" << (cpuTime() - dupl_time) << endl;

    //BVE ***ONLY***
    solver->set_intree_probe(false);
    solver->set_distill(false);
    //Don't eliminate the orignial variables
    for(uint32_t var: *sampling_set) {
        dont_elim.push_back(Lit(var, false));
        dont_elim.push_back(Lit(var+orig_num_vars, false));
    }
    double simpBVETime = cpuTime();
    cout << "c [mis] CMS::simplify() with *only* BVE..." << endl;
    solver->set_bve(1);
    solver->set_verbosity(0);
    string str("occ-bve");
    solver->simplify(&dont_elim, &str);
    solver->set_verbosity(0);
    cout << "c [mis] CMS::simplify() with *only* BVE finished. T: "
    << cpuTime() - simpBVETime
    << endl;


    //Add the connection clauses, indicator variables, etc.
    double duplTime = cpuTime();
    add_fixed_clauses();
    cout << "c [mis] Adding fixed clauses time: " << (cpuTime()-duplTime) << endl;

    //Seen needs re-init, because we got new variables
    seen.clear();
    seen.resize(solver->nVars(), 0);
}

void Common::calc_community_parts()
{
    vector<vector<Lit>> cnf;
    solver->start_getting_small_clauses(
        std::numeric_limits<uint32_t>::max(),
        std::numeric_limits<uint32_t>::max(),
        false);

    bool ret = true;
    vector<Lit> cl;
    map<pair<uint32_t, uint32_t>, long double> edges;
    while(ret) {
        ret = solver->get_next_small_clause(cl);
        if (!ret) {
            continue;
        }

        if (cl.size() == 1) {
            continue;
        }

        //Update VIG graph
        long double weight = 1.0L/((long double)cl.size()*((long double)cl.size()-1.0L)/2.0L);
        for(uint32_t i = 0; i < cl.size(); i ++) {
            for(uint32_t i2 = i+1; i2 < cl.size(); i2 ++) {
                uint32_t v1 = cl[i].var();
                uint32_t v2 = cl[i2].var();
                assert(v1 < orig_num_vars);
                assert(v2 < orig_num_vars);

                //must start with smallest
                if (v2  < v1) {
                    std::swap(v1, v2);
                }
                auto edge = make_pair(v1, v2);
                auto it = edges.find(edge);
                if (it == edges.end()) {
                    edges[edge] = weight;
                } else {
                    it->second+=weight;
                }
            }
        }
    }
    solver->end_getting_small_clauses();

    double myTime = cpuTime();
    LouvainC::Communities graph;
    for(const auto& it: edges) {
        graph.add_edge(it.first.first, it.first.second, it.second);
    }
    graph.calculate(true);
    commpart.clear();
    commpart.resize(orig_num_vars, -1);
    auto mapping = graph.get_mapping();
    for(const auto& x: mapping) {
        assert(x.first < orig_num_vars);
        commpart[x.first] = x.second;
        if (x.second == -1) {
            continue;
        }
        if ((unsigned)x.second >= commpart_incs.size()) {
            commpart_incs.resize(x.second+1, -1);
        }
        commpart_incs[x.second] = std::max(
            commpart_incs[x.second],
            incidence[x.first]);
    }

    var_to_num_communities.resize(orig_num_vars);
    solver->start_getting_small_clauses(
        std::numeric_limits<uint32_t>::max(),
        std::numeric_limits<uint32_t>::max(),
        false);
    ret = true;
    while(ret) {
        ret = solver->get_next_small_clause(cl);
        if (!ret) {
            continue;
        }

        if (cl.size() == 1) {
            continue;
        }

        for(uint32_t i = 0; i < cl.size(); i ++) {
            for(uint32_t i2 = i+1; i2 < cl.size(); i2 ++) {
                uint32_t v = cl[i].var();
                uint32_t comm = commpart[cl[i2].var()];
                var_to_num_communities[v].insert(comm);
            }
        }
    }
    solver->end_getting_small_clauses();

    cout << "c [mis-comm] Number of communities: " << commpart_incs.size()
    << " T: " << (cpuTime() - myTime)
    << endl;
}
