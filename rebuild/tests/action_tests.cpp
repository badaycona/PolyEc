#include "model.hpp"
#include <iostream>
#include <set>
using namespace pe;
namespace {
int checks=0;std::set<std::string> exercised;
void check(bool b,const std::string& why){++checks;if(!b)throw std::runtime_error(why);}
void ok(Result r){check(r.ok,r.message);}
Result send(World& w,Id actor,const std::string& verb,Id target=0,int amount=1,int option=0,std::map<std::string,int> params={},std::string text=""){
    exercised.insert(verb);return w.submit({actor,verb,target,amount,option,w.state().revision,std::move(text),std::move(params)});
}
void reject(World& w,Id actor,const std::string& verb,Id target,const std::string& reason,int amount=1,int option=0,std::map<std::string,int> params={}){
    auto before=w.save();auto r=send(w,actor,verb,target,amount,option,params);check(!r.ok&&r.message.find(reason)!=std::string::npos,"Expected "+reason+"; got "+r.message);check(w.save()==before,"Rejected command changed state");
}
void go(World& w,int actor,int site){ok(send(w,actor,"move",site));ok(w.advance(1));}
void finish(World& w,int actor){int count=0;while(person(w.state(),actor).activity.kind!=ActivityKind::Idle){ok(w.advance(1));check(++count<30,"Activity stuck");}}
void roundtrip(World& w){World copy;auto text=w.save();ok(copy.load(text));check(copy.save()==text,"Save not exact");ok(copy.advance(2));ok(w.advance(2));check(copy.save()==w.save(),"Resume changed outcomes");}
void loaded(World& w,State s){ok(w.load(encode(s)));}
void malformed(World& w,State s,const std::string& why){auto before=w.save();auto r=w.load(encode(s));check(!r.ok,"Corruption accepted: "+why);check(w.save()==before,"Bad load changed state");}
}
int main(){try{
    World assets;go(assets,4,11);ok(send(assets,2,"manage",11,2,0,{{"good",0}}));
    check(person(assets.state(),2).account.inventory[Good::Food]==2&&location(assets.state(),11).stock[Good::Food]==3,"Owner take stock");
    reject(assets,4,"manage",11,"Only owner");ok(send(assets,2,"transfer",4,7));ok(send(assets,2,"transfer",4,1,1,{{"good",0}}));
    check(person(assets.state(),4).account.money==19&&person(assets.state(),4).account.inventory[Good::Food]==2,"Gifts use donor's assets");
    ok(send(assets,2,"manage",11,1,1,{{"good",0}}));check(location(assets.state(),11).stock[Good::Food]==4,"Store goods back");
    reject(assets,4,"transfer",5,"present");reject(assets,2,"transfer",4,"Insufficient funds",1000);reject(assets,2,"manage",11,"Unknown good",1,0,{{"good",9}});
    reject(assets,4,"meal",0,"Unknown parameter",1,0,{{"price",1}});

    World sale;ok(send(sale,4,"propose",5,1,0,{{"price",2},{"duration",20}}));
    check(person(sale.state(),4).account.inventory[Good::Food]==0&&sale.state().proposals.back().reserved[Good::Food]==1,"Sale stock reserved");
    reject(sale,3,"respond",1,"Not addressed");reject(sale,4,"respond",1,"Not addressed");
    ok(send(sale,5,"counter",1,1,0,{{"price",1}}));check(sale.state().proposals[0].author==5&&sale.state().proposals[0].awaiting==4,"Counter has actual consent turn");
    reject(sale,5,"respond",1,"Not addressed");roundtrip(sale);ok(send(sale,4,"respond",1));
    check(person(sale.state(),4).account.money==13&&person(sale.state(),5).account.money==2&&sale.state().government.account.money==101,"Counter price1 + tax1 computed independently");
    check(person(sale.state(),5).account.inventory[Good::Food]==1&&sale.state().proposals[0].status==ContractStatus::Fulfilled,"Goods received exactly once");reject(sale,5,"respond",1,"no longer");
    World pub;go(pub,4,11);ok(send(pub,2,"propose",0,1,0,{{"price",2},{"source_site",11}}));ok(send(pub,4,"respond",1));check(person(pub.state(),4).account.money==9&&location(pub.state(),11).stock[Good::Food]==4,"Public owned-site sale real stock/tax");
    World returning;ok(send(returning,4,"propose",5,1,0,{{"price",2}}));ok(send(returning,5,"respond",1,1,1));check(person(returning.state(),4).account.inventory[Good::Food]==1,"Reject returns goods");
    ok(send(returning,4,"propose",0,1,0,{{"price",2}}));ok(send(returning,4,"respond",2,1,2));check(person(returning.state(),4).account.inventory[Good::Food]==1,"Withdraw returns goods");
    ok(send(returning,4,"propose",0,1,0,{{"price",2},{"duration",2}}));ok(returning.advance(2));check(returning.state().proposals.back().status==ContractStatus::Expired&&person(returning.state(),4).account.inventory[Good::Food]==1,"Expiry returns goods");
    auto corrupted=decode(sale.save());corrupted.proposals[0].author=3;malformed(sale,corrupted,"forged consent author");

    World loan;go(loan,1,13);ok(send(loan,1,"propose",5,10,1,{{"price",2},{"duration",3}}));
    check(person(loan.state(),1).account.money==50&&person(loan.state(),5).account.money==4,"Principal held, not issued before consent");
    ok(send(loan,5,"respond",1));check(person(loan.state(),5).account.money==14&&loan.state().debts[0].outstanding==12,"Loan issues10 debt12");
    reject(loan,4,"repay",1,"Only borrower");ok(send(loan,5,"repay",1,3));check(loan.state().debts[0].outstanding==9,"Partial repayment3");
    roundtrip(loan);ok(loan.advance(1));check(loan.state().debts[0].status==2&&person(loan.state(),5).account.money==11,"Overdue does not debit automatically");
    reject(loan,5,"repay",1,"exceeds",10);ok(send(loan,5,"repay",1,9));check(loan.state().debts[0].status==1&&person(loan.state(),1).account.money==62&&person(loan.state(),5).account.money==2,"Loan repaid principal and agreed interest");
    ok(send(loan,1,"propose",5,5,1,{{"duration",1}}));ok(loan.advance(1));check(person(loan.state(),1).account.money==62,"Unaccepted loan expires refund");
    corrupted=decode(loan.save());corrupted.debts[0].borrower=4;malformed(loan,corrupted,"debt party differs from consent");
    check(loan.view(4,false).find("\"outstanding\"")==std::string::npos,"Other person's debt is private");

    World lease;go(lease,2,15);go(lease,5,15);reject(lease,5,"rest",0,"residence");
    ok(send(lease,2,"propose",5,1,2,{{"price",2},{"duration",2},{"site",15}}));ok(send(lease,5,"respond",1));
    check(person(lease.state(),2).account.money==222&&person(lease.state(),5).account.money==2,"Lease prepaid2");
    ok(send(lease,5,"rest"));ok(lease.advance(2));check(lease.state().leases[0].status==1&&person(lease.state(),5).location==15,"Lease expiry does not remove occupant");
    finish(lease,5);reject(lease,5,"rest",0,"residence");
    ok(send(lease,2,"propose",0,1,2,{{"price",1},{"duration",12}}));ok(send(lease,5,"respond",2));ok(send(lease,5,"end_lease",2));
    check(lease.state().leases[1].status==2&&person(lease.state(),5).account.money==1&&person(lease.state(),5).location==15,"Tenant termination no false refund/eviction");
    reject(lease,2,"end_lease",2,"Only tenant");
    corrupted=decode(lease.save());corrupted.leases[0].landlord=3;malformed(lease,corrupted,"forged lease owner");

    World delivery;go(delivery,1,13);ok(send(delivery,1,"propose",4,1,3,{{"price",3},{"duration",8}}));ok(send(delivery,4,"respond",1));
    check(person(delivery.state(),1).account.money==57&&person(delivery.state(),4).account.inventory[Good::Food]==1,"Accept is obligation, not delivery");
    roundtrip(delivery);ok(send(delivery,4,"fulfill",1));check(person(delivery.state(),1).account.inventory[Good::Food]==1&&person(delivery.state(),4).account.money==15,"Delivery real goods for reward3");
    reject(delivery,4,"fulfill",1,"No accepted");
    ok(send(delivery,1,"propose",5,1,3,{{"price",2},{"duration",2}}));ok(send(delivery,5,"respond",2));reject(delivery,5,"fulfill",2,"Insufficient goods");ok(delivery.advance(2));
    check(delivery.state().proposals.back().status==ContractStatus::Defaulted&&person(delivery.state(),1).account.money==57,"Missed deadline refunds reward, no goods spawn");
    ok(send(delivery,1,"propose",5,1,3,{{"price",2}}));ok(send(delivery,5,"respond",3));ok(send(delivery,5,"respond",3,1,2));check(delivery.state().proposals.back().status==ContractStatus::Cancelled&&person(delivery.state(),1).account.money==57,"Accepted obligation can be cancelled explicitly");

    World office;ok(send(office,1,"propose",4,1,4));check(office.state().government.office.holder==1,"Appointment not accepted yet");ok(send(office,4,"respond",1));
    check(office.state().government.office.holder==4&&person(office.state(),4).id==4,"Office changes, Person identity stays");reject(office,1,"grant",5,"permission");ok(send(office,4,"grant",5,5));
    check(office.state().government.account.money==95&&person(office.state(),4).account.money==12,"Office holder spends public, not own money");
    reject(office,4,"dismiss",0,"Only King");ok(send(office,1,"dismiss"));reject(office,4,"grant",5,"permission");check(office.state().government.office.holder==0,"Office vacant after dismissal");
    ok(send(office,1,"announce",0,1,0,{},"Office is vacant"));reject(office,4,"announce",0,"Only King");ok(send(office,1,"inspect"));
    reject(office,2,"propose",3,"Only King",1,4);ok(send(office,1,"propose",3,1,4));ok(send(office,3,"respond",2,1,1));check(office.state().government.office.holder==0,"Rejected appointment grants no authority");

    World workshop;ok(send(workshop,3,"revoke_job",12));ok(send(workshop,3,"offer",12,1,0,{{"wage",9}}));go(workshop,5,12);ok(send(workshop,5,"work",12));
    check(person(workshop.state(),5).activity.escrow==9,"Custom wage reserved9");reject(workshop,3,"equip",12,"during",1,0,{{"source_site",12}});finish(workshop,5);
    check(person(workshop.state(),5).account.money==13&&location(workshop.state(),12).tool_uses==11,"Agreed wage9, tool wears once");
    ok(send(workshop,3,"equip",12,1,0,{{"source_site",12}}));check(location(workshop.state(),12).tool_uses==12&&workshop.state().consumed[Good::Tool]==1,"Tool replacement consumes old installed tool");
    World self;ok(send(self,2,"produce",11));check(person(self.state(),2).activity.escrow==0,"Self-production no wage escrow");roundtrip(self);finish(self,2);check(person(self.state(),2).account.money==220&&location(self.state(),11).stock[Good::Food]==8,"Owner production actual output no fake payment");
    World broken;auto b=decode(broken.save());b.territory.locations.at(12).tool_uses=0;loaded(broken,b);go(broken,5,12);reject(broken,5,"work",12,"working tool");ok(send(broken,3,"equip",12,1,0,{{"source_site",12}}));ok(send(broken,5,"work",12));ok(send(broken,5,"cancel"));check(location(broken.state(),12).tool_uses==12,"Cancelled job does not wear tool");

    World water;go(water,4,14);ok(send(water,4,"collect"));ok(send(water,4,"cancel"));check(location(water.state(),14).stock[Good::Water]==120,"Cancelled collection returns reserve");
    ok(send(water,4,"collect"));finish(water,4);check(person(water.state(),4).account.inventory[Good::Water]==1&&location(water.state(),14).stock[Good::Water]==119,"Collect water actual transfer");go(water,4,15);ok(send(water,4,"drink",0,1,1));finish(water,4);check(person(water.state(),4).account.inventory[Good::Water]==0,"Drink own water away from well");
    World care;auto h=decode(care.save());h.people.at(4).location=15;h.people.at(4).body.health=80;h.people.at(4).account.inventory[Good::Water]=1;--h.territory.locations.at(14).stock[Good::Water];loaded(care,h);
    ok(send(care,4,"care"));ok(care.advance(3));roundtrip(care);finish(care,4);check(person(care.state(),4).body.health==85&&care.state().consumed[Good::Food]==1&&care.state().consumed[Good::Water]==1,"Care requires time and real food/water");
    reject(care,4,"care",0,"requires own");
    World invitation;ok(send(invitation,4,"talk",5));ok(send(invitation,5,"invitation",1));check(invitation.state().requests[0].status==3,"Recipient declines pending conversation");ok(send(invitation,4,"talk",5));ok(send(invitation,4,"invitation",2,1,1));reject(invitation,5,"accept",2,"pending");
    World oldsave;auto before=oldsave.save();check(!oldsave.load("POLYEC 1 trial-a1\n").ok&&oldsave.save()==before,"Save1 rejected explicitly, no fake compatibility");
    for(auto&[verb,a]:action_registry())check(a.handler!=nullptr&&a.definition.verb==verb,"Registry definition/handler mismatch");
    std::cout<<"PASS "<<checks<<" action/domain checks; exercised "<<exercised.size()<<" verbs; catalog "<<action_registry().size()<<" runnable definitions\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
