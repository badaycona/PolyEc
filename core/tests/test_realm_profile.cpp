#include "polyec/realm.hpp"
#include "polyec/world_atlas.hpp"

#include <iostream>
#include <stdexcept>

namespace {
int checks = 0;
int failures = 0;
void check(bool value, const char* message) {
    ++checks;
    if (!value) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}
}

int main() {
    using namespace polyec;
    Realm old_profile(71, 6, 128);
    check(old_profile.invariant() && old_profile.view().provinces.size() == 7 &&
          old_profile.distance(0, 6) == 1,
          "the existing seven-region profile keeps its province count and road behavior");

    Realm national_profile(72, 26, 128);
    check(national_profile.invariant() && national_profile.regions() == 26 &&
          national_profile.view().provinces.size() == kAtlasTerritoryCount,
          "region bound 26 creates exactly the atlas territory IDs 0 through 26");
    check(national_profile.national_transport_routes().size() == 59 &&
          national_profile.national_transport_routes().front().open,
          "a national Realm owns a complete default route snapshot");
    check(national_profile.person(1).region == 26 &&
          national_profile.person(1).sovereign &&
          national_profile.distance(24, 25) == -1,
          "the sovereign can occupy royal-domain ID 26 while the legacy 7x7 road grid stays disabled");

    const auto before_reject = national_profile.fingerprint();
    check(!national_profile.route_command(1, 24, 25, true).success &&
          national_profile.fingerprint() == before_reject,
          "the legacy Campaign route command rejects atlas regions without mutating a 27-region Realm");
    const auto close_river = national_profile.execute(TransportRouteCommand{
        1, national_profile.person(1).decisions + 1, 0, false});
    check(close_river.success && !national_profile.national_transport_routes().front().open,
          "the sovereign can close one configured national route through a Realm command");
    const auto reject_before_policy_query = national_profile.fingerprint();
    bool candidates_rejected=false, mask_rejected=false, reused_mask_rejected=false,
         observation_rejected=false, reused_observation_rejected=false;
    std::array<Candidate,realm_actions> atlas_candidates{};
    try { (void)national_profile.candidates(1); } catch (const std::logic_error&) { candidates_rejected=true; }
    try { (void)national_profile.mask(1); } catch (const std::logic_error&) { mask_rejected=true; }
    try { (void)national_profile.mask(1,atlas_candidates); } catch (const std::logic_error&) { reused_mask_rejected=true; }
    try { (void)national_profile.observation(1); } catch (const std::logic_error&) { observation_rejected=true; }
    try { (void)national_profile.observation(1,atlas_candidates); } catch (const std::logic_error&) { reused_observation_rejected=true; }
    check(candidates_rejected && mask_rejected && reused_mask_rejected &&
          observation_rejected && reused_observation_rejected &&
          national_profile.fingerprint()==reject_before_policy_query && national_profile.invariant(),
          "the legacy seven-region policy interface rejects the atlas profile before producing incomplete or overlapping data");

    const auto legacy_candidates=old_profile.candidates(1);
    const auto legacy_observation=old_profile.observation(1);
    const auto legacy_snapshot=old_profile.view();
    bool exact_legacy_prices=true;
    for(int region=0;region<=6;++region)for(int good=0;good<kLegacyGoodCount;++good){
        const auto canonical=size_t(canonical_from_legacy_good(good));
        const size_t feature=size_t(24+region*kLegacyGoodCount+good);
        exact_legacy_prices=exact_legacy_prices &&
            legacy_observation[feature]==legacy_snapshot.provinces[size_t(region)].price[canonical]/float(kGoodReferencePrice[canonical]);
    }
    check(exact_legacy_prices && 24+6*kLegacyGoodCount+(kLegacyGoodCount-1)<realm_state,
          "known prices for all seven legacy regions occupy exactly the state features and stop before action features");
    check(legacy_observation[realm_state]==int(legacy_candidates[0].kind)/32.f &&
          legacy_observation[realm_state+realm_features+5]==old_profile.mask(1)[1],
          "known action type and legal-action mask values begin at the fixed action-feature boundary");

    const auto serialized = national_profile.serialize("national-region-profile");
    Realm restored;
    std::string error;
    const bool restored_ok = restored.deserialize(serialized, "national-region-profile", error);
    if (!restored_ok) std::cerr << "RESTORE ERROR: " << error << '\n';
    check(serialized.rfind("POLYEC_REALM_28 ", 0) == 0 &&
          restored_ok &&
          restored.regions() == 26 && restored.view().provinces.size() == kAtlasTerritoryCount &&
          restored.national_transport_routes() == national_profile.national_transport_routes() &&
          restored.fingerprint() == national_profile.fingerprint() && restored.invariant(),
          "schema 28 round-trips the route snapshot, empty shipment ledger, and 27-territory Realm state");

    auto schema27=serialized;
    const auto shipment_section=schema27.find(" TRANSPORT_SHIPMENT_1 ");
    check(shipment_section!=std::string::npos,"schema 28 writes an independent shipment ledger section");
    schema27.erase(shipment_section);
    schema27.replace(0,std::string("POLYEC_REALM_28").size(),"POLYEC_REALM_27");
    Realm migrated27;
    check(migrated27.deserialize(schema27,"national-region-profile",error)&&
          migrated27.national_transport_routes()==national_profile.national_transport_routes()&&
          migrated27.transport_shipments().empty()&&migrated27.transport_cargo_lots().empty()&&migrated27.invariant(),
          "schema 27 retains route configuration and migrates into an empty shipment ledger");

    auto schema26 = serialized;
    const auto route_section = schema26.find(" NATIONAL_TRANSPORT_1 ");
    check(route_section != std::string::npos, "schema 27 writes a dedicated route snapshot section");
    schema26.erase(route_section);
    schema26.replace(0, std::string("POLYEC_REALM_28").size(), "POLYEC_REALM_26");
    Realm migrated;
    check(migrated.deserialize(schema26, "national-region-profile", error) &&
          migrated.national_transport_routes().size() == 59 &&
          migrated.national_transport_routes().front().open && migrated.invariant(),
          "schema 27 migration creates default route data without carrying a later closure forward");

    auto malformed = serialized;
    const auto river_direction = malformed.find(" 0 0 2 1 0 ", route_section);
    check(river_direction != std::string::npos, "route section exposes explicit river endpoints in save data");
    malformed.replace(river_direction, std::string(" 0 0 2 1 0 ").size(), " 0 2 0 1 0 ");
    const auto destination_before_bad_load = restored.fingerprint();
    check(!restored.deserialize(malformed, "national-region-profile", error) &&
          restored.fingerprint() == destination_before_bad_load,
          "a save with a reversed river config is rejected before replacing the current Realm");

    bool rejected = false;
    try { Realm invalid_profile(73, 27, 128); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "rejects an invalid region bound beyond the 27 atlas entries");

    if (failures) return 1;
    std::cout << "REALM PROFILE: " << checks << '/' << checks << " checks passed\n";
    return 0;
}






