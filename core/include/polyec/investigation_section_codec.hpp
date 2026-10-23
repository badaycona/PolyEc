#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the Investigation count and six-field records in the existing Realm
// court save token order. Realm owns investigations_ and validates references.
inline void write_investigation_section(std::ostream& out,
                                       const std::vector<Investigation>& investigations) {
    out << investigations.size() << ' ';
    for (const auto& i : investigations) {
        out << i.id << ' ' << i.owner << ' ' << i.cause << ' ' << i.region << ' '
            << i.due << ' ' << i.completed << ' ';
    }
}

// Reads into a temporary vector. Count and record parse failures preserve the
// legacy "Save court fields" error (not "Save count limit"). Realm performs
// owner/region validation after this read and before later court extensions.
inline void read_investigation_section(std::istream& in,
                                      std::vector<Investigation>& investigations) {
    size_t count = 0;
    in >> count;
    if (!in || count > 1000000) throw std::runtime_error("Save court fields");

    std::vector<Investigation> staged(count);
    for (auto& i : staged) {
        if (!(in >> i.id >> i.owner >> i.cause >> i.region >> i.due
              >> i.completed))
            throw std::runtime_error("Save court fields");
    }
    investigations = std::move(staged);
}

} // namespace detail
} // namespace polyec
