#include "polyec/realm.hpp"
#include "polyec/production_manager.hpp"
#include "work_authorization_fixture.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
void check(bool condition,const char* label){++checks;if(!condition)throw std::runtime_error(label);}
std::vector<std::string> tokens(const std::string& text){
    std::istringstream in(text);std::vector<std::string> out;std::string word;while(in>>word)out.push_back(word);return out;
}
std::string joined(const std::vector<std::string>& words){std::ostringstream out;for(const auto& word:words)out<<word<<' ';return out.str();}
Realm restored(const Realm& source,const std::vector<std::string>& words){
    Realm result(1,1,128);std::string error;
    if(!result.deserialize(joined(words),"config-test",error))throw std::runtime_error(error);
    return result;
}
Realm fixture(const GameplayConfig& config){
    const char* phase="construct";try{
    Realm world(71,2,128,false,config);
    const uint64_t worker=65;const int region=world.person(worker).region;
    const uint64_t owner=region==world.regions()?1:uint64_t(region+2);
    phase="create";
    const auto result=world.execute(EstablishmentCommand{owner,world.person(owner).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),region,EstablishmentIndustry::ClothWorkshop,-1,30});
    if(!result.success)throw std::runtime_error(result.reason);
    phase="supply";
    const auto supplied=world.execute(EstablishmentTransferCommand{owner,world.person(owner).decisions+1,result.session,
        EstablishmentTransferOperation::DepositGoods,9,5});
    if(!supplied.success)throw std::runtime_error(supplied.reason);
    phase="authorize";
    const auto authorized=world.execute(WorkAuthorizationCommand{owner,world.person(owner).decisions+1,result.session,worker,
        WorkAuthorizationOperation::Set,0,32});
    if(!authorized.success)throw std::runtime_error(authorized.reason);
    phase="work";
    const auto work=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
        0,0,32,0,1000,ActivityKind::Work,region,result.session});
    if(!work.success)throw std::runtime_error(work.reason);return world;
    }catch(const std::exception& error){throw std::runtime_error(std::string("config fixture ")+phase+": "+error.what());}
}
Realm site_budget(Realm world,int64_t amount){
    const auto site=world.establishments().at(0);const uint64_t owner=site.region==world.regions()?1:uint64_t(site.region+2);
    const auto operation=amount<site.cash?EstablishmentTransferOperation::WithdrawCash:EstablishmentTransferOperation::DepositCash;
    const int64_t difference=amount<site.cash?site.cash-amount:amount-site.cash;
    if(difference){const auto result=world.execute(EstablishmentTransferCommand{owner,world.person(owner).decisions+1,site.id,operation,-1,difference});
        if(!result.success)throw std::runtime_error(result.reason);}
    return world;
}
void replay(const GameplayConfig& config){
    auto world=fixture(config);int64_t previous=0;
    for(int64_t tick:{0,7,8,16,32,64,128}){
        world.advance_ticks(tick-previous);previous=tick;
        const auto site=world.establishments().at(0);
        const auto at=site.processed_work_units.find(65);
        std::cout<<tick<<' '<<world.fingerprint()<<' '<<config.fingerprint()<<' '<<site.stock[9]<<' '<<site.stock[11]<<' '
                 <<(at==site.processed_work_units.end()?0:at->second)<<' '<<(site.work_pay.remainders.contains(65)?site.work_pay.remainders.at(65):0)<<'\n';
    }
}
}
int main(int argc,char** argv){try{
    if(argc==3&&std::string(argv[1])=="--replay"){replay(GameplayConfig::load(argv[2]));return 0;}
    const std::string defaults=R"(schema_version = 1
[work]
ticks_per_unit = 16
wage_per_unit = 1
[establishment]
default_capacity = 1
[recipes.cloth]
fiber_input = 1
cloth_output = 1
[recipes.tools]
iron_input = 1
fuel_input = 1
tools_output = 1
[recipes.military_supply]
tools_input = 1
cloth_input = 1
military_supply_output = 1
)";
    check(GameplayConfig::parse(defaults)==GameplayConfig{},"file defaults equal frozen historical defaults");
    auto marketV2=defaults;marketV2.replace(marketV2.find("schema_version = 1"),18,"schema_version = 2");
    marketV2+="[market]\nsystem_version = 1\nresponse_basis_points = 1000\nmaximum_change_basis_points = 1000\nprice_floor = 1\nprice_ceiling = 1000000\n";
    const auto marketConfig=GameplayConfig::parse(marketV2);
    check(marketConfig==GameplayConfig{}&&marketConfig.market_enabled()&&
          marketConfig.market_response_basis_points()==1000&&marketConfig.market_maximum_change_basis_points()==1000&&
          marketConfig.market_price_floor()==1&&marketConfig.market_price_ceiling()==1000000,
          "config v2 freezes the trial monthly market settings");
    auto saltV3=marketV2;const auto versionAt=saltV3.find("schema_version = 2");
    saltV3.replace(versionAt,std::string("schema_version = 2").size(),"schema_version = 3");
    saltV3+="[recipes.salt]\nfuel_input = 2\nsalt_output = 3\n";
    const auto saltConfig=GameplayConfig::parse(saltV3);
    check(saltConfig.salt_fuel_input()==2&&saltConfig.salt_output()==3,
          "config v3 records and exposes the Saltworks input and output quantities");
    check(GameplayConfig::historical().historical_defaults()&&!GameplayConfig::historical().market_enabled(),
          "explicit historical config keeps dynamic prices disabled");
    auto changed=GameplayConfig{}.values();changed[0]=8;changed[1]=3;changed[2]=2;changed[3]=2;changed[4]=3;
    const auto config=GameplayConfig::from_values(changed);
    check(config.fingerprint()!=GameplayConfig{}.fingerprint(),"normalized config identity distinguishes rules");
    auto rejected=[&](const std::string& text,const std::string& path){
        try{GameplayConfig::parse(text);check(false,"invalid config accepted");}
        catch(const std::invalid_argument& error){check(std::string(error.what()).find(path)!=std::string::npos,"rejection names config field");}
    };
    for(const std::string value:{"0","-1","65","1.5","01","18446744073709551616"}){
        auto text=defaults;const auto at=text.find("ticks_per_unit = 16");text.replace(at,19,"ticks_per_unit = "+value);rejected(text,"work.ticks_per_unit");
    }
    rejected(defaults+"mystery = 2\n","recipes.military_supply.mystery");
    rejected(defaults+"military_supply_output = 2\n","recipes.military_supply.military_supply_output");
    auto invertedMarket=marketV2;const auto floorAt=invertedMarket.find("price_floor = 1");invertedMarket.replace(floorAt,15,"price_floor = 20");
    const auto ceilingAt=invertedMarket.find("price_ceiling = 1000000");invertedMarket.replace(ceilingAt,std::string("price_ceiling = 1000000").size(),"price_ceiling = 10");
    rejected(invertedMarket,"market.price_floor");
    auto badResponse=marketV2;const auto responseAt=badResponse.find("response_basis_points = 1000");badResponse.replace(responseAt,std::string("response_basis_points = 1000").size(),"response_basis_points = 10001");
    rejected(badResponse,"market.response_basis_points");
    rejected(saltV3.substr(0,saltV3.find("salt_output = 3")),"recipes.salt.salt_output");
    rejected(defaults+"[work]\n","work");
    rejected(defaults.substr(defaults.find('\n')+1),"schema_version");
    rejected(defaults.substr(0,defaults.find("military_supply_output")),"recipes.military_supply.military_supply_output");
    check(GameplayConfig::parse("# header\r\n"+defaults+"# tail\n")==GameplayConfig{},"whitespace and comments normalize identically");
    check(GameplayConfig::parse(defaults+"# \xc4\x91\n")==GameplayConfig{},"valid UTF-8 comments normalize identically");
    rejected(defaults+"# invalid \xff\n","$encoding");
    rejected(defaults+std::string("# forbidden ")+char(1)+"\n","$encoding");
    rejected(defaults+"# bare carriage return\r","$encoding");
    auto world=fixture(config);
    check(world.establishments()[0].capacity==2,"omitted capacity uses snapshot default");
    check(!world.pilot_policy_compatible(),"custom gameplay rejects historical pilot model");
    auto uninterrupted=world;uninterrupted.advance_ticks(64);
    world.advance_ticks(8);const auto saved=world.serialize("config-test");
    Realm resumed(1,1,128);std::string error;check(resumed.deserialize(saved,"config-test",error),error.c_str());
    resumed.advance_ticks(56);
    check(resumed.fingerprint()==uninterrupted.fingerprint(),"custom rules survive mid-work save and replay");
    const auto site=resumed.establishments()[0];
    const int64_t expectedProductionUnits=(32*(150+resumed.person(65).skill))/(200*config.ticks_per_unit());
    check(site.stock[9]==1&&site.stock[11]==6&&site.processed_work_units.at(65)==expectedProductionUnits,
          "custom recipe produces two batches while work-unit count reflects the worker's skill");
    check(resumed.schedule_book().settled_work_units.at(65)==4,"settlement uses same configured work unit");
    const auto wageKey=WorkRegionKey{65,resumed.person(65).region};
    check(site.work_pay.paid_units.at(65)==4&&resumed.invariant(),"configured wages conserve money and settle four units");
    {
        Establishment input{};input.id=1;input.stock[9]=std::numeric_limits<int64_t>::max();
        std::vector<Person> people(1);people[0].id=1;people[0].skill=50;
        const std::map<WorkEstablishmentKey,int64_t> attendance{{{1,1},8}};
        const std::map<WorkEstablishmentKey,int64_t> attention{{{1,1},8*kAttentionUnitsPerEffortTick}};
        const std::map<WorkEstablishmentKey,int64_t> productivity{{{1,1},8*kAttentionUnitsPerEffortTick*200}};
        std::vector<Establishment> sites{input};
        const auto plan=ProductionManager::plan({sites,attendance,attention,productivity,people,1,config});
        check(plan.site_stock_delta(0,9)==-2&&plan.site_stock_delta(0,11)==3,"large available input uses division safely with non-unit recipe quantities");
        sites[0].stock[11]=std::numeric_limits<int64_t>::max()-2;
        try{ProductionManager::plan({sites,attendance,attention,productivity,people,1,config});check(false,"configured output overflow accepted");}
        catch(const std::overflow_error&){check(sites[0].stock[9]==std::numeric_limits<int64_t>::max(),"output overflow leaves input stock unchanged");}
    }

    auto shortage=site_budget(fixture(config),1);const auto openingMoney=shortage.total_money();
    shortage.advance_ticks(64);
    check(shortage.establishments()[0].work_pay.remainders.at(65)==1&&shortage.establishments()[0].work_pay.paid_units.count(65)==0,"partial wage retains one money unit without crediting a complete work unit");
    check(shortage.total_money()==openingMoney&&shortage.invariant(),"partial wage is conserved");
    auto badRemainder=tokens(shortage.serialize("config-test"));
    size_t payAt=std::find(badRemainder.begin(),badRemainder.end(),"ESTABLISH_14")-badRemainder.begin()+2;
    payAt+=39+2*std::stoull(badRemainder.at(payAt+38));
    const size_t workers=std::stoull(badRemainder.at(payAt++));
    for(size_t row=0;row<workers;++row)payAt+=2+2*std::stoull(badRemainder.at(payAt+1));
    for(int map=0;map<2;++map)payAt+=1+2*std::stoull(badRemainder.at(payAt));
    badRemainder.at(payAt+2)="3";
    const auto shortageBefore=shortage.fingerprint();
    check(!shortage.deserialize(joined(badRemainder),"config-test",error)&&shortage.fingerprint()==shortageBefore,"remainder equal to a full wage rejects atomically");
    Realm shortLoaded(1,1,128);check(shortLoaded.deserialize(shortage.serialize("config-test"),"config-test",error),error.c_str());
    auto next=site_budget(shortage,2);auto nextLoaded=site_budget(shortLoaded,2);
    next.advance_ticks(64);nextLoaded.advance_ticks(64);
    check(next.establishments()[0].work_pay.remainders.count(65)==0&&next.establishments()[0].work_pay.paid_units.at(65)==1,"later two money units complete exactly the previously partial wage");
    check(next.fingerprint()==nextLoaded.fingerprint()&&next.invariant(),"partial wage replay after load is exact");

    auto corrupt=tokens(saved);const auto configAt=std::find(corrupt.begin(),corrupt.end(),"CONFIG_3")-corrupt.begin();
    corrupt.at(size_t(configAt)+2)="9";const auto before=resumed.fingerprint();
    check(!resumed.deserialize(joined(corrupt),"config-test",error)&&resumed.fingerprint()==before,"mismatched snapshot fingerprint rejects without replacing world");
    Realm historical(71,2,128);auto old=test_fixtures::save_as_v15(historical.serialize("config-test"));
    old.replace(0,15,"POLYEC_REALM_14");
    const auto historicalConfig=old.find(" CONFIG_1 ");check(historicalConfig!=std::string::npos,"v14 fixture locates config tail");old.erase(historicalConfig);
    {
        std::istringstream header(old);std::string magic,bundle;int day=0,regions=0;uint64_t seed=0,next=0;
        header>>magic>>std::quoted(bundle)>>day>>regions>>seed;const auto seedEnd=header.tellg();header>>next;
        const auto begin=static_cast<size_t>(seedEnd)+1;const auto end=old.find(' ',begin);
        check(end!=std::string::npos&&next>=historical.land_holdings().size(),"v14 fixture locates pre-land identifier watermark");
        old.replace(begin,end-begin,std::to_string(next-historical.land_holdings().size()));
    }
    const bool migratedHistorical=resumed.deserialize(old,"config-test",error);
    check(migratedHistorical,error.c_str());
    check(resumed.gameplay_config().historical_defaults(),
          "v14 migration restores frozen gameplay defaults");
    check(resumed.land_holdings().empty(),
          "v14 migration does not fabricate land titles");
    check(resumed.invariant(),"v14 migrated world satisfies Realm invariants");
    const auto file=std::filesystem::temp_directory_path()/("polyec-config-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".toml");
    struct Cleanup{std::filesystem::path path;~Cleanup(){std::error_code ignored;std::filesystem::remove(path,ignored);}} cleanup{file};
    {std::ofstream out(file);out<<defaults;}
    const auto fileConfig=GameplayConfig::load(file.string());Realm fromFile(71,2,128,false,fileConfig);
    const auto fileSave=fromFile.serialize("config-test");
    {std::ofstream out(file);out<<"schema_version = 999\n";}
    Realm afterFileEdit(71,2,128,false,config);
    check(afterFileEdit.deserialize(fileSave,"config-test",error)&&afterFileEdit.gameplay_config()==fileConfig,"save resumes its own snapshot after external file changes");
    std::cout<<"Gameplay config checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';return 1;}}






