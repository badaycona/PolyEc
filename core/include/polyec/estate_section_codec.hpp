#pragma once

#include "polyec/realm.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace polyec::detail {

inline void write_estate_section(std::ostream& out, int64_t legacy_unassigned,
                                 const std::vector<EstateCase>& cases) {
    out << " ESTATE_1 " << legacy_unassigned << ' ' << cases.size() << ' ';
    for (const auto& item : cases) {
        out << item.decedent << ' ' << item.opened_day << ' ' << item.region << ' ' << item.cash << ' ';
        for (auto amount : item.inventory) out << amount << ' ';
        const auto write_ids = [&](const auto& ids) { out << ids.size() << ' '; for (auto id : ids) out << id << ' '; };
        write_ids(item.establishments); write_ids(item.land_holdings); write_ids(item.land_use_rights); write_ids(item.loans);
    }
}

inline std::vector<EstateCase> read_estate_section(std::istream& in, size_t person_count,
    int regions, uint64_t next_id, int64_t& legacy_unassigned, int realm_schema) {
    std::string tag; size_t count = 0;
    in >> tag >> legacy_unassigned >> count;
    if (!in || tag != "ESTATE_1" || legacy_unassigned < 0 || count > person_count || count > 1000000)
        throw std::runtime_error("Save estate header");
    std::vector<EstateCase> staged; staged.reserve(count);
    uint64_t previous_decedent = 0;
    for (size_t row = 0; row < count; ++row) {
        EstateCase item{};
        in >> item.decedent >> item.opened_day >> item.region >> item.cash;
        if (!in || item.decedent == 0 || item.decedent > person_count || (row && item.decedent <= previous_decedent) ||
            item.opened_day < 0 || item.region < 0 || item.region > regions || item.cash < 0)
            throw std::runtime_error("Save estate fields");
        const int good_count = realm_schema >= 55 ? kGoodIdCount : 27;
        for (int good = 0; good < good_count; ++good) {
            auto& amount = item.inventory[static_cast<size_t>(good)];
            in >> amount;
            if (!in || amount < 0) throw std::runtime_error("Save estate inventory");
        }
        const auto read_ids = [&](std::vector<uint64_t>& ids) {
            size_t n = 0; in >> n;
            if (!in || n > 1000000) throw std::runtime_error("Save estate reference count");
            ids.reserve(n); uint64_t previous = 0;
            for (size_t i = 0; i < n; ++i) {
                uint64_t id = 0; in >> id;
                if (!in || id == 0 || id >= next_id || (i && id <= previous)) throw std::runtime_error("Save estate reference");
                ids.push_back(id); previous = id;
            }
        };
        read_ids(item.establishments); read_ids(item.land_holdings); read_ids(item.land_use_rights); read_ids(item.loans);
        previous_decedent = item.decedent; staged.push_back(std::move(item));
    }
    return staged;
}

} // namespace polyec::detail
