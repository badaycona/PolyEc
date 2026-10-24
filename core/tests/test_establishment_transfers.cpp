#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <exception>
#include <iostream>
#include <limits>
#include <new>
#include <sstream>
#include <stdexcept>

// Fault injection belongs only to this executable. It exercises actual vector
// allocations in commands; the production core has no test bypass or flag.
int allocations_until_failure=-1;
std::size_t failing_allocation_bytes=0;
const char* fault_phase="none";
int fault_index=-1;
std::size_t excluded_debug_proxies=0;
void* operator new(std::size_t bytes){
    bool domainAllocation=true;
#if defined(_MSC_VER) && _ITERATOR_DEBUG_LEVEL > 0
    // MSVC xstring's noexcept move allocates an iterator proxy. Failing that
    // debugger bookkeeping terminates the process rather than testing a
    // recoverable command allocation. Keep iterator checks enabled; exclude
    // only this size from injected failures. Site/fact buffers and tree nodes
    // on the transfer paths are larger and remain covered in both builds.
    // A single authorization window is also this size: its buffer failure is
    // covered by Release injection; Debug cannot distinguish it from a proxy.
    static_assert(sizeof(polyec::Establishment)>sizeof(std::_Container_proxy));
    static_assert(sizeof(polyec::Fact)>sizeof(std::_Container_proxy));
    domainAllocation=bytes!=sizeof(std::_Container_proxy);
    if(!domainAllocation&&allocations_until_failure>=0)++excluded_debug_proxies;
#endif
    if(domainAllocation){
        if(allocations_until_failure==0){allocations_until_failure=-1;failing_allocation_bytes=bytes;throw std::bad_alloc();}
        if(allocations_until_failure>0)--allocations_until_failure;
    }
    if(void* memory=std::malloc(bytes?bytes:1))return memory;
    throw std::bad_alloc();
}
void operator delete(void* memory)noexcept{std::free(memory);}
void operator delete(void* memory,std::size_t)noexcept{std::free(memory);}
using namespace polyec;
using Operation=EstablishmentTransferOperation;
namespace {
int checks=0;
void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
uint64_t create(Realm& world,uint64_t actor=65,EstablishmentOwnerKind kind=EstablishmentOwnerKind::Person,uint64_t owner=65){
    auto result=world.execute(EstablishmentCommand{actor,world.person(actor).decisions+1,kind,owner,world.person(actor).region,EstablishmentIndustry::ClothWorkshop,1,10});
    if(!result.success)throw std::runtime_error(result.reason);return result.session;
}
RealmResult move(Realm& world,uint64_t actor,uint64_t site,Operation operation,int good,int64_t amount){
    return world.execute(EstablishmentTransferCommand{actor,world.person(actor).decisions+1,site,operation,good,amount});
}
int64_t held(const Realm& world,size_t good){
    int64_t total=0;for(int id=1;id<=world.count();++id)total+=world.person(id).inventory[good];
    for(const auto& province:world.view().provinces)total+=province.stock[good];
    for(const auto& site:world.establishments())total+=site.stock[good];return total;
}
void reject(Realm& world,EstablishmentTransferCommand command,const char* label){
    const auto before=world.fingerprint();const auto result=world.execute(command);
    check(!result.success&&world.fingerprint()==before,label);
}
Realm dead_owner(const Realm& source,uint64_t actor){
    std::istringstream input(source.serialize("transfer-test"));std::vector<std::string> words;std::string word;
    while(input>>word)words.push_back(word);
    const size_t people=size_t(source.count());constexpr size_t personRecord=23+kLegacyCapabilities.size()+kGoodIdCount;bool found=false;
    for(size_t i=100;i+1+personRecord*people<words.size();++i){
        if(words[i]!=std::to_string(people))continue;
        bool dense=true;for(size_t row=0;row<people;++row)if(words[i+1+personRecord*row]!=std::to_string(row+1)){dense=false;break;}
        if(dense){words[i+1+personRecord*(actor-1)+14]="0";found=true;break;}
    }
    if(!found)throw std::runtime_error("dead-owner fixture Person section");
    std::ostringstream out;for(const auto& token:words)out<<token<<' ';
    Realm result(1,1,128);std::string error;if(!result.deserialize(out.str(),"transfer-test",error))throw std::runtime_error(error);return result;
}
void replay(){
    Realm world(71,2,128);const uint64_t worker=65;const int region=world.person(worker).region;
    if(!world.execute(RealmCommand{worker,world.person(worker).decisions+1,{Intent::Buy,region,int(GoodId::Fiber),2}}).success)throw std::runtime_error("replay Fiber buy");
    const auto site=create(world);
    if(!move(world,worker,site,Operation::DepositGoods,9,2).success)throw std::runtime_error("replay deposit");
    if(!world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Submit,0,0,32,0,1000,ActivityKind::Work,region,site}).success)throw std::runtime_error("replay Work");
    int64_t prior=0;
    for(int64_t tick:{0,15,16,17,32,64,128}){
        world.advance_ticks(tick-prior);prior=tick;
        auto observed=world.establishments()[0];
        std::cout<<tick<<' '<<world.fingerprint()<<' '<<observed.cash<<' '<<observed.stock[9]<<' '<<observed.stock[11]<<'\n';
    }
}
void market_replay(){
    Realm world(71,2,128);const uint64_t worker=65;const int region=world.person(worker).region;
    auto result=world.execute(RealmCommand{worker,world.person(worker).decisions+1,
        {Intent::Buy,region,int(GoodId::Fiber),2,0}});
    if(!result.success)throw std::runtime_error("market replay Fiber buy");
    const auto site=create(world,worker);
    if(!move(world,worker,site,Operation::DepositGoods,int(GoodId::Fiber),2).success)
        throw std::runtime_error("market replay Fiber deposit");
    result=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,0,32,0,1000,ActivityKind::Work,region,site});
    if(!result.success)throw std::runtime_error("market replay Work submit");
    world.advance_ticks(64);
    if(world.establishments().empty()||world.establishments()[0].stock[size_t(GoodId::Cloth)]!=1)
        throw std::runtime_error("market replay Cloth production");
    if(!move(world,worker,site,Operation::WithdrawGoods,int(GoodId::Cloth),1).success)
        throw std::runtime_error("market replay Cloth withdrawal");
    result=world.execute(RealmCommand{worker,world.person(worker).decisions+1,
        {Intent::Sell,region,int(GoodId::Cloth),1,0}});
    if(!result.success)throw std::runtime_error("market replay Cloth sale");

    const auto province=world.view().provinces[size_t(region)];
    uint64_t buyer=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto&candidate=world.person(id);
        if(id!=worker&&candidate.alive&&candidate.destination==-1&&candidate.region==region&&
           candidate.family==Family::Civilian&&candidate.age_days>=6*365&&
           world.available_funds(id)>=province.price[size_t(GoodId::Cloth)]+1){buyer=id;break;}
    }
    if(!buyer)throw std::runtime_error("market replay buyer selection");
    result=world.execute(RealmCommand{buyer,world.person(buyer).decisions+1,
        {Intent::Buy,region,int(GoodId::Cloth),1,0}});
    if(!result.success||!world.invariant())throw std::runtime_error("market replay Cloth purchase");
    const auto finalProvince=world.view().provinces[size_t(region)];
    const auto finalSite=world.establishments()[0];
    std::cout<<world.fingerprint()<<' '<<world.person(worker).cash<<' '<<buyer<<' '
             <<world.person(buyer).cash<<' '<<finalSite.cash<<' '
             <<finalSite.stock[size_t(GoodId::Fiber)]<<' '<<finalSite.stock[size_t(GoodId::Cloth)]<<' '
             <<finalProvince.firm<<' '<<finalProvince.treasury<<' '
             <<finalProvince.stock[size_t(GoodId::Cloth)]<<' '<<finalProvince.tax_receipts<<' '
             <<finalProvince.sales_tax_remainder<<' '<<world.total_money()<<' '<<int(world.invariant())<<'\n';
}
}
#include "work_authorization_checks.hpp"
int main(int argc,char** argv){try{
    if(argc==2&&std::string(argv[1])=="--authorization-replay"){authorization_replay();return 0;}
    if(argc==2&&std::string(argv[1])=="--market-replay"){market_replay();return 0;}
    std::set_terminate([]{
        allocations_until_failure=-1;
        std::fprintf(stderr,"Unhandled failure in transfer test: phase=%s index=%d bytes=%zu checks=%d\n",fault_phase,fault_index,failing_allocation_bytes,checks);
        try{if(auto error=std::current_exception())std::rethrow_exception(error);}
        catch(const std::exception& error){std::fprintf(stderr,"Exception: %s\n",error.what());}
        catch(...){std::fprintf(stderr,"Non-standard exception\n");}
        std::fflush(stderr);std::_Exit(3);
    });
    if(argc==2&&std::string(argv[1])=="--replay"){replay();return 0;}
    Realm world(71,2,128);const uint64_t worker=65;const int region=world.person(worker).region;
    const auto money=world.total_money(),fiber=held(world,9);
    check(world.execute(RealmCommand{worker,1,{Intent::Buy,region,int(GoodId::Fiber),2}}).success,"real command buys two Fiber");
    check(world.invariant(),"a completed local market purchase preserves the Realm invariant");
    const auto sellCash=world.person(worker).cash;
    check(world.execute(RealmCommand{worker,world.person(worker).decisions+1,{Intent::Sell,region,int(GoodId::Fiber),1}}).success,"canonical GoodId sells one Fiber");
    check(world.person(worker).inventory[9]==1&&world.view().provinces[size_t(region)].stock[9]>0&&
          world.person(worker).cash>sellCash,
          "canonical Sell moves inventory to the market and pays the seller");
    check(world.total_money()==money,
          "canonical Sell transfers existing money without changing the realm total");
    check(world.invariant(),"canonical Sell leaves the complete Realm invariant valid");
    check(world.execute(RealmCommand{worker,world.person(worker).decisions+1,{Intent::Buy,region,int(GoodId::Fiber),1}}).success,
          "canonical GoodId buys Fiber back after Sell");
    const auto site=create(world);
    const auto deposited=move(world,worker,site,Operation::DepositGoods,9,2);
    check(deposited.success&&deposited.fact>0,"goods deposit commits a fact");
    check(world.person(worker).inventory[9]==0&&world.establishments()[0].stock[9]==2,"two Fiber moved from owner to site");
    check(held(world,9)==fiber&&world.total_money()==money&&world.invariant(),"Buy, creation and internal deposit conserve totals");
    const auto personalCash=world.person(worker).cash;
    check(move(world,worker,site,Operation::DepositCash,-1,5).success&&world.establishments()[0].cash==15,"cash refill draws from owner");
    check(move(world,worker,site,Operation::WithdrawCash,-1,3).success&&world.establishments()[0].cash==12&&world.person(worker).cash==personalCash-2,"cash withdraw returns to owner");
    check(move(world,worker,site,Operation::WithdrawGoods,9,1).success&&world.person(worker).inventory[9]==1,"goods withdrawal returns to owner");
    check(move(world,worker,site,Operation::DepositGoods,9,1).success&&held(world,9)==fiber,"goods roundtrip preserves stock");
    const EstablishmentTransferCommand valid{worker,world.person(worker).decisions+1,site,Operation::WithdrawGoods,9,1};
    for(int64_t amount:{int64_t(0),int64_t(-1),int64_t(3),std::numeric_limits<int64_t>::max()}){auto command=valid;command.amount=amount;reject(world,command,"invalid or unfunded amount rejects atomically");}
    for(int good:{-1,27,99}){auto command=valid;command.good=good;reject(world,command,"invalid canonical good rejects atomically");}
    {auto command=valid;command.operation=static_cast<Operation>(99);reject(world,command,"unknown operation rejects atomically");}
    {auto command=valid;command.establishment_id=0;reject(world,command,"unknown site rejects atomically");}
    {auto command=valid;command.actor=0;reject(world,command,"unknown actor rejects atomically");}
    {auto command=valid;command.sequence--;reject(world,command,"stale transfer rejects atomically");}
    {auto command=valid;command.actor=66;command.sequence=world.person(66).decisions+1;reject(world,command,"local nonowner cannot withdraw");}
    {auto command=valid;command.operation=Operation::DepositCash;reject(world,command,"cash transfer cannot name a good");}
    {auto command=valid;command.operation=Operation::DepositGoods;command.good=10;reject(world,command,"Beeswax cannot be taken from empty holdings as Fiber");}
    {auto dead=dead_owner(world,worker);reject(dead,valid,"deceased owner cannot transfer remaining site assets");}
    {
        Realm carrying=Realm::scenario(71,4,35,-1,-1,10000);const uint64_t trader=4;const int local=carrying.person(trader).region;
        auto buy=[&]{return carrying.execute(RealmCommand{trader,carrying.person(trader).decisions+1,{Intent::Buy,local,int(GoodId::Fiber),16}}).success;};
        const auto id=create(carrying,trader,EstablishmentOwnerKind::Person,trader);
        check(buy(),"funded carrying fixture buys first full load");
        check(move(carrying,trader,id,Operation::DepositGoods,9,16).success,"carrying fixture deposits first load");
        check(buy(),"carrying fixture buys second full load");
        reject(carrying,{trader,carrying.person(trader).decisions+1,id,Operation::WithdrawGoods,9,1},"site withdrawal cannot bypass personal carrying capacity");
    }
    auto traveling=world;
    check(traveling.execute(RealmCommand{worker,traveling.person(worker).decisions+1,{Intent::Travel,(region+1)%3}}).success,"owner begins real travel");
    {auto command=valid;command.sequence=traveling.person(worker).decisions+1;reject(traveling,command,"traveling owner cannot transfer");}
    traveling.advance(10);
    {auto command=valid;command.sequence=traveling.person(worker).decisions+1;reject(traveling,command,"owner in another region cannot transfer");}

    Realm publicWorld(71,2,128);const auto publicSite=create(publicWorld,1,EstablishmentOwnerKind::Public,2);
    const auto treasury=publicWorld.view().provinces[2].treasury;const auto actorCash=publicWorld.person(1).cash;
    check(move(publicWorld,1,publicSite,Operation::DepositCash,-1,7).success&&publicWorld.view().provinces[2].treasury==treasury-7&&publicWorld.person(1).cash==actorCash,"public refill draws from treasury rather than private cash");
    const auto stock=publicWorld.view().provinces[2].stock[9];
    check(move(publicWorld,1,publicSite,Operation::DepositGoods,9,2).success&&publicWorld.view().provinces[2].stock[9]==stock-2,"public stock deposit uses Province inventory");
    check(move(publicWorld,1,publicSite,Operation::WithdrawGoods,9,1).success&&publicWorld.view().provinces[2].stock[9]==stock-1,"public withdrawal returns to Province");
    reject(publicWorld,{65,publicWorld.person(65).decisions+1,publicSite,Operation::WithdrawCash,-1,1},"local civilian cannot manage public site");
    check(publicWorld.invariant(),"public transfer invariant");

    Realm organization(71,2,128);const auto orgId=uint64_t(1+4*organization.person(worker).region);
    check(organization.execute(RealmCommand{worker,1,{Intent::Join,region,0,20,30}}).success,"real Join funds organization account");
    const auto orgSite=create(organization,worker,EstablishmentOwnerKind::Organization,orgId);
    const auto memberCash=organization.person(worker).cash;const auto orgMoney=organization.total_money();
    check(move(organization,worker,orgSite,Operation::DepositCash,-1,2).success&&organization.person(worker).cash==memberCash,"organization deposit uses account rather than member cash");
    check(move(organization,worker,orgSite,Operation::WithdrawCash,-1,1).success&&organization.total_money()==orgMoney&&organization.invariant(),"organization cash roundtrip conserved");
    reject(organization,{worker,organization.person(worker).decisions+1,orgSite,Operation::DepositGoods,9,1},"unmodeled organization inventory rejects");
    reject(organization,{66,organization.person(66).decisions+1,orgSite,Operation::WithdrawCash,-1,1},"nonmember cannot withdraw organization cash");

    authorization_checks();
    int allocationFailures=0;bool creationSucceeded=false;
    for(int failedAllocation=0;failedAllocation<16;++failedAllocation){
        Realm allocationWorld(71,2,128);const auto before=allocationWorld.fingerprint();bool failed=false;
        RealmResult result;
        fault_phase="fresh creation";fault_index=failedAllocation;
        allocations_until_failure=failedAllocation;
        try{result=allocationWorld.execute(EstablishmentCommand{worker,1,EstablishmentOwnerKind::Person,worker,region,EstablishmentIndustry::ClothWorkshop,1,10});}
        catch(const std::bad_alloc&){failed=true;}
        allocations_until_failure=-1;
        fault_phase="none";
        if(failed){++allocationFailures;check(allocationWorld.fingerprint()==before,"every creation allocation failure preserves money, IDs, facts and decisions");}
        else{creationSucceeded=result.success;break;}
    }
    check(creationSucceeded&&allocationFailures>=2,"fault injection visits all creation allocations until successful commit");
    {
        Realm populated(71,2,128);for(int siteIndex=0;siteIndex<8;++siteIndex)create(populated);
        int failuresAtGrowth=0;bool grew=false;
        for(int allocation=0;allocation<128;++allocation){
            Realm growing=populated;const auto before=growing.fingerprint();bool failed=false;uint64_t newSite=0;
            fault_phase="populated creation";fault_index=allocation;allocations_until_failure=allocation;
            try{newSite=create(growing);}catch(const std::bad_alloc&){failed=true;}
            allocations_until_failure=-1;
            fault_phase="none";
            if(failed){++failuresAtGrowth;check(growing.fingerprint()==before,"copying prior site records during reserve rejects without mutation on allocation failure");}
            else{grew=newSite!=0&&growing.establishments().size()==9;break;}
        }
        check(grew&&failuresAtGrowth>=2,"fault injection covers populated vector growth through successful commit");
    }
    {
        Realm allocationWorld(71,2,128);const auto id=create(allocationWorld);
        // Fact capacity is prepared geometrically; fill the initial eight slots.
        for(int count=0;count<7;++count)check(move(allocationWorld,worker,id,Operation::DepositCash,-1,1).success,"prepare fact capacity boundary");
        const auto before=allocationWorld.fingerprint();bool failed=false;
        fault_phase="transfer fact append";fault_index=0;allocations_until_failure=0;
        try{move(allocationWorld,worker,id,Operation::DepositCash,-1,1);}catch(const std::bad_alloc&){failed=true;}
        allocations_until_failure=-1;
        fault_phase="none";
        check(failed&&allocationWorld.fingerprint()==before,"transfer fact allocation failure precedes cash mutation");
    }

    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Submit,0,0,32,0,1000,ActivityKind::Work,region,site}).success,"Work follows real transferred inputs");
    auto uninterrupted=world;uninterrupted.advance_ticks(64);
    world.advance_ticks(16);const auto saved=world.serialize("transfer-test");Realm continued(1,1,128);std::string error;
    check(continued.deserialize(saved,"transfer-test",error),error.c_str());continued.advance_ticks(48);
    check(continued.fingerprint()==uninterrupted.fingerprint()&&continued.invariant(),"command-only stock cycle replays across mid-work save");
    check(continued.establishments()[0].stock[9]==1&&continued.establishments()[0].stock[11]==1,"real supplied workshop produces one batch at capacity one");
    const auto siteId=continued.establishments()[0].id;
    const auto cloth=size_t(GoodId::Cloth);
    const auto beforeSale=continued.view().provinces[size_t(region)];
    const int64_t sellerCash=continued.person(worker).cash;
    const int64_t salePrice=std::max(1,beforeSale.price[cloth]-1);
    check(move(continued,worker,siteId,Operation::WithdrawGoods,int(GoodId::Cloth),1).success,
          "owner withdraws produced Cloth through the Realm transfer command");
    const auto sold=continued.execute(RealmCommand{worker,continued.person(worker).decisions+1,
        {Intent::Sell,region,int(GoodId::Cloth),1,0}});
    const auto afterSale=continued.view().provinces[size_t(region)];
    check(sold.success&&continued.person(worker).cash==sellerCash+salePrice&&
          afterSale.firm==beforeSale.firm-salePrice&&afterSale.stock[cloth]==beforeSale.stock[cloth]+1,
          "the workshop owner sells one produced Cloth at the existing local-market price");

    uint64_t buyer=0;
    for(uint64_t id=1;id<=uint64_t(continued.count());++id){
        const auto&candidate=continued.person(id);
        if(id!=worker&&candidate.alive&&candidate.destination==-1&&candidate.region==region&&
           candidate.family==Family::Civilian&&candidate.age_days>=6*365&&
           continued.available_funds(id)>=afterSale.price[cloth]+1){buyer=id;break;}
    }
    check(buyer!=0,"the same region has an adult civilian able to afford one Cloth");
    const int64_t buyerCash=continued.person(buyer).cash;
    const int64_t buyerInventory=continued.person(buyer).inventory[cloth];
    const int64_t gross=afterSale.price[cloth]+1;
    const int64_t taxNumerator=gross*afterSale.tax+afterSale.sales_tax_remainder;
    const int64_t expectedTax=taxNumerator/100;
    const int expectedRemainder=int(taxNumerator%100);
    const auto bought=continued.execute(RealmCommand{buyer,continued.person(buyer).decisions+1,
        {Intent::Buy,region,int(GoodId::Cloth),1,0}});
    const auto afterBuy=continued.view().provinces[size_t(region)];
    check(bought.success&&continued.person(buyer).cash==buyerCash-gross&&
          continued.person(buyer).inventory[cloth]==buyerInventory+1&&
          afterBuy.stock[cloth]==afterSale.stock[cloth]-1,
          "a different resident buys the Cloth through the same region market");
    check(afterBuy.treasury==afterSale.treasury+expectedTax&&
          afterBuy.tax_receipts==afterSale.tax_receipts+expectedTax&&
          afterBuy.sales_tax_remainder==expectedRemainder&&
          afterBuy.firm==afterSale.firm+gross-expectedTax&&continued.total_money()==money&&continued.invariant(),
          "purchase tax, fractional remainder, market balances and total money match the ledger");
    const auto completedSave=continued.serialize("transfer-test");
    Realm completedLoad(1,1,128);
    check(completedLoad.deserialize(completedSave,"transfer-test",error)&&
          completedLoad.fingerprint()==continued.fingerprint()&&completedLoad.invariant(),
          "the finished production and market chain survives a final save/load roundtrip");

    {
        constexpr int urbanRegion=14;
        Realm urban(49202,26,256);
        const uint64_t governor=urbanRegion+2;
        const uint64_t otherGovernor=urbanRegion+3;
        const auto beforeMoney=urban.total_money();
        const auto beforeTreasury=urban.view().provinces[size_t(urbanRegion)].treasury;
        const int64_t maximumGrant=beforeTreasury/20;
        const auto urbanChoices=urban.candidates(governor);
        const auto urbanMask=urban.mask(governor,urbanChoices);
        check(realm_actions==400&&realm_obs==5182&&
              urban.model_contract()=="realm-household-fund-goods30-config-v44",
              "council approval and refusal actions add 54 choices and receive a new model contract");
        check(urban.observation(governor)[size_t(realm_municipal_fund_observation_index)]==0.f&&
              urban.observation(65)[size_t(realm_municipal_fund_observation_index)]==0.f,
              "the new municipal balance feature starts at zero and reveals no council balance to a civilian policy");
        for(int i=0;i<realm_municipal_fund_candidate_count;++i){const int percent=kMunicipalGrantPercentages[size_t(i)];
            check(urbanChoices[size_t(realm_municipal_fund_candidate_begin+i)].kind==Intent::FundMunicipal&&
                  urbanChoices[size_t(realm_municipal_fund_candidate_begin+i)].good==percent&&
                  urbanChoices[size_t(realm_municipal_fund_candidate_begin+i)].amount==municipal_grant_amount(beforeTreasury,percent)&&
                  urbanMask[size_t(realm_municipal_fund_candidate_begin+i)]==1.f,
                  "Governance receives a legal candidate for each 1, 2, 3 or 5 percent grant");}
        const auto civilianBefore=urban.fingerprint();
        auto denied=urban.execute(MunicipalBudgetCommand{65,urban.person(65).decisions+1,urbanRegion,maximumGrant});
        check(!denied.success&&urban.fingerprint()==civilianBefore,
              "a civilian cannot move regional treasury money into an urban fund");

        const auto overLimitBefore=urban.fingerprint();
        denied=urban.execute(MunicipalBudgetCommand{governor,urban.person(governor).decisions+1,
                                                    urbanRegion,maximumGrant+1});
        check(!denied.success&&urban.fingerprint()==overLimitBefore,
              "a grant one đồng above the 5 percent limit is rejected without partial changes");
        const auto grant=urban.execute(RealmCommand{governor,urban.person(governor).decisions+1,
            urbanChoices[size_t(realm_municipal_fund_candidate_begin+3)]});
        check(grant.success&&urban.municipal_funds()[size_t(urbanRegion)]==maximumGrant&&
              urban.view().provinces[size_t(urbanRegion)].treasury==beforeTreasury-maximumGrant&&
              urban.total_money()==beforeMoney&&urban.invariant()&&
              std::abs(urban.observation(governor)[size_t(realm_municipal_fund_observation_index)]-0.05f)<1e-6f&&
              urban.observation(65)[size_t(realm_municipal_fund_observation_index)]==0.f,
              "the learned-policy candidate transfers exactly 5 percent from treasury to the separate urban account");
        const auto grantFacts=urban.view(governor).facts;
        check(std::any_of(grantFacts.begin(),grantFacts.end(),[&](const Fact& fact){return fact.id==grant.fact&&fact.kind==229&&fact.amount==maximumGrant;}),
              "the successful transfer leaves a readable fact with the amount and its cause");
        const auto afterGrant=urban.fingerprint();
        denied=urban.execute(MunicipalBudgetCommand{governor,urban.person(governor).decisions+1,urbanRegion,1});
        check(!denied.success&&urban.fingerprint()==afterGrant,
              "the same region cannot receive a second grant during the same 30 day period");
        const auto usedMask=urban.mask(governor,urban.candidates(governor));
        check(std::none_of(usedMask.begin()+realm_municipal_fund_candidate_begin,
                           usedMask.begin()+realm_municipal_fund_candidate_begin+realm_municipal_fund_candidate_count,
                           [](float enabled){return enabled>.5f;}),
              "all four candidate grants are masked until the next 30 day period");

        const auto wrongRegionBefore=urban.fingerprint();
        denied=urban.execute(EstablishmentCommand{otherGovernor,urban.person(otherGovernor).decisions+1,
            EstablishmentOwnerKind::Municipal,uint64_t(urbanRegion),urbanRegion,
            EstablishmentIndustry::TeaGarden,1,50});
        check(!denied.success&&urban.fingerprint()==wrongRegionBefore,
              "a different region's authority cannot create an urban owned workshop here");

        const auto cityChoices=urban.candidates(governor);
        const auto cityMask=urban.mask(governor,cityChoices);
        const size_t teaGarden=size_t(realm_municipal_special_candidate_begin+1);
        check(cityChoices[teaGarden].kind==Intent::CreateEstablishment&&
              cityChoices[teaGarden].good==int(EstablishmentIndustry::TeaGarden)&&
              cityChoices[teaGarden].amount==50&&cityChoices[teaGarden].term==-2&&cityMask[teaGarden]==1.f,
              "after funding, Governance can choose a 50 đồng city-owned Tea Garden from the policy action list");
        const auto siteResult=urban.execute(RealmCommand{governor,urban.person(governor).decisions+1,cityChoices[teaGarden]});
        const auto site=siteResult.session;
        check(siteResult.success&&urban.establishments().size()==1&&
              urban.establishments()[0].owner_kind==EstablishmentOwnerKind::Municipal&&
              urban.establishments()[0].cash==50&&
              urban.municipal_funds()[size_t(urbanRegion)]==maximumGrant-50,
              "the urban account funds a workshop and ownership remains separate from the regional treasury");
        uint64_t worker=0;
        for(uint64_t id=1;id<=uint64_t(urban.count());++id){const auto& person=urban.person(id);
            if(person.alive&&person.destination==-1&&person.region==urbanRegion&&
               person.family==Family::Civilian&&person.age_days>=14*365){worker=id;break;}}
        check(worker!=0,"the same region has an adult resident who can choose whether to work");
        const auto authorization=urban.execute(WorkAuthorizationCommand{governor,
            urban.person(governor).decisions+1,site,worker,WorkAuthorizationOperation::Set,
            urban.game_tick(),urban.game_tick()+32});
        const auto work=urban.execute(ScheduleCommand{worker,urban.person(worker).decisions+1,
            ScheduleCommand::Operation::Submit,0,urban.game_tick(),urban.game_tick()+32,0,1000,
            ActivityKind::Work,urbanRegion,site});
        check(authorization.success&&work.success,"work in the urban funded workshop still requires authorization and a worker's own Work order");
        urban.advance_ticks(64);
        check(urban.establishments()[0].source_units==24&&
              urban.establishments()[0].stock[size_t(GoodId::Tea)]==1&&
              urban.invariant()&&urban.total_money()==beforeMoney,
              "the existing daily production pipeline creates one Tea without creating money or granting free goods");
        urban.advance(29);
        const int64_t nextPeriodLimit=urban.view().provinces[size_t(urbanRegion)].treasury/20;
        const auto nextGrant=urban.execute(MunicipalBudgetCommand{governor,
            urban.person(governor).decisions+1,urbanRegion,nextPeriodLimit});
        check(nextGrant.success&&urban.municipal_funds()[size_t(urbanRegion)]==maximumGrant-50+nextPeriodLimit&&
              urban.invariant()&&urban.total_money()==beforeMoney,
              "a new grant becomes available after 30 days and is recalculated from the then current treasury");

        const std::string urbanSave=urban.serialize("urban-civic-fund-test");
        check(urbanSave.rfind("POLYEC_REALM_67 ",0)==0&&
              urbanSave.find(" MUNICIPAL_FUNDS_1 27 ")!=std::string::npos,
              "new saves store each urban fund and its most recent grant period");
        Realm resumed(1,1,128);
        check(resumed.deserialize(urbanSave,"urban-civic-fund-test",error)&&
              resumed.fingerprint()==urban.fingerprint()&&resumed.municipal_funds()==urban.municipal_funds()&&
              resumed.establishments()[0].owner_kind==EstablishmentOwnerKind::Municipal&&resumed.invariant(),
              "save/load preserves municipal money, grant period, workshop ownership and production exactly");
        const auto protectedFingerprint=resumed.fingerprint();
        auto badHeader=urbanSave;const auto fundTag=badHeader.find("MUNICIPAL_FUNDS_1 27");
        check(fundTag!=std::string::npos,"save contains the versioned municipal ledger header");
        badHeader.replace(fundTag,std::string("MUNICIPAL_FUNDS_1 27").size(),"MUNICIPAL_FUNDS_1 26");
        check(!resumed.deserialize(badHeader,"urban-civic-fund-test",error)&&resumed.fingerprint()==protectedFingerprint,
              "save reader rejects a fund count that does not match the world without replacing the live world");
        auto badBalance=urbanSave;
        const auto fundAmount=urban.municipal_funds()[size_t(urbanRegion)];
        const std::string savedPair=std::to_string(fundAmount)+" 1 ";
        const auto pairAt=badBalance.find(savedPair,fundTag);
        check(pairAt!=std::string::npos,"save includes the current urban balance and grant period");
        badBalance.replace(pairAt,savedPair.size(),"-1 1 ");
        check(!resumed.deserialize(badBalance,"urban-civic-fund-test",error)&&resumed.fingerprint()==protectedFingerprint,
              "save reader rejects a negative municipal balance without changing the loaded world");
        auto futureGrant=urbanSave;const auto futurePair=futureGrant.find(savedPair,fundTag);
        futureGrant.replace(futurePair,savedPair.size(),std::to_string(fundAmount)+" 99 ");
        check(!resumed.deserialize(futureGrant,"urban-civic-fund-test",error)&&resumed.fingerprint()==protectedFingerprint,
              "save reader rejects a grant period later than the saved game day without changing the loaded world");

        Realm legacySource(49202,26,256);
        auto legacy48=legacySource.serialize("urban-civic-fund-test");
        test_fixtures::downgrade_save_50_to_49(legacy48);
        test_fixtures::downgrade_save_49_to_48(legacy48);
        Realm migrated(1,1,128);
        const bool migratedOk=migrated.deserialize(legacy48,"urban-civic-fund-test",error);
        const auto migratedFunds=migrated.municipal_funds();
        check(migratedOk&&std::all_of(migratedFunds.begin(),migratedFunds.end(),
                          [](int64_t amount){return amount==0;})&&migrated.invariant(),
              "schema48 saves load with empty urban accounts and valid existing regional money");
    }
    {
        constexpr int relief_region=14;
        constexpr uint64_t relief_governor=uint64_t(relief_region+2);
        Realm relief(49203,26,256);
        const int64_t grant=relief.view().provinces[size_t(relief_region)].treasury/20;
        check(grant>0&&relief.execute(MunicipalBudgetCommand{relief_governor,
                  relief.person(relief_governor).decisions+1,relief_region,grant}).success,
              "Governance places an existing five percent grant in the local municipal account");
        const int64_t fund_before=relief.municipal_funds()[size_t(relief_region)];
        const int64_t expected=std::min<int64_t>({1000000,fund_before,std::max<int64_t>(1,fund_before/10)});
        std::array<Candidate,realm_actions> choices{};
        const auto observation=relief.observation(relief_governor,choices);
        const auto action=choices[size_t(realm_municipal_relief_candidate)];
        check(action.kind==Intent::Aid&&action.target==relief_region&&action.good==1&&action.amount==expected&&
              relief.mask(relief_governor,choices)[size_t(realm_municipal_relief_candidate)]==1.f&&
              Realm::action_name(action)=="Cứu trợ từ quỹ đô thị"&&
              observation[size_t(realm_state+realm_municipal_relief_candidate*realm_features+10)]==1.f,
              "the local Aid candidate identifies municipal money and the policy sees that source");
        const int64_t treasury_before=relief.view().provinces[size_t(relief_region)].treasury;
        const int64_t firm_before=relief.view().provinces[size_t(relief_region)].firm;
        const int unrest_before=relief.view().provinces[size_t(relief_region)].unrest;
        const int64_t money_before=relief.total_money();
        std::vector<int64_t> cash_before(size_t(relief.count()+1));
        int civilian_count=0;
        for(uint64_t id=1;id<=uint64_t(relief.count());++id){
            cash_before[size_t(id)]=relief.person(id).cash;
            const auto&citizen=relief.person(id);
            civilian_count+=citizen.alive&&citizen.family==Family::Civilian&&citizen.region==relief_region;
        }
        check(civilian_count>0,"the municipal relief fixture contains local Civilian recipients");
        const auto result=relief.execute({relief_governor,relief.person(relief_governor).decisions+1,action});
        int64_t distributed=0;
        for(uint64_t id=1;id<=uint64_t(relief.count());++id){
            const auto&citizen=relief.person(id);
            if(citizen.family==Family::Civilian&&citizen.region==relief_region)
                distributed+=citizen.cash-cash_before[size_t(id)];
        }
        check(result.success&&distributed==expected&&
              relief.municipal_funds()[size_t(relief_region)]==fund_before-expected&&
              relief.view().provinces[size_t(relief_region)].treasury==treasury_before&&
              relief.view().provinces[size_t(relief_region)].firm==firm_before&&
              relief.view().provinces[size_t(relief_region)].unrest==std::max(0,unrest_before-3)&&
              relief.total_money()==money_before&&relief.invariant(),
              "municipal Aid distributes exactly its stated amount, leaves treasury untouched and preserves total money");
        const auto facts=relief.view(relief_governor).facts;
        check(std::any_of(facts.begin(),facts.end(),[&](const Fact&fact){
                  return fact.id==result.fact&&fact.kind==232&&fact.region==relief_region&&fact.amount==expected;
              }),"municipal Aid records its distinct source, region and amount in a Fact");
        auto oversized=action;oversized.amount=relief.municipal_funds()[size_t(relief_region)]+1;
        const auto unchanged=relief.fingerprint();
        check(!relief.execute({relief_governor,relief.person(relief_governor).decisions+1,oversized}).success&&
              relief.fingerprint()==unchanged,"Aid cannot overspend the municipal account and reject is atomic");
        auto wrong_source=action;wrong_source.good=2;
        check(!relief.execute({relief_governor,relief.person(relief_governor).decisions+1,wrong_source}).success&&
              relief.fingerprint()==unchanged,"Aid rejects an unknown funding source without changing the world");
        auto wrong_region=action;wrong_region.target=relief_region+1;
        check(!relief.execute({relief_governor,relief.person(relief_governor).decisions+1,wrong_region}).success&&
              relief.fingerprint()==unchanged,"local municipal Aid cannot send its account balance to another region");
        const uint64_t other_governor=uint64_t(relief_region+3);
        check(relief.person(other_governor).family==Family::Governance&&
              relief.person(other_governor).region!=relief_region,
              "the reject fixture uses a Governance actor assigned to a different region");
        check(!relief.execute({other_governor,relief.person(other_governor).decisions+1,action}).success&&
              relief.fingerprint()==unchanged,"a different region's Governance cannot spend this municipal account");
        check(!relief.execute({65,relief.person(65).decisions+1,action}).success&&
              relief.fingerprint()==unchanged,"a Civilian cannot copy the Governance-only municipal Aid command");
        check(!relief.execute({relief_governor,relief.person(relief_governor).decisions,action}).success&&
              relief.fingerprint()==unchanged,"a stale municipal Aid sequence is rejected without partial distribution");
        const auto saved=relief.serialize("municipal-relief-roundtrip");
        Realm restored(1,1,128);std::string relief_error;
        check(restored.deserialize(saved,"municipal-relief-roundtrip",relief_error)&&
              restored.fingerprint()==relief.fingerprint()&&restored.invariant(),
              "save/load preserves the municipal balance after a completed aid transfer");
        relief.advance(1);restored.advance(1);
        check(restored.fingerprint()==relief.fingerprint()&&relief.total_money()==money_before&&relief.invariant(),
              "the saved Aid result continues identically through one game day without creating or refunding money");
        auto depleted=relief;
        auto spend_remaining=depleted.candidates(relief_governor)[size_t(realm_municipal_relief_candidate)];
        spend_remaining.amount=depleted.municipal_funds()[size_t(relief_region)];
        check(spend_remaining.amount>0&&
              depleted.execute({relief_governor,depleted.person(relief_governor).decisions+1,spend_remaining}).success&&
              depleted.municipal_funds()[size_t(relief_region)]==0&&
              depleted.mask(relief_governor)[size_t(realm_municipal_relief_candidate)]==0.f&&
              depleted.invariant(),"spending the remaining account balance disables municipal Aid without overdrawing");
    }
    std::cout<<"Establishment transfer checks="<<checks<<"; excluded MSVC debug proxies="<<excluded_debug_proxies<<'\n';return 0;
}catch(const std::exception& error){allocations_until_failure=-1;std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';return 1;}}








