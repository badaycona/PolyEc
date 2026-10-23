#include "polyec/realm.hpp"

#include <algorithm>
#include <limits>
#include <set>

namespace polyec {

std::vector<uint64_t> Realm::military_battle_candidates(int region)const{
    std::vector<uint64_t> result;
    if(region!=kMilitaryRaidTargetRegion||regions_!=int(kAtlasTerritoryCount)-1)return result;
    for(const auto& volunteer:people_){
        const auto& service=volunteer.military_service;
        if(volunteer.alive&&volunteer.health>0&&volunteer.family==Family::Civilian&&volunteer.region==region&&
           service.active&&service.combat_consent&&day_>=service.start_day&&day_<service.end_day&&
           service.mobilized_until_day>day_)result.push_back(volunteer.id);
    }
    return result;
}

RealmResult Realm::execute(const MilitaryRaidCommand& command){
    if(command.partner_id<0||command.partner_id>=kForeignTradePartnerCount)
        return {false,"Unknown external power"};
    if(regions_!=int(kAtlasTerritoryCount)-1)
        return {false,"Coastal raids require the Atlas Realm profile"};
    auto& partner=foreign_trade_partners_[size_t(command.partner_id)];
    if(partner.decision_sequence==std::numeric_limits<uint64_t>::max()||next_id_==std::numeric_limits<uint64_t>::max()||
       next_fact_==std::numeric_limits<uint64_t>::max())return {false,"Raid sequence or record ID overflow"};
    if(command.sequence!=partner.decision_sequence+1)
        return {false,"Stale or duplicate external-power decision sequence"};
    if(command.force_units<1||command.force_units>kMilitaryRaidUnitMaximum)
        return {false,"Raid force must contain 1–8 units"};
    if(std::any_of(military_raids_.begin(),military_raids_.end(),[](const MilitaryRaid& raid){
        return raid.status==MilitaryRaidStatus::Pending;
    }))return {false,"Another coastal raid is still awaiting a response"};
    const auto sea_route=foreign_trade_route_plan(ForeignTradeOperation::Import,1);
    if(!sea_route.reachable)return {false,"Coastal corridor to Nam Cang is closed or unavailable"};
    const int64_t cost=command.force_units*kMilitaryRaidCashPerUnit;
    if(partner.money<cost||partner.goods[size_t(GoodId::DryFood)]<command.force_units)
        return {false,"External power lacks the cash or DryFood to fund the raid"};
    if(carrier_>std::numeric_limits<int64_t>::max()-cost||
       consumed_[size_t(GoodId::DryFood)]>std::numeric_limits<int64_t>::max()-command.force_units)
        return {false,"Raid supply ledger overflow"};

    military_raids_.reserve(military_raids_.size()+1);
    MilitaryRaid raid;raid.id=next_id_;raid.partner_id=command.partner_id;
    raid.target_region=kMilitaryRaidTargetRegion;raid.created_day=day_;raid.force_units=command.force_units;
    transfer(partner.money,carrier_,cost);
    partner.goods[size_t(GoodId::DryFood)]-=command.force_units;
    consumed_[size_t(GoodId::DryFood)]+=command.force_units;
    ++partner.decision_sequence;++next_id_;
    military_raids_.push_back(raid);
    event(1,kMilitaryRaidTargetRegion,248,command.force_units,command.partner_id+1);
    return {true,"External power funded a coastal raid against Nam Cang",next_fact_-1,raid.id};
}

RealmResult Realm::execute(const MilitaryBattleCommand& command){
    if(command.actor==0||command.actor>people_.size())return {false,"Unknown actor"};
    auto& actor=mutable_person(command.actor);
    if(!actor.alive)return {false,"Actor deceased"};
    if(actor.decisions==std::numeric_limits<uint64_t>::max()||actor.sampling==std::numeric_limits<uint64_t>::max()||
       next_fact_==std::numeric_limits<uint64_t>::max())return {false,"Decision or fact counter overflow"};
    if(command.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!actor.sovereign||actor.family!=Family::Governance)
        return {false,"Only the Sovereign may order port defense or retreat"};
    const auto raid_it=std::find_if(military_raids_.begin(),military_raids_.end(),[&](const MilitaryRaid& raid){
        return raid.id==command.raid_id&&raid.status==MilitaryRaidStatus::Pending;
    });
    if(raid_it==military_raids_.end())return {false,"No pending raid with this record ID"};
    if(command.retreat&&!command.defenders.empty())return {false,"Retreat cannot also commit defenders"};
    if(!command.retreat&&(command.defenders.empty()||command.defenders.size()>16))
        return {false,"Defense requires 1–16 named volunteers"};

    std::vector<uint64_t> defenders=command.defenders;
    std::sort(defenders.begin(),defenders.end());
    if(std::adjacent_find(defenders.begin(),defenders.end())!=defenders.end())
        return {false,"A volunteer may only be listed once"};
    int defense_score=0;
    const auto battleReady=military_battle_candidates(raid_it->target_region);
    for(const uint64_t id:defenders){
        if(id==0||id>people_.size())return {false,"Unknown defending volunteer"};
        const auto& volunteer=person(id);
        if(!std::binary_search(battleReady.begin(),battleReady.end(),id))
            return {false,"Every defender must be alive, healthy, consenting, mobilized and present at Nam Cang"};
        const int military=capability_value(id,LegacyCapability::Military);
        const int weapon=volunteer.inventory[size_t(GoodId::Weapon)]>0?1:0;
        defense_score+=1+military/50+weapon;
    }
    if(provinces_[size_t(raid_it->target_region)].stock[size_t(GoodId::Salt)]<0)
        return {false,"Port Salt ledger is invalid"};

    auto& raid=*raid_it;
    const bool repelled=!command.retreat&&defense_score>raid.force_units;
    const int attacker_losses=command.retreat?0:(repelled?raid.force_units:std::min(raid.force_units,defense_score/2));
    const int surviving_attackers=raid.force_units-attacker_losses;
    int defender_casualties=0;
    if(!command.retreat&&!repelled){
        defender_casualties=std::min<int>(int(defenders.size()),std::max(1,(raid.force_units+1)/2));
    }
    int64_t looted=0;
    auto& partner=foreign_trade_partners_[size_t(raid.partner_id)];
    auto& port=provinces_[size_t(raid.target_region)];
    if(!repelled&&surviving_attackers>0){
        looted=std::min<int64_t>({surviving_attackers,port.stock[size_t(GoodId::Salt)],partner.salt_capacity});
        if(looted>std::numeric_limits<int64_t>::max()-partner.goods[size_t(GoodId::Salt)])
            return {false,"Partner Salt ledger overflow"};
    }
    const uint64_t required_facts=1+uint64_t(defender_casualties>0)+uint64_t(looted>0);
    if(next_fact_>std::numeric_limits<uint64_t>::max()-required_facts)
        return {false,"Battle fact counter overflow"};

    raid.defense_score=defense_score;raid.attacker_losses=attacker_losses;
    raid.defender_casualties=defender_casualties;raid.salt_looted=looted;
    raid.defenders=defenders;raid.resolved_day=day_;
    raid.status=command.retreat?MilitaryRaidStatus::Withdrawn:
        (repelled?MilitaryRaidStatus::Repelled:MilitaryRaidStatus::Successful);
    for(int i=0;i<defender_casualties;++i)
        mutable_person(defenders[size_t(defenders.size()-1-size_t(i))]).health=0;
    if(looted>0){
        port.stock[size_t(GoodId::Salt)]-=looted;
        partner.goods[size_t(GoodId::Salt)]+=looted;
        partner.salt_capacity-=looted;
    }
    actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
    event(actor.id,raid.target_region,repelled?249:250,command.retreat?0:defense_score,raid.id);
    if(defender_casualties>0)event(actor.id,raid.target_region,251,defender_casualties,raid.id);
    if(looted>0)event(actor.id,raid.target_region,252,looted,raid.id);
    return {true,repelled?"Nam Cang defenders repelled the raid":
        (command.retreat?"The Sovereign ordered a withdrawal; the raiders looted the port":"The raiders breached the port defense"),
        next_fact_-1,raid.id};
}

} // namespace polyec
