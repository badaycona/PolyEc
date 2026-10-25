#pragma once

#include "polyec/world_atlas.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace polyec {

enum class TransportFeeAccount : uint8_t { ProvinceTreasury = 0 };
enum class TransportServiceGoal : uint8_t { Fastest = 0, LowestFee = 1 };

struct NationalTransportRoute {
    uint8_t atlas_link = 0;
    uint8_t from = 0;
    uint8_t to = 0;
    AtlasRouteMode mode = AtlasRouteMode::Unspecified;
    bool open = true;
    int travel_days = 1;
    int capacity_per_day = 1;
    int64_t fee_per_unit = 0;
    TransportFeeAccount fee_account = TransportFeeAccount::ProvinceTreasury;
    int fee_recipient_region = -1;
    bool operator==(const NationalTransportRoute&) const = default;
};

struct NationalTransportPathPlan {
    bool reachable = false;
    int total_days = 0;
    int64_t total_fee = 0;
    std::vector<uint8_t> regions;
    std::vector<size_t> route_indices;
};

// Converts the approved atlas topology into explicit directed service records
// and selects a route for a policy-supplied service goal. It owns no Realm data.
class NationalTransportManager {
public:
    static constexpr uint32_t config_version = 2;
    static constexpr int atlas_year_days = 365;
    static constexpr int north_pass_winter_start = 273;
    static constexpr int north_pass_winter_end = 365;
    static constexpr uint8_t north_pass_atlas_link = 15;
    static std::vector<NationalTransportRoute> default_routes();
    static bool seasonally_passable(int absolute_day, uint8_t atlas_link) noexcept;
    static int next_seasonally_passable_day(int absolute_day, uint8_t atlas_link) noexcept;
    static bool routes_are_well_formed(
        const std::vector<NationalTransportRoute>& routes,
        int region_count = int(kAtlasTerritoryCount)) noexcept;
    static NationalTransportPathPlan plan(
        int current_day,
        uint8_t from,
        uint8_t to,
        int64_t quantity,
        TransportServiceGoal goal,
        const std::vector<NationalTransportRoute>& routes,
        const std::array<int64_t,kAtlasLinkCount>& waiting_units_by_link = {});
};

} // namespace polyec
