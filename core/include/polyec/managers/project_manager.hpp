#pragma once
#include <cstdint>
#include <vector>
#include "../realm.hpp"

namespace polyec {

// Pure count helper matching the original Realm::legal Project criteria
// exactly (d5751bb): a project is active when it is not completed AND in the
// given region AND of the given kind. Realm keeps ownership of projects_.
class ProjectManager {
public:
    static int active_projects_count(const std::vector<Project> &projects, int region, int kind) {
        int active = 0;
        for (const auto &j : projects)
            if (!j.completed && j.region == region && j.kind == kind) ++active;
        return active;
    }
};

}
