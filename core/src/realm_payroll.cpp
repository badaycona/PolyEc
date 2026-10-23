#include "polyec/realm.hpp"
#include "polyec/managers/finance_manager.hpp"
#include "polyec/managers/employment_manager.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace polyec {
namespace {
int64_t sum_units(int64_t a,int64_t b){
    if(a<0||b<0||b>std::numeric_limits<int64_t>::max()-a)throw std::overflow_error("payroll units overflow");
    return a+b;
}
int64_t units_at(const std::map<uint64_t,int64_t>& values,uint64_t actor){
    const auto found=values.find(actor);return found==values.end()?0:found->second;
}
int64_t regional_at(const std::map<WorkRegionKey,int64_t>& values,const WorkRegionKey& key){
    const auto found=values.find(key);return found==values.end()?0:found->second;
}
struct SiteWorkSnapshot {std::size_t index;int64_t eligible;};
struct RegionWorkSnapshot {int64_t ticks=0,unassigned=0;std::vector<SiteWorkSnapshot> sites;};
using WorkSnapshots=std::map<WorkRegionKey,RegionWorkSnapshot>;
WorkSnapshots collect_work(const ScheduleBook& book,std::span<const Establishment> sites,
                           std::size_t people,int regions,int64_t denominator){
    WorkSnapshots result;
    for(const auto& [key,ticks]:book.work_present_ticks_by_region){
        if(key.first==0||key.first>people||key.second<0||key.second>regions||ticks<=0||ticks>book.cursor)
            throw std::logic_error("invalid regional payroll attendance");
        result.emplace(key,RegionWorkSnapshot{ticks,ticks,{}});
    }
    for(const auto& [key,ticks]:book.work_present_ticks_by_establishment){
        if(ticks<=0)throw std::logic_error("invalid site payroll attendance");
        const auto site=std::lower_bound(sites.begin(),sites.end(),key.second,
            [](const Establishment& value,uint64_t id){return value.id<id;});
        if(site==sites.end()||site->id!=key.second)throw std::logic_error("unknown payroll establishment");
        auto region=result.find({key.first,site->region});
        if(region==result.end()||ticks>region->second.unassigned)
            throw std::logic_error("site attendance exceeds regional attribution");
        region->second.unassigned-=ticks;
        region->second.sites.push_back({std::size_t(site-sites.begin()),ticks/denominator});
    }
    for(auto& [key,region]:result){(void)key;region.unassigned/=denominator;}
    return result;
}
template<class Accessor>
bool consistent_credits(const WorkSnapshots& snapshots,const WorkCreditRegions& credits,
                        const std::map<WorkRegionKey,int64_t>& paid,const std::map<WorkRegionKey,int64_t>& remainders,
                        const ScheduleBook& book,std::span<const Establishment> sites,std::size_t people,
                        int64_t denominator,int64_t wage,Accessor state){
    for(const auto& [key,credit]:credits)
        if(!snapshots.contains(key)||credit.legacy_units<=0)return false;
    for(std::size_t index=0;index<sites.size();++index){
        const auto& pay=state(index);
        for(const auto* ledger:{&pay.legacy_credit,&pay.paid_units,&pay.remainders}){
            if(ledger->size()>people)return false;
            for(const auto& [actor,value]:*ledger)
                if(actor==0||actor>people||value<=0||!book.work_present_ticks_by_establishment.contains({actor,sites[index].id}))return false;
        }
    }
    for(const auto& [key,context]:snapshots){
        const auto found=credits.find(key);const WorkRegionCredit credit=found==credits.end()?WorkRegionCredit{}:found->second;
        if(credit.legacy_units<0||credit.legacy_units>context.ticks/denominator||credit.unassigned_units<0||
           credit.unassigned_units>context.unassigned||credit.bridge_units<0||credit.bridge_units>int64_t(context.sites.size()))return false;
        int64_t credited=sum_units(credit.unassigned_units,credit.bridge_units);
        for(const auto& site:context.sites){
            const auto& pay=state(site.index);
            const int64_t old=units_at(pay.legacy_credit,key.first);
            if(old>site.eligible)return false;
            credited=sum_units(credited,old);
            const int64_t entitled=credit.bridge_units?0:site.eligible-old;
            const int64_t actual=units_at(pay.paid_units,key.first),remainder=units_at(pay.remainders,key.first);
            if(actual<0||actual>entitled||remainder<0||remainder>=wage||(remainder&&actual==entitled))return false;
        }
        if(credited!=credit.legacy_units)return false;
        const int64_t entitled=credit.bridge_units?credit.legacy_units:
            sum_units(credit.legacy_units,context.unassigned-credit.unassigned_units);
        const int64_t actual=regional_at(paid,key),remainder=regional_at(remainders,key);
        if(actual<0||actual>entitled||remainder<0||remainder>=wage||(remainder&&actual==entitled))return false;
    }
    return true;
}
int64_t money_due(int64_t eligible,int64_t paid,int64_t remainder,int64_t wage){
    if(eligible<0||paid<0||paid>eligible||remainder<0||remainder>=wage)
        throw std::logic_error("invalid payroll watermark");
    const int64_t units=eligible-paid;
    if(units>std::numeric_limits<int64_t>::max()/wage)throw std::overflow_error("work wage claim overflow");
    const int64_t gross=units*wage;
    if(remainder>gross)throw std::logic_error("pay remainder exceeds entitlement");
    return gross-remainder;
}
}

