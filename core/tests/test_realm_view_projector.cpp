#include "polyec/realm_view_projector.hpp"

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    try {
        std::vector<polyec::Person> people(3);
        people[0].id = 1; people[0].alive = true; people[0].age_days = 40 * 365; people[0].decisions = 8;
        people[1].id = 2; people[1].alive = true; people[1].age_days = 17 * 365; people[1].decisions = 2;
        people[2].id = 3; people[2].alive = false; people[2].age_days = 4 * 365; people[2].decisions = 5;
        std::vector<polyec::Province> provinces(2);
        provinces[0].treasury = 1234; provinces[0].stock[0] = 17;
        provinces[1].school = 29; provinces[1].stock[9] = 31;
        std::vector<polyec::Fact> visible{{7, 3, 1, 2, 11, 1, 203, 25, false}};
        std::vector<polyec::PlayerKnowledge> knowledge{{1,7,11,42,polyec::KnowledgeKind::SecretEvidence,-1}};
        polyec::Draft draft{2, {4, 5, 6}, 3, 1, true};

        const auto view = polyec::detail::project_realm_view(11, 1, people, provinces, visible, knowledge, draft, 98765);
        check(view.day == 11 && view.regions == 1 && view.persons == 3, "world dimensions are projected");
        check(view.alive == 2 && view.children == 1 && view.decisions == 15, "population summary matches source records");
        check(view.money == 98765 && view.provinces.size() == 2 && view.provinces[0].stock[0] == 17 &&
              view.provinces[1].school == 29, "money and province snapshot are copied");
        check(view.facts.size() == 1 && view.facts[0].id == 7 && view.draft.tier == 2 &&
              view.draft.cards == std::array<int, 3>{4, 5, 6} && view.draft.chosen == 1 && view.draft.rerolled,
              "visible facts and draft are preserved");
        check(view.knowledge.size()==1&&view.knowledge[0].observer==1&&view.knowledge[0].fact==7&&
              view.knowledge[0].source_investigation_id==42&&
              view.knowledge[0].kind==polyec::KnowledgeKind::SecretEvidence&&
              view.knowledge[0].confidence_basis_points==-1,
              "owner-specific knowledge and its source are preserved");

        const auto empty = polyec::detail::project_realm_view(0, 1, {}, {}, {}, {}, {}, 0);
        check(empty.persons == 0 && empty.alive == 0 && empty.children == 0 && empty.decisions == 0 &&
              empty.provinces.empty() && empty.facts.empty() && empty.knowledge.empty() && empty.money == 0,
              "empty snapshot projects empty summary");
        std::printf("Realm view projector: %d checks passed\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Realm view projector failed after %d checks: %s\n", checks, error.what());
        return 1;
    }
}






