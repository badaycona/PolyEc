#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace polyec;

namespace {
int checks=0;
void check(bool condition,const char* message){
    ++checks;
    if(!condition)throw std::runtime_error(message);
}
uint64_t adult_civilian_at(Realm& world,int region,uint64_t exclude=0){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(id!=exclude&&person.alive&&person.family==Family::Civilian&&person.region==region&&person.age_days>=6*365)
            return id;
    }
    throw std::runtime_error("fixture has no eligible civilian in requested region");
}
std::string as_schema43_with_28_funds(const std::string& current){
    std::string schema47=current;
    if(schema47.rfind("POLYEC_REALM_67 ",0)==0)test_fixtures::downgrade_save_55_to_54(schema47);
    if(schema47.rfind("POLYEC_REALM_54 ",0)==0)test_fixtures::downgrade_save_49_to_48(schema47);
    if(schema47.rfind("POLYEC_REALM_48 ",0)==0)test_fixtures::downgrade_save_48_to_47(schema47);
    std::istringstream input(schema47);std::vector<std::string> tokens;std::string token;
    while(input>>token)tokens.push_back(token);
    constexpr size_t organization_begin=2+9+7+3*kGoodIdCount+7*7;
    constexpr size_t old_count=28;
    if(tokens.empty()||tokens[0]!="POLYEC_REALM_47"||
       tokens.size()<organization_begin+kOrganizationCount+1)
        throw std::runtime_error("current save has unexpected primary header layout");
    const auto shipment_section=std::find(tokens.begin(),tokens.end(),"TRANSPORT_SHIPMENT_2");
    if(shipment_section==tokens.end())throw std::runtime_error("current save has no public transport section");
    const size_t shipment_at=size_t(shipment_section-tokens.begin());
    if(std::stoull(tokens.at(shipment_at+3))!=0||std::stoull(tokens.at(shipment_at+4))!=0)
        throw std::runtime_error("schema43 fixture conversion requires no active shipments");
    tokens[shipment_at]="TRANSPORT_SHIPMENT_1";
    const auto service_section=std::find(tokens.begin(),tokens.end(),"MILITARY_SERVICE_1"); if(service_section!=tokens.end())tokens.erase(service_section,tokens.end());
    tokens[0]="POLYEC_REALM_43";
    tokens.erase(tokens.begin()+static_cast<std::ptrdiff_t>(organization_begin+old_count),
                 tokens.begin()+static_cast<std::ptrdiff_t>(organization_begin+kOrganizationCount));
    std::ostringstream output;
    for(const auto& value:tokens)output<<value<<' ';
    return output.str();
}
}

