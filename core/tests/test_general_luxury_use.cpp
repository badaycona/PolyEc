#include "polyec/realm.hpp"
#include "current_save_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmGeneralLuxuryTestAccess {
    static void seed_region_good(Realm& world,int region,GoodId good,int64_t amount){
        world.provinces_[size_t(region)].stock[size_t(good)]+=amount;
        world.opening_[size_t(good)]+=amount;
    }
    static void set_cash(Realm& world,uint64_t actor,int64_t value){
        auto& person=world.mutable_person(actor);world.opening_money_+=value-person.cash;person.cash=value;
    }
    static void set_age(Realm& world,uint64_t actor,int value){world.mutable_person(actor).age_days=value;}
    static void set_busy_until(Realm& world,uint64_t actor,int value){world.mutable_person(actor).busy_until=value;}
    static void set_service(Realm& world,uint64_t actor,bool active){world.mutable_person(actor).military_service.active=active;}
    static void set_social(Realm& world,uint64_t actor,int value){world.mutable_person(actor).social=value;}
    static void set_consumed(Realm& world,GoodId good,int64_t value){world.consumed_[size_t(good)]=value;}
    static int64_t consumed(const Realm& world,GoodId good){return world.consumed_[size_t(good)];}
};
}

namespace {
int checks=0;
void check(bool ok,const std::string& message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t adult_civilian(const Realm& world,int region,uint64_t exclude=0){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& p=world.person(id);
        if(id!=exclude&&p.alive&&p.destination==-1&&p.region==region&&p.family==Family::Civilian&&p.age_days>=18*365)
            return id;
    }
    return 0;
}
bool transfer(Realm& world,uint64_t manager,uint64_t site,EstablishmentTransferOperation op,GoodId good,int amount=1){
    return world.execute(EstablishmentTransferCommand{manager,world.person(manager).decisions+1,site,op,int(good),amount}).success;
}
void work(Realm& world,uint64_t manager,uint64_t worker,uint64_t site,int region,int64_t duration){
    check(world.execute(WorkAuthorizationCommand{manager,world.person(manager).decisions+1,site,worker,
        WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+duration}).success,
        "manager grants a normal work authorization at the selected site");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
        0,world.game_tick(),world.game_tick()+duration,0,1000,ActivityKind::Work,region,site}).success,
        "worker submits a normal work schedule through the command interface");
}
void process_day(Realm& world,uint64_t manager,uint64_t worker,uint64_t site,int region,int64_t duration){
    work(world,manager,worker,site,region,duration);world.advance_ticks(kSubdayUnitsPerDay);
}
}

