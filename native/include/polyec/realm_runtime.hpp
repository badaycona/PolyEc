#pragma once
#include "polyec/realm.hpp"
#include "polyec/policy.hpp"
#include <memory>
#include <array>
#include <optional>
namespace polyec {
struct RealmRoutePreview {
    bool supported = false;
    bool reachable = false;
    int route_days = 0;
    int person_days = 0;
    int64_t route_fee = 0;
    int64_t migration_fee = 0;
    bool travel_available = false;
    bool migration_available = false;
    std::vector<uint8_t> regions;
    std::vector<NationalTransportRoute> legs;
};
// Only commit the exact candidate/terms that the policy observed.
std::array<float,realm_actions> revalidate_snapshot(const Realm&,uint64_t,const std::array<Candidate,realm_actions>&,const std::array<float,realm_actions>&);
class RealmRuntime {
public:
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    static constexpr size_t runtime_phase_count = 7;
    const std::array<uint64_t, runtime_phase_count>& runtime_phase_profile_ns() const {
        return runtime_phase_profile_ns_;
    }
#endif
    bool load_models(const std::array<std::string,3>&paths,const std::string&bundle,uint64_t seed,int population=10000,
                     bool allow_incompatible_pilot_for_development=false,int complex_actors=0);
    // Clearly labeled validation mode: real Atlas Realm rules, no autonomous NPC policy.
    bool load_atlas_manual_test(uint64_t seed,int population=512);
    bool advance(int days);
    bool step_one_day();
    bool pause(std::optional<uint64_t> expected_pause_revision = std::nullopt);
    bool resume(std::optional<uint64_t> expected_pause_revision = std::nullopt);
    bool paused() const;
    uint64_t pause_revision() const;
    bool command(uint64_t id,int action,const std::string& expected="");
    bool military_combat_consent(uint64_t actor,bool consent,const std::string& expected="");
    bool military_raid(int partner_id,int force_units,const std::string& expected="");
    bool military_battle(uint64_t actor,uint64_t raid_id,bool retreat,const std::vector<uint64_t>& defenders,
                         const std::string& expected="");
    bool military_counterattack(uint64_t actor,int region,bool retreat,const std::vector<uint64_t>& defenders,
                                const std::string& expected="");
    bool foreign_trade_suspension(uint64_t actor,int partner_id,bool suspended,const std::string& expected="");
    bool foreign_trade_offer(uint64_t actor,int partner_id,ForeignTradeOperation operation,int good,
                             int64_t quantity,const std::string& expected="");
    RealmRoutePreview route_preview(uint64_t actor,int destination) const;
    bool land_title_transfer(uint64_t actor,uint64_t holding_id,uint64_t recipient,const std::string& expected="");
    bool land_use_grant(uint64_t actor,uint64_t holding_id,uint64_t grantee,const std::string& expected="");
    bool land_use_revoke(uint64_t actor,uint64_t holding_id,const std::string& expected="");
    bool private_residence_build(uint64_t actor,uint64_t holding_id,const std::string& expected="");
    bool private_housing_command(uint64_t actor,uint64_t residence_id,PrivateHousingOperation operation,
                                 uint64_t tenant_household=0,int64_t rent_per_period=0,
                                 const std::string& expected="");
    bool market_order(uint64_t actor,MarketOrderOperation operation,uint64_t order_id,int region,int good,
                      int64_t quantity,int64_t maximum_unit_price,uint64_t&result_order_id,
                      const std::string&expected="");
    bool debug_worksite_cycle(uint64_t actor,const std::string& expected,
                              uint64_t& establishment_id,int64_t& actor_cash,
                              int64_t& site_cash,int64_t& fiber,int64_t& cloth,
                              int64_t& paid_units);
    bool route(uint64_t id,int from,int to,bool open){if(!loaded_)return false;auto r=world_->route_command(id,from,to,open);error_=r.success?"":r.reason;return r.success;}
    bool save(const std::string&path);
    bool restore(const std::string&path, bool interactive = false);
    bool restore_interactive(const std::string&path);
    bool choose(int card){return world_&&world_->choose(card);}
    bool reroll(int slot){return world_&&world_->reroll(slot);}
    const Realm* world()const{return world_.get();}
    const std::string& error()const{return error_;}
    bool loaded()const{return loaded_;}
    bool atlas_manual_test()const{return loaded_&&atlas_manual_test_;}
    double inference_ms()const{return inference_ms_;}
    uint64_t inference_count()const{return inference_count_;}
    void stop(const std::string&message){loaded_=false;atlas_manual_test_=false;error_=message;}
private:
    std::unique_ptr<Realm> world_;
    std::array<std::unique_ptr<NativePolicy>,3> policies_;
    std::vector<std::array<float,64>> memories_;
    std::vector<Family> memory_roles_;void sync_roles();
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    std::array<uint64_t, runtime_phase_count> runtime_phase_profile_ns_{};
#endif
    uint64_t seed_=1,inference_count_=0;double inference_ms_=0;
    std::string bundle_,error_;bool loaded_=false,atlas_manual_test_=false;
    bool allow_incompatible_pilot_for_development_=false;
    std::string loaded_model_contract_;
    bool decisions(Family family);
};
}
