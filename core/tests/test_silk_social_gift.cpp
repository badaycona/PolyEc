#include "polyec/realm.hpp"
#include "current_save_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmSilkSocialTestAccess {
    static void give(Realm& world,uint64_t actor,int64_t amount=1){
        world.mutable_person(actor).inventory[size_t(GoodId::Silk)]+=amount;
        world.opening_[size_t(GoodId::Silk)]+=amount;
    }
    static int64_t consumed(const Realm& world){return world.consumed_[size_t(GoodId::Silk)];}
    static void set_consumed(Realm& world,int64_t value){world.consumed_[size_t(GoodId::Silk)]=value;}
    static void set_social(Realm& world,uint64_t actor,int value){world.mutable_person(actor).social=value;}
    static void set_cash(Realm& world,uint64_t actor,int64_t value){world.mutable_person(actor).cash=value;}
    static void set_service(Realm& world,uint64_t actor,bool active){world.mutable_person(actor).military_service.active=active;}
    static void set_source(Realm& world,uint64_t site,int64_t value){
        for(auto& establishment:world.establishments_)if(establishment.id==site){establishment.source_units=value;return;}
        throw std::runtime_error("missing Silkworm Farm");
    }
};
}

namespace {
int checks=0;
void check(bool ok,const std::string& message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t adult_civilian(const Realm& world,int region){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.alive&&person.region==region&&person.family==Family::Civilian&&person.age_days>=14*365&&
           world.household_authority(world.household_of(id))==id)return id;
    }
    return 0;
}
}

