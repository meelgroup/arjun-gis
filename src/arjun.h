/******************************************
Copyright (C) 2020 Mate Soos

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
***********************************************/

#ifndef ARJUN_H__
#define ARJUN_H__

#include <cstdint>
#include <vector>
#include <utility>
#include <string>
#include <tuple>
#include <cryptominisat5/cryptominisat.h>
#include <cryptominisat5/solvertypesmini.h>

namespace ArjunNS {
    struct ArjPrivateData;
    #ifdef _WIN32
    class __declspec(dllexport) Arjun
    #else
    class Arjun
    #endif
    {
    public:
        Arjun();
        ~Arjun();
        std::string get_version_info();
        std::string get_compilation_env();
        std::string get_solver_version_info();

        // Adding CNF
        uint32_t nVars();
        void new_var();
        bool add_xor_clause(const std::vector<uint32_t>& vars, bool rhs);
        bool add_clause(const std::vector<CMSat::Lit>& lits);
        bool add_bnn_clause(
            const std::vector<CMSat::Lit>& lits,
            signed cutoff,
            CMSat::Lit out = CMSat::lit_Undef);
        void new_vars(uint32_t num);

        // Perform indep set calculation
        uint32_t set_starting_sampling_set(const std::vector<uint32_t>& vars);
        uint32_t start_with_clean_sampling_set();
        std::vector<uint32_t> get_indep_set();
        uint32_t get_orig_num_vars() const;
        void varreplace();
        std::vector<uint32_t> get_empty_occ_sampl_vars() const;

        // by anna; For group independent support
        /** by anna; maps variable name to the index of the variable group that
         * it is a member of. */
        std::vector<uint32_t> var2var_group;
        /** by anna; maps variable group index to the variable names that are
         * members of that group. */
        std::vector<std::vector<uint32_t>> var_groups;
        /** by anna */
        void set_variable_groups(
          const std::vector<uint32_t>& _var2var_group,
          const std::vector<std::vector<uint32_t>>& _var_groups);
        /** by anna */
        void set_group_independent_support(uint32_t group_ind);
        /** by anna; for debugging purposes only. TODO: remove when done. */
        void print_var_groups(); // by anna

        //Get clauses
        void start_getting_small_clauses(uint32_t max_len, uint32_t max_glue, bool red = true);
        bool get_next_small_clause(std::vector<CMSat::Lit>& ret); //returns FALSE if no more
        void end_getting_small_clauses();
        const std::vector<CMSat::Lit> get_simplified_cnf() const;
        std::tuple<std::pair<std::vector<std::vector<CMSat::Lit>>, uint32_t>, std::vector<uint32_t>, uint32_t>
            get_fully_simplified_renumbered_cnf(
            const std::vector<uint32_t>& sampl_set,
            const std::vector<uint32_t>& empty_vars,
            const uint32_t orig_num_vars);
        const std::vector<CMSat::BNN*>& get_bnns() const;
        std::vector<CMSat::Lit> get_zero_assigned_lits() const;
        std::vector<std::pair<CMSat::Lit, CMSat::Lit> > get_all_binary_xors() const;

        //Set config
        void set_seed(uint32_t seed);
        void set_verbosity(uint32_t verb);
        void set_fast_backw(bool fast_backw);
        void set_distill(bool distill);
        void set_intree(bool intree);
        void set_guess(bool guess);
        void set_simp(bool simp);
        void set_pre_simplify(bool simp);
        void set_incidence_sort(uint32_t incidence_sort);
        void set_or_gate_based(bool or_gate_based);
        void set_xor_gates_based(bool xor_gates_based);
        void set_probe_based(bool probe_based);
        void set_forward(bool forward);
        void set_backward(bool backward);
        void set_assign_fwd_val(bool assign_fwd_val);
        void set_backw_max_confl(uint32_t backw_max_confl);
        void set_gauss_jordan(bool gauss_jordan);
        void set_regularly_simplify(bool reg_simp);
        void set_fwd_group(uint32_t forward_group);
        void set_find_xors(bool find_xors);
        void set_backbone_simpl(bool backbone_simpl);
        void set_ite_gate_based(bool ite_gate_based);
        void set_irreg_gate_based(const bool irreg_gate_based);
        void set_gate_sort_special(bool gate_sort_special);
        void set_backbone_simpl_max_confl(uint64_t backbone_simpl_max_confl);
        //void set_polar_mode(CMSat::PolarityMode mode);
        void set_pred_forever_cutoff(int pred_forever_cutoff = -1);
        void set_every_pred_reduce(int every_pred_reduce = -1);
        void set_empty_occs_based(const bool empty_occs_based);
        void set_mirror_empty(const bool mirror_empty);

        //Get config
        uint32_t get_verbosity() const;
        bool get_fast_backw() const;
        bool get_distill() const;
        bool get_intree() const;
        bool get_guess() const;
        bool get_pre_simplify() const;
        uint32_t get_incidence_sort() const;
        bool get_or_gate_based() const;
        bool get_xor_gates_based() const;
        bool get_probe_based() const;
        bool get_forward() const;
        bool get_backward() const;
        bool get_assign_fwd_val() const;
        uint32_t get_backw_max_confl() const;
        bool get_gauss_jordan() const;
        bool get_regularly_simplify() const;
        uint32_t get_fwd_group() const;
        bool get_find_xors() const;
        bool get_backbone_simpl() const;
        bool get_ite_gate_based() const;
        bool get_irreg_gate_based() const;
        bool get_gate_sort_special() const;

        long unsigned get_backbone_simpl_max_confl() const;

    private:
        ArjPrivateData* arjdata = NULL;
    };
}

#endif
