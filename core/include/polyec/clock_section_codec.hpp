#pragma once

#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace polyec {
namespace detail {

// Writes the CLOCK_1 section in the existing Realm save token order. There is
// no count prefix; this is one fixed tag followed by two fixed Realm-owned
// scalar values. Realm owns paused_ and pause_revision_.
inline void write_clock_section(std::ostream& out, bool paused, uint64_t pause_revision) {
    out << " CLOCK_1 " << (paused ? 1 : 0) << ' ' << pause_revision << ' ';
}

// Reads the CLOCK_1 tag, pause flag and revision into local temporaries and
// publishes them to the output references only after the tag matches, the flag
// is in {0,1} and the revision has parsed. A failure throws
// std::runtime_error("Save clock header") before assignment, so the caller's
// values remain unchanged. No range or semantic validation is added; negative
// tokens accepted by std::istream extraction into uint64_t (such as -1 under
// MSVC) are preserved.
inline void read_clock_section(std::istream& in, bool& paused, uint64_t& pause_revision) {
    std::string clockTag;
    int pausedFlag = 0;
    uint64_t stagedRevision = 0;

    in >> clockTag >> pausedFlag >> stagedRevision;
    if (!in || clockTag != "CLOCK_1" || (pausedFlag != 0 && pausedFlag != 1)) {
        throw std::runtime_error("Save clock header");
    }
    paused = pausedFlag == 1;
    pause_revision = stagedRevision;
}

} // namespace detail
} // namespace polyec