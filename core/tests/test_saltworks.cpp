#include "polyec/realm.hpp"
#include <iostream>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
}

int main(){try{
    Realm world(49201,26,256);
    constexpr int region=8;
    const uint64_t governor=region+2; // governance leaders occupy region IDs 0..26; ID 1 is at region 26.
    uint64_t buyer=0,worker=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.region!=region||!person.alive)continue;
        if(person.family==Family::Economic&&!buyer)buyer=id;
        if(person.family==Family::Civilian&&person.age_days>=14*365&&!worker)worker=id;
    }
    check(world.person(governor).region==region&&world.person(governor).taxing_authority,
          "atlas region 8 has its local public authority");
    check(buyer&&worker,"atlas region 8 has a buyer and a worker");
    const auto candidates=world.candidates(governor);
    constexpr size_t saltSlot=size_t(realm_establishment_candidate_begin+9);
    check(candidates[saltSlot].kind==Intent::CreateEstablishment&&candidates[saltSlot].good==4&&
          candidates[saltSlot].target==region&&world.mask(governor,candidates)[saltSlot]==1.f,
          "fourth establishment industry is available to the region 8 authority");
    const auto created=world.execute({governor,world.person(governor).decisions+1,candidates[saltSlot]});
    check(created.success&&world.establishments().size()==1&&
          world.establishments()[0].industry==EstablishmentIndustry::Saltworks,
          "policy command creates a public Saltworks");
    const uint64_t site=created.session;
    const auto wrongRegion=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,7,EstablishmentIndustry::Saltworks,1,25});
    check(!wrongRegion.success,"Saltworks placement outside atlas region 8 is rejected");
    const auto fuel=world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,site,
        EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1});
    check(fuel.success,"real regional Fuel is transferred into the public workshop");
    const auto authorization=world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,
        site,worker,WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32});
    check(authorization.success,"local authority authorizes a present worker");
    const auto work=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
        ActivityKind::Work,region,site});
    check(work.success,"worker submits Work through the timeline command");
    world.advance_ticks(64);
    check(world.establishments()[0].stock[size_t(GoodId::Fuel)]==0&&
          world.establishments()[0].stock[size_t(GoodId::Salt)]==1,
          "one completed batch consumes one Fuel and creates one Salt");
    const auto saved=world.serialize("saltworks-trial-1");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(saved,"saltworks-trial-1",error),error.c_str());
    resumed.advance_ticks(64);
    check(resumed.establishments()[0].stock[size_t(GoodId::Salt)]==1&&
          resumed.establishments()[0].stock[size_t(GoodId::Fuel)]==0,
          "save and load do not repeat the completed production batch");
    const auto withdrawn=resumed.execute(EstablishmentTransferCommand{governor,
        resumed.person(governor).decisions+1,site,EstablishmentTransferOperation::WithdrawGoods,
        int(GoodId::Salt),1});
    check(withdrawn.success&&resumed.establishments()[0].stock[size_t(GoodId::Salt)]==0,
          "public owner withdraws the produced Salt into the region stock");
    check(resumed.view(governor).provinces[size_t(region)].stock[size_t(GoodId::Salt)]==1,
          "withdrawal makes the same produced Salt available at the regional market");
    const auto purchase=resumed.execute({buyer,resumed.person(buyer).decisions+1,
        {Intent::Buy,region,int(GoodId::Salt),1}});
    check(purchase.success&&resumed.person(buyer).inventory[size_t(GoodId::Salt)]==1&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(GoodId::Salt)]==0,
          "buyer purchases the produced Salt through the existing market transaction");
    const auto sale=resumed.execute({buyer,resumed.person(buyer).decisions+1,
        {Intent::Sell,region,int(GoodId::Salt),1}});
    check(sale.success&&resumed.person(buyer).inventory[size_t(GoodId::Salt)]==0&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(GoodId::Salt)]==1,
          "a market seller can offer the produced Salt through the existing sale transaction");
    const auto repurchase=resumed.execute({buyer,resumed.person(buyer).decisions+1,
        {Intent::Buy,region,int(GoodId::Salt),1}});
    check(repurchase.success&&resumed.person(buyer).inventory[size_t(GoodId::Salt)]==1&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(GoodId::Salt)]==0,
          "the Salt returned by Sell remains purchasable through the same market");
    check(resumed.invariant(),"producer, regional stock, and buyer inventory conserve goods and money");
    std::cout<<"Saltworks checks="<<checks<<"\n";return 0;
}catch(const std::exception& error){std::cerr<<"Saltworks failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}





