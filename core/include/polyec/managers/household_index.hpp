#pragma once
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "../realm.hpp"

namespace polyec {

// Pure rebuild helper for Realm's derived household index. Realm keeps
// ownership of households_; this only computes the label -> ascending member
// IDs map with the SAME validation as Realm::index_household (d5751bb): a zero
// household label or zero person ID is a hard error, never a silent skip.
class HouseholdIndex {
public:
    static uint64_t guardian(const std::vector<Person> &people,
                             const std::unordered_map<uint64_t, std::vector<uint64_t>> &households,
                             const Person &child) {
        if (child.age_days >= 18 * 365) return 0;
        auto household = households.find(child.household_membership.household_id);
        if (household == households.end()) return 0;
        uint64_t id = 0;
        int oldest = 0;
        for (uint64_t member : household->second) {
            if (member == 0 || member > people.size()) continue;
            const auto &adult = people[member - 1];
            if (adult.id == child.id ||
                adult.household_membership.household_id != child.household_membership.household_id ||
                !adult.alive) continue;
            if (adult.age_days >= 18 * 365 && adult.age_days > oldest) {
                id = member;
                oldest = adult.age_days;
            }
        }
        return id;
    }

    static uint64_t authority(const std::vector<Person> &people,
                              const std::unordered_map<uint64_t, std::vector<uint64_t>> &households,
                              uint64_t household_id) {
        if (!household_id || household_id > people.size()) return 0;
        auto record = households.find(household_id);
        if (record == households.end()) return 0;
        const auto &head = people[household_id - 1];
        if (head.alive && head.household_membership.household_id == household_id && head.age_days >= 18 * 365)
            return household_id;
        uint64_t id = 0;
        int oldest = 0;
        for (uint64_t member : record->second) {
            if (member == 0 || member > people.size()) continue;
            const auto &adult = people[member - 1];
            if (adult.alive && adult.age_days >= 18 * 365 && adult.age_days > oldest) {
                id = member;
                oldest = adult.age_days;
            }
        }
        return id;
    }

    static std::unordered_map<uint64_t, std::vector<uint64_t>> build(const std::vector<Person> &people) {
        std::unordered_map<uint64_t, std::vector<uint64_t>> households;
        households.reserve(people.size());
        for (const auto &p : people) {
            if (!p.household_membership.household_id || !p.id) throw std::logic_error("household membership label");
            auto &members = households[p.household_membership.household_id];
            if (members.empty() || members.back() < p.id) members.push_back(p.id);
            else if (std::find(members.begin(), members.end(), p.id) == members.end())
                members.insert(std::lower_bound(members.begin(), members.end(), p.id), p.id);
        }
        return households;
    }
};

}
