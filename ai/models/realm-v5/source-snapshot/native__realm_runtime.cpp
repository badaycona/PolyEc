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
bool RealmRuntime::load_models(const std::array<std::string,3>&paths,const std::string&bundle,uint64_t seed,int population){
    loaded_=false;try{for(int i=0;i<3;++i){policies_[i]=std::make_unique<NativePolicy>(realm_obs,realm_actions);if(!policies_[i]->load(paths[i])){error_=policies_[i]->error();return false;}}
    world_=std::make_unique<Realm>(seed,6,population);memories_.assign(population,{});seed_=seed;bundle_="realm5|"+bundle;inference_count_=0;inference_ms_=0;loaded_=true;error_.clear();return true;}catch(const std::exception&e){error_=e.what();return false;}
}
bool RealmRuntime::decisions(Family family){auto ids=world_->due(family);for(size_t start=0;start<ids.size();start+=128){int n=int(std::min<size_t>(128,ids.size()-start));std::vector<float> obs,hidden;obs.reserve(n*realm_obs);hidden.reserve(n*64);for(int j=0;j<n;++j){auto id=ids[start+j];auto o=world_->observation(id);obs.insert(obs.end(),o.begin(),o.end());hidden.insert(hidden.end(),memories_[id-1].begin(),memories_[id-1].end());}
    auto t=std::chrono::steady_clock::now();auto out=policies_[int(family)]->predict(obs,hidden,n);inference_ms_+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();if(!out.success){stop(out.error);return false;}
    for(int j=0;j<n;++j){auto id=ids[start+j];auto m=world_->mask(id);auto&person=world_->person(id);float max=-1e30f;for(int a=0;a<realm_actions;++a)if(m[a])max=std::max(max,out.logits[j*realm_actions+a]);double sum=0;std::array<double,realm_actions> weights{};for(int a=0;a<realm_actions;++a)if(m[a]){weights[a]=std::exp(std::clamp(double(out.logits[j*realm_actions+a]-max),-80.,0.));sum+=weights[a];}double u=double(mix(seed_^mix(id)^mix(person.sampling))>>11)*0x1.0p-53*sum;int chosen=0;for(int a=0;a<realm_actions;++a){u-=weights[a];if(u<0){chosen=a;break;}}
        auto result=world_->execute({id,person.decisions+1,world_->candidates(id)[chosen]});if(!result.success){stop("Decision commit failed: "+result.reason);return false;}std::copy_n(out.hidden.data()+j*64,64,memories_[id-1].begin());++inference_count_;
    }}return true;
}
bool RealmRuntime::advance(int days){if(!loaded_||days<1||days>30){error_="Realm not loaded or day chunk outside1..30";return false;}if(world_->draft().chosen<0){error_="Choose a draft card first";return false;}
    for(int day=0;day<days;++day){for(auto f:{Family::Governance,Family::Economic,Family::Civilian})if(!decisions(f))return false;world_->advance();memories_.resize(world_->count());if(!world_->invariant()){stop("Realm accounting invariant failed");return false;}}error_.clear();return true;
}
bool RealmRuntime::command(uint64_t id,int action){if(!loaded_||action<0||action>=realm_actions||!id||id>uint64_t(world_->count())){error_="Invalid player command";return false;}auto r=world_->execute({id,world_->person(id).decisions+1,world_->candidates(id)[action]});error_=r.success?"":r.reason;return r.success;}
bool RealmRuntime::save(const std::string&path){if(!loaded_)return false;try{std::ostringstream s;s<<"POLYEC_RUNTIME_1 "<<seed_<<' '<<inference_count_<<' '<<std::setprecision(17)<<inference_ms_<<' '<<std::quoted(bundle_)<<' '<<memories_.size()<<' ';for(auto&m:memories_)for(float v:m)s<<std::setprecision(9)<<v<<' ';s<<std::quoted(world_->serialize(bundle_));auto data=s.str();auto dest=std::filesystem::u8path(path),temp=dest;temp+=".tmp";{std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<checksum(data)<<'\n'<<data;out.flush();if(!out)throw std::runtime_error("Save write failed");}auto backup=dest;backup+=".bak";if(std::filesystem::exists(dest))std::filesystem::copy_file(dest,backup,std::filesystem::copy_options::overwrite_existing);std::filesystem::rename(temp,dest);error_.clear();return true;}catch(const std::exception&e){error_=e.what();return false;}}
bool RealmRuntime::restore(const std::string&path){if(!loaded_)return false;try{std::ifstream in(std::filesystem::u8path(path),std::ios::binary);uint64_t hash=0;in>>hash;in.get();std::string data((std::istreambuf_iterator<char>(in)),{});if(!in||data.size()>256*1024*1024||checksum(data)!=hash)throw std::runtime_error("Save corrupt/checksum failed");std::istringstream s(data);std::string magic,bundle,world;uint64_t seed,count;double latency;size_t n;s>>magic>>seed>>count>>latency>>std::quoted(bundle)>>n;if(magic!="POLYEC_RUNTIME_1"||bundle!=bundle_||n<128||n>1000000)throw std::runtime_error("Save schema/model mismatch");std::vector<std::array<float,64>> memories(n);for(auto&m:memories)for(float&v:m){s>>v;if(!s||!std::isfinite(v))throw std::runtime_error("Save memory invalid");}s>>std::quoted(world);if(!s)throw std::runtime_error("Save incomplete");Realm restored(1,1,128);std::string error;if(!restored.deserialize(world,bundle_,error)||n!=size_t(restored.count()))throw std::runtime_error("Save world invalid: "+error);*world_=std::move(restored);memories_=std::move(memories);seed_=seed;inference_count_=count;inference_ms_=latency;error_.clear();return true;}catch(const std::exception&e){error_=e.what();return false;}}
}
