#include "polyec/sim.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace polyec {
namespace {
bool territory_exists(const std::vector<Territory> &territories, const EntityId &id) {
    return std::any_of(territories.begin(), territories.end(),
        [&](const Territory &existing) { return existing.id == id; });
}
}

GameWorld::GameWorld() {
    m_state.selected.value = 0;
    m_state.date.day = 0;
    m_state.paused = true;
    m_state.pause_revision = 0;
}

bool GameWorld::add_territory(const Territory &t) {
    if (t.id.value == 0 || territory_exists(m_state.territories, t.id)) return false;
    m_state.territories.push_back(t);
    return true;
}

PlayerView GameWorld::player_view() const {
    return PlayerView{m_state.date, m_state.paused, m_state.selected, m_state.territories};
}

const std::vector<Territory> &GameWorld::territories() const {
    return m_state.territories;
}

EntityId GameWorld::get_selected() const { return m_state.selected; }
GameDate GameWorld::get_date() const { return m_state.date; }
bool GameWorld::is_paused() const { return m_state.paused; }

CommandResult GameWorld::execute(const Command &cmd) {
    const auto snapshot = this->snapshot();
    CommandResult r;
    switch (cmd.type) {
        case CommandType::SelectTerritory: {
            if (!territory_exists(m_state.territories, cmd.target)) {
                r.success = false;
                r.reason = "invalid_target";
                r.date_version = m_state.date.day;
                restore(snapshot);
                return r;
            }
            m_state.selected = cmd.target;
            r.success = true;
            r.reason = "ok";
            r.date_version = m_state.date.day;
            return r;
        }
        case CommandType::SetPause: {
            if (!m_state.paused) ++m_state.pause_revision;
            m_state.paused = true;
            r.success = true;
            r.reason = "paused";
            r.date_version = m_state.date.day;
            return r;
        }
        case CommandType::Resume: {
            if (m_state.paused) ++m_state.pause_revision;
            m_state.paused = false;
            r.success = true;
            r.reason = "resumed";
            r.date_version = m_state.date.day;
            return r;
        }
        case CommandType::AdvanceDay: {
            if (m_state.paused) {
                r.success = false;
                r.reason = "paused";
                r.date_version = m_state.date.day;
                restore(snapshot);
                return r;
            }
            if (m_state.date.day == std::numeric_limits<int>::max()) {
                restore(snapshot);
                return CommandResult{false, "date_limit", m_state.date.day};
            }
            m_state.date += 1;
            r.success = true;
            r.reason = "advanced";
            r.date_version = m_state.date.day;
            return r;
        }
        case CommandType::None:
        default: {
            r.success = false;
            r.reason = "unknown";
            r.date_version = m_state.date.day;
            restore(snapshot);
            return r;
        }
    }
}

// ---------------------------------------------------------------------------
// SimulationController
// ---------------------------------------------------------------------------

SimulationController::SimulationController(std::shared_ptr<GameWorld> world)
    : m_world(world), m_fake(false), m_scale(1.0), m_accum(0.0) {
    if (!m_world) throw std::invalid_argument("SimulationController requires a world");
    m_seen_pause_revision = m_world->pause_revision();
}

bool SimulationController::set_time_scale(double s) {
    // reject NaN / Inf / negative: keep the last known-good scale
    if (!std::isfinite(s) || s < 0.0 || s > 4.0) {
        return false;
    }
    m_scale = s;
    return true;
}

bool SimulationController::is_paused() const {
    // single source of truth: the world's pause state
    return m_world->is_paused();
}

void SimulationController::set_paused(bool p) {
    if (p) {
        m_world->execute(Command{CommandType::SetPause, EntityId{}});
    } else {
        m_world->execute(Command{CommandType::Resume, EntityId{}});
    }
    // Explicit pause/resume is a decision point: drop whatever sub-day time was
    // pending so resuming never replays the time spent paused.
    m_accum = 0.0;
    m_seen_pause_revision = m_world->pause_revision();
}

void SimulationController::update(double dt_real_seconds) {
    // 1) reject non-finite or negative elapsed time: no state change at all
    if (!std::isfinite(dt_real_seconds) || dt_real_seconds < 0.0) {
        return;
    }

    // Observe transitions even when pause/resume both occurred between ticks.
    if (m_seen_pause_revision != m_world->pause_revision()) {
        m_accum = 0.0;
        m_seen_pause_revision = m_world->pause_revision();
    }

    // 2) long gap (sleep, focus loss, machine wake, debugger stop): pause and
    //    drop the gap instead of catching the calendar up
    if (dt_real_seconds > kMaxRealDeltaSeconds) {
        set_paused(true);
        return;
    }

    // 3) normal frame: paused world does not accumulate or advance
    if (m_world->is_paused()) {
        return;
    }

    m_accum += dt_real_seconds * m_scale;
    if (!std::isfinite(m_accum)) {
        m_accum = 0.0;
        set_paused(true);
        return;
    }

    int advanced = 0;
    while (m_accum >= kSecondsPerGameDay && advanced < kMaxAdvancePerUpdate) {
        m_accum -= kSecondsPerGameDay;
        CommandResult r = m_world->execute(Command{CommandType::AdvanceDay, EntityId{}});
        ++advanced;
        if (!r.success) {
            // world rejected the advance (it is paused now): drop the residual
            m_accum = 0.0;
            return;
        }
    }

    // Any remaining whole days beyond the per-frame cap are dropped, never
    // queued for a later frame.
    if (m_accum >= kSecondsPerGameDay) {
        m_accum = 0.0;
    }
}

CommandResult SimulationController::step_day() {
    // Single command through the same pipeline as UI input: rejected while
    // paused, no partial state change. The sub-day accumulator is untouched so
    // fractional time keeps accumulating as usual.
    return m_world->execute(Command{CommandType::AdvanceDay, EntityId{}});
}

}
