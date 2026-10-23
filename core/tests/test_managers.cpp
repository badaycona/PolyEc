// Focused manager parity/conservation/boundary tests for the extracted
// FinanceManager / HouseholdIndex / ProjectManager helpers. These exercise the
// helpers directly on local containers; they do NOT introduce a second
// production payroll — Realm::payroll delegates to FinanceManager::payroll and
// the full-realm behavior is covered by realm_tests.
#include "polyec/managers/employment_manager.hpp"
#include "polyec/managers/estate_manager.hpp"
#include "polyec/estate_section_codec.hpp"
#include "polyec/managers/finance_manager.hpp"
#include "polyec/managers/household_index.hpp"
#include "polyec/managers/peerage_succession_manager.hpp"
#include "polyec/managers/project_manager.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

using polyec::EmploymentManager;
using polyec::EstateManager;
using polyec::EstateDeathPlan;
using polyec::EstateBeneficiaryTier;
using polyec::EstateCase;
using polyec::Edge;
using polyec::Family;
using polyec::Establishment;
using polyec::EstablishmentOwnerKind;
using polyec::LandHolding;
using polyec::LandUseRight;
using polyec::Loan;
using polyec::FinanceManager;
using polyec::HouseholdIndex;
using polyec::PayrollBatchGroup;
using polyec::PayrollBatchPlan;
using polyec::PayrollPlan;
using polyec::Person;
using polyec::Project;
using polyec::ProjectManager;
using polyec::Province;
using polyec::RelationKind;

