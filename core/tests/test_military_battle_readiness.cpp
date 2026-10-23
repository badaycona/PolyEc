#include "polyec/realm.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace polyec;

namespace {
int checks=0;
void check(bool condition,const char* message){
    ++checks;
    if(!condition)throw std::runtime_error(message);
}
Candidate service_offer(const Realm& world,uint64_t governor){
    const auto choices=world.candidates(governor);
    for(int i=realm_military_service_candidate_begin;
        i<realm_military_service_candidate_begin+realm_military_service_candidate_count;++i)
        if(choices[size_t(i)].kind==Intent::Offer)return choices[size_t(i)];
    return {};
}
}

int main(){
    try{
        constexpr uint64_t seed=9201,governor=2;
        Realm world(seed,6,128);
        const auto offer=service_offer(world,governor);
        check(offer.kind==Intent::Offer,"fixture must expose a voluntary service offer");
        const auto offered=world.execute({governor,world.person(governor).decisions+1,offer});
        check(offered.success,"governor must be able to offer service through the normal command pipeline");
        const auto volunteer=uint64_t(offer.target);
        world.advance(10);
        const auto accepted=world.execute({volunteer,world.person(volunteer).decisions+1,
            world.candidates(volunteer)[28]});
        check(accepted.success,"the named volunteer must accept their own service offer");
        world.advance(1);

        auto roster=world.military_unit_rosters();
        check(roster.size()==1&&roster[0].members==std::vector<uint64_t>{volunteer},
              "the accepted service record must create exactly one roster member");
        check(roster[0].combat_consented_members.empty(),
              "service membership alone must not count as combat consent");
        check(roster[0].battle_ready_members.empty(),
              "service membership alone must not count as battle readiness");
        check(world.execute(MilitaryCombatConsentCommand{
                  volunteer,world.person(volunteer).decisions+1,true}).success,
              "only the volunteer can grant their own combat consent");
        roster=world.military_unit_rosters();
        check(roster[0].combat_consented_members==std::vector<uint64_t>{volunteer}&&
              roster[0].battle_ready_members.empty(),
              "consent without an active muster window must not make the volunteer battle-ready");
        check(world.execute({governor,world.person(governor).decisions+1,
                  {Intent::Mobilize,world.person(governor).region,0,40,5}}).success,
              "the local governor must be able to open the existing muster window");
        roster=world.military_unit_rosters();
        check(roster[0].battle_ready_members==std::vector<uint64_t>{volunteer},
              "a consenting volunteer in an active muster window must appear as available to a battle command");

        check(world.execute(MilitaryCombatConsentCommand{
                  volunteer,world.person(volunteer).decisions+1,false}).success,
              "the volunteer must be able to withdraw consent before a battle starts");
        roster=world.military_unit_rosters();
        check(roster[0].combat_consented_members.empty()&&roster[0].battle_ready_members.empty(),
              "withdrawn consent must immediately remove combat eligibility");
        check(world.execute(MilitaryCombatConsentCommand{
                  volunteer,world.person(volunteer).decisions+1,true}).success,
              "the volunteer may explicitly consent again before a battle starts");

        const auto saved=world.serialize("military-battle-readiness");
        Realm restored(1,1,128);
        std::string error;
        check(restored.deserialize(saved,"military-battle-readiness",error),
              "the existing save format must restore service, consent and muster source records");
        const auto restored_rosters=restored.military_unit_rosters();
        check(restored_rosters.size()==1&&
              restored_rosters[0].members==std::vector<uint64_t>{volunteer}&&
              restored_rosters[0].combat_consented_members==std::vector<uint64_t>{volunteer}&&
              restored_rosters[0].battle_ready_members==std::vector<uint64_t>{volunteer}&&
              restored.invariant(),
              "save/load must derive the same combat-ready volunteers without a duplicated roster save section");
        check(world.execute(MilitaryCombatConsentCommand{
                  volunteer,world.person(volunteer).decisions+1,false}).success&&
              world.military_unit_rosters()[0].battle_ready_members.empty(),
              "a later withdrawal must also remove the volunteer from the derived battle-ready view");

        std::cout<<checks<<" military battle-readiness checks passed; seed="<<seed
                 <<"; schema="<<realm_schema<<"\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"military battle-readiness failure after "<<checks<<" checks: "<<error.what()<<"\n";
        return 1;
    }
}
