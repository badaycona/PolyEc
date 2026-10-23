#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace polyec;

namespace polyec {
struct RealmObservationTestAccess {
    static void set_reference_scenario(Realm& world,bool value){world.reference_scenario_=value;}
    static int64_t produced(const Realm& world,GoodId good){return world.produced_[size_t(good)];}
    static void prepare_routes(Realm& world){for(auto& route:world.national_transport_routes_)route.open=true;}
    static void ensure_market_stock(Realm& world,int region,GoodId good,int64_t amount){
        auto& stock=world.provinces_.at(size_t(region)).stock[size_t(good)];
        if(stock>=amount)return;
        const int64_t added=amount-stock;
        if(world.opening_[size_t(good)]>std::numeric_limits<int64_t>::max()-added)
            throw std::runtime_error("test market stock overflow");
        stock=amount;world.opening_[size_t(good)]+=added;
    }
};
}

namespace {
int checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
void act(Realm& world,uint64_t actor,Candidate action){
    const auto result=world.execute({actor,world.person(actor).decisions+1,action});
    if(!result.success)throw std::runtime_error(std::string("expected action to succeed: ")+result.reason);
    if(!world.invariant())throw std::runtime_error("Realm invariant failed after accepted action");
}
Candidate service_offer(const Realm& world,uint64_t governor){
    for(int slot=realm_military_service_candidate_begin;
        slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot)
        if(world.mask(governor)[size_t(slot)]&&world.candidates(governor)[size_t(slot)].kind==Intent::Offer){
            const auto& candidate=world.person(uint64_t(world.candidates(governor)[size_t(slot)].target));
            const auto view=world.view();
            const auto& province=view.provinces[size_t(candidate.region)];
            const bool becomes_teacher=candidate.age_days>=18*365&&candidate.skill>=20&&
                province.teachers<(province.school+19)/20&&province.treasury>=2;
            if(!becomes_teacher)return world.candidates(governor)[size_t(slot)];
        }
    throw std::runtime_error("fixture has no eligible service offer for a goods-producing worker");
}
struct Actors{uint64_t governor=0,sovereign=0,volunteer=0;int source=0,destination=0;};
Actors actors_for(Realm& world){
    Actors a;
    for(uint64_t id=1;id<=uint64_t(world.count());++id)
        if(world.person(id).alive&&world.person(id).sovereign)a.sovereign=id;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(!person.alive||person.family!=Family::Governance||!person.taxing_authority||person.sovereign||
           !world.view().provinces[size_t(person.region)].charter||person.region==world.person(a.sovereign).region)continue;
        try{(void)service_offer(world,id);a.governor=id;break;}catch(const std::runtime_error&){}
    }
    if(!a.governor||!a.sovereign)throw std::runtime_error("fixture lacks separate Governance and Sovereign actors");
    a.source=world.person(a.governor).region;a.destination=world.person(a.sovereign).region;
    a.volunteer=uint64_t(service_offer(world,a.governor).target);
    RealmObservationTestAccess::ensure_market_stock(world,a.source,GoodId::DryFood,4);
    return a;
}
void close_every_route(Realm& world,uint64_t sovereign){
    for(size_t i=0;i<world.national_transport_routes().size();++i)
        if(world.national_transport_routes()[i].open){
            const auto result=world.execute(TransportRouteCommand{sovereign,world.person(sovereign).decisions+1,uint8_t(i),false});
            if(!result.success)throw std::runtime_error("could not close public route: "+result.reason);
        }
}
void open_every_route(Realm& world,uint64_t sovereign){
    for(size_t i=0;i<world.national_transport_routes().size();++i)
        if(!world.national_transport_routes()[i].open){
            const auto result=world.execute(TransportRouteCommand{sovereign,world.person(sovereign).decisions+1,uint8_t(i),true});
            if(!result.success)throw std::runtime_error("could not reopen public route: "+result.reason);
        }
}
}

