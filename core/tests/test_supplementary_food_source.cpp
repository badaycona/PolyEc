#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace polyec;

namespace polyec {
struct RealmSupplementaryFoodTestAccess {
    static void set_source(Realm& world,uint64_t site,int64_t amount){
        auto found=std::find_if(world.establishments_.begin(),world.establishments_.end(),
            [&](const Establishment& value){return value.id==site;});
        if(found==world.establishments_.end())throw std::runtime_error("missing seed press site");
        found->source_units=amount;
    }
    static void prepare_volunteer(Realm& world,uint64_t actor,uint64_t commander){
        auto& p=world.people_.at(size_t(actor-1));
        p.nutrition=50;p.health=60;
        auto& source=world.provinces_.at(size_t(p.region)).treasury;
        constexpr int64_t escrow=2*kMilitaryServiceTrialWagePerDay;
        if(source<escrow)throw std::runtime_error("test service treasury cannot fund escrow");
        source-=escrow;
        p.military_service.active=true;
        p.military_service.commander=commander;
        p.military_service.start_day=world.day_;
        p.military_service.end_day=world.day_+2;
        p.military_service.source_region=p.region;
        p.military_service.wage_per_day=kMilitaryServiceTrialWagePerDay;
        p.military_service.escrow=escrow;
    }
    static int64_t consumed(const Realm& world){return world.consumed_[size_t(GoodId::SupplementaryFood)];}
    static int64_t consumed_good(const Realm& world,GoodId good){return world.consumed_[size_t(good)];}
    static int64_t depot(const Realm& world,int region){return world.military_supplementary_ration_depots_.at(size_t(region));}
    static void give_personal_good(Realm& world,uint64_t actor,GoodId good,int64_t amount){
        world.mutable_person(actor).inventory[size_t(good)]+=amount;
        world.opening_[size_t(good)]+=amount;
    }
    static void clear_region_dry_food(Realm& world,int region){
        auto& stock=world.provinces_.at(size_t(region)).stock[size_t(GoodId::DryFood)];
        world.consumed_[size_t(GoodId::DryFood)]+=stock;
        stock=0;
    }
    static void set_nutrition(Realm& world,uint64_t actor,int value){world.mutable_person(actor).nutrition=value;}
    static int event_count(const Realm& world,uint64_t actor,int kind){
        return int(std::count_if(world.facts_.begin(),world.facts_.end(),[&](const Fact& fact){
            return fact.actor==actor&&fact.kind==kind;
        }));
    }
    static void run_day_people(Realm& world){world.day_people();}
};
}

namespace {
int checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
uint64_t find_person(const Realm& world,int region,Family family,bool meal_day=false){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& p=world.person(id);
        if(p.alive&&p.destination==-1&&p.region==region&&p.family==family&&p.age_days>=14*365&&
           (!meal_day||int(id%7)==world.day()%7))return id;
    }
    return 0;
}
}

