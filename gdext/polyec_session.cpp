#include "polyec/sim.hpp"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <chrono>
#include <memory>
#ifdef POLYEC_AI_ENABLED
#include "polyec/lab.hpp"
#include "polyec/realm_runtime.hpp"
#include <godot_cpp/classes/file_access.hpp>
#endif

using namespace godot;

class PolyEcSession : public RefCounted {
    GDCLASS(PolyEcSession, RefCounted)
    using Clock = std::chrono::steady_clock;
    std::shared_ptr<polyec::GameWorld> world = std::make_shared<polyec::GameWorld>();
    polyec::SimulationController controller{world};
    Clock::time_point anchor = Clock::now();
    bool active = true;
#ifdef POLYEC_AI_ENABLED
    polyec::EconomicLab lab;
    polyec::RealmRuntime realm;
#endif

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("snapshot"), &PolyEcSession::snapshot);
        ClassDB::bind_method(D_METHOD("command", "name", "id"), &PolyEcSession::command, DEFVAL(int64_t(0)));
        ClassDB::bind_method(D_METHOD("tick"), &PolyEcSession::tick);
        ClassDB::bind_method(D_METHOD("set_active", "value"), &PolyEcSession::set_active);
        ClassDB::bind_method(D_METHOD("set_speed", "value"), &PolyEcSession::set_speed);
        ClassDB::bind_method(D_METHOD("rl_available"), &PolyEcSession::rl_available);
#ifdef POLYEC_AI_ENABLED
        ClassDB::bind_method(D_METHOD("rl_load", "path", "sha256", "seed"), &PolyEcSession::rl_load);
        ClassDB::bind_method(D_METHOD("rl_step"), &PolyEcSession::rl_step);
        ClassDB::bind_method(D_METHOD("rl_snapshot"), &PolyEcSession::rl_snapshot);
        ClassDB::bind_method(D_METHOD("realm_load", "paths", "hashes", "seed", "population"), &PolyEcSession::realm_load);
        ClassDB::bind_method(D_METHOD("realm_step", "days"), &PolyEcSession::realm_step);
        ClassDB::bind_method(D_METHOD("realm_snapshot", "actor"), &PolyEcSession::realm_snapshot,DEFVAL(int64_t(1)));
        ClassDB::bind_method(D_METHOD("realm_command", "actor", "action"), &PolyEcSession::realm_command);
        ClassDB::bind_method(D_METHOD("realm_draft", "card", "reroll"), &PolyEcSession::realm_draft);
        ClassDB::bind_method(D_METHOD("realm_save", "path", "restore"), &PolyEcSession::realm_save);
        ClassDB::bind_method(D_METHOD("realm_route", "actor", "from", "to", "open"), &PolyEcSession::realm_route);
#endif
    }
public:
    bool rl_available() const {
#ifdef POLYEC_AI_ENABLED
        return true;
#else
        return false;
#endif
    }
