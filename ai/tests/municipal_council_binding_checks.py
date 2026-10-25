"""Check that C++ council commands and observer filtering reach Python intact."""

import polyec_env as core


assert core.REALM_SCHEMA == "realm-v43"
assert core.REALM_ACTIONS == 400 and core.REALM_OBS == 5182

world = core.Realm(20261024, 26, 512)
sovereign = world.view(1)
assert len(sovereign["urban_census"]) == 27
assert all(row["council_representative_id"] == 0 for row in sovereign["urban_census"])
assert all(row["council_nominee_id"] == 0 for row in sovereign["urban_census"])

nominee = next((person_id for person_id in range(1, 513)
                if world.mask(person_id)[71] == 1.0), None)
assert nominee is not None, "Atlas opening should have an eligible adult urban resident"
nominee_person = world.person(nominee)
region = nominee_person["region"]
ok, reason = world.step(nominee, 71)
assert ok, reason

site_row = next(row for row in world.view(1)["urban_census"]
                if row["territory_id"] == region)
site_id = site_row["site_id"]
assert site_row["council_nominee_id"] == nominee
assert site_row["council_nomination_day"] == world.view(1)["day"]
assert world.mask(1)[282 + site_id] == 1.0
ok, reason = world.step(1, 282 + site_id)
assert ok, reason

appointed = world.view(1)["urban_census"]
site_row = next(row for row in appointed if row["site_id"] == site_id)
assert site_row["council_representative_id"] == nominee
assert site_row["council_nominee_id"] == 0
assert site_row["council_term_end_day"] == world.view(1)["day"] + 4 * 365

local_rows = world.view(nominee)["urban_census"]
assert len(local_rows) == 1 and local_rows[0]["site_id"] == site_id
other_region_actor = next(person_id for person_id in range(1, 513)
                          if person_id != 1 and world.person(person_id)["region"] != region
                          and world.view(person_id)["urban_census"])
assert all(row["territory_id"] != region
           for row in world.view(other_region_actor)["urban_census"])

print("municipal_council_binding_checks: 13 assertions passed")
