#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace polyec;

namespace polyec {
struct RealmTransportTestAccess {
    static void open_all_routes(Realm& world){for(auto& route:world.national_transport_routes_)route.open=true;}
    static void ensure_market_stock(Realm& world,int region,int64_t amount){
        ensure_market_stock(world,region,GoodId::DryFood,amount);
    }
    static void ensure_market_stock(Realm& world,int region,GoodId good,int64_t amount){
        const auto index=size_t(good);
        auto& stock=world.provinces_.at(size_t(region)).stock[index];
        if(stock>=amount)return;
        const auto added=amount-stock;
        if(world.opening_[index]>std::numeric_limits<int64_t>::max()-added)
            throw std::runtime_error("test market stock overflow");
        stock=amount;world.opening_[index]+=added;
    }
    static void suppress_random_death(Realm& world){world.reference_scenario_=true;}
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
std::vector<std::string> tokens(const std::string& text){
    std::istringstream input(text);std::vector<std::string> result;std::string token;
    while(input>>token)result.push_back(token);return result;
}
std::string text(const std::vector<std::string>& values){
    std::ostringstream output;for(const auto& value:values)output<<value<<' ';return output.str();
}
std::string as_schema53_without_new_supply_section(const std::string& save){
    std::string downgraded=save;
    test_fixtures::downgrade_save_58_to_57(downgraded);
    test_fixtures::downgrade_save_56_to_55(downgraded);
    test_fixtures::downgrade_save_55_to_54(downgraded);
    auto values=tokens(downgraded);
    if(values.empty()||values[0]!="POLYEC_REALM_54")throw std::runtime_error("schema54 fixture header missing");
    const auto section=std::find(values.begin(),values.end(),"MILITARY_SUPPLY_OBLIGATION_1");
    if(section==values.end())throw std::runtime_error("new supply section missing");
    values.erase(section,values.end());values[0]="POLYEC_REALM_53";return text(values);
}
struct Fixture{Realm world;uint64_t governor=0,sovereign=0,volunteer=0;int source=0,destination=0;
    Fixture():world(9201,26,128){
        RealmTransportTestAccess::suppress_random_death(world);
        RealmTransportTestAccess::open_all_routes(world);
        for(uint64_t id=1;id<=uint64_t(world.count());++id){
            const auto& person=world.person(id);
            if(person.alive&&person.sovereign)sovereign=id;
        }
        for(uint64_t id=1;id<=uint64_t(world.count());++id){
            const auto& person=world.person(id);
            if(!person.alive||person.family!=Family::Governance||!person.taxing_authority||person.sovereign||
               !world.view().provinces[size_t(person.region)].charter||person.region==world.person(sovereign).region)continue;
            for(int slot=realm_military_service_candidate_begin;
                slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot)
                if(world.candidates(id)[size_t(slot)].kind==Intent::Offer){governor=id;break;}
            if(governor)break;
        }
        if(!sovereign||!governor)throw std::runtime_error("Atlas fixture lacks separate sovereign/governor");
        source=world.person(governor).region;destination=world.person(sovereign).region;
        for(int slot=realm_military_service_candidate_begin;
            slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot)
            if(world.mask(governor)[size_t(slot)]){volunteer=uint64_t(world.candidates(governor)[size_t(slot)].target);break;}
        if(!volunteer)throw std::runtime_error("Atlas fixture lacks eligible volunteer");
        RealmTransportTestAccess::ensure_market_stock(world,source,8);
    }
    void accept_personnel_obligation(){
        int service_offer=-1;
        for(int slot=realm_military_service_candidate_begin;
            slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot)
            if(world.mask(governor)[size_t(slot)]){service_offer=slot;break;}
        act(world,governor,world.candidates(governor)[size_t(service_offer)]);
        world.advance(10);
        act(world,volunteer,world.candidates(volunteer)[28]);
        world.advance(1);
        act(world,governor,{Intent::Mobilize,source,0,40,30});
        const auto offer=world.candidates(governor)[size_t(realm_military_obligation_offer_candidate)];
        check(offer.kind==Intent::Offer&&offer.good==4,"fixture can offer a real accepted volunteer");
        act(world,governor,offer);
        world.advance(10);
        act(world,sovereign,world.candidates(sovereign)[28]);
        check(world.military_obligations().size()==1&&world.invariant(),"personnel obligation accepted before supply promise");
    }
    void buy_and_ship(int64_t quantity,int target=-1){
        if(target<0)target=destination;
        const auto purchase=world.execute(MilitaryRationPurchaseCommand{
            governor,world.person(governor).decisions+1,source,quantity,int(GoodId::DryFood)});
        if(!purchase.success)throw std::runtime_error("public DryFood purchase failed: "+purchase.reason);
        const auto shipment_action=world.candidates(governor)[size_t(realm_military_ration_shipment_candidate_begin+target)];
        check(world.mask(governor)[size_t(realm_military_ration_shipment_candidate_begin+target)]==1.f,
              "a purchased DryFood quantity creates a legal public shipment");
        act(world,governor,shipment_action);
    }
    void buy_and_ship_supplementary(int64_t quantity,int target){
        RealmTransportTestAccess::ensure_market_stock(world,source,GoodId::SupplementaryFood,quantity);
        const auto purchase=world.execute(MilitaryRationPurchaseCommand{
            governor,world.person(governor).decisions+1,source,quantity,int(GoodId::SupplementaryFood)});
        if(!purchase.success)throw std::runtime_error("public SupplementaryFood purchase failed: "+purchase.reason);
        const auto slot=size_t(realm_supplementary_ration_shipment_candidate_begin+target);
        check(world.mask(governor)[slot]==1.f,"purchased SupplementaryFood creates a separate public shipment");
        act(world,governor,world.candidates(governor)[slot]);
    }
};
}

int main(){try{
    Fixture fixture;auto& world=fixture.world;const auto governor=fixture.governor,sovereign=fixture.sovereign;
    fixture.accept_personnel_obligation();

    // This shipment is created before any DryFood promise. It may arrive later,
    // but its earlier ID must never satisfy a commitment made afterward.
    fixture.buy_and_ship(1);
    const auto old_shipment=world.transport_shipments().back().id;
    const auto supply_candidate=world.candidates(governor)[size_t(realm_military_supply_obligation_candidate)];
    check(supply_candidate.kind==Intent::Offer&&supply_candidate.good==6&&supply_candidate.amount==1&&
          supply_candidate.target==int(sovereign)&&world.mask(governor)[size_t(realm_military_supply_obligation_candidate)]==1.f,
          "Governance sees the exact DryFood commitment candidate for its accepted muster");
    check(Realm::action_name(supply_candidate)=="Đề nghị cung cấp lương thực quân vụ",
          "the commitment has an understandable player-facing name");

    const auto old_schema53=as_schema53_without_new_supply_section(world.serialize("supply-obligation"));
    Realm migrated(1,1,128);std::string error;
    check(migrated.deserialize(old_schema53,"supply-obligation",error)&&
          migrated.military_supply_obligations().empty()&&migrated.invariant(),
          "schema53 migration creates no new supply commitments");

    act(world,governor,supply_candidate);world.advance(10);
    check(world.offers(sovereign).size()>=1,"sovereign can review the public supply promise");
    const auto pending=world.serialize("supply-obligation");Realm pending_loaded(1,1,128);
    check(pending_loaded.deserialize(pending,"supply-obligation",error)&&
          pending_loaded.serialize("supply-obligation")==pending&&pending_loaded.offers(sovereign).back().kind==7,
          "pending supply promise survives exact save and load");
    const auto reject_fingerprint=world.fingerprint();
    const auto offered_action=supply_candidate;
    const auto wrong_actor=world.execute({fixture.volunteer,world.person(fixture.volunteer).decisions+1,offered_action});
    if(wrong_actor.success||world.fingerprint()!=reject_fingerprint)
        throw std::runtime_error("wrong-actor rejection changed Realm: success="+std::to_string(wrong_actor.success)+
            " volunteer="+std::to_string(fixture.volunteer)+" family="+std::to_string(int(world.person(fixture.volunteer).family))+
            " governor="+std::to_string(governor)+" offer="+std::to_string(int(offered_action.kind))+","+
            std::to_string(offered_action.good)+","+std::to_string(offered_action.amount)+","+std::to_string(offered_action.term)+
            " reason="+wrong_actor.reason+" before="+std::to_string(reject_fingerprint)+" after="+
            std::to_string(world.fingerprint()));
    check(true,"a volunteer cannot submit the Governance supply proposal");
    act(world,sovereign,world.candidates(sovereign)[29]);
    check(world.military_supply_obligations().empty()&&world.invariant(),
          "Sovereign rejection creates no food obligation and does not seize money");

    const auto retry=world.candidates(governor)[size_t(realm_military_supply_obligation_candidate)];
    check(retry.kind==Intent::Offer&&retry.good==6,"rejected promise can be offered again without changing the source obligation");
    act(world,governor,retry);world.advance(10);act(world,sovereign,world.candidates(sovereign)[28]);
    auto supply=world.military_supply_obligations().at(0);
    check(supply.personnel_obligation_id==world.military_obligations().at(0).id&&supply.required==1&&
          supply.first_eligible_shipment_id>old_shipment&&supply.first_due_day==world.day()+14&&
          supply.delivered==0&&world.invariant(),
          "acceptance fixes one DryFood per named member and excludes the earlier shipment ID");
    const auto accepted=world.serialize("supply-obligation");Realm accepted_loaded(1,1,128);
    check(accepted_loaded.deserialize(accepted,"supply-obligation",error)&&
          accepted_loaded.serialize("supply-obligation")==accepted&&
          accepted_loaded.military_supply_obligations().at(0).delivered==0,
          "accepted supply obligation survives save/load before any eligible delivery");
    int other_destination=0;
    while(other_destination==fixture.source||other_destination==fixture.destination)++other_destination;
    fixture.buy_and_ship(1,other_destination);
    const auto wrong_destination_id=world.transport_shipments().back().id;
    for(int day=0;day<13&&world.transport_shipments().back().delivered==0;++day)world.advance(1);
    check(world.transport_shipments().back().id==wrong_destination_id&&world.transport_shipments().back().delivered==1&&
          world.military_supply_obligations().at(0).delivered==0&&world.invariant(),
          "a real DryFood shipment to a different destination cannot satisfy the promised destination");
    fixture.buy_and_ship_supplementary(1,fixture.destination);
    const auto supplementary_id=world.transport_shipments().back().id;
    for(int day=0;day<13&&world.transport_shipments().back().delivered==0;++day)world.advance(1);
    check(world.transport_shipments().back().id==supplementary_id&&world.transport_shipments().back().delivered==1&&
          world.military_supply_obligations().at(0).delivered==0&&world.invariant(),
          "SupplementaryFood sent through the same route goes to its separate depot and cannot count as DryFood");
    const auto personal_purchase=world.execute({fixture.volunteer,world.person(fixture.volunteer).decisions+1,
        {Intent::Buy,fixture.source,int(GoodId::DryFood),1,0}});
    if(!personal_purchase.success)throw std::runtime_error("volunteer could not buy personal DryFood for shipment exclusion check: "+personal_purchase.reason);
    const auto personal_shipment=world.execute(TransportShipmentCommand{fixture.volunteer,
        world.person(fixture.volunteer).decisions+1,0,TransportShipmentOperation::Create,fixture.destination,
        int(GoodId::DryFood),1,TransportServiceGoal::Fastest,false});
    if(!personal_shipment.success)throw std::runtime_error("personal DryFood shipment was not created: "+personal_shipment.reason);
    for(int day=0;day<13&&world.transport_shipments().back().delivered==0;++day)world.advance(1);
    check(world.transport_shipments().back().owner_kind==TransportShipmentOwnerKind::Person&&
          world.transport_shipments().back().delivered==1&&world.military_supply_obligations().at(0).delivered==0&&world.invariant(),
          "a real personal DryFood shipment to the correct destination cannot satisfy the public Governance commitment");
    fixture.buy_and_ship(1);
    const auto canceled_id=world.transport_shipments().back().id;
    const auto canceled=world.execute(TransportShipmentCommand{governor,world.person(governor).decisions+1,canceled_id,
        TransportShipmentOperation::Cancel,-1,-1,1,TransportServiceGoal::Fastest,true});
    check(canceled.success&&world.transport_shipments().back().canceled==1&&
          world.military_supply_obligations().at(0).delivered==0&&world.invariant(),
          "a canceled public DryFood shipment is refunded and does not count as delivered");
    auto malformed=tokens(accepted);const auto supply_section=std::find(malformed.begin(),malformed.end(),"MILITARY_SUPPLY_OBLIGATION_1");
    if(supply_section==malformed.end())throw std::runtime_error("supply save section missing");
    const auto section_index=size_t(supply_section-malformed.begin());
    malformed.at(section_index+2+13)="-1";
    const auto malformed_fingerprint=accepted_loaded.fingerprint();
    check(!accepted_loaded.deserialize(text(malformed),"supply-obligation",error)&&
          accepted_loaded.fingerprint()==malformed_fingerprint,
          "negative delivered quantity rejects a malformed save without changing the destination Realm");

    world.advance(14);
    supply=world.military_supply_obligations().at(0);
    check(supply.due_checked&&supply.first_deadline_checked&&supply.delivered_at_first_deadline==0&&
          supply.shortage_at_first_deadline==1&&
          world.candidates(governor)[size_t(realm_military_supply_obligation_candidate)].good==7&&world.invariant(),
          "deadline records the missing DryFood and exposes exactly one extension request");
    const auto shortage_save=world.serialize("supply-obligation");Realm shortage_loaded(1,1,128);
    check(shortage_loaded.deserialize(shortage_save,"supply-obligation",error)&&
          shortage_loaded.serialize("supply-obligation")==shortage_save&&
          shortage_loaded.military_supply_obligations().at(0).shortage_at_first_deadline==1,
          "first shortage history survives save/load");
    act(world,governor,world.candidates(governor)[size_t(realm_military_supply_obligation_candidate)]);
    world.advance(10);act(world,sovereign,world.candidates(sovereign)[28]);
    supply=world.military_supply_obligations().at(0);
    check(supply.extensions==1&&supply.due_day==world.day()+7&&supply.shortage_at_first_deadline==1&&
          !supply.due_checked&&world.invariant(),
          "one Sovereign-approved extension adds seven days and keeps the first deadline record");

    fixture.buy_and_ship(1);
    const auto in_transit=world.serialize("supply-obligation");Realm resumed(1,1,128);
    check(resumed.deserialize(in_transit,"supply-obligation",error)&&
          resumed.serialize("supply-obligation")==in_transit&&resumed.invariant(),
          "eligible DryFood shipment and obligation survive save/load while in transit");
    for(int day=0;day<7&&world.military_supply_obligations().at(0).completed_day<0;++day){
        world.advance(1);resumed.advance(1);
    }
    supply=world.military_supply_obligations().at(0);
    check(supply.completed_day>=0&&supply.delivered==1&&supply.shortage_at_first_deadline==1&&
          resumed.fingerprint()==world.fingerprint()&&world.invariant()&&resumed.invariant(),
          "only actual arrival of the later public DryFood shipment completes the obligation and both timelines match");
    const auto completed=world.serialize("supply-obligation");Realm completed_loaded(1,1,128);
    check(completed_loaded.deserialize(completed,"supply-obligation",error)&&
          completed_loaded.serialize("supply-obligation")==completed&&completed_loaded.invariant(),
          "completion date, delivered amount, and first shortage survive exact roundtrip");

    Fixture closure;auto& closing_world=closure.world;
    closure.accept_personnel_obligation();
    const auto commitment=closing_world.candidates(closure.governor)[size_t(realm_military_supply_obligation_candidate)];
    act(closing_world,closure.governor,commitment);closing_world.advance(10);
    act(closing_world,closure.sovereign,closing_world.candidates(closure.sovereign)[28]);
    closing_world.advance(14);
    act(closing_world,closure.governor,closing_world.candidates(closure.governor)[size_t(realm_military_supply_obligation_candidate)]);
    closing_world.advance(10);
    check(closing_world.offers(closure.sovereign).back().kind==8,
          "the Sovereign can review an extension while the promised food is still short");
    closure.buy_and_ship(1);
    for(int day=0;day<7&&closing_world.military_supply_obligations().at(0).completed_day<0;++day)
        closing_world.advance(1);
    bool pending_extension=false;
    for(const auto& offer:closing_world.offers(closure.sovereign))
        if(offer.kind==8&&!offer.resolved)pending_extension=true;
    check(closing_world.military_supply_obligations().at(0).completed_day>=0&&!pending_extension&&closing_world.invariant(),
          "arrival of all promised food closes a still-pending extension offer without leaving an impossible proposal");

    Fixture muster_closure;auto& muster_world=muster_closure.world;
    muster_closure.accept_personnel_obligation();
    const auto muster_commitment=muster_world.candidates(muster_closure.governor)[size_t(realm_military_supply_obligation_candidate)];
    act(muster_world,muster_closure.governor,muster_commitment);
    const auto migration=muster_world.execute({muster_closure.volunteer,
        muster_world.person(muster_closure.volunteer).decisions+1,
        {Intent::Migrate,muster_closure.destination,0,0,0}});
    if(!migration.success)throw std::runtime_error("volunteer could not accept the already-consented move to the muster destination: "+migration.reason);
    muster_world.advance(12);
    bool stale_personnel_offer=false;
    for(const auto& offer:muster_world.offers(muster_closure.sovereign))
        if(offer.kind==7&&!offer.resolved)stale_personnel_offer=true;
    check(muster_world.military_obligations().at(0).completed_day>=0&&!stale_personnel_offer&&muster_world.invariant(),
          "when the volunteer finishes the source muster before response, its pending food offer is withdrawn cleanly");
    std::cout<<"Military supply obligation checks="<<checks<<"\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<"\n";return 1;}}
