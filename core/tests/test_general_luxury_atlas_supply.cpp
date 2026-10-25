#include "polyec/realm.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmGeneralLuxuryTestAccess {
    static void set_cash(Realm& world,uint64_t actor,int64_t value){
        auto& person=world.mutable_person(actor);world.opening_money_+=value-person.cash;person.cash=value;
    }
    static void seed_region_good(Realm& world,int region,GoodId good,int64_t amount){
        world.provinces_[size_t(region)].stock[size_t(good)]+=amount;
        world.opening_[size_t(good)]+=amount;
    }
};
}

namespace {
int checks=0;
void check(bool ok,const std::string&message){++checks;if(!ok)throw std::runtime_error(message);}
uint64_t person_at(const Realm&world,int region,Family family,bool adult=false,uint64_t exclude=0){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto&p=world.person(id);
        if(id!=exclude&&p.alive&&p.region==region&&p.destination==-1&&p.family==family&&
           (!adult||p.age_days>=18*365))return id;
    }
    return 0;
}
uint64_t create_public(Realm&world,int region,EstablishmentIndustry industry){
    const uint64_t governor=uint64_t(region+2);
    const auto result=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),region,industry,1,25});
    check(result.success,"regional authority creates the planned public producer/workshop");
    return result.session;
}
bool transfer(Realm&world,uint64_t manager,uint64_t site,EstablishmentTransferOperation operation,
              GoodId good,int amount=1){
    return world.execute(EstablishmentTransferCommand{manager,world.person(manager).decisions+1,
        site,operation,int(good),amount}).success;
}
void produce_one(Realm&world,uint64_t governor,uint64_t worker,uint64_t site,int region,int64_t duration=32){
    check(world.execute(WorkAuthorizationCommand{governor,world.person(governor).decisions+1,site,worker,
        WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+duration}).success,
        "regional owner authorizes one normal production shift");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+duration,0,1000,
        ActivityKind::Work,region,site}).success,
        "worker schedules production through the shared command pipeline");
    world.advance_ticks(kSubdayUnitsPerDay);
}
Candidate action(const Realm&world,uint64_t actor,Intent kind,int target,int good){
    const auto candidates=world.candidates(actor);const auto mask=world.mask(actor,candidates);
    for(size_t i=0;i<candidates.size();++i)
        if(candidates[i].kind==kind&&candidates[i].target==target&&candidates[i].good==good&&mask[i]>.5f)
            return candidates[i];
    return {};
}
Candidate travel_action(const Realm&world,uint64_t actor,int target){
    const auto candidates=world.candidates(actor);const auto mask=world.mask(actor,candidates);
    for(size_t i=0;i<candidates.size();++i)
        if(candidates[i].kind==Intent::Travel&&candidates[i].target==target&&mask[i]>.5f)
            return candidates[i];
    return {};
}
uint8_t route_to_close(const Realm&world,int from,int to){
    for(size_t i=0;i<world.national_transport_routes().size();++i){
        const auto&route=world.national_transport_routes()[i];
        if(route.from==from&&route.to==to&&route.open)return uint8_t(i);
    }
    throw std::runtime_error("expected directed Atlas route exists and is open");
}
void ship_to_region12(Realm&world,uint64_t merchant,GoodId good,int sourceRegion,
                      size_t expectedLegs,int64_t expectedFee,int expectedTransitDays,int blockedEdgeFrom=-1){
    const auto bought=world.execute(RealmCommand{merchant,world.person(merchant).decisions+1,
        {Intent::Buy,sourceRegion,int(good),1}});
    check(bought.success,"merchant buys the produced good from its source region");
    if(blockedEdgeFrom>=0){
        const auto routeIndex=route_to_close(world,blockedEdgeFrom,blockedEdgeFrom-1);
        const auto closed=world.execute(TransportRouteCommand{1,world.person(1).decisions+1,routeIndex,false});
        check(closed.success,"sovereign closes the selected directed Atlas route for the paired shortage branch");
    }
    const auto before=world.fingerprint();
    if(blockedEdgeFrom>=0){
        const auto rejected=world.execute(TransportShipmentCommand{merchant,world.person(merchant).decisions+1,0,
            TransportShipmentOperation::Create,12,int(good),1,TransportServiceGoal::Fastest});
        check(!rejected.success&&world.fingerprint()==before&&world.person(merchant).inventory[size_t(good)]==1,
              "a closed mandatory link rejects shipment before changing cargo, cash or the Realm");
        return;
    }
    const int64_t cashBeforeFee=world.person(merchant).cash;
    const auto paidShipment=world.execute(TransportShipmentCommand{merchant,world.person(merchant).decisions+1,0,
        TransportShipmentOperation::Create,12,int(good),1,TransportServiceGoal::Fastest});
    check(paidShipment.success,"merchant creates a real multi-link shipment and pays its route fee");
    check(world.transport_shipments().back().route_indices.size()==expectedLegs&&
          world.person(merchant).cash==cashBeforeFee-expectedFee&&
          world.transport_shipments().back().fee_escrow==expectedFee,
          "shipment charges the merchant the route fee and records the matching route length and escrow");
    const std::string pending=world.serialize("general-luxury-atlas-pending-shipment-v1");
    std::string loadError;
    check(world.deserialize(pending,"general-luxury-atlas-pending-shipment-v1",loadError)&&
          world.invariant()&&world.transport_shipments().back().delivered==0&&
          world.transport_shipments().back().fee_escrow==expectedFee,
          "save/load preserves a shipment while its cargo is still travelling: "+loadError);
    const Candidate travel=travel_action(world,merchant,12);
    check(travel.kind==Intent::Travel,"merchant receives a legal travel action to the destination region");
    check(world.execute({merchant,world.person(merchant).decisions+1,travel}).success,
          "merchant travels to the shipment destination through the Atlas route");
    const int departureDay=world.day();
    while(world.transport_shipments().back().delivered==0&&world.day()-departureDay<20)world.advance(1);
    const int actualTransitDays=world.day()-departureDay;
    check(actualTransitDays==expectedTransitDays,"shipment completes on the expected Atlas route schedule");
    check(world.day()==departureDay+expectedTransitDays&&world.person(merchant).region==12&&
          world.person(merchant).destination==-1&&
          world.transport_shipments().back().delivered==1&&world.person(merchant).inventory[size_t(good)]==1,
          "shipment arrives to its owner while travel completes");
    const auto sold=world.execute(RealmCommand{merchant,world.person(merchant).decisions+1,
        {Intent::Sell,12,int(good),1}});
    check(sold.success,"merchant sells the delivered input into region12 stock");
    check(world.invariant(),"source-region shipment and destination sale preserve all account invariants");
}
}

