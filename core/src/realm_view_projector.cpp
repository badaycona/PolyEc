#include "polyec/realm_view_projector.hpp"
#include <utility>

namespace polyec::detail {

RealmView project_realm_view(int day, int regions,
                             const std::vector<Person>& people,
                             const std::vector<Province>& provinces,
                             std::vector<Fact> visible_facts,
                             std::vector<PlayerKnowledge> player_knowledge,
                             Draft draft, int64_t money) {
    RealmView view{};
    view.day = day;
    view.regions = regions;
    view.persons = people.size();
    view.money = money;
    view.provinces = provinces;
    view.facts = std::move(visible_facts);
    view.knowledge = std::move(player_knowledge);
    view.draft = draft;
    for (const auto& person : people) {
        view.alive += person.alive;
        view.children += person.alive && person.age_days < 18 * 365;
        view.decisions += person.decisions;
    }
    return view;
}

} // namespace polyec::detail
