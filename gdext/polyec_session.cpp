#include "polyec/sim.hpp"
#include "polyec/campaign.hpp"
#include "polyec/atlas_cartography.hpp"
#include "polyec/atlas_settlements.hpp"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <chrono>
#include <algorithm>
#include <limits>
#include <set>
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
    std::unique_ptr<polyec::Campaign> campaign;
    std::string campaign_error;
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
        ClassDB::bind_method(D_METHOD("capability_catalog"), &PolyEcSession::capability_catalog);
        ClassDB::bind_method(D_METHOD("atlas_map_data"), &PolyEcSession::atlas_map_data);
        ClassDB::bind_method(D_METHOD("rl_available"), &PolyEcSession::rl_available);
        ClassDB::bind_method(D_METHOD("m4_start", "seed", "population"), &PolyEcSession::m4_start, DEFVAL(int64_t(37)), DEFVAL(int64_t(10000)));
        ClassDB::bind_method(D_METHOD("m4_snapshot"), &PolyEcSession::m4_snapshot);
        ClassDB::bind_method(D_METHOD("m4_fingerprint"), &PolyEcSession::m4_fingerprint);
        ClassDB::bind_method(D_METHOD("m4_preview", "from", "to", "quantity"), &PolyEcSession::m4_preview);
        ClassDB::bind_method(D_METHOD("m4_step", "days"), &PolyEcSession::m4_step);
        ClassDB::bind_method(D_METHOD("m4_step_one_day"), &PolyEcSession::m4_step_one_day);
        ClassDB::bind_method(D_METHOD("m4_pause"), &PolyEcSession::m4_pause);
        ClassDB::bind_method(D_METHOD("m4_resume"), &PolyEcSession::m4_resume);
        ClassDB::bind_method(D_METHOD("m4_paused"), &PolyEcSession::m4_paused);
        ClassDB::bind_method(D_METHOD("m4_pause_revision"), &PolyEcSession::m4_pause_revision);
        ClassDB::bind_method(D_METHOD("m4_command", "action", "target", "quantity", "sequence", "revision"), &PolyEcSession::m4_command);
        ClassDB::bind_method(D_METHOD("m4_save", "path", "restore"), &PolyEcSession::m4_save);
#ifdef POLYEC_AI_ENABLED
        ClassDB::bind_method(D_METHOD("rl_load", "path", "sha256", "seed"), &PolyEcSession::rl_load);
        ClassDB::bind_method(D_METHOD("rl_step"), &PolyEcSession::rl_step);
        ClassDB::bind_method(D_METHOD("rl_snapshot"), &PolyEcSession::rl_snapshot);
        ClassDB::bind_method(D_METHOD("realm_schema"), &PolyEcSession::realm_schema);
        ClassDB::bind_method(D_METHOD("realm_model_contract"), &PolyEcSession::realm_model_contract);
        ClassDB::bind_method(D_METHOD("realm_load", "paths", "hashes", "seed", "population"), &PolyEcSession::realm_load);
        ClassDB::bind_method(D_METHOD("realm_atlas_manual_start", "seed", "population"), &PolyEcSession::realm_atlas_manual_start,DEFVAL(int64_t(20261038)),DEFVAL(int64_t(512)));
        ClassDB::bind_method(D_METHOD("realm_step", "days"), &PolyEcSession::realm_step);
        ClassDB::bind_method(D_METHOD("realm_step_one_day"), &PolyEcSession::realm_step_one_day);
        ClassDB::bind_method(D_METHOD("realm_pause"), &PolyEcSession::realm_pause);
        ClassDB::bind_method(D_METHOD("realm_resume"), &PolyEcSession::realm_resume);
        ClassDB::bind_method(D_METHOD("realm_paused"), &PolyEcSession::realm_paused);
        ClassDB::bind_method(D_METHOD("realm_pause_revision"), &PolyEcSession::realm_pause_revision);
        ClassDB::bind_method(D_METHOD("realm_snapshot", "actor"), &PolyEcSession::realm_snapshot,DEFVAL(int64_t(1)));
        ClassDB::bind_method(D_METHOD("realm_command", "actor", "action", "expected"), &PolyEcSession::realm_command,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_military_combat_consent", "actor", "consent", "expected"), &PolyEcSession::realm_military_combat_consent,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_military_raid", "partner_id", "force_units", "expected"), &PolyEcSession::realm_military_raid,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_military_battle", "actor", "raid_id", "retreat", "defenders", "expected"), &PolyEcSession::realm_military_battle,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_military_counterattack", "actor", "region", "retreat", "defenders", "expected"), &PolyEcSession::realm_military_counterattack,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_foreign_trade_suspension", "actor", "partner_id", "suspended", "expected"), &PolyEcSession::realm_foreign_trade_suspension,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_foreign_trade_offer", "actor", "partner_id", "operation", "good", "quantity", "expected"), &PolyEcSession::realm_foreign_trade_offer,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_title_transfer", "actor", "holding_id", "recipient", "expected"), &PolyEcSession::realm_title_transfer,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_land_use_rights"), &PolyEcSession::realm_land_use_rights);
        ClassDB::bind_method(D_METHOD("realm_land_use_grant", "actor", "holding_id", "grantee", "expected"), &PolyEcSession::realm_land_use_grant,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_land_use_revoke", "actor", "holding_id", "expected"), &PolyEcSession::realm_land_use_revoke,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_private_residence_build", "actor", "holding_id", "expected"), &PolyEcSession::realm_private_residence_build,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_private_housing_command", "actor", "residence_id", "operation", "tenant_household", "rent_per_period", "expected"), &PolyEcSession::realm_private_housing_command,DEFVAL(int64_t(0)),DEFVAL(int64_t(0)),DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_market_order", "actor", "operation", "order_id", "region", "good", "quantity", "maximum_unit_price", "expected"), &PolyEcSession::realm_market_order,DEFVAL(String()));
        ClassDB::bind_method(D_METHOD("realm_debug_worksite_cycle", "actor", "expected"), &PolyEcSession::realm_debug_worksite_cycle);
        ClassDB::bind_method(D_METHOD("realm_draft", "card", "reroll"), &PolyEcSession::realm_draft);
        ClassDB::bind_method(D_METHOD("realm_save", "path", "restore"), &PolyEcSession::realm_save);
        ClassDB::bind_method(D_METHOD("realm_route", "actor", "from", "to", "open"), &PolyEcSession::realm_route);
        ClassDB::bind_method(D_METHOD("realm_route_preview", "actor", "destination"), &PolyEcSession::realm_route_preview);
        ClassDB::bind_method(D_METHOD("realm_national_transport_route", "actor", "route_index", "open", "expected"), &PolyEcSession::realm_national_transport_route);
#endif
    }
