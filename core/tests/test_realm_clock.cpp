#include "polyec/realm.hpp"
#include "polyec/realm_clock.hpp"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
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

        std::cout << "realm clock checks=" << checks << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
}
