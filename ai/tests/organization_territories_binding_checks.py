"""Check organization ID49 and expanded full-map observations through C++ binding."""
import json

import polyec_env as core

REGION = 12
ORG_ID = 1 + 4 * REGION
world = core.Realm(49501, 26, 256)
people = [world.person(person_id) for person_id in range(1, 257)]
residents = [p["id"] for p in people if p["alive"] and p["region"] == REGION
             and p["family"] == 2 and p["age_days"] >= 6 * 365]
member, outsider = residents[:2]

assert core.REALM_SCHEMA == "realm-v30"
assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
assert len(world.observe(member)) == 3484 and len(world.mask(member)) == 263
checks = 4

before_money = world.view()["money"]
result = world.intent_command(member, world.person(member)["decisions"] + 1,
                              18, REGION, 0, 2, 30)
assert result[0], result
joined = world.observe(member)
assert abs(float(joined[95]) - ORG_ID / 108.0) < 1e-6
assert abs(float(joined[96]) - 0.002) < 1e-6
assert world.view()["money"] == before_money and world.invariant()
checks += 3

created = world.establishment_command(member, world.person(member)["decisions"] + 1,
                                      2, ORG_ID, REGION, 1, 1, 1)
assert created[0], created
site = world.establishments()[0]
assert site["owner_kind"] == 2 and site["owner_id"] == ORG_ID and site["region"] == REGION
after_member = world.fingerprint()
checks += 2

rejected = world.establishment_command(outsider, world.person(outsider)["decisions"] + 1,
                                       2, ORG_ID, REGION, 1, 1, 1)
assert not rejected[0] and rejected[1] == "Actor not organization member"
assert world.fingerprint() == after_member and world.invariant()
checks += 2

saved = world.serialize("organization-territories-v1")
assert saved.startswith('POLYEC_REALM_52 "')
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "organization-territories-v1")[0]
assert abs(float(resumed.observe(member)[95]) - ORG_ID / 108.0) < 1e-6
assert resumed.establishments()[0]["owner_id"] == ORG_ID and resumed.invariant()
checks += 3

print(json.dumps({"checks": checks, "schema": core.REALM_SCHEMA,
                  "observation_values": core.REALM_OBS,
                  "action_choices": core.REALM_ACTIONS,
                  "organization_id": ORG_ID, "region": REGION,
                  "member_authorization": "accepted", "nonmember": "rejected atomically",
                  "save_schema": 44, "invariant": resumed.invariant()}, sort_keys=True))
