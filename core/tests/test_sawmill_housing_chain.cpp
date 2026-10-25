#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>

using namespace polyec;
namespace polyec {
struct RealmObservationTestAccess {
    static int64_t exhaust_market_tools(Realm& world,int region){
        auto& stock=world.provinces_[size_t(region)].stock[size_t(GoodId::Tools)];
        const int64_t exhausted=stock;
        if(exhausted>std::numeric_limits<int64_t>::max()-world.consumed_[size_t(GoodId::Tools)])
            throw std::runtime_error("opening Tools consumption ledger overflow");
        stock=0;
        world.consumed_[size_t(GoodId::Tools)]+=exhausted;
        return exhausted;
    }
    static void create_housing_shortage(Realm& world,int region){
        world.urban_housing_capacity_[size_t(region)]=0;
        world.refresh_demography();
    }
    static int64_t consumed(const Realm& world,GoodId good){return world.consumed_[size_t(good)];}
};
}
namespace {
int checks=0;
void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t find_person(const Realm& world,Family family,uint64_t except=0,int region=6){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.alive&&person.destination==-1&&person.region==region&&person.family==family&&
           person.age_days>=14*365&&id!=except)return id;
    }
    return 0;
}
uint64_t find_urban_adult(const Realm& world,int region,uint64_t except=0){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.alive&&person.destination==-1&&person.region==region&&
           person.residence_kind==ResidenceKind::Urban&&person.age_days>=18*365&&id!=except)return id;
    }
    return 0;
}
void complete_work(Realm& world,uint64_t governor,uint64_t site,uint64_t worker){
    auto authorization=world.execute(WorkAuthorizationCommand{governor,
        world.person(governor).decisions+1,site,worker,WorkAuthorizationOperation::Set,
        world.game_tick(),world.game_tick()+64});
    if(!authorization.success)throw std::runtime_error("work authorization failed: "+authorization.reason);
    auto schedule=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+64,0,1000,
        ActivityKind::Work,world.person(worker).region,site});
    if(!schedule.success)throw std::runtime_error("Work schedule failed: "+schedule.reason);
    world.advance_ticks(64);
    if(!world.invariant())throw std::runtime_error("Realm invariant failed after one Work period");
}
}

