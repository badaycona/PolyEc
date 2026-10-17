// Focused manager parity/conservation/boundary tests for the extracted
// FinanceManager / HouseholdIndex / ProjectManager helpers. These exercise the
// helpers directly on local containers; they do NOT introduce a second
// production payroll — Realm::payroll delegates to FinanceManager::payroll and
// the full-realm behavior is covered by realm_tests.
#include "polyec/managers/finance_manager.hpp"
#include "polyec/managers/household_index.hpp"
#include "polyec/managers/project_manager.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

using polyec::FinanceManager;
using polyec::HouseholdIndex;
using polyec::Person;
using polyec::Project;
using polyec::ProjectManager;
using polyec::Province;

namespace {

int failures = 0;

void check(bool cond, const std::string &what) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", what.c_str());
        ++failures;
    }
}

struct World {
    std::vector<Person> people;
    std::vector<Province> provinces;
    World() : people(4), provinces(2) {
        for (size_t i = 0; i < people.size(); ++i) {
            people[i].id = i + 1;
            people[i].cash = 0;
            people[i].tax_remainder = 0;
        }
        provinces[0].treasury = 1000;
        provinces[0].firm = 1000;
        provinces[0].tax = 10;
        provinces[0].tax_receipts = 0;
        provinces[1].treasury = 0;
        provinces[1].firm = 0;
        provinces[1].tax = 0;
        provinces[1].tax_receipts = 0;
    }
    int64_t money() const {
        int64_t n = 0;
        for (const auto &p : people) n += p.cash;
        for (const auto &v : provinces) n += v.firm + v.treasury;
        return n;
    }
};

void test_transfer() {
    int64_t a = 10, b = 0;
    FinanceManager::transfer(a, b, 4);
    check(a == 6 && b == 4, "transfer moves funds");
    bool threw = false;
    try { FinanceManager::transfer(a, b, 100); } catch (const std::logic_error &) { threw = true; }
    check(threw && a == 6 && b == 4, "unfunded transfer throws without mutation");
    threw = false;
    try { FinanceManager::transfer(a, b, -1); } catch (const std::logic_error &) { threw = true; }
    check(threw && a == 6 && b == 4, "negative transfer throws without mutation");
}

void test_payroll_private_firm() {
    World w;
    std::vector<std::pair<uint64_t,int64_t>> claims{{1, 100}, {2, 100}};
    FinanceManager::payroll(/*day=*/0, /*region=*/0, /*public_employer=*/false, claims, w.people, w.provinces);
    // budget = min(firm=1000, due=200) = 200; each paid 100, then taxed 10.
    check(w.people[0].cash == 90 && w.people[1].cash == 90, "private payroll pays claims net of tax");
    check(w.provinces[0].firm == 800, "private payroll debits firm account");
    // tax: assessed = 100*10 + 0 = 1000; tax = 10 per person.
    check(w.provinces[0].treasury == 1020, "private payroll tax credited to treasury");
    check(w.provinces[0].tax_receipts == 20, "tax_receipts counts collected tax");
    check(w.people[0].tax_remainder == 0 && w.people[1].tax_remainder == 0, "remainder cleared at exact multiple");
    check(w.money() == 2000, "private payroll conserves total money");
}

void test_payroll_public_treasury() {
    World w;
    std::vector<std::pair<uint64_t,int64_t>> claims{{1, 100}, {2, 100}};
    FinanceManager::payroll(0, 0, /*public_employer=*/true, claims, w.people, w.provinces);
    check(w.provinces[0].treasury == 1000 - 200 + 20, "public payroll net treasury = -wages +tax");
    check(w.people[0].cash == 90 && w.people[1].cash == 90, "public payroll pays claims net of tax");
    check(w.money() == 2000, "public payroll conserves total money");
    check(w.provinces[0].tax_receipts == 20, "public payroll counts tax_receipts");
}

void test_day_based_rounding() {
    // Two claims, budget odd → one coin of remainder goes to a rotating index.
    auto run = [](int day) {
        World w;
        w.provinces[0].firm = 0;
        w.provinces[0].treasury = 3; // public budget = min(3, 5) = 3
        w.provinces[0].tax = 0;
        std::vector<std::pair<uint64_t,int64_t>> claims{{1, 2}, {2, 3}};
        FinanceManager::payroll(day, 0, true, claims, w.people, w.provinces);
        return std::pair<int64_t,int64_t>{w.people[0].cash, w.people[1].cash};
    };
    // due=5, budget=3: base paid = [1,1], left=1 → index (day+0)%2.
    auto d0 = run(0);
    auto d1 = run(1);
    check(d0.first == 2 && d0.second == 1, "day 0 remainder goes to claim offset 0");
    check(d1.first == 1 && d1.second == 2, "day 1 remainder goes to claim offset 1");
    // d0 and d1 must differ in WHO gets the extra coin when claim sizes allow:
    // claim0 max 2, claim1 max 3; base [1,1] left 1: day0 gives to claim0 → [2,1];
    // day1 gives to claim1 → [1,2].
    check(d0.first != d1.first || d0.second != d1.second,
          "rotating remainder phase depends on day, not region");
}

