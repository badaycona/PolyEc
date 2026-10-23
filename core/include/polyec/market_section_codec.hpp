#pragma once

#include "polyec/realm.hpp"
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace polyec::detail {

inline void write_market_section(
    std::ostream& out,int64_t escrow,int settled_day,int processed_month,
    const std::vector<MarketOrder>& orders,
    const std::vector<std::array<MarketMonthlyTotals,kGoodIdCount>>& monthly){
    out<<" MARKET_2 "<<escrow<<' '<<settled_day<<' '<<processed_month<<' '<<orders.size()<<' ';
    for(const auto&order:orders){
        out<<order.id<<' '<<order.owner<<' '<<order.region<<' '<<order.good<<' '
           <<order.placed_day<<' '<<order.settle_day<<' '<<order.funding_household<<' '
           <<static_cast<int>(order.priority)<<' '<<order.quantity<<' '<<order.maximum_unit_price<<' '
           <<order.personal_escrow<<' '<<order.household_escrow<<' '
           <<order.organization_escrow<<' '<<order.organization_id<<' ';
    }
    out<<monthly.size()<<' ';
    for(const auto&region:monthly)for(const auto&totals:region)
        out<<totals.demand<<' '<<totals.unmet<<' ';
}

inline void read_market_section(
    std::istream& in,const std::vector<Person>&people,int regions,uint64_t next_id,int day,
    int64_t&escrow,int&settled_day,int&processed_month,
    std::vector<MarketOrder>&orders,
    std::vector<std::array<MarketMonthlyTotals,kGoodIdCount>>&monthly,
    const GameplayConfig&config,int realm_schema=55){
    std::string tag;size_t count=0,region_count=0;
    in>>tag>>escrow>>settled_day>>processed_month>>count;
    if(!in||(tag!="MARKET_1"&&tag!="MARKET_2")||
       ((realm_schema>=55)!=(tag=="MARKET_2"))||escrow<0||settled_day!=day||
       processed_month!=day/30||count>1000000||count>people.size()*64)
        throw std::runtime_error("Save market header");
    std::vector<MarketOrder> staged;staged.reserve(count);
    std::unordered_set<uint64_t> ids;int64_t staged_escrow=0;
    uint64_t previous_id=0;
    for(size_t row=0;row<count;++row){
        MarketOrder order;int priority=0;
        in>>order.id>>order.owner>>order.region>>order.good>>order.placed_day>>order.settle_day
          >>order.funding_household>>priority>>order.quantity>>order.maximum_unit_price
          >>order.personal_escrow>>order.household_escrow>>order.organization_escrow>>order.organization_id;
        order.priority=static_cast<MarketPriority>(priority);
        const auto reserved=MarketManager::checked_total(order.maximum_unit_price,order.quantity);
        int64_t account_sum=0;
        if(!in||!order.id||order.id<=previous_id||order.id>=next_id||!ids.insert(order.id).second||
           !order.owner||order.owner>people.size()||!people[order.owner-1].alive||order.region<0||order.region>regions||
           order.good<0||order.good>=(realm_schema>=55?kGoodIdCount:27)||order.placed_day<0||order.placed_day>day||
           order.settle_day!=day+1||order.settle_day!=order.placed_day+1||
           static_cast<int>(order.priority)<0||static_cast<int>(order.priority)>2||
           order.quantity<=0||order.quantity>1000000||order.maximum_unit_price<config.market_price_floor()||
           order.maximum_unit_price>config.market_price_ceiling()||!reserved||
           order.personal_escrow<0||order.household_escrow<0||order.organization_escrow<0||
           order.personal_escrow>std::numeric_limits<int64_t>::max()-order.household_escrow)
            throw std::runtime_error("Save market order fields");
        account_sum=order.personal_escrow+order.household_escrow;
        if(order.organization_escrow>0){
            if(order.organization_id<1||order.organization_id>int(kOrganizationCount)||account_sum!=0||order.organization_escrow!=*reserved)throw std::runtime_error("Save market organization escrow");
        }else{
            if(order.organization_id!=0||account_sum!=*reserved)throw std::runtime_error("Save market funding split");
        }
        if((order.household_escrow>0&&(!order.funding_household||order.funding_household>people.size()))||
           (order.household_escrow==0&&order.funding_household!=0))throw std::runtime_error("Save market household reference");
        if(order.personal_escrow>std::numeric_limits<int64_t>::max()-order.household_escrow||
           order.organization_escrow>std::numeric_limits<int64_t>::max()-account_sum)
            throw std::runtime_error("Save market escrow overflow");
        const int64_t liability=order.organization_escrow>0?order.organization_escrow:account_sum;
        if(liability>std::numeric_limits<int64_t>::max()-staged_escrow)throw std::runtime_error("Save market escrow overflow");
        staged_escrow+=liability;
        previous_id=order.id;staged.push_back(order);
    }
    in>>region_count;
    if(!in||region_count!=size_t(regions)+1)throw std::runtime_error("Save market region totals");
    std::vector<std::array<MarketMonthlyTotals,kGoodIdCount>> staged_monthly(region_count);
    const int good_count=realm_schema>=55?kGoodIdCount:27;
    for(auto&region:staged_monthly)for(int good=0;good<good_count;++good){
        auto& totals=region[static_cast<size_t>(good)];
        in>>totals.demand>>totals.unmet;
        if(!in||totals.demand<0||totals.unmet<0||totals.unmet>totals.demand)
            throw std::runtime_error("Save market monthly totals");
    }
    if(staged_escrow!=escrow)throw std::runtime_error("Save market escrow liability mismatch");
    for(const auto&order:staged)if(!ids.contains(order.id))throw std::runtime_error("Save market order index");
    orders=std::move(staged);monthly=std::move(staged_monthly);
}

} // namespace polyec::detail
