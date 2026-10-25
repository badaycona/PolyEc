#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

namespace polyec::test_fixtures {
inline void strip_empty_military_control_section(std::string& source) {
    const auto section = source.rfind(" MILITARY_CONTROL_1 ");
    if (section == std::string::npos)
        throw std::runtime_error("current save lacks MILITARY_CONTROL_1");
    std::istringstream input(source.substr(section + 1));
    std::string tag;
    size_t count = 0;
    input >> tag >> count;
    if (!input || tag != "MILITARY_CONTROL_1" || count != 27)
        throw std::runtime_error("invalid current military control section");
    for (size_t region = 0; region < count; ++region) {
        int partner = 0, garrison = 0;
        input >> partner >> garrison;
        if (!input || partner != -1 || garrison != 0)
            throw std::runtime_error("legacy fixture requires empty military occupation records");
    }
    input >> std::ws;
    if (!input.eof()) throw std::runtime_error("trailing current military control fields");
    source.erase(section);
}
inline void strip_empty_apprenticeship_section(std::string& source) {
    const auto section = source.rfind(" APPRENTICESHIP_1 ");
    if (section == std::string::npos)
        throw std::runtime_error("current save lacks APPRENTICESHIP_1");
    std::istringstream input(source.substr(section + 1));
    std::string tag;
    size_t count = 0;
    input >> tag >> count;
    if (!input || tag != "APPRENTICESHIP_1" || count != 0)
        throw std::runtime_error("legacy fixture requires an empty apprenticeship section");
    source.erase(section);
}
} // namespace polyec::test_fixtures
