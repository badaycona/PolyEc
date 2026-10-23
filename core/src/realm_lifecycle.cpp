#include "polyec/realm.hpp"

#include "polyec/managers/employment_manager.hpp"
#include "polyec/managers/estate_manager.hpp"

#include <algorithm>
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
#include <chrono>
#endif
#include <limits>
#include <unordered_set>

namespace polyec {

void Realm::record_unmet_essential_need(Person& person){
    const int before=person.nutrition;
    person.nutrition=std::max(0,person.nutrition-20);
    person.health=std::max(0,person.health-2);
    if(before>60&&person.nutrition<=60)
        event(person.id,person.region,113,person.nutrition);
}

void Realm::day_people(){
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    using ProfileClock=std::chrono::steady_clock;
    auto profileStarted=ProfileClock::now();
    auto finishProfile=[this,&profileStarted](size_t phase){
        const auto elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(
            ProfileClock::now()-profileStarted).count();
        if(elapsed>0)people_phase_profile_ns_[phase]+=static_cast<uint64_t>(elapsed);
        profileStarted=ProfileClock::now();
    };
#endif
    // A muster obligation is accepted by the recipient and carries the exact
    // roster. Index its assignments once; location must be checked later in
    // the person loop because a migration can finish earlier that same day.
    struct AcceptedMusterAssignment { int source_region; int destination_region; uint64_t commander; };
    std::unordered_multimap<uint64_t,AcceptedMusterAssignment> accepted_muster_assignments;
    for(const auto& obligation:military_obligations_){
        for(const uint64_t member_id:obligation.member_ids)
            accepted_muster_assignments.emplace(member_id,AcceptedMusterAssignment{
                obligation.source_region,obligation.destination_region,obligation.obligor});
    }
    struct DeathToday { uint64_t actor; int event_code; };
    std::vector<DeathToday> deceased_today;
    deceased_today.reserve(people_.size());
    for(auto&p:people_)if(p.alive){++p.age_days;
        if(p.military_service.active){
            auto& service=p.military_service;
            if(service.mobilized_until_day>0&&day_>=service.mobilized_until_day){
                const auto commander=service.commander;
                const auto source_region=service.source_region;
                service.mobilized_until_day=0;
                event(p.id,source_region,228,0,commander);
            }
            if(day_>=service.start_day&&day_<service.end_day){
                if(service.escrow<service.wage_per_day||service.paid_total>std::numeric_limits<int64_t>::max()-service.wage_per_day)
                    throw std::logic_error("military service payroll escrow is inconsistent");
                transfer(service.escrow,p.cash,service.wage_per_day);
                service.paid_total+=service.wage_per_day;
            }
            if(day_+1>=service.end_day)end_military_service(p,115);
        }
        if(p.office_appointment.hired_by&&p.office_appointment.salary_due<=day_){auto&employer=person(p.office_appointment.hired_by);auto&source=provinces_[employer.region].treasury;if(source>=EmploymentManager::kOfficeSalary){transfer(source,p.cash,EmploymentManager::kOfficeSalary);p.office_appointment.salary_due=day_+EmploymentManager::kOfficeSalaryPeriod;}else {EmploymentManager::end_office(p);event(p.id,p.region,110);}}
        if(p.destination>=0&&p.busy_until<=day_){p.region=p.destination;p.destination=-1;p.work_assignment.job=-1;event(p.id,p.region,int(Intent::Migrate));}
        auto&v=provinces_[p.region];
        if(EmploymentManager::account_day(p,v,day_)==0&&!schedule_book_.timeline_contract){int legacy=p.region%7;size_t g=size_t(canonical_from_legacy_good(legacy));int n=1+p.skill/40;v.stock[g]+=n;produced_[g]+=n;}
        if(p.school_days>0){++v.school_used;--p.school_days;if(p.school_days==0){p.skill=std::min(100,p.skill+5+capability_value(p.id,LegacyCapability::Strategy)/20+(v.teachers?v.teacher_skill_sum/std::max(1,v.teachers)/25:0));event(p.id,p.region,int(Intent::Study),p.skill);}}
        // Essentials are a biological process, not a strategic purchase selector.
        if(day_%7==int(p.id%7)){
            bool accepted_muster_destination=false;
            if(p.military_service.active&&p.military_service.end_day>day_&&
               p.military_service.mobilized_until_day>day_){
                const auto [first,last]=accepted_muster_assignments.equal_range(p.id);
                for(auto assignment=first;assignment!=last;++assignment){
                    const auto& accepted=assignment->second;
                    if(p.military_service.commander==accepted.commander&&
                       p.military_service.source_region==accepted.source_region&&
                       p.region==accepted.destination_region&&p.destination==-1){
                        accepted_muster_destination=true;break;
                    }
                }
            }
            const bool eligible_for_regional_ration=p.military_service.active&&
                (p.region==p.military_service.source_region||accepted_muster_destination);
            bool base_meal_consumed=false;
            if(eligible_for_regional_ration&&military_ration_depots_[size_t(p.region)]>0){
                --military_ration_depots_[size_t(p.region)];++consumed_[size_t(GoodId::DryFood)];p.nutrition=std::min(100,p.nutrition+20);p.health=std::min(100,p.health+1);base_meal_consumed=true;
            }else if(p.inventory[0]>0){
                --p.inventory[0];++consumed_[size_t(GoodId::DryFood)];p.nutrition=std::min(100,p.nutrition+20);p.health=std::min(100,p.health+1);base_meal_consumed=true;
            }else if(config_.market_enabled()){
                if(!place_essential_market_order(p,v)){
                    record_unmet_essential_need(p);
                }
            }else if(v.stock[0]>0&&available_funds(p.id)>=v.price[0]){
                const auto food_tax=MarketManager::assess_sales_tax(v.price[0],v.tax,v.sales_tax_remainder);
                if(food_tax&&v.tax_receipts<=std::numeric_limits<int64_t>::max()-food_tax->amount){
                    spend(p,v.firm,v.price[0]);sales_tax(v,v.price[0]);--v.stock[0];++consumed_[size_t(GoodId::DryFood)];p.nutrition=std::min(100,p.nutrition+20);p.health=std::min(100,p.health+1);base_meal_consumed=true;
                }else record_unmet_essential_need(p);
            }else if(v.stock[0]>0&&p.organization>0&&organization_funds_[p.organization-1]>=v.price[0]){
                const auto food_tax=MarketManager::assess_sales_tax(v.price[0],v.tax,v.sales_tax_remainder);
                if(food_tax&&v.tax_receipts<=std::numeric_limits<int64_t>::max()-food_tax->amount){
                    transfer(organization_funds_[p.organization-1],v.firm,v.price[0]);sales_tax(v,v.price[0]);--v.stock[0];++consumed_[size_t(GoodId::DryFood)];p.nutrition=std::min(100,p.nutrition+20);p.health=std::min(100,p.health+1);base_meal_consumed=true;
                }else record_unmet_essential_need(p);
            }else record_unmet_essential_need(p);
            if(eligible_for_regional_ration&&base_meal_consumed&&
               military_supplementary_ration_depots_[size_t(p.region)]>0){
                --military_supplementary_ration_depots_[size_t(p.region)];
                ++consumed_[size_t(GoodId::SupplementaryFood)];
                p.nutrition=std::min(100,p.nutrition+10);
            }
        }
        if(p.age_days==18*365)event(p.id,p.region,101);
        if(p.age_days==75*365)event(p.id,p.region,111,p.age_days);
    }
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    finishProfile(0);
#endif
    settle_market_day();
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    finishProfile(1);
#endif
    for(auto&p:people_)if(p.alive){
        if(p.health<=0){
            p.health=0;
            deceased_today.push_back({p.id,100});
        }else if(!reference_scenario_&&p.age_days>75*365&&rng_()%3650==0){
            deceased_today.push_back({p.id,112});
        }
    }
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    finishProfile(2);
#endif
    // Resolve everyone who died during this daily pass before choosing heirs.
    // Inheritance then uses the same end-of-phase survivor set and cannot fund
    // another resident's food purchase earlier or later in this loop.
    for(const auto death:deceased_today){
        auto& deceased=people_[static_cast<size_t>(death.actor-1)];
        if(deceased.military_service.active)end_military_service(deceased,115);
        cancel_market_orders(death.actor);
        auto plan=EstateManager::prepare_death(deceased,day_,estate_,legacy_estate_unassigned_,estate_cases_,
            establishments_,land_holdings_,land_use_rights_,loans_);
        EstateManager::apply_death(plan,deceased,estate_,estate_cases_);
    }
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    finishProfile(3);
#endif
    for(const auto death:deceased_today){
        const auto id=death.actor;
        auto& deceased=people_[static_cast<size_t>(id-1)];
        const bool has_transport_assets=std::any_of(transport_shipments_.begin(),transport_shipments_.end(),[id](const TransportShipment& shipment){
            return shipment.owner==id&&shipment.delivered+shipment.canceled<shipment.quantity;
        });
        if(has_transport_assets)continue;
        const auto claims=EstateManager::prepare_cash_claims(id,estate_,legacy_estate_unassigned_,
            credit_pool_,estate_cases_,loans_,provinces_);
        EstateManager::apply_cash_claims(claims,estate_,legacy_estate_unassigned_,credit_pool_,
            estate_cases_,loans_,provinces_);
        const auto distribution=EstateManager::prepare_distribution(id,estate_,
            legacy_estate_unassigned_,estate_cases_,people_,edges_,provinces_,loans_,
            establishments_,land_holdings_,land_use_rights_);
        EstateManager::apply_distribution(distribution,estate_,legacy_estate_unassigned_,
            estate_cases_,people_,provinces_,loans_,establishments_,land_holdings_,land_use_rights_);
        event(id,deceased.region,death.event_code,deceased.age_days);
    }
    if(!deceased_today.empty())rebuild_personal_establishment_index();
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    finishProfile(4);
#endif
}

} // namespace polyec
