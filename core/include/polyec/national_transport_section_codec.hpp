#pragma once

#include "polyec/national_transport_manager.hpp"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace polyec::detail {

inline void write_national_transport_section(
    std::ostream& out, const std::vector<NationalTransportRoute>& routes) {
    out << " NATIONAL_TRANSPORT_1 " << NationalTransportManager::config_version << ' '
        << routes.size() << ' ';
    for (const auto& route : routes)
        out << unsigned(route.atlas_link) << ' ' << unsigned(route.from) << ' '
            << unsigned(route.to) << ' ' << unsigned(route.mode) << ' ' << route.open << ' '
            << route.travel_days << ' ' << route.capacity_per_day << ' ' << route.fee_per_unit << ' '
            << unsigned(route.fee_account) << ' ' << route.fee_recipient_region << ' ';
}

inline std::vector<NationalTransportRoute> read_national_transport_section(
    std::istream& in, int region_bound) {
    std::string tag;
    uint32_t version = 0;
    size_t count = 0;
    in >> tag >> version >> count;
    if (!in || tag != "NATIONAL_TRANSPORT_1" ||
        version != NationalTransportManager::config_version || count > kAtlasLinkCount * 2)
        throw std::runtime_error("Save national transport header");
    std::vector<NationalTransportRoute> staged;
    staged.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        unsigned link = 0, from = 0, to = 0, mode = 0, account = 0;
        int open = 0;
        NationalTransportRoute route;
        in >> link >> from >> to >> mode >> open >> route.travel_days >> route.capacity_per_day
           >> route.fee_per_unit >> account >> route.fee_recipient_region;
        if (!in || link > UINT8_MAX || from > UINT8_MAX || to > UINT8_MAX ||
            mode > unsigned(AtlasRouteMode::CoastalShipping) || (open != 0 && open != 1) ||
            account > unsigned(TransportFeeAccount::ProvinceTreasury))
            throw std::runtime_error("Save national transport fields");
        route.atlas_link = uint8_t(link);
        route.from = uint8_t(from);
        route.to = uint8_t(to);
        route.mode = static_cast<AtlasRouteMode>(mode);
        route.open = open != 0;
        route.fee_account = static_cast<TransportFeeAccount>(account);
        staged.push_back(route);
    }
    if (region_bound == int(kAtlasTerritoryCount) - 1) {
        const auto defaults = NationalTransportManager::default_routes();
        if (staged.size() != defaults.size()) throw std::runtime_error("Save national transport coverage");
        for (size_t i = 0; i < staged.size(); ++i)
            if (staged[i].atlas_link != defaults[i].atlas_link ||
                staged[i].from != defaults[i].from || staged[i].to != defaults[i].to)
                throw std::runtime_error("Save national transport route identity");
        if (!NationalTransportManager::routes_are_well_formed(staged))
            throw std::runtime_error("Save national transport config");
    } else if (region_bound == 1 || region_bound == 2 || region_bound == 6) {
        if (!staged.empty()) throw std::runtime_error("Save national transport profile mismatch");
    } else {
        throw std::runtime_error("Save national transport geography");
    }
    return staged;
}

} // namespace polyec::detail
