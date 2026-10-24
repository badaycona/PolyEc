#include "work_authorization_fixture.hpp"
#include "polyec/realm.hpp"
#include <iostream>
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
}
int main(){try{
    constexpr int region=11,incense=int(GoodId::Incense);
    Realm world(49301,26,256);
    const uint64_t governor=region+2,worker=find_person(world,region,Family::Civilian,true);
    const uint64_t buyer=find_person(world,region,Family::Economic,true);
    check(world.person(governor).taxing_authority&&worker&&buyer,"region 11 has authority, worker and buyer actors");
    const auto candidates=world.candidates(governor);
    check(realm_incense_works_candidate_begin==164&&realm_actions==400&&realm_obs==5182,
          "Incense choices are appended after existing actions and match observation size");
    for(int tier=0;tier<3;++tier)
        check(candidates[size_t(164+tier)].kind==Intent::CreateEstablishment&&
              candidates[size_t(164+tier)].good==int(EstablishmentIndustry::IncenseWorks)&&
              world.mask(governor,candidates)[size_t(164+tier)]==1.f,
              "authority receives the three legal Incense Works funding choices");
    const auto created=world.execute({governor,world.person(governor).decisions+1,candidates[164]});
    check(created.success&&world.establishments().size()==1&&
          world.establishments()[0].industry==EstablishmentIndustry::IncenseWorks&&
          world.establishments()[0].source_good==incense&&world.establishments()[0].source_units==100,
          "region 11 site begins with 100 abstract source units for Incense");
    const uint64_t site=created.session;
    const auto wrong=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,10,EstablishmentIndustry::IncenseWorks,1,25});
    check(!wrong.success&&world.establishments().size()==1,"Incense Works rejects a site outside region 11");
    const auto duplicate=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::IncenseWorks,1,25});
    check(!duplicate.success&&world.establishments().size()==1&&world.invariant(),"region 11 cannot receive a second Incense source site");

    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
          "authority grants existing Work authorization to a present adult worker");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
          ActivityKind::Work,region,site}).success,"worker submits Work through the shared schedule command");
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==99&&world.establishments()[0].stock[size_t(incense)]==1&&
          world.establishments()[0].stock[size_t(GoodId::Tea)]==0,
          "one completed Work batch consumes one source unit and creates Incense only");
    world.advance(180);
    check(world.establishments()[0].source_units==99,"Incense source does not regenerate as game days pass");

    const std::string saved=world.serialize("incense-works-trial-1");
    check(saved.rfind("POLYEC_REALM_73 ",0)==0&&saved.find(" ESTABLISH_14 1 ")!=std::string::npos,
          "save uses schema 41 and the new establishment section");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(saved,"incense-works-trial-1",error),error.c_str());
    check(resumed.fingerprint()==world.fingerprint()&&resumed.establishments()[0].source_units==99&&
          resumed.establishments()[0].stock[size_t(incense)]==1,"save/load preserves Incense source and output without replay");
    const auto wrongGood=world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
        site,EstablishmentTransferOperation::WithdrawGoods,int(GoodId::Tea),1});
    check(!wrongGood.success&&world.invariant(),"output ledger does not expose Tea as an Incense substitute");
    const auto withdrawal=world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
        site,EstablishmentTransferOperation::WithdrawGoods,incense,1});
    check(withdrawal.success&&world.view(governor).provinces[size_t(region)].stock[size_t(incense)]==1,
          "owner withdraws Incense into the regional market stock");
    check(world.execute({buyer,world.person(buyer).decisions+1,{Intent::Borrow,region,0,300,180}}).success,
          "buyer uses an existing loan within the 300-unit credit limit to fund a market purchase");
    const auto bought=world.execute({buyer,world.person(buyer).decisions+1,{Intent::Buy,region,incense,1}});
    check(bought.success&&world.person(buyer).inventory[size_t(incense)]==1,
          "buyer purchases canonical Incense through the market command");
    const auto sold=world.execute({buyer,world.person(buyer).decisions+1,{Intent::Sell,region,incense,1}});
    check(sold.success&&world.person(buyer).inventory[size_t(incense)]==0&&world.invariant()&&resumed.invariant(),
          "selling Incense preserves the Realm and trade invariants");

    std::string old=test_fixtures::downgrade_organization_funds_44_to_43(saved);
    old.replace(0,std::string("POLYEC_REALM_43").size(),"POLYEC_REALM_40");
    const auto tag=old.find("ESTABLISH_13");old.replace(tag,std::string("ESTABLISH_13").size(),"ESTABLISH_10");
    Realm oldReader(1,1,128);
    check(!oldReader.deserialize(old,"incense-works-trial-1",error),"schema 40 rejects the newer Incense industry record");
    std::cout<<"Incense Works checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"Incense Works failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}
