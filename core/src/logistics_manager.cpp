#include "polyec/logistics_manager.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace polyec {
namespace {
bool same_undirected_edge(int a_from, int a_to, int b_from, int b_to) noexcept {
    return (a_from == b_from && a_to == b_to) ||
           (a_from == b_to && a_to == b_from);
}

bool has_route(const std::vector<LogisticsRouteInput>& routes, int from, int to) noexcept {
    for (const auto& route : routes)
        if (same_undirected_edge(route.from, route.to, from, to)) return true;
    return false;
}
}

LogisticsPathPlan LogisticsManager::plan(
    int region_count,
    int current_day,
    int from,
    int to,
    int64_t quantity,
    const std::vector<LogisticsRouteInput>& routes,
    const std::vector<LogisticsReservationInput>& reservations) {
    LogisticsPathPlan result;
    if (region_count < 1 || region_count > max_regions || current_day < 0 ||
        from < 0 || from >= region_count || to < 0 || to >= region_count || quantity < 1)
        return result;

    for (const auto& route : routes)
        if (route.from < 0 || route.from >= region_count || route.to < 0 ||
            route.to >= region_count || route.from == route.to || route.travel_days < 1 ||
            route.capacity_per_day < 1 || route.additional_days < 0)
            return result;
    for (const auto& reservation : reservations)
        if (reservation.first_load_day < 0 ||
            reservation.last_load_day < reservation.first_load_day ||
            !has_route(routes, reservation.from, reservation.to))
            return result;

    if (from == to) {
        result.reachable = true;
        result.regions.push_back(from);
        return result;
    }

    std::array<int64_t, max_regions> arrival;
    std::array<int, max_regions> previous, starts, ends;
    std::array<bool, max_regions> done{};
    arrival.fill(std::numeric_limits<int64_t>::max());
    previous.fill(-1);
    starts.fill(0);
    ends.fill(0);
    arrival[size_t(from)] = current_day;

    for (int iteration = 0; iteration < region_count; ++iteration) {
        int current = -1;
        for (int candidate = 0; candidate < region_count; ++candidate)
            if (!done[size_t(candidate)] &&
                (current < 0 || arrival[size_t(candidate)] < arrival[size_t(current)]))
                current = candidate;
        if (current < 0 || arrival[size_t(current)] == std::numeric_limits<int64_t>::max()) break;
        done[size_t(current)] = true;

        for (const auto& route : routes) {
            if (!route.open || (route.from != current &&
                                (route.to != current || !route.allow_reverse))) continue;
            const int next = route.from == current ? route.to : route.from;
            const int64_t capacity = route.capacity_per_day;
            const int64_t batches = quantity / capacity + (quantity % capacity != 0);
            if (batches < 1 || batches > std::numeric_limits<int>::max()) continue;

            int64_t load_start = arrival[size_t(current)];
            bool shifted = true;
            while (shifted) {
                shifted = false;
                const int64_t load_end = load_start + batches - 1;
                if (load_end > std::numeric_limits<int>::max()) break;
                for (const auto& reservation : reservations) {
                    if (!same_undirected_edge(current, next, reservation.from, reservation.to)) continue;
                    if (load_start <= reservation.last_load_day &&
                        load_end >= reservation.first_load_day) {
                        load_start = int64_t(reservation.last_load_day) + 1;
                        shifted = true;
                    }
                }
            }

            const int64_t load_end = load_start + batches - 1;
            const int64_t next_arrival = load_end + route.travel_days + route.additional_days;
            if (load_end > std::numeric_limits<int>::max() ||
                next_arrival > std::numeric_limits<int>::max()) continue;
            if (next_arrival < arrival[size_t(next)]) {
                arrival[size_t(next)] = next_arrival;
                previous[size_t(next)] = current;
                starts[size_t(next)] = int(load_start);
                ends[size_t(next)] = int(load_end);
            }
        }
    }

    if (arrival[size_t(to)] == std::numeric_limits<int64_t>::max()) return result;
    result.reachable = true;
    result.days = int(arrival[size_t(to)] - current_day);
    for (int region = to; region >= 0; region = previous[size_t(region)]) {
        result.regions.push_back(region);
        if (region == from) break;
        result.leg_starts.push_back(starts[size_t(region)]);
        result.leg_ends.push_back(ends[size_t(region)]);
        result.leg_arrivals.push_back(int(arrival[size_t(region)]));
    }
    std::reverse(result.regions.begin(), result.regions.end());
    std::reverse(result.leg_starts.begin(), result.leg_starts.end());
    std::reverse(result.leg_ends.begin(), result.leg_ends.end());
    std::reverse(result.leg_arrivals.begin(), result.leg_arrivals.end());
    return result;
}

} // namespace polyec