int main(){try{
    constexpr int region=12;
    constexpr auto silk=GoodId::Silk;
    constexpr const char* saveName="silk-source-social-gift-v1";
    Realm world(20261070,int(kAtlasTerritoryCount)-1,384);
    const uint64_t authority=region+2;
    const uint64_t worker=adult_civilian(world,region);
    check(worker&&world.person(authority).taxing_authority,"Atlas region12 has a governing authority and adult civilian household head");
    check(world.model_contract()=="realm-household-fund-goods30-config-v54-atlas-north-pass-v1"&&realm_actions==400&&realm_obs==5182,
          "Silk action advances the model contract and keeps the existing tensor sizes");
    check(world.candidates(worker)[realm_silk_social_candidate].kind==Intent::Hold,
          "Silk social action is absent when the Civilian owns no Silk");

    const auto create=world.candidates(authority)[size_t(realm_specialty_product_candidate_begin)];
    check(create.kind==Intent::CreateEstablishment&&create.good==int(EstablishmentIndustry::SilkwormFarm)&&
          world.mask(authority)[size_t(realm_specialty_product_candidate_begin)]==1.f,
          "region12 exposes a Silkworm Farm through the existing specialty-establishment slot");
    const auto created=world.execute({authority,world.person(authority).decisions+1,create});
    check(created.success&&world.establishments().size()==1&&world.establishments()[0].industry==EstablishmentIndustry::SilkwormFarm&&
          world.establishments()[0].source_good==int(silk)&&world.establishments()[0].source_units==25,
          "new Farm starts with 25 of 100 private source units, separate from saleable Silk stock");
    const uint64_t site=created.session;
    const auto wrongRegion=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,region,11,EstablishmentIndustry::SilkwormFarm,1,25});
    const auto duplicate=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::SilkwormFarm,1,25});
    check(!wrongRegion.success&&!duplicate.success&&world.establishments().size()==1&&world.invariant(),
          "wrong-region and duplicate Farm creation are rejected without changing the world");
    check(world.execute(WorkAuthorizationCommand{authority,world.person(authority).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success&&
          world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
          0,world.game_tick(),world.game_tick()+32,0,1000,ActivityKind::Work,region,site}).success,
          "the Farm owner authorizes a normal Work schedule at the source");
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==24&&world.establishments()[0].stock[size_t(silk)]==1&&world.invariant(),
          "completed scheduled Work turns one source unit into one actual Silk");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,site,
          EstablishmentTransferOperation::WithdrawGoods,int(silk),1}).success&&
          world.view(authority).provinces[size_t(region)].stock[size_t(silk)]==1,
          "the owner transfers produced Silk into the ordinary regional market stock");

    const int64_t price=world.view(worker).provinces[size_t(region)].price[size_t(silk)];
    uint64_t supporter=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        if(id!=worker&&world.person(id).alive&&world.person(id).family==Family::Civilian&&
           world.person(id).region==region&&world.household_of(id)==world.household_of(worker)&&world.person(id).cash>=5){
            supporter=id;break;
        }
    }
    while(world.person(worker).cash<price+4&&supporter){
        const auto transfer=world.execute({supporter,world.person(supporter).decisions+1,{Intent::Support,int(worker),0,5}});
        if(!transfer.success)break;
    }
    check(world.person(worker).cash>=price+4,"an existing household transfer funds both the market order and later Silk gift fee");
    check(world.execute(MarketOrderCommand{worker,world.person(worker).decisions+1,0,MarketOrderOperation::Place,
          region,int(silk),1,price}).success,"the Civilian places a standard Silk market order");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.person(worker).inventory[size_t(silk)]==1&&world.invariant(),
          "market settlement delivers one Silk into the buyer's personal inventory");
    const auto action=world.candidates(worker)[realm_silk_social_candidate];
    check(action.kind==Intent::Social&&action.good==int(silk)&&action.amount==4&&action.term==7&&
          Realm::action_name(action)=="Giao lưu bằng lụa"&&world.mask(worker)[realm_silk_social_candidate]==1.f,
          "eligible Civilian sees a named Silk gift with its four-coin cost and seven-day duration");
    check(world.candidates(worker)[realm_silver_social_candidate].kind==Intent::Hold&&
          world.candidates(worker)[27].kind==Intent::Social,
          "Silk uses the distinct social slot and does not change the Silver action");
    const int64_t cashBefore=world.person(worker).cash;
    const int socialBefore=world.person(worker).social;
    const int64_t fundBefore=world.view(worker).provinces[size_t(region)].firm;
    const int64_t consumedBefore=RealmSilkSocialTestAccess::consumed(world);
    check(world.execute({worker,world.person(worker).decisions+1,action}).success&&
          world.person(worker).cash==cashBefore-4&&world.view(worker).provinces[size_t(region)].firm==fundBefore+4&&
          world.person(worker).inventory[size_t(silk)]==0&&RealmSilkSocialTestAccess::consumed(world)==consumedBefore+1&&
          world.person(worker).social==std::min(100,socialBefore+12)&&world.person(worker).busy_until==world.day()+7&&world.invariant(),
          "Silk gift consumes its owner's item, pays the regional fund, raises social by 12 and occupies seven days");
    const auto saved=world.serialize(saveName);
    check(saved.rfind("POLYEC_REALM_75 ",0)==0,"Silkworm Farm is written in save schema70");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,saveName,error)&&restored.fingerprint()==world.fingerprint()&&
          restored.establishments()[0].industry==EstablishmentIndustry::SilkwormFarm&&
          RealmSilkSocialTestAccess::consumed(restored)==consumedBefore+1&&restored.invariant(),
          "schema70 roundtrip preserves the Farm, consumed Silk, money, social and busy time: "+error);
    auto badSave=saved;
    const size_t marker=badSave.find(" ESTABLISH_14 1 ");
    const size_t regionStart=badSave.find(' ',marker+std::string(" ESTABLISH_14 1 ").size())+1;
    const size_t regionEnd=badSave.find(' ',regionStart);
    badSave.replace(regionStart,regionEnd-regionStart,"11");
    const auto restoredBefore=restored.fingerprint();
    check(!restored.deserialize(badSave,saveName,error)&&restored.fingerprint()==restoredBefore,
          "schema70 refuses a Farm moved away from region12 without replacing the loaded world");

    Realm seasons(20261076,int(kAtlasTerritoryCount)-1,256);
    const uint64_t seasonAuthority=region+2;
    const auto seasonCandidate=seasons.candidates(seasonAuthority)[size_t(realm_specialty_product_candidate_begin)];
    const auto seasonFarm=seasons.execute({seasonAuthority,seasons.person(seasonAuthority).decisions+1,seasonCandidate});
    check(seasonFarm.success,"a separate Atlas world creates a Farm for source regeneration checks");
    RealmSilkSocialTestAccess::set_source(seasons,seasonFarm.session,24);
    seasons.advance(89);
    check(seasons.establishments()[0].source_units==24,"Silk source does not regenerate before day90");
    seasons.advance(1);
    check(seasons.establishments()[0].source_units==49,"day90 adds25 source units to the Farm");
    RealmSilkSocialTestAccess::set_source(seasons,seasonFarm.session,100);
    seasons.advance(90);
    check(seasons.establishments()[0].source_units==100,"seasonal source growth stops at100");

    Realm shortage(20261077,int(kAtlasTerritoryCount)-1,256);
    const uint64_t shortageAuthority=region+2;
    const uint64_t shortageWorker=adult_civilian(shortage,region);
    const auto shortageCandidate=shortage.candidates(shortageAuthority)[size_t(realm_specialty_product_candidate_begin)];
    const auto shortageFarm=shortage.execute({shortageAuthority,shortage.person(shortageAuthority).decisions+1,shortageCandidate});
    check(shortageFarm.success,"another Atlas world creates a Farm for finite-source shortage checks");
    RealmSilkSocialTestAccess::set_source(shortage,shortageFarm.session,0);
    check(shortage.execute(WorkAuthorizationCommand{shortageAuthority,shortage.person(shortageAuthority).decisions+1,
          shortageFarm.session,shortageWorker,WorkAuthorizationOperation::Set,shortage.game_tick(),shortage.game_tick()+32}).success&&
          shortage.execute(ScheduleCommand{shortageWorker,shortage.person(shortageWorker).decisions+1,
          ScheduleCommand::Operation::Submit,0,shortage.game_tick(),shortage.game_tick()+32,0,1000,
          ActivityKind::Work,region,shortageFarm.session}).success,
          "authorized Work remains a normal schedule even when the Farm source is empty");
    shortage.advance_ticks(64);
    check(shortage.establishments()[0].source_units==0&&shortage.establishments()[0].stock[size_t(silk)]==0,
          "completed Work cannot create Silk after the Farm source is depleted");

    Realm capped(20261071,int(kAtlasTerritoryCount)-1,256);
    const uint64_t capActor=adult_civilian(capped,region);
    RealmSilkSocialTestAccess::give(capped,capActor);
    RealmSilkSocialTestAccess::set_social(capped,capActor,95);
    check(capped.execute({capActor,capped.person(capActor).decisions+1,capped.candidates(capActor)[realm_silk_social_candidate]}).success&&
          capped.person(capActor).social==100,"the twelve-point Silk benefit cannot exceed social100");

    Realm active(20261072,int(kAtlasTerritoryCount)-1,256);
    const uint64_t activeActor=adult_civilian(active,region);
    RealmSilkSocialTestAccess::give(active,activeActor);
    RealmSilkSocialTestAccess::set_service(active,activeActor,true);
    const auto activeBefore=active.fingerprint();
    check(active.candidates(activeActor)[realm_silk_social_candidate].kind==Intent::Hold&&
          !active.execute({activeActor,active.person(activeActor).decisions+1,
              {Intent::Social,region,int(silk),4,7}}).success&&active.fingerprint()==activeBefore,
          "active military service hides the Silk gift and forges cannot spend the Civilian's item");

    Realm poor(20261073,int(kAtlasTerritoryCount)-1,256);
    const uint64_t poorActor=adult_civilian(poor,region);
    RealmSilkSocialTestAccess::give(poor,poorActor);
    RealmSilkSocialTestAccess::set_cash(poor,poorActor,3);
    const auto poorBefore=poor.fingerprint();
    check(!poor.execute({poorActor,poor.person(poorActor).decisions+1,
          {Intent::Social,region,int(silk),4,7}}).success&&poor.fingerprint()==poorBefore,
          "a personal cash balance below four rejects the gift without partial spending");

    Realm overflow(20261074,int(kAtlasTerritoryCount)-1,256);
    const uint64_t overflowActor=adult_civilian(overflow,region);
    RealmSilkSocialTestAccess::give(overflow,overflowActor);
    RealmSilkSocialTestAccess::set_consumed(overflow,std::numeric_limits<int64_t>::max());
    const auto overflowBefore=overflow.fingerprint();
    check(!overflow.execute({overflowActor,overflow.person(overflowActor).decisions+1,
          {Intent::Social,region,int(silk),4,7}}).success&&overflow.fingerprint()==overflowBefore,
          "a full consumed-Silk ledger rejects atomically");

    Realm old(20261075,26,128);
    auto oldSave=old.serialize("silk-legacy");test_fixtures::strip_empty_apprenticeship_section(oldSave);oldSave.replace(0,15,"POLYEC_REALM_68");
    Realm migrated(1,1,128);
    check(migrated.deserialize(oldSave,"silk-legacy",error)&&migrated.serialize("silk-legacy").rfind("POLYEC_REALM_75 ",0)==0,
          "schema68 saves without a Silkworm Farm migrate forward to schema70");
    std::cout<<"Silk social gift checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"Silk social gift failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}
