#pragma once

#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace polyec::detail {

inline void write_urban_housing_section(std::ostream& out,
                                        const std::vector<int64_t>& capacity) {
    out << "URBAN_HOUSING_1 " << capacity.size() << ' ';
    for (const auto beds : capacity) out << beds << ' ';
}

inline void read_urban_housing_section(std::istream& in, int regions,
                                       std::vector<int64_t>& capacity) {
    std::string tag;
    size_t count = 0;
    if (!(in >> tag >> count) || tag != "URBAN_HOUSING_1" ||
        count != size_t(regions + 1))
        throw std::runtime_error("Save urban housing header");
    std::vector<int64_t> staged(count);
    for (auto& beds : staged) {
        if (!(in >> beds) || beds < 0 || beds > 1000000000LL)
            throw std::runtime_error("Save urban housing capacity");
        if (regions != 26 && beds != 0)
            throw std::runtime_error("Save housing outside Atlas");
    }
    capacity = std::move(staged);
}

} // namespace polyec::detail
