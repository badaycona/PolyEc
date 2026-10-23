"""Exercise the policy workshop path through shared Realm commands."""
import polyec_env as core

world = core.Realm(37, 6, 256)
economic = next(
    person_id for person_id in range(1, 257)
    if world.person(person_id)["family"] == 1 and world.person(person_id)["cash"] >= 100
)
person = world.person(economic)
mask = world.mask(economic)
assert list(mask[80:89]) == [1.0] * 9
opening_cash = person["cash"]
observation = world.observe(economic)
fiber_buy = next(
    slot for slot in range(core.REALM_ACTIONS)
    if world.mask(economic)[slot] > 0
    and abs(observation[128 + slot * 12] - 1 / 32) < 1e-6
    and abs(observation[128 + slot * 12 + 2] - 9 / 26) < 1e-6
)
assert world.mask(economic)[101] == 0.0  # No worksite exists yet.
ok, reason = world.step(economic, 80)  # Cloth workshop, 25 cash funding.
assert ok, reason
site = world.establishments()[0]
assert (site["owner_kind"], site["owner_id"], site["region"], site["industry"], site["cash"]) == (
    0, economic, person["region"], 1, 25
)
assert world.person(economic)["cash"] == opening_cash - 25 and world.invariant()
assert world.mask(economic)[101] == 0.0  # Site exists, but the actor has no Fiber.
observation = world.observe(economic)
fiber_buy = next(
    slot for slot in range(core.REALM_ACTIONS)
    if world.mask(economic)[slot] > 0
    and abs(observation[128 + slot * 12] - 1 / 32) < 1e-6
    and abs(observation[128 + slot * 12 + 2] - 9 / 26) < 1e-6
)
ok, reason = world.step(economic, fiber_buy)
assert ok, reason
assert world.mask(economic)[101] == 1.0
transfer_observation = world.observe(economic)[128 + 101 * 12:128 + 102 * 12]
assert abs(transfer_observation[0] - 26 / 32) < 1e-6  # TransferEstablishmentGoods
assert transfer_observation[5] == 1.0 and transfer_observation[6] == 1.0
assert abs(transfer_observation[1] - 1 / 7) < 1e-6  # Cloth workshop among seven supported industries
assert transfer_observation[10] > 0.0  # Actor has recipe input in personal inventory
ok, reason = world.step(economic, 101)  # Deposit recipe input.
assert ok, reason
ok, reason, _ = world.schedule_command(
    0, economic, world.person(economic)["decisions"] + 1, 0, world.game_tick,
    world.game_tick + 1, 0, 5, 0, -1, 0
)
assert ok, reason
assert world.timeline_contract and len(world.observe(economic)) == core.REALM_OBS
assert len(world.mask(economic)) == core.REALM_ACTIONS and world.mask(economic)[104] == 1.0
ok, reason = world.step(economic, 104)  # Authorize a present second worker.
assert ok, reason

authority = next(
    person_id for person_id in range(1, 257)
    if world.person(person_id)["family"] == 0 and world.person(person_id)["taxing_authority"]
)
treasury_before = world.view()["provinces"][world.person(authority)["region"]]["treasury"]
ok, reason = world.step(authority, 80)
assert ok, reason
sites = world.establishments()
assert len(sites) == 2 and sites[1]["owner_kind"] == 1
assert sites[1]["cash"] == 25
assert world.view()["provinces"][world.person(authority)["region"]]["treasury"] == treasury_before - 25
assert world.invariant()
print("POLICY WORKSHOP BINDING: Fiber buy/deposit, private/public creation, post-timeline access, and authorization passed")





