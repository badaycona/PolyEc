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
    static std::unordered_map<uint64_t, std::vector<uint64_t>> build(const std::vector<Person> &people) {
        std::unordered_map<uint64_t, std::vector<uint64_t>> households;
        households.reserve(people.size());
        for (const auto &p : people) {
            if (!p.household || !p.id) throw std::logic_error("household membership label");
            auto &members = households[p.household];
            if (members.empty() || members.back() < p.id) members.push_back(p.id);
            else if (std::find(members.begin(), members.end(), p.id) == members.end())
                members.insert(std::lower_bound(members.begin(), members.end(), p.id), p.id);
        }
        return households;
    }
};

}
