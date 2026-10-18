#include "polyec/activity.hpp"

#include <algorithm>
#include <limits>

namespace polyec {
namespace activity {
namespace {

bool checked_add(int64_t a, int64_t b, int64_t *out) {
    if (b >= 0) {
        if (a > std::numeric_limits<int64_t>::max() - b) return false;
    } else {
        if (a < std::numeric_limits<int64_t>::min() - b) return false;
    }
    *out = a + b;
    return true;
}

// All evaluator quantities accumulated here are nonnegative (rates, segment
// lengths, unit totals); dynamic operands are guarded explicitly.
bool checked_mul_nonneg(int64_t a, int64_t b, int64_t *out) {
    if (a < 0 || b < 0) return false;
    if (a == 0 || b == 0) {
        *out = 0;
        return true;
    }
    if (a > std::numeric_limits<int64_t>::max() / b) return false;
    *out = a * b;
    return true;
}

EvaluateResult reject(RejectReason reason) {
    EvaluateResult result;
    result.ok = false;
    result.reason = reason;
    result.results.clear();
    return result;
}

// A session clipped to the horizon window. row indexes the output row so the
// input order never influences accounting. Clipped-out sessions never become
// items; their rows simply stay zero.
struct Item {
    std::size_t row;
    const SessionView *view;  // not owned
    int64_t begin;
    int64_t end;
};

// Priority first (higher first, comparison only — never subtraction, so int32
// extremes are safe), then id for deterministic grouping. Ties inside one group
// are treated identically by the allocation rules, so no ID/input-order winner
// can be induced by this ordering.
bool item_order(const Item &a, const Item &b) {
    if (a.view->priority != b.view->priority) return a.view->priority > b.view->priority;
    return a.view->id < b.view->id;
}

}  // namespace

EvaluateResult evaluateActivity(uint64_t actor_id, const Horizon &horizon,
                                const std::vector<SessionView> &sessions) {
    if (horizon.begin < 0 || horizon.end <= horizon.begin) return reject(RejectReason::InvalidHorizon);
    if (actor_id == 0) return reject(RejectReason::InvalidActor);

    const std::size_t n = sessions.size();
    std::vector<SessionResult> rows(n);
    std::vector<Item> items;
    items.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        const SessionView &s = sessions[i];
        if (s.id == 0) return reject(RejectReason::InvalidSession);
        if (s.actor == 0 || s.actor != actor_id) return reject(RejectReason::MixedActor);
        if (s.begin < 0 || s.end <= s.begin) return reject(RejectReason::InvalidSession);
        if (s.demand_rate < kMinDemandRate || s.demand_rate > kMaxDemandRate)
            return reject(RejectReason::InvalidSession);
        rows[i].id = s.id;
        if (s.end > horizon.begin && s.begin < horizon.end) {
            Item item;
            item.row = i;
            item.view = &s;
            item.begin = s.begin < horizon.begin ? horizon.begin : s.begin;
            item.end = s.end > horizon.end ? horizon.end : s.end;
            items.push_back(item);
        }
    }

    // Duplicate ids across the whole call: sort a stable index copy by id and
    // check neighbors (compare only; uint64 ids have no overflow here).
    {
        std::vector<std::size_t> by_id(n);
        for (std::size_t i = 0; i < n; ++i) by_id[i] = i;
        std::sort(by_id.begin(), by_id.end(),
                  [&rows](std::size_t a, std::size_t b) { return rows[a].id < rows[b].id; });
        for (std::size_t i = 1; i < n; ++i) {
            if (rows[by_id[i - 1]].id == rows[by_id[i]].id) return reject(RejectReason::DuplicateId);
        }
    }

    // Segment boundaries at session / horizon endpoints, deduplicated; each
    // segment is covered fully or not at all by every active session.
    std::vector<int64_t> bounds;
    bounds.reserve(2 * items.size());
    for (const Item &item : items) {
        bounds.push_back(item.begin);
        bounds.push_back(item.end);
    }
    std::sort(bounds.begin(), bounds.end());
    bounds.erase(std::unique(bounds.begin(), bounds.end()), bounds.end());

