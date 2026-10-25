#include "polyec/realm.hpp"

#include <iostream>
#include <stdexcept>

using namespace polyec;

namespace polyec {
struct RealmPotteryMealVesselTestAccess {
    static void set_inventory(Realm& world,uint64_t actor,GoodId good,int64_t amount){
        world.mutable_person(actor).inventory[size_t(good)]+=amount;
        world.opening_[size_t(good)]+=amount;
    }
    static void set_nutrition(Realm& world,uint64_t actor,int amount){world.mutable_person(actor).nutrition=amount;}
    static void make_active_service(Realm& world,uint64_t actor){
        auto& p=world.mutable_person(actor);p.military_service.active=true;
        p.military_service.start_day=world.day_+100;p.military_service.end_day=world.day_+200;
    }
    static void clear_regional_food(Realm& world,uint64_t actor){
        auto& stock=world.provinces_.at(size_t(world.person(actor).region)).stock[size_t(GoodId::DryFood)];
        world.consumed_[size_t(GoodId::DryFood)]+=stock;stock=0;
    }
    static void give_good(Realm& world,uint64_t actor,GoodId good,int64_t amount){
        world.mutable_person(actor).inventory[size_t(good)]+=amount;
        world.opening_[size_t(good)]+=amount;
    }
    static int64_t consumed(const Realm& world,GoodId good){return world.consumed_[size_t(good)];}
    static void advance_to_meal(Realm& world,uint64_t actor){
        int days=(int(actor%7)-world.day()%7+7)%7;if(days==0)days=7;
        world.advance(days);
    }
};
}

namespace {
int checks=0;
void check(bool condition,const char* message){++checks;if(!condition)throw std::runtime_error(message);}
uint64_t adult_civilian_in_region(const Realm& world,int region){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.alive&&person.family==Family::Civilian&&person.region==region&&
           person.age_days>=18*365&&person.destination==-1)return id;
    }
    return 0;
}
}

