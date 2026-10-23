#include "polyec/realm.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <stdexcept>
#include <tuple>
#include <unordered_map>

namespace polyec {
namespace {
bool add_money(int64_t&target,int64_t amount){
    if(amount<0||target<0||amount>std::numeric_limits<int64_t>::max()-target)return false;
    target+=amount;return true;
}
int64_t inventory_load(const Person&person){
    int64_t total=0;
    for(const auto amount:person.inventory){
        if(amount<0||amount>std::numeric_limits<int64_t>::max()-total)
            throw std::overflow_error("market inventory load overflow");
        total+=amount;
    }
    return total;
}
}

MarketMonthlyTotals Realm::market_monthly_totals(int region,int good)const{
    if(region<0||region>regions_||good<0||good>=kGoodIdCount)
        throw std::out_of_range("market monthly total index");
    return market_monthly_.at(size_t(region)).at(size_t(good));
}

RealmResult Realm::execute(const MarketOrderCommand&command){
    if(!command.actor||command.actor>people_.size())return {false,"Unknown actor"};
    auto&actor=mutable_person(command.actor);
    if(!actor.alive)return {false,"Actor deceased"};
    if(actor.decisions==UINT64_MAX||actor.sampling==UINT64_MAX||next_fact_==UINT64_MAX)
        return {false,"Decision or fact counter overflow"};
    if(command.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!config_.market_enabled())return {false,"Market orders are disabled in this saved world"};

    if(command.operation==MarketOrderOperation::Cancel){
        const auto found=std::lower_bound(market_orders_.begin(),market_orders_.end(),command.order_id,
            [](const MarketOrder&order,uint64_t id){return order.id<id;});
        if(!command.order_id||found==market_orders_.end()||found->id!=command.order_id||found->owner!=actor.id||
           found->settle_day<=day_)return {false,"Order is missing, belongs to another buyer, or is already settling"};
        const MarketOrder order=*found;
        const int64_t reserved=*MarketManager::checked_total(order.maximum_unit_price,order.quantity);
        if(order.personal_escrow>std::numeric_limits<int64_t>::max()-actor.cash||
           order.household_escrow>0&&!households_.contains(order.funding_household))
            return {false,"Refund account is not available"};
        auto household=household_funds_.find(order.funding_household);
        const int64_t householdCash=household==household_funds_.end()?0:household->second;
        if(order.household_escrow>std::numeric_limits<int64_t>::max()-householdCash||
           order.organization_escrow>0&&organization_funds_[size_t(order.organization_id-1)]>
               std::numeric_limits<int64_t>::max()-order.organization_escrow||
           reserved>market_escrow_||order.personal_escrow>std::numeric_limits<int64_t>::max()-order.household_escrow)
            return {false,"Refund would exceed a money-account limit"};
        if(order.personal_escrow)transfer(market_escrow_,actor.cash,order.personal_escrow);
        if(order.household_escrow){
            auto&fund=household_funds_[order.funding_household];
            transfer(market_escrow_,fund,order.household_escrow);
        }
        if(order.organization_escrow)
            transfer(market_escrow_,organization_funds_[size_t(order.organization_id-1)],order.organization_escrow);
        market_orders_.erase(found);
        actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
        event(actor.id,actor.region,222,0,order.id);
        return {true,"Order cancelled and reservation refunded",next_fact_-1,order.id};
    }
    if(command.operation!=MarketOrderOperation::Place)return {false,"Unknown market-order operation"};
    if(command.order_id||command.region!=actor.region||command.region<0||command.region>regions_||
       command.good<0||command.good>=kGoodIdCount||command.quantity<=0||command.quantity>1000000||
       command.maximum_unit_price<config_.market_price_floor()||
       command.maximum_unit_price>config_.market_price_ceiling()||day_>=365000||next_id_==UINT64_MAX||
       (actor.family==Family::Civilian&&actor.age_days<6*365))
        return {false,"Order terms, age, region, or price limit is invalid"};
    const auto reserve=MarketManager::checked_total(command.maximum_unit_price,command.quantity);
    if(!reserve||*reserve>std::numeric_limits<int64_t>::max()-market_escrow_)
        return {false,"Order reservation is not representable"};
    int64_t pendingSlots=0;
    for(const auto&order:market_orders_)if(order.owner==actor.id&&order.priority==MarketPriority::Ordinary){
        if(order.quantity>std::numeric_limits<int64_t>::max()-pendingSlots)
            return {false,"Existing orders exceed personal inventory capacity"};
        pendingSlots+=order.quantity;
    }
    const int64_t load=inventory_load(actor);
    if(load>16||pendingSlots>16-load||command.quantity>16-load-pendingSlots)
        return {false,"Order exceeds available personal inventory capacity"};
    auto household=household_funds_.find(actor.household_membership.household_id);
    const int64_t householdCash=household==household_funds_.end()?0:household->second;
    if(actor.cash>std::numeric_limits<int64_t>::max()-householdCash||actor.cash+householdCash<*reserve)
        return {false,"Insufficient personal and household funds"};
    const int64_t personalPart=std::min(actor.cash,*reserve);
    const int64_t householdPart=*reserve-personalPart;
    if(householdPart&&household==household_funds_.end())return {false,"Household funds are unavailable"};
    MarketOrder order;order.id=next_id_;order.owner=actor.id;order.region=command.region;order.good=command.good;
    order.placed_day=day_;order.settle_day=day_+1;order.priority=MarketPriority::Ordinary;
    order.quantity=command.quantity;order.maximum_unit_price=command.maximum_unit_price;
    order.personal_escrow=personalPart;order.household_escrow=householdPart;
    order.funding_household=householdPart?actor.household_membership.household_id:0;
    if(personalPart)transfer(actor.cash,market_escrow_,personalPart);
    if(householdPart){
        transfer(household->second,market_escrow_,householdPart);
        if(household->second==0)household_funds_.erase(household);
    }
    ++next_id_;market_orders_.push_back(order);
    actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
    event(actor.id,actor.region,221,command.quantity,order.id);
    return {true,"Order accepted and money reserved",next_fact_-1,order.id};
}

bool Realm::place_essential_market_order(Person&person,Province&province){
    if(!person.alive||!config_.market_enabled()||day_>=365000||next_id_==UINT64_MAX)return false;
    const int64_t price=province.price[0];
    if(price<config_.market_price_floor()||price>config_.market_price_ceiling())return false;
    const auto reserve=MarketManager::checked_total(price,1);
    if(!reserve||*reserve>std::numeric_limits<int64_t>::max()-market_escrow_)return false;
    MarketOrder order;order.id=next_id_;order.owner=person.id;order.region=person.region;order.good=0;
    order.placed_day=day_;order.settle_day=day_;order.priority=MarketPriority::Essential;
    order.quantity=1;order.maximum_unit_price=price;
    const int64_t funds=available_funds(person.id);
    if(funds>=*reserve){
        auto household=household_funds_.find(person.household_membership.household_id);
        const int64_t personal=std::min(person.cash,*reserve);
        const int64_t fromHousehold=*reserve-personal;
        if(fromHousehold&&household==household_funds_.end())return false;
        order.personal_escrow=personal;order.household_escrow=fromHousehold;
        order.funding_household=fromHousehold?person.household_membership.household_id:0;
        if(personal)transfer(person.cash,market_escrow_,personal);
        if(fromHousehold){
            transfer(household->second,market_escrow_,fromHousehold);
            if(household->second==0)household_funds_.erase(household);
        }
    }else if(person.organization>0&&person.organization<=int(kOrganizationCount)&&
             organization_funds_[size_t(person.organization-1)]>=*reserve){
        order.organization_escrow=*reserve;order.organization_id=person.organization;
        transfer(organization_funds_[size_t(person.organization-1)],market_escrow_,*reserve);
    }else return false;
    ++next_id_;market_orders_.push_back(order);
    return true;
}

void Realm::cancel_market_orders(uint64_t owner){
    for(size_t index=0;index<market_orders_.size();){
        const auto&order=market_orders_[index];
        if(order.owner!=owner){++index;continue;}
        auto&person=mutable_person(owner);
        const int64_t reserved=*MarketManager::checked_total(order.maximum_unit_price,order.quantity);
        auto household=household_funds_.find(order.funding_household);
        const int64_t householdCash=household==household_funds_.end()?0:household->second;
        if(order.personal_escrow>std::numeric_limits<int64_t>::max()-person.cash||
           order.household_escrow>std::numeric_limits<int64_t>::max()-householdCash||
           order.organization_escrow>std::numeric_limits<int64_t>::max()-organization_funds_[size_t(std::max(0,order.organization_id-1))]||
           reserved>market_escrow_)throw std::overflow_error("death market-order refund cannot be represented");
        if(order.personal_escrow)transfer(market_escrow_,person.cash,order.personal_escrow);
        if(order.household_escrow){
            auto&fund=household_funds_[order.funding_household];
            transfer(market_escrow_,fund,order.household_escrow);
        }
        if(order.organization_escrow)
            transfer(market_escrow_,organization_funds_[size_t(order.organization_id-1)],order.organization_escrow);
        market_orders_.erase(market_orders_.begin()+std::ptrdiff_t(index));
    }
}

void Realm::settle_market_day(){
    if(!config_.market_enabled()){
        market_settled_day_=day_;return;
    }
    if(day_>market_settled_day_+1||day_<market_settled_day_)throw std::logic_error("market settlement day sequence mismatch");
    if(day_==market_settled_day_){
        const bool hasDue=std::any_of(market_orders_.begin(),market_orders_.end(),
            [&](const MarketOrder&order){return order.settle_day<=day_;});
        if(!hasDue)return;
    }

    auto stagedProvinces=provinces_;
    auto stagedMonthly=market_monthly_;
    auto stagedConsumed=consumed_;
    auto stagedHouseholds=household_funds_;
    auto stagedOrganizations=organization_funds_;
    std::unordered_map<uint64_t,int64_t> stagedCash;
    std::unordered_map<uint64_t,std::array<int64_t,kGoodIdCount>> stagedInventory;
    std::vector<MarketAllocation> allocations(market_orders_.size());
    std::vector<bool> due(market_orders_.size(),false),cancelled(market_orders_.size(),false);
    std::map<std::pair<int,int>,std::vector<size_t>> groups;
    int64_t stagedEscrow=market_escrow_;
    for(size_t i=0;i<market_orders_.size();++i){
        const auto&order=market_orders_[i];
        if(order.settle_day>day_)continue;
        due[i]=true;
        const int64_t reserved=*MarketManager::checked_total(order.maximum_unit_price,order.quantity);
        if(reserved>stagedEscrow)throw std::logic_error("market escrow liability changed before settlement");
        stagedEscrow-=reserved;
        if(!person(order.owner).alive||person(order.owner).region!=order.region){
            cancelled[i]=true;
            const auto cash=stagedCash.contains(order.owner)?stagedCash.at(order.owner):person(order.owner).cash;
            if(order.personal_escrow>std::numeric_limits<int64_t>::max()-cash)
                throw std::overflow_error("market cancellation personal refund overflow");
            stagedCash[order.owner]=cash+order.personal_escrow;
            if(order.household_escrow){
                auto it=stagedHouseholds.find(order.funding_household);
                int64_t value=it==stagedHouseholds.end()?0:it->second;
                if(!households_.contains(order.funding_household)||!add_money(value,order.household_escrow))
                    throw std::overflow_error("market cancellation household refund overflow");
                stagedHouseholds[order.funding_household]=value;
            }
            if(order.organization_escrow){
                auto&value=stagedOrganizations[size_t(order.organization_id-1)];
                if(!add_money(value,order.organization_escrow))throw std::overflow_error("market cancellation organization refund overflow");
            }
            continue;
        }
        groups[{order.region,order.good}].push_back(i);
    }

    for(const auto&[key,indices]:groups){
        const auto [region,good]=key;auto&province=stagedProvinces[size_t(region)];
        std::vector<MarketAllocationOrder> inputs;inputs.reserve(indices.size());
        int64_t demand=0;
        for(const auto index:indices){
            const auto&order=market_orders_[index];
            if(order.quantity>std::numeric_limits<int64_t>::max()-demand)
                throw std::overflow_error("market monthly demand overflow");
            demand+=order.quantity;
            inputs.push_back({order.id,order.priority,order.quantity,order.maximum_unit_price});
        }
        auto fills=MarketManager::allocate(province.stock[size_t(good)],province.price[size_t(good)],inputs);
        if(!fills)throw std::overflow_error("market allocation inputs are invalid or exceed integer limits");
        int64_t filled=0;
        for(size_t n=0;n<indices.size();++n){
            allocations[indices[n]]=fills->at(n);
            if(fills->at(n).quantity>std::numeric_limits<int64_t>::max()-filled)
                throw std::overflow_error("market fill total overflow");
            filled+=fills->at(n).quantity;
        }
        auto&totals=stagedMonthly[size_t(region)][size_t(good)];
        if(demand>std::numeric_limits<int64_t>::max()-totals.demand||
           demand-filled>std::numeric_limits<int64_t>::max()-totals.unmet)
            throw std::overflow_error("market monthly ledger overflow");
        totals.demand+=demand;totals.unmet+=demand-filled;
    }

    std::vector<uint64_t> fedByMarket,unfedByMarket;
    for(size_t index=0;index<market_orders_.size();++index){
        if(!due[index]||cancelled[index])continue;
        const auto&order=market_orders_[index];
        const int64_t fill=allocations[index].quantity;
        auto&province=stagedProvinces[size_t(order.region)];
        if(fill==0){
            auto value=stagedCash.contains(order.owner)?stagedCash.at(order.owner):person(order.owner).cash;
            if(!add_money(value,order.personal_escrow))throw std::overflow_error("market unfilled personal refund overflow");
            if(order.personal_escrow)stagedCash[order.owner]=value;
            if(order.household_escrow){
                auto it=stagedHouseholds.find(order.funding_household);
                int64_t householdValue=it==stagedHouseholds.end()?0:it->second;
                if(!households_.contains(order.funding_household)||!add_money(householdValue,order.household_escrow))
                    throw std::overflow_error("market unfilled household refund overflow");
                stagedHouseholds[order.funding_household]=householdValue;
            }
            if(order.organization_escrow&&!add_money(stagedOrganizations[size_t(order.organization_id-1)],order.organization_escrow))
                throw std::overflow_error("market unfilled organization refund overflow");
        }
        if(fill>0){
            const auto gross=MarketManager::checked_total(province.price[size_t(order.good)],fill);
            if(!gross)throw std::overflow_error("market settlement charge overflow");
            const auto tax=MarketManager::assess_sales_tax(*gross,province.tax,province.sales_tax_remainder);
            if(!tax||tax->amount>std::numeric_limits<int64_t>::max()-province.tax_receipts)
                throw std::overflow_error("market settlement tax overflow");
            const int64_t sellerNet=*gross-tax->amount;
            if(sellerNet>std::numeric_limits<int64_t>::max()-province.firm||
               tax->amount>std::numeric_limits<int64_t>::max()-province.treasury)
                throw std::overflow_error("market seller account overflow");
            if(fill>province.stock[size_t(order.good)])throw std::logic_error("market filled more than available stock");
            province.firm+=sellerNet;province.treasury+=tax->amount;province.tax_receipts+=tax->amount;
            province.sales_tax_remainder=tax->remainder;province.stock[size_t(order.good)]-=fill;
            const int64_t charge=*gross;
            const int64_t personalSpent=std::min(charge,order.personal_escrow);
            const int64_t afterPersonal=charge-personalSpent;
            const int64_t householdSpent=std::min(afterPersonal,order.household_escrow);
            const int64_t organizationSpent=afterPersonal-householdSpent;
            if(organizationSpent>order.organization_escrow)throw std::logic_error("market charge exceeds order escrow");
            const int64_t personalRefund=order.personal_escrow-personalSpent;
            const int64_t householdRefund=order.household_escrow-householdSpent;
            const int64_t organizationRefund=order.organization_escrow-organizationSpent;
            if(personalRefund){
                auto value=stagedCash.contains(order.owner)?stagedCash.at(order.owner):person(order.owner).cash;
                if(!add_money(value,personalRefund))throw std::overflow_error("market personal refund overflow");
                stagedCash[order.owner]=value;
            }
            if(householdRefund){
                auto it=stagedHouseholds.find(order.funding_household);
                int64_t value=it==stagedHouseholds.end()?0:it->second;
                if(!households_.contains(order.funding_household)||!add_money(value,householdRefund))
                    throw std::overflow_error("market household refund overflow");
                stagedHouseholds[order.funding_household]=value;
            }
            if(organizationRefund&&!add_money(stagedOrganizations[size_t(order.organization_id-1)],organizationRefund))
                throw std::overflow_error("market organization refund overflow");

            if(order.priority==MarketPriority::Essential){
                if(order.good!=0||fill>1)throw std::logic_error("unsupported essential market order");
                if(stagedConsumed[0]==std::numeric_limits<int64_t>::max())throw std::overflow_error("food consumption ledger overflow");
                ++stagedConsumed[0];fedByMarket.push_back(order.owner);
            }else{
                auto inventoryIt=stagedInventory.try_emplace(order.owner,person(order.owner).inventory).first;
                int64_t load=0;
                for(const auto amount:inventoryIt->second){
                    if(amount<0||amount>std::numeric_limits<int64_t>::max()-load)
                        throw std::overflow_error("market buyer inventory load overflow");
                    load+=amount;
                }
                // Every valid order settles one game day after placement, and
                // this phase runs after day_begin advances that day. All
                // ordinary orders still in this settlement are therefore due;
                // command admission already reserved slots across open orders.
                constexpr int64_t reservedSlots=0;
                auto&goodAmount=inventoryIt->second[size_t(order.good)];
                if(load>16||fill>16-load||reservedSlots>16-load-fill||
                   fill>std::numeric_limits<int64_t>::max()-goodAmount)
                    throw std::logic_error("market delivery exceeds buyer inventory capacity");
                goodAmount+=fill;
            }
        }
        if(order.priority==MarketPriority::Essential&&fill==0)unfedByMarket.push_back(order.owner);
    }

    // Rebuild open orders only after every region/product has been validated.
    std::vector<MarketOrder> remainingOrders;remainingOrders.reserve(market_orders_.size());
    for(size_t i=0;i<market_orders_.size();++i)if(!due[i])remainingOrders.push_back(market_orders_[i]);

    // Commit the staged regional books and account refunds. Inventory fills
    // are applied in a separate actor delta map to keep the commit scalar-only.
    // (The map is populated in the follow-up block after capacity validation.)
    provinces_=std::move(stagedProvinces);market_monthly_=std::move(stagedMonthly);
    consumed_=stagedConsumed;household_funds_=std::move(stagedHouseholds);organization_funds_=stagedOrganizations;
    for(const auto&[id,cash]:stagedCash)mutable_person(id).cash=cash;
    for(const auto&[id,inventory]:stagedInventory)mutable_person(id).inventory=inventory;
    market_escrow_=stagedEscrow;market_orders_=std::move(remainingOrders);market_settled_day_=day_;
    for(const auto id:fedByMarket){auto&person=mutable_person(id);person.nutrition=std::min(100,person.nutrition+20);person.health=std::min(100,person.health+1);}
    for(const auto id:unfedByMarket)record_unmet_essential_need(mutable_person(id));
}

bool Realm::update_monthly_market_prices(){
    if(!config_.market_enabled()||day_<=0||day_%30!=0)return true;
    const int month=day_/30;if(month<=market_processed_month_)return true;
    const MonthlyPriceSettings settings{config_.market_response_basis_points(),
        config_.market_maximum_change_basis_points(),config_.market_price_floor(),config_.market_price_ceiling()};
    auto staged=provinces_;
    for(int region=0;region<=regions_;++region)for(int good=0;good<kGoodIdCount;++good){
        const auto&totals=market_monthly_[size_t(region)][size_t(good)];
        const auto price=MarketManager::monthly_price(provinces_[size_t(region)].price[size_t(good)],
            totals.demand,totals.unmet,provinces_[size_t(region)].stock[size_t(good)],settings);
        if(!price)return false;
        staged[size_t(region)].price[size_t(good)]=static_cast<int>(*price);
    }
    provinces_=std::move(staged);
    for(auto&region:market_monthly_)for(auto&totals:region)totals={};
    market_processed_month_=month;
    return true;
}

} // namespace polyec
