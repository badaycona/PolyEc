#include "work_authorization_fixture.hpp"
#include "polyec/schedule_section_codec.hpp"
#include "polyec/realm.hpp"
#include "polyec/managers/employment_manager.hpp"
#include "polyec/market_manager.hpp"
#include "polyec/loan_section_codec.hpp"
#include "polyec/project_section_codec.hpp"
#include "polyec/fact_section_codec.hpp"
#include "polyec/knowledge_section_codec.hpp"
#include "polyec/edge_section_codec.hpp"
#include "polyec/proposal_section_codec.hpp"
#include "polyec/realm_primary_header_codec.hpp"
#include "polyec/body_state_section_codec.hpp"
#include "polyec/land_title_section_codec.hpp"
#include "polyec/parentage_section_codec.hpp"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <sstream>
using namespace polyec;
int checks=0;
void check_at(bool ok,const char* label,int line){++checks;if(!ok)throw std::runtime_error(std::string("line ")+std::to_string(line)+": "+label);}
#define check(ok,label) check_at((ok),(label),__LINE__)
namespace polyec {
struct RealmObservationTestAccess {
    static Person& mutable_person(Realm& world,uint64_t actor){return world.mutable_person(actor);}
    static void endow_person_good(Realm&world,uint64_t actor,GoodId good,int64_t quantity){
        world.mutable_person(actor).inventory[size_t(good)]+=quantity;
        world.opening_[size_t(good)]+=quantity;
    }
    static void remove_all_food_and_set_death_health(Realm& world,uint64_t actor){
        auto& person=world.mutable_person(actor);person.inventory[0]=0;person.health=1;person.nutrition=0;
        int64_t removed=0;
        for(auto& province:world.provinces_){removed+=province.stock[0];province.stock[0]=0;}
        for(auto& resident:world.people_){removed+=resident.inventory[0];resident.inventory[0]=0;}
        if(removed>std::numeric_limits<int64_t>::max()-world.consumed_[0])throw std::runtime_error("food death fixture overflow");
        world.consumed_[0]+=removed;
    }
    static void set_province_treasury(Realm& world,int region,int64_t amount){
        auto& province=world.provinces_.at(size_t(region));province.firm+=province.treasury-amount;province.treasury=amount;
    }
    static void set_province_unrest(Realm& world,int region,int amount){world.provinces_.at(size_t(region)).unrest=amount;}
    static uint64_t peek_rng(Realm& world){auto copy=world.rng_;return copy();}
    static void set_personality(Realm& world,uint64_t actor,const std::array<int,6>& values){world.mutable_person(actor).personality.values=values;}
    static size_t knowledge_count(const Realm& world,uint64_t observer,KnowledgeKind kind){return size_t(std::count_if(world.knowledge_.begin(),world.knowledge_.end(),[&](const Knowledge& row){return row.observer==observer&&row.kind==kind;}));}
    static int report_delay(const Realm& world){return world.report_delay();}
    static uint64_t add_test_fact(Realm& world,int region,bool secret,uint64_t actor=1){
        world.event(actor,region,777,1,0,secret);
        return world.facts_.back().id;
    }
    static uint64_t knowledge_source(const Realm& world,uint64_t fact){
        const auto row=std::find_if(world.knowledge_.begin(),world.knowledge_.end(),
            [fact](const Knowledge& known){return known.fact==fact;});
        return row==world.knowledge_.end()?0:row->source_investigation_id;
    }
    static void set_knowledge_source(Realm& world,uint64_t value){
        if(world.knowledge_.empty())throw std::runtime_error("knowledge fixture is empty");
        world.knowledge_.front().source_investigation_id=value;
    }
    static void set_knowledge_kind(Realm& world,KnowledgeKind value){
        if(world.knowledge_.empty())throw std::runtime_error("knowledge fixture is empty");
        world.knowledge_.front().kind=value;
    }
    static void set_knowledge_confidence(Realm& world,int value){
        if(world.knowledge_.empty())throw std::runtime_error("knowledge fixture is empty");
        world.knowledge_.front().confidence_basis_points=value;
    }
    static Knowledge knowledge(const Realm& world,uint64_t fact){
        const auto row=std::find_if(world.knowledge_.begin(),world.knowledge_.end(),
            [fact](const Knowledge& known){return known.fact==fact;});
        if(row==world.knowledge_.end())throw std::runtime_error("knowledge fixture row missing");
        return *row;
    }
    static void set_knowledge_observer(Realm& world,uint64_t value){
        if(world.knowledge_.empty())throw std::runtime_error("knowledge fixture is empty");
        world.knowledge_.front().observer=value;
    }
    static void set_investigation_region(Realm& world,int value){
        if(world.investigations_.empty())throw std::runtime_error("investigation fixture is empty");
        world.investigations_.front().region=value;
    }
    static void set_fact_secret(Realm& world,uint64_t id,bool value){
        const auto fact=std::find_if(world.facts_.begin(),world.facts_.end(),
            [id](const Fact& row){return row.id==id;});
        if(fact==world.facts_.end())throw std::runtime_error("fact fixture is empty");
        fact->secret=value;
    }
    static std::string as_schema32(const Realm& world,const std::string& bundle){
        std::istringstream input(test_fixtures::save_as_v33(world.serialize(bundle)));std::vector<std::string> words;std::string word;
        while(input>>word)words.push_back(word);
        const auto knowledge=std::find(words.begin(),words.end(),"KNOWLEDGE_3");
        if(knowledge==words.end())throw std::runtime_error("fixture missing KNOWLEDGE_3 section");
        const size_t at=size_t(knowledge-words.begin()),count=std::stoull(words.at(at+1));
        const size_t rows=at+2,end=rows+6*count;
        if(end>words.size())throw std::runtime_error("fixture truncated KNOWLEDGE_3 rows");
        std::vector<std::string> legacy;legacy.reserve(words.size()-3*count-1);
        legacy.insert(legacy.end(),words.begin(),knowledge);legacy.push_back("KNOWLEDGE_2");legacy.push_back(words[at+1]);
        for(size_t row=0;row<count;++row){const size_t start=rows+6*row;
            legacy.insert(legacy.end(),words.begin()+std::ptrdiff_t(start),words.begin()+std::ptrdiff_t(start+4));}
        legacy.insert(legacy.end(),words.begin()+std::ptrdiff_t(end),words.end());
        legacy.front()="POLYEC_REALM_32";
        std::ostringstream output;for(const auto& value:legacy)output<<value<<' ';return output.str();
    }
    static std::string as_schema30_without_knowledge_sources(const Realm& world,const std::string& bundle){
        std::istringstream input(as_schema32(world,bundle));std::vector<std::string> legacy;std::string word;
        while(input>>word)legacy.push_back(word);
        legacy.erase(std::find(legacy.begin(),legacy.end(),"CAPABILITY_SCHEMA_1"),legacy.end());
        legacy.front()="POLYEC_REALM_31";
        // Remove the investigation source to form a genuine pre-provenance schema30.
        const auto legacy_tag=std::find(legacy.begin(),legacy.end(),"KNOWLEDGE_2");
        const size_t legacy_at=size_t(legacy_tag-legacy.begin()),legacy_count=std::stoull(legacy.at(legacy_at+1));
        std::vector<std::string> schema30;schema30.insert(schema30.end(),legacy.begin(),legacy_tag);
        schema30.push_back(legacy[legacy_at+1]);
        for(size_t row=0;row<legacy_count;++row){const size_t start=legacy_at+2+4*row;
            schema30.insert(schema30.end(),legacy.begin()+std::ptrdiff_t(start),legacy.begin()+std::ptrdiff_t(start+3));}
        const size_t legacy_end=legacy_at+2+4*legacy_count;
        schema30.insert(schema30.end(),legacy.begin()+std::ptrdiff_t(legacy_end),legacy.end());
        schema30.front()="POLYEC_REALM_30";
        std::ostringstream output;for(const auto& value:schema30)output<<value<<' ';return output.str();
    }
    static bool lineal_kinship(const Realm& world,uint64_t first,uint64_t second){
        return world.marriage_related(first,second);
    }
    static void prepare_large_royal_loan(Realm& world){
        world.opening_money_+=1000000-world.credit_pool_;
        world.credit_pool_=1000000;
        world.royal_reference_=2000000;
        world.draft_.chosen=4;
    }
    static uint64_t prepare_estate_claim_death(Realm& world,int64_t claim_amount=20) {
        uint64_t actor=0;
        for(uint64_t id=1;id<=world.people_.size();++id)
            if(world.people_[id-1].alive&&world.people_[id-1].family==Family::Civilian){actor=id;break;}
        if(!actor)throw std::runtime_error("estate claim fixture has no living civilian");
        auto& person=world.people_[actor-1];
        uint64_t beneficiary=0;
        for(uint64_t id=actor+1;id<=world.people_.size();++id)
            if(world.people_[id-1].alive&&world.people_[id-1].family==Family::Civilian&&
                world.people_[id-1].age_days>=18*365&&id%7==uint64_t(world.day_%7)){beneficiary=id;break;}
        if(!beneficiary)throw std::runtime_error("estate distribution fixture has no living beneficiary");
        uint64_t living_parent=0;
        for(uint64_t id=1;id<=world.people_.size();++id)
            if(id!=actor&&id!=beneficiary&&world.people_[id-1].alive&&
               world.people_[id-1].age_days>=18*365){living_parent=id;break;}
        if(!living_parent)throw std::runtime_error("estate distribution fixture has no living parent-tier beneficiary");
        std::erase_if(world.edges_,[&](const Edge& edge){return edge.from==actor||edge.to==actor;});
        world.edges_.push_back({actor,beneficiary,RelationKind::Spouse,false});
        world.edges_.push_back({actor,living_parent,RelationKind::ParentChild,true});
        const uint64_t holding_id=world.next_id_++;
        world.land_holdings_.push_back({holding_id,1,1,actor});
        uint64_t grantee=0;
        for(uint64_t id=1;id<=world.people_.size();++id)
            if(id!=actor&&id!=beneficiary&&world.people_[id-1].alive){grantee=id;break;}
        if(!grantee)throw std::runtime_error("estate land fixture has no live use grantee");
        const uint64_t right_id=world.next_id_++;
        world.land_use_rights_.push_back({right_id,holding_id,grantee,actor});
        auto& province=world.provinces_[1];
        if(person.cash>province.firm)throw std::runtime_error("estate claim fixture cannot fund test cash");
        province.firm+=person.cash-1000;
        person.cash=1000;
        person.region=1;person.destination=-1;person.health=-1;
        const auto removed_food=province.stock[0]+person.inventory[0];province.stock[0]=0;person.inventory[0]=0;world.consumed_[0]+=removed_food;
        const size_t fiber=static_cast<size_t>(GoodId::Fiber);
        if(province.stock[fiber]<5)throw std::runtime_error("estate goods fixture lacks Fiber in the decedent's region");
        province.stock[fiber]-=5;person.inventory[fiber]+=5;
        Establishment site;site.id=world.next_id_++;site.region=1;site.owner_kind=EstablishmentOwnerKind::Person;
        site.owner_id=actor;site.industry=EstablishmentIndustry::ClothWorkshop;site.capacity=10;site.cash=10;
        if(person.cash<site.cash)throw std::runtime_error("estate workshop fixture cannot fund its account");
        person.cash-=site.cash;site.work_authorizations[beneficiary].push_back({0,10000});
        site.authorization_window_count=1;world.establishments_.push_back(site);world.rebuild_personal_establishment_index();
        auto& beneficiary_person=world.people_[beneficiary-1];
        province.firm+=beneficiary_person.cash;beneficiary_person.cash=0;
        beneficiary_person.region=1;beneficiary_person.destination=-1;
        const auto household=world.household_funds_.find(beneficiary_person.household_membership.household_id);
        if(household!=world.household_funds_.end()){
            province.firm+=household->second;world.household_funds_.erase(household);
        }
        int due_residents=0;
        for(const auto& resident:world.people_)if(resident.alive&&resident.id%7==uint64_t(world.day_%7))++due_residents;
        province.stock[0]=due_residents+1;province.price[0]=1;world.produced_[0]+=due_residents+1;
        world.credit_pool_=1000;
        world.provinces_[1].firm-=1000;
        const uint64_t loan_id=world.next_id_++;
        world.loans_.push_back(Loan{loan_id,actor,1,world.day_+90,claim_amount,0,0,false,true,world.next_fact_});
        world.refresh_demography();
        return actor;
    }
    static void add_living_property_for_isolation(Realm& world,uint64_t decedent) {
        uint64_t owner=0,grantee=0;
        for(const auto& person:world.people_){
            if(!person.alive||person.id==decedent||person.family!=Family::Civilian)continue;
            if(!owner)owner=person.id;else if(person.id!=owner){grantee=person.id;break;}
        }
        if(!owner||!grantee)throw std::runtime_error("living-property fixture lacks two unrelated living people");
        const uint64_t holdingId=world.next_id_++;
        const auto region=world.people_[owner-1].region;
        world.land_holdings_.push_back({holdingId,region,1,owner});
        const uint64_t rightId=world.next_id_++;
        world.land_use_rights_.push_back({rightId,holdingId,grantee,owner});
        auto& province=world.provinces_[region];
        province.firm+=world.people_[owner-1].cash-10;
        world.people_[owner-1].cash=10;
        Establishment site;site.id=world.next_id_++;site.region=region;site.owner_kind=EstablishmentOwnerKind::Person;
        site.owner_id=owner;site.industry=EstablishmentIndustry::ClothWorkshop;site.capacity=3;site.cash=4;
        world.people_[owner-1].cash-=site.cash;
        site.work_authorizations[grantee].push_back({0,10000});site.authorization_window_count=1;
        world.establishments_.push_back(site);world.rebuild_personal_establishment_index();
    }
    static std::array<uint64_t,4> prepare_minor_estate_land_death(Realm& world) {
        uint64_t decedent=0,minor=0,guardian=0;
        for(const auto& candidate:world.people_){
            if(!candidate.alive||candidate.family!=Family::Civilian||candidate.age_days<18*365)continue;
            const auto household=candidate.household_membership.household_id;
            uint64_t child=0,adult=0;
            for(const auto& member:world.people_){
                if(!member.alive||member.household_membership.household_id!=household||member.id==candidate.id)continue;
                if(member.age_days<18*365&&!child)child=member.id;
                if(member.age_days>=18*365&&(!adult||member.age_days>world.people_[adult-1].age_days))adult=member.id;
            }
            if(child&&adult){decedent=candidate.id;minor=child;guardian=adult;break;}
        }
        if(!decedent)throw std::runtime_error("minor estate fixture lacks a household with an adult guardian");
        std::erase_if(world.edges_,[&](const Edge& edge){
            return edge.from==decedent||edge.to==decedent||edge.from==minor||edge.to==minor;
        });
        world.edges_.push_back({minor,decedent,RelationKind::ParentChild,true});
        const auto holding=world.next_id_++;
        world.land_holdings_.push_back({holding,world.people_[decedent-1].region,1,decedent});
        const auto grantee=guardian;
        const auto grant=world.execute(LandUseGrantCommand{decedent,
            world.people_[decedent-1].decisions+1,holding,grantee});
        if(!grant.success)throw std::runtime_error("minor estate fixture could not grant land use: "+grant.reason);
        world.people_[decedent-1].health=-1;
        world.refresh_demography();
        return {decedent,minor,guardian,holding};
    }
    static std::array<uint64_t,3> prepare_multiple_heir_death(Realm& world) {
        const auto decedent=prepare_estate_claim_death(world,20);
        const auto relations=world.relations();
        const auto original_heir=std::find_if(relations.begin(),relations.end(),[&](const Edge& edge){
            return edge.kind==RelationKind::Spouse&&(edge.from==decedent||edge.to==decedent);
        });
        if(original_heir==relations.end())throw std::runtime_error("multiple-heir fixture lacks initial kin member");
        const auto first=original_heir->from==decedent?original_heir->to:original_heir->from;
        uint64_t second=0;
        for(const auto& person:world.people_)
            if(person.alive&&person.id!=decedent&&person.id!=first&&person.age_days<18*365&&
               person.family==Family::Civilian){second=person.id;break;}
        if(!second)throw std::runtime_error("multiple-heir fixture lacks a second living child");
        std::erase_if(world.edges_,[&](const Edge& edge){
            return edge.from==decedent||edge.to==decedent||edge.from==first||edge.to==first||
                   edge.from==second||edge.to==second;
        });
        world.edges_.push_back({first,decedent,RelationKind::ParentChild,true});
        world.edges_.push_back({second,decedent,RelationKind::ParentChild,true});
        for(const auto id:{first,second}){
            auto& heir=world.people_[id-1];
            world.provinces_[heir.region].firm+=heir.cash;
            heir.cash=0;heir.age_days=10*365;heir.work_assignment={};heir.school_days=0;heir.destination=-1;
        }
        world.refresh_demography();
        return {decedent,first,second};
    }
    static uint64_t set_household_fund_from_province(Realm& world,uint64_t member,int64_t amount) {
        if(member==0||member>world.people_.size()||amount<0)throw std::runtime_error("invalid household-fund fixture input");
        const auto& person=world.people_[member-1];
        const auto household=person.household_membership.household_id;
        if(!household)throw std::runtime_error("estate household-fund fixture has no household ID");
        auto& province=world.provinces_[size_t(person.region)];
        const auto currentIt=world.household_funds_.find(household);
        const int64_t current=currentIt==world.household_funds_.end()?0:currentIt->second;
        if(amount>current){const auto addition=amount-current;if(province.firm<addition)throw std::runtime_error("province cannot endow estate household fund");province.firm-=addition;}
        else province.firm+=current-amount;
        if(amount)world.household_funds_[household]=amount;else world.household_funds_.erase(household);
        return household;
    }
    static void place_in_separate_household(Realm& world,uint64_t person) {
        if(person==0||person>world.people_.size())throw std::runtime_error("invalid separate-household fixture person");
        world.people_[person-1].household_membership.household_id=person;
        world.reindex_households();
    }
    static void move_food_from_province(Realm& world,uint64_t person,int64_t quantity) {
        if(person==0||person>world.people_.size()||quantity<0)throw std::runtime_error("invalid food fixture input");
        auto& resident=world.people_[person-1];
        auto& province=world.provinces_[size_t(resident.region)];
        if(province.stock[0]<quantity)throw std::runtime_error("province cannot supply estate food fixture");
        province.stock[0]-=quantity;
        resident.inventory[0]+=quantity;
    }
    static std::array<uint64_t,3> prepare_household_authority_death(Realm& world) {
        uint64_t decedent=0,successor=0;
        for(const auto& person:world.people_){
            if(!person.alive||person.family!=Family::Civilian||person.age_days<18*365)continue;
            if(!decedent)decedent=person.id;else {successor=person.id;break;}
        }
        if(!decedent||!successor)throw std::runtime_error("household succession fixture lacks two adult civilians");
        for(auto& person:world.people_)
            if(person.id!=decedent&&person.id!=successor&&person.household_membership.household_id==decedent)
                person.household_membership.household_id=person.id;
        world.people_[successor-1].household_membership.household_id=decedent;
        world.reindex_households();
        auto& head=world.people_[decedent-1];auto& next=world.people_[successor-1];
        next.age_days=std::max(next.age_days,head.age_days+365);
        world.reindex_households();
        if(world.household_authority(decedent)!=decedent)
            throw std::runtime_error("household succession fixture head is not the current authority");
        std::erase_if(world.edges_,[&](const Edge& edge){return edge.from==decedent||edge.to==decedent||edge.from==successor||edge.to==successor;});
        world.edges_.push_back({std::min(decedent,successor),std::max(decedent,successor),RelationKind::Spouse,false});
        auto& province=world.provinces_[1];
        province.firm+=head.cash-1000;head.cash=1000;head.region=1;head.destination=-1;head.health=-1;
        province.firm+=next.cash;next.cash=0;next.region=1;next.destination=-1;
        set_household_fund_from_province(world,decedent,37);
        world.refresh_demography();
        return {decedent,successor,decedent};
    }
    static std::array<uint64_t,3> prepare_household_minor_lock_death(Realm& world,int minorAgeDays=10*365) {
        uint64_t decedent=0,minor=0,household=0;
        for(const auto& [label,members]:world.households_){
            if(label==0||label>world.people_.size())continue;
            for(const auto member:members)
                if(member!=label&&member%7!=uint64_t(world.day_%7)&&
                   member%7!=uint64_t((world.day_+1)%7)&&member%7!=uint64_t((world.day_+2)%7)){
                    decedent=label;minor=member;household=label;break;
                }
            if(decedent)break;
        }
        if(!decedent||!minor)throw std::runtime_error("minor-fund fixture lacks a household head and non-due member");
        for(auto& person:world.people_)
            if(person.id!=decedent&&person.id!=minor&&person.household_membership.household_id==household)
                person.household_membership.household_id=person.id;
        auto& head=world.people_[decedent-1];auto& child=world.people_[minor-1];
        head.family=Family::Civilian;head.age_days=40*365;head.region=1;head.destination=-1;head.health=-1;
        child.family=Family::Civilian;child.age_days=minorAgeDays;child.region=1;child.destination=-1;child.health=100;
        auto& province=world.provinces_[1];
        province.firm+=head.cash-1000;head.cash=1000;
        province.firm+=child.cash;child.cash=0;
        if(province.stock[0]<2)throw std::runtime_error("minor-fund fixture lacks DryFood for two game days");
        province.stock[0]-=2;child.inventory[0]+=2;
        std::erase_if(world.edges_,[&](const Edge& edge){return edge.from==decedent||edge.to==decedent||edge.from==minor||edge.to==minor;});
        world.edges_.push_back({minor,decedent,RelationKind::ParentChild,true});
        world.reindex_households();
        if(world.household_authority(household)!=decedent)
            throw std::runtime_error("minor-fund fixture head is not the adult household authority");
        set_household_fund_from_province(world,decedent,37);
        world.refresh_demography();
        return {decedent,minor,household};
    }
    static std::array<uint64_t,4> prepare_estate_fallback_death(Realm& world,int tier) {
        std::array<uint64_t,4> ids{};
        for(const auto& person:world.people_){
            if(!person.alive||person.family!=Family::Civilian||person.age_days<18*365||
               person.id%7==uint64_t(world.day_%7))continue;
            const bool ownsTitle=std::any_of(world.land_holdings_.begin(),world.land_holdings_.end(),
                [&](const LandHolding& holding){return holding.title_holder==person.id;});
            const bool ownsSite=std::any_of(world.establishments_.begin(),world.establishments_.end(),
                [&](const Establishment& site){return site.owner_kind==EstablishmentOwnerKind::Person&&site.owner_id==person.id;});
            if(ownsTitle||ownsSite)continue;
            auto slot=std::find(ids.begin(),ids.end(),uint64_t(0));
            if(slot==ids.end())break;
            *slot=person.id;
        }
        if(std::any_of(ids.begin(),ids.end(),[](uint64_t id){return id==0;}))
            throw std::runtime_error("estate fallback fixture lacks four living adult civilians without personal land or workshops");
        const auto decedent=ids[0],first=ids[1],second=ids[2],ancestor=ids[3];
        std::erase_if(world.edges_,[&](const Edge& edge){return edge.from==decedent||edge.to==decedent;});
        if(tier==0){
            world.edges_.push_back({decedent,first,RelationKind::ParentChild,true});
            world.edges_.push_back({decedent,second,RelationKind::ParentChild,true});
        }else if(tier==1){
            std::erase_if(world.edges_,[&](const Edge& edge){
                return edge.kind==RelationKind::ParentChild&&edge.legal_parentage&&edge.to==ancestor;
            });
            world.edges_.push_back({decedent,ancestor,RelationKind::ParentChild,true});
            world.edges_.push_back({second,ancestor,RelationKind::ParentChild,true});
            auto& parent=world.people_[ancestor-1];
            auto& province=world.provinces_[size_t(parent.region)];
            province.firm+=parent.cash;parent.cash=0;parent.alive=false;
        }else if(tier==4){
            world.edges_.push_back({first,decedent,RelationKind::ParentChild,true});
            world.edges_.push_back({decedent,second,RelationKind::Spouse,false});
        }else if(tier==3){
            world.edges_.push_back({first,decedent,RelationKind::ParentChild,false});
        }else if(tier!=2)throw std::runtime_error("unknown estate fallback fixture tier");
        auto& deceased=world.people_[decedent-1];
        auto& province=world.provinces_[1];
        province.firm+=deceased.cash-100;deceased.cash=100;
        deceased.family=Family::Civilian;deceased.age_days=40*365;deceased.region=1;
        deceased.destination=-1;deceased.health=-1;
        for(const auto id:{first,second}){
            auto& person=world.people_[id-1];person.family=Family::Civilian;person.age_days=40*365;
            person.region=1;person.destination=-1;person.health=100;
        }
        if(tier==4)world.people_[first-1].age_days=10*365;
        constexpr auto paper=size_t(GoodId::Paper);
        province.stock[paper]+=5;world.opening_[paper]+=5;
        province.stock[paper]-=5;deceased.inventory[paper]+=5;
        world.refresh_demography();
        if(!world.invariant())throw std::runtime_error("estate fallback fixture violates Realm conservation");
        return ids;
    }
    static std::array<uint64_t,2> add_estate_title_and_use_right(Realm& world,uint64_t decedent,uint64_t grantee) {
        const auto holding=world.next_id_++;
        world.land_holdings_.push_back({holding,1,1,decedent});
        const auto right=world.next_id_++;
        world.land_use_rights_.push_back({right,holding,grantee,decedent});
        if(!world.invariant())throw std::runtime_error("estate title fixture violates Realm ownership invariants");
        return {holding,right};
    }
    static std::array<uint64_t,4> prepare_full_office_vacancy(Realm& world) {
        if(world.people_.empty()||!world.people_[0].sovereign)
            throw std::runtime_error("office vacancy fixture lacks its appointing sovereign");
        std::array<uint64_t,4> candidates{};
        world.referrals_.clear();
        for(auto& person:world.people_){
            if(!person.alive||person.id==1||person.family!=Family::Civilian||person.age_days<18*365||
               person.id%7==uint64_t((world.day_+1)%7)||person.office_appointment.hired_by)continue;
            auto slot=std::find(candidates.begin(),candidates.end(),uint64_t(0));
            if(slot==candidates.end())break;
            *slot=person.id;person.candidate=true;person.health=100;
            world.referrals_.push_back(person.id);
        }
        if(std::any_of(candidates.begin(),candidates.end(),[](uint64_t id){return id==0;}))
            throw std::runtime_error("office vacancy fixture lacks four eligible adult candidates");
        auto& province=world.provinces_[size_t(world.people_[0].region)];
        if(province.treasury<4*EmploymentManager::kAppointmentFee){
            const auto needed=4*EmploymentManager::kAppointmentFee-province.treasury;
            if(province.firm<needed)throw std::runtime_error("office vacancy fixture lacks appointment funding");
            province.firm-=needed;province.treasury+=needed;
        }
        world.refresh_demography();
        if(!world.invariant())throw std::runtime_error("office vacancy fixture violates Realm accounting");
        return candidates;
    }
    static void mark_person_for_death_at_people_phase(Realm& world,uint64_t person) {
        if(person==0||person>world.people_.size()||!world.people_[person-1].alive)
            throw std::runtime_error("office death fixture has no living appointee");
        world.people_[person-1].health=-1;
    }
    static void run_people_phase(Realm& world){world.day_people();}
    static void set_reference_scenario(Realm& world,bool value){world.reference_scenario_=value;}
    static size_t fact_count(const Realm& world,uint64_t actor,int kind){
        return size_t(std::count_if(world.facts_.begin(),world.facts_.end(),[&](const Fact& fact){
            return fact.actor==actor&&fact.kind==kind;
        }));
    }
    static void prepare_age_threshold(Realm& world,uint64_t actor){
        for(auto& person:world.people_)if(person.alive)person.age_days=30*365;
        world.mutable_person(actor).age_days=75*365-1;
    }
    static void prepare_age_mortality(Realm& world,uint64_t actor){
        for(auto& person:world.people_)if(person.alive)person.age_days=30*365;
        world.mutable_person(actor).age_days=75*365;
        for(uint64_t seed=0;seed<1000000;++seed){
            std::mt19937_64 sample(seed);
            if(sample()%3650==0){world.rng_.seed(seed);return;}
        }
        throw std::runtime_error("could not find deterministic old-age mortality fixture seed");
    }
    static void prepare_starvation_boundary(Realm& world,uint64_t actor,int health,int nutrition,bool marketEnabled=false){
        auto& person=world.mutable_person(actor);
        auto& province=world.provinces_[1];
        world.transfer(person.cash,province.firm,person.cash);
        person.region=1;person.destination=-1;person.health=health;person.nutrition=nutrition;
        world.consumed_[0]+=province.stock[0]+person.inventory[0];
        person.inventory[0]=0;
        province.stock[0]=0;
        world.config_=marketEnabled?GameplayConfig{}:GameplayConfig::historical();
        if(marketEnabled)world.transfer(province.firm,person.cash,province.price[0]);
        world.refresh_demography();
    }
    static void set_timeline_contract(Realm& world,bool enabled){world.schedule_book_.timeline_contract=enabled;}
    static const std::array<int64_t,kGoodIdCount>& produced(const Realm& world){return world.produced_;}
    static void set_legacy_worker(Realm& world,uint64_t actor){
        auto& person=world.people_.at(size_t(actor-1));
        person.work_assignment={person.region,0};person.busy_until=world.day_+1;
    }
    static void set_body(Realm& world,uint64_t actor,int sleep_debt,int exertion){
        auto& body=world.people_.at(size_t(actor-1)).body_state;
        body.sleep_debt=sleep_debt;body.exertion=exertion;
    }
};
}

void test_estate_cash_claim_death_transition(){
    Realm world(29,2,128);
    const auto actor=RealmObservationTestAccess::prepare_estate_claim_death(world);
    const auto relations=world.relations();
    const auto beneficiary=std::find_if(relations.begin(),relations.end(),[&](const Edge& edge){return edge.kind==RelationKind::Spouse&&(edge.from==actor||edge.to==actor);});
    const auto beneficiary_id=beneficiary->from==actor?beneficiary->to:beneficiary->from;
    const auto beneficiary_cash=world.person(beneficiary_id).cash;
    const auto beneficiary_fiber=world.person(beneficiary_id).inventory[static_cast<size_t>(GoodId::Fiber)];
    const auto parent=std::find_if(relations.begin(),relations.end(),[&](const Edge& edge){
        return edge.kind==RelationKind::ParentChild&&edge.legal_parentage&&edge.from==actor;
    });
    check(parent!=relations.end(),"estate priority fixture has a verified living parent");
    const auto parent_id=parent->to;
    const auto parent_cash=world.person(parent_id).cash;
    const auto parent_fiber=world.person(parent_id).inventory[static_cast<size_t>(GoodId::Fiber)];
    const auto sites_before=world.establishments();
    const auto site_before=std::find_if(sites_before.begin(),sites_before.end(),[&](const Establishment& site){return site.owner_id==actor;});
    check(site_before!=sites_before.end(),"estate workshop fixture has a personal workshop for the person who will die");
    const auto inherited_establishment_id=site_before->id;
    const auto title_before=world.land_holdings();
    const auto title_it=std::max_element(title_before.begin(),title_before.end(),[&](const LandHolding& left,const LandHolding& right){
        const bool left_owned=left.title_holder==actor,right_owned=right.title_holder==actor;
        if(left_owned!=right_owned)return !left_owned;
        return left.id<right.id;
    });
    check(title_it!=title_before.end()&&title_it->title_holder==actor,"estate land fixture has a title for the person who will die");
    const auto inherited_holding_id=title_it->id;
    const auto money=world.total_money();
    check(world.invariant()&&world.estate_cases().empty(),"estate claim fixture starts invariant and living");
    RealmObservationTestAccess::run_people_phase(world);
    const auto cases=world.estate_cases();const auto loans=world.loans();
    const auto royal_funds=world.royal_funds();
    const int64_t expected_inheritance=990-(actor%7==uint64_t(world.view().day%7)?1:0)-20;
    check(!world.person(actor).alive&&cases.size()==1&&cases[0].decedent==actor,
          "Realm death phase opens the decedent's estate case");
    check(cases[0].cash==0&&cases[0].loans.size()==1&&loans.back().paid==20&&
          cases[0].inventory[static_cast<size_t>(GoodId::Fiber)]==0&&royal_funds[0]==1020&&
          world.person(beneficiary_id).cash==beneficiary_cash+expected_inheritance&&
          world.person(beneficiary_id).inventory[static_cast<size_t>(GoodId::Fiber)]==beneficiary_fiber+5&&
          world.person(parent_id).cash==parent_cash&&
          world.person(parent_id).inventory[static_cast<size_t>(GoodId::Fiber)]==parent_fiber,
          "death transition pays debt, gives the estate to the spouse, and skips a living parent in the lower-priority tier");
    check(world.invariant()&&world.total_money()==money,
          "estate claim settlement preserves world money and invariants");
    const auto holdings=world.land_holdings();
    const auto inherited=std::find_if(holdings.begin(),holdings.end(),[&](const LandHolding& holding){return holding.id==inherited_holding_id;});
    const auto rights=world.land_use_rights();
    const auto inherited_right=std::find_if(rights.begin(),rights.end(),[&](const LandUseRight& right){return right.holding_id==inherited_holding_id;});
    check(inherited!=holdings.end()&&inherited->title_holder==beneficiary_id&&inherited_right!=rights.end()&&
          inherited_right->grantor==beneficiary_id,
          "death pipeline transfers a sole-heir land title and preserves its existing use permission");
    const auto sites=world.establishments();
    const auto inherited_site=std::find_if(sites.begin(),sites.end(),[&](const Establishment& site){return site.id==inherited_establishment_id;});
    check(inherited_site!=sites.end()&&inherited_site->owner_kind==EstablishmentOwnerKind::Person&&
          inherited_site->owner_id==beneficiary_id&&inherited_site->cash==10&&
          inherited_site->work_authorizations==site_before->work_authorizations,
          "death pipeline transfers the personal workshop while preserving its account and worker authorization");
    const auto owner_command=world.execute(EstablishmentTransferCommand{beneficiary_id,
        world.person(beneficiary_id).decisions+1,inherited_establishment_id,
        EstablishmentTransferOperation::WithdrawCash,-1,1});
    check(owner_command.success&&world.person(beneficiary_id).cash==beneficiary_cash+expected_inheritance+1&&world.invariant(),
          "new adult owner can manage the inherited workshop through the existing Realm command");
    const auto saved=world.serialize("estate-cash-claims");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,"estate-cash-claims",error),error.c_str());
    check(restored.invariant()&&restored.fingerprint()==world.fingerprint()&&
          restored.estate_cases()==cases&&restored.loans().back().paid==20&&restored.land_holdings()==holdings&&
          restored.land_use_rights()==rights&&restored.establishments().size()==sites.size(),
          "distributed estate, inherited land title, use permission, workshop and updated loan survive save/load exactly");
}

void test_inherited_workshop_continues_work_and_production(){
    Realm world(41,2,128);
    const auto decedent=RealmObservationTestAccess::prepare_estate_claim_death(world);

    const auto relations=world.relations();
    const auto spouse_it=std::find_if(relations.begin(),relations.end(),[&](const Edge& edge){
        return edge.kind==RelationKind::Spouse&&(edge.from==decedent||edge.to==decedent);
    });
    check(spouse_it!=relations.end(),"continuation fixture has its sole adult heir");
    const auto heir=spouse_it->from==decedent?spouse_it->to:spouse_it->from;
    const auto sites_before=world.establishments();
    const auto site_before=std::find_if(sites_before.begin(),sites_before.end(),[&](const Establishment& site){return site.owner_id==decedent;});
    check(site_before!=sites_before.end(),"continuation fixture has the decedent's workshop");
    const auto site_id=site_before->id;
    const auto stocked=world.execute(EstablishmentTransferCommand{decedent,world.person(decedent).decisions+1,
        site_id,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fiber),2});
    check(stocked.success,"decedent puts real Fiber into the workshop before death");
    const auto scheduled=world.execute(ScheduleCommand{heir,world.person(heir).decisions+1,
        ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,site_before->region,site_id});
    check(scheduled.success,"authorized heir schedules a real work shift before succession");
    const auto money_before=world.total_money();
    RealmObservationTestAccess::run_people_phase(world);
    const auto heir_cash_before_work=world.person(heir).cash;
    const auto inherited_sites=world.establishments();
    const auto inherited=std::find_if(inherited_sites.begin(),inherited_sites.end(),[&](const Establishment& site){return site.id==site_id;});
    check(!world.person(decedent).alive&&world.loans().back().paid==20&&inherited!=inherited_sites.end()&&inherited->owner_id==heir&&
          inherited->cash==10&&inherited->stock[size_t(GoodId::Fiber)]==2&&
          inherited->work_authorizations==site_before->work_authorizations,
          "adult heir receives the same funded and authorized workshop after the debt is settled");
    world.advance_ticks(64);
    const auto sites_after=world.establishments();
    const auto site_it=std::find_if(sites_after.begin(),sites_after.end(),[&](const Establishment& row){return row.id==site_id;});
    check(site_it!=sites_after.end(),"inherited workshop remains after its work day");
    const auto& site=*site_it;
    const auto attendance=world.schedule_book().work_present_ticks_by_establishment.find({heir,site_id});
    const auto paid=site.work_pay.paid_units.find(heir);
    check(world.view().day==1&&attendance!=world.schedule_book().work_present_ticks_by_establishment.end()&&attendance->second==64&&
          paid!=site.work_pay.paid_units.end()&&paid->second==4,
          "the inherited-workshop shift records 64 physical ticks and four paid work units");
    check(site.cash==6&&world.person(heir).cash==heir_cash_before_work+4&&
          site.stock[size_t(GoodId::Fiber)]==0&&site.stock[size_t(GoodId::Cloth)]==2&&
          world.total_money()==money_before&&world.invariant(),
          "site cash funds wages, two Fiber become two Cloth, and money conservation holds");
    const int region=site.region;
    const auto cloth=size_t(GoodId::Cloth);
    uint64_t buyer=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto&person=world.person(id);
        if(id!=heir&&person.alive&&person.destination==-1&&person.region==region&&
           person.age_days>=6*365&&person.cash>=world.view().provinces[size_t(region)].price[cloth]+1){buyer=id;break;}
    }
    check(buyer>0,"inherited-workshop market fixture has a local funded buyer");
    const auto withdrawn=world.execute(EstablishmentTransferCommand{heir,world.person(heir).decisions+1,
        site_id,EstablishmentTransferOperation::WithdrawGoods,int(GoodId::Cloth),1});
    check(withdrawn.success&&world.person(heir).inventory[cloth]>0&&
          world.establishments().front().stock[cloth]==1,
          "new workshop owner withdraws one produced Cloth through the owner command");
    const auto market_before_sale=world.view().provinces[size_t(region)];
    const auto heir_cash_before_sale=world.person(heir).cash;
    const int sale_price=std::max(1,market_before_sale.price[cloth]-1);
    const auto sold=world.execute({heir,world.person(heir).decisions+1,
        {Intent::Sell,region,int(GoodId::Cloth),1,0}});
    const auto market_after_sale=world.view().provinces[size_t(region)];
    check(sold.success&&world.person(heir).cash==heir_cash_before_sale+sale_price&&
          market_after_sale.firm==market_before_sale.firm-sale_price&&
          market_after_sale.stock[cloth]==market_before_sale.stock[cloth]+1&&
          market_after_sale.tax_receipts==market_before_sale.tax_receipts&&world.invariant(),
          "inherited workshop output reaches the local market through a real sale");
    const auto market_before_buy=world.view().provinces[size_t(region)];
    const auto buyer_cash_before=world.person(buyer).cash;
    const auto buyer_inventory_before=world.person(buyer).inventory[cloth];
    const int64_t gross=market_before_buy.price[cloth]+1;
    const int64_t tax_numerator=gross*market_before_buy.tax+market_before_buy.sales_tax_remainder;
    const int64_t expected_tax=tax_numerator/100;
    const int expected_remainder=int(tax_numerator%100);
    const auto bought=world.execute({buyer,world.person(buyer).decisions+1,
        {Intent::Buy,region,int(GoodId::Cloth),1,0}});
    const auto market_after_buy=world.view().provinces[size_t(region)];
    check(bought.success&&world.person(buyer).cash==buyer_cash_before-gross&&
          world.person(buyer).inventory[cloth]==buyer_inventory_before+1&&
          market_after_buy.stock[cloth]==market_before_buy.stock[cloth]-1,
          "local buyer purchases inherited workshop output at current market price");
    check(market_after_buy.treasury==market_before_buy.treasury+expected_tax&&
          market_after_buy.tax_receipts==market_before_buy.tax_receipts+expected_tax&&
          market_after_buy.sales_tax_remainder==expected_remainder&&
          market_after_buy.firm==market_before_buy.firm+gross-expected_tax&&
          world.total_money()==money_before&&world.invariant(),
          "market purchase records the existing sales tax and conserves money");
    const auto save=world.serialize("inherited-workshop-continuation");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(save,"inherited-workshop-continuation",error),error.c_str());
    check(restored.fingerprint()==world.fingerprint()&&restored.invariant()&&
          restored.establishments().size()==sites_after.size()&&
          restored.establishments().front().id==site_id&&restored.establishments().front().owner_id==heir&&
          restored.establishments().front().cash==world.establishments().front().cash&&
          restored.establishments().front().stock==world.establishments().front().stock&&
          restored.establishments().front().work_pay.paid_units==site.work_pay.paid_units&&
          restored.establishments().front().processed_work_units==site.processed_work_units&&
          restored.schedule_book().work_present_ticks_by_establishment==world.schedule_book().work_present_ticks_by_establishment&&
          restored.person(buyer).cash==world.person(buyer).cash&&
          restored.person(buyer).inventory[cloth]==world.person(buyer).inventory[cloth]&&
          restored.view().provinces[size_t(region)].tax_receipts==market_after_buy.tax_receipts&&
          restored.view().provinces[size_t(region)].sales_tax_remainder==market_after_buy.sales_tax_remainder&&
          restored.view().provinces[size_t(region)].stock[cloth]==market_after_buy.stock[cloth],
          "work, wages, production, sale, tax and inherited ownership survive save/load exactly");
}
void test_estate_keeps_household_fund_outside_personal_claim(){
    Realm world(43,2,128);
    const auto decedent=RealmObservationTestAccess::prepare_estate_claim_death(world);
    const auto household=world.household_of(decedent);
    const auto relations=world.relations();
    const auto spouse=std::find_if(relations.begin(),relations.end(),[&](const Edge& edge){
        return edge.kind==RelationKind::Spouse&&(edge.from==decedent||edge.to==decedent);
    });
    check(spouse!=relations.end(),"fund isolation fixture has one living adult heir");
    const auto heir=spouse->from==decedent?spouse->to:spouse->from;
    if(world.household_of(heir)==household){
        RealmObservationTestAccess::place_in_separate_household(world,heir);
    }
    const auto heir_household=world.household_of(heir);
    check(heir_household!=household,"estate fixture keeps the spouse in a separate household");
    RealmObservationTestAccess::set_household_fund_from_province(world,decedent,37);
    RealmObservationTestAccess::set_household_fund_from_province(world,heir,23);
    RealmObservationTestAccess::move_food_from_province(world,heir,1);
    const auto heir_cash=world.person(heir).cash;
    const auto total_money=world.total_money();
    check(world.household_fund(household)==37&&world.household_fund(heir_household)==23&&world.invariant(),
          "both separate household funds start funded without creating money");
    RealmObservationTestAccess::run_people_phase(world);
    const auto cases=world.estate_cases();
    const auto estate=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& item){return item.decedent==decedent;});
    const int64_t personal_inheritance=990-(decedent%7==uint64_t(world.view().day%7)?1:0)-20;
    check(estate!=cases.end()&&estate->cash==0&&world.person(decedent).cash==0&&
          world.person(heir).cash==heir_cash+personal_inheritance,
          "only personal cash after loan repayment is distributed to the heir");
    check(world.household_fund(household)==37,
          "death settlement leaves the decedent household account outside the personal estate");
    check(world.household_fund(heir_household)==23,
          "death settlement leaves the heir household account outside the personal estate");
    check(world.total_money()==total_money&&world.invariant(),
          "death settlement conserves all money with two separately funded households");
    const auto save=world.serialize("estate-household-fund-isolation");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(save,"estate-household-fund-isolation",error),error.c_str());
    check(restored.household_fund(household)==37&&restored.household_fund(heir_household)==23&&
          restored.estate_cases()==cases&&
          restored.person(decedent).cash==0&&restored.person(heir).cash==heir_cash+personal_inheritance&&
          restored.total_money()==total_money&&restored.fingerprint()==world.fingerprint(),
          "save/load preserves the shared fund separately from the heir's personal money");
}
void test_household_fund_survives_authority_death(){
    Realm world(53,2,128);
    const auto fixture=RealmObservationTestAccess::prepare_household_authority_death(world);
    const auto decedent=fixture[0],successor=fixture[1],household=fixture[2];
    const auto totalMoney=world.total_money();
    check(world.household_authority(household)==decedent&&world.household_fund(household)==37&&world.invariant(),
          "funded household starts with its adult head as the authorized account holder");
    RealmObservationTestAccess::run_people_phase(world);
    const auto cases=world.estate_cases();
    const auto estate=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& item){return item.decedent==decedent;});
    check(!world.person(decedent).alive&&estate!=cases.end(),
          "household head dies and receives a separate personal estate case");
    check(estate->cash==0,"personal estate cash is distributed without moving household money");
    check(world.household_authority(household)==successor,
          "the living adult becomes account authority after the household head dies");
    check(world.household_fund(household)==37,
          "common fund retains its balance when the account authority dies");
    check(world.total_money()==totalMoney&&world.invariant(),
          "authority death preserves total money and the Realm ledger invariant");
    const auto successorCash=world.person(successor).cash;
    const auto withdrawal=world.execute(HouseholdFundCommand{successor,world.person(successor).decisions+1,
        household,HouseholdFundOperation::Withdraw,5});
    check(withdrawal.success&&world.household_fund(household)==32&&world.person(successor).cash==successorCash+5&&
          world.total_money()==totalMoney&&world.invariant(),
          "the replacement authority can withdraw only the requested amount through the Realm command");
    const auto save=world.serialize("household-authority-after-death");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(save,"household-authority-after-death",error),error.c_str());
    check(restored.household_fund(household)==32&&restored.household_authority(household)==successor&&
          restored.estate_cases()==world.estate_cases()&&restored.person(successor).cash==successorCash+5&&
          restored.total_money()==totalMoney&&restored.fingerprint()==world.fingerprint(),
          "save/load preserves the household account, successor authority and personal estate separately");
}
void test_household_fund_stays_locked_with_only_minor_survivor(){
    Realm world(59,2,128);
    const auto fixture=RealmObservationTestAccess::prepare_household_minor_lock_death(world);
    const auto decedent=fixture[0],minor=fixture[1],household=fixture[2];
    const auto totalMoney=world.total_money();
    check(world.household_authority(household)==decedent&&world.household_fund(household)==37&&world.invariant(),
          "minor-lock fixture begins with adult authority and a conserved shared fund");
    RealmObservationTestAccess::run_people_phase(world);
    const auto cases=world.estate_cases();
    const auto estate=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& item){return item.decedent==decedent;});
    check(!world.person(decedent).alive&&world.person(minor).alive&&world.person(minor).age_days<18*365&&estate!=cases.end(),
          "adult dies and the only surviving household member remains a minor heir");
    check(estate->cash==0&&world.person(minor).cash>0&&world.household_authority(household)==0&&
          world.household_fund(household)==37&&world.total_money()==totalMoney&&world.invariant(),
          "minor inherits personal cash while shared funds remain locked without an adult authority");
    const auto before=world.fingerprint();
    const auto cashBefore=world.person(minor).cash;
    const auto rejected=world.execute(HouseholdFundCommand{minor,world.person(minor).decisions+1,
        household,HouseholdFundOperation::Withdraw,5});
    check(!rejected.success&&world.fingerprint()==before&&world.person(minor).cash==cashBefore&&
          world.household_fund(household)==37&&world.total_money()==totalMoney&&world.invariant(),
          "minor withdrawal is rejected atomically without taking personal or shared money");
    const auto save=world.serialize("minor-household-fund-lock");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(save,"minor-household-fund-lock",error),error.c_str());
    check(restored.household_fund(household)==37&&restored.household_authority(household)==0&&
          restored.estate_cases()==world.estate_cases()&&restored.person(minor).cash==cashBefore&&
          restored.total_money()==totalMoney&&restored.fingerprint()==world.fingerprint(),
          "save/load preserves a locked household fund separately from the minor's inheritance");
}
void test_household_fund_authority_unlocks_at_adulthood(){
    Realm world(60,2,128);
    const auto fixture=RealmObservationTestAccess::prepare_household_minor_lock_death(world,18*365-3);
    const auto decedent=fixture[0],member=fixture[1],household=fixture[2];
    const auto totalMoney=world.total_money();
    RealmObservationTestAccess::run_people_phase(world);
    check(!world.person(decedent).alive&&world.person(member).age_days==18*365-2&&
          world.household_authority(household)==0&&world.household_fund(household)==37,
          "after the adult dies, the underage survivor has no authority and the shared fund stays locked");
    world.advance(2);
    check(world.person(member).age_days==18*365&&world.person(member).alive&&
          world.household_authority(household)==member&&world.household_fund(household)==37&&
          world.person(member).inventory[0]==2&&world.invariant(),
          "two Realm game days bring the survivor to adulthood without spending the household fund");
    const auto personalCash=world.person(member).cash;
    const auto withdrawal=world.execute(HouseholdFundCommand{member,world.person(member).decisions+1,
        household,HouseholdFundOperation::Withdraw,5});
    check(withdrawal.success&&world.household_fund(household)==32&&
          world.person(member).cash==personalCash+5&&world.total_money()==totalMoney&&world.invariant(),
          "the newly adult authority withdraws five through Realm, conserving all money");
    const auto save=world.serialize("household-authority-after-maturity");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(save,"household-authority-after-maturity",error),error.c_str());
    check(restored.household_authority(household)==member&&restored.household_fund(household)==32&&
          restored.person(member).age_days==18*365&&restored.person(member).cash==personalCash+5&&
          restored.total_money()==totalMoney&&restored.invariant()&&restored.fingerprint()==world.fingerprint(),
          "save/load preserves newly gained adult authority and the completed withdrawal");
}
void test_estate_parent_sibling_and_public_fallback_through_realm(){
    const auto check_saved=[](const Realm& world,const std::string& label){
        const auto save=world.serialize(label);Realm restored(1,1,128);std::string error;
        check(restored.deserialize(save,label,error),error.c_str());
        check(restored.fingerprint()==world.fingerprint()&&restored.total_money()==world.total_money()&&
              restored.invariant(),"estate fallback save/load retains distributions and balances");
    };
    {
        Realm world(61,2,128);const auto ids=RealmObservationTestAccess::prepare_estate_fallback_death(world,0);
        const auto decedent=ids[0],first=ids[1],second=ids[2];
        const auto money=world.total_money();const auto estateMoney=world.balances()[4];
        const auto firstCash=world.person(first).cash,secondCash=world.person(second).cash;
        const auto firstPaper=world.person(first).inventory[size_t(GoodId::Paper)];
        const auto secondPaper=world.person(second).inventory[size_t(GoodId::Paper)];
        RealmObservationTestAccess::run_people_phase(world);
        const auto low=std::min(first,second),high=std::max(first,second);
        check(!world.person(decedent).alive&&
              world.person(low).cash==(low==first?firstCash:secondCash)+50&&
              world.person(high).cash==(high==first?firstCash:secondCash)+50,
              "when no child or spouse survives, both living parents receive equal cash shares");
        check(world.person(low).inventory[size_t(GoodId::Paper)]==(low==first?firstPaper:secondPaper)+3,
              "the lower-ID parent receives the indivisible Paper remainder");
        check(world.person(high).inventory[size_t(GoodId::Paper)]==(high==first?firstPaper:secondPaper)+2,
              "the higher-ID parent receives the even Paper share");
        const auto cases=world.estate_cases();
        const auto record=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& item){return item.decedent==decedent;});
        check(record!=cases.end()&&record->cash==0&&world.balances()[4]==estateMoney&&
              world.total_money()==money&&world.invariant(),
              "parent distribution removes all cash from the estate while preserving total money and invariants");
        check_saved(world,"realm-estate-parent-fallback");
    }
    {
        Realm world(62,2,128);const auto ids=RealmObservationTestAccess::prepare_estate_fallback_death(world,1);
        const auto decedent=ids[0],sibling=ids[2],ancestor=ids[3];
        const auto money=world.total_money();const auto estateMoney=world.balances()[4];
        const auto cash=world.person(sibling).cash,paper=world.person(sibling).inventory[size_t(GoodId::Paper)];
        RealmObservationTestAccess::run_people_phase(world);
        check(!world.person(decedent).alive&&!world.person(ancestor).alive&&world.person(sibling).cash==cash+100&&
              world.person(sibling).inventory[size_t(GoodId::Paper)]==paper+5&&
              world.balances()[4]==estateMoney&&world.total_money()==money&&world.invariant(),
              "when parents are dead and no child or spouse survives, the living sibling receives the estate");
        check_saved(world,"realm-estate-sibling-fallback");
    }
    {
        Realm world(63,2,128);const auto ids=RealmObservationTestAccess::prepare_estate_fallback_death(world,2);
        const auto decedent=ids[0];const auto money=world.total_money();
        const auto estateMoney=world.balances()[4];const auto region=world.person(decedent).region;
        const auto paperBefore=world.view().provinces[size_t(region)].stock[size_t(GoodId::Paper)];
        RealmObservationTestAccess::run_people_phase(world);
        const auto paperAfter=world.view().provinces[size_t(region)].stock[size_t(GoodId::Paper)];
        const auto cases=world.estate_cases();
        const auto record=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& item){return item.decedent==decedent;});
        check(!world.person(decedent).alive&&record!=cases.end()&&record->cash==0&&
              world.balances()[4]==estateMoney+100&&
              paperAfter==paperBefore+5&&world.total_money()==money&&world.invariant(),
              "with no living legal heirs, the public estate account keeps the money and Province receives the goods");
        check_saved(world,"realm-estate-public-fallback");
    }
}
void test_estate_unverified_parentage_does_not_inherit_through_realm(){
    Realm world(64,2,128);
    const auto ids=RealmObservationTestAccess::prepare_estate_fallback_death(world,3);
    const auto decedent=ids[0],unverifiedChild=ids[1];
    const auto cash=world.person(unverifiedChild).cash;
    const auto paper=world.person(unverifiedChild).inventory[size_t(GoodId::Paper)];
    const auto totalMoney=world.total_money();const auto estateMoney=world.balances()[4];
    const auto region=world.person(decedent).region;
    const auto publicPaper=world.view().provinces[size_t(region)].stock[size_t(GoodId::Paper)];
    const auto relations=world.relations();
    const auto relation=std::find_if(relations.begin(),relations.end(),[&](const Edge& edge){
        return edge.from==unverifiedChild&&edge.to==decedent&&edge.kind==RelationKind::ParentChild;
    });
    check(relation!=relations.end()&&!relation->legal_parentage,
          "unverified parentage fixture carries a typed but non-legal ParentChild edge");
    RealmObservationTestAccess::run_people_phase(world);
    const auto cases=world.estate_cases();
    const auto estate=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& item){return item.decedent==decedent;});
    check(!world.person(decedent).alive&&world.person(unverifiedChild).alive&&
          world.person(unverifiedChild).cash==cash&&
          world.person(unverifiedChild).inventory[size_t(GoodId::Paper)]==paper,
          "an unverified parent-child relationship receives no cash or goods inheritance");
    check(estate!=cases.end()&&estate->cash==0&&estate->inventory[size_t(GoodId::Paper)]==0&&
          world.balances()[4]==estateMoney+100&&
          world.view().provinces[size_t(region)].stock[size_t(GoodId::Paper)]==publicPaper+5&&
          world.total_money()==totalMoney&&world.invariant(),
          "the legal-heir fallback routes money to the public estate and goods to the local Province");
    const auto save=world.serialize("estate-unverified-parentage");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(save,"estate-unverified-parentage",error),error.c_str());
    check(restored.fingerprint()==world.fingerprint()&&restored.estate_cases()==world.estate_cases()&&restored.invariant(),
          "save/load preserves unverified parentage and the public estate distribution");
}
void test_estate_legal_child_precedes_spouse_through_realm(){
    Realm world(65,2,128);
    const auto ids=RealmObservationTestAccess::prepare_estate_fallback_death(world,4);
    const auto decedent=ids[0],child=ids[1],spouse=ids[2];
    const auto childCash=world.person(child).cash;
    const auto childPaper=world.person(child).inventory[size_t(GoodId::Paper)];
    const auto spouseCash=world.person(spouse).cash;
    const auto spousePaper=world.person(spouse).inventory[size_t(GoodId::Paper)];
    const auto totalMoney=world.total_money();
    const auto relations=world.relations();
    check(std::any_of(relations.begin(),relations.end(),[&](const Edge& edge){
              return edge.from==child&&edge.to==decedent&&edge.kind==RelationKind::ParentChild&&edge.legal_parentage;
          })&&std::any_of(relations.begin(),relations.end(),[&](const Edge& edge){
              return edge.kind==RelationKind::Spouse&&((edge.from==decedent&&edge.to==spouse)||(edge.from==spouse&&edge.to==decedent));
          }),"priority fixture contains a verified child and a living spouse");
    RealmObservationTestAccess::run_people_phase(world);
    const auto cases=world.estate_cases();
    const auto estate=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& item){return item.decedent==decedent;});
    check(!world.person(decedent).alive&&world.person(child).alive&&world.person(spouse).alive&&
          world.person(child).cash==childCash+100&&
          world.person(child).inventory[size_t(GoodId::Paper)]==childPaper+5&&
          world.person(spouse).cash==spouseCash&&
          world.person(spouse).inventory[size_t(GoodId::Paper)]==spousePaper,
          "a verified living child receives the complete estate before the living spouse");
    check(estate!=cases.end()&&estate->cash==0&&estate->inventory[size_t(GoodId::Paper)]==0&&
          world.total_money()==totalMoney&&world.invariant(),
          "the child-first distribution empties estate cash and goods without changing total money");
    const auto save=world.serialize("estate-child-before-spouse");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(save,"estate-child-before-spouse",error),error.c_str());
    check(restored.fingerprint()==world.fingerprint()&&restored.estate_cases()==cases&&
          restored.person(child).cash==childCash+100&&
          restored.person(child).inventory[size_t(GoodId::Paper)]==childPaper+5&&
          restored.person(spouse).cash==spouseCash&&restored.invariant(),
          "save/load preserves the child-first inheritance and leaves spouse assets unchanged");
}
void test_estate_fallback_land_titles_through_realm(){
    const auto check_saved=[](const Realm& world,const std::string& label){
        const auto save=world.serialize(label);Realm restored(1,1,128);std::string error;
        check(restored.deserialize(save,label,error),error.c_str());
        check(restored.fingerprint()==world.fingerprint()&&restored.land_holdings()==world.land_holdings()&&
              restored.land_use_rights()==world.land_use_rights()&&restored.estate_cases()==world.estate_cases()&&
              restored.invariant(),"estate fallback land title, use permission and case survive save/load");
    };
    {
        Realm world(71,2,128);const auto ids=RealmObservationTestAccess::prepare_estate_fallback_death(world,0);
        const auto decedent=ids[0],grantee=ids[1];
        const auto records=RealmObservationTestAccess::add_estate_title_and_use_right(world,decedent,grantee);
        const auto holding=records[0],right=records[1];const auto money=world.total_money();
        RealmObservationTestAccess::run_people_phase(world);
        const auto holdings=world.land_holdings();const auto rights=world.land_use_rights();const auto cases=world.estate_cases();
        const auto title=std::find_if(holdings.begin(),holdings.end(),[&](const LandHolding& row){return row.id==holding;});
        const auto use=std::find_if(rights.begin(),rights.end(),[&](const LandUseRight& row){return row.id==right;});
        const auto estate=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& row){return row.decedent==decedent;});
        check(title!=holdings.end()&&title->title_holder==decedent&&use!=rights.end()&&use->grantor==decedent&&
              use->grantee==grantee&&estate!=cases.end()&&estate->land_holdings==std::vector<uint64_t>{holding}&&
              estate->land_use_rights==std::vector<uint64_t>{right}&&world.invariant()&&world.total_money()==money,
              "two living parents share the heir tier, so the indivisible holding and its current permission stay in the estate");
        check_saved(world,"realm-parent-tier-land-retention");
    }
    {
        Realm world(72,2,128);const auto ids=RealmObservationTestAccess::prepare_estate_fallback_death(world,1);
        const auto decedent=ids[0],sibling=ids[2],grantee=ids[1];
        const auto records=RealmObservationTestAccess::add_estate_title_and_use_right(world,decedent,grantee);
        const auto holding=records[0],right=records[1];const auto money=world.total_money();
        RealmObservationTestAccess::run_people_phase(world);
        const auto holdings=world.land_holdings();const auto rights=world.land_use_rights();const auto cases=world.estate_cases();
        const auto title=std::find_if(holdings.begin(),holdings.end(),[&](const LandHolding& row){return row.id==holding;});
        const auto use=std::find_if(rights.begin(),rights.end(),[&](const LandUseRight& row){return row.id==right;});
        const auto estate=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& row){return row.decedent==decedent;});
        check(title!=holdings.end()&&title->title_holder==sibling&&use!=rights.end()&&use->grantor==sibling&&
              use->grantee==grantee&&estate!=cases.end()&&estate->land_holdings.empty()&&
              estate->land_use_rights.empty()&&world.invariant()&&world.total_money()==money,
              "one living adult sibling receives the holding and becomes grantor without displacing its land user");
        check_saved(world,"realm-sibling-tier-land-transfer");
    }
    {
        Realm world(73,2,128);const auto ids=RealmObservationTestAccess::prepare_estate_fallback_death(world,2);
        const auto decedent=ids[0],grantee=ids[1];
        const auto records=RealmObservationTestAccess::add_estate_title_and_use_right(world,decedent,grantee);
        const auto holding=records[0],right=records[1];const auto money=world.total_money();
        RealmObservationTestAccess::run_people_phase(world);
        const auto holdings=world.land_holdings();const auto rights=world.land_use_rights();const auto cases=world.estate_cases();
        const auto title=std::find_if(holdings.begin(),holdings.end(),[&](const LandHolding& row){return row.id==holding;});
        const auto use=std::find_if(rights.begin(),rights.end(),[&](const LandUseRight& row){return row.id==right;});
        const auto estate=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& row){return row.decedent==decedent;});
        check(title!=holdings.end()&&title->title_holder==decedent&&use!=rights.end()&&use->grantor==decedent&&
              use->grantee==grantee&&estate!=cases.end()&&estate->land_holdings==std::vector<uint64_t>{holding}&&
              estate->land_use_rights==std::vector<uint64_t>{right}&&world.invariant()&&world.total_money()==money,
              "without a legal heir, land title and the living user's permission remain attached to the estate case");
        check_saved(world,"realm-public-tier-land-retention");
    }
}
void test_deceased_appointee_releases_authority_office_slot(){
    Realm world(80,2,128);
    const auto candidates=RealmObservationTestAccess::prepare_full_office_vacancy(world);
    const auto totalMoney=world.total_money();
    const auto appointmentsInUse=[&](const Realm& realm){
        int count=0;
        for(uint64_t id=1;id<=realm.count();++id){
            const auto& person=realm.person(id);
            count+=person.alive&&person.office_appointment.office&&person.office_appointment.hired_by==1;
        }
        return count;
    };
    for(int slot=0;slot<3;++slot){
        const auto result=world.execute({1,world.person(1).decisions+1,world.candidates(1)[15]});
        check(result.success&&appointmentsInUse(world)==slot+1&&world.invariant(),
              "appointing authority fills each of its three funded office positions");
    }
    const auto full=world.fingerprint();
    const auto rejected=world.execute({1,world.person(1).decisions+1,world.candidates(1)[15]});
    check(!rejected.success&&world.fingerprint()==full&&appointmentsInUse(world)==3,
          "fourth appointment is rejected atomically when the authority has no open position");
    const auto deceased=candidates[0];
    RealmObservationTestAccess::mark_person_for_death_at_people_phase(world,deceased);
    world.advance(1);
    check(!world.person(deceased).alive&&world.person(deceased).office_appointment.office&&
          world.person(deceased).office_appointment.hired_by==1&&appointmentsInUse(world)==2&&world.invariant(),
          "death leaves the person's historical appointment on their record but frees the active office position");
    const auto replacement=world.execute({1,world.person(1).decisions+1,world.candidates(1)[15]});
    check(replacement.success&&world.person(candidates[3]).office_appointment.office&&
          world.person(candidates[3]).office_appointment.hired_by==1&&appointmentsInUse(world)==3&&
          world.person(deceased).office_appointment.hired_by==1&&world.total_money()==totalMoney&&world.invariant(),
          "authority appoints the waiting candidate into the vacancy without transferring the dead person's record");
    const auto save=world.serialize("death-releases-appointed-office");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(save,"death-releases-appointed-office",error),error.c_str());
    check(restored.fingerprint()==world.fingerprint()&&appointmentsInUse(restored)==3&&restored.invariant(),
          "save/load preserves the active replacements and deceased person's historical appointment");
    world.advance(1);restored.advance(1);
    check(restored.fingerprint()==world.fingerprint()&&appointmentsInUse(restored)==3&&
          world.total_money()==totalMoney&&restored.invariant(),
          "Realm continuation after load preserves the vacancy decision without salary duplication");
}
void test_estate_does_not_mutate_other_living_property(){
    Realm world(47,2,128);
    const auto decedent=RealmObservationTestAccess::prepare_estate_claim_death(world);
    RealmObservationTestAccess::add_living_property_for_isolation(world,decedent);
    std::vector<LandHolding> livingTitles;
    for(const auto& title:world.land_holdings())if(title.title_holder!=decedent)livingTitles.push_back(title);
    std::vector<LandUseRight> livingRights;
    for(const auto& right:world.land_use_rights())if(right.grantor!=decedent)livingRights.push_back(right);
    std::vector<Establishment> livingSites;
    for(const auto& site:world.establishments())
        if(site.owner_kind!=EstablishmentOwnerKind::Person||site.owner_id!=decedent)livingSites.push_back(site);
    const auto totalMoney=world.total_money();
    check(!livingTitles.empty()&&!livingSites.empty()&&world.invariant(),
          "living-property isolation fixture has separate title, workshop and valid ledger");
    RealmObservationTestAccess::run_people_phase(world);
    const auto titlesAfter=world.land_holdings();
    const auto rightsAfter=world.land_use_rights();
    const auto sitesAfter=world.establishments();
    const auto sameSite=[](const Establishment& left,const Establishment& right){
        return left.id==right.id&&left.region==right.region&&left.owner_kind==right.owner_kind&&left.owner_id==right.owner_id&&
               left.industry==right.industry&&left.capacity==right.capacity&&left.cash==right.cash&&left.stock==right.stock&&left.authorization_window_count==right.authorization_window_count&&
               left.last_production_day==right.last_production_day&&left.processed_work_units==right.processed_work_units&&
               left.work_authorizations==right.work_authorizations&&left.work_pay.legacy_credit==right.work_pay.legacy_credit&&
               left.work_pay.paid_units==right.work_pay.paid_units&&left.work_pay.remainders==right.work_pay.remainders;
    };
    const bool titlesUnchanged=std::all_of(livingTitles.begin(),livingTitles.end(),[&](const LandHolding& old){
        const auto current=std::find_if(titlesAfter.begin(),titlesAfter.end(),[&](const LandHolding& row){return row.id==old.id;});
        return current!=titlesAfter.end()&&*current==old;
    });
    const bool rightsUnchanged=std::all_of(livingRights.begin(),livingRights.end(),[&](const LandUseRight& old){
        const auto current=std::find_if(rightsAfter.begin(),rightsAfter.end(),[&](const LandUseRight& row){return row.id==old.id;});
        return current!=rightsAfter.end()&&*current==old;
    });
    const bool sitesUnchanged=std::all_of(livingSites.begin(),livingSites.end(),[&](const Establishment& old){
        const auto current=std::find_if(sitesAfter.begin(),sitesAfter.end(),[&](const Establishment& row){return row.id==old.id;});
        return current!=sitesAfter.end()&&sameSite(old,*current);
    });
    check(titlesUnchanged&&rightsUnchanged&&sitesUnchanged,
          "another living owner's land, use permission and workshop records remain byte-for-byte equivalent in their fields");
    check(world.total_money()==totalMoney&&world.invariant(),
          "settling one person's estate preserves total money and Realm invariants");
    const auto save=world.serialize("estate-living-property-isolation");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(save,"estate-living-property-isolation",error),error.c_str());
    check(restored.fingerprint()==world.fingerprint()&&restored.invariant(),
          "living property and estate settlement survive save/load together");
}
void test_estate_underfunded_claim_death_transition(){
    Realm world(31,2,128);
    const auto actor=RealmObservationTestAccess::prepare_estate_claim_death(world,2000);
    const auto relations=world.relations();
    const auto spouse=std::find_if(relations.begin(),relations.end(),[&](const Edge& edge){
        return edge.kind==RelationKind::Spouse&&(edge.from==actor||edge.to==actor);
    });
    check(spouse!=relations.end(),"underfunded estate fixture has a living spouse heir");
    const auto beneficiary=spouse->from==actor?spouse->to:spouse->from;
    const auto beneficiary_cash=world.person(beneficiary).cash;
    const auto estate_money=world.person(actor).cash;
    const auto sites_before=world.establishments();
    const auto site_before=std::find_if(sites_before.begin(),sites_before.end(),[&](const Establishment& site){return site.owner_id==actor;});
    const auto holdings_before=world.land_holdings();
    const auto rights_before=world.land_use_rights();
    const auto holding_before=std::find_if(holdings_before.begin(),holdings_before.end(),[&](const LandHolding& row){
        return row.title_holder==actor&&std::any_of(rights_before.begin(),rights_before.end(),
            [&](const LandUseRight& right){return right.holding_id==row.id;});
    });
    check(site_before!=sites_before.end()&&holding_before!=holdings_before.end()&&
          world.invariant()&&world.estate_cases().empty(),"underfunded estate starts with title, workshop, claim and valid Realm");
    const auto site_id=site_before->id,holding_id=holding_before->id,loan_id=world.loans().back().id;
    const auto total_money=world.total_money();
    RealmObservationTestAccess::run_people_phase(world);
    const auto cases=world.estate_cases();
    const auto loans=world.loans();
    const auto case_it=std::find_if(cases.begin(),cases.end(),[&](const EstateCase& item){return item.decedent==actor;});
    const auto loan_it=std::find_if(loans.begin(),loans.end(),[&](const Loan& item){return item.id==loan_id;});
    const auto titles=world.land_holdings();
    const auto title=std::find_if(titles.begin(),titles.end(),[&](const LandHolding& row){return row.id==holding_id;});
    const auto sites=world.establishments();
    const auto site=std::find_if(sites.begin(),sites.end(),[&](const Establishment& row){return row.id==site_id;});
    const auto rights=world.land_use_rights();
    const auto right=std::find_if(rights.begin(),rights.end(),[&](const LandUseRight& row){return row.holding_id==holding_id;});
    const int64_t expected_cash=estate_money-(actor%7==uint64_t(world.view().day%7)?1:0);
    check(!world.person(actor).alive&&case_it!=cases.end()&&loan_it!=loans.end()&&
          case_it->cash==0&&case_it->loans==std::vector<uint64_t>{loan_id}&&loan_it->paid==0&&
          world.person(beneficiary).cash==beneficiary_cash+expected_cash&&world.debt(beneficiary)==0,
          "shortfall leaves the loan unpaid while remaining estate cash reaches the heir without transferring debt");
    check(title!=titles.end()&&title->title_holder==actor&&site!=sites.end()&&site->owner_id==actor&&
          right!=rights.end()&&right->grantor==actor&&
          std::find(case_it->land_holdings.begin(),case_it->land_holdings.end(),holding_id)!=case_it->land_holdings.end()&&
          std::find(case_it->establishments.begin(),case_it->establishments.end(),site_id)!=case_it->establishments.end()&&
          std::find(case_it->land_use_rights.begin(),case_it->land_use_rights.end(),right->id)!=case_it->land_use_rights.end(),
          "unpaid claim keeps title, use permission and workshop referenced by the estate");
    check(world.invariant()&&world.total_money()==total_money,
          "underfunded estate distribution preserves Realm accounting");
    const auto saved=world.serialize("underfunded-estate-claim");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,"underfunded-estate-claim",error),error.c_str());
    const auto restored_sites=restored.establishments();
    const auto restored_site=std::find_if(restored_sites.begin(),restored_sites.end(),
        [&](const Establishment& row){return row.id==site_id;});
    check(restored.fingerprint()==world.fingerprint()&&restored.estate_cases()==cases&&
          restored.loans().size()==loans.size()&&restored.loans().back().paid==0&&
          restored.land_holdings()==titles&&restored.land_use_rights()==rights&&
          restored_site!=restored_sites.end()&&restored_site->owner_id==actor&&restored_site->cash==site->cash&&
          restored.invariant(),
          "unpaid claim and retained estate property survive save/load exactly");
}

void test_estate_multiple_heirs_cash_goods_death_transition(){
    Realm world(59,2,128);
    const auto [decedent,first,second]=RealmObservationTestAccess::prepare_multiple_heir_death(world);
    const auto titles_before=world.land_holdings();
    const auto sites_before=world.establishments();
    const auto site_before=std::find_if(sites_before.begin(),sites_before.end(),[&](const Establishment& site){return site.owner_id==decedent;});
    const auto rights_before=world.land_use_rights();
    const auto money_before=world.total_money();
    check(site_before!=sites_before.end()&&world.invariant()&&world.debt(first)==0&&world.debt(second)==0,
          "multiple-heir fixture is valid and heirs hold no inherited debt");
    const auto site_id=site_before->id;
    const auto first_fiber=world.person(first).inventory[size_t(GoodId::Fiber)];
    const auto second_fiber=world.person(second).inventory[size_t(GoodId::Fiber)];
    const auto estate_cash=world.person(decedent).cash;
    RealmObservationTestAccess::run_people_phase(world);
    const auto heirs=std::array<uint64_t,2>{std::min(first,second),std::max(first,second)};
    const int64_t available_cash=estate_cash-(decedent%7==uint64_t(world.view().day%7)?1:0)-20;
    const int64_t cash_share=available_cash/2,cash_remainder=available_cash%2;
    check(world.person(heirs[0]).cash==cash_share+cash_remainder&&
          world.person(heirs[1]).cash==cash_share,
          "cash divides evenly between the two legal child heirs with remainder assigned by ID");
    check(world.person(first).inventory[size_t(GoodId::Fiber)]-first_fiber==2+(first<second?1:0)&&
          world.person(second).inventory[size_t(GoodId::Fiber)]-second_fiber==2+(second<first?1:0),
          "five estate Fiber units divide evenly and the remainder goes to the lower-ID heir");
    const auto estate=world.estate_cases();
    const auto case_it=std::find_if(estate.begin(),estate.end(),[&](const EstateCase& item){return item.decedent==decedent;});
    const auto sites_after=world.establishments();
    const auto site_after=std::find_if(sites_after.begin(),sites_after.end(),[&](const Establishment& site){return site.id==site_id;});
    check(case_it!=estate.end()&&case_it->cash==0&&case_it->inventory[size_t(GoodId::Fiber)]==0&&
          case_it->land_holdings.size()>=1&&case_it->establishments.size()>=1&&
          case_it->loans.size()==1&&world.loans().back().paid==20&&site_after!=sites_after.end()&&site_after->owner_id==decedent,
          "paid claim and divisible property leave the case while title/workshop remain recorded");
    check(world.land_holdings()==titles_before&&world.establishments().size()==sites_before.size()&&
          world.land_use_rights()==rights_before&&world.invariant()&&world.total_money()==money_before,
          "multiple heirs do not implicitly split titles or workshops, and accounting is conserved");
    const auto saved=world.serialize("multiple-heir-estate");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,"multiple-heir-estate",error),error.c_str());
    check(restored.fingerprint()==world.fingerprint()&&restored.estate_cases()==estate&&
          restored.land_holdings()==titles_before&&restored.land_use_rights()==rights_before&&restored.invariant(),
          "multiple-heir distribution and retained property survive save/load");
}

void test_minor_estate_land_guardian_continuation(){
    Realm world(73,2,128);
    const auto [decedent,minor,expected_guardian,holding_id]=
        RealmObservationTestAccess::prepare_minor_estate_land_death(world);
    const auto initial_money=world.total_money();
    check(world.guardian(minor)==expected_guardian&&world.invariant(),
          "minor inheritance fixture has a current adult guardian and valid land permission");
    RealmObservationTestAccess::run_people_phase(world);
    const auto titles=world.land_holdings();
    const auto title=std::find_if(titles.begin(),titles.end(),[&](const LandHolding& row){return row.id==holding_id;});
    const auto rights=world.land_use_rights();
    const auto right=std::find_if(rights.begin(),rights.end(),[&](const LandUseRight& row){return row.holding_id==holding_id;});
    check(!world.person(decedent).alive&&title!=titles.end()&&title->title_holder==minor&&
          right!=rights.end()&&right->grantor==minor,
          "death transfer gives the holding to the minor and preserves its user under the new holder");
    check(world.guardian(minor)==expected_guardian&&world.invariant()&&world.total_money()==initial_money,
          "inheritance keeps the existing guardian, world invariant and money total");
    const auto revoke=world.execute(LandUseRevokeCommand{expected_guardian,
        world.person(expected_guardian).decisions+1,holding_id});
    const auto guardianFacts=world.view(expected_guardian).facts;
    const auto revokeFact=std::find_if(guardianFacts.begin(),guardianFacts.end(),[&](const Fact& fact){return fact.id==revoke.fact;});
    const auto titlesAfter=world.land_holdings();
    check(revoke.success&&world.land_use_rights().empty()&&titlesAfter.size()==titles.size()&&
          std::find_if(titlesAfter.begin(),titlesAfter.end(),[&](const LandHolding& row){return row.id==holding_id;})->title_holder==minor&&
          revokeFact!=guardianFacts.end()&&revokeFact->actor==expected_guardian&&revokeFact->kind==214&&
          world.invariant()&&world.total_money()==initial_money,
          "current guardian revokes the inherited land permission without taking title or money");
    const auto saved=world.serialize("minor-estate-guardian-land");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,"minor-estate-guardian-land",error),error.c_str());
    check(restored.guardian(minor)==expected_guardian&&restored.land_holdings()==titlesAfter&&
          restored.land_use_rights().empty()&&restored.invariant()&&restored.fingerprint()==world.fingerprint(),
          "minor title, current guardian authority and completed revocation survive save/load");
}

void test_complex_actor_population_parameter(){
    auto legacy=std::make_unique<Realm>(37,6,10000);
    int legacyComplex=0;
    for(int id=1;id<=legacy->count();++id)
        legacyComplex+=legacy->person(id).family!=Family::Civilian;
    check(legacyComplex==128,"default large Realm retains 128 complex actors");

    auto profile=std::make_unique<Realm>(37,6,10000,false,GameplayConfig{},512);
    int governance=0,economic=0;
    for(int id=1;id<=profile->count();++id){
        governance+=profile->person(id).family==Family::Governance;
        economic+=profile->person(id).family==Family::Economic;
    }
    check(governance==256&&economic==256,"explicit 512 complex actors split evenly by family");
    const auto balances=profile->balances();
    int64_t balanceTotal=0;for(auto value:balances)balanceTotal+=value;
    check(profile->invariant()&&profile->total_money()==balanceTotal,
          "512 actor profile preserves Realm ledger invariant");
    const auto serialized=profile->serialize("complex-actor-profile");
    check(serialized.rfind("POLYEC_REALM_76 ",0)==0,"current save includes typed knowledge, world state and urban housing capacity");
    auto restored=std::make_unique<Realm>(1,1,128);std::string error;
    const bool profileLoaded=restored->deserialize(serialized,"complex-actor-profile",error);
    check(profileLoaded,error.c_str());
    int restoredComplex=0;
    for(int id=1;id<=restored->count();++id)
        restoredComplex+=restored->person(id).family!=Family::Civilian;
    check(restoredComplex==512&&restored->fingerprint()==profile->fingerprint()&&restored->invariant(),
          "512 actor family assignments survive save and load");

    {
        Realm aggregateEstate = Realm::scenario(37, 1, -1, -1, -1, -1, -1, -1, -1, 0);
        const auto saved = test_fixtures::save_as_v25(aggregateEstate.serialize("legacy-estate-migration"));
        std::vector<std::string> tokens; { std::istringstream in(saved); std::string token; while (in >> token) tokens.push_back(token); }
        const auto estateTag=std::find(tokens.begin(),tokens.end(),"ESTATE_1");
        check(estateTag!=tokens.end(), "schema 28 contains estate ledger section");
        tokens.resize(size_t(estateTag-tokens.begin()));
        tokens[0] = "POLYEC_REALM_23";
        std::ostringstream legacySave; for (const auto& token : tokens) legacySave << token << ' ';
        Realm migrated(1, 1, 128); std::string migrationError;
        const bool migrationLoaded=migrated.deserialize(legacySave.str(), "legacy-estate-migration", migrationError);
        check(migrationLoaded, migrationError.c_str());
    check(migrated.invariant() && migrated.serialize("legacy-estate-migration").rfind("POLYEC_REALM_76 ", 0) == 0,
              "schema 23 aggregate estate cash migrates to unassigned legacy balance");
    }

    bool rejected=false;
    try{Realm invalid(37,6,10000,false,GameplayConfig{},513);}catch(const std::invalid_argument&){rejected=true;}
    check(rejected,"odd complex actor count rejected");
    rejected=false;
    try{Realm invalid(37,6,10000,false,GameplayConfig{},2049);}catch(const std::invalid_argument&){rejected=true;}
    check(rejected,"complex actor count above profile bound rejected");
}
void test_canonical_market_transactions(){
    Realm world(71,2,128);uint64_t actor=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id)
        if(world.person(id).family==Family::Economic){actor=id;break;}
    check(actor!=0,"canonical market fixture has economic actor");
    const int region=world.person(actor).region;
    std::istringstream input(world.serialize("canonical-market-27"));
    std::vector<std::string> tokens;std::string token;
    while(input>>token)tokens.push_back(token);
    const size_t people=static_cast<size_t>(world.count());size_t person_start=tokens.size();
    for(size_t i=0;i+1+59*people<=tokens.size();++i){
        if(tokens[i]!=std::to_string(people))continue;
        bool dense=true;
        for(size_t row=0;row<people;++row)
            if(tokens[i+1+59*row]!=std::to_string(row+1)){dense=false;break;}
        if(dense){person_start=i+1;break;}
    }
    check(person_start<tokens.size(),"canonical market fixture finds Person section");
    const size_t province_count_at=person_start+59*people;
    check(province_count_at<tokens.size()&&tokens[province_count_at]==std::to_string(world.view().provinces.size()),
          "canonical market fixture finds Province section");
    const size_t province_start=province_count_at+1+66*static_cast<size_t>(region);
    const int64_t cash_before=std::stoll(tokens[person_start+59*static_cast<size_t>(actor-1)+16]);
    const int64_t added_cash=50000-cash_before;
    check(added_cash>=0,"canonical market fixture raises buyer cash");
    tokens[person_start+59*static_cast<size_t>(actor-1)+16]="50000";
    tokens[7]=std::to_string(std::stoll(tokens[7])+added_cash);
    for(int good=0;good<kGoodIdCount;++good){
        const size_t stock_index=province_start+12+static_cast<size_t>(good);
        tokens[stock_index]=std::to_string(std::stoll(tokens[stock_index])+1);
        const size_t opening_index=18+static_cast<size_t>(good);
        tokens[opening_index]=std::to_string(std::stoll(tokens[opening_index])+1);
    }
    std::ostringstream patched;for(const auto& value:tokens)patched<<value<<' ';
    std::string error;
    check(world.deserialize(patched.str(),"canonical-market-27",error),"canonical market fixture loads conserved stock");
    check(world.invariant(),"canonical market fixture satisfies whole-world invariant");
    const int64_t total_money=world.total_money();
    std::array<int64_t,kGoodIdCount> stock_before{};
    for(int good=0;good<kGoodIdCount;++good)
        stock_before[static_cast<size_t>(good)]=world.view().provinces[static_cast<size_t>(region)].stock[static_cast<size_t>(good)];
    for(int good=0;good<kGoodIdCount;++good){
        auto result=world.execute({actor,world.person(actor).decisions+1,{Intent::Buy,region,good,1}});
        check(result.success&&world.person(actor).inventory[static_cast<size_t>(good)]==1&&
                  world.view().provinces[static_cast<size_t>(region)].stock[static_cast<size_t>(good)]==stock_before[static_cast<size_t>(good)]-1&&
                  world.total_money()==total_money&&world.invariant(),
              "canonical GoodId Buy moves the exact product and conserves money");
        result=world.execute({actor,world.person(actor).decisions+1,{Intent::Sell,region,good,1}});
        check(result.success&&world.person(actor).inventory[static_cast<size_t>(good)]==0&&
                  world.view().provinces[static_cast<size_t>(region)].stock[static_cast<size_t>(good)]==stock_before[static_cast<size_t>(good)]&&
                  world.total_money()==total_money&&world.invariant(),
              "canonical GoodId Sell returns the exact product and conserves money");
    }
}
uint64_t work_site(Realm& world,int region,uint64_t worker=129){
    for(const auto&e:world.establishments())if(e.region==region){test_fixtures::authorize_work(world,e.id,worker);return e.id;}
    const uint64_t governor=region==world.regions()?1:uint64_t(region+2);
    auto result=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),region,EstablishmentIndustry::ClothWorkshop,1,1});
    if(!result.success)throw std::runtime_error("test work establishment: "+result.reason);
    test_fixtures::authorize_work(world,result.session,worker);
    return result.session;
}
void test_realm_recipe_rejects_same_group_substitutions(){
    struct Case {
        uint64_t seed;
        EstablishmentIndustry industry;
        GoodId substitute;
        GoodId companion;
        bool has_companion;
        GoodId output;
        const char* name;
    };
    constexpr std::array<Case,5> cases{{
        {401,EstablishmentIndustry::ClothWorkshop,GoodId::Beeswax,GoodId::DryFood,false,GoodId::Cloth,"Beeswax for Fiber"},
        {402,EstablishmentIndustry::ToolWorkshop,GoodId::Copper,GoodId::Fuel,true,GoodId::Tools,"Copper for Iron"},
        {403,EstablishmentIndustry::ToolWorkshop,GoodId::Silver,GoodId::Fuel,true,GoodId::Tools,"Silver for Iron"},
        {404,EstablishmentIndustry::ToolWorkshop,GoodId::Gold,GoodId::Fuel,true,GoodId::Tools,"Gold for Iron"},
        {405,EstablishmentIndustry::MilitarySupplyWorkshop,GoodId::Silk,GoodId::Tools,true,GoodId::MilitarySupply,"Silk for Cloth"}
    }};
    for(const auto& row:cases){
        Realm world(row.seed,2,256);
        const uint64_t worker=129;
        const int region=world.person(worker).region;
        const uint64_t governor=region==world.regions()?1:uint64_t(region+2);
        const auto created=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
            EstablishmentOwnerKind::Public,uint64_t(region),region,row.industry,1,1});
        check(created.success,"same-group substitution fixture creates the matching public workshop");
        const uint64_t site=created.session;
        test_fixtures::authorize_work(world,site,worker);
        const auto submitted=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
            ScheduleCommand::Operation::Submit,0,0,32,0,1000,ActivityKind::Work,region,site});
        check(submitted.success,"same-group substitution fixture accepts two Work units");

        std::vector<std::string> tokens;
        {std::istringstream input(world.serialize("realm-no-substitution"));std::string token;while(input>>token)tokens.push_back(token);}
        const auto tag=std::find(tokens.begin(),tokens.end(),"ESTABLISH_14");
        check(tag!=tokens.end(),"same-group substitution fixture locates establishment save section");
        const size_t record=size_t(tag-tokens.begin())+2;
        const auto seed_stock=[&](GoodId id){
            const size_t index=static_cast<size_t>(id);
            tokens[record+7+index]=std::to_string(std::stoll(tokens.at(record+7+index))+1);
            tokens[18+index]=std::to_string(std::stoll(tokens.at(18+index))+1);
        };
        seed_stock(row.substitute);
        if(row.has_companion)seed_stock(row.companion);
        std::ostringstream seeded_save;for(size_t i=0;i<tokens.size();++i){if(i)seeded_save<<' ';seeded_save<<tokens[i];}
        Realm seeded(1,1,128);std::string error;
        check(seeded.deserialize(seeded_save.str(),"realm-no-substitution",error),error.c_str());
        check(seeded.invariant(),"same-group substitution fixture keeps the opening goods ledger balanced");
        const int64_t money_before=seeded.total_money();
        seeded.advance_ticks(64);
        const auto sites=seeded.establishments();
        const auto produced_site=std::find_if(sites.begin(),sites.end(),[&](const Establishment& candidate){return candidate.id==site;});
        check(seeded.view().day==1&&produced_site!=sites.end(),"one full game day reaches the workshop production phase");
        const int64_t expectedUnits=(32*(150+seeded.person(worker).skill))/(200*16);
        check(produced_site->processed_work_units.at(worker)==expectedUnits&&produced_site->last_production_day==1,
              "the exact attention and personal skill determine work processed even without recipe input");
        check(produced_site->stock[static_cast<size_t>(row.substitute)]==1&&
              produced_site->stock[static_cast<size_t>(row.output)]==0&&
              (!row.has_companion||produced_site->stock[static_cast<size_t>(row.companion)]==1),
              row.name);
        check(seeded.total_money()==money_before&&seeded.invariant(),
              "same-group substitution attempt preserves money and whole-Realm good conservation");
        const auto saved=seeded.serialize("realm-no-substitution");
        Realm restored(1,1,128);
        check(restored.deserialize(saved,"realm-no-substitution",error),error.c_str());
        check(restored.fingerprint()==seeded.fingerprint()&&restored.invariant()&&
              restored.establishments().front().stock==produced_site->stock&&
              restored.establishments().front().processed_work_units==produced_site->processed_work_units,
              "same-group substitution rejection and forfeited Work survive save/load");
    }
}
std::string downgrade_save_to_v13(const std::string& data){
    std::istringstream input(test_fixtures::save_as_v15(data));std::vector<std::string> tokens;std::string token;while(input>>token)tokens.push_back(token);
    if(tokens.empty()||(tokens[0]!="POLYEC_REALM_15"&&tokens[0]!="POLYEC_REALM_14"))throw std::runtime_error("current fixture magic");
    const auto config=std::find(tokens.begin(),tokens.end(),"CONFIG_1");
    tokens.erase(config,tokens.end());
    tokens[0]="POLYEC_REALM_13";
    const size_t productionSection=std::find(tokens.begin(),tokens.end(),"ESTABLISH_3")-tokens.begin();
    if(productionSection>=tokens.size())throw std::runtime_error("v14 establishment section missing");
    tokens[productionSection]="ESTABLISH_2";
    const size_t productionCount=size_t(std::stoull(tokens.at(productionSection+1)));
    size_t productionRecord=productionSection+2;
    for(size_t row=0;row<productionCount;++row){
        const size_t watermarkCount=size_t(std::stoull(tokens.at(productionRecord+35)));
        const size_t recordEnd=productionRecord+36+2*watermarkCount;
        tokens.erase(tokens.begin()+std::ptrdiff_t(productionRecord+34),tokens.begin()+std::ptrdiff_t(recordEnd));
        productionRecord+=34;
    }
    std::ostringstream output;for(size_t i=0;i<tokens.size();++i){if(i)output<<' ';output<<tokens[i];}return output.str();
}
std::string downgrade_empty_goods_save_to_v12(const std::string& data,bool omitEstablishments=false){
    std::istringstream input(downgrade_save_to_v13(data));std::vector<std::string> tokens;std::string token;while(input>>token)tokens.push_back(token);
    const std::array<size_t,7> legacy{{0,9,11,5,4,16,14}};
    auto shrink=[&](size_t start){std::vector<std::string> values;for(size_t index:legacy)values.push_back(tokens.at(start+index));tokens.erase(tokens.begin()+std::ptrdiff_t(start),tokens.begin()+std::ptrdiff_t(start+27));tokens.insert(tokens.begin()+std::ptrdiff_t(start),values.begin(),values.end());};
    tokens[0]="POLYEC_REALM_12";size_t at=18;for(int i=0;i<3;++i){shrink(at);at+=7;}
    size_t peopleAt=0;for(size_t candidate=100;candidate<tokens.size();++candidate){size_t count=0;try{count=size_t(std::stoull(tokens[candidate]));}catch(...){continue;}if(count<128||count>1000000||candidate+1+56*count>tokens.size())continue;bool dense=true;for(size_t row=0;row<count;++row)if(tokens[candidate+1+59*row]!=std::to_string(row+1)){dense=false;break;}if(dense){peopleAt=candidate;break;}}
    if(!peopleAt)throw std::runtime_error("v13 fixture people block");
    const size_t people=size_t(std::stoull(tokens[peopleAt++]));
    for(size_t row=0;row<people;++row){const size_t start=peopleAt+row*36;std::vector<std::string> goods;for(size_t index:legacy)goods.push_back(tokens.at(start+29+index));tokens.erase(tokens.begin()+std::ptrdiff_t(start+29),tokens.begin()+std::ptrdiff_t(start+56));tokens.insert(tokens.begin()+std::ptrdiff_t(start+29),goods.begin(),goods.end());}
    size_t provinceCountAt=peopleAt+people*36;const size_t provinces=size_t(std::stoull(tokens.at(provinceCountAt++)));
    for(size_t row=0;row<provinces;++row){const size_t start=provinceCountAt+row*26;std::vector<std::string> fields;for(size_t i=0;i<12;++i)fields.push_back(tokens.at(start+i));for(size_t index:legacy)fields.push_back(tokens.at(start+12+index));for(size_t index:legacy)fields.push_back(tokens.at(start+39+index));tokens.erase(tokens.begin()+std::ptrdiff_t(start),tokens.begin()+std::ptrdiff_t(start+66));tokens.insert(tokens.begin()+std::ptrdiff_t(start),fields.begin(),fields.end());}
    const size_t establishment=std::find(tokens.begin(),tokens.end(),"ESTABLISH_2")-tokens.begin();if(establishment>=tokens.size())throw std::runtime_error("v13 establishment section missing");
    const size_t establishmentCount=size_t(std::stoull(tokens.at(establishment+1)));
    if(omitEstablishments){if(establishmentCount!=0)throw std::runtime_error("expected empty establishment section");tokens.erase(tokens.begin()+std::ptrdiff_t(establishment),tokens.end());}
    else{tokens[establishment]="ESTABLISH_1";const std::array<int,16> groupGood{{0,1,2,3,4,5,-1,9,11,14,16,-1,-1,-1,-1,-1}};const size_t firstRecord=establishment+2;for(size_t row=0;row<establishmentCount;++row){const size_t start=firstRecord+row*23;std::vector<std::string> stocks;for(int good:groupGood)stocks.push_back(good<0?"0":tokens.at(start+7+size_t(good)));tokens.erase(tokens.begin()+std::ptrdiff_t(start+7),tokens.begin()+std::ptrdiff_t(start+34));tokens.insert(tokens.begin()+std::ptrdiff_t(start+7),stocks.begin(),stocks.end());}}
    std::ostringstream output;for(size_t i=0;i<tokens.size();++i){if(i)output<<' ';output<<tokens[i];}return output.str();
}
// Historical regional-payroll fixture, not a runtime validator bypass. Generic
// Submit supplies the actual sequence/fact bookkeeping (identical event200),
// then only the saved session kind/region is encoded as pre-site Work in v12.
bool legacy_work_fixture(Realm& world,uint64_t actor,int region,int64_t begin,int64_t end){
    if(!world.establishments().empty())throw std::runtime_error("legacy payroll fixture requires no sites");
    const auto& person=world.person(actor);
    if(person.region!=region||person.destination!=-1||!person.alive)throw std::runtime_error("legacy fixture worker not present");
    const auto accepted=world.execute(ScheduleCommand{actor,person.decisions+1,ScheduleCommand::Operation::Submit,
        0,begin,end,0,1000});
    if(!accepted.success)throw std::runtime_error(accepted.reason);
    const auto source=test_fixtures::save_as_v28(world.serialize("legacy-payroll-fixture"));
    std::istringstream input(source);std::vector<std::string> tokens;std::string token;
    while(input>>token)tokens.push_back(token);
    const size_t schedule=std::find(tokens.begin(),tokens.end(),"SCHEDULE_5")-tokens.begin();
    if(schedule>=tokens.size())throw std::runtime_error("legacy payroll fixture missing SCHEDULE_5");
    const size_t sessionCount=std::stoull(tokens.at(schedule+5));
    size_t session=schedule+11;bool found=false;
    for(size_t row=0;row<sessionCount;++row,session+=17){
        if(std::stoull(tokens.at(session))==accepted.session&&std::stoull(tokens.at(session+1))==actor){
            tokens[session+14]="1";tokens[session+15]=std::to_string(region);tokens[session+16]="0";found=true;break;
        }
    }
    if(!found)throw std::runtime_error("legacy payroll fixture missing submitted session");
    std::ostringstream historicalSource;for(const auto& value:tokens)historicalSource<<value<<' ';
    const auto historical=downgrade_empty_goods_save_to_v12(historicalSource.str());
    check(historical.rfind("POLYEC_REALM_12",0)==0,"regional payroll fixture has actual historical grammar");
    Realm loaded(1,1,128);std::string error;
    if(!loaded.deserialize(historical,"legacy-payroll-fixture",error))throw std::runtime_error(error);
    world=std::move(loaded);return true;
}
int main(){try{
{
    constexpr uint64_t governor=2;
    auto offer_action=[&](Realm& world,int term){
        const auto candidates=world.candidates(governor);
        for(int slot=realm_military_service_candidate_begin;slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot)
            if(candidates[size_t(slot)].kind==Intent::Offer){auto action=candidates[size_t(slot)];action.term=term;return action;}
        return Candidate{};
    };
    Realm rejected(9201,6,128);
    const auto offer=offer_action(rejected,30);
    check(realm_actions==400&&realm_obs==5182&&offer.kind==Intent::Offer&&offer.good==3&&
          offer.amount==kMilitaryServiceTrialWagePerDay&&offer.term==30&&rejected.mask(governor)[realm_military_service_candidate_begin]==1.f,
          "new Governance choices point to a same-region eligible adult civilian and expose a 30-day, four-cash daily wage offer");
    check(Realm::action_name(offer)=="Đề nghị phục vụ",
          "the player-facing action label explains that this is a voluntary service proposal");
    const auto initial_money=rejected.total_money();
    const auto initial_treasury=rejected.view().provinces[size_t(rejected.person(governor).region)].treasury;
    const auto sent_offer=rejected.execute({governor,1,offer});
    if(!sent_offer.success)throw std::runtime_error("volunteer offer failed validation: "+sent_offer.reason);
    check(sent_offer.success,"eligible volunteer service proposal commits through the command pipeline");
    check(rejected.total_money()==initial_money,"offer escrow is included in conserved total money");
    check(rejected.view().provinces[size_t(rejected.person(governor).region)].treasury==initial_treasury-120,
          "sending the offer moves exactly 120 cash out of its regional treasury");
    check(rejected.invariant(),"world accounting accepts an unresolved, fully funded service offer");
    const auto duplicate_fingerprint=rejected.fingerprint();
    auto duplicate=offer;duplicate.target=offer.target;
    check(!rejected.execute({governor,2,duplicate}).success&&rejected.fingerprint()==duplicate_fingerprint,
          "one volunteer cannot receive two unresolved service offers from the same or another governor");
    Realm wrong_region(9201,6,128);
    auto cross_region_offer=offer_action(wrong_region,30);
    uint64_t outsider=0;
    for(uint64_t id=1;id<=uint64_t(wrong_region.count());++id){const auto& candidate=wrong_region.person(id);
        if(candidate.family==Family::Civilian&&candidate.alive&&candidate.age_days>=18*365&&candidate.region!=wrong_region.person(governor).region){outsider=id;break;}}
    check(outsider!=0,"cross-region rejection fixture has an adult civilian outside the governor's region");
    cross_region_offer.target=int(outsider);
    const auto wrong_region_before=wrong_region.fingerprint();
    check(!wrong_region.execute({governor,1,cross_region_offer}).success&&wrong_region.fingerprint()==wrong_region_before,
          "Governance cannot reserve treasury money for a volunteer who lives in another region");
    Realm underfunded_service(9201,6,128);
    auto unfunded_offer=offer_action(underfunded_service,30);
    const auto local_region=underfunded_service.person(governor).region;
    RealmObservationTestAccess::set_province_treasury(underfunded_service,local_region,119);
    const auto underfunded_before=underfunded_service.fingerprint();
    check(underfunded_service.mask(governor)[realm_military_service_candidate_begin]==0.f&&
          !underfunded_service.execute({governor,1,unfunded_offer}).success&&
          underfunded_service.fingerprint()==underfunded_before&&underfunded_service.invariant(),
          "a treasury short by one coin cannot create an offer or change any account");
    Realm expired_offer(9201,6,128),no_offer(9201,6,128);
    const auto expiring=offer_action(expired_offer,30);
    check(expired_offer.execute({governor,1,expiring}).success,"service offer enters escrow before its response window");
    expired_offer.advance(14);no_offer.advance(14);
    check(expired_offer.offers(governor).empty()&&
          expired_offer.view().provinces[size_t(expired_offer.person(governor).region)].treasury==
              no_offer.view().provinces[size_t(no_offer.person(governor).region)].treasury&&
          expired_offer.total_money()==no_offer.total_money()&&expired_offer.invariant(),
          "an unanswered offer expires after its seven-day response window and returns all reserved cash");
    const auto recipient=uint64_t(offer.target);
    rejected.advance(10);
    const auto treasury_before_reject=rejected.view().provinces[size_t(rejected.person(governor).region)].treasury;
    const auto money_before_reject=rejected.total_money();
    const auto reject_action=rejected.candidates(recipient)[29];
    check(reject_action.kind==Intent::Reject&&rejected.execute({recipient,rejected.person(recipient).decisions+1,reject_action}).success&&
          rejected.view().provinces[size_t(rejected.person(governor).region)].treasury==treasury_before_reject+120&&
          rejected.total_money()==money_before_reject&&rejected.invariant(),
          "the recipient can reject after preparation and the complete escrow returns to the same regional treasury");

    std::string save_error;
    Realm serving(9201,6,128);
    const auto service_offer=offer_action(serving,30);
    const auto service_target=uint64_t(service_offer.target);
    const int service_region=serving.person(governor).region;
    const auto treasury_before=serving.view().provinces[size_t(service_region)].treasury;
    const auto money_before=serving.total_money();
    check(serving.execute({governor,1,service_offer}).success,"eligible volunteer service proposal is created");
    serving.advance(10);
    const auto accept_action=serving.candidates(service_target)[28];
    check(accept_action.kind==Intent::Accept&&serving.execute({service_target,serving.person(service_target).decisions+1,accept_action}).success&&
          serving.person(service_target).military_service.active&&serving.person(service_target).military_service.commander==governor&&
          serving.person(service_target).military_service.escrow==120&&serving.total_money()==money_before&&serving.invariant(),
          "acceptance creates one named service record and transfers the reserved 120 cash without changing total money");
    const auto serving_fingerprint=serving.fingerprint();
    const auto first_roster=serving.military_unit_rosters();
    check(first_roster.size()==1&&first_roster[0].source_region==service_region&&
          first_roster[0].commander==governor&&first_roster[0].members==std::vector<uint64_t>{service_target}&&
          first_roster[0].id==(uint64_t(service_region+1)<<32|governor)&&
          first_roster[0].payroll_escrow==120&&first_roster[0].paid_total==0&&
          serving.fingerprint()==serving_fingerprint&&serving.total_money()==money_before,
          "read-only roster groups the accepted volunteer under a stable source-region and commander ID without changing the world");
    serving.advance(1);
    check(serving.person(service_target).military_service.paid_total==4&&
          serving.person(service_target).military_service.escrow==116&&serving.total_money()==money_before&&serving.invariant(),
          "the first service day pays exactly four cash and leaves 116 reserved for later days");
    const auto paid_roster=serving.military_unit_rosters();
    check(paid_roster.size()==1&&paid_roster[0].payroll_escrow==116&&paid_roster[0].paid_total==4,
          "roster payroll summary follows the named service record after a daily wage is paid");
    const auto muster_candidate=Candidate{Intent::Mobilize,service_region,0,40,5};
    const auto muster_feature_index=size_t(realm_state+16*realm_features+10);
    const auto muster_treasury_before=serving.view().provinces[size_t(service_region)].treasury;
    const auto muster_firm_before=serving.view().provinces[size_t(service_region)].firm;
    const auto muster_unrest_before=serving.view().provinces[size_t(service_region)].unrest;
    const auto muster_money_before=serving.total_money();
    const int expected_muster_until=std::min(serving.view().day+1+muster_candidate.term,
        serving.person(service_target).military_service.end_day);
    const auto muster=serving.execute({governor,serving.person(governor).decisions+1,muster_candidate});
    const auto muster_rosters=serving.military_unit_rosters();
    check(muster.success&&serving.view().provinces[size_t(service_region)].treasury==muster_treasury_before-40&&
          serving.view().provinces[size_t(service_region)].firm==muster_firm_before+40&&
          serving.view().provinces[size_t(service_region)].unrest==std::max(0,muster_unrest_before-
              serving.capability_value(governor,LegacyCapability::Military)/10)&&serving.total_money()==muster_money_before&&
          serving.person(service_target).military_service.mobilized_until_day==expected_muster_until&&
          muster_rosters.size()==1&&muster_rosters[0].mobilized_members==std::vector<uint64_t>{service_target}&&
          muster_rosters[0].mobilized_until_day==expected_muster_until&&
          std::abs(serving.observation(governor)[muster_feature_index]-0.001f)<1e-7f&&serving.invariant(),
          "Mobilize preserves its 40-cash and unrest effects while marking only the commander's accepted volunteer until the shorter of the requested window or service agreement");
    const auto mobilization_facts=serving.view().facts;
    check(mobilization_facts.size()>=2&&mobilization_facts.back().kind==227&&mobilization_facts.back().amount==5&&
          mobilization_facts.back().target==1&&mobilization_facts.back().parent+1==muster.fact,
          "a local volunteer muster fact records its requested five-day window, member count and parent Mobilize fact");
    auto repeated_short_muster=serving;
    auto shorter_muster=muster_candidate;shorter_muster.term=1;
    check(repeated_short_muster.execute({governor,repeated_short_muster.person(governor).decisions+1,shorter_muster}).success&&
          repeated_short_muster.person(service_target).military_service.mobilized_until_day==expected_muster_until&&
          repeated_short_muster.invariant(),
          "repeating a shorter Mobilize command does not shorten an already open volunteer muster window");
    auto repeated_long_muster=serving;
    auto longer_muster=muster_candidate;longer_muster.term=30;
    const int capped_muster_until=std::min(serving.view().day+1+longer_muster.term,
        serving.person(service_target).military_service.end_day);
    check(repeated_long_muster.execute({governor,repeated_long_muster.person(governor).decisions+1,longer_muster}).success&&
          repeated_long_muster.person(service_target).military_service.mobilized_until_day==capped_muster_until&&
          capped_muster_until<=repeated_long_muster.person(service_target).military_service.end_day&&
          repeated_long_muster.invariant(),
          "repeating a longer Mobilize command extends readiness only up to the volunteer agreement end day");
    auto muster_window=serving;
    muster_window.advance(expected_muster_until-muster_window.view().day-1);
    check(muster_window.military_unit_rosters()[0].mobilized_members==std::vector<uint64_t>{service_target},
          "a five-day muster window remains active through the last included game day");
    muster_window.advance(1);
    const auto ended_muster=muster_window.military_unit_rosters();
    const auto muster_expiry_facts=muster_window.view(governor).facts;
    check(ended_muster.size()==1&&ended_muster[0].mobilized_members.empty()&&ended_muster[0].mobilized_until_day==0&&
          muster_window.person(service_target).military_service.active&&
          muster_window.person(service_target).military_service.end_day==serving.person(service_target).military_service.end_day&&
          std::any_of(muster_expiry_facts.begin(),muster_expiry_facts.end(),[](const Fact& fact){return fact.kind==228;}),
          "muster expires after five days, leaves the volunteer service agreement unchanged and records the end of the window");
    auto death_world=serving;
    while((death_world.view().day+1)%7!=int(service_target%7))death_world.advance(1);
    RealmObservationTestAccess::remove_all_food_and_set_death_health(death_world,service_target);
    const auto money_before_death=death_world.total_money();
    const auto unpaid_before_death=death_world.person(service_target).military_service.escrow;
    const auto treasury_before_death=death_world.view().provinces[size_t(service_region)].treasury;
    death_world.advance(1);
    check(!death_world.person(service_target).alive&&!death_world.person(service_target).military_service.active&&
          death_world.view().provinces[size_t(service_region)].treasury==treasury_before_death+unpaid_before_death-4&&
          death_world.total_money()==money_before_death&&death_world.invariant()&&death_world.military_unit_rosters().empty(),
          "death ends service, pays the due day once, returns every remaining escrow coin and preserves total money");
    Realm roster_world(9201,6,128);
    const auto first_offer=offer_action(roster_world,30);const uint64_t first_member=uint64_t(first_offer.target);
    check(roster_world.execute({governor,1,first_offer}).success,"roster fixture creates first volunteer invitation");
    roster_world.advance(10);
    check(roster_world.execute({first_member,roster_world.person(first_member).decisions+1,roster_world.candidates(first_member)[28]}).success,
          "first roster fixture member voluntarily accepts service");
    const auto second_offer=offer_action(roster_world,30);const uint64_t second_member=uint64_t(second_offer.target);
    check(second_member!=first_member&&roster_world.person(second_member).region==roster_world.person(governor).region&&
          roster_world.execute({governor,roster_world.person(governor).decisions+1,second_offer}).success,
          "the same regional commander can invite a second eligible volunteer");
    roster_world.advance(10);
    check(roster_world.execute({second_member,roster_world.person(second_member).decisions+1,roster_world.candidates(second_member)[28]}).success,
          "second roster fixture member voluntarily accepts service");
    const auto grouped_rosters=roster_world.military_unit_rosters();
    const auto expected_roster_escrow=roster_world.person(first_member).military_service.escrow+
        roster_world.person(second_member).military_service.escrow;
    const auto expected_roster_paid=roster_world.person(first_member).military_service.paid_total+
        roster_world.person(second_member).military_service.paid_total;
    const std::vector<uint64_t> expected_roster_members{
        std::min(first_member,second_member),std::max(first_member,second_member)};
    check(grouped_rosters.size()==1&&grouped_rosters[0].members==expected_roster_members&&
          grouped_rosters[0].payroll_escrow==expected_roster_escrow&&grouped_rosters[0].paid_total==expected_roster_paid,
          "two accepted volunteers under the same local commander share one sorted roster with summed remaining and paid wages");
    const auto consent_result=roster_world.execute(MilitaryCombatConsentCommand{
        first_member,roster_world.person(first_member).decisions+1,true});
    const auto mobilize_result=roster_world.execute({governor,roster_world.person(governor).decisions+1,
        {Intent::Mobilize,roster_world.person(governor).region,0,40,5}});
    const auto ready_rosters=roster_world.military_unit_rosters();
    check(consent_result.success&&mobilize_result.success&&ready_rosters.size()==1&&
          ready_rosters[0].combat_consented_members==std::vector<uint64_t>{first_member}&&
          ready_rosters[0].battle_ready_members==std::vector<uint64_t>{first_member},
          "a volunteer appears battle-ready only after personal combat consent, active service, local presence and a current muster window");
    const auto consent_withdrawn=roster_world.execute(MilitaryCombatConsentCommand{
        first_member,roster_world.person(first_member).decisions+1,false});
    check(consent_withdrawn.success&&roster_world.military_unit_rosters()[0].combat_consented_members.empty()&&
          roster_world.military_unit_rosters()[0].battle_ready_members.empty()&&roster_world.invariant(),
          "withdrawing personal combat consent immediately removes the volunteer from every combat-eligible roster view");
    check(roster_world.execute(MilitaryCombatConsentCommand{
              first_member,roster_world.person(first_member).decisions+1,true}).success,
          "the volunteer may separately consent again before a battle begins");
    const auto grouped_save=roster_world.serialize("military-unit-roster-roundtrip");
    Realm grouped_restored(1,1,128);
    const bool grouped_loaded=grouped_restored.deserialize(grouped_save,"military-unit-roster-roundtrip",save_error);
    if(!grouped_loaded)std::cerr<<"military roster roundtrip diagnostic: "<<save_error<<"\n";
    check(grouped_loaded&&
          grouped_restored.military_unit_rosters().size()==1&&
          grouped_restored.military_unit_rosters()[0].id==grouped_rosters[0].id&&
          grouped_restored.military_unit_rosters()[0].members==grouped_rosters[0].members&&
          grouped_restored.military_unit_rosters()[0].combat_consented_members==std::vector<uint64_t>{first_member}&&
          grouped_restored.military_unit_rosters()[0].battle_ready_members==std::vector<uint64_t>{first_member}&&
          grouped_restored.military_unit_rosters()[0].payroll_escrow==grouped_rosters[0].payroll_escrow&&
          grouped_restored.military_unit_rosters()[0].paid_total==grouped_rosters[0].paid_total,
          "save/load reconstructs the same derived unit identity, members and payroll totals without a second roster save section");
    auto one_left=roster_world;
    check(one_left.execute({first_member,one_left.person(first_member).decisions+1,one_left.candidates(first_member)[31]}).success&&
          one_left.military_unit_rosters().size()==1&&one_left.military_unit_rosters()[0].members.size()==1&&
          one_left.military_unit_rosters()[0].members[0]==second_member&&one_left.invariant(),
          "voluntary resignation removes only that member and keeps the commander roster while another volunteer remains");
    Realm depot_world(9201,6,128);
    uint64_t depot_governor=0;int depot_region=-1;
    for(uint64_t id=1;id<=uint64_t(depot_world.count());++id){
        const auto& candidate=depot_world.person(id);
        if(candidate.alive&&candidate.family==Family::Governance&&candidate.taxing_authority&&
           depot_world.view().provinces[size_t(candidate.region)].charter&&
           depot_world.view().provinces[size_t(candidate.region)].stock[size_t(GoodId::DryFood)]>=4){
            depot_governor=id;depot_region=candidate.region;break;
        }
    }
    check(depot_governor!=0,"regional ration purchase fixture has local authorized Governance, charter and four DryFood in public stock");
    const auto ration_candidates=depot_world.candidates(depot_governor);
    const auto ration_mask=depot_world.mask(depot_governor,ration_candidates);
    const auto ration_action=ration_candidates[size_t(realm_military_ration_purchase_candidate_begin+1)];
    const auto ration_observation=depot_world.observation(depot_governor);
    check(realm_actions==400&&realm_state==382&&realm_obs==5182&&realm_schema==std::string("realm-v45")&&
          ration_action.kind==Intent::BuyMilitaryRations&&ration_action.target==depot_region&&
          ration_action.good==int(GoodId::DryFood)&&ration_action.amount==4&&ration_mask[size_t(realm_military_ration_purchase_candidate_begin+1)]==1.f&&
          std::abs(ration_observation[size_t(realm_ration_depot_observation_begin+depot_region)])<1e-7f&&
          Realm::action_name(ration_action)=="Mua lương quân vụ",
          "the expanded model exposes four legal local ration quantities, 27 regional depot observations and the Vietnamese action label");
    const auto depot_before=depot_world.view().provinces[size_t(depot_region)];
    const auto depot_money=depot_world.total_money();
    const auto depot_cost=MarketManager::purchase_total(depot_before.price[size_t(GoodId::DryFood)],4);
    const auto depot_tax=depot_cost?MarketManager::assess_sales_tax(*depot_cost,depot_before.tax,depot_before.sales_tax_remainder):std::nullopt;
    check(depot_cost&&depot_tax,"regional ration purchase fixture has representable price and tax");
    const auto purchase=depot_world.execute({depot_governor,
        depot_world.person(depot_governor).decisions+1,ration_action});
    const auto depot_after=depot_world.view().provinces[size_t(depot_region)];
    check(purchase.success&&depot_world.military_ration_stock(depot_region)==4&&
          depot_after.stock[size_t(GoodId::DryFood)]==depot_before.stock[size_t(GoodId::DryFood)]-4&&
          depot_after.treasury==depot_before.treasury-*depot_cost+depot_tax->amount&&
          depot_after.firm==depot_before.firm+*depot_cost-depot_tax->amount&&
          depot_after.tax_receipts==depot_before.tax_receipts+depot_tax->amount&&
          std::abs(depot_world.observation(depot_governor)[size_t(realm_ration_depot_observation_begin+depot_region)]-0.004f)<1e-7f&&
          depot_world.total_money()==depot_money&&depot_world.invariant(),
          "Governance buys four DryFood for the public depot using treasury, the current market price and the normal sales tax while preserving money and goods");
    const auto purchased_fingerprint=depot_world.fingerprint();
    check(!depot_world.execute(MilitaryRationPurchaseCommand{depot_governor,
              depot_world.person(depot_governor).decisions+1,depot_region,0}).success&&
          !depot_world.execute(MilitaryRationPurchaseCommand{depot_governor,
              depot_world.person(depot_governor).decisions+1,depot_region,kMilitaryRationPurchaseMaximum+1}).success&&
          depot_world.fingerprint()==purchased_fingerprint,
          "zero and over-limit ration purchases reject without changing accounts or decision counters");
    const auto saved_depot=depot_world.serialize("military-ration-depot-roundtrip");
    Realm restored_depot(1,1,128);
    check(saved_depot.rfind("POLYEC_REALM_76 ",0)==0,"current ration save includes the military-control section written by the core");
    if(!restored_depot.deserialize(saved_depot,"military-ration-depot-roundtrip",save_error))
        throw std::runtime_error("schema46 ration save rejected: "+save_error);
    check(restored_depot.military_ration_stock(depot_region)==4,"schema47 load restores the purchased depot stock");
    check(restored_depot.fingerprint()==depot_world.fingerprint()&&restored_depot.invariant(),
          "schema47 save/load preserves the regional ration stock with the same world fingerprint");
    auto legacy46=saved_depot;
    test_fixtures::downgrade_save_48_to_47(legacy46);
    const auto shipment_v2=legacy46.find(" TRANSPORT_SHIPMENT_2 ");
    check(shipment_v2!=std::string::npos,"schema47 save contains transport section version2");
    legacy46.replace(shipment_v2+1,std::string("TRANSPORT_SHIPMENT_2").size(),"TRANSPORT_SHIPMENT_1");
    legacy46.replace(0,std::string("POLYEC_REALM_47").size(),"POLYEC_REALM_46");
    check(restored_depot.deserialize(legacy46,"military-ration-depot-roundtrip",save_error)&&
          restored_depot.military_ration_stock(depot_region)==4&&restored_depot.invariant(),
          "schema46 migration retains depot stock and reads its earlier personal-shipment section");
    Realm schema45_source(9201,6,128);
    auto legacy45=schema45_source.serialize("military-ration-depot-roundtrip");
    test_fixtures::downgrade_save_48_to_47(legacy45);
    const auto depot_section=legacy45.find(" MILITARY_RATION_DEPOT_1 ");
    check(depot_section!=std::string::npos,"schema47 save contains the explicit regional ration section");
    legacy45.erase(depot_section);
    test_fixtures::downgrade_empty_transport_shipment_2_to_1(legacy45);
    legacy45.replace(0,std::string("POLYEC_REALM_47").size(),"POLYEC_REALM_45");
    check(restored_depot.deserialize(legacy45,"military-ration-depot-roundtrip",save_error)&&
          restored_depot.military_ration_stock(depot_region)==0&&restored_depot.invariant(),
          "schema45 migration initializes every regional depot empty without creating food");
    const auto saved_service=serving.serialize("military-service-roundtrip");
    Realm restored(1,1,128);
    check(saved_service.rfind("POLYEC_REALM_76 ",0)==0&&saved_service.find(" MILITARY_SERVICE_3 ")!=std::string::npos&&restored.deserialize(saved_service,"military-service-roundtrip",save_error)&&
          restored.person(service_target).military_service.active&&restored.person(service_target).military_service.paid_total==4&&
          restored.person(service_target).military_service.mobilized_until_day==expected_muster_until&&
          restored.military_unit_rosters()[0].mobilized_members==std::vector<uint64_t>{service_target}&&
          restored.fingerprint()==serving.fingerprint()&&restored.invariant(),
          "schema60 save/load preserves an active volunteer, paid wages, escrow and the current muster deadline exactly");
    auto migration_source=serving;
    migration_source.advance(expected_muster_until-migration_source.view().day);
    auto legacy47=migration_source.serialize("military-service-roundtrip");
    test_fixtures::downgrade_save_48_to_47(legacy47);
    Realm migrated47(1,1,128);
    check(migrated47.deserialize(legacy47,"military-service-roundtrip",save_error)&&
          migrated47.person(service_target).military_service.active&&
          migrated47.person(service_target).military_service.mobilized_until_day==0&&migrated47.invariant(),
          "schema47 migration keeps the volunteer record and starts without a historical muster marker");
    auto malformed_tokens=std::vector<std::string>{};
    {std::istringstream input(saved_service);std::string token;while(input>>token)malformed_tokens.push_back(token);}
    const auto service_tag=std::find(malformed_tokens.begin(),malformed_tokens.end(),"MILITARY_SERVICE_3");
    const size_t service_row=size_t(service_tag-malformed_tokens.begin())+2+size_t(service_target-1)*10;
    malformed_tokens[service_row+8]="115";
    std::ostringstream malformed_save;for(const auto& token:malformed_tokens)malformed_save<<token<<' ';
    const auto before_malformed=restored.fingerprint();
    check(!restored.deserialize(malformed_save.str(),"military-service-roundtrip",save_error)&&
          restored.fingerprint()==before_malformed&&restored.invariant(),
          "a save whose service escrow exceeds the recorded unpaid wages is rejected without replacing the loaded world");
    const auto treasury_before_resign=serving.view().provinces[size_t(service_region)].treasury;
    auto resigned=serving;
    const auto resignation=resigned.candidates(service_target)[31];
    check(resignation.kind==Intent::Resign&&resigned.execute({service_target,resigned.person(service_target).decisions+1,resignation}).success&&
          !resigned.person(service_target).military_service.active&&
          resigned.view().provinces[size_t(service_region)].treasury==treasury_before_resign+116&&
          resigned.total_money()==money_before&&resigned.invariant(),
          "voluntary early resignation returns the unpaid 116 cash and keeps the four already paid with the person");
    const auto rejected_work=Candidate{Intent::Work,serving.person(service_target).region,0,0,30};
    check(serving.mask(service_target)[1+serving.person(service_target).region]==0.f&&
          !serving.execute({service_target,serving.person(service_target).decisions+1,rejected_work}).success&&serving.invariant(),
          "a serving volunteer cannot begin a new ordinary Work assignment");
    const auto before_scheduled_work=serving.fingerprint();
    ScheduleCommand scheduled_work{service_target,serving.person(service_target).decisions+1,
        ScheduleCommand::Operation::Submit,0,serving.game_tick(),serving.game_tick()+1,0,1000};
    scheduled_work.kind=ActivityKind::Work;scheduled_work.region=serving.person(service_target).region;scheduled_work.establishment_id=1;
    check(!serving.execute(scheduled_work).success&&serving.fingerprint()==before_scheduled_work,
          "the direct work-schedule command also rejects a new Work session during service");
    check(!serving.execute({service_target,serving.person(service_target).decisions+1,
          {Intent::WorkAtSite,1,0,0,1}}).success&&serving.fingerprint()==before_scheduled_work,
          "the establishment Work action is blocked by the same service rule");
    Realm legacy_base(9201,6,128);
    auto legacy44=legacy_base.serialize("military-service-roundtrip");
    test_fixtures::downgrade_save_48_to_47(legacy44);
    legacy44.erase(legacy44.find(" MILITARY_SERVICE_1 "));
    test_fixtures::downgrade_empty_transport_shipment_2_to_1(legacy44);
    legacy44.replace(0,std::string("POLYEC_REALM_47").size(),"POLYEC_REALM_44");
    Realm migrated(1,1,128);
    check(migrated.deserialize(legacy44,"military-service-roundtrip",save_error)&&
          !migrated.person(service_target).military_service.active&&migrated.person(service_target).military_service.escrow==0&&
          migrated.invariant(),"schema44 migration initializes every person without service or phantom military escrow");

    Realm one_day(9201,6,128);
    auto short_offer=offer_action(one_day,1);const auto short_target=uint64_t(short_offer.target);
    check(one_day.execute({governor,1,short_offer}).success,"one-day service term can be offered");
    one_day.advance(10);
    const auto short_accept=one_day.candidates(short_target)[28];
    check(one_day.execute({short_target,one_day.person(short_target).decisions+1,short_accept}).success,"one-day service term can be accepted");
    one_day.advance(1);
    const auto short_facts=one_day.view(short_target).facts;
    check(!one_day.person(short_target).military_service.active,
          "one-day service closes after its one paid day");
    check(one_day.invariant(),"one-day service end returns remaining escrow and preserves accounting");
    check(std::count_if(short_facts.begin(),short_facts.end(),[](const Fact& fact){return fact.kind==115;})>=1,
          "one-day service emits an end-of-service event");

    Realm service_cost(9201,6,128),work_control(9201,6,128);
    RealmObservationTestAccess::set_reference_scenario(service_cost,true);
    RealmObservationTestAccess::set_reference_scenario(work_control,true);
    const auto cost_offer=offer_action(service_cost,30);
    const auto control_offer=offer_action(work_control,30);
    const auto cost_worker=uint64_t(cost_offer.target);
    check(cost_offer.target==control_offer.target&&
          service_cost.execute({governor,1,cost_offer}).success&&
          work_control.execute({governor,1,control_offer}).success,
          "paired labor worlds invite the same civilian using the same service contract");
    service_cost.advance(10);work_control.advance(10);
    const int work_region=service_cost.person(cost_worker).region;
    const auto service_accept=service_cost.candidates(cost_worker)[28];
    const auto control_reject=work_control.candidates(cost_worker)[29];
    check(service_accept.kind==Intent::Accept&&control_reject.kind==Intent::Reject&&
          service_cost.execute({cost_worker,service_cost.person(cost_worker).decisions+1,service_accept}).success&&
          work_control.execute({cost_worker,work_control.person(cost_worker).decisions+1,control_reject}).success&&
          service_cost.total_money()==work_control.total_money()&&service_cost.invariant()&&work_control.invariant(),
          "service acceptance and control rejection return both worlds to the same total money before labor begins");
    const int work_good=canonical_from_legacy_good(work_region%7);
    const auto production_before=RealmObservationTestAccess::produced(service_cost)[size_t(work_good)];
    const auto control_production_before=RealmObservationTestAccess::produced(work_control)[size_t(work_good)];
    const auto control_work=work_control.candidates(cost_worker)[size_t(1+work_region)];
    const auto service_work=Candidate{Intent::Work,work_region,0,0,30};
    check(control_work.kind==Intent::Work&&work_control.mask(cost_worker)[size_t(1+work_region)]==1.f&&
          work_control.execute({cost_worker,work_control.person(cost_worker).decisions+1,control_work}).success,
          "the person who declined service starts the same legal 30-day Work in the same region");
    const auto service_before_reject=service_cost.fingerprint();
    check(!service_cost.execute({cost_worker,service_cost.person(cost_worker).decisions+1,service_work}).success&&
          service_cost.fingerprint()==service_before_reject,
          "the same Work command is rejected atomically while the matched person is serving");
    const int expected_daily_output=1+service_cost.person(cost_worker).skill/40;
    service_cost.advance(8);work_control.advance(8);
    const auto service_output=RealmObservationTestAccess::produced(service_cost)[size_t(work_good)]-production_before;
    const auto control_output=RealmObservationTestAccess::produced(work_control)[size_t(work_good)]-control_production_before;
    check(service_output==0&&control_output==int64_t(8*expected_daily_output)&&
          control_output>service_output&&work_control.person(cost_worker).work_assignment.job==work_region&&
          service_cost.person(cost_worker).military_service.paid_total==8*kMilitaryServiceTrialWagePerDay&&
          service_cost.invariant()&&work_control.invariant(),
          "eight matched days record zero Work output while serving and the exact existing daily output in the control");
    std::cout<<"VOLUNTEER_WORK_COST,seed,9201,region,"<<work_region<<",person,"<<cost_worker
             <<",skill,"<<service_cost.person(cost_worker).skill<<",days,8,good,"<<work_good
             <<",output_serving,"<<service_output<<",output_control,"<<control_output
             <<",daily_control,"<<expected_daily_output<<'\n';
}
{
    Realm world(806,6,128);
    constexpr uint64_t local_governor=2,other_governor=3,civilian=15;
    RealmObservationTestAccess::set_province_unrest(world,0,55);
    const auto candidate=world.candidates(local_governor)[16];
    check(candidate.kind==Intent::Mobilize&&candidate.target==0&&candidate.amount==40&&candidate.term==30,
          "local Governance candidate exposes the existing 40-cash, 30-day Mobilize parameters");
    check(world.mask(local_governor)[16]==1.f,
          "Governance can mobilize its own region when its treasury covers the cost");
    const auto before=world.view().provinces[0];
    const auto money_before=world.total_money();
    const int reduction=world.person(local_governor).capability(CapabilityId::Military)/10;
    const auto result=world.execute({local_governor,1,candidate});
    check(result.success,"valid local Mobilize command commits");
    const auto after=world.view().provinces[0];
    check(after.treasury==before.treasury-40&&after.firm==before.firm+40,
          "Mobilize transfers exactly 40 from the regional treasury to its firm");
    check(after.unrest==55-reduction&&world.total_money()==money_before&&world.invariant(),
          "Mobilize lowers unrest by military capability divided by ten and conserves money");
    Realm one_day_term(806,6,128),thirty_day_term(806,6,128);
    RealmObservationTestAccess::set_province_unrest(one_day_term,0,55);
    RealmObservationTestAccess::set_province_unrest(thirty_day_term,0,55);
    auto one_day_action=one_day_term.candidates(local_governor)[16];one_day_action.term=1;
    auto thirty_day_action=thirty_day_term.candidates(local_governor)[16];thirty_day_action.term=30;
    check(one_day_term.execute({local_governor,1,one_day_action}).success&&
          thirty_day_term.execute({local_governor,1,thirty_day_action}).success&&
          one_day_term.fingerprint()==thirty_day_term.fingerprint(),
          "current Mobilize behavior does not use its accepted 1–30 day term");
    const auto facts=world.view(local_governor).facts;
    check(std::any_of(facts.begin(),facts.end(),[&](const Fact& fact){
              return fact.kind==int(Intent::Mobilize)&&fact.actor==local_governor&&fact.region==0&&fact.amount==40;
          }),"successful Mobilize records its actor, region and paid amount");

    auto wrong_region=world.candidates(other_governor)[16];
    wrong_region.target=0;
    const auto before_wrong_region=world.fingerprint();
    check(world.mask(other_governor,world.candidates(other_governor))[16]==1.f&&
          !world.execute({other_governor,1,wrong_region}).success&&world.fingerprint()==before_wrong_region,
          "a governor cannot mobilize in a different region by changing the candidate target");
    const auto civilian_action=Candidate{Intent::Mobilize,world.person(civilian).region,0,40,30};
    const auto before_civilian=world.fingerprint();
    check(!world.execute({civilian,1,civilian_action}).success&&world.fingerprint()==before_civilian,
          "a civilian cannot issue the Governance-only Mobilize command");
    for(const int term:{0,31}){
        Realm invalid_term(806,6,128);
        auto action=invalid_term.candidates(local_governor)[16];action.term=term;
        const auto before_invalid_term=invalid_term.fingerprint();
        check(!invalid_term.execute({local_governor,1,action}).success&&
              invalid_term.fingerprint()==before_invalid_term&&invalid_term.invariant(),
              "Mobilize rejects a service duration outside 1–30 days without changing the world");
    }
    Realm underfunded(806,6,128);
    RealmObservationTestAccess::set_province_treasury(underfunded,0,39);
    const auto before_underfunded=underfunded.fingerprint();
    check(underfunded.mask(local_governor)[16]==0.f&&
          !underfunded.execute({local_governor,1,underfunded.candidates(local_governor)[16]}).success&&
          underfunded.view().provinces[0].treasury==39&&
          underfunded.fingerprint()==before_underfunded&&underfunded.invariant(),
          "insufficient regional treasury disables Mobilize and reject leaves balances intact");
}
{
    Stats stats;
    for(size_t i=0;i<kLegacyCapabilities.size();++i){
        const int value=int(i*13+7);
        stats.set(kLegacyCapabilities[i],value);
        check(stats.get(kLegacyCapabilities[i])==value,"named legacy capability reads its assigned value");
        check(stats.values[i]==value,"named legacy capability preserves the serialized slot order");
    }
    static_assert(static_cast<unsigned>(CapabilityId::Strategy)==0);
    static_assert(static_cast<unsigned>(CapabilityId::Military)==5);
    static_assert(static_cast<unsigned>(CapabilityId::LaborSkill)==6);
    std::array<std::string,kCapabilityCount> catalogKeys{};
    for(size_t i=0;i<kCapabilityCount;++i){
        const auto& definition=kCapabilityCatalog[i];
        check(static_cast<size_t>(definition.id)==i&&definition.key&&definition.unit&&definition.owner&&
              definition.minimum==0&&definition.maximum==100&&std::string(definition.owner)=="Person",
              "capability catalog has stable ID, owner, unit and ordered score range");
        check(definition.scenario_configurable==(i<kLegacyCapabilities.size())&&
              definition.always_active==(i==kLegacyCapabilities.size())&&
              definition.disabled_value==(i<kLegacyCapabilities.size()?kInactiveCapabilityNeutralValue:-1),
              "catalog metadata matches scenario mask and always-active labor rule");
        catalogKeys[i]=definition.key;
    }
    std::sort(catalogKeys.begin(),catalogKeys.end());
    check(std::adjacent_find(catalogKeys.begin(),catalogKeys.end())==catalogKeys.end(),
          "capability catalog keys are unique");
    Person labor;labor.skill=73;
    check(labor.capability(CapabilityId::LaborSkill)==73&&
          std::string(kCapabilityKeys[6])=="labor_skill"&&
          std::string(kCapabilityCatalog[6].unit)=="labor skill point"&&
          kCapabilityCatalog[6].minimum==0&&kCapabilityCatalog[6].maximum==100,
          "stable labor_skill ID maps to Person.skill with its documented unit and range");
    labor.skill=0;check(labor.capability(CapabilityId::LaborSkill)==0,"labor skill accepts its lower bound");
    labor.skill=100;check(labor.capability(CapabilityId::LaborSkill)==100,"labor skill accepts its upper bound");
}
{
    constexpr uint32_t selected=0b101001; // strategy, diplomacy, military
    Realm custom=Realm::scenario(412,129,30,-1,-1,-1,-1,-1,-1,-1,selected);
    const auto& actor=custom.person(129);
    check(custom.capability_mask()==selected&&custom.capability_active(LegacyCapability::Strategy)&&
          !custom.capability_active(LegacyCapability::Administration)&&custom.capability_active(LegacyCapability::Diplomacy),
          "scenario selects a nonempty subset by stable capability ID");
    check(custom.capability_value(129,LegacyCapability::Administration)==kInactiveCapabilityNeutralValue&&
          custom.capability_value(129,LegacyCapability::Investigation)==kInactiveCapabilityNeutralValue,
          "omitted capability reads the documented neutral value");
    check(custom.capability_value(129,LegacyCapability::Strategy)==actor.stats.get(LegacyCapability::Strategy),
          "active capability retains the Person-owned legacy value");
    check(custom.capability_value(129,CapabilityId::LaborSkill)==actor.skill&&
          std::string(kCapabilityKeys[6])=="labor_skill",
          "labor_skill catalog value comes from the same Person field used by gameplay");
    check(custom.capability_schema().back()==CapabilityId::LaborSkill&&custom.capability_schema().size()==4,
          "scenario keeps labor skill in the always-required capability schema when legacy dimensions are disabled");
    Realm mutedAdmin(413,1,128,false,GameplayConfig{},0,selected);
    Realm neutralAdmin(413,1,128);RealmObservationTestAccess::mutable_person(neutralAdmin,1).stats.set(LegacyCapability::Administration,kInactiveCapabilityNeutralValue);
    check(RealmObservationTestAccess::report_delay(mutedAdmin)==RealmObservationTestAccess::report_delay(neutralAdmin),
          "a removed gameplay capability uses 50 in the existing report-delay formula");
    const auto observed=custom.observation(129);
    check(observed[10]==actor.stats.get(LegacyCapability::Strategy)/100.f&&
          observed[11]==kInactiveCapabilityNeutralValue/100.f&&
          observed[13]==actor.stats.get(LegacyCapability::Diplomacy)/100.f&&
          observed[14]==kInactiveCapabilityNeutralValue/100.f,
          "observation preserves slot meaning and fills omitted dimensions with neutral values");
    const auto defaultContract=Realm(1,1,128).model_contract();
    check(defaultContract=="realm-household-fund-goods30-config-v54-atlas-north-pass-v1-salted-meal-v1"&&
          custom.model_contract()==defaultContract+"-caps-41",
          "custom schema gets a distinct model contract while default remains compatible");
    const auto saved=custom.serialize("capability-schema");
    check(saved.rfind("POLYEC_REALM_76 ",0)==0&&saved.find(" CAPABILITY_SCHEMA_1 41 ")!=std::string::npos,
          "current save stores selected capability IDs as a stable mask");
    Realm restored(1,1,128);std::string error;
    const bool customLoaded=restored.deserialize(saved,"capability-schema",error);
    check(customLoaded,error.c_str());
    check(restored.capability_mask()==selected&&restored.person(129).stats.values==actor.stats.values&&
          restored.observation(129)==observed&&restored.model_contract()==custom.model_contract(),
          "custom schema save/load preserves mask, Person values, observation and model identity");
    std::string schema31=RealmObservationTestAccess::as_schema32(custom,"capability-schema");const auto capabilityAt=schema31.rfind(" CAPABILITY_SCHEMA_1 ");
    schema31.erase(capabilityAt);schema31.replace(0,15,"POLYEC_REALM_31");
    Realm migrated(1,1,128);
    const bool schema31Loaded=migrated.deserialize(schema31,"capability-schema",error);
    check(schema31Loaded,error.c_str());
    check(migrated.capability_mask()==kDefaultCapabilityMask,
          "schema31 migration restores the default capability set");
    check(migrated.person(129).stats.values==actor.stats.values,
          "schema31 migration restores all six legacy dimensions without changing Person values");
    const auto stable=restored.fingerprint();
    Realm invalidLowSkill=custom;RealmObservationTestAccess::mutable_person(invalidLowSkill,129).skill=-1;
    check(!invalidLowSkill.invariant(),"labor skill below zero fails the Realm invariant");
    check(!restored.deserialize(invalidLowSkill.serialize("capability-schema"),"capability-schema",error)&&
          restored.fingerprint()==stable,"save with negative labor skill is rejected atomically");
    Realm invalidHighSkill=custom;RealmObservationTestAccess::mutable_person(invalidHighSkill,129).skill=101;
    check(!invalidHighSkill.invariant(),"labor skill above 100 fails the Realm invariant");
    check(!restored.deserialize(invalidHighSkill.serialize("capability-schema"),"capability-schema",error)&&
          restored.fingerprint()==stable,"save with labor skill above 100 is rejected atomically");
    std::string emptyMask=saved;const auto maskAt=emptyMask.rfind(" CAPABILITY_SCHEMA_1 ");
    emptyMask.replace(maskAt,std::string(" CAPABILITY_SCHEMA_1 41 ").size()," CAPABILITY_SCHEMA_1 0 ");
    check(!restored.deserialize(emptyMask,"capability-schema",error)&&restored.fingerprint()==stable,
          "empty schema mask is rejected without replacing the loaded Realm");
    std::string unknownMask=saved;const auto unknownAt=unknownMask.rfind(" CAPABILITY_SCHEMA_1 ");
    unknownMask.replace(unknownAt,std::string(" CAPABILITY_SCHEMA_1 41 ").size()," CAPABILITY_SCHEMA_1 64 ");
    check(!restored.deserialize(unknownMask,"capability-schema",error)&&restored.fingerprint()==stable,
          "unregistered capability bit is rejected without replacing the loaded Realm");
    std::string missingSection=saved;missingSection.erase(missingSection.rfind(" CAPABILITY_SCHEMA_1 "));
    check(!restored.deserialize(missingSection,"capability-schema",error)&&restored.fingerprint()==stable,
          "schema32 missing capability section is rejected without replacing the loaded Realm");
    Realm outOfRange=custom;RealmObservationTestAccess::mutable_person(outOfRange,129).stats.set(LegacyCapability::Strategy,101);
    check(!outOfRange.invariant(),"capability values above the documented 0–100 range fail the Realm invariant");
    const auto invalidValueSave=outOfRange.serialize("capability-schema");
    check(!restored.deserialize(invalidValueSave,"capability-schema",error)&&restored.fingerprint()==stable,
          "save with an out-of-range Person capability is rejected atomically");
    bool rejected=false;try{Realm invalid(1,1,128,false,GameplayConfig{},0,0);}catch(const std::invalid_argument&){rejected=true;}
    check(rejected,"scenario rejects an empty capability schema before world creation");
}
{
    auto rejects_unknown_observer=[](const Realm& world,uint64_t observer){
        try{(void)world.view(observer);}
        catch(const std::out_of_range& error){return std::string(error.what())=="person ID";}
        return false;
    };
    Realm empty(41,1,128);
    check(rejects_unknown_observer(empty,0),"empty Realm view rejects observer ID zero");
    check(rejects_unknown_observer(empty,empty.count()+1),"empty Realm view rejects observer beyond population");
    Realm public_fact(42,1,128);RealmObservationTestAccess::add_test_fact(public_fact,public_fact.person(1).region,false);
    check(rejects_unknown_observer(public_fact,0),"public fact view rejects invalid observer before regional lookup");
    Realm secret_fact(43,1,128);RealmObservationTestAccess::add_test_fact(secret_fact,secret_fact.person(1).region,true);
    check(rejects_unknown_observer(secret_fact,secret_fact.count()+1),"secret fact view rejects invalid observer before secret filtering");
    check(empty.view(1).day==empty.day()&&public_fact.view(1).facts.size()==1&&
          secret_fact.view(1).facts.size()==1,"valid fact owners continue to read their existing facts");
}
{
    const std::string bundle="knowledge-provenance";
    Realm world(44,1,128);const int region=world.person(1).region;
    const uint64_t fact=RealmObservationTestAccess::add_test_fact(world,region,true,2);
    const auto beforeInvestigationView=world.view(1);
    check(std::none_of(beforeInvestigationView.facts.begin(),beforeInvestigationView.facts.end(),
        [fact](const Fact& row){return row.id==fact;}),"secret fact stays hidden from unrelated observer before investigation");
    check(std::none_of(beforeInvestigationView.knowledge.begin(),beforeInvestigationView.knowledge.end(),
        [fact](const PlayerKnowledge& row){return row.fact==fact;}),"secret evidence has no PlayerKnowledge row before Investigation");
    RealmObservationTestAccess::endow_person_good(world,1,GoodId::Paper,1);
    const auto command=world.candidates(1)[13];
    const auto result=world.execute({1,world.person(1).decisions+1,command});
    if(!result.success)std::cerr<<"Investigation fixture rejected: "<<result.reason<<'\n';
    check(result.success,"real investigation command is accepted");
    const auto investigation=world.investigations().back();
    world.advance(investigation.due-world.day());
    const auto createdKnowledge=RealmObservationTestAccess::knowledge(world,fact);
    check(createdKnowledge.source_investigation_id==investigation.id&&
          createdKnowledge.kind==KnowledgeKind::SecretEvidence&&
          createdKnowledge.confidence_basis_points==kUnknownKnowledgeConfidence&&world.invariant(),
          "investigation records sourced secret evidence without claiming measured confidence");
    const auto agentObservation=world.observation(1),otherAgentObservation=world.observation(2);
    const size_t evidenceCount=RealmObservationTestAccess::knowledge_count(world,1,KnowledgeKind::SecretEvidence);
    const size_t legacyCount=RealmObservationTestAccess::knowledge_count(world,1,KnowledgeKind::UnknownLegacy);
    check(agentObservation[realm_knowledge_evidence_observation_index]==float(evidenceCount)/10.f&&
          agentObservation[realm_knowledge_legacy_observation_index]==float(legacyCount)/10.f,
          "policy receives the current observer's counts for sourced evidence and unclassified legacy rows");
    check(otherAgentObservation[realm_knowledge_evidence_observation_index]==
              float(RealmObservationTestAccess::knowledge_count(world,2,KnowledgeKind::SecretEvidence))/10.f&&
          otherAgentObservation[realm_knowledge_legacy_observation_index]==
              float(RealmObservationTestAccess::knowledge_count(world,2,KnowledgeKind::UnknownLegacy))/10.f,
          "another actor's Knowledge rows do not appear in the investigation owner's observation");
    const auto ownerKnowledge=world.view(1).knowledge;
    const auto otherKnowledge=world.view(2).knowledge;
    check(std::any_of(ownerKnowledge.begin(),ownerKnowledge.end(),[&](const PlayerKnowledge& row){
              return row.fact==fact&&row.observer==1&&row.source_investigation_id==investigation.id&&
                     row.kind==KnowledgeKind::SecretEvidence&&row.confidence_basis_points==-1;
          })&&std::none_of(otherKnowledge.begin(),otherKnowledge.end(),[fact](const PlayerKnowledge& row){return row.fact==fact;}),
          "RealmView gives evidence source only to the observer whose investigation produced it");

    Realm restored(45,1,128);std::string error;
    const bool restoredOk=restored.deserialize(world.serialize(bundle),bundle,error);
    check(restoredOk,(std::string("schema34 Realm load: ")+error).c_str());
    const auto restoredView=restored.view(1);
    check(RealmObservationTestAccess::knowledge_source(restored,fact)==investigation.id&&
          RealmObservationTestAccess::knowledge(restored,fact).kind==KnowledgeKind::SecretEvidence&&
          RealmObservationTestAccess::knowledge(restored,fact).confidence_basis_points==kUnknownKnowledgeConfidence&&
          std::any_of(restoredView.facts.begin(),restoredView.facts.end(),
              [fact](const Fact& row){return row.id==fact;}),
          "schema34 save/load preserves investigation evidence and the existing filtered view");
    check(std::any_of(restoredView.knowledge.begin(),restoredView.knowledge.end(),[&](const PlayerKnowledge& row){
              return row.fact==fact&&row.source_investigation_id==investigation.id&&row.kind==KnowledgeKind::SecretEvidence;
          }),"save/load keeps the observer-specific PlayerKnowledge view");

    const auto schema32=RealmObservationTestAccess::as_schema32(world,bundle);
    Realm migrated32(451,1,128);error.clear();
    check(migrated32.deserialize(schema32,bundle,error),(std::string("schema32 knowledge migration: ")+error).c_str());
    const auto migrated32Knowledge=RealmObservationTestAccess::knowledge(migrated32,fact);
    check(migrated32Knowledge.kind==KnowledgeKind::SecretEvidence&&
          migrated32Knowledge.source_investigation_id==investigation.id&&
          migrated32Knowledge.confidence_basis_points==kUnknownKnowledgeConfidence,
          "schema32 source IDs migrate to evidence type while confidence remains unknown");
    std::string schema31=schema32;const size_t schema31Capability=schema31.rfind(" CAPABILITY_SCHEMA_1 ");
    check(schema31Capability!=std::string::npos,"schema31 migration fixture has the schema32-only capability section");
    schema31.erase(schema31Capability);schema31.replace(0,15,"POLYEC_REALM_31");
    Realm migrated31(452,1,128);error.clear();
    check(migrated31.deserialize(schema31,bundle,error),(std::string("schema31 knowledge migration: ")+error).c_str());
    const auto migrated31Knowledge=RealmObservationTestAccess::knowledge(migrated31,fact);
    check(migrated31Knowledge.kind==KnowledgeKind::SecretEvidence&&
          migrated31Knowledge.source_investigation_id==investigation.id&&
          migrated31Knowledge.confidence_basis_points==kUnknownKnowledgeConfidence,
          "schema31 source IDs migrate to evidence type while confidence remains unknown");

    const std::string oldSave=RealmObservationTestAccess::as_schema30_without_knowledge_sources(world,bundle);
    Realm migrated(46,1,128);error.clear();
    const bool migratedOk=migrated.deserialize(oldSave,bundle,error);
    check(migratedOk,(std::string("schema30 migration: ")+error).c_str());
    check(RealmObservationTestAccess::knowledge_source(migrated,fact)==0&&
          migrated.serialize(bundle).rfind("POLYEC_REALM_76 ",0)==0&&
          RealmObservationTestAccess::knowledge(migrated,fact).kind==KnowledgeKind::UnknownLegacy&&
          RealmObservationTestAccess::knowledge(migrated,fact).confidence_basis_points==kUnknownKnowledgeConfidence&&
          migrated.observation(1)[realm_knowledge_legacy_observation_index]==
              float(RealmObservationTestAccess::knowledge_count(migrated,1,KnowledgeKind::UnknownLegacy))/10.f,
          "schema30 migration preserves knowledge without inventing type, source or confidence");

    auto rejected_atomically=[&](Realm broken,const char* label){
        Realm destination(47,1,128);const auto before=destination.fingerprint();error.clear();
        const bool loaded=destination.deserialize(broken.serialize(bundle),bundle,error);
        check(!loaded&&destination.fingerprint()==before,label);
    };
    Realm badSource=world;RealmObservationTestAccess::set_knowledge_source(badSource,UINT64_MAX);
    rejected_atomically(std::move(badSource),"unknown investigation source ID is rejected without changing destination");
    Realm wrongOwner=world;RealmObservationTestAccess::set_knowledge_observer(wrongOwner,2);
    rejected_atomically(std::move(wrongOwner),"knowledge source owned by another person is rejected atomically");
    Realm wrongRegion=world;RealmObservationTestAccess::set_investigation_region(wrongRegion,region==0?1:0);
    rejected_atomically(std::move(wrongRegion),"knowledge source from another region is rejected atomically");
    Realm notSecret=world;RealmObservationTestAccess::set_fact_secret(notSecret,fact,false);
    rejected_atomically(std::move(notSecret),"investigation cannot source a non-secret fact in save data");
    Realm badKind=world;RealmObservationTestAccess::set_knowledge_kind(badKind,static_cast<KnowledgeKind>(9));
    check(!badKind.invariant(),"unsupported Knowledge type fails the Realm invariant");
    rejected_atomically(std::move(badKind),"save containing an unsupported Knowledge type is rejected atomically");
    Realm badConfidence=world;RealmObservationTestAccess::set_knowledge_confidence(badConfidence,10001);
    check(!badConfidence.invariant(),"confidence above 10000 basis points fails the Realm invariant");
    rejected_atomically(std::move(badConfidence),"save containing out-of-range confidence is rejected atomically");
    Realm legacyWithSource=world;RealmObservationTestAccess::set_knowledge_kind(legacyWithSource,KnowledgeKind::UnknownLegacy);
    check(!legacyWithSource.invariant(),"legacy unknown type cannot retain a current investigation source");
    rejected_atomically(std::move(legacyWithSource),"save pairing legacy unknown type with a source is rejected atomically");
}
test_estate_cash_claim_death_transition();
test_inherited_workshop_continues_work_and_production();
test_realm_recipe_rejects_same_group_substitutions();
test_estate_keeps_household_fund_outside_personal_claim();
test_household_fund_survives_authority_death();
test_household_fund_stays_locked_with_only_minor_survivor();
test_household_fund_authority_unlocks_at_adulthood();
test_estate_parent_sibling_and_public_fallback_through_realm();
test_estate_unverified_parentage_does_not_inherit_through_realm();
test_estate_legal_child_precedes_spouse_through_realm();
test_estate_fallback_land_titles_through_realm();
test_deceased_appointee_releases_authority_office_slot();
test_estate_does_not_mutate_other_living_property();
test_estate_underfunded_claim_death_transition();
test_estate_multiple_heirs_cash_goods_death_transition();
test_minor_estate_land_guardian_continuation();
test_complex_actor_population_parameter();
test_canonical_market_transactions();
    {
        std::vector<Person> source(3),decoded(3);
        source[0].id=1;source[1].id=2;source[2].id=3;
        source[1].body_state={37,82};source[2].body_state={0,1};
        std::ostringstream encoded;detail::write_body_state_section(encoded,source);
        std::istringstream input(encoded.str());detail::read_body_state_section(input,decoded,2,30);
        check(decoded[0].body_state.sleep_debt==0&&decoded[0].body_state.exertion==0,"sparse body codec default");
        check(decoded[1].body_state.sleep_debt==37&&decoded[1].body_state.exertion==82&&decoded[2].body_state.exertion==1,"sparse body codec roundtrip");
        auto malformed=encoded.str();const auto value=malformed.find("37 82");check(value!=std::string::npos,"body codec corruption fixture");
        malformed.replace(value,5,"101 82");std::istringstream bad(malformed);std::vector<Person> unchanged(3);bool rejected=false;
        try{detail::read_body_state_section(bad,unchanged,2,30);}catch(const std::runtime_error&){rejected=true;}
        check(rejected&&unchanged[1].body_state.sleep_debt==0,"body codec out-of-range rejects before publish");
        Realm latest(37,2,128);const auto current=latest.serialize("body-state");
        check(current.rfind("POLYEC_REALM_76 ",0)==0&&current.find(" KNOWLEDGE_3 0 ")!=std::string::npos&&current.find(" BODY_3 0 ")!=std::string::npos&&current.find(" HOUSEHOLD_1 0 ")!=std::string::npos&&current.find(" LAND_1 ")!=std::string::npos&&current.find(" LANDUSE_1 0 ")!=std::string::npos&&current.find(" PARENTAGE_1 ")!=std::string::npos&&current.find(" MARKET_2 ")!=std::string::npos&&current.find(" SCHEDULE_7 ")!=std::string::npos&&current.find(" CAPABILITY_SCHEMA_1 63 ")!=std::string::npos&&current.find(" PERSONALITY_1 128 ")!=std::string::npos&&current.find(" URBAN_HOUSING_1 ")!=std::string::npos,"schema40 saves existing world data, source ledgers, urban housing and six personality values per person");
        std::string legacy=test_fixtures::save_as_v25(current);const auto body=legacy.find(" BODY_2 0 ");check(body!=std::string::npos,"empty body section fixture");legacy.erase(body);legacy.replace(0,15,"POLYEC_REALM_17");
        Realm migrated(1,1,128);std::string error;check(migrated.deserialize(legacy,"body-state",error),"schema 17 body migration");
        check(migrated.person(1).body_state.sleep_debt==0&&migrated.person(1).body_state.exertion==0,"schema 17 body defaults");
        auto v18=test_fixtures::save_as_v25(current);const auto bodyTag=v18.find(" BODY_2 0 ");check(bodyTag!=std::string::npos,"v18 fixture body section");v18.replace(bodyTag,std::string(" BODY_2 0 ").size()," BODY_1 1 1 37 82 ");const auto v18Fund=v18.find(" HOUSEHOLD_1 ");check(v18Fund!=std::string::npos,"v18 fixture household section");v18.erase(v18Fund);v18.replace(0,15,"POLYEC_REALM_18");
        Realm migrated18(1,1,128);check(migrated18.deserialize(v18,"body-state",error),"schema 18 body migration");
        check(migrated18.person(1).body_state.sleep_debt==37&&migrated18.person(1).body_state.exertion==82,"schema 18 body fields retained");
        auto populated=current;const auto emptyBody=populated.find(" BODY_3 0 ");populated.replace(emptyBody,std::string(" BODY_3 0 ").size()," BODY_3 1 1 37 82 0 0 0 0 0 0 -1 0 ");
        Realm roundTrip(1,1,128);const bool bodyLoaded=roundTrip.deserialize(populated,"body-state",error);check(bodyLoaded,error.c_str());
        check(roundTrip.person(1).body_state.sleep_debt==37&&roundTrip.person(1).body_state.exertion==82&&roundTrip.serialize("body-state")==populated,"nonzero body save exact roundtrip");
        Realm preserved(37,2,128);const auto before=preserved.fingerprint();
        auto invalid=current;const auto empty=invalid.find(" BODY_3 0 ");check(empty!=std::string::npos,"body bounds corruption fixture");
        invalid.replace(empty,std::string(" BODY_3 0 ").size()," BODY_3 1 1 101 0 0 0 0 0 0 -1 0 ");
        check(!preserved.deserialize(invalid,"body-state",error)&&preserved.fingerprint()==before,"invalid body save rejects atomically");
    }
    Realm appointment_save_oracle(7,2,128);
    auto historicalOracle=test_fixtures::save_as_v25(appointment_save_oracle.serialize("fingerprint"));
    {
        // A pre-land save had no title IDs reserved in the shared non-Person namespace.
        std::istringstream header(historicalOracle);std::string magic,bundle;int day=0,regions=0;
        uint64_t seed=0,next=0;header>>magic>>std::quoted(bundle)>>day>>regions>>seed;
        const auto afterSeed=header.tellg();header>>next;
        const auto begin=static_cast<size_t>(afterSeed)+1;
        const auto end=historicalOracle.find(' ',begin);
        check(end!=std::string::npos&&next>=appointment_save_oracle.land_holdings().size(),
              "historical save fixture locates pre-land ID watermark");
        historicalOracle.replace(begin,end-begin,
            std::to_string(next-appointment_save_oracle.land_holdings().size()));
    }
    historicalOracle.erase(historicalOracle.find(" CONFIG_1 "));
    historicalOracle.replace(historicalOracle.find("ESTABLISH_5"),11,"ESTABLISH_3");
    historicalOracle.replace(0,15,"POLYEC_REALM_14");
    uint64_t historicalHash=1469598103934665603ull;
    for(unsigned char c:historicalOracle){historicalHash^=c;historicalHash*=1099511628211ull;}
    check(historicalHash==4173853339963961369ULL,
          "default configuration preserves complete historical v14 save bytes");
    check(Realm::action_name(Candidate{Intent::Hold})=="Chờ"&&
          Realm::action_name(Candidate{Intent::Hire})=="Tuyển dụng"&&
          Realm::action_name(Candidate{Intent::CreateEstablishment})=="Tạo cơ sở sản xuất"&&
          Realm::action_name(Candidate{Intent::TransferEstablishmentGoods})=="Chuyển hàng xưởng"&&
          Realm::action_name(Candidate{Intent::AuthorizeWorkAtSite})=="Cho phép làm tại xưởng"&&
          Realm::action_name(Candidate{Intent::WorkAtSite})=="Làm tại xưởng",
          "action labels remain Vietnamese UTF-8");
    Realm w(37,6,10000);check(w.invariant(),"opening invariant");check(w.count()==10000,"real persons");check(w.relations().size()>19000,"sparse family/social graph");
    {
        Realm bodyObservation(37,2,128);
        RealmObservationTestAccess::set_body(bodyObservation,1,0,0);
        RealmObservationTestAccess::set_body(bodyObservation,2,50,25);
        RealmObservationTestAccess::set_body(bodyObservation,3,100,100);
        const auto beforeRead=bodyObservation.fingerprint();
        const auto first=bodyObservation.observation(1),middle=bodyObservation.observation(2),last=bodyObservation.observation(3);
        check(first[108]==0.f&&first[109]==0.f&&middle[108]==.5f&&middle[109]==.25f&&
              last[108]==1.f&&last[109]==1.f,
              "body observation uses normalized sleep debt/exertion in state slots 108/109");
        check(bodyObservation.observation(2)[108]==middle[108]&&bodyObservation.observation(2)[109]==middle[109],
              "each policy observation reads the requested actor's own body values");
        check(bodyObservation.fingerprint()==beforeRead&&bodyObservation.invariant(),
              "reading body observation does not mutate the Realm");
        check(std::string(bodyObservation.model_contract())=="realm-household-fund-goods30-config-v54-atlas-north-pass-v1-salted-meal-v1",
              "non-timeline body observation has a new model contract");
        const auto timelineResult=bodyObservation.execute(ScheduleCommand{1,
            bodyObservation.person(1).decisions+1,ScheduleCommand::Operation::Submit,0,0,1,0,1000});
        check(timelineResult.success&&bodyObservation.model_contract()==
              std::string("realm-worksite-payroll-goods30-config-v54-atlas-north-pass-v1-salted-meal-v1"),
              "timeline body observation has a separately versioned model contract");
        Realm historicalBody(37,2,128,false,GameplayConfig::historical());
        check(std::string(historicalBody.model_contract())=="realm-household-fund-goods30-v49",
              "historical-default non-timeline contract includes the body observation version");
        const auto historicalTimeline=historicalBody.execute(ScheduleCommand{1,
            historicalBody.person(1).decisions+1,ScheduleCommand::Operation::Submit,0,0,1,0,1000});
        check(historicalTimeline.success&&std::string(historicalBody.model_contract())==
              "realm-worksite-payroll-goods30-v49",
              "historical-default timeline contract includes the body observation version");
    }
    {
        uint64_t economic=0;
        for(uint64_t id=1;id<=uint64_t(w.count());++id)if(w.person(id).family==Family::Economic){economic=id;break;}
        check(economic!=0,"canonical goods fixture has economic actor");
        auto candidates=w.candidates(economic);
        std::array<bool,kGoodIdCount> bought{},sold{};
        for(int slot=1;slot<8;++slot){const auto good=canonical_from_legacy_good(slot-1);check(candidates[slot].kind==Intent::Buy&&candidates[slot].good==good,"legacy Buy slot maps to canonical GoodId");check(candidates[8+slot-1].kind==Intent::Sell&&candidates[8+slot-1].good==good,"legacy Sell slot maps to canonical GoodId");bought[size_t(good)]=true;sold[size_t(good)]=true;}
        for(int ordinal=0;ordinal<kGoodIdCount-kLegacyGoodCount;++ordinal){const int buy=32+ordinal,sell=52+ordinal;const auto good=candidates[buy].good;check(candidates[buy].kind==Intent::Buy&&candidates[sell].kind==Intent::Sell&&candidates[sell].good==good&&valid_good_id(good),"added slots expose paired canonical Buy/Sell");check(!bought[size_t(good)]&&!sold[size_t(good)],"canonical product appears once per trade direction");bought[size_t(good)]=true;sold[size_t(good)]=true;}
        for(int good=0;good<kGoodIdCount;++good)check(bought[size_t(good)]&&sold[size_t(good)],"all catalog products have Buy and Sell action slots");
        auto invalidGoodBefore=w.fingerprint();
        auto invalidBuy=w.execute({economic,w.person(economic).decisions+1,{Intent::Buy,w.person(economic).region,kGoodIdCount,1}});
        check(!invalidBuy.success&&w.fingerprint()==invalidGoodBefore,"out-of-catalog GoodId Buy rejects atomically before indexing");
        auto unavailableGoodBefore=w.fingerprint();
        auto unavailableBuy=w.execute({economic,w.person(economic).decisions+1,{Intent::Buy,w.person(economic).region,int(GoodId::GeneralLuxury),1}});
        check(!unavailableBuy.success&&w.fingerprint()==unavailableGoodBefore,"unstocked canonical product Buy rejects atomically");
    }
    {
        Realm traits(711,2,128),sameSeed(711,2,128),otherSeed(712,2,128);
        bool varied=false,allInRange=true,allRepeatable=true;
        for(uint64_t id=1;id<=uint64_t(traits.count());++id){
            const auto& values=traits.person(id).personality.values;
            allRepeatable&=values==sameSeed.person(id).personality.values;
            for(const int value:values)allInRange&=value>=25&&value<=75;
            varied|=values!=traits.person(1).personality.values;
        }
        check(allRepeatable&&allInRange,"seeded personality is repeatable and within [25,75] for every person");
        check(varied&&traits.person(1).personality.values!=otherSeed.person(1).personality.values,
              "different people and world seeds produce different personality profiles");
        const auto rngBefore=RealmObservationTestAccess::peek_rng(traits);
        RealmObservationTestAccess::set_personality(traits,1,{0,20,40,60,80,100});
        check(RealmObservationTestAccess::peek_rng(traits)==rngBefore,
              "personality values do not consume the Realm random-number stream");
        RealmObservationTestAccess::set_personality(traits,2,{100,80,60,40,20,0});
        const auto observation=traits.observation(1);
        check(observation[110]==0.f&&observation[111]==.2f&&observation[112]==.4f&&observation[113]==.6f&&observation[114]==.8f&&observation[115]==1.f,
              "observation slots 110 through 115 expose the current actor's six normalized traits");
        const auto saved=traits.serialize("personality");Realm loaded(9,1,128);std::string error;
        check(loaded.deserialize(saved,"personality",error)&&loaded.person(1).personality.values==traits.person(1).personality.values&&
              loaded.person(2).personality.values==traits.person(2).personality.values,
              "schema35 saves and restores each person's six values");
        const auto oldSave=test_fixtures::save_as_v34(saved);Realm oldLoaded(9,1,128);
        check(oldLoaded.deserialize(oldSave,"personality",error),"schema34 remains loadable");
        const bool neutral=std::all_of(oldLoaded.person(1).personality.values.begin(),oldLoaded.person(1).personality.values.end(),[](int value){return value==kPersonalityNeutral;});
        check(neutral&&oldLoaded.person(2).personality.values==Personality{}.values,
              "schema34 migration assigns neutral 50 to all six traits");
        Realm preserved(9,1,128);const auto before=preserved.fingerprint();auto invalid=saved;
        const auto tag=invalid.find(" PERSONALITY_1 128 ");check(tag!=std::string::npos,"trait corruption fixture locates the saved section");
        const auto value=tag+std::string(" PERSONALITY_1 128 ").size();const auto end=invalid.find(' ',value);
        invalid.replace(value,end-value,"101");
        check(!preserved.deserialize(invalid,"personality",error)&&preserved.fingerprint()==before,
              "out-of-range saved trait rejects without partially replacing the destination");
    }
    // M5 v11: policy candidates use the same typed transactions as the player path,
    // including candidate access after the timeline contract has been enabled.
    {
        Realm policyWork(37,6,256);
        uint64_t owner=0;
        for(uint64_t id=1;id<=uint64_t(policyWork.count());++id){
            const auto&p=policyWork.person(id);
            if(p.family==Family::Economic&&p.cash>=25){owner=id;break;}
        }
        check(owner!=0,"policy worksite fixture has funded economic owner");
        auto choices=policyWork.candidates(owner);
        const auto noSiteMask=policyWork.mask(owner);
        check(std::all_of(noSiteMask.begin()+realm_worksite_candidate_begin,noSiteMask.end(),
                          [](float value){return value==0.f;}),
              "empty worksite rows have no Hold aliases");
        check(policyWork.mask(owner)[80]>0,"v11 private site candidate available");
        auto created=policyWork.execute({owner,policyWork.person(owner).decisions+1,choices[80]});
        check(created.success&&policyWork.establishments().size()==1,"v11 candidate creates private Cloth workshop");
        const uint64_t site=policyWork.establishments().front().id;
        const int region=policyWork.establishments().front().region;
        check(policyWork.mask(owner)[realm_worksite_candidate_begin]==0,
              "policy cannot deposit recipe input before holding Fiber");
        choices=policyWork.candidates(owner);
        int fiberBuy=-1;
        for(int slot=0;slot<realm_establishment_candidate_begin;++slot)
            if(choices[size_t(slot)].kind==Intent::Buy&&choices[size_t(slot)].good==int(GoodId::Fiber)){fiberBuy=slot;break;}
        check(fiberBuy>=0&&policyWork.mask(owner)[size_t(fiberBuy)]>0,"policy can buy canonical Fiber before deposit");
        auto boughtFiber=policyWork.execute({owner,policyWork.person(owner).decisions+1,choices[size_t(fiberBuy)]});
        check(boughtFiber.success,"policy buys Fiber through Realm command");
        choices=policyWork.candidates(owner);
        const auto oneSiteMask=policyWork.mask(owner);
        check(choices[101].target==int(site)&&oneSiteMask[101]>0&&oneSiteMask[102]==0&&oneSiteMask[106]==0,
              "policy sees only valid input/output actions for its sampled site");
        const auto siteObservation=policyWork.observation(owner);
        const size_t transferFeatures=size_t(realm_state+103*realm_features);
        const size_t workFeatures=size_t(realm_state+105*realm_features);
        check(std::abs(siteObservation[transferFeatures+8])<1e-7f&&
              std::abs(siteObservation[transferFeatures+9]-.001f)<1e-7f&&
              std::abs(siteObservation[workFeatures+8]-.025f)<1e-7f,
              "site policy features expose stock, capacity, and site cash");
        check(policyWork.execute({owner,policyWork.person(owner).decisions+1,choices[101]}).success,
              "policy deposits recipe input through typed transfer");
        ScheduleCommand timeline{};
        timeline.actor=owner;timeline.sequence=policyWork.person(owner).decisions+1;
        timeline.operation=ScheduleCommand::Operation::Submit;timeline.begin=policyWork.game_tick();
        timeline.end=timeline.begin+1;timeline.demand_rate=activity::kMaxDemandRate;
        timeline.kind=ActivityKind::Generic;timeline.region=-1;
        check(policyWork.execute(timeline).success&&policyWork.timeline_contract(),"fixture enters Work timeline");
        choices=policyWork.candidates(owner);
        check(policyWork.mask(owner)[104]>0&&choices[104].target==int(site),"policy candidates remain available after timeline migration");
        const int worker=int(choices[104].good);
        check(worker>0&&worker!=int(owner),"policy selects another present worker for authorization");
        const auto authorizationObservation=policyWork.observation(owner);
        const size_t authorizationFeatures=size_t(realm_state+104*realm_features);
        check(authorizationObservation[authorizationFeatures+10]==policyWork.person(uint64_t(worker)).skill/100.f&&
              authorizationObservation[authorizationFeatures+8]==policyWork.establishments().front().cash/1000.f,
              "authorization features expose selected worker skill and site cash");
        check(policyWork.execute({owner,policyWork.person(owner).decisions+1,choices[104]}).success,
              "policy grants 31-day Work authorization through typed command");
        auto workerChoices=policyWork.candidates(uint64_t(worker));
        check(policyWork.mask(uint64_t(worker))[105]>0&&workerChoices[105].target==int(site),"authorized worker receives site Work candidate");
        auto work=policyWork.execute({uint64_t(worker),policyWork.person(uint64_t(worker)).decisions+1,workerChoices[105]});
        check(work.success&&policyWork.invariant(),"worker schedules one-day Work at the selected site");
        const auto continuation=policyWork.serialize("v11-worksite");
        Realm resumed(1,1,128);std::string resumeError;
        check(resumed.deserialize(continuation,"v11-worksite",resumeError),resumeError.c_str());
        policyWork.advance();resumed.advance();
        check(policyWork.fingerprint()==resumed.fingerprint()&&policyWork.invariant(),
              "production and payroll continue identically after save/load");
        check(policyWork.establishments().front().stock[size_t(GoodId::Fiber)]==0&&
              policyWork.establishments().front().stock[size_t(GoodId::Cloth)]==1,
              "one attended workday consumes one Fiber and produces one Cloth");
        auto afterProduction=policyWork.candidates(owner);
        check(afterProduction[103].target==int(site)&&policyWork.mask(owner)[103]>0,
              "owner receives the produced Cloth withdrawal candidate");
        const int64_t clothBefore=policyWork.person(owner).inventory[size_t(GoodId::Cloth)];
        check(policyWork.execute({owner,policyWork.person(owner).decisions+1,afterProduction[103]}).success,
              "owner withdraws produced Cloth through typed transfer");
        const auto resumedChoices=resumed.candidates(owner);
        check(resumed.execute({owner,resumed.person(owner).decisions+1,resumedChoices[103]}).success&&
              resumed.fingerprint()==policyWork.fingerprint()&&
              policyWork.person(owner).inventory[size_t(GoodId::Cloth)]==clothBefore+1&&
              policyWork.establishments().front().stock[size_t(GoodId::Cloth)]==0,
              "output withdrawal conserves product and preserves save continuation replay");
    }
    auto before=w.fingerprint();auto r=w.execute({1,1,{Intent::Tax,2,0,20}});check(!r.success&&w.fingerprint()==before,"unauthorized tax is atomic");
    check(w.execute({1,1,{Intent::Tax,w.person(1).region,0,20}}).success,"charter local tax");before=w.fingerprint();check(!w.execute({1,1,{Intent::Tax,w.person(1).region,0,5}}).success&&w.fingerprint()==before,"duplicate command");
    uint64_t merchant=65;int64_t wealth=w.person(merchant).cash-w.debt(merchant);check(w.execute({merchant,1,{Intent::Borrow,w.person(merchant).region,0,100,180}}).success,"funded loan");check(w.person(merchant).cash-w.debt(merchant)==wealth-4,"loan not income with interest");
    check(w.execute({merchant,2,{Intent::Repay,w.person(merchant).region,0,100}}).success,"repay");check(w.invariant(),"loan invariant");
    check(w.execute({1,2,{Intent::Project,w.person(1).region,0,200,60}}).success,"project funded and materials");w.advance(65);check(w.projects()[0].completed&&w.view().provinces[w.person(1).region].road==1,"project duration and stat");
    std::string data=w.serialize("test"),error;Realm restored(2,1,128);check(restored.deserialize(data,"test",error),"save load");check(restored.fingerprint()==w.fingerprint(),"save exact state RNG");before=restored.fingerprint();check(!restored.deserialize(data,"wrong",error)&&restored.fingerprint()==before,"wrong bundle atomic");check(!restored.deserialize(data.substr(0,data.size()/2),"test",error)&&restored.fingerprint()==before,"truncated save atomic");
    w.advance(100);restored.advance(100);check(w.fingerprint()==restored.fingerprint(),"continued replay");
    for(int seed=0;seed<60;++seed){Realm draft(seed,6,128);auto d=draft.view().draft;check(d.cards[0]!=d.cards[1]&&d.cards[1]!=d.cards[2]&&d.cards[0]!=d.cards[2],"three distinct cards");check(draft.reroll(1),"one reroll");check(!draft.reroll(0),"no second reroll");int card=draft.view().draft.cards[0];check(draft.choose(card),"choose offered");before=draft.fingerprint();check(!draft.choose(card)&&before==draft.fingerprint(),"endowment once");check(draft.invariant(),"augment invariant");}
    std::mt19937_64 rng(73);
    Realm hungry=Realm::scenario(37,129,30,50,0,100,30,30,0,50);check(hungry.execute({129,1,{Intent::Rest,hungry.person(129).region,0,2,2}}).success,"rest remains a legal intent");check(hungry.person(129).health==50&&hungry.person(129).body_state.rest_until_tick==128,"rest is pending until game-time deadline");hungry.advance(2);check(hungry.person(129).health==50&&hungry.person(129).body_state.rest_until_tick==0,"starving rest completes without health recovery");
    for(const int startingHealth:{1,2,3}){
        Realm boundary=Realm::scenario(370+uint64_t(startingHealth),133,30,50,0,100,30,30,0,startingHealth);
        RealmObservationTestAccess::prepare_starvation_boundary(boundary,133,startingHealth,0);
        RealmObservationTestAccess::run_people_phase(boundary);
        const auto& survivor=boundary.person(133);
        check(survivor.health==(startingHealth<=2?0:1),"starvation health stays at zero after lethal boundary");
        check(survivor.alive==(startingHealth>2),"starvation death threshold is unchanged at health zero");
        check(RealmObservationTestAccess::fact_count(boundary,133,100)==size_t(startingHealth<=2),"health failure retains the existing public death fact");
        check(boundary.invariant(),"starvation health boundary preserves Realm invariants");
    }
    Realm unfedMarket=Realm::scenario(375,133,30,50,0,100,30,30,0,1);
    RealmObservationTestAccess::prepare_starvation_boundary(unfedMarket,133,1,0,true);
    RealmObservationTestAccess::run_people_phase(unfedMarket);
    check(!unfedMarket.person(133).alive&&unfedMarket.person(133).health==0,
          "unfilled essential market order clamps lethal health loss to zero");
    check(unfedMarket.invariant(),"unfilled essential market order preserves Realm invariants");
    Realm negativeHealthSave=Realm::scenario(374,129,30,50,0,100,30,30,100,50);
    RealmObservationTestAccess::mutable_person(negativeHealthSave,129).health=-1;
    Realm negativeHealthLoaded(1,1,128);std::string negativeHealthError;
    check(negativeHealthLoaded.deserialize(negativeHealthSave.serialize("health-bound"),"health-bound",negativeHealthError),"legacy negative health value normalizes during staged load");
    check(negativeHealthLoaded.person(129).health==0&&negativeHealthLoaded.invariant(),"loaded health is normalized to the closed zero-to-one-hundred range");
    Realm ageWarning=Realm::scenario(376,2,30,50,0,100,30,30,100,100);
    RealmObservationTestAccess::prepare_age_threshold(ageWarning,2);
    RealmObservationTestAccess::set_reference_scenario(ageWarning,true);
    RealmObservationTestAccess::run_people_phase(ageWarning);
    check(RealmObservationTestAccess::fact_count(ageWarning,2,111)==1,"crossing age 75 creates one public warning fact");
    Realm ageWarningLoaded(1,1,128);std::string ageWarningError;
    check(ageWarningLoaded.deserialize(ageWarning.serialize("age-warning"),"age-warning",ageWarningError),"age warning save reload");
    RealmObservationTestAccess::set_reference_scenario(ageWarningLoaded,true);
    RealmObservationTestAccess::run_people_phase(ageWarningLoaded);
    check(RealmObservationTestAccess::fact_count(ageWarningLoaded,2,111)==1,"continuing a loaded world does not repeat the age-75 warning");
    Realm ageDeath=Realm::scenario(377,1,75,50,0,100,30,30,100,100);
    RealmObservationTestAccess::prepare_age_mortality(ageDeath,1);
    RealmObservationTestAccess::run_people_phase(ageDeath);
    check(!ageDeath.person(1).alive&&RealmObservationTestAccess::fact_count(ageDeath,1,112)==1&&
          RealmObservationTestAccess::fact_count(ageDeath,1,100)==0,
          "old-age mortality keeps its existing chance and records its own public fact");
    Realm fed=Realm::scenario(37,129,30,50,0,100,30,30,100,50);fed.advance(1);check(fed.person(129).body_state.sleep_debt==1,"one awake day accrues one sleep-debt day-equivalent");check(fed.execute({129,1,{Intent::Rest,fed.person(129).region,0,1,1}}).success&&fed.person(129).health==50,"fed recovery does not happen at command");fed.advance_ticks(16);check(fed.person(129).health==50,"Rest does not heal mid-interval");const auto restSave=fed.serialize("rest-replay");Realm fedRestored(1,1,128);std::string restError;check(fedRestored.deserialize(restSave,"rest-replay",restError),"mid-rest save load");check(fedRestored.person(129).body_state.rest_until_tick==128,"mid-rest interval restored");check(fed.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success&&fedRestored.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success,"pause during Rest");bool pausedRest=false,restoredPausedRest=false;try{fed.advance_ticks(64);}catch(const std::runtime_error&){pausedRest=true;}try{fedRestored.advance_ticks(64);}catch(const std::runtime_error&){restoredPausedRest=true;}check(pausedRest&&restoredPausedRest&&fed.person(129).body_state.sleep_debt==1,"pause does not advance body state");check(fed.execute(RealmClockCommand{RealmClockCommand::Operation::Resume}).success&&fedRestored.execute(RealmClockCommand{RealmClockCommand::Operation::Resume}).success,"resume during Rest");fed.advance();fedRestored.advance();check(fed.person(129).body_state.sleep_debt==0&&fedRestored.person(129).body_state.sleep_debt==0,"actual Rest offsets awake debt");check(fed.person(129).health==54&&fedRestored.person(129).health==54,"64 Rest ticks restore four health once after completion");check(fed.serialize("rest-replay")==fedRestored.serialize("rest-replay"),"Rest save/load continuation matches");
    {
        // Exact signed partial-day balance: 16 awake ticks and 48 resting
        // ticks leave -32/64 day-equivalent. Save/load must retain that
        // remainder, which offsets the following awake days without rounding
        // each partial interval independently.
        Realm partialRest=Realm::scenario(37,129,30,50,0,100,30,30,100,50);
        partialRest.advance(1);
        partialRest.advance_ticks(16);
        check(partialRest.execute({129,1,{Intent::Rest,partialRest.person(129).region,0,1,1}}).success,
              "partial-day Rest starts after sixteen awake ticks");
        partialRest.advance_ticks(48);
        check(partialRest.person(129).body_state.sleep_debt==1&&
              partialRest.person(129).body_state.sleep_balance_remainder==-32&&
              partialRest.person(129).health==53,
              "sixteen awake plus forty-eight resting ticks retain negative half-day remainder and restore three health");
        const std::string partialSave=partialRest.serialize("partial-rest-balance");
        Realm partialRestored(1,1,128);std::string partialError;
        check(partialRestored.deserialize(partialSave,"partial-rest-balance",partialError),
              partialError.c_str());
        check(partialRestored.person(129).body_state.sleep_balance_remainder==-32,
              "partial-day signed remainder survives save/load");
        partialRest.advance_ticks(64);partialRestored.advance_ticks(64);
        check(partialRest.person(129).body_state.sleep_debt==1&&
              partialRest.person(129).body_state.sleep_balance_remainder==32&&
              partialRest.fingerprint()==partialRestored.fingerprint(),
              "saved half-day rest credit offsets the next full awake day");
        partialRest.advance_ticks(64);partialRestored.advance_ticks(64);
        check(partialRest.person(129).body_state.sleep_debt==2&&
              partialRest.person(129).body_state.sleep_balance_remainder==32&&
              partialRest.fingerprint()==partialRestored.fingerprint()&&partialRest.invariant(),
              "signed remainder reaches the next whole debt day without drift");

        Realm sixteenRest=Realm::scenario(381,129,30,50,0,100,30,30,100,50);
        sixteenRest.advance_ticks(48);
        check(sixteenRest.execute({129,1,{Intent::Rest,sixteenRest.person(129).region,0,1,1}}).success,
              "sixteen-tick Rest interval starts at tick forty-eight");
        sixteenRest.advance_ticks(16);
        check(sixteenRest.person(129).health==51,"sixteen actual Rest ticks restore one health");

        Realm longRest=Realm::scenario(382,129,30,50,0,100,30,30,100,50);
        check(longRest.execute({129,1,{Intent::Rest,longRest.person(129).region,0,2,2}}).success,
              "two-day Rest interval starts");
        longRest.advance_ticks(128);
        check(longRest.person(129).health==58,"128 actual Rest ticks restore eight health");

        Realm underfedRest=Realm::scenario(383,129,30,50,0,100,30,30,39,50);
        underfedRest.advance(1);
        check(underfedRest.execute({129,1,{Intent::Rest,underfedRest.person(129).region,0,1,1}}).success,
              "underfed actor can still Rest");
        underfedRest.advance(1);
        check(underfedRest.person(129).nutrition==39&&underfedRest.person(129).health==50,
              "nutrition below forty prevents Rest health recovery");

        Realm thresholdRest=Realm::scenario(384,129,30,50,0,100,30,30,40,50);
        thresholdRest.advance(1);
        check(thresholdRest.execute({129,1,{Intent::Rest,thresholdRest.person(129).region,0,1,1}}).success,
              "nutrition threshold actor can Rest");
        thresholdRest.advance(1);
        check(thresholdRest.person(129).nutrition==40&&thresholdRest.person(129).health==54,
              "nutrition forty permits duration-based Rest health recovery");

        Realm cappedRest=Realm::scenario(385,129,30,50,0,100,30,30,100,99);
        check(cappedRest.execute({129,1,{Intent::Rest,cappedRest.person(129).region,0,1,1}}).success,
              "near-maximum health actor can Rest");
        cappedRest.advance_ticks(64);
        check(cappedRest.person(129).health==100,"Rest health recovery is capped at one hundred");

        auto patchBodyBalance=[&](const Realm& source,int debt,int remainder,const char* bundle){
            std::vector<std::string> tokens;
            {std::istringstream in(source.serialize(bundle));
             std::string token;while(in>>token)tokens.push_back(token);}
            const size_t bodyAt=std::find(tokens.begin(),tokens.end(),"BODY_3")-tokens.begin();
            check(bodyAt<tokens.size(),"sleep clamp fixture locates BODY_3 section");
            const size_t bodyCount=std::stoull(tokens.at(bodyAt+1));
            bool found=false;
            for(size_t row=0;row<bodyCount;++row){
                const size_t start=bodyAt+2+row*11;
                if(std::stoull(tokens.at(start))==129){
                    tokens[start+1]=std::to_string(debt);
                    tokens[start+3]=std::to_string(remainder);
                    found=true;break;
                }
            }
            check(found,"sleep clamp fixture locates actor body record");
            std::ostringstream out;
            for(size_t i=0;i<tokens.size();++i){if(i)out<<' ';out<<tokens[i];}
            return out.str();
        };

        Realm lowerClampBase=Realm::scenario(37,129,30,50,0,100,30,30,100,50);
        lowerClampBase.advance(1);
        check(lowerClampBase.execute({129,1,{Intent::Rest,lowerClampBase.person(129).region,0,1,1}}).success,
              "zero-debt clamp fixture starts full-day Rest");
        const std::string lowerClampSave=patchBodyBalance(lowerClampBase,0,-32,"sleep-lower-clamp");
        Realm lowerClamp(1,1,128);
        check(lowerClamp.deserialize(lowerClampSave,"sleep-lower-clamp",partialError),partialError.c_str());
        lowerClamp.advance(1);
        check(lowerClamp.person(129).body_state.sleep_debt==0&&
              lowerClamp.person(129).body_state.sleep_balance_remainder==0&&lowerClamp.invariant(),
              "Rest at zero debt clears outward negative remainder");

        Realm upperClampBase=Realm::scenario(37,129,30,50,0,100,30,30,100,50);
        upperClampBase.advance(1);
        const std::string upperClampSave=patchBodyBalance(upperClampBase,100,32,"sleep-upper-clamp");
        Realm upperClamp(1,1,128);
        check(upperClamp.deserialize(upperClampSave,"sleep-upper-clamp",partialError),partialError.c_str());
        upperClamp.advance(1);
        check(upperClamp.person(129).body_state.sleep_debt==100&&
              upperClamp.person(129).body_state.sleep_balance_remainder==0&&upperClamp.invariant(),
              "awake day at maximum debt clears outward positive remainder");
    }
    {
        std::vector<std::string> tokens;{std::istringstream in(restSave);std::string token;while(in>>token)tokens.push_back(token);}
        const size_t body=std::find(tokens.begin(),tokens.end(),"BODY_3")-tokens.begin();
        const size_t bodyCount=std::stoull(tokens.at(body+1));bool patched=false;
        for(size_t row=0;row<bodyCount;++row){const size_t start=body+2+row*11;if(std::stoull(tokens.at(start))==129){tokens[start+9]=std::to_string((fed.person(129).region+1)%(fed.regions()+1));patched=true;break;}}
        check(patched,"Rest region mismatch fixture patched");
        std::ostringstream out;for(size_t i=0;i<tokens.size();++i){if(i)out<<' ';out<<tokens[i];}
        Realm movedRest(1,1,128);check(movedRest.deserialize(out.str(),"rest-replay",restError),restError.c_str());
        movedRest.advance(1);check(movedRest.person(129).health==50&&movedRest.person(129).body_state.sleep_debt==2,
            "Rest away from its recorded region grants neither sleep credit nor health recovery");
        std::vector<std::string> deadTokens;{std::istringstream in(restSave);std::string token;while(in>>token)deadTokens.push_back(token);}
        size_t personStart=0;const size_t people=size_t(fed.count());
        for(size_t candidate=100;candidate<deadTokens.size();++candidate){size_t count=0;try{count=size_t(std::stoull(deadTokens[candidate]));}catch(...){continue;}
            if(count!=people||candidate+1+56*count>deadTokens.size())continue;bool dense=true;
            for(size_t row=0;row<count;++row)if(deadTokens[candidate+1+59*row]!=std::to_string(row+1)){dense=false;break;}
            if(dense){personStart=candidate+1;break;}}
        check(personStart!=0,"Rest death fixture locates person rows");
        deadTokens[personStart+56*128+6]="0";deadTokens[personStart+56*128+14]="0";
        std::ostringstream deadSave;for(size_t i=0;i<deadTokens.size();++i){if(i)deadSave<<' ';deadSave<<deadTokens[i];}
        Realm deadRest(1,1,128);check(deadRest.deserialize(deadSave.str(),"rest-replay",restError),restError.c_str());
        deadRest.advance(1);check(!deadRest.person(129).alive&&deadRest.person(129).health==0&&
            deadRest.person(129).body_state.rest_until_tick==0,"dead actor receives no Rest recovery and its pending interval clears");
    }
    Realm restWork(37,6,256);const uint64_t restActor=129;const int restRegion=restWork.person(restActor).region;const auto site=work_site(restWork,restRegion,restActor);check(restWork.execute({restActor,restWork.person(restActor).decisions+1,{Intent::Rest,restRegion,0,0,1}}).success,"work conflict Rest starts");const auto restBeforeWorkReject=restWork.fingerprint();auto workDuringRest=restWork.execute(ScheduleCommand{restActor,restWork.person(restActor).decisions+1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,restRegion,site});check(!workDuringRest.success&&restWork.fingerprint()==restBeforeWorkReject,"Work overlapping Rest rejects atomically");restWork.advance();auto workAfterRest=restWork.execute(ScheduleCommand{restActor,restWork.person(restActor).decisions+1,ScheduleCommand::Operation::Submit,0,64,128,0,1000,ActivityKind::Work,restRegion,site});check(workAfterRest.success&&restWork.invariant(),"Work may start after Rest interval");
    Realm salary=Realm::scenario(37,1,35,50,0,0,0,0);auto salary_money=salary.total_money();salary.advance(29);check(salary.person(1).cash==0,"salary not minted before due date");salary.advance();check(salary.person(1).cash>0&&salary.total_money()==salary_money,"funded appointed role income");
    Realm roadcap(73,6,128);for(int level=0;level<4;++level){check(roadcap.execute({1,roadcap.person(1).decisions+1,{Intent::Project,roadcap.person(1).region,0,200,60}}).success,"fund road upgrade");roadcap.advance(65);}auto capped=roadcap.fingerprint();check(!roadcap.execute({1,roadcap.person(1).decisions+1,{Intent::Project,roadcap.person(1).region,0,200,60}}).success&&roadcap.fingerprint()==capped,"no infinite road reward beyond physical level cap");
    Realm authority(37,6,10000);auto authority_before=authority.fingerprint();check(!authority.execute({8,1,{Intent::Tax,authority.person(8).region,0,20}}).success&&authority.fingerprint()==authority_before,"advisor cannot change charter tax");check(authority.observation(1)[97]==1&&authority.observation(8)[97]==0,"public authority is observed");
    Realm rounding(37,6,256);for(int i=0;i<3;++i)check(rounding.execute({1,uint64_t(i+1),{Intent::Aid,0,0,1}}).success,"small funded relief");int beneficiaries=0;for(uint64_t id=15;id<=256;++id)if(rounding.person(id).cash==101)++beneficiaries;check(beneficiaries==3,"relief rounding does not always favor same ID");
    // Preserve prior Work -> advance 30 -> Study success -> job==-1 test.
    Realm employment=Realm::scenario(37,129,30);int home=employment.person(129).region;check(employment.execute({129,1,{Intent::Work,home,0,0,30}}).success,"start employment");check(employment.job_type(129)==JobType::Worker,"worker classification after start");check(employment.employment_state(129)==EmploymentState::Employed,"worker is Employed");{const Job* jd=employment.job_descriptor(129);check(jd&&jd->is_work_job(),"worker is work-job");check(jd&&jd->type()==JobType::Worker,"worker descriptor type");check(jd&&jd->name()=="Worker","worker descriptor name");}employment.advance(30);check(employment.execute({129,2,{Intent::Study,home,0,5,30}}).success,"study after contract ends");check(employment.person(129).work_assignment.job==-1,"study cannot renew an expired wage contract");check(employment.job_type(129)==JobType::None,"none after expiry");check(employment.employment_state(129)==EmploymentState::Idle,"Idle after expiry");check(employment.job_descriptor(129)==nullptr,"expired descriptor is nullptr");
    // Pure resolver tests on constructed Person fixtures (no serializer surgery).
    {Person w;w.alive=true;w.work_assignment.job=0;w.work_assignment.job_kind=0;w.busy_until=10;auto d=describe_job(w,5);check(d&&d->type()==JobType::Worker&&d->is_work_job(),"resolver worker");
     Person t;t.alive=true;t.work_assignment.job=1;t.work_assignment.job_kind=1;t.busy_until=10;auto dt=describe_job(t,5);check(dt&&dt->type()==JobType::Teacher&&dt->is_work_job(),"resolver teacher");
     Person o;o.alive=true;o.office_appointment.office=true;o.office_appointment.hired_by=1;o.family=Family::Governance;auto dof=describe_job(o,5);check(dof&&dof->type()==JobType::AppointedOffice&&!dof->is_work_job(),"resolver appointed office");
     Person idle;idle.alive=true;idle.work_assignment.job=-1;idle.work_assignment.job_kind=0;idle.busy_until=0;check(describe_job(idle,5)==nullptr,"resolver idle");
     Person expired;expired.alive=true;expired.work_assignment.job=0;expired.work_assignment.job_kind=0;expired.busy_until=4;check(describe_job(expired,5)==nullptr,"resolver expired");
     Person dead;dead.alive=false;dead.work_assignment.job=0;dead.work_assignment.job_kind=0;dead.busy_until=10;check(describe_job(dead,5)==nullptr,"resolver deceased");
     Person ik;ik.alive=true;ik.work_assignment.job=0;ik.work_assignment.job_kind=2;ik.busy_until=10;check(describe_job(ik,5)==nullptr,"resolver invalid job_kind");
     Person io;io.alive=true;io.office_appointment.office=true;io.office_appointment.hired_by=0;io.family=Family::Governance;io.work_assignment.job=0;io.work_assignment.job_kind=0;io.busy_until=10;auto dio=describe_job(io,5);check(dio&&dio->type()==JobType::Worker,"resolver invalid office falls through to worker");}
    // Teacher classification: pure resolver fixture; job_kind must be exactly 1.
    {Person tw;tw.alive=true;tw.work_assignment.job=1;tw.work_assignment.job_kind=1;tw.busy_until=10;check(describe_job(tw,5)&&describe_job(tw,5)->type()==JobType::Teacher,"teacher fixture job_kind==1");}
    // Deterministic Appointed Office via Court fixture: find seed whose draft contains card 6.
    // Mirrors the court fixture in test_realm_court.cpp: Realm(seed,6,256), limit 2000, Appoint at candidates(1)[15].
    bool found=false;Realm office_world(1,1,128);uint64_t officer=0;int officer_region=-1;
    for(int seed=1;seed<2000&&!found;++seed){Realm tw(seed,6,256);auto d=tw.draft();if(std::find(d.cards.begin(),d.cards.end(),6)==d.cards.end())continue;check(tw.choose(6),"choose card 6");Realm rejected=tw;const int rejected_region=rejected.person(1).region;const int64_t relief=rejected.view().provinces[rejected_region].treasury-19;check(relief>0,"appointment rejection fixture can drain treasury");check(rejected.execute({1,1,{Intent::Aid,rejected_region,0,relief}}).success,"appointment rejection fixture drains treasury");const auto actor_before=rejected.person(1);const int employed_before=rejected.view().provinces[rejected_region].employed;const int teachers_before=rejected.view().provinces[rejected_region].teachers;auto reject_before=rejected.fingerprint();auto rejected_result=rejected.execute({1,rejected.person(1).decisions+1,rejected.candidates(1)[15]});check(!rejected_result.success&&rejected.fingerprint()==reject_before&&rejected.person(1).work_assignment.job==actor_before.work_assignment.job&&rejected.person(1).busy_until==actor_before.busy_until&&rejected.view().provinces[rejected_region].employed==employed_before&&rejected.view().provinces[rejected_region].teachers==teachers_before,"appointment legal rejection is atomic for actor counters and assignment");auto result=tw.execute({1,1,tw.candidates(1)[15]});check(result.success,"appoint succeeds");for(uint64_t id=1;id<=256;++id){const auto&p=tw.person(id);if(p.office_appointment.office&&p.office_appointment.hired_by!=0&&p.family==Family::Governance){check(tw.job_type(id)==JobType::AppointedOffice,"office classification");check(tw.employment_state(id)==EmploymentState::Employed,"office is Employed");{const Job* od=tw.job_descriptor(id);check(od&&!od->is_work_job(),"office is not work-job");check(od&&od->type()==JobType::AppointedOffice,"office descriptor type");}officer=id;officer_region=tw.person(1).region;office_world=tw;found=true;break;}}}
    check(found,"deterministic appointment found");
    // T2.3 shared public account: a one-unit work wage paid before day_people
    // can leave a funded-at-threshold office salary unpaid and end the office.
    {
        check(officer>0&&officer_region==office_world.person(1).region,"T2.3 salary fixture officer/employer region");
        Realm control=office_world;
        const auto historicalControl=test_fixtures::save_as_v25(control.serialize("legacy-payroll-control"));
        std::string controlMigrationError;
        const bool controlMigrated=control.deserialize(historicalControl,"legacy-payroll-control",controlMigrationError);
        check(controlMigrated,controlMigrationError.c_str());
        uint64_t teacher=0;
        for(uint64_t id=8;id<=256;++id){const auto&p=office_world.person(id);
            if(p.alive&&p.family!=Family::Governance&&p.region==officer_region&&p.age_days>=18*365&&p.skill>=20){teacher=id;break;}}
        check(teacher>0,"T2.3 salary fixture teacher candidate");
        check(office_world.execute({teacher,office_world.person(teacher).decisions+1,{Intent::Work,officer_region,0,0,30}}).success,"T2.3 salary fixture teacher assignment");
        check(legacy_work_fixture(office_world,teacher,officer_region,1856,1872),"T2.3 salary fixture historical Work");
        check(control.execute(ScheduleCommand{1,control.person(1).decisions+1,
            ScheduleCommand::Operation::Submit,0,1856,1872,0,1000,ActivityKind::Generic,-1}).success,
            "T2.3 salary fixture control timeline");
        office_world.advance(29);control.advance(29);
        // Day 30 also pays actor 1's public role top-up (20 gross, 2 tax) and
        // adds 2 tax from the private top-up, so 36 leaves exactly the 20 the
        // office salary needs in the control world.
        const int64_t relief=office_world.view().provinces[officer_region].treasury-36;
        const int64_t control_relief=control.view().provinces[officer_region].treasury-36;
        check(relief>0&&relief==control_relief,"T2.3 salary fixture treasury parity");
        check(office_world.execute({1,office_world.person(1).decisions+1,{Intent::Aid,officer_region,0,relief}}).success,
            "T2.3 salary fixture drains test treasury");
        check(control.execute({1,control.person(1).decisions+1,{Intent::Aid,officer_region,0,control_relief}}).success,
            "T2.3 salary fixture drains control treasury");
        check(office_world.view().provinces[officer_region].treasury==36&&
              control.view().provinces[officer_region].treasury==36,"T2.3 salary fixture at threshold");
        const int64_t control_cash=control.person(officer).cash;
        office_world.advance();control.advance();
        check(!office_world.person(officer).office_appointment.office&&office_world.person(officer).office_appointment.hired_by==0&&
              office_world.person(officer).family==Family::Civilian,
              "T2.3 work wage can starve shared office salary");
        check(control.person(officer).office_appointment.office&&control.person(officer).office_appointment.hired_by==1,
              "T2.3 control office remains appointed");
        check(control.person(officer).office_appointment.salary_due==60,"T2.3 control salary due advanced");
        check(control.view().provinces[officer_region].treasury==0,
              "T2.3 control salary consumed threshold");
        check(control.person(officer).cash>=control_cash+19,
              "T2.3 control salary credited");
        check(office_world.view().provinces[officer_region].treasury==19,
              "T2.3 wage consumed shared treasury before salary");
        bool ended=false;for(const auto&f:office_world.view(officer).facts)if(f.kind==110)ended=true;
        check(ended,"T2.3 starved office salary emits end event");
    }
    Realm family=Realm::scenario(37,129,8,50,0,0);const auto familyHousehold=family.household_of(129);const auto familyGuardian=family.guardian(129);check(familyGuardian>0&&family.person(familyGuardian).cash>=5,"study fixture has a living guardian with private cash");auto initial_money=family.total_money();check(family.available_funds(129)==0,"guardian private cash is not household purchasing power");check(family.execute(HouseholdFundCommand{familyGuardian,family.person(familyGuardian).decisions+1,familyHousehold,HouseholdFundOperation::Deposit,5}).success,"guardian explicitly deposits to household budget");auto funded_money=family.total_money();check(family.available_funds(129)==5,"child sees household fund as own-needs budget");check(family.execute({129,1,{Intent::Study,family.person(129).region,0,5,30}}).success,"child study spends household fund, not guardian private cash");check(family.household_fund(familyHousehold)==0&&family.person(129).cash==0&&family.total_money()==funded_money&&family.invariant(),"household tuition transfer conserves funds");check(initial_money==funded_money,"deposit does not create money");
    Realm guild=Realm::scenario(39,129,30);auto group_money=guild.total_money();check(guild.execute({129,1,{Intent::Join,guild.person(129).region,2,2,30}}).success,"join actual organization account");check(guild.person(129).organization>0&&guild.person(129).social==0,"membership does not farm social reward");check(guild.total_money()==group_money&&guild.invariant(),"organization funds conserved");
    Realm routes(37,6,256);check(routes.distance(0,1)==1,"open shortest route");auto route_before=routes.fingerprint();check(!routes.route_command(8,0,1,false).success&&routes.fingerprint()==route_before,"economic cannot close public route");check(routes.route_command(1,0,1,false).success,"route jurisdiction");check(routes.distance(0,1)==6,"route disruption forces detour");check(routes.route_command(1,0,6,false).success,"second closure");check(routes.distance(0,1)==-1,"isolated market unreachable");
    Realm childhood(5,6,256);uint64_t child=0;for(uint64_t id=129;id<=256;++id)if(childhood.person(id).age_days<14*365){child=id;break;}check(child>0,"child with unique ID");check(!childhood.mask(child)[1+childhood.person(child).region],"child labor restricted by age");auto cold=childhood.fingerprint();check(!childhood.execute({child,1,{Intent::Borrow,childhood.person(child).region,0,100,180}}).success&&childhood.fingerprint()==cold,"child loan rejected atomically");
    // M5 household membership is a dynamic record, not an ID..ID+3 window. Fixtures are
    // built by scalar surgery on the real versioned save text, never on private state.
    auto patch_save=[](const Realm& base,const std::vector<std::pair<uint64_t,int>>& fields,const std::vector<std::string>& values){
        std::vector<std::string> tokens;{std::istringstream in(base.serialize("m5fixture"));std::string token;while(in>>token)tokens.push_back(token);}
        const size_t people=base.count();const size_t begin=[&]{for(size_t j=0;j+1+59*people<=tokens.size();++j){if(tokens[j]!=std::to_string(people))continue;
            bool dense=true;for(size_t k=0;k<people;++k)if(tokens[j+1+56*k]!=std::to_string(k+1)){dense=false;break;}
            if(dense)return j+1;}throw std::runtime_error("person block not located in save");}();
        for(size_t i=0;i<fields.size();++i)tokens[begin+56*(fields[i].first-1)+size_t(fields[i].second)]=values[i];
        std::string out;for(size_t i=0;i<tokens.size();++i){if(i)out+=' ';out+=tokens[i];}return out;};
    int loadFixtureIndex=0;
    auto load_fixture=[&](const std::string& text){++loadFixtureIndex;Realm w(1,1,128);std::string err;if(!w.deserialize(text,"m5fixture",err))throw std::runtime_error("household fixture "+std::to_string(loadFixtureIndex)+" rejected: "+err);return w;};
    auto patch_tagged_record_field=[](const Realm& base,const std::string& tag,size_t field,const std::string& value){
        std::vector<std::string> tokens;{std::istringstream input(base.serialize("m5fixture"));std::string token;while(input>>token)tokens.push_back(token);}
        const auto marker=std::find(tokens.begin(),tokens.end(),tag);if(marker==tokens.end())throw std::runtime_error("missing tagged section");
        const size_t index=size_t(marker-tokens.begin())+2+field;if(index>=tokens.size())throw std::runtime_error("tagged field outside save");
        tokens[index]=value;std::ostringstream output;for(size_t i=0;i<tokens.size();++i){if(i)output<<' ';output<<tokens[i];}return output.str();
    };
    auto patch_realm_header=[](const Realm& base,size_t field,const std::string& value){
        std::vector<std::string> tokens;{std::istringstream input(base.serialize("m5fixture"));std::string token;while(input>>token)tokens.push_back(token);}
        if(field>=11||tokens.size()<=field)throw std::runtime_error("realm header field outside save");
        tokens[field]=value;std::ostringstream output;for(size_t i=0;i<tokens.size();++i){if(i)output<<' ';output<<tokens[i];}return output.str();
    };
    auto patch_loan_field=[](const Realm& base,const Loan& loan,size_t field,const std::string& value){
        const std::array<std::string,8> record{{std::to_string(loan.id),std::to_string(loan.debtor),std::to_string(loan.creditor),
            std::to_string(loan.due),std::to_string(loan.principal),std::to_string(loan.interest),std::to_string(loan.paid),
            std::to_string(loan.defaulted)}};
        if(field>=record.size())throw std::runtime_error("loan field outside record");
        std::vector<std::string> tokens;{std::istringstream input(base.serialize("m5fixture"));std::string token;while(input>>token)tokens.push_back(token);}
        size_t found=tokens.size(),matches=0;
        for(size_t i=0;i+record.size()<=tokens.size();++i)if(std::equal(record.begin(),record.end(),tokens.begin()+std::ptrdiff_t(i))){found=i;++matches;}
        if(matches!=1)throw std::runtime_error("loan record not uniquely located");
        tokens[found+field]=value;std::ostringstream output;for(size_t i=0;i<tokens.size();++i){if(i)output<<' ';output<<tokens[i];}return output.str();
    };
    auto patch_fact_field=[](const Realm& base,const Fact& fact,size_t field,const std::string& value){
        const std::array<std::string,9> record{{std::to_string(fact.id),std::to_string(fact.parent),std::to_string(fact.actor),
            std::to_string(fact.target),std::to_string(fact.day),std::to_string(fact.region),std::to_string(fact.kind),
            std::to_string(fact.amount),std::to_string(fact.secret)}};
        if(field>=record.size())throw std::runtime_error("fact field outside record");
        std::vector<std::string> tokens;{std::istringstream input(base.serialize("m5fixture"));std::string token;while(input>>token)tokens.push_back(token);}
        size_t found=tokens.size(),matches=0;
        for(size_t i=0;i+record.size()<=tokens.size();++i)if(std::equal(record.begin(),record.end(),tokens.begin()+std::ptrdiff_t(i))){found=i;++matches;}
        if(matches!=1)throw std::runtime_error("fact record not uniquely located");
        tokens[found+field]=value;std::ostringstream output;for(size_t i=0;i<tokens.size();++i){if(i)output<<' ';output<<tokens[i];}return output.str();
    };
    auto rejects_save_atomically=[&](Realm& destination,const std::string& text,const char* label){
        const auto before=destination.fingerprint();std::string error;
        const bool loaded=destination.deserialize(text,"m5fixture",error);
        check(!loaded&&destination.fingerprint()==before,label);
    };
    {
        Realm commandBase(37,6,256);
        auto deadActor=load_fixture(patch_save(commandBase,{{129,14}},{"0"}));
        const auto deadBefore=deadActor.fingerprint();
        const auto deadHold=deadActor.execute({129,deadActor.person(129).decisions+1,{Intent::Hold}});
        check(!deadHold.success&&deadHold.reason=="Actor deceased"&&deadActor.fingerprint()==deadBefore,
              "RealmCommand Hold rejects deceased actor atomically");

        const std::string maximum=std::to_string(std::numeric_limits<uint64_t>::max());
        auto decisionOverflow=load_fixture(patch_save(commandBase,{{65,2}},{maximum}));
        const auto decisionBefore=decisionOverflow.fingerprint();
        const auto wrapped=decisionOverflow.execute({65,0,{Intent::Hold}});
        check(!wrapped.success&&wrapped.reason=="Decision counter overflow"&&
              decisionOverflow.fingerprint()==decisionBefore,
              "RealmCommand sequence cannot wrap at maximum decision counter");

        auto samplingOverflow=load_fixture(patch_save(commandBase,{{65,3}},{maximum}));
        const auto samplingBefore=samplingOverflow.fingerprint();
        const auto noSampling= samplingOverflow.execute({65,samplingOverflow.person(65).decisions+1,{Intent::Hold}});
        check(!noSampling.success&&noSampling.reason=="Decision counter overflow"&&
              samplingOverflow.fingerprint()==samplingBefore,
              "RealmCommand rejects maximum sampling counter atomically");

        std::vector<std::string> factTokens;
        {std::istringstream input(commandBase.serialize("m5fixture"));std::string token;while(input>>token)factTokens.push_back(token);}
        factTokens[6]=maximum;
        std::ostringstream factSave;for(size_t i=0;i<factTokens.size();++i){if(i)factSave<<' ';factSave<<factTokens[i];}
        auto factOverflow=load_fixture(factSave.str());
        const auto factBefore=factOverflow.fingerprint();
        const auto noFact=factOverflow.execute({65,factOverflow.person(65).decisions+1,{Intent::Hold}});
        check(!noFact.success&&noFact.reason=="Fact counter overflow"&&factOverflow.fingerprint()==factBefore,
              "RealmCommand rejects maximum fact counter atomically");
    }
    {
        const std::string maximum=std::to_string(std::numeric_limits<uint64_t>::max());
        Realm base(37,6,256),destination=base;
        rejects_save_atomically(destination,patch_realm_header(base,5,"0"),"zero shared ID allocator rejected atomically");
        rejects_save_atomically(destination,patch_realm_header(base,6,"0"),"zero fact allocator rejected atomically");

        Realm factBase(37,6,256);const int chosen=factBase.draft().cards[0];
        check(factBase.choose(chosen)&&factBase.route_command(1,0,1,false).success,
              "fact allocator uniqueness fixture created");
        auto observedFacts=factBase.view(1).facts;
        const auto firstFact=std::find_if(observedFacts.begin(),observedFacts.end(),[](const Fact& fact){return fact.kind==106;});
        const auto secondFact=std::find_if(observedFacts.begin(),observedFacts.end(),[](const Fact& fact){return fact.kind==107;});
        check(firstFact!=observedFacts.end()&&secondFact!=observedFacts.end(),"fact records visible to fixture actor");
        Realm factDestination=factBase;
        rejects_save_atomically(factDestination,patch_fact_field(factBase,*secondFact,0,std::to_string(firstFact->id)),
                                "duplicate fact ID rejected atomically");
        rejects_save_atomically(factDestination,patch_realm_header(factBase,6,std::to_string(secondFact->id)),
                                "fact allocator not beyond existing records rejected atomically");

        const std::string half=std::to_string(std::numeric_limits<int64_t>::max()/2);
        rejects_save_atomically(destination,patch_save(base,{{129,16},{130,16}},{half,half}),
                                "money account sum overflow rejected atomically");
        auto expectedOverflow=patch_realm_header(base,7,maximum);
        {std::istringstream input(expectedOverflow);std::vector<std::string> tokens;std::string token;while(input>>token)tokens.push_back(token);
         tokens[10]="1";std::ostringstream output;for(size_t i=0;i<tokens.size();++i){if(i)output<<' ';output<<tokens[i];}expectedOverflow=output.str();}
        rejects_save_atomically(destination,expectedOverflow,"opening money plus endowment overflow rejected atomically");

        const int region=base.person(1).region;
        auto site=base.execute(EstablishmentCommand{1,base.person(1).decisions+1,EstablishmentOwnerKind::Public,
            uint64_t(region),region,EstablishmentIndustry::ClothWorkshop,1,1});
        check(site.success,"ID invariant fixture establishment created");
        Realm siteDestination=base;
        rejects_save_atomically(siteDestination,patch_tagged_record_field(base,"ESTABLISH_14",0,"0"),
                                "zero establishment ID rejected atomically");
        rejects_save_atomically(siteDestination,patch_realm_header(base,5,"1"),
                                "allocator behind an allocated record ID rejected atomically");
        Realm personNamespace=siteDestination;std::string personNamespaceError;
        check(personNamespace.deserialize(patch_tagged_record_field(base,"ESTABLISH_14",0,"1"),"m5fixture",personNamespaceError)&&
              personNamespace.establishments().size()==1&&personNamespace.establishments()[0].id==1,
              "Person and Establishment numeric ID namespaces may overlap");

        Realm loanBase(37,6,10000);const uint64_t borrower=65;const int loanRegion=loanBase.person(borrower).region;
        const auto borrowed=loanBase.execute({borrower,loanBase.person(borrower).decisions+1,
            {Intent::Borrow,loanRegion,0,100,180}});
        check(borrowed.success&&loanBase.loans().size()==1,"loan overflow fixture created");
        Realm loanDestination=loanBase;
        rejects_save_atomically(loanDestination,patch_loan_field(loanBase,loanBase.loans()[0],5,maximum),
                                "loan principal plus interest overflow rejected atomically");

        Realm largeRoyalLoan(37,6,128);
        RealmObservationTestAccess::prepare_large_royal_loan(largeRoyalLoan);
        const auto principal=int64_t(1000000);
        const auto expectedInterest=(principal/100)*15+((principal%100)*15+99)/100;
        const auto largeBorrow=largeRoyalLoan.execute({1,largeRoyalLoan.person(1).decisions+1,
            {Intent::Borrow,largeRoyalLoan.person(1).region,1,principal,1825}});
        check(largeBorrow.success&&largeRoyalLoan.loans().size()==1&&
              largeRoyalLoan.loans()[0].interest==expectedInterest&&
              largeRoyalLoan.debt(1)==principal+expectedInterest,
              "maximum valid royal loan preserves ceiling interest arithmetic");
        const auto beforeRejectedRepeat=largeRoyalLoan.fingerprint();
        const auto repeatedBorrow=largeRoyalLoan.execute({1,largeRoyalLoan.person(1).decisions+1,
            {Intent::Borrow,largeRoyalLoan.person(1).region,1,1,1825}});
        check(!repeatedBorrow.success&&largeRoyalLoan.fingerprint()==beforeRejectedRepeat,
              "interest-inclusive royal borrowing limit rejects atomically");
        check(largeRoyalLoan.invariant(),"maximum valid royal loan preserves Realm accounting");
        largeRoyalLoan.advance();
        const auto loanAfterDay=largeRoyalLoan.fingerprint();
        std::string loanSaveError;
        const auto loanSave=largeRoyalLoan.serialize("large-royal-loan");
        Realm reloadedLargeLoan(37,6,128);
        check(reloadedLargeLoan.deserialize(loanSave,"large-royal-loan",loanSaveError)&&
              reloadedLargeLoan.fingerprint()==loanAfterDay&&reloadedLargeLoan.debt(1)==principal+expectedInterest,
              "large valid royal loan survives one day and save/load");
    }
    // A rejected command consumes no sequence, so every rejection below must be issued with
    // the actor's CURRENT decision count to reach household/authority validation at all.
    auto rejects=[&](Realm& world,uint64_t actor,const Candidate& action,const char* label){auto before=world.fingerprint();auto sequence=world.person(actor).decisions+1;auto result=world.execute({actor,sequence,action});check(!result.success&&world.fingerprint()==before&&result.reason!="Stale or duplicate decision ID",label);};
    // T2.3 pays completed physical work per actor/region at a 16-tick unit floor.
    auto submit_work=[&](Realm& world,uint64_t actor,int region,int64_t begin,int64_t end){
        return legacy_work_fixture(world,actor,region,begin,end);
    };
    auto regional_paid=[&](const Realm& world,uint64_t actor,int region){
        const auto&paid=world.schedule_book().paid_work_units_by_region;
        auto found=paid.find(WorkRegionKey{actor,region});
        return found==paid.end()?int64_t(0):found->second;
    };
    auto regional_ticks=[&](const Realm& world,uint64_t actor,int region)->int64_t{
        const auto&ticks=world.schedule_book().work_present_ticks_by_region;
        auto found=ticks.find(WorkRegionKey{actor,region});
        return found==ticks.end()?-1:found->second;
    };
    auto work_once=[&](int64_t end)->std::pair<int64_t,int64_t>{
        Realm w(37,6,256);
        const uint64_t actor=129;
        const int region=w.person(actor).region;
        const int64_t cash=w.person(actor).cash;
        check(w.person(actor).work_assignment.job_kind==0&&w.person(actor).family==Family::Civilian,"T2.3 private civilian fixture");
        check(submit_work(w,actor,region,0,end),"T2.3 threshold submit");
        w.advance_ticks(64);
        return {w.person(actor).cash-cash,regional_paid(w,actor,region)};
    };
    {
        auto r15=work_once(15),r16=work_once(16),r31=work_once(31),r32=work_once(32);
        check(r15.first==0&&r15.second==0,"T2.3 15 ticks pays no unit");
        check(r16.first==1&&r16.second==1,"T2.3 16 ticks pays one unit");
        check(r31.first==1&&r31.second==1,"T2.3 31 ticks pays one unit");
        check(r32.first==2&&r32.second==2,"T2.3 32 ticks pays two units");
    }
    {
        Realm replacement(37,6,256);const uint64_t actor=129;const int region=replacement.person(actor).region;
        check(replacement.execute({actor,1,{Intent::Work,region,0,0,30}}).success,"T2.3 replacement assignment");
        check(replacement.execute(ScheduleCommand{1,replacement.person(1).decisions+1,
            ScheduleCommand::Operation::Submit,0,0,1,0,1000,ActivityKind::Generic,-1}).success,
            "T2.3 replacement timeline");
        replacement.advance(30);
        check(replacement.schedule_book().work_present_ticks.empty()&&
              replacement.schedule_book().work_present_ticks_by_region.empty()&&
              regional_paid(replacement,actor,region)==0,
              "T2.3 active assignment without typed work pays no wage");
    }
    auto split_work=[&](int64_t first,int64_t second)->std::pair<int64_t,int64_t>{
        Realm w(37,6,256);
        const uint64_t actor=129;
        const int a=w.person(actor).region,b=(a+1)%6;
        const int64_t cash=w.person(actor).cash;
        check(submit_work(w,actor,a,0,first),"T2.3 split first submit");
        w.advance_ticks(first);
        Realm moved=load_fixture(patch_save(w,{{actor,4}},{std::to_string(b)}));
        check(submit_work(moved,actor,b,30,30+second),"T2.3 split second submit");
        moved.advance_ticks(64-first);
        check(regional_ticks(moved,actor,a)==first,"T2.3 split first ticks");
        check(regional_ticks(moved,actor,b)==second,"T2.3 split second ticks");
        return {moved.person(actor).cash-cash,regional_paid(moved,actor,a)+regional_paid(moved,actor,b)};
    };
    {
        auto s8=split_work(8,8),s15=split_work(15,15),s16=split_work(16,16);
        check(s8.first==0&&s8.second==0,"T2.3 per-region 8+8 floors to zero");
        check(s15.first==0&&s15.second==0,"T2.3 per-region 15+15 floors to zero");
        check(s16.first==2&&s16.second==2,"T2.3 per-region 16+16 pays each region once");
    }
    {
        Realm delay(37,6,256);const uint64_t actor=129;const int region=delay.person(actor).region;
        const int64_t cash=delay.person(actor).cash;
        check(submit_work(delay,actor,region,0,64),"T2.3 delay submit");
        delay.advance_ticks(63);
        check(delay.schedule_book().paid_work_units_by_region.empty()&&delay.person(actor).cash==cash,"T2.3 no intraday wage");
        delay.advance_ticks(1);
        check(regional_paid(delay,actor,region)==4&&delay.person(actor).cash==cash+4,"T2.3 boundary payroll pays completed day");
    }
    {
        Realm dead(37,6,256);const uint64_t actor=129;const int region=dead.person(actor).region;
        check(submit_work(dead,actor,region,0,30),"T2.3 death submit");
        dead.advance_ticks(64);
        check(regional_paid(dead,actor,region)==1&&regional_ticks(dead,actor,region)==30,"T2.3 death fixture baseline");
        Realm frozen=load_fixture(patch_save(dead,{{actor,14}},{"0"}));
        frozen.advance_ticks(64);
        check(!frozen.person(actor).alive,"T2.3 death fixture alive flag");
        check(regional_paid(frozen,actor,region)==1,"T2.3 death freezes unpaid watermark");
        check(regional_ticks(frozen,actor,region)==30,"T2.3 death leaves attendance frozen");
        check(regional_paid(frozen,actor,region)<=30/16,"T2.3 death never overpays eligible units");
    }
    {
        Realm topup(37,6,256);const int region=topup.person(1).region;
        check(!topup.person(1).office_appointment.hired_by&&topup.person(1).family==Family::Governance,"T2.3 governance top-up fixture");
        uint64_t economic=0;
        for(uint64_t id=1;id<=uint64_t(topup.count())&&!economic;++id){
            const auto&p=topup.person(id);
            if(!p.office_appointment.hired_by&&p.family==Family::Economic)economic=id;
        }
        check(economic>0,"T2.3 economic top-up fixture");
        check(submit_work(topup,1,region,1856,1872),"T2.3 top-up work submit");
        topup.advance_ticks(1856);
        const int64_t cash1=topup.person(1).cash,cash3=topup.person(economic).cash;
        topup.advance_ticks(64);
        check(regional_paid(topup,1,region)==1,"T2.3 role top-up does not advance work watermark");
        check(topup.person(1).cash-cash1==19,"T2.3 same-actor work wage precedes 10% taxed top-up");
        check(topup.person(economic).cash-cash3==18,"T2.3 economic role top-up pays public net");
        check(regional_paid(topup,economic,topup.person(economic).region)==0,"T2.3 role-only actor has no wage watermark");
        check(regional_ticks(topup,1,region)==16,"T2.3 top-up attendance exact");
    }
    {
        Realm dup(37,6,256);const uint64_t actor=129;const int a=dup.person(actor).region,b=(a+1)%6;
        check(submit_work(dup,actor,a,0,16),"T2.3 duplicate-key first submit");
        dup.advance_ticks(16);
        Realm moved=load_fixture(patch_save(dup,{{actor,4}},{std::to_string(b)}));
        check(submit_work(moved,actor,b,30,46),"T2.3 duplicate-key second submit");
        moved.advance_ticks(47);
        std::string ddata=moved.serialize("test");Realm loaded(2,1,128);std::string derror;
        check(loaded.deserialize(ddata,"test",derror),"T2.3 duplicate actor/region roundtrip");
        check(loaded.fingerprint()==moved.fingerprint(),"T2.3 duplicate actor/region fingerprint");
        check(regional_ticks(loaded,actor,a)==16&&regional_ticks(loaded,actor,b)==16,"T2.3 duplicate actor/region map restored");
        loaded.advance_ticks(1);
        check(regional_paid(loaded,actor,a)==1&&regional_paid(loaded,actor,b)==1,"T2.3 duplicate actor/region pays independently");
        check(loaded.invariant(),"T2.3 duplicate actor/region invariant");
    }
    auto find_startable=[&](Realm& world,uint64_t from,bool teacher_only){
        const int region=world.person(1).region;
        const Province province=world.view().provinces[region];
        if(province.employed>=province.jobs)return uint64_t(0);
        for(uint64_t id=from;id<=uint64_t(world.count());++id){
            const auto& p=world.person(id);
            if(!p.alive||p.work_assignment.job>=0||p.family==Family::Governance||p.region!=region||p.age_days<14*365)continue;
            if(teacher_only&&(p.age_days<18*365||p.skill<20))continue;
            return id;
        }
        return uint64_t(0);
    };
    // Work starts classify the same way as the production threshold, rebuild
    // counters daily, enforce capacity atomically, renew, expire inclusively and
    // survive save/load exactly.
    Realm work=Realm::scenario(37,129,30,50,0,0,0,1);
    const int work_region=work.person(129).region;
    uint64_t worker=0;
    for(uint64_t id=129;id<=uint64_t(work.count())&&!worker;++id){const auto&p=work.person(id);if(p.alive&&p.work_assignment.job<0&&p.family!=Family::Governance&&p.region==work_region&&p.age_days>=14*365)worker=id;}
    check(worker>0,"startable worker in a one-job region");
    auto work_before=work.fingerprint();
    check(!work.execute({worker,work.person(worker).decisions+1,{Intent::Work,work_region,0,0,31}}).success&&work.fingerprint()==work_before,"work term 31 rejected atomically");
    check(work.execute({worker,work.person(worker).decisions+1,{Intent::Work,work_region,0,0,30}}).success,"work starts a worker");
    check(work.person(worker).work_assignment.job==work_region&&work.person(worker).work_assignment.job_kind==0&&work.person(worker).busy_until==30,"worker assignment fields");
    check(work.job_type(worker)==JobType::Worker&&work.employment_state(worker)==EmploymentState::Employed,"worker classification");
    check(work.view().provinces[work_region].employed==1&&work.view().provinces[work_region].teachers==0,"worker start counters");
    uint64_t blocked=0;
    for(uint64_t id=15;id<=uint64_t(work.count())&&!blocked;++id){const auto&p=work.person(id);if(p.alive&&p.work_assignment.job<0&&p.family!=Family::Governance&&p.region==work_region&&p.age_days>=14*365)blocked=id;}
    check(blocked>0,"second eligible worker exists");
    auto full_before=work.fingerprint();
    check(!work.execute({blocked,work.person(blocked).decisions+1,{Intent::Work,work_region,0,0,30}}).success&&work.fingerprint()==full_before,"full worker capacity rejects atomically");
    work.advance(30);
    check(work.person(worker).work_assignment.job==work_region&&work.view().provinces[work_region].employed==1,"daily rebuild restores worker counters at the contract boundary");
    check(work.execute({worker,work.person(worker).decisions+1,{Intent::Work,work_region,0,0,30}}).success,"worker renews at full capacity after the contract day");
    check(work.person(worker).busy_until==60&&work.view().provinces[work_region].employed==1,"renewal keeps one accounted worker");
    work.advance(1);
    check(work.person(worker).work_assignment.job==work_region&&work.view().provinces[work_region].employed==1,"daily rebuild restores renewed worker counters");
    std::string work_data=work.serialize("employment"),work_error;
    Realm work_loaded(1,1,128);
    check(work_loaded.deserialize(work_data,"employment",work_error),"employment save parses");
    check(work_loaded.fingerprint()==work.fingerprint(),"employment save is exact");
    work.advance(30);
    work_loaded.advance(30);
    check(work.person(worker).work_assignment.job==-1&&work.job_type(worker)==JobType::None&&work.employment_state(worker)==EmploymentState::Idle,"work expiry clears the assignment");
    check(work.view().provinces[work_region].employed==0&&work.view().provinces[work_region].teachers==0,"expiry rebuilds zero counters");
    check(work.fingerprint()==work_loaded.fingerprint(),"employment continuation is exact");
    Realm underage(5,6,256);
    uint64_t underage_child=0;
    for(uint64_t id=129;id<=uint64_t(underage.count());++id)if(underage.person(id).age_days<14*365){underage_child=id;break;}
    check(underage_child>0,"underage worker fixture");
    auto child_before=underage.fingerprint();
    check(!underage.execute({underage_child,underage.person(underage_child).decisions+1,{Intent::Work,underage.person(underage_child).region,0,0,30}}).success&&underage.fingerprint()==child_before,"underage work rejected atomically");
    Realm teacher_world(37,6,256);
    const int teacher_region=teacher_world.person(1).region;
    uint64_t teacher=find_startable(teacher_world,15,true);
    check(teacher>0,"startable teacher candidate");
    check(teacher_world.execute({teacher,teacher_world.person(teacher).decisions+1,{Intent::Work,teacher_region,0,0,30}}).success,"work starts a teacher");
    check(teacher_world.person(teacher).work_assignment.job_kind==1&&teacher_world.job_type(teacher)==JobType::Teacher,"teacher classification after start");
    check(teacher_world.view().provinces[teacher_region].teachers==1,"teacher counter after start");
    teacher_world.advance(1);
    check(teacher_world.view().provinces[teacher_region].teachers==1&&teacher_world.view().provinces[teacher_region].teacher_skill_sum==teacher_world.person(teacher).skill,"teacher counters rebuild daily");
    Realm sparse_base(37,6,256);auto sparse_base_before=sparse_base.fingerprint();
    // A birth joins the parent household with a fresh non-consecutive ID. Governance and
    // economic founders are singleton households, so pick a newborn that actually shares a
    // multi-member household with a living adult.
    Realm births(37,6,10000,false,GameplayConfig::historical());const int opening_population=births.count();
    for(int day=0;day<4000&&births.count()==opening_population;++day)births.advance();
    check(births.count()>opening_population,"birth extends a household with a fresh ID");
    uint64_t newborn=0,parent=0;
    for(uint64_t id=opening_population+1;id<=uint64_t(births.count())&&!newborn;++id){
        if(births.guardian(id)==0)continue;
        for(auto&e:births.relations())if(e.from==id&&e.kind==RelationKind::ParentChild){parent=e.to;check(e.legal_parentage,"new birth edge has legal parentage provenance");break;}
        if(parent&&births.same_household(id,parent)&&births.household_of(id)==births.household_of(parent))newborn=id;}
    check(newborn>0&&parent>0,"newborn in a shared household with a living adult");
    check(std::all_of(births.person(newborn).personality.values.begin(),births.person(newborn).personality.values.end(),[](int value){return value>=25&&value<=75;}),
          "newborn receives an in-range personality profile derived from the world seed and new person ID");
    check(RealmObservationTestAccess::lineal_kinship(births,newborn,parent),
          "birth incrementally adds the new parent-child relation to the marriage index");
    auto members=births.household_members(births.household_of(newborn));
    check(members.size()>1&&std::find(members.begin(),members.end(),newborn)!=members.end()&&std::is_sorted(members.begin(),members.end()),"birth membership is real, not an ID window");
    check(members.back()-members.front()>3&&births.guardian(newborn)!=0&&births.same_household(newborn,births.guardian(newborn)),"non-consecutive member is reachable as guardian");
    std::string household_data=births.serialize("m5"),household_error;Realm reloaded(3,2,128);
    check(reloaded.deserialize(household_data,"m5",household_error),"household birth save loads");
    check(reloaded.fingerprint()==births.fingerprint()&&reloaded.household_members(births.household_of(newborn))==members,"household membership survives load as derived state");
    const auto reloaded_relations=reloaded.relations();
    check(std::any_of(reloaded_relations.begin(),reloaded_relations.end(),[&](const Edge& e){return e.from==newborn&&e.to==parent&&e.legal_parentage;}),"schema27 save preserves legal parentage marker");
    check(RealmObservationTestAccess::lineal_kinship(reloaded,newborn,parent),
          "save/load rebuilds the marriage index for legal parentage");
    check(reloaded.guardian(newborn)==births.guardian(newborn)&&reloaded.invariant()&&reloaded.household_records()>0,"guardian and invariant survive load");
    std::string schema24_birth=test_fixtures::save_as_v25(household_data);const auto parentageAt=schema24_birth.find(" PARENTAGE_1 ");
    check(parentageAt!=std::string::npos,"parentage section fixture present");schema24_birth.erase(parentageAt);schema24_birth.replace(0,15,"POLYEC_REALM_24");
    Realm migratedBirth(1,1,128);check(migratedBirth.deserialize(schema24_birth,"m5",household_error),household_error.c_str());
    const auto migrated_relations=migratedBirth.relations();
    check(std::any_of(migrated_relations.begin(),migrated_relations.end(),[&](const Edge& e){return e.from==newborn&&e.to==parent&&e.legal_parentage;}),"schema24 migration restores parentage only from matching birth fact");
    std::vector<std::string> corrupt_parentage_tokens;{std::istringstream in(household_data);std::string token;while(in>>token)corrupt_parentage_tokens.push_back(token);}
    const auto parentage_tag=std::find(corrupt_parentage_tokens.begin(),corrupt_parentage_tokens.end(),"PARENTAGE_1");
    check(parentage_tag!=corrupt_parentage_tokens.end()&&std::next(parentage_tag,2)!=corrupt_parentage_tokens.end(),"parentage section has an index to validate");
    *std::next(parentage_tag,2)="0";
    std::ostringstream corrupt_parentage_save;for(size_t i=0;i<corrupt_parentage_tokens.size();++i){if(i)corrupt_parentage_save<<' ';corrupt_parentage_save<<corrupt_parentage_tokens[i];}
    const auto before_corrupt_parentage=migratedBirth.fingerprint();
    check(!migratedBirth.deserialize(corrupt_parentage_save.str(),"m5",household_error)&&migratedBirth.fingerprint()==before_corrupt_parentage,
          "schema27 rejects invalid parentage index without replacing destination");
    // Non-consecutive living adult, and every in-window adult dead: the old ID..ID+3 scan
    // could never find a guardian here.
    Realm sparse_world=load_fixture(patch_save(sparse_base,{{52,1},{15,14},{17,14},{18,14}},{"15","0","0","0"}));
    check(sparse_world.invariant()&&sparse_world.count()==256,"sparse household fixture invariant");
    auto sparse_members=sparse_world.household_members(15);
    check(sparse_members.size()==5&&std::find(sparse_members.begin(),sparse_members.end(),uint64_t(52))!=sparse_members.end(),"non-consecutive member recorded in the household");
    check(52>18&&sparse_world.guardian(16)==52&&sparse_world.guardian(52)==0,"guardian found outside any ID..ID+3 window");
    check(sparse_world.household_authority(15)==52&&sparse_world.available_funds(16)==sparse_world.person(16).cash,"guardian private cash is not included in household budget");
    check(sparse_world.same_household(16,52)&&!sparse_world.same_household(16,60)&&!sparse_world.same_household(0,16),"same household authority is membership, not proximity");
    // Same-household authority: a living adult member receives the household transfer. The
    // expected authority is derived from the fixture, never assumed from an age guess.
    Realm authority_world=load_fixture(patch_save(sparse_base,{{52,1},{16,5},{15,14},{17,14},{18,14},{52,5}},{"15",std::to_string(30*365),"0","0","0",std::to_string(40*365)}));
    uint64_t oldest=0;for(uint64_t member:authority_world.household_members(15)){auto&p=authority_world.person(member);if(p.alive&&p.age_days>=18*365&&(oldest==0||p.age_days>authority_world.person(oldest).age_days))oldest=member;}
    check(oldest==52&&authority_world.household_authority(15)==oldest&&authority_world.candidates(16)[19].target==int(oldest)&&authority_world.mask(16)[19]==1.f,"adult household member offers support to the oldest living adult");
    int64_t household_money=authority_world.total_money();int64_t household_before=authority_world.person(52).cash,sender_before=authority_world.person(16).cash;
    check(authority_world.execute({16,1,{Intent::Support,int(oldest),0,5}}).success,"same household support commits");
    check(authority_world.total_money()==household_money&&authority_world.invariant(),"household support conserves money");
    auto support_before=authority_world.fingerprint();
    rejects(authority_world,16,{Intent::Support,60,0,5},"foreign household target rejected atomically");
    rejects(authority_world,16,{Intent::Support,16,0,5},"self target rejected atomically");
    rejects(authority_world,16,{Intent::Support,15,0,5},"deceased household member rejected atomically");
    rejects(authority_world,16,{Intent::Support,0,0,5},"zero target rejected before any person lookup");
    check(authority_world.person(oldest).cash==household_before+5&&authority_world.person(16).cash==sender_before-5,"only the intended household recipient gained funds");
    // Dead/missing guardian: no living adult in the household must not throw or mint money.
    Realm orphan_world=load_fixture(patch_save(sparse_base,{{15,14},{17,14},{18,14}},{"0","0","0"}));
    check(orphan_world.invariant()&&orphan_world.guardian(16)==0&&orphan_world.household_authority(15)==0,"household without a living adult has no authority");
    check(orphan_world.available_funds(16)==orphan_world.person(16).cash,"orphan budget is own cash only");
    auto orphan_before=orphan_world.fingerprint();orphan_world.mask(16);check(orphan_world.fingerprint()==orphan_before,"orphan household observation is safe");
    check(orphan_world.candidates(16)[19].kind==Intent::Hold,"orphan household offers no support target");
    rejects(orphan_world,16,{Intent::Support,0,0,5},"orphan support rejected without a person lookup");
    rejects(orphan_world,16,{Intent::Support,15,0,5},"dead authority rejected atomically");
    int64_t orphan_money=orphan_world.total_money();
    rejects(orphan_world,16,{Intent::Study,orphan_world.person(16).region,0,1000000,30},"unfunded orphan study rejected without a guardian transfer");
    check(orphan_world.execute({16,orphan_world.person(16).decisions+1,{Intent::Study,orphan_world.person(16).region,0,5,30}}).success,"orphan child studies from own cash");
    check(orphan_world.total_money()==orphan_money&&orphan_world.invariant(),"orphan study conserves money");
    // A stale sequence is rejected before gameplay rules, and must not be read as authority.
    auto stale_before=orphan_world.fingerprint();
    check(!orphan_world.execute({16,99,{Intent::Study,orphan_world.person(16).region,0,5,30}}).success&&orphan_world.fingerprint()==stale_before,"stale sequence rejected atomically");
    // Corrupt household records are rejected and leave the live world untouched.
    Realm corrupt(1,1,128);auto corrupt_before=corrupt.fingerprint();std::string corrupt_error;
    check(!corrupt.deserialize(patch_save(sparse_base,{{20,1}},{"0"}),"m5fixture",corrupt_error)&&corrupt.fingerprint()==corrupt_before,"zero household label rejected atomically");
    check(!corrupt.deserialize(patch_save(sparse_base,{{20,1}},{std::to_string(100000)}),"m5fixture",corrupt_error)&&corrupt.fingerprint()==corrupt_before,"household label beyond the person range rejected atomically");
    // The fixture source is the pre-serialization original; the loaded fixtures are separate worlds.
    check(sparse_base.invariant()&&sparse_base.count()==256&&sparse_base.fingerprint()==sparse_base_before,"fixture source world unchanged by fixture surgery");
    check(sparse_world.fingerprint()!=sparse_base_before&&orphan_world.fingerprint()!=sparse_base_before,"patched fixtures differ from their source world");

    for(int seed=1;seed<=8;++seed){Realm sim(seed,6,256);for(int week=0;week<1044;++week){for(auto family:{Family::Governance,Family::Economic,Family::Civilian})for(auto id:sim.due(family)){auto mask=sim.mask(id);std::vector<int> available;for(int i=0;i<realm_actions;++i)if(mask[i])available.push_back(i);int a=available[rng()%available.size()];check(sim.execute({id,sim.person(id).decisions+1,sim.candidates(id)[a]}).success,"legal random action committed");}sim.advance(7);check(sim.invariant(),"20year world accounting");}}
    auto patch_payroll_save=[&](const Realm& source,int region,int64_t new_firm,uint64_t donor,uint64_t dead_actor){
        std::istringstream input(source.serialize("m5fixture"));
        std::vector<std::string> tokens;std::string token;
        while(input>>token)tokens.push_back(token);
        const size_t people=static_cast<size_t>(source.count());
        size_t person_start=tokens.size();
        for(size_t i=0;i+1+59*people<=tokens.size();++i){
            if(tokens[i]!=std::to_string(people))continue;
            bool dense=true;
            for(size_t k=0;k<people;++k)if(tokens[i+1+56*k]!=std::to_string(k+1)){dense=false;break;}
            if(dense){person_start=i+1;break;}
        }
        check(person_start<tokens.size(),"T2.3 patch people block found");
        const size_t province_count_at=person_start+59*people;
        check(province_count_at<tokens.size()&&tokens[province_count_at]==std::to_string(source.view().provinces.size()),"T2.3 patch province count");
        const size_t province_start=province_count_at+1;
        check(region>=0&&static_cast<size_t>(region)<source.view().provinces.size(),"T2.3 patch region bound");
        const size_t firm_index=province_start+66*static_cast<size_t>(region)+1;
        check(firm_index<tokens.size(),"T2.3 patch firm index");
        check(donor>0&&donor<=people,"T2.3 patch donor bound");
        const size_t donor_cash_index=person_start+56*static_cast<size_t>(donor-1)+16;
        check(donor_cash_index<tokens.size(),"T2.3 patch donor cash index");
        const int64_t old_firm=std::stoll(tokens[firm_index]);
        const int64_t adjusted_cash=std::stoll(tokens[donor_cash_index])+old_firm-new_firm;
        check(new_firm>=0&&adjusted_cash>=0,"T2.3 patch keeps accounts nonnegative");
        tokens[firm_index]=std::to_string(new_firm);
        tokens[donor_cash_index]=std::to_string(adjusted_cash);
        if(dead_actor){
            check(dead_actor<=people,"T2.3 patch dead actor bound");
            const size_t alive_index=person_start+56*static_cast<size_t>(dead_actor-1)+14;
            check(alive_index<tokens.size(),"T2.3 patch alive index");
            tokens[alive_index]="0";
        }
        std::string output;
        for(size_t i=0;i<tokens.size();++i){if(i)output+=' ';output+=tokens[i];}
        return output;
    };
    // One cash unit funds only half of the 32-tick attendance claim; replenishment
    // settles the unpaid unit once and a later payroll cannot duplicate it.
    {
        const uint64_t actor=130;Realm setup(37,6,256);const int region=setup.person(actor).region;
        check(setup.person(actor).work_assignment.job_kind==0&&setup.person(actor).family==Family::Civilian,"T2.3 shortage civilian actor");
        check(submit_work(setup,actor,region,0,32),"T2.3 shortage work submit");
        const int64_t money_before=setup.total_money();
        const std::string limited_save=patch_payroll_save(setup,region,1,1,0);
        Realm first(2,1,128);std::string first_error;
        check(first.deserialize(limited_save,"m5fixture",first_error),"T2.3 shortage constrained save load");
        check(first.invariant()&&first.total_money()==money_before&&first.view().provinces[region].firm==1,"T2.3 shortage constrained save conservation");
        const int64_t actor_cash=first.person(actor).cash;
        first.advance_ticks(64);
        check(regional_paid(first,actor,region)==1&&regional_ticks(first,actor,region)==32,"T2.3 shortage first partial payroll");
        check(first.person(actor).cash==actor_cash+1,"T2.3 shortage first unit credits one cash");

        check(first.invariant()&&first.total_money()==money_before,"T2.3 shortage first payroll conservation");
        const std::string replenished_save=patch_payroll_save(first,region,1,1,0);
        Realm retry(2,1,128);std::string retry_error;
        check(retry.deserialize(replenished_save,"m5fixture",retry_error),"T2.3 shortage replenished save load");
        check(retry.invariant()&&retry.total_money()==money_before,"T2.3 shortage replenishment conservation");
        retry.advance_ticks(64);
        check(regional_paid(retry,actor,region)==2,"T2.3 shortage retry pays remainder");
        check(retry.person(actor).cash==actor_cash+2,"T2.3 shortage retry credits exactly one more unit");
        check(retry.invariant()&&retry.total_money()==money_before,"T2.3 shortage retry conservation");
        retry.advance_ticks(64);
        check(regional_paid(retry,actor,region)==2&&retry.person(actor).cash==actor_cash+2,"T2.3 shortage later payroll does not duplicate");
        check(retry.invariant()&&retry.total_money()==money_before,"T2.3 shortage final conservation");
    }
    // Death after a partial payment freezes the unpaid 32-tick gap.
    {
        const uint64_t actor=130;Realm setup(37,6,256);const int region=setup.person(actor).region;
        check(setup.person(actor).work_assignment.job_kind==0&&setup.person(actor).family==Family::Civilian,"T2.3 death civilian actor");
        check(submit_work(setup,actor,region,0,32),"T2.3 death work submit");
        const int64_t money_before=setup.total_money();
        const std::string limited_save=patch_payroll_save(setup,region,1,1,0);
        Realm partial(2,1,128);std::string partial_error;
        check(partial.deserialize(limited_save,"m5fixture",partial_error),"T2.3 death constrained save load");
        check(partial.invariant()&&partial.total_money()==money_before&&partial.view().provinces[region].firm==1,"T2.3 death constrained save conservation");
        const int64_t actor_cash=partial.person(actor).cash;
        partial.advance_ticks(64);
        check(regional_paid(partial,actor,region)==1&&regional_ticks(partial,actor,region)==32,"T2.3 death leaves unpaid attendance gap");
        check(partial.person(actor).cash==actor_cash+1,"T2.3 death first partial payment");
        const std::string dead_save=patch_payroll_save(partial,region,1,1,actor);
        Realm dead(2,1,128);std::string dead_error;
        check(dead.deserialize(dead_save,"m5fixture",dead_error),"T2.3 death state save load");
        const int64_t dead_money=dead.total_money(),dead_cash=dead.person(actor).cash;
        check(!dead.person(actor).alive&&regional_paid(dead,actor,region)==1&&regional_ticks(dead,actor,region)==32,"T2.3 death freezes unpaid gap at transition");
        check(dead.invariant()&&dead_money==money_before,"T2.3 death transition conservation");
        dead.advance_ticks(128);
        check(!dead.person(actor).alive&&regional_paid(dead,actor,region)==1&&regional_ticks(dead,actor,region)==32,"T2.3 dead actor never receives deferred gap");
        check(dead.person(actor).cash==dead_cash&&dead.invariant()&&dead.total_money()==dead_money,"T2.3 death later payroll conservation");
    }
    {
        // R4 Loan codec: malformed count and truncated record are rejected with
        // the legacy error and do not replace an existing output vector.
        std::vector<Loan> existing{Loan{7,8,9,10,11,12,13,true,true,42}};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Loan> out=existing;
            bool threw=false; std::string msg;
            try { detail::read_loan_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            // Preserve every primary and court-extension field on failure.
            check(out.size()==existing.size() &&
                  (!existing.empty() &&
                   out[0].id==existing[0].id &&
                   out[0].debtor==existing[0].debtor &&
                   out[0].creditor==existing[0].creditor &&
                   out[0].due==existing[0].due &&
                   out[0].principal==existing[0].principal &&
                   out[0].interest==existing[0].interest &&
                   out[0].paid==existing[0].paid &&
                   out[0].defaulted==existing[0].defaulted &&
                   out[0].royal_pool==existing[0].royal_pool &&
                   out[0].cause==existing[0].cause),
                  "loan codec failure preserves output loan");
            return {threw,msg};
        };
        auto r1=read_throws("1000001 1 2 3 4 5 6 7 0");
        check(r1.first && r1.second=="Save count limit","loan codec malformed count throws legacy error");
        auto r2=read_throws("not-a-number");
        check(r2.first && r2.second=="Save count limit","loan codec bad count throws legacy error");
        auto r3=read_throws("1 1 2 3 4 5 6 7");
        check(r3.first && r3.second=="Save count limit","loan codec truncated record throws legacy error");
        // Valid round trip preserves token order/values.
        {
            std::ostringstream out;
            detail::write_loan_section(out,existing);
            check(out.str()=="1 7 8 9 10 11 12 13 1 ",
                  "loan codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Loan> parsed;
            detail::read_loan_section(in,parsed);
            check(parsed.size()==1 && parsed[0].id==7 && parsed[0].debtor==8 &&
                  parsed[0].creditor==9 && parsed[0].due==10 &&
                  parsed[0].principal==11 && parsed[0].interest==12 &&
                  parsed[0].paid==13 && parsed[0].defaulted==true,
                  "loan codec valid record round trips");
        }
    }
    {
        // R4 Project codec: malformed count and truncated record are rejected with
        // the legacy error and do not replace an existing output vector.
        std::vector<Project> existing{Project{7,8,9,10,11,12,true,true,false,42}};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Project> out=existing;
            bool threw=false; std::string msg;
            try { detail::read_project_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            // Preserve every primary and court-extension field on failure.
            check(out.size()==existing.size() &&
                  (!existing.empty() &&
                   out[0].id==existing[0].id &&
                   out[0].owner==existing[0].owner &&
                   out[0].region==existing[0].region &&
                   out[0].due==existing[0].due &&
                   out[0].kind==existing[0].kind &&
                   out[0].budget==existing[0].budget &&
                   out[0].completed==existing[0].completed &&
                   out[0].engineer==existing[0].engineer &&
                   out[0].stalled==existing[0].stalled &&
                   out[0].cause==existing[0].cause),
                  "project codec failure preserves output project");
            return {threw,msg};
        };
        auto r1=read_throws("1000001 7 8 9 10 11 12 1");
        check(r1.first && r1.second=="Save count limit","project codec malformed count throws legacy error");
        auto r2=read_throws("not-a-number");
        check(r2.first && r2.second=="Save count limit","project codec bad count throws legacy error");
        auto r3=read_throws("1 7 8 9 10 11 12");
        check(r3.first && r3.second=="Save count limit","project codec truncated record throws legacy error");
        // Valid round trip preserves token order/values.
        {
            std::ostringstream out;
            detail::write_project_section(out,existing);
            check(out.str()=="1 7 8 9 10 11 12 1 ",
                  "project codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Project> parsed;
            detail::read_project_section(in,parsed);
            check(parsed.size()==1 && parsed[0].id==7 && parsed[0].owner==8 &&
                  parsed[0].region==9 && parsed[0].due==10 &&
                  parsed[0].kind==11 && parsed[0].budget==12 &&
                  parsed[0].completed==true,
                  "project codec valid record round trips");
        }
    }
    {
        // R4 Fact codec: malformed count and truncated record are rejected with
        // the legacy error and do not replace an existing output vector.
        std::vector<Fact> existing{Fact{7,8,9,10,11,12,13,14,false}};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Fact> out=existing;
            bool threw=false; std::string msg;
            try { detail::read_fact_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            // Preserve every primary field on failure.
            check(out.size()==existing.size() &&
                  (!existing.empty() &&
                   out[0].id==existing[0].id &&
                   out[0].parent==existing[0].parent &&
                   out[0].actor==existing[0].actor &&
                   out[0].target==existing[0].target &&
                   out[0].day==existing[0].day &&
                   out[0].region==existing[0].region &&
                   out[0].kind==existing[0].kind &&
                   out[0].amount==existing[0].amount &&
                   out[0].secret==existing[0].secret),
                  "fact codec failure preserves output fact");
            return {threw,msg};
        };
        auto r1=read_throws("8193 7 8 9 10 11 12 13 14 0");
        check(r1.first && r1.second=="Save count limit","fact codec excessive count throws legacy error");
        auto r2=read_throws("1000001 7 8 9 10 11 12 13 14 0");
        check(r2.first && r2.second=="Save count limit","fact codec excessive count throws legacy error");
        auto r3=read_throws("not-a-number");
        check(r3.first && r3.second=="Save count limit","fact codec bad count throws legacy error");
        auto r4=read_throws("1 7 8 9 10 11 12 13 14");
        check(r4.first && r4.second=="Save count limit","fact codec truncated record throws legacy error");
        // Valid round trip preserves token order/values.
        {
            std::ostringstream out;
            detail::write_fact_section(out,existing);
            check(out.str()=="1 7 8 9 10 11 12 13 14 0 ",
                  "fact codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Fact> parsed;
            detail::read_fact_section(in,parsed);
            check(parsed.size()==1 && parsed[0].id==7 && parsed[0].parent==8 &&
                  parsed[0].actor==9 && parsed[0].target==10 &&
                  parsed[0].day==11 && parsed[0].region==12 &&
                  parsed[0].kind==13 && parsed[0].amount==14 &&
                  parsed[0].secret==false,
                  "fact codec valid record round trips");
        }
    }
    {
        // Typed Realm relation codec preserves established integer tokens and stages invalid kinds.
        {
            const std::vector<Edge> source{{7, 8, RelationKind::Agreement}};
            std::ostringstream encoded;
            detail::write_edge_section(encoded, source);
            check(encoded.str() == "1 7 8 2 ", "typed relation writer preserves schema token");
            std::vector<Edge> decoded{{9, 10, RelationKind::Sibling}};
            std::istringstream validInput(encoded.str());
            detail::read_edge_section(validInput, decoded,22);
            check(decoded.size() == 1 && decoded[0].from == 7 && decoded[0].to == 8 &&
                  decoded[0].kind == RelationKind::Agreement, "typed relation reader round trips enum");
            const auto before = decoded;
            std::istringstream invalidInput("1 11 12 3 ");
            bool invalidRejected = false;
            try { detail::read_edge_section(invalidInput, decoded); }
            catch (const std::runtime_error& ex) { invalidRejected = std::string(ex.what()) == "Save relation kind"; }
            check(invalidRejected && decoded.size() == before.size() && decoded[0].from == before[0].from &&
                  decoded[0].to == before[0].to && decoded[0].kind == before[0].kind,
                  "invalid relation kind rejected without replacing staged result");
            std::istringstream spouseInput("1 7 8 3 ");
            detail::read_edge_section(spouseInput, decoded,23);
            check(decoded.size()==1&&decoded[0].kind==RelationKind::Spouse,"schema23 relation codec accepts spouse kind");
        }

        // Typed Knowledge codec rejects malformed rows and preserves existing output.
        std::vector<Knowledge> existing{Knowledge{7,8,9,10,KnowledgeKind::SecretEvidence,-1}};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Knowledge> out=existing;
            bool threw=false; std::string msg;
            try { detail::read_knowledge_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            // Preserve every primary field on failure.
            check(out.size()==existing.size() &&
                  (!existing.empty() &&
                   out[0].observer==existing[0].observer &&
                   out[0].fact==existing[0].fact &&
                   out[0].arrives==existing[0].arrives &&
                   out[0].source_investigation_id==existing[0].source_investigation_id&&
                   out[0].kind==existing[0].kind&&out[0].confidence_basis_points==existing[0].confidence_basis_points),
                  "knowledge codec failure preserves output knowledge");
            return {threw,msg};
        };
        auto r1=read_throws("KNOWLEDGE_3 1000001 7 8 9 10 1 -1");
        check(r1.first && r1.second=="Save count limit","knowledge codec excessive count throws legacy error");
        auto r2=read_throws("KNOWLEDGE_V99 1 7 8 9 10 1 -1");
        check(r2.first && r2.second=="Save knowledge section","knowledge codec rejects unknown section version");
        auto r3=read_throws("KNOWLEDGE_3 1 7 8 9 10 1");
        check(r3.first && r3.second=="Save knowledge fields","knowledge codec rejects a typed row missing confidence");
        // Valid round trip preserves token order/values.
        {
            std::ostringstream out;
            detail::write_knowledge_section(out,existing);
            check(out.str()=="KNOWLEDGE_3 1 7 8 9 10 1 -1 ",
                  "knowledge codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Knowledge> parsed;
            detail::read_knowledge_section(in,parsed);
            check(parsed.size()==1 && parsed[0].observer==7 && parsed[0].fact==8 &&
                  parsed[0].arrives==9 && parsed[0].source_investigation_id==10,
                  "knowledge codec valid record round trips");
            check(parsed[0].kind==KnowledgeKind::SecretEvidence&&
                  parsed[0].confidence_basis_points==kUnknownKnowledgeConfidence,
                  "knowledge codec preserves evidence type and unknown confidence");
        }
        {
            std::istringstream in("KNOWLEDGE_2 1 7 8 9 10 ");
            std::vector<Knowledge> migrated;
            detail::read_knowledge_section(in,migrated,32);
            check(migrated.size()==1&&migrated[0].kind==KnowledgeKind::SecretEvidence&&
                  migrated[0].confidence_basis_points==kUnknownKnowledgeConfidence,
                  "schema32 source migrates to evidence with unknown confidence");
        }
        {
            std::istringstream legacyInput("1 7 8 9 ");
            std::vector<Knowledge> migrated;
            detail::read_knowledge_section(legacyInput,migrated,30);
            check(migrated.size()==1&&migrated[0].observer==7&&migrated[0].fact==8&&
                  migrated[0].arrives==9&&migrated[0].source_investigation_id==0&&
                  migrated[0].kind==KnowledgeKind::UnknownLegacy&&
                  migrated[0].confidence_basis_points==kUnknownKnowledgeConfidence,
                  "schema30 knowledge has unknown type, source and confidence");
        }
        for(const std::string malformed:{
                "KNOWLEDGE_3 1 7 8 9 0 1 -1",
                "KNOWLEDGE_3 1 7 8 9 10 0 -1",
                "KNOWLEDGE_3 1 7 8 9 10 1 -2",
                "KNOWLEDGE_3 1 7 8 9 10 1 10001",
                "KNOWLEDGE_3 1 7 8 9 10 2 -1"}){
            const auto rejected=read_throws(malformed);
            check(rejected.first&&rejected.second=="Save knowledge fields",
                  "typed knowledge rejects invalid type, source pairing or confidence");
        }
        // Duplicate (observer, fact) rows remain in input order in the codec vector.
        {
            std::vector<Knowledge> duplicate_rows{Knowledge{7,8,9,10,KnowledgeKind::SecretEvidence,-1},Knowledge{7,8,5,11,KnowledgeKind::SecretEvidence,-1}};
            std::ostringstream out;
            detail::write_knowledge_section(out,duplicate_rows);
            std::istringstream in(out.str());
            std::vector<Knowledge> parsed;
            detail::read_knowledge_section(in,parsed);
            check(parsed.size()==2 && parsed[0].observer==7 && parsed[0].fact==8 &&
                  parsed[0].arrives==9 && parsed[1].observer==7 &&
                  parsed[0].source_investigation_id==10 && parsed[0].kind==KnowledgeKind::SecretEvidence && parsed[1].fact==8 &&
                  parsed[1].arrives==5 && parsed[1].source_investigation_id==11,
                  "knowledge codec duplicate rows remain ordered");
        }
    }
    {
        // R4 Proposal codec: malformed/excessive count stays Save count limit;
        // truncated primary record throws the legacy Save court fields error.
        std::vector<Proposal> existing{Proposal{7,8,9,10,11,false}};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Proposal> out=existing;
            bool threw=false; std::string msg;
            try { detail::read_proposal_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            // Preserve every primary field on failure.
            check(out.size()==existing.size() &&
                  (!existing.empty() &&
                   out[0].id==existing[0].id &&
                   out[0].from==existing[0].from &&
                   out[0].to==existing[0].to &&
                   out[0].amount==existing[0].amount &&
                   out[0].due==existing[0].due &&
                   out[0].resolved==existing[0].resolved),
                  "proposal codec failure preserves output proposal");
            return {threw,msg};
        };
        auto r1=read_throws("1000001 7 8 9 10 11 0");
        check(r1.first && r1.second=="Save count limit","proposal codec excessive count throws legacy error");
        auto r2=read_throws("not-a-number");
        check(r2.first && r2.second=="Save count limit","proposal codec bad count throws legacy error");
        auto r3=read_throws("1 7 8 9 10 11");
        check(r3.first && r3.second=="Save court fields","proposal codec truncated record throws legacy court error");
        // Valid round trip preserves token order/values.
        {
            std::ostringstream out;
            detail::write_proposal_section(out,existing);
            check(out.str()=="1 7 8 9 10 11 0 ",
                  "proposal codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Proposal> parsed;
            detail::read_proposal_section(in,parsed);
            check(parsed.size()==1 && parsed[0].id==7 && parsed[0].from==8 &&
                  parsed[0].to==9 && parsed[0].amount==10 &&
                  parsed[0].due==11 && parsed[0].resolved==false,
                  "proposal codec valid record round trips");
        }
    }
    {
        using namespace polyec::detail;
        RealmPrimaryHeader existing{};
        existing.day=10; existing.regions=2; existing.seed=3; existing.next_id=4; existing.next_fact=5;
        existing.opening_money=6; existing.carrier=7; existing.estate=8; existing.endowment=9;
        existing.draft.tier=10; existing.draft.seen=11; existing.draft.chosen=12; existing.draft.rerolled=true;
        existing.draft.cards={14,15,16};
        existing.opening={17,18,19,20,21,22,23};
        existing.produced={24,25,26,27,28,29,30};
        existing.consumed={31,32,33,34,35,36,37};
        for(int i=0;i<7;++i) for(int j=0;j<7;++j) existing.routes[i][j]=(i!=j);
        for(size_t i=0;i<kOrganizationCount;++i) existing.organization_funds[i]=100+int64_t(i);
        existing.rng.seed(42);

        auto same=[&](const RealmPrimaryHeader&a,const RealmPrimaryHeader&b)->bool{
            if(a.day!=b.day||a.regions!=b.regions||a.seed!=b.seed||a.next_id!=b.next_id||a.next_fact!=b.next_fact||a.opening_money!=b.opening_money||a.carrier!=b.carrier||a.estate!=b.estate||a.endowment!=b.endowment) return false;
            if(a.draft.tier!=b.draft.tier||a.draft.seen!=b.draft.seen||a.draft.chosen!=b.draft.chosen||a.draft.rerolled!=b.draft.rerolled) return false;
            for(int i=0;i<3;++i) if(a.draft.cards[i]!=b.draft.cards[i]) return false;
            for(int i=0;i<7;++i){ if(a.opening[i]!=b.opening[i]||a.produced[i]!=b.produced[i]||a.consumed[i]!=b.consumed[i]) return false; for(int j=0;j<7;++j) if(a.routes[i][j]!=b.routes[i][j]) return false; }
            for(size_t i=0;i<kOrganizationCount;++i) if(a.organization_funds[i]!=b.organization_funds[i]) return false;
            return a.rng==b.rng;
        };

        {
            std::istringstream in("1 2 3 4 5 6 7 8");
            RealmPrimaryHeader out=existing;
            bool ok=read_realm_primary_header_initial(in,out);
            check(!ok,"primary header initial truncated returns false");
            check(in.fail(),"primary header initial truncated leaves stream failed");
            check(same(out,existing),"primary header initial truncation preserves output");
        }
        {
            std::istringstream in("1 2 x 4 5 6 7 8 9");
            RealmPrimaryHeader out=existing;
            bool ok=read_realm_primary_header_initial(in,out);
            check(!ok,"primary header initial nonnumeric returns false");
            check(in.fail(),"primary header initial nonnumeric leaves stream failed");
            check(same(out,existing),"primary header initial nonnumeric preserves output");
        }
        {
            std::istringstream in("1 2 3 4 5 6 7 8 9");
            RealmPrimaryHeader header=existing;
            header.draft.tier=777; header.draft.cards={9,8,7}; header.opening[0]=12345; header.rng.seed(7);
            bool init_ok=read_realm_primary_header_initial(in,header);
            check(init_ok,"primary header initial valid parse for tail truncation setup");
            RealmPrimaryHeader before=header;
            bool tail_ok=read_realm_primary_header_tail(in,header);
            check(!tail_ok,"primary header tail truncated returns false");
            check(in.fail(),"primary header tail truncated leaves stream failed");
            check(same(header,before),"primary header tail truncation preserves output");
        }
        {
            std::istringstream in("1 2 3 4 5 6 7 8 9 not-a-number");
            RealmPrimaryHeader header=existing;
            header.draft.tier=777; header.draft.cards={9,8,7}; header.opening[0]=12345; header.rng.seed(7);
            bool init_ok=read_realm_primary_header_initial(in,header);
            check(init_ok,"primary header initial valid parse for tail nonnumeric setup");
            RealmPrimaryHeader before=header;
            bool tail_ok=read_realm_primary_header_tail(in,header);
            check(!tail_ok,"primary header tail nonnumeric returns false");
            check(in.fail(),"primary header tail nonnumeric leaves stream failed");
            check(same(header,before),"primary header tail nonnumeric preserves output");
        }
        {
            RealmPrimaryHeader a=existing;
            a.rng.seed(42);
            std::ostringstream out;
            write_realm_primary_header(out,a);
            std::istringstream in(out.str());
            RealmPrimaryHeader b{};
            bool init_ok=read_realm_primary_header_initial(in,b);
            bool tail_ok=read_realm_primary_header_tail(in,b,44);
            check(init_ok && tail_ok,"primary header valid parse");
            check(same(a,b),"primary header round trip preserves every field and rng");
        }
        {
            RealmPrimaryHeader header{};
            header.day=1; header.regions=2; header.seed=3; header.next_id=4; header.next_fact=5;
            header.opening_money=6; header.carrier=7; header.estate=8; header.endowment=9;
            header.draft.tier=10; header.draft.seen=11; header.draft.chosen=12; header.draft.rerolled=true;
            header.draft.cards={14,15,16};
            header.opening={17,18,19,20,21,22,23};
            header.produced={24,25,26,27,28,29,30};
            header.consumed={31,32,33,34,35,36,37};
            for(int i=0;i<7;++i) for(int j=0;j<7;++j) header.routes[i][j]=(i!=j);
            for(size_t i=0;i<kOrganizationCount;++i) header.organization_funds[i]=100+int64_t(i);
            header.rng.seed(42);
            std::ostringstream out;
            write_realm_primary_header(out,header);
            check(out.str().rfind("1 2 3 4 5 6 7 8 9 10 11 12 1 14 15 16 ",0)==0, "primary header canonical writer prefix");
        }
    }
    {
        Realm base(37,2,128);
        std::string data=base.serialize("test");
        auto tokens_of=[&](const std::string& text){ std::vector<std::string> toks; std::istringstream in(text); std::string x; while(in>>x) toks.push_back(x); return toks; };
        auto join_tokens=[&](const std::vector<std::string>& toks){ std::string out; for(size_t i=0;i<toks.size();++i){ if(i) out+=' '; out+=toks[i]; } return out; };
        {
            auto toks=tokens_of(data); toks[3]="3";
            Realm r(1,1,128); auto before=r.fingerprint(); std::string err;
            check(!r.deserialize(join_tokens(toks),"test",err) && err=="Save geography" && r.fingerprint()==before, "Realm invalid regions maps Save geography and preserves destination");
        }
        {
            auto toks=tokens_of(data); toks.resize(8);
            Realm r(1,1,128); auto before=r.fingerprint(); std::string err;
            check(!r.deserialize(join_tokens(toks),"test",err) && err=="Save geography" && r.fingerprint()==before, "Realm initial header failure maps Save geography");
        }
        {
            auto toks=tokens_of(data); toks.resize(11);
            Realm r(1,1,128); auto before=r.fingerprint(); std::string err;
            check(!r.deserialize(join_tokens(toks),"test",err) && err=="Save count limit" && r.fingerprint()==before, "Realm tail header failure maps Save count limit");
        }
    }
    // M5 T2.4 Gate 1: establishment ownership, funded creation, and v12 save section.
    {
        Realm equity(37,2,128);
        const uint64_t economicOwner=5,otherActor=4;
        const int equityRegion=equity.person(economicOwner).region;
        const double ownerBeforeCreate=equity.utility(economicOwner);
        const double otherBeforeCreate=equity.utility(otherActor);
        auto equitySite=equity.execute(EstablishmentCommand{economicOwner,equity.person(economicOwner).decisions+1,
            EstablishmentOwnerKind::Person,economicOwner,equityRegion,EstablishmentIndustry::ClothWorkshop,1,1});
        check(equitySite.success&&std::abs(equity.utility(economicOwner)-ownerBeforeCreate)<1e-9,
              "person-owned workshop funding remains in owner's net wealth");
        check(std::abs(equity.utility(otherActor)-otherBeforeCreate)<1e-9,
              "another person's workshop asset is excluded from utility");
        auto fiberBuy=equity.execute({economicOwner,equity.person(economicOwner).decisions+1,
            {Intent::Buy,equityRegion,int(GoodId::Fiber),1}});
        check(fiberBuy.success&&equity.person(economicOwner).inventory[size_t(GoodId::Fiber)]==1,
              "equity fixture buys Fiber through Realm trade command");
        const double ownerBeforeDeposit=equity.utility(economicOwner);
        auto fiberDeposit=equity.execute(EstablishmentTransferCommand{economicOwner,equity.person(economicOwner).decisions+1,
            equitySite.session,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fiber),1});
        check(fiberDeposit.success&&std::abs(equity.utility(economicOwner)-ownerBeforeDeposit)<1e-9,
              "moving Fiber from owner's inventory into their workshop preserves equity");
        check(equity.invariant(),"owner enterprise index matches command-created site");

        Realm wageEquity(37,2,128);
        const uint64_t wageOwner=5;
        const int wageRegion=wageEquity.person(wageOwner).region;
        uint64_t worker=0;
        for(uint64_t id=1;id<=uint64_t(wageEquity.count());++id)
            if(id!=wageOwner&&wageEquity.person(id).alive&&wageEquity.person(id).region==wageRegion){worker=id;break;}
        check(worker!=0,"enterprise liability fixture has a worker in the owner's region");
        const auto wageSite=wageEquity.execute(EstablishmentCommand{wageOwner,wageEquity.person(wageOwner).decisions+1,
            EstablishmentOwnerKind::Person,wageOwner,wageRegion,EstablishmentIndustry::ClothWorkshop,1,1});
        check(wageSite.success,"enterprise liability fixture creates privately owned site");
        check(wageEquity.execute(WorkAuthorizationCommand{wageOwner,wageEquity.person(wageOwner).decisions+1,
            wageSite.session,worker,WorkAuthorizationOperation::Set,0,kSubdayUnitsPerDay}).success,
            "owner authorizes worker before a one-day shift");
        ScheduleCommand shift{};shift.actor=worker;shift.sequence=wageEquity.person(worker).decisions+1;
        shift.operation=ScheduleCommand::Operation::Submit;shift.begin=0;shift.end=kSubdayUnitsPerDay;
        shift.demand_rate=activity::kMaxDemandRate;shift.kind=ActivityKind::Work;shift.region=wageRegion;
        shift.establishment_id=wageSite.session;
        check(wageEquity.execute(shift).success,"authorized worker submits worksite shift");
        wageEquity.advance();
        const auto sitesAfterShift=wageEquity.establishments();
        const auto& siteAfterShift=sitesAfterShift.front();
        const auto attendance=wageEquity.schedule_book().work_present_ticks_by_establishment.find({worker,siteAfterShift.id});
        const auto paid=siteAfterShift.work_pay.paid_units.find(worker);
        const auto legacy=siteAfterShift.work_pay.legacy_credit.find(worker);
        const auto remainder=siteAfterShift.work_pay.remainders.find(worker);
        const int64_t eligible=attendance==wageEquity.schedule_book().work_present_ticks_by_establishment.end()?0:
            attendance->second/wageEquity.gameplay_config().ticks_per_unit();
        const int64_t paidUnits=paid==siteAfterShift.work_pay.paid_units.end()?0:paid->second;
        const int64_t legacyUnits=legacy==siteAfterShift.work_pay.legacy_credit.end()?0:legacy->second;
        const int64_t wageRemainder=remainder==siteAfterShift.work_pay.remainders.end()?0:remainder->second;
        const int64_t outstanding=std::max<int64_t>(0,eligible-legacyUnits-paidUnits)*wageEquity.gameplay_config().wage_per_unit()-wageRemainder;
        check(outstanding>0,"unpaid site wages remain a liability after cash-limited payroll");
        double manualWealth=double(wageEquity.person(wageOwner).cash-wageEquity.debt(wageOwner));
        for(size_t good=0;good<kGoodIdCount;++good){
            manualWealth+=double(wageEquity.person(wageOwner).inventory[good])*kGoodReferencePrice[good];
            manualWealth+=double(siteAfterShift.stock[good])*kGoodReferencePrice[good];
        }
        manualWealth+=double(siteAfterShift.cash-outstanding);
        const auto& wagePerson=wageEquity.person(wageOwner);
        const double manualUtility=(wagePerson.alive?0.0:-20.0)+manualWealth/100.0+
            wagePerson.health*.05+wagePerson.nutrition*.01;
        check(std::abs(wageEquity.utility(wageOwner)-manualUtility)<1e-9,
              "economic utility subtracts unpaid site wages from private enterprise equity");

        Realm empty(37,2,128);
        const std::string emptySave=empty.serialize("establishment-gate");
        const size_t emptyTag=emptySave.find(" ESTABLISH_13 0 ");
        check(emptyTag!=std::string::npos&&emptySave.substr(emptyTag,emptySave.find(" CONFIG_3 ")-emptyTag)==" ESTABLISH_13 0 ","establishment empty section exact bytes");

        Realm owned(37,2,128);
        const uint64_t owner=65;
        const int64_t money=owned.total_money();
        const auto oldBalances=owned.balances();
        EstablishmentCommand create{owner,owned.person(owner).decisions+1,EstablishmentOwnerKind::Person,
                                    owner,owned.person(owner).region,EstablishmentIndustry::ClothWorkshop,1,1};
        auto created=owned.execute(create);
        check(created.success&&owned.establishments().size()==1,"person establishment create");
        check(owned.establishments()[0].id==created.session&&owned.establishments()[0].capacity==1,"establishment id result and default capacity");
        check(owned.total_money()==money&&owned.balances()[0]==oldBalances[0]-1&&owned.balances()[9]==1,"person funding conserved and categorized");
        check(owned.invariant(),"person establishment invariant");

        const auto serializeTokens=[](const std::string& text){std::vector<std::string> out;std::istringstream in(text);std::string token;while(in>>token)out.push_back(token);return out;};
        const auto joinTokens=[](const std::vector<std::string>& tokens){std::ostringstream out;for(size_t i=0;i<tokens.size();++i){if(i)out<<' ';out<<tokens[i];}return out.str();};
        const auto findSection=[](const std::vector<std::string>& tokens){for(size_t i=0;i<tokens.size();++i)if(tokens[i]=="ESTABLISH_14")return i;return tokens.size();};
        auto tokens=serializeTokens(owned.serialize("establishment-gate"));
        const size_t section=findSection(tokens);
        check(section+44==size_t(std::find(tokens.begin(),tokens.end(),"CONFIG_3")-tokens.begin()),"establishment record has 27 canonical product stock slots, production and source-ledger fields");
        check(tokens[section+1]=="1"&&tokens[section+8]=="1","establishment scalar field order");
        const std::string validSaved=owned.serialize("establishment-gate");
        std::ostringstream expectedSection;expectedSection<<" ESTABLISH_13 1 "<<owned.establishments()[0].id<<' '
            <<owned.establishments()[0].region<<" 0 "<<owner<<" 1 1 1 ";
        for(int i=0;i<kEstablishmentStockSlots;++i)expectedSection<<"0 ";
        expectedSection<<"0 0 0 0 0 0 -1 0 ";
        const auto establishAt=validSaved.find(" ESTABLISH_13 ");
        check(validSaved.substr(establishAt,validSaved.find(" CONFIG_3 ")-establishAt)==expectedSection.str(),"establishment nonempty section exact bytes");
        Realm loaded(2,1,128);std::string saveError;
        check(loaded.deserialize(validSaved,"establishment-gate",saveError),"establishment save roundtrip");
        check(loaded.serialize("establishment-gate")==validSaved&&loaded.invariant(),"establishment deterministic roundtrip");

        // Frozen v12 holdings cover every legacy good in Person, Province,
        // and Establishment records before the explicit v13 canonical migration.
        auto holdings=serializeTokens(validSaved);
        const size_t personCountAt=[&]{for(size_t j=100;j<holdings.size();++j){size_t count=0;try{count=size_t(std::stoull(holdings[j]));}catch(...){continue;}if(count<128||count>1000000||j+1+56*count>holdings.size())continue;bool dense=true;for(size_t k=0;k<count;++k)if(holdings[j+1+56*k]!=std::to_string(k+1)){dense=false;break;}if(dense)return j;}throw std::runtime_error("migration fixture person block");}();
        const size_t peopleCount=size_t(std::stoull(holdings[personCountAt]));const size_t peopleStart=personCountAt+1;
        const size_t provinceCountAt=peopleStart+peopleCount*56;const size_t provinceCount=size_t(std::stoull(holdings[provinceCountAt]));const size_t provincesStart=provinceCountAt+1;
        const size_t establishmentAt=findSection(holdings);const std::array<int,7> legacyToGood{{0,9,11,5,4,16,14}};
        for(size_t legacy=0;legacy<legacyToGood.size();++legacy){const size_t good=size_t(legacyToGood[legacy]);const int personAmount=1+int(legacy),provinceAmount=2+int(legacy),siteAmount=3+int(legacy);
            holdings[peopleStart+56*(owner-1)+29+good]=std::to_string(personAmount);
            const size_t provinceStockAt=provincesStart+12+good;
            holdings[provinceStockAt]=std::to_string(std::stoll(holdings[provinceStockAt])+provinceAmount);
            holdings[establishmentAt+9+good]=std::to_string(siteAmount);
            holdings[18+good]=std::to_string(std::stoll(holdings[18+good])+personAmount+provinceAmount+siteAmount);
        }
        const std::string v14Holdings=joinTokens(holdings);Realm canonicalLoaded(2,1,128);
        const bool canonicalHoldingsOk=canonicalLoaded.deserialize(v14Holdings,"establishment-gate",saveError);
        check(canonicalHoldingsOk,canonicalHoldingsOk?"canonical holdings fixture validates":saveError.c_str());
        const std::string v13Holdings=downgrade_save_to_v13(v14Holdings);
        Realm v13Loaded(2,1,128);
        check(v13Loaded.deserialize(v13Holdings,"establishment-gate",saveError),saveError.c_str());
        const std::string v12Holdings=downgrade_empty_goods_save_to_v12(v14Holdings);
        check(v12Holdings.rfind("POLYEC_REALM_12",0)==0,"v12 goods fixture tag");
        Realm migratedHoldings(2,1,128);
        check(migratedHoldings.deserialize(v12Holdings,"establishment-gate",saveError),saveError.c_str());
        for(size_t legacy=0;legacy<legacyToGood.size();++legacy){const size_t good=size_t(legacyToGood[legacy]);
            check(migratedHoldings.person(owner).inventory[good]==1+int64_t(legacy),"v12 Person inventory maps to canonical good");
            check(migratedHoldings.view().provinces[0].stock[good]==owned.view().provinces[0].stock[good]+2+int64_t(legacy),"v12 Province stock maps to canonical good");
            check(migratedHoldings.establishments()[0].stock[good]==3+int64_t(legacy),"v12 Establishment group stock maps to canonical good");
        }
        check(migratedHoldings.invariant(),"v12 canonical holdings conserve all mapped goods");

        auto badEnum=tokens;badEnum[section+6]="99";
        const uint64_t destinationBefore=loaded.fingerprint();
        check(!loaded.deserialize(joinTokens(badEnum),"establishment-gate",saveError)&&loaded.fingerprint()==destinationBefore,"bad establishment industry rejects atomically");
        auto badOwnerKind=tokens;badOwnerKind[section+4]="99";
        check(!loaded.deserialize(joinTokens(badOwnerKind),"establishment-gate",saveError)&&loaded.fingerprint()==destinationBefore,"bad establishment owner kind rejects atomically");
        auto badCapacity=tokens;badCapacity[section+7]="0";
        check(!loaded.deserialize(joinTokens(badCapacity),"establishment-gate",saveError)&&loaded.fingerprint()==destinationBefore,"bad establishment capacity rejects atomically");
        const auto truncatedEstablishment=validSaved.substr(0,establishAt+std::string(" ESTABLISH_13 1 ").size());
        check(!loaded.deserialize(truncatedEstablishment,"establishment-gate",saveError)&&loaded.fingerprint()==destinationBefore,"truncated establishment rejects atomically");

        Realm deceasedOwner=load_fixture(patch_save(owned,{{owner,14}},{"0"}));
        check(!deceasedOwner.person(owner).alive&&deceasedOwner.establishments().size()==1&&deceasedOwner.invariant(),"establishment survives owner death");

        std::string v10=downgrade_empty_goods_save_to_v12(emptySave,true);
        v10.replace(0,std::string("POLYEC_REALM_12").size(),"POLYEC_REALM_10");
        const size_t scheduleTag=v10.find(" SCHEDULE_5 ");
        check(scheduleTag!=std::string::npos,"schedule section present for v10 migration");
        const size_t establishmentTag=v10.find(" ESTABLISH_1 ");
        check(establishmentTag==std::string::npos,"pre-Establishment v10 fixture has no establishment section");
        v10.resize(scheduleTag);
        v10+=" SCHEDULE_4 0 0 1 0 0 0 0 0 0 ";
        Realm migrated(2,1,128);
        const bool migratedOk=migrated.deserialize(v10,"establishment-gate",saveError);
        check(migratedOk&&migrated.establishments().empty(),migratedOk?"v10 migrates with no establishments":saveError.c_str());

        auto badMapped=tokens;badMapped[section+9]="1";
        Realm invalidMapped(2,1,128);
        check(!invalidMapped.deserialize(joinTokens(badMapped),"establishment-gate",saveError),"unaccounted mapped stock rejects");
        badMapped[45]="1"; // Canonical produced_[DryFood] balances establishment stock slot 0.
        Realm balancedMapped(2,1,128);
        check(balancedMapped.deserialize(joinTokens(badMapped),"establishment-gate",saveError)&&balancedMapped.invariant(),"mapped establishment stock joins conservation");
        auto badUnmapped=tokens;badUnmapped[section+10]="1";
        Realm invalidUnmapped(2,1,128);
        check(!invalidUnmapped.deserialize(joinTokens(badUnmapped),"establishment-gate",saveError),"unmapped establishment stock rejects");

        auto rejectAtomic=[&](EstablishmentCommand command,const char* label){
            const uint64_t before=owned.fingerprint();const uint64_t decisions=owned.person(owner).decisions;
            const int64_t moneyBefore=owned.total_money();const auto result=owned.execute(command);
            check(!result.success&&owned.fingerprint()==before&&owned.person(owner).decisions==decisions&&owned.total_money()==moneyBefore,label);
        };
        auto rejected=create;rejected.sequence=owned.person(owner).decisions+2;rejectAtomic(rejected,"establishment stale sequence atomic");
        rejected=create;rejected.sequence=owned.person(owner).decisions+1;rejected.owner_id=owner+1;rejectAtomic(rejected,"establishment owner mismatch atomic");
        rejected=create;rejected.sequence=owned.person(owner).decisions+1;rejected.funding=0;rejectAtomic(rejected,"establishment minimum funding atomic");
        rejected=create;rejected.sequence=owned.person(owner).decisions+1;rejected.funding=INT64_MAX;rejectAtomic(rejected,"establishment insufficient funds atomic");
        rejected=create;rejected.sequence=owned.person(owner).decisions+1;rejected.capacity=0;rejectAtomic(rejected,"establishment zero capacity atomic");
        rejected=create;rejected.sequence=owned.person(owner).decisions+1;rejected.capacity=kEstablishmentCapacityMax+1;rejectAtomic(rejected,"establishment capacity bounds atomic");
    }
    {
        Realm counterBase(37,2,128);
        const std::string max=std::to_string(std::numeric_limits<uint64_t>::max());
        Realm counterWorld=load_fixture(patch_save(counterBase,{{65,2},{65,3}},{max,max}));
        EstablishmentCommand cmd{65,0,EstablishmentOwnerKind::Person,65,counterWorld.person(65).region,
                                 EstablishmentIndustry::ClothWorkshop,1,1};
        const uint64_t before=counterWorld.fingerprint();
        check(!counterWorld.execute(cmd).success&&counterWorld.fingerprint()==before,"establishment decision counter overflow rejects atomically");

        std::vector<std::string> headerTokens;{std::istringstream in(counterBase.serialize("m5fixture"));std::string token;while(in>>token)headerTokens.push_back(token);}
        headerTokens[5]=max;
        std::ostringstream patched;for(size_t i=0;i<headerTokens.size();++i){if(i)patched<<' ';patched<<headerTokens[i];}
        Realm idOverflow=load_fixture(patched.str());
        cmd={65,idOverflow.person(65).decisions+1,EstablishmentOwnerKind::Person,65,idOverflow.person(65).region,
             EstablishmentIndustry::ClothWorkshop,1,1};
        const uint64_t idBefore=idOverflow.fingerprint();
        check(!idOverflow.execute(cmd).success&&idOverflow.fingerprint()==idBefore,"establishment ID counter overflow rejects atomically");

        auto factTokens=headerTokens;factTokens[5]=std::to_string(counterBase.land_holdings().back().id+1);factTokens[6]=max;
        std::ostringstream factPatched;for(size_t i=0;i<factTokens.size();++i){if(i)factPatched<<' ';factPatched<<factTokens[i];}
        Realm factOverflow=load_fixture(factPatched.str());
        cmd={65,factOverflow.person(65).decisions+1,EstablishmentOwnerKind::Person,65,factOverflow.person(65).region,
             EstablishmentIndustry::ClothWorkshop,1,1};
        const uint64_t factBefore=factOverflow.fingerprint();
        check(!factOverflow.execute(cmd).success&&factOverflow.fingerprint()==factBefore,"establishment fact counter overflow rejects atomically");
    }
    {
        Realm publicWorld(37,6,256);
        const uint64_t actor=1;const int region=publicWorld.person(actor).region;
        EstablishmentCommand cmd{actor,publicWorld.person(actor).decisions+1,EstablishmentOwnerKind::Public,
                                 static_cast<uint64_t>(region),region,EstablishmentIndustry::ToolWorkshop,1,1};
        const int64_t treasury=publicWorld.view().provinces[region].treasury,money=publicWorld.total_money();
        auto result=publicWorld.execute(cmd);
        check(result.success&&publicWorld.total_money()==money&&publicWorld.establishments().size()==1,"public establishment funded by local treasury");
        check(publicWorld.view().provinces[region].treasury==treasury-1&&publicWorld.establishments()[0].cash==1,"public treasury transfer");
        Realm unauthorized(37,6,256);cmd.actor=8;cmd.sequence=unauthorized.person(8).decisions+1;cmd.region=unauthorized.person(8).region;cmd.owner_id=static_cast<uint64_t>(cmd.region);
        const uint64_t before=unauthorized.fingerprint();check(!unauthorized.execute(cmd).success&&unauthorized.fingerprint()==before,"public establishment authority reject atomic");
    }
    {
        Realm organization=Realm::scenario(39,129,30);
        check(organization.execute({129,1,{Intent::Join,organization.person(129).region,2,2,30}}).success,"establishment organization membership fixture");
        const int org=organization.person(129).organization;const int64_t fundsBefore=organization.balances()[5],money=organization.total_money();
        EstablishmentCommand cmd{129,organization.person(129).decisions+1,EstablishmentOwnerKind::Organization,
                                 static_cast<uint64_t>(org),organization.person(129).region,EstablishmentIndustry::MilitarySupplyWorkshop,1,1};
        auto result=organization.execute(cmd);
        check(result.success&&organization.total_money()==money&&organization.balances()[5]==fundsBefore-1&&organization.balances()[9]==1,"organization establishment funding conserved");
        Realm notMember=Realm::scenario(39,129,30);cmd.sequence=notMember.person(129).decisions+1;
        const uint64_t before=notMember.fingerprint();check(!notMember.execute(cmd).success&&notMember.fingerprint()==before,"organization establishment membership reject atomic");
    }
    {
        Realm collision(37,6,10000);
        check(collision.execute({65,1,{Intent::Borrow,collision.person(65).region,0,100,180}}).success,"establishment shared-ID loan fixture");
        EstablishmentCommand cmd{65,collision.person(65).decisions+1,EstablishmentOwnerKind::Person,65,
                                 collision.person(65).region,EstablishmentIndustry::ClothWorkshop,1,1};
        check(collision.execute(cmd).success,"establishment after loan");
        const auto serializeTokens=[](const std::string& text){std::vector<std::string> out;std::istringstream in(text);std::string token;while(in>>token)out.push_back(token);return out;};
        const auto joinTokens=[](const std::vector<std::string>& tokens){std::ostringstream out;for(size_t i=0;i<tokens.size();++i){if(i)out<<' ';out<<tokens[i];}return out.str();};
        const auto findSection=[](const std::vector<std::string>& tokens){for(size_t i=0;i<tokens.size();++i)if(tokens[i]=="ESTABLISH_14")return i;return tokens.size();};
        auto tokens=serializeTokens(collision.serialize("establishment-collision"));const size_t section=findSection(tokens);
        tokens[section+2]=std::to_string(collision.loans().front().id);
        Realm badSharedId(2,1,128);std::string err;
        check(!badSharedId.deserialize(joinTokens(tokens),"establishment-collision",err),"shared non-Person ID collision rejects");
        tokens[section+2]="1";
        Realm personIdCollision(2,1,128);
        check(personIdCollision.deserialize(joinTokens(tokens),"establishment-collision",err)&&personIdCollision.establishments()[0].id==1,"Person and Establishment numeric ID collision allowed");
    }
    {
        // Seed recipe stock through a literal save fixture so the production
        // path itself never invents an input or adds a transfer command.
        Realm base(51,2,128);
        const uint64_t worker=65;
        RealmObservationTestAccess::mutable_person(base,worker).skill=100;
        const int region=base.person(worker).region;
        const uint64_t site=work_site(base,region,worker);
        check(base.execute(ScheduleCommand{worker,base.person(worker).decisions+1,
            ScheduleCommand::Operation::Submit,0,0,32,0,1000,ActivityKind::Work,region,site}).success,
            "production integration Work schedule");
        std::vector<std::string> seededTokens;{
            std::istringstream in(base.serialize("production-integration"));
            std::string token;while(in>>token)seededTokens.push_back(token);
        }
        const size_t section=std::find(seededTokens.begin(),seededTokens.end(),"ESTABLISH_14")-seededTokens.begin();
        check(section<seededTokens.size(),"production integration establishment section");
        const size_t good=size_t(GoodId::Fiber);
        const size_t record=section+2;
        seededTokens[record+7+good]="1";
        seededTokens[18+good]=std::to_string(std::stoll(seededTokens[18+good])+1);
        std::ostringstream seededText;for(size_t i=0;i<seededTokens.size();++i){if(i)seededText<<' ';seededText<<seededTokens[i];}
        Realm seeded(2,1,128);std::string err;
        check(seeded.deserialize(seededText.str(),"production-integration",err),err.c_str());
        check(seeded.invariant()&&seeded.establishments()[0].stock[good]==1,"production stock fixture conserves Fiber");

        Realm uninterrupted=seeded;
        Realm split=seeded;
        uninterrupted.advance_ticks(64);
        split.advance_ticks(16);
        const std::string midSave=split.serialize("production-integration");
        Realm resumed(2,1,128);
        check(resumed.deserialize(midSave,"production-integration",err),err.c_str());
        resumed.advance_ticks(48);
        check(uninterrupted.fingerprint()==resumed.fingerprint(),"mid-shift save/load equals uninterrupted production");
        check(resumed.establishments()[0].stock[size_t(GoodId::Fiber)]==0&&
              resumed.establishments()[0].stock[size_t(GoodId::Cloth)]==1,
              "one Fiber batch produces one Cloth");
        check(resumed.establishments()[0].processed_work_units.at(worker)==2&&
              resumed.establishments()[0].last_production_day==1,
              "capacity consumes both new work units and saves day marker");
        check(resumed.person(worker).body_state.exertion==2&&
              resumed.person(worker).body_state.exertion_effort_attention_processed==32000,
              "32,000 credited Work attention units add two exertion points at the day boundary");
        check(resumed.schedule_book().work_effort_attention_by_establishment.at({worker,site})==32000,
              "two full-attention work units are saved separately from 32 presence ticks");
        check(resumed.schedule_book().work_productivity_attention_numerator_by_establishment.at({worker,site})==
                  32000*(150+resumed.person(worker).skill)&&resumed.person(worker).body_state.exertion==2,
              "production uses skill-weighted attention while body exertion still uses raw attention");
        check(resumed.invariant(),"production transaction conserves goods");

        const auto afterProduction=resumed.serialize("production-integration");
        std::vector<std::string> malformedEffortTokens;{
            std::istringstream in(afterProduction);std::string token;while(in>>token)malformedEffortTokens.push_back(token);
        }
        const size_t scheduleAt=std::find(malformedEffortTokens.begin(),malformedEffortTokens.end(),"SCHEDULE_7")-
                               malformedEffortTokens.begin();
        check(scheduleAt<malformedEffortTokens.size()&&std::stoull(malformedEffortTokens.at(scheduleAt+11))==1&&
              std::stoull(malformedEffortTokens.at(scheduleAt+12))==1,
              "schema 34 save contains one raw and one skill-weighted site effort row");
        const size_t effortAt=scheduleAt+13+17*std::stoull(malformedEffortTokens.at(scheduleAt+5))+
            2*std::stoull(malformedEffortTokens.at(scheduleAt+6))+2*std::stoull(malformedEffortTokens.at(scheduleAt+7))+
            3*std::stoull(malformedEffortTokens.at(scheduleAt+8))+3*std::stoull(malformedEffortTokens.at(scheduleAt+9))+
            3*std::stoull(malformedEffortTokens.at(scheduleAt+10));
        malformedEffortTokens.at(effortAt+2)="32001";
        std::ostringstream malformedEffortSave;
        for(size_t i=0;i<malformedEffortTokens.size();++i){if(i)malformedEffortSave<<' ';malformedEffortSave<<malformedEffortTokens[i];}
        Realm afterLoad(2,1,128);
        check(afterLoad.deserialize(afterProduction,"production-integration",err)&&
              afterLoad.serialize("production-integration")==afterProduction,
              "schema 34 production and both attention measures roundtrip byte-for-byte");
        const auto schema33Production=test_fixtures::save_as_v33(afterProduction);
        Realm migrated33(2,1,128);
        check(migrated33.deserialize(schema33Production,"production-integration",err)&&
              migrated33.schedule_book().work_productivity_attention_numerator_by_establishment.at({worker,site})==6400000&&
              migrated33.establishments()[0].processed_work_units.at(worker)==2,
              "schema 33 migration carries old effort at neutral productivity without changing past output");
        const auto beforeMalformedEffort=afterLoad.fingerprint();
        check(!afterLoad.deserialize(malformedEffortSave.str(),"production-integration",err)&&
              afterLoad.fingerprint()==beforeMalformedEffort,
              "effort above physical presence rejects without replacing the loaded Realm");
        auto malformedProductivityTokens=malformedEffortTokens;
        malformedProductivityTokens.at(effortAt+2)="32000";
        const size_t productivityAt=effortAt+3*std::stoull(malformedEffortTokens.at(scheduleAt+11));
        malformedProductivityTokens.at(productivityAt+2)="1";
        std::ostringstream malformedProductivitySave;
        for(size_t i=0;i<malformedProductivityTokens.size();++i){if(i)malformedProductivitySave<<' ';malformedProductivitySave<<malformedProductivityTokens[i];}
        check(!afterLoad.deserialize(malformedProductivitySave.str(),"production-integration",err)&&
              afterLoad.fingerprint()==beforeMalformedEffort,
              "skill-weighted attention outside its raw-attention bounds rejects atomically");
        afterLoad.advance_ticks(64);
        check(afterLoad.establishments()[0].stock[size_t(GoodId::Cloth)]==1&&afterLoad.invariant(),
              "later empty day does not duplicate production");

        // A higher-priority Generic session uses half the actor's attention.
        // The Work session remains physically present for 32 ticks, but only
        // receives 16,000 units, including across a save/load split.
        Realm halfAttention=seeded;
        auto focus=halfAttention.execute(ScheduleCommand{worker,
            halfAttention.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
            0,0,32,1,500});
        check(focus.success,"competing Generic attention session accepted");
        halfAttention.advance_ticks(16);
        check(halfAttention.schedule_book().work_present_ticks_by_establishment.at({worker,site})==16&&
              halfAttention.schedule_book().work_effort_attention_by_establishment.at({worker,site})==8000,
              "first half shift records 16 presence ticks and 8,000 Work attention units");
        Realm halfResumed(2,1,128);
        check(halfResumed.deserialize(halfAttention.serialize("production-half-attention"),
              "production-half-attention",err),err.c_str());
        halfResumed.advance_ticks(48);
        check(halfResumed.schedule_book().work_present_ticks_by_establishment.at({worker,site})==32&&
              halfResumed.schedule_book().work_effort_attention_by_establishment.at({worker,site})==16000&&
              halfResumed.person(worker).body_state.exertion==1&&
              halfResumed.establishments()[0].stock[size_t(GoodId::Cloth)]==1&&
              halfResumed.establishments()[0].processed_work_units.at(worker)==1&&halfResumed.invariant(),
              "half-rate attention across save/load produces exactly one recipe batch");

        // A competing session can use all attention. Physical Work attendance
        // still accrues for the wage ledger, but it cannot create output.
        Realm noWorkAttention=seeded;
        auto fullFocus=noWorkAttention.execute(ScheduleCommand{worker,
            noWorkAttention.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
            0,0,32,1,1000});
        check(fullFocus.success,"full-capacity Generic session accepted");
        noWorkAttention.advance_ticks(64);
        check(noWorkAttention.schedule_book().work_present_ticks_by_establishment.at({worker,site})==32&&
              noWorkAttention.schedule_book().work_effort_attention_by_establishment.empty()&&
              noWorkAttention.person(worker).body_state.exertion==0&&
              noWorkAttention.establishments()[0].stock[size_t(GoodId::Cloth)]==0&&
              noWorkAttention.establishments()[0].processed_work_units.empty()&&noWorkAttention.invariant(),
              "full Work presence with no attention delivered to Work creates no product");

        Realm pausedEffort=seeded;
        check(pausedEffort.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success,
              "effort pause fixture pauses clock");
        bool effortPauseRejected=false;
        try{pausedEffort.advance_ticks(64);}catch(const std::runtime_error&){effortPauseRejected=true;}
        check(effortPauseRejected&&pausedEffort.person(worker).body_state.exertion==0&&
              pausedEffort.person(worker).body_state.exertion_effort_attention_processed==0,
              "paused game time adds neither exertion nor consumed-attention watermark");
        check(pausedEffort.execute(RealmClockCommand{RealmClockCommand::Operation::Resume}).success,
              "effort pause fixture resumes clock");
        pausedEffort.advance_ticks(64);
        check(pausedEffort.person(worker).body_state.exertion==2&&pausedEffort.invariant(),
              "resumed Work applies its attention once when the game day completes");
        auto requireBodyLoad=[&](Realm& destination,const std::string& data,const char* bundle,const char* label){
            const bool ok=destination.deserialize(data,bundle,err);
            if(!ok){const std::string diagnostic=std::string(label)+": "+err;check(false,diagnostic.c_str());}
            check(ok,label);
        };

        Realm effortRemainder=seeded;
        check(effortRemainder.execute(ScheduleCommand{worker,
              effortRemainder.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
              0,0,32,1,750}).success,"three-quarter Generic session leaves one-quarter attention for Work");
        effortRemainder.advance_ticks(64);
        check(effortRemainder.schedule_book().work_effort_attention_by_establishment.at({worker,site})==8000&&
              effortRemainder.person(worker).body_state.exertion==0&&
              effortRemainder.person(worker).body_state.exertion_effort_attention_remainder==8000,
              "8,000 Work attention units preserve a half-point exertion remainder");
        Realm effortRemainderReloaded(2,1,128);
        requireBodyLoad(effortRemainderReloaded,effortRemainder.serialize("exertion-effort-remainder"),
                        "exertion-effort-remainder","partial effort save reloads");
        check(effortRemainderReloaded.execute(ScheduleCommand{worker,
              effortRemainderReloaded.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
              0,64,96,0,1000,ActivityKind::Work,region,site}).success,
              "second-day Work session accepted after restoring exertion remainder");
        check(effortRemainderReloaded.execute(ScheduleCommand{worker,
              effortRemainderReloaded.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
              0,64,96,1,750}).success,"second-day attention competition accepted");
        effortRemainderReloaded.advance_ticks(64);
        check(effortRemainderReloaded.person(worker).body_state.exertion==1&&
              effortRemainderReloaded.person(worker).body_state.exertion_effort_attention_remainder==0&&
              effortRemainderReloaded.invariant(),
              "effort remainder survives save/load and combines with the next day without duplication");

        auto withBodyRow=[](const Realm& source,uint64_t actor,int sleepDebt,int exertion,
                            int effortRemainder,int restRemainder,int64_t restBegin,
                            int64_t restEnd,int restRegion,bool restPending,const char* bundle){
            std::string save=source.serialize(bundle);
            const auto empty=save.find(" BODY_3 0 ");
            if(empty==std::string::npos)throw std::runtime_error("body fixture expected an empty BODY_3 section");
            std::ostringstream row;
            row<<" BODY_3 1 "<<actor<<' '<<sleepDebt<<' '<<exertion<<" 0 0 "
               <<effortRemainder<<' '<<restRemainder<<' '<<restBegin<<' '<<restEnd<<' '
               <<restRegion<<' '<<restPending<<' ';
            save.replace(empty,std::string(" BODY_3 0 ").size(),row.str());
            return save;
        };
        Realm saturatedBase=seeded;
        const auto saturatedSave=withBodyRow(saturatedBase,worker,0,99,5000,0,0,0,-1,false,
                                              "exertion-upper-clamp");
        Realm saturated(2,1,128);
        requireBodyLoad(saturated,saturatedSave,"exertion-upper-clamp","upper exertion clamp save loads");
        saturated.advance_ticks(64);
        check(saturated.person(worker).body_state.exertion==100&&
              saturated.person(worker).body_state.exertion_effort_attention_remainder==0&&
              saturated.invariant(),
              "effort that reaches the upper bound clears the partial point beyond 100");

        Realm recoveryBase=Realm::scenario(61,129,30,50,0,100,30,30,100,50);
        const int recoveryRegion=recoveryBase.person(129).region;
        auto partialRestSave=withBodyRow(recoveryBase,129,0,2,0,0,0,8,recoveryRegion,true,
                                          "exertion-partial-rest");
        Realm partialRecovery(1,1,128);
        requireBodyLoad(partialRecovery,partialRestSave,"exertion-partial-rest","partial Rest save loads");
        partialRecovery.advance_ticks(64);
        check(partialRecovery.person(129).body_state.exertion==2&&
              partialRecovery.person(129).body_state.exertion_rest_tick_remainder==8,
              "eight actual Rest ticks save as a half-point recovery remainder");
        Realm partialRecoveryReloaded(1,1,128);
        requireBodyLoad(partialRecoveryReloaded,partialRecovery.serialize("exertion-partial-rest"),
                        "exertion-partial-rest","partial Rest remainder survives save/load");
        std::vector<std::string> restTokens;
        {std::istringstream in(partialRecoveryReloaded.serialize("exertion-partial-rest"));
         std::string token;while(in>>token)restTokens.push_back(token);}
        const size_t bodyAt=std::find(restTokens.begin(),restTokens.end(),"BODY_3")-restTokens.begin();
        const size_t bodyCount=std::stoull(restTokens.at(bodyAt+1));size_t bodyStart=restTokens.size();
        for(size_t row=0;row<bodyCount;++row){const size_t candidate=bodyAt+2+row*11;
            if(std::stoull(restTokens.at(candidate))==129){bodyStart=candidate;break;}}
        check(bodyStart<restTokens.size(),"partial Rest continuation locates the saved actor body row");
        restTokens[bodyStart+7]="64";restTokens[bodyStart+8]="72";
        restTokens[bodyStart+9]=std::to_string(recoveryRegion);restTokens[bodyStart+10]="1";
        std::ostringstream nextRestSave;
        for(const auto& token:restTokens)nextRestSave<<token<<' ';
        Realm nextRestDay(1,1,128);
        requireBodyLoad(nextRestDay,nextRestSave.str(),"exertion-partial-rest","next-day Rest interval loads");
        nextRestDay.advance_ticks(64);
        check(nextRestDay.person(129).body_state.exertion==1&&
              nextRestDay.person(129).body_state.exertion_rest_tick_remainder==0&&
              nextRestDay.invariant(),
              "saved eight-tick Rest remainder combines with the next actual Rest interval after reload");

        auto lowerRestSave=withBodyRow(recoveryBase,129,0,1,0,15,0,64,recoveryRegion,true,
                                        "exertion-lower-clamp");
        Realm lowerExertion(1,1,128);
        requireBodyLoad(lowerExertion,lowerRestSave,"exertion-lower-clamp","lower exertion clamp save loads");
        lowerExertion.advance_ticks(64);
        check(lowerExertion.person(129).body_state.exertion==0&&
              lowerExertion.person(129).body_state.exertion_rest_tick_remainder==0&&
              lowerExertion.invariant(),
              "Rest recovery at zero clears the partial recovery remainder");

        // An older save has the processed output and presence but no attention
        // field. Migration seeds only the attention already represented by
        // the processed watermark; an empty next day cannot replay production.
        const std::string schema29=test_fixtures::save_as_v29(afterProduction);
        Realm migrated29(2,1,128);
        requireBodyLoad(migrated29,schema29,"production-integration","schema 29 body migration loads");
        check(migrated29.person(worker).body_state.exertion==2&&
              migrated29.person(worker).body_state.exertion_effort_attention_processed==32000&&
              migrated29.invariant(),
              "schema 29 migration anchors the body watermark to saved Work attention without resetting exertion");
        migrated29.advance_ticks(64);
        check(migrated29.person(worker).body_state.exertion==2&&migrated29.invariant(),
              "schema 29 migration does not count pre-save attention as new exertion");

        const std::string schema28=test_fixtures::save_as_v28(afterProduction);
        Realm migrated28(2,1,128);
        const bool migration28ok=migrated28.deserialize(schema28,"production-integration",err);
        check(migration28ok,err.c_str());
        check(migrated28.schedule_book().work_effort_attention_by_establishment.at({worker,site})==32000&&
              migrated28.establishments()[0].stock[size_t(GoodId::Cloth)]==1&&
              migrated28.person(worker).body_state.exertion_effort_attention_processed==32000&&
              migrated28.person(worker).body_state.exertion==2,
              "schema 28 migration seeds exactly the attention represented by two saved work units");
        migrated28.advance_ticks(64);
        check(migrated28.establishments()[0].stock[size_t(GoodId::Cloth)]==1&&
              migrated28.person(worker).body_state.exertion==2&&migrated28.invariant(),
              "schema 28 migration does not repeat historical production or add old exertion again");

        // The same Realm command/time/save path must execute the other two
        // published starter recipes, not just the Cloth recipe above.
        struct RecipeCase {
            uint64_t seed;
            EstablishmentIndustry industry;
            std::array<GoodId,3> inputs;
            size_t input_count;
            GoodId output;
            const char* name;
        };
        constexpr std::array<RecipeCase,2> remainingRecipes{{
            {53,EstablishmentIndustry::ToolWorkshop,{GoodId::Iron,GoodId::Copper,GoodId::Fuel},3,
             GoodId::Tools,"Tools"},
            {55,EstablishmentIndustry::MilitarySupplyWorkshop,{GoodId::Tools,GoodId::Cloth,GoodId::Fuel},2,
             GoodId::MilitarySupply,"MilitarySupply"}
        }};
        for(const auto& recipe:remainingRecipes){
            Realm recipeBase(recipe.seed,2,128);
            const uint64_t recipeWorker=65;
            RealmObservationTestAccess::mutable_person(recipeBase,recipeWorker).skill=100;
            const int recipeRegion=recipeBase.person(recipeWorker).region;
            auto created=recipeBase.execute(EstablishmentCommand{recipeWorker,
                recipeBase.person(recipeWorker).decisions+1,EstablishmentOwnerKind::Person,
                recipeWorker,recipeRegion,recipe.industry,1,1});
            check(created.success,"secondary recipe establishment created");
            test_fixtures::authorize_work(recipeBase,created.session,recipeWorker);
            check(recipeBase.execute(ScheduleCommand{recipeWorker,recipeBase.person(recipeWorker).decisions+1,
                ScheduleCommand::Operation::Submit,0,0,32,0,1000,ActivityKind::Work,
                recipeRegion,created.session}).success,"secondary recipe Work scheduled");

            std::vector<std::string> recipeTokens;
            {std::istringstream in(recipeBase.serialize("secondary-recipe"));
             std::string token;while(in>>token)recipeTokens.push_back(token);}
            const size_t establishAt=std::find(recipeTokens.begin(),recipeTokens.end(),"ESTABLISH_14")-
                                     recipeTokens.begin();
            check(establishAt<recipeTokens.size(),"secondary recipe save locates establishment stock");
            const size_t establishmentRecord=establishAt+2;
            for(size_t input=0;input<recipe.input_count;++input){
                const size_t inputIndex=size_t(recipe.inputs[input]);
                recipeTokens[establishmentRecord+7+inputIndex]="1";
                recipeTokens[18+inputIndex]=std::to_string(std::stoll(recipeTokens[18+inputIndex])+1);
            }
            std::ostringstream recipeSeededText;
            for(size_t i=0;i<recipeTokens.size();++i){
                if(i)recipeSeededText<<' ';
                recipeSeededText<<recipeTokens[i];
            }
            Realm recipeSeeded(2,1,128);std::string recipeError;
            check(recipeSeeded.deserialize(recipeSeededText.str(),"secondary-recipe",recipeError),
                  recipeError.c_str());
            check(recipeSeeded.invariant(),"secondary recipe opening stock is conserved");

            Realm recipeUninterrupted=recipeSeeded;
            Realm recipeSplit=recipeSeeded;
            recipeUninterrupted.advance_ticks(64);
            recipeSplit.advance_ticks(16);
            Realm recipeResumed(2,1,128);
            check(recipeResumed.deserialize(recipeSplit.serialize("secondary-recipe"),
                  "secondary-recipe",recipeError),recipeError.c_str());
            recipeResumed.advance_ticks(48);
            check(recipeUninterrupted.fingerprint()==recipeResumed.fingerprint(),
                  "secondary recipe save/load continues like uninterrupted time");
            const auto completedSite=recipeResumed.establishments()[0];
            check(std::none_of(recipe.inputs.begin(),recipe.inputs.begin()+std::ptrdiff_t(recipe.input_count),
                      [&](GoodId input){return completedSite.stock[size_t(input)]!=0;})&&
                  completedSite.stock[size_t(recipe.output)]==1,
                  "secondary recipe consumes exact inputs and creates one output");
            check(completedSite.processed_work_units.at(recipeWorker)==2&&
                  completedSite.last_production_day==1&&recipeResumed.invariant(),
                  "secondary recipe records completed work and conserves the world");
            const std::string completedSave=recipeResumed.serialize("secondary-recipe");
            Realm recipeRoundTrip(2,1,128);
            check(recipeRoundTrip.deserialize(completedSave,"secondary-recipe",recipeError)&&
                  recipeRoundTrip.serialize("secondary-recipe")==completedSave&&
                  recipeRoundTrip.invariant(),"secondary recipe output survives exact save/load");

            const int64_t totalMoneyBeforeSale=recipeRoundTrip.total_money();
            const int64_t actorCashBeforeSale=recipeRoundTrip.person(recipeWorker).cash;
            const int64_t localStockBeforeSale=recipeRoundTrip.view().provinces[size_t(recipeRegion)].stock[
                size_t(recipe.output)];
            const int64_t outputPrice=recipeRoundTrip.view().provinces[size_t(recipeRegion)].price[
                size_t(recipe.output)];
            const int64_t expectedSaleProceeds=std::max<int64_t>(1,outputPrice-1);
            auto withdrawn=recipeRoundTrip.execute(EstablishmentTransferCommand{recipeWorker,
                recipeRoundTrip.person(recipeWorker).decisions+1,created.session,
                EstablishmentTransferOperation::WithdrawGoods,int(recipe.output),1});
            check(withdrawn.success&&recipeRoundTrip.person(recipeWorker).inventory[size_t(recipe.output)]==1&&
                  recipeRoundTrip.establishments()[0].stock[size_t(recipe.output)]==0&&
                  recipeRoundTrip.invariant(),"recipe output transfers from workshop to its owner");
            auto sold=recipeRoundTrip.execute({recipeWorker,recipeRoundTrip.person(recipeWorker).decisions+1,
                {Intent::Sell,recipeRegion,int(recipe.output),1}});
            check(sold.success&&recipeRoundTrip.person(recipeWorker).inventory[size_t(recipe.output)]==0&&
                  recipeRoundTrip.view().provinces[size_t(recipeRegion)].stock[size_t(recipe.output)]==
                      localStockBeforeSale+1&&
                  recipeRoundTrip.person(recipeWorker).cash==actorCashBeforeSale+expectedSaleProceeds&&
                  recipeRoundTrip.total_money()==totalMoneyBeforeSale&&recipeRoundTrip.invariant(),
                  "recipe output sells through the local Realm market and conserves money");
        }

        // A missing companion input must not consume the available half of a
        // recipe. Work is still recorded as processed under the existing
        // shortage-forfeit rule.
        Realm shortageBase(59,2,128);
        constexpr uint64_t shortageWorker=65;
        RealmObservationTestAccess::mutable_person(shortageBase,shortageWorker).skill=100;
        const int shortageRegion=shortageBase.person(shortageWorker).region;
        auto shortageSite=shortageBase.execute(EstablishmentCommand{shortageWorker,
            shortageBase.person(shortageWorker).decisions+1,EstablishmentOwnerKind::Person,
            shortageWorker,shortageRegion,EstablishmentIndustry::ToolWorkshop,1,1});
        check(shortageSite.success,"shortage recipe private tool workshop created");
        test_fixtures::authorize_work(shortageBase,shortageSite.session,shortageWorker);
        check(shortageBase.execute(ScheduleCommand{shortageWorker,
            shortageBase.person(shortageWorker).decisions+1,ScheduleCommand::Operation::Submit,
            0,0,32,0,1000,ActivityKind::Work,shortageRegion,shortageSite.session}).success,
            "shortage recipe Work scheduled");
        std::vector<std::string> shortageTokens;
        {std::istringstream in(shortageBase.serialize("recipe-shortage"));
         std::string token;while(in>>token)shortageTokens.push_back(token);}
        const size_t shortageEstablishAt=std::find(shortageTokens.begin(),shortageTokens.end(),"ESTABLISH_14")-
                                         shortageTokens.begin();
        check(shortageEstablishAt<shortageTokens.size(),"shortage save locates establishment stock");
        const size_t shortageRecord=shortageEstablishAt+2;
        const size_t ironIndex=size_t(GoodId::Iron);
        shortageTokens[shortageRecord+7+ironIndex]="1";
        shortageTokens[18+ironIndex]=std::to_string(std::stoll(shortageTokens[18+ironIndex])+1);
        std::ostringstream shortageText;
        for(size_t i=0;i<shortageTokens.size();++i){
            if(i)shortageText<<' ';
            shortageText<<shortageTokens[i];
        }
        Realm shortageWorld(2,1,128);std::string shortageError;
        check(shortageWorld.deserialize(shortageText.str(),"recipe-shortage",shortageError),
              shortageError.c_str());
        shortageWorld.advance_ticks(64);
        const auto shortageCompleted=shortageWorld.establishments()[0];
        check(shortageCompleted.stock[ironIndex]==1&&
              shortageCompleted.stock[size_t(GoodId::Fuel)]==0&&
              shortageCompleted.stock[size_t(GoodId::Tools)]==0,
              "missing Fuel prevents partial Iron consumption and Tools output");
        check(shortageCompleted.processed_work_units.at(shortageWorker)==2&&
              shortageCompleted.last_production_day==1&&shortageWorld.invariant(),
              "input shortage records considered work and preserves Realm invariant");

        Realm coworkers=seeded;
        uint64_t secondWorker=0;
        for(uint64_t candidate=1;candidate<=uint64_t(coworkers.count());++candidate)
            if(candidate!=worker&&coworkers.person(candidate).alive&&coworkers.person(candidate).region==region){secondWorker=candidate;break;}
        check(secondWorker!=0,"production fixture has a second coworker");
        RealmObservationTestAccess::mutable_person(coworkers,secondWorker).skill=50;
        test_fixtures::authorize_work(coworkers,site,secondWorker);
        check(coworkers.execute(ScheduleCommand{secondWorker,coworkers.person(secondWorker).decisions+1,
            ScheduleCommand::Operation::Submit,0,0,16,0,1000,ActivityKind::Work,region,site}).success,
            "coworker submits a shorter shift");
        coworkers.advance_ticks(64);
        check(coworkers.establishments()[0].processed_work_units.at(worker)==2&&
              coworkers.establishments()[0].processed_work_units.at(secondWorker)==1,
              "coworkers retain different processed work totals");
        Realm coworkersLoaded(2,1,128);
        const auto coworkersSave=coworkers.serialize("production-coworkers");
        check(coworkersLoaded.deserialize(coworkersSave,"production-coworkers",err)&&
              coworkersLoaded.serialize("production-coworkers")==coworkersSave,
              "save with unequal coworker watermarks roundtrips");

        // Schemas 12 and 13 had site attendance but no production markers.
        // Migration baselines the already-earned units so loading cannot
        // retroactively produce them at the next day boundary.
        Realm preProduction=seeded;
        preProduction.advance_ticks(32);
        const std::string preV14=preProduction.serialize("production-integration");
        for(const auto& migratedText:{downgrade_save_to_v13(preV14),
                                       downgrade_empty_goods_save_to_v12(preV14)}){
            Realm migrated(2,1,128);
            check(migrated.deserialize(migratedText,"production-integration",err),err.c_str());
            check(migrated.establishments()[0].processed_work_units.at(worker)==2&&
                  migrated.establishments()[0].last_production_day==0,
                  "v12/v13 migration baselines saved site attendance");
            migrated.advance_ticks(32);
            check(migrated.establishments()[0].stock[size_t(GoodId::Cloth)]==0&&migrated.invariant(),
                  "v12/v13 migration prevents retroactive output");
        }

        std::vector<std::string> corruptTokens;{
            std::istringstream in(afterProduction);std::string token;while(in>>token)corruptTokens.push_back(token);
        }
        const size_t savedTag=std::find(corruptTokens.begin(),corruptTokens.end(),"ESTABLISH_14")-corruptTokens.begin();
        const size_t savedRecord=savedTag+2;
        check(corruptTokens[savedRecord+35]=="1","production save contains one actor watermark");
        corruptTokens[savedRecord+37]="3";
        std::ostringstream corruptSave;for(size_t i=0;i<corruptTokens.size();++i){if(i)corruptSave<<' ';corruptSave<<corruptTokens[i];}
        const auto beforeReject=afterLoad.fingerprint();
        check(!afterLoad.deserialize(corruptSave.str(),"production-integration",err)&&
              afterLoad.fingerprint()==beforeReject,
              "malformed establishment work watermark rejects without replacing destination");
    }
    {
        // Legacy daily goods remain only for worlds that have not entered the
        // command/timeline economy. The employment counters still rebuild in
        // timeline mode; only the unbacked product creation is suppressed.
        Realm legacyPeople(63,2,128),timelinePeople=legacyPeople;
        RealmObservationTestAccess::set_legacy_worker(legacyPeople,2);
        RealmObservationTestAccess::set_legacy_worker(timelinePeople,2);
        RealmObservationTestAccess::set_timeline_contract(timelinePeople,true);
        const auto legacyBefore=RealmObservationTestAccess::produced(legacyPeople);
        const auto timelineBefore=RealmObservationTestAccess::produced(timelinePeople);
        RealmObservationTestAccess::run_people_phase(legacyPeople);
        RealmObservationTestAccess::run_people_phase(timelinePeople);
        int64_t legacyDailyOutput=0,timelineDailyOutput=0;
        for(size_t good=0;good<kGoodIdCount;++good){
            legacyDailyOutput+=RealmObservationTestAccess::produced(legacyPeople)[good]-legacyBefore[good];
            timelineDailyOutput+=RealmObservationTestAccess::produced(timelinePeople)[good]-timelineBefore[good];
        }
        check(legacyDailyOutput>0,"timeline-off people phase retains legacy daily product source");
        check(timelineDailyOutput==0,"timeline people phase does not create products from region or job labels");
        check(std::string(legacyPeople.model_contract())=="realm-household-fund-goods30-config-v54-atlas-north-pass-v1-salted-meal-v1",
              "timeline-off model contract remains compatible");
        check(std::string(timelinePeople.model_contract())=="realm-worksite-payroll-goods30-config-v54-atlas-north-pass-v1-salted-meal-v1",
              "timeline model contract versions changed production sources");
    }
    {
        // Freeze usable inputs immediately before the monthly compatibility
        // pilot, then compare matching Cloth/Tools sites independently.
        Realm day29(63,2,128);day29.advance(29);
        std::vector<std::string> tokens;{
            std::istringstream in(day29.serialize("legacy-production"));
            std::string token;while(in>>token)tokens.push_back(token);
        }
        size_t peopleAt=0;
        for(size_t candidate=100;candidate<tokens.size();++candidate){
            size_t count=0;try{count=size_t(std::stoull(tokens[candidate]));}catch(...){continue;}
            if(count<128||count>1000000||candidate+1+56*count>tokens.size())continue;
            bool dense=true;for(size_t row=0;row<count;++row)
                if(tokens[candidate+1+59*row]!=std::to_string(row+1)){dense=false;break;}
            if(dense){peopleAt=candidate;break;}
        }
        check(peopleAt!=0,"legacy production fixture locates Person section");
        const size_t people=size_t(std::stoull(tokens[peopleAt]));
        const size_t provinceCountAt=peopleAt+1+people*56;
        const size_t provinceStart=provinceCountAt+1;
        for(int region:{0,1})for(GoodId product:{GoodId::Fiber,GoodId::Iron,GoodId::Fuel}){
            const size_t productId=size_t(product);
            const size_t slot=provinceStart+size_t(region)*66+12+productId;
            tokens[slot]=std::to_string(std::stoll(tokens[slot])+10);
            tokens[18+productId]=std::to_string(std::stoll(tokens[18+productId])+10);
        }
        std::ostringstream seededSave;for(size_t i=0;i<tokens.size();++i){if(i)seededSave<<' ';seededSave<<tokens[i];}
        Realm seeded(2,1,128);std::string err;
        check(seeded.deserialize(seededSave.str(),"legacy-production",err),err.c_str());
        auto createSite=[](Realm& world,int region,EstablishmentIndustry industry){
            const uint64_t actor=region==world.regions()?1:uint64_t(region+2);
            auto result=world.execute(EstablishmentCommand{actor,world.person(actor).decisions+1,
                EstablishmentOwnerKind::Public,uint64_t(region),region,industry,1,1});
            if(!result.success)throw std::runtime_error("legacy compatibility site: "+result.reason);
        };
        auto enterTimeline=[](Realm& world){
            const int64_t tick=world.game_tick();
            ScheduleCommand command{};
            command.actor=1;
            command.sequence=world.person(1).decisions+1;
            command.operation=ScheduleCommand::Operation::Submit;
            command.begin=tick;
            command.end=tick+1;
            command.demand_rate=activity::kMaxDemandRate;
            command.kind=ActivityKind::Generic;
            command.region=-1;
            auto result=world.execute(command);
            if(!result.success)throw std::runtime_error("legacy compatibility timeline: "+result.reason);
        };
        Realm oldControl=seeded,oldWithSites=seeded;
        createSite(oldWithSites,0,EstablishmentIndustry::ClothWorkshop);
        createSite(oldWithSites,1,EstablishmentIndustry::ToolWorkshop);
        check(!oldWithSites.timeline_contract(),"site creation alone keeps legacy timeline mode");
        oldControl.advance();oldWithSites.advance();
        for(int region=0;region<=oldControl.regions();++region)
            for(GoodId product:{GoodId::Cloth,GoodId::Tools})
                check(oldControl.view().provinces[size_t(region)].stock[size_t(product)]==
                      oldWithSites.view().provinces[size_t(region)].stock[size_t(product)],
                      "timeline-off empty-site legacy pilot replay is unchanged");

        Realm newControl=seeded,newWithSites=seeded;
        createSite(newWithSites,0,EstablishmentIndustry::ClothWorkshop);
        createSite(newWithSites,1,EstablishmentIndustry::ToolWorkshop);
        enterTimeline(newControl);enterTimeline(newWithSites);
        newControl.advance();newWithSites.advance();
        const auto controlAfter=newControl.view(),sitesAfter=newWithSites.view();
        for(int region=0;region<=newControl.regions();++region)
            for(GoodId product:{GoodId::Cloth,GoodId::Tools})
                check(sitesAfter.provinces[size_t(region)].stock[size_t(product)]==
                      controlAfter.provinces[size_t(region)].stock[size_t(product)],
                      "timeline world has no province-level Cloth/Tools output with or without a matching site");
        check(newControl.invariant()&&newWithSites.invariant(),
              "timeline production-source suppression preserves goods conservation");
    }
    auto start=std::chrono::steady_clock::now();for(int day=0;day<90;++day){for(auto id:w.due(Family::Civilian)){auto mask=w.mask(id);int a=mask[1+w.person(id).region]?1+w.person(id).region:0;w.execute({id,w.person(id).decisions+1,w.candidates(id)[a]});}w.advance();check(w.invariant(),"profile A accounting");}
    std::cout<<"REALM TESTS: "<<checks<<" checks passed; profile A10000 persons, "<<w.relations().size()<<" sparse edges;90days "<<std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()<<"s\n";return 0;
}catch(const std::exception&e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}
