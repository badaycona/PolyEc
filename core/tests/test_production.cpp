#include "polyec/production_manager.hpp"

#include <array>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>

using namespace polyec;

namespace {
int checks = 0;
void check(bool ok, const char* label) {
    ++checks;
    if (!ok) throw std::runtime_error(label);
}

constexpr size_t good(GoodId id) { return static_cast<size_t>(id); }

Person make_person(uint64_t id, bool alive = true) {
    Person person;
    person.id = id;
    person.alive = alive;
    person.skill = 50;
    return person;
}

Establishment make_site(uint64_t id, EstablishmentIndustry industry, int capacity = 1) {
    Establishment site;
    site.id = id;
    site.region = 0;
    site.owner_kind = EstablishmentOwnerKind::Public;
    site.owner_id = 0;
    site.industry = industry;
    site.capacity = capacity;
    return site;
}

ProductionPlan plan(std::vector<Establishment>& sites,
                    std::map<WorkEstablishmentKey, int64_t>& attendance,
                    std::vector<Person>& people, int day) {
    std::map<WorkEstablishmentKey, int64_t> full_attention;
    std::map<WorkEstablishmentKey, int64_t> productivity;
    for(const auto&[key,ticks]:attendance){
        full_attention[key]=ticks*kAttentionUnitsPerEffortTick;
        productivity[key]=full_attention[key]*(150+people.at(size_t(key.first-1)).skill);
    }
    return ProductionManager::plan({sites, attendance, full_attention, productivity, people, day});
}

template<class Fn>
void rejects(Fn&& fn, const char* label) {
    bool rejected = false;
    try { fn(); }
    catch (const std::logic_error&) { rejected = true; }
    catch (const std::overflow_error&) { rejected = true; }
    check(rejected, label);
}
}

