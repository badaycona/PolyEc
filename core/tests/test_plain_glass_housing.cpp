#include "polyec/realm.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmPrivateHousingTestAccess {
    static int64_t consumed_plain_glass(const Realm&world){return world.consumed_[size_t(GoodId::PlainGlass)];}
    static void prepare_owner(Realm&world,uint64_t actor){
        auto&owner=world.mutable_person(actor);
        owner.family=Family::Civilian;owner.sovereign=false;owner.taxing_authority=false;
        owner.work_assignment={};owner.office_appointment={};owner.destination=-1;owner.busy_until=0;
        owner.region=10;owner.residence_kind=ResidenceKind::Urban;owner.urban_site_id=10;
        owner.cash+=500;world.opening_money_+=500;
        owner.inventory[size_t(GoodId::ConstructionMaterial)]=2;
        owner.inventory[size_t(GoodId::Tools)]=1;
        world.opening_[size_t(GoodId::ConstructionMaterial)]+=2;
        world.opening_[size_t(GoodId::Tools)]+=1;
        world.land_holdings_[0].title_holder=actor;world.land_holdings_[0].region=10;
        world.land_holdings_[0].area_units=1;
        for(auto&member:world.people_)if(member.household_membership.household_id==owner.household_membership.household_id){
            member.region=10;member.destination=-1;member.residence_kind=ResidenceKind::Urban;
            member.urban_site_id=10;
        }
        world.refresh_demography();
    }
};
}

namespace {
int checks=0;
void check(bool ok,const std::string&message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t find_adult(const Realm&world,int region,uint64_t exclude=0){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto&person=world.person(id);
        if(id!=exclude&&person.alive&&person.region==region&&person.family==Family::Civilian&&
           person.age_days>=14*365&&person.destination==-1)return id;
    }
    return 0;
}
}

int main(){try{
    constexpr int region=10;
    constexpr auto glass=GoodId::PlainGlass;
    Realm world(20261061,26,256);
    const uint64_t authority=region+2;
    const uint64_t worker=find_adult(world,region);
    check(world.model_contract()=="realm-household-fund-goods30-config-v46"&&
          realm_actions==400&&realm_obs==5182,
          "the changed house-material rule is represented by model contract v44 without changing tensor shape");
    check(worker!=0&&world.person(authority).taxing_authority,
          "the glass region has its public authority and a separate adult worker");

    const Candidate create=world.candidates(authority)[264];
    check(create.kind==Intent::CreateEstablishment&&create.good==int(EstablishmentIndustry::Glassworks)&&
          world.mask(authority)[264]==1.f,
          "the authority receives the existing Glassworks creation choice");
    const auto created=world.execute({authority,world.person(authority).decisions+1,create});
    check(created.success,"the authority creates the region's Glassworks through the ordinary command");
    const uint64_t site=created.session;
    check(world.establishments().back().source_good==int(glass)&&
          world.establishments().back().source_units==100,
          "Glassworks records its finite PlainGlass source separately from product stock");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,
          site,EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1}).success,
          "the Glassworks receives one actual Fuel from the regional market");
    check(world.execute(WorkAuthorizationCommand{authority,world.person(authority).decisions+1,site,worker,
          WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
          "the authority authorizes the worker for the next Work period");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
          ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
          ActivityKind::Work,region,site}).success,
          "the worker schedules ordinary Work at the Glassworks");
    world.advance_ticks(64);
    check(world.establishments().back().stock[size_t(glass)]==1&&
          world.establishments().back().stock[size_t(GoodId::Fuel)]==0&&
          world.establishments().back().source_units==99&&world.invariant(),
          "one real Work batch consumes Fuel and a source unit to make exactly one PlainGlass");
    check(world.execute(EstablishmentTransferCommand{authority,world.person(authority).decisions+1,
          site,EstablishmentTransferOperation::WithdrawGoods,int(glass),1}).success&&
          world.view(authority).provinces[region].stock[size_t(glass)]==1,
          "the authority transfers the finished pane to the regional market");

    const uint64_t owner=worker;
    RealmPrivateHousingTestAccess::prepare_owner(world,owner);
    check(world.invariant(),"the test owner has a valid urban title, ordinary build materials and personal funds");
    const int64_t price=world.view(owner).provinces[region].price[size_t(glass)];
    const auto order=world.execute(MarketOrderCommand{owner,world.person(owner).decisions+1,0,
        MarketOrderOperation::Place,region,int(glass),1,price});
    check(order.success&&world.person(owner).inventory[size_t(glass)]==0&&world.market_orders().size()==1,
          "the title holder buys one pane through a normal market order");
    world.advance_ticks(kSubdayUnitsPerDay);
    check(world.person(owner).inventory[size_t(glass)]==1&&world.invariant(),
          "market settlement moves the exact pane into the owner's personal inventory");

    const auto beforeConsumed=RealmPrivateHousingTestAccess::consumed_plain_glass(world);
    const auto candidates=world.candidates(owner);
    const auto build=std::find_if(candidates.begin()+realm_private_build_candidate_begin,
        candidates.begin()+realm_private_offer_candidate_begin,
        [](const Candidate&candidate){return candidate.kind==Intent::PrivateResidenceBuild;});
    check(build!=candidates.begin()+realm_private_offer_candidate_begin&&
          world.mask(owner)[size_t(build-candidates.begin())]==1.f,
          "the existing learned-policy build choice is enabled after Glassworks output is personally acquired");
    const auto built=world.execute({owner,world.person(owner).decisions+1,*build});
    check(built.success&&world.private_residences().size()==1&&
          world.person(owner).inventory[size_t(glass)]==0&&
          RealmPrivateHousingTestAccess::consumed_plain_glass(world)==beforeConsumed+1&&world.invariant(),
          "building the private home consumes its owner's pane exactly once in the same transaction");

    const std::string saved=world.serialize("plain-glass-housing-use-v1");
    check(saved.rfind("POLYEC_REALM_68 ",0)==0&&saved.find(" PRIVATE_HOUSING_1 1 ")!=std::string::npos,
          "the new material requirement does not add a save field or change schema68");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(saved,"plain-glass-housing-use-v1",error),
          "the world with a home under construction loads: "+error);
    check(restored.fingerprint()==world.fingerprint()&&restored.invariant()&&
          RealmPrivateHousingTestAccess::consumed_plain_glass(restored)==beforeConsumed+1,
          "save/load preserves the consumed glass ledger and unfinished house");
    restored.advance(kPrivateResidenceBuildDays);
    check(restored.private_residences().front().completed&&restored.invariant(),
          "the glazed house completes on the existing thirty-day schedule");
    std::cout<<"PlainGlass housing checks="<<checks<<'\n';return 0;
}catch(const std::exception&error){std::cerr<<"PlainGlass housing failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}
