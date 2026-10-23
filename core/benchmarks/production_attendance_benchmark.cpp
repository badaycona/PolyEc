#include "polyec/production_manager.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <limits>
#include <vector>

int main(int argc, char** argv) {
    using namespace polyec;
    const int iterations = argc > 1 ? std::max(1, std::stoi(argv[1])) : 9;
    constexpr size_t site_count = 1200;
    constexpr size_t workers_per_site = 8;

    std::vector<Person> people(workers_per_site);
    for (size_t i = 0; i < people.size(); ++i) {
        people[i].id = i + 1;
        people[i].alive = true;
        people[i].skill = 50;
    }
    std::vector<Establishment> sites(site_count);
    std::map<WorkEstablishmentKey, int64_t> attendance;
    std::map<WorkEstablishmentKey, int64_t> effort;
    std::map<WorkEstablishmentKey, int64_t> productivity;
    for (size_t i = 0; i < site_count; ++i) {
        Establishment& site = sites[i];
        site.id = i + 1;
        site.region = 0;
        site.owner_kind = EstablishmentOwnerKind::Public;
        site.owner_id = 0;
        site.industry = EstablishmentIndustry::ClothWorkshop;
        site.capacity = 1;
        site.stock[static_cast<size_t>(GoodId::Fiber)] = 1;
        for (size_t worker = 0; worker < workers_per_site; ++worker) {
            const WorkEstablishmentKey key{worker + 1, site.id};
            attendance.emplace(key, 16);
            effort.emplace(key, 16 * kAttentionUnitsPerEffortTick);
            productivity.emplace(key, 16 * kAttentionUnitsPerEffortTick * 200);
        }
    }

    std::vector<double> samples;
    samples.reserve(static_cast<size_t>(iterations));
    size_t planned_sites = 0;
    for (int i = 0; i < iterations + 1; ++i) {
        const auto start = std::chrono::steady_clock::now();
        const auto plan = ProductionManager::plan({sites, attendance, effort, productivity, people, 1, GameplayConfig{}});
        const auto elapsed = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - start).count();
        if (plan.site_count() != site_count || plan.site_batches(0) != 1)
            return 1;
        planned_sites = plan.site_count();
        if (i != 0) samples.push_back(elapsed);
    }
    std::sort(samples.begin(), samples.end());
    const double median = samples[samples.size() / 2];
    std::cout << "PROFILE production attendance indexing: sites=" << site_count
              << " attendance=" << attendance.size()
              << " workers_per_site=" << workers_per_site
              << " iterations=" << iterations
              << " median_ms=" << median
              << " sites_planned=" << planned_sites << '\n';
    return 0;
}
