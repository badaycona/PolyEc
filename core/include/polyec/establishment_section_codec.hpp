#pragma once

#include "polyec/establishment.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

inline void write_establishment_section(std::ostream& out,
                                        const std::vector<Establishment>& establishments) {
    std::vector<Establishment> sorted = establishments;
    std::sort(sorted.begin(), sorted.end(),
              [](const Establishment& a, const Establishment& b) { return a.id < b.id; });
    out << " ESTABLISH_14 " << sorted.size() << ' ';
    for (const auto& e : sorted) {
        out << e.id << ' ' << e.region << ' ' << static_cast<int>(e.owner_kind) << ' '
            << e.owner_id << ' ' << static_cast<int>(e.industry) << ' ' << e.capacity << ' '
            << e.cash << ' ';
        for (int g = 0; g < kEstablishmentStockSlots; ++g) out << e.stock[g] << ' ';
        out << e.last_production_day << ' ' << e.processed_work_units.size() << ' ';
        for (const auto& [actor, units] : e.processed_work_units)
            out << actor << ' ' << units << ' ';
        out << e.work_authorizations.size() << ' ';
        for(const auto& [worker,windows]:e.work_authorizations){
            out << worker << ' ' << windows.size() << ' ';
            for(const auto& window:windows)out << window.begin << ' ' << window.end << ' ';
        }
        for(const auto* ledger:{&e.work_pay.legacy_credit,&e.work_pay.paid_units,&e.work_pay.remainders}){
            out<<ledger->size()<<' ';for(const auto& [actor,value]:*ledger)out<<actor<<' '<<value<<' ';
        }
        out << e.source_good << ' ' << e.source_units << ' ';
    }
}