int main(){try{
    constexpr int teaRegion=14,incenseRegion=11,workshopRegion=12;
    constexpr auto tea=GoodId::Tea,incense=GoodId::Incense,silk=GoodId::Silk,luxury=GoodId::GeneralLuxury;
    Realm supplied(20261121,26,512);
    const uint64_t teaGovernor=teaRegion+2,incenseGovernor=incenseRegion+2,luxuryGovernor=workshopRegion+2;
    const uint64_t teaWorker=person_at(supplied,teaRegion,Family::Civilian,true);
    const uint64_t incenseWorker=person_at(supplied,incenseRegion,Family::Civilian,true);
    const uint64_t silkWorker=person_at(supplied,workshopRegion,Family::Civilian,true);
    const uint64_t teaMerchant=person_at(supplied,teaRegion,Family::Economic,true);
    const uint64_t incenseMerchant=person_at(supplied,incenseRegion,Family::Economic,true);
    const uint64_t buyer=person_at(supplied,workshopRegion,Family::Civilian,true);
    check(teaWorker&&incenseWorker&&silkWorker&&teaMerchant&&incenseMerchant&&buyer,
          "fixed Atlas seed provides real workers, source-region merchants and a local buyer");
    RealmGeneralLuxuryTestAccess::set_cash(supplied,buyer,500);
    const uint64_t teaGarden=create_public(supplied,teaRegion,EstablishmentIndustry::TeaGarden);
    const uint64_t incenseWorks=create_public(supplied,incenseRegion,EstablishmentIndustry::IncenseWorks);
    const uint64_t silkFarm=create_public(supplied,workshopRegion,EstablishmentIndustry::SilkwormFarm);
    const uint64_t atelier=create_public(supplied,workshopRegion,EstablishmentIndustry::GeneralLuxuryAtelier);
    produce_one(supplied,teaGovernor,teaWorker,teaGarden,teaRegion);
    produce_one(supplied,incenseGovernor,incenseWorker,incenseWorks,incenseRegion);
    produce_one(supplied,luxuryGovernor,silkWorker,silkFarm,workshopRegion);
    check(supplied.establishments()[0].stock[size_t(tea)]==1&&supplied.establishments()[0].source_units==24&&
          supplied.establishments()[1].stock[size_t(incense)]==1&&supplied.establishments()[1].source_units==99&&
          supplied.establishments()[2].stock[size_t(silk)]==1&&supplied.establishments()[2].source_units==24,
          "normal Work creates all three inputs and consumes one finite source unit at each producer");
    check(transfer(supplied,teaGovernor,teaGarden,EstablishmentTransferOperation::WithdrawGoods,tea)&&
          transfer(supplied,incenseGovernor,incenseWorks,EstablishmentTransferOperation::WithdrawGoods,incense)&&
          transfer(supplied,luxuryGovernor,silkFarm,EstablishmentTransferOperation::WithdrawGoods,silk),
          "each producer moves its output into its own region market stock");
    ship_to_region12(supplied,teaMerchant,tea,teaRegion,2,2,7);
    ship_to_region12(supplied,incenseMerchant,incense,incenseRegion,6,6,19);
    check(supplied.view(luxuryGovernor).provinces[size_t(workshopRegion)].stock[size_t(tea)]==1&&
          supplied.view(luxuryGovernor).provinces[size_t(workshopRegion)].stock[size_t(incense)]==1,
          "Tea and Incense have crossed from their producing regions and entered region12 market stock");
    check(supplied.establishments()[2].stock[size_t(silk)]==0&&
          supplied.view(luxuryGovernor).provinces[size_t(workshopRegion)].stock[size_t(silk)]==1,
          "Silk has left its source farm and is present once in the local market stock");
    // Silk was withdrawn into region12 stock above. Put all three purchased/produced goods into the Atelier.
    check(transfer(supplied,luxuryGovernor,atelier,EstablishmentTransferOperation::DepositGoods,tea)&&
          transfer(supplied,luxuryGovernor,atelier,EstablishmentTransferOperation::DepositGoods,incense)&&
          transfer(supplied,luxuryGovernor,atelier,EstablishmentTransferOperation::DepositGoods,silk),
          "region12 manager moves the three actual market goods into the atelier");
    check(supplied.establishments().back().stock[size_t(tea)]==1&&
          supplied.establishments().back().stock[size_t(incense)]==1&&
          supplied.establishments().back().stock[size_t(silk)]==1,
          "Atelier receives the exact recipe inputs after long-distance transport");
    const std::string readyToCraft=supplied.serialize("general-luxury-atlas-ready-to-craft-v1");
    std::string readyError;
    check(supplied.deserialize(readyToCraft,"general-luxury-atlas-ready-to-craft-v1",readyError)&&
          supplied.invariant()&&supplied.transport_shipments().size()==2&&
          supplied.transport_shipments()[0].delivered==1&&supplied.transport_shipments()[1].delivered==1&&
          supplied.establishments().back().stock[size_t(tea)]==1&&
          supplied.establishments().back().stock[size_t(incense)]==1&&
          supplied.establishments().back().stock[size_t(silk)]==1,
          "save/load immediately before crafting preserves both delivered shipments and exact recipe inputs: "+readyError);
    int atelierWorkShifts=0;
    while(supplied.establishments().back().stock[size_t(luxury)]==0&&atelierWorkShifts<5){
        produce_one(supplied,luxuryGovernor,silkWorker,atelier,workshopRegion,48);
        ++atelierWorkShifts;
    }
    check(atelierWorkShifts==3,
          "the fixed trial worker accumulates three production units across three normal 48-tick Work sessions");
    check(supplied.establishments().back().stock[size_t(luxury)]==3&&
          supplied.establishments().back().stock[size_t(tea)]==0&&
          supplied.establishments().back().stock[size_t(incense)]==0&&
          supplied.establishments().back().stock[size_t(silk)]==0&&supplied.invariant(),
          "normal Work sessions accumulate enough output to consume all three inputs atomically and create three luxuries");
    check(transfer(supplied,luxuryGovernor,atelier,EstablishmentTransferOperation::WithdrawGoods,luxury,3),
          "finished luxuries move into region12 market stock");
    const auto purchase=supplied.execute(RealmCommand{buyer,supplied.person(buyer).decisions+1,
        {Intent::Buy,workshopRegion,int(luxury),1}});
    check(purchase.success&&supplied.person(buyer).inventory[size_t(luxury)]==1,
          "a civilian buys one product of the integrated multi-region chain");
    const Candidate social=action(supplied,buyer,Intent::Social,workshopRegion,int(luxury));
    const int64_t eventCashBefore=supplied.person(buyer).cash;
    const int64_t localFundBefore=supplied.view(buyer).provinces[size_t(workshopRegion)].firm;
    const int socialBefore=supplied.person(buyer).social;
    check(social.kind==Intent::Social&&supplied.execute({buyer,supplied.person(buyer).decisions+1,social}).success&&
          supplied.person(buyer).cash==eventCashBefore-20&&
          supplied.view(buyer).provinces[size_t(workshopRegion)].firm==localFundBefore+20&&
          supplied.person(buyer).inventory[size_t(luxury)]==0&&socialBefore==0&&
          supplied.person(buyer).social==25&&
          supplied.person(buyer).busy_until==supplied.day()+14,
          "the buyer uses the purchased item through its real social consumer");
    const std::string midway=supplied.serialize("general-luxury-atlas-supply-v1");
    Realm restored(1,1,128);std::string error;
    check(restored.deserialize(midway,"general-luxury-atlas-supply-v1",error)&&restored.invariant()&&
          restored.transport_shipments().size()==2&&restored.apprenticeships().empty(),
          "save/load retains both cross-region shipments and the final production/consumer ledgers");

    Realm blocked(20261121,26,512);
    const uint64_t blockedMerchant=person_at(blocked,incenseRegion,Family::Economic,true);
    const uint64_t blockedGovernor=incenseRegion+2;
    const uint64_t blockedWorker=person_at(blocked,incenseRegion,Family::Civilian,true);
    const uint64_t blockedWorks=create_public(blocked,incenseRegion,EstablishmentIndustry::IncenseWorks);
    produce_one(blocked,blockedGovernor,blockedWorker,blockedWorks,incenseRegion);
    check(transfer(blocked,blockedGovernor,blockedWorks,EstablishmentTransferOperation::WithdrawGoods,incense),
          "shortage branch produces and stocks the route-dependent good before the route is closed");
    ship_to_region12(blocked,blockedMerchant,incense,incenseRegion,6,6,19,11);
    check(blocked.transport_shipments().empty()&&blocked.view(blockedGovernor).provinces[size_t(incenseRegion)].stock[size_t(incense)]==0,
          "closed mandatory edge leaves no pending shipment and no cargo lost from the merchant");
    check(blocked.person(blockedMerchant).inventory[size_t(incense)]==1&&
          blocked.establishments().front().stock[size_t(incense)]==0&&blocked.invariant(),
          "rejected shipment returns neither a partial delivery nor a hidden workshop input");
    const uint64_t localManager=workshopRegion+2;
    const uint64_t localWorker=person_at(blocked,workshopRegion,Family::Civilian,true);
    check(localWorker!=0,"shortage branch has a real local Atelier worker");
    const uint64_t blockedAtelier=create_public(blocked,workshopRegion,EstablishmentIndustry::GeneralLuxuryAtelier);
    RealmGeneralLuxuryTestAccess::seed_region_good(blocked,workshopRegion,tea,1);
    RealmGeneralLuxuryTestAccess::seed_region_good(blocked,workshopRegion,silk,1);
    check(transfer(blocked,localManager,blockedAtelier,EstablishmentTransferOperation::DepositGoods,tea)&&
          transfer(blocked,localManager,blockedAtelier,EstablishmentTransferOperation::DepositGoods,silk),
          "the paired recipe test prepares its two available ingredients in the local store");
    produce_one(blocked,localManager,localWorker,blockedAtelier,workshopRegion,48);
    check(blocked.establishments().back().stock[size_t(luxury)]==0&&
          blocked.establishments().back().stock[size_t(tea)]==1&&
          blocked.establishments().back().stock[size_t(silk)]==1&&blocked.invariant(),
          "Atelier Work cannot create luxury or consume Tea/Silk while the required imported Incense is missing");
    std::cout<<"general_luxury_atlas_supply_checks="<<checks
             <<" atelier_work_shifts="<<atelierWorkShifts<<'\n';
    return 0;
}catch(const std::exception&error){std::cerr<<"FAILED after "<<checks<<" checks: "<<error.what()<<'\n';return 1;}}
