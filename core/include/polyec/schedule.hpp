#pragma once
#include "polyec/activity.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace polyec {

inline constexpr int kSubdayUnitsPerDay = 64;

struct ActivitySession {
    uint64_t id = 0;
    uint64_t actor = 0;
    int64_t begin = 0;
    int64_t end = 0;
    int32_t priority = 0;
    int32_t demand_rate = 0;
    int64_t requested = 0;
    int64_t delivered = 0;
    int64_t unmet = 0;
    int64_t capacity_unmet = 0;
    int64_t unresolved_unmet = 0;
    bool retired = false;
    int64_t retired_tick = 0;
    uint64_t retired_cause = 0;
};

struct ScheduleBook {
    int64_t cursor = 0;
    bool timeline_contract = false;
    uint64_t next_session_id = 1;
    std::unordered_map<uint64_t, std::vector<ActivitySession>> sessions;
};

struct ScheduleCommand {
    enum class Operation { Submit, Cancel, Reprioritize };
    uint64_t actor = 0;
    uint64_t sequence = 0;
    Operation operation = Operation::Submit;
    uint64_t session = 0;
    int64_t begin = 0;
    int64_t end = 0;
    int32_t priority = 0;
    int32_t demand_rate = 0;
};

} // namespace polyec
