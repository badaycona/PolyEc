#pragma once

#include <cstdint>

namespace polyec {

// Abstract, region-scoped land title. Geometry, rent, and jurisdiction are
// separate systems; use permission is stored as an independent LandUseRight.
struct LandHolding {
    uint64_t id = 0;
    int region = 0;
    int64_t area_units = 0;
    uint64_t title_holder = 0;
    bool operator==(const LandHolding&) const = default;
};

// Explicit, unpriced title transfer. Realm remains the sole state mutator.
struct LandTitleTransferCommand {
    uint64_t actor = 0;
    uint64_t sequence = 0;
    uint64_t holding_id = 0;
    uint64_t recipient = 0;
};

// A revocable, unpriced permission for one person to use one whole holding.
struct LandUseRight {
    uint64_t id = 0;
    uint64_t holding_id = 0;
    uint64_t grantee = 0;
    uint64_t grantor = 0;
    bool operator==(const LandUseRight&) const = default;
};

struct LandUseGrantCommand {
    uint64_t actor = 0;
    uint64_t sequence = 0;
    uint64_t holding_id = 0;
    uint64_t grantee = 0;
};

struct LandUseRevokeCommand {
    uint64_t actor = 0;
    uint64_t sequence = 0;
    uint64_t holding_id = 0;
};

} // namespace polyec
