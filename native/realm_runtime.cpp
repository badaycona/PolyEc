#include "polyec/realm_runtime.hpp"
#include "polyec/model_manifest.hpp"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace polyec {
namespace {uint64_t mix(uint64_t x){x+=0x9e3779b97f4a7c15ull;x=(x^(x>>30))*0xbf58476d1ce4e5b9ull;x=(x^(x>>27))*0x94d049bb133111ebull;return x^(x>>31);}uint64_t checksum(const std::string&s){uint64_t h=1469598103934665603ull;for(unsigned char c:s){h^=c;h*=1099511628211ull;}return h;}}
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
namespace {
void add_runtime_profile(std::array<uint64_t, RealmRuntime::runtime_phase_count>& profile,
                         size_t phase, std::chrono::steady_clock::time_point started) {
    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - started).count();
    if (elapsed > 0) profile[phase] += static_cast<uint64_t>(elapsed);
}
}
#endif
std::array<float,realm_actions> revalidate_snapshot(const Realm&w,uint64_t id,const std::array<Candidate,realm_actions>&seen,const std::array<float,realm_actions>&seen_mask){
    auto current=w.candidates(id);auto valid=w.mask(id,current);
    for(int a=0;a<realm_actions;++a){auto&x=seen[a];auto&y=current[a];valid[a]=valid[a]&&seen_mask[a]&&x.kind==y.kind&&x.target==y.target&&x.good==y.good&&x.amount==y.amount&&x.term==y.term?1.f:0.f;}return valid;
}
bool RealmRuntime::load_models(const std::array<std::string,3>&paths,const std::string&bundle,uint64_t seed,int population,bool allowIncompatiblePilot,int complexActors){
    loaded_=false;atlas_manual_test_=false;allow_incompatible_pilot_for_development_=false;loaded_model_contract_.clear();try{
    world_=std::make_unique<Realm>(seed,6,population,false,GameplayConfig{},complexActors);if(!world_->gameplay_config().historical_defaults()){error_="Gameplay configuration not supported by this model bundle";world_.reset();return false;}
    constexpr std::array<const char*,3>families{"governance","economic","civilian"};bool anyLegacy=false,anyCurrent=false;
    for(int i=0;i<3;++i){ModelManifest manifest;std::string manifestError;if(!read_model_manifest(std::filesystem::u8path(paths[i]),manifest,manifestError)){error_=manifestError;world_.reset();return false;}
        bool legacy=false;if(!compatible_model_manifest(manifest,families[i],world_->model_contract(),allowIncompatiblePilot,legacy,manifestError)){error_=manifestError;world_.reset();return false;}anyLegacy|=legacy;anyCurrent|=!legacy;}
    if(anyLegacy&&anyCurrent){error_="Model bundle mixes legacy and versioned contracts";world_.reset();return false;}
    for(int i=0;i<3;++i){policies_[i]=std::make_unique<NativePolicy>(realm_obs,realm_actions);if(!policies_[i]->load(paths[i])){error_=policies_[i]->error();world_.reset();return false;}}
    loaded_model_contract_=world_->model_contract();allow_incompatible_pilot_for_development_=anyLegacy;memories_.assign(population,{});memory_roles_.clear();sync_roles();seed_=seed;bundle_="realm8|"+bundle;inference_count_=0;inference_ms_=0;
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    runtime_phase_profile_ns_.fill(0);
#endif
    loaded_=true;error_.clear();return true;}catch(const std::exception&e){error_=e.what();world_.reset();return false;}
}
bool RealmRuntime::load_atlas_manual_test(uint64_t seed,int population){
    loaded_=false;atlas_manual_test_=false;allow_incompatible_pilot_for_development_=false;
    loaded_model_contract_.clear();bundle_.clear();memories_.clear();memory_roles_.clear();
    for(auto&policy:policies_)policy.reset();
    if(population<128||population>100000){error_="Atlas manual-test population must be in 128..100000";world_.reset();return false;}
    try{
        world_=std::make_unique<Realm>(seed,int(kAtlasTerritoryCount)-1,population);
        memories_.assign(size_t(population),{});sync_roles();seed_=seed;
        bundle_="atlas-manual-test-v1";loaded_model_contract_=world_->model_contract();
        inference_count_=0;inference_ms_=0;atlas_manual_test_=true;loaded_=true;
        auto paused=world_->execute(RealmClockCommand{RealmClockCommand::Operation::Pause,world_->pause_revision()});
        if(!paused.success){loaded_=false;atlas_manual_test_=false;error_="Atlas manual-test pause failed: "+paused.reason;world_.reset();return false;}
        error_.clear();return true;
    }catch(const std::exception&error){loaded_=false;atlas_manual_test_=false;world_.reset();error_=error.what();return false;}
}
void RealmRuntime::sync_roles(){for(size_t i=0;i<memories_.size();++i){auto family=world_->person(i+1).family;if(i>=memory_roles_.size())memory_roles_.push_back(family);else if(memory_roles_[i]!=family){memories_[i].fill(0);memory_roles_[i]=family;}}}
bool RealmRuntime::decisions(Family family){
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    auto phaseStarted=std::chrono::steady_clock::now();
#endif
    sync_roles();
    auto ids=world_->due(family);
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    add_runtime_profile(runtime_phase_profile_ns_,0,phaseStarted);
#endif
    for(size_t start=0;start<ids.size();start+=128){
        int n=int(std::min<size_t>(128,ids.size()-start));
        std::vector<float> obs,hidden;
        std::vector<std::array<Candidate,realm_actions>> seen;
        std::vector<std::array<float,realm_actions>> seen_masks;
        obs.reserve(n*realm_obs);hidden.reserve(n*64);seen.reserve(n);seen_masks.reserve(n);
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
        phaseStarted=std::chrono::steady_clock::now();
#endif
        for(int j=0;j<n;++j){auto id=ids[start+j];seen.emplace_back();auto o=world_->observation(id,seen.back());std::array<float,realm_actions> m{};for(int a=0;a<realm_actions;++a)m[a]=o[realm_state+a*realm_features+5];seen_masks.push_back(m);obs.insert(obs.end(),o.begin(),o.end());hidden.insert(hidden.end(),memories_[id-1].begin(),memories_[id-1].end());}
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
        add_runtime_profile(runtime_phase_profile_ns_,1,phaseStarted);
#endif
        auto t=std::chrono::steady_clock::now();
        auto out=policies_[int(family)]->predict(obs,hidden,n);
        const double inferenceDuration=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();
        inference_ms_+=inferenceDuration;
        if(!out.success){stop(out.error);return false;}
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
        add_runtime_profile(runtime_phase_profile_ns_,2,t);
        phaseStarted=std::chrono::steady_clock::now();
#endif
        for(int j=0;j<n;++j){auto id=ids[start+j];auto m=revalidate_snapshot(*world_,id,seen[j],seen_masks[j]);auto&person=world_->person(id);float max=-1e30f;for(int a=0;a<realm_actions;++a)if(m[a])max=std::max(max,out.logits[j*realm_actions+a]);double sum=0;std::array<double,realm_actions> weights{};for(int a=0;a<realm_actions;++a)if(m[a]){weights[a]=std::exp(std::clamp(double(out.logits[j*realm_actions+a]-max),-80.,0.));sum+=weights[a];}double u=double(mix(seed_^mix(id)^mix(person.sampling))>>11)*0x1.0p-53*sum;int chosen=0;for(int a=0;a<realm_actions;++a){u-=weights[a];if(u<0){chosen=a;break;}}
            auto result=world_->execute({id,person.decisions+1,seen[j][chosen]});if(!result.success){stop("Decision commit failed: "+result.reason);return false;}std::copy_n(out.hidden.data()+j*64,64,memories_[id-1].begin());++inference_count_;
        }
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
        add_runtime_profile(runtime_phase_profile_ns_,3,phaseStarted);
#endif
    }
    return true;
}
bool RealmRuntime::paused() const { return loaded_ && world_ && world_->paused(); }
uint64_t RealmRuntime::pause_revision() const { return loaded_ && world_ ? world_->pause_revision() : 0; }
bool RealmRuntime::pause(std::optional<uint64_t> expected_pause_revision) {
    if (!loaded_ || !world_) { error_ = "Realm not loaded"; return false; }
    auto result = world_->execute(RealmClockCommand{RealmClockCommand::Operation::Pause, expected_pause_revision});
    error_ = result.success ? "" : result.reason;
    return result.success;
}
bool RealmRuntime::resume(std::optional<uint64_t> expected_pause_revision) {
    if (!loaded_ || !world_) { error_ = "Realm not loaded"; return false; }
    if (world_->timeline_contract() && !atlas_manual_test_) { error_ = "Learned pilot policies are unavailable for timeline worlds"; return false; }
    auto result = world_->execute(RealmClockCommand{RealmClockCommand::Operation::Resume, expected_pause_revision});
    error_ = result.success ? "" : result.reason;
    return result.success;
}
bool RealmRuntime::advance(int days){if(!loaded_||!world_||days<1||days>30){error_="Realm not loaded or day chunk outside1..30";return false;}if(world_->timeline_contract()&&!atlas_manual_test_){error_="Learned pilot policies are unavailable for timeline worlds";return false;}const int64_t maxTick=365000LL*kSubdayUnitsPerDay;const int64_t requestedTicks=static_cast<int64_t>(days)*kSubdayUnitsPerDay;if(world_->game_tick()>maxTick||requestedTicks>maxTick-world_->game_tick()){error_="Realm advance exceeds maximum horizon";return false;}if(!atlas_manual_test_&&world_->draft().chosen<0){error_="Choose a draft card first";return false;}if(world_->paused()){error_="Realm is paused";return false;}
    try {
        for(int day=0;day<days;++day){
            if(!atlas_manual_test_)for(auto f:{Family::Governance,Family::Economic,Family::Civilian})if(!decisions(f))return false;
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
            auto phaseStarted=std::chrono::steady_clock::now();
#endif
            world_->advance();
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
            add_runtime_profile(runtime_phase_profile_ns_,4,phaseStarted);
            phaseStarted=std::chrono::steady_clock::now();
#endif
            memories_.resize(world_->count());sync_roles();
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
            add_runtime_profile(runtime_phase_profile_ns_,5,phaseStarted);
            phaseStarted=std::chrono::steady_clock::now();
#endif
            if(!world_->invariant()){stop("Realm accounting invariant failed");return false;}
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
            add_runtime_profile(runtime_phase_profile_ns_,6,phaseStarted);
#endif
        }error_.clear();return true;
    } catch(const std::exception&e){error_=e.what();return false;}
}
bool RealmRuntime::step_one_day(){if(!loaded_||!world_){error_="Realm not loaded";return false;}if(world_->timeline_contract()&&!atlas_manual_test_){error_="Learned pilot policies are unavailable for timeline worlds";return false;}const int64_t maxTick=365000LL*kSubdayUnitsPerDay;if(world_->game_tick()>maxTick||64>maxTick-world_->game_tick()){error_="Realm advance exceeds maximum horizon";return false;}if(!atlas_manual_test_&&world_->draft().chosen<0){error_="Choose a draft card first";return false;}
    try {
        if(!atlas_manual_test_)for(auto f:{Family::Governance,Family::Economic,Family::Civilian})if(!decisions(f))return false;
        auto result=world_->execute(RealmClockCommand{RealmClockCommand::Operation::StepOneDay,world_->pause_revision()});
        if(!result.success){error_=result.reason;return false;}
        memories_.resize(world_->count());sync_roles();
        if(!world_->invariant()){stop("Realm accounting invariant failed");return false;}
        error_.clear();return true;
    } catch(const std::exception&e){error_=e.what();return false;}
}
bool RealmRuntime::command(uint64_t id,int action,const std::string&expected){if(!loaded_||action<0||action>=realm_actions||!id||id>uint64_t(world_->count())){error_="Invalid player command";return false;}if(world_->timeline_contract()&&!atlas_manual_test_){error_="Legacy policy actions are unavailable for timeline worlds";return false;}if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before committing this command";return false;}auto r=world_->execute({id,world_->person(id).decisions+1,world_->candidates(id)[action]});if(r.success)sync_roles();error_=r.success?"":r.reason;return r.success;}
bool RealmRuntime::military_combat_consent(uint64_t actor,bool consent,const std::string&expected){
    if(!loaded_||!world_||!actor||actor>uint64_t(world_->count())){error_="Invalid military-consent actor";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before changing combat consent";return false;}
    const auto decisions=world_->person(actor).decisions;
    if(decisions==UINT64_MAX){error_="Military-consent decision sequence overflow";return false;}
    const auto result=world_->execute(MilitaryCombatConsentCommand{actor,decisions+1,consent});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::military_raid(int partner_id,int force_units,const std::string&expected){
    if(!loaded_||!world_){error_="Realm not loaded";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before starting a coastal raid";return false;}
    if(partner_id<0||partner_id>=kForeignTradePartnerCount){error_="Unknown external power";return false;}
    const auto partners=world_->foreign_trade_partners();
    const auto sequence=partners[size_t(partner_id)].decision_sequence==UINT64_MAX?
        UINT64_MAX:partners[size_t(partner_id)].decision_sequence+1;
    const auto result=world_->execute(MilitaryRaidCommand{partner_id,sequence,force_units});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::military_battle(uint64_t actor,uint64_t raid_id,bool retreat,
                                   const std::vector<uint64_t>&defenders,const std::string&expected){
    if(!loaded_||!world_||!actor||actor>uint64_t(world_->count())){error_="Invalid battle-response actor";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before resolving the battle";return false;}
    const auto decisions=world_->person(actor).decisions;
    const auto sequence=decisions==UINT64_MAX?UINT64_MAX:decisions+1;
    const auto result=world_->execute(MilitaryBattleCommand{actor,sequence,raid_id,retreat,defenders});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::military_counterattack(uint64_t actor,int region,bool retreat,
                                          const std::vector<uint64_t>&defenders,const std::string&expected){
    if(!loaded_||!world_||!actor||actor>uint64_t(world_->count())){error_="Invalid counterattack actor";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before ordering a counterattack";return false;}
    const auto decisions=world_->person(actor).decisions;
    const uint64_t sequence=decisions==UINT64_MAX?UINT64_MAX:decisions+1;
    const auto result=world_->execute(MilitaryCounterattackCommand{actor,sequence,region,retreat,defenders});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::foreign_trade_suspension(uint64_t actor,int partner_id,bool suspended,const std::string&expected){
    if(!loaded_||!world_||!actor||actor>uint64_t(world_->count())){error_="Invalid foreign-trade policy actor";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before changing foreign trade policy";return false;}
    const auto decisions=world_->person(actor).decisions;
    const auto sequence=decisions==UINT64_MAX?UINT64_MAX:decisions+1;
    const auto result=world_->execute(ForeignTradeSuspensionCommand{actor,sequence,partner_id,suspended});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::foreign_trade_offer(uint64_t actor,int partner_id,ForeignTradeOperation operation,int good,
                                      int64_t quantity,const std::string&expected){
    if(!loaded_||!world_||!actor||actor>uint64_t(world_->count())){error_="Invalid foreign-trade offer actor";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before reserving trade resources";return false;}
    const auto decisions=world_->person(actor).decisions;
    const uint64_t sequence=decisions==UINT64_MAX?UINT64_MAX:decisions+1;
    const auto result=world_->execute(ForeignTradeOfferCommand{actor,sequence,partner_id,operation,good,quantity});
    error_=result.success?"":result.reason;
    return result.success;
}
RealmRoutePreview RealmRuntime::route_preview(uint64_t actor,int destination) const {
    RealmRoutePreview result;
    if (!loaded_ || !world_ || !actor || actor > uint64_t(world_->count()) ||
        world_->regions() != int(kAtlasTerritoryCount) - 1 ||
        destination < 0 || destination >= int(kAtlasTerritoryCount)) return result;
    result.supported = true;
    const auto& person = world_->person(actor);
    const int origin = person.region;
    if (origin < 0 || origin >= int(kAtlasTerritoryCount)) {
        result.supported = false;
        return result;
    }
    if (origin == destination) {
        result.reachable = true;
        result.regions.push_back(static_cast<uint8_t>(origin));
        return result;
    }
    const auto plan = NationalTransportManager::plan(
        world_->day(), static_cast<uint8_t>(origin), static_cast<uint8_t>(destination), 1,
        TransportServiceGoal::Fastest, world_->national_transport_routes());
    if (!plan.reachable) return result;
    result.reachable = true;
    result.route_days = std::max(0, plan.total_days - 1);
    result.person_days = world_->actor_travel_days(actor, destination);
    result.route_fee = plan.total_fee;
    const auto candidates = world_->candidates(actor);
    const auto legal = world_->mask(actor, candidates);
    for (int i = 0; i < realm_actions; ++i) {
        if (!legal[size_t(i)] || candidates[size_t(i)].target != destination) continue;
        if (candidates[size_t(i)].kind == Intent::Travel) result.travel_available = true;
        if (candidates[size_t(i)].kind == Intent::Migrate) result.migration_available = true;
    }
    if (result.migration_available && result.person_days > 0)
        result.migration_fee = 4 + int64_t(2) * result.person_days;
    result.regions = plan.regions;
    const auto& routes = world_->national_transport_routes();
    for (const size_t index : plan.route_indices) {
        if (index >= routes.size()) {
            result = RealmRoutePreview{};
            return result;
        }
        result.legs.push_back(routes[index]);
    }
    return result;
}
bool RealmRuntime::land_title_transfer(uint64_t actor,uint64_t holding_id,uint64_t recipient,const std::string&expected){
    if(!loaded_||!world_||!actor||actor>uint64_t(world_->count())){error_="Invalid title transfer actor";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before committing this title transfer";return false;}
    const auto decisions=world_->person(actor).decisions;
    const uint64_t sequence=decisions==UINT64_MAX?UINT64_MAX:decisions+1;
    const auto result=world_->execute(LandTitleTransferCommand{actor,sequence,holding_id,recipient});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::land_use_grant(uint64_t actor,uint64_t holding_id,uint64_t grantee,const std::string&expected){
    if(!loaded_||!world_||!actor||actor>uint64_t(world_->count())){error_="Invalid land use actor";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before granting land use";return false;}
    const auto decisions=world_->person(actor).decisions;
    const uint64_t sequence=decisions==UINT64_MAX?UINT64_MAX:decisions+1;
    const auto result=world_->execute(LandUseGrantCommand{actor,sequence,holding_id,grantee});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::land_use_revoke(uint64_t actor,uint64_t holding_id,const std::string&expected){
    if(!loaded_||!world_||!actor||actor>uint64_t(world_->count())){error_="Invalid land use actor";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before revoking land use";return false;}
    const auto decisions=world_->person(actor).decisions;
    const uint64_t sequence=decisions==UINT64_MAX?UINT64_MAX:decisions+1;
    const auto result=world_->execute(LandUseRevokeCommand{actor,sequence,holding_id});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::private_residence_build(uint64_t actor,uint64_t holding_id,const std::string&expected){
    if(!loaded_||!world_){error_="Realm not loaded";return false;}
    if(world_->regions()!=int(kAtlasTerritoryCount)-1){error_="Private homes require an Atlas27 world";return false;}
    if(!actor||actor>uint64_t(world_->count())){error_="Invalid private-home builder";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){
        error_="World changed; refresh before building a private home";return false;
    }
    const auto decisions=world_->person(actor).decisions;
    if(decisions==UINT64_MAX){error_="Private-home decision sequence overflow";return false;}
    const auto result=world_->execute(PrivateResidenceBuildCommand{actor,decisions+1,holding_id});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::private_housing_command(uint64_t actor,uint64_t residence_id,
        PrivateHousingOperation operation,uint64_t tenant_household,int64_t rent_per_period,
        const std::string&expected){
    if(!loaded_||!world_){error_="Realm not loaded";return false;}
    if(world_->regions()!=int(kAtlasTerritoryCount)-1){error_="Private leases require an Atlas27 world";return false;}
    if(!actor||actor>uint64_t(world_->count())){error_="Invalid private-housing actor";return false;}
    if(operation>PrivateHousingOperation::EndLease){error_="Unknown private-housing operation";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){
        error_="World changed; refresh before changing a private lease";return false;
    }
    const auto decisions=world_->person(actor).decisions;
    if(decisions==UINT64_MAX){error_="Private-housing decision sequence overflow";return false;}
    const auto result=world_->execute(PrivateHousingCommand{actor,decisions+1,residence_id,
        tenant_household,operation,rent_per_period});
    error_=result.success?"":result.reason;
    return result.success;
}
bool RealmRuntime::market_order(uint64_t actor,MarketOrderOperation operation,uint64_t orderId,int region,int good,
                                int64_t quantity,int64_t maximumUnitPrice,uint64_t&resultOrderId,
                                const std::string&expected){
    resultOrderId=0;
    if(!loaded_||!world_){error_="Realm not loaded";return false;}
    if(!actor||actor>uint64_t(world_->count())){error_="Invalid market-order actor";return false;}
    if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){
        error_="World changed; refresh before committing this market order";return false;
    }
    const auto decisions=world_->person(actor).decisions;
    if(decisions==UINT64_MAX){error_="Market-order decision sequence overflow";return false;}
    const auto result=world_->execute(MarketOrderCommand{actor,decisions+1,orderId,operation,region,good,
                                                         quantity,maximumUnitPrice});
    error_=result.success?"":result.reason;
    if(!result.success)return false;
    resultOrderId=result.session;
    return true;
}
bool RealmRuntime::debug_worksite_cycle(uint64_t actor,const std::string&expected,uint64_t&siteId,
                                        int64_t&actorCash,int64_t&siteCash,int64_t&fiber,
                                        int64_t&cloth,int64_t&paidUnits){
    auto reject=[&](const std::string&message){error_=message;return false;};
    if(!loaded_||!world_)return reject("Realm not loaded");
    if(!world_->paused())return reject("Pause Realm before running the development worksite cycle");
    if(expected!=std::to_string(world_->fingerprint()))return reject("World changed; refresh before running the development worksite cycle");
    if(!actor||actor>uint64_t(world_->count()))return reject("Invalid worksite test actor");
    const auto&person=world_->person(actor);
    if(!person.alive||person.cash<100)return reject("Worksite test actor must be alive with at least 100 cash");
    const int region=person.region;
    auto result=world_->execute(RealmCommand{actor,person.decisions+1,{Intent::Buy,region,int(GoodId::Fiber),2}});
    if(!result.success)return reject("Buy Fiber: "+result.reason);
    result=world_->execute(EstablishmentCommand{actor,world_->person(actor).decisions+1,
        EstablishmentOwnerKind::Person,actor,region,EstablishmentIndustry::ClothWorkshop,1,20});
    if(!result.success)return reject("Create/fund worksite: "+result.reason);
    siteId=result.session;
    result=world_->execute(EstablishmentTransferCommand{actor,world_->person(actor).decisions+1,siteId,
        EstablishmentTransferOperation::DepositGoods,int(GoodId::Fiber),2});
    if(!result.success)return reject("Deposit Fiber: "+result.reason);
    const int64_t begin=world_->game_tick();
    result=world_->execute(ScheduleCommand{actor,world_->person(actor).decisions+1,
        ScheduleCommand::Operation::Submit,0,begin,begin+kSubdayUnitsPerDay,0,5,
        ActivityKind::Work,region,siteId});
    if(!result.success)return reject("Submit Work: "+result.reason);
    result=world_->execute(RealmClockCommand{RealmClockCommand::Operation::StepOneDay,world_->pause_revision()});
    if(!result.success)return reject("Settle workday: "+result.reason);
    const auto found=std::find_if(world_->establishments().begin(),world_->establishments().end(),
        [&](const Establishment&site){return site.id==siteId;});
    if(found==world_->establishments().end())return reject("Worksite disappeared after settlement");
    actorCash=world_->person(actor).cash;siteCash=found->cash;
    fiber=found->stock[size_t(GoodId::Fiber)];cloth=found->stock[size_t(GoodId::Cloth)];
    const auto paid=found->work_pay.paid_units.find(actor);
    paidUnits=paid==found->work_pay.paid_units.end()?0:paid->second;
    if(!world_->invariant()||fiber!=1||cloth!=1||paidUnits<=0)
        return reject("Worksite cycle validation failed: fiber="+std::to_string(fiber)+
            " cloth="+std::to_string(cloth)+" paid="+std::to_string(paidUnits)+
            " site_cash="+std::to_string(siteCash)+" invariant="+(world_->invariant()?"true":"false"));
    error_.clear();return true;
}
bool RealmRuntime::save(const std::string&path){if(!loaded_)return false;try{std::ostringstream s;s<<"POLYEC_RUNTIME_1 "<<seed_<<' '<<inference_count_<<' '<<std::setprecision(17)<<inference_ms_<<' '<<std::quoted(bundle_)<<' '<<memories_.size()<<' ';for(auto&m:memories_)for(float v:m)s<<std::setprecision(9)<<v<<' ';s<<std::quoted(world_->serialize(bundle_));auto data=s.str();auto dest=std::filesystem::u8path(path),temp=dest;temp+=".tmp";{std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<checksum(data)<<'\n'<<data;out.flush();if(!out)throw std::runtime_error("Save write failed");}auto backup=dest;backup+=".bak";if(std::filesystem::exists(dest))std::filesystem::copy_file(dest,backup,std::filesystem::copy_options::overwrite_existing);std::filesystem::rename(temp,dest);error_.clear();return true;}catch(const std::exception&e){error_=e.what();return false;}}
bool RealmRuntime::restore(const std::string&path, bool interactive){
    if(!loaded_)return false;
    std::ifstream in(std::filesystem::u8path(path),std::ios::binary);
    uint64_t hash=0;in>>hash;in.get();
    std::string data((std::istreambuf_iterator<char>(in)),{});
    if(!in||data.size()>256*1024*1024||checksum(data)!=hash){error_="Save corrupt/checksum failed";return false;}
    std::istringstream s(data);std::string magic,bundle,world;uint64_t seed=0,count=0;double latency=0;size_t n=0;
    s>>magic>>seed>>count>>latency>>std::quoted(bundle)>>n;
    if(magic!="POLYEC_RUNTIME_1"||bundle!=bundle_||n<128||n>1000000){error_="Save schema/model mismatch";return false;}
    std::vector<std::array<float,64>> memories(n);
    for(auto&m:memories)for(float&v:m){s>>v;if(!s||!std::isfinite(v)){error_="Save memory invalid";return false;}}
    s>>std::quoted(world);
    if(!s){error_="Save incomplete";return false;}
    Realm restored(1,1,128);std::string error;
    if(!restored.deserialize(world,bundle_,error)||n!=size_t(restored.count())){error_="Save world invalid: "+error;return false;}
    if(restored.timeline_contract()&&!atlas_manual_test_){error_="Timeline world not supported by this runtime";return false;}
    if(atlas_manual_test_&&(restored.regions()!=int(kAtlasTerritoryCount)-1||
       restored.gameplay_config()!=world_->gameplay_config())){
        error_="Manual Atlas save does not match the active Atlas scenario";return false;
    }
    if(!restored.gameplay_config().historical_defaults()&&
       (!atlas_manual_test_||restored.regions()!=int(kAtlasTerritoryCount)-1||
        restored.gameplay_config()!=world_->gameplay_config())){
        error_="Gameplay configuration not supported by this model bundle";return false;
    }
    if(restored.model_contract()!=loaded_model_contract_&&!allow_incompatible_pilot_for_development_&&!atlas_manual_test_){error_="Model contract mismatch: "+std::string(restored.model_contract());return false;}
    if(interactive&&!restored.paused()){
        auto pauseResult=restored.execute(RealmClockCommand{RealmClockCommand::Operation::Pause,restored.pause_revision()});
        if(!pauseResult.success){error_="Restore pause normalization failed: "+pauseResult.reason;return false;}
    }
    *world_=std::move(restored);memories_=std::move(memories);memory_roles_.clear();sync_roles();seed_=seed;inference_count_=count;inference_ms_=latency;error_.clear();return true;
}
bool RealmRuntime::restore_interactive(const std::string&path){return restore(path,true);}
}
