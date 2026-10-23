#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
uint64_t trader_at_port(const Realm& world){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.family==Family::Economic&&person.region==8&&person.alive)return id;
    }
    return 0;
}
std::string with_first_foreign_arrival(std::string save,int arrival_day){
    const std::string marker=" FOREIGN_TRADE_6 ";
    const size_t section=save.find(marker);
    if(section==std::string::npos)throw std::runtime_error("save lacks foreign trade section");
    size_t cursor=section+marker.size();
    auto skip_token=[&](){
        cursor=save.find_first_not_of(" \t\r\n",cursor);
        if(cursor==std::string::npos)throw std::runtime_error("foreign trade section is truncated");
        const size_t end=save.find_first_of(" \t\r\n",cursor);
        if(end==std::string::npos)throw std::runtime_error("foreign trade section token is truncated");
        cursor=end;
    };
    for(size_t i=kForeignTradePartnerCount*(12+kGoodIdCount)+1+6;i>0;--i)skip_token();
    cursor=save.find_first_not_of(" \t\r\n",cursor);
    const size_t end=save.find_first_of(" \t\r\n",cursor);
    save.replace(cursor,end-cursor,std::to_string(arrival_day));
    return save;
}
uint64_t sovereign_in(const Realm& world){
    for(uint64_t id=1;id<=uint64_t(world.count());++id)
        if(world.person(id).alive&&world.person(id).sovereign&&world.person(id).family==Family::Governance)return id;
    return 0;
}
std::string downgrade_trade_history_save_59_to_57(std::string save){
    if(save.rfind("POLYEC_REALM_60 ",0)==0)test_fixtures::downgrade_save_60_to_59(save);
    std::istringstream input(save);std::vector<std::string> words;std::string word;
    while(input>>word)words.push_back(word);
    if(words.empty()||words[0]!="POLYEC_REALM_59")throw std::runtime_error("expected schema59 save");
    const auto section=std::find(words.begin(),words.end(),"FOREIGN_TRADE_6");
    if(section==words.end())throw std::runtime_error("missing foreign trade v6 section");
    const size_t tag=size_t(section-words.begin());
    const size_t partner_stride=3+kGoodIdCount+1+8;
    for(int partner=kForeignTradePartnerCount-1;partner>=0;--partner){
        const size_t first=tag+1+size_t(partner)*partner_stride+3+kGoodIdCount+1;
        if(first+8>words.size())throw std::runtime_error("truncated foreign trade v6 partner");
        words.erase(words.begin()+std::ptrdiff_t(first),words.begin()+std::ptrdiff_t(first+8));
    }
    size_t cursor=tag+1+kForeignTradePartnerCount*(3+kGoodIdCount+1);
    if(cursor>=words.size())throw std::runtime_error("truncated foreign trade order header");
    const size_t order_count=std::stoull(words[cursor++]);
    cursor+=order_count*11;
    if(cursor>=words.size())throw std::runtime_error("truncated foreign trade offer header");
    const size_t offer_count=std::stoull(words[cursor]);
    const size_t offers_end=cursor+1+offer_count*16;
    if(offers_end>words.size())throw std::runtime_error("truncated foreign trade offers");
    words.erase(words.begin()+std::ptrdiff_t(cursor),words.begin()+std::ptrdiff_t(offers_end));
    words[tag]="FOREIGN_TRADE_4";words[0]="POLYEC_REALM_57";
    std::ostringstream output;for(const auto& value:words)output<<value<<' ';return output.str();
}
std::string with_max_partner_a_import_history(std::string save){
    std::istringstream input(save);std::vector<std::string> words;std::string word;
    while(input>>word)words.push_back(word);
    const auto section=std::find(words.begin(),words.end(),"FOREIGN_TRADE_6");
    if(section==words.end())throw std::runtime_error("missing foreign trade v6 section for overflow fixture");
    const size_t first=size_t(section-words.begin())+1+3+kGoodIdCount+1;
    if(first+4>=words.size())throw std::runtime_error("truncated partner history for overflow fixture");
    const auto maximum=std::to_string(std::numeric_limits<int64_t>::max());
    words[first]=maximum;words[first+2]=maximum;words[first+4]=maximum;
    std::ostringstream output;for(const auto& value:words)output<<value<<' ';return output.str();
}
}

