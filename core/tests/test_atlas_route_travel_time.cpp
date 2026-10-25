#include "polyec/realm.hpp"

#include <array>
#include <iostream>
#include <limits>

namespace polyec {
struct RealmObservationTestAccess {
    static void add_warhorse(Realm& world,uint64_t actor){
        ++world.mutable_person(actor).inventory[size_t(GoodId::WarHorse)];
        ++world.opening_[size_t(GoodId::WarHorse)];
    }
    static void provide_route_cash(Realm& world,uint64_t actor,int64_t amount){
        auto& person=world.mutable_person(actor);
        if(person.cash<amount){
            const auto added=amount-person.cash;
            person.cash=amount;
            world.opening_money_+=added;
        }
    }
};
}

namespace {
int failures = 0;
void check(bool ok, const std::string& message) {
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

std::array<std::array<int, polyec::kAtlasTerritoryCount>, polyec::kAtlasTerritoryCount>
expected_days(const std::vector<polyec::NationalTransportRoute>& routes) {
    using namespace polyec;
    std::array<std::array<int, kAtlasTerritoryCount>, kAtlasTerritoryCount> d{};
    for (auto& row : d) row.fill(1000000);
    for (size_t i = 0; i < kAtlasTerritoryCount; ++i) d[i][i] = 0;
    for (const auto& route : routes) if (route.open)
        d[route.from][route.to] = std::min(d[route.from][route.to], route.travel_days);
    for (size_t k = 0; k < kAtlasTerritoryCount; ++k)
        for (size_t i = 0; i < kAtlasTerritoryCount; ++i)
            for (size_t j = 0; j < kAtlasTerritoryCount; ++j)
                d[i][j] = std::min(d[i][j], d[i][k] + d[k][j]);
    return d;
}
}

int main() {
    using namespace polyec;
    Realm world(20261023, 26, 512);
    check(world.travel_days(16,17)==3,
          "the direct northern-border road applies the three-day highland corridor time");
    check(world.travel_days(6,7)==4,
          "the direct western road applies the four-day forest-pass corridor time");
    auto expected = expected_days(world.national_transport_routes());
    for (size_t from = 0; from < kAtlasTerritoryCount; ++from)
        for (size_t to = 0; to < kAtlasTerritoryCount; ++to) {
            const int want = expected[from][to] >= 1000000 ? -1 : expected[from][to];
            check(world.travel_days(int(from), int(to)) == want,
                  "Realm travel time equals an independent shortest-day route calculation");
        }

    const auto& sovereign = world.person(1);
    const int origin = sovereign.region;
    int target = -1;
    for (int region = 0; region < int(kAtlasTerritoryCount); ++region)
        if (region != origin && world.travel_days(origin, region) > 0) { target = region; break; }
    check(target >= 0, "Sovereign has an open travel destination");
    if (target >= 0) {
        const int days = world.travel_days(origin, target);
        const int64_t cash_before = world.person(1).cash;
        const auto moved = world.execute({1, world.person(1).decisions + 1,
                                          {Intent::Migrate, target, 0, 0, 0}});
        check(moved.success, "valid migration uses an open Atlas route");
        check(world.person(1).cash == cash_before - (4 + 2 * days),
              "migration cost uses the selected route's number of travel days");
        check(world.person(1).busy_until == days + 1,
              "migration arrival uses selected route time plus the departure processing day");
        const std::string saved = world.serialize("atlas-route-time-test");
        Realm restored;
        std::string error;
        check(restored.deserialize(saved, "atlas-route-time-test", error), "in-flight migration roundtrip loads");
        check(restored.person(1).destination == target && restored.person(1).busy_until == days + 1,
              "save/load preserves destination and arrival day");
        restored.advance(days + 2);
        check(restored.person(1).region == target && restored.person(1).destination == -1,
              "migrant arrives at the selected destination after route time");
    }

    Realm blocked(20261024, 26, 512);
    const int blocked_origin = blocked.person(1).region;
    std::vector<uint8_t> outgoing;
    for (size_t i = 0; i < blocked.national_transport_routes().size(); ++i)
        if (blocked.national_transport_routes()[i].from == blocked_origin)
            outgoing.push_back(static_cast<uint8_t>(i));
    for (const auto index : outgoing)
        check(blocked.execute(TransportRouteCommand{1, blocked.person(1).decisions + 1, index, false}).success,
              "Sovereign can close each outgoing test route");
    bool all_unreachable = !outgoing.empty();
    for (int region = 0; region < int(kAtlasTerritoryCount); ++region)
        if (region != blocked_origin && blocked.travel_days(blocked_origin, region) >= 0) all_unreachable = false;
    check(all_unreachable, "closed outgoing routes leave no usable path from the origin");
    const auto before_reject = blocked.fingerprint();
    const auto rejected = blocked.execute({1, blocked.person(1).decisions + 1,
                                           {Intent::Migrate, (blocked_origin + 1) % int(kAtlasTerritoryCount), 0, 0, 0}});
    check(!rejected.success, "migration with every outgoing route closed is rejected");
    check(blocked.fingerprint() == before_reject, "closed-route rejection does not partially change the world");

    Realm small(17, 6, 256);
    check(small.travel_days(0, 1) == small.distance(0, 1), "small profile retains existing hop-based travel behavior");
    const uint64_t smallEconomic=uint64_t(small.regions()+2);
    RealmObservationTestAccess::add_warhorse(small,smallEconomic);
    const auto smallMounted=small.candidates(smallEconomic);
    check(smallMounted[15].kind==Intent::Travel&&smallMounted[15].target==0&&
          smallMounted[15].good==0&&smallMounted[15].term==0,
          "WarHorse does not change small-profile Travel candidates");

    Realm mounted(20261025,26,512);
    uint64_t traveler=0;
    for(uint64_t id=1;id<=512;++id){
        const auto& person=mounted.person(id);
        if(person.alive&&person.family==Family::Civilian&&person.age_days>=14*365){traveler=id;break;}
    }
    check(traveler>0,"Atlas fixture contains an adult Civilian for mounted travel");
    if(traveler>0){
        RealmObservationTestAccess::provide_route_cash(mounted,traveler,10000);
        int destination=-1,routeDays=0;
        const auto initialCandidates=mounted.candidates(traveler);
        const auto initialMask=mounted.mask(traveler,initialCandidates);
        for(int region=0;region<int(kAtlasTerritoryCount);++region){
            const int days=mounted.travel_days(mounted.person(traveler).region,region);
            const size_t slot=region<7?size_t(8+region):
                size_t(realm_extended_travel_candidate_begin+region-realm_extended_travel_region_begin);
            if(region!=mounted.person(traveler).region&&days>1&&initialMask[slot]==1.f){
                destination=region;routeDays=days;break;
            }
        }
        check(destination>=0,"adult Civilian has an open Atlas route longer than one day");
        if(destination>=0){
            const size_t slot=destination<7?size_t(8+destination):
                size_t(realm_extended_travel_candidate_begin+destination-realm_extended_travel_region_begin);
            const auto before=mounted.candidates(traveler)[slot];
            check(before.kind==Intent::Migrate&&before.good==0&&before.term==routeDays,
                  "unmounted migration candidate shows the full route time");
            const uint64_t other=traveler==1?2:1;
            RealmObservationTestAccess::add_warhorse(mounted,other);
            check(mounted.candidates(traveler)[slot].term==routeDays,
                  "another person's WarHorse does not shorten this actor's route");
            RealmObservationTestAccess::add_warhorse(mounted,traveler);
            check(mounted.invariant(),"personal WarHorse preserves the world goods balance");
            const auto action=mounted.candidates(traveler)[slot];
            const int mountedDays=std::max(1,routeDays-1);
            check(action.kind==Intent::Migrate&&action.good==int(GoodId::WarHorse)&&action.term==mountedDays&&
                  mounted.mask(traveler)[slot]==1.f,
                  "own WarHorse shortens the displayed route by one day and keeps it legal");
            const int64_t cashBefore=mounted.person(traveler).cash;
            const auto departed=mounted.execute({traveler,mounted.person(traveler).decisions+1,action});
            check(departed.success&&mounted.person(traveler).cash==cashBefore-(4+2*mountedDays)&&
                  mounted.person(traveler).busy_until==mounted.day()+mountedDays+1&&
                  mounted.person(traveler).inventory[size_t(GoodId::WarHorse)]==1&&mounted.invariant(),
                  "mounted migration charges the reduced route fee, arrives earlier and keeps the durable WarHorse");
            const std::string inFlight=mounted.serialize("warhorse-travel-v1");
            Realm restored(1,1,128);std::string error;
            check(restored.deserialize(inFlight,"warhorse-travel-v1",error),
                  "save/load accepts an in-flight mounted migration: "+error);
            Realm uninterrupted=mounted;
            uninterrupted.advance(mountedDays+2);
            restored.advance(mountedDays+2);
            check(restored.fingerprint()==uninterrupted.fingerprint()&&
                  restored.person(traveler).region==destination&&
                  restored.person(traveler).inventory[size_t(GoodId::WarHorse)]==1,
                  "mounted arrival and remaining horse match after save/load continuation");
        }
    }

    Realm blockedHorse(20261026,26,512);
    uint64_t blockedTraveler=0;
    for(uint64_t id=1;id<=512;++id){
        const auto& person=blockedHorse.person(id);
        if(person.alive&&person.family==Family::Civilian&&person.age_days>=14*365){blockedTraveler=id;break;}
    }
    check(blockedTraveler>0,"blocked-route fixture contains an adult Civilian");
    if(blockedTraveler>0){
    RealmObservationTestAccess::add_warhorse(blockedHorse,blockedTraveler);
    const int horseOrigin=blockedHorse.person(blockedTraveler).region;
    std::vector<uint8_t> horseOutgoing;
    for(size_t i=0;i<blockedHorse.national_transport_routes().size();++i)
        if(blockedHorse.national_transport_routes()[i].from==horseOrigin)horseOutgoing.push_back(uint8_t(i));
    for(const auto index:horseOutgoing)
        check(blockedHorse.execute(TransportRouteCommand{1,blockedHorse.person(1).decisions+1,index,false}).success,
              "sovereign closes outgoing routes in the mounted rejection fixture");
    const auto blockedHorseBefore=blockedHorse.fingerprint();
    const int horseTarget=(horseOrigin+1)%int(kAtlasTerritoryCount);
    const size_t horseSlot=horseTarget<7?size_t(8+horseTarget):
        size_t(realm_extended_travel_candidate_begin+horseTarget-realm_extended_travel_region_begin);
    const auto horseAction=blockedHorse.candidates(blockedTraveler)[horseSlot];
    const auto horseRejected=blockedHorse.execute({blockedTraveler,blockedHorse.person(blockedTraveler).decisions+1,horseAction});
    check(!horseRejected.success&&blockedHorse.fingerprint()==blockedHorseBefore,
          "a personal WarHorse cannot bypass closed Atlas routes or partially charge its owner");
    }
    return failures == 0 ? 0 : 1;
}
