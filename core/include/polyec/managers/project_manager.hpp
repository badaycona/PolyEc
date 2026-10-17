#pragma once
#include <vector>
#include "../types.hpp"

namespace polyec {

class ProjectManager {
public:
    // Minimal helper to count active projects in a region by kind.
    static int active_projects_count(const std::vector<Project>& projects, int region, int kind) {
        int active = 0;
        for (const auto &j : projects) if (!j.completed && j.region == region && j.kind == kind) ++active;
        return active;
    }
};

}
