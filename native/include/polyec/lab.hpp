#pragma once
#include "polyec/policy.hpp"
#include "polyec/trade.hpp"
#include <vector>

namespace polyec {
struct LabDecision {
    int id=0,action=0,origin=0,destination=0,day=0;
    int64_t cash=0;
    float reward=0;
    double inference_ms=0;
};
class EconomicLab {
public:
    bool load(const std::string& path,uint64_t seed);
    bool step();
    void stop(const std::string& message) {loaded_=false;error_=message;memory_.assign(64,0.f);}
    TradeView view() const {return world_.view();}
    const std::vector<LabDecision>& history() const {return history_;}
    const std::string& error() const {return error_;}
    bool loaded() const {return loaded_;}
    uint64_t fingerprint() const {return world_.fingerprint();}
private:
    TradeWorld world_;
    NativePolicy policy_;
    std::vector<float> memory_=std::vector<float>(64,0.f);
    std::vector<LabDecision> history_;
    std::string error_;
    bool loaded_=false;
};
}
