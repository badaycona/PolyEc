#pragma once

#include <cstdint>

namespace polyec {

// A private home is attached to an explicit land title. Its lease is a
// household-to-household agreement, not an inferred use of land area.
struct PrivateResidence {
    uint64_t id=0,holding_id=0,tenant_household=0,offered_household=0;
    int region=0,construction_due_day=0,next_rent_day=-1,lease_end_day=-1,offer_expires_day=-1;
    int64_t rent_per_period=0,offered_rent=0;
    bool completed=false;
    bool operator==(const PrivateResidence&) const = default;
};

enum class PrivateHousingOperation : uint8_t { Offer=0, Accept=1, Decline=2, EndLease=3 };

struct PrivateHousingCommand {
    uint64_t actor=0,sequence=0,residence_id=0,tenant_household=0;
    PrivateHousingOperation operation=PrivateHousingOperation::Offer;
    int64_t rent_per_period=0;
};

struct PrivateResidenceBuildCommand {
    uint64_t actor=0,sequence=0,holding_id=0;
};

} // namespace polyec
