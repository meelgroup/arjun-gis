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

#include "common.h"

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
        << "c [arjun] Sampling var set contains over 100 variables, not displaying"
        << endl;
    } else {
        cout << "c [arjun] Sampling set: ";
        for (auto v: *sampling_set) {
            cout << v+1 << ", ";
        }
        cout << endl;
    }
    cout << "c [arjun] Orig size         : " << sampling_set->size() << endl;
}

void Common::add_fixed_clauses()
{
    double fix_cl_time = cpuTime();
    dont_elim.clear();
    var_to_indic.clear();
    var_to_indic.resize(orig_num_vars, var_Undef);
    indic_to_var.clear();
    indic_to_var.resize(solver->nVars(), var_Undef);

    //Indicator variable is TRUE when they are NOT equal
    // Original variable: a
    // Copied variable: b
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

    //Don't eliminate the sampling variables
    // TODO: why only the negative literals?
    for(uint32_t var: *sampling_set) {
        dont_elim.push_back(Lit(var, false));
        dont_elim.push_back(Lit(var+orig_num_vars, false));
    }
    if (conf.verb) {
        cout << "c [arjun] Adding fixed clauses time: " << (cpuTime()-fix_cl_time) << endl;
    }
}

void Common::duplicate_problem()
{
    solver->set_verbosity(std::max<int>(conf.verb-2, 0));

    //Duplicate the already simplified problem
    if (conf.verb) cout << "c [arjun] Duplicating CNF..." << endl;
    double dupl_time = cpuTime();
    vector<Lit> cnf = get_cnf();

    solver->new_vars(orig_num_vars);
    vector<Lit> cl;
    for(Lit l: cnf) {
        if (l != lit_Undef) {
            l = Lit(l.var()+orig_num_vars, l.sign());
            cl.push_back(l);
            continue;
        }
        solver->add_clause(cl);
        cl.clear();
    }

    vector<BNN*> bnns = solver->get_bnns();
    vector<Lit> lits;
    for (const BNN* bnn: bnns) {
        if (bnn == NULL) {
            continue;
        }
        lits.clear();
        for (const auto& l: *bnn) {
            lits.push_back(Lit(l.var()+orig_num_vars, l.sign()));
        }
        if (bnn->set) {
            assert(bnn->out == lit_Undef);
            solver->add_bnn_clause(lits, bnn->cutoff);
        } else {
            assert(bnn->out != lit_Undef);
            Lit out = Lit(bnn->out.var()+orig_num_vars, bnn->out.sign());
            solver->add_bnn_clause(lits, bnn->cutoff, out);
        }
    }
    if (conf.verb) cout << "c [arjun] Duplicated CNF. T: " << (cpuTime() - dupl_time) << endl;
}

vector<Lit> Common::get_cnf()
{
    vector<Lit> cnf;
    solver->get_all_irred_clauses(cnf);
    return cnf;
}

void Common::get_incidence()
{
    incidence.resize(orig_num_vars, 0);
    incidence_probing.resize(orig_num_vars, 0);
    vector<uint32_t> inc = solver->get_lit_incidence();
    for(uint32_t i = 0; i < orig_num_vars; i++) {
        Lit l = Lit(i, true);
        if (conf.incidence_sort == 10) {
            incidence[l.var()] = inc[l.toInt()] + inc[(~l).toInt()];
        } else if (conf.incidence_sort == 11) {
            incidence[l.var()] = std::max(inc[l.toInt()], inc[(~l).toInt()]);
        } else {
            incidence[l.var()] = std::min(inc[l.toInt()],inc[(~l).toInt()]);
        }
    }
}

void Common::set_up_solver()
{
    assert(solver == NULL);
    solver = new SATSolver(NULL);
    solver->set_up_for_arjun();
    solver->set_renumber(0);
    solver->set_bve(0);
    solver->set_verbosity(std::max((int)conf.verb-2, 0));
    solver->set_intree_probe(conf.intree && conf.simp);
    solver->set_distill(conf.distill && conf.simp);
    solver->set_sls(0);
}

bool Common:: simplify_bve_only()
{
    //BVE ***ONLY***, don't eliminate the orignial variables

    solver->set_intree_probe(false);
    solver->set_distill(false);
    for(uint32_t var: *sampling_set) {
        dont_elim.push_back(Lit(var, false));
        dont_elim.push_back(Lit(var+orig_num_vars, false));
    }
    double simpBVETime = cpuTime();
    if (conf.verb) {
        cout << "c [arjun] CMS::simplify() with *only* BVE..." << endl;
    }

    //Do BVE
    if (conf.simp) {
        solver->set_bve(1);
        solver->set_verbosity(std::max((int)conf.verb-2, 0));
        string str("occ-bve");
        if (solver->simplify(&dont_elim, &str) == l_False) {
            return false;
        }
        if (conf.verb) {
            cout << "c [arjun] CMS::simplify() with *only* BVE finished. T: "
            << cpuTime() - simpBVETime
            << endl;
        }
    }
    solver->set_intree_probe(true);
    solver->set_distill(true);
    return true;
}

bool Common::run_gauss_jordan()
{
    if (conf.gauss_jordan && conf.simp) {
        string str = "occ-xor";
        solver->set_bve(0);
        solver->set_allow_otf_gauss();
        solver->set_xor_detach(false);
        if (solver->simplify(&dont_elim, &str) == l_False) {
            return false;
        }
    }
    return true;
}

bool Common::preproc_and_duplicate()
{
    orig_num_vars = solver->nVars();
    seen.clear();
    seen.resize(solver->nVars(), 0);

    get_incidence();
    if (conf.simp && !simplify()) return false;
    //incidence = solver->get_var_incidence(); //NOTE: makes it slower
    duplicate_problem();
    if (!simplify_bve_only()) return false;

    add_fixed_clauses(); //Add the connection clauses, indicator variables, etc.
    if (!run_gauss_jordan()) return false;;

    //Seen needs re-init, because we got new variables
    seen.clear(); seen.resize(solver->nVars(), 0);

    solver->set_simplify(conf.regularly_simplify && conf.simp);
    solver->set_intree_probe(conf.intree && conf.simp);
    solver->set_distill(conf.distill && conf.simp);
    solver->set_find_xors(conf.gauss_jordan && conf.simp);
    return true;
}

bool Common::in_variable_group(uint32_t var)
{
    assert(var2var_group.size() > var);
    return var2var_group[var];
}

uint32_t Common::get_group_idx(uint32_t var)
{
    return var2var_group[var];
}
