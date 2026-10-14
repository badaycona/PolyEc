#include "polyec/realm.hpp"
#include <algorithm>
#include <istream>
#include <ostream>
namespace polyec {
std::vector<Proposal> Realm::offers(uint64_t observer)const{std::vector<Proposal> result;for(auto&o:proposals_)if(!o.resolved&&o.due>=day_&&(o.from==observer||(o.to==observer&&o.ready<=day_)))result.push_back(o);return result;}
void Realm::initialize_court_reference(){
    if(royal_reference_>0)return;
    // A deterministic opening-economy reference, never the campaign's future RNG.
    Realm reference=*this;reference.reference_scenario_=true;reference.draft_.chosen=-1;
    auto region=person(1).region;auto before=reference.provinces_[region].tax_receipts;
    reference.advance(365);royal_reference_=std::max<int64_t>(1,reference.provinces_[region].tax_receipts-before);
}
int Realm::report_delay(bool inventory)const{
    int prep=inventory?std::max(1,10-(std::min(100,person(1).stats.values[1]+(draft_.chosen==0?10:0)))/20):1;
    if(draft_.chosen==0)prep=std::max(1,(prep*80+99)/100);
    int courier=draft_.chosen==7?3:4;return prep+courier;
}
int Realm::office_capacity(uint64_t actor)const{return person(actor).sovereign?3+bonus_offices_:0;}
int Realm::office_used(uint64_t actor)const{int n=0;for(auto&p:people_)n+=p.alive&&p.office&&p.hired_by==actor;return n;}
void Realm::apply_augment(int card){
    auto&crown=provinces_[person(1).region];
    if(card==3){int64_t food=90*std::max(1,crown.residents)/7;crown.food_capacity+=food;crown.stock[0]+=food;opening_[0]+=food;}
    if(card==4||card==8){initialize_court_reference();auto amount=card==4?royal_reference_/2:royal_reference_;(card==4?credit_pool_:restricted_fund_)+=amount;endowment_+=amount;}
    if(card==6||card==11){int wanted=card==6?3:2;for(auto&p:people_){if(!wanted)break;if(p.family!=Family::Civilian||p.region!=person(1).region||p.age_days<18*365||!p.alive)continue;p.candidate=true;p.special_advisor=card==11;p.stats.values[card==6?(3-wanted)%3+1:(wanted==2?1:4)]=card==6?70+(3-wanted)*5:85;referrals_.push_back(p.id);--wanted;}}
    if(card==9)pact_=true;
    if(card==10)crown.school+=8;
    if(card==11)bonus_offices_=2;
}
bool Realm::project_materials(int region,int kind)const{auto&v=provinces_[region];return kind<2?v.stock[3]>=4&&v.stock[5]>=2:v.stock[0]>=10&&v.stock[2]>=1;}
void Realm::start_project(uint64_t owner,int region,int kind,int64_t budget,int term,bool from_escrow,uint64_t cause){
    auto&p=mutable_person(owner);auto&v=provinces_[region];
    if(!from_escrow){auto&funds=p.family==Family::Governance?provinces_[p.region].treasury:p.cash;int64_t match=p.sovereign&&kind<2?std::min(restricted_fund_,budget/2):0;transfer(restricted_fund_,v.firm,match);transfer(funds,v.firm,budget-match);}
    if(kind<2){if(!from_escrow){v.stock[3]-=4;v.stock[5]-=2;}consumed_[3]+=4;consumed_[5]+=2;}
    else {v.stock[0]-=10;v.stock[2]-=1;consumed_[0]+=10;consumed_[2]+=1;}
    bool engineer=p.sovereign&&draft_.chosen==5&&kind==0;for(auto&j:projects_)if(!j.completed&&j.engineer)engineer=false;
    int productivity=100+std::min(50,p.stats.values[1]/5+(engineer?20:0));
    int duration=term*100/productivity;
    if(kind==2&&p.sovereign&&draft_.chosen==1)duration=(duration*85+99)/100;
    if(kind==3&&p.sovereign&&draft_.chosen==10)duration=(duration*100+124)/125;
    // Economic expertise reduces procurement preparation, never the bill/materials.
    duration+=std::max(1,5-p.stats.values[2]/25);
    Project j{next_id_++,owner,region,day_+std::max(1,duration),kind,budget};j.engineer=engineer;j.cause=cause?cause:next_fact_;projects_.push_back(j);
}
void Realm::finish_projects(){for(auto&j:projects_)if(!j.completed&&j.due<=day_){
    auto&v=provinces_[j.region];bool ready=true;
    if(j.kind<2){if(j.kind==0)v.road=std::min(4,v.road+1);else v.school+=4;}
    else {ready=false;if(v.teachers>0)for(auto&p:people_)if(p.alive&&p.region==j.region&&((j.kind==2&&p.school_days>0)||(j.kind==3&&p.job>=0&&p.job_kind==1))){p.skill=std::min(100,p.skill+10);ready=true;}}
    j.stalled=!ready;if(!ready){j.due=day_+7;continue;}j.completed=true;event(j.owner,j.region,int(Intent::Project),j.budget,j.id,false,j.cause);
}}
bool Realm::legal_proposal_response(uint64_t actor,const Candidate&action)const{
    for(auto&o:proposals_)if(o.id==uint64_t(action.target)&&o.to==actor&&!o.resolved&&o.due>=day_&&o.ready<=day_){
        if(action.kind==Intent::Reject||o.kind==0)return true;
        auto&p=person(actor);return p.alive&&p.taxing_authority&&provinces_[p.region].treasury>=o.amount&&project_materials(p.region,0)&&provinces_[p.region].road<4;
    }return false;
}
void Realm::create_proposal(uint64_t actor,const Candidate&action){
    auto&p=mutable_person(actor);Proposal o{next_id_++,actor,uint64_t(action.target),action.amount,day_+action.term};o.cause=next_fact_;o.source_region=p.region;o.public_source=p.family==Family::Governance;
    if(action.good==2){o.kind=2;o.parties={2,3,4};o.regions={person(2).region,person(3).region,person(4).region};o.to=o.parties[0];}
    else {o.mediation=action.good==1;transfer(o.public_source?provinces_[p.region].treasury:p.cash,o.escrow,action.amount);int prep=std::max(1,10*100/(100+p.stats.values[3]));if(o.mediation){prep=(prep*80+99)/100;mediation_ready_=day_+180;}o.ready=day_+prep;o.due=o.ready+action.term;}
    proposals_.push_back(o);
}
void Realm::refund_proposal(Proposal&o){
    if(o.kind==0){auto&from=mutable_person(o.from);transfer(o.escrow,o.public_source?provinces_[o.source_region].treasury:from.cash,o.escrow);}
    else for(int i=0;i<3;++i)if(o.pledged[i]){auto&v=provinces_[o.regions[i]];transfer(o.escrow,v.treasury,o.amount);v.stock[3]+=4;v.stock[5]+=2;o.pledged[i]=false;}
    o.resolved=true;
}
void Realm::resolve_proposal(uint64_t actor,const Candidate&action){for(auto&o:proposals_)if(o.id==uint64_t(action.target)){
    if(action.kind==Intent::Reject){refund_proposal(o);return;}
    if(o.kind==0){transfer(o.escrow,mutable_person(actor).cash,o.escrow);edges_.push_back({o.from,actor,2});o.resolved=true;return;}
    int slot=-1;for(int i=0;i<3;++i)if(o.parties[i]==actor&&!o.pledged[i]){slot=i;break;}if(slot<0)return;
    auto&v=provinces_[o.regions[slot]];transfer(v.treasury,o.escrow,o.amount);v.stock[3]-=4;v.stock[5]-=2;o.pledged[slot]=true;
    for(int i=0;i<3;++i)if(!o.pledged[i]){o.to=o.parties[i];return;}
    for(int i=0;i<3;++i){transfer(o.escrow,provinces_[o.regions[i]].firm,o.amount);start_project(o.parties[i],o.regions[i],0,o.amount,60,true,o.cause);}o.resolved=true;
}}
void Realm::write_court(std::ostream&s)const{
    s<<royal_reference_<<' '<<credit_pool_<<' '<<restricted_fund_<<' '<<mediation_ready_<<' '<<bonus_offices_<<' '<<pact_<<' '<<reference_scenario_<<' ';
    s<<referrals_.size()<<' ';for(auto id:referrals_)s<<id<<' ';
    s<<investigations_.size()<<' ';for(auto&i:investigations_)s<<i.id<<' '<<i.owner<<' '<<i.cause<<' '<<i.region<<' '<<i.due<<' '<<i.completed<<' ';
    for(auto&p:people_)s<<p.candidate<<' '<<p.office<<' '<<p.special_advisor<<' '<<p.hired_by<<' '<<p.salary_due<<' '<<p.office_notice<<' '<<p.last_decision_day<<' ';
    for(auto&v:provinces_)s<<v.tax_receipts<<' '<<v.food_capacity<<' '<<v.teacher_skill_sum<<' ';
    for(auto&l:loans_)s<<l.royal_pool<<' '<<l.cause<<' ';
    for(auto&j:projects_)s<<j.engineer<<' '<<j.stalled<<' '<<j.cause<<' ';
    for(auto&o:proposals_){s<<o.kind<<' '<<o.ready<<' '<<o.escrow<<' '<<o.public_source<<' '<<o.mediation<<' '<<o.source_region<<' '<<o.cause<<' ';for(auto id:o.parties)s<<id<<' ';for(auto r:o.regions)s<<r<<' ';for(bool b:o.pledged)s<<b<<' ';}
}
bool Realm::read_court(std::istream&s){
    s>>royal_reference_>>credit_pool_>>restricted_fund_>>mediation_ready_>>bonus_offices_>>pact_>>reference_scenario_;
    size_t n=0;s>>n;if(!s||n>people_.size())return false;referrals_.resize(n);for(auto&id:referrals_){s>>id;if(id==0||id>people_.size())return false;}
    s>>n;if(!s||n>1000000)return false;investigations_.resize(n);for(auto&i:investigations_){s>>i.id>>i.owner>>i.cause>>i.region>>i.due>>i.completed;if(i.owner==0||i.owner>people_.size()||i.region<0||i.region>regions_)return false;}
    for(auto&p:people_){s>>p.candidate>>p.office>>p.special_advisor>>p.hired_by>>p.salary_due>>p.office_notice>>p.last_decision_day;if(p.hired_by>people_.size())return false;}
    for(auto&v:provinces_)s>>v.tax_receipts>>v.food_capacity>>v.teacher_skill_sum;
    for(auto&l:loans_)s>>l.royal_pool>>l.cause;
    for(auto&j:projects_)s>>j.engineer>>j.stalled>>j.cause;
    for(auto&o:proposals_){s>>o.kind>>o.ready>>o.escrow>>o.public_source>>o.mediation>>o.source_region>>o.cause;for(auto&id:o.parties)s>>id;for(auto&r:o.regions)s>>r;for(auto&b:o.pledged)s>>b;if(o.kind<0||o.kind>2||o.source_region<0||o.source_region>regions_)return false;for(auto r:o.regions)if(r<0||r>regions_)return false;}
    return bool(s);
}
}
