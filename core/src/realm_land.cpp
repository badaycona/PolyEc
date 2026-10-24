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

RealmResult Realm::execute(const PrivateResidenceBuildCommand& command) {
    if (command.actor == 0 || command.actor > people_.size()) return {false, "Unknown actor"};
    auto& actor = mutable_person(command.actor);
    if (actor.decisions == UINT64_MAX || actor.sampling == UINT64_MAX)
        return {false, "Decision counter overflow"};
    if (command.sequence != actor.decisions + 1) return {false, "Stale or duplicate decision ID"};
    if (!actor.alive || actor.age_days < 18 * 365 || regions_ != int(kAtlasTerritoryCount)-1 ||
        actor.destination != -1 || actor.busy_until > day_ || actor.residence_kind != ResidenceKind::Urban)
        return {false, "A private home requires an available adult living at an Atlas urban site"};
    const auto holding = std::lower_bound(land_holdings_.begin(), land_holdings_.end(), command.holding_id,
        [](const LandHolding& value, uint64_t id) { return value.id < id; });
    if (holding == land_holdings_.end() || holding->id != command.holding_id ||
        holding->title_holder != command.actor || holding->region != actor.urban_site_id ||
        holding->region != actor.region)
        return {false, "Actor must own an urban holding at their current residence"};
    if (std::any_of(private_residences_.begin(), private_residences_.end(), [&](const auto& home) {
            return home.holding_id == command.holding_id;
        })) return {false, "This holding already has a private home"};
    const auto material = size_t(GoodId::ConstructionMaterial);
    const auto tools = size_t(GoodId::Tools);
    if (actor.cash < kPrivateResidenceBuildCost ||
        actor.inventory[material] < kPrivateResidenceConstructionMaterial ||
        actor.inventory[tools] < kPrivateResidenceTools)
        return {false, "Building requires 40 personal coins, two ConstructionMaterial, and one Tools"};
    if (day_ > 365000-kPrivateResidenceBuildDays || next_id_ == UINT64_MAX || next_fact_ == UINT64_MAX ||
        consumed_[material] > std::numeric_limits<int64_t>::max()-kPrivateResidenceConstructionMaterial ||
        consumed_[tools] > std::numeric_limits<int64_t>::max()-kPrivateResidenceTools)
        return {false, "Private home construction limit reached"};

    // Reserve every container before changing balances so allocation failure
    // cannot leave the construction paid for without a project record.
    private_residences_.reserve(private_residences_.size()+1);
    facts_.reserve(facts_.size()+1);
    const auto id = next_id_++;
    transfer(actor.cash,provinces_[size_t(holding->region)].firm,kPrivateResidenceBuildCost);
    actor.inventory[material]-=kPrivateResidenceConstructionMaterial;
    actor.inventory[tools]-=kPrivateResidenceTools;
    consumed_[material]+=kPrivateResidenceConstructionMaterial;
    consumed_[tools]+=kPrivateResidenceTools;
    private_residences_.push_back({id,command.holding_id,0,0,holding->region,
        day_+kPrivateResidenceBuildDays,-1,-1,-1,0,0,false});
    actor.last_decision_day=day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id,holding->region,250,kPrivateResidenceBeds,id);
    return {true,"Committed",next_fact_-1};
}

