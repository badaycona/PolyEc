#include "polyec/world_atlas.hpp"
#include "polyec/atlas_cartography.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>

int main() {
    if (!polyec::atlas_map_catalog_is_well_formed() ||
        polyec::kAtlasMapPolygons.size() != polyec::kWorldAtlas.size()) return 26;
    for (std::size_t i = 0; i < polyec::kWorldAtlas.size(); ++i) {
        const auto& geometry = polyec::kAtlasMapPolygons[i];
        if (geometry.territory_id != polyec::kWorldAtlas[i].id ||
            geometry.label_anchor.x != (geometry.vertices[0].x + geometry.vertices[3].x) / 2.0 ||
            geometry.label_anchor.y != (geometry.vertices[0].y + geometry.vertices[3].y) / 2.0)
            return 27;
        for (std::size_t edge = 0; edge < geometry.vertices.size(); ++edge) {
            const auto& a = geometry.vertices[edge];
            const auto& b = geometry.vertices[(edge + 1) % geometry.vertices.size()];
            const double side = (b.x - a.x) * (geometry.label_anchor.y - a.y) -
                                (b.y - a.y) * (geometry.label_anchor.x - a.x);
            if (side <= 0.0) return 28;
            for (std::size_t other_edge = edge + 1; other_edge < geometry.vertices.size(); ++other_edge) {
                const std::size_t next_edge = (other_edge + 1) % geometry.vertices.size();
                if (other_edge == edge || other_edge == (edge + 1) % geometry.vertices.size() ||
                    next_edge == edge) continue;
                const auto& c = geometry.vertices[other_edge];
                const auto& d = geometry.vertices[next_edge];
                const auto cross = [](const auto& p, const auto& q, const auto& r) {
                    return (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
                };
                const double ab_c = cross(a, b, c), ab_d = cross(a, b, d);
                const double cd_a = cross(c, d, a), cd_b = cross(c, d, b);
                if (((ab_c < 0 && ab_d > 0) || (ab_c > 0 && ab_d < 0)) &&
                    ((cd_a < 0 && cd_b > 0) || (cd_a > 0 && cd_b < 0))) return 30;
            }
        }
        for (std::size_t other = 0; other < polyec::kAtlasMapPolygons.size(); ++other) {
            if (other == i) continue;
            const auto& foreign_anchor = polyec::kAtlasMapPolygons[other].label_anchor;
            bool inside = true;
            for (std::size_t edge = 0; edge < geometry.vertices.size(); ++edge) {
                const auto& a = geometry.vertices[edge];
                const auto& b = geometry.vertices[(edge + 1) % geometry.vertices.size()];
                const double side = (b.x - a.x) * (foreign_anchor.y - a.y) -
                                    (b.y - a.y) * (foreign_anchor.x - a.x);
                inside &= side > 0.0;
            }
            if (inside) return 29;
        }
    }
    if (!polyec::world_atlas_is_well_formed()) return 1;
    if (!polyec::atlas_specialty_mappings_are_well_formed()) return 11;
    if (polyec::kWorldAtlas.size() != 27 || polyec::kAtlasVassalCount != 24 ||
        polyec::kAtlasRoyalCount != 3) return 2;
    if (polyec::kAtlasSpecialtyMappings.size() != 24) return 12;
    constexpr std::array<polyec::AtlasSpecialtyMapping, 24> expected_specialties{{
        {0, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::DryFood, std::nullopt},
        {1, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::DryFood, std::nullopt},
        {2, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::SupplementaryFood, std::nullopt},
        {3, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Cloth, std::nullopt},
        {4, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Fiber, std::nullopt},
        {5, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Iron, polyec::GoodId::Iron},
        {6, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Timber, polyec::GoodId::Timber},
        {7, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::NonferrousMetal, polyec::GoodId::Copper},
        {8, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Salt, polyec::GoodId::Salt},
        {9, polyec::AtlasSpecialtyKind::TransportAsset, std::nullopt, std::nullopt},
        {10, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::ConstructionMaterial, polyec::GoodId::PlainGlass},
        {11, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Luxury, polyec::GoodId::Incense},
        {12, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Cloth, polyec::GoodId::Silk},
        {13, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Paper, polyec::GoodId::Paper},
        {14, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Luxury, polyec::GoodId::Tea},
        {15, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::ConstructionMaterial, polyec::GoodId::Pottery},
        {16, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Livestock, polyec::GoodId::WarHorse},
        {17, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Livestock, polyec::GoodId::Leather},
        {18, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::DryFood, std::nullopt},
        {19, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::MilitarySupply, polyec::GoodId::Weapon},
        {20, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Medicine, std::nullopt},
        {21, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::Fiber, polyec::GoodId::Beeswax},
        {22, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::ConstructionMaterial, std::nullopt},
        {23, polyec::AtlasSpecialtyKind::Good, polyec::GoodGroup::NonferrousMetal, polyec::GoodId::Silver},
    }};
    constexpr std::array<std::string_view, 24> expected_specialty_names{
        "Gạo nước", "Cá khô", "Dầu hạt", "Vải gai", "Len trắng", "Sắt rèn",
        "Gỗ đóng tàu", "Đồng thỏi", "Muối biển", "Tàu đi biển", "Thủy tinh thường",
        "Trầm hương", "Lụa nhuộm", "Giấy tốt", "Trà", "Đồ gốm thường", "Ngựa chiến",
        "Da thuộc", "Lúa mạch", "Binh khí", "Dược liệu", "Sáp ong", "Đá xây", "Bạc"};
    std::size_t good_count = 0, asset_count = 0, unresolved_count = 0, product_count = 0;
    for (std::size_t i = 0; i < expected_specialties.size(); ++i) {
        const auto& actual = polyec::kAtlasSpecialtyMappings[i];
        const auto& expected = expected_specialties[i];
        if (actual.territory_id != expected.territory_id || actual.kind != expected.kind ||
            actual.group != expected.group ||
            actual.product != expected.product ||
            polyec::atlas_specialty_mapping(expected.territory_id) != &actual ||
            polyec::kWorldAtlas[expected.territory_id].specialty != expected_specialty_names[i]) return 13;
        if (actual.kind == polyec::AtlasSpecialtyKind::Good) ++good_count;
        else if (actual.kind == polyec::AtlasSpecialtyKind::TransportAsset) ++asset_count;
        else ++unresolved_count;
        if (actual.product) ++product_count;
    }
    if (good_count != 23 || asset_count != 1 || unresolved_count != 0 || product_count != 15) return 16;
    auto duplicate_specialty = expected_specialties;
    duplicate_specialty[1].territory_id = duplicate_specialty[0].territory_id;
    auto royal_domain_specialty = expected_specialties;
    royal_domain_specialty[0].territory_id = static_cast<std::uint8_t>(polyec::kAtlasVassalCount);
    auto invalid_product_specialty = expected_specialties;
    invalid_product_specialty[5].product = static_cast<polyec::GoodId>(255);
    auto mismatched_group_specialty = expected_specialties;
    mismatched_group_specialty[5].group = polyec::GoodGroup::Fiber;
    if (polyec::atlas_specialty_mappings_are_well_formed(duplicate_specialty) ||
        polyec::atlas_specialty_mappings_are_well_formed(royal_domain_specialty) ||
        polyec::atlas_specialty_mappings_are_well_formed(invalid_product_specialty) ||
        polyec::atlas_specialty_mappings_are_well_formed(mismatched_group_specialty)) return 15;
    if (polyec::atlas_specialty_mapping(24) != nullptr) return 14;
    if (polyec::kWorldAtlas[0].name != "Thanh Điền" ||
        polyec::kWorldAtlas[23].name != "Ngân Khê" ||
        polyec::kWorldAtlas[24].name != "Kinh Kỳ" ||
        polyec::kWorldAtlas[25].name != "Hoàng Điền" ||
        polyec::kWorldAtlas[26].name != "Trực Lộ") return 3;
    if (polyec::kWorldAtlas[0].population_thousands != 420 ||
        polyec::kWorldAtlas[0].area_square_kilometers != 8000 ||
        polyec::kWorldAtlas[24].population_thousands.has_value() ||
        polyec::kWorldAtlas[24].area_square_kilometers.has_value() ||
        polyec::kWorldAtlas[25].population_thousands.has_value() ||
        polyec::kWorldAtlas[25].area_square_kilometers.has_value() ||
        polyec::kWorldAtlas[26].population_thousands.has_value() ||
        polyec::kWorldAtlas[26].area_square_kilometers.has_value()) return 4;
    if (!polyec::world_atlas_links_are_well_formed() ||
        polyec::kWorldAtlasLinks.size() != 31) return 7;

    std::array<std::size_t, 7> corridor_link_counts{};
    for (const auto& link : polyec::kWorldAtlasLinks)
        ++corridor_link_counts[static_cast<std::size_t>(link.corridor)];
    constexpr std::array<std::size_t, 7> expected_link_counts{3, 2, 3, 6, 5, 7, 5};
    if (corridor_link_counts != expected_link_counts) return 8;

    const auto has_link = [](std::uint8_t from, std::uint8_t to,
                             polyec::AtlasCorridor corridor,
                             polyec::AtlasRouteMode mode,
                             polyec::AtlasLinkDirection direction) {
        for (const auto& link : polyec::kWorldAtlasLinks)
            if (link.from == from && link.to == to && link.corridor == corridor &&
                link.mode == mode && link.direction == direction) return true;
        return false;
    };
    const auto has_corridor_link = [](std::uint8_t from, std::uint8_t to,
                                      polyec::AtlasCorridor corridor) {
        for (const auto& link : polyec::kWorldAtlasLinks)
            if (link.from == from && link.to == to && link.corridor == corridor) return true;
        return false;
    };
    const auto path_is_present = [&](const auto& path, polyec::AtlasCorridor corridor) {
        for (std::size_t i = 1; i < path.size(); ++i)
            if (!has_corridor_link(path[i - 1], path[i], corridor)) return false;
        return true;
    };
    constexpr std::array<std::uint8_t, 4> river_path{0, 2, 1, 8};
    constexpr std::array<std::uint8_t, 3> road_path{5, 4, 0};
    constexpr std::array<std::uint8_t, 4> coastal_path{8, 9, 10, 11};
    constexpr std::array<std::uint8_t, 7> east_path{0, 3, 12, 13, 14, 15, 9};
    constexpr std::array<std::uint8_t, 6> north_path{4, 16, 17, 18, 19, 5};
    constexpr std::array<std::uint8_t, 8> west_path{5, 6, 7, 23, 22, 20, 21, 2};
    if (!path_is_present(river_path, polyec::AtlasCorridor::GreatRiver) ||
        !path_is_present(road_path, polyec::AtlasCorridor::RoadImprovement) ||
        !path_is_present(coastal_path, polyec::AtlasCorridor::Coastal) ||
        !path_is_present(east_path, polyec::AtlasCorridor::East) ||
        !path_is_present(north_path, polyec::AtlasCorridor::North) ||
        !path_is_present(west_path, polyec::AtlasCorridor::West)) return 9;

    if (!has_link(0, 2, polyec::AtlasCorridor::GreatRiver,
                  polyec::AtlasRouteMode::River, polyec::AtlasLinkDirection::Directed) ||
        !has_link(5, 4, polyec::AtlasCorridor::RoadImprovement,
                  polyec::AtlasRouteMode::Road, polyec::AtlasLinkDirection::Bidirectional) ||
        !has_link(8, 9, polyec::AtlasCorridor::Coastal,
                  polyec::AtlasRouteMode::CoastalShipping,
                  polyec::AtlasLinkDirection::Bidirectional) ||
        !has_link(0, 3, polyec::AtlasCorridor::East,
                  polyec::AtlasRouteMode::Unspecified,
                  polyec::AtlasLinkDirection::Bidirectional) ||
        !has_link(24, 26, polyec::AtlasCorridor::RoyalDomains,
                  polyec::AtlasRouteMode::Unspecified,
                  polyec::AtlasLinkDirection::Unspecified) ||
        !has_corridor_link(24, 0, polyec::AtlasCorridor::RoyalDomains) ||
        !has_corridor_link(24, 26, polyec::AtlasCorridor::RoyalDomains) ||
        !has_corridor_link(25, 0, polyec::AtlasCorridor::RoyalDomains) ||
        !has_corridor_link(25, 2, polyec::AtlasCorridor::RoyalDomains) ||
        !has_corridor_link(26, 19, polyec::AtlasCorridor::RoyalDomains)) return 10;

    if (!polyec::atlas_link_allows_direction(polyec::kWorldAtlasLinks[0], 0, 2) ||
        polyec::atlas_link_allows_direction(polyec::kWorldAtlasLinks[0], 2, 0) ||
        !polyec::atlas_link_allows_direction(polyec::kWorldAtlasLinks[3], 5, 4) ||
        !polyec::atlas_link_allows_direction(polyec::kWorldAtlasLinks[3], 4, 5) ||
        polyec::atlas_link_allows_direction(polyec::kWorldAtlasLinks[26], 24, 0)) return 17;

    const auto river_connection = polyec::atlas_topological_path(0, 8);
    constexpr std::array<std::uint8_t, 4> expected_river_connection{0, 2, 1, 8};
    if (!river_connection.reachable || river_connection.size != expected_river_connection.size() ||
        !std::equal(expected_river_connection.begin(), expected_river_connection.end(),
                    river_connection.territories.begin())) return 18;
    for (std::size_t i = 1; i < river_connection.size; ++i) {
        bool allowed = false;
        for (const auto& link : polyec::kWorldAtlasLinks)
            allowed |= polyec::atlas_link_allows_direction(
                link, river_connection.territories[i - 1], river_connection.territories[i]);
        if (!allowed) return 19;
    }
    for (std::uint8_t from = 0; from < polyec::kAtlasVassalCount; ++from) {
        for (std::uint8_t to = 0; to < polyec::kAtlasVassalCount; ++to) {
            const auto path = polyec::atlas_topological_path(from, to);
            if (!path.reachable || path.size == 0 || path.size > polyec::kAtlasVassalCount)
                return 21;
            for (std::size_t i = 1; i < path.size; ++i) {
                bool allowed = false;
                for (const auto& link : polyec::kWorldAtlasLinks)
                    allowed |= polyec::atlas_link_allows_direction(
                        link, path.territories[i - 1], path.territories[i]);
                if (!allowed) return 22;
            }
        }
    }
    const auto direct_road = polyec::atlas_topological_path(5, 4);
    std::array<bool, polyec::kAtlasLinkCount> unavailable_links{};
    unavailable_links[3] = true;
    const auto detour_around_closed_road = polyec::atlas_topological_path(5, 4, unavailable_links);
    constexpr std::array<std::uint8_t, 6> expected_north_detour{5, 19, 18, 17, 16, 4};
    if (!direct_road.reachable || direct_road.size != 2 ||
        !detour_around_closed_road.reachable ||
        detour_around_closed_road.size != expected_north_detour.size() ||
        !std::equal(expected_north_detour.begin(), expected_north_detour.end(),
                    detour_around_closed_road.territories.begin())) return 23;
    for (std::size_t i = 1; i < detour_around_closed_road.size; ++i) {
        bool allowed = false;
        for (std::size_t link_index = 0; link_index < polyec::kWorldAtlasLinks.size(); ++link_index) {
            if (unavailable_links[link_index]) continue;
            allowed |= polyec::atlas_link_allows_direction(
                polyec::kWorldAtlasLinks[link_index],
                detour_around_closed_road.territories[i - 1],
                detour_around_closed_road.territories[i]);
        }
        if (!allowed) return 24;
    }
    unavailable_links.fill(true);
    const auto every_link_closed = polyec::atlas_topological_path(0, 8, unavailable_links);
    if (every_link_closed.reachable || every_link_closed.size != 0) return 25;
    const auto same_territory = polyec::atlas_topological_path(7, 7);
    const auto unknown_royal_direction = polyec::atlas_topological_path(24, 0);
    const auto invalid_territory = polyec::atlas_topological_path(27, 0);
    if (!same_territory.reachable || same_territory.size != 1 ||
        same_territory.territories[0] != 7 || unknown_royal_direction.reachable ||
        unknown_royal_direction.size != 0 || invalid_territory.reachable ||
        invalid_territory.size != 0) return 20;

    constexpr std::array<std::string_view, 6> groups{
        "lưu vực", "cao nguyên", "duyên hải", "đông lộ", "biên bắc", "tây lộ"};
    constexpr std::array<std::uint64_t, 6> expected_population{1200, 435, 720, 830, 520, 480};
    constexpr std::array<std::uint64_t, 6> expected_area{26000, 47000, 17500, 24500, 63000, 34000};
    std::array<std::uint64_t, 6> population{};
    std::array<std::uint64_t, 6> area{};
    std::uint64_t total_population = 0;
    std::uint64_t total_area = 0;
    for (std::size_t i = 0; i < polyec::kAtlasVassalCount; ++i) {
        const auto& territory = polyec::kWorldAtlas[i];
        std::size_t group = groups.size();
        for (std::size_t j = 0; j < groups.size(); ++j)
            if (territory.region_group == groups[j]) group = j;
        if (group == groups.size()) return 5;
        if (!territory.population_thousands || !territory.area_square_kilometers) return 5;
        population[group] += *territory.population_thousands;
        area[group] += *territory.area_square_kilometers;
        total_population += *territory.population_thousands;
        total_area += *territory.area_square_kilometers;
    }
    if (population != expected_population || area != expected_area ||
        total_population != 4185 || total_area != 212000) return 6;

    std::cout << "WORLD ATLAS TESTS: 27 territories, six group totals, "
                 "31 minimum links, directed topology queries, 24 specialty classifications, "
                 "4,185 thousand people and 212,000 km2 verified\n";
    return 0;
}






