#include "polyec/realm.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmObservationTestAccess {
    static void add_silver(Realm& world,uint64_t actor,int64_t amount=1){
        world.mutable_person(actor).inventory[size_t(GoodId::Silver)]+=amount;
        world.opening_[size_t(GoodId::Silver)]+=amount;
    }
    static int64_t consumed_silver(const Realm& world){return world.consumed_[size_t(GoodId::Silver)];}
    static void set_consumed_silver(Realm& world,int64_t amount){world.consumed_[size_t(GoodId::Silver)]=amount;}
    static void set_social(Realm& world,uint64_t actor,int value){world.mutable_person(actor).social=value;}
    static void set_cash(Realm& world,uint64_t actor,int64_t amount){world.mutable_person(actor).cash=amount;}
};
}

namespace {
int checks=0;
void check(bool ok,const std::string&message){++checks;if(!ok)throw std::runtime_error(message);}
}

int main(){try{
    constexpr int region=23;
    constexpr auto silver=GoodId::Silver;
    constexpr const char* saveName="silver-social-consumer-v1";
    Realm world(20261051,26,256);
    const uint64_t authority=region+2;
    uint64_t worker=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto&person=world.person(id);
        if(person.alive&&person.region==region&&person.family==Family::Civilian&&person.age_days>=14*365&&
           world.household_authority(world.household_of(id))==id){worker=id;break;}
    }
    check(world.model_contract()=="realm-household-fund-goods30-config-v46"&&realm_actions==400&&realm_obs==5182,
          "Silver social uses the new action meaning without changing tensor shapes");
    check(worker&&world.person(authority).taxing_authority,"region23 has its mining authority and an adult worker");
    check(world.candidates(worker)[size_t(realm_silver_social_candidate)].kind==Intent::Hold,
          "Silver gift is absent before the actor personally owns Silver");

    const auto mine=world.execute({authority,world.person(authority).decisions+1,world.candidates(authority)[95]});
    check(mine.success&&world.establishments().size()==1&&world.establishments().back().source_good==int(silver),
          "the region authority establishes the existing Silver mine through a normal candidate");
    const auto site=mine.session;
    check(world.execute(WorkAuthorizationCommand{authority,world.person(authority).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
          "the authority grants the selected worker permission to work at the mine");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
          0,world.game_tick(),world.game_tick()+32,0,1000,ActivityKind::Work,region,site}).success,
          "the worker submits an ordinary Work schedule");
    world.advance_ticks(64);
    check(world.establishments().back().stock[size_t(silver)]==1&&world.establishments().back().source_units==99,
          "real scheduled work consumes one bounded ore unit and produces one Silver");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,site,
          EstablishmentTransferOperation::WithdrawGoods,int(silver),1}).success&&
          world.view(authority).provinces[size_t(region)].stock[size_t(silver)]==1,
          "the public authority transfers the produced Silver to the regional market");

    const auto household=world.household_of(worker);
    uint64_t supporter=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto&person=world.person(id);
        if(id!=worker&&person.alive&&person.family==Family::Civilian&&person.age_days>=6*365&&
           person.region==region&&world.household_of(id)==household&&
           world.household_authority(household)==worker){supporter=id;break;}
    }
    check(supporter>0,"the buyer has an eligible same-household member for ordinary household support");
    const int64_t silverPrice=world.view(worker).provinces[size_t(region)].price[size_t(silver)];
    while(world.person(worker).cash<silverPrice+2&&supporter&&world.person(supporter).cash>=5){
        const auto support=world.execute({supporter,world.person(supporter).decisions+1,
            {Intent::Support,int(worker),0,5}});
        if(!support.success)break;
    }
    check(world.person(worker).cash>=silverPrice+2,"lawful household support funds the buyer's market purchase and later fee");
    const auto order=world.execute(MarketOrderCommand{worker,world.person(worker).decisions+1,0,
        MarketOrderOperation::Place,region,int(silver),1,silverPrice});
    check(order.success&&world.person(worker).inventory[size_t(silver)]==0&&world.market_orders().size()==1,
          "the Civilian places a normal market order and Silver remains in escrow until settlement");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.person(worker).inventory[size_t(silver)]==1&&world.invariant(),
          "next-day market settlement delivers exactly one Silver to the buyer");

    const auto action=world.candidates(worker)[size_t(realm_silver_social_candidate)];
    check(action.kind==Intent::Social&&action.good==int(silver)&&action.amount==2&&action.term==7&&
          Realm::action_name(action)=="Giao lưu cùng bạc"&&world.mask(worker)[size_t(realm_silver_social_candidate)]==1.f,
          "the eligible owner sees the separately named, legal one-Silver action");
    check(world.candidates(worker)[17].kind==Intent::Social&&world.candidates(worker)[24].kind==Intent::Hold&&
          world.candidates(worker)[25].kind==Intent::Hold,
          "ordinary social is unchanged and Silver does not unlock Tea or Incense");
    const auto beforeCash=world.person(worker).cash;
    const auto beforeSocial=world.person(worker).social;
    const auto beforeSilver=world.person(worker).inventory[size_t(silver)];
    const auto beforeFund=world.view(worker).provinces[size_t(region)].firm;
    const auto beforeConsumed=RealmObservationTestAccess::consumed_silver(world);
    const auto used=world.execute({worker,world.person(worker).decisions+1,action});
    check(used.success&&world.person(worker).cash==beforeCash-2&&
          world.view(worker).provinces[size_t(region)].firm==beforeFund+2&&
          world.person(worker).inventory[size_t(silver)]==beforeSilver-1&&
          RealmObservationTestAccess::consumed_silver(world)==beforeConsumed+1&&
          world.person(worker).social==std::min(100,beforeSocial+10)&&world.invariant(),
          "Silver is spent from its owner's inventory, two coins go to the regional fund and social rises by ten");
    check(world.person(worker).busy_until==world.day()+7,"the action occupies seven game days");
    const auto busyBefore=world.fingerprint();
    check(!world.execute({worker,world.person(worker).decisions+1,action}).success&&world.fingerprint()==busyBefore,
          "a busy worker cannot repeat the gift action or spend a second Silver");

    const auto saved=world.serialize(saveName);
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,saveName,error),"Silver social state saves and loads: "+error);
    check(restored.fingerprint()==world.fingerprint()&&restored.person(worker).inventory[size_t(silver)]==0&&
          RealmObservationTestAccess::consumed_silver(restored)==beforeConsumed+1&&restored.invariant(),
          "save/load retains Silver consumption, social level, busy time and all ledgers");
    const int remaining=std::max(0,restored.person(worker).busy_until-restored.day());
    restored.advance(remaining);world.advance(remaining);
    check(restored.fingerprint()==world.fingerprint(),"restored and uninterrupted worlds continue identically after the busy period");

    Realm capped(20261052,26,256);
    uint64_t capActor=0;
    for(uint64_t id=1;id<=uint64_t(capped.count());++id)if(capped.person(id).family==Family::Civilian&&capped.person(id).age_days>=6*365){capActor=id;break;}
    check(capActor>0,"cap fixture has an eligible Civilian");
    RealmObservationTestAccess::add_silver(capped,capActor);
    RealmObservationTestAccess::set_social(capped,capActor,98);
    const auto capAction=capped.candidates(capActor)[size_t(realm_silver_social_candidate)];
    const auto capResult=capped.execute({capActor,capped.person(capActor).decisions+1,capAction});
    check(capResult.success&&capped.person(capActor).social==100&&capped.invariant(),
          "the ten-point benefit stops at the existing social maximum of 100");

    Realm noSilver(20261053,26,256);
    uint64_t noSilverActor=0;
    for(uint64_t id=1;id<=uint64_t(noSilver.count());++id)if(noSilver.person(id).family==Family::Civilian&&noSilver.person(id).age_days>=6*365){noSilverActor=id;break;}
    const auto noSilverBefore=noSilver.fingerprint();
    const auto forged=Candidate{Intent::Social,noSilver.person(noSilverActor).region,int(silver),2,7};
    check(!noSilver.execute({noSilverActor,noSilver.person(noSilverActor).decisions+1,forged}).success&&
          noSilver.fingerprint()==noSilverBefore,"candidate spoofing cannot consume Silver the actor does not own");

    Realm wrongRole(20261054,26,256);
    const uint64_t economic=2;
    check(wrongRole.candidates(economic)[size_t(realm_silver_social_candidate)].kind!=Intent::Social,
          "Economic role does not receive the Civilian Silver gift action");

    Realm poor(20261055,26,256);
    uint64_t poorActor=0;
    for(uint64_t id=1;id<=uint64_t(poor.count());++id)if(poor.person(id).family==Family::Civilian&&poor.person(id).age_days>=6*365){poorActor=id;break;}
    RealmObservationTestAccess::add_silver(poor,poorActor);
    RealmObservationTestAccess::set_cash(poor,poorActor,1);
    const auto poorBefore=poor.fingerprint();
    check(!poor.execute({poorActor,poor.person(poorActor).decisions+1,poor.candidates(poorActor)[size_t(realm_silver_social_candidate)]}).success&&
          poor.fingerprint()==poorBefore,"a personal balance below two coins rejects without consuming Silver");

    Realm overflow(20261056,26,256);
    uint64_t overflowActor=0;
    for(uint64_t id=1;id<=uint64_t(overflow.count());++id)if(overflow.person(id).family==Family::Civilian&&overflow.person(id).age_days>=6*365){overflowActor=id;break;}
    RealmObservationTestAccess::add_silver(overflow,overflowActor);
    RealmObservationTestAccess::set_consumed_silver(overflow,std::numeric_limits<int64_t>::max());
    const auto overflowBefore=overflow.fingerprint();
    check(!overflow.execute({overflowActor,overflow.person(overflowActor).decisions+1,
          overflow.candidates(overflowActor)[size_t(realm_silver_social_candidate)]}).success&&
          overflow.fingerprint()==overflowBefore,"consumed-Silver ledger overflow rejects atomically");

    std::cout<<"Silver social checks="<<checks<<'\n';return 0;
}catch(const std::exception&error){std::cerr<<"Silver social failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}
