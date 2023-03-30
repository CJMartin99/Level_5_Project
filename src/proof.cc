/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "proof.hh"

#include <algorithm>
#include <iterator>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <tuple>
#include <utility>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/stream.hpp>

using std::copy;
using std::endl;
using std::find;
using std::function;
using std::istreambuf_iterator;
using std::make_unique;
using std::map;
using std::max;
using std::min;
using std::move;
using std::ofstream;
using std::optional;
using std::ostream;
using std::ostreambuf_iterator;
using std::pair;
using std::set;
using std::string;
using std::stringstream;
using std::to_string;
using std::tuple;
using std::unique_ptr;
using std::vector;

using boost::iostreams::bzip2_compressor;
using boost::iostreams::file_sink;
using boost::iostreams::filtering_ostream;

#include <fmt/core.h>
#include <fmt/os.h>

/* There will be 3/4 versions of this code to reflect the different test version that will be run */
/* Preproccessing code will be used to seperate them */
namespace
{
    auto make_compressed_ostream(const string & fn) -> unique_ptr<ostream>
    {
        auto out = make_unique<filtering_ostream>();
        out->push(bzip2_compressor());
        out->push(file_sink(fn));
        return out;
    }
}

ProofError::ProofError(const string & m) noexcept :
    _message("Proof error: " + m)
{
}

auto ProofError::what() const noexcept -> const char *
{
    return _message.c_str();
}

#ifdef ORIGINAL
struct Proof::Imp
{
    string opb_filename, log_filename;
    stringstream model_stream, model_prelude_stream;
    unique_ptr<ostream> proof_stream;
    bool friendly_names;
    bool bz2 = false;
    bool super_extra_verbose = false;

    map<pair<long, long>, string> variable_mappings;
    map<long, string> binary_variable_mappings;
    map<tuple<long, long, long>, string> connected_variable_mappings;
    map<tuple<long, long, long, long>, string> connected_variable_mappings_aux;
    map<long, long> at_least_one_value_constraints, at_most_one_value_constraints, injectivity_constraints;
    map<tuple<long, long, long, long>, long> adjacency_lines;
    map<pair<long, long>, long> eliminations;

    #ifdef VECTOR
    vector<vector<int>> non_edge_constraints;
    #else
    map<pair<long, long>, long> non_edge_constraints;
    #endif
    
    long objective_line = 0;

    long nb_constraints = 0;
    long proof_line = 0;
    int largest_level_set = 0;

    bool clique_encoding = false;

    bool doing_hom_colour_proof = false;
    NamedVertex hom_colour_proof_p, hom_colour_proof_t;
    vector<NamedVertex> p_clique;
    map<int, NamedVertex> t_clique_neighbourhood;
    map<pair<pair<NamedVertex, NamedVertex>, pair<NamedVertex, NamedVertex> >, long> clique_for_hom_non_edge_constraints;

    vector<pair<int, int> > zero_in_proof_objectives;
};

Proof::Proof(const string & opb_file, const string & log_file, bool f, bool b, bool s) :
    _imp(new Imp)
{
    _imp->opb_filename = opb_file;
    _imp->log_filename = log_file;
    _imp->friendly_names = f;
    _imp->bz2 = b;
    _imp->super_extra_verbose = s;
}

Proof::Proof(Proof &&) = default;

Proof::~Proof() = default;

auto Proof::operator= (Proof &&) -> Proof & = default;

auto Proof::create_cp_variable(int pattern_vertex, int target_size,
        const function<auto (int) -> string> & pattern_name,
        const function<auto (int) -> string> & target_name) -> void
{
    for (int i = 0 ; i < target_size ; ++i)
        if (_imp->friendly_names)
            _imp->variable_mappings.emplace(pair{ pattern_vertex, i }, pattern_name(pattern_vertex) + "_" + target_name(i));
        else
            _imp->variable_mappings.emplace(pair{ pattern_vertex, i }, to_string(_imp->variable_mappings.size() + 1));

    _imp->model_stream << "* vertex " << pattern_vertex << " domain" << endl;
    for (int i = 0 ; i < target_size ; ++i)
        _imp->model_stream << "1 x" << _imp->variable_mappings[{ pattern_vertex, i }] << " ";
    _imp->model_stream << ">= 1 ;" << endl;
    _imp->at_least_one_value_constraints.emplace(pattern_vertex, ++_imp->nb_constraints);

    for (int i = 0 ; i < target_size ; ++i)
        _imp->model_stream << "-1 x" << _imp->variable_mappings[{ pattern_vertex, i }] << " ";
    _imp->model_stream << ">= -1 ;" << endl;
    _imp->at_most_one_value_constraints.emplace(pattern_vertex, ++_imp->nb_constraints);
}

auto Proof::create_injectivity_constraints(int pattern_size, int target_size) -> void
{
    for (int v = 0 ; v < target_size ; ++v) {
        _imp->model_stream << "* injectivity on value " << v << endl;

        for (int p = 0 ; p < pattern_size ; ++p) {
            auto x = _imp->variable_mappings.find(pair{ p, v });
            if (x != _imp->variable_mappings.end())
                _imp->model_stream << "-1 x" << x->second << " ";
        }
        _imp->model_stream << ">= -1 ;" << endl;
        _imp->injectivity_constraints.emplace(v, ++_imp->nb_constraints);
    }
}

auto Proof::create_forbidden_assignment_constraint(int p, int t) -> void
{
    _imp->model_stream << "* forbidden assignment" << endl;
    _imp->model_stream << "1 ~x" << _imp->variable_mappings[pair{ p, t }] << " >= 1 ;" << endl;
    ++_imp->nb_constraints;
    _imp->eliminations.emplace(pair{ p, t }, _imp->nb_constraints);
}

auto Proof::start_adjacency_constraints_for(int p, int t) -> void
{
    _imp->model_stream << "* adjacency " << p << " maps to " << t << endl;
}

auto Proof::create_adjacency_constraint(int p, int q, int t, const vector<int> & uu, bool) -> void
{
    _imp->model_stream << "1 ~x" << _imp->variable_mappings[pair{ p, t }];
    for (auto & u : uu)
        _imp->model_stream << " 1 x" << _imp->variable_mappings[pair{ q, u }];
    _imp->model_stream << " >= 1 ;" << endl;
    _imp->adjacency_lines.emplace(tuple{ 0, p, q, t }, ++_imp->nb_constraints);
}

auto Proof::finalise_model() -> void
{
    unique_ptr<ostream> f = (_imp->bz2 ? make_compressed_ostream(_imp->opb_filename + ".bz2") : make_unique<ofstream>(_imp->opb_filename));

    *f << "* #variable= " << (_imp->variable_mappings.size() + _imp->binary_variable_mappings.size()
            + _imp->connected_variable_mappings.size() + _imp->connected_variable_mappings_aux.size())
        << " #constraint= " << _imp->nb_constraints << endl;
    copy(istreambuf_iterator<char>{ _imp->model_prelude_stream }, istreambuf_iterator<char>{}, ostreambuf_iterator<char>{ *f });
    _imp->model_prelude_stream.clear();
    copy(istreambuf_iterator<char>{ _imp->model_stream }, istreambuf_iterator<char>{}, ostreambuf_iterator<char>{ *f });
    _imp->model_stream.clear();

    if (! *f)
        throw ProofError{ "Error writing opb file to '" + _imp->opb_filename + "'" };

    _imp->proof_stream = (_imp->bz2 ? make_compressed_ostream(_imp->log_filename + ".bz2") : make_unique<ofstream>(_imp->log_filename));

    *_imp->proof_stream << "pseudo-Boolean proof version 1.0" << endl;

    *_imp->proof_stream << "f " << _imp->nb_constraints << " 0" << endl;
    _imp->proof_line += _imp->nb_constraints;

    if (! *_imp->proof_stream)
        throw ProofError{ "Error writing proof file to '" + _imp->log_filename + "'" };
}

auto Proof::finish_unsat_proof() -> void
{
#ifndef COMMENTS
    *_imp->proof_stream << "* asserting that we've proved unsat" << endl;
#endif
    *_imp->proof_stream << "u >= 1 ;" << endl;
    ++_imp->proof_line;
    *_imp->proof_stream << "c " << _imp->proof_line << " 0" << endl;
}

auto Proof::emit_hall_set_or_violator(const vector<NamedVertex> & lhs, const vector<NamedVertex> & rhs) -> void
{
    *_imp->proof_stream << "* hall set or violator {";
    for (auto & l : lhs)
        *_imp->proof_stream << " " << l.second;
    *_imp->proof_stream << " } / {";
    for (auto & r : rhs)
        *_imp->proof_stream << " " << r.second;
    *_imp->proof_stream << " }" << endl;
    *_imp->proof_stream << "p";
    bool first = true;
    for (auto & l : lhs) {
        if (first) {
            first = false;
            *_imp->proof_stream << " " << _imp->at_least_one_value_constraints[l.first];
        }
        else
            *_imp->proof_stream << " " << _imp->at_least_one_value_constraints[l.first] << " +";
    }
    for (auto & r : rhs)
        *_imp->proof_stream << " " << _imp->injectivity_constraints[r.first] << " +";
    *_imp->proof_stream << " 0" << endl;
    ++_imp->proof_line;
}

auto Proof::root_propagation_failed() -> void
{
    *_imp->proof_stream << "* root node propagation failed" << endl;
}

