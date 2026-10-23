#include "polyec/logistics_manager.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

namespace {
int checks = 0;
int failures = 0;
void check(bool value, const char* message) {
    ++checks;
    if (!value) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}
}

int main() {
    using namespace polyec;
    const std::vector<LogisticsRouteInput> network{
        {0, 1, 2, 10, 0, true},
        {1, 2, 3, 10, 0, true},
        {0, 2, 10, 100, 0, true},
    };
    const std::vector<LogisticsRouteInput> no_routes;
    const std::vector<LogisticsReservationInput> no_reservations;
    const auto multi_leg = LogisticsManager::plan(3, 0, 0, 2, 20, network, no_reservations);
    check(multi_leg.reachable && multi_leg.days == 7 &&
          multi_leg.regions == std::vector<int>{0, 1, 2},
          "chooses earliest multi-leg route after splitting cargo into daily batches");
    check(multi_leg.leg_starts == std::vector<int>{0, 3} &&
          multi_leg.leg_ends == std::vector<int>{1, 4} &&
          multi_leg.leg_arrivals == std::vector<int>{3, 7},
          "reports inclusive loading slots and arrival day for each leg");

    const auto reverse = LogisticsManager::plan(3, 0, 2, 0, 20, network, no_reservations);
    check(reverse.reachable && reverse.days == 7 &&
          reverse.regions == std::vector<int>{2, 1, 0},
          "routes are usable in either direction and share the same capacity");

    const std::vector<LogisticsRouteInput> downstream_river{{0, 1, 1, 64, 0, true, false}};
    const auto river_downstream = LogisticsManager::plan(2, 0, 0, 1, 1,
                                                          downstream_river, no_reservations);
    const auto river_upstream = LogisticsManager::plan(2, 0, 1, 0, 1,
                                                        downstream_river, no_reservations);
    check(river_downstream.reachable && river_downstream.regions == std::vector<int>{0, 1} &&
          !river_upstream.reachable,
          "a directed river permits the atlas direction and rejects an unapproved reverse trip");

    const std::vector<LogisticsRouteInput> mixed_directions{
        {0, 1, 1, 64, 0, true, false}, {1, 2, 1, 64, 0, true, true}};
    const auto mixed_reverse = LogisticsManager::plan(3, 0, 2, 0, 1,
                                                       mixed_directions, no_reservations);
    check(!mixed_reverse.reachable,
          "a reverse-enabled road cannot make a neighboring one-way river traversable backward");

    const std::vector<LogisticsRouteInput> tied_routes{
        {0, 2, 1, 10, 0, true}, {2, 3, 1, 10, 0, true},
        {0, 1, 1, 10, 0, true}, {1, 3, 1, 10, 0, true},
    };
    const auto tied = LogisticsManager::plan(4, 0, 0, 3, 1, tied_routes, no_reservations);
    check(tied.reachable && tied.days == 2 && tied.regions == std::vector<int>{0, 1, 3},
          "equal-arrival routes keep Campaign's deterministic region-ID tie behavior");

    const std::vector<LogisticsRouteInput> delayed{{0, 1, 2, 10, 4, true}};
    const auto delayed_plan = LogisticsManager::plan(2, 5, 0, 1, 10, delayed, no_reservations);
    check(delayed_plan.reachable && delayed_plan.days == 6 &&
          delayed_plan.leg_starts == std::vector<int>{5} &&
          delayed_plan.leg_arrivals == std::vector<int>{11},
          "applies caller-supplied extra travel days without inventing route values");

    const std::vector<LogisticsRouteInput> single_route{{0, 1, 1, 10, 0, true}};
    const std::vector<LogisticsReservationInput> chained_reservations{
        {1, 0, 0, 2}, {0, 1, 3, 4}};
    const auto queued = LogisticsManager::plan(2, 0, 0, 1, 10,
                                                single_route, chained_reservations);
    check(queued.reachable && queued.leg_starts == std::vector<int>{5} &&
          queued.leg_ends == std::vector<int>{5} && queued.days == 6,
          "moves loading after consecutive reservations in either direction");

    auto closed_route = single_route;
    closed_route[0].open = false;
    const auto closed = LogisticsManager::plan(2, 0, 0, 1, 1, closed_route, no_reservations);
    check(!closed.reachable && closed.regions.empty(), "closed route is not traversed");

    const auto same_region = LogisticsManager::plan(2, 9, 1, 1, 1, single_route, no_reservations);
    check(same_region.reachable && same_region.days == 0 &&
          same_region.regions == std::vector<int>{1} && same_region.leg_starts.empty(),
          "same source and destination needs no route or loading slots");

    check(!LogisticsManager::plan(0, 0, 0, 0, 1, no_routes, no_reservations).reachable &&
          !LogisticsManager::plan(28, 0, 0, 0, 1, no_routes, no_reservations).reachable &&
          !LogisticsManager::plan(2, -1, 0, 1, 1, single_route, no_reservations).reachable &&
          !LogisticsManager::plan(2, 0, 2, 1, 1, single_route, no_reservations).reachable &&
          !LogisticsManager::plan(2, 0, 0, 1, 0, single_route, no_reservations).reachable,
          "rejects invalid region count, day, region IDs and quantity");

    auto invalid_route = single_route;
    invalid_route[0].capacity_per_day = 0;
    check(!LogisticsManager::plan(2, 0, 0, 1, 1, invalid_route, no_reservations).reachable,
          "rejects invalid route capacity");
    auto invalid_reservation = chained_reservations;
    invalid_reservation[0].last_load_day = -1;
    check(!LogisticsManager::plan(2, 0, 0, 1, 1,
                                  single_route, invalid_reservation).reachable,
          "rejects invalid reservation interval");
    const std::vector<LogisticsReservationInput> reservation_without_route{{0, 2, 1, 2}};
    check(!LogisticsManager::plan(3, 0, 0, 1, 1,
                                  single_route, reservation_without_route).reachable,
          "rejects a reservation that references no route");
    check(!LogisticsManager::plan(2, 0, 0, 1, std::numeric_limits<int64_t>::max(),
                                  single_route, no_reservations).reachable,
          "rejects a batch count outside the supported day range");
    check(network.size() == 3 && network[0].capacity_per_day == 10 &&
          no_reservations.empty(), "planner does not mutate route or reservation inputs");

    if (failures) return 1;
    std::cout << "LOGISTICS MANAGER: " << checks << '/' << checks << " checks passed\n";
    return 0;
}






