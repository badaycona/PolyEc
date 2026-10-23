#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the primary Loan count and records in the existing Realm save token order.
// Court-owned extension fields (royal_pool, cause) remain in write_court.
inline void write_loan_section(std::ostream& out,
                               const std::vector<Loan>& loans) {
    out << loans.size() << ' ';
    for (const auto& l : loans) {
        out << l.id << ' ' << l.debtor << ' ' << l.creditor << ' ' << l.due << ' '
            << l.principal << ' ' << l.interest << ' ' << l.paid << ' '
            << l.defaulted << ' ';
    }
}

// Reads into a temporary vector. Realm validates references and the complete
// world after all remaining sections have been parsed.
inline void read_loan_section(std::istream& in, std::vector<Loan>& loans) {
    size_t count = 0;
    in >> count;
    if (!in || count > 1000000) throw std::runtime_error("Save count limit");

    std::vector<Loan> staged(count);
    for (auto& l : staged) {
        if (!(in >> l.id >> l.debtor >> l.creditor >> l.due >> l.principal
              >> l.interest >> l.paid >> l.defaulted))
            throw std::runtime_error("Save count limit");
    }
    loans = std::move(staged);
}

} // namespace detail
} // namespace polyec
