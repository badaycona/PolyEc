#include "polyec/realm_runtime.hpp"
#include <algorithm>
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
 auto stale=std::to_string(runtime.world()->fingerprint());check(runtime.advance(1),"time may change after player snapshot");auto current=runtime.world()->fingerprint();check(!runtime.command(1,0,stale)&&runtime.world()->fingerprint()==current,"stale player terms rejected atomically");
 uint64_t ref=0;for(int i=1;i<=256;++i){auto&p=runtime.world()->person(i);if(p.candidate&&p.alive){ref=i;break;}}check(ref>0,"living existing applicant");
 check(runtime.save(path),"pre appointment snapshot");auto before=memories(path);check(std::any_of(before[ref-1].begin(),before[ref-1].end(),[](float x){return x!=0;}),"actual recurrent civilian history");
 check(runtime.command(1,15),"player appoint uses native validator");check(runtime.world()->person(ref).family==Family::Governance&&!runtime.world()->person(ref).taxing_authority,"role transition preserves restricted authority");
 check(runtime.save(path),"post appointment snapshot");auto after=memories(path);check(std::all_of(after[ref-1].begin(),after[ref-1].end(),[](float x){return x==0;}),"family transition clears old policy memory");check(before[0]==after[0],"manual appointment preserves ruler memory");
 RealmRuntime restored;check(restored.load_models(paths,argv[4],seed,256)&&restored.restore(path),"restore appointed role with memory");check(runtime.advance(30)&&restored.advance(30),"co-play after role transition");check(runtime.world()->fingerprint()==restored.world()->fingerprint()&&runtime.inference_count()==restored.inference_count(),"exact role transition replay");
 check(runtime.command(ref,31),"advisor independently may resign");check(runtime.world()->person(ref).family==Family::Civilian&&runtime.world()->person(ref).hired_by==0,"same ID returns to civilian");check(runtime.save(path),"post resignation snapshot");auto resigned=memories(path);check(std::all_of(resigned[ref-1].begin(),resigned[ref-1].end(),[](float x){return x==0;}),"resignation clears governance memory");check(restored.restore(path),"restore resignation");check(runtime.advance(30)&&restored.advance(30)&&runtime.world()->fingerprint()==restored.world()->fingerprint(),"resignation continuation exact");
 std::cout<<"RUNTIME CONTRACTS: "<<checks<<" checks passed\n";return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}
