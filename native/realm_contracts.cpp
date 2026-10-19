#include "polyec/realm_runtime.hpp"
#include <algorithm>
#include <optional>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace polyec;
int checks=0;void check(bool ok,const char*s){++checks;if(!ok)throw std::runtime_error(s);}
std::vector<std::array<float,64>> memories(const std::string&path){std::ifstream in(std::filesystem::u8path(path));uint64_t checksum,seed,count;double latency;std::string magic,bundle;size_t n;in>>checksum>>magic>>seed>>count>>latency>>std::quoted(bundle)>>n;check(magic=="POLYEC_RUNTIME_1"&&n==256,"runtime memory format");std::vector<std::array<float,64>> result(n);for(auto&m:result)for(auto&v:m)in>>v;check(bool(in),"complete memory snapshot");return result;}
int main(int argc,char**argv){if(argc!=6)return 2;try{
 Realm inbox(37,6,256);check(inbox.execute({1,1,{Intent::Offer,2,0,50,30}}).success,"first funded proposal");check(inbox.execute({8,1,{Intent::Offer,2,0,50,30}}).success,"second funded proposal");inbox.advance(12);auto offers=inbox.candidates(2);auto observed_mask=inbox.mask(2);check(observed_mask[28]>0,"first proposal observed");check(inbox.offers(3).empty()&&inbox.offers(2).size()==2,"proposal terms restricted to sender and ready recipient");check(inbox.execute({2,1,inbox.candidates(2)[29]}).success,"resolve first proposal");check(inbox.mask(2)[28]>0&&revalidate_snapshot(inbox,2,offers,observed_mask)[28]==0,"batch cannot accept a different unobserved contract");
 auto terms=inbox.candidates(1);auto term_mask=inbox.mask(1);check(inbox.execute({1,2,{Intent::Project,inbox.person(1).region,0,200,60}}).success,"treasury changes after observation");check(revalidate_snapshot(inbox,1,terms,term_mask)[6]==0,"batch cannot silently change observed aid amount");
 std::array<std::string,3>paths{argv[1],argv[2],argv[3]};std::string path=argv[5];uint64_t seed=0;
 for(int i=1;i<2000;++i){Realm w(i,6,256);auto d=w.draft();if(std::find(d.cards.begin(),d.cards.end(),6)!=d.cards.end()){seed=i;break;}}
 check(seed>0,"A07 runtime fixture");RealmRuntime runtime;check(runtime.load_models(paths,argv[4],seed,256),"native family models");check(runtime.choose(6),"choose referrals");check(runtime.advance(30)&&runtime.advance(5),"civilian memories populated before appointment");
 RealmRuntime unloaded;check(!unloaded.paused()&&unloaded.pause_revision()==0&&!unloaded.pause()&&!unloaded.resume(),"unloaded runtime clock rejected");
 uint64_t clockRevision=runtime.pause_revision();check(!runtime.paused(),"loaded runtime starts unpaused");
 check(runtime.pause(),"runtime pause"); check(runtime.paused(),"runtime paused");check(runtime.pause_revision()==clockRevision+1,"pause increments revision");check(runtime.pause(),"pause idempotent");check(runtime.pause_revision()==clockRevision+1,"idempotent pause keeps revision");check(runtime.pause(0)==false,"stale pause expected rejected");check(runtime.paused()&&runtime.pause_revision()==clockRevision+1,"stale pause left state unchanged");
 auto pausedFingerprint=runtime.world()->fingerprint();uint64_t pausedCount=runtime.inference_count();uint64_t pausedTick=runtime.world()->game_tick();
  check(!runtime.advance(1),"paused runtime advance rejected");check(runtime.world()->fingerprint()==pausedFingerprint,"paused advance fingerprint unchanged");check(runtime.inference_count()==pausedCount,"paused advance inference unchanged");check(runtime.world()->game_tick()==pausedTick,"paused advance did not move clock");
  uint64_t manualRevision=runtime.pause_revision();int manualDay=runtime.world()->day();uint64_t manualTick=runtime.world()->game_tick();uint64_t manualCount=runtime.inference_count();auto manualFingerprint=runtime.world()->fingerprint();
  check(runtime.paused(),"runtime paused before manual step");
  check(runtime.step_one_day(),"paused manual one-day succeeds");
  check(runtime.paused(),"paused manual one-day leaves paused");
  check(runtime.pause_revision()==manualRevision,"paused manual one-day leaves revision");
  check(runtime.world()->game_tick()==manualTick+64,"paused manual one-day 64 ticks");
  check(runtime.world()->day()==manualDay+1,"paused manual one-day one day");
  check(runtime.inference_count()>manualCount,"paused manual one-day runs inference");
  check(runtime.world()->fingerprint()!=manualFingerprint,"paused manual one-day changes world");
  check(runtime.world()->invariant(),"paused manual one-day invariant");
 check(runtime.resume(),"runtime resume");check(!runtime.paused(),"runtime resumed");uint64_t resumedRevision=runtime.pause_revision();uint64_t resumedFingerprint=runtime.world()->fingerprint();uint64_t resumedCount=runtime.inference_count();uint64_t resumedTick=runtime.world()->game_tick();check(runtime.resume(resumedRevision-1)==false,"stale resume expected rejected");check(runtime.error()=="Stale clock revision","stale resume error");check(!runtime.paused(),"stale resume kept unpaused");check(runtime.pause_revision()==resumedRevision,"stale resume kept revision");check(runtime.world()->fingerprint()==resumedFingerprint,"stale resume kept fingerprint");check(runtime.inference_count()==resumedCount,"stale resume kept inference");check(runtime.world()->game_tick()==resumedTick,"stale resume kept clock");check(runtime.pause(resumedRevision-1)==false,"stale pause expected rejected");check(runtime.error()=="Stale clock revision","stale pause error");check(!runtime.paused(),"stale pause kept unpaused");check(runtime.pause_revision()==resumedRevision,"stale pause kept revision");
 check(runtime.pause(),"pause before save");check(runtime.save(path),"save paused runtime");RealmRuntime restoredPaused;check(restoredPaused.load_models(paths,argv[4],seed,256),"restore model load");check(restoredPaused.restore(path),"restore paused runtime");check(restoredPaused.paused(),"restored paused flag");check(restoredPaused.pause_revision()==runtime.pause_revision(),"restored pause revision");check(restoredPaused.world()->fingerprint()==runtime.world()->fingerprint(),"restored paused fingerprint");check(restoredPaused.inference_count()==runtime.inference_count(),"restored inference count");check(restoredPaused.resume(),"restored resume");check(!restoredPaused.paused(),"restored manual resume");check(restoredPaused.advance(1),"advance after restored resume");
  check(runtime.resume(),"resume original runtime");
  uint64_t unpausedRevision=runtime.pause_revision();int unpausedDay=runtime.world()->day();uint64_t unpausedTick=runtime.world()->game_tick();uint64_t unpausedCount=runtime.inference_count();auto unpausedFingerprint=runtime.world()->fingerprint();
  check(!runtime.paused(),"runtime unpaused before manual step");
  check(runtime.step_one_day(),"unpaused manual one-day succeeds");
  check(!runtime.paused(),"unpaused manual one-day leaves unpaused");
  check(runtime.pause_revision()==unpausedRevision,"unpaused manual one-day leaves revision");
  check(runtime.world()->game_tick()==unpausedTick+64,"unpaused manual one-day 64 ticks");
  check(runtime.world()->day()==unpausedDay+1,"unpaused manual one-day one day");
  check(runtime.inference_count()>unpausedCount,"unpaused manual one-day runs inference");
  check(runtime.world()->fingerprint()!=unpausedFingerprint,"unpaused manual one-day changes world");
  check(runtime.world()->invariant(),"unpaused manual one-day invariant");
 auto stale=std::to_string(runtime.world()->fingerprint());check(runtime.advance(1),"time may change after player snapshot");auto current=runtime.world()->fingerprint();check(!runtime.command(1,0,stale)&&runtime.world()->fingerprint()==current,"stale player terms rejected atomically");
 uint64_t ref=0;for(int i=1;i<=256;++i){auto&p=runtime.world()->person(i);if(p.candidate&&p.alive){ref=i;break;}}check(ref>0,"living existing applicant");
 check(runtime.save(path),"pre appointment snapshot");auto before=memories(path);check(std::any_of(before[ref-1].begin(),before[ref-1].end(),[](float x){return x!=0;}),"actual recurrent civilian history");
 check(runtime.command(1,15),"player appoint uses native validator");check(runtime.world()->person(ref).family==Family::Governance&&!runtime.world()->person(ref).taxing_authority,"role transition preserves restricted authority");
 check(runtime.save(path),"post appointment snapshot");auto after=memories(path);check(std::all_of(after[ref-1].begin(),after[ref-1].end(),[](float x){return x==0;}),"family transition clears old policy memory");check(before[0]==after[0],"manual appointment preserves ruler memory");
 RealmRuntime restored;check(restored.load_models(paths,argv[4],seed,256)&&restored.restore(path),"restore appointed role with memory");check(runtime.advance(30)&&restored.advance(30),"co-play after role transition");check(runtime.world()->fingerprint()==restored.world()->fingerprint()&&runtime.inference_count()==restored.inference_count(),"exact role transition replay");
 check(runtime.command(ref,31),"advisor independently may resign");check(runtime.world()->person(ref).family==Family::Civilian&&runtime.world()->person(ref).hired_by==0,"same ID returns to civilian");check(runtime.save(path),"post resignation snapshot");auto resigned=memories(path);check(std::all_of(resigned[ref-1].begin(),resigned[ref-1].end(),[](float x){return x==0;}),"resignation clears governance memory");check(restored.restore(path),"restore resignation");check(runtime.advance(30)&&restored.advance(30)&&runtime.world()->fingerprint()==restored.world()->fingerprint(),"resignation continuation exact");
  RealmRuntime draftless;check(draftless.load_models(paths,argv[4],seed,256),"draftless runtime loads");uint64_t draftlessTick=draftless.world()->game_tick();uint64_t draftlessCount=draftless.inference_count();auto draftlessFingerprint=draftless.world()->fingerprint();check(!draftless.step_one_day(),"manual step requires chosen draft");check(draftless.error()=="Choose a draft card first","manual step draft preflight error");check(draftless.world()->game_tick()==draftlessTick,"draft preflight did not move clock");check(draftless.inference_count()==draftlessCount,"draft preflight did not infer");check(draftless.world()->fingerprint()==draftlessFingerprint,"draft preflight did not mutate world");
  check(runtime.resume(),"resume runtime before unpaused save");check(runtime.save(path),"save unpaused runtime");RealmRuntime restoredUnpaused;check(restoredUnpaused.load_models(paths,argv[4],seed,256),"restore model load");check(restoredUnpaused.restore_interactive(path),"interactive restore from unpaused save");check(restoredUnpaused.paused(),"interactive restore normalizes unpaused save");check(runtime.pause(),"pause live after unpaused save");check(restoredUnpaused.world()->fingerprint()==runtime.world()->fingerprint(),"interactive restore fingerprint");  check(restoredUnpaused.inference_count()==runtime.inference_count(),"interactive restore inference count");
  const std::string missingPath=path+".missing";std::filesystem::remove(missingPath);uint64_t missingFingerprint=runtime.world()->fingerprint();uint64_t missingRevision=runtime.pause_revision();check(!runtime.restore(missingPath),"missing save rejected");check(!runtime.error().empty(),"missing save error surfaced");check(runtime.world()->fingerprint()==missingFingerprint,"missing save keeps world");check(runtime.paused(),"missing save keeps paused");check(runtime.pause_revision()==missingRevision,"missing save keeps revision");
  std::cout<<"RUNTIME CONTRACTS: "<<checks<<" checks passed\n";return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}