void Realm::migrate_work_credits(){
    const auto snapshots=collect_work(schedule_book_,establishments_,people_.size(),regions_,config_.ticks_per_unit());
    for(const auto& [key,context]:snapshots){
        std::vector<WorkCreditSource> sources;sources.reserve(context.sites.size());
        for(const auto& site:context.sites)sources.push_back({establishments_[site.index].id,site.eligible,0});
        const auto initial=initialize_work_credit(context.ticks/config_.ticks_per_unit(),context.unassigned,sources);
        if(initial.region.legacy_units)work_credits_.emplace(key,initial.region);
        for(std::size_t index=0;index<initial.sites.size();++index)
            if(initial.sites[index].credited_units)establishments_[context.sites[index].index].work_pay.legacy_credit.emplace(key.first,initial.sites[index].credited_units);
    }
}
bool Realm::work_credit_consistent()const{
    if(!schedule_book_.timeline_contract&&!work_credits_.empty())return false;
    try{
        const auto snapshots=collect_work(schedule_book_,establishments_,people_.size(),regions_,config_.ticks_per_unit());
        return consistent_credits(snapshots,work_credits_,schedule_book_.paid_work_units_by_region,work_pay_remainders_,
            schedule_book_,establishments_,people_.size(),config_.ticks_per_unit(),config_.wage_per_unit(),
            [&](std::size_t index)->const EstablishmentWorkPay&{return establishments_[index].work_pay;});
    }catch(const std::logic_error&){return false;}catch(const std::overflow_error&){return false;}
}

