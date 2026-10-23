"""Check that Python can see and use the current public ration depot contract."""
import polyec_env as core


assert core.REALM_SCHEMA == "realm-v30"
assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
world = core.Realm(37, 26, 128)
assert world.military_unit_rosters == []
view = world.view()
assert len(view["provinces"]) == 27
assert all(row["military_ration_stock"] == 0 for row in view["provinces"])
assert all(row["military_supplementary_ration_stock"] == 0 for row in view["provinces"])

case = None
for actor in range(1, 129):
    person = world.person(actor)
    if person["family"] != 0 or not person["taxing_authority"]:
        continue
    mask = world.mask(actor)
    purchase = next((i for i in range(189, 193) if mask[i]), None)
    if purchase is None:
        continue
    case = actor, purchase
    break
assert case is not None, "fixture contains a local Governor with a legal depot purchase"

actor, purchase = case
region = world.person(actor)["region"]
before_treasury = view["provinces"][region]["treasury"]
ok, reason = world.step(actor, purchase)
assert ok, reason
after = world.view()
assert after["provinces"][region]["military_ration_stock"] > 0
assert after["provinces"][region]["treasury"] < before_treasury
assert world.invariant()

shipment = next((i for i in range(193, 220) if world.mask(actor)[i]), None)
assert shipment is not None, "purchased ration creates a legal route shipment choice"
ok, reason = world.step(actor, shipment)
assert ok, reason
public = [row for row in world.transport_shipments if row["owner_kind"] == 1]
assert public and public[-1]["source_region"] == region
assert public[-1]["owner"] == actor
assert world.invariant()

created = public[-1]
ok, reason, _, _ = world.transport_shipment_command(
    actor, world.person(actor)["decisions"] + 1, shipment_id=created["id"], operation=1,
    quantity=created["quantity"], military_ration_depot=True)
assert ok, reason
ok, reason, _, _ = world.transport_shipment_command(
    actor, world.person(actor)["decisions"] + 1, operation=0,
    destination=created["destination"], good=created["good"], quantity=created["quantity"],
    goal=created["service_goal"], military_ration_depot=True)
assert ok, reason
assert world.transport_shipments[-1]["owner_kind"] == 1
assert world.transport_shipments[-1]["source_region"] == region
assert world.invariant()

supplementary_case = None
for candidate_actor in range(1, 129):
    person = world.person(candidate_actor)
    if person["family"] != 0 or not person["taxing_authority"]:
        continue
    purchase_action = next((i for i in range(232, 236) if world.mask(candidate_actor)[i]), None)
    if purchase_action is not None:
        supplementary_case = candidate_actor, purchase_action
        break
assert supplementary_case is not None, "Atlas specialty source has a legal SupplementaryFood purchase"
supplementary_actor, supplementary_purchase = supplementary_case
supplementary_region = world.person(supplementary_actor)["region"]
ok, reason = world.step(supplementary_actor, supplementary_purchase)
assert ok, reason
after_supplementary = world.view()
assert after_supplementary["provinces"][supplementary_region]["military_supplementary_ration_stock"] > 0
assert after_supplementary["provinces"][supplementary_region]["military_ration_stock"] == 0
supplementary_shipment = next((i for i in range(236, 263) if world.mask(supplementary_actor)[i]), None)
assert supplementary_shipment is not None, "SupplementaryFood stock creates its own legal route choice"
ok, reason = world.step(supplementary_actor, supplementary_shipment)
assert ok, reason
assert world.transport_shipments[-1]["good"] == 1
assert world.invariant()

print({"schema": core.REALM_SCHEMA, "observation_values": core.REALM_OBS,
       "action_choices": core.REALM_ACTIONS, "actor": actor, "source_region": region,
       "dry_food_depot_stock_after_purchase": after["provinces"][region]["military_ration_stock"],
       "supplementary_depot_stock_after_purchase": after_supplementary["provinces"][region]["military_supplementary_ration_stock"],
       "supplementary_source_region": supplementary_region,
       "supplementary_public_shipment": world.transport_shipments[-1]["id"], "checks": 22})
