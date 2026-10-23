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
    constexpr int region=23,silver=int(GoodId::Silver);
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
          "atlas region 23 has a local public authority");
    check(world.person(otherGovernor).region==24&&world.person(otherGovernor).taxing_authority&&buyer&&worker,
          "region 24 authority and region 23 buyer and adult worker are available for checks");
    check(world.mask(governor)[95]==1.f&&world.mask(governor)[96]==1.f&&world.mask(governor)[97]==1.f,
          "region 23 authority receives the existing three Mining funding choices");
    const auto created=world.execute({governor,world.person(governor).decisions+1,world.candidates(governor)[95]});
    check(created.success&&world.establishments().size()==1&&
          world.establishments()[0].industry==EstablishmentIndustry::Mining&&
          world.establishments()[0].source_good==silver&&world.establishments()[0].source_units==100,
          "creating the unique region 23 Mining site initializes a bounded Silver source");
    const uint64_t site=created.session;
    const auto wrongRegion=world.execute(EstablishmentCommand{otherGovernor,
        world.person(otherGovernor).decisions+1,EstablishmentOwnerKind::Public,24,24,
        EstablishmentIndustry::Mining,1,25});
    check(!wrongRegion.success,"Mining cannot create a Silver source outside atlas region 23");
    const auto duplicate=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::Mining,1,25});
    check(!duplicate.success&&world.establishments().size()==1&&world.invariant(),
          "Silver site cannot be initialized a second time in the same region");

    const auto authorization=world.execute(WorkAuthorizationCommand{governor,
        world.person(governor).decisions+1,site,worker,WorkAuthorizationOperation::Set,
        world.game_tick(),world.game_tick()+32});
    check(authorization.success,"region 23 authority authorizes a worker at the Silver mine");
    const auto work=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
        ActivityKind::Work,region,site});
    check(work.success,"worker submits Work at the Silver mine through the shared schedule command");
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==99&&world.establishments()[0].stock[size_t(silver)]==1&&
          world.establishments()[0].stock[size_t(GoodId::Copper)]==0,
          "one batch consumes one Silver ore unit and creates Silver rather than Copper");

    const std::string saved=world.serialize("silver-mine-source-trial-1");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(saved,"silver-mine-source-trial-1",error),error.c_str());
    check(resumed.establishments()[0].source_good==silver&&resumed.establishments()[0].source_units==99&&
          resumed.establishments()[0].stock[size_t(silver)]==1&&resumed.fingerprint()==world.fingerprint(),
          "schema 39 roundtrip retains Silver source and production exactly");

    std::istringstream input(saved);std::vector<std::string> words;std::string word;
    while(input>>word)words.push_back(word);
    const auto section=std::find(words.begin(),words.end(),"ESTABLISH_14");
    size_t cursor=size_t(section-words.begin())+2;
    cursor+=39+2*std::stoull(words.at(cursor+38));
    const size_t authWorkers=std::stoull(words.at(cursor++));
    for(size_t i=0;i<authWorkers;++i){++cursor;const size_t windows=std::stoull(words.at(cursor++));cursor+=2*windows;}
    for(int ledger=0;ledger<3;++ledger){const size_t entries=std::stoull(words.at(cursor++));cursor+=2*entries;}
    check(words.at(cursor)=="7"&&words.at(cursor+1)=="99","save labels the Mining source as Silver");
    words[cursor]=std::to_string(int(GoodId::Copper));
    std::ostringstream corruptOutput;for(const auto& value:words)corruptOutput<<value<<' ';
    const auto beforeCorrupt=resumed.fingerprint();
    check(!resumed.deserialize(corruptOutput.str(),"silver-mine-source-trial-1",error)&&
          resumed.fingerprint()==beforeCorrupt,
          "a save pairing Mining at region 23 with Copper is rejected atomically");

    resumed.advance(90-resumed.day());
    check(resumed.day()==90&&resumed.establishments()[0].source_units==99,
          "Silver ore does not regenerate at the 90-day Forestry recovery interval");
    const auto outputCandidates=resumed.candidates(governor);
    check(outputCandidates[size_t(realm_worksite_candidate_begin+2)].good==silver&&
          resumed.mask(governor,outputCandidates)[size_t(realm_worksite_candidate_begin+2)]==1.f,
          "worksite output candidate identifies and permits withdrawal of Silver");

    const auto withdrawn=resumed.execute(EstablishmentTransferCommand{governor,
        resumed.person(governor).decisions+1,site,EstablishmentTransferOperation::WithdrawGoods,silver,1});
    check(withdrawn.success&&resumed.view(governor).provinces[size_t(region)].stock[size_t(silver)]==1,
          "public mine transfers Silver into the regional market stock");
    const auto purchase=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Buy,region,silver,1}});
    check(purchase.success&&resumed.person(buyer).inventory[size_t(silver)]==1,
          "buyer purchases Silver through the existing market command");
    const auto sale=resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Sell,region,silver,1}});
    check(sale.success&&resumed.person(buyer).inventory[size_t(silver)]==0&&
          resumed.view(governor).provinces[size_t(region)].stock[size_t(silver)]==1,
          "buyer can sell Silver back through the existing market command");
    check(resumed.invariant()&&world.invariant(),"live and resumed Silver mine worlds preserve Realm invariants");
    std::cout<<"Silver Mining checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"Silver Mining failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}





