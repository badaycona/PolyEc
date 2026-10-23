#pragma once

#include "polyec/land_holding.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace polyec::detail {

inline void write_land_title_section(std::ostream& out,
                                     const std::vector<LandHolding>& holdings) {
    std::vector<LandHolding> sorted = holdings;
    std::sort(sorted.begin(), sorted.end(),
              [](const LandHolding& a, const LandHolding& b) { return a.id < b.id; });
    out << " LAND_1 " << sorted.size() << ' ';
    for (const auto& holding : sorted)
        out << holding.id << ' ' << holding.region << ' ' << holding.area_units << ' '
            << holding.title_holder << ' ';
}

inline std::vector<LandHolding> read_land_title_section(std::istream& in,
                                                        size_t person_count,
                                                        int regions) {
    std::string tag;
    size_t count = 0;
    in >> tag >> count;
    if (!in || tag != "LAND_1" || count > 1000000)
        throw std::runtime_error("Save land title header");
    std::vector<LandHolding> staged;
    staged.reserve(count);
    uint64_t previous = 0;
    for (size_t row = 0; row < count; ++row) {
        LandHolding holding{};
        in >> holding.id >> holding.region >> holding.area_units >> holding.title_holder;
        if (!in || holding.id == 0 || (row && holding.id <= previous) || holding.region < 0 ||
            holding.region > regions || holding.area_units <= 0 || holding.title_holder == 0 ||
            holding.title_holder > person_count)
            throw std::runtime_error("Save land title fields");
        previous = holding.id;
        staged.push_back(holding);
    }
    return staged;
}

} // namespace polyec::detail
