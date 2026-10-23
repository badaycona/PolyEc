#include "polyec/realm.hpp"

#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace polyec;

namespace polyec {
struct RealmObservationTestAccess {
    static void add_secret(Realm& world,uint64_t actor,uint64_t target,int region){world.event(actor,region,777,0,target,true);}
    static void investigate(Realm& world,uint64_t owner,int region){world.investigations_.push_back({world.next_id_++,owner,0,region,world.day()+1,false});}
    static int naive_secret_known(const Realm& world,uint64_t observer){int count=0;for(const auto&fact:world.facts_)if(fact.secret&&world.knows(observer,fact))++count;return count;}
};
}

namespace {
int checks = 0;

void check(bool condition, const char* label) {
    ++checks;
    if (!condition) throw std::runtime_error(label);
}

void compare_snapshot(const Realm& world, uint64_t actor, const char* label) {
    const auto legacyObservation = world.observation(actor);
    const auto legacyCandidates = world.candidates(actor);
    const auto legacyMask = world.mask(actor);

    std::array<Candidate, realm_actions> candidates{};
    const auto reusedObservation = world.observation(actor, candidates);
    const auto reusedMask = world.mask(actor, candidates);
    check(reusedObservation == legacyObservation, label);
    bool candidatesEqual = true;
    for (int action = 0; action < realm_actions; ++action) {
        const auto& left = candidates[size_t(action)];
        const auto& right = legacyCandidates[size_t(action)];
        candidatesEqual = candidatesEqual && left.kind == right.kind && left.target == right.target &&
            left.good == right.good && left.amount == right.amount && left.term == right.term;
    }
    check(candidatesEqual, "snapshot overload preserves every candidate and term");
    check(reusedMask == legacyMask, "snapshot mask overload preserves every legal-action bit");
    for (int action = 0; action < realm_actions; ++action)
        check(reusedObservation[realm_state + action * realm_features + 5] == legacyMask[action],
              "observation mask features equal the standalone mask");
}
}

int main() {
    try {
        auto world = std::make_unique<Realm>(37, 6, 1024);
        const uint64_t governance = 1;
        const uint64_t economic = 52;
        const uint64_t civilian = 103;
        compare_snapshot(*world, governance, "governance decision snapshot matches legacy path");
        compare_snapshot(*world, economic, "economic decision snapshot matches legacy path");
        compare_snapshot(*world, civilian, "civilian decision snapshot matches legacy path");

        auto deposit = world->execute(HouseholdFundCommand{
            governance, world->person(governance).decisions + 1, 1,
            HouseholdFundOperation::Deposit, 1});
        check(deposit.success, "snapshot fixture commits an unrelated household-fund mutation");
        compare_snapshot(*world, governance, "mutated governance decision snapshot remains identical");
        compare_snapshot(*world, economic, "mutated economic decision snapshot remains identical");
        compare_snapshot(*world, civilian, "mutated civilian decision snapshot remains identical");

        RealmObservationTestAccess::add_secret(*world,10,11,0);
        for(uint64_t actor:{10ULL,11ULL,20ULL})
            check(world->observation(actor)[87]==RealmObservationTestAccess::naive_secret_known(*world,actor)/10.f,
                  "secret actor and target visibility matches naive feature oracle");
        RealmObservationTestAccess::investigate(*world,20,0);
        world->advance();
        for(uint64_t actor:{10ULL,11ULL,20ULL})
            check(world->observation(actor)[87]==RealmObservationTestAccess::naive_secret_known(*world,actor)/10.f,
                  "investigation knowledge matches naive feature oracle");
        std::string error;
        const auto serialized=world->serialize("secret-index-test");
        Realm restored(5,6,128);
        if(!restored.deserialize(serialized,"secret-index-test",error))throw std::runtime_error("secret knowledge fixture restores: "+error);
        for(uint64_t actor:{10ULL,11ULL,20ULL})
            check(restored.observation(actor)[87]==RealmObservationTestAccess::naive_secret_known(restored,actor)/10.f,
                  "loaded secret knowledge matches naive feature oracle");
        for(int fact=0;fact<8200;++fact)RealmObservationTestAccess::add_secret(*world,10,11,0);
        for(uint64_t actor:{10ULL,11ULL,20ULL})
            check(world->observation(actor)[87]==RealmObservationTestAccess::naive_secret_known(*world,actor)/10.f,
                  "fact pruning rebuild matches naive feature oracle");
        std::cout << "DECISION SNAPSHOT TESTS: " << checks << " checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after " << checks << ": " << error.what() << '\n';
        return 1;
    }
}






