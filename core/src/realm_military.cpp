#include "polyec/realm.hpp"
#include "polyec/market_manager.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <utility>

namespace polyec {

RealmResult Realm::execute(const MilitaryCombatConsentCommand& command){
    if(command.actor==0||command.actor>people_.size())return {false,"Unknown actor"};
    auto& actor=mutable_person(command.actor);
    if(!actor.alive)return {false,"Actor deceased"};
    if(actor.decisions==UINT64_MAX||actor.sampling==UINT64_MAX||next_fact_==UINT64_MAX)
        return {false,"Decision or fact counter overflow"};
    if(command.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    auto& service=actor.military_service;
    if(actor.family!=Family::Civilian||!service.active||day_<service.start_day||day_>=service.end_day)
        return {false,"Combat consent requires the volunteer's active service"};
    if(service.combat_consent==command.consent)return {false,"Combat consent is already in the requested state"};
    service.combat_consent=command.consent;
    actor.last_decision_day=day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id,actor.region,command.consent?240:241,command.consent?1:0,service.commander);
    return {true,command.consent?"Volunteer consented to combat during this service term":"Volunteer withdrew combat consent",next_fact_-1,service.commander};
}

std::vector<MilitaryUnitRoster> Realm::military_unit_rosters() const {
    std::map<std::pair<int,uint64_t>,MilitaryUnitRoster> grouped;
    for(const auto& person:people_){
        const auto& service=person.military_service;
        if(!service.active)continue;
        if(service.commander==0||service.commander>UINT32_MAX||service.source_region<0||service.source_region>regions_)
            throw std::logic_error("military roster source record is invalid");
        auto& roster=grouped[{service.source_region,service.commander}];
        roster.source_region=service.source_region;
        roster.commander=service.commander;
        roster.id=(uint64_t(service.source_region+1)<<32)|service.commander;
        if(service.escrow>std::numeric_limits<int64_t>::max()-roster.payroll_escrow||
           service.paid_total>std::numeric_limits<int64_t>::max()-roster.paid_total)
            throw std::overflow_error("military roster payroll total overflow");
        roster.payroll_escrow+=service.escrow;
        roster.paid_total+=service.paid_total;
        roster.members.push_back(person.id);
        if(service.mobilized_until_day>day_){
            roster.mobilized_members.push_back(person.id);
            if(roster.mobilized_until_day==0||service.mobilized_until_day<roster.mobilized_until_day)
                roster.mobilized_until_day=service.mobilized_until_day;
        }
        if(service.combat_consent)roster.combat_consented_members.push_back(person.id);
        if(service.combat_consent&&day_>=service.start_day&&day_<service.end_day&&
           service.mobilized_until_day>day_)
            roster.battle_ready_members.push_back(person.id);
    }
    std::vector<MilitaryUnitRoster> result;
    result.reserve(grouped.size());
    for(auto&[key,roster]:grouped){
        (void)key;
        std::sort(roster.members.begin(),roster.members.end());
        result.push_back(std::move(roster));
    }
    return result;
}

RealmResult Realm::execute(const MilitaryRationPurchaseCommand& command){
    if(command.actor==0||command.actor>people_.size())return {false,"Unknown actor"};
    auto& actor=mutable_person(command.actor);
    if(!actor.alive)return {false,"Actor deceased"};
    if(actor.decisions==UINT64_MAX||actor.sampling==UINT64_MAX||next_fact_==UINT64_MAX)
        return {false,"Decision or fact counter overflow"};
    if(command.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(actor.family!=Family::Governance||!has_local_government_authority(actor)||
       command.region!=actor.region||command.region<0||command.region>regions_||
       !provinces_[size_t(command.region)].charter)
        return {false,"Military ration purchase requires local public authority"};
    if(command.quantity<=0||command.quantity>kMilitaryRationPurchaseMaximum)
        return {false,"Invalid military ration quantity"};

    auto& province=provinces_[size_t(command.region)];
    if(command.good!=int(GoodId::DryFood)&&command.good!=int(GoodId::SupplementaryFood))
        return {false,"Invalid military ration good"};
    const size_t good=size_t(command.good);
    auto& depot=command.good==int(GoodId::DryFood)?military_ration_depots_[size_t(command.region)]:
        military_supplementary_ration_depots_[size_t(command.region)];
    if(province.stock[good]<command.quantity)return {false,"Insufficient local ration stock"};
    if(depot>
       std::numeric_limits<int64_t>::max()-command.quantity)
        return {false,"Military ration depot overflow"};
    const int64_t price=province.price[good];
    if(price<config_.market_price_floor()||price>config_.market_price_ceiling())
        return {false,"Invalid local ration-good price"};
    const auto gross=MarketManager::purchase_total(price,command.quantity);
    if(!gross||province.treasury<*gross)return {false,"Insufficient local treasury"};
    if(*gross>std::numeric_limits<int64_t>::max()-province.firm)
        return {false,"Local firm balance overflow"};
    const auto tax=MarketManager::assess_sales_tax(*gross,province.tax,province.sales_tax_remainder);
    if(!tax||tax->amount>std::numeric_limits<int64_t>::max()-province.tax_receipts)
        return {false,"Sales tax ledger overflow"};

    transfer(province.treasury,province.firm,*gross);
    sales_tax(province,*gross);
    province.stock[good]-=command.quantity;
    depot+=command.quantity;
    actor.last_decision_day=day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id,command.region,226,command.quantity,command.good);
    return {true,"Military ration stock purchased",next_fact_-1,uint64_t(command.region)};
}

} // namespace polyec
