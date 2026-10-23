#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the Province court extension fields in the existing Realm save token
// order. There is no count prefix; the number of records is fixed by the
// primary Province section. Primary Province fields are not touched.
inline void write_province_court_extension_section(std::ostream& out,
                                                 const std::vector<Province>& provinces) {
    for (const auto& v : provinces) {
        out << v.tax_receipts << ' ' << v.food_capacity << ' ' << v.teacher_skill_sum << ' ';
    }
}

// Reads extension fields into a staged copy of the incoming vector. The copy
// is published only after every field in every record has parsed, so a parse
// failure leaves the caller's vector unchanged. All parse failures throw
// std::runtime_error("Save court fields"); there is no count prefix and no
// "Save count limit" error for this section. No range validation is added.
inline void read_province_court_extension_section(std::istream& in,
                                                std::vector<Province>& provinces) {
    std::vector<Province> staged = provinces;
    for (auto& v : staged) {
        if (!(in >> v.tax_receipts >> v.food_capacity >> v.teacher_skill_sum))
            throw std::runtime_error("Save court fields");
    }
    provinces = std::move(staged);
}

} // namespace detail
} // namespace polyec
