#include "model.hpp"
namespace pe::contracts {
namespace {
Proposal& find(State& s,int id) {
    auto i=std::find_if(s.proposals.begin(),s.proposals.end(),[&](const Proposal& p){return p.id==id;});
    require(i!=s.proposals.end(),"Unknown proposal");return *i;
}
Inventory& origin(State& s,const Proposal& p) {
    return p.source_site ? location(s,p.source_site).stock : person(s,p.from).account.inventory;
}
void release(State& s,Proposal& p,ContractStatus status,const std::string& cause) {
    account(s,p.from).money+=p.escrow;p.escrow=0;
    for(int i=0;i<goods;++i)origin(s,p).units[i]+=p.reserved.units[i];
    p.reserved={};p.status=status;
    std::vector<Id> who{p.from};if(p.to)who.push_back(p.to);
    emit(s,"Proposal#"+std::to_string(p.id)+" "+cause,who);
}
void money_from_escrow(State& s,Proposal& p,Id recipient) {
    account(s,recipient).money+=p.escrow;p.escrow=0;
}
int live_slots(const State& s,Id site) {
    return int(std::count_if(s.leases.begin(),s.leases.end(),[&](const Lease& l){return l.site==site&&l.status==0&&l.end>s.tick;}));
}
}
Money escrow_total(const State& s){Money n=0;for(auto& p:s.proposals)n+=p.escrow;return n;}
Inventory reserved_total(const State& s){Inventory n;for(auto& p:s.proposals)for(int i=0;i<goods;++i)n.units[i]+=p.reserved.units[i];return n;}
bool residence_at(const State& s,Id actor,Id site,int tick){
    const auto& l=location(s,site);
    if(std::find(l.residents.begin(),l.residents.end(),actor)!=l.residents.end())return true;
    return std::any_of(s.leases.begin(),s.leases.end(),[&](const Lease& lease){return lease.tenant==actor&&lease.site==site&&lease.start<=tick&&tick<lease.end;});
}
bool residence(const State& s,Id actor,Id site){return residence_at(s,actor,site,s.tick);}
void propose(State& s,const Command& c){
    auto& actor=person(s,c.actor);idle(actor);
    require(c.option>=0&&c.option<=4,"Unknown proposal kind");
    require(s.proposals.size()<2000,"Proposal capacity reached");
    Proposal p;p.id=s.next_contract++;p.kind=ProposalKind(c.option);p.from=p.author=actor.id;p.to=p.awaiting=c.target;p.site=actor.location;
    p.amount=c.amount;p.price=c.param("price",p.kind==ProposalKind::Loan||p.kind==ProposalKind::Appointment?0:1);
    p.duration=c.param("duration",12);p.deadline=s.tick+p.duration;
    require(p.duration>=1&&p.duration<=1440&&p.price>=0&&p.price<=100,"Price/duration outside trial limits");
    int g=c.param("good",0);require(g>=0&&g<goods,"Unknown good");p.good=Good(g);
    require(p.from!=p.to,"Cannot contract with yourself");
    if(p.to){auto& other=person(s,p.to);require(other.body.health>0,"Counterparty cannot consent");if(p.kind!=ProposalKind::Appointment)require(other.location==actor.location,"Counterparty must be present to negotiate");}
    else require(p.kind==ProposalKind::Sale||p.kind==ProposalKind::Lease,"This proposal requires a named recipient");
    if(p.kind==ProposalKind::Sale){
        require(p.amount<=20&&p.price>=1,"Sale requires quantity1..20 and price1..100");
        p.source_site=c.param("source_site",0);
        if(p.source_site){auto& site=location(s,p.source_site);require(site.owner==p.from&&site.id==actor.location,"No ownership/location permission for source stock");}
        auto& stock=origin(s,p);require(stock[p.good]>=p.amount,"Insufficient goods to reserve sale");stock[p.good]-=p.amount;p.reserved[p.good]=p.amount;
    }else if(p.kind==ProposalKind::Loan){
        require(actor.account.money>=p.amount,"Insufficient loan principal");actor.account.money-=p.amount;p.escrow=p.amount;
    }else if(p.kind==ProposalKind::Lease){
        p.site=c.param("site",15);auto& home=location(s,p.site);
        require(home.id==15&&home.owner==p.from&&actor.location==home.id,"Only home owner at homes may offer lease");
        require(p.amount==1&&p.price>=1,"Lease is one prepaid residence right");
    }else if(p.kind==ProposalKind::Delivery){
        require(p.amount<=20&&p.price>=1,"Delivery requires quantity1..20 and reward1..100");
        require(actor.account.money>=p.price,"Insufficient delivery reward escrow");actor.account.money-=p.price;p.escrow=p.price;
    }else{
        require(p.from==1&&p.to!=0,"Only King may propose treasury appointment");p.amount=1;p.price=0;
    }
    s.proposals.push_back(p);std::vector<Id> who{p.from};if(p.to)who.push_back(p.to);
    emit(s,actor.name+" proposed#"+std::to_string(p.id)+" kind"+std::to_string(c.option)+"; awaiting explicit consent",who,p.kind==ProposalKind::Appointment);
}
void counter(State& s,const Command& c){
    auto& actor=person(s,c.actor);idle(actor);auto& p=find(s,c.target);
    require(p.status==ContractStatus::Pending&&s.tick<p.deadline,"Proposal no longer pending");
    require(p.kind==ProposalKind::Sale||p.kind==ProposalKind::Lease,"Only sale/lease supports price counteroffer");
    require((p.awaiting==actor.id)||(p.awaiting==0&&actor.id!=p.from),"Not your turn to counter");
    require(actor.location==p.site,"Counterparty must visit offer site");
    int price=c.param("price",c.amount);require(price>=1&&price<=100,"Counter price outside1..100");
    if(p.to==0)p.to=actor.id;
    p.price=price;p.author=actor.id;p.awaiting=actor.id==p.from?p.to:p.from;
    emit(s,actor.name+" countered proposal#"+std::to_string(p.id)+" price"+std::to_string(price)+"; other party must accept",{p.from,p.to});
}
void respond(State& s,const Command& c){
    auto& actor=person(s,c.actor);idle(actor);auto& p=find(s,c.target);
    if(p.kind==ProposalKind::Delivery&&p.status==ContractStatus::Active&&c.option==2){require(c.actor==p.from||c.actor==p.to,"Only delivery parties may cancel obligation");person(s,p.from).relations[p.to]=std::max(0,person(s,p.from).relations[p.to]-1);release(s,p,ContractStatus::Cancelled,"accepted delivery cancelled by "+actor.name);return;}
    require(p.status==ContractStatus::Pending&&s.tick<p.deadline,"Proposal no longer pending");
    require(c.option>=0&&c.option<=2,"Response must accept0/reject1/withdraw2");
    if(c.option==2){require(actor.id==p.from,"Only original proposer may withdraw reserved assets");release(s,p,ContractStatus::Cancelled,"withdrawn by proposer");return;}
    require(p.awaiting==actor.id||(p.awaiting==0&&actor.id!=p.from),"Not addressed to actor / awaiting other party");
    if(c.option==1){release(s,p,ContractStatus::Rejected,"rejected by "+actor.name);return;}
    require(p.author!=actor.id,"Cannot accept your own terms");
    Id recipient=p.to?p.to:actor.id;auto& other=person(s,recipient);
    require(other.body.health>0,"Recipient cannot consent");
    if(p.kind!=ProposalKind::Appointment)require(other.location==p.site,"Recipient must visit proposal site");
    if(p.kind==ProposalKind::Sale){
        Transaction{}.money(recipient,p.from,Money(p.price)*p.amount).money(recipient,100,p.amount).goods(p.reserved,other.account.inventory,p.good,p.amount).commit(s);
        emit(s,"PUBLIC tax receipt: "+std::to_string(p.amount)+" from proposal#"+std::to_string(p.id),{recipient},true);
        p.status=ContractStatus::Fulfilled;
    }else if(p.kind==ProposalKind::Loan){
        require(s.debts.size()<2000,"Debt capacity reached");
        money_from_escrow(s,p,recipient);s.debts.push_back({s.next_debt++,p.id,p.from,recipient,p.amount,p.price,Money(p.amount)+p.price,s.tick+p.duration,0});p.status=ContractStatus::Fulfilled;
    }else if(p.kind==ProposalKind::Lease){
        require(!residence(s,recipient,p.site),"Recipient already has residence right");require(live_slots(s,p.site)<2,"No lease slot available");
        require(s.leases.size()<2000,"Lease capacity reached");
        Transaction{}.money(recipient,p.from,p.price).commit(s);
        s.leases.push_back({s.next_lease++,p.id,p.from,recipient,p.site,s.tick,s.tick+p.duration,p.price,0});p.status=ContractStatus::Fulfilled;
    }else if(p.kind==ProposalKind::Delivery){
        p.status=ContractStatus::Active;p.deadline=s.tick+p.duration;
    }else{
        s.government.office={recipient,true};p.status=ContractStatus::Fulfilled;
        for(auto& q:s.proposals)if(q.id!=p.id&&q.kind==ProposalKind::Appointment&&q.status==ContractStatus::Pending)q.status=ContractStatus::Cancelled;
        emit(s,"PUBLIC: "+other.name+" accepted treasury Office; property remains public",{1,recipient},true);
    }
    p.to=recipient;emit(s,"Proposal#"+std::to_string(p.id)+" accepted; state"+std::to_string(int(p.status)),{p.from,recipient});
}
void fulfill(State& s,const Command& c){
    auto& actor=person(s,c.actor);idle(actor);auto& p=find(s,c.target);
    require(p.kind==ProposalKind::Delivery&&p.status==ContractStatus::Active,"No accepted delivery obligation");
    require(c.actor==p.to,"Only obligated recipient may deliver");require(s.tick<p.deadline,"Delivery deadline passed");
    auto& requester=person(s,p.from);require(actor.location==requester.location,"Delivery requires meeting requester");
    Transaction{}.goods(actor.account.inventory,requester.account.inventory,p.good,p.amount).commit(s);money_from_escrow(s,p,actor.id);p.status=ContractStatus::Fulfilled;
    emit(s,actor.name+" fulfilled delivery#"+std::to_string(p.id)+"; goods and reward transferred",{p.from,p.to});
}
void repay(State& s,const Command& c){
    auto& actor=person(s,c.actor);idle(actor);
    auto i=std::find_if(s.debts.begin(),s.debts.end(),[&](const Debt& d){return d.id==c.target;});require(i!=s.debts.end(),"Unknown debt");
    require(i->borrower==c.actor,"Only borrower chooses repayment from own funds");require(i->status!=1&&c.amount<=i->outstanding,"Repayment exceeds outstanding debt");
    Transaction{}.money(c.actor,i->lender,c.amount).commit(s);i->outstanding-=c.amount;if(i->outstanding==0)i->status=1;
    emit(s,actor.name+" repaid"+std::to_string(c.amount)+" on debt#"+std::to_string(i->id)+"; outstanding"+std::to_string(i->outstanding),{i->lender,i->borrower});
}
void end_lease(State& s,const Command& c){
    auto& actor=person(s,c.actor);idle(actor);auto i=std::find_if(s.leases.begin(),s.leases.end(),[&](const Lease& l){return l.id==c.target;});
    require(i!=s.leases.end()&&i->tenant==c.actor,"Only tenant may end own lease");require(i->status==0,"Lease no longer active");i->status=2;i->end=s.tick;
    emit(s,actor.name+" ended lease#"+std::to_string(i->id)+"; prepaid rent not refunded; location unchanged",{i->landlord,i->tenant});
}
void advance(State& s){
    for(auto& p:s.proposals){
        if(p.status==ContractStatus::Pending&&s.tick>=p.deadline)release(s,p,ContractStatus::Expired,"expired; escrow returned");
        else if(p.status==ContractStatus::Active&&s.tick>=p.deadline){
            auto& worker=person(s,p.to);worker.relations[p.from]=std::max(0,worker.relations[p.from]-1);
            auto& requester=person(s,p.from);requester.relations[p.to]=std::max(0,requester.relations[p.to]-1);
            release(s,p,ContractStatus::Defaulted,"delivery deadline missed; reward returned; no goods fabricated");
        }
    }
    for(auto& d:s.debts)if(d.status==0&&s.tick>=d.due){d.status=2;emit(s,"Debt#"+std::to_string(d.id)+" overdue; no automatic debit or penalty",{d.lender,d.borrower});}
    for(auto& l:s.leases)if(l.status==0&&s.tick>=l.end){l.status=1;emit(s,"Lease#"+std::to_string(l.id)+" expired; occupant not removed",{l.landlord,l.tenant});}
}
void validate(const State& s){
    require(s.proposals.size()<=2000&&s.debts.size()<=2000&&s.leases.size()<=2000,"Contract capacity exceeded");
    require(s.next_contract>0&&s.next_debt>0&&s.next_lease>0,"Invalid contract sequence");int previous=0;
    auto find_proposal=[&](int id)->const Proposal&{auto i=std::find_if(s.proposals.begin(),s.proposals.end(),[&](const Proposal&p){return p.id==id;});require(i!=s.proposals.end(),"Contract references missing proposal");return *i;};
    for(auto& p:s.proposals){
        require(p.id>previous&&p.id<s.next_contract,"Invalid proposal identity");previous=p.id;
        require(int(p.kind)>=0&&int(p.kind)<=4&&int(p.status)>=0&&int(p.status)<=6,"Invalid proposal type/status");
        person(s,p.from);location(s,p.site);if(p.to)person(s,p.to);require(p.from!=p.to,"Self contract");
        require(p.author==p.from||(p.to&&p.author==p.to),"Invalid term author");require(p.awaiting==p.from||p.awaiting==p.to||(p.awaiting==0&&p.status!=ContractStatus::Pending),"Invalid consent turn");
        require(int(p.good)>=0&&int(p.good)<goods&&p.amount>=1&&p.amount<=1000&&p.price>=0&&p.price<=100&&p.duration>=1&&p.duration<=1440&&p.deadline>=0&&p.deadline<=1001440,"Invalid proposal terms");
        if(p.source_site)require(location(s,p.source_site).owner==p.from&&p.source_site==p.site&&p.kind==ProposalKind::Sale,"Invalid reserved source");
        bool pending=p.status==ContractStatus::Pending,active=p.status==ContractStatus::Active;
        if(pending)require(p.deadline>s.tick&&p.author!=p.awaiting,"Expired/unconsented proposal");
        if(active)require(p.kind==ProposalKind::Delivery&&p.to&&p.deadline>s.tick,"Invalid active obligation");
        Money expected=(pending&&p.kind==ProposalKind::Loan)?p.amount:((pending||active)&&p.kind==ProposalKind::Delivery?p.price:0);
        require(p.escrow==expected,"Invalid contract money escrow");Inventory expected_stock;
        if(pending&&p.kind==ProposalKind::Sale)expected_stock[p.good]=p.amount;
        require(p.reserved.units==expected_stock.units,"Invalid reserved contract goods");
        if(p.kind==ProposalKind::Sale||p.kind==ProposalKind::Delivery)require(p.amount<=20&&p.price>=1,"Invalid goods contract terms");
        if(p.kind==ProposalKind::Lease)require(p.amount==1&&p.site==15&&location(s,p.site).owner==p.from&&p.price>=1,"Invalid lease owner/terms");
        if(p.kind==ProposalKind::Appointment)require(p.from==1&&p.to&&p.amount==1&&p.price==0,"Invalid appointment authority");
    }
    previous=0;std::vector<int> debt_proposals,lease_proposals;
    for(auto& d:s.debts){require(d.id>previous&&d.id<s.next_debt,"Invalid debt identity");previous=d.id;auto& p=find_proposal(d.proposal);
        require(p.kind==ProposalKind::Loan&&p.status==ContractStatus::Fulfilled&&p.from==d.lender&&p.to==d.borrower&&d.principal==p.amount&&d.interest==p.price,"Debt terms differ from consent");
        require(d.outstanding>=0&&d.outstanding<=d.principal+d.interest&&d.status>=0&&d.status<=2&&(d.status==1)==(d.outstanding==0),"Invalid debt balance/status");
        require(d.due>=0&&d.due<=1001440&&(d.status!=0||d.due>s.tick),"Invalid debt due time");
        require(std::find(debt_proposals.begin(),debt_proposals.end(),d.proposal)==debt_proposals.end(),"Duplicate loan issuance");debt_proposals.push_back(d.proposal);
    }
    for(auto& p:s.proposals)if(p.kind==ProposalKind::Loan&&p.status==ContractStatus::Fulfilled)require(std::find(debt_proposals.begin(),debt_proposals.end(),p.id)!=debt_proposals.end(),"Issued loan lacks debt");
    previous=0;
    for(auto& l:s.leases){require(l.id>previous&&l.id<s.next_lease,"Invalid lease identity");previous=l.id;auto& p=find_proposal(l.proposal);
        require(p.kind==ProposalKind::Lease&&p.status==ContractStatus::Fulfilled&&p.from==l.landlord&&p.to==l.tenant&&p.site==l.site&&p.price==l.rent,"Lease terms differ from consent");
        require(l.start>=0&&l.end>=l.start&&l.start<=s.tick&&l.end<=1001440&&l.status>=0&&l.status<=2&&(l.status!=0||l.end>s.tick),"Invalid lease time/status");
        require(std::find(lease_proposals.begin(),lease_proposals.end(),l.proposal)==lease_proposals.end(),"Duplicate lease right");lease_proposals.push_back(l.proposal);
    }
    for(auto& p:s.proposals)if(p.kind==ProposalKind::Lease&&p.status==ContractStatus::Fulfilled)require(std::find(lease_proposals.begin(),lease_proposals.end(),p.id)!=lease_proposals.end(),"Accepted lease lacks right");
    require(live_slots(s,15)<=2,"Lease capacity exceeded");
}
}
