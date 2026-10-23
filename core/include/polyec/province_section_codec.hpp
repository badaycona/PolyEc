#pragma once

#include "polyec/realm.hpp"
#include "polyec/world_atlas.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the primary Province fields in the existing Realm save token order.
// Court-owned Province counters remain in the court extension section.
inline void write_province_section(std::ostream& out,
                                   const std::vector<Province>& provinces) {
    out << provinces.size() << ' ';
    for (const auto& p : provinces) {
        out << p.treasury << ' ' << p.firm << ' ' << p.tax << ' ' << p.school << ' '
            << p.school_used << ' ' << p.jobs << ' ' << p.employed << ' ' << p.road << ' '
            << p.unrest << ' ' << p.teachers << ' ' << p.charter << ' '
            << p.sales_tax_remainder << ' ';
        for (auto value : p.stock) out << value << ' ';
        for (auto value : p.price) out << value << ' ';
    }
}

// Reads into a temporary vector. Realm validates geography and the complete
// world after all remaining sections have been parsed.
inline void read_province_section(std::istream& in,
                                  std::vector<Province>& provinces,
                                  int realm_schema) {
    size_t count = 0;
    in >> count;
    if (!in || count > kAtlasTerritoryCount) throw std::runtime_error("Save count limit");

    std::vector<Province> staged(count);
    for (auto& p : staged) {
        in >> p.treasury >> p.firm >> p.tax >> p.school >> p.school_used >> p.jobs
           >> p.employed >> p.road >> p.unrest >> p.teachers >> p.charter
           >> p.sales_tax_remainder;
        if (realm_schema <= 12) {
            for (int legacy = 0; legacy < kLegacyGoodCount; ++legacy) {
                int64_t value = 0;
                in >> value;
                const int good = canonical_from_legacy_good(legacy);
                if (!in || good < 0) throw std::runtime_error("Save province stock");
                p.stock[static_cast<size_t>(good)] = value;
            }
            for (int legacy = 0; legacy < kLegacyGoodCount; ++legacy) {
                int value = 0;
                in >> value;
                const int good = canonical_from_legacy_good(legacy);
                if (!in || good < 0) throw std::runtime_error("Save province price");
                p.price[static_cast<size_t>(good)] = value;
            }
        } else {
            const int good_count = realm_schema >= 55 ? kGoodIdCount : 27;
            for (int good = 0; good < good_count; ++good) in >> p.stock[static_cast<size_t>(good)];
            for (int good = 0; good < good_count; ++good) in >> p.price[static_cast<size_t>(good)];
        }
        if (!in) throw std::runtime_error("Save province fields");
    }
    provinces = std::move(staged);
}

} // namespace detail
} // namespace polyec
