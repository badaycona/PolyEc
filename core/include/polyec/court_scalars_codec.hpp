#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>

namespace polyec {
namespace detail {

// Writes the seven leading court scalars in the existing Realm save token
// order. There is no count prefix; these are fixed Realm-owned scalar values.
// Primary Person/Province/... fields are not touched.
inline void write_court_scalars(std::ostream& out,
                               int64_t royal_reference,
                               int64_t credit_pool,
                               int64_t restricted_fund,
                               int mediation_ready,
                               int bonus_offices,
                               bool pact,
                               bool reference_scenario) {
    out << royal_reference << ' ' << credit_pool << ' ' << restricted_fund << ' '
        << mediation_ready << ' ' << bonus_offices << ' ' << pact << ' '
        << reference_scenario << ' ';
}

// Reads the seven leading court scalars into local temporaries and publishes
// them to the output references only after every scalar has parsed. A parse
// failure throws std::runtime_error("Save court fields") before assignment, so
// the caller's values remain unchanged. There is no count prefix and no
// "Save count limit" error for this section. No range validation is added.
// Truncation is not field-boundary detection (unframed section).
inline void read_court_scalars(std::istream& in,
                              int64_t& royal_reference,
                              int64_t& credit_pool,
                              int64_t& restricted_fund,
                              int& mediation_ready,
                              int& bonus_offices,
                              bool& pact,
                              bool& reference_scenario) {
    int64_t staged_royal_reference = royal_reference;
    int64_t staged_credit_pool = credit_pool;
    int64_t staged_restricted_fund = restricted_fund;
    int staged_mediation_ready = mediation_ready;
    int staged_bonus_offices = bonus_offices;
    bool staged_pact = pact;
    bool staged_reference_scenario = reference_scenario;

    if (!(in >> staged_royal_reference >> staged_credit_pool
          >> staged_restricted_fund >> staged_mediation_ready
          >> staged_bonus_offices >> staged_pact >> staged_reference_scenario))
        throw std::runtime_error("Save court fields");

    royal_reference = staged_royal_reference;
    credit_pool = staged_credit_pool;
    restricted_fund = staged_restricted_fund;
    mediation_ready = staged_mediation_ready;
    bonus_offices = staged_bonus_offices;
    pact = staged_pact;
    reference_scenario = staged_reference_scenario;
}

} // namespace detail
} // namespace polyec
