#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the Loan court extension fields in the existing Realm save token
// order. There is no count prefix; the number of records is fixed by the
// primary Loan section. Primary Loan fields are not touched.
inline void write_loan_court_extension_section(std::ostream& out,
                                             const std::vector<Loan>& loans) {
    for (const auto& l : loans) {
        out << l.royal_pool << ' ' << l.cause << ' ';
    }
}

// Reads extension fields into a staged copy of the incoming vector. The copy
// is published only after every field in every record has parsed, so a parse
// failure leaves the caller's vector unchanged. All parse failures throw
// std::runtime_error("Save court fields"); there is no count prefix and no
// "Save count limit" error for this section. No range validation is added.
inline void read_loan_court_extension_section(std::istream& in,
                                            std::vector<Loan>& loans) {
    std::vector<Loan> staged = loans;
    for (auto& l : staged) {
        if (!(in >> l.royal_pool >> l.cause))
            throw std::runtime_error("Save court fields");
    }
    loans = std::move(staged);
}

} // namespace detail
} // namespace polyec
