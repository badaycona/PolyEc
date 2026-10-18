#pragma once
// Pure, stateless per-actor activity evaluator (M5 prerequisite slice).
//
// Scope guard: this unit ONLY totals attention-capacity demand over an abstract
// tick window for one actor, from immutable session views. It has NO dependency
// on Realm, saves, actions, observation, rewards, or health/wage/care state, and
// it never mutates its input. Realm command/clock/save integration is a later
// gate and is out of scope here.
//
// Capacity ownership: the global attention capacity is a constant 1000 attention
// units per tick for the single actor. It is NOT a session field and NOT a call
// input. Sessions demand an integer attention rate in [1, 1000] units/tick; the
// evaluator descends priorities per segment against a remaining-rate budget and
// multiplies an assigned rate by segment length only when accumulating integer
// delivered/unmet units. All unmet unit totals are exact integers, never
// proportional blends.

#include <cstdint>
#include <vector>

namespace polyec {
namespace activity {

// Constant global attention capacity rate for the single actor (per tick).
inline constexpr int32_t kActorCapacityRate = 1000;

// Inclusive bounds on a session's attention demand rate (units per tick).
inline constexpr int32_t kMinDemandRate = 1;
inline constexpr int32_t kMaxDemandRate = 1000;

// Absolute half-open evaluation window in abstract ticks: [begin, end).
// Valid iff 0 <= begin < end. There is no implicit origin 0 and no window
// length parameter; ticks carry no physical minutes/day semantics.
struct Horizon {
    int64_t begin = 0;
    int64_t end = 0;
};

// Immutable per-session request view (POD; the evaluator never mutates input).
struct SessionView {
    uint64_t id = 0;                  // positive, unique within the call
    uint64_t actor = 0;               // positive, must equal the call's actor
    int64_t begin = 0;                // absolute, nonnegative
    int64_t end = 0;                  // absolute, begin < end (half-open)
    int32_t priority = 0;             // explicit; any int32; higher served first
    int32_t demand_rate = 0;          // attention units demanded per tick [1, 1000]
};

// Rejection categories. ok == true iff reason == RejectReason::None.
enum class RejectReason : int32_t {
    None = 0,
    InvalidHorizon,   // begin < 0 or end <= begin (includes zero-length)
    InvalidActor,     // actor_id == 0
    InvalidSession,   // zero id / bad begin<end / begin<0 / rate out of bounds
    MixedActor,       // session.actor != actor_id (covers nonpositive actor)
    DuplicateId,      // repeated positive session id within the call
    Overflow,         // checked int64 arithmetic refusal (no partial output)
};

// Per-session integer unit totals (exact, never proportional).
// Invariants: delivered + unmet == requested; unmet == capacity_unmet +
// unresolved_unmet (the two reason counters sum to the shortfall exactly).
struct SessionResult {
    uint64_t id = 0;
    int64_t requested = 0;        // demand_rate * clipped overlap length
    int64_t delivered = 0;        // assigned_rate * segment length, summed
    int64_t unmet = 0;            // capacity_unmet + unresolved_unmet
    int64_t capacity_unmet = 0;   // unmet due to exhausted/consumed capacity
    int64_t unresolved_unmet = 0; // unmet due to withheld capacity (tie rule)
};

// ok/reason/results contract. Rejections carry EMPTY results (no partial
// output) and leave inputs untouched; success sorts results by session id
// ascending and preserves exactly one row per valid input session, including
// zero-total rows for sessions clipped fully out of the horizon.
struct EvaluateResult {
    bool ok = false;
    RejectReason reason = RejectReason::None;
    std::vector<SessionResult> results;
};

// Evaluate one actor's sessions over the absolute window horizon [begin, end).
// Pure function: no global state, no Realm access, no input mutation.
EvaluateResult evaluateActivity(uint64_t actor_id, const Horizon &horizon,
                                const std::vector<SessionView> &sessions);

}  // namespace activity
}  // namespace polyec
