#include "polyec/realm_runtime.hpp"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace polyec {
namespace {uint64_t mix(uint64_t x){x+=0x9e3779b97f4a7c15ull;x=(x^(x>>30))*0xbf58476d1ce4e5b9ull;x=(x^(x>>27))*0x94d049bb133111ebull;return x^(x>>31);}uint64_t checksum(const std::string&s){uint64_t h=1469598103934665603ull;for(unsigned char c:s){h^=c;h*=1099511628211ull;}return h;}}
std::array<float,realm_actions> revalidate_snapshot(const Realm&w,uint64_t id,const std::array<Candidate,realm_actions>&seen,const std::array<float,realm_actions>&seen_mask){
    auto current=w.candidates(id);auto valid=w.mask(id);
    for(int a=0;a<realm_actions;++a){auto&x=seen[a];auto&y=current[a];valid[a]=valid[a]&&seen_mask[a]&&x.kind==y.kind&&x.target==y.target&&x.good==y.good&&x.amount==y.amount&&x.term==y.term?1.f:0.f;}return valid;
}
bool RealmRuntime::load_models(const std::array<std::string,3>&paths,const std::string&bundle,uint64_t seed,int population){
    loaded_=false;try{for(int i=0;i<3;++i){policies_[i]=std::make_unique<NativePolicy>(realm_obs,realm_actions);if(!policies_[i]->load(paths[i])){error_=policies_[i]->error();return false;}}
    world_=std::make_unique<Realm>(seed,6,population);memories_.assign(population,{});memory_roles_.clear();sync_roles();seed_=seed;bundle_="realm6|"+bundle;inference_count_=0;inference_ms_=0;loaded_=true;error_.clear();return true;}catch(const std::exception&e){error_=e.what();return false;}
}
void RealmRuntime::sync_roles(){for(size_t i=0;i<memories_.size();++i){auto family=world_->person(i+1).family;if(i>=memory_roles_.size())memory_roles_.push_back(family);else if(memory_roles_[i]!=family){memories_[i].fill(0);memory_roles_[i]=family;}}}
bool RealmRuntime::decisions(Family family){sync_roles();auto ids=world_->due(family);for(size_t start=0;start<ids.size();start+=128){int n=int(std::min<size_t>(128,ids.size()-start));std::vector<float> obs,hidden;std::vector<std::array<Candidate,realm_actions>> seen;std::vector<std::array<float,realm_actions>> seen_masks;obs.reserve(n*realm_obs);hidden.reserve(n*64);for(int j=0;j<n;++j){auto id=ids[start+j];auto o=world_->observation(id);seen.push_back(world_->candidates(id));std::array<float,realm_actions> m{};for(int a=0;a<realm_actions;++a)m[a]=o[realm_state+a*realm_features+5];seen_masks.push_back(m);obs.insert(obs.end(),o.begin(),o.end());hidden.insert(hidden.end(),memories_[id-1].begin(),memories_[id-1].end());}
    auto t=std::chrono::steady_clock::now();auto out=policies_[int(family)]->predict(obs,hidden,n);inference_ms_+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();if(!out.success){stop(out.error);return false;}
    for(int j=0;j<n;++j){auto id=ids[start+j];auto m=revalidate_snapshot(*world_,id,seen[j],seen_masks[j]);auto&person=world_->person(id);float max=-1e30f;for(int a=0;a<realm_actions;++a)if(m[a])max=std::max(max,out.logits[j*realm_actions+a]);double sum=0;std::array<double,realm_actions> weights{};for(int a=0;a<realm_actions;++a)if(m[a]){weights[a]=std::exp(std::clamp(double(out.logits[j*realm_actions+a]-max),-80.,0.));sum+=weights[a];}double u=double(mix(seed_^mix(id)^mix(person.sampling))>>11)*0x1.0p-53*sum;int chosen=0;for(int a=0;a<realm_actions;++a){u-=weights[a];if(u<0){chosen=a;break;}}
        auto result=world_->execute({id,person.decisions+1,seen[j][chosen]});if(!result.success){stop("Decision commit failed: "+result.reason);return false;}std::copy_n(out.hidden.data()+j*64,64,memories_[id-1].begin());++inference_count_;
    }}return true;
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
    auto result = world_->execute(RealmClockCommand{RealmClockCommand::Operation::Resume, expected_pause_revision});
    error_ = result.success ? "" : result.reason;
    return result.success;
}
bool RealmRuntime::advance(int days){if(!loaded_||!world_||days<1||days>30){error_="Realm not loaded or day chunk outside1..30";return false;}const int64_t maxTick=365000LL*kSubdayUnitsPerDay;const int64_t requestedTicks=static_cast<int64_t>(days)*kSubdayUnitsPerDay;if(world_->game_tick()>maxTick||requestedTicks>maxTick-world_->game_tick()){error_="Realm advance exceeds maximum horizon";return false;}if(world_->draft().chosen<0){error_="Choose a draft card first";return false;}if(world_->paused()){error_="Realm is paused";return false;}
    try {
        for(int day=0;day<days;++day){for(auto f:{Family::Governance,Family::Economic,Family::Civilian})if(!decisions(f))return false;world_->advance();memories_.resize(world_->count());sync_roles();if(!world_->invariant()){stop("Realm accounting invariant failed");return false;}}error_.clear();return true;
    } catch(const std::exception&e){error_=e.what();return false;}
}
bool RealmRuntime::step_one_day(){if(!loaded_||!world_){error_="Realm not loaded";return false;}const int64_t maxTick=365000LL*kSubdayUnitsPerDay;if(world_->game_tick()>maxTick||64>maxTick-world_->game_tick()){error_="Realm advance exceeds maximum horizon";return false;}if(world_->draft().chosen<0){error_="Choose a draft card first";return false;}
    try {
        for(auto f:{Family::Governance,Family::Economic,Family::Civilian})if(!decisions(f))return false;
        auto result=world_->execute(RealmClockCommand{RealmClockCommand::Operation::StepOneDay,world_->pause_revision()});
        if(!result.success){error_=result.reason;return false;}
        memories_.resize(world_->count());sync_roles();
        if(!world_->invariant()){stop("Realm accounting invariant failed");return false;}
        error_.clear();return true;
    } catch(const std::exception&e){error_=e.what();return false;}
}
bool RealmRuntime::command(uint64_t id,int action,const std::string&expected){if(!loaded_||action<0||action>=realm_actions||!id||id>uint64_t(world_->count())){error_="Invalid player command";return false;}if(!expected.empty()&&expected!=std::to_string(world_->fingerprint())){error_="World changed; refresh before committing this command";return false;}auto r=world_->execute({id,world_->person(id).decisions+1,world_->candidates(id)[action]});if(r.success)sync_roles();error_=r.success?"":r.reason;return r.success;}
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
    if(restored.timeline_contract()){error_="Timeline world not supported by this runtime";return false;}
    if(interactive&&!restored.paused()){
        auto pauseResult=restored.execute(RealmClockCommand{RealmClockCommand::Operation::Pause,restored.pause_revision()});
        if(!pauseResult.success){error_="Restore pause normalization failed: "+pauseResult.reason;return false;}
    }
    *world_=std::move(restored);memories_=std::move(memories);memory_roles_.clear();sync_roles();seed_=seed;inference_count_=count;inference_ms_=latency;error_.clear();return true;
}
bool RealmRuntime::restore_interactive(const std::string&path){return restore(path,true);}
}
