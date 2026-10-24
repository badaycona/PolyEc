#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmObservationTestAccess {
    static void give_supply(Realm& world,uint64_t person,int64_t amount){
        world.mutable_person(person).inventory[size_t(GoodId::MilitarySupply)]=amount;
        world.opening_[size_t(GoodId::MilitarySupply)]+=amount;
    }
    static void give_leather(Realm& world,uint64_t person,int64_t amount){
        world.mutable_person(person).inventory[size_t(GoodId::Leather)]=amount;
        world.opening_[size_t(GoodId::Leather)]+=amount;
    }
    static void prepare_routes(Realm& world){for(auto& route:world.national_transport_routes_)route.open=true;}
    static void ensure_port_salt(Realm& world,int64_t amount){
        auto& stock=world.provinces_.at(size_t(kMilitaryRaidTargetRegion)).stock[size_t(GoodId::Salt)];
        if(stock>=amount)return;
        const auto added=amount-stock;
        stock=amount;world.opening_[size_t(GoodId::Salt)]+=added;
    }
};
}

namespace {
int checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
Candidate offer_for(const Realm& world,uint64_t governor){
    const auto candidates=world.candidates(governor);const auto mask=world.mask(governor,candidates);
    for(int slot=realm_military_service_candidate_begin;
        slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot)
        if(mask[size_t(slot)]&&candidates[size_t(slot)].kind==Intent::Offer)return candidates[size_t(slot)];
    throw std::runtime_error("Atlas region 8 has no eligible volunteer");
}
void act(Realm& world,uint64_t actor,Candidate candidate){
    const auto result=world.execute(RealmCommand{actor,world.person(actor).decisions+1,candidate});
    if(!result.success)throw std::runtime_error("expected setup action to succeed: "+result.reason);
    if(!world.invariant())throw std::runtime_error("Realm invariant failed during battle fixture setup");
}
uint64_t prepare_volunteer(Realm& world){
    constexpr uint64_t sovereign=1,governor=10;
    if(!world.person(governor).alive||world.person(governor).family!=Family::Governance||world.person(governor).region!=8)
        throw std::runtime_error("seed fixture no longer places a Governance actor in Nam Cang");
    const uint64_t volunteer=uint64_t(offer_for(world,governor).target);
    act(world,governor,offer_for(world,governor));
    world.advance(10);
    const auto response=world.candidates(volunteer)[28];
    if(response.kind!=Intent::Accept)throw std::runtime_error("volunteer lacks a ready service proposal");
    act(world,volunteer,response);
    world.advance(1);
    auto consent=world.execute(MilitaryCombatConsentCommand{volunteer,world.person(volunteer).decisions+1,true});
    if(!consent.success)throw std::runtime_error("volunteer consent failed: "+consent.reason);
    act(world,governor,Candidate{Intent::Mobilize,8,0,40,30});
    if(!world.invariant())throw std::runtime_error("Realm invariant failed after volunteer mobilization");
    (void)sovereign;
    return volunteer;
}
std::vector<uint64_t> prepare_two_volunteers(Realm& world){
    constexpr uint64_t governor=10;
    if(!world.person(governor).alive||world.person(governor).family!=Family::Governance||world.person(governor).region!=8)
        throw std::runtime_error("seed fixture no longer places a Governance actor in Nam Cang");
    const auto candidates=world.candidates(governor);const auto mask=world.mask(governor,candidates);
    std::vector<uint64_t> volunteers;
    std::vector<Candidate> offers;
    for(int slot=realm_military_service_candidate_begin;
        slot<realm_military_service_candidate_begin+realm_military_service_candidate_count&&offers.size()<2;++slot)
        if(mask[size_t(slot)]&&candidates[size_t(slot)].kind==Intent::Offer){
            const uint64_t target=uint64_t(candidates[size_t(slot)].target);
            if(std::find(volunteers.begin(),volunteers.end(),target)==volunteers.end()){
                volunteers.push_back(target);offers.push_back(candidates[size_t(slot)]);
            }
        }
    if(offers.size()!=2)throw std::runtime_error("seed fixture needs two eligible volunteers in Nam Cang");
    for(const auto& offer:offers)act(world,governor,offer);
    world.advance(10);
    for(const auto volunteer:volunteers){
        const auto response=world.candidates(volunteer)[28];
        if(response.kind!=Intent::Accept)throw std::runtime_error("a second volunteer lacks a ready service proposal");
        act(world,volunteer,response);
    }
    world.advance(1);
    for(const auto volunteer:volunteers){
        const auto result=world.execute(MilitaryCombatConsentCommand{volunteer,world.person(volunteer).decisions+1,true});
        if(!result.success)throw std::runtime_error("second volunteer consent failed: "+result.reason);
    }
    act(world,governor,Candidate{Intent::Mobilize,8,0,40,30});
    if(!world.invariant())throw std::runtime_error("Realm invariant failed after two-volunteer mobilization");
    return volunteers;
}
}

