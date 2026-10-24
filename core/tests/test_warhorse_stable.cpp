#include "work_authorization_fixture.hpp"
#include "polyec/realm.hpp"
#include "polyec/production_manager.hpp"
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
    constexpr int region=16,horse=int(GoodId::Horse),warhorse=int(GoodId::WarHorse);
    Realm world(49401,26,256);
    const uint64_t governor=region+2,worker=find_person(world,region,Family::Civilian,true);
    const uint64_t buyer=find_person(world,region,Family::Economic,true);
    check(world.person(governor).taxing_authority&&worker&&buyer,"region 16 has a public authority, worker and buyer");
    const auto candidates=world.candidates(governor);
    check(realm_warhorse_stable_candidate_begin==167&&realm_actions==400&&realm_obs==5182,
          "Warhorse choices are appended after existing actions and match observation size");
    for(int tier=0;tier<3;++tier)
        check(candidates[size_t(167+tier)].kind==Intent::CreateEstablishment&&
              candidates[size_t(167+tier)].good==int(EstablishmentIndustry::WarhorseStable)&&
              world.mask(governor,candidates)[size_t(167+tier)]==1.f,
              "region 16 authority receives the three valid Stable funding choices");
    check(world.view(governor).provinces[size_t(region)].stock[size_t(horse)]>0,
          "Horse stock exists in region 16 before creating the training site");
    const auto created=world.execute({governor,world.person(governor).decisions+1,candidates[167]});
    check(created.success&&world.establishments().size()==1&&
          world.establishments()[0].industry==EstablishmentIndustry::WarhorseStable&&
          world.establishments()[0].source_good==kNoEstablishmentSourceGood,
          "creating a Stable does not create Horse or WarHorse source inventory");
    const uint64_t site=created.session;
    const auto wrong=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,15,EstablishmentIndustry::WarhorseStable,1,25});
    check(!wrong.success,"Warhorse Stable cannot be created outside atlas region 16");
    const auto duplicate=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::WarhorseStable,1,25});
    check(!duplicate.success&&world.establishments().size()==1&&world.invariant(),"region 16 permits at most one Warhorse Stable");

    const auto recipe=production_recipe(EstablishmentIndustry::WarhorseStable,world.gameplay_config());
    check(recipe.input_count==1&&recipe.inputs[0].good==size_t(horse)&&recipe.inputs[0].amount==1,
          "training recipe accepts exactly one Horse and no Livestock substitute");
    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
          "authority authorizes a present adult worker using existing permission rules");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
          ActivityKind::Work,region,site}).success,"worker submits Work through the existing schedule command");
    world.advance_ticks(64);
    check(world.establishments()[0].stock[size_t(horse)]==0&&
          world.establishments()[0].stock[size_t(warhorse)]==0,
          "Work without a real Horse does not produce a WarHorse");

    check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
          site,EstablishmentTransferOperation::DepositGoods,horse,1}).success,
          "authority transfers one real Horse from regional stock into the site");
    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
          "authority renews Work authorization for the next interval");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
          ActivityKind::Work,region,site}).success,"worker submits a second authorized Work interval");
    world.advance_ticks(64);
    check(world.establishments()[0].stock[size_t(horse)]==0&&
          world.establishments()[0].stock[size_t(warhorse)]==1&&world.invariant(),
          "one Horse and one complete Work batch create exactly one WarHorse");
    const auto siteCandidates=world.candidates(governor);
    check(siteCandidates[size_t(realm_worksite_candidate_begin+2)].good==warhorse&&
          siteCandidates[size_t(realm_worksite_candidate_begin)].good==horse,
          "site actions name Horse as the input and WarHorse as the output");

    const auto saved=world.serialize("warhorse-stable-trial-1");
    check(saved.rfind("POLYEC_REALM_67 ",0)==0&&saved.find(" ESTABLISH_14 1 ")!=std::string::npos,
          "save uses schema42 and the new establishment record tag");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(saved,"warhorse-stable-trial-1",error),error.c_str());
    check(resumed.fingerprint()==world.fingerprint()&&resumed.establishments()[0].stock[size_t(warhorse)]==1,
          "save/load preserves Horse consumption and WarHorse output");
    check(resumed.execute(EstablishmentTransferCommand{governor,resumed.person(governor).decisions+1,
          site,EstablishmentTransferOperation::WithdrawGoods,warhorse,1}).success,
          "owner withdraws the trained WarHorse into regional stock");
    check(resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Borrow,region,0,300,180}}).success,
          "buyer uses existing credit rules to pay the catalog price");
    const auto bought=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Buy,region,warhorse,1}});
    check(bought.success&&resumed.person(buyer).inventory[size_t(warhorse)]==1,
          "merchant purchases WarHorse through the existing market command");
    const auto sold=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Sell,region,warhorse,1}});
    check(sold.success&&resumed.person(buyer).inventory[size_t(warhorse)]==0&&resumed.invariant(),
          "WarHorse trade preserves the market and world accounting rules");

    std::string prior=test_fixtures::downgrade_organization_funds_44_to_43(saved);
    prior.replace(0,std::string("POLYEC_REALM_43").size(),"POLYEC_REALM_41");
    const auto tag=prior.find("ESTABLISH_13");prior.replace(tag,std::string("ESTABLISH_13").size(),"ESTABLISH_11");
    Realm oldReader(1,1,128);
    check(!oldReader.deserialize(prior,"warhorse-stable-trial-1",error),"schema41 rejects the later Warhorse industry record");
    Realm legacyWorld(49402,26,256);std::string schema41=test_fixtures::downgrade_organization_funds_44_to_43(legacyWorld.serialize("warhorse-stable-trial-1"));
    schema41.replace(0,std::string("POLYEC_REALM_43").size(),"POLYEC_REALM_41");
    const auto emptyTag=schema41.find("ESTABLISH_13");schema41.replace(emptyTag,std::string("ESTABLISH_13").size(),"ESTABLISH_11");
    Realm migrated(1,1,128);
    check(migrated.deserialize(schema41,"warhorse-stable-trial-1",error)&&migrated.establishments().empty()&&
          migrated.serialize("warhorse-stable-trial-1").rfind("POLYEC_REALM_67 ",0)==0,
          "schema41 migrates without inventing a Stable or Horse/WarHorse stock");
    std::cout<<"Warhorse Stable checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"Warhorse Stable failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}




