#pragma once
#include <cstdint>
#include <map>
#include <span>
#include <utility>
#include <vector>

namespace polyec {
// Accounting credit for work already entitled under the regional wage rule.
// Credit is not cash and does not imply that the historical wage was paid.
struct WorkRegionCredit {
    int64_t legacy_units=0,unassigned_units=0,bridge_units=0;
    bool operator==(const WorkRegionCredit&) const = default;
};
using WorkCreditRegions=std::map<std::pair<uint64_t,int>,WorkRegionCredit>;
struct WorkCreditSource {
    uint64_t establishment_id=0;
    int64_t eligible_units=0,credited_units=0;
    bool operator==(const WorkCreditSource&) const = default;
};
struct WorkCreditAllocation {
    WorkRegionCredit region;
    std::vector<WorkCreditSource> sites;
    int64_t regional_entitlement=0;
};
// Pure migration baseline. Sources must have zero prior credit, positive IDs
// in ascending order. No worker/account/world state is owned or changed here.
WorkCreditAllocation initialize_work_credit(int64_t legacy_units,int64_t unassigned_eligible,
                                            std::span<const WorkCreditSource> sites);
// Pure pro-rata bridge allocation, rotating rounding over positive available
// claims in canonical order (unassigned, then ascending establishment ID).
WorkCreditAllocation allocate_work_credit(const WorkRegionCredit& region,int64_t unassigned_eligible,
                                          std::span<const WorkCreditSource> sites,int64_t day);
}
