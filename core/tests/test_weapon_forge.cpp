#include "work_authorization_fixture.hpp"
#include "polyec/realm.hpp"
#include <iostream>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t find_person(const Realm& world,int region,Family family,bool adult=false){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& p=world.person(id);
        if(p.alive&&p.destination==-1&&p.region==region&&p.family==family&&(!adult||p.age_days>=14*365))return id;
    }
    return 0;
}
}
int main(){try{
    constexpr int region=19,iron=int(GoodId::Iron),tools=int(GoodId::Tools),weapon=int(GoodId::Weapon);
    Realm world(49501,26,256);
    const uint64_t governor=region+2,worker=find_person(world,region,Family::Civilian,true);
    const uint64_t buyer=find_person(world,region,Family::Economic,true);
    check(world.person(governor).taxing_authority&&worker&&buyer,"region 19 has a public authority, worker and buyer");
    const auto candidates=world.candidates(governor);
    check(realm_weapon_forge_candidate_begin==170&&realm_actions==400&&realm_obs==5182,
          "Weapon Forge choices append after existing actions and match observation size");
    for(int tier=0;tier<3;++tier)
        check(candidates[size_t(170+tier)].kind==Intent::CreateEstablishment&&
              candidates[size_t(170+tier)].good==int(EstablishmentIndustry::WeaponForge)&&
              world.mask(governor,candidates)[size_t(170+tier)]==1.f,
              "region 19 authority receives three valid Forge funding choices");
    check(world.view(governor).provinces[size_t(region)].stock[size_t(iron)]>0&&
          world.view(governor).provinces[size_t(region)].stock[size_t(tools)]>0,
          "the two canonical input goods exist in the region before creating the Forge");
    const auto created=world.execute({governor,world.person(governor).decisions+1,candidates[170]});
    check(created.success&&world.establishments().size()==1&&
          world.establishments()[0].industry==EstablishmentIndustry::WeaponForge,
          "creating a Forge does not grant materials or Weapon stock");
    const uint64_t site=created.session;
    const auto wrong=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,18,EstablishmentIndustry::WeaponForge,1,25});
    check(!wrong.success,"Weapon Forge rejects a region other than atlas region19");
    const auto duplicate=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::WeaponForge,1,25});
    check(!duplicate.success&&world.establishments().size()==1&&world.invariant(),"only one Weapon Forge may exist in region19");

    check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
          site,EstablishmentTransferOperation::DepositGoods,iron,1}).success,
          "authority transfers one real Iron into the Forge");
    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
          "authority authorizes a present adult worker under current Work rules");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
          ActivityKind::Work,region,site}).success,"worker submits an authorized Work interval");
    world.advance_ticks(64);
    check(world.establishments()[0].stock[size_t(iron)]==1&&
          world.establishments()[0].stock[size_t(tools)]==0&&
          world.establishments()[0].stock[size_t(weapon)]==0,
          "missing Tools prevents Weapon output and preserves the deposited Iron");

    check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
          site,EstablishmentTransferOperation::DepositGoods,tools,1}).success,
          "authority supplies a real Tool separately after the first batch lacked it");
    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
          "authority renews Work authorization for the next interval");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
          ActivityKind::Work,region,site}).success,"worker submits the next authorized Work interval");
    world.advance_ticks(64);
    check(world.establishments()[0].stock[size_t(iron)]==0&&
          world.establishments()[0].stock[size_t(tools)]==0&&
          world.establishments()[0].stock[size_t(weapon)]==1&&world.invariant(),
          "one full batch consumes exactly one Iron and one Tool and creates one Weapon");
    const auto siteCandidates=world.candidates(governor);
    check(siteCandidates[size_t(realm_worksite_candidate_begin)].good==iron&&
          siteCandidates[size_t(realm_worksite_candidate_begin+1)].good==tools&&
          siteCandidates[size_t(realm_worksite_candidate_begin+2)].good==weapon,
          "worksite actions identify the exact two inputs and Weapon output");

    const auto saved=world.serialize("weapon-forge-trial-1");
    check(saved.rfind("POLYEC_REALM_70 ",0)==0&&saved.find(" ESTABLISH_14 1 ")!=std::string::npos,
          "save uses schema43 and the new establishment section tag");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(saved,"weapon-forge-trial-1",error),error.c_str());
    check(resumed.fingerprint()==world.fingerprint()&&resumed.establishments()[0].stock[size_t(weapon)]==1,
          "save/load retains the Forge output without repeating production");
    check(resumed.execute(EstablishmentTransferCommand{governor,resumed.person(governor).decisions+1,
          site,EstablishmentTransferOperation::WithdrawGoods,weapon,1}).success,
          "Forge owner withdraws Weapon to the regional market stock");
    check(resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Borrow,region,0,300,180}}).success,
          "buyer uses existing credit rules to pay for the market purchase");
    const auto bought=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Buy,region,weapon,1}});
    check(bought.success&&resumed.person(buyer).inventory[size_t(weapon)]==1,
          "buyer purchases canonical Weapon through the existing market command");
    const auto sold=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Sell,region,weapon,1}});
    check(sold.success&&resumed.person(buyer).inventory[size_t(weapon)]==0&&resumed.invariant(),
          "Weapon sale preserves market and accounting invariants");

    std::string schema42=test_fixtures::downgrade_organization_funds_44_to_43(saved);
    schema42.replace(0,std::string("POLYEC_REALM_43").size(),"POLYEC_REALM_42");
    const auto oldTag=schema42.find("ESTABLISH_13");schema42.replace(oldTag,std::string("ESTABLISH_13").size(),"ESTABLISH_12");
    Realm oldReader(1,1,128);
    check(!oldReader.deserialize(schema42,"weapon-forge-trial-1",error),"schema42 rejects industry ID11");
    Realm legacyWorld(49502,26,256);std::string empty42=test_fixtures::downgrade_organization_funds_44_to_43(legacyWorld.serialize("weapon-forge-trial-1"));
    empty42.replace(0,std::string("POLYEC_REALM_43").size(),"POLYEC_REALM_42");
    const auto emptyTag=empty42.find("ESTABLISH_13");empty42.replace(emptyTag,std::string("ESTABLISH_13").size(),"ESTABLISH_12");
    Realm migrated(1,1,128);
    check(migrated.deserialize(empty42,"weapon-forge-trial-1",error)&&migrated.establishments().empty()&&
          migrated.serialize("weapon-forge-trial-1").rfind("POLYEC_REALM_70 ",0)==0,
          "schema42 migrates without inventing Forge, Iron, Tools or Weapon stock");
    std::cout<<"Weapon Forge checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"Weapon Forge failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}


