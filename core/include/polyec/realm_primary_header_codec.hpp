#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <random>

namespace polyec {
namespace detail {

struct RealmPrimaryHeader {
    int day = 0;
    int regions = 0;
    uint64_t seed = 0;
    uint64_t next_id = 1;
    uint64_t next_fact = 1;
    int64_t opening_money = 0;
    int64_t carrier = 0;
    int64_t estate = 0;
    int64_t endowment = 0;
    Draft draft{};
    std::array<int64_t, kGoodIdCount> opening{};
    std::array<int64_t, kGoodIdCount> produced{};
    std::array<int64_t, kGoodIdCount> consumed{};
    std::array<std::array<bool, 7>, 7> routes{};
    std::array<int64_t, kOrganizationCount> organization_funds{};
    std::mt19937_64 rng{};
};

inline void write_realm_primary_header(std::ostream& out,
                                       const RealmPrimaryHeader& header) {
    out << header.day << ' ' << header.regions << ' ' << header.seed << ' '
        << header.next_id << ' ' << header.next_fact << ' ' << header.opening_money << ' '
        << header.carrier << ' ' << header.estate << ' ' << header.endowment << ' ';
    out << header.draft.tier << ' ' << header.draft.seen << ' ' << header.draft.chosen << ' '
        << header.draft.rerolled << ' ';
    for (auto card : header.draft.cards) out << card << ' ';
    for (auto const* array : {&header.opening, &header.produced, &header.consumed})
        for (auto value : *array) out << value << ' ';
    for (auto const& route : header.routes)
        for (bool open : route) out << open << ' ';
    for (auto funds : header.organization_funds) out << funds << ' ';
    out << header.rng << ' ';
}

inline bool read_realm_primary_header_initial(std::istream& in,
                                             RealmPrimaryHeader& header) {
    RealmPrimaryHeader staged = header;
    if (!(in >> staged.day >> staged.regions >> staged.seed >> staged.next_id >>
          staged.next_fact >> staged.opening_money >> staged.carrier >>
          staged.estate >> staged.endowment)) {
        return false;
    }
    header = staged;
    return true;
}

inline bool read_realm_primary_header_tail(std::istream& in,
                                          RealmPrimaryHeader& header,
                                          int realm_schema = 12) {
    RealmPrimaryHeader staged = header;
    if (!(in >> staged.draft.tier >> staged.draft.seen >> staged.draft.chosen >>
          staged.draft.rerolled)) {
        return false;
    }
    for (auto& card : staged.draft.cards) {
        if (!(in >> card)) return false;
    }
    for (auto* array : {&staged.opening, &staged.produced, &staged.consumed}) {
        if (realm_schema <= 12) {
            for (int legacy = 0; legacy < kLegacyGoodCount; ++legacy) {
                int64_t value = 0;
                if (!(in >> value)) return false;
                const int good = canonical_from_legacy_good(legacy);
                if (good < 0) return false;
                (*array)[static_cast<size_t>(good)] = value;
            }
        } else {
            const int good_count = realm_schema >= 55 ? kGoodIdCount : 27;
            for (int good = 0; good < good_count; ++good)
                if (!(in >> (*array)[static_cast<size_t>(good)])) return false;
        }
    }
    for (auto& route : staged.routes) {
        for (auto& open : route) {
            if (!(in >> open)) return false;
        }
    }
    staged.organization_funds.fill(0);
    const size_t organization_count = realm_schema >= 44 ? kOrganizationCount : 28;
    for (size_t i = 0; i < organization_count; ++i) {
        if (!(in >> staged.organization_funds[i])) return false;
    }
    if (!(in >> staged.rng)) return false;
    header = staged;
    return true;
}

} // namespace detail
} // namespace polyec