int main(){try{
    Realm world(61327,26,256);
    const uint64_t trader=trader_at_port(world);
    const uint64_t governor=10;
    uint64_t sovereign=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id)
        if(world.person(id).alive&&world.person(id).family==Family::Governance&&world.person(id).sovereign){sovereign=id;break;}
    uint64_t worker=0;
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& person=world.person(id);
        if(person.family==Family::Civilian&&person.region==8&&person.alive&&person.age_days>=14*365){worker=id;break;}
    }
    check(trader&&worker&&sovereign&&world.person(governor).taxing_authority,"the 27-region scenario has a merchant, worker, port authority and sovereign");
    check(world.foreign_trade_cash()==kForeignTradeOpeningCash&&
          world.foreign_trade_goods()[size_t(GoodId::DryFood)]==kForeignTradeOpeningDryFood&&
          world.foreign_trade_salt_capacity()==kForeignTradeSaltCapacity&&world.invariant(),
          "the foreign partner begins with the declared finite cash, stock and storage capacity");
    const auto partners=world.foreign_trade_partners();
    check(std::all_of(partners.begin(),partners.end(),[](const ForeignTradePartner& partner){
        return partner.money==kForeignTradePartnerOpeningCash&&partner.escrow==0&&
            partner.goods[size_t(GoodId::DryFood)]==kForeignTradePartnerOpeningDryFood&&
            partner.salt_capacity==kForeignTradePartnerSaltCapacity;
    }),"three distinct partner ledgers begin with separately bounded trial endowments");

    Realm overflowSource(61329,26,128);const uint64_t overflowTrader=trader_at_port(overflowSource);
    const auto maxHistorySave=with_max_partner_a_import_history(overflowSource.serialize("trade-history-overflow-v1"));
    Realm overflowWorld(1,1,128);std::string overflowError;
    check(overflowWorld.deserialize(maxHistorySave,"trade-history-overflow-v1",overflowError)&&overflowWorld.invariant(),
          "the upper boundary for persisted trade history remains a valid non-negative cumulative ledger");
    const auto overflowChoices=overflowWorld.candidates(overflowTrader);const auto overflowMask=overflowWorld.mask(overflowTrader,overflowChoices);
    const auto overflowFingerprint=overflowWorld.fingerprint();
    check(overflowMask[size_t(realm_foreign_trade_candidate_begin)]==0.f&&
          !overflowWorld.execute(ForeignTradeCommand{overflowTrader,overflowWorld.person(overflowTrader).decisions+1,
              ForeignTradeOperation::Import,int(GoodId::DryFood),1}).success&&
          overflowWorld.fingerprint()==overflowFingerprint&&overflowWorld.invariant(),
          "an import that would overflow completed-order totals is masked and rejected without reserving cash or goods");

    Realm partnerWorld(61328,26,256);
    const uint64_t partnerTrader=trader_at_port(partnerWorld);
    const auto partnerActions=partnerWorld.candidates(partnerTrader);
    const auto partnerImport=partnerWorld.execute({partnerTrader,partnerWorld.person(partnerTrader).decisions+1,
        partnerActions[size_t(realm_foreign_trade_partner_candidate_begin)]});
    check(partnerImport.success&&partnerWorld.foreign_trade_orders().size()==1&&
          partnerWorld.foreign_trade_orders()[0].partner_id==1&&
          partnerWorld.foreign_trade_partners()[1].goods[size_t(GoodId::DryFood)]==31&&
          partnerWorld.foreign_trade_partners()[1].escrow==25&&
          partnerWorld.foreign_trade_partners()[0].goods[size_t(GoodId::DryFood)]==32&&
          partnerWorld.foreign_trade_partners()[2].goods[size_t(GoodId::DryFood)]==32,
          "a partner B import records partner identity and reserves only B goods and escrow");
    check(partnerWorld.foreign_trade_partners()[1].completed_import_orders==0&&
          partnerWorld.foreign_trade_partners()[1].imported_units==0,
          "a booked shipment is not reported as completed before its arrival and settlement day");
    const uint64_t partnerSovereign=sovereign_in(partnerWorld);
    check(partnerSovereign!=0,"partner policy fixture identifies its living Sovereign");
    uint64_t nonSovereignGovernor=0;
    for(uint64_t id=1;id<=uint64_t(partnerWorld.count());++id)
        if(partnerWorld.person(id).alive&&partnerWorld.person(id).family==Family::Governance&&
           !partnerWorld.person(id).sovereign){nonSovereignGovernor=id;break;}
    check(nonSovereignGovernor!=0,"policy rejection fixture has a living non-Sovereign official");
    const auto unauthorizedFingerprint=partnerWorld.fingerprint();
    check(!partnerWorld.execute(ForeignTradeSuspensionCommand{nonSovereignGovernor,
          partnerWorld.person(nonSovereignGovernor).decisions+1,1,true}).success&&
          partnerWorld.fingerprint()==unauthorizedFingerprint,
          "a local official cannot change national policy or partially alter the partner ledger");
    const auto stalePolicy=partnerWorld.execute(ForeignTradeSuspensionCommand{partnerSovereign,
        partnerWorld.person(partnerSovereign).decisions+2,1,true});
    check(!stalePolicy.success&&partnerWorld.fingerprint()==unauthorizedFingerprint,
          "a stale Sovereign sequence rejects without changing diplomacy or accounting");
    const auto suspension=partnerWorld.execute(ForeignTradeSuspensionCommand{partnerSovereign,
        partnerWorld.person(partnerSovereign).decisions+1,1,true});
    check(suspension.success&&partnerWorld.foreign_trade_partners()[1].trade_suspended&&
          !partnerWorld.foreign_trade_partners()[0].trade_suspended&&partnerWorld.foreign_trade_orders().size()==1&&
          partnerWorld.invariant(),"Sovereign suspends only partner B while its already accepted import remains booked");
    const auto blockedChoices=partnerWorld.candidates(partnerTrader);
    const auto blockedMask=partnerWorld.mask(partnerTrader,blockedChoices);
    check(blockedMask[size_t(realm_foreign_trade_partner_candidate_begin)]==0.f&&
          blockedMask[size_t(realm_foreign_trade_partner_candidate_begin+3)]==0.f,
          "suspended partner B cannot receive new imports or exports");
    const auto blockedFingerprint=partnerWorld.fingerprint();
    check(!partnerWorld.execute(ForeignTradeCommand{partnerTrader,
          partnerWorld.person(partnerTrader).decisions+1,ForeignTradeOperation::ImportPartnerB,
          int(GoodId::DryFood),1}).success&&partnerWorld.fingerprint()==blockedFingerprint,
          "a typed order is rejected atomically when diplomacy suspends that partner");
    const auto suspendedSave=partnerWorld.serialize("foreign-trade-suspension-v1");
    Realm suspendedResume(1,1,128);
    std::string policyError;
    check(suspendedResume.deserialize(suspendedSave,"foreign-trade-suspension-v1",policyError)&&
          suspendedResume.foreign_trade_partners()[1].trade_suspended&&suspendedResume.invariant(),
          "save/load preserves the selected partner's suspension");
    Realm schema57Resume(1,1,128);
    const auto legacySuspension=downgrade_trade_history_save_59_to_57(suspendedSave);
    const bool migrated57=schema57Resume.deserialize(legacySuspension,"foreign-trade-suspension-v1",policyError);
    if(!migrated57)std::cerr<<"schema57 migration detail: "<<policyError<<'\n';
    check(migrated57&&
          schema57Resume.foreign_trade_partners()[1].trade_suspended&&
          schema57Resume.foreign_trade_partners()[1].completed_import_orders==0&&schema57Resume.invariant(),
          "schema57 migration preserves suspension and defaults unavailable history to zero");
    auto legacySchema56=legacySuspension;test_fixtures::downgrade_save_57_to_56(legacySchema56);
    Realm schema56Resume(1,1,128);
    check(schema56Resume.deserialize(legacySchema56,"foreign-trade-suspension-v1",policyError)&&
          !schema56Resume.foreign_trade_partners()[1].trade_suspended&&
          schema56Resume.foreign_trade_partners()[1].completed_import_orders==0&&schema56Resume.invariant(),
          "schema56 migration defaults unavailable suspension and trade history to original empty/open values");
    partnerWorld.advance(3);
    check(partnerWorld.foreign_trade_partners()[1].money==kForeignTradePartnerOpeningCash+24&&
          partnerWorld.foreign_trade_partners()[1].escrow==0&&
          partnerWorld.foreign_trade_partners()[1].completed_import_orders==1&&
          partnerWorld.foreign_trade_partners()[1].imported_units==1&&
          partnerWorld.foreign_trade_partners()[1].import_gross==24&&
          partnerWorld.foreign_trade_partners()[0].money==kForeignTradePartnerOpeningCash&&
          partnerWorld.foreign_trade_partners()[2].money==kForeignTradePartnerOpeningCash&&
          partnerWorld.invariant(),
          "arrival pays only the named supplier account and clears its settlement escrow");
    const auto partnerHistorySave=partnerWorld.serialize("foreign-trade-history-v1");
    Realm partnerHistoryResume(1,1,128);
    check(partnerHistoryResume.deserialize(partnerHistorySave,"foreign-trade-history-v1",policyError)&&
          partnerHistoryResume.foreign_trade_partners()[1]==partnerWorld.foreign_trade_partners()[1]&&
          partnerHistoryResume.invariant(),"save/load preserves completed trade counts, units and gross values");
    check(partnerWorld.foreign_trade_partners()[1].trade_suspended&&partnerWorld.foreign_trade_orders().empty(),
          "an accepted shipment settles on schedule without reopening future trade");

    const auto duplicatePolicy=partnerWorld.execute(ForeignTradeSuspensionCommand{partnerSovereign,
        partnerWorld.person(partnerSovereign).decisions+1,1,true});
    check(!duplicatePolicy.success,"repeating the existing suspension is rejected");
    const auto reopened=partnerWorld.execute(ForeignTradeSuspensionCommand{partnerSovereign,
        partnerWorld.person(partnerSovereign).decisions+1,1,false});
    check(reopened.success&&!partnerWorld.foreign_trade_partners()[1].trade_suspended&&
          partnerWorld.invariant(),"the Sovereign can reopen new trade without changing settled account balances");

    Realm importOfferWorld(61331,26,256);
    const uint64_t importOfferTrader=trader_at_port(importOfferWorld);
    const int64_t importOfferCashBefore=importOfferWorld.person(importOfferTrader).cash;
    const int64_t importPartnerFoodBefore=importOfferWorld.foreign_trade_partners()[0].goods[size_t(GoodId::DryFood)];
    const auto importOfferResult=importOfferWorld.execute(ForeignTradeOfferCommand{importOfferTrader,
        importOfferWorld.person(importOfferTrader).decisions+1,0,ForeignTradeOperation::Import,int(GoodId::DryFood),1});
    const auto importOffers=importOfferWorld.foreign_trade_offers();
    check(importOfferResult.success&&importOffers.size()==1&&importOffers[0].status==ForeignTradeOfferStatus::Pending&&
          importOffers[0].held_cash>24&&importOffers[0].held_good==1&&
          importOfferWorld.person(importOfferTrader).cash==importOfferCashBefore-importOffers[0].held_cash&&
          importOfferWorld.foreign_trade_partners()[0].goods[size_t(GoodId::DryFood)]==importPartnerFoodBefore-1&&
          importOfferWorld.foreign_trade_orders().empty()&&importOfferWorld.invariant(),
          "an import offer reserves the trader's full cost and partner food without booking delivery");
    const std::string pendingOfferSave=importOfferWorld.serialize("foreign-trade-offer-pending-v1");
    Realm pendingOfferResume(1,1,128);std::string offerError;
    check(pendingOfferResume.deserialize(pendingOfferSave,"foreign-trade-offer-pending-v1",offerError)&&
          pendingOfferResume.foreign_trade_offers()==importOffers&&
          pendingOfferResume.foreign_trade_partners()==importOfferWorld.foreign_trade_partners()&&pendingOfferResume.invariant(),
          "save/load preserves the pending offer, due date, escrow and partner response cursor");
    const auto legacyFixtureResponse=importOfferWorld.execute(ForeignTradeResponseCommand{0,1,importOffers[0].id,false});
    check(legacyFixtureResponse.success&&importOfferWorld.invariant(),
          "the legacy migration fixture resolves its pending hold before converting to a schema without offers");
    auto schema58OfferSave=importOfferWorld.serialize("foreign-trade-offer-pending-v1");
    test_fixtures::downgrade_save_59_to_58(schema58OfferSave);
    Realm schema58OfferResume(1,1,128);
    check(schema58OfferResume.deserialize(schema58OfferSave,"foreign-trade-offer-pending-v1",offerError)&&
          schema58OfferResume.foreign_trade_offers().empty()&&
          schema58OfferResume.foreign_trade_partners()[0].decision_sequence==0&&schema58OfferResume.invariant(),
          "schema58 migration carries no resolved-offer records and defaults the newer partner response sequence to zero");
    const auto staleOfferResponse=pendingOfferResume.execute(ForeignTradeResponseCommand{0,2,importOffers[0].id,false});
    check(!staleOfferResponse.success&&pendingOfferResume.foreign_trade_offers()==importOffers&&pendingOfferResume.invariant(),
          "a skipped partner response sequence cannot resolve an offer or release held resources");
    const auto wrongPartnerResponse=pendingOfferResume.execute(ForeignTradeResponseCommand{1,1,importOffers[0].id,false});
    check(!wrongPartnerResponse.success&&pendingOfferResume.foreign_trade_offers()==importOffers&&
          pendingOfferResume.foreign_trade_partners()[1].decision_sequence==0&&pendingOfferResume.invariant(),
          "a different partner cannot answer an offer addressed to partner A");
    const auto pendingImportRoute=std::find_if(pendingOfferResume.national_transport_routes().begin(),
        pendingOfferResume.national_transport_routes().end(),[](const NationalTransportRoute& route){
            return route.from==9&&route.to==8;
        });
    check(pendingImportRoute!=pendingOfferResume.national_transport_routes().end(),
          "pending offer fixture resolves the real inbound coastal route");
    const uint8_t pendingImportRouteIndex=uint8_t(pendingImportRoute-pendingOfferResume.national_transport_routes().begin());
    const uint64_t pendingOfferSovereign=sovereign_in(pendingOfferResume);
    const auto closedPendingRoute=pendingOfferResume.execute(TransportRouteCommand{pendingOfferSovereign,
        pendingOfferResume.person(pendingOfferSovereign).decisions+1,pendingImportRouteIndex,false});
    const auto routeBlockedFingerprint=pendingOfferResume.fingerprint();
    check(closedPendingRoute.success&&
          !pendingOfferResume.execute(ForeignTradeResponseCommand{0,1,importOffers[0].id,true}).success&&
          pendingOfferResume.fingerprint()==routeBlockedFingerprint&&
          pendingOfferResume.foreign_trade_offers()[0].status==ForeignTradeOfferStatus::Pending&&pendingOfferResume.invariant(),
          "acceptance waits while the coastal route is closed and leaves the pending hold untouched");
    check(pendingOfferResume.execute(TransportRouteCommand{pendingOfferSovereign,
          pendingOfferResume.person(pendingOfferSovereign).decisions+1,pendingImportRouteIndex,true}).success&&
          pendingOfferResume.invariant(),
          "the Sovereign can reopen the same route before the offer deadline");
    const auto acceptOffer=pendingOfferResume.execute(ForeignTradeResponseCommand{0,1,importOffers[0].id,true});
    const auto acceptedOffers=pendingOfferResume.foreign_trade_offers();
    check(acceptOffer.success&&acceptedOffers[0].status==ForeignTradeOfferStatus::Accepted&&
          acceptedOffers[0].held_cash==0&&acceptedOffers[0].held_good==0&&
          acceptedOffers[0].order_id==acceptOffer.session&&pendingOfferResume.foreign_trade_orders().size()==1&&
          pendingOfferResume.foreign_trade_partners()[0].decision_sequence==1&&pendingOfferResume.invariant(),
          "only the named partner's explicit response books the accepted import and consumes its response sequence");
    const auto duplicateResponseFingerprint=pendingOfferResume.fingerprint();
    check(!pendingOfferResume.execute(ForeignTradeResponseCommand{0,2,importOffers[0].id,false}).success&&
          pendingOfferResume.fingerprint()==duplicateResponseFingerprint&&pendingOfferResume.invariant(),
          "a resolved offer cannot be answered a second time or refund its resources twice");
    pendingOfferResume.advance(3);
    check(pendingOfferResume.foreign_trade_orders().empty()&&
          pendingOfferResume.foreign_trade_partners()[0].completed_import_orders==1&&pendingOfferResume.invariant(),
          "an accepted import follows the existing three-day route and records history only after settlement");

    Realm rejectOfferWorld(61332,26,256);const uint64_t rejectOfferTrader=trader_at_port(rejectOfferWorld);
    const int64_t rejectCashBefore=rejectOfferWorld.person(rejectOfferTrader).cash;
    const int64_t rejectFoodBefore=rejectOfferWorld.foreign_trade_partners()[1].goods[size_t(GoodId::DryFood)];
    const auto rejectOffer=rejectOfferWorld.execute(ForeignTradeOfferCommand{rejectOfferTrader,
        rejectOfferWorld.person(rejectOfferTrader).decisions+1,1,ForeignTradeOperation::ImportPartnerB,int(GoodId::DryFood),1});
    const auto rejectId=rejectOffer.session;
    const auto rejectResponse=rejectOfferWorld.execute(ForeignTradeResponseCommand{1,1,rejectId,false});
    check(rejectOffer.success&&rejectResponse.success&&rejectOfferWorld.foreign_trade_orders().empty()&&
          rejectOfferWorld.foreign_trade_offers()[0].status==ForeignTradeOfferStatus::Rejected&&
          rejectOfferWorld.person(rejectOfferTrader).cash==rejectCashBefore&&
          rejectOfferWorld.foreign_trade_partners()[1].goods[size_t(GoodId::DryFood)]==rejectFoodBefore&&
          rejectOfferWorld.foreign_trade_partners()[1].decision_sequence==1&&rejectOfferWorld.invariant(),
          "partner B can explicitly reject, returning the buyer's cash and its own DryFood exactly once");

    Realm expiryWorld(61334,26,256);const uint64_t expiryTrader=trader_at_port(expiryWorld);
    const int64_t expiryPartnerFoodBefore=expiryWorld.foreign_trade_partners()[0].goods[size_t(GoodId::DryFood)];
    const auto expiringOffer=expiryWorld.execute(ForeignTradeOfferCommand{expiryTrader,
        expiryWorld.person(expiryTrader).decisions+1,0,ForeignTradeOperation::Import,int(GoodId::DryFood),1});
    expiryWorld.advance(8);
    check(expiringOffer.success&&expiryWorld.foreign_trade_offers()[0].status==ForeignTradeOfferStatus::Expired&&
          expiryWorld.foreign_trade_offers()[0].held_cash==0&&expiryWorld.foreign_trade_offers()[0].held_good==0&&
          expiryWorld.foreign_trade_partners()[0].goods[size_t(GoodId::DryFood)]==expiryPartnerFoodBefore&&
          expiryWorld.foreign_trade_orders().empty()&&
          expiryWorld.foreign_trade_partners()[0].decision_sequence==0&&expiryWorld.invariant(),
          "an unanswered offer expires after seven full game days and returns reserved money and goods automatically");

    const auto choices=world.candidates(trader);const auto mask=world.mask(trader,choices);
    const size_t importOne=size_t(realm_foreign_trade_candidate_begin);
    check(choices[importOne].kind==Intent::ForeignTrade&&choices[importOne].good==int(GoodId::DryFood)&&
          choices[importOne].amount==1&&choices[importOne].term==int(ForeignTradeOperation::Import)&&mask[importOne]==1.f,
          "a merchant at Nam Cang can choose the one-unit DryFood import");
    check(world.observation(trader)[size_t(realm_foreign_trade_stock_observation_index)]==1.f&&
          world.observation(trader)[size_t(realm_foreign_trade_capacity_observation_index)]==1.f&&
          world.observation(trader)[size_t(realm_foreign_trade_cash_observation_index)]==1.f&&
          world.observation(governor)[size_t(realm_foreign_trade_stock_observation_index)]==0.f,
          "only a port merchant observes the bounded foreign account");
    const auto partnerChoices=world.candidates(trader);const auto partnerMask=world.mask(trader,partnerChoices);
    for(int partner=1;partner<kForeignTradePartnerCount;++partner){
        const size_t slot=size_t(realm_foreign_trade_partner_candidate_begin+(partner-1)*6);
        check(partnerChoices[slot].kind==Intent::ForeignTrade&&
              partnerChoices[slot].term==int(foreign_trade_operation(partner,true))&&partnerMask[slot]==1.f,
              "each additional partner has its own legal import candidate");
        const size_t state=size_t(realm_foreign_trade_partner_observation_begin+(partner-1)*3);
        check(world.observation(trader)[state]==1.f&&world.observation(trader)[state+1]==1.f&&
              world.observation(trader)[state+2]==1.f,
              "the merchant observes the independent stock, storage and cash of each additional partner");
    }

    size_t importRoute=world.national_transport_routes().size(),exportRoute=world.national_transport_routes().size();
    for(size_t i=0;i<world.national_transport_routes().size();++i){
        const auto& route=world.national_transport_routes()[i];
        const auto& link=kWorldAtlasLinks[route.atlas_link];
        if(link.from!=8||link.to!=9||link.mode!=AtlasRouteMode::CoastalShipping)continue;
        if(route.from==9&&route.to==8)importRoute=i;
        if(route.from==8&&route.to==9)exportRoute=i;
    }
    check(importRoute<world.national_transport_routes().size()&&exportRoute<world.national_transport_routes().size(),
          "Atlas supplies both directions of the Nam Cang to Hai Chau coastal route");
    check(world.national_transport_routes()[importRoute].travel_days==2&&
          world.national_transport_routes()[importRoute].capacity_per_day==64&&
          world.national_transport_routes()[importRoute].fee_per_unit==1,
          "foreign shipment timing and route charge come from the current transport service record");
    const auto closedImport=world.execute(TransportRouteCommand{sovereign,world.person(sovereign).decisions+1,uint8_t(importRoute),false});
    check(closedImport.success,"sovereign can close the coastal route used by incoming imports");
    check(world.mask(trader,choices)[importOne]==0.f,"closed incoming coastal route turns off import action mask");
    const auto closedImportFingerprint=world.fingerprint();
    check(!world.execute(ForeignTradeCommand{trader,world.person(trader).decisions+1,ForeignTradeOperation::Import,
              int(GoodId::DryFood),1}).success&&world.fingerprint()==closedImportFingerprint,
          "closed incoming route rejects the typed import without changing money or stock");
    check(world.execute(TransportRouteCommand{sovereign,world.person(sovereign).decisions+1,uint8_t(importRoute),true}).success,
          "reopening the incoming route restores service");

    const int64_t moneyBefore=world.total_money();
    const auto withoutOrderSave=world.serialize("foreign-trade-v1");
    const int64_t treasuryBefore=world.view().provinces[8].treasury;
    const int64_t destinationRouteTreasuryBefore=world.view().provinces[9].treasury;
    const int64_t taxReceiptsBefore=world.view().provinces[8].tax_receipts;
    const auto imported=world.execute({trader,world.person(trader).decisions+1,choices[importOne]});
    check(imported.success&&world.foreign_trade_goods()[size_t(GoodId::DryFood)]==31&&
          world.foreign_trade_orders().size()==1&&world.foreign_trade_orders()[0].arrival_day==3&&
          world.foreign_trade_escrow()==25&&world.view().provinces[8].treasury==treasuryBefore+2&&
          world.view().provinces[9].treasury==destinationRouteTreasuryBefore+1&&
          world.total_money()==moneyBefore&&world.invariant(),
          "import reserves DryFood, escrows price and duty, pays port and coastal-route fees, and keeps total money");
    check(world.execute(TransportRouteCommand{sovereign,world.person(sovereign).decisions+1,uint8_t(importRoute),false}).success&&
          world.foreign_trade_orders().size()==1,
          "closing the route after accepting an import does not cancel or refund its booked voyage");
    const auto importFingerprint=world.fingerprint();
    const auto duplicate=world.execute({trader,world.person(trader).decisions+1,choices[importOne]});
    check(!duplicate.success&&world.fingerprint()==importFingerprint,"the same port lane rejects a second order on the same day atomically");
    std::string error;
    const auto inTransit=world.serialize("foreign-trade-v1");
    Realm resumed(1,1,128);
    const bool inTransitLoaded=resumed.deserialize(inTransit,"foreign-trade-v1",error);
    if(inTransitLoaded&&resumed.fingerprint()!=world.fingerprint()){
        const auto roundtrip=resumed.serialize("foreign-trade-v1");
        size_t diff=0;while(diff<inTransit.size()&&diff<roundtrip.size()&&inTransit[diff]==roundtrip[diff])++diff;
        std::cerr<<"foreign save roundtrip mismatch at byte "<<diff<<" source="<<inTransit.substr(diff,80)
                 <<" restored="<<roundtrip.substr(diff,80)<<'\n';
    }
    check(inTransitLoaded&&resumed.fingerprint()==world.fingerprint(),
          error.empty()?"save/load preserves a foreign shipment in transit":error.c_str());
    Realm legacyPending(1,1,128);
    const auto oldArrivalSave=with_first_foreign_arrival(inTransit,4);
    check(legacyPending.deserialize(oldArrivalSave,"foreign-trade-v1",error)&&
          legacyPending.foreign_trade_orders().size()==1&&legacyPending.foreign_trade_orders()[0].arrival_day==4&&
          legacyPending.invariant(),
          error.empty()?"save loading preserves an already-issued D+4 foreign order from the earlier route version":error.c_str());
    Realm control(1,1,128);
    check(control.deserialize(withoutOrderSave,"foreign-trade-v1",error),
          error.empty()?"control world loads before the import order":"control save failed to load");
    resumed.advance(2);
    control.advance(2);
    check(resumed.day()==2&&resumed.foreign_trade_orders().size()==1&&
          resumed.view().provinces[8].stock[size_t(GoodId::DryFood)]==control.view().provinces[8].stock[size_t(GoodId::DryFood)],
          "cargo does not enter the local market before its three-day crossing is complete");
    resumed.advance(1);
    control.advance(1);
    check(resumed.day()==3&&resumed.foreign_trade_orders().empty(),
          "the import settles after one scheduled departure day and the route's two travel days");
    legacyPending.advance(3);
    check(legacyPending.day()==3&&legacyPending.foreign_trade_orders().size()==1,
          "a saved earlier-version order is not pulled forward to the new D+3 schedule");
    legacyPending.advance(1);
    check(legacyPending.day()==4&&legacyPending.foreign_trade_orders().empty()&&legacyPending.invariant(),
          "the earlier-version order settles on its originally saved D+4 arrival day");
    check(resumed.view().provinces[8].stock[size_t(GoodId::DryFood)]==
          control.view().provinces[8].stock[size_t(GoodId::DryFood)]+1,
          "the imported DryFood reaches the local market after the last day's consumption is complete");
    check(resumed.foreign_trade_cash()==kForeignTradeOpeningCash+24&&resumed.foreign_trade_escrow()==0,
          "arrival pays the foreign supplier and clears the import escrow");
    check(resumed.foreign_trade_partners()[0].completed_import_orders==1&&
          resumed.foreign_trade_partners()[0].imported_units==1&&
          resumed.foreign_trade_partners()[0].import_gross==24&&
          resumed.foreign_trade_partners()[0].completed_export_orders==0,
          "only the completed one-unit import is added to its partner-specific trade history");
    check(resumed.view().provinces[8].tax_receipts==control.view().provinces[8].tax_receipts+1&&
          resumed.view().provinces[8].tax_receipts>=taxReceiptsBefore+1,
          "the one-unit import records exactly one unit of customs duty");
    check(resumed.invariant(),"import settlement preserves Realm money, goods and record invariants");

    const auto saltSlot=size_t(realm_establishment_candidate_begin+9);
    const auto governorChoices=resumed.candidates(governor);
    const auto created=resumed.execute({governor,resumed.person(governor).decisions+1,governorChoices[saltSlot]});
    check(created.success,"the local authority opens a Saltworks using the existing public-facility command");
    const uint64_t site=created.session;
    check(resumed.execute(EstablishmentTransferCommand{governor,resumed.person(governor).decisions+1,site,
              EstablishmentTransferOperation::DepositGoods,int(GoodId::Fuel),1}).success,
          "the Saltworks receives one Fuel from the real regional stock");
    check(resumed.execute(WorkAuthorizationCommand{governor,resumed.person(governor).decisions+1,site,worker,
              WorkAuthorizationOperation::Set,resumed.game_tick(),resumed.game_tick()+32}).success,
          "the authority authorizes an existing local worker");
    check(resumed.execute(ScheduleCommand{worker,resumed.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
              0,resumed.game_tick(),resumed.game_tick()+32,0,1000,ActivityKind::Work,8,site}).success,
          "the worker submits a real Work schedule for the Saltworks");
    resumed.advance_ticks(64);
    check(resumed.establishments()[0].stock[size_t(GoodId::Salt)]==1,"one completed Saltworks batch creates one Salt");
    check(resumed.execute(EstablishmentTransferCommand{governor,resumed.person(governor).decisions+1,site,
              EstablishmentTransferOperation::WithdrawGoods,int(GoodId::Salt),1}).success,
          "the owner moves the produced Salt into the regional market");
    check(resumed.execute({trader,resumed.person(trader).decisions+1,
              {Intent::Buy,8,int(GoodId::Salt),1}}).success&&
          resumed.person(trader).inventory[size_t(GoodId::Salt)]==1,
          "the merchant obtains Salt through the existing local market");

    const int64_t exportSaltBefore=resumed.person(trader).inventory[size_t(GoodId::Salt)];
    const int64_t exportPartnerMoneyBefore=resumed.foreign_trade_partners()[2].money;
    const int64_t exportCapacityBefore=resumed.foreign_trade_partners()[2].salt_capacity;
    const auto exportOffer=resumed.execute(ForeignTradeOfferCommand{trader,
        resumed.person(trader).decisions+1,2,ForeignTradeOperation::ExportPartnerC,int(GoodId::Salt),1});
    const auto exportOfferRecords=resumed.foreign_trade_offers();
    const auto pendingExportSave=resumed.serialize("foreign-trade-export-offer-v1");
    Realm pendingExportResume(1,1,128);
    check(pendingExportResume.deserialize(pendingExportSave,"foreign-trade-export-offer-v1",error)&&
          pendingExportResume.foreign_trade_offers()==exportOfferRecords&&
          pendingExportResume.foreign_trade_partners()[2].offer_cash_escrow==28&&pendingExportResume.invariant(),
          "save/load preserves an export offer's held Salt, partner cash and reserved receiving capacity");
    const auto exportRejected=resumed.execute(ForeignTradeResponseCommand{2,1,exportOffer.session,false});
    check(exportOffer.success&&exportRejected.success&&exportOfferRecords.back().held_good==1&&
          resumed.person(trader).inventory[size_t(GoodId::Salt)]==exportSaltBefore&&
          resumed.foreign_trade_partners()[2].money==exportPartnerMoneyBefore&&
          resumed.foreign_trade_partners()[2].salt_capacity==exportCapacityBefore&&resumed.invariant(),
          "a rejected Salt export restores the merchant's Salt and the partner's reserved cash and storage capacity");

    const auto exportChoices=resumed.candidates(trader);
    const size_t exportOne=size_t(realm_foreign_trade_candidate_begin+3);
    check(exportChoices[exportOne].kind==Intent::ForeignTrade&&exportChoices[exportOne].good==int(GoodId::Salt)&&
          exportChoices[exportOne].term==int(ForeignTradeOperation::Export)&&resumed.mask(trader,exportChoices)[exportOne]==1.f,
          "a merchant holding Salt can choose the one-unit export");
    const auto closedExport=resumed.execute(TransportRouteCommand{sovereign,resumed.person(sovereign).decisions+1,uint8_t(exportRoute),false});
    check(closedExport.success&&resumed.mask(trader,exportChoices)[exportOne]==0.f,
          "closing the outward coastal route disables the export action");
    const auto closedExportFingerprint=resumed.fingerprint();
    check(!resumed.execute(ForeignTradeCommand{trader,resumed.person(trader).decisions+1,ForeignTradeOperation::Export,
              int(GoodId::Salt),1}).success&&resumed.fingerprint()==closedExportFingerprint,
          "closed outward route rejects a typed export without reserving Salt or partner money");
    check(resumed.execute(TransportRouteCommand{sovereign,resumed.person(sovereign).decisions+1,uint8_t(exportRoute),true}).success&&
          resumed.mask(trader,exportChoices)[exportOne]==1.f,
          "reopening the outward route restores export eligibility");
    const int64_t traderCashBeforeExport=resumed.person(trader).cash;
    const int64_t foreignCashBeforeExport=resumed.foreign_trade_cash();
    const int64_t exportTreasuryBefore=resumed.view().provinces[8].treasury;
    const int64_t exportRouteTreasuryBefore=resumed.view().provinces[9].treasury;
    const auto withoutExportSave=resumed.serialize("foreign-trade-v1");
    const auto exported=resumed.execute({trader,resumed.person(trader).decisions+1,exportChoices[exportOne]});
    check(exported.success&&resumed.person(trader).inventory[size_t(GoodId::Salt)]==0&&
          resumed.foreign_trade_salt_capacity()==31&&resumed.foreign_trade_cash()==foreignCashBeforeExport-28&&
          resumed.person(trader).cash==traderCashBeforeExport-3&&resumed.total_money()==moneyBefore&&resumed.invariant()&&
          resumed.view().provinces[8].treasury==exportTreasuryBefore+3&&
          resumed.view().provinces[9].treasury==exportRouteTreasuryBefore,
          "export reserves Salt and partner payment, paying the port and outward route fees to their configured accounts");
    const auto exportPending=resumed.serialize("foreign-trade-v1");
    Realm exportRestored(1,1,128);
    check(exportRestored.deserialize(exportPending,"foreign-trade-v1",error)&&
          exportRestored.fingerprint()==resumed.fingerprint(),"save/load keeps the outgoing shipment and reserved partner capacity");
    Realm exportControl(1,1,128);
    check(exportControl.deserialize(withoutExportSave,"foreign-trade-v1",error),
          error.empty()?"control world loads before the export order":"export control save failed to load");
    exportRestored.advance(4);
    exportControl.advance(4);
    check(exportRestored.foreign_trade_goods()[size_t(GoodId::Salt)]==1&&
          exportRestored.foreign_trade_salt_capacity()==31,
          "export arrival credits the partner with Salt and uses its finite storage capacity");
    check(exportRestored.person(trader).cash==exportControl.person(trader).cash+24&&
          exportRestored.foreign_trade_escrow()==0,
          "export arrival adds net proceeds after the one-unit duty and both already-paid route charges");
    check(exportRestored.foreign_trade_partners()[0].completed_export_orders==1&&
          exportRestored.foreign_trade_partners()[0].exported_units==1&&
          exportRestored.foreign_trade_partners()[0].export_gross==28&&
          exportRestored.foreign_trade_partners()[0].completed_import_orders==1,
          "the completed Salt export updates only partner A's export order count, units and pre-fee gross value");
    check(exportRestored.view().provinces[8].tax_receipts==exportControl.view().provinces[8].tax_receipts+1&&
          exportRestored.view().provinces[8].treasury>=exportTreasuryBefore+1,
          "export settlement adds one customs duty to the port's tax record and treasury");
    check(exportRestored.invariant(),"export settlement preserves Realm money, goods and record invariants");

    Realm legacySource(61328,26,128);
    std::string schema49=legacySource.serialize("foreign-trade-v1");
    test_fixtures::downgrade_save_50_to_49(schema49);
    Realm migrated(1,1,128);
    check(migrated.deserialize(schema49,"foreign-trade-v1",error)&&
          migrated.foreign_trade_cash()==kForeignTradeOpeningCash&&
          migrated.foreign_trade_goods()[size_t(GoodId::DryFood)]==kForeignTradeOpeningDryFood&&
          migrated.foreign_trade_salt_capacity()==kForeignTradeSaltCapacity&&migrated.invariant(),
          "schema49 migration creates the documented one-time external endowment and records it in conservation totals");

    std::cout<<"Foreign trade checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"Foreign trade failure after "<<checks<<": "<<error.what()<<'\n';return 1;}}
