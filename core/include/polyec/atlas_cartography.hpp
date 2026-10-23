#pragma once

#include "polyec/world_atlas.hpp"

#include <array>
#include <cstdint>

namespace polyec {

// atlas-cartography-v1 is authored fictional geometry. Coordinates are map
// units and must not be interpreted as kilometres or measured area.
struct AtlasMapPoint { double x; double y; };
struct AtlasMapPolygon {
    uint8_t territory_id;
    AtlasMapPoint label_anchor;
    std::array<AtlasMapPoint, 6> vertices;
};

inline constexpr double kAtlasMapUnitScale = 1.0;
inline constexpr double kAtlasMapHexRadius = 78.0;
inline constexpr std::array<AtlasMapPolygon, kAtlasTerritoryCount> kAtlasMapPolygons = [] {
    std::array<AtlasMapPolygon, kAtlasTerritoryCount> result{};
    // Four staggered rows form one fictional contiguous atlas. Six territories
    // occupy the first row; seven occupy each remaining row.
    for (size_t id = 0; id < kAtlasTerritoryCount; ++id) {
        const size_t row = id < 6 ? 0 : 1 + (id - 6) / 7;
        const size_t column = id < 6 ? id : (id - 6) % 7;
        const double x = (row % 2 == 0 ? 105.0 : 172.55) + 135.0 * column;
        const double y = 130.0 + 117.0 * row;
        constexpr std::array<AtlasMapPoint, 6> offsets{{
            {0.0, -78.0}, {67.55, -39.0}, {67.55, 39.0},
            {0.0, 78.0}, {-67.55, 39.0}, {-67.55, -39.0}}};
        auto& polygon = result[id];
        polygon.territory_id = static_cast<uint8_t>(id);
        polygon.label_anchor = {x, y};
        for (size_t vertex = 0; vertex < offsets.size(); ++vertex)
            polygon.vertices[vertex] = {x + offsets[vertex].x, y + offsets[vertex].y};
    }
    return result;
}();

constexpr bool atlas_map_catalog_is_well_formed() noexcept {
    for (size_t i = 0; i < kAtlasMapPolygons.size(); ++i) {
        const auto& polygon = kAtlasMapPolygons[i];
        if (polygon.territory_id != i || polygon.vertices.size() < 3) return false;
        double twice_area = 0.0;
        for (size_t j = 0; j < polygon.vertices.size(); ++j) {
            const auto& a = polygon.vertices[j];
            const auto& b = polygon.vertices[(j + 1) % polygon.vertices.size()];
            if (a.x < 0 || a.x > 1100 || a.y < 0 || a.y > 650) return false;
            twice_area += a.x * b.y - b.x * a.y;
        }
        if (twice_area <= 0.0) return false;
        if (polygon.label_anchor.x < 0 || polygon.label_anchor.x > 1100 ||
            polygon.label_anchor.y < 0 || polygon.label_anchor.y > 650) return false;
    }
    return true;
}
static_assert(atlas_map_catalog_is_well_formed());

} // namespace polyec
