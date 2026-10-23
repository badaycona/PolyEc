#pragma once

#include "polyec/good_catalog.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace polyec {

inline constexpr size_t kAtlasTerritoryCount = 27;
inline constexpr size_t kAtlasVassalCount = 24;
inline constexpr size_t kAtlasRoyalCount = 3;
inline constexpr size_t kAtlasLinkCount = 31;

struct AtlasTerritory {
    uint8_t id;
    std::string_view name;
    std::string_view region_group;
    std::optional<uint32_t> population_thousands;
    std::optional<uint32_t> area_square_kilometers;
    std::string_view specialty;
    std::string_view advantages_and_constraints;
    bool royal_domain;
};

// Population and area values for L01-L24 are atlas context only. They do not
// define the number of simulated Person records. Royal-domain demographics
// have not been specified and therefore remain absent in this reference catalog.
inline constexpr std::array<AtlasTerritory, kAtlasTerritoryCount> kWorldAtlas{{
    {0,  "Thanh Điền",   "lưu vực",    420,  8000, "Gạo nước",       "Đất màu, sông lớn; lũ và quyền thủy lợi tập trung", false},
    {1,  "Liên Trạch",   "lưu vực",    210,  5000, "Cá khô",         "Đầm hồ, vận tải thủy; bệnh và thiếu đất khô", false},
    {2,  "Phù Sa",       "lưu vực",    310,  7000, "Dầu hạt",       "Ruộng luân canh, gần chợ; phụ thuộc đê điều", false},
    {3,  "Đông Viên",    "lưu vực",    260,  6000, "Vải gai",        "Đông dân, vùng nguyên liệu; năng suất ruộng thấp", false},
    {4,  "Bạch Khâu",    "cao nguyên", 150, 14000, "Len trắng",      "Đồng cỏ; thiếu gạo và thợ dệt", false},
    {5,  "Hắc Sơn",      "cao nguyên", 110, 11000, "Sắt rèn",        "Quặng tốt; đường đèo, nhập lương thực", false},
    {6,  "Tùng Lĩnh",    "cao nguyên",  95, 13000, "Gỗ đóng tàu",   "Rừng, sông thả gỗ; tái sinh rừng chậm", false},
    {7,  "Xích Nham",    "cao nguyên",  80,  9000, "Đồng thỏi",     "Mỏ đồng; cần nhiên liệu và vốn", false},
    {8,  "Nam Cảng",     "duyên hải",  280,  3000, "Muối biển",     "Cảng lớn; phụ thuộc gạo nhập và an ninh biển", false},
    {9,  "Hải Châu",     "duyên hải",  190,  4000, "Tàu đi biển",   "Thợ tàu, bến sâu; cần gỗ, sắt, vải buồm", false},
    {10, "Bạch Sa",      "duyên hải",  130,  4500, "Thủy tinh thường", "Nguồn silica hữu hạn; lò cần nhiên liệu", false},
    {11, "Trầm Hải",     "duyên hải",  120,  6000, "Trầm hương",    "Rừng đặc hữu, hàng giá trị cao; cầu xa xỉ biến động", false},
    {12, "Vân Lụa",      "đông lộ",    240,  6000, "Lụa nhuộm",     "Thợ và mạng khách hàng; thương hội hạn chế gia nhập", false},
    {13, "Cẩm Giang",    "đông lộ",    230,  5500, "Giấy tốt",      "Sông, kỹ năng; tranh nước với ruộng và xưởng", false},
    {14, "Hồng Diệp",    "đông lộ",    160,  8000, "Trà",           "Khí hậu, thương hiệu; mùa vụ và phí đường", false},
    {15, "Bích Lò",      "đông lộ",    200,  5000, "Đồ gốm thường", "Nguồn đất sét hữu hạn; lò cần nhiên liệu", false},
    {16, "Bắc Mã",       "biên bắc",   90,  20000, "Ngựa chiến",    "Đồng cỏ, quân sự; dân thưa, phòng thủ đắt", false},
    {17, "Phong Ải",     "biên bắc",  120,  15000, "Da thuộc",      "Phụ phẩm da hữu hạn; không làm chết cá thể mô phỏng", false},
    {18, "Hoàng Nguyên", "biên bắc",  170,  18000, "Lúa mạch",      "Ruộng rộng; khô hạn và xa thị trường", false},
    {19, "Thiết Quan",   "biên bắc",  140,  10000, "Binh khí",      "Thợ quân giới, vị trí phòng thủ; lệ thuộc sắt và đơn đặt hàng", false},
    {20, "Dược Cốc",     "tây lộ",    100,   9000, "Dược liệu",     "Khí hậu đa dạng; khó tiếp cận, chất lượng không đồng đều", false},
    {21, "Lâm Mật",      "tây lộ",    115,  10000, "Sáp ong",       "Rừng và sản phẩm phụ; quyền sử dụng rừng tranh chấp", false},
    {22, "Thạch Kiều",   "tây lộ",    180,   7000, "Đá xây",        "Mỏ và nút giao; hàng nặng, phụ thuộc công trình", false},
    {23, "Ngân Khê",     "tây lộ",     85,   8000, "Bạc",           "Khoáng sản giá trị cao; dễ lệ thuộc thu nhập mỏ", false},
    {24, "Kinh Kỳ",      "triều đình", std::nullopt, std::nullopt, "Hành chính và tiêu dùng", "Khu triều đình; dân số và sản xuất tính trong tổng quốc gia nhưng hạch toán riêng", true},
    {25, "Hoàng Điền",   "triều đình", std::nullopt, std::nullopt, "Ruộng và kho dự trữ", "Khu triều đình; dân số và sản xuất tính trong tổng quốc gia nhưng hạch toán riêng", true},
    {26, "Trực Lộ",      "triều đình", std::nullopt, std::nullopt, "Hậu cần và xưởng nhà nước", "Khu triều đình; dân số và sản xuất tính trong tổng quốc gia nhưng hạch toán riêng", true}
}};

