#pragma once

#include "polyec/realm.hpp"
#include <vector>

namespace polyec::detail {

// Projects a read-only Realm snapshot into the value passed to UI/runtime.
// Visibility filtering belongs to Realm; visible_facts is already filtered.
RealmView project_realm_view(int day, int regions,
                             const std::vector<Person>& people,
                             const std::vector<Province>& provinces,
                             std::vector<Fact> visible_facts,
                             std::vector<PlayerKnowledge> player_knowledge,
                             Draft draft, int64_t money);

} // namespace polyec::detail
