#pragma once

#include <cstdint>
#include <istream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace polyec::detail {

inline void write_household_fund_section(std::ostream& out,
                                         const std::map<uint64_t, int64_t>& funds) {
    out << " HOUSEHOLD_1 " << funds.size() << ' ';
    for (const auto& [household, cash] : funds) out << household << ' ' << cash << ' ';
}

inline std::map<uint64_t, int64_t> read_household_fund_section(std::istream& in,
                                                               uint64_t person_count) {
    std::string tag;
    std::size_t count = 0;
    in >> tag >> count;
    if (!in || tag != "HOUSEHOLD_1" || count > person_count)
        throw std::runtime_error("Save household fund header");
    std::map<uint64_t, int64_t> staged;
    uint64_t previous = 0;
    for (std::size_t row = 0; row < count; ++row) {
        uint64_t household = 0;
        int64_t cash = 0;
        in >> household >> cash;
        if (!in || household == 0 || household > person_count || (row && household <= previous) || cash <= 0)
            throw std::runtime_error("Save household fund fields");
        previous = household;
        staged.emplace(household, cash);
    }
    return staged;
}

} // namespace polyec::detail