#ifdef POLYEC_AI_ENABLED
    bool realm_load(const Array&paths,const Array&hashes,int64_t seed,int64_t population){
        controller.set_paused(true);anchor=Clock::now();
        if(paths.size()!=3||hashes.size()!=3||population<128||population>100000){realm.stop("Invalid model bundle/profile");return false;}
        std::array<std::string,3> names;std::string bundle;
        for(int i=0;i<3;++i){String path=paths[i],hash=hashes[i];if(hash.length()!=64||FileAccess::get_sha256(path)!=hash){realm.stop("Realm model checksum mismatch");return false;}names[i]=path.utf8().get_data();bundle+=hash.utf8().get_data();}
        return realm.load_models(names,bundle,uint64_t(seed),int(population));
    }
    bool realm_step(int64_t days){if(!active||days<1||days>30)return false;return realm.advance(int(days));}
    bool realm_command(int64_t actor,int64_t action){if(!active)return false;return realm.command(uint64_t(actor),int(action));}
    bool realm_route(int64_t actor,int64_t from,int64_t to,bool open){return active&&realm.route(uint64_t(actor),int(from),int(to),open);}
    bool realm_draft(int64_t card,bool reroll){return reroll?realm.reroll(int(card)):realm.choose(int(card));}
    bool realm_save(const String&path,bool restore){return restore?realm.restore(path.utf8().get_data()):realm.save(path.utf8().get_data());}
    Dictionary realm_snapshot(int64_t actor=1)const{
        Dictionary d;d["loaded"]=realm.loaded();d["error"]=String::utf8(realm.error().c_str());
        auto*w=realm.world();if(!w)return d;if(actor<1||actor>w->count())actor=1;auto v=w->view(uint64_t(actor));
        d["day"]=v.day;d["persons"]=int64_t(v.persons);d["alive"]=int64_t(v.alive);d["children"]=int64_t(v.children);d["decisions"]=int64_t(v.decisions);d["money"]=v.money;d["invariant"]=w->invariant();d["fingerprint"]=String::num_uint64(w->fingerprint());d["inferences"]=int64_t(realm.inference_count());d["inference_ms"]=realm.inference_ms();
        Array cards;for(int c:v.draft.cards)cards.push_back(c);d["cards"]=cards;d["chosen"]=v.draft.chosen;d["tier"]=v.draft.tier;d["rerolled"]=v.draft.rerolled;
        Array regions;for(auto&p:v.provinces){Dictionary r;r["treasury"]=p.treasury;r["firm"]=p.firm;r["unrest"]=p.unrest;r["road"]=p.road;r["jobs"]=p.jobs;r["workers"]=p.employed;r["school"]=p.school;r["students"]=p.school_used;r["tax"]=p.tax;Array prices,stock;for(int g=0;g<7;++g){prices.push_back(p.price[g]);stock.push_back(p.stock[g]);}r["prices"]=prices;r["stock"]=stock;regions.push_back(r);}d["regions"]=regions;
        auto&p=w->person(actor);Dictionary person;person["id"]=actor;person["age"]=p.age_days/365;person["role"]=int(p.family);person["cash"]=p.cash;person["region"]=p.region;person["health"]=p.health;person["skill"]=p.skill;person["goal"]=p.goal;person["alive"]=p.alive;person["debt"]=w->debt(actor);person["decisions"]=int64_t(p.decisions);person["household"]=int64_t(p.household);Array stats;for(int x:p.stats.values)stats.push_back(x);person["stats"]=stats;d["person"]=person;
        Array actions;auto cs=w->candidates(actor);auto ms=w->mask(actor);for(int a=0;a<polyec::realm_actions;++a)if(a==0||cs[a].kind!=polyec::Intent::Hold){Dictionary c;c["index"]=a;c["legal"]=ms[a]>.5f;c["name"]=String::utf8(polyec::Realm::action_name(cs[a]).c_str());c["target"]=cs[a].target;c["good"]=cs[a].good;c["amount"]=cs[a].amount;c["term"]=cs[a].term;actions.push_back(c);}d["actions"]=actions;
        Array facts;int start=std::max(0,int(v.facts.size())-40);for(int i=start;i<int(v.facts.size());++i){auto&f=v.facts[i];Dictionary x;x["id"]=int64_t(f.id);x["parent"]=int64_t(f.parent);x["actor"]=int64_t(f.actor);x["target"]=int64_t(f.target);x["day"]=f.day;x["region"]=f.region;x["kind"]=f.kind;x["amount"]=f.amount;facts.push_back(x);}d["facts"]=facts;
        d["loans"]=int(w->loans().size());d["projects"]=int(w->projects().size());d["edges"]=int(w->relations().size());return d;
    }
    bool rl_load(const String& path,const String& sha256,int64_t seed) {
        controller.set_paused(true);
        anchor=Clock::now();
        if(sha256.length()!=64 || FileAccess::get_sha256(path)!=sha256) {
            lab.stop("Model checksum mismatch; RL stopped");return false;
        }
        return lab.load(path.utf8().get_data(),uint64_t(seed));
    }
    Dictionary rl_step() {
        Dictionary result;
        if(!active) {result["success"]=false;result["error"]="Window is inactive";return result;}
        result["success"]=lab.step();
        result["error"]=String::utf8(lab.error().c_str());
        return result;
    }
    Dictionary rl_snapshot() const {
        auto v=lab.view();Dictionary out;
        out["loaded"]=lab.loaded();out["error"]=String::utf8(lab.error().c_str());
        out["runtime"]=String::utf8(polyec::NativePolicy::version().c_str());
        out["day"]=v.day;out["decision"]=v.decision;out["region"]=v.region;
        out["cash"]=v.cash;out["profit"]=v.cash-500;out["done"]=v.done;
        out["fingerprint"]=String::num_uint64(lab.fingerprint());
        Array cargo;for(auto quantity:v.cargo) cargo.push_back(quantity);out["cargo"]=cargo;
        Array markets;
        static const char* names[]={"Thanh Điền","Hắc Sơn","Bạch Khâu","Thiết Lĩnh","Lâm Giang","Vân Hải"};
        for(int r=0;r<int(v.markets.size());++r) {
            Dictionary m;m["name"]=String::utf8(names[r]);m["id"]=r;
            Array prices,stock;
            for(int g=0;g<7;++g) {prices.push_back(v.markets[r].price[g]);stock.push_back(v.markets[r].stock[g]);}
            m["prices"]=prices;m["stock"]=stock;
            m["firm_cash"]=v.markets[r].firm_cash;m["household_cash"]=v.markets[r].household_cash;
            markets.push_back(m);
        }
        out["markets"]=markets;
        Array decisions;
        for(const auto& d:lab.history()) {
            Dictionary item;item["id"]=d.id;item["action"]=d.action;item["from"]=d.origin;
            item["to"]=d.destination;item["day"]=d.day;item["cash"]=d.cash;
            item["inference_ms"]=d.inference_ms;decisions.push_back(item);
        }
        out["history"]=decisions;
        out["inferences"]=int64_t(lab.history().size());
        return out;
    }
