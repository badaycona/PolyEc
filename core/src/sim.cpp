#include "polyec/sim.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace polyec {

GameWorld::GameWorld() {
    m_selected.value = 0;
    m_date.day = 0;
    m_paused = true;
}

bool GameWorld::add_territory(const Territory &t) {
    if (t.id.value == 0 || std::any_of(m_territories.begin(), m_territories.end(),
            [&](const Territory &existing) { return existing.id == t.id; })) return false;
    m_territories.push_back(t);
    return true;
}

PlayerView GameWorld::player_view() const {
    return PlayerView{m_date, m_paused, m_selected, m_territories};
}

const std::vector<Territory> &GameWorld::territories() const {
    return m_territories;
}

EntityId GameWorld::get_selected() const { return m_selected; }
GameDate GameWorld::get_date() const { return m_date; }
bool GameWorld::is_paused() const { return m_paused; }

CommandResult GameWorld::execute(const Command &cmd) {
    CommandResult r;
    switch (cmd.type) {
        case CommandType::SelectTerritory: {
            // validate target exists; reject leaves selection untouched
            auto it = std::find_if(m_territories.begin(), m_territories.end(),
                                   [&](const Territory &x) { return x.id == cmd.target; });
            if (it == m_territories.end()) {
                r.success = false;
                r.reason = "invalid_target";
                r.date_version = m_date.day;
                return r;
            }
            m_selected = cmd.target;
            r.success = true;
            r.reason = "ok";
            r.date_version = m_date.day;
            return r;
        }
        case CommandType::SetPause: {
            if (!m_paused) ++m_pause_revision;
            m_paused = true;
            r.success = true;
            r.reason = "paused";
            r.date_version = m_date.day;
            return r;
        }
        case CommandType::Resume: {
            if (m_paused) ++m_pause_revision;
            m_paused = false;
            r.success = true;
            r.reason = "resumed";
            r.date_version = m_date.day;
            return r;
        }
        case CommandType::AdvanceDay: {
            // paused world never advances the calendar, no matter the caller
            if (m_paused) {
                r.success = false;
                r.reason = "paused";
                r.date_version = m_date.day;
                return r;
            }
            if (m_date.day == std::numeric_limits<int>::max()) {
                return CommandResult{false, "date_limit", m_date.day};
            }
            m_date += 1;
            r.success = true;
            r.reason = "advanced";
            r.date_version = m_date.day;
            return r;
        }
        case CommandType::None:
        default: {
            r.success = false;
            r.reason = "unknown";
            r.date_version = m_date.day;
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
