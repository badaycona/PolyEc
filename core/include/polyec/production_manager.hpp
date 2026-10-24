#pragma once
#include "polyec/establishment.hpp"
#include "polyec/realm.hpp"
#include "polyec/schedule.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {

// Amount of one recipe input/output good consumed or produced per batch.
// Recipe rows come from the Gate 4 product-source matrix: exact canonical
// GoodId inputs, no substitution across groups or deferred products.
struct ProductionGoodAmount { size_t good = 0; int64_t amount = 0; };

// Immutable recipe descriptor keyed by typed, validated industries. One output
// product per batch, up to three inventory goods and one optional site resource.
struct ProductionRecipe {
    EstablishmentIndustry industry = EstablishmentIndustry::ClothWorkshop;
    std::array<ProductionGoodAmount, 3> inputs{};
    size_t input_count = 0;
    size_t output_good = 0;
    int64_t output_amount = 1;
    int source_good = kNoEstablishmentSourceGood;
    int64_t source_units_per_batch = 0;
};

inline constexpr int64_t kProductionTicksPerUnit = kAttendanceTicksPerWorkUnit;
inline constexpr int64_t kProductionShortageForfeit = 1; // no labor debt or carryover
inline constexpr int64_t kLaborProductivityDenominator = 200;

inline constexpr std::array<ProductionRecipe, 23> kProductionRecipes{{
    {EstablishmentIndustry::ClothWorkshop, {{ProductionGoodAmount{9, 1}}}, 1, 11},
    {EstablishmentIndustry::ToolWorkshop, {{ProductionGoodAmount{5, 1}, ProductionGoodAmount{6, 1}, ProductionGoodAmount{4, 1}}}, 3, 16},
    {EstablishmentIndustry::MilitarySupplyWorkshop, {{ProductionGoodAmount{16, 1}, ProductionGoodAmount{11, 1}}}, 2, 17},
    {EstablishmentIndustry::Saltworks, {{ProductionGoodAmount{4, 1}}}, 1, 2},
    {EstablishmentIndustry::Forestry, {}, 0, 3, 1, int(GoodId::Timber), 1},
    {EstablishmentIndustry::Mining, {}, 0, 6, 1, int(GoodId::Copper), 1},
    {EstablishmentIndustry::Mining, {}, 0, 7, 1, int(GoodId::Silver), 1},
    {EstablishmentIndustry::PaperMill, {{ProductionGoodAmount{3, 1}}}, 1, 20},
    {EstablishmentIndustry::TeaGarden, {}, 0, 22, 1, int(GoodId::Tea), 1},
    {EstablishmentIndustry::IncenseWorks, {}, 0, 23, 1, int(GoodId::Incense), 1},
    {EstablishmentIndustry::WarhorseStable, {{ProductionGoodAmount{14, 1}}}, 1, 15},
    {EstablishmentIndustry::WeaponForge, {{ProductionGoodAmount{5, 1}, ProductionGoodAmount{16, 1}}}, 2, 18},
    {EstablishmentIndustry::Glassworks, {{ProductionGoodAmount{4, 1}}}, 1, size_t(GoodId::PlainGlass), 1, int(GoodId::PlainGlass), 1},
    {EstablishmentIndustry::PotteryKiln, {{ProductionGoodAmount{4, 1}}}, 1, size_t(GoodId::Pottery), 1, int(GoodId::Pottery), 1},
    {EstablishmentIndustry::Tannery, {}, 0, size_t(GoodId::Leather), 1, int(GoodId::Leather), 1},
    {EstablishmentIndustry::MedicinalGarden, {}, 0, size_t(GoodId::Medicine), 1, int(GoodId::Medicine), 1},
    {EstablishmentIndustry::SeedPressWorks, {}, 0, size_t(GoodId::SupplementaryFood), 1, int(GoodId::SupplementaryFood), 1},
    {EstablishmentIndustry::SawmillWorks, {{ProductionGoodAmount{3, 1}}}, 1, size_t(GoodId::ConstructionMaterial), 1},
    {EstablishmentIndustry::Apiary, {}, 0, size_t(GoodId::Beeswax), 1, int(GoodId::Beeswax), 1},
    {EstablishmentIndustry::SilkwormFarm, {}, 0, size_t(GoodId::Silk), 1, int(GoodId::Silk), 1},
    {EstablishmentIndustry::GoldMine, {}, 0, size_t(GoodId::Gold), 1, int(GoodId::Gold), 1},
    {EstablishmentIndustry::LivestockRanch, {}, 0, size_t(GoodId::Livestock), 1, int(GoodId::Livestock), 1},
    {EstablishmentIndustry::Butchery, {{ProductionGoodAmount{size_t(GoodId::Livestock), 1}}}, 1,
        size_t(GoodId::DryFood), 3},
}};

