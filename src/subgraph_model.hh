/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_SUBGRAPH_SOLVER_GUARD_SRC_SUBGRAPH_MODEL_HH
#define GLASGOW_SUBGRAPH_SOLVER_GUARD_SRC_SUBGRAPH_MODEL_HH 1

#include "formats/input_graph.hh"
#include "svo_bitset.hh"
#include "homomorphism.hh"
#include "proof.hh"

#include <memory>

class SubgraphModel
{
    private:
        struct Imp;
        std::unique_ptr<Imp> _imp;

        auto _build_exact_path_graphs(std::vector<SVOBitset> & graph_rows, unsigned size, unsigned & idx,
                unsigned number_of_exact_path_graphs) -> void;

        auto _build_distance3_graphs(std::vector<SVOBitset> & graph_rows, unsigned size, unsigned & idx) -> void;

        auto _build_k4_graphs(std::vector<SVOBitset> & graph_rows, unsigned size, unsigned & idx) -> void;

    public:
        using PatternAdjacencyBitsType = uint8_t;

        const unsigned max_graphs;
        unsigned pattern_size, target_size;

        auto has_less_thans() const -> bool;
        std::vector<std::pair<unsigned, unsigned> > pattern_less_thans_in_convenient_order;

        SubgraphModel(const InputGraph & target, const InputGraph & pattern, const HomomorphismParams & params);
        ~SubgraphModel();

        auto pattern_vertex_for_proof(int v) const -> NamedVertex;
        auto target_vertex_for_proof(int v) const -> NamedVertex;

        auto prepare(const HomomorphismParams & params) -> bool;

        auto pattern_adjacency_bits(int p, int q) const -> PatternAdjacencyBitsType;
        auto pattern_graph_row(int g, int p) const -> const SVOBitset &;
        auto target_graph_row(int g, int t) const -> const SVOBitset &;

        auto pattern_degree(int g, int p) const -> unsigned;
        auto target_degree(int g, int t) const -> unsigned;
        auto largest_target_degree() const -> unsigned;

        auto has_vertex_labels() const -> bool;
        auto has_edge_labels() const -> bool;
        auto pattern_vertex_label(int p) const -> int;
        auto target_vertex_label(int p) const -> int;
        auto pattern_edge_label(int p, int q) const -> int;
        auto target_edge_label(int t, int u) const -> int;

        auto pattern_has_loop(int p) const -> bool;
        auto target_has_loop(int t) const -> bool;
};

#endif