public:
    Dictionary atlas_map_data() const {
        Array regions;
        for (const auto& polygon : polyec::kAtlasMapPolygons) {
            const auto& territory = polyec::kWorldAtlas[polygon.territory_id];
            Dictionary row;
            row["id"] = int(polygon.territory_id);
            row["name"] = String::utf8(territory.name.data());
            row["group"] = String::utf8(territory.region_group.data());
            row["specialty"] = String::utf8(territory.specialty.data());
            row["description"] = String::utf8(territory.advantages_and_constraints.data());
            row["royal"] = territory.royal_domain;
            row["anchor"] = Vector2(polygon.label_anchor.x, polygon.label_anchor.y);
            PackedVector2Array points;
            for (const auto& point : polygon.vertices) points.push_back(Vector2(point.x, point.y));
            row["polygon"] = points;
            regions.push_back(row);
        }
        Array links;
        for (const auto& link : polyec::kWorldAtlasLinks) {
            Dictionary row;
            row["from"] = int(link.from); row["to"] = int(link.to);
            row["corridor"] = int(link.corridor); row["mode"] = int(link.mode);
            row["direction"] = int(link.direction);
            links.push_back(row);
        }
        Array settlements;
        for (const auto& site : polyec::kAtlasUrbanSites) {
            const char* kind = "Trung tâm vùng";
            switch (site.kind) {
                case polyec::AtlasUrbanSiteKind::RegionalCenter: kind = "Trung tâm vùng"; break;
                case polyec::AtlasUrbanSiteKind::Port: kind = "Cảng"; break;
                case polyec::AtlasUrbanSiteKind::RoyalCenter: kind = "Trung tâm triều đình"; break;
                case polyec::AtlasUrbanSiteKind::RoyalGranary: kind = "Khu kho triều đình"; break;
                case polyec::AtlasUrbanSiteKind::RoyalWorks: kind = "Khu xưởng triều đình"; break;
            }
            Dictionary row;
            row["id"] = int(site.id);
            row["territory_id"] = int(site.territory_id);
            row["name"] = String::utf8(site.name.data());
            row["kind"] = String::utf8(kind);
            row["position"] = Vector2(site.position.x, site.position.y);
            settlements.push_back(row);
        }
        Dictionary result;
        result["version"] = "atlas-cartography-v1";
        result["site_version"] = "atlas-urban-sites-v1";
        result["unit"] = "map_unit";
        result["regions"] = regions;
        result["links"] = links;
        result["settlements"] = settlements;
        return result;
    }
    Array capability_catalog() const {
        Array result;
        for(const auto& definition:polyec::kCapabilityCatalog){
            Dictionary row;row["id"]=int(definition.id);row["key"]=String::utf8(definition.key);
            row["unit"]=String::utf8(definition.unit);row["minimum"]=definition.minimum;
            row["maximum"]=definition.maximum;row["owner"]=String::utf8(definition.owner);
            row["scenario_configurable"]=definition.scenario_configurable;
            row["always_active"]=definition.always_active;row["disabled_value"]=definition.disabled_value;
            result.push_back(row);
        }
        return result;
    }
    bool m4_start(int64_t seed=37,int64_t population=10000){try{auto next=std::make_unique<polyec::Campaign>(uint64_t(seed),int(population));next->pause();campaign=std::move(next);campaign_error.clear();return true;}catch(const std::exception&e){campaign_error=e.what();return false;}}
    bool m4_step(int64_t days){if(!active||!campaign||days<1||days>30){campaign_error=!active?"Cửa sổ không hoạt động":(!campaign?"Chưa mở sổ M4":"Khoảng ngày phải trong 1–30");return false;}try{campaign->advance(int(days));campaign_error.clear();return true;}catch(const std::exception&e){campaign_error=e.what();return false;}}
    bool m4_step_one_day(){if(!active||!campaign){campaign_error=!active?"Cửa sổ không hoạt động":"Chưa mở sổ M4";return false;}try{campaign->step_one_day();campaign_error.clear();return true;}catch(const std::exception&e){campaign_error=e.what();return false;}}
    bool m4_pause(){if(!campaign){campaign_error="Chưa mở sổ M4";return false;}try{campaign->pause();campaign_error.clear();return true;}catch(const std::exception&e){campaign_error=e.what();return false;}}
    bool m4_resume(){if(!active||!campaign){campaign_error=!active?"Cửa sổ không hoạt động":"Chưa mở sổ M4";return false;}try{campaign->resume();campaign_error.clear();return true;}catch(const std::exception&e){campaign_error=e.what();return false;}}
    bool m4_paused()const{return campaign&&campaign->paused();}
    uint64_t m4_pause_revision()const{return campaign?campaign->pause_revision():0;}
    bool m4_command(int64_t action,int64_t target,int64_t quantity,int64_t sequence,int64_t revision){
        if(!active||!campaign){campaign_error="Cửa sổ không hoạt động";return false;}
        if(action<0||action>6||target<0||target>6||sequence<1||revision<0||uint64_t(revision)!=campaign->revision()){campaign_error="Lệnh hoặc hồ sơ đã cũ; hãy xem lại";return false;}
        auto r=campaign->execute({1,uint64_t(sequence),polyec::CampaignAction(action),int(target),quantity});campaign_error=r.success?"":r.reason;return r.success;
    }
    bool m4_save(const String&path,bool restore){if(!campaign){campaign_error="Chưa mở sổ M4";return false;}return restore?campaign->load(path.utf8().get_data(),campaign_error,true):campaign->save(path.utf8().get_data(),campaign_error);}
    String m4_fingerprint()const{return campaign?String::num_uint64(campaign->fingerprint()):String();}
    Dictionary m4_preview(int64_t from,int64_t to,int64_t quantity)const{
        Dictionary d;if(!campaign||from<0||from>6||to<0||to>6)return d;auto p=campaign->preview(int(from),int(to),quantity);d["reachable"]=p.reachable;d["days"]=p.days;d["fee"]=p.fee;Array path;for(auto r:p.regions)path.push_back(r);d["path"]=path;return d;
    }
    Dictionary m4_snapshot()const{
        Dictionary d;d["loaded"]=bool(campaign);d["error"]=String::utf8(campaign_error.c_str());if(!campaign)return d;auto v=campaign->view();
        d["day"]=v.realm.day;d["population"]=int64_t(v.realm.persons);d["alive"]=int64_t(v.realm.alive);d["money"]=v.realm.money;d["crown"]=v.crown;d["invariant"]=v.invariant;d["revision"]=int64_t(v.revision);d["sequence"]=int64_t(v.player_sequence);d["workshop_cash"]=v.workshop_cash;d["workshop_workers"]=v.workshop_workers;d["cloth"]=v.cloth;d["inspecting_until"]=v.inspecting_until;d["paused"]=v.paused;d["pause_revision"]=int64_t(v.pause_revision);
        Array regions;for(int i=0;i<int(v.regions.size());++i){auto&g=v.regions[i];auto&p=v.realm.provinces[i];Dictionary r;r["id"]=i;r["name"]=String::utf8(g.name.c_str());r["terrain"]=String::utf8(g.terrain.c_str());r["center"]=Vector2(g.center.x,g.center.y);PackedVector2Array boundary;for(auto point:g.boundary)boundary.push_back(Vector2(point.x,point.y));r["polygon"]=boundary;r["food"]=p.stock[0];r["cover"]=int(p.stock[0]*7/std::max(1,p.residents));r["residents"]=p.residents;r["treasury"]=p.treasury;r["price"]=p.price[0];r["unrest"]=p.unrest;r["road"]=p.road;r["harvest"]=g.harvest;regions.push_back(r);}d["regions"]=regions;
        Array routes;for(auto&r:v.routes){Dictionary x;x["from"]=r.from;x["to"]=r.to;x["days"]=r.days;x["capacity"]=r.capacity;x["terrain"]=String::utf8(r.terrain.c_str());x["open"]=r.open;x["claimed"]=r.claimed;x["works_due"]=r.works_due;routes.push_back(x);}d["routes"]=routes;
        Array cargo;for(auto&c:v.cargo){Dictionary x;x["id"]=int64_t(c.id);x["cause"]=int64_t(c.cause);x["from"]=c.from;x["to"]=c.to;x["quantity"]=c.quantity;x["cost"]=c.cost;x["depart"]=c.depart;x["arrives"]=c.arrives;x["delivered"]=c.delivered;Array path,starts,ends,arrivals;for(int r:c.path)path.push_back(r);for(int t:c.leg_starts)starts.push_back(t);for(int t:c.leg_ends)ends.push_back(t);for(int t:c.leg_arrivals)arrivals.push_back(t);x["path"]=path;x["starts"]=starts;x["ends"]=ends;x["arrivals"]=arrivals;cargo.push_back(x);}d["cargo"]=cargo;
        Array cases;for(auto&k:v.cases){Dictionary x;x["id"]=int64_t(k.id);x["cause"]=int64_t(k.cause);x["region"]=k.region;x["title"]=String::utf8(k.title.c_str());x["opened"]=k.opened;x["closed"]=k.closed;x["evidence"]=k.evidence;cases.push_back(x);}d["cases"]=cases;
        Array journal;for(auto&f:v.journal){Dictionary x;x["id"]=int64_t(f.id);x["parent"]=int64_t(f.parent);x["day"]=f.day;x["region"]=f.region;x["kind"]=f.kind;x["amount"]=f.amount;journal.push_back(x);}d["journal"]=journal;
        int64_t debt=0;bool overdue=false;for(auto&l:v.loans)if(l.debtor==3){debt+=l.principal+l.interest-l.paid;overdue|=l.paid<l.principal+l.interest&&l.due<=v.realm.day;}d["debt"]=debt;d["overdue"]=overdue;return d;
    }
    bool rl_available() const {
#ifdef POLYEC_AI_ENABLED
        return true;
#else
        return false;
#endif
    }
