#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the primary Project count and records in the existing Realm save token order.
// Court-owned extension fields (engineer, stalled, cause) remain in write_court.
inline void write_project_section(std::ostream& out,
                                 const std::vector<Project>& projects) {
    out << projects.size() << ' ';
    for (const auto& j : projects) {
        out << j.id << ' ' << j.owner << ' ' << j.region << ' ' << j.due << ' '
            << j.kind << ' ' << j.budget << ' ' << j.completed << ' ';
    }
}

// Reads into a temporary vector. Realm validates references and the complete
// world after all remaining sections have been parsed.
inline void read_project_section(std::istream& in, std::vector<Project>& projects) {
    size_t count = 0;
    in >> count;
    if (!in || count > 1000000) throw std::runtime_error("Save count limit");

    std::vector<Project> staged(count);
    for (auto& j : staged) {
        if (!(in >> j.id >> j.owner >> j.region >> j.due >> j.kind
              >> j.budget >> j.completed))
            throw std::runtime_error("Save count limit");
    }
    projects = std::move(staged);
}

} // namespace detail
} // namespace polyec
