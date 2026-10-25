#include "model.hpp"
namespace pe::production {
namespace {
void begin(State& s,const Command& c,bool self){
    auto& p=person(s,c.actor);idle(p);auto& l=location(s,c.target);
    require(l.id==11||l.id==12,"No recipe here");require(p.location==l.id,"Worker must be at site");
    if(self)require(p.id==l.owner,"Only owner can choose self production");else require(p.id!=l.owner,"Owner cannot accept own employment offer");
    require((self||l.job)&&l.tool&&l.tool_uses>0,"No posted job or working tool");
    for(auto&[id,q]:s.people)require(q.activity.kind!=ActivityKind::Work||q.activity.target!=l.id,"Production slot occupied");
    require(p.body.fatigue<95,"Too fatigued to work");auto& owner=account(s,l.owner);
    Good input=l.id==11?Good::Seed:Good::Wood;int n=l.id==11?1:2;
    require(l.stock[input]>=n,"Insufficient production input");require(self||owner.money>=l.wage,"Employer lacks wage funds");
    start(s,p,ActivityKind::Work,l.id,l.id==11?6:8,self?1:0);
    p.activity.escrow=self?0:l.wage;owner.money-=p.activity.escrow;p.activity.reserved[input]=n;l.stock[input]-=n;l.job=false;
    emit(s,p.name+(self?" chose self production; no wage transfer":" accepted "+l.name+" job; wage"+std::to_string(l.wage)+" and inputs reserved"),{p.id,l.owner});
}
}
void offer(State& s,const Command& c){
    auto& p=person(s,c.actor);idle(p);auto& l=location(s,c.target);require(l.id==11||l.id==12,"Site has no production recipe");
    require(l.owner==p.id,"No ownership permission to post job");require(p.location==l.id,"Owner must visit site");
    for(auto&[id,q]:s.people)require(q.activity.kind!=ActivityKind::Work||q.activity.target!=l.id,"Production slot occupied");
    require(!l.job,"Job already posted; revoke before changing terms");int wage=c.param("wage",6);require(wage>=1&&wage<=100,"Wage must be1..100");
    require(p.account.money>=wage,"Employer cannot fund wage");l.wage=wage;l.job=true;emit(s,p.name+" posted work at "+l.name+" wage"+std::to_string(wage),{p.id},true);
}
void revoke(State& s,const Command& c){
    auto& p=person(s,c.actor);idle(p);auto& l=location(s,c.target);require(l.owner==p.id&&p.location==l.id,"Only owner present may revoke job");
    require(l.job,"No unaccepted job to revoke");l.job=false;emit(s,p.name+" revoked unaccepted job at "+l.name,{p.id},true);
}
void work(State& s,const Command& c){begin(s,c,false);}
void produce(State& s,const Command& c){begin(s,c,true);}
void buy(State&s,const Command&c){auto&p=person(s,c.actor);idle(p);auto&l=location(s,c.target);require(l.id==11||l.id==12,"No standing sale here");require(p.location==l.id,"Buyer must visit sale site");require(p.id!=l.owner,"Cannot buy your own standing stock");require(c.amount<=20,"Sale quantity exceeds20");Good good=l.id==11?Good::Food:Good::Tool;Money price=l.id==11?4:8;
 Transaction{}.money(p.id,l.owner,price*c.amount).money(p.id,100,c.amount).goods(l.stock,p.account.inventory,good,c.amount).commit(s);
 emit(s,p.name+" bought "+std::to_string(c.amount)+(l.id==11?" food":" tool")+"; paid seller"+std::to_string(price*c.amount)+" + tax"+std::to_string(c.amount),{p.id,l.owner});emit(s,"PUBLIC tax receipt: "+std::to_string(c.amount)+" from "+p.name,{p.id},true);}
void cancel(State&s,Person&p){auto&l=location(s,p.activity.target);account(s,l.owner).money+=p.activity.escrow;for(int i=0;i<goods;++i)l.stock.units[i]+=p.activity.reserved.units[i];p.activity.escrow=0;p.activity.reserved={};}
void advance(State&s,Person&p){if(p.body.fatigue>=95||p.body.health<=0){cancel_activity(s,p.id,"body cannot continue work; escrow/input returned");return;}auto&a=p.activity;bool farm=a.target==11;double rate=p.experience.efficiency(farm,p.body.joy);a.progress+=rate;p.body.fatigue=std::min(100.,p.body.fatigue+2);(farm?p.experience.farming:p.experience.crafting)+=1;
 if(a.progress+1e-9>=a.required){auto&l=location(s,a.target);for(int i=0;i<goods;++i)s.consumed.units[i]+=a.reserved.units[i];s.created[farm?Good::Food:Good::Tool]+=farm?3:1;l.stock[farm?Good::Food:Good::Tool]+=farm?3:1;--l.tool_uses;Money wage=a.escrow;p.account.money+=wage;p.activity={};emit(s,p.name+" completed "+l.name+"; inputs consumed, output "+(farm?"3 food":"1 tool")+" stored; wage"+std::to_string(wage)+" paid; tool capacity"+std::to_string(l.tool_uses),{p.id,l.owner});}}
}
