#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"
#include "polyec/investigation_section_codec.hpp"
#include "polyec/referrals_section_codec.hpp"
#include "polyec/loan_court_extension_codec.hpp"
#include "polyec/person_court_extension_codec.hpp"
#include "polyec/project_court_extension_codec.hpp"
#include "polyec/province_court_extension_codec.hpp"
#include "polyec/proposal_court_extension_codec.hpp"
#include "polyec/court_scalars_codec.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
using namespace polyec;
int n=0;void test(bool b,const char*s){++n;if(!b)throw std::runtime_error(s);}
Realm fixture(int card,bool activate=true){for(int seed=1;seed<2000;++seed){Realm w(seed,6,256);auto d=w.draft();if(std::find(d.cards.begin(),d.cards.end(),card)!=d.cards.end()){if(activate)test(w.choose(card),"choose known augment");return w;}}throw std::runtime_error("draft coverage");}
void act(Realm&w,uint64_t id,Candidate c){const auto result=w.execute({id,w.person(id).decisions+1,c});if(!result.success){std::ostringstream message;message<<"court action rejected: actor="<<id<<" intent="<<int(c.kind)<<" target="<<c.target<<" good="<<c.good<<" amount="<<c.amount<<" reason="<<result.reason;throw std::runtime_error(message.str());}test(w.invariant(),"court ledger reconciles");}
void roundtrip(Realm&w){std::string error;Realm r(1,1,128);test(r.deserialize(w.serialize("court"),"court",error),"court save parses");test(r.fingerprint()==w.fingerprint(),"court save exact");w.advance(7);r.advance(7);test(r.fingerprint()==w.fingerprint(),"court continuation exact");}
namespace polyec {
struct RealmObservationTestAccess {
    static void grant_test_good(Realm& world,uint64_t person,GoodId good,int64_t amount){world.people_[size_t(person-1)].inventory[size_t(good)]+=amount;world.opening_[size_t(good)]+=amount;}
    static void mark_dead(Realm& world,uint64_t person){world.people_[size_t(person-1)].health=-1;}
    static void run_people_phase(Realm& world){world.day_people();}
    static void set_lineal_kinship(Realm& world,uint64_t grandparent,uint64_t parent,uint64_t grandchild){
        std::erase_if(world.edges_,[&](const Edge& edge){
            return edge.from==grandparent||edge.to==grandparent||
                   edge.from==parent||edge.to==parent||
                   edge.from==grandchild||edge.to==grandchild;
        });
        world.edges_.push_back({parent,grandparent,RelationKind::ParentChild,false});
        world.edges_.push_back({grandchild,parent,RelationKind::ParentChild,false});
        world.rebuild_kinship_indices();
    }
    static void set_common_parent_siblings(Realm& world,uint64_t commonParent,uint64_t first,uint64_t second){
        std::erase_if(world.edges_,[&](const Edge& edge){
            return edge.from==commonParent||edge.to==commonParent||
                   edge.from==first||edge.to==first||edge.from==second||edge.to==second;
        });
        world.edges_.push_back({first,commonParent,RelationKind::ParentChild,false});
        world.edges_.push_back({second,commonParent,RelationKind::ParentChild,false});
        world.rebuild_kinship_indices();
    }
    static void add_parent_child_relation(Realm& world,uint64_t child,uint64_t parent){
        world.edges_.push_back({child,parent,RelationKind::ParentChild,false});
        world.index_kinship_edge(world.edges_.back());
    }
};
}
int main(){try{
for(int card=0;card<12;++card){auto w=fixture(card);test(w.invariant(),"opening court ledger");auto region=w.person(1).region;auto total=w.total_money();
 if(card==0||card==7){auto plain=fixture(card,false);test(w.observation(1)[105]<plain.observation(1)[105],"augment reduces public report latency against identical opening");auto before=w.view().facts.size();w.advance(30);bool inventory=false;for(auto&f:w.view().facts)inventory|=f.kind==108;test(!inventory,"inventory report has genuine delay");w.advance(12);for(auto&f:w.view().facts)inventory|=f.kind==108;test(inventory,"inventory report delivered");(void)before;}
 if(card==1||card==10){uint64_t teacher=0;for(uint64_t id=15;id<=256;++id){auto&p=w.person(id);if(p.family==Family::Civilian&&p.region==region&&p.age_days>=18*365&&p.skill>=20){teacher=id;break;}}test(teacher>0,"existing teacher");act(w,teacher,{Intent::Work,region,0,0,30});test(w.person(teacher).work_assignment.job_kind==1&&w.job_type(teacher)==JobType::Teacher&&w.view().provinces[region].teachers==1,"teacher starts with a daily counter");act(w,1,{Intent::Project,region,card==1?2:3,120,60});auto plain=fixture(card,false);act(plain,teacher,{Intent::Work,region,0,0,30});act(plain,1,{Intent::Project,region,card==1?2:3,120,60});test(w.projects().back().due<plain.projects().back().due,"specialized training improves identical funded project");if(card==10){act(w,1,{Intent::Project,region,3,120,60});auto before=w.fingerprint();test(!w.execute({1,w.person(1).decisions+1,{Intent::Project,region,3,120,60}}).success&&before==w.fingerprint(),"third teacher course rejected atomically");}}
 if(card==2){act(w,1,w.candidates(1)[14]);act(w,1,w.candidates(1)[22]);test(w.mask(1)[22]==0,"mediation cooldown");w.advance(12);act(w,2,w.candidates(2)[29]);}
 if(card==3){test(w.view().provinces[region].food_capacity>w.view().provinces[region].residents*10,"three month capacity");}
 if(card==4){test(w.royal_reference()>0&&w.royal_funds()[0]==w.royal_reference()/2,"reference-funded credit offer");auto c=w.candidates(1)[17];if(c.amount>0){auto pool=w.royal_funds()[0];act(w,1,c);test(w.loans().back().royal_pool&&w.loans().back().due==1825&&w.royal_funds()[0]==pool-c.amount,"five year pool debt");act(w,1,{Intent::Repay,region,0,c.amount});test(w.royal_funds()[0]==pool,"repayment to creditor pool");}}
 if(card==5){act(w,1,{Intent::Project,region,0,200,60});test(w.projects().back().engineer,"physical engineer assignment");auto plain=fixture(card,false);act(plain,1,{Intent::Project,region,0,200,60});test(w.projects().back().due<plain.projects().back().due,"engineer improves productivity without removing material cost");}
 if(card==6||card==11){int candidates=0;for(uint64_t id=1;id<=256;++id)candidates+=w.person(id).candidate;test(candidates==(card==6?3:2),"existing applicants not new people");test(w.count()==256,"no duplicated persons");std::vector<uint64_t> applicants;for(uint64_t id=1;id<=256;++id)if(w.person(id).candidate)applicants.push_back(id);for(auto id:applicants)act(w,id,{Intent::Work,w.person(id).region,0,0,30});std::vector<int> employed_before_appointment;for(auto&v:w.view().provinces)employed_before_appointment.push_back(v.employed);act(w,1,w.candidates(1)[15]);uint64_t hired=0;for(uint64_t id=1;id<=256;++id)if(w.person(id).office_appointment.hired_by==1)hired=id;test(hired>0&&w.person(hired).family==Family::Governance&&!w.person(hired).taxing_authority,"funded advisor role with restricted authority");test(w.person(hired).work_assignment.job==-1&&w.person(hired).busy_until==0&&w.person(hired).school_days==0,"appointment clears prior work fields");test(w.view().provinces[w.person(hired).region].employed==employed_before_appointment[size_t(w.person(hired).region)]-1,"appointment clears the selected worker counter");auto notice_before=w.fingerprint();test(!w.execute({hired,w.person(hired).decisions+1,{Intent::Resign,w.person(hired).region,0,0}}).success&&w.fingerprint()==notice_before,"advisor cannot resign before notice");w.advance(30);const auto cash_before_resign=w.person(hired).cash;const auto money_before_resign=w.total_money();act(w,hired,w.candidates(hired)[31]);test(w.person(hired).family==Family::Civilian&&!w.person(hired).office_appointment.office&&w.person(hired).office_appointment.hired_by==0,"advisor resigns and releases the appointment after notice");test(w.person(hired).cash==cash_before_resign&&w.total_money()==money_before_resign,"voluntary resignation leaves private cash and total money unchanged");if(card==6){Realm failed=fixture(6);int fr=failed.person(1).region;act(failed,1,failed.candidates(1)[15]);uint64_t unpaid=0;for(uint64_t id=1;id<=256;++id)if(failed.person(id).office_appointment.hired_by==1)unpaid=id;test(unpaid>0,"office salary fixture");failed.advance(29);test(failed.view().provinces[fr].treasury>0,"salary fixture treasury remains funded before drain");act(failed,1,{Intent::Aid,fr,0,failed.view().provinces[fr].treasury});test(failed.view().provinces[fr].treasury==0,"salary fixture treasury drained");failed.advance();bool failed_event=false;for(auto&f:failed.view(unpaid).facts)if(f.kind==110)failed_event=true;test(failed_event&&!failed.person(unpaid).office_appointment.office&&failed.person(unpaid).office_appointment.hired_by==0&&failed.person(unpaid).family==Family::Civilian,"unpaid office salary ends the appointment");test(failed.person(unpaid).office_appointment.salary_due==30&&failed.person(unpaid).office_appointment.office_notice==30,"salary failure leaves timers stale");}}
 if(card==7){RealmObservationTestAccess::grant_test_good(w,1,GoodId::Paper,1);act(w,1,w.candidates(1)[13]);RealmObservationTestAccess::grant_test_good(w,1,GoodId::Paper,1);act(w,1,w.candidates(1)[13]);test(w.investigations().size()==2&&w.mask(1)[13]==0,"two simultaneous investigations");}
 if(card==8){auto pool=w.royal_funds()[1];auto treasury=w.view().provinces[region].treasury;act(w,1,{Intent::Project,region,0,200,60});auto match=std::min<int64_t>(pool,100);test(w.royal_funds()[1]==pool-match&&w.view().provinces[region].treasury==treasury-200+match,"restricted half matching");auto remaining=w.royal_funds()[1];act(w,1,{Intent::Aid,region,0,1});test(w.royal_funds()[1]==remaining,"relief cannot spend restricted fund");}
 if(card==9){act(w,1,w.candidates(1)[21]);act(w,2,w.candidates(2)[28]);roundtrip(w);act(w,3,w.candidates(3)[29]);test(w.projects().empty()&&w.balances()[8]==0,"joint rejection refunds escrow and makes no fake project");act(w,1,w.candidates(1)[21]);for(uint64_t id:{2,3,4})act(w,id,w.candidates(id)[28]);test(w.projects().size()==3,"three independent funded acceptances");}
 test(w.total_money()==total,"augment money conserved after opening");roundtrip(w);
}
    // R4 Investigation codec focused checks.
    {
        using namespace polyec::detail;
        std::vector<Investigation> existing{Investigation{7,8,9,10,11,false}};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Investigation> out=existing;
            bool threw=false; std::string msg;
            try { read_investigation_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            test(out.size()==existing.size() &&
                 (!existing.empty() &&
                  out[0].id==existing[0].id &&
                  out[0].owner==existing[0].owner &&
                  out[0].cause==existing[0].cause &&
                  out[0].region==existing[0].region &&
                  out[0].due==existing[0].due &&
                  out[0].completed==existing[0].completed),
                 "investigation codec failure preserves output investigation");
            return {threw,msg};
        };
        auto r1=read_throws("1000001 7 8 9 10 11 0");
        test(r1.first && r1.second=="Save court fields","investigation codec excessive count throws court error");
        auto r2=read_throws("not-a-number");
        test(r2.first && r2.second=="Save court fields","investigation codec bad count throws court error");
        auto r3=read_throws("1 7 8 9 10 11");
        test(r3.first && r3.second=="Save court fields","investigation codec truncated record throws court error");
        {
            std::ostringstream out;
            write_investigation_section(out,existing);
            test(out.str()=="1 7 8 9 10 11 0 ",
                 "investigation codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Investigation> parsed;
            read_investigation_section(in,parsed);
            test(parsed.size()==1 && parsed[0].id==7 && parsed[0].owner==8 &&
                 parsed[0].cause==9 && parsed[0].region==10 &&
                 parsed[0].due==11 && parsed[0].completed==false,
                 "investigation codec valid record round trips");
        }
        {
            std::vector<Investigation> rows{Investigation{7,8,9,10,11,false},Investigation{12,13,14,15,16,true}};
            std::ostringstream out;
            write_investigation_section(out,rows);
            std::istringstream in(out.str());
            std::vector<Investigation> parsed;
            read_investigation_section(in,parsed);
            test(parsed.size()==2 && parsed[0].id==7 && parsed[1].id==12,
                 "investigation codec preserves input order");
        }
    }
    // R4 Proposal court extension codec focused checks.
    {
        using namespace polyec::detail;
        Proposal existing{};
        existing.id=1; existing.from=2; existing.to=3; existing.amount=4; existing.due=5; existing.resolved=true;
        existing.kind=1; existing.ready=12; existing.escrow=13; existing.public_source=true; existing.mediation=false; existing.source_region=14; existing.cause=15;
        existing.parties={16,17,18}; existing.regions={1,2,3}; existing.pledged={false,true,false};
        std::vector<Proposal> existingVec{existing};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Proposal> out=existingVec;
            bool threw=false; std::string msg;
            try { read_proposal_court_extension_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            test(out.size()==existingVec.size(),
                 "proposal court extension codec failure preserves count");
            if(!existingVec.empty() && !out.empty()){
                auto&o=out[0]; auto&e=existingVec[0];
                test(o.id==e.id && o.from==e.from && o.to==e.to && o.amount==e.amount && o.due==e.due && o.resolved==e.resolved,
                     "proposal court extension codec failure preserves primary fields");
                test(o.kind==e.kind && o.ready==e.ready && o.escrow==e.escrow && o.public_source==e.public_source && o.mediation==e.mediation && o.source_region==e.source_region && o.cause==e.cause,
                     "proposal court extension codec failure preserves extension fields");
                for(int i=0;i<3;++i){ test(o.parties[i]==e.parties[i],"proposal court extension parties preserved"); test(o.regions[i]==e.regions[i],"proposal court extension regions preserved"); test(o.pledged[i]==e.pledged[i],"proposal court extension pledged preserved"); }
            }
            return {threw,msg};
        };
        auto r1=read_throws("1 12 13 1 0 14 15 16 17 18 1 2 3 0 1");
        test(r1.first && r1.second=="Save court fields","proposal court extension codec truncated pledged throws court error");
        auto r2=read_throws("1 12 13 1 0 14 15 16 17 18 1 2");
        test(r2.first && r2.second=="Save court fields","proposal court extension codec truncated regions throws court error");
        auto r3=read_throws("1 12 13 1 0 14 15 16 17");
        test(r3.first && r3.second=="Save court fields","proposal court extension codec truncated parties throws court error");
        auto r4=read_throws("not-a-number");
        test(r4.first && r4.second=="Save court fields","proposal court extension codec bad input throws court error");
        {
            std::ostringstream out;
            write_proposal_court_extension_section(out,existingVec);
            test(out.str()=="1 12 13 1 0 14 15 16 17 18 1 2 3 0 1 0 0 ",
                 "proposal court extension codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Proposal> parsed=existingVec;
            read_proposal_court_extension_section(in,parsed);
            test(parsed.size()==1 && parsed[0].kind==1 && parsed[0].ready==12 &&
                 parsed[0].escrow==13 && parsed[0].public_source==true &&
                 parsed[0].mediation==false && parsed[0].source_region==14 &&
                 parsed[0].cause==15,
                 "proposal court extension codec valid record round trips");
            test(parsed[0].parties[0]==16 && parsed[0].parties[1]==17 && parsed[0].parties[2]==18,
                 "proposal court extension codec parties round trip");
            test(parsed[0].regions[0]==1 && parsed[0].regions[1]==2 && parsed[0].regions[2]==3,
                 "proposal court extension codec regions round trip");
            test(parsed[0].pledged[0]==false && parsed[0].pledged[1]==true && parsed[0].pledged[2]==false,
                 "proposal court extension codec pledged round trip");
        }
        {
            Proposal second=existing;
            second.kind=2; second.ready=22; second.escrow=23; second.public_source=false; second.mediation=true; second.source_region=24; second.cause=25;
            second.parties={26,27,28}; second.regions={4,5,6}; second.pledged={true,false,true};
            std::vector<Proposal> rows{existing,second};
            std::ostringstream out;
            write_proposal_court_extension_section(out,rows);
            std::istringstream in(out.str());
            std::vector<Proposal> parsed=rows;
            read_proposal_court_extension_section(in,parsed);
            test(parsed.size()==2 && parsed[0].kind==1 && parsed[1].kind==2 &&
                 parsed[0].parties[0]==16 && parsed[1].parties[0]==26,
                 "proposal court extension codec preserves input row order");
        }
    }
    // R4 Loan court extension codec focused checks.
    {
        using namespace polyec::detail;
        Loan existing{};
        existing.id=1; existing.debtor=2; existing.creditor=3; existing.due=4; existing.principal=5; existing.interest=6; existing.paid=7; existing.defaulted=true;
        existing.royal_pool=true; existing.cause=15;
        std::vector<Loan> existingVec{existing};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Loan> out=existingVec;
            bool threw=false; std::string msg;
            try { read_loan_court_extension_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            test(out.size()==existingVec.size(),
                 "loan court extension codec failure preserves count");
            if(!existingVec.empty() && !out.empty()){
                auto&o=out[0]; auto&e=existingVec[0];
                test(o.id==e.id && o.debtor==e.debtor && o.creditor==e.creditor && o.due==e.due && o.principal==e.principal && o.interest==e.interest && o.paid==e.paid && o.defaulted==e.defaulted,
                     "loan court extension codec failure preserves primary fields");
                test(o.royal_pool==e.royal_pool && o.cause==e.cause,
                     "loan court extension codec failure preserves extension fields");
            }
            return {threw,msg};
        };
        auto r1=read_throws("1");
        test(r1.first && r1.second=="Save court fields","loan court extension codec truncated extension throws court error");
        auto r2=read_throws("not-a-number");
        test(r2.first && r2.second=="Save court fields","loan court extension codec bad input throws court error");
        {
            std::ostringstream out;
            write_loan_court_extension_section(out,existingVec);
            test(out.str()=="1 15 ","loan court extension codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Loan> parsed=existingVec;
            read_loan_court_extension_section(in,parsed);
            test(parsed.size()==1 && parsed[0].royal_pool==true && parsed[0].cause==15,
                 "loan court extension codec valid record round trips");
        }
        {
            Loan second=existing;
            second.royal_pool=false; second.cause=25;
            std::vector<Loan> rows{existing,second};
            std::ostringstream out;
            write_loan_court_extension_section(out,rows);
            std::istringstream in(out.str());
            std::vector<Loan> parsed=rows;
            read_loan_court_extension_section(in,parsed);
            test(parsed.size()==2 && parsed[0].royal_pool==true && parsed[1].royal_pool==false,
                 "loan court extension codec preserves input row order");
        }
    }
    // R4 Project court extension codec focused checks.
    {
        using namespace polyec::detail;
        Project existing{};
        existing.id=1; existing.owner=2; existing.region=3; existing.due=4; existing.kind=5; existing.budget=6; existing.completed=true;
        existing.engineer=true; existing.stalled=false; existing.cause=15;
        std::vector<Project> existingVec{existing};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Project> out=existingVec;
            bool threw=false; std::string msg;
            try { read_project_court_extension_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            test(out.size()==existingVec.size(),
                 "project court extension codec failure preserves count");
            if(!existingVec.empty() && !out.empty()){
                auto&o=out[0]; auto&e=existingVec[0];
                test(o.id==e.id && o.owner==e.owner && o.region==e.region && o.due==e.due && o.kind==e.kind && o.budget==e.budget && o.completed==e.completed,
                     "project court extension codec failure preserves primary fields");
                test(o.engineer==e.engineer && o.stalled==e.stalled && o.cause==e.cause,
                     "project court extension codec failure preserves extension fields");
            }
            return {threw,msg};
        };
        auto r1=read_throws("1");
        test(r1.first && r1.second=="Save court fields","project court extension codec truncated stalled throws court error");
        auto r2=read_throws("1 0");
        test(r2.first && r2.second=="Save court fields","project court extension codec truncated cause throws court error");
        auto r3=read_throws("not-a-number");
        test(r3.first && r3.second=="Save court fields","project court extension codec bad input throws court error");
        {
            std::ostringstream out;
            write_project_court_extension_section(out,existingVec);
            test(out.str()=="1 0 15 ","project court extension codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Project> parsed=existingVec;
            read_project_court_extension_section(in,parsed);
            test(parsed.size()==1 && parsed[0].engineer==true && parsed[0].stalled==false && parsed[0].cause==15,
                 "project court extension codec valid record round trips");
        }
        {
            Project second=existing;
            second.engineer=false; second.stalled=true; second.cause=25;
            std::vector<Project> rows{existing,second};
            std::ostringstream out;
            write_project_court_extension_section(out,rows);
            std::istringstream in(out.str());
            std::vector<Project> parsed=rows;
            read_project_court_extension_section(in,parsed);
            test(parsed.size()==2 && parsed[0].engineer==true && parsed[1].engineer==false &&
                 parsed[0].stalled==false && parsed[1].stalled==true,
                 "project court extension codec preserves input row order");
        }
    }
    // R4 Person court extension codec focused checks.
    {
        using namespace polyec::detail;
        Person existing{};
        existing.id=42; existing.region=3; existing.age_days=10; existing.health=90; existing.skill=7; existing.cash=123; existing.alive=true; existing.family=Family::Civilian;
        existing.work_assignment.job=5; existing.busy_until=6; existing.destination=7; existing.goal=8; existing.school_days=9; existing.social=10;
        existing.tax_remainder=11; existing.organization=12; existing.work_assignment.job_kind=1; existing.taxing_authority=true; existing.sovereign=false; existing.nutrition=95;
        existing.stats.values={1,2,3,4,5,6}; existing.inventory={7,8,9,10,11,12,13};
        existing.household_membership.household_id=2;
        existing.decisions=100; existing.sampling=200;
        existing.candidate=true; existing.special_advisor=false; existing.last_decision_day=7;
        existing.office_appointment.office=true; existing.office_appointment.hired_by=2; existing.office_appointment.salary_due=30; existing.office_appointment.office_notice=5;
        std::vector<Person> existingVec{existing};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Person> out=existingVec;
            bool threw=false; std::string msg;
            try { read_person_court_extension_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            test(out.size()==existingVec.size(),"person court extension codec failure preserves count");
            if(!existingVec.empty() && !out.empty()){
                auto&o=out[0]; auto&e=existingVec[0];
                test(o.id==e.id && o.region==e.region && o.age_days==e.age_days && o.health==e.health && o.skill==e.skill && o.cash==e.cash && o.alive==e.alive,
                     "person court extension codec failure preserves primary fields");
                test(o.work_assignment.job==e.work_assignment.job && o.busy_until==e.busy_until && o.destination==e.destination && o.goal==e.goal && o.school_days==e.school_days && o.social==e.social,
                     "person court extension codec failure preserves more primary fields");
                test(o.tax_remainder==e.tax_remainder && o.organization==e.organization && o.work_assignment.job_kind==e.work_assignment.job_kind && o.taxing_authority==e.taxing_authority && o.sovereign==e.sovereign && o.nutrition==e.nutrition,
                     "person court extension codec failure preserves authority/nutrition fields");
                test(o.household_membership.household_id==e.household_membership.household_id && o.decisions==e.decisions && o.sampling==e.sampling,
                     "person court extension codec failure preserves household/decision fields");
                for(int i=0;i<6;++i) test(o.stats.values[i]==e.stats.values[i],"person court extension stats preserved");
                for(int i=0;i<7;++i) test(o.inventory[i]==e.inventory[i],"person court extension inventory preserved");
                test(o.candidate==e.candidate && o.office_appointment.office==e.office_appointment.office && o.special_advisor==e.special_advisor && o.office_appointment.hired_by==e.office_appointment.hired_by && o.office_appointment.salary_due==e.office_appointment.salary_due && o.office_appointment.office_notice==e.office_appointment.office_notice && o.last_decision_day==e.last_decision_day,
                     "person court extension codec failure preserves extension fields");
            }
            return {threw,msg};
        };
        auto r1=read_throws("1 1 0 2 30 5");
        test(r1.first && r1.second=="Save court fields","person court extension codec truncated last field throws court error");
        auto r2=read_throws("1 1 0 2 30");
        test(r2.first && r2.second=="Save court fields","person court extension codec truncated multiple fields throws court error");
        auto r3=read_throws("not-a-number");
        test(r3.first && r3.second=="Save court fields","person court extension codec bad input throws court error");
        {
            std::ostringstream out;
            write_person_court_extension_section(out,existingVec);
            test(out.str()=="1 1 0 2 30 5 7 ","person court extension codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Person> parsed=existingVec;
            read_person_court_extension_section(in,parsed);
            test(parsed.size()==1 && parsed[0].candidate==true && parsed[0].office_appointment.office==true && parsed[0].special_advisor==false && parsed[0].office_appointment.hired_by==2 && parsed[0].office_appointment.salary_due==30 && parsed[0].office_appointment.office_notice==5 && parsed[0].last_decision_day==7,
                  "person court extension codec valid record round trips");
        }
        {
            Person second=existing;
            second.candidate=false; second.office_appointment.office=false; second.special_advisor=true; second.office_appointment.hired_by=3; second.office_appointment.salary_due=40; second.office_appointment.office_notice=6; second.last_decision_day=8;
            std::vector<Person> rows{existing,second};
            std::ostringstream out;
            write_person_court_extension_section(out,rows);
            std::istringstream in(out.str());
            std::vector<Person> parsed=rows;
            read_person_court_extension_section(in,parsed);
            test(parsed.size()==2 && parsed[0].candidate==true && parsed[1].candidate==false &&
                 parsed[0].office_appointment.hired_by==2 && parsed[1].office_appointment.hired_by==3 &&
                 parsed[0].last_decision_day==7 && parsed[1].last_decision_day==8,
                 "person court extension codec preserves input row order");
        }
    }
    // R4 Province court extension codec focused checks.
    {
        using namespace polyec::detail;
        Province existing{};
        existing.treasury=10; existing.firm=20; existing.stock[0]=1; existing.stock[6]=6; existing.price[0]=2; existing.price[6]=9; existing.tax=3; existing.school=4; existing.school_used=5; existing.jobs=6; existing.employed=7; existing.road=1; existing.unrest=8; existing.teachers=9; existing.charter=true; existing.residents=10; existing.health_sum=11; existing.sales_tax_remainder=12;
        existing.tax_receipts=20; existing.food_capacity=30; existing.teacher_skill_sum=50;
        std::vector<Province> existingVec{existing};
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<Province> out=existingVec;
            bool threw=false; std::string msg;
            try { read_province_court_extension_section(in,out); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            test(out.size()==existingVec.size(),"province court extension codec failure preserves count");
            if(!existingVec.empty() && !out.empty()){
                auto&o=out[0]; auto&e=existingVec[0];
                test(o.treasury==e.treasury && o.firm==e.firm,
                     "province court extension codec failure preserves treasury/firm");
                for(int i=0;i<7;++i) test(o.stock[i]==e.stock[i],"province court extension stock preserved");
                for(int i=0;i<7;++i) test(o.price[i]==e.price[i],"province court extension price preserved");
                test(o.tax==e.tax && o.school==e.school && o.school_used==e.school_used && o.jobs==e.jobs && o.employed==e.employed && o.road==e.road && o.unrest==e.unrest && o.teachers==e.teachers && o.charter==e.charter && o.residents==e.residents && o.health_sum==e.health_sum && o.sales_tax_remainder==e.sales_tax_remainder,
                     "province court extension codec failure preserves primary fields");
                test(o.tax_receipts==e.tax_receipts && o.food_capacity==e.food_capacity && o.teacher_skill_sum==e.teacher_skill_sum,
                     "province court extension codec failure preserves extension fields");
            }
            return {threw,msg};
        };
        auto r1=read_throws("20");
        test(r1.first && r1.second=="Save court fields","province court extension codec truncated food_capacity throws court error");
        auto r2=read_throws("20 30");
        test(r2.first && r2.second=="Save court fields","province court extension codec truncated teacher_skill_sum throws court error");
        auto r3=read_throws("not-a-number");
        test(r3.first && r3.second=="Save court fields","province court extension codec bad input throws court error");
        {
            std::ostringstream out;
            write_province_court_extension_section(out,existingVec);
            test(out.str()=="20 30 50 ","province court extension codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<Province> parsed=existingVec;
            read_province_court_extension_section(in,parsed);
            test(parsed.size()==1 && parsed[0].tax_receipts==20 && parsed[0].food_capacity==30 && parsed[0].teacher_skill_sum==50,
                  "province court extension codec valid record round trips");
        }
        {
            Province second=existing;
            second.tax_receipts=21; second.food_capacity=31; second.teacher_skill_sum=51;
            std::vector<Province> rows{existing,second};
            std::ostringstream out;
            write_province_court_extension_section(out,rows);
            std::istringstream in(out.str());
            std::vector<Province> parsed=rows;
            read_province_court_extension_section(in,parsed);
            test(parsed.size()==2 && parsed[0].tax_receipts==20 && parsed[1].tax_receipts==21 &&
                 parsed[0].teacher_skill_sum==50 && parsed[1].teacher_skill_sum==51,
                  "province court extension codec preserves input row order");
        }
    }
    // R4 Court scalars codec focused checks.
    {
        using namespace polyec::detail;
        int64_t rr=100, cp=20, rf=30; int mr=456, bo=2; bool pact=true, rs=false;
        auto read_throws=[&](const std::string& text)->std::pair<bool,std::string>{
            std::istringstream in(text);
            int64_t o_rr=rr, o_cp=cp, o_rf=rf; int o_mr=mr, o_bo=bo; bool o_pact=pact, o_rs=rs;
            bool threw=false; std::string msg;
            try { read_court_scalars(in, o_rr, o_cp, o_rf, o_mr, o_bo, o_pact, o_rs); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            test(o_rr==rr && o_cp==cp && o_rf==rf && o_mr==mr && o_bo==bo && o_pact==pact && o_rs==rs,
                 "court scalars codec failure preserves output values");
            return {threw,msg};
        };
        auto r1=read_throws("100");
        test(r1.first && r1.second=="Save court fields","court scalars codec one token throws court error");
        auto r2=read_throws("100 20");
        test(r2.first && r2.second=="Save court fields","court scalars codec two tokens throws court error");
        auto r3=read_throws("100 20 30");
        test(r3.first && r3.second=="Save court fields","court scalars codec three tokens throws court error");
        auto r4=read_throws("100 20 30 456");
        test(r4.first && r4.second=="Save court fields","court scalars codec four tokens throws court error");
        auto r5=read_throws("100 20 30 456 2");
        test(r5.first && r5.second=="Save court fields","court scalars codec five tokens throws court error");
        auto r6=read_throws("100 20 30 456 2 1");
        test(r6.first && r6.second=="Save court fields","court scalars codec six tokens throws court error");
        auto rbad=read_throws("not-a-number");
        test(rbad.first && rbad.second=="Save court fields","court scalars codec bad input throws court error");
        {
            std::ostringstream out;
            write_court_scalars(out, 100, 20, 30, 456, 2, true, false);
            test(out.str()=="100 20 30 456 2 1 0 ","court scalars codec canonical writer string");
            std::istringstream in(out.str());
            int64_t a=0,b=0,c=0; int d=0,e=0; bool f=false,g=false;
            read_court_scalars(in, a,b,c,d,e,f,g);
            test(a==100 && b==20 && c==30 && d==456 && e==2 && f==true && g==false,
                 "court scalars codec valid round trips");
        }
    }
    // R4 Referrals codec focused checks.
    {
        using namespace polyec::detail;
        std::vector<uint64_t> existing{7,8,9};
        auto read_throws=[&](const std::string& text, size_t people_size)->std::pair<bool,std::string>{
            std::istringstream in(text);
            std::vector<uint64_t> out=existing;
            bool threw=false; std::string msg;
            try { read_referrals_section(in,out,people_size); }
            catch(const std::runtime_error&e){ threw=true; msg=e.what(); }
            test(out.size()==existing.size() &&
                 (!existing.empty() &&
                  out[0]==existing[0] && out[1]==existing[1] && out[2]==existing[2]),
                 "referrals codec failure preserves output referrals");
            return {threw,msg};
        };
        auto r1=read_throws("3 1 2 3",2);
        test(r1.first && r1.second=="Save court fields","referrals codec excessive count throws court error");
        auto r2=read_throws("not-a-number",2);
        test(r2.first && r2.second=="Save court fields","referrals codec bad count throws court error");
        auto r3=read_throws("3 7 8",10);
        test(r3.first && r3.second=="Save court fields","referrals codec truncated id stream throws court error");
        // Codec-level ID non-validation.
        {
            std::istringstream in("2 0 4");
            std::vector<uint64_t> parsed;
            read_referrals_section(in,parsed,3);
            test(parsed.size()==2 && parsed[0]==0 && parsed[1]==4,
                  "referrals codec does not validate ids");
        }
        {
            std::ostringstream out;
            write_referrals_section(out,existing);
            test(out.str()=="3 7 8 9 ","referrals codec canonical writer string");
            std::istringstream in(out.str());
            std::vector<uint64_t> parsed;
            read_referrals_section(in,parsed,9);
            test(parsed.size()==3 && parsed[0]==7 && parsed[1]==8 && parsed[2]==9,
                  "referrals codec valid round trips");
        }
        {
            std::vector<uint64_t> rows{7,8,9,10};
            std::ostringstream out;
            write_referrals_section(out,rows);
            std::istringstream in(out.str());
            std::vector<uint64_t> parsed;
            read_referrals_section(in,parsed,10);
            test(parsed.size()==4 && parsed[0]==7 && parsed[1]==8 && parsed[2]==9 && parsed[3]==10,
                  "referrals codec preserves input order");
        }
    }
    // R4 Referrals Realm-level corrupted-save checks.
    {
        for (int card : {6, 11}) {
            Realm w = fixture(card);
            std::vector<uint64_t> applicants;
            for (uint64_t id = 1; id <= 256; ++id) if (w.person(id).candidate) applicants.push_back(id);
            test(applicants.size() == (card == 6 ? 3 : 2), "referrals augment count");
            std::string data = w.serialize("court");
            std::ostringstream seq;
            seq << applicants.size();
            for (auto id : applicants) seq << ' ' << id;
            std::string needle = seq.str();
            size_t occ = 0, search = 0;
            while ((search = data.find(needle, search)) != std::string::npos) { ++occ; search += needle.size(); }
            test(occ == 1, "referrals token block isolatable");
            size_t pos = data.find(needle);
            test(pos != std::string::npos && pos < data.find(" CLOCK_1 "), "referrals block before clock");
            size_t idStart = pos + std::to_string(applicants.size()).size() + 1;
            size_t idEnd = data.find(' ', idStart);
            {
                std::string mutated = data.substr(0, idStart) + "0" + data.substr(idEnd);
                Realm r(1, 1, 128);
                auto before = r.fingerprint();
                std::string error;
                bool ok = r.deserialize(mutated, "court", error);
                test(!ok && error == "Save court fields" && r.fingerprint() == before, "referrals id 0 rejects and preserves destination");
            }
            {
                std::string mutated = data.substr(0, idStart) + "257" + data.substr(idEnd);
                Realm r(1, 1, 128);
                auto before = r.fingerprint();
                std::string error;
                bool ok = r.deserialize(mutated, "court", error);
                test(!ok && error == "Save court fields" && r.fingerprint() == before, "referrals id > people size rejects and preserves destination");
            }
        }
    }
    // M5.4 learned policy can create a real local workshop via the same core command.
    {
        Realm w(37,6,256);
        uint64_t economic=0,governor=0;
        for(uint64_t id=1;id<=uint64_t(w.count());++id){
            const auto&person=w.person(id);
            if(!economic&&person.family==Family::Economic&&person.cash>=100)economic=id;
            if(!governor&&person.family==Family::Governance&&person.taxing_authority)governor=id;
        }
        test(economic!=0&&governor!=0,"workshop policy fixture has funded economic owner and local authority");
        const auto choices=w.candidates(economic);
        const auto legal=w.mask(economic,choices);
        for(int industry=1;industry<=4;++industry)for(int tier=0;tier<3;++tier){
            const int slot=realm_establishment_candidate_begin+(industry-1)*3+tier;
            test(choices[size_t(slot)].kind==Intent::CreateEstablishment&&
                 choices[size_t(slot)].target==w.person(economic).region&&
                 choices[size_t(slot)].good==industry&&
                 choices[size_t(slot)].amount==std::array<int64_t,3>{25,50,100}[size_t(tier)]&&
                 choices[size_t(slot)].term==-1,"workshop action encodes industry, region and funding tier");
        }
        test(legal[realm_establishment_candidate_begin+8]==1.f,"funded local owner can select a 100-unit military-supply workshop");
        const auto openingCash=w.person(economic).cash;
        const auto created=w.execute({economic,w.person(economic).decisions+1,
                                      choices[size_t(realm_establishment_candidate_begin+2)]});
        test(created.success&&created.session!=0&&w.establishments().size()==1,
             "policy workshop action creates through EstablishmentCommand");
        const auto sites=w.establishments();
        const auto&site=sites.front();
        test(site.owner_kind==EstablishmentOwnerKind::Person&&site.owner_id==economic,
             "private workshop title belongs to policy actor");
        test(site.region==w.person(economic).region&&site.industry==EstablishmentIndustry::ClothWorkshop,
             "private workshop uses selected local region and industry");
        test(site.cash==100&&w.person(economic).cash==openingCash-100&&w.invariant(),
             "private workshop receives exact selected funding and conserves money");
        const auto staleBefore=w.fingerprint();
        const auto stale=w.execute({economic,w.person(economic).decisions,
                                    choices[size_t(realm_establishment_candidate_begin)]});
        test(!stale.success&&w.fingerprint()==staleBefore,"replayed workshop action rejects atomically");
        auto wrongRegion=choices[size_t(realm_establishment_candidate_begin)];
        wrongRegion.target=(w.person(economic).region+1)%(w.regions()+1);
        const auto invalidBefore=w.fingerprint();
        test(!w.execute({economic,w.person(economic).decisions+1,wrongRegion}).success&&
             w.fingerprint()==invalidBefore,"policy cannot create outside its current region");
        const auto authorityChoices=w.candidates(governor);
        const auto authorityMask=w.mask(governor,authorityChoices);
        test(authorityMask[realm_establishment_candidate_begin+2]==1.f,
             "local taxing authority may create a public workshop");
        const auto treasury=w.view().provinces[size_t(w.person(governor).region)].treasury;
        const auto publicResult=w.execute({governor,w.person(governor).decisions+1,
            authorityChoices[size_t(realm_establishment_candidate_begin+2)]});
        test(publicResult.success&&w.establishments().size()==2&&
             w.establishments().back().owner_kind==EstablishmentOwnerKind::Public&&
             w.establishments().back().cash==100&&
             w.view().provinces[size_t(w.person(governor).region)].treasury==treasury-100&&w.invariant(),
             "public workshop uses the regional treasury once");
    }
    // M5.3 marriage: consent is a proposal response, settlement escrow is
    // conserved, and a spouse edge appears only after the named acceptance.
    {
        Realm w=fixture(0);
        const auto proposerHousehold=w.household_of(1);
        const auto recipientHousehold=w.household_of(2);
        test(proposerHousehold!=recipientHousehold,"marriage fixture starts with two separate households");
        const auto fundDepositTotal=w.total_money();
        test(w.execute(HouseholdFundCommand{1,w.person(1).decisions+1,proposerHousehold,
                 HouseholdFundOperation::Deposit,7}).success&&
             w.execute(HouseholdFundCommand{2,w.person(2).decisions+1,recipientHousehold,
                 HouseholdFundOperation::Deposit,11}).success,
             "each future spouse deposits into their own household fund");
        test(w.household_fund(proposerHousehold)==7&&w.household_fund(recipientHousehold)==11&&
             w.total_money()==fundDepositTotal&&w.invariant(),
             "funding both households changes account ownership without creating money");
        const auto beforeCandidates=w.fingerprint();
        const auto marriageChoices=w.candidates(1);
        std::set<uint64_t> targetIds;
        for(int action=realm_marriage_candidate_begin;action<realm_marriage_candidate_begin+realm_marriage_candidate_count;++action){
            const auto&candidate=marriageChoices[size_t(action)];
            if(candidate.kind==Intent::Marry)targetIds.insert(uint64_t(candidate.target));
        }
        test(targetIds.size()==realm_marriage_candidate_count&&!targetIds.contains(1),"policy receives eight distinct non-self marriage candidates");
        test(w.fingerprint()==beforeCandidates,"marriage candidate generation does not mutate world state");
        auto marriageObservation=w.observation(1);
        for(int action=realm_marriage_candidate_begin;action<realm_marriage_candidate_begin+realm_marriage_candidate_count;++action){
            const auto target=uint64_t(marriageChoices[size_t(action)].target);
            const auto offset=realm_state+size_t(action)*realm_features;
            test(std::abs(marriageObservation[offset+1]-w.person(target).age_days/36500.f)<1e-6f,"marriage candidate exposes target age instead of saturated ID");
            test(std::abs(marriageObservation[offset+2]-w.person(target).region/6.f)<1e-6f,"marriage candidate exposes target region");
        }
        const auto total=w.total_money();
        const auto proposerCashBeforeOffer=w.person(1).cash;
        auto propose=w.candidates(1);
        propose[72]={Intent::Marry,2,0,50,30};
        auto proposalMask=w.mask(1,propose);
        test(proposalMask[72]==1.f,"adult unrelated target can receive marriage offer");
        act(w,1,propose[72]);
        auto pending=w.offers(1);
        test(pending.size()==1&&pending[0].kind==3&&pending[0].escrow==50&&!pending[0].resolved,"marriage proposal records exact settlement escrow");
        test(w.person(1).cash==proposerCashBeforeOffer-50&&w.balances()[8]==50&&w.total_money()==total,
             "marriage escrow conserves money before consent");
        auto preConsentEdges=w.relations();
        test(std::none_of(preConsentEdges.begin(),preConsentEdges.end(),[](const Edge&e){return e.kind==RelationKind::Spouse;}),"proposal alone creates no spouse relation");
        std::string error;Realm restored(1,1,128);
        test(restored.deserialize(w.serialize("court"),"court",error)&&restored.fingerprint()==w.fingerprint(),"pending marriage escrow round trips in schema23");
        restored.advance(std::max(1,pending[0].ready-restored.day()));
        auto accept=restored.candidates(2)[28];
        test(accept.kind==Intent::Accept&&accept.target==int(pending[0].id),"recipient receives explicit accept action");
        const auto proposerBeforeAccept=restored.person(1).cash,recipientBeforeAccept=restored.person(2).cash;
        act(restored,2,accept);
        auto edges=restored.relations();
        test(std::any_of(edges.begin(),edges.end(),[](const Edge&e){return e.kind==RelationKind::Spouse&&e.from==1&&e.to==2;}),"acceptance creates canonical spouse relation");
        test(restored.person(1).cash==proposerBeforeAccept&&restored.person(2).cash==recipientBeforeAccept+50&&restored.balances()[8]==0&&restored.total_money()==total,"acceptance settles escrow once and conserves money");
        test(restored.household_of(1)==proposerHousehold&&restored.household_of(2)==recipientHousehold&&
             restored.household_fund(proposerHousehold)==7&&restored.household_fund(recipientHousehold)==11,
             "acceptance leaves both household memberships and balances separate");
        const auto foreignFundFingerprint=restored.fingerprint();
        const auto foreignWithdraw=restored.execute(HouseholdFundCommand{2,restored.person(2).decisions+1,
            proposerHousehold,HouseholdFundOperation::Withdraw,1});
        test(!foreignWithdraw.success&&restored.fingerprint()==foreignFundFingerprint&&
             restored.household_fund(proposerHousehold)==7&&restored.household_fund(recipientHousehold)==11,
             "marriage does not let one spouse withdraw from the other household fund");
        const auto validWithdraw=restored.execute(HouseholdFundCommand{1,restored.person(1).decisions+1,
            proposerHousehold,HouseholdFundOperation::Withdraw,3});
        test(validWithdraw.success&&restored.household_fund(proposerHousehold)==4&&
             restored.household_fund(recipientHousehold)==11&&restored.person(1).cash==proposerBeforeAccept+3&&
             restored.total_money()==total&&restored.invariant(),
             "a spouse can still withdraw only from their own household fund after marriage");
        auto second=restored.candidates(1);second[72]={Intent::Marry,3,0,10,30};
        test(restored.mask(1,second)[72]==0.f,"living spouse blocks a second marriage");
        RealmObservationTestAccess::mark_dead(restored,2);
        RealmObservationTestAccess::run_people_phase(restored);
        test(!restored.person(2).alive&&restored.person(1).alive&&restored.invariant(),
             "Realm death phase records the first spouse as deceased without removing the historic relation");
        test(restored.household_of(1)==proposerHousehold&&restored.household_of(2)==recipientHousehold&&
             restored.household_fund(proposerHousehold)==4&&restored.household_fund(recipientHousehold)==11,
             "death settlement keeps both common funds with their original households");
        auto widowChoices=restored.candidates(1);
        auto newSpouse=std::find_if(widowChoices.begin()+realm_marriage_candidate_begin,
            widowChoices.begin()+realm_marriage_candidate_begin+realm_marriage_candidate_count,
            [](const Candidate& candidate){return candidate.kind==Intent::Marry&&candidate.target!=2;});
        test(newSpouse!=widowChoices.begin()+realm_marriage_candidate_begin+realm_marriage_candidate_count,
             "a widowed adult receives a new eligible marriage candidate");
        const auto newSpouseId=uint64_t(newSpouse->target);
        const auto proposerCash=restored.person(1).cash;
        const auto newHousehold=restored.person(newSpouseId).household_membership.household_id;
        const auto widowTotal=restored.total_money();
        test(restored.mask(1,widowChoices)[size_t(newSpouse-widowChoices.begin())]==1.f,
             "widowed adult can offer marriage to the selected eligible person");
        act(restored,1,*newSpouse);
        const auto remarriageOffer=restored.offers(1);
        test(remarriageOffer.size()==1&&remarriageOffer[0].kind==3&&remarriageOffer[0].escrow==50&&
             restored.person(1).cash==proposerCash-50&&restored.balances()[8]==50,
             "remarriage holds the exact proposed settlement in the existing escrow account");
        auto widowedEdges=restored.relations();
        test(std::any_of(widowedEdges.begin(),widowedEdges.end(),[](const Edge& edge){
                 return edge.kind==RelationKind::Spouse&&edge.from==1&&edge.to==2;
             })&&std::none_of(widowedEdges.begin(),widowedEdges.end(),[&](const Edge& edge){
                 return edge.kind==RelationKind::Spouse&&((edge.from==1&&edge.to==newSpouseId)||
                     (edge.from==newSpouseId&&edge.to==1));
             }),"offer preserves the historic spouse edge and creates no new marriage before consent");
        std::string remarriageError;Realm remarriageLoaded(1,1,128);
        test(remarriageLoaded.deserialize(restored.serialize("widowed-remarriage"),"widowed-remarriage",remarriageError)&&
             remarriageLoaded.fingerprint()==restored.fingerprint(),"pending remarriage escrow and history survive save/load");
        const auto readyOffer=remarriageLoaded.offers(1).front();
        remarriageLoaded.advance(std::max(1,readyOffer.ready-remarriageLoaded.day()));
        const auto proposerCashAtAcceptance=remarriageLoaded.person(1).cash;
        const auto newSpouseCashAtAcceptance=remarriageLoaded.person(newSpouseId).cash;
        const auto totalAtAcceptance=remarriageLoaded.total_money();
        auto newRecipientChoices=remarriageLoaded.candidates(newSpouseId);
        const auto acceptRemarriage=newRecipientChoices[28];
        test(acceptRemarriage.kind==Intent::Accept&&acceptRemarriage.target==int(readyOffer.id),
             "the named new recipient receives the acceptance choice after the waiting period");
        act(remarriageLoaded,newSpouseId,acceptRemarriage);
        auto remarriedEdges=remarriageLoaded.relations();
        test(std::any_of(remarriedEdges.begin(),remarriedEdges.end(),[&](const Edge& edge){
                 return edge.kind==RelationKind::Spouse&&edge.from==1&&edge.to==2;
             })&&std::any_of(remarriedEdges.begin(),remarriedEdges.end(),[&](const Edge& edge){
                 return edge.kind==RelationKind::Spouse&&edge.from==1&&edge.to==newSpouseId;
             })&&!remarriageLoaded.person(2).alive,
             "acceptance records the new spouse while retaining the deceased spouse as history");
        test(remarriageLoaded.person(1).cash==proposerCashAtAcceptance&&
             remarriageLoaded.person(newSpouseId).cash==newSpouseCashAtAcceptance+50&&
             remarriageLoaded.balances()[8]==0&&remarriageLoaded.total_money()==totalAtAcceptance&&
             totalAtAcceptance==widowTotal&&
             remarriageLoaded.person(1).household_membership.household_id==proposerHousehold&&
             remarriageLoaded.person(newSpouseId).household_membership.household_id==newHousehold&&
             remarriageLoaded.invariant(),
             "new spouse accepts once, the settlement conserves money, and marriage does not merge households");
        test(std::count_if(remarriedEdges.begin(),remarriedEdges.end(),[&](const Edge& edge){
                 return edge.kind==RelationKind::Spouse&&((edge.from==1&&remarriageLoaded.person(edge.to).alive)||
                     (edge.to==1&&remarriageLoaded.person(edge.from).alive));
             })==1,"after remarriage the living person has exactly one living spouse");
        std::string finalError;Realm finalLoaded(1,1,128);
        test(finalLoaded.deserialize(remarriageLoaded.serialize("widowed-remarriage-accepted"),
             "widowed-remarriage-accepted",finalError)&&finalLoaded.fingerprint()==remarriageLoaded.fingerprint(),
             "accepted remarriage and both spouse records survive save/load");
        std::string schema22=test_fixtures::save_as_v25(fixture(0).serialize("court"));schema22.erase(schema22.find(" ESTATE_1 "));schema22.replace(0,15,"POLYEC_REALM_22");
        Realm old(1,1,128);test(old.deserialize(schema22,"court",error)&&old.serialize("court").rfind("POLYEC_REALM_75 ",0)==0,"schema22 world without spouse edges migrates to the current save schema");
    }
    {
        Realm rejected=fixture(0);
        auto proposal=rejected.candidates(1)[72];proposal={Intent::Marry,2,0,50,30};act(rejected,1,proposal);
        auto pending=rejected.offers(1).front();rejected.advance(std::max(1,pending.ready-rejected.day()));
        const auto cashBeforeReject=rejected.person(1).cash;
        act(rejected,2,{Intent::Reject,int(pending.id)});
        test(rejected.person(1).cash==cashBeforeReject+50&&rejected.balances()[8]==0,"rejection refunds marriage escrow exactly");
        Realm expired=fixture(0);
        auto expiring=expired.candidates(1)[72];expiring={Intent::Marry,2,0,50,1};act(expired,1,expiring);
        const auto expiration=expired.offers(1).front();const auto totalBeforeExpiry=expired.total_money();expired.advance(expiration.due-expired.day()+1);
        test(expired.offers(1).empty()&&expired.balances()[8]==0&&expired.total_money()==totalBeforeExpiry&&expired.invariant(),"expiry refunds marriage escrow through proposal lifecycle");
    }
    {
        Realm w=fixture(0);auto unrelated=w.candidates(1);unrelated[72]={Intent::Marry,2,0,50,30};act(w,1,unrelated[72]);
        auto duplicate=w.candidates(1);duplicate[72]={Intent::Marry,3,0,10,30};
        test(w.mask(1,duplicate)[72]==0.f,"person cannot open concurrent marriage proposal");
        auto competing=w.candidates(2);competing[72]={Intent::Marry,3,0,10,30};
        test(w.mask(2,competing)[72]==0.f,"recipient with pending marriage proposal cannot be proposed to by another person");
        auto kin=w.candidates(17);kin[72]={Intent::Marry,18,0,10,30};
        test(w.person(17).age_days>=18*365&&w.person(18).age_days>=18*365&&w.mask(17,kin)[72]==0.f,"adult siblings cannot marry");
        Realm sharedParent=fixture(0);uint64_t commonParent=1,firstSibling=0,secondSibling=0;
        for(uint64_t id=2;id<=uint64_t(sharedParent.count())&&!firstSibling;++id)
            if(sharedParent.person(id).alive&&sharedParent.person(id).age_days>=18*365)firstSibling=id;
        for(uint64_t id=2;id<=uint64_t(sharedParent.count())&&!secondSibling;++id)
            if(id!=firstSibling&&sharedParent.person(id).alive&&sharedParent.person(id).age_days>=18*365)secondSibling=id;
        test(firstSibling&&secondSibling,"common-parent sibling fixture has two adults");
        RealmObservationTestAccess::set_common_parent_siblings(sharedParent,commonParent,firstSibling,secondSibling);
        const auto commonParentRelations=sharedParent.relations();
        test(std::none_of(commonParentRelations.begin(),commonParentRelations.end(),[&](const Edge& edge){
            return edge.kind==RelationKind::Sibling&&
                ((edge.from==firstSibling&&edge.to==secondSibling)||(edge.from==secondSibling&&edge.to==firstSibling));
        }),"common-parent sibling fixture has no direct sibling edge");
        auto commonParentOffer=sharedParent.candidates(firstSibling);
        commonParentOffer[72]={Intent::Marry,int(secondSibling),0,10,30};
        const auto commonParentBefore=sharedParent.fingerprint();
        test(sharedParent.mask(firstSibling,commonParentOffer)[72]==0.f&&
             !sharedParent.execute({firstSibling,sharedParent.person(firstSibling).decisions+1,commonParentOffer[72]}).success&&
             sharedParent.fingerprint()==commonParentBefore,
             "shared-parent siblings without an explicit sibling edge cannot marry");
        std::string commonParentError;Realm commonParentLoaded(1,1,128);
        test(commonParentLoaded.deserialize(sharedParent.serialize("court"),"court",commonParentError),commonParentError.c_str());
        auto loadedSiblingOffer=commonParentLoaded.candidates(firstSibling);
        loadedSiblingOffer[72]={Intent::Marry,int(secondSibling),0,10,30};
        test(commonParentLoaded.mask(firstSibling,loadedSiblingOffer)[72]==0.f,
             "save/load rebuilds sibling relationships from shared parents");
        const auto beforeIncrementalSiblingEdges=sharedParent.relations();
        uint64_t laterSibling=0;
        for(uint64_t id=2;id<=uint64_t(sharedParent.count())&&!laterSibling;++id)
            if(id!=commonParent&&id!=firstSibling&&id!=secondSibling&&
               sharedParent.person(id).alive&&sharedParent.person(id).age_days>=18*365&&
               std::none_of(beforeIncrementalSiblingEdges.begin(),beforeIncrementalSiblingEdges.end(),[&](const Edge& edge){
                   return edge.kind==RelationKind::ParentChild&&edge.from==id&&edge.to==commonParent;
               }))laterSibling=id;
        test(laterSibling>0,"incremental sibling fixture has another adult");
        RealmObservationTestAccess::add_parent_child_relation(sharedParent,laterSibling,commonParent);
        auto incrementalSiblingOffer=sharedParent.candidates(laterSibling);
        incrementalSiblingOffer[72]={Intent::Marry,int(firstSibling),0,10,30};
        test(sharedParent.mask(laterSibling,incrementalSiblingOffer)[72]==0.f,
             "adding a child to an existing parent incrementally indexes every sibling");
        const auto relations=w.relations();
        const auto adultParentEdge=std::find_if(relations.begin(),relations.end(),[&](const Edge& edge){
            return edge.kind==RelationKind::ParentChild&&edge.from!=edge.to&&
                w.person(edge.from).alive&&w.person(edge.to).alive&&
                w.person(edge.from).age_days>=18*365&&w.person(edge.to).age_days>=18*365;
        });
        test(adultParentEdge!=relations.end(),"court fixture has a typed parent-child relation between living adults");
        auto directKin=w.candidates(adultParentEdge->from);
        directKin[72]={Intent::Marry,int(adultParentEdge->to),0,10,30};
        test(w.mask(adultParentEdge->from,directKin)[72]==0.f,
             "direct parent-child relation blocks marriage between adults");

        Realm lineal=fixture(0);
        uint64_t grandparent=1,parent=0,grandchild=2;
        for(uint64_t id=3;id<=uint64_t(lineal.count());++id)
            if(lineal.person(id).alive&&lineal.person(id).age_days>=18*365){parent=id;break;}
        test(parent>0,"lineal marriage fixture has three living adults");
        RealmObservationTestAccess::set_lineal_kinship(lineal,grandparent,parent,grandchild);
        auto descendantChoice=lineal.candidates(grandchild);
        descendantChoice[72]={Intent::Marry,int(grandparent),0,50,30};
        const auto linealBefore=lineal.fingerprint();
        test(lineal.mask(grandchild,descendantChoice)[72]==0.f,
             "grandparent and grandchild cannot offer marriage");
        test(!lineal.execute({grandchild,lineal.person(grandchild).decisions+1,descendantChoice[72]}).success&&
             lineal.fingerprint()==linealBefore,
             "multi-generation lineal marriage offer rejects without changing the Realm");
        std::string linealError;Realm linealLoaded(1,1,128);
        test(linealLoaded.deserialize(lineal.serialize("court"),"court",linealError),linealError.c_str());
        auto loadedDescendantChoice=linealLoaded.candidates(grandchild);
        loadedDescendantChoice[72]={Intent::Marry,int(grandparent),0,50,30};
        test(linealLoaded.mask(grandchild,loadedDescendantChoice)[72]==0.f,
             "save/load rebuilds the multi-generation kinship index");

        Realm changedKinship=fixture(0);
        auto pendingChoice=changedKinship.candidates(grandparent);
        pendingChoice[72]={Intent::Marry,int(grandchild),0,50,30};
        test(changedKinship.mask(grandparent,pendingChoice)[72]==1.f,
             "unrelated adults can make an offer before a later kinship fact");
        act(changedKinship,grandparent,pendingChoice[72]);
        const auto pendingMarriage=changedKinship.offers(grandparent).front();
        RealmObservationTestAccess::set_lineal_kinship(changedKinship,grandparent,parent,grandchild);
        changedKinship.advance(std::max(1,pendingMarriage.ready-changedKinship.day()));
        const auto acceptance=changedKinship.candidates(grandchild)[28];
        const auto beforeLinealAcceptance=changedKinship.fingerprint();
        test(acceptance.kind==Intent::Accept&&acceptance.target==int(pendingMarriage.id)&&
             !changedKinship.execute({grandchild,changedKinship.person(grandchild).decisions+1,acceptance}).success&&
             changedKinship.fingerprint()==beforeLinealAcceptance,
             "acceptance rechecks multi-generation kinship and rejects atomically");

        uint64_t minor=0;for(uint64_t id=1;id<=w.count();++id)if(w.person(id).age_days<18*365){minor=id;break;}
        auto underage=w.candidates(1);underage[72]={Intent::Marry,int(minor),0,10,30};
        test(minor>0&&w.mask(1,underage)[72]==0.f,"minor cannot marry");
    }
    std::cout<<"COURT TESTS: "<<n<<" checks passed\n";return 0;}catch(const std::exception&e){std::cerr<<"FAIL after "<<n<<": "<<e.what()<<'\n';return 1;}}
