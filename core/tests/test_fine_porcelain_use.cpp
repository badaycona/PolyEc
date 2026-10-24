#include "polyec/realm.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmFinePorcelainTestAccess {
    static void give(Realm& world,uint64_t actor,int64_t amount=1){
        world.mutable_person(actor).inventory[size_t(GoodId::FinePorcelain)]+=amount;
        world.opening_[size_t(GoodId::FinePorcelain)]+=amount;
    }
    static int64_t consumed(const Realm& world){return world.consumed_[size_t(GoodId::FinePorcelain)];}
    static void set_consumed(Realm& world,int64_t value){world.consumed_[size_t(GoodId::FinePorcelain)]=value;}
    static void set_age(Realm& world,uint64_t actor,int value){world.mutable_person(actor).age_days=value;}
    static void set_cash(Realm& world,uint64_t actor,int64_t value){auto& person=world.mutable_person(actor);world.opening_money_+=value-person.cash;person.cash=value;}
    static void set_social(Realm& world,uint64_t actor,int value){world.mutable_person(actor).social=value;}
};
}

namespace {
int checks=0;
void check(bool ok,const std::string& message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t civilian_at(const Realm& world,int region,uint64_t exclude=0){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& p=world.person(id);
        if(id!=exclude&&p.alive&&p.destination==-1&&p.region==region&&p.family==Family::Civilian&&
           p.age_days>=18*365)return id;
    }
    return 0;
}
void schedule_work(Realm& world,uint64_t manager,uint64_t worker,uint64_t site,int region){
    check(world.execute(WorkAuthorizationCommand{manager,world.person(manager).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+48}).success,
          "the owner authorizes the local worker through the normal work command");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+48,0,1000,
          ActivityKind::Work,region,site}).success,
          "the worker submits the normal scheduled work interval");
}
void fund_household_member(Realm& world,uint64_t recipient,int64_t target){
    for(int attempt=0;world.person(recipient).cash<target&&attempt<32;++attempt){
        uint64_t supporter=0;
        for(uint64_t id=1;id<=uint64_t(world.count());++id)
            if(id!=recipient&&world.person(id).alive&&world.household_of(id)==world.household_of(recipient)&&
               world.person(id).cash>=5){supporter=id;break;}
        if(!supporter||!world.execute({supporter,world.person(supporter).decisions+1,
             {Intent::Support,int(recipient),0,5}}).success)break;
    }
}
}

