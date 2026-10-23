#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
#include "../realm.hpp"
#include "finance_manager.hpp"

namespace polyec {

// Stateless employment transition helpers. Realm remains the sole owner of
// Person/Province data and the command/sequence/legality boundary; these
// functions only perform already-approved transitions.
class EmploymentManager {
public:
    static constexpr int64_t kAppointmentFee = 20;
    static constexpr int64_t kOfficeSalary = 20;
    static constexpr int kOfficeSalaryPeriod = 30;
    static constexpr int kOfficeNoticeDays = 30;

    // Exact Realm::legal Work predicate, including representability of
    // day + term. Kept here so legality and transition share one definition.
    static bool can_start_work(const Person &person, const Province &province,
                               int target, int term, int day) noexcept {
        return person.family != Family::Governance &&
               person.age_days >= 14 * 365 && target == person.region &&
               (province.employed < province.jobs || person.work_assignment.job == person.region) &&
               term > 0 && term <= 30 &&
               day <= std::numeric_limits<int>::max() - term;
    }

    // Compatibility overload for callers whose day is not part of the query;
    // Realm::legal uses the day-aware form above.
    static bool can_start_work(const Person &person, const Province &province,
                               int target, int term) noexcept {
        return can_start_work(person, province, target, term, 0);
    }

    // Exact preflight shared by Realm::legal(Appoint) and appoint(). Candidate
    // selection stays in Realm's referral order; this validates the selected
    // appointee before any mutation.
    static bool can_appoint(const Person &person, const std::vector<Province> &provinces,
                            int64_t employer_treasury, uint64_t employer,
                            int day) noexcept {
        if (!person.alive || employer == 0 ||
            person.region < 0 || size_t(person.region) >= provinces.size() ||
            employer_treasury < kAppointmentFee ||
            person.cash > std::numeric_limits<int64_t>::max() - kAppointmentFee) {
            return false;
        }
        const int period = std::max(kOfficeSalaryPeriod, kOfficeNoticeDays);
        return day <= std::numeric_limits<int>::max() - period;
    }

    static bool can_resign(const Person &person, int day) noexcept {
        return person.office_appointment.hired_by != 0 && day >= person.office_appointment.office_notice;
    }

    static bool has_active_work(const Person &person, int day) noexcept {
        return person.work_assignment.job >= 0 && person.busy_until >= day;
    }

    // Exact production teacher rule: age/skill, one teacher per 20 school
    // capacity, and a funded treasury.
    static bool teacher_eligible(const Person &person, const Province &province) noexcept {
        return person.age_days >= 18 * 365 && person.skill >= 20 &&
               province.teachers < (province.school + 19) / 20 &&
               province.treasury >= 2;
    }

    static void reset_day_counters(std::vector<Province> &provinces) noexcept {
        for (auto &province : provinces) {
            province.employed = 0;
            province.teachers = 0;
            province.teacher_skill_sum = 0;
        }
    }

    // Returns 0 for an active worker (production path), 1 for an active
    // teacher, and -1 after clearing an expired assignment.
    static int account_day(Person &person, Province &province, int day) noexcept {
        if (person.work_assignment.job >= 0 && person.busy_until >= day) {
            ++province.employed;
            if (person.work_assignment.job_kind == 1) {
                ++province.teachers;
                province.teacher_skill_sum += person.skill;
                return 1;
            }
            return 0;
        }
        person.work_assignment.job = -1;
        return -1;
    }

    // Exact legacy pre-switch clear: decrement counters whenever a job was
    // assigned, even if busy_until has already expired, and clamp at zero.
    static void clear_work(Person &person, Province &province) noexcept {
        if (person.work_assignment.job < 0) return;
        province.employed = std::max(0, province.employed - 1);
        if (person.work_assignment.job_kind == 1) province.teachers = std::max(0, province.teachers - 1);
        person.work_assignment.job = -1;
    }

    static void start_work(Person &person, std::vector<Province> &provinces,
                           int day, int region, int term) {
        if (!person.alive) throw std::logic_error("deceased worker");
        if (region < 0 || size_t(region) >= provinces.size())
            throw std::out_of_range("work region");
        if (region != person.region) throw std::logic_error("work region mismatch");
        if (term <= 0) throw std::logic_error("invalid work term");
        if (day > std::numeric_limits<int>::max() - term)
            throw std::overflow_error("work day overflow");

        auto &province = provinces[size_t(region)];
        if (!can_start_work(person, province, region, term, day))
            throw std::logic_error("ineligible work transition");
        // Realm already ran the clearsJob block for the command; this also keeps
        // direct manager calls on the exact same transition path.
        clear_work(person, province);

        const int kind = teacher_eligible(person, province) ? 1 : 0;
        person.work_assignment.job = region;
        person.work_assignment.job_kind = kind;
        person.busy_until = day + term;
        ++province.employed;
        if (kind == 1) ++province.teachers;
    }

    static void appoint(Person &person, std::vector<Province> &provinces,
                        int64_t &employer_treasury, uint64_t employer, int day) {
        if (!person.alive) throw std::logic_error("deceased appointee");
        if (employer == 0) throw std::logic_error("missing employer");
        if (person.region < 0 || size_t(person.region) >= provinces.size())
            throw std::out_of_range("appointee region");
        if (person.cash > std::numeric_limits<int64_t>::max() - kAppointmentFee)
            throw std::overflow_error("appointment cash overflow");
        if (day > std::numeric_limits<int>::max() - kOfficeNoticeDays)
            throw std::overflow_error("appointment day overflow");
        if (employer_treasury < kAppointmentFee)
            throw std::logic_error("unfunded appointment");
        if (!can_appoint(person, provinces, employer_treasury, employer, day))
            throw std::logic_error("ineligible appointment");

        auto &province = provinces[size_t(person.region)];
        // Appointment clears the selected person's prior work assignment; the
        // employer's own clearsJob block is handled by Realm before the switch.
        clear_work(person, province);
        person.work_assignment.job = -1;
        person.school_days = 0;
        person.busy_until = day;
        person.candidate = false;
        person.office_appointment.office = true;
        person.office_appointment.hired_by = employer;
        person.office_appointment.salary_due = day + kOfficeSalaryPeriod;
        person.office_appointment.office_notice = day + kOfficeNoticeDays;
        person.family = Family::Governance;
        person.taxing_authority = false;
        FinanceManager::transfer(employer_treasury, person.cash, kAppointmentFee);
    }

    // Shared by voluntary resignation and failed office salary payment.
    static void end_office(Person &person) noexcept {
        person.office_appointment.office = false;
        person.office_appointment.hired_by = 0;
        person.family = Family::Civilian;
    }

    static void resign(Person &person) {
        if (person.office_appointment.hired_by == 0) throw std::logic_error("resign without office");
        end_office(person);
    }
};

} // namespace polyec
