#pragma once
#include <vector>
#include <unordered_map>
#include "../types.hpp"

namespace polyec {

class HouseholdIndex {
public:
    // Build a household index: label -> ascending member IDs
    static std::unordered_map<uint64_t, std::vector<uint64_t>> build(const std::vector<Person>& people) {
        std::unordered_map<uint64_t, std::vector<uint64_t>> households;
        households.reserve(people.size());
        for (const auto &p : people) {
            if (!p.household) continue;
            auto &members = households[p.household];
            if (members.empty() || members.back() < p.id) members.push_back(p.id);
            else if (std::find(members.begin(), members.end(), p.id) == members.end())
                members.insert(std::lower_bound(members.begin(), members.end(), p.id), p.id);
        }
        return households;
    }
};

}
