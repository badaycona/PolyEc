#include "work_authorization_fixture.hpp"
#include "polyec/realm.hpp"
#include "polyec/schedule.hpp"
#include "polyec/establishment_section_codec.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <string>
#include <set>
#include <vector>
using namespace polyec;
int checks=0;
void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
uint64_t work_site(Realm& world,int region,uint64_t worker=129){
    for(const auto&e:world.establishments())if(e.region==region){test_fixtures::authorize_work(world,e.id,worker);return e.id;}
    const uint64_t governor=region==world.regions()?1:uint64_t(region+2);
    auto result=world.execute(EstablishmentCommand{governor,world.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(region),region,EstablishmentIndustry::ClothWorkshop,1,100});
    if(!result.success)throw std::runtime_error("test work establishment: "+result.reason);
    test_fixtures::authorize_work(world,result.session,worker);
    return result.session;
}
std::string downgrade_goods_to_v12(const std::string& data){
    std::istringstream latest(test_fixtures::save_as_v15(data));std::vector<std::string> tokens;std::string token;
    while(latest>>token)tokens.push_back(token);
    if(tokens.empty()||tokens[0]!="POLYEC_REALM_15"||tokens.size()<178)throw std::runtime_error("v15 goods fixture header");
    tokens.erase(std::find(tokens.begin(),tokens.end(),"CONFIG_1"),tokens.end());
    tokens[0]="POLYEC_REALM_13";
    const size_t production=std::find(tokens.begin(),tokens.end(),"ESTABLISH_3")-tokens.begin();
    if(production>=tokens.size())throw std::runtime_error("v14 establishment fixture section");
    tokens[production]="ESTABLISH_2";
    const size_t production_count=static_cast<size_t>(std::stoull(tokens.at(production+1)));
    size_t production_record=production+2;
    for(size_t row=0;row<production_count;++row){
        const size_t watermark_count=static_cast<size_t>(std::stoull(tokens.at(production_record+38)));
        const size_t record_end=production_record+39+2*watermark_count;
        tokens.erase(tokens.begin()+static_cast<std::ptrdiff_t>(production_record+34),
                     tokens.begin()+static_cast<std::ptrdiff_t>(record_end));
        production_record+=34;
    }
    std::ostringstream v13;for(size_t i=0;i<tokens.size();++i){if(i)v13<<' ';v13<<tokens[i];}
    std::istringstream input(v13.str());tokens.clear();
    while(input>>token)tokens.push_back(token);
    const std::array<size_t,7> legacy{{0,9,11,5,4,16,14}};
    auto shrink=[&](size_t start,size_t width){
        std::vector<std::string> values;values.reserve(7);
        for(size_t index:legacy)values.push_back(tokens.at(start+index));
        tokens.erase(tokens.begin()+static_cast<std::ptrdiff_t>(start),tokens.begin()+static_cast<std::ptrdiff_t>(start+width));
        tokens.insert(tokens.begin()+static_cast<std::ptrdiff_t>(start),values.begin(),values.end());
    };
    tokens[0]="POLYEC_REALM_12";
    // 9 primary scalars + 4 draft scalars + 3 draft cards.
    size_t at=18;for(int i=0;i<3;++i){shrink(at,27);at+=7;}
    // Removing 60 ledger tokens shifts every following section.
    at=0;
    for(size_t candidate=100;candidate<tokens.size();++candidate){
        size_t count=0;try{count=static_cast<size_t>(std::stoull(tokens[candidate]));}catch(...){continue;}
        if(count<128||count>1000000||candidate+1+56*count>tokens.size())continue;
        bool dense=true;for(size_t row=0;row<count;++row)if(tokens[candidate+1+56*row]!=std::to_string(row+1)){dense=false;break;}
        if(dense){at=candidate;break;}
    }
    if(at==0)throw std::runtime_error("v13 person block not found");
    const size_t people=static_cast<size_t>(std::stoull(tokens.at(at++)));
    for(size_t row=0;row<people;++row){const size_t start=at+row*36;std::vector<std::string> inventory;for(size_t index:legacy)inventory.push_back(tokens.at(start+29+index));tokens.erase(tokens.begin()+static_cast<std::ptrdiff_t>(start+29),tokens.begin()+static_cast<std::ptrdiff_t>(start+56));tokens.insert(tokens.begin()+static_cast<std::ptrdiff_t>(start+29),inventory.begin(),inventory.end());}
    at+=people*36;
    const size_t provinces=static_cast<size_t>(std::stoull(tokens.at(at++)));
    for(size_t row=0;row<provinces;++row){const size_t start=at+row*26;std::vector<std::string> fields;for(size_t i=0;i<12;++i)fields.push_back(tokens.at(start+i));for(size_t index:legacy)fields.push_back(tokens.at(start+12+index));for(size_t index:legacy)fields.push_back(tokens.at(start+39+index));tokens.erase(tokens.begin()+static_cast<std::ptrdiff_t>(start),tokens.begin()+static_cast<std::ptrdiff_t>(start+66));tokens.insert(tokens.begin()+static_cast<std::ptrdiff_t>(start),fields.begin(),fields.end());}
    const size_t establishment=std::find(tokens.begin(),tokens.end(),"ESTABLISH_2")-tokens.begin();
    if(establishment<tokens.size()){
        tokens[establishment]="ESTABLISH_1";const size_t count=static_cast<size_t>(std::stoull(tokens.at(establishment+1)));
        size_t start=establishment+2;const std::array<int,16> groupGood{{0,1,2,3,4,5,-1,9,11,14,16,-1,-1,-1,-1,-1}};
        for(size_t row=0;row<count;++row){const size_t record=start+row*23;std::vector<std::string> stocks;for(int good:groupGood)stocks.push_back(good<0?"0":tokens.at(record+7+size_t(good)));tokens.erase(tokens.begin()+static_cast<std::ptrdiff_t>(record+7),tokens.begin()+static_cast<std::ptrdiff_t>(record+34));tokens.insert(tokens.begin()+static_cast<std::ptrdiff_t>(record+7),stocks.begin(),stocks.end());}
    }
    std::ostringstream output;for(size_t i=0;i<tokens.size();++i){if(i)output<<' ';output<<tokens[i];}return output.str();
}
int main(){try{
    const uint64_t A=129;
    Realm w(37,6,256);
    // Frozen SCHEDULE_5 bytes for deterministic Work + Generic sessions,
    // one completed day, settlement and regional payroll watermarks.
    {
        Realm golden(7,2,128);
        const int region=golden.person(1).region;
        check(region==2,"codec golden work region");
        const auto goldenSite=work_site(golden,region,1);
        check(golden.execute(ScheduleCommand{1,golden.person(1).decisions+1,ScheduleCommand::Operation::Submit,
            0,0,64,0,5,ActivityKind::Work,region,goldenSite}).success,"codec golden Work submit");
        check(golden.execute(ScheduleCommand{2,1,ScheduleCommand::Operation::Submit,
            0,0,32,0,3}).success,"codec golden Generic submit");
        golden.advance_ticks(64);
        const std::string current=golden.serialize("schedule-codec-golden");
        const std::string serialized=test_fixtures::save_as_v28(current);
        const size_t section_at=serialized.find(" SCHEDULE_5 ");
        check(section_at!=std::string::npos,"codec golden section exists");
        std::string expectedSchedule=
            " SCHEDULE_5 1 64 3 1 2 1 1 1 0 1 1 1 0 64 0 5 320 320 0 0 0 0 0 0 1 2 129 2 2 0 32 0 3 96 96 0 0 0 0 0 0 0 -1 0 1 64 1 4 1 2 64 1 129 64 ";
        for(size_t at=0;(at=expectedSchedule.find(" 129",at))!=std::string::npos;at+=std::to_string(goldenSite).size())
            expectedSchedule.replace(at+1,3,std::to_string(goldenSite));
        const size_t establishmentAt=serialized.find(" ESTABLISH_5 ",section_at);
        check(serialized.substr(section_at,establishmentAt-section_at)==expectedSchedule,"codec golden SCHEDULE_5 bytes");
        const size_t currentEstablishmentAt=current.find(" ESTABLISH_14 ");
        std::ostringstream establishmentOracle;detail::write_establishment_section(establishmentOracle,golden.establishments());
        std::istringstream actualEstablishment(current.substr(currentEstablishmentAt,
            current.find(" CONFIG_3 ")-currentEstablishmentAt));
        std::istringstream expectedEstablishment(establishmentOracle.str());
        std::vector<std::string> actualEstablishmentFields,expectedEstablishmentFields;
        std::string field;
        while(actualEstablishment>>field)actualEstablishmentFields.push_back(field);
        while(expectedEstablishment>>field)expectedEstablishmentFields.push_back(field);
        check(actualEstablishmentFields==expectedEstablishmentFields,
              "schedule fixture establishment section follows its codec");
    }
    check(w.game_tick()==0,"initial cursor");
    check(!w.timeline_contract(),"legacy default");
    auto r=w.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000});
    check(r.success&&r.session>0&&r.fact>0,"submit receipt");
    check(w.game_tick()==0,"submit does not advance");
    check(w.timeline_contract(),"sticky");
    check(w.schedule_book().sessions.at(A).size()==1,"session stored");
    uint64_t firstId=r.session;
    auto r2=w.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,64,128,0,1000});
    check(r2.success,"overlap accepted");
    auto before=w.fingerprint();
    auto rs=w.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,128,192,0,1000});
    check(!rs.success&&w.fingerprint()==before,"stale atomic");
    auto before0=w.fingerprint();
    check(!w.execute(ScheduleCommand{0,3,ScheduleCommand::Operation::Submit,0,128,192,0,1000}).success&&w.fingerprint()==before0,"actor0 reject");

    // empty cancel leaves no canonical empty entry
    Realm ec(37,6,256);
    auto er=ec.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000});
    check(er.success,"empty-cancel submit");
    auto ecr=ec.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Cancel,er.session,0,0,0,0});
    check(ecr.success,"empty-cancel success");
    check(ec.schedule_book().sessions.find(A)==ec.schedule_book().sessions.end(),"no empty actor entry");
    check(ec.timeline_contract(),"sticky after cancel");
    check(ec.invariant(),"empty cancel invariant");
    std::string ecd=ec.serialize("test"),ece; Realm ecr2(2,1,128);
    check(ecr2.deserialize(ecd,"test",ece),"empty cancel save load");
    check(ecr2.schedule_book().sessions.find(A)==ecr2.schedule_book().sessions.end(),"empty cancel restored");

    // processed-time reject
    Realm w2(37,6,256);
    check(w2.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"submit2");
    w2.advance_ticks(32);
    auto beforePast=w2.fingerprint();
    uint64_t firstId2=w2.schedule_book().sessions.at(A).front().id;
    check(!w2.execute(ScheduleCommand{A,w2.person(A).decisions+1,ScheduleCommand::Operation::Cancel,firstId2,0,0,0,0}).success&&w2.fingerprint()==beforePast,"processed cancel reject");

    // wrong owner
    Realm two(37,6,256);
    check(two.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,500}).success,"submit A");
    uint64_t sid=two.schedule_book().sessions.at(A).front().id;
    auto bw=two.fingerprint();
    check(!two.execute(ScheduleCommand{130,two.person(130).decisions+1,ScheduleCommand::Operation::Cancel,sid,0,0,0,0}).success&&two.fingerprint()==bw,"wrong owner cancel");
    auto br=two.fingerprint();
    check(!two.execute(ScheduleCommand{130,two.person(130).decisions+1,ScheduleCommand::Operation::Reprioritize,sid,0,0,5}).success&&two.fingerprint()==br,"wrong owner reprioritize");

    // exact attention counters
    Realm wc(37,6,256);
    check(wc.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"submit counters");
    wc.advance_ticks(32);
    check(wc.game_tick()==32,"partial cursor");
    auto&sc=wc.schedule_book().sessions.at(A).front();
    check(sc.requested==32000&&sc.delivered==32000&&sc.unmet==0,"exact partial counters");
    wc.advance_ticks(32);
    check(wc.game_tick()==64,"boundary cursor");
    check(wc.day()==1,"daily once at boundary");
    auto&scb=wc.schedule_book().sessions.at(A).front();
    check(scb.requested==64000&&scb.delivered==64000&&scb.unmet==0,"exact full counters");

    // truly overlapping sessions and busy actor submit
    Realm ov(37,6,256);
    int homeOv=ov.person(A).region;
    check(ov.execute({A,1,{Intent::Work,homeOv,0,0,30}}).success,"work busy");
    check(ov.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,0,64,0,600}).success,"busy submit 1");
    check(ov.execute(ScheduleCommand{A,3,ScheduleCommand::Operation::Submit,0,0,64,0,400}).success,"busy submit 2 overlap");
    ov.advance_ticks(64);
    auto&ovs=ov.schedule_book().sessions.at(A);
    // find sessions
    uint64_t id600=ovs[0].id, id400=ovs[1].id;
    (void)id600;(void)id400;
    int64_t req600=0,del600=0,req400=0,del400=0;
    for(auto const&s:ovs){if(s.demand_rate==600){req600=s.requested;del600=s.delivered;}else{req400=s.requested;del400=s.delivered;}}
    check(req600==38400&&del600==38400,"compatible higher session");
    check(req400==25600&&del400==25600,"compatible lower session");
    check(ov.person(A).work_assignment.job==homeOv,"busy job preserved");

    // overcommit equal priority -> unresolved exact counters
    Realm oc(37,6,256);
    check(oc.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"oc submit1");
    check(oc.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"oc submit2");
    oc.advance_ticks(64);
    int64_t ocReq=0,ocDel=0,ocUnres=0,ocCap=0;
    for(auto const&s:oc.schedule_book().sessions.at(A)){ocReq+=s.requested;ocDel+=s.delivered;ocUnres+=s.unresolved_unmet;ocCap+=s.capacity_unmet;}
    check(ocReq==128000,"oc requested");
    check(ocDel==0,"oc delivered none");
    check(ocUnres==128000,"oc unresolved exact");
    check(ocCap==0,"oc capacity none");

    // exclusive higher consumes capacity; lower shortage
    Realm ex(37,6,256);
    check(ex.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,5,1000}).success,"ex high submit");
    check(ex.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,0,64,1,1000}).success,"ex low submit");
    ex.advance_ticks(64);
    int64_t highReq=0,highDel=0,lowReq=0,lowDel=0,lowCap=0;
    for(auto const&s:ex.schedule_book().sessions.at(A)){if(s.priority==5){highReq=s.requested;highDel=s.delivered;}else{lowReq=s.requested;lowDel=s.delivered;lowCap=s.capacity_unmet;}}
    check(highReq==64000&&highDel==64000,"ex high full");
    check(lowReq==64000&&lowDel==0,"ex low none");
    check(lowCap==64000,"ex low capacity shortage");

    // reprioritize future success+effect
    Realm rp(37,6,256);
    auto rps=rp.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,64,128,0,500});
    check(rps.success,"future submit");
    auto rpr=rp.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Reprioritize,rps.session,0,0,7,0});
    check(rpr.success,"reprioritize future");
    check(rp.schedule_book().sessions.at(A).front().priority==7,"reprioritize effect");

    // ongoing edit reject
    Realm oe(37,6,256);
    auto oes=oe.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,500});
    check(oes.success,"ongoing submit");
    oe.advance_ticks(32);
    auto oeb=oe.fingerprint();
    check(!oe.execute(ScheduleCommand{A,oe.person(A).decisions+1,ScheduleCommand::Operation::Reprioritize,oes.session,0,0,9,0}).success&&oe.fingerprint()==oeb,"ongoing reprioritize reject");
    auto oecb=oe.fingerprint();
    check(!oe.execute(ScheduleCommand{A,oe.person(A).decisions+1,ScheduleCommand::Operation::Cancel,oes.session,0,0,0,0}).success&&oe.fingerprint()==oecb,"ongoing cancel reject");

    // death spanning session
    Realm wd=Realm::scenario(37,A,30,50,0,0,30,30,0,3);
    auto wds1=wd.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,20*64,0,1000});
    check(wds1.success,"death span submit");
    uint64_t deathId=wds1.session;
    auto wds2=wd.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,20*64,21*64,0,1000});
    check(wds2.success,"future session submit");
    uint64_t futureId=wds2.session;
    wd.advance_ticks(15*64+1);
    auto&sd=wd.schedule_book().sessions.at(A).front();
    auto&sf=*std::find_if(wd.schedule_book().sessions.at(A).begin(),wd.schedule_book().sessions.at(A).end(),[&](const ActivitySession&s){return s.id==futureId;});
    (void)deathId;
    check(sd.retired,"span retired on death");
    check(sd.retired_tick>0&&sd.retired_tick<=wd.game_tick(),"span retired tick valid");
    check(sd.requested==1000*std::min(sd.retired_tick,20*64LL),"span requested clipped");
    int64_t reqBefore=sd.requested;
    wd.advance_ticks(64);
    auto&sd2=wd.schedule_book().sessions.at(A).front();
    check(sd2.requested==reqBefore,"no ghost delivery after death");
    check(sf.retired,"future retired before begin");
    const auto sf2=std::find_if(wd.schedule_book().sessions.at(A).begin(),wd.schedule_book().sessions.at(A).end(),
        [&](const ActivitySession&s){return s.id==futureId;});
    check(sf2!=wd.schedule_book().sessions.at(A).end()&&sf2->retired,"future remains retired after next day");
    check(sf2!=wd.schedule_book().sessions.at(A).end()&&sf2->requested==0,"future zero counters");
    // dead owner command reject
    auto wdb=wd.fingerprint();
    check(!wd.execute(ScheduleCommand{A,wd.person(A).decisions+1,ScheduleCommand::Operation::Submit,0,wd.game_tick(),wd.game_tick()+64,0,1000}).success&&wd.fingerprint()==wdb,"dead owner submit reject");
    // save/load retirement
    std::string wdd=wd.serialize("test"),wde; Realm wdr(2,1,128);
    check(wdr.deserialize(wdd,"test",wde),"death save load");
    check(wdr.fingerprint()==wd.fingerprint(),"death fingerprint");

    // paired economics before/at boundary
    Realm pa(37,6,256), pb(37,6,256);
    check(pb.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"paired submit");
    pa.advance_ticks(32); pb.advance_ticks(32);
    check(pa.day()==0&&pb.day()==0,"before boundary no daily");
    check(pa.balances()==pb.balances(),"before boundary balances");
    pa.advance_ticks(32); pb.advance_ticks(32);
    check(pa.day()==1&&pb.day()==1,"boundary daily once");
    check(pa.balances()==pb.balances(),"paired balances");
    for(size_t i=0;i<pa.view().provinces.size();++i){
        check(pa.view().provinces[i].treasury==pb.view().provinces[i].treasury,"paired treasury");
        check(pa.view().provinces[i].firm==pb.view().provinces[i].firm,"paired firm");
        for(int g=0;g<7;++g)check(pa.view().provinces[i].stock[g]==pb.view().provinces[i].stock[g],"paired stock");
    }
    for(uint64_t id=1;id<=pa.count();++id){
        check(pa.person(id).cash==pb.person(id).cash,"paired cash");
        for(int g=0;g<7;++g)check(pa.person(id).inventory[g]==pb.person(id).inventory[g],"paired inventory");
        check(pa.person(id).age_days==pb.person(id).age_days,"paired age");
        check(pa.person(id).health==pb.person(id).health,"paired health");
        check(pa.person(id).nutrition==pb.person(id).nutrition,"paired nutrition");
        check(pa.person(id).skill==pb.person(id).skill,"paired skill");
        check(pa.person(id).work_assignment.job==pb.person(id).work_assignment.job,"paired job");
    }

    // split fingerprint with genuine scheduled worlds
    Realm sp1(37,6,256), sp2(37,6,256);
    check(sp1.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,600}).success,"sp submit1");
    check(sp1.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,0,64,0,400}).success,"sp submit2");
    check(sp2.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,600}).success,"sp2 submit1");
    check(sp2.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,0,64,0,400}).success,"sp2 submit2");
    sp1.advance_ticks(17); sp1.advance_ticks(79);
    sp2.advance_ticks(96);
    check(sp1.fingerprint()==sp2.fingerprint(),"split fingerprint");

    // midinterval save continue
    Realm mc(37,6,256);
    check(mc.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"mid submit");
    mc.advance_ticks(10);
    std::string mcd=mc.serialize("test"),mce; Realm mcr(2,1,128);
    check(mcr.deserialize(mcd,"test",mce),"mid save");
    mcr.advance_ticks(20);
    Realm mfull(37,6,256);
    check(mfull.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"mid full submit");
    mfull.advance_ticks(30);
    check(mcr.fingerprint()==mfull.fingerprint(),"midinterval continue");

    // shared sequence legacy->schedule->legacy->duplicate
    Realm ss(37,6,256);
    int ssr=ss.person(1).region;
    check(ss.execute({1,1,{Intent::Tax,ssr,0,20}}).success,"seq legacy 1");
    check(ss.execute(ScheduleCommand{1,2,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"seq schedule 2");
    check(ss.execute({1,3,{Intent::Tax,ssr,0,20}}).success,"seq legacy 3");
    auto ssb=ss.fingerprint();
    check(!ss.execute({1,3,{Intent::Tax,ssr,0,20}}).success&&ss.fingerprint()==ssb,"duplicate seq reject");

    // invalid rejects fingerprint (no empty insertion)
    Realm iv(37,6,256);
    check(iv.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"iv submit");
    auto ivb=iv.fingerprint();
    iv.advance_ticks(10);
    auto ivafter=iv.fingerprint();
    check(!iv.execute(ScheduleCommand{A,iv.person(A).decisions+1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success&&iv.fingerprint()==ivafter,"past begin reject");
    // reset for other invalid checks on fresh world
    Realm iv2(37,6,256);
    auto iv2b=iv2.fingerprint();
    check(!iv2.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,64,0,0,1000}).success&&iv2.fingerprint()==iv2b,"reversed range reject");
    check(!iv2.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,0,0,1000}).success&&iv2.fingerprint()==iv2b,"zero range reject");
    check(!iv2.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,0}).success&&iv2.fingerprint()==iv2b,"zero rate reject");
    check(!iv2.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1001}).success&&iv2.fingerprint()==iv2b,"high rate reject");
    check(!iv2.execute(ScheduleCommand{A+1000,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success&&iv2.fingerprint()==iv2b,"bad actor reject");
    check(!iv2.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,64,64,0,1000}).success&&iv2.fingerprint()==iv2b,"empty range reject");
    check(iv2.schedule_book().sessions.find(A)==iv2.schedule_book().sessions.end(),"no empty insertion");

    // negative advance / overflow
    Realm na(37,6,256);
    auto nab=na.fingerprint();
    bool threw=false;
    try{na.advance_ticks(-1);}catch(std::invalid_argument&){threw=true;}
    check(threw&&na.fingerprint()==nab,"negative advance reject");
    threw=false;
    try{na.advance_ticks(365000LL*64+1);}catch(std::invalid_argument&){threw=true;}
    check(threw&&na.fingerprint()==nab,"overflow advance reject");

    // Work sessions identify a site; Generic retains its legacy defaults.
    Realm typed(37,6,256);
    int typedRegion=typed.person(A).region;
    check(typed.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"generic submit");
    check(typed.schedule_book().sessions.at(A).front().kind==ActivityKind::Generic&&typed.schedule_book().sessions.at(A).front().region==-1,"generic defaults");
    check(typed.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,typedRegion,work_site(typed,typedRegion)}).success,"work submit");
    const uint64_t typedSite=typed.schedule_book().sessions.at(A).back().establishment_id;
    check(typed.schedule_book().sessions.at(A).back().kind==ActivityKind::Work&&typed.schedule_book().sessions.at(A).back().region==typedRegion&&typedSite!=0,"work typed stored");
    check(typed.execute(ScheduleCommand{A,3,ScheduleCommand::Operation::Reprioritize,typed.schedule_book().sessions.at(A).back().id,0,0,5}).success&&
          typed.schedule_book().sessions.at(A).back().establishment_id==typedSite,"reprioritize preserves worksite");
    const auto typedBeforeGenericSite=typed.fingerprint();
    check(!typed.execute(ScheduleCommand{A,4,ScheduleCommand::Operation::Submit,0,64,96,0,1000,ActivityKind::Generic,-1,typedSite}).success&&
          typed.fingerprint()==typedBeforeGenericSite,"generic cannot name a site");
    typed.advance_ticks(64);
    check(typed.schedule_book().work_present_ticks.at(A)==64,"work attendance credited");

    Realm badKind(37,6,256);
    auto badKindBefore=badKind.fingerprint();
    check(!badKind.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,static_cast<ActivityKind>(2),-1}).success&&badKind.fingerprint()==badKindBefore,"invalid kind atomic");
    check(!badKind.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Generic,0}).success&&badKind.fingerprint()==badKindBefore,"generic region atomic");
    check(!badKind.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,-1}).success&&badKind.fingerprint()==badKindBefore,"work negative region atomic");
    check(!badKind.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,badKind.regions()+1}).success&&badKind.fingerprint()==badKindBefore,"work high region atomic");
    check(!badKind.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,badKind.person(A).region}).success&&badKind.fingerprint()==badKindBefore,"work requires a site");
    check(!badKind.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,badKind.person(A).region,999999}).success&&badKind.fingerprint()==badKindBefore,"unknown worksite rejected atomically");
    check(badKind.schedule_book().sessions.empty()&&badKind.schedule_book().work_present_ticks.empty()&&!badKind.timeline_contract(),"invalid schedules leave no state");
    const auto upperSite=work_site(badKind,badKind.regions());
    check(badKind.execute(ScheduleCommand{1,badKind.person(1).decisions+1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,badKind.regions(),upperSite}).success,"work region upper bound");

    // Overlapping Work sessions credit the union once.
    Realm unioned(37,6,256);
    int unionRegion=unioned.person(A).region;
    check(unioned.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,40,0,1000,ActivityKind::Work,unionRegion,work_site(unioned,unionRegion)}).success,"union submit 1");
    check(unioned.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,20,64,0,1000,ActivityKind::Work,unionRegion,work_site(unioned,unionRegion)}).success,"union submit 2");
    unioned.advance_ticks(64);
    check(unioned.schedule_book().work_present_ticks.at(A)==64,"union credited once");
    unioned.advance_ticks(64);
    check(unioned.schedule_book().work_present_ticks.at(A)==64,"no credit outside sessions");

    // Cross-site overlaps reject; adjacent site sessions partition physical attendance.
    Realm crossSite(37,6,256);
    const int crossRegion=crossSite.person(A).region;
    const uint64_t firstSite=work_site(crossSite,crossRegion);
    const uint64_t governor=crossRegion==crossSite.regions()?1:uint64_t(crossRegion+2);
    auto secondCreate=crossSite.execute(EstablishmentCommand{governor,crossSite.person(governor).decisions+1,
        EstablishmentOwnerKind::Public,uint64_t(crossRegion),crossRegion,EstablishmentIndustry::ToolWorkshop,1,1});
    check(secondCreate.success&&secondCreate.session!=firstSite,"second work establishment created");
    test_fixtures::authorize_work(crossSite,secondCreate.session,A);
    check(crossSite.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,32,0,1000,
        ActivityKind::Work,crossRegion,firstSite}).success,"first site session submitted");
    const auto crossBefore=crossSite.fingerprint();
    check(!crossSite.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,16,48,0,1000,
        ActivityKind::Work,crossRegion,secondCreate.session}).success&&crossSite.fingerprint()==crossBefore,
        "different site overlap rejected atomically");
    check(crossSite.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,32,64,0,1000,
        ActivityKind::Work,crossRegion,secondCreate.session}).success,"adjacent second site session submitted");
    crossSite.advance_ticks(64);
    check(crossSite.schedule_book().work_present_ticks.at(A)==64,"cross-site physical union exact");
    check(crossSite.schedule_book().work_present_ticks_by_establishment.at({A,firstSite})==32&&
          crossSite.schedule_book().work_present_ticks_by_establishment.at({A,secondCreate.session})==32,
          "attendance reconciles by establishment");
    check(crossSite.invariant(),"cross-site attendance invariant");

    // Region mismatch never counts as physical Work presence.
    Realm mismatch(37,6,256);
    int mismatchRegion=mismatch.person(A).region==0?1:0;
    const auto mismatchSite=work_site(mismatch,mismatchRegion);
    const auto mismatchBefore=mismatch.fingerprint();
    check(!mismatch.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,mismatchRegion,mismatchSite}).success&&mismatch.fingerprint()==mismatchBefore,"mismatch site rejected atomically");
    check(mismatch.schedule_book().work_present_ticks.find(A)==mismatch.schedule_book().work_present_ticks.end(),"region mismatch no credit");

    // Chunk splits produce the same attendance as one contiguous advance.
    Realm splitOne(37,6,256),splitTwo(37,6,256);
    int splitRegion=splitOne.person(A).region;
    check(splitOne.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,splitRegion,work_site(splitOne,splitRegion)}).success,"split one submit 1");
    check(splitOne.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,32,128,0,1000,ActivityKind::Work,splitRegion,work_site(splitOne,splitRegion)}).success,"split one submit 2");
    check(splitTwo.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,splitRegion,work_site(splitTwo,splitRegion)}).success,"split two submit 1");
    check(splitTwo.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,32,128,0,1000,ActivityKind::Work,splitRegion,work_site(splitTwo,splitRegion)}).success,"split two submit 2");
    splitOne.advance_ticks(96);
    splitTwo.advance_ticks(17); splitTwo.advance_ticks(79);
    check(splitOne.fingerprint()==splitTwo.fingerprint(),"attendance split fingerprint");
    check(splitOne.schedule_book().work_present_ticks.at(A)==96,"attendance split union");

    // Travel blocks presence until arrival; the original Work region then
    // no longer matches the actor's physical region.
    Realm travel(37,6,256);
    int travelRegion=travel.person(A).region;
    int travelTarget=(travelRegion+1)%(travel.regions()+1);
    check(travel.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,128,0,1000,ActivityKind::Work,travelRegion,work_site(travel,travelRegion)}).success,"travel work submit");
    check(travel.execute({A,2,{Intent::Travel,travelTarget,0,0,0}}).success,"travel command");
    check(travel.person(A).destination!=-1,"travel destination set");
    const auto travellingBefore=travel.fingerprint();
    const auto travelSite=work_site(travel,travelRegion);
    check(!travel.execute(ScheduleCommand{A,travel.person(A).decisions+1,ScheduleCommand::Operation::Submit,0,128,192,0,1000,
        ActivityKind::Work,travelRegion,travelSite}).success&&travel.fingerprint()==travellingBefore,"travelling actor cannot submit new work");
    travel.advance_ticks(64);
    check(travel.schedule_book().work_present_ticks.find(A)==travel.schedule_book().work_present_ticks.end(),"travel no credit");
    check(travel.schedule_book().work_effort_attention_by_establishment.empty(),"attention delivered while travelling earns no worksite effort");
    travel.advance_ticks(64);
    check(travel.schedule_book().work_present_ticks.find(A)==travel.schedule_book().work_present_ticks.end(),"arrived region mismatch no credit");
    check(travel.schedule_book().work_effort_attention_by_establishment.empty(),"attention after arrival away from worksite earns no worksite effort");

    // Fixture surgery on the real versioned save text for dead/travelling actors.
    auto person_row=[](const Person&p,bool alive,int destination){
        std::ostringstream out;
        out<<p.id<<' '<<p.household_membership.household_id<<' '<<p.decisions<<' '<<p.sampling<<' '<<p.region<<' '<<p.age_days<<' '<<p.health<<' '<<p.skill<<' '<<p.work_assignment.job<<' '<<p.busy_until<<' '<<destination<<' '<<p.goal<<' '<<p.school_days<<' '<<p.social<<' '<<(alive?1:0)<<' '<<int(p.family)<<' '<<p.cash<<' '<<p.tax_remainder<<' '<<p.organization<<' '<<p.work_assignment.job_kind<<' '<<p.taxing_authority<<' '<<p.sovereign<<' '<<p.nutrition<<' ';
        for(auto v:p.stats.values)out<<v<<' ';
        for(auto v:p.inventory)out<<v<<' ';
        return out.str();
    };
    auto patch_person=[&](const std::string&data,const Person&p,bool alive,int destination){
        auto oldRow=person_row(p,p.alive,p.destination);
        auto newRow=person_row(p,alive,destination);
        auto pos=data.find(oldRow);
        if(pos==std::string::npos||data.find(oldRow,pos+1)!=std::string::npos)throw std::runtime_error("person row not uniquely located");
        return data.substr(0,pos)+newRow+data.substr(pos+oldRow.size());
    };

    Realm deadSeed(37,6,256);
    int deadRegion=deadSeed.person(A).region;
    check(deadSeed.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,deadRegion,work_site(deadSeed,deadRegion)}).success,"dead fixture submit");
    std::string deadData=patch_person(deadSeed.serialize("test"),deadSeed.person(A),false,deadSeed.person(A).destination);
    Realm dead; std::string deadError;
    check(dead.deserialize(deadData,"test",deadError),"dead fixture load");
    check(!dead.person(A).alive,"dead fixture is dead");
    dead.advance_ticks(64);
    check(dead.schedule_book().work_present_ticks.find(A)==dead.schedule_book().work_present_ticks.end(),"dead actor no attendance");
    check(dead.invariant(),"dead attendance invariant");

    Realm invalidSeed(37,6,256);
    int invalidRegion=invalidSeed.person(A).region;
    check(invalidSeed.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,invalidRegion,work_site(invalidSeed,invalidRegion)}).success,"invalid destination fixture submit");
    std::string invalidData=patch_person(invalidSeed.serialize("test"),invalidSeed.person(A),true,-2);
    Realm invalidDest; std::string invalidError;
    check(invalidDest.deserialize(invalidData,"test",invalidError),"invalid destination fixture load");
    check(invalidDest.person(A).destination==-2,"invalid destination fixture value");
    invalidDest.advance_ticks(64);
    check(invalidDest.schedule_book().work_present_ticks.find(A)==invalidDest.schedule_book().work_present_ticks.end(),"invalid negative destination no attendance");
    check(invalidDest.invariant(),"invalid destination attendance invariant");

    // v12 save/load keeps typed site attendance, regional watermarks and settlement across a mid-day split.
    Realm mid(37,6,256);
    int midRegion=mid.person(A).region;
    check(mid.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,128,0,1000,ActivityKind::Work,midRegion,work_site(mid,midRegion)}).success,"mid typed submit");
    mid.advance_ticks(20);
    std::string midData=mid.serialize("test");
    check(midData.rfind("POLYEC_REALM_51",0)==0,"current save magic");
    check(midData.find(" SCHEDULE_7 ")!=std::string::npos,"current schedule tag");
    check(mid.serialize("test")==midData,"v14 deterministic save");
    Realm midRestored(2,1,128); std::string midError;
    check(midRestored.deserialize(midData,"test",midError),"typed roundtrip");
    check(midRestored.fingerprint()==mid.fingerprint(),"typed roundtrip fingerprint");
    check(midRestored.schedule_book().work_present_ticks.at(A)==20,"attendance restored");
    check(midRestored.schedule_book().work_present_ticks_by_region.at({A,midRegion})==20,"regional attendance restored");
    check(midRestored.schedule_book().work_present_ticks_by_establishment.at({A,mid.schedule_book().sessions.at(A).front().establishment_id})==20,"site attendance restored");
    check(midRestored.schedule_book().paid_work_units_by_region.empty(),"regional paid restored empty");
    check(midRestored.schedule_book().settled_day==0&&midRestored.schedule_book().settled_work_units.empty(),"midday settlement empty");
    check(midRestored.schedule_book().sessions.at(A).front().kind==ActivityKind::Work,"kind restored");
    check(midRestored.schedule_book().sessions.at(A).front().region==midRegion,"region restored");
    check(midRestored.invariant(),"typed roundtrip invariant");
    midRestored.advance_ticks(44);
    Realm midFull(37,6,256);
    check(midFull.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,128,0,1000,ActivityKind::Work,midRegion,work_site(midFull,midRegion)}).success,"mid full submit");
    midFull.advance_ticks(64);
    check(midRestored.fingerprint()==midFull.fingerprint(),"midday resume equivalence");
    check(midRestored.schedule_book().settled_day==1&&midRestored.schedule_book().settled_work_units.at(A)==4,"midday boundary settled");
    check(midRestored.establishments()[0].work_pay.paid_units.at(A)==4&&midRestored.schedule_book().paid_work_units_by_region.empty(),"midday regional wage watermark");

    // T2.2 settlement converts attendance to units at completed days only.
    auto boundaryUnits=[A](int64_t end)->int64_t{
        Realm w(37,6,256);int r=w.person(A).region;
        check(w.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,end,0,1000,ActivityKind::Work,r,work_site(w,r)}).success,"floor submit");
        w.advance_ticks(64);
        auto found=w.schedule_book().settled_work_units.find(A);
        return found==w.schedule_book().settled_work_units.end()?0:found->second;
    };
    check(boundaryUnits(15)==0,"15 ticks no unit");
    check(boundaryUnits(16)==1,"16 ticks one unit");
    check(boundaryUnits(31)==1,"31 ticks one unit");
    check(boundaryUnits(32)==2,"32 ticks two units");

    // Multi-day attendance settles cumulatively and splits fingerprint-exact.
    Realm multiOne(37,6,256),multiSplit(37,6,256);
    int multiRegion=multiOne.person(A).region;
    check(multiOne.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,256,0,1000,ActivityKind::Work,multiRegion,work_site(multiOne,multiRegion)}).success,"multi one submit");
    check(multiSplit.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,256,0,1000,ActivityKind::Work,multiRegion,work_site(multiSplit,multiRegion)}).success,"multi split submit");
    multiOne.advance_ticks(256);
    multiSplit.advance_ticks(17);multiSplit.advance_ticks(79);multiSplit.advance_ticks(160);
    check(multiOne.fingerprint()==multiSplit.fingerprint(),"multi-day settlement split fingerprint");
    check(multiOne.schedule_book().settled_day==4&&multiOne.schedule_book().settled_work_units.at(A)==16,"multi-day settled totals");
    check(multiOne.schedule_book().work_present_ticks.at(A)==256,"multi-day attendance exact");

    // Mid-day save and restore continues settlement without re-crediting.
    Realm mdr(37,6,256);
    int mdrRegion=mdr.person(A).region;
    check(mdr.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,256,0,1000,ActivityKind::Work,mdrRegion,work_site(mdr,mdrRegion)}).success,"mdr submit");
    mdr.advance_ticks(80);
    check(mdr.schedule_book().settled_day==1&&mdr.schedule_book().settled_work_units.at(A)==4,"mdr day1 units");
    std::string mdrData=mdr.serialize("test"); Realm mdrRestored(2,1,128); std::string mdrError;
    check(mdrRestored.deserialize(mdrData,"test",mdrError),"mdr save restore");
    check(mdrRestored.fingerprint()==mdr.fingerprint(),"mdr restore exact");
    mdrRestored.advance_ticks(48);
    Realm mdrFull(37,6,256);
    check(mdrFull.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,256,0,1000,ActivityKind::Work,mdrRegion,work_site(mdrFull,mdrRegion)}).success,"mdr full submit");
    mdrFull.advance_ticks(128);
    check(mdrRestored.fingerprint()==mdrFull.fingerprint(),"mdr resume equivalence");
    check(mdrRestored.schedule_book().settled_day==2&&mdrRestored.schedule_book().settled_work_units.at(A)==8,"mdr day2 units");

    // Idle boundaries advance the watermark without double counting.
    Realm idle(37,6,256);
    int idleRegion=idle.person(A).region;
    check(idle.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,idleRegion,work_site(idle,idleRegion)}).success,"idle submit");
    idle.advance_ticks(64);
    int64_t idleAttendance=idle.schedule_book().work_present_ticks.at(A);
    int64_t idleUnits=idle.schedule_book().settled_work_units.at(A);
    check(idleAttendance==64&&idleUnits==4&&idle.schedule_book().settled_day==1,"idle day1 settled");
    idle.advance_ticks(64);
    check(idle.schedule_book().work_present_ticks.at(A)==idleAttendance,"idle attendance unchanged");
    check(idle.schedule_book().settled_work_units.at(A)==idleUnits&&idle.schedule_book().settled_day==2,"idle no double count");

    // Legacy timeline carries no settlement until typed Work arrives, with no backfill.
    Realm late(37,6,256);
    late.advance_ticks(128);
    check(!late.timeline_contract()&&late.schedule_book().settled_day==0&&late.schedule_book().settled_work_units.empty(),"legacy no settlement");
    int lateRegion=late.person(A).region;
    check(late.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,128,192,0,1000,ActivityKind::Work,lateRegion,work_site(late,lateRegion)}).success,"late work submit");
    late.advance_ticks(64);
    check(late.schedule_book().work_present_ticks.at(A)==64,"late work attendance exact");
    check(late.schedule_book().settled_day==3&&late.schedule_book().settled_work_units.at(A)==4,"late work day3 units");

    // Downgrade helpers perform deterministic token surgery on v12 text.
    auto schedule_parts=[&](const std::string&source,bool retainEstablishments=false)->std::pair<std::string,std::vector<std::string>>{
        const std::string data=downgrade_goods_to_v12(source);
        const std::string tag=" SCHEDULE_5 ";
        if(data.rfind("POLYEC_REALM_12",0)!=0)throw std::runtime_error("v12 magic missing");
        size_t tagPos=data.find(tag);
        if(tagPos==std::string::npos)throw std::runtime_error("v12 schedule tag missing");
        std::istringstream in(data.substr(tagPos+tag.size()));
        std::vector<std::string> tokens; std::string token; while(in>>token)tokens.push_back(token);
        std::string prefix=data.substr(0,tagPos);
        const size_t establishmentTag=data.find(" ESTABLISH_1 ");
        if(establishmentTag!=std::string::npos&&!retainEstablishments){
            std::istringstream est(data.substr(establishmentTag));std::string tag;int64_t count=0;est>>tag>>count;
            if(!est||count<0)throw std::runtime_error("v12 establishment count missing");
            std::istringstream head(prefix);std::vector<std::string> h;while(head>>token)h.push_back(token);
            if(h.size()<10)throw std::runtime_error("realm header truncated");
            int64_t refunded=0;
            for(int64_t row=0;row<count;++row){
                uint64_t id=0,owner=0;int region=0,kind=0,industry=0,capacity=0;int64_t cash=0,value=0;
                est>>id>>region>>kind>>owner>>industry>>capacity>>cash;refunded+=cash;
                for(int good=0;good<16;++good){est>>value;if(value!=0)throw std::runtime_error("cannot drop stocked historical site");}
            }
            if(!est)throw std::runtime_error("historical site refund fixture");
            h[9]=std::to_string(std::stoll(h[9])+refunded);
            std::ostringstream adjusted;for(size_t i=0;i<h.size();++i){if(i)adjusted<<' ';adjusted<<h[i];}prefix=adjusted.str();
        }
        return {prefix,tokens};
    };
    auto downgrade_to_v9=[&](const std::string&data){
        auto parts=schedule_parts(data);auto const&tokens=parts.second;
        if(tokens.size()<10)throw std::runtime_error("v12 schedule tokens missing");
        size_t sessionCount=size_t(std::stoul(tokens[4])),attendanceCount=size_t(std::stoul(tokens[5]));
        size_t settledCount=size_t(std::stoul(tokens[6])),at=10;
        if(at+sessionCount*17+attendanceCount*2+settledCount*2>tokens.size())throw std::runtime_error("v12 v9 tokens missing");
        std::ostringstream rebuilt;
        rebuilt<<parts.first<<" SCHEDULE_3 "<<tokens[0]<<' '<<tokens[1]<<' '<<tokens[2]<<' '<<tokens[3]<<' '<<tokens[4]<<' '<<tokens[5]<<' '<<tokens[6]<<' ';
        for(size_t i=0;i<sessionCount;++i){for(int f=0;f<16;++f)rebuilt<<tokens[at+f]<<' ';at+=17;}
        for(size_t i=0;i<attendanceCount;++i){rebuilt<<tokens[at]<<' '<<tokens[at+1]<<' ';at+=2;}
        for(size_t i=0;i<settledCount;++i){rebuilt<<tokens[at]<<' '<<tokens[at+1]<<' ';at+=2;}
        std::string out=rebuilt.str();out.replace(0,15,"POLYEC_REALM_9");return out;
    };
    auto downgrade_to_v8=[&](const std::string&data){
        auto parts=schedule_parts(data);auto const&tokens=parts.second;
        if(tokens.size()<10)throw std::runtime_error("v12 schedule tokens missing");
        size_t sessionCount=size_t(std::stoul(tokens[4])),attendanceCount=size_t(std::stoul(tokens[5])),at=10;
        if(at+sessionCount*17+attendanceCount*2>tokens.size())throw std::runtime_error("v12 v8 tokens missing");
        std::ostringstream rebuilt;
        rebuilt<<parts.first<<" SCHEDULE_2 "<<tokens[0]<<' '<<tokens[1]<<' '<<tokens[2]<<' '<<tokens[4]<<' '<<tokens[5]<<' ';
        for(size_t i=0;i<sessionCount;++i){for(int f=0;f<16;++f)rebuilt<<tokens[at+f]<<' ';at+=17;}
        for(size_t i=0;i<attendanceCount;++i){rebuilt<<tokens[at]<<' '<<tokens[at+1]<<' ';at+=2;}
        std::string out=rebuilt.str();out.replace(0,15,"POLYEC_REALM_8");return out;
    };
    auto downgrade_to_v7=[&](const std::string&data){
        auto parts=schedule_parts(data);auto const&tokens=parts.second;
        if(tokens.size()<10)throw std::runtime_error("v12 schedule tokens missing");
        size_t sessionCount=size_t(std::stoul(tokens[4])),at=10;
        if(at+sessionCount*17>tokens.size())throw std::runtime_error("v12 v7 tokens missing");
        std::ostringstream rebuilt;
        rebuilt<<parts.first<<" SCHEDULE_1 "<<tokens[0]<<' '<<tokens[1]<<' '<<tokens[2]<<' '<<tokens[4]<<' ';
        for(size_t i=0;i<sessionCount;++i){for(int f=0;f<14;++f)rebuilt<<tokens[at+f]<<' ';at+=17;}
        std::string out=rebuilt.str();out.replace(0,15,"POLYEC_REALM_7");return out;
    };
    auto downgrade_to_v11=[&](const std::string&data){
        auto parts=schedule_parts(data,true);auto const&tokens=parts.second;
        const size_t sessionCount=size_t(std::stoul(tokens[4])),attendanceCount=size_t(std::stoul(tokens[5]));
        const size_t settledCount=size_t(std::stoul(tokens[6])),regionalTickCount=size_t(std::stoul(tokens[7]));
        const size_t regionalPaidCount=size_t(std::stoul(tokens[8])),siteTickCount=size_t(std::stoul(tokens[9]));
        size_t at=10;
        std::ostringstream rebuilt;rebuilt<<parts.first<<" SCHEDULE_4 ";
        for(size_t i=0;i<9;++i)rebuilt<<tokens[i]<<' ';
        for(size_t i=0;i<sessionCount;++i){for(int f=0;f<16;++f)rebuilt<<tokens[at+f]<<' ';at+=17;}
        for(size_t i=0;i<attendanceCount*2+settledCount*2+regionalTickCount*3+regionalPaidCount*3;++i)rebuilt<<tokens[at+i]<<' ';
        at+=attendanceCount*2+settledCount*2+regionalTickCount*3+regionalPaidCount*3+siteTickCount*3;
        for(;at<tokens.size();++at)rebuilt<<tokens[at]<<' ';
        std::string out=rebuilt.str();out.replace(0,15,"POLYEC_REALM_11");return out;
    };

    // v11 has Establishments but Work sessions predate site attribution.
    Realm typed11(37,6,256);
    const int typed11Region=typed11.person(A).region;
    const uint64_t typed11Site=work_site(typed11,typed11Region);
    check(typed11.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,
        ActivityKind::Work,typed11Region,typed11Site}).success,"v11 source Work submit");
    typed11.advance_ticks(32);
    const std::string v11Fixture=downgrade_to_v11(typed11.serialize("test"));
    check(v11Fixture.rfind("POLYEC_REALM_11",0)==0&&v11Fixture.find(" SCHEDULE_4 ")!=std::string::npos,
          "v11 migration fixture format");
    Realm migrated11(2,1,128);std::string migrate11Error;
    const bool migrated11Ok=migrated11.deserialize(v11Fixture,"test",migrate11Error);
    check(migrated11Ok,migrated11Ok?"v11 migration loads":migrate11Error.c_str());
    check(migrated11.establishments().size()==1&&migrated11.establishments()[0].id==typed11Site,
          "v11 Establishment retained");
    check(migrated11.schedule_book().sessions.at(A).front().establishment_id==0&&
          migrated11.schedule_book().work_present_ticks_by_establishment.empty(),"v11 Work remains site-unassigned");
    check(migrated11.schedule_book().work_present_ticks.at(A)==32&&
          migrated11.schedule_book().work_present_ticks_by_region.at({A,typed11Region})==32,
          "v11 attendance preserved without site backfill");
    const auto legacyOverlapBefore=migrated11.fingerprint();
    check(!migrated11.execute(ScheduleCommand{A,migrated11.person(A).decisions+1,ScheduleCommand::Operation::Submit,
        0,32,48,0,1000,ActivityKind::Work,typed11Region,typed11Site}).success&&
        migrated11.fingerprint()==legacyOverlapBefore,"new work cannot overlap legacy unassigned work");
    check(migrated11.invariant(),"v11 migration invariant");

    // v9 migration keeps legacy global attendance/settlement exactly and starts regional watermarks empty.
    Realm typed9(37,6,256);
    int typed9Region=typed9.person(A).region;
    check(typed9.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,typed9Region,work_site(typed9,typed9Region)}).success,"v9 work submit");
    typed9.advance_ticks(64);
    check(typed9.schedule_book().work_present_ticks.at(A)==64&&typed9.schedule_book().settled_work_units.at(A)==4&&typed9.schedule_book().work_present_ticks_by_region.at({A,typed9Region})==64,"v9 fixture baseline");
    std::string typed9Data=downgrade_to_v9(typed9.serialize("test"));
    check(typed9Data.rfind("POLYEC_REALM_9",0)==0&&typed9Data.find(" SCHEDULE_3 ")!=std::string::npos,"v9 fixture tag");
    Realm migrated9(2,1,128); std::string migrate9Error;
    const bool migrated9Ok=migrated9.deserialize(typed9Data,"test",migrate9Error);
    check(migrated9Ok,migrated9Ok?"v9 migration load":migrate9Error.c_str());
    check(migrated9.serialize("test").rfind("POLYEC_REALM_51",0)==0,"v9 re-serializes current schema");
    check(migrated9.schedule_book().work_present_ticks.at(A)==64,"v9 attendance preserved");
    check(migrated9.schedule_book().settled_day==1&&migrated9.schedule_book().settled_work_units.at(A)==4,"v9 settlement preserved");
    check(migrated9.schedule_book().work_present_ticks_by_region.empty()&&migrated9.schedule_book().paid_work_units_by_region.empty(),"v9 regional maps start empty");
    check(migrated9.invariant(),"v9 migration invariant");
    migrated9.advance_ticks(64);
    check(migrated9.schedule_book().paid_work_units_by_region.empty(),"v9 no retroactive work pay");
    check(migrated9.schedule_book().work_present_ticks.at(A)==64&&migrated9.schedule_book().settled_work_units.at(A)==4,"v9 migration ignores unmapped days");
    check(migrated9.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,128,128,192,0,1000,ActivityKind::Work,typed9Region,work_site(migrated9,typed9Region)}).success,"v9 new work submit");
    migrated9.advance_ticks(64);
    check(migrated9.schedule_book().work_present_ticks.at(A)==128,"v9 new global attendance increments");
    check(migrated9.schedule_book().work_present_ticks_by_region.at({A,typed9Region})==64,"v9 new regional attendance starts at zero");
    check(migrated9.establishments()[0].work_pay.paid_units.at(A)==4&&migrated9.schedule_book().paid_work_units_by_region.empty(),"v9 new work pays only new units");
    check(migrated9.invariant(),"v9 post-pay invariant");

    // v8 migration keeps typed sessions and attendance; settlement starts empty.
    Realm typed8(37,6,256);
    int typed8Region=typed8.person(A).region;
    check(typed8.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"v8 generic submit");
    check(typed8.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,0,64,0,500,ActivityKind::Work,typed8Region,work_site(typed8,typed8Region)}).success,"v8 work submit");
    typed8.advance_ticks(32);
    std::string typed8Data=downgrade_to_v8(typed8.serialize("test"));
    check(typed8Data.rfind("POLYEC_REALM_8",0)==0&&typed8Data.find(" SCHEDULE_2 ")!=std::string::npos,"v8 fixture tag");
    Realm migrated8(2,1,128); std::string migrate8Error;
    check(migrated8.deserialize(typed8Data,"test",migrate8Error),"v8 migration load");
    check(migrated8.serialize("test").rfind("POLYEC_REALM_51",0)==0,"v8 re-serializes current schema");
    check(migrated8.schedule_book().sessions.at(A).size()==2,"v8 sessions restored");
    check(migrated8.schedule_book().work_present_ticks.at(A)==32,"v8 attendance restored");
    check(migrated8.schedule_book().work_present_ticks_by_region.empty()&&migrated8.schedule_book().paid_work_units_by_region.empty(),"v8 regional maps start empty");
    check(migrated8.schedule_book().settled_day==0&&migrated8.schedule_book().settled_work_units.empty(),"v8 settlement starts empty");
    check(migrated8.invariant(),"v8 migration invariant");
    migrated8.advance_ticks(32);
    check(migrated8.schedule_book().settled_day==1&&migrated8.schedule_book().settled_work_units.at(A)==4,"v8 boundary settlement");
    check(migrated8.schedule_book().paid_work_units_by_region.at({A,typed8Region})==2,"v8 pays only post-migration regional ticks");
    migrated8.advance_ticks(64);
    check(migrated8.schedule_book().settled_work_units.at(A)==4&&migrated8.schedule_book().settled_day==2,"v8 no double count");
    check(migrated8.schedule_book().paid_work_units_by_region.at({A,typed8Region})==2,"v8 no retroactive work pay");

    // v7 migration: legacy sessions are Generic/-1 and carry no attendance/settlement.
    Realm modern(37,6,256);
    int modernRegion=modern.person(A).region;
    check(modern.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,modernRegion,work_site(modern,modernRegion)}).success,"modern submit");
    modern.advance_ticks(10);
    std::string legacyData=downgrade_to_v7(modern.serialize("test"));
    check(legacyData.rfind("POLYEC_REALM_7",0)==0,"v7 fixture magic");
    check(legacyData.find(" SCHEDULE_1 ")!=std::string::npos&&legacyData.find(" SCHEDULE_5 ")==std::string::npos,"v7 fixture tag");
    Realm migrated(2,1,128); std::string migrateError;
    check(migrated.deserialize(legacyData,"test",migrateError),"v7 migration load");
    check(migrated.serialize("test").rfind("POLYEC_REALM_51",0)==0,"v7 re-serializes current schema");
    auto const&migratedSession=migrated.schedule_book().sessions.at(A).front();
    check(migratedSession.kind==ActivityKind::Generic&&migratedSession.region==-1,"v7 session defaults");
    check(migrated.schedule_book().work_present_ticks.empty()&&migrated.schedule_book().work_present_ticks_by_region.empty(),"v7 attendance empty");
    check(migrated.schedule_book().settled_day==0&&migrated.schedule_book().settled_work_units.empty(),"v7 settlement empty");
    check(migrated.invariant(),"v7 migration invariant");

    // Corrupt saves with canonical SCHEDULE_5 section, attendance, settlement and regional watermarks.
    Realm cs(1,1,128);std::string csError;
    std::ifstream oldOracle(std::filesystem::path(POLYEC_TEST_FIXTURE_DIR)/"work-v16"/"codec-cs.save");
    const std::string oldCs((std::istreambuf_iterator<char>(oldOracle)),{});
    check(oldCs.rfind("POLYEC_REALM_16 ",0)==0&&cs.deserialize(oldCs,"test",csError),"independent frozen v16 payroll codec oracle loads");
    const int csRegion=cs.person(A).region;
    std::string csd=test_fixtures::save_as_v28(cs.serialize("test"));
    size_t tagPos=csd.find(" SCHEDULE_5 ");
    check(tagPos!=std::string::npos,"schedule tag present");
    std::string prefix=csd.substr(0,tagPos+12);
    std::string suffix=csd.substr(tagPos+12);
    std::vector<std::string> toks;
    std::istringstream tss(suffix);
    std::string tok; while(tss>>tok) toks.push_back(tok);
    const size_t sessionAt=10,attendanceAt=61,settlementAt=65,regionalTickAt=69,regionalPaidAt=75,siteTicksAt=81;
    check(toks.size()==129+9+7+12+16+2+2+2+4*cs.land_holdings().size()+2+3+2+6+2*27*(cs.regions()+1)+5+3+5,"schedule, production, config, body, household, land title, empty land-use, estate, parentage, market, route, and empty-shipment header token count");
    check(toks[0]=="1","base timeline flag");
    check(toks[1]=="64","base cursor");
    check(toks[3]=="1","base settled day");
    check(toks[4]=="3","base session count");
    check(toks[5]=="2","base attendance count");
    check(toks[6]=="2","base settled count");
    check(toks[7]=="2"&&toks[8]=="2"&&toks[9]=="2","base regional and site counts");
    check(toks[attendanceAt]==std::to_string(A)&&toks[attendanceAt+2]==std::to_string(A+1),"attendance sorted by actor");
    check(toks[settlementAt]==std::to_string(A)&&toks[settlementAt+2]==std::to_string(A+1),"settlement sorted by actor");
    check(toks[regionalTickAt]==std::to_string(A)&&toks[regionalTickAt+3]==std::to_string(A+1),"regional attendance sorted by actor");
    check(toks[regionalPaidAt]==std::to_string(A)&&toks[regionalPaidAt+3]==std::to_string(A+1),"regional paid sorted by actor");
    check(toks[siteTicksAt]==std::to_string(A)&&toks[siteTicksAt+3]==std::to_string(A+1),"site attendance sorted by actor");
    auto mutate=[&](const std::vector<std::pair<size_t,std::string>>&changes)->std::string{
        std::vector<std::string> t=toks;
        for(auto const&c:changes)t[c.first]=c.second;
        std::ostringstream j; for(size_t i=0;i<t.size();++i){ if(i)j<<' '; j<<t[i]; }
        return prefix+j.str();
    };
    auto reject=[&](const std::string&d,const char*label,const char*reason){
        Realm r(2,1,128); auto rb=r.fingerprint(); std::string e;
        bool ok=!r.deserialize(d,"test",e);
        check(ok&&r.fingerprint()==rb,label);
        if(reason&&e.find(reason)==std::string::npos)throw std::runtime_error(std::string(label)+": "+e);
    };
    reject(mutate({{1,"128"}}),"cursor day mismatch","Save schedule day mismatch");
    reject(mutate({{0,"0"}}),"flag false with schedule","Save schedule timeline flag");
    reject(mutate({{sessionAt+17,toks[sessionAt]}}),"duplicate id","Save schedule id");
    reject(mutate({{sessionAt+1,"0"}}),"actor0","Save schedule actor");
    reject(mutate({{sessionAt+1,"999999"}}),"actor oor","Save schedule actor");
    reject(mutate({{sessionAt+5,"1001"}}),"bad rate","Save schedule rate");
    reject(mutate({{sessionAt+34+6,"9223372036854775807"},{sessionAt+34+7,"9223372036854775807"},{sessionAt+34+8,"0"}}),"huge counters","Save schedule processed mismatch");
    reject(mutate({{sessionAt+6,"1"},{sessionAt+7,"0"},{sessionAt+8,"1"},{sessionAt+9,"1"},{sessionAt+10,"0"}}),"impossible requested","Save schedule processed mismatch");
    reject(mutate({{2,toks[sessionAt]}}),"next id collision","Save schedule next id");
    reject(mutate({{sessionAt+14,"2"}}),"invalid kind","Save schedule kind");
    reject(mutate({{sessionAt+15,"-1"}}),"work region negative","Save schedule work region");
    reject(mutate({{sessionAt+15,"7"}}),"work region high","Save schedule work region");
    reject(mutate({{sessionAt+17+15,"0"}}),"generic region set","Save schedule generic region");
    reject(mutate({{5,"257"}}),"attendance count limit","Save schedule attendance count limit");
    reject(mutate({{6,"257"}}),"settled count limit","Save schedule settled count limit");
    reject(mutate({{7,"100000"}}),"regional attendance count limit","Save schedule regional count limit");
    reject(mutate({{8,"100000"}}),"regional paid count limit","Save schedule regional count limit");
    reject(mutate({{settlementAt+1,"-1"}}),"settled negative","Save schedule settled units");
    reject(mutate({{3,"100"}}),"settled day beyond cursor","Save schedule settled day");
    reject(mutate({{settlementAt,"0"}}),"settled actor0","Save schedule settled actor");
    reject(mutate({{settlementAt,"999999"}}),"settled actor oor","Save schedule settled actor");
    reject(mutate({{settlementAt,"0"},{settlementAt+1,"0"}}),"settled zero-unit actor0","Save schedule settled actor");
    reject(mutate({{settlementAt,"999999"},{settlementAt+1,"0"}}),"settled zero-unit actor oor","Save schedule settled actor");
    reject(mutate({{settlementAt+1,"5"}}),"settled units over cursor","Save schedule settled units");
    reject(mutate({{settlementAt,"200"}}),"settled actor without attendance","Save schedule settled attendance");
    reject(mutate({{3,"0"}}),"settled day zero with settlement","Save schedule settlement day");
    reject(mutate({{regionalTickAt+2,"0"}}),"regional attendance zero","Save schedule regional attendance ticks");
    reject(mutate({{regionalTickAt+1,"7"}}),"regional attendance region high","Save schedule regional attendance region");
    reject(mutate({{regionalTickAt+1,"-1"}}),"regional attendance region negative","Save schedule regional attendance region");
    reject(mutate({{regionalTickAt+2,"65"}}),"regional attendance over cursor","Save schedule regional attendance ticks");
    reject(mutate({{regionalTickAt,"0"}}),"regional attendance actor0","Save schedule regional attendance actor");
    reject(mutate({{regionalTickAt,"999999"}}),"regional attendance actor oor","Save schedule regional attendance actor");
    reject(mutate({{regionalTickAt+3,toks[regionalTickAt]},{regionalTickAt+4,toks[regionalTickAt+1]}}),"duplicate regional attendance key","Save schedule regional attendance key");
    reject(mutate({{regionalPaidAt+2,"0"}}),"regional paid zero","Save schedule regional paid units");
    reject(mutate({{regionalPaidAt,"0"}}),"regional paid actor0","Save schedule regional paid actor");
    reject(mutate({{regionalPaidAt,"999999"}}),"regional paid actor oor","Save schedule regional paid actor");
    reject(mutate({{regionalPaidAt+1,"7"}}),"regional paid region high","Save schedule regional paid region");
    reject(mutate({{regionalPaidAt,"200"}}),"regional paid without attendance","Save schedule regional paid attendance");
    reject(mutate({{regionalPaidAt+2,"5"}}),"regional paid above floor","Save schedule regional paid attendance");
    reject(mutate({{regionalPaidAt+3,toks[regionalPaidAt]},{regionalPaidAt+4,toks[regionalPaidAt+1]}}),"duplicate regional paid key","Save schedule regional paid key");
    reject(mutate({{siteTicksAt+1,"0"}}),"zero site attendance ID","Save schedule establishment attendance ID");
    reject(mutate({{siteTicksAt+1,"999999"}}),"unknown site attendance ID","Corrupt save/invariant");
    reject(mutate({{siteTicksAt+3,toks[siteTicksAt]},{siteTicksAt+4,toks[siteTicksAt+1]}}),"duplicate site attendance key","Save schedule establishment attendance key");
    reject(mutate({{siteTicksAt+2,"65"}}),"site attendance after cursor","Save schedule establishment attendance ticks");
    {
        std::string wrongTag=csd;
        size_t pos=wrongTag.find(" SCHEDULE_5 ");
        check(pos!=std::string::npos,"wrong schedule-tag fixture exists");
        wrongTag.replace(pos,std::string(" SCHEDULE_5 ").size()," SCHEDULE_2 ");
        reject(wrongTag,"v10 wrong tag","Save schedule header");
    }
    {
        std::vector<std::string> dup=toks;
        dup[settlementAt]=std::to_string(A);dup[settlementAt+1]="4";
        dup[settlementAt+2]=std::to_string(A);dup[settlementAt+3]="1";
        std::ostringstream j; for(size_t i=0;i<dup.size();++i){ if(i)j<<' '; j<<dup[i]; }
        reject(prefix+j.str(),"duplicate settled actor","Save schedule settled actor");
    }
    {
        std::string custom=prefix+"0 64 "+toks[2]+" 0 0 0 0 1 0 0 "+std::to_string(A)+" 0 1 ESTABLISH_5 0 ";
        const auto& saved=csd;custom+=saved.substr(saved.find(" CONFIG_2 "));
        reject(custom,"timeline false nonzero regional attendance","Save schedule timeline flag");
    }
    {
        std::vector<std::string> sum=toks;
        sum[8]="0";
        sum[regionalTickAt+3]=sum[regionalTickAt];
        sum[regionalTickAt+4]=std::to_string(csRegion==0?1:0);
        sum.erase(sum.begin()+std::ptrdiff_t(regionalPaidAt),sum.begin()+std::ptrdiff_t(regionalPaidAt+6));
        std::ostringstream j; for(size_t i=0;i<sum.size();++i){ if(i)j<<' '; j<<sum[i]; }
        reject(prefix+j.str(),"regional attendance sum exceeds global","Save schedule regional attendance total");
    }
    // zero attendance is normalized away on a mid-day save (and its regional record must be removed).
    {
        Realm zs(37,6,256);
        int zsRegion=zs.person(A).region,zsRegionB=zs.person(A+1).region;
        check(zs.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,zsRegion,work_site(zs,zsRegion)}).success,"zs submit 1");
        check(zs.execute(ScheduleCommand{A+1,1,ScheduleCommand::Operation::Submit,0,0,64,0,500,ActivityKind::Work,zsRegionB,work_site(zs,zsRegionB,A+1)}).success,"zs submit 2");
        zs.advance_ticks(10);
        std::string zsd=test_fixtures::save_as_v28(zs.serialize("test"));
        size_t zTag=zsd.find(" SCHEDULE_5 ");
        check(zTag!=std::string::npos,"zs tag present");
        std::string zPrefix=zsd.substr(0,zTag+12),zSuffix=zsd.substr(zTag+12);
        std::vector<std::string> zt; std::istringstream zin(zSuffix); std::string ztoke; while(zin>>ztoke)zt.push_back(ztoke);
        const size_t zAttendanceAt=10+2*17,zRegionalAt=zAttendanceAt+4;
        check(zt.size()==zRegionalAt+12+38+14+16+2+2+2+4*zs.land_holdings().size()+2+3+2+6+2*27*(zs.regions()+1)+5+3+5,"zs schedule, site production, config, body, household, land title, land-use, estate, parentage, market, route, and empty-shipment header token count");
        std::vector<std::string> ztoks=zt;
        ztoks[zAttendanceAt+1]="0";
        ztoks[7]="1";
        ztoks.erase(ztoks.begin()+std::ptrdiff_t(zRegionalAt),ztoks.begin()+std::ptrdiff_t(zRegionalAt+3));
        ztoks[9]="1";
        const size_t siteAt=zRegionalAt+3;
        ztoks.erase(ztoks.begin()+std::ptrdiff_t(siteAt),ztoks.begin()+std::ptrdiff_t(siteAt+3));
        std::ostringstream zj; for(size_t i=0;i<ztoks.size();++i){ if(i)zj<<' '; zj<<ztoks[i]; }
        Realm zr(2,1,128); std::string ze;
        check(zr.deserialize(zPrefix+zj.str(),"test",ze),"zero attendance load");
        check(zr.schedule_book().work_present_ticks.count(A)==0&&zr.schedule_book().work_present_ticks.at(A+1)==10,"zero attendance dropped");
        check(zr.schedule_book().work_present_ticks_by_region.count({A,zsRegion})==0&&zr.schedule_book().work_present_ticks_by_region.at({A+1,zsRegionB})==10,"zero regional attendance dropped");
        check(zr.invariant(),"zero attendance invariant");
    }
    // zero settled units is normalized away on a boundary save.
    {
        Realm zr(2,1,128); std::string ze;
        check(zr.deserialize(mutate({{settlementAt+1,"0"}}),"test",ze),"zero settled units load");
        check(zr.schedule_book().settled_work_units.count(A)==0&&zr.schedule_book().settled_work_units.at(A+1)==4,"zero settled units dropped");
        check(zr.invariant(),"zero settled units invariant");
    }
    // settlement units cannot exceed the attendance floor even when cursor allows.
    {
        Realm sd2(37,6,256);
        int sd2Region=sd2.person(A).region;
        check(sd2.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000,ActivityKind::Work,sd2Region,work_site(sd2,sd2Region)}).success,"sd2 submit");
        sd2.advance_ticks(128);
        check(sd2.schedule_book().work_present_ticks.at(A)==64&&sd2.schedule_book().settled_work_units.at(A)==4&&sd2.schedule_book().settled_day==2,"sd2 baseline");
        std::string sd2d=test_fixtures::save_as_v28(sd2.serialize("test"));
        size_t sTag=sd2d.find(" SCHEDULE_5 ");
        check(sTag!=std::string::npos,"sd2 tag present");
        std::string sPrefix=sd2d.substr(0,sTag+12),sSuffix=sd2d.substr(sTag+12);
        std::vector<std::string> st; std::istringstream sin(sSuffix); std::string stoke; while(sin>>stoke)st.push_back(stoke);
        const size_t sSettlementAt=10+17+2;
        size_t bodyRecords=0;for(uint64_t actor=1;actor<=uint64_t(sd2.count());++actor)bodyRecords+=sd2.person(actor).alive?1:0;
        check(st.size()==sSettlementAt+51+9+16+2+2+2+4*sd2.land_holdings().size()+8*bodyRecords+2+3+2+6+2*27*(sd2.regions()+1)+5+3+5,"sd2 schedule, site production, config, body, household, land title, land-use, estate, parentage, market, route, and empty-shipment header token count");
        std::vector<std::string> stoks=st; stoks[sSettlementAt+1]="5";
        std::ostringstream sj; for(size_t i=0;i<stoks.size();++i){ if(i)sj<<' '; sj<<stoks[i]; }
        reject(sPrefix+sj.str(),"settled units beyond attendance","Save schedule settled attendance");
    }
    // truncated tail (not half world)
    {
        Realm rtr(2,1,128); auto rtrb=rtr.fingerprint(); std::string etr;
        check(!rtr.deserialize(csd.substr(0,csd.size()-5),"test",etr)&&rtr.fingerprint()==rtrb,"truncated tail reject");
    }
    // sticky guards
    Realm sg(37,6,256);
    check(sg.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"guard submit");
    check(sg.schedule_book().sessions.at(A).size()==1,"guard session");
    auto sgc=sg.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Cancel,sg.schedule_book().sessions.at(A).front().id,0,0,0,0});
    (void)sgc;
    check(sg.timeline_contract(),"contract sticky after cancel");
    check(sg.candidates(A).size()==realm_actions,"policy candidates remain available after timeline contract");
    check(sg.mask(A).size()==realm_actions,"policy mask remains available after timeline contract");
    check(sg.observation(A).size()==realm_obs,"policy observation remains available after timeline contract");

    std::cout<<"schedule checks="<<checks<<"\n";
    return 0;
}catch(const std::exception&e){std::cerr<<"FAILED after "<<checks<<" checks: "<<e.what()<<"\n";return 1;}}
