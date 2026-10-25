#include "model.hpp"
#include <set>
namespace pe {
Person& person(State& s,Id id){auto i=s.people.find(id);require(i!=s.people.end(),"Unknown actor/person");return i->second;}
const Person& person(const State& s,Id id){auto i=s.people.find(id);require(i!=s.people.end(),"Unknown actor/person");return i->second;}
std::vector<Location*> locations(State& s){std::vector<Location*> out;for(auto&[id,l]:s.territory.locations)out.push_back(&l);for(auto&[id,t]:s.neighbors)for(auto&[lid,l]:t.locations)out.push_back(&l);for(auto&[id,r]:s.routes){out.push_back(&r.camp);out.push_back(&r.road);}std::sort(out.begin(),out.end(),[](auto a,auto b){return a->id<b->id;});return out;}
std::vector<const Location*> locations(const State& s){std::vector<const Location*> out;for(auto&[id,l]:s.territory.locations)out.push_back(&l);for(auto&[id,t]:s.neighbors)for(auto&[lid,l]:t.locations)out.push_back(&l);for(auto&[id,r]:s.routes){out.push_back(&r.camp);out.push_back(&r.road);}std::sort(out.begin(),out.end(),[](auto a,auto b){return a->id<b->id;});return out;}
Location& location(State& s,Id id){for(auto l:locations(s))if(l->id==id)return *l;throw RuleError("Unknown location");}
const Location& location(const State& s,Id id){for(auto l:locations(s))if(l->id==id)return *l;throw RuleError("Unknown location");}
Organization& institution(State& s,Id id){if(id==s.government.id)return s.government;auto i=s.other_governments.find(id);require(i!=s.other_governments.end(),"Unknown organization");return i->second;}
const Organization& institution(const State& s,Id id){if(id==s.government.id)return s.government;auto i=s.other_governments.find(id);require(i!=s.other_governments.end(),"Unknown organization");return i->second;}
Account& account(State& s,Id id){if(id==s.government.id||s.other_governments.contains(id))return institution(s,id).account;return person(s,id).account;}
Id treasury_for_location(const State& s,Id id){const auto& l=location(s,id);if(l.region==s.territory.id)return s.territory.treasury;auto i=s.neighbors.find(l.region);if(i!=s.neighbors.end())return i->second.treasury;for(auto&[rid,r]:s.routes)if(r.camp.id==id||r.road.id==id)return r.collector;throw RuleError("Location lacks treasury jurisdiction");}
bool co_located(const State& s,Id a,Id b){const auto& p=person(s,a);const auto& q=person(s,b);if(p.location!=q.location)return false;if(location(s,p.location).kind==SiteKind::Road)return p.journey.route&&p.journey.route==q.journey.route&&p.journey.coordinate==q.journey.coordinate;return true;}
void idle(const Person& p){require(p.activity.kind==ActivityKind::Idle,"Actor is busy; finish or cancel activity first");require(p.body.health>0,"Actor cannot act: health exhausted");}
void start(State& s,Person& p,ActivityKind k,Id target,double required,int option){p.activity={};p.activity.kind=k;p.activity.target=target;p.activity.required=required;p.activity.option=option;p.activity.started=s.tick;}
void emit(State& s,std::string text,std::vector<Id> witnesses,bool pub){
 Event e{s.next_event++,s.tick,std::move(text),std::move(witnesses),pub};
 if(pub){for(auto&[id,p]:s.people)if(location(s,p.location).kind==SiteKind::Court)e.witnesses.push_back(id);}
 else {auto initial=e.witnesses;for(Id id:initial){for(auto&[other,p]:s.people)if(co_located(s,id,other))e.witnesses.push_back(other);}}
 std::sort(e.witnesses.begin(),e.witnesses.end());e.witnesses.erase(std::unique(e.witnesses.begin(),e.witnesses.end()),e.witnesses.end());
 for(Id id:e.witnesses){auto& p=person(s,id);p.knowledge.push_back({e.id,e.text,id,s.tick,s.tick});}
 s.events.push_back(std::move(e));
}
void observe(State& s,Id id){auto&p=person(s,id);auto&l=location(s,p.location);std::string text="Observed "+l.name+"; present:";for(auto&[other,q]:s.people)if(co_located(s,id,other))text+=" "+q.name;
 if(l.has_sale)text+="; public sale good"+std::to_string(int(l.sale_good))+" price"+std::to_string(l.price)+" + tax1";if(l.recipe.work>0)text+="; job "+(l.job?"open wage"+std::to_string(l.wage):"closed");for(auto&[rid,r]:s.routes)if(l.id==r.from||l.id==r.to||l.id==r.road.id||l.id==r.camp.id)text+="; route"+std::to_string(rid)+(r.open?" open":" closed");
 emit(s,text,{id});
}
void cancel_activity(State&s,Id id,const std::string& why){auto&p=person(s,id);require(p.activity.kind!=ActivityKind::Idle,"Nothing to cancel");if(p.activity.kind==ActivityKind::Travel){transport::interrupt(s,p,why);return;}
 if(p.activity.kind==ActivityKind::Work)production::cancel(s,p);
 if(p.activity.kind==ActivityKind::Collect)location(s,p.activity.target).stock[Good::Water]+=p.activity.reserved[Good::Water];
 if(p.activity.kind==ActivityKind::Meal||p.activity.kind==ActivityKind::Drink||p.activity.kind==ActivityKind::Care)for(int i=0;i<goods;++i)s.consumed.units[i]+=p.activity.reserved.units[i];
 if(p.activity.kind==ActivityKind::Talk){int rid=p.activity.option;auto&other=person(s,p.activity.target);other.activity={};for(auto&r:s.requests)if(r.id==rid)r.status=3;}
 p.activity={};emit(s,p.name+" activity cancelled: "+why,{id});
}
World::World(std::uint32_t seed,bool two_regions){
 state_.seed=seed;state_.government.account.money=100;const char*names[]={"","King","Dien","Moc","An","Binh"};int balances[]={0,60,220,160,12,4};int locs[]={0,10,11,12,13,13};
 std::uint32_t rng=seed;for(int id=1;id<=5;++id){Person p;p.id=id;p.name=names[id];p.location=locs[id];p.account.id=id;p.account.money=balances[id];p.goal=id>=4?"Earn food reserve":"Maintain estate";rng=1664525*rng+1013904223;p.tendencies={int(rng%81)+10,int((rng>>8)%81)+10,int((rng>>16)%81)+10};p.account.inventory[Good::Food]=id==4?1:0;state_.people.emplace(id,p);}
 auto add=[&](int id,int owner,const char*name,int x,int y){Location l;l.id=id;l.owner=owner;l.name=name;l.x=x;l.y=y;state_.territory.locations.emplace(id,l);};
 add(10,100,"Court",320,110);add(11,2,"Farm",150,300);add(12,3,"Workshop",520,300);add(13,100,"Market",335,400);add(14,100,"Well",150,500);add(15,2,"Homes",520,500);
 auto&farm=location(state_,11);farm.stock[Good::Seed]=20;farm.stock[Good::Food]=5;farm.tool=true;farm.tool_uses=12;farm.job=true;
 auto&shop=location(state_,12);shop.stock[Good::Wood]=12;shop.stock[Good::Tool]=2;shop.tool=true;shop.tool_uses=12;shop.job=true;
 farm.kind=SiteKind::Production;farm.has_sale=true;farm.sale_good=Good::Food;farm.price=4;farm.recipe.input[Good::Seed]=1;farm.recipe.output[Good::Food]=3;farm.recipe.work=6;farm.recipe.skill=WorkSkill::Farming;
 shop.kind=SiteKind::Production;shop.has_sale=true;shop.sale_good=Good::Tool;shop.price=8;shop.recipe.input[Good::Wood]=2;shop.recipe.output[Good::Tool]=1;shop.recipe.work=8;
 location(state_,10).kind=SiteKind::Court;location(state_,13).kind=SiteKind::Market;location(state_,14).kind=SiteKind::Well;location(state_,15).kind=SiteKind::Residence;location(state_,15).lodging_capacity=2;
 location(state_,14).stock[Good::Water]=120;location(state_,15).residents={1,2,4};
 if(two_regions){
 state_.fixture_regions=2;state_.initial_money=926;state_.initial_goods={{13,20,38,6,222}};
 for(auto [id,name,money,loc]:std::vector<std::tuple<int,std::string,int,int>>{{6,"Luu",50,13},{7,"Tung",180,22},{8,"Lan",80,25}}){Person p;p.id=id;p.name=name;p.location=loc;p.account.id=id;p.account.money=money;p.account.inventory[Good::Food]=id==6?2:1;if(id==6)p.account.inventory[Good::Water]=2;p.goal=id==6?"Trade food in Tung Lam and return with wood":"Maintain local livelihood";state_.people.emplace(id,p);}
 Territory t;t.id=2;t.name="Tung Lam";t.treasury=200;
 auto site=[&](int id,int owner,const char* name,int x,int y,SiteKind kind){Location l;l.id=id;l.owner=owner;l.name=name;l.x=x;l.y=y;l.region=2;l.kind=kind;t.locations.emplace(id,l);};
 site(20,200,"Tung Lam Court",1080,110,SiteKind::Court);site(21,7,"Lumber yard",850,300,SiteKind::Market);site(22,7,"Tung Lam Workshop",1220,300,SiteKind::Production);site(23,200,"Tung Lam Market",1035,400,SiteKind::Market);site(24,200,"Tung Lam Well",850,500,SiteKind::Well);site(25,8,"Tung Lam Inn",1220,500,SiteKind::Residence);
 auto& yard=t.locations.at(21);yard.stock[Good::Wood]=20;yard.has_sale=true;yard.sale_good=Good::Wood;yard.price=3;
 auto& craft=t.locations.at(22);craft.recipe=shop.recipe;craft.stock[Good::Wood]=6;craft.stock[Good::Tool]=1;craft.has_sale=true;craft.sale_good=Good::Tool;craft.price=6;craft.tool=true;craft.tool_uses=12;craft.job=true;
 t.locations.at(24).stock[Good::Water]=100;auto& inn=t.locations.at(25);inn.stock[Good::Food]=3;inn.has_sale=true;inn.sale_good=Good::Food;inn.price=9;inn.lodging_capacity=2;inn.residents={8};state_.neighbors.emplace(2,t);
 Organization gov;gov.id=200;gov.name="Tung Lam treasury";gov.account.id=200;gov.account.money=60;gov.office={7,true};state_.other_governments.emplace(200,gov);
 Route r;r.camp.id=16;r.camp.owner=100;r.camp.name="Road camp";r.camp.x=685;r.camp.y=400;r.camp.region=0;r.camp.kind=SiteKind::Camp;r.road.id=17;r.road.owner=100;r.road.name="On the road";r.road.x=685;r.road.y=400;r.road.region=0;r.road.kind=SiteKind::Road;state_.routes.emplace(1,r);
 }
 emit(state_,"PUBLIC: treasury100 grants only by King; sales tax1 per unit; public well120; permanent homes rights for1,2,4; others negotiate lease",{1,2,3,4,5},true);
 emit(state_,"PUBLIC: Farm offers food4+tax1 and work6; Workshop tools8+tax1 and work6",{1,2,3,4,5},true);
 if(two_regions)emit(state_,"PUBLIC: route1 links markets13/23;120 minutes, load12, toll2 to treasury100; Tung Lam wood3, inn food9",{1,2,3,4,5,6,7,8},true);
 for(auto&[id,p]:state_.people)observe(state_,id);
 validate(state_);
}
Result World::submit(const Command&c){try{require(c.revision==state_.revision,"Stale command revision; refresh choices");person(state_,c.actor);require(c.amount>=1&&c.amount<=1000,"Amount out of bounds");require(c.text.size()<=160,"Goal too long");require(state_.events.size()<19000,"Event capacity reached; start a new fixture");State next=state_;
 const auto& handlers=action_registry();
 auto h=handlers.find(c.verb);require(h!=handlers.end(),"Unknown action");require(c.params.size()<=8,"Too many named parameters");for(auto&[name,value]:c.params){require(std::find(h->second.named_parameters.begin(),h->second.named_parameters.end(),name)!=h->second.named_parameters.end(),"Unknown parameter for action: "+name);require(value>=0&&value<=1000000,"Parameter value out of bounds");}h->second.handler(next,c);++next.revision;validate(next);state_=std::move(next);return {true,"Accepted: "+c.verb};}catch(const std::exception&e){return {false,e.what()};}}
Result World::advance(int steps){try{require(steps>0&&steps<=144,"Advance must be 1..144 steps");State next=state_;require(next.tick<=1000000-steps,"Time limit reached");require(next.events.size()<19000,"Event capacity reached");for(int i=0;i<steps;++i){++next.tick;contracts::advance(next);for(auto&[id,p]:next.people)p.body.elapse();for(auto&[id,p]:next.people){switch(p.activity.kind){case ActivityKind::Travel:transport::advance(next,p);break;case ActivityKind::Work:production::advance(next,p);break;case ActivityKind::Talk:case ActivityKind::Study:civic::advance(next,p);break;case ActivityKind::Idle:break;default:life::advance(next,p);}}++next.revision;}validate(next);state_=std::move(next);return {true,"Advanced "+std::to_string(steps)+" x 10 minutes"};}catch(const std::exception&e){return {false,e.what()};}}
std::string World::save()const{return encode(state_);}Result World::load(const std::string&text){try{State next=decode(text);validate(next);state_=std::move(next);return {true,"Loaded validated save"};}catch(const std::exception&e){return {false,e.what()};}}
std::string World::view(Id id,bool debug)const{return project(state_,id,debug);}
}

