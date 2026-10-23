#include "polyec/realm.hpp"
#include "polyec/realm_clock.hpp"
#include "polyec/clock_section_codec.hpp"
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
namespace {
int checks = 0;
void check(bool cond, const char* msg) {
    ++checks;
    if (!cond) {
        std::cerr << "FAIL: " << msg << "\n";
        throw std::runtime_error(msg);
    }
}
}

int main() {
    try {
        using polyec::Realm;
        using polyec::RealmClockCommand;
        using polyec::RealmClockController;

        // 1. Bare realm defaults unpaused.
        {
            Realm r(1, 1, 128);
            check(!r.paused(), "bare realm unpaused");
            check(r.pause_revision() == 0, "bare realm revision 0");
        }

        // 2. Controller constructor pauses via core command.
        {
            Realm r(1, 1, 128);
            RealmClockController c(r, 1.0);
            check(r.paused(), "controller constructor pauses");
            check(c.active(), "controller active");
            check(c.speed() == 1.0, "controller speed");
            check(c.accumulated_ticks() == 0.0, "controller accum zero");
            check(c.seen_pause_revision() == r.pause_revision(), "controller seen revision");
        }

        // 2b. Constructor invalid speed throws before pausing/modifying Realm.
        {
            for (double sp : {-1.0, (double)NAN, (double)INFINITY, 9.0}) {
                Realm r(90, 1, 128);
                auto fp = r.fingerprint();
                bool paused = r.paused();
                uint64_t rev = r.pause_revision();
                bool threw = false;
                try {
                    RealmClockController c(r, sp);
                    (void)c;
                } catch (const std::exception&) {
                    threw = true;
                }
                check(threw, "invalid constructor speed throws");
                check(r.fingerprint() == fp, "invalid constructor speed fingerprint unchanged");
                check(r.paused() == paused, "invalid constructor speed paused unchanged");
                check(r.pause_revision() == rev, "invalid constructor speed revision unchanged");
            }
        }

        // 3. Subtick retention and phase boundary.
        {
            Realm r(2, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume allowed");
            check(!r.paused(), "core unpaused");
            int64_t t0 = r.game_tick();
            int d0 = r.day();
            check(c.update(1.0 / 128.0), "subtick update accepted");
            check(r.game_tick() == t0, "subtick no advance");
            check(c.accumulated_ticks() > 0.49 && c.accumulated_ticks() < 0.51, "fraction retained");
            check(c.update(1.0 / 128.0), "second subtick accepted");
            check(r.game_tick() == t0 + 1, "two halves advance one");
            check(r.day() == d0, "no day boundary");
        }

        // 4. One-second phase boundary.
        {
            Realm r(3, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            int64_t t0 = r.game_tick();
            int d0 = r.day();
            bool ok = true;
            for (int i = 0; i < 8; ++i) ok = c.update(0.125) && ok;
            check(ok, "eight small updates");
            check(r.game_tick() == t0 + 64, "64 ticks");
            check(r.day() == d0 + 1, "one day");
        }

        // 5. Focus loss pauses and drops accum; regain stays paused.
        {
            Realm r(4, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            check(c.update(0.1), "update before focus");
            c.set_active(false);
            check(!c.active(), "inactive");
            check(r.paused(), "focus loss pauses");
            check(c.accumulated_ticks() == 0.0, "focus drops accum");
            int64_t before = r.game_tick();
            check(!c.update(0.1), "inactive update rejected");
            check(r.game_tick() == before, "no advance while inactive");
            c.set_active(true);
            check(c.active(), "active regained");
            check(r.paused(), "remains paused after regain");
            check(c.set_paused(true), "pause idempotent while paused");
            check(c.set_paused(false), "manual resume allowed");
            check(!r.paused(), "resumed");
        }

        // 6. Resume refused while inactive.
        {
            Realm r(5, 1, 128);
            RealmClockController c(r, 1.0);
            c.set_active(false);
            check(!c.set_paused(false), "resume refused while inactive");
            check(r.paused(), "still paused");
        }

        // 7. Long gap pauses and drops accum.
        {
            Realm r(6, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            check(c.update(0.1), "small update");
            check(!c.update(0.5), "long gap rejected/paused");
            check(r.paused(), "long gap pauses");
            check(c.accumulated_ticks() == 0.0, "long gap drops accum");
        }

        // 8. No catch-up after pause.
        {
            Realm r(7, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            check(c.update(0.1), "update");
            int64_t before = r.game_tick();
            check(!c.update(0.5), "gap pauses");
            check(c.set_paused(false), "resume");
            check(c.update(0.05), "post-resume update");
              // 0.05s = 3.2 ticks -> 3 ticks, no backlog from 0.5s
              check(r.game_tick() == before + 3, "no catch-up");

        }

        // 9. Scales 0,1,4,8.
        {
            for (double sp : {0.0, 1.0, 4.0, 8.0}) {
                Realm r(8, 1, 128);
                RealmClockController c(r, sp);
                check(c.set_paused(false), "resume");
                int64_t before = r.game_tick();
                check(c.update(0.25), "scale update");
                int expected = static_cast<int>(std::floor(0.25 * sp * 64.0));
                check(r.game_tick() - before == expected, "scale ticks");
            }
        }

        // 10. Speed invalid no mutation.
        {
            Realm r(9, 1, 128);
            RealmClockController c(r, 2.0);
            double sp = c.speed();
            double acc = c.accumulated_ticks();
            check(!c.set_speed(-1.0), "negative speed rejected");
            check(!c.set_speed(9.0), "too high speed rejected");
            check(!c.set_speed(NAN), "nan speed rejected");
            check(!c.set_speed(INFINITY), "inf speed rejected");
            check(c.speed() == sp, "speed unchanged");
            check(c.accumulated_ticks() == acc, "accum unchanged");
        }

        // 11. dt invalid no mutation.
        {
            Realm r(10, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            int64_t before = r.game_tick();
            double acc = c.accumulated_ticks();
            check(!c.update(-0.1), "negative dt rejected");
            check(!c.update(NAN), "nan dt rejected");
            check(!c.update(INFINITY), "inf dt rejected");
            check(r.game_tick() == before, "tick unchanged");
            check(c.accumulated_ticks() == acc, "accum unchanged");
        }

        // 12. External pause+resume between updates drops accum.
        {
            Realm r(11, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            check(c.update(0.2), "accum 12.8->12");
            check(c.accumulated_ticks() > 0.7, "accum remainder");
            // external pause+resume
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "ext pause");
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Resume}).success, "ext resume");
            int64_t before = r.game_tick();
            check(c.update(0.01), "post-external update");
            // remainder dropped; 0.01s=0.64 ticks -> 0
            check(r.game_tick() == before, "no stale accum used");
        }

        // 12b. Controller resume after external pause clears stale accum.
        {
            Realm r(20, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            check(c.update(0.2), "accum update");
            check(c.accumulated_ticks() > 0.7, "accum remainder");
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "external pause");
            check(c.set_paused(false), "controller resume after external pause");
            check(c.accumulated_ticks() == 0.0, "controller resume clears accum");
            int64_t before = r.game_tick();
            check(c.update(0.01), "post-resume update");
            check(r.game_tick() == before, "no stale accum used");
        }

        // 12c. Regain active pauses even after direct core resume; no catch-up.
        {
            Realm r(21, 1, 128);
            RealmClockController c(r, 1.0);
            c.set_active(false);
            check(!c.active(), "inactive");
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Resume}).success, "direct core resume while inactive");
            check(!r.paused(), "core unpaused by direct resume");
            c.set_active(true);
            check(c.active(), "active regained");
            check(r.paused(), "remains paused awaiting manual resume");
            check(c.accumulated_ticks() == 0.0, "regain drops accum");
            int64_t before = r.game_tick();
            check(!c.update(0.1), "update refused while paused");
            check(r.game_tick() == before, "no catch-up");
            check(c.set_paused(false), "manual resume");
            check(!r.paused(), "manually resumed");
        }

        // 13. Direct core pause rejects positive advance.
        {
            Realm r(12, 1, 128);
            int64_t t0 = r.game_tick();
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "pause");
            auto fp = r.fingerprint();
            bool threw = false;
            try { r.advance_ticks(5); } catch (const std::runtime_error&) { threw = true; }
            check(threw, "positive advance throws while paused");
            check(r.fingerprint() == fp, "fingerprint unchanged");
            check(r.game_tick() == t0, "cursor unchanged");
            // zero no-op allowed
            r.advance_ticks(0);
            check(r.game_tick() == t0, "zero no-op");
        }

        // 14. advance(days) paused reject.
        {
            Realm r(13, 1, 128);
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "pause");
            auto fp = r.fingerprint();
            bool threw = false;
            try { r.advance(1); } catch (const std::runtime_error&) { threw = true; }
            check(threw && r.fingerprint() == fp, "advance days paused reject");
            // zero days no-op
            r.advance(0);
            check(r.fingerprint() == fp, "zero days no-op");
        }

        // 15. expected revision 0 is valid initially, then stale after transition.
        {
            Realm r(14, 1, 128);
            auto first = r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause, 0});
            check(first.success, "expected 0 succeeds initially");
            auto fp = r.fingerprint();
            RealmClockCommand stale{RealmClockCommand::Operation::Resume, 0};
            auto res = r.execute(stale);
            check(!res.success, "expected 0 stale after transition");
            check(r.paused(), "still paused");
            check(r.fingerprint() == fp, "fingerprint unchanged");
        }

        // 16. Idempotent pause/resume no revision increment.
        {
            Realm r(15, 1, 128);
            uint64_t r0 = r.pause_revision();
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "pause");
            uint64_t r1 = r.pause_revision();
            check(r1 == r0 + 1, "pause increments");
            // idempotent pause
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "pause again");
            check(r.pause_revision() == r1, "idempotent no increment");
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Resume}).success, "resume");
            check(r.pause_revision() == r1 + 1, "resume increments");
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Resume}).success, "resume again");
            check(!r.paused(), "unpaused");
        }

        // 17. Unknown operation rejected (cast).
        {
            Realm r(16, 1, 128);
            auto fp = r.fingerprint();
            uint64_t rev = r.pause_revision();
            RealmClockCommand cmd{};
            cmd.operation = static_cast<RealmClockCommand::Operation>(999);
            auto res = r.execute(cmd);
            check(!res.success, "unknown op rejected");
            check(r.pause_revision() == rev, "revision unchanged");
            check(r.fingerprint() == fp, "fingerprint unchanged TEST13");
        }

        // 18. step_day only when unpaused+active.
        {
            Realm r(17, 1, 128);
            RealmClockController c(r, 1.0);
            int64_t t0 = r.game_tick();
            check(!c.step_day(), "step_day refused while paused");
            check(r.game_tick() == t0, "no step while paused");
            check(c.set_paused(false), "resume");
            check(c.step_day(), "step_day allowed");
            check(r.game_tick() == t0 + 64, "step_day +64");
            // inactive
            c.set_active(false);
            int64_t t1 = r.game_tick();
            check(!c.step_day(), "step_day refused while inactive");
            check(r.game_tick() == t1, "no step while inactive");
        }

        // 19. Save/load preserves paused flag, revision, cursor; reset_after_restore.
        {
            const std::string path = "m5_t1b1_clock_save.bin";
            const std::string bundle = "test";
            Realm r(18, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            check(c.update(0.25), "update 16 ticks");
            check(c.set_paused(true), "pause");
            std::string err;
            check(r.save(path, bundle, err), "save ok");
            Realm r2(1, 1, 128);
            check(r2.load(path, bundle, err), "load ok");
            check(r2.paused() == r.paused(), "paused preserved");
            check(r2.pause_revision() == r.pause_revision(), "revision preserved");
            check(r2.game_tick() == r.game_tick(), "cursor preserved");
            RealmClockController c2(r2, 1.0);
            // constructor paused (idempotent if already paused)
            c2.reset_after_restore();
            check(c2.accumulated_ticks() == 0.0, "reset clears accum");
            check(c2.seen_pause_revision() == r2.pause_revision(), "reset syncs revision");

            // reset clears residual even if restored revision equals old seen.
            c2.set_paused(false);
            check(c2.update(0.2), "accum after restore");
            check(c2.accumulated_ticks() > 0.7, "residual present");
            c2.reset_after_restore();
            check(c2.accumulated_ticks() == 0.0, "reset clears residual same revision");

            // pure paused + cursor roundtrip.
            Realm r3(1, 1, 128);
            check(r3.load(path, bundle, err), "reload ok");
            check(r3.paused(), "pure paused preserved");
            check(r3.game_tick() == r.game_tick(), "pure cursor preserved");
        }

        // 19b. Malformed CLOCK_1 rejected.
        {
            const std::string bundle = "test";
            Realm r(18, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            check(c.set_paused(true), "pause");
            std::string data = r.serialize(bundle);
            std::string err;
            // wrong tag
            {
                std::string bad = data;
                size_t pos = bad.find("CLOCK_1");
                check(pos != std::string::npos, "clock tag present");
                bad.replace(pos, 7, "CLOCK_X");
                Realm w(1, 1, 128);
                check(!w.deserialize(bad, bundle, err), "wrong clock tag rejected");
            }
            // wrong bool
            {
                std::string bad = data;
                size_t pos = bad.find("CLOCK_1 1");
                if (pos == std::string::npos) pos = bad.find("CLOCK_1 0");
                check(pos != std::string::npos, "clock bool present");
                // flip the single flag digit after tag+space
                size_t digit = bad.find_first_of("01", pos + 7);
                check(digit != std::string::npos, "clock flag digit present");
                bad[digit] = (bad[digit] == '1') ? '2' : '2';
                Realm w(1, 1, 128);
                check(!w.deserialize(bad, bundle, err), "wrong clock bool rejected");
            }
            // truncated
            {
                std::string bad = data.substr(0, data.size() - 5);
                Realm w(1, 1, 128);
                check(!w.deserialize(bad, bundle, err), "truncated clock rejected");
            }
        }

        // 20. Phase boundaries no duplicate day.
        {
            Realm r(19, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resume");
            int d0 = r.day();
            bool ok = true;
            for (int i = 0; i < 8; ++i) ok = c.update(0.125) && ok;
            check(ok, "update 64");
            check(r.day() == d0 + 1, "day +1");
            for (int i = 0; i < 4; ++i) ok = c.update(0.125) && ok;
            check(ok, "update 32");
            check(r.day() == d0 + 1, "no duplicate day");
            for (int i = 0; i < 4; ++i) ok = c.update(0.125) && ok;
            check(ok, "update 32");
            check(r.day() == d0 + 2, "day +2");
        }

        // T1b1 review fix: unchanged active=true is a no-op, so external
        // pause/resume transition is detected and stale fraction discarded.
        {
            Realm r(22, 1, 128);
            RealmClockController c(r, 1.0);
            check(c.set_paused(false), "resumed clock");
            check(c.update(1.0 / 128.0), "resumed clock update 1/128");
            check(c.accumulated_ticks() > 0.49 && c.accumulated_ticks() < 0.51, "resumed clock 0.5 ticks");
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "external realm pause");
            check(r.execute(RealmClockCommand{RealmClockCommand::Operation::Resume}).success, "external realm resume");
            int64_t before = r.game_tick();
            c.set_active(true);
            check(c.active(), "already active stays active");
            check(c.update(1.0 / 128.0), "update after external pause/resume");
            check(r.game_tick() == before, "old fraction discarded, game_tick remains 0");
            check(c.accumulated_ticks() > 0.49 && c.accumulated_ticks() < 0.51, "new pending 0.5");
            check(c.update(1.0 / 128.0), "next update");
            check(r.game_tick() == before + 1, "next 1/128 advances 1");
        }

        // T1b1 overflow: core Pause at UINT64_MAX rejects atomically.
        {
            const std::string bundle = "test";
            const std::string maxstr = std::to_string(std::numeric_limits<uint64_t>::max());
            Realm base(100, 1, 128);
            std::string data = base.serialize(bundle);
            const std::string needle = "CLOCK_1 0 0";
            size_t pos = data.find(needle);
            check(pos != std::string::npos, "unpaused clock needle present");
            std::string beforeState = data;
            beforeState.replace(pos, needle.size(), "CLOCK_1 0 " + maxstr);
            Realm w(1, 1, 128);
            std::string err;
            check(w.deserialize(beforeState, bundle, err), "unpaused MAX deserialize ok");
            check(!w.paused(), "unpaused MAX state");
            check(w.pause_revision() == std::numeric_limits<uint64_t>::max(), "unpaused MAX revision");
            std::string serializedBefore = w.serialize(bundle);
            uint64_t rev = w.pause_revision();
            auto res = w.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
            check(!res.success, "Pause overflow rejected");
            check(res.reason == "Pause revision overflow", "Pause overflow reason exact");
            check(!w.paused(), "Pause overflow leaves unpaused");
            check(w.pause_revision() == rev, "Pause overflow revision unchanged");
            check(w.serialize(bundle) == serializedBefore, "Pause overflow serialized state unchanged");
        }

        // T1b1 overflow: core Resume at UINT64_MAX rejects atomically.
        {
            const std::string bundle = "test";
            const std::string maxstr = std::to_string(std::numeric_limits<uint64_t>::max());
            Realm base(101, 1, 128);
            check(base.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "base pause for Resume overflow");
            std::string data = base.serialize(bundle);
            const std::string needle = "CLOCK_1 1 1";
            size_t pos = data.find(needle);
            check(pos != std::string::npos, "paused clock needle present");
            std::string beforeState = data;
            beforeState.replace(pos, needle.size(), "CLOCK_1 1 " + maxstr);
            Realm w(1, 1, 128);
            std::string err;
            check(w.deserialize(beforeState, bundle, err), "paused MAX deserialize ok");
            check(w.paused(), "paused MAX state");
            check(w.pause_revision() == std::numeric_limits<uint64_t>::max(), "paused MAX revision");
            std::string serializedBefore = w.serialize(bundle);
            uint64_t rev = w.pause_revision();
            auto res = w.execute(RealmClockCommand{RealmClockCommand::Operation::Resume});
            check(!res.success, "Resume overflow rejected");
            check(res.reason == "Pause revision overflow", "Resume overflow reason exact");
            check(w.paused(), "Resume overflow leaves paused");
            check(w.pause_revision() == rev, "Resume overflow revision unchanged");
            check(w.serialize(bundle) == serializedBefore, "Resume overflow serialized state unchanged");
        }

        // T1b1 overflow: controller update must surface failed Pause at UINT64_MAX.
        {
            const std::string bundle = "test";
            const uint64_t maxrev = std::numeric_limits<uint64_t>::max();
            const std::string maxstr = std::to_string(maxrev);
            Realm base(102, 1, 128);
            check(base.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "base pause for controller overflow");
            std::string data = base.serialize(bundle);
            const std::string needle = "CLOCK_1 1 1";
            size_t pos = data.find(needle);
            check(pos != std::string::npos, "controller overflow clock needle present");
            std::string beforeState = data;
            beforeState.replace(pos, needle.size(), "CLOCK_1 1 " + std::to_string(maxrev - 1));
            Realm w(1, 1, 128);
            std::string err;
            check(w.deserialize(beforeState, bundle, err), "paused MAX-1 deserialize ok");
            check(w.paused(), "paused MAX-1 state");
            check(w.pause_revision() == maxrev - 1, "paused MAX-1 revision");
            RealmClockController c(w, 1.0);
            check(w.paused(), "controller constructor idempotent pause");
            check(w.pause_revision() == maxrev - 1, "constructor revision unchanged");
            check(c.seen_pause_revision() == maxrev - 1, "constructor seen revision");
            check(c.set_paused(false), "resume to MAX");
            check(!w.paused(), "unpaused at MAX");
            check(w.pause_revision() == maxrev, "revision MAX after resume");
            check(c.accumulated_ticks() == 0.0, "accum cleared by resume");
            check(c.update(1.0 / 128.0), "small update creates residual");
            check(c.accumulated_ticks() > 0.49 && c.accumulated_ticks() < 0.51, "residual accum present");

            int64_t beforeTick = w.game_tick();
            double beforeAccum = c.accumulated_ticks();
            uint64_t beforeRev = w.pause_revision();
            bool beforePaused = w.paused();
            std::string serializedBefore = w.serialize(bundle);
            uint64_t beforeSeen = c.seen_pause_revision();

            bool threw = false;
            try {
                c.update(0.5);
            } catch (const std::runtime_error& e) {
                threw = true;
                check(std::string(e.what()) == "clock pause failed", "controller overflow throw reason");
            }
            check(threw, "controller update overflow throws");
            check(w.game_tick() == beforeTick, "controller overflow no tick advance");
            check(c.accumulated_ticks() == beforeAccum, "controller overflow accum unchanged");
            check(w.pause_revision() == beforeRev, "controller overflow revision unchanged");
            check(w.paused() == beforePaused, "controller overflow paused state unchanged");
            check(w.serialize(bundle) == serializedBefore, "controller overflow serialized state unchanged");
            check(c.seen_pause_revision() == beforeSeen, "controller overflow seen revision unchanged");
        }

        // T1b1 overflow: already-paused at MAX follows idempotent long-gap path.
        {
            const std::string bundle = "test";
            const uint64_t maxrev = std::numeric_limits<uint64_t>::max();
            const std::string maxstr = std::to_string(maxrev);
            Realm base(103, 1, 128);
            check(base.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success, "base pause for idempotent MAX");
            std::string data = base.serialize(bundle);
            const std::string needle = "CLOCK_1 1 1";
            size_t pos = data.find(needle);
            check(pos != std::string::npos, "idempotent MAX clock needle present");
            std::string beforeState = data;
            beforeState.replace(pos, needle.size(), "CLOCK_1 1 " + maxstr);
            Realm w(1, 1, 128);
            std::string err;
            check(w.deserialize(beforeState, bundle, err), "paused MAX deserialize ok");
            check(w.paused(), "paused MAX state");
            check(w.pause_revision() == maxrev, "paused MAX revision");
            RealmClockController c(w, 1.0);
            check(c.accumulated_ticks() == 0.0, "idempotent MAX accum starts zero");
            check(!c.update(0.5), "idempotent MAX long gap returns false");
            check(w.paused(), "idempotent MAX remains paused");
            check(c.accumulated_ticks() == 0.0, "idempotent MAX accum dropped");
        }

        // StepOneDay clock command: exact one day, paused state/revision unchanged.
        {
            Realm r(900, 1, 128);
            r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
            int64_t t0 = r.game_tick();
            int d0 = r.day();
            uint64_t rev0 = r.pause_revision();
            bool paused0 = r.paused();
            auto result = r.execute(RealmClockCommand{RealmClockCommand::Operation::StepOneDay, rev0});
            check(result.success, "StepOneDay succeeds while paused");
            check(r.paused() == paused0, "StepOneDay leaves paused unchanged");
            check(r.pause_revision() == rev0, "StepOneDay leaves revision unchanged");
            check(r.game_tick() == t0 + 64, "StepOneDay advances exactly 64 ticks");
            check(r.day() == d0 + 1, "StepOneDay advances exactly one day");
        }
        {
            Realm r(901, 1, 128);
            int64_t t0 = r.game_tick();
            int d0 = r.day();
            uint64_t rev0 = r.pause_revision();
            auto result = r.execute(RealmClockCommand{RealmClockCommand::Operation::StepOneDay, rev0});
            check(result.success, "StepOneDay succeeds while unpaused");
            check(!r.paused(), "StepOneDay leaves unpaused unchanged");
            check(r.pause_revision() == rev0, "StepOneDay unpaused revision unchanged");
            check(r.game_tick() == t0 + 64, "StepOneDay unpaused 64 ticks");
            check(r.day() == d0 + 1, "StepOneDay unpaused one day");
        }
        // Stale expected revision rejects without time mutation.
        {
            Realm r(902, 1, 128);
            r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
            uint64_t stale = r.pause_revision();
            r.execute(RealmClockCommand{RealmClockCommand::Operation::Resume});
            r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
            int64_t t0 = r.game_tick();
            auto result = r.execute(RealmClockCommand{RealmClockCommand::Operation::StepOneDay, stale});
            check(!result.success, "stale StepOneDay rejected");
            check(r.game_tick() == t0, "stale StepOneDay no tick mutation");
            check(r.paused(), "stale StepOneDay leaves paused");
        }
        // Ordinary advance still rejects while paused.
        {
            Realm r(903, 1, 128);
            r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
            int64_t t0 = r.game_tick();
            bool threw = false;
            try { r.advance(1); } catch (const std::runtime_error&) { threw = true; }
            check(threw, "ordinary advance throws while paused");
            check(r.game_tick() == t0, "ordinary advance no tick while paused");
            check(r.paused(), "ordinary advance leaves paused");
        }
        // Horizon boundary: one final day is allowed, then StepOneDay rejects without time mutation.
        {
            Realm r(904, 1, 128);
            r.advance_ticks(364999LL * 64);
            check(r.game_tick() == 364999LL * 64, "horizon fixture advanced");
            r.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
            uint64_t rev0 = r.pause_revision();
            auto finalDay = r.execute(RealmClockCommand{RealmClockCommand::Operation::StepOneDay, rev0});
            check(finalDay.success, "horizon final StepOneDay succeeds");
            check(r.game_tick() == 365000LL * 64, "horizon final StepOneDay reaches limit");
            int64_t t0 = r.game_tick();
            auto result = r.execute(RealmClockCommand{RealmClockCommand::Operation::StepOneDay, rev0});
            check(!result.success, "horizon StepOneDay rejected");
            check(result.reason == "ticks bounds", "horizon StepOneDay reason exact");
            check(r.game_tick() == t0, "horizon StepOneDay no tick mutation");
            check(r.pause_revision() == rev0, "horizon StepOneDay revision unchanged");
        }

        // R4 CLOCK_1 codec checks.
        {
            // Exact canonical bytes.
            {
                std::ostringstream out;
                polyec::detail::write_clock_section(out, true, 42);
                check(out.str() == " CLOCK_1 1 42 ", "clock codec canonical bytes");
            }
            // Round trips, including 0 and UINT64_MAX.
            {
                struct Case { bool paused; uint64_t rev; };
                Case cases[] = {{false, 0}, {true, 42}, {false, std::numeric_limits<uint64_t>::max()}, {true, std::numeric_limits<uint64_t>::max()}};
                for (auto& c : cases) {
                    std::ostringstream out;
                    polyec::detail::write_clock_section(out, c.paused, c.rev);
                    std::istringstream in(out.str());
                    bool rp = !c.paused;
                    uint64_t rr = c.rev ^ 1ULL;
                    polyec::detail::read_clock_section(in, rp, rr);
                    check(rp == c.paused, "clock codec roundtrip paused");
                    check(rr == c.rev, "clock codec roundtrip revision");
                }
            }
            // Failure cases preserve outputs.
            auto expect_fail = [&](const std::string& input, bool pausedIn, uint64_t revIn) {
                std::istringstream in(input);
                bool p = pausedIn;
                uint64_t r = revIn;
                bool threw = false;
                try {
                    polyec::detail::read_clock_section(in, p, r);
                } catch (const std::runtime_error& e) {
                    threw = (std::string(e.what()) == "Save clock header");
                } catch (...) {
                    threw = false;
                }
                check(threw, "clock codec failure throws Save clock header");
                check(p == pausedIn, "clock codec failure preserves paused");
                check(r == revIn, "clock codec failure preserves revision");
            };
            expect_fail("CLOCK_X 1 0", false, 7);
            expect_fail("CLOCK_1 2 0", false, 7);
            expect_fail("CLOCK_1", false, 7);
            expect_fail("CLOCK_1 1", false, 7);
            expect_fail("CLOCK_1 1 not-a-number", false, 7);
            expect_fail("CLOCK_1 1 18446744073709551616", false, 7);
            expect_fail("", false, 7);
            // Negative token parses to UINT64_MAX under MSVC extraction.
            {
                std::istringstream in("CLOCK_1 1 -1");
                bool p = false;
                uint64_t r = 7;
                polyec::detail::read_clock_section(in, p, r);
                check(p, "clock codec negative token paused");
                check(r == std::numeric_limits<uint64_t>::max(), "clock codec negative token UINT64_MAX");
            }
        }

        // R4 CLOCK_1 Realm-level malformed checks.
        {
            const std::string bundle = "test";
            Realm base(18, 1, 128);
            RealmClockController c(base, 1.0);
            check(c.set_paused(false), "resume");
            check(c.set_paused(true), "pause");
            std::string data = base.serialize(bundle);
            // tag mutation
            {
                std::string bad = data;
                size_t pos = bad.find("CLOCK_1");
                check(pos != std::string::npos, "clock tag present");
                bad.replace(pos, 7, "CLOCK_X");
                Realm dest(1, 1, 128);
                auto fp = dest.fingerprint();
                std::string err;
                check(!dest.deserialize(bad, bundle, err), "realm clock tag rejected");
                check(err == "Save clock header", "realm clock tag error text");
                check(dest.fingerprint() == fp, "realm clock tag dest fingerprint unchanged");
            }
            // flag mutation to 2
            {
                std::string bad = data;
                size_t pos = bad.find("CLOCK_1");
                check(pos != std::string::npos, "clock tag present");
                size_t digit = bad.find_first_of("01", pos + 7);
                check(digit != std::string::npos, "clock flag digit present");
                bad[digit] = '2';
                Realm dest(1, 1, 128);
                auto fp = dest.fingerprint();
                std::string err;
                check(!dest.deserialize(bad, bundle, err), "realm clock flag rejected");
                check(err == "Save clock header", "realm clock flag error text");
                check(dest.fingerprint() == fp, "realm clock flag dest fingerprint unchanged");
            }
            // revision truncated
            {
                std::string bad = data;
                size_t pos = bad.find("CLOCK_1");
                check(pos != std::string::npos, "clock tag present");
                bad = bad.substr(0, pos + 9);
                Realm dest(1, 1, 128);
                auto fp = dest.fingerprint();
                std::string err;
                check(!dest.deserialize(bad, bundle, err), "realm clock revision truncated rejected");
                check(err == "Save clock header", "realm clock revision truncated error text");
                check(dest.fingerprint() == fp, "realm clock revision truncated dest fingerprint unchanged");
            }
        }

        std::cout << "realm clock checks=" << checks << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
}






