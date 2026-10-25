#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec::detail {

inline void write_apprenticeship_section(std::ostream& out,
                                         const std::vector<Apprenticeship>& records) {
    out << " APPRENTICESHIP_1 " << records.size() << ' ';
    for (const auto& record : records) {
        out << record.id << ' ' << record.mentor << ' ' << record.apprentice << ' '
            << record.establishment_id << ' ' << record.started_day << ' '
            << record.due_day << ' ' << record.practice_days << ' '
            << record.started_tick << ' ' << record.due_tick << ' '
            << record.mentor_ticks_observed << ' ' << record.apprentice_ticks_observed << ' '
            << record.tuition << ' ' << unsigned(record.status) << ' ';
    }
}

inline void read_apprenticeship_section(std::istream& in,
                                        std::vector<Apprenticeship>& records) {
    std::string marker;
    size_t count = 0;
    if (!(in >> marker >> count) || marker != "APPRENTICESHIP_1" || count > 1000000)
        throw std::runtime_error("Save apprenticeship header");
    std::vector<Apprenticeship> staged(count);
    for (auto& record : staged) {
        unsigned status = 0;
        if (!(in >> record.id >> record.mentor >> record.apprentice >> record.establishment_id
                 >> record.started_day >> record.due_day >> record.practice_days
                 >> record.started_tick >> record.due_tick
                 >> record.mentor_ticks_observed >> record.apprentice_ticks_observed
                 >> record.tuition >> status) || status > unsigned(ApprenticeshipStatus::Interrupted))
            throw std::runtime_error("Save apprenticeship fields");
        record.status = static_cast<ApprenticeshipStatus>(status);
    }
    records = std::move(staged);
}

} // namespace polyec::detail