auto Proof::guessing(int depth, const NamedVertex & branch_v, const NamedVertex & val) -> void
{
    *_imp->proof_stream << "* [" << depth << "] guessing " << branch_v.second << "=" << val.second << endl;
}

auto Proof::propagation_failure(const vector<pair<int, int> > & decisions, const NamedVertex & branch_v, const NamedVertex & val) -> void
{
    *_imp->proof_stream << "* [" << decisions.size() << "] propagation failure on " << branch_v.second << "=" << val.second << endl;
    *_imp->proof_stream << "u ";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " 1 ~x" << _imp->variable_mappings[pair{ var, val }];
    *_imp->proof_stream << " >= 1 ;" << endl;
    ++_imp->proof_line;
}

auto Proof::incorrect_guess(const vector<pair<int, int> > & decisions, bool failure) -> void
{
    if (failure)
        *_imp->proof_stream << "* [" << decisions.size() << "] incorrect guess" << endl;
    else
        *_imp->proof_stream << "* [" << decisions.size() << "] backtracking" << endl;

    *_imp->proof_stream << "u";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " 1 ~x" << _imp->variable_mappings[pair{ var, val }];
    *_imp->proof_stream << " >= 1 ;" << endl;
    ++_imp->proof_line;
}

auto Proof::out_of_guesses(const vector<pair<int, int> > &) -> void
{
}

auto Proof::unit_propagating(const NamedVertex & var, const NamedVertex & val) -> void
{
    *_imp->proof_stream << "* unit propagating " << var.second << "=" << val.second << endl;
}

auto Proof::start_level(int l) -> void
{
    *_imp->proof_stream << "# " << l << endl;
    _imp->largest_level_set = max(_imp->largest_level_set, l);
}

auto Proof::back_up_to_level(int l) -> void
{
    *_imp->proof_stream << "# " << l << endl;
    _imp->largest_level_set = max(_imp->largest_level_set, l);
}

auto Proof::forget_level(int l) -> void
{
    if (_imp->largest_level_set >= l)
        *_imp->proof_stream << "w " << l << endl;
}

auto Proof::back_up_to_top() -> void
{
    *_imp->proof_stream << "# " << 0 << endl;
}

auto Proof::post_restart_nogood(const vector<pair<int, int> > & decisions) -> void
{
    *_imp->proof_stream << "* [" << decisions.size() << "] restart nogood" << endl;
    *_imp->proof_stream << "u";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " 1 ~x" << _imp->variable_mappings[pair{ var, val }];
    *_imp->proof_stream << " >= 1 ;" << endl;
    ++_imp->proof_line;
}

auto Proof::post_solution(const vector<pair<NamedVertex, NamedVertex> > & decisions) -> void
{
    *_imp->proof_stream << "* found solution";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " " << var.second << "=" << val.second;
    *_imp->proof_stream << endl;

    *_imp->proof_stream << "v";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " x" << _imp->variable_mappings[pair{ var.first, val.first }];
    *_imp->proof_stream << endl;
    ++_imp->proof_line;
}

auto Proof::post_solution(const vector<int> & solution) -> void
{
    *_imp->proof_stream << "v";
    for (auto & v : solution)
        *_imp->proof_stream << " x" << _imp->binary_variable_mappings[v];
    *_imp->proof_stream << endl;
    ++_imp->proof_line;
}

auto Proof::new_incumbent(const vector<pair<int, bool> > & solution) -> void
{
    *_imp->proof_stream << "o";
    for (auto & [ v, t ] : solution)
        *_imp->proof_stream << " " << (t ? "" : "~") << "x" << _imp->binary_variable_mappings[v];
    for (auto & [ v, w ] : _imp->zero_in_proof_objectives)
        *_imp->proof_stream << " ~" << "x" << _imp->variable_mappings[pair{ v, w }];
    *_imp->proof_stream << endl;
    _imp->objective_line = ++_imp->proof_line;
}

auto Proof::new_incumbent(const vector<tuple<NamedVertex, NamedVertex, bool> > & decisions) -> void
{
    *_imp->proof_stream << "o";
    for (auto & [ var, val, t ] : decisions)
        *_imp->proof_stream << " " << (t ? "" : "~") << "x" << _imp->variable_mappings[pair{ var.first, val.first }];
    *_imp->proof_stream << endl;
    _imp->objective_line = ++_imp->proof_line;
}

auto Proof::create_binary_variable(int vertex,
                const function<auto (int) -> string> & name) -> void
{
    if (_imp->friendly_names)
        _imp->binary_variable_mappings.emplace(vertex, name(vertex));
    else
        _imp->binary_variable_mappings.emplace(vertex, to_string(_imp->binary_variable_mappings.size() + 1));
}

auto Proof::create_objective(int n, optional<int> d) -> void
{
    if (d) {
        _imp->model_stream << "* objective" << endl;
        for (int v = 0 ; v < n ; ++ v)
            _imp->model_stream << "1 x" << _imp->binary_variable_mappings[v] << " ";
        _imp->model_stream << ">= " << *d << ";" << endl;
        _imp->objective_line = ++_imp->nb_constraints;
    }
    else {
        _imp->model_prelude_stream << "min:";
        for (int v = 0 ; v < n ; ++ v)
            _imp->model_prelude_stream << " -1 x" << _imp->binary_variable_mappings[v];
        _imp->model_prelude_stream << " ;" << endl;
    }
}

#ifdef VECTOR
auto Proof::create_non_edge_constraint_vector(int size) -> void
{
    vector<vector<int>> non_edges;
    for (int i=0; i<size; i++) {
        vector<int> single_row;
        single_row.assign(size,0);
        non_edges.push_back(single_row); 
    }
    _imp->non_edge_constraints = non_edges;
}
#endif

auto Proof::create_non_edge_constraint(int p, int q) -> void
{
    _imp->model_stream << "-1 x" << _imp->binary_variable_mappings[p] << " -1 x" << _imp->binary_variable_mappings[q] << " >= -1 ;" << endl;

    ++_imp->nb_constraints;
    #ifdef VECTOR
    _imp->non_edge_constraints[p][q] = _imp->nb_constraints;
    _imp->non_edge_constraints[q][p] = _imp->nb_constraints;
    #else
    _imp->non_edge_constraints.emplace(pair{ p, q }, _imp->nb_constraints);
    _imp->non_edge_constraints.emplace(pair{ q, p }, _imp->nb_constraints);
    #endif
}

auto Proof::backtrack_from_binary_variables(const vector<int> & v) -> void
{
    if (! _imp->doing_hom_colour_proof) {
        *_imp->proof_stream << "u";
        for (auto & w : v)
            *_imp->proof_stream << " 1 ~x" << _imp->binary_variable_mappings[w];
        *_imp->proof_stream << " >= 1 ;" << endl;
        ++_imp->proof_line;
    }
    else {
#ifndef COMMENTS
        *_imp->proof_stream << "* backtrack shenanigans, depth " << v.size() << endl;
#endif
        function<auto (unsigned, const vector<pair<int, int> > &) -> void> f;
        f = [&] (unsigned d, const vector<pair<int, int> > & trail) -> void {
            if (d == v.size()) {
                *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ _imp->hom_colour_proof_p.first, _imp->hom_colour_proof_t.first }];
                for (auto & t : trail)
                    *_imp->proof_stream << " 1 ~x" << _imp->variable_mappings[t];
                *_imp->proof_stream << " >= 1 ;" << endl;
                ++_imp->proof_line;
            }
            else {
                for (auto & p : _imp->p_clique) {
                    vector<pair<int, int> > new_trail{ trail };
                    new_trail.emplace_back(pair{ p.first, _imp->t_clique_neighbourhood.find(v[d])->second.first });
                    f(d + 1, new_trail);
                }
            }
        };
        f(0, {});
    }
}

auto Proof::colour_bound(const vector<vector<int> > & ccs) -> void
{
#ifndef COMMENTS
    *_imp->proof_stream << "* bound, ccs";
    for (auto & cc : ccs) {
        *_imp->proof_stream << " [";
        for (auto & c : cc)
            *_imp->proof_stream << " " << c;
        *_imp->proof_stream << " ]";
    }
    *_imp->proof_stream << endl;
#endif

    vector<long> to_sum;
    auto do_one_cc = [&] (const auto & cc, const auto & non_edge_constraint) {
        if (cc.size() > 2) {
            *_imp->proof_stream << "p " << non_edge_constraint(cc[0], cc[1]);

            for (unsigned i = 2 ; i < cc.size() ; ++i) {
                *_imp->proof_stream << " " << i << " *";
                for (unsigned j = 0 ; j < i ; ++j)
                    *_imp->proof_stream << " " << non_edge_constraint(cc[i], cc[j]) << " +";
                *_imp->proof_stream << " " << (i + 1) << " d";
            }

            *_imp->proof_stream << endl;
            to_sum.push_back(++_imp->proof_line);
        }
        else if (cc.size() == 2) {
            to_sum.push_back(non_edge_constraint(cc[0], cc[1]));
        }
    };

    for (auto & cc : ccs) {
        if (_imp->doing_hom_colour_proof) {
            vector<pair<NamedVertex, NamedVertex> > bigger_cc;
            for (auto & c : cc)
                for (auto & v : _imp->p_clique)
                    bigger_cc.push_back(pair{ v, _imp->t_clique_neighbourhood.find(c)->second });

            *_imp->proof_stream << "* colour class [";
            for (auto & c : bigger_cc)
                *_imp->proof_stream << " " << c.first.second << "/" << c.second.second;
            *_imp->proof_stream << " ]" << endl;

            do_one_cc(bigger_cc, [&] (const pair<NamedVertex, NamedVertex> & a, const pair<NamedVertex, NamedVertex> & b) -> long {
                    return _imp->clique_for_hom_non_edge_constraints[pair{ a, b }];
                    });
        }
        else {
#ifdef VECTOR
            do_one_cc(cc, [&] (int a, int b) -> long { return _imp->non_edge_constraints[a][b]; });
#else
            do_one_cc(cc, [&] (int a, int b) -> long { return _imp->non_edge_constraints[pair{ a, b }]; });
#endif
        }

#ifdef COLOUR
        if (cc.size() != 1) {
            *_imp->proof_stream << "p " << _imp->objective_line;
            for (auto & t : to_sum)
                *_imp->proof_stream << " " << t << " +";
            *_imp->proof_stream << endl;
            ++_imp->proof_line;
        }
#else
        *_imp->proof_stream << "p " << _imp->objective_line;
        for (auto & t : to_sum)
            *_imp->proof_stream << " " << t << " +";
        *_imp->proof_stream << endl;
        ++_imp->proof_line;
#endif
    }
}

