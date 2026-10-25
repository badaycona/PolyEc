#include "polyec/realm.hpp"
#include <iostream>
#include <stdexcept>
using namespace polyec;
namespace polyec {
struct RealmSaltedMealTestAccess {
    static void give(Realm& w,uint64_t id,GoodId good,int64_t n){w.mutable_person(id).inventory[size_t(good)]+=n;w.opening_[size_t(good)]+=n;}
    static void nutrition(Realm& w,uint64_t id,int n){w.mutable_person(id).nutrition=n;}
    static int64_t consumed(const Realm&w,GoodId g){return w.consumed_[size_t(g)];}
    static void clear_regional_food(Realm&w,uint64_t id){auto& stock=w.provinces_.at(size_t(w.person(id).region)).stock[size_t(GoodId::DryFood)];w.consumed_[size_t(GoodId::DryFood)]+=stock;stock=0;}
    static void active_service(Realm&w,uint64_t id){auto&p=w.mutable_person(id);p.military_service.active=true;p.military_service.start_day=w.day_+100;p.military_service.end_day=w.day_+200;}
    static void meal(Realm&w,uint64_t id){int days=(int(id%7)-w.day()%7+7)%7;if(days==0)days=7;w.advance(days);}
};
}
namespace {int checks=0;void check(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}}
int main(){try{
    Realm salted(20261025,6,256),plain(20261025,6,256);uint64_t actor=0;
    for(uint64_t id=1;id<=uint64_t(salted.count());++id){const auto&p=salted.person(id);if(p.family==Family::Civilian&&p.age_days>=18*365){actor=id;break;}}
    check(actor&&salted.person(actor).family==Family::Civilian,"fixture contains adult Civilian");
    check(salted.model_contract()=="realm-household-fund-goods30-config-v54-atlas-north-pass-v1-salted-meal-v1"&&realm_actions==400&&realm_obs==5182,"Salt consumer has a distinct contract without changing tensor shapes");
    for(auto*w:{&salted,&plain}){RealmSaltedMealTestAccess::give(*w,actor,GoodId::DryFood,1);RealmSaltedMealTestAccess::nutrition(*w,actor,50);}
    RealmSaltedMealTestAccess::give(salted,actor,GoodId::Salt,1);
    const auto before=RealmSaltedMealTestAccess::consumed(salted,GoodId::Salt);const int health=salted.person(actor).health;
    RealmSaltedMealTestAccess::meal(salted,actor);RealmSaltedMealTestAccess::meal(plain,actor);
    check(salted.person(actor).nutrition==75&&plain.person(actor).nutrition==70,"successful DryFood meal consumes personal Salt for five extra nutrition");
    check(salted.person(actor).inventory[size_t(GoodId::Salt)]==0&&RealmSaltedMealTestAccess::consumed(salted,GoodId::Salt)==before+1,"one Salt is removed and recorded exactly once");
    check(salted.person(actor).health==health,"Salt does not change health");
    Realm missing(20261025,6,256);RealmSaltedMealTestAccess::give(missing,actor,GoodId::DryFood,1);RealmSaltedMealTestAccess::nutrition(missing,actor,50);RealmSaltedMealTestAccess::meal(missing,actor);
    check(missing.person(actor).nutrition==70&&RealmSaltedMealTestAccess::consumed(missing,GoodId::Salt)==0,"meal without Salt keeps normal nutrition and consumes none");
    Realm cap(20261025,6,256);RealmSaltedMealTestAccess::give(cap,actor,GoodId::DryFood,1);RealmSaltedMealTestAccess::give(cap,actor,GoodId::Salt,1);RealmSaltedMealTestAccess::nutrition(cap,actor,75);RealmSaltedMealTestAccess::meal(cap,actor);
    check(cap.person(actor).nutrition==100&&cap.person(actor).inventory[size_t(GoodId::Salt)]==0,"Salt bonus respects nutrition cap");
    Realm already_full(20261025,6,256);RealmSaltedMealTestAccess::give(already_full,actor,GoodId::DryFood,1);RealmSaltedMealTestAccess::give(already_full,actor,GoodId::Salt,1);RealmSaltedMealTestAccess::nutrition(already_full,actor,100);RealmSaltedMealTestAccess::meal(already_full,actor);
    check(already_full.person(actor).nutrition==100&&already_full.person(actor).inventory[size_t(GoodId::Salt)]==1&&RealmSaltedMealTestAccess::consumed(already_full,GoodId::Salt)==0,"nutrition already at one hundred does not waste Salt");    Realm nofood(20261025,6,256);RealmSaltedMealTestAccess::give(nofood,actor,GoodId::Salt,1);RealmSaltedMealTestAccess::nutrition(nofood,actor,50);RealmSaltedMealTestAccess::clear_regional_food(nofood,actor);RealmSaltedMealTestAccess::meal(nofood,actor);
    check(nofood.person(actor).inventory[size_t(GoodId::Salt)]==1&&RealmSaltedMealTestAccess::consumed(nofood,GoodId::Salt)==0,"unmet DryFood need does not consume Salt");
    Realm service(20261025,6,256);RealmSaltedMealTestAccess::give(service,actor,GoodId::DryFood,1);RealmSaltedMealTestAccess::give(service,actor,GoodId::Salt,1);RealmSaltedMealTestAccess::nutrition(service,actor,50);RealmSaltedMealTestAccess::active_service(service,actor);RealmSaltedMealTestAccess::meal(service,actor);
    check(service.person(actor).nutrition==70&&service.person(actor).inventory[size_t(GoodId::Salt)]==1,"active military service does not use civilian Salt meal benefit");
    check(salted.invariant(),"Salt meal preserves Realm invariant");
    const auto save=salted.serialize("salted-meal-v1");Realm loaded(1,6,256);std::string error;
    check(loaded.deserialize(save,"salted-meal-v1",error),"save/load accepts meal with consumed Salt");
    check(loaded.serialize("salted-meal-v1")==save&&loaded.person(actor).inventory[size_t(GoodId::Salt)]==0&&RealmSaltedMealTestAccess::consumed(loaded,GoodId::Salt)==before+1,"save/load does not repeat the Salt meal transaction");
    Realm integrated(20261026,26,256);constexpr int region=8;const uint64_t authority=region+2;uint64_t citizen=0;
    for(uint64_t id=1;id<=uint64_t(integrated.count());++id){const auto&p=integrated.person(id);if(p.region==region&&p.alive&&p.family==Family::Civilian&&p.age_days>=18*365){citizen=id;break;}}
    check(citizen&&integrated.person(authority).taxing_authority,"Atlas region8 has a local authority and an adult Civilian");
    RealmSaltedMealTestAccess::give(integrated,citizen,GoodId::DryFood,1);RealmSaltedMealTestAccess::nutrition(integrated,citizen,50);
    const auto create=integrated.execute(EstablishmentCommand{authority,integrated.person(authority).decisions+1,EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::Saltworks,1,25});
    check(create.success,"local authority creates a Saltworks through the normal Realm command");const uint64_t site=create.session;
    check(integrated.execute(EstablishmentTransferCommand{authority,integrated.person(authority).decisions+1,site,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1}).success,"Saltworks receives one market Fuel");
    check(integrated.execute(WorkAuthorizationCommand{authority,integrated.person(authority).decisions+1,site,citizen,WorkAuthorizationOperation::Set,integrated.game_tick(),integrated.game_tick()+32}).success,"Civilian is authorized to work at the Saltworks");
    check(integrated.execute(ScheduleCommand{citizen,integrated.person(citizen).decisions+1,ScheduleCommand::Operation::Submit,0,integrated.game_tick(),integrated.game_tick()+32,0,1000,ActivityKind::Work,region,site}).success,"Civilian submits Work through the normal schedule command");
    integrated.advance_ticks(64);check(integrated.establishments()[0].stock[size_t(GoodId::Salt)]==1,"completed production batch creates one Salt");
    check(integrated.execute(EstablishmentTransferCommand{authority,integrated.person(authority).decisions+1,site,EstablishmentTransferOperation::WithdrawGoods,int(GoodId::Salt),1}).success,"public owner transfers produced Salt to regional market");
    check(integrated.execute({citizen,integrated.person(citizen).decisions+1,{Intent::Buy,region,int(GoodId::Salt),1}}).success,"Civilian buys the produced Salt through the market");
    const auto salt_consumed_before=RealmSaltedMealTestAccess::consumed(integrated,GoodId::Salt);RealmSaltedMealTestAccess::meal(integrated,citizen);
    check(integrated.person(citizen).nutrition==75&&integrated.person(citizen).inventory[size_t(GoodId::Salt)]==0&&RealmSaltedMealTestAccess::consumed(integrated,GoodId::Salt)==salt_consumed_before+1,"same Realm completes Saltworks→market purchase→DryFood meal→Salt consumption");
    check(integrated.invariant(),"integrated producer-to-consumer chain preserves Realm invariant");
    std::cout<<"SALTED MEAL: "<<checks<<" checks passed\n";return 0;
}catch(const std::exception&e){std::cerr<<"SALTED MEAL FAILURE: "<<e.what()<<"\n";return 1;}}
