#include "polyec/realm.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace polyec;
namespace {
int checks=0;
void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
}

int main(){try{
    constexpr int region=7,copper=int(GoodId::Copper);
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
          "atlas region 7 has a local public authority");
    check(world.person(otherGovernor).region==8&&world.person(otherGovernor).taxing_authority&&buyer&&worker,
          "region 8 authority and region 7 buyer and adult worker are available for checks");
    const auto candidates=world.candidates(governor);
    constexpr size_t miningSlot=size_t(realm_establishment_candidate_begin+15);
    check(candidates[miningSlot].kind==Intent::CreateEstablishment&&
          candidates[miningSlot].good==int(EstablishmentIndustry::Mining)&&candidates[miningSlot].target==region&&
          world.mask(governor,candidates)[miningSlot]==1.f,
          "region 7 authority receives the Mining creation command");
    const auto created=world.execute({governor,world.person(governor).decisions+1,candidates[miningSlot]});
    check(created.success&&world.establishments().size()==1&&
          world.establishments()[0].industry==EstablishmentIndustry::Mining&&
          world.establishments()[0].source_good==copper&&world.establishments()[0].source_units==100,
          "creating the unique public mine starts a bounded Copper source with 100 ore units");
    const uint64_t site=created.session;
    const auto beforeInvalid=world.fingerprint();
    const auto wrongRegion=world.execute(EstablishmentCommand{otherGovernor,
        world.person(otherGovernor).decisions+1,EstablishmentOwnerKind::Public,8,8,
        EstablishmentIndustry::Mining,1,25});
    check(!wrongRegion.success&&world.fingerprint()==beforeInvalid,
          "a public mine outside atlas region 7 is rejected without a partial change");
    const auto duplicate=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::Mining,1,25});
    check(!duplicate.success&&world.establishments().size()==1&&world.invariant(),
          "the same Copper source cannot be initialized again by creating a duplicate mine");
    const auto authorization=world.execute(WorkAuthorizationCommand{governor,
        world.person(governor).decisions+1,site,worker,WorkAuthorizationOperation::Set,
        world.game_tick(),world.game_tick()+32});
    check(authorization.success,"region 7 authority authorizes an adult worker at the mine");
    const auto work=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
        ActivityKind::Work,region,site});
    check(work.success,"worker submits Work at the mine through the ordinary schedule command");
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==99&&world.establishments()[0].stock[size_t(copper)]==1,
          "one completed Work batch consumes one ore unit and creates one Copper");

    const std::string saved=world.serialize("copper-mine-source-trial-1");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(saved,"copper-mine-source-trial-1",error),error.c_str());
    check(resumed.establishments()[0].source_good==copper&&resumed.establishments()[0].source_units==99&&
          resumed.establishments()[0].stock[size_t(copper)]==1&&resumed.fingerprint()==world.fingerprint(),
          "schema 39 save/load retains the Copper source, produced goods, and world state exactly");

    const auto secondAuthorization=resumed.execute(WorkAuthorizationCommand{governor,
        resumed.person(governor).decisions+1,site,worker,WorkAuthorizationOperation::Set,
        resumed.game_tick(),resumed.game_tick()+32});
    check(secondAuthorization.success,"the worker receives another valid Work window");
    const auto secondWork=resumed.execute(ScheduleCommand{worker,resumed.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,resumed.game_tick(),resumed.game_tick()+32,0,1000,
        ActivityKind::Work,region,site});
    check(secondWork.success,"worker can submit another valid Work interval");
    resumed.advance_ticks(64);
    check(resumed.establishments()[0].source_units==98&&resumed.establishments()[0].stock[size_t(copper)]==2,
          "the mine uses one further ore unit for one further Copper batch");
    resumed.advance(90-resumed.day());
    check(resumed.day()==90&&resumed.establishments()[0].source_units==98,
          "Copper ore does not regenerate at the 90-day Forestry recovery interval");

    std::istringstream saveInput(saved);std::vector<std::string> saveWords;std::string word;
    while(saveInput>>word)saveWords.push_back(word);
    const auto section=std::find(saveWords.begin(),saveWords.end(),"ESTABLISH_14");
    check(section!=saveWords.end(),"serialized mine includes its current establishment section");
    size_t cursor=size_t(section-saveWords.begin())+2;
    cursor+=39+2*std::stoull(saveWords.at(cursor+38));
    const size_t authWorkers=std::stoull(saveWords.at(cursor++));
    for(size_t i=0;i<authWorkers;++i){++cursor;const size_t windows=std::stoull(saveWords.at(cursor++));cursor+=2*windows;}
    for(int ledger=0;ledger<3;++ledger){const size_t entries=std::stoull(saveWords.at(cursor++));cursor+=2*entries;}
    check(saveWords.at(cursor)=="6"&&saveWords.at(cursor+1)=="99","serialized mine identifies Copper and its remaining ore");
    saveWords[cursor]="3";
    std::ostringstream corruptOutput;for(const auto& value:saveWords)corruptOutput<<value<<' ';
    const auto corrupt=corruptOutput.str();
    const auto beforeCorrupt=resumed.fingerprint();
    check(!resumed.deserialize(corrupt,"copper-mine-source-trial-1",error)&&resumed.fingerprint()==beforeCorrupt,
          "a save pairing Mining with Timber is rejected without replacing the live world");

    std::vector<std::string> depletedWords;std::istringstream depletedInput(saved);
    while(depletedInput>>word)depletedWords.push_back(word);
    const auto depletedSection=std::find(depletedWords.begin(),depletedWords.end(),"ESTABLISH_14");
    size_t depletedCursor=size_t(depletedSection-depletedWords.begin())+2;
    depletedCursor+=39+2*std::stoull(depletedWords.at(depletedCursor+38));
    const size_t depletedWorkers=std::stoull(depletedWords.at(depletedCursor++));
    for(size_t i=0;i<depletedWorkers;++i){++depletedCursor;const size_t windows=std::stoull(depletedWords.at(depletedCursor++));depletedCursor+=2*windows;}
    for(int ledger=0;ledger<3;++ledger){const size_t entries=std::stoull(depletedWords.at(depletedCursor++));depletedCursor+=2*entries;}
    depletedWords.at(depletedCursor+1)="0";
    std::ostringstream depletedSave;for(const auto& value:depletedWords)depletedSave<<value<<' ';
    Realm depleted(1,1,128);
    check(depleted.deserialize(depletedSave.str(),"copper-mine-source-trial-1",error),error.c_str());
    const auto renewedAuthorization=depleted.execute(WorkAuthorizationCommand{governor,
        depleted.person(governor).decisions+1,site,worker,WorkAuthorizationOperation::Set,
        depleted.game_tick(),depleted.game_tick()+32});
    check(renewedAuthorization.success,"worker can renew Work authorization when the mine is exhausted");
    const auto exhaustedWork=depleted.execute(ScheduleCommand{worker,depleted.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,depleted.game_tick(),depleted.game_tick()+32,0,1000,
        ActivityKind::Work,region,site});
    check(exhaustedWork.success,"valid Work can continue at an exhausted Copper mine");
    depleted.advance_ticks(64);
    check(depleted.establishments()[0].source_units==0&&depleted.establishments()[0].stock[size_t(copper)]==1,
          "an exhausted mine cannot create extra Copper from valid Work");

    const auto withdrawn=resumed.execute(EstablishmentTransferCommand{governor,
        resumed.person(governor).decisions+1,site,EstablishmentTransferOperation::WithdrawGoods,copper,1});
    check(withdrawn.success&&resumed.establishments()[0].stock[size_t(copper)]==1&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(copper)]==1,
          "public owner transfers produced Copper into the regional market stock");
    const auto purchase=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Buy,region,copper,1}});
    check(purchase.success&&resumed.person(buyer).inventory[size_t(copper)]==1&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(copper)]==0,
          "buyer purchases Copper through the existing market command");
    const auto sale=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Sell,region,copper,1}});
    check(sale.success&&resumed.person(buyer).inventory[size_t(copper)]==0&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(copper)]==1,
          "buyer can sell Copper back through the existing market command");
    check(resumed.invariant()&&world.invariant()&&depleted.invariant(),"live, resumed and exhausted mine worlds preserve Realm invariants");
    std::cout<<"Copper Mining checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"Copper Mining failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}





