extends SceneTree

var checks := 0
var failed := false

func check(ok: bool, message: String) -> void:
	checks += 1
	if not ok:
		failed = true
		push_error("MILITARY OCCUPATION UI FAIL: " + message)

func _initialize() -> void:
	call_deferred("run")

func find_legal_action(snapshot: Dictionary, action_name: String) -> Dictionary:
	for action in snapshot.get("actions", []):
		if str(action.get("name", "")) == action_name and bool(action.get("legal", false)):
			return action
	return {}

func find_button_with_prefix(node: Node, prefix: String) -> Button:
	for child in node.get_children():
		if child is Button and str(child.text).begins_with(prefix):
			return child
		var nested := find_button_with_prefix(child, prefix)
		if nested != null:
			return nested
	return null

func run() -> void:
	var session = ClassDB.instantiate("PolyEcSession")
	check(session.has_method("realm_military_counterattack"), "GDExtension exposes the counterattack command")
	check(session.realm_schema() == "realm-v45", "GDExtension reports the Realm model contract with terrain route time")
	check(session.realm_atlas_manual_start(9208, 512), "manual Atlas fixture starts through the current C++ Realm")
	if failed:
		quit(1)
		return

	var sovereign: Dictionary = session.realm_snapshot(1)
	check(sovereign.get("military_occupations", []) is Array and sovereign.military_occupations.is_empty(), "a new Atlas save does not report an invented occupation")
	check(sovereign.get("military_battle_candidates_by_region", {}) is Dictionary, "the snapshot groups eligible volunteers by actual target region")
	var fingerprint := str(sovereign.get("fingerprint", ""))
	var rejected: Dictionary = session.realm_military_counterattack(1, 12, false, PackedInt64Array(), fingerprint)
	var after_reject: Dictionary = session.realm_snapshot(1)
	check(not bool(rejected.get("success", false)) and str(after_reject.get("fingerprint", "")) == fingerprint, "counterattacking an unoccupied region is rejected without changing the world")
	var stale: Dictionary = session.realm_military_counterattack(1, 12, true, PackedInt64Array(), "stale-world-fingerprint")
	check(not bool(stale.get("success", false)) and str(session.realm_snapshot(1).get("fingerprint", "")) == fingerprint, "a stale UI snapshot cannot submit a counterattack")

	var text: String = load("res://ui/realm/realm_panel.gd").military_occupation_text({
		"region": 12, "region_name": "Vân Lụa Thành", "partner_id": 1, "garrison_units": 4, "partner_dry_food": 9,
	})
	check(text.contains("Vân Lụa Thành") and text.contains("đối tác B đang giữ 4 quân") and text.contains("còn 9 lương khô"), "the panel explains the held region, partner, garrison and supply stock")
	var raid_text: String = load("res://ui/realm/realm_panel.gd").military_raid_text({
		"target_region": 12, "target_region_name": "Vân Lụa Thành", "partner_id": 0, "force_units": 3, "status": 0,
	})
	check(raid_text.contains("Vân Lụa Thành"), "multi-region raid text names its selected target instead of assuming Nam Cang")

	# Prepare the battle only through public Realm candidates and commands. One
	# defender is lost to an eight-unit raid; the remaining named volunteers can
	# then test the bound counterattack command against the saved occupation.
	var governor_id := 10 # Atlas region 8's seeded local Governance actor.
	var volunteers: Array[int] = []
	var setup: Dictionary = session.realm_snapshot(governor_id)
	while volunteers.size() < 10:
		var service_offer := find_legal_action(setup, "Đề nghị phục vụ")
		if service_offer.is_empty():
			break
		var volunteer_id := int(service_offer.get("target", 0))
		if volunteer_id <= 0 or volunteers.has(volunteer_id):
			break
		if not session.realm_command(governor_id, int(service_offer.index), str(setup.fingerprint)):
			break
		volunteers.append(volunteer_id)
		setup = session.realm_snapshot(governor_id)
	check(volunteers.size() == 10, "ten local volunteers can be recruited through normal candidate offers for the integrated battle fixture")
	if volunteers.size() == 10:
		check(session.realm_resume() and session.realm_step(10) and session.realm_pause(), "ten game days deliver the ordinary service offers")
		for volunteer_id in volunteers:
			var volunteer_snapshot: Dictionary = session.realm_snapshot(volunteer_id)
			var accept := find_legal_action(volunteer_snapshot, "Chấp thuận")
			if accept.is_empty() or not session.realm_command(volunteer_id, int(accept.index), str(volunteer_snapshot.fingerprint)):
				failed = true
				push_error("MILITARY OCCUPATION UI FAIL: a volunteer could not accept the ordinary service offer")
				break
			checks += 1
		check(session.realm_resume() and session.realm_step(1) and session.realm_pause(), "one more day activates volunteer service")
		for volunteer_id in volunteers:
			var consent_snapshot: Dictionary = session.realm_snapshot(volunteer_id)
			var consent_result: bool = session.realm_military_combat_consent(volunteer_id, true, str(consent_snapshot.fingerprint))
			check(consent_result, "each volunteer separately consents to combat through the native command")
		var muster_snapshot: Dictionary = session.realm_snapshot(governor_id)
		var mobilize := find_legal_action(muster_snapshot, "Huy động")
		check(not mobilize.is_empty() and session.realm_command(governor_id, int(mobilize.index), str(muster_snapshot.fingerprint)), "the local Governor mobilizes the consenting roster through the ordinary action")
		var raid: Dictionary = session.realm_military_raid(0, 8, str(session.realm_snapshot(1).fingerprint))
		check(bool(raid.get("success", false)), "the external partner starts an eight-unit raid using its existing finite ledgers")
		if bool(raid.get("success", false)):
			var pending_snapshot: Dictionary = session.realm_snapshot(1)
			var ready_by_region: Dictionary = pending_snapshot.get("military_battle_candidates_by_region", {})
			var battle_candidates: Array = ready_by_region.get(8, [])
			var first_defender := PackedInt64Array()
			if not battle_candidates.is_empty(): first_defender.append(int(battle_candidates[0].get("id", 0)))
			var pending_raids: Array = pending_snapshot.get("military_raids", [])
			var raid_id := int(pending_raids.back().get("id", 0)) if not pending_raids.is_empty() else 0
			var battle: Dictionary = session.realm_military_battle(1, raid_id, false, first_defender, str(pending_snapshot.fingerprint))
			check(not first_defender.is_empty() and bool(battle.get("success", false)), "the selected volunteers resolve the raid through the same C++ battle command")
			if bool(battle.get("success", false)):
				var captured: Dictionary = session.realm_snapshot(1)
				var occupations: Array = captured.get("military_occupations", [])
				var region_occupation: Dictionary = {}
				for occupation in occupations:
					if int(occupation.get("region", -1)) == 8: region_occupation = occupation
				check(not region_occupation.is_empty() and int(region_occupation.get("garrison_units", 0)) > 0, "a successful raid appears as a real region controller in the next native snapshot")
				var local_view: Dictionary = session.realm_snapshot(governor_id)
				var remote_view: Dictionary = session.realm_snapshot(14)
				var local_occupations: Array = local_view.get("military_occupations", [])
				check(local_occupations.size() == 1 and not local_occupations[0].has("partner_dry_food"), "the occupied region can see its garrison without seeing the partner's wider food ledger")
				check(remote_view.get("military_occupations", []).is_empty(), "a person in another region does not receive the occupied region's controller details")
				var panel: Control = load("res://ui/realm/realm_panel.gd").new()
				panel.session = session
				root.add_child(panel)
				await process_frame
				panel.actor = 1
				panel.refresh()
				var counterattack_button := find_button_with_prefix(panel, "Phản công tại Nam Cảng")
				check(counterattack_button != null and not counterattack_button.disabled, "the visible Realm panel offers counterattack with the surviving volunteers at the held region")
				if counterattack_button != null and not counterattack_button.disabled:
					counterattack_button.pressed.emit()
					var recovered: Dictionary = session.realm_snapshot(1)
					var remaining_occupations: Array = recovered.get("military_occupations", [])
					var still_held := false
					for occupation in remaining_occupations:
						if int(occupation.get("region", -1)) == 8: still_held = true
					check(not still_held and bool(recovered.get("invariant", false)), "clicking the panel button orders the C++ counterattack and the later snapshot shows Realm control restored")

	if failed:
		print("MILITARY OCCUPATION GODOT SMOKE: FAILED (", checks, " checks)")
		quit(1)
	else:
		print("MILITARY OCCUPATION GODOT SMOKE: ", checks, "/", checks, " checks passed")
		quit(0)
