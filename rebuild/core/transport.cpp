#include "model.hpp"
namespace pe {
Route& route(State& s,Id id){auto i=s.routes.find(id);require(i!=s.routes.end(),"Unknown route");return i->second;}
const Route& route(const State& s,Id id){auto i=s.routes.find(id);require(i!=s.routes.end(),"Unknown route");return i->second;}
}
namespace pe::transport {
int carried_weight(const Person& p){constexpr int weights[]={1,1,2,3,1};int n=0;for(int i=0;i<goods;++i)n+=(p.account.inventory.units[i]+p.activity.reserved.units[i])*weights[i];return n;}
namespace {
void matching(const Person& p,const Command& c){require(p.journey.route!=0&&(c.target==0||c.target==p.journey.route),"No matching journey");}
int coordinate(const Route& r,Id endpoint){require(endpoint==r.from||endpoint==r.to,"Invalid journey endpoint");return endpoint==r.from?0:r.length;}
void leg(State& s,Person& p){
    auto& j=p.journey;auto& r=route(s,j.route);int distance=std::abs(coordinate(r,j.destination)-j.coordinate);
    if(distance==0){Id dest=j.destination;p.journey={};p.activity={};p.location=dest;observe(s,p.id);return;}
    require(r.open||j.destination==j.origin,"Route closed; only retreat to origin is allowed");
    require(p.body.fatigue<95,"Too fatigued to travel");require(carried_weight(p)<=r.capacity,"Carried load exceeds route capacity");
    j.paused=false;j.leg_start=j.coordinate;p.location=r.road.id;start(s,p,ActivityKind::Travel,r.id,distance);
}
}
void travel(State& s,const Command& c){
    auto& p=person(s,c.actor);idle(p);require(p.journey.route==0,"Journey already exists; resume or turn back");auto& r=route(s,c.target);
    require(p.location==r.from||p.location==r.to,"Travel must start at a route endpoint");require(r.open,"Route is closed");
    require(p.body.fatigue<95,"Too fatigued to travel");require(carried_weight(p)<=r.capacity,"Carried load exceeds route capacity");
    Id origin=p.location;Transaction{}.money(p.id,r.collector,r.toll).commit(s);
    p.journey={};auto& j=p.journey;j.route=r.id;j.origin=origin;j.destination=origin==r.from?r.to:r.from;j.coordinate=coordinate(r,origin);j.leg_start=j.coordinate;j.toll_paid=r.toll;j.started=s.tick;
    leg(s,p);emit(s,p.name+" started route"+std::to_string(r.id)+"; toll"+std::to_string(r.toll)+" paid to account"+std::to_string(r.collector)+"; cargo weight"+std::to_string(carried_weight(p)),{p.id});
}
void interrupt(State& s,Person& p,const std::string& cause){auto& j=p.journey;require(j.route!=0,"No journey to interrupt");const auto& r=route(s,j.route);j.paused=true;p.activity={};p.location=j.coordinate==r.length/2?r.camp.id:r.road.id;emit(s,p.name+" journey paused at coordinate"+std::to_string(j.coordinate)+": "+cause,{p.id});}
void pause(State& s,const Command& c){auto& p=person(s,c.actor);matching(p,c);require(p.activity.kind==ActivityKind::Travel,"Only moving journey may be paused");interrupt(s,p,"actor chose stop");}
void resume(State& s,const Command& c){auto& p=person(s,c.actor);idle(p);matching(p,c);require(p.journey.paused,"Journey is not paused");leg(s,p);emit(s,p.name+" resumed journey; no second toll",{p.id});}
void turnback(State& s,const Command& c){auto& p=person(s,c.actor);idle(p);matching(p,c);require(p.journey.paused,"Pause before turning back");p.journey.destination=p.journey.origin;leg(s,p);emit(s,p.name+" chose retreat toward origin; no teleport/refund",{p.id});}
void control(State& s,const Command& c){auto& p=person(s,c.actor);idle(p);auto& r=route(s,c.target);require(p.id==r.controller,"No route control permission");require(c.option==0||c.option==1,"Route mode close0/open1");bool open=c.option==1;require(r.open!=open,"Route already has requested state");r.open=open;emit(s,"PUBLIC: "+p.name+(open?" opened ":" closed ")+r.name+"; travelers must react",{p.id},true);}
void advance(State& s,Person& p){auto& j=p.journey;auto& r=route(s,j.route);
    bool blocked=!r.open&&j.destination!=j.origin;if(blocked||p.body.fatigue>=95||p.body.health<=0){interrupt(s,p,blocked?"route closed ahead":"body cannot continue");return;}
    j.coordinate+=j.destination==r.to?1:-1;++j.steps;p.activity.progress+=1;p.body.fatigue=std::min(100.,p.body.fatigue+1);
    if(j.coordinate==coordinate(r,j.destination)){Id destination=j.destination;int steps=j.steps;Money toll=j.toll_paid;p.activity={};p.journey={};p.location=destination;emit(s,p.name+" arrived after"+std::to_string(steps)+" movement steps; toll paid once"+std::to_string(toll),{p.id});observe(s,p.id);}
}
void validate(const State& s){
    require(s.routes.size()==(s.fixture_regions==2?1u:0u),"Invalid route fixture");
    for(auto&[id,r]:s.routes){require(id==r.id&&r.id==1&&r.length==12&&r.capacity==12&&r.toll==2&&r.controller==1&&r.collector==100,"Invalid route terms");person(s,r.controller);institution(s,r.collector);require(location(s,r.from).region==1&&location(s,r.to).region==2,"Invalid route topology");require(r.camp.id==16&&r.road.id==17&&r.camp.kind==SiteKind::Camp&&r.road.kind==SiteKind::Road&&r.camp.region==0&&r.road.region==0,"Invalid transit locations");}
    for(auto&[id,p]:s.people){const auto& j=p.journey;
        if(j.route==0){require(j.origin==0&&j.destination==0&&j.coordinate==0&&j.steps==0&&!j.paused&&j.toll_paid==0&&j.started==0&&j.leg_start==0&&p.activity.kind!=ActivityKind::Travel,"Incomplete journey record");require(location(s,p.location).kind!=SiteKind::Road&&location(s,p.location).kind!=SiteKind::Camp,"Transit occupant lacks journey");continue;}
        const auto& r=route(s,j.route);require((j.origin==r.from||j.origin==r.to)&&(j.destination==r.from||j.destination==r.to),"Invalid journey endpoints");
        require(j.coordinate>=0&&j.coordinate<=r.length&&j.leg_start>=0&&j.leg_start<=r.length&&j.toll_paid==r.toll,"Invalid journey position/toll");
        require(j.started>=0&&j.started<=s.tick&&j.steps>=0&&j.steps<=s.tick-j.started&&j.steps>=std::abs(j.coordinate-coordinate(r,j.origin)),"Invalid journey elapsed movement");
        require(j.coordinate!=coordinate(r,j.destination),"Arrived journey was not settled");require(carried_weight(p)<=r.capacity,"Carried load exceeds route capacity");
        if(j.paused){require(p.activity.kind!=ActivityKind::Travel&&p.location==(j.coordinate==r.length/2?r.camp.id:r.road.id),"Paused journey location mismatch");}
        else {const auto& a=p.activity;require(p.location==r.road.id&&a.kind==ActivityKind::Travel&&a.target==r.id&&a.progress==std::abs(j.coordinate-j.leg_start)&&a.required==std::abs(coordinate(r,j.destination)-j.leg_start),"Journey/activity progress mismatch");}
    }
}
}