#endif
    PolyEcSession() {
        world->add_territory({polyec::EntityId{7}, "Thanh Điền", 75, 95, 490, 475});
        world->add_territory({polyec::EntityId{8}, "Hắc Sơn", 480, 70, 430, 485});
    }
    Dictionary snapshot() const {
        auto view = world->player_view();
        Dictionary result;
        result["day"] = view.date.day;
        result["paused"] = view.paused;
        result["selected"] = int64_t(view.selected.value);
        result["speed"] = controller.get_time_scale();
        Array territories;
        for (const auto& region : view.territories) {
            Dictionary t;
            t["id"] = int64_t(region.id.value);
            t["name"] = String::utf8(region.name.c_str());
            t["x"] = region.x; t["y"] = region.y;
            t["w"] = region.w; t["h"] = region.h;
            territories.push_back(t);
        }
        result["territories"] = territories;
        return result;
    }
    Dictionary command(const String& name, int64_t id = 0) {
        Dictionary result;
        bool success = true;
        String reason = "ok";
        if (name == "select") {
            auto r = world->execute({polyec::CommandType::SelectTerritory, polyec::EntityId{uint64_t(id)}});
            success = r.success;
            reason = String::utf8(r.reason.c_str());
        } else if (name == "pause") {
            controller.set_paused(true);
            anchor = Clock::now();
        } else if (name == "resume" || name == "toggle_pause") {
            bool resume = name == "resume" || controller.is_paused();
            if (resume && !active) {
                success = false; reason = "inactive";
            } else {
                controller.set_paused(!resume);
                anchor = Clock::now();
            }
        } else {
            success = false; reason = "unknown_command";
        }
        result["success"] = success;
        result["reason"] = reason;
        return result;
    }
    void tick() {
        auto now = Clock::now();
        double elapsed = std::chrono::duration<double>(now - anchor).count();
        anchor = now;
        if (active) controller.update(elapsed);
    }
    void set_active(bool value) {
        active = value;
        if (!active) controller.set_paused(true);
        anchor = Clock::now();
    }
    bool set_speed(double value) {
        bool accepted = controller.set_time_scale(value);
        if (accepted) anchor = Clock::now();
        return accepted;
    }
};

void initialize_polyec(ModuleInitializationLevel level) {
    if (level == MODULE_INITIALIZATION_LEVEL_SCENE) ClassDB::register_class<PolyEcSession>();
}
void uninitialize_polyec(ModuleInitializationLevel) {}
extern "C" {
GDExtensionBool GDE_EXPORT polyec_library_init(GDExtensionInterfaceGetProcAddress get_proc_address,
        GDExtensionClassLibraryPtr library, GDExtensionInitialization* initialization) {
    GDExtensionBinding::InitObject init(get_proc_address, library, initialization);
    init.register_initializer(initialize_polyec);
    init.register_terminator(uninitialize_polyec);
    init.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
}
