"""Check bounded foreign trade through the C++ core Python API used by training."""
import polyec_env as core


def main() -> None:
    world = core.Realm(61327, 26, 256)
    merchant = next(
        person_id for person_id in range(1, 257)
        if world.person(person_id)["alive"]
        and world.person(person_id)["family"] == 1
        and world.person(person_id)["region"] == 8
    )
    governor = 10
    assert core.REALM_SCHEMA == "realm-v30"
    assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
    assert world.model_contract == "realm-household-fund-goods27-config-v32"
    import_route = next(row for row in world.transport_routes if row["from"] == 9 and row["to"] == 8)
    assert import_route["open"] and import_route["travel_days"] == 2 and import_route["fee_per_unit"] == 1
    assert world.foreign_trade_cash == 5000
    assert world.foreign_trade_goods[0] == 32
    assert world.foreign_trade_salt_capacity == 32
    assert world.foreign_trade_escrow == 0
    assert world.mask(merchant)[32:35] == [1.0] * 3
    assert world.mask(merchant)[35:38] == [0.0] * 3
    assert world.observe(merchant)[117:120] == [1.0, 1.0, 1.0]
    assert world.observe(governor)[117:120] == [0.0, 0.0, 0.0]
    before = world.fingerprint()
    rejected, reason, _ = world.foreign_trade_command(
        governor, world.person(governor)["decisions"] + 1, 0, 0, 1
    )
    assert not rejected and reason and world.fingerprint() == before

    accepted, reason, order_id = world.foreign_trade_command(
        merchant, world.person(merchant)["decisions"] + 1, 0, 0, 1
    )
    assert accepted, reason
    assert order_id > 0
    assert world.foreign_trade_goods[0] == 31
    assert world.foreign_trade_escrow == 25
    orders = world.foreign_trade_orders()
    assert len(orders) == 1 and orders[0]["id"] == order_id
    assert orders[0]["created_day"] == 0 and orders[0]["arrival_day"] == 3
    pending = world.serialize("foreign-binding-v1")
    restored = core.Realm(1, 1, 128)
    loaded, error = restored.deserialize(pending, "foreign-binding-v1")
    assert loaded, error
    assert restored.foreign_trade_orders() == orders
    assert restored.invariant()

    duplicate_before = world.fingerprint()
    duplicate, reason, _ = world.foreign_trade_command(
        merchant, world.person(merchant)["decisions"] + 1, 0, 0, 1
    )
    assert not duplicate and reason and world.fingerprint() == duplicate_before
    print("foreign_trade_binding_checks=22")


if __name__ == "__main__":
    main()

