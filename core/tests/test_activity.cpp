// Focused runtime tests for the pure stateless activity evaluator
// (polyec::activity::evaluateActivity). Pattern per test_managers.cpp: plain
// main(), check() counting failures to stderr, hardcoded expected integers.
// No allocator duplication: expected values are hand-computed constants, never
// recomputed by a second implementation of the rule set in this file.
// Runtime checks are active in Release (no assert(), no NDEBUG-only checks).
#include "polyec/activity.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <vector>

using polyec::activity::EvaluateResult;
using polyec::activity::Horizon;
using polyec::activity::RejectReason;
using polyec::activity::SessionResult;
using polyec::activity::SessionView;
using polyec::activity::evaluateActivity;

namespace {

constexpr uint64_t kActor = 7u;

int failures = 0;

void check(bool cond, const std::string &what) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", what.c_str());
        ++failures;
    }
}

SessionView mk(uint64_t id, int64_t begin, int64_t end, int32_t priority,
               int32_t demand_rate) {
    SessionView s;
    s.id = id;
    s.actor = kActor;
    s.begin = begin;
    s.end = end;
    s.priority = priority;
    s.demand_rate = demand_rate;
    return s;
}

// One row expected at position `slot` in the (id-sorted) results, with exact
// hardcoded counters. Also verifies delivered + unmet == requested and
// unmet == capacity_unmet + unresolved_unmet on that row.
void row(const EvaluateResult &r, std::size_t slot, uint64_t id, int64_t req,
         int64_t del, int64_t cap, int64_t unr) {
    check(r.ok, "evaluation ok");
    if (!r.ok) return;
    const std::string w = " row " + std::to_string(slot) + " (id " +
                          std::to_string(static_cast<unsigned long long>(id)) + ")";
    if (slot >= r.results.size()) {
        check(false, "missing" + w);
        return;
    }
    const SessionResult &s = r.results[slot];
    check(s.id == id, "id" + w);
    check(s.requested == req, "requested" + w);
    check(s.delivered == del, "delivered" + w);
    check(s.capacity_unmet == cap, "capacity_unmet" + w);
    check(s.unresolved_unmet == unr, "unresolved_unmet" + w);
    check(s.unmet == cap + unr, "unmet == cap+unr" + w);
    check(s.delivered + s.unmet == s.requested, "delivered+unmet==requested" + w);
}

void rejectCheck(const EvaluateResult &r, RejectReason reason, const char *what) {
    check(!r.ok, std::string(what) + " must reject");
    check(r.reason == reason, std::string(what) + " reason");
    check(r.results.empty(), std::string(what) + " empty output");
}

bool same(const SessionView &a, const SessionView &b) {
    return a.id == b.id && a.actor == b.actor && a.begin == b.begin &&
           a.end == b.end && a.priority == b.priority &&
           a.demand_rate == b.demand_rate;
}

// Shared scenario: two segments inside a 10-tick window. Used as-is by the
// mixed-reasons, split-additivity and translation tests.
std::vector<SessionView> mixedSet() {
    // Segment [0,5): X(800, pri 6) full; Y(500, pri 5) singleton overflow with
    // remaining 200 -> Y delivered 200*5, shortfall 300*5.
    // Segment [5,10): U,V,W (400 each, pri 4) tied overflow -> withheld; Z
    // (100, pri 2) blocked -> unresolved too.
    return {
        mk(1, 0, 5, 6, 800),  // X
        mk(2, 0, 5, 5, 500),  // Y
        mk(3, 5, 10, 4, 400)  // U
        ,
        mk(4, 5, 10, 4, 400),  // V
        mk(5, 5, 10, 4, 400),  // W
        mk(6, 5, 10, 2, 100),  // Z
    };
}