auto Proof::prepare_hom_clique_proof(const NamedVertex & p, const NamedVertex & t, unsigned size) -> void
{
    *_imp->proof_stream << "* clique of size " << size << " around neighbourhood of " << p.second << " but not " << t.second << endl;
    *_imp->proof_stream << "# 1" << endl;
    _imp->doing_hom_colour_proof = true;
    _imp->hom_colour_proof_p = p;
    _imp->hom_colour_proof_t = t;
}

auto Proof::start_hom_clique_proof(const NamedVertex & p, vector<NamedVertex> && p_clique, const NamedVertex & t, map<int, NamedVertex> && t_clique_neighbourhood) -> void
{
    _imp->p_clique = move(p_clique);
    _imp->t_clique_neighbourhood = move(t_clique_neighbourhood);

    *_imp->proof_stream << "* hom clique objective" << endl;
    vector<long> to_sum;
    for (auto & q : _imp->p_clique) {
        *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }];
        for (auto & u : _imp->t_clique_neighbourhood)
            *_imp->proof_stream << " 1 x" << _imp->variable_mappings[pair{ q.first, u.second.first }];
        *_imp->proof_stream << " >= 1 ;" << endl;
        to_sum.push_back(++_imp->proof_line);
    }

    *_imp->proof_stream << "p";
    bool first = true;
    for (auto & t : to_sum) {
        *_imp->proof_stream << " " << t;
        if (! first)
            *_imp->proof_stream << " +";
        first = false;
    }
    *_imp->proof_stream << endl;
    _imp->objective_line = ++_imp->proof_line;

    *_imp->proof_stream << "* hom clique non edges for injectivity" << endl;

    for (auto & p : _imp->p_clique)
        for (auto & q : _imp->p_clique)
            if (p != q) {
                for (auto & [ _, t ] : _imp->t_clique_neighbourhood) {
                    *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }] << " 1 ~x" << _imp->variable_mappings[pair{ q.first, t.first }] << " >= 1 ;" << endl;
                    ++_imp->proof_line;
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, t }, pair{ q, t } }, _imp->proof_line);
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ q, t }, pair{ p, t } }, _imp->proof_line);
                }
            }

    *_imp->proof_stream << "* hom clique non edges for variables" << endl;

    for (auto & p : _imp->p_clique)
        for (auto & [ _, t ] : _imp->t_clique_neighbourhood) {
            for (auto & [ _, u ] : _imp->t_clique_neighbourhood) {
                if (t != u) {
                    *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }] << " 1 ~x" << _imp->variable_mappings[pair{ p.first, u.first }] << " >= 1 ;" << endl;
                    ++_imp->proof_line;
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, t }, pair{ p, u } }, _imp->proof_line);
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, u }, pair{ p, t } }, _imp->proof_line);
                }
            }
        }
}

auto Proof::finish_hom_clique_proof(const NamedVertex & p, const NamedVertex & t, unsigned size) -> void
{
    *_imp->proof_stream << "* end clique of size " << size << " around neighbourhood of " << p.second << " but not " << t.second << endl;
    *_imp->proof_stream << "# 0" << endl;
    *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }] << " >= 1 ;" << endl;
    *_imp->proof_stream << "w 1" << endl;
    ++_imp->proof_line;
    _imp->doing_hom_colour_proof = false;
    _imp->clique_for_hom_non_edge_constraints.clear();
}

auto Proof::add_hom_clique_non_edge(
        const NamedVertex & pp,
        const NamedVertex & tt,
        const std::vector<NamedVertex> & p_clique,
        const NamedVertex & t,
        const NamedVertex & u) -> void
{
    *_imp->proof_stream << "* hom clique non edges for " << t.second << " " << u.second << endl;
    for (auto & p : p_clique) {
        for (auto & q : p_clique) {
            if (p != q) {
                *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ pp.first, tt.first }]
                    << " 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }]
                    << " 1 ~x" << _imp->variable_mappings[pair{ q.first, u.first }] << " >= 1 ;" << endl;
                ++_imp->proof_line;
                _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, t }, pair{ q, u } }, _imp->proof_line);
                _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ q, u }, pair{ p, t } }, _imp->proof_line);
            }
        }
    }
}

auto Proof::not_connected_in_underlying_graph(const std::vector<int> & x, int y) -> void
{
    *_imp->proof_stream << "u 1 ~x" << _imp->binary_variable_mappings[y];
    for (auto & v : x)
        *_imp->proof_stream << " 1 ~x" << _imp->binary_variable_mappings[v];
    *_imp->proof_stream << " >= 1 ;" << endl;
    ++_imp->proof_line;
}

auto Proof::has_clique_model() const -> bool
{
    return _imp->clique_encoding;
}

auto Proof::create_clique_encoding(
        const vector<pair<int, int> > & enc,
        const vector<pair<int, int> > & zero_in_proof_objectives) -> void
{
    _imp->clique_encoding = true;
    for (unsigned i = 0 ; i < enc.size() ; ++i)
        _imp->binary_variable_mappings.emplace(i, _imp->variable_mappings[enc[i]]);

    _imp->zero_in_proof_objectives = zero_in_proof_objectives;
}

auto Proof::super_extra_verbose() const -> bool
{
    return _imp->super_extra_verbose;
}

auto Proof::show_domains(const string & s, const std::vector<std::pair<NamedVertex, std::vector<NamedVertex> > > & domains) -> void
{
    *_imp->proof_stream << "* " << s << ", domains follow" << endl;
    for (auto & [ p, ts ] : domains) {
        *_imp->proof_stream << "*    " << p.second << " size " << ts.size() << " = {";
        for (auto & t : ts)
            *_imp->proof_stream << " " << t.second;
        *_imp->proof_stream << " }" << endl;
    }
}

auto Proof::propagated(const NamedVertex & p, const NamedVertex & t, int g, int n_values, const NamedVertex & q) -> void
{
    *_imp->proof_stream << "* adjacency propagation from " << p.second << " -> " << t.second << " in graph pairs " << g << " deleted " << n_values << " values from " << q.second << endl;
}
#endif

#ifdef NEWLINE
struct Proof::Imp
{
    string opb_filename, log_filename;
    stringstream model_stream, model_prelude_stream;
    unique_ptr<ostream> proof_stream;
    bool friendly_names;
    bool bz2 = false;
    bool super_extra_verbose = false;

    map<pair<long, long>, string> variable_mappings;
    map<long, string> binary_variable_mappings;
    map<tuple<long, long, long>, string> connected_variable_mappings;
    map<tuple<long, long, long, long>, string> connected_variable_mappings_aux;
    map<long, long> at_least_one_value_constraints, at_most_one_value_constraints, injectivity_constraints;
    map<tuple<long, long, long, long>, long> adjacency_lines;
    map<pair<long, long>, long> eliminations;
    map<pair<long, long>, long> non_edge_constraints;
    long objective_line = 0;

    long nb_constraints = 0;
    long proof_line = 0;
    int largest_level_set = 0;

    bool clique_encoding = false;

    bool doing_hom_colour_proof = false;
    NamedVertex hom_colour_proof_p, hom_colour_proof_t;
    vector<NamedVertex> p_clique;
    map<int, NamedVertex> t_clique_neighbourhood;
    map<pair<pair<NamedVertex, NamedVertex>, pair<NamedVertex, NamedVertex> >, long> clique_for_hom_non_edge_constraints;

    vector<pair<int, int> > zero_in_proof_objectives;
};

Proof::Proof(const string & opb_file, const string & log_file, bool f, bool b, bool s) :
    _imp(new Imp)
{
    _imp->opb_filename = opb_file;
    _imp->log_filename = log_file;
    _imp->friendly_names = f;
    _imp->bz2 = b;
    _imp->super_extra_verbose = s;
}

Proof::Proof(Proof &&) = default;

Proof::~Proof() = default;

auto Proof::operator= (Proof &&) -> Proof & = default;

