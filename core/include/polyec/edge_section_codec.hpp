#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the Edge count and records in the existing Realm save token order.
inline void write_edge_section(std::ostream& out,
                              const std::vector<Edge>& edges) {
    out << edges.size() << ' ';
    for (const auto& e : edges) out << e.from << ' ' << e.to << ' ' << static_cast<unsigned>(e.kind) << ' ';
}

// Reads into a temporary vector. Realm validates references and the complete
// world after all remaining sections have been parsed.
inline void read_edge_section(std::istream& in, std::vector<Edge>& edges, int schema=22) {
    size_t count = 0;
    in >> count;
    if (!in || count > 8000000) throw std::runtime_error("Save count limit");

    std::vector<Edge> staged(count);
    for (auto& e : staged) {
        int kind = -1;
        if (!(in >> e.from >> e.to >> kind))
            throw std::runtime_error("Save count limit");
        if (kind < static_cast<int>(RelationKind::ParentChild) ||
            kind > static_cast<int>(schema >= 23 ? RelationKind::Spouse : RelationKind::Agreement))
            throw std::runtime_error("Save relation kind");
        e.kind = static_cast<RelationKind>(kind);
    }
    edges = std::move(staged);
}

} // namespace detail
} // namespace polyec
