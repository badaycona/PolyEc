#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the schema-independent Person section in the existing Realm token
// order. Realm owns the records and all cross-record validation.
inline void write_person_section(std::ostream& out, const std::vector<Person>& people) {
    out << people.size() << ' ';
    for (const auto& p : people) {
        out << p.id << ' ' << p.household_membership.household_id << ' '
            << p.decisions << ' ' << p.sampling << ' ' << p.region << ' '
            << p.age_days << ' ' << p.health << ' ' << p.skill << ' '
            << p.work_assignment.job << ' ' << p.busy_until << ' '
            << p.destination << ' ' << p.goal << ' ' << p.school_days << ' '
            << p.social << ' ' << p.alive << ' ' << int(p.family) << ' '
            << p.cash << ' ' << p.tax_remainder << ' ' << p.organization << ' '
            << p.work_assignment.job_kind << ' ' << p.taxing_authority << ' '
            << p.sovereign << ' ' << p.nutrition << ' ';
        for (auto value : p.stats.values) out << value << ' ';
        for (auto value : p.inventory) out << value << ' ';
    }
}

// Reads into a temporary vector and publishes it only after all records have
// been consumed. Realm still owns index rebuild and whole-world validation.
inline void read_person_section(std::istream& in, std::vector<Person>& people,
                                int regions, int realm_schema) {
    size_t count = 0;
    in >> count;
    if (!in || count > 1000000) throw std::runtime_error("Save count limit");

    std::vector<Person> staged(count);
    for (auto& p : staged) {
        int family = -1;
        in >> p.id >> p.household_membership.household_id >> p.decisions >> p.sampling
           >> p.region >> p.age_days >> p.health >> p.skill >> p.work_assignment.job
           >> p.busy_until >> p.destination >> p.goal >> p.school_days >> p.social
           >> p.alive >> family >> p.cash >> p.tax_remainder >> p.organization
           >> p.work_assignment.job_kind >> p.taxing_authority >> p.sovereign >> p.nutrition;
        if (p.region < 0 || p.region > regions)
            throw std::runtime_error("Save person geography");
        if (p.skill < kCapabilityMinimum || p.skill > kCapabilityMaximum)
            throw std::runtime_error("Save person skill");
        if (family < 0 || family > 2) throw std::runtime_error("Save role");
        if (p.household_membership.household_id == 0 ||
            p.household_membership.household_id > count)
            throw std::runtime_error("Save person household");
        p.family = Family(family);
        for (auto& value : p.stats.values) in >> value;
        if (realm_schema <= 12) {
            for (int legacy = 0; legacy < kLegacyGoodCount; ++legacy) {
                int64_t value = 0;
                in >> value;
                const int good = canonical_from_legacy_good(legacy);
                if (!in || good < 0) throw std::runtime_error("Save person inventory");
                p.inventory[static_cast<size_t>(good)] = value;
            }
        } else {
            const int good_count = realm_schema >= 55 ? kGoodIdCount : 27;
            for (int good = 0; good < good_count; ++good) in >> p.inventory[static_cast<size_t>(good)];
        }
        if (!in) throw std::runtime_error("Save person inventory");
    }
    people = std::move(staged);
}

} // namespace detail
} // namespace polyec
