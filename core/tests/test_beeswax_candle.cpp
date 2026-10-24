#include "polyec/realm.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace polyec;

namespace polyec {
struct RealmBeeswaxCandleTestAccess {
    static void give(Realm& world,uint64_t actor,GoodId good,int64_t amount){
        world.people_.at(size_t(actor-1)).inventory[size_t(good)]=amount;
    }
    static void set_sleep_debt(Realm& world,uint64_t actor,int amount){
        auto& body=world.people_.at(size_t(actor-1)).body_state;
        body.sleep_debt=amount;body.sleep_balance_remainder=0;
    }
    static void set_health(Realm& world,uint64_t actor,int amount){
        world.people_.at(size_t(actor-1)).health=amount;
    }
    static void set_source(Realm& world,uint64_t site,int64_t amount){
        auto found=std::find_if(world.establishments_.begin(),world.establishments_.end(),
            [&](const Establishment& value){return value.id==site;});
        if(found==world.establishments_.end())throw std::runtime_error("missing Apiary");
        found->source_units=amount;
    }
    static void set_active_service(Realm& world,uint64_t actor,bool active){
        world.people_.at(size_t(actor-1)).military_service.active=active;
    }
    static int64_t consumed(const Realm& world,GoodId good){return world.consumed_[size_t(good)];}
    static size_t event_count(const Realm& world,int kind){
        return std::count_if(world.facts_.begin(),world.facts_.end(),[&](const Fact& fact){return fact.kind==kind;});
    }
};
}

namespace {
int checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
uint64_t person_in_region(const Realm& world,int region,Family family,bool adult=false){
    for(uint64_t actor=1;actor<=uint64_t(world.count());++actor){
        const auto& person=world.person(actor);
        if(person.alive&&person.region==region&&person.destination==-1&&person.family==family&&
           (!adult||person.age_days>=18*365))return actor;
    }
    return 0;
}
void rest_one_day(Realm& world,uint64_t actor){
    const auto result=world.execute({actor,world.person(actor).decisions+1,
        {Intent::Rest,world.person(actor).region,0,0,1}});
    if(!result.success)throw std::runtime_error(("Rest rejected: "+result.reason).c_str());
}
}

