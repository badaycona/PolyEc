// Focused manager parity/conservation/boundary tests for the extracted
// FinanceManager / HouseholdIndex / ProjectManager helpers. These exercise the
// helpers directly on local containers; they do NOT introduce a second
// production payroll — Realm::payroll delegates to FinanceManager::payroll and
// the full-realm behavior is covered by realm_tests.
#include "polyec/managers/employment_manager.hpp"
#include "polyec/managers/finance_manager.hpp"
#include "polyec/managers/household_index.hpp"
#include "polyec/managers/project_manager.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

using polyec::EmploymentManager;
using polyec::Family;
using polyec::FinanceManager;
using polyec::HouseholdIndex;
using polyec::PayrollPlan;
using polyec::Person;
using polyec::Project;
using polyec::ProjectManager;
using polyec::Province;

namespace {

// Opaque plan contract: callers must not be able to forge one; only
// FinanceManager::prepare (friend) can construct a plan.
static_assert(!std::is_default_constructible_v<PayrollPlan>,
              "PayrollPlan must not be default constructible by callers");

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

// R1 slice: tax may exceed the wage (assessed = pay*tax_rate/100); the gap is
// funded by the person's opening cash, with no implicit tax<=paid assumption.
void test_payroll_high_tax_funded_by_opening_cash() {
    World w;
    w.provinces[0].tax = 500; // 500% of wage
    w.people[0].cash = 100;   // opening cash funds tax above wage
    const int64_t before = w.money();
    std::vector<std::pair<uint64_t,int64_t>> claims{{1, 10}};
    FinanceManager::payroll(0, 0, false, claims, w.people, w.provinces);
    // paid 10; assessed = 10*500 = 5000; tax = 50; cash = 100+10-50 = 60.
    check(w.people[0].cash == 60, "tax above wage funded by opening cash");
    check(w.provinces[0].firm == 990, "firm debited by wage only");
    check(w.provinces[0].treasury == 1050, "treasury receives full assessed tax");
    check(w.provinces[0].tax_receipts == 50, "receipts count tax above wage");
    check(w.people[0].tax_remainder == 0, "remainder stays zero at exact multiple");
    check(w.money() == before, "high-tax payroll conserves total money");
}

void test_payroll_underfunded_tax_rejects_atomically() {
    World w;
    w.provinces[0].tax = 500;
    // person cash 0: after pay 10 the cash 10 < tax 50 -> reject before any write.
    const int64_t before = w.money();
    std::vector<std::pair<uint64_t,int64_t>> claims{{1, 10}};
    bool threw = false;
    try { FinanceManager::payroll(0, 0, false, claims, w.people, w.provinces); }
    catch (const std::logic_error &) { threw = true; }
    check(threw, "underfunded tax rejects");
    check(w.people[0].cash == 0 && w.provinces[0].firm == 1000 &&
          w.provinces[0].treasury == 1000 && w.provinces[0].tax_receipts == 0,
          "underfunded tax causes no mutation");
    check(w.money() == before, "underfunded tax reject conserves money");
}

// Same person twice: claims simulate sequentially, remainder carries between
// claims and no dedupe/ID priority is applied.
void test_duplicate_claim_accumulates_remainder() {
    World w;
    w.provinces[0].tax = 33;
    const int64_t before = w.money();
    std::vector<std::pair<uint64_t,int64_t>> claims{{1, 1}, {1, 1}};
    auto plan = FinanceManager::prepare(0, 0, false, claims, w.people, w.provinces);
    check(plan.claim_count() == 2 && plan.recipient_count() == 1,
          "duplicate claims keep two claims and one recipient");
    check(plan.claim_id(0) == 1 && plan.claim_id(1) == 1,
          "plan preserves duplicate claim ids in order");
    check(plan.paid(0) == 1 && plan.paid(1) == 1 && plan.tax(0) == 0 && plan.tax(1) == 0,
          "plan simulates each duplicate claim sequentially");
    FinanceManager::apply(plan, 0, w.people, w.provinces);
    // claim1: pay 1, assessed 33 -> tax 0, remainder 33, cash 1.
    // claim2: pay 1, assessed 33+33=66 -> tax 0, remainder 66, cash 2.
    check(w.people[0].cash == 2, "duplicate claims pay sequentially");
    check(w.people[0].tax_remainder == 66, "duplicate claims accumulate remainder");
    check(w.provinces[0].firm == 998, "firm debited once per duplicate claim");
    check(w.provinces[0].treasury == 1000 && w.provinces[0].tax_receipts == 0,
          "sub-cent tax stays in remainder, not treasury");
    check(w.money() == before, "duplicate-claim payroll conserves money");
}

void test_prepare_purity_and_double_apply() {
    World w;
    const std::vector<std::pair<uint64_t,int64_t>> claims{{1, 100}, {2, 100}};
    const int64_t firm0 = w.provinces[0].firm, tres0 = w.provinces[0].treasury;
    const int64_t cash0 = w.people[0].cash, cash1 = w.people[1].cash;
    auto plan = FinanceManager::prepare(0, 0, false, claims, w.people, w.provinces);
    check(!plan.noop(), "prepare returns a real plan for funded claims");
    check(plan.day() == 0 && plan.region() == 0 && !plan.public_employer(),
          "plan records payroll coordinates");
    check(plan.claim_count() == 2 && plan.recipient_count() == 2, "plan sizes");
    check(w.provinces[0].firm == firm0 && w.provinces[0].treasury == tres0 &&
          w.people[0].cash == cash0 && w.people[1].cash == cash1,
          "prepare mutates nothing");
    // Determinism: second prepare on unchanged inputs yields identical scalars.
    auto plan2 = FinanceManager::prepare(0, 0, false, claims, w.people, w.provinces);
    check(plan2.paid(0) == plan.paid(0) && plan2.paid(1) == plan.paid(1) &&
          plan2.tax(0) == plan.tax(0) && plan2.tax(1) == plan.tax(1) &&
          plan2.final_account() == plan.final_account() &&
          plan2.final_treasury() == plan.final_treasury() &&
          plan2.final_tax_receipts() == plan.final_tax_receipts(),
          "prepare is deterministic on unchanged inputs");
    // Independent scalar oracle: budget 200, paid 100 each, tax 10 each.
    check(plan.paid(0) == 100 && plan.tax(0) == 10 && plan.final_account() == 800 &&
          plan.final_treasury() == 1020 && plan.final_tax_receipts() == 20,
          "plan scalars match the private-firm oracle");
    FinanceManager::apply(plan, 0, w.people, w.provinces);
    check(w.people[0].cash == 90 && w.people[1].cash == 90 &&
          w.provinces[0].firm == 800 && w.provinces[0].treasury == 1020 &&
          w.provinces[0].tax_receipts == 20,
          "apply commits the prepared payroll");
    bool threw = false;
    try { FinanceManager::apply(plan, 0, w.people, w.provinces); }
    catch (const std::logic_error &) { threw = true; }
    check(threw, "double apply rejected as stale");
    check(w.people[0].cash == 90 && w.people[1].cash == 90 &&
          w.provinces[0].firm == 800 && w.provinces[0].treasury == 1020 &&
          w.provinces[0].tax_receipts == 20,
          "double apply causes no further mutation");
}

// apply() revalidates the plan's exact read set; any drift rejects with zero
// mutation beyond the drift itself.
void test_apply_stale_read_set_rejects() {
    const std::vector<std::pair<uint64_t,int64_t>> claim1{{1, 100}};
    // Stale day.
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces);
        bool threw = false;
        try { FinanceManager::apply(plan, /*current_day=*/1, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw && w.people[0].cash == 0 && w.provinces[0].firm == 1000,
              "stale day rejects without mutation");
    }
    // Tax rate changed after prepare.
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces);
        w.provinces[0].tax = 20; // deliberate external change
        const int64_t cash0 = w.people[0].cash, firm0 = w.provinces[0].firm;
        const int64_t tres0 = w.provinces[0].treasury, rec0 = w.provinces[0].tax_receipts;
        bool threw = false;
        try { FinanceManager::apply(plan, 0, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw && w.people[0].cash == cash0 && w.provinces[0].firm == firm0 &&
              w.provinces[0].treasury == tres0 && w.provinces[0].tax_receipts == rec0,
              "stale tax rejects without further mutation");
    }
    // Firm balance changed (private read set).
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces);
        w.provinces[0].firm = 999;
        const int64_t cash0 = w.people[0].cash;
        bool threw = false;
        try { FinanceManager::apply(plan, 0, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw && w.people[0].cash == cash0 && w.provinces[0].firm == 999,
              "stale firm account rejects without further mutation");
    }
    // Treasury changed (in the private read set as tax destination).
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces);
        w.provinces[0].treasury = 999;
        const int64_t cash0 = w.people[0].cash, firm0 = w.provinces[0].firm;
        bool threw = false;
        try { FinanceManager::apply(plan, 0, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw && w.people[0].cash == cash0 && w.provinces[0].firm == firm0 &&
              w.provinces[0].treasury == 999,
              "stale treasury rejects without further mutation");
    }
    // Canonical person id replaced.
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces);
        w.people[0].id = 99;
        const int64_t firm0 = w.provinces[0].firm;
        bool threw = false;
        try { FinanceManager::apply(plan, 0, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw && w.people[0].cash == 0 && w.provinces[0].firm == firm0,
              "stale person id rejects without mutation");
    }
    // Recipient cash drift: prepare snapshotted 0, live cash is 5.
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces);
        w.people[0].cash = 5;
        const int64_t firm0 = w.provinces[0].firm, tres0 = w.provinces[0].treasury;
        const int64_t rec0 = w.provinces[0].tax_receipts;
        bool threw = false;
        try { FinanceManager::apply(plan, 0, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw && w.people[0].cash == 5 && w.provinces[0].firm == firm0 &&
              w.provinces[0].treasury == tres0 && w.provinces[0].tax_receipts == rec0,
              "stale recipient cash rejects without additional mutation");
    }
    // Recipient tax-remainder drift.
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces);
        w.people[0].tax_remainder = 7;
        const int64_t cash0 = w.people[0].cash, firm0 = w.provinces[0].firm;
        const int64_t tres0 = w.provinces[0].treasury;
        bool threw = false;
        try { FinanceManager::apply(plan, 0, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw && w.people[0].cash == cash0 && w.people[0].tax_remainder == 7 &&
              w.provinces[0].firm == firm0 && w.provinces[0].treasury == tres0,
              "stale recipient remainder rejects without additional mutation");
    }
    // People vector shrunk below a recipient index.
    {
        World w;
        std::vector<std::pair<uint64_t,int64_t>> claim4{{4, 100}};
        auto plan = FinanceManager::prepare(0, 0, false, claim4, w.people, w.provinces);
        w.people.resize(2); // recipient index 3 no longer exists
        const int64_t firm0 = w.provinces[0].firm;
        bool threw = false;
        try { FinanceManager::apply(plan, 0, w.people, w.provinces); }
        catch (const std::out_of_range &) { threw = true; }
        check(threw && w.provinces[0].firm == firm0,
              "shrunk people vector rejects recipient without mutation");
    }
}

