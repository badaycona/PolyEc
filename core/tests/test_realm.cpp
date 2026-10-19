#include "polyec/realm.hpp"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <sstream>
using namespace polyec;
int checks=0;
void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
int main(){try{
    Realm w(37,6,10000);check(w.invariant(),"opening invariant");check(w.count()==10000,"real persons");check(w.relations().size()>19000,"sparse family/social graph");
    auto before=w.fingerprint();auto r=w.execute({1,1,{Intent::Tax,2,0,20}});check(!r.success&&w.fingerprint()==before,"unauthorized tax is atomic");
    check(w.execute({1,1,{Intent::Tax,w.person(1).region,0,20}}).success,"charter local tax");before=w.fingerprint();check(!w.execute({1,1,{Intent::Tax,w.person(1).region,0,5}}).success&&w.fingerprint()==before,"duplicate command");
    uint64_t merchant=65;int64_t wealth=w.person(merchant).cash-w.debt(merchant);check(w.execute({merchant,1,{Intent::Borrow,w.person(merchant).region,0,100,180}}).success,"funded loan");check(w.person(merchant).cash-w.debt(merchant)==wealth-4,"loan not income with interest");
    check(w.execute({merchant,2,{Intent::Repay,w.person(merchant).region,0,100}}).success,"repay");check(w.invariant(),"loan invariant");
    check(w.execute({1,2,{Intent::Project,w.person(1).region,0,200,60}}).success,"project funded and materials");w.advance(65);check(w.projects()[0].completed&&w.view().provinces[w.person(1).region].road==1,"project duration and stat");
    std::string data=w.serialize("test"),error;Realm restored(2,1,128);check(restored.deserialize(data,"test",error),"save load");check(restored.fingerprint()==w.fingerprint(),"save exact state RNG");before=restored.fingerprint();check(!restored.deserialize(data,"wrong",error)&&restored.fingerprint()==before,"wrong bundle atomic");check(!restored.deserialize(data.substr(0,data.size()/2),"test",error)&&restored.fingerprint()==before,"truncated save atomic");
    w.advance(100);restored.advance(100);check(w.fingerprint()==restored.fingerprint(),"continued replay");
    for(int seed=0;seed<60;++seed){Realm draft(seed,6,128);auto d=draft.view().draft;check(d.cards[0]!=d.cards[1]&&d.cards[1]!=d.cards[2]&&d.cards[0]!=d.cards[2],"three distinct cards");check(draft.reroll(1),"one reroll");check(!draft.reroll(0),"no second reroll");int card=draft.view().draft.cards[0];check(draft.choose(card),"choose offered");before=draft.fingerprint();check(!draft.choose(card)&&before==draft.fingerprint(),"endowment once");check(draft.invariant(),"augment invariant");}
    std::mt19937_64 rng(73);
    Realm hungry=Realm::scenario(37,129,30,50,0,100,30,30,0,50);check(hungry.execute({129,1,{Intent::Rest,hungry.person(129).region,0,2,7}}).success,"rest remains a legal intent");check(hungry.person(129).health==50,"rest cannot replace food when starving");Realm fed=Realm::scenario(37,129,30,50,0,100,30,30,100,50);check(fed.execute({129,1,{Intent::Rest,fed.person(129).region,0,2,7}}).success&&fed.person(129).health==65,"fed recovery uses the same action");
    Realm salary=Realm::scenario(37,1,35,50,0,0,0,0);auto salary_money=salary.total_money();salary.advance(29);check(salary.person(1).cash==0,"salary not minted before due date");salary.advance();check(salary.person(1).cash>0&&salary.total_money()==salary_money,"funded appointed role income");
    Realm roadcap(73,6,128);for(int level=0;level<4;++level){check(roadcap.execute({1,roadcap.person(1).decisions+1,{Intent::Project,roadcap.person(1).region,0,200,60}}).success,"fund road upgrade");roadcap.advance(65);}auto capped=roadcap.fingerprint();check(!roadcap.execute({1,roadcap.person(1).decisions+1,{Intent::Project,roadcap.person(1).region,0,200,60}}).success&&roadcap.fingerprint()==capped,"no infinite road reward beyond physical level cap");
    Realm authority(37,6,10000);auto authority_before=authority.fingerprint();check(!authority.execute({8,1,{Intent::Tax,authority.person(8).region,0,20}}).success&&authority.fingerprint()==authority_before,"advisor cannot change charter tax");check(authority.observation(1)[97]==1&&authority.observation(8)[97]==0,"public authority is observed");
    Realm rounding(37,6,256);for(int i=0;i<3;++i)check(rounding.execute({1,uint64_t(i+1),{Intent::Aid,0,0,1}}).success,"small funded relief");int beneficiaries=0;for(uint64_t id=15;id<=256;++id)if(rounding.person(id).cash==101)++beneficiaries;check(beneficiaries==3,"relief rounding does not always favor same ID");
    // Preserve prior Work -> advance 30 -> Study success -> job==-1 test.
    Realm employment=Realm::scenario(37,129,30);int home=employment.person(129).region;check(employment.execute({129,1,{Intent::Work,home,0,0,30}}).success,"start employment");check(employment.job_type(129)==JobType::Worker,"worker classification after start");check(employment.employment_state(129)==EmploymentState::Employed,"worker is Employed");{const Job* jd=employment.job_descriptor(129);check(jd&&jd->is_work_job(),"worker is work-job");check(jd&&jd->type()==JobType::Worker,"worker descriptor type");check(jd&&jd->name()=="Worker","worker descriptor name");}employment.advance(30);check(employment.execute({129,2,{Intent::Study,home,0,5,30}}).success,"study after contract ends");check(employment.person(129).job==-1,"study cannot renew an expired wage contract");check(employment.job_type(129)==JobType::None,"none after expiry");check(employment.employment_state(129)==EmploymentState::Idle,"Idle after expiry");check(employment.job_descriptor(129)==nullptr,"expired descriptor is nullptr");
    // Pure resolver tests on constructed Person fixtures (no serializer surgery).
    {Person w;w.alive=true;w.job=0;w.job_kind=0;w.busy_until=10;auto d=describe_job(w,5);check(d&&d->type()==JobType::Worker&&d->is_work_job(),"resolver worker");
     Person t;t.alive=true;t.job=1;t.job_kind=1;t.busy_until=10;auto dt=describe_job(t,5);check(dt&&dt->type()==JobType::Teacher&&dt->is_work_job(),"resolver teacher");
     Person o;o.alive=true;o.office=true;o.hired_by=1;o.family=Family::Governance;auto dof=describe_job(o,5);check(dof&&dof->type()==JobType::AppointedOffice&&!dof->is_work_job(),"resolver appointed office");
     Person idle;idle.alive=true;idle.job=-1;idle.job_kind=0;idle.busy_until=0;check(describe_job(idle,5)==nullptr,"resolver idle");
     Person expired;expired.alive=true;expired.job=0;expired.job_kind=0;expired.busy_until=4;check(describe_job(expired,5)==nullptr,"resolver expired");
     Person dead;dead.alive=false;dead.job=0;dead.job_kind=0;dead.busy_until=10;check(describe_job(dead,5)==nullptr,"resolver deceased");
     Person ik;ik.alive=true;ik.job=0;ik.job_kind=2;ik.busy_until=10;check(describe_job(ik,5)==nullptr,"resolver invalid job_kind");
     Person io;io.alive=true;io.office=true;io.hired_by=0;io.family=Family::Governance;io.job=0;io.job_kind=0;io.busy_until=10;auto dio=describe_job(io,5);check(dio&&dio->type()==JobType::Worker,"resolver invalid office falls through to worker");}
    // Teacher classification: pure resolver fixture; job_kind must be exactly 1.
    {Person tw;tw.alive=true;tw.job=1;tw.job_kind=1;tw.busy_until=10;check(describe_job(tw,5)&&describe_job(tw,5)->type()==JobType::Teacher,"teacher fixture job_kind==1");}
    // Deterministic Appointed Office via Court fixture: find seed whose draft contains card 6.
    // Mirrors the court fixture in test_realm_court.cpp: Realm(seed,6,256), limit 2000, Appoint at candidates(1)[15].
    bool found=false;
    for(int seed=1;seed<2000&&!found;++seed){Realm tw(seed,6,256);auto d=tw.draft();if(std::find(d.cards.begin(),d.cards.end(),6)==d.cards.end())continue;check(tw.choose(6),"choose card 6");Realm rejected=tw;const int rejected_region=rejected.person(1).region;const int64_t relief=rejected.view().provinces[rejected_region].treasury-19;check(relief>0,"appointment rejection fixture can drain treasury");check(rejected.execute({1,1,{Intent::Aid,rejected_region,0,relief}}).success,"appointment rejection fixture drains treasury");const auto actor_before=rejected.person(1);const int employed_before=rejected.view().provinces[rejected_region].employed;const int teachers_before=rejected.view().provinces[rejected_region].teachers;auto reject_before=rejected.fingerprint();auto rejected_result=rejected.execute({1,rejected.person(1).decisions+1,rejected.candidates(1)[15]});check(!rejected_result.success&&rejected.fingerprint()==reject_before&&rejected.person(1).job==actor_before.job&&rejected.person(1).busy_until==actor_before.busy_until&&rejected.view().provinces[rejected_region].employed==employed_before&&rejected.view().provinces[rejected_region].teachers==teachers_before,"appointment legal rejection is atomic for actor counters and assignment");auto result=tw.execute({1,1,tw.candidates(1)[15]});check(result.success,"appoint succeeds");for(uint64_t id=1;id<=256;++id){const auto&p=tw.person(id);if(p.office&&p.hired_by!=0&&p.family==Family::Governance){check(tw.job_type(id)==JobType::AppointedOffice,"office classification");check(tw.employment_state(id)==EmploymentState::Employed,"office is Employed");{const Job* od=tw.job_descriptor(id);check(od&&!od->is_work_job(),"office is not work-job");check(od&&od->type()==JobType::AppointedOffice,"office descriptor type");}found=true;break;}}}
    check(found,"deterministic appointment found");
    Realm family=Realm::scenario(37,129,8,50,0,0);check(family.available_funds(129)>0,"funded guardian budget");auto initial_money=family.total_money();check(family.execute({129,1,{Intent::Study,family.person(129).region,0,5,30}}).success,"child can use authorized household school budget");check(family.total_money()==initial_money&&family.invariant(),"household transfer conservation");
    Realm guild=Realm::scenario(39,129,30);auto group_money=guild.total_money();check(guild.execute({129,1,{Intent::Join,guild.person(129).region,2,2,30}}).success,"join actual organization account");check(guild.person(129).organization>0&&guild.person(129).social==0,"membership does not farm social reward");check(guild.total_money()==group_money&&guild.invariant(),"organization funds conserved");
    Realm routes(37,6,256);check(routes.distance(0,1)==1,"open shortest route");auto route_before=routes.fingerprint();check(!routes.route_command(8,0,1,false).success&&routes.fingerprint()==route_before,"economic cannot close public route");check(routes.route_command(1,0,1,false).success,"route jurisdiction");check(routes.distance(0,1)==6,"route disruption forces detour");check(routes.route_command(1,0,6,false).success,"second closure");check(routes.distance(0,1)==-1,"isolated market unreachable");
    Realm childhood(5,6,256);uint64_t child=0;for(uint64_t id=129;id<=256;++id)if(childhood.person(id).age_days<14*365){child=id;break;}check(child>0,"child with unique ID");check(!childhood.mask(child)[1+childhood.person(child).region],"child labor restricted by age");auto cold=childhood.fingerprint();check(!childhood.execute({child,1,{Intent::Borrow,childhood.person(child).region,0,100,180}}).success&&childhood.fingerprint()==cold,"child loan rejected atomically");
    // M5 household membership is a dynamic record, not an ID..ID+3 window. Fixtures are
    // built by scalar surgery on the real versioned save text, never on private state.
    auto patch_save=[](const Realm& base,const std::vector<std::pair<uint64_t,int>>& fields,const std::vector<std::string>& values){
        std::vector<std::string> tokens;{std::istringstream in(base.serialize("m5fixture"));std::string token;while(in>>token)tokens.push_back(token);}
        const size_t people=base.count();const size_t begin=[&]{for(size_t j=0;j+1+36*people<=tokens.size();++j){if(tokens[j]!=std::to_string(people))continue;
            bool dense=true;for(size_t k=0;k<people;++k)if(tokens[j+1+36*k]!=std::to_string(k+1)){dense=false;break;}
            if(dense)return j+1;}throw std::runtime_error("person block not located in save");}();
        for(size_t i=0;i<fields.size();++i)tokens[begin+36*(fields[i].first-1)+size_t(fields[i].second)]=values[i];
        std::string out;for(size_t i=0;i<tokens.size();++i){if(i)out+=' ';out+=tokens[i];}return out;};
    auto load_fixture=[&](const std::string& text){Realm w(1,1,128);std::string err;if(!w.deserialize(text,"m5fixture",err))throw std::runtime_error("household fixture rejected: "+err);return w;};
    // A rejected command consumes no sequence, so every rejection below must be issued with
    // the actor's CURRENT decision count to reach household/authority validation at all.
    auto rejects=[&](Realm& world,uint64_t actor,const Candidate& action,const char* label){auto before=world.fingerprint();auto sequence=world.person(actor).decisions+1;auto result=world.execute({actor,sequence,action});check(!result.success&&world.fingerprint()==before&&result.reason!="Stale or duplicate decision ID",label);};
    auto find_startable=[&](Realm& world,uint64_t from,bool teacher_only){
        const int region=world.person(1).region;
        const Province province=world.view().provinces[region];
        if(province.employed>=province.jobs)return uint64_t(0);
        for(uint64_t id=from;id<=uint64_t(world.count());++id){
            const auto& p=world.person(id);
            if(!p.alive||p.job>=0||p.family==Family::Governance||p.region!=region||p.age_days<14*365)continue;
            if(teacher_only&&(p.age_days<18*365||p.skill<20))continue;
            return id;
        }
        return uint64_t(0);
    };
    // Work starts classify the same way as the production threshold, rebuild
    // counters daily, enforce capacity atomically, renew, expire inclusively and
    // survive save/load exactly.
    Realm work=Realm::scenario(37,129,30,50,0,0,0,1);
    const int work_region=work.person(129).region;
    uint64_t worker=0;
    for(uint64_t id=129;id<=uint64_t(work.count())&&!worker;++id){const auto&p=work.person(id);if(p.alive&&p.job<0&&p.family!=Family::Governance&&p.region==work_region&&p.age_days>=14*365)worker=id;}
    check(worker>0,"startable worker in a one-job region");
    auto work_before=work.fingerprint();
    check(!work.execute({worker,work.person(worker).decisions+1,{Intent::Work,work_region,0,0,31}}).success&&work.fingerprint()==work_before,"work term 31 rejected atomically");
    check(work.execute({worker,work.person(worker).decisions+1,{Intent::Work,work_region,0,0,30}}).success,"work starts a worker");
    check(work.person(worker).job==work_region&&work.person(worker).job_kind==0&&work.person(worker).busy_until==30,"worker assignment fields");
    check(work.job_type(worker)==JobType::Worker&&work.employment_state(worker)==EmploymentState::Employed,"worker classification");
    check(work.view().provinces[work_region].employed==1&&work.view().provinces[work_region].teachers==0,"worker start counters");
    uint64_t blocked=0;
    for(uint64_t id=15;id<=uint64_t(work.count())&&!blocked;++id){const auto&p=work.person(id);if(p.alive&&p.job<0&&p.family!=Family::Governance&&p.region==work_region&&p.age_days>=14*365)blocked=id;}
    check(blocked>0,"second eligible worker exists");
    auto full_before=work.fingerprint();
    check(!work.execute({blocked,work.person(blocked).decisions+1,{Intent::Work,work_region,0,0,30}}).success&&work.fingerprint()==full_before,"full worker capacity rejects atomically");
    work.advance(30);
    check(work.person(worker).job==work_region&&work.view().provinces[work_region].employed==1,"daily rebuild restores worker counters at the contract boundary");
    check(work.execute({worker,work.person(worker).decisions+1,{Intent::Work,work_region,0,0,30}}).success,"worker renews at full capacity after the contract day");
    check(work.person(worker).busy_until==60&&work.view().provinces[work_region].employed==1,"renewal keeps one accounted worker");
    work.advance(1);
    check(work.person(worker).job==work_region&&work.view().provinces[work_region].employed==1,"daily rebuild restores renewed worker counters");
    std::string work_data=work.serialize("employment"),work_error;
    Realm work_loaded(1,1,128);
    check(work_loaded.deserialize(work_data,"employment",work_error),"employment save parses");
    check(work_loaded.fingerprint()==work.fingerprint(),"employment save is exact");
    work.advance(30);
    work_loaded.advance(30);
    check(work.person(worker).job==-1&&work.job_type(worker)==JobType::None&&work.employment_state(worker)==EmploymentState::Idle,"work expiry clears the assignment");
    check(work.view().provinces[work_region].employed==0&&work.view().provinces[work_region].teachers==0,"expiry rebuilds zero counters");
    check(work.fingerprint()==work_loaded.fingerprint(),"employment continuation is exact");
    Realm underage(5,6,256);
    uint64_t underage_child=0;
    for(uint64_t id=129;id<=uint64_t(underage.count());++id)if(underage.person(id).age_days<14*365){underage_child=id;break;}
    check(underage_child>0,"underage worker fixture");
    auto child_before=underage.fingerprint();
    check(!underage.execute({underage_child,underage.person(underage_child).decisions+1,{Intent::Work,underage.person(underage_child).region,0,0,30}}).success&&underage.fingerprint()==child_before,"underage work rejected atomically");
    Realm teacher_world(37,6,256);
    const int teacher_region=teacher_world.person(1).region;
    uint64_t teacher=find_startable(teacher_world,15,true);
    check(teacher>0,"startable teacher candidate");
    check(teacher_world.execute({teacher,teacher_world.person(teacher).decisions+1,{Intent::Work,teacher_region,0,0,30}}).success,"work starts a teacher");
    check(teacher_world.person(teacher).job_kind==1&&teacher_world.job_type(teacher)==JobType::Teacher,"teacher classification after start");
    check(teacher_world.view().provinces[teacher_region].teachers==1,"teacher counter after start");
    teacher_world.advance(1);
    check(teacher_world.view().provinces[teacher_region].teachers==1&&teacher_world.view().provinces[teacher_region].teacher_skill_sum==teacher_world.person(teacher).skill,"teacher counters rebuild daily");
    Realm sparse_base(37,6,256);auto sparse_base_before=sparse_base.fingerprint();
    // A birth joins the parent household with a fresh non-consecutive ID. Governance and
    // economic founders are singleton households, so pick a newborn that actually shares a
    // multi-member household with a living adult.
    Realm births(37,6,10000);const int opening_population=births.count();
    for(int day=0;day<4000&&births.count()==opening_population;++day)births.advance();
    check(births.count()>opening_population,"birth extends a household with a fresh ID");
    uint64_t newborn=0,parent=0;
    for(uint64_t id=opening_population+1;id<=uint64_t(births.count())&&!newborn;++id){
        if(births.guardian(id)==0)continue;
        for(auto&e:births.relations())if(e.from==id&&e.kind==0){parent=e.to;break;}
        if(parent&&births.same_household(id,parent)&&births.household_of(id)==births.household_of(parent))newborn=id;}
    check(newborn>0&&parent>0,"newborn in a shared household with a living adult");
    auto members=births.household_members(births.household_of(newborn));
    check(members.size()>1&&std::find(members.begin(),members.end(),newborn)!=members.end()&&std::is_sorted(members.begin(),members.end()),"birth membership is real, not an ID window");
    check(members.back()-members.front()>3&&births.guardian(newborn)!=0&&births.same_household(newborn,births.guardian(newborn)),"non-consecutive member is reachable as guardian");
    std::string household_data=births.serialize("m5"),household_error;Realm reloaded(3,2,128);
    check(reloaded.deserialize(household_data,"m5",household_error),"household birth save loads");
    check(reloaded.fingerprint()==births.fingerprint()&&reloaded.household_members(births.household_of(newborn))==members,"household membership survives load as derived state");
    check(reloaded.guardian(newborn)==births.guardian(newborn)&&reloaded.invariant()&&reloaded.household_records()>0,"guardian and invariant survive load");
    // Non-consecutive living adult, and every in-window adult dead: the old ID..ID+3 scan
    // could never find a guardian here.
    Realm sparse_world=load_fixture(patch_save(sparse_base,{{52,1},{15,14},{17,14},{18,14}},{"15","0","0","0"}));
    check(sparse_world.invariant()&&sparse_world.count()==256,"sparse household fixture invariant");
    auto sparse_members=sparse_world.household_members(15);
    check(sparse_members.size()==5&&std::find(sparse_members.begin(),sparse_members.end(),uint64_t(52))!=sparse_members.end(),"non-consecutive member recorded in the household");
    check(52>18&&sparse_world.guardian(16)==52&&sparse_world.guardian(52)==0,"guardian found outside any ID..ID+3 window");
    check(sparse_world.household_authority(15)==52&&sparse_world.available_funds(16)==sparse_world.person(16).cash+sparse_world.person(52).cash,"household authority and authorized budget");
    check(sparse_world.same_household(16,52)&&!sparse_world.same_household(16,60)&&!sparse_world.same_household(0,16),"same household authority is membership, not proximity");
    // Same-household authority: a living adult member receives the household transfer. The
    // expected authority is derived from the fixture, never assumed from an age guess.
    Realm authority_world=load_fixture(patch_save(sparse_base,{{52,1},{16,5},{15,14},{17,14},{18,14},{52,5}},{"15",std::to_string(30*365),"0","0","0",std::to_string(40*365)}));
    uint64_t oldest=0;for(uint64_t member:authority_world.household_members(15)){auto&p=authority_world.person(member);if(p.alive&&p.age_days>=18*365&&(oldest==0||p.age_days>authority_world.person(oldest).age_days))oldest=member;}
    check(oldest==52&&authority_world.household_authority(15)==oldest&&authority_world.candidates(16)[19].target==int(oldest)&&authority_world.mask(16)[19]==1.f,"adult household member offers support to the oldest living adult");
    int64_t household_money=authority_world.total_money();int64_t household_before=authority_world.person(52).cash,sender_before=authority_world.person(16).cash;
    check(authority_world.execute({16,1,{Intent::Support,int(oldest),0,5}}).success,"same household support commits");
    check(authority_world.total_money()==household_money&&authority_world.invariant(),"household support conserves money");
    auto support_before=authority_world.fingerprint();
    rejects(authority_world,16,{Intent::Support,60,0,5},"foreign household target rejected atomically");
    rejects(authority_world,16,{Intent::Support,16,0,5},"self target rejected atomically");
    rejects(authority_world,16,{Intent::Support,15,0,5},"deceased household member rejected atomically");
    rejects(authority_world,16,{Intent::Support,0,0,5},"zero target rejected before any person lookup");
    check(authority_world.person(oldest).cash==household_before+5&&authority_world.person(16).cash==sender_before-5,"only the intended household recipient gained funds");
    // Dead/missing guardian: no living adult in the household must not throw or mint money.
    Realm orphan_world=load_fixture(patch_save(sparse_base,{{15,14},{17,14},{18,14}},{"0","0","0"}));
    check(orphan_world.invariant()&&orphan_world.guardian(16)==0&&orphan_world.household_authority(15)==0,"household without a living adult has no authority");
    check(orphan_world.available_funds(16)==orphan_world.person(16).cash,"orphan budget is own cash only");
    auto orphan_before=orphan_world.fingerprint();orphan_world.mask(16);check(orphan_world.fingerprint()==orphan_before,"orphan household observation is safe");
    check(orphan_world.candidates(16)[19].kind==Intent::Hold,"orphan household offers no support target");
    rejects(orphan_world,16,{Intent::Support,0,0,5},"orphan support rejected without a person lookup");
    rejects(orphan_world,16,{Intent::Support,15,0,5},"dead authority rejected atomically");
    int64_t orphan_money=orphan_world.total_money();
    rejects(orphan_world,16,{Intent::Study,orphan_world.person(16).region,0,1000000,30},"unfunded orphan study rejected without a guardian transfer");
    check(orphan_world.execute({16,orphan_world.person(16).decisions+1,{Intent::Study,orphan_world.person(16).region,0,5,30}}).success,"orphan child studies from own cash");
    check(orphan_world.total_money()==orphan_money&&orphan_world.invariant(),"orphan study conserves money");
    // A stale sequence is rejected before gameplay rules, and must not be read as authority.
    auto stale_before=orphan_world.fingerprint();
    check(!orphan_world.execute({16,99,{Intent::Study,orphan_world.person(16).region,0,5,30}}).success&&orphan_world.fingerprint()==stale_before,"stale sequence rejected atomically");
    // Corrupt household records are rejected and leave the live world untouched.
    Realm corrupt(1,1,128);auto corrupt_before=corrupt.fingerprint();std::string corrupt_error;
    check(!corrupt.deserialize(patch_save(sparse_base,{{20,1}},{"0"}),"m5fixture",corrupt_error)&&corrupt.fingerprint()==corrupt_before,"zero household label rejected atomically");
    check(!corrupt.deserialize(patch_save(sparse_base,{{20,1}},{std::to_string(100000)}),"m5fixture",corrupt_error)&&corrupt.fingerprint()==corrupt_before,"household label beyond the person range rejected atomically");
    // The fixture source is the pre-serialization original; the loaded fixtures are separate worlds.
    check(sparse_base.invariant()&&sparse_base.count()==256&&sparse_base.fingerprint()==sparse_base_before,"fixture source world unchanged by fixture surgery");
    check(sparse_world.fingerprint()!=sparse_base_before&&orphan_world.fingerprint()!=sparse_base_before,"patched fixtures differ from their source world");

    for(int seed=1;seed<=8;++seed){Realm sim(seed,6,256);for(int week=0;week<1044;++week){for(auto family:{Family::Governance,Family::Economic,Family::Civilian})for(auto id:sim.due(family)){auto mask=sim.mask(id);std::vector<int> available;for(int i=0;i<32;++i)if(mask[i])available.push_back(i);int a=available[rng()%available.size()];check(sim.execute({id,sim.person(id).decisions+1,sim.candidates(id)[a]}).success,"legal random action committed");}sim.advance(7);check(sim.invariant(),"20year world accounting");}}
    auto start=std::chrono::steady_clock::now();for(int day=0;day<90;++day){for(auto id:w.due(Family::Civilian)){auto mask=w.mask(id);int a=mask[1+w.person(id).region]?1+w.person(id).region:0;w.execute({id,w.person(id).decisions+1,w.candidates(id)[a]});}w.advance();check(w.invariant(),"profile A accounting");}
    std::cout<<"REALM TESTS: "<<checks<<" checks passed; profile A10000 persons, "<<w.relations().size()<<" sparse edges;90days "<<std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()<<"s\n";return 0;
}catch(const std::exception&e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}
