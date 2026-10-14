#pragma once
#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace polyec {
inline constexpr int kGoods=7, kRegions=6, kActions=21;
inline constexpr int kState=100, kFeatures=10, kObservation=kState+kActions*kFeatures;
inline constexpr std::array<int,7> kReference{20,14,35,45,18,65,90};
struct Market {
    int64_t firm_cash=20000, household_cash=20000;
    std::array<int64_t,kGoods> stock{};
    std::array<int,kGoods> price{};
    std::array<float,kGoods> geography{};
};
struct TradeView {
    int day=0, decision=0, region=0, transit_days=0, destination=0;
    int64_t cash=500, carrier_cash=0;
    std::array<int64_t,kGoods> cargo{};
    std::vector<Market> markets;
    std::array<int64_t,kGoods> produced{}, consumed{};
    int trades=0, travels=0, holds=0;
    bool done=false;
};
struct TradeStep { float reward=0; bool done=false; int action=0; };

// Early economic curriculum only. All currency/stock mutations, production,
// public observations and action validation live here for app and training.
class TradeWorld {
public:
    explicit TradeWorld(uint64_t seed=1,int regions=6,int horizon=64);
    void reset(uint64_t seed,int regions=6,int horizon=64);
    TradeView view() const { return state_; }
    std::array<float,kObservation> observation() const;
    std::array<float,kActions> mask() const;
    bool commit(int action); // Reject is atomic; does NOT advance time.
    TradeStep step(int action); // Valid action then seven world days.
    bool invariant() const;
    uint64_t fingerprint() const;
    int64_t wealth() const;
    int64_t liquidation_value() const;
    int64_t total_money() const;
    int horizon() const { return horizon_; }
    static std::string action_name(int action);
private:
    TradeView state_;
    std::mt19937_64 rng_;
    int horizon_=64;
    std::array<int64_t,kGoods> initial_stock_{};
    int64_t initial_money_=0;
    int lot(int action) const;
    int travel_distance(int destination) const;
    int travel_fee(int destination) const;
    void advance_day();
    void liquidate();
    void consume(Market& m,int good,int64_t quantity);
    void produce(Market& m,int good,int64_t quantity);
};
struct ScaleResult { uint64_t persons=0,edges=0,bytes=0,checksum=0; double milliseconds=0; };
ScaleResult citizen_primitive_spike(uint64_t count,int iterations=12);
}
