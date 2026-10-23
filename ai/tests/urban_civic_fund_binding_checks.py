"""Check the urban civic fund through the same C++ Realm used by training."""
import polyec_env as core


def main() -> None:
    world = core.Realm(901, 6, 128)
    actor = 1  # Sovereign, assigned to region 6 in this fixture.
    region = world.person(actor)["region"]
    assert region == 6
    assert core.REALM_SCHEMA == "realm-v30"
    assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
    assert world.model_contract == "realm-household-fund-goods27-config-v32"
    assert len(world.municipal_funds) == 7 and world.municipal_funds[region] == 0
    assert world.observe(actor)[116] == 0.0

    mask = world.mask(actor)
    assert all(mask[index] == 1.0 for index in range(32, 36))
    ok, reason = world.step(actor, 35)
    assert ok, reason
    assert world.municipal_funds[region] == 500
    assert world.view(actor)["provinces"][region]["treasury"] == 9500
    assert abs(world.observe(actor)[116] - 0.05) < 1e-6
    assert world.invariant()

    relief_index = 69
    relief_observation_index = 301 + relief_index * 12 + 10
    assert world.mask(actor)[relief_index] == 1.0
    assert abs(world.observe(actor)[relief_observation_index] - 1.0) < 1e-6
    opening_money = world.view(actor)["money"]
    ok, reason = world.step(actor, relief_index)
    assert ok, reason
    assert world.municipal_funds[region] == 450
    assert world.view(actor)["provinces"][region]["treasury"] == 9500
    assert world.view(actor)["money"] == opening_money and world.invariant()
    assert any(fact["kind"] == 232 and fact["amount"] == 50
               for fact in world.view(actor)["facts"])

    used = world.mask(actor)
    assert all(used[index] == 0.0 for index in range(32, 36))
    fingerprint = world.fingerprint()
    rejected, reason, _ = world.municipal_budget_command(
        actor, world.person(actor)["decisions"] + 1, region, 1
    )
    assert not rejected and reason
    assert world.fingerprint() == fingerprint

    assert world.mask(actor)[37] == 1.0
    ok, reason = world.step(actor, 37)
    assert ok, reason
    row = world.establishments()[0]
    assert row["owner_kind"] == 3 and row["cash"] == 50
    assert world.municipal_funds[region] == 400

    ok, reason, site = world.establishment_command(
        actor,
        world.person(actor)["decisions"] + 1,
        3,  # Municipal owner kind.
        region,
        region,
        1,  # Cloth workshop.
        1,
        25,
    )
    assert ok, reason
    typed_site = next(item for item in world.establishments() if item["id"] == site)
    assert typed_site["owner_kind"] == 3 and typed_site["cash"] == 25
    assert world.municipal_funds[region] == 375
    assert world.invariant()
    assert world.observe(65)[116] == 0.0  # Civilians cannot observe the council account in policy input.
    print("urban_civic_fund_binding_checks=27")


if __name__ == "__main__":
    main()

