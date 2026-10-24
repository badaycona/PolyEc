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
    static void add_personal_tea(Realm& world,uint64_t owner,int64_t amount=1){
        world.mutable_person(owner).inventory[size_t(GoodId::Tea)]+=amount;
        world.opening_[size_t(GoodId::Tea)]+=amount;
    }
    static void set_social(Realm& world,uint64_t owner,int amount){world.mutable_person(owner).social=amount;}
    static int64_t consumed_tea(const Realm& world){return world.consumed_[size_t(GoodId::Tea)];}
    static void set_consumed_tea(Realm& world,int64_t amount){world.consumed_[size_t(GoodId::Tea)]=amount;}
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
        constexpr uint64_t otherCivilian=6;
        constexpr const char* bundle="tea-social-consumer";

        Realm withoutTea(81,1,128);
        const auto absent=withoutTea.candidates(civilian);
        check(absent[size_t(realm_tea_social_candidate)].kind==Intent::Hold,
              "tea social choice is not offered when the actor owns no Tea");
        RealmObservationTestAccess::add_personal_tea(withoutTea,otherCivilian);
        check(withoutTea.candidates(civilian)[size_t(realm_tea_social_candidate)].kind==Intent::Hold,
              "Tea in another person's inventory does not unlock the actor's choice");

        Realm world(82,1,128);
        check(world.model_contract()=="realm-household-fund-goods30-config-v47"&&realm_actions==400,
              "Tea, Copper, Incense and WarHorse travel use the current contract without changing tensor size");
        RealmObservationTestAccess::add_personal_tea(world,civilian,2);
        check(world.invariant(),"test Tea holdings preserve the opening goods ledger");
        const auto offered=world.candidates(civilian);
        const auto teaAction=offered[size_t(realm_tea_social_candidate)];
        check(teaAction.kind==Intent::Social&&teaAction.good==int(GoodId::Tea)&&
              Realm::action_name(teaAction)=="Giao lưu cùng trà"&&world.mask(civilian)[size_t(realm_tea_social_candidate)]==1.f,
              "eligible Civilian sees a separately named, legal Tea social action");
        check(offered[17].kind==Intent::Social&&offered[17].good==0,
              "ordinary social action remains available as a separate choice");

        const auto ordinaryBefore=world.person(civilian).inventory[size_t(GoodId::Tea)];
        const auto ordinary=world.execute({civilian,1,offered[17]});
        check(ordinary.success&&world.person(civilian).inventory[size_t(GoodId::Tea)]==ordinaryBefore&&
              RealmObservationTestAccess::consumed_tea(world)==0&&world.person(civilian).social==2,
              "ordinary social action does not consume carried Tea or change its existing reward");
        world.advance(7);
        const auto teaNow=world.candidates(civilian)[size_t(realm_tea_social_candidate)];
        const auto beforeSocial=world.person(civilian).social;
        const auto beforeTea=world.person(civilian).inventory[size_t(GoodId::Tea)];
        const auto teaResult=world.execute({civilian,2,teaNow});
        check(teaResult.success&&world.person(civilian).inventory[size_t(GoodId::Tea)]==beforeTea-1&&
              RealmObservationTestAccess::consumed_tea(world)==1&&
              world.person(civilian).social==beforeSocial+5&&world.invariant(),
              "Tea social spends one owned item and gives the trial five-point social increase");
        check(world.person(civilian).busy_until==world.day()+7,
              "Tea social uses the existing seven-day busy period");
        const auto beforeBusyReject=world.fingerprint();
        const auto busyReject=world.execute({civilian,3,teaNow});
        check(!busyReject.success&&world.fingerprint()==beforeBusyReject,
              "a second Tea social while busy is rejected without another debit");

        Realm restored(83,1,128);
        std::string error;
        check(restored.deserialize(world.serialize(bundle),bundle,error),
              "Tea holdings, social level and busy time load: "+error);
        check(restored.person(civilian).inventory[size_t(GoodId::Tea)]==1&&
              RealmObservationTestAccess::consumed_tea(restored)==1&&restored.invariant(),
              "save/load preserves Tea inventory debit and consumed-goods ledger");
        const int remaining=std::max(0,restored.person(civilian).busy_until-restored.day());
        restored.advance(remaining);
        world.advance(remaining);
        check(restored.invariant()&&world.fingerprint()==restored.fingerprint(),
              "saved and original worlds continue to the same state after the busy period");

        Realm capped(84,1,128);
        RealmObservationTestAccess::add_personal_tea(capped,civilian);
        RealmObservationTestAccess::set_social(capped,civilian,98);
        const auto capResult=capped.execute({civilian,1,capped.candidates(civilian)[size_t(realm_tea_social_candidate)]});
        check(capResult.success&&capped.person(civilian).social==100&&capped.invariant(),
              "Tea social bonus stops at the existing social maximum of 100");

        Realm wrongRole(85,1,128);
        RealmObservationTestAccess::add_personal_tea(wrongRole,3);
        check(wrongRole.candidates(3)[size_t(realm_tea_social_candidate)].kind!=Intent::Social,
              "Economic role does not receive the Civilian-only Tea social choice");

        Realm overflow(86,1,128);
        RealmObservationTestAccess::add_personal_tea(overflow,civilian);
        RealmObservationTestAccess::set_consumed_tea(overflow,std::numeric_limits<int64_t>::max());
        const auto overflowBefore=overflow.fingerprint();
        const auto overflowResult=overflow.execute({civilian,1,overflow.candidates(civilian)[size_t(realm_tea_social_candidate)]});
        check(!overflowResult.success&&overflow.fingerprint()==overflowBefore,
              "consumed-Tea ledger overflow rejects before any gameplay change");

        std::cout<<"Tea social checks passed: "<<checks<<"\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"Tea social check failed: "<<error.what()<<"\n";
        return 1;
    }
}
