#include "work_authorization_fixture.hpp"
#include "polyec/realm.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace polyec;
namespace {
int checks=0;
void check(bool condition,const char* message){++checks;if(!condition)throw std::runtime_error(message);}
uint64_t find_person(const Realm& world,int region,Family family,bool adult=false){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.alive&&person.destination==-1&&person.region==region&&person.family==family&&
           (!adult||person.age_days>=14*365))return id;
    }
    return 0;
}
std::vector<std::string> words(const std::string& save){
    std::istringstream input(save);std::vector<std::string> result;std::string word;
    while(input>>word)result.push_back(word);return result;
}
size_t tea_source_good_index(const std::vector<std::string>& saveWords){
    const auto tag=std::find(saveWords.begin(),saveWords.end(),"ESTABLISH_14");
    if(tag==saveWords.end())throw std::runtime_error("missing ESTABLISH_14 section");
    size_t cursor=size_t(tag-saveWords.begin())+2;
    const size_t watermarks=size_t(std::stoull(saveWords.at(cursor+38)));
    cursor+=39+2*watermarks;
    const size_t workers=size_t(std::stoull(saveWords.at(cursor++)));
    for(size_t i=0;i<workers;++i){++cursor;const size_t windows=size_t(std::stoull(saveWords.at(cursor++)));cursor+=2*windows;}
    for(int ledger=0;ledger<3;++ledger){const size_t entries=size_t(std::stoull(saveWords.at(cursor++)));cursor+=2*entries;}
    return cursor;
}
std::string join(const std::vector<std::string>& values){
    std::ostringstream output;for(const auto& value:values)output<<value<<' ';return output.str();
}
}

