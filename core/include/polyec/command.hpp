#pragma once

#include "polyec/types.hpp"

namespace polyec {

enum class CommandType {
    SelectTerritory,
    AdvanceDay,
    SetPause,
    Resume,
    None
};

struct Command {
    CommandType type = CommandType::None;
    EntityId target;
    bool operator==(const Command &other) const {
        return type == other.type && target == other.target;
    }
};

struct CommandResult {
    bool success = false;
    std::string reason;
    int date_version = 0;
};

// Detached value snapshot; changing it never mutates the simulation.
struct PlayerView {
    GameDate date;
    bool paused = true;
    EntityId selected;
    std::vector<Territory> territories;
};

}
