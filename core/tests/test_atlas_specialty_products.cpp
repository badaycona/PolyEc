#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t find_person(const Realm& world,int region,Family family,bool adult=false){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& p=world.person(id);
        if(p.alive&&p.destination==-1&&p.region==region&&p.family==family&&(!adult||p.age_days>=14*365))return id;
    }
    return 0;
}
struct ProductCase { int region; EstablishmentIndustry industry; GoodId good; int candidate; bool burns_fuel; };
}
int main(){try{
    constexpr ProductCase products[]{
        {10,EstablishmentIndustry::Glassworks,GoodId::PlainGlass,264,true},
        {15,EstablishmentIndustry::PotteryKiln,GoodId::Pottery,267,true},
        {17,EstablishmentIndustry::Tannery,GoodId::Leather,270,false}};
    for(const auto& product:products){
        Realm world(uint64_t(60100+product.region),26,256);
        const uint64_t authority=uint64_t(product.region+2);
        const uint64_t worker=find_person(world,product.region,Family::Civilian,true);
        check(authority<=uint64_t(world.count())&&world.person(authority).taxing_authority&&worker,
              "the source region has a public authority and an adult worker");
        const auto candidates=world.candidates(authority);
        check(realm_schema==std::string_view("realm-v36")&&realm_actions==400&&realm_obs==5182,
              "the product additions use the versioned v31 observation and action contract");
        const auto& create=candidates[size_t(product.candidate)];
        check(create.kind==Intent::CreateEstablishment&&create.target==product.region&&
              create.good==int(product.industry)&&world.mask(authority,candidates)[size_t(product.candidate)]==1.f,
              "the matching Atlas authority receives a legal creation choice");
        const auto created=world.execute({authority,world.person(authority).decisions+1,create});
        check(created.success,"the authority creates the specialty producer with its real treasury funds");
        const uint64_t site=created.session;
        const auto initial=world.establishments().front();
        check(initial.source_good==int(product.good)&&initial.source_units==kEstablishmentSourceUnitsMaximum&&
              initial.stock[size_t(product.good)]==0,
              "a source ledger is finite and separate from product inventory");
        const auto wrong=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
            EstablishmentOwnerKind::Public,uint64_t(product.region),product.region-1,product.industry,1,25});
        check(!wrong.success&&world.establishments().size()==1&&world.invariant(),
              "a direct command rejects a producer in the wrong Atlas region without partial creation");
        const auto duplicate=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
            EstablishmentOwnerKind::Public,uint64_t(product.region),product.region,product.industry,1,25});
        check(!duplicate.success&&world.establishments().size()==1&&world.invariant(),
              "the finite Atlas source allows only one producer site in its region");
        if(product.burns_fuel){
            const int64_t fuel=world.view(authority).provinces[size_t(product.region)].stock[size_t(GoodId::Fuel)];
            check(fuel>0,"the region has real Fuel available for its kiln recipe");
            check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,
                site,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1}).success,
                "the owner transfers one actual Fuel into the producer");
        }
        check(world.execute(WorkAuthorizationCommand{authority,world.person(authority).decisions+1,site,worker,
              WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
              "the owner authorizes a present adult worker for the upcoming work interval");
        check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
              ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
              ActivityKind::Work,product.region,site}).success,
              "the worker submits the authorized interval through the normal schedule command");
        world.advance_ticks(64);
        const auto produced=world.establishments().front();
        check(produced.stock[size_t(product.good)]==1&&produced.source_units==kEstablishmentSourceUnitsMaximum-1&&
              (!product.burns_fuel||produced.stock[size_t(GoodId::Fuel)]==0)&&world.invariant(),
              "one complete Work batch consumes its finite source and real Fuel when the recipe requires it");
        const auto worksite=world.candidates(authority);
        bool reports_product=false;
        for(size_t i=size_t(realm_worksite_candidate_begin);i<size_t(realm_worksite_candidate_begin+
             realm_worksite_candidate_sites*realm_worksite_actions_per_site);++i)
            reports_product|=worksite[i].good==int(product.good);
        check(reports_product,"worksite actions report the exact canonical output GoodId");
        const std::string saved=world.serialize("atlas-specialty-products-v1");
        check(saved.rfind("POLYEC_REALM_69 ",0)==0&&saved.find(" ESTABLISH_14 1 ")!=std::string::npos,
              "the current save records 30 goods and establishment schema14");
        Realm loaded(1,1,128);std::string error;
        if(!loaded.deserialize(saved,"atlas-specialty-products-v1",error)){
            const auto marker=saved.find(" SCHEDULE_");
            throw std::runtime_error("schema64 load rejected the specialty save: "+error+" near "+
                saved.substr(marker,120));
        }
        check(true,"schema64 load accepts the specialty save");
        check(loaded.fingerprint()==world.fingerprint()&&loaded.establishments().front().stock[size_t(product.good)]==1&&
              loaded.establishments().front().source_units==kEstablishmentSourceUnitsMaximum-1,
              "save/load preserves the new product, remaining finite source and no extra production");
        const int64_t marketBefore=loaded.view(authority).provinces[size_t(product.region)].stock[size_t(product.good)];
        check(loaded.execute(EstablishmentTransferCommand{authority,loaded.person(authority).decisions+1,
              site,EstablishmentTransferOperation::WithdrawGoods,int(product.good),1}).success&&
              loaded.view(authority).provinces[size_t(product.region)].stock[size_t(product.good)]==marketBefore+1,
              "the public site owner withdraws the exact specialty output into the regional market through the ordinary transfer command");
        check(loaded.invariant(),
              "the regional market ledger and new product GoodId remain valid after withdrawal");
        const uint64_t buyer=find_person(loaded,product.region,Family::Economic,true);
        check(buyer!=0,"the specialty region has a real adult economic market participant");
        const auto borrowed=loaded.execute({buyer,loaded.person(buyer).decisions+1,
              {Intent::Borrow,product.region,0,300,180}});
        check(borrowed.success,"a market participant obtains funds using the existing loan command");
        const auto bought=loaded.execute({buyer,loaded.person(buyer).decisions+1,
              {Intent::Buy,product.region,int(product.good),1}});
        check(bought.success&&loaded.person(buyer).inventory[size_t(product.good)]==1&&
              loaded.view(authority).provinces[size_t(product.region)].stock[size_t(product.good)]==marketBefore,
              "the buyer purchases the exact specialty GoodId from the regional market");
        const auto sold=loaded.execute({buyer,loaded.person(buyer).decisions+1,
              {Intent::Sell,product.region,int(product.good),1}});
        check(sold.success&&loaded.person(buyer).inventory[size_t(product.good)]==0&&
              loaded.view(authority).provinces[size_t(product.region)].stock[size_t(product.good)]==marketBefore+1&&
              loaded.invariant(),
              "the buyer can sell the exact specialty GoodId back through the existing market command");
    }
    std::ifstream fixture(std::string(POLYEC_TEST_FIXTURE_DIR)+"/atlas-specialty-schema54.save",std::ios::binary);
    check(bool(fixture),"the fixed schema54 migration fixture is available");
    const std::string legacy((std::istreambuf_iterator<char>(fixture)),std::istreambuf_iterator<char>());
    check(legacy.rfind("POLYEC_REALM_54 ",0)==0,"the migration fixture was written by the previous schema54 serializer");
    Realm migrated(1,1,128);std::string migrationError;
    if(!migrated.deserialize(legacy,"atlas-specialty-migration-v1",migrationError))
        throw std::runtime_error("schema54 migration rejected the old save: "+migrationError);
    check(migrated.establishments().size()==1&&migrated.establishments()[0].industry==EstablishmentIndustry::Forestry&&
          migrated.establishments()[0].source_good==int(GoodId::Timber)&&
          migrated.establishments()[0].source_units==kEstablishmentSourceUnitsMaximum&&
          migrated.invariant()&&
          migrated.serialize("atlas-specialty-migration-v1").rfind("POLYEC_REALM_69 ",0)==0,
          "a real schema54 world retains its existing Forestry site and loads forward as schema55");
    for(uint64_t person=1;person<=uint64_t(migrated.count());++person)
        for(int good=27;good<30;++good)
            check(migrated.person(person).inventory[size_t(good)]==0,"schema54 migration does not invent new personal goods");
    const auto migratedView=migrated.view(1);
    for(const auto& province:migratedView.provinces)for(int good=27;good<30;++good)
        check(province.stock[size_t(good)]==0,"schema54 migration does not invent new regional stock");
    Realm currentForDowngrade(20261024,26,256);
    const uint64_t forestAuthority=8;
    check(currentForDowngrade.execute(EstablishmentCommand{forestAuthority,
          currentForDowngrade.person(forestAuthority).decisions+1,EstablishmentOwnerKind::Public,
          6,6,EstablishmentIndustry::Forestry,1,25}).success,
          "a current world with a pre-existing source site is prepared for an old-save-reader check");
    std::string downgraded=currentForDowngrade.serialize("atlas-specialty-migration-v1");
    test_fixtures::downgrade_save_55_to_54(downgraded);
    Realm schema54Reader(1,1,128);std::string downgradeError;
    if(!schema54Reader.deserialize(downgraded,"atlas-specialty-migration-v1",downgradeError))
        throw std::runtime_error("schema55-to-54 compatibility fixture is malformed: "+downgradeError);
    check(schema54Reader.establishments().size()==1&&schema54Reader.establishments()[0].source_units==100&&
          schema54Reader.invariant(),"the schema55-to-54 test converter preserves the old Forestry row");
    std::cout<<"Atlas specialty product checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"Atlas specialty product failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}

