#include "polyec/realm_clock.hpp"
#include "polyec/realm.hpp"

#include <cmath>
#include <stdexcept>

namespace polyec {

RealmClockController::RealmClockController(Realm& realm, double speed)
    : realm_(realm), active_(true), seen_revision_(realm.pause_revision()) {
    if (!std::isfinite(speed) || speed < 0.0 || speed > 8.0) {
        throw std::invalid_argument("clock speed bounds");
    }
    speed_ = speed;
    accum_ticks_ = 0.0;
    auto paused = realm_.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
    if (!paused.success) throw std::runtime_error("clock pause failed");
    seen_revision_ = realm_.pause_revision();
}

void RealmClockController::set_active(bool active) {
    const bool was_active = active_;
    if (!active) {
        auto paused = realm_.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
        if (!paused.success) throw std::runtime_error("clock pause failed");
        accum_ticks_ = 0.0;
        seen_revision_ = realm_.pause_revision();
        active_ = false;
        return;
    }
    // Regaining activity does not auto-resume the core: it must remain paused
    // and await an explicit manual resume. An unchanged active=true call is a
    // true no-op: do not overwrite seen_revision_, so a later update can detect
    // an external pause/resume transition. Only an actual inactive->active
    // transition forces pause and drops accumulated ticks.
    if (!was_active) {
        auto paused = realm_.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
        if (!paused.success) throw std::runtime_error("clock pause failed");
        accum_ticks_ = 0.0;
        seen_revision_ = realm_.pause_revision();
        active_ = true;
    }
}

bool RealmClockController::set_paused(bool paused) {
    if (paused) {
        auto r = realm_.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
        if (!r.success) return false;
        accum_ticks_ = 0.0;
        seen_revision_ = realm_.pause_revision();
        return true;
    }
    if (!active_) return false;
    auto r = realm_.execute(RealmClockCommand{RealmClockCommand::Operation::Resume});
    if (!r.success) return false;
    accum_ticks_ = 0.0;
    seen_revision_ = realm_.pause_revision();
    return true;
}

bool RealmClockController::set_speed(double speed) {
    if (!std::isfinite(speed) || speed < 0.0 || speed > 8.0) return false;
    speed_ = speed;
    return true;
}

bool RealmClockController::update(double dt) {
    if (!std::isfinite(dt) || dt < 0.0) return false;
    if (!active_) {
        accum_ticks_ = 0.0;
        seen_revision_ = realm_.pause_revision();
        return false;
    }
    if (dt > 0.25) {
        auto paused = realm_.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
        if (!paused.success) throw std::runtime_error("clock pause failed");
        accum_ticks_ = 0.0;
        seen_revision_ = realm_.pause_revision();
        return false;
    }
    uint64_t current_rev = realm_.pause_revision();
    if (current_rev != seen_revision_) {
        accum_ticks_ = 0.0;
        seen_revision_ = current_rev;
    }
    if (realm_.paused()) {
        accum_ticks_ = 0.0;
        seen_revision_ = realm_.pause_revision();
        return false;
    }
    if (speed_ == 0.0) {
        seen_revision_ = realm_.pause_revision();
        return true;
    }
    accum_ticks_ += dt * speed_ * 64.0;
    int64_t ticks = static_cast<int64_t>(std::floor(accum_ticks_));
    if (ticks < 0) ticks = 0;
    if (ticks > 512) ticks = 512;
    if (ticks > 0) {
        try {
            realm_.advance_ticks(ticks);
        } catch (...) {
            accum_ticks_ = 0.0;
            seen_revision_ = realm_.pause_revision();
            return false;
        }
        accum_ticks_ -= static_cast<double>(ticks);
        seen_revision_ = realm_.pause_revision();
    }
    return true;
}

bool RealmClockController::step_day() {
    if (!active_ || realm_.paused()) return false;
    try {
        realm_.advance_ticks(64);
    } catch (...) {
        return false;
    }
    accum_ticks_ = 0.0;
    seen_revision_ = realm_.pause_revision();
    return true;
}

void RealmClockController::reset_after_restore() {
    accum_ticks_ = 0.0;
    seen_revision_ = realm_.pause_revision();
}

} // namespace polyec