RealmResult Realm::execute(const PrivateHousingCommand& command) {
    if (command.actor == 0 || command.actor > people_.size()) return {false,"Unknown actor"};
    auto& actor=mutable_person(command.actor);
    if(actor.decisions==UINT64_MAX||actor.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
    if(command.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!actor.alive||regions_!=int(kAtlasTerritoryCount)-1)return {false,"Private leases are only available in Atlas"};
    if(command.rent_per_period!=kPrivateResidenceRentPerPeriod&&command.operation==PrivateHousingOperation::Offer)
        return {false,"Trial rent is fixed at two coins per thirty game days"};
    auto home=std::find_if(private_residences_.begin(),private_residences_.end(),[&](const auto& value){return value.id==command.residence_id;});
    if(home==private_residences_.end())return {false,"Unknown private home"};
    const auto holding=std::lower_bound(land_holdings_.begin(),land_holdings_.end(),home->holding_id,
        [](const LandHolding& value,uint64_t id){return value.id<id;});
    if(holding==land_holdings_.end()||holding->id!=home->holding_id)return {false,"Private home has no land title"};
    if(command.operation==PrivateHousingOperation::Offer){
        if(!home->completed||holding->title_holder!=command.actor||home->tenant_household||home->offered_household||
           command.tenant_household==0||command.tenant_household>people_.size()||
           household_authority(command.tenant_household)!=command.tenant_household||
           command.tenant_household==holding->title_holder||next_fact_==UINT64_MAX||day_>365000-kPrivateResidenceOfferDays)
            return {false,"Only the living title holder can offer a completed vacant home to another household authority"};
        const auto& tenant=person(command.tenant_household);
        if(!tenant.alive||tenant.age_days<18*365||tenant.residence_kind!=ResidenceKind::Urban||
           tenant.urban_site_id>=kAtlasTerritoryCount)
            return {false,"Tenant must be an adult household authority in an Atlas urban household"};
        for(const auto& other:private_residences_)if(other.tenant_household==command.tenant_household||
            other.offered_household==command.tenant_household)return {false,"Household already has a private-home lease or offer"};
        facts_.reserve(facts_.size()+1);
        home->offered_household=command.tenant_household;
        home->offered_rent=command.rent_per_period;
        home->offer_expires_day=day_+kPrivateResidenceOfferDays;
        actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
        event(actor.id,home->region,251,home->offered_rent,home->id);
        return {true,"Committed",next_fact_-1};
    }
    if(command.operation==PrivateHousingOperation::Accept||command.operation==PrivateHousingOperation::Decline){
        if(home->offered_household!=command.tenant_household||command.tenant_household==0||
           household_authority(command.tenant_household)!=command.actor||home->offer_expires_day<day_)
            return {false,"This lease offer is not addressed to the actor's household"};
        facts_.reserve(facts_.size()+1);
        if(command.operation==PrivateHousingOperation::Accept){
            if(!home->completed||holding->title_holder==command.tenant_household||
               actor.residence_kind!=ResidenceKind::Urban||actor.urban_site_id>=kAtlasTerritoryCount||
               day_>365000-kPrivateResidenceLeaseDays||day_>365000-kPrivateResidenceRentPeriodDays)
                return {false,"Tenant must still live at this site and the home must be ready"};
            home->tenant_household=command.tenant_household;
            home->rent_per_period=home->offered_rent;
            home->next_rent_day=day_+kPrivateResidenceRentPeriodDays;
            home->lease_end_day=day_+kPrivateResidenceLeaseDays;
            home->offered_household=0;home->offered_rent=0;home->offer_expires_day=-1;
            event(actor.id,home->region,252,home->rent_per_period,home->id);
        }else{
            home->offered_household=0;home->offered_rent=0;home->offer_expires_day=-1;
            event(actor.id,home->region,253,0,home->id);
        }
        actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
        return {true,"Committed",next_fact_-1};
    }
    if(command.operation==PrivateHousingOperation::EndLease){
        const bool landlord=holding->title_holder==command.actor;
        const bool tenant=home->tenant_household&&household_authority(home->tenant_household)==command.actor;
        if(!home->tenant_household||(!landlord&&!tenant))return {false,"Actor is neither the landlord nor tenant household authority"};
        facts_.reserve(facts_.size()+1);
        home->tenant_household=0;home->rent_per_period=0;home->next_rent_day=-1;home->lease_end_day=-1;
        actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
        event(actor.id,home->region,254,0,home->id);
        return {true,"Committed",next_fact_-1};
    }
    return {false,"Unknown private housing operation"};
}

} // namespace polyec
