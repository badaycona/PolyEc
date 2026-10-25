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
        ensure_salt(world,kMilitaryRaidTargetRegion,amount);
    }
    static void ensure_salt(Realm& world,int region,int64_t amount){
        auto& stock=world.provinces_.at(size_t(region)).stock[size_t(GoodId::Salt)];
        if(stock>=amount)return;
        const auto added=amount-stock;
        stock=amount;world.opening_[size_t(GoodId::Salt)]+=added;
    }
    static void spend_partner_dry_food_for_fixture(Realm& world,int partner_id,int64_t amount){
        auto& partner=world.foreign_trade_partners_.at(size_t(partner_id));
        auto& stock=partner.goods[size_t(GoodId::DryFood)];
        if(amount<0||stock<amount||world.consumed_[size_t(GoodId::DryFood)]>
           std::numeric_limits<int64_t>::max()-amount)throw std::runtime_error("invalid controlled occupation-supply shortage fixture");
        stock-=amount;world.consumed_[size_t(GoodId::DryFood)]+=amount;
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
    throw std::runtime_error("Atlas region has no eligible volunteer");
}
void act(Realm& world,uint64_t actor,Candidate candidate){
    const auto result=world.execute(RealmCommand{actor,world.person(actor).decisions+1,candidate});
    if(!result.success)throw std::runtime_error("expected setup action to succeed: "+result.reason);
    if(!world.invariant())throw std::runtime_error("Realm invariant failed during battle fixture setup");
}
uint64_t prepare_volunteer(Realm& world,int region=8){
    constexpr uint64_t sovereign=1;const uint64_t governor=uint64_t(region+2);
    if(!world.person(governor).alive||world.person(governor).family!=Family::Governance||world.person(governor).region!=region)
        throw std::runtime_error("seed fixture no longer places a Governance actor in the selected Atlas region");
    const uint64_t volunteer=uint64_t(offer_for(world,governor).target);
    act(world,governor,offer_for(world,governor));
    world.advance(10);
    const auto response=world.candidates(volunteer)[28];
    if(response.kind!=Intent::Accept)throw std::runtime_error("volunteer lacks a ready service proposal");
    act(world,volunteer,response);
    world.advance(1);
    auto consent=world.execute(MilitaryCombatConsentCommand{volunteer,world.person(volunteer).decisions+1,true});
    if(!consent.success)throw std::runtime_error("volunteer consent failed: "+consent.reason);
    act(world,governor,Candidate{Intent::Mobilize,region,0,40,30});
    if(!world.invariant())throw std::runtime_error("Realm invariant failed after volunteer mobilization");
    (void)sovereign;
    return volunteer;
}
std::vector<uint64_t> prepare_two_volunteers(Realm& world,int region=8){
    const uint64_t governor=uint64_t(region+2);
    if(!world.person(governor).alive||world.person(governor).family!=Family::Governance||world.person(governor).region!=region)
        throw std::runtime_error("seed fixture no longer places a Governance actor in the selected Atlas region");
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
    act(world,governor,Candidate{Intent::Mobilize,region,0,40,30});
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
          schema65Resume.serialize("coastal-raid-battle-v1").rfind("POLYEC_REALM_76 ",0)==0,
          "schema65 raid data migrates without a supply-use list and the next save writes current schema76");
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

    Realm multiFrontWorld(9206,26,128);RealmObservationTestAccess::prepare_routes(multiFrontWorld);
    const auto initialRoutes=multiFrontWorld.national_transport_routes();
    const auto target12Plan=NationalTransportManager::plan(0,8,12,2,TransportServiceGoal::Fastest,initialRoutes);
    check(target12Plan.reachable&&!target12Plan.route_indices.empty(),
          "the selected Atlas target is reachable from the landing port through open routes");
    std::array<int64_t,kAtlasTerritoryCount> feesByRegion{};
    for(const auto routeIndex:target12Plan.route_indices){
        const auto& route=initialRoutes[routeIndex];feesByRegion[size_t(route.fee_recipient_region)]+=route.fee_per_unit*2;
    }
    std::array<int64_t,kAtlasTerritoryCount> routeTreasuriesBefore{};
    for(size_t region=0;region<routeTreasuriesBefore.size();++region)
        routeTreasuriesBefore[region]=multiFrontWorld.view().provinces[region].treasury;
    const auto partner0=multiFrontWorld.foreign_trade_partners()[0];
    const auto attack12=multiFrontWorld.execute(MilitaryRaidCommand{0,partner0.decision_sequence+1,2,12});
    check(attack12.success&&multiFrontWorld.military_raids().back().target_region==12&&
          multiFrontWorld.foreign_trade_partners()[0].money==partner0.money-2*kMilitaryRaidCashPerUnit-target12Plan.total_fee&&
          multiFrontWorld.foreign_trade_partners()[0].goods[size_t(GoodId::DryFood)]==partner0.goods[size_t(GoodId::DryFood)]-2&&
          multiFrontWorld.invariant(),
          "an inland raid funds its force, rations and route fee and records the selected region");
    for(size_t region=0;region<feesByRegion.size();++region)
        if(feesByRegion[region]>0)check(multiFrontWorld.view().provinces[region].treasury==
            routeTreasuriesBefore[region]+feesByRegion[region],
            "each inland route fee is credited to its listed regional treasury exactly once");
    const auto partner1=multiFrontWorld.foreign_trade_partners()[1];
    const auto partner2=multiFrontWorld.foreign_trade_partners()[2];
    check(multiFrontWorld.execute(MilitaryRaidCommand{1,partner1.decision_sequence+1,1,13}).success&&
          multiFrontWorld.execute(MilitaryRaidCommand{2,partner2.decision_sequence+1,1,14}).success&&
          multiFrontWorld.military_raids().size()==3&&multiFrontWorld.invariant(),
          "three different external powers can keep three distinct Atlas fronts pending together");
    const auto threeFrontFingerprint=multiFrontWorld.fingerprint();
    check(!multiFrontWorld.execute(MilitaryRaidCommand{0,multiFrontWorld.foreign_trade_partners()[0].decision_sequence+1,1,15}).success&&
          multiFrontWorld.fingerprint()==threeFrontFingerprint&&
          !multiFrontWorld.execute(MilitaryRaidCommand{2,multiFrontWorld.foreign_trade_partners()[2].decision_sequence+1,1,12}).success&&
          multiFrontWorld.fingerprint()==threeFrontFingerprint,
          "a partner cannot open a second front and two attacks cannot target one region");
    const auto threeFrontSave=multiFrontWorld.serialize("atlas-multi-region-raid-fronts-v1");
    Realm threeFrontResume(1,1,128);
    check(threeFrontResume.deserialize(threeFrontSave,"atlas-multi-region-raid-fronts-v1",error)&&
          threeFrontResume.military_raids()==multiFrontWorld.military_raids()&&
          threeFrontResume.serialize("atlas-multi-region-raid-fronts-v1")==threeFrontSave&&threeFrontResume.invariant(),
          "save/load preserves all three attackers and each distinct target");
    const auto inlandSaltBefore=threeFrontResume.view().provinces[12].stock[size_t(GoodId::Salt)];
    RealmObservationTestAccess::ensure_salt(threeFrontResume,12,std::max<int64_t>(1,inlandSaltBefore));
    const int64_t inlandSaltFunded=threeFrontResume.view().provinces[12].stock[size_t(GoodId::Salt)];
    const auto inlandRaid=threeFrontResume.military_raids().front();
    const auto inlandPartnerSalt=threeFrontResume.foreign_trade_partners()[size_t(inlandRaid.partner_id)].goods[size_t(GoodId::Salt)];
    const auto inlandRetreat=threeFrontResume.execute(MilitaryBattleCommand{sovereign,
        threeFrontResume.person(sovereign).decisions+1,inlandRaid.id,true,{}});
    check(inlandRetreat.success&&threeFrontResume.military_raids().front().salt_looted==1&&
          threeFrontResume.view().provinces[12].stock[size_t(GoodId::Salt)]==inlandSaltFunded-1&&
          threeFrontResume.foreign_trade_partners()[size_t(inlandRaid.partner_id)].goods[size_t(GoodId::Salt)]==inlandPartnerSalt+1&&
          threeFrontResume.invariant(),
          "retreat transfers Salt from the attacked inland region to the attacking partner without creating stock");
    threeFrontResume.advance(1);
    const auto inlandResolvedSave=threeFrontResume.serialize("atlas-multi-region-raid-fronts-v1");
    Realm inlandResolvedResume(1,1,128);
    check(inlandResolvedResume.deserialize(inlandResolvedSave,"atlas-multi-region-raid-fronts-v1",error)&&
          inlandResolvedResume.military_raids().front()==threeFrontResume.military_raids().front()&&
          inlandResolvedResume.view().provinces[12].stock[size_t(GoodId::Salt)]==
              threeFrontResume.view().provinces[12].stock[size_t(GoodId::Salt)]&&inlandResolvedResume.invariant(),
          "an inland battle, normal day update and final save/load preserve its target, outcome and looted stock");

    Realm blockedInland(9207,26,128);RealmObservationTestAccess::prepare_routes(blockedInland);
    bool closedAnIncomingRoute=false;
    for(size_t index=0;index<blockedInland.national_transport_routes().size();++index){
        const auto route=blockedInland.national_transport_routes()[index];
        if(route.to==12){
            const auto result=blockedInland.execute(TransportRouteCommand{sovereign,
                blockedInland.person(sovereign).decisions+1,uint8_t(index),false});
            if(!result.success)throw std::runtime_error("could not close an incoming route for blocked-front test");
            closedAnIncomingRoute=true;
        }
    }
    const auto blockedFrontFingerprint=blockedInland.fingerprint();
    check(closedAnIncomingRoute&&
          !blockedInland.execute(MilitaryRaidCommand{0,blockedInland.foreign_trade_partners()[0].decision_sequence+1,1,12}).success&&
          blockedInland.fingerprint()==blockedFrontFingerprint,
          "a target with all incoming routes closed rejects the attack without charging the external partner");
    const auto invalidTargetFingerprint=blockedInland.fingerprint();
    check(!blockedInland.execute(MilitaryRaidCommand{0,blockedInland.foreign_trade_partners()[0].decision_sequence+1,1,27}).success&&
          blockedInland.fingerprint()==invalidTargetFingerprint,
          "a region ID beyond the Atlas is rejected atomically");

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

    Realm occupationWorld(9208,26,128);RealmObservationTestAccess::prepare_routes(occupationWorld);
    const auto localVolunteers=prepare_two_volunteers(occupationWorld,12);
    RealmObservationTestAccess::give_supply(occupationWorld,localVolunteers[1],1);
    const auto occupationPartner=occupationWorld.foreign_trade_partners()[0];
    const auto occupationRaid=occupationWorld.execute(MilitaryRaidCommand{0,
        occupationPartner.decision_sequence+1,kMilitaryRaidUnitMaximum,12});
    check(occupationRaid.success,"a foreign power can fund an occupation campaign against Atlas region 12");
    const auto occupationBattle=occupationWorld.execute(MilitaryBattleCommand{sovereign,
        occupationWorld.person(sovereign).decisions+1,occupationRaid.session,false,{localVolunteers[0]}});
    const auto initialOccupation=occupationWorld.military_occupations()[12];
    check(occupationBattle.success&&occupationWorld.military_raids().back().status==MilitaryRaidStatus::Successful&&
          initialOccupation.partner_id==0&&initialOccupation.garrison_units>0&&occupationWorld.invariant(),
          "winning the region 12 battle assigns a foreign military controller and retains only surviving attackers");
    const auto regionalGovernor=uint64_t(14);
    const auto taxBeforeReject=occupationWorld.fingerprint();
    const auto occupiedTax=occupationWorld.execute(RealmCommand{regionalGovernor,
        occupationWorld.person(regionalGovernor).decisions+1,Candidate{Intent::Tax,12,0,20}});
    check(!occupiedTax.success&&occupationWorld.fingerprint()==taxBeforeReject,
          "the regional Governor cannot change the tax rate while the region is under foreign military control");
    const auto occupiedEstablishmentFingerprint=occupationWorld.fingerprint();
    check(!occupationWorld.execute(EstablishmentCommand{regionalGovernor,
          occupationWorld.person(regionalGovernor).decisions+1,EstablishmentOwnerKind::Public,12,12,
          EstablishmentIndustry::ClothWorkshop,-1,100}).success&&
          occupationWorld.fingerprint()==occupiedEstablishmentFingerprint,
          "a regional Governor cannot use the public treasury to create a public workshop during occupation");
    const auto occupiedMunicipalFingerprint=occupationWorld.fingerprint();
    check(!occupationWorld.execute(MunicipalBudgetCommand{regionalGovernor,
          occupationWorld.person(regionalGovernor).decisions+1,12,1}).success&&
          occupationWorld.fingerprint()==occupiedMunicipalFingerprint,
          "a regional Governor cannot transfer money from the local treasury into the municipal fund during occupation");
    const auto occupiedRationFingerprint=occupationWorld.fingerprint();
    check(!occupationWorld.execute(MilitaryRationPurchaseCommand{regionalGovernor,
          occupationWorld.person(regionalGovernor).decisions+1,12,1,int(GoodId::DryFood)}).success&&
          occupationWorld.fingerprint()==occupiedRationFingerprint,
          "a regional Governor cannot draw from the public treasury to buy military rations during occupation");
    const auto occupiedRoute=std::find_if(occupationWorld.national_transport_routes().begin(),
        occupationWorld.national_transport_routes().end(),[](const NationalTransportRoute& route){return route.from==12||route.to==12;});
    if(occupiedRoute==occupationWorld.national_transport_routes().end())throw std::runtime_error("occupation fixture has no route touching region 12");
    const uint8_t occupiedRouteIndex=uint8_t(occupiedRoute-occupationWorld.national_transport_routes().begin());
    const auto routeBeforeReject=occupationWorld.fingerprint();
    check(!occupationWorld.execute(TransportRouteCommand{sovereign,occupationWorld.person(sovereign).decisions+1,
          occupiedRouteIndex,!occupiedRoute->open}).success&&occupationWorld.fingerprint()==routeBeforeReject,
          "the Sovereign cannot alter a national route whose endpoint is held by an external garrison");
    const auto occupationSave=occupationWorld.serialize("military-occupation-v1");
    check(occupationSave.rfind("POLYEC_REALM_76 ",0)==0&&occupationSave.find(" MILITARY_CONTROL_1 27 ")!=std::string::npos,
          "save76 stores all 27 region controllers and garrison sizes");
    Realm occupiedResume(1,1,128);
    check(occupiedResume.deserialize(occupationSave,"military-occupation-v1",error)&&
          occupiedResume.military_occupations()==occupationWorld.military_occupations()&&occupiedResume.invariant(),
          "save/load preserves the occupying partner, garrison size and blocked local authority");
    auto legacy75=occupationSave;
    legacy75.replace(0,std::string("POLYEC_REALM_76").size(),"POLYEC_REALM_75");
    Realm invalidLegacy(1,1,128);std::string legacyControlError;
    check(!invalidLegacy.deserialize(legacy75,"military-occupation-v1",legacyControlError),
          "a populated occupation section cannot be silently removed to imitate a legacy save");
    Realm legacyEmptyWorld(9209,26,128);auto legacyEmpty=legacyEmptyWorld.serialize("military-occupation-v1");
    test_fixtures::strip_empty_military_control_section(legacyEmpty);
    legacyEmpty.replace(0,std::string("POLYEC_REALM_76").size(),"POLYEC_REALM_75");
    Realm legacyEmptyResume(1,1,128);
    const bool loadedLegacyEmpty=legacyEmptyResume.deserialize(legacyEmpty,"military-occupation-v1",error);
    const auto legacyControllers=legacyEmptyResume.military_occupations();
    check(loadedLegacyEmpty&&
          std::none_of(legacyControllers.begin(),legacyControllers.end(),
              [](const MilitaryOccupation& occupation){return occupation.occupied();})&&legacyEmptyResume.invariant(),
          "save75 migrates with every region under the Realm's original military control");

    auto postCapturePartner=occupiedResume.foreign_trade_partners()[0];
    const int garrisonBeforeShortage=occupiedResume.military_occupations()[12].garrison_units;
    if(garrisonBeforeShortage<=1)throw std::runtime_error("occupation test requires a multi-unit captured garrison");
    RealmObservationTestAccess::spend_partner_dry_food_for_fixture(occupiedResume,0,
        postCapturePartner.goods[size_t(GoodId::DryFood)]-1);
    occupiedResume.advance(1);
    check(occupiedResume.military_occupations()[12].garrison_units==1&&
          occupiedResume.foreign_trade_partners()[0].goods[size_t(GoodId::DryFood)]==0&&occupiedResume.invariant(),
          "one ration supports one of the held garrison; the missing daily food removes the other units");
    const int counterattackScore=1+occupiedResume.capability_value(localVolunteers[1],LegacyCapability::Military)/50+
        int(occupiedResume.person(localVolunteers[1]).inventory[size_t(GoodId::Weapon)]>0)+
        int(occupiedResume.person(localVolunteers[1]).inventory[size_t(GoodId::Leather)]>0)+1;
    const auto recapture=occupiedResume.execute(MilitaryCounterattackCommand{sovereign,
        occupiedResume.person(sovereign).decisions+1,12,false,{localVolunteers[1]}});
    check(counterattackScore>1&&recapture.success&&!occupiedResume.region_military_occupied(12)&&
          occupiedResume.person(localVolunteers[1]).inventory[size_t(GoodId::MilitarySupply)]==0&&
          occupiedResume.invariant(),
          "a consented volunteer uses personal MilitarySupply to beat the one remaining garrison unit and restore local control");
    const auto recapturedSave=occupiedResume.serialize("military-occupation-v1");
    Realm recapturedResume(1,1,128);
    check(recapturedResume.deserialize(recapturedSave,"military-occupation-v1",error)&&
          !recapturedResume.region_military_occupied(12)&&recapturedResume.invariant(),
          "the recovered region remains under Realm control after a later save/load");
    std::cout<<"Military battle checks passed: "<<checks<<"\n";return 0;
}catch(const std::exception& error){std::cerr<<"Military battle failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}
