#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the Project court extension fields in the existing Realm save token
// order. There is no count prefix; the number of records is fixed by the
// primary Project section. Primary Project fields are not touched.
inline void write_project_court_extension_section(std::ostream& out,
                                                 const std::vector<Project>& projects) {
    for (const auto& j : projects) {
        out << j.engineer << ' ' << j.stalled << ' ' << j.cause << ' ';
    }
}

// Reads extension fields into a staged copy of the incoming vector. The copy
// is published only after every field in every record has parsed, so a parse
// failure leaves the caller's vector unchanged. All parse failures throw
// std::runtime_error("Save court fields"); there is no count prefix and no
// "Save count limit" error for this section. No range validation is added.
inline void read_project_court_extension_section(std::istream& in,
                                                std::vector<Project>& projects) {
    std::vector<Project> staged = projects;
    for (auto& j : staged) {
        if (!(in >> j.engineer >> j.stalled >> j.cause))
            throw std::runtime_error("Save court fields");
    }
    projects = std::move(staged);
}

} // namespace detail
} // namespace polyec
