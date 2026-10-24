#include "polyec/realm.hpp"
#include "polyec/person_section_codec.hpp"
#include "polyec/realm_primary_header_codec.hpp"
#include "polyec/province_section_codec.hpp"
#include "polyec/edge_section_codec.hpp"
#include "polyec/loan_section_codec.hpp"
#include "polyec/project_section_codec.hpp"
#include "polyec/fact_section_codec.hpp"
#include "polyec/knowledge_section_codec.hpp"
#include "polyec/proposal_section_codec.hpp"
#include "polyec/schedule_section_codec.hpp"
#include "polyec/clock_section_codec.hpp"
#include "polyec/establishment_section_codec.hpp"
#include "polyec/work_credit_section_codec.hpp"
#include "polyec/body_state_section_codec.hpp"
#include "polyec/household_fund_section_codec.hpp"
#include "polyec/land_title_section_codec.hpp"
#include "polyec/land_use_section_codec.hpp"
#include "polyec/estate_section_codec.hpp"
#include "polyec/parentage_section_codec.hpp"
#include "polyec/market_section_codec.hpp"
#include "polyec/national_transport_section_codec.hpp"
#include "polyec/transport_shipment_section_codec.hpp"
#include "polyec/military_ration_depot_section_codec.hpp"
#include "polyec/municipal_fund_section_codec.hpp"
#include "polyec/foreign_trade_section_codec.hpp"
#include "polyec/military_obligation_section_codec.hpp"
#include "polyec/military_supply_obligation_section_codec.hpp"
#include "polyec/military_raid_section_codec.hpp"
#include "polyec/urban_residence_section_codec.hpp"
#include "polyec/urban_housing_section_codec.hpp"
#include "polyec/municipal_council_section_codec.hpp"
#include "polyec/private_housing_section_codec.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <set>
#include <tuple>
#include <utility>