    for (std::size_t k = 1; k < bounds.size(); ++k) {
        const int64_t seg_begin = bounds[k - 1];
        const int64_t seg_end = bounds[k];
        const int64_t seg_len = seg_end - seg_begin;  // > 0 by construction

        std::vector<Item> active;
        for (const Item &item : items) {
            if (item.begin <= seg_begin && item.end >= seg_end) active.push_back(item);
        }
        if (active.empty()) continue;
        std::sort(active.begin(), active.end(), item_order);

        int64_t remaining = kActorCapacityRate;  // attention rate units per tick
        bool withheld = false;                   // tie overflow happened: segment blocked
        std::size_t g = 0;
        while (g < active.size()) {
            const std::size_t h = g;
            const int32_t priority = active[g].view->priority;
            int64_t group_rate = 0;
            while (g < active.size() && active[g].view->priority == priority) {
                if (!checked_add(group_rate, active[g].view->demand_rate, &group_rate))
                    return reject(RejectReason::Overflow);
                ++g;
            }
            // Group demand request units, exactly once per member/segment.
            for (std::size_t m = h; m < g; ++m) {
                int64_t requested_units = 0;
                if (!checked_mul_nonneg(active[m].view->demand_rate, seg_len, &requested_units))
                    return reject(RejectReason::Overflow);
                SessionResult &row_out = rows[active[m].row];
                if (!checked_add(row_out.requested, requested_units, &row_out.requested))
                    return reject(RejectReason::Overflow);
            }
            if (withheld) {
                // Segment blocked by an earlier tied overflow: all lower
                // demand (any group) is unresolved with nothing delivered.
                for (std::size_t m = h; m < g; ++m) {
                    int64_t units = 0;
                    if (!checked_mul_nonneg(active[m].view->demand_rate, seg_len, &units))
                        return reject(RejectReason::Overflow);
                    SessionResult &out = rows[active[m].row];
                    if (!checked_add(out.unresolved_unmet, units, &out.unresolved_unmet))
                        return reject(RejectReason::Overflow);
                }
            } else if (remaining == 0) {
                // Capacity fully consumed by higher priorities (delivered or
                // singleton-assigned): everyone here is a capacity shortfall,
                // even ties.
                for (std::size_t m = h; m < g; ++m) {
                    int64_t units = 0;
                    if (!checked_mul_nonneg(active[m].view->demand_rate, seg_len, &units))
                        return reject(RejectReason::Overflow);
                    SessionResult &out = rows[active[m].row];
                    if (!checked_add(out.capacity_unmet, units, &out.capacity_unmet))
                        return reject(RejectReason::Overflow);
                }
            } else if (group_rate <= remaining) {
                // Fits: full delivery, remaining rate descends by G (rate
                // units, NOT rate * len).
                for (std::size_t m = h; m < g; ++m) {
                    int64_t units = 0;
                    if (!checked_mul_nonneg(active[m].view->demand_rate, seg_len, &units))
                        return reject(RejectReason::Overflow);
                    SessionResult &out = rows[active[m].row];
                    if (!checked_add(out.delivered, units, &out.delivered))
                        return reject(RejectReason::Overflow);
                }
                remaining -= group_rate;
            } else if (g - h == 1) {
                // Singleton overflow: deliver the remaining rate to the only
                // member, then the segment's remaining rate is exhausted.
                const int64_t rate = active[h].view->demand_rate;
                int64_t assigned = rate < remaining ? rate : remaining;
                int64_t delivered_units = 0;
                if (!checked_mul_nonneg(remaining, seg_len, &delivered_units))
                    return reject(RejectReason::Overflow);
                int64_t shortfall_units = 0;
                if (!checked_mul_nonneg(rate - assigned, seg_len, &shortfall_units))
                    return reject(RejectReason::Overflow);
                SessionResult &out = rows[active[h].row];
                if (!checked_add(out.delivered, delivered_units, &out.delivered))
                    return reject(RejectReason::Overflow);
                if (!checked_add(out.capacity_unmet, shortfall_units, &out.capacity_unmet))
                    return reject(RejectReason::Overflow);
                remaining = 0;
            } else {
                // Tied overflow with positive remaining: withhold the whole
                // remainder; nobody is picked by ID or input order; every
                // lower-priority group in this segment becomes unresolved.
                withheld = true;
                for (std::size_t m = h; m < g; ++m) {
                    int64_t units = 0;
                    if (!checked_mul_nonneg(active[m].view->demand_rate, seg_len, &units))
                        return reject(RejectReason::Overflow);
                    SessionResult &out = rows[active[m].row];
                    if (!checked_add(out.unresolved_unmet, units, &out.unresolved_unmet))
                        return reject(RejectReason::Overflow);
                }
            }
        }
    }

    // Reason counters sum to the shortfall; delivered + unmet == requested.
    for (std::size_t i = 0; i < n; ++i) {
        SessionResult &row = rows[i];
        if (!checked_add(row.capacity_unmet, row.unresolved_unmet, &row.unmet))
            return reject(RejectReason::Overflow);
        int64_t delivered_plus_unmet = 0;
        if (!checked_add(row.delivered, row.unmet, &delivered_plus_unmet))
            return reject(RejectReason::Overflow);
        if (delivered_plus_unmet != row.requested) return reject(RejectReason::Overflow);
    }

    // Report sorted by session id ascending; ids are unique positive values.
    std::vector<std::size_t> order(n);
    for (std::size_t i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(),
              [&rows](std::size_t a, std::size_t b) { return rows[a].id < rows[b].id; });
    std::vector<SessionResult> sorted_results;
    sorted_results.reserve(n);
    for (std::size_t i = 0; i < n; ++i) sorted_results.push_back(rows[order[i]]);

    EvaluateResult result;
    result.ok = true;
    result.reason = RejectReason::None;
    result.results = std::move(sorted_results);
    return result;
}

}  // namespace activity
}  // namespace polyec
