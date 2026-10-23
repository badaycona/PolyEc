#include "polyec/realm_runtime.hpp"
#include "polyec/model_manifest.hpp"
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
 {const auto root=std::filesystem::temp_directory_path()/"polyec-model-manifest-contract-test";std::filesystem::remove_all(root);const auto familyDir=root/"civilian";std::filesystem::create_directories(familyDir);const auto model=familyDir/"civilian.onnx";
  {std::ofstream manifest(familyDir/"manifest.json");manifest<<R"({"family":"civilian","schema":"realm-v31","model_contract":"realm-household-fund-goods30-v33","sha256":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","observation":3766,"actions":282,"hidden":64,"trained_with_rl":true,"imitation":false})";}
 ModelManifest parsed;std::string manifestError;check(read_model_manifest(model,parsed,manifestError),"read current model manifest");bool legacy=false;
  check(compatible_model_manifest(parsed,"civilian","realm-household-fund-goods30-v33",false,legacy,manifestError)&&!legacy,"accept current multi-partner foreign trade contract");
  ModelManifest oldSchema=parsed;oldSchema.schema="realm-v28";
  check(!compatible_model_manifest(oldSchema,"civilian","realm-household-fund-goods30-v33",true,legacy,manifestError),"reject an older Realm schema rather than relabeling its action meanings");
 ModelManifest priorMunicipalRelief=parsed;priorMunicipalRelief.model_contract="realm-household-fund-goods27-v26";
  check(!compatible_model_manifest(priorMunicipalRelief,"civilian","realm-household-fund-goods30-v33",false,legacy,manifestError),"reject household v26 because candidate meanings changed after municipal aid");
 ModelManifest priorTimelineRelief=parsed;priorTimelineRelief.model_contract="realm-worksite-payroll-goods27-config-v29";
  check(!compatible_model_manifest(priorTimelineRelief,"civilian","realm-worksite-payroll-goods27-config-v30",false,legacy,manifestError),"reject timeline v29 because foreign trade now depends on live route state");
  const auto customCapabilityContract=Realm::scenario(1,1,-1,-1,-1,-1,-1,-1,-1,-1,0b101001).model_contract();
  ModelManifest customCapabilityModel=parsed;customCapabilityModel.model_contract=customCapabilityContract;
  check(compatible_model_manifest(customCapabilityModel,"civilian",customCapabilityContract,false,legacy,manifestError)&&!legacy,"accept a model declaring the selected capability schema");
  check(!compatible_model_manifest(customCapabilityModel,"civilian","realm-household-fund-goods27-config-v16",true,legacy,manifestError),"development override cannot load a custom-schema model under the default capability schema");
  check(!compatible_model_manifest(parsed,"civilian",customCapabilityContract,false,legacy,manifestError),"reject a default-schema model when a scenario selects another capability set");
  ModelManifest oldHousehold=parsed;oldHousehold.model_contract="realm-household-fund-goods27-v5";
  check(!compatible_model_manifest(oldHousehold,"civilian","realm-household-fund-goods27-v20",false,legacy,manifestError),"reject household v5 model after body-health transition change");
  check(!compatible_model_manifest(oldHousehold,"civilian","realm-household-fund-goods27-v20",true,legacy,manifestError),"development override cannot relabel household v5 as v17");
  ModelManifest priorHousehold=parsed;priorHousehold.model_contract="realm-household-fund-goods27-v6";
  check(!compatible_model_manifest(priorHousehold,"civilian","realm-household-fund-goods27-v20",false,legacy,manifestError),"reject household v6 model after Rest recovery change");
  check(!compatible_model_manifest(priorHousehold,"civilian","realm-household-fund-goods27-v20",true,legacy,manifestError),"development override cannot relabel household v6 as v17");
  priorHousehold.model_contract="realm-household-fund-goods27-v7";
  check(!compatible_model_manifest(priorHousehold,"civilian","realm-household-fund-goods27-v20",false,legacy,manifestError),"reject household v7 model after personality observation change");
  check(!compatible_model_manifest(priorHousehold,"civilian","realm-household-fund-goods27-v20",true,legacy,manifestError),"development override cannot relabel household v7 as v17");
  priorHousehold.model_contract="realm-household-fund-goods27-v8";
  check(!compatible_model_manifest(priorHousehold,"civilian","realm-household-fund-goods27-v20",false,legacy,manifestError),"reject household v8 model after agent knowledge observation change");
  check(!compatible_model_manifest(priorHousehold,"civilian","realm-household-fund-goods27-v20",true,legacy,manifestError),"development override cannot relabel household v8 as v17");
  ModelManifest oldTimeline=parsed;oldTimeline.model_contract="realm-worksite-payroll-goods27-config-v6";
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",false,legacy,manifestError),"reject timeline v6 model after body observation change");
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",true,legacy,manifestError),"development override cannot relabel timeline v6 as v19");
  oldTimeline.model_contract="realm-worksite-payroll-goods27-config-v7";
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",false,legacy,manifestError),"reject timeline v7 model after production skill semantics change");
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",true,legacy,manifestError),"development override cannot relabel timeline v7 as v19");
  oldTimeline.model_contract="realm-worksite-payroll-goods27-config-v8";
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",false,legacy,manifestError),"reject timeline v8 model after Rest recovery change");
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",true,legacy,manifestError),"development override cannot relabel timeline v8 as v19");
  oldTimeline.model_contract="realm-worksite-payroll-goods27-config-v9";
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",false,legacy,manifestError),"reject timeline v9 model after personality observation change");
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",true,legacy,manifestError),"development override cannot relabel timeline v9 as v19");
  oldTimeline.model_contract="realm-worksite-payroll-goods27-config-v10";
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",false,legacy,manifestError),"reject timeline v10 model after agent knowledge observation change");
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-config-v30",true,legacy,manifestError),"development override cannot relabel timeline v10 as v19");
  oldTimeline.model_contract="realm-worksite-payroll-goods27-v6";
  check(!compatible_model_manifest(oldTimeline,"civilian","realm-worksite-payroll-goods27-v22",false,legacy,manifestError),"reject historical-default timeline v6 model after body observation change");
  check(!compatible_model_manifest(parsed,"economic","realm-household-fund-goods27-v20",true,legacy,manifestError),"reject wrong family under dev override");
  check(!compatible_model_manifest(parsed,"civilian","wrong-contract",true,legacy,manifestError),"reject contract mismatch under dev override");
  parsed.actions=80;check(!compatible_model_manifest(parsed,"civilian","realm-household-fund-goods27-v20",true,legacy,manifestError),"reject old action dimension under dev override");
 parsed.actions=89;parsed.observation=1196;check(!compatible_model_manifest(parsed,"civilian","realm-household-fund-goods27-v20",true,legacy,manifestError),"dev override cannot bypass current tensor shape");
  {std::ofstream manifest(familyDir/"manifest.json",std::ios::trunc);manifest<<R"({"family":"civilian","schema":"realm-v9","model_contract":"realm-household-fund-goods27-v3","sha256":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","observation":1088,"actions":80,"hidden":64,"trained_with_rl":true,"imitation":false})";}
  check(read_model_manifest(model,parsed,manifestError),"read v9 model metadata");
 check(!compatible_model_manifest(parsed,"civilian","realm-household-fund-goods27-v20",true,legacy,manifestError),"dev override cannot bypass legacy tensor shape");
  {std::ofstream manifest(familyDir/"manifest.json",std::ios::trunc);manifest<<R"({"family":"civilian","schema":"realm-v8","model_contract":"","sha256":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","observation":1004,"actions":73,"hidden":64,"trained_with_rl":true,"imitation":false})";}
  check(!read_model_manifest(model,parsed,manifestError),"reject empty declared model contract");
  {std::ofstream manifest(familyDir/"manifest.json",std::ios::trunc);manifest<<R"({"family":"civilian","schema":"realm-v8","sha256":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","observation":1004,"actions":73,"hidden":64,"trained_with_rl":true,"imitation":false})";}
  check(read_model_manifest(model,parsed,manifestError),"read v7 legacy metadata");
  check(!compatible_model_manifest(parsed,"civilian","realm-household-fund-goods27-v20",false,legacy,manifestError),"reject unversioned old metadata by default");
  check(!compatible_model_manifest(parsed,"civilian","realm-household-fund-goods27-v20",true,legacy,manifestError),"dev override cannot bypass current tensor shape change");
  std::filesystem::remove_all(root);
 }
 Realm inbox(37,6,256);check(inbox.execute({1,1,{Intent::Offer,2,0,50,30}}).success,"first funded proposal");check(inbox.execute({8,1,{Intent::Offer,2,0,50,30}}).success,"second funded proposal");inbox.advance(12);auto offers=inbox.candidates(2);auto observed_mask=inbox.mask(2);check(observed_mask[28]>0,"first proposal observed");check(inbox.offers(3).empty()&&inbox.offers(2).size()==2,"proposal terms restricted to sender and ready recipient");check(inbox.execute({2,1,inbox.candidates(2)[29]}).success,"resolve first proposal");check(inbox.mask(2)[28]>0&&revalidate_snapshot(inbox,2,offers,observed_mask)[28]==0,"batch cannot accept a different unobserved contract");
 auto terms=inbox.candidates(1);auto term_mask=inbox.mask(1);check(inbox.execute({1,2,{Intent::Project,inbox.person(1).region,0,200,60}}).success,"treasury changes after observation");check(revalidate_snapshot(inbox,1,terms,term_mask)[6]==0,"batch cannot silently change observed aid amount");
 std::array<std::string,3>paths{argv[1],argv[2],argv[3]};std::string path=argv[5];uint64_t seed=0;
 for(int i=1;i<2000;++i){Realm w(i,6,256);auto d=w.draft();if(std::find(d.cards.begin(),d.cards.end(),6)!=d.cards.end()){seed=i;break;}}
 check(seed>0,"A07 runtime fixture");RealmRuntime runtime;
 {RealmRuntime unloaded;uint64_t orderId=99;
  check(!unloaded.market_order(1,MarketOrderOperation::Place,0,0,1,1,100,orderId),
        "unloaded native runtime rejects market order");
  check(orderId==0&&unloaded.error()=="Realm not loaded","unloaded market order clears result ID and explains rejection");}
 if(!runtime.load_models(paths,argv[4],seed,256,false)){
  check(!runtime.error().empty(),"runtime explains why the historical v6 bundle is incompatible with the current Realm");
  RealmRuntime development;check(!development.load_models(paths,argv[4],seed,256,true),"development override cannot bypass changed v9/v11 tensor shape");
  check(!development.loaded(),"historical bundle rejection leaves runtime unloaded");
  std::cout<<"RUNTIME CONTRACTS: "<<checks<<" checks passed; historical bundle rejected; v10 inference not run because no compatible trained bundle is installed\n";return 0;
 }
 check(runtime.loaded(),"native v7 family models");check(runtime.choose(6),"choose referrals");check(runtime.advance(30)&&runtime.advance(5),"civilian memories populated before appointment");
 uint64_t marketBuyer=0;int marketRegion=0;int64_t marketPrice=0;
 for(uint64_t id=1;id<=uint64_t(runtime.world()->count());++id){const auto&person=runtime.world()->person(id);
  if(!person.alive||person.age_days<6*365)continue;
  const auto household=runtime.world()->household_of(id);
  if(runtime.world()->household_authority(household)!=id)continue;
  const auto price=runtime.world()->view(id).provinces[person.region].price[1];
  if(person.cash<price)continue;
  marketBuyer=id;marketRegion=person.region;marketPrice=price;break;
 }
 check(marketBuyer>0,"native market-order fixture finds an adult household authority with enough personal cash");
 const auto marketFingerprint=runtime.world()->fingerprint();const auto marketMoney=runtime.world()->total_money();
 uint64_t marketOrderId=0;
 check(!runtime.market_order(marketBuyer,MarketOrderOperation::Place,0,marketRegion,1,1,marketPrice,
                            marketOrderId,"stale-native-market-view")&&marketOrderId==0&&
       runtime.world()->fingerprint()==marketFingerprint,
       "native market-order command rejects a stale player view before changing the Realm");
 check(runtime.market_order(marketBuyer,MarketOrderOperation::Place,0,marketRegion,1,1,marketPrice,
                           marketOrderId,std::to_string(marketFingerprint))&&marketOrderId>0,
       "native runtime places order through the shared Realm command");
 check(runtime.world()->market_orders().size()==1&&runtime.world()->market_orders().front().id==marketOrderId&&
       runtime.world()->market_escrow()==marketPrice&&runtime.world()->total_money()==marketMoney,
       "native place reserves exactly the requested funds without changing total money");
 const auto orderFingerprint=runtime.world()->fingerprint();uint64_t cancelledOrderId=0;
 check(runtime.market_order(marketBuyer,MarketOrderOperation::Cancel,marketOrderId,0,0,0,0,
                           cancelledOrderId,std::to_string(orderFingerprint))&&cancelledOrderId==marketOrderId,
       "native owner cancels the order through the shared Realm command");
 check(runtime.world()->market_orders().empty()&&runtime.world()->market_escrow()==0&&
       runtime.world()->total_money()==marketMoney&&runtime.world()->invariant(),
       "native cancellation refunds the reservation and preserves the Realm invariant");
 uint64_t titleActor=0,titleRecipient=0,titleId=0;
 for(const auto&holding:runtime.world()->land_holdings()){
  const auto&holder=runtime.world()->person(holding.title_holder);
  if(!holder.alive||holder.age_days<18*365)continue;
  for(uint64_t id=1;id<=uint64_t(runtime.world()->count());++id){const auto&candidate=runtime.world()->person(id);
   if(id!=holder.id&&candidate.alive&&candidate.age_days>=18*365){titleActor=holder.id;titleRecipient=id;titleId=holding.id;break;}}
  if(titleId)break;
 }
 check(titleId>0,"runtime fixture finds adult title holder and living adult recipient");
 const auto useStartFingerprint=runtime.world()->fingerprint();
 check(runtime.land_use_grant(titleActor,titleId,titleRecipient,std::to_string(useStartFingerprint)),
       "native runtime grants land use through Realm command");
 auto useRights=runtime.world()->land_use_rights();
 check(useRights.size()==1&&useRights.front().holding_id==titleId&&useRights.front().grantor==titleActor&&
       useRights.front().grantee==titleRecipient&&runtime.world()->invariant(),
       "native runtime exposes the active whole-holding use right");
 auto grantedFingerprint=runtime.world()->fingerprint();
 check(!runtime.land_title_transfer(titleActor,titleId,titleRecipient,std::to_string(grantedFingerprint))&&
       runtime.error()=="Revoke land use right before title transfer"&&runtime.world()->fingerprint()==grantedFingerprint,
       "native runtime blocks title transfer while use permission is active");
 check(runtime.land_use_revoke(titleActor,titleId,std::to_string(grantedFingerprint))&&
       runtime.world()->land_use_rights().empty()&&runtime.world()->invariant(),
       "native runtime revokes whole-holding use and preserves invariant");
 const auto transferFingerprint=runtime.world()->fingerprint();const auto transferMoney=runtime.world()->total_money();
 check(runtime.land_title_transfer(titleActor,titleId,titleRecipient,std::to_string(transferFingerprint)),
       "native runtime commits title transfer through Realm command");
 const auto updatedHoldings=runtime.world()->land_holdings();
 const auto transferred=std::find_if(updatedHoldings.begin(),updatedHoldings.end(),
       [titleId](const LandHolding&holding){return holding.id==titleId;});
 check(transferred!=updatedHoldings.end()&&transferred->title_holder==titleRecipient&&
       runtime.world()->fingerprint()!=transferFingerprint&&runtime.world()->total_money()==transferMoney&&runtime.world()->invariant(),
       "native runtime title transfer preserves ledger and whole-world invariant");
 check(!runtime.land_title_transfer(titleActor,titleId,titleRecipient,std::to_string(transferFingerprint))&&
       runtime.error()=="World changed; refresh before committing this title transfer",
       "native runtime rejects stale title command before execution");
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
 check(runtime.command(ref,31),"advisor independently may resign");check(runtime.world()->person(ref).family==Family::Civilian&&runtime.world()->person(ref).office_appointment.hired_by==0,"same ID returns to civilian");check(runtime.save(path),"post resignation snapshot");auto resigned=memories(path);check(std::all_of(resigned[ref-1].begin(),resigned[ref-1].end(),[](float x){return x==0;}),"resignation clears governance memory");check(restored.restore(path),"restore resignation");check(runtime.advance(30)&&restored.advance(30)&&runtime.world()->fingerprint()==restored.world()->fingerprint(),"resignation continuation exact");
  RealmRuntime draftless;check(draftless.load_models(paths,argv[4],seed,256),"draftless runtime loads");uint64_t draftlessTick=draftless.world()->game_tick();uint64_t draftlessCount=draftless.inference_count();auto draftlessFingerprint=draftless.world()->fingerprint();check(!draftless.step_one_day(),"manual step requires chosen draft");check(draftless.error()=="Choose a draft card first","manual step draft preflight error");check(draftless.world()->game_tick()==draftlessTick,"draft preflight did not move clock");check(draftless.inference_count()==draftlessCount,"draft preflight did not infer");check(draftless.world()->fingerprint()==draftlessFingerprint,"draft preflight did not mutate world");
  check(runtime.resume(),"resume runtime before unpaused save");check(runtime.save(path),"save unpaused runtime");RealmRuntime restoredUnpaused;check(restoredUnpaused.load_models(paths,argv[4],seed,256),"restore model load");check(restoredUnpaused.restore_interactive(path),"interactive restore from unpaused save");check(restoredUnpaused.paused(),"interactive restore normalizes unpaused save");check(runtime.pause(),"pause live after unpaused save");check(restoredUnpaused.world()->fingerprint()==runtime.world()->fingerprint(),"interactive restore fingerprint");  check(restoredUnpaused.inference_count()==runtime.inference_count(),"interactive restore inference count");
  const std::string missingPath=path+".missing";std::filesystem::remove(missingPath);uint64_t missingFingerprint=runtime.world()->fingerprint();uint64_t missingRevision=runtime.pause_revision();check(!runtime.restore(missingPath),"missing save rejected");check(!runtime.error().empty(),"missing save error surfaced");check(runtime.world()->fingerprint()==missingFingerprint,"missing save keeps world");check(runtime.paused(),"missing save keeps paused");check(runtime.pause_revision()==missingRevision,"missing save keeps revision");
  std::cout<<"RUNTIME CONTRACTS: "<<checks<<" checks passed\n";return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}