inline ProductionRecipe production_recipe(EstablishmentIndustry industry,const GameplayConfig& config={},
                                           int sourceGood=kNoEstablishmentSourceGood) {
    for (auto r : kProductionRecipes) if (r.industry == industry&&
        (r.source_good==sourceGood||(sourceGood==kNoEstablishmentSourceGood&&r.source_units_per_batch==0))) {
        const auto& v=config.values();
        switch(industry){
        case EstablishmentIndustry::ClothWorkshop:r.inputs[0].amount=v[3];r.output_amount=v[4];break;
        case EstablishmentIndustry::ToolWorkshop:r.inputs[0].amount=v[5];r.inputs[2].amount=v[6];r.output_amount=v[7];break;
        case EstablishmentIndustry::MilitarySupplyWorkshop:r.inputs[0].amount=v[8];r.inputs[1].amount=v[9];r.output_amount=v[10];break;
        case EstablishmentIndustry::Saltworks:r.inputs[0].amount=v[16];r.output_amount=v[17];break;
        case EstablishmentIndustry::Forestry:case EstablishmentIndustry::Mining:case EstablishmentIndustry::PaperMill:
        case EstablishmentIndustry::TeaGarden:case EstablishmentIndustry::IncenseWorks:case EstablishmentIndustry::WarhorseStable:case EstablishmentIndustry::WeaponForge:
        case EstablishmentIndustry::Glassworks:case EstablishmentIndustry::PotteryKiln:case EstablishmentIndustry::Tannery:
        case EstablishmentIndustry::MedicinalGarden:case EstablishmentIndustry::SeedPressWorks:
        case EstablishmentIndustry::Apiary:case EstablishmentIndustry::SilkwormFarm:case EstablishmentIndustry::GoldMine:
        case EstablishmentIndustry::LivestockRanch:case EstablishmentIndustry::Butchery:break;
        case EstablishmentIndustry::SawmillWorks:break;
        }
        return r;
    }
    throw std::logic_error("unknown production industry");
}

// Staged per-site production delta for one processed site. watermark_final is
// the complete post-commit sparse processed-work-unit map of that site (all
// entries positive; std::map keeps deterministic ascending actor order);
// stock_delta is a signed, nonnegative-final canonical GoodId count; batches
// is the number of complete recipe batches (possibly zero under shortage,
// which still forfeits the considered units through watermark_final).
struct EstablishmentProductionDelta {
    uint64_t id = 0;
    int64_t batches = 0;
    int64_t last_production_day = 0;
    std::array<int64_t, kGoodIdCount> stock_delta{};
    std::map<uint64_t, int64_t> watermark_final{};
    int64_t source_units_delta = 0;
};

// Immutable, opaque daily production plan. Only ProductionManager::plan can
// populate it (private fields, no public constructor), so the commit step in
// Realm may trust the staged writes after revalidating the exact read set
// against live state. Const accessors expose staged results for oracles/tests.
class ProductionPlan {
public:
    bool noop() const { return sites_.empty(); }
    int day() const { return day_; }
    size_t site_count() const { return sites_.size(); }
    uint64_t site_id(size_t i) const { return sites_.at(i).id; }
    int64_t site_batches(size_t i) const { return sites_.at(i).batches; }
    int64_t site_last_production_day(size_t i) const { return sites_.at(i).last_production_day; }
    int64_t site_stock_delta(size_t i, size_t good) const { return sites_.at(i).stock_delta.at(good); }
    size_t site_watermark_count(size_t i) const { return sites_.at(i).watermark_final.size(); }
    uint64_t site_watermark_actor(size_t i, size_t j) const {
        auto it = sites_.at(i).watermark_final.begin();
        std::advance(it, static_cast<std::ptrdiff_t>(j));
        return it->first;
    }
    int64_t site_watermark_units(size_t i, uint64_t actor) const {
        const auto found = sites_.at(i).watermark_final.find(actor);
        return found == sites_.at(i).watermark_final.end() ? 0 : found->second;
    }
    std::array<int64_t, kGoodIdCount> produced() const { return produced_; }
    std::array<int64_t, kGoodIdCount> consumed() const { return consumed_; }
    const EstablishmentProductionDelta& site(size_t i) const { return sites_.at(i); }

private:
    friend class ProductionManager;
    ProductionPlan() = default;