int main(){try{
    constexpr int region=2;
    constexpr size_t supplementary=size_t(GoodId::SupplementaryFood);
    static_assert(int(GoodId::SupplementaryFood)==1);
    static_assert(realm_actions==400&&realm_obs==5182);
    Realm world(73102,26,512);
    const uint64_t governor=uint64_t(region+2);
    const uint64_t worker=find_person(world,region,Family::Civilian);
    check(world.person(governor).taxing_authority&&worker,
          "Phù Sa has its public authority and an adult worker");
    const auto candidates=world.candidates(governor);
    const auto& create=candidates[size_t(realm_specialty_product_candidate_begin+2)];
    check(create.kind==Intent::CreateEstablishment&&create.target==region&&
          create.good==int(EstablishmentIndustry::SeedPressWorks)&&
          world.mask(governor,candidates)[size_t(realm_specialty_product_candidate_begin+2)]==1.f,
          "the region 2 authority receives the Seed Press Works choice without adding action slots");
    const auto created=world.execute({governor,world.person(governor).decisions+1,create});
    check(created.success&&world.establishments().size()==1,
          "the authority creates the source through the ordinary establishment command");
    const uint64_t site=created.session;
    check(world.establishments()[0].industry==EstablishmentIndustry::SeedPressWorks&&
          world.establishments()[0].source_good==int(GoodId::SupplementaryFood)&&
          world.establishments()[0].source_units==100&&world.establishments()[0].cash==100,
          "the site has100 source units, a separate100 đồng operating fund and no product stock");
    const auto wrongRegion=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,2,1,EstablishmentIndustry::SeedPressWorks,1,25});
    check(!wrongRegion.success&&world.establishments().size()==1&&world.invariant(),
          "a direct command rejects a source site outside Phù Sa without partial changes");
    const auto duplicate=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,2,2,EstablishmentIndustry::SeedPressWorks,1,25});
    check(!duplicate.success&&world.establishments().size()==1,
          "a second regional source site is rejected");

    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,site,worker,
        WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
        "the owner authorizes an adult worker for the scheduled interval");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
        ActivityKind::Work,region,site}).success,
        "the worker submits legal Work through the shared schedule command");
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==99&&
          world.establishments()[0].stock[supplementary]==1&&
          world.establishments()[0].stock[size_t(GoodId::DryFood)]==0&&world.invariant(),
          "one complete16-tick production unit creates one SupplementaryFood and consumes one internal source unit");
    RealmSupplementaryFoodTestAccess::set_source(world,site,0);
    const auto secondStart=world.game_tick();
    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,site,worker,
        WorkAuthorizationOperation::Set,secondStart,secondStart+32}).success&&
        world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
            ScheduleCommand::Operation::Submit,0,secondStart,secondStart+32,0,1000,
            ActivityKind::Work,region,site}).success,
        "the worker can schedule another valid Work interval after the first batch");
    world.advance_ticks(32);
    check(world.establishments()[0].source_units==0&&
          world.establishments()[0].stock[supplementary]==1&&world.invariant(),
          "an exhausted source cannot produce a second batch even when another complete Work interval exists");
    RealmSupplementaryFoodTestAccess::set_source(world,site,99);
    const auto saved=world.serialize("supplementary-food-source-v1");
    check(saved.rfind("POLYEC_REALM_72 ",0)==0&&saved.find(" ESTABLISH_14 1 ")!=std::string::npos,
          "schema64 records both new industries while keeping establishment records readable");
    Realm loaded(1,1,128);std::string error;
    check(loaded.deserialize(saved,"supplementary-food-source-v1",error)&&
          loaded.fingerprint()==world.fingerprint()&&loaded.establishments()[0].source_units==99&&
          loaded.establishments()[0].stock[supplementary]==1,
          "save/load preserves both source and product without repeating production");

    auto malformed=loaded.serialize("supplementary-food-source-v1");
    const auto establishment=malformed.find("ESTABLISH_14 1 ");
    check(establishment!=std::string::npos,"the establishment record is present for a malformed-save check");
    // The record begins with id, region, owner-kind, owner-id, then industry.
    size_t regionToken=malformed.find(' ',establishment+std::string("ESTABLISH_14 1 ").size());
    ++regionToken;size_t afterRegion=malformed.find(' ',regionToken);
    malformed.replace(regionToken,afterRegion-regionToken,"1");
    const auto before=loaded.fingerprint();
    check(!loaded.deserialize(malformed,"supplementary-food-source-v1",error)&&loaded.fingerprint()==before,
          "a save that moves Seed Press Works out of region 2 is rejected atomically");

    // Feed the existing volunteer consumer from the new producer in the same
    // territory: garden stock -> market stock -> public ration depot -> meal.
    const auto withdrawn=loaded.execute(EstablishmentTransferCommand{governor,
        loaded.person(governor).decisions+1,site,EstablishmentTransferOperation::WithdrawGoods,
        int(GoodId::SupplementaryFood),1});
    check(withdrawn.success&&loaded.view(governor).provinces[region].stock[supplementary]>0,
          "the owner transfers the actual produced SupplementaryFood into the regional market");
    const auto depotPurchase=loaded.execute(MilitaryRationPurchaseCommand{governor,
        loaded.person(governor).decisions+1,region,1,int(GoodId::SupplementaryFood)});
    check(depotPurchase.success&&RealmSupplementaryFoodTestAccess::depot(loaded,region)==1,
          "Governance purchases that market stock into the separate SupplementaryFood depot");
    constexpr int destination=3;
    const auto shipSlot=size_t(realm_supplementary_ration_shipment_candidate_begin+destination);
    check(loaded.mask(governor)[shipSlot]==1.f&&
          loaded.execute({governor,loaded.person(governor).decisions+1,
              loaded.candidates(governor)[shipSlot]}).success,
          "Governance sends the producer's purchased SupplementaryFood through the existing Atlas shipment route");
    const uint64_t shipment=loaded.transport_shipments().back().id;
    for(int day=0;day<30&&loaded.transport_shipments().back().delivered==0;++day)loaded.advance(1);
    check(loaded.transport_shipments().back().id==shipment&&
          loaded.transport_shipments().back().delivered==1&&
          RealmSupplementaryFoodTestAccess::depot(loaded,destination)==1&&loaded.invariant(),
          "the actual producer stock arrives in the destination's separate SupplementaryFood depot");
    const uint64_t destinationGovernor=uint64_t(destination+2);
    const uint64_t volunteer=find_person(loaded,destination,Family::Civilian,true);
    check(volunteer!=0,"an adult destination volunteer candidate exists on the current ration day");
    check(loaded.execute({volunteer,loaded.person(volunteer).decisions+1,
        {Intent::Buy,destination,int(GoodId::DryFood),1}}).success,
        "the volunteer obtains the normal DryFood meal before receiving the supplement");
    RealmSupplementaryFoodTestAccess::prepare_volunteer(loaded,volunteer,destinationGovernor);
    const int64_t consumedBefore=RealmSupplementaryFoodTestAccess::consumed(loaded);
    RealmSupplementaryFoodTestAccess::run_day_people(loaded);
    check(RealmSupplementaryFoodTestAccess::depot(loaded,destination)==0&&
          RealmSupplementaryFoodTestAccess::consumed(loaded)==consumedBefore+1&&
          loaded.person(volunteer).nutrition==80&&loaded.invariant(),
          "the eligible volunteer consumes exactly one depot SupplementaryFood after DryFood for +10 nutrition");

    RealmSupplementaryFoodTestAccess::set_source(loaded,site,60);
    loaded.advance(89-loaded.day());
    check(loaded.day()==89&&loaded.establishments()[0].source_units==60,
          "the source does not refill before its90-day harvest boundary");
    loaded.advance(1);
    check(loaded.day()==90&&loaded.establishments()[0].source_units==85,
          "the day90 harvest restores exactly25 units without exceeding the100-unit cap");
    const auto seasonSave=loaded.serialize("supplementary-food-source-v1");
    Realm seasonLoaded(1,1,128);
    check(seasonLoaded.deserialize(seasonSave,"supplementary-food-source-v1",error)&&
          seasonLoaded.establishments()[0].source_units==85,
          "save/load at day90 preserves the refill and does not apply it twice");
    RealmSupplementaryFoodTestAccess::set_source(seasonLoaded,site,90);
    seasonLoaded.advance(90);
    check(seasonLoaded.day()==180&&seasonLoaded.establishments()[0].source_units==100,
          "the day180 refill stops at100 units instead of overflowing the source ledger");

    std::string previousSchema=saved;
    test_fixtures::downgrade_save_64_to_63(previousSchema);
    previousSchema.replace(0,std::string("POLYEC_REALM_63").size(),"POLYEC_REALM_62");
    Realm oldVersion(1,1,128);
    check(!oldVersion.deserialize(previousSchema,"supplementary-food-source-v1",error),
          "schema62 rejects industry16 instead of silently interpreting it as an older establishment");
    check(loaded.invariant()&&seasonLoaded.invariant(),
          "the production, market, depot and seasonal source ledgers remain internally consistent");

    Realm civilianMeal(73103,26,512);
    const uint64_t civilian=find_person(civilianMeal,region,Family::Civilian,true);
    check(civilian!=0,"an ordinary Civilian has a scheduled weekly meal day");
    RealmSupplementaryFoodTestAccess::set_nutrition(civilianMeal,civilian,60);
    RealmSupplementaryFoodTestAccess::give_personal_good(civilianMeal,civilian,GoodId::DryFood,1);
    RealmSupplementaryFoodTestAccess::give_personal_good(civilianMeal,civilian,GoodId::SupplementaryFood,1);
    const auto dryBefore=RealmSupplementaryFoodTestAccess::consumed_good(civilianMeal,GoodId::DryFood);
    const auto supplementBefore=RealmSupplementaryFoodTestAccess::consumed(civilianMeal);
    RealmSupplementaryFoodTestAccess::run_day_people(civilianMeal);
    check(civilianMeal.person(civilian).nutrition==90&&
          civilianMeal.person(civilian).inventory[size_t(GoodId::DryFood)]==0&&
          civilianMeal.person(civilian).inventory[supplementary]==0&&
          RealmSupplementaryFoodTestAccess::consumed_good(civilianMeal,GoodId::DryFood)>dryBefore&&
          RealmSupplementaryFoodTestAccess::consumed(civilianMeal)==supplementBefore+1&&
          RealmSupplementaryFoodTestAccess::event_count(civilianMeal,civilian,238)==1&&
          civilianMeal.invariant(),
          "an ordinary Civilian eats one personal SupplementaryFood only after DryFood, gains10 nutrition and records the consumption");

    Realm noBaseMeal(73104,26,512,false,GameplayConfig::historical());
    const uint64_t hungry=find_person(noBaseMeal,region,Family::Civilian,true);
    check(hungry!=0,"the no-base-meal fixture has a Civilian on the weekly ration day");
    RealmSupplementaryFoodTestAccess::clear_region_dry_food(noBaseMeal,region);
    RealmSupplementaryFoodTestAccess::give_personal_good(noBaseMeal,hungry,GoodId::SupplementaryFood,1);
    RealmSupplementaryFoodTestAccess::set_nutrition(noBaseMeal,hungry,60);
    const auto noBaseSupplement=RealmSupplementaryFoodTestAccess::consumed(noBaseMeal);
    RealmSupplementaryFoodTestAccess::run_day_people(noBaseMeal);
    check(noBaseMeal.person(hungry).inventory[supplementary]==1&&
          RealmSupplementaryFoodTestAccess::consumed(noBaseMeal)==noBaseSupplement&&
          RealmSupplementaryFoodTestAccess::event_count(noBaseMeal,hungry,238)==0&&
          noBaseMeal.person(hungry).nutrition<60&&noBaseMeal.invariant(),
          "SupplementaryFood cannot replace a missing DryFood meal or be consumed after an unmet base need");

    Realm noBorrow(73105,26,512);
    const uint64_t unprovided=find_person(noBorrow,region,Family::Civilian,true);
    check(unprovided!=0,"the no-borrow fixture has a Civilian on the weekly ration day");
    const uint64_t otherOwner=unprovided==2?3:2;
    RealmSupplementaryFoodTestAccess::give_personal_good(noBorrow,unprovided,GoodId::DryFood,1);
    RealmSupplementaryFoodTestAccess::give_personal_good(noBorrow,otherOwner,GoodId::SupplementaryFood,1);
    RealmSupplementaryFoodTestAccess::set_nutrition(noBorrow,unprovided,60);
    RealmSupplementaryFoodTestAccess::run_day_people(noBorrow);
    check(noBorrow.person(unprovided).nutrition==80&&
          noBorrow.person(unprovided).inventory[supplementary]==0&&
          noBorrow.person(otherOwner).inventory[supplementary]==1&&
          RealmSupplementaryFoodTestAccess::consumed(noBorrow)==0&&noBorrow.invariant(),
          "a Civilian cannot borrow another person's SupplementaryFood even when it is the only supplement in the world");

    Realm nutritionCap(73106,26,512);
    const uint64_t full=find_person(nutritionCap,region,Family::Civilian,true);
    check(full!=0,"the nutrition-cap fixture has a Civilian on the weekly ration day");
    RealmSupplementaryFoodTestAccess::set_nutrition(nutritionCap,full,90);
    RealmSupplementaryFoodTestAccess::give_personal_good(nutritionCap,full,GoodId::DryFood,1);
    RealmSupplementaryFoodTestAccess::give_personal_good(nutritionCap,full,GoodId::SupplementaryFood,1);
    const auto capConsumed=RealmSupplementaryFoodTestAccess::consumed(nutritionCap);
    RealmSupplementaryFoodTestAccess::run_day_people(nutritionCap);
    check(nutritionCap.person(full).nutrition==100&&nutritionCap.person(full).inventory[supplementary]==1&&
          RealmSupplementaryFoodTestAccess::consumed(nutritionCap)==capConsumed&&nutritionCap.invariant(),
          "the Civilian does not waste an extra meal when the base meal already fills nutrition to100");
    std::cout<<"SupplementaryFood source checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"SupplementaryFood source failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}
