#include "polyec/realm.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmApprenticeshipTestAccess {
    static void prepare(Realm& world,uint64_t mentor,uint64_t learner,int region){
        auto& teacher=world.mutable_person(mentor);
        teacher.family=Family::Civilian;teacher.alive=true;teacher.region=region;teacher.destination=-1;
        teacher.age_days=35*365;teacher.skill=45;teacher.busy_until=0;teacher.military_service={};
        auto& student=world.mutable_person(learner);
        student.family=Family::Civilian;student.alive=true;student.region=region;student.destination=-1;
        student.age_days=20*365;student.skill=20;student.busy_until=0;student.military_service={};
        student.cash=100;
        world.refresh_demography();
    }
    static void set_cash(Realm& world,uint64_t id,int64_t value){
        auto& person=world.mutable_person(id);world.opening_money_+=value-person.cash;person.cash=value;
    }
    static void set_skill(Realm& world,uint64_t id,int value){world.mutable_person(id).skill=value;}
};
}

namespace {
int checks=0;
void check(bool ok,const std::string& message){++checks;if(!ok)throw std::runtime_error(message);}
Candidate find_action(const Realm& world,uint64_t actor,Intent kind,uint64_t target=0){
    for(const auto& candidate:world.candidates(actor))
        if(candidate.kind==kind&&(target==0||uint64_t(candidate.target)==target))return candidate;
    return {};
}
uint64_t start_course(Realm& world,uint64_t mentor,uint64_t learner,uint64_t site){
    const auto offer=find_action(world,mentor,Intent::OfferApprenticeship,learner);
    check(offer.kind==Intent::OfferApprenticeship&&uint64_t(offer.good)==site,
          "a qualified workshop owner receives a candidate naming the exact learner and workshop");
    check(world.execute({mentor,world.person(mentor).decisions+1,offer}).success,
          "mentor creates a seven-day offer through the Realm command path");
    const auto accept=world.candidates(learner)[28];
    check(accept.kind==Intent::Accept,"learner receives the existing Accept action for the offer");
    check(world.execute({learner,world.person(learner).decisions+1,accept}).success,
          "learner accepts the offer and starts the course");
    return uint64_t(accept.target);
}
void practice_day(Realm& world,uint64_t mentor,uint64_t learner,uint64_t site){
    for(const uint64_t actor:{mentor,learner}){
        const auto work=find_action(world,actor,Intent::WorkAtSite,site);
        check(work.kind==Intent::WorkAtSite,"participant can schedule Work at the contract workshop");
        check(world.execute({actor,world.person(actor).decisions+1,work}).success,
              "participant schedules one full workshop day through the ordinary Work action");
    }
    world.advance(1);
}
}

