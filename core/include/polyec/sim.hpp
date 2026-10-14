#pragma once

#include "polyec/types.hpp"
#include "polyec/command.hpp"
#include <vector>
#include <memory>

namespace polyec {

// GameWorld owns the authoritative simulation state (territories, game date,
// pause flag, selection). Every mutation arrives as a Command and returns a
// CommandResult; a rejected command must not change any part of the state.
class GameWorld {
public:
    GameWorld();
    ~GameWorld() = default;

    bool add_territory(const Territory &t);
    const std::vector<Territory> &territories() const;
    EntityId get_selected() const;
    GameDate get_date() const;
    bool is_paused() const;
    uint64_t pause_revision() const { return m_pause_revision; }
    PlayerView player_view() const;

    CommandResult execute(const Command &cmd);

private:
    std::vector<Territory> m_territories;
    EntityId m_selected;
    GameDate m_date;
    bool m_paused = true;
    uint64_t m_pause_revision = 0;
};

// SimulationController owns wall-clock bookkeeping only: fake-time mode, time
// scale and the sub-day accumulator. It NEVER owns the pause flag; the pause
// flag lives in GameWorld and the controller mutates it exclusively through
// SetPause/Resume commands, so UI and AI observe exactly one state.
class SimulationController {
public:
    explicit SimulationController(std::shared_ptr<GameWorld> world);

    // --- fake time -------------------------------------------------------
    // Fake time does not mean "one day per update". It means the elapsed time
    // handed to update() is injected by the caller (test/tool) instead of being
    // sampled from a real monotonic clock. All accumulation rules below still
    // apply, so the day count is a deterministic function of injected time.
    void set_fake_time(bool fake) { m_fake = fake; }
    bool is_fake_time() const { return m_fake; }

    // --- time scale ------------------------------------------------------
    // M0a supports scales from zero through 4x; rejects all other input.
    bool set_time_scale(double s);
    double get_time_scale() const { return m_scale; }

    // --- pause (delegates to the world's authoritative state) ------------
    bool is_paused() const;
    void set_paused(bool p);

    // --- clock -----------------------------------------------------------
    // dt_real_seconds is the elapsed real time since the previous call (or the
    // injected elapsed time when fake time is enabled). Must be finite and >= 0;
    // invalid values are ignored without touching state. A delta larger than
    // kMaxRealDeltaSeconds (sleep, focus loss, hitch) auto-pauses the world and
    // discards the pending time: no catch-up run.
    void update(double dt_real_seconds);

    // Deterministic single-day step for tests/dev tools. Rejected by the world
    // while paused (returns success == false, date unchanged).
    CommandResult step_day();

    // Seconds of accumulated sub-day time still pending (diagnostic / test aid).
    double pending_seconds() const { return m_accum; }

    // 1x == 1 game day per real second (M0a tuning; see docs/opencode-handoff.md).
    static constexpr double kSecondsPerGameDay = 1.0;
    // Reject-and-pause threshold for elapsed time: anything above this is not a
    // frame delta but a gap (window unfocused, machine slept, debugger stopped,
    // huge hitch). The world is paused and the gap is dropped so pause never
    // silently runs the calendar forward. 0.25 s ~= 4 frames at 60 Hz.
    static constexpr double kMaxRealDeltaSeconds = 0.25;
    // Upper bound of days advanced in a single update() call; leftover time is
    // dropped (never queued) so one frame cannot turn into a stall.
    static constexpr int kMaxAdvancePerUpdate = 8;

private:
    std::shared_ptr<GameWorld> m_world;
    bool m_fake = false;
    double m_scale = 1.0;
    double m_accum = 0.0; // sub-day remainder in game-time seconds at scale 1
    uint64_t m_seen_pause_revision = 0;
};

}
