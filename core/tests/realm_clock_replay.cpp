#include "polyec/realm.hpp"
#include "polyec/realm_clock.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int checks = 0;
void check(bool cond, const char* msg) {
    ++checks;
    if (!cond) throw std::runtime_error(msg);
}
std::string sessionsCanonical(const polyec::Realm& w) {
    std::ostringstream out;
    const auto& book = w.schedule_book();
    std::vector<std::pair<uint64_t, polyec::ActivitySession>> rows;
    for (const auto& [actor, sessions] : book.sessions)
        for (const auto& s : sessions) rows.push_back({actor, s});
    std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second.id < b.second.id;
    });
    for (const auto& [actor, s] : rows) {
        out << "actor=" << actor
            << " id=" << s.id
            << " begin=" << s.begin
            << " end=" << s.end
            << " priority=" << s.priority
            << " demand_rate=" << s.demand_rate
            << " requested=" << s.requested
            << " delivered=" << s.delivered
            << " unmet=" << s.unmet
            << " capacity_unmet=" << s.capacity_unmet
            << " unresolved_unmet=" << s.unresolved_unmet
            << " retired=" << (s.retired ? 1 : 0)
            << " retired_tick=" << s.retired_tick
            << " retired_cause=" << s.retired_cause
            << " kind=" << int(s.kind)
            << " region=" << s.region
            << " establishment_id=" << s.establishment_id << ";";
    }
    return out.str();
}
void checkpoint(int id, const std::string& fixture, const std::string& op, const polyec::Realm& w) {
    std::cout << "CHECKPOINT " << id << " " << fixture << " " << op << "\n";
    std::cout << "SERIALIZED " << w.serialize("binding-check") << "\n";
    std::cout << "SESSIONS " << sessionsCanonical(w) << "\n";
}
}