auto Proof::create_cp_variable(int pattern_vertex, int target_size,
        const function<auto (int) -> string> & pattern_name,
        const function<auto (int) -> string> & target_name) -> void
{
    for (int i = 0 ; i < target_size ; ++i)
        if (_imp->friendly_names)
            _imp->variable_mappings.emplace(pair{ pattern_vertex, i }, pattern_name(pattern_vertex) + "_" + target_name(i));
        else
            _imp->variable_mappings.emplace(pair{ pattern_vertex, i }, to_string(_imp->variable_mappings.size() + 1));

    _imp->model_stream << "* vertex " << pattern_vertex << " domain" << "\n";
    for (int i = 0 ; i < target_size ; ++i)
        _imp->model_stream << "1 x" << _imp->variable_mappings[{ pattern_vertex, i }] << " ";
    _imp->model_stream << ">= 1 ;" << "\n";
    _imp->at_least_one_value_constraints.emplace(pattern_vertex, ++_imp->nb_constraints);

    for (int i = 0 ; i < target_size ; ++i)
        _imp->model_stream << "-1 x" << _imp->variable_mappings[{ pattern_vertex, i }] << " ";
    _imp->model_stream << ">= -1 ;" << "\n";
    _imp->at_most_one_value_constraints.emplace(pattern_vertex, ++_imp->nb_constraints);
}

auto Proof::create_injectivity_constraints(int pattern_size, int target_size) -> void
{
    for (int v = 0 ; v < target_size ; ++v) {
        _imp->model_stream << "* injectivity on value " << v << "\n";

        for (int p = 0 ; p < pattern_size ; ++p) {
            auto x = _imp->variable_mappings.find(pair{ p, v });
            if (x != _imp->variable_mappings.end())
                _imp->model_stream << "-1 x" << x->second << " ";
        }
        _imp->model_stream << ">= -1 ;" << "\n";
        _imp->injectivity_constraints.emplace(v, ++_imp->nb_constraints);
    }
}

auto Proof::create_forbidden_assignment_constraint(int p, int t) -> void
{
    _imp->model_stream << "* forbidden assignment" << "\n";
    _imp->model_stream << "1 ~x" << _imp->variable_mappings[pair{ p, t }] << " >= 1 ;" << "\n";
    ++_imp->nb_constraints;
    _imp->eliminations.emplace(pair{ p, t }, _imp->nb_constraints);
}

auto Proof::start_adjacency_constraints_for(int p, int t) -> void
{
    _imp->model_stream << "* adjacency " << p << " maps to " << t << "\n";
}

auto Proof::create_adjacency_constraint(int p, int q, int t, const vector<int> & uu, bool) -> void
{
    _imp->model_stream << "1 ~x" << _imp->variable_mappings[pair{ p, t }];
    for (auto & u : uu)
        _imp->model_stream << " 1 x" << _imp->variable_mappings[pair{ q, u }];
    _imp->model_stream << " >= 1 ;" << "\n";
    _imp->adjacency_lines.emplace(tuple{ 0, p, q, t }, ++_imp->nb_constraints);
}

auto Proof::finalise_model() -> void
{
    unique_ptr<ostream> f = (_imp->bz2 ? make_compressed_ostream(_imp->opb_filename + ".bz2") : make_unique<ofstream>(_imp->opb_filename));

    *f << "* #variable= " << (_imp->variable_mappings.size() + _imp->binary_variable_mappings.size()
            + _imp->connected_variable_mappings.size() + _imp->connected_variable_mappings_aux.size())
        << " #constraint= " << _imp->nb_constraints << "\n";
    copy(istreambuf_iterator<char>{ _imp->model_prelude_stream }, istreambuf_iterator<char>{}, ostreambuf_iterator<char>{ *f });
    _imp->model_prelude_stream.clear();
    copy(istreambuf_iterator<char>{ _imp->model_stream }, istreambuf_iterator<char>{}, ostreambuf_iterator<char>{ *f });
    _imp->model_stream.clear();

    if (! *f)
        throw ProofError{ "Error writing opb file to '" + _imp->opb_filename + "'" };

    _imp->proof_stream = (_imp->bz2 ? make_compressed_ostream(_imp->log_filename + ".bz2") : make_unique<ofstream>(_imp->log_filename));

    *_imp->proof_stream << "pseudo-Boolean proof version 1.0" << "\n";

    *_imp->proof_stream << "f " << _imp->nb_constraints << " 0" << "\n";
    _imp->proof_line += _imp->nb_constraints;

    if (! *_imp->proof_stream)
        throw ProofError{ "Error writing proof file to '" + _imp->log_filename + "'" };
}

auto Proof::finish_unsat_proof() -> void
{
    *_imp->proof_stream << "* asserting that we've proved unsat" << "\n";
    *_imp->proof_stream << "u >= 1 ;" << "\n";
    ++_imp->proof_line;
    *_imp->proof_stream << "c " << _imp->proof_line << " 0" << "\n";
}

auto Proof::emit_hall_set_or_violator(const vector<NamedVertex> & lhs, const vector<NamedVertex> & rhs) -> void
{
    *_imp->proof_stream << "* hall set or violator {";
    for (auto & l : lhs)
        *_imp->proof_stream << " " << l.second;
    *_imp->proof_stream << " } / {";
    for (auto & r : rhs)
        *_imp->proof_stream << " " << r.second;
    *_imp->proof_stream << " }" << "\n";
    *_imp->proof_stream << "p";
    bool first = true;
    for (auto & l : lhs) {
        if (first) {
            first = false;
            *_imp->proof_stream << " " << _imp->at_least_one_value_constraints[l.first];
        }
        else
            *_imp->proof_stream << " " << _imp->at_least_one_value_constraints[l.first] << " +";
    }
    for (auto & r : rhs)
        *_imp->proof_stream << " " << _imp->injectivity_constraints[r.first] << " +";
    *_imp->proof_stream << " 0" << "\n";
    ++_imp->proof_line;
}

auto Proof::root_propagation_failed() -> void
{
    *_imp->proof_stream << "* root node propagation failed" << "\n";
}

auto Proof::guessing(int depth, const NamedVertex & branch_v, const NamedVertex & val) -> void
{
    *_imp->proof_stream << "* [" << depth << "] guessing " << branch_v.second << "=" << val.second << "\n";
}

auto Proof::propagation_failure(const vector<pair<int, int> > & decisions, const NamedVertex & branch_v, const NamedVertex & val) -> void
{
    *_imp->proof_stream << "* [" << decisions.size() << "] propagation failure on " << branch_v.second << "=" << val.second << "\n";
    *_imp->proof_stream << "u ";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " 1 ~x" << _imp->variable_mappings[pair{ var, val }];
    *_imp->proof_stream << " >= 1 ;" << "\n";
    ++_imp->proof_line;
}

auto Proof::incorrect_guess(const vector<pair<int, int> > & decisions, bool failure) -> void
{
    if (failure)
        *_imp->proof_stream << "* [" << decisions.size() << "] incorrect guess" << "\n";
    else
        *_imp->proof_stream << "* [" << decisions.size() << "] backtracking" << "\n";

    *_imp->proof_stream << "u";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " 1 ~x" << _imp->variable_mappings[pair{ var, val }];
    *_imp->proof_stream << " >= 1 ;" << "\n";
    ++_imp->proof_line;
}

auto Proof::out_of_guesses(const vector<pair<int, int> > &) -> void
{
}

auto Proof::unit_propagating(const NamedVertex & var, const NamedVertex & val) -> void
{
    *_imp->proof_stream << "* unit propagating " << var.second << "=" << val.second << "\n";
}

auto Proof::start_level(int l) -> void
{
    *_imp->proof_stream << "# " << l << "\n";
    _imp->largest_level_set = max(_imp->largest_level_set, l);
}

auto Proof::back_up_to_level(int l) -> void
{
    *_imp->proof_stream << "# " << l << "\n";
    _imp->largest_level_set = max(_imp->largest_level_set, l);
}

auto Proof::forget_level(int l) -> void
{
    if (_imp->largest_level_set >= l)
        *_imp->proof_stream << "w " << l << "\n";
}

auto Proof::back_up_to_top() -> void
{
    *_imp->proof_stream << "# " << 0 << "\n";
}

auto Proof::post_restart_nogood(const vector<pair<int, int> > & decisions) -> void
{
    *_imp->proof_stream << "* [" << decisions.size() << "] restart nogood" << "\n";
    *_imp->proof_stream << "u";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " 1 ~x" << _imp->variable_mappings[pair{ var, val }];
    *_imp->proof_stream << " >= 1 ;" << "\n";
    ++_imp->proof_line;
}

auto Proof::post_solution(const vector<pair<NamedVertex, NamedVertex> > & decisions) -> void
{
    *_imp->proof_stream << "* found solution";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " " << var.second << "=" << val.second;
    *_imp->proof_stream << "\n";

    *_imp->proof_stream << "v";
    for (auto & [ var, val ] : decisions)
        *_imp->proof_stream << " x" << _imp->variable_mappings[pair{ var.first, val.first }];
    *_imp->proof_stream << "\n";
    ++_imp->proof_line;
}

auto Proof::post_solution(const vector<int> & solution) -> void
{
    *_imp->proof_stream << "v";
    for (auto & v : solution)
        *_imp->proof_stream << " x" << _imp->binary_variable_mappings[v];
    *_imp->proof_stream << "\n";
    ++_imp->proof_line;
}

auto Proof::new_incumbent(const vector<pair<int, bool> > & solution) -> void
{
    *_imp->proof_stream << "o";
    for (auto & [ v, t ] : solution)
        *_imp->proof_stream << " " << (t ? "" : "~") << "x" << _imp->binary_variable_mappings[v];
    for (auto & [ v, w ] : _imp->zero_in_proof_objectives)
        *_imp->proof_stream << " ~" << "x" << _imp->variable_mappings[pair{ v, w }];
    *_imp->proof_stream << "\n";
    _imp->objective_line = ++_imp->proof_line;
}

