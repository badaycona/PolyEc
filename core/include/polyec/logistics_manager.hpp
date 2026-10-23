#pragma once

#include <cstdint>
#include <vector>

namespace polyec {

struct LogisticsRouteInput {
    int from = 0;
    int to = 0;
    int travel_days = 1;
    int capacity_per_day = 1;
    int additional_days = 0;
    bool open = true;
    // Campaign routes retain their historical two-way behavior by default.
    // National atlas rivers set this false to forbid an implicit upstream trip.
    bool allow_reverse = true;
    bool operator==(const LogisticsRouteInput&) const = default;
};

struct LogisticsReservationInput {
    int from = 0;
    int to = 0;
    int first_load_day = 0;
    int last_load_day = 0;
    bool operator==(const LogisticsReservationInput&) const = default;
};

struct LogisticsPathPlan {
    bool reachable = false;
    int days = 0;
    std::vector<int> regions;
    std::vector<int> leg_starts;
    std::vector<int> leg_ends;
    std::vector<int> leg_arrivals;
};

class LogisticsManager {
public:
    static constexpr int max_regions = 27;

    // Load slots cover both endpoints of each integer day. Reservations use
    // the same inclusive convention and are shared by travel in both directions.
    // Equal arrivals retain the first predecessor: regions are examined by ID,
    // then adjacent routes in input order, matching Campaign's prior behavior.
    // Invalid inputs fail closed with an unreachable result; inputs are read-only.
    static LogisticsPathPlan plan(
        int region_count,
        int current_day,
        int from,
        int to,
        int64_t quantity,
        const std::vector<LogisticsRouteInput>& routes,
        const std::vector<LogisticsReservationInput>& reservations);
};

} // namespace polyec
