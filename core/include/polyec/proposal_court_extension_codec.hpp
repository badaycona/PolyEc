#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the Proposal court extension fields in the existing Realm save token
// order. There is no count prefix; the number of records is fixed by the
// primary Proposal section. Primary Proposal fields are not touched.
inline void write_proposal_court_extension_section(std::ostream& out,
                                                  const std::vector<Proposal>& proposals) {
    for (const auto& o : proposals) {
        out << o.kind << ' ' << o.ready << ' ' << o.escrow << ' '
            << o.public_source << ' ' << o.mediation << ' '
            << o.source_region << ' ' << o.cause << ' ';
        for (auto id : o.parties) out << id << ' ';
        for (auto r : o.regions) out << r << ' ';
        for (bool b : o.pledged) out << b << ' ';
        out << o.service_term << ' ';
    }
}

// Reads extension fields into a staged copy of the incoming vector. The copy
// is published only after every field in every record has parsed, so a parse
// failure leaves the caller's vector unchanged. All parse failures throw
// std::runtime_error("Save court fields"); there is no count prefix and no
// "Save count limit" error for this section. Realm performs kind/source_region/
// regions validation after this read.
inline void read_proposal_court_extension_section(std::istream& in,
                                                  std::vector<Proposal>& proposals,
                                                  int realm_schema=45) {
    std::vector<Proposal> staged = proposals;
    for (auto& o : staged) {
        if (!(in >> o.kind >> o.ready >> o.escrow >> o.public_source >>
              o.mediation >> o.source_region >> o.cause))
            throw std::runtime_error("Save court fields");
        for (auto& id : o.parties) {
            if (!(in >> id)) throw std::runtime_error("Save court fields");
        }
        for (auto& r : o.regions) {
            if (!(in >> r)) throw std::runtime_error("Save court fields");
        }
        for (auto& b : o.pledged) {
            if (!(in >> b)) throw std::runtime_error("Save court fields");
        }
        if (realm_schema >= 45 && !(in >> o.service_term))
            throw std::runtime_error("Save court fields");
    }
    proposals = std::move(staged);
}

} // namespace detail
} // namespace polyec