namespace {

// Opaque plan contract: callers must not be able to forge one; only
// FinanceManager::prepare (friend) can construct a plan.
static_assert(!std::is_default_constructible_v<PayrollPlan>,
              "PayrollPlan must not be default constructible by callers");
static_assert(!std::is_default_constructible_v<PayrollBatchPlan>,
              "PayrollBatchPlan must not be default constructible by callers");
static_assert(!std::is_default_constructible_v<EstateDeathPlan>,
              "EstateDeathPlan must not be default constructible by callers");

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
    a=5;b=std::numeric_limits<int64_t>::max()-1;threw=false;
    try{FinanceManager::transfer(a,b,2);}catch(const std::overflow_error&){threw=true;}
    check(threw&&a==5&&b==std::numeric_limits<int64_t>::max()-1,"destination overflow rejects before debiting source");
    b=-1;threw=false;
    try{FinanceManager::transfer(a,b,1);}catch(const std::logic_error&){threw=true;}
    check(threw&&a==5&&b==-1,"negative destination rejects without mutation");
    a=std::numeric_limits<int64_t>::max();
    FinanceManager::transfer(a,a,1);
    check(a==std::numeric_limits<int64_t>::max(),"self-transfer at maximum balance is a no-op");
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

// A staged batch must simulate every group against one overlay, in order.
void test_batch_matches_sequential_groups() {
    World staged, sequential;
    std::vector<PayrollBatchGroup> groups{
        {0, true, {{1, 120}, {2, 80}}},
        {0, true, {{1, 60}, {3, 40}}},
        {1, false, {{4, 30}}},
    };
    auto plan = FinanceManager::prepare_batch(3, groups, staged.people, staged.provinces);
    check(plan.claim_count() == 5, "batch claim count matches concatenated groups");
    check(plan.claim_id(0) == 1 && plan.claim_region(0) == 0 && plan.claim_public(0),
          "batch claim accessors preserve group order");
    check(plan.claim_id(4) == 4 && plan.claim_region(4) == 1 && !plan.claim_public(4),
          "batch final claim accessors");
    FinanceManager::apply_batch(plan, 3, staged.people, staged.provinces);
    for (const auto &group : groups)
        FinanceManager::payroll(3, group.region, group.public_employer, group.claims,
                                sequential.people, sequential.provinces);
    bool same = staged.money() == sequential.money();
    for (size_t i = 0; i < staged.people.size(); ++i)
        same = same && staged.people[i].cash == sequential.people[i].cash &&
               staged.people[i].tax_remainder == sequential.people[i].tax_remainder;
    for (size_t i = 0; i < staged.provinces.size(); ++i)
        same = same && staged.provinces[i].firm == sequential.provinces[i].firm &&
               staged.provinces[i].treasury == sequential.provinces[i].treasury &&
               staged.provinces[i].tax_receipts == sequential.provinces[i].tax_receipts;
    check(same, "batch overlay equals sequential prepare+apply exactly");
    check(staged.money() == 2000, "batch conserves total money");
}

void test_batch_shared_underfunding_pro_rata() {
    World staged;
    staged.provinces[0].firm = 30;
    std::vector<PayrollBatchGroup> groups{
        {0, false, {{1, 20}}},
        {0, false, {{2, 20}}},
    };
    auto plan = FinanceManager::prepare_batch(0, groups, staged.people, staged.provinces);
    check(plan.paid(0) == 20 && plan.paid(1) == 10,
          "later underfunded group sees the running overlay balance");
    FinanceManager::apply_batch(plan, 0, staged.people, staged.provinces);
    check(staged.people[0].cash == 18 && staged.people[1].cash == 9 &&
          staged.provinces[0].firm == 0,
          "batch underfunding is a successful net payment");
    check(staged.money() == 1030, "batch underfunding conserves money");
}

void test_batch_apply_is_atomic_after_stale_read() {
    World w;
    std::vector<PayrollBatchGroup> groups{
        {0, true, {{1, 20}}},
        {0, true, {{2, 20}}},
    };
    auto plan = FinanceManager::prepare_batch(0, groups, w.people, w.provinces);
    const auto people_before = w.people;
    const auto provinces_before = w.provinces;
    w.people[1].cash = 7; // stale second recipient
    const auto stale_people = w.people;
    const auto stale_provinces = w.provinces;
    bool threw = false;
    try {
        FinanceManager::apply_batch(plan, 0, w.people, w.provinces);
    } catch (const std::logic_error &) {
        threw = true;
    }
    bool unchanged = threw && w.people.size() == stale_people.size() &&
                     w.provinces.size() == stale_provinces.size();
    for (size_t i = 0; unchanged && i < w.people.size(); ++i)
        unchanged = w.people[i].cash == stale_people[i].cash &&
                    w.people[i].tax_remainder == stale_people[i].tax_remainder;
    for (size_t i = 0; unchanged && i < w.provinces.size(); ++i)
        unchanged = w.provinces[i].firm == stale_provinces[i].firm &&
                    w.provinces[i].treasury == stale_provinces[i].treasury &&
                    w.provinces[i].tax_receipts == stale_provinces[i].tax_receipts;
    check(unchanged, "stale batch read rejects without writing any group");
    check(people_before[0].cash == 0 && provinces_before[0].treasury == 1000,
          "batch prepare itself mutated nothing");
}

void test_batch_prepare_failure_leaves_inputs_unchanged() {
    World w;
    const auto people_before = w.people;
    const auto provinces_before = w.provinces;
    std::vector<PayrollBatchGroup> groups{
        {0, false, {{1, 20}}},
        {0, false, {{9999, 20}}},
    };
    bool threw = false;
    try {
        FinanceManager::prepare_batch(0, groups, w.people, w.provinces);
    } catch (const std::out_of_range &) {
        threw = true;
    }
    bool unchanged = threw && w.people.size() == people_before.size() &&
                     w.provinces.size() == provinces_before.size();
    for (size_t i = 0; unchanged && i < w.people.size(); ++i)
        unchanged = w.people[i].cash == people_before[i].cash;
    for (size_t i = 0; unchanged && i < w.provinces.size(); ++i)
        unchanged = w.provinces[i].firm == provinces_before[i].firm &&
                    w.provinces[i].treasury == provinces_before[i].treasury;
    check(unchanged, "second-group prepare failure leaves every live input unchanged");
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
    check(worker.work_assignment.job == 0 && worker.work_assignment.job_kind == 0 && worker.busy_until == 30,
          "worker assignment fields");
    check(v.employed == 1 && v.teachers == 0, "worker counters");

    EmploymentManager::start_work(teacher, w.provinces, 0, 0, 30);
    check(teacher.work_assignment.job == 0 && teacher.work_assignment.job_kind == 1 && teacher.busy_until == 30,
          "teacher assignment fields");
    check(v.employed == 2 && v.teachers == 1, "teacher counters");
    check(EmploymentManager::has_active_work(teacher, 30) &&
          !EmploymentManager::has_active_work(teacher, 31),
          "busy_until is inclusive for active work");

    Person switcher = worker;
    switcher.age_days = 18 * 365;
    switcher.skill = 20;
    switcher.work_assignment.job = 0;
    switcher.work_assignment.job_kind = 0;
    switcher.busy_until = 5;
    v.employed = 1;
    v.teachers = 0;
    EmploymentManager::start_work(switcher, w.provinces, 6, 0, 30);
    check(switcher.work_assignment.job_kind == 1 && v.employed == 1 && v.teachers == 1,
          "worker-to-teacher switch clears and reaccounts counters");

    v.employed = 3;
    v.teachers = 2;
    Person expired;
    expired.work_assignment.job = 0;
    expired.work_assignment.job_kind = 1;
    expired.busy_until = 5;
    EmploymentManager::clear_work(expired, v);
    check(expired.work_assignment.job == -1 && v.employed == 2 && v.teachers == 1,
          "expired assignment still decrements legacy counters");
    v.employed = 0;
    v.teachers = 0;
    Person expired_again;
    expired_again.work_assignment.job = 0;
    expired_again.work_assignment.job_kind = 1;
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
    full.work_assignment.job = 0;
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
    check(threw && overflow.work_assignment.job == overflow_before.work_assignment.job &&
          overflow.busy_until == overflow_before.busy_until &&
          v.employed == 0 && v.teachers == 0,
          "work day overflow rejects without mutation");
}

void test_employment_daily_helpers() {
    World w;
    auto &v = w.provinces[0];
    Person worker;
    worker.work_assignment.job = 0;
    worker.work_assignment.job_kind = 0;
    worker.busy_until = 10;
    worker.skill = 50;
    check(EmploymentManager::account_day(worker, v, 10) == 0 &&
          v.employed == 1 && v.teachers == 0,
          "day accounting counts an inclusive active worker");
    worker.work_assignment.job_kind = 1;
    worker.busy_until = 20;
    check(EmploymentManager::account_day(worker, v, 11) == 1 &&
          v.employed == 2 && v.teachers == 1 &&
          v.teacher_skill_sum == 50,
          "day accounting counts teacher skill");
    check(EmploymentManager::account_day(worker, v, 21) == -1 &&
          worker.work_assignment.job == -1 && v.employed == 2 && v.teachers == 1,
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
    p.work_assignment.job = 0;
    p.work_assignment.job_kind = 1;
    p.busy_until = 10;
    p.school_days = 4;
    p.candidate = true;
    const int64_t total = w.money();

    EmploymentManager::appoint(p, w.provinces, v.treasury, 1, 0);
    check(v.treasury == 980 && p.cash == 20 && w.money() == total,
          "appointment fee is conserved and paid");
    check(p.office_appointment.office && p.office_appointment.hired_by == 1 && p.family == Family::Governance &&
           !p.taxing_authority,
          "appointment installs a restricted office");
    check(p.work_assignment.job == -1 && p.busy_until == 0 && p.school_days == 0 &&
          !p.candidate,
          "appointment clears prior work and candidacy");
    check(p.office_appointment.salary_due == 30 && p.office_appointment.office_notice == 30,
          "appointment timers use day 0");
    check(v.employed == 2 && v.teachers == 1,
          "appointment clears the selected worker counters");
    check(!EmploymentManager::can_resign(p, 29) &&
          EmploymentManager::can_resign(p, 30),
          "resignation opens on the notice day");

    EmploymentManager::resign(p);
    check(!p.office_appointment.office && p.office_appointment.hired_by == 0 && p.family == Family::Civilian,
          "resignation ends the office");
    check(p.office_appointment.salary_due == 30 && p.office_appointment.office_notice == 30,
          "resignation leaves timers stale");

    p.office_appointment.office = true;
    p.office_appointment.hired_by = 1;
    p.family = Family::Governance;
    EmploymentManager::end_office(p);
    check(!p.office_appointment.office && p.office_appointment.hired_by == 0 && p.family == Family::Civilian,
          "failed salary uses the same office exit");
    check(p.office_appointment.salary_due == 30 && p.office_appointment.office_notice == 30,
          "failed salary leaves timers stale");

    Person q;
    q.region = 0;
    q.work_assignment.job = -1;
    const auto q_before = q;
    int64_t low = 19;
    const int employed_before = v.employed;
    const int teachers_before = v.teachers;
    bool threw = false;
    try { EmploymentManager::appoint(q, w.provinces, low, 1, 0); }
    catch (const std::logic_error &) { threw = true; }
    check(threw && q.cash == q_before.cash && !q.office_appointment.office &&
          q.office_appointment.hired_by == 0 && low == 19 &&
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
    check(threw && rich.cash == rich_before.cash && !rich.office_appointment.office &&
          rich.office_appointment.hired_by == 0 && funded == 1000,
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
    check(threw && late.cash == late_before.cash && !late.office_appointment.office &&
          late.office_appointment.hired_by == 0 && day_funded == 1000,
          "appointment day overflow rejects before mutation");

    Person idle;
    const auto idle_before = idle;
    threw = false;
    try { EmploymentManager::resign(idle); }
    catch (const std::logic_error &) { threw = true; }
    check(threw && idle.cash == idle_before.cash && !idle.office_appointment.office &&
          idle.office_appointment.hired_by == 0,
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
    people[0].id = 1; people[0].household_membership.household_id = 1;
    people[1].id = 2; people[1].household_membership.household_id = 1;
    people[2].id = 3; people[2].household_membership.household_id = 7;
    people[3].id = 4; people[3].household_membership.household_id = 1;
    auto idx = HouseholdIndex::build(people);
    check(idx.size() == 2, "household index has two labels");
    check(idx[1] == std::vector<uint64_t>({1, 2, 4}), "members sorted ascending");
    check(idx[7] == std::vector<uint64_t>({3}), "single-member household");
    // Zero label must throw, never silently skip (parity with index_household).
    people[2].household_membership.household_id = 0;
    bool threw = false;
    try { HouseholdIndex::build(people); } catch (const std::logic_error &) { threw = true; }
    check(threw, "zero household label throws (no silent skip)");

    std::vector<Person> family(52);
    for (size_t i = 0; i < family.size(); ++i) {
        family[i].id = i + 1;
        family[i].household_membership.household_id = 15;
    }
    family[14].age_days = 17 * 365; // Household head is not yet an adult.
    family[15].age_days = 5 * 365;  // Minor whose derived guardian is queried.
    family[16].age_days = 40 * 365;
    family[51].age_days = 40 * 365;
    const std::unordered_map<uint64_t, std::vector<uint64_t>> family_index{{15, {15, 16, 17, 52}}};
    check(HouseholdIndex::guardian(family, family_index, family[15]) == 17,
          "equal-age guardian tie selects the lowest member ID");
    check(HouseholdIndex::authority(family, family_index, 15) == 17,
          "equal-age authority tie selects the lowest member ID");
    family[16].alive = false;
    check(HouseholdIndex::guardian(family, family_index, family[15]) == 52 &&
              HouseholdIndex::authority(family, family_index, 15) == 52,
          "guardian and authority reselect the next living adult");
    family[51].alive = false;
    check(HouseholdIndex::guardian(family, family_index, family[15]) == 0 &&
              HouseholdIndex::authority(family, family_index, 15) == 0,
          "household with no living adult has no derived guardian or authority");
    family[14].age_days = 18 * 365;
    check(HouseholdIndex::guardian(family, family_index, family[15]) == 15 &&
              HouseholdIndex::authority(family, family_index, 15) == 15,
          "adult household head becomes authority and can guard a minor");
    family[15].age_days = 18 * 365;
    check(HouseholdIndex::guardian(family, family_index, family[15]) == 0,
          "adult does not have a derived guardian");
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

std::vector<Establishment> payroll_sites(int64_t first=100,int64_t second=50){
    std::vector<Establishment> sites(2);
    sites[0].id=10;sites[0].owner_id=1;sites[0].cash=first;
    sites[1].id=20;sites[1].owner_id=2;sites[1].cash=second;
    return sites;
}

void test_site_payroll_hand_calculated(){
    World w;w.provinces[0].treasury=100;auto sites=payroll_sites();
    const int64_t opening=w.money()+150;
    const std::vector<PayrollBatchGroup> groups{{0,false,{{1,80},{2,80}},10},{0,false,{{2,60},{3,40}},20}};
    const auto plan=FinanceManager::prepare_batch(1,groups,w.people,w.provinces,sites);
    check(sites[0].cash==100&&sites[1].cash==50&&w.people[0].cash==0&&w.provinces[0].treasury==100,"site payroll prepare is pure");
    check(plan.establishment_count()==2&&plan.claim_establishment(0)==10&&plan.claim_establishment(3)==20,"opaque payroll plan retains stable site source IDs");
    check(plan.paid(0)==50&&plan.paid(1)==50&&plan.paid(2)==30&&plan.paid(3)==20,"site budgets allocate 50/50 then 30/20 gross");
    check(plan.tax(0)==5&&plan.tax(1)==5&&plan.tax(2)==3&&plan.tax(3)==2,"site payroll shares the exact tax calculation");
    FinanceManager::apply_batch(plan,1,w.people,w.provinces,sites);
    check(sites[0].cash==0&&sites[1].cash==0&&w.provinces[0].firm==1000,"site payroll debits its accounts without spending Province firm");
    check(w.people[0].cash==45&&w.people[1].cash==72&&w.people[2].cash==18,"multiple sites sharing a recipient accumulate exact net pay");
    check(w.provinces[0].treasury==115&&w.provinces[0].tax_receipts==15,"site taxes reach Province once");
    check(w.money()+sites[0].cash+sites[1].cash==opening,"site payroll conserves money across every source");
    bool threw=false;try{FinanceManager::apply_batch(plan,1,w.people,w.provinces,sites);}catch(const std::logic_error&){threw=true;}
    check(threw&&w.people[1].cash==72,"applying the same site plan twice rejects before mutation");
}

void test_site_payroll_shared_sources_and_tax_remainder(){
    {
        World w;auto sites=payroll_sites(100,0);
        const std::vector<PayrollBatchGroup> groups{{0,false,{{1,80}},10},{0,false,{{2,40}},10}};
        const auto plan=FinanceManager::prepare_batch(1,groups,w.people,w.provinces,sites);
        check(plan.establishment_count()==1&&plan.paid(0)==80&&plan.paid(1)==20,"repeated site groups cannot spend the same budget twice");
        FinanceManager::apply_batch(plan,1,w.people,w.provinces,sites);
        check(sites[0].cash==0&&w.people[0].cash==72&&w.people[1].cash==18,"shared underfunded source pays exactly its initial cash");
    }
    {
        World w;auto sites=payroll_sites(6,6);
        const std::vector<PayrollBatchGroup> groups{{0,false,{{1,6}},10},{0,false,{{1,6}},20}};
        const auto plan=FinanceManager::prepare_batch(1,groups,w.people,w.provinces,sites);
        FinanceManager::apply_batch(plan,1,w.people,w.provinces,sites);
        check(w.people[0].cash==11&&w.people[0].tax_remainder==20&&w.provinces[0].tax_receipts==1,"recipient tax remainder carries across employers");
    }
    {
        World w;w.provinces[0].treasury=100;auto sites=payroll_sites(50,0);
        const std::vector<PayrollBatchGroup> groups{{0,false,{{1,100}}},{0,false,{{1,80}},10},{0,true,{{2,200}}}};
        const auto plan=FinanceManager::prepare_batch(1,groups,w.people,w.provinces,sites);
        check(plan.paid(0)==100&&plan.paid(1)==50&&plan.paid(2)==115,"mixed sources preserve explicit ordering and earlier tax availability");
        FinanceManager::apply_batch(plan,1,w.people,w.provinces,sites);
        check(w.provinces[0].firm==900&&w.provinces[0].treasury==11&&sites[0].cash==0,"mixed Province and site accounts commit exact final balances");
        check(w.people[0].cash==135&&w.people[1].cash==104&&w.people[1].tax_remainder==50,"mixed payroll gives hand-calculated net pay and tax remainder");
        check(w.money()+sites[0].cash+sites[1].cash==1150,"mixed-source payroll conserves every monetary account");
    }
}

void test_site_payroll_stale_read_set_atomicity(){
    for(int mutation=0;mutation<5;++mutation){
        World w;auto sites=payroll_sites();
        const std::vector<PayrollBatchGroup> groups{{0,false,{{1,80}},10},{0,false,{{2,40}},20}};
        const auto plan=FinanceManager::prepare_batch(1,groups,w.people,w.provinces,sites);
        switch(mutation){
        case 0:++sites[1].cash;break;
        case 1:sites[1].region=1;break;
        case 2:sites[1].id=21;break;
        case 3:std::swap(sites[0],sites[1]);break;
        case 4:sites.pop_back();break;
        }
        const auto expected=sites;const auto money=w.money();bool threw=false;
        try{FinanceManager::apply_batch(plan,1,w.people,w.provinces,sites);}catch(const std::logic_error&){threw=true;}
        bool unchanged=sites.size()==expected.size();
        for(size_t i=0;i<sites.size();++i)unchanged=unchanged&&sites[i].id==expected[i].id&&sites[i].region==expected[i].region&&sites[i].cash==expected[i].cash;
        check(threw&&unchanged&&w.money()==money&&w.people[0].cash==0&&w.people[1].cash==0&&w.provinces[0].tax_receipts==0,"late stale site read rejects the complete batch before any payout");
    }
    World w;auto sites=payroll_sites();
    const auto plan=FinanceManager::prepare_batch(1,{{0,false,{{1,1}},10}},w.people,w.provinces,sites);
    bool threw=false;try{FinanceManager::apply_batch(plan,1,w.people,w.provinces);}catch(const std::out_of_range&){threw=true;}
    check(threw&&w.people[0].cash==0&&sites[0].cash==100,"legacy apply overload cannot silently omit site accounts");
}

void test_site_payroll_prepare_failure_is_pure(){
    {
        World w;auto sites=payroll_sites(-1,0);
        const auto zero=FinanceManager::prepare_batch(1,{{0,false,{{1,0}},10}},w.people,w.provinces,sites);
        check(zero.paid(0)==0&&zero.claim_establishment(0)==10&&zero.establishment_count()==0,"zero-due site claims validate identity without reading an unused account balance");
        bool threw=false;
        try{FinanceManager::prepare_batch(1,{{0,false,{{1,0}},999}},w.people,w.provinces,sites);}catch(const std::out_of_range&){threw=true;}
        check(threw,"zero-due site claims still reject an unknown source ID");
    }
    for(int bad=0;bad<4;++bad){
        World w;auto sites=payroll_sites();
        std::vector<PayrollBatchGroup> groups{{0,false,{{1,80}},10},{0,false,{{2,40}},20}};
        if(bad==0)sites[1].cash=-1;
        if(bad==1)groups[1].establishment_id=999;
        if(bad==2)groups[1].region=1;
        if(bad==3)groups[1].public_employer=true;
        const auto before=sites;const auto money=w.money();bool threw=false;
        try{FinanceManager::prepare_batch(1,groups,w.people,w.provinces,sites);}catch(const std::logic_error&){threw=true;}
        check(threw&&sites[0].cash==before[0].cash&&sites[1].cash==before[1].cash&&w.money()==money&&w.people[0].cash==0,"invalid later site group leaves all live inputs unchanged");
    }
    {
        World w;auto sites=payroll_sites(std::numeric_limits<int64_t>::max(),0);bool threw=false;
        try{FinanceManager::prepare_batch(1,{{0,false,{{1,std::numeric_limits<int64_t>::max()}},10}},w.people,w.provinces,sites);}catch(const std::overflow_error&){threw=true;}
        check(threw&&sites[0].cash==std::numeric_limits<int64_t>::max()&&w.people[0].cash==0,"site pro-rata multiplication overflow rejects without mutation");
    }
    {
        World w;w.people[0].cash=std::numeric_limits<int64_t>::max();auto sites=payroll_sites(1,0);bool threw=false;
        try{FinanceManager::prepare_batch(1,{{0,false,{{1,1}},10}},w.people,w.provinces,sites);}catch(const std::overflow_error&){threw=true;}
        check(threw&&sites[0].cash==1&&w.people[0].cash==std::numeric_limits<int64_t>::max(),"site recipient cash overflow rejects without mutation");
    }
}

} // namespace

void test_estate_death_settlement() {
    Person person;
    person.id = 7;
    person.region = 1;
    person.alive = true;
    person.cash = 100;
    person.inventory[0] = 4;
    person.inventory[3] = 6;
    int64_t estate = 500;
    int64_t legacy_unassigned = 500;
    std::vector<EstateCase> cases;
    std::vector<Establishment> sites;
    Establishment ownSite; ownSite.id = 20; ownSite.owner_kind = EstablishmentOwnerKind::Person; ownSite.owner_id = 7; sites.push_back(ownSite);
    Establishment publicSite; publicSite.id = 21; publicSite.owner_kind = EstablishmentOwnerKind::Public; publicSite.owner_id = 7; sites.push_back(publicSite);
    std::vector<LandHolding> holdings{{30, 1, 2, 7}, {31, 1, 1, 8}};
    std::vector<LandUseRight> rights{{40, 30, 8, 7}, {41, 31, 7, 8}};
    std::vector<Loan> loans{{50, 7, 0, 1, 10, 2, 0, false, false, 0}, {51, 8, 0, 1, 10, 0, 10, false, false, 0}};

    const auto initialPerson = person;
    const auto plan = EstateManager::prepare_death(person, 12, estate, legacy_unassigned, cases, sites, holdings, rights, loans);
    check(person.alive == initialPerson.alive && person.cash == initialPerson.cash &&
          person.inventory == initialPerson.inventory && cases.empty() && estate == 500,
          "estate prepare is pure");
    EstateManager::apply_death(plan, person, estate, cases);
    check(!person.alive && person.cash == 0 &&
          std::all_of(person.inventory.begin(), person.inventory.end(), [](int64_t value) { return value == 0; }),
          "estate apply retires person-owned cash and goods");
    check(estate == 600 && cases.size() == 1 && cases[0].decedent == 7 && cases[0].opened_day == 12 &&
          cases[0].cash == 100 && cases[0].inventory == initialPerson.inventory &&
          cases[0].establishments == std::vector<uint64_t>{20} && cases[0].land_holdings == std::vector<uint64_t>{30} &&
          cases[0].land_use_rights == std::vector<uint64_t>{40} && cases[0].loans == std::vector<uint64_t>{50},
          "estate case captures liquid assets and owned record references");
    check(initialPerson.cash + 500 == estate + person.cash && cases[0].inventory == initialPerson.inventory,
          "estate death preserves cash accounting and each good in its case");
    {
        std::stringstream encoded;
        polyec::detail::write_estate_section(encoded, legacy_unassigned, cases);
        int64_t decodedLegacy = 0;
        const auto decoded = polyec::detail::read_estate_section(encoded, 10, 3, 100, decodedLegacy);
        check(decodedLegacy == legacy_unassigned && decoded == cases,
              "estate case and legacy balance roundtrip through schema section");
    }
    bool threw = false;
    try { EstateManager::apply_death(plan, person, estate, cases); }
    catch (const std::logic_error&) { threw = true; }
    check(threw && !person.alive && estate == 600 && cases.size() == 1,
          "estate death plan cannot apply twice");

    Person stalePerson = initialPerson;
    int64_t staleEstate = 500;
    std::vector<EstateCase> staleCases;
    const auto stalePlan = EstateManager::prepare_death(stalePerson, 12, staleEstate, 500, staleCases, sites, holdings, rights, loans);
    staleCases.push_back(EstateCase{.decedent=2});
    threw = false;
    try { EstateManager::apply_death(stalePlan, stalePerson, staleEstate, staleCases); }
    catch (const std::logic_error&) { threw = true; }
    check(threw && stalePerson.alive && stalePerson.cash == initialPerson.cash && staleEstate == 500 && staleCases.size() == 1,
          "stale estate case read set rejects before any settlement write");

    stalePerson = initialPerson;
    staleEstate = 500;
    staleCases.clear();
    const auto personPlan = EstateManager::prepare_death(stalePerson, 12, staleEstate, 500, staleCases, sites, holdings, rights, loans);
    stalePerson.cash -= 1;
    threw = false;
    try { EstateManager::apply_death(personPlan, stalePerson, staleEstate, staleCases); }
    catch (const std::logic_error&) { threw = true; }
    check(threw && stalePerson.alive && stalePerson.cash == initialPerson.cash - 1 &&
          staleEstate == 500 && staleCases.empty(),
          "stale Person read set rejects before any settlement write");

    stalePerson = initialPerson;
    staleEstate = 500;
    const auto estatePlan = EstateManager::prepare_death(stalePerson, 12, staleEstate, 500, staleCases, sites, holdings, rights, loans);
    staleEstate += 1;
    threw = false;
    try { EstateManager::apply_death(estatePlan, stalePerson, staleEstate, staleCases); }
    catch (const std::logic_error&) { threw = true; }
    check(threw && stalePerson.alive && stalePerson.cash == initialPerson.cash &&
          staleEstate == 501 && staleCases.empty(),
          "stale estate balance rejects before any settlement write");

    Person overflowPerson = initialPerson;
    int64_t overflowEstate = std::numeric_limits<int64_t>::max();
    threw = false;
    try { (void)EstateManager::prepare_death(overflowPerson, 12, overflowEstate, overflowEstate, {}, sites, holdings, rights, loans); }
    catch (const std::overflow_error&) { threw = true; }
    check(threw && overflowPerson.alive && overflowPerson.cash == initialPerson.cash &&
          overflowPerson.inventory == initialPerson.inventory && overflowEstate == std::numeric_limits<int64_t>::max(),
          "estate balance overflow rejects pure preparation");

    Person invalidPerson = initialPerson;
    invalidPerson.inventory[2] = -1;
    threw = false;
    try { (void)EstateManager::prepare_death(invalidPerson, 12, 500, 500, {}, sites, holdings, rights, loans); }
    catch (const std::logic_error&) { threw = true; }
    check(threw && invalidPerson.alive && invalidPerson.cash == initialPerson.cash,
          "invalid estate goods reject pure preparation");

    {
        std::vector<Person> people(4);
        for (size_t i=0;i<people.size();++i) { people[i].id=i+1; people[i].cash=2; people[i].alive=true; }
        people[2].alive=false;
        std::vector<Edge> relations{{1,3,RelationKind::ParentChild,true},
                                    {2,3,RelationKind::ParentChild,true}};
        std::vector<EstateCase> distributionCases{{.decedent=3,.cash=11}};
        distributionCases[0].inventory[1]=5;distributionCases[0].inventory[2]=2;
        std::vector<Province> distributionProvinces(1);
        int64_t distributionEstate=11, publicFund=0;
        const auto distribution=EstateManager::prepare_distribution(3,distributionEstate,publicFund,
            distributionCases,people,relations,distributionProvinces);
        check(distributionEstate==11&&people[0].cash==2&&people[1].cash==2&&distributionCases[0].cash==11,
              "estate distribution preparation is pure");
        EstateManager::apply_distribution(distribution,distributionEstate,publicFund,distributionCases,people,distributionProvinces);
        check(people[0].cash==8&&people[1].cash==7&&people[2].cash==2&&distributionCases[0].cash==0&&
              people[0].inventory[1]==3&&people[1].inventory[1]==2&&people[0].inventory[2]==1&&
              people[1].inventory[2]==1&&distributionCases[0].inventory[1]==0&&distributionCases[0].inventory[2]==0&&
              distributionEstate==0&&publicFund==0,
              "cash and each estate good split evenly with indivisible units going to the lowest ID");
        check(people[0].cash+people[1].cash+people[2].cash+distributionEstate==17,
              "beneficiary cash transfer conserves money");
        check(people[0].inventory[1]+people[1].inventory[1]+distributionCases[0].inventory[1]==5&&
              people[0].inventory[2]+people[1].inventory[2]+distributionCases[0].inventory[2]==2,
              "beneficiary goods transfer conserves each product quantity");
    }
    {
        std::vector<Person> people(3);
        for (size_t i = 0; i < people.size(); ++i) { people[i].id = i + 1; people[i].alive = i != 1; }
        people[0].age_days = 10 * 365;
        std::vector<Edge> relations{{1, 2, RelationKind::Spouse, false}};
        std::vector<EstateCase> cases{{.decedent = 2, .land_holdings = {4}, .land_use_rights = {5}}};
        std::vector<Loan> loans;
        std::vector<LandHolding> holdings{{4, 0, 3, 2}};
        std::vector<LandUseRight> rights{{5, 4, 3, 2}};
        std::vector<Province> provinces(1);
        int64_t estate = 0, public_fund = 0;
        const auto plan = EstateManager::prepare_distribution(2, estate, public_fund, cases, people,
            relations, provinces, loans, holdings, rights);
        check(holdings[0].title_holder == 2 && rights[0].grantor == 2 && cases[0].land_holdings.size() == 1,
              "estate title preparation leaves live ownership unchanged");
        EstateManager::apply_distribution(plan, estate, public_fund, cases, people, provinces, loans, holdings, rights);
        check(holdings[0].title_holder == 1 && rights[0].grantor == 1 && rights[0].grantee == 3 &&
              cases[0].land_holdings.empty() && cases[0].land_use_rights.empty() && people[0].age_days < 18 * 365,
              "sole minor heir receives the title while its existing use permission continues");
    }
    {
        std::vector<Person> people(4);
        for (size_t i = 0; i < people.size(); ++i) { people[i].id = i + 1; people[i].alive = i != 2; }
        std::vector<Edge> relations{{1, 3, RelationKind::ParentChild, true}, {2, 3, RelationKind::ParentChild, true}};
        std::vector<EstateCase> cases{{.decedent = 3, .establishments = {9}, .land_holdings = {4}}};
        Establishment site; site.id = 9; site.owner_kind = EstablishmentOwnerKind::Person; site.owner_id = 3;
        std::vector<Establishment> establishments{site};
        std::vector<LandHolding> holdings{{4, 0, 1, 3}};
        std::vector<Loan> loans;
        std::vector<LandUseRight> rights;
        std::vector<Province> provinces(1);
        int64_t estate = 0, public_fund = 0;
        const auto plan = EstateManager::prepare_distribution(3, estate, public_fund, cases, people,
            relations, provinces, loans, establishments, holdings, rights);
        EstateManager::apply_distribution(plan, estate, public_fund, cases, people, provinces, loans, establishments, holdings, rights);
        check(holdings[0].title_holder == 3 && cases[0].land_holdings == std::vector<uint64_t>{4}&&
              establishments[0].owner_id == 3 && cases[0].establishments == std::vector<uint64_t>{9},
              "land and workshop remain in the estate when several children would need an ownership split");
    }
    {
        std::vector<Person> people(2); people[0].id = 1; people[0].alive = false; people[1].id = 2; people[1].alive = false;
        std::vector<EstateCase> cases{{.decedent = 2, .establishments = {9}, .land_holdings = {4}}};
        Establishment site; site.id = 9; site.owner_kind = EstablishmentOwnerKind::Person; site.owner_id = 2;
        std::vector<Establishment> establishments{site};
        std::vector<Loan> loans;
        std::vector<LandHolding> holdings{{4, 0, 1, 2}};
        std::vector<LandUseRight> rights;
        std::vector<Province> provinces(1);
        int64_t estate = 0, public_fund = 0;
        const auto plan = EstateManager::prepare_distribution(2, estate, public_fund, cases, people,
            {}, provinces, loans, establishments, holdings, rights);
        EstateManager::apply_distribution(plan, estate, public_fund, cases, people, provinces, loans, establishments, holdings, rights);
        check(holdings[0].title_holder == 2 && cases[0].land_holdings == std::vector<uint64_t>{4}&&
              establishments[0].owner_id == 2 && cases[0].establishments == std::vector<uint64_t>{9},
              "land and workshop remain in the estate when no heir is alive");
    }
    {
        std::vector<Person> people(2); people[0].id = 1; people[0].alive = true; people[1].id = 2; people[1].alive = false;
        std::vector<Edge> relations{{1, 2, RelationKind::Spouse, false}};
        std::vector<EstateCase> cases{{.decedent = 2, .establishments = {9}, .land_holdings = {4}, .loans = {6}}};
        std::vector<Loan> loans{{6, 2, 0, 10, 5, 0, 0, false, false, 0}};
        Establishment site; site.id = 9; site.owner_kind = EstablishmentOwnerKind::Person; site.owner_id = 2;
        std::vector<Establishment> establishments{site};
        std::vector<LandHolding> holdings{{4, 0, 1, 2}};
        std::vector<LandUseRight> rights;
        std::vector<Province> provinces(1);
        int64_t estate = 0, public_fund = 0;
        const auto plan = EstateManager::prepare_distribution(2, estate, public_fund, cases, people,
            relations, provinces, loans, establishments, holdings, rights);
        EstateManager::apply_distribution(plan, estate, public_fund, cases, people, provinces, loans, establishments, holdings, rights);
        check(holdings[0].title_holder == 2 && cases[0].land_holdings == std::vector<uint64_t>{4}&&
              establishments[0].owner_id == 2 && cases[0].establishments == std::vector<uint64_t>{9},
              "land and workshop remain in the estate while an unpaid claim remains");
    }
    {
        std::vector<Person> people(2); people[0].id = 1; people[0].alive = true; people[0].cash = 10;
        people[1].id = 2; people[1].alive = false;
        std::vector<Edge> relations{{1, 2, RelationKind::Spouse, false}};
        std::vector<EstateCase> cases{{.decedent = 2, .cash = 5, .land_holdings = {4}}};
        std::vector<Loan> loans;
        std::vector<LandHolding> holdings{{4, 0, 1, 2}};
        std::vector<LandUseRight> rights;
        std::vector<Province> provinces(1);
        int64_t estate = 5, public_fund = 0;
        const auto plan = EstateManager::prepare_distribution(2, estate, public_fund, cases, people,
            relations, provinces, loans, holdings, rights);
        holdings[0].area_units++;
        bool rejected = false;
        try { EstateManager::apply_distribution(plan, estate, public_fund, cases, people, provinces, loans, holdings, rights); }
        catch (const std::logic_error&) { rejected = true; }
        check(rejected && people[0].cash == 10 && estate == 5 && cases[0].cash == 5 &&
              holdings[0].title_holder == 2,
              "stale land title rejects the full estate transfer before cash or inventory changes");
    }
    {
        std::vector<Person> people(3); people[0].id = 1; people[0].alive = true; people[0].age_days = 30 * 365;
        people[1].id = 2; people[1].alive = false; people[2].id = 3; people[2].alive = true;
        std::vector<Edge> relations{{1, 2, RelationKind::Spouse, false}};
        std::vector<EstateCase> cases{{.decedent = 2, .establishments = {8}}};
        Establishment site; site.id = 8; site.region = 0; site.owner_kind = EstablishmentOwnerKind::Person;
        site.owner_id = 2; site.industry = polyec::EstablishmentIndustry::ClothWorkshop; site.capacity = 9; site.cash = 17;
        site.stock[static_cast<size_t>(polyec::GoodId::Fiber)] = 6; site.last_production_day = 12;
        site.processed_work_units.emplace(3, 7); site.work_authorizations[3].push_back({160, 960});
        site.authorization_window_count = 1; site.work_pay.legacy_credit[3] = 5; site.work_pay.paid_units[3] = 2;
        site.work_pay.remainders[3] = 1;
        const auto original = site;
        std::vector<Establishment> establishments{site};
        std::vector<Loan> loans;
        std::vector<LandHolding> holdings;
        std::vector<LandUseRight> rights;
        std::vector<Province> provinces(1);
        int64_t estate = 0, public_fund = 0;
        const auto plan = EstateManager::prepare_distribution(2, estate, public_fund, cases, people,
            relations, provinces, loans, establishments, holdings, rights);
        EstateManager::apply_distribution(plan, estate, public_fund, cases, people, provinces,
            loans, establishments, holdings, rights);
        const auto& transferred = establishments.front();
        check(transferred.owner_kind == original.owner_kind && transferred.owner_id == 1 && transferred.region == original.region &&
              transferred.industry == original.industry && transferred.capacity == original.capacity && transferred.cash == original.cash &&
              transferred.stock == original.stock && transferred.last_production_day == original.last_production_day &&
              transferred.processed_work_units == original.processed_work_units &&
              transferred.work_authorizations == original.work_authorizations &&
              transferred.authorization_window_count == original.authorization_window_count &&
              transferred.work_pay.legacy_credit == original.work_pay.legacy_credit &&
              transferred.work_pay.paid_units == original.work_pay.paid_units &&
              transferred.work_pay.remainders == original.work_pay.remainders && cases[0].establishments.empty(),
              "sole adult heir receives the personal workshop without changing its funds, goods, workers or production record");
    }
    {
        std::vector<Person> people(2); people[0].id = 1; people[0].alive = true; people[0].age_days = 17 * 365;
        people[1].id = 2; people[1].alive = false;
        std::vector<Edge> relations{{1, 2, RelationKind::Spouse, false}};
        std::vector<EstateCase> cases{{.decedent = 2, .establishments = {8}}};
        Establishment site; site.id = 8; site.owner_kind = EstablishmentOwnerKind::Person; site.owner_id = 2;
        std::vector<Establishment> establishments{site};
        std::vector<Loan> loans; std::vector<LandHolding> holdings; std::vector<LandUseRight> rights;
        std::vector<Province> provinces(1); int64_t estate = 0, public_fund = 0;
        const auto plan = EstateManager::prepare_distribution(2, estate, public_fund, cases, people,
            relations, provinces, loans, establishments, holdings, rights);
        EstateManager::apply_distribution(plan, estate, public_fund, cases, people, provinces,
            loans, establishments, holdings, rights);
        check(establishments[0].owner_id == 2 && cases[0].establishments == std::vector<uint64_t>{8},
              "personal workshop stays in the estate until a minor's management authority is defined");
    }
    {
        std::vector<Person> people(2); people[0].id = 1; people[0].alive = true; people[0].age_days = 30 * 365;
        people[0].cash = 10; people[1].id = 2; people[1].alive = false;
        std::vector<Edge> relations{{1, 2, RelationKind::Spouse, false}};
        std::vector<EstateCase> cases{{.decedent = 2, .cash = 5, .establishments = {8}}};
        Establishment site; site.id = 8; site.owner_kind = EstablishmentOwnerKind::Person; site.owner_id = 2;
        std::vector<Establishment> establishments{site};
        std::vector<Loan> loans; std::vector<LandHolding> holdings; std::vector<LandUseRight> rights;
        std::vector<Province> provinces(1); int64_t estate = 5, public_fund = 0;
        const auto plan = EstateManager::prepare_distribution(2, estate, public_fund, cases, people,
            relations, provinces, loans, establishments, holdings, rights);
        establishments[0].owner_id = 3;
        bool rejected = false;
        try { EstateManager::apply_distribution(plan, estate, public_fund, cases, people, provinces,
            loans, establishments, holdings, rights); }
        catch (const std::logic_error&) { rejected = true; }
        check(rejected && people[0].cash == 10 && estate == 5 && cases[0].cash == 5 &&
              establishments[0].owner_id == 3,
              "stale workshop owner rejects the full estate transfer before cash changes");
    }
    {
        std::vector<Person> people(2); people[0].id=1; people[0].alive=true; people[1].id=2; people[1].alive=false;
        std::vector<EstateCase> distributionCases{{.decedent=2,.cash=9}};
        distributionCases[0].inventory[1]=7;
        std::vector<Province> distributionProvinces(1);distributionProvinces[0].stock[1]=2;
        int64_t distributionEstate=13, publicFund=4;
        const auto distribution=EstateManager::prepare_distribution(2,distributionEstate,publicFund,
            distributionCases,people,{},distributionProvinces);
        EstateManager::apply_distribution(distribution,distributionEstate,publicFund,distributionCases,people,distributionProvinces);
        check(distributionEstate==13&&publicFund==13&&distributionCases[0].cash==0&&distributionCases[0].inventory[1]==0&&
              people[0].cash==100&&distributionProvinces[0].stock[1]==9,
              "unclaimed estate cash and goods enter the public ledgers without changing totals");
    }
    {
        std::vector<Person> people(2); people[0].id=1; people[0].alive=true; people[1].id=2; people[1].alive=false;
        people[0].cash=std::numeric_limits<int64_t>::max();
        std::vector<EstateCase> distributionCases{{.decedent=2,.cash=1}};
        std::vector<Province> distributionProvinces(1);
        int64_t distributionEstate=1, publicFund=0;
        bool distributionThrew=false;
        try { (void)EstateManager::prepare_distribution(2,distributionEstate,publicFund,
            distributionCases,people,{{1,2,RelationKind::Spouse,false}},distributionProvinces); }
        catch (const std::overflow_error&) { distributionThrew=true; }
        check(distributionThrew&&distributionEstate==1&&distributionCases[0].cash==1&&publicFund==0,
              "recipient balance overflow rejects before estate mutation");
    }
    {
        std::vector<Person> people(2); people[0].id=1; people[0].alive=true; people[1].id=2; people[1].alive=false;
        people[0].inventory[1]=std::numeric_limits<int64_t>::max();
        std::vector<EstateCase> distributionCases{{.decedent=2}};distributionCases[0].inventory[1]=1;
        std::vector<Province> distributionProvinces(1);
        int64_t distributionEstate=0, publicFund=0;
        bool distributionThrew=false;
        try { (void)EstateManager::prepare_distribution(2,distributionEstate,publicFund,
            distributionCases,people,{{1,2,RelationKind::Spouse,false}},distributionProvinces); }
        catch (const std::overflow_error&) { distributionThrew=true; }
        check(distributionThrew&&distributionCases[0].inventory[1]==1&&
              people[0].inventory[1]==std::numeric_limits<int64_t>::max(),
              "recipient goods overflow rejects before estate mutation");
    }
    {
        std::vector<Person> people(2); people[0].id=1; people[0].alive=true; people[0].cash=10;
        people[1].id=2; people[1].alive=false;
        std::vector<Edge> relations{{1,2,RelationKind::Spouse,false}};
        std::vector<EstateCase> distributionCases{{.decedent=2,.cash=5}};
        std::vector<Province> distributionProvinces(1);
        int64_t distributionEstate=5, publicFund=0;
        const auto distribution=EstateManager::prepare_distribution(2,distributionEstate,publicFund,
            distributionCases,people,relations,distributionProvinces);
        people[0].cash++;
        bool distributionThrew=false;
        try { EstateManager::apply_distribution(distribution,distributionEstate,publicFund,distributionCases,people,distributionProvinces); }
        catch (const std::logic_error&) { distributionThrew=true; }
        check(distributionThrew&&people[0].cash==11&&distributionEstate==5&&distributionCases[0].cash==5,
              "stale beneficiary balance rejects before any estate distribution write");
    }
    {
        std::vector<Person> people(2);people[0].id=1;people[0].alive=true;people[0].cash=10;
        people[1].id=2;people[1].alive=false;
        std::vector<EstateCase> distributionCases{{.decedent=2}};distributionCases[0].inventory[1]=5;
        std::vector<Province> distributionProvinces(1);
        const auto distribution=EstateManager::prepare_distribution(2,0,0,distributionCases,people,
            {{1,2,RelationKind::Spouse,false}},distributionProvinces);
        people[0].inventory[1]=1;
        int64_t distributionEstate=0,publicFund=0;
        bool distributionThrew=false;
        try { EstateManager::apply_distribution(distribution,distributionEstate,publicFund,distributionCases,people,distributionProvinces); }
        catch (const std::logic_error&) { distributionThrew=true; }
        check(distributionThrew&&people[0].inventory[1]==1&&distributionCases[0].inventory[1]==5,
              "stale beneficiary goods reject before any estate distribution write");
    }
    {
        std::vector<Person> people(2);people[0].id=1;people[0].alive=true;people[1].id=2;people[1].alive=false;
        std::vector<EstateCase> distributionCases{{.decedent=2}};distributionCases[0].inventory[1]=5;
        std::vector<Province> distributionProvinces(1);distributionProvinces[0].stock[1]=4;
        int64_t distributionEstate=0,publicFund=0;
        const auto distribution=EstateManager::prepare_distribution(2,0,0,distributionCases,people,{},distributionProvinces);
        distributionProvinces[0].stock[1]=5;
        bool distributionThrew=false;
        try { EstateManager::apply_distribution(distribution,distributionEstate,publicFund,distributionCases,people,distributionProvinces); }
        catch (const std::logic_error&) { distributionThrew=true; }
        check(distributionThrew&&distributionProvinces[0].stock[1]==5&&distributionCases[0].inventory[1]==5&&publicFund==0,
              "stale public goods stock rejects before any estate distribution write");
    }

    {
        std::vector<EstateCase> claimCases{{.decedent=7,.cash=100,.loans={50,51}}};
        std::vector<Loan> claimLoans{{50,7,0,40,10,2,2,false,false,0},
                                     {51,7,1,50,20,5,0,false,true,0}};
        std::vector<Province> claimProvinces(2);
        claimProvinces[0].firm=100;claimProvinces[1].firm=200;
        int64_t claimEstate=100,claimCreditPool=300;
        const auto claimPlan=EstateManager::prepare_cash_claims(7,claimEstate,0,claimCreditPool,
            claimCases,claimLoans,claimProvinces);
        check(claimEstate==100&&claimCases[0].cash==100&&claimLoans[0].paid==2&&
              claimProvinces[0].firm==100&&claimCreditPool==300,
              "estate cash claim preparation leaves all ledgers unchanged");
        EstateManager::apply_cash_claims(claimPlan,claimEstate,0,claimCreditPool,claimCases,claimLoans,claimProvinces);
        check(claimEstate==65&&claimCases[0].cash==65&&claimLoans[0].paid==12&&claimLoans[1].paid==25&&
              claimProvinces[0].firm==110&&claimCreditPool==325,
              "estate pays Province and royal-pool creditors in full from its own cash");
        check(claimEstate==claimCases[0].cash&&claimEstate+claimProvinces[0].firm+
              claimProvinces[1].firm+claimCreditPool==700,
              "estate claim transfer conserves money across estate and creditor accounts");
    }
    {
        std::vector<EstateCase> claimCases{{.decedent=7,.cash=20,.loans={50,51}}};
        std::vector<Loan> claimLoans{{50,7,0,40,10,2,2,false,false,0},
                                     {51,7,1,50,20,5,0,false,true,0}};
        std::vector<Province> claimProvinces(2);
        int64_t claimEstate=20,claimCreditPool=300;
        const auto beforeCases=claimCases;const auto beforeLoans=claimLoans;
        const auto beforeFirms=std::array<int64_t,2>{claimProvinces[0].firm,claimProvinces[1].firm};
        const auto claimPlan=EstateManager::prepare_cash_claims(7,claimEstate,0,claimCreditPool,
            claimCases,claimLoans,claimProvinces);
        EstateManager::apply_cash_claims(claimPlan,claimEstate,0,claimCreditPool,claimCases,claimLoans,claimProvinces);
        check(claimEstate==20&&claimCases==beforeCases&&claimLoans[0].paid==beforeLoans[0].paid&&
              claimLoans[1].paid==beforeLoans[1].paid&&claimCreditPool==300&&
              claimProvinces[0].firm==beforeFirms[0]&&claimProvinces[1].firm==beforeFirms[1],
              "insufficient estate cash leaves every creditor and estate record unchanged");
    }
    {
        std::vector<EstateCase> claimCases{{.decedent=7,.cash=100,.loans={50}}};
        std::vector<Loan> claimLoans{{50,7,0,40,10,0,0,false,false,0}};
        std::vector<Province> claimProvinces(1);
        claimProvinces[0].firm=std::numeric_limits<int64_t>::max();
        int64_t claimEstate=100,claimCreditPool=0;
        bool claimThrew=false;
        try { (void)EstateManager::prepare_cash_claims(7,claimEstate,0,claimCreditPool,
            claimCases,claimLoans,claimProvinces); }
        catch (const std::overflow_error&) { claimThrew=true; }
        check(claimThrew&&claimEstate==100&&claimCases[0].cash==100&&claimLoans[0].paid==0,
              "creditor account overflow rejects estate settlement before mutation");
    }
    {
        std::vector<EstateCase> claimCases{{.decedent=7,.cash=100,.loans={50}}};
        std::vector<Loan> claimLoans{{50,7,0,40,10,0,0,false,false,0}};
        std::vector<Province> claimProvinces(1);
        int64_t claimEstate=100,claimCreditPool=0;
        const auto claimPlan=EstateManager::prepare_cash_claims(7,claimEstate,0,claimCreditPool,
            claimCases,claimLoans,claimProvinces);
        ++claimProvinces[0].firm;
        bool claimThrew=false;
        try { EstateManager::apply_cash_claims(claimPlan,claimEstate,0,claimCreditPool,
            claimCases,claimLoans,claimProvinces); }
        catch (const std::logic_error&) { claimThrew=true; }
        check(claimThrew&&claimEstate==100&&claimCases[0].cash==100&&claimLoans[0].paid==0&&
              claimCreditPool==0,
              "stale creditor balance rejects before changing estate or loan rows");
    }

    std::vector<Person> kin(8);
    for (size_t i = 0; i < kin.size(); ++i) { kin[i].id = i + 1; kin[i].alive = true; }
    kin[0].alive = false;
    kin[1].alive = false; // A deceased child does not block the next living tier.
    const std::vector<Edge> family{
        {2, 1, RelationKind::ParentChild, true}, {3, 1, RelationKind::ParentChild, true},
        {1, 4, RelationKind::Spouse}, {1, 5, RelationKind::ParentChild, true},
        {1, 6, RelationKind::ParentChild, true}, {7, 5, RelationKind::ParentChild, true},
        {8, 5, RelationKind::ParentChild, true}
    };
    auto heirs = EstateManager::resolve_beneficiaries(1, kin, family);
    check(heirs.tier == EstateBeneficiaryTier::Children && heirs.recipients == std::vector<uint64_t>{3},
          "living direct children inherit before spouse, parents and siblings");
    kin[2].alive = false;
    heirs = EstateManager::resolve_beneficiaries(1, kin, family);
    check(heirs.tier == EstateBeneficiaryTier::Spouse && heirs.recipients == std::vector<uint64_t>{4},
          "spouse inherits when no direct child is living");
    kin[3].alive = false;
    heirs = EstateManager::resolve_beneficiaries(1, kin, family);
    check(heirs.tier == EstateBeneficiaryTier::Parents && heirs.recipients == (std::vector<uint64_t>{5, 6}),
          "living parents share before siblings in deterministic ID order");
    kin[4].alive = false;
    kin[5].alive = false;
    heirs = EstateManager::resolve_beneficiaries(1, kin, family);
    check(heirs.tier == EstateBeneficiaryTier::Siblings && heirs.recipients == (std::vector<uint64_t>{7, 8}),
          "siblings are resolved through shared parent relationships");
    kin[6].alive = false;
    kin[7].alive = false;
    heirs = EstateManager::resolve_beneficiaries(1, kin, family);
    check(heirs.tier == EstateBeneficiaryTier::PublicEstate && heirs.recipients.empty(),
          "no living kin selects the public estate tier without inventing a recipient");
    const std::vector<Edge> unverified_household_edges{
        {2, 1, RelationKind::ParentChild}, {3, 1, RelationKind::ParentChild},
        {1, 5, RelationKind::ParentChild}, {7, 5, RelationKind::ParentChild},
        {8, 5, RelationKind::ParentChild}, {1, 7, RelationKind::Sibling}
    };
    kin[0].alive=false;kin[1].alive=true;kin[2].alive=true;kin[4].alive=true;kin[6].alive=true;kin[7].alive=true;
    heirs=EstateManager::resolve_beneficiaries(1,kin,unverified_household_edges);
    check(heirs.tier==EstateBeneficiaryTier::PublicEstate&&heirs.recipients.empty(),
          "unverified household parent and sibling edges cannot inherit");
    threw = false;
    try { (void)EstateManager::resolve_beneficiaries(9, kin, family); }
    catch (const std::invalid_argument&) { threw = true; }
    check(threw, "beneficiary resolution rejects unknown decedent ID");
    kin[0].alive = true;
    threw = false;
    try { (void)EstateManager::resolve_beneficiaries(1, kin, family); }
    catch (const std::invalid_argument&) { threw = true; }
    check(threw, "beneficiary resolution rejects a living person");
}

void test_peerage_successor_resolution() {
    using polyec::PeerageSuccessionManager;
    using polyec::PeerageSuccessionState;

    std::vector<Person> people(5);
    for (size_t i = 0; i < people.size(); ++i) {
        people[i].id = i + 1;
        people[i].alive = true;
    }
    people[0].alive = false;
    people[0].age_days = 60 * 365;
    people[1].age_days = 17 * 365;
    people[2].age_days = 16 * 365;
    people[3].age_days = 55 * 365;
    people[4].alive = false;
    people[4].age_days = 65 * 365;
    const std::vector<Edge> relations{
        {2, 1, RelationKind::ParentChild, true},
        {3, 1, RelationKind::ParentChild, true},
        {4, 1, RelationKind::ParentChild, false},
        {5, 1, RelationKind::ParentChild, true},
    };

    auto resolution = PeerageSuccessionManager::resolve_successor(1, people, relations);
    check(resolution.state == PeerageSuccessionState::SuccessorSelected &&
          resolution.successor_id == 2 && resolution.candidates.empty(),
          "peerage selects the eldest living child with verified legal parentage");

    resolution = PeerageSuccessionManager::resolve_successor(1, people,
        {{2, 1, RelationKind::ParentChild, true}, {2, 1, RelationKind::ParentChild, true}});
    check(resolution.state == PeerageSuccessionState::SuccessorSelected &&
          resolution.successor_id == 2,
          "duplicate parentage records do not turn one successor into an age tie");

    people[2].age_days = 17 * 365;
    resolution = PeerageSuccessionManager::resolve_successor(1, people, relations);
    check(resolution.state == PeerageSuccessionState::RecognitionRequired &&
          resolution.successor_id == 0 && resolution.candidates == std::vector<uint64_t>{2, 3},
          "peerage age tie remains pending recognition with sorted candidates");

    const std::vector<Edge> unverified{{4, 1, RelationKind::ParentChild, false}};
    resolution = PeerageSuccessionManager::resolve_successor(1, people, unverified);
    check(resolution.state == PeerageSuccessionState::RecognitionRequired &&
          resolution.successor_id == 0 && resolution.candidates.empty(),
          "no verified living child leaves peerage succession pending recognition");

    resolution = PeerageSuccessionManager::resolve_successor(1, people,
        {{1, 2, RelationKind::ParentChild, true}});
    check(resolution.state == PeerageSuccessionState::RecognitionRequired &&
          resolution.successor_id == 0 && resolution.candidates.empty(),
          "peerage succession only follows child-to-parent relation edges");

    bool threw = false;
    try {
        (void)PeerageSuccessionManager::resolve_successor(1, people,
            {{99, 1, RelationKind::ParentChild, true}});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "peerage succession rejects invalid relationship IDs");

    auto malformed_people = people;
    malformed_people[1].id = 99;
    threw = false;
    try {
        (void)PeerageSuccessionManager::resolve_successor(1, malformed_people, relations);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "peerage succession rejects non-canonical person IDs");

    malformed_people = people;
    malformed_people[1].age_days = -1;
    threw = false;
    try {
        (void)PeerageSuccessionManager::resolve_successor(1, malformed_people, relations);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "peerage succession rejects negative person age");

    threw = false;
    try {
        (void)PeerageSuccessionManager::resolve_successor(1, people,
            {{2, 1, RelationKind::Sibling, true}});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "peerage succession rejects legal-parentage flag on a non-parent edge");

    threw = false;
    people[0].alive = true;
    try {
        (void)PeerageSuccessionManager::resolve_successor(1, people, relations);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "peerage succession rejects a living title holder");
}

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
    test_batch_matches_sequential_groups();
    test_batch_shared_underfunding_pro_rata();
    test_batch_apply_is_atomic_after_stale_read();
    test_batch_prepare_failure_leaves_inputs_unchanged();
    test_estate_death_settlement();
    test_site_payroll_hand_calculated();
    test_site_payroll_shared_sources_and_tax_remainder();
    test_site_payroll_stale_read_set_atomicity();
    test_site_payroll_prepare_failure_is_pure();
    test_employment_boundaries();
    test_employment_daily_helpers();
    test_employment_appointment_and_office();
    test_employment_appointment_preflight();
    test_household_index();
    test_project_manager();
    test_peerage_successor_resolution();
    if (failures) {
        std::fprintf(stderr, "%d manager test failure(s)\n", failures);
        return 1;
    }
    std::printf("manager tests passed\n");
    return 0;
}






