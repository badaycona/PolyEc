#pragma once

#include "polyec/land_holding.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace polyec::detail {

inline void write_land_use_section(std::ostream& out, const std::vector<LandUseRight>& rights) {
    out << " LANDUSE_1 " << rights.size() << ' ';
    for (const auto& right : rights)
        out << right.id << ' ' << right.holding_id << ' ' << right.grantee << ' '
            << right.grantor << ' ';
}

inline std::vector<LandUseRight> read_land_use_section(std::istream& in, size_t person_count,
                                                       const std::vector<LandHolding>& holdings,
                                                       uint64_t next_id) {
    std::string tag;
    size_t count = 0;
    in >> tag >> count;
    if (!in || tag != "LANDUSE_1" || count > holdings.size() || count > 1000000)
        throw std::runtime_error("Save land use header");

    std::vector<LandUseRight> staged;
    staged.reserve(count);
    uint64_t previousId = 0;
    uint64_t previousHolding = 0;
    for (size_t row = 0; row < count; ++row) {
        LandUseRight right{};
        in >> right.id >> right.holding_id >> right.grantee >> right.grantor;
        if (!in || right.id == 0 || right.id >= next_id || (row && right.id <= previousId) ||
            right.holding_id == 0 || right.grantee == 0 || right.grantee > person_count ||
            right.grantor == 0 || right.grantor > person_count)
            throw std::runtime_error("Save land use fields");
        const auto holding = std::lower_bound(holdings.begin(), holdings.end(), right.holding_id,
            [](const LandHolding& value, uint64_t id) { return value.id < id; });
        if (holding == holdings.end() || holding->id != right.holding_id ||
            holding->title_holder != right.grantor || right.grantee == right.grantor ||
            (row && right.holding_id == previousHolding))
            throw std::runtime_error("Save land use ownership");
        previousId = right.id;
        previousHolding = right.holding_id;
        staged.push_back(right);
    }
    return staged;
}

} // namespace polyec::detail
