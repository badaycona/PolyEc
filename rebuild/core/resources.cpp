#include "model.hpp"
namespace pe::resources {
namespace {
Good good(const Command& c){int g=c.param("good",0);require(g>=0&&g<goods,"Unknown good");return Good(g);}
Inventory& source(State& s,const Command& c){
    auto& p=person(s,c.actor);Id id=c.param("source_site",0);if(!id)return p.account.inventory;
    auto& l=location(s,id);require(l.owner==p.id&&p.location==id,"No ownership/location permission for source stock");return l.stock;
}
}
void manage(State& s,const Command& c){
    auto& p=person(s,c.actor);idle(p);auto& l=location(s,c.target);
    require(l.owner==p.id&&l.id==p.location,"Only owner present may manage site stock");require(c.option==0||c.option==1,"Manage mode take0/store1");
    Good g=good(c);if(c.option==0)Transaction{}.goods(l.stock,p.account.inventory,g,c.amount).commit(s);else Transaction{}.goods(p.account.inventory,l.stock,g,c.amount).commit(s);
    emit(s,p.name+" moved owned stock "+std::to_string(c.amount)+(c.option?" into site":" into personal inventory"),{p.id});
}
void transfer(State& s,const Command& c){
    auto& p=person(s,c.actor);idle(p);auto& recipient=person(s,c.target);require(recipient.id!=p.id&&co_located(s,p.id,recipient.id),"Gift recipient must be another person present");
    if(c.option==0){require(c.param("source_site",0)==0,"Gift money must come from own account");Transaction{}.money(p.id,recipient.id,c.amount).commit(s);}
    else {require(c.option==1,"Transfer mode money0/goods1");Transaction{}.goods(source(s,c),recipient.account.inventory,good(c),c.amount).commit(s);}
    emit(s,p.name+" voluntarily transferred "+std::to_string(c.amount)+(c.option?" goods":" money")+" to "+recipient.name,{p.id,recipient.id});
}
void equip(State& s,const Command& c){
    auto& p=person(s,c.actor);idle(p);auto& site=location(s,c.target);require((site.recipe.work>0)&&site.owner==p.id&&p.location==site.id,"Only owner at production site may replace tool");
    for(auto&[id,worker]:s.people)require(worker.activity.kind!=ActivityKind::Work||worker.activity.target!=site.id,"Cannot replace tool during committed production");
    auto& stock=source(s,c);require(stock[Good::Tool]>0,"Replacement requires real tool");--stock[Good::Tool];if(site.tool)++s.consumed[Good::Tool];site.tool=true;site.tool_uses=12;
    emit(s,p.name+" installed real tool at "+site.name+";12 batches capacity",{p.id});
}
}
