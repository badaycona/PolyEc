#pragma once

#include "polyec/realm.hpp"

#include <istream>
#include <ostream>
#include <set>
#include <string>
#include <stdexcept>
#include <vector>

namespace polyec::detail {

inline void write_parentage_section(std::ostream& out, const std::vector<Edge>& edges) {
    size_t count = 0;
    for (const auto& edge : edges) count += edge.legal_parentage ? 1 : 0;
    out << " PARENTAGE_1 " << count << ' ';
    for (size_t i = 0; i < edges.size(); ++i)
        if (edges[i].legal_parentage) out << (i + 1) << ' ';
}

inline void read_parentage_section(std::istream& in, std::vector<Edge>& edges) {
    std::string tag;
    size_t count = 0;
    if (!(in >> tag >> count) || tag != "PARENTAGE_1" || count > edges.size())
        throw std::runtime_error("Save parentage header");
    std::vector<size_t> indices(count);
    size_t previous = 0;
    for (auto& index : indices) {
        if (!(in >> index) || index == 0 || index <= previous || index > edges.size())
            throw std::runtime_error("Save parentage index");
        const auto& edge = edges[index - 1];
        if (edge.kind != RelationKind::ParentChild || edge.from == edge.to)
            throw std::runtime_error("Save parentage relation");
        previous = index;
    }
    for (auto& edge : edges) edge.legal_parentage = false;
    for (const auto index : indices) edges[index - 1].legal_parentage = true;
}

inline void migrate_legacy_parentage(std::vector<Edge>& edges, const std::vector<Fact>& facts) {
    std::set<std::pair<uint64_t,uint64_t>> witnessed_births;
    for (const auto& fact : facts)
        if (fact.kind == 105 && fact.actor != 0 && fact.target != 0)
            witnessed_births.emplace(fact.actor, fact.target);
    for (auto& edge : edges)
        edge.legal_parentage = edge.kind == RelationKind::ParentChild && edge.from != edge.to &&
            witnessed_births.contains({edge.from, edge.to});
}

} // namespace polyec::detail