int main(){try{
    Realm control(9201,26,128),intervention(9201,26,128);
    RealmObservationTestAccess::set_reference_scenario(control,true);
    RealmObservationTestAccess::set_reference_scenario(intervention,true);
    RealmObservationTestAccess::prepare_routes(control);RealmObservationTestAccess::prepare_routes(intervention);
    const Actors c=actors_for(control),t=actors_for(intervention);
    check(c.governor==t.governor&&c.sovereign==t.sovereign&&c.volunteer==t.volunteer&&c.source==t.source&&c.destination==t.destination,
          "paired worlds start with the same named people, regions, route network, and seed");

    act(control,c.governor,service_offer(control,c.governor));
    act(intervention,t.governor,service_offer(intervention,t.governor));
    control.advance(10);intervention.advance(10);
    act(control,c.volunteer,control.candidates(c.volunteer)[29]);
    act(intervention,t.volunteer,intervention.candidates(t.volunteer)[28]);
    const auto before_service_consent=intervention.fingerprint();
    check(!intervention.execute(MilitaryCombatConsentCommand{
              t.volunteer,intervention.person(t.volunteer).decisions+1,true}).success&&
          intervention.fingerprint()==before_service_consent,
          "a service acceptance that starts tomorrow does not allow consent before the service period begins");
    intervention.advance(1);control.advance(1);
    const auto consent_sequence=intervention.person(t.volunteer).decisions+1;
    const auto consent=intervention.execute(MilitaryCombatConsentCommand{t.volunteer,consent_sequence,true});
    check(consent.success&&intervention.person(t.volunteer).military_service.combat_consent&&intervention.invariant(),
          "only the volunteer can separately consent to combat after their service begins");
    const auto consent_save=intervention.serialize("military-combat-consent");
    Realm consent_restored(1,1,128);std::string consent_error;
    check(consent_restored.deserialize(consent_save,"military-combat-consent",consent_error)&&
          consent_restored.person(t.volunteer).military_service.combat_consent&&
          consent_restored.serialize("military-combat-consent")==consent_save,
          "an active volunteer's combat consent survives an exact save/load roundtrip");
    auto legacy55_consent=consent_save;
    test_fixtures::downgrade_save_56_to_55(legacy55_consent);
    Realm migrated55(1,1,128);
    check(migrated55.deserialize(legacy55_consent,"military-combat-consent",consent_error)&&
          migrated55.person(t.volunteer).military_service.active&&
          !migrated55.person(t.volunteer).military_service.combat_consent&&migrated55.invariant(),
          "schema55 saves keep active service and migrate to no combat consent because that choice did not yet exist");
    consent_restored.advance(31);
    check(!consent_restored.person(t.volunteer).military_service.active&&
          !consent_restored.person(t.volunteer).military_service.combat_consent&&consent_restored.invariant(),
          "the choice is cleared when its service term ends");
    const auto stale_consent_fingerprint=intervention.fingerprint();
    check(!intervention.execute(MilitaryCombatConsentCommand{t.volunteer,consent_sequence,true}).success&&
          intervention.fingerprint()==stale_consent_fingerprint,
          "a repeated decision sequence is rejected without changing consent or world state");
    const auto revoke=intervention.execute(MilitaryCombatConsentCommand{
        t.volunteer,intervention.person(t.volunteer).decisions+1,false});
    check(revoke.success&&!intervention.person(t.volunteer).military_service.combat_consent&&intervention.invariant(),
          "the volunteer can withdraw combat consent while still serving");
    const auto unauthorized_fingerprint=intervention.fingerprint();
    check(!intervention.execute(MilitaryCombatConsentCommand{
              t.governor,intervention.person(t.governor).decisions+1,true}).success&&
          intervention.fingerprint()==unauthorized_fingerprint,
          "a commander cannot grant combat consent on the volunteer's behalf");
    const auto treasury_before=intervention.view().provinces[size_t(t.source)].treasury;
    const auto firm_before=intervention.view().provinces[size_t(t.source)].firm;
    act(intervention,t.governor,{Intent::Mobilize,t.source,0,40,30});
    check(intervention.view().provinces[size_t(t.source)].treasury==treasury_before-40&&
          intervention.view().provinces[size_t(t.source)].firm==firm_before+40,
          "the Mobilize command transfers its existing 40 cash from regional treasury to firm");
    const auto muster_offer=intervention.candidates(t.governor)[size_t(realm_military_obligation_offer_candidate)];
    check(muster_offer.kind==Intent::Offer&&muster_offer.target==int(t.sovereign),
          "Governance can offer the newly mobilized volunteer before the service window becomes too short");
    act(intervention,t.governor,muster_offer);
    const int work_region=intervention.person(t.volunteer).region;
    const int work_good=canonical_from_legacy_good(work_region%7);
    const auto intervention_production_before=RealmObservationTestAccess::produced(intervention,GoodId(work_good));
    const auto control_production_before=RealmObservationTestAccess::produced(control,GoodId(work_good));
    const auto control_work=control.candidates(c.volunteer)[size_t(1+work_region)];
    const Candidate same_work{Intent::Work,work_region,0,0,30};
    check(control_work.kind==Intent::Work,"the volunteer who declined service can take the matched Work action");
    act(control,c.volunteer,control_work);
    const auto rejected_fingerprint=intervention.fingerprint();
    check(!intervention.execute({t.volunteer,intervention.person(t.volunteer).decisions+1,same_work}).success&&
          intervention.fingerprint()==rejected_fingerprint,
          "the same Work action is rejected atomically while service is active");
    const auto payroll_before_interval=intervention.person(t.volunteer).military_service.paid_total;
    intervention.advance(8);control.advance(8);
    const auto treatment_output=RealmObservationTestAccess::produced(intervention,GoodId(work_good))-intervention_production_before;
    const auto control_output=RealmObservationTestAccess::produced(control,GoodId(work_good))-control_production_before;
    const auto payroll_paid_window=intervention.person(t.volunteer).military_service.paid_total-payroll_before_interval;
    check(treatment_output==0&&control_output>0&&
          payroll_paid_window==8*kMilitaryServiceTrialWagePerDay&&
          intervention.invariant()&&control.invariant(),
          "over eight matched days service prevents Work output while public payroll pays the existing daily wage");

    intervention.advance(2);control.advance(2);
    act(intervention,t.sovereign,intervention.candidates(t.sovereign)[28]);
    check(intervention.military_obligations().size()==1&&
          intervention.military_obligations()[0].member_ids==std::vector<uint64_t>{t.volunteer},
          "Sovereign acceptance creates a roster naming the volunteer after the matched work interval");
    const auto supply_offer=intervention.candidates(t.governor)[size_t(realm_military_supply_obligation_candidate)];
    check(supply_offer.kind==Intent::Offer&&supply_offer.target==int(t.sovereign),
          "the open accepted muster exposes a supply promise to its Sovereign");
    act(intervention,t.governor,supply_offer);intervention.advance(10);
    act(intervention,t.sovereign,intervention.candidates(t.sovereign)[28]);
    check(intervention.military_supply_obligations().size()==1&&intervention.military_supply_obligations()[0].required==1,
          "Sovereign consent fixes one DryFood for the named volunteer before travel");

    const auto migration=intervention.execute({t.volunteer,intervention.person(t.volunteer).decisions+1,
        {Intent::Migrate,t.destination,0,10,5+intervention.distance(t.source,t.destination)}});
    if(!migration.success)throw std::runtime_error("the consenting volunteer could not start the public migration command: "+migration.reason);
    intervention.advance(intervention.distance(t.source,t.destination)+2);
    check(intervention.person(t.volunteer).region==t.destination&&
          intervention.military_obligations()[0].delivered_member_ids==std::vector<uint64_t>{t.volunteer}&&
          intervention.military_obligations()[0].completed_day>=0&&intervention.invariant(),
          "only arrival through the real journey completes the named muster roster");

    close_every_route(intervention,t.sovereign);
    intervention.advance(14);
    const auto shortage=intervention.military_supply_obligations()[0];
    check(shortage.due_checked&&shortage.shortage_at_first_deadline==1&&shortage.delivered==0&&
          intervention.invariant(),"closed routes leave the promised depot empty and record one DryFood shortage");
    const auto extension_offer=intervention.candidates(t.governor)[size_t(realm_military_supply_obligation_candidate)];
    check(extension_offer.kind==Intent::Offer&&extension_offer.good==7,
          "Governance can ask for the one permitted extension after the first shortage");
    act(intervention,t.governor,extension_offer);intervention.advance(10);
    check(intervention.offers(t.sovereign).back().kind==8,
          "the exact same shortage state now contains a ready Sovereign decision");
    const auto shortage_save=intervention.serialize("military-mobilization-chain");
    Realm rejected(1,1,128),accepted(1,1,128);std::string error;
    check(rejected.deserialize(shortage_save,"military-mobilization-chain",error)&&
          accepted.deserialize(shortage_save,"military-mobilization-chain",error)&&
          rejected.serialize("military-mobilization-chain")==shortage_save&&accepted.invariant(),
          "the identical shortage state roundtrips before the two Sovereign decisions");
    const auto reject_before=rejected.fingerprint();
    const auto reject_total_money=rejected.total_money();
    const auto reject_source=rejected.view().provinces[size_t(t.source)];
    const auto reject_destination=rejected.view().provinces[size_t(t.destination)];
    const auto reject_shipment_count=rejected.transport_shipments().size();
    act(rejected,t.sovereign,rejected.candidates(t.sovereign)[29]);
    check(rejected.fingerprint()!=reject_before&&rejected.military_supply_obligations()[0].extensions==0&&
          rejected.military_supply_obligations()[0].due_day==shortage.due_day&&
          rejected.military_supply_obligations()[0].shortage_at_first_deadline==shortage.shortage_at_first_deadline&&
          rejected.total_money()==reject_total_money&&
          rejected.view().provinces[size_t(t.source)].treasury==reject_source.treasury&&
          rejected.view().provinces[size_t(t.source)].firm==reject_source.firm&&
          rejected.view().provinces[size_t(t.destination)].treasury==reject_destination.treasury&&
          rejected.view().provinces[size_t(t.destination)].firm==reject_destination.firm&&
          rejected.military_ration_stock(t.destination)==0&&
          rejected.transport_shipments().size()==reject_shipment_count,
          "rejecting records the decision while preserving money, depots, shipments, shortage, and deadline");
    act(accepted,t.sovereign,accepted.candidates(t.sovereign)[28]);
    check(accepted.military_supply_obligations()[0].extensions==1&&
          accepted.military_supply_obligations()[0].shortage_at_first_deadline==1&&
          accepted.military_supply_obligations()[0].due_day==accepted.day()+7,
          "accepting starts the one permitted seven-day extension at the decision date and preserves first shortage history");
    open_every_route(accepted,t.sovereign);
    RealmObservationTestAccess::ensure_market_stock(accepted,t.source,GoodId::DryFood,1);
    const auto purchase=accepted.execute(MilitaryRationPurchaseCommand{t.governor,
        accepted.person(t.governor).decisions+1,t.source,1,int(GoodId::DryFood)});
    if(!purchase.success)throw std::runtime_error("Governance could not buy promised DryFood: "+purchase.reason);
    const auto shipment_candidate=accepted.candidates(t.governor)[size_t(realm_military_ration_shipment_candidate_begin+t.destination)];
    check(accepted.mask(t.governor)[size_t(realm_military_ration_shipment_candidate_begin+t.destination)]==1.f,
          "the real purchase creates a legal public shipment to the promised region");
    act(accepted,t.governor,shipment_candidate);
    const auto& shipment=accepted.transport_shipments().back();
    check(shipment.owner_kind==TransportShipmentOwnerKind::MilitaryRationDepot&&shipment.source_region==t.source&&
          shipment.destination==t.destination&&shipment.good==int(GoodId::DryFood)&&shipment.quantity==1,
          "the shipment ledger names the public depot owner, correct regions, DryFood, and exact quantity");
    const auto transit_save=accepted.serialize("military-mobilization-chain");Realm resumed(1,1,128);
    check(resumed.deserialize(transit_save,"military-mobilization-chain",error)&&
          resumed.serialize("military-mobilization-chain")==transit_save,
          "the real shipment in transit survives exact save and load");
    for(int day=0;day<14&&accepted.military_supply_obligations()[0].completed_day<0;++day){accepted.advance(1);resumed.advance(1);}
    check(accepted.military_supply_obligations()[0].completed_day>=0&&
          accepted.military_supply_obligations()[0].delivered==1&&
          accepted.military_supply_obligations()[0].shortage_at_first_deadline==1&&
          accepted.transport_shipments().back().delivered==1&&
          resumed.fingerprint()==accepted.fingerprint()&&accepted.invariant()&&resumed.invariant(),
          "only actual arrival fills the depot and completes the obligation while both loaded timelines agree");
    std::cout<<"Military mobilization chain checks="<<checks
             <<"; seed=9201 regions=26; work_output_control="<<control_output
             <<" work_output_during_service="<<treatment_output
             <<" payroll_paid_during_matched_eight_days="<<payroll_paid_window
             <<" first_shortage="<<shortage.shortage_at_first_deadline<<" final_delivered="
             <<accepted.military_supply_obligations()[0].delivered<<'\n';
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
