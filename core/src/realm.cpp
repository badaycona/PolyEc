#include "polyec/realm.hpp"
#include "polyec/world_atlas.hpp"
#include "polyec/production_manager.hpp"
#include "polyec/managers/finance_manager.hpp"
#include "polyec/managers/employment_manager.hpp"
#include "polyec/managers/household_index.hpp"
#include "polyec/managers/project_manager.hpp"
#include "polyec/jobs/worker_job.hpp"
#include "polyec/jobs/teacher_job.hpp"
#include "polyec/jobs/office_role_job.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <set>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace polyec {
namespace {
bool same(const Candidate&a,const Candidate&b){return a.kind==b.kind&&a.target==b.target&&a.good==b.good&&a.amount==b.amount&&a.term==b.term;}
uint64_t process_noise(uint64_t x){x+=0x9e3779b97f4a7c15ull;x=(x^(x>>30))*0xbf58476d1ce4e5b9ull;x=(x^(x>>27))*0x94d049bb133111ebull;return x^(x>>31);}
Personality seeded_personality(uint64_t seed,uint64_t person_id){
    Personality result;
    for(size_t axis=0;axis<result.values.size();++axis){
        const uint64_t key=seed^process_noise(person_id+0x632be59bd9b4e019ull)^process_noise(uint64_t(axis)+0x8cb92baa3f3d8dd7ull);
        result.values[axis]=25+int(process_noise(key)%51);
    }
    return result;
}
constexpr int kExtraGoodCount=kGoodIdCount-kLegacyGoodCount;
constexpr int kExtraBuyBegin=32;
constexpr int kExtraSellBegin=kExtraBuyBegin+kExtraGoodCount;
constexpr int canonical_good_for_extra_slot(int slot) noexcept {
    const int ordinal=slot<kExtraSellBegin?slot-kExtraBuyBegin:slot-kExtraSellBegin;
    int seen=0;
    for(int good=0;good<kGoodIdCount;++good){
        bool legacy=false;
        for(int old=0;old<kLegacyGoodCount;++old)
            legacy|=canonical_from_legacy_good(old)==good;
        if(!legacy&&seen++==ordinal)return good;
    }
    return -1;
}
// Physical Work attendance is the length of the union of clipped half-open
// intervals, so overlapping sessions cannot be counted twice.
int64_t union_ticks(std::vector<std::pair<int64_t,int64_t>>& intervals){
    if(intervals.empty())return 0;
    std::sort(intervals.begin(),intervals.end());
    int64_t total=0,begin=intervals[0].first,end=intervals[0].second;
    for(size_t i=1;i<intervals.size();++i){
        if(intervals[i].first<=end)end=std::max(end,intervals[i].second);
        else{total+=end-begin;begin=intervals[i].first;end=intervals[i].second;}
    }
    return total+(end-begin);
}
// Unique (actor, region) keys are bounded by people * (regions + 1), the
// inclusive province-ID range 0..regions. Checked so a corrupt count cannot
// overflow before allocation.
bool checked_region_key_bound(size_t people,int regions,size_t&bound){
    bound=0;
    if(regions<0)return false;
    const size_t slots=size_t(regions)+1;
    if(people>std::numeric_limits<size_t>::max()/slots)return false;
    bound=people*slots;
    return true;
}
bool region_maps_consistent(const std::map<WorkRegionKey,int64_t>&regional_ticks,
                            const std::map<WorkRegionKey,int64_t>&regional_paid,
                            const std::unordered_map<uint64_t,int64_t>&global_ticks,
                            bool timeline,size_t people,int regions,int64_t cursor,size_t bound,int64_t ticks_per_unit){
    if(regional_ticks.size()>bound)return false;
    if(regional_paid.size()>bound)return false;
    if(!timeline&&(!regional_ticks.empty()||!regional_paid.empty()))return false;
    std::unordered_map<uint64_t,int64_t> sums;
    for(auto const&[key,ticks]:regional_ticks){
        const uint64_t actor=key.first;
        const int region=key.second;
        if(actor==0||actor>people)return false;
        if(region<0||region>regions)return false;
        if(ticks<=0||ticks>cursor)return false;
        int64_t&sum=sums[actor];
        if(ticks>std::numeric_limits<int64_t>::max()-sum)return false;
        sum+=ticks;
    }
    for(auto const&[key,units]:regional_paid){
        const uint64_t actor=key.first;
        const int region=key.second;
        if(actor==0||actor>people)return false;
        if(region<0||region>regions)return false;
        if(units<=0)return false;
        auto ticks=regional_ticks.find(key);
        if(ticks==regional_ticks.end())return false;
        if(units>ticks->second/ticks_per_unit)return false;
    }
    for(auto const&[actor,sum]:sums){
        auto global=global_ticks.find(actor);
        const int64_t total=global==global_ticks.end()?0:global->second;
        if(sum>total)return false;
    }
    return true;
}
int64_t loan_interest(int64_t principal,int rate){
    if(principal<0||rate<0)throw std::logic_error("invalid loan interest input");
    // Preserve the existing ceiling-to-whole-currency rule without multiplying
    // the full principal by the rate, which can overflow for large loans.
    const int64_t hundreds=principal/100;
    const int64_t remainder=principal%100;
    return hundreds*rate+(remainder*rate+99)/100;
}
int64_t outstanding(const Loan&loan){
    if(loan.principal<0||loan.interest<0||loan.paid<0||
       loan.principal>std::numeric_limits<int64_t>::max()-loan.interest)
        return std::numeric_limits<int64_t>::max();
    const int64_t due=loan.principal+loan.interest;
    return loan.paid>=due?0:due-loan.paid;
}
bool within_loan_limit(int64_t current,int64_t amount,int64_t limit){
    return current>=0&&amount>0&&limit>=current&&amount<=limit-current;
}
}
Realm::Realm(uint64_t seed,int regions,int people,bool advisor_fixture,GameplayConfig config,int complex_actors,uint32_t capability_mask):config_(std::move(config)),capability_mask_(capability_mask),regions_(regions),seed_(seed),rng_(seed){
    if((regions!=1&&regions!=2&&regions!=6&&regions!=int(kAtlasTerritoryCount)-1)||people<128||people>1000000)throw std::invalid_argument("realm profile bounds");
    if(capability_mask_==0||(capability_mask_&~kDefaultCapabilityMask)!=0)throw std::invalid_argument("capability schema mask");
    if(complex_actors<0||complex_actors>2048||(complex_actors!=0&&(complex_actors<128||complex_actors%2!=0||complex_actors>people)))throw std::invalid_argument("realm complex actor bounds");
    provinces_.resize(regions+1);
    municipal_funds_.assign(size_t(regions+1),0);
    urban_housing_capacity_.assign(size_t(regions+1),0);
    if(regions==int(kAtlasTerritoryCount)-1){
        municipal_councils_.resize(kAtlasTerritoryCount);
        for(size_t site=0;site<municipal_councils_.size();++site)municipal_councils_[site].site_id=uint8_t(site);
    }
    urban_residents_cache_.assign(size_t(regions+1),0);
    urban_arrivals_reserved_.assign(size_t(regions+1),0);
    municipal_last_funded_period_.assign(size_t(regions+1),-1);
    military_ration_depots_.resize(regions+1);
    military_supplementary_ration_depots_.resize(regions+1);
    market_monthly_.resize(regions+1);
    if(regions<=6)for(int r=0;r<=regions;++r){routes_[r][(r+1)%(regions+1)]=true;routes_[(r+1)%(regions+1)][r]=true;}
    if(regions==int(kAtlasTerritoryCount)-1)
        national_transport_routes_=NationalTransportManager::default_routes();
    for(int r=0;r<=regions;++r){auto&p=provinces_[r];for(int legacy=0;legacy<kLegacyGoodCount;++legacy){const int g=canonical_from_legacy_good(legacy);p.stock[size_t(g)]=200+int(rng_()%150);opening_[size_t(g)]+=p.stock[size_t(g)];}p.jobs=std::max(32,people/(regions+1));p.school=std::max(8,people/(regions+1)/8);}
    if(regions==int(kAtlasTerritoryCount)-1){
        provinces_[2].stock[size_t(GoodId::SupplementaryFood)]=kMilitarySupplementaryRationOpeningStock;
        opening_[size_t(GoodId::SupplementaryFood)]+=kMilitarySupplementaryRationOpeningStock;
        for(auto& partner:foreign_trade_partners_){
            partner.money=kForeignTradePartnerOpeningCash;
            partner.goods[size_t(GoodId::DryFood)]=kForeignTradePartnerOpeningDryFood;
            partner.salt_capacity=kForeignTradePartnerSaltCapacity;
            opening_[size_t(GoodId::DryFood)]+=kForeignTradePartnerOpeningDryFood;
        }
    }
    people_.reserve(people+1000);
    const int leaders=complex_actors?complex_actors/2:(people>=10000?64:regions+1);
    for(int i=0;i<people;++i){Person p;p.id=next_id_++;p.personality=seeded_personality(seed_,p.id);p.region=i%(regions+1);p.household_membership.household_id = p.id;p.age_days=(i<2*leaders?35:((i%5==0)?8+int(rng_()%8):18+int(rng_()%44)))*365;
        p.family=i<leaders?Family::Governance:(i<2*leaders?Family::Economic:Family::Civilian);
        if(p.family!=Family::Civilian)p.region=i%(regions+1);p.taxing_authority=i<regions+1;p.sovereign=i==0;if(i<regions+1)p.region=i==0?regions:i-1;
        if(advisor_fixture&&i==127){p.family=Family::Governance;p.age_days=35*365;p.taxing_authority=false;p.sovereign=false;}
        p.cash=p.family==Family::Economic?500:100;p.skill=5+int(rng_()%30);p.goal=int(rng_()%3);
        for(auto capability:kLegacyCapabilities)p.stats.set(capability,25+int(rng_()%56));
        p.stats.set(kLegacyCapabilities[size_t(i)%kLegacyCapabilities.size()],65+int(rng_()%21));
        people_.push_back(p);
        if(i>=2*leaders){uint64_t base=2*leaders+1+uint64_t((i-2*leaders)/4)*4;people_.back().household_membership.household_id = base;people_.back().region=int((i-2*leaders)/4)%(regions+1);edges_.push_back({p.id,base,RelationKind::ParentChild});if(i>2*leaders)edges_.push_back({p.id,p.id-1,RelationKind::Sibling});}
    }
    if(regions_==int(kAtlasTerritoryCount)-1){
        for(auto& person:people_){
            if(urban_residence_hash_v1(seed_,person.id)%5==0){
                person.residence_kind=ResidenceKind::Urban;
                person.urban_site_id=static_cast<uint8_t>(person.region);
            }else person.residence_kind=ResidenceKind::Rural;
        }
    }
    rebuild_kinship_indices();
    secret_known_counts_.assign(people_.size(),0);
    knowledge_observation_counts_.assign(people_.size(),{});
    reindex_households();
    rebuild_personal_establishment_index();
    // The starting land grant is explicit scenario endowment: one abstract
    // holding for each founding household member. It changes no money or goods.
    land_holdings_.reserve(households_.size());
    for (const auto& [household, members] : households_) {
        (void)members;
        land_holdings_.push_back({next_id_++, people_[household - 1].region, 1, household});
    }
    std::array<int,4> pool{};int roll=int(rng_()%100);draft_.tier=roll<50?0:(roll<85?1:2);
    for(int i=0;i<4;++i)pool[i]=4*draft_.tier+i;std::shuffle(pool.begin(),pool.end(),rng_);
    for(int i=0;i<3;++i){draft_.cards[i]=pool[i];draft_.seen|=1<<pool[i];}
    for(auto&v:provinces_)v.food_capacity=v.stock[0];opening_money_=total_money();refresh_demography();initialize_urban_housing_baseline();prices();
}
const Person& Realm::person(uint64_t id) const {if(id==0||id>people_.size())throw std::out_of_range("person ID");return people_[id-1];}
std::vector<UrbanCensus> Realm::urban_census() const {
    if(regions_!=int(kAtlasTerritoryCount)-1)return {};
    std::vector<UrbanCensus> result;
    result.reserve(kAtlasUrbanSites.size());
    for(const auto& site:kAtlasUrbanSites){
        const auto capacity=urban_housing_capacity_.at(size_t(site.territory_id));
        result.push_back({site.id,site.territory_id,0,0,0,capacity,0,0});
    }
    for(const auto& person:people_)if(person.alive){
        auto& row=result[size_t(person.region)];
        switch(person.residence_kind){
        case ResidenceKind::Urban:++row.urban_residents;break;
        case ResidenceKind::Rural:++row.rural_residents;break;
        case ResidenceKind::Unknown:++row.unknown_residence;break;
        }
    }
    for(auto& row:result){
        int64_t private_occupants=0,private_beds=0,private_vacancies=0;
        for(const auto& home:private_residences_)if(home.completed&&home.tenant_household&&
            home.region==row.territory_id&&home.lease_end_day>day_){
            int64_t occupants=0;
            for(const auto& person:people_)if(person.alive&&person.residence_kind==ResidenceKind::Urban&&
                person.household_membership.household_id==home.tenant_household&&
                (person.region==home.region||person.destination==home.region))++occupants;
            private_occupants+=occupants;
            private_beds+=kPrivateResidenceBeds;
            private_vacancies+=std::max<int64_t>(0,kPrivateResidenceBeds-occupants);
        }
        const auto allUsage=int64_t(row.urban_residents)+urban_arrivals_reserved_.at(size_t(row.territory_id));
        const auto publicUsage=std::max<int64_t>(0,allUsage-private_occupants);
        row.housing_capacity+=private_beds;
        row.unhoused_residents=std::max<int64_t>(0,publicUsage-urban_housing_capacity_[size_t(row.territory_id)]);
        row.available_beds=std::max<int64_t>(0,urban_housing_capacity_[size_t(row.territory_id)]-publicUsage)+private_vacancies;
    }
    return result;
}
Realm Realm::scenario(uint64_t seed,uint64_t actor,int age,int stat,int goal,int budget,int schools,int jobs,int nutrition,int health,uint32_t capability_mask){
    Realm w(seed,6,256,false,GameplayConfig{},0,capability_mask);auto&p=w.mutable_person(actor);
    if(age>100||stat>100||goal>2||budget>10000||schools>1000||jobs>1000||nutrition>100||health>100)throw std::invalid_argument("Scenario fixture bounds");
    if(age>=0)p.age_days=age*365;if(stat>=0)p.stats.values.fill(stat);if(goal>=0)p.goal=goal;if(budget>=0)p.cash=budget;
    if(nutrition>=0)p.nutrition=nutrition;if(health>=0){p.health=health;if(health==0){p.alive=false;w.legacy_estate_unassigned_+=p.cash;w.transfer(p.cash,w.estate_,p.cash);}}
    if(schools>=0)w.provinces_[p.region].school=schools;if(jobs>=0)w.provinces_[p.region].jobs=jobs;
    w.opening_money_=w.total_money();w.refresh_demography();return w;
}
Person& Realm::mutable_person(uint64_t id){return const_cast<Person&>(std::as_const(*this).person(id));}
void Realm::transfer(int64_t&from,int64_t&to,int64_t amount){FinanceManager::transfer(from,to,amount);}
void Realm::index_household(uint64_t household,uint64_t person){
    if(!household||!person)throw std::logic_error("household membership label");
    auto&members=households_[household];
    if(members.empty()||members.back()<person)members.push_back(person);
    else if(std::find(members.begin(),members.end(),person)==members.end())members.insert(std::lower_bound(members.begin(),members.end(),person),person);
}
void Realm::reindex_households(){households_=HouseholdIndex::build(people_);}
void Realm::index_kinship_edge(const Edge&edge){
    if(!edge.from||!edge.to||edge.from>people_.size()||edge.to>people_.size())return;
    if(edge.kind==RelationKind::ParentChild){
        auto&parents=parents_by_child_[edge.from];
        if(std::find(parents.begin(),parents.end(),edge.to)==parents.end()){
            parents.push_back(edge.to);
            if(edge.from!=edge.to){
                auto&siblings=children_by_parent_[edge.to];
                for(const auto sibling:siblings){
                    siblings_by_person_[edge.from].insert(sibling);
                    siblings_by_person_[sibling].insert(edge.from);
                }
                siblings.insert(edge.from);
            }
        }
    }
    else if(edge.kind==RelationKind::Sibling){
        siblings_by_person_[edge.from].insert(edge.to);
        siblings_by_person_[edge.to].insert(edge.from);
    }
}
void Realm::rebuild_kinship_indices(){
    parents_by_child_.clear();children_by_parent_.clear();siblings_by_person_.clear();
    for(const auto&edge:edges_)index_kinship_edge(edge);
}
bool Realm::households_consistent()const{
    size_t total=0;
    for(const auto&[label,members]:households_){
        if(!label||label>people_.size()||members.empty())return false;
        total+=members.size();
        for(uint64_t id:members)if(!id||id>people_.size()||people_[id-1].household_membership.household_id!=label)return false;
    }
    return total==people_.size();
}
uint64_t Realm::guardian_of(const Person& child)const{
    return HouseholdIndex::guardian(people_,households_,child);
}
uint64_t Realm::guardian(uint64_t id)const{return guardian_of(person(id));}
uint64_t Realm::household_authority(uint64_t household)const{
    return HouseholdIndex::authority(people_,households_,household);
}
uint64_t Realm::household_of(uint64_t id)const{return person(id).household_membership.household_id;}
std::vector<uint64_t> Realm::household_members(uint64_t household)const{auto record=households_.find(household);return record==households_.end()?std::vector<uint64_t>{}:record->second;}
bool Realm::same_household(uint64_t a,uint64_t b)const{if(!a||!b||a>people_.size()||b>people_.size())return false;return people_[a-1].household_membership.household_id==people_[b-1].household_membership.household_id;}
size_t Realm::household_records()const{return households_.size();}
int64_t Realm::available_funds(uint64_t id)const{const auto&p=person(id);const int64_t fund=household_fund(p.household_membership.household_id);if(p.cash>std::numeric_limits<int64_t>::max()-fund)throw std::overflow_error("Available household funds are not representable");return p.cash+fund;}
void Realm::spend(Person&p,int64_t&to,int64_t amount){if(amount<0)throw std::logic_error("Negative household expense");const auto own=std::min(p.cash,amount);transfer(p.cash,to,own);const int64_t remainder=amount-own;if(remainder){auto fund=household_funds_.find(p.household_membership.household_id);if(fund==household_funds_.end()||fund->second<remainder)throw std::logic_error("Insufficient household fund for personal expense");transfer(fund->second,to,remainder);if(fund->second==0)household_funds_.erase(fund);}}
int64_t Realm::debt(uint64_t id)const{
    int64_t result=0;
    for(const auto&loan:loans_)if(loan.debtor==id){
        const int64_t due=outstanding(loan);
        if(due>std::numeric_limits<int64_t>::max()-result)return std::numeric_limits<int64_t>::max();
        result+=due;
    }
    return result;
}
void Realm::event(uint64_t actor,int region,int kind,int64_t amount,uint64_t target,bool secret,uint64_t parent){
    Fact f{next_fact_++,parent,actor,target,day_,region,kind,amount,secret};facts_.push_back(f);
    if(secret){
        if(actor&&actor<=secret_known_counts_.size())++secret_known_counts_[actor-1];
        if(target&&target<=secret_known_counts_.size()&&target!=actor)++secret_known_counts_[target-1];
    }
    // Local public reporting takes time; secrets require a completed investigation.
    if(facts_.size()>8192){auto cutoff=facts_[4096].id;facts_.erase(facts_.begin(),facts_.begin()+4096);std::erase_if(knowledge_,[cutoff](auto&k){return k.fact<cutoff;});for(auto&[observer,known]:knowledge_index_)std::erase_if(known,[cutoff](auto&item){return item.first<cutoff;});rebuild_secret_known_counts();}
}
void Realm::rebuild_secret_known_counts(){
    secret_known_counts_.assign(people_.size(),0);
    knowledge_observation_counts_.assign(people_.size(),{});
    for(const auto& row:knowledge_)if(row.observer>0&&row.observer<=people_.size()){
        if(row.kind==KnowledgeKind::SecretEvidence)++knowledge_observation_counts_[row.observer-1][0];
        else if(row.kind==KnowledgeKind::UnknownLegacy)++knowledge_observation_counts_[row.observer-1][1];
    }
    for(const auto&f:facts_)if(f.secret){
        if(f.actor&&f.actor<=people_.size())++secret_known_counts_[f.actor-1];
        if(f.target&&f.target<=people_.size()&&f.target!=f.actor)++secret_known_counts_[f.target-1];
    }
    for(const auto&[observer,known]:knowledge_index_){
        if(!observer||observer>people_.size())continue;
        for(const auto&[fact_id,arrives]:known){
            if(arrives>day_)continue;
            const auto fact=std::lower_bound(facts_.begin(),facts_.end(),fact_id,[](const Fact&f,uint64_t id){return f.id<id;});
            if(fact==facts_.end()||fact->id!=fact_id||!fact->secret||fact->actor==observer||fact->target==observer)continue;
            ++secret_known_counts_[observer-1];
        }
    }
}
bool Realm::knows(uint64_t observer,const Fact&f)const{
    if(f.kind==108&&observer==1)return f.day+report_delay(true)<=day_;
    if(f.actor==observer||f.target==observer)return true;
    if(!f.secret){if(observer==1&&f.day+report_delay(f.kind==108)<=day_)return true;return f.region==person(observer).region&&f.day<day_;}
    auto observer_it=knowledge_index_.find(observer);if(observer_it!=knowledge_index_.end()){auto fact_it=observer_it->second.find(f.id);if(fact_it!=observer_it->second.end()&&fact_it->second<=day_)return true;}
    return false;
}
std::vector<uint64_t> Realm::due(Family family)const{
    std::vector<uint64_t> ids;int cadence=family==Family::Economic?7:30;
    for(auto&p:people_)if(p.alive&&p.family==family&&p.busy_until<=day_&&p.last_decision_day!=day_&&day_%cadence==int(p.id%cadence))ids.push_back(p.id);return ids;
}
bool Realm::military_service_eligible(const Person& candidate)const{
    if(!candidate.alive||candidate.family!=Family::Civilian||candidate.age_days<18*365||
       candidate.destination!=-1||candidate.military_service.active||candidate.office_appointment.hired_by!=0||
       describe_job(candidate,day_)!=nullptr)return false;
    const auto sessions=schedule_book_.sessions.find(candidate.id);
    return sessions==schedule_book_.sessions.end()||std::none_of(sessions->second.begin(),sessions->second.end(),[&](const ActivitySession& session){
        return !session.retired&&session.kind==ActivityKind::Work&&session.end>schedule_book_.cursor;
    });
}
void Realm::end_military_service(Person& candidate,int fact_kind){
    auto& service=candidate.military_service;
    if(!service.active)return;
    const auto commander=service.commander;
    const auto region=service.source_region;
    const auto paid=service.paid_total;
    if(region<0||region>regions_)throw std::logic_error("invalid military service treasury region");
    transfer(service.escrow,provinces_[region].treasury,service.escrow);
    service={};
    event(candidate.id,candidate.region,fact_kind,paid,commander);
}
std::array<Candidate,realm_actions> Realm::candidates(uint64_t id)const{
    auto&p=person(id);std::array<Candidate,realm_actions> c{};
    if(regions_==int(kAtlasTerritoryCount)-1&&p.residence_kind==ResidenceKind::Urban&&p.urban_site_id<kAtlasTerritoryCount&&p.age_days>=18*365&&p.alive){
        const auto& council=municipal_councils_.at(p.urban_site_id);
        if(council.representative==id&&day_<council.term_end_day)c[size_t(realm_council_nomination_candidate)]={Intent::NominateCouncil,p.urban_site_id,2};
        else if((council.representative==0||day_>=council.term_end_day)&&council.nominee==0)c[size_t(realm_council_nomination_candidate)]={Intent::NominateCouncil,p.urban_site_id,0};
        else if(council.nominee==id)c[size_t(realm_council_nomination_candidate)]={Intent::NominateCouncil,p.urban_site_id,1};
        if(council.representative==id&&day_<council.term_end_day){
            c[281]={Intent::Project,p.region,4,kUrbanHousingProjectBudget,kUrbanHousingProjectTermDays};
            c[31]={Intent::Resign,p.region};
        }
    }
    if(p.sovereign&&regions_==int(kAtlasTerritoryCount)-1){
        for(size_t site=0;site<municipal_councils_.size();++site){
            const auto& council=municipal_councils_[site];
            if(council.nominee==0)continue;
            c[size_t(realm_council_accept_candidate_begin)+site]={Intent::ResolveCouncilNomination,int(site),1};
            c[size_t(realm_council_reject_candidate_begin)+site]={Intent::ResolveCouncilNomination,int(site),0};
        }
    }
    if(p.family==Family::Economic){
        for(int legacy=0;legacy<kLegacyGoodCount;++legacy){const int good=canonical_from_legacy_good(legacy);c[1+legacy]={Intent::Buy,p.region,good,4};c[8+legacy]={Intent::Sell,p.region,good,4};}
        for(int ordinal=0;ordinal<kExtraGoodCount;++ordinal){const int good=canonical_good_for_extra_slot(kExtraBuyBegin+ordinal);c[kExtraBuyBegin+ordinal]={Intent::Buy,p.region,good,4};c[kExtraSellBegin+ordinal]={Intent::Sell,p.region,good,4};}
        for(int r=0;r<7;++r){
            if(regions_==int(kAtlasTerritoryCount)-1){
                const bool mounted=p.inventory[size_t(GoodId::WarHorse)]>0;
                c[15+r]={Intent::Travel,r,mounted?int(GoodId::WarHorse):0,0,person_travel_days(p,r)};
            }else c[15+r]={Intent::Travel,r};
        }
        for(int r=realm_extended_travel_region_begin;r<=regions_;++r){
            const bool mounted=regions_==int(kAtlasTerritoryCount)-1&&p.inventory[size_t(GoodId::WarHorse)]>0;
            c[size_t(realm_extended_travel_candidate_begin+r-realm_extended_travel_region_begin)]=
                regions_==int(kAtlasTerritoryCount)-1
                    ?Candidate{Intent::Travel,r,mounted?int(GoodId::WarHorse):0,0,person_travel_days(p,r)}
                    :Candidate{Intent::Travel,r};
        }
        c[22]={Intent::Borrow,p.region,0,100,180};c[23]={Intent::Repay,p.region,0,std::min<int64_t>(100,debt(id))};
        c[24]={Intent::Project,p.region,0,200,60};c[25]={Intent::Hire,p.region,0,20,30};
        c[26]={Intent::Offer,1+p.region,0,50,30};c[27]={Intent::Project,p.region,1,150,45};c[30]={Intent::Work,p.region,0,0,30};
    }else if(p.family==Family::Governance){
        c[281]={Intent::Project,p.region,4,kUrbanHousingProjectBudget,kUrbanHousingProjectTermDays};
        c[1]={Intent::Tax,p.region,0,5};c[2]={Intent::Tax,p.region,0,10};c[3]={Intent::Tax,p.region,0,20};
        c[4]={Intent::Project,p.region,0,200,60};c[5]={Intent::Project,p.region,1,150,45};
        for(int r=0;r<7;++r)c[6+r]={Intent::Aid,r,0,std::max<int64_t>(1,provinces_[p.region].treasury/10)};
        c[13]={Intent::Investigate,p.region,0,30,10};c[14]={Intent::Offer,(int(id)%(regions_+1))+1,0,50,30};
        c[15]={Intent::Appoint,p.region,0,20,30};c[16]={Intent::Mobilize,p.region,0,40,30};
        c[17]={Intent::Borrow,p.region,0,100,180};
        if(p.sovereign&&draft_.chosen==4)c[17]={Intent::Borrow,p.region,1,std::min<int64_t>(100,credit_pool_),1825};
        c[19]={Intent::Project,p.region,2,120,60};c[20]={Intent::Project,p.region,3,120,60};
        if(p.sovereign&&pact_)c[21]={Intent::Offer,0,2,100,90};
        if(p.sovereign&&draft_.chosen==2)c[22]={Intent::Offer,2,1,50,30};
        if(p.office_appointment.hired_by||std::any_of(municipal_councils_.begin(),municipal_councils_.end(),[&](const MunicipalCouncil& council){return council.representative==id&&day_<council.term_end_day;}))c[31]={Intent::Resign,p.region};c[18]={Intent::Repay,p.region,0,std::min<int64_t>(100,debt(id))};
        int selected=0;
        for(const auto& target:people_)if(selected<realm_military_service_candidate_count&&military_service_eligible(target)&&target.region==p.region){
            c[size_t(realm_military_service_candidate_begin+selected)]={Intent::Offer,int(target.id),3,kMilitaryServiceTrialWagePerDay,30};
            ++selected;
        }
        for(int purchase=0;purchase<realm_military_ration_purchase_candidate_count;++purchase)
            c[size_t(realm_military_ration_purchase_candidate_begin+purchase)]={Intent::BuyMilitaryRations,p.region,
                int(GoodId::DryFood),kMilitaryRationPurchaseQuantities[size_t(purchase)],0};
        for(int purchase=0;purchase<realm_supplementary_ration_purchase_candidate_count;++purchase)
            c[size_t(realm_supplementary_ration_purchase_candidate_begin+purchase)]={Intent::BuyMilitaryRations,p.region,
                int(GoodId::SupplementaryFood),kMilitaryRationPurchaseQuantities[size_t(purchase)],0};
        if(regions_==int(kAtlasTerritoryCount)-1){
            const int64_t shipment_quantity=std::min<int64_t>(64,military_ration_depots_[size_t(p.region)]);
            for(int destination=0;destination<=regions_;++destination)if(destination!=p.region)
                c[size_t(realm_military_ration_shipment_candidate_begin+destination)]={Intent::ShipMilitaryRations,
                    destination,int(GoodId::DryFood),shipment_quantity,int(TransportServiceGoal::Fastest)};
            const int64_t supplementary_quantity=std::min<int64_t>(64,military_supplementary_ration_depots_[size_t(p.region)]);
            for(int destination=0;destination<=regions_;++destination)if(destination!=p.region)
                c[size_t(realm_supplementary_ration_shipment_candidate_begin+destination)]={Intent::ShipMilitaryRations,
                    destination,int(GoodId::SupplementaryFood),supplementary_quantity,int(TransportServiceGoal::Fastest)};
        }
        if(p.taxing_authority&&!p.sovereign&&day_<=365000-30){
            const Person* sovereign=nullptr;
            for(const auto& candidate:people_)if(candidate.alive&&candidate.sovereign){sovereign=&candidate;break;}
            bool has_open=false;
            for(const auto& obligation:military_obligations_)
                if(obligation.obligor==id&&obligation.completed_day<0){has_open=true;break;}
            for(const auto& proposal:proposals_)
                if(!proposal.resolved&&proposal.from==id&&proposal.kind==5){has_open=true;break;}
            if(sovereign&&!has_open){
                const auto eligible=eligible_muster_members(id,p.region,day_+30);
                if(!eligible.empty())c[size_t(realm_military_obligation_offer_candidate)]={
                    Intent::Offer,int(sovereign->id),4,std::min<int64_t>(4,int64_t(eligible.size())),14};
            }
            for(const auto& obligation:military_obligations_)
                if(obligation.obligor==id&&obligation.completed_day<0&&obligation.due_checked&&
                   obligation.shortage_at_last_deadline>0&&obligation.extensions==0&&day_<=365000-8){
                    c[size_t(realm_military_obligation_renegotiate_candidate)]={
                        Intent::Offer,int(obligation.recipient),5,7,7};
                    break;
                }
            if(c[size_t(realm_military_supply_obligation_candidate)].kind==Intent::Hold){
                for(const auto& obligation:military_supply_obligations_)
                    if(obligation.obligor==id&&obligation.completed_day<0&&obligation.due_checked&&
                       obligation.shortage_at_last_deadline>0&&obligation.extensions==0&&day_<=365000-8){
                        c[size_t(realm_military_supply_obligation_candidate)]={Intent::Offer,
                            int(obligation.recipient),7,7,7};
                        break;
                    }
            }
            if(c[size_t(realm_military_supply_obligation_candidate)].kind==Intent::Hold){
                for(const auto& source:military_obligations_){
                    if(source.obligor!=id||source.completed_day>=0)continue;
                    bool already=false;
                    for(const auto& supply:military_supply_obligations_)
                        if(supply.personnel_obligation_id==source.id){already=true;break;}
                    for(const auto& proposal:proposals_)
                        if(!proposal.resolved&&proposal.kind==7&&proposal.cause==source.id){already=true;break;}
                    if(!already&&source.required_members>=1&&source.required_members<=4){
                        c[size_t(realm_military_supply_obligation_candidate)]={Intent::Offer,
                            int(source.recipient),6,source.required_members,14};
                        break;
                    }
                }
            }
        }
    }else{
        for(int r=0;r<7;++r){
            c[1+r]={Intent::Work,r,0,0,30};
            if(regions_==int(kAtlasTerritoryCount)-1){
                const bool mounted=p.inventory[size_t(GoodId::WarHorse)]>0;
                c[8+r]={Intent::Migrate,r,mounted?int(GoodId::WarHorse):0,0,person_travel_days(p,r)};
            }else c[8+r]={Intent::Migrate,r,0,10,5+std::abs(r-p.region)};
        }
        for(int r=realm_extended_travel_region_begin;r<=regions_;++r){
            if(regions_==int(kAtlasTerritoryCount)-1){
                const bool mounted=p.inventory[size_t(GoodId::WarHorse)]>0;
                c[size_t(realm_extended_travel_candidate_begin+r-realm_extended_travel_region_begin)]=
                    {Intent::Migrate,r,mounted?int(GoodId::WarHorse):0,0,person_travel_days(p,r)};
            }else c[size_t(realm_extended_travel_candidate_begin+r-realm_extended_travel_region_begin)]={Intent::Migrate,r,0,10,5+std::abs(r-p.region)};
        }
        c[15]={Intent::Study,p.region,0,5,30};c[16]={Intent::Rest,p.region,0,2,7};
        c[17]={Intent::Social,p.region,0,1,7};c[18]={Intent::Join,p.region,0,2,30};
        if(p.family==Family::Civilian&&p.alive&&p.age_days>=6*365&&
           p.inventory[size_t(GoodId::Tea)]>0)
            c[size_t(realm_tea_social_candidate)]={Intent::Social,p.region,int(GoodId::Tea),1,7};
        if(p.family==Family::Civilian&&p.alive&&p.age_days>=6*365&&
           p.inventory[size_t(GoodId::Incense)]>0)
            c[size_t(realm_incense_social_candidate)]={Intent::Social,p.region,int(GoodId::Incense),2,7};
        if(p.family==Family::Civilian&&p.alive&&p.age_days>=6*365&&
           p.inventory[size_t(GoodId::Silver)]>0)
            c[size_t(realm_silver_social_candidate)]={Intent::Social,p.region,int(GoodId::Silver),2,7};
        for(int group=1;group<4;++group)c[20+group]={Intent::Join,p.region,group,2,30};
        auto authority=household_authority(p.household_membership.household_id);if(authority)c[19]={Intent::Support,int(authority),0,5};c[20]={Intent::Buy,p.region,0,1};
    }
    if(p.family==Family::Economic&&regions_==int(kAtlasTerritoryCount)-1&&p.region==8){
        constexpr std::array<int64_t,3> quantities{1,4,8};
        constexpr std::array<int,3> partner_candidate_begins{
            realm_foreign_trade_candidate_begin,
            realm_foreign_trade_partner_candidate_begin,
            realm_foreign_trade_partner_candidate_begin+6,
        };
        for(int partner=0;partner<kForeignTradePartnerCount;++partner)
            for(int tier=0;tier<3;++tier){
                c[size_t(partner_candidate_begins[size_t(partner)]+tier)]={Intent::ForeignTrade,8,int(GoodId::DryFood),quantities[size_t(tier)],int(foreign_trade_operation(partner,true))};
                c[size_t(partner_candidate_begins[size_t(partner)]+3+tier)]={Intent::ForeignTrade,8,int(GoodId::Salt),quantities[size_t(tier)],int(foreign_trade_operation(partner,false))};
        }
    }
    if(p.family==Family::Governance)
        for(int i=0;i<realm_municipal_fund_candidate_count;++i){
            const int percentage=kMunicipalGrantPercentages[size_t(i)];
            c[size_t(realm_municipal_fund_candidate_begin+i)]={Intent::FundMunicipal,p.region,percentage,
                municipal_grant_amount(provinces_[size_t(p.region)].treasury,percentage),0};
        }
    if(p.family==Family::Governance&&p.taxing_authority&&provinces_[size_t(p.region)].charter){
        const int64_t municipal_balance=municipal_funds_[size_t(p.region)];
        if(municipal_balance>0){
            const int64_t trial_amount=std::max<int64_t>(1,municipal_balance/10);
            c[size_t(realm_municipal_relief_candidate)]={Intent::Aid,p.region,1,
                std::min<int64_t>({1000000,municipal_balance,trial_amount})};
        }
    }
    if(p.family==Family::Civilian&&p.military_service.active)c[31]={Intent::Resign,p.region};
    const auto candidateMix=[](uint64_t x){x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);};
    if(people_.size()>1){
        std::array<uint64_t,realm_marriage_candidate_count> selected{};
        const uint64_t base=candidateMix(id^candidateMix(p.sampling)^candidateMix(uint64_t(day_)));
        for(int i=0;i<realm_marriage_candidate_count;++i){
            uint64_t target=base+i*0x9e3779b97f4a7c15ULL;
            target=candidateMix(target)%people_.size()+1;
            for(size_t attempts=0;attempts<people_.size()&&(target==id||std::find(selected.begin(),selected.begin()+i,target)!=selected.begin()+i);++attempts)
                target=target%people_.size()+1;
            if(target==id||std::find(selected.begin(),selected.begin()+i,target)!=selected.begin()+i)continue;
            selected[size_t(i)]=target;
            c[size_t(realm_marriage_candidate_begin+i)]={Intent::Marry,int(target),0,50,30};
        }
    }
    constexpr std::array<int64_t,3> establishmentFunding{25,50,100};
    for(int industry=1;industry<=7;++industry)for(int tier=0;tier<3;++tier){
        const int slot=realm_establishment_candidate_begin+(industry-1)*3+tier;
        c[size_t(slot)]={Intent::CreateEstablishment,p.region,industry,
                         establishmentFunding[size_t(tier)],-1};
    }
    if(p.region==14)for(int tier=0;tier<3;++tier)
        c[size_t(realm_tea_garden_candidate_begin+tier)]={Intent::CreateEstablishment,p.region,
            int(EstablishmentIndustry::TeaGarden),establishmentFunding[size_t(tier)],-1};
    if(p.region==11)for(int tier=0;tier<3;++tier)
        c[size_t(realm_incense_works_candidate_begin+tier)]={Intent::CreateEstablishment,p.region,
            int(EstablishmentIndustry::IncenseWorks),establishmentFunding[size_t(tier)],-1};
    if(p.region==16)for(int tier=0;tier<3;++tier)
        c[size_t(realm_warhorse_stable_candidate_begin+tier)]={Intent::CreateEstablishment,p.region,
            int(EstablishmentIndustry::WarhorseStable),establishmentFunding[size_t(tier)],-1};
    if(p.region==19)for(int tier=0;tier<3;++tier)
        c[size_t(realm_weapon_forge_candidate_begin+tier)]={Intent::CreateEstablishment,p.region,
            int(EstablishmentIndustry::WeaponForge),establishmentFunding[size_t(tier)],-1};
    constexpr std::array<EstablishmentIndustry,3> atlasProducts{
        EstablishmentIndustry::Glassworks, EstablishmentIndustry::PotteryKiln,
        EstablishmentIndustry::Tannery};
    constexpr std::array<int,3> atlasProductRegions{10,15,17};
    for(size_t product=0;product<atlasProducts.size();++product){
        if(p.region!=atlasProductRegions[product])continue;
        for(int tier=0;tier<3;++tier){
            const int offset=int(product)*3+tier;
            c[size_t(realm_specialty_product_candidate_begin+offset)]={Intent::CreateEstablishment,
                p.region,int(atlasProducts[product]),establishmentFunding[size_t(tier)],-1};
            if(p.family==Family::Governance)
                c[size_t(realm_municipal_specialty_product_candidate_begin+offset)]={Intent::CreateEstablishment,
                    p.region,int(atlasProducts[product]),establishmentFunding[size_t(tier)],-2};
        }
    }
    if(p.region==20)for(int tier=0;tier<3;++tier){
        c[size_t(realm_specialty_product_candidate_begin+tier)]={Intent::CreateEstablishment,p.region,
            int(EstablishmentIndustry::MedicinalGarden),establishmentFunding[size_t(tier)],-1};
        if(p.family==Family::Governance)
            c[size_t(realm_municipal_specialty_product_candidate_begin+tier)]={Intent::CreateEstablishment,p.region,
                int(EstablishmentIndustry::MedicinalGarden),establishmentFunding[size_t(tier)],-2};
    }
    if(p.region==2)for(int tier=0;tier<3;++tier){
        c[size_t(realm_specialty_product_candidate_begin+tier)]={Intent::CreateEstablishment,p.region,
            int(EstablishmentIndustry::SeedPressWorks),establishmentFunding[size_t(tier)],-1};
        if(p.family==Family::Governance)
            c[size_t(realm_municipal_specialty_product_candidate_begin+tier)]={Intent::CreateEstablishment,p.region,
                int(EstablishmentIndustry::SeedPressWorks),establishmentFunding[size_t(tier)],-2};
    }
    if(p.region==6)for(int tier=0;tier<3;++tier){
        c[size_t(realm_specialty_product_candidate_begin+tier)]={Intent::CreateEstablishment,p.region,
            int(EstablishmentIndustry::SawmillWorks),establishmentFunding[size_t(tier)],-1};
        if(p.family==Family::Governance)
            c[size_t(realm_municipal_specialty_product_candidate_begin+tier)]={Intent::CreateEstablishment,
                p.region,int(EstablishmentIndustry::SawmillWorks),establishmentFunding[size_t(tier)],-2};
    }
    if(p.family==Family::Governance){
        for(int industry=1;industry<=7;++industry)for(int tier=0;tier<3;++tier){
            const int slot=realm_municipal_workshop_candidate_begin+(industry-1)*3+tier;
            c[size_t(slot)]={Intent::CreateEstablishment,p.region,industry,establishmentFunding[size_t(tier)],-2};
        }
        const auto municipalSpecial=[&](int offset,EstablishmentIndustry industry,int sourceRegion){
            if(p.region!=sourceRegion)return;
            for(int tier=0;tier<3;++tier)
                c[size_t(realm_municipal_special_candidate_begin+offset+tier)]={Intent::CreateEstablishment,
                    p.region,int(industry),establishmentFunding[size_t(tier)],-2};
        };
        municipalSpecial(0,EstablishmentIndustry::TeaGarden,14);
        municipalSpecial(3,EstablishmentIndustry::IncenseWorks,11);
        municipalSpecial(6,EstablishmentIndustry::WarhorseStable,16);
        municipalSpecial(9,EstablishmentIndustry::WeaponForge,19);
    }
    std::array<const Establishment*,realm_worksite_candidate_sites> siteRows{};
    if(!establishments_.empty()){
        size_t index=size_t(candidateMix(id^candidateMix(p.sampling)^candidateMix(uint64_t(day_)))%establishments_.size());
        const size_t count=std::min(siteRows.size(),establishments_.size());
        for(size_t row=0;row<count;++row){
            for(size_t attempt=0;attempt<establishments_.size();++attempt){
                const auto&site=establishments_[index];index=(index+1)%establishments_.size();
                if(site.id<=uint64_t(INT32_MAX)){siteRows[row]=&site;break;}
            }
        }
    }
    const auto&values=config_.values();
    auto good_for_industry=[](EstablishmentIndustry industry,int sourceGood){
        switch(industry){
        case EstablishmentIndustry::ClothWorkshop:return GoodId::Cloth;
        case EstablishmentIndustry::ToolWorkshop:return GoodId::Tools;
        case EstablishmentIndustry::MilitarySupplyWorkshop:return GoodId::MilitarySupply;
        case EstablishmentIndustry::Saltworks:return GoodId::Salt;
        case EstablishmentIndustry::Forestry:return GoodId::Timber;
        case EstablishmentIndustry::Mining:return valid_good_id(sourceGood)?static_cast<GoodId>(sourceGood):GoodId::Copper;
        case EstablishmentIndustry::PaperMill:return GoodId::Paper;
        case EstablishmentIndustry::TeaGarden:return GoodId::Tea;
        case EstablishmentIndustry::IncenseWorks:return GoodId::Incense;
        case EstablishmentIndustry::WarhorseStable:return GoodId::WarHorse;
        case EstablishmentIndustry::WeaponForge:return GoodId::Weapon;
        case EstablishmentIndustry::Glassworks:return GoodId::PlainGlass;
        case EstablishmentIndustry::PotteryKiln:return GoodId::Pottery;
        case EstablishmentIndustry::Tannery:return GoodId::Leather;
        case EstablishmentIndustry::MedicinalGarden:return GoodId::Medicine;
        case EstablishmentIndustry::SeedPressWorks:return GoodId::SupplementaryFood;
        case EstablishmentIndustry::SawmillWorks:return GoodId::ConstructionMaterial;
        }
        return GoodId::Fiber;
    };
    auto first_input=[](EstablishmentIndustry industry,int sourceGood){
        switch(industry){
        case EstablishmentIndustry::ClothWorkshop:return GoodId::Fiber;
        case EstablishmentIndustry::ToolWorkshop:return GoodId::Iron;
        case EstablishmentIndustry::MilitarySupplyWorkshop:return GoodId::Tools;
        case EstablishmentIndustry::Saltworks:return GoodId::Fuel;
        case EstablishmentIndustry::Forestry:return GoodId::Timber;
        case EstablishmentIndustry::Mining:return valid_good_id(sourceGood)?static_cast<GoodId>(sourceGood):GoodId::Copper;
        case EstablishmentIndustry::PaperMill:return GoodId::Timber;
        case EstablishmentIndustry::TeaGarden:return GoodId::Tea;
        case EstablishmentIndustry::IncenseWorks:return GoodId::Incense;
        case EstablishmentIndustry::WarhorseStable:return GoodId::Horse;
        case EstablishmentIndustry::WeaponForge:return GoodId::Iron;
        case EstablishmentIndustry::Glassworks:case EstablishmentIndustry::PotteryKiln:return GoodId::Fuel;
        case EstablishmentIndustry::Tannery:return GoodId::Leather;
        case EstablishmentIndustry::MedicinalGarden:return GoodId::Medicine;
        case EstablishmentIndustry::SeedPressWorks:return GoodId::SupplementaryFood;
        case EstablishmentIndustry::SawmillWorks:return GoodId::Timber;
        }
        return GoodId::Fiber;
    };
    for(size_t row=0;row<siteRows.size();++row){
        const int base=realm_worksite_candidate_begin+int(row)*realm_worksite_actions_per_site;
        const auto*site=siteRows[row];
        if(!site){
            c[size_t(base)]={Intent::TransferEstablishmentGoods,0,-1,0,int(EstablishmentTransferOperation::DepositGoods)};
            c[size_t(base+1)]={Intent::TransferEstablishmentGoods,0,-1,0,int(EstablishmentTransferOperation::DepositGoods)};
            c[size_t(base+2)]={Intent::TransferEstablishmentGoods,0,-1,0,int(EstablishmentTransferOperation::WithdrawGoods)};
            c[size_t(base+3)]={Intent::AuthorizeWorkAtSite,0,0,0,31};
            c[size_t(base+4)]={Intent::WorkAtSite,0,0,0,1};
            continue;
        }
        const int idValue=int(site->id);
        int64_t inputOne=0,inputTwo=0,output=0;
        GoodId input1=first_input(site->industry,site->source_good),input2=GoodId::Fiber;
        GoodId outGood=good_for_industry(site->industry,site->source_good);
        switch(site->industry){
        case EstablishmentIndustry::ClothWorkshop:inputOne=values[3];output=values[4];break;
        case EstablishmentIndustry::ToolWorkshop:
            inputOne=values[5];output=values[7];
            if(site->stock[size_t(GoodId::Copper)]<1){inputTwo=1;input2=GoodId::Copper;}
            else{inputTwo=values[6];input2=GoodId::Fuel;}
            break;
        case EstablishmentIndustry::MilitarySupplyWorkshop:inputOne=values[8];inputTwo=values[9];input2=GoodId::Cloth;output=values[10];break;
        case EstablishmentIndustry::Saltworks:inputOne=values[16];output=values[17];break;
        case EstablishmentIndustry::Forestry:case EstablishmentIndustry::Mining:case EstablishmentIndustry::TeaGarden:case EstablishmentIndustry::IncenseWorks:case EstablishmentIndustry::MedicinalGarden:case EstablishmentIndustry::SeedPressWorks:output=1;break;
        case EstablishmentIndustry::PaperMill:case EstablishmentIndustry::WarhorseStable:case EstablishmentIndustry::SawmillWorks:inputOne=1;output=1;break;
        case EstablishmentIndustry::WeaponForge:inputOne=1;inputTwo=1;input2=GoodId::Tools;output=1;break;
        case EstablishmentIndustry::Glassworks:case EstablishmentIndustry::PotteryKiln:inputOne=1;output=1;break;
        case EstablishmentIndustry::Tannery:output=1;break;
        }
        c[size_t(base)]={Intent::TransferEstablishmentGoods,idValue,int(input1),inputOne,int(EstablishmentTransferOperation::DepositGoods)};
        c[size_t(base+1)]={Intent::TransferEstablishmentGoods,idValue,inputTwo?int(input2):-1,inputTwo,int(EstablishmentTransferOperation::DepositGoods)};
        c[size_t(base+2)]={Intent::TransferEstablishmentGoods,idValue,int(outGood),output,int(EstablishmentTransferOperation::WithdrawGoods)};
        uint64_t workerCandidate=0;
        size_t workerIndex=size_t(candidateMix(id^candidateMix(site->id)^candidateMix(p.sampling)^candidateMix(uint64_t(day_)))%people_.size());
        for(size_t attempt=0;attempt<people_.size();++attempt){
            const auto&worker=people_[workerIndex];workerIndex=(workerIndex+1)%people_.size();
            if(worker.id!=id&&worker.alive&&worker.destination==-1&&worker.region==site->region){workerCandidate=worker.id;break;}
        }
        c[size_t(base+3)]={Intent::AuthorizeWorkAtSite,idValue,int(workerCandidate),0,31};
        c[size_t(base+4)]={Intent::WorkAtSite,idValue,0,0,1};
    }
    for(auto&proposal:proposals_)if(!proposal.resolved&&proposal.to==id&&proposal.due>=day_&&proposal.ready<=day_){c[28]={Intent::Accept,int(proposal.id)};c[29]={Intent::Reject,int(proposal.id)};break;}
    if(regions_==int(kAtlasTerritoryCount)-1&&p.alive&&p.region>=0&&p.region<int(kAtlasTerritoryCount)){
        std::vector<Candidate> buildOptions,offerOptions,responseOptions,endLeaseOptions;
        std::vector<LandHolding> holdings=land_holdings_;
        std::sort(holdings.begin(),holdings.end(),[](const auto&a,const auto&b){return a.id<b.id;});
        for(const auto&holding:holdings){
            if(holding.id>uint64_t(INT32_MAX)||holding.title_holder!=id||holding.region!=p.region||
               p.residence_kind!=ResidenceKind::Urban||p.urban_site_id>=kAtlasTerritoryCount||
               holding.region!=p.urban_site_id||p.age_days<18*365)continue;
            const bool hasHome=std::any_of(private_residences_.begin(),private_residences_.end(),
                [&](const auto&home){return home.holding_id==holding.id;});
            if(!hasHome)buildOptions.push_back({Intent::PrivateResidenceBuild,int(holding.id),0,0,0});
        }
        std::vector<std::pair<uint64_t,int>> authorities;
        for(const auto&candidate:people_){
            if(!candidate.alive||candidate.age_days<18*365||candidate.region<0||candidate.region>=int(kAtlasTerritoryCount)||
               candidate.residence_kind!=ResidenceKind::Urban||candidate.urban_site_id>=kAtlasTerritoryCount)continue;
            const auto household=candidate.household_membership.household_id;
            if(household&&household_authority(household)==candidate.id)authorities.emplace_back(household,candidate.region);
        }
        for(const auto&home:private_residences_){
            if(home.id>uint64_t(INT32_MAX))continue;
            const auto holding=std::lower_bound(land_holdings_.begin(),land_holdings_.end(),home.holding_id,
                [](const LandHolding&value,uint64_t key){return value.id<key;});
            if(holding==land_holdings_.end()||holding->id!=home.holding_id)continue;
            if(holding->title_holder==id&&home.completed&&!home.tenant_household&&!home.offered_household){
                for(const auto&[household,householdRegion]:authorities){
                    if(householdRegion!=home.region)continue;
                    if(household==p.household_membership.household_id)continue;
                    bool occupied=false;
                    for(const auto&other:private_residences_)
                        if(other.tenant_household==household||other.offered_household==household){occupied=true;break;}
                    if(!occupied)offerOptions.push_back({Intent::PrivateHousing,int(home.id),
                        int(PrivateHousingOperation::Offer),int64_t(household),0});
                }
            }
            if(home.offered_household&&home.offered_household==p.household_membership.household_id&&
               household_authority(home.offered_household)==id){
                responseOptions.push_back({Intent::PrivateHousing,int(home.id),int(PrivateHousingOperation::Accept),int64_t(home.offered_household),0});
                responseOptions.push_back({Intent::PrivateHousing,int(home.id),int(PrivateHousingOperation::Decline),int64_t(home.offered_household),0});
            }
            if(home.tenant_household&&(holding->title_holder==id||
               household_authority(home.tenant_household)==id))
                endLeaseOptions.push_back({Intent::PrivateHousing,int(home.id),int(PrivateHousingOperation::EndLease),int64_t(home.tenant_household),0});
        }
        const auto fillWindow=[&](const std::vector<Candidate>&options,int begin,int capacity){
            if(options.empty())return;
            const size_t offset=size_t(p.decisions%options.size());
            const size_t count=std::min<size_t>(size_t(capacity),options.size());
            for(size_t index=0;index<count;++index)
                c[size_t(begin)+index]=options[(offset+index)%options.size()];
        };
        fillWindow(buildOptions,realm_private_build_candidate_begin,realm_private_build_candidate_count);
        fillWindow(offerOptions,realm_private_offer_candidate_begin,realm_private_offer_candidate_count);
        fillWindow(responseOptions,realm_private_response_candidate_begin,realm_private_response_candidate_count);
        fillWindow(endLeaseOptions,realm_private_end_lease_candidate_begin,realm_private_end_lease_candidate_count);
    }
    return c;
}
bool Realm::legal(uint64_t id,const Candidate&c,std::string&reason)const{
    if(id==0||id>people_.size()){reason="Unknown actor";return false;}auto&p=person(id);
    if(c.kind==Intent::Hold)return true;
    if(!p.alive){reason="Actor deceased";return false;}
    bool busyBlocks=p.busy_until>day_;
    if(schedule_book_.timeline_contract)switch(c.kind){case Intent::Buy:case Intent::BuyMilitaryRations:case Intent::ShipMilitaryRations:case Intent::ForeignTrade:case Intent::Sell:case Intent::Borrow:case Intent::Repay:case Intent::Tax:case Intent::Aid:case Intent::Investigate:case Intent::Offer:case Intent::Accept:case Intent::Reject:case Intent::Support:case Intent::Hire:case Intent::Project:case Intent::Appoint:case Intent::Resign:case Intent::Marry:case Intent::CreateEstablishment:case Intent::TransferEstablishmentGoods:case Intent::AuthorizeWorkAtSite:case Intent::WorkAtSite:case Intent::FundMunicipal:case Intent::NominateCouncil:case Intent::ResolveCouncilNomination:case Intent::PrivateResidenceBuild:case Intent::PrivateHousing:busyBlocks=false;break;default:break;}
    if(busyBlocks){reason="Action duration not finished";return false;}
    if(c.amount<0||c.amount>1000000||c.target<0){reason="Invalid terms";return false;}
    if(c.kind==Intent::PrivateResidenceBuild){
        if(regions_!=int(kAtlasTerritoryCount)-1||c.target<=0||c.good!=0||c.amount!=0||c.term!=0||
           uint64_t(c.target)>uint64_t(INT32_MAX)||!p.alive||p.age_days<18*365||p.destination!=-1||
           p.busy_until>day_||p.residence_kind!=ResidenceKind::Urban||p.urban_site_id>=kAtlasTerritoryCount)return false;
        const auto holding=std::lower_bound(land_holdings_.begin(),land_holdings_.end(),uint64_t(c.target),
            [](const LandHolding&value,uint64_t key){return value.id<key;});
        if(holding==land_holdings_.end()||holding->id!=uint64_t(c.target)||holding->title_holder!=id||
           holding->region!=p.region||holding->region!=p.urban_site_id||
           std::any_of(private_residences_.begin(),private_residences_.end(),
                [&](const auto&home){return home.holding_id==holding->id;}))return false;
        if(p.cash<kPrivateResidenceBuildCost||
           p.inventory[size_t(GoodId::ConstructionMaterial)]<kPrivateResidenceConstructionMaterial||
           p.inventory[size_t(GoodId::Tools)]<kPrivateResidenceTools||
           p.inventory[size_t(GoodId::PlainGlass)]<kPrivateResidencePlainGlass||
           day_>365000-kPrivateResidenceBuildDays||
           next_id_==UINT64_MAX||next_fact_==UINT64_MAX||
           consumed_[size_t(GoodId::ConstructionMaterial)]>std::numeric_limits<int64_t>::max()-kPrivateResidenceConstructionMaterial||
           consumed_[size_t(GoodId::Tools)]>std::numeric_limits<int64_t>::max()-kPrivateResidenceTools||
           consumed_[size_t(GoodId::PlainGlass)]>std::numeric_limits<int64_t>::max()-kPrivateResidencePlainGlass)return false;
        return true;
    }
    if(c.kind==Intent::PrivateHousing){
        if(regions_!=int(kAtlasTerritoryCount)-1||c.target<=0||uint64_t(c.target)>uint64_t(INT32_MAX)||
           c.amount<=0||c.good<int(PrivateHousingOperation::Offer)||c.good>int(PrivateHousingOperation::EndLease)||c.term!=0)return false;
        const auto home=std::lower_bound(private_residences_.begin(),private_residences_.end(),uint64_t(c.target),
            [](const PrivateResidence&value,uint64_t key){return value.id<key;});
        if(home==private_residences_.end()||home->id!=uint64_t(c.target)||home->region!=p.region)return false;
        const auto holding=std::lower_bound(land_holdings_.begin(),land_holdings_.end(),home->holding_id,
            [](const LandHolding&value,uint64_t key){return value.id<key;});
        if(holding==land_holdings_.end()||holding->id!=home->holding_id||next_fact_==UINT64_MAX)return false;
        const auto operation=static_cast<PrivateHousingOperation>(c.good);
        const auto household=uint64_t(c.amount);
        if(operation==PrivateHousingOperation::Offer){
            if(!home->completed||holding->title_holder!=id||home->tenant_household||home->offered_household||
               household>people_.size()||household==p.household_membership.household_id||
               household_authority(household)==0||same_household(holding->title_holder,household)||
               day_>365000-kPrivateResidenceOfferDays)return false;
            const auto&tenant=person(household_authority(household));
            if(!tenant.alive||tenant.age_days<18*365||tenant.residence_kind!=ResidenceKind::Urban||
               tenant.region!=home->region||tenant.urban_site_id!=uint8_t(home->region))return false;
            for(const auto&other:private_residences_)
                if(other.tenant_household==household||other.offered_household==household)return false;
            return true;
        }
        if(operation==PrivateHousingOperation::Accept||operation==PrivateHousingOperation::Decline){
            if(home->offered_household!=household||household_authority(household)!=id||home->offer_expires_day<day_)return false;
            if(operation==PrivateHousingOperation::Accept&&(!home->completed||same_household(holding->title_holder,household)||
               p.residence_kind!=ResidenceKind::Urban||p.urban_site_id!=uint8_t(home->region)||
               day_>365000-kPrivateResidenceLeaseDays||day_>365000-kPrivateResidenceRentPeriodDays))return false;
            return true;
        }
        const bool landlord=holding->title_holder==id;
        const bool tenant=home->tenant_household&&home->tenant_household==household&&
            household_authority(home->tenant_household)==id;
        return home->tenant_household&&home->tenant_household==household&&(landlord||tenant);
    }
    if(c.kind==Intent::Accept||c.kind==Intent::Reject){if(legal_proposal_response(id,c))return true;reason="Proposal unavailable";return false;}
    if(c.kind==Intent::Support){if(p.family!=Family::Civilian||p.age_days<6*365||c.target<=0||c.target!=int(household_authority(p.household_membership.household_id))||uint64_t(c.target)==id||c.target>int(people_.size())||!person(c.target).alive||p.cash<c.amount){reason="Household transfer unavailable";return false;}return true;}
    if(c.kind==Intent::Marry){
        if(c.target<=0||uint64_t(c.target)>people_.size()||uint64_t(c.target)==id||c.term<1||c.term>30||p.cash<c.amount||
           !marriage_eligible(id)||!marriage_eligible(uint64_t(c.target))||marriage_related(id,uint64_t(c.target))||marriage_pending(id,uint64_t(c.target)))return false;
        return true;
    }
    if(c.kind==Intent::CreateEstablishment){
        constexpr std::array<int64_t,3> establishmentFunding{25,50,100};
        const bool validFunding=std::find(establishmentFunding.begin(),establishmentFunding.end(),c.amount)!=establishmentFunding.end();
        if(c.target!=p.region||c.good<1||c.good>17||(c.term!=-1&&c.term!=-2)||!validFunding||
           (c.good==int(EstablishmentIndustry::Saltworks)&&c.target!=8)||
           (c.good==int(EstablishmentIndustry::Forestry)&&c.target!=6)||
           (c.good==int(EstablishmentIndustry::Forestry)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::Forestry&&site.region==6;}))||
           (c.good==int(EstablishmentIndustry::Mining)&&c.target!=7&&c.target!=23)||
           (c.good==int(EstablishmentIndustry::Mining)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::Mining&&site.region==c.target;}))||
           (c.good==int(EstablishmentIndustry::PaperMill)&&c.target!=13)||
           (c.good==int(EstablishmentIndustry::PaperMill)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::PaperMill&&site.region==13;}))||
           (c.good==int(EstablishmentIndustry::TeaGarden)&&c.target!=14)||
           (c.good==int(EstablishmentIndustry::TeaGarden)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::TeaGarden&&site.region==14;}))||
           (c.good==int(EstablishmentIndustry::IncenseWorks)&&c.target!=11)||
           (c.good==int(EstablishmentIndustry::IncenseWorks)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::IncenseWorks&&site.region==11;}))||
           (c.good==int(EstablishmentIndustry::WarhorseStable)&&c.target!=16)||
           (c.good==int(EstablishmentIndustry::WarhorseStable)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::WarhorseStable&&site.region==16;}))||
           (c.good==int(EstablishmentIndustry::WeaponForge)&&c.target!=19)||
           (c.good==int(EstablishmentIndustry::WeaponForge)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::WeaponForge&&site.region==19;}))||
           (c.good==int(EstablishmentIndustry::Glassworks)&&c.target!=10)||
           (c.good==int(EstablishmentIndustry::Glassworks)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::Glassworks&&site.region==10;}))||
           (c.good==int(EstablishmentIndustry::PotteryKiln)&&c.target!=15)||
           (c.good==int(EstablishmentIndustry::PotteryKiln)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::PotteryKiln&&site.region==15;}))||
           (c.good==int(EstablishmentIndustry::Tannery)&&c.target!=17)||
           (c.good==int(EstablishmentIndustry::Tannery)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::Tannery&&site.region==17;}))||
           (c.good==int(EstablishmentIndustry::MedicinalGarden)&&c.target!=20)||
           (c.good==int(EstablishmentIndustry::MedicinalGarden)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::MedicinalGarden&&site.region==20;}))||
           (c.good==int(EstablishmentIndustry::SeedPressWorks)&&c.target!=2)||
           (c.good==int(EstablishmentIndustry::SeedPressWorks)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::SeedPressWorks&&site.region==2;}))||
           (c.good==int(EstablishmentIndustry::SawmillWorks)&&(regions_!=int(kAtlasTerritoryCount)-1||c.target!=6))||
           (c.good==int(EstablishmentIndustry::SawmillWorks)&&std::any_of(establishments_.begin(),establishments_.end(),
               [&](const Establishment& site){return site.industry==EstablishmentIndustry::SawmillWorks&&site.region==6;}))||
           next_id_==UINT64_MAX||next_fact_==UINT64_MAX||establishments_.size()>=1000000)return false;
        if(c.term==-2)
            return p.family==Family::Governance&&p.taxing_authority&&provinces_[size_t(p.region)].charter&&
                   municipal_funds_[size_t(p.region)]>=c.amount;
        if(p.family==Family::Governance)
            return p.taxing_authority&&provinces_[p.region].charter&&provinces_[p.region].treasury>=c.amount;
        return p.cash>=c.amount;
    }
    if(c.kind==Intent::TransferEstablishmentGoods){
        if(c.target<=0||c.good<0||c.good>=int(kGoodIdCount)||c.amount<=0||
           (c.term!=int(EstablishmentTransferOperation::DepositGoods)&&
            c.term!=int(EstablishmentTransferOperation::WithdrawGoods))||next_fact_==UINT64_MAX)return false;
        const auto site=std::lower_bound(establishments_.begin(),establishments_.end(),uint64_t(c.target),
            [](const Establishment& value,uint64_t target){return value.id<target;});
        if(site==establishments_.end()||site->id!=uint64_t(c.target)||
           p.destination!=-1||p.region!=site->region||!manages_establishment(p,*site))return false;
        const size_t good=size_t(c.good);
        const bool deposit=c.term==int(EstablishmentTransferOperation::DepositGoods);
        int64_t source=0,destination=0;
        if(site->owner_kind==EstablishmentOwnerKind::Person){source=deposit?p.inventory[good]:site->stock[good];destination=deposit?site->stock[good]:p.inventory[good];}
        else if(site->owner_kind==EstablishmentOwnerKind::Public){source=deposit?provinces_[size_t(site->region)].stock[good]:site->stock[good];destination=deposit?site->stock[good]:provinces_[size_t(site->region)].stock[good];}
        else return false;
        if(source<c.amount||destination<0||destination>std::numeric_limits<int64_t>::max()-c.amount)return false;
        if(!deposit&&site->owner_kind==EstablishmentOwnerKind::Person){
            int64_t load=0;for(auto quantity:p.inventory){if(quantity<0||quantity>16-load)return false;load+=quantity;}
            if(c.amount>16-load)return false;
        }
        return true;
    }
    if(c.kind==Intent::AuthorizeWorkAtSite){
        constexpr int64_t authorization_days=31;
        const int64_t maxTick=365000LL*kSubdayUnitsPerDay;
        if(c.target<=0||c.good<=0||uint64_t(c.good)>people_.size()||c.term!=authorization_days||c.amount!=0||next_fact_==UINT64_MAX||
           schedule_book_.cursor>maxTick-authorization_days*kSubdayUnitsPerDay)return false;
        const auto site=std::lower_bound(establishments_.begin(),establishments_.end(),uint64_t(c.target),
            [](const Establishment& value,uint64_t target){return value.id<target;});
        const auto&worker=person(uint64_t(c.good));
        if(site==establishments_.end()||site->id!=uint64_t(c.target)||
           p.destination!=-1||p.region!=site->region||!manages_establishment(p,*site)||
           !worker.alive||worker.destination!=-1||worker.region!=site->region)return false;
        const int64_t begin=schedule_book_.cursor,end=begin+authorization_days*kSubdayUnitsPerDay;
        const auto previous=site->work_authorizations.find(uint64_t(c.good));
        if(previous!=site->work_authorizations.end()&&previous->second.size()==1&&
           previous->second.front()==WorkAuthorizationWindow{begin,end})return false;
        const auto sessions=schedule_book_.sessions.find(uint64_t(c.good));
        if(sessions!=schedule_book_.sessions.end())for(const auto& session:sessions->second){
            if(session.retired||session.kind!=ActivityKind::Work||session.establishment_id!=site->id||session.end<=schedule_book_.cursor)continue;
            if(begin>std::max(session.begin,schedule_book_.cursor)||end<session.end)return false;
        }
        return true;
    }
    if(c.kind==Intent::WorkAtSite){
        if(p.military_service.active)return false;
        const int64_t maxTick=365000LL*kSubdayUnitsPerDay;
        if(c.target<=0||c.term!=1||c.amount!=0||next_fact_==UINT64_MAX||
           schedule_book_.cursor>maxTick-kSubdayUnitsPerDay)return false;
        const auto site=std::lower_bound(establishments_.begin(),establishments_.end(),uint64_t(c.target),
            [](const Establishment& value,uint64_t target){return value.id<target;});
        if(site==establishments_.end()||site->id!=uint64_t(c.target)||
           p.destination!=-1||p.region!=site->region)return false;
        const int64_t begin=schedule_book_.cursor,end=begin+kSubdayUnitsPerDay;
        if(!manages_establishment(p,*site)){
            const auto authorization=site->work_authorizations.find(id);
            if(authorization==site->work_authorizations.end()||
               std::none_of(authorization->second.begin(),authorization->second.end(),[&](const WorkAuthorizationWindow& window){return window.begin<=begin&&end<=window.end;}))return false;
        }
        const auto sessions=schedule_book_.sessions.find(id);
        if(sessions!=schedule_book_.sessions.end())for(const auto& session:sessions->second)
            if(!session.retired&&session.begin<end&&begin<session.end)return false;
        return true;
    }
    if(c.kind==Intent::BuyMilitaryRations){
        if(p.family!=Family::Governance||!p.taxing_authority||p.region!=c.target||
           (c.good!=int(GoodId::DryFood)&&c.good!=int(GoodId::SupplementaryFood))||
           c.term!=0||c.amount<=0||c.amount>kMilitaryRationPurchaseMaximum||!provinces_[p.region].charter||
           provinces_[p.region].stock[size_t(c.good)]<c.amount)return false;
        const auto depot=c.good==int(GoodId::DryFood)?military_ration_depots_[size_t(p.region)]:
            military_supplementary_ration_depots_[size_t(p.region)];
        if(depot>std::numeric_limits<int64_t>::max()-c.amount)return false;
        const auto gross=MarketManager::purchase_total(provinces_[p.region].price[size_t(c.good)],c.amount);
        const auto tax=gross?MarketManager::assess_sales_tax(*gross,provinces_[p.region].tax,
            provinces_[p.region].sales_tax_remainder):std::nullopt;
        return gross&&tax&&provinces_[p.region].treasury>=*gross&&
            provinces_[p.region].firm<=std::numeric_limits<int64_t>::max()-*gross&&
            provinces_[p.region].tax_receipts<=std::numeric_limits<int64_t>::max()-tax->amount;
    }
    if(c.kind==Intent::ShipMilitaryRations){
        if(p.family!=Family::Governance||!p.taxing_authority||!provinces_[p.region].charter||
           regions_!=int(kAtlasTerritoryCount)-1||c.target<0||c.target>regions_||c.target==p.region||
           (c.good!=int(GoodId::DryFood)&&c.good!=int(GoodId::SupplementaryFood))||c.amount<=0||c.amount>64||
           c.term!=int(TransportServiceGoal::Fastest)||next_id_==UINT64_MAX||
           next_transport_lot_id_==UINT64_MAX||transport_shipments_.size()>=1000000||
           transport_cargo_lots_.size()>=1000000||transport_escrow_<0)return false;
        const auto depot=c.good==int(GoodId::DryFood)?military_ration_depots_[size_t(p.region)]:
            military_supplementary_ration_depots_[size_t(p.region)];
        if(c.amount>depot)return false;
        std::array<int64_t,kAtlasLinkCount> waiting_units_by_link{};
        if(!transport_waiting_units_by_link(waiting_units_by_link))return false;
        const auto plan=NationalTransportManager::plan(day_,uint8_t(p.region),uint8_t(c.target),c.amount,
            TransportServiceGoal::Fastest,national_transport_routes_,waiting_units_by_link);
        return plan.reachable&&!plan.route_indices.empty()&&provinces_[size_t(p.region)].treasury>=plan.total_fee&&
            plan.total_fee<=std::numeric_limits<int64_t>::max()-transport_escrow_;
    }
    if(c.kind==Intent::Offer){
        if(c.good==2){if(!p.sovereign||!pact_||regions_<3)return false;for(auto&o:proposals_)if(o.kind==2&&!o.resolved)return false;return true;}
        if(c.good==4){
            if(p.family!=Family::Governance||!p.taxing_authority||p.sovereign||c.target<=0||
               uint64_t(c.target)>people_.size()||c.amount<1||c.amount>4||c.term!=14||
               day_>365000-30||next_id_==UINT64_MAX||next_fact_==UINT64_MAX)return false;
            const auto& recipient=person(uint64_t(c.target));
            if(!recipient.alive||!recipient.sovereign||recipient.id==id)return false;
            for(const auto& obligation:military_obligations_)
                if(obligation.obligor==id&&obligation.completed_day<0)return false;
            for(const auto& proposal:proposals_)
                if(!proposal.resolved&&proposal.from==id&&proposal.kind==5)return false;
            const auto eligible=eligible_muster_members(id,p.region,day_+30);
            return size_t(c.amount)<=eligible.size();
        }
        if(c.good==5){
            if(p.family!=Family::Governance||!p.taxing_authority||p.sovereign||c.target<=0||
               uint64_t(c.target)>people_.size()||c.amount!=7||c.term!=7||day_>365000-8||
               next_id_==UINT64_MAX||next_fact_==UINT64_MAX)return false;
            const auto& recipient=person(uint64_t(c.target));
            if(!recipient.alive||!recipient.sovereign)return false;
            for(const auto& obligation:military_obligations_)
                if(obligation.obligor==id&&obligation.recipient==uint64_t(c.target)&&
                   obligation.completed_day<0&&obligation.due_checked&&
                   obligation.shortage_at_last_deadline>0&&obligation.extensions==0){
                    for(const auto& proposal:proposals_)
                        if(!proposal.resolved&&proposal.kind==6&&proposal.cause==obligation.id)return false;
                    return true;
                }
            return false;
        }
        if(c.good==6){
            if(p.family!=Family::Governance||!p.taxing_authority||p.sovereign||c.target<=0||
               uint64_t(c.target)>people_.size()||c.term!=14||day_>365000-14||
               next_id_==UINT64_MAX||next_fact_==UINT64_MAX)return false;
            const auto& recipient=person(uint64_t(c.target));
            if(!recipient.alive||!recipient.sovereign)return false;
            for(const auto& source:military_obligations_){
                if(source.obligor!=id||source.recipient!=uint64_t(c.target)||source.completed_day>=0||
                   c.amount!=source.required_members)continue;
                bool already=false;
                for(const auto& supply:military_supply_obligations_)
                    if(supply.personnel_obligation_id==source.id){already=true;break;}
                for(const auto& proposal:proposals_)
                    if(!proposal.resolved&&proposal.kind==7&&proposal.cause==source.id){already=true;break;}
                if(!already)return true;
            }
            return false;
        }
        if(c.good==7){
            if(p.family!=Family::Governance||!p.taxing_authority||p.sovereign||c.target<=0||
               uint64_t(c.target)>people_.size()||c.amount!=7||c.term!=7||day_>365000-8||
               next_id_==UINT64_MAX||next_fact_==UINT64_MAX)return false;
            const auto& recipient=person(uint64_t(c.target));
            if(!recipient.alive||!recipient.sovereign)return false;
            for(const auto& supply:military_supply_obligations_)
                if(supply.obligor==id&&supply.recipient==uint64_t(c.target)&&supply.completed_day<0&&
                   supply.due_checked&&supply.shortage_at_last_deadline>0&&supply.extensions==0){
                    for(const auto& proposal:proposals_)
                        if(!proposal.resolved&&proposal.kind==8&&proposal.cause==supply.id)return false;
                    return true;
                }
            return false;
        }
        if(c.good==3){
            if(p.family!=Family::Governance||c.amount!=kMilitaryServiceTrialWagePerDay||c.term<1||c.term>30||
               c.target<=0||uint64_t(c.target)>people_.size()||uint64_t(c.target)==id||
               !military_service_eligible(person(uint64_t(c.target)))||person(uint64_t(c.target)).region!=p.region||
               c.amount>std::numeric_limits<int64_t>::max()/c.term||
               provinces_[p.region].treasury<c.amount*c.term||day_>365000-40||next_id_==UINT64_MAX||next_fact_==UINT64_MAX)return false;
            for(const auto& proposal:proposals_)if(!proposal.resolved&&proposal.kind==4&&proposal.to==uint64_t(c.target))return false;
            return true;
        }
        if(p.family==Family::Civilian||c.target<=0||uint64_t(c.target)==id||uint64_t(c.target)>people_.size()||c.amount<=0)return false;
        auto source=p.family==Family::Governance?provinces_[p.region].treasury:p.cash;if(source<c.amount)return false;
        if(c.good==1&&(!p.sovereign||draft_.chosen!=2||day_<mediation_ready_))return false;
        for(auto&o:proposals_)if(!o.resolved&&o.from==id&&o.kind==0&&o.mediation==(c.good==1))return false;return true;
    }
    if(c.target>regions_){reason="Outside curriculum geography";return false;}
    auto&v=provinces_[c.target];auto&local=provinces_[p.region];
    auto funds=p.family==Family::Governance?local.treasury:p.cash;
    switch(c.kind){
    case Intent::Buy:{
        if((p.family==Family::Civilian&&p.age_days<6*365)||c.target!=p.region||!valid_good_id(c.good)||c.amount<=0||
           local.stock[size_t(c.good)]<c.amount)break;
        const auto cost=MarketManager::purchase_total(local.price[size_t(c.good)],c.amount);
        if(!cost||available_funds(id)<*cost)break;
        const auto tax=MarketManager::assess_sales_tax(*cost,local.tax,local.sales_tax_remainder);
        if(!tax||local.tax_receipts>std::numeric_limits<int64_t>::max()-tax->amount)break;
        int64_t load=0;for(auto n:p.inventory)load+=n;
        int64_t reservedSlots=0;for(const auto&order:market_orders_)if(order.owner==id&&order.priority==MarketPriority::Ordinary){
            if(order.quantity>std::numeric_limits<int64_t>::max()-reservedSlots)break;
            reservedSlots+=order.quantity;
        }
        if(load<=16&&reservedSlots<=16-load&&c.amount<=16-load-reservedSlots)return true;
        break;
    }
    case Intent::Sell:{
        if(c.target!=p.region||!valid_good_id(c.good)||c.amount<=0||p.inventory[size_t(c.good)]<c.amount)break;
        const auto proceeds=MarketManager::sale_proceeds(local.price[size_t(c.good)],c.amount);
        if(proceeds&&local.firm>=*proceeds)return true;
        break;
    }
    case Intent::Travel:case Intent::Migrate:{
        const int days=person_travel_days(p,c.target);
        if(c.target!=p.region&&days>0&&p.cash>=4+2*days&&p.age_days>=14*365){
            if(regions_==int(kAtlasTerritoryCount)-1&&p.residence_kind==ResidenceKind::Urban){
                if(c.target>regions_)break;
                const auto household=p.household_membership.household_id;
                const auto home=std::find_if(private_residences_.begin(),private_residences_.end(),[&](const auto& value){
                    return value.tenant_household==household&&value.region==c.target&&value.completed&&value.lease_end_day>day_;
                });
                int64_t householdAtDestination=0;
                for(const auto& member:people_)if(member.alive&&member.residence_kind==ResidenceKind::Urban&&
                    member.household_membership.household_id==household&&
                    (member.region==c.target||member.destination==c.target))++householdAtDestination;
                const bool privateBed=home!=private_residences_.end()&&householdAtDestination<kPrivateResidenceBeds;
                const bool publicBed=urban_housing_capacity_[size_t(c.target)]>
                    urban_residents_cache_[size_t(c.target)]+urban_arrivals_reserved_[size_t(c.target)];
                if(!privateBed&&!publicBed)break;
            }
            return true;
        }
        break;
    }
    case Intent::Borrow:if(c.good==1){if(p.sovereign&&draft_.chosen==4&&credit_pool_>=c.amount&&c.term==1825&&within_loan_limit(debt(id),c.amount,royal_reference_/2))return true;break;}if(p.family!=Family::Civilian&&c.target==p.region&&local.firm>=c.amount&&within_loan_limit(debt(id),c.amount,300)&&c.term>=30&&c.term<=180)return true;break;
    case Intent::Repay:if(c.amount>0&&debt(id)>=c.amount&&funds>=c.amount)return true;break;
    case Intent::Project:{
        if(c.good==4){
            const bool councilRepresentative=regions_==int(kAtlasTerritoryCount)-1&&
                std::any_of(municipal_councils_.begin(),municipal_councils_.end(),[&](const MunicipalCouncil& council){
                    return council.site_id==uint8_t(p.region)&&council.representative==id&&day_<council.term_end_day;
                });
            if(regions_!=int(kAtlasTerritoryCount)-1||
               !((p.family==Family::Governance&&p.taxing_authority)||councilRepresentative)||
               c.target!=p.region||!provinces_[size_t(p.region)].charter||
               c.amount!=kUrbanHousingProjectBudget||c.term!=kUrbanHousingProjectTermDays||
               day_>365000-kUrbanHousingProjectTermDays||
               municipal_funds_[size_t(p.region)]<c.amount||
               urban_residents_cache_[size_t(p.region)]<=urban_housing_capacity_[size_t(p.region)]||
               urban_housing_capacity_[size_t(p.region)]>1000000000LL-kUrbanHousingBedsPerBlock||
               consumed_[size_t(GoodId::ConstructionMaterial)]>std::numeric_limits<int64_t>::max()-4||
               consumed_[size_t(GoodId::Tools)]>std::numeric_limits<int64_t>::max()-2||
               !project_materials(p.region,c.good)||
               ProjectManager::active_projects_count(projects_,p.region,c.good)>0||
               next_id_==UINT64_MAX||next_fact_==UINT64_MAX)return false;
            return true;
        }
        int64_t match=p.sovereign&&c.good<2?std::min(restricted_fund_,c.amount/2):0;
        if(p.family==Family::Civilian||c.target!=p.region||funds<c.amount-match||c.amount<=0||c.term<30||c.term>180||c.good<0||c.good>3||!project_materials(p.region,c.good))break;
        if(c.good==0&&local.road>=4)break;if(c.good==1&&local.school>=std::max(8,int(people_.size())/(regions_+1)/4))break;
        if(c.good>=2&&(local.teachers==0||!p.sovereign))break;
        int active=ProjectManager::active_projects_count(projects_,p.region,c.good);
        if(active<(c.good==3&&draft_.chosen==10?2:1))return true;break;}
    case Intent::Hire:if(p.family==Family::Economic&&c.target==p.region&&p.cash>=c.amount&&c.amount>0)return true;break;
    case Intent::Tax:if(p.family==Family::Governance&&p.taxing_authority&&c.target==p.region&&c.amount>=0&&c.amount<=20&&local.charter)return true;break;
    case Intent::Aid:
        if(c.good==1){
            if(p.family==Family::Governance&&p.taxing_authority&&p.region==c.target&&
               local.charter&&c.amount>0&&municipal_funds_[size_t(p.region)]>=c.amount)return true;
            break;
        }
        if(c.good==0&&p.family==Family::Governance&&(c.target==p.region||p.sovereign)&&funds>=c.amount&&c.amount>0)return true;
        break;
    case Intent::Investigate:{
        int active=0;
        for(const auto& investigation:investigations_)active+=investigation.owner==id&&!investigation.completed;
        const size_t paper=size_t(GoodId::Paper);
        if(p.family==Family::Governance&&c.target==p.region&&funds>=c.amount&&c.amount>0&&
           p.inventory[paper]>=1&&consumed_[paper]<std::numeric_limits<int64_t>::max()&&
           active<(p.sovereign&&draft_.chosen==7?2:1))return true;
        break;
    }
    case Intent::Work:if(!p.military_service.active&&EmploymentManager::can_start_work(p,v,c.target,c.term,day_))return true;break;
    case Intent::Study:if(p.family==Family::Civilian&&p.age_days>=6*365&&p.age_days<65*365&&p.skill<100&&c.target==p.region&&v.school_used<v.school&&available_funds(id)>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Rest:{
        if(p.family!=Family::Civilian||c.target!=p.region||c.amount<0||p.cash<c.amount||c.term<=0||c.term>30||
           day_>365000-c.term||p.body_state.rest_until_tick>schedule_book_.cursor)break;
        const int64_t endTick=int64_t(day_+c.term)*kSubdayUnitsPerDay;
        const auto sessions=schedule_book_.sessions.find(id);
        if(sessions!=schedule_book_.sessions.end())for(const auto& session:sessions->second)
            if(!session.retired&&session.kind==ActivityKind::Work&&session.begin<endTick&&schedule_book_.cursor<session.end)
                return false;
        return true;
    }
    case Intent::Social:{
        if(p.family!=Family::Civilian||p.age_days<6*365||c.target!=p.region||
           p.cash<c.amount||c.term<=0||c.term>30)break;
        if(c.good==0)return true;
        const size_t tea=size_t(GoodId::Tea);
        if(c.good==int(GoodId::Tea)&&p.inventory[tea]>0&&
           consumed_[tea]<std::numeric_limits<int64_t>::max())return true;
        const size_t incense=size_t(GoodId::Incense);
        if(c.good==int(GoodId::Incense)&&p.inventory[incense]>0&&
           consumed_[incense]<std::numeric_limits<int64_t>::max())return true;
        const size_t silver=size_t(GoodId::Silver);
        if(c.good==int(GoodId::Silver)&&p.inventory[silver]>0&&
           consumed_[silver]<std::numeric_limits<int64_t>::max())return true;
        break;
    }
    case Intent::Join:if(p.family==Family::Civilian&&p.age_days>=6*365&&c.target==p.region&&c.good>=0&&c.good<4&&p.organization!=1+4*p.region+c.good&&p.cash>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Resign:if(p.military_service.active||EmploymentManager::can_resign(p,day_)||std::any_of(municipal_councils_.begin(),municipal_councils_.end(),[&](const MunicipalCouncil& council){return council.representative==id&&day_<council.term_end_day;}))return true;break;
    case Intent::Appoint:if(p.sovereign&&office_used(id)<office_capacity(id)&&funds>=EmploymentManager::kAppointmentFee){for(auto ref:referrals_){auto&a=person(ref);if(!a.alive||!a.candidate)continue;return EmploymentManager::can_appoint(a,provinces_,funds,p.id,day_);}}break;
    case Intent::NominateCouncil:{
        if(regions_!=int(kAtlasTerritoryCount)-1||p.residence_kind!=ResidenceKind::Urban||
           p.urban_site_id>=kAtlasTerritoryCount||p.age_days<18*365||c.target!=int(p.urban_site_id)||c.good<0||c.good>2)break;
        const auto& council=municipal_councils_[p.urban_site_id];
        if(c.good==0&&(council.representative==0||day_>=council.term_end_day)&&council.nominee==0)return true;
        if(c.good==1&&council.nominee==id&&(council.representative==0||day_>=council.term_end_day))return true;
        if(c.good==2&&council.representative==id&&day_<council.term_end_day)return true;
        break;
    }
    case Intent::ResolveCouncilNomination:{
        if(!p.sovereign||regions_!=int(kAtlasTerritoryCount)-1||c.target<0||c.target>=int(kAtlasTerritoryCount)||c.good<0||c.good>1)break;
        const auto& council=municipal_councils_[size_t(c.target)];
        if(council.nominee==0||(council.representative!=0&&day_<council.term_end_day))break;
        const auto& nominee=person(council.nominee);
        if(!nominee.alive||nominee.age_days<18*365||nominee.residence_kind!=ResidenceKind::Urban||nominee.urban_site_id!=uint8_t(c.target))break;
        if(c.good==1&&day_<=365000-4*365)return true;
        if(c.good==0)return true;
        break;
    }
    case Intent::Mobilize:if(p.family==Family::Governance&&c.target==p.region&&funds>=c.amount&&c.term>0&&c.term<=30&&day_<=365000-c.term-1)return true;break;
    case Intent::FundMunicipal:{
        if(p.family!=Family::Governance||!p.taxing_authority||c.target!=p.region||c.term!=0||!local.charter||
           municipal_last_funded_period_[size_t(p.region)]==day_/30||c.amount<=0)break;
        const bool knownPercentage=std::find(kMunicipalGrantPercentages.begin(),kMunicipalGrantPercentages.end(),c.good)!=kMunicipalGrantPercentages.end();
        if(knownPercentage&&c.amount==municipal_grant_amount(local.treasury,c.good))return true;
        break;
    }
    case Intent::ForeignTrade:{
        constexpr std::array<int64_t,3> quantities{1,4,8};
        const auto operation=static_cast<ForeignTradeOperation>(c.term);
        const int partner_id=foreign_trade_partner_id(operation);
        if(partner_id<0||partner_id>=kForeignTradePartnerCount)return false;
        const bool importing=foreign_trade_is_import(operation);
        const auto& partner=foreign_trade_partners_[size_t(partner_id)];
        if(partner.trade_suspended)return false;
        const auto completed_orders=importing?partner.completed_import_orders:partner.completed_export_orders;
        const auto completed_units=importing?partner.imported_units:partner.exported_units;
        const auto completed_gross=importing?partner.import_gross:partner.export_gross;
        const int64_t order_gross=(importing?24:28)*c.amount;
        if(completed_orders==std::numeric_limits<int64_t>::max()||completed_units<0||completed_gross<0||
           c.amount>std::numeric_limits<int64_t>::max()-completed_units||
           order_gross>std::numeric_limits<int64_t>::max()-completed_gross)return false;
        if(regions_!=int(kAtlasTerritoryCount)-1||p.family!=Family::Economic||p.region!=8||p.destination!=-1||
           c.target!=8||std::find(quantities.begin(),quantities.end(),c.amount)==quantities.end()||
           day_>=365000||next_id_==UINT64_MAX||c.term<0||c.term>=2*kForeignTradePartnerCount)return false;
        const auto routePlan=foreign_trade_route_plan(operation,c.amount);
        if(!routePlan.reachable||routePlan.route_indices.empty()){
            reason="The required coastal route is closed or unavailable";
            return false;
        }
        if(routePlan.total_days<1||routePlan.total_fee>std::numeric_limits<int64_t>::max()-2*c.amount)return false;
        if(std::any_of(foreign_trade_orders_.begin(),foreign_trade_orders_.end(),
            [&](const ForeignTradeOrder& order){return order.created_day==day_;}))return false;
        if(importing){
            if(c.good!=int(GoodId::DryFood)||partner.goods[size_t(GoodId::DryFood)]<c.amount)return false;
            const int64_t gross=24*c.amount,duty=gross/20,fee=2*c.amount+routePlan.total_fee;
            if(gross>std::numeric_limits<int64_t>::max()-duty||gross+duty>std::numeric_limits<int64_t>::max()-fee||
               p.cash<gross+duty+fee||partner.escrow>std::numeric_limits<int64_t>::max()-(gross+duty)||
               partner.money>std::numeric_limits<int64_t>::max()-gross||
               provinces_[8].treasury>std::numeric_limits<int64_t>::max()-2*c.amount)return false;
            for(size_t routeIndex:routePlan.route_indices){
                const auto& route=national_transport_routes_[routeIndex];
                const int64_t routeFee=route.fee_per_unit*c.amount;
                if(provinces_[size_t(route.fee_recipient_region)].treasury>std::numeric_limits<int64_t>::max()-routeFee)return false;
            }
        }else{
            if(c.good!=int(GoodId::Salt)||p.inventory[size_t(GoodId::Salt)]<c.amount||
               partner.salt_capacity<c.amount)return false;
            const int64_t gross=28*c.amount,duty=gross/20,fee=2*c.amount+routePlan.total_fee;
            if(p.cash<fee||partner.escrow>std::numeric_limits<int64_t>::max()-gross||partner.money<gross||
               provinces_[8].treasury>std::numeric_limits<int64_t>::max()-2*c.amount||
               partner.goods[size_t(GoodId::Salt)]>std::numeric_limits<int64_t>::max()-c.amount)return false;
            for(size_t routeIndex:routePlan.route_indices){
                const auto& route=national_transport_routes_[routeIndex];
                const int64_t routeFee=route.fee_per_unit*c.amount;
                if(provinces_[size_t(route.fee_recipient_region)].treasury>std::numeric_limits<int64_t>::max()-routeFee)return false;
            }
            (void)duty;
        }
        return true;
    }
    default:break;
    }
    reason="Resources, charter, age, capacity or location prevents action";return false;
}
bool Realm::marriage_eligible(uint64_t id)const{
    if(!id||id>people_.size())return false;
    const auto&candidate=person(id);
    if(!candidate.alive||candidate.age_days<18*365)return false;
    for(const auto&edge:edges_)if(edge.kind==RelationKind::Spouse){
        const auto other=edge.from==id?edge.to:(edge.to==id?edge.from:0);
        if(other&&other<=people_.size()&&person(other).alive)return false;
    }
    return true;
}
bool Realm::marriage_related(uint64_t first,uint64_t second)const{
    const auto sibling_it=siblings_by_person_.find(first);
    if(sibling_it!=siblings_by_person_.end()&&sibling_it->second.contains(second))return true;
    const auto has_ancestor=[&](uint64_t descendant,uint64_t possible_ancestor){
        std::vector<uint64_t> pending{descendant};
        std::unordered_set<uint64_t> visited;
        while(!pending.empty()){
            const auto person=pending.back();pending.pop_back();
            if(!visited.insert(person).second)continue;
            const auto it=parents_by_child_.find(person);
            if(it==parents_by_child_.end())continue;
            for(const auto parent:it->second){
                if(parent==possible_ancestor)return true;
                if(!visited.contains(parent))pending.push_back(parent);
            }
        }
        return false;
    };
    return has_ancestor(first,second)||has_ancestor(second,first);
}
bool Realm::marriage_pending(uint64_t first,uint64_t second,uint64_t except)const{
    for(const auto&proposal:proposals_){
        if(proposal.id==except||proposal.resolved||proposal.kind!=3)continue;
        if(proposal.from==first||proposal.from==second||proposal.to==first||proposal.to==second)return true;
    }
    return false;
}
std::array<float,realm_actions> Realm::mask(uint64_t id)const{
    const auto candidateSnapshot=candidates(id);
    return mask(id,candidateSnapshot);
}
std::array<float,realm_actions> Realm::mask(uint64_t id,const std::array<Candidate,realm_actions>&c)const{
    (void)person(id);
    std::array<float,realm_actions> m{};for(int i=0;i<realm_actions;++i){std::string why;m[i]=(i==0||c[i].kind!=Intent::Hold)&&legal(id,c[i],why)?1.f:0.f;}return m;
}
RealmResult Realm::execute(const RealmCommand&cmd){
    std::string why;if(cmd.actor==0||cmd.actor>people_.size())return {false,"Unknown actor"};auto&p=mutable_person(cmd.actor);
    if(!p.alive)return {false,"Actor deceased"};
    if(p.decisions==UINT64_MAX||p.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
    if(next_fact_==UINT64_MAX)return {false,"Fact counter overflow"};
    if(cmd.sequence!=p.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!legal(cmd.actor,cmd.action,why))return {false,why};
    auto c=cmd.action;
    if(c.kind==Intent::CreateEstablishment){
        const auto ownerKind=c.term==-2?EstablishmentOwnerKind::Municipal:
            p.family==Family::Governance?EstablishmentOwnerKind::Public:EstablishmentOwnerKind::Person;
        const uint64_t ownerId=ownerKind==EstablishmentOwnerKind::Person?p.id:uint64_t(c.target);
        return execute(EstablishmentCommand{p.id,cmd.sequence,ownerKind,ownerId,c.target,
            static_cast<EstablishmentIndustry>(c.good),-1,c.amount});
    }
    if(c.kind==Intent::FundMunicipal)
        return execute(MunicipalBudgetCommand{p.id,cmd.sequence,c.target,c.amount});
    if(c.kind==Intent::PrivateResidenceBuild)
        return execute(PrivateResidenceBuildCommand{p.id,cmd.sequence,uint64_t(c.target)});
    if(c.kind==Intent::PrivateHousing)
        return execute(PrivateHousingCommand{p.id,cmd.sequence,uint64_t(c.target),uint64_t(c.amount),
            static_cast<PrivateHousingOperation>(c.good),
            c.good==int(PrivateHousingOperation::Offer)?kPrivateResidenceRentPerPeriod:0});
    if(c.kind==Intent::ForeignTrade)
        return execute(ForeignTradeCommand{p.id,cmd.sequence,static_cast<ForeignTradeOperation>(c.term),c.good,c.amount});
    if(c.kind==Intent::BuyMilitaryRations)
        return execute(MilitaryRationPurchaseCommand{p.id,cmd.sequence,p.region,c.amount,c.good});
    if(c.kind==Intent::ShipMilitaryRations)
        return execute(TransportShipmentCommand{p.id,cmd.sequence,0,TransportShipmentOperation::Create,
            c.target,c.good,c.amount,static_cast<TransportServiceGoal>(c.term),true});
    if(c.kind==Intent::TransferEstablishmentGoods)
        return execute(EstablishmentTransferCommand{p.id,cmd.sequence,uint64_t(c.target),
            static_cast<EstablishmentTransferOperation>(c.term),c.good,c.amount});
    if(c.kind==Intent::AuthorizeWorkAtSite)
        return execute(WorkAuthorizationCommand{p.id,cmd.sequence,uint64_t(c.target),uint64_t(c.good),
            WorkAuthorizationOperation::Set,schedule_book_.cursor,
            schedule_book_.cursor+int64_t(c.term)*kSubdayUnitsPerDay});
    if(c.kind==Intent::WorkAtSite){
        const auto site=std::lower_bound(establishments_.begin(),establishments_.end(),uint64_t(c.target),
            [](const Establishment& value,uint64_t target){return value.id<target;});
        return execute(ScheduleCommand{p.id,cmd.sequence,ScheduleCommand::Operation::Submit,
            0,schedule_book_.cursor,schedule_book_.cursor+kSubdayUnitsPerDay,0,
            activity::kMaxDemandRate,ActivityKind::Work,site->region,site->id});
    }
    auto&v=provinces_[p.region];auto&funds=p.family==Family::Governance?v.treasury:p.cash;
    // Validation above covers every source/capacity before any mutation.
    bool clearsJob=!schedule_book_.timeline_contract||(c.kind==Intent::Travel||c.kind==Intent::Migrate||c.kind==Intent::Work||c.kind==Intent::Study||c.kind==Intent::Rest||c.kind==Intent::Social||c.kind==Intent::Join||c.kind==Intent::Mobilize);
    if(c.kind!=Intent::Hold&&clearsJob)EmploymentManager::clear_work(p,v);
    int mobilized_members=0;
    switch(c.kind){
    case Intent::Hold:break;
    case Intent::Buy:{
        const size_t g=size_t(c.good);
        const auto cost=MarketManager::purchase_total(v.price[g],c.amount);
        const auto tax=cost?MarketManager::assess_sales_tax(*cost,v.tax,v.sales_tax_remainder):std::nullopt;
        if(!cost||!tax||v.tax_receipts>std::numeric_limits<int64_t>::max()-tax->amount)
            throw std::logic_error("validated market purchase arithmetic changed before commit");
        spend(p,v.firm,*cost);
        sales_tax(v,*cost);
        v.stock[g]-=c.amount;p.inventory[g]+=c.amount;break;
    }
    case Intent::Sell:{
        const size_t g=size_t(c.good);
        const auto proceeds=MarketManager::sale_proceeds(v.price[g],c.amount);
        if(!proceeds)throw std::logic_error("validated market sale arithmetic changed before commit");
        transfer(v.firm,p.cash,*proceeds);v.stock[g]+=c.amount;p.inventory[g]-=c.amount;break;
    }
    case Intent::Travel:case Intent::Migrate:{
        const int days=person_travel_days(p,c.target);if(days<=0)return {false,"No open route"};
        for(auto& council:municipal_councils_){
            if(council.representative==p.id){council.representative=0;council.term_end_day=-1;}
            if(council.nominee==p.id){council.nominee=0;council.nomination_day=-1;}
        }
        transfer(p.cash,carrier_,4+2*days);p.destination=c.target;p.busy_until=day_+std::max(1,days+1-v.road/2);
        if(regions_==int(kAtlasTerritoryCount)-1&&p.residence_kind==ResidenceKind::Urban)refresh_demography();
        break;
    }
    case Intent::Borrow:{bool pool=c.good==1;const auto interest=loan_interest(c.amount,pool?15:4);transfer(pool?credit_pool_:v.firm,funds,c.amount);Loan loan{next_id_++,p.id,p.region,day_+c.term,c.amount,interest};loan.royal_pool=pool;loan.cause=next_fact_;loans_.push_back(loan);break;}
    case Intent::Repay:{auto left=c.amount;for(auto&l:loans_)if(l.debtor==p.id&&left>0){auto n=std::min(left,outstanding(l));transfer(funds,l.royal_pool?credit_pool_:provinces_[l.creditor].firm,n);l.paid+=n;left-=n;}break;}
    case Intent::Project:start_project(p.id,p.region,c.good,c.amount,c.term);break;
    case Intent::Hire:transfer(p.cash,v.firm,c.amount);v.jobs+=1;break;
    case Intent::Tax:v.tax=int(c.amount);break;
    case Intent::Aid:{
        int64_t& aid_funds=c.good==1?municipal_funds_[size_t(p.region)]:funds;
        std::vector<uint64_t> residents;
        for(auto&citizen:people_)if(citizen.alive&&citizen.family==Family::Civilian&&citizen.region==c.target)
            residents.push_back(citizen.id);
        if(residents.empty())transfer(aid_funds,provinces_[size_t(c.target)].firm,c.amount);
        else{
            const int64_t share=c.amount/int64_t(residents.size()),remainder=c.amount%int64_t(residents.size());
            for(auto resident:residents)transfer(aid_funds,mutable_person(resident).cash,share);
            const size_t start=size_t((day_+p.decisions)%int(residents.size()));
            for(int64_t i=0;i<remainder;++i)
                transfer(aid_funds,mutable_person(residents[(start+size_t(i))%residents.size()]).cash,1);
        }
        provinces_[size_t(c.target)].unrest=std::max(0,provinces_[size_t(c.target)].unrest-3);
        break;
    }
    case Intent::Investigate:{
        const size_t paper=size_t(GoodId::Paper);
        if(p.inventory[paper]<1||consumed_[paper]==std::numeric_limits<int64_t>::max())
            throw std::logic_error("validated investigation paper changed before commit");
        transfer(funds,v.firm,c.amount);
        --p.inventory[paper];
        ++consumed_[paper];
        int due=day_+std::max(1,60*(150-capability_value(p.id,LegacyCapability::Investigation))/100);
        investigations_.push_back({next_id_++,p.id,next_fact_,p.region,due});
        break;
    }
    case Intent::Offer:create_proposal(p.id,c);break;
    case Intent::Marry:create_proposal(p.id,c);break;
    case Intent::Accept:case Intent::Reject:resolve_proposal(p.id,c);break;
    case Intent::Work:EmploymentManager::start_work(p,provinces_,day_,p.region,c.term);break;
    case Intent::Study:spend(p,v.firm,c.amount);p.school_days=c.term;++v.school_used;p.busy_until=day_+c.term;break;
    case Intent::Rest:{
        const int64_t endTick=int64_t(day_+c.term)*kSubdayUnitsPerDay;
        transfer(p.cash,v.firm,c.amount);
        p.body_state.rest_begin_tick=schedule_book_.cursor;
        p.body_state.rest_until_tick=endTick;
        p.body_state.rest_region=p.region;
        p.body_state.rest_health_pending=true;
        p.busy_until=day_+c.term;
        break;
    }
    case Intent::Social:{
        transfer(p.cash,v.firm,c.amount);
        if(c.good==int(GoodId::Tea)){
            const size_t tea=size_t(GoodId::Tea);
            if(p.inventory[tea]<1||consumed_[tea]==std::numeric_limits<int64_t>::max())
                throw std::logic_error("validated social tea changed before commit");
            --p.inventory[tea];
            ++consumed_[tea];
            p.social=std::min(100,p.social+5);
        }else if(c.good==int(GoodId::Incense)){
            const size_t incense=size_t(GoodId::Incense);
            if(p.inventory[incense]<1||consumed_[incense]==std::numeric_limits<int64_t>::max())
                throw std::logic_error("validated social incense changed before commit");
            --p.inventory[incense];
            ++consumed_[incense];
            p.social=std::min(100,p.social+8);
        }else if(c.good==int(GoodId::Silver)){
            const size_t silver=size_t(GoodId::Silver);
            if(p.inventory[silver]<1||consumed_[silver]==std::numeric_limits<int64_t>::max())
                throw std::logic_error("validated social silver changed before commit");
            --p.inventory[silver];
            ++consumed_[silver];
            p.social=std::min(100,p.social+10);
        }else p.social=std::min(100,p.social+2);
        p.busy_until=day_+c.term;
        break;
    }
    case Intent::Join:p.organization=1+4*p.region+c.good;transfer(p.cash,organization_funds_[p.organization-1],c.amount);p.busy_until=day_+c.term;break;
    case Intent::Support:transfer(p.cash,mutable_person(c.target).cash,c.amount);break;
    case Intent::Appoint:for(auto ref:referrals_){auto&a=mutable_person(ref);if(!a.alive||!a.candidate)continue;EmploymentManager::appoint(a,provinces_,funds,p.id,day_);break;}break;
    case Intent::NominateCouncil:{
        auto& council=municipal_councils_[p.urban_site_id];
        if(c.good==0){council.nominee=p.id;council.nomination_day=day_;}
        else if(c.good==1){council.nominee=0;council.nomination_day=-1;}
        else {council.representative=0;council.term_end_day=-1;}
        break;
    }
    case Intent::ResolveCouncilNomination:{
        auto& council=municipal_councils_[size_t(c.target)];
        if(c.good==1){council.representative=council.nominee;council.term_end_day=day_+4*365;}
        council.nominee=0;council.nomination_day=-1;
        break;
    }
    case Intent::Resign:{
        if(p.military_service.active)end_military_service(p,115);
        else if(EmploymentManager::can_resign(p,day_))EmploymentManager::resign(p);
        for(auto& council:municipal_councils_)if(council.representative==p.id){council.representative=0;council.term_end_day=-1;}
        break;
    }
    case Intent::Mobilize:{
        transfer(funds,v.firm,c.amount);
        v.unrest=std::max(0,v.unrest-capability_value(p.id,LegacyCapability::Military)/10);
        const int until=day_+1+c.term;
        for(auto&member:people_){auto&service=member.military_service;
            if(!service.active||service.commander!=p.id||service.source_region!=p.region)continue;
            service.mobilized_until_day=std::max(service.mobilized_until_day,std::min(until,service.end_day));
            ++mobilized_members;
        }
        break;
    }
    case Intent::CreateEstablishment:break; // dispatched through EstablishmentCommand above
    case Intent::TransferEstablishmentGoods:case Intent::AuthorizeWorkAtSite:case Intent::WorkAtSite:break; // typed command dispatch above
    case Intent::FundMunicipal:break; // dispatched through MunicipalBudgetCommand above
    }
    p.last_decision_day=day_;++p.decisions;++p.sampling;
    event(p.id,p.region,c.kind==Intent::Aid&&c.good==1?232:int(c.kind),c.amount,uint64_t(c.target));
    if(c.kind==Intent::Mobilize&&mobilized_members>0)event(p.id,p.region,227,c.term,mobilized_members,false,next_fact_-1);
    return {true,"Committed",next_fact_-1};
}
RealmResult Realm::execute(const ScheduleCommand&cmd){
    if(cmd.actor==0||cmd.actor>people_.size())return {false,"Unknown actor"};
    auto&p=mutable_person(cmd.actor);
    if(p.decisions==UINT64_MAX||p.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
    if(cmd.sequence!=p.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!p.alive)return {false,"Actor deceased"};
    if(next_fact_==UINT64_MAX)return {false,"Fact counter overflow"};
    auto&book=schedule_book_;
    const int64_t maxTick=365000LL*kSubdayUnitsPerDay;
    if(cmd.operation==ScheduleCommand::Operation::Submit){
        if(cmd.kind!=ActivityKind::Generic&&cmd.kind!=ActivityKind::Work)return {false,"Invalid activity kind"};
        int canonicalRegion=cmd.region;
        if(cmd.kind==ActivityKind::Work){
            if(p.military_service.active)return {false,"Active military service prevents starting Work"};
            if(cmd.region<0||cmd.region>regions_)return {false,"Invalid work region"};
            if(cmd.establishment_id==0)return {false,"Work requires an establishment"};
            auto site=std::lower_bound(establishments_.begin(),establishments_.end(),cmd.establishment_id,
                [](const Establishment& e,uint64_t id){return e.id<id;});
            if(site==establishments_.end()||site->id!=cmd.establishment_id)return {false,"Unknown establishment"};
            if(site->region!=cmd.region||p.region!=site->region||p.destination!=-1)return {false,"Work establishment region mismatch"};
            canonicalRegion=site->region;
        }
        else if(cmd.region!=-1)return {false,"Invalid generic region"};
        else if(cmd.establishment_id!=0)return {false,"Generic activity cannot name an establishment"};
        if(cmd.begin<book.cursor)return {false,"Processed time"};
        if(cmd.begin>=cmd.end)return {false,"Invalid session range"};
        if(cmd.begin>maxTick||cmd.end>maxTick)return {false,"Session outside world horizon"};
        if(cmd.demand_rate<activity::kMinDemandRate||cmd.demand_rate>activity::kMaxDemandRate)return {false,"Invalid demand rate"};
        if(cmd.kind==ActivityKind::Work){
            const auto site=std::lower_bound(establishments_.begin(),establishments_.end(),cmd.establishment_id,
                [](const Establishment& e,uint64_t id){return e.id<id;});
            if(!manages_establishment(p,*site)){
                const auto authorization=site->work_authorizations.find(cmd.actor);
                if(authorization==site->work_authorizations.end()||
                   std::none_of(authorization->second.begin(),authorization->second.end(),[&](const WorkAuthorizationWindow& window){
                       return window.begin<=cmd.begin&&cmd.end<=window.end;
                   }))return {false,"Work requires owner authorization"};
            }
        }
        if(p.decisions==UINT64_MAX||book.next_session_id==UINT64_MAX||p.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
        uint64_t newId=book.next_session_id;
        std::vector<ActivitySession> staged;
        auto found=book.sessions.find(cmd.actor);
        if(found!=book.sessions.end())staged=found->second;
        if(cmd.kind==ActivityKind::Work){
            if(p.body_state.rest_until_tick>cmd.begin&&p.body_state.rest_begin_tick<cmd.end)
                return {false,"Work overlaps Rest"};
            for(const auto& existing:staged){
                if(existing.retired||existing.kind!=ActivityKind::Work||existing.establishment_id==cmd.establishment_id)continue;
                if(cmd.begin<existing.end&&existing.begin<cmd.end)return {false,"Overlapping work at different establishments"};
            }
        }
        ActivitySession session{newId,cmd.actor,cmd.begin,cmd.end,cmd.priority,cmd.demand_rate};
        session.kind=cmd.kind;
        session.region=canonicalRegion;
        session.establishment_id=cmd.establishment_id;
        staged.push_back(session);
        std::vector<activity::SessionView> views;
        for(auto const&s:staged)views.push_back({s.id,s.actor,s.begin,s.end,s.priority,s.demand_rate});
        auto probe=activity::evaluateActivity(cmd.actor,activity::Horizon{cmd.begin,cmd.end},views);
        if(!probe.ok){
            return {false,"Schedule arithmetic overflow"};
        }
        RealmResult result{true,"Committed",next_fact_,newId};
        if(facts_.size()==facts_.capacity())facts_.reserve(std::min<size_t>(8193,std::max<size_t>(8,facts_.capacity()*2)));
        if(found==book.sessions.end())book.sessions.emplace(cmd.actor,std::move(staged));else found->second.swap(staged);
        book.next_session_id=newId+1;
        book.timeline_contract=true;
        p.last_decision_day=day_;++p.decisions;++p.sampling;
        event(cmd.actor,p.region,200,0,0,false,0);
        return result;
    }
    auto it=book.sessions.find(cmd.actor);
    if(it==book.sessions.end())return {false,"Unknown session"};
    auto sit=std::find_if(it->second.begin(),it->second.end(),[&](const ActivitySession&s){return s.id==cmd.session;});
    if(sit==it->second.end())return {false,"Unknown session"};
    if(sit->retired||sit->begin<book.cursor)return {false,"Processed time"};
    if(cmd.operation==ScheduleCommand::Operation::Cancel){
        if(p.decisions==UINT64_MAX||p.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
        std::vector<ActivitySession> staged=it->second;
        staged.erase(std::remove_if(staged.begin(),staged.end(),[&](const ActivitySession&s){return s.id==cmd.session;}),staged.end());
        std::vector<activity::SessionView> views;
        for(auto const&s:staged)views.push_back({s.id,s.actor,s.begin,s.end,s.priority,s.demand_rate});
        if(!staged.empty()){auto probe=activity::evaluateActivity(cmd.actor,activity::Horizon{sit->begin,sit->end},views);if(!probe.ok)return {false,"Schedule arithmetic overflow"};}
        RealmResult result{true,"Committed",next_fact_,cmd.session};
        if(facts_.size()==facts_.capacity())facts_.reserve(std::min<size_t>(8193,std::max<size_t>(8,facts_.capacity()*2)));
        it->second.swap(staged);
        if(it->second.empty())book.sessions.erase(cmd.actor);
        p.last_decision_day=day_;++p.decisions;++p.sampling;
        event(cmd.actor,p.region,201,0,cmd.session,false,0);
        return result;
    }
    if(cmd.operation==ScheduleCommand::Operation::Reprioritize){
        if(p.decisions==UINT64_MAX||p.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
        std::vector<ActivitySession> staged=it->second;
        for(auto&s:staged)if(s.id==cmd.session)s.priority=cmd.priority;
        std::vector<activity::SessionView> views;
        for(auto const&s:staged)views.push_back({s.id,s.actor,s.begin,s.end,s.priority,s.demand_rate});
        auto probe=activity::evaluateActivity(cmd.actor,activity::Horizon{sit->begin,sit->end},views);
        if(!probe.ok)return {false,"Schedule arithmetic overflow"};
        RealmResult result{true,"Committed",next_fact_,cmd.session};
        if(facts_.size()==facts_.capacity())facts_.reserve(std::min<size_t>(8193,std::max<size_t>(8,facts_.capacity()*2)));
        it->second.swap(staged);
        p.last_decision_day=day_;++p.decisions;++p.sampling;
        event(cmd.actor,p.region,202,0,cmd.session,false,0);
        return result;
    }
    return {false,"Unsupported schedule operation"};
}
RealmResult Realm::execute(const RealmClockCommand&cmd){
    if(cmd.operation!=RealmClockCommand::Operation::Pause&&cmd.operation!=RealmClockCommand::Operation::Resume&&cmd.operation!=RealmClockCommand::Operation::StepOneDay)return {false,"Unknown clock operation"};
    if(cmd.expected_pause_revision.has_value()&&*cmd.expected_pause_revision!=pause_revision_)return {false,"Stale clock revision"};
    if(cmd.operation==RealmClockCommand::Operation::Pause){
        if(paused_)return {true,"Committed"};
        if(pause_revision_==UINT64_MAX)return {false,"Pause revision overflow"};
        paused_=true;++pause_revision_;return {true,"Committed"};
    }
    if(cmd.operation==RealmClockCommand::Operation::Resume){
        if(!paused_)return {true,"Committed"};
        if(pause_revision_==UINT64_MAX)return {false,"Pause revision overflow"};
        paused_=false;++pause_revision_;return {true,"Committed"};
    }
    const int64_t maxTick=365000LL*kSubdayUnitsPerDay;
    if(schedule_book_.cursor>maxTick||64>maxTick-schedule_book_.cursor)return {false,"ticks bounds"};
    try{
        advance_ticks_preserving_paused(64);
    }catch(const std::exception&e){
        return {false,e.what()};
    }
    return {true,"Committed"};
}
int Realm::distance(int from,int to)const{
    if(from<0||from>regions_||to<0||to>regions_)return -1;
    if(regions_==int(kAtlasTerritoryCount)-1){
        if(!atlas_distance_cache_valid_)rebuild_atlas_distance_cache();
        return atlas_distance_cache_[size_t(from)][size_t(to)];
    }
    if(regions_>6||from>6||to>6)return -1;
    std::array<int,7>d;d.fill(100);d[from]=0;
    for(int k=0;k<=regions_;++k)for(int a=0;a<=regions_;++a)for(int b=0;b<=regions_;++b)if(routes_[a][b])d[b]=std::min(d[b],d[a]+1);
    return d[to]>=100?-1:d[to];
}
int Realm::travel_days(int from,int to)const{
    if(from<0||from>regions_||to<0||to>regions_)return -1;
    if(regions_!=int(kAtlasTerritoryCount)-1)return distance(from,to);
    const auto plan=NationalTransportManager::plan(day_,static_cast<uint8_t>(from),static_cast<uint8_t>(to),1,
        TransportServiceGoal::Fastest,national_transport_routes_);
    if(!plan.reachable)return -1;
    // The transport planner includes the initial departure/dispatch day. A
    // migrating Person is already dispatched, so retain only travel time.
    return std::max(0,plan.total_days-1);
}
int Realm::person_travel_days(const Person& actor,int destination)const{
    const int days=travel_days(actor.region,destination);
    if(days<=0||regions_!=int(kAtlasTerritoryCount)-1||
       actor.inventory[size_t(GoodId::WarHorse)]<=0)return days;
    return std::max(1,days-1);
}
void Realm::rebuild_atlas_distance_cache()const{
    for(auto&row:atlas_distance_cache_)row.fill(-1);
    for(size_t source=0;source<kAtlasTerritoryCount;++source){
        auto&distances=atlas_distance_cache_[source];distances[source]=0;
        std::queue<uint8_t> frontier;frontier.push(uint8_t(source));
        while(!frontier.empty()){
            const uint8_t current=frontier.front();frontier.pop();
            for(const auto&route:national_transport_routes_){
                if(!route.open||route.from!=current||distances[route.to]>=0)continue;
                distances[route.to]=distances[current]+1;frontier.push(route.to);
            }
        }
    }
    atlas_distance_cache_valid_=true;
}
RealmResult Realm::route_command(uint64_t actor,int from,int to,bool open){
    if(!actor||actor>people_.size()||regions_>6||from<0||to<0||from>regions_||to>regions_||from>6||to>6||from==to)return {false,"Route bounds"};
    auto&p=person(actor);if(!p.alive||p.family!=Family::Governance||(!p.sovereign&&p.region!=from))return {false,"Route jurisdiction"};
    if((from+1)%(regions_+1)!=to&&(to+1)%(regions_+1)!=from)return {false,"No constructed route"};
    routes_[from][to]=routes_[to][from]=open;event(actor,from,107,open,to);return {true,"Route changed",next_fact_-1};
}
void Realm::prices(){for(int r=0;r<=regions_;++r)for(int g=0;g<kGoodIdCount;++g){auto&v=provinces_[r];const int reference=kGoodReferencePrice[size_t(g)];v.price[size_t(g)]=std::clamp(int(reference*(1.5-double(v.stock[size_t(g)])/800.0)),2,5*reference);}}
void Realm::sales_tax(Province&v,int64_t gross){
    const auto tax=MarketManager::assess_sales_tax(gross,v.tax,v.sales_tax_remainder);
    if(!tax||v.tax_receipts>std::numeric_limits<int64_t>::max()-tax->amount)
        throw std::logic_error("invalid sales tax ledger arithmetic");
    transfer(v.firm,v.treasury,tax->amount);
    v.sales_tax_remainder=tax->remainder;
    v.tax_receipts+=tax->amount;
}
void Realm::payroll(int region,bool public_employer,const std::vector<std::pair<uint64_t,int64_t>>&claims){
    FinanceManager::payroll(day_,region,public_employer,claims,people_,provinces_);
}
int64_t Realm::harvest_quantity(int region,int good,int residents,int weather,int productivity)const{
    if(weather<0)weather=reference_scenario_?100:75+int(process_noise(seed_^uint64_t(day_/90)*7411)%51)+int(process_noise(seed_^uint64_t(day_/90)*19937^uint64_t(region/2+1)*431)%11)-5;
    return (int64_t(residents)*15+30)*(good==region%7?2:1)*weather*productivity/10000;
}
void RealmDayPipeline::run(Realm& realm){
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    auto timed = [&realm](size_t index, auto&& phase) {
        const auto start = std::chrono::steady_clock::now();
        phase();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - start).count();
        if (elapsed > 0) realm.day_phase_profile_ns_[index] += static_cast<uint64_t>(elapsed);
    };
    timed(0, [&] { realm.day_begin(); });
    realm.process_transport_day();
    timed(1, [&] { realm.day_body(); });
    timed(2, [&] { realm.day_payroll(); });
    timed(3, [&] { realm.day_people(); });
    timed(4, [&] { realm.day_production(); });
    timed(5, [&] { realm.day_harvest(); });
    timed(6, [&] { realm.finish_projects(); });
    timed(7, [&] { realm.day_investigations(); });
    timed(8, [&] { realm.day_treasury(); });
    timed(9, [&] { realm.day_loans(); });
    timed(10, [&] { realm.day_proposals(); });
    timed(11, [&] { realm.day_births(); });
    timed(12, [&] { realm.day_end(); });
    realm.process_military_obligations_day();
    realm.process_military_supply_obligations_day();
    realm.process_foreign_trade_day();
    realm.process_foreign_trade_offer_expirations();
    timed(13, [&] { realm.day_settlement(); });
#else
    realm.day_begin();
    realm.process_transport_day();
    realm.day_body();
    realm.day_payroll();
    realm.day_people();
    realm.day_production();
    realm.day_harvest();
    realm.finish_projects();
    realm.day_investigations();
    realm.day_treasury();
    realm.day_loans();
    realm.day_proposals();
    realm.day_births();
    realm.day_end();
    realm.process_military_obligations_day();
    realm.process_military_supply_obligations_day();
    realm.process_foreign_trade_day();
    realm.process_foreign_trade_offer_expirations();
    realm.day_settlement();
#endif
}
void Realm::day_begin(){
    ++day_;
    rebuild_secret_known_counts();
    for(auto&v:provinces_)v.school_used=0;
    EmploymentManager::reset_day_counters(provinces_);
}
void Realm::day_body(){
    const int64_t dayEnd=schedule_book_.cursor;
    const int64_t dayBegin=dayEnd-kSubdayUnitsPerDay;
    if(dayBegin<0||dayEnd%kSubdayUnitsPerDay!=0)throw std::logic_error("body day boundary");
    std::map<uint64_t,int64_t> effortAttentionByActor;
    for(const auto&[key,attention]:schedule_book_.work_effort_attention_by_establishment){
        int64_t& total=effortAttentionByActor[key.first];
        if(attention<0||attention>std::numeric_limits<int64_t>::max()-total)
            throw std::overflow_error("body work effort attention total overflow");
        total+=attention;
    }
    for(auto& person:people_){
        auto& body=person.body_state;
        const auto effort=effortAttentionByActor.find(person.id);
        const int64_t totalEffort=effort==effortAttentionByActor.end()?0:effort->second;
        if(totalEffort<body.exertion_effort_attention_processed)
            throw std::logic_error("body work effort attention watermark moved backwards");
        if(!person.alive){
            body.exertion_effort_attention_processed=totalEffort;
            body.exertion_effort_attention_remainder=0;
            body.exertion_rest_tick_remainder=0;
            if(body.rest_health_pending&&body.rest_until_tick<=dayEnd){
                body.rest_begin_tick=0;body.rest_until_tick=0;body.rest_region=-1;body.rest_health_pending=false;
            }
            continue;
        }
        int64_t restTicks=0;
        if(body.rest_until_tick>dayBegin&&body.rest_begin_tick<dayEnd&&
           person.destination==-1&&person.region==body.rest_region){
            restTicks=std::max<int64_t>(0,std::min(dayEnd,body.rest_until_tick)-
                std::max(dayBegin,body.rest_begin_tick));
        }
        const int64_t newEffortAttention=totalEffort-body.exertion_effort_attention_processed;
        const int64_t newEffortPoints=newEffortAttention/kExertionAttentionUnitsPerPoint;
        const int64_t effortRemainder=int64_t(body.exertion_effort_attention_remainder)+
            newEffortAttention%kExertionAttentionUnitsPerPoint;
        const int64_t effortPoints=newEffortPoints+effortRemainder/kExertionAttentionUnitsPerPoint;
        if(body.exertion<100){
            const int64_t room=100-body.exertion;
            if(effortPoints>=room){body.exertion=100;body.exertion_effort_attention_remainder=0;}
            else{
                body.exertion+=int(effortPoints);
                body.exertion_effort_attention_remainder=int(effortRemainder%kExertionAttentionUnitsPerPoint);
            }
        }else body.exertion_effort_attention_remainder=0;
        body.exertion_effort_attention_processed=totalEffort;
        const int64_t restTotal=restTicks+body.exertion_rest_tick_remainder;
        const int64_t recoveryPoints=restTotal/kExertionRestTicksPerPoint;
        if(body.exertion==0){
            body.exertion_rest_tick_remainder=0;
        }else if(recoveryPoints>=body.exertion){
            body.exertion=0;body.exertion_rest_tick_remainder=0;
        }else{
            body.exertion-=int(recoveryPoints);
            body.exertion_rest_tick_remainder=int(restTotal%kExertionRestTicksPerPoint);
        }
        const int64_t balance=body.sleep_balance_remainder+
            (kSubdayUnitsPerDay-restTicks)-restTicks;
        const int64_t delta=balance/kSubdayUnitsPerDay;
        const int64_t remainder=balance-delta*kSubdayUnitsPerDay;
        const int nextDebt=std::clamp<int64_t>(int64_t(body.sleep_debt)+delta,0,100);
        body.sleep_debt=nextDebt;
        body.sleep_balance_remainder=(nextDebt==100&&delta>0)||(nextDebt==0&&delta<0)?0:int(remainder);
        if(body.rest_health_pending&&body.rest_until_tick<=dayEnd){
            if(person.region==body.rest_region&&person.nutrition>=40){
                const int old=person.health;
                const int64_t completedRestTicks=body.rest_until_tick-body.rest_begin_tick;
                const int64_t recovery=completedRestTicks/kExertionRestTicksPerPoint;
                person.health=static_cast<int>(std::min<int64_t>(100,int64_t(person.health)+recovery));
                provinces_[size_t(person.region)].health_sum+=person.health-old;
            }
            body.rest_begin_tick=0;body.rest_until_tick=0;body.rest_region=-1;body.rest_health_pending=false;
        }
    }
}
void Realm::day_production(){
    if(schedule_book_.timeline_contract)produce_with_establishments();
    // Timeline worlds source craft output only through real Establishment work.
    // Keep the monthly province pilot for legacy worlds without a timeline.
    if(!schedule_book_.timeline_contract&&day_%30==0){
        for(int r=0;r<=regions_;++r){auto&v=provinces_[r];int workers=0;for(auto&p:people_)if(p.alive&&p.region==r&&p.work_assignment.job>=0&&p.work_assignment.job_kind==0)++workers;
        for(int legacyOutput:{2,5}){
            const size_t output=size_t(canonical_from_legacy_good(legacyOutput));const size_t input=size_t(canonical_from_legacy_good(legacyOutput==2?1:3));const size_t fuel=size_t(canonical_from_legacy_good(4));int n=int(std::min<int64_t>({int64_t(2+workers/4),v.stock[input],v.stock[fuel]}));v.stock[input]-=n;v.stock[fuel]-=n;consumed_[input]+=n;consumed_[fuel]+=n;v.stock[output]+=n;produced_[output]+=n;}
        if(v.stock[0]==0){v.unrest=std::min(100,v.unrest+5);event(1+r,r,102,workers,0,true);}else v.unrest=std::max(0,v.unrest-1);
    }}
}
// Staged daily establishment production. The planner owns no live state; the
// plan is revalidated against the exact read set, then committed as one
// transaction of scalar writes and fully built map/vector swaps. The phase
// runs inside the advancing day pipeline, so this slice promises no rollback
// of earlier day phases; it must not write RNG or facts.
void Realm::produce_with_establishments(){
    const ProductionManager::Input input{establishments_,schedule_book_.work_present_ticks_by_establishment,
        schedule_book_.work_effort_attention_by_establishment,
        schedule_book_.work_productivity_attention_numerator_by_establishment,people_,day_,config_};
    const auto plan=ProductionManager::plan(input);
    if(plan.day()!=day_)throw std::logic_error("production plan day mismatch");
    size_t index=0;
    uint64_t previous_id=0;
    int64_t produced_delta[kGoodIdCount]{},consumed_delta[kGoodIdCount]{};
    for(size_t entry=0;entry<plan.site_count();++entry){
        const auto&delta=plan.site(entry);
        if(delta.id<=previous_id)throw std::logic_error("production plan order");
        previous_id=delta.id;
        while(index<establishments_.size()&&establishments_[index].id<delta.id)++index;
        if(index>=establishments_.size())throw std::logic_error("production plan unknown site");
        if(establishments_[index].id!=delta.id)throw std::logic_error("production plan unknown site");
        const Establishment&site=establishments_[index];
        if(site.last_production_day>=delta.last_production_day||delta.last_production_day!=day_)
            throw std::logic_error("production repeat day");
        const ProductionRecipe recipe=production_recipe(site.industry,config_,site.source_good);
        const int64_t batches=delta.batches;
        if(batches<0||batches>site.capacity||batches>kEstablishmentCapacityMax)
            throw std::logic_error("production capacity mismatch");
        for(size_t g=0;g<kGoodIdCount;++g){
            bool is_recipe_good=g==recipe.output_good;
            for(size_t r=0;r<recipe.input_count;++r)
                is_recipe_good=is_recipe_good||g==recipe.inputs[r].good;
            if(is_recipe_good)continue;
            if(delta.stock_delta[g]!=0)throw std::logic_error("production off-recipe delta");
        }
        if(batches>std::numeric_limits<int64_t>::max()/recipe.output_amount)
            throw std::overflow_error("production output bound overflow");
        const int64_t output_quantity=batches*recipe.output_amount;
        if(delta.stock_delta[recipe.output_good]!=output_quantity)
            throw std::logic_error("production output delta mismatch");
        for(size_t r=0;r<recipe.input_count;++r){
            const size_t good=recipe.inputs[r].good;
            if(batches>std::numeric_limits<int64_t>::max()/recipe.inputs[r].amount)
                throw std::overflow_error("production input bound overflow");
            if(delta.stock_delta[good]!=-batches*recipe.inputs[r].amount)
                throw std::logic_error("production input delta mismatch");
        }
        if(recipe.source_units_per_batch<0||
           batches>std::numeric_limits<int64_t>::max()/std::max<int64_t>(1,recipe.source_units_per_batch))
            throw std::overflow_error("production source resource input bound overflow");
        const int64_t expectedSourceDelta=-batches*recipe.source_units_per_batch;
        if((recipe.source_units_per_batch>0&&recipe.source_good!=site.source_good)||
           delta.source_units_delta!=expectedSourceDelta||
           (delta.source_units_delta<0&&site.source_units< -delta.source_units_delta)||
           (delta.source_units_delta>0&&site.source_units>kEstablishmentSourceUnitsMaximum-delta.source_units_delta))
            throw std::logic_error("production source resource delta mismatch");
        for(size_t g=0;g<kGoodIdCount;++g){
            const int64_t take=delta.stock_delta[g];
            if(take==0)continue;
            if(take<0){
                if(take==std::numeric_limits<int64_t>::min()||site.stock[g]<-take)
                    throw std::logic_error("production input shortage");
            }else if(site.stock[g]>std::numeric_limits<int64_t>::max()-take)
                throw std::overflow_error("production stock overflow");
        }
        if(delta.watermark_final.size()>people_.size())throw std::logic_error("production watermark count");
        uint64_t previous_actor=0;
        for(const auto&[actor,units]:delta.watermark_final){
            if(actor==0||actor>people_.size())throw std::logic_error("production watermark actor");
            if(units<=0)throw std::logic_error("production watermark units");
            if(actor<=previous_actor)throw std::logic_error("production watermark order");
            previous_actor=actor;
            const auto ticks=schedule_book_.work_present_ticks_by_establishment.find({actor,delta.id});
            const auto productivity=schedule_book_.work_productivity_attention_numerator_by_establishment.find({actor,delta.id});
            const int64_t attended=ticks==schedule_book_.work_present_ticks_by_establishment.end()?0:ticks->second;
            const int64_t numerator=productivity==schedule_book_.work_productivity_attention_numerator_by_establishment.end()?0:
                productivity->second;
            const int64_t eligible=numerator/(kLaborProductivityDenominator*kAttentionUnitsPerEffortTick*config_.ticks_per_unit());
            const auto prior=site.processed_work_units.find(actor);
            const int64_t before=prior==site.processed_work_units.end()?0:prior->second;
            if(units<before||units>eligible)throw std::logic_error("production watermark mismatch");
        }
        // Ledger deltas: the aggregate plan arrays must exactly equal the sum
        // of per-entry recipe outputs/inputs committed here.
        if(batches>0){
            size_t good=recipe.output_good;
            if(produced_delta[good]>std::numeric_limits<int64_t>::max()-output_quantity)
                throw std::overflow_error("production produced ledger overflow");
            produced_delta[good]+=output_quantity;
            for(size_t r=0;r<recipe.input_count;++r){
                const size_t input_good=recipe.inputs[r].good;
                const int64_t take=batches*recipe.inputs[r].amount;
                if(consumed_delta[input_good]>std::numeric_limits<int64_t>::max()-take)
                    throw std::overflow_error("production consumed ledger overflow");
                consumed_delta[input_good]+=take;
            }
        }
    }
    for(size_t g=0;g<kGoodIdCount;++g){
        if(plan.produced()[g]!=produced_delta[g]||plan.consumed()[g]!=consumed_delta[g])
            throw std::logic_error("production ledger mismatch");
        if(produced_delta[g]>0&&produced_[g]>std::numeric_limits<int64_t>::max()-produced_delta[g])
            throw std::overflow_error("production ledger overflow");
        if(consumed_delta[g]>0&&consumed_[g]>std::numeric_limits<int64_t>::max()-consumed_delta[g])
            throw std::overflow_error("production ledger overflow");
    }
    // Stage the complete next establishment vector with every allocation
    // already succeeded, then publish by move; no live field is touched before.
    std::vector<Establishment> staged=establishments_;
    size_t index2=0;
    for(size_t entry=0;entry<plan.site_count();++entry){
        const auto&delta=plan.site(entry);
        while(index2<staged.size()&&staged[index2].id<delta.id)++index2;
        Establishment&site=staged[index2];
        site.source_units+=delta.source_units_delta;
        for(size_t g=0;g<kGoodIdCount;++g){
            if(delta.stock_delta[g]==0)continue;
            site.stock[g]+=delta.stock_delta[g];
        }
        site.last_production_day=delta.last_production_day;
        site.processed_work_units=delta.watermark_final;
    }
    for(size_t g=0;g<kGoodIdCount;++g){
        produced_[g]+=produced_delta[g];
        consumed_[g]+=consumed_delta[g];
    }
    establishments_=std::move(staged);
}
void Realm::day_harvest(){
    if(day_%90==0){for(int r=0;r<=regions_;++r){auto&v=provinces_[r];int residents=0;for(auto&p:people_)if(p.alive&&p.region==r)++residents;
        // Correlated regional season; labor and geography affect harvest.
        for(int legacy:{0,1,3,4,6}){const size_t g=size_t(canonical_from_legacy_good(legacy));auto n=harvest_quantity(r,legacy,residents);v.stock[g]+=n;produced_[g]+=n;}
        for(auto& site:establishments_)if(site.industry==EstablishmentIndustry::Forestry&&site.region==r&&
            site.source_good==int(GoodId::Timber)&&site.source_units<kEstablishmentSourceUnitsMaximum)++site.source_units;
        for(auto& site:establishments_)if(site.industry==EstablishmentIndustry::TeaGarden&&r==14&&
            site.source_good==int(GoodId::Tea))site.source_units=std::min<int64_t>(kEstablishmentSourceUnitsMaximum,site.source_units+25);
        for(auto& site:establishments_)if(site.industry==EstablishmentIndustry::SeedPressWorks&&r==2&&
            site.source_good==int(GoodId::SupplementaryFood))site.source_units=std::min<int64_t>(
                kEstablishmentSourceUnitsMaximum,site.source_units+25);
    }
    }
}
void Realm::day_investigations(){
    for(auto&i:investigations_)if(!i.completed&&i.due<=day_){i.completed=true;for(auto&f:facts_)if(f.region==i.region&&f.secret&&f.day<=i.due){auto&known=knowledge_index_[i.owner];auto prior=known.find(f.id);const bool newly_known=prior==known.end()||prior->second>day_;knowledge_.push_back({i.owner,f.id,day_,i.id,KnowledgeKind::SecretEvidence,kUnknownKnowledgeConfidence});known[f.id]=day_;if(i.owner&&i.owner<=people_.size())++knowledge_observation_counts_[i.owner-1][0];if(newly_known&&i.owner&&i.owner<=people_.size()&&i.owner!=f.actor&&i.owner!=f.target)++secret_known_counts_[i.owner-1];}event(i.owner,i.region,109,0,i.id,false,i.cause);}
}
void Realm::day_treasury(){
    if(day_%30==0){auto&v=provinces_[person(1).region];event(1,person(1).region,108,v.stock[0]);if(draft_.chosen==3){int64_t spoiled=std::max<int64_t>(0,v.stock[0]-v.food_capacity);spoiled+=std::min(v.stock[0]-spoiled,v.stock[0]/1000);v.stock[0]-=spoiled;consumed_[0]+=spoiled;if(v.treasury>0)transfer(v.treasury,v.firm,1);}}
}
void Realm::day_loans(){
    for(auto&l:loans_)if(!l.defaulted&&l.due<=day_&&l.paid<l.principal+l.interest){l.defaulted=true;event(l.debtor,person(l.debtor).region,103,l.principal+l.interest-l.paid,l.id);}
}
void Realm::day_proposals(){
    for(auto&o:proposals_)if(!o.resolved&&o.due<day_){refund_proposal(o);event(o.from,person(o.from).region,104,o.amount,o.to);}
}
void Realm::day_births(){
    // Births conserve estate money and preserve independent IDs/relationships.
    if(!reference_scenario_&&day_%365==0&&people_.size()<1000000){size_t n=people_.size();for(size_t i=0;i<n;++i){auto&parent=people_[i];if(!parent.alive||parent.health<60||parent.nutrition<60||parent.age_days<20*365||parent.age_days>45*365||parent.cash<50||rng_()%100>=6)continue;
        Person child;child.id=people_.size()+1;child.personality=seeded_personality(seed_,child.id);child.region=parent.region;child.residence_kind=parent.residence_kind;child.urban_site_id=parent.urban_site_id;child.household_membership.household_id=parent.household_membership.household_id;child.age_days=0;child.cash=0;child.goal=int(rng_()%3);transfer(parent.cash,child.cash,20);people_.push_back(child);personal_establishment_indices_.emplace_back();secret_known_counts_.push_back(0);knowledge_observation_counts_.push_back({});index_household(child.household_membership.household_id,child.id);edges_.push_back({child.id,uint64_t(i+1),RelationKind::ParentChild,true});index_kinship_edge(edges_.back());event(child.id,child.region,105,0,uint64_t(i+1));}}
}
void Realm::day_end(){
    refresh_demography();
    if(config_.market_enabled()){
        if(day_%30==0&&!update_monthly_market_prices())
            throw std::overflow_error("monthly market price calculation failed");
    }else{
        prices();
        market_processed_month_=day_/30;
    }
    market_settled_day_=day_;
}
void Realm::day_settlement(){
    // Settlement is pure schedule bookkeeping. It must not touch attendance,
    // people, provinces, cash, RNG, facts, production, payroll or sessions.
    if(!schedule_book_.timeline_contract)return;
    const int64_t completed=schedule_book_.cursor/kSubdayUnitsPerDay;
    if(completed<=0)return;
    for(auto const&[actor,ticks]:schedule_book_.work_present_ticks){
        if(actor==0||actor>people_.size())continue;
        const int64_t eligible=ticks/config_.ticks_per_unit();
        auto prior=schedule_book_.settled_work_units.find(actor);
        const int64_t settled=prior==schedule_book_.settled_work_units.end()?0:prior->second;
        if(eligible>settled)schedule_book_.settled_work_units[actor]=eligible;
    }
    schedule_book_.settled_day=completed;
}
void Realm::refresh_demography(){
    for(auto&v:provinces_){v.residents=0;v.health_sum=0;}
    urban_residents_cache_.assign(size_t(regions_+1),0);
    urban_arrivals_reserved_.assign(size_t(regions_+1),0);
    for(auto&p:people_){
        auto&v=provinces_[p.region];++v.residents;v.health_sum+=p.alive?p.health:0;
        if(regions_==int(kAtlasTerritoryCount)-1&&p.alive&&p.residence_kind==ResidenceKind::Urban){
            ++urban_residents_cache_[size_t(p.region)];
            if(p.destination>=0&&p.destination!=p.region)
                ++urban_arrivals_reserved_[size_t(p.destination)];
        }
    }
}
void Realm::initialize_urban_housing_baseline(){
    urban_housing_capacity_.assign(size_t(regions_+1),0);
    if(regions_!=int(kAtlasTerritoryCount)-1)return;
    // On legacy-save migration, residence has just been restored but the
    // temporary World's caches still describe its constructor population.
    refresh_demography();
    for(int region=0;region<=regions_;++region){
        const auto residents=urban_residents_cache_[size_t(region)];
        if(residents>std::numeric_limits<int64_t>::max()-(kUrbanHousingBedsPerBlock-1))
            throw std::overflow_error("Opening urban housing capacity overflow");
        urban_housing_capacity_[size_t(region)]=
            ((residents+kUrbanHousingBedsPerBlock-1)/kUrbanHousingBedsPerBlock)*kUrbanHousingBedsPerBlock;
    }
}
void Realm::advance(int days){if(days<0||days>36500)throw std::invalid_argument("days bounds");advance_ticks(static_cast<int64_t>(days)*kSubdayUnitsPerDay);}
void Realm::advance_ticks(int64_t ticks){
    if(ticks<0)throw std::invalid_argument("ticks bounds");
    if(ticks>0 && paused_)throw std::runtime_error("Realm is paused");
    advance_ticks_preserving_paused(ticks);
}
void Realm::advance_ticks_preserving_paused(int64_t ticks){
    if(ticks<0)throw std::invalid_argument("ticks bounds");
    const int64_t maxTick=365000LL*kSubdayUnitsPerDay;
    if(schedule_book_.cursor>maxTick||ticks>maxTick-schedule_book_.cursor)throw std::invalid_argument("ticks bounds");
    int64_t target=schedule_book_.cursor+ticks;
    while(schedule_book_.cursor<target){
        int64_t nextBoundary=((schedule_book_.cursor/kSubdayUnitsPerDay)+1)*kSubdayUnitsPerDay;
        int64_t chunk=std::min(target,nextBoundary)-schedule_book_.cursor;
        if(chunk<=0)throw std::runtime_error("Schedule cursor stuck");
        // Stage the whole sparse schedule book before any commit.
        ScheduleBook staged=schedule_book_;
        // retire dead owners in staged book without deleting evidence.
        for(auto&[actor,sessions]:staged.sessions){
            if(actor==0||actor>people_.size()||!person(actor).alive){
                for(auto&s:sessions)if(!s.retired){s.retired=true;s.retired_tick=staged.cursor;s.retired_cause=100;}
            }
        }
        std::map<WorkEstablishmentKey,int64_t> deliveredEffortByActorSite;
        // integrate attention for each live actor into staged sessions.
        for(auto&[actor,sessions]:staged.sessions){
            if(actor==0||actor>people_.size()||!person(actor).alive)continue;
            const Person& owner=person(actor);
            std::vector<activity::SessionView> views;
            for(auto const&s:sessions)if(!s.retired&&s.begin<staged.cursor+chunk&&s.end>staged.cursor)views.push_back({s.id,s.actor,s.begin,s.end,s.priority,s.demand_rate});
            if(views.empty())continue;
            auto result=activity::evaluateActivity(actor,activity::Horizon{staged.cursor,staged.cursor+chunk},views);
            if(!result.ok)throw std::runtime_error("Schedule attention overflow");
            for(auto const&r:result.results){
                for(auto&s:sessions)if(s.id==r.id){
                    s.requested+=r.requested;s.delivered+=r.delivered;s.unmet+=r.unmet;
                    s.capacity_unmet+=r.capacity_unmet;s.unresolved_unmet+=r.unresolved_unmet;
                    if(staged.timeline_contract&&s.kind==ActivityKind::Work&&s.establishment_id!=0&&
                       owner.destination==-1&&owner.region==s.region&&r.delivered>0){
                        auto& attention=deliveredEffortByActorSite[{actor,s.establishment_id}];
                        if(r.delivered>std::numeric_limits<int64_t>::max()-attention)
                            throw std::overflow_error("work effort attention overflow");
                        attention+=r.delivered;
                    }
                }
            }
        }
        // credit cumulative unique physical Work presence in the staged book.
        for(auto&[actor,sessions]:staged.sessions){
            if(actor==0||actor>people_.size())continue;
            const Person&owner=person(actor);
            if(!owner.alive||owner.destination!=-1)continue;
            std::vector<std::pair<int64_t,int64_t>> work;
            std::map<uint64_t,std::vector<std::pair<int64_t,int64_t>>> byEstablishment;
            for(auto const&s:sessions){
                if(s.retired||s.kind!=ActivityKind::Work||s.region!=owner.region)continue;
                int64_t lo=std::max(s.begin,staged.cursor);
                int64_t hi=std::min(s.end,staged.cursor+chunk);
                if(lo<hi){work.emplace_back(lo,hi);if(s.establishment_id!=0)byEstablishment[s.establishment_id].emplace_back(lo,hi);}
            }
            int64_t credited=union_ticks(work);
            if(credited>0){
                int64_t&global=staged.work_present_ticks[actor];
                if(credited>std::numeric_limits<int64_t>::max()-global)
                    throw std::overflow_error("work attendance overflow");
                global+=credited;
                if(staged.timeline_contract){
                    int64_t&regional=staged.work_present_ticks_by_region[{actor,owner.region}];
                    if(credited>std::numeric_limits<int64_t>::max()-regional)
                        throw std::overflow_error("regional work attendance overflow");
                    regional+=credited;
                    int64_t siteTotal=0;
                    for(auto&[establishment,intervals]:byEstablishment){
                        const int64_t siteTicks=union_ticks(intervals);
                        if(siteTicks<=0)continue;
                        int64_t&site=staged.work_present_ticks_by_establishment[{actor,establishment}];
                        if(siteTicks>std::numeric_limits<int64_t>::max()-site)
                            throw std::overflow_error("establishment work attendance overflow");
                        site+=siteTicks;
                        if(siteTicks>std::numeric_limits<int64_t>::max()-siteTotal)
                            throw std::overflow_error("establishment attendance total overflow");
                        siteTotal+=siteTicks;
                        const auto delivered=deliveredEffortByActorSite.find({actor,establishment});
                        if(delivered!=deliveredEffortByActorSite.end()&&delivered->second>0){
                            if(siteTicks>std::numeric_limits<int64_t>::max()/kAttentionUnitsPerEffortTick||
                               delivered->second>siteTicks*kAttentionUnitsPerEffortTick)
                                throw std::logic_error("work effort attention exceeds presence");
                            int64_t& effort=staged.work_effort_attention_by_establishment[{actor,establishment}];
                            if(delivered->second>std::numeric_limits<int64_t>::max()-effort)
                                throw std::overflow_error("work effort attention total overflow");
                            effort+=delivered->second;
                            const int64_t skill_factor=150+owner.skill;
                            if(delivered->second>std::numeric_limits<int64_t>::max()/skill_factor)
                                throw std::overflow_error("work productivity attention overflow");
                            const int64_t weighted=delivered->second*skill_factor;
                            int64_t& productivity=staged.work_productivity_attention_numerator_by_establishment[{actor,establishment}];
                            if(weighted>std::numeric_limits<int64_t>::max()-productivity)
                                throw std::overflow_error("work productivity attention total overflow");
                            productivity+=weighted;
                        }
                    }
                    if(siteTotal>credited)throw std::logic_error("establishment attendance exceeds physical presence");
                }
            }
        }
        // commit staged book and cursor.
        schedule_book_=std::move(staged);
        schedule_book_.cursor+=chunk;
        if(schedule_book_.cursor%kSubdayUnitsPerDay==0)RealmDayPipeline::run(*this);
    }
}
std::array<int64_t,11> Realm::balances()const{std::array<int64_t,11>b{};for(auto&p:people_)b[0]+=p.cash;for(size_t r=0;r<provinces_.size();++r){b[1]+=provinces_[r].firm;b[2]+=provinces_[r].treasury;if(r<municipal_funds_.size())b[2]+=municipal_funds_[r];}b[3]=carrier_;for(const auto&partner:foreign_trade_partners_)b[3]+=partner.money+partner.escrow+partner.offer_cash_escrow;for(const auto&offer:foreign_trade_offers_)b[3]+=offer.held_cash;b[4]=estate_;for(auto f:organization_funds_)b[5]+=f;b[6]=credit_pool_;b[7]=restricted_fund_;for(auto&o:proposals_)b[8]+=o.escrow;for(const auto&p:people_)b[8]+=p.military_service.escrow;for(auto&e:establishments_)b[9]+=e.cash;for(const auto&[household,cash]:household_funds_){(void)household;b[10]+=cash;}return b;}
bool Realm::checked_total_money(int64_t& total)const{
    total=0;
    auto add=[&](int64_t amount){
        if(amount<0||amount>std::numeric_limits<int64_t>::max()-total)return false;
        total+=amount;return true;
    };
    if(!add(carrier_)||!add(estate_)||!add(credit_pool_)||!add(restricted_fund_)||
       !add(market_escrow_)||!add(transport_escrow_))return false;
    for(const auto&partner:foreign_trade_partners_)
        if(!add(partner.money)||!add(partner.escrow)||!add(partner.offer_cash_escrow))return false;
    for(const auto& offer:foreign_trade_offers_)if(!add(offer.held_cash))return false;
    for(const auto& proposal:proposals_)if(!add(proposal.escrow))return false;
    for(const auto& person:people_)if(!add(person.cash)||!add(person.military_service.escrow))return false;
    for(const auto& province:provinces_)if(!add(province.firm)||!add(province.treasury))return false;
    for(const auto fund:municipal_funds_)if(!add(fund))return false;
    for(auto funds:organization_funds_)if(!add(funds))return false;
    for(const auto& establishment:establishments_)if(!add(establishment.cash))return false;
    for(const auto& [household,cash]:household_funds_){(void)household;if(!add(cash))return false;}
    return true;
}
int64_t Realm::total_money()const{
    int64_t total=0;
    if(!checked_total_money(total))throw std::overflow_error("Realm total money is not representable");
    return total;
}
bool Realm::invariant(const std::array<int64_t,kGoodIdCount>& bonded_goods)const{
    int64_t money=0,expectedMoney=0;
    if(capability_mask_==0||(capability_mask_&~kDefaultCapabilityMask)!=0)return false;
    if(urban_housing_capacity_.size()!=size_t(regions_+1)||
       urban_residents_cache_.size()!=size_t(regions_+1)||
       urban_arrivals_reserved_.size()!=size_t(regions_+1))return false;
    if(municipal_councils_.size()!=(regions_==int(kAtlasTerritoryCount)-1?kAtlasTerritoryCount:0))return false;
    std::vector<int64_t> expectedUrban(size_t(regions_+1)),expectedArrivals(size_t(regions_+1));
    for(const auto beds:urban_housing_capacity_)if(beds<0||beds>1000000000LL)return false;
    if(regions_!=int(kAtlasTerritoryCount)-1&&
       std::any_of(urban_housing_capacity_.begin(),urban_housing_capacity_.end(),[](int64_t beds){return beds!=0;}))return false;
    for(const auto& p:people_){
        for(const int value:p.stats.values)if(value<0||value>100)return false;
        for(const int value:p.personality.values)if(value<kPersonalityMinimum||value>kPersonalityMaximum)return false;
        if(static_cast<unsigned>(p.residence_kind)>static_cast<unsigned>(ResidenceKind::Urban))return false;
        if(p.residence_kind==ResidenceKind::Urban){
            if(regions_!=int(kAtlasTerritoryCount)-1||p.urban_site_id>=kAtlasUrbanSites.size()||
               kAtlasUrbanSites[p.urban_site_id].territory_id!=p.region)return false;
            if(p.alive){++expectedUrban[size_t(p.region)];if(p.destination>=0&&p.destination!=p.region)++expectedArrivals[size_t(p.destination)];}
        }else if(p.urban_site_id!=kNoUrbanSite)return false;
    }
    std::unordered_set<uint64_t> council_people;
    for(size_t site=0;site<municipal_councils_.size();++site){
        const auto& council=municipal_councils_[site];
        if(council.site_id!=site)return false;
        if(council.representative==0){if(council.term_end_day!=-1)return false;}
        else{
            if(council.representative>people_.size()||council.term_end_day<=0||council.term_end_day>365000||
               !council_people.insert(council.representative).second)return false;
            const auto& representative=person(council.representative);
            if(!representative.alive||representative.residence_kind!=ResidenceKind::Urban||representative.urban_site_id!=site)return false;
        }
        if(council.nominee==0){if(council.nomination_day!=-1)return false;}
        else{
            if(council.representative!=0&&day_<council.term_end_day)return false;
            if(council.nominee>people_.size()||council.nomination_day<0||council.nomination_day>day_||
               !council_people.insert(council.nominee).second)return false;
            const auto& nominee=person(council.nominee);
            if(!nominee.alive||nominee.age_days<18*365||nominee.residence_kind!=ResidenceKind::Urban||nominee.urban_site_id!=site)return false;
        }
    }
    if(expectedUrban!=urban_residents_cache_||expectedArrivals!=urban_arrivals_reserved_)return false;
    if(!checked_total_money(money)||opening_money_<0||endowment_<0||legacy_estate_unassigned_<0||
       endowment_>std::numeric_limits<int64_t>::max()-opening_money_)return false;
    if(regions_==int(kAtlasTerritoryCount)-1){
        const auto defaults=NationalTransportManager::default_routes();
        if(national_transport_routes_.size()!=defaults.size()||
           !NationalTransportManager::routes_are_well_formed(national_transport_routes_))return false;
        for(size_t i=0;i<defaults.size();++i)
            if(national_transport_routes_[i].atlas_link!=defaults[i].atlas_link||
               national_transport_routes_[i].from!=defaults[i].from||
               national_transport_routes_[i].to!=defaults[i].to)return false;
    }else if(!national_transport_routes_.empty()||!transport_shipments_.empty()||
             !transport_cargo_lots_.empty()||transport_escrow_!=0)return false;
    int64_t estate_accounted=legacy_estate_unassigned_;
    uint64_t previous_estate_decedent=0;
    for(const auto& item:estate_cases_){
        if(item.decedent==0||item.decedent<=previous_estate_decedent||item.decedent>people_.size()||
           person(item.decedent).alive||item.opened_day<0||item.opened_day>day_||item.region<0||item.region>regions_||
           item.cash<0||item.cash>std::numeric_limits<int64_t>::max()-estate_accounted)return false;
        estate_accounted+=item.cash;previous_estate_decedent=item.decedent;
        for(auto amount:item.inventory)if(amount<0)return false;
    }
    if(estate_accounted!=estate_)return false;
    if(foreign_trade_orders_.size()>1000000||foreign_trade_offers_.size()>1000000)return false;
    for(const auto&partner:foreign_trade_partners_){
        if(partner.money<0||partner.escrow<0||partner.offer_cash_escrow<0||partner.salt_capacity<0||partner.salt_capacity>kForeignTradePartnerSaltCapacity)return false;
        if(partner.completed_import_orders<0||partner.completed_export_orders<0||
           partner.imported_units<partner.completed_import_orders||partner.exported_units<partner.completed_export_orders||
           partner.import_gross<0||partner.export_gross<0)return false;
        if(regions_!=int(kAtlasTerritoryCount)-1&&
           (partner.money||partner.escrow||partner.offer_cash_escrow||partner.decision_sequence||partner.salt_capacity||partner.trade_suspended||
            partner.completed_import_orders||partner.completed_export_orders||partner.imported_units||partner.exported_units||
            partner.import_gross||partner.export_gross||
            std::any_of(partner.goods.begin(),partner.goods.end(),[](int64_t amount){return amount!=0;})))return false;
        for(size_t good=0;good<partner.goods.size();++good)
            if(partner.goods[good]<0||(good!=size_t(GoodId::DryFood)&&good!=size_t(GoodId::Salt)&&partner.goods[good]!=0))return false;
        if(partner.goods[size_t(GoodId::DryFood)]>kForeignTradePartnerOpeningDryFood||
           partner.goods[size_t(GoodId::Salt)]>kForeignTradePartnerSaltCapacity)return false;
    }
    expectedMoney=opening_money_+endowment_;
    if(money!=expectedMoney||provinces_.size()!=size_t(regions_+1)||market_monthly_.size()!=size_t(regions_+1)||
       market_escrow_<0||market_settled_day_!=day_||market_processed_month_!=day_/30||next_id_==0||next_fact_==0)return false;
    std::set<uint64_t> recordIds;
    auto validRecordId=[&](uint64_t id){
        // Person IDs are a separate namespace and may numerically overlap these
        // records as births extend the dense Person vector.
        return id!=0&&id<next_id_&&recordIds.insert(id).second;
    };
    if(military_raids_.size()>1000000)return false;
    unsigned pendingRaids=0;
    for(const auto& raid:military_raids_){
        const unsigned status=static_cast<unsigned>(raid.status);
        if(!validRecordId(raid.id)||raid.partner_id<0||raid.partner_id>=kForeignTradePartnerCount||
           raid.target_region!=kMilitaryRaidTargetRegion||regions_!=int(kAtlasTerritoryCount)-1||
           raid.created_day<0||raid.created_day>day_||raid.force_units<1||raid.force_units>kMilitaryRaidUnitMaximum||
           status>static_cast<unsigned>(MilitaryRaidStatus::Withdrawn)||raid.defenders.size()>16||
           raid.defense_score<0||raid.defense_score>80||raid.attacker_losses<0||raid.attacker_losses>raid.force_units||
           raid.defender_casualties<0||raid.defender_casualties>int(raid.defenders.size())||
           raid.salt_looted<0||raid.salt_looted>kForeignTradePartnerSaltCapacity||
           !std::is_sorted(raid.defenders.begin(),raid.defenders.end())||
           std::adjacent_find(raid.defenders.begin(),raid.defenders.end())!=raid.defenders.end()||
           std::any_of(raid.defenders.begin(),raid.defenders.end(),[&](uint64_t id){return id==0||id>people_.size();})||
           !std::is_sorted(raid.supplied_defenders.begin(),raid.supplied_defenders.end())||
           std::adjacent_find(raid.supplied_defenders.begin(),raid.supplied_defenders.end())!=raid.supplied_defenders.end()||
           !std::includes(raid.defenders.begin(),raid.defenders.end(),raid.supplied_defenders.begin(),raid.supplied_defenders.end()))return false;
        if(raid.status==MilitaryRaidStatus::Pending){
            if(++pendingRaids>1||raid.resolved_day!=-1||raid.defense_score||raid.attacker_losses||
               raid.defender_casualties||raid.salt_looted||!raid.defenders.empty()||!raid.supplied_defenders.empty())return false;
        }else{
            if(raid.resolved_day<raid.created_day||raid.resolved_day>day_)return false;
            if(raid.status==MilitaryRaidStatus::Repelled){
                if(raid.defense_score<=raid.force_units||raid.attacker_losses!=raid.force_units||
                   raid.defender_casualties||raid.salt_looted||raid.defenders.empty())return false;
            }else if(raid.status==MilitaryRaidStatus::Successful){
                const int expectedLosses=std::min(raid.force_units,raid.defense_score/2);
                const int expectedCasualties=std::min<int>(int(raid.defenders.size()),std::max(1,(raid.force_units+1)/2));
                if(raid.defense_score>raid.force_units||raid.defenders.empty()||
                   raid.attacker_losses!=expectedLosses||raid.defender_casualties!=expectedCasualties||
                   raid.salt_looted>raid.force_units-raid.attacker_losses)return false;
            }else if(raid.status==MilitaryRaidStatus::Withdrawn){
                if(raid.defense_score||raid.attacker_losses||raid.defender_casualties||!raid.defenders.empty()||!raid.supplied_defenders.empty()||
                   raid.salt_looted>raid.force_units)return false;
            }
        }
    }
    std::set<uint64_t> factIds;
    for(const auto& fact:facts_)
        if(fact.id==0||fact.id>=next_fact_||!factIds.insert(fact.id).second)return false;
    int64_t escrow_liability=0;uint64_t previous_order=0;
    for(const auto&order:market_orders_){
        const auto reserved=MarketManager::checked_total(order.maximum_unit_price,order.quantity);
        if(!validRecordId(order.id)||order.id<=previous_order||order.owner==0||order.owner>people_.size()||
           !person(order.owner).alive||order.region<0||order.region>regions_||order.good<0||order.good>=kGoodIdCount||
           order.placed_day<0||order.placed_day>day_||order.settle_day!=day_+1||order.settle_day!=order.placed_day+1||
           static_cast<int>(order.priority)<0||static_cast<int>(order.priority)>2||order.quantity<=0||order.quantity>1000000||
           order.maximum_unit_price<config_.market_price_floor()||order.maximum_unit_price>config_.market_price_ceiling()||!reserved||
           order.personal_escrow<0||order.household_escrow<0||order.organization_escrow<0||
           order.personal_escrow>std::numeric_limits<int64_t>::max()-order.household_escrow)return false;
        const int64_t personal_household=order.personal_escrow+order.household_escrow;
        int64_t liability=personal_household;
        if(order.organization_escrow>0){
            if(order.organization_id<1||order.organization_id>int(kOrganizationCount)||personal_household!=0||order.organization_escrow!=*reserved)return false;
            liability=order.organization_escrow;
        }else if(order.organization_id!=0||personal_household!=*reserved)return false;
        if((order.household_escrow>0&&(!order.funding_household||order.funding_household>people_.size()))||
           (order.household_escrow==0&&order.funding_household!=0)||
           liability>std::numeric_limits<int64_t>::max()-escrow_liability)return false;
        escrow_liability+=liability;previous_order=order.id;
    }
    if(escrow_liability!=market_escrow_)return false;
    std::array<int64_t,kForeignTradePartnerCount> foreignEscrowLiability{},foreignOfferEscrowLiability{},pendingExports{},pendingOfferExports{};
    std::array<int,kForeignTradePartnerCount> pendingOfferCount{};
    uint64_t previousForeignOrder=0;
    int previousForeignOrderDay=-1;
    const auto coastalAtlasLink=std::find_if(kWorldAtlasLinks.begin(),kWorldAtlasLinks.end(),[](const AtlasLink& link){
        return link.from==8&&link.to==9&&link.corridor==AtlasCorridor::Coastal&&
               link.mode==AtlasRouteMode::CoastalShipping&&link.direction==AtlasLinkDirection::Bidirectional;
    });
    if((!foreign_trade_orders_.empty()||!foreign_trade_offers_.empty())&&coastalAtlasLink==kWorldAtlasLinks.end())return false;
    const uint8_t coastalLinkIndex=coastalAtlasLink==kWorldAtlasLinks.end()?0:uint8_t(coastalAtlasLink-kWorldAtlasLinks.begin());
    for(const auto& offer:foreign_trade_offers_){
        const unsigned operation=static_cast<unsigned>(offer.operation);
        if(!validRecordId(offer.id)||offer.owner==0||offer.owner>people_.size()||offer.partner_id<0||
           offer.partner_id>=kForeignTradePartnerCount||operation>=2*kForeignTradePartnerCount||
           foreign_trade_partner_id(offer.operation)!=offer.partner_id||offer.created_day<0||offer.created_day>day_||
           offer.due_day!=offer.created_day+7||offer.due_day>365000||
           (offer.quantity!=1&&offer.quantity!=4&&offer.quantity!=8)||offer.gross<=0||
           offer.duty!=offer.gross/20||offer.port_fee!=2*offer.quantity||offer.route_fee<0||
           static_cast<unsigned>(offer.status)>static_cast<unsigned>(ForeignTradeOfferStatus::Expired))return false;
        const bool importing=foreign_trade_is_import(offer.operation);
        const int expectedGood=importing?int(GoodId::DryFood):int(GoodId::Salt);
        const int64_t expectedGross=(importing?24:28)*offer.quantity;
        if(offer.good!=expectedGood||offer.gross!=expectedGross||offer.held_cash<0||offer.held_good<0)return false;
        auto& pendingCount=pendingOfferCount[size_t(offer.partner_id)];
        if(offer.status==ForeignTradeOfferStatus::Pending){
            if(offer.due_day<day_||++pendingCount>1||offer.held_good!=offer.quantity||offer.order_id!=0)return false;
            const int64_t expectedHold=importing?offer.gross+offer.duty+offer.port_fee+offer.route_fee:offer.port_fee+offer.route_fee;
            if(offer.held_cash!=expectedHold)return false;
            if(!importing){
                if(offer.gross>std::numeric_limits<int64_t>::max()-foreignOfferEscrowLiability[size_t(offer.partner_id)]||
                   offer.quantity>std::numeric_limits<int64_t>::max()-pendingOfferExports[size_t(offer.partner_id)])return false;
                foreignOfferEscrowLiability[size_t(offer.partner_id)]+=offer.gross;
                pendingOfferExports[size_t(offer.partner_id)]+=offer.quantity;
            }
        }else{
            if(offer.held_cash||offer.held_good)return false;
            if(offer.status==ForeignTradeOfferStatus::Accepted){
                if(offer.order_id<=offer.id||offer.order_id>=next_id_)return false;
            }else if(offer.order_id!=0)return false;
        }
    }
    for(const auto& order:foreign_trade_orders_){
        const unsigned operation=static_cast<unsigned>(order.operation);
        const int partner_id=foreign_trade_partner_id(order.operation);
        const bool importing=foreign_trade_is_import(order.operation);
        const uint8_t from=importing?9:8;
        const uint8_t to=importing?8:9;
        const auto route=std::find_if(national_transport_routes_.begin(),national_transport_routes_.end(),
            [&](const NationalTransportRoute& value){return value.atlas_link==coastalLinkIndex&&value.from==from&&value.to==to;});
        if(!validRecordId(order.id)||order.id<=previousForeignOrder||order.owner==0||order.owner>people_.size()||
           operation>=2*kForeignTradePartnerCount||partner_id<0||partner_id>=kForeignTradePartnerCount||
           order.partner_id!=partner_id||order.created_day<0||order.created_day>day_||
           route==national_transport_routes_.end()||order.arrival_day>365000||
           order.arrival_day-route->travel_days<order.created_day+1||
           order.arrival_day<=day_||order.created_day==previousForeignOrderDay||
           order.quantity!=1&&order.quantity!=4&&order.quantity!=8||order.gross<0||order.duty<0||order.escrow<0)return false;
        const int expectedGood=importing?int(GoodId::DryFood):int(GoodId::Salt);
        const int64_t expectedGross=(importing?24:28)*order.quantity;
        const int64_t expectedDuty=expectedGross/20;
        const int64_t expectedEscrow=importing?expectedGross+expectedDuty:expectedGross;
        if(order.good!=expectedGood||order.gross!=expectedGross||order.duty!=expectedDuty||order.escrow!=expectedEscrow||
           order.escrow>std::numeric_limits<int64_t>::max()-foreignEscrowLiability[size_t(partner_id)])return false;
        foreignEscrowLiability[size_t(partner_id)]+=order.escrow;
        if(!importing){
            if(order.quantity>std::numeric_limits<int64_t>::max()-pendingExports[size_t(partner_id)])return false;
            pendingExports[size_t(partner_id)]+=order.quantity;
        }
        previousForeignOrder=order.id;previousForeignOrderDay=order.created_day;
    }
    for(size_t partner_id=0;partner_id<foreign_trade_partners_.size();++partner_id){
        const auto& partner=foreign_trade_partners_[partner_id];
        if(foreignEscrowLiability[partner_id]!=partner.escrow||foreignOfferEscrowLiability[partner_id]!=partner.offer_cash_escrow)return false;
        if(regions_==int(kAtlasTerritoryCount)-1&&
           partner.goods[size_t(GoodId::Salt)]+partner.salt_capacity+pendingExports[partner_id]+pendingOfferExports[partner_id]!=kForeignTradePartnerSaltCapacity)return false;
    }
    uint64_t previous_shipment=0,previous_lot=0;
    int64_t calculated_transport_escrow=0;
    std::map<uint64_t,int64_t> held_by_shipment;
    std::map<uint64_t,int64_t> fee_by_shipment;
    std::map<uint8_t,int64_t> capacity_loaded_today;
    if(transport_shipments_.size()>1000000||transport_cargo_lots_.size()>1000000||next_transport_lot_id_==0)return false;
    for(const auto&shipment:transport_shipments_){
        const auto owner_kind=static_cast<unsigned>(shipment.owner_kind);
        if(!validRecordId(shipment.id)||shipment.id<=previous_shipment||shipment.owner==0||shipment.owner>people_.size()||
           owner_kind>unsigned(TransportShipmentOwnerKind::MilitaryRationDepot)||
           (shipment.owner_kind==TransportShipmentOwnerKind::MilitaryRationDepot&&
            (shipment.source_region<0||shipment.source_region>regions_||
             (shipment.good!=int(GoodId::DryFood)&&shipment.good!=int(GoodId::SupplementaryFood))))||
           shipment.created_day<0||shipment.created_day>day_||shipment.destination<0||shipment.destination>regions_||
           !valid_good_id(shipment.good)||static_cast<unsigned>(shipment.service_goal)>unsigned(TransportServiceGoal::LowestFee)||
           shipment.quantity<=0||shipment.quantity>1000000||shipment.delivered<0||shipment.canceled<0||
           shipment.delivered>shipment.quantity||shipment.canceled>shipment.quantity-shipment.delivered||
           shipment.fee_escrow<0||shipment.route_indices.empty()||shipment.route_indices.size()>size_t(regions_))return false;
        int expected_from=-1;
        std::array<bool,kAtlasTerritoryCount> visited_regions{};
        for(const size_t route_index:shipment.route_indices){
            if(route_index>=national_transport_routes_.size())return false;
            const auto&route=national_transport_routes_[route_index];
            if(expected_from>=0&&route.from!=expected_from)return false;
            if(!visited_regions[route.from]&&expected_from<0)visited_regions[route.from]=true;
            if(visited_regions[route.to])return false;
            visited_regions[route.to]=true;
            expected_from=route.to;
        }
        if(expected_from!=shipment.destination||
           (shipment.owner_kind==TransportShipmentOwnerKind::MilitaryRationDepot&&
            national_transport_routes_.at(shipment.route_indices.front()).from!=shipment.source_region)||
           shipment.fee_escrow>std::numeric_limits<int64_t>::max()-calculated_transport_escrow)return false;
        calculated_transport_escrow+=shipment.fee_escrow;
        previous_shipment=shipment.id;
    }
    for(const auto&lot:transport_cargo_lots_){
        if(lot.id==0||lot.id<=previous_lot||lot.id>=next_transport_lot_id_||lot.quantity<=0||
           lot.current_region<0||lot.current_region>regions_||lot.arrival_day<0)return false;
        const auto shipment=std::lower_bound(transport_shipments_.begin(),transport_shipments_.end(),lot.shipment_id,
            [](const TransportShipment&value,uint64_t id){return value.id<id;});
        if(shipment==transport_shipments_.end()||shipment->id!=lot.shipment_id||lot.route_position>=shipment->route_indices.size())return false;
        const auto&route=national_transport_routes_[shipment->route_indices[lot.route_position]];
        if(route.from!=lot.current_region||(lot.arrival_day>0&&
           (lot.arrival_day<day_||lot.arrival_day-day_>route.travel_days)))return false;
        if(lot.quantity>std::numeric_limits<int64_t>::max()-held_by_shipment[lot.shipment_id])return false;
        held_by_shipment[lot.shipment_id]+=lot.quantity;
        int64_t remaining_fee_per_unit=0;
        const size_t first_unpaid=lot.route_position+(lot.arrival_day>0?1:0);
        for(size_t leg=first_unpaid;leg<shipment->route_indices.size();++leg){
            const int64_t fee=national_transport_routes_[shipment->route_indices[leg]].fee_per_unit;
            if(fee>std::numeric_limits<int64_t>::max()-remaining_fee_per_unit)return false;
            remaining_fee_per_unit+=fee;
        }
        if(remaining_fee_per_unit&&lot.quantity>std::numeric_limits<int64_t>::max()/remaining_fee_per_unit)return false;
        const int64_t liability=remaining_fee_per_unit*lot.quantity;
        if(liability>std::numeric_limits<int64_t>::max()-fee_by_shipment[lot.shipment_id])return false;
        fee_by_shipment[lot.shipment_id]+=liability;
        if(lot.arrival_day>0&&lot.arrival_day-route.travel_days==day_){
            if(lot.quantity>std::numeric_limits<int64_t>::max()-capacity_loaded_today[route.atlas_link])return false;
            capacity_loaded_today[route.atlas_link]+=lot.quantity;
        }
        previous_lot=lot.id;
    }
    for(const auto&order:foreign_trade_orders_){
        const bool importing=foreign_trade_is_import(order.operation);
        const uint8_t from=importing?9:8;
        const uint8_t to=importing?8:9;
        const auto route=std::find_if(national_transport_routes_.begin(),national_transport_routes_.end(),
            [&](const NationalTransportRoute&value){return value.atlas_link==coastalLinkIndex&&value.from==from&&value.to==to;});
        if(route==national_transport_routes_.end())return false;
        if(order.arrival_day-route->travel_days==day_){
            if(order.quantity>std::numeric_limits<int64_t>::max()-capacity_loaded_today[coastalLinkIndex])return false;
            capacity_loaded_today[coastalLinkIndex]+=order.quantity;
        }
    }
    if(calculated_transport_escrow!=transport_escrow_)return false;
    for(const auto&shipment:transport_shipments_){
        const int64_t remaining=shipment.quantity-shipment.delivered-shipment.canceled;
        if(held_by_shipment[shipment.id]!=remaining||fee_by_shipment[shipment.id]!=shipment.fee_escrow)return false;
        if(remaining>0&&shipment.owner_kind==TransportShipmentOwnerKind::Person&&!person(shipment.owner).alive){
            const auto estate=std::lower_bound(estate_cases_.begin(),estate_cases_.end(),shipment.owner,
                [](const EstateCase&value,uint64_t id){return value.decedent<id;});
            if(estate==estate_cases_.end()||estate->decedent!=shipment.owner)return false;
        }
    }
    for(const auto&[link,quantity]:capacity_loaded_today){
        const auto route=std::find_if(national_transport_routes_.begin(),national_transport_routes_.end(),
            [link](const NationalTransportRoute&value){return value.atlas_link==link;});
        if(route==national_transport_routes_.end()||quantity>route->capacity_per_day)return false;
    }
    for(const auto&region:market_monthly_)for(const auto&totals:region)
        if(totals.demand<0||totals.unmet<0||totals.unmet>totals.demand)return false;
    for(auto funds:organization_funds_)if(funds<0)return false;
    if(municipal_funds_.size()!=size_t(regions_+1)||municipal_last_funded_period_.size()!=size_t(regions_+1))return false;
    for(size_t region=0;region<municipal_funds_.size();++region)
        if(municipal_funds_[region]<0||municipal_last_funded_period_[region]<-1||
           municipal_last_funded_period_[region]>day_/30)return false;
    for(const auto&[household,cash]:household_funds_)
        if(cash<=0||!households_.contains(household))return false;
    std::map<uint64_t,int64_t> effortAttentionByActor;
    for(const auto&[key,attention]:schedule_book_.work_effort_attention_by_establishment){
        const auto [actor,establishment]=key;(void)establishment;
        if(actor==0||actor>people_.size()||attention<0||attention>std::numeric_limits<int64_t>::max()-effortAttentionByActor[actor])return false;
        effortAttentionByActor[actor]+=attention;
    }
    if(military_ration_depots_.size()!=size_t(regions_+1))return false;
    if(military_supplementary_ration_depots_.size()!=size_t(regions_+1))return false;
    auto stock=bonded_goods;for(auto n:stock)if(n<0)return false;
    for(const auto depot:military_ration_depots_){if(depot<0||depot>std::numeric_limits<int64_t>::max()-stock[size_t(GoodId::DryFood)])return false;stock[size_t(GoodId::DryFood)]+=depot;}
    for(const auto depot:military_supplementary_ration_depots_){if(depot<0||depot>std::numeric_limits<int64_t>::max()-stock[size_t(GoodId::SupplementaryFood)])return false;stock[size_t(GoodId::SupplementaryFood)]+=depot;}
    for(size_t i=0;i<people_.size();++i){auto&p=people_[i];const auto&body=p.body_state;const bool noRest=body.rest_begin_tick==0&&body.rest_until_tick==0&&body.rest_region==-1&&!body.rest_health_pending;const bool activeRest=body.rest_begin_tick>=0&&body.rest_begin_tick<=schedule_book_.cursor&&body.rest_until_tick>body.rest_begin_tick&&body.rest_until_tick<=365000LL*kSubdayUnitsPerDay&&body.rest_region>=0&&body.rest_region<=regions_&&body.rest_health_pending;const auto effort=effortAttentionByActor.find(p.id);const int64_t knownEffort=effort==effortAttentionByActor.end()?0:effort->second;if(p.id!=i+1||p.nutrition<0||p.nutrition>100||p.health>100||p.skill<kCapabilityMinimum||p.skill>kCapabilityMaximum||body.sleep_debt<0||body.sleep_debt>100||body.exertion<0||body.exertion>100||body.sleep_balance_remainder<-63||body.sleep_balance_remainder>63||body.exertion_effort_attention_processed<0||body.exertion_effort_attention_processed>knownEffort||body.exertion_effort_attention_remainder<0||body.exertion_effort_attention_remainder>=kExertionAttentionUnitsPerPoint||body.exertion_rest_tick_remainder<0||body.exertion_rest_tick_remainder>=kExertionRestTicksPerPoint||(!noRest&&!activeRest)||p.work_assignment.job_kind<0||p.work_assignment.job_kind>1||p.organization<0||p.organization>int(kOrganizationCount)||p.tax_remainder<0||p.tax_remainder>=100||p.cash<0||p.region<0||p.region>regions_||p.household_membership.household_id==0||p.household_membership.household_id>people_.size())return false;for(size_t g=0;g<kGoodIdCount;++g){if(p.inventory[g]<0||p.inventory[g]>std::numeric_limits<int64_t>::max()-stock[g])return false;stock[g]+=p.inventory[g];}}
    for(const auto& person:people_){const auto& service=person.military_service;
        if(!service.active){if(service.combat_consent||service.commander||service.start_day||service.end_day||service.source_region||service.mobilized_until_day||service.wage_per_day||service.escrow||service.paid_total)return false;continue;}
        if(!person.alive||person.family!=Family::Civilian||service.commander==0||service.commander>people_.size()||
           service.start_day<1||service.end_day<=service.start_day||service.end_day-service.start_day>30||
           service.end_day>365000||service.source_region<0||service.source_region>regions_||
           (service.combat_consent&&day_<service.start_day)||
           service.mobilized_until_day<0||service.mobilized_until_day>service.end_day||
           (service.mobilized_until_day>0&&service.mobilized_until_day<=day_)||
           service.wage_per_day!=kMilitaryServiceTrialWagePerDay||service.escrow<0||service.paid_total<0||
           service.wage_per_day>std::numeric_limits<int64_t>::max()/(service.end_day-service.start_day))return false;
        const int64_t total=service.wage_per_day*(service.end_day-service.start_day);
        if(service.paid_total>total||service.paid_total%service.wage_per_day!=0||service.escrow!=total-service.paid_total)return false;
    }
    if(military_obligations_.size()>1000000)return false;
    std::unordered_set<uint64_t> obligation_ids,assigned_members;
    std::unordered_set<uint64_t> open_obligors,pending_obligation_offers,pending_supply_offers,pending_extensions;
    for(const auto& obligation:military_obligations_){
        if(obligation.id==0||obligation.id>=next_id_||!obligation_ids.insert(obligation.id).second||
           obligation.obligor==0||obligation.obligor>people_.size()||obligation.recipient==0||obligation.recipient>people_.size()||
           obligation.source_region<0||obligation.source_region>regions_||obligation.destination_region<0||obligation.destination_region>regions_||
           obligation.created_day<0||obligation.created_day>day_||obligation.first_due_day!=obligation.created_day+14||
           obligation.due_day<obligation.first_due_day||obligation.due_day>day_+14||obligation.completed_day>day_||obligation.completed_day<-1||
           obligation.extensions<0||obligation.extensions>1||obligation.required_members<1||obligation.required_members>4||
           obligation.member_ids.size()!=size_t(obligation.required_members)||obligation.delivered_member_ids.size()>obligation.member_ids.size()||
           obligation.delivered_at_first_deadline<0||obligation.delivered_at_first_deadline>obligation.required_members||
           obligation.shortage_at_first_deadline<0||(obligation.first_deadline_checked&&obligation.delivered_at_first_deadline+obligation.shortage_at_first_deadline!=obligation.required_members)||
           obligation.delivered_at_last_deadline<0||obligation.delivered_at_last_deadline>obligation.required_members||
           obligation.shortage_at_last_deadline<0||(obligation.due_checked&&obligation.delivered_at_last_deadline+obligation.shortage_at_last_deadline!=obligation.required_members)||
           (!obligation.first_deadline_checked&&(obligation.delivered_at_first_deadline||obligation.shortage_at_first_deadline))||
           (!obligation.due_checked&&(obligation.delivered_at_last_deadline||obligation.shortage_at_last_deadline))||
           (obligation.due_checked&&obligation.due_day>day_)||(obligation.extensions==1&&!obligation.first_deadline_checked)||
           (obligation.completed_day>=0&&obligation.delivered_member_ids.size()!=obligation.member_ids.size())||
           (obligation.completed_day<0&&obligation.delivered_member_ids.size()==obligation.member_ids.size()))return false;
        if(obligation.completed_day<0&&!open_obligors.insert(obligation.obligor).second)return false;
        if(!std::is_sorted(obligation.member_ids.begin(),obligation.member_ids.end())||
           std::adjacent_find(obligation.member_ids.begin(),obligation.member_ids.end())!=obligation.member_ids.end()||
           !std::is_sorted(obligation.delivered_member_ids.begin(),obligation.delivered_member_ids.end())||
           std::adjacent_find(obligation.delivered_member_ids.begin(),obligation.delivered_member_ids.end())!=obligation.delivered_member_ids.end())return false;
        for(auto id:obligation.member_ids){
            if(id==0||id>people_.size())return false;
            if(obligation.completed_day<0&&!assigned_members.insert(id).second)return false;
            if(std::find(obligation.delivered_member_ids.begin(),obligation.delivered_member_ids.end(),id)==obligation.delivered_member_ids.end())continue;
        }
        for(auto id:obligation.delivered_member_ids)
            if(!std::binary_search(obligation.member_ids.begin(),obligation.member_ids.end(),id))return false;
    }
    if(military_supply_obligations_.size()>1000000)return false;
    std::unordered_set<uint64_t> supply_ids,supplied_personnel_obligations,open_supply_obligations;
    for(const auto& supply:military_supply_obligations_){
        const auto source=std::find_if(military_obligations_.begin(),military_obligations_.end(),[&](const auto& item){
            return item.id==supply.personnel_obligation_id&&item.obligor==supply.obligor&&item.recipient==supply.recipient&&
                item.source_region==supply.source_region&&item.destination_region==supply.destination_region;
        });
        if(supply.id==0||supply.id>=next_id_||!supply_ids.insert(supply.id).second||source==military_obligations_.end()||
           !supplied_personnel_obligations.insert(supply.personnel_obligation_id).second||
           supply.obligor==0||supply.obligor>people_.size()||supply.recipient==0||supply.recipient>people_.size()||
           supply.source_region<0||supply.source_region>regions_||supply.destination_region<0||supply.destination_region>regions_||
           supply.created_day<0||supply.created_day>day_||supply.first_due_day!=supply.created_day+14||
           supply.due_day<supply.first_due_day||supply.due_day>day_+14||supply.completed_day>day_||supply.completed_day<-1||
           supply.extensions<0||supply.extensions>1||supply.required!=source->required_members||
           supply.first_eligible_shipment_id==0||supply.first_eligible_shipment_id>next_id_||
           supply.delivered_at_first_deadline<0||supply.delivered_at_first_deadline>supply.required||
           supply.shortage_at_first_deadline<0||
           (supply.first_deadline_checked&&supply.delivered_at_first_deadline+supply.shortage_at_first_deadline!=supply.required)||
           supply.delivered_at_last_deadline<0||supply.delivered_at_last_deadline>supply.required||
           supply.shortage_at_last_deadline<0||
           (supply.due_checked&&supply.delivered_at_last_deadline+supply.shortage_at_last_deadline!=supply.required)||
           (!supply.first_deadline_checked&&(supply.delivered_at_first_deadline||supply.shortage_at_first_deadline))||
           (!supply.due_checked&&(supply.delivered_at_last_deadline||supply.shortage_at_last_deadline))||
           (supply.due_checked&&supply.due_day>day_)||(supply.extensions==1&&!supply.first_deadline_checked))return false;
        int64_t delivered=0;
        for(const auto& shipment:transport_shipments_)if(shipment.id>=supply.first_eligible_shipment_id&&
            shipment.owner_kind==TransportShipmentOwnerKind::MilitaryRationDepot&&shipment.owner==supply.obligor&&
            shipment.source_region==supply.source_region&&shipment.destination==supply.destination_region&&
            shipment.good==int(GoodId::DryFood)){
            if(shipment.delivered>std::numeric_limits<int64_t>::max()-delivered)return false;
            delivered+=shipment.delivered;
        }
        delivered=std::min(delivered,supply.required);
        if(delivered!=supply.delivered||
           (supply.completed_day>=0&&delivered<supply.required)||(supply.completed_day<0&&delivered>=supply.required))return false;
        if(supply.completed_day<0&&!open_supply_obligations.insert(supply.personnel_obligation_id).second)return false;
    }
    for(auto&p:provinces_){if(p.firm<0||p.treasury<0||p.sales_tax_remainder<0||p.sales_tax_remainder>=100)return false;for(size_t g=0;g<kGoodIdCount;++g){if(p.stock[g]<0||p.stock[g]>std::numeric_limits<int64_t>::max()-stock[g])return false;stock[g]+=p.stock[g];}}
    for(auto const&e:establishments_){for(size_t g=0;g<kGoodIdCount;++g){const int64_t amount=e.stock[g];if(amount<0||amount>std::numeric_limits<int64_t>::max()-stock[g])return false;stock[g]+=amount;}}
    for(const auto&shipment:transport_shipments_)for(const auto&lot:transport_cargo_lots_)if(lot.shipment_id==shipment.id){
        const size_t good=size_t(shipment.good);
        if(lot.quantity>std::numeric_limits<int64_t>::max()-stock[good])return false;
        stock[good]+=lot.quantity;
    }
    for(const auto&partner:foreign_trade_partners_)
        for(size_t good=0;good<partner.goods.size();++good){
            const auto amount=partner.goods[good];
            if(amount<0||amount>std::numeric_limits<int64_t>::max()-stock[good])return false;
            stock[good]+=amount;
        }
    for(const auto&order:foreign_trade_orders_){
        const size_t good=size_t(order.good);
        if(order.quantity>std::numeric_limits<int64_t>::max()-stock[good])return false;
        stock[good]+=order.quantity;
    }
    for(const auto&offer:foreign_trade_offers_)if(offer.held_good){
        const size_t good=size_t(offer.good);
        if(offer.held_good>std::numeric_limits<int64_t>::max()-stock[good])return false;
        stock[good]+=offer.held_good;
    }
    for(const auto& item:estate_cases_)for(size_t g=0;g<kGoodIdCount;++g){const auto amount=item.inventory[g];if(amount<0||amount>std::numeric_limits<int64_t>::max()-stock[g])return false;stock[g]+=amount;}
    std::unordered_set<uint64_t> pending_service_recipients;
    for(auto&o:proposals_){
        if(o.escrow<0||o.kind<0||o.kind>8)return false;
        if(o.kind==4){
            if(o.from==0||o.from>people_.size()||o.to==0||o.to>people_.size()||o.source_region<0||o.source_region>regions_||
               o.amount!=kMilitaryServiceTrialWagePerDay||o.service_term<1||o.service_term>30||
               o.amount>std::numeric_limits<int64_t>::max()/o.service_term||
               (o.resolved?o.escrow!=0:o.escrow!=o.amount*o.service_term))return false;
            if(!o.resolved&&(!pending_service_recipients.insert(o.to).second||person(o.to).military_service.active))return false;
        }else if(o.kind==5||o.kind==6||o.kind==7||o.kind==8){
            if(o.from==0||o.from>people_.size()||o.to==0||o.to>people_.size()||o.source_region<0||o.source_region>regions_||
               o.amount<1||o.amount>((o.kind==5||o.kind==7)?4:7)||o.escrow!=0||
               ((o.kind==5||o.kind==7)&&o.service_term!=14)||((o.kind==6||o.kind==8)&&o.service_term!=7))return false;
            if(o.kind==5&&(o.roster_members.size()!=size_t(o.amount)||
               !std::is_sorted(o.roster_members.begin(),o.roster_members.end())||
               std::adjacent_find(o.roster_members.begin(),o.roster_members.end())!=o.roster_members.end()))return false;
            if(o.kind!=5&&!o.roster_members.empty())return false;
            if(!o.resolved&&o.kind==5&&!pending_obligation_offers.insert(o.from).second)return false;
            if(!o.resolved&&(o.kind==6||o.kind==8)){
                if(!pending_extensions.insert(o.cause).second)return false;
                if(o.kind==6){
                    const auto linked=std::find_if(military_obligations_.begin(),military_obligations_.end(),[&](const auto& item){
                        return item.id==o.cause&&item.obligor==o.from&&item.recipient==o.to&&item.completed_day<0&&
                            item.due_checked&&item.shortage_at_last_deadline>0&&item.extensions==0;
                    });
                    if(linked==military_obligations_.end())return false;
                }else{
                    const auto linked=std::find_if(military_supply_obligations_.begin(),military_supply_obligations_.end(),[&](const auto& item){
                        return item.id==o.cause&&item.obligor==o.from&&item.recipient==o.to&&item.completed_day<0&&
                            item.due_checked&&item.shortage_at_last_deadline>0&&item.extensions==0;
                    });
                    if(linked==military_supply_obligations_.end())return false;
                }
            }
            if(o.kind==7){
                const auto source=std::find_if(military_obligations_.begin(),military_obligations_.end(),[&](const auto& item){
                    return item.id==o.cause&&item.obligor==o.from&&item.recipient==o.to&&
                        item.required_members==o.amount;
                });
                if(source==military_obligations_.end())return false;
                const auto accepted=std::find_if(military_supply_obligations_.begin(),military_supply_obligations_.end(),[&](const auto& item){
                    return item.id==o.id&&item.personnel_obligation_id==o.cause;
                });
                if(o.resolved){
                    // Rejected proposals have no linked supply record. Accepted
                    // proposals keep their source link after personnel service ends.
                    if(accepted!=military_supply_obligations_.end()&&
                       (accepted->obligor!=o.from||accepted->recipient!=o.to||accepted->required!=o.amount))return false;
                }else{
                    if(source->completed_day>=0)return false;
                    if(accepted!=military_supply_obligations_.end()||!pending_supply_offers.insert(o.cause).second)return false;
                    for(const auto& item:military_supply_obligations_)
                        if(item.personnel_obligation_id==o.cause)return false;
                }
            }
            if(o.kind==8&&o.resolved){
                const auto accepted=std::find_if(military_supply_obligations_.begin(),military_supply_obligations_.end(),[&](const auto& item){
                    return item.id==o.cause&&item.extensions==1;
                });
                // A resolved kind-8 proposal may have been rejected; its source
                // can therefore still have zero extensions.
                const auto source=std::find_if(military_supply_obligations_.begin(),military_supply_obligations_.end(),[&](const auto& item){return item.id==o.cause;});
                if(source==military_supply_obligations_.end()||
                   (accepted!=military_supply_obligations_.end()&&source->due_day<o.due))return false;
            }
        }else if(o.service_term!=0)return false;
        if(o.kind==2&&!o.resolved)for(bool pledged:o.pledged)if(pledged){
            const size_t iron=size_t(canonical_from_legacy_good(3));
            const size_t tools=size_t(canonical_from_legacy_good(5));
            if(stock[iron]>std::numeric_limits<int64_t>::max()-4||
               stock[tools]>std::numeric_limits<int64_t>::max()-2)return false;
            stock[iron]+=4;stock[tools]+=2;
        }
    }
    if(credit_pool_<0||restricted_fund_<0)return false;
    for(size_t g=0;g<kGoodIdCount;++g){if(consumed_[g]<0||opening_[g]<0||produced_[g]<0||stock[g]>std::numeric_limits<int64_t>::max()-consumed_[g]||opening_[g]>std::numeric_limits<int64_t>::max()-produced_[g]||stock[g]+consumed_[g]!=opening_[g]+produced_[g])return false;}
    for(const auto&l:loans_)if(!validRecordId(l.id)||l.principal<=0||l.principal>1000000||l.interest<0||l.interest>std::numeric_limits<int64_t>::max()-l.principal||l.paid<0||l.paid>l.principal+l.interest||l.debtor==0||l.debtor>people_.size()||l.creditor<0||l.creditor>regions_||l.due<0)return false;
    for(const auto&project:projects_){
        if(!validRecordId(project.id)||project.kind<0||project.kind>4||project.region<0||project.region>regions_)return false;
        if(project.kind==4&&(regions_!=int(kAtlasTerritoryCount)-1||project.owner==0||project.owner>people_.size()||
           person(project.owner).region!=project.region||
           project.budget!=kUrbanHousingProjectBudget))return false;
    }
    if(knowledge_observation_counts_.size()!=people_.size())return false;
    std::vector<std::array<uint32_t,2>> expectedKnowledgeCounts(people_.size());
    for(const auto&known:knowledge_){
        const auto kind=static_cast<unsigned>(known.kind);
        if(kind>static_cast<unsigned>(KnowledgeKind::SecretEvidence)||
           known.observer==0||known.observer>people_.size()||
           known.confidence_basis_points<kUnknownKnowledgeConfidence||
           known.confidence_basis_points>kKnowledgeConfidenceBasisPointMaximum)return false;
        if(known.kind==KnowledgeKind::SecretEvidence)++expectedKnowledgeCounts[known.observer-1][0];
        else ++expectedKnowledgeCounts[known.observer-1][1];
        if(known.kind==KnowledgeKind::SecretEvidence){if(known.source_investigation_id==0)return false;}
        else if(known.source_investigation_id!=0||known.confidence_basis_points!=kUnknownKnowledgeConfidence)return false;
    }
    if(knowledge_observation_counts_!=expectedKnowledgeCounts)return false;
    const bool hasSourcedKnowledge=std::any_of(knowledge_.begin(),knowledge_.end(),
        [](const Knowledge&known){return known.source_investigation_id!=0;});
    std::unordered_map<uint64_t,const Investigation*> investigationsById;
    if(hasSourcedKnowledge){
        investigationsById.reserve(investigations_.size());
        for(const auto&investigation:investigations_){
            if(!validRecordId(investigation.id))return false;
            investigationsById.emplace(investigation.id,&investigation);
        }
        for(const auto&known:knowledge_)if(known.source_investigation_id){
            const auto source=investigationsById.find(known.source_investigation_id);
            const auto fact=std::lower_bound(facts_.begin(),facts_.end(),known.fact,
                [](const Fact&value,uint64_t id){return value.id<id;});
            if(source==investigationsById.end()||fact==facts_.end()||fact->id!=known.fact||
               source->second->owner!=known.observer||!source->second->completed||
               source->second->region!=fact->region||!fact->secret||fact->day>source->second->due||
               known.arrives<source->second->due||known.arrives>day_)return false;
        }
    }else for(const auto&investigation:investigations_)if(!validRecordId(investigation.id))return false;
    for(const auto&proposal:proposals_)if(!validRecordId(proposal.id))return false;
    uint64_t previousLandId = 0;
    for (const auto& holding : land_holdings_) {
        if (holding.id <= previousLandId || !validRecordId(holding.id) ||
            holding.region < 0 || holding.region > regions_ || holding.area_units <= 0 ||
            holding.title_holder == 0 || holding.title_holder > people_.size()) return false;
        previousLandId = holding.id;
    }
    uint64_t previousLandUseId = 0;
    std::set<uint64_t> landUseHoldings;
    for (const auto& right : land_use_rights_) {
        if (right.id <= previousLandUseId || !validRecordId(right.id) || right.grantee == 0 ||
            right.grantee > people_.size() || right.grantor == 0 || right.grantor > people_.size() ||
            right.grantee == right.grantor || !landUseHoldings.insert(right.holding_id).second) return false;
        const auto holding = std::lower_bound(land_holdings_.begin(), land_holdings_.end(), right.holding_id,
            [](const LandHolding& value, uint64_t id) { return value.id < id; });
        if (holding == land_holdings_.end() || holding->id != right.holding_id ||
            holding->title_holder != right.grantor) return false;
        previousLandUseId = right.id;
    }
    uint64_t previousResidenceId=0;
    std::set<uint64_t> residenceHoldings,residenceTenants;
    for(const auto& home:private_residences_){
        if(regions_!=int(kAtlasTerritoryCount)-1||home.id<=previousResidenceId||!validRecordId(home.id)||
           home.region<0||home.region>regions_||home.construction_due_day<0||home.construction_due_day>365000||
           (home.completed&&home.construction_due_day>day_)||(!home.completed&&home.construction_due_day<=day_)||
           !residenceHoldings.insert(home.holding_id).second)return false;
        const auto holding=std::lower_bound(land_holdings_.begin(),land_holdings_.end(),home.holding_id,
            [](const LandHolding& value,uint64_t id){return value.id<id;});
        if(holding==land_holdings_.end()||holding->id!=home.holding_id||holding->region!=home.region)return false;
        const bool leased=home.tenant_household!=0,offered=home.offered_household!=0;
        if(leased){
            if(offered||home.tenant_household>people_.size()||home.tenant_household==holding->title_holder||
               !residenceTenants.insert(home.tenant_household).second||home.rent_per_period!=kPrivateResidenceRentPerPeriod||
               home.next_rent_day<day_||home.next_rent_day>home.lease_end_day||home.lease_end_day>365000||
               !home.completed)return false;
        }else if(home.rent_per_period!=0||home.next_rent_day!=-1||home.lease_end_day!=-1)return false;
        if(offered){
            if(leased||home.offered_household>people_.size()||home.offered_household==holding->title_holder||
               !residenceTenants.insert(home.offered_household).second||home.offered_rent!=kPrivateResidenceRentPerPeriod||
               home.offer_expires_day<day_||home.offer_expires_day>365000||!home.completed)return false;
        }else if(home.offered_rent!=0||home.offer_expires_day!=-1)return false;
        previousResidenceId=home.id;
    }
    for(const auto& item:estate_cases_){
        const auto valid_refs=[](const std::vector<uint64_t>& ids){return std::is_sorted(ids.begin(),ids.end())&&std::adjacent_find(ids.begin(),ids.end())==ids.end();};
        if(!valid_refs(item.establishments)||!valid_refs(item.land_holdings)||!valid_refs(item.land_use_rights)||!valid_refs(item.loans))return false;
        for(auto id:item.establishments){
            auto it=std::lower_bound(establishments_.begin(),establishments_.end(),id,[](const Establishment& value,uint64_t key){return value.id<key;});
            if(it==establishments_.end()||it->id!=id||it->owner_kind!=EstablishmentOwnerKind::Person||it->owner_id!=item.decedent)return false;
        }
        for(auto id:item.land_holdings){
            auto it=std::lower_bound(land_holdings_.begin(),land_holdings_.end(),id,[](const LandHolding& value,uint64_t key){return value.id<key;});
            if(it==land_holdings_.end()||it->id!=id||it->title_holder!=item.decedent)return false;
        }
        for(auto id:item.land_use_rights){
            auto it=std::lower_bound(land_use_rights_.begin(),land_use_rights_.end(),id,[](const LandUseRight& value,uint64_t key){return value.id<key;});
            if(it==land_use_rights_.end()||it->id!=id||it->grantor!=item.decedent)return false;
        }
        for(auto id:item.loans){
            auto it=std::find_if(loans_.begin(),loans_.end(),[&](const Loan& value){return value.id==id;});
            if(it==loans_.end()||it->debtor!=item.decedent)return false;
        }
    }
    std::set<std::pair<uint64_t,uint64_t>> spousePairs;
    std::vector<int> livingSpouses(people_.size(),0);
    for(auto&e:edges_){
        if(!e.from||!e.to||e.from>people_.size()||e.to>people_.size()||e.kind>RelationKind::Spouse||
           (e.legal_parentage&&(e.kind!=RelationKind::ParentChild||e.from==e.to)))return false;
        if(e.kind==RelationKind::Spouse){
            if(e.from>=e.to||!spousePairs.emplace(e.from,e.to).second)return false;
            if(person(e.from).alive&&person(e.to).alive&&
               (++livingSpouses[e.from-1]>1||++livingSpouses[e.to-1]>1))return false;
        }
    }
    if(!households_consistent())return false;
    if(schedule_book_.cursor<0||schedule_book_.cursor>365000LL*kSubdayUnitsPerDay)return false;
    if(day_!=static_cast<int>(schedule_book_.cursor/kSubdayUnitsPerDay))return false;
    std::set<uint64_t> seen;
    for(auto const&[actor,sessions]:schedule_book_.sessions){
        if(actor==0||actor>people_.size())return false;
        if(sessions.empty())return false;
        for(auto const&s:sessions){
            if(s.id==0||seen.count(s.id))return false;
            seen.insert(s.id);
            if(s.actor!=actor)return false;
            if(s.kind==ActivityKind::Work){
                if(s.region<0||s.region>regions_)return false;
                if(s.establishment_id!=0){
                    auto site=std::lower_bound(establishments_.begin(),establishments_.end(),s.establishment_id,
                        [](const Establishment& e,uint64_t id){return e.id<id;});
                    if(site==establishments_.end()||site->id!=s.establishment_id||site->region!=s.region)return false;
                }
            }else if(s.establishment_id!=0||s.region!=-1)return false;
            if(s.begin<0||s.begin>=s.end||s.end>365000LL*kSubdayUnitsPerDay)return false;
            if(s.demand_rate<activity::kMinDemandRate||s.demand_rate>activity::kMaxDemandRate)return false;
            if(s.requested<0||s.delivered<0||s.unmet<0||s.capacity_unmet<0||s.unresolved_unmet<0)return false;
            if(s.delivered>s.requested||s.capacity_unmet>s.unmet||s.unresolved_unmet>s.unmet)return false;
             if(s.delivered>s.requested)return false;
             if(s.unmet!=s.requested-s.delivered)return false;
            if(s.capacity_unmet>s.unmet)return false;
            if(s.unresolved_unmet!=s.unmet-s.capacity_unmet)return false;
            if(s.retired&&(s.retired_tick<0||s.retired_tick>schedule_book_.cursor))return false;
            int64_t processedEnd=s.retired?s.retired_tick:schedule_book_.cursor;if(processedEnd>s.end)processedEnd=s.end;if(processedEnd<s.begin)processedEnd=s.begin;if(s.requested!=s.demand_rate*(processedEnd-s.begin))return false;
            if(s.retired){if(s.retired_tick<0||s.retired_tick>schedule_book_.cursor)return false;if(!s.retired_cause)return false;}
            else {if(s.retired_tick!=0||s.retired_cause!=0)return false;}
        }
        // Work sessions at different sites may not overlap. Legacy site-zero
        // sessions remain readable and may overlap with one another.
        std::vector<const ActivitySession*> siteWork;
        for(const auto& s:sessions)if(!s.retired&&s.kind==ActivityKind::Work)siteWork.push_back(&s);
        std::sort(siteWork.begin(),siteWork.end(),[](auto a,auto b){return std::tie(a->begin,a->end,a->establishment_id)<std::tie(b->begin,b->end,b->establishment_id);});
        std::multimap<int64_t,uint64_t> activeEnds;
        std::map<uint64_t,size_t> activeSites;
        for(const auto* s:siteWork){
            while(!activeEnds.empty()&&activeEnds.begin()->first<=s->begin){auto it=activeEnds.begin();auto foundSite=activeSites.find(it->second);if(foundSite!=activeSites.end()&&--foundSite->second==0)activeSites.erase(foundSite);activeEnds.erase(it);}
            if(activeSites.size()>1||(activeSites.size()==1&&activeSites.begin()->first!=s->establishment_id))return false;
            activeEnds.emplace(s->end,s->establishment_id);++activeSites[s->establishment_id];
        }
    }
    if(schedule_book_.timeline_contract){
        // sessions may exist; no extra constraint.
    } else {
        if(!schedule_book_.sessions.empty())return false;
    }
    if(!schedule_book_.timeline_contract&&(!schedule_book_.work_present_ticks.empty()||!schedule_book_.work_present_ticks_by_establishment.empty()||!schedule_book_.work_effort_attention_by_establishment.empty()||!schedule_book_.work_productivity_attention_numerator_by_establishment.empty()))return false;
    for(auto const&[actor,ticks]:schedule_book_.work_present_ticks){
        if(actor==0||actor>people_.size())return false;
        if(ticks<=0||ticks>schedule_book_.cursor)return false;
    }
    std::unordered_map<uint64_t,int64_t> siteAttendanceTotals;
    std::map<WorkRegionKey,int64_t> siteRegionTotals;
    for(auto const&[key,ticks]:schedule_book_.work_present_ticks_by_establishment){
        const auto [actor,establishment]=key;
        if(actor==0||actor>people_.size()||establishment==0||ticks<=0||ticks>schedule_book_.cursor)return false;
        auto site=std::lower_bound(establishments_.begin(),establishments_.end(),establishment,
            [](const Establishment& e,uint64_t id){return e.id<id;});
        if(site==establishments_.end()||site->id!=establishment)return false;
        auto& sum=siteAttendanceTotals[actor];if(ticks>std::numeric_limits<int64_t>::max()-sum)return false;sum+=ticks;
        auto& regionSum=siteRegionTotals[{actor,site->region}];if(ticks>std::numeric_limits<int64_t>::max()-regionSum)return false;regionSum+=ticks;
    }
    for(const auto&[actor,ticks]:siteAttendanceTotals){auto total=schedule_book_.work_present_ticks.find(actor);if(total==schedule_book_.work_present_ticks.end()||ticks>total->second)return false;}
    for(const auto&[key,ticks]:siteRegionTotals){auto regional=schedule_book_.work_present_ticks_by_region.find(key);if(regional==schedule_book_.work_present_ticks_by_region.end()||ticks>regional->second)return false;}
    for(const auto&[key,attention]:schedule_book_.work_effort_attention_by_establishment){
        const auto attended=schedule_book_.work_present_ticks_by_establishment.find(key);
        if(attention<=0||attended==schedule_book_.work_present_ticks_by_establishment.end()||
           attended->second>std::numeric_limits<int64_t>::max()/kAttentionUnitsPerEffortTick||
           attention>attended->second*kAttentionUnitsPerEffortTick)return false;
        const auto [actor,establishment]=key;
        if(actor==0||actor>people_.size())return false;
        auto site=std::lower_bound(establishments_.begin(),establishments_.end(),establishment,
            [](const Establishment& e,uint64_t id){return e.id<id;});
        if(site==establishments_.end()||site->id!=establishment)return false;
    }
    for(const auto&[key,numerator]:schedule_book_.work_productivity_attention_numerator_by_establishment){
        const auto raw=schedule_book_.work_effort_attention_by_establishment.find(key);
        if(raw==schedule_book_.work_effort_attention_by_establishment.end()||raw->second<=0||
           raw->second>std::numeric_limits<int64_t>::max()/250||
           numerator<raw->second*150||numerator>raw->second*250)return false;
        const auto [actor,establishment]=key;
        if(actor==0||actor>people_.size())return false;
        auto site=std::lower_bound(establishments_.begin(),establishments_.end(),establishment,
            [](const Establishment& e,uint64_t id){return e.id<id;});
        if(site==establishments_.end()||site->id!=establishment)return false;
    }
    if(schedule_book_.work_productivity_attention_numerator_by_establishment.size()!=
       schedule_book_.work_effort_attention_by_establishment.size())return false;
    if(schedule_book_.settled_day<0||schedule_book_.settled_day>schedule_book_.cursor/kSubdayUnitsPerDay)return false;
    if(schedule_book_.settled_work_units.size()>people_.size())return false;
    if(!schedule_book_.timeline_contract&&(schedule_book_.settled_day!=0||!schedule_book_.settled_work_units.empty()))return false;
    if(!schedule_book_.settled_work_units.empty()&&schedule_book_.settled_day<=0)return false;
    for(auto const&[actor,units]:schedule_book_.settled_work_units){
        if(actor==0||actor>people_.size())return false;
        if(units<=0||units>schedule_book_.cursor/config_.ticks_per_unit())return false;
        auto attended=schedule_book_.work_present_ticks.find(actor);
        const int64_t ticks=attended==schedule_book_.work_present_ticks.end()?0:attended->second;
        if(units>ticks/config_.ticks_per_unit())return false;
    }
    size_t region_bound=0;
    if(!checked_region_key_bound(people_.size(),regions_,region_bound))return false;
    if(!region_maps_consistent(schedule_book_.work_present_ticks_by_region,
                               schedule_book_.paid_work_units_by_region,
                               schedule_book_.work_present_ticks,
                               schedule_book_.timeline_contract,people_.size(),regions_,
                               schedule_book_.cursor,region_bound,config_.ticks_per_unit()))return false;
    if(work_pay_remainders_.size()>region_bound)return false;
    if(!schedule_book_.timeline_contract&&!work_pay_remainders_.empty())return false;
    for(const auto&[key,remainder]:work_pay_remainders_){
        if(remainder<=0||remainder>=config_.wage_per_unit())return false;
        const auto ticks=schedule_book_.work_present_ticks_by_region.find(key);
        if(ticks==schedule_book_.work_present_ticks_by_region.end())return false;
        const auto paid=schedule_book_.paid_work_units_by_region.find(key);
        const int64_t units=paid==schedule_book_.paid_work_units_by_region.end()?0:paid->second;
        if(units>=ticks->second/config_.ticks_per_unit())return false;
    }
    if(schedule_book_.next_session_id<1)return false;
    for(uint64_t id:seen)if(id>=schedule_book_.next_session_id)return false;
    // Establishment invariants.
    std::set<uint64_t> estSeen;
    for (auto const& e : establishments_) {
        if (e.id == 0) return false;
        if (e.id >= next_id_) return false;
        if (!validRecordId(e.id)) return false;
        if (!estSeen.insert(e.id).second) return false;
        if (e.region < 0 || e.region > regions_) return false;
        if (e.owner_kind != EstablishmentOwnerKind::Person &&
            e.owner_kind != EstablishmentOwnerKind::Public &&
            e.owner_kind != EstablishmentOwnerKind::Organization &&
            e.owner_kind != EstablishmentOwnerKind::Municipal) return false;
        if (e.industry != EstablishmentIndustry::ClothWorkshop &&
            e.industry != EstablishmentIndustry::ToolWorkshop &&
            e.industry != EstablishmentIndustry::MilitarySupplyWorkshop &&
            e.industry != EstablishmentIndustry::Saltworks &&
            e.industry != EstablishmentIndustry::Forestry &&
            e.industry != EstablishmentIndustry::Mining &&
            e.industry != EstablishmentIndustry::PaperMill &&
            e.industry != EstablishmentIndustry::TeaGarden &&
            e.industry != EstablishmentIndustry::IncenseWorks &&
            e.industry != EstablishmentIndustry::WarhorseStable &&
            e.industry != EstablishmentIndustry::WeaponForge &&
            e.industry != EstablishmentIndustry::Glassworks &&
            e.industry != EstablishmentIndustry::PotteryKiln &&
            e.industry != EstablishmentIndustry::Tannery &&
            e.industry != EstablishmentIndustry::MedicinalGarden &&
            e.industry != EstablishmentIndustry::SeedPressWorks &&
            e.industry != EstablishmentIndustry::SawmillWorks) return false;
        if(e.industry==EstablishmentIndustry::Saltworks&&e.region!=8)return false;
        if(e.industry==EstablishmentIndustry::Forestry&&e.region!=6)return false;
        if(e.industry==EstablishmentIndustry::Mining&&e.region!=7&&e.region!=23)return false;
        if(e.industry==EstablishmentIndustry::PaperMill&&e.region!=13)return false;
        if(e.industry==EstablishmentIndustry::TeaGarden&&e.region!=14)return false;
        if(e.industry==EstablishmentIndustry::IncenseWorks&&e.region!=11)return false;
        if(e.industry==EstablishmentIndustry::WarhorseStable&&e.region!=16)return false;
        if(e.industry==EstablishmentIndustry::WeaponForge&&e.region!=19)return false;
        if(e.industry==EstablishmentIndustry::Glassworks&&e.region!=10)return false;
        if(e.industry==EstablishmentIndustry::PotteryKiln&&e.region!=15)return false;
        if(e.industry==EstablishmentIndustry::Tannery&&e.region!=17)return false;
        if(e.industry==EstablishmentIndustry::MedicinalGarden&&e.region!=20)return false;
        if(e.industry==EstablishmentIndustry::SeedPressWorks&&e.region!=2)return false;
        if(e.industry==EstablishmentIndustry::SawmillWorks&&
           (regions_!=int(kAtlasTerritoryCount)-1||e.region!=6))return false;
        if(e.industry==EstablishmentIndustry::SawmillWorks&&e.region!=6)return false;
        const bool sourceIndustry=e.industry==EstablishmentIndustry::Forestry||e.industry==EstablishmentIndustry::Mining||
            e.industry==EstablishmentIndustry::TeaGarden||e.industry==EstablishmentIndustry::IncenseWorks||
            e.industry==EstablishmentIndustry::Glassworks||e.industry==EstablishmentIndustry::PotteryKiln||
            e.industry==EstablishmentIndustry::Tannery||e.industry==EstablishmentIndustry::MedicinalGarden||
            e.industry==EstablishmentIndustry::SeedPressWorks;
        const int expectedSource=establishment_source_good(e.industry,e.region);
        if((sourceIndustry&&(e.source_good!=expectedSource||e.source_units<0||
                            e.source_units>kEstablishmentSourceUnitsMaximum))||
           (!sourceIndustry&&(e.source_good!=kNoEstablishmentSourceGood||e.source_units!=0)))return false;
        if (e.capacity < 1 || e.capacity > kEstablishmentCapacityMax) return false;
        if (e.cash < 0) return false;
        size_t authorizationWindows=0;
        if(e.work_authorizations.size()>people_.size())return false;
        for(const auto& [worker,windows]:e.work_authorizations){
            if(worker==0||worker>people_.size()||windows.empty()||windows.size()>1000000-authorizationWindows)return false;
            authorizationWindows+=windows.size();
            int64_t previousEnd=-1;
            for(const auto& window:windows){
                if(window.begin<0||window.begin<=previousEnd||window.end<=window.begin||window.end>365000LL*kSubdayUnitsPerDay)return false;
                previousEnd=window.end;
            }
        }
        if(authorizationWindows!=e.authorization_window_count)return false;
        for (int g = 0; g < kEstablishmentStockSlots; ++g) if (e.stock[g] < 0) return false;
        // Gate 4/T3.3 production bookkeeping invariants: the day marker stays
        // inside [0, day], and processed units never exceed attended effort
        // derived from delivered attention at the site.
        // legacy timeline-off worlds carry no production bookkeeping at all.
        if (e.last_production_day < 0 || e.last_production_day > day_) return false;
        if (!schedule_book_.timeline_contract && !e.processed_work_units.empty()) return false;
        {
            uint64_t previousWatermarkActor = 0;
            for (const auto& [actor, units] : e.processed_work_units) {
                if (actor == 0 || actor > people_.size() || units <= 0) return false;
                if (actor <= previousWatermarkActor) return false;
                previousWatermarkActor = actor;
                const auto productivity=schedule_book_.work_productivity_attention_numerator_by_establishment.find({actor,e.id});
                const int64_t numerator=productivity==schedule_book_.work_productivity_attention_numerator_by_establishment.end()?0:
                    productivity->second;
                const int64_t eligible=numerator/(kLaborProductivityDenominator*kAttentionUnitsPerEffortTick*config_.ticks_per_unit());
                if (units > eligible) return false;
            }
        }
        // owner reference
        if (e.owner_kind == EstablishmentOwnerKind::Person) {
            if (e.owner_id == 0 || e.owner_id > people_.size()) return false;
        } else if (e.owner_kind == EstablishmentOwnerKind::Public ||
                   e.owner_kind == EstablishmentOwnerKind::Municipal) {
            if (e.owner_id > static_cast<uint64_t>(regions_) || e.owner_id != static_cast<uint64_t>(e.region)) return false;
        } else {
            if (e.owner_id < 1 || e.owner_id > kOrganizationCount ||
                e.region != static_cast<int>((e.owner_id - 1) / kOrganizationGroupsPerTerritory)) return false;
        }
    }
    for(size_t i=0;i<establishments_.size();++i)if(establishments_[i].industry==EstablishmentIndustry::Forestry||
        establishments_[i].industry==EstablishmentIndustry::Mining||
        establishments_[i].industry==EstablishmentIndustry::PaperMill||
        establishments_[i].industry==EstablishmentIndustry::TeaGarden||
        establishments_[i].industry==EstablishmentIndustry::IncenseWorks||
        establishments_[i].industry==EstablishmentIndustry::WarhorseStable||
        establishments_[i].industry==EstablishmentIndustry::WeaponForge||
        establishments_[i].industry==EstablishmentIndustry::Glassworks||
        establishments_[i].industry==EstablishmentIndustry::PotteryKiln||
        establishments_[i].industry==EstablishmentIndustry::Tannery||
        establishments_[i].industry==EstablishmentIndustry::MedicinalGarden||
        establishments_[i].industry==EstablishmentIndustry::SeedPressWorks||
        establishments_[i].industry==EstablishmentIndustry::SawmillWorks)
        for(size_t j=i+1;j<establishments_.size();++j)
            if(establishments_[j].industry==establishments_[i].industry&&establishments_[j].region==establishments_[i].region)return false;
    // sorted
    for (size_t i = 1; i < establishments_.size(); ++i) {
        if (establishments_[i - 1].id >= establishments_[i].id) return false;
    }
    if(personal_establishment_indices_.size()!=people_.size())return false;
    {
        std::vector<std::vector<std::size_t>> expected(people_.size());
        for(std::size_t index=0;index<establishments_.size();++index){
            const auto& site=establishments_[index];
            if(site.owner_kind==EstablishmentOwnerKind::Person)
                expected[std::size_t(site.owner_id-1)].push_back(index);
        }
        if(expected!=personal_establishment_indices_)return false;
    }
    if(!work_credit_consistent())return false;
    return true;
}

void Realm::rebuild_personal_establishment_index(){
    personal_establishment_indices_.assign(people_.size(),{});
    for(std::size_t index=0;index<establishments_.size();++index){
        const auto& site=establishments_[index];
        if(site.owner_kind==EstablishmentOwnerKind::Person&&site.owner_id>0&&site.owner_id<=people_.size())
            personal_establishment_indices_[std::size_t(site.owner_id-1)].push_back(index);
    }
}
double Realm::personal_enterprise_equity(uint64_t actor)const{
    if(actor==0||actor>personal_establishment_indices_.size())return 0.;
    double equity=0.;
    const int64_t wage=config_.wage_per_unit(),ticksPerUnit=config_.ticks_per_unit();
    for(const std::size_t index:personal_establishment_indices_[std::size_t(actor-1)]){
        const auto& site=establishments_[index];
        equity+=double(site.cash);
        for(std::size_t good=0;good<kGoodIdCount;++good)
            equity+=double(site.stock[good])*kGoodReferencePrice[good];
        for(const auto& [worker,processed]:site.processed_work_units){
            (void)processed;
            if(worker==0||worker>people_.size()||!people_[std::size_t(worker-1)].alive)continue;
            const auto attendance=schedule_book_.work_present_ticks_by_establishment.find({worker,site.id});
            if(attendance==schedule_book_.work_present_ticks_by_establishment.end()||attendance->second<=0)continue;
            const int64_t eligible=attendance->second/ticksPerUnit;
            const auto credit=work_credits_.find({worker,site.region});
            if(credit!=work_credits_.end()&&credit->second.bridge_units>0)continue;
            const auto old=site.work_pay.legacy_credit.find(worker);
            const int64_t legacy=old==site.work_pay.legacy_credit.end()?0:old->second;
            const int64_t entitled=std::max<int64_t>(0,eligible-legacy);
            const auto paid=site.work_pay.paid_units.find(worker);
            const int64_t paidUnits=paid==site.work_pay.paid_units.end()?0:paid->second;
            const auto remainder=site.work_pay.remainders.find(worker);
            const int64_t wageRemainder=remainder==site.work_pay.remainders.end()?0:remainder->second;
            if(entitled>paidUnits)
                equity-=double(entitled-paidUnits)*double(wage)-double(wageRemainder);
        }
    }
    return equity;
}
double Realm::utility(uint64_t id)const{auto&p=person(id);auto&v=provinces_[p.region];double wealth=p.cash-debt(id);for(size_t g=0;g<kGoodIdCount;++g)wealth+=p.inventory[g]*kGoodReferencePrice[g];
    const double enterprise=personal_enterprise_equity(id);
    wealth+=enterprise;
    if(p.family==Family::Economic)return (p.alive?0:-20)+wealth/100.+p.health*.05+p.nutrition*.01;
    if(p.family==Family::Governance){double wellbeing=0;int n=0;for(auto&c:people_)if(c.region==p.region){wellbeing+=c.alive?c.health:0;++n;}double reserve_target=std::max(100.,double(n)*3.);double fiscal=5*std::min(1.,v.treasury/reserve_target);return (p.alive?0:-100)+(p.goal==0?2.:1.)*fiscal+(p.goal==1?3.:1.)*wellbeing/std::max(1,n)-v.unrest*.4+std::min(v.road,4)*5+std::min(v.school,std::max(8,n/4))*.1-debt(id)*.01;}
    return (p.alive?0:-100)+p.health*.8+p.nutrition*.2+(p.goal==0?2.:.7)*p.skill+(p.goal==1?.08:.02)*wealth+(p.goal==2?2.:.3)*p.social;
}
std::array<float,realm_obs> Realm::observation(uint64_t id)const{
    std::array<Candidate,realm_actions> candidateSnapshot{};
    return observation(id,candidateSnapshot);
}
std::array<float,realm_obs> Realm::observation(uint64_t id,std::array<Candidate,realm_actions>&candidateSnapshot)const{
    const float regionScale=float(std::max(1,regions_));
    const float targetScale=float(std::max(1,regions_+1));
    std::array<float,realm_obs> o{};auto&p=person(id);auto&v=provinces_[p.region];
    o[0]=float(p.family);o[1]=p.age_days/36500.f;o[2]=p.cash/500.f;o[3]=p.health/100.f;o[4]=p.skill/100.f;o[5]=p.goal/2.f;o[6]=p.region/regionScale;o[7]=day_/7300.f;o[8]=debt(id)/300.f;o[9]=p.social/100.f;
    o[94]=available_funds(id)/500.f;o[97]=p.taxing_authority;o[98]=p.sovereign;
    o[99]=float(v.health_sum)/float(std::max(1,v.residents)*100);o[100]=p.nutrition/100.f;
    o[101]=p.office_appointment.office;o[102]=p.candidate;o[103]=p.sovereign?credit_pool_/1000.f:0;o[104]=p.sovereign?restricted_fund_/1000.f:0;o[105]=report_delay(true)/30.f;
    int active_cases=0;for(auto&i:investigations_)active_cases+=!i.completed&&i.owner==id;o[106]=active_cases/2.f;o[107]=p.sovereign?office_used(id)/5.f:0;
    o[size_t(realm_municipal_fund_observation_index)]=p.family==Family::Governance?
        std::min(1.f,float(municipal_funds_[size_t(p.region)])/10000.f):0.f;
    if(p.family==Family::Economic&&p.region==8&&regions_==int(kAtlasTerritoryCount)-1){
        for(int partner_id=0;partner_id<kForeignTradePartnerCount;++partner_id){
            const auto&partner=foreign_trade_partners_[size_t(partner_id)];
            const int base=partner_id==0?realm_foreign_trade_stock_observation_index:
                realm_foreign_trade_partner_observation_begin+(partner_id-1)*3;
            o[size_t(base)]=float(partner.goods[size_t(GoodId::DryFood)])/float(kForeignTradePartnerOpeningDryFood);
            o[size_t(base+1)]=float(partner.salt_capacity)/float(kForeignTradePartnerSaltCapacity);
            o[size_t(base+2)]=std::min(1.f,float(partner.money)/float(kForeignTradePartnerOpeningCash));
        }
    }
    for(const auto& obligation:military_obligations_)if(obligation.completed_day<0&&
        (obligation.obligor==id||obligation.recipient==id||
         std::find(obligation.member_ids.begin(),obligation.member_ids.end(),id)!=obligation.member_ids.end())){
        const bool member=std::find(obligation.member_ids.begin(),obligation.member_ids.end(),id)!=obligation.member_ids.end();
        const int delivered=int(obligation.delivered_member_ids.size());
        o[size_t(realm_military_obligation_required_observation_index)]=obligation.required_members/4.f;
        o[size_t(realm_military_obligation_delivery_observation_index)]=float(delivered)/float(obligation.required_members);
        o[size_t(realm_military_obligation_days_left_observation_index)]=std::clamp(float(obligation.due_day-day_)/14.f,0.f,1.f);
        o[size_t(realm_military_obligation_overdue_observation_index)]=obligation.due_checked&&obligation.shortage_at_last_deadline>0?1.f:0.f;
        o[size_t(realm_military_obligation_first_shortage_observation_index)]=obligation.first_deadline_checked?obligation.shortage_at_first_deadline/4.f:0.f;
        o[size_t(realm_military_obligation_extension_observation_index)]=obligation.extensions?1.f:0.f;
        o[size_t(realm_military_obligation_recipient_observation_index)]=obligation.recipient==id?1.f:0.f;
        o[size_t(realm_military_obligation_member_observation_index)]=member?1.f:0.f;
        break;
    }
    o[realm_sleep_debt_observation_index]=p.body_state.sleep_debt/100.f;
    o[realm_exertion_observation_index]=p.body_state.exertion/100.f;
    const auto& knowledge_counts=knowledge_observation_counts_[id-1];
    o[realm_knowledge_evidence_observation_index]=knowledge_counts[0]/10.f;
    o[realm_knowledge_legacy_observation_index]=knowledge_counts[1]/10.f;
    for(size_t axis=0;axis<p.personality.values.size();++axis)o[realm_personality_observation_begin+int(axis)]=p.personality.values[axis]/100.f;
    o[95]=float(p.organization)/float(kOrganizationCount);o[96]=p.organization>0?organization_funds_[p.organization-1]/1000.f:0;
    for(size_t i=0;i<kLegacyCapabilities.size();++i)o[10+i]=(capability_active(kLegacyCapabilities[i])?p.stats.get(kLegacyCapabilities[i]):kInactiveCapabilityNeutralValue)/100.f;o[16]=(draft_.chosen+1)/12.f;
    o[17]=v.school_used/float(std::max(1,v.school));o[18]=v.employed/float(std::max(1,v.jobs));
    o[19]=p.family==Family::Governance?v.treasury/10000.f:0;o[20]=v.unrest/100.f;o[21]=v.road/10.f;o[22]=p.alive;o[23]=p.age_days>=18*365;
    for(int r=0;r<=regions_;++r)for(int legacy=0;legacy<kLegacyGoodCount;++legacy){
        const size_t g=size_t(canonical_from_legacy_good(legacy));
        const int index=r<7?24+r*kLegacyGoodCount+legacy:
            realm_base_state+(r-7)*kLegacyGoodCount+legacy;
        o[size_t(index)]=provinces_[r].price[g]/float(kGoodReferencePrice[g]);
    }
    for(int r=0;r<kAtlasTerritoryCount;++r)
        o[size_t(realm_ration_depot_observation_begin+r)]=r<=regions_?
            float(military_ration_depots_[size_t(r)])/1000.f:0.f;
    for(int r=0;r<kAtlasTerritoryCount;++r)
        o[size_t(realm_supplementary_ration_observation_begin+r)]=r<=regions_?
            float(military_supplementary_ration_depots_[size_t(r)])/1000.f:0.f;
    for(int destination=0;destination<kAtlasTerritoryCount;++destination){
        int64_t remaining_total=0;int minimum_days=std::numeric_limits<int>::max();
        for(const auto& obligation:military_supply_obligations_){
            if(obligation.completed_day>=0||obligation.destination_region!=destination||
               (id!=obligation.obligor&&id!=obligation.recipient))continue;
            int64_t delivered=0;
            for(const auto& shipment:transport_shipments_)if(shipment.id>=obligation.first_eligible_shipment_id&&
                shipment.owner_kind==TransportShipmentOwnerKind::MilitaryRationDepot&&shipment.owner==obligation.obligor&&
                shipment.source_region==obligation.source_region&&shipment.destination==destination&&
                shipment.good==int(GoodId::DryFood)&&shipment.delivered<=std::numeric_limits<int64_t>::max()-delivered)
                delivered+=shipment.delivered;
            const int64_t remaining=std::max<int64_t>(0,obligation.required-std::min(delivered,obligation.required));
            if(remaining>std::numeric_limits<int64_t>::max()-remaining_total)remaining_total=std::numeric_limits<int64_t>::max();
            else remaining_total+=remaining;
            minimum_days=std::min(minimum_days,obligation.due_checked?0:std::max(0,obligation.due_day-day_));
        }
        const size_t value_index=size_t(realm_military_supply_obligation_observation_begin+destination*2);
        o[value_index]=std::min(1.f,float(remaining_total)/4.f);
        if(minimum_days!=std::numeric_limits<int>::max())o[value_index+1]=float(minimum_days)/14.f;
    }
    for(int legacy=0;legacy<kLegacyGoodCount;++legacy){const size_t g=size_t(canonical_from_legacy_good(legacy));o[73+legacy]=p.inventory[g]/16.f;o[80+legacy]=v.stock[g]/1000.f;}
    o[87]=secret_known_counts_[id-1]/10.f;
    candidateSnapshot=candidates(id);auto ms=mask(id,candidateSnapshot);
    for(int i=0;i<realm_actions;++i){auto&c=candidateSnapshot[i];auto*f=o.data()+realm_state+i*realm_features;const int good=valid_good_id(c.good)?c.good:0;const size_t g=size_t(good);f[0]=int(c.kind)/32.f;
        f[1]=c.target/targetScale;f[2]=good/float(kGoodIdCount-1);f[3]=c.amount/200.f;f[4]=c.term/180.f;
        f[5]=ms[i];f[6]=c.target==p.region;f[7]=std::abs(c.target-p.region)/regionScale;
        f[8]=v.price[g]/float(kGoodReferencePrice[g]);f[9]=v.stock[g]/1000.f;
        f[10]=p.inventory[g]/16.f;f[11]=v.school/100.f;
        if(c.kind==Intent::BuyMilitaryRations)f[10]=float(military_ration_depot_stock(p.region,c.good))/1000.f;
        if(c.kind==Intent::ShipMilitaryRations){
            int64_t pending=0;
            for(const auto& shipment:transport_shipments_)if(
                shipment.owner_kind==TransportShipmentOwnerKind::MilitaryRationDepot&&
                shipment.destination==c.target&&shipment.good==c.good){
                const int64_t remaining=shipment.quantity-shipment.delivered-shipment.canceled;
                if(remaining>0&&remaining<=std::numeric_limits<int64_t>::max()-pending)pending+=remaining;
            }
            f[10]=float(pending)/1000.f;
        }
        if(c.kind==Intent::Aid&&c.good==1)f[10]=1.f;
        if(c.kind==Intent::Mobilize){int eligible=0;for(const auto&member:people_)
            if(member.military_service.active&&member.military_service.commander==p.id&&
               member.military_service.source_region==p.region)++eligible;
            f[10]=float(eligible)/1000.f;
        }
        if((c.kind==Intent::TransferEstablishmentGoods||c.kind==Intent::AuthorizeWorkAtSite||c.kind==Intent::WorkAtSite)&&c.target>0){
            const auto site=std::lower_bound(establishments_.begin(),establishments_.end(),uint64_t(c.target),
                [](const Establishment& value,uint64_t target){return value.id<target;});
            if(site!=establishments_.end()&&site->id==uint64_t(c.target)){
                f[3]=c.amount/200.f;f[4]=c.term/31.f;f[6]=site->region==p.region;f[7]=std::abs(site->region-p.region)/regionScale;
                f[9]=site->capacity/1000.f;
                if(c.kind==Intent::AuthorizeWorkAtSite&&c.good>0&&c.good<=int(people_.size())){
                    const auto&worker=person(uint64_t(c.good));f[1]=worker.age_days/36500.f;f[2]=int(site->industry)/7.f;
                    f[8]=site->cash/1000.f;f[10]=worker.skill/100.f;const auto authorization=site->work_authorizations.find(uint64_t(c.good));
                    f[11]=authorization!=site->work_authorizations.end()&&!authorization->second.empty();
                }else{
                    f[1]=int(site->industry)/7.f;f[2]=site->region/regionScale;
                    f[8]=(c.kind==Intent::WorkAtSite)?site->cash/1000.f:(valid_good_id(c.good)?site->stock[g]/1000.f:site->cash/1000.f);
                if(site->source_good!=kNoEstablishmentSourceGood)
                    f[8]=float(site->source_units)/float(kEstablishmentSourceUnitsMaximum);
                    f[10]=valid_good_id(c.good)?p.inventory[g]/16.f:0.f;
                    const auto authorization=site->work_authorizations.find(id);
                    f[11]=authorization!=site->work_authorizations.end()&&!authorization->second.empty();
                }
            }
        }
        else if(c.kind==Intent::Marry&&c.target>0&&c.target<=int(people_.size())){const auto&target=person(uint64_t(c.target));f[1]=target.age_days/36500.f;f[2]=target.region/regionScale;f[6]=target.region==p.region;f[7]=std::abs(target.region-p.region)/regionScale;}
        else{f[1]=c.target/targetScale;f[2]=good/float(kGoodIdCount-1);f[6]=c.target==p.region;f[7]=std::abs(c.target-p.region)/regionScale;}
    }
    return o;
}

const Job* describe_job(const Person& person, int current_day) noexcept {
    if (!person.alive) return nullptr;
    // Appointed office takes precedence; requires a valid appointment.
    if (person.office_appointment.office && person.office_appointment.hired_by != 0 && person.family == Family::Governance) {
        return &kAppointedOfficeJob;
    }
    // Active work job: canonical assignment with job_kind exactly 0 or 1.
    if (person.work_assignment.job >= 0 && person.busy_until >= current_day) {
        if (person.work_assignment.job_kind == 0) return &kWorkerJob;
        if (person.work_assignment.job_kind == 1) return &kTeacherJob;
    }
    return nullptr;
}

JobType Realm::job_type(uint64_t id) const {
    if (id < 1 || id > people_.size()) return JobType::None;
    const Job* job = describe_job(people_[id - 1], day_);
    return job ? job->type() : JobType::None;
}

EmploymentState Realm::employment_state(uint64_t id) const {
    return (job_type(id) == JobType::None) ? EmploymentState::Idle : EmploymentState::Employed;
}

const Job* Realm::job_descriptor(uint64_t id) const {
    if (id < 1 || id > people_.size()) return nullptr;
    return describe_job(people_[id - 1], day_);
}

bool Realm::reroll(int slot){if(slot<0||slot>2||draft_.rerolled||draft_.chosen>=0||day_!=0)return false;for(int i=4*draft_.tier;i<4*draft_.tier+4;++i)if(!(draft_.seen&(1<<i))){draft_.cards[slot]=i;draft_.seen|=1<<i;draft_.rerolled=true;return true;}return false;}
bool Realm::choose(int card){if(day_||draft_.chosen>=0||std::find(draft_.cards.begin(),draft_.cards.end(),card)==draft_.cards.end())return false;draft_.chosen=card;
    apply_augment(card);
    event(1,regions_,106,card);return true;
}
std::string Realm::action_name(const Candidate&c){
    if(c.kind==Intent::PrivateResidenceBuild)return "Xây nhà tư trên thửa #"+std::to_string(c.target);
    if(c.kind==Intent::PrivateHousing){
        const char*operation="Thao tác nhà tư";
        switch(static_cast<PrivateHousingOperation>(c.good)){
        case PrivateHousingOperation::Offer:operation="Mời hộ thuê nhà";break;
        case PrivateHousingOperation::Accept:operation="Chấp thuận thuê nhà";break;
        case PrivateHousingOperation::Decline:operation="Từ chối thuê nhà";break;
        case PrivateHousingOperation::EndLease:operation="Kết thúc hợp đồng thuê";break;
        }
        return std::string(operation)+" #"+std::to_string(c.target)+" · hộ #"+std::to_string(c.amount);
    }
    if(c.kind==Intent::Social&&c.good==int(GoodId::Tea))return "Giao lưu cùng trà";
    if(c.kind==Intent::Social&&c.good==int(GoodId::Incense))return "Giao lưu cùng trầm hương";
    if(c.kind==Intent::Social&&c.good==int(GoodId::Silver))return "Giao lưu cùng bạc";
    if(c.kind==Intent::NominateCouncil&&c.target>=0&&c.target<int(kAtlasTerritoryCount)){
        const std::string site(kAtlasUrbanSites[size_t(c.target)].name);
        if(c.good==0)return "Tự ứng cử đại diện hội đồng tại "+site;
        if(c.good==1)return "Rút hồ sơ ứng cử tại "+site;
        if(c.good==2)return "Từ chức đại diện hội đồng tại "+site;
    }
    if(c.kind==Intent::ResolveCouncilNomination&&c.target>=0&&c.target<int(kAtlasTerritoryCount)){
        const std::string site(kAtlasUrbanSites[size_t(c.target)].name);
        return std::string(c.good==1?"Bổ nhiệm người tự ứng cử tại ":"Từ chối hồ sơ ứng cử tại ")+site;
    }
    if(c.kind==Intent::Offer&&c.good==3)return "Đề nghị phục vụ";if(c.kind==Intent::Offer&&c.good==4)return "Đề nghị tập kết quân tình nguyện";if(c.kind==Intent::Offer&&c.good==5)return "Xin gia hạn nghĩa vụ quân sự";if(c.kind==Intent::Offer&&c.good==6)return "Đề nghị cung cấp lương thực quân vụ";if(c.kind==Intent::Offer&&c.good==7)return "Xin gia hạn cung cấp lương thực quân vụ";if(c.kind==Intent::Aid&&c.good==1)return "Cứu trợ từ quỹ đô thị";if(c.kind==Intent::Project&&c.good==4)return "Xây nhà ở công cộng";if(c.kind==Intent::ForeignTrade){const auto operation=static_cast<ForeignTradeOperation>(c.term);const int partner=foreign_trade_partner_id(operation);if(partner<0)return "Giao dịch đối tác không xác định";return std::string(foreign_trade_is_import(operation)?"Nhập từ đối tác ":"Xuất cho đối tác ")+char('A'+partner)+" qua cảng";}static const char* names[]={"Chờ","Mua","Bán","Vận tải","Vay","Trả nợ","Dự án","Tuyển dụng","Thuế","Cứu trợ","Điều tra","Đề nghị","Chấp thuận","Từ chối","Làm việc","Học","Nghỉ","Giao lưu","Tham gia hội","Di cư","Giúp gia đình","Bổ nhiệm","Huy động","Từ chức","Kết hôn","Tạo cơ sở sản xuất","Chuyển hàng xưởng","Cho phép làm tại xưởng","Làm tại xưởng","Mua lương quân vụ","Gửi lương quân vụ","Cấp quỹ công trình đô thị","Ngoại thương"};return names[int(c.kind)];}


} // namespace polyec