int main(){try{
    Realm world(9201,26,128);RealmObservationTestAccess::prepare_routes(world);
    constexpr uint64_t sovereign=1;
    const uint64_t volunteer=prepare_volunteer(world);
    const auto initialPartner=world.foreign_trade_partners()[0];
    const auto invalidFingerprint=world.fingerprint();
    check(!world.execute(MilitaryRaidCommand{0,initialPartner.decision_sequence+1,kMilitaryRaidUnitMaximum+1}).success&&
          world.fingerprint()==invalidFingerprint,
          "an external power cannot send an over-limit force or partially spend its ledger");
    const auto launched=world.execute(MilitaryRaidCommand{0,initialPartner.decision_sequence+1,8});
    check(launched.success&&world.military_raids().size()==1&&world.military_raids()[0].status==MilitaryRaidStatus::Pending&&
          world.foreign_trade_partners()[0].money==initialPartner.money-8*kMilitaryRaidCashPerUnit&&
          world.foreign_trade_partners()[0].goods[size_t(GoodId::DryFood)]==initialPartner.goods[size_t(GoodId::DryFood)]-8&&
          world.invariant(),
          "a named external power pays the fixed cash and food cost for eight units and creates one pending raid against Nam Cang");
    const auto pendingFingerprint=world.fingerprint();
    check(!world.execute(MilitaryRaidCommand{1,world.foreign_trade_partners()[1].decision_sequence+1,1}).success&&
          world.fingerprint()==pendingFingerprint,
          "a second external power cannot start a concurrent raid while the first awaits a response");
    const auto pendingSave=world.serialize("coastal-raid-battle-v1");
    Realm resumed(1,1,128);std::string error;
    if(!resumed.deserialize(pendingSave,"coastal-raid-battle-v1",error))
        throw std::runtime_error("pending raid load rejected: "+error);
    check(
          resumed.military_raids()==world.military_raids()&&
          resumed.serialize("coastal-raid-battle-v1")==pendingSave,
          "save/load preserves the exact pending raid and its named opponent");
    auto schema65Save=pendingSave;test_fixtures::downgrade_save_66_to_65(schema65Save);
    Realm schema65Resume(1,1,128);
    check(schema65Resume.deserialize(schema65Save,"coastal-raid-battle-v1",error)&&
          schema65Resume.military_raids()==world.military_raids()&&
          schema65Resume.serialize("coastal-raid-battle-v1").rfind("POLYEC_REALM_73 ",0)==0,
          "schema65 raid data migrates without a supply-use list and the next save writes schema70");
    auto legacySave=pendingSave;test_fixtures::downgrade_save_60_to_59(legacySave);
    Realm legacy(1,1,128);
    check(legacy.deserialize(legacySave,"coastal-raid-battle-v1",error)&&legacy.military_raids().empty()&&legacy.invariant(),
          "a schema59 save without the new raid record still loads with no invented conflict");

    Realm closedPort(9204,26,128);RealmObservationTestAccess::prepare_routes(closedPort);
    const auto importRoute=std::find_if(closedPort.national_transport_routes().begin(),closedPort.national_transport_routes().end(),
        [](const NationalTransportRoute& route){return route.from==9&&route.to==8;});
    const auto routeIndex=uint8_t(importRoute-closedPort.national_transport_routes().begin());
    check(importRoute!=closedPort.national_transport_routes().end()&&
          closedPort.execute(TransportRouteCommand{sovereign,closedPort.person(sovereign).decisions+1,routeIndex,false}).success,
          "the fixture can close the Nam Cang coastal corridor through the public route command");
    const auto closedPortFingerprint=closedPort.fingerprint();
    check(!closedPort.execute(MilitaryRaidCommand{0,closedPort.foreign_trade_partners()[0].decision_sequence+1,1}).success&&
          closedPort.fingerprint()==closedPortFingerprint,
          "a raid is rejected without spending resources while the coastal corridor is closed");

    const int military=world.capability_value(volunteer,LegacyCapability::Military);
    const int weapon=world.person(volunteer).inventory[size_t(GoodId::Weapon)]>0?1:0;
    RealmObservationTestAccess::give_supply(world,volunteer,1);
    const int defenseScore=1+military/50+weapon+1;
    const auto staleSequence=world.person(sovereign).decisions+1;
    const auto unauthorizedFingerprint=world.fingerprint();
    check(!world.execute(MilitaryBattleCommand{10,world.person(10).decisions+1,launched.session,false,{volunteer}}).success&&
          world.fingerprint()==unauthorizedFingerprint,
          "a regional Governor cannot decide the Sovereign's response to a raid");
    const auto unauthorized=world.fingerprint();
    check(!world.execute(MilitaryBattleCommand{sovereign,staleSequence,launched.session,false,{volunteer,volunteer}}).success&&
          world.fingerprint()==unauthorized,
          "duplicate defender IDs are rejected without changing the pending battle");
    const auto revoked=world.execute(MilitaryCombatConsentCommand{volunteer,world.person(volunteer).decisions+1,false});
    check(revoked.success,"the volunteer can withdraw combat consent before the Sovereign responds");
    const auto revokedFingerprint=world.fingerprint();
    check(!world.execute(MilitaryBattleCommand{sovereign,staleSequence,launched.session,false,{volunteer}}).success&&
          world.fingerprint()==revokedFingerprint,
          "the battle command rechecks current consent and rejects a volunteer who withdrew it");
    const auto restoredConsent=world.execute(MilitaryCombatConsentCommand{volunteer,world.person(volunteer).decisions+1,true});
    check(restoredConsent.success,"the volunteer can give fresh consent before the battle is resolved");
    const auto resolved=world.execute(MilitaryBattleCommand{sovereign,staleSequence,launched.session,false,{volunteer}});
    check(resolved.success&&world.military_raids()[0].status==MilitaryRaidStatus::Successful&&
          world.military_raids()[0].defense_score==defenseScore&&
          world.military_raids()[0].supplied_defenders==std::vector<uint64_t>{volunteer}&&
          world.person(volunteer).inventory[size_t(GoodId::MilitarySupply)]==0&&
          world.military_raids()[0].defender_casualties==1&&world.person(volunteer).health==0&&
          world.invariant(),
          "when the one volunteer cannot exceed the four-unit force, the raid succeeds and records a casualty");
    world.advance(1);
    check(!world.person(volunteer).alive&&world.invariant(),
          "the next normal day processes a battle casualty through the existing death and estate rules");
    const auto resolvedSave=world.serialize("coastal-raid-battle-v1");
    Realm resolvedResume(1,1,128);
    check(resolvedResume.deserialize(resolvedSave,"coastal-raid-battle-v1",error)&&
          resolvedResume.military_raids()==world.military_raids()&&resolvedResume.invariant(),
          "resolved raid outcome, casualty and death survive a save/load roundtrip");

    Realm defendedWorld(9203,26,128);RealmObservationTestAccess::prepare_routes(defendedWorld);
    const auto defenders=prepare_two_volunteers(defendedWorld);
    const auto defendedRaid=defendedWorld.execute(MilitaryRaidCommand{1,
        defendedWorld.foreign_trade_partners()[1].decision_sequence+1,1});
    check(defendedRaid.success,"a second external power can attack a different save world");
    const auto repelled=defendedWorld.execute(MilitaryBattleCommand{sovereign,
        defendedWorld.person(sovereign).decisions+1,defendedRaid.session,false,defenders});
    check(repelled.success&&defendedWorld.military_raids()[0].status==MilitaryRaidStatus::Repelled&&
          defendedWorld.military_raids()[0].defense_score>1&&
          defendedWorld.military_raids()[0].supplied_defenders.empty()&&
          defendedWorld.military_raids()[0].attacker_losses==1&&
          defendedWorld.military_raids()[0].defender_casualties==0&&
          std::all_of(defenders.begin(),defenders.end(),[&](uint64_t id){return defendedWorld.person(id).health>0;})&&
          defendedWorld.invariant(),
          "two consenting mobilized defenders beat a one-unit raid without losing health or Salt");

    Realm retreatWorld(9202,26,128);RealmObservationTestAccess::prepare_routes(retreatWorld);
    RealmObservationTestAccess::ensure_port_salt(retreatWorld,1);
    const auto retreatPartner=retreatWorld.foreign_trade_partners()[2];
    const auto retreatRaid=retreatWorld.execute(MilitaryRaidCommand{2,retreatPartner.decision_sequence+1,1});
    check(retreatRaid.success,"a second named external power can start a raid after loading a separate world");
    const auto retreat=retreatWorld.execute(MilitaryBattleCommand{sovereign,
        retreatWorld.person(sovereign).decisions+1,retreatRaid.session,true,{}});
    check(retreat.success&&retreatWorld.military_raids()[0].status==MilitaryRaidStatus::Withdrawn&&
          retreatWorld.military_raids()[0].salt_looted==1&&retreatWorld.invariant(),
          "retreat leaves volunteers unharmed and transfers one available Salt to the raiding partner");

    Realm armoredWorld(9205,26,128),unarmoredWorld(9205,26,128);
    RealmObservationTestAccess::prepare_routes(armoredWorld);
    RealmObservationTestAccess::prepare_routes(unarmoredWorld);
    const uint64_t armoredVolunteer=prepare_volunteer(armoredWorld);
    const uint64_t unarmoredVolunteer=prepare_volunteer(unarmoredWorld);
    check(armoredVolunteer==unarmoredVolunteer,
          "paired armor test worlds select the same volunteer from the same seed");
    const int baselineDefense=1+armoredWorld.capability_value(armoredVolunteer,LegacyCapability::Military)/50+
        (armoredWorld.person(armoredVolunteer).inventory[size_t(GoodId::Weapon)]>0?1:0);
    const auto armoredRaid=armoredWorld.execute(MilitaryRaidCommand{0,
        armoredWorld.foreign_trade_partners()[0].decision_sequence+1,baselineDefense});
    const auto unarmoredRaid=unarmoredWorld.execute(MilitaryRaidCommand{0,
        unarmoredWorld.foreign_trade_partners()[0].decision_sequence+1,baselineDefense});
    check(armoredRaid.success&&unarmoredRaid.success&&baselineDefense>=1&&baselineDefense<=kMilitaryRaidUnitMaximum,
          "paired raid forces are valid and equal the volunteer's defense before armor");
    RealmObservationTestAccess::give_leather(armoredWorld,armoredVolunteer,2);
    RealmObservationTestAccess::give_leather(unarmoredWorld,unarmoredVolunteer==2?3:2,1);
    check(armoredWorld.invariant()&&unarmoredWorld.invariant(),
          "personal and unrelated Leather inventory are included in their source ledgers");
    const std::string armoredPending=armoredWorld.serialize("leather-armor-battle-v1");
    Realm armoredResume(1,1,128);
    check(armoredResume.deserialize(armoredPending,"leather-armor-battle-v1",error)&&
          armoredResume.person(armoredVolunteer).inventory[size_t(GoodId::Leather)]==2&&
          armoredResume.military_raids()==armoredWorld.military_raids(),
          "save/load during a pending raid preserves the defender's personally owned Leather");
    const auto armoredResult=armoredResume.execute(MilitaryBattleCommand{sovereign,
        armoredResume.person(sovereign).decisions+1,armoredRaid.session,false,{armoredVolunteer}});
    const auto unarmoredResult=unarmoredWorld.execute(MilitaryBattleCommand{sovereign,
        unarmoredWorld.person(sovereign).decisions+1,unarmoredRaid.session,false,{unarmoredVolunteer}});
    check(armoredResult.success&&unarmoredResult.success&&
          armoredResume.military_raids()[0].defense_score==baselineDefense+1&&
          unarmoredWorld.military_raids()[0].defense_score==baselineDefense&&
          armoredResume.military_raids()[0].status==MilitaryRaidStatus::Repelled&&
          unarmoredWorld.military_raids()[0].status==MilitaryRaidStatus::Successful,
          "one personal Leather adds one defense and flips an equal-strength paired raid from breach to repelled");
    check(armoredResume.person(armoredVolunteer).inventory[size_t(GoodId::Leather)]==2&&
          unarmoredWorld.person(unarmoredVolunteer).inventory[size_t(GoodId::Leather)]==0&&
          armoredResume.person(armoredVolunteer).health>0&&unarmoredWorld.person(unarmoredVolunteer).health==0&&
          armoredResume.invariant()&&unarmoredWorld.invariant(),
          "durable armor remains with its owner, prevents the paired casualty, and no other person's Leather is borrowed");
    std::cout<<"Military battle checks passed: "<<checks<<"\n";return 0;
}catch(const std::exception& error){std::cerr<<"Military battle failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}
