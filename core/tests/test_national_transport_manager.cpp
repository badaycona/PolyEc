#include "polyec/national_transport_manager.hpp"

#include <algorithm>
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
    auto routes = NationalTransportManager::default_routes();
    check(routes.size() == 59 && NationalTransportManager::routes_are_well_formed(routes),
          "builds a valid explicit route configuration from all 31 atlas links");
    check(std::any_of(routes.begin(), routes.end(), [](const auto& route) {
              return route.atlas_link == 0 && route.from == 0 && route.to == 2 &&
                     route.mode == AtlasRouteMode::River && route.travel_days == 1;
          }) &&
          std::none_of(routes.begin(), routes.end(), [](const auto& route) {
              return route.atlas_link == 0 && route.from == 2 && route.to == 0;
          }), "keeps the first river in its mapped downstream direction only");
    check(std::count_if(routes.begin(), routes.end(), [](const auto& route) {
              return route.atlas_link >= 26 && route.from == 24 && route.to == 0;
          }) == 1 &&
          std::count_if(routes.begin(), routes.end(), [](const auto& route) {
              return route.atlas_link >= 26 && route.from == 0 && route.to == 24;
          }) == 1, "creates the documented two-way provisional road for a royal-domain atlas link");
    check(std::all_of(routes.begin(), routes.end(), [](const auto& route) {
              return route.capacity_per_day == 64 && route.fee_per_unit == 1 &&
                     route.fee_account == TransportFeeAccount::ProvinceTreasury &&
                     route.fee_recipient_region == route.from;
          }), "assigns explicit daily capacity, fee and departure-region treasury to every direction");
    check(std::all_of(routes.begin(),routes.end(),[](const auto& route){
        const auto corridor=kWorldAtlasLinks[route.atlas_link].corridor;
        const int expected=corridor==AtlasCorridor::GreatRiver?1:
            corridor==AtlasCorridor::North?3:corridor==AtlasCorridor::West?4:2;
        return route.travel_days==expected;
    }),"assigns the versioned terrain travel time to every direction of all Atlas corridors");
    check(std::all_of(routes.begin(),routes.end(),[](const auto& route){
        if(route.atlas_link==15)return route.travel_days==3;
        if(route.atlas_link==20)return route.travel_days==4;
        return true;
    }),"both bi-directional northern and western mountain roads inherit their corridor resistance");

    check(NationalTransportManager::seasonally_passable(272,15)&&
          !NationalTransportManager::seasonally_passable(273,15)&&
          !NationalTransportManager::seasonally_passable(364,15)&&
          NationalTransportManager::seasonally_passable(365,15),
          "north-pass closure begins on game day273 and ends on the next year boundary365");
    check(NationalTransportManager::seasonally_passable(273,14)&&
          NationalTransportManager::next_seasonally_passable_day(273,15)==365&&
          NationalTransportManager::next_seasonally_passable_day(272,15)==272,
          "only Atlas link15 closes and its next opening day is calculated from the absolute calendar");
    {
        const auto spring=NationalTransportManager::plan(0,16,17,1,TransportServiceGoal::Fastest,routes);
        const auto before_winter=NationalTransportManager::plan(271,16,17,1,TransportServiceGoal::Fastest,routes);
        const auto winter=NationalTransportManager::plan(272,16,17,1,TransportServiceGoal::Fastest,routes);
        const auto year_rollover=NationalTransportManager::plan(364,16,17,1,TransportServiceGoal::Fastest,routes);
        check(spring.reachable&&spring.regions==std::vector<uint8_t>{16,17}&&spring.total_days==4,
              "an open-season north-pass trip keeps its existing next-day load and three-day travel ETA");
        check(before_winter.reachable&&before_winter.regions==std::vector<uint8_t>{16,17}&&before_winter.total_days==4,
              "a cargo loaded on the last autumn day continues through a winter boundary already in transit");
        check(winter.reachable&&winter.regions==std::vector<uint8_t>{16,4,5,19,18,17}&&winter.total_days==15,
              "Fastest chooses the available fourteen-day-base detour instead of waiting through winter");
        check(year_rollover.reachable&&year_rollover.regions==std::vector<uint8_t>{16,17}&&year_rollover.total_days==4,
              "a departure ready on day365 uses the reopened north pass without a full-year delay");
    }
    {
        auto north_only=routes;
        for(auto& route:north_only)route.open=route.atlas_link==15&&route.from==16&&route.to==17;
        std::array<int64_t,kAtlasLinkCount> waiting{};
        const auto boundary=NationalTransportManager::plan(272,16,17,65,TransportServiceGoal::Fastest,north_only);
        waiting[15]=64;
        const auto queued=NationalTransportManager::plan(272,16,17,65,TransportServiceGoal::Fastest,north_only,waiting);
        check(boundary.reachable&&boundary.total_days==97,
              "a 65-unit lot resumes on the first two spring service days after the winter closure");
        check(queued.reachable&&queued.total_days==98,
              "winter days contribute no capacity when the planner accounts for queued cargo");
        auto two_leg_north=routes;
        for(auto& route:two_leg_north)route.open=(route.from==4&&route.to==16)||(route.from==16&&route.to==17);
        waiting.fill(0);
        waiting[15]=192;
        const auto queued_before_ready=NationalTransportManager::plan(270,4,17,1,
            TransportServiceGoal::Fastest,two_leg_north,waiting);
        check(queued_before_ready.reachable&&queued_before_ready.regions==std::vector<uint8_t>{4,16,17}&&
              queued_before_ready.total_days==99,
              "open autumn capacity clears part of a pass queue before the second leg reaches it");
    }

    const auto river_fast = NationalTransportManager::plan(
        0, 0, 8, 1, TransportServiceGoal::Fastest, routes);
    check(river_fast.reachable && river_fast.regions == std::vector<uint8_t>{0, 2, 1, 8} &&
          river_fast.total_days == 4 && river_fast.total_fee == 3,
          "finds the three-edge downstream river route with next-day loading, hand-checkable time and fee");
    {
        auto only_river_path=routes;
        for(size_t i=0;i<only_river_path.size();++i)
            only_river_path[i].open=std::find(river_fast.route_indices.begin(),river_fast.route_indices.end(),i)!=river_fast.route_indices.end();
        std::array<int64_t,kAtlasLinkCount> waiting{};
        waiting[only_river_path[river_fast.route_indices[1]].atlas_link]=128;
        const auto downstream_queue=NationalTransportManager::plan(
            0,0,8,1,TransportServiceGoal::Fastest,only_river_path,waiting);
        check(downstream_queue.reachable&&downstream_queue.route_indices==river_fast.route_indices&&
              downstream_queue.total_days==river_fast.total_days+1,
              "a downstream queue is reduced by capacity used while the new cargo travels toward that edge");
    }
    auto fee_choice_routes = routes;
    for (auto& route : fee_choice_routes)
        if (route.atlas_link == 4) route.fee_per_unit = 100;
    const auto fastest = NationalTransportManager::plan(
        0, 0, 4, 1, TransportServiceGoal::Fastest, fee_choice_routes);
    const auto cheapest = NationalTransportManager::plan(
        0, 0, 4, 1, TransportServiceGoal::LowestFee, fee_choice_routes);
    check(fastest.reachable && fastest.regions == std::vector<uint8_t>{0, 4} &&
          cheapest.reachable && cheapest.total_fee < fastest.total_fee &&
          cheapest.total_days > fastest.total_days,
          "service goal selects either the fast direct road or a cheaper longer route");
    {
        std::array<int64_t,kAtlasLinkCount> waiting{};
        waiting[4]=256;
        const auto still_cheapest=NationalTransportManager::plan(
            0,0,4,1,TransportServiceGoal::LowestFee,fee_choice_routes,waiting);
        check(still_cheapest.reachable&&still_cheapest.route_indices==cheapest.route_indices,
              "LowestFee keeps fee as its primary choice when the direct route has a waiting queue");
    }
    {
        auto direct_only=routes;
        for(auto& route:direct_only)route.open=route.from==0&&route.to==4;
        const auto direct=std::find_if(direct_only.begin(),direct_only.end(),[](const auto& route){return route.open;});
        check(direct!=direct_only.end(),"queue fixture locates direct route");
        const auto baseline_partial=NationalTransportManager::plan(0,0,4,80,TransportServiceGoal::Fastest,direct_only);
        const auto baseline_full=NationalTransportManager::plan(0,0,4,64,TransportServiceGoal::Fastest,direct_only);
        std::array<int64_t,kAtlasLinkCount> waiting{};
        waiting[direct->atlas_link]=16;
        const auto partial=NationalTransportManager::plan(0,0,4,80,TransportServiceGoal::Fastest,direct_only,waiting);
        waiting[direct->atlas_link]=64;
        const auto full_day=NationalTransportManager::plan(0,0,4,64,TransportServiceGoal::Fastest,direct_only,waiting);
        check(baseline_partial.reachable&&partial.reachable&&partial.total_days==baseline_partial.total_days,
              "partial queue that fits unused capacity does not delay final delivery");
        check(baseline_full.reachable&&full_day.reachable&&full_day.total_days==baseline_full.total_days+1,
              "one full day of earlier cargo delays the new order by one day on the shared atlas link");
        waiting[direct->atlas_link]=-1;
        check(!NationalTransportManager::plan(0,0,4,1,TransportServiceGoal::Fastest,direct_only,waiting).reachable,
              "negative queued cargo rejects the planning snapshot");
        waiting[direct->atlas_link]=std::numeric_limits<int64_t>::max();
        check(!NationalTransportManager::plan(0,0,4,1,TransportServiceGoal::Fastest,direct_only,waiting).reachable,
              "queued quantity addition overflow fails closed");
    }
    const auto reverse_trip = NationalTransportManager::plan(
        0, 8, 0, 1, TransportServiceGoal::Fastest, routes);
    check(reverse_trip.reachable && reverse_trip.regions.size() > 2 &&
          !(reverse_trip.regions[0] == 8 && reverse_trip.regions[1] == 1),
          "finds a valid detour rather than silently reversing the river");

    auto closed = routes;
    for (auto& route : closed) route.open = false;
    check(!NationalTransportManager::plan(0, 0, 8, 1, TransportServiceGoal::Fastest, closed).reachable,
          "rejects a trip when every route is closed");

    auto bad_river = routes;
    bad_river.push_back({0, 2, 0, AtlasRouteMode::River, true, 1, 64, 1,
                         TransportFeeAccount::ProvinceTreasury, 2});
    check(!NationalTransportManager::routes_are_well_formed(bad_river),
          "rejects an explicitly configured upstream river edge");
    auto missing_recipient = routes;
    missing_recipient[0].fee_recipient_region = -1;
    check(!NationalTransportManager::routes_are_well_formed(missing_recipient),
          "rejects a route without a valid fee recipient");
    auto duplicate = routes;
    duplicate.push_back(routes.front());
    check(!NationalTransportManager::routes_are_well_formed(duplicate),
          "rejects a duplicate direction for one atlas link");
    auto bad_time = routes;
    bad_time[0].travel_days = 0;
    check(!NationalTransportManager::routes_are_well_formed(bad_time),
          "rejects a zero-day route");
    auto excessive_time=routes;
    excessive_time[0].travel_days=365001;
    check(!NationalTransportManager::routes_are_well_formed(excessive_time),
          "rejects travel duration beyond the supported save/day horizon");
    auto excessive_capacity=routes;
    excessive_capacity[0].capacity_per_day=1000001;
    check(!NationalTransportManager::routes_are_well_formed(excessive_capacity),
          "rejects capacity outside the supported integer bound");
    check(!NationalTransportManager::routes_are_well_formed(routes, 6),
          "does not pretend the 27-region national atlas is a six-region Realm");

    auto overflow = routes;
    for (auto& route : overflow) route.open = false;
    overflow[0].open = true;
    overflow[0].fee_per_unit = std::numeric_limits<int64_t>::max();
    const auto overflow_plan = NationalTransportManager::plan(
        0, 0, 2, 2, TransportServiceGoal::Fastest, overflow);
    check(!overflow_plan.reachable,
          "fails closed when total fee multiplication exceeds the money range");

    if (failures) return 1;
    std::cout << "NATIONAL TRANSPORT MANAGER: " << checks << '/' << checks
              << " checks passed\n";
    return 0;
}






