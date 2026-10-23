#include "polyec/market_manager.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>

using namespace polyec;

namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    try {
        check(MarketManager::checked_total(7, 3) == 21, "checked product uses the exact total");
        check(MarketManager::checked_total(0, 9) == 0, "zero unit price is safe");
        check(!MarketManager::checked_total(-1, 1), "negative unit price is rejected");
        check(!MarketManager::checked_total(1, 0), "zero quantity is rejected");
        check(!MarketManager::checked_total(1, -1), "negative quantity is rejected");
        check(MarketManager::checked_total(1, std::numeric_limits<int64_t>::max()) ==
              std::numeric_limits<int64_t>::max(), "maximum representable exact product is accepted");
        check(!MarketManager::checked_total(2, std::numeric_limits<int64_t>::max()),
              "multiplication overflow is rejected before evaluating the product");

        check(MarketManager::purchase_total(4, 3) == 15, "purchase keeps the existing listed-price plus one rule");
        check(!MarketManager::purchase_total(std::numeric_limits<int64_t>::max(), 1),
              "purchase quote rejects unit-price increment overflow");
        check(MarketManager::sale_proceeds(4, 3) == 9, "sale keeps the existing listed-price minus one rule");
        check(MarketManager::sale_proceeds(0, 2) == 2, "sale keeps the one-unit minimum price");
        check(!MarketManager::sale_proceeds(5, std::numeric_limits<int64_t>::max()),
              "sale quote rejects multiplication overflow");

        check(MarketManager::assess_sales_tax(11, 10, 0) == MarketTaxAssessment{1, 10},
              "tax assessment preserves integer quotient and remainder");
        check(MarketManager::assess_sales_tax(1, 10, 95) == MarketTaxAssessment{1, 5},
              "prior tax remainder carries into the next purchase");
        check(MarketManager::assess_sales_tax(std::numeric_limits<int64_t>::max(), 100, 99) ==
              MarketTaxAssessment{std::numeric_limits<int64_t>::max(), 99},
              "maximum gross at a full rate avoids intermediate overflow");
        check(!MarketManager::assess_sales_tax(-1, 10, 0), "negative gross is rejected");
        check(!MarketManager::assess_sales_tax(1, -1, 0), "negative tax rate is rejected");
        check(!MarketManager::assess_sales_tax(1, 101, 0), "tax rate above 100 is rejected");
        check(!MarketManager::assess_sales_tax(1, 10, 100), "invalid prior remainder is rejected");

        const std::vector<MarketAllocationOrder> priorityOrders{
            {30,MarketPriority::Ordinary,2,20},
            {20,MarketPriority::Essential,2,20},
            {10,MarketPriority::Contract,2,20}};
        const auto priorityFill=MarketManager::allocate(3,10,priorityOrders);
        check(priorityFill&&*priorityFill==std::vector<MarketAllocation>{{30,0},{20,1},{10,2}},
              "contract round receives stock before essential and ordinary rounds");
        const std::vector<MarketAllocationOrder> proportionalOrders{
            {1,MarketPriority::Ordinary,1,10},
            {2,MarketPriority::Ordinary,3,10}};
        const auto proportional=MarketManager::allocate(2,10,proportionalOrders);
        check(proportional&&*proportional==std::vector<MarketAllocation>{{1,1},{2,1}},
              "largest-remainder ties resolve by ascending order ID");
        const auto priceMiss=MarketManager::allocate(4,11,{{7,MarketPriority::Ordinary,3,10}});
        check(priceMiss&&priceMiss->front().quantity==0,
              "orders above their maximum unit price receive no stock");
        const auto excessStock=MarketManager::allocate(9,10,{{4,MarketPriority::Ordinary,2,10}});
        check(excessStock&&excessStock->front().quantity==2,
              "stock above demand does not overfill an order");
        const auto extremeShare=MarketManager::allocate(
            std::numeric_limits<int64_t>::max()-1,1,
            {{8,MarketPriority::Ordinary,std::numeric_limits<int64_t>::max()-1,1}});
        check(extremeShare&&extremeShare->front().quantity==std::numeric_limits<int64_t>::max()-1,
              "maximum representable order allocation avoids an intermediate product overflow");
        const auto nearLimit=std::numeric_limits<int64_t>::max();
        const int64_t equalLargeQuantity=(nearLimit-1)/2;
        const int64_t oddLargeStock=nearLimit-2;
        const auto largeProRata=MarketManager::allocate(oddLargeStock,1,{
            {1,MarketPriority::Ordinary,equalLargeQuantity,1},
            {2,MarketPriority::Ordinary,equalLargeQuantity,1}});
        check(largeProRata&&largeProRata->at(0).quantity==oddLargeStock/2+1&&
              largeProRata->at(1).quantity==oddLargeStock/2,
              "large pro-rata products use exact quotient remainders and stable tie-breaking");
        check(!MarketManager::allocate(-1,1,{}),"negative market stock is rejected");
        check(!MarketManager::allocate(1,1,{{1,MarketPriority::Ordinary,1,1},{1,MarketPriority::Ordinary,1,1}}),
              "duplicate order IDs are rejected");
        check(!MarketManager::allocate(1,1,{{1,MarketPriority::Ordinary,0,1}}),
              "nonpositive order quantity is rejected");
        check(!MarketManager::allocate(1,1,{{1,MarketPriority::Ordinary,1,-1}}),
              "negative order price limit is rejected");
        check(!MarketManager::allocate(1,1,{{1,static_cast<MarketPriority>(3),1,1}}),
              "unsupported market priority is rejected");
        const auto overflowBatch=MarketManager::allocate(1,1,{
            {1,MarketPriority::Ordinary,std::numeric_limits<int64_t>::max(),1},
            {2,MarketPriority::Ordinary,1,1}});
        check(!overflowBatch,"overflowing total demand is rejected before allocation");

        const MonthlyPriceSettings settings{};
        check(MarketManager::monthly_price(10,10,5,10,settings)==11,
              "monthly price uses hand-calculated demand pressure and half-up rounding");
        check(MarketManager::monthly_price(10,10,0,20,settings)==9,
              "monthly price falls by the configured limit when stock is twice its target");
        check(MarketManager::monthly_price(10,0,0,0,settings)==10,
              "zero observed demand leaves price unchanged");
        check(MarketManager::monthly_price(1,10,10,0,settings)==1,
              "monthly price respects the configured floor");
        check(MarketManager::monthly_price(1000000,10,0,20,settings)==900000,
              "monthly price respects the configured ceiling");
        check(!MarketManager::monthly_price(10,1,2,1,settings),
              "unmet demand greater than observed demand is rejected");
        check(!MarketManager::monthly_price(10,1,0,1,{10001,1000,1,1000000}),
              "out-of-range price response setting is rejected");
        check(!MarketManager::monthly_price(10,1,0,1,{1000,1000,10,9}),
              "inverted price limits are rejected");

        std::cout << "Market manager checks=" << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after " << checks << ": " << error.what() << '\n';
        return 1;
    }
}






