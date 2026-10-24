#pragma once
#include "polyec/schedule.hpp"
#include <algorithm>
#include <istream>
#include <limits>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace polyec {

// Realm::serialize writes only the newest section. Older Schedule tags are
// read-only migration formats handled by read_schedule_section.
inline void write_schedule_section(std::ostream& out, const ScheduleBook& book) {
    std::vector<ActivitySession> allSessions;
    for (const auto&[actor, sessions] : book.sessions)
        for (const auto&s : sessions)
            allSessions.push_back(s);
    std::sort(allSessions.begin(), allSessions.end(),
              [](const ActivitySession&a, const ActivitySession&b) { return a.id < b.id; });

    std::vector<std::pair<uint64_t, int64_t>> allAttendance(
        book.work_present_ticks.begin(), book.work_present_ticks.end());
    std::sort(allAttendance.begin(), allAttendance.end(),
              [](const auto&a, const auto&b) { return a.first < b.first; });
    std::vector<std::pair<uint64_t, int64_t>> allSettled(
        book.settled_work_units.begin(), book.settled_work_units.end());
    std::sort(allSettled.begin(), allSettled.end(),
              [](const auto&a, const auto&b) { return a.first < b.first; });
    std::vector<std::pair<WorkRegionKey, int64_t>> allRegionalTicks;
    for (const auto&[key, ticks] : book.work_present_ticks_by_region)
        if (ticks > 0) allRegionalTicks.push_back({key, ticks});
    std::sort(allRegionalTicks.begin(), allRegionalTicks.end(),
              [](const auto&a, const auto&b) { return a.first < b.first; });
    std::vector<std::pair<WorkRegionKey, int64_t>> allRegionalPaid;
    for (const auto&[key, units] : book.paid_work_units_by_region)
        if (units > 0) allRegionalPaid.push_back({key, units});
    std::sort(allRegionalPaid.begin(), allRegionalPaid.end(),
              [](const auto&a, const auto&b) { return a.first < b.first; });
    std::vector<std::pair<WorkEstablishmentKey, int64_t>> allEstablishmentTicks;
    for (const auto&[key, ticks] : book.work_present_ticks_by_establishment)
        if (ticks > 0) allEstablishmentTicks.push_back({key, ticks});
    std::sort(allEstablishmentTicks.begin(), allEstablishmentTicks.end(),
              [](const auto&a, const auto&b) { return a.first < b.first; });
    std::vector<std::pair<WorkEstablishmentKey, int64_t>> allEffortAttention;
    for (const auto&[key, units] : book.work_effort_attention_by_establishment)
        if (units > 0) allEffortAttention.push_back({key, units});
    std::sort(allEffortAttention.begin(), allEffortAttention.end(),
              [](const auto&a, const auto&b) { return a.first < b.first; });
    std::vector<std::pair<WorkEstablishmentKey, int64_t>> allProductivityAttention;
    for (const auto&[key, units] : book.work_productivity_attention_numerator_by_establishment)
        if (units > 0) allProductivityAttention.push_back({key, units});
    std::sort(allProductivityAttention.begin(), allProductivityAttention.end(),
              [](const auto&a, const auto&b) { return a.first < b.first; });
    out << " SCHEDULE_7 " << (book.timeline_contract ? 1 : 0) << ' '
        << book.cursor << ' ' << book.next_session_id << ' ' << book.settled_day << ' '
        << allSessions.size() << ' ' << allAttendance.size() << ' '
        << allSettled.size() << ' ' << allRegionalTicks.size() << ' '
        << allRegionalPaid.size() << ' ' << allEstablishmentTicks.size() << ' '
        << allEffortAttention.size() << ' ' << allProductivityAttention.size() << ' ';
    for (const auto&session : allSessions)
        out << session.id << ' ' << session.actor << ' ' << session.begin << ' '
            << session.end << ' ' << session.priority << ' ' << session.demand_rate << ' '
            << session.requested << ' ' << session.delivered << ' ' << session.unmet << ' '
            << session.capacity_unmet << ' ' << session.unresolved_unmet << ' '
            << session.retired << ' ' << session.retired_tick << ' '
            << session.retired_cause << ' ' << int(session.kind) << ' ' << session.region << ' '
            << session.establishment_id << ' ';
    for (const auto&entry : allAttendance) out << entry.first << ' ' << entry.second << ' ';
    for (const auto&entry : allSettled) out << entry.first << ' ' << entry.second << ' ';
    for (const auto&entry : allRegionalTicks)
        out << entry.first.first << ' ' << entry.first.second << ' ' << entry.second << ' ';
    for (const auto&entry : allRegionalPaid)
        out << entry.first.first << ' ' << entry.first.second << ' ' << entry.second << ' ';
    for (const auto&entry : allEstablishmentTicks)
        out << entry.first.first << ' ' << entry.first.second << ' ' << entry.second << ' ';
    for (const auto&entry : allEffortAttention)
        out << entry.first.first << ' ' << entry.first.second << ' ' << entry.second << ' ';
    for (const auto&entry : allProductivityAttention)
        out << entry.first.first << ' ' << entry.first.second << ' ' << entry.second << ' ';
}

