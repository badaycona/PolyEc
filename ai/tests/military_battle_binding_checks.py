"""Check that Python exposes the same raid and battle records as the C++ core."""
import polyec_env as core


world = core.Realm(41401, 26, 128)
assert world.military_raids == []
assert world.military_battle_candidates(8) == []
assert world.military_battle_candidates(26) == []
partner = world.foreign_trade_partners[0]
assert partner["decision_sequence"] == 0
before = world.fingerprint()
ok, reason, _, _ = world.military_raid_command(0, 1, 9)
assert not ok and reason and world.fingerprint() == before

ok, reason, _, raid_id = world.military_raid_command(0, 1, 1)
assert ok, reason
assert raid_id > 0
assert world.military_raids == [{
    "id": raid_id,
    "partner_id": 0,
    "target_region": 8,
    "created_day": 0,
    "resolved_day": -1,
    "force_units": 1,
    "defense_score": 0,
    "attacker_losses": 0,
    "defender_casualties": 0,
    "salt_looted": 0,
    "status": 0,
    "defenders": [],
}]

pending_save = world.serialize("military-battle-binding")
restored = core.Realm(1, 1, 128)
loaded, error = restored.deserialize(pending_save, "military-battle-binding")
assert loaded, error
assert restored.military_raids == world.military_raids
assert restored.invariant()

sovereign = next(person_id for person_id in range(1, 129)
                 if world.person(person_id)["family"] == 0
                 and world.person(person_id)["alive"]
                 and world.person(person_id)["taxing_authority"]
                 and world.person(person_id)["region"] == 26)
before = world.fingerprint()
ok, reason, _, _ = world.military_battle_command(
    sovereign, world.person(sovereign)["decisions"] + 1, raid_id, True, []
)
assert ok, reason
assert world.military_raids[0]["status"] == 3
assert world.military_raids[0]["defenders"] == []
assert world.invariant()

closed = core.Realm(41402, 26, 128)
assert closed.route(1, 9, 8, False)
before = closed.fingerprint()
ok, reason, _, _ = closed.military_raid_command(0, 1, 1)
assert not ok and reason and closed.fingerprint() == before

print({"schema": core.REALM_SCHEMA, "raid_id": raid_id, "resolved_status": 3,
       "closed_route_rejected": True, "invariant": world.invariant()})
