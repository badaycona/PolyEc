#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

namespace polyec::test_fixtures {
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
