#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the referrals count and IDs in the existing Realm court save token
// order. Realm owns referrals_ and validates the IDs.
inline void write_referrals_section(std::ostream& out,
                                   const std::vector<uint64_t>& referrals) {
    out << referrals.size() << ' ';
    for (auto id : referrals) out << id << ' ';
}

// Reads the referrals count and IDs into a local vector and publishes it to
// the output reference only after every ID has parsed. Count parse failure,
// excessive count, or a truncated ID stream throws
// std::runtime_error("Save court fields") before assignment, so the caller's
// vector remains unchanged. Per-ID domain validation stays in Realm.
inline void read_referrals_section(std::istream& in,
                                  std::vector<uint64_t>& referrals,
                                  size_t people_size) {
    size_t count = 0;
    in >> count;
    if (!in || count > people_size) throw std::runtime_error("Save court fields");

    std::vector<uint64_t> staged(count);
    for (auto& id : staged) {
        if (!(in >> id)) throw std::runtime_error("Save court fields");
    }
    referrals = std::move(staged);
}

} // namespace detail
} // namespace polyec