int main(){
    try{
        static_assert(kAtlasTerritoryCount==27);
        static_assert(kOrganizationGroupsPerTerritory==4);
        static_assert(kOrganizationCount==108);
        Realm world(49501,26,256);
        const auto member=adult_civilian_at(world,12);
        const auto outsider=adult_civilian_at(world,12,member);
        const auto opening_money=world.total_money();
        auto joined=world.execute({member,world.person(member).decisions+1,{Intent::Join,12,0,2,30}});
        check(joined.success,"resident can use the existing Join command in region12");
        check(world.person(member).organization==49,"region12 group0 has stable organization ID49");
        check(world.total_money()==opening_money&&world.invariant(),"joining transfers the existing fee without creating or losing money");
        const auto joined_observation=world.observation(member);
        const auto outsider_observation=world.observation(outsider);
        check(outsider_observation[95]==0.0f&&outsider_observation[96]==0.0f,
              "a non-member observation does not reveal the organization's ID or balance");
        check(std::abs(joined_observation[95]-49.0f/108.0f)<1e-6f,"organization observation is normalized over the full ID range");
        check(std::abs(joined_observation[96]-0.002f)<1e-6f,"organization observation reports the member's actual fund balance");
        const auto view=world.view(member);
        for(int region=0;region<kAtlasTerritoryCount;++region){
            for(size_t legacy=0;legacy<kLegacyGoodCount;++legacy){
                const size_t offset=region<7
                    ?size_t(24+region*int(kLegacyGoodCount))+legacy
                    :realm_base_state+size_t(region-7)*kLegacyGoodCount+legacy;
                const size_t good=size_t(canonical_from_legacy_good(int(legacy)));
                check(std::abs(joined_observation[offset]-float(view.provinces[size_t(region)].price[good])/
                               float(kGoodReferencePrice[good]))<1e-6f,
                      "each territory price stays in its designated observation slot");
            }
        }
        const auto candidates=world.candidates(member);
        check(std::abs(joined_observation[realm_state]-float(int(candidates[0].kind))/32.0f)<1e-6f,
              "action feature block begins after all 27 territory states");
        check(int(joined_observation.size())==5182&&realm_state==382,"full geography observation also carries 54 site-specific council decisions");

        const auto after_join=world.total_money();
        const auto created=world.execute(EstablishmentCommand{member,world.person(member).decisions+1,
            EstablishmentOwnerKind::Organization,49,12,EstablishmentIndustry::ClothWorkshop,1,1});
        check(created.success,"member can spend organization funds under the existing establishment rule");
        check(world.establishments().size()==1&&world.establishments()[0].owner_id==49,"site records the exact organization owner");
        check(world.total_money()==after_join&&world.invariant(),"organization-funded site preserves total money");
        const auto after_member_action=world.fingerprint();
        const auto rejected=world.execute(EstablishmentCommand{outsider,world.person(outsider).decisions+1,
            EstablishmentOwnerKind::Organization,49,12,EstablishmentIndustry::ClothWorkshop,1,1});
        check(!rejected.success&&rejected.reason=="Actor not organization member","non-member cannot spend the region12 organization account");
        check(world.fingerprint()==after_member_action&&world.invariant(),"rejected non-member command makes no partial change");
        const auto wrong_region=world.execute(EstablishmentCommand{member,world.person(member).decisions+1,
            EstablishmentOwnerKind::Organization,49,13,EstablishmentIndustry::ClothWorkshop,1,1});
        check(!wrong_region.success&&wrong_region.reason=="Organization home region mismatch","organization account cannot fund a site in another region");
        check(world.fingerprint()==after_member_action,"wrong-region reject leaves money and world unchanged");

        const auto save=world.serialize("organization-territories-v1");
        check(save.rfind("POLYEC_REALM_67 ",0)==0,"current save writes schema67");
        Realm restored(1,1,128);std::string error;
        check(restored.deserialize(save,"organization-territories-v1",error),error.c_str());
        check(restored.person(member).organization==49&&restored.establishments().size()==1&&restored.invariant(),"schema45 round trip keeps region12 membership and owned site");
        check(std::abs(restored.observation(member)[96]-world.observation(member)[96])<1e-6f,"schema45 round trip keeps the organization balance");

        Realm legacy(177,6,256);
        const auto old_member=adult_civilian_at(legacy,6);
        const auto legacy_join=legacy.execute({old_member,legacy.person(old_member).decisions+1,{Intent::Join,6,0,2,30}});
        check(legacy_join.success&&legacy.person(old_member).organization==25,"legacy-range group remains usable");
        const auto old_save=as_schema43_with_28_funds(legacy.serialize("organization-territories-v1"));
        Realm migrated(1,1,128);
        check(migrated.deserialize(old_save,"organization-territories-v1",error),error.c_str());
        check(migrated.person(old_member).organization==25&&migrated.invariant(),"schema43 migration preserves existing membership");
        check(std::abs(migrated.observation(old_member)[96]-legacy.observation(old_member)[96])<1e-6f,"schema43 migration keeps old account funds");
        check(migrated.serialize("organization-territories-v1").rfind("POLYEC_REALM_67 ",0)==0,"migrated world writes current schema67 on next save");

        Realm upper_bound(49503,26,256);
        const auto upper_member=adult_civilian_at(upper_bound,26);
        const auto upper_join=upper_bound.execute({upper_member,upper_bound.person(upper_member).decisions+1,
            {Intent::Join,26,3,2,30}});
        check(upper_join.success&&upper_bound.person(upper_member).organization==108,
              "last territory and group produce the inclusive organization ID108 boundary");
        check(std::abs(upper_bound.observation(upper_member)[95]-1.0f)<1e-6f&&upper_bound.invariant(),
              "maximum organization ID remains normalized and invariant-valid");

        std::cout<<"Organization territory checks="<<checks<<"\n";
        return 0;
    }catch(const std::exception&error){
        std::cerr<<"Organization territory test failed after "<<checks<<" checks: "<<error.what()<<"\n";
        return 1;
    }
}
