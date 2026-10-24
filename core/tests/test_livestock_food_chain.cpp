#include "polyec/realm.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmLivestockFoodChainTestAccess {
    static void set_herd(Realm& world,uint64_t site,int64_t amount){
        for(auto& establishment:world.establishments_)if(establishment.id==site){establishment.source_units=amount;return;}
        throw std::runtime_error("missing Livestock Ranch");
    }
};
}

namespace {
int checks=0;
void check(bool ok,const std::string& message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t adult_worker(const Realm& world,int region,uint64_t exclude=0){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& p=world.person(id);
        if(id!=exclude&&p.alive&&p.destination==-1&&p.region==region&&p.family==Family::Civilian&&
           p.age_days>=18*365)return id;
    }
    return 0;
}
void schedule_work(Realm& world,uint64_t manager,uint64_t worker,uint64_t site,int region){
    check(world.execute(WorkAuthorizationCommand{manager,world.person(manager).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
          "the public site authorizes an adult local worker");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
          ActivityKind::Work,region,site}).success,
          "the worker submits the normal Work schedule for one batch");
}
}

int main(){try{
    constexpr int region=18;
    Realm world(20261086,int(kAtlasTerritoryCount)-1,512);
    const uint64_t authority=region+2;
    const uint64_t ranchWorker=adult_worker(world,region,authority);
    const uint64_t butcherWorker=adult_worker(world,region,ranchWorker);
    check(authority<=uint64_t(world.count())&&world.person(authority).taxing_authority&&ranchWorker&&butcherWorker,
          "Atlas region18 has a public authority and two adult civilian workers");
    check(realm_schema==std::string_view("realm-v38")&&world.model_contract()==
          "realm-household-fund-goods30-config-v49"&&realm_actions==400&&realm_obs==5182,
          "Livestock advances the model and config versions without changing tensor dimensions");

    const auto ranchCandidate=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin)];
    const auto butcherCandidate=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin+3)];
    check(ranchCandidate.kind==Intent::CreateEstablishment&&ranchCandidate.good==int(EstablishmentIndustry::LivestockRanch)&&
          world.mask(authority)[size_t(realm_specialty_product_candidate_begin)]==1.f,
          "region18 exposes a public Livestock Ranch through the existing specialty slots");
    check(butcherCandidate.kind==Intent::CreateEstablishment&&butcherCandidate.good==int(EstablishmentIndustry::Butchery)&&
          world.mask(authority)[size_t(realm_specialty_product_candidate_begin+3)]==1.f,
          "region18 exposes a public Butchery through a separate specialty slot group");
    const auto ranchResult=world.execute({authority,world.person(authority).decisions+1,ranchCandidate});
    const auto butcherResult=world.execute({authority,world.person(authority).decisions+1,butcherCandidate});
    check(ranchResult.success&&butcherResult.success&&world.establishments().size()==2&&world.invariant(),
          "the public ranch and public Butchery can both be funded in their designated region");
    const uint64_t ranch=ranchResult.session,butchery=butcherResult.session;
    const auto& ranchSite=world.establishments()[0];
    check(ranchSite.industry==EstablishmentIndustry::LivestockRanch&&
          ranchSite.source_good==int(GoodId::Livestock)&&ranchSite.source_units==25&&
          ranchSite.stock[size_t(GoodId::Livestock)]==0,
          "the ranch starts with 25 abstract herd units separate from saleable Livestock stock");

    const auto wrongRegion=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),17,EstablishmentIndustry::Butchery,1,25});
    const auto duplicateRanch=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),region,EstablishmentIndustry::LivestockRanch,1,25});
    check(!wrongRegion.success&&!duplicateRanch.success&&world.establishments().size()==2&&world.invariant(),
          "wrong-region and duplicate ranch commands reject without partial changes");

    schedule_work(world,authority,ranchWorker,ranch,region);
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==24&&world.establishments()[0].stock[size_t(GoodId::Livestock)]==1&&
          world.invariant(),"16 Work ticks consume one herd unit and produce one Livestock");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,ranch,
          EstablishmentTransferOperation::WithdrawGoods,int(GoodId::Livestock),1}).success&&
          world.view(authority).provinces[size_t(region)].stock[size_t(GoodId::Livestock)]==1,
          "the animal enters the region stock through the normal establishment transfer command");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,butchery,
          EstablishmentTransferOperation::DepositGoods,int(GoodId::Livestock),1}).success&&
          world.establishments()[1].stock[size_t(GoodId::Livestock)]==1,
          "the public Butchery receives Livestock from the regional ledger through the existing transfer command");
    const auto worksite=world.candidates(authority);
    bool butcherInputs=false,butcherOutputs=false;
    for(size_t i=size_t(realm_worksite_candidate_begin);i<size_t(realm_worksite_candidate_begin+
        realm_worksite_candidate_sites*realm_worksite_actions_per_site);++i){
        butcherInputs|=worksite[i].kind==Intent::TransferEstablishmentGoods&&worksite[i].good==int(GoodId::Livestock)&&
            worksite[i].term==int(EstablishmentTransferOperation::DepositGoods);
        butcherOutputs|=worksite[i].kind==Intent::TransferEstablishmentGoods&&worksite[i].good==int(GoodId::DryFood)&&
            worksite[i].term==int(EstablishmentTransferOperation::WithdrawGoods);
    }
    check(butcherInputs&&butcherOutputs,"Butchery actions identify Livestock as input and DryFood as output");
    schedule_work(world,authority,butcherWorker,butchery,region);
    world.advance_ticks(64);
    check(world.establishments()[1].stock[size_t(GoodId::Livestock)]==0&&
          world.establishments()[1].stock[size_t(GoodId::DryFood)]==3&&world.invariant(),
          "one Livestock plus 16 Work ticks is processed into three DryFood");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,butchery,
          EstablishmentTransferOperation::WithdrawGoods,int(GoodId::DryFood),3}).success&&
          world.view(authority).provinces[size_t(region)].stock[size_t(GoodId::DryFood)]>=3,
          "processed food enters regional stock and can use the existing market system");
    const int64_t foodPrice=world.view(ranchWorker).provinces[size_t(region)].price[size_t(GoodId::DryFood)];
    const auto buyFood=world.execute(MarketOrderCommand{ranchWorker,world.person(ranchWorker).decisions+1,0,
          MarketOrderOperation::Place,region,int(GoodId::DryFood),1,foodPrice});
    check(buyFood.success,"a civilian can order processed DryFood through the existing regional market");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.person(ranchWorker).inventory[size_t(GoodId::DryFood)]==1&&world.invariant(),
          "the market settles the locally produced food into the civilian's personal inventory");

    const auto beforeShortage=world.establishments()[1].stock[size_t(GoodId::DryFood)];
    schedule_work(world,authority,butcherWorker,butchery,region);
    world.advance_ticks(64);
    check(world.establishments()[1].stock[size_t(GoodId::Livestock)]==0&&
          world.establishments()[1].stock[size_t(GoodId::DryFood)]==beforeShortage&&world.invariant(),
          "a Butchery without Livestock cannot create food or change inventory");

    world.advance(90-world.day());
    check(world.day()==90&&world.establishments()[0].source_units==49,
          "at game day90 the herd source gains25 units without exceeding its100-unit limit (day="+
          std::to_string(world.day())+", herd="+std::to_string(world.establishments()[0].source_units)+")");
    world.advance(270);
    check(world.day()==360&&world.establishments()[0].source_units==100,
          "the herd regenerates on the 90-day schedule and stops at its100-unit cap");
    world.advance(90);
    check(world.day()==450&&world.establishments()[0].source_units==100,
          "another regeneration date cannot increase a full100-unit herd");
    auto saved=world.serialize("livestock-food-chain-v1");
    check(saved.rfind("POLYEC_REALM_71 ",0)==0,"current save writes schema71 for the new industries");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,"livestock-food-chain-v1",error)&&restored.invariant()&&
          restored.fingerprint()==world.fingerprint()&&restored.establishments().size()==2,
          "save and load preserve both sites, herd amount and goods stock without changing the world: "+error);

    Realm depleted(20261087,int(kAtlasTerritoryCount)-1,256);
    const uint64_t depletedAuthority=region+2;
    const uint64_t depletedWorker=adult_worker(depleted,region,depletedAuthority);
    const auto depletedRanch=depleted.execute({depletedAuthority,depleted.person(depletedAuthority).decisions+1,
        depleted.candidates(depletedAuthority)[size_t(realm_specialty_product_candidate_begin)]});
    check(depletedRanch.success,"a separate world creates a ranch for the empty-herd case");
    RealmLivestockFoodChainTestAccess::set_herd(depleted,depletedRanch.session,0);
    schedule_work(depleted,depletedAuthority,depletedWorker,depletedRanch.session,region);
    depleted.advance_ticks(64);
    check(depleted.establishments()[0].source_units==0&&
          depleted.establishments()[0].stock[size_t(GoodId::Livestock)]==0&&depleted.invariant(),
          "scheduled Work cannot create Livestock when the herd source is empty");
    std::cout<<"Livestock food chain checks: "<<checks<<"\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"FAILED after "<<checks<<" checks: "<<e.what()<<"\n";return 1;}}