int main(){try{
    constexpr uint64_t mentor=129,learner=130;
    Realm world=Realm::scenario(20261091,mentor,35,45);
    RealmApprenticeshipTestAccess::prepare(world,mentor,learner,world.person(mentor).region);
    const int region=world.person(mentor).region;
    const auto created=world.execute(EstablishmentCommand{mentor,world.person(mentor).decisions+1,
        EstablishmentOwnerKind::Person,mentor,region,EstablishmentIndustry::ClothWorkshop,1,25});
    check(created.success,"mentor owns a real, funded workshop in the same region");
    const auto sites=world.establishments();
    check(sites.size()==1,"the apprenticeship fixture has one identifiable workshop");
    const uint64_t site=sites.front().id;
    check(realm_schema==std::string_view("realm-v42")&&
          world.model_contract()=="realm-household-fund-goods30-config-v53"&&
          realm_state==382&&realm_actions==400&&realm_obs==5182,
          "the new action meaning receives a new contract while tensor dimensions stay the same");
    const auto mentorCandidates=world.candidates(mentor);
    const auto offerIndex=std::find_if(mentorCandidates.begin(),mentorCandidates.end(),[&](const Candidate& value){
        return value.kind==Intent::OfferApprenticeship&&value.target==int(learner)&&value.good==int(site);
    });
    check(offerIndex!=mentorCandidates.end()&&
          offerIndex>=mentorCandidates.begin()+realm_apprenticeship_civilian_candidate_begin+1&&
          offerIndex<=mentorCandidates.begin()+70,
          "Civilian apprenticeship offers occupy free slots32–70 without replacing council slot71");
    check(Realm::action_name(*offerIndex).find("Mời học nghề cho người #130 tại xưởng #")==0,
          "the offer action names the learner and the exact workshop in player-facing text");
    const auto governance=world.candidates(1);
    check(governance[size_t(realm_council_nomination_candidate)].kind==Intent::Hold||
          governance[size_t(realm_council_nomination_candidate)].kind==Intent::NominateCouncil,
          "the reserved council action position remains a council action, never an apprenticeship");

    const int64_t learnerCashBefore=world.person(learner).cash;
    const int64_t workshopCashBefore=world.establishments().front().cash;
    const uint64_t courseId=start_course(world,mentor,learner,site);
    check(world.person(learner).cash==learnerCashBefore-kApprenticeshipTrialTuition&&
          world.establishments().front().cash==workshopCashBefore+kApprenticeshipTrialTuition,
          "acceptance transfers the fixed tuition from the learner to the workshop exactly once");
    check(world.apprenticeships().size()==1&&world.apprenticeships().front().id==courseId&&
          world.apprenticeships().front().status==ApprenticeshipStatus::Active&&
          world.apprenticeships().front().practice_days==0&&world.invariant(),
          "the active course records both people, the workshop and zero initial practice days");
    check(world.execute(WorkAuthorizationCommand{mentor,world.person(mentor).decisions+1,site,learner,
          WorkAuthorizationOperation::Revoke,0,0}).success==false,
          "the workshop owner cannot independently revoke a learner's contract work permission");

    for(int day=0;day<4;++day)practice_day(world,mentor,learner,site);
    check(world.apprenticeships().front().practice_days==4&&world.person(learner).skill==20,
          "only four days where both people attend the same workshop count, with no early skill award");
    const int64_t moneyBeforeLoad=world.total_money();
    const int64_t tickBeforeLoad=world.game_tick();
    const std::string halfway=world.serialize("formal-apprenticeship-halfway");
    check(halfway.rfind("POLYEC_REALM_75 ",0)==0&&halfway.find(" APPRENTICESHIP_1 1 ")!=std::string::npos,
          "schema75 saves an in-progress course as a dedicated section");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(halfway,"formal-apprenticeship-halfway",error)&&resumed.invariant(),
          "loading a save in the middle of the course preserves a valid Realm: "+error);
    check(resumed.total_money()==moneyBeforeLoad&&resumed.game_tick()==tickBeforeLoad&&
          resumed.person(learner).skill==20&&resumed.apprenticeships().front().practice_days==4,
          "load does not charge tuition again or duplicate the four recorded practice days");
    std::string malformed=halfway;
    const auto apprenticeshipSection=malformed.rfind(" APPRENTICESHIP_1 ");
    check(apprenticeshipSection!=std::string::npos,"active save has a dedicated apprenticeship section");
    malformed.replace(apprenticeshipSection,std::string::npos," APPRENTICESHIP_1 1000001 ");
    Realm protectedTarget=Realm::scenario(20261095,mentor,35,45);
    const auto protectedFingerprint=protectedTarget.fingerprint();
    check(!protectedTarget.deserialize(malformed,"formal-apprenticeship-halfway",error)&&
          protectedTarget.fingerprint()==protectedFingerprint&&protectedTarget.invariant(),
          "a malformed oversized apprenticeship section is rejected without publishing partial save data");
    for(int day=0;day<6;++day)practice_day(resumed,mentor,learner,site);
    check(resumed.apprenticeships().front().practice_days==10&&
          resumed.apprenticeships().front().status==ApprenticeshipStatus::Completed&&
          resumed.person(learner).skill==30&&resumed.invariant(),
          "ten paired full days complete the course and add ten skill points exactly once");
    const int completedSkill=resumed.person(learner).skill;
    resumed.advance(1);
    check(resumed.person(learner).skill==completedSkill&&
          resumed.apprenticeships().front().practice_days==10,
          "a completed course cannot award the skill increase twice");
    const std::string completed=resumed.serialize("formal-apprenticeship-completed");
    Realm completedLoad(1,1,128);
    check(completedLoad.deserialize(completed,"formal-apprenticeship-completed",error)&&
          completedLoad.apprenticeships().front().status==ApprenticeshipStatus::Completed&&
          completedLoad.person(learner).skill==completedSkill,
          "save/load after completion retains both the result and final skill");

    Realm insufficient=Realm::scenario(20261092,mentor,35,45);
    RealmApprenticeshipTestAccess::prepare(insufficient,mentor,learner,insufficient.person(mentor).region);
    const int insufficientRegion=insufficient.person(mentor).region;
    check(insufficient.execute(EstablishmentCommand{mentor,insufficient.person(mentor).decisions+1,
          EstablishmentOwnerKind::Person,mentor,insufficientRegion,EstablishmentIndustry::ClothWorkshop,1,25}).success,
          "insufficient-funds fixture owns a real workshop");
    const uint64_t insufficientSite=insufficient.establishments().front().id;
    const Candidate insufficientOffer=find_action(insufficient,mentor,Intent::OfferApprenticeship,learner);
    check(insufficient.execute({mentor,insufficient.person(mentor).decisions+1,insufficientOffer}).success,
          "the second learner receives a separate offer");
    RealmApprenticeshipTestAccess::set_cash(insufficient,learner,0);
    const auto beforeReject=insufficient.fingerprint();
    const auto invalidAccept=insufficient.candidates(learner)[28];
    check(!insufficient.mask(learner)[28]&&
          !insufficient.execute({learner,insufficient.person(learner).decisions+1,invalidAccept}).success&&
          insufficient.fingerprint()==beforeReject,
          "insufficient tuition rejects acceptance without changing any other Realm field");
    (void)insufficientSite;

    Realm declined=Realm::scenario(20261096,mentor,35,45);
    RealmApprenticeshipTestAccess::prepare(declined,mentor,learner,declined.person(mentor).region);
    const int declinedRegion=declined.person(mentor).region;
    check(declined.execute(EstablishmentCommand{mentor,declined.person(mentor).decisions+1,
          EstablishmentOwnerKind::Person,mentor,declinedRegion,EstablishmentIndustry::ClothWorkshop,1,25}).success,
          "declining fixture owns a workshop");
    const Candidate declinedOffer=find_action(declined,mentor,Intent::OfferApprenticeship,learner);
    check(declined.execute({mentor,declined.person(mentor).decisions+1,declinedOffer}).success,
          "mentor sends an offer that can be declined without creating a course");
    const auto reject=declined.candidates(learner)[29];
    const int64_t declinedLearnerCash=declined.person(learner).cash;
    const int64_t declinedWorkshopCash=declined.establishments().front().cash;
    check(reject.kind==Intent::Reject&&declined.execute({learner,declined.person(learner).decisions+1,reject}).success&&
          declined.apprenticeships().empty()&&declined.person(learner).cash==declinedLearnerCash&&
          declined.establishments().front().cash==declinedWorkshopCash&&declined.invariant(),
          "the learner can refuse voluntarily; refusal creates no course and transfers no tuition");

    Realm expired=Realm::scenario(20261097,mentor,35,45);
    RealmApprenticeshipTestAccess::prepare(expired,mentor,learner,expired.person(mentor).region);
    const int expiredRegion=expired.person(mentor).region;
    check(expired.execute(EstablishmentCommand{mentor,expired.person(mentor).decisions+1,
          EstablishmentOwnerKind::Person,mentor,expiredRegion,EstablishmentIndustry::ClothWorkshop,1,25}).success,
          "expiry fixture owns a workshop");
    const Candidate expiringOffer=find_action(expired,mentor,Intent::OfferApprenticeship,learner);
    check(expired.execute({mentor,expired.person(mentor).decisions+1,expiringOffer}).success,
          "mentor sends a time-limited offer");
    expired.advance(8);
    check(expired.offers(learner).empty()&&expired.apprenticeships().empty()&&expired.invariant(),
          "an unanswered offer expires after the seven-day response window with no payment or course");

    Realm cancellation=Realm::scenario(20261093,mentor,35,45);
    RealmApprenticeshipTestAccess::prepare(cancellation,mentor,learner,cancellation.person(mentor).region);
    const int cancellationRegion=cancellation.person(mentor).region;
    check(cancellation.execute(EstablishmentCommand{mentor,cancellation.person(mentor).decisions+1,
          EstablishmentOwnerKind::Person,mentor,cancellationRegion,EstablishmentIndustry::ClothWorkshop,1,25}).success,
          "cancellation fixture owns a workshop");
    const uint64_t cancellationSite=cancellation.establishments().front().id;
    (void)start_course(cancellation,mentor,learner,cancellationSite);
    const Candidate end=find_action(cancellation,learner,Intent::EndApprenticeship);
    check(end.kind==Intent::EndApprenticeship&&cancellation.execute({learner,
          cancellation.person(learner).decisions+1,end}).success&&
          cancellation.apprenticeships().front().status==ApprenticeshipStatus::Cancelled&&
          cancellation.invariant(),
          "either named participant can end the contract and remove its temporary work permission");

    Realm legacySource=Realm::scenario(20261094,mentor,35,45);
    std::string legacy=legacySource.serialize("formal-apprenticeship-legacy");
    const auto section=legacy.rfind(" APPRENTICESHIP_1 0 ");
    check(section!=std::string::npos,"schema75 writes the new section even when it contains no courses");
    legacy.erase(section);
    legacy.replace(0,std::string("POLYEC_REALM_75").size(),"POLYEC_REALM_74");
    Realm migrated(1,1,128);
    check(migrated.deserialize(legacy,"formal-apprenticeship-legacy",error)&&
          migrated.apprenticeships().empty()&&migrated.invariant(),
          "schema74 loads with no apprenticeship records and still satisfies all Realm invariants");
    std::cout<<"formal_apprenticeship_checks="<<checks<<"\n";
    return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<"\n";return 1;}}