int main() {
    try {
        check(good_catalog_is_well_formed(), "canonical product catalog satisfies structural invariants");
        constexpr std::array<size_t, kGoodGroupCount> expected_group_sizes{
            1, 1, 1, 1, 1, 1, 3, 2, 2, 4, 1, 2, 3, 1, 1, 5};
        std::array<size_t, kGoodGroupCount> actual_group_sizes{};
        for (const auto& product : kGoodCatalog)
            ++actual_group_sizes[static_cast<size_t>(product.group)];
        check(actual_group_sizes == expected_group_sizes,
              "all 16 groups contain the declared 30 canonical products");

        // Three hand-calculated recipe oracles, using exact GoodId products.
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 10}, 16}};
            std::vector<Establishment> sites{make_site(10, EstablishmentIndustry::ClothWorkshop)};
            sites[0].stock[good(GoodId::Fiber)] = 2;
            auto result = plan(sites, attendance, people, 1);
            check(result.site_count() == 1 && result.site_id(0) == 10, "cloth site staged");
            check(result.site_batches(0) == 1, "cloth one batch");
            check(result.site_stock_delta(0, good(GoodId::Fiber)) == -1 &&
                  result.site_stock_delta(0, good(GoodId::Cloth)) == 1, "cloth exact recipe delta");
            check(result.consumed()[good(GoodId::Fiber)] == 1 &&
                  result.produced()[good(GoodId::Cloth)] == 1, "cloth exact ledgers");
            check(result.site_watermark_units(0, 1) == 1 &&
                  result.site_last_production_day(0) == 1, "cloth unit watermark and day");
        }
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 20}, 16}};
            std::vector<Establishment> sites{make_site(20, EstablishmentIndustry::ToolWorkshop)};
            sites[0].stock[good(GoodId::Iron)] = 1;
            sites[0].stock[good(GoodId::Fuel)] = 1;
            auto result = plan(sites, attendance, people, 1);
            check(result.site_batches(0) == 1 &&
                  result.site_stock_delta(0, good(GoodId::Iron)) == -1 &&
                  result.site_stock_delta(0, good(GoodId::Fuel)) == -1 &&
                  result.site_stock_delta(0, good(GoodId::Tools)) == 1, "tool exact recipe");
        }
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 30}, 16}};
            std::vector<Establishment> sites{make_site(30, EstablishmentIndustry::MilitarySupplyWorkshop)};
            sites[0].stock[good(GoodId::Tools)] = 1;
            sites[0].stock[good(GoodId::Cloth)] = 1;
            auto result = plan(sites, attendance, people, 1);
            check(result.site_batches(0) == 1 &&
                  result.site_stock_delta(0, good(GoodId::Tools)) == -1 &&
                  result.site_stock_delta(0, good(GoodId::Cloth)) == -1 &&
                  result.site_stock_delta(0, good(GoodId::MilitarySupply)) == 1,
                  "military supply exact recipe");
        }
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 31}, 16}};
            std::vector<Establishment> sites{make_site(31, EstablishmentIndustry::Forestry)};
            sites[0].region=6;
            sites[0].source_good=int(GoodId::Timber);
            sites[0].source_units=100;
            auto result=plan(sites,attendance,people,1);
            check(result.site_batches(0)==1,"forestry one full Work unit yields one batch");
            check(result.site_stock_delta(0,good(GoodId::Timber))==1,"forestry creates one Timber");
            check(result.site(0).source_units_delta==-1,"forestry consumes one source unit");
            check(result.produced()[good(GoodId::Timber)]==1&&
                  result.consumed()[good(GoodId::Timber)]==0,
                  "forest resources do not masquerade as a GoodId ledger input");
            sites[0].source_units=0;
            auto exhausted=plan(sites,attendance,people,1);
            check(exhausted.site_batches(0)==0&&exhausted.site(0).source_units_delta==0&&
                  exhausted.site_stock_delta(0,good(GoodId::Timber))==0,
                  "an exhausted forest cannot produce or debit negative resources");
        }
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 32}, 16}};
            std::vector<Establishment> sites{make_site(32, EstablishmentIndustry::PaperMill)};
            auto short_input = plan(sites, attendance, people, 1);
            check(short_input.site_batches(0) == 0 &&
                  short_input.site_stock_delta(0, good(GoodId::Timber)) == 0 &&
                  short_input.site_stock_delta(0, good(GoodId::Paper)) == 0,
                  "Paper Mill cannot produce Paper when its exact Timber input is missing");
            sites[0].stock[good(GoodId::Fiber)] = 1;
            sites[0].stock[good(GoodId::Silk)] = 1;
            auto substitute_only = plan(sites, attendance, people, 1);
            check(substitute_only.site_batches(0) == 0 &&
                  substitute_only.site_stock_delta(0, good(GoodId::Paper)) == 0,
                  "Fiber and Silk cannot substitute for Timber in the Paper recipe");
            sites[0].stock[good(GoodId::Timber)] = 1;
            auto supplied = plan(sites, attendance, people, 1);
            check(supplied.site_batches(0) == 1 &&
                  supplied.site_stock_delta(0, good(GoodId::Timber)) == -1 &&
                  supplied.site_stock_delta(0, good(GoodId::Paper)) == 1 &&
                  supplied.consumed()[good(GoodId::Timber)] == 1 &&
                  supplied.produced()[good(GoodId::Paper)] == 1,
                  "one full Work unit consumes one Timber and produces one Paper");
        }
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 33}, 16}};
            std::vector<Establishment> sites{make_site(33, EstablishmentIndustry::TeaGarden)};
            sites[0].region=14;sites[0].source_good=int(GoodId::Tea);sites[0].source_units=25;
            const auto supplied=plan(sites,attendance,people,1);
            check(supplied.site_batches(0)==1&&supplied.site(0).source_units_delta==-1&&
                  supplied.site_stock_delta(0,good(GoodId::Tea))==1&&
                  supplied.site_stock_delta(0,good(GoodId::Silk))==0&&
                  supplied.produced()[good(GoodId::Tea)]==1&&supplied.consumed()[good(GoodId::Tea)]==0,
                  "one Tea Garden Work unit consumes one harvest unit and creates Tea without Silk substitution");
            sites[0].source_units=0;
            const auto exhausted=plan(sites,attendance,people,1);
            check(exhausted.site_batches(0)==0&&exhausted.site(0).source_units_delta==0&&
                  exhausted.site_stock_delta(0,good(GoodId::Tea))==0,
                  "an exhausted Tea source cannot create output or debit below zero");
        }
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 34}, 16}};
            std::vector<Establishment> sites{make_site(34, EstablishmentIndustry::WarhorseStable)};
            sites[0].region=16;
            sites[0].stock[good(GoodId::Livestock)]=1;
            const auto wrong_input=plan(sites,attendance,people,1);
            check(wrong_input.site_batches(0)==0&&
                  wrong_input.site_stock_delta(0,good(GoodId::Livestock))==0&&
                  wrong_input.site_stock_delta(0,good(GoodId::WarHorse))==0,
                  "Livestock cannot replace the exact Horse input for Warhorse training");
            sites[0].stock[good(GoodId::Horse)]=1;
            const auto trained=plan(sites,attendance,people,1);
            check(trained.site_batches(0)==1&&
                  trained.site_stock_delta(0,good(GoodId::Horse))==-1&&
                  trained.site_stock_delta(0,good(GoodId::WarHorse))==1&&
                  trained.consumed()[good(GoodId::Horse)]==1&&
                  trained.produced()[good(GoodId::WarHorse)]==1,
                  "one exact Work unit consumes one Horse and creates one WarHorse");
        }
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 35}, 16}};
            std::vector<Establishment> sites{make_site(35, EstablishmentIndustry::WeaponForge)};
            sites[0].region=19;
            sites[0].stock[good(GoodId::Iron)]=1;
            sites[0].stock[good(GoodId::Tools)]=1;
            const auto supplied=plan(sites,attendance,people,1);
            check(supplied.site_batches(0)==1&&
                  supplied.site_stock_delta(0,good(GoodId::Iron))==-1&&
                  supplied.site_stock_delta(0,good(GoodId::Tools))==-1&&
                  supplied.site_stock_delta(0,good(GoodId::Weapon))==1,
                  "one Work unit consumes exactly one Iron and one Tool to create one Weapon");
            sites[0].stock[good(GoodId::Iron)]=0;
            sites[0].stock[good(GoodId::Copper)]=1;
            const auto substitute=plan(sites,attendance,people,1);
            check(substitute.site_batches(0)==0&&
                  substitute.site_stock_delta(0,good(GoodId::Copper))==0&&
                  substitute.site_stock_delta(0,good(GoodId::Tools))==0&&
                  substitute.site_stock_delta(0,good(GoodId::Weapon))==0,
                  "Copper cannot replace Iron even when the other Weapon input is present");
            sites[0].stock[good(GoodId::MilitarySupply)]=1;
            const auto outputSubstitute=plan(sites,attendance,people,1);
            check(outputSubstitute.site_batches(0)==0&&
                  outputSubstitute.site_stock_delta(0,good(GoodId::MilitarySupply))==0&&
                  outputSubstitute.site_stock_delta(0,good(GoodId::Weapon))==0,
                  "MilitarySupply cannot substitute for the distinct Weapon product or input recipe");
        }

        // A shared GoodGroup never makes a different GoodId a recipe input.
        const auto no_substitution = [&](EstablishmentIndustry industry, uint64_t site_id,
                                         GoodId substitute, GoodId companion, bool has_companion,
                                         GoodId output, const char* label) {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, site_id}, 16}};
            std::vector<Establishment> sites{make_site(site_id, industry)};
            sites[0].stock[good(substitute)] = 2;
            if (has_companion) sites[0].stock[good(companion)] = 2;
            const auto result = plan(sites, attendance, people, 1);
            check(result.site_batches(0) == 0 && result.site_watermark_units(0, 1) == 1, label);
            check(result.site_stock_delta(0, good(substitute)) == 0 &&
                  result.site_stock_delta(0, good(output)) == 0 &&
                  (!has_companion || result.site_stock_delta(0, good(companion)) == 0),
                  "failed recipe leaves substitute, companion and output unchanged");
            check(result.consumed()[good(substitute)] == 0 &&
                  result.produced()[good(output)] == 0,
                  "failed recipe does not record substitute consumption or output");
        };
        no_substitution(EstablishmentIndustry::ClothWorkshop, 101,
                        GoodId::Beeswax, GoodId::DryFood, false, GoodId::Cloth,
                        "Beeswax cannot replace Fiber in the cloth recipe");
        no_substitution(EstablishmentIndustry::ToolWorkshop, 102,
                        GoodId::Copper, GoodId::Fuel, true, GoodId::Tools,
                        "Copper cannot replace Iron in the tool recipe");
        no_substitution(EstablishmentIndustry::ToolWorkshop, 103,
                        GoodId::Silver, GoodId::Fuel, true, GoodId::Tools,
                        "Silver cannot replace Iron in the tool recipe");
        no_substitution(EstablishmentIndustry::ToolWorkshop, 104,
                        GoodId::Gold, GoodId::Fuel, true, GoodId::Tools,
                        "Gold cannot replace Iron in the tool recipe");
        no_substitution(EstablishmentIndustry::MilitarySupplyWorkshop, 105,
                        GoodId::Silk, GoodId::Tools, true, GoodId::MilitarySupply,
                        "Silk cannot replace Cloth in the military supply recipe");

        // No attendance and a partial unit do not produce or create a watermark.
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance;
            std::vector<Establishment> sites{make_site(40, EstablishmentIndustry::ClothWorkshop)};
            sites[0].stock[good(GoodId::Fiber)] = 1;
            check(plan(sites, attendance, people, 1).site_watermark_count(0) == 0,
                  "no attendance keeps empty watermark");
            attendance[{1, 40}] = 15;
            auto partial = plan(sites, attendance, people, 1);
            check(partial.site_batches(0) == 0 && partial.site_watermark_count(0) == 0,
                  "partial unit creates no work or watermark");
        }

        // Production now needs attention delivered while present, not just
        // physical attendance. Two half-rate shifts give one work unit; even
        // one missing attention unit keeps the watermark below that boundary.
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 41}, 32}};
            std::map<WorkEstablishmentKey, int64_t> effort{{{1, 41}, 16000}};
            std::vector<Establishment> sites{make_site(41, EstablishmentIndustry::ClothWorkshop)};
            sites[0].stock[good(GoodId::Fiber)] = 1;
            auto run=[&]{
                std::map<WorkEstablishmentKey,int64_t> productivity;
                for(const auto&[key,units]:effort)productivity[key]=units*(150+people.at(size_t(key.first-1)).skill);
                return ProductionManager::plan({sites,attendance,effort,productivity,people,1});
            };
            auto complete = run();
            check(complete.site_batches(0) == 1 && complete.site_watermark_units(0, 1) == 1,
                  "half attention over twice the presence earns one recipe unit");
            effort[{1, 41}] = 15999;
            auto below_threshold = run();
            check(below_threshold.site_batches(0) == 0 && below_threshold.site_watermark_count(0) == 0,
                  "attention just below one unit does not produce or advance watermark");
            effort.clear();
            auto absentAttention = run();
            check(absentAttention.site_batches(0) == 0 && absentAttention.site_watermark_count(0) == 0,
                  "full presence without delivered attention cannot produce");
            effort[{1, 41}] = 32001;
            rejects([&]{(void)run();},
                    "effort above physical-attendance attention cap rejects");
        }

        // Skill changes only the production conversion. The trial rule is
        // exact rational arithmetic: (150 + skill) / 200 per raw attention.
        {
            std::vector<Establishment> sites{make_site(42, EstablishmentIndustry::ClothWorkshop)};
            sites[0].stock[good(GoodId::Fiber)] = 1;
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 42}, 16}};
            people[0].skill = 50;
            check(plan(sites, attendance, people, 1).site_batches(0) == 1,
                  "skill 50 preserves one batch from sixteen full-attention ticks");
            people[0].skill = 0;
            attendance[{1, 42}] = 21;
            check(plan(sites, attendance, people, 1).site_batches(0) == 0,
                  "skill 0 has not reached one batch after twenty-one full-attention ticks");
            attendance[{1, 42}] = 22;
            check(plan(sites, attendance, people, 1).site_batches(0) == 1,
                  "skill 0 reaches one batch after twenty-two full-attention ticks");
            people[0].skill = 100;
            attendance[{1, 42}] = 12;
            check(plan(sites, attendance, people, 1).site_batches(0) == 0,
                  "skill 100 remains below one batch after twelve full-attention ticks");
            attendance[{1, 42}] = 13;
            check(plan(sites, attendance, people, 1).site_batches(0) == 1,
                  "skill 100 reaches one batch after thirteen full-attention ticks");
        }

        // Two workers yield two units, while capacity limits batches and both
        // actors' complete units are consumed from the daily opportunity.
        {
            std::vector<Person> people{make_person(1), make_person(2)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 50}, 16}, {{2, 50}, 16}};
            std::vector<Establishment> sites{make_site(50, EstablishmentIndustry::ClothWorkshop, 1)};
            sites[0].stock[good(GoodId::Fiber)] = 4;
            auto one = plan(sites, attendance, people, 1);
            check(one.site_batches(0) == 1 && one.site_watermark_units(0, 1) == 1 &&
                  one.site_watermark_units(0, 2) == 1, "capacity one consumes both actors' units");
            sites[0].capacity = 2;
            auto two = plan(sites, attendance, people, 1);
            check(two.site_batches(0) == 2 && two.produced()[good(GoodId::Cloth)] == 2,
                  "capacity two produces both batches");
        }

        // Shortage forfeits that day's unit; restocking alone cannot recover it,
        // while a new unit after restock can produce.
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 60}, 16}};
            std::vector<Establishment> sites{make_site(60, EstablishmentIndustry::ClothWorkshop)};
            auto short_day = plan(sites, attendance, people, 1);
            check(short_day.site_batches(0) == 0 && short_day.site_watermark_units(0, 1) == 1,
                  "shortage forfeits unit and advances watermark");
            sites[0].processed_work_units = short_day.site(0).watermark_final;
            sites[0].last_production_day = 1;
            sites[0].stock[good(GoodId::Fiber)] = 1;
            auto restock_only = plan(sites, attendance, people, 2);
            check(restock_only.site_batches(0) == 0 &&
                  restock_only.site_watermark_units(0, 1) == 1,
                  "restock alone does not repay forfeited work");
            attendance[{1, 60}] = 32;
            auto new_unit = plan(sites, attendance, people, 2);
            check(new_unit.site_batches(0) == 1 &&
                  new_unit.site_watermark_units(0, 1) == 2,
                  "new unit after restock produces");
        }

        // Marking an empty day retains prior watermarks. Repeating a processed
        // day is a true no-op and cannot delete or double count them.
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 70}, 16}};
            std::vector<Establishment> sites{make_site(70, EstablishmentIndustry::ClothWorkshop)};
            sites[0].stock[good(GoodId::Fiber)] = 1;
            sites[0].processed_work_units[1] = 1;
            sites[0].last_production_day = 1;
            auto empty_day = plan(sites, attendance, people, 2);
            check(empty_day.site_count() == 1 && empty_day.site_batches(0) == 0 &&
                  empty_day.site_watermark_units(0, 1) == 1,
                  "no-new-work day preserves prior watermark");
            sites[0].processed_work_units = empty_day.site(0).watermark_final;
            sites[0].last_production_day = 2;
            auto repeated = plan(sites, attendance, people, 2);
            check(repeated.noop() && repeated.site_count() == 0,
                  "repeated day has empty plan");
        }

        // Dead actors forfeit their eligible units by saved watermark without
        // contributing to output. Sites are ordered by stable ID in the plan.
        {
            std::vector<Person> people{make_person(1, false)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 90}, 16}};
            std::vector<Establishment> sites{make_site(90, EstablishmentIndustry::ClothWorkshop)};
            sites[0].stock[good(GoodId::Fiber)] = 1;
            auto dead = plan(sites, attendance, people, 1);
            check(dead.site_batches(0) == 0 && dead.site_watermark_units(0, 1) == 1,
                  "dead actor unit permanently forfeited");
            std::vector<Person> living{make_person(1), make_person(2)};
            std::vector<Establishment> unordered{make_site(92, EstablishmentIndustry::ClothWorkshop),
                                                 make_site(91, EstablishmentIndustry::ClothWorkshop)};
            std::map<WorkEstablishmentKey, int64_t> both{{{1, 91}, 16}, {{2, 92}, 16}};
            auto stable = plan(unordered, both, living, 1);
            check(stable.site_id(0) == 91 && stable.site_id(1) == 92,
                  "stable ascending site order");
        }
        {
            std::vector<Person> people{make_person(1), make_person(2)};
            people[1].alive = false;
            std::vector<Establishment> sites{make_site(92, EstablishmentIndustry::ClothWorkshop),
                                             make_site(91, EstablishmentIndustry::ClothWorkshop)};
            for(auto& site:sites)site.stock[good(GoodId::Fiber)]=1;
            std::map<WorkEstablishmentKey,int64_t> attendance{{{1,91},16},{{2,91},32},{{1,92},16}};
            const auto indexed=plan(sites,attendance,people,1);
            check(indexed.site_id(0)==91&&indexed.site_batches(0)==1&&
                  indexed.site_watermark_units(0,1)==1&&indexed.site_watermark_units(0,2)==2,
                  "site attendance index preserves actor order and dead-worker watermark");
            check(indexed.site_id(1)==92&&indexed.site_batches(1)==1&&
                  indexed.site_watermark_units(1,1)==1,
                  "site attendance index keeps records with different sites separate");
            attendance[{1,999}]=16;
            rejects([&]{(void)plan(sites,attendance,people,1);},"orphan site attendance rejects after indexed planning");
        }

        // Checked input validation and stock arithmetic fail before a plan can
        // be committed to any Realm state.
        {
            std::vector<Person> people{make_person(1)};
            std::map<WorkEstablishmentKey, int64_t> attendance{{{1, 100}, 16}};
            std::vector<Establishment> sites{make_site(100, EstablishmentIndustry::ClothWorkshop)};
            sites[0].stock[good(GoodId::Fiber)] = 1;
            sites[0].stock[good(GoodId::Cloth)] = std::numeric_limits<int64_t>::max();
            rejects([&] { (void)plan(sites, attendance, people, 1); }, "maximum output stock overflow rejects");
            sites[0].stock[good(GoodId::Cloth)] = 0;
            sites[0].processed_work_units[1] = 2;
            rejects([&] { (void)plan(sites, attendance, people, 1); }, "watermark above eligible attendance rejects");
            sites[0].processed_work_units.clear();
            sites[0].capacity = 0;
            rejects([&] { (void)plan(sites, attendance, people, 1); }, "invalid production capacity rejects");
            sites[0].capacity = 1;
            sites[0].stock[good(GoodId::Beeswax)] = -1;
            rejects([&] { (void)plan(sites, attendance, people, 1); }, "negative stock rejects");
            sites[0].stock[good(GoodId::Beeswax)] = 0;
            attendance[{2, 100}] = 16;
            rejects([&] { (void)plan(sites, attendance, people, 1); }, "invalid attendance actor rejects");
        }

        // Saltworks consumes only Fuel and produces only Salt. Its trial
        // quantities are read from the versioned world configuration.
        {
            std::vector<Person> people{make_person(1)};
            const std::map<WorkEstablishmentKey,int64_t> attendance{{{1,101},16}};
            const std::map<WorkEstablishmentKey,int64_t> effort{{{1,101},16*kAttentionUnitsPerEffortTick}};
            const std::map<WorkEstablishmentKey,int64_t> productivity{{{1,101},16*kAttentionUnitsPerEffortTick*200}};
            std::vector<Establishment> sites{make_site(101,EstablishmentIndustry::Saltworks)};
            sites[0].stock[good(GoodId::Fuel)]=1;
            auto trial=ProductionManager::plan({sites,attendance,effort,productivity,people,1,{}});
            check(trial.site_batches(0)==1&&trial.site_stock_delta(0,good(GoodId::Fuel))==-1&&
                  trial.site_stock_delta(0,good(GoodId::Salt))==1&&
                  trial.consumed()[good(GoodId::Fuel)]==1&&trial.produced()[good(GoodId::Salt)]==1,
                  "trial Saltworks recipe consumes one Fuel and yields one Salt");
            auto values=GameplayConfig{}.values();values[16]=2;values[17]=3;
            const auto configured=ProductionManager::plan({sites,attendance,effort,productivity,people,1,
                                                            GameplayConfig::from_values(values)});
            check(configured.site_batches(0)==0,"configured two-Fuel recipe waits when only one Fuel is present");
            sites[0].stock[good(GoodId::Fuel)]=2;
            const auto scaled=ProductionManager::plan({sites,attendance,effort,productivity,people,1,
                                                       GameplayConfig::from_values(values)});
            check(scaled.site_stock_delta(0,good(GoodId::Fuel))==-2&&
                  scaled.site_stock_delta(0,good(GoodId::Salt))==3,
                  "Salt recipe input and output follow the saved configuration");
            sites[0].stock[good(GoodId::Salt)]=std::numeric_limits<int64_t>::max()-2;
            try{(void)ProductionManager::plan({sites,attendance,effort,productivity,people,1,
                                               GameplayConfig::from_values(values)});
                check(false,"Salt output overflow accepted");}
            catch(const std::overflow_error&){
                check(sites[0].stock[good(GoodId::Fuel)]==2,
                      "Salt output overflow leaves live input stock untouched");
            }
        }

        // Atlas specialty recipes use exact output GoodIds, real Fuel where
        // specified, and a finite source ledger independent from inventory.
        {
            struct Specialty { EstablishmentIndustry industry; GoodId source; GoodId output; bool fuel; int region; };
            constexpr Specialty specialties[]{
                {EstablishmentIndustry::Glassworks,GoodId::PlainGlass,GoodId::PlainGlass,true,10},
                {EstablishmentIndustry::PotteryKiln,GoodId::Pottery,GoodId::Pottery,true,15},
                {EstablishmentIndustry::Tannery,GoodId::Leather,GoodId::Leather,false,17}};
            std::vector<Person> people{make_person(1)};
            for(size_t i=0;i<std::size(specialties);++i){
                const auto item=specialties[i];const uint64_t siteId=200+i;
                const std::map<WorkEstablishmentKey,int64_t> attendance{{{1,siteId},16}};
                const std::map<WorkEstablishmentKey,int64_t> effort{{{1,siteId},16*kAttentionUnitsPerEffortTick}};
                const std::map<WorkEstablishmentKey,int64_t> productivity{{{1,siteId},16*kAttentionUnitsPerEffortTick*200}};
                std::vector<Establishment> sites{make_site(siteId,item.industry)};
                sites[0].region=item.region;sites[0].source_good=int(item.source);sites[0].source_units=100;
                if(item.fuel){
                    const auto missingFuel=ProductionManager::plan({sites,attendance,effort,productivity,people,1,{}});
                    check(missingFuel.site_batches(0)==0&&missingFuel.site_stock_delta(0,good(item.output))==0&&
                          missingFuel.site(0).source_units_delta==0,
                          "an Atlas glass or pottery batch waits when real Fuel stock is missing");
                    sites[0].stock[good(GoodId::Fuel)]=1;
                }
                const auto supplied=ProductionManager::plan({sites,attendance,effort,productivity,people,1,{}});
                check(supplied.site_batches(0)==1&&supplied.site_stock_delta(0,good(item.output))==1&&
                      supplied.site(0).source_units_delta==-1&&
                      (!item.fuel||supplied.site_stock_delta(0,good(GoodId::Fuel))==-1),
                      "one exact Atlas specialty Work batch consumes its source and creates only its named product");
                sites[0].source_units=0;
                const auto exhausted=ProductionManager::plan({sites,attendance,effort,productivity,people,1,{}});
                check(exhausted.site_batches(0)==0&&exhausted.site_stock_delta(0,good(item.output))==0&&
                      exhausted.site(0).source_units_delta==0&&
                      (!item.fuel||exhausted.site_stock_delta(0,good(GoodId::Fuel))==0),
                      "an exhausted Atlas source cannot create goods or consume Fuel");
            }
        }

        std::cout << "production checks=" << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after " << checks << ": " << error.what() << '\n';
        return 1;
    }
}






