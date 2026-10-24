#pragma once

#include "polyec/realm.hpp"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace polyec::detail {

inline void write_urban_residence_section(std::ostream& out,
                                          const std::vector<Person>& people) {
    out << " URBAN_RESIDENCE_1 " << people.size() << ' ';
    for (const auto& person : people)
        out << person.id << ' ' << static_cast<unsigned>(person.residence_kind)
            << ' ' << static_cast<unsigned>(person.urban_site_id) << ' ';
}

inline void read_urban_residence_section(std::istream& in,
                                         std::vector<Person>& people,
                                         int regions) {
    std::string tag;
    size_t count = 0;
    in >> tag >> count;
    if (!in || tag != "URBAN_RESIDENCE_1" || count != people.size())
        throw std::runtime_error("Save urban residence header");

    // Validate the entire section before publishing any field to the staged Realm.
    std::vector<std::pair<uint64_t, std::pair<unsigned, unsigned>>> staged;
    staged.reserve(count);
    for (const auto& person : people) {
        uint64_t id = 0;
        unsigned kind = 0, site = 0;
        in >> id >> kind >> site;
        if (!in || id != person.id || kind > static_cast<unsigned>(ResidenceKind::Urban) || site > 255)
            throw std::runtime_error("Save urban residence row");
        const auto residence = static_cast<ResidenceKind>(kind);
        if (residence == ResidenceKind::Urban) {
            if (regions != int(kAtlasTerritoryCount) - 1 || site >= kAtlasUrbanSites.size() ||
                kAtlasUrbanSites[site].territory_id != person.region)
                throw std::runtime_error("Save urban residence site");
        } else if (site != kNoUrbanSite) {
            throw std::runtime_error("Save non-urban residence site");
        }
        staged.push_back({id, {kind, site}});
    }
    for (size_t i = 0; i < people.size(); ++i) {
        people[i].residence_kind = static_cast<ResidenceKind>(staged[i].second.first);
        people[i].urban_site_id = static_cast<uint8_t>(staged[i].second.second);
    }
}

} // namespace polyec::detail
