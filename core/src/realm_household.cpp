#include "polyec/realm.hpp"

#include <limits>
#include <map>

namespace polyec {

int64_t Realm::household_fund(uint64_t household) const {
    if (!household || household > people_.size()) return 0;
    const auto found = household_funds_.find(household);
    return found == household_funds_.end() ? 0 : found->second;
}

RealmResult Realm::execute(const HouseholdFundCommand& cmd) {
    if (cmd.actor == 0 || cmd.actor > people_.size()) return {false, "Unknown actor"};
    auto& actor = mutable_person(cmd.actor);
    if (actor.decisions == UINT64_MAX || actor.sampling == UINT64_MAX)
        return {false, "Decision counter overflow"};
    if (cmd.sequence != actor.decisions + 1) return {false, "Stale or duplicate decision ID"};
    if (!actor.alive) return {false, "Actor deceased"};
    if (!cmd.household_id || cmd.household_id > people_.size() ||
        actor.household_membership.household_id != cmd.household_id)
        return {false, "Actor is not a household member"};
    if (cmd.amount <= 0) return {false, "Amount must be positive"};

    const bool deposit = cmd.operation == HouseholdFundOperation::Deposit;
    if (!deposit && cmd.operation != HouseholdFundOperation::Withdraw)
        return {false, "Invalid household fund operation"};
    if (deposit) {
        if (actor.age_days < 18 * 365) return {false, "Deposit requires adult household member"};
        if (actor.cash < cmd.amount) return {false, "Insufficient personal funds"};
    } else if (household_authority(cmd.household_id) != actor.id) {
        return {false, "Actor cannot withdraw household funds"};
    }

    auto found = household_funds_.find(cmd.household_id);
    const int64_t balance = found == household_funds_.end() ? 0 : found->second;
    if (deposit && balance > std::numeric_limits<int64_t>::max() - cmd.amount)
        return {false, "Household fund overflow"};
    if (!deposit && balance < cmd.amount) return {false, "Insufficient household funds"};
    if (!deposit && actor.cash > std::numeric_limits<int64_t>::max() - cmd.amount)
        return {false, "Personal funds overflow"};
    if (next_fact_ == UINT64_MAX) return {false, "Fact counter overflow"};

    // Prepare a new sparse account node and event capacity before authoritative writes.
    std::map<uint64_t, int64_t> staged;
    if (deposit && found == household_funds_.end()) staged.emplace(cmd.household_id, cmd.amount);
    if (facts_.size() <= 8192 && facts_.size() == facts_.capacity()) facts_.reserve(facts_.size() + 1);

    if (deposit) {
        actor.cash -= cmd.amount;
        if (found == household_funds_.end()) household_funds_.insert(staged.extract(cmd.household_id));
        else found->second += cmd.amount;
    } else {
        found->second -= cmd.amount;
        actor.cash += cmd.amount;
        if (found->second == 0) household_funds_.erase(found);
    }
    actor.last_decision_day = day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id, actor.region, deposit ? 210 : 211, cmd.amount, 0, true);
    return {true, "Committed", next_fact_ - 1};
}

} // namespace polyec
