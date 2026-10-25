#include "polyec/realm.hpp"
#include "current_save_fixture.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmDecorativeGlassTestAccess {
    static void give(Realm& world,uint64_t actor,int64_t amount=1){
        world.mutable_person(actor).inventory[size_t(GoodId::DecorativeGlass)]+=amount;
        world.opening_[size_t(GoodId::DecorativeGlass)]+=amount;
    }
    static int64_t consumed(const Realm& world){return world.consumed_[size_t(GoodId::DecorativeGlass)];}
    static void set_consumed(Realm& world,int64_t value){world.consumed_[size_t(GoodId::DecorativeGlass)]=value;}
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
           p.age_days>=14*365)return id;
    }
    return 0;
}
void schedule_work(Realm& world,uint64_t manager,uint64_t worker,uint64_t site,int region){
    check(world.execute(WorkAuthorizationCommand{manager,world.person(manager).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
          "the owner authorizes the local worker through the normal work command");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
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
    constexpr int region=10;
    constexpr auto plain=GoodId::PlainGlass;
    constexpr auto decorative=GoodId::DecorativeGlass;
    Realm world(20261088,int(kAtlasTerritoryCount)-1,512);
    const uint64_t authority=region+2;
    const uint64_t glassWorker=civilian_at(world,region);
    const uint64_t decorativeWorker=civilian_at(world,region,glassWorker);
    const uint64_t buyer=civilian_at(world,region,decorativeWorker);
    check(authority<=uint64_t(world.count())&&world.person(authority).taxing_authority&&glassWorker&&decorativeWorker&&buyer,
          "Atlas region10 has the public authority and three local adult civilians");
    check(realm_schema==std::string_view("realm-v45")&&world.model_contract()==
          "realm-household-fund-goods30-config-v54-atlas-north-pass-v1"&&realm_actions==400&&realm_obs==5182,
          "DecorativeGlass advances the model/config versions without changing tensor dimensions");

    const auto glassCandidate=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin)];
    const auto decorativeCandidate=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin+3)];
    check(glassCandidate.kind==Intent::CreateEstablishment&&glassCandidate.good==int(EstablishmentIndustry::Glassworks)&&
          decorativeCandidate.kind==Intent::CreateEstablishment&&
          decorativeCandidate.good==int(EstablishmentIndustry::DecorativeGlassWorks)&&
          world.mask(authority)[size_t(realm_specialty_product_candidate_begin+3)]==1.f,
          "region10 keeps PlainGlass Glassworks and exposes a separate DecorativeGlass producer");
    const auto glassResult=world.execute({authority,world.person(authority).decisions+1,glassCandidate});
    const auto decorativeResult=world.execute({authority,world.person(authority).decisions+1,decorativeCandidate});
    check(glassResult.success&&decorativeResult.success&&world.establishments().size()==2&&world.invariant(),
          "both public producers can be funded and created in region10");
    const uint64_t glassworks=glassResult.session,decorativeWorks=decorativeResult.session;
    check(world.establishments()[0].industry==EstablishmentIndustry::Glassworks&&
          world.establishments()[1].industry==EstablishmentIndustry::DecorativeGlassWorks&&
          world.establishments()[1].source_good==kNoEstablishmentSourceGood&&world.invariant(),
          "the new workshop has no fake finite source and remains distinct from the PlainGlass source");
    const auto wrongRegion=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),9,EstablishmentIndustry::DecorativeGlassWorks,1,25});
    const auto duplicate=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),region,EstablishmentIndustry::DecorativeGlassWorks,1,25});
    check(!wrongRegion.success&&!duplicate.success&&world.establishments().size()==2&&world.invariant(),
          "wrong-region and duplicate DecorativeGlass workshop creation reject atomically");

    const int64_t fuel=world.view(authority).provinces[size_t(region)].stock[size_t(GoodId::Fuel)];
    check(fuel>=2&&world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,
          glassworks,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1}).success&&
          world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,
          decorativeWorks,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1}).success,
          "Fuel from the regional stock is transferred separately into both production sites");
    schedule_work(world,authority,decorativeWorker,decorativeWorks,region);
    world.advance_ticks(64);
    check(world.establishments()[1].stock[size_t(decorative)]==0&&
          world.establishments()[1].stock[size_t(GoodId::Fuel)]==1&&world.invariant(),
          "Fuel alone cannot be consumed or create partial output while PlainGlass is missing");
    schedule_work(world,authority,glassWorker,glassworks,region);
    world.advance_ticks(64);
    check(world.establishments()[0].stock[size_t(plain)]==1&&
          world.establishments()[0].stock[size_t(GoodId::Fuel)]==0&&
          world.establishments()[0].source_units==99&&world.invariant(),
          "Glassworks consumes Fuel and one source unit to create a real PlainGlass input");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,glassworks,
          EstablishmentTransferOperation::WithdrawGoods,int(plain),1}).success&&
          world.view(authority).provinces[size_t(region)].stock[size_t(plain)]==1,
          "PlainGlass reaches the regional stock using the ordinary public transfer");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,decorativeWorks,
          EstablishmentTransferOperation::DepositGoods,int(plain),1}).success&&
          world.establishments()[1].stock[size_t(plain)]==1,
          "the DecorativeGlass producer receives PlainGlass from the regional stock");
    const auto worksite=world.candidates(authority);
    bool reportsPlain=false,reportsFuel=false,reportsDecorative=false;
    for(size_t i=size_t(realm_worksite_candidate_begin);i<size_t(realm_worksite_candidate_begin+
        realm_worksite_candidate_sites*realm_worksite_actions_per_site);++i){
        reportsPlain|=worksite[i].kind==Intent::TransferEstablishmentGoods&&worksite[i].good==int(plain)&&
            worksite[i].term==int(EstablishmentTransferOperation::DepositGoods);
        reportsFuel|=worksite[i].kind==Intent::TransferEstablishmentGoods&&worksite[i].good==int(GoodId::Fuel)&&
            worksite[i].term==int(EstablishmentTransferOperation::DepositGoods);
        reportsDecorative|=worksite[i].kind==Intent::TransferEstablishmentGoods&&worksite[i].good==int(decorative)&&
            worksite[i].term==int(EstablishmentTransferOperation::WithdrawGoods);
    }
    check(reportsPlain&&reportsFuel&&reportsDecorative,
          "worksite actions show both exact inputs and the separate DecorativeGlass output");

    const auto beforeShortage=world.fingerprint();
    schedule_work(world,authority,decorativeWorker,decorativeWorks,region);
    world.advance_ticks(64);
    check(world.establishments()[1].stock[size_t(decorative)]==1&&
          world.establishments()[1].stock[size_t(plain)]==0&&
          world.establishments()[1].stock[size_t(GoodId::Fuel)]==0&&world.invariant(),
          "one PlainGlass plus one Fuel and16 Work ticks create one DecorativeGlass");
    check(beforeShortage!=world.fingerprint(),"a completed production transaction changes the world fingerprint");
    const auto afterBatch=world.fingerprint();
    schedule_work(world,authority,buyer,decorativeWorks,region);
    world.advance_ticks(64);
    check(world.establishments()[1].stock[size_t(decorative)]==1&&
          world.establishments()[1].stock[size_t(plain)]==0&&
          world.establishments()[1].stock[size_t(GoodId::Fuel)]==0&&world.invariant()&&afterBatch!=world.fingerprint(),
          "a second scheduled batch cannot consume only one missing input or create partial output");

    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,decorativeWorks,
          EstablishmentTransferOperation::WithdrawGoods,int(decorative),1}).success&&
          world.view(authority).provinces[size_t(region)].stock[size_t(decorative)]==1,
          "the finished glass enters regional stock for ordinary purchase");
    const int64_t price=world.view(buyer).provinces[size_t(region)].price[size_t(decorative)];
    fund_household_member(world,buyer,price+6);
    RealmDecorativeGlassTestAccess::set_cash(world,buyer,price+6);
    check(world.person(buyer).cash>=price+6,"the buyer has enough personal money for purchase and social use");
    check(world.execute(MarketOrderCommand{buyer,world.person(buyer).decisions+1,0,MarketOrderOperation::Place,
          region,int(decorative),1,price}).success,"the civilian places a regular market order for DecorativeGlass");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.person(buyer).inventory[size_t(decorative)]==1&&world.invariant(),
          "market settlement gives the finished item to the buyer's personal inventory");
    const auto social=world.candidates(buyer)[realm_decorative_glass_social_candidate];
    check(social.kind==Intent::Social&&social.good==int(decorative)&&social.amount==6&&social.term==7&&
          Realm::action_name(social)=="Giao lưu bằng kính trang trí"&&
          world.mask(buyer)[realm_decorative_glass_social_candidate]==1.f,
          "an eligible buyer sees the correct social action, six-coin cost and seven-day duration");
    const int64_t cashBefore=world.person(buyer).cash;
    const int socialBefore=world.person(buyer).social;
    const int64_t regionalFund=world.view(buyer).provinces[size_t(region)].firm;
    const int64_t consumedBefore=RealmDecorativeGlassTestAccess::consumed(world);
    check(world.execute({buyer,world.person(buyer).decisions+1,social}).success&&
          world.person(buyer).cash==cashBefore-6&&world.view(buyer).provinces[size_t(region)].firm==regionalFund+6&&
          world.person(buyer).inventory[size_t(decorative)]==0&&
          RealmDecorativeGlassTestAccess::consumed(world)==consumedBefore+1&&
          world.person(buyer).social==std::min(100,socialBefore+15)&&
          world.person(buyer).busy_until==world.day()+7&&world.invariant(),
          "social use consumes the personal item, pays the region, adds15 social and occupies seven game days");

    const auto saved=world.serialize("decorative-glass-production-use-v1");
    check(saved.rfind("POLYEC_REALM_75 ",0)==0,"new save writes schema72 for DecorativeGlass Works");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,"decorative-glass-production-use-v1",error)&&
          restored.fingerprint()==world.fingerprint()&&restored.invariant()&&
          restored.establishments().size()==2&&
          RealmDecorativeGlassTestAccess::consumed(restored)==consumedBefore+1,
          "schema72 roundtrip preserves both producers, stock, social use, money and busy time: "+error);
    std::string legacy=Realm(20261089,int(kAtlasTerritoryCount)-1,256).serialize("decorative-glass-legacy");
    test_fixtures::strip_empty_apprenticeship_section(legacy);
    legacy.replace(0,std::string("POLYEC_REALM_72").size(),"POLYEC_REALM_71");
    Realm legacyLoaded(1,1,128);
    check(legacyLoaded.deserialize(legacy,"decorative-glass-legacy",error)&&
          legacyLoaded.serialize("decorative-glass-legacy").rfind("POLYEC_REALM_75 ",0)==0,
          "schema71 worlds without the new industry still load and write the newest save version");

    std::string invalidLegacy=saved;
    test_fixtures::strip_empty_apprenticeship_section(invalidLegacy);
    invalidLegacy.replace(0,std::string("POLYEC_REALM_72").size(),"POLYEC_REALM_71");
    Realm rejectedLegacy(1,1,128);
    check(!rejectedLegacy.deserialize(invalidLegacy,"decorative-glass-invalid-legacy",error),
          "a schema71 save cannot smuggle in the schema72-only DecorativeGlass Works");

    Realm boundary(20261090,int(kAtlasTerritoryCount)-1,256);
    const uint64_t boundaryBuyer=civilian_at(boundary,region);
    RealmDecorativeGlassTestAccess::give(boundary,boundaryBuyer);
    RealmDecorativeGlassTestAccess::set_social(boundary,boundaryBuyer,95);
    const auto boundaryAction=boundary.candidates(boundaryBuyer)[realm_decorative_glass_social_candidate];
    check(boundary.execute({boundaryBuyer,boundary.person(boundaryBuyer).decisions+1,boundaryAction}).success&&
          boundary.person(boundaryBuyer).social==100,"the DecorativeGlass social benefit cannot exceed100");

    Realm invalid(20261091,int(kAtlasTerritoryCount)-1,256);
    const uint64_t invalidActor=civilian_at(invalid,region);
    RealmDecorativeGlassTestAccess::give(invalid,invalidActor);
    RealmDecorativeGlassTestAccess::set_age(invalid,invalidActor,13*365);
    const auto beforeReject=invalid.fingerprint();
    check(invalid.candidates(invalidActor)[realm_decorative_glass_social_candidate].kind==Intent::Hold&&
          !invalid.execute({invalidActor,invalid.person(invalidActor).decisions+1,
              {Intent::Social,region,int(decorative),6,7}}).success&&invalid.fingerprint()==beforeReject,
          "a person younger than14 cannot consume DecorativeGlass socially and the rejection is atomic");

    Realm exhausted(20261092,int(kAtlasTerritoryCount)-1,256);
    const uint64_t exhaustedActor=civilian_at(exhausted,region);
    RealmDecorativeGlassTestAccess::give(exhausted,exhaustedActor);
    RealmDecorativeGlassTestAccess::set_consumed(exhausted,std::numeric_limits<int64_t>::max());
    const auto exhaustedBefore=exhausted.fingerprint();
    check(!exhausted.execute({exhaustedActor,exhausted.person(exhaustedActor).decisions+1,
              {Intent::Social,region,int(decorative),6,7}}).success&&exhausted.fingerprint()==exhaustedBefore,
          "a full consumer counter rejects without partially spending money or the item");
    std::cout<<"DecorativeGlass production and social-use checks: "<<checks<<"\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"FAILED after "<<checks<<" checks: "<<e.what()<<"\n";return 1;}}
