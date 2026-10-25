#include "work_authorization_fixture.hpp"
#include "polyec/establishment_section_codec.hpp"
#include "polyec/realm.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace polyec;
namespace polyec {
struct RealmObservationTestAccess {
    static void add_personal_cash(Realm& world, uint64_t owner, int64_t quantity) {
        world.mutable_person(owner).cash += quantity;
        world.opening_money_ += quantity;
    }
    static void add_secret_fact(Realm& world, int region, uint64_t actor) {
        world.event(actor, region, 777, 1, 0, true);
    }
    static int64_t consumed_paper(const Realm& world) {
        return world.consumed_[size_t(GoodId::Paper)];
    }
};
}
namespace {
int checks = 0;
void check(bool ok, const char* message) {
    ++checks;
    if (!ok) throw std::runtime_error(message);
}
uint64_t find_person(const Realm& world, int region, Family family, bool adult = false) {
    uint64_t selected = 0;
    for (uint64_t id = 1; id <= uint64_t(world.count()); ++id) {
        const auto& person = world.person(id);
        if (person.alive && person.destination == -1 && person.region == region &&
            person.family == family && (!adult || person.age_days >= 14 * 365) &&
            (!selected || person.cash > world.person(selected).cash)) selected = id;
    }
    return selected;
}
int shortest_open_route_hops(const Realm& world, int from, int to) {
    std::array<int,kAtlasTerritoryCount> distance{};
    distance.fill(-1);
    std::vector<uint8_t> frontier{uint8_t(from)};
    distance[size_t(from)] = 0;
    for (size_t head = 0; head < frontier.size(); ++head) {
        const uint8_t current = frontier[head];
        for (const auto& route : world.national_transport_routes()) {
            if (!route.open || route.from != current || distance[route.to] >= 0) continue;
            distance[route.to] = distance[current] + 1;
            frontier.push_back(route.to);
        }
    }
    return distance[size_t(to)];
}
}

