#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <iostream>
#include <stdexcept>

using namespace polyec;

namespace polyec {
struct RealmMunicipalCouncilTestAccess {
    static void prepare_housing(Realm& world,int region) {
        world.provinces_[size_t(region)].treasury-=kUrbanHousingProjectBudget;
        world.municipal_funds_[size_t(region)]+=kUrbanHousingProjectBudget;
        const size_t material=size_t(GoodId::ConstructionMaterial);
        const size_t tools=size_t(GoodId::Tools);
        world.provinces_[size_t(region)].stock[material]+=4;
        world.provinces_[size_t(region)].stock[tools]+=2;
        world.opening_[material]+=4;
        world.opening_[tools]+=2;
        world.urban_housing_capacity_[size_t(region)]=0;
        world.refresh_demography();
    }
    static void set_day(Realm& world,int day){world.day_=day;}
    static void set_health(Realm& world,uint64_t person,int health){world.mutable_person(person).health=health;}
};
} // namespace polyec

namespace {
int checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}

uint64_t find_urban_adult(const Realm& world,uint64_t except=0){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.alive&&person.destination<0&&person.residence_kind==ResidenceKind::Urban&&
           person.age_days>=18*365&&id!=except)return id;
    }
    return 0;
}
}

int main(){try{
    Realm world(20261024,26,256);
    const uint64_t nominee=find_urban_adult(world,1);
    check(nominee!=0,"Atlas opening contains an adult Urban resident who can volunteer");
    const int site=world.person(nominee).region;
    const auto money_before=world.total_money();
    const auto nominate=world.candidates(nominee)[size_t(realm_council_nomination_candidate)];
    check(nominate.kind==Intent::NominateCouncil&&world.mask(nominee)[size_t(realm_council_nomination_candidate)]==1.f,
          "eligible resident receives a self-nomination command for their own settlement");
    check(Realm::action_name(nominate).find("Tự ứng cử đại diện hội đồng tại ")==0,
          "resident sees that nomination is voluntary and tied to the named settlement");
    check(world.execute({nominee,world.person(nominee).decisions+1,nominate}).success,
          "resident can volunteer without a fee or change to ownership");
    auto councils=world.municipal_councils();
    check(councils[size_t(site)].nominee==nominee&&councils[size_t(site)].nomination_day==world.view(1).day,
          "save-owned council record stores the nominee and nomination day");
    check(world.total_money()==money_before,"volunteering does not create or transfer money");

    const auto sovereign_candidates=world.candidates(1);
    const int reject_slot=realm_council_reject_candidate_begin+site;
    check(sovereign_candidates[size_t(reject_slot)].kind==Intent::ResolveCouncilNomination&&
          world.mask(1)[size_t(reject_slot)]==1.f,"Sovereign can refuse the nomination for that settlement");
    check(Realm::action_name(sovereign_candidates[size_t(reject_slot)]).find("Từ chối hồ sơ ứng cử tại ")==0,
          "Sovereign sees a settlement-specific refusal instead of an unexplained generic rejection");
    check(world.execute({1,world.person(1).decisions+1,sovereign_candidates[size_t(reject_slot)]}).success,
          "refusal clears the pending nomination");
    check(world.municipal_councils()[size_t(site)].nominee==0,"refused seat can receive a new volunteer");

    const auto renominate=world.candidates(nominee)[size_t(realm_council_nomination_candidate)];
    check(world.execute({nominee,world.person(nominee).decisions+1,renominate}).success,
          "resident may volunteer again after refusal");
    const auto accept_candidates=world.candidates(1);
    const int accept_slot=realm_council_accept_candidate_begin+site;
    check(accept_candidates[size_t(accept_slot)].kind==Intent::ResolveCouncilNomination&&
          world.mask(1)[size_t(accept_slot)]==1.f,"Sovereign can accept only the pending resident of the chosen settlement");
    check(world.execute({1,world.person(1).decisions+1,accept_candidates[size_t(accept_slot)]}).success,
          "acceptance appoints the volunteer for a four-year trial term");
    councils=world.municipal_councils();
    check(councils[size_t(site)].representative==nominee&&
          councils[size_t(site)].term_end_day==world.view(1).day+4*365&&councils[size_t(site)].nominee==0,
          "acceptance records one representative and an exact game-day end date");

    RealmMunicipalCouncilTestAccess::prepare_housing(world,site);
    const auto representative_actions=world.candidates(nominee);
    const auto housing=representative_actions[281];
    check(housing.kind==Intent::Project&&housing.good==4&&world.mask(nominee)[281]==1.f,
          "current representative can submit the existing public-housing project command");
    const auto outsider=find_urban_adult(world,nominee);
    check(outsider!=0&&world.mask(outsider)[281]==0.f,
          "another resident cannot spend the council fund without appointment");
    check(world.execute({nominee,world.person(nominee).decisions+1,housing}).success,
          "representative starts the housing project through the normal command pipeline");
    check(world.municipal_funds()[size_t(site)]==0,"housing debit comes from this council's municipal fund");
    check(world.invariant(),"council appointment and authorized housing preserve world invariants");

    const auto saved=world.serialize("municipal-council-trial-1");
    check(saved.starts_with("POLYEC_REALM_67 ")&&saved.find(" MUNICIPAL_COUNCIL_1 27 ")!=std::string::npos,
          "schema67 save writes all 27 council identities and their current representatives");
    Realm loaded(1,1,128);std::string error;
    check(loaded.deserialize(saved,"municipal-council-trial-1",error),error.c_str());
    check(loaded.municipal_councils()[size_t(site)]==world.municipal_councils()[size_t(site)]&&
          loaded.invariant(),"save/load retains the representative and council invariants");
    auto schema64=saved;
    test_fixtures::downgrade_save_66_to_65(schema64);
    const size_t council_section=schema64.rfind(" MUNICIPAL_COUNCIL_1 ");
    check(council_section!=std::string::npos,"current save has an isolated final council section");
    schema64.resize(council_section);
    schema64.replace(0,schema64.find(' '),"POLYEC_REALM_64");
    Realm migrated(1,1,128);error.clear();
    check(migrated.deserialize(schema64,"municipal-council-trial-1",error),error.c_str());
    check(migrated.municipal_councils()[size_t(site)].representative==0&&
          migrated.municipal_funds()[size_t(site)]==world.municipal_funds()[size_t(site)]&&migrated.invariant(),
          "schema64 migration creates empty councils and preserves existing public fund balances");
    auto corrupt=saved;
    const size_t count_marker=corrupt.rfind("MUNICIPAL_COUNCIL_1 27 ");
    corrupt.replace(count_marker,std::string("MUNICIPAL_COUNCIL_1 27 ").size(),"MUNICIPAL_COUNCIL_1 26 ");
    const auto old_fingerprint=loaded.fingerprint();error.clear();
    check(!loaded.deserialize(corrupt,"municipal-council-trial-1",error)&&loaded.fingerprint()==old_fingerprint,
          "malformed council save is rejected without changing the open Realm");

    const auto resign=world.candidates(nominee)[31];
    check(resign.kind==Intent::Resign&&world.mask(nominee)[31]==1.f,
          "representative receives the shared resignation action");
    check(world.execute({nominee,world.person(nominee).decisions+1,resign}).success&&
          world.municipal_councils()[size_t(site)].representative==0,
          "resignation immediately removes council spending authority");
    uint64_t second=find_urban_adult(world,nominee);
    while(second&&world.person(second).region==site)second=find_urban_adult(world,second);
    check(second!=0,"another settlement has an independent resident who can volunteer");
    const int second_site=world.person(second).region;
    const auto second_nomination=world.candidates(second)[size_t(realm_council_nomination_candidate)];
    check(world.execute({second,world.person(second).decisions+1,second_nomination}).success,
          "independent settlement accepts its own nomination");
    const auto second_accept=world.candidates(1)[size_t(realm_council_accept_candidate_begin+second_site)];
    check(world.execute({1,world.person(1).decisions+1,second_accept}).success,
          "Sovereign can appoint a representative in a different settlement");
    const int term_end=world.municipal_councils()[size_t(second_site)].term_end_day;
    RealmMunicipalCouncilTestAccess::set_day(world,term_end);
    const auto expired_mask=world.mask(second);
    check(expired_mask[281]==0.f&&expired_mask[size_t(realm_council_nomination_candidate)]==1.f,
          "housing authority ends on the term end day and the seat opens for a new volunteer");

    const auto renew=world.candidates(second)[size_t(realm_council_nomination_candidate)];
    check(world.execute({second,world.person(second).decisions+1,renew}).success,
          "expired representative may volunteer again after the previous term ends");
    const auto renew_accept=world.candidates(1)[size_t(realm_council_accept_candidate_begin+second_site)];
    check(world.execute({1,world.person(1).decisions+1,renew_accept}).success,
          "Sovereign may appoint a fresh term after expiry");
    int migration_target=-1;
    for(int target=0;target<int(kAtlasTerritoryCount);++target){
        if(target==second_site||world.travel_days(second_site,target)<=0)continue;
        const auto census=world.urban_census();
        if(census[size_t(target)].available_beds>0){migration_target=target;break;}
    }
    check(migration_target>=0,"another reachable settlement has an unreserved urban bed");
    const int travel=world.travel_days(second_site,migration_target);
    const auto migration_result=world.execute({second,world.person(second).decisions+1,
        {Intent::Migrate,migration_target,0,10,travel+1}});
    check(migration_result.success&&world.municipal_councils()[size_t(second_site)].representative==0,
          "accepting migration immediately removes the representative's authority at the old settlement");

    Realm death_world(20261025,26,256);
    const uint64_t dying=find_urban_adult(death_world);
    check(dying!=0,"another settlement contains a living adult resident for the death lifecycle check");
    const int dying_site=death_world.person(dying).urban_site_id;
    const auto death_nomination=death_world.candidates(dying)[size_t(realm_council_nomination_candidate)];
    check(death_world.execute({dying,death_world.person(dying).decisions+1,death_nomination}).success,
          "adult resident can volunteer for a different settlement");
    const auto death_appointment=death_world.candidates(1)[size_t(realm_council_accept_candidate_begin+dying_site)];
    check(death_world.execute({1,death_world.person(1).decisions+1,death_appointment}).success,
          "Sovereign appoints the second resident before the death check");
    RealmMunicipalCouncilTestAccess::set_health(death_world,dying,0);
    death_world.advance();
    check(!death_world.person(dying).alive&&death_world.municipal_councils()[size_t(dying_site)].representative==0,
          "the daily death lifecycle clears the dead representative and council seat");
    check(death_world.invariant(),"death cleanup retains valid council references");
    check(checks==41,"municipal council focused scenarios all ran");
    std::cout<<"municipal council: "<<checks<<" checks passed\n";
    return 0;
}catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';return 1;}}
