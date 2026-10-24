#pragma once

#include "polyec/land_holding.hpp"
#include "polyec/realm.hpp"

#include <algorithm>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace polyec::detail {

inline void write_private_housing_section(std::ostream& out,
                                          const std::vector<PrivateResidence>& homes) {
    std::vector<PrivateResidence> sorted = homes;
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    out << " PRIVATE_HOUSING_1 " << sorted.size() << ' ';
    for (const auto& home : sorted)
        out << home.id << ' ' << home.holding_id << ' ' << home.tenant_household << ' '
            << home.offered_household << ' ' << home.offered_rent << ' ' << home.offer_expires_day << ' '
            << home.region << ' ' << home.construction_due_day << ' ' << home.next_rent_day << ' '
            << home.lease_end_day << ' ' << home.rent_per_period << ' ' << home.completed << ' ';
}

inline std::vector<PrivateResidence> read_private_housing_section(
    std::istream& in, size_t person_count, int regions, int day, uint64_t next_id,
    const std::vector<LandHolding>& holdings) {
    std::string tag;
    size_t count = 0;
    in >> tag >> count;
    if (!in || tag != "PRIVATE_HOUSING_1" || count > 1000000 ||
        (regions != int(kAtlasTerritoryCount)-1 && count != 0))
        throw std::runtime_error("Save private housing header");
    std::vector<PrivateResidence> staged;
    staged.reserve(count);
    std::unordered_set<uint64_t> used_holdings;
    uint64_t previous = 0;
    for (size_t row = 0; row < count; ++row) {
        PrivateResidence home{};
        in >> home.id >> home.holding_id >> home.tenant_household >> home.offered_household
           >> home.offered_rent >> home.offer_expires_day >> home.region
           >> home.construction_due_day >> home.next_rent_day >> home.lease_end_day
           >> home.rent_per_period >> home.completed;
        const auto holding = std::lower_bound(holdings.begin(), holdings.end(), home.holding_id,
            [](const LandHolding& value, uint64_t id) { return value.id < id; });
        const bool has_lease = home.tenant_household != 0;
        const bool has_offer = home.offered_household != 0;
        if (!in || home.id == 0 || home.id >= next_id || (row && home.id <= previous) ||
            home.holding_id == 0 || holding == holdings.end() || holding->id != home.holding_id ||
            home.region < 0 || home.region > regions || holding->region != home.region ||
            !used_holdings.insert(home.holding_id).second ||
            (has_lease && (home.tenant_household > person_count || home.tenant_household == holding->title_holder ||
                           home.rent_per_period <= 0 || home.rent_per_period > 1000000 ||
                           home.next_rent_day < day || home.lease_end_day < home.next_rent_day || has_offer)) ||
            (!has_lease && (home.rent_per_period != 0 || home.next_rent_day != -1 || home.lease_end_day != -1)) ||
            (has_offer && (has_lease || home.offered_household > person_count ||
                           home.offered_household == holding->title_holder || home.offered_rent <= 0 ||
                           home.offered_rent > 1000000 || home.offer_expires_day < day)) ||
            (!has_offer && (home.offered_rent != 0 || home.offer_expires_day != -1)) ||
            home.construction_due_day < 0 || home.construction_due_day > 365000)
            throw std::runtime_error("Save private housing fields");
        previous = home.id;
        staged.push_back(home);
    }
    return staged;
}

} // namespace polyec::detail
