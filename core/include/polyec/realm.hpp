#pragma once
#include "polyec/schedule.hpp"
#include "polyec/realm_clock.hpp"
#include "polyec/establishment.hpp"
#include "polyec/gameplay_config.hpp"
#include "polyec/market_manager.hpp"
#include "polyec/national_transport_manager.hpp"
#include "polyec/work_credit.hpp"
#include "polyec/market_manager.hpp"
#include "polyec/household_fund.hpp"
#include "polyec/land_holding.hpp"
#include "polyec/private_residence.hpp"
#include "polyec/world_atlas.hpp"
#include "polyec/atlas_settlements.hpp"
#include "polyec/jobs/job.hpp"
#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <iosfwd>

namespace polyec {
inline constexpr size_t kOrganizationGroupsPerTerritory=4;
inline constexpr size_t kOrganizationCount=kAtlasTerritoryCount*kOrganizationGroupsPerTerritory;
inline constexpr const char* realm_schema="realm-v43";
inline constexpr const char* realm_reward_version="realm-v14-person-enterprise-equity-atlas-products-v1";
inline constexpr int realm_base_state=128, realm_features=12;
inline constexpr int realm_pre_ration_state=realm_base_state+int((kAtlasTerritoryCount-7)*kLegacyGoodCount);
inline constexpr int realm_ration_depot_observation_begin=realm_pre_ration_state;
inline constexpr int realm_foreign_trade_extra_observation_begin=realm_pre_ration_state+int(kAtlasTerritoryCount);
inline constexpr int realm_foreign_trade_extra_observation_count=6;
inline constexpr int realm_supplementary_ration_observation_begin=realm_foreign_trade_extra_observation_begin+realm_foreign_trade_extra_observation_count;
inline constexpr int realm_supplementary_ration_observation_count=int(kAtlasTerritoryCount);
inline constexpr int realm_military_supply_obligation_observation_begin=realm_supplementary_ration_observation_begin+realm_supplementary_ration_observation_count;
inline constexpr int realm_military_supply_obligation_observation_count=int(kAtlasTerritoryCount)*2;
inline constexpr int realm_state=realm_military_supply_obligation_observation_begin+realm_military_supply_obligation_observation_count;
inline constexpr int realm_actions=400;
inline constexpr int realm_tea_social_candidate=24;
inline constexpr int realm_incense_social_candidate=25;
inline constexpr int realm_silver_social_candidate=26;
inline constexpr int realm_silk_social_candidate=27;
inline constexpr int realm_gold_social_candidate=28;
inline constexpr int realm_decorative_glass_social_candidate=29;
inline constexpr int realm_fine_porcelain_social_candidate=30;
inline constexpr int realm_general_luxury_social_candidate=31;
inline constexpr int realm_council_nomination_candidate=71;
inline constexpr int realm_council_accept_candidate_begin=282;
inline constexpr int realm_council_reject_candidate_begin=309;
inline constexpr int realm_council_decision_candidate_count=27;
inline constexpr int realm_private_housing_candidate_begin=336;
inline constexpr int realm_private_build_candidate_count=16;
inline constexpr int realm_private_build_candidate_begin=realm_private_housing_candidate_begin;
inline constexpr int realm_private_offer_candidate_count=16;
inline constexpr int realm_private_offer_candidate_begin=realm_private_build_candidate_begin+realm_private_build_candidate_count;
inline constexpr int realm_private_response_candidate_count=16;
inline constexpr int realm_private_response_candidate_begin=realm_private_offer_candidate_begin+realm_private_offer_candidate_count;
inline constexpr int realm_private_end_lease_candidate_count=16;
inline constexpr int realm_private_end_lease_candidate_begin=realm_private_response_candidate_begin+realm_private_response_candidate_count;
static_assert(realm_private_end_lease_candidate_begin+realm_private_end_lease_candidate_count==realm_actions);
inline constexpr int realm_sleep_debt_observation_index=108, realm_exertion_observation_index=109;
inline constexpr int realm_personality_observation_begin=110, realm_personality_observation_count=6;
inline constexpr int realm_knowledge_evidence_observation_index=88, realm_knowledge_legacy_observation_index=89;
inline constexpr int realm_marriage_candidate_begin=72, realm_marriage_candidate_count=8;
inline constexpr int realm_establishment_candidate_begin=80, realm_establishment_candidate_count=21;
inline constexpr int realm_worksite_candidate_begin=101, realm_worksite_candidate_sites=8, realm_worksite_actions_per_site=5;
inline constexpr int realm_extended_travel_candidate_begin=141, realm_extended_travel_region_begin=7;
inline constexpr int realm_tea_garden_candidate_begin=161, realm_tea_garden_candidate_count=3;
inline constexpr int realm_incense_works_candidate_begin=164, realm_incense_works_candidate_count=3;
inline constexpr int realm_warhorse_stable_candidate_begin=167, realm_warhorse_stable_candidate_count=3;
inline constexpr int realm_weapon_forge_candidate_begin=170, realm_weapon_forge_candidate_count=3;
inline constexpr int realm_military_service_candidate_begin=173, realm_military_service_candidate_count=16;
inline constexpr int64_t kMilitaryServiceTrialWagePerDay=4;
inline constexpr int realm_military_ration_purchase_candidate_begin=189, realm_military_ration_purchase_candidate_count=4;
inline constexpr int realm_military_ration_shipment_candidate_begin=193, realm_military_ration_shipment_candidate_count=27;
inline constexpr int realm_supplementary_ration_purchase_candidate_begin=232, realm_supplementary_ration_purchase_candidate_count=4;
inline constexpr int realm_supplementary_ration_shipment_candidate_begin=236, realm_supplementary_ration_shipment_candidate_count=27;
inline constexpr int realm_municipal_fund_candidate_begin=32, realm_municipal_fund_candidate_count=4;
inline constexpr int realm_municipal_relief_candidate=69;
inline constexpr int realm_military_obligation_offer_candidate=27;
inline constexpr int realm_military_obligation_renegotiate_candidate=70;
inline constexpr int realm_military_supply_obligation_candidate=263;
inline constexpr int realm_specialty_product_candidate_begin=264, realm_specialty_product_candidate_count=9;
inline constexpr int realm_municipal_specialty_product_candidate_begin=273, realm_municipal_specialty_product_candidate_count=9;
inline constexpr int realm_municipal_fund_observation_index=116;
inline constexpr int realm_foreign_trade_candidate_begin=32, realm_foreign_trade_candidate_count=6;
inline constexpr int realm_foreign_trade_partner_candidate_begin=220, realm_foreign_trade_partner_candidate_count=12;
inline constexpr int realm_apprenticeship_civilian_candidate_begin=32, realm_apprenticeship_civilian_candidate_count=39;
inline constexpr int realm_apprenticeship_economic_candidate_begin=44, realm_apprenticeship_economic_candidate_count=27;
inline constexpr int64_t kApprenticeshipTrialV1Tuition=20;
inline constexpr int64_t kApprenticeshipTrialTuition=10;
constexpr bool apprenticeship_tuition_is_supported(int64_t tuition) noexcept {
    return tuition==kApprenticeshipTrialTuition||tuition==kApprenticeshipTrialV1Tuition;
}
inline constexpr int kApprenticeshipTrialTermDays=30;
inline constexpr int kApprenticeshipTrialPracticeDays=10;
inline constexpr int kApprenticeshipTrialSkillGain=10;
inline constexpr int kApprenticeshipTrialMentorSkill=40;
inline constexpr int realm_foreign_trade_partner_observation_begin=realm_foreign_trade_extra_observation_begin;
inline constexpr int realm_foreign_trade_stock_observation_index=117;
inline constexpr int realm_foreign_trade_capacity_observation_index=118;
inline constexpr int realm_foreign_trade_cash_observation_index=119;
inline constexpr int realm_military_obligation_required_observation_index=120;
inline constexpr int realm_military_obligation_delivery_observation_index=121;
inline constexpr int realm_military_obligation_days_left_observation_index=122;
inline constexpr int realm_military_obligation_overdue_observation_index=123;
inline constexpr int realm_military_obligation_first_shortage_observation_index=124;
inline constexpr int realm_military_obligation_extension_observation_index=125;
inline constexpr int realm_military_obligation_recipient_observation_index=126;
inline constexpr int realm_military_obligation_member_observation_index=127;
inline constexpr int64_t kForeignTradeOpeningCash=5000;
inline constexpr int64_t kForeignTradeOpeningDryFood=32;
inline constexpr int64_t kForeignTradeSaltCapacity=32;
inline constexpr int kForeignTradePartnerCount=3;
inline constexpr int64_t kForeignTradePartnerOpeningCash=5000;
inline constexpr int64_t kForeignTradePartnerOpeningDryFood=32;
inline constexpr int64_t kForeignTradePartnerSaltCapacity=32;
inline constexpr int kMilitaryRaidUnitMaximum=8;
inline constexpr int kMilitaryRaidPendingMaximum=kForeignTradePartnerCount;
inline constexpr int64_t kMilitaryRaidCashPerUnit=25;
inline constexpr int kMilitaryRaidTargetRegion=8;
inline constexpr int64_t kMilitarySupplementaryRationOpeningStock=32;
inline constexpr int64_t kUrbanHousingBedsPerBlock=8;
inline constexpr int64_t kUrbanHousingProjectBudget=80;
inline constexpr int kUrbanHousingProjectTermDays=60;
inline constexpr int64_t kPrivateResidenceBuildCost=40;
inline constexpr int64_t kPrivateResidenceConstructionMaterial=2;
inline constexpr int64_t kPrivateResidenceTools=1;
inline constexpr int64_t kPrivateResidencePlainGlass=1;
inline constexpr int64_t kPrivateResidenceBeds=4;
inline constexpr int kPrivateResidenceBuildDays=30;
inline constexpr int64_t kPrivateResidenceRentPerPeriod=2;
inline constexpr int kPrivateResidenceRentPeriodDays=30;
inline constexpr int kPrivateResidenceLeaseDays=180;
inline constexpr int kPrivateResidenceOfferDays=7;
inline constexpr int realm_municipal_workshop_candidate_begin=36, realm_municipal_workshop_candidate_count=21;
inline constexpr int realm_municipal_special_candidate_begin=57, realm_municipal_special_candidate_count=12;
inline constexpr std::array<int64_t,realm_military_ration_purchase_candidate_count> kMilitaryRationPurchaseQuantities{1,4,16,64};
inline constexpr int realm_obs=realm_state+realm_actions*realm_features;
enum class Family:int { Governance=0,Economic=1,Civilian=2 };
inline constexpr uint8_t kNoUrbanSite=255;
enum class ResidenceKind:uint8_t { Unknown=0,Rural=1,Urban=2 };
inline constexpr uint64_t urban_residence_hash_v1(uint64_t seed,uint64_t person_id) noexcept {
    uint64_t value=seed^(person_id*0x9E3779B97F4A7C15ull);
    value+=0x9E3779B97F4A7C15ull;
    value=(value^(value>>30))*0xBF58476D1CE4E5B9ull;
    value=(value^(value>>27))*0x94D049BB133111EBull;
    return value^(value>>31);
}
enum class Intent:int { Hold,Buy,Sell,Travel,Borrow,Repay,Project,Hire,Tax,Aid,Investigate,Offer,Accept,Reject,Work,Study,Rest,Social,Join,Migrate,Support,Appoint,Mobilize,Resign,Marry,CreateEstablishment,TransferEstablishmentGoods,AuthorizeWorkAtSite,WorkAtSite,BuyMilitaryRations,ShipMilitaryRations,FundMunicipal,ForeignTrade,NominateCouncil,ResolveCouncilNomination,PrivateResidenceBuild,PrivateHousing,OfferApprenticeship,EndApprenticeship };
struct WorkAssignment { int job=-1; int job_kind=0; };
struct HouseholdMembership { uint64_t household_id=0; };
// Named bridge for the six positional stats retained by legacy saves. This is
// not the configurable capability schema proposed for the agent-centric model.
enum class LegacyCapability : uint8_t {
    Strategy=0, Administration=1, EconomicPlanning=2,
    Diplomacy=3, Investigation=4, Military=5
};
// Stable catalog IDs. The first six preserve the serialized legacy Stats order;
// LaborSkill maps to the existing Person::skill field and is always active.
enum class CapabilityId : uint8_t {
    Strategy=0, Administration=1, EconomicPlanning=2,
    Diplomacy=3, Investigation=4, Military=5, LaborSkill=6
};
inline constexpr std::array<LegacyCapability,6> kLegacyCapabilities{
    LegacyCapability::Strategy, LegacyCapability::Administration,
    LegacyCapability::EconomicPlanning, LegacyCapability::Diplomacy,
    LegacyCapability::Investigation, LegacyCapability::Military
};
inline constexpr std::array<const char*,6> kLegacyCapabilityKeys{
    "strategy", "administration", "economic_planning",
    "diplomacy", "investigation", "military"
};
inline constexpr uint32_t kDefaultCapabilityMask=(1u<<kLegacyCapabilities.size())-1u;
inline constexpr int kInactiveCapabilityNeutralValue=50;
inline constexpr int kCapabilityMinimum=0;
inline constexpr int kCapabilityMaximum=100;
struct CapabilityDefinition {
    CapabilityId id;
    const char* key;
    const char* unit;
    int minimum;
    int maximum;
    const char* owner;
    bool scenario_configurable;
    bool always_active;
    int disabled_value; // -1 means this capability cannot be disabled.
};
inline constexpr std::array<CapabilityDefinition,7> kCapabilityCatalog{{
    {CapabilityId::Strategy,"strategy","legacy capability point",0,100,"Person",true,false,kInactiveCapabilityNeutralValue},
    {CapabilityId::Administration,"administration","legacy capability point",0,100,"Person",true,false,kInactiveCapabilityNeutralValue},
    {CapabilityId::EconomicPlanning,"economic_planning","legacy capability point",0,100,"Person",true,false,kInactiveCapabilityNeutralValue},
    {CapabilityId::Diplomacy,"diplomacy","legacy capability point",0,100,"Person",true,false,kInactiveCapabilityNeutralValue},
    {CapabilityId::Investigation,"investigation","legacy capability point",0,100,"Person",true,false,kInactiveCapabilityNeutralValue},
    {CapabilityId::Military,"military","legacy capability point",0,100,"Person",true,false,kInactiveCapabilityNeutralValue},
    {CapabilityId::LaborSkill,"labor_skill","labor skill point",0,100,"Person",false,true,-1}
}};
inline constexpr size_t kCapabilityCount=kCapabilityCatalog.size();
constexpr std::array<const char*,kCapabilityCount> capability_keys_from_catalog(){
    std::array<const char*,kCapabilityCount> keys{};
    for(size_t i=0;i<kCapabilityCount;++i)keys[i]=kCapabilityCatalog[i].key;
    return keys;
}
inline constexpr auto kCapabilityKeys=capability_keys_from_catalog();
struct Stats {
    std::array<int,6> values{50,50,50,50,50,50};
    constexpr int get(LegacyCapability capability) const noexcept {
        return values[static_cast<size_t>(capability)];
    }
    constexpr void set(LegacyCapability capability,int value) noexcept {
        values[static_cast<size_t>(capability)]=value;
    }
};
// Private decision tendencies, stored on each person and exposed only to that
// person's policy observation. Order is part of the model input definition.
enum class PersonalityAxis : uint8_t { Ambition=0, MaterialSelfInterest=1, CommunityConcern=2, Commitment=3, RiskAcceptance=4, Patience=5 };
inline constexpr int kPersonalityNeutral=50, kPersonalityMinimum=0, kPersonalityMaximum=100;
struct Personality { std::array<int,realm_personality_observation_count> values{kPersonalityNeutral,kPersonalityNeutral,kPersonalityNeutral,kPersonalityNeutral,kPersonalityNeutral,kPersonalityNeutral}; };
struct OfficeAppointment {
    bool office=false;
    uint64_t hired_by=0;
    int salary_due=30;
    int office_notice=0;
};
struct MilitaryService {
    bool active=false;
    bool combat_consent=false;
    uint64_t commander=0;
    int start_day=0,end_day=0,source_region=0;
    int mobilized_until_day=0;
    int64_t wage_per_day=0,escrow=0,paid_total=0;
};
// A volunteer separately chooses whether this service term may include combat.
// The person, not a commander or Sovereign, owns this decision.
struct MilitaryCombatConsentCommand {uint64_t actor=0,sequence=0;bool consent=false;};
struct MilitaryUnitRoster {
    uint64_t id=0,commander=0;
    int source_region=0;
    std::vector<uint64_t> members;
    std::vector<uint64_t> mobilized_members;
    // Derived readiness views. Combat consent alone does not deploy a person;
    // the battle command also checks the selected battle region at execution.
    std::vector<uint64_t> combat_consented_members;
    std::vector<uint64_t> battle_ready_members;
    int mobilized_until_day=0;
    int64_t payroll_escrow=0,paid_total=0;
};
enum class MilitaryRaidStatus : uint8_t { Pending=0,Repelled=1,Successful=2,Withdrawn=3 };
struct MilitaryRaid {
    uint64_t id=0;
    int partner_id=0,target_region=kMilitaryRaidTargetRegion,created_day=0,resolved_day=-1;
    int force_units=0,defense_score=0,attacker_losses=0,defender_casualties=0;
    int64_t salt_looted=0;
    MilitaryRaidStatus status=MilitaryRaidStatus::Pending;
    std::vector<uint64_t> defenders;
    std::vector<uint64_t> supplied_defenders;
    bool operator==(const MilitaryRaid&) const = default;
};
// An external partner explicitly funds an abstract coastal raid; no enemy
// Person records are invented. The actor uses its existing response sequence.
struct MilitaryRaidCommand {int partner_id=-1;uint64_t sequence=0;int force_units=0;int target_region=kMilitaryRaidTargetRegion;};
// Only the Sovereign may resolve the pending raid. Defenders are revalidated
// against consent, readiness, health, and their current location.
struct MilitaryBattleCommand {
    uint64_t actor=0,sequence=0,raid_id=0;
    bool retreat=false;
    std::vector<uint64_t> defenders;
};
// A consensual promise to deliver named, already-serving volunteers to the
// sovereign. Delivery records are historical once a volunteer arrives.
struct MilitaryObligation {
    uint64_t id=0,obligor=0,recipient=0;
    int source_region=0,destination_region=0,created_day=0,first_due_day=0,due_day=0;
    int completed_day=-1,extensions=0;
    int required_members=0,delivered_at_first_deadline=0,shortage_at_first_deadline=0;
    int delivered_at_last_deadline=0,shortage_at_last_deadline=0;
    bool first_deadline_checked=false,due_checked=false;
    std::vector<uint64_t> member_ids,delivered_member_ids;
};
// A consensual public DryFood commitment attached to one accepted personnel
// obligation. Delivery is counted only from later public depot shipments.
struct MilitarySupplyObligation {
    uint64_t id=0,personnel_obligation_id=0,obligor=0,recipient=0;
    uint64_t first_eligible_shipment_id=0;
    int source_region=0,destination_region=0,created_day=0,first_due_day=0,due_day=0;
    int completed_day=-1,extensions=0;
    int64_t required=0,delivered=0,delivered_at_first_deadline=0,shortage_at_first_deadline=0;
    int64_t delivered_at_last_deadline=0,shortage_at_last_deadline=0;
    bool first_deadline_checked=false,due_checked=false;
};
inline constexpr int64_t kMilitaryRationPurchaseMaximum=64;
struct MilitaryRationPurchaseCommand {
    uint64_t actor=0,sequence=0;
    int region=-1;
    int64_t quantity=0;
    int good=int(GoodId::DryFood);
};

// T3.4/T3.12 trial units: 16,000 credited Work attention units add one
// exertion point; each 16 actual Rest ticks recover one exertion and one health
// point when the Rest interval completes and nutrition is sufficient.
inline constexpr int64_t kExertionAttentionUnitsPerPoint=16000;
inline constexpr int64_t kExertionRestTicksPerPoint=16;

// Body indicators are stored separately from health and attention. These
// bounded scales carry state only; T3 transitions interpret them over game time.
struct BodyState {
    int sleep_debt=0;
    int exertion=0;
    int sleep_balance_remainder=0;
    int64_t exertion_effort_attention_processed=0;
    int exertion_effort_attention_remainder=0;
    int exertion_rest_tick_remainder=0;
    int64_t rest_begin_tick=0,rest_until_tick=0;
    int rest_region=-1;
    bool rest_health_pending=false;
};

struct Person {
    uint64_t id=0;
    HouseholdMembership household_membership{};
    uint64_t decisions=0,sampling=0;
    WorkAssignment work_assignment;
    int region=0,age_days=0,health=100,skill=0,busy_until=0,destination=-1;
    ResidenceKind residence_kind=ResidenceKind::Unknown;
    uint8_t urban_site_id=kNoUrbanSite;
    int goal=0,school_days=0,social=0; bool alive=true;
    int tax_remainder=0;
    int organization=0;
    bool taxing_authority=false,sovereign=false;
    int nutrition=100;
    bool candidate=false,special_advisor=false;
    int last_decision_day=-1;
    OfficeAppointment office_appointment;
    MilitaryService military_service;
    BodyState body_state;
    Personality personality;
    Family family=Family::Civilian; Stats stats;
    int64_t cash=100; std::array<int64_t,kGoodIdCount> inventory{};
    int capability(CapabilityId id) const {
        const auto index=static_cast<unsigned>(id);
        if(index<kLegacyCapabilities.size())return stats.values[index];
        if(id==CapabilityId::LaborSkill)return skill;
        throw std::out_of_range("capability ID");
    }
};
struct UrbanCensus {
    uint8_t site_id=0,territory_id=0;
    uint32_t urban_residents=0,rural_residents=0,unknown_residence=0;
    int64_t housing_capacity=0,unhoused_residents=0,available_beds=0;
    bool operator==(const UrbanCensus&) const = default;
};
struct MunicipalCouncil {
    uint8_t site_id=0;
    uint64_t representative=0,nominee=0;
    int nomination_day=-1,term_end_day=-1;
    bool operator==(const MunicipalCouncil&) const = default;
};
// Pure read-only job classifier: returns the shared singleton descriptor for the
// person's active assignment at current_day, or nullptr for no active job.
// Alive + valid canonical assignment required: appointed office
// (office_appointment.office && office_appointment.hired_by!=0 && Governance)
// or active work job (work_assignment.job>=0 && busy_until>=day with
// work_assignment.job_kind exactly 0 or 1). Unknown job_kind/dead/expired/
// invalid -> nullptr.
const Job* describe_job(const Person& person, int current_day) noexcept;
struct Province { int64_t treasury=10000,firm=30000;std::array<int64_t,kGoodIdCount> stock{};std::array<int,kGoodIdCount> price{};int tax=10,school=8,school_used=0,jobs=32,employed=0,road=0,unrest=0,teachers=0;bool charter=true;int residents=0;int64_t health_sum=0;int sales_tax_remainder=0;int64_t tax_receipts=0,food_capacity=0;int teacher_skill_sum=0; };
enum class RelationKind : uint8_t { ParentChild=0, Sibling=1, Agreement=2, Spouse=3 };
struct Edge { uint64_t from=0,to=0;RelationKind kind=RelationKind::ParentChild;bool legal_parentage=false; };
// creditor is the provincial firm-account index, not a Person ID.
struct Loan { uint64_t id=0,debtor=0;int creditor=0,due=0;int64_t principal=0,interest=0,paid=0;bool defaulted=false,royal_pool=false;uint64_t cause=0; };
struct Project {uint64_t id=0,owner=0;int region=0,due=0,kind=0;int64_t budget=0;bool completed=false,engineer=false,stalled=false;uint64_t cause=0;};
struct Fact {uint64_t id=0,parent=0,actor=0,target=0;int day=0,region=0,kind=0;int64_t amount=0;bool secret=false;};
// Old rows retain unknown type/confidence. Investigation reports are evidence,
// not automatically confirmed facts; -1 is the explicit unknown-confidence value.
enum class KnowledgeKind : uint8_t { UnknownLegacy=0, SecretEvidence=1 };
inline constexpr int kUnknownKnowledgeConfidence=-1;
inline constexpr int kKnowledgeConfidenceBasisPointMaximum=10000;
struct Knowledge {
    uint64_t observer=0,fact=0;
    int arrives=0;
    uint64_t source_investigation_id=0;
    KnowledgeKind kind=KnowledgeKind::UnknownLegacy;
    int confidence_basis_points=kUnknownKnowledgeConfidence;
};
// A read-only Knowledge row projected for its owner. Unlike RealmView::facts,
// this contains only explicit records created by an information source.
struct PlayerKnowledge {
    uint64_t observer=0,fact=0;
    int arrives=0;
    uint64_t source_investigation_id=0;
    KnowledgeKind kind=KnowledgeKind::UnknownLegacy;
    int confidence_basis_points=kUnknownKnowledgeConfidence;
};
struct Proposal {uint64_t id=0,from=0,to=0;int64_t amount=0;int due=0;bool resolved=false;int kind=0,ready=0;int64_t escrow=0;bool public_source=false,mediation=false;int source_region=0;uint64_t cause=0;std::array<uint64_t,3> parties{};std::array<int,3> regions{};std::array<bool,3> pledged{};int service_term=0;std::vector<uint64_t> roster_members;};
enum class ApprenticeshipStatus : uint8_t { Active=0, Completed=1, Expired=2, Cancelled=3, Interrupted=4 };
struct Apprenticeship {
    uint64_t id=0,mentor=0,apprentice=0,establishment_id=0;
    int started_day=0,due_day=0,practice_days=0;
    int64_t started_tick=0,due_tick=0,mentor_ticks_observed=0,apprentice_ticks_observed=0;
    int64_t tuition=0;
    ApprenticeshipStatus status=ApprenticeshipStatus::Active;
    bool operator==(const Apprenticeship&) const = default;
};
struct Investigation {uint64_t id=0,owner=0,cause=0;int region=0,due=0;bool completed=false;};
struct EstateCase {
    uint64_t decedent = 0;
    int opened_day = 0;
    int region = 0;
    int64_t cash = 0;
    std::array<int64_t, kGoodIdCount> inventory{};
    std::vector<uint64_t> establishments;
    std::vector<uint64_t> land_holdings;
    std::vector<uint64_t> land_use_rights;
    std::vector<uint64_t> loans;
    bool operator==(const EstateCase&) const = default;
};
struct Candidate {Intent kind=Intent::Hold;int target=0,good=0;int64_t amount=0;int term=0;};
struct RealmCommand {uint64_t actor=0,sequence=0;Candidate action;};
enum class MarketOrderOperation : uint8_t { Place=0, Cancel=1 };
struct MarketOrderCommand {
    uint64_t actor=0,sequence=0,order_id=0;
    MarketOrderOperation operation=MarketOrderOperation::Place;
    int region=0,good=0;
    int64_t quantity=0,maximum_unit_price=0;
};
struct MarketOrder {
    uint64_t id=0,owner=0;
    int region=0,good=0,placed_day=0,settle_day=0;
    uint64_t funding_household=0;
    MarketPriority priority=MarketPriority::Ordinary;
    int64_t quantity=0,maximum_unit_price=0;
    int64_t personal_escrow=0,household_escrow=0,organization_escrow=0;
    int organization_id=0;
    bool operator==(const MarketOrder&) const = default;
};
struct MarketMonthlyTotals {
    int64_t demand=0,unmet=0;
    bool operator==(const MarketMonthlyTotals&) const = default;
};
struct TransportRouteCommand {
    uint64_t actor=0,sequence=0;
    uint8_t route_index=0;
    bool open=false;
};
enum class TransportShipmentOperation : uint8_t { Create=0, Cancel=1 };
struct TransportShipmentCommand {
    uint64_t actor=0,sequence=0,shipment_id=0;
    TransportShipmentOperation operation=TransportShipmentOperation::Create;
    int destination=-1,good=-1;
    int64_t quantity=0;
    TransportServiceGoal service_goal=TransportServiceGoal::Fastest;
    bool military_ration_depot=false;
};
enum class TransportShipmentOwnerKind : uint8_t { Person=0, MilitaryRationDepot=1 };
struct TransportShipment {
    uint64_t id=0,owner=0;
    int created_day=0,source_region=-1,destination=0,good=0;
    TransportShipmentOwnerKind owner_kind=TransportShipmentOwnerKind::Person;
    TransportServiceGoal service_goal=TransportServiceGoal::Fastest;
    int64_t quantity=0,delivered=0,canceled=0,fee_escrow=0;
    std::vector<size_t> route_indices;
    bool operator==(const TransportShipment&) const = default;
};
struct TransportCargoLot {
    uint64_t id=0,shipment_id=0;
    size_t route_position=0;
    int current_region=0,arrival_day=0;
    int64_t quantity=0;
    bool operator==(const TransportCargoLot&) const = default;
};
enum class ForeignTradeOperation : uint8_t { Import=0, Export=1, ImportPartnerB=2, ExportPartnerB=3, ImportPartnerC=4, ExportPartnerC=5 };
inline constexpr bool foreign_trade_is_import(ForeignTradeOperation operation) {
    return (static_cast<unsigned>(operation)<6)&&(static_cast<unsigned>(operation)%2==0);
}
inline constexpr int foreign_trade_partner_id(ForeignTradeOperation operation) {
    return static_cast<unsigned>(operation)<6?int(static_cast<unsigned>(operation)/2):-1;
}
inline constexpr ForeignTradeOperation foreign_trade_operation(int partner,bool importing) {
    return partner>=0&&partner<kForeignTradePartnerCount
        ? static_cast<ForeignTradeOperation>(partner*2+(importing?0:1))
        : static_cast<ForeignTradeOperation>(255);
}
struct ForeignTradeCommand {uint64_t actor=0,sequence=0;ForeignTradeOperation operation=ForeignTradeOperation::Import;int good=0;int64_t quantity=0;};
struct ForeignTradeSuspensionCommand {uint64_t actor=0,sequence=0;int partner_id=0;bool suspended=false;};
struct ForeignTradeOfferCommand {uint64_t actor=0,sequence=0;int partner_id=0;ForeignTradeOperation operation=ForeignTradeOperation::Import;int good=0;int64_t quantity=0;};
struct ForeignTradeResponseCommand {int partner_id=0;uint64_t sequence=0,offer_id=0;bool accept=false;};
struct ForeignTradeOrder {
    uint64_t id=0,owner=0;
    int partner_id=0;
    ForeignTradeOperation operation=ForeignTradeOperation::Import;
    int good=0,created_day=0,arrival_day=0;
    int64_t quantity=0,gross=0,duty=0,escrow=0;
    bool operator==(const ForeignTradeOrder&) const = default;
};
enum class ForeignTradeOfferStatus : uint8_t { Pending=0,Accepted=1,Rejected=2,Expired=3 };
struct ForeignTradeOffer {
    uint64_t id=0,owner=0;
    int partner_id=0;
    ForeignTradeOperation operation=ForeignTradeOperation::Import;
    int good=0,created_day=0,due_day=0;
    int64_t quantity=0,gross=0,duty=0,held_cash=0,held_good=0,port_fee=0,route_fee=0;
    ForeignTradeOfferStatus status=ForeignTradeOfferStatus::Pending;
    uint64_t order_id=0;
    bool operator==(const ForeignTradeOffer&) const = default;
};
struct ForeignTradePartner {
    int64_t money=0,escrow=0,salt_capacity=0;
    int64_t offer_cash_escrow=0;
    uint64_t decision_sequence=0;
    std::array<int64_t,kGoodIdCount> goods{};
    bool trade_suspended=false;
    int64_t completed_import_orders=0,completed_export_orders=0;
    int64_t imported_units=0,exported_units=0;
    int64_t import_gross=0,export_gross=0;
    bool operator==(const ForeignTradePartner&) const = default;
};
struct RealmResult {bool success=false;std::string reason;uint64_t fact=0;uint64_t session=0;};
struct Draft {int tier=0;std::array<int,3> cards{};int seen=0,chosen=-1;bool rerolled=false;};
struct RealmView {int day=0,regions=0;uint64_t persons=0,alive=0,children=0,decisions=0;int64_t money=0;std::vector<Province> provinces;std::vector<Fact> facts;std::vector<PlayerKnowledge> knowledge;Draft draft;};
class Realm;
// Stateless coordinator for one Realm day. Owns only the approved phase order;
// all phase work and mutable state stay in Realm.
class RealmDayPipeline {
    friend class Realm;
    static void run(Realm& realm);
};
// Owns all mutable campaign state. Snapshots and observations are copies.
class Realm {
    friend class Campaign;
    friend class RealmDayPipeline;
    friend struct RealmObservationTestAccess;
    friend struct RealmTransportTestAccess;
    friend struct RealmUrbanResidenceTestAccess;
    friend struct RealmMedicineTestAccess;
    friend struct RealmSupplementaryFoodTestAccess;
    friend struct RealmPotteryMealVesselTestAccess;
    friend struct RealmBeeswaxCandleTestAccess;
    friend struct RealmSilkSocialTestAccess;
    friend struct RealmGoldSocialTestAccess;
    friend struct RealmLivestockFoodChainTestAccess;
    friend struct RealmDecorativeGlassTestAccess;
    friend struct RealmFinePorcelainTestAccess;
    friend struct RealmGeneralLuxuryTestAccess;
    friend struct RealmApprenticeshipTestAccess;
    friend struct RealmMunicipalCouncilTestAccess;
    friend struct RealmPrivateHousingTestAccess;
public:
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    static constexpr size_t day_phase_count = 14;
    static constexpr size_t people_phase_count = 5;
    const std::array<uint64_t, day_phase_count>& day_phase_profile_ns() const { return day_phase_profile_ns_; }
    const std::array<uint64_t, people_phase_count>& people_phase_profile_ns() const { return people_phase_profile_ns_; }
#endif
    Realm(uint64_t seed=1,int regions=6,int people=10000,bool advisor_fixture=false,GameplayConfig config={},int complex_actors=0,uint32_t capability_mask=kDefaultCapabilityMask);
    const GameplayConfig& gameplay_config() const {return config_;}
    int64_t work_pay_remainder(uint64_t actor,int region) const {
        const auto it=work_pay_remainders_.find({actor,region});
        return it==work_pay_remainders_.end()?0:it->second;
    }
    // Explicit initial-condition fixtures for paired eval. Never mutates a live world.
    static Realm scenario(uint64_t seed,uint64_t actor,int age=-1,int stat=-1,int goal=-1,int budget=-1,int schools=-1,int jobs=-1,int nutrition=-1,int health=-1,uint32_t capability_mask=kDefaultCapabilityMask);
    const Person& person(uint64_t id) const;
    std::vector<uint64_t> due(Family family) const;
    std::array<Candidate,realm_actions> candidates(uint64_t actor) const;
    const std::vector<Apprenticeship>& apprenticeships() const { return apprenticeships_; }
    std::array<float,realm_actions> mask(uint64_t actor) const;
    std::array<float,realm_actions> mask(uint64_t actor,const std::array<Candidate,realm_actions>& candidates) const;
    std::array<float,realm_obs> observation(uint64_t actor) const;
    std::array<float,realm_obs> observation(uint64_t actor,std::array<Candidate,realm_actions>& candidates_snapshot) const;
    RealmResult execute(const RealmCommand& command);
    RealmResult execute(const MarketOrderCommand& command);
    RealmResult execute(const TransportRouteCommand& command);
    RealmResult execute(const TransportShipmentCommand& command);
    RealmResult execute(const MilitaryRationPurchaseCommand& command);
    RealmResult execute(const MilitaryCombatConsentCommand& command);
    RealmResult execute(const MilitaryRaidCommand& command);
    RealmResult execute(const MilitaryBattleCommand& command);
    RealmResult execute(const ScheduleCommand& command);
    RealmResult execute(const RealmClockCommand& command);
RealmResult execute(const EstablishmentCommand& command);
    RealmResult execute(const MunicipalBudgetCommand& command);
    RealmResult execute(const ForeignTradeCommand& command);
    RealmResult execute(const ForeignTradeSuspensionCommand& command);
    RealmResult execute(const ForeignTradeOfferCommand& command);
    RealmResult execute(const ForeignTradeResponseCommand& command);
    RealmResult execute(const EstablishmentTransferCommand& command);
    RealmResult execute(const WorkAuthorizationCommand& command);
    RealmResult execute(const HouseholdFundCommand& command);
    RealmResult execute(const LandTitleTransferCommand& command);
    RealmResult execute(const LandUseGrantCommand& command);
    RealmResult execute(const LandUseRevokeCommand& command);
    RealmResult execute(const PrivateResidenceBuildCommand& command);
    RealmResult execute(const PrivateHousingCommand& command);
    const WorkCreditRegions& work_credits() const {return work_credits_;}
    void advance(int days=1);
    void advance_ticks(int64_t ticks);
    RealmView view(uint64_t observer=1) const;
    // A containing domain may own bonded goods (e.g. M4 manifests in transit).
    bool invariant(const std::array<int64_t,kGoodIdCount>& bonded_goods={}) const;
    uint64_t fingerprint() const;
    int64_t total_money() const;
    std::array<int64_t,11> balances()const;
std::vector<Establishment> establishments() const { return establishments_; }
    std::vector<int64_t> municipal_funds() const { return municipal_funds_; }
    int64_t foreign_trade_cash() const { return foreign_trade_partners_[0].money; }
    int64_t foreign_trade_escrow() const { return foreign_trade_partners_[0].escrow; }
    int64_t foreign_trade_salt_capacity() const { return foreign_trade_partners_[0].salt_capacity; }
    std::array<int64_t,kGoodIdCount> foreign_trade_goods() const { return foreign_trade_partners_[0].goods; }
    std::array<ForeignTradePartner,kForeignTradePartnerCount> foreign_trade_partners() const { return foreign_trade_partners_; }
    std::vector<ForeignTradeOrder> foreign_trade_orders() const { return foreign_trade_orders_; }
    std::vector<ForeignTradeOffer> foreign_trade_offers() const { return foreign_trade_offers_; }
    std::vector<MilitaryObligation> military_obligations() const { return military_obligations_; }
    std::vector<MilitarySupplyObligation> military_supply_obligations() const { return military_supply_obligations_; }
    std::vector<MilitaryRaid> military_raids() const { return military_raids_; }
    std::vector<UrbanCensus> urban_census() const;
    std::vector<MunicipalCouncil> municipal_councils() const { return municipal_councils_; }
    std::vector<LandHolding> land_holdings() const { return land_holdings_; }
    std::vector<LandUseRight> land_use_rights() const { return land_use_rights_; }
    std::vector<PrivateResidence> private_residences() const { return private_residences_; }
    int64_t royal_reference()const{return royal_reference_;}
    std::array<int64_t,2> royal_funds()const{return {credit_pool_,restricted_fund_};}
    std::vector<Proposal> offers(uint64_t observer)const;
    std::vector<Investigation> investigations()const{return investigations_;}
    double utility(uint64_t actor) const;
    bool reroll(int slot);
    RealmResult route_command(uint64_t actor,int from,int to,bool open);
    int distance(int from,int to)const;
    int travel_days(int from,int to)const;
    Draft draft()const{return draft_;}
    bool choose(int card);
    bool save(const std::string& path,const std::string& bundle,std::string& error) const;
    bool load(const std::string& path,const std::string& bundle,std::string& error);
    std::string serialize(const std::string& bundle) const;
    bool deserialize(const std::string& data,const std::string& bundle,std::string& error,const std::array<int64_t,kGoodIdCount>& bonded_goods={});
    int day() const {return day_;}
    int regions() const {return regions_;}
    int count() const {return int(people_.size());}
    int64_t game_tick() const {return schedule_book_.cursor;}
    bool paused() const {return paused_;}
    uint64_t pause_revision() const {return pause_revision_;}
    bool timeline_contract() const {return schedule_book_.timeline_contract;}
    // Historical realm-v6 pilots have the wrong action/observation shape and
    // cannot be reused after the canonical-goods action surface migration.
    bool pilot_policy_compatible() const {return false;}
    bool capability_active(LegacyCapability capability) const {
        return (capability_mask_&(1u<<static_cast<unsigned>(capability)))!=0;
    }
    int capability_value(uint64_t actor,LegacyCapability capability) const {
        const auto& p=person(actor);
        return capability_active(capability)?p.stats.get(capability):kInactiveCapabilityNeutralValue;
    }
    int capability_value(uint64_t actor,CapabilityId capability) const {
        const auto index=static_cast<unsigned>(capability);
        if(index<kLegacyCapabilities.size())
            return capability_value(actor,static_cast<LegacyCapability>(index));
        if(capability==CapabilityId::LaborSkill)return person(actor).capability(capability);
        throw std::out_of_range("capability ID");
    }
    uint32_t capability_mask() const {return capability_mask_;}
    std::vector<CapabilityId> capability_schema() const {
        std::vector<CapabilityId> result;result.reserve(kCapabilityCount);
        for(size_t i=0;i<kLegacyCapabilities.size();++i)
            if(capability_mask_&(1u<<i))result.push_back(static_cast<CapabilityId>(i));
        result.push_back(CapabilityId::LaborSkill);
        return result;
    }
    std::string model_contract() const {
        std::string result;
        if (schedule_book_.timeline_contract)
            result=config_.historical_defaults() ? "realm-worksite-payroll-goods30-v54" : "realm-worksite-payroll-goods30-config-v54";
        else result=config_.historical_defaults() ? "realm-household-fund-goods30-v54" : "realm-household-fund-goods30-config-v54";
        if(capability_mask_!=kDefaultCapabilityMask)result+="-caps-"+std::to_string(capability_mask_);
        return result;
    }
    const ScheduleBook& schedule_book() const {return schedule_book_;}
    int64_t debt(uint64_t actor) const;
    int64_t available_funds(uint64_t actor)const;
    // Read-only employment/job classification queries
    EmploymentState employment_state(uint64_t id) const;
    JobType job_type(uint64_t id) const;
    // nullptr when the person has no active job (None/Idle).
    const Job* job_descriptor(uint64_t id) const;
    // Sparse household membership. A household label is the ID of its founding member,
    // but membership is a dynamic record: a birth joins the parent household with a
    // fresh non-consecutive ID, so never assume four consecutive member IDs.
    uint64_t household_of(uint64_t person) const;
    std::vector<uint64_t> household_members(uint64_t household) const;
    bool same_household(uint64_t a,uint64_t b) const;
    uint64_t guardian(uint64_t person) const;
    // Representative authority of a household: the living adult founding member,
    // otherwise the oldest living adult member; 0 when the household has none.
    uint64_t household_authority(uint64_t household) const;
    size_t household_records() const;
    int64_t household_fund(uint64_t household) const;
    size_t household_fund_records() const { return household_funds_.size(); }
    std::vector<Loan> loans() const {return loans_;}
    std::vector<MarketOrder> market_orders() const {return market_orders_;}
    const std::vector<NationalTransportRoute>& national_transport_routes() const {return national_transport_routes_;}
    const std::vector<TransportShipment>& transport_shipments() const {return transport_shipments_;}
    const std::vector<TransportCargoLot>& transport_cargo_lots() const {return transport_cargo_lots_;}
    int64_t military_ration_stock(int region) const {return military_ration_depots_.at(size_t(region));}
    int64_t military_ration_stock(int region,int good) const {
        if(good==int(GoodId::DryFood))return military_ration_depots_.at(size_t(region));
        if(good==int(GoodId::SupplementaryFood))return military_supplementary_ration_depots_.at(size_t(region));
        throw std::out_of_range("military ration good");
    }
    std::vector<MilitaryUnitRoster> military_unit_rosters() const;
    std::vector<uint64_t> military_battle_candidates(int region) const;
    int64_t transport_escrow() const {return transport_escrow_;}
    MarketMonthlyTotals market_monthly_totals(int region,int good) const;
    int64_t market_escrow() const {return market_escrow_;}
    std::vector<EstateCase> estate_cases() const {return estate_cases_;}
    std::vector<Project> projects() const {return projects_;}
    std::vector<Edge> relations() const {return edges_;}
    static std::string action_name(const Candidate& c);
private:
    int64_t& military_ration_depot_stock(int region,int good) {
        if(good==int(GoodId::DryFood))return military_ration_depots_.at(size_t(region));
        if(good==int(GoodId::SupplementaryFood))return military_supplementary_ration_depots_.at(size_t(region));
        throw std::out_of_range("military ration good");
    }
    const int64_t& military_ration_depot_stock(int region,int good) const {
        if(good==int(GoodId::DryFood))return military_ration_depots_.at(size_t(region));
        if(good==int(GoodId::SupplementaryFood))return military_supplementary_ration_depots_.at(size_t(region));
        throw std::out_of_range("military ration good");
    }
    GameplayConfig config_{};
    uint32_t capability_mask_=kDefaultCapabilityMask;
    std::map<WorkRegionKey,int64_t> work_pay_remainders_;
    WorkCreditRegions work_credits_;
    int day_=0,regions_=6;uint64_t seed_=0,next_id_=1,next_fact_=1;
    std::mt19937_64 rng_;Draft draft_;
    std::vector<Person> people_;std::vector<Province> provinces_;std::vector<int64_t> military_ration_depots_,military_supplementary_ration_depots_;
    std::vector<Edge> edges_;std::vector<Loan> loans_;std::vector<Project> projects_;std::vector<Establishment> establishments_;std::vector<LandHolding> land_holdings_;std::vector<LandUseRight> land_use_rights_;std::vector<PrivateResidence> private_residences_;
    std::vector<Fact> facts_;std::vector<Knowledge> knowledge_;std::vector<Proposal> proposals_;std::vector<Apprenticeship> apprenticeships_;
    std::vector<int64_t> urban_housing_capacity_;
    std::vector<MunicipalCouncil> municipal_councils_;
    std::vector<int64_t> urban_residents_cache_,urban_arrivals_reserved_;
    std::vector<MarketOrder> market_orders_;
    std::vector<NationalTransportRoute> national_transport_routes_;
    mutable bool atlas_distance_cache_valid_=false;
    mutable std::array<std::array<int,kAtlasTerritoryCount>,kAtlasTerritoryCount> atlas_distance_cache_{};
    std::vector<TransportShipment> transport_shipments_;
    std::vector<TransportCargoLot> transport_cargo_lots_;
    std::vector<ForeignTradeOrder> foreign_trade_orders_;
    std::vector<ForeignTradeOffer> foreign_trade_offers_;
    std::vector<MilitaryObligation> military_obligations_;
    std::vector<MilitarySupplyObligation> military_supply_obligations_;
    std::vector<MilitaryRaid> military_raids_;
    std::array<ForeignTradePartner,kForeignTradePartnerCount> foreign_trade_partners_{};
    uint64_t next_transport_lot_id_=1;
    int64_t transport_escrow_=0;
    std::vector<std::array<MarketMonthlyTotals,kGoodIdCount>> market_monthly_;
    int64_t market_escrow_=0;
    int market_settled_day_=0,market_processed_month_=0;
#ifdef POLYEC_ENABLE_REALM_PHASE_PROFILE
    std::array<uint64_t, day_phase_count> day_phase_profile_ns_{};
    std::array<uint64_t, people_phase_count> people_phase_profile_ns_{};
#endif
    std::unordered_map<uint64_t,std::unordered_map<uint64_t,int>> knowledge_index_;
    // Derived owner-to-site index used by per-decision personal wealth queries.
    std::vector<std::vector<std::size_t>> personal_establishment_indices_;
    // Derived observation feature: active secret facts known by each person.
    // Rebuilt on load/day boundary; event and investigation paths keep it current.
    std::vector<uint32_t> secret_known_counts_;
    // Derived per-observer counts by KnowledgeKind: SecretEvidence, UnknownLegacy.
    std::vector<std::array<uint32_t,2>> knowledge_observation_counts_;
    // Derived household membership index: label -> ascending member IDs. It adds no
    // money and no second world; it is rebuilt from
    // Person::household_membership.household_id after any load.
    std::unordered_map<uint64_t,std::vector<uint64_t>> households_;
    // Derived family indices used by marriage eligibility; rebuilt after load
    // and incrementally extended when a birth adds a parent-child edge.
    std::unordered_map<uint64_t,std::vector<uint64_t>> parents_by_child_;
    std::unordered_map<uint64_t,std::unordered_set<uint64_t>> children_by_parent_;
    std::unordered_map<uint64_t,std::unordered_set<uint64_t>> siblings_by_person_;
    std::map<uint64_t,int64_t> household_funds_;
    std::array<std::array<bool,7>,7> routes_{};
    void rebuild_atlas_distance_cache() const;
    int person_travel_days(const Person& person,int destination) const;
    std::array<int64_t,kOrganizationCount> organization_funds_{};
    std::vector<int64_t> municipal_funds_;
    std::vector<int> municipal_last_funded_period_;
    int64_t royal_reference_=0,credit_pool_=0,restricted_fund_=0;
    int mediation_ready_=0,bonus_offices_=0;bool pact_=false,reference_scenario_=false;
    std::vector<uint64_t> referrals_;std::vector<Investigation> investigations_;
    std::array<int64_t,kGoodIdCount> opening_{},produced_{},consumed_{};
    int64_t opening_money_=0,carrier_=0,estate_=0,legacy_estate_unassigned_=0,endowment_=0;
    std::vector<EstateCase> estate_cases_;
    ScheduleBook schedule_book_;
    bool paused_ = false;
    uint64_t pause_revision_ = 0;
    Person& mutable_person(uint64_t id);
    bool manages_establishment(const Person& actor,const Establishment& site) const;
    void migrate_work_credits();
    bool work_credit_consistent() const;
    bool checked_total_money(int64_t& total) const;
    uint64_t guardian_of(const Person& person)const;
    void index_household(uint64_t household,uint64_t person);
    void reindex_households();
    void index_kinship_edge(const Edge& edge);
    void rebuild_kinship_indices();
    void rebuild_personal_establishment_index();
    double personal_enterprise_equity(uint64_t actor) const;
    bool households_consistent()const;
    void spend(Person& person,int64_t& destination,int64_t amount);
    void payroll(int region,bool public_employer,const std::vector<std::pair<uint64_t,int64_t>>& claims);
    void sales_tax(Province& province,int64_t gross);
    int64_t harvest_quantity(int region,int good,int residents,int weather=-1,int productivity=100)const;
    void initialize_court_reference();
    void apply_augment(int card);
    int report_delay(bool inventory=false)const;
    int office_capacity(uint64_t actor)const;
    int office_used(uint64_t actor)const;
    bool project_materials(int region,int kind)const;
    void start_project(uint64_t owner,int region,int kind,int64_t budget,int term,bool from_escrow=false,uint64_t cause=0);
    void finish_projects();
    bool legal_proposal_response(uint64_t actor,const Candidate& action)const;
    bool military_service_eligible(const Person& person)const;
    std::vector<uint64_t> eligible_muster_members(uint64_t commander,int region,int required_until)const;
    void end_military_service(Person& person,int fact_kind);
    bool marriage_eligible(uint64_t actor)const;
    bool marriage_related(uint64_t first,uint64_t second)const;
    bool marriage_pending(uint64_t first,uint64_t second,uint64_t except=0)const;
    void create_proposal(uint64_t actor,const Candidate& action);
    void resolve_proposal(uint64_t actor,const Candidate& action);
    void refund_proposal(Proposal& proposal);
    void write_court(std::ostream& stream)const;
    bool read_court(std::istream& stream,int schema);
    bool legal(uint64_t actor,const Candidate& c,std::string& reason) const;
    void refresh_demography();
    void initialize_urban_housing_baseline();
    void day_begin();
    void day_body();
    void day_payroll();
    void day_people();
    void day_apprenticeships();
    void process_private_housing();
    void settle_market_day();
    void cancel_market_orders(uint64_t owner);
    bool place_essential_market_order(Person& person,Province& province);
    bool update_monthly_market_prices();
    void day_production();
    // Staged establishment production under the timeline contract (Gate 4):
    // pure planner plus full revalidation and single-transaction commit.
    void produce_with_establishments();
    void day_harvest();
    void day_investigations();
    void day_treasury();
    void day_loans();
    void day_proposals();
    void day_births();
    void day_end();
    void day_settlement();
    void process_transport_day();
    void process_foreign_trade_day();
    void process_foreign_trade_offer_expirations();
    void resolve_foreign_trade_offer(ForeignTradeOffer& offer,ForeignTradeOfferStatus outcome);
    void process_military_obligations_day();
    void process_military_supply_obligations_day();
    NationalTransportPathPlan foreign_trade_route_plan(ForeignTradeOperation operation,int64_t quantity) const;
    bool transport_waiting_units_by_link(std::array<int64_t,kAtlasLinkCount>& waiting) const;
    void settle_estate_if_transport_complete(uint64_t decedent);
    void prices();void event(uint64_t actor,int region,int kind,int64_t amount=0,uint64_t target=0,bool secret=false,uint64_t parent=0);
    void record_unmet_essential_need(Person& person);
    void advance_ticks_preserving_paused(int64_t ticks);
    void transfer(int64_t& from,int64_t& to,int64_t amount);
    bool knows(uint64_t observer,const Fact& f) const;
    void rebuild_secret_known_counts();
};
}