auto Proof::new_incumbent(const vector<tuple<NamedVertex, NamedVertex, bool> > & decisions) -> void
{
    *_imp->proof_stream << "o";
    for (auto & [ var, val, t ] : decisions)
        *_imp->proof_stream << " " << (t ? "" : "~") << "x" << _imp->variable_mappings[pair{ var.first, val.first }];
    *_imp->proof_stream << "\n";
    _imp->objective_line = ++_imp->proof_line;
}

auto Proof::create_binary_variable(int vertex,
                const function<auto (int) -> string> & name) -> void
{
    if (_imp->friendly_names)
        _imp->binary_variable_mappings.emplace(vertex, name(vertex));
    else
        _imp->binary_variable_mappings.emplace(vertex, to_string(_imp->binary_variable_mappings.size() + 1));
}

auto Proof::create_objective(int n, optional<int> d) -> void
{
    if (d) {
        _imp->model_stream << "* objective" << "\n";
        for (int v = 0 ; v < n ; ++ v)
            _imp->model_stream << "1 x" << _imp->binary_variable_mappings[v] << " ";
        _imp->model_stream << ">= " << *d << ";" << "\n";
        _imp->objective_line = ++_imp->nb_constraints;
    }
    else {
        _imp->model_prelude_stream << "min:";
        for (int v = 0 ; v < n ; ++ v)
            _imp->model_prelude_stream << " -1 x" << _imp->binary_variable_mappings[v];
        _imp->model_prelude_stream << " ;" << "\n";
    }
}

auto Proof::create_non_edge_constraint(int p, int q) -> void
{
    _imp->model_stream << "-1 x" << _imp->binary_variable_mappings[p] << " -1 x" << _imp->binary_variable_mappings[q] << " >= -1 ;" << "\n";

    ++_imp->nb_constraints;
    _imp->non_edge_constraints.emplace(pair{ p, q }, _imp->nb_constraints);
    _imp->non_edge_constraints.emplace(pair{ q, p }, _imp->nb_constraints);
}

auto Proof::backtrack_from_binary_variables(const vector<int> & v) -> void
{
    if (! _imp->doing_hom_colour_proof) {
        *_imp->proof_stream << "u";
        for (auto & w : v)
            *_imp->proof_stream << " 1 ~x" << _imp->binary_variable_mappings[w];
        *_imp->proof_stream << " >= 1 ;" << "\n";
        ++_imp->proof_line;
    }
    else {
        *_imp->proof_stream << "* backtrack shenanigans, depth " << v.size() << "\n";
        function<auto (unsigned, const vector<pair<int, int> > &) -> void> f;
        f = [&] (unsigned d, const vector<pair<int, int> > & trail) -> void {
            if (d == v.size()) {
                *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ _imp->hom_colour_proof_p.first, _imp->hom_colour_proof_t.first }];
                for (auto & t : trail)
                    *_imp->proof_stream << " 1 ~x" << _imp->variable_mappings[t];
                *_imp->proof_stream << " >= 1 ;" << "\n";
                ++_imp->proof_line;
            }
            else {
                for (auto & p : _imp->p_clique) {
                    vector<pair<int, int> > new_trail{ trail };
                    new_trail.emplace_back(pair{ p.first, _imp->t_clique_neighbourhood.find(v[d])->second.first });
                    f(d + 1, new_trail);
                }
            }
        };
        f(0, {});
    }
}

auto Proof::colour_bound(const vector<vector<int> > & ccs) -> void
{
    *_imp->proof_stream << "* bound, ccs";
    for (auto & cc : ccs) {
        *_imp->proof_stream << " [";
        for (auto & c : cc)
            *_imp->proof_stream << " " << c;
        *_imp->proof_stream << " ]";
    }
    *_imp->proof_stream << "\n";

    vector<long> to_sum;
    auto do_one_cc = [&] (const auto & cc, const auto & non_edge_constraint) {
        if (cc.size() > 2) {
            *_imp->proof_stream << "p " << non_edge_constraint(cc[0], cc[1]);

            for (unsigned i = 2 ; i < cc.size() ; ++i) {
                *_imp->proof_stream << " " << i << " *";
                for (unsigned j = 0 ; j < i ; ++j)
                    *_imp->proof_stream << " " << non_edge_constraint(cc[i], cc[j]) << " +";
                *_imp->proof_stream << " " << (i + 1) << " d";
            }

            *_imp->proof_stream << "\n";
            to_sum.push_back(++_imp->proof_line);
        }
        else if (cc.size() == 2) {
            to_sum.push_back(non_edge_constraint(cc[0], cc[1]));
        }
    };

    for (auto & cc : ccs) {
        if (_imp->doing_hom_colour_proof) {
            vector<pair<NamedVertex, NamedVertex> > bigger_cc;
            for (auto & c : cc)
                for (auto & v : _imp->p_clique)
                    bigger_cc.push_back(pair{ v, _imp->t_clique_neighbourhood.find(c)->second });

            *_imp->proof_stream << "* colour class [";
            for (auto & c : bigger_cc)
                *_imp->proof_stream << " " << c.first.second << "/" << c.second.second;
            *_imp->proof_stream << " ]" << "\n";

            do_one_cc(bigger_cc, [&] (const pair<NamedVertex, NamedVertex> & a, const pair<NamedVertex, NamedVertex> & b) -> long {
                    return _imp->clique_for_hom_non_edge_constraints[pair{ a, b }];
                    });
        }
        else
            do_one_cc(cc, [&] (int a, int b) -> long { return _imp->non_edge_constraints[pair{ a, b }]; });

        *_imp->proof_stream << "p " << _imp->objective_line;
        for (auto & t : to_sum)
            *_imp->proof_stream << " " << t << " +";
        *_imp->proof_stream << "\n";
        ++_imp->proof_line;
    }
}

auto Proof::prepare_hom_clique_proof(const NamedVertex & p, const NamedVertex & t, unsigned size) -> void
{
    *_imp->proof_stream << "* clique of size " << size << " around neighbourhood of " << p.second << " but not " << t.second << "\n";
    *_imp->proof_stream << "# 1" << "\n";
    _imp->doing_hom_colour_proof = true;
    _imp->hom_colour_proof_p = p;
    _imp->hom_colour_proof_t = t;
}

auto Proof::start_hom_clique_proof(const NamedVertex & p, vector<NamedVertex> && p_clique, const NamedVertex & t, map<int, NamedVertex> && t_clique_neighbourhood) -> void
{
    _imp->p_clique = move(p_clique);
    _imp->t_clique_neighbourhood = move(t_clique_neighbourhood);

    *_imp->proof_stream << "* hom clique objective" << "\n";
    vector<long> to_sum;
    for (auto & q : _imp->p_clique) {
        *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }];
        for (auto & u : _imp->t_clique_neighbourhood)
            *_imp->proof_stream << " 1 x" << _imp->variable_mappings[pair{ q.first, u.second.first }];
        *_imp->proof_stream << " >= 1 ;" << "\n";
        to_sum.push_back(++_imp->proof_line);
    }

    *_imp->proof_stream << "p";
    bool first = true;
    for (auto & t : to_sum) {
        *_imp->proof_stream << " " << t;
        if (! first)
            *_imp->proof_stream << " +";
        first = false;
    }
    *_imp->proof_stream << "\n";
    _imp->objective_line = ++_imp->proof_line;

    *_imp->proof_stream << "* hom clique non edges for injectivity" << "\n";

    for (auto & p : _imp->p_clique)
        for (auto & q : _imp->p_clique)
            if (p != q) {
                for (auto & [ _, t ] : _imp->t_clique_neighbourhood) {
                    *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }] << " 1 ~x" << _imp->variable_mappings[pair{ q.first, t.first }] << " >= 1 ;" << "\n";
                    ++_imp->proof_line;
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, t }, pair{ q, t } }, _imp->proof_line);
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ q, t }, pair{ p, t } }, _imp->proof_line);
                }
            }

    *_imp->proof_stream << "* hom clique non edges for variables" << "\n";

    for (auto & p : _imp->p_clique)
        for (auto & [ _, t ] : _imp->t_clique_neighbourhood) {
            for (auto & [ _, u ] : _imp->t_clique_neighbourhood) {
                if (t != u) {
                    *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }] << " 1 ~x" << _imp->variable_mappings[pair{ p.first, u.first }] << " >= 1 ;" << "\n";
                    ++_imp->proof_line;
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, t }, pair{ p, u } }, _imp->proof_line);
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, u }, pair{ p, t } }, _imp->proof_line);
                }
            }
        }
}

auto Proof::finish_hom_clique_proof(const NamedVertex & p, const NamedVertex & t, unsigned size) -> void
{
    *_imp->proof_stream << "* end clique of size " << size << " around neighbourhood of " << p.second << " but not " << t.second << "\n";
    *_imp->proof_stream << "# 0" << "\n";
    *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }] << " >= 1 ;" << "\n";
    *_imp->proof_stream << "w 1" << "\n";
    ++_imp->proof_line;
    _imp->doing_hom_colour_proof = false;
    _imp->clique_for_hom_non_edge_constraints.clear();
}