std::vector<SessionResult> mixedExpected() {
    // X: requested 800*5 = 4000, delivered 4000.
    // Y: requested 500*5 = 2500, delivered 200*5 = 1000, capacity_unmet 300*5 = 1500.
    // U/V/W: requested 400*5 = 2000, unresolved_unmet 2000.
    // Z: requested 100*5 = 500, unresolved_unmet 500.
    SessionResult x;
    x.id = 1; x.requested = 4000; x.delivered = 4000; x.unmet = 0;
    x.capacity_unmet = 0; x.unresolved_unmet = 0;
    SessionResult y;
    y.id = 2; y.requested = 2500; y.delivered = 1000; y.unmet = 1500;
    y.capacity_unmet = 1500; y.unresolved_unmet = 0;
    SessionResult u;
    u.id = 3; u.requested = 2000; u.delivered = 0; u.unmet = 2000;
    u.capacity_unmet = 0; u.unresolved_unmet = 2000;
    SessionResult v;
    v.id = 4; v.requested = 2000; v.delivered = 0; v.unmet = 2000;
    v.capacity_unmet = 0; v.unresolved_unmet = 2000;
    SessionResult wres;
    wres.id = 5; wres.requested = 2000; wres.delivered = 0; wres.unmet = 2000;
    wres.capacity_unmet = 0; wres.unresolved_unmet = 2000;
    SessionResult z;
    z.id = 6; z.requested = 500; z.delivered = 0; z.unmet = 500;
    z.capacity_unmet = 0; z.unresolved_unmet = 500;
    return {x, y, u, v, wres, z};
}

void test_exclusive_full() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(kActor, h, {mk(1, 0, 10, 5, 10)});
    check(r.ok, "exclusive ok");
    check(r.results.size() == 1, "exclusive one row");
    row(r, 0, 1, 100, 100, 0, 0);  // 10 units/tick * 10 ticks
}

void test_compatible_fit() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(
        kActor, h, {mk(1, 0, 10, 5, 600), mk(2, 0, 10, 5, 400)});
    check(r.ok && r.results.size() == 2, "compatible ok");
    row(r, 0, 1, 6000, 6000, 0, 0);
    row(r, 1, 2, 4000, 4000, 0, 0);  // 600+400 == capacity 1000, both full
}

void test_singleton_partial() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(
        kActor, h, {mk(1, 0, 10, 5, 600), mk(2, 0, 10, 4, 600)});
    check(r.ok && r.results.size() == 2, "singleton partial ok");
    row(r, 0, 1, 6000, 6000, 0, 0);
    // remaining rate 400 -> 4000 units delivered; 2000 units shortfall.
    row(r, 1, 2, 6000, 4000, 2000, 0);
}

void test_overcommit_valid() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(
        kActor, h,
        {mk(1, 0, 10, 5, 500), mk(2, 0, 10, 4, 500), mk(3, 0, 10, 3, 500)});
    check(r.ok, "overcommit is valid (not a structural reject)");
    check(r.results.size() == 3, "overcommit three rows");
    row(r, 0, 1, 5000, 5000, 0, 0);
    row(r, 1, 2, 5000, 5000, 0, 0);
    row(r, 2, 3, 5000, 0, 5000, 0);  // remaining rate 0 -> capacity shortfall
}

void test_ties_withhold_lower_id() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(
        kActor, h,
        {mk(1, 0, 10, 3, 600), mk(2, 0, 10, 3, 600), mk(3, 0, 10, 1, 100)});
    check(r.ok && r.results.size() == 3, "ties ok");
    // Tied overflow with remaining 1000 > 0: withheld; all tied unmet as
    // UnresolvedPriority; lower priority (id3) also unresolved, nothing
    // delivered anywhere and no ID winner between id1/id2.
    row(r, 0, 1, 6000, 0, 0, 6000);
    row(r, 1, 2, 6000, 0, 0, 6000);
    row(r, 2, 3, 1000, 0, 0, 1000);
}

