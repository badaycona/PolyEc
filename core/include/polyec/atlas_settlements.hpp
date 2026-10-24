#pragma once

#include "polyec/atlas_cartography.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace polyec {

// atlas-urban-sites-v1 names one fixed fictional center inside each Atlas
// territory. A site is map content only; it does not create an urban census,
// Person, account, ownership right, market stock, or route.
enum class AtlasUrbanSiteKind : uint8_t {
    RegionalCenter,
    Port,
    RoyalCenter,
    RoyalGranary,
    RoyalWorks,
};

struct AtlasUrbanSite {
    uint8_t id;
    uint8_t territory_id;
    std::string_view name;
    AtlasUrbanSiteKind kind;
    AtlasMapPoint position;
};

inline constexpr std::array<AtlasUrbanSite, kAtlasTerritoryCount> kAtlasUrbanSites{{
    {0, 0, "Long Phố", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[0].label_anchor.x - 25.0, kAtlasMapPolygons[0].label_anchor.y + 28.0}},
    {1, 1, "Hà Trạch", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[1].label_anchor.x - 25.0, kAtlasMapPolygons[1].label_anchor.y + 28.0}},
    {2, 2, "Phù Thành", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[2].label_anchor.x - 25.0, kAtlasMapPolygons[2].label_anchor.y + 28.0}},
    {3, 3, "Đông Viên", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[3].label_anchor.x - 25.0, kAtlasMapPolygons[3].label_anchor.y + 28.0}},
    {4, 4, "Bạch Khâu", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[4].label_anchor.x - 25.0, kAtlasMapPolygons[4].label_anchor.y + 28.0}},
    {5, 5, "Hắc Sơn Trấn", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[5].label_anchor.x - 25.0, kAtlasMapPolygons[5].label_anchor.y + 28.0}},
    {6, 6, "Tùng Quan", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[6].label_anchor.x - 25.0, kAtlasMapPolygons[6].label_anchor.y + 28.0}},
    {7, 7, "Xích Lô", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[7].label_anchor.x - 25.0, kAtlasMapPolygons[7].label_anchor.y + 28.0}},
    {8, 8, "Nam Cảng", AtlasUrbanSiteKind::Port,
     {kAtlasMapPolygons[8].label_anchor.x - 25.0, kAtlasMapPolygons[8].label_anchor.y + 28.0}},
    {9, 9, "Hải Châu", AtlasUrbanSiteKind::Port,
     {kAtlasMapPolygons[9].label_anchor.x - 25.0, kAtlasMapPolygons[9].label_anchor.y + 28.0}},
    {10, 10, "Sa Kính", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[10].label_anchor.x - 25.0, kAtlasMapPolygons[10].label_anchor.y + 28.0}},
    {11, 11, "Trầm Hương Phố", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[11].label_anchor.x - 25.0, kAtlasMapPolygons[11].label_anchor.y + 28.0}},
    {12, 12, "Vân Lụa Thành", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[12].label_anchor.x - 25.0, kAtlasMapPolygons[12].label_anchor.y + 28.0}},
    {13, 13, "Cẩm Giang Thành", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[13].label_anchor.x - 25.0, kAtlasMapPolygons[13].label_anchor.y + 28.0}},
    {14, 14, "Hồng Diệp Trấn", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[14].label_anchor.x - 25.0, kAtlasMapPolygons[14].label_anchor.y + 28.0}},
    {15, 15, "Bích Lò", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[15].label_anchor.x - 25.0, kAtlasMapPolygons[15].label_anchor.y + 28.0}},
    {16, 16, "Bắc Mã Thành", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[16].label_anchor.x - 25.0, kAtlasMapPolygons[16].label_anchor.y + 28.0}},
    {17, 17, "Phong Ải", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[17].label_anchor.x - 25.0, kAtlasMapPolygons[17].label_anchor.y + 28.0}},
    {18, 18, "Hoàng Nguyên Thành", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[18].label_anchor.x - 25.0, kAtlasMapPolygons[18].label_anchor.y + 28.0}},
    {19, 19, "Thiết Quan", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[19].label_anchor.x - 25.0, kAtlasMapPolygons[19].label_anchor.y + 28.0}},
    {20, 20, "Dược Thành", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[20].label_anchor.x - 25.0, kAtlasMapPolygons[20].label_anchor.y + 28.0}},
    {21, 21, "Mật Lâm", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[21].label_anchor.x - 25.0, kAtlasMapPolygons[21].label_anchor.y + 28.0}},
    {22, 22, "Thạch Kiều", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[22].label_anchor.x - 25.0, kAtlasMapPolygons[22].label_anchor.y + 28.0}},
    {23, 23, "Ngân Khê", AtlasUrbanSiteKind::RegionalCenter,
     {kAtlasMapPolygons[23].label_anchor.x - 25.0, kAtlasMapPolygons[23].label_anchor.y + 28.0}},
    {24, 24, "Kinh Kỳ", AtlasUrbanSiteKind::RoyalCenter,
     {kAtlasMapPolygons[24].label_anchor.x - 25.0, kAtlasMapPolygons[24].label_anchor.y + 28.0}},
    {25, 25, "Hoàng Kho", AtlasUrbanSiteKind::RoyalGranary,
     {kAtlasMapPolygons[25].label_anchor.x - 25.0, kAtlasMapPolygons[25].label_anchor.y + 28.0}},
    {26, 26, "Trực Xưởng", AtlasUrbanSiteKind::RoyalWorks,
     {kAtlasMapPolygons[26].label_anchor.x - 25.0, kAtlasMapPolygons[26].label_anchor.y + 28.0}},
}};

constexpr bool atlas_point_in_polygon(AtlasMapPoint point,
                                      const AtlasMapPolygon& polygon) noexcept {
    for (size_t edge = 0; edge < polygon.vertices.size(); ++edge) {
        const auto& a = polygon.vertices[edge];
        const auto& b = polygon.vertices[(edge + 1) % polygon.vertices.size()];
        const double side = (b.x - a.x) * (point.y - a.y) -
                            (b.y - a.y) * (point.x - a.x);
        if (side <= 0.0) return false;
    }
    return true;
}

constexpr const AtlasUrbanSite* atlas_urban_site(uint8_t id) noexcept {
    return id < kAtlasUrbanSites.size() ? &kAtlasUrbanSites[id] : nullptr;
}

constexpr bool atlas_urban_sites_are_well_formed() noexcept {
    if (kAtlasUrbanSites.size() != kAtlasTerritoryCount) return false;
    for (size_t i = 0; i < kAtlasUrbanSites.size(); ++i) {
        const auto& site = kAtlasUrbanSites[i];
        if (site.id != i || site.territory_id != i || site.name.empty() ||
            site.kind > AtlasUrbanSiteKind::RoyalWorks ||
            !atlas_point_in_polygon(site.position, kAtlasMapPolygons[i])) return false;
        for (size_t other = i + 1; other < kAtlasUrbanSites.size(); ++other)
            if (site.name == kAtlasUrbanSites[other].name) return false;
    }
    return true;
}

static_assert(atlas_urban_sites_are_well_formed());

} // namespace polyec