auto Proof::add_hom_clique_non_edge(
        const NamedVertex & pp,
        const NamedVertex & tt,
        const std::vector<NamedVertex> & p_clique,
        const NamedVertex & t,
        const NamedVertex & u) -> void
{
    *_imp->proof_stream << "* hom clique non edges for " << t.second << " " << u.second << "\n";
    for (auto & p : p_clique) {
        for (auto & q : p_clique) {
            if (p != q) {
                *_imp->proof_stream << "u 1 ~x" << _imp->variable_mappings[pair{ pp.first, tt.first }]
                    << " 1 ~x" << _imp->variable_mappings[pair{ p.first, t.first }]
                    << " 1 ~x" << _imp->variable_mappings[pair{ q.first, u.first }] << " >= 1 ;" << "\n";
                ++_imp->proof_line;
                _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, t }, pair{ q, u } }, _imp->proof_line);
                _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ q, u }, pair{ p, t } }, _imp->proof_line);
            }
        }
    }
}

auto Proof::not_connected_in_underlying_graph(const std::vector<int> & x, int y) -> void
{
    *_imp->proof_stream << "u 1 ~x" << _imp->binary_variable_mappings[y];
    for (auto & v : x)
        *_imp->proof_stream << " 1 ~x" << _imp->binary_variable_mappings[v];
    *_imp->proof_stream << " >= 1 ;" << "\n";
    ++_imp->proof_line;
}

auto Proof::has_clique_model() const -> bool
{
    return _imp->clique_encoding;
}

auto Proof::create_clique_encoding(
        const vector<pair<int, int> > & enc,
        const vector<pair<int, int> > & zero_in_proof_objectives) -> void
{
    _imp->clique_encoding = true;
    for (unsigned i = 0 ; i < enc.size() ; ++i)
        _imp->binary_variable_mappings.emplace(i, _imp->variable_mappings[enc[i]]);

    _imp->zero_in_proof_objectives = zero_in_proof_objectives;
}

auto Proof::super_extra_verbose() const -> bool
{
    return _imp->super_extra_verbose;
}

auto Proof::show_domains(const string & s, const std::vector<std::pair<NamedVertex, std::vector<NamedVertex> > > & domains) -> void
{
    *_imp->proof_stream << "* " << s << ", domains follow" << "\n";
    for (auto & [ p, ts ] : domains) {
        *_imp->proof_stream << "*    " << p.second << " size " << ts.size() << " = {";
        for (auto & t : ts)
            *_imp->proof_stream << " " << t.second;
        *_imp->proof_stream << " }" << "\n";
    }
}

auto Proof::propagated(const NamedVertex & p, const NamedVertex & t, int g, int n_values, const NamedVertex & q) -> void
{
    *_imp->proof_stream << "* adjacency propagation from " << p.second << " -> " << t.second << " in graph pairs " << g << " deleted " << n_values << " values from " << q.second << "\n";
}
#endif

#ifdef FMT
struct Proof::Imp
{
    string opb_filename, log_filename;
    stringstream model_stream, model_prelude_stream;
    /* fmt::ostream proof_stream; */
    FILE* proof_file;
    bool friendly_names;
    bool bz2 = false;
    bool super_extra_verbose = false;

    map<pair<long, long>, string> variable_mappings;
    map<long, string> binary_variable_mappings;
    map<tuple<long, long, long>, string> connected_variable_mappings;
    map<tuple<long, long, long, long>, string> connected_variable_mappings_aux;
    map<long, long> at_least_one_value_constraints, at_most_one_value_constraints, injectivity_constraints;
    map<tuple<long, long, long, long>, long> adjacency_lines;
    map<pair<long, long>, long> eliminations;
    map<pair<long, long>, long> non_edge_constraints;
    long objective_line = 0;

    long nb_constraints = 0;
    long proof_line = 0;
    int largest_level_set = 0;

    bool clique_encoding = false;

    bool doing_hom_colour_proof = false;
    NamedVertex hom_colour_proof_p, hom_colour_proof_t;
    vector<NamedVertex> p_clique;
    map<int, NamedVertex> t_clique_neighbourhood;
    map<pair<pair<NamedVertex, NamedVertex>, pair<NamedVertex, NamedVertex> >, long> clique_for_hom_non_edge_constraints;

    vector<pair<int, int> > zero_in_proof_objectives;
};

Proof::Proof(const string & opb_file, const string & log_file, bool f, bool b, bool s) :
    _imp(new Imp)
{
    _imp->opb_filename = opb_file;
    _imp->log_filename = log_file;
    _imp->friendly_names = f;
    _imp->bz2 = b;
    _imp->super_extra_verbose = s;
    _imp->proof_file = fopen(log_file.c_str(),"w");
    /* auto stream = fmt::output_file(log_file); */
    /* _imp->proof_stream = fmt::output_file(log_file); */
}

Proof::Proof(Proof &&) = default;

Proof::~Proof() = default;

auto Proof::operator= (Proof &&) -> Proof & = default;

auto Proof::create_cp_variable(int pattern_vertex, int target_size,
        const function<auto (int) -> string> & pattern_name,
        const function<auto (int) -> string> & target_name) -> void
{
    for (int i = 0 ; i < target_size ; ++i)
        if (_imp->friendly_names)
            _imp->variable_mappings.emplace(pair{ pattern_vertex, i }, pattern_name(pattern_vertex) + "_" + target_name(i));
        else
            _imp->variable_mappings.emplace(pair{ pattern_vertex, i }, to_string(_imp->variable_mappings.size() + 1));

    _imp->model_stream << "* vertex " << pattern_vertex << " domain" << endl;
    for (int i = 0 ; i < target_size ; ++i)
        _imp->model_stream << "1 x" << _imp->variable_mappings[{ pattern_vertex, i }] << " ";
    _imp->model_stream << ">= 1 ;" << endl;
    _imp->at_least_one_value_constraints.emplace(pattern_vertex, ++_imp->nb_constraints);

    for (int i = 0 ; i < target_size ; ++i)
        _imp->model_stream << "-1 x" << _imp->variable_mappings[{ pattern_vertex, i }] << " ";
    _imp->model_stream << ">= -1 ;" << endl;
    _imp->at_most_one_value_constraints.emplace(pattern_vertex, ++_imp->nb_constraints);
}

auto Proof::create_injectivity_constraints(int pattern_size, int target_size) -> void
{
    for (int v = 0 ; v < target_size ; ++v) {
        _imp->model_stream << "* injectivity on value " << v << endl;

        for (int p = 0 ; p < pattern_size ; ++p) {
            auto x = _imp->variable_mappings.find(pair{ p, v });
            if (x != _imp->variable_mappings.end())
                _imp->model_stream << "-1 x" << x->second << " ";
        }
        _imp->model_stream << ">= -1 ;" << endl;
        _imp->injectivity_constraints.emplace(v, ++_imp->nb_constraints);
    }
}

auto Proof::create_forbidden_assignment_constraint(int p, int t) -> void
{
    _imp->model_stream << "* forbidden assignment" << endl;
    _imp->model_stream << "1 ~x" << _imp->variable_mappings[pair{ p, t }] << " >= 1 ;" << endl;
    ++_imp->nb_constraints;
    _imp->eliminations.emplace(pair{ p, t }, _imp->nb_constraints);
}

auto Proof::start_adjacency_constraints_for(int p, int t) -> void
{
    _imp->model_stream << "* adjacency " << p << " maps to " << t << endl;
}

auto Proof::create_adjacency_constraint(int p, int q, int t, const vector<int> & uu, bool) -> void
{
    _imp->model_stream << "1 ~x" << _imp->variable_mappings[pair{ p, t }];
    for (auto & u : uu)
        _imp->model_stream << " 1 x" << _imp->variable_mappings[pair{ q, u }];
    _imp->model_stream << " >= 1 ;" << endl;
    _imp->adjacency_lines.emplace(tuple{ 0, p, q, t }, ++_imp->nb_constraints);
}

auto Proof::finalise_model() -> void
{
    unique_ptr<ostream> f = /* fmt::output_file(_imp->log_filename) */ (_imp->bz2 ? make_compressed_ostream(_imp->opb_filename + ".bz2") : make_unique<ofstream>(_imp->opb_filename));

    *f << "* #variable= " << (_imp->variable_mappings.size() + _imp->binary_variable_mappings.size()
            + _imp->connected_variable_mappings.size() + _imp->connected_variable_mappings_aux.size())
        << " #constraint= " << _imp->nb_constraints << endl;
    copy(istreambuf_iterator<char>{ _imp->model_prelude_stream }, istreambuf_iterator<char>{}, ostreambuf_iterator<char>{ *f });
    _imp->model_prelude_stream.clear();
    copy(istreambuf_iterator<char>{ _imp->model_stream }, istreambuf_iterator<char>{}, ostreambuf_iterator<char>{ *f });
    _imp->model_stream.clear();

    if (! *f)
        throw ProofError{ "Error writing opb file to '" + _imp->opb_filename + "'" };

    /* _imp->proof_stream = fmt::output_file(_imp->log_filename); (_imp->bz2 ? make_compressed_ostream(_imp->log_filename + ".bz2") : make_unique<ofstream>(_imp->log_filename)); */

    fmt::println(_imp->proof_file, "pseudo-Boolean proof version 1.0");

    fmt::println(_imp->proof_file, "f {} 0",_imp->nb_constraints);
    _imp->proof_line += _imp->nb_constraints;

    /* if (! *_imp->proof_stream)
        throw ProofError{ "Error writing proof file to '" + _imp->log_filename + "'" }; */
}