void test_remaining_zero_ties_get_capacity() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(kActor, h,
                                    {mk(1, 0, 10, 5, 600), mk(2, 0, 10, 5, 400),
                                     mk(3, 0, 10, 2, 100), mk(4, 0, 10, 2, 100)});
    check(r.ok && r.results.size() == 4, "rem0 ties ok");
    row(r, 0, 1, 6000, 6000, 0, 0);
    row(r, 1, 2, 4000, 4000, 0, 0);  // 600+400 == 1000: full, remaining -> 0
    // remaining 0 from delivered higher => CapacityShortfall even though tied.
    row(r, 2, 3, 1000, 0, 1000, 0);
    row(r, 3, 4, 1000, 0, 1000, 0);
}

void test_mixed_reasons() {
    const Horizon h{0, 10};
    const auto full = evaluateActivity(kActor, h, mixedSet());
    const auto expected = mixedExpected();
    check(full.ok, "mixed ok");
    // Verified above per row; assert against expected table here.
    check(full.results.size() == expected.size(), "mixed row count");
    for (std::size_t i = 0; i < full.results.size() && i < expected.size(); ++i) {
        check(full.results[i].id == expected[i].id, "mixed id order");
        check(full.results[i].requested == expected[i].requested, "mixed requested");
        check(full.results[i].delivered == expected[i].delivered, "mixed delivered");
        check(full.results[i].unmet == expected[i].unmet, "mixed unmet");
        check(full.results[i].capacity_unmet == expected[i].capacity_unmet,
              "mixed capacity_unmet");
        check(full.results[i].unresolved_unmet == expected[i].unresolved_unmet,
              "mixed unresolved_unmet");
    }
}

void test_clipping_and_nonoverlap() {
    const Horizon h{0, 10};
    // id1 straddles the right edge -> clipped to [5,10); id2 lies fully past the
    // window -> zero totals but present; id3 nonoverlapping with id1.
    const auto r = evaluateActivity(
        kActor, h, {mk(1, 5, 15, 1, 100), mk(2, 10, 20, 1, 100), mk(3, 0, 2, 3, 900)});
    check(r.ok && r.results.size() == 3, "clip ok");
    row(r, 0, 1, 500, 500, 0, 0);     // 100*5 clipped ticks
    row(r, 1, 2, 0, 0, 0, 0);         // fully clipped out: zero totals, kept
    row(r, 2, 3, 1800, 1800, 0, 0);   // 900*2, fresh segment
}

void test_empty_sessions() {
    const Horizon h{0, 5};
    const auto r = evaluateActivity(kActor, h, {});
    check(r.ok, "empty sessions ok for valid actor+horizon");
    check(r.reason == RejectReason::None, "empty sessions reason none");
    check(r.results.empty(), "empty sessions -> empty results");
}

// Field-wise comparator for result rows (test-local; the API type carries no
// operator== and none is added for tests).
bool rowEqual(const SessionResult &a, const SessionResult &b) {
    return a.id == b.id && a.requested == b.requested && a.delivered == b.delivered &&
           a.unmet == b.unmet && a.capacity_unmet == b.capacity_unmet &&
           a.unresolved_unmet == b.unresolved_unmet;
}

bool resultsEqual(const EvaluateResult &a, const EvaluateResult &b, const char *what) {
    check(a.ok == b.ok, what);
    bool equal = a.ok == b.ok && a.results.size() == b.results.size();
    check(equal, std::string(what) + " (size/flags)");
    if (!equal) return false;
    for (std::size_t i = 0; i < a.results.size(); ++i) {
        check(rowEqual(a.results[i], b.results[i]),
              std::string(what) + " row " + std::to_string(i));
        if (!rowEqual(a.results[i], b.results[i])) equal = false;
    }
    return equal;
}

void test_reorder_invariance() {
    const Horizon h{0, 10};
    const auto a = evaluateActivity(kActor, h, mixedSet());
    // ONE local vector, then reverse in place (never two temporaries).
    std::vector<SessionView> reversed = mixedSet();
    std::reverse(reversed.begin(), reversed.end());
    const auto b = evaluateActivity(kActor, h, reversed);
    check(a.ok && b.ok, "reorder ok");
    resultsEqual(a, b, "reorder invariance (results identical)");
}