void Realm::day_payroll(){
    if(!schedule_book_.timeline_contract){
        // Frozen pilot behavior: same wages, role top-ups, sources and order.
        std::vector<std::vector<std::pair<uint64_t,int64_t>>> private_claims(size_t(regions_)+1),public_claims(size_t(regions_)+1);
        for(auto&p:people_)if(p.alive){
            if(EmploymentManager::has_active_work(p,day_)){if(p.work_assignment.job_kind==1)public_claims[p.region].push_back({p.id,2});else private_claims[p.region].push_back({p.id,1+p.skill/25});}
            if(!p.office_appointment.hired_by&&day_%30==0&&p.family!=Family::Civilian){auto&claims=p.family==Family::Governance?public_claims[p.region]:private_claims[p.region];claims.push_back({p.id,20});}
        }
        for(int r=0;r<=regions_;++r){payroll(r,false,private_claims[r]);payroll(r,true,public_claims[r]);}
        return;
    }
    const int64_t denominator=config_.ticks_per_unit(),wage=config_.wage_per_unit();
    const auto snapshots=collect_work(schedule_book_,establishments_,people_.size(),regions_,denominator);
    auto nextPaid=schedule_book_.paid_work_units_by_region;
    auto nextRemainders=work_pay_remainders_;
    auto nextCredits=work_credits_;
    // Only touched wage maps are copied. Stock and authorization stay in place.
    std::map<std::size_t,EstablishmentWorkPay> stagedSites;
    auto stageSite=[&](std::size_t index)->EstablishmentWorkPay&{
        return stagedSites.try_emplace(index,establishments_[index].work_pay).first->second;
    };
    struct PendingClaim {uint64_t actor;int priority;int64_t amount,entitlement;};
    std::vector<std::vector<PendingClaim>> privateClaims(size_t(regions_)+1),publicClaims(size_t(regions_)+1);
    std::map<uint64_t,std::vector<PendingClaim>> siteClaims;
    if(day_%30==0)for(const auto& person:people_)if(person.alive&&!person.office_appointment.hired_by&&person.family!=Family::Civilian){
        auto& claims=person.family==Family::Governance?publicClaims[person.region]:privateClaims[person.region];
        claims.push_back({person.id,1,20,0});
    }
    for(const auto& [key,context]:snapshots){
        const auto found=work_credits_.find(key);
        const WorkRegionCredit previous=found==work_credits_.end()?WorkRegionCredit{}:found->second;
        std::vector<WorkCreditSource> sources;sources.reserve(context.sites.size());
        for(const auto& site:context.sites){
            const auto& live=establishments_[site.index];
            sources.push_back({live.id,site.eligible,units_at(live.work_pay.legacy_credit,key.first)});
        }
        const auto credit=allocate_work_credit(previous,context.unassigned,sources,day_);
        if(credit.region.legacy_units)nextCredits[key]=credit.region;
        const auto& person=Realm::person(key.first);
        for(std::size_t slot=0;slot<context.sites.size();++slot){
            const auto& source=credit.sites[slot];const auto index=context.sites[slot].index;
            const auto& live=establishments_[index].work_pay;
            if(source.credited_units!=units_at(live.legacy_credit,key.first)){
                auto& staged=stageSite(index);
                if(source.credited_units)staged.legacy_credit[key.first]=source.credited_units;
                else staged.legacy_credit.erase(key.first);
            }
            if(!person.alive)continue; // historical pending wage freezes on death
            const int64_t entitled=source.eligible_units-source.credited_units;
            const int64_t due=money_due(entitled,units_at(live.paid_units,key.first),units_at(live.remainders,key.first),wage);
            if(due)siteClaims[source.establishment_id].push_back({key.first,0,due,entitled});
        }
        if(person.alive){
            const int64_t due=money_due(credit.regional_entitlement,regional_at(nextPaid,key),regional_at(nextRemainders,key),wage);
            if(due){
                auto& claims=person.work_assignment.job_kind==1?publicClaims[key.second]:privateClaims[key.second];
                claims.push_back({key.first,0,due,credit.regional_entitlement});
            }
        }
    }
    for(int region=0;region<=regions_;++region)for(auto* claims:{&privateClaims[region],&publicClaims[region]})
        std::sort(claims->begin(),claims->end(),[](const auto& a,const auto& b){return a.actor!=b.actor?a.actor<b.actor:a.priority<b.priority;});
    struct ClaimTag {uint64_t actor,site;int region;bool public_employer,wage;std::size_t site_index;int64_t entitlement;};
    std::vector<ClaimTag> tags;std::vector<PayrollBatchGroup> groups;
    for(int region=0;region<=regions_;++region){
        for(bool publicEmployer:{false,true}){
            const auto& claims=publicEmployer?publicClaims[region]:privateClaims[region];
            if(claims.empty())continue;
            PayrollBatchGroup group;group.region=region;group.public_employer=publicEmployer;
            for(const auto& claim:claims){
                group.claims.push_back({claim.actor,claim.amount});
                tags.push_back({claim.actor,0,region,publicEmployer,claim.priority==0,0,claim.entitlement});
            }
            groups.push_back(std::move(group));
        }
        for(const auto& [siteId,claims]:siteClaims){
            const auto site=std::lower_bound(establishments_.begin(),establishments_.end(),siteId,
                [](const Establishment& value,uint64_t id){return value.id<id;});
            if(site==establishments_.end()||site->id!=siteId)throw std::logic_error("unknown wage source");
            if(site->region!=region)continue;
            PayrollBatchGroup group;group.region=region;group.establishment_id=siteId;
            const auto index=std::size_t(site-establishments_.begin());
            for(const auto& claim:claims){
                group.claims.push_back({claim.actor,claim.amount});
                tags.push_back({claim.actor,siteId,region,false,true,index,claim.entitlement});
            }
            groups.push_back(std::move(group));
        }
    }
    const auto plan=FinanceManager::prepare_batch(day_,groups,people_,provinces_,establishments_);
    if(plan.claim_count()!=tags.size())throw std::logic_error("payroll batch allocation mismatch");
    for(std::size_t row=0;row<tags.size();++row){
        const auto& tag=tags[row];
        if(plan.claim_id(row)!=tag.actor||plan.claim_region(row)!=tag.region||plan.claim_public(row)!=tag.public_employer||
           plan.claim_establishment(row)!=tag.site)throw std::logic_error("payroll batch source mismatch");
        if(!tag.wage)continue;
        const int64_t gross=plan.paid(row);
        if(gross<0)throw std::logic_error("negative wage allocation");
        if(gross==0)continue;
        const WorkRegionKey key{tag.actor,tag.region};
        int64_t previous=0,paid=0;
        if(tag.site){
            const auto& state=stageSite(tag.site_index);previous=units_at(state.remainders,tag.actor);paid=units_at(state.paid_units,tag.actor);
        }else{previous=regional_at(nextRemainders,key);paid=regional_at(nextPaid,key);}
        const int64_t accumulated=sum_units(previous,gross);
        const int64_t units=sum_units(paid,accumulated/wage),remainder=accumulated%wage;
        if(units>tag.entitlement||(remainder&&units==tag.entitlement))throw std::logic_error("paid wage exceeds entitlement");
        if(tag.site){
            auto& state=stageSite(tag.site_index);
            if(units)state.paid_units[tag.actor]=units;
            if(remainder)state.remainders[tag.actor]=remainder;else state.remainders.erase(tag.actor);
        }else{
            if(units)nextPaid[key]=units;
            if(remainder)nextRemainders[key]=remainder;else nextRemainders.erase(key);
        }
    }
    if(!consistent_credits(snapshots,nextCredits,nextPaid,nextRemainders,schedule_book_,establishments_,people_.size(),denominator,wage,
        [&](std::size_t index)->const EstablishmentWorkPay&{
            const auto found=stagedSites.find(index);return found==stagedSites.end()?establishments_[index].work_pay:found->second;
        }))throw std::logic_error("work credit payroll invariant");
    FinanceManager::apply_batch(plan,day_,people_,provinces_,establishments_);
    // Every allocation and validation is complete. Commit scalar finance first,
    // then noexcept swaps of the prepared bookkeeping; no full Person copy.
    schedule_book_.paid_work_units_by_region.swap(nextPaid);
    work_pay_remainders_.swap(nextRemainders);work_credits_.swap(nextCredits);
    for(auto& [index,state]:stagedSites){
        auto& live=establishments_[index].work_pay;
        live.legacy_credit.swap(state.legacy_credit);live.paid_units.swap(state.paid_units);live.remainders.swap(state.remainders);
    }
}
}