// Read a complete SCHEDULE_1/2/3/4/5/6/7 section from the input stream.
// The reader stages into a local ScheduleBook and only copies/moves it to out
// after the entire section succeeds. Preserves schema-specific defaults and
// validates existing section rules. Returns true on success, false on failure
// with error string (out unchanged on failure).
// Caller (Realm::deserialize) must be positioned after CLOCK_1 tag.
// Parameters:
//   in           - input stream (positioned after CLOCK_1)
//   realm_schema - schema version (7=SCHEDULE_1, 8=SCHEDULE_2, 9=SCHEDULE_3, 10-11=SCHEDULE_4, 12-28=SCHEDULE_5, 29-33=SCHEDULE_6, 34+=SCHEDULE_7)
//   people_count - number of people in the realm (for validation bounds)
//   regions        - number of regions (for validation bounds)
//   realm_day    - current realm day (for validation)
//   out          - ScheduleBook output, updated only on success
//   error        - error output on failure
inline bool read_schedule_section(std::istream& in, int realm_schema,
                          size_t people_count, int regions, int realm_day,
                          ScheduleBook& out, std::string& error, int64_t ticks_per_unit=16) {
    if (realm_schema < 7 || realm_schema > 67 || ticks_per_unit<1 || ticks_per_unit>64) {
        error = "Save schedule schema or tick rate";
        return false;
    }

    // Read section tag and header counts.
    std::string scheduleTag;
    int timelineFlag = 0;
    int64_t cursor = 0;
    uint64_t nextSession = 1;
    int64_t settledDay = 0;
    size_t sessionCount = 0, attendanceCount = 0, settledCount = 0;
    size_t regionalTickCount = 0, regionalPaidCount = 0, establishmentTickCount = 0, effortAttentionCount = 0,
           productivityAttentionCount = 0;

    // Determine expected tag based on schema.
    const std::string expectedTag =
        realm_schema == 7 ? "SCHEDULE_1"
        : realm_schema == 8 ? "SCHEDULE_2"
        : realm_schema == 9 ? "SCHEDULE_3"
        : realm_schema >= 34 ? "SCHEDULE_7"
        : realm_schema >= 29 ? "SCHEDULE_6"
        : realm_schema >= 12 ? "SCHEDULE_5" : "SCHEDULE_4";

    in >> scheduleTag >> timelineFlag >> cursor >> nextSession;
    if (!in || scheduleTag != expectedTag ||
        (timelineFlag != 0 && timelineFlag != 1) ||
        cursor < 0 || nextSession < 1) {
        error = "Save schedule header (expected " + expectedTag + ", got " + scheduleTag +
            ", timeline=" + std::to_string(timelineFlag) + ", cursor=" + std::to_string(cursor) +
            ", next=" + std::to_string(nextSession) + ")";
        return false;
    }

    // Read schema-specific counts.
    if (realm_schema >= 10) {
        in >> settledDay >> sessionCount >> attendanceCount >> settledCount
           >> regionalTickCount >> regionalPaidCount;
        if (realm_schema >= 12) in >> establishmentTickCount;
        if (realm_schema >= 29) in >> effortAttentionCount;
        if (realm_schema >= 34) in >> productivityAttentionCount;
        if (!in) {
            error = "Save schedule counts";
            return false;
        }
    } else {
        if (realm_schema == 9) in >> settledDay;
        in >> sessionCount;
        if (realm_schema >= 8) in >> attendanceCount;
        if (realm_schema == 9) in >> settledCount;
        if (realm_schema == 9 && !in) {
            error = "Save schedule header";
            return false;
        }
        if (!in) {
            error = "Save schedule header";
            return false;
        }
    }

    // Basic count limits.
    if (sessionCount > 1000000) {
        error = "Save schedule count limit";
        return false;
    }
    if (attendanceCount > people_count) {
        error = "Save schedule attendance count limit";
        return false;
    }
    if (settledCount > people_count) {
        error = "Save schedule settled count limit";
        return false;
    }

    // Region key bound check: people * (regions + 1) must not overflow.
    size_t regionBound = 0;
    if (regions < 0) {
        error = "Save schedule region bound";
        return false;
    }
    const size_t slots = size_t(regions) + 1;
    if (people_count > std::numeric_limits<size_t>::max() / slots) {
        error = "Save schedule region bound";
        return false;
    }
    regionBound = people_count * slots;
    if (regionalTickCount > regionBound || regionalPaidCount > regionBound) {
        error = "Save schedule regional count limit";
        return false;
    }
    if (establishmentTickCount > 1000000) {
        error = "Save schedule establishment count limit";
        return false;
    }
    if (effortAttentionCount > 1000000) {
        error = "Save schedule effort attention count limit";
        return false;
    }
    if (productivityAttentionCount > 1000000) {
        error = "Save schedule productivity attention count limit";
        return false;
    }

    // Timeline flag consistency: off means no attendance/regional/settlement.
    if (timelineFlag == 0 &&
        (attendanceCount > 0 || regionalTickCount > 0 || regionalPaidCount > 0 ||
         establishmentTickCount > 0 || effortAttentionCount > 0 || productivityAttentionCount > 0)) {
        error = "Save schedule timeline flag";
        return false;
    }
    if (timelineFlag == 0 && settledDay != 0) {
        error = "Save schedule settled day";
        return false;
    }

    // Cursor bounds.
    const int64_t maxTick = 365000LL * 64;
    if (cursor > maxTick) {
        error = "Save schedule cursor";
        return false;
    }
    if (settledDay < 0 || settledDay > cursor / 64) {
        error = "Save schedule settled day";
        return false;
    }
    if (realm_day != static_cast<int>(cursor / 64)) {
        error = "Save schedule day mismatch";
        return false;
    }

    // Read all sessions.
    std::vector<ActivitySession> loaded;
    loaded.reserve(sessionCount);
    std::set<uint64_t> seenIds;
    for (size_t i = 0; i < sessionCount; ++i) {
        ActivitySession sess{};
        int retired = 0;
        in >> sess.id >> sess.actor >> sess.begin >> sess.end >>
            sess.priority >> sess.demand_rate >> sess.requested >>
            sess.delivered >> sess.unmet >> sess.capacity_unmet >>
            sess.unresolved_unmet >> retired >> sess.retired_tick >>
            sess.retired_cause;
        if (!in) {
            error = "Save schedule fields";
            return false;
        }
        // Schema 7 (SCHEDULE_1) is Generic/-1 only; schemas 8-12 have kind+region.
        if (realm_schema <= 7) {
            sess.kind = ActivityKind::Generic;
            sess.region = -1;
        } else {
            int kind = 0;
            in >> kind >> sess.region;
            if (!in) {
                error = "Save schedule fields";
                return false;
            }
            if (kind < 0 || kind > 1) {
                error = "Save schedule kind";
                return false;
            }
            sess.kind = static_cast<ActivityKind>(kind);
            if (realm_schema >= 12) in >> sess.establishment_id;
            if (!in) {
                error = "Save schedule fields";
                return false;
            }
            if (sess.kind == ActivityKind::Work) {
                if (sess.region < 0 || sess.region > regions) {
                    error = "Save schedule work region";
                    return false;
                }
            } else if (sess.region != -1) {
                error = "Save schedule generic region";
                return false;
            } else if (sess.establishment_id != 0) {
                error = "Save schedule generic establishment";
                return false;
            }
        }
        if (sess.id == 0 || seenIds.count(sess.id)) {
            error = "Save schedule id";
            return false;
        }
        seenIds.insert(sess.id);
        if (sess.actor == 0 || static_cast<size_t>(sess.actor) > people_count) {
            error = "Save schedule actor";
            return false;
        }
        if (sess.begin < 0 || sess.begin >= sess.end || sess.end > maxTick) {
            error = "Save schedule range";
            return false;
        }
        if (sess.demand_rate < activity::kMinDemandRate ||
            sess.demand_rate > activity::kMaxDemandRate) {
            error = "Save schedule rate";
            return false;
        }
        if (sess.requested < 0 || sess.delivered < 0 ||
            sess.unmet < 0 || sess.capacity_unmet < 0 ||
            sess.unresolved_unmet < 0) {
            error = "Save schedule totals";
            return false;
        }
        if (sess.delivered > sess.requested) {
            error = "Save schedule delivered";
            return false;
        }
        if (sess.unmet != sess.requested - sess.delivered) {
            error = "Save schedule total mismatch";
            return false;
        }
        if (sess.capacity_unmet > sess.unmet) {
            error = "Save schedule totals";
            return false;
        }
        if (sess.unresolved_unmet != sess.unmet - sess.capacity_unmet) {
            error = "Save schedule reason mismatch";
            return false;
        }
        if (retired != 0 && retired != 1) {
            error = "Save schedule retired";
            return false;
        }
        sess.retired = (retired == 1);
        if (sess.retired_tick < 0 || sess.retired_tick > cursor) {
            error = "Save schedule retired tick";
            return false;
        }
        if (sess.retired) {
            if (sess.retired_cause == 0) {
                error = "Save schedule retired cause";
                return false;
            }
        } else if (sess.retired_tick != 0 || sess.retired_cause != 0) {
            error = "Save schedule retired";
            return false;
        }
        // Processed-end check.
        int64_t processedEnd = sess.retired ? sess.retired_tick : cursor;
        if (processedEnd > sess.end) processedEnd = sess.end;
        if (processedEnd < sess.begin) processedEnd = sess.begin;
        if (sess.requested != sess.demand_rate * (processedEnd - sess.begin)) {
            error = "Save schedule processed mismatch";
            return false;
        }
        if (sess.delivered > sess.requested) {
            error = "Save schedule delivered";
            return false;
        }
        loaded.push_back(sess);
    }

    // Validate next_session_id against max session ID.
    if (!seenIds.empty()) {
        uint64_t maxId = *std::max_element(seenIds.begin(), seenIds.end());
        if (nextSession <= maxId) {
            error = "Save schedule next id";
            return false;
        }
    }

    // Read attendance (unique actor->ticks map).
    std::unordered_map<uint64_t, int64_t> loadedAttendance;
    for (size_t i = 0; i < attendanceCount; ++i) {
        uint64_t actor = 0;
        int64_t ticks = 0;
        in >> actor >> ticks;
        if (!in) {
            error = "Save schedule attendance fields";
            return false;
        }
        if (actor == 0 || actor > static_cast<uint64_t>(people_count)) {
            error = "Save schedule attendance actor";
            return false;
        }
        if (ticks < 0 || ticks > static_cast<int64_t>(cursor)) {
            error = "Save schedule attendance ticks";
            return false;
        }
        if (!loadedAttendance.emplace(actor, ticks).second) {
            error = "Save schedule attendance actor";
            return false;
        }
    }
    // Normalize away zero-tick entries.
    for (auto it = loadedAttendance.begin(); it != loadedAttendance.end();)
        if (it->second == 0) it = loadedAttendance.erase(it);
        else ++it;

    // Read settled work units (unique actor->units map).
    std::unordered_map<uint64_t, int64_t> loadedSettlement;
    for (size_t i = 0; i < settledCount; ++i) {
        uint64_t actor = 0;
        int64_t units = 0;
        in >> actor >> units;
        if (!in) {
            error = "Save schedule settled fields";
            return false;
        }
        if (units < 0) {
            error = "Save schedule settled units";
            return false;
        }
        if (actor == 0 || actor > static_cast<uint64_t>(people_count)) {
            error = "Save schedule settled actor";
            return false;
        }
        if (!loadedSettlement.emplace(actor, units).second) {
            error = "Save schedule settled actor";
            return false;
        }
    }
    // Normalize away zero-unit entries.
    for (auto it = loadedSettlement.begin(); it != loadedSettlement.end();)
        if (it->second == 0) it = loadedSettlement.erase(it);
        else ++it;

    // Read regional ticks (actor, region, ticks).
    std::map<WorkRegionKey, int64_t> loadedRegionalTicks;
    for (size_t i = 0; i < regionalTickCount; ++i) {
        uint64_t actor = 0;
        int region = -1;
        int64_t ticks = 0;
        in >> actor >> region >> ticks;
        if (!in) {
            error = "Save schedule regional attendance fields";
            return false;
        }
        if (actor == 0 || actor > static_cast<uint64_t>(people_count)) {
            error = "Save schedule regional attendance actor";
            return false;
        }
        if (region < 0 || region > regions) {
            error = "Save schedule regional attendance region";
            return false;
        }
        if (ticks <= 0 || ticks > static_cast<int64_t>(cursor)) {
            error = "Save schedule regional attendance ticks";
            return false;
        }
        if (!loadedRegionalTicks.emplace(WorkRegionKey{actor, region}, ticks).second) {
            error = "Save schedule regional attendance key";
            return false;
        }
    }

    // Read regional paid units (actor, region, units).
    std::map<WorkRegionKey, int64_t> loadedRegionalPaid;
    for (size_t i = 0; i < regionalPaidCount; ++i) {
        uint64_t actor = 0;
        int region = -1;
        int64_t units = 0;
        in >> actor >> region >> units;
        if (!in) {
            error = "Save schedule regional paid fields";
            return false;
        }
        if (actor == 0 || actor > static_cast<uint64_t>(people_count)) {
            error = "Save schedule regional paid actor";
            return false;
        }
        if (region < 0 || region > regions) {
            error = "Save schedule regional paid region";
            return false;
        }
        if (units <= 0) {
            error = "Save schedule regional paid units";
            return false;
        }
        auto ticksIt = loadedRegionalTicks.find(WorkRegionKey{actor, region});
        if (ticksIt == loadedRegionalTicks.end()) {
            error = "Save schedule regional paid attendance";
            return false;
        }
        if (units > ticksIt->second / ticks_per_unit) {
            error = "Save schedule regional paid attendance";
            return false;
        }
        if (!loadedRegionalPaid.emplace(WorkRegionKey{actor, region}, units).second) {
            error = "Save schedule regional paid key";
            return false;
        }
    }

    // Read site-qualified attendance (actor, establishment, ticks), schema 12 only.
    std::map<WorkEstablishmentKey, int64_t> loadedEstablishmentTicks;
    for (size_t i = 0; i < establishmentTickCount; ++i) {
        uint64_t actor = 0, establishment = 0;
        int64_t ticks = 0;
        in >> actor >> establishment >> ticks;
        if (!in) {
            error = "Save schedule establishment attendance fields";
            return false;
        }
        if (actor == 0 || actor > static_cast<uint64_t>(people_count)) {
            error = "Save schedule establishment attendance actor";
            return false;
        }
        if (establishment == 0) {
            error = "Save schedule establishment attendance ID";
            return false;
        }
        if (ticks <= 0 || ticks > static_cast<int64_t>(cursor)) {
            error = "Save schedule establishment attendance ticks";
            return false;
        }
        if (!loadedEstablishmentTicks.emplace(WorkEstablishmentKey{actor, establishment}, ticks).second) {
            error = "Save schedule establishment attendance key";
            return false;
        }
    }

    // Work attention credited while physically present (actor, establishment, units).
    std::map<WorkEstablishmentKey, int64_t> loadedEffortAttention;
    for (size_t i = 0; i < effortAttentionCount; ++i) {
        uint64_t actor = 0, establishment = 0;
        int64_t units = 0;
        in >> actor >> establishment >> units;
        if (!in || actor == 0 || actor > static_cast<uint64_t>(people_count) ||
            establishment == 0 || units <= 0) {
            error = "Save schedule effort attention fields";
            return false;
        }
        const WorkEstablishmentKey key{actor, establishment};
        const auto attended = loadedEstablishmentTicks.find(key);
        if (attended == loadedEstablishmentTicks.end() ||
            attended->second > std::numeric_limits<int64_t>::max() / kAttentionUnitsPerEffortTick ||
            units > attended->second * kAttentionUnitsPerEffortTick) {
            error = "Save schedule effort attention exceeds physical attendance";
            return false;
        }
        if (!loadedEffortAttention.emplace(key, units).second) {
            error = "Save schedule effort attention key";
            return false;
        }
    }

    // Skill-weighted production attention numerator (actor, establishment,
    // attention units multiplied by an integer factor with denominator 200).
    std::map<WorkEstablishmentKey, int64_t> loadedProductivityAttention;
    for (size_t i = 0; i < productivityAttentionCount; ++i) {
        uint64_t actor = 0, establishment = 0;
        int64_t numerator = 0;
        in >> actor >> establishment >> numerator;
        if (!in || actor == 0 || actor > static_cast<uint64_t>(people_count) ||
            establishment == 0 || numerator <= 0) {
            error = "Save schedule productivity attention fields";
            return false;
        }
        const WorkEstablishmentKey key{actor, establishment};
        const auto raw = loadedEffortAttention.find(key);
        if (raw == loadedEffortAttention.end() || raw->second <= 0 ||
            raw->second > std::numeric_limits<int64_t>::max() / 250 ||
            numerator < raw->second * 150 || numerator > raw->second * 250) {
            error = "Save schedule productivity attention range";
            return false;
        }
        if (!loadedProductivityAttention.emplace(key, numerator).second) {
            error = "Save schedule productivity attention key";
            return false;
        }
    }
    if (realm_schema >= 34) {
        if (loadedProductivityAttention.size() != loadedEffortAttention.size()) {
            error = "Save schedule productivity attention coverage";
            return false;
        }
        for (const auto& [key, raw] : loadedEffortAttention) {
            (void)raw;
            if (loadedProductivityAttention.find(key) == loadedProductivityAttention.end()) {
                error = "Save schedule productivity attention missing";
                return false;
            }
        }
    }

    // Regional attendance total consistency: per-actor sum of regional ticks
    // must not exceed global attendance.
    std::unordered_map<uint64_t, int64_t> regionalSums;
    for (const auto&[key, ticks] : loadedRegionalTicks) {
        int64_t& sum = regionalSums[key.first];
        if (ticks > std::numeric_limits<int64_t>::max() - sum) {
            error = "Save schedule regional attendance total";
            return false;
        }
        sum += ticks;
    }
    for (const auto&[actor, sum] : regionalSums) {
        auto global = loadedAttendance.find(actor);
        const int64_t total = global == loadedAttendance.end() ? 0 : global->second;
        if (sum > total) {
            error = "Save schedule regional attendance total";
            return false;
        }
    }

    // Site-qualified attendance totals cannot exceed the actor's global union.
    std::unordered_map<uint64_t, int64_t> establishmentSums;
    for (const auto&[key, ticks] : loadedEstablishmentTicks) {
        int64_t& sum = establishmentSums[key.first];
        if (ticks > std::numeric_limits<int64_t>::max() - sum) {
            error = "Save schedule establishment attendance total";
            return false;
        }
        sum += ticks;
    }
    for (const auto&[actor, sum] : establishmentSums) {
        auto global = loadedAttendance.find(actor);
        const int64_t total = global == loadedAttendance.end() ? 0 : global->second;
        if (sum > total) {
            error = "Save schedule establishment attendance total";
            return false;
        }
    }

    // Timeline flag zero checks: no live data allowed.
    if (timelineFlag == 0 &&
        (!loaded.empty() || !loadedAttendance.empty() ||
         !loadedSettlement.empty() || !loadedRegionalTicks.empty() ||
         !loadedRegionalPaid.empty() || !loadedEstablishmentTicks.empty() ||
         !loadedEffortAttention.empty() || !loadedProductivityAttention.empty())) {
        error = "Save schedule timeline flag";
        return false;
    }
    if (timelineFlag == 0 && settledDay != 0) {
        error = "Save schedule settled day";
        return false;
    }
    if (!loadedSettlement.empty() && settledDay <= 0) {
        error = "Save schedule settlement day";
        return false;
    }
    for (const auto&[actor, units] : loadedSettlement) {
        if (actor == 0 || actor > static_cast<uint64_t>(people_count)) {
            error = "Save schedule settled actor";
            return false;
        }
        if (units > cursor / ticks_per_unit) {
            error = "Save schedule settled units";
            return false;
        }
        auto attended = loadedAttendance.find(actor);
        const int64_t ticks = attended == loadedAttendance.end() ? 0 : attended->second;
        if (units > ticks / ticks_per_unit) {
            error = "Save schedule settled attendance";
            return false;
        }
    }

    // Success: apply loaded data to the output ScheduleBook.
    ScheduleBook staged;
    staged.cursor = cursor;
    staged.timeline_contract = timelineFlag == 1;
    staged.next_session_id = nextSession;
    for (const auto&s : loaded) staged.sessions[s.actor].push_back(s);
    for (auto&[actor, sessions] : staged.sessions)
        std::sort(sessions.begin(), sessions.end(),
                  [](const ActivitySession&a, const ActivitySession&b) { return a.id < b.id; });
    staged.work_present_ticks = std::move(loadedAttendance);
    staged.settled_day = settledDay;
    staged.settled_work_units = std::move(loadedSettlement);
    staged.work_present_ticks_by_region = std::move(loadedRegionalTicks);
    staged.paid_work_units_by_region = std::move(loadedRegionalPaid);
    staged.work_present_ticks_by_establishment = std::move(loadedEstablishmentTicks);
    staged.work_effort_attention_by_establishment = std::move(loadedEffortAttention);
    staged.work_productivity_attention_numerator_by_establishment = std::move(loadedProductivityAttention);

    out = std::move(staged);
    error.clear();
    return true;
}

} // namespace polyec

