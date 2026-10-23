#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the primary Proposal count and records in the existing Realm save token order.
// Court-owned extension fields remain in write_court.
inline void write_proposal_section(std::ostream& out,
                                  const std::vector<Proposal>& proposals) {
    out << proposals.size() << ' ';
    for (const auto& p : proposals) {
        out << p.id << ' ' << p.from << ' ' << p.to << ' ' << p.amount << ' '
            << p.due << ' ' << p.resolved << ' ';
    }
}

// Reads into a temporary vector. Realm validates references and the complete
// world after all remaining sections have been parsed. A truncated primary
// record preserves the downstream "Save court fields" error.
inline void read_proposal_section(std::istream& in,
                                 std::vector<Proposal>& proposals) {
    size_t count = 0;
    in >> count;
    if (!in || count > 1000000) throw std::runtime_error("Save count limit");

    std::vector<Proposal> staged(count);
    for (auto& p : staged) {
        if (!(in >> p.id >> p.from >> p.to >> p.amount >> p.due
              >> p.resolved))
            throw std::runtime_error("Save court fields");
    }
    proposals = std::move(staged);
}

} // namespace detail
} // namespace polyec
