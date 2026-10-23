import json

import polyec_env as core


world = core.Realm(9041, 1, 256)
household = authority = contributor = 0
for person_id in range(1, 257):
    label = world.household_of(person_id)
    candidate_authority = world.household_authority(label)
    if not candidate_authority:
        continue
    candidate_contributor = next((member for member in range(1, 257)
                                  if world.household_of(member) == label
                                  and member != candidate_authority
                                  and world.person(member)["age_days"] >= 18 * 365), 0)
    if candidate_contributor:
        household, authority, contributor = label, candidate_authority, candidate_contributor
        break

assert household and authority and contributor
assert world.household_fund(household) == 0
deposit = world.household_fund_command(contributor, world.person(contributor)["decisions"] + 1,
                                       household, 0, 60)
assert deposit[0], deposit
assert world.household_fund(household) == 60
assert world.person(contributor)["cash"] == 40
withdraw = world.household_fund_command(authority, world.person(authority)["decisions"] + 1,
                                        household, 1, 25)
assert withdraw[0], withdraw
assert world.household_fund(household) == 35
saved = world.serialize("household-binding")
restored = core.Realm(1, 1, 128)
ok, error = restored.deserialize(saved, "household-binding")
assert ok, error
assert restored.household_fund(household) == 35
assert restored.fingerprint() == world.fingerprint()
assert world.invariant() and restored.invariant()

print(json.dumps({"checks": 11, "household_fund": 35,
                  "command": "shared C++ Realm transaction",
                  "schema": saved.split(" ", 1)[0]}, sort_keys=True))

school = core.Realm.scenario(37, 129, 8, 50, 0, 0)
school_household = school.household_of(129)
adult = school.household_authority(school_household)
assert adult and school.person(adult)["cash"] >= 5
assert school.mask(129)[15] == 0, "guardian private cash must not enable learner spending"
ok, reason, _ = school.household_fund_command(adult, school.person(adult)["decisions"] + 1,
                                              school_household, 0, 5)
assert ok, reason
assert school.mask(129)[15] == 1, "funded household enables learner's own Study"
cash_before = school.person(129)["cash"]
ok, reason, _ = school.intent_command(129, school.person(129)["decisions"] + 1,
                                      15, school.person(129)["region"], 0, 5, 1)
assert ok, reason
assert school.person(129)["cash"] == cash_before and school.household_fund(school_household) == 0
assert school.invariant()
print(json.dumps({"checks": 7, "case": "Study pays from household account; guardian private cash is excluded"}, sort_keys=True))