int main(){try{
    constexpr int region=6;
    constexpr auto timber=GoodId::Timber;
    constexpr auto construction=GoodId::ConstructionMaterial;
    constexpr auto iron=GoodId::Iron;
    constexpr auto copper=GoodId::Copper;
    constexpr auto fuel=GoodId::Fuel;
    constexpr auto tools=GoodId::Tools;
    constexpr uint64_t seed=20261024;
    Realm world(seed,26,256);
    const int64_t exhaustedOpeningTools=RealmObservationTestAccess::exhaust_market_tools(world,region);
    check(exhaustedOpeningTools>0&&world.invariant(),
          "the scarcity scenario spends opening Tools through the goods ledger without creating replacements");
    const uint64_t governor=region+2;
    const uint64_t forest_worker=find_person(world,Family::Civilian);
    const uint64_t sawmill_worker=find_person(world,Family::Civilian,forest_worker);
    const uint64_t tool_worker=find_person(world,Family::Civilian,sawmill_worker);
    const uint64_t merchant=find_person(world,Family::Economic);
    const uint64_t mine_governor=9, mine_worker=find_person(world,Family::Civilian,0,7);
    const uint64_t copper_merchant=[&]{
        for(uint64_t id=1;id<=uint64_t(world.count());++id){
            const auto& person=world.person(id);
            if(person.alive&&person.region==7&&person.family==Family::Economic)return id;
        }
        return uint64_t(0);
    }();
    check(world.person(governor).taxing_authority&&world.person(mine_governor).taxing_authority&&
          forest_worker&&sawmill_worker&&tool_worker&&merchant&&copper_merchant&&
          world.view(governor).provinces[size_t(region)].stock[size_t(iron)]>=2&&
          world.view(governor).provinces[size_t(region)].stock[size_t(fuel)]>=2,
          "Atlas opening has local authorities, workers, a merchant and the documented legacy Iron/Fuel stocks");

    const auto forest=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::Forestry,1,25});
    check(forest.success,"Forestry can be established as the real Timber source");
    if(!forest.success)return 1;
    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,forest.session,
        forest_worker,WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+64}).success,
        "authority grants Work at the Forestry site");
    check(world.execute(ScheduleCommand{forest_worker,world.person(forest_worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+64,0,1000,
        ActivityKind::Work,region,forest.session}).success,"worker submits normal Forestry Work");
    world.advance_ticks(64);
    check(world.establishments().front().stock[size_t(timber)]==1,
          "completed Forestry Work creates one Timber from a bounded source unit");
    check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
        forest.session,EstablishmentTransferOperation::WithdrawGoods,int(timber),1}).success,
        "public authority moves the produced Timber into regional market stock");

    const auto sawmill=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::SawmillWorks,1,25});
    check(sawmill.success,"the unique region 6 Sawmill accepts its versioned trial recipe");
    if(!sawmill.success)return 1;
    for(int unit=0;unit<4;++unit){
        complete_work(world,governor,forest.session,forest_worker);
        check(world.establishments().front().stock[size_t(timber)]>=1,
              "Forestry Work produces a real Timber unit from its bounded source");
        check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
            forest.session,EstablishmentTransferOperation::WithdrawGoods,int(timber),1}).success,
            "public Forestry owner withdraws produced Timber through the goods-transfer command");
        check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
            sawmill.session,EstablishmentTransferOperation::DepositGoods,int(timber),1}).success,
            "public Sawmill receives that same Timber through the goods-transfer command");
    }
    check(world.establishments()[size_t(1)].stock[size_t(timber)]==4,
          "four Forestry outputs are present at the Sawmill before conversion");
    for(int unit=0;unit<4;++unit)complete_work(world,governor,sawmill.session,sawmill_worker);
    check(world.establishments()[size_t(1)].stock[size_t(timber)]==0&&
          world.establishments()[size_t(1)].stock[size_t(construction)]==4,
          "four Sawmill Work completions convert exactly four Timber into four ConstructionMaterial");
    check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
        sawmill.session,EstablishmentTransferOperation::WithdrawGoods,int(construction),4}).success,
        "the authority transfers all four real building materials into the regional market stock");

    const auto mine=world.execute(EstablishmentCommand{mine_governor,world.person(mine_governor).decisions+1,
        EstablishmentOwnerKind::Public,7,7,EstablishmentIndustry::Mining,1,25});
    check(mine.success&&world.establishments().back().industry==EstablishmentIndustry::Mining&&
          world.establishments().back().source_good==int(copper),
          "region 7 authority creates the bounded Copper mine using the Atlas industry command");
    for(int unit=0;unit<2;++unit)complete_work(world,mine_governor,mine.session,mine_worker);
    check(world.establishments().back().stock[size_t(copper)]==2&&
          world.establishments().back().source_units==98,
          "two mine Work periods produce two Copper from the finite ore source");
    check(world.execute(EstablishmentTransferCommand{mine_governor,world.person(mine_governor).decisions+1,
        mine.session,EstablishmentTransferOperation::WithdrawGoods,int(copper),2}).success&&
          world.view(mine_governor).provinces[7].stock[size_t(copper)]==2,
          "the mine authority places produced Copper into region 7 market stock");
    check(world.execute(RealmCommand{copper_merchant,world.person(copper_merchant).decisions+1,
        {Intent::Buy,7,int(copper),2}}).success&&
          world.person(copper_merchant).inventory[size_t(copper)]==2,
          "a regional merchant buys the Copper from the actual mine output");
    const auto copperShipment=world.execute(TransportShipmentCommand{copper_merchant,
        world.person(copper_merchant).decisions+1,0,TransportShipmentOperation::Create,
        region,int(copper),2,TransportServiceGoal::Fastest});
    check(copperShipment.success&&world.person(copper_merchant).inventory[size_t(copper)]==0,
          "the merchant pays to ship both Copper units from region 7 to the ToolWorkshop region");
    const auto copperTravelOptions=world.candidates(copper_merchant);
    const auto travelToMill=std::find_if(copperTravelOptions.begin(),copperTravelOptions.end(),[](const Candidate& action){
        return action.kind==Intent::Travel&&action.target==region;
    });
    check(travelToMill!=copperTravelOptions.end()&&
          world.mask(copper_merchant,copperTravelOptions)[size_t(travelToMill-copperTravelOptions.begin())]>0,
          "merchant can travel to region 6 while the paid Copper shipment is in transit");
    const auto copperTravel=world.execute({copper_merchant,world.person(copper_merchant).decisions+1,*travelToMill});
    check(copperTravel.success,"merchant travels to the receiving market using an open route");
    for(int day=0;day<90;++day){
        const auto shipments=world.transport_shipments();
        const auto shipped=std::find_if(shipments.begin(),shipments.end(),[&](const TransportShipment& item){return item.id==copperShipment.session;});
        if(world.person(copper_merchant).region==region&&shipped!=shipments.end()&&shipped->delivered==2)break;
        world.advance(1);
    }
    check(world.person(copper_merchant).region==region&&
          world.person(copper_merchant).inventory[size_t(copper)]==2&&
          world.transport_shipments().back().delivered==2,
          "the route delivers both Copper units to the merchant in region 6");
    check(world.execute(RealmCommand{copper_merchant,world.person(copper_merchant).decisions+1,
        {Intent::Sell,region,int(copper),2}}).success&&
          world.view(copper_merchant).provinces[size_t(region)].stock[size_t(copper)]==2,
          "merchant sells the delivered Copper into the ToolWorkshop's local supply market");

    const auto toolWorkshop=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,region,region,EstablishmentIndustry::ToolWorkshop,1,25});
    check(toolWorkshop.success,"the local authority creates a ToolWorkshop through the establishment command");
    for(const auto good:{iron,copper,fuel})
        check(world.execute(EstablishmentTransferCommand{governor,world.person(governor).decisions+1,
            toolWorkshop.session,EstablishmentTransferOperation::DepositGoods,int(good),2}).success,
            "ToolWorkshop receives two units of each required Iron, Copper and Fuel input");
    check(world.establishments().back().stock[size_t(iron)]==2&&
          world.establishments().back().stock[size_t(copper)]==2&&
          world.establishments().back().stock[size_t(fuel)]==2,
          "ToolWorkshop input inventory matches its three-good recipe before Work");
    for(int unit=0;unit<2;++unit)complete_work(world,governor,toolWorkshop.session,tool_worker);
    check(world.establishments().back().stock[size_t(iron)]==0&&
          world.establishments().back().stock[size_t(copper)]==0&&
          world.establishments().back().stock[size_t(fuel)]==0&&
          world.establishments().back().stock[size_t(tools)]==2,
          "two ToolWorkshop Work periods turn the three real inputs into two Tools");
    const auto withdrawTools=world.execute(EstablishmentTransferCommand{governor,
        world.person(governor).decisions+1,toolWorkshop.session,
        EstablishmentTransferOperation::WithdrawGoods,int(tools),2});
    if(!withdrawTools.success)throw std::runtime_error("ToolWorkshop output transfer rejected: "+withdrawTools.reason);
    check(world.view(governor).provinces[size_t(region)].stock[size_t(tools)]==2,
          "the public owner places both produced Tools into an otherwise empty local market");

    const uint64_t nominee=find_urban_adult(world,region);
    const uint64_t sovereign=1;
    check(nominee&&world.person(sovereign).sovereign,
          "region 6 has an adult urban resident who may voluntarily seek the council seat");
    RealmObservationTestAccess::create_housing_shortage(world,region);
    check(world.urban_census()[size_t(region)].unhoused_residents>0,
          "the explicit test scenario has residents without public beds before the housing response");
    const auto nomination=world.candidates(nominee)[size_t(realm_council_nomination_candidate)];
    check(nomination.kind==Intent::NominateCouncil&&world.mask(nominee)[size_t(realm_council_nomination_candidate)]==1.f,
          "the resident receives the real self-nomination choice");
    check(world.execute({nominee,world.person(nominee).decisions+1,nomination}).success,
          "resident voluntarily nominates themselves for their own settlement");
    const int appointmentSlot=realm_council_accept_candidate_begin+region;
    const auto appointment=world.candidates(sovereign)[size_t(appointmentSlot)];
    check(appointment.kind==Intent::ResolveCouncilNomination&&world.mask(sovereign)[size_t(appointmentSlot)]==1.f,
          "the Sovereign receives the settlement-specific appointment choice");
    check(world.execute({sovereign,world.person(sovereign).decisions+1,appointment}).success&&
          world.municipal_councils()[size_t(region)].representative==nominee,
          "Sovereign appoints the volunteer under the existing council rule");

    const size_t grantSlot=size_t(realm_municipal_fund_candidate_begin+3);
    const auto grant=world.candidates(governor)[grantSlot];
    check(grant.kind==Intent::FundMunicipal&&world.mask(governor)[grantSlot]==1.f&&
          grant.amount>=kUrbanHousingProjectBudget,
          "the local authority can fund the city account with the existing five-percent grant command");
    check(world.execute({governor,world.person(governor).decisions+1,grant}).success&&
          world.municipal_funds()[size_t(region)]>=kUrbanHousingProjectBudget,
          "the housing fund receives real treasury money through FundMunicipal");
    const auto localStock=world.view(governor).provinces[size_t(region)].stock;
    check(localStock[size_t(construction)]==4&&localStock[size_t(tools)]==2,
          "regional market holds only four new Sawmill materials and two new Tools needed by housing");
    const auto housing=world.candidates(nominee)[281];
    check(housing.kind==Intent::Project&&housing.good==4&&world.mask(nominee)[281]==1.f,
          "appointed representative may start the housing project at their settlement");
    const uint64_t outsider=find_person(world,Family::Civilian,nominee);
    if(outsider){
        const auto beforeUnauthorized=world.fingerprint();
        const auto unauthorized=world.execute({outsider,world.person(outsider).decisions+1,housing});
        check(!unauthorized.success&&world.fingerprint()==beforeUnauthorized,
              "a different civilian cannot spend council funds or consume project materials");
    }
    check(world.execute({nominee,world.person(nominee).decisions+1,housing}).success,
          "appointed representative starts public housing using the produced material and municipal fund");
    if(world.municipal_funds()[size_t(region)]!=grant.amount-kUrbanHousingProjectBudget||
       world.view(governor).provinces[size_t(region)].stock[size_t(construction)]!=0||
       world.view(governor).provinces[size_t(region)].stock[size_t(tools)]!=0||
       RealmObservationTestAccess::consumed(world,construction)!=4||
       RealmObservationTestAccess::consumed(world,tools)!=exhaustedOpeningTools+2){
        std::ostringstream detail;
        detail<<"housing debit mismatch: grant="<<grant.amount
              <<" municipal="<<world.municipal_funds()[size_t(region)]
              <<" construction stock="<<world.view(governor).provinces[size_t(region)].stock[size_t(construction)]
              <<" tools stock="<<world.view(governor).provinces[size_t(region)].stock[size_t(tools)]
              <<" consumed construction="<<RealmObservationTestAccess::consumed(world,construction)
              <<" consumed tools="<<RealmObservationTestAccess::consumed(world,tools);
        throw std::runtime_error(detail.str());
    }
    check(world.municipal_funds()[size_t(region)]==grant.amount-kUrbanHousingProjectBudget&&
          world.view(governor).provinces[size_t(region)].stock[size_t(construction)]==0&&
          world.view(governor).provinces[size_t(region)].stock[size_t(tools)]==0&&
          RealmObservationTestAccess::consumed(world,construction)==4&&
          RealmObservationTestAccess::consumed(world,tools)==exhaustedOpeningTools+2&&world.invariant(),
          "housing debits exactly four ConstructionMaterial, two Tools and its budget once");

    const auto save=world.serialize("sawmill-housing-trial-1");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(save,"sawmill-housing-trial-1",error),error.c_str());
    check(resumed.fingerprint()==world.fingerprint()&&resumed.projects().back().due==world.projects().back().due&&
          resumed.municipal_councils()[size_t(region)].representative==nominee&&resumed.invariant(),
          "save/load keeps the appointed builder, public project deadline and consumed-material ledgers");
    world.advance(kUrbanHousingProjectTermDays);
    resumed.advance(kUrbanHousingProjectTermDays);
    check(world.fingerprint()==resumed.fingerprint()&&world.projects().back().completed&&
          world.urban_census()[size_t(region)].housing_capacity==kUrbanHousingBedsPerBlock&&
          world.invariant()&&resumed.invariant(),
          "the live and resumed worlds complete the eight-bed public housing project on the same day");
    check(checks>=40,"Sawmill, Tools, council and housing chain includes all expected checks");
    std::cout<<"housing integration seed="<<seed<<" opening Tools spent="<<exhaustedOpeningTools
             <<"; grant="<<grant.amount<<"; representative="<<nominee
             <<"; final day="<<world.day()<<'\n';
    std::cout<<"SAWMILL HOUSING CHAIN: "<<checks<<" checks passed\n";
    return 0;
}catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';return 1;}}
