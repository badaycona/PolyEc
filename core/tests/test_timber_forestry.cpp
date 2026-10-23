#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
std::vector<std::string> tokens(const std::string& save){
    std::istringstream input(save);std::vector<std::string> words;std::string word;
    while(input>>word)words.push_back(word);return words;
}
std::string join(const std::vector<std::string>& words){
    std::ostringstream output;for(const auto& word:words)output<<word<<' ';return output.str();
}
size_t forest_field(std::vector<std::string>& words){
    const auto tag=std::find(words.begin(),words.end(),"ESTABLISH_14");
    if(tag==words.end())throw std::runtime_error("missing current establishment save section");
    const size_t section=size_t(tag-words.begin()),count=std::stoull(words.at(section+1));
    if(count!=1)throw std::runtime_error("expected one Forestry site in save fixture");
    size_t cursor=section+2;
    const size_t watermarks=std::stoull(words.at(cursor+38));cursor+=39+2*watermarks;
    const size_t workers=std::stoull(words.at(cursor++));
    for(size_t worker=0;worker<workers;++worker){
        ++cursor;const size_t windows=std::stoull(words.at(cursor++));cursor+=2*windows;
    }
    for(int ledger=0;ledger<3;++ledger){
        const size_t entries=std::stoull(words.at(cursor++));cursor+=2*entries;
    }
    return cursor+1;
}
}

