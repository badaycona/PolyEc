#include "polyec/campaign.hpp"
#include "polyec/campaign_state_codec.hpp"
#include <array>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <chrono>
using namespace polyec;
int checks=0,failures=0;
void check(bool ok,const char*message){++checks;if(!ok){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
CampaignResult command(Campaign&w,CampaignAction action,int target=1,int64_t quantity=200){return w.execute({1,w.view().player_sequence,action,target,quantity});}
void advance(Campaign&w,int days){while(days>0){int step=std::min(30,days);w.advance(step);days-=step;}}
// R4 golden Campaign state payload (docs/design/R4-campaign-state-codec-plan.md):
// 622 bytes captured from the unchanged pre-refactor implementation with the
// canonical fixture Campaign(37,256) + Weave(2) + Relief(1,200), i.e. the payload
// bytes between the "POLYEC_M4_1 " magic prefix and the trailing quoted Realm
// string of a full Campaign save. SHA-256:
// 7136e667aeb2c5c6d42e028e338beda1dece03c7d36aed01ca2ea2f51a722cb4
// Every byte is emitted as a \xHH escape so the literal is source-encoding-proof.
// Do not regenerate this literal from refactored output.
// R4-GOLDEN-BEGIN
namespace r4_golden{
static const char bytes[]=
"\x33\x37\x20\x32\x20\x33\x20\x31\x33\x20\x30\x20"
"\x30\x20\x30\x20\x39\x20\x31\x20\x32\x20\x35\x20"
"\x38\x30\x20\x22\xc4\x90\xc3\xa8\x6f\x20\x48\xe1"
"\xba\xaf\x63\x20\x53\xc6\xa1\x6e\x22\x20\x31\x20"
"\x31\x20\x30\x20\x32\x20\x30\x20\x33\x20\x32\x30"
"\x30\x20\x22\xc4\x90\xc6\xb0\xe1\xbb\x9d\x6e\x67"
"\x20\x63\x61\x6f\x20\x6e\x67\x75\x79\xc3\xaa\x6e"
"\x22\x20\x31\x20\x30\x20\x30\x20\x30\x20\x34\x20"
"\x32\x20\x32\x34\x30\x20\x22\x43\xe1\xba\xa7\x75"
"\x20\x73\xc3\xb4\x6e\x67\x20\x54\x68\x61\x6e\x68"
"\x22\x20\x31\x20\x30\x20\x30\x20\x34\x20\x35\x20"
"\x32\x20\x33\x30\x30\x20\x22\x53\xc3\xb4\x6e\x67"
"\x20\x72\x61\x20\x63\xe1\xba\xa3\x6e\x67\x22\x20"
"\x31\x20\x30\x20\x30\x20\x30\x20\x36\x20\x32\x20"
"\x33\x30\x30\x20\x22\xc4\x90\xc6\xb0\xe1\xbb\x9d"
"\x6e\x67\x20\x6b\x68\x6f\x20\x68\x6f\xc3\xa0\x6e"
"\x67\x20\x67\x69\x61\x22\x20\x31\x20\x30\x20\x30"
"\x20\x32\x20\x33\x20\x34\x20\x31\x34\x30\x20\x22"
"\xc4\x90\xc3\xa8\x6f\x20\x70\x68\xc3\xad\x61\x20"
"\x62\xe1\xba\xaf\x63\x22\x20\x31\x20\x30\x20\x30"
"\x20\x33\x20\x34\x20\x34\x20\x31\x34\x30\x20\x22"
"\xc4\x90\xc6\xb0\xe1\xbb\x9d\x6e\x67\x20\x72\xe1"
"\xbb\xab\x6e\x67\x22\x20\x31\x20\x30\x20\x30\x20"
"\x35\x20\x36\x20\x34\x20\x32\x34\x30\x20\x22\xc4"
"\x90\xc6\xb0\xe1\xbb\x9d\x6e\x67\x20\x64\x75\x79"
"\xc3\xaa\x6e\x20\x68\xe1\xba\xa3\x69\x22\x20\x31"
"\x20\x30\x20\x30\x20\x31\x20\x36\x20\x31\x38\x20"
"\x36\x30\x20\x22\xc4\x90\xc6\xb0\xe1\xbb\x9d\x6e"
"\x67\x20\x76\xc3\xb2\x6e\x67\x20\x6d\x69\xe1\xbb"
"\x81\x6e\x20\x74\xc3\xa2\x79\x22\x20\x31\x20\x30"
"\x20\x30\x20\x31\x20\x31\x31\x20\x31\x32\x20\x36"
"\x20\x31\x20\x30\x20\x30\x20\x31\x36\x20\x32\x30"
"\x30\x20\x34\x35\x34\x35\x20\x30\x20\x34\x20\x36"
"\x20\x30\x20\x32\x20\x31\x20\x30\x20\x32\x20\x35"
"\x20\x30\x20\x32\x20\x37\x20\x32\x20\x35\x20\x31"
"\x36\x20\x30\x20\x31\x31\x20\x31\x20\x30\x20\x30"
"\x20\x30\x20\x32\x31\x37\x20\x31\x31\x39\x20\x32"
"\x20\x30\x20\x30\x20\x31\x20\x32\x31\x37\x20\x31"
"\x30\x30\x20\x33\x20\x30\x20\x30\x20\x32\x20\x32"
"\x31\x37\x20\x31\x38\x34\x20\x34\x20\x30\x20\x30"
"\x20\x33\x20\x32\x31\x37\x20\x31\x37\x39\x20\x35"
"\x20\x30\x20\x30\x20\x34\x20\x32\x31\x37\x20\x31"
"\x31\x38\x20\x36\x20\x30\x20\x30\x20\x35\x20\x32"
"\x31\x37\x20\x31\x36\x30\x20\x37\x20\x30\x20\x30"
"\x20\x36\x20\x32\x31\x37\x20\x33\x34\x30\x20\x38"
"\x20\x30\x20\x30\x20\x31\x20\x32\x31\x38\x20\x31"
"\x20\x39\x20\x30\x20\x30\x20\x31\x20\x32\x30\x34"
"\x20\x32\x30\x30\x20\x31\x30\x20\x30\x20\x30\x20"
"\x32\x20\x32\x31\x30\x20\x36\x20\x31\x32\x20\x31"
"\x31\x20\x30\x20\x31\x20\x32\x30\x30\x20\x32\x30"
"\x30\x20\x36\x20\x32\x33\x20\x32\x34\x20\x32\x35"
"\x20\x35\x32\x20\x35\x33\x20\x35\x34\x20"
;
static const std::string value(bytes,sizeof(bytes)-1);
}
// R4-GOLDEN-END

int main(){try{
    {
        detail::CampaignStatePayload source;
        for(int legacy=0;legacy<kLegacyGoodCount;++legacy){
            Cargo item{};item.id=uint64_t(legacy+1);item.from=0;item.to=1;item.good=legacy;item.depart=0;item.arrives=2;item.quantity=legacy+1;item.path={0,1};item.leg_starts={0};item.leg_ends={0};item.leg_arrivals={2};source.cargo.push_back(item);
        }
        std::ostringstream encoded;detail::write_campaign_state(encoded,source);detail::CampaignStatePayload decoded;std::istringstream input(encoded.str());detail::read_campaign_state(input,decoded);
        std::array<int64_t,kGoodIdCount> expected{};for(int legacy=0;legacy<kLegacyGoodCount;++legacy)expected[size_t(canonical_from_legacy_good(legacy))]=legacy+1;
        check(decoded.bonded_goods==expected,"M4 legacy cargo IDs map once to canonical bonded goods");
    }
    Campaign w(37,256);check(w.invariant(),"opening ledger");auto opening=w.view();check(opening.regions.size()==7&&opening.routes.size()==9,"six provinces and crown share core map");check(opening.cases.empty(),"case reporting has game-time delay");
    Campaign clock(37,256);check(!clock.paused()&&clock.pause_revision()==0,"campaign starts unpaused");clock.pause();check(clock.paused()&&clock.pause_revision()==1&&clock.view().paused&&clock.view().pause_revision==1,"campaign pause is Realm-owned");auto paused=clock.fingerprint();bool rejected=false;try{clock.advance(1);}catch(const std::runtime_error&e){rejected=std::string(e.what())=="Campaign is paused";}check(rejected&&clock.fingerprint()==paused,"paused automatic advance rejected before controller");
    rejected=false;try{clock.resume(0);}catch(const std::runtime_error&){rejected=true;}check(rejected&&clock.paused()&&clock.pause_revision()==1,"stale campaign resume rejected");
    auto day=clock.view().realm.day;auto campaign_revision=clock.revision();clock.step_one_day(1);check(clock.view().realm.day==day+1&&clock.revision()==campaign_revision+1&&clock.paused()&&clock.pause_revision()==1,"manual campaign day while paused runs once");paused=clock.fingerprint();campaign_revision=clock.revision();rejected=false;try{clock.step_one_day(0);}catch(const std::runtime_error&){rejected=true;}check(rejected&&clock.fingerprint()==paused&&clock.revision()==campaign_revision,"stale manual campaign day atomic");
    clock.resume(1);check(!clock.paused()&&clock.pause_revision()==2,"campaign resume advances revision");day=clock.view().realm.day;campaign_revision=clock.revision();clock.step_one_day();check(clock.view().realm.day==day+1&&clock.revision()==campaign_revision+1&&!clock.paused()&&clock.pause_revision()==2,"manual campaign day preserves unpaused state");
    {
        Campaign relief(37,512);
        advance(relief,2);
        auto before_relief=relief.view();
        auto shortage=std::find_if(before_relief.cases.begin(),before_relief.cases.end(),[](const Case& item){return item.region==1&&item.closed<0;});
        check(shortage!=before_relief.cases.end(),"reported food-and-livelihood case becomes visible after two game days");
        check(command(relief,CampaignAction::Relief,1,500).success,"funded relief is accepted for the reported shortage region");
        auto dispatched=relief.view();
        const int arrival=dispatched.cargo.back().arrives;
        check(arrival>dispatched.realm.day&&!dispatched.cargo.back().delivered,"relief is bonded in transit until its route arrival day");
        Campaign restored_relief(1,128);std::string relief_error;
        check(restored_relief.deserialize(relief.serialize(),relief_error)&&restored_relief.fingerprint()==relief.fingerprint(),"save and load preserve the open case and in-transit relief");
        advance(relief,arrival-dispatched.realm.day);
        advance(restored_relief,arrival-dispatched.realm.day);
        advance(relief,2);
        advance(restored_relief,2);
        auto delivered=relief.view();
        auto resolved=std::find_if(delivered.cases.begin(),delivered.cases.end(),[](const Case& item){return item.region==1;});
        const bool relief_recorded=std::any_of(delivered.journal.begin(),delivered.journal.end(),[](const Fact& fact){return fact.kind==202&&fact.region==1&&fact.amount==500;});
        check(delivered.cargo.back().delivered&&relief_recorded,"arrival records transfer of all 500 grain units into the province");
        check(resolved!=delivered.cases.end()&&resolved->closed==arrival,"food-and-livelihood case closes when delivered grain restores the recorded stock threshold");
        check(relief.fingerprint()==restored_relief.fingerprint()&&relief.invariant()&&restored_relief.invariant(),"saved continuation matches uninterrupted relief through delivery and case closure");
    }
    auto before=w.fingerprint();check(!w.execute({2,1,CampaignAction::Relief,1,200}).success&&w.fingerprint()==before,"authority rejection atomic");check(!command(w,CampaignAction::Relief,9).success&&w.fingerprint()==before,"target rejection atomic");check(!command(w,CampaignAction::Relief,1,5001).success&&w.fingerprint()==before,"quantity rejection atomic");check(!command(w,CampaignAction::Arbitrate).success&&w.fingerprint()==before,"no evidence no ruling");
    auto slow=w.preview(6,1,200);check(slow.reachable&&slow.days==16&&slow.fee==345&&slow.regions==std::vector<int>{6,0,2,1},"original route search keeps the fastest reachable path under the toll delay");check(slow.leg_starts==std::vector<int>{0,2,5}&&slow.leg_ends==std::vector<int>{0,2,7}&&slow.leg_arrivals==std::vector<int>{2,5,16},"original route preview preserves daily shared-capacity slots and arrival days");
    check(command(w,CampaignAction::Road).success,"funded road order");advance(w,15);auto stalled=w.view();check(stalled.routes[0].works_due>stalled.realm.day,"unsettled toll right stalls project");check(!stalled.cases.empty(),"case emerges from resources and blocked rights");
    check(command(w,CampaignAction::Negotiate).success,"compensated toll agreement");advance(w,1);auto fast=w.preview(6,1,200);check(fast.reachable&&fast.days==7&&fast.fee==165&&fast.regions==std::vector<int>{6,0,2,1}&&w.view().routes[0].capacity>80,"removing the toll delay and increasing road capacity preserves the expected faster route");
    auto seq=w.view().player_sequence;check(command(w,CampaignAction::Relief,1,200).success,"dispatch funded cargo");auto shipped=w.view();check(!shipped.cargo.back().delivered&&shipped.realm.money==opening.realm.money&&shipped.invariant,"goods are conserved while in transit");before=w.fingerprint();check(!w.execute({1,seq,CampaignAction::Relief,1,200}).success&&w.fingerprint()==before,"duplicate command rejected");
    {
        // Exercise the shared Realm production transaction while Campaign
        // cargo remains bonded in transit, then validate the full wrapper save.
        Campaign bonded_campaign(37,256);
        check(command(bonded_campaign,CampaignAction::Relief,1,200).success,"production bond fixture dispatch");
        const std::string campaign_data=bonded_campaign.serialize();
        std::istringstream campaign_input(campaign_data);std::string magic;campaign_input>>magic;
        detail::CampaignStatePayload payload;detail::read_campaign_state(campaign_input,payload);
        std::string realm_data;campaign_input>>std::quoted(realm_data);
        check(bool(campaign_input)&&payload.cargo.size()==1&&!payload.cargo[0].delivered,
              "production bond fixture has undelivered cargo");
        Realm world(1,1,128);std::string realm_error;
        check(world.deserialize(realm_data,"m4-controller-v1",realm_error,payload.bonded_goods),realm_error.c_str());
        const uint64_t worker=129;const int region=world.person(worker).region;
        auto created=world.execute(EstablishmentCommand{worker,world.person(worker).decisions+1,
            EstablishmentOwnerKind::Person,worker,region,EstablishmentIndustry::ClothWorkshop,1,1});
        check(created.success,"production bond fixture establishment");
        check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
            ScheduleCommand::Operation::Submit,0,0,16,0,1000,ActivityKind::Work,region,created.session}).success,
            "production bond fixture Work schedule");
        std::vector<std::string> tokens;{
            std::istringstream in(world.serialize("m4-controller-v1"));std::string token;while(in>>token)tokens.push_back(token);
        }
        const auto sectionIt=std::find_if(tokens.begin(),tokens.end(),[](const std::string& token){return token.rfind("ESTABLISH_",0)==0;});
        const size_t section=size_t(sectionIt-tokens.begin());
        check(section<tokens.size(),"production bond fixture save section");
        if(section<tokens.size()){
            const size_t stock=section+2+7+size_t(GoodId::Fiber);
            if(stock<tokens.size()&&18+size_t(GoodId::Fiber)<tokens.size()){
                tokens[stock]="1";tokens[18+size_t(GoodId::Fiber)]=std::to_string(std::stoll(tokens[18+size_t(GoodId::Fiber)])+1);
            }else check(false,"production bond fixture token bounds");
        }
        std::ostringstream patched;for(size_t i=0;i<tokens.size();++i){if(i)patched<<' ';patched<<tokens[i];}
        Realm ready(1,1,128);
        check(ready.deserialize(patched.str(),"m4-controller-v1",realm_error,payload.bonded_goods),realm_error.c_str());
        ready.advance_ticks(64);
        check(ready.establishments()[0].stock[size_t(GoodId::Cloth)]==1&&
              ready.invariant(payload.bonded_goods),"production conserves with bonded campaign goods");
        std::ostringstream composed;composed<<"POLYEC_M4_1 ";detail::write_campaign_state(composed,payload);
        composed<<std::quoted(ready.serialize("m4-controller-v1"));
        Campaign restored_bonded(1,128);
        check(restored_bonded.deserialize(composed.str(),realm_error)&&restored_bonded.invariant()&&
              !restored_bonded.view().cargo[0].delivered,
              "Campaign accepts production save while cargo remains bonded");
    }
    std::string data=w.serialize(),error;Campaign restored(1,128);check(restored.deserialize(data,error),"restore cargo rights and case state");check(restored.fingerprint()==w.fingerprint(),"exact replay snapshot");before=restored.fingerprint();check(!restored.deserialize(data.substr(0,data.size()/2),error)&&restored.fingerprint()==before,"truncated save rejection atomic");check(!restored.deserialize(data+" junk",error)&&restored.fingerprint()==before,"trailing data rejected");
    advance(w,fast.days);advance(restored,fast.days);check(w.view().cargo.back().delivered,"cargo arrives after game-time duration");check(w.fingerprint()==restored.fingerprint(),"controller and RNG resume deterministically");
    check(command(w,CampaignAction::Refinance).success,"creditor actually paid");check(w.view().loans.front().paid==208,"debt principal and interest cleared");
    before=w.fingerprint();check(!command(w,CampaignAction::Relief,1,5000).success&&w.fingerprint()==before,"cannot ship more than finite remaining grain");advance(w,30);bool closed=false;for(auto&k:w.view().cases)closed|=k.region==1&&k.closed>=0;check(closed,"player can close a complete case");
    Campaign legal(37,256);check(command(legal,CampaignAction::Survey).success,"funded survey");before=legal.fingerprint();check(!command(legal,CampaignAction::Arbitrate).success&&legal.fingerprint()==before,"investigation does not reveal evidence immediately");advance(legal,3);check(command(legal,CampaignAction::Arbitrate).success,"evidence enables alternate resolution");check(legal.view().realm.provinces[1].unrest>=10,"ruling has political cost");
    Campaign workshop(37,256);auto money=workshop.view().realm.money;check(command(workshop,CampaignAction::Weave,2).success,"real adults and tools required for workshop");before=workshop.fingerprint();check(!command(workshop,CampaignAction::Weave,2).success&&workshop.fingerprint()==before,"no duplicate recruitment");advance(workshop,14);check(workshop.view().cloth>0&&workshop.view().workshop_workers>0,"fiber fuel labor become cloth");check(workshop.view().realm.money==money&&workshop.invariant(),"workshop income and wages conserve money");
    Campaign traffic(37,256);int clear_days=traffic.preview(6,1,100).days;check(command(traffic,CampaignAction::Relief,1,100).success,"first route reservation");int arrival=traffic.view().cargo.back().arrives;check(command(traffic,CampaignAction::Relief,1,100).success,"second route reservation");check(traffic.view().cargo.back().arrives>arrival&&traffic.invariant(),"split shipments cannot exceed shared road capacity");data=traffic.serialize();check(restored.deserialize(data,error)&&restored.fingerprint()==traffic.fingerprint(),"reservations survive save restore");advance(traffic,40);check(traffic.preview(6,1,100).days==clear_days,"expired slots do not delay future cargo");
    auto path=(std::filesystem::temp_directory_path()/"polyec-m4-test.polyec").string();check(workshop.save(path,error),"save writes");check(workshop.save(path,error),"save replaces with backup on Windows");check(std::filesystem::exists(path+".bak"),"backup retained");Campaign headless_restore(1,128);check(headless_restore.load(path,error)&&!headless_restore.paused()&&headless_restore.fingerprint()==workshop.fingerprint(),"headless restore preserves saved pause state");Campaign interactive_restore(1,128);check(interactive_restore.load(path,error,true)&&interactive_restore.paused()&&interactive_restore.pause_revision()==workshop.pause_revision()+1&&interactive_restore.view().realm.day==workshop.view().realm.day,"interactive restore stages paused world");data=interactive_restore.serialize();Campaign paused_headless(1,128);check(paused_headless.deserialize(data,error)&&paused_headless.paused()&&paused_headless.fingerprint()==interactive_restore.fingerprint(),"headless restore preserves paused save");before=restored.fingerprint();{std::ofstream f(path,std::ios::app);f<<'x';}check(!restored.load(path,error)&&restored.fingerprint()==before,"corrupt checksum leaves live world intact");Campaign staged_restore(1,128);staged_restore.pause();auto staged=staged_restore.fingerprint();check(!staged_restore.load(path,error,true)&&staged_restore.fingerprint()==staged&&staged_restore.paused(),"failed interactive restore preserves paused live world");std::filesystem::remove(path);std::filesystem::remove(path+".bak");
    for(int seed=1;seed<=6;++seed){Campaign stress(seed,256);command(stress,CampaignAction::Weave,2);for(int month=0;month<24;++month){advance(stress,30);check(stress.invariant(),"multi-seed weather debt lifecycle ledger");}auto v=stress.view();check(v.realm.alive>0,"stress population survives");bool harvest=false;for(auto&f:v.journal)harvest|=f.kind==201;check(harvest,"weather reduction emerges from seeded harvest");}
    auto start=std::chrono::steady_clock::now();Campaign profile(37,10000);command(profile,CampaignAction::Negotiate);command(profile,CampaignAction::Road);advance(profile,12);command(profile,CampaignAction::Refinance);check(command(profile,CampaignAction::Relief,1,5000).success,"10k funded relief workload");command(profile,CampaignAction::Weave,2);advance(profile,90);check(profile.invariant(),"10k full workload");closed=false;for(auto&k:profile.view().cases)closed|=k.region==1&&k.closed>=0;check(closed,"complete case also closes with 10000 people");std::cout<<"M4 workload 10k/102 days: "<<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()<<" ms\n";
    // R4 campaign-state codec fixtures (docs/design/R4-campaign-state-codec-plan.md).
    {
        Campaign f(37,256);
        check(command(f,CampaignAction::Weave,2).success,"codec fixture funded weave");
        check(command(f,CampaignAction::Relief,1,200).success,"codec fixture funded relief");
        check(f.view().cargo.size()==1,"codec fixture single cargo record");
        check(f.view().workshop_workers==6,"codec fixture recruited workshop crew");
        const std::string& gold=r4_golden::value;
        check(gold.size()==622,"golden payload literal is 622 bytes");
        const std::string full=f.serialize();
        check(full.compare(0,12,"POLYEC_M4_1 ")==0,"fixture save keeps magic prefix");
        detail::CampaignStatePayload staged;std::istringstream in(full.substr(12));
        detail::read_campaign_state(in,staged);
        std::ostringstream emit;detail::write_campaign_state(emit,staged);
        check(emit.str()==gold,"codec writer reproduces golden payload bytes");
        std::istringstream whole(full.substr(12));detail::CampaignStatePayload audit;
        detail::read_campaign_state(whole,audit);
        const size_t payload_end=12+size_t(whole.tellg())+1;// reader sits before the one trailing separator
        check(payload_end>=12+gold.size(),"reader left stream before quoted Realm token");
        const std::string slice=full.substr(12,payload_end-12);
        check(slice==gold,"parser-selected payload slice matches golden bytes");
        const std::string realm_tail=full.substr(payload_end);
        std::istringstream ts(realm_tail);std::string realm_tok;ts>>std::quoted(realm_tok);ts>>std::ws;
        size_t realm_tokens=0;for(size_t p=realm_tok.find("POLYEC_REALM_28");p!=std::string::npos;p=realm_tok.find("POLYEC_REALM_28",p+1))++realm_tokens;
        check(bool(ts)&&ts.eof()&&realm_tokens==1&&realm_tok.compare(0,15,"POLYEC_REALM_28")==0,"payload tail holds exactly one Realm token");
        {std::istringstream tok(gold);uint64_t seed=0,rev=0,seq=0,nxt=0;int insp=0;int64_t cloth=0;bool surveyed=false;
         tok>>seed>>rev>>seq>>nxt>>insp>>surveyed>>cloth;
         check(staged.seed==seed&&staged.revision==rev&&staged.sequence==seq&&staged.next_record==nxt&&staged.inspecting_until==insp&&staged.surveyed==surveyed&&staged.workshop_output==cloth,"staged payload scalars match golden tokens");}
        auto rv=f.view();
        auto route_eq=[&](const MapRoute&a,const MapRoute&b){return a.from==b.from&&a.to==b.to&&a.days==b.days&&a.capacity==b.capacity&&a.terrain==b.terrain&&a.open==b.open&&a.claimed==b.claimed&&a.works_due==b.works_due;};
        auto cargo_eq=[&](const Cargo&a,const Cargo&b){return a.id==b.id&&a.cause==b.cause&&a.from==b.from&&a.to==b.to&&a.good==b.good&&a.depart==b.depart&&a.arrives==b.arrives&&a.quantity==b.quantity&&a.cost==b.cost&&a.delivered==b.delivered&&a.path==b.path&&a.leg_starts==b.leg_starts&&a.leg_ends==b.leg_ends&&a.leg_arrivals==b.leg_arrivals;};
        bool routes_match=staged.routes.size()==rv.routes.size();for(size_t i=0;routes_match&&i<rv.routes.size();++i)routes_match=route_eq(staged.routes[i],rv.routes[i]);
        bool cargo_match=staged.cargo.size()==rv.cargo.size();for(size_t i=0;cargo_match&&i<rv.cargo.size();++i)cargo_match=cargo_eq(staged.cargo[i],rv.cargo[i]);
        check(routes_match,"staged routes match canonical fixture view");
        check(cargo_match,"staged cargo matches canonical fixture view");
        check(staged.workers.size()==size_t(rv.workshop_workers),"staged workers match canonical fixture view");
        check(staged.cases.size()>=rv.cases.size()&&staged.journal.size()>=rv.journal.size(),"staged journal superset of day-filtered view");
        std::array<int64_t,kGoodIdCount> manual{};for(auto&c:staged.cargo)if(!c.delivered)manual.at(size_t(canonical_from_legacy_good(c.good)))+=c.quantity;
        check(staged.bonded_goods==manual,"bonded totals equal manual undelivered-cargo sum");
        check(!staged.cargo[0].delivered&&staged.bonded_goods.at(size_t(canonical_from_legacy_good(staged.cargo[0].good)))==staged.cargo[0].quantity,"bonded slot equals fixture cargo quantity");
        // Item 3: malformed/truncated saves through Campaign::deserialize - exact
        // error strings and unchanged destinations (including a pre-paused one).
        Campaign dst;const uint64_t dst_fp=dst.fingerprint();
        Campaign paused_dst;paused_dst.pause();const uint64_t paused_fp=paused_dst.fingerprint();
        const std::string magic="POLYEC_M4_1 ",head="37 2 3 13 0 0 0 ";
        auto reject=[&](const std::string&data,const char*expected,const char*label){
            check(!dst.deserialize(data,error)&&error==expected&&dst.fingerprint()==dst_fp,label);
            const std::string paused_label=std::string(label)+" (paused destination)";
            check(!paused_dst.deserialize(data,error)&&error==expected&&paused_dst.paused()&&paused_dst.pause_revision()==1&&paused_dst.fingerprint()==paused_fp,paused_label.c_str());
        };
        reject(magic+head+"10 ","M4 save record bound","route count over bound rejected atomically");
        reject(magic+head+"0 100001 ","M4 save record bound","cargo count over bound rejected atomically");
        reject(magic+head+"0 0 100001 ","M4 save record bound","case count over bound rejected atomically");
        reject(magic+head+"0 0 0 1000001 ","M4 save record bound","journal count over bound rejected atomically");
        reject(magic+head+"0 0 0 0 7 ","M4 save record bound","worker count over bound rejected atomically");
        reject(magic+"37 2 3","M4 save record bound","truncated scalar tail surfaces at next count");
        reject(magic+head+"1 1 2 5 80","M4 save record bound","truncated route record surfaces at next count");
        reject(magic+head+"0 1 11 12 6 1 0 0 16","M4 save record bound","truncated cargo record surfaces at next count");
        reject(magic+head+"0 1 11 12 6 1 0 0 16 200 4545 0 0 ","M4 path missing","cargo path count zero rejected atomically");
        reject(magic+head+"0 1 11 12 6 1 0 0 16 200 4545 0 1 6 ","M4 path missing","cargo path count one rejected atomically");
        reject(magic+head+"0 1 11 12 6 1 7 0 16 200 4545 0 2 6 1 0 0 1 ","M4 manifest invalid","cargo good out of range rejected atomically");
        reject(magic+head+"0 1 11 12 6 1 0 0 16 0 4545 0 2 6 1 0 0 1 ","M4 manifest invalid","cargo quantity zero rejected atomically");
        reject(magic+head+"0 0 0 0 2 23","M4 save incomplete","truncated worker tail surfaces at quoted Realm read");
        // Item 4: spliced Realm suffix variants - codec errors outrank Realm suffix parsing.
        const std::string quoted_realm=full.substr(payload_end);
        auto splice=[&](const std::string&payload_bytes,const std::string&suffix){std::ostringstream input;input<<"POLYEC_M4_1 "<<payload_bytes<<suffix;return input.str();};
        const std::string input_a=splice(emit.str(),"\"not the realm serialization\"");
        check(!dst.deserialize(input_a,error)&&dst.fingerprint()==dst_fp&&error.compare(0,std::string("M4 world/accounting invalid").size(),"M4 world/accounting invalid")==0,"spliced alternate Realm body rejected by world validation");
        check(!paused_dst.deserialize(input_a,error)&&paused_dst.paused()&&paused_dst.pause_revision()==1&&paused_dst.fingerprint()==paused_fp,"spliced alternate Realm body preserves paused destination");
        const std::string input_b=splice(emit.str(),"");
        check(!dst.deserialize(input_b,error)&&error=="M4 save incomplete"&&dst.fingerprint()==dst_fp,"spliced missing Realm quote surfaces as incomplete");
        check(!paused_dst.deserialize(input_b,error)&&error=="M4 save incomplete"&&paused_dst.paused()&&paused_dst.pause_revision()==1&&paused_dst.fingerprint()==paused_fp,"missing Realm quote preserves paused destination");
        detail::CampaignStatePayload variant=staged;variant.cargo[0].path.clear();variant.cargo[0].leg_starts.clear();variant.cargo[0].leg_ends.clear();variant.cargo[0].leg_arrivals.clear();
        std::ostringstream ve;detail::write_campaign_state(ve,variant);
        const std::string input_c=splice(ve.str(),quoted_realm);
        check(!dst.deserialize(input_c,error)&&error=="M4 path missing"&&dst.fingerprint()==dst_fp,"codec path error outranks valid Realm suffix");
        check(!paused_dst.deserialize(input_c,error)&&error=="M4 path missing"&&paused_dst.paused()&&paused_dst.pause_revision()==1&&paused_dst.fingerprint()==paused_fp,"codec path error preserves paused destination");
    }
    std::cout<<"CAMPAIGN: "<<checks-failures<<'/'<<checks<<" checks passed\n";return failures?1:0;
}catch(const std::exception&e){std::cerr<<"EXCEPTION: "<<e.what()<<'\n';return 1;}}






