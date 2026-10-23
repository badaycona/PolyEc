#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace polyec {

struct MarketTaxAssessment {
    int64_t amount = 0;
    int remainder = 0;
    bool operator==(const MarketTaxAssessment&) const = default;
};

enum class MarketPriority : int { Contract = 0, Essential = 1, Ordinary = 2 };
struct MarketAllocationOrder {
    uint64_t id = 0;
    MarketPriority priority = MarketPriority::Ordinary;
    int64_t quantity = 0;
    int64_t maximum_unit_price = 0;
};
struct MarketAllocation {
    uint64_t order_id = 0;
    int64_t quantity = 0;
    bool operator==(const MarketAllocation&) const = default;
};
struct MonthlyPriceSettings {
    int64_t response_basis_points = 1000;
    int64_t maximum_change_basis_points = 1000;
    int64_t floor = 1;
    int64_t ceiling = 1000000;
};

// Pure checked arithmetic for the current local-market transaction rules.
// This helper selects no strategy and owns no Realm state.
class MarketManager {
public:
    static std::optional<int64_t> checked_total(int64_t unit_price, int64_t quantity);
    static std::optional<int64_t> purchase_total(int64_t listed_price, int64_t quantity);
    static std::optional<int64_t> sale_proceeds(int64_t listed_price, int64_t quantity);
    static std::optional<MarketTaxAssessment> assess_sales_tax(
        int64_t gross, int tax_rate, int prior_remainder);
    // Returns one fill record per input order, in input order. nullopt means
    // an invalid batch or arithmetic that cannot be represented safely.
    static std::optional<std::vector<MarketAllocation>> allocate(
        int64_t available_stock, int64_t listed_price,
        const std::vector<MarketAllocationOrder>& orders);
    static std::optional<int64_t> monthly_price(
        int64_t old_price, int64_t demand, int64_t unmet, int64_t ending_stock,
        const MonthlyPriceSettings& settings);
};

} // namespace polyec
