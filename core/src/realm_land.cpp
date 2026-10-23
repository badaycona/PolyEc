#include "polyec/realm.hpp"

#include <algorithm>
#include <limits>

namespace polyec {

RealmResult Realm::execute(const LandTitleTransferCommand& command) {
    if (command.actor == 0 || command.actor > people_.size()) return {false, "Unknown actor"};
    auto& actor = mutable_person(command.actor);
    if (actor.decisions == UINT64_MAX || actor.sampling == UINT64_MAX)
        return {false, "Decision counter overflow"};
    if (command.sequence != actor.decisions + 1) return {false, "Stale or duplicate decision ID"};
    if (!actor.alive) return {false, "Actor deceased"};
    if (actor.age_days < 18 * 365) return {false, "Title transfer requires adult holder"};
    if (command.recipient == 0 || command.recipient > people_.size()) return {false, "Unknown recipient"};
    const auto& recipient = person(command.recipient);
    if (!recipient.alive) return {false, "Recipient deceased"};
    if (command.recipient == command.actor) return {false, "Recipient is current title holder"};

    auto found = std::lower_bound(land_holdings_.begin(), land_holdings_.end(), command.holding_id,
        [](const LandHolding& holding, uint64_t id) { return holding.id < id; });
    if (found == land_holdings_.end() || found->id != command.holding_id)
        return {false, "Unknown land title"};
    if (found->title_holder != command.actor) return {false, "Actor is not title holder"};
    if (std::any_of(land_use_rights_.begin(), land_use_rights_.end(), [&](const LandUseRight& right) {
            return right.holding_id == command.holding_id;
        })) return {false, "Revoke land use right before title transfer"};
    if (next_fact_ == UINT64_MAX) return {false, "Fact counter overflow"};

    // Reserve the event before the first authoritative write. The transfer
    // changes one scalar; money, area, region and title ID remain unchanged.
    if (facts_.size() == facts_.capacity()) facts_.reserve(facts_.size() + 1);
    const int region = found->region;
    const int64_t area = found->area_units;
    found->title_holder = command.recipient;
    actor.last_decision_day = day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id, region, 212, area, command.recipient);
    return {true, "Committed", next_fact_ - 1};
}

RealmResult Realm::execute(const LandUseGrantCommand& command) {
    if (command.actor == 0 || command.actor > people_.size()) return {false, "Unknown actor"};
    auto& actor = mutable_person(command.actor);
    if (actor.decisions == UINT64_MAX || actor.sampling == UINT64_MAX)
        return {false, "Decision counter overflow"};
    if (command.sequence != actor.decisions + 1) return {false, "Stale or duplicate decision ID"};
    if (!actor.alive) return {false, "Actor deceased"};
    if (actor.age_days < 18 * 365) return {false, "Land use requires adult title holder"};
    if (command.grantee == 0 || command.grantee > people_.size()) return {false, "Unknown grantee"};
    const auto& grantee = person(command.grantee);
    if (!grantee.alive) return {false, "Grantee deceased"};
    if (command.grantee == command.actor) return {false, "Title holder cannot grant use to self"};

    auto holding = std::lower_bound(land_holdings_.begin(), land_holdings_.end(), command.holding_id,
        [](const LandHolding& value, uint64_t id) { return value.id < id; });
    if (holding == land_holdings_.end() || holding->id != command.holding_id)
        return {false, "Unknown land title"};
    const auto& title_holder = person(holding->title_holder);
    const bool guardian_admin = title_holder.alive && title_holder.age_days < 18 * 365 &&
        guardian_of(title_holder) == command.actor;
    if (holding->title_holder != command.actor && !guardian_admin)
        return {false, "Actor is not title holder or dependent guardian"};
    if (next_id_ == UINT64_MAX) return {false, "Record ID overflow"};
    if (next_fact_ == UINT64_MAX) return {false, "Fact counter overflow"};

    facts_.reserve(facts_.size() + 1);
    land_use_rights_.reserve(land_use_rights_.size() + 1);
    auto existing = std::find_if(land_use_rights_.begin(), land_use_rights_.end(),
        [&](const LandUseRight& right) { return right.holding_id == command.holding_id; });
    const uint64_t rightId = next_id_++;
    if (existing != land_use_rights_.end()) land_use_rights_.erase(existing);
    land_use_rights_.push_back({rightId, command.holding_id, command.grantee, holding->title_holder});
    actor.last_decision_day = day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id, holding->region, 213, holding->area_units, command.grantee);
    return {true, "Committed", next_fact_ - 1};
}

RealmResult Realm::execute(const LandUseRevokeCommand& command) {
    if (command.actor == 0 || command.actor > people_.size()) return {false, "Unknown actor"};
    auto& actor = mutable_person(command.actor);
    if (actor.decisions == UINT64_MAX || actor.sampling == UINT64_MAX)
        return {false, "Decision counter overflow"};
    if (command.sequence != actor.decisions + 1) return {false, "Stale or duplicate decision ID"};
    if (!actor.alive) return {false, "Actor deceased"};
    if (actor.age_days < 18 * 365) return {false, "Land use requires adult title holder"};

    auto holding = std::lower_bound(land_holdings_.begin(), land_holdings_.end(), command.holding_id,
        [](const LandHolding& value, uint64_t id) { return value.id < id; });
    if (holding == land_holdings_.end() || holding->id != command.holding_id)
        return {false, "Unknown land title"};
    const auto& title_holder = person(holding->title_holder);
    const bool guardian_admin = title_holder.alive && title_holder.age_days < 18 * 365 &&
        guardian_of(title_holder) == command.actor;
    if (holding->title_holder != command.actor && !guardian_admin)
        return {false, "Actor is not title holder or dependent guardian"};
    auto right = std::find_if(land_use_rights_.begin(), land_use_rights_.end(),
        [&](const LandUseRight& value) { return value.holding_id == command.holding_id; });
    if (right == land_use_rights_.end()) return {false, "No active land use right"};
    if (next_fact_ == UINT64_MAX) return {false, "Fact counter overflow"};

    facts_.reserve(facts_.size() + 1);
    const int region = holding->region;
    land_use_rights_.erase(right);
    actor.last_decision_day = day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id, region, 214, 0, command.holding_id);
    return {true, "Committed", next_fact_ - 1};
}

} // namespace polyec
