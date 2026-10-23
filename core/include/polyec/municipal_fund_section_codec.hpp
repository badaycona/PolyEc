#pragma once

#include <cstdint>
#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace polyec::detail {

inline void write_municipal_fund_section(std::ostream& out,
                                         const std::vector<int64_t>& funds,
                                         const std::vector<int>& last_funded_period) {
    if (funds.size() != last_funded_period.size())
        throw std::runtime_error("Municipal fund section dimensions");
    out << " MUNICIPAL_FUNDS_1 " << funds.size() << ' ';
    for (std::size_t region=0; region<funds.size(); ++region)
        out << funds[region] << ' ' << last_funded_period[region] << ' ';
}

inline void read_municipal_fund_section(std::istream& in,
                                        std::vector<int64_t>& funds,
                                        std::vector<int>& last_funded_period,
                                         std::size_t expected_regions,
                                        int current_day) {
    std::string tag;
    std::size_t count=0;
    in >> tag >> count;
    if (!in || tag != "MUNICIPAL_FUNDS_1" || count != expected_regions)
        throw std::runtime_error("Save municipal fund header");
    std::vector<int64_t> staged_funds(count);
    std::vector<int> staged_periods(count);
    for (std::size_t region=0; region<count; ++region) {
        in >> staged_funds[region] >> staged_periods[region];
        if (!in || staged_funds[region] < 0 || staged_periods[region] < -1 ||
            staged_periods[region] > current_day/30)
            throw std::runtime_error("Save municipal fund fields");
    }
    funds=std::move(staged_funds);
    last_funded_period=std::move(staged_periods);
}

} // namespace polyec::detail
