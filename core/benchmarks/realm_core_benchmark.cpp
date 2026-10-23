#include "polyec/realm.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

int percentile_index(size_t count, double percentile) {
    return static_cast<int>(std::min(count - 1, static_cast<size_t>(count * percentile)));
}

int count_complex_actors(const polyec::Realm& world) {
    int count = 0;
    for (int id = 1; id <= world.count(); ++id)
        count += world.person(static_cast<uint64_t>(id)).family != polyec::Family::Civilian;
    return count;
}

#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
constexpr std::array<const char*, polyec::Realm::day_phase_count> phase_names{
    "day_begin", "body", "payroll", "people", "production", "harvest", "projects",
    "investigations", "treasury", "loans", "proposals", "births", "day_end", "settlement"};
constexpr std::array<const char*, polyec::Realm::people_phase_count> people_phase_names{
    "resident_daily_pass", "market_settlement", "death_scan", "death_recording",
    "estate_distribution_and_index"};
#endif

} // namespace

int main(int argc, char** argv) {
    using namespace polyec;
    if (argc != 6) {
        std::cerr << "usage: polyec_realm_core_benchmark <population> <complex_actors> <seed> <days> <output.json>\n";
        return 2;
    }

    int population = 0;
    int complex_actors = 0;
    uint64_t seed = 0;
    int days = 0;
    try {
        population = std::stoi(argv[1]);
        complex_actors = std::stoi(argv[2]);
        seed = std::stoull(argv[3]);
        days = std::stoi(argv[4]);
    } catch (const std::exception&) {
        std::cerr << "population, actors, seed and days must be valid integers\n";
        return 2;
    }
    if (population < 128 || population > 1000000 || days < 1 || days > 36500 ||
        complex_actors < 0 || complex_actors > 2048 ||
        (complex_actors != 0 && (complex_actors < 128 || complex_actors % 2 != 0 ||
                                 complex_actors > population))) {
        std::cerr << "benchmark arguments are outside the supported Realm bounds\n";
        return 2;
    }

    Realm world(seed, 6, population, false, GameplayConfig{}, complex_actors);
    const auto draft = world.draft();
    if (!world.choose(draft.cards[0])) {
        std::cerr << "could not initialize Realm draft\n";
        return 1;
    }
    const int actual_complex_actors = count_complex_actors(world);
    const int64_t opening_money = world.total_money();
    std::vector<double> day_ms;
    day_ms.reserve(static_cast<size_t>(days));
    bool money_conserved = true;
    bool invariant = world.invariant();
    uint64_t verification_ns = 0;
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    std::array<uint64_t, Realm::day_phase_count> phase_ns{};
    std::array<uint64_t, Realm::people_phase_count> people_phase_ns{};
#endif

    for (int day = 0; day < days; ++day) {
        const auto started = std::chrono::steady_clock::now();
        world.advance(1);
        day_ms.push_back(std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - started).count());

        const auto verification_started = std::chrono::steady_clock::now();
        money_conserved = money_conserved && world.total_money() == opening_money;
        invariant = invariant && world.invariant();
        verification_ns += static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - verification_started).count());
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
        const auto& current = world.day_phase_profile_ns();
        for (size_t phase = 0; phase < phase_ns.size(); ++phase)
            phase_ns[phase] = current[phase];
        const auto& current_people = world.people_phase_profile_ns();
        for (size_t phase = 0; phase < people_phase_ns.size(); ++phase)
            people_phase_ns[phase] = current_people[phase];
#endif
    }

    std::sort(day_ms.begin(), day_ms.end());
    std::ofstream output(argv[5], std::ios::binary);
    if (!output) {
        std::cerr << "could not open benchmark output\n";
        return 1;
    }
    output << "{\"profile\":\"core-only-scale-diagnostic\",\"population_target\":" << population
           << ",\"complex_actors_requested\":" << complex_actors
           << ",\"complex_actors_opening\":" << actual_complex_actors
           << ",\"population_opening\":" << population
           << ",\"population_end\":" << world.count()
           << ",\"regions\":" << world.regions() + 1
           << ",\"graph_edges\":" << world.relations().size()
           << ",\"seed\":" << seed << ",\"days\":" << days
           << ",\"full_day_p50_ms\":" << day_ms[static_cast<size_t>(percentile_index(day_ms.size(), 0.50))]
           << ",\"full_day_p95_ms\":" << day_ms[static_cast<size_t>(percentile_index(day_ms.size(), 0.95))]
           << ",\"full_day_max_ms\":" << day_ms.back()
           << ",\"verification_total_ms\":" << static_cast<double>(verification_ns) / 1000000.0
           << ",\"money_conserved\":" << (money_conserved ? "true" : "false")
           << ",\"invariant\":" << (invariant ? "true" : "false")
           << ",\"policy_inference_included\":false,\"renderer_included\":false,\"save_load_included\":false"
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
           << ",\"realm_phase_profile_enabled\":true,\"realm_phase_profile_ns\":{";
    for (size_t phase = 0; phase < phase_ns.size(); ++phase) {
        if (phase) output << ',';
        output << '\"' << phase_names[phase] << "\":" << phase_ns[phase];
    }
    output << '}';
    output << ",\"realm_people_profile_ns\":{";
    for (size_t phase = 0; phase < people_phase_ns.size(); ++phase) {
        if (phase) output << ',';
        output << '\"' << people_phase_names[phase] << "\":" << people_phase_ns[phase];
    }
    output << '}';
#else
           << ",\"realm_phase_profile_enabled\":false,\"realm_phase_profile_ns\":{}";
#endif
    output << '}';
    output.close();
    if (!output) {
        std::cerr << "could not finish benchmark output\n";
        return 1;
    }

    std::cout << "PROFILE core-only scale diagnostic: " << world.count() << " people, "
              << actual_complex_actors << " complex actors, " << days << " days, p95 "
              << day_ms[static_cast<size_t>(percentile_index(day_ms.size(), 0.95))]
              << " ms; policy, renderer and save/load excluded\n";
    return money_conserved && invariant ? 0 : 1;
}
