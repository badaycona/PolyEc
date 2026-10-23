#include "polyec/national_transport_manager.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <queue>
#include <tuple>

namespace polyec {
namespace {

AtlasRouteMode configured_mode(const AtlasLink& link) noexcept {
    if (link.corridor == AtlasCorridor::East || link.corridor == AtlasCorridor::North ||
        link.corridor == AtlasCorridor::West || link.corridor == AtlasCorridor::RoyalDomains)
        return AtlasRouteMode::Road;
    return link.mode;
}

bool direction_approved(const AtlasLink& link, uint8_t from, uint8_t to) noexcept {
    if (link.direction == AtlasLinkDirection::Unspecified)
        return link.corridor == AtlasCorridor::RoyalDomains &&
               ((link.from == from && link.to == to) || (link.from == to && link.to == from));
    return atlas_link_allows_direction(link, from, to);
}

bool checked_add(int64_t a, int64_t b, int64_t& out) noexcept {
    if (a < 0 || b < 0 || a > std::numeric_limits<int64_t>::max() - b) return false;
    out = a + b;
    return true;
}

bool checked_multiply(int64_t a, int64_t b, int64_t& out) noexcept {
    if (a < 0 || b < 0 || (a && b > std::numeric_limits<int64_t>::max() / a)) return false;
    out = a * b;
    return true;
}

int load_days(int64_t quantity, int capacity) noexcept {
    const int64_t days = quantity / capacity + (quantity % capacity != 0);
    return days > std::numeric_limits<int>::max() ? 0 : int(days);
}

struct Label {
    int64_t primary = std::numeric_limits<int64_t>::max();
    int64_t secondary = std::numeric_limits<int64_t>::max();
    int64_t projected_days = 0;
    std::vector<uint8_t> regions;
    std::vector<size_t> route_indices;
    bool settled = false;
};

bool better(int64_t primary, int64_t secondary, const std::vector<uint8_t>& regions,
            const Label& current) {
    if (primary != current.primary) return primary < current.primary;
    if (secondary != current.secondary) return secondary < current.secondary;
    return std::lexicographical_compare(regions.begin(), regions.end(),
                                        current.regions.begin(), current.regions.end());
}

} // namespace

std::vector<NationalTransportRoute> NationalTransportManager::default_routes() {
    std::vector<NationalTransportRoute> routes;
    routes.reserve(kAtlasLinkCount * 2);
    for (size_t index = 0; index < kWorldAtlasLinks.size(); ++index) {
        const auto& link = kWorldAtlasLinks[index];
        const auto mode = configured_mode(link);
        const int travel_days = mode == AtlasRouteMode::River ? 1 : 2;
        auto add = [&](uint8_t from, uint8_t to) {
            routes.push_back({uint8_t(index), from, to, mode, true, travel_days, 64, 1,
                              TransportFeeAccount::ProvinceTreasury, int(from)});
        };
        if (link.direction == AtlasLinkDirection::Directed) {
            add(link.from, link.to);
        } else if (link.direction == AtlasLinkDirection::Bidirectional ||
                   link.corridor == AtlasCorridor::RoyalDomains) {
            add(link.from, link.to);
            add(link.to, link.from);
        }
    }
    return routes;
}

bool NationalTransportManager::routes_are_well_formed(
    const std::vector<NationalTransportRoute>& routes, int region_count) noexcept {
    if (region_count != int(kAtlasTerritoryCount) || routes.empty() ||
        routes.size() > kAtlasLinkCount * 2) return false;
    for (size_t i = 0; i < routes.size(); ++i) {
        const auto& route = routes[i];
        if (route.atlas_link >= kWorldAtlasLinks.size() || route.from >= region_count ||
            route.to >= region_count || route.from == route.to || route.travel_days < 1 || route.travel_days > 365000 ||
            route.capacity_per_day < 1 || route.capacity_per_day > 1000000 || route.fee_per_unit < 0 ||
            route.fee_account != TransportFeeAccount::ProvinceTreasury ||
            route.fee_recipient_region < 0 || route.fee_recipient_region >= region_count)
            return false;
        const auto& link = kWorldAtlasLinks[route.atlas_link];
        if (!direction_approved(link, route.from, route.to) ||
            route.mode != configured_mode(link)) return false;
        if (link.corridor == AtlasCorridor::GreatRiver &&
            (route.from != link.from || route.to != link.to)) return false;
        for (size_t previous = 0; previous < i; ++previous) {
            if (routes[previous].atlas_link != route.atlas_link) continue;
            if (routes[previous].from == route.from && routes[previous].to == route.to)
                return false;
            if (routes[previous].capacity_per_day != route.capacity_per_day ||
                routes[previous].travel_days != route.travel_days ||
                routes[previous].fee_per_unit != route.fee_per_unit ||
                routes[previous].mode != route.mode) return false;
        }
    }
    return true;
}

NationalTransportPathPlan NationalTransportManager::plan(
    int current_day, uint8_t from, uint8_t to, int64_t quantity, TransportServiceGoal goal,
    const std::vector<NationalTransportRoute>& routes,
    const std::array<int64_t,kAtlasLinkCount>& waiting_units_by_link) {
    NationalTransportPathPlan result;
    if (current_day < 0 || from >= kAtlasTerritoryCount || to >= kAtlasTerritoryCount ||
        quantity < 1 || goal > TransportServiceGoal::LowestFee ||
        !routes_are_well_formed(routes) ||
        std::any_of(waiting_units_by_link.begin(),waiting_units_by_link.end(),[](int64_t value){return value<0;})) return result;
    if (from == to) {
        result.reachable = true;
        result.regions.push_back(from);
        return result;
    }

    std::array<Label, kAtlasTerritoryCount> labels{};
    labels[from].primary = 0;
    labels[from].secondary = 0;
    labels[from].regions.push_back(from);
    for (size_t iteration = 0; iteration < kAtlasTerritoryCount; ++iteration) {
        int current = -1;
        for (size_t region = 0; region < kAtlasTerritoryCount; ++region) {
            if (labels[region].settled || labels[region].primary == std::numeric_limits<int64_t>::max()) continue;
            if (current < 0 || better(labels[region].primary, labels[region].secondary,
                                      labels[region].regions, labels[size_t(current)]))
                current = int(region);
        }
        if (current < 0) break;
        auto& label = labels[size_t(current)];
        label.settled = true;
        if (current == to) break;

        for (size_t route_index = 0; route_index < routes.size(); ++route_index) {
            const auto& route = routes[route_index];
            if (!route.open || route.from != current) continue;
            const int64_t ready_day=label.projected_days==0?1:label.projected_days;
            const int64_t prior_capacity=ready_day-1>std::numeric_limits<int64_t>::max()/route.capacity_per_day
                ?std::numeric_limits<int64_t>::max():(ready_day-1)*route.capacity_per_day;
            const int64_t waiting=waiting_units_by_link[route.atlas_link];
            const int64_t remaining_wait=waiting>prior_capacity?waiting-prior_capacity:0;
            if(quantity>std::numeric_limits<int64_t>::max()-remaining_wait)continue;
            const int days_to_load=load_days(remaining_wait+quantity,route.capacity_per_day);
            if(!days_to_load)continue;
            const int64_t arrival=ready_day+days_to_load-1+route.travel_days;
            if(arrival>std::numeric_limits<int>::max())continue;
            int64_t leg_fee = 0;
            if (!checked_multiply(route.fee_per_unit, quantity, leg_fee)) continue;
            int64_t primary=0,secondary=0;
            if(goal==TransportServiceGoal::Fastest){
                primary=arrival;
                if(!checked_add(label.secondary,leg_fee,secondary))continue;
            }else{
                if(!checked_add(label.primary,leg_fee,primary))continue;
                secondary=arrival;
            }
            auto path_regions = label.regions;
            path_regions.push_back(route.to);
            auto& next = labels[route.to];
            if (!next.settled && better(primary, secondary, path_regions, next)) {
                next.primary = primary;
                next.secondary = secondary;
                next.projected_days=arrival;
                next.regions = std::move(path_regions);
                next.route_indices = label.route_indices;
                next.route_indices.push_back(route_index);
            }
        }
    }

    const auto& destination = labels[to];
    if (!destination.settled || destination.secondary > std::numeric_limits<int64_t>::max() ||
        destination.regions.empty()) return result;
    int64_t total_fee = 0;
    for (size_t leg=0;leg<destination.route_indices.size();++leg) {
        const size_t index=destination.route_indices[leg];
        const auto& route = routes[index];
        int64_t fee = 0, next_fee = 0;
        if (!checked_multiply(route.fee_per_unit, quantity, fee) ||
            !checked_add(total_fee, fee, next_fee)) return result;
        total_fee = next_fee;
    }
    const int64_t total_days=destination.projected_days;
    if (total_days < 1 || int64_t(current_day) + total_days > std::numeric_limits<int>::max()) return result;
    result.reachable = true;
    result.total_days = int(total_days);
    result.total_fee = total_fee;
    result.regions = destination.regions;
    result.route_indices = destination.route_indices;
    return result;
}

} // namespace polyec
