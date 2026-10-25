#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace polyec;
namespace {
int checks = 0;
void check(bool ok, const char* label) { ++checks; if (!ok) throw std::runtime_error(label); }
void reject_unchanged(Realm& world, HouseholdFundCommand command, const char* label) {
    const auto before = world.fingerprint();
    const auto result = world.execute(command);
    check(!result.success && world.fingerprint() == before, label);
}
}

int main() {
    try {
        Realm world(9041, 1, 256);
        check(!world.pilot_policy_compatible() && std::string(world.model_contract()) == "realm-household-fund-goods30-config-v54-atlas-north-pass-v1",
              "household spending semantics version the learned model contract");
        const Realm legacy = world;
        uint64_t household = 0, authority = 0, contributor = 0, outsider = 0;
        for (uint64_t id = 1; id <= uint64_t(world.count()) && !household; ++id) {
            const auto members = world.household_members(world.household_of(id));
            const auto candidateAuthority = world.household_authority(world.household_of(id));
            uint64_t candidateContributor = 0;
            for (const auto member : members) {
                if (member != candidateAuthority && world.person(member).age_days >= 18 * 365) {
                    candidateContributor = member;
                    break;
                }
            }
            if (candidateAuthority && candidateContributor) {
                household = world.household_of(id);
                authority = candidateAuthority;
                contributor = candidateContributor;
            }
        }
        check(household != 0 && authority && contributor, "fixture has adult authority and contributor");
        for (uint64_t id = 1; id <= uint64_t(world.count()); ++id) {
            if (world.household_of(id) != household && world.person(id).age_days >= 18 * 365) { outsider = id; break; }
        }
        check(outsider != 0, "fixture has adult outsider");
        check(world.household_fund(household) == 0 && world.household_fund_records() == 0, "fund starts empty");

        const auto total = world.total_money();
        const auto personCash = world.person(contributor).cash;
        auto result = world.execute({contributor, world.person(contributor).decisions + 1, household,
                                     HouseholdFundOperation::Deposit, 60});
        check(result.success && world.household_fund(household) == 60, "adult deposits into household fund");
        check(world.person(contributor).cash == personCash - 60 && world.total_money() == total, "deposit transfers money without creating it");
        auto balances = world.balances();
        int64_t balanceTotal = 0;
        for (const auto balance : balances) balanceTotal += balance;
        check(balances.size() == 11 && balances[10] == 60 && balanceTotal == world.total_money(), "balances append household fund aggregate and reconcile");

        reject_unchanged(world, {outsider, world.person(outsider).decisions + 1, household, HouseholdFundOperation::Deposit, 1}, "outsider deposit rejected atomically");
        reject_unchanged(world, {contributor, world.person(contributor).decisions + 1, household, HouseholdFundOperation::Withdraw, 1}, "non-authority withdrawal rejected atomically");
        reject_unchanged(world, {contributor, world.person(contributor).decisions + 1, household, HouseholdFundOperation::Deposit, 0}, "zero deposit rejected atomically");
        reject_unchanged(world, {contributor, world.person(contributor).decisions + 1, household, HouseholdFundOperation::Deposit, INT64_MAX}, "insufficient deposit rejected atomically");
        reject_unchanged(world, {authority, world.person(authority).decisions + 2, household, HouseholdFundOperation::Withdraw, 1}, "stale sequence rejected atomically");

        const auto authorityCash = world.person(authority).cash;
        result = world.execute({authority, world.person(authority).decisions + 1, household,
                                HouseholdFundOperation::Withdraw, 25});
        check(result.success && world.household_fund(household) == 35, "authority withdraws from household fund");
        check(world.person(authority).cash == authorityCash + 25 && world.total_money() == total, "withdrawal conserves total money");
        check(world.invariant(), "fund state preserves Realm invariant");

        Realm restored(1, 1, 128);
        std::string error;
        const auto saved = world.serialize("household-test");
        check(saved.rfind("POLYEC_REALM_75 ", 0) == 0 && saved.find(" HOUSEHOLD_1 1 ") != std::string::npos, "current schema writes sparse household section");
        const bool roundTrip = restored.deserialize(saved, "household-test", error);
        check(roundTrip && restored.household_fund(household) == 35, roundTrip ? "schema 23 round trip preserves fund" : error.c_str());

        std::string schema19 = test_fixtures::save_as_v25(legacy.serialize("household-test"));
        const auto landSection = schema19.find(" LAND_1 ");
        check(landSection != std::string::npos, "current serializer includes land migration section");
        schema19.erase(landSection);
        schema19.replace(0, std::string("POLYEC_REALM_25").size(), "POLYEC_REALM_19");
        const auto fundSection = schema19.find(" HOUSEHOLD_1 ");
        check(fundSection != std::string::npos, "current serializer has migration section");
        schema19.erase(fundSection);
        Realm migrated(1, 1, 128);
        check(migrated.deserialize(schema19, "household-test", error) && migrated.household_fund(household) == 0 && migrated.invariant(), "schema 19 migrates with empty household funds");

        Realm school = Realm::scenario(37, 129, 8, 50, 0, 0);
        const auto schoolHousehold = school.household_of(129);
        const auto guardian = school.guardian(129);
        check(guardian != 0 && school.person(guardian).cash >= 5 && school.available_funds(129) == 0,
              "guardian's private cash is not learner budget");
        check(school.execute(HouseholdFundCommand{guardian, school.person(guardian).decisions + 1,
              schoolHousehold, HouseholdFundOperation::Deposit, 5}).success, "explicit tuition fund deposit");
        const auto beforeTuitionSave = school.serialize("tuition-test");
        Realm resumedSchool(1, 1, 128);
        check(resumedSchool.deserialize(beforeTuitionSave, "tuition-test", error), "tuition save before payment loads");
        const auto tuitionCommand = RealmCommand{129, resumedSchool.person(129).decisions + 1,
            {Intent::Study, resumedSchool.person(129).region, 0, 5, 1}};
        const auto schoolFirm = resumedSchool.view().provinces[resumedSchool.person(129).region].firm;
        const auto ownCash = resumedSchool.person(129).cash;
        const auto tuitionTotal = resumedSchool.total_money();
        check(resumedSchool.available_funds(129) == 5 && resumedSchool.execute(tuitionCommand).success,
              "child pays tuition from household fund");
        check(resumedSchool.household_fund(schoolHousehold) == 0 && resumedSchool.person(129).cash == ownCash &&
              resumedSchool.view().provinces[resumedSchool.person(129).region].firm == schoolFirm + 5 &&
              resumedSchool.total_money() == tuitionTotal && resumedSchool.invariant(), "tuition ledger charges school and conserves money");

        Realm noFundStudy = Realm::scenario(37, 129, 8, 50, 0, 0);
        const auto noFundBefore = noFundStudy.fingerprint();
        check(noFundStudy.person(noFundStudy.guardian(129)).cash >= 5 && noFundStudy.available_funds(129) == 0,
              "private guardian money cannot mask empty household fund");
        const auto rejectedStudy = noFundStudy.execute({129, 1, {Intent::Study, noFundStudy.person(129).region, 0, 5, 1}});
        check(!rejectedStudy.success && noFundStudy.fingerprint() == noFundBefore, "unfunded tuition rejects atomically");

        Realm splitStudy = Realm::scenario(37, 129, 8, 50, 0, 3);
        const auto splitHousehold = splitStudy.household_of(129);
        const auto splitGuardian = splitStudy.guardian(129);
        check(splitStudy.execute(HouseholdFundCommand{splitGuardian, splitStudy.person(splitGuardian).decisions + 1,
              splitHousehold, HouseholdFundOperation::Deposit, 5}).success, "fund split tuition account");
        check(splitStudy.execute({129, 1, {Intent::Study, splitStudy.person(129).region, 0, 5, 1}}).success &&
              splitStudy.person(129).cash == 0 && splitStudy.household_fund(splitHousehold) == 3,
              "tuition spends personal cash before household funds");

        Realm insufficientStudy = Realm::scenario(37, 129, 8, 50, 0, 3);
        const auto insufficientHousehold = insufficientStudy.household_of(129);
        const auto insufficientGuardian = insufficientStudy.guardian(129);
        check(insufficientStudy.execute(HouseholdFundCommand{insufficientGuardian,
              insufficientStudy.person(insufficientGuardian).decisions + 1, insufficientHousehold,
              HouseholdFundOperation::Deposit, 1}).success, "partially fund insufficient tuition");
        const auto insufficientBefore = insufficientStudy.fingerprint();
        check(!insufficientStudy.execute({129, 1, {Intent::Study, insufficientStudy.person(129).region, 0, 5, 1}}).success &&
              insufficientStudy.fingerprint() == insufficientBefore, "combined personal and household shortage rejects atomically");

        Realm food = Realm::scenario(37, 129, 8, 50, 0, 0);
        const auto foodHousehold = food.household_of(129);
        const auto foodAdult = food.household_authority(foodHousehold);
        const auto foodAction = food.candidates(129)[20];
        const auto foodRegion = food.person(129).region;
        const auto foodCost = int64_t(food.view().provinces[size_t(foodRegion)].price[0] + 1) * foodAction.amount;
        check(foodAction.kind == Intent::Buy && foodCost > 0 && food.mask(129)[20] == 0,
              "household fund is required for child food purchase when personal cash is zero");
        check(food.execute(HouseholdFundCommand{foodAdult, food.person(foodAdult).decisions + 1, foodHousehold,
              HouseholdFundOperation::Deposit, foodCost}).success, "deposit food budget");
        const auto foodTotal = food.total_money();
        check(food.execute({129, 1, foodAction}).success, "child buys food from household fund");
        check(food.household_fund(foodHousehold) == 0 && food.person(129).cash == 0 &&
              food.person(129).inventory[0] == 1 && food.total_money() == foodTotal && food.invariant(),
              "personal food expense charges household fund and preserves ledger");

        auto malformed = saved;
        const auto amountAt = malformed.find(" HOUSEHOLD_1 1 ") + std::string(" HOUSEHOLD_1 1 ").size();
        malformed.replace(amountAt, std::to_string(household).size(), "999999");
        const auto before = restored.fingerprint();
        check(!restored.deserialize(malformed, "household-test", error) && restored.fingerprint() == before, "malformed household section leaves destination unchanged");
        std::cout << "household_fund_checks=" << checks << '\n';
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }
}
