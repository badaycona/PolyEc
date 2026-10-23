#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace polyec {

// Stable high-level groups from IMPLEMENTATION_PLAN.md §15.2. Groups describe
// categories; GoodId distinguishes products that must not be substituted.
enum class GoodGroup : uint8_t {
    DryFood = 0,
    SupplementaryFood = 1,
    Salt = 2,
    Timber = 3,
    Fuel = 4,
    Iron = 5,
    NonferrousMetal = 6,
    Fiber = 7,
    Cloth = 8,
    Livestock = 9,
    Tools = 10,
    MilitarySupply = 11,
    ConstructionMaterial = 12,
    Paper = 13,
    Medicine = 14,
    Luxury = 15
};

// Flattened product IDs. Product IDs are save identities, not UI action slots.
enum class GoodId : uint8_t {
    DryFood = 0,
    SupplementaryFood = 1,
    Salt = 2,
    Timber = 3,
    Fuel = 4,
    Iron = 5,
    Copper = 6,
    Silver = 7,
    Gold = 8,
    Fiber = 9,
    Beeswax = 10,
    Cloth = 11,
    Silk = 12,
    Livestock = 13,
    Horse = 14,
    WarHorse = 15,
    Tools = 16,
    MilitarySupply = 17,
    Weapon = 18,
    ConstructionMaterial = 19,
    Paper = 20,
    Medicine = 21,
    Tea = 22,
    Incense = 23,
    DecorativeGlass = 24,
    FinePorcelain = 25,
    GeneralLuxury = 26,
    PlainGlass = 27,
    Pottery = 28,
    Leather = 29
};

inline constexpr int kGoodGroupCount = 16;
inline constexpr int kGoodIdCount = 30;
inline constexpr int kLegacyGoodCount = 7;

struct GoodDescriptor {
    GoodId id;
    GoodGroup group;
    const char* name;
};

inline constexpr std::array<GoodDescriptor, kGoodIdCount> kGoodCatalog{{
    {GoodId::DryFood, GoodGroup::DryFood, "dry_food"},
    {GoodId::SupplementaryFood, GoodGroup::SupplementaryFood, "supplementary_food"},
    {GoodId::Salt, GoodGroup::Salt, "salt"},
    {GoodId::Timber, GoodGroup::Timber, "timber"},
    {GoodId::Fuel, GoodGroup::Fuel, "fuel"},
    {GoodId::Iron, GoodGroup::Iron, "iron"},
    {GoodId::Copper, GoodGroup::NonferrousMetal, "copper"},
    {GoodId::Silver, GoodGroup::NonferrousMetal, "silver"},
    {GoodId::Gold, GoodGroup::NonferrousMetal, "gold"},
    {GoodId::Fiber, GoodGroup::Fiber, "fiber"},
    {GoodId::Beeswax, GoodGroup::Fiber, "beeswax"},
    {GoodId::Cloth, GoodGroup::Cloth, "cloth"},
    {GoodId::Silk, GoodGroup::Cloth, "silk"},
    {GoodId::Livestock, GoodGroup::Livestock, "livestock"},
    {GoodId::Horse, GoodGroup::Livestock, "horse"},
    {GoodId::WarHorse, GoodGroup::Livestock, "war_horse"},
    {GoodId::Tools, GoodGroup::Tools, "tools"},
    {GoodId::MilitarySupply, GoodGroup::MilitarySupply, "military_supply"},
    {GoodId::Weapon, GoodGroup::MilitarySupply, "weapon"},
    {GoodId::ConstructionMaterial, GoodGroup::ConstructionMaterial, "construction_material"},
    {GoodId::Paper, GoodGroup::Paper, "paper"},
    {GoodId::Medicine, GoodGroup::Medicine, "medicine"},
    {GoodId::Tea, GoodGroup::Luxury, "tea"},
    {GoodId::Incense, GoodGroup::Luxury, "incense"},
    {GoodId::DecorativeGlass, GoodGroup::Luxury, "decorative_glass"},
    {GoodId::FinePorcelain, GoodGroup::Luxury, "fine_porcelain"},
    {GoodId::GeneralLuxury, GoodGroup::Luxury, "general_luxury"},
    {GoodId::PlainGlass, GoodGroup::ConstructionMaterial, "plain_glass"},
    {GoodId::Pottery, GoodGroup::ConstructionMaterial, "pottery"},
    {GoodId::Leather, GoodGroup::Livestock, "leather"}
}};

// Historical Realm action and M4 cargo slots: food, fiber, cloth, iron,
// fuel, tools, horses. Keep this mapping at compatibility boundaries only.
inline constexpr std::array<GoodId, kLegacyGoodCount> kLegacyGoodToCanonical{{
    GoodId::DryFood, GoodId::Fiber, GoodId::Cloth, GoodId::Iron,
    GoodId::Fuel, GoodId::Tools, GoodId::Horse
}};

// Initial accounting reference values. Legacy product values preserve the
// original seven-slot pilot references; additional products are uncalibrated.
inline constexpr std::array<int, kGoodIdCount> kGoodReferencePrice{{
    20, 28, 35, 45, 18, 45, 65, 100, 120, 14, 40, 35, 120, 50,
    90, 200, 65, 90, 110, 55, 60, 130, 100, 160, 250, 300, 150,
    90, 55, 80
}};

constexpr bool good_catalog_is_well_formed() noexcept {
    std::array<size_t, kGoodGroupCount> products_per_group{};
    for (size_t i = 0; i < kGoodCatalog.size(); ++i) {
        if (static_cast<size_t>(kGoodCatalog[i].id) != i) return false;
        const auto group = static_cast<size_t>(kGoodCatalog[i].group);
        if (group >= kGoodGroupCount) return false;
        ++products_per_group[group];
        const std::string_view name = kGoodCatalog[i].name ? kGoodCatalog[i].name : "";
        if (name.empty()) return false;
        for (size_t prior = 0; prior < i; ++prior)
            if (name == kGoodCatalog[prior].name) return false;
        if (kGoodReferencePrice[i] <= 0) return false;
    }
    for (size_t count : products_per_group) if (count == 0) return false;
    for (size_t i = 0; i < kLegacyGoodToCanonical.size(); ++i) {
        const auto good = static_cast<size_t>(kLegacyGoodToCanonical[i]);
        if (good >= kGoodCatalog.size()) return false;
        for (size_t j = 0; j < i; ++j)
            if (kLegacyGoodToCanonical[j] == kLegacyGoodToCanonical[i]) return false;
    }
    return true;
}
static_assert(good_catalog_is_well_formed());

constexpr bool valid_good_id(int value) noexcept {
    return value >= 0 && value < kGoodIdCount;
}

constexpr GoodGroup good_group(GoodId id) noexcept {
    const auto index = static_cast<unsigned>(id);
    return index < kGoodCatalog.size() ? kGoodCatalog[index].group : GoodGroup::DryFood;
}

constexpr int canonical_good_index(GoodId id) noexcept {
    return static_cast<int>(id);
}

constexpr int canonical_from_legacy_good(int legacy_index) noexcept {
    return legacy_index >= 0 && legacy_index < kLegacyGoodCount
        ? static_cast<int>(kLegacyGoodToCanonical[static_cast<size_t>(legacy_index)]) : -1;
}

} // namespace polyec