// apply() validates exactly the fields prepare() read: public payroll does not
// read firm, non-recipient people are outside the read set.
void test_apply_exact_read_set() {
    // Public plan ignores an unrelated firm change.
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, true, {{1, 100}}, w.people, w.provinces);
        w.provinces[0].firm = 555; // not in the public read set
        FinanceManager::apply(plan, 0, w.people, w.provinces);
        check(w.provinces[0].firm == 555, "public plan ignores unrelated firm change");
        check(w.provinces[0].treasury == 1000 - 100 + 10,
              "public apply commits net treasury");
        check(w.people[0].cash == 90, "public apply pays the recipient");
    }
    // Private plan is invalidated by a treasury change (tax destination).
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, {{1, 100}}, w.people, w.provinces);
        w.provinces[0].treasury = 42;
        bool threw = false;
        try { FinanceManager::apply(plan, 0, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw, "private plan invalidates on treasury change");
    }
    // People outside the claims stay untouched.
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, {{1, 100}}, w.people, w.provinces);
        w.people[2].cash = 77; // person 3 is not a recipient
        FinanceManager::apply(plan, 0, w.people, w.provinces);
        check(w.people[2].cash == 77, "non-recipient cash untouched");
        check(w.people[0].cash == 90, "recipient paid by exact-read-set apply");
    }
}

