#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes the Person court extension fields in the existing Realm save token
// order. There is no count prefix; the number of records is fixed by the
// primary Person section. Primary Person fields are not touched.
inline void write_person_court_extension_section(std::ostream& out,
                                                const std::vector<Person>& people) {
    for (const auto& p : people) {
        out << p.candidate << ' ' << p.office_appointment.office << ' '
            << p.special_advisor << ' ' << p.office_appointment.hired_by << ' '
            << p.office_appointment.salary_due << ' '
            << p.office_appointment.office_notice << ' '
            << p.last_decision_day << ' ';
    }
}

// Reads extension fields into a staged copy of the incoming vector. The copy
// is published only after every field in every record has parsed, so a parse
// failure leaves the caller's vector unchanged. All parse failures throw
// std::runtime_error("Save court fields"); there is no count prefix and no
// "Save count limit" error for this section. No range validation is added.
inline void read_person_court_extension_section(std::istream& in,
                                               std::vector<Person>& people) {
    std::vector<Person> staged = people;
    for (auto& p : staged) {
        if (!(in >> p.candidate >> p.office_appointment.office
              >> p.special_advisor >> p.office_appointment.hired_by
              >> p.office_appointment.salary_due
              >> p.office_appointment.office_notice
              >> p.last_decision_day))
            throw std::runtime_error("Save court fields");
    }
    people = std::move(staged);
}

} // namespace detail
} // namespace polyec
