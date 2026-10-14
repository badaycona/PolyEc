#include "polyec/lab.hpp"
#include <chrono>
#include <exception>

namespace polyec {
bool EconomicLab::load(const std::string& path,uint64_t seed) {
    loaded_=false;error_.clear();history_.clear();memory_.assign(64,0.f);
    try {world_.reset(seed,6,64);} catch(const std::exception& e) {error_=e.what();return false;}
    if(!policy_.load(path)) {error_=policy_.error();return false;}
    loaded_=true;return true;
}
bool EconomicLab::step() {
    if(!loaded_) {if(error_.empty()) error_="Learned policy is not loaded";return false;}
    if(world_.view().done) return false;
    try {
        auto obs=world_.observation();auto mask=world_.mask();
        auto before=world_.view();
        auto begin=std::chrono::steady_clock::now();
        auto output=policy_.predict(std::vector<float>(obs.begin(),obs.end()),memory_,1);
        double milliseconds=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
        if(!output.success) {loaded_=false;error_=output.error;return false;}
        int action=0;float best=-1e30f;
        for(int a=0;a<kActions;++a) if(mask[a] && output.logits[a]>best) {best=output.logits[a];action=a;}
        auto result=world_.step(action);auto after=world_.view();
        memory_=std::move(output.hidden); // exactly once for the committed decision
        history_.push_back({after.decision,action,before.region,after.region,after.day,after.cash,result.reward,milliseconds});
        return true;
    } catch(const std::exception& e) {loaded_=false;error_=e.what();return false;}
}
}
