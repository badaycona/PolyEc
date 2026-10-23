#pragma once

#include <cstdint>

namespace polyec {

enum class HouseholdFundOperation : int { Deposit = 0, Withdraw = 1 };

struct HouseholdFundCommand {
    uint64_t actor = 0;
    uint64_t sequence = 0;
    uint64_t household_id = 0;
    HouseholdFundOperation operation = HouseholdFundOperation::Deposit;
    int64_t amount = 0;
};

} // namespace polyec
