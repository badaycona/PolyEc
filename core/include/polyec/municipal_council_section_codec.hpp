#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec::detail {

inline void write_municipal_council_section(std::ostream& out,
                                             const std::vector<MunicipalCouncil>& councils) {
    out << "MUNICIPAL_COUNCIL_1 " << councils.size() << ' ';
    for (const auto& council : councils) {
        out << unsigned(council.site_id) << ' ' << council.representative << ' '
            << council.nominee << ' ' << council.nomination_day << ' '
            << council.term_end_day << ' ';
    }
}

inline void read_municipal_council_section(std::istream& in, int regions,
                                            std::vector<MunicipalCouncil>& councils) {
    std::string tag;
    size_t count = 0;
    const size_t expected=regions==26?27:0;
    if (!(in >> tag >> count) || tag != "MUNICIPAL_COUNCIL_1" || count != expected)
        throw std::runtime_error("Save municipal council header");
    std::vector<MunicipalCouncil> staged(count);
    for (size_t index = 0; index < count; ++index) {
        unsigned site = 0;
        auto& council = staged[index];
        if (!(in >> site >> council.representative >> council.nominee
              >> council.nomination_day >> council.term_end_day) ||
            site != index || site >= 27)
            throw std::runtime_error("Save municipal council row");
        council.site_id = static_cast<uint8_t>(site);
    }
    councils = std::move(staged);
}

} // namespace polyec::detail
