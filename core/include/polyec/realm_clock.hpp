#pragma once
#include <cstdint>
#include <optional>
namespace polyec {

// Forward declaration: RealmClockController must not create a include cycle
// with realm.hpp. The controller only holds a reference and delegates to
// Realm's public clock commands; complete Realm type is required in the .cpp.
class Realm;

// Control command for the Realm-owned pause clock. This is a control-plane
// command, not an NPC decision: it changes no money, actors, or strategy.
struct RealmClockCommand {
    enum class Operation { Pause, Resume };
    Operation operation = Operation::Pause;
    // nullopt means unconditional. A value (including 0) is an exact expectation
    // and must equal Realm::pause_revision(); 0 is a valid initial revision.
    std::optional<uint64_t> expected_pause_revision = std::nullopt;

    // Compatibility aliases for call sites/tests that use RealmClockCommand::Pause.
    static constexpr Operation Pause = Operation::Pause;
    static constexpr Operation Resume = Operation::Resume;
};

// Wall-clock adapter around a Realm. Owns only transient timing state; the
// paused flag and revision live in Realm. Never serialized.
class RealmClockController {
public:
    explicit RealmClockController(Realm& realm, double speed = 1.0);

    Realm& realm() const noexcept { return realm_; }

    bool active() const noexcept { return active_; }
    double speed() const noexcept { return speed_; }
    double accumulated_ticks() const noexcept { return accum_ticks_; }
    uint64_t seen_pause_revision() const noexcept { return seen_revision_; }

    void set_active(bool active);
    bool set_paused(bool paused);
    bool set_speed(double speed);
    bool update(double dt);
    bool step_day();
    void reset_after_restore();

private:
    Realm& realm_;
    double speed_ = 1.0;
    double accum_ticks_ = 0.0;
    bool active_ = true;
    uint64_t seen_revision_ = 0;
};

} // namespace polyec
