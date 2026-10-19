#pragma once
#include "polyec/realm.hpp"
#include "polyec/policy.hpp"
#include <memory>
#include <array>
#include <optional>
namespace polyec {
// Only commit the exact candidate/terms that the policy observed.
std::array<float,realm_actions> revalidate_snapshot(const Realm&,uint64_t,const std::array<Candidate,realm_actions>&,const std::array<float,realm_actions>&);
class RealmRuntime {
public:
    bool load_models(const std::array<std::string,3>&paths,const std::string&bundle,uint64_t seed,int population=10000);
    bool advance(int days);
    bool step_one_day();
    bool pause(std::optional<uint64_t> expected_pause_revision = std::nullopt);
    bool resume(std::optional<uint64_t> expected_pause_revision = std::nullopt);
    bool paused() const;
    uint64_t pause_revision() const;
    bool command(uint64_t id,int action,const std::string& expected="");
    bool route(uint64_t id,int from,int to,bool open){if(!loaded_)return false;auto r=world_->route_command(id,from,to,open);error_=r.success?"":r.reason;return r.success;}
    bool save(const std::string&path);
    bool restore(const std::string&path, bool interactive = false);
    bool restore_interactive(const std::string&path);
    bool choose(int card){return world_&&world_->choose(card);}
    bool reroll(int slot){return world_&&world_->reroll(slot);}
    const Realm* world()const{return world_.get();}
    const std::string& error()const{return error_;}
    bool loaded()const{return loaded_;}
    double inference_ms()const{return inference_ms_;}
    uint64_t inference_count()const{return inference_count_;}
    void stop(const std::string&message){loaded_=false;error_=message;}
private:
    std::unique_ptr<Realm> world_;
    std::array<std::unique_ptr<NativePolicy>,3> policies_;
    std::vector<std::array<float,64>> memories_;
    std::vector<Family> memory_roles_;void sync_roles();
    uint64_t seed_=1,inference_count_=0;double inference_ms_=0;
    std::string bundle_,error_;bool loaded_=false;
    bool decisions(Family family);
};
}
