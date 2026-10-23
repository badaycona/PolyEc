#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the primary Fact count and records in the existing Realm save token order.
inline void write_fact_section(std::ostream& out,
                               const std::vector<Fact>& facts) {
    out << facts.size() << ' ';
    for (const auto& f : facts) {
        out << f.id << ' ' << f.parent << ' ' << f.actor << ' ' << f.target << ' '
            << f.day << ' ' << f.region << ' ' << f.kind << ' ' << f.amount << ' '
            << f.secret << ' ';
    }
}

// Reads into a temporary vector. Realm validates references and the complete
// world after all remaining sections have been parsed.
inline void read_fact_section(std::istream& in, std::vector<Fact>& facts) {
    size_t count = 0;
    in >> count;
    if (!in || count > 8192) throw std::runtime_error("Save count limit");

    std::vector<Fact> staged(count);
    for (auto& f : staged) {
        if (!(in >> f.id >> f.parent >> f.actor >> f.target >> f.day
              >> f.region >> f.kind >> f.amount >> f.secret))
            throw std::runtime_error("Save count limit");
    }
    facts = std::move(staged);
}

} // namespace detail
} // namespace polyec
