#include "polyec/realm.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmObservationTestAccess {
    static void add_incense(Realm& world,uint64_t actor,int64_t amount=1){
        world.mutable_person(actor).inventory[size_t(GoodId::Incense)]+=amount;
        world.opening_[size_t(GoodId::Incense)]+=amount;
    }
    static void set_social(Realm& world,uint64_t actor,int amount){world.mutable_person(actor).social=amount;}
    static void set_cash(Realm& world,uint64_t actor,int64_t amount){world.mutable_person(actor).cash=amount;}
    static int64_t consumed(const Realm& world){return world.consumed_[size_t(GoodId::Incense)];}
    static void set_consumed(Realm& world,int64_t amount){world.consumed_[size_t(GoodId::Incense)]=amount;}
};
}

namespace {
int checks=0;
void check(bool ok,const std::string& message){
    ++checks;
    if(!ok)throw std::runtime_error(message);
}
}

int main(){
    try{
        constexpr uint64_t civilian=5;
        constexpr uint64_t other_civilian=6;
        constexpr const char* save_name="incense-social-consumer";

        Realm without_incense(91,1,128);
        check(without_incense.candidates(civilian)[size_t(realm_incense_social_candidate)].kind==Intent::Hold,
              "Incense social choice is absent when the Civilian owns no Incense");
        RealmObservationTestAccess::add_incense(without_incense,other_civilian);
        check(without_incense.candidates(civilian)[size_t(realm_incense_social_candidate)].kind==Intent::Hold,
              "Incense in another person's inventory does not unlock the choice");

        Realm world(92,1,128);
        check(world.model_contract()=="realm-household-fund-goods30-config-v44"&&realm_actions==400,
              "Incense action meaning has a new model contract without changing action count");
        RealmObservationTestAccess::add_incense(world,civilian,2);
        check(world.invariant(),"test Incense inventory preserves opening goods ledger");
        auto actions=world.candidates(civilian);
        const auto incense_action=actions[size_t(realm_incense_social_candidate)];
        check(incense_action.kind==Intent::Social&&incense_action.good==int(GoodId::Incense)&&
              incense_action.amount==2&&incense_action.term==7&&
              Realm::action_name(incense_action)=="Giao lưu cùng trầm hương"&&
              world.mask(civilian)[size_t(realm_incense_social_candidate)]==1.f,
              "eligible Civilian receives the named, affordable Incense action at slot 25");
        check(actions[size_t(realm_tea_social_candidate)].kind==Intent::Hold&&
              actions[17].kind==Intent::Social,
              "Incense does not unlock Tea, and ordinary Social stays available");

        const int64_t initial_cash=world.person(civilian).cash;
        const int64_t initial_social=world.person(civilian).social;
        const auto ordinary=world.execute({civilian,1,actions[17]});
        check(ordinary.success&&world.person(civilian).inventory[size_t(GoodId::Incense)]==2&&
              RealmObservationTestAccess::consumed(world)==0&&
              world.person(civilian).cash==initial_cash-1&&
              world.person(civilian).social==initial_social+2,
              "ordinary Social keeps its one-coin price, two-point benefit and does not consume Incense");
        world.advance(7);
        actions=world.candidates(civilian);
        const int64_t before_social=world.person(civilian).social;
        const int64_t before_cash=world.person(civilian).cash;
        const auto performed=world.execute({civilian,2,actions[size_t(realm_incense_social_candidate)]});
        check(performed.success&&world.person(civilian).inventory[size_t(GoodId::Incense)]==1&&
              RealmObservationTestAccess::consumed(world)==1&&
              world.person(civilian).cash==before_cash-2&&
              world.person(civilian).social==std::min<int64_t>(100,before_social+8)&&world.invariant(),
              "Incense Social spends one owned item and two coins and adds eight social points");
        check(world.person(civilian).busy_until==world.day()+7,
              "Incense Social uses the existing seven-day game-time duration");
        const auto busy_before=world.fingerprint();
        check(!world.execute({civilian,3,actions[size_t(realm_incense_social_candidate)]}).success&&
              world.fingerprint()==busy_before,
              "busy Civilian cannot repeat the choice and no second debit occurs");

        std::string error;
        Realm resumed(93,1,128);
        check(resumed.deserialize(world.serialize(save_name),save_name,error),
              "Incense stock and social result can be loaded: "+error);
        check(resumed.person(civilian).inventory[size_t(GoodId::Incense)]==1&&
              RealmObservationTestAccess::consumed(resumed)==1&&resumed.invariant(),
              "save preserves remaining Incense and exact consumed ledger");
        Realm uninterrupted=world;
        uninterrupted.advance(7);
        resumed.advance(7);
        check(uninterrupted.fingerprint()==resumed.fingerprint(),
              "saved and uninterrupted worlds continue identically after the busy period");

        Realm capped(94,1,128);
        RealmObservationTestAccess::add_incense(capped,civilian);
        RealmObservationTestAccess::set_social(capped,civilian,98);
        const auto cap_result=capped.execute({civilian,1,capped.candidates(civilian)[size_t(realm_incense_social_candidate)]});
        check(cap_result.success&&capped.person(civilian).social==100&&capped.invariant(),
              "Incense bonus stops at social level 100");

        Realm wrong_role(95,1,128);
        RealmObservationTestAccess::add_incense(wrong_role,3);
        check(wrong_role.candidates(3)[size_t(realm_incense_social_candidate)].kind!=Intent::Social,
              "Economic role does not receive the Civilian-only Incense action");

        Realm insufficient_cash(96,1,128);
        RealmObservationTestAccess::add_incense(insufficient_cash,civilian);
        RealmObservationTestAccess::set_cash(insufficient_cash,civilian,1);
        const auto poor_before=insufficient_cash.fingerprint();
        check(!insufficient_cash.execute({civilian,1,insufficient_cash.candidates(civilian)[size_t(realm_incense_social_candidate)]}).success&&
              insufficient_cash.fingerprint()==poor_before,
              "insufficient personal cash rejects atomically without consuming Incense");

        Realm overflow(97,1,128);
        RealmObservationTestAccess::add_incense(overflow,civilian);
        RealmObservationTestAccess::set_consumed(overflow,std::numeric_limits<int64_t>::max());
        const auto overflow_before=overflow.fingerprint();
        check(!overflow.execute({civilian,1,overflow.candidates(civilian)[size_t(realm_incense_social_candidate)]}).success&&
              overflow.fingerprint()==overflow_before,
              "consumed-Incense ledger overflow rejects before money, goods or social state changes");

        std::cout<<"Incense social checks passed: "<<checks<<"\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"Incense social check failed: "<<error.what()<<'\n';
        return 1;
    }
}