int main() {
    try {
        constexpr int forest_region = 6;
        constexpr int mill_region = 13;
        constexpr int timber = int(GoodId::Timber);
        constexpr int paper = int(GoodId::Paper);
        Realm world(49201, 26, 256);
        const uint64_t forest_governor = forest_region + 2;
        const uint64_t mill_governor = mill_region + 2;
        uint64_t forest_worker = find_person(world, forest_region, Family::Civilian, true);
        uint64_t merchant = find_person(world, forest_region, Family::Economic, true);
        check(world.person(forest_governor).taxing_authority &&
              world.person(mill_governor).taxing_authority && forest_worker && merchant,
              "local authorities, a Forestry worker and a merchant are present in the source region");

        const size_t paper_candidate = size_t(realm_establishment_candidate_begin + 18);
        auto candidates = world.candidates(mill_governor);
        check(candidates[paper_candidate].kind == Intent::CreateEstablishment &&
              candidates[paper_candidate].good == int(EstablishmentIndustry::PaperMill) &&
              candidates[paper_candidate].target == mill_region &&
              world.mask(mill_governor, candidates)[paper_candidate] == 1.f,
              "region 13 authority can choose the single Paper Mill site candidate");
        const uint64_t region13_traveler = find_person(world, forest_region, Family::Economic, true);
        const auto travel_choices = world.candidates(region13_traveler);
        int travel_to_13 = -1;
        for (int index = 15; index < realm_actions; ++index)
            if (travel_choices[size_t(index)].kind == Intent::Travel &&
                travel_choices[size_t(index)].target == mill_region) travel_to_13 = index;
        check(travel_to_13 == realm_extended_travel_candidate_begin + mill_region - realm_extended_travel_region_begin,
              "the new final action offers travel to atlas region 13 while legacy candidate slots stay in place");
        uint64_t civilian = find_person(world, forest_region, Family::Civilian, true);
        const auto migrate_choices = world.candidates(civilian);
        check(migrate_choices[size_t(travel_to_13)].kind == Intent::Migrate &&
              migrate_choices[size_t(travel_to_13)].target == mill_region,
              "the same appended destination offers Migrate to a civilian without moving earlier actions");
        bool every_extended_region_is_mapped = true;
        for (int region = realm_extended_travel_region_begin; region <= 26; ++region) {
            const size_t index = size_t(realm_extended_travel_candidate_begin +
                region - realm_extended_travel_region_begin);
            every_extended_region_is_mapped &= travel_choices[index].kind == Intent::Travel &&
                travel_choices[index].target == region && migrate_choices[index].kind == Intent::Migrate &&
                migrate_choices[index].target == region;
        }
        check(every_extended_region_is_mapped,
              "all appended travel and migration choices map one-to-one to atlas regions 7 through 26");
        const auto wrong_region = world.execute(EstablishmentCommand{forest_governor,
            world.person(forest_governor).decisions + 1, EstablishmentOwnerKind::Public,
            forest_region, forest_region, EstablishmentIndustry::PaperMill, 1, 25});
        check(!wrong_region.success && world.establishments().empty(),
              "Paper Mill creation outside region 13 is rejected without adding a site");

        const auto forest_created = world.execute(EstablishmentCommand{forest_governor,
            world.person(forest_governor).decisions + 1, EstablishmentOwnerKind::Public,
            forest_region, forest_region, EstablishmentIndustry::Forestry, 1, 25});
        check(forest_created.success && world.establishments().size() == 1 &&
              world.establishments()[0].source_good == timber &&
              world.establishments()[0].source_units == 100,
              "the established Forestry site in region 6 is the source of the test Timber");
        const uint64_t forest_site = forest_created.session;
        check(world.execute(WorkAuthorizationCommand{forest_governor,
            world.person(forest_governor).decisions + 1, forest_site, forest_worker,
            WorkAuthorizationOperation::Set, world.game_tick(), world.game_tick() + 32}).success,
            "region 6 authority grants the Forestry worker a valid Work period");
        check(world.execute(ScheduleCommand{forest_worker, world.person(forest_worker).decisions + 1,
            ScheduleCommand::Operation::Submit, 0, world.game_tick(), world.game_tick() + 32,
            0, 1000, ActivityKind::Work, forest_region, forest_site}).success,
            "Forestry worker submits Work through the regular schedule command");
        world.advance_ticks(64);
        check(world.establishments()[0].source_units == 99 &&
              world.establishments()[0].stock[size_t(timber)] == 1,
              "one completed Forestry Work unit consumes one source unit and creates one Timber");

        check(world.execute(EstablishmentTransferCommand{forest_governor,
            world.person(forest_governor).decisions + 1, forest_site,
            EstablishmentTransferOperation::WithdrawGoods, timber, 1}).success &&
              world.view(forest_governor).provinces[size_t(forest_region)].stock[size_t(timber)] == 1,
              "the public Forestry owner moves its produced Timber into the region 6 market stock");
        check(world.execute(RealmCommand{merchant, world.person(merchant).decisions + 1,
            {Intent::Buy, forest_region, timber, 1}}).success &&
              world.person(merchant).inventory[size_t(timber)] == 1,
              "merchant buys that Timber through the ordinary market command");
        const auto shipment = world.execute(TransportShipmentCommand{merchant,
            world.person(merchant).decisions + 1, 0, TransportShipmentOperation::Create,
            mill_region, timber, 1, TransportServiceGoal::Fastest});
        check(shipment.success && world.person(merchant).inventory[size_t(timber)] == 0 &&
              !world.transport_shipments().empty() && world.transport_shipments().back().owner == merchant,
              "merchant pays for a real route and sends the purchased Timber to region 13");
        const auto merchantChoices = world.candidates(merchant);
        int merchantTravel = -1;
        for (int index = 15; index < realm_actions; ++index)
            if (merchantChoices[size_t(index)].kind == Intent::Travel &&
                merchantChoices[size_t(index)].target == mill_region) merchantTravel = index;
        check(merchantTravel >= 0 && world.mask(merchant)[size_t(merchantTravel)] == 1.f,
              "merchant has a legal region 13 travel choice after paying for the shipment");
        const auto travelRoute = NationalTransportManager::plan(world.day(), uint8_t(forest_region),
            uint8_t(mill_region), 1, TransportServiceGoal::Fastest, world.national_transport_routes());
        check(travelRoute.reachable && travelRoute.route_indices.size() > 0,
              "real travel uses an open, directed route path from region 6 to region 13");
        const int travelDistance = shortest_open_route_hops(world, forest_region, mill_region);
        check(travelDistance > 0,
              "travel distance is the shortest number of currently open directed route segments");
        const int64_t cashBeforeTravel = world.person(merchant).cash;
        const int plannedTravelDays = merchantChoices[size_t(merchantTravel)].term;
        const int travelArrivalDay = world.day() + std::max(1, plannedTravelDays + 1 -
            world.view(merchant).provinces[size_t(forest_region)].road / 2);
        const auto travel = world.execute(RealmCommand{merchant,
            world.person(merchant).decisions + 1, merchantChoices[size_t(merchantTravel)]});
        check(travel.success, travel.reason.c_str());
        if(world.person(merchant).cash != cashBeforeTravel - (4 + 2 * plannedTravelDays) ||
           world.person(merchant).busy_until != travelArrivalDay){
            std::ostringstream detail;detail<<"travel fee/time mismatch: hops="<<travelDistance<<" planned_days="<<plannedTravelDays
                <<" cash_before="<<cashBeforeTravel<<" cash_after="<<world.person(merchant).cash
                <<" expected_cash="<<(cashBeforeTravel-(4+2*plannedTravelDays))
                <<" busy_until="<<world.person(merchant).busy_until<<" expected_day="<<travelArrivalDay;
            throw std::runtime_error(detail.str());
        }
        world.advance(40);
        check(world.person(merchant).region == mill_region && world.person(merchant).destination == -1 &&
              world.person(merchant).inventory[size_t(timber)] == 1 &&
              world.transport_shipments().back().delivered == 1,
              "the route delivers the purchased Timber to its owner while the owner travels to region 13");

        const auto mill_created = world.execute(EstablishmentCommand{merchant,
            world.person(merchant).decisions + 1, EstablishmentOwnerKind::Person,
            merchant, mill_region, EstablishmentIndustry::PaperMill, 1, 25});
        check(mill_created.success && world.establishments().size() == 2 &&
              world.establishments().back().industry == EstablishmentIndustry::PaperMill &&
              world.establishments().back().source_good == kNoEstablishmentSourceGood &&
              world.establishments().back().source_units == 0,
              "merchant creates the unique Paper Mill with no invented natural-resource ledger");
        const uint64_t mill_site = mill_created.session;
        check(world.execute(EstablishmentCommand{mill_governor,
            world.person(mill_governor).decisions + 1, EstablishmentOwnerKind::Public,
            mill_region, mill_region, EstablishmentIndustry::PaperMill, 1, 25}).success == false &&
              world.establishments().size() == 2,
              "a second Paper Mill in region 13 is rejected");

        uint64_t mill_worker = find_person(world, mill_region, Family::Civilian, true);
        check(mill_worker != 0 && mill_worker != merchant,
              "an adult worker is available beside the Paper Mill");
        check(world.execute(EstablishmentTransferCommand{merchant,
            world.person(merchant).decisions + 1, mill_site,
            EstablishmentTransferOperation::DepositGoods, timber, 1}).success &&
              world.establishments().back().stock[size_t(timber)] == 1 &&
              world.person(merchant).inventory[size_t(timber)] == 0,
              "the Paper Mill owner deposits the delivered Timber into the establishment stock");
        check(world.execute(WorkAuthorizationCommand{merchant,
            world.person(merchant).decisions + 1, mill_site, mill_worker,
            WorkAuthorizationOperation::Set, world.game_tick(), world.game_tick() + 32}).success,
            "the Paper Mill owner authorizes an adult worker to perform Work");
        check(world.execute(ScheduleCommand{mill_worker, world.person(mill_worker).decisions + 1,
            ScheduleCommand::Operation::Submit, 0, world.game_tick(), world.game_tick() + 32,
            0, 1000, ActivityKind::Work, mill_region, mill_site}).success,
            "the authorized worker schedules Work at the Paper Mill");
        const std::string before_production = world.serialize("paper-mill-trial-1");
        Realm resumed(1, 1, 128);
        std::string error;
        check(resumed.deserialize(before_production, "paper-mill-trial-1", error),
              "save/load accepts the schema 40 Paper Mill world");
        const auto loadedFingerprint = resumed.fingerprint();
        std::istringstream oldIndustry("ESTABLISH_8 1 129 2 1 2 12 1 10");
        std::vector<Establishment> decodedOldIndustry;
        check(!detail::read_establishment_section(oldIndustry, 128, 27, 38,
              decodedOldIndustry, error) && decodedOldIndustry.empty() &&
              error == "Save establishment industry" && resumed.fingerprint() == loadedFingerprint,
              "schema 38 establishment reader rejects the newer Paper Mill industry without changing the loaded world");
        world.advance_ticks(64);
        resumed.advance_ticks(64);
        check(world.fingerprint() == resumed.fingerprint() &&
              world.establishments().back().stock[size_t(timber)] == 0 &&
              world.establishments().back().stock[size_t(paper)] == 1 &&
              world.establishments().back().stock[size_t(GoodId::Fiber)] == 0,
              "saved and live worlds consume exactly one Timber and produce exactly one Paper, without substituting Fiber");
        auto output = world.execute(EstablishmentTransferCommand{merchant,
            world.person(merchant).decisions + 1, mill_site,
            EstablishmentTransferOperation::WithdrawGoods, paper, 1});
        check(output.success && world.person(merchant).inventory[size_t(paper)] == 1 &&
              world.establishments().back().stock[size_t(paper)] == 0,
              "owner withdraws the Paper output through the normal goods-transfer command");
        check(world.execute(RealmCommand{merchant, world.person(merchant).decisions + 1,
            {Intent::Sell, mill_region, paper, 1}}).success &&
              world.person(merchant).inventory[size_t(paper)] == 0,
              "merchant sells the produced Paper through the existing market command");
        RealmObservationTestAccess::add_personal_cash(world, mill_governor, 100);
        check(world.invariant(),
              "test opening purse is funded explicitly and reconciled in the world money ledger");
        const auto governorCashBeforePaperPurchase = world.person(mill_governor).cash;
        const auto buyPaper = world.execute(RealmCommand{mill_governor,
            world.person(mill_governor).decisions + 1, {Intent::Buy, mill_region, paper, 1}});
        if (!buyPaper.success) {
            std::ostringstream detail;
            detail << "Governance Paper Buy rejected: " << buyPaper.reason
                   << "; price=" << world.view(mill_governor).provinces[size_t(mill_region)].price[size_t(paper)]
                   << "; cash=" << governorCashBeforePaperPurchase
                   << "; stock=" << world.view(mill_governor).provinces[size_t(mill_region)].stock[size_t(paper)];
            throw std::runtime_error(detail.str());
        }
        check(buyPaper.success && world.person(mill_governor).inventory[size_t(paper)] == 1 &&
              world.person(mill_governor).cash < governorCashBeforePaperPurchase &&
              world.invariant(),
              "Governance buys one Paper from the market using the region treasury through the regular Buy action");
        const std::string afterPaperPurchase = world.serialize("paper-investigation-trial-1");
        Realm resumedIntegration(1, 1, 128);
        check(resumedIntegration.deserialize(afterPaperPurchase,
              "paper-investigation-trial-1", error) &&
              resumedIntegration.person(mill_governor).inventory[size_t(paper)] == 1 &&
              resumedIntegration.invariant(),
              "save/load preserves Governance's Paper bought from the regional market");
        world.advance(1);
        resumedIntegration.advance(1);
        check(world.fingerprint() == resumedIntegration.fingerprint() &&
              world.market_orders().empty() && world.person(mill_governor).inventory[size_t(paper)] == 1 &&
              world.view(mill_governor).provinces[size_t(mill_region)].stock[size_t(paper)] == 0 &&
              world.invariant(),
              "the completed market purchase removes the single Paper from local stock and survives one day of play");
        RealmObservationTestAccess::add_secret_fact(world, mill_region, merchant);
        RealmObservationTestAccess::add_secret_fact(resumedIntegration, mill_region, merchant);
        const auto paperBeforeInvestigation = world.person(mill_governor).inventory[size_t(paper)];
        const auto investigationCommand = world.candidates(mill_governor)[13];
        const auto investigation = world.execute({mill_governor,
            world.person(mill_governor).decisions + 1, investigationCommand});
        const auto resumedInvestigation = resumedIntegration.execute({mill_governor,
            resumedIntegration.person(mill_governor).decisions + 1,
            resumedIntegration.candidates(mill_governor)[13]});
        check(investigation.success && resumedInvestigation.success &&
              world.person(mill_governor).inventory[size_t(paper)] == paperBeforeInvestigation - 1 &&
              RealmObservationTestAccess::consumed_paper(world) == 1 &&
              world.investigations().back().owner == mill_governor && world.invariant(),
              "Governance spends its purchased Paper to open an Investigation through the normal action");
        const uint64_t investigationId = world.investigations().back().id;
        const int daysUntilInvestigation = world.investigations().back().due - world.day();
        const std::string beforeInvestigationCompletes = world.serialize("paper-investigation-trial-1");
        Realm resumedCompletion(1, 1, 128);
        check(resumedCompletion.deserialize(beforeInvestigationCompletes,
              "paper-investigation-trial-1", error) &&
              resumedCompletion.person(mill_governor).inventory[size_t(paper)] == 0 &&
              RealmObservationTestAccess::consumed_paper(resumedCompletion) == 1 &&
              resumedCompletion.invariant(),
              "save/load keeps the consumed Paper and pending Investigation before it is due");
        world.advance(daysUntilInvestigation);
        resumedIntegration.advance(daysUntilInvestigation);
        resumedCompletion.advance(daysUntilInvestigation);
        const auto knowledge = world.view(mill_governor).knowledge;
        const auto completed = world.investigations().back();
        check(completed.completed && completed.id == investigationId &&
              std::any_of(knowledge.begin(), knowledge.end(), [&](const PlayerKnowledge& row) {
                  return row.kind == KnowledgeKind::SecretEvidence &&
                         row.source_investigation_id == investigationId;
              }), "completed Investigation gives Governance evidence linked to its own case record");
        check(world.fingerprint() == resumedIntegration.fingerprint() &&
              world.fingerprint() == resumedCompletion.fingerprint() &&
              world.invariant() && resumedIntegration.invariant() && resumedCompletion.invariant(),
              "uninterrupted, purchase-resumed and investigation-resumed worlds reach the same valid state");

        Realm closedRoutes(49201, 26, 256);
        const uint64_t sovereign = 1;
        check(closedRoutes.person(sovereign).family == Family::Governance &&
              closedRoutes.person(sovereign).sovereign,
              "national route closure checks use the existing sovereign authority");
        const uint64_t initially_connected = find_person(closedRoutes, forest_region, Family::Economic, true);
        const size_t closed_travel_action = size_t(realm_extended_travel_candidate_begin +
            mill_region - realm_extended_travel_region_begin);
        check(closedRoutes.mask(initially_connected)[closed_travel_action] == 1.f,
              "an open route path makes the region 13 travel action legal before route changes");
        for (size_t route = 0; route < closedRoutes.national_transport_routes().size(); ++route) {
            const auto result = closedRoutes.execute(TransportRouteCommand{sovereign,
                closedRoutes.person(sovereign).decisions + 1, uint8_t(route), false});
            check(result.success, "sovereign closes an existing national route through its command");
        }
        const uint64_t stranded = find_person(closedRoutes, forest_region, Family::Economic, true);
        check(closedRoutes.candidates(stranded)[closed_travel_action].target == mill_region &&
              closedRoutes.mask(stranded)[closed_travel_action] == 0.f,
              "destination remains visible but a traveler cannot act when all connecting routes are closed");

        std::cout << "Paper Mill checks=" << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Paper Mill failure after " << checks << ": " << error.what() << '\n';
        return 1;
    }
}