void test_horizon_split_additivity() {
    const Horizon full{0, 10};
    const Horizon left{0, 4};     // 4 is NOT a session endpoint of mixedSet
    const Horizon right{4, 10};
    const auto whole = evaluateActivity(kActor, full, mixedSet());
    const auto l = evaluateActivity(kActor, left, mixedSet());
    const auto r = evaluateActivity(kActor, right, mixedSet());
    check(whole.ok && l.ok && r.ok, "split runs ok");
    check(whole.results.size() == mixedSet().size(), "split whole rows");
    // Size gates must FAIL loudly, never silently skip the additive assertions.
    check(l.results.size() == whole.results.size(), "split left rows");
    check(r.results.size() == whole.results.size(), "split right rows");
    // Per-session additivity of every counter across [0,4) + [4,10).
    // l: X 3200/3200; Y 2000/800 cap1200; U,V,W,Z clipped out (0 rows totals).
    // r: X 800/800; Y 500/200 cap300; U/V/W 2000 unr; Z 500 unr.
    if (l.results.size() == whole.results.size() &&
        r.results.size() == whole.results.size()) {
        for (std::size_t i = 0; i < whole.results.size(); ++i) {
            const SessionResult &o = whole.results[i];
            const SessionResult &ls = l.results[i];
            const SessionResult &rs = r.results[i];
            check(ls.id == o.id && rs.id == o.id, "split ids align");
            check(ls.requested + rs.requested == o.requested, "split requested add");
            check(ls.delivered + rs.delivered == o.delivered, "split delivered add");
            check(ls.capacity_unmet + rs.capacity_unmet == o.capacity_unmet,
                  "split capacity add");
            check(ls.unresolved_unmet + rs.unresolved_unmet == o.unresolved_unmet,
                  "split unresolved add");
            check(ls.unmet + rs.unmet == o.unmet, "split unmet add");
        }
    }
}

void test_translation() {
    const Horizon full{0, 10};
    const Horizon shifted{7, 17};
    std::vector<SessionView> base = mixedSet();
    std::vector<SessionView> moved;
    moved.reserve(base.size());
    for (const SessionView &s : base) {
        SessionView m = s;
        m.begin += 7;
        m.end += 7;
        moved.push_back(m);
    }
    const auto a = evaluateActivity(kActor, full, base);
    const auto b = evaluateActivity(kActor, shifted, moved);
    check(a.ok && b.ok, "translation ok");
    resultsEqual(a, b, "translation invariance");
}

void test_rate_bounds_and_negative_priority() {
    const Horizon h{0, 10};
    const auto hi = evaluateActivity(kActor, h, {mk(1, 0, 10, 0, 1000)});
    check(hi.ok && hi.results.size() == 1, "rate max ok");
    row(hi, 0, 1, 10000, 10000, 0, 0);
    const auto lo = evaluateActivity(kActor, h, {mk(1, 0, 10, 0, 1)});
    check(lo.ok && lo.results.size() == 1, "rate min ok");
    row(lo, 0, 1, 10, 10, 0, 0);

    // Negative explicit priorities sort strictly below positive ones.
    const auto neg = evaluateActivity(
        kActor, h, {mk(1, 0, 10, -5, 100), mk(2, 0, 10, 4, 950)});
    check(neg.ok && neg.results.size() == 2, "negative priority ok");
    row(neg, 0, 1, 1000, 500, 500, 0);   // served last: remaining rate was 50
    row(neg, 1, 2, 9500, 9500, 0, 0);
}