int main() {
    try {
        using polyec::Realm;
        using polyec::RealmClockCommand;
        using polyec::ScheduleCommand;

        // Fixture A: equal-priority overlap yields unresolved attention.
        Realm a(7, 2, 128);
        const uint64_t actor = 1;
        uint64_t seq = a.person(actor).decisions + 1;
        auto a1 = a.execute(ScheduleCommand{actor, seq, ScheduleCommand::Operation::Submit, 0, 0, 64, 0, 600});
        check(a1.success, "A submit 1");
        seq = a.person(actor).decisions + 1;
        auto a2 = a.execute(ScheduleCommand{actor, seq, ScheduleCommand::Operation::Submit, 0, 0, 64, 0, 600});
        check(a2.success, "A submit 2");
        a.advance_ticks(64);
        const auto& aSessions = a.schedule_book().sessions.at(actor);
        check(aSessions.size() == 2, "A session count");
        for (const auto& s : aSessions) {
            check(s.requested == 38400, "A requested");
            check(s.delivered == 0, "A delivered");
            check(s.unresolved_unmet == 38400, "A unresolved");
        }
        checkpoint(1, "A", "overlap_advance", a);

        // Fixture B: partial advance, boundary, save/restore continuation.
        Realm b(7, 2, 128);
        seq = b.person(actor).decisions + 1;
        auto bSession = b.execute(ScheduleCommand{actor, seq, ScheduleCommand::Operation::Submit, 0, 0, 128, 0, 5});
        check(bSession.success, "B submit");
        b.advance_ticks(60);
        check(b.game_tick() == 60, "B tick 60");
        check(b.view().day == 0, "B day 0");
        const auto& bSessions60 = b.schedule_book().sessions.at(actor);
        check(bSessions60.front().requested == 300, "B requested 300");
        check(bSessions60.front().delivered == 300, "B delivered 300");
        checkpoint(2, "B", "advance_60", b);
        b.advance_ticks(5);
        check(b.game_tick() == 65, "B tick 65");
        check(b.view().day == 1, "B day 1");
        const auto& bSessions65 = b.schedule_book().sessions.at(actor);
        check(bSessions65.front().requested == 325, "B requested 325");
        check(bSessions65.front().delivered == 325, "B delivered 325");
        // no debug
        checkpoint(3, "B", "advance_65", b);
        std::string saved = b.serialize("binding-check");
        Realm restored(1, 1, 128);
        std::string error;
        check(restored.deserialize(saved, "binding-check", error), "B restore");
        check(error.empty(), "B restore error");
        check(restored.game_tick() == b.game_tick(), "B restore tick");
        check(restored.serialize("binding-check") == saved, "B restore serialized");
        checkpoint(4, "B", "restored", restored);
        Realm uninterrupted(7, 2, 128);
        uint64_t uninterruptedSeq = uninterrupted.person(actor).decisions + 1;
        auto uninterruptedSubmit = uninterrupted.execute(ScheduleCommand{actor, uninterruptedSeq, ScheduleCommand::Operation::Submit, 0, 0, 128, 0, 5});
        check(uninterruptedSubmit.success, "B uninterrupted submit");
        uninterrupted.advance_ticks(128);
        b.advance_ticks(63);
        restored.advance_ticks(63);
        check(b.game_tick() == 128, "B tick 128");
        check(b.view().day == 2, "B day 2");
        const auto& bSessions128 = b.schedule_book().sessions.at(actor);
        check(bSessions128.front().requested == 640, "B requested 640");
        check(bSessions128.front().delivered == 640, "B delivered 640");
        check(b.serialize("binding-check") == restored.serialize("binding-check"), "B continuation");
        check(b.serialize("binding-check") == uninterrupted.serialize("binding-check"), "B one-shot equivalence");
        checkpoint(5, "B", "continued", b);

        // Fixture C: command validation before any time advances.
        Realm c(7, 2, 128);
        const uint64_t cActor = 1;
        const uint64_t otherActor = 2;
        seq = c.person(cActor).decisions + 1;
        auto cSubmit = c.execute(ScheduleCommand{cActor, seq, ScheduleCommand::Operation::Submit, 0, 0, 64, 0, 5});
        check(cSubmit.success, "C submit");
        uint64_t cSession = cSubmit.session;
        std::string cBefore = c.serialize("binding-check");
        // successful cancel
        seq = c.person(cActor).decisions + 1;
        auto cCancel = c.execute(ScheduleCommand{cActor, seq, ScheduleCommand::Operation::Cancel, cSession, 0, 0, 0, 0});
        check(cCancel.success, "C cancel success");
        check(c.schedule_book().sessions.find(cActor) == c.schedule_book().sessions.end(), "C cancel removed");
        // successful reprioritize
        seq = c.person(cActor).decisions + 1;
        auto cSubmit2 = c.execute(ScheduleCommand{cActor, seq, ScheduleCommand::Operation::Submit, 0, 0, 64, 0, 5});
        check(cSubmit2.success, "C submit 2");
        uint64_t cSession2 = cSubmit2.session;
        seq = c.person(cActor).decisions + 1;
        auto cReprio = c.execute(ScheduleCommand{cActor, seq, ScheduleCommand::Operation::Reprioritize, cSession2, 0, 0, 7, 5});
        check(cReprio.success, "C reprioritize success");
        const auto& cSessions = c.schedule_book().sessions.at(cActor);
        check(cSessions.front().priority == 7, "C priority updated");
        // wrong owner
        std::string beforeWrong = c.serialize("binding-check");
        auto wrongOwner = c.execute(ScheduleCommand{otherActor, c.person(otherActor).decisions + 1, ScheduleCommand::Operation::Cancel, cSession2, 0, 0, 0, 0});
        check(!wrongOwner.success, "C wrong owner rejected");
        check(wrongOwner.reason == "Unknown session", "C wrong owner reason");
        check(c.serialize("binding-check") == beforeWrong, "C wrong owner unchanged");
        // stale sequence
        std::string beforeStale = c.serialize("binding-check");
        auto stale = c.execute(ScheduleCommand{cActor, c.person(cActor).decisions, ScheduleCommand::Operation::Submit, 0, 64, 128, 0, 5});
        check(!stale.success, "C stale rejected");
        check(stale.reason == "Stale or duplicate decision ID", "C stale reason");
        check(c.serialize("binding-check") == beforeStale, "C stale unchanged");
        // duplicate operation value (unsupported)
        std::string beforeBadOp = c.serialize("binding-check");
        auto badOp = c.execute(ScheduleCommand{cActor, c.person(cActor).decisions + 1, static_cast<ScheduleCommand::Operation>(9), cSession2, 64, 128, 0, 5});
        check(!badOp.success, "C bad operation rejected");
        check(badOp.reason == "Unsupported schedule operation", "C bad operation reason");
        check(c.serialize("binding-check") == beforeBadOp, "C bad operation unchanged");
        // invalid range
        std::string beforeRange = c.serialize("binding-check");
        auto badRange = c.execute(ScheduleCommand{cActor, c.person(cActor).decisions + 1, ScheduleCommand::Operation::Submit, 0, 64, 64, 0, 5});
        check(!badRange.success, "C bad range rejected");
        check(badRange.reason == "Invalid session range", "C bad range reason");
        check(c.serialize("binding-check") == beforeRange, "C bad range unchanged");
        // invalid rate
        std::string beforeRate = c.serialize("binding-check");
        auto badRate = c.execute(ScheduleCommand{cActor, c.person(cActor).decisions + 1, ScheduleCommand::Operation::Submit, 0, 64, 128, 0, 0});
        check(!badRate.success, "C bad rate rejected");
        check(badRate.reason == "Invalid demand rate", "C bad rate reason");
        check(c.serialize("binding-check") == beforeRate, "C bad rate unchanged");
        checkpoint(6, "C", "validation", c);

        // Fixture D: clock overflow through serialized fixtures.
        auto replaceClock = [](const std::string& source, int pausedFlag, uint64_t revision) {
            const std::string marker = " CLOCK_1 ";
            std::size_t markerPos = source.find(marker);
            check(markerPos != std::string::npos, "D clock marker");
            std::size_t pausedPos = markerPos + marker.size();
            std::size_t spaceAfterPaused = source.find(' ', pausedPos);
            check(spaceAfterPaused != std::string::npos, "D clock space");
            std::size_t revStart = spaceAfterPaused + 1;
            std::size_t revEnd = source.find(' ', revStart);
            check(revEnd != std::string::npos, "D clock revision end");
            return source.substr(0, pausedPos) + (pausedFlag ? "1" : "0") + " " + std::to_string(revision) + source.substr(revEnd);
        };
        std::string base = Realm(7, 2, 128).serialize("binding-check");
        std::string pausedFixture = replaceClock(base, 1, UINT64_MAX);
        std::string unpausedFixture = replaceClock(base, 0, UINT64_MAX);
        check(pausedFixture != base && unpausedFixture != base, "D fixtures changed");
        Realm pausedMax(1, 1, 128);
        std::string pErr;
        check(pausedMax.deserialize(pausedFixture, "binding-check", pErr), "D paused restore");
        check(pErr.empty(), "D paused restore error");
        check(pausedMax.paused(), "D paused max state");
        check(pausedMax.pause_revision() == UINT64_MAX, "D paused max revision");
        auto pausedPause = pausedMax.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
        check(pausedPause.success, "D paused pause idempotent");
        check(pausedMax.pause_revision() == UINT64_MAX, "D paused pause revision");
        auto pausedResume = pausedMax.execute(RealmClockCommand{RealmClockCommand::Operation::Resume});
        check(!pausedResume.success, "D paused resume rejected");
        check(pausedResume.reason == "Pause revision overflow", "D paused resume reason");
        check(pausedMax.paused(), "D paused resume leaves paused");
        Realm unpausedMax(1, 1, 128);
        std::string uErr;
        check(unpausedMax.deserialize(unpausedFixture, "binding-check", uErr), "D unpaused restore");
        check(uErr.empty(), "D unpaused restore error");
        check(!unpausedMax.paused(), "D unpaused max state");
        check(unpausedMax.pause_revision() == UINT64_MAX, "D unpaused max revision");
        auto unpausedPause = unpausedMax.execute(RealmClockCommand{RealmClockCommand::Operation::Pause});
        check(!unpausedPause.success, "D unpaused pause rejected");
        check(unpausedPause.reason == "Pause revision overflow", "D unpaused pause reason");
        check(!unpausedMax.paused(), "D unpaused pause leaves unpaused");
        auto unpausedResume = unpausedMax.execute(RealmClockCommand{RealmClockCommand::Operation::Resume});
        check(unpausedResume.success, "D unpaused resume idempotent");
        check(unpausedMax.pause_revision() == UINT64_MAX, "D unpaused resume revision");
        checkpoint(7, "D", "overflow", pausedMax);

        std::cout << "REALM CLOCK REPLAY: " << checks << " checks passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << "\n";
        return 1;
    }
}

