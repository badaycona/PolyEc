#pragma once
#include "polyec/activity.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace polyec {

inline constexpr int kSubdayUnitsPerDay = 64;
// One settled attendance work unit is 16 physical Work ticks. T=16 is a
// Codex-delegated working assumption, not gameplay/balance approval.
inline constexpr int64_t kAttendanceTicksPerWorkUnit = 16;
// P6.2 trial-1: evaluator attention is capped at 1,000 units per game tick.
// One fully delivered tick while physically present counts as one effort tick.
inline constexpr int64_t kAttentionUnitsPerEffortTick = 1000;
// One completed 16-tick work unit pays one cash. This is a Codex-delegated
// working assumption, not balance approval.
inline constexpr int64_t kWorkWagePerUnit = 1;

// Sparse key for pay attendance/watermarks: (actor_id, physical_region).
// std::map keeps deterministic iteration in ascending (actor, region) order;
// callers never rely on unordered iteration to build money batches.
using WorkRegionKey = std::pair<uint64_t, int>;
// Site-qualified attendance is sparse by (actor_id, establishment_id).
using WorkEstablishmentKey = std::pair<uint64_t, uint64_t>;

// Work sessions need a physical province so attendance is based on presence in
// the region the Work was scheduled for. Generic sessions keep the historical
// behavior and carry no region.
enum class ActivityKind : int { Generic = 0, Work = 1 };

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
    ActivityKind kind = ActivityKind::Generic;
    int region = -1;
    uint64_t establishment_id = 0;
};

struct ScheduleBook {
    int64_t cursor = 0;
    bool timeline_contract = false;
    uint64_t next_session_id = 1;
    std::unordered_map<uint64_t, std::vector<ActivitySession>> sessions;
    // Cumulative unique physical Work ticks per actor. Sparse by design.
    std::unordered_map<uint64_t, int64_t> work_present_ticks;
    // Completed-day watermark and cumulative settled Work units per actor.
    // Units are lifetime bookkeeping only; they are not money, wage, output,
    // attention or effort. Sparse and positive-only by design.
    int64_t settled_day = 0;
    std::unordered_map<uint64_t, int64_t> settled_work_units;
    // Post-migration physical Work ticks per (actor, work region). Positive
    // only and gated on the timeline contract. Migrated global ticks above are
    // deliberately un-attributed and never become regional attendance.
    std::map<WorkRegionKey, int64_t> work_present_ticks_by_region;
    // Cumulative completed 16-tick blocks already paid per (actor, region).
    // Positive-only and monotonic; never above regional eligible units.
    std::map<WorkRegionKey, int64_t> paid_work_units_by_region;
    // Cumulative unique physical Work ticks per actor and establishment. Site
    // zero is legacy-only and is never entered in this map.
    std::map<WorkEstablishmentKey, int64_t> work_present_ticks_by_establishment;
    // Cumulative delivered attention credited as effort only while the actor
    // was physically present at the corresponding worksite. One thousand
    // attention units equal one effort tick under the current trial rule.
    std::map<WorkEstablishmentKey, int64_t> work_effort_attention_by_establishment;
    // Cumulative attention weighted by the worker's labor skill when each
    // interval was credited. The denominator is 200; raw effort above remains
    // the source for exertion and attendance conservation.
    std::map<WorkEstablishmentKey, int64_t> work_productivity_attention_numerator_by_establishment;
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
    ActivityKind kind = ActivityKind::Generic;
    int region = -1;
    uint64_t establishment_id = 0;
};

} // namespace polyec
