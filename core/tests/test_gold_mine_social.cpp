#include "polyec/realm.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmGoldSocialTestAccess {
    static void give(Realm& world,uint64_t actor,int64_t amount=1){
        world.mutable_person(actor).inventory[size_t(GoodId::Gold)]+=amount;
        world.opening_[size_t(GoodId::Gold)]+=amount;
    }
    static int64_t consumed(const Realm& world){return world.consumed_[size_t(GoodId::Gold)];}
    static void set_consumed(Realm& world,int64_t value){world.consumed_[size_t(GoodId::Gold)]=value;}
    static void set_social(Realm& world,uint64_t actor,int value){world.mutable_person(actor).social=value;}
    static void set_cash(Realm& world,uint64_t actor,int64_t value){world.mutable_person(actor).cash=value;}
    static void set_age(Realm& world,uint64_t actor,int value){world.mutable_person(actor).age_days=value;}
    static void set_service(Realm& world,uint64_t actor,bool active){world.mutable_person(actor).military_service.active=active;}
    static void set_busy(Realm& world,uint64_t actor,int value){world.mutable_person(actor).busy_until=value;}
    static void set_source(Realm& world,uint64_t site,int64_t value){
        for(auto& establishment:world.establishments_)if(establishment.id==site){establishment.source_units=value;return;}
        throw std::runtime_error("missing Gold Mine");
    }
};
}

namespace {
int checks=0;
void check(bool ok,const std::string& message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t eligible_civilian(const Realm& world,int region){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.alive&&person.region==region&&person.family==Family::Civilian&&person.age_days>=18*365&&
           world.household_authority(world.household_of(id))==id)return id;
    }
    return 0;
}
}

