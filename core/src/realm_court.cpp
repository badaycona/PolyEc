#include "polyec/realm.hpp"
#include "polyec/court_scalars_codec.hpp"
#include "polyec/investigation_section_codec.hpp"
#include "polyec/person_court_extension_codec.hpp"
#include "polyec/province_court_extension_codec.hpp"
#include "polyec/loan_court_extension_codec.hpp"
#include "polyec/project_court_extension_codec.hpp"
#include "polyec/proposal_court_extension_codec.hpp"
#include "polyec/referrals_section_codec.hpp"
#include <algorithm>
#include <istream>
#include <ostream>
namespace polyec {
namespace { constexpr size_t realm_good(int legacy) { return static_cast<size_t>(canonical_from_legacy_good(legacy)); } }
std::vector<Proposal> Realm::offers(uint64_t observer)const{std::vector<Proposal> result;for(auto&o:proposals_)if(!o.resolved&&o.due>=day_&&(o.from==observer||(o.to==observer&&o.ready<=day_)))result.push_back(o);return result;}
void Realm::initialize_court_reference(){
    if(royal_reference_>0)return;
    // A deterministic opening-economy reference, never the campaign's future RNG.
    Realm reference=*this;reference.reference_scenario_=true;reference.draft_.chosen=-1;
    auto region=person(1).region;auto before=reference.provinces_[region].tax_receipts;
    reference.advance(365);royal_reference_=std::max<int64_t>(1,reference.provinces_[region].tax_receipts-before);
}
int Realm::report_delay(bool inventory)const{
    int prep=inventory?std::max(1,10-(std::min(100,capability_value(1,LegacyCapability::Administration)+(draft_.chosen==0?10:0)))/20):1;
    if(draft_.chosen==0)prep=std::max(1,(prep*80+99)/100);
    int courier=draft_.chosen==7?3:4;return prep+courier;
}
int Realm::office_capacity(uint64_t actor)const{return person(actor).sovereign?3+bonus_offices_:0;}
int Realm::office_used(uint64_t actor)const{int n=0;for(auto&p:people_)n+=p.alive&&p.office_appointment.office&&p.office_appointment.hired_by==actor;return n;}
void Realm::apply_augment(int card){
    auto&crown=provinces_[person(1).region];
    if(card==3){int64_t food=90*std::max(1,crown.residents)/7;crown.food_capacity+=food;crown.stock[realm_good(0)]+=food;opening_[realm_good(0)]+=food;}
    if(card==4||card==8){initialize_court_reference();auto amount=card==4?royal_reference_/2:royal_reference_;(card==4?credit_pool_:restricted_fund_)+=amount;endowment_+=amount;}
    if(card==6||card==11){int wanted=card==6?3:2;for(auto&p:people_){if(!wanted)break;if(p.family!=Family::Civilian||p.region!=person(1).region||p.age_days<18*365||!p.alive)continue;p.candidate=true;p.special_advisor=card==11;const size_t specialty=card==6?size_t((3-wanted)%3+1):size_t(wanted==2?1:4);p.stats.set(kLegacyCapabilities[specialty],card==6?70+(3-wanted)*5:85);referrals_.push_back(p.id);--wanted;}}
    if(card==9)pact_=true;
    if(card==10)crown.school+=8;
    if(card==11)bonus_offices_=2;
}
bool Realm::project_materials(int region,int kind)const{auto&v=provinces_[region];if(kind==4)return v.stock[size_t(GoodId::ConstructionMaterial)]>=4&&v.stock[size_t(GoodId::Tools)]>=2;return kind<2?v.stock[realm_good(3)]>=4&&v.stock[realm_good(5)]>=2:v.stock[realm_good(0)]>=10&&v.stock[realm_good(2)]>=1;}
void Realm::start_project(uint64_t owner,int region,int kind,int64_t budget,int term,bool from_escrow,uint64_t cause){
    auto&p=mutable_person(owner);auto&v=provinces_[region];
    if(kind==4){transfer(municipal_funds_[size_t(region)],v.firm,budget);v.stock[size_t(GoodId::ConstructionMaterial)]-=4;v.stock[size_t(GoodId::Tools)]-=2;consumed_[size_t(GoodId::ConstructionMaterial)]+=4;consumed_[size_t(GoodId::Tools)]+=2;}
    else if(!from_escrow){auto&funds=p.family==Family::Governance?provinces_[p.region].treasury:p.cash;int64_t match=p.sovereign&&kind<2?std::min(restricted_fund_,budget/2):0;transfer(restricted_fund_,v.firm,match);transfer(funds,v.firm,budget-match);}
    if(kind<2){if(!from_escrow){v.stock[realm_good(3)]-=4;v.stock[realm_good(5)]-=2;}consumed_[realm_good(3)]+=4;consumed_[realm_good(5)]+=2;}
    else {v.stock[realm_good(0)]-=10;v.stock[realm_good(2)]-=1;consumed_[realm_good(0)]+=10;consumed_[realm_good(2)]+=1;}
    bool engineer=p.sovereign&&draft_.chosen==5&&kind==0;for(auto&j:projects_)if(!j.completed&&j.engineer)engineer=false;
    int productivity=100+std::min(50,capability_value(owner,LegacyCapability::Administration)/5+(engineer?20:0));
    int duration=kind==4?kUrbanHousingProjectTermDays:term*100/productivity;
    if(kind==2&&p.sovereign&&draft_.chosen==1)duration=(duration*85+99)/100;
    if(kind==3&&p.sovereign&&draft_.chosen==10)duration=(duration*100+124)/125;
    // Economic expertise reduces procurement preparation, never the bill/materials.
    if(kind!=4)duration+=std::max(1,5-capability_value(owner,LegacyCapability::EconomicPlanning)/25);
    Project j{next_id_++,owner,region,day_+std::max(1,duration),kind,budget};j.engineer=engineer;j.cause=cause?cause:next_fact_;projects_.push_back(j);
}
void Realm::finish_projects(){for(auto&j:projects_)if(!j.completed&&j.due<=day_){
    auto&v=provinces_[j.region];bool ready=true;
    if(j.kind<2){if(j.kind==0)v.road=std::min(4,v.road+1);else v.school+=4;}
    else if(j.kind==4){if(regions_!=int(kAtlasTerritoryCount)-1||urban_housing_capacity_[size_t(j.region)]>std::numeric_limits<int64_t>::max()-kUrbanHousingBedsPerBlock)throw std::logic_error("housing project capacity overflow or non-Atlas project");urban_housing_capacity_[size_t(j.region)]+=kUrbanHousingBedsPerBlock;}
    else {ready=false;if(v.teachers>0)for(auto&p:people_)if(p.alive&&p.region==j.region&&((j.kind==2&&p.school_days>0)||(j.kind==3&&p.work_assignment.job>=0&&p.work_assignment.job_kind==1))){p.skill=std::min(100,p.skill+10);ready=true;}}
    j.stalled=!ready;if(!ready){j.due=day_+7;continue;}j.completed=true;event(j.owner,j.region,int(Intent::Project),j.budget,j.id,false,j.cause);
}}
bool Realm::legal_proposal_response(uint64_t actor,const Candidate&action)const{
    for(auto&o:proposals_)if(o.id==uint64_t(action.target)&&o.to==actor&&!o.resolved&&o.due>=day_&&o.ready<=day_){
        if(action.kind==Intent::Reject||o.kind==0)return true;
        if(o.kind==9){
            if(action.kind!=Intent::Accept||actor==0||actor>people_.size()||o.from==0||o.from>people_.size()||
               !apprenticeship_tuition_is_supported(o.amount)||o.service_term!=kApprenticeshipTrialTermDays||
               o.escrow!=0||o.cause==0||o.cause>uint64_t(INT32_MAX)||o.source_region<0||o.source_region>regions_||
               day_>365000-kApprenticeshipTrialTermDays||
               schedule_book_.cursor>365000LL*kSubdayUnitsPerDay-int64_t(kApprenticeshipTrialTermDays)*kSubdayUnitsPerDay||
               apprenticeships_.size()>=1000000)return false;
            const auto&mentor=person(o.from);const auto&learner=person(actor);
            const auto site=std::lower_bound(establishments_.begin(),establishments_.end(),o.cause,
                [](const Establishment& value,uint64_t target){return value.id<target;});
            if(site==establishments_.end()||site->id!=o.cause||site->owner_kind!=EstablishmentOwnerKind::Person||
               site->owner_id!=mentor.id||site->region!=o.source_region||mentor.region!=site->region||
               mentor.destination!=-1||!mentor.alive||mentor.age_days<18*365||
               mentor.skill<kApprenticeshipTrialMentorSkill||mentor.military_service.active||
               learner.family!=Family::Civilian||!learner.alive||learner.age_days<18*365||learner.age_days>=31*365||
               learner.skill>=80||learner.region!=site->region||learner.destination!=-1||learner.busy_until>day_||
               learner.military_service.active||available_funds(actor)<o.amount||
               site->cash>std::numeric_limits<int64_t>::max()-o.amount||
               site->work_authorizations.contains(actor))return false;
            return !std::any_of(apprenticeships_.begin(),apprenticeships_.end(),[&](const auto&course){
                return course.status==ApprenticeshipStatus::Active&&
                    (course.mentor==mentor.id||course.apprentice==mentor.id||course.apprentice==actor||
                     course.establishment_id==site->id);
            });
        }
        if(o.kind==5){
            if(actor==0||actor>people_.size()||o.from==0||o.from>people_.size()||
               !person(actor).alive||!person(actor).sovereign||person(actor).id!=o.to||
               person(o.from).family!=Family::Governance||!has_local_government_authority(person(o.from))||
               person(o.from).sovereign||person(o.from).region!=o.source_region||
               o.amount<1||o.amount>4||o.service_term!=14||o.escrow!=0||day_>365000-16)return false;
            for(const auto& obligation:military_obligations_)
                if(obligation.obligor==o.from&&obligation.completed_day<0)return false;
            if(size_t(o.amount)!=o.roster_members.size()||!std::is_sorted(o.roster_members.begin(),o.roster_members.end())||
               std::adjacent_find(o.roster_members.begin(),o.roster_members.end())!=o.roster_members.end())return false;
            const auto eligible=eligible_muster_members(o.from,o.source_region,day_+16);
            return std::all_of(o.roster_members.begin(),o.roster_members.end(),[&](uint64_t member){
                return std::binary_search(eligible.begin(),eligible.end(),member);
            });
        }
        if(o.kind==6){
            if(actor==0||actor>people_.size()||!person(actor).alive||!person(actor).sovereign||person(actor).id!=o.to||
               o.from==0||o.from>people_.size()||person(o.from).family!=Family::Governance||
               !has_local_government_authority(person(o.from))||person(o.from).sovereign||o.service_term!=7||o.escrow!=0||
               o.amount!=7||day_>365000-7)return false;
            for(const auto& obligation:military_obligations_)
                if(obligation.id==o.cause&&obligation.obligor==o.from&&obligation.recipient==actor&&
                   obligation.completed_day<0&&obligation.due_checked&&obligation.shortage_at_last_deadline>0&&
                   obligation.extensions==0)return true;
            return false;
        }
        if(o.kind==7){
            if(actor==0||actor>people_.size()||!person(actor).alive||!person(actor).sovereign||person(actor).id!=o.to||
               o.from==0||o.from>people_.size()||person(o.from).family!=Family::Governance||
               !has_local_government_authority(person(o.from))||person(o.from).sovereign||o.service_term!=14||o.escrow!=0||
               o.amount<1||o.amount>4||day_>365000-14)return false;
            for(const auto& source:military_obligations_)
                if(source.id==o.cause&&source.obligor==o.from&&source.recipient==actor&&source.completed_day<0&&
                   source.required_members==o.amount){
                    for(const auto& supply:military_supply_obligations_)
                        if(supply.personnel_obligation_id==source.id)return false;
                    return true;
                }
            return false;
        }
        if(o.kind==8){
            if(actor==0||actor>people_.size()||!person(actor).alive||!person(actor).sovereign||person(actor).id!=o.to||
               o.from==0||o.from>people_.size()||person(o.from).family!=Family::Governance||
               !has_local_government_authority(person(o.from))||person(o.from).sovereign||o.service_term!=7||o.escrow!=0||
               o.amount!=7||day_>365000-7)return false;
            for(const auto& supply:military_supply_obligations_)
                if(supply.id==o.cause&&supply.obligor==o.from&&supply.recipient==actor&&supply.completed_day<0&&
                   supply.due_checked&&supply.shortage_at_last_deadline>0&&supply.extensions==0)return true;
            return false;
        }
        if(o.kind==4){
            if(actor==0||actor>people_.size()||o.from==0||o.from>people_.size()||
               !military_service_eligible(person(actor))||o.service_term<1||o.service_term>30||
               o.amount!=kMilitaryServiceTrialWagePerDay||o.escrow!=o.amount*o.service_term||
               person(actor).region!=o.source_region||person(o.from).family!=Family::Governance||
               person(o.from).region!=o.source_region||day_>=365000)return false;
            return true;
        }
        if(o.kind==3)return marriage_eligible(actor)&&marriage_eligible(o.from)&&!marriage_related(o.from,actor)&&!marriage_pending(o.from,actor,o.id)&&
            o.escrow<=std::numeric_limits<int64_t>::max()-person(actor).cash;
        auto&p=person(actor);return p.alive&&has_local_government_authority(p)&&provinces_[p.region].treasury>=o.amount&&project_materials(p.region,0)&&provinces_[p.region].road<4;
    }return false;
}
void Realm::create_proposal(uint64_t actor,const Candidate&action){
    auto&p=mutable_person(actor);Proposal o{next_id_++,actor,uint64_t(action.target),action.amount,day_+action.term};o.cause=next_fact_;o.source_region=p.region;o.public_source=p.family==Family::Governance;
    if(action.kind==Intent::OfferApprenticeship){o.kind=9;o.cause=uint64_t(action.good);o.service_term=action.term;o.public_source=false;o.escrow=0;o.ready=day_;o.due=day_+7;}
    else if(action.kind==Intent::Marry){o.kind=3;o.public_source=false;transfer(p.cash,o.escrow,action.amount);o.ready=day_+std::max(1,10*100/(100+capability_value(actor,LegacyCapability::Diplomacy)));o.due=o.ready+action.term;}
    else if(action.good==2){o.kind=2;o.parties={2,3,4};o.regions={person(2).region,person(3).region,person(4).region};o.to=o.parties[0];}
    else if(action.good==3){o.kind=4;o.service_term=action.term;o.public_source=true;const int64_t escrow=action.amount*action.term;transfer(provinces_[p.region].treasury,o.escrow,escrow);o.ready=day_+std::max(1,10*100/(100+capability_value(actor,LegacyCapability::Diplomacy)));o.due=o.ready+7;}
    else if(action.good==4){o.kind=5;o.amount=action.amount;o.service_term=14;o.public_source=true;o.escrow=0;const auto eligible=eligible_muster_members(actor,p.region,day_+30);o.roster_members.assign(eligible.begin(),eligible.begin()+action.amount);o.ready=day_+std::max(1,10*100/(100+capability_value(actor,LegacyCapability::Diplomacy)));o.due=o.ready+7;}
    else if(action.good==5){o.kind=6;o.service_term=7;o.public_source=true;o.escrow=0;for(const auto&obligation:military_obligations_)if(obligation.obligor==actor&&obligation.recipient==uint64_t(action.target)&&obligation.completed_day<0&&obligation.due_checked&&obligation.shortage_at_last_deadline>0&&obligation.extensions==0){o.cause=obligation.id;break;}o.amount=7;o.ready=day_+std::max(1,10*100/(100+capability_value(actor,LegacyCapability::Diplomacy)));o.due=o.ready+7;}
    else if(action.good==6){o.kind=7;o.service_term=14;o.public_source=true;o.escrow=0;for(const auto&obligation:military_obligations_)if(obligation.obligor==actor&&obligation.recipient==uint64_t(action.target)&&obligation.completed_day<0&&obligation.required_members==action.amount){o.cause=obligation.id;break;}o.ready=day_+std::max(1,10*100/(100+capability_value(actor,LegacyCapability::Diplomacy)));o.due=o.ready+7;}
    else if(action.good==7){o.kind=8;o.service_term=7;o.public_source=true;o.escrow=0;for(const auto&obligation:military_supply_obligations_)if(obligation.obligor==actor&&obligation.recipient==uint64_t(action.target)&&obligation.completed_day<0&&obligation.due_checked&&obligation.shortage_at_last_deadline>0&&obligation.extensions==0){o.cause=obligation.id;break;}o.amount=7;o.ready=day_+std::max(1,10*100/(100+capability_value(actor,LegacyCapability::Diplomacy)));o.due=o.ready+7;}
    else {o.mediation=action.good==1;transfer(o.public_source?provinces_[p.region].treasury:p.cash,o.escrow,action.amount);int prep=std::max(1,10*100/(100+capability_value(actor,LegacyCapability::Diplomacy)));if(o.mediation){prep=(prep*80+99)/100;mediation_ready_=day_+180;}o.ready=day_+prep;o.due=o.ready+action.term;}
    proposals_.push_back(o);
}
void Realm::refund_proposal(Proposal&o){
    if(o.kind==0||o.kind==3){auto&from=mutable_person(o.from);transfer(o.escrow,o.public_source?provinces_[o.source_region].treasury:from.cash,o.escrow);}
    else if(o.kind==4)transfer(o.escrow,provinces_[o.source_region].treasury,o.escrow);
    else if(o.kind==5||o.kind==6||o.kind==7||o.kind==8||o.kind==9){if(o.escrow!=0)throw std::logic_error("obligation or apprenticeship proposal has escrow");}
    else for(int i=0;i<3;++i)if(o.pledged[i]){auto&v=provinces_[o.regions[i]];transfer(o.escrow,v.treasury,o.amount);v.stock[realm_good(3)]+=4;v.stock[realm_good(5)]+=2;o.pledged[i]=false;}
    o.resolved=true;
}
void Realm::resolve_proposal(uint64_t actor,const Candidate&action){for(auto&o:proposals_)if(o.id==uint64_t(action.target)){
    if(action.kind==Intent::Reject){refund_proposal(o);return;}
    if(o.kind==0){transfer(o.escrow,mutable_person(actor).cash,o.escrow);edges_.push_back({o.from,actor,RelationKind::Agreement});o.resolved=true;return;}
    if(o.kind==3){transfer(o.escrow,mutable_person(actor).cash,o.escrow);edges_.push_back({std::min(o.from,uint64_t(actor)),std::max(o.from,uint64_t(actor)),RelationKind::Spouse});o.resolved=true;return;}
    if(o.kind==9){
        auto site=std::lower_bound(establishments_.begin(),establishments_.end(),o.cause,
            [](const Establishment& value,uint64_t target){return value.id<target;});
        if(site==establishments_.end()||site->id!=o.cause)throw std::logic_error("validated apprenticeship site disappeared");
        const int64_t end=schedule_book_.cursor+int64_t(o.service_term)*kSubdayUnitsPerDay;
        auto [authorization,inserted]=site->work_authorizations.try_emplace(actor);
        if(!inserted||!authorization->second.empty())throw std::logic_error("validated apprentice authorization changed");
        authorization->second.reserve(1);
        apprenticeships_.reserve(apprenticeships_.size()+1);
        spend(mutable_person(actor),site->cash,o.amount);
        authorization->second.push_back({schedule_book_.cursor,end});
        ++site->authorization_window_count;
        const auto mentorTicks=schedule_book_.work_present_ticks_by_establishment.find({o.from,site->id});
        const auto apprenticeTicks=schedule_book_.work_present_ticks_by_establishment.find({actor,site->id});
        apprenticeships_.push_back({o.id,o.from,actor,site->id,day_,day_+o.service_term,0,
            schedule_book_.cursor,end,
            mentorTicks==schedule_book_.work_present_ticks_by_establishment.end()?0:mentorTicks->second,
            apprenticeTicks==schedule_book_.work_present_ticks_by_establishment.end()?0:apprenticeTicks->second,
            o.amount,ApprenticeshipStatus::Active});
        o.resolved=true;
        event(actor,site->region,240,o.amount,o.from,false,o.id);
        return;
    }
    if(o.kind==4){
        auto& recipient=mutable_person(actor);
        recipient.military_service={true,false,o.from,day_+1,day_+1+o.service_term,o.source_region,0,o.amount,o.escrow,0};
        o.escrow=0;o.resolved=true;
        event(actor,recipient.region,114,o.amount,o.from);
        return;
    }
    if(o.kind==5){
        const auto& eligible=o.roster_members;
        MilitaryObligation obligation;obligation.id=o.id;obligation.obligor=o.from;obligation.recipient=o.to;
        obligation.source_region=o.source_region;obligation.destination_region=person(actor).region;
        obligation.created_day=day_;obligation.first_due_day=day_+14;obligation.due_day=obligation.first_due_day;
        obligation.required_members=int(o.amount);
        obligation.member_ids.assign(eligible.begin(),eligible.begin()+o.amount);
        obligation.member_ids.shrink_to_fit();
        military_obligations_.push_back(std::move(obligation));
        o.resolved=true;
        event(o.from,o.source_region,229,o.amount,actor,false,o.id);
        return;
    }
    if(o.kind==6){
        for(auto& obligation:military_obligations_)if(obligation.id==o.cause&&obligation.obligor==o.from&&obligation.recipient==actor){
            obligation.due_day=day_+7;obligation.due_checked=false;obligation.extensions=1;
            obligation.delivered_at_last_deadline=0;obligation.shortage_at_last_deadline=0;
            o.resolved=true;event(o.from,obligation.source_region,231,obligation.due_day,actor,false,o.id);return;
        }
        return;
    }
    if(o.kind==7){
        for(const auto& source:military_obligations_)if(source.id==o.cause&&source.obligor==o.from&&source.recipient==actor){
            MilitarySupplyObligation supply;supply.id=o.id;supply.personnel_obligation_id=source.id;
            supply.obligor=o.from;supply.recipient=actor;supply.source_region=source.source_region;
            supply.destination_region=source.destination_region;supply.created_day=day_;
            supply.first_due_day=day_+14;supply.due_day=supply.first_due_day;
            supply.first_eligible_shipment_id=next_id_;supply.required=source.required_members;
            military_supply_obligations_.push_back(supply);o.resolved=true;
            event(o.from,source.source_region,235,supply.required,actor,false,source.id);return;
        }
        return;
    }
    if(o.kind==8){
        for(auto& supply:military_supply_obligations_)if(supply.id==o.cause&&supply.obligor==o.from&&supply.recipient==actor){
            supply.due_day=day_+7;supply.due_checked=false;supply.extensions=1;
            supply.delivered_at_last_deadline=0;supply.shortage_at_last_deadline=0;
            o.resolved=true;event(o.from,supply.source_region,236,supply.due_day,actor,false,supply.id);return;
        }
        return;
    }
    int slot=-1;for(int i=0;i<3;++i)if(o.parties[i]==actor&&!o.pledged[i]){slot=i;break;}if(slot<0)return;
    auto&v=provinces_[o.regions[slot]];transfer(v.treasury,o.escrow,o.amount);v.stock[realm_good(3)]-=4;v.stock[realm_good(5)]-=2;o.pledged[slot]=true;
    for(int i=0;i<3;++i)if(!o.pledged[i]){o.to=o.parties[i];return;}
    for(int i=0;i<3;++i){transfer(o.escrow,provinces_[o.regions[i]].firm,o.amount);start_project(o.parties[i],o.regions[i],0,o.amount,60,true,o.cause);}o.resolved=true;
}}
void Realm::write_court(std::ostream&s)const{
    detail::write_court_scalars(s, royal_reference_, credit_pool_, restricted_fund_, mediation_ready_, bonus_offices_, pact_, reference_scenario_);
    detail::write_referrals_section(s, referrals_);
    detail::write_investigation_section(s, investigations_);
    detail::write_person_court_extension_section(s, people_);
    detail::write_province_court_extension_section(s, provinces_);
    detail::write_loan_court_extension_section(s, loans_);
    detail::write_project_court_extension_section(s, projects_);
    detail::write_proposal_court_extension_section(s, proposals_);
}
bool Realm::read_court(std::istream&s,int schema){
    try { detail::read_court_scalars(s, royal_reference_, credit_pool_, restricted_fund_, mediation_ready_, bonus_offices_, pact_, reference_scenario_); }
    catch (const std::runtime_error&) { return false; }
    std::vector<uint64_t> staged_referrals;
    try { detail::read_referrals_section(s, staged_referrals, people_.size()); }
    catch (const std::runtime_error&) { return false; }
    for (auto id : staged_referrals) if (id == 0 || id > people_.size()) return false;
    referrals_ = std::move(staged_referrals);
    {std::vector<Investigation> staged_investigations;try{detail::read_investigation_section(s,staged_investigations);}catch(const std::runtime_error&){return false;}for(auto&i:staged_investigations){if(i.owner==0||i.owner>people_.size()||i.region<0||i.region>regions_)return false;}investigations_=std::move(staged_investigations);}
    {try{detail::read_person_court_extension_section(s,people_);}catch(const std::runtime_error&){return false;}for(auto&p:people_)if(p.office_appointment.hired_by>people_.size())return false;}
    {try{detail::read_province_court_extension_section(s,provinces_);}catch(const std::runtime_error&){return false;}}
    {try{detail::read_loan_court_extension_section(s,loans_);}catch(const std::runtime_error&){return false;}}
    {try{detail::read_project_court_extension_section(s,projects_);}catch(const std::runtime_error&){return false;}}
    {try{detail::read_proposal_court_extension_section(s,proposals_,schema);}catch(const std::runtime_error&){return false;}for(auto&o:proposals_){if(o.kind<0||o.kind>(schema>=75?9:schema>=54?8:schema>=51?6:schema>=45?4:schema>=23?3:2)||o.source_region<0||o.source_region>regions_)return false;for(auto r:o.regions)if(r<0||r>regions_)return false;}}
    return bool(s);
}
}