enum class AtlasSpecialtyKind : uint8_t {
    Good,
    TransportAsset,
    Unresolved,
};

struct AtlasSpecialtyMapping {
    uint8_t territory_id;
    AtlasSpecialtyKind kind;
    std::optional<GoodGroup> group;
    std::optional<GoodId> product;
};

// Group-only records deliberately leave the exact product unset. The ship is
// a transport asset. Plain glass, pottery and leather have distinct goods;
// their IDs do not imply substitution with decorative glass, fine porcelain,
// or livestock.
inline constexpr std::array<AtlasSpecialtyMapping, kAtlasVassalCount> kAtlasSpecialtyMappings{{
    {0,  AtlasSpecialtyKind::Good, GoodGroup::DryFood, std::nullopt},
    {1,  AtlasSpecialtyKind::Good, GoodGroup::DryFood, std::nullopt},
    {2,  AtlasSpecialtyKind::Good, GoodGroup::SupplementaryFood, std::nullopt},
    {3,  AtlasSpecialtyKind::Good, GoodGroup::Cloth, std::nullopt},
    {4,  AtlasSpecialtyKind::Good, GoodGroup::Fiber, std::nullopt},
    {5,  AtlasSpecialtyKind::Good, GoodGroup::Iron, GoodId::Iron},
    {6,  AtlasSpecialtyKind::Good, GoodGroup::Timber, GoodId::Timber},
    {7,  AtlasSpecialtyKind::Good, GoodGroup::NonferrousMetal, GoodId::Copper},
    {8,  AtlasSpecialtyKind::Good, GoodGroup::Salt, GoodId::Salt},
    {9,  AtlasSpecialtyKind::TransportAsset, std::nullopt, std::nullopt},
    {10, AtlasSpecialtyKind::Good, GoodGroup::ConstructionMaterial, GoodId::PlainGlass},
    {11, AtlasSpecialtyKind::Good, GoodGroup::Luxury, GoodId::Incense},
    {12, AtlasSpecialtyKind::Good, GoodGroup::Cloth, GoodId::Silk},
    {13, AtlasSpecialtyKind::Good, GoodGroup::Paper, GoodId::Paper},
    {14, AtlasSpecialtyKind::Good, GoodGroup::Luxury, GoodId::Tea},
    {15, AtlasSpecialtyKind::Good, GoodGroup::ConstructionMaterial, GoodId::Pottery},
    {16, AtlasSpecialtyKind::Good, GoodGroup::Livestock, GoodId::WarHorse},
    {17, AtlasSpecialtyKind::Good, GoodGroup::Livestock, GoodId::Leather},
    {18, AtlasSpecialtyKind::Good, GoodGroup::DryFood, std::nullopt},
    {19, AtlasSpecialtyKind::Good, GoodGroup::MilitarySupply, GoodId::Weapon},
    {20, AtlasSpecialtyKind::Good, GoodGroup::Medicine, std::nullopt},
    {21, AtlasSpecialtyKind::Good, GoodGroup::Fiber, GoodId::Beeswax},
    {22, AtlasSpecialtyKind::Good, GoodGroup::ConstructionMaterial, std::nullopt},
    {23, AtlasSpecialtyKind::Good, GoodGroup::NonferrousMetal, GoodId::Silver},
}};