int main(){try{
    constexpr int region=21;
    constexpr size_t beeswax=size_t(GoodId::Beeswax);
    static_assert(int(GoodId::Beeswax)==10);
    static_assert(realm_actions==400&&realm_obs==5182);

    Realm world(20261064,int(kAtlasTerritoryCount)-1,512);
    const uint64_t authority=uint64_t(region+2);
    const uint64_t worker=person_in_region(world,region,Family::Civilian,true);
    const uint64_t buyer=person_in_region(world,region,Family::Civilian,true);
    check(worker&&buyer&&world.person(authority).taxing_authority,
          "Atlas region 21 has an authorized public owner and adult civilian workers/buyers");
    const auto create=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin)];
    check(create.kind==Intent::CreateEstablishment&&create.target==region&&
          create.good==int(EstablishmentIndustry::Apiary)&&world.mask(authority)[size_t(realm_specialty_product_candidate_begin)]==1.f,
          "Atlas region 21 exposes Apiary through the existing specialty-establishment action");
    const auto created=world.execute({authority,world.person(authority).decisions+1,create});
    check(created.success&&world.establishments().size()==1&&
          world.establishments()[0].source_good==int(GoodId::Beeswax)&&
          world.establishments()[0].source_units==25&&world.establishments()[0].stock[beeswax]==0,
          "a new Apiary starts with 25 of 100 source units separate from Beeswax stock");
    const uint64_t site=created.session;
    const auto wrongRegion=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,region,20,EstablishmentIndustry::Apiary,1,25});
    check(!wrongRegion.success&&world.establishments().size()==1&&world.invariant(),
          "a direct command cannot establish an Apiary outside region 21 or partially change the world");
    const auto duplicate=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::Apiary,1,25});
    check(!duplicate.success&&world.establishments().size()==1,
          "the same region cannot receive a second Apiary source site");
    check(world.execute(WorkAuthorizationCommand{authority,world.person(authority).decisions+1,site,worker,
        WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success&&
          world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
          ActivityKind::Work,region,site}).success,
          "the Apiary owner authorizes the adult worker and the shared schedule accepts Work");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.establishments()[0].source_units==24&&world.establishments()[0].stock[beeswax]==1&&world.invariant(),
          "one complete Work batch consumes one source unit and creates one Beeswax");

    const auto withdrawal=world.execute(EstablishmentTransferCommand{authority,
        world.person(authority).decisions+1,site,EstablishmentTransferOperation::WithdrawGoods,
        int(GoodId::Beeswax),1});
    check(withdrawal.success&&world.view(authority).provinces[size_t(region)].stock[beeswax]>0,
          "the owner transfers the actual Apiary output to the regional market stock");
    const int64_t price=world.view(buyer).provinces[size_t(region)].price[beeswax];
    if(world.person(buyer).cash<price)
        check(world.execute({buyer,world.person(buyer).decisions+1,
            {Intent::Borrow,region,0,100,180}}).success,"the buyer can use an existing loan if needed");
    check(world.execute(MarketOrderCommand{buyer,world.person(buyer).decisions+1,0,
          MarketOrderOperation::Place,region,int(GoodId::Beeswax),1,price}).success,
          "the Civilian places a normal market order for Beeswax");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.person(buyer).inventory[beeswax]==1&&world.invariant(),
          "market settlement puts Apiary output in the buyer's personal inventory");

    RealmBeeswaxCandleTestAccess::set_sleep_debt(world,buyer,10);
    RealmBeeswaxCandleTestAccess::set_health(world,buyer,80);
    const int healthBefore=world.person(buyer).health;
    rest_one_day(world,buyer);
    const auto middle=world.serialize("beeswax-candle-mid-rest");
    Realm loaded(1,1,128);std::string error;
    check(loaded.deserialize(middle,"beeswax-candle-mid-rest",error),
          ("an in-progress Rest with personal Beeswax loads: "+error).c_str());
    check(loaded.person(buyer).inventory[beeswax]==1&&RealmBeeswaxCandleTestAccess::consumed(loaded,GoodId::Beeswax)==0,
          "saving during Rest keeps the candle personal and does not consume it early");
    loaded.advance_ticks(kSubdayUnitsPerDay-1);
    check(loaded.person(buyer).inventory[beeswax]==1&&RealmBeeswaxCandleTestAccess::event_count(loaded,239)==0,
          "the candle remains unused until the full Rest interval has elapsed");
    loaded.advance_ticks(1);
    check(loaded.person(buyer).inventory[beeswax]==0&&
          RealmBeeswaxCandleTestAccess::consumed(loaded,GoodId::Beeswax)==1&&
          RealmBeeswaxCandleTestAccess::event_count(loaded,239)==1,
          "completed Rest burns one owned Beeswax and records one clear event");
    check(loaded.person(buyer).body_state.sleep_debt==4&&loaded.person(buyer).health==healthBefore+4&&loaded.invariant(),
          "Rest keeps its usual four health points for one day while the candle reduces sleep debt by five more points");
    const auto afterRest=loaded.serialize("beeswax-candle-mid-rest");
    Realm resumed(1,1,128);
    check(resumed.deserialize(afterRest,"beeswax-candle-mid-rest",error)&&
          resumed.serialize("beeswax-candle-mid-rest")==afterRest&&
          RealmBeeswaxCandleTestAccess::event_count(resumed,239)==1,
          "save/load after completion preserves the consumed item and does not repeat the event");

    Realm noBenefit(20261064,int(kAtlasTerritoryCount)-1,512);
    const uint64_t noBenefitActor=person_in_region(noBenefit,region,Family::Civilian,true);
    RealmBeeswaxCandleTestAccess::give(noBenefit,noBenefitActor,GoodId::Beeswax,1);
    RealmBeeswaxCandleTestAccess::set_sleep_debt(noBenefit,noBenefitActor,0);
    rest_one_day(noBenefit,noBenefitActor);noBenefit.advance_ticks(kSubdayUnitsPerDay);
    check(noBenefit.person(noBenefitActor).inventory[beeswax]==1&&
          RealmBeeswaxCandleTestAccess::consumed(noBenefit,GoodId::Beeswax)==0&&
          RealmBeeswaxCandleTestAccess::event_count(noBenefit,239)==0,
          "a candle is not wasted when the owner has no sleep debt to reduce");

    Realm activeService(20261067,int(kAtlasTerritoryCount)-1,256);
    const uint64_t servingActor=person_in_region(activeService,region,Family::Civilian,true);
    RealmBeeswaxCandleTestAccess::give(activeService,servingActor,GoodId::Beeswax,1);
    RealmBeeswaxCandleTestAccess::set_sleep_debt(activeService,servingActor,10);
    rest_one_day(activeService,servingActor);
    RealmBeeswaxCandleTestAccess::set_active_service(activeService,servingActor,true);
    activeService.advance_ticks(kSubdayUnitsPerDay);
    check(activeService.person(servingActor).inventory[beeswax]==1&&
          RealmBeeswaxCandleTestAccess::consumed(activeService,GoodId::Beeswax)==0&&
          RealmBeeswaxCandleTestAccess::event_count(activeService,239)==0,
          "a Civilian who enters active military service before Rest completes keeps their Beeswax");

    Realm clampWorld(20261068,int(kAtlasTerritoryCount)-1,256);
    const uint64_t clampActor=person_in_region(clampWorld,region,Family::Civilian,true);
    RealmBeeswaxCandleTestAccess::give(clampWorld,clampActor,GoodId::Beeswax,1);
    RealmBeeswaxCandleTestAccess::set_sleep_debt(clampWorld,clampActor,6);
    rest_one_day(clampWorld,clampActor);clampWorld.advance_ticks(kSubdayUnitsPerDay);
    check(clampWorld.person(clampActor).body_state.sleep_debt==0&&
          clampWorld.person(clampActor).inventory[beeswax]==0&&
          RealmBeeswaxCandleTestAccess::consumed(clampWorld,GoodId::Beeswax)==1,
          "the Rest recovery plus candle benefit clamps sleep debt at zero without a negative value");

    Realm seasonal(20261065,int(kAtlasTerritoryCount)-1,256);
    const uint64_t seasonalAuthority=uint64_t(region+2);
    const auto seasonalCreate=seasonal.candidates(seasonalAuthority)[size_t(realm_specialty_product_candidate_begin)];
    const auto seasonalSite=seasonal.execute({seasonalAuthority,seasonal.person(seasonalAuthority).decisions+1,seasonalCreate});
    check(seasonalSite.success,"a separate Atlas world creates an Apiary for source-season checks");
    Realm sourceShortage(20261069,int(kAtlasTerritoryCount)-1,256);
    const uint64_t shortageAuthority=uint64_t(region+2);
    const uint64_t sourceWorker=person_in_region(sourceShortage,region,Family::Civilian,true);
    const auto shortageCreate=sourceShortage.candidates(shortageAuthority)[size_t(realm_specialty_product_candidate_begin)];
    const auto shortageSite=sourceShortage.execute({shortageAuthority,sourceShortage.person(shortageAuthority).decisions+1,shortageCreate});
    check(shortageSite.success,"a separate Atlas world creates an Apiary for depleted-source checks");
    RealmBeeswaxCandleTestAccess::set_source(sourceShortage,shortageSite.session,0);
    check(sourceShortage.execute(WorkAuthorizationCommand{shortageAuthority,sourceShortage.person(shortageAuthority).decisions+1,
        shortageSite.session,sourceWorker,WorkAuthorizationOperation::Set,sourceShortage.game_tick(),sourceShortage.game_tick()+32}).success&&
          sourceShortage.execute(ScheduleCommand{sourceWorker,sourceShortage.person(sourceWorker).decisions+1,
          ScheduleCommand::Operation::Submit,0,sourceShortage.game_tick(),sourceShortage.game_tick()+32,0,1000,
          ActivityKind::Work,region,shortageSite.session}).success,
          "the Apiary owner can authorize a real worker even when its finite source is depleted");
    sourceShortage.advance_ticks(kSubdayUnitsPerDay);
    check(sourceShortage.establishments()[0].source_units==0&&sourceShortage.establishments()[0].stock[beeswax]==0,
          "depleted Apiary source produces no Beeswax despite completed Work");
    RealmBeeswaxCandleTestAccess::set_source(seasonal,seasonalSite.session,24);
    seasonal.advance(89);
    check(seasonal.establishments()[0].source_units==24,"Apiary source does not replenish before day 90");
    seasonal.advance(1);
    check(seasonal.establishments()[0].source_units==49,"day 90 adds 25 source units to the Apiary");
    RealmBeeswaxCandleTestAccess::set_source(seasonal,seasonalSite.session,100);
    seasonal.advance(90);
    check(seasonal.establishments()[0].source_units==100,"seasonal replenishment stops at the source cap of 100");

    const auto currentSave=world.serialize("beeswax-schema");
    check(currentSave.rfind("POLYEC_REALM_72 ",0)==0&&currentSave.find(" ESTABLISH_14 1 ")!=std::string::npos,
          "schema70 extends the establishment industry enum without changing record fields");
    Realm roundTrip(1,1,128);
    check(roundTrip.deserialize(currentSave,"beeswax-schema",error)&&
          roundTrip.establishments()[0].industry==EstablishmentIndustry::Apiary&&roundTrip.invariant(),
          "schema70 roundtrip restores the Apiary and its source record");
    auto badSave=currentSave;
    const auto siteAt=badSave.find(" ESTABLISH_14 1 ");
    const auto regionAt=badSave.find(' ',siteAt+std::string(" ESTABLISH_14 1 ").size());
    const auto regionEnd=badSave.find(' ',regionAt+1);
    badSave.replace(regionAt+1,regionEnd-regionAt-1,"20");
    const auto before=roundTrip.fingerprint();
    check(!roundTrip.deserialize(badSave,"beeswax-schema",error)&&roundTrip.fingerprint()==before,
          "a schema70 Apiary moved away from its Atlas source region is rejected atomically");
    Realm oldBaseline(20261066,int(kAtlasTerritoryCount)-1,256);
    auto oldSave=oldBaseline.serialize("beeswax-legacy");oldSave.replace(0,15,"POLYEC_REALM_67");
    Realm oldLoaded(1,1,128);
    check(oldLoaded.deserialize(oldSave,"beeswax-legacy",error)&&
          oldLoaded.serialize("beeswax-legacy").rfind("POLYEC_REALM_72 ",0)==0&&oldLoaded.invariant(),
          "schema67 saves without Apiary migrate and are written as schema70");

    check(world.model_contract()=="realm-worksite-payroll-goods30-config-v50"&&
          std::string_view(realm_schema)=="realm-v39",
          "the new rule advances the model contract without changing tensor sizes");
    std::cout<<"Beeswax source and candle checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"Beeswax source and candle failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}
