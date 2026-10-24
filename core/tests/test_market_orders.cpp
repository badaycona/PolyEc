#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace polyec;

namespace polyec {
struct RealmObservationTestAccess {
    static void clear_stock(Realm& world,int region,int good){
        auto& province=world.provinces_[size_t(region)];
        const int64_t amount=province.stock[size_t(good)];
        if(amount>std::numeric_limits<int64_t>::max()-world.consumed_[size_t(good)])
            throw std::overflow_error("market test stock fixture overflow");
        world.consumed_[size_t(good)]+=amount;
        province.stock[size_t(good)]=0;
    }
    static void cancel_orders_for_death(Realm& world,uint64_t owner){world.cancel_market_orders(owner);}
    static void settle_market_day(Realm& world){world.settle_market_day();}
};
}

int main(){
    try{
        int checks=0;
        auto check=[&](bool condition,const char* message){++checks;if(!condition)throw std::runtime_error(message);};
        Realm world(901,1,128);
        const int good=1;
        uint64_t buyer=0;
        for(uint64_t id=1;id<=uint64_t(world.count());++id){
            const auto& candidate=world.person(id);
            if(candidate.alive&&candidate.age_days>=6*365&&id%7!=1&&
               world.household_authority(world.household_of(id))==id&&candidate.cash>0&&
               world.gameplay_config().market_price_floor()<=world.view().provinces[size_t(candidate.region)].price[size_t(good)]&&
               world.available_funds(id)>=world.view().provinces[size_t(candidate.region)].price[size_t(good)]){
                buyer=id;break;
            }
        }
        check(buyer!=0,"fixture has a living eligible buyer with enough funds");
        const int region=world.person(buyer).region;
        const int64_t price=world.view().provinces[size_t(region)].price[size_t(good)];
        const int64_t initialHouseholdDeposit=world.person(buyer).cash;
        const auto deposited=world.execute(HouseholdFundCommand{buyer,world.person(buyer).decisions+1,
            world.household_of(buyer),HouseholdFundOperation::Deposit,initialHouseholdDeposit});
        check(deposited.success&&world.person(buyer).cash==0&&
              world.household_fund(world.household_of(buyer))==initialHouseholdDeposit,
              "fixture moves the buyer's money into the household account through its real command");
        RealmObservationTestAccess::clear_stock(world,region,good);
        check(world.invariant(),"zero-stock market fixture preserves the goods ledger");
        const int64_t moneyBefore=world.total_money();
        const int64_t cashBefore=world.person(buyer).cash;
        const int64_t householdBefore=world.household_fund(world.household_of(buyer));
        const int64_t maximumUnitPrice=initialHouseholdDeposit;
        const auto placed=world.execute(MarketOrderCommand{buyer,world.person(buyer).decisions+1,0,
            MarketOrderOperation::Place,region,good,1,maximumUnitPrice});
        check(placed.success&&placed.session!=0,"funded purchase order is accepted with a stable order ID");
        check(world.market_escrow()==initialHouseholdDeposit&&world.market_orders().size()==1&&world.invariant(),
              "accepted order reserves its maximum price in the market account");
        check(world.market_orders().front().personal_escrow==0&&world.market_orders().front().household_escrow==initialHouseholdDeposit,
              "order records household money as its exact funding source");
        RealmObservationTestAccess::settle_market_day(world);
        check(world.market_orders().size()==1&&world.market_escrow()==initialHouseholdDeposit,
              "same-day settlement check leaves a next-day order and its escrow unchanged");

        const std::string saved=world.serialize("market-order-test");
        Realm restored(1,1,128);std::string error;
        const bool loaded=restored.deserialize(saved,"market-order-test",error);
        check(loaded&&restored.fingerprint()==world.fingerprint(),loaded?"pending order round-trips exactly":error.c_str());
        std::string corruptEscrow=saved;
        const auto marketHeader=corruptEscrow.find(" MARKET_2 ");
        check(marketHeader!=std::string::npos,"save includes the versioned market ledger");
        const auto escrowBegin=marketHeader+std::string(" MARKET_2 ").size();
        const auto escrowEnd=corruptEscrow.find(' ',escrowBegin);
        corruptEscrow.replace(escrowBegin,escrowEnd-escrowBegin,std::to_string(initialHouseholdDeposit+1));
        const auto destinationBefore=restored.fingerprint();
        const bool badEscrowLoaded=restored.deserialize(corruptEscrow,"market-order-test",error);
        check(!badEscrowLoaded&&restored.fingerprint()==destinationBefore,
              "save loader rejects an escrow amount that disagrees with open-order liabilities without replacing the current world");

        Realm historicalSource(904,1,128);
        const auto oldSave=test_fixtures::save_as_v25(historicalSource.serialize("market-order-legacy"));
        Realm historical(1,1,128);
        const bool historicalLoaded=historical.deserialize(oldSave,"market-order-legacy",error);
        check(historicalLoaded&&!historical.gameplay_config().market_enabled()&&historical.market_orders().empty()&&
              historical.market_escrow()==0&&historical.invariant(),
              historicalLoaded?"schema25 migrates without enabling the new market rules":error.c_str());
        restored.advance(1);
        check(restored.market_orders().empty()&&restored.market_escrow()==0,
              "an order with no available goods closes after its due day");
        const auto settledFingerprint=restored.fingerprint();
        RealmObservationTestAccess::settle_market_day(restored);
        check(restored.fingerprint()==settledFingerprint,
              "repeating settlement on the same completed day does not change the world twice");
        check(restored.total_money()==moneyBefore,"an unfilled order conserves total money");
        check(restored.person(buyer).cash==cashBefore,"an unfilled order restores the buyer's personal cash");
        check(restored.household_fund(restored.household_of(buyer))==householdBefore,
              "an unfilled order restores the household's original fund");
        const auto totals=restored.market_monthly_totals(region,good);
        check(totals.demand==1&&totals.unmet==1,
              "unfilled funded demand is recorded once for monthly pricing");
        check(restored.invariant(),"unfilled settlement leaves the Realm accounting invariant valid");

        const int64_t oldPrice=restored.view().provinces[size_t(region)].price[size_t(good)];
        restored.advance(29);
        const auto endingStock=restored.view().provinces[size_t(region)].stock[size_t(good)];
        const int64_t pressure=std::clamp<int64_t>(1+(1-endingStock),-1,1);
        const int64_t basisPoints=10000+1000*pressure;
        const int64_t expectedPrice=(oldPrice*basisPoints+5000)/10000;
        check(restored.view().provinces[size_t(region)].price[size_t(good)]==expectedPrice,
              "month-end price follows the hand-calculated shortage and stock rule");
        check(restored.market_monthly_totals(region,good)==MarketMonthlyTotals{},
              "month-end price update clears the completed month's demand totals");
        const auto monthSave=restored.serialize("market-order-test");
        Realm monthReloaded(1,1,128);
        const bool monthLoaded=monthReloaded.deserialize(monthSave,"market-order-test",error);
        check(monthLoaded&&monthReloaded.fingerprint()==restored.fingerprint(),
              monthLoaded?"month-end price and cleared totals round-trip":error.c_str());
        monthReloaded.advance(1);
        check(monthReloaded.view().provinces[size_t(region)].price[size_t(good)]==expectedPrice,
              "loading after month end does not repeat the same price update");

        Realm filled(902,1,128);
        uint64_t filledBuyer=0;
        int filledRegion=0,filledGood=-1;
        int64_t listed=0;
        for(uint64_t id=1;id<=uint64_t(filled.count());++id){
            const auto& candidate=filled.person(id);
            const int candidateRegion=candidate.region;
            const auto& province=filled.view().provinces[size_t(candidateRegion)];
            if(!candidate.alive||candidate.age_days<6*365||id%7==1)continue;
            for(int candidateGood=1;candidateGood<kGoodIdCount;++candidateGood)
                if(province.stock[size_t(candidateGood)]>0&&filled.available_funds(id)>=province.price[size_t(candidateGood)]){
                    filledBuyer=id;filledRegion=candidateRegion;filledGood=candidateGood;listed=province.price[size_t(candidateGood)];break;
                }
            if(filledBuyer)break;
        }
        check(filledBuyer!=0,"fill fixture has a funded buyer and real local stock");
        const auto filledBefore=filled.person(filledBuyer).inventory[size_t(filledGood)];
        const auto filledMoney=filled.total_money();
        const auto filledOrder=filled.execute(MarketOrderCommand{filledBuyer,filled.person(filledBuyer).decisions+1,0,
            MarketOrderOperation::Place,filledRegion,filledGood,1,listed});
        check(filledOrder.success,"funded order for an available item is accepted");
        filled.advance(1);
        check(filled.person(filledBuyer).inventory[size_t(filledGood)]==filledBefore+1&&
              filled.market_monthly_totals(filledRegion,filledGood)==MarketMonthlyTotals{1,0},
              "settlement transfers one real item and records fulfilled demand");
        check(filled.market_orders().empty()&&filled.market_escrow()==0&&filled.total_money()==filledMoney&&filled.invariant(),
              "filled order closes, releases escrow, and conserves money");

        Realm reservedCapacity(906,1,128);
        uint64_t reservedBuyer=0;int reservedRegion=0,reservedGood=-1;int64_t reservedPrice=0;
        for(uint64_t id=1;id<=uint64_t(reservedCapacity.count())&&!reservedBuyer;++id){
            const auto&candidate=reservedCapacity.person(id);
            if(!candidate.alive||candidate.age_days<6*365)continue;
            int64_t inventoryLoad=0;for(const auto amount:candidate.inventory)inventoryLoad+=amount;
            if(inventoryLoad!=0)continue;
            const auto&province=reservedCapacity.view().provinces[size_t(candidate.region)];
            for(int candidateGood=1;candidateGood<kGoodIdCount;++candidateGood){
                const int64_t price=province.price[size_t(candidateGood)];
                if(province.stock[size_t(candidateGood)]>=16&&price>0&&price<=std::numeric_limits<int64_t>::max()/16&&
                   reservedCapacity.available_funds(id)>=price*16){
                    reservedBuyer=id;reservedRegion=candidate.region;reservedGood=candidateGood;reservedPrice=price;break;
                }
            }
        }
        check(reservedBuyer!=0,"capacity fixture has a buyer, stock for sixteen items, and enough money");
        const auto firstReserved=reservedCapacity.execute(MarketOrderCommand{reservedBuyer,
            reservedCapacity.person(reservedBuyer).decisions+1,0,MarketOrderOperation::Place,
            reservedRegion,reservedGood,9,reservedPrice});
        check(firstReserved.success,"first ordinary order reserves nine inventory places");
        const auto secondReserved=reservedCapacity.execute(MarketOrderCommand{reservedBuyer,
            reservedCapacity.person(reservedBuyer).decisions+1,0,MarketOrderOperation::Place,
            reservedRegion,reservedGood,7,reservedPrice});
        check(secondReserved.success,"second ordinary order reserves the remaining seven inventory places");
        check(reservedCapacity.invariant(),"two-order inventory-capacity fixture remains a valid Realm");
        const auto reservedBeforeMoney=reservedCapacity.total_money();
        reservedCapacity.advance(1);
        check(reservedCapacity.person(reservedBuyer).inventory[size_t(reservedGood)]==16&&
              reservedCapacity.market_orders().empty()&&reservedCapacity.invariant()&&
              reservedCapacity.total_money()==reservedBeforeMoney,
              "two same-day orders fill exactly sixteen inventory places and conserve money");

        Realm cancelled(903,1,128);
        uint64_t cancelBuyer=0;int cancelRegion=0;int64_t cancelLimit=0;
        for(uint64_t id=1;id<=uint64_t(cancelled.count());++id){
            const auto& candidate=cancelled.person(id);
            const auto& province=cancelled.view().provinces[size_t(candidate.region)];
            if(candidate.alive&&candidate.age_days>=6*365&&cancelled.available_funds(id)>=province.price[size_t(good)]){
                cancelBuyer=id;cancelRegion=candidate.region;cancelLimit=province.price[size_t(good)];break;
            }
        }
        check(cancelBuyer!=0,"cancellation fixture has a funded buyer");
        const auto cancelCash=cancelled.person(cancelBuyer).cash;
        const auto cancelFund=cancelled.household_fund(cancelled.household_of(cancelBuyer));
        const auto cancelTotal=cancelled.total_money();
        const auto cancellable=cancelled.execute(MarketOrderCommand{cancelBuyer,cancelled.person(cancelBuyer).decisions+1,0,
            MarketOrderOperation::Place,cancelRegion,good,1,cancelLimit});
        check(cancellable.success,"cancellable order is placed");
        const auto cancelledOrder=cancelled.execute(MarketOrderCommand{cancelBuyer,cancelled.person(cancelBuyer).decisions+1,
            cancellable.session,MarketOrderOperation::Cancel,0,0,0,0});
        check(cancelledOrder.success&&cancelled.market_orders().empty()&&cancelled.market_escrow()==0,
              "owner can cancel an open order before settlement");
        check(cancelled.person(cancelBuyer).cash==cancelCash&&
              cancelled.household_fund(cancelled.household_of(cancelBuyer))==cancelFund&&
              cancelled.total_money()==cancelTotal&&cancelled.invariant(),
              "cancellation returns each funding source exactly and conserves money");
        const auto afterCancel=cancelled.fingerprint();
        const auto duplicateCancel=cancelled.execute(MarketOrderCommand{cancelBuyer,cancelled.person(cancelBuyer).decisions+1,
            cancellable.session,MarketOrderOperation::Cancel,0,0,0,0});
        check(!duplicateCancel.success&&cancelled.fingerprint()==afterCancel,
              "repeated cancellation is rejected without changing the world");

        Realm deathRefund(905,1,128);
        uint64_t decedent=0;int deceasedRegion=0;
        for(uint64_t id=1;id<=uint64_t(deathRefund.count());++id){
            const auto& candidate=deathRefund.person(id);
            if(candidate.alive&&candidate.age_days>=6*365&&deathRefund.household_authority(deathRefund.household_of(id))==id&&candidate.cash>0){
                decedent=id;deceasedRegion=candidate.region;break;
            }
        }
        check(decedent!=0,"death-refund fixture has an adult household authority");
        const int64_t inheritedFund=deathRefund.person(decedent).cash;
        const int64_t deathMoney=deathRefund.total_money();
        check(deathRefund.execute(HouseholdFundCommand{decedent,deathRefund.person(decedent).decisions+1,
            deathRefund.household_of(decedent),HouseholdFundOperation::Deposit,inheritedFund}).success,
            "death-refund fixture places money in the household account");
        const auto pendingDeathOrder=deathRefund.execute(MarketOrderCommand{decedent,deathRefund.person(decedent).decisions+1,0,
            MarketOrderOperation::Place,deceasedRegion,good,1,inheritedFund});
        check(pendingDeathOrder.success&&deathRefund.market_orders().front().household_escrow==inheritedFund,
              "death-refund fixture reserves the whole household account");
        RealmObservationTestAccess::cancel_orders_for_death(deathRefund,decedent);
        check(deathRefund.market_orders().empty()&&deathRefund.market_escrow()==0&&
              deathRefund.household_fund(deathRefund.household_of(decedent))==inheritedFund&&
              deathRefund.total_money()==deathMoney&&deathRefund.invariant(),
              "death lifecycle cancels an open order and restores its household funds before estate settlement");
        std::cout<<"market order checks="<<checks<<'\n';
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"MARKET ORDER TEST FAILED: "<<error.what()<<'\n';
        return 1;
    }
}