int main(){try{
    constexpr int region=12;
    constexpr auto tea=GoodId::Tea;
    constexpr auto incense=GoodId::Incense;
    constexpr auto silk=GoodId::Silk;
    constexpr auto luxury=GoodId::GeneralLuxury;
    constexpr const char* saveName="general-luxury-production-use-v1";
    Realm world(20261089,int(kAtlasTerritoryCount)-1,512);
    const uint64_t authority=region+2;
    const uint64_t worker=adult_civilian(world,region);
    const uint64_t buyer=adult_civilian(world,region,worker);
    check(world.person(authority).taxing_authority&&worker&&buyer,
          "Atlas region12 provides its public authority and two adult civilians");
    check(realm_schema==std::string_view("realm-v43")&&
          world.model_contract()=="realm-household-fund-goods30-config-v54"&&realm_actions==400&&realm_obs==5182,
          "adding the luxury product versions the save/model contracts without changing model tensor sizes");
    check(world.candidates(worker)[realm_general_luxury_social_candidate].kind==Intent::Hold,
          "the private luxury event is absent until the Civilian owns the product and can pay");

    const auto farmCandidate=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin)];
    const auto atelierCandidate=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin+3)];
    check(farmCandidate.kind==Intent::CreateEstablishment&&farmCandidate.good==int(EstablishmentIndustry::SilkwormFarm)&&
          atelierCandidate.kind==Intent::CreateEstablishment&&atelierCandidate.good==int(EstablishmentIndustry::GeneralLuxuryAtelier)&&
          world.mask(authority)[size_t(realm_specialty_product_candidate_begin+3)]==1.f,
          "region12 retains the Silkworm Farm slots and exposes the separate General Luxury atelier");
    const auto farmResult=world.execute({authority,world.person(authority).decisions+1,farmCandidate});
    const auto atelierResult=world.execute({authority,world.person(authority).decisions+1,atelierCandidate});
    check(farmResult.success&&atelierResult.success&&world.establishments().size()==2&&world.invariant(),
          "the public Farm and atelier can both be created with ordinary funding commands");
    const uint64_t farm=farmResult.session,atelier=atelierResult.session;
    check(world.establishments()[1].industry==EstablishmentIndustry::GeneralLuxuryAtelier&&
          world.establishments()[1].source_good==kNoEstablishmentSourceGood&&world.invariant(),
          "the atelier has its own identity and does not pretend finished goods are a renewable source");
    const auto wrongRegion=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),11,EstablishmentIndustry::GeneralLuxuryAtelier,1,25});
    const auto duplicate=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),region,EstablishmentIndustry::GeneralLuxuryAtelier,1,25});
    check(!wrongRegion.success&&!duplicate.success&&world.establishments().size()==2&&world.invariant(),
          "wrong-region and duplicate luxury ateliers reject without changing the world");

    // Tea and Incense are test-stocked in the region: their production and long-distance transport
    // are separate systems; this test starts at the normal regional stock/transfer boundary.
    RealmGeneralLuxuryTestAccess::seed_region_good(world,region,tea,2);
    RealmGeneralLuxuryTestAccess::seed_region_good(world,region,incense,2);
    check(transfer(world,authority,atelier,EstablishmentTransferOperation::DepositGoods,tea)&&
          transfer(world,authority,atelier,EstablishmentTransferOperation::DepositGoods,incense),
          "Tea and Incense move from the regional stores into the atelier using ordinary transfer commands");
    process_day(world,authority,worker,farm,region,32);
    check(world.establishments()[0].stock[size_t(silk)]==1&&world.establishments()[0].source_units==24&&world.invariant(),
          "the existing Silkworm Farm turns Work and one finite source unit into saleable Silk");
    check(transfer(world,authority,farm,EstablishmentTransferOperation::WithdrawGoods,silk)&&
          transfer(world,authority,atelier,EstablishmentTransferOperation::DepositGoods,silk),
          "newly produced Silk travels through regional stock into the atelier by ordinary transfers");
    check(world.establishments()[1].stock[size_t(tea)]==1&&world.establishments()[1].stock[size_t(incense)]==1&&
          world.establishments()[1].stock[size_t(silk)]==1,
          "the atelier holds one actual unit of each of its three required ingredients");

    for(const auto missing:{tea,incense,silk}){
        check(transfer(world,authority,atelier,EstablishmentTransferOperation::WithdrawGoods,missing),
              "one recipe input can be withdrawn without touching the other two");
        const auto before=world.fingerprint();
        process_day(world,authority,worker,atelier,region,48);
        check(world.establishments()[1].stock[size_t(luxury)]==0&&
              world.establishments()[1].stock[size_t(tea)]>=0&&world.establishments()[1].stock[size_t(incense)]>=0&&
              world.establishments()[1].stock[size_t(silk)]>=0&&world.invariant()&&before!=world.fingerprint(),
              "work with one missing ingredient creates no partial luxury output or negative inventory");
        check(transfer(world,authority,atelier,EstablishmentTransferOperation::DepositGoods,missing),
              "the missing ingredient can be returned before the next shortage case");
    }
    const auto teaConsumed=RealmGeneralLuxuryTestAccess::consumed(world,tea);
    const auto incenseConsumed=RealmGeneralLuxuryTestAccess::consumed(world,incense);
    const auto silkConsumed=RealmGeneralLuxuryTestAccess::consumed(world,silk);
    process_day(world,authority,worker,atelier,region,48);
    check(world.establishments()[1].stock[size_t(luxury)]==3&&
          world.establishments()[1].stock[size_t(tea)]==0&&world.establishments()[1].stock[size_t(incense)]==0&&
          world.establishments()[1].stock[size_t(silk)]==0&&
          RealmGeneralLuxuryTestAccess::consumed(world,tea)==teaConsumed+1&&
          RealmGeneralLuxuryTestAccess::consumed(world,incense)==incenseConsumed+1&&
          RealmGeneralLuxuryTestAccess::consumed(world,silk)==silkConsumed+1&&world.invariant(),
          "three completed Work units consume all three ingredients once and create exactly three products");

    check(transfer(world,authority,atelier,EstablishmentTransferOperation::WithdrawGoods,luxury,3)&&
          world.view(authority).provinces[size_t(region)].stock[size_t(luxury)]==3,
          "the finished goods enter the region store for ordinary market sales");
    const int64_t price=world.view(buyer).provinces[size_t(region)].price[size_t(luxury)];
    RealmGeneralLuxuryTestAccess::set_cash(world,buyer,price+20);
    check(world.execute(MarketOrderCommand{buyer,world.person(buyer).decisions+1,0,MarketOrderOperation::Place,
          region,int(luxury),1,price}).success,"a Civilian places an ordinary order for one finished luxury item");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.person(buyer).inventory[size_t(luxury)]==1&&world.invariant(),
          "the market delivers one finished item into the buyer's personal inventory");
    const auto event=world.candidates(buyer)[realm_general_luxury_social_candidate];
    check(event.kind==Intent::Social&&event.good==int(luxury)&&event.amount==20&&event.term==14&&
          Realm::action_name(event)=="Tổ chức dạ hội xa xỉ"&&
          world.mask(buyer)[realm_general_luxury_social_candidate]==1.f,
          "an eligible adult Civilian sees the correctly named private event and its price and duration");
    const int ageBefore=world.person(buyer).age_days;
    RealmGeneralLuxuryTestAccess::set_age(world,buyer,17*365);
    auto rejectBefore=world.fingerprint();
    check(!world.execute({buyer,world.person(buyer).decisions+1,event}).success&&world.fingerprint()==rejectBefore,
          "a person younger than18 cannot start the event and the rejected command changes nothing");
    RealmGeneralLuxuryTestAccess::set_age(world,buyer,ageBefore);
    RealmGeneralLuxuryTestAccess::set_service(world,buyer,true);rejectBefore=world.fingerprint();
    check(!world.execute({buyer,world.person(buyer).decisions+1,event}).success&&world.fingerprint()==rejectBefore,
          "a person in military service cannot start the private event");
    RealmGeneralLuxuryTestAccess::set_service(world,buyer,false);
    RealmGeneralLuxuryTestAccess::set_busy_until(world,buyer,world.day()+1);rejectBefore=world.fingerprint();
    check(!world.execute({buyer,world.person(buyer).decisions+1,event}).success&&world.fingerprint()==rejectBefore,
          "a person already committed to another activity cannot start the event");
    RealmGeneralLuxuryTestAccess::set_busy_until(world,buyer,world.day());
    const int64_t eligibleCash=world.person(buyer).cash;
    RealmGeneralLuxuryTestAccess::set_cash(world,buyer,19);rejectBefore=world.fingerprint();
    check(world.mask(buyer)[realm_general_luxury_social_candidate]==0.f&&
          !world.execute({buyer,world.person(buyer).decisions+1,event}).success&&world.fingerprint()==rejectBefore,
          "having only19 personal coins hides and atomically rejects the20-coin event");
    RealmGeneralLuxuryTestAccess::set_cash(world,buyer,eligibleCash);
    const auto priorConsumed=RealmGeneralLuxuryTestAccess::consumed(world,luxury);
    RealmGeneralLuxuryTestAccess::set_consumed(world,luxury,std::numeric_limits<int64_t>::max());rejectBefore=world.fingerprint();
    check(!world.execute({buyer,world.person(buyer).decisions+1,event}).success&&world.fingerprint()==rejectBefore,
          "an exhausted product ledger rejects before charging money or consuming the personal item");
    RealmGeneralLuxuryTestAccess::set_consumed(world,luxury,priorConsumed);
    RealmGeneralLuxuryTestAccess::set_social(world,buyer,95);
    const int64_t cashBefore=world.person(buyer).cash;
    const int64_t fundBefore=world.view(buyer).provinces[size_t(region)].firm;
    check(world.execute({buyer,world.person(buyer).decisions+1,event}).success&&
          world.person(buyer).cash==cashBefore-20&&world.view(buyer).provinces[size_t(region)].firm==fundBefore+20&&
          world.person(buyer).inventory[size_t(luxury)]==0&&world.person(buyer).social==100&&
          world.person(buyer).busy_until==world.day()+14&&world.invariant(),
          "the event consumes the personal item, pays the regional fund, raises personal social and occupies14 game days");

    const std::string saved=world.serialize(saveName);
    check(saved.rfind("POLYEC_REALM_75 ",0)==0,"the new atelier is written using save schema74");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,saveName,error)&&restored.fingerprint()==world.fingerprint()&&
          restored.establishments().size()==2&&restored.invariant(),
          "save/load preserves the Farm, atelier, transactions and private event: "+error);
    std::string oldVersion=saved;
    test_fixtures::strip_empty_apprenticeship_section(oldVersion);
    oldVersion.replace(0,std::string("POLYEC_REALM_75 ").size(),"POLYEC_REALM_73 ");
    Realm rejected(1,1,128);
    check(!rejected.deserialize(oldVersion,saveName,error),
          "schema73 cannot load a schema74 save that contains the new industry");
    Realm priorWorld(20261090,int(kAtlasTerritoryCount)-1,256);
    std::string priorSave=priorWorld.serialize("general-luxury-legacy");
    test_fixtures::strip_empty_apprenticeship_section(priorSave);
    priorSave.replace(0,std::string("POLYEC_REALM_75 ").size(),"POLYEC_REALM_73 ");
    Realm migrated(1,1,128);
    check(migrated.deserialize(priorSave,"general-luxury-legacy",error)&&
          migrated.serialize("general-luxury-legacy").rfind("POLYEC_REALM_75 ",0)==0&&migrated.invariant(),
          "schema73 worlds without the new workshop still load and write the current save format");
    std::cout<<"General Luxury production and personal event checks: "<<checks<<"\n";
    return 0;
}catch(const std::exception& error){std::cerr<<"FAILED after "<<checks<<" checks: "<<error.what()<<"\n";return 1;}}
