#include "polyec/realm.hpp"
#include "polyec/schedule.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <set>
#include <vector>
using namespace polyec;
int checks=0;
void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
int main(){try{
    const uint64_t A=129;
    Realm w(37,6,256);
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
    check(ov.person(A).job==homeOv,"busy job preserved");

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
    check(sf.requested==0,"future zero counters");
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
        check(pa.person(id).job==pb.person(id).job,"paired job");
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

    // corrupt saves with canonical SCHEDULE_1 section
    Realm cs(37,6,256);
    check(cs.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"corrupt submit");
    check(cs.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Submit,0,64,128,0,500}).success,"corrupt submit 2");
    cs.advance_ticks(10);
    std::string csd=cs.serialize("test"),cse;
    size_t tagPos=csd.find(" SCHEDULE_1 ");
    check(tagPos!=std::string::npos,"schedule tag present");
    std::string prefix=csd.substr(0,tagPos+12); // includes trailing space after tag
    std::string suffix=csd.substr(tagPos+12);
    std::vector<std::string> toks;
    std::istringstream tss(suffix);
    std::string tok; while(tss>>tok) toks.push_back(tok);
    check(toks.size()==4+2*14,"schedule token count");
    check(toks[0]=="1","base timeline flag");
    check(toks[1]=="10","base cursor");
    check(toks[3]=="2","base session count");
    auto mutate=[&](const std::vector<std::pair<int,std::string>>&changes)->std::string{
        std::vector<std::string> t=toks;
        for(auto const&c:changes)t[c.first]=c.second;
        std::ostringstream j; for(size_t i=0;i<t.size();++i){ if(i)j<<' '; j<<t[i]; }
        return prefix+j.str();
    };
    auto reject=[&](const std::string&d,const char*label,const char*reason){
        Realm r(2,1,128); auto rb=r.fingerprint(); std::string e;
        bool ok=!r.deserialize(d,"test",e);
        check(ok&&r.fingerprint()==rb,label);
        if(reason) check(e.find(reason)!=std::string::npos,label);
    };
    // cursor64 with day0 (mutate cursor token)
    reject(mutate({{1,"64"}}),"cursor day mismatch","Save schedule day mismatch");
    // timeline0 with sessions
    reject(mutate({{0,"0"}}),"flag false with sessions","Save schedule timeline flag");
    // duplicate id between two sessions
    reject(mutate({{4+14,toks[4]}}),"duplicate id","Save schedule id");
    // actor0
    reject(mutate({{4+1,"0"}}),"actor0","Save schedule actor");
    // actor out of range
    reject(mutate({{4+1,"999999"}}),"actor oor","Save schedule actor");
    // rate1001
    reject(mutate({{4+5,"1001"}}),"bad rate","Save schedule rate");
    // requested/delivered both INT64_MAX, unmet0 => processed mismatch no overflow
    reject(mutate({{4+6,"9223372036854775807"},{4+7,"9223372036854775807"},{4+8,"0"}}),"huge counters","Save schedule processed mismatch");
    // impossible requested1 at elapsed10
    reject(mutate({{4+6,"1"},{4+7,"0"},{4+8,"1"},{4+9,"1"},{4+10,"0"}}),"impossible requested","Save schedule processed mismatch");
    // next id collision
    reject(mutate({{2,toks[4]}}),"next id collision","Save schedule next id");
    // truncated tail (not half world)
    Realm rtr(2,1,128); auto rtrb=rtr.fingerprint(); std::string etr;
    check(!rtr.deserialize(csd.substr(0,csd.size()-5),"test",etr)&&rtr.fingerprint()==rtrb,"truncated tail reject");
    // sticky guards
    Realm sg(37,6,256);
    check(sg.execute(ScheduleCommand{A,1,ScheduleCommand::Operation::Submit,0,0,64,0,1000}).success,"guard submit");
    check(sg.schedule_book().sessions.at(A).size()==1,"guard session");
    auto sgc=sg.execute(ScheduleCommand{A,2,ScheduleCommand::Operation::Cancel,sg.schedule_book().sessions.at(A).front().id,0,0,0,0});
    (void)sgc;
    check(sg.timeline_contract(),"contract sticky after cancel");
    bool guardThrew=false;
    try{sg.candidates(A);}catch(std::runtime_error&e){guardThrew=std::string(e.what()).find("timeline contract")!=std::string::npos;}
    check(guardThrew,"candidates guard");
    guardThrew=false;
    try{sg.mask(A);}catch(std::runtime_error&e){guardThrew=std::string(e.what()).find("timeline contract")!=std::string::npos;}
    check(guardThrew,"mask guard");
    guardThrew=false;
    try{sg.observation(A);}catch(std::runtime_error&e){guardThrew=std::string(e.what()).find("timeline contract")!=std::string::npos;}
    check(guardThrew,"observation guard");

    std::cout<<"schedule checks="<<checks<<"\n";
    return 0;
}catch(const std::exception&e){std::cerr<<"FAILED: "<<e.what()<<"\n";return 1;}}
