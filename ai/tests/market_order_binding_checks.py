"""Exercise market-order commands through the shared C++ Realm Python binding."""

import polyec_env as core


checks = 0


def check(condition, message):
    global checks
    checks += 1
    if not condition:
        raise AssertionError(message)


world = core.Realm(901, 1, 128)
good = 1
buyer = 0
region = -1
unit_price = 0
for actor in range(1, world.view(1)["persons"] + 1):
    person = world.person(actor)
    if not person["alive"] or person["age_days"] < 6 * 365 or actor % 7 == 1:
        continue
    actor_region = person["region"]
    price = world.view(actor)["provinces"][actor_region]["prices"][good]
    if person["cash"] >= price:
        buyer, region, unit_price = actor, actor_region, price
        break
check(buyer > 0, "fixture finds a living buyer with enough funds")

opening_total = world.view(buyer)["money"]
opening_cash = world.person(buyer)["cash"]
placed, reason, order_id = world.market_order_command(
    buyer,
    world.person(buyer)["decisions"] + 1,
    core.MarketOrderOperation.Place,
    0,
    region,
    good,
    1,
    unit_price,
)
check(placed and reason == "Order accepted and money reserved" and order_id > 0,
      "Place reaches the core and returns its stable order ID and explanation")
orders = world.market_orders()
check(len(orders) == 1 and orders[0]["id"] == order_id, "Python query reads the core's open order")
check(orders[0]["owner"] == buyer and orders[0]["region"] == region and orders[0]["good"] == good,
      "open order retains its owner, region and product")
check(orders[0]["quantity"] == 1 and orders[0]["maximum_unit_price"] == unit_price,
      "open order retains requested amount and maximum price")
check(orders[0]["personal_escrow"] + orders[0]["household_escrow"] == unit_price and
      orders[0]["organization_escrow"] == 0 and world.market_escrow == unit_price,
      "held money records the exact funding source and matches the market escrow")
check(world.person(buyer)["cash"] + orders[0]["personal_escrow"] == opening_cash and
      world.view(buyer)["money"] == opening_total,
      "placing an order holds money without changing total Realm money")

before_stale = world.fingerprint()
stale, _, _ = world.market_order_command(
    buyer, world.person(buyer)["decisions"], core.MarketOrderOperation.Cancel, order_id
)
check(not stale and world.fingerprint() == before_stale, "stale cancellation is rejected without mutation")

saved = world.serialize("python-market-order")
restored = core.Realm(1, 1, 128)
loaded, error = restored.deserialize(saved, "python-market-order")
check(loaded, error)
check(restored.fingerprint() == world.fingerprint() and restored.market_orders() == orders,
      "pending order and its held money round-trip through the C++ save loader")

cancelled, reason, _ = restored.market_order_command(
    buyer,
    restored.person(buyer)["decisions"] + 1,
    core.MarketOrderOperation.Cancel,
    order_id,
)
check(cancelled and reason == "Order cancelled and reservation refunded" and restored.market_orders() == [],
      "owner cancels the open order and receives the core's explanation")
check(restored.market_escrow == 0 and restored.person(buyer)["cash"] == opening_cash and
      restored.view(buyer)["money"] == opening_total,
      "cancellation releases the original funding source and preserves total money")

before_bad = restored.fingerprint()
bad_quantity, _, _ = restored.market_order_command(
    buyer,
    restored.person(buyer)["decisions"] + 1,
    core.MarketOrderOperation.Place,
    0,
    region,
    good,
    0,
    unit_price,
)
check(not bad_quantity and restored.fingerprint() == before_bad,
      "zero-quantity order is rejected by the core without partial state change")

print(f"market_order_binding_checks={checks} order_id={order_id} held={unit_price}")