#ifdef POLYEC_AI_ENABLED
    String realm_schema()const{return polyec::realm_schema;}
    String realm_model_contract()const{const polyec::Realm world(1,1,128);const auto contract=world.model_contract();return String::utf8(contract.c_str());}
    bool realm_load(const Array&paths,const Array&hashes,int64_t seed,int64_t population){
        controller.set_paused(true);anchor=Clock::now();
        if(paths.size()!=3||hashes.size()!=3||population<128||population>100000){realm.stop("Invalid model bundle/profile");return false;}
        std::array<std::string,3> names;std::string bundle;
        for(int i=0;i<3;++i){String path=paths[i],hash=hashes[i];if(hash.length()!=64||FileAccess::get_sha256(path)!=hash){realm.stop("Realm model checksum mismatch");return false;}names[i]=path.utf8().get_data();bundle+=hash.utf8().get_data();}
        if(!realm.load_models(names,bundle,uint64_t(seed),int(population)))return false;
        if(!realm.pause()){realm.stop(realm.error().empty()?"Realm interactive pause failed":realm.error());return false;}
        return true;
    }
    bool realm_atlas_manual_start(int64_t seed=20261024,int64_t population=512){
        controller.set_paused(true);anchor=Clock::now();
        if(seed<0||population<128||population>100000){realm.stop("Atlas manual-test seed/population is outside valid bounds");return false;}
        return realm.load_atlas_manual_test(uint64_t(seed),int(population));
    }
    bool realm_step(int64_t days){if(!active||days<1||days>30)return false;return realm.advance(int(days));}
    bool realm_step_one_day(){if(!active)return false;return realm.step_one_day();}
    bool realm_pause(){return realm.pause();}
    bool realm_resume(){if(!active)return false;return realm.resume();}
    bool realm_paused()const{return realm.paused();}
    uint64_t realm_pause_revision()const{return realm.pause_revision();}
    bool realm_command(int64_t actor,int64_t action,const String&expected=String()){if(!active)return false;return realm.command(uint64_t(actor),int(action),expected.utf8().get_data());}
    bool realm_military_combat_consent(int64_t actor,bool consent,const String&expected=String()){if(!active)return false;return realm.military_combat_consent(uint64_t(actor),consent,expected.utf8().get_data());}
    Dictionary realm_military_raid(int partnerId,int forceUnits,const String&expected=String()){
        Dictionary result;const bool success=active&&realm.military_raid(partnerId,forceUnits,expected.utf8().get_data());
        result["success"]=success;result["error"]=String::utf8(realm.error().c_str());
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    Dictionary realm_military_battle(int64_t actor,int64_t raidId,bool retreat,const PackedInt64Array&defenderIds,const String&expected=String()){
        std::vector<uint64_t> defenders;defenders.reserve(size_t(defenderIds.size()));
        for(int64_t id:defenderIds)if(id>0)defenders.push_back(uint64_t(id));else defenders.push_back(0);
        Dictionary result;const bool success=active&&actor>0&&raidId>0&&
            realm.military_battle(uint64_t(actor),uint64_t(raidId),retreat,defenders,expected.utf8().get_data());
        result["success"]=success;result["error"]=String::utf8(realm.error().c_str());
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    Dictionary realm_military_counterattack(int64_t actor,int region,bool retreat,const PackedInt64Array&defenderIds,const String&expected=String()){
        std::vector<uint64_t> defenders;defenders.reserve(size_t(defenderIds.size()));
        for(int64_t id:defenderIds)if(id>0)defenders.push_back(uint64_t(id));else defenders.push_back(0);
        Dictionary result;const bool success=active&&actor>0&&region>=0&&region<int(polyec::kAtlasTerritoryCount)&&
            realm.military_counterattack(uint64_t(actor),region,retreat,defenders,expected.utf8().get_data());
        result["success"]=success;result["error"]=String::utf8(realm.error().c_str());
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    bool realm_foreign_trade_suspension(int64_t actor,int partnerId,bool suspended,const String&expected=String()){if(!active)return false;return realm.foreign_trade_suspension(uint64_t(actor),partnerId,suspended,expected.utf8().get_data());}
    bool realm_foreign_trade_offer(int64_t actor,int partnerId,int operation,int good,int64_t quantity,const String&expected=String()){if(!active||operation<0||operation>=2*polyec::kForeignTradePartnerCount)return false;return realm.foreign_trade_offer(uint64_t(actor),partnerId,static_cast<polyec::ForeignTradeOperation>(operation),good,quantity,expected.utf8().get_data());}
    Dictionary realm_title_transfer(int64_t actor,int64_t holding,int64_t recipient,const String&expected=String()){
        Dictionary result;const bool success=active&&actor>0&&holding>0&&recipient>0&&
            realm.land_title_transfer(uint64_t(actor),uint64_t(holding),uint64_t(recipient),expected.utf8().get_data());
        result["success"]=success;result["error"]=String::utf8(realm.error().c_str());
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    Dictionary realm_private_residence_build(int64_t actor,int64_t holding,const String&expected=String()){
        Dictionary result;const bool success=active&&actor>0&&holding>0&&
            realm.private_residence_build(uint64_t(actor),uint64_t(holding),expected.utf8().get_data());
        result["success"]=success;result["error"]=String::utf8(realm.error().c_str());
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    Dictionary realm_private_housing_command(int64_t actor,int64_t residenceId,int operation,
            int64_t tenantHousehold,int64_t rentPerPeriod,const String&expected=String()){
        Dictionary result;bool success=false;
        if(!active)result["error"]="Cửa sổ không hoạt động";
        else if(actor<1||residenceId<1||operation<0||operation>int(polyec::PrivateHousingOperation::EndLease)||
                tenantHousehold<0||rentPerPeriod<0)result["error"]="Thông tin hợp đồng nhà tư không hợp lệ";
        else success=realm.private_housing_command(uint64_t(actor),uint64_t(residenceId),
            static_cast<polyec::PrivateHousingOperation>(operation),uint64_t(tenantHousehold),rentPerPeriod,
            expected.utf8().get_data());
        if(!result.has("error"))result["error"]=String::utf8(realm.error().c_str());
        result["success"]=success;
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    Array realm_land_use_rights()const{
        Array result;if(const auto* world=realm.world())for(const auto&right:world->land_use_rights()){
            Dictionary row;row["id"]=int64_t(right.id);row["holding_id"]=int64_t(right.holding_id);
            row["grantee"]=int64_t(right.grantee);row["grantor"]=int64_t(right.grantor);result.push_back(row);
        }return result;
    }
    Dictionary realm_land_use_grant(int64_t actor,int64_t holding,int64_t grantee,const String&expected=String()){
        Dictionary result;const bool success=active&&actor>0&&holding>0&&grantee>0&&
            realm.land_use_grant(uint64_t(actor),uint64_t(holding),uint64_t(grantee),expected.utf8().get_data());
        result["success"]=success;result["error"]=String::utf8(realm.error().c_str());
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    Dictionary realm_land_use_revoke(int64_t actor,int64_t holding,const String&expected=String()){
        Dictionary result;const bool success=active&&actor>0&&holding>0&&
            realm.land_use_revoke(uint64_t(actor),uint64_t(holding),expected.utf8().get_data());
        result["success"]=success;result["error"]=String::utf8(realm.error().c_str());
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    Dictionary realm_market_order(int64_t actor,int64_t operation,int64_t orderId,int64_t region,int64_t good,
                                  int64_t quantity,int64_t maximumUnitPrice,const String&expected=String()){
        Dictionary result;uint64_t resultOrderId=0;bool success=false;
        if(!active)result["error"]="Cửa sổ không hoạt động";
        else if(actor<1||operation<0||operation>1||orderId<0||region<0||region>=27||good<0||good>=polyec::kGoodIdCount)
            result["error"]="Thông tin đơn mua không hợp lệ";
        else{
            success=realm.market_order(uint64_t(actor),static_cast<polyec::MarketOrderOperation>(operation),
                uint64_t(orderId),int(region),int(good),quantity,maximumUnitPrice,resultOrderId,
                expected.utf8().get_data());
            result["error"]=String::utf8(realm.error().c_str());
        }
        result["success"]=success;result["order_id"]=int64_t(resultOrderId);
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    Dictionary realm_debug_worksite_cycle(int64_t actor,const String&expected){
        Dictionary result;uint64_t site=0;int64_t actorCash=0,siteCash=0,fiber=0,cloth=0,paid=0;
        const bool success=active&&realm.debug_worksite_cycle(uint64_t(actor),expected.utf8().get_data(),site,actorCash,siteCash,fiber,cloth,paid);
        result["success"]=success;result["error"]=String::utf8(realm.error().c_str());result["establishment_id"]=int64_t(site);
        result["actor_cash"]=actorCash;result["site_cash"]=siteCash;result["fiber"]=fiber;result["cloth"]=cloth;result["paid_units"]=paid;
        if(success)result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    bool realm_route(int64_t actor,int64_t from,int64_t to,bool open){return active&&realm.route(uint64_t(actor),int(from),int(to),open);}
    Dictionary realm_national_transport_route(int64_t actor,int64_t routeIndex,bool open,const String&expected=String()){
        Dictionary result;
        bool success=false;
        if(routeIndex<0||routeIndex>std::numeric_limits<uint8_t>::max())
            result["error"]="Invalid national-route index";
        else if(!active)
            result["error"]="Cửa sổ không hoạt động";
        else
            success=realm.national_transport_route(actor>0?uint64_t(actor):0,int(routeIndex),open,expected.utf8().get_data());
        result["success"]=success;
        if(!result.has("error"))result["error"]=String::utf8(realm.error().c_str());
        if(success&&realm.world())result["fingerprint"]=String::num_uint64(realm.world()->fingerprint());
        return result;
    }
    Dictionary realm_route_preview(int64_t actor,int64_t destination) const {
        Dictionary result;
        const auto preview = realm.route_preview(actor > 0 ? uint64_t(actor) : 0, int(destination));
        result["supported"] = preview.supported;
        result["reachable"] = preview.reachable;
        if (!preview.supported) {
            result["message"] = "Xem trước tuyến Atlas cần một thế giới Atlas27 đang mở";
            return result;
        }
        if (!preview.reachable) {
            result["message"] = "Hiện không có tuyến mở tới vùng đã chọn";
            return result;
        }
        result["route_days"] = preview.route_days;
        result["person_days"] = preview.person_days;
        result["route_fee"] = preview.route_fee;
        result["migration_fee"] = preview.migration_fee;
        result["travel_available"] = preview.travel_available;
        result["migration_available"] = preview.migration_available;
        Array regions;
        for (const auto region : preview.regions) {
            Dictionary point;
            point["id"] = int(region);
            point["name"] = String::utf8(polyec::kWorldAtlas[region].name.data());
            regions.push_back(point);
        }
        result["regions"] = regions;
        Array legs;
        for (const auto& route : preview.legs) {
            Dictionary leg;
            leg["from"] = int(route.from);
            leg["to"] = int(route.to);
            leg["from_name"] = String::utf8(polyec::kWorldAtlas[route.from].name.data());
            leg["to_name"] = String::utf8(polyec::kWorldAtlas[route.to].name.data());
            leg["days"] = route.travel_days;
            leg["fee"] = route.fee_per_unit;
            leg["open"] = route.open;
            if (route.atlas_link < polyec::kWorldAtlasLinks.size()) {
                const auto& link = polyec::kWorldAtlasLinks[route.atlas_link];
                switch (link.corridor) {
                    case polyec::AtlasCorridor::GreatRiver: leg["corridor"] = "Đại Giang"; break;
                    case polyec::AtlasCorridor::RoadImprovement: leg["corridor"] = "Đường cải thiện"; break;
                    case polyec::AtlasCorridor::Coastal: leg["corridor"] = "Duyên hải"; break;
                    case polyec::AtlasCorridor::East: leg["corridor"] = "Hành lang phía Đông"; break;
                    case polyec::AtlasCorridor::North: leg["corridor"] = "Hành lang phía Bắc"; break;
                    case polyec::AtlasCorridor::West: leg["corridor"] = "Hành lang phía Tây"; break;
                    case polyec::AtlasCorridor::RoyalDomains: leg["corridor"] = "Đường miền triều đình"; break;
                }
                switch (link.mode) {
                    case polyec::AtlasRouteMode::River: leg["mode"] = "đường sông"; break;
                    case polyec::AtlasRouteMode::Road: leg["mode"] = "đường bộ"; break;
                    case polyec::AtlasRouteMode::CoastalShipping: leg["mode"] = "đường biển"; break;
                    case polyec::AtlasRouteMode::Unspecified: leg["mode"] = "chưa phân loại"; break;
                }
            }
            legs.push_back(leg);
        }
        result["legs"] = legs;
        return result;
    }
    bool realm_draft(int64_t card,bool reroll){return reroll?realm.reroll(int(card)):realm.choose(int(card));}
    bool realm_save(const String&path,bool restore){return restore?realm.restore_interactive(path.utf8().get_data()):realm.save(path.utf8().get_data());}
    Dictionary realm_snapshot(int64_t actor=1)const{
        Dictionary d;d["loaded"]=realm.loaded();d["atlas_manual_test"]=realm.atlas_manual_test();d["error"]=String::utf8(realm.error().c_str());
        auto*w=realm.world();if(!w)return d;if(actor<1||actor>w->count())actor=1;auto v=w->view(uint64_t(actor));
        d["day"]=v.day;d["persons"]=int64_t(v.persons);d["alive"]=int64_t(v.alive);d["children"]=int64_t(v.children);d["decisions"]=int64_t(v.decisions);d["money"]=v.money;d["invariant"]=w->invariant();d["fingerprint"]=String::num_uint64(w->fingerprint());d["inferences"]=int64_t(realm.inference_count());d["inference_ms"]=realm.inference_ms();
        Array cards;for(int c:v.draft.cards)cards.push_back(c);d["cards"]=cards;d["chosen"]=v.draft.chosen;d["tier"]=v.draft.tier;d["rerolled"]=v.draft.rerolled;
        Array regions;for(int region=0;region<int(v.provinces.size());++region){auto&p=v.provinces[size_t(region)];Dictionary r;r["treasury"]=p.treasury;r["urban_fund"]=w->municipal_funds()[size_t(region)];r["firm"]=p.firm;r["unrest"]=p.unrest;r["road"]=p.road;r["jobs"]=p.jobs;r["workers"]=p.employed;r["school"]=p.school;r["students"]=p.school_used;r["tax"]=p.tax;r["military_ration_stock"]=w->military_ration_stock(region);r["military_supplementary_ration_stock"]=w->military_ration_stock(region,int(polyec::GoodId::SupplementaryFood));Array prices,stock;for(int good=0;good<polyec::kGoodIdCount;++good){prices.push_back(p.price[size_t(good)]);stock.push_back(p.stock[size_t(good)]);}r["prices"]=prices;r["stock"]=stock;regions.push_back(r);}d["regions"]=regions;
        Array establishments;for(const auto&site:w->establishments()){Dictionary row;row["id"]=int64_t(site.id);row["region"]=site.region;row["owner_kind"]=int(site.owner_kind);row["owner_id"]=int64_t(site.owner_id);row["industry"]=int(site.industry);row["cash"]=site.cash;row["capacity"]=site.capacity;row["source_good"]=site.source_good;row["source_units"]=site.source_units;Array stock;for(const auto quantity:site.stock)stock.push_back(quantity);row["stock"]=stock;establishments.push_back(row);}d["establishments"]=establishments;
        const auto&viewer=w->person(actor);
        Array apprenticeships;for(const auto&course:w->apprenticeships()){
            const auto&sites=w->establishments();
            const auto site=std::find_if(sites.begin(),sites.end(),[&](const polyec::Establishment&value){return value.id==course.establishment_id;});
            if(!viewer.sovereign&&viewer.region!=w->person(course.mentor).region&&actor!=int64_t(course.mentor)&&actor!=int64_t(course.apprentice))continue;
            Dictionary row;row["id"]=int64_t(course.id);row["mentor"]=int64_t(course.mentor);row["apprentice"]=int64_t(course.apprentice);
            row["establishment_id"]=int64_t(course.establishment_id);row["region"]=site==sites.end()?-1:site->region;
            row["started_day"]=course.started_day;row["due_day"]=course.due_day;row["practice_days"]=course.practice_days;
            row["status"]=int(course.status);row["tuition"]=course.tuition;apprenticeships.push_back(row);
        }d["apprenticeships"]=apprenticeships;
        auto&p=w->person(actor);Dictionary person;person["id"]=actor;person["age"]=p.age_days/365;person["role"]=int(p.family);person["sovereign"]=p.sovereign;person["cash"]=p.cash;person["region"]=p.region;Array personalInventory;for(const auto quantity:p.inventory)personalInventory.push_back(quantity);person["inventory"]=personalInventory;person["health"]=p.health;person["nutrition"]=p.nutrition;person["sleep_debt"]=p.body_state.sleep_debt;person["exertion"]=p.body_state.exertion;person["authority"]=p.taxing_authority;person["office"]=p.office_appointment.office;person["candidate"]=p.candidate;person["job_kind"]=p.work_assignment.job_kind;person["skill"]=p.skill;person["goal"]=p.goal;person["alive"]=p.alive;person["debt"]=w->debt(actor);person["decisions"]=int64_t(p.decisions);person["household"]=int64_t(p.household_membership.household_id);const auto&service=p.military_service;Dictionary serviceView;serviceView["active"]=service.active;serviceView["combat_consent"]=service.combat_consent;serviceView["commander"]=int64_t(service.commander);serviceView["start_day"]=service.start_day;serviceView["end_day"]=service.end_day;serviceView["source_region"]=service.source_region;serviceView["wage_per_day"]=service.wage_per_day;serviceView["escrow"]=service.escrow;serviceView["paid_total"]=service.paid_total;person["military_service"]=serviceView;Array stats;Dictionary namedStats;Array activeCapabilities;for(size_t i=0;i<polyec::kLegacyCapabilities.size();++i){const int value=p.stats.get(polyec::kLegacyCapabilities[i]);stats.push_back(value);namedStats[String::utf8(polyec::kLegacyCapabilityKeys[i])]=value;if(w->capability_active(polyec::kLegacyCapabilities[i]))activeCapabilities.push_back(String::utf8(polyec::kLegacyCapabilityKeys[i]));}person["stats"]=stats;person["legacy_capabilities"]=namedStats;person["active_capabilities"]=activeCapabilities;Dictionary capabilities;for(size_t i=0;i<polyec::kCapabilityCount;++i)capabilities[String::utf8(polyec::kCapabilityKeys[i])]=w->capability_value(actor,static_cast<polyec::CapabilityId>(i));Array capabilitySchema;for(const auto capabilityId:w->capability_schema())capabilitySchema.push_back(String::utf8(polyec::kCapabilityKeys[static_cast<size_t>(capabilityId)]));person["capabilities"]=capabilities;person["capability_schema"]=capabilitySchema;d["person"]=person;
        if(p.sovereign&&w->regions()==int(polyec::kAtlasTerritoryCount)-1){
            Array nationalRoutes;
            const auto&routes=w->national_transport_routes();
            for(size_t routeIndex=0;routeIndex<routes.size();++routeIndex){
                const auto&route=routes[routeIndex];
                Dictionary item;item["index"]=int(routeIndex);item["from"]=int(route.from);item["to"]=int(route.to);
                item["from_name"]=String::utf8(polyec::kWorldAtlas[route.from].name.data());
                item["to_name"]=String::utf8(polyec::kWorldAtlas[route.to].name.data());
                item["open"]=route.open;item["travel_days"]=route.travel_days;item["fee_per_unit"]=route.fee_per_unit;
                item["occupied_endpoint"]=w->region_military_occupied(route.from)||w->region_military_occupied(route.to);
                if(route.atlas_link<polyec::kWorldAtlasLinks.size()){
                    const auto&link=polyec::kWorldAtlasLinks[route.atlas_link];
                    switch(link.corridor){
                        case polyec::AtlasCorridor::GreatRiver:item["corridor"]="Đại Giang";break;
                        case polyec::AtlasCorridor::RoadImprovement:item["corridor"]="Đường cải thiện";break;
                        case polyec::AtlasCorridor::Coastal:item["corridor"]="Duyên hải";break;
                        case polyec::AtlasCorridor::East:item["corridor"]="Hành lang phía Đông";break;
                        case polyec::AtlasCorridor::North:item["corridor"]="Hành lang phía Bắc";break;
                        case polyec::AtlasCorridor::West:item["corridor"]="Hành lang phía Tây";break;
                        case polyec::AtlasCorridor::RoyalDomains:item["corridor"]="Đường miền triều đình";break;
                    }
                }
                nationalRoutes.push_back(item);
            }
            d["national_routes"]=nationalRoutes;
        }
        Array census;const auto councils=w->municipal_councils();for(const auto& row:w->urban_census())if(p.sovereign||row.territory_id==p.region){Dictionary item;item["site_id"]=int(row.site_id);item["territory_id"]=int(row.territory_id);item["site_name"]=String::utf8(polyec::kAtlasUrbanSites[row.site_id].name.data());item["urban_residents"]=int64_t(row.urban_residents);item["rural_residents"]=int64_t(row.rural_residents);item["unknown_residence"]=int64_t(row.unknown_residence);item["housing_capacity"]=row.housing_capacity;item["unhoused_residents"]=row.unhoused_residents;item["available_beds"]=row.available_beds;if(row.site_id<councils.size()){const auto&council=councils[row.site_id];item["council_representative_id"]=int64_t(council.representative);item["council_nominee_id"]=int64_t(council.nominee);item["council_nomination_day"]=council.nomination_day;item["council_term_end_day"]=council.term_end_day;}census.push_back(item);}d["urban_census"]=census;
        const auto holdings=w->land_holdings();Array visibleHoldings;Array tenantHouseholds;
        for(const auto&land:holdings)if(land.title_holder==uint64_t(actor)||p.sovereign||land.region==p.region){
            Dictionary item;item["id"]=int64_t(land.id);item["region"]=land.region;
            item["owner_id"]=int64_t(land.title_holder);item["owned_by_actor"]=land.title_holder==uint64_t(actor);
            visibleHoldings.push_back(item);
        }
        d["land_holdings"]=visibleHoldings;
        std::set<uint64_t> seenHouseholds;
        for(uint64_t id=1;id<=w->count();++id){const auto&candidate=w->person(id);
            if(!candidate.alive||candidate.region!=p.region||candidate.residence_kind!=polyec::ResidenceKind::Urban||
               candidate.urban_site_id>=polyec::kAtlasTerritoryCount||candidate.age_days<18*365)continue;
            const auto household=candidate.household_membership.household_id;
            if(!household||w->household_authority(household)!=id||!seenHouseholds.insert(household).second)continue;
            Dictionary item;item["household_id"]=int64_t(household);item["authority_id"]=int64_t(id);
            item["region"]=candidate.region;item["site_id"]=candidate.urban_site_id;
            tenantHouseholds.push_back(item);
        }
        d["private_housing_households"]=tenantHouseholds;
        Array residences;for(const auto&home:w->private_residences()){
            auto holding=std::find_if(holdings.begin(),holdings.end(),[&](const polyec::LandHolding&value){return value.id==home.holding_id;});
            const bool householdParty=home.tenant_household==p.household_membership.household_id||home.offered_household==p.household_membership.household_id;
            const bool titleHolder=holding!=holdings.end()&&holding->title_holder==actor;
            if(!p.sovereign&&home.region!=p.region&&!titleHolder&&!householdParty)continue;
            Dictionary item;item["id"]=int64_t(home.id);item["holding_id"]=int64_t(home.holding_id);item["region"]=home.region;
            item["owner_id"]=int64_t(holding==holdings.end()?0:holding->title_holder);item["completed"]=home.completed;
            item["construction_due_day"]=home.construction_due_day;item["tenant_household"]=int64_t(home.tenant_household);
            item["offered_household"]=int64_t(home.offered_household);item["offered_rent"]=home.offered_rent;
            item["offer_expires_day"]=home.offer_expires_day;item["rent_per_period"]=home.rent_per_period;
            item["next_rent_day"]=home.next_rent_day;item["lease_end_day"]=home.lease_end_day;residences.push_back(item);
        }d["private_residences"]=residences;
        if(v.provinces.size()>8&&(p.sovereign||(p.family==polyec::Family::Economic&&p.region==8))){
            Dictionary trade;Array partners;int partnerId=0;for(const auto&partner:w->foreign_trade_partners()){
                Dictionary ledger;ledger["partner_id"]=partnerId;
                ledger["cash"]=partner.money;ledger["escrow"]=partner.escrow;
                ledger["dry_food"]=partner.goods[size_t(polyec::GoodId::DryFood)];
                ledger["salt"]=partner.goods[size_t(polyec::GoodId::Salt)];
                ledger["salt_capacity"]=partner.salt_capacity;ledger["trade_suspended"]=partner.trade_suspended;
                ledger["decision_sequence"]=String::num_uint64(partner.decision_sequence);
                ledger["offer_cash_escrow"]=partner.offer_cash_escrow;
                ledger["completed_import_orders"]=partner.completed_import_orders;
                ledger["completed_export_orders"]=partner.completed_export_orders;
                ledger["imported_units"]=partner.imported_units;ledger["exported_units"]=partner.exported_units;
                ledger["import_gross"]=partner.import_gross;ledger["export_gross"]=partner.export_gross;
                partners.push_back(ledger);++partnerId;
            }trade["partners"]=partners;
            auto route_info=[&](uint8_t from,uint8_t to){
                Dictionary info;
                for(const auto&route:w->national_transport_routes()){
                    if(route.atlas_link>=polyec::kWorldAtlasLinks.size())continue;
                    const auto&link=polyec::kWorldAtlasLinks[route.atlas_link];
                    if(link.from!=8||link.to!=9||link.mode!=polyec::AtlasRouteMode::CoastalShipping||
                       route.from!=from||route.to!=to)continue;
                    info["open"]=route.open;info["travel_days"]=route.travel_days;
                    info["capacity_per_day"]=route.capacity_per_day;info["fee_per_unit"]=route.fee_per_unit;
                    return info;
                }
                info["open"]=false;info["travel_days"]=0;info["capacity_per_day"]=0;info["fee_per_unit"]=0;
                return info;
            };
            trade["import_route"]=route_info(9,8);trade["export_route"]=route_info(8,9);
            Array orders;for(const auto&order:w->foreign_trade_orders()){Dictionary item;item["partner_id"]=order.partner_id;item["operation"]=int(order.operation);item["quantity"]=order.quantity;item["arrival_day"]=order.arrival_day;item["good"]=order.good;orders.push_back(item);}
            trade["orders"]=orders;
            Array offers;const auto allOffers=w->foreign_trade_offers();const size_t firstOffer=allOffers.size()>24?allOffers.size()-24:0;
            for(size_t offerIndex=firstOffer;offerIndex<allOffers.size();++offerIndex){const auto&offer=allOffers[offerIndex];Dictionary row;row["id"]=String::num_uint64(offer.id);row["owner"]=int64_t(offer.owner);row["partner_id"]=offer.partner_id;row["operation"]=int(offer.operation);row["good"]=offer.good;row["quantity"]=offer.quantity;row["created_day"]=offer.created_day;row["due_day"]=offer.due_day;row["status"]=int(offer.status);row["held_cash"]=offer.held_cash;row["held_good"]=offer.held_good;row["order_id"]=String::num_uint64(offer.order_id);offers.push_back(row);}
            trade["offers"]=offers;d["foreign_trade"]=trade;
        }
        Array actions;auto cs=w->candidates(actor);auto ms=w->mask(actor);for(int a=0;a<polyec::realm_actions;++a)if(a==0||cs[a].kind!=polyec::Intent::Hold){Dictionary c;c["index"]=a;c["legal"]=ms[a]>.5f;c["name"]=String::utf8(polyec::Realm::action_name(cs[a]).c_str());c["target"]=cs[a].target;c["good"]=cs[a].good;c["amount"]=cs[a].amount;c["term"]=cs[a].term;actions.push_back(c);}d["actions"]=actions;d["timeline_contract"]=w->timeline_contract();const auto modelContract=w->model_contract();d["model_contract"]=String::utf8(modelContract.c_str());
        Array facts;int start=std::max(0,int(v.facts.size())-40);for(int i=start;i<int(v.facts.size());++i){auto&f=v.facts[i];Dictionary x;x["id"]=int64_t(f.id);x["parent"]=int64_t(f.parent);x["actor"]=int64_t(f.actor);x["target"]=int64_t(f.target);x["day"]=f.day;x["region"]=f.region;x["kind"]=f.kind;x["amount"]=f.amount;facts.push_back(x);}d["facts"]=facts;
        Array knowledge;for(const auto&k:v.knowledge){Dictionary x;x["observer"]=int64_t(k.observer);x["fact"]=int64_t(k.fact);x["arrives"]=k.arrives;x["source_investigation_id"]=int64_t(k.source_investigation_id);x["kind"]=int(k.kind);x["confidence_basis_points"]=k.confidence_basis_points;knowledge.push_back(x);}d["knowledge"]=knowledge;
        Array marketOrders;for(const auto&order:w->market_orders()){Dictionary row;row["id"]=int64_t(order.id);row["owner"]=int64_t(order.owner);row["region"]=order.region;row["good"]=order.good;row["quantity"]=order.quantity;row["maximum_unit_price"]=order.maximum_unit_price;row["personal_escrow"]=order.personal_escrow;row["household_escrow"]=order.household_escrow;row["organization_escrow"]=order.organization_escrow;marketOrders.push_back(row);}d["market_orders"]=marketOrders;d["market_escrow"]=w->market_escrow();
        Array transportShipments;for(const auto&shipment:w->transport_shipments()){Dictionary row;row["id"]=int64_t(shipment.id);row["owner"]=int64_t(shipment.owner);row["owner_kind"]=int(shipment.owner_kind);row["source_region"]=shipment.source_region;row["created_day"]=shipment.created_day;row["destination"]=shipment.destination;row["good"]=shipment.good;row["service_goal"]=int(shipment.service_goal);row["quantity"]=shipment.quantity;row["delivered"]=shipment.delivered;row["canceled"]=shipment.canceled;row["fee_escrow"]=shipment.fee_escrow;transportShipments.push_back(row);}d["transport_shipments"]=transportShipments;
        Array militaryUnits;for(const auto&unit:w->military_unit_rosters()){Dictionary row;row["id"]=String::num_uint64(unit.id);row["source_region"]=unit.source_region;row["commander"]=int64_t(unit.commander);Array members;for(auto member:unit.members)members.push_back(int64_t(member));row["members"]=members;Array mobilizedMembers;for(auto member:unit.mobilized_members)mobilizedMembers.push_back(int64_t(member));row["mobilized_members"]=mobilizedMembers;Array combatConsented;for(auto member:unit.combat_consented_members)combatConsented.push_back(int64_t(member));row["combat_consented_members"]=combatConsented;Array battleReady;for(auto member:unit.battle_ready_members)battleReady.push_back(int64_t(member));row["battle_ready_members"]=battleReady;row["mobilized_until_day"]=unit.mobilized_until_day;row["payroll_escrow"]=unit.payroll_escrow;row["paid_total"]=unit.paid_total;militaryUnits.push_back(row);}d["military_unit_rosters"]=militaryUnits;
        Array militaryRaids;for(const auto&raid:w->military_raids()){
            if(!p.sovereign&&p.region!=raid.target_region)continue;
            Dictionary row;row["id"]=String::num_uint64(raid.id);row["partner_id"]=raid.partner_id;
            row["target_region"]=raid.target_region;row["target_region_name"]=String::utf8(polyec::kAtlasUrbanSites[size_t(raid.target_region)].name.data());row["created_day"]=raid.created_day;
            row["resolved_day"]=raid.resolved_day;row["force_units"]=raid.force_units;
            row["defense_score"]=raid.defense_score;row["attacker_losses"]=raid.attacker_losses;
            row["defender_casualties"]=raid.defender_casualties;row["salt_looted"]=raid.salt_looted;
            row["status"]=int(raid.status);Array defenderIds;
            for(const auto id:raid.defenders)defenderIds.push_back(int64_t(id));
            row["defenders"]=defenderIds;Array suppliedDefenderIds;
            for(const auto id:raid.supplied_defenders)suppliedDefenderIds.push_back(int64_t(id));
            row["supplied_defenders"]=suppliedDefenderIds;militaryRaids.push_back(row);
        }d["military_raids"]=militaryRaids;
        std::set<int> engagementRegions;
        if(p.sovereign){
            for(const auto&raid:w->military_raids())if(raid.status==polyec::MilitaryRaidStatus::Pending)engagementRegions.insert(raid.target_region);
            const auto currentOccupations=w->military_occupations();
            for(size_t region=0;region<currentOccupations.size();++region)if(currentOccupations[region].occupied())engagementRegions.insert(int(region));
        }
        Dictionary battleCandidatesByRegion;
        for(const int region:engagementRegions){
            Array battleCandidates;
            if(p.sovereign)for(const auto id:w->military_battle_candidates(region)){
                Dictionary row;const auto&volunteer=w->person(id);row["id"]=int64_t(id);row["health"]=volunteer.health;
                row["military_capability"]=w->capability_value(id,polyec::LegacyCapability::Military);
                row["has_weapon"]=volunteer.inventory[size_t(polyec::GoodId::Weapon)]>0;
                row["has_military_supply"]=volunteer.inventory[size_t(polyec::GoodId::MilitarySupply)]>0;
                battleCandidates.push_back(row);
            }
            battleCandidatesByRegion[region]=battleCandidates;
        }
        d["military_battle_candidates_by_region"]=battleCandidatesByRegion;
        Array occupations;const auto currentOccupations=w->military_occupations();
        const auto occupationPartners=w->foreign_trade_partners();
        for(size_t region=0;region<currentOccupations.size();++region){
            const auto&occupation=currentOccupations[region];if(!occupation.occupied())continue;
            if(!p.sovereign&&p.region!=int(region))continue;
            Dictionary item;item["region"]=int(region);item["partner_id"]=occupation.partner_id;
            item["region_name"]=String::utf8(polyec::kAtlasUrbanSites[region].name.data());
            item["garrison_units"]=occupation.garrison_units;
            if(p.sovereign)item["partner_dry_food"]=occupationPartners[size_t(occupation.partner_id)].goods[size_t(polyec::GoodId::DryFood)];
            occupations.push_back(item);
        }
        d["military_occupations"]=occupations;
        Array militaryObligations;for(const auto&item:w->military_obligations()){const bool is_member=std::find(item.member_ids.begin(),item.member_ids.end(),actor)!=item.member_ids.end();if(item.obligor!=actor&&item.recipient!=actor&&!is_member&&!(p.family==polyec::Family::Governance&&(p.region==item.source_region||p.region==item.destination_region)))continue;Dictionary row;row["id"]=int64_t(item.id);row["obligor"]=int64_t(item.obligor);row["recipient"]=int64_t(item.recipient);row["source_region"]=item.source_region;row["destination_region"]=item.destination_region;row["created_day"]=item.created_day;row["first_due_day"]=item.first_due_day;row["due_day"]=item.due_day;row["completed_day"]=item.completed_day;row["extensions"]=item.extensions;row["required_members"]=item.required_members;row["delivered_at_first_deadline"]=item.delivered_at_first_deadline;row["shortage_at_first_deadline"]=item.shortage_at_first_deadline;row["delivered_at_last_deadline"]=item.delivered_at_last_deadline;row["shortage_at_last_deadline"]=item.shortage_at_last_deadline;row["first_deadline_checked"]=item.first_deadline_checked;row["due_checked"]=item.due_checked;Array members;for(auto id:item.member_ids)members.push_back(int64_t(id));row["members"]=members;Array delivered;for(auto id:item.delivered_member_ids)delivered.push_back(int64_t(id));row["delivered_members"]=delivered;militaryObligations.push_back(row);}d["military_obligations"]=militaryObligations;
        Array militarySupplyObligations;for(const auto&item:w->military_supply_obligations()){
            if(item.obligor!=actor&&item.recipient!=actor&&
               !(p.family==polyec::Family::Governance&&(p.region==item.source_region||p.region==item.destination_region)))continue;
            Dictionary row;row["id"]=int64_t(item.id);row["personnel_obligation_id"]=int64_t(item.personnel_obligation_id);
            row["obligor"]=int64_t(item.obligor);row["recipient"]=int64_t(item.recipient);
            row["source_region"]=item.source_region;row["destination_region"]=item.destination_region;
            row["created_day"]=item.created_day;row["first_due_day"]=item.first_due_day;row["due_day"]=item.due_day;
            row["completed_day"]=item.completed_day;row["extensions"]=item.extensions;row["required"]=item.required;
            row["delivered"]=item.delivered;row["delivered_at_first_deadline"]=item.delivered_at_first_deadline;
            row["shortage_at_first_deadline"]=item.shortage_at_first_deadline;
            row["delivered_at_last_deadline"]=item.delivered_at_last_deadline;
            row["shortage_at_last_deadline"]=item.shortage_at_last_deadline;
            row["first_deadline_checked"]=item.first_deadline_checked;row["due_checked"]=item.due_checked;
            militarySupplyObligations.push_back(row);
        }d["military_supply_obligations"]=militarySupplyObligations;
        Array offers;for(auto&o:w->offers(actor)){Dictionary offer;offer["id"]=int64_t(o.id);offer["from"]=int64_t(o.from);offer["to"]=int64_t(o.to);offer["kind"]=o.kind;offer["amount"]=o.amount;offer["service_term"]=o.service_term;offer["escrow"]=o.escrow;offer["due"]=o.due;Array roster;for(auto id:o.roster_members)roster.push_back(int64_t(id));offer["roster_members"]=roster;offers.push_back(offer);}d["offers"]=offers;auto funds=w->royal_funds();d["credit_pool"]=funds[0];d["restricted_fund"]=funds[1];d["royal_reference"]=w->royal_reference();d["schema"]=String(polyec::realm_schema);d["loans"]=int(w->loans().size());d["projects"]=int(w->projects().size());d["edges"]=int(w->relations().size());return d;
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
        for (const auto& polygon : polyec::kAtlasMapPolygons) {
            const auto& territory = polyec::kWorldAtlas[polygon.territory_id];
            double min_x = polygon.vertices[0].x, max_x = min_x;
            double min_y = polygon.vertices[0].y, max_y = min_y;
            for (const auto& point : polygon.vertices) {
                min_x = std::min(min_x, point.x); max_x = std::max(max_x, point.x);
                min_y = std::min(min_y, point.y); max_y = std::max(max_y, point.y);
            }
            // GameWorld reserves entity ID 0; its shell view stores Atlas ID + 1
            // while the public map continues to expose the canonical Atlas ID.
            world->add_territory({polyec::EntityId{uint64_t(polygon.territory_id) + 1},
                std::string(territory.name), min_x, min_y, max_x - min_x, max_y - min_y});
        }
    }
    Dictionary snapshot() const {
        auto view = world->player_view();
        Dictionary result;
        result["day"] = view.date.day;
        result["paused"] = view.paused;
        result["selected"] = int64_t(view.selected.value == 0 ? 0 : view.selected.value - 1);
        result["speed"] = controller.get_time_scale();
        Array territories;
        for (const auto& region : view.territories) {
            Dictionary t;
            t["id"] = int64_t(region.id.value - 1);
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
            if (id < 0 || id >= int64_t(polyec::kAtlasTerritoryCount)) {
                success = false;
                reason = "Atlas territory ID out of range";
            } else {
                auto r = world->execute({polyec::CommandType::SelectTerritory, polyec::EntityId{uint64_t(id) + 1}});
                success = r.success;
                reason = String::utf8(r.reason.c_str());
            }
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