// One session accumulating BOTH unmet reason counters across different
// segments against different causes (required mixed-counter scenario):
//   - [0,5): pri-3 exclusive rate-1000 session fully consumes capacity;
//     LOW-spanning session gets capacity_unmet for that segment.
//   - [5,10): two pri-2 tied sessions overflow with remaining 1000 > 0 ->
//     capacity withheld; LOW gets unresolved_unmet for that segment.
void test_mixed_counters_one_session() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(
        kActor, h,
        {mk(4, 0, 10, 0, 100),   // LOW spanning both segments
         mk(1, 0, 5, 3, 1000),   // exclusive higher, exhausts capacity
         mk(2, 5, 10, 2, 700),   // tie member
         mk(3, 5, 10, 2, 700)}); // tie member
    check(r.ok && r.results.size() == 4, "mixed counters ok");
    row(r, 0, 1, 5000, 5000, 0, 0);
    row(r, 1, 2, 3500, 0, 0, 3500);
    row(r, 2, 3, 3500, 0, 0, 3500);
    // LOW: requested 100*10 = 1000; capacity_unmet 100*5 = 500 (segment [0,5),
    // remaining 0 from delivered higher) AND unresolved_unmet 100*5 = 500
    // (segment [5,10 withheld) — both reason counters on one session.
    row(r, 3, 4, 1000, 0, 500, 500);
}

// Ties reaching existing capacity shortfall: a higher singleton was PARTIALLY
// delivered (assigned remaining 400 of its 900), leaving remaining 0; the tied
// pair below gets CapacityShortfall, never UnresolvedPriority.
void test_tie_after_partial_delivery() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(
        kActor, h,
        {mk(1, 0, 10, 5, 600), mk(2, 0, 10, 4, 900), mk(3, 0, 10, 2, 100),
         mk(4, 0, 10, 2, 100)});
    check(r.ok && r.results.size() == 4, "tie-after-partial ok");
    row(r, 0, 1, 6000, 6000, 0, 0);
    row(r, 1, 2, 9000, 4000, 5000, 0);  // singleton partial: 400*10 delivered
    row(r, 2, 3, 1000, 0, 1000, 0);     // remaining 0 -> capacity shortfall
    row(r, 3, 4, 1000, 0, 1000, 0);
}

// Tie overflow after a higher group consumed only PART of the remaining rate
// (still positive): withholding is explicit, lower fits do not get delivered.
void test_tie_after_partial_consumption() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(
        kActor, h,
        {mk(1, 0, 10, 9, 600), mk(2, 0, 10, 5, 300), mk(3, 0, 10, 5, 300),
         mk(4, 0, 10, 1, 50)});
    check(r.ok && r.results.size() == 4, "tie-after-partial-consumption ok");
    row(r, 0, 1, 6000, 6000, 0, 0);      // remaining 1000 -> 400
    row(r, 1, 2, 3000, 0, 0, 3000);      // G=600 > 400, withheld
    row(r, 2, 3, 3000, 0, 0, 3000);
    row(r, 3, 4, 500, 0, 0, 500);        // blocked lower demand also unresolved
}