constexpr const AtlasSpecialtyMapping* atlas_specialty_mapping(uint8_t territory_id) noexcept {
    for (const auto& mapping : kAtlasSpecialtyMappings)
        if (mapping.territory_id == territory_id) return &mapping;
    return nullptr;
}

template <size_t MappingCount>
constexpr bool atlas_specialty_mappings_are_well_formed(
    const std::array<AtlasSpecialtyMapping, MappingCount>& mappings) noexcept {
    if (MappingCount != kAtlasVassalCount) return false;
    for (size_t i = 0; i < mappings.size(); ++i) {
        const auto& mapping = mappings[i];
        if (mapping.territory_id >= kAtlasVassalCount ||
            mapping.kind > AtlasSpecialtyKind::Unresolved) return false;
        if (mapping.kind == AtlasSpecialtyKind::Good) {
            if (!mapping.group || static_cast<size_t>(*mapping.group) >= kGoodGroupCount) return false;
        } else if (mapping.group || mapping.product) {
            return false;
        }
        if (mapping.product) {
            const auto product_id = static_cast<size_t>(*mapping.product);
            if (mapping.kind != AtlasSpecialtyKind::Good || product_id >= kGoodCatalog.size() ||
                good_group(*mapping.product) != *mapping.group)
                return false;
        }
        for (size_t prior = 0; prior < i; ++prior)
            if (mappings[prior].territory_id == mapping.territory_id) return false;
    }
    return true;
}
constexpr bool atlas_specialty_mappings_are_well_formed() noexcept {
    return atlas_specialty_mappings_are_well_formed(kAtlasSpecialtyMappings);
}
static_assert(atlas_specialty_mappings_are_well_formed());

// Minimum atlas links from IMPLEMENTATION_PLAN.md §12.3. These are map-data
// corridors only; they do not yet define travel time, capacity, cost, safety,
// sovereignty, tolls, seasonality, coordinates, or an operational trade route.
enum class AtlasCorridor : uint8_t {
    GreatRiver,
    RoadImprovement,
    Coastal,
    East,
    North,
    West,
    RoyalDomains,
};

enum class AtlasRouteMode : uint8_t {
    Unspecified,
    River,
    Road,
    CoastalShipping,
};

enum class AtlasLinkDirection : uint8_t {
    Unspecified,
    Directed,
    Bidirectional,
};

struct AtlasLink {
    uint8_t from;
    uint8_t to;
    AtlasCorridor corridor;
    AtlasRouteMode mode;
    AtlasLinkDirection direction;
};

struct AtlasTopologyPath {
    std::array<uint8_t, kAtlasTerritoryCount> territories{};
    uint8_t size = 0;
    bool reachable = false;
};

constexpr bool atlas_link_allows_direction(const AtlasLink& link,
                                           uint8_t from,
                                           uint8_t to) noexcept {
    if (link.direction == AtlasLinkDirection::Directed)
        return link.from == from && link.to == to;
    if (link.direction == AtlasLinkDirection::Bidirectional)
        return (link.from == from && link.to == to) ||
               (link.from == to && link.to == from);
    return false;
}

constexpr AtlasTopologyPath atlas_topological_path(uint8_t from,
                                                   uint8_t to) noexcept;
constexpr AtlasTopologyPath atlas_topological_path(
    uint8_t from, uint8_t to,
    const std::array<bool, kAtlasLinkCount>& unavailable_links) noexcept;

