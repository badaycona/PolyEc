#include "polyec/realm.hpp"
#include "polyec/realm_view_projector.hpp"
#include <unordered_set>

namespace polyec {
RealmView Realm::view(uint64_t observer)const{
    // Validate the caller before filtering: knows() can otherwise encounter an
    // invalid ID only for some fact kinds, making rejection data-dependent.
    (void)person(observer);
    std::vector<Fact> visibleFacts;
    std::unordered_set<uint64_t> visibleFactIds;
    for(const auto& fact:facts_)if(knows(observer,fact)){
        visibleFacts.push_back(fact);visibleFactIds.insert(fact.id);
    }
    std::vector<PlayerKnowledge> playerKnowledge;
    for(const auto& known:knowledge_)if(known.observer==observer&&visibleFactIds.contains(known.fact))
        playerKnowledge.push_back({known.observer,known.fact,known.arrives,known.source_investigation_id,
                                   known.kind,known.confidence_basis_points});
    return detail::project_realm_view(day_,regions_,people_,provinces_,std::move(visibleFacts),
        std::move(playerKnowledge),draft_,total_money());
}
} // namespace polyec
