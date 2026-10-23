#include "polyec/realm_runtime.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <vector>

namespace {
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
constexpr std::array<const char*, polyec::Realm::day_phase_count> phaseNames{
    "day_begin", "body", "payroll", "people", "production", "harvest", "projects",
    "investigations", "treasury", "loans", "proposals", "births", "day_end", "settlement"};
constexpr std::array<const char*, polyec::RealmRuntime::runtime_phase_count> runtimePhaseNames{
    "role_sync_due", "observation_pack", "policy_inference", "action_revalidate_commit",
    "realm_advance", "role_resize_sync", "invariant"};
#endif
int count_complex_actors(const polyec::Realm& world) {
    int count = 0;
    for (int id = 1; id <= world.count(); ++id)
        count += world.person(static_cast<uint64_t>(id)).family != polyec::Family::Civilian;
    return count;
}

void write_array(std::ostream& out, const std::array<int64_t, 11>& values) {
    out << '[';
    for (size_t i = 0; i < values.size(); ++i) {
        if (i) out << ',';
        out << values[i];
    }
    out << ']';
}

std::optional<bool> manifest_r1_claim(const std::string& model_path) {
    std::ifstream input(std::filesystem::u8path(model_path).parent_path() / "manifest.json",
                        std::ios::binary);
    if (!input) return std::nullopt;
    const std::string json((std::istreambuf_iterator<char>(input)), {});
    const std::regex field("\\\"full_R1_pass\\\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (!std::regex_search(json, match, field)) return std::nullopt;
    return match[1].str() == "true";
}
}

int main(int argc, char** argv) {
    if (argc < 6 || argc > 10) return 2;
    const int days = argc >= 7 ? std::stoi(argv[6]) : 365;
    const uint64_t seed = argc >= 8 ? std::stoull(argv[7]) : 37;
    const int population = argc >= 9 ? std::stoi(argv[8]) : 10000;
    const int complexActors = argc >= 10 ? std::stoi(argv[9]) : 0;
    if (days < 1 || days > 36500 || population < 128 || population > 1000000 ||
        complexActors < 0 || complexActors > 2048 ||
        (complexActors != 0 && (complexActors < 128 || complexActors % 2 != 0 || complexActors > population))) return 2;

    polyec::RealmRuntime runtime;
    if (!runtime.load_models({argv[1], argv[2], argv[3]}, argv[4], seed, population, true, complexActors)) {
        std::cerr << runtime.error();
        return 1;
    }
    runtime.choose(runtime.world()->draft().cards[0]);
    const auto start = runtime.world()->view();
    const int actualComplexActors = count_complex_actors(*runtime.world());
    std::vector<double> times;
    std::vector<double> inferenceDayTimes;
    std::vector<double> nonInferenceDayTimes;
    bool phaseTimesConsistent = true;
    times.reserve(static_cast<size_t>(days));
    inferenceDayTimes.reserve(static_cast<size_t>(days));
    nonInferenceDayTimes.reserve(static_cast<size_t>(days));
    for (int day = 0; day < days; ++day) {
        if (day == 30) runtime.route(1, 0, 1, false);
        const double inferenceBefore = runtime.inference_ms();
        const auto started = std::chrono::steady_clock::now();
        if (!runtime.advance(1)) {
            std::cerr << runtime.error();
            return 1;
        }
        const double fullDay = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - started).count();
        const double inferenceDay = runtime.inference_ms() - inferenceBefore;
        phaseTimesConsistent = phaseTimesConsistent && inferenceDay >= 0.0 &&
            inferenceDay <= fullDay + 0.05;
        times.push_back(fullDay);
        inferenceDayTimes.push_back(inferenceDay);
        nonInferenceDayTimes.push_back(std::max(0.0, fullDay - inferenceDay));
    }

    const auto view = runtime.world()->view();
    std::sort(times.begin(), times.end());
    std::sort(inferenceDayTimes.begin(), inferenceDayTimes.end());
    std::sort(nonInferenceDayTimes.begin(), nonInferenceDayTimes.end());
    size_t households = 0;
    for (int id = actualComplexActors + 1; id <= runtime.world()->count(); ++id)
        households += runtime.world()->person(static_cast<uint64_t>(id)).household_membership.household_id ==
                      static_cast<uint64_t>(id);
    const bool profileBPopulation = population > 10000 || actualComplexActors > 128;
    const std::array<std::optional<bool>, 3> r1Claims{
        manifest_r1_claim(argv[1]), manifest_r1_claim(argv[2]), manifest_r1_claim(argv[3])};
    const bool allR1ClaimsDeclared = std::all_of(r1Claims.begin(), r1Claims.end(),
                                                 [](const auto& value) { return value.has_value(); });
    const bool allR1ClaimsPass = allR1ClaimsDeclared && std::all_of(
        r1Claims.begin(), r1Claims.end(), [](const auto& value) { return *value; });
    const bool anyR1ClaimFails = std::any_of(
        r1Claims.begin(), r1Claims.end(), [](const auto& value) { return value && !*value; });
    const std::string qualityLabel = anyR1ClaimFails ? "manifest_R1_false" :
        (allR1ClaimsPass ? "manifest_R1_true_unverified" : "manifest_R1_unspecified");
    const std::string profileLabel = profileBPopulation
        ? "B-population-scale-only; " + std::to_string(actualComplexActors) +
              " complex actors; " + qualityLabel
        : "A; " + qualityLabel;
    const auto balances = runtime.world()->balances();

