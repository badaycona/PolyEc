#include "model.hpp"
#include <iomanip>
#include <sstream>
namespace pe {
namespace {
void invout(std::ostream&o,const Inventory&i){for(int n:i.units)o<<n<<' ';}
void invin(std::istream&i,Inventory&v){for(int&n:v.units)i>>n;}
size_t count(std::istream&i,size_t max){long long n=-1;i>>n;require(bool(i)&&n>=0&&n<=static_cast<long long>(max),"Save count exceeds bounds");return size_t(n);}
std::string str(std::istream&i,size_t max=2000){std::string s;i>>std::quoted(s);require(bool(i)&&s.size()<=max,"Invalid save string");return s;}
void locout(std::ostream&o,const Location&l){
 o<<l.id<<' '<<l.owner<<' '<<std::quoted(l.name)<<' '<<l.x<<' '<<l.y<<' '<<l.tool<<' '<<l.job<<' '<<l.wage<<' '<<l.tool_uses<<' ';invout(o,l.stock);
 o<<l.region<<' '<<int(l.kind)<<' '<<l.lodging_capacity<<' '<<l.has_sale<<' '<<int(l.sale_good)<<' '<<l.price<<' '<<l.recipe.work<<' '<<int(l.recipe.skill)<<' ';invout(o,l.recipe.input);invout(o,l.recipe.output);
 o<<l.residents.size()<<' ';for(Id r:l.residents)o<<r<<' ';o<<'\n';
}
Location locin(std::istream&i){Location l;int kind=-1,good=-1,skill=-1;i>>l.id>>l.owner;l.name=str(i,160);i>>l.x>>l.y>>l.tool>>l.job>>l.wage>>l.tool_uses;invin(i,l.stock);i>>l.region>>kind>>l.lodging_capacity>>l.has_sale>>good>>l.price>>l.recipe.work>>skill;l.kind=SiteKind(kind);l.sale_good=Good(good);l.recipe.skill=WorkSkill(skill);invin(i,l.recipe.input);invin(i,l.recipe.output);size_t n=count(i,8);for(size_t k=0;k<n;++k){Id id=0;i>>id;l.residents.push_back(id);}return l;}
void govout(std::ostream&o,const Organization&g){o<<g.id<<' '<<std::quoted(g.name)<<' '<<g.account.id<<' '<<g.account.money<<' '<<g.office.holder<<' '<<g.office.can_grant<<' ';invout(o,g.account.inventory);o<<'\n';}
Organization govin(std::istream&i){Organization g;i>>g.id;g.name=str(i,160);i>>g.account.id>>g.account.money>>g.office.holder>>g.office.can_grant;invin(i,g.account.inventory);return g;}
void regionout(std::ostream&o,const Territory&t){o<<t.id<<' '<<std::quoted(t.name)<<' '<<t.treasury<<' '<<t.locations.size()<<'\n';for(auto&[id,l]:t.locations)locout(o,l);}
Territory regionin(std::istream&i){Territory t;i>>t.id;t.name=str(i,160);i>>t.treasury;size_t n=count(i,6);require(n==6,"Wrong location count");for(size_t j=0;j<n;++j){auto l=locin(i);require(t.locations.emplace(l.id,l).second,"Duplicate location");}return t;}
}
std::string encode(const State&s){std::ostringstream o;o<<std::setprecision(17);
 o<<"POLYEC 3 trial-b1\n"<<s.seed<<' '<<s.tick<<' '<<s.revision<<' '<<s.next_event<<' '<<s.next_request<<' '<<s.initial_money<<' '<<s.next_contract<<' '<<s.next_debt<<' '<<s.next_lease<<' '<<s.fixture_regions<<'\n';
 invout(o,s.created);invout(o,s.consumed);invout(o,s.initial_goods);o<<'\n';
 govout(o,s.government);o<<s.other_governments.size()<<'\n';for(auto&[id,g]:s.other_governments)govout(o,g);
 regionout(o,s.territory);o<<s.neighbors.size()<<'\n';for(auto&[id,t]:s.neighbors)regionout(o,t);
 o<<s.routes.size()<<'\n';for(auto&[id,r]:s.routes){o<<r.id<<' '<<r.from<<' '<<r.to<<' '<<r.controller<<' '<<r.collector<<' '<<std::quoted(r.name)<<' '<<r.length<<' '<<r.capacity<<' '<<r.toll<<' '<<r.open<<'\n';locout(o,r.camp);locout(o,r.road);}
 o<<s.people.size()<<'\n';for(auto&[id,p]:s.people){o<<id<<' '<<std::quoted(p.name)<<' '<<p.location<<' '<<p.account.id<<' '<<p.account.money<<' ';invout(o,p.account.inventory);o<<'\n'<<p.body.hunger<<' '<<p.body.thirst<<' '<<p.body.fatigue<<' '<<p.body.health<<' '<<p.body.joy<<'\n'<<p.experience.farming<<' '<<p.experience.crafting<<' '<<p.experience.communication<<' '<<p.tendencies.security<<' '<<p.tendencies.pleasure<<' '<<p.tendencies.mobility<<' '<<std::quoted(p.goal)<<'\n';
 const auto&a=p.activity;o<<int(a.kind)<<' '<<a.target<<' '<<a.progress<<' '<<a.required<<' '<<a.escrow<<' '<<a.option<<' '<<a.started<<' '<<std::quoted(a.cause)<<' ';invout(o,a.reserved);o<<'\n';
 const auto&j=p.journey;o<<j.route<<' '<<j.origin<<' '<<j.destination<<' '<<j.coordinate<<' '<<j.steps<<' '<<j.leg_start<<' '<<j.paused<<' '<<j.toll_paid<<' '<<j.started<<'\n';
 o<<p.relations.size()<<' ';for(auto&[other,v]:p.relations)o<<other<<' '<<v<<' ';o<<'\n'<<p.knowledge.size()<<'\n';for(auto&f:p.knowledge)o<<f.event<<' '<<f.source<<' '<<f.happened<<' '<<f.received<<' '<<std::quoted(f.text)<<'\n';}
 o<<s.requests.size()<<'\n';for(auto&r:s.requests)o<<r.id<<' '<<r.from<<' '<<r.to<<' '<<r.status<<'\n';
 o<<s.events.size()<<'\n';for(auto&e:s.events){o<<e.id<<' '<<e.tick<<' '<<e.public_notice<<' '<<std::quoted(e.text)<<' '<<e.witnesses.size()<<' ';for(Id id:e.witnesses)o<<id<<' ';o<<'\n';}
 o<<s.proposals.size()<<'\n';for(auto& p:s.proposals){o<<p.id<<' '<<int(p.kind)<<' '<<p.from<<' '<<p.to<<' '<<p.author<<' '<<p.awaiting<<' '<<p.site<<' '<<p.source_site<<' '<<int(p.good)<<' '<<p.amount<<' '<<p.price<<' '<<p.duration<<' '<<p.deadline<<' '<<int(p.status)<<' '<<p.escrow<<' ';invout(o,p.reserved);o<<'\n';}
 o<<s.debts.size()<<'\n';for(auto& d:s.debts)o<<d.id<<' '<<d.proposal<<' '<<d.lender<<' '<<d.borrower<<' '<<d.principal<<' '<<d.interest<<' '<<d.outstanding<<' '<<d.due<<' '<<d.status<<'\n';
 o<<s.leases.size()<<'\n';for(auto& l:s.leases)o<<l.id<<' '<<l.proposal<<' '<<l.landlord<<' '<<l.tenant<<' '<<l.site<<' '<<l.start<<' '<<l.end<<' '<<l.rent<<' '<<l.status<<'\n';o<<"END\n";return o.str();
}
State decode(const std::string&text){require(text.size()<=16000000,"Save too large");std::istringstream i(text);std::string magic,rule;int version=0;i>>magic>>version>>rule;require(magic=="POLYEC"&&version==3&&rule=="trial-b1","Unsupported save/rules version");State s;
 i>>s.seed>>s.tick>>s.revision>>s.next_event>>s.next_request>>s.initial_money>>s.next_contract>>s.next_debt>>s.next_lease>>s.fixture_regions;invin(i,s.created);invin(i,s.consumed);invin(i,s.initial_goods);
 s.government=govin(i);size_t n=count(i,1);for(size_t k=0;k<n;++k){auto g=govin(i);require(s.other_governments.emplace(g.id,g).second,"Duplicate organization");}
 s.territory=regionin(i);n=count(i,1);for(size_t k=0;k<n;++k){auto t=regionin(i);require(s.neighbors.emplace(t.id,t).second,"Duplicate region");}
 n=count(i,1);for(size_t k=0;k<n;++k){Route r;i>>r.id>>r.from>>r.to>>r.controller>>r.collector;r.name=str(i,160);i>>r.length>>r.capacity>>r.toll>>r.open;r.camp=locin(i);r.road=locin(i);require(s.routes.emplace(r.id,r).second,"Duplicate route");}
 n=count(i,8);for(size_t k=0;k<n;++k){Person p;i>>p.id;p.name=str(i,160);i>>p.location>>p.account.id>>p.account.money;invin(i,p.account.inventory);i>>p.body.hunger>>p.body.thirst>>p.body.fatigue>>p.body.health>>p.body.joy>>p.experience.farming>>p.experience.crafting>>p.experience.communication>>p.tendencies.security>>p.tendencies.pleasure>>p.tendencies.mobility;p.goal=str(i,160);
 int kind=-1;i>>kind>>p.activity.target>>p.activity.progress>>p.activity.required>>p.activity.escrow>>p.activity.option>>p.activity.started;p.activity.kind=ActivityKind(kind);p.activity.cause=str(i,160);invin(i,p.activity.reserved);
 auto&j=p.journey;i>>j.route>>j.origin>>j.destination>>j.coordinate>>j.steps>>j.leg_start>>j.paused>>j.toll_paid>>j.started;
 size_t m=count(i,8);for(size_t h=0;h<m;++h){Id id=0;int value=0;i>>id>>value;require(p.relations.emplace(id,value).second,"Duplicate relation");}m=count(i,20000);for(size_t h=0;h<m;++h){Fact f;i>>f.event>>f.source>>f.happened>>f.received;f.text=str(i);p.knowledge.push_back(f);}require(s.people.emplace(p.id,p).second,"Duplicate person");}
 n=count(i,2000);for(size_t k=0;k<n;++k){Request r;i>>r.id>>r.from>>r.to>>r.status;s.requests.push_back(r);}
 n=count(i,20000);for(size_t k=0;k<n;++k){Event e;i>>e.id>>e.tick>>e.public_notice;e.text=str(i);size_t m=count(i,8);for(size_t h=0;h<m;++h){Id id=0;i>>id;e.witnesses.push_back(id);}s.events.push_back(e);}
 n=count(i,2000);for(size_t k=0;k<n;++k){Proposal p;int kind=-1,g=-1,status=-1;i>>p.id>>kind>>p.from>>p.to>>p.author>>p.awaiting>>p.site>>p.source_site>>g>>p.amount>>p.price>>p.duration>>p.deadline>>status>>p.escrow;invin(i,p.reserved);p.kind=ProposalKind(kind);p.good=Good(g);p.status=ContractStatus(status);s.proposals.push_back(p);}
 n=count(i,2000);for(size_t k=0;k<n;++k){Debt d;i>>d.id>>d.proposal>>d.lender>>d.borrower>>d.principal>>d.interest>>d.outstanding>>d.due>>d.status;s.debts.push_back(d);}
 n=count(i,2000);for(size_t k=0;k<n;++k){Lease l;i>>l.id>>l.proposal>>l.landlord>>l.tenant>>l.site>>l.start>>l.end>>l.rent>>l.status;s.leases.push_back(l);}
 std::string end;i>>end;require(bool(i)&&end=="END","Truncated or malformed save");i>>std::ws;require(i.eof(),"Trailing save data");return s;
}
}
