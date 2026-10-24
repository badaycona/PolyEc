#pragma once
#include "polyec/good_catalog.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace polyec {

enum class EstablishmentOwnerKind : int {
    Person = 0,
    Public = 1,
    Organization = 2,
    Municipal = 3
};

// A region's trial municipal account is funded only through this atomic
// treasury transfer. The period limit is enforced by Realm, not by callers.
struct MunicipalBudgetCommand {
    uint64_t actor=0,sequence=0;
    int region=0;
    int64_t amount=0;
};
inline constexpr std::array<int,4> kMunicipalGrantPercentages{1,2,3,5};
constexpr int64_t municipal_grant_amount(int64_t treasury,int percentage) noexcept {
    if(treasury<=0||percentage<=0||percentage>5)return 0;
    return (treasury/100)*percentage+((treasury%100)*percentage)/100;
}

enum class EstablishmentIndustry : int {
    ClothWorkshop = 1,
    ToolWorkshop = 2,
    MilitarySupplyWorkshop = 3,
    Saltworks = 4,
    Forestry = 5,
    Mining = 6,
    PaperMill = 7,
    TeaGarden = 8,
    IncenseWorks = 9,
    WarhorseStable = 10,
    WeaponForge = 11,
    Glassworks = 12,
    PotteryKiln = 13,
    Tannery = 14,
    MedicinalGarden = 15,
    SeedPressWorks = 16,
    SawmillWorks = 17,
    Apiary = 18,
    SilkwormFarm = 19,
    GoldMine = 20,
    LivestockRanch = 21,
    Butchery = 22,
    DecorativeGlassWorks = 23,
    FinePorcelainWorks = 24,
    GeneralLuxuryAtelier = 25
};

inline constexpr int kEstablishmentCapacityMax = 1000000;
inline constexpr int64_t kEstablishmentSourceUnitsMaximum = 100;
inline constexpr int kNoEstablishmentSourceGood = -1;
inline constexpr int establishment_source_good(EstablishmentIndustry industry,int region){
    if(industry==EstablishmentIndustry::Forestry&&region==6)return int(GoodId::Timber);
    if(industry==EstablishmentIndustry::Mining&&region==7)return int(GoodId::Copper);
    if(industry==EstablishmentIndustry::Mining&&region==23)return int(GoodId::Silver);
    if(industry==EstablishmentIndustry::TeaGarden&&region==14)return int(GoodId::Tea);
    if(industry==EstablishmentIndustry::IncenseWorks&&region==11)return int(GoodId::Incense);
    if(industry==EstablishmentIndustry::Glassworks&&region==10)return int(GoodId::PlainGlass);
    if(industry==EstablishmentIndustry::PotteryKiln&&region==15)return int(GoodId::Pottery);
    if(industry==EstablishmentIndustry::Tannery&&region==17)return int(GoodId::Leather);
    if(industry==EstablishmentIndustry::MedicinalGarden&&region==20)return int(GoodId::Medicine);
    if(industry==EstablishmentIndustry::SeedPressWorks&&region==2)return int(GoodId::SupplementaryFood);
    if(industry==EstablishmentIndustry::SawmillWorks&&region==6)return kNoEstablishmentSourceGood;
    if(industry==EstablishmentIndustry::Apiary&&region==21)return int(GoodId::Beeswax);
    if(industry==EstablishmentIndustry::SilkwormFarm&&region==12)return int(GoodId::Silk);
    if(industry==EstablishmentIndustry::GoldMine&&region==23)return int(GoodId::Gold);
    if(industry==EstablishmentIndustry::LivestockRanch&&region==18)return int(GoodId::Livestock);
    return kNoEstablishmentSourceGood;
}
inline constexpr int kEstablishmentStockSlots = kGoodIdCount;
// Gate 1 stored one slot per group in ESTABLISH_1. Only historical groups
// with an explicit legacy product mapping can contain stock in that version.
inline constexpr std::array<int, 16> kGate1EstablishmentSlotToGoodId = {
    0, 1, 2, 3, 4, 5, -1, 9, 11, 14, 16, -1, -1, -1, -1, -1};
// ESTABLISH_1 group slot -> old seven-slot ledger, for Realm v11/v12 migration.
inline constexpr std::array<int, 16> kGate1EstablishmentSlotToLegacy = {
    0, -1, -1, -1, 4, 3, -1, 1, 2, 6, 5, -1, -1, -1, -1, -1};

struct WorkAuthorizationWindow {
    int64_t begin=0,end=0;
    bool operator==(const WorkAuthorizationWindow&) const = default;
};
struct EstablishmentWorkPay {
    std::map<uint64_t,int64_t> legacy_credit,paid_units,remainders;
};
struct Establishment {
    uint64_t id = 0;
    int region = 0;
    EstablishmentOwnerKind owner_kind = EstablishmentOwnerKind::Person;
    uint64_t owner_id = 0;
    EstablishmentIndustry industry = EstablishmentIndustry::ClothWorkshop;
    int capacity = 1;
    int64_t cash = 0;
    // Abstract, site-held resource ledger. source_good is a canonical GoodId
    // describing what the resource can produce; the units are not inventory.
    int source_good = kNoEstablishmentSourceGood;
    int64_t source_units = 0;
    std::array<int64_t, kGoodIdCount> stock{};
    // Gate 4 production bookkeeping. last_production_day is the newest Realm
    // day whose production phase already processed this site; zero for a site
    // never processed and for every pre-Gate-4 loaded world. processed_work_units
    // is the sparse, cumulative, positive-only count of 16-tick site work units
    // already consumed by production per living-or-once-living actor; written
    // sorted by actor ID and never above that actor/site's eligible attendance.
    int64_t last_production_day = 0;
    std::map<uint64_t, int64_t> processed_work_units{};
    std::map<uint64_t,std::vector<WorkAuthorizationWindow>> work_authorizations{};
    std::size_t authorization_window_count=0; // derived sparse index; rebuilt on load
    EstablishmentWorkPay work_pay;
};

struct EstablishmentCommand {
    uint64_t actor = 0;
    uint64_t sequence = 0;
    EstablishmentOwnerKind owner_kind = EstablishmentOwnerKind::Person;
    uint64_t owner_id = 0;
    int region = 0;
    EstablishmentIndustry industry = EstablishmentIndustry::ClothWorkshop;
    int capacity = -1; // request this world's validated configuration default
    int64_t funding = 0;
};

enum class EstablishmentTransferOperation : int {
    DepositCash=0, WithdrawCash=1, DepositGoods=2, WithdrawGoods=3
};
struct EstablishmentTransferCommand {
    uint64_t actor=0,sequence=0,establishment_id=0;
    EstablishmentTransferOperation operation=EstablishmentTransferOperation::DepositCash;
    int good=-1; // canonical GoodId for goods; -1 for cash
    int64_t amount=0;
};

enum class WorkAuthorizationOperation : int { Set=0, Revoke=1 };
struct WorkAuthorizationCommand {
    uint64_t actor=0,sequence=0,establishment_id=0,worker=0;
    WorkAuthorizationOperation operation=WorkAuthorizationOperation::Set;
    int64_t begin=0,end=0;
};

} // namespace polyec
