#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace polyec {

struct EntityId {
    uint64_t value = 0;
    bool operator==(const EntityId &other) const { return value == other.value; }
    bool operator!=(const EntityId &other) const { return value != other.value; }
    bool operator<(const EntityId &other) const { return value < other.value; }
};

struct GameDate {
    int day = 0;
    GameDate() = default;
    explicit GameDate(int d) : day(d) {}
    GameDate &operator+=(int delta) { day += delta; return *this; }
    bool operator==(const GameDate &other) const { return day == other.day; }
    bool operator<(const GameDate &other) const { return day < other.day; }
};

struct Territory {
    EntityId id;
    std::string name;
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
};

// Explicit state container for the authoritative world snapshot.
// This keeps entity data (territories) and mutable runtime state separate,
// making command validation and transactional rollback easier to reason about.
struct WorldState {
    GameDate date;
    EntityId selected;
    std::vector<Territory> territories;
    bool paused = true;
    uint64_t pause_revision = 0;
};

}