auto Proof::finish_unsat_proof() -> void
{
    fmt::println(_imp->proof_file, "* asserting that we've proved unsat");
    fmt::println(_imp->proof_file, "u >= 1 ;");
    ++_imp->proof_line;
    fmt::println(_imp->proof_file, "c {} 0", _imp->proof_line);
}

auto Proof::emit_hall_set_or_violator(const vector<NamedVertex> & lhs, const vector<NamedVertex> & rhs) -> void
{
    fmt::print(_imp->proof_file, "* hall set or violator {");
    for (auto & l : lhs)
        fmt::print(_imp->proof_file, " {}", l.second);
    fmt::print(_imp->proof_file, " } / {");
    for (auto & r : rhs)
        fmt::print(_imp->proof_file, " {}", r.second);
    fmt::println(_imp->proof_file, " }");
    fmt::print(_imp->proof_file, "p");
    bool first = true;
    for (auto & l : lhs) {
        if (first) {
            first = false;
            fmt::println(_imp->proof_file, " {}", _imp->at_least_one_value_constraints[l.first]);
        }
        else
            fmt::println(_imp->proof_file, " {} +", _imp->at_least_one_value_constraints[l.first]);
    }
    for (auto & r : rhs)
        fmt::print(_imp->proof_file, " {} +", _imp->injectivity_constraints[r.first]);
    fmt::println(_imp->proof_file, " 0");
    ++_imp->proof_line;
}

auto Proof::root_propagation_failed() -> void
{
    fmt::println(_imp->proof_file, "* root node propagation failed");
}

auto Proof::guessing(int depth, const NamedVertex & branch_v, const NamedVertex & val) -> void
{
    fmt::println(_imp->proof_file, "* [{}] guessing {}={}", depth, branch_v.second, val.second);
}

auto Proof::propagation_failure(const vector<pair<int, int> > & decisions, const NamedVertex & branch_v, const NamedVertex & val) -> void
{
    fmt::println(_imp->proof_file, "* [{}] propagation failure on {}={}", decisions.size(), branch_v.second, val.second);
    fmt::print(_imp->proof_file, "u ");
    for (auto & [ var, val ] : decisions)
        fmt::print(_imp->proof_file, " 1 ~x{}", _imp->variable_mappings[pair{ var, val }]);
    fmt::println(_imp->proof_file, " >= 1 ;");
    ++_imp->proof_line;
}

auto Proof::incorrect_guess(const vector<pair<int, int> > & decisions, bool failure) -> void
{
    if (failure)
        fmt::println(_imp->proof_file, "* [{}] incorrect guess", decisions.size());
    else
        fmt::println(_imp->proof_file, "* [{}] backtracking", decisions.size());

    fmt::print(_imp->proof_file, "u");
    for (auto & [ var, val ] : decisions)
        fmt::print(_imp->proof_file, " 1 ~x{}", _imp->variable_mappings[pair{ var, val }]);
    fmt::println(_imp->proof_file, " >= 1 ;");
    ++_imp->proof_line;
}

auto Proof::out_of_guesses(const vector<pair<int, int> > &) -> void
{
}

auto Proof::unit_propagating(const NamedVertex & var, const NamedVertex & val) -> void
{
    fmt::println(_imp->proof_file, "* unit propagating {}={}", var.second, val.second);
}

auto Proof::start_level(int l) -> void
{
    fmt::println(_imp->proof_file, "# {}", l);
    _imp->largest_level_set = max(_imp->largest_level_set, l);
}

auto Proof::back_up_to_level(int l) -> void
{
    fmt::println(_imp->proof_file, "# {}", l);
    _imp->largest_level_set = max(_imp->largest_level_set, l);
}

auto Proof::forget_level(int l) -> void
{
    if (_imp->largest_level_set >= l)
        fmt::println(_imp->proof_file, "w {}", l);
}

auto Proof::back_up_to_top() -> void
{
    fmt::println(_imp->proof_file, "# {}", 0);
}

auto Proof::post_restart_nogood(const vector<pair<int, int> > & decisions) -> void
{
    fmt::println(_imp->proof_file, "* [{}] restart nogood", decisions.size());
    fmt::print(_imp->proof_file, "u");
    for (auto & [ var, val ] : decisions)
        fmt::print(_imp->proof_file, " 1 ~x{}", _imp->variable_mappings[pair{ var, val }]);
    fmt::println(_imp->proof_file, " >= 1 ;");
    ++_imp->proof_line;
}

auto Proof::post_solution(const vector<pair<NamedVertex, NamedVertex> > & decisions) -> void
{
    fmt::print(_imp->proof_file, "* found solution");
    for (auto & [ var, val ] : decisions)
        fmt::print(_imp->proof_file, " {}={}", var.second, val.second);
    fmt::println(_imp->proof_file, "");

    fmt::print(_imp->proof_file, "v");
    for (auto & [ var, val ] : decisions)
        fmt::print(_imp->proof_file, " x{}", _imp->variable_mappings[pair{ var.first, val.first }]);
    fmt::println(_imp->proof_file, "");
    ++_imp->proof_line;
}

auto Proof::post_solution(const vector<int> & solution) -> void
{
    fmt::print(_imp->proof_file, "v");
    for (auto & v : solution)
        fmt::print(_imp->proof_file, " x", _imp->binary_variable_mappings[v]);
    fmt::println(_imp->proof_file, "");
    ++_imp->proof_line;
}

auto Proof::new_incumbent(const vector<pair<int, bool> > & solution) -> void
{
    fmt::print(_imp->proof_file, "o");
    for (auto & [ v, t ] : solution)
        fmt::print(_imp->proof_file, " {}x{}", (t ? "" : "~"), _imp->binary_variable_mappings[v]);
    for (auto & [ v, w ] : _imp->zero_in_proof_objectives)
        fmt::print(_imp->proof_file, " ~x{}", _imp->variable_mappings[pair{ v, w }]);
    fmt::println(_imp->proof_file, "");
    _imp->objective_line = ++_imp->proof_line;
}

auto Proof::new_incumbent(const vector<tuple<NamedVertex, NamedVertex, bool> > & decisions) -> void
{
    fmt::print(_imp->proof_file, "o");
    for (auto & [ var, val, t ] : decisions)
        fmt::print(_imp->proof_file, " {}x{}", (t ? "" : "~"), _imp->variable_mappings[pair{ var.first, val.first }]);
    fmt::println(_imp->proof_file, "");
    _imp->objective_line = ++_imp->proof_line;
}

auto Proof::create_binary_variable(int vertex,
                const function<auto (int) -> string> & name) -> void
{
    if (_imp->friendly_names)
        _imp->binary_variable_mappings.emplace(vertex, name(vertex));
    else
        _imp->binary_variable_mappings.emplace(vertex, to_string(_imp->binary_variable_mappings.size() + 1));
}

auto Proof::create_objective(int n, optional<int> d) -> void
{
    if (d) {
        _imp->model_stream << "* objective" << endl;
        for (int v = 0 ; v < n ; ++ v)
            _imp->model_stream << "1 x" << _imp->binary_variable_mappings[v] << " ";
        _imp->model_stream << ">= " << *d << ";" << endl;
        _imp->objective_line = ++_imp->nb_constraints;
    }
    else {
        _imp->model_prelude_stream << "min:";
        for (int v = 0 ; v < n ; ++ v)
            _imp->model_prelude_stream << " -1 x" << _imp->binary_variable_mappings[v];
        _imp->model_prelude_stream << " ;" << endl;
    }
}

auto Proof::create_non_edge_constraint(int p, int q) -> void
{
    _imp->model_stream << "-1 x" << _imp->binary_variable_mappings[p] << " -1 x" << _imp->binary_variable_mappings[q] << " >= -1 ;" << endl;

    ++_imp->nb_constraints;
    _imp->non_edge_constraints.emplace(pair{ p, q }, _imp->nb_constraints);
    _imp->non_edge_constraints.emplace(pair{ q, p }, _imp->nb_constraints);
}

auto Proof::backtrack_from_binary_variables(const vector<int> & v) -> void
{
    if (! _imp->doing_hom_colour_proof) {
        fmt::print(_imp->proof_file, "u");
        for (auto & w : v)
            fmt::print(_imp->proof_file, " 1 ~x{}", _imp->binary_variable_mappings[w]);
        fmt::println(_imp->proof_file, " >= 1 ;");
        ++_imp->proof_line;
    }
    else {
        fmt::println(_imp->proof_file, "* backtrack shenanigans, depth {}", v.size());
        function<auto (unsigned, const vector<pair<int, int> > &) -> void> f;
        f = [&] (unsigned d, const vector<pair<int, int> > & trail) -> void {
            if (d == v.size()) {
                fmt::print(_imp->proof_file, "u 1 ~x{}", _imp->variable_mappings[pair{ _imp->hom_colour_proof_p.first, _imp->hom_colour_proof_t.first }]);
                for (auto & t : trail)
                    fmt::print(_imp->proof_file, " 1 ~x{}", _imp->variable_mappings[t]);
                fmt::println(_imp->proof_file, " >= 1 ;");
                ++_imp->proof_line;
            }
            else {
                for (auto & p : _imp->p_clique) {
                    vector<pair<int, int> > new_trail{ trail };
                    new_trail.emplace_back(pair{ p.first, _imp->t_clique_neighbourhood.find(v[d])->second.first });
                    f(d + 1, new_trail);
                }
            }
        };
        f(0, {});
    }
}

