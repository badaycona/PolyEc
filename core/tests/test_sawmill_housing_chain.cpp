#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <iostream>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t find_person(const Realm& world,Family family,uint64_t except=0){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.alive&&person.destination==-1&&person.region==6&&person.family==family&&
           person.age_days>=14*365&&id!=except)return id;
    }
    return 0;
}
}

int main(){try{
    constexpr int region=6;
    constexpr auto timber=GoodId::Timber;
    constexpr auto construction=GoodId::ConstructionMaterial;
    Realm world(20261024,26,256);
    const uint64_t governor=region+2;
    const uint64_t forest_worker=find_person(world,Family::Civilian);
    const uint64_t sawmill_worker=find_person(world,Family::Civilian,forest_worker);
    const uint64_t merchant=find_person(world,Family::Economic);
    check(world.person(governor).taxing_authority&&forest_worker&&sawmill_worker&&merchant,
          "region 6 fixture has its public authority, two workers and a market merchant");

    const auto forest=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::Forestry,1,25});
    check(forest.success,"Forestry can be established as the real Timber source");
    if(!forest.success)return 1;
    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,forest.session,
        forest_worker,WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+64}).success,
        "authority grants Work at the Forestry site");
    check(world.execute(ScheduleCommand{forest_worker,world.person(forest_worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+64,0,1000,
        ActivityKind::Work,region,forest.session}).success,"worker submits normal Forestry Work");
    world.advance_ticks(64);
    check(world.establishments().front().stock[size_t(timber)]==1,
          "completed Forestry Work creates one Timber from a bounded source unit");
    check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
        forest.session,EstablishmentTransferOperation::WithdrawGoods,int(timber),1}).success,
        "public authority moves the produced Timber into regional market stock");

    const auto sawmill=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::SawmillWorks,1,25});
    check(sawmill.success,"the unique region 6 Sawmill accepts its versioned trial recipe");
    if(!sawmill.success)return 1;
    check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
        sawmill.session,EstablishmentTransferOperation::DepositGoods,int(timber),1}).success,
        "Sawmill receives the actual market Timber through the normal transfer command");
    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,sawmill.session,
        sawmill_worker,WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+64}).success,
        "authority grants a second worker Work at the Sawmill");
    check(world.execute(ScheduleCommand{sawmill_worker,world.person(sawmill_worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+64,0,1000,
        ActivityKind::Work,region,sawmill.session}).success,"worker submits normal Sawmill Work");
    world.advance_ticks(64);
    auto sites=world.establishments();
    check(sites.back().stock[size_t(timber)]==0&&sites.back().stock[size_t(construction)]==1,
          "sixteen attendance ticks consume the Timber and produce one ConstructionMaterial");
    check(world.invariant(),"Forestry to Sawmill production conserves goods and preserves world invariants");

    const auto save=world.serialize("sawmill-housing-trial-1");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(save,"sawmill-housing-trial-1",error),error.c_str());
    check(resumed.fingerprint()==world.fingerprint()&&
          resumed.establishments().back().stock[size_t(construction)]==1,
          "save/load preserves the Sawmill recipe, its remaining stock and finished building material");
    check(checks>=14,"Sawmill chain includes all expected checks");
    std::cout<<"SAWMILL HOUSING CHAIN: "<<checks<<" checks passed\n";
    return 0;
}catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';return 1;}}
