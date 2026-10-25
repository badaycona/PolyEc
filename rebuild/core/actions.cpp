#include "model.hpp"
namespace pe {
const std::map<std::string,RegisteredAction>& action_registry(){
    static const auto registry=[] {
        std::map<std::string,RegisteredAction> r;
        auto add=[&](const char*verb,const char*params,const char*description,ActionHandler handler){std::vector<std::string> names;std::string v=verb;
        if(v=="propose")names={"good","price","duration","source_site","site"};
        else if(v=="manage")names={"good"};else if(v=="transfer")names={"good","source_site"};
        else if(v=="equip")names={"source_site"};else if(v=="offer")names={"wage"};else if(v=="counter")names={"price"};
        require(r.emplace(verb,RegisteredAction{{verb,params,description},handler,names}).second,"Duplicate action definition");};
        add("move","target:site","Travel10 minutes",life::move);
        add("offer","target:site wage:1..100","Owner posts one funded job",production::offer);
        add("revoke_job","target:site","Owner withdraws unaccepted job",production::revoke);
        add("work","target:site","Accept job with wage/input escrow",production::work);
        add("produce","target:site","Owner performs own production; no wage fiction",production::produce);
        add("buy","target:site amount:1..20","Standing fixture sale with tax",production::buy);
        add("manage","target:site option:take0/store1 good amount","Owner moves site stock",resources::manage);
        add("transfer","target:person option:money0/goods1 good source_site amount","Voluntary transfer from own assets",resources::transfer);
        add("equip","target:site source_site","Replace installed tool from real owned inventory",resources::equip);
        add("propose","target:person|public0 option:sale0/loan1/lease2/delivery3/appointment4 good price duration source_site site amount","Create terms and reserve own offered assets",contracts::propose);
        add("counter","target:proposal price","Change pending sale/lease price; other party must consent",contracts::counter);
        add("respond","target:proposal option:accept0/reject1/withdraw2","Resolve pending terms with explicit consent",contracts::respond);
        add("fulfill","target:proposal","Deliver own goods to requester and receive escrow reward",contracts::fulfill);
        add("repay","target:debt amount","Borrower pays outstanding from own account",contracts::repay);
        add("end_lease","target:lease","Tenant ends prepaid residence right; no forced movement",contracts::end_lease);
        add("meal","option:simple0/enjoyable1","Prepare using own food and time",life::meal);
        add("drink","option:well0/own-water1","Consume real water",life::drink);
        add("collect","","Collect1 public well water in10 minutes",life::collect);
        add("rest","","Rest at lawful residence30 minutes",life::rest);
        add("care","","Own food1/water1 plus60 minutes at residence; conditional recovery",life::care);
        add("talk","target:person","Invite information exchange without automatic acceptance",civic::talk);
        add("accept","target:invitation","Recipient accepts conversation",civic::accept);
        add("invitation","target:invitation option:decline0/withdraw1","Explicitly decline/withdraw invitation",civic::invitation);
        add("study","","Read public records at court20 minutes",civic::study);
        add("inspect","","Record direct local observation",civic::inspect);
        add("grant","target:person amount:1..50","Authorized office holder spends treasury",civic::grant);
        add("dismiss","","King removes office authority; public assets stay public",civic::dismiss);
        add("announce","text","King/office holder publishes at court",civic::announce);
        add("goal","text","Change remembered objective",civic::goal);
        add("cancel","","Cancel active activity; apply resource rules",[](State&s,const Command&c){cancel_activity(s,c.actor,"actor choice");});
        return r;
    }();return registry;
}
std::vector<ActionDefinition> World::definitions(){std::vector<ActionDefinition> out;for(auto&[verb,a]:action_registry())out.push_back(a.definition);return out;}
}