auto Proof::colour_bound(const vector<vector<int> > & ccs) -> void
{
    fmt::print(_imp->proof_file, "* bound, ccs");
    for (auto & cc : ccs) {
        fmt::print(_imp->proof_file, " [");
        for (auto & c : cc)
            fmt::print(_imp->proof_file, " {}", c);
        fmt::print(_imp->proof_file, " ]");
    }
    fmt::println(_imp->proof_file, "");

    vector<long> to_sum;
    auto do_one_cc = [&] (const auto & cc, const auto & non_edge_constraint) {
        if (cc.size() > 2) {
            fmt::print(_imp->proof_file, "p {}", non_edge_constraint(cc[0], cc[1]));

            for (unsigned i = 2 ; i < cc.size() ; ++i) {
                fmt::print(_imp->proof_file, " {} *", i);
                for (unsigned j = 0 ; j < i ; ++j)
                    fmt::print(_imp->proof_file, " {} +", non_edge_constraint(cc[i], cc[j]));
                fmt::print(_imp->proof_file, " {} d", (i + 1));
            }

            fmt::println(_imp->proof_file, "");
            to_sum.push_back(++_imp->proof_line);
        }
        else if (cc.size() == 2) {
            to_sum.push_back(non_edge_constraint(cc[0], cc[1]));
        }
    };

    for (auto & cc : ccs) {
        if (_imp->doing_hom_colour_proof) {
            vector<pair<NamedVertex, NamedVertex> > bigger_cc;
            for (auto & c : cc)
                for (auto & v : _imp->p_clique)
                    bigger_cc.push_back(pair{ v, _imp->t_clique_neighbourhood.find(c)->second });

            fmt::print(_imp->proof_file, "* colour class [");
            for (auto & c : bigger_cc)
                fmt::print(_imp->proof_file, " {}/{}", c.first.second, c.second.second);
            fmt::println(_imp->proof_file, " ]");

            do_one_cc(bigger_cc, [&] (const pair<NamedVertex, NamedVertex> & a, const pair<NamedVertex, NamedVertex> & b) -> long {
                    return _imp->clique_for_hom_non_edge_constraints[pair{ a, b }];
                    });
        }
        else
            do_one_cc(cc, [&] (int a, int b) -> long { return _imp->non_edge_constraints[pair{ a, b }]; });

        fmt::print(_imp->proof_file, "p {}", _imp->objective_line);
        for (auto & t : to_sum)
            fmt::print(_imp->proof_file, " {} +", t);
        fmt::println(_imp->proof_file, "");
        ++_imp->proof_line;
    }
}

auto Proof::prepare_hom_clique_proof(const NamedVertex & p, const NamedVertex & t, unsigned size) -> void
{
    fmt::println(_imp->proof_file, "* clique of size {} around neighbourhood of {} but not {}", size, p.second, t.second);
    fmt::println(_imp->proof_file, "# 1");
    _imp->doing_hom_colour_proof = true;
    _imp->hom_colour_proof_p = p;
    _imp->hom_colour_proof_t = t;
}

auto Proof::start_hom_clique_proof(const NamedVertex & p, vector<NamedVertex> && p_clique, const NamedVertex & t, map<int, NamedVertex> && t_clique_neighbourhood) -> void
{
    _imp->p_clique = move(p_clique);
    _imp->t_clique_neighbourhood = move(t_clique_neighbourhood);

    fmt::println(_imp->proof_file, "* hom clique objective");
    vector<long> to_sum;
    for (auto & q : _imp->p_clique) {
        fmt::print(_imp->proof_file, "u 1 ~x{}", _imp->variable_mappings[pair{ p.first, t.first }]);
        for (auto & u : _imp->t_clique_neighbourhood)
            fmt::print(_imp->proof_file, " 1 x{}", _imp->variable_mappings[pair{ q.first, u.second.first }]);
        fmt::println(_imp->proof_file, " >= 1 ;");
        to_sum.push_back(++_imp->proof_line);
    }

    fmt::print(_imp->proof_file, "p");
    bool first = true;
    for (auto & t : to_sum) {
        fmt::print(_imp->proof_file, " {}", t);
        if (! first)
            fmt::print(_imp->proof_file, " +");
        first = false;
    }
    fmt::println(_imp->proof_file, "");
    _imp->objective_line = ++_imp->proof_line;

    fmt::println(_imp->proof_file, "* hom clique non edges for injectivity");

    for (auto & p : _imp->p_clique)
        for (auto & q : _imp->p_clique)
            if (p != q) {
                for (auto & [ _, t ] : _imp->t_clique_neighbourhood) {
                    fmt::println(_imp->proof_file, "u 1 ~x{} 1 ~x{} >= 1 ;", _imp->variable_mappings[pair{ p.first, t.first }], _imp->variable_mappings[pair{ q.first, t.first }]);
                    ++_imp->proof_line;
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, t }, pair{ q, t } }, _imp->proof_line);
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ q, t }, pair{ p, t } }, _imp->proof_line);
                }
            }

    fmt::println(_imp->proof_file, "* hom clique non edges for variables");

    for (auto & p : _imp->p_clique)
        for (auto & [ _, t ] : _imp->t_clique_neighbourhood) {
            for (auto & [ _, u ] : _imp->t_clique_neighbourhood) {
                if (t != u) {
                    fmt::println(_imp->proof_file, "u 1 ~x{} 1 ~x{} >= 1 ;", _imp->variable_mappings[pair{ p.first, t.first }], _imp->variable_mappings[pair{ p.first, u.first }]);
                    ++_imp->proof_line;
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, t }, pair{ p, u } }, _imp->proof_line);
                    _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, u }, pair{ p, t } }, _imp->proof_line);
                }
            }
        }
}

auto Proof::finish_hom_clique_proof(const NamedVertex & p, const NamedVertex & t, unsigned size) -> void
{
    fmt::println(_imp->proof_file, "* end clique of size {} around neighbourhood of {} but not {}", size, p.second, t.second);
    fmt::println(_imp->proof_file, "# 0");
    fmt::println(_imp->proof_file, "u 1 ~x{} >= 1 ;", _imp->variable_mappings[pair{ p.first, t.first }]);
    fmt::println(_imp->proof_file, "w 1");
    ++_imp->proof_line;
    _imp->doing_hom_colour_proof = false;
    _imp->clique_for_hom_non_edge_constraints.clear();
}

auto Proof::add_hom_clique_non_edge(
        const NamedVertex & pp,
        const NamedVertex & tt,
        const std::vector<NamedVertex> & p_clique,
        const NamedVertex & t,
        const NamedVertex & u) -> void
{
    fmt::println(_imp->proof_file, "* hom clique non edges for {} {}", t.second, u.second);
    for (auto & p : p_clique) {
        for (auto & q : p_clique) {
            if (p != q) {
                fmt::println(_imp->proof_file, "u 1 ~x{} 1 ~x{} 1 ~x{} >= 1 ;", _imp->variable_mappings[pair{ pp.first, tt.first }], _imp->variable_mappings[pair{ p.first, t.first }], _imp->variable_mappings[pair{ q.first, u.first }]);
                ++_imp->proof_line;
                _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ p, t }, pair{ q, u } }, _imp->proof_line);
                _imp->clique_for_hom_non_edge_constraints.emplace(pair{ pair{ q, u }, pair{ p, t } }, _imp->proof_line);
            }
        }
    }
}

auto Proof::not_connected_in_underlying_graph(const std::vector<int> & x, int y) -> void
{
    fmt::print(_imp->proof_file, "u 1 ~x{}", _imp->binary_variable_mappings[y]);
    for (auto & v : x)
        fmt::print(_imp->proof_file, " 1 ~x{}", _imp->binary_variable_mappings[v]);
    fmt::println(_imp->proof_file, " >= 1 ;");
    ++_imp->proof_line;
}

auto Proof::has_clique_model() const -> bool
{
    return _imp->clique_encoding;
}

auto Proof::create_clique_encoding(
        const vector<pair<int, int> > & enc,
        const vector<pair<int, int> > & zero_in_proof_objectives) -> void
{
    _imp->clique_encoding = true;
    for (unsigned i = 0 ; i < enc.size() ; ++i)
        _imp->binary_variable_mappings.emplace(i, _imp->variable_mappings[enc[i]]);

    _imp->zero_in_proof_objectives = zero_in_proof_objectives;
}

auto Proof::super_extra_verbose() const -> bool
{
    return _imp->super_extra_verbose;
}

auto Proof::show_domains(const string & s, const std::vector<std::pair<NamedVertex, std::vector<NamedVertex> > > & domains) -> void
{
    fmt::println(_imp->proof_file, "* {} domains follow", s);
    for (auto & [ p, ts ] : domains) {
        fmt::print(_imp->proof_file, "*    {} size {} = {", p.second, ts.size());
        for (auto & t : ts)
            fmt::print(_imp->proof_file, " {}", t.second);
        fmt::println(_imp->proof_file, " }");
    }
}

auto Proof::propagated(const NamedVertex & p, const NamedVertex & t, int g, int n_values, const NamedVertex & q) -> void
{
    fmt::println(_imp->proof_file, "* adjacency propagation from {} -> {} in graph pairs {} deleted {} values from {}", p.second, t.second, g, n_values, q.second);
}
#endif