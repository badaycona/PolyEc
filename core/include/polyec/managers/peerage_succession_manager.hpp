#pragma once

#include "../realm.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace polyec {

enum class PeerageSuccessionState : uint8_t {
    SuccessorSelected,
    RecognitionRequired,
};

struct PeerageSuccessionResolution {
    PeerageSuccessionState state = PeerageSuccessionState::RecognitionRequired;
    uint64_t successor_id = 0;
    std::vector<uint64_t> candidates;
};

// Resolves the eldest living, legally verified direct child for hereditary
// peerage. It does not mutate Realm or determine how a pending title is granted.
class PeerageSuccessionManager {
public:
    static PeerageSuccessionResolution resolve_successor(
        uint64_t deceased, const std::vector<Person>& people,
        const std::vector<Edge>& relations) {
        if (deceased == 0 || deceased > people.size() ||
            people[static_cast<size_t>(deceased - 1)].id != deceased ||
            people[static_cast<size_t>(deceased - 1)].alive)
            throw std::invalid_argument("invalid peerage title holder");

        for (size_t i = 0; i < people.size(); ++i) {
            if (people[i].id != i + 1 || people[i].age_days < 0)
                throw std::invalid_argument("invalid peerage person record");
        }
        for (const auto& edge : relations) {
            if (edge.from == 0 || edge.to == 0 || edge.from > people.size() ||
                edge.to > people.size() || edge.kind > RelationKind::Spouse ||
                (edge.legal_parentage &&
                    (edge.kind != RelationKind::ParentChild || edge.from == edge.to)))
                throw std::invalid_argument("invalid peerage relation edge");
        }

        int eldest_age_days = -1;
        std::vector<uint64_t> eldest_children;
        for (const auto& edge : relations) {
            if (edge.kind != RelationKind::ParentChild || !edge.legal_parentage ||
                edge.to != deceased || !people[static_cast<size_t>(edge.from - 1)].alive)
                continue;
            const auto& child = people[static_cast<size_t>(edge.from - 1)];
            if (child.age_days > eldest_age_days) {
                eldest_age_days = child.age_days;
                eldest_children.assign(1, child.id);
            } else if (child.age_days == eldest_age_days) {
                eldest_children.push_back(child.id);
            }
        }

        std::sort(eldest_children.begin(), eldest_children.end());
        eldest_children.erase(std::unique(eldest_children.begin(), eldest_children.end()),
                              eldest_children.end());
        if (eldest_children.size() == 1)
            return {PeerageSuccessionState::SuccessorSelected, eldest_children.front(), {}};
        if (eldest_children.empty())
            return {PeerageSuccessionState::RecognitionRequired, 0, {}};
        return {PeerageSuccessionState::RecognitionRequired, 0, std::move(eldest_children)};
    }
};

} // namespace polyec
