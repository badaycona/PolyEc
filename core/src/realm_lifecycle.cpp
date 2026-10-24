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
        if(p.destination>=0&&p.busy_until<=day_){p.region=p.destination;if(p.residence_kind==ResidenceKind::Urban)p.urban_site_id=static_cast<uint8_t>(p.region);p.destination=-1;p.work_assignment.job=-1;event(p.id,p.region,int(Intent::Migrate));}
        const size_t medicine=size_t(GoodId::Medicine);
        if(p.health>0&&p.health<=60&&p.inventory[medicine]>0&&consumed_[medicine]<std::numeric_limits<int64_t>::max()&&next_fact_<UINT64_MAX){
            --p.inventory[medicine];++consumed_[medicine];p.health=std::min(70,p.health+10);
            event(p.id,p.region,237,1,int(GoodId::Medicine));
        }
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
            }else if(p.family==Family::Civilian&&!p.military_service.active&&base_meal_consumed&&
                     p.inventory[size_t(GoodId::Pottery)]>0&&p.nutrition<100){
                p.nutrition=std::min(100,p.nutrition+5);
            }
            if(p.family==Family::Civilian&&!p.military_service.active&&base_meal_consumed&&
                     p.nutrition<100&&
                     p.inventory[size_t(GoodId::SupplementaryFood)]>0&&
                     consumed_[size_t(GoodId::SupplementaryFood)]<std::numeric_limits<int64_t>::max()){
                --p.inventory[size_t(GoodId::SupplementaryFood)];
                ++consumed_[size_t(GoodId::SupplementaryFood)];
                p.nutrition=std::min(100,p.nutrition+10);
                event(p.id,p.region,238,1,int(GoodId::SupplementaryFood));
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
        for(auto& council:municipal_councils_){
            if(council.representative==death.actor){council.representative=0;council.term_end_day=-1;}
            if(council.nominee==death.actor){council.nominee=0;council.nomination_day=-1;}
        }
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
    process_private_housing();
    if(!deceased_today.empty())rebuild_personal_establishment_index();
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    finishProfile(4);
#endif
}

void Realm::process_private_housing(){
    for(auto& home:private_residences_){
        if(home.offered_household&&home.offer_expires_day<=day_){
            home.offered_household=0;home.offered_rent=0;home.offer_expires_day=-1;
        }
        if(!home.completed&&home.construction_due_day<=day_){
            home.completed=true;
            const auto title=std::lower_bound(land_holdings_.begin(),land_holdings_.end(),home.holding_id,
                [](const LandHolding& value,uint64_t id){return value.id<id;});
            const uint64_t owner=title!=land_holdings_.end()&&title->id==home.holding_id?title->title_holder:0;
            event(owner,home.region,255,kPrivateResidenceBeds,home.holding_id);
        }
        if(!home.tenant_household)continue;
        const auto holding=std::lower_bound(land_holdings_.begin(),land_holdings_.end(),home.holding_id,
            [](const LandHolding& value,uint64_t id){return value.id<id;});
        if(holding==land_holdings_.end()||holding->id!=home.holding_id)throw std::logic_error("private home lost its land title");
        const auto landlord=holding->title_holder;
        const auto tenant_authority=household_authority(home.tenant_household);
        const auto clearLease=[&](int64_t reason){
            home.tenant_household=0;home.rent_per_period=0;home.next_rent_day=-1;home.lease_end_day=-1;
            event(landlord,home.region,254,reason,home.id);
        };
        if(day_>=home.lease_end_day){clearLease(0);continue;}
        if(!tenant_authority||tenant_authority>people_.size()||!person(tenant_authority).alive||
           landlord==0||landlord>people_.size()||!person(landlord).alive){clearLease(-1);continue;}
        if(day_<home.next_rent_day)continue;
        auto& tenant=mutable_person(tenant_authority);
        auto& owner=mutable_person(landlord);
        if(available_funds(tenant_authority)<home.rent_per_period||
           owner.cash>std::numeric_limits<int64_t>::max()-home.rent_per_period){clearLease(-1);continue;}
        facts_.reserve(facts_.size()+1);
        spend(tenant,owner.cash,home.rent_per_period);
        if(home.next_rent_day>365000-kPrivateResidenceRentPeriodDays)throw std::logic_error("private rent schedule overflow");
        home.next_rent_day+=kPrivateResidenceRentPeriodDays;
        event(tenant_authority,home.region,256,home.rent_per_period,home.id);
    }
}

} // namespace polyec