namespace polyec {
namespace {
uint64_t hash_text(const std::string& s) {
    uint64_t h = 1469598103934665603ull;
    for (unsigned char c : s) { h ^= c; h *= 1099511628211ull; }
    return h;
}
}

// Text format uses explicit scalar fields, not compiler-dependent struct bytes.
std::string Realm::serialize(const std::string&bundle)const{
    std::ostringstream s;s<<"POLYEC_REALM_73 "<<std::quoted(bundle)<<' ';
    detail::RealmPrimaryHeader header{};
    header.day=day_;header.regions=regions_;header.seed=seed_;header.next_id=next_id_;header.next_fact=next_fact_;
    header.opening_money=opening_money_;header.carrier=carrier_;header.estate=estate_;header.endowment=endowment_;
    header.draft=draft_;header.opening=opening_;header.produced=produced_;header.consumed=consumed_;
    header.routes=routes_;header.organization_funds=organization_funds_;header.rng=rng_;
    detail::write_realm_primary_header(s,header);
    detail::write_person_section(s,people_);
    detail::write_province_section(s,provinces_);
    detail::write_edge_section(s,edges_);
    detail::write_loan_section(s,loans_);
    detail::write_project_section(s,projects_);
    detail::write_fact_section(s,facts_);
    detail::write_knowledge_section(s,knowledge_);
    detail::write_proposal_section(s, proposals_);
    write_court(s);
    detail::write_clock_section(s, paused_, pause_revision_);
    write_schedule_section(s, schedule_book_);
    detail::write_establishment_section(s, establishments_);
    write_gameplay_config(s,config_);
    s<<" WORKPAY_1 "<<work_pay_remainders_.size()<<' ';
    for(const auto&[key,remainder]:work_pay_remainders_)s<<key.first<<' '<<key.second<<' '<<remainder<<' ';
    detail::write_work_credit_section(s,work_credits_);
    detail::write_body_state_section(s,people_);
    detail::write_household_fund_section(s,household_funds_);
    detail::write_land_title_section(s,land_holdings_);
    detail::write_land_use_section(s,land_use_rights_);
    detail::write_estate_section(s,legacy_estate_unassigned_,estate_cases_);
    detail::write_parentage_section(s,edges_);
    detail::write_market_section(s,market_escrow_,market_settled_day_,market_processed_month_,market_orders_,market_monthly_);
    detail::write_national_transport_section(s,national_transport_routes_);
    detail::write_transport_shipment_section(s,next_transport_lot_id_,transport_escrow_,transport_shipments_,transport_cargo_lots_);
    s<<" CAPABILITY_SCHEMA_1 "<<capability_mask_<<' ';
    s<<" PERSONALITY_1 "<<people_.size()<<' ';
    for(const auto& person:people_)for(const int value:person.personality.values)s<<value<<' ';
    s<<" MILITARY_SERVICE_3 "<<people_.size()<<' ';
    for(const auto& person:people_){const auto& service=person.military_service;
        s<<service.active<<' '<<service.combat_consent<<' '<<service.commander<<' '<<service.start_day<<' '<<service.end_day<<' '
         <<service.source_region<<' '<<service.mobilized_until_day<<' '<<service.wage_per_day<<' '<<service.escrow<<' '<<service.paid_total<<' ';}
    detail::write_military_ration_depot_section_v2(s,military_ration_depots_,military_supplementary_ration_depots_);
    detail::write_municipal_fund_section(s,municipal_funds_,municipal_last_funded_period_);
    detail::write_foreign_trade_section(s,foreign_trade_partners_,foreign_trade_orders_,foreign_trade_offers_);
    detail::write_military_obligation_section(s,military_obligations_,proposals_);
    detail::write_military_supply_obligation_section(s,military_supply_obligations_);
    detail::write_military_raid_section(s,military_raids_);
    detail::write_urban_residence_section(s,people_);
    detail::write_urban_housing_section(s,urban_housing_capacity_);
    detail::write_municipal_council_section(s,municipal_councils_);
    detail::write_private_housing_section(s,private_residences_);
    return s.str();
}
bool Realm::deserialize(const std::string&data,const std::string&bundle,std::string&error,const std::array<int64_t,kGoodIdCount>& bonded_goods){try{
    if(data.size()>256*1024*1024)throw std::runtime_error("Save size limit");
    auto stagedWorld=std::make_unique<Realm>(1,1,128);Realm& w=*stagedWorld;w.land_holdings_.clear();w.land_use_rights_.clear();std::istringstream s(data);std::string magic,b;s>>magic>>std::quoted(b);
    int schema=0;
    if(magic=="POLYEC_REALM_73")schema=73;
    else if(magic=="POLYEC_REALM_72")schema=72;
    else if(magic=="POLYEC_REALM_71")schema=71;
    else if(magic=="POLYEC_REALM_70")schema=70;
    else if(magic=="POLYEC_REALM_69")schema=69;
    if(magic=="POLYEC_REALM_68")schema=68;
    if(magic=="POLYEC_REALM_67")schema=67;
    if(magic=="POLYEC_REALM_66")schema=66;
    if(magic=="POLYEC_REALM_65")schema=65;
    if(magic=="POLYEC_REALM_64")schema=64;
    if(magic=="POLYEC_REALM_63")schema=63;
    if(magic=="POLYEC_REALM_62")schema=62;
    if(magic=="POLYEC_REALM_61")schema=61;
    if(magic=="POLYEC_REALM_60")schema=60;
    if(magic=="POLYEC_REALM_59")schema=59;
    if(magic=="POLYEC_REALM_58")schema=58;
    if(magic=="POLYEC_REALM_57")schema=57;
    if(magic=="POLYEC_REALM_56")schema=56;
    if(magic=="POLYEC_REALM_55")schema=55;
    if(magic=="POLYEC_REALM_54")schema=54;
    if(magic=="POLYEC_REALM_53")schema=53;
    if(magic=="POLYEC_REALM_50")schema=50;
    if(magic=="POLYEC_REALM_52")schema=52;
    if(magic=="POLYEC_REALM_51")schema=51;
    if(magic=="POLYEC_REALM_49")schema=49;
    if(magic=="POLYEC_REALM_48")schema=48;
    if(magic=="POLYEC_REALM_47")schema=47;
    if(magic=="POLYEC_REALM_46")schema=46;
    if(magic=="POLYEC_REALM_45")schema=45;
    if(magic=="POLYEC_REALM_44")schema=44;else if(magic=="POLYEC_REALM_43")schema=43;else if(magic=="POLYEC_REALM_42")schema=42;else if(magic=="POLYEC_REALM_41")schema=41;else if(magic=="POLYEC_REALM_40")schema=40;else if(magic=="POLYEC_REALM_39")schema=39;else if(magic=="POLYEC_REALM_38")schema=38;else if(magic=="POLYEC_REALM_37")schema=37;else if(magic=="POLYEC_REALM_36")schema=36;else if(magic=="POLYEC_REALM_35")schema=35;else if(magic=="POLYEC_REALM_34")schema=34;else if(magic=="POLYEC_REALM_33")schema=33;else if(magic=="POLYEC_REALM_32")schema=32;else if(magic=="POLYEC_REALM_31")schema=31;else if(magic=="POLYEC_REALM_30")schema=30;else if(magic=="POLYEC_REALM_29")schema=29;else if(magic=="POLYEC_REALM_28")schema=28;else if(magic=="POLYEC_REALM_27")schema=27;else if(magic=="POLYEC_REALM_26")schema=26;else if(magic=="POLYEC_REALM_25")schema=25;else if(magic=="POLYEC_REALM_24")schema=24;else if(magic=="POLYEC_REALM_23")schema=23;else if(magic=="POLYEC_REALM_22")schema=22;else if(magic=="POLYEC_REALM_21")schema=21;else if(magic=="POLYEC_REALM_20")schema=20;else if(magic=="POLYEC_REALM_19")schema=19;else if(magic=="POLYEC_REALM_18")schema=18;else if(magic=="POLYEC_REALM_17")schema=17;else if(magic=="POLYEC_REALM_16")schema=16;else if(magic=="POLYEC_REALM_15")schema=15;else if(magic=="POLYEC_REALM_14")schema=14;else if(magic=="POLYEC_REALM_13")schema=13;else if(magic=="POLYEC_REALM_12")schema=12;else if(magic=="POLYEC_REALM_11")schema=11;else if(magic=="POLYEC_REALM_10")schema=10;else if(magic=="POLYEC_REALM_9")schema=9;else if(magic=="POLYEC_REALM_8")schema=8;else if(magic=="POLYEC_REALM_7")schema=7;
    const bool legacy=schema==7;
    if(schema==0||b!=bundle)throw std::runtime_error("Save schema/model bundle mismatch");
    detail::RealmPrimaryHeader header{};
    if(!detail::read_realm_primary_header_initial(s,header))throw std::runtime_error("Save geography");
    if(header.regions!=1&&header.regions!=2&&header.regions!=6&&header.regions!=int(kAtlasTerritoryCount)-1)throw std::runtime_error("Save geography");
    const bool tail_ok=detail::read_realm_primary_header_tail(s,header,schema);
    if(tail_ok){
        w.day_=header.day;w.regions_=header.regions;w.seed_=header.seed;w.next_id_=header.next_id;w.next_fact_=header.next_fact;
        w.military_ration_depots_.assign(size_t(w.regions_+1),0);
        w.military_supplementary_ration_depots_.assign(size_t(w.regions_+1),0);
        w.municipal_funds_.assign(size_t(w.regions_+1),0);
        w.municipal_last_funded_period_.assign(size_t(w.regions_+1),-1);
        w.opening_money_=header.opening_money;w.carrier_=header.carrier;w.estate_=header.estate;w.endowment_=header.endowment;
        w.draft_=header.draft;w.opening_=header.opening;w.produced_=header.produced;w.consumed_=header.consumed;
        w.routes_=header.routes;w.organization_funds_=header.organization_funds;w.rng_=header.rng;
        if(schema<27&&w.regions_==int(kAtlasTerritoryCount)-1)
            w.national_transport_routes_=NationalTransportManager::default_routes();
    }
    detail::read_person_section(s,w.people_,w.regions_,schema);
    // Older writers could leave a deceased person below zero after the final
    // unmet-food penalty. Normalize that terminal value while the load is
    // still staged so every successfully loaded Realm keeps health in [0,100].
    for(auto& person:w.people_)if(person.health<0)person.health=0;
    auto size=[&](size_t max){size_t n=0;s>>n;if(!s||n>max)throw std::runtime_error("Save count limit");return n;};
    w.reindex_households();
    detail::read_province_section(s,w.provinces_,schema);
    detail::read_edge_section(s,w.edges_,schema);
    detail::read_loan_section(s,w.loans_);
    detail::read_project_section(s,w.projects_);
    detail::read_fact_section(s,w.facts_);
    detail::read_knowledge_section(s,w.knowledge_,schema);for(auto&k:w.knowledge_){auto&known=w.knowledge_index_[k.observer];auto previous=known.find(k.fact);if(previous==known.end()||previous->second>k.arrives)known[k.fact]=k.arrives;}w.rebuild_secret_known_counts();
    detail::read_proposal_section(s, w.proposals_);
    if (!w.read_court(s,schema)) throw std::runtime_error("Save court fields");
    detail::read_clock_section(s, w.paused_, w.pause_revision_);
    if(schema>=15){
        const auto config_at=data.rfind(schema>=36?" CONFIG_3 ":schema>=26?" CONFIG_2 ":" CONFIG_1 ");
        if(config_at==std::string::npos||config_at<static_cast<size_t>(s.tellg()))throw std::runtime_error("Save config section missing");
        std::istringstream tail(data.substr(config_at));w.config_=read_gameplay_config(tail);
        std::string pay_tag;size_t count=0;tail>>pay_tag>>count;
        if(!tail||pay_tag!="WORKPAY_1"||count>size_t(w.count())*(size_t(w.regions_)+1))throw std::runtime_error("Save work pay remainder header");
        WorkRegionKey previous{};
        for(size_t row=0;row<count;++row){
            uint64_t actor=0;int region=-1;int64_t remainder=0;tail>>actor>>region>>remainder;
            const WorkRegionKey key{actor,region};
            if(!tail||actor==0||actor>uint64_t(w.count())||region<0||region>w.regions_||remainder<=0||remainder>=w.config_.wage_per_unit()||(row&&key<=previous))
                throw std::runtime_error("Save work pay remainder fields");
            previous=key;w.work_pay_remainders_.emplace(key,remainder);
        }
        if(schema>=17)w.work_credits_=detail::read_work_credit_section(tail,w.people_.size(),w.regions_);
        if(schema>=18)detail::read_body_state_section(tail,w.people_,w.regions_,schema);
        if(schema>=20)w.household_funds_=detail::read_household_fund_section(tail,w.people_.size());
        if(schema>=21)w.land_holdings_=detail::read_land_title_section(tail,w.people_.size(),w.regions_);
        if(schema>=22)w.land_use_rights_=detail::read_land_use_section(tail,w.people_.size(),w.land_holdings_,w.next_id_);
        if(schema>=24)w.estate_cases_=detail::read_estate_section(tail,w.people_.size(),w.regions_,w.next_id_,w.legacy_estate_unassigned_,schema);
        else w.legacy_estate_unassigned_=w.estate_;
        if(schema>=25)detail::read_parentage_section(tail,w.edges_);
        if(schema>=26)detail::read_market_section(tail,w.people_,w.regions_,w.next_id_,w.day_,
            w.market_escrow_,w.market_settled_day_,w.market_processed_month_,w.market_orders_,w.market_monthly_,w.config_,schema);
        if(schema>=27)w.national_transport_routes_=detail::read_national_transport_section(tail,w.regions_);
        if(schema>=28)detail::read_transport_shipment_section(tail,w.next_transport_lot_id_,w.transport_escrow_,w.transport_shipments_,w.transport_cargo_lots_,schema);
        if(schema>=32){std::string capabilityTag;uint32_t mask=0;tail>>capabilityTag>>mask;if(!tail||capabilityTag!="CAPABILITY_SCHEMA_1"||mask==0||(mask&~kDefaultCapabilityMask)!=0)throw std::runtime_error("Save capability schema");w.capability_mask_=mask;}
        if(schema>=35){std::string personalityTag;size_t count=0;tail>>personalityTag>>count;if(!tail||personalityTag!="PERSONALITY_1"||count!=w.people_.size())throw std::runtime_error("Save personality header");for(auto& person:w.people_)for(auto& value:person.personality.values){tail>>value;if(!tail||value<kPersonalityMinimum||value>kPersonalityMaximum)throw std::runtime_error("Save personality value");}}
        if(schema>=45){
            std::string serviceTag;size_t count=0;tail>>serviceTag>>count;
            const char* expected_service_tag=schema>=56?"MILITARY_SERVICE_3":schema>=48?"MILITARY_SERVICE_2":"MILITARY_SERVICE_1";
            if(!tail||serviceTag!=expected_service_tag||count!=w.people_.size())throw std::runtime_error("Save military service header");
            for(auto& person:w.people_){auto& service=person.military_service;
                tail>>service.active;
                if(schema>=56)tail>>service.combat_consent;
                tail>>service.commander>>service.start_day>>service.end_day>>service.source_region;
                if(schema>=48)tail>>service.mobilized_until_day;
                tail>>service.wage_per_day>>service.escrow>>service.paid_total;
                if(!tail)throw std::runtime_error("Save military service fields");
            }
        }
        if(schema>=53)detail::read_military_ration_depot_section_v2(tail,w.military_ration_depots_,w.military_supplementary_ration_depots_,size_t(w.regions_+1));
        else if(schema>=46)detail::read_military_ration_depot_section(tail,w.military_ration_depots_,size_t(w.regions_+1));
        if(schema>=49)detail::read_municipal_fund_section(tail,w.municipal_funds_,w.municipal_last_funded_period_,size_t(w.regions_+1),w.day_);
        if(schema>=52)detail::read_foreign_trade_section(tail,w.foreign_trade_partners_,w.foreign_trade_orders_,w.foreign_trade_offers_,w.people_.size(),w.regions_,w.day_,w.next_id_);
        else if(schema>=50)detail::read_foreign_trade_section_v1(tail,w.foreign_trade_partners_[0],w.foreign_trade_orders_,w.people_.size(),w.regions_,w.day_);
        if(schema>=51)detail::read_military_obligation_section(tail,w.military_obligations_,w.proposals_,w.people_.size(),w.regions_,w.day_,w.next_id_);
        if(schema>=54)detail::read_military_supply_obligation_section(tail,w.military_supply_obligations_,w.military_obligations_,w.people_.size(),w.regions_,w.day_,w.next_id_);
        if(schema>=60)detail::read_military_raid_section(tail,w.military_raids_,w.people_.size(),w.regions_,w.day_,w.next_id_);
        if(schema>=61)detail::read_urban_residence_section(tail,w.people_,w.regions_);
        if(schema>=64)detail::read_urban_housing_section(tail,w.regions_,w.urban_housing_capacity_);
        else w.initialize_urban_housing_baseline();
        if(schema>=65)detail::read_municipal_council_section(tail,w.regions_,w.municipal_councils_);
        else if(w.regions_==int(kAtlasTerritoryCount)-1){
            w.municipal_councils_.resize(kAtlasTerritoryCount);
            for(size_t site=0;site<w.municipal_councils_.size();++site)w.municipal_councils_[site].site_id=uint8_t(site);
        }
        if(schema>=67)w.private_residences_=detail::read_private_housing_section(
            tail,w.people_.size(),w.regions_,w.day_,w.next_id_,w.land_holdings_);
        else w.private_residences_.clear();
        tail>>std::ws;if(!tail.eof())throw std::runtime_error("Save config trailing fields");
        const auto read_position=s.tellg();
        s.str(data.substr(0,config_at));s.clear();s.seekg(read_position);
    }
    // Schemas before 15 have no gameplay-config tail, so they skip the block
    // above entirely. Initialize the housing/council data introduced later
    // for those saves as well; otherwise the staged Realm keeps constructor
    // sized arrays from its temporary one-region world and fails invariants
    // after loading older geographies.
    if(schema<15){
        if(schema<64)w.initialize_urban_housing_baseline();
        if(schema<65&&w.regions_==int(kAtlasTerritoryCount)-1){
            w.municipal_councils_.resize(kAtlasTerritoryCount);
            for(size_t site=0;site<w.municipal_councils_.size();++site)
                w.municipal_councils_[site].site_id=uint8_t(site);
        }
    }
    if(schema<27&&w.regions_!=int(kAtlasTerritoryCount)-1)
        w.national_transport_routes_.clear();
    if(schema<26){
        auto legacy_config=w.config_.values();legacy_config[11]=0;
        w.config_=GameplayConfig::from_values(legacy_config);
        w.market_escrow_=0;w.market_orders_.clear();w.market_monthly_.assign(size_t(w.regions_)+1,{});
        w.market_settled_day_=w.day_;w.market_processed_month_=w.day_/30;
    }
    if(schema<25)detail::migrate_legacy_parentage(w.edges_,w.facts_);
    if(schema<15)w.legacy_estate_unassigned_=w.estate_;
    if(schema<50&&w.regions_==int(kAtlasTerritoryCount)-1){
        if(w.endowment_>std::numeric_limits<int64_t>::max()-kForeignTradeOpeningCash||
           w.opening_[size_t(GoodId::DryFood)]>std::numeric_limits<int64_t>::max()-kForeignTradeOpeningDryFood)
            throw std::runtime_error("Foreign trade migration endowment overflow");
        auto& partner=w.foreign_trade_partners_[0];
        partner.money=kForeignTradeOpeningCash;
        partner.goods[size_t(GoodId::DryFood)]=kForeignTradeOpeningDryFood;
        partner.salt_capacity=kForeignTradeSaltCapacity;
        w.endowment_+=kForeignTradeOpeningCash;
        w.opening_[size_t(GoodId::DryFood)]+=kForeignTradeOpeningDryFood;
    }
    if(schema<52&&w.regions_==int(kAtlasTerritoryCount)-1){
        for(size_t id=1;id<w.foreign_trade_partners_.size();++id){
            auto& partner=w.foreign_trade_partners_[id];
            if(w.endowment_>std::numeric_limits<int64_t>::max()-kForeignTradePartnerOpeningCash||
               w.opening_[size_t(GoodId::DryFood)]>std::numeric_limits<int64_t>::max()-kForeignTradePartnerOpeningDryFood)
                throw std::runtime_error("Foreign trade partner migration endowment overflow");
            partner.money=kForeignTradePartnerOpeningCash;
            partner.goods[size_t(GoodId::DryFood)]=kForeignTradePartnerOpeningDryFood;
            partner.salt_capacity=kForeignTradePartnerSaltCapacity;
            w.endowment_+=kForeignTradePartnerOpeningCash;
            w.opening_[size_t(GoodId::DryFood)]+=kForeignTradePartnerOpeningDryFood;
        }
    }
    if(!read_schedule_section(s, schema, w.people_.size(), w.regions_, w.day_, w.schedule_book_, error,w.config_.ticks_per_unit()))
        throw std::runtime_error(error);
    if(schema>=11&&!detail::read_establishment_section(s,w.people_.size(),w.regions_,schema,w.establishments_,error))
        throw std::runtime_error(error);
    if(schema<16){
        for(const auto& [actor,sessions]:w.schedule_book_.sessions)for(const auto& session:sessions){
            if(session.retired||session.kind!=ActivityKind::Work||session.establishment_id==0||session.end<=w.schedule_book_.cursor)continue;
            auto site=std::lower_bound(w.establishments_.begin(),w.establishments_.end(),session.establishment_id,
                [](const Establishment& e,uint64_t id){return e.id<id;});
            if(site==w.establishments_.end()||site->id!=session.establishment_id)continue; // invariant rejects bad references
            site->work_authorizations[actor].push_back({std::max(session.begin,w.schedule_book_.cursor),session.end});
        }
        for(auto& site:w.establishments_)for(auto& [worker,windows]:site.work_authorizations){
            (void)worker;
            std::sort(windows.begin(),windows.end(),[](const auto& a,const auto& b){return a.begin<b.begin;});
            size_t count=0;
            for(const auto window:windows){
                if(count&&window.begin<=windows[count-1].end)windows[count-1].end=std::max(windows[count-1].end,window.end);
                else windows[count++]=window;
            }
            windows.resize(count);
            site.authorization_window_count+=count;
        }
    }
    // Gate 4 production bookkeeping. Schema 14 records carry the processed-work
    // watermarks directly and are validated against the just-loaded site
    // attendance. Schema 13 and 12 saves contain no production fields, so the
    // migration baselines every site's processed units to the unprocessed
    // eligible units of its saved site attendance, preventing retroactive
    // production for pre-Gate-4 worlds.
    if(schema>=11){
        if(schema<14)for(auto& site:w.establishments_)site.last_production_day=w.day_;
        for(const auto&[key,ticks]:w.schedule_book_.work_present_ticks_by_establishment){
            const auto [actor,establishment]=key;(void)actor;
            auto site=std::lower_bound(w.establishments_.begin(),w.establishments_.end(),establishment,
                [](const Establishment& e,uint64_t id){return e.id<id;});
            if(site==w.establishments_.end()||site->id!=establishment)continue;
            const int64_t eligible=ticks/w.config_.ticks_per_unit();
            // Current saves are validated actor-by-actor by Realm::invariant.
            // Comparing all site watermarks with this actor's attendance would
            // reject valid coworkers who completed different numbers of units.
            if(schema<14&&eligible>0)site->processed_work_units.emplace(uint64_t(actor),eligible);
            if(site->last_production_day<0||site->last_production_day>w.day_)
                throw std::runtime_error("Save establishment production day");
        }
    }
    if(schema<29){
        for(const auto& site:w.establishments_)for(const auto&[actor,units]:site.processed_work_units){
            const int64_t per_unit=kAttentionUnitsPerEffortTick*w.config_.ticks_per_unit();
            if(units<=0||units>std::numeric_limits<int64_t>::max()/per_unit)
                throw std::runtime_error("Save legacy production attention baseline");
            w.schedule_book_.work_effort_attention_by_establishment[{actor,site.id}]=units*per_unit;
        }
    }
    if(schema<34){
        for(const auto&[key,attention]:w.schedule_book_.work_effort_attention_by_establishment){
            if(attention<=0||attention>std::numeric_limits<int64_t>::max()/200)
                throw std::runtime_error("Save legacy productivity attention baseline");
            w.schedule_book_.work_productivity_attention_numerator_by_establishment[key]=attention*200;
        }
    }
    if(schema<30){
        std::map<uint64_t,int64_t> effortAttentionByActor;
        for(const auto&[key,attention]:w.schedule_book_.work_effort_attention_by_establishment){
            int64_t& total=effortAttentionByActor[key.first];
            if(attention<0||attention>std::numeric_limits<int64_t>::max()-total)
                throw std::runtime_error("Save body effort attention baseline overflow");
            total+=attention;
        }
        for(const auto&[actor,total]:effortAttentionByActor){
            if(actor==0||actor>w.people_.size())throw std::runtime_error("Save body effort attention actor");
            w.people_[size_t(actor-1)].body_state.exertion_effort_attention_processed=total;
        }
    }
    w.rebuild_personal_establishment_index();
    w.rebuild_kinship_indices();
    if(schema<17)w.migrate_work_credits();
    if(w.provinces_.size()!=size_t(w.regions_+1))throw std::runtime_error("Save province count");w.refresh_demography();s>>std::ws;if(!s.eof()||s.fail()&&!s.eof()||w.people_.size()<128||w.day_<0||w.day_>365000||!w.invariant(bonded_goods))throw std::runtime_error("Corrupt save/invariant");*this=std::move(w);error.clear();return true;
}catch(const std::exception&e){error=e.what();return false;}}
bool Realm::save(const std::string&path,const std::string&bundle,std::string&error)const{try{auto dest=std::filesystem::u8path(path),temp=dest;temp+=".tmp";auto data=serialize(bundle);{std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<hash_text(data)<<'\n'<<data;out.flush();if(!out)throw std::runtime_error("Save write failed");}auto backup=dest;backup+=".bak";if(std::filesystem::exists(dest))std::filesystem::copy_file(dest,backup,std::filesystem::copy_options::overwrite_existing);std::filesystem::rename(temp,dest);error.clear();return true;}catch(const std::exception&e){error=e.what();return false;}}
bool Realm::load(const std::string&path,const std::string&bundle,std::string&error){try{std::ifstream in(std::filesystem::u8path(path),std::ios::binary);uint64_t checksum;in>>checksum;in.get();std::string data((std::istreambuf_iterator<char>(in)),{});if(!in||hash_text(data)!=checksum)throw std::runtime_error("Save checksum/read failed");return deserialize(data,bundle,error);}catch(const std::exception&e){error=e.what();return false;}}
uint64_t Realm::fingerprint()const{return hash_text(serialize("fingerprint"));}

} // namespace polyec
