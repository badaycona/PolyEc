#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
const char* stage="setup";
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
void act(Realm& world,uint64_t actor,Candidate action){
    const auto result=world.execute({actor,world.person(actor).decisions+1,action});
    if(!result.success)throw std::runtime_error("expected military obligation action to succeed: "+result.reason);
    if(!world.invariant())throw std::runtime_error(std::string("military obligation invariant failed at ")+stage+" after command kind="+
        std::to_string(int(action.kind))+" good="+std::to_string(action.good)+" amount="+std::to_string(action.amount));
}
uint64_t proposal_id(const Realm& world,uint64_t recipient){
    const auto candidate=world.candidates(recipient)[28];
    return candidate.kind==Intent::Accept?uint64_t(candidate.target):0;
}
}

int main(){try{
    {
        Realm legacy(9201,6,128);std::string old=legacy.serialize("military-obligation");
        test_fixtures::downgrade_save_52_to_51(old);
        test_fixtures::downgrade_save_51_to_50(old);
        Realm migrated(1,1,128);std::string migrationError;
        check(migrated.deserialize(old,"military-obligation",migrationError)&&migrated.military_obligations().empty()&&
              migrated.invariant(),"schema50 migration creates no invented military obligations");
    }
    Realm world(9201,6,128);constexpr uint64_t governor=2;uint64_t sovereign=0,volunteer=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id)
        if(world.person(id).alive&&world.person(id).family==Family::Governance&&world.person(id).sovereign)sovereign=id;
    for(int slot=realm_military_service_candidate_begin;
        slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot)
        if(world.candidates(governor)[size_t(slot)].kind==Intent::Offer){volunteer=uint64_t(world.candidates(governor)[size_t(slot)].target);break;}
    check(sovereign&&volunteer&&world.person(governor).taxing_authority&&
          world.person(governor).region!=world.person(sovereign).region,"service fixture has a separate local governor and sovereign");
    check(world.invariant(),"opening world preserves its ledgers");
    const int source=world.person(governor).region,destination=world.person(sovereign).region;
    act(world,governor,world.candidates(governor)[size_t(realm_military_service_candidate_begin)]);
    world.advance(10);
    if(!world.invariant())throw std::runtime_error("realm invariant failed before accepting volunteer service");
    check(proposal_id(world,volunteer)!=0,"volunteer receives a service offer after its preparation period");
    act(world,volunteer,world.candidates(volunteer)[28]);
    world.advance(1);
    act(world,governor,{Intent::Mobilize,source,0,40,30});
    const auto offer=world.candidates(governor)[size_t(realm_military_obligation_offer_candidate)];
    check(offer.kind==Intent::Offer&&offer.good==4&&offer.target==int(sovereign)&&offer.amount==1&&
          world.mask(governor)[size_t(realm_military_obligation_offer_candidate)]==1.f,
          "governor can offer one named, already-serving volunteer to the sovereign");
    check(Realm::action_name(offer)=="Đề nghị tập kết quân tình nguyện",
          "the offer has a clear player-facing description");
    const auto unchanged=world.fingerprint();
    check(!world.execute({governor,world.person(governor).decisions+1,
        {Intent::Offer,int(sovereign),4,2,14}}).success&&world.fingerprint()==unchanged,
        "offer larger than the available roster is rejected without partial change");
    act(world,governor,offer);
    world.advance(10);
    const auto pendingOffers=world.offers(sovereign);
    check(pendingOffers.size()==1&&pendingOffers[0].kind==5&&
          pendingOffers[0].roster_members==std::vector<uint64_t>{volunteer},
          "the sovereign can review the exact volunteer IDs before deciding");
    const auto offerSave=world.serialize("military-obligation");Realm offerLoaded(1,1,128);std::string offerError;
    check(offerLoaded.deserialize(offerSave,"military-obligation",offerError)&&
          offerLoaded.serialize("military-obligation")==offerSave&&
          offerLoaded.offers(sovereign).at(0).roster_members==pendingOffers[0].roster_members,
          "a pending offer retains the exact named roster after save and load");
    check(proposal_id(world,sovereign)!=0,"sovereign receives the named muster proposal");
    act(world,sovereign,world.candidates(sovereign)[28]);
    auto obligations=world.military_obligations();
    check(obligations.size()==1&&obligations[0].required_members==1&&obligations[0].member_ids==std::vector<uint64_t>{volunteer}&&
          obligations[0].recipient==sovereign&&obligations[0].source_region==source&&
          obligations[0].destination_region==destination&&obligations[0].due_day==world.day()+14,
          "acceptance creates a fixed, named roster with the agreed recipient and due date");
    check(world.observation(governor)[size_t(realm_military_obligation_required_observation_index)]==.25f&&
          world.observation(governor)[size_t(realm_military_obligation_days_left_observation_index)]==1.f&&
          world.observation(sovereign)[size_t(realm_military_obligation_recipient_observation_index)]==1.f&&
          world.observation(volunteer)[size_t(realm_military_obligation_member_observation_index)]==1.f,
          "the people responsible for delivery, receipt, and travel can each observe their current obligation role");
    const auto midSave=world.serialize("military-obligation");Realm midLoaded(1,1,128);std::string error;
    check(midLoaded.deserialize(midSave,"military-obligation",error)&&midLoaded.serialize("military-obligation")==midSave,
          "an open obligation survives exact save and load");
    uint64_t destination_governor=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& candidate=world.person(id);
        if(candidate.alive&&candidate.family==Family::Governance&&candidate.taxing_authority&&
           candidate.region==destination&&world.view().provinces[size_t(destination)].charter){
            destination_governor=id;break;
        }
    }
    check(destination_governor!=0,"destination has a local public authority able to buy a real ration");
    const auto ration_purchase=world.execute(MilitaryRationPurchaseCommand{
        destination_governor,world.person(destination_governor).decisions+1,destination,1});
    check(ration_purchase.success&&world.military_ration_stock(destination)==1&&world.invariant(),
          "the receiving authority buys one DryFood into its own depot without creating stock");
    const auto travel=world.execute({volunteer,world.person(volunteer).decisions+1,
        {Intent::Migrate,destination,0,10,5+world.distance(source,destination)}});
    check(travel.success,"named volunteer can migrate to the receiving region through the normal travel command");
    world.advance(world.distance(source,destination)+2);
    obligations=world.military_obligations();
    check(obligations[0].delivered_member_ids==std::vector<uint64_t>{volunteer}&&obligations[0].completed_day>=0&&world.invariant(),
          "delivery is recorded once after the volunteer finishes the real journey");
    const auto completedSave=world.serialize("military-obligation");Realm completedLoaded(1,1,128);
    check(completedLoaded.deserialize(completedSave,"military-obligation",error)&&
          completedLoaded.serialize("military-obligation")==completedSave,
          "completed delivery history survives save and load");
    const auto destination_meal_save=world.serialize("muster-destination-ration");
    Realm destination_meal_loaded(1,1,128);
    check(destination_meal_loaded.deserialize(destination_meal_save,"muster-destination-ration",error)&&
          destination_meal_loaded.serialize("muster-destination-ration")==destination_meal_save&&
          destination_meal_loaded.military_ration_stock(destination)==1,
          "save/load preserves the accepted member, actual destination and destination depot stock");
    const auto personal_food_before=world.person(volunteer).inventory[size_t(GoodId::DryFood)];
    const int nutrition_before=world.person(volunteer).nutrition;
    const int health_before=world.person(volunteer).health;
    const int days_to_meal=(int(volunteer%7)-world.view().day%7+7)%7;
    world.advance(days_to_meal==0?7:days_to_meal);
    destination_meal_loaded.advance(days_to_meal==0?7:days_to_meal);
    check(world.person(volunteer).region==destination&&world.person(volunteer).military_service.mobilized_until_day>world.view().day&&
          world.military_ration_stock(destination)==0&&
          world.person(volunteer).inventory[size_t(GoodId::DryFood)]==personal_food_before&&
          world.person(volunteer).nutrition==std::min(100,nutrition_before+20)&&
          world.person(volunteer).health==std::min(100,health_before+1)&&
          destination_meal_loaded.fingerprint()==world.fingerprint()&&world.invariant()&&destination_meal_loaded.invariant(),
          "a named volunteer who reached the receiving region eats one depot ration first, and save/load follows the same meal");
    const auto resignation=world.candidates(volunteer)[31];
    check(resignation.kind==Intent::Resign&&world.execute({volunteer,world.person(volunteer).decisions+1,resignation}).success,
          "the volunteer can still resign through the existing service rule after delivery");
    const auto second_purchase=world.execute(MilitaryRationPurchaseCommand{
        destination_governor,world.person(destination_governor).decisions+1,destination,1});
    check(second_purchase.success&&world.military_ration_stock(destination)==1,
          "the destination authority can restock after the volunteer leaves service");
    world.advance(7);
    check(world.military_ration_stock(destination)==1&&world.invariant(),
          "an accepted roster record does not keep supplying someone whose service has ended");

    stage="shortage fixture";Realm shortage(9201,6,128);const auto shortageGovernor=governor,shortageSovereign=sovereign;
    uint64_t shortageVolunteer=0;
    for(int slot=realm_military_service_candidate_begin;
        slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot)
        if(shortage.candidates(shortageGovernor)[size_t(slot)].kind==Intent::Offer){
            shortageVolunteer=uint64_t(shortage.candidates(shortageGovernor)[size_t(slot)].target);break;
        }
    check(shortageVolunteer!=0,"shortage fixture has a local service volunteer");
    stage="shortage service offer";act(shortage,shortageGovernor,shortage.candidates(shortageGovernor)[size_t(realm_military_service_candidate_begin)]);
    shortage.advance(10);stage="shortage service accept";act(shortage,shortageVolunteer,shortage.candidates(shortageVolunteer)[28]);
    shortage.advance(1);stage="shortage mobilize";act(shortage,shortageGovernor,{Intent::Mobilize,shortage.person(shortageGovernor).region,0,40,30});
    stage="shortage proposal";act(shortage,shortageGovernor,shortage.candidates(shortageGovernor)[size_t(realm_military_obligation_offer_candidate)]);
    shortage.advance(10);stage="shortage sovereign accept";act(shortage,shortageSovereign,shortage.candidates(shortageSovereign)[28]);
    const auto obligation_id=shortage.military_obligations().at(0).id;
    act(shortage,shortageVolunteer,shortage.candidates(shortageVolunteer)[31]);
    shortage.advance(14);
    auto overdue=shortage.military_obligations().at(0);
    check(overdue.due_checked&&overdue.first_deadline_checked&&overdue.shortage_at_first_deadline==1&&
          overdue.delivered_member_ids.empty()&&shortage.candidates(shortageGovernor)[size_t(realm_military_obligation_renegotiate_candidate)].good==5,
          "deadline records the missing named volunteer and offers one negotiation to extend");
    check(shortage.observation(shortageGovernor)[size_t(realm_military_obligation_overdue_observation_index)]==1.f&&
          shortage.observation(shortageGovernor)[size_t(realm_military_obligation_first_shortage_observation_index)]==.25f,
          "the governor policy can observe the overdue flag and historical shortage in normalized state slots");
    const auto firstDue=overdue.due_day;
    stage="extension proposal";act(shortage,shortageGovernor,shortage.candidates(shortageGovernor)[size_t(realm_military_obligation_renegotiate_candidate)]);
    const auto extensionSave=shortage.serialize("military-obligation");Realm extensionLoaded(1,1,128);
    check(extensionLoaded.deserialize(extensionSave,"military-obligation",error)&&
          extensionLoaded.serialize("military-obligation")==extensionSave,
          "a pending extension request and its shortage survive save and load");
    shortage.advance(10);stage="extension accept";act(shortage,shortageSovereign,shortage.candidates(shortageSovereign)[28]);
    overdue=shortage.military_obligations().at(0);
    check(overdue.id==obligation_id&&overdue.extensions==1&&overdue.due_day==shortage.day()+7&&
          overdue.first_due_day==firstDue&&overdue.shortage_at_first_deadline==1&&!overdue.due_checked,
          "sovereign acceptance adds exactly seven days and preserves the first deadline record");
    const auto extendedDue=overdue.due_day;
    shortage.advance(7);overdue=shortage.military_obligations().at(0);
    check(shortage.day()==extendedDue&&overdue.due_checked&&overdue.shortage_at_last_deadline==1&&
          shortage.mask(shortageGovernor)[size_t(realm_military_obligation_renegotiate_candidate)]==0.f&&shortage.invariant(),
          "the final shortage is recorded at the extended date and no second extension is offered");

    Realm stale(9201,6,128);uint64_t staleVolunteer=0;
    for(int slot=realm_military_service_candidate_begin;
        slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot)
        if(stale.candidates(governor)[size_t(slot)].kind==Intent::Offer){
            staleVolunteer=uint64_t(stale.candidates(governor)[size_t(slot)].target);break;
        }
    check(staleVolunteer!=0,"stale offer fixture has a service volunteer");
    act(stale,governor,stale.candidates(governor)[size_t(realm_military_service_candidate_begin)]);
    stale.advance(10);act(stale,staleVolunteer,stale.candidates(staleVolunteer)[28]);stale.advance(1);
    act(stale,governor,{Intent::Mobilize,stale.person(governor).region,0,40,30});
    act(stale,governor,stale.candidates(governor)[size_t(realm_military_obligation_offer_candidate)]);
    stale.advance(10);
    act(stale,staleVolunteer,stale.candidates(staleVolunteer)[31]);
    const auto staleFingerprint=stale.fingerprint();
    check(stale.execute({sovereign,stale.person(sovereign).decisions+1,stale.candidates(sovereign)[28]}).success==false&&
          stale.fingerprint()==staleFingerprint,
          "the sovereign cannot accept a named roster after its volunteer has left service, and rejection is atomic");
    act(stale,sovereign,stale.candidates(sovereign)[29]);
    std::cout<<"Military obligation checks="<<checks<<"\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<"\n";return 1;}}