int main(){try{
    Realm with_vessel(20261062,6,256),without_vessel(20261062,6,256);
    uint64_t actor=0;
    for(uint64_t id=1;id<=uint64_t(with_vessel.count());++id)
        if(with_vessel.person(id).family==Family::Civilian&&with_vessel.person(id).age_days>=18*365){actor=id;break;}
    check(actor>0,
          "paired actor is an adult Civilian");
    check(with_vessel.model_contract()=="realm-household-fund-goods30-config-v54"&&
          realm_actions==400&&realm_obs==5182,
          "the meal transition change advances the model contract without changing observation or action shapes");
    RealmPotteryMealVesselTestAccess::set_inventory(with_vessel,actor,GoodId::DryFood,1);
    RealmPotteryMealVesselTestAccess::set_inventory(with_vessel,actor,GoodId::Pottery,1);
    RealmPotteryMealVesselTestAccess::set_inventory(without_vessel,actor,GoodId::DryFood,1);
    RealmPotteryMealVesselTestAccess::set_nutrition(with_vessel,actor,50);
    RealmPotteryMealVesselTestAccess::set_nutrition(without_vessel,actor,50);
    RealmPotteryMealVesselTestAccess::advance_to_meal(with_vessel,actor);
    RealmPotteryMealVesselTestAccess::advance_to_meal(without_vessel,actor);
    const int health_before=with_vessel.person(actor).health;
    const auto consumed_before=RealmPotteryMealVesselTestAccess::consumed(with_vessel,GoodId::Pottery);
    check(with_vessel.person(actor).nutrition==75&&without_vessel.person(actor).nutrition==70,
          "a personally owned Pottery vessel adds five nutrition after the successful weekly DryFood meal");
    check(with_vessel.person(actor).health==health_before&&with_vessel.person(actor).inventory[size_t(GoodId::Pottery)]==1&&
          RealmPotteryMealVesselTestAccess::consumed(with_vessel,GoodId::Pottery)==consumed_before,
          "the durable vessel remains in personal inventory and does not change health or consumed-goods accounting");

    Realm no_food(20261062,6,256);
    RealmPotteryMealVesselTestAccess::set_inventory(no_food,actor,GoodId::Pottery,1);
    RealmPotteryMealVesselTestAccess::set_nutrition(no_food,actor,50);
    RealmPotteryMealVesselTestAccess::clear_regional_food(no_food,actor);
    RealmPotteryMealVesselTestAccess::advance_to_meal(no_food,actor);
    check(no_food.person(actor).nutrition<50,
          "own Pottery does not replace a missing DryFood meal or prevent unmet-need loss");

    Realm other_owner(20261062,6,256);
    RealmPotteryMealVesselTestAccess::set_inventory(other_owner,actor,GoodId::DryFood,1);
    RealmPotteryMealVesselTestAccess::set_inventory(other_owner,actor+1,GoodId::Pottery,1);
    RealmPotteryMealVesselTestAccess::set_nutrition(other_owner,actor,50);
    RealmPotteryMealVesselTestAccess::advance_to_meal(other_owner,actor);
    check(other_owner.person(actor).nutrition==70,
          "Pottery held by another person cannot improve this actor's meal");

    Realm military(20261062,6,256);
    RealmPotteryMealVesselTestAccess::set_inventory(military,actor,GoodId::DryFood,1);
    RealmPotteryMealVesselTestAccess::set_inventory(military,actor,GoodId::Pottery,1);
    RealmPotteryMealVesselTestAccess::set_nutrition(military,actor,50);
    RealmPotteryMealVesselTestAccess::make_active_service(military,actor);
    RealmPotteryMealVesselTestAccess::advance_to_meal(military,actor);
    check(military.person(actor).nutrition==70,
          "a Civilian in active military service does not receive the civilian Pottery meal benefit");

    Realm cap(20261062,6,256);
    RealmPotteryMealVesselTestAccess::set_inventory(cap,actor,GoodId::DryFood,1);
    RealmPotteryMealVesselTestAccess::set_inventory(cap,actor,GoodId::Pottery,1);
    RealmPotteryMealVesselTestAccess::set_nutrition(cap,actor,80);
    RealmPotteryMealVesselTestAccess::advance_to_meal(cap,actor);
    check(cap.person(actor).nutrition==100,"base meal plus vessel benefit respects the nutrition cap of one hundred");

    check(with_vessel.invariant(),"meal use preserves Realm ownership and supply accounting");
    const auto saved=with_vessel.serialize("pottery-meal-vessel-v1");
    Realm restored(1,6,256);std::string error;
    check(restored.deserialize(saved,"pottery-meal-vessel-v1",error),("meal and personal vessel save/load: "+error).c_str());
    check(restored.serialize("pottery-meal-vessel-v1")==saved&&
          restored.person(actor).inventory[size_t(GoodId::Pottery)]==1&&
          restored.person(actor).nutrition==with_vessel.person(actor).nutrition,
          "save/load preserves nutrition and durable personal Pottery inventory");

    constexpr int pottery_region=15;
    Realm integrated(20261063,26,256);
    const uint64_t authority=pottery_region+2;
    const uint64_t kiln_worker=adult_civilian_in_region(integrated,pottery_region);
    check(kiln_worker!=0&&integrated.person(authority).taxing_authority,
          "the Pottery region has its normal authority and an adult worker");
    const Candidate create_kiln=integrated.candidates(authority)[267];
    check(create_kiln.kind==Intent::CreateEstablishment&&
          create_kiln.good==int(EstablishmentIndustry::PotteryKiln),
          "the existing Atlas candidate creates the Pottery Kiln");
    const auto kiln_created=integrated.execute({authority,integrated.person(authority).decisions+1,create_kiln});
    check(kiln_created.success&&integrated.establishments().back().industry==EstablishmentIndustry::PotteryKiln,
          "the authority builds the Pottery Kiln using the normal establishment command");
    const uint64_t kiln=kiln_created.session;
    check(integrated.execute(EstablishmentTransferCommand{authority,integrated.person(authority).decisions+1,
          kiln,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1}).success,
          "the kiln receives Fuel from the regional market");
    check(integrated.execute(WorkAuthorizationCommand{authority,integrated.person(authority).decisions+1,kiln,kiln_worker,
          WorkAuthorizationOperation::Set,integrated.game_tick(),integrated.game_tick()+32}).success&&
          integrated.execute(ScheduleCommand{kiln_worker,integrated.person(kiln_worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,integrated.game_tick(),integrated.game_tick()+32,0,1000,
          ActivityKind::Work,pottery_region,kiln}).success,
          "the adult schedules authorized Work through the shared scheduling command");
    integrated.advance_ticks(64);
    check(integrated.establishments().back().stock[size_t(GoodId::Pottery)]==1&&
          integrated.establishments().back().stock[size_t(GoodId::Fuel)]==0&&integrated.invariant(),
          "a real Work batch consumes Fuel and finite clay source to make one Pottery");
    check(integrated.execute(EstablishmentTransferCommand{authority,integrated.person(authority).decisions+1,
          kiln,EstablishmentTransferOperation::WithdrawGoods,int(GoodId::Pottery),1}).success,
          "the kiln owner transfers the finished Pottery to the regional market");
    const uint64_t buyer=adult_civilian_in_region(integrated,pottery_region);
    check(buyer!=0,"an adult Civilian can buy Pottery where it is made");
    const int64_t pottery_price=integrated.view(buyer).provinces[size_t(pottery_region)].price[size_t(GoodId::Pottery)];
    if(integrated.person(buyer).cash<pottery_price)
        check(integrated.execute({buyer,integrated.person(buyer).decisions+1,
              {Intent::Borrow,pottery_region,0,100,180}}).success,
              "the buyer funds the purchase through the existing loan command");
    check(integrated.execute(MarketOrderCommand{buyer,integrated.person(buyer).decisions+1,0,
          MarketOrderOperation::Place,pottery_region,int(GoodId::Pottery),1,pottery_price}).success,
          "the Civilian places a normal market order for one Pottery");
    integrated.advance_ticks(kSubdayUnitsPerDay);
    check(integrated.person(buyer).inventory[size_t(GoodId::Pottery)]==1&&integrated.invariant(),
          "market settlement puts the purchased Pottery in its buyer's personal inventory");
    RealmPotteryMealVesselTestAccess::give_good(integrated,buyer,GoodId::DryFood,1);
    RealmPotteryMealVesselTestAccess::set_nutrition(integrated,buyer,50);
    RealmPotteryMealVesselTestAccess::advance_to_meal(integrated,buyer);
    check(integrated.person(buyer).nutrition==75&&integrated.person(buyer).inventory[size_t(GoodId::Pottery)]==1,
          "a Civilian receives the meal-vessel bonus after buying and eating with Pottery");
    check(integrated.invariant(),"production, trade and meal use preserve Realm accounting");
    std::cout<<"Pottery meal vessel checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"Pottery meal vessel failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}
