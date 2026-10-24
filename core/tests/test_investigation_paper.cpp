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
    static void add_personal_paper(Realm& world,uint64_t owner,int64_t quantity=1){
        world.mutable_person(owner).inventory[size_t(GoodId::Paper)]+=quantity;
        world.opening_[size_t(GoodId::Paper)]+=quantity;
    }
    static void add_secret_fact(Realm& world,int region,uint64_t actor){
        world.event(actor,region,777,1,0,true);
    }
    static int64_t consumed_paper(const Realm& world){return world.consumed_[size_t(GoodId::Paper)];}
    static void set_consumed_paper(Realm& world,int64_t quantity){
        world.consumed_[size_t(GoodId::Paper)]=quantity;
    }
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
        constexpr uint64_t investigator=1;
        constexpr uint64_t other=5;
        constexpr const char* bundle="investigation-paper-consumer";

        Realm missingPaper(44,1,128);
        const auto missingCommand=missingPaper.candidates(investigator)[13];
        const auto missingBefore=missingPaper.fingerprint();
        const auto rejected=missingPaper.execute({investigator,1,missingCommand});
        check(!rejected.success,"investigation without the investigator's own Paper is rejected");
        check(missingPaper.fingerprint()==missingBefore&&missingPaper.investigations().empty(),
              "missing Paper rejection changes no money, inventory, ledger, facts or investigation");

        Realm paperOwnedBySomeoneElse(44,1,128);
        RealmObservationTestAccess::add_personal_paper(paperOwnedBySomeoneElse,other);
        const auto otherPaperBefore=paperOwnedBySomeoneElse.fingerprint();
        const auto notBorrowed=paperOwnedBySomeoneElse.execute(
            {investigator,1,paperOwnedBySomeoneElse.candidates(investigator)[13]});
        check(!notBorrowed.success&&paperOwnedBySomeoneElse.fingerprint()==otherPaperBefore&&
              paperOwnedBySomeoneElse.person(other).inventory[size_t(GoodId::Paper)]==1,
              "investigation cannot take Paper from another person's inventory");

        Realm world(44,1,128);
        const int region=world.person(investigator).region;
        RealmObservationTestAccess::add_personal_paper(world,investigator);
        RealmObservationTestAccess::add_secret_fact(world,region,other);
        check(world.invariant(),"test starting inventory and opening-goods ledger balance");
        const auto before=world.person(investigator).inventory[size_t(GoodId::Paper)];
        const auto result=world.execute({investigator,1,world.candidates(investigator)[13]});
        check(result.success,"funded Governance actor with personal Paper can open an investigation");
        check(world.person(investigator).inventory[size_t(GoodId::Paper)]==before-1&&
              RealmObservationTestAccess::consumed_paper(world)==1&&world.invariant(),
              "accepted investigation consumes exactly one personal Paper and balances the goods ledger");
        const auto investigation=world.investigations().back();
        check(!investigation.completed&&investigation.owner==investigator,
              "Paper is consumed when the pending investigation is accepted");
        const auto pendingKnowledge=world.view(investigator).knowledge;
        check(std::none_of(pendingKnowledge.begin(),pendingKnowledge.end(),
              [](const PlayerKnowledge& row){return row.kind==KnowledgeKind::SecretEvidence;}),
              "accepted investigation does not reveal evidence before its due day");

        Realm restored(45,1,128);
        std::string error;
        check(restored.deserialize(world.serialize(bundle),bundle,error),
              "pending investigation and consumed Paper load from save: "+error);
        check(restored.person(investigator).inventory[size_t(GoodId::Paper)]==0&&
              RealmObservationTestAccess::consumed_paper(restored)==1&&restored.invariant(),
              "save/load preserves the personal inventory debit and consumed-goods ledger");
        const int remaining=investigation.due-world.day();
        world.advance(remaining);
        restored.advance(remaining);
        const auto completed=restored.investigations().back();
        const auto knowledge=restored.view(investigator).knowledge;
        check(completed.completed&&std::any_of(knowledge.begin(),knowledge.end(),[&](const PlayerKnowledge& row){
                  return row.source_investigation_id==completed.id&&row.kind==KnowledgeKind::SecretEvidence;
              }),"loaded investigation completes on game time and keeps its evidence source");
        check(restored.invariant()&&world.fingerprint()==restored.fingerprint(),
              "original and loaded worlds reach the same state after investigation completion");

        Realm wrongRegion(44,1,128);
        RealmObservationTestAccess::add_personal_paper(wrongRegion,investigator);
        auto wrongRegionAction=wrongRegion.candidates(investigator)[13];
        wrongRegionAction.target=wrongRegionAction.target==0?1:0;
        const auto wrongRegionBefore=wrongRegion.fingerprint();
        const auto wrongRegionResult=wrongRegion.execute({investigator,1,wrongRegionAction});
        check(!wrongRegionResult.success&&wrongRegion.fingerprint()==wrongRegionBefore,
              "wrong-region investigation is rejected without spending its Paper");

        Realm wrongActor(44,1,128);
        RealmObservationTestAccess::add_personal_paper(wrongActor,other);
        const auto wrongActorBefore=wrongActor.fingerprint();
        const auto wrongActorResult=wrongActor.execute({other,1,wrongActor.candidates(other)[13]});
        check(!wrongActorResult.success&&wrongActor.fingerprint()==wrongActorBefore,
              "non-Governance actor cannot spend Paper to investigate");

        Realm overflow(44,1,128);
        RealmObservationTestAccess::add_personal_paper(overflow,investigator);
        RealmObservationTestAccess::set_consumed_paper(overflow,std::numeric_limits<int64_t>::max());
        const auto overflowBefore=overflow.fingerprint();
        const auto overflowResult=overflow.execute({investigator,1,overflow.candidates(investigator)[13]});
        check(!overflowResult.success&&overflow.fingerprint()==overflowBefore,
              "consumed-Paper ledger overflow rejects before changing the world");

        std::cout<<"Investigation Paper checks passed: "<<checks<<"\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"Investigation Paper check failed: "<<error.what()<<"\n";
        return 1;
    }
}