int main(){try{
    constexpr int region=6;
    constexpr int timber=int(GoodId::Timber);
    Realm world(49201,26,256);
    const uint64_t governor=region+2,otherGovernor=region+3;
    uint64_t buyer=0,worker=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.region!=region||!person.alive)continue;
        if(person.family==Family::Economic&&!buyer)buyer=id;
        if(person.family==Family::Civilian&&person.age_days>=14*365&&!worker)worker=id;
    }
    check(world.person(governor).region==region&&world.person(governor).taxing_authority,
          "atlas region 6 has a local public authority");
    check(world.person(otherGovernor).region==7&&world.person(otherGovernor).taxing_authority,
          "atlas region 7 has a separate public authority for rejection checks");
    check(buyer&&worker,"atlas region 6 has a market buyer and adult worker");

    const auto candidates=world.candidates(governor);
    constexpr size_t forestrySlot=size_t(realm_establishment_candidate_begin+12);
    check(candidates[forestrySlot].kind==Intent::CreateEstablishment&&
          candidates[forestrySlot].good==int(EstablishmentIndustry::Forestry)&&
          candidates[forestrySlot].target==region&&world.mask(governor,candidates)[forestrySlot]==1.f,
          "region 6 authority receives the Forestry creation command");
    const auto created=world.execute({governor,world.person(governor).decisions+1,candidates[forestrySlot]});
    check(created.success&&world.establishments().size()==1&&
          world.establishments()[0].industry==EstablishmentIndustry::Forestry&&
          world.establishments()[0].source_good==timber&&
          world.establishments()[0].source_units==kEstablishmentSourceUnitsMaximum,
          "creating the unique public Forestry site initializes its bounded resource once");
    const uint64_t site=created.session;

    const auto beforeInvalid=world.fingerprint();
    const auto wrongRegion=world.execute(EstablishmentCommand{otherGovernor,
        world.person(otherGovernor).decisions+1,EstablishmentOwnerKind::Public,7,7,
        EstablishmentIndustry::Forestry,1,25});
    check(!wrongRegion.success&&world.fingerprint()==beforeInvalid,
          "a public Forestry site outside atlas region 6 is rejected without a partial change");
    const auto duplicate=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::Forestry,1,25});
    check(!duplicate.success&&world.establishments().size()==1&&world.invariant(),
          "the same forest cannot be initialized again by creating a duplicate site");

    const auto authorization=world.execute(WorkAuthorizationCommand{governor,
        world.person(governor).decisions+1,site,worker,WorkAuthorizationOperation::Set,
        world.game_tick(),world.game_tick()+32});
    check(authorization.success,"region 6 authority authorizes a present worker at the Forestry site");
    const auto work=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
        ActivityKind::Work,region,site});
    check(work.success,"worker submits production Work through the ordinary schedule command");
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==99&&
          world.establishments()[0].stock[size_t(timber)]==1,
          "one completed batch consumes one forest unit and creates one Timber");

    const std::string saved=world.serialize("timber-forest-source-trial-1");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(saved,"timber-forest-source-trial-1",error),error.c_str());
    check(resumed.establishments()[0].source_units==99&&
          resumed.establishments()[0].stock[size_t(timber)]==1&&resumed.fingerprint()==world.fingerprint(),
          "schema 40 save/load retains the remaining forest and produced Timber exactly");
    const auto legacyV37=test_fixtures::downgrade_establishments_8_to_7(
        test_fixtures::downgrade_establishments_9_to_8(
        test_fixtures::downgrade_establishments_10_to_9(
            test_fixtures::downgrade_establishments_11_to_10(
                test_fixtures::downgrade_establishments_12_to_11(
                    test_fixtures::downgrade_establishments_13_to_12(saved))))));
    Realm migrated(1,1,128);
    check(migrated.deserialize(legacyV37,"timber-forest-source-trial-1",error),error.c_str());
    check(migrated.establishments()[0].source_good==timber&&migrated.establishments()[0].source_units==99&&
          migrated.establishments()[0].stock[size_t(timber)]==1&&migrated.invariant(),
          "schema 37 forest units migrate to the shared Timber source ledger without changing quantity");
    resumed.advance_ticks(64);
    check(resumed.establishments()[0].source_units==99&&
          resumed.establishments()[0].stock[size_t(timber)]==1,
          "loading a completed production day does not consume or create a second batch");

    auto malformed=tokens(saved);malformed.at(forest_field(malformed))="101";
    const auto beforeMalformed=resumed.fingerprint();
    check(!resumed.deserialize(join(malformed),"timber-forest-source-trial-1",error)&&
          resumed.fingerprint()==beforeMalformed,
          "save data above the forest limit is rejected without replacing the live world");

    auto depletedWords=tokens(saved);depletedWords.at(forest_field(depletedWords))="0";
    Realm depleted(1,1,128);
    check(depleted.deserialize(join(depletedWords),"timber-forest-source-trial-1",error),error.c_str());
    const auto secondAuthorization=depleted.execute(WorkAuthorizationCommand{governor,
        depleted.person(governor).decisions+1,site,worker,WorkAuthorizationOperation::Set,
        depleted.game_tick(),depleted.game_tick()+32});
    check(secondAuthorization.success,"the remaining authorized work window can be renewed at the site");
    const auto secondWork=depleted.execute(ScheduleCommand{worker,depleted.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,depleted.game_tick(),depleted.game_tick()+32,0,1000,
        ActivityKind::Work,region,site});
    check(secondWork.success,"worker can submit another valid Work interval at an exhausted site");
    depleted.advance_ticks(64);
    check(depleted.establishments()[0].source_units==0&&
          depleted.establishments()[0].stock[size_t(timber)]==1,
          "valid Work at an exhausted forest cannot create extra Timber");

    const auto withdrawn=resumed.execute(EstablishmentTransferCommand{governor,
        resumed.person(governor).decisions+1,site,EstablishmentTransferOperation::WithdrawGoods,timber,1});
    check(withdrawn.success&&resumed.establishments()[0].stock[size_t(timber)]==0&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(timber)]==1,
          "public owner transfers the produced Timber to the regional market stock");
    const auto purchase=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Buy,region,timber,1}});
    check(purchase.success&&resumed.person(buyer).inventory[size_t(timber)]==1&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(timber)]==0,
          "buyer purchases Forestry output through the existing market transaction");
    const auto sale=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Sell,region,timber,1}});
    check(sale.success&&resumed.person(buyer).inventory[size_t(timber)]==0&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(timber)]==1,
          "buyer can return Timber to the same market through the normal Sell command");

    const auto paused=resumed.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
    check(paused.success&&resumed.paused(),"game clock can pause before a forest recovery day");
    const int pausedDay=resumed.day();const int64_t pausedForest=resumed.establishments()[0].source_units;
    bool pauseBlocked=false;try{resumed.advance_ticks(64);}catch(const std::runtime_error&){pauseBlocked=true;}
    check(pauseBlocked&&resumed.day()==pausedDay&&resumed.establishments()[0].source_units==pausedForest,
          "pause prevents game days and forest recovery from advancing");
    check(resumed.execute(RealmClockCommand{RealmClockCommand::Operation::Resume}).success,
          "game clock resumes through its ordinary control command");
    resumed.advance(90-pausedDay);
    check(resumed.day()==90&&resumed.establishments()[0].source_units==100,
          "one completed 90-day game cycle restores one forest unit up to the cap");
    check(resumed.invariant()&&world.invariant()&&depleted.invariant(),
          "live, resumed and exhausted Forestry worlds preserve Realm invariants");
    std::cout<<"Timber Forestry checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"Timber Forestry failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}