int main(){try{
    constexpr int region=15;
    constexpr auto pottery=GoodId::Pottery;
    constexpr auto porcelain=GoodId::FinePorcelain;
    Realm world(20261088,int(kAtlasTerritoryCount)-1,512);
    const uint64_t authority=region+2;
    const uint64_t potteryWorker=civilian_at(world,region);
    const uint64_t porcelainWorker=civilian_at(world,region,potteryWorker);
    const uint64_t buyer=civilian_at(world,region,porcelainWorker);
    check(authority<=uint64_t(world.count())&&world.person(authority).taxing_authority&&potteryWorker&&porcelainWorker&&buyer,
          "Atlas region15 has the public authority and three local adult civilians");
    check(realm_schema==std::string_view("realm-v40")&&world.model_contract()==
          "realm-household-fund-goods30-config-v51"&&realm_actions==400&&realm_obs==5182,
          "FinePorcelain advances the model/config versions without changing tensor dimensions");

    const auto potteryCandidate=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin+3)];
    const auto porcelainCandidate=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin+6)];
    check(potteryCandidate.kind==Intent::CreateEstablishment&&potteryCandidate.good==int(EstablishmentIndustry::PotteryKiln)&&
          porcelainCandidate.kind==Intent::CreateEstablishment&&
          porcelainCandidate.good==int(EstablishmentIndustry::FinePorcelainWorks)&&
          world.mask(authority)[size_t(realm_specialty_product_candidate_begin+6)]==1.f,
          "region15 keeps the PotteryKiln and exposes a separate FinePorcelain producer");
    const auto potteryResult=world.execute({authority,world.person(authority).decisions+1,potteryCandidate});
    const auto porcelainResult=world.execute({authority,world.person(authority).decisions+1,porcelainCandidate});
    check(potteryResult.success&&porcelainResult.success&&world.establishments().size()==2&&world.invariant(),
          "both public producers can be funded and created in region15");
    const uint64_t potteryKiln=potteryResult.session,porcelainWorks=porcelainResult.session;
    check(world.establishments()[0].industry==EstablishmentIndustry::PotteryKiln&&
          world.establishments()[1].industry==EstablishmentIndustry::FinePorcelainWorks&&
          world.establishments()[1].source_good==kNoEstablishmentSourceGood&&world.invariant(),
          "the new workshop has no fake finite source and remains distinct from the Pottery source");
    const auto wrongRegion=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),9,EstablishmentIndustry::FinePorcelainWorks,1,25});
    const auto duplicate=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),region,EstablishmentIndustry::FinePorcelainWorks,1,25});
    check(!wrongRegion.success&&!duplicate.success&&world.establishments().size()==2&&world.invariant(),
          "wrong-region and duplicate FinePorcelain workshop creation reject atomically");

    const int64_t fuel=world.view(authority).provinces[size_t(region)].stock[size_t(GoodId::Fuel)];
    check(fuel>=2&&world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,
          potteryKiln,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1}).success&&
          world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,
          porcelainWorks,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1}).success,
          "Fuel from the regional stock is transferred separately into both production sites");
    schedule_work(world,authority,porcelainWorker,porcelainWorks,region);
    world.advance_ticks(96);
    check(world.establishments()[1].stock[size_t(porcelain)]==0&&
          world.establishments()[1].stock[size_t(GoodId::Fuel)]==1&&world.invariant(),
          "Fuel alone cannot be consumed or create partial output while Pottery is missing");
    schedule_work(world,authority,potteryWorker,potteryKiln,region);
    world.advance_ticks(96);
    check(world.establishments()[0].stock[size_t(pottery)]==1&&
          world.establishments()[0].stock[size_t(GoodId::Fuel)]==0&&
          world.establishments()[0].source_units==99&&world.invariant(),
          "PotteryKiln consumes Fuel and one source unit to create a real Pottery input");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,potteryKiln,
          EstablishmentTransferOperation::WithdrawGoods,int(pottery),1}).success&&
          world.view(authority).provinces[size_t(region)].stock[size_t(pottery)]==1,
          "Pottery reaches the regional stock using the ordinary public transfer");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,porcelainWorks,
          EstablishmentTransferOperation::DepositGoods,int(pottery),1}).success&&
          world.establishments()[1].stock[size_t(pottery)]==1,
          "the FinePorcelain producer receives Pottery from the regional stock");
    const auto worksite=world.candidates(authority);
    bool reportsPottery=false,reportsFuel=false,reportsPorcelain=false;
    for(size_t i=size_t(realm_worksite_candidate_begin);i<size_t(realm_worksite_candidate_begin+
        realm_worksite_candidate_sites*realm_worksite_actions_per_site);++i){
        reportsPottery|=worksite[i].kind==Intent::TransferEstablishmentGoods&&worksite[i].good==int(pottery)&&
            worksite[i].term==int(EstablishmentTransferOperation::DepositGoods);
        reportsFuel|=worksite[i].kind==Intent::TransferEstablishmentGoods&&worksite[i].good==int(GoodId::Fuel)&&
            worksite[i].term==int(EstablishmentTransferOperation::DepositGoods);
        reportsPorcelain|=worksite[i].kind==Intent::TransferEstablishmentGoods&&worksite[i].good==int(porcelain)&&
            worksite[i].term==int(EstablishmentTransferOperation::WithdrawGoods);
    }
    check(reportsPottery&&reportsFuel&&reportsPorcelain,
          "worksite actions show both exact inputs and the separate FinePorcelain output");

    const auto beforeShortage=world.fingerprint();
    schedule_work(world,authority,porcelainWorker,porcelainWorks,region);
    world.advance_ticks(96);
    check(world.establishments()[1].stock[size_t(porcelain)]==1&&
          world.establishments()[1].stock[size_t(pottery)]==0&&
          world.establishments()[1].stock[size_t(GoodId::Fuel)]==0&&world.invariant(),
          "one Pottery plus one Fuel and24 Work ticks create one FinePorcelain");
    check(beforeShortage!=world.fingerprint(),"a completed production transaction changes the world fingerprint");
    const auto afterBatch=world.fingerprint();
    schedule_work(world,authority,buyer,porcelainWorks,region);
    world.advance_ticks(96);
    check(world.establishments()[1].stock[size_t(porcelain)]==1&&
          world.establishments()[1].stock[size_t(pottery)]==0&&
          world.establishments()[1].stock[size_t(GoodId::Fuel)]==0&&world.invariant()&&afterBatch!=world.fingerprint(),
          "a second scheduled batch cannot consume only one missing input or create partial output");

    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,porcelainWorks,
          EstablishmentTransferOperation::WithdrawGoods,int(porcelain),1}).success&&
          world.view(authority).provinces[size_t(region)].stock[size_t(porcelain)]==1,
          "the finished porcelain enters regional stock for ordinary purchase");
    const int64_t price=world.view(buyer).provinces[size_t(region)].price[size_t(porcelain)];
    fund_household_member(world,buyer,price+12);
    RealmFinePorcelainTestAccess::set_cash(world,buyer,price+12);
    check(world.person(buyer).cash>=price+12,"the buyer has enough personal money for purchase and social use");
    check(world.execute(MarketOrderCommand{buyer,world.person(buyer).decisions+1,0,MarketOrderOperation::Place,
          region,int(porcelain),1,price}).success,"the civilian places a regular market order for FinePorcelain");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.person(buyer).inventory[size_t(porcelain)]==1&&world.invariant(),
          "market settlement gives FinePorcelain to the buyer's personal inventory");
    const auto social=world.candidates(buyer)[realm_fine_porcelain_social_candidate];
    check(social.kind==Intent::Social&&social.good==int(porcelain)&&social.amount==12&&social.term==7&&
          Realm::action_name(social)=="Tiếp khách bằng đồ sứ tinh xảo"&&
          world.mask(buyer)[realm_fine_porcelain_social_candidate]==1.f,
          "an eligible buyer sees the correct hosting action, twelve-coin cost and seven-day duration");
    const int64_t cashBefore=world.person(buyer).cash;
    const int socialBefore=world.person(buyer).social;
    const int64_t regionalFund=world.view(buyer).provinces[size_t(region)].firm;
    const int64_t consumedBefore=RealmFinePorcelainTestAccess::consumed(world);
    check(world.execute({buyer,world.person(buyer).decisions+1,social}).success&&
          world.person(buyer).cash==cashBefore-12&&world.view(buyer).provinces[size_t(region)].firm==regionalFund+12&&
          world.person(buyer).inventory[size_t(porcelain)]==0&&
          RealmFinePorcelainTestAccess::consumed(world)==consumedBefore+1&&
          world.person(buyer).social==std::min(100,socialBefore+18)&&
          world.person(buyer).busy_until==world.day()+7&&world.invariant(),
          "hosting consumes the personal item, pays the region, adds18 social and occupies seven game days");

    const auto saved=world.serialize("fine-porcelain-production-use-v1");
    check(saved.rfind("POLYEC_REALM_73 ",0)==0,"new save writes schema73 for FinePorcelain Works");
    Realm restored(1,1,128);std::string error;
    const bool roundtripOk=restored.deserialize(saved,"fine-porcelain-production-use-v1",error);
    check(roundtripOk&&
          restored.fingerprint()==world.fingerprint()&&restored.invariant()&&
          restored.establishments().size()==2&&
          RealmFinePorcelainTestAccess::consumed(restored)==consumedBefore+1,
          "schema73 roundtrip preserves both producers, stock, social use, money and busy time: "+error);
    std::string legacy=Realm(20261089,int(kAtlasTerritoryCount)-1,256).serialize("fine-porcelain-legacy");
    legacy.replace(0,std::string("POLYEC_REALM_73").size(),"POLYEC_REALM_72");
    Realm legacyLoaded(1,1,128);
    check(legacyLoaded.deserialize(legacy,"fine-porcelain-legacy",error)&&
          legacyLoaded.serialize("fine-porcelain-legacy").rfind("POLYEC_REALM_73 ",0)==0,
          "schema72 worlds without the new industry still load and write the newest save version");

    std::string invalidLegacy=saved;
    invalidLegacy.replace(0,std::string("POLYEC_REALM_73").size(),"POLYEC_REALM_72");
    Realm rejectedLegacy(1,1,128);
    check(!rejectedLegacy.deserialize(invalidLegacy,"porcelain-glass-invalid-legacy",error),
          "a schema72 save cannot smuggle in the schema73-only FinePorcelain Works");

    Realm boundary(20261090,int(kAtlasTerritoryCount)-1,256);
    const uint64_t boundaryBuyer=civilian_at(boundary,region);
    RealmFinePorcelainTestAccess::give(boundary,boundaryBuyer);
    RealmFinePorcelainTestAccess::set_social(boundary,boundaryBuyer,95);
    const auto boundaryAction=boundary.candidates(boundaryBuyer)[realm_fine_porcelain_social_candidate];
    check(boundary.execute({boundaryBuyer,boundary.person(boundaryBuyer).decisions+1,boundaryAction}).success&&
          boundary.person(boundaryBuyer).social==100,"the FinePorcelain hosting benefit cannot exceed100");

    Realm invalid(20261091,int(kAtlasTerritoryCount)-1,256);
    const uint64_t invalidActor=civilian_at(invalid,region);
    RealmFinePorcelainTestAccess::give(invalid,invalidActor);
    RealmFinePorcelainTestAccess::set_age(invalid,invalidActor,17*365);
    const auto beforeReject=invalid.fingerprint();
    check(invalid.candidates(invalidActor)[realm_fine_porcelain_social_candidate].kind==Intent::Hold&&
          !invalid.execute({invalidActor,invalid.person(invalidActor).decisions+1,
              {Intent::Social,region,int(porcelain),12,7}}).success&&invalid.fingerprint()==beforeReject,
          "a person younger than18 cannot host a FinePorcelain event and the rejection is atomic");

    Realm exhausted(20261092,int(kAtlasTerritoryCount)-1,256);
    const uint64_t exhaustedActor=civilian_at(exhausted,region);
    RealmFinePorcelainTestAccess::give(exhausted,exhaustedActor);
    RealmFinePorcelainTestAccess::set_consumed(exhausted,std::numeric_limits<int64_t>::max());
    const auto exhaustedBefore=exhausted.fingerprint();
    check(!exhausted.execute({exhaustedActor,exhausted.person(exhaustedActor).decisions+1,
              {Intent::Social,region,int(porcelain),12,7}}).success&&exhausted.fingerprint()==exhaustedBefore,
          "a full consumer counter rejects without partially spending money or the item");
    std::cout<<"FinePorcelain production and hosting checks: "<<checks<<"\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"FAILED after "<<checks<<" checks: "<<e.what()<<"\n";return 1;}}
