// Bundled Zig's libc++ exposes std::jthread only with this experimental flag.
// Keep the workaround scoped to Clang-based toolchains.
#if defined(__clang__)
#define _LIBCPP_ENABLE_EXPERIMENTAL
#endif
#include "polyec/realm.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <thread>
#include <exception>
#ifdef POLYEC_AI_ENABLED
#include "polyec/policy.hpp"
#endif
namespace py=pybind11;using namespace polyec;
py::dict realm_view(const Realm&w,uint64_t observer){
    const auto v=w.view(observer);py::dict d;
    d["day"]=v.day;d["persons"]=v.persons;d["alive"]=v.alive;d["children"]=v.children;
    d["money"]=v.money;d["decisions"]=v.decisions;d["cards"]=v.draft.cards;d["chosen"]=v.draft.chosen;
    py::list ps;for(auto&p:v.provinces){py::dict x;py::list stock,prices,goodStock,goodPrices;
        for(int legacy=0;legacy<kLegacyGoodCount;++legacy){const auto good=size_t(canonical_from_legacy_good(legacy));stock.append(p.stock[good]);prices.append(p.price[good]);}
        for(size_t good=0;good<kGoodIdCount;++good){goodStock.append(p.stock[good]);goodPrices.append(p.price[good]);}
        x["stock"]=stock;x["prices"]=prices;x["good_stock"]=goodStock;x["good_prices"]=goodPrices;
        x["treasury"]=p.treasury;x["firm"]=p.firm;x["unrest"]=p.unrest;
        x["military_ration_stock"]=w.military_ration_stock(int(ps.size()));
        x["military_supplementary_ration_stock"]=w.military_ration_stock(int(ps.size()),int(GoodId::SupplementaryFood));
        x["road"]=p.road;x["jobs"]=p.jobs;x["employed"]=p.employed;x["school"]=p.school;x["school_used"]=p.school_used;ps.append(x);}
    d["provinces"]=ps;
    py::list census;const auto& observer_person=w.person(observer);
    const auto councils=w.municipal_councils();
    for(const auto& row:w.urban_census())if(observer_person.sovereign||row.territory_id==observer_person.region){
        py::dict item;item["site_id"]=row.site_id;item["territory_id"]=row.territory_id;
        item["urban_residents"]=row.urban_residents;item["rural_residents"]=row.rural_residents;
        item["unknown_residence"]=row.unknown_residence;item["housing_capacity"]=row.housing_capacity;
        item["unhoused_residents"]=row.unhoused_residents;item["available_beds"]=row.available_beds;
        if(row.site_id<councils.size()){
            const auto& council=councils[row.site_id];
            item["council_representative_id"]=council.representative;
            item["council_nominee_id"]=council.nominee;
            item["council_nomination_day"]=council.nomination_day;
            item["council_term_end_day"]=council.term_end_day;
        }
        census.append(item);
    }
    d["urban_census"]=census;
    py::list residences;const auto holdings=w.land_holdings();
    for(const auto& home:w.private_residences()){
        const auto holding=std::find_if(holdings.begin(),holdings.end(),
            [&](const LandHolding& value){return value.id==home.holding_id;});
        const bool household_party=home.tenant_household==observer_person.household_membership.household_id||
            home.offered_household==observer_person.household_membership.household_id;
        const bool is_title_holder=holding!=holdings.end()&&holding->title_holder==observer;
        if(!observer_person.sovereign&&home.region!=observer_person.region&&!is_title_holder&&!household_party)continue;
        py::dict item;item["id"]=home.id;item["holding_id"]=home.holding_id;item["region"]=home.region;
        item["completed"]=home.completed;item["construction_due_day"]=home.construction_due_day;
        item["tenant_household"]=home.tenant_household;item["offered_household"]=home.offered_household;
        item["offered_rent"]=home.offered_rent;item["offer_expires_day"]=home.offer_expires_day;
        item["rent_per_period"]=home.rent_per_period;item["next_rent_day"]=home.next_rent_day;
        item["lease_end_day"]=home.lease_end_day;residences.append(item);
    }
    d["private_residences"]=residences;
    py::list fs;for(auto&f:v.facts){py::dict x;x["id"]=f.id;x["parent"]=f.parent;x["kind"]=f.kind;x["actor"]=f.actor;x["day"]=f.day;x["amount"]=f.amount;fs.append(x);}d["facts"]=fs;
    py::list knowledge;for(const auto&k:v.knowledge){py::dict x;x["observer"]=k.observer;x["fact"]=k.fact;x["arrives"]=k.arrives;
        x["source_investigation_id"]=k.source_investigation_id;x["kind"]=unsigned(k.kind);x["confidence_basis_points"]=k.confidence_basis_points;knowledge.append(x);}
    d["knowledge"]=knowledge;return d;
}
class RealmBatch {
    struct Outcome {bool alive=false,insolvent=false;int health=0,nutrition=0,skill=0,days=0,due=0,defaults=0,decisions=0;int64_t cash=0,debt=0;bool authority=false;double reference=1,raw_utility=0;};
    std::vector<Realm> worlds;std::vector<uint64_t> actors;std::vector<int> steps;std::vector<double> opening;std::vector<Outcome> outcomes_;
    std::vector<uint8_t> finished_;std::vector<float> final_scores_;std::vector<std::mt19937_64> partner_rng_;
    uint64_t seed_;int role_,regions_,horizon_,workers_;
    uint64_t actor(uint64_t seed){if(role_==0)return seed%2==0?128:1+seed%(regions_+1);if(role_==1)return 2+regions_+seed%(regions_+1);return 2*(regions_+1)+1+seed%(128-2*(regions_+1));}
    void reset(int i){uint64_t seed=seed_++;worlds[i]=Realm(seed,regions_,128,role_==0&&seed%2==0);worlds[i].choose(worlds[i].draft().cards[seed%3]);actors[i]=actor(seed);if(role_==2)while(worlds[i].person(actors[i]).candidate){actors[i]=2*(regions_+1)+1+(actors[i]-2*(regions_+1))%(128-2*(regions_+1));}steps[i]=0;opening[i]=worlds[i].utility(actors[i]);finished_[i]=0;final_scores_[i]=0;partner_rng_[i].seed(seed^0xd0a1b41e91ull);}
    void transition(int i,int64_t action,float&reward,bool&done,float&score){
        if(finished_[i]){reward=0;done=true;score=final_scores_[i];return;}
        auto&w=worlds[i];auto id=actors[i];double before=w.utility(id);
        auto result=w.execute({id,w.person(id).decisions+1,w.candidates(id)[action]});if(!result.success)throw std::runtime_error(result.reason);
        for(int day=0;day<(role_==1?7:30);++day){
            // Explicit training partners only: civilian work/study; other families
            // uniformly sample legal actions. App uses learned actors instead.
            for(auto family:{Family::Governance,Family::Economic,Family::Civilian})for(auto peer:w.due(family))if(peer!=id){
                auto m=w.mask(peer);int a=0;
                if(family==Family::Civilian){int work=1+w.person(peer).region;a=m[work]?work:(m[15]?15:0);}
                else {std::vector<int> legal;for(int j=0;j<realm_actions;++j)if(m[j])legal.push_back(j);a=legal[partner_rng_[i]()%legal.size()];}
                auto r=w.execute({peer,w.person(peer).decisions+1,w.candidates(peer)[a]});if(!r.success)throw std::runtime_error("Training partner commit failed");
            }
            w.advance();if(!w.person(id).alive)break;
        }
        if(!w.invariant())throw std::runtime_error("Realm invariant failure");
        reward=float((w.utility(id)-before)/std::max(1.,std::abs(opening[i])));done=++steps[i]>=horizon_||!w.person(id).alive;score=done?float((w.utility(id)-opening[i])/std::max(1.,std::abs(opening[i]))):0;
        if(done){auto&p=w.person(id);auto&o=outcomes_[i];o={};o.alive=p.alive;o.health=p.health;o.nutrition=p.nutrition;o.cash=p.cash;o.skill=p.skill;o.debt=w.debt(id);o.days=w.day();o.decisions=steps[i];o.authority=p.taxing_authority;o.reference=std::max(1.,std::abs(opening[i]));o.raw_utility=w.utility(id)-opening[i];o.insolvent=p.cash<o.debt;for(auto&l:w.loans())if(l.debtor==id&&l.due<=w.day()){++o.due;o.defaults+=l.defaulted;}finished_[i]=1;final_scores_[i]=score;}
    }
public:
    RealmBatch(int count,uint64_t seed,int role,int regions,int horizon,int workers=4):seed_(seed),role_(role),regions_(regions),horizon_(horizon),workers_(workers){
        if(count<1||count>128||role<0||role>2||horizon<1||horizon>1024||workers<1||workers>8)throw std::invalid_argument("Realm batch bounds");
        for(int i=0;i<count;++i)worlds.emplace_back(1,regions,128);actors.resize(count);steps.resize(count);opening.resize(count);outcomes_.resize(count);finished_.resize(count);final_scores_.resize(count);partner_rng_.resize(count);for(int i=0;i<count;++i)reset(i);
    }
    py::tuple observe(){py::array_t<float>obs({int(worlds.size()),realm_obs}),mask({int(worlds.size()),realm_actions});for(int i=0;i<int(worlds.size());++i){auto o=worlds[i].observation(actors[i]);auto m=worlds[i].mask(actors[i]);if(finished_[i]){m.fill(0);m[0]=1;}std::copy(o.begin(),o.end(),obs.mutable_data()+i*realm_obs);std::copy(m.begin(),m.end(),mask.mutable_data()+i*realm_actions);}return py::make_tuple(obs,mask);}
    py::list outcomes()const{py::list result;for(auto&o:outcomes_){py::dict d;d["alive"]=o.alive;d["health"]=o.health;d["nutrition"]=o.nutrition;d["cash"]=o.cash;d["skill"]=o.skill;d["debt"]=o.debt;d["days"]=o.days;d["decisions"]=o.decisions;d["taxing_authority"]=o.authority;d["reference_utility"]=o.reference;d["raw_utility_delta"]=o.raw_utility;d["insolvent"]=o.insolvent;d["obligations_due"]=o.due;d["defaults"]=o.defaults;result.append(d);}return result;}
    py::tuple step(py::array_t<int64_t,py::array::c_style|py::array::forcecast>actions,bool auto_reset=true){
        int n=int(worlds.size());if(actions.ndim()!=1||actions.size()!=n)throw std::invalid_argument("Realm action shape");
        for(int i=0;i<n;++i)if(actions.data()[i]<0||actions.data()[i]>=realm_actions||(finished_[i]?actions.data()[i]!=0:!worlds[i].mask(actors[i])[actions.data()[i]]))throw std::invalid_argument("Invalid realm action");
        py::array_t<float> rewards(n),scores(n);py::array_t<bool>dones(n);
        auto*act=actions.data();auto*r=rewards.mutable_data();auto*sc=scores.mutable_data();auto*done=dones.mutable_data();
        std::array<std::exception_ptr,8>errors{};
        {py::gil_scoped_release release;std::vector<std::jthread>threads;int count=std::min(workers_,n);
            for(int worker=0;worker<count;++worker)threads.emplace_back([&,worker,count]{try{for(int i=worker;i<n;i+=count)transition(i,act[i],r[i],done[i],sc[i]);}catch(...){errors[worker]=std::current_exception();}});
        }
        for(auto&e:errors)if(e)std::rethrow_exception(e);
        if(auto_reset)for(int i=0;i<n;++i)if(done[i])reset(i);
        auto o=observe();return py::make_tuple(o[0],o[1],rewards,dones,scores);
    }
};
class RealmNativeTag {};
#ifdef POLYEC_AI_ENABLED
class RealmNativePolicy:public NativePolicy {public:using NativePolicy::NativePolicy;};
#endif
void bind_realm(py::module_&m){m.attr("REALM_OBS")=realm_obs;m.attr("REALM_ACTIONS")=realm_actions;m.attr("REALM_SCHEMA")=realm_schema;m.attr("REALM_REWARD_VERSION")=realm_reward_version;m.attr("REALM_TICKS_PER_DAY")=kSubdayUnitsPerDay;
    m.attr("REALM_PRIVATE_HOUSING_CANDIDATE_BEGIN")=realm_private_housing_candidate_begin;
    m.attr("REALM_PRIVATE_BUILD_CANDIDATE_BEGIN")=realm_private_build_candidate_begin;
    m.attr("REALM_PRIVATE_BUILD_CANDIDATE_COUNT")=realm_private_build_candidate_count;
    m.attr("REALM_PRIVATE_OFFER_CANDIDATE_BEGIN")=realm_private_offer_candidate_begin;
    m.attr("REALM_PRIVATE_OFFER_CANDIDATE_COUNT")=realm_private_offer_candidate_count;
    m.attr("REALM_PRIVATE_RESPONSE_CANDIDATE_BEGIN")=realm_private_response_candidate_begin;
    m.attr("REALM_PRIVATE_RESPONSE_CANDIDATE_COUNT")=realm_private_response_candidate_count;
    m.attr("REALM_PRIVATE_END_LEASE_CANDIDATE_BEGIN")=realm_private_end_lease_candidate_begin;
    m.attr("REALM_PRIVATE_END_LEASE_CANDIDATE_COUNT")=realm_private_end_lease_candidate_count;
    py::enum_<MarketOrderOperation>(m,"MarketOrderOperation")
        .value("Place",MarketOrderOperation::Place).value("Cancel",MarketOrderOperation::Cancel);
    py::class_<Realm>(m,"Realm").def(py::init([](uint64_t seed,int regions,int people,const std::string& config_path,bool taxing_authority,uint32_t capability_mask){
        const auto config=config_path.empty()?GameplayConfig{}:GameplayConfig::load(config_path);
        return Realm(seed,regions,people,taxing_authority,config,0,capability_mask);
    }),py::arg("seed")=1,py::arg("regions")=6,py::arg("people")=10000,py::arg("config_path")="",py::arg("taxing_authority")=false,py::arg("capability_mask")=kDefaultCapabilityMask)
    .def_property_readonly("config_fingerprint",[](const Realm&w){return w.gameplay_config().fingerprint();})
    .def_property_readonly("config_values",[](const Realm&w){return w.gameplay_config().values();})
    .def_property_readonly("work_ticks_per_unit",[](const Realm&w){return w.gameplay_config().ticks_per_unit();})
    .def_property_readonly("capability_mask",&Realm::capability_mask)
    .def("work_pay_remainder",&Realm::work_pay_remainder)
    .def("view",&realm_view,py::arg("observer")=1)
    .def("debt",&Realm::debt)
    .def("loans",[](const Realm&w){py::list result;for(const auto&loan:w.loans()){py::dict row;row["id"]=loan.id;row["debtor"]=loan.debtor;row["due"]=loan.due;row["defaulted"]=loan.defaulted;row["principal"]=loan.principal;row["interest"]=loan.interest;row["paid"]=loan.paid;result.append(row);}return result;})
    .def("observe",static_cast<std::array<float,realm_obs>(Realm::*)(uint64_t)const>(&Realm::observation))
    .def("mask",static_cast<std::array<float,realm_actions>(Realm::*)(uint64_t)const>(&Realm::mask))
    .def("advance",&Realm::advance,py::arg("days")=1).def("invariant",[](const Realm&w){return w.invariant();}).def("fingerprint",&Realm::fingerprint).def("due",[](Realm&w,int role){return w.due(Family(role));})
    .def("step",[](Realm&w,uint64_t id,int a){if(a<0||a>=realm_actions)throw std::invalid_argument("Action bounds");auto result=w.execute({id,w.person(id).decisions+1,w.candidates(id)[a]});return py::make_tuple(result.success,result.reason);})
    .def("intent_command",[](Realm&w,uint64_t actor,uint64_t sequence,int intent,int target,int good,int64_t amount,int term){
        const auto result=w.execute(RealmCommand{actor,sequence,{static_cast<Intent>(intent),target,good,amount,term}});
        return py::make_tuple(result.success,result.reason,result.fact);
    },py::arg("actor"),py::arg("sequence"),py::arg("intent"),py::arg("target")=0,py::arg("good")=0,py::arg("amount")=0,py::arg("term")=0)
    .def("private_residence_build",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t holding_id){
        const auto result=w.execute(PrivateResidenceBuildCommand{actor,sequence,holding_id});
        return py::make_tuple(result.success,result.reason,result.fact);
    },py::arg("actor"),py::arg("sequence"),py::arg("holding_id"))
    .def("private_housing_command",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t residence_id,
                                      int operation,uint64_t tenant_household,int64_t rent_per_period){
        if(operation<0||operation>int(PrivateHousingOperation::EndLease))
            return py::make_tuple(bool(false),std::string("Unknown private housing operation"),uint64_t(0));
        const auto result=w.execute(PrivateHousingCommand{actor,sequence,residence_id,tenant_household,
            static_cast<PrivateHousingOperation>(operation),rent_per_period});
        return py::make_tuple(bool(result.success),std::string(result.reason),uint64_t(result.fact));
    },py::arg("actor"),py::arg("sequence"),py::arg("residence_id"),py::arg("operation"),
      py::arg("tenant_household")=0,py::arg("rent_per_period")=0)
    .def("transport_shipment_command",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t shipment_id,int operation,int destination,int good,int64_t quantity,int goal,bool military_ration_depot)->py::tuple{
        if(operation<0||operation>int(TransportShipmentOperation::Cancel))
            return py::make_tuple(false,std::string("Unknown shipment operation"),uint64_t(0),uint64_t(0));
        if(goal<0||goal>int(TransportServiceGoal::LowestFee))
            return py::make_tuple(false,std::string("Invalid transport service goal"),uint64_t(0),uint64_t(0));
        const auto result=w.execute(TransportShipmentCommand{actor,sequence,shipment_id,
            static_cast<TransportShipmentOperation>(operation),destination,good,quantity,
            static_cast<TransportServiceGoal>(goal),military_ration_depot});
        return py::make_tuple(bool(result.success),std::string(result.reason),uint64_t(result.fact),uint64_t(result.session));
    },py::arg("actor"),py::arg("sequence"),py::arg("shipment_id")=0,py::arg("operation")=0,
      py::arg("destination")=-1,py::arg("good")=-1,py::arg("quantity")=0,py::arg("goal")=0,py::arg("military_ration_depot")=false)
    .def("transport_route_command",[](Realm&w,uint64_t actor,uint64_t sequence,size_t route_index,bool open)->py::tuple{
        if(route_index>=w.national_transport_routes().size())
            return py::make_tuple(false,std::string("National route is unavailable"),uint64_t(0),uint64_t(0));
        const auto result=w.execute(TransportRouteCommand{actor,sequence,uint8_t(route_index),open});
        return py::make_tuple(bool(result.success),std::string(result.reason),uint64_t(result.fact),uint64_t(result.session));
    },py::arg("actor"),py::arg("sequence"),py::arg("route_index"),py::arg("open"))
    .def_property_readonly("transport_routes",[](const Realm&w){py::list result;for(const auto&route:w.national_transport_routes()){
        py::dict row;row["atlas_link"]=route.atlas_link;row["from"]=route.from;row["to"]=route.to;
        row["mode"]=int(route.mode);row["open"]=route.open;row["travel_days"]=route.travel_days;
        row["capacity_per_day"]=route.capacity_per_day;row["fee_per_unit"]=route.fee_per_unit;
        row["fee_recipient_region"]=route.fee_recipient_region;result.append(row);}return result;})
    .def_property_readonly("transport_shipments",[](const Realm&w){py::list result;for(const auto&shipment:w.transport_shipments()){
        py::dict row;row["id"]=shipment.id;row["owner"]=shipment.owner;row["owner_kind"]=int(shipment.owner_kind);row["source_region"]=shipment.source_region;row["created_day"]=shipment.created_day;
        row["destination"]=shipment.destination;row["good"]=shipment.good;row["service_goal"]=int(shipment.service_goal);
        row["quantity"]=shipment.quantity;row["delivered"]=shipment.delivered;row["canceled"]=shipment.canceled;
        row["fee_escrow"]=shipment.fee_escrow;row["route_indices"]=shipment.route_indices;result.append(row);}return result;})
    .def_property_readonly("military_unit_rosters",[](const Realm&w){py::list result;for(const auto&unit:w.military_unit_rosters()){
        py::dict row;row["id"]=unit.id;row["source_region"]=unit.source_region;row["commander"]=unit.commander;
        row["members"]=unit.members;row["mobilized_members"]=unit.mobilized_members;
        row["combat_consented_members"]=unit.combat_consented_members;row["battle_ready_members"]=unit.battle_ready_members;
        row["mobilized_until_day"]=unit.mobilized_until_day;
        row["payroll_escrow"]=unit.payroll_escrow;row["paid_total"]=unit.paid_total;result.append(row);}return result;})
    .def("military_battle_candidates",&Realm::military_battle_candidates,py::arg("region"))
    .def_property_readonly("military_raids",[](const Realm&w){py::list result;for(const auto&raid:w.military_raids()){
        py::dict row;row["id"]=raid.id;row["partner_id"]=raid.partner_id;row["target_region"]=raid.target_region;
        row["created_day"]=raid.created_day;row["resolved_day"]=raid.resolved_day;row["force_units"]=raid.force_units;
        row["defense_score"]=raid.defense_score;row["attacker_losses"]=raid.attacker_losses;
        row["defender_casualties"]=raid.defender_casualties;row["salt_looted"]=raid.salt_looted;
        row["status"]=int(raid.status);row["defenders"]=raid.defenders;
        row["supplied_defenders"]=raid.supplied_defenders;result.append(row);}return result;})
    .def_property_readonly("military_supply_obligations",[](const Realm&w){py::list result;for(const auto&item:w.military_supply_obligations()){
        py::dict row;row["id"]=item.id;row["personnel_obligation_id"]=item.personnel_obligation_id;
        row["obligor"]=item.obligor;row["recipient"]=item.recipient;row["source_region"]=item.source_region;
        row["destination_region"]=item.destination_region;row["created_day"]=item.created_day;
        row["first_due_day"]=item.first_due_day;row["due_day"]=item.due_day;row["completed_day"]=item.completed_day;
        row["extensions"]=item.extensions;row["required"]=item.required;row["delivered"]=item.delivered;
        row["delivered_at_first_deadline"]=item.delivered_at_first_deadline;
        row["shortage_at_first_deadline"]=item.shortage_at_first_deadline;
        row["delivered_at_last_deadline"]=item.delivered_at_last_deadline;
        row["shortage_at_last_deadline"]=item.shortage_at_last_deadline;
        row["first_deadline_checked"]=item.first_deadline_checked;row["due_checked"]=item.due_checked;
        result.append(row);}return result;})
    .def_property_readonly("transport_cargo_lots",[](const Realm&w){py::list result;for(const auto&lot:w.transport_cargo_lots()){
        py::dict row;row["id"]=lot.id;row["shipment_id"]=lot.shipment_id;row["route_position"]=lot.route_position;
        row["current_region"]=lot.current_region;row["arrival_day"]=lot.arrival_day;row["quantity"]=lot.quantity;
        result.append(row);}return result;})
    .def_property_readonly("transport_escrow",&Realm::transport_escrow)
    .def("person",[](Realm&w,uint64_t id){auto&p=w.person(id);py::dict d;d["id"]=id;d["region"]=p.region;d["age_days"]=p.age_days;d["cash"]=p.cash;d["inventory"]=p.inventory;d["health"]=p.health;d["nutrition"]=p.nutrition;d["sleep_debt"]=p.body_state.sleep_debt;d["exertion"]=p.body_state.exertion;d["rest_until_tick"]=p.body_state.rest_until_tick;d["job_kind"]=p.work_assignment.job_kind;d["taxing_authority"]=p.taxing_authority;d["skill"]=p.skill;d["goal"]=p.goal;d["stats"]=p.stats.values;py::dict legacyCapabilities;py::list activeCapabilities;for(size_t i=0;i<kLegacyCapabilities.size();++i){legacyCapabilities[kLegacyCapabilityKeys[i]]=p.stats.get(kLegacyCapabilities[i]);if(w.capability_active(kLegacyCapabilities[i]))activeCapabilities.append(kLegacyCapabilityKeys[i]);}d["legacy_capabilities"]=legacyCapabilities;d["active_capabilities"]=activeCapabilities;py::dict capabilities;for(size_t i=0;i<kCapabilityCount;++i)capabilities[kCapabilityKeys[i]]=w.capability_value(p.id,static_cast<CapabilityId>(i));py::list capabilitySchema;for(auto capabilityId:w.capability_schema())capabilitySchema.append(kCapabilityKeys[static_cast<size_t>(capabilityId)]);d["capabilities"]=capabilities;d["capability_schema"]=capabilitySchema;d["family"]=int(p.family);d["alive"]=p.alive;d["decisions"]=p.decisions;d["sampling"]=p.sampling;auto est=w.employment_state(id);d["employment_state"]=est==EmploymentState::Employed?"Employed":"Idle";auto jt=w.job_type(id);switch(jt){case JobType::Worker:d["job_type"]="Worker";break;case JobType::Teacher:d["job_type"]="Teacher";break;case JobType::AppointedOffice:d["job_type"]="AppointedOffice";break;default:d["job_type"]="None";break;}return d;})
    .def("utility",&Realm::utility).def("choose",&Realm::choose).def("reroll",&Realm::reroll).def("serialize",&Realm::serialize).def("deserialize",[](Realm&w,const std::string&s,const std::string&b){std::string error;bool ok=w.deserialize(s,b,error);return py::make_tuple(ok,error);})
    .def_static("scenario",&Realm::scenario,py::arg("seed"),py::arg("actor"),py::arg("age")=-1,py::arg("stat")=-1,py::arg("goal")=-1,py::arg("budget")=-1,py::arg("schools")=-1,py::arg("jobs")=-1,py::arg("nutrition")=-1,py::arg("health")=-1,py::arg("capability_mask")=kDefaultCapabilityMask)
    .def("route",[](Realm&w,uint64_t actor,int from,int to,bool open){return w.route_command(actor,from,to,open).success;})
    .def_property_readonly("paused",&Realm::paused)
    .def_property_readonly("pause_revision",&Realm::pause_revision)
    .def_property_readonly("day",&Realm::day)
    .def_property_readonly("game_tick",&Realm::game_tick)
    .def_property_readonly("timeline_contract",&Realm::timeline_contract)
    .def_property_readonly("pilot_policy_compatible",&Realm::pilot_policy_compatible)
    .def_property_readonly("model_contract",&Realm::model_contract)
    .def_property_readonly("schedule_cursor",[](const Realm&w){return w.schedule_book().cursor;})
    .def_property_readonly("schedule_next_session_id",[](const Realm&w){return w.schedule_book().next_session_id;})
    .def_property_readonly("settled_day",[](const Realm&w){return w.schedule_book().settled_day;})
    .def("advance_ticks",&Realm::advance_ticks,py::arg("ticks"))
    .def("pause",[](Realm&w,std::optional<uint64_t>expected=std::nullopt){auto r=w.execute(RealmClockCommand{RealmClockCommand::Operation::Pause,expected});return py::make_tuple(r.success,r.reason);},py::arg("expected")=py::none())
    .def("resume",[](Realm&w,std::optional<uint64_t>expected=std::nullopt){auto r=w.execute(RealmClockCommand{RealmClockCommand::Operation::Resume,expected});return py::make_tuple(r.success,r.reason);},py::arg("expected")=py::none())
    .def("establishment_command",[](Realm&w,uint64_t actor,uint64_t sequence,int owner_kind,uint64_t owner_id,int region,int industry,int capacity,int64_t funding){
        EstablishmentCommand cmd;cmd.actor=actor;cmd.sequence=sequence;cmd.owner_kind=static_cast<EstablishmentOwnerKind>(owner_kind);cmd.owner_id=owner_id;cmd.region=region;cmd.industry=static_cast<EstablishmentIndustry>(industry);cmd.capacity=capacity;cmd.funding=funding;
        auto r=w.execute(cmd);return py::make_tuple(r.success,r.reason,r.session);
    },py::arg("actor"),py::arg("sequence"),py::arg("owner_kind"),py::arg("owner_id"),py::arg("region"),py::arg("industry"),py::arg("capacity"),py::arg("funding"))
    .def("municipal_budget_command",[](Realm&w,uint64_t actor,uint64_t sequence,int region,int64_t amount){
        const auto result=w.execute(MunicipalBudgetCommand{actor,sequence,region,amount});
        return py::make_tuple(result.success,result.reason,result.fact);
    },py::arg("actor"),py::arg("sequence"),py::arg("region"),py::arg("amount"))
    .def("foreign_trade_command",[](Realm&w,uint64_t actor,uint64_t sequence,int operation,int good,int64_t quantity){
        const auto result=w.execute(ForeignTradeCommand{actor,sequence,static_cast<ForeignTradeOperation>(operation),good,quantity});
        return py::make_tuple(result.success,result.reason,result.session);
    },py::arg("actor"),py::arg("sequence"),py::arg("operation"),py::arg("good"),py::arg("quantity"))
    .def("military_raid_command",[](Realm&w,int partner_id,uint64_t sequence,int force_units){
        const auto result=w.execute(MilitaryRaidCommand{partner_id,sequence,force_units});
        return py::make_tuple(result.success,result.reason,result.fact,result.session);
    },py::arg("partner_id"),py::arg("sequence"),py::arg("force_units"))
    .def("military_battle_command",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t raid_id,bool retreat,
                                      const std::vector<uint64_t>&defenders){
        const auto result=w.execute(MilitaryBattleCommand{actor,sequence,raid_id,retreat,defenders});
        return py::make_tuple(result.success,result.reason,result.fact,result.session);
    },py::arg("actor"),py::arg("sequence"),py::arg("raid_id"),py::arg("retreat"),py::arg("defenders"))
    .def_property_readonly("foreign_trade_cash",&Realm::foreign_trade_cash)
    .def_property_readonly("foreign_trade_escrow",&Realm::foreign_trade_escrow)
    .def_property_readonly("foreign_trade_salt_capacity",&Realm::foreign_trade_salt_capacity)
    .def_property_readonly("foreign_trade_goods",&Realm::foreign_trade_goods)
    .def_property_readonly("foreign_trade_partners",[](const Realm&w){py::list rows;int partner_id=0;for(const auto&partner:w.foreign_trade_partners()){
        py::dict row;row["partner_id"]=partner_id++;row["money"]=partner.money;row["escrow"]=partner.escrow;
        row["decision_sequence"]=partner.decision_sequence;
        row["salt_capacity"]=partner.salt_capacity;row["goods"]=partner.goods;rows.append(row);}return rows;})
    .def("foreign_trade_orders",[](const Realm&w){py::list rows;for(const auto&order:w.foreign_trade_orders()){
        py::dict row;row["id"]=order.id;row["owner"]=order.owner;row["partner_id"]=order.partner_id;row["operation"]=int(order.operation);row["good"]=order.good;
        row["created_day"]=order.created_day;row["arrival_day"]=order.arrival_day;row["quantity"]=order.quantity;
        row["gross"]=order.gross;row["duty"]=order.duty;row["escrow"]=order.escrow;rows.append(row);}return rows;})
    .def("establishments",[](const Realm&w){py::list result;for(const auto&e:w.establishments()){py::dict d;d["id"]=e.id;d["region"]=e.region;d["owner_kind"]=int(e.owner_kind);d["owner_id"]=e.owner_id;d["industry"]=int(e.industry);d["capacity"]=e.capacity;d["cash"]=e.cash;d["source_good"]=e.source_good;d["source_units"]=e.source_units;d["forest_units"]=e.source_good==int(GoodId::Timber)?e.source_units:0;d["stock"]=e.stock;d["legacy_credit"]=e.work_pay.legacy_credit;d["paid_work_units"]=e.work_pay.paid_units;d["wage_remainders"]=e.work_pay.remainders;result.append(d);}return result;})
    .def_property_readonly("municipal_funds",&Realm::municipal_funds)
    .def("land_holdings",[](const Realm&w){py::list result;for(const auto&holding:w.land_holdings()){py::dict d;d["id"]=holding.id;d["region"]=holding.region;d["area_units"]=holding.area_units;d["title_holder"]=holding.title_holder;result.append(d);}return result;})
    .def("land_title_transfer",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t holding,uint64_t recipient){
        const auto result=w.execute(LandTitleTransferCommand{actor,sequence,holding,recipient});
        return py::make_tuple(result.success,result.reason,result.fact);
    })
    .def("land_use_rights",[](const Realm&w){py::list result;for(const auto&right:w.land_use_rights()){py::dict d;d["id"]=right.id;d["holding_id"]=right.holding_id;d["grantee"]=right.grantee;d["grantor"]=right.grantor;result.append(d);}return result;})
    .def("land_use_grant",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t holding,uint64_t grantee){
        const auto result=w.execute(LandUseGrantCommand{actor,sequence,holding,grantee});
        return py::make_tuple(result.success,result.reason,result.fact);
    })
    .def("land_use_revoke",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t holding){
        const auto result=w.execute(LandUseRevokeCommand{actor,sequence,holding});
        return py::make_tuple(result.success,result.reason,result.fact);
    })
    .def("establishment_transfer",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t site,int operation,int good,int64_t amount){
        EstablishmentTransferCommand command{actor,sequence,site,static_cast<EstablishmentTransferOperation>(operation),good,amount};
        const auto result=w.execute(command);return py::make_tuple(result.success,result.reason,result.fact);
    },py::arg("actor"),py::arg("sequence"),py::arg("establishment_id"),py::arg("operation"),py::arg("good"),py::arg("amount"))
    .def_property_readonly("work_credits",[](const Realm&w){py::dict result;for(const auto& [key,value]:w.work_credits())result[py::make_tuple(key.first,key.second)]=py::make_tuple(value.legacy_units,value.unassigned_units,value.bridge_units);return result;})
    .def("work_authorization_command",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t site,uint64_t worker,int operation,int64_t begin,int64_t end){
        const auto result=w.execute(WorkAuthorizationCommand{actor,sequence,site,worker,static_cast<WorkAuthorizationOperation>(operation),begin,end});
        return py::make_tuple(result.success,result.reason,result.fact);
    },py::arg("actor"),py::arg("sequence"),py::arg("establishment_id"),py::arg("worker"),py::arg("operation"),py::arg("begin")=0,py::arg("end")=0)
    .def("household_fund_command",[](Realm&w,uint64_t actor,uint64_t sequence,uint64_t household,int operation,int64_t amount){
        const auto result=w.execute(HouseholdFundCommand{actor,sequence,household,static_cast<HouseholdFundOperation>(operation),amount});
        return py::make_tuple(result.success,result.reason,result.fact);
    },py::arg("actor"),py::arg("sequence"),py::arg("household_id"),py::arg("operation"),py::arg("amount"))
    .def("market_order_command",[](Realm&w,uint64_t actor,uint64_t sequence,int operation,uint64_t order_id,int region,int good,int64_t quantity,int64_t maximum_unit_price){
        const auto result=w.execute(MarketOrderCommand{actor,sequence,order_id,static_cast<MarketOrderOperation>(operation),region,good,quantity,maximum_unit_price});
        return py::make_tuple(result.success,result.reason,result.session);
    },py::arg("actor"),py::arg("sequence"),py::arg("operation"),py::arg("order_id")=0,py::arg("region")=0,py::arg("good")=0,py::arg("quantity")=0,py::arg("maximum_unit_price")=0)
    .def("market_orders",[](const Realm&w){py::list rows;for(const auto&order:w.market_orders()){py::dict row;row["id"]=order.id;row["owner"]=order.owner;row["region"]=order.region;row["good"]=order.good;row["placed_day"]=order.placed_day;row["settle_day"]=order.settle_day;row["funding_household"]=order.funding_household;row["priority"]=int(order.priority);row["quantity"]=order.quantity;row["maximum_unit_price"]=order.maximum_unit_price;row["personal_escrow"]=order.personal_escrow;row["household_escrow"]=order.household_escrow;row["organization_escrow"]=order.organization_escrow;rows.append(row);}return rows;})
    .def_property_readonly("market_escrow",&Realm::market_escrow)
    .def("household_of",&Realm::household_of)
    .def("household_authority",&Realm::household_authority)
    .def("household_fund",&Realm::household_fund)
    .def_property_readonly("household_fund_records",&Realm::household_fund_records)
    .def("schedule_command",[](Realm&w,int operation,uint64_t actor,uint64_t sequence,uint64_t session,int64_t begin,int64_t end,int32_t priority,int32_t demand_rate,int kind,int region,uint64_t establishment_id){
        ScheduleCommand cmd;cmd.operation=static_cast<ScheduleCommand::Operation>(operation);cmd.actor=actor;cmd.sequence=sequence;cmd.session=session;cmd.begin=begin;cmd.end=end;cmd.priority=priority;cmd.demand_rate=demand_rate;cmd.kind=static_cast<ActivityKind>(kind);cmd.region=region;cmd.establishment_id=establishment_id;
        auto r=w.execute(cmd);return py::make_tuple(r.success,r.reason,r.session);
    },py::arg("operation"),py::arg("actor"),py::arg("sequence"),py::arg("session")=0,py::arg("begin")=0,py::arg("end")=0,py::arg("priority")=0,py::arg("demand_rate")=0,py::arg("kind")=0,py::arg("region")=-1,py::arg("establishment_id")=0)
    .def("schedule_sessions",[](const Realm&w,std::optional<uint64_t>actor=std::nullopt){
        struct Record{uint64_t actor;ActivitySession session;};
        std::vector<Record> records;
        const auto&book=w.schedule_book();
        for(auto const&[a,sessions]:book.sessions){
            if(actor&&a!=*actor)continue;
            for(auto const&s:sessions)records.push_back({a,s});
        }
        std::sort(records.begin(),records.end(),[](const Record&a,const Record&b){if(a.actor!=b.actor)return a.actor<b.actor;return a.session.id<b.session.id;});
        py::list result;
        for(auto const&record:records){
            auto const&s=record.session;
            py::dict d;
            d["actor"]=record.actor;d["id"]=s.id;d["begin"]=s.begin;d["end"]=s.end;d["priority"]=s.priority;d["demand_rate"]=s.demand_rate;d["requested"]=s.requested;d["delivered"]=s.delivered;d["unmet"]=s.unmet;d["capacity_unmet"]=s.capacity_unmet;d["unresolved_unmet"]=s.unresolved_unmet;d["retired"]=s.retired;d["retired_tick"]=s.retired_tick;d["retired_cause"]=s.retired_cause;d["kind"]=int(s.kind);d["region"]=s.region;d["establishment_id"]=s.establishment_id;
            result.append(d);
        }
        return result;
    },py::arg("actor")=py::none())
    .def("work_present_ticks",[](const Realm&w,uint64_t actor){
        const auto&ticks=w.schedule_book().work_present_ticks;
        auto found=ticks.find(actor);
        return found==ticks.end()?int64_t(0):found->second;
    },py::arg("actor"))
    .def("settled_work_units",[](const Realm&w,uint64_t actor){
        const auto&units=w.schedule_book().settled_work_units;
        auto found=units.find(actor);
        return found==units.end()?int64_t(0):found->second;
    },py::arg("actor"))
    .def_property_readonly("work_wage_per_unit",[](const Realm&w){return w.gameplay_config().wage_per_unit();})
    .def("work_present_ticks_by_region",[](const Realm&w,uint64_t actor,int region){
        const auto&ticks=w.schedule_book().work_present_ticks_by_region;
        auto found=ticks.find(WorkRegionKey{actor,region});
        return found==ticks.end()?int64_t(0):found->second;
    },py::arg("actor"),py::arg("region"))
    .def("work_present_ticks_by_establishment",[](const Realm&w,uint64_t actor,uint64_t establishment){
        const auto&ticks=w.schedule_book().work_present_ticks_by_establishment;
        auto found=ticks.find(WorkEstablishmentKey{actor,establishment});
        return found==ticks.end()?int64_t(0):found->second;
    },py::arg("actor"),py::arg("establishment"))
    .def("paid_work_units_by_region",[](const Realm&w,uint64_t actor,int region){
        const auto&units=w.schedule_book().paid_work_units_by_region;
        auto found=units.find(WorkRegionKey{actor,region});
        return found==units.end()?int64_t(0):found->second;
    },py::arg("actor"),py::arg("region"));
    py::class_<RealmBatch>(m,"RealmBatch").def(py::init<int,uint64_t,int,int,int,int>(),py::arg("count"),py::arg("seed"),py::arg("role"),py::arg("regions"),py::arg("horizon"),py::arg("workers")=4).def("observe",&RealmBatch::observe).def("step",&RealmBatch::step,py::arg("actions"),py::arg("auto_reset")=true).def("outcomes",&RealmBatch::outcomes);
#ifdef POLYEC_AI_ENABLED
    py::class_<RealmNativePolicy>(m,"RealmNativePolicy").def(py::init<int,int>(),py::arg("observations")=realm_obs,py::arg("actions")=realm_actions).def("load",&NativePolicy::load).def("error",&NativePolicy::error).def("predict",[](RealmNativePolicy&p,py::array_t<float,py::array::c_style|py::array::forcecast>o,py::array_t<float,py::array::c_style|py::array::forcecast>h){if(o.ndim()!=2||o.shape(1)!=realm_obs||h.ndim()!=2||h.shape(0)!=o.shape(0)||h.shape(1)!=64)throw std::invalid_argument("Realm ONNX input shape");auto r=p.predict({o.data(),o.data()+o.size()},{h.data(),h.data()+h.size()},int(o.shape(0)));if(!r.success)throw std::runtime_error(r.error);py::array_t<float>logits({int(o.shape(0)),realm_actions}),memory({int(o.shape(0)),64});std::copy(r.logits.begin(),r.logits.end(),logits.mutable_data());std::copy(r.hidden.begin(),r.hidden.end(),memory.mutable_data());return py::make_tuple(logits,memory);});
#endif
}
