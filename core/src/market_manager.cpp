#include "polyec/market_manager.hpp"

#include <algorithm>
#include <limits>
#include <numeric>
#include <unordered_set>
#include <utility>

namespace polyec {
namespace {
struct MulDivResult { int64_t quotient=0,remainder=0; };

// Exact floor(a*b/divisor) and its remainder without forming a*b. Allocation
// calls this only when 0 <= a <= divisor and 0 <= b <= divisor, so the result
// is representable in int64_t.
std::optional<MulDivResult> mul_div(int64_t a,int64_t b,int64_t divisor){
    if(a<0||b<0||divisor<=0||a>divisor||b>divisor)return std::nullopt;
    int64_t quotient=0,remainder=0;
    for(int bit=62;bit>=0;--bit){
        if(quotient>std::numeric_limits<int64_t>::max()/2)return std::nullopt;
        quotient*=2;
        if(remainder>=divisor-remainder){
            remainder-=divisor-remainder;
            if(quotient==std::numeric_limits<int64_t>::max())return std::nullopt;
            ++quotient;
        }else remainder+=remainder;
        if((b>>bit)&1){
            if(remainder>=divisor-a){
                remainder-=divisor-a;
                if(quotient==std::numeric_limits<int64_t>::max())return std::nullopt;
                ++quotient;
            }else remainder+=a;
        }
    }
    return MulDivResult{quotient,remainder};
}

// floor(numerator*scale/denominator), where 0 <= numerator <= denominator.
// The binary search compares against ceil(k*denominator/scale), calculated
// without an overflowing product.
int64_t fraction_scaled(int64_t numerator,int64_t denominator,int64_t scale){
    int64_t low=0,high=scale;
    while(low<high){
        const int64_t middle=low+(high-low+1)/2;
        const int64_t whole=(denominator/scale)*middle;
        const int64_t tail=(denominator%scale)*middle;
        const int64_t threshold=whole+tail/scale+(tail%scale!=0);
        if(numerator>=threshold)low=middle;else high=middle-1;
    }
    return low;
}
}

std::optional<int64_t> MarketManager::checked_total(int64_t unit_price, int64_t quantity) {
    if (unit_price < 0 || quantity <= 0) return std::nullopt;
    if (unit_price != 0 && quantity > std::numeric_limits<int64_t>::max() / unit_price)
        return std::nullopt;
    return unit_price * quantity;
}

std::optional<int64_t> MarketManager::purchase_total(int64_t listed_price, int64_t quantity) {
    if (listed_price < 0 || listed_price == std::numeric_limits<int64_t>::max())
        return std::nullopt;
    return checked_total(listed_price + 1, quantity);
}

std::optional<int64_t> MarketManager::sale_proceeds(int64_t listed_price, int64_t quantity) {
    if (quantity <= 0) return std::nullopt;
    const int64_t unit_price = std::max<int64_t>(1, listed_price - (listed_price > 0 ? 1 : 0));
    return checked_total(unit_price, quantity);
}

std::optional<MarketTaxAssessment> MarketManager::assess_sales_tax(
    int64_t gross, int tax_rate, int prior_remainder) {
    if (gross < 0 || tax_rate < 0 || tax_rate > 100 || prior_remainder < 0 || prior_remainder >= 100)
        return std::nullopt;

    // Keep the exact integer formula while avoiding gross * tax_rate as an
    // intermediate. Since rate <= 100, the quotient component cannot exceed
    // gross; the remainder component is bounded below 10,000.
    const int64_t whole = (gross / 100) * tax_rate;
    const int64_t fractional = (gross % 100) * tax_rate + prior_remainder;
    const int64_t amount = whole + fractional / 100;
    return MarketTaxAssessment{amount, int(fractional % 100)};
}

std::optional<std::vector<MarketAllocation>> MarketManager::allocate(
    int64_t available_stock,int64_t listed_price,
    const std::vector<MarketAllocationOrder>&orders){
    if(available_stock<0||listed_price<0)return std::nullopt;
    std::unordered_set<uint64_t> ids;
    for(const auto&order:orders){
        if(!order.id||order.quantity<=0||order.maximum_unit_price<0||
           static_cast<int>(order.priority)<static_cast<int>(MarketPriority::Contract)||
           static_cast<int>(order.priority)>static_cast<int>(MarketPriority::Ordinary)||
           !ids.insert(order.id).second)return std::nullopt;
    }
    std::vector<MarketAllocation> result;result.reserve(orders.size());
    for(const auto&order:orders)result.push_back({order.id,0});
    int64_t remaining=available_stock;
    for(const auto priority:{MarketPriority::Contract,MarketPriority::Essential,MarketPriority::Ordinary}){
        std::vector<size_t> eligible;
        int64_t total=0;
        for(size_t i=0;i<orders.size();++i){
            const auto&order=orders[i];
            if(order.priority!=priority||order.maximum_unit_price<listed_price)continue;
            if(order.quantity>std::numeric_limits<int64_t>::max()-total)return std::nullopt;
            total+=order.quantity;eligible.push_back(i);
        }
        if(total==0||remaining==0)continue;
        if(remaining>=total){
            for(const auto index:eligible)result[index].quantity=orders[index].quantity;
            remaining-=total;
            continue;
        }
        struct Remainder {size_t index;int64_t value;uint64_t id;};
        std::vector<Remainder> remainders;remainders.reserve(eligible.size());
        int64_t allocated=0;
        for(const auto index:eligible){
            const auto share=mul_div(remaining,orders[index].quantity,total);
            if(!share||share->quotient>remaining-allocated)return std::nullopt;
            result[index].quantity=share->quotient;allocated+=share->quotient;
            remainders.push_back({index,share->remainder,orders[index].id});
        }
        std::sort(remainders.begin(),remainders.end(),[](const auto&a,const auto&b){
            return a.value!=b.value?a.value>b.value:a.id<b.id;
        });
        int64_t left=remaining-allocated;
        for(const auto&entry:remainders){
            if(left==0)break;
            if(result[entry.index].quantity>=orders[entry.index].quantity)return std::nullopt;
            ++result[entry.index].quantity;--left;
        }
        if(left!=0)return std::nullopt;
        remaining=0;
    }
    return result;
}

std::optional<int64_t> MarketManager::monthly_price(
    int64_t old_price,int64_t demand,int64_t unmet,int64_t ending_stock,
    const MonthlyPriceSettings&settings){
    constexpr int64_t scale=10000;
    if(settings.response_basis_points<0||settings.response_basis_points>scale||
       settings.maximum_change_basis_points<0||settings.maximum_change_basis_points>scale||
       settings.floor<=0||settings.ceiling<settings.floor||settings.ceiling>1000000||
       old_price<settings.floor||old_price>settings.ceiling||demand<0||unmet<0||unmet>demand||ending_stock<0)return std::nullopt;
    if(demand==0)return old_price;
    const int64_t target=std::max<int64_t>(1,demand);
    int64_t supply_pressure=0;
    if(ending_stock<=target)
        supply_pressure=fraction_scaled(target-ending_stock,target,scale);
    else if(ending_stock-target>=target)
        supply_pressure=-scale;
    else
        supply_pressure=-fraction_scaled(ending_stock-target,target,scale);
    const int64_t demand_pressure=fraction_scaled(unmet,demand,scale);
    const int64_t pressure=std::clamp(demand_pressure+supply_pressure,-scale,scale);
    const int64_t raw_change=settings.response_basis_points*pressure/scale;
    const int64_t change=std::clamp(raw_change,-settings.maximum_change_basis_points,
                                     settings.maximum_change_basis_points);
    const int64_t multiplier=scale+change;
    // The configured ceiling is 1,000,000 and multiplier is at most 20,000.
    const int64_t scaled=old_price*multiplier;
    const int64_t rounded=(scaled+scale/2)/scale;
    return std::clamp(rounded,settings.floor,settings.ceiling);
}

} // namespace polyec