// Territory IDs match their positions in kWorldAtlas: L01-L24 are 0-23,
// Kinh Kỳ/Hoàng Điền/Trực Lộ are 24-26. Direction in the source route list is
// retained for the river corridor; it is not a claim about all-season travel.
inline constexpr std::array<AtlasLink, kAtlasLinkCount> kWorldAtlasLinks{{
    {0, 2, AtlasCorridor::GreatRiver, AtlasRouteMode::River, AtlasLinkDirection::Directed},
    {2, 1, AtlasCorridor::GreatRiver, AtlasRouteMode::River, AtlasLinkDirection::Directed},
    {1, 8, AtlasCorridor::GreatRiver, AtlasRouteMode::River, AtlasLinkDirection::Directed},
    {5, 4, AtlasCorridor::RoadImprovement, AtlasRouteMode::Road, AtlasLinkDirection::Bidirectional},
    {4, 0, AtlasCorridor::RoadImprovement, AtlasRouteMode::Road, AtlasLinkDirection::Bidirectional},
    {8, 9, AtlasCorridor::Coastal, AtlasRouteMode::CoastalShipping, AtlasLinkDirection::Bidirectional},
    {9, 10, AtlasCorridor::Coastal, AtlasRouteMode::CoastalShipping, AtlasLinkDirection::Bidirectional},
    {10, 11, AtlasCorridor::Coastal, AtlasRouteMode::CoastalShipping, AtlasLinkDirection::Bidirectional},
    {0, 3, AtlasCorridor::East, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {3, 12, AtlasCorridor::East, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {12, 13, AtlasCorridor::East, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {13, 14, AtlasCorridor::East, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {14, 15, AtlasCorridor::East, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {15, 9, AtlasCorridor::East, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {4, 16, AtlasCorridor::North, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {16, 17, AtlasCorridor::North, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {17, 18, AtlasCorridor::North, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {18, 19, AtlasCorridor::North, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {19, 5, AtlasCorridor::North, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {5, 6, AtlasCorridor::West, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {6, 7, AtlasCorridor::West, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {7, 23, AtlasCorridor::West, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {23, 22, AtlasCorridor::West, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {22, 20, AtlasCorridor::West, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {20, 21, AtlasCorridor::West, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {21, 2, AtlasCorridor::West, AtlasRouteMode::Unspecified, AtlasLinkDirection::Bidirectional},
    {24, 0, AtlasCorridor::RoyalDomains, AtlasRouteMode::Unspecified, AtlasLinkDirection::Unspecified},
    {24, 26, AtlasCorridor::RoyalDomains, AtlasRouteMode::Unspecified, AtlasLinkDirection::Unspecified},
    {25, 0, AtlasCorridor::RoyalDomains, AtlasRouteMode::Unspecified, AtlasLinkDirection::Unspecified},
    {25, 2, AtlasCorridor::RoyalDomains, AtlasRouteMode::Unspecified, AtlasLinkDirection::Unspecified},
    {26, 19, AtlasCorridor::RoyalDomains, AtlasRouteMode::Unspecified, AtlasLinkDirection::Unspecified},
}};

// Finds a map-topology connection using only links whose direction is known.
// Route mode, passability, time, capacity, cost, safety and sovereignty are
// deliberately outside this result; this is not a logistics route planner.
constexpr AtlasTopologyPath atlas_topological_path(uint8_t from,
                                                   uint8_t to,
                                                   const std::array<bool, kAtlasLinkCount>& unavailable_links) noexcept {
    AtlasTopologyPath result;
    if (from >= kAtlasTerritoryCount || to >= kAtlasTerritoryCount) return result;
    if (from == to) {
        result.territories[0] = from;
        result.size = 1;
        result.reachable = true;
        return result;
    }

    std::array<int8_t, kAtlasTerritoryCount> parent{};
    parent.fill(-1);
    std::array<uint8_t, kAtlasTerritoryCount> queue{};
    size_t head = 0, tail = 0;
    parent[from] = static_cast<int8_t>(from);
    queue[tail++] = from;
    while (head < tail && parent[to] < 0) {
        const uint8_t current = queue[head++];
        for (size_t link_index = 0; link_index < kWorldAtlasLinks.size(); ++link_index) {
            if (unavailable_links[link_index]) continue;
            const auto& link = kWorldAtlasLinks[link_index];
            uint8_t next = kAtlasTerritoryCount;
            if (link.from == current &&
                (link.direction == AtlasLinkDirection::Directed ||
                 link.direction == AtlasLinkDirection::Bidirectional)) {
                next = link.to;
            } else if (link.to == current &&
                       link.direction == AtlasLinkDirection::Bidirectional) {
                next = link.from;
            }
            if (next < kAtlasTerritoryCount && parent[next] < 0) {
                parent[next] = static_cast<int8_t>(current);
                queue[tail++] = next;
            }
        }
    }
    if (parent[to] < 0) return result;

    std::array<uint8_t, kAtlasTerritoryCount> reverse{};
    size_t count = 0;
    for (int current = to; current != from; current = parent[size_t(current)])
        reverse[count++] = static_cast<uint8_t>(current);
    reverse[count++] = from;
    for (size_t i = 0; i < count; ++i) result.territories[i] = reverse[count - 1 - i];
    result.size = static_cast<uint8_t>(count);
    result.reachable = true;
    return result;
}

constexpr AtlasTopologyPath atlas_topological_path(uint8_t from, uint8_t to) noexcept {
    return atlas_topological_path(from, to, std::array<bool, kAtlasLinkCount>{});
}

constexpr bool world_atlas_links_are_well_formed() noexcept {
    for (size_t i = 0; i < kWorldAtlasLinks.size(); ++i) {
        const auto& link = kWorldAtlasLinks[i];
        if (link.from >= kAtlasTerritoryCount || link.to >= kAtlasTerritoryCount ||
            link.from == link.to || link.corridor > AtlasCorridor::RoyalDomains ||
            link.mode > AtlasRouteMode::CoastalShipping ||
            link.direction > AtlasLinkDirection::Bidirectional) return false;
        if (link.corridor == AtlasCorridor::GreatRiver &&
            (link.mode != AtlasRouteMode::River || link.direction != AtlasLinkDirection::Directed)) return false;
        if (link.corridor == AtlasCorridor::RoadImprovement &&
            (link.mode != AtlasRouteMode::Road || link.direction != AtlasLinkDirection::Bidirectional)) return false;
        if (link.corridor == AtlasCorridor::Coastal &&
            (link.mode != AtlasRouteMode::CoastalShipping || link.direction != AtlasLinkDirection::Bidirectional)) return false;
        if (link.corridor >= AtlasCorridor::East && link.mode != AtlasRouteMode::Unspecified)
            return false;
        for (size_t prior = 0; prior < i; ++prior) {
            const auto& other = kWorldAtlasLinks[prior];
            if (link.corridor == other.corridor && link.from == other.from &&
                link.to == other.to) return false;
        }
    }
    return true;
}
static_assert(world_atlas_links_are_well_formed());

constexpr bool world_atlas_is_well_formed() noexcept {
    size_t vassals = 0;
    size_t royal = 0;
    std::array<size_t, 6> groups{};
    for (size_t i = 0; i < kWorldAtlas.size(); ++i) {
        const auto& territory = kWorldAtlas[i];
        if (territory.id != i || territory.name.empty() || territory.specialty.empty() ||
            territory.advantages_and_constraints.empty()) return false;
        for (size_t prior = 0; prior < i; ++prior)
            if (kWorldAtlas[prior].name == territory.name) return false;
        if (territory.royal_domain) {
            ++royal;
            if (i < kAtlasVassalCount || territory.population_thousands.has_value() ||
                territory.area_square_kilometers.has_value() || territory.region_group != "triều đình") return false;
        } else {
            ++vassals;
            if (i >= kAtlasVassalCount || !territory.population_thousands ||
                !territory.area_square_kilometers || *territory.population_thousands == 0 ||
                *territory.area_square_kilometers == 0) return false;
            size_t group = 6;
            if (territory.region_group == "lưu vực") group = 0;
            else if (territory.region_group == "cao nguyên") group = 1;
            else if (territory.region_group == "duyên hải") group = 2;
            else if (territory.region_group == "đông lộ") group = 3;
            else if (territory.region_group == "biên bắc") group = 4;
            else if (territory.region_group == "tây lộ") group = 5;
            if (group == 6) return false;
            ++groups[group];
        }
    }
    if (vassals != kAtlasVassalCount || royal != kAtlasRoyalCount) return false;
    for (size_t count : groups) if (count != 4) return false;
    return true;
}
static_assert(world_atlas_is_well_formed());

} // namespace polyec