void test_rejections() {
    const Horizon h{0, 10};
    // duplicate id
    rejectCheck(evaluateActivity(kActor, h, {mk(5, 0, 5, 1, 10), mk(5, 6, 8, 1, 10)}),
                RejectReason::DuplicateId, "duplicate id");
    // mixed actors
    {
        SessionView wrong = mk(1, 0, 5, 1, 10);
        wrong.actor = 8;
        rejectCheck(evaluateActivity(kActor, h, {wrong}), RejectReason::MixedActor,
                    "mixed actor");
        wrong.actor = 0;
        rejectCheck(evaluateActivity(kActor, h, {wrong}), RejectReason::MixedActor,
                    "zero actor session");
    }
    // invalid rates
    rejectCheck(evaluateActivity(kActor, h, {mk(1, 0, 5, 1, 0)}),
                RejectReason::InvalidSession, "rate zero");
    rejectCheck(evaluateActivity(kActor, h, {mk(1, 0, 5, 1, 1001)}),
                RejectReason::InvalidSession, "rate 1001");
    // invalid times
    rejectCheck(evaluateActivity(kActor, h, {mk(1, 5, 5, 1, 10)}),
                RejectReason::InvalidSession, "empty session range");
    rejectCheck(evaluateActivity(kActor, h, {mk(1, 6, 3, 1, 10)}),
                RejectReason::InvalidSession, "reversed session range");
    rejectCheck(evaluateActivity(kActor, h, {mk(1, -1, 3, 1, 10)}),
                RejectReason::InvalidSession, "negative session begin");
    // invalid horizons
    rejectCheck(evaluateActivity(kActor, Horizon{5, 5}, {}),
                RejectReason::InvalidHorizon, "zero-length horizon");
    rejectCheck(evaluateActivity(kActor, Horizon{5, 4}, {}),
                RejectReason::InvalidHorizon, "reversed horizon");
    rejectCheck(evaluateActivity(kActor, Horizon{-1, 5}, {}),
                RejectReason::InvalidHorizon, "negative horizon begin");
    // invalid actor for the call itself
    rejectCheck(evaluateActivity(0u, h, {}),
                RejectReason::InvalidActor, "zero caller actor");
    rejectCheck(evaluateActivity(0u, h, {mk(1, 0, 5, 1, 10)}),
                RejectReason::InvalidActor, "zero caller actor with sessions");
    // checked overflow: rate 1000 over a near-full int64 window overflows
    // requested = rate * length.
    const int64_t big = std::numeric_limits<int64_t>::max();
    rejectCheck(
        evaluateActivity(kActor, Horizon{0, big}, {mk(1, 0, big, 1, 1000)}),
        RejectReason::Overflow, "int64 overflow");
    // No input mutation (const-ref API; compare a deep copy after calls).
    {
        std::vector<SessionView> input = mixedSet();
        std::vector<SessionView> before = input;
        evaluateActivity(kActor, h, input);
        bool same_after_ok = true;
        for (std::size_t i = 0; i < input.size(); ++i)
            same_after_ok = same_after_ok && same(input[i], before[i]);
        check(same_after_ok, "no input mutation after ok call");
        input.push_back(mk(9, 0, 5, 1, 2000));  // invalid rate -> reject path
        std::vector<SessionView> before_reject = input;
        rejectCheck(evaluateActivity(kActor, h, input),
                    RejectReason::InvalidSession, "reject after rate");
        bool same_after_reject = input.size() == before_reject.size();
        for (std::size_t i = 0; i < input.size() && i < before_reject.size(); ++i)
            same_after_reject = same_after_reject && same(input[i], before_reject[i]);
        check(same_after_reject, "no input mutation after reject");
    }
}

void test_id_sorted_output() {
    const Horizon h{0, 10};
    const auto r = evaluateActivity(
        kActor, h, {mk(30, 0, 10, 1, 100), mk(10, 0, 10, 1, 100),
                    mk(20, 0, 10, 1, 100)});
    check(r.ok && r.results.size() == 3, "sorted output ok");
    check(r.results[0].id == 10 && r.results[1].id == 20 && r.results[2].id == 30,
          "results sorted by id ascending");
    row(r, 0, 10, 1000, 1000, 0, 0);
    row(r, 1, 20, 1000, 1000, 0, 0);
    row(r, 2, 30, 1000, 1000, 0, 0);  // 300 <= 1000 all fit
}

}  // namespace

int main() {
    test_exclusive_full();
    test_compatible_fit();
    test_singleton_partial();
    test_overcommit_valid();
    test_ties_withhold_lower_id();
    test_remaining_zero_ties_get_capacity();
    test_mixed_reasons();
    test_mixed_counters_one_session();
    test_tie_after_partial_delivery();
    test_tie_after_partial_consumption();
    test_clipping_and_nonoverlap();
    test_empty_sessions();
    test_reorder_invariance();
    test_horizon_split_additivity();
    test_translation();
    test_rate_bounds_and_negative_priority();
    test_rejections();
    test_id_sorted_output();
    if (failures) {
        std::fprintf(stderr, "%d activity test failure(s)\n", failures);
        return 1;
    }
    std::printf("activity tests passed\n");
    return 0;
}
