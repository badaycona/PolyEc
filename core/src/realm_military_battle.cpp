#include "polyec/realm.hpp"

#include <algorithm>
#include <limits>
#include <set>

namespace polyec {

std::vector<uint64_t> Realm::military_battle_candidates(int region)const{
    std::vector<uint64_t> result;
    if(region<0||region>=int(kAtlasTerritoryCount)||regions_!=int(kAtlasTerritoryCount)-1)return result;
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
    if(command.target_region<0||command.target_region>=int(kAtlasTerritoryCount))
        return {false,"Raid target must be one of the 27 Atlas regions"};
    if(region_military_occupied(command.target_region))
        return {false,"The target is already held by an external garrison; the Sovereign must counterattack to recover it"};
    auto& partner=foreign_trade_partners_[size_t(command.partner_id)];
    if(partner.decision_sequence==std::numeric_limits<uint64_t>::max()||next_id_==std::numeric_limits<uint64_t>::max()||
       next_fact_==std::numeric_limits<uint64_t>::max())return {false,"Raid sequence or record ID overflow"};
    if(command.sequence!=partner.decision_sequence+1)
        return {false,"Stale or duplicate external-power decision sequence"};
    if(command.force_units<1||command.force_units>kMilitaryRaidUnitMaximum)
        return {false,"Raid force must contain 1–8 units"};
    int pending=0;
    for(const auto& raid:military_raids_)if(raid.status==MilitaryRaidStatus::Pending){
        ++pending;
        if(raid.partner_id==command.partner_id)return {false,"This external power already has a raid awaiting a response"};
        if(raid.target_region==command.target_region)return {false,"Another raid already targets this region"};
    }
    if(pending>=kMilitaryRaidPendingMaximum)return {false,"All three external powers already have pending raids"};
    const auto sea_route=foreign_trade_route_plan(ForeignTradeOperation::Import,1);
    if(!sea_route.reachable)return {false,"Coastal corridor to Nam Cang is closed or unavailable"};
    std::array<int64_t,kAtlasLinkCount> waiting{};
    if(!transport_waiting_units_by_link(waiting))return {false,"Invalid or overflowing shared route queue"};
    const auto inland_route=NationalTransportManager::plan(day_,uint8_t(kMilitaryRaidTargetRegion),
        uint8_t(command.target_region),command.force_units,TransportServiceGoal::Fastest,
        national_transport_routes_,waiting);
    if(!inland_route.reachable)return {false,"No open, available transport route reaches the target from Nam Cang"};
    const int64_t force_cost=int64_t(command.force_units)*kMilitaryRaidCashPerUnit;
    const int64_t route_cost=inland_route.total_fee;
    if(route_cost>std::numeric_limits<int64_t>::max()-force_cost)return {false,"Raid funding total overflow"};
    const int64_t total_cost=force_cost+route_cost;
    if(partner.money<total_cost||partner.goods[size_t(GoodId::DryFood)]<command.force_units)
        return {false,"External power lacks the cash or DryFood to fund the raid"};
    if(carrier_>std::numeric_limits<int64_t>::max()-force_cost||
       consumed_[size_t(GoodId::DryFood)]>std::numeric_limits<int64_t>::max()-command.force_units)
        return {false,"Raid supply ledger overflow"};
    std::array<int64_t,kAtlasTerritoryCount> route_fees_by_region{};
    for(const size_t route_index:inland_route.route_indices){
        const auto& route=national_transport_routes_[route_index];
        const int64_t fee=route.fee_per_unit*command.force_units;
        auto& region_fee=route_fees_by_region[size_t(route.fee_recipient_region)];
        if(fee>std::numeric_limits<int64_t>::max()-region_fee)
            return {false,"Raid route fee total overflow"};
        region_fee+=fee;
    }
    for(size_t region=0;region<route_fees_by_region.size();++region)
        if(route_fees_by_region[region]>std::numeric_limits<int64_t>::max()-provinces_[region].treasury)
            return {false,"Raid route fee recipient balance overflow"};

    military_raids_.reserve(military_raids_.size()+1);
    MilitaryRaid raid;raid.id=next_id_;raid.partner_id=command.partner_id;
    raid.target_region=command.target_region;raid.created_day=day_;raid.force_units=command.force_units;
    transfer(partner.money,carrier_,force_cost);
    for(size_t region=0;region<route_fees_by_region.size();++region)
        if(route_fees_by_region[region]>0)
            transfer(partner.money,provinces_[region].treasury,route_fees_by_region[region]);
    partner.goods[size_t(GoodId::DryFood)]-=command.force_units;
    consumed_[size_t(GoodId::DryFood)]+=command.force_units;
    ++partner.decision_sequence;++next_id_;
    military_raids_.push_back(raid);
    event(1,command.target_region,248,command.force_units,command.partner_id+1);
    return {true,command.target_region==kMilitaryRaidTargetRegion?
        "External power funded a coastal raid against Nam Cang":
        "External power funded a raid through Nam Cang against the selected region",next_fact_-1,raid.id};
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
    std::vector<uint64_t> supplied_defenders;
    const auto battleReady=military_battle_candidates(raid_it->target_region);
    supplied_defenders.reserve(defenders.size());
    for(const uint64_t id:defenders){
        if(id==0||id>people_.size())return {false,"Unknown defending volunteer"};
        const auto& volunteer=person(id);
        if(!std::binary_search(battleReady.begin(),battleReady.end(),id))
            return {false,"Every defender must be alive, healthy, consenting, mobilized and present at Nam Cang"};
        const int military=capability_value(id,LegacyCapability::Military);
        const int weapon=volunteer.inventory[size_t(GoodId::Weapon)]>0?1:0;
        const int leather_armor=volunteer.inventory[size_t(GoodId::Leather)]>0?1:0;
        const bool supplied=volunteer.inventory[size_t(GoodId::MilitarySupply)]>0;
        defense_score+=1+military/50+weapon+leather_armor+int(supplied);
        if(supplied)supplied_defenders.push_back(id);
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
    if(consumed_[size_t(GoodId::MilitarySupply)]>
       std::numeric_limits<int64_t>::max()-int64_t(supplied_defenders.size()))
        return {false,"MilitarySupply consumption ledger overflow"};
    if(!repelled&&surviving_attackers>0){
        looted=std::min<int64_t>({surviving_attackers,port.stock[size_t(GoodId::Salt)],partner.salt_capacity});
        if(looted>std::numeric_limits<int64_t>::max()-partner.goods[size_t(GoodId::Salt)])
            return {false,"Partner Salt ledger overflow"};
    }
    const bool will_occupy=!command.retreat&&!repelled&&surviving_attackers>0;
    const uint64_t required_facts=1+uint64_t(defender_casualties>0)+uint64_t(looted>0)+uint64_t(will_occupy);
    if(next_fact_>std::numeric_limits<uint64_t>::max()-required_facts)
        return {false,"Battle fact counter overflow"};

    raid.defense_score=defense_score;raid.attacker_losses=attacker_losses;
    raid.defender_casualties=defender_casualties;raid.salt_looted=looted;
    raid.defenders=defenders;raid.supplied_defenders=supplied_defenders;raid.resolved_day=day_;
    raid.status=command.retreat?MilitaryRaidStatus::Withdrawn:
        (repelled?MilitaryRaidStatus::Repelled:MilitaryRaidStatus::Successful);
    for(int i=0;i<defender_casualties;++i)
        mutable_person(defenders[size_t(defenders.size()-1-size_t(i))]).health=0;
    for(const uint64_t id:supplied_defenders)
        --mutable_person(id).inventory[size_t(GoodId::MilitarySupply)];
    consumed_[size_t(GoodId::MilitarySupply)]+=int64_t(supplied_defenders.size());
    if(looted>0){
        port.stock[size_t(GoodId::Salt)]-=looted;
        partner.goods[size_t(GoodId::Salt)]+=looted;
        partner.salt_capacity-=looted;
    }
    if(raid.status==MilitaryRaidStatus::Successful&&surviving_attackers>0){
        auto& occupation=military_occupations_[size_t(raid.target_region)];
        occupation.partner_id=raid.partner_id;
        occupation.garrison_units=surviving_attackers;
        event(actor.id,raid.target_region,253,surviving_attackers,raid.partner_id+1);
    }
    actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
    event(actor.id,raid.target_region,repelled?249:250,command.retreat?0:defense_score,raid.id);
    if(defender_casualties>0)event(actor.id,raid.target_region,251,defender_casualties,raid.id);
    if(looted>0)event(actor.id,raid.target_region,252,looted,raid.id);
    return {true,repelled?"The region's defenders repelled the raid":
        (command.retreat?"The Sovereign ordered a withdrawal; the raiders looted the target region":
                         (will_occupy?"The raiders breached the region's defense and occupied it":
                                      "The raiders breached the region's defense")),
        next_fact_-1,raid.id};
}

RealmResult Realm::execute(const MilitaryCounterattackCommand& command){
    if(command.actor==0||command.actor>people_.size())return {false,"Unknown actor"};
    auto& actor=mutable_person(command.actor);
    if(!actor.alive)return {false,"Actor deceased"};
    if(actor.decisions==std::numeric_limits<uint64_t>::max()||actor.sampling==std::numeric_limits<uint64_t>::max()||
       next_fact_==std::numeric_limits<uint64_t>::max())return {false,"Decision or fact counter overflow"};
    if(command.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!actor.sovereign||actor.family!=Family::Governance)return {false,"Only the Sovereign may order a counterattack"};
    if(command.region<0||command.region>=int(kAtlasTerritoryCount)||regions_!=int(kAtlasTerritoryCount)-1)
        return {false,"Counterattack requires an Atlas region"};
    auto& occupation=military_occupations_[size_t(command.region)];
    if(!occupation.occupied())return {false,"The selected region has no external garrison"};
    if(command.retreat&&!command.defenders.empty())return {false,"Retreat cannot also commit defenders"};
    if(!command.retreat&&(command.defenders.empty()||command.defenders.size()>16))
        return {false,"Counterattack requires 1–16 named volunteers"};
    std::vector<uint64_t> defenders=command.defenders;
    std::sort(defenders.begin(),defenders.end());
    if(std::adjacent_find(defenders.begin(),defenders.end())!=defenders.end())
        return {false,"A volunteer may only be listed once"};
    int defense_score=0;
    std::vector<uint64_t> supplied_defenders;
    const auto battle_ready=military_battle_candidates(command.region);
    for(const uint64_t id:defenders){
        if(id==0||id>people_.size())return {false,"Unknown defending volunteer"};
        const auto& volunteer=person(id);
        if(!std::binary_search(battle_ready.begin(),battle_ready.end(),id))
            return {false,"Every defender must be alive, healthy, consenting, mobilized and present in the occupied region"};
        const int military=capability_value(id,LegacyCapability::Military);
        const int weapon=volunteer.inventory[size_t(GoodId::Weapon)]>0?1:0;
        const int leather=volunteer.inventory[size_t(GoodId::Leather)]>0?1:0;
        const bool supplied=volunteer.inventory[size_t(GoodId::MilitarySupply)]>0;
        defense_score+=1+military/50+weapon+leather+int(supplied);
        if(supplied)supplied_defenders.push_back(id);
    }
    if(consumed_[size_t(GoodId::MilitarySupply)]>
       std::numeric_limits<int64_t>::max()-int64_t(supplied_defenders.size()))
        return {false,"MilitarySupply consumption ledger overflow"};
    const bool recaptured=!command.retreat&&defense_score>occupation.garrison_units;
    const int attacker_losses=command.retreat?0:(recaptured?occupation.garrison_units:
        std::min(occupation.garrison_units,defense_score/2));
    const int defender_casualties=command.retreat||recaptured?0:
        std::min<int>(int(defenders.size()),std::max(1,(occupation.garrison_units+1)/2));
    const uint64_t required_facts=1+uint64_t(defender_casualties>0);
    if(next_fact_>std::numeric_limits<uint64_t>::max()-required_facts)
        return {false,"Counterattack fact counter overflow"};

    for(const uint64_t id:supplied_defenders)--mutable_person(id).inventory[size_t(GoodId::MilitarySupply)];
    consumed_[size_t(GoodId::MilitarySupply)]+=int64_t(supplied_defenders.size());
    for(int i=0;i<defender_casualties;++i)
        mutable_person(defenders[size_t(defenders.size()-1-size_t(i))]).health=0;
    if(recaptured)occupation=MilitaryOccupation{};
    else if(!command.retreat)occupation.garrison_units-=attacker_losses;
    actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
    event(actor.id,command.region,recaptured?255:(command.retreat?256:257),
        command.retreat?0:defense_score,occupation.garrison_units);
    if(defender_casualties>0)event(actor.id,command.region,258,defender_casualties,occupation.garrison_units);
    return {true,recaptured?"The region returned to the Realm's military control":
        (command.retreat?"The Sovereign withdrew and the garrison kept the region":
                         "The counterattack failed; surviving volunteers and garrison remain"),
        next_fact_-1,uint64_t(command.region)};
}

void Realm::process_military_occupations_day(){
    if(regions_!=int(kAtlasTerritoryCount)-1)return;
    std::array<int,kForeignTradePartnerCount> required{};
    for(const auto& occupation:military_occupations_)
        if(occupation.occupied())required[size_t(occupation.partner_id)]+=occupation.garrison_units;
    for(size_t partner_id=0;partner_id<required.size();++partner_id){
        const int total=required[partner_id];if(total==0)continue;
        auto& partner=foreign_trade_partners_[partner_id];
        const int available=int(std::min<int64_t>(partner.goods[size_t(GoodId::DryFood)],total));
        const int rationed_total=std::min(total,available);
        std::array<int,kAtlasTerritoryCount> rationed_by_region{};
        int assigned=0;
        for(size_t region=0;region<kAtlasTerritoryCount;++region){
            const auto& occupation=military_occupations_[region];
            if(!occupation.occupied()||size_t(occupation.partner_id)!=partner_id)continue;
            const int rationed=int((int64_t(rationed_total)*occupation.garrison_units)/total);
            rationed_by_region[region]=rationed;assigned+=rationed;
        }
        for(size_t region=0;region<kAtlasTerritoryCount&&assigned<rationed_total;++region){
            const auto& occupation=military_occupations_[region];
            if(occupation.occupied()&&size_t(occupation.partner_id)==partner_id&&
               rationed_by_region[region]<occupation.garrison_units){
                ++rationed_by_region[region];++assigned;
            }
        }
        if(consumed_[size_t(GoodId::DryFood)]>std::numeric_limits<int64_t>::max()-rationed_total)
            throw std::overflow_error("Occupation DryFood consumption ledger overflow");
        partner.goods[size_t(GoodId::DryFood)]-=rationed_total;
        consumed_[size_t(GoodId::DryFood)]+=rationed_total;
        for(size_t region=0;region<kAtlasTerritoryCount;++region){
            auto& occupation=military_occupations_[region];
            if(!occupation.occupied()||size_t(occupation.partner_id)!=partner_id)continue;
            const int lost=occupation.garrison_units-rationed_by_region[region];
            if(lost==0)continue;
            occupation.garrison_units=rationed_by_region[region];
            if(occupation.garrison_units==0)occupation=MilitaryOccupation{};
            event(1,int(region),259,lost,occupation.garrison_units);
        }
    }
}

} // namespace polyec