// New state-domain validation runs only on the due>0 path and rejects without
// mutation.
void test_prepare_state_validation() {
    const std::vector<std::pair<uint64_t,int64_t>> claim1{{1, 100}};
    {
        World w;
        w.provinces[0].tax = -1;
        bool threw = false;
        try { FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw, "negative tax rate rejects");
    }
    {
        World w;
        w.provinces[0].firm = -10;
        bool threw = false;
        try { FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw, "negative payroll account rejects");
    }
    {
        World w;
        w.people[0].cash = -1;
        bool threw = false;
        try { FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw, "negative person cash rejects");
    }
    {
        World w;
        w.people[0].tax_remainder = 100;
        bool threw = false;
        try { FinanceManager::prepare(0, 0, false, claim1, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw, "tax remainder above 99 rejects");
    }
    // Through the wrapper: rejects leave the world untouched.
    {
        World w;
        w.provinces[0].tax = -1;
        const int64_t before = w.money();
        bool threw = false;
        try { FinanceManager::payroll(0, 0, false, claim1, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw && w.money() == before, "state validation rejects without mutation");
    }
    // First read of a recipient requires its canonical 1-based id.
    {
        World w;
        w.people[0].id = 2; // vector index 0 claimed as person ID 1
        const int64_t before = w.money();
        bool threw = false;
        try { FinanceManager::payroll(0, 0, false, claim1, w.people, w.provinces); }
        catch (const std::logic_error &) { threw = true; }
        check(threw, "inconsistent canonical person id rejects");
        check(w.people[0].cash == 0 && w.provinces[0].firm == 1000 &&
              w.provinces[0].treasury == 1000 && w.provinces[0].tax_receipts == 0 &&
              w.money() == before,
              "inconsistent canonical person id rejects atomically");
    }
}

void test_payroll_overflow_rejects() {
    // Due sum overflows before any state read beyond validation.
    {
        World w;
        const int64_t max = std::numeric_limits<int64_t>::max();
        const int64_t before = w.money();
        std::vector<std::pair<uint64_t,int64_t>> claims{{1, max}, {2, max}};
        bool threw = false;
        try { FinanceManager::payroll(0, 0, false, claims, w.people, w.provinces); }
        catch (const std::overflow_error &) { threw = true; }
        check(threw, "due sum overflow throws overflow_error");
        check(w.people[0].cash == 0 && w.provinces[0].firm == 1000 &&
              w.money() == before,
              "due sum overflow causes no mutation");
    }
    // budget*claim overflows the pro-rata product: budget 8e9, claim 4e9.
    {
        World w;
        w.provinces[0].firm = 8'000'000'000LL;
        w.provinces[0].treasury = 0;
        w.provinces[0].tax = 0;
        const int64_t before = w.money();
        std::vector<std::pair<uint64_t,int64_t>> claims{
            {1, 4'000'000'000LL}, {2, 4'000'000'000LL}};
        bool threw = false;
        try { FinanceManager::payroll(0, 0, false, claims, w.people, w.provinces); }
        catch (const std::overflow_error &) { threw = true; }
        check(threw, "budget*claim overflow throws overflow_error");
        check(w.money() == before, "budget*claim overflow causes no mutation");
    }
    // Final recipient cash addition overflows: cash INT64_MAX, pay 1.
    // (Do not call World::money() here; summing INT64_MAX + 1 would overflow.)
    {
        World w;
        const int64_t max = std::numeric_limits<int64_t>::max();
        w.people[0].cash = max;
        w.provinces[0].firm = 1;
        w.provinces[0].treasury = 0;
        w.provinces[0].tax = 0;
        std::vector<std::pair<uint64_t,int64_t>> claims{{1, 1}};
        bool threw = false;
        try { FinanceManager::payroll(0, 0, false, claims, w.people, w.provinces); }
        catch (const std::overflow_error &) { threw = true; }
        check(threw, "final recipient cash addition overflow throws overflow_error");
        check(w.people[0].cash == max && w.provinces[0].firm == 1 &&
              w.provinces[0].treasury == 0 && w.provinces[0].tax_receipts == 0,
              "final cash addition overflow causes no mutation");
    }
    // tax_receipts addition overflows independently of treasury: receipts start
    // at INT64_MAX-10 while treasury starts at 0, so tres survives tax 11.
    {
        World w;
        const int64_t max = std::numeric_limits<int64_t>::max();
        w.provinces[0].firm = 110;
        w.provinces[0].treasury = 0;
        w.provinces[0].tax = 10;
        w.provinces[0].tax_receipts = max - 10;
        const int64_t before = w.money();
        std::vector<std::pair<uint64_t,int64_t>> claims{{1, 110}};
        bool threw = false;
        try { FinanceManager::payroll(0, 0, false, claims, w.people, w.provinces); }
        catch (const std::overflow_error &) { threw = true; }
        check(threw, "tax_receipts addition overflow throws overflow_error");
        check(w.people[0].cash == 0 && w.provinces[0].firm == 110 &&
              w.provinces[0].treasury == 0 && w.provinces[0].tax_receipts == max - 10 &&
              w.money() == before,
              "tax_receipts addition overflow causes no mutation");
    }
}

// Early-return order is part of the preserved contract: empty -> region ->
// per-claim id/negative validation -> due==0, then due>0 state validation.
void test_payroll_early_return_ordering() {
    // Empty claims: no-op even with an invalid region or invalid state.
    {
        World w;
        w.provinces[0].tax = -5; // state validation must not be reached
        std::vector<std::pair<uint64_t,int64_t>> empty;
        bool threw = false;
        try { FinanceManager::payroll(0, /*region=*/7, true, empty, w.people, w.provinces); }
        catch (...) { threw = true; }
        check(!threw, "empty claims skip region and state validation");
        auto plan = FinanceManager::prepare(0, 7, true, empty, w.people, w.provinces);
        check(plan.noop(), "empty-claims plan is noop");
        FinanceManager::apply(plan, /*current_day=*/999, w.people, w.provinces);
        check(w.people[0].cash == 0 && w.provinces[0].firm == 1000,
              "noop apply mutates nothing on any day");
    }
    // Invalid person ID throws even when the claim amount is zero.
    {
        World w;
        bool threw = false;
        try {
            std::vector<std::pair<uint64_t,int64_t>> claims{{99, 0}};
            FinanceManager::payroll(0, 0, true, claims, w.people, w.provinces);
        } catch (const std::out_of_range &) { threw = true; }
        check(threw, "invalid person ID throws even for a zero claim");
    }
    // Negative claim throws before the due sum.
    {
        World w;
        const int64_t before = w.money();
        bool threw = false;
        try {
            std::vector<std::pair<uint64_t,int64_t>> claims{{1, -5}};
            FinanceManager::payroll(0, 0, true, claims, w.people, w.provinces);
        } catch (const std::logic_error &) { threw = true; }
        check(threw && w.money() == before, "negative claim throws without mutation");
    }
    // Within a claim the person-ID check runs before the negative-amount check.
    {
        World w;
        bool out_of_range_hit = false, logic_hit = false;
        try {
            std::vector<std::pair<uint64_t,int64_t>> claims{{0, -5}};
            FinanceManager::payroll(0, 0, true, claims, w.people, w.provinces);
        } catch (const std::out_of_range &) { out_of_range_hit = true; }
        catch (const std::logic_error &) { logic_hit = true; }
        check(out_of_range_hit && !logic_hit,
              "person ID 0 throws out_of_range before the negative check");
    }
    // Valid zero-due claims: noop after all validation.
    {
        World w;
        auto plan = FinanceManager::prepare(0, 0, false, {{1, 0}, {2, 0}}, w.people, w.provinces);
        check(plan.noop(), "zero-due plan is noop");
        FinanceManager::apply(plan, 0, w.people, w.provinces);
        check(w.money() == 2000, "zero-due apply mutates nothing");
    }
}

// The thin payroll() wrapper must equal apply(prepare(...)) exactly.
void test_payroll_wrapper_matches_prepare_apply() {
    World a, b;
    const std::vector<std::pair<uint64_t,int64_t>> claims{{1, 70}, {2, 30}, {3, 100}};
    auto plan = FinanceManager::prepare(2, 0, false, claims, a.people, a.provinces);
    FinanceManager::apply(plan, 2, a.people, a.provinces);
    FinanceManager::payroll(2, 0, false, claims, b.people, b.provinces);
    bool same = a.money() == b.money();
    for (size_t i = 0; i < a.people.size(); ++i)
        same = same && a.people[i].cash == b.people[i].cash &&
               a.people[i].tax_remainder == b.people[i].tax_remainder;
    for (size_t i = 0; i < a.provinces.size(); ++i)
        same = same && a.provinces[i].firm == b.provinces[i].firm &&
               a.provinces[i].treasury == b.provinces[i].treasury &&
               a.provinces[i].tax_receipts == b.provinces[i].tax_receipts;
    check(same, "payroll wrapper equals prepare+apply on identical inputs");
    // Mixed-claim oracle: due 200, budget 200, paid full, tax 10% each.
    check(b.people[0].cash == 63 && b.people[1].cash == 27 && b.people[2].cash == 90,
          "mixed-claim wrapper pays net-of-tax wages");
    check(b.provinces[0].firm == 800 && b.provinces[0].treasury == 1020 &&
          b.provinces[0].tax_receipts == 20,
          "mixed-claim wrapper accounts firm/treasury/receipts");
}

void test_employment_boundaries() {
    World w;
    auto &v = w.provinces[0];
    v.jobs = 2;
    v.school = 40;
    v.treasury = 1000;

    auto &young = w.people[0];
    young.age_days = 13 * 365 + 364;
    young.skill = 100;
    auto &worker = w.people[1];
    worker.age_days = 18 * 365;
    worker.skill = 19;
    auto &teacher = w.people[2];
    teacher.age_days = 18 * 365;
    teacher.skill = 20;
    auto &governance = w.people[3];
    governance.age_days = 30 * 365;
    governance.family = Family::Governance;

    check(!EmploymentManager::can_start_work(young, v, 0, 30),
          "under-14 cannot start work");
    check(EmploymentManager::can_start_work(worker, v, 0, 30),
          "adult civilian can start work");
    check(!EmploymentManager::can_start_work(worker, v, 1, 30),
          "work target must match region");
    check(!EmploymentManager::can_start_work(worker, v, 0, 0),
          "zero work term rejected");
    check(!EmploymentManager::can_start_work(worker, v, 0, 31),
          "work term above 30 rejected");
    // Realm day is bounded by 365000 in saves, so this defensive boundary is
    // exercised directly rather than through an unreachable live fixture.
    check(!EmploymentManager::can_start_work(worker, v, 0, 1,
                                             std::numeric_limits<int>::max()),
          "work day overflow rejected by the pure preflight");
    check(EmploymentManager::can_start_work(worker, v, 0, 1,
                                            std::numeric_limits<int>::max() - 1),
          "work day max-1 plus one is representable");
    check(!EmploymentManager::can_start_work(worker, v, 0, 2,
                                             std::numeric_limits<int>::max() - 1),
          "work end overflow rejected by the pure preflight");
    check(!EmploymentManager::can_start_work(governance, v, 0, 30),
          "governance cannot take a work contract");
    check(!EmploymentManager::has_active_work(worker, 0),
          "idle person has no active work");

    v.teachers = 0;
    v.school = 0;
    check(!EmploymentManager::teacher_eligible(teacher, v),
          "zero school capacity has no teacher slot");
    v.school = 20;
    check(EmploymentManager::teacher_eligible(teacher, v),
          "school 20 has one teacher slot");
    v.teachers = 1;
    check(!EmploymentManager::teacher_eligible(teacher, v),
          "teacher capacity is one per 20 school slots");
    v.school = 40;
    v.teachers = 0;
    v.treasury = 1;
    check(!EmploymentManager::teacher_eligible(teacher, v),
          "teacher requires funded treasury");
    v.treasury = 2;
    check(EmploymentManager::teacher_eligible(teacher, v),
          "teacher slot is affordable at treasury 2");

    v.jobs = 2;
    v.employed = 0;
    v.teachers = 0;
    v.school = 40;
    v.treasury = 1000;
    EmploymentManager::start_work(worker, w.provinces, 0, 0, 30);
    check(worker.job == 0 && worker.job_kind == 0 && worker.busy_until == 30,
          "worker assignment fields");
    check(v.employed == 1 && v.teachers == 0, "worker counters");

    EmploymentManager::start_work(teacher, w.provinces, 0, 0, 30);
    check(teacher.job == 0 && teacher.job_kind == 1 && teacher.busy_until == 30,
          "teacher assignment fields");
    check(v.employed == 2 && v.teachers == 1, "teacher counters");
    check(EmploymentManager::has_active_work(teacher, 30) &&
          !EmploymentManager::has_active_work(teacher, 31),
          "busy_until is inclusive for active work");

    Person switcher = worker;
    switcher.age_days = 18 * 365;
    switcher.skill = 20;
    switcher.job = 0;
    switcher.job_kind = 0;
    switcher.busy_until = 5;
    v.employed = 1;
    v.teachers = 0;
    EmploymentManager::start_work(switcher, w.provinces, 6, 0, 30);
    check(switcher.job_kind == 1 && v.employed == 1 && v.teachers == 1,
          "worker-to-teacher switch clears and reaccounts counters");

    v.employed = 3;
    v.teachers = 2;
    Person expired;
    expired.job = 0;
    expired.job_kind = 1;
    expired.busy_until = 5;
    EmploymentManager::clear_work(expired, v);
    check(expired.job == -1 && v.employed == 2 && v.teachers == 1,
          "expired assignment still decrements legacy counters");
    v.employed = 0;
    v.teachers = 0;
    Person expired_again;
    expired_again.job = 0;
    expired_again.job_kind = 1;
    EmploymentManager::clear_work(expired_again, v);
    check(v.employed == 0 && v.teachers == 0,
          "legacy counter clear clamps at zero");

    v.jobs = 1;
    v.employed = 1;
    Person full;
    full.age_days = 18 * 365;
    full.skill = 10;
    check(!EmploymentManager::can_start_work(full, v, 0, 30),
          "full worker capacity rejects a new worker");
    full.job = 0;
    check(EmploymentManager::can_start_work(full, v, 0, 30),
          "an assigned worker may renew at full capacity");

    Person overflow;
    overflow.age_days = 18 * 365;
    const auto overflow_before = overflow;
    v.employed = 0;
    v.teachers = 0;
    bool threw = false;
    try {
        EmploymentManager::start_work(overflow, w.provinces,
                                      std::numeric_limits<int>::max(), 0, 1);
    } catch (const std::overflow_error &) { threw = true; }
    check(threw && overflow.job == overflow_before.job &&
          overflow.busy_until == overflow_before.busy_until &&
          v.employed == 0 && v.teachers == 0,
          "work day overflow rejects without mutation");
}

void test_employment_daily_helpers() {
    World w;
    auto &v = w.provinces[0];
    Person worker;
    worker.job = 0;
    worker.job_kind = 0;
    worker.busy_until = 10;
    worker.skill = 50;
    check(EmploymentManager::account_day(worker, v, 10) == 0 &&
          v.employed == 1 && v.teachers == 0,
          "day accounting counts an inclusive active worker");
    worker.job_kind = 1;
    worker.busy_until = 20;
    check(EmploymentManager::account_day(worker, v, 11) == 1 &&
          v.employed == 2 && v.teachers == 1 &&
          v.teacher_skill_sum == 50,
          "day accounting counts teacher skill");
    check(EmploymentManager::account_day(worker, v, 21) == -1 &&
          worker.job == -1 && v.employed == 2 && v.teachers == 1,
          "expired work is cleared without touching today's counters");
    v.employed = 9;
    v.teachers = 8;
    v.teacher_skill_sum = 77;
    EmploymentManager::reset_day_counters(w.provinces);
    check(v.employed == 0 && v.teachers == 0 && v.teacher_skill_sum == 0,
          "daily reset clears all employment counters");
}

void test_employment_appointment_and_office() {
    World w;
    auto &v = w.provinces[0];
    v.treasury = 1000;
    v.employed = 3;
    v.teachers = 2;
    auto &p = w.people[0];
    p.region = 0;
    p.cash = 0;
    p.job = 0;
    p.job_kind = 1;
    p.busy_until = 10;
    p.school_days = 4;
    p.candidate = true;
    const int64_t total = w.money();

    EmploymentManager::appoint(p, w.provinces, v.treasury, 1, 0);
    check(v.treasury == 980 && p.cash == 20 && w.money() == total,
          "appointment fee is conserved and paid");
    check(p.office && p.hired_by == 1 && p.family == Family::Governance &&
          !p.taxing_authority,
          "appointment installs a restricted office");
    check(p.job == -1 && p.busy_until == 0 && p.school_days == 0 &&
          !p.candidate,
          "appointment clears prior work and candidacy");
    check(p.salary_due == 30 && p.office_notice == 30,
          "appointment timers use day 0");
    check(v.employed == 2 && v.teachers == 1,
          "appointment clears the selected worker counters");
    check(!EmploymentManager::can_resign(p, 29) &&
          EmploymentManager::can_resign(p, 30),
          "resignation opens on the notice day");

    EmploymentManager::resign(p);
    check(!p.office && p.hired_by == 0 && p.family == Family::Civilian,
          "resignation ends the office");
    check(p.salary_due == 30 && p.office_notice == 30,
          "resignation leaves timers stale");

    p.office = true;
    p.hired_by = 1;
    p.family = Family::Governance;
    EmploymentManager::end_office(p);
    check(!p.office && p.hired_by == 0 && p.family == Family::Civilian,
          "failed salary uses the same office exit");
    check(p.salary_due == 30 && p.office_notice == 30,
          "failed salary leaves timers stale");

    Person q;
    q.region = 0;
    q.job = -1;
    const auto q_before = q;
    int64_t low = 19;
    const int employed_before = v.employed;
    const int teachers_before = v.teachers;
    bool threw = false;
    try { EmploymentManager::appoint(q, w.provinces, low, 1, 0); }
    catch (const std::logic_error &) { threw = true; }
    check(threw && q.cash == q_before.cash && !q.office &&
          q.hired_by == 0 && low == 19 &&
          v.employed == employed_before && v.teachers == teachers_before,
          "unfunded appointment rejects before mutation");

    Person rich;
    rich.region = 0;
    rich.cash = std::numeric_limits<int64_t>::max();
    const auto rich_before = rich;
    int64_t funded = 1000;
    threw = false;
    try { EmploymentManager::appoint(rich, w.provinces, funded, 1, 0); }
    catch (const std::overflow_error &) { threw = true; }
    check(threw && rich.cash == rich_before.cash && !rich.office &&
          rich.hired_by == 0 && funded == 1000,
          "appointment cash overflow rejects before mutation");

    Person late;
    late.region = 0;
    const auto late_before = late;
    int64_t day_funded = 1000;
    threw = false;
    try {
        EmploymentManager::appoint(late, w.provinces, day_funded, 1,
                                   std::numeric_limits<int>::max());
    } catch (const std::overflow_error &) { threw = true; }
    check(threw && late.cash == late_before.cash && !late.office &&
          late.hired_by == 0 && day_funded == 1000,
          "appointment day overflow rejects before mutation");

    Person idle;
    const auto idle_before = idle;
    threw = false;
    try { EmploymentManager::resign(idle); }
    catch (const std::logic_error &) { threw = true; }
    check(threw && idle.cash == idle_before.cash && !idle.office &&
          idle.hired_by == 0,
          "resignation without office rejects without mutation");
}

void test_employment_appointment_preflight() {
    World w;
    auto &v = w.provinces[0];
    v.treasury = 1000;
    Person p;
    p.region = 0;
    p.cash = 0;

    check(EmploymentManager::can_appoint(p, w.provinces, v.treasury, 1, 0),
          "funded in-range appointee is eligible");
    check(!EmploymentManager::can_appoint(p, w.provinces, 19, 1, 0),
          "appointment rejects treasury below fee");
    check(!EmploymentManager::can_appoint(p, w.provinces, v.treasury, 0, 0),
          "appointment rejects missing employer");
    check(!EmploymentManager::can_appoint(p, w.provinces, v.treasury, 1,
                                          std::numeric_limits<int>::max()),
          "appointment rejects day overflow");

    // A save fixture with cash above INT64_MAX-20 cannot satisfy Realm's
    // invariant: total_money() is a representable int64 sum, so this boundary
    // is tested directly on the pure preflight.
    p.cash = std::numeric_limits<int64_t>::max() - 10;
    check(!EmploymentManager::can_appoint(p, w.provinces, v.treasury, 1, 0),
          "appointment rejects appointee cash overflow");
    p.cash = std::numeric_limits<int64_t>::max() - 20;
    check(EmploymentManager::can_appoint(p, w.provinces, v.treasury, 1, 0),
          "appointee cash plus fee exactly fits");

    p.cash = 0;
    p.region = 9;
    check(!EmploymentManager::can_appoint(p, w.provinces, v.treasury, 1, 0),
          "appointment rejects out-of-range appointee region");
    p.region = 0;
    p.alive = false;
    check(!EmploymentManager::can_appoint(p, w.provinces, v.treasury, 1, 0),
          "appointment rejects deceased appointee");

    p.alive = true;
    p.cash = 0;
    check(EmploymentManager::can_appoint(
              p, w.provinces, v.treasury, 1,
              std::numeric_limits<int>::max() - 30),
          "appointment day plus notice exactly fits");
    check(!EmploymentManager::can_appoint(
              p, w.provinces, v.treasury, 1,
              std::numeric_limits<int>::max() - 29),
          "appointment day plus notice overflow rejected");
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
    test_payroll_high_tax_funded_by_opening_cash();
    test_payroll_underfunded_tax_rejects_atomically();
    test_duplicate_claim_accumulates_remainder();
    test_prepare_purity_and_double_apply();
    test_apply_stale_read_set_rejects();
    test_apply_exact_read_set();
    test_prepare_state_validation();
    test_payroll_overflow_rejects();
    test_payroll_early_return_ordering();
    test_payroll_wrapper_matches_prepare_apply();
    test_employment_boundaries();
    test_employment_daily_helpers();
    test_employment_appointment_and_office();
    test_employment_appointment_preflight();
    test_household_index();
    test_project_manager();
    if (failures) {
        std::fprintf(stderr, "%d manager test failure(s)\n", failures);
        return 1;
    }
    std::printf("manager tests passed\n");
    return 0;
}