int main(){try{
    constexpr int region=23;
    constexpr auto gold=GoodId::Gold;
    constexpr const char* saveName="gold-mine-social-use-v1";
    Realm world(20261080,int(kAtlasTerritoryCount)-1,384);
    const uint64_t authority=region+2;
    const uint64_t worker=eligible_civilian(world,region);
    check(worker&&world.person(authority).taxing_authority,"Atlas region23 has a mine authority and an adult civilian household head");
    check(world.model_contract()=="realm-household-fund-goods30-config-v49"&&realm_schema==std::string_view("realm-v38")&&
          realm_actions==400&&realm_obs==5182,"Gold social use advances the contract and keeps tensor sizes unchanged");
    check(world.candidates(worker)[realm_gold_social_candidate].kind==Intent::Hold,
          "Gold social use is absent before the civilian personally owns Gold");

    const auto create=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin)];
    const std::string createDebug=" candidate="+std::to_string(int(create.kind))+" good="+std::to_string(create.good)+
        " target="+std::to_string(create.target)+" region="+std::to_string(world.person(authority).region)+
        " family="+std::to_string(int(world.person(authority).family))+" mask="+
        std::to_string(world.mask(authority)[size_t(realm_specialty_product_candidate_begin)]);
    check(create.kind==Intent::CreateEstablishment&&create.good==int(EstablishmentIndustry::GoldMine)&&
          world.mask(authority)[size_t(realm_specialty_product_candidate_begin)]==1.f,
          "region23 exposes a Gold Mine in the existing specialty establishment slots"+createDebug);
    const auto created=world.execute({authority,world.person(authority).decisions+1,create});
    check(created.success&&world.establishments().size()==1&&world.establishments()[0].industry==EstablishmentIndustry::GoldMine&&
          world.establishments()[0].source_good==int(gold)&&world.establishments()[0].source_units==100,
          "a new Gold Mine starts with 100 finite source units separate from saleable stock");
    const uint64_t site=created.session;
    const auto wrongRegion=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,region,22,EstablishmentIndustry::GoldMine,1,25});
    const auto duplicate=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::GoldMine,1,25});
    check(!wrongRegion.success&&!duplicate.success&&world.establishments().size()==1&&world.invariant(),
          "a mine in another region and a duplicate Gold Mine are rejected without partial change");

    check(world.execute(WorkAuthorizationCommand{authority,world.person(authority).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success&&
          world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
          0,world.game_tick(),world.game_tick()+32,0,1000,ActivityKind::Work,region,site}).success,
          "the mine owner authorizes the worker through the existing schedule command");
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==99&&world.establishments()[0].stock[size_t(gold)]==1&&world.invariant(),
          "completed scheduled Work consumes one source unit and creates one Gold");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,site,
          EstablishmentTransferOperation::WithdrawGoods,int(gold),1}).success&&
          world.view(authority).provinces[size_t(region)].stock[size_t(gold)]==1,
          "the produced Gold can enter normal regional market stock");

    const int64_t price=world.view(worker).provinces[size_t(region)].price[size_t(gold)];
    while(world.person(worker).cash<price+20){
        uint64_t supporter=0;
        for(uint64_t id=1;id<=uint64_t(world.count());++id)
            if(id!=worker&&world.person(id).alive&&world.person(id).family==Family::Civilian&&
               world.person(id).region==region&&world.household_of(id)==world.household_of(worker)&&world.person(id).cash>=5){
                supporter=id;break;
            }
        if(!supporter||!world.execute({supporter,world.person(supporter).decisions+1,{Intent::Support,int(worker),0,5}}).success)break;
    }
    check(world.person(worker).cash>=price+20,"an existing household transfer funds the purchase and later social fee cash="+
          std::to_string(world.person(worker).cash)+" price="+std::to_string(price));
    check(world.execute(MarketOrderCommand{worker,world.person(worker).decisions+1,0,MarketOrderOperation::Place,
          region,int(gold),1,price}).success,"the civilian buys Gold through the regular market order");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.person(worker).inventory[size_t(gold)]==1&&world.invariant(),
          "market settlement places the purchased Gold in the buyer's personal inventory");
    const auto action=world.candidates(worker)[realm_gold_social_candidate];
    check(action.kind==Intent::Social&&action.good==int(gold)&&action.amount==20&&action.term==14&&
          Realm::action_name(action)=="Giao lưu với trang sức vàng"&&world.mask(worker)[realm_gold_social_candidate]==1.f,
          "eligible adult civilian sees the named Gold action, its cost and fourteen-day duration");
    const int64_t cashBefore=world.person(worker).cash;
    const int socialBefore=world.person(worker).social;
    const int64_t fundBefore=world.view(worker).provinces[size_t(region)].firm;
    const int64_t consumedBefore=RealmGoldSocialTestAccess::consumed(world);
    check(world.execute({worker,world.person(worker).decisions+1,action}).success&&
          world.person(worker).cash==cashBefore-20&&world.view(worker).provinces[size_t(region)].firm==fundBefore+20&&
          world.person(worker).inventory[size_t(gold)]==0&&RealmGoldSocialTestAccess::consumed(world)==consumedBefore+1&&
          world.person(worker).social==std::min(100,socialBefore+20)&&world.person(worker).busy_until==world.day()+14&&world.invariant(),
          "the action consumes personal Gold, pays the regional fund, raises social by20 and occupies14 days");

    const auto saved=world.serialize(saveName);
    check(saved.rfind("POLYEC_REALM_71 ",0)==0,"Gold Mine is written with save schema70");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,saveName,error)&&restored.fingerprint()==world.fingerprint()&&
          restored.establishments()[0].industry==EstablishmentIndustry::GoldMine&&
          RealmGoldSocialTestAccess::consumed(restored)==consumedBefore+1&&restored.invariant(),
          "schema70 roundtrip preserves Gold Mine, inventories, social use and money: "+error);
    auto badSave=saved;
    const size_t marker=badSave.find(" ESTABLISH_14 1 ");
    const size_t regionStart=badSave.find(' ',marker+std::string(" ESTABLISH_14 1 ").size())+1;
    const size_t regionEnd=badSave.find(' ',regionStart);
    badSave.replace(regionStart,regionEnd-regionStart,"22");
    const auto restoredBefore=restored.fingerprint();
    check(!restored.deserialize(badSave,saveName,error)&&restored.fingerprint()==restoredBefore,
          "schema70 rejects a Gold Mine moved away from its region without replacing the loaded world");

    Realm finite(20261081,int(kAtlasTerritoryCount)-1,256);
    const auto finiteMine=finite.execute({authority,finite.person(authority).decisions+1,
        finite.candidates(authority)[size_t(realm_specialty_product_candidate_begin)]});
    check(finiteMine.success,"a separate world creates a Gold Mine for depletion checks");
    RealmGoldSocialTestAccess::set_source(finite,finiteMine.session,0);
    const auto finiteWorker=eligible_civilian(finite,region);
    check(finite.execute(WorkAuthorizationCommand{authority,finite.person(authority).decisions+1,finiteMine.session,
          finiteWorker,WorkAuthorizationOperation::Set,finite.game_tick(),finite.game_tick()+32}).success&&
          finite.execute(ScheduleCommand{finiteWorker,finite.person(finiteWorker).decisions+1,ScheduleCommand::Operation::Submit,
          0,finite.game_tick(),finite.game_tick()+32,0,1000,ActivityKind::Work,region,finiteMine.session}).success,
          "an authorized worker may keep the normal schedule after a Gold vein is exhausted");
    finite.advance_ticks(64);
    check(finite.establishments()[0].source_units==0&&finite.establishments()[0].stock[size_t(gold)]==0,
          "completed Work cannot create Gold after the source is depleted");
    finite.advance(365);
    check(finite.establishments()[0].source_units==0,"the finite Gold vein is not replenished by passing game days");

    Realm boundary(20261082,int(kAtlasTerritoryCount)-1,256);
    const uint64_t boundaryActor=eligible_civilian(boundary,region);
    RealmGoldSocialTestAccess::give(boundary,boundaryActor);
    RealmGoldSocialTestAccess::set_social(boundary,boundaryActor,95);
    const auto boundaryAction=boundary.candidates(boundaryActor)[realm_gold_social_candidate];
    check(boundary.execute({boundaryActor,boundary.person(boundaryActor).decisions+1,boundaryAction}).success&&
          boundary.person(boundaryActor).social==100,"the Gold benefit cannot exceed social100");
    RealmGoldSocialTestAccess::set_age(boundary,boundaryActor,17*365);
    RealmGoldSocialTestAccess::give(boundary,boundaryActor);
    const auto youngBefore=boundary.fingerprint();
    check(boundary.candidates(boundaryActor)[realm_gold_social_candidate].kind==Intent::Hold&&
          !boundary.execute({boundaryActor,boundary.person(boundaryActor).decisions+1,
              {Intent::Social,region,int(gold),20,14}}).success&&boundary.fingerprint()==youngBefore,
          "a Civilian younger than18 cannot use the Gold action and rejection is atomic");

    Realm military(20261083,int(kAtlasTerritoryCount)-1,256);
    const uint64_t militaryActor=eligible_civilian(military,region);
    RealmGoldSocialTestAccess::give(military,militaryActor);
    RealmGoldSocialTestAccess::set_service(military,militaryActor,true);
    const auto militaryBefore=military.fingerprint();
    check(military.candidates(militaryActor)[realm_gold_social_candidate].kind==Intent::Hold&&
          !military.execute({militaryActor,military.person(militaryActor).decisions+1,
              {Intent::Social,region,int(gold),20,14}}).success&&military.fingerprint()==militaryBefore,
          "a Civilian in active military service cannot use Gold socially");
    Realm busy(20261084,int(kAtlasTerritoryCount)-1,256);
    const uint64_t busyActor=eligible_civilian(busy,region);
    RealmGoldSocialTestAccess::give(busy,busyActor);
    RealmGoldSocialTestAccess::set_busy(busy,busyActor,busy.day()+1);
    const auto busyBefore=busy.fingerprint();
    check(busy.candidates(busyActor)[realm_gold_social_candidate].kind==Intent::Hold&&
          !busy.execute({busyActor,busy.person(busyActor).decisions+1,
              {Intent::Social,region,int(gold),20,14}}).success&&busy.fingerprint()==busyBefore,
          "a busy Civilian cannot start another Gold social use");
    Realm poor(20261085,int(kAtlasTerritoryCount)-1,256);
    const uint64_t poorActor=eligible_civilian(poor,region);
    RealmGoldSocialTestAccess::give(poor,poorActor);
    RealmGoldSocialTestAccess::set_cash(poor,poorActor,19);
    const auto poorBefore=poor.fingerprint();
    check(poor.candidates(poorActor)[realm_gold_social_candidate].kind==Intent::Hold&&
          !poor.execute({poorActor,poor.person(poorActor).decisions+1,
              {Intent::Social,region,int(gold),20,14}}).success&&poor.fingerprint()==poorBefore,
          "19 personal coins are insufficient for the 20-coin Gold action");
    Realm overflow(20261086,int(kAtlasTerritoryCount)-1,256);
    const uint64_t overflowActor=eligible_civilian(overflow,region);
    RealmGoldSocialTestAccess::give(overflow,overflowActor);
    RealmGoldSocialTestAccess::set_consumed(overflow,std::numeric_limits<int64_t>::max());
    const auto overflowBefore=overflow.fingerprint();
    check(!overflow.execute({overflowActor,overflow.person(overflowActor).decisions+1,
              overflow.candidates(overflowActor)[realm_gold_social_candidate]}).success&&overflow.fingerprint()==overflowBefore,
          "a full consumed-Gold ledger rejects without changing the world");
    std::cout<<"Gold mine and social-use checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"Gold mine social-use failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}