    int day_ = 0;
    std::vector<EstablishmentProductionDelta> sites_;
    std::array<int64_t, kGoodIdCount> produced_{};
    std::array<int64_t, kGoodIdCount> consumed_{};
};

// Stateless, read-only daily production planner for the three delegated starter
// industries. It owns no live state and no RNG: it reads the establishment
// vector, the site-qualified attendance map and per-actor liveness, and stages
// one complete, checked plan of marker/watermark/stock/ledger deltas that Realm
// validates and commits as a single transaction.
class ProductionManager {
public:
    struct Input {
        const std::vector<Establishment>& establishments;
        const std::map<WorkEstablishmentKey, int64_t>& site_attendance;
        const std::map<WorkEstablishmentKey, int64_t>& site_effort_attention;
        const std::map<WorkEstablishmentKey, int64_t>& site_productivity_attention_numerator;
        const std::vector<Person>& people;
        int day = 0;
        GameplayConfig config{};
    };

    // Build the staged plan. Throws std::logic_error/overflow_error on a
    // malformed world (unknown actor key, attendance below watermark, invalid
    // capacity) before any caller state is touched. Sites whose production day
    // marker already equals the requested day are reported skipped, so a
    // repeated production phase yields a no-op plan.
    static ProductionPlan plan(const Input& input) {
        ProductionPlan plan;
        plan.day_ = input.day;
        if (input.day < 0) throw std::logic_error("negative production day");
        // Full next processed-work-unit maps, staged before any live write.
        std::vector<std::map<uint64_t, int64_t>> next_watermarks;
        next_watermarks.reserve(input.establishments.size());
        for (const Establishment& site : input.establishments) {
            if (site.last_production_day < 0 || site.last_production_day > input.day)
                throw std::logic_error("invalid production day marker");
            next_watermarks.emplace_back(site.processed_work_units);
        }
        std::vector<std::optional<ProductionRecipe>> recipes(input.establishments.size());
        std::vector<int64_t> new_living_units(input.establishments.size(), 0);
        std::map<uint64_t, size_t> site_indices;
        // The source map is sorted by (actor, establishment), so this sparse
        // per-site index retains ascending actor order while avoiding a full
        // attendance scan for every establishment.
        std::map<uint64_t, std::vector<std::pair<uint64_t, int64_t>>> attendance_by_site;
        for (const auto& [key, ticks] : input.site_attendance)
            attendance_by_site[key.second].emplace_back(key.first, ticks);
        for (size_t i = 0; i < input.establishments.size(); ++i) {
            const Establishment& site = input.establishments[i];
            if (site.id == 0 || !site_indices.emplace(site.id, i).second)
                throw std::logic_error("invalid production site ID");
            if (site.last_production_day == input.day) continue;
            recipes[i] = production_recipe(site.industry,input.config,site.source_good);
            for (int64_t stock : site.stock)
                if (stock < 0) throw std::logic_error("negative production stock");
            const ProductionRecipe& sourceRecipe=*recipes[i];
            if(sourceRecipe.source_units_per_batch>0){
                if(site.source_good!=sourceRecipe.source_good||site.source_units<0||
                   site.source_units>kEstablishmentSourceUnitsMaximum)
                    throw std::logic_error("invalid establishment source resource");
            }else if(site.source_good!=kNoEstablishmentSourceGood||site.source_units!=0)
                throw std::logic_error("unexpected establishment source resource");
            for (const auto& [actor, prior_units] : site.processed_work_units) {
                if (actor == 0 || actor > input.people.size() || prior_units <= 0)
                    throw std::logic_error("invalid production watermark");
                const auto attendance = input.site_attendance.find({actor, site.id});
                const auto productivity = input.site_productivity_attention_numerator.find({actor, site.id});
                const int64_t attendance_ticks = attendance == input.site_attendance.end() ? 0 : attendance->second;
                const int64_t productivity_numerator = productivity == input.site_productivity_attention_numerator.end()
                    ? 0 : productivity->second;
                const int64_t divisor = kLaborProductivityDenominator * kAttentionUnitsPerEffortTick * input.config.ticks_per_unit();
                const int64_t eligible = productivity_numerator / divisor;
                if (prior_units > eligible)
                    throw std::logic_error("production watermark exceeds attendance");
            }
            const auto siteAttendance=attendance_by_site.find(site.id);
            if(siteAttendance!=attendance_by_site.end()){
                for (const auto& [actor, ticks] : siteAttendance->second) {
                    if (actor > input.people.size())
                        throw std::logic_error("invalid site attendance actor");
                    if (actor == 0 || ticks <= 0)
                        throw std::logic_error("invalid site attendance ticks");
                    const Person& person = input.people[actor - 1];
                    if (person.id != actor) throw std::logic_error("invalid site attendance person");
                    const auto productivity = input.site_productivity_attention_numerator.find({actor, site.id});
                    const int64_t productivity_numerator = productivity == input.site_productivity_attention_numerator.end()
                        ? 0 : productivity->second;
                    const int64_t divisor = kLaborProductivityDenominator * kAttentionUnitsPerEffortTick * input.config.ticks_per_unit();
                    const int64_t eligible = productivity_numerator / divisor;
                    int64_t watermark = 0;
                    const auto prior = next_watermarks[i].find(actor);
                    if (prior != next_watermarks[i].end()) watermark = prior->second;
                    if (watermark > eligible)
                        throw std::logic_error("production watermark exceeds attendance");
                    const int64_t units = eligible - watermark;
                    if (units <= 0) continue;
                    auto& staged = next_watermarks[i][actor];
                    if (units > std::numeric_limits<int64_t>::max() - staged)
                        throw std::overflow_error("production watermark overflow");
                    staged += units;
                    // Dead actors' accumulated units are advanced in the saved
                    // watermark, but never count toward a production batch.
                    if (person.alive) {
                        if (units > std::numeric_limits<int64_t>::max() - new_living_units[i])
                            throw std::overflow_error("production eligible units overflow");
                        new_living_units[i] += units;
                    }
                }
            }
        }
        // Every attendance record must resolve to a live Establishment. This
        // catches orphan keys even when a site has no production units.
        for (const auto& [key, ticks] : input.site_attendance) {
            const auto [actor, site_id] = key;
            if (actor == 0 || actor > input.people.size() || ticks <= 0 ||
                site_indices.find(site_id) == site_indices.end())
                throw std::logic_error("invalid site attendance record");
            if (input.people[actor - 1].id != actor)
                throw std::logic_error("invalid site attendance person");
        }
        for (const auto& [key, attention] : input.site_effort_attention) {
            const auto [actor, site_id] = key;
            const auto attendance = input.site_attendance.find(key);
            if (actor == 0 || actor > input.people.size() || attention <= 0 ||
                attendance == input.site_attendance.end() || attendance->second <= 0 ||
                site_indices.find(site_id) == site_indices.end() ||
                attendance->second > std::numeric_limits<int64_t>::max() / kAttentionUnitsPerEffortTick ||
                attention > attendance->second * kAttentionUnitsPerEffortTick)
                throw std::logic_error("invalid site effort attention record");
            if (input.people[actor - 1].id != actor)
                throw std::logic_error("invalid site effort attention person");
        }
        if (input.site_productivity_attention_numerator.size() != input.site_effort_attention.size())
            throw std::logic_error("site productivity attention coverage mismatch");
        for (const auto& [key, numerator] : input.site_productivity_attention_numerator) {
            const auto [actor, site_id] = key;
            const auto raw = input.site_effort_attention.find(key);
            if (actor == 0 || actor > input.people.size() || numerator <= 0 ||
                raw == input.site_effort_attention.end() || raw->second <= 0 ||
                raw->second > std::numeric_limits<int64_t>::max() / 250 ||
                numerator < raw->second * 150 || numerator > raw->second * 250 ||
                site_indices.find(site_id) == site_indices.end())
                throw std::logic_error("invalid site productivity attention record");
            if (input.people[actor - 1].id != actor)
                throw std::logic_error("invalid site productivity attention person");
        }
        // Batch arithmetic checked before any stock/ledger commit downstream:
        // batches = min(new eligible units, capacity, every required input / amount).
        for (size_t i = 0; i < input.establishments.size(); ++i) {
            if (!recipes[i]) continue;
            const Establishment& site = input.establishments[i];
            if (site.last_production_day == input.day) continue;
            const ProductionRecipe& recipe = *recipes[i];
            int64_t batches = 0;
            if (site.capacity <= 0 || site.capacity > kEstablishmentCapacityMax)
                throw std::logic_error("invalid production capacity");
            const int64_t eligible_units = new_living_units[i];
            if (eligible_units > 0) {
                if (site.capacity < eligible_units) batches = site.capacity;
                else batches = eligible_units;
                for (size_t r = 0; r < recipe.input_count; ++r) {
                    const int64_t amount = recipe.inputs[r].amount;
                    if (amount <= 0) throw std::logic_error("invalid production recipe input");
                    if (site.stock[recipe.inputs[r].good] < 0)
                        throw std::logic_error("negative production input stock");
                    const int64_t available = site.stock[recipe.inputs[r].good] / amount;
                    if (batches > available) batches = available;
                }
                if (recipe.source_units_per_batch < 0)
                    throw std::logic_error("invalid source resource recipe input");
                if (recipe.source_units_per_batch > 0) {
                    const int64_t available = site.source_units / recipe.source_units_per_batch;
                    if (batches > available) batches = available;
                }
                if (batches < 0) throw std::logic_error("negative production batch count");
            }
            // Mark each site once for this day and retain all prior watermarks,
            // even when there was no new work or inputs were short.
            EstablishmentProductionDelta delta{site.id, batches, input.day, {},
                                                std::move(next_watermarks[i])};
            if (batches > 0) {
                for (size_t r = 0; r < recipe.input_count; ++r) {
                    const size_t good = recipe.inputs[r].good;
                    if(batches>std::numeric_limits<int64_t>::max()/recipe.inputs[r].amount)
                        throw std::overflow_error("production input bound overflow");
                    const int64_t take = batches * recipe.inputs[r].amount;
                    if (site.stock[good] < take)
                        throw std::logic_error("production input shortage");
                    delta.stock_delta[good] -= take;
                    if (plan.consumed_[good] > std::numeric_limits<int64_t>::max() - take)
                        throw std::overflow_error("production consumed ledger overflow");
                    plan.consumed_[good] += take;
                }
                const size_t output = recipe.output_good;
                if(batches>std::numeric_limits<int64_t>::max()/recipe.output_amount)
                    throw std::overflow_error("production output bound overflow");
                const int64_t output_quantity=batches*recipe.output_amount;
                if (site.stock[output] > std::numeric_limits<int64_t>::max() - output_quantity)
                    throw std::overflow_error("production output stock overflow");
                delta.stock_delta[output] += output_quantity;
                if (plan.produced_[output] > std::numeric_limits<int64_t>::max() - output_quantity)
                    throw std::overflow_error("production produced ledger overflow");
                plan.produced_[output] += output_quantity;
                if (recipe.source_units_per_batch > 0) {
                    if (batches > std::numeric_limits<int64_t>::max() / recipe.source_units_per_batch)
                        throw std::overflow_error("source resource input bound overflow");
                    const int64_t used = batches * recipe.source_units_per_batch;
                    if (site.source_units < used) throw std::logic_error("site source resource shortage");
                    delta.source_units_delta = -used;
                }
            }
            plan.sites_.push_back(std::move(delta));
        }
        // Deterministic ascending stable site-ID order for validation and commit.
        std::sort(plan.sites_.begin(), plan.sites_.end(),
                  [](const EstablishmentProductionDelta& a, const EstablishmentProductionDelta& b) {
                      return a.id < b.id;
                  });
        return plan;
    }

private:
    ProductionManager() = delete;
};

} // namespace polyec