inline bool read_establishment_section(std::istream& in, size_t people_count, int regions,
                                       int realm_schema, std::vector<Establishment>& out,
                                       std::string& error) {
    std::string tag;
    size_t count = 0;
    in >> tag >> count;
    const std::string expected = realm_schema >= 55 ? "ESTABLISH_14" : realm_schema >= 43 ? "ESTABLISH_13" : realm_schema >= 42 ? "ESTABLISH_12" : realm_schema >= 41 ? "ESTABLISH_11" : realm_schema >= 40 ? "ESTABLISH_10" : realm_schema >= 39 ? "ESTABLISH_9" : realm_schema >= 38 ? "ESTABLISH_8" : realm_schema >= 37 ? "ESTABLISH_7" : realm_schema >= 36 ? "ESTABLISH_6" : realm_schema >= 17 ? "ESTABLISH_5" : realm_schema >= 16 ? "ESTABLISH_4" : realm_schema >= 14 ? "ESTABLISH_3"
                                 : realm_schema == 13 ? "ESTABLISH_2" : "ESTABLISH_1";
    if (!in || tag != expected) {
        error = "Save establishment header";
        return false;
    }
    if (count > 1000000) {
        error = "Save establishment count limit";
        return false;
    }

    std::vector<Establishment> staged;
    staged.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        Establishment e{};
        int ownerKind = 0;
        int industry = 0;
        in >> e.id >> e.region >> ownerKind >> e.owner_id >> industry >> e.capacity >> e.cash;
        if (!in) {
            error = "Save establishment fields";
            return false;
        }
        if (ownerKind < 0 || ownerKind > (realm_schema >= 49 ? 3 : 2)) {
            error = "Save establishment owner kind";
            return false;
        }
        if (industry < 1 || industry > (realm_schema >= 64 ? 17 : realm_schema >= 63 ? 16 : realm_schema >= 62 ? 15 : realm_schema >= 55 ? 14 : realm_schema >= 43 ? 11 : realm_schema >= 42 ? 10 : realm_schema >= 41 ? 9 : realm_schema >= 40 ? 8 : realm_schema >= 39 ? 7 : realm_schema >= 38 ? 6 : realm_schema >= 37 ? 5 : realm_schema >= 36 ? 4 : 3)) {
            error = "Save establishment industry";
            return false;
        }
        e.owner_kind = static_cast<EstablishmentOwnerKind>(ownerKind);
        e.industry = static_cast<EstablishmentIndustry>(industry);
        if (realm_schema >= 13) {
            const int stock_count = realm_schema >= 55 ? kGoodIdCount : 27;
            for (int g = 0; g < stock_count; ++g) in >> e.stock[g];
        } else {
            for (size_t slot = 0; slot < kGate1EstablishmentSlotToGoodId.size(); ++slot) {
                int64_t value = 0;
                in >> value;
                const int good = kGate1EstablishmentSlotToGoodId[slot];
                if (!in || value < 0 || (good < 0 && value != 0)) {
                    error = "Save establishment stock";
                    return false;
                }
                if (good >= 0) e.stock[static_cast<size_t>(good)] = value;
            }
        }
        if (!in || std::any_of(e.stock.begin(), e.stock.end(),
                               [](int64_t value) { return value < 0; })) {
            error = "Save establishment stock";
            return false;
        }
        if (realm_schema >= 14) {
            // ESTABLISH_3 production bookkeeping: production-day marker plus the
            // sparse, actor-sorted, positive-only processed-work-unit map.
            in >> e.last_production_day;
            size_t watermarkCount = 0;
            in >> watermarkCount;
            if (!in) {
                error = "Save establishment production fields";
                return false;
            }
            if (e.last_production_day < 0) {
                error = "Save establishment production day";
                return false;
            }
            if (watermarkCount > people_count) {
                error = "Save establishment watermark count limit";
                return false;
            }
            uint64_t previousActor = 0;
            for (size_t w = 0; w < watermarkCount; ++w) {
                uint64_t actor = 0;
                int64_t units = 0;
                in >> actor >> units;
                if (!in) {
                    error = "Save establishment watermark fields";
                    return false;
                }
                if (actor == 0 || actor > people_count) {
                    error = "Save establishment watermark actor";
                    return false;
                }
                if (units <= 0) {
                    error = "Save establishment watermark units";
                    return false;
                }
                if (actor <= previousActor) {
                    error = "Save establishment watermark order";
                    return false;
                }
                previousActor = actor;
                e.processed_work_units.emplace(actor, units);
            }
        }
        if(realm_schema>=16){
            size_t workers=0,totalWindows=0;uint64_t previous=0;
            in>>workers;
            if(!in||workers>people_count){error="Save authorization count";return false;}
            for(size_t row=0;row<workers;++row){
                uint64_t worker=0;size_t windows=0;in>>worker>>windows;
                if(!in||worker<=previous||worker>people_count||windows==0||windows>1000000-totalWindows){
                    error="Save authorization worker";return false;
                }
                previous=worker;totalWindows+=windows;
                std::vector<WorkAuthorizationWindow> ranges;ranges.reserve(windows);
                for(size_t n=0;n<windows;++n){
                    WorkAuthorizationWindow range;in>>range.begin>>range.end;
                    if(!in||range.begin<0||range.end<=range.begin||range.end>365000LL*64||
                       (!ranges.empty()&&range.begin<=ranges.back().end)){
                        error="Save authorization range";return false;
                    }
                    ranges.push_back(range);
                }
                e.work_authorizations.emplace(worker,std::move(ranges));
            }
            e.authorization_window_count=totalWindows;
        }
        if(realm_schema>=17)for(auto* ledger:{&e.work_pay.legacy_credit,&e.work_pay.paid_units,&e.work_pay.remainders}){
            size_t entries=0;in>>entries;
            if(!in||entries>people_count){error="Save site payroll count";return false;}
            uint64_t previous=0;
            for(size_t row=0;row<entries;++row){
                uint64_t actor=0;int64_t value=0;in>>actor>>value;
                if(!in||actor<=previous||actor>people_count||value<=0){error="Save site payroll fields";return false;}
                ledger->emplace(actor,value);previous=actor;
            }
        }
        if(realm_schema==37){
            int64_t legacyForestUnits=0;in>>legacyForestUnits;
            if(e.industry==EstablishmentIndustry::Forestry){
                e.source_good=int(GoodId::Timber);e.source_units=legacyForestUnits;
            }else if(legacyForestUnits!=0){
                error="Save establishment legacy forest resource";return false;
            }
        }else if(realm_schema>=38)in>>e.source_good>>e.source_units;
        if(!in){error="Save establishment source fields";return false;}
        // basic range checks
        if (e.id == 0) {
            error = "Save establishment id";
            return false;
        }
        if (e.region < 0 || e.region > regions) {
            error = "Save establishment region";
            return false;
        }
        if (e.capacity < 1 || e.capacity > kEstablishmentCapacityMax) {
            error = "Save establishment capacity";
            return false;
        }
        if (e.cash < 0) {
            error = "Save establishment cash";
            return false;
        }
        const bool sourceIndustry=e.industry==EstablishmentIndustry::Forestry||e.industry==EstablishmentIndustry::Mining||
            e.industry==EstablishmentIndustry::TeaGarden||e.industry==EstablishmentIndustry::IncenseWorks||
            (realm_schema>=55&&(e.industry==EstablishmentIndustry::Glassworks||
             e.industry==EstablishmentIndustry::PotteryKiln||e.industry==EstablishmentIndustry::Tannery))||
            (realm_schema>=62&&e.industry==EstablishmentIndustry::MedicinalGarden)||
            (realm_schema>=63&&e.industry==EstablishmentIndustry::SeedPressWorks);
        const int expectedSource=establishment_source_good(e.industry,e.region);
        if(e.industry==EstablishmentIndustry::Mining&&e.region!=7&&e.region!=23){
            error="Save Mining source region";return false;
        }
        if(realm_schema>=55&&((e.industry==EstablishmentIndustry::Glassworks&&e.region!=10)||
           (e.industry==EstablishmentIndustry::PotteryKiln&&e.region!=15)||
           (e.industry==EstablishmentIndustry::Tannery&&e.region!=17))){
            error="Save Atlas specialty source region";return false;
        }
        if(realm_schema>=62&&e.industry==EstablishmentIndustry::MedicinalGarden&&e.region!=20){
            error="Save Medicinal Garden source region";return false;
        }
        if(realm_schema>=63&&e.industry==EstablishmentIndustry::SeedPressWorks&&e.region!=2){
            error="Save Seed Press Works source region";return false;
        }
        if(realm_schema>=64&&e.industry==EstablishmentIndustry::SawmillWorks&&
           (regions!=int(kAtlasTerritoryCount)-1||e.region!=6)){
            error="Save Sawmill Works region";return false;
        }
        if((sourceIndustry&&(e.source_good!=expectedSource||e.source_units<0||
                            e.source_units>kEstablishmentSourceUnitsMaximum))||
           (!sourceIndustry&&(e.source_good!=kNoEstablishmentSourceGood||e.source_units!=0))){
            error="Save establishment source resource";
            return false;
        }
        // owner_id range checks
        if (e.owner_kind == EstablishmentOwnerKind::Person) {
            if (e.owner_id == 0 || e.owner_id > people_count) {
                error = "Save establishment owner";
                return false;
            }
        } else if (e.owner_kind == EstablishmentOwnerKind::Public) {
            if (e.owner_id > static_cast<uint64_t>(regions)) {
                error = "Save establishment owner";
                return false;
            }
        } else {
            if (e.owner_id < 1 || e.owner_id > kOrganizationCount) {
                error = "Save establishment owner";
                return false;
            }
        }
        staged.push_back(e);
    }

    // require sorted by id
    for (size_t i = 1; i < staged.size(); ++i) {
        if (staged[i - 1].id >= staged[i].id) {
            error = "Save establishment order";
            return false;
        }
    }

    out = std::move(staged);
    error.clear();
    return true;
}

} // namespace detail
} // namespace polyec
