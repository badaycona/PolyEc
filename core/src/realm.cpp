#include "polyec/realm.hpp"
#include "polyec/trade.hpp"
#include "polyec/managers/finance_manager.hpp"
#include "polyec/managers/household_index.hpp"
#include "polyec/managers/project_manager.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <set>
#include <utility>

namespace polyec {
namespace {
bool same(const Candidate&a,const Candidate&b){return a.kind==b.kind&&a.target==b.target&&a.good==b.good&&a.amount==b.amount&&a.term==b.term;}
uint64_t hash_text(const std::string&s){uint64_t h=1469598103934665603ull;for(unsigned char c:s){h^=c;h*=1099511628211ull;}return h;}
uint64_t process_noise(uint64_t x){x+=0x9e3779b97f4a7c15ull;x=(x^(x>>30))*0xbf58476d1ce4e5b9ull;x=(x^(x>>27))*0x94d049bb133111ebull;return x^(x>>31);}
}
Realm::Realm(uint64_t seed,int regions,int people,bool advisor_fixture):regions_(regions),seed_(seed),rng_(seed){
    if((regions!=1&&regions!=2&&regions!=6)||people<128||people>1000000)throw std::invalid_argument("realm profile bounds");
    provinces_.resize(regions+1);
    for(int r=0;r<=regions;++r){routes_[r][(r+1)%(regions+1)]=true;routes_[(r+1)%(regions+1)][r]=true;}
    for(int r=0;r<=regions;++r){auto&p=provinces_[r];for(int g=0;g<7;++g){p.stock[g]=200+int(rng_()%150);opening_[g]+=p.stock[g];}p.jobs=std::max(32,people/(regions+1));p.school=std::max(8,people/(regions+1)/8);}
    people_.reserve(people+1000);
    const int leaders=people>=10000?64:regions+1;
    for(int i=0;i<people;++i){Person p;p.id=next_id_++;p.region=i%(regions+1);p.household=p.id;p.age_days=(i<2*leaders?35:((i%5==0)?8+int(rng_()%8):18+int(rng_()%44)))*365;
        p.family=i<leaders?Family::Governance:(i<2*leaders?Family::Economic:Family::Civilian);
        if(p.family!=Family::Civilian)p.region=i%(regions+1);p.taxing_authority=i<regions+1;p.sovereign=i==0;if(i<regions+1)p.region=i==0?regions:i-1;
        if(advisor_fixture&&i==127){p.family=Family::Governance;p.age_days=35*365;p.taxing_authority=false;p.sovereign=false;}
        p.cash=p.family==Family::Economic?500:100;p.skill=5+int(rng_()%30);p.goal=int(rng_()%3);
        for(auto&v:p.stats.values)v=25+int(rng_()%56);p.stats.values[i%6]=65+int(rng_()%21);
        people_.push_back(p);
        if(i>=2*leaders){uint64_t base=2*leaders+1+uint64_t((i-2*leaders)/4)*4;people_.back().household=base;people_.back().region=int((i-2*leaders)/4)%(regions+1);edges_.push_back({p.id,base,0});if(i>2*leaders)edges_.push_back({p.id,p.id-1,1});}
    }
    reindex_households();
    std::array<int,4> pool{};int roll=int(rng_()%100);draft_.tier=roll<50?0:(roll<85?1:2);
    for(int i=0;i<4;++i)pool[i]=4*draft_.tier+i;std::shuffle(pool.begin(),pool.end(),rng_);
    for(int i=0;i<3;++i){draft_.cards[i]=pool[i];draft_.seen|=1<<pool[i];}
    for(auto&v:provinces_)v.food_capacity=v.stock[0];opening_money_=total_money();refresh_demography();prices();
}
const Person& Realm::person(uint64_t id) const {if(id==0||id>people_.size())throw std::out_of_range("person ID");return people_[id-1];}
Realm Realm::scenario(uint64_t seed,uint64_t actor,int age,int stat,int goal,int budget,int schools,int jobs,int nutrition,int health){
    Realm w(seed,6,256);auto&p=w.mutable_person(actor);
    if(age>100||stat>100||goal>2||budget>10000||schools>1000||jobs>1000||nutrition>100||health>100)throw std::invalid_argument("Scenario fixture bounds");
    if(age>=0)p.age_days=age*365;if(stat>=0)p.stats.values.fill(stat);if(goal>=0)p.goal=goal;if(budget>=0)p.cash=budget;
    if(nutrition>=0)p.nutrition=nutrition;if(health>=0){p.health=health;if(health==0){p.alive=false;w.transfer(p.cash,w.estate_,p.cash);}}
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
bool Realm::households_consistent()const{
    size_t total=0;
    for(const auto&[label,members]:households_){
        if(!label||label>people_.size()||members.empty())return false;
        total+=members.size();
        for(uint64_t id:members)if(!id||id>people_.size()||people_[id-1].household!=label)return false;
    }
    return total==people_.size();
}
uint64_t Realm::guardian_of(const Person& child)const{
    if(child.age_days>=18*365)return 0;
    auto household=households_.find(child.household);
    if(household==households_.end())return 0;
    uint64_t id=0;int oldest=0;
    for(uint64_t member:household->second){
        const auto&adult=people_[member-1];
        if(adult.id==child.id||adult.household!=child.household||!adult.alive)continue;
        if(adult.age_days>=18*365&&adult.age_days>oldest){id=member;oldest=adult.age_days;}
    }
    return id;
}
uint64_t Realm::guardian(uint64_t id)const{return guardian_of(person(id));}
uint64_t Realm::household_authority(uint64_t household)const{
    if(!household||household>people_.size())return 0;
    auto record=households_.find(household);
    if(record==households_.end())return 0;
    const auto&head=people_[household-1];
    if(head.alive&&head.household==household&&head.age_days>=18*365)return household;
    uint64_t id=0;int oldest=0;
    for(uint64_t member:record->second){const auto&adult=people_[member-1];if(adult.alive&&adult.age_days>=18*365&&adult.age_days>oldest){id=member;oldest=adult.age_days;}}
    return id;
}
uint64_t Realm::household_of(uint64_t id)const{return person(id).household;}
std::vector<uint64_t> Realm::household_members(uint64_t household)const{auto record=households_.find(household);return record==households_.end()?std::vector<uint64_t>{}:record->second;}
bool Realm::same_household(uint64_t a,uint64_t b)const{if(!a||!b||a>people_.size()||b>people_.size())return false;return people_[a-1].household==people_[b-1].household;}
size_t Realm::household_records()const{return households_.size();}
int64_t Realm::available_funds(uint64_t id)const{auto&p=person(id);auto parent=guardian_of(p);return p.cash+(parent?person(parent).cash:0);}
void Realm::spend(Person&p,int64_t&to,int64_t amount){auto own=std::min(p.cash,amount);transfer(p.cash,to,own);if(amount>own){auto parent=guardian_of(p);if(!parent)throw std::logic_error("Missing funded guardian");transfer(mutable_person(parent).cash,to,amount-own);}}
int64_t Realm::debt(uint64_t id)const{int64_t result=0;for(auto&l:loans_)if(l.debtor==id)result+=l.principal+l.interest-l.paid;return result;}
void Realm::event(uint64_t actor,int region,int kind,int64_t amount,uint64_t target,bool secret,uint64_t parent){
    Fact f{next_fact_++,parent,actor,target,day_,region,kind,amount,secret};facts_.push_back(f);
    // Local public reporting takes time; secrets require a completed investigation.
    if(facts_.size()>8192){auto cutoff=facts_[4096].id;facts_.erase(facts_.begin(),facts_.begin()+4096);std::erase_if(knowledge_,[cutoff](auto&k){return k.fact<cutoff;});for(auto&[observer,known]:knowledge_index_)std::erase_if(known,[cutoff](auto&item){return item.first<cutoff;});}
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
std::array<Candidate,realm_actions> Realm::candidates(uint64_t id)const{if(schedule_book_.timeline_contract)throw std::runtime_error("Legacy policy access disabled under timeline contract");    auto&p=person(id);std::array<Candidate,realm_actions> c{};
    if(p.family==Family::Economic){
        for(int g=0;g<7;++g){c[1+g]={Intent::Buy,p.region,g,4};c[8+g]={Intent::Sell,p.region,g,4};}
        for(int r=0;r<7;++r)c[15+r]={Intent::Travel,r};
        c[22]={Intent::Borrow,p.region,0,100,180};c[23]={Intent::Repay,p.region,0,std::min<int64_t>(100,debt(id))};
        c[24]={Intent::Project,p.region,0,200,60};c[25]={Intent::Hire,p.region,0,20,30};
        c[26]={Intent::Offer,1+p.region,0,50,30};c[27]={Intent::Project,p.region,1,150,45};c[30]={Intent::Work,p.region,0,0,30};
    }else if(p.family==Family::Governance){
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
        if(p.hired_by)c[31]={Intent::Resign,p.region};c[18]={Intent::Repay,p.region,0,std::min<int64_t>(100,debt(id))};
    }else{
        for(int r=0;r<7;++r){c[1+r]={Intent::Work,r,0,0,30};c[8+r]={Intent::Migrate,r,0,10,5+std::abs(r-p.region)};}
        c[15]={Intent::Study,p.region,0,5,30};c[16]={Intent::Rest,p.region,0,2,7};
        c[17]={Intent::Social,p.region,0,1,7};c[18]={Intent::Join,p.region,0,2,30};
        for(int group=1;group<4;++group)c[20+group]={Intent::Join,p.region,group,2,30};
        auto authority=household_authority(p.household);if(authority)c[19]={Intent::Support,int(authority),0,5};c[20]={Intent::Buy,p.region,0,1};
    }
    for(auto&proposal:proposals_)if(!proposal.resolved&&proposal.to==id&&proposal.due>=day_&&proposal.ready<=day_){c[28]={Intent::Accept,int(proposal.id)};c[29]={Intent::Reject,int(proposal.id)};break;}
    return c;
}
bool Realm::legal(uint64_t id,const Candidate&c,std::string&reason)const{
    if(id==0||id>people_.size()){reason="Unknown actor";return false;}auto&p=person(id);
    if(c.kind==Intent::Hold)return true;
    if(!p.alive){reason="Actor deceased";return false;}
    bool busyBlocks=p.busy_until>day_;
    if(schedule_book_.timeline_contract)switch(c.kind){case Intent::Buy:case Intent::Sell:case Intent::Borrow:case Intent::Repay:case Intent::Tax:case Intent::Aid:case Intent::Investigate:case Intent::Offer:case Intent::Accept:case Intent::Reject:case Intent::Support:case Intent::Hire:case Intent::Project:case Intent::Appoint:case Intent::Resign:busyBlocks=false;break;default:break;}
    if(busyBlocks){reason="Action duration not finished";return false;}
    if(c.amount<0||c.amount>1000000||c.target<0){reason="Invalid terms";return false;}
    if(c.kind==Intent::Accept||c.kind==Intent::Reject){if(legal_proposal_response(id,c))return true;reason="Proposal unavailable";return false;}
    if(c.kind==Intent::Support){if(p.family!=Family::Civilian||p.age_days<6*365||c.target<=0||c.target!=int(household_authority(p.household))||uint64_t(c.target)==id||c.target>int(people_.size())||!person(c.target).alive||p.cash<c.amount){reason="Household transfer unavailable";return false;}return true;}
    if(c.kind==Intent::Offer){
        if(c.good==2){if(!p.sovereign||!pact_||regions_<3)return false;for(auto&o:proposals_)if(o.kind==2&&!o.resolved)return false;return true;}
        if(p.family==Family::Civilian||c.target<=0||uint64_t(c.target)==id||uint64_t(c.target)>people_.size()||c.amount<=0)return false;
        auto source=p.family==Family::Governance?provinces_[p.region].treasury:p.cash;if(source<c.amount)return false;
        if(c.good==1&&(!p.sovereign||draft_.chosen!=2||day_<mediation_ready_))return false;
        for(auto&o:proposals_)if(!o.resolved&&o.from==id&&o.kind==0&&o.mediation==(c.good==1))return false;return true;
    }
    if(c.target>regions_){reason="Outside curriculum geography";return false;}
    auto&v=provinces_[c.target];auto&local=provinces_[p.region];
    auto funds=p.family==Family::Governance?local.treasury:p.cash;
    switch(c.kind){
    case Intent::Buy:if((p.family!=Family::Civilian||p.age_days>=6*365)&&c.target==p.region&&c.good>=0&&c.good<7&&c.amount>0&&local.stock[c.good]>=c.amount&&p.cash>=(local.price[c.good]+1)*c.amount){int64_t load=0;for(auto n:p.inventory)load+=n;if(load+c.amount<=16)return true;}break;
    case Intent::Sell:if(c.target==p.region&&c.good>=0&&c.good<7&&c.amount>0&&p.inventory[c.good]>=c.amount&&local.firm>=std::max(1,local.price[c.good]-1)*c.amount)return true;break;
    case Intent::Travel:case Intent::Migrate:if(c.target!=p.region&&distance(p.region,c.target)>0&&p.cash>=4+2*distance(p.region,c.target)&&p.age_days>=14*365)return true;break;
    case Intent::Borrow:if(c.good==1){if(p.sovereign&&draft_.chosen==4&&c.amount>0&&credit_pool_>=c.amount&&c.term==1825&&debt(id)+c.amount<=royal_reference_/2)return true;break;}if(p.family!=Family::Civilian&&c.target==p.region&&c.amount>0&&local.firm>=c.amount&&debt(id)+c.amount<=300&&c.term>=30&&c.term<=180)return true;break;
    case Intent::Repay:if(c.amount>0&&debt(id)>=c.amount&&funds>=c.amount)return true;break;
    case Intent::Project:{int64_t match=p.sovereign&&c.good<2?std::min(restricted_fund_,c.amount/2):0;
        if(p.family==Family::Civilian||c.target!=p.region||funds<c.amount-match||c.amount<=0||c.term<30||c.term>180||c.good<0||c.good>3||!project_materials(p.region,c.good))break;
        if(c.good==0&&local.road>=4)break;if(c.good==1&&local.school>=std::max(8,int(people_.size())/(regions_+1)/4))break;
        if(c.good>=2&&(local.teachers==0||!p.sovereign))break;
        int active=ProjectManager::active_projects_count(projects_,p.region,c.good);
        if(active<(c.good==3&&draft_.chosen==10?2:1))return true;break;}
    case Intent::Hire:if(p.family==Family::Economic&&c.target==p.region&&p.cash>=c.amount&&c.amount>0)return true;break;
    case Intent::Tax:if(p.family==Family::Governance&&p.taxing_authority&&c.target==p.region&&c.amount>=0&&c.amount<=20&&local.charter)return true;break;
    case Intent::Aid:if(p.family==Family::Governance&&(c.target==p.region||p.sovereign)&&funds>=c.amount&&c.amount>0)return true;break;
    case Intent::Investigate:{int active=0;for(auto&i:investigations_)active+=i.owner==id&&!i.completed;if(p.family==Family::Governance&&c.target==p.region&&funds>=c.amount&&c.amount>0&&active<(p.sovereign&&draft_.chosen==7?2:1))return true;break;}
    case Intent::Work:if(p.family!=Family::Governance&&p.age_days>=14*365&&c.target==p.region&&(v.employed<v.jobs||p.job==p.region)&&c.term>0&&c.term<=30)return true;break;
    case Intent::Study:if(p.family==Family::Civilian&&p.age_days>=6*365&&p.age_days<65*365&&p.skill<100&&c.target==p.region&&v.school_used<v.school&&available_funds(id)>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Rest:if(p.family==Family::Civilian&&c.target==p.region&&p.cash>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Social:if(p.family==Family::Civilian&&p.age_days>=6*365&&c.target==p.region&&p.cash>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Join:if(p.family==Family::Civilian&&p.age_days>=6*365&&c.target==p.region&&c.good>=0&&c.good<4&&p.organization!=1+4*p.region+c.good&&p.cash>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Resign:if(p.hired_by&&day_>=p.office_notice)return true;break;
    case Intent::Appoint:if(p.sovereign&&office_used(id)<office_capacity(id)&&funds>=20){for(auto ref:referrals_)if(person(ref).alive&&person(ref).candidate)return true;}break;
    case Intent::Mobilize:if(p.family==Family::Governance&&c.target==p.region&&funds>=c.amount&&c.term>0&&c.term<=30)return true;break;
    default:break;
    }
    reason="Resources, charter, age, capacity or location prevents action";return false;
}
std::array<float,realm_actions> Realm::mask(uint64_t id)const{if(schedule_book_.timeline_contract)throw std::runtime_error("Legacy policy access disabled under timeline contract");    std::array<float,realm_actions> m{};auto c=candidates(id);for(int i=0;i<realm_actions;++i){std::string why;m[i]=(i==0||c[i].kind!=Intent::Hold)&&legal(id,c[i],why)?1.f:0.f;}return m;
}
RealmResult Realm::execute(const RealmCommand&cmd){
    std::string why;if(cmd.actor==0||cmd.actor>people_.size())return {false,"Unknown actor"};auto&p=mutable_person(cmd.actor);
    if(cmd.sequence!=p.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!legal(cmd.actor,cmd.action,why))return {false,why};
    auto c=cmd.action;auto&v=provinces_[p.region];auto&funds=p.family==Family::Governance?v.treasury:p.cash;
    // Validation above covers every source/capacity before any mutation.
    bool clearsJob=!schedule_book_.timeline_contract||(c.kind==Intent::Travel||c.kind==Intent::Migrate||c.kind==Intent::Work||c.kind==Intent::Study||c.kind==Intent::Rest||c.kind==Intent::Social||c.kind==Intent::Join||c.kind==Intent::Mobilize);
    if(c.kind!=Intent::Hold&&p.job>=0&&clearsJob){v.employed=std::max(0,v.employed-1);if(p.job_kind==1)v.teachers=std::max(0,v.teachers-1);p.job=-1;}
    switch(c.kind){
    case Intent::Hold:break;
    case Intent::Buy:transfer(p.cash,v.firm,(v.price[c.good]+1)*c.amount);sales_tax(v,(v.price[c.good]+1)*c.amount);v.stock[c.good]-=c.amount;p.inventory[c.good]+=c.amount;break;
    case Intent::Sell:transfer(v.firm,p.cash,std::max(1,v.price[c.good]-1)*c.amount);v.stock[c.good]+=c.amount;p.inventory[c.good]-=c.amount;break;
    case Intent::Travel:case Intent::Migrate:{int dist=distance(p.region,c.target);transfer(p.cash,carrier_,4+2*dist);p.destination=c.target;p.busy_until=day_+std::max(1,dist+1-v.road/2);break;}
    case Intent::Borrow:{bool pool=c.good==1;transfer(pool?credit_pool_:v.firm,funds,c.amount);Loan loan{next_id_++,p.id,p.region,day_+c.term,c.amount,(c.amount*(pool?15:4)+99)/100};loan.royal_pool=pool;loan.cause=next_fact_;loans_.push_back(loan);break;}
    case Intent::Repay:{auto left=c.amount;for(auto&l:loans_)if(l.debtor==p.id&&left>0){auto n=std::min(left,l.principal+l.interest-l.paid);transfer(funds,l.royal_pool?credit_pool_:provinces_[l.creditor].firm,n);l.paid+=n;left-=n;}break;}
    case Intent::Project:start_project(p.id,p.region,c.good,c.amount,c.term);break;
    case Intent::Hire:transfer(p.cash,v.firm,c.amount);v.jobs+=1;break;
    case Intent::Tax:v.tax=int(c.amount);break;
    case Intent::Aid:{std::vector<uint64_t> residents;for(auto&citizen:people_)if(citizen.alive&&citizen.family==Family::Civilian&&citizen.region==c.target)residents.push_back(citizen.id);if(residents.empty())transfer(funds,provinces_[c.target].firm,c.amount);else{int64_t share=c.amount/int64_t(residents.size()),remainder=c.amount%int64_t(residents.size());for(auto id:residents)transfer(funds,mutable_person(id).cash,share);size_t start=(day_+p.decisions)%residents.size();for(int64_t i=0;i<remainder;++i)transfer(funds,mutable_person(residents[(start+i)%residents.size()]).cash,1);}provinces_[c.target].unrest=std::max(0,provinces_[c.target].unrest-3);break;}
    case Intent::Investigate:{transfer(funds,v.firm,c.amount);int due=day_+std::max(1,60*(150-p.stats.values[4])/100);investigations_.push_back({next_id_++,p.id,next_fact_,p.region,due});break;}
    case Intent::Offer:create_proposal(p.id,c);break;
    case Intent::Accept:case Intent::Reject:resolve_proposal(p.id,c);break;
    case Intent::Work:p.job=p.region;p.job_kind=(p.age_days>=18*365&&p.skill>=20&&v.teachers<(v.school+19)/20&&v.treasury>=2)?1:0;if(p.job_kind==1)++v.teachers;++v.employed;p.busy_until=day_+c.term;break;
    case Intent::Study:spend(p,v.firm,c.amount);p.school_days=c.term;++v.school_used;p.busy_until=day_+c.term;break;
    case Intent::Rest:{transfer(p.cash,v.firm,c.amount);int before=p.health;p.health=std::min(100,p.health+(p.nutrition>=40?15:0));v.health_sum+=p.health-before;p.busy_until=day_+c.term;break;}
    case Intent::Social:transfer(p.cash,v.firm,c.amount);p.social=std::min(100,p.social+2);p.busy_until=day_+c.term;break;
    case Intent::Join:p.organization=1+4*p.region+c.good;transfer(p.cash,organization_funds_[p.organization-1],c.amount);p.busy_until=day_+c.term;break;
    case Intent::Support:transfer(p.cash,mutable_person(c.target).cash,c.amount);break;
    case Intent::Appoint:for(auto ref:referrals_){auto&a=mutable_person(ref);if(!a.alive||!a.candidate)continue;if(a.job>=0){auto&av=provinces_[a.region];av.employed=std::max(0,av.employed-1);if(a.job_kind==1)av.teachers=std::max(0,av.teachers-1);}a.job=-1;a.school_days=0;a.busy_until=day_;a.candidate=false;a.office=true;a.hired_by=p.id;a.salary_due=day_+30;a.office_notice=day_+30;a.family=Family::Governance;a.taxing_authority=false;transfer(funds,a.cash,20);break;}break;
    case Intent::Resign:p.office=false;p.hired_by=0;p.family=Family::Civilian;break;
    case Intent::Mobilize:transfer(funds,v.firm,c.amount);v.unrest=std::max(0,v.unrest-p.stats.values[5]/10);break;
    }
    p.last_decision_day=day_;++p.decisions;++p.sampling;event(p.id,p.region,int(c.kind),c.amount,uint64_t(c.target));return {true,"Committed",next_fact_-1};
}
RealmResult Realm::execute(const ScheduleCommand&cmd){
    if(cmd.actor==0||cmd.actor>people_.size())return {false,"Unknown actor"};
    auto&p=mutable_person(cmd.actor);
    if(cmd.sequence!=p.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!p.alive)return {false,"Actor deceased"};
    if(next_fact_==UINT64_MAX)return {false,"Fact counter overflow"};
    auto&book=schedule_book_;
    const int64_t maxTick=365000LL*kSubdayUnitsPerDay;
    if(cmd.operation==ScheduleCommand::Operation::Submit){
        if(cmd.begin<book.cursor)return {false,"Processed time"};
        if(cmd.begin>=cmd.end)return {false,"Invalid session range"};
        if(cmd.begin>maxTick||cmd.end>maxTick)return {false,"Session outside world horizon"};
        if(cmd.demand_rate<activity::kMinDemandRate||cmd.demand_rate>activity::kMaxDemandRate)return {false,"Invalid demand rate"};
        if(p.decisions==UINT64_MAX||book.next_session_id==UINT64_MAX||p.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
        uint64_t newId=book.next_session_id;
        std::vector<ActivitySession> staged;
        auto found=book.sessions.find(cmd.actor);
        if(found!=book.sessions.end())staged=found->second;
        ActivitySession session{newId,cmd.actor,cmd.begin,cmd.end,cmd.priority,cmd.demand_rate};
        staged.push_back(session);
        std::vector<activity::SessionView> views;
        for(auto const&s:staged)views.push_back({s.id,s.actor,s.begin,s.end,s.priority,s.demand_rate});
        auto probe=activity::evaluateActivity(cmd.actor,activity::Horizon{cmd.begin,cmd.end},views);
        if(!probe.ok){
            return {false,"Schedule arithmetic overflow"};
        }
        book.next_session_id=newId+1;
        if(found==book.sessions.end())book.sessions.emplace(cmd.actor,std::move(staged));else found->second=std::move(staged);
        book.timeline_contract=true;
        p.last_decision_day=day_;++p.decisions;++p.sampling;
        event(cmd.actor,p.region,200,0,0,false,0);
        uint64_t factId=next_fact_==0?0:next_fact_-1;
        return {true,"Committed",factId,newId};
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
        it->second=std::move(staged);
        if(it->second.empty())book.sessions.erase(cmd.actor);
        p.last_decision_day=day_;++p.decisions;++p.sampling;
        event(cmd.actor,p.region,201,0,cmd.session,false,0);
        uint64_t factId=next_fact_==0?0:next_fact_-1;
        return {true,"Committed",factId,cmd.session};
    }
    if(cmd.operation==ScheduleCommand::Operation::Reprioritize){
        if(p.decisions==UINT64_MAX||p.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
        std::vector<ActivitySession> staged=it->second;
        for(auto&s:staged)if(s.id==cmd.session)s.priority=cmd.priority;
        std::vector<activity::SessionView> views;
        for(auto const&s:staged)views.push_back({s.id,s.actor,s.begin,s.end,s.priority,s.demand_rate});
        auto probe=activity::evaluateActivity(cmd.actor,activity::Horizon{sit->begin,sit->end},views);
        if(!probe.ok)return {false,"Schedule arithmetic overflow"};
        it->second=std::move(staged);
        p.last_decision_day=day_;++p.decisions;++p.sampling;
        event(cmd.actor,p.region,202,0,cmd.session,false,0);
        uint64_t factId=next_fact_==0?0:next_fact_-1;
        return {true,"Committed",factId,cmd.session};
    }
    return {false,"Unsupported schedule operation"};
}
RealmResult Realm::execute(const RealmClockCommand&cmd){
    if(cmd.operation!=RealmClockCommand::Operation::Pause&&cmd.operation!=RealmClockCommand::Operation::Resume)return {false,"Unknown clock operation"};
    if(cmd.expected_pause_revision.has_value()&&*cmd.expected_pause_revision!=pause_revision_)return {false,"Stale clock revision"};    if(cmd.operation==RealmClockCommand::Operation::Pause){
        if(paused_)return {true,"Committed"};
        if(pause_revision_==UINT64_MAX)return {false,"Pause revision overflow"};
        paused_=true;++pause_revision_;return {true,"Committed"};
    }
    if(!paused_)return {true,"Committed"};
    if(pause_revision_==UINT64_MAX)return {false,"Pause revision overflow"};
    paused_=false;++pause_revision_;return {true,"Committed"};
}
int Realm::distance(int from,int to)const{
    if(from<0||from>regions_||to<0||to>regions_)return -1;
    std::array<int,7>d;d.fill(100);d[from]=0;
    for(int k=0;k<=regions_;++k)for(int a=0;a<=regions_;++a)for(int b=0;b<=regions_;++b)if(routes_[a][b])d[b]=std::min(d[b],d[a]+1);
    return d[to]>=100?-1:d[to];
}
RealmResult Realm::route_command(uint64_t actor,int from,int to,bool open){
    if(!actor||actor>people_.size()||from<0||to<0||from>regions_||to>regions_||from==to)return {false,"Route bounds"};
    auto&p=person(actor);if(!p.alive||p.family!=Family::Governance||(!p.sovereign&&p.region!=from))return {false,"Route jurisdiction"};
    if((from+1)%(regions_+1)!=to&&(to+1)%(regions_+1)!=from)return {false,"No constructed route"};
    routes_[from][to]=routes_[to][from]=open;event(actor,from,107,open,to);return {true,"Route changed",next_fact_-1};
}
void Realm::prices(){for(int r=0;r<=regions_;++r)for(int g=0;g<7;++g){auto&v=provinces_[r];v.price[g]=std::clamp(int(kReference[g]*(1.5-double(v.stock[g])/800.0)),2,5*kReference[g]);}}
void Realm::payroll(int region,bool public_employer,const std::vector<std::pair<uint64_t,int64_t>>&claims){
    FinanceManager::payroll(day_,region,public_employer,claims,people_,provinces_);
}
void Realm::sales_tax(Province&v,int64_t gross){int64_t assessed=gross*v.tax+v.sales_tax_remainder;int64_t amount=assessed/100;v.sales_tax_remainder=int(assessed%100);transfer(v.firm,v.treasury,amount);v.tax_receipts+=amount;}
int64_t Realm::harvest_quantity(int region,int good,int residents,int weather,int productivity)const{
    if(weather<0)weather=reference_scenario_?100:75+int(process_noise(seed_^uint64_t(day_/90)*7411)%51)+int(process_noise(seed_^uint64_t(day_/90)*19937^uint64_t(region/2+1)*431)%11)-5;
    return (int64_t(residents)*15+30)*(good==region%7?2:1)*weather*productivity/10000;
}
void Realm::daily(){
    ++day_;
    for(auto&v:provinces_) {v.employed=0;v.school_used=0;v.teachers=0;v.teacher_skill_sum=0;}
    std::array<std::vector<std::pair<uint64_t,int64_t>>,7> private_claims,public_claims;
    for(auto&p:people_)if(p.alive){
        if(p.job>=0&&p.busy_until>=day_){if(p.job_kind==1)public_claims[p.region].push_back({p.id,2});else private_claims[p.region].push_back({p.id,1+p.skill/25});}
        if(!p.hired_by&&day_%30==0&&p.family!=Family::Civilian){auto&claims=p.family==Family::Governance?public_claims[p.region]:private_claims[p.region];claims.push_back({p.id,20});}
    }
    for(int r=0;r<=regions_;++r){payroll(r,false,private_claims[r]);payroll(r,true,public_claims[r]);}
    for(auto&p:people_)if(p.alive){++p.age_days;
        if(p.hired_by&&p.salary_due<=day_){auto&employer=person(p.hired_by);auto&source=provinces_[employer.region].treasury;if(source>=20){transfer(source,p.cash,20);p.salary_due=day_+30;}else {p.hired_by=0;p.office=false;p.family=Family::Civilian;event(p.id,p.region,110);}}
        if(p.destination>=0&&p.busy_until<=day_){p.region=p.destination;p.destination=-1;p.job=-1;event(p.id,p.region,int(Intent::Migrate));}
        auto&v=provinces_[p.region];
        if(p.job>=0&&p.busy_until>=day_){++v.employed;if(p.job_kind==1){++v.teachers;v.teacher_skill_sum+=p.skill;}
            else {int g=p.region%7;int n=1+p.skill/40;v.stock[g]+=n;produced_[g]+=n;}
        }else p.job=-1;
        if(p.school_days>0){++v.school_used;--p.school_days;if(p.school_days==0){p.skill=std::min(100,p.skill+5+p.stats.values[0]/20+(v.teachers?v.teacher_skill_sum/std::max(1,v.teachers)/25:0));event(p.id,p.region,int(Intent::Study),p.skill);}}
        // Essentials are a biological process, not a strategic purchase selector.
        if(day_%7==int(p.id%7)){if(p.inventory[0]>0){--p.inventory[0];++consumed_[0];p.nutrition=std::min(100,p.nutrition+20);p.health=std::min(100,p.health+1);}
            else if(v.stock[0]>0&&available_funds(p.id)>=v.price[0]){spend(p,v.firm,v.price[0]);sales_tax(v,v.price[0]);--v.stock[0];++consumed_[0];p.nutrition=std::min(100,p.nutrition+20);p.health=std::min(100,p.health+1);}
            else if(v.stock[0]>0&&p.organization>0&&organization_funds_[p.organization-1]>=v.price[0]){transfer(organization_funds_[p.organization-1],v.firm,v.price[0]);sales_tax(v,v.price[0]);--v.stock[0];++consumed_[0];p.nutrition=std::min(100,p.nutrition+20);p.health=std::min(100,p.health+1);}else {p.nutrition=std::max(0,p.nutrition-20);p.health-=2;}}
        if(p.health<=0||(!reference_scenario_&&p.age_days>75*365&&rng_()%3650==0)){p.alive=false;transfer(p.cash,estate_,p.cash);for(int g=0;g<7;++g){v.stock[g]+=p.inventory[g];p.inventory[g]=0;}event(p.id,p.region,100,p.age_days);}
        if(p.age_days==18*365)event(p.id,p.region,101);
    }
    if(day_%30==0){for(int r=0;r<=regions_;++r){auto&v=provinces_[r];int workers=0;for(auto&p:people_)if(p.alive&&p.region==r&&p.job>=0&&p.job_kind==0)++workers;
        for(int g:{2,5}){int input=g==2?1:3;int n=std::min<int64_t>({int64_t(2+workers/4),v.stock[input],v.stock[4]});v.stock[input]-=n;v.stock[4]-=n;consumed_[input]+=n;consumed_[4]+=n;v.stock[g]+=n;produced_[g]+=n;}
        if(v.stock[0]==0){v.unrest=std::min(100,v.unrest+5);event(1+r,r,102,workers,0,true);}else v.unrest=std::max(0,v.unrest-1);
    }}
    if(day_%90==0){for(int r=0;r<=regions_;++r){auto&v=provinces_[r];int residents=0;for(auto&p:people_)if(p.alive&&p.region==r)++residents;
        // Correlated regional season; labor and geography affect harvest.
        for(int g:{0,1,3,4,6}){auto n=harvest_quantity(r,g,residents);v.stock[g]+=n;produced_[g]+=n;}}
    }
    finish_projects();
    for(auto&i:investigations_)if(!i.completed&&i.due<=day_){i.completed=true;for(auto&f:facts_)if(f.region==i.region&&f.secret&&f.day<=i.due){knowledge_.push_back({i.owner,f.id,day_});knowledge_index_[i.owner][f.id]=day_;}event(i.owner,i.region,109,0,i.id,false,i.cause);}
    if(day_%30==0){auto&v=provinces_[person(1).region];event(1,person(1).region,108,v.stock[0]);if(draft_.chosen==3){int64_t spoiled=std::max<int64_t>(0,v.stock[0]-v.food_capacity);spoiled+=std::min(v.stock[0]-spoiled,v.stock[0]/1000);v.stock[0]-=spoiled;consumed_[0]+=spoiled;if(v.treasury>0)transfer(v.treasury,v.firm,1);}}
    for(auto&l:loans_)if(!l.defaulted&&l.due<=day_&&l.paid<l.principal+l.interest){l.defaulted=true;event(l.debtor,person(l.debtor).region,103,l.principal+l.interest-l.paid,l.id);}
    for(auto&o:proposals_)if(!o.resolved&&o.due<day_){refund_proposal(o);event(o.from,person(o.from).region,104,o.amount,o.to);}
    // Births conserve estate money and preserve independent IDs/relationships.
    if(!reference_scenario_&&day_%365==0&&people_.size()<1000000){size_t n=people_.size();for(size_t i=0;i<n;++i){auto&parent=people_[i];if(!parent.alive||parent.health<60||parent.nutrition<60||parent.age_days<20*365||parent.age_days>45*365||parent.cash<50||rng_()%100>=6)continue;
        Person child;child.id=people_.size()+1;child.region=parent.region;child.household=parent.household;child.age_days=0;child.cash=0;child.goal=int(rng_()%3);transfer(parent.cash,child.cash,20);people_.push_back(child);index_household(child.household,child.id);edges_.push_back({child.id,uint64_t(i+1),0});event(child.id,child.region,105,0,uint64_t(i+1));}}
    refresh_demography();prices();
}
void Realm::refresh_demography(){for(auto&v:provinces_){v.residents=0;v.health_sum=0;}for(auto&p:people_){auto&v=provinces_[p.region];++v.residents;v.health_sum+=p.alive?p.health:0;}}
void Realm::advance(int days){if(days<0||days>36500)throw std::invalid_argument("days bounds");advance_ticks(static_cast<int64_t>(days)*kSubdayUnitsPerDay);}
void Realm::advance_ticks(int64_t ticks){
    if(ticks<0)throw std::invalid_argument("ticks bounds");
    if(ticks>0 && paused_)throw std::runtime_error("Realm is paused");
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
        // integrate attention for each live actor into staged sessions.
        for(auto&[actor,sessions]:staged.sessions){
            if(actor==0||actor>people_.size()||!person(actor).alive)continue;
            std::vector<activity::SessionView> views;
            for(auto const&s:sessions)if(!s.retired&&s.begin<staged.cursor+chunk&&s.end>staged.cursor)views.push_back({s.id,s.actor,s.begin,s.end,s.priority,s.demand_rate});
            if(views.empty())continue;
            auto result=activity::evaluateActivity(actor,activity::Horizon{staged.cursor,staged.cursor+chunk},views);
            if(!result.ok)throw std::runtime_error("Schedule attention overflow");
            for(auto const&r:result.results){
                for(auto&s:sessions)if(s.id==r.id){s.requested+=r.requested;s.delivered+=r.delivered;s.unmet+=r.unmet;s.capacity_unmet+=r.capacity_unmet;s.unresolved_unmet+=r.unresolved_unmet;}
            }
        }
        // commit staged book and cursor.
        schedule_book_=std::move(staged);
        schedule_book_.cursor+=chunk;
        if(schedule_book_.cursor%kSubdayUnitsPerDay==0)daily();
    }
}
std::array<int64_t,9> Realm::balances()const{std::array<int64_t,9>b{};for(auto&p:people_)b[0]+=p.cash;for(auto&p:provinces_){b[1]+=p.firm;b[2]+=p.treasury;}b[3]=carrier_;b[4]=estate_;for(auto f:organization_funds_)b[5]+=f;b[6]=credit_pool_;b[7]=restricted_fund_;for(auto&o:proposals_)b[8]+=o.escrow;return b;}
int64_t Realm::total_money()const{int64_t n=carrier_+estate_+credit_pool_+restricted_fund_;for(auto&o:proposals_)n+=o.escrow;for(auto&p:people_)n+=p.cash;for(auto&p:provinces_)n+=p.firm+p.treasury;for(auto funds:organization_funds_)n+=funds;return n;}
bool Realm::invariant(const std::array<int64_t,7>& bonded_goods)const{
    if(total_money()!=opening_money_+endowment_||provinces_.size()!=size_t(regions_+1))return false;for(auto funds:organization_funds_)if(funds<0)return false;
    auto stock=bonded_goods;for(auto n:stock)if(n<0)return false;for(size_t i=0;i<people_.size();++i){auto&p=people_[i];if(p.id!=i+1||p.nutrition<0||p.nutrition>100||p.job_kind<0||p.job_kind>1||p.organization<0||p.organization>28||p.tax_remainder<0||p.tax_remainder>=100||p.cash<0||p.region<0||p.region>regions_||p.health>100||p.household==0||p.household>people_.size())return false;for(int g=0;g<7;++g){if(p.inventory[g]<0)return false;stock[g]+=p.inventory[g];}}
    for(auto&p:provinces_){if(p.firm<0||p.treasury<0||p.sales_tax_remainder<0||p.sales_tax_remainder>=100)return false;for(int g=0;g<7;++g){if(p.stock[g]<0)return false;stock[g]+=p.stock[g];}}
    for(auto&o:proposals_){if(o.escrow<0)return false;if(o.kind==2&&!o.resolved)for(bool pledged:o.pledged)if(pledged){stock[3]+=4;stock[5]+=2;}}if(credit_pool_<0||restricted_fund_<0)return false;
    for(int g=0;g<7;++g)if(stock[g]+consumed_[g]!=opening_[g]+produced_[g])return false;
    for(auto&l:loans_)if(l.principal<=0||l.principal>1000000||l.interest<0||l.paid<0||l.paid>l.principal+l.interest||l.debtor==0||l.debtor>people_.size()||l.creditor<0||l.creditor>regions_||l.due<0)return false;
    for(auto&e:edges_)if(!e.from||!e.to||e.from>people_.size()||e.to>people_.size())return false;
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
    }
    if(schedule_book_.timeline_contract){
        // sessions may exist; no extra constraint.
    } else {
        if(!schedule_book_.sessions.empty())return false;
    }
    if(schedule_book_.next_session_id<1)return false;
    for(uint64_t id:seen)if(id>=schedule_book_.next_session_id)return false;
    return true;
}
RealmView Realm::view(uint64_t observer)const{RealmView v;v.day=day_;v.regions=regions_;v.persons=people_.size();v.money=total_money();v.provinces=provinces_;v.draft=draft_;for(auto&p:people_){v.alive+=p.alive;v.children+=p.alive&&p.age_days<18*365;v.decisions+=p.decisions;}for(auto&f:facts_)if(knows(observer,f))v.facts.push_back(f);return v;}
double Realm::utility(uint64_t id)const{auto&p=person(id);auto&v=provinces_[p.region];double wealth=p.cash-debt(id);for(int g=0;g<7;++g)wealth+=p.inventory[g]*kReference[g];
    if(p.family==Family::Economic)return (p.alive?0:-20)+wealth/100.+p.health*.05+p.nutrition*.01;
    if(p.family==Family::Governance){double wellbeing=0;int n=0;for(auto&c:people_)if(c.region==p.region){wellbeing+=c.alive?c.health:0;++n;}double reserve_target=std::max(100.,double(n)*3.);double fiscal=5*std::min(1.,v.treasury/reserve_target);return (p.alive?0:-100)+(p.goal==0?2.:1.)*fiscal+(p.goal==1?3.:1.)*wellbeing/std::max(1,n)-v.unrest*.4+std::min(v.road,4)*5+std::min(v.school,std::max(8,n/4))*.1-debt(id)*.01;}
    return (p.alive?0:-100)+p.health*.8+p.nutrition*.2+(p.goal==0?2.:.7)*p.skill+(p.goal==1?.08:.02)*wealth+(p.goal==2?2.:.3)*p.social;
}
std::array<float,realm_obs> Realm::observation(uint64_t id)const{if(schedule_book_.timeline_contract)throw std::runtime_error("Legacy policy access disabled under timeline contract");    std::array<float,realm_obs> o{};auto&p=person(id);auto&v=provinces_[p.region];
    o[0]=float(p.family);o[1]=p.age_days/36500.f;o[2]=p.cash/500.f;o[3]=p.health/100.f;o[4]=p.skill/100.f;o[5]=p.goal/2.f;o[6]=p.region/6.f;o[7]=day_/7300.f;o[8]=debt(id)/300.f;o[9]=p.social/100.f;
    o[94]=available_funds(id)/500.f;o[97]=p.taxing_authority;o[98]=p.sovereign;
    o[99]=float(v.health_sum)/float(std::max(1,v.residents)*100);o[100]=p.nutrition/100.f;
    o[101]=p.office;o[102]=p.candidate;o[103]=p.sovereign?credit_pool_/1000.f:0;o[104]=p.sovereign?restricted_fund_/1000.f:0;o[105]=report_delay(true)/30.f;
    int active_cases=0;for(auto&i:investigations_)active_cases+=!i.completed&&i.owner==id;o[106]=active_cases/2.f;o[107]=p.sovereign?office_used(id)/5.f:0;
    o[95]=p.organization/28.f;o[96]=p.organization>0?organization_funds_[p.organization-1]/1000.f:0;
    for(int i=0;i<6;++i)o[10+i]=p.stats.values[i]/100.f;o[16]=(draft_.chosen+1)/12.f;
    o[17]=v.school_used/float(std::max(1,v.school));o[18]=v.employed/float(std::max(1,v.jobs));
    o[19]=p.family==Family::Governance?v.treasury/10000.f:0;o[20]=v.unrest/100.f;o[21]=v.road/10.f;o[22]=p.alive;o[23]=p.age_days>=18*365;
    for(int r=0;r<=regions_;++r)for(int g=0;g<7;++g)o[24+r*7+g]=provinces_[r].price[g]/float(kReference[g]);
    for(int g=0;g<7;++g){o[73+g]=p.inventory[g]/16.f;o[80+g]=v.stock[g]/1000.f;}
    int known=0;for(auto&f:facts_)if(f.secret&&knows(id,f))++known;o[87]=known/10.f;
    auto cs=candidates(id);auto ms=mask(id);
    for(int i=0;i<realm_actions;++i){auto&c=cs[i];auto*f=o.data()+realm_state+i*realm_features;f[0]=int(c.kind)/24.f;f[1]=c.target/7.f;f[2]=c.good/7.f;f[3]=c.amount/200.f;f[4]=c.term/180.f;f[5]=ms[i];f[6]=c.target==p.region;f[7]=std::abs(c.target-p.region)/6.f;f[8]=v.price[std::clamp(c.good,0,6)]/float(kReference[std::clamp(c.good,0,6)]);f[9]=v.stock[std::clamp(c.good,0,6)]/1000.f;f[10]=p.inventory[std::clamp(c.good,0,6)]/16.f;f[11]=v.school/100.f;}
    return o;
}
bool Realm::reroll(int slot){if(slot<0||slot>2||draft_.rerolled||draft_.chosen>=0||day_!=0)return false;for(int i=4*draft_.tier;i<4*draft_.tier+4;++i)if(!(draft_.seen&(1<<i))){draft_.cards[slot]=i;draft_.seen|=1<<i;draft_.rerolled=true;return true;}return false;}
bool Realm::choose(int card){if(day_||draft_.chosen>=0||std::find(draft_.cards.begin(),draft_.cards.end(),card)==draft_.cards.end())return false;draft_.chosen=card;
    apply_augment(card);
    event(1,regions_,106,card);return true;
}
std::string Realm::action_name(const Candidate&c){static const char* names[]={"Chờ","Mua","Bán","Vận tải","Vay","Trả nợ","Dự án","Tuyển dụng","Thuế","Cứu trợ","Điều tra","Đề nghị","Chấp thuận","Từ chối","Làm việc","Học","Nghỉ","Giao lưu","Tham gia hội","Di cư","Giúp gia đình","Bổ nhiệm","Huy động","Từ chức"};return names[int(c.kind)];}

// Text format uses explicit scalar fields, not compiler-dependent struct bytes.
std::string Realm::serialize(const std::string&bundle)const{
    std::ostringstream s;s<<"POLYEC_REALM_7 "<<std::quoted(bundle)<<' '<<day_<<' '<<regions_<<' '<<seed_<<' '<<next_id_<<' '<<next_fact_<<' '<<opening_money_<<' '<<carrier_<<' '<<estate_<<' '<<endowment_<<' ';
    s<<draft_.tier<<' '<<draft_.seen<<' '<<draft_.chosen<<' '<<draft_.rerolled<<' ';for(auto v:draft_.cards)s<<v<<' ';for(auto&a:{opening_,produced_,consumed_})for(auto v:a)s<<v<<' ';
    for(auto&r:routes_)for(bool open:r)s<<open<<' ';for(auto funds:organization_funds_)s<<funds<<' ';s<<rng_<<' '<<people_.size()<<' ';for(auto&p:people_){s<<p.id<<' '<<p.household<<' '<<p.decisions<<' '<<p.sampling<<' '<<p.region<<' '<<p.age_days<<' '<<p.health<<' '<<p.skill<<' '<<p.job<<' '<<p.busy_until<<' '<<p.destination<<' '<<p.goal<<' '<<p.school_days<<' '<<p.social<<' '<<p.alive<<' '<<int(p.family)<<' '<<p.cash<<' '<<p.tax_remainder<<' '<<p.organization<<' '<<p.job_kind<<' '<<p.taxing_authority<<' '<<p.sovereign<<' '<<p.nutrition<<' ';for(auto v:p.stats.values)s<<v<<' ';for(auto v:p.inventory)s<<v<<' ';}
    s<<provinces_.size()<<' ';for(auto&p:provinces_){s<<p.treasury<<' '<<p.firm<<' '<<p.tax<<' '<<p.school<<' '<<p.school_used<<' '<<p.jobs<<' '<<p.employed<<' '<<p.road<<' '<<p.unrest<<' '<<p.teachers<<' '<<p.charter<<' '<<p.sales_tax_remainder<<' ';for(auto v:p.stock)s<<v<<' ';for(auto v:p.price)s<<v<<' ';}
    s<<edges_.size()<<' ';for(auto&e:edges_)s<<e.from<<' '<<e.to<<' '<<e.kind<<' ';
    s<<loans_.size()<<' ';for(auto&l:loans_)s<<l.id<<' '<<l.debtor<<' '<<l.creditor<<' '<<l.due<<' '<<l.principal<<' '<<l.interest<<' '<<l.paid<<' '<<l.defaulted<<' ';
    s<<projects_.size()<<' ';for(auto&j:projects_)s<<j.id<<' '<<j.owner<<' '<<j.region<<' '<<j.due<<' '<<j.kind<<' '<<j.budget<<' '<<j.completed<<' ';
    s<<facts_.size()<<' ';for(auto&f:facts_)s<<f.id<<' '<<f.parent<<' '<<f.actor<<' '<<f.target<<' '<<f.day<<' '<<f.region<<' '<<f.kind<<' '<<f.amount<<' '<<f.secret<<' ';
    s<<knowledge_.size()<<' ';for(auto&k:knowledge_)s<<k.observer<<' '<<k.fact<<' '<<k.arrives<<' ';
    s<<proposals_.size()<<' ';for(auto&o:proposals_)s<<o.id<<' '<<o.from<<' '<<o.to<<' '<<o.amount<<' '<<o.due<<' '<<o.resolved<<' ';write_court(s);
    std::vector<ActivitySession> allSessions;for(auto const&[actor,sessions]:schedule_book_.sessions)for(auto const&s:sessions)allSessions.push_back(s);std::sort(allSessions.begin(),allSessions.end(),[](const ActivitySession&a,const ActivitySession&b){return a.id<b.id;});
    s<<" CLOCK_1 "<<(paused_?1:0)<<' '<<pause_revision_<<' ';
    s<<" SCHEDULE_1 "<<(schedule_book_.timeline_contract?1:0)<<' '<<schedule_book_.cursor<<' '<<schedule_book_.next_session_id<<' '<<allSessions.size()<<' ';
    for(auto const&session:allSessions)s<<session.id<<' '<<session.actor<<' '<<session.begin<<' '<<session.end<<' '<<session.priority<<' '<<session.demand_rate<<' '<<session.requested<<' '<<session.delivered<<' '<<session.unmet<<' '<<session.capacity_unmet<<' '<<session.unresolved_unmet<<' '<<session.retired<<' '<<session.retired_tick<<' '<<session.retired_cause<<' ';
    return s.str();
}
bool Realm::deserialize(const std::string&data,const std::string&bundle,std::string&error,const std::array<int64_t,7>& bonded_goods){try{
    if(data.size()>256*1024*1024)throw std::runtime_error("Save size limit");Realm w(1,1,128);std::istringstream s(data);std::string magic,b;s>>magic>>std::quoted(b);if(magic!="POLYEC_REALM_7"||b!=bundle)throw std::runtime_error("Save schema/model bundle mismatch");
    s>>w.day_>>w.regions_>>w.seed_>>w.next_id_>>w.next_fact_>>w.opening_money_>>w.carrier_>>w.estate_>>w.endowment_;if(!s||(w.regions_!=1&&w.regions_!=2&&w.regions_!=6))throw std::runtime_error("Save geography");
    s>>w.draft_.tier>>w.draft_.seen>>w.draft_.chosen>>w.draft_.rerolled;for(auto&v:w.draft_.cards)s>>v;for(auto*a:{&w.opening_,&w.produced_,&w.consumed_})for(auto&v:*a)s>>v;for(auto&r:w.routes_)for(auto&open:r)s>>open;for(auto&funds:w.organization_funds_)s>>funds;s>>w.rng_;
    auto size=[&](size_t max){size_t n=0;s>>n;if(!s||n>max)throw std::runtime_error("Save count limit");return n;};
    w.people_.resize(size(1000000));for(auto&p:w.people_){int family=-1;s>>p.id>>p.household>>p.decisions>>p.sampling>>p.region>>p.age_days>>p.health>>p.skill>>p.job>>p.busy_until>>p.destination>>p.goal>>p.school_days>>p.social>>p.alive>>family>>p.cash>>p.tax_remainder>>p.organization>>p.job_kind>>p.taxing_authority>>p.sovereign>>p.nutrition;if(p.region<0||p.region>w.regions_)throw std::runtime_error("Save person geography");if(family<0||family>2)throw std::runtime_error("Save role");if(p.household==0||p.household>w.people_.size())throw std::runtime_error("Save person household");p.family=Family(family);for(auto&v:p.stats.values)s>>v;for(auto&v:p.inventory)s>>v;}
    w.reindex_households();
    w.provinces_.resize(size(7));for(auto&p:w.provinces_){s>>p.treasury>>p.firm>>p.tax>>p.school>>p.school_used>>p.jobs>>p.employed>>p.road>>p.unrest>>p.teachers>>p.charter>>p.sales_tax_remainder;for(auto&v:p.stock)s>>v;for(auto&v:p.price)s>>v;}
    w.edges_.resize(size(8000000));for(auto&e:w.edges_)s>>e.from>>e.to>>e.kind;
    w.loans_.resize(size(1000000));for(auto&l:w.loans_)s>>l.id>>l.debtor>>l.creditor>>l.due>>l.principal>>l.interest>>l.paid>>l.defaulted;
    w.projects_.resize(size(1000000));for(auto&j:w.projects_)s>>j.id>>j.owner>>j.region>>j.due>>j.kind>>j.budget>>j.completed;
    w.facts_.resize(size(8192));for(auto&f:w.facts_)s>>f.id>>f.parent>>f.actor>>f.target>>f.day>>f.region>>f.kind>>f.amount>>f.secret;
    w.knowledge_.resize(size(1000000));for(auto&k:w.knowledge_){s>>k.observer>>k.fact>>k.arrives;auto&known=w.knowledge_index_[k.observer];auto previous=known.find(k.fact);if(previous==known.end()||previous->second>k.arrives)known[k.fact]=k.arrives;}
    w.proposals_.resize(size(1000000));for(auto&o:w.proposals_)s>>o.id>>o.from>>o.to>>o.amount>>o.due>>o.resolved;
    if(!w.read_court(s))throw std::runtime_error("Save court fields");
    std::string clockTag;int pausedFlag=0;uint64_t pauseRevision=0;s>>clockTag>>pausedFlag>>pauseRevision;if(!s||clockTag!="CLOCK_1"||(pausedFlag!=0&&pausedFlag!=1))throw std::runtime_error("Save clock header");w.paused_=pausedFlag==1;w.pause_revision_=pauseRevision;
    std::string scheduleTag;int timelineFlag=0;int64_t cursor=0;uint64_t nextSession=1;size_t sessionCount=0;s>>scheduleTag>>timelineFlag>>cursor>>nextSession>>sessionCount;if(!s||scheduleTag!="SCHEDULE_1"||(timelineFlag!=0&&timelineFlag!=1)||cursor<0||nextSession<1)throw std::runtime_error("Save schedule header");
    if(sessionCount>1000000)throw std::runtime_error("Save schedule count limit");
    const int64_t maxTick=365000LL*kSubdayUnitsPerDay;
    if(cursor>maxTick)throw std::runtime_error("Save schedule cursor");
    if(w.day_!=static_cast<int>(cursor/kSubdayUnitsPerDay))throw std::runtime_error("Save schedule day mismatch");
    std::vector<ActivitySession> loaded;loaded.reserve(sessionCount);std::set<uint64_t> seenIds;
    for(size_t i=0;i<sessionCount;++i){ActivitySession sess{};int retired=0;s>>sess.id>>sess.actor>>sess.begin>>sess.end>>sess.priority>>sess.demand_rate>>sess.requested>>sess.delivered>>sess.unmet>>sess.capacity_unmet>>sess.unresolved_unmet>>retired>>sess.retired_tick>>sess.retired_cause;if(!s)throw std::runtime_error("Save schedule fields");if(sess.id==0||seenIds.count(sess.id))throw std::runtime_error("Save schedule id");seenIds.insert(sess.id);if(sess.actor==0||sess.actor>w.people_.size())throw std::runtime_error("Save schedule actor");if(sess.begin<0||sess.begin>=sess.end||sess.end>maxTick)throw std::runtime_error("Save schedule range");if(sess.demand_rate<activity::kMinDemandRate||sess.demand_rate>activity::kMaxDemandRate)throw std::runtime_error("Save schedule rate");if(sess.requested<0||sess.delivered<0||sess.unmet<0||sess.capacity_unmet<0||sess.unresolved_unmet<0)throw std::runtime_error("Save schedule totals");if(sess.delivered>sess.requested)throw std::runtime_error("Save schedule totals");if(sess.unmet!=sess.requested-sess.delivered)throw std::runtime_error("Save schedule total mismatch");if(sess.capacity_unmet>sess.unmet)throw std::runtime_error("Save schedule totals");if(sess.unresolved_unmet!=sess.unmet-sess.capacity_unmet)throw std::runtime_error("Save schedule reason mismatch");if(retired!=0&&retired!=1)throw std::runtime_error("Save schedule retired");sess.retired=retired==1;if(sess.retired_tick<0||sess.retired_tick>cursor)throw std::runtime_error("Save schedule retired tick");if(sess.retired){if(sess.retired_cause==0)throw std::runtime_error("Save schedule retired cause");}else{if(sess.retired_tick!=0||sess.retired_cause!=0)throw std::runtime_error("Save schedule retired fields");}
        int64_t processedEnd=sess.retired?sess.retired_tick:cursor;if(processedEnd>sess.end)processedEnd=sess.end;if(processedEnd<sess.begin)processedEnd=sess.begin;int64_t expected=sess.demand_rate*(processedEnd-sess.begin);if(expected!=sess.requested)throw std::runtime_error("Save schedule processed mismatch");if(sess.delivered>sess.requested)throw std::runtime_error("Save schedule delivered");loaded.push_back(sess);}
    if(!seenIds.empty()){
        uint64_t maxId=*std::max_element(seenIds.begin(),seenIds.end());
        if(nextSession<=maxId)throw std::runtime_error("Save schedule next id");
    }
    if(timelineFlag==0&&!loaded.empty())throw std::runtime_error("Save schedule timeline flag");
    w.schedule_book_.cursor=cursor;
    w.schedule_book_.timeline_contract=timelineFlag==1;
    w.schedule_book_.next_session_id=nextSession;
    w.schedule_book_.sessions.clear();
    for(auto const&s:loaded)w.schedule_book_.sessions[s.actor].push_back(s);
    for(auto&[actor,sessions]:w.schedule_book_.sessions)std::sort(sessions.begin(),sessions.end(),[](const ActivitySession&a,const ActivitySession&b){return a.id<b.id;});
    if(w.provinces_.size()!=size_t(w.regions_+1))throw std::runtime_error("Save province count");w.refresh_demography();s>>std::ws;if(!s.eof()||s.fail()&&!s.eof()||w.people_.size()<128||w.day_<0||w.day_>365000||!w.invariant(bonded_goods))throw std::runtime_error("Corrupt save/invariant");*this=std::move(w);error.clear();return true;
}catch(const std::exception&e){error=e.what();return false;}}
bool Realm::save(const std::string&path,const std::string&bundle,std::string&error)const{try{auto dest=std::filesystem::u8path(path),temp=dest;temp+=".tmp";auto data=serialize(bundle);{std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<hash_text(data)<<'\n'<<data;out.flush();if(!out)throw std::runtime_error("Save write failed");}auto backup=dest;backup+=".bak";if(std::filesystem::exists(dest))std::filesystem::copy_file(dest,backup,std::filesystem::copy_options::overwrite_existing);std::filesystem::rename(temp,dest);error.clear();return true;}catch(const std::exception&e){error=e.what();return false;}}
bool Realm::load(const std::string&path,const std::string&bundle,std::string&error){try{std::ifstream in(std::filesystem::u8path(path),std::ios::binary);uint64_t checksum;in>>checksum;in.get();std::string data((std::istreambuf_iterator<char>(in)),{});if(!in||hash_text(data)!=checksum)throw std::runtime_error("Save checksum/read failed");return deserialize(data,bundle,error);}catch(const std::exception&e){error=e.what();return false;}}
uint64_t Realm::fingerprint()const{return hash_text(serialize("fingerprint"));}
}
