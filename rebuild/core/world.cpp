#include "model.hpp"
#include <set>
namespace pe {
Person& person(State& s,Id id){auto i=s.people.find(id);require(i!=s.people.end(),"Unknown actor/person");return i->second;}
const Person& person(const State& s,Id id){auto i=s.people.find(id);require(i!=s.people.end(),"Unknown actor/person");return i->second;}
Location& location(State& s,Id id){auto i=s.territory.locations.find(id);require(i!=s.territory.locations.end(),"Unknown location");return i->second;}
const Location& location(const State& s,Id id){auto i=s.territory.locations.find(id);require(i!=s.territory.locations.end(),"Unknown location");return i->second;}
Account& account(State& s,Id id){if(id==100)return s.government.account;return person(s,id).account;}
void idle(const Person& p){require(p.activity.kind==ActivityKind::Idle,"Actor is busy; finish or cancel activity first");require(p.body.health>0,"Actor cannot act: health exhausted");}
void start(State& s,Person& p,ActivityKind k,Id target,double required,int option){p.activity={};p.activity.kind=k;p.activity.target=target;p.activity.required=required;p.activity.option=option;p.activity.started=s.tick;}
void emit(State& s,std::string text,std::vector<Id> witnesses,bool pub){
 Event e{s.next_event++,s.tick,std::move(text),std::move(witnesses),pub};
 if(pub){for(auto&[id,p]:s.people)if(p.location==10)e.witnesses.push_back(id);}
 else {auto initial=e.witnesses;for(Id id:initial){Id loc=person(s,id).location;for(auto&[other,p]:s.people)if(p.location==loc)e.witnesses.push_back(other);}}
 std::sort(e.witnesses.begin(),e.witnesses.end());e.witnesses.erase(std::unique(e.witnesses.begin(),e.witnesses.end()),e.witnesses.end());
 for(Id id:e.witnesses){auto& p=person(s,id);p.knowledge.push_back({e.id,e.text,id,s.tick,s.tick});}
 s.events.push_back(std::move(e));
}
void observe(State& s,Id id){auto&p=person(s,id);auto&l=location(s,p.location);std::string text="Observed "+l.name+"; present:";for(auto&[other,q]:s.people)if(q.location==l.id)text+=" "+q.name;
 if(l.id==11||l.id==12)text+="; public sale price "+std::to_string(l.id==11?4:8)+" + tax1; job "+(l.job?"open wage"+std::to_string(l.wage):"closed");
 emit(s,text,{id});
}
void cancel_activity(State&s,Id id,const std::string& why){auto&p=person(s,id);require(p.activity.kind!=ActivityKind::Idle,"Nothing to cancel");
 if(p.activity.kind==ActivityKind::Work)production::cancel(s,p);
 if(p.activity.kind==ActivityKind::Collect)location(s,14).stock[Good::Water]+=p.activity.reserved[Good::Water];
 if(p.activity.kind==ActivityKind::Meal||p.activity.kind==ActivityKind::Drink||p.activity.kind==ActivityKind::Care)for(int i=0;i<goods;++i)s.consumed.units[i]+=p.activity.reserved.units[i];
 if(p.activity.kind==ActivityKind::Talk){int rid=p.activity.option;auto&other=person(s,p.activity.target);other.activity={};for(auto&r:s.requests)if(r.id==rid)r.status=3;}
 p.activity={};emit(s,p.name+" activity cancelled: "+why,{id});
}
World::World(std::uint32_t seed){
 state_.seed=seed;state_.government.account.money=100;const char*names[]={"","King","Dien","Moc","An","Binh"};int balances[]={0,60,220,160,12,4};int locs[]={0,10,11,12,13,13};
 std::uint32_t rng=seed;for(int id=1;id<=5;++id){Person p;p.id=id;p.name=names[id];p.location=locs[id];p.account.id=id;p.account.money=balances[id];p.goal=id>=4?"Earn food reserve":"Maintain estate";rng=1664525*rng+1013904223;p.tendencies={int(rng%81)+10,int((rng>>8)%81)+10,int((rng>>16)%81)+10};p.account.inventory[Good::Food]=id==4?1:0;state_.people.emplace(id,p);}
 auto add=[&](int id,int owner,const char*name,int x,int y){Location l;l.id=id;l.owner=owner;l.name=name;l.x=x;l.y=y;state_.territory.locations.emplace(id,l);};
 add(10,100,"Court",320,110);add(11,2,"Farm",150,300);add(12,3,"Workshop",520,300);add(13,100,"Market",335,400);add(14,100,"Well",150,500);add(15,2,"Homes",520,500);
 auto&farm=location(state_,11);farm.stock[Good::Seed]=20;farm.stock[Good::Food]=5;farm.tool=true;farm.tool_uses=12;farm.job=true;
 auto&shop=location(state_,12);shop.stock[Good::Wood]=12;shop.stock[Good::Tool]=2;shop.tool=true;shop.tool_uses=12;shop.job=true;
 location(state_,14).stock[Good::Water]=120;location(state_,15).residents={1,2,4};
 emit(state_,"PUBLIC: treasury100 grants only by King; sales tax1 per unit; public well120; permanent homes rights for1,2,4; others negotiate lease",{1,2,3,4,5},true);
 emit(state_,"PUBLIC: Farm offers food4+tax1 and work6; Workshop tools8+tax1 and work6",{1,2,3,4,5},true);
 for(auto&[id,p]:state_.people)observe(state_,id);
 validate(state_);
}
Result World::submit(const Command&c){try{require(c.revision==state_.revision,"Stale command revision; refresh choices");person(state_,c.actor);require(c.amount>=1&&c.amount<=1000,"Amount out of bounds");require(c.text.size()<=160,"Goal too long");require(state_.events.size()<19000,"Event capacity reached; start a new fixture");State next=state_;
 const auto& handlers=action_registry();
 auto h=handlers.find(c.verb);require(h!=handlers.end(),"Unknown action");require(c.params.size()<=8,"Too many named parameters");for(auto&[name,value]:c.params){require(std::find(h->second.named_parameters.begin(),h->second.named_parameters.end(),name)!=h->second.named_parameters.end(),"Unknown parameter for action: "+name);require(value>=0&&value<=1000000,"Parameter value out of bounds");}h->second.handler(next,c);++next.revision;validate(next);state_=std::move(next);return {true,"Accepted: "+c.verb};}catch(const std::exception&e){return {false,e.what()};}}
Result World::advance(int steps){try{require(steps>0&&steps<=144,"Advance must be 1..144 steps");State next=state_;require(next.tick<=1000000-steps,"Time limit reached");require(next.events.size()<19000,"Event capacity reached");for(int i=0;i<steps;++i){++next.tick;contracts::advance(next);for(auto&[id,p]:next.people)p.body.elapse();for(auto&[id,p]:next.people){switch(p.activity.kind){case ActivityKind::Work:production::advance(next,p);break;case ActivityKind::Talk:case ActivityKind::Study:civic::advance(next,p);break;case ActivityKind::Idle:break;default:life::advance(next,p);}}++next.revision;}validate(next);state_=std::move(next);return {true,"Advanced "+std::to_string(steps)+" x 10 minutes"};}catch(const std::exception&e){return {false,e.what()};}}
std::string World::save()const{return encode(state_);}Result World::load(const std::string&text){try{State next=decode(text);validate(next);state_=std::move(next);return {true,"Loaded validated save"};}catch(const std::exception&e){return {false,e.what()};}}
std::string World::view(Id id,bool debug)const{return project(state_,id,debug);}
}

