"""Independent arithmetic oracle for the reviewed payroll migration contract.

Enumerates every legal allocation of bridge credit across three sources.
It does not implement or select the production pro-rata allocation algorithm.
"""
from itertools import product
import json


def possible_entitlements(ticks_per_unit, old_ticks, new_ticks):
    old_eligible = [n // ticks_per_unit for n in old_ticks]
    eligible = [n // ticks_per_unit for n in new_ticks]
    old_region = sum(old_ticks) // ticks_per_unit
    bridge = old_region - sum(old_eligible)
    available = [end - start for start, end in zip(old_eligible, eligible)]
    allocated = min(bridge, sum(available))
    candidates = []
    for allocation in product(*(range(min(n, allocated) + 1) for n in available)):
        if sum(allocation) != allocated:
            continue
        credits = [prior + offset for prior, offset in zip(old_eligible, allocation)]
        remaining = bridge - allocated
        rights = [old_region + eligible[0] - credits[0]]
        rights += [eligible[i] - credits[i] for i in (1, 2)]
        assert old_region == sum(credits) + remaining
        assert all(n >= 0 for n in rights)
        assert sum(rights) == max(old_region, sum(eligible))
        assert sum(rights) <= sum(new_ticks) // ticks_per_unit
        assert rights[0] >= old_region  # outstanding historical debt survives
        if remaining:
            assert rights == [old_region, 0, 0]
        candidates.append(rights)
    assert candidates
    return old_region, candidates


checks = 0
for denominator in range(1, 6):
    for old in product(range(2 * denominator), repeat=3):
        for added in product((0, 1, denominator), repeat=3):
            new = [start + delta for start, delta in zip(old, added)]
            possible_entitlements(denominator, old, new)
            checks += 1

examples = [
    ((0, 8, 0), (0, 16, 0), 1),
    ((0, 8, 8), (0, 16, 16), 2),
    ((8, 8, 0), (8, 16, 0), 1),
    ((8, 8, 0), (16, 16, 0), 2),
    ((0, 32, 16), (0, 32, 16), 3),
]
for old, new, expected in examples:
    _, candidates = possible_entitlements(16, old, new)
    assert all(sum(rights) == expected for rights in candidates)

# A wage of three with one money unit already paid: old debt two survives,
# plus three new money units, for lifetime entitlement six (not nine).
old_region, candidates = possible_entitlements(16, (0, 8, 8), (0, 16, 16))
assert old_region == 1
for rights in candidates:
    assert rights[0] * 3 - 1 == 2
    assert sum(rights[1:]) * 3 == 3
    assert 1 + (rights[0] * 3 - 1) + sum(rights[1:]) * 3 == 6

print(json.dumps({"exhaustive_scenarios": checks, "sources": 3,
                  "denominators": [1, 2, 3, 4, 5], "contract_examples": 6,
                  "scope": "arithmetic contract; no Realm payroll integration claimed"}))