void test_partial_funds() {
    World w;
    w.provinces[0].firm = 50; // only 50 of 200 due
    w.provinces[0].treasury = 0;
    w.provinces[0].tax = 0;
    const int64_t before = w.money();
    std::vector<std::pair<uint64_t,int64_t>> claims{{1, 100}, {2, 100}};
    FinanceManager::payroll(0, 0, false, claims, w.people, w.provinces);
    check(w.people[0].cash + w.people[1].cash == 50, "partial funds pay only the budget");
    check(w.provinces[0].firm == 0, "partial funds drain the account");
    check(w.money() == before, "partial payroll conserves total money");
}

void test_empty_and_zero_total() {
    World w;
    const auto before = w.money();
    std::vector<std::pair<uint64_t,int64_t>> empty;
    FinanceManager::payroll(0, 0, true, empty, w.people, w.provinces);
    check(w.money() == before, "empty claims are a no-op");
    std::vector<std::pair<uint64_t,int64_t>> zeros{{1, 0}, {2, 0}};
    FinanceManager::payroll(0, 0, true, zeros, w.people, w.provinces);
    check(w.money() == before, "zero-total claims are a safe no-op (no div by zero)");
    check(w.people[0].cash == 0 && w.provinces[0].tax_receipts == 0, "zero-total claims mutate nothing");
}

void test_invalid_inputs_no_partial_mutation() {
    World w;
    const auto before = w.money();
    const auto receipts_before = w.provinces[0].tax_receipts;
    bool threw = false;
    try {
        std::vector<std::pair<uint64_t,int64_t>> claims{{1, 100}, {99, 100}};
        FinanceManager::payroll(0, 0, true, claims, w.people, w.provinces);
    } catch (const std::out_of_range &) { threw = true; }
    check(threw, "unknown person ID throws");
    check(w.money() == before && w.people[0].cash == 0 &&
          w.provinces[0].tax_receipts == receipts_before,
          "invalid person ID causes no partial mutation");

    threw = false;
    try {
        std::vector<std::pair<uint64_t,int64_t>> claims{{1, 100}};
        FinanceManager::payroll(0, /*region=*/7, true, claims, w.people, w.provinces);
    } catch (const std::out_of_range &) { threw = true; }
    check(threw && w.money() == before, "invalid region throws without mutation");

    threw = false;
    try {
        std::vector<std::pair<uint64_t,int64_t>> claims{{0, 100}};
        FinanceManager::payroll(0, 0, true, claims, w.people, w.provinces);
    } catch (const std::out_of_range &) { threw = true; }
    check(threw && w.money() == before, "zero person ID throws without mutation");
}

void test_household_index() {
    std::vector<Person> people(4);
    people[0].id = 1; people[0].household = 1;
    people[1].id = 2; people[1].household = 1;
    people[2].id = 3; people[2].household = 7;
    people[3].id = 4; people[3].household = 1;
    auto idx = HouseholdIndex::build(people);
    check(idx.size() == 2, "household index has two labels");
    check(idx[1] == std::vector<uint64_t>({1, 2, 4}), "members sorted ascending");
    check(idx[7] == std::vector<uint64_t>({3}), "single-member household");
    // Zero label must throw, never silently skip (parity with index_household).
    people[2].household = 0;
    bool threw = false;
    try { HouseholdIndex::build(people); } catch (const std::logic_error &) { threw = true; }
    check(threw, "zero household label throws (no silent skip)");
}

void test_project_manager() {
    std::vector<Project> projects(4);
    projects[0] = Project{}; projects[0].region = 1; projects[0].kind = 2; projects[0].completed = false;
    projects[1] = Project{}; projects[1].region = 1; projects[1].kind = 2; projects[1].completed = true;
    projects[2] = Project{}; projects[2].region = 1; projects[2].kind = 0; projects[2].completed = false;
    projects[3] = Project{}; projects[3].region = 2; projects[3].kind = 2; projects[3].completed = false;
    check(ProjectManager::active_projects_count(projects, 1, 2) == 1,
          "active count matches !completed && region && kind");
    check(ProjectManager::active_projects_count(projects, 1, 0) == 1, "kind filter exact");
    check(ProjectManager::active_projects_count(projects, 2, 2) == 1, "region filter exact");
    check(ProjectManager::active_projects_count(projects, 3, 2) == 0, "no active projects elsewhere");
    check(ProjectManager::active_projects_count({}, 1, 2) == 0, "empty projects is zero");
}

} // namespace

int main() {
    test_transfer();
    test_payroll_private_firm();
    test_payroll_public_treasury();
    test_day_based_rounding();
    test_partial_funds();
    test_empty_and_zero_total();
    test_invalid_inputs_no_partial_mutation();
    test_household_index();
    test_project_manager();
    if (failures) {
        std::fprintf(stderr, "%d manager test failure(s)\n", failures);
        return 1;
    }
    std::printf("manager tests passed\n");
    return 0;
}
