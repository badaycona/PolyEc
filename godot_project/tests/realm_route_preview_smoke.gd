extends SceneTree

var checks := 0
var failed := false

func check(ok: bool, message: String) -> void:
	checks += 1
	if not ok:
		failed = true
		push_error("ROUTE PREVIEW FAIL: " + message)

func _initialize() -> void:
	call_deferred("run")

func run() -> void:
	var session = ClassDB.instantiate("PolyEcSession")
	check(session != null and session.has_method("realm_route_preview"), "GDExtension exposes the read-only Realm route query")
	if session == null or not session.has_method("realm_route_preview"):
		quit(1)
		return
	check(session.realm_atlas_manual_start(20261038, 512), "current Atlas27 Realm starts through C++")
	var panel_script = load("res://ui/realm/realm_panel.gd")
	var panel = panel_script.new()
	panel.session = session
	root.add_child(panel)
	await process_frame
	check(panel.route_destination != null and panel.route_destination.item_count == 27, "Realm panel lists all 27 Atlas destinations")
	var snapshot: Dictionary = session.realm_snapshot(1)
	var actor_region := int(snapshot.person.region)
	var destination := (actor_region + 1) % 27
	var before := str(snapshot.fingerprint)
	for target in range(27):
		if target == actor_region:
			continue
		var route: Dictionary = session.realm_route_preview(1, target)
		var route_points: Array = route.get("regions", [])
		check(bool(route.get("supported", false)) and bool(route.get("reachable", false)), "C++ preview finds a route from the current actor to Atlas region %d" % target)
		check(route_points.size() >= 2 and int(route_points.front().id) == actor_region and int(route_points.back().id) == target, "route %d has correct actor and destination endpoints" % target)
	var preview: Dictionary = session.realm_route_preview(1, destination)
	check(bool(preview.get("supported", false)) and bool(preview.get("reachable", false)), "actor's selected destination has a route returned by the C++ planner")
	var points: Array = preview.get("regions", [])
	check(points.size() >= 2 and int(points.front().id) == actor_region and int(points.back().id) == destination, "route names and endpoints match the selected actor and destination")
	check(not preview.has("stock") and not preview.has("shipments") and not preview.has("partner_cash"), "route query does not return private economic ledgers")
	var has_travel_candidate := false
	var has_migrate_candidate := false
	for action in snapshot.get("actions", []):
		if int(action.get("target", -1)) != destination or not bool(action.get("legal", false)):
			continue
		if str(action.get("name", "")) == "Vận tải": has_travel_candidate = true
		if str(action.get("name", "")) == "Di cư": has_migrate_candidate = true
	check(bool(preview.get("travel_available", false)) == has_travel_candidate, "Travel availability matches the current legal C++ candidate")
	check(bool(preview.get("migration_available", false)) == has_migrate_candidate, "Migrate availability matches the current legal C++ candidate")
	if has_migrate_candidate:
		check(int(preview.get("migration_fee", -1)) == 4 + 2 * int(preview.person_days), "Migrate preview fee matches the current Realm formula")
	panel.actor = 1
	panel.route_destination.select(destination)
	panel.refresh()
	check(panel.route_preview_label.text.begins_with("Đường:"), "player-facing Realm panel shows the C++ route result")
	check(str(session.realm_snapshot(1).get("fingerprint", "")) == before, "route preview leaves the Realm fingerprint unchanged")
	var invalid: Dictionary = session.realm_route_preview(0, 0)
	check(not bool(invalid.get("supported", false)), "invalid actor is rejected without indexing person data")
	print("ROUTE PREVIEW %s: %d checks; actor region %d to %d" % ["FAIL" if failed else "PASS", checks, actor_region, destination])
	quit(1 if failed else 0)