int main(){try{
    constexpr int region=14,tea=int(GoodId::Tea);
    Realm world(49202,26,256);
    const uint64_t governor=region+2,otherGovernor=region+3;
    const uint64_t worker=find_person(world,region,Family::Civilian,true);
    const uint64_t traveler=find_person(world,region,Family::Economic,true);
    const uint64_t buyer=traveler;
    check(world.person(governor).taxing_authority&&worker&&buyer&&traveler,"region 14 authority, adult worker and economic traveler are available");
    const auto candidates=world.candidates(governor);
    check(realm_tea_garden_candidate_begin==161&&realm_actions==282&&realm_obs==3766,
          "Tea Garden uses three appended choices without moving actions 0 through 160");
    for(int tier=0;tier<3;++tier)
        check(candidates[size_t(161+tier)].kind==Intent::CreateEstablishment&&
              candidates[size_t(161+tier)].good==int(EstablishmentIndustry::TeaGarden)&&
              world.mask(governor,candidates)[size_t(161+tier)]==1.f,
              "region 14 authority receives a legal Tea Garden funding choice");
    check(world.candidates(traveler)[realm_extended_travel_candidate_begin+26-realm_extended_travel_region_begin].target==26,
          "the prior Travel destination at action 160 remains unchanged");

    const auto created=world.execute({governor,world.person(governor).decisions+1,candidates[161]});
    check(created.success&&world.establishments().size()==1&&
          world.establishments()[0].industry==EstablishmentIndustry::TeaGarden&&
          world.establishments()[0].source_good==tea&&world.establishments()[0].source_units==25,
          "creating one Tea Garden in region 14 initializes its 25 trial harvest units");
    const uint64_t site=created.session;
    const auto wrongRegion=world.execute(EstablishmentCommand{otherGovernor,world.person(otherGovernor).decisions+1,
        EstablishmentOwnerKind::Public,15,15,EstablishmentIndustry::TeaGarden,1,25});
    check(!wrongRegion.success,"Tea Garden cannot be created outside atlas region 14");
    const auto duplicate=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::TeaGarden,1,25});
    check(!duplicate.success&&world.establishments().size()==1&&world.invariant(),
          "a second Tea Garden cannot duplicate the region 14 source");

    const auto authorization=world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,
        site,worker,WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32});
    check(authorization.success,"regional authority authorizes the adult worker at the Tea Garden");
    const auto work=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
        ActivityKind::Work,region,site});
    check(work.success,"worker submits Work through the shared schedule command");
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==24&&world.establishments()[0].stock[size_t(tea)]==1&&
          world.establishments()[0].stock[size_t(GoodId::Silk)]==0,
          "one complete Work batch consumes one source unit and creates Tea only");

    const std::string saved=world.serialize("tea-garden-trial-1");
    check(saved.rfind("POLYEC_REALM_58 ",0)==0&&saved.find(" ESTABLISH_14 1 ")!=std::string::npos,
          "the new Tea industry is written in save schema 40");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(saved,"tea-garden-trial-1",error),error.c_str());
    check(resumed.fingerprint()==world.fingerprint()&&resumed.establishments()[0].source_units==24&&
          resumed.establishments()[0].stock[size_t(tea)]==1,
          "save/load preserves source units and Tea output without repeating production");

    auto corrupt=words(saved);const size_t sourceAt=tea_source_good_index(corrupt);
    check(corrupt.at(sourceAt)=="22"&&corrupt.at(sourceAt+1)=="24","save stores canonical Tea source and remaining units");
    corrupt[sourceAt]=std::to_string(int(GoodId::Paper));const auto before=resumed.fingerprint();
    check(!resumed.deserialize(join(corrupt),"tea-garden-trial-1",error)&&resumed.fingerprint()==before,
          "a Tea Garden paired with the wrong GoodId is rejected without changing the loaded world");

    const auto worksite=resumed.candidates(governor);
    check(worksite[size_t(realm_worksite_candidate_begin+2)].good==tea,
          "the source site's worksite output action points to Tea");
    const auto withdrawal=resumed.execute(EstablishmentTransferCommand{governor,
        resumed.person(governor).decisions+1,site,EstablishmentTransferOperation::WithdrawGoods,tea,1});
    check(withdrawal.success&&resumed.view(governor).provinces[size_t(region)].stock[size_t(tea)]==1,
          "the authority can move Tea from the garden into the regional market stock");
    const auto borrowing=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Borrow,region,0,100,180}});
    check(borrowing.success,"the merchant obtains a normal 100-unit loan before buying the higher-priced Tea");
    const auto bought=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Buy,region,tea,1}});
    if(!bought.success)throw std::runtime_error(std::string("Tea market purchase rejected: ")+bought.reason+
        ", region="+std::to_string(resumed.person(buyer).region)+
        ", cash/funds="+std::to_string(resumed.person(buyer).cash)+"/"+std::to_string(resumed.available_funds(buyer))+
        ", market stock/price="+std::to_string(resumed.view(governor).provinces[size_t(region)].stock[size_t(tea)])+
        "/"+std::to_string(resumed.view(governor).provinces[size_t(region)].price[size_t(tea)]));
    check(bought.success&&resumed.person(buyer).inventory[size_t(tea)]==1,
          "an agent can buy real Tea through the existing market command");
    const auto sold=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Sell,region,tea,1}});
    check(sold.success&&resumed.person(buyer).inventory[size_t(tea)]==0&&
          world.invariant()&&resumed.invariant(),"Tea trade preserves the market and Realm ledgers");

    resumed.advance(89-resumed.day());
    check(resumed.day()==89&&resumed.establishments()[0].source_units==24,
          "source does not refill before the first 90-day season boundary");
    resumed.advance(1);
    check(resumed.day()==90&&resumed.establishments()[0].source_units==49,
          "the day 90 boundary adds exactly 25 Tea harvest units");
    const std::string boundarySave=resumed.serialize("tea-garden-trial-1");
    Realm afterBoundary(1,1,128);
    check(afterBoundary.deserialize(boundarySave,"tea-garden-trial-1",error)&&
          afterBoundary.establishments()[0].source_units==49,
          "saving and loading at the seasonal boundary does not apply the refill twice");
    afterBoundary.advance(90);
    check(afterBoundary.day()==180&&afterBoundary.establishments()[0].source_units==74,
          "the next 90-day season applies one additional refill");
    afterBoundary.advance(270);
    check(afterBoundary.day()==450&&afterBoundary.establishments()[0].source_units==100,
          "seasonal refills stop at the source ledger maximum of 100");

    Realm legacy(49203,26,256);std::string schema39=test_fixtures::downgrade_organization_funds_44_to_43(legacy.serialize("tea-garden-trial-1"));
    schema39.replace(0,std::string("POLYEC_REALM_43").size(),"POLYEC_REALM_39");
    const size_t tag=schema39.find("ESTABLISH_13");schema39.replace(tag,std::string("ESTABLISH_13").size(),"ESTABLISH_9");
    Realm migrated(1,1,128);
    check(migrated.deserialize(schema39,"tea-garden-trial-1",error)&&migrated.establishments().empty()&&
          migrated.serialize("tea-garden-trial-1").rfind("POLYEC_REALM_58 ",0)==0,
          "schema 39 migrates to 40 without inventing a Tea Garden or Tea stock");
    std::cout<<"Tea Garden checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"Tea Garden failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}







