#include "polyec/realm.hpp"
#include "polyec/trade.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace polyec {
namespace {
bool same(const Candidate&a,const Candidate&b){return a.kind==b.kind&&a.target==b.target&&a.good==b.good&&a.amount==b.amount&&a.term==b.term;}
uint64_t hash_text(const std::string&s){uint64_t h=1469598103934665603ull;for(unsigned char c:s){h^=c;h*=1099511628211ull;}return h;}
}
Realm::Realm(uint64_t seed,int regions,int people):regions_(regions),seed_(seed),rng_(seed){
    if((regions!=1&&regions!=2&&regions!=6)||people<128||people>1000000)throw std::invalid_argument("realm profile bounds");
    provinces_.resize(regions+1);
    for(int r=0;r<=regions;++r){routes_[r][(r+1)%(regions+1)]=true;routes_[(r+1)%(regions+1)][r]=true;}
    for(int r=0;r<=regions;++r){auto&p=provinces_[r];for(int g=0;g<7;++g){p.stock[g]=200+int(rng_()%150);opening_[g]+=p.stock[g];}p.jobs=std::max(32,people/(regions+1));p.school=std::max(8,people/(regions+1)/8);}
    people_.reserve(people+1000);
    const int leaders=people>=10000?64:regions+1;
    for(int i=0;i<people;++i){Person p;p.id=next_id_++;p.region=i%(regions+1);p.household=p.id;p.age_days=(i<2*leaders?35:((i%5==0)?8+int(rng_()%8):18+int(rng_()%44)))*365;
        p.family=i<leaders?Family::Governance:(i<2*leaders?Family::Economic:Family::Civilian);
        if(p.family!=Family::Civilian)p.region=i%(regions+1);p.taxing_authority=i<regions+1;p.sovereign=i==0;
        p.cash=p.family==Family::Economic?500:100;p.skill=5+int(rng_()%30);p.goal=int(rng_()%3);
        for(auto&v:p.stats.values)v=25+int(rng_()%56);p.stats.values[i%6]=65+int(rng_()%21);
        people_.push_back(p);
        if(i>=2*leaders){uint64_t base=2*leaders+1+uint64_t((i-2*leaders)/4)*4;people_.back().household=base;edges_.push_back({p.id,base,0});if(i>2*leaders)edges_.push_back({p.id,p.id-1,1});}
    }
    std::array<int,4> pool{};int roll=int(rng_()%100);draft_.tier=roll<50?0:(roll<85?1:2);
    for(int i=0;i<4;++i)pool[i]=4*draft_.tier+i;std::shuffle(pool.begin(),pool.end(),rng_);
    for(int i=0;i<3;++i){draft_.cards[i]=pool[i];draft_.seen|=1<<pool[i];}
    opening_money_=total_money();prices();
}
const Person& Realm::person(uint64_t id) const {if(id==0||id>people_.size())throw std::out_of_range("person ID");return people_[id-1];}
Realm Realm::scenario(uint64_t seed,uint64_t actor,int age,int stat,int goal,int budget,int schools,int jobs){
    Realm w(seed,6,256);auto&p=w.mutable_person(actor);
    if(age>100||stat>100||goal>2||budget>10000||schools>1000||jobs>1000)throw std::invalid_argument("Scenario fixture bounds");
    if(age>=0)p.age_days=age*365;if(stat>=0)p.stats.values.fill(stat);if(goal>=0)p.goal=goal;if(budget>=0)p.cash=budget;
    if(schools>=0)w.provinces_[p.region].school=schools;if(jobs>=0)w.provinces_[p.region].jobs=jobs;
    w.opening_money_=w.total_money();return w;
}
Person& Realm::mutable_person(uint64_t id){return const_cast<Person&>(std::as_const(*this).person(id));}
void Realm::transfer(int64_t&from,int64_t&to,int64_t amount){if(amount<0||from<amount)throw std::logic_error("unfunded transfer");from-=amount;to+=amount;}
uint64_t Realm::guardian(const Person&p)const{if(p.age_days>=18*365)return 0;uint64_t id=0;int oldest=0;for(uint64_t candidate=p.household;candidate<p.household+4&&candidate<=people_.size();++candidate){auto&adult=person(candidate);if(adult.id!=p.id&&adult.alive&&adult.household==p.household&&adult.age_days>=18*365&&adult.age_days>oldest){id=candidate;oldest=adult.age_days;}}return id;}
int64_t Realm::available_funds(uint64_t id)const{auto&p=person(id);auto parent=guardian(p);return p.cash+(parent?person(parent).cash:0);}
void Realm::spend(Person&p,int64_t&to,int64_t amount){auto own=std::min(p.cash,amount);transfer(p.cash,to,own);if(amount>own){auto parent=guardian(p);if(!parent)throw std::logic_error("Missing funded guardian");transfer(mutable_person(parent).cash,to,amount-own);}}
int64_t Realm::debt(uint64_t id)const{int64_t result=0;for(auto&l:loans_)if(l.debtor==id)result+=l.principal+l.interest-l.paid;return result;}
void Realm::event(uint64_t actor,int region,int kind,int64_t amount,uint64_t target,bool secret,uint64_t parent){
    Fact f{next_fact_++,parent,actor,target,day_,region,kind,amount,secret};facts_.push_back(f);
    // Local public reporting takes time; secrets require a completed investigation.
    if(facts_.size()>8192){auto cutoff=facts_[4096].id;facts_.erase(facts_.begin(),facts_.begin()+4096);std::erase_if(knowledge_,[cutoff](auto&k){return k.fact<cutoff;});for(auto&[observer,known]:knowledge_index_)std::erase_if(known,[cutoff](auto&item){return item.first<cutoff;});}
}
bool Realm::knows(uint64_t observer,const Fact&f)const{
    if(f.actor==observer||f.target==observer)return true;
    if(!f.secret){if(observer==1&&f.day+(draft_.chosen==7?2:4)<=day_)return true;return f.region==person(observer).region&&f.day<day_;}
    auto observer_it=knowledge_index_.find(observer);if(observer_it!=knowledge_index_.end()){auto fact_it=observer_it->second.find(f.id);if(fact_it!=observer_it->second.end()&&fact_it->second<=day_)return true;}
    return false;
}
std::vector<uint64_t> Realm::due(Family family)const{
    std::vector<uint64_t> ids;int cadence=family==Family::Economic?7:30;
    for(auto&p:people_)if(p.alive&&p.family==family&&p.busy_until<=day_&&day_%cadence==int(p.id%cadence))ids.push_back(p.id);return ids;
}
std::array<Candidate,realm_actions> Realm::candidates(uint64_t id)const{
    auto&p=person(id);std::array<Candidate,realm_actions> c{};
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
        c[17]={Intent::Borrow,p.region,0,100,180};c[18]={Intent::Repay,p.region,0,std::min<int64_t>(100,debt(id))};
    }else{
        for(int r=0;r<7;++r){c[1+r]={Intent::Work,r,0,0,30};c[8+r]={Intent::Migrate,r,0,10,5+std::abs(r-p.region)};}
        c[15]={Intent::Study,p.region,0,5,draft_.chosen==1?26:30};c[16]={Intent::Rest,p.region,0,2,7};
        c[17]={Intent::Social,p.region,0,1,7};c[18]={Intent::Join,p.region,0,2,30};
        for(int group=1;group<4;++group)c[20+group]={Intent::Join,p.region,group,2,30};
        c[19]={Intent::Support,int(p.household),0,5};c[20]={Intent::Buy,p.region,0,1};
    }
    for(auto&proposal:proposals_)if(!proposal.resolved&&proposal.to==id&&proposal.due>=day_){c[28]={Intent::Accept,int(proposal.id)};c[29]={Intent::Reject,int(proposal.id)};break;}
    return c;
}
bool Realm::legal(uint64_t id,const Candidate&c,std::string&reason)const{
    if(id==0||id>people_.size()){reason="Unknown actor";return false;}auto&p=person(id);
    if(c.kind==Intent::Hold)return true;
    if(!p.alive){reason="Actor deceased";return false;}if(p.busy_until>day_){reason="Action duration not finished";return false;}
    if(c.amount<0||c.amount>1000000||c.target<0){reason="Invalid terms";return false;}
    if(c.kind==Intent::Accept||c.kind==Intent::Reject){for(auto&o:proposals_)if(o.id==uint64_t(c.target)&&o.to==id&&!o.resolved&&o.due>=day_){auto&from=person(o.from);auto source=from.family==Family::Governance?provinces_[from.region].treasury:from.cash;if(c.kind==Intent::Reject||source>=o.amount)return true;}reason="Proposal unavailable";return false;}
    if(c.kind==Intent::Support){if(p.family!=Family::Civilian||p.age_days<6*365||c.target!=int(p.household)||uint64_t(c.target)==id||c.target>int(people_.size())||!person(c.target).alive||p.cash<c.amount){reason="Household transfer unavailable";return false;}return true;}
    if(c.kind==Intent::Offer){if(p.family!=Family::Civilian&&c.target>0&&uint64_t(c.target)!=id&&uint64_t(c.target)<=people_.size()&&p.cash>=c.amount&&c.amount>0){for(auto&o:proposals_)if(!o.resolved&&o.from==id)return false;return true;}reason="Proposal unavailable";return false;}
    if(c.target>regions_){reason="Outside curriculum geography";return false;}
    auto&v=provinces_[c.target];auto&local=provinces_[p.region];
    auto funds=p.family==Family::Governance?local.treasury:p.cash;
    switch(c.kind){
    case Intent::Buy:if((p.family!=Family::Civilian||p.age_days>=6*365)&&c.target==p.region&&c.good>=0&&c.good<7&&c.amount>0&&local.stock[c.good]>=c.amount&&p.cash>=(local.price[c.good]+1)*c.amount){int64_t load=0;for(auto n:p.inventory)load+=n;if(load+c.amount<=16)return true;}break;
    case Intent::Sell:if(c.target==p.region&&c.good>=0&&c.good<7&&c.amount>0&&p.inventory[c.good]>=c.amount&&local.firm>=std::max(1,local.price[c.good]-1)*c.amount)return true;break;
    case Intent::Travel:case Intent::Migrate:if(c.target!=p.region&&distance(p.region,c.target)>0&&p.cash>=4+2*distance(p.region,c.target)&&p.age_days>=14*365)return true;break;
    case Intent::Borrow:if(p.family!=Family::Civilian&&c.target==p.region&&c.amount>0&&local.firm>=c.amount&&debt(id)+c.amount<=300&&c.term>=30&&c.term<=180)return true;break;
    case Intent::Repay:if(c.amount>0&&debt(id)>=c.amount&&funds>=c.amount)return true;break;
    case Intent::Project:if(p.family!=Family::Civilian&&c.target==p.region&&funds>=c.amount&&c.amount>0&&c.term>=30&&c.term<=180&&local.stock[3]>=4&&local.stock[5]>=2&&((c.good==0&&local.road<4)||(c.good==1&&local.school<std::max(8,int(people_.size())/(regions_+1)/4)))){for(auto&j:projects_)if(!j.completed&&j.region==p.region&&j.kind==c.good){reason="Project already active";return false;}return true;}break;
    case Intent::Hire:if(p.family==Family::Economic&&c.target==p.region&&p.cash>=c.amount&&c.amount>0)return true;break;
    case Intent::Tax:if(p.family==Family::Governance&&p.taxing_authority&&c.target==p.region&&c.amount>=0&&c.amount<=20&&local.charter)return true;break;
    case Intent::Aid:if(p.family==Family::Governance&&(c.target==p.region||p.sovereign)&&funds>=c.amount&&c.amount>0)return true;break;
    case Intent::Investigate:if(p.family==Family::Governance&&c.target==p.region&&funds>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Work:if(p.family!=Family::Governance&&p.age_days>=14*365&&c.target==p.region&&(v.employed<v.jobs||p.job==p.region)&&c.term>0&&c.term<=30)return true;break;
    case Intent::Study:if(p.family==Family::Civilian&&p.age_days>=6*365&&p.age_days<65*365&&p.skill<100&&c.target==p.region&&v.school_used<v.school&&available_funds(id)>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Rest:if(p.family==Family::Civilian&&c.target==p.region&&p.cash>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Social:if(p.family==Family::Civilian&&p.age_days>=6*365&&c.target==p.region&&p.cash>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Join:if(p.family==Family::Civilian&&p.age_days>=6*365&&c.target==p.region&&c.good>=0&&c.good<4&&p.organization!=1+4*p.region+c.good&&p.cash>=c.amount&&c.term>0&&c.term<=30)return true;break;
    case Intent::Appoint:case Intent::Mobilize:if(p.family==Family::Governance&&c.target==p.region&&funds>=c.amount&&c.term>0&&c.term<=30)return true;break;
    default:break;
    }
    reason="Resources, charter, age, capacity or location prevents action";return false;
}
std::array<float,realm_actions> Realm::mask(uint64_t id)const{
    std::array<float,realm_actions> m{};auto c=candidates(id);for(int i=0;i<realm_actions;++i){std::string why;m[i]=(i==0||c[i].kind!=Intent::Hold)&&legal(id,c[i],why)?1.f:0.f;}return m;
}
RealmResult Realm::execute(const RealmCommand&cmd){
    std::string why;if(cmd.actor==0||cmd.actor>people_.size())return {false,"Unknown actor"};auto&p=mutable_person(cmd.actor);
    if(cmd.sequence!=p.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!legal(cmd.actor,cmd.action,why))return {false,why};
    auto c=cmd.action;auto&v=provinces_[p.region];auto&funds=p.family==Family::Governance?v.treasury:p.cash;
    // Validation above covers every source/capacity before any mutation.
    if(c.kind!=Intent::Hold&&p.job>=0){v.employed=std::max(0,v.employed-1);if(p.job_kind==1)v.teachers=std::max(0,v.teachers-1);p.job=-1;}
    switch(c.kind){
    case Intent::Hold:break;
    case Intent::Buy:transfer(p.cash,v.firm,(v.price[c.good]+1)*c.amount);v.stock[c.good]-=c.amount;p.inventory[c.good]+=c.amount;break;
    case Intent::Sell:transfer(v.firm,p.cash,std::max(1,v.price[c.good]-1)*c.amount);v.stock[c.good]+=c.amount;p.inventory[c.good]-=c.amount;break;
    case Intent::Travel:case Intent::Migrate:{int dist=distance(p.region,c.target);transfer(p.cash,carrier_,4+2*dist);p.destination=c.target;p.busy_until=day_+std::max(1,dist+1-v.road/2);break;}
    case Intent::Borrow:transfer(v.firm,funds,c.amount);loans_.push_back({next_id_++,p.id,p.region,day_+c.term,c.amount,(c.amount*(draft_.chosen==4?3:4)+99)/100});break;
    case Intent::Repay:{auto left=c.amount;for(auto&l:loans_)if(l.debtor==p.id&&left>0){auto n=std::min(left,l.principal+l.interest-l.paid);transfer(funds,provinces_[l.creditor].firm,n);l.paid+=n;left-=n;}break;}
    case Intent::Project:transfer(funds,v.firm,c.amount);v.stock[3]-=4;v.stock[5]-=2;consumed_[3]+=4;consumed_[5]+=2;projects_.push_back({next_id_++,p.id,p.region,day_+std::max(1,c.term*100/(100+p.stats.values[1]/5+(draft_.chosen==5?20:0))),c.good,c.amount});break;
    case Intent::Hire:transfer(p.cash,v.firm,c.amount);v.jobs+=1;break;
    case Intent::Tax:v.tax=int(c.amount);break;
    case Intent::Aid:{std::vector<uint64_t> residents;for(auto&citizen:people_)if(citizen.alive&&citizen.family==Family::Civilian&&citizen.region==c.target)residents.push_back(citizen.id);if(residents.empty())transfer(funds,provinces_[c.target].firm,c.amount);else{int64_t share=c.amount/int64_t(residents.size()),remainder=c.amount%int64_t(residents.size());for(auto id:residents)transfer(funds,mutable_person(id).cash,share);size_t start=(day_+p.decisions)%residents.size();for(int64_t i=0;i<remainder;++i)transfer(funds,mutable_person(residents[(start+i)%residents.size()]).cash,1);}provinces_[c.target].unrest=std::max(0,provinces_[c.target].unrest-3);break;}
    case Intent::Investigate:transfer(funds,v.firm,c.amount);for(auto&f:facts_)if(f.region==p.region&&f.secret){int arrives=day_+std::max(1,c.term*(150-p.stats.values[4])/100);auto&known=knowledge_index_[p.id];auto previous=known.find(f.id);if(previous==known.end()||previous->second>arrives){knowledge_.push_back({p.id,f.id,arrives});known[f.id]=arrives;}}p.busy_until=day_+c.term;break;
    case Intent::Offer:proposals_.push_back({next_id_++,p.id,uint64_t(c.target),c.amount,day_+c.term});break;
    case Intent::Accept:case Intent::Reject:for(auto&o:proposals_)if(o.id==uint64_t(c.target)){if(c.kind==Intent::Accept){auto&from=mutable_person(o.from);auto&source=from.family==Family::Governance?provinces_[from.region].treasury:from.cash;transfer(source,p.cash,o.amount);edges_.push_back({o.from,p.id,2});}o.resolved=true;}break;
    case Intent::Work:p.job=p.region;p.job_kind=(p.age_days>=18*365&&p.skill>=20&&v.teachers<(v.school+19)/20&&v.treasury>=2)?1:0;if(p.job_kind==1)++v.teachers;++v.employed;p.busy_until=day_+c.term;break;
    case Intent::Study:spend(p,v.firm,c.amount);p.school_days=c.term;++v.school_used;p.busy_until=day_+c.term;break;
    case Intent::Rest:transfer(p.cash,v.firm,c.amount);p.health=std::min(100,p.health+15);p.busy_until=day_+c.term;break;
    case Intent::Social:transfer(p.cash,v.firm,c.amount);p.social=std::min(100,p.social+2);p.busy_until=day_+c.term;break;
    case Intent::Join:p.organization=1+4*p.region+c.good;transfer(p.cash,organization_funds_[p.organization-1],c.amount);p.busy_until=day_+c.term;break;
    case Intent::Support:transfer(p.cash,mutable_person(c.target).cash,c.amount);break;
    case Intent::Appoint:transfer(funds,v.firm,c.amount);v.school+=1;break;
    case Intent::Mobilize:transfer(funds,v.firm,c.amount);v.unrest=std::max(0,v.unrest-p.stats.values[5]/10);break;
    }
    ++p.decisions;++p.sampling;event(p.id,p.region,int(c.kind),c.amount,uint64_t(c.target));return {true,"Committed",next_fact_-1};
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
    if(claims.empty())return;auto&v=provinces_[region];auto&account=public_employer?v.treasury:v.firm;
    int64_t due=0;for(auto&claim:claims)due+=claim.second;int64_t budget=std::min(account,due),left=budget;
    std::vector<int64_t> paid(claims.size());
    for(size_t i=0;i<claims.size();++i){paid[i]=budget*claims[i].second/due;left-=paid[i];}
    // Public pro-rata clearing with rotating rounding, never permanent ID priority.
    for(size_t offset=0;left>0&&offset<claims.size();++offset){size_t i=(day_+offset)%claims.size();if(paid[i]<claims[i].second){++paid[i];--left;}}
    for(size_t i=0;i<claims.size();++i){auto&p=mutable_person(claims[i].first);transfer(account,p.cash,paid[i]);int64_t assessed=paid[i]*v.tax+p.tax_remainder;int64_t tax=assessed/100;p.tax_remainder=int(assessed%100);transfer(p.cash,v.treasury,tax);}
}
void Realm::daily(){
    ++day_;
    for(auto&v:provinces_) {v.employed=0;v.school_used=0;v.teachers=0;}
    std::array<std::vector<std::pair<uint64_t,int64_t>>,7> private_claims,public_claims;
    for(auto&p:people_)if(p.alive){
        if(p.job>=0&&p.busy_until>=day_){if(p.job_kind==1)public_claims[p.region].push_back({p.id,2});else private_claims[p.region].push_back({p.id,1+p.skill/25});}
        if(day_%30==0&&p.family!=Family::Civilian){auto&claims=p.family==Family::Governance?public_claims[p.region]:private_claims[p.region];claims.push_back({p.id,20});}
    }
    for(int r=0;r<=regions_;++r){payroll(r,false,private_claims[r]);payroll(r,true,public_claims[r]);}
    for(auto&p:people_)if(p.alive){++p.age_days;
        if(p.destination>=0&&p.busy_until<=day_){p.region=p.destination;p.destination=-1;p.job=-1;event(p.id,p.region,int(Intent::Migrate));}
        auto&v=provinces_[p.region];
        if(p.job>=0&&p.busy_until>=day_){++v.employed;if(p.job_kind==1)++v.teachers;
            else {int g=p.region%7;int n=1+p.skill/40;v.stock[g]+=n;produced_[g]+=n;}
        }else p.job=-1;
        if(p.school_days>0){++v.school_used;--p.school_days;if(p.school_days==0){p.skill=std::min(100,p.skill+5+p.stats.values[0]/20);event(p.id,p.region,int(Intent::Study),p.skill);}}
        // Essentials are a biological process, not a strategic purchase selector.
        if(day_%7==int(p.id%7)){if(p.inventory[0]>0){--p.inventory[0];++consumed_[0];p.health=std::min(100,p.health+1);}
            else if(v.stock[0]>0&&available_funds(p.id)>=v.price[0]){spend(p,v.firm,v.price[0]);--v.stock[0];++consumed_[0];p.health=std::min(100,p.health+1);}
            else if(v.stock[0]>0&&p.organization>0&&organization_funds_[p.organization-1]>=v.price[0]){transfer(organization_funds_[p.organization-1],v.firm,v.price[0]);--v.stock[0];++consumed_[0];p.health=std::min(100,p.health+1);}else p.health-=2;}
        if(p.health<=0||(p.age_days>75*365&&rng_()%3650==0)){p.alive=false;transfer(p.cash,estate_,p.cash);for(int g=0;g<7;++g){v.stock[g]+=p.inventory[g];p.inventory[g]=0;}event(p.id,p.region,100,p.age_days);}
        if(p.age_days==18*365)event(p.id,p.region,101);
    }
    if(day_%30==0){for(int r=0;r<=regions_;++r){auto&v=provinces_[r];int workers=0;for(auto&p:people_)if(p.alive&&p.region==r&&p.job>=0&&p.job_kind==0)++workers;
        for(int g:{2,5}){int input=g==2?1:3;int n=std::min<int64_t>({int64_t(2+workers/4),v.stock[input],v.stock[4]});v.stock[input]-=n;v.stock[4]-=n;consumed_[input]+=n;consumed_[4]+=n;v.stock[g]+=n;produced_[g]+=n;}
        if(v.stock[0]==0){v.unrest=std::min(100,v.unrest+5);event(1+r,r,102,workers,0,true);}else v.unrest=std::max(0,v.unrest-1);
    }}
    if(day_%90==0){for(int r=0;r<=regions_;++r){auto&v=provinces_[r];int residents=0;for(auto&p:people_)if(p.alive&&p.region==r)++residents;
        // Correlated regional season; labor and geography affect harvest.
        int weather=75+int(rng_()%51);for(int g:{0,1,3,4,6}){int n=(residents*15+30)*(g==r%7?2:1)*weather/100;v.stock[g]+=n;produced_[g]+=n;}}
    }
    for(auto&j:projects_)if(!j.completed&&j.due<=day_){j.completed=true;if(j.kind==0)++provinces_[j.region].road;else provinces_[j.region].school+=4;event(j.owner,j.region,int(Intent::Project),j.budget,j.id);}
    for(auto&l:loans_)if(!l.defaulted&&l.due<=day_&&l.paid<l.principal+l.interest){l.defaulted=true;event(l.debtor,person(l.debtor).region,103,l.principal+l.interest-l.paid,l.id);}
    for(auto&o:proposals_)if(!o.resolved&&o.due<day_){o.resolved=true;event(o.from,person(o.from).region,104,o.amount,o.to);}
    // Births conserve estate money and preserve independent IDs/relationships.
    if(day_%365==0&&people_.size()<1000000){size_t n=people_.size();for(size_t i=0;i<n;++i){auto&parent=people_[i];if(!parent.alive||parent.health<60||parent.age_days<20*365||parent.age_days>45*365||parent.cash<50||rng_()%100>=6)continue;
        Person child;child.id=people_.size()+1;child.region=parent.region;child.household=parent.household;child.age_days=0;child.cash=0;child.goal=int(rng_()%3);transfer(parent.cash,child.cash,20);people_.push_back(child);edges_.push_back({child.id,uint64_t(i+1),0});event(child.id,child.region,105,0,uint64_t(i+1));}}
    prices();
}
void Realm::advance(int days){if(days<0||days>36500)throw std::invalid_argument("days bounds");for(int i=0;i<days;++i)daily();}
int64_t Realm::total_money()const{int64_t n=carrier_+estate_;for(auto&p:people_)n+=p.cash;for(auto&p:provinces_)n+=p.firm+p.treasury;for(auto funds:organization_funds_)n+=funds;return n;}
bool Realm::invariant()const{
    if(total_money()!=opening_money_+endowment_||provinces_.size()!=size_t(regions_+1))return false;for(auto funds:organization_funds_)if(funds<0)return false;
    std::array<int64_t,7> stock{};for(size_t i=0;i<people_.size();++i){auto&p=people_[i];if(p.id!=i+1||p.job_kind<0||p.job_kind>1||p.organization<0||p.organization>28||p.tax_remainder<0||p.tax_remainder>=100||p.cash<0||p.region<0||p.region>regions_||p.health>100)return false;for(int g=0;g<7;++g){if(p.inventory[g]<0)return false;stock[g]+=p.inventory[g];}}
    for(auto&p:provinces_){if(p.firm<0||p.treasury<0)return false;for(int g=0;g<7;++g){if(p.stock[g]<0)return false;stock[g]+=p.stock[g];}}
    for(int g=0;g<7;++g)if(stock[g]+consumed_[g]!=opening_[g]+produced_[g])return false;
    for(auto&l:loans_)if(l.paid<0||l.paid>l.principal+l.interest||l.debtor==0||l.debtor>people_.size())return false;
    for(auto&e:edges_)if(!e.from||!e.to||e.from>people_.size()||e.to>people_.size())return false;
    return true;
}
RealmView Realm::view(uint64_t observer)const{RealmView v;v.day=day_;v.regions=regions_;v.persons=people_.size();v.money=total_money();v.provinces=provinces_;v.draft=draft_;for(auto&p:people_){v.alive+=p.alive;v.children+=p.alive&&p.age_days<18*365;v.decisions+=p.decisions;}for(auto&f:facts_)if(knows(observer,f))v.facts.push_back(f);return v;}
double Realm::utility(uint64_t id)const{auto&p=person(id);auto&v=provinces_[p.region];double wealth=p.cash-debt(id);for(int g=0;g<7;++g)wealth+=p.inventory[g]*kReference[g];
    if(p.family==Family::Economic)return (p.alive?0:-20)+wealth/100.+p.health*.05;
    if(p.family==Family::Governance){double wellbeing=0;int n=0;for(auto&c:people_)if(c.region==p.region){wellbeing+=c.alive?c.health:0;++n;}double reserve_target=std::max(100.,double(n)*3.);double fiscal=5*std::min(1.,v.treasury/reserve_target);return (p.alive?0:-100)+(p.goal==0?2.:1.)*fiscal+(p.goal==1?3.:1.)*wellbeing/std::max(1,n)-v.unrest*.4+std::min(v.road,4)*5+std::min(v.school,std::max(8,n/4))*.1-debt(id)*.01;}
    return (p.alive?0:-100)+p.health*.8+(p.goal==0?2.:.7)*p.skill+(p.goal==1?.08:.02)*wealth+(p.goal==2?2.:.3)*p.social;
}
std::array<float,realm_obs> Realm::observation(uint64_t id)const{
    std::array<float,realm_obs> o{};auto&p=person(id);auto&v=provinces_[p.region];
    o[0]=float(p.family);o[1]=p.age_days/36500.f;o[2]=p.cash/500.f;o[3]=p.health/100.f;o[4]=p.skill/100.f;o[5]=p.goal/2.f;o[6]=p.region/6.f;o[7]=day_/7300.f;o[8]=debt(id)/300.f;o[9]=p.social/100.f;
    o[94]=available_funds(id)/500.f;o[97]=p.taxing_authority;o[98]=p.sovereign;
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
    // Typed effects; finite endowments recorded in the opening ledger.
    auto&crown=provinces_.back();if(card==3){int64_t food=90*std::max(1,int(people_.size())/(regions_+1))/7;crown.stock[0]+=food;opening_[0]+=food;}
    if(card==4){crown.firm+=5000;endowment_+=5000;}
    if(card==6){for(int i=0;i<3;++i)people_[14+i].stats.values[i+1]=75;}
    if(card==8){crown.treasury+=10000;endowment_+=10000;}
    if(card==10)crown.school+=8;if(card==11){crown.school+=2;people_[17].stats.values[1]=80;people_[18].stats.values[4]=80;}
    event(1,regions_,106,card);return true;
}
std::string Realm::action_name(const Candidate&c){static const char* names[]={"Chờ","Mua","Bán","Vận tải","Vay","Trả nợ","Dự án","Tuyển dụng","Thuế","Cứu trợ","Điều tra","Đề nghị","Chấp thuận","Từ chối","Làm việc","Học","Nghỉ","Giao lưu","Tham gia hội","Di cư","Giúp gia đình","Bổ nhiệm","Huy động"};return names[int(c.kind)];}

// Text format uses explicit scalar fields, not compiler-dependent struct bytes.
std::string Realm::serialize(const std::string&bundle)const{
    std::ostringstream s;s<<"POLYEC_REALM_5 "<<std::quoted(bundle)<<' '<<day_<<' '<<regions_<<' '<<seed_<<' '<<next_id_<<' '<<next_fact_<<' '<<opening_money_<<' '<<carrier_<<' '<<estate_<<' '<<endowment_<<' ';
    s<<draft_.tier<<' '<<draft_.seen<<' '<<draft_.chosen<<' '<<draft_.rerolled<<' ';for(auto v:draft_.cards)s<<v<<' ';for(auto&a:{opening_,produced_,consumed_})for(auto v:a)s<<v<<' ';
    for(auto&r:routes_)for(bool open:r)s<<open<<' ';for(auto funds:organization_funds_)s<<funds<<' ';s<<rng_<<' '<<people_.size()<<' ';for(auto&p:people_){s<<p.id<<' '<<p.household<<' '<<p.decisions<<' '<<p.sampling<<' '<<p.region<<' '<<p.age_days<<' '<<p.health<<' '<<p.skill<<' '<<p.job<<' '<<p.busy_until<<' '<<p.destination<<' '<<p.goal<<' '<<p.school_days<<' '<<p.social<<' '<<p.alive<<' '<<int(p.family)<<' '<<p.cash<<' '<<p.tax_remainder<<' '<<p.organization<<' '<<p.job_kind<<' '<<p.taxing_authority<<' '<<p.sovereign<<' ';for(auto v:p.stats.values)s<<v<<' ';for(auto v:p.inventory)s<<v<<' ';}
    s<<provinces_.size()<<' ';for(auto&p:provinces_){s<<p.treasury<<' '<<p.firm<<' '<<p.tax<<' '<<p.school<<' '<<p.school_used<<' '<<p.jobs<<' '<<p.employed<<' '<<p.road<<' '<<p.unrest<<' '<<p.teachers<<' '<<p.charter<<' ';for(auto v:p.stock)s<<v<<' ';for(auto v:p.price)s<<v<<' ';}
    s<<edges_.size()<<' ';for(auto&e:edges_)s<<e.from<<' '<<e.to<<' '<<e.kind<<' ';
    s<<loans_.size()<<' ';for(auto&l:loans_)s<<l.id<<' '<<l.debtor<<' '<<l.creditor<<' '<<l.due<<' '<<l.principal<<' '<<l.interest<<' '<<l.paid<<' '<<l.defaulted<<' ';
    s<<projects_.size()<<' ';for(auto&j:projects_)s<<j.id<<' '<<j.owner<<' '<<j.region<<' '<<j.due<<' '<<j.kind<<' '<<j.budget<<' '<<j.completed<<' ';
    s<<facts_.size()<<' ';for(auto&f:facts_)s<<f.id<<' '<<f.parent<<' '<<f.actor<<' '<<f.target<<' '<<f.day<<' '<<f.region<<' '<<f.kind<<' '<<f.amount<<' '<<f.secret<<' ';
    s<<knowledge_.size()<<' ';for(auto&k:knowledge_)s<<k.observer<<' '<<k.fact<<' '<<k.arrives<<' ';
    s<<proposals_.size()<<' ';for(auto&o:proposals_)s<<o.id<<' '<<o.from<<' '<<o.to<<' '<<o.amount<<' '<<o.due<<' '<<o.resolved<<' ';return s.str();
}
bool Realm::deserialize(const std::string&data,const std::string&bundle,std::string&error){try{
    if(data.size()>256*1024*1024)throw std::runtime_error("Save size limit");Realm w(1,1,128);std::istringstream s(data);std::string magic,b;s>>magic>>std::quoted(b);if(magic!="POLYEC_REALM_5"||b!=bundle)throw std::runtime_error("Save schema/model bundle mismatch");
    s>>w.day_>>w.regions_>>w.seed_>>w.next_id_>>w.next_fact_>>w.opening_money_>>w.carrier_>>w.estate_>>w.endowment_;
    s>>w.draft_.tier>>w.draft_.seen>>w.draft_.chosen>>w.draft_.rerolled;for(auto&v:w.draft_.cards)s>>v;for(auto*a:{&w.opening_,&w.produced_,&w.consumed_})for(auto&v:*a)s>>v;for(auto&r:w.routes_)for(auto&open:r)s>>open;for(auto&funds:w.organization_funds_)s>>funds;s>>w.rng_;
    auto size=[&](size_t max){size_t n=0;s>>n;if(!s||n>max)throw std::runtime_error("Save count limit");return n;};
    w.people_.resize(size(1000000));for(auto&p:w.people_){int family=-1;s>>p.id>>p.household>>p.decisions>>p.sampling>>p.region>>p.age_days>>p.health>>p.skill>>p.job>>p.busy_until>>p.destination>>p.goal>>p.school_days>>p.social>>p.alive>>family>>p.cash>>p.tax_remainder>>p.organization>>p.job_kind>>p.taxing_authority>>p.sovereign;if(family<0||family>2)throw std::runtime_error("Save role");p.family=Family(family);for(auto&v:p.stats.values)s>>v;for(auto&v:p.inventory)s>>v;}
    w.provinces_.resize(size(7));for(auto&p:w.provinces_){s>>p.treasury>>p.firm>>p.tax>>p.school>>p.school_used>>p.jobs>>p.employed>>p.road>>p.unrest>>p.teachers>>p.charter;for(auto&v:p.stock)s>>v;for(auto&v:p.price)s>>v;}
    w.edges_.resize(size(8000000));for(auto&e:w.edges_)s>>e.from>>e.to>>e.kind;
    w.loans_.resize(size(1000000));for(auto&l:w.loans_)s>>l.id>>l.debtor>>l.creditor>>l.due>>l.principal>>l.interest>>l.paid>>l.defaulted;
    w.projects_.resize(size(1000000));for(auto&j:w.projects_)s>>j.id>>j.owner>>j.region>>j.due>>j.kind>>j.budget>>j.completed;
    w.facts_.resize(size(8192));for(auto&f:w.facts_)s>>f.id>>f.parent>>f.actor>>f.target>>f.day>>f.region>>f.kind>>f.amount>>f.secret;
    w.knowledge_.resize(size(1000000));for(auto&k:w.knowledge_){s>>k.observer>>k.fact>>k.arrives;auto&known=w.knowledge_index_[k.observer];auto previous=known.find(k.fact);if(previous==known.end()||previous->second>k.arrives)known[k.fact]=k.arrives;}
    w.proposals_.resize(size(1000000));for(auto&o:w.proposals_)s>>o.id>>o.from>>o.to>>o.amount>>o.due>>o.resolved;
    s>>std::ws;if(!s.eof()||s.fail()&&!s.eof()||w.people_.size()<128||w.day_<0||w.day_>365000||!w.invariant())throw std::runtime_error("Corrupt save/invariant");*this=std::move(w);error.clear();return true;
}catch(const std::exception&e){error=e.what();return false;}}
bool Realm::save(const std::string&path,const std::string&bundle,std::string&error)const{try{auto dest=std::filesystem::u8path(path),temp=dest;temp+=".tmp";auto data=serialize(bundle);{std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<hash_text(data)<<'\n'<<data;out.flush();if(!out)throw std::runtime_error("Save write failed");}auto backup=dest;backup+=".bak";if(std::filesystem::exists(dest))std::filesystem::copy_file(dest,backup,std::filesystem::copy_options::overwrite_existing);std::filesystem::rename(temp,dest);error.clear();return true;}catch(const std::exception&e){error=e.what();return false;}}
bool Realm::load(const std::string&path,const std::string&bundle,std::string&error){try{std::ifstream in(std::filesystem::u8path(path),std::ios::binary);uint64_t checksum;in>>checksum;in.get();std::string data((std::istreambuf_iterator<char>(in)),{});if(!in||hash_text(data)!=checksum)throw std::runtime_error("Save checksum/read failed");return deserialize(data,bundle,error);}catch(const std::exception&e){error=e.what();return false;}}
uint64_t Realm::fingerprint()const{return hash_text(serialize("fingerprint"));}
}