    std::ofstream out(argv[5]);
    if (!out) {
        std::cerr << "Could not open benchmark output";
        return 1;
    }
    out << "{\"profile\":\"" << profileLabel << "\",\"model_contract\":\""
        << runtime.world()->model_contract() << "\",\"model_manifest_full_R1\":";
    if (!allR1ClaimsDeclared) out << "null";
    else out << (allR1ClaimsPass ? "true" : "false");
    out << ",\"model_quality_validation\":\"not_performed_by_benchmark\","
        << "\"population_target\":" << population << ",\"complex_actors_requested\":" << complexActors
        << ",\"config_fingerprint\":\"" << runtime.world()->gameplay_config().fingerprint()
        << "\",\"days\":" << days << ",\"population_opening\":" << start.persons
        << ",\"population_end\":" << view.persons << ",\"alive_end\":" << view.alive
        << ",\"children_end\":" << view.children << ",\"complex_actors_opening\":" << actualComplexActors
        << ",\"households\":" << households << ",\"graph_edges\":" << runtime.world()->relations().size()
        << ",\"accounts\":" << view.persons + 44 << ",\"facilities\":7,\"decisions\":"
        << runtime.inference_count() << ",\"total_inference_ms\":" << runtime.inference_ms()
        << ",\"full_day_p50_ms\":" << times[times.size() / 2]
        << ",\"full_day_p95_ms\":" << times[static_cast<size_t>(times.size() * 0.95)]
        << ",\"full_day_max_ms\":" << times.back()
        << ",\"inference_day_p50_ms\":" << inferenceDayTimes[inferenceDayTimes.size() / 2]
        << ",\"inference_day_p95_ms\":" << inferenceDayTimes[static_cast<size_t>(inferenceDayTimes.size() * 0.95)]
        << ",\"inference_day_max_ms\":" << inferenceDayTimes.back()
        << ",\"non_inference_day_p50_ms\":" << nonInferenceDayTimes[nonInferenceDayTimes.size() / 2]
        << ",\"non_inference_day_p95_ms\":" << nonInferenceDayTimes[static_cast<size_t>(nonInferenceDayTimes.size() * 0.95)]
        << ",\"non_inference_day_max_ms\":" << nonInferenceDayTimes.back()
        << ",\"phase_times_consistent\":" << (phaseTimesConsistent ? "true" : "false");
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    out << ",\"realm_phase_profile_ns\":{";
    for (size_t i = 0; i < phaseNames.size(); ++i) {
        if (i) out << ',';
        out << '\"' << phaseNames[i] << "\":" << runtime.world()->day_phase_profile_ns()[i];
    }
    out << "},\"runtime_phase_profile_ns\":{";
    for (size_t i = 0; i < runtimePhaseNames.size(); ++i) {
        if (i) out << ',';
        out << '\"' << runtimePhaseNames[i] << "\":" << runtime.runtime_phase_profile_ns()[i];
    }
    out << "},\"realm_phase_profile_enabled\":true";
#else
    out << ",\"realm_phase_profile_enabled\":false";
#endif
    out << ",\"money_conserved\":"
        << (start.money == view.money ? "true" : "false") << ",\"invariant\":"
        << (runtime.world()->invariant() ? "true" : "false") << ",\"route_shock_day\":30,\"route_shock_applied\":"
        << (days > 30 ? "true" : "false") << ",\"render_included\":false,\"save_included\":false,\"account_balances\":";
    write_array(out, balances);
    out << ",\"food_stock_end\":[";
    for (size_t i = 0; i < view.provinces.size(); ++i) {
        if (i) out << ',';
        out << view.provinces[i].stock[0];
    }
    out << "],\"family_outcomes\":[";
    for (int role = 0; role < 3; ++role) {
        int alive = 0, count = 0, working = 0;
        int64_t health = 0, nutrition = 0, cash = 0;
        for (int id = 1; id <= runtime.world()->count(); ++id) {
            const auto& person = runtime.world()->person(static_cast<uint64_t>(id));
            if (static_cast<int>(person.family) != role) continue;
            ++count;
            if (person.alive) {
                ++alive;
                health += person.health;
                nutrition += person.nutrition;
                working += person.work_assignment.job >= 0;
            }
            cash += person.cash;
        }
        if (role) out << ',';
        out << "{\"role\":" << role << ",\"persons\":" << count << ",\"alive\":" << alive
            << ",\"working\":" << working << ",\"mean_living_health\":"
            << static_cast<double>(health) / std::max(1, alive) << ",\"mean_living_nutrition\":"
            << static_cast<double>(nutrition) / std::max(1, alive) << ",\"cash\":" << cash << '}';
    }
    out << "]}";
    out.close();
    if (!out) {
        std::cerr << "Could not finish benchmark output";
        return 1;
    }
    const char* profileType = profileBPopulation ? "B-population-scale-only" : "A";
    std::cout << "PROFILE " << profileType << ':' << population << " people," << actualComplexActors
              << " complex actors," << days << " days," << runtime.inference_count()
              << " current-contract inferences (" << qualityLabel << "),p95 "
              << times[static_cast<size_t>(times.size() * 0.95)] << "ms\n";
}
