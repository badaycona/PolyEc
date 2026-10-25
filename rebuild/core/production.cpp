#include "model.hpp"
namespace pe::production {
namespace {
void begin(State& s,const Command& c,bool self){
    auto& p=person(s,c.actor);idle(p);auto& l=location(s,c.target);
    require(l.recipe.work>0,"No recipe here");require(p.location==l.id,"Worker must be at site");
    if(self)require(p.id==l.owner,"Only owner can choose self production");else require(p.id!=l.owner,"Owner cannot accept own employment offer");
    require((self||l.job)&&l.tool&&l.tool_uses>0,"No posted job or working tool");
    for(auto&[id,q]:s.people)require(q.activity.kind!=ActivityKind::Work||q.activity.target!=l.id,"Production slot occupied");
    require(p.body.fatigue<95,"Too fatigued to work");auto& owner=account(s,l.owner);
    for(int i=0;i<goods;++i)require(l.stock.units[i]>=l.recipe.input.units[i],"Insufficient production input");require(self||owner.money>=l.wage,"Employer lacks wage funds");
    start(s,p,ActivityKind::Work,l.id,l.recipe.work,self?1:0);
    p.activity.escrow=self?0:l.wage;owner.money-=p.activity.escrow;p.activity.reserved=l.recipe.input;for(int i=0;i<goods;++i)l.stock.units[i]-=l.recipe.input.units[i];l.job=false;
    emit(s,p.name+(self?" chose self production; no wage transfer":" accepted "+l.name+" job; wage"+std::to_string(l.wage)+" and inputs reserved"),{p.id,l.owner});
}
}
void offer(State& s,const Command& c){
    auto& p=person(s,c.actor);idle(p);auto& l=location(s,c.target);require(l.recipe.work>0,"Site has no production recipe");
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
void buy(State&s,const Command&c){auto&p=person(s,c.actor);idle(p);auto&l=location(s,c.target);require(l.has_sale,"No standing sale here");require(p.location==l.id,"Buyer must visit sale site");require(p.id!=l.owner,"Cannot buy your own standing stock");require(c.amount<=20,"Sale quantity exceeds20");Good good=l.sale_good;Money price=l.price;
 Transaction{}.money(p.id,l.owner,price*c.amount).money(p.id,treasury_for_location(s,l.id),c.amount).goods(l.stock,p.account.inventory,good,c.amount).commit(s);
 emit(s,p.name+" bought "+std::to_string(c.amount)+(" units of good"+std::to_string(int(good)))+"; paid seller"+std::to_string(price*c.amount)+" + tax"+std::to_string(c.amount),{p.id,l.owner});emit(s,"PUBLIC tax receipt: "+std::to_string(c.amount)+" from "+p.name,{p.id},true);}
void cancel(State&s,Person&p){auto&l=location(s,p.activity.target);account(s,l.owner).money+=p.activity.escrow;for(int i=0;i<goods;++i)l.stock.units[i]+=p.activity.reserved.units[i];p.activity.escrow=0;p.activity.reserved={};}
void advance(State&s,Person&p){if(p.body.fatigue>=95||p.body.health<=0){cancel_activity(s,p.id,"body cannot continue work; escrow/input returned");return;}auto&a=p.activity;const auto& recipe=location(s,a.target).recipe;bool farm=recipe.skill==WorkSkill::Farming;double rate=p.experience.efficiency(farm,p.body.joy);a.progress+=rate;p.body.fatigue=std::min(100.,p.body.fatigue+2);(farm?p.experience.farming:p.experience.crafting)+=1;
 if(a.progress+1e-9>=a.required){auto&l=location(s,a.target);for(int i=0;i<goods;++i)s.consumed.units[i]+=a.reserved.units[i];for(int i=0;i<goods;++i){s.created.units[i]+=l.recipe.output.units[i];l.stock.units[i]+=l.recipe.output.units[i];}--l.tool_uses;Money wage=a.escrow;p.account.money+=wage;p.activity={};emit(s,p.name+" completed "+l.name+"; inputs consumed, output "+(l.recipe.skill==WorkSkill::Farming?"3 food":"1 tool")+" stored; wage"+std::to_string(wage)+" paid; tool capacity"+std::to_string(l.tool_uses),{p.id,l.owner});}}
}
