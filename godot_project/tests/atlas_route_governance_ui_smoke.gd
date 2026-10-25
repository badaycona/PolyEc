extends SceneTree

var checks := 0
var failed := false

func check(ok: bool, message: String) -> void:
	checks += 1
	if not ok:
		failed = true
		push_error("ATLAS ROUTE GOVERNANCE FAIL: " + message)

func click_route_control(panel: Control, route_index: int) -> bool:
	var selector: OptionButton = panel.commands.get_node_or_null("NationalRouteSelector")
	var action: Button = panel.commands.get_node_or_null("NationalRouteAction")
	if selector == null or action == null:
		return false
	var item_index := selector.get_item_index(route_index)
	if item_index < 0:
		return false
	selector.select(item_index)
	selector.item_selected.emit(item_index)
	if action.disabled:
		return false
	action.pressed.emit()
	return true

func route_from_snapshot(snapshot: Dictionary, index: int) -> Dictionary:
	for route in snapshot.get("national_routes", []):
		if int(route.get("index", -1)) == index:
			return route
	return {}

func _initialize() -> void:
	call_deferred("run")

func run() -> void:
	var session = ClassDB.instantiate("PolyEcSession")
	check(session != null and session.has_method("realm_national_transport_route"), "Session exposes the Sovereign route command")
	if session == null or not session.has_method("realm_national_transport_route"):
		quit(1)
		return
	check(session.realm_atlas_manual_start(20261038, 512), "Atlas27 Realm starts through the current C++ core")
	var panel_script = load("res://ui/realm/realm_panel.gd")
	var panel = panel_script.new()
	panel.session = session
	root.add_child(panel)
	await process_frame
	panel.refresh()
	var initial: Dictionary = session.realm_snapshot(1)
	var routes: Array = initial.get("national_routes", [])
	var origin := int(initial.person.region)
	var destination := (origin + 1) % 27
	check(routes.size() > 27, "Sovereign snapshot lists directional national routes")
	check(not session.realm_snapshot(2).has("national_routes"), "a non-Sovereign snapshot does not receive route-control data")
	check(panel.commands.get_node_or_null("NationalRouteSelector") != null, "Sovereign sees the route selector in Realm panel")
	var outgoing: Array[int] = []
	for route in routes:
		if int(route.get("from", -1)) == origin and bool(route.get("open", false)):
			outgoing.append(int(route.get("index", -1)))
	check(not outgoing.is_empty(), "fixture has at least one open route leaving Sovereign region")
	var original_fingerprint := str(initial.get("fingerprint", ""))
	for route_index in outgoing:
		check(click_route_control(panel, route_index), "panel closes outgoing route %d through C++ command" % route_index)
		var after_close: Dictionary = route_from_snapshot(session.realm_snapshot(1), route_index)
		check(not bool(after_close.get("open", true)), "route %d is now closed" % route_index)
	var blocked: Dictionary = session.realm_route_preview(1, destination)
	check(not bool(blocked.get("reachable", true)), "preview reports unreachable after every outgoing route is closed")
	var stable := str(session.realm_snapshot(1).get("fingerprint", ""))
	var stale: Dictionary = session.realm_national_transport_route(1, outgoing[0], true, original_fingerprint)
	check(not bool(stale.get("success", false)) and str(session.realm_snapshot(1).fingerprint) == stable, "stale route request is rejected without changing Realm")
	var unauthorized: Dictionary = session.realm_national_transport_route(2, outgoing[0], true, stable)
	check(not bool(unauthorized.get("success", false)) and str(session.realm_snapshot(1).fingerprint) == stable, "non-Sovereign route request is rejected atomically")
	check(click_route_control(panel, outgoing[0]), "panel reopens a selected national route")
	var reopened: Dictionary = route_from_snapshot(session.realm_snapshot(1), outgoing[0])
	check(bool(reopened.get("open", false)), "reopened route state appears in refreshed snapshot")
	check(bool(session.realm_route_preview(1, destination).get("reachable", false)), "route preview becomes reachable after reopening an outgoing edge")
	var after_open_fingerprint := str(session.realm_snapshot(1).fingerprint)
	var redundant: Dictionary = session.realm_national_transport_route(1, outgoing[0], true, after_open_fingerprint)
	check(not bool(redundant.get("success", false)) and str(session.realm_snapshot(1).fingerprint) == after_open_fingerprint, "redundant open command is rejected without mutation")
	var save_path := ProjectSettings.globalize_path("res://route-governance-smoke-save.polyec")
	check(session.realm_save(save_path, false), "save stores route changes made through panel")
	check(click_route_control(panel, outgoing[0]), "panel closes the route after the save")
	check(not bool(route_from_snapshot(session.realm_snapshot(1), outgoing[0]).get("open", true)), "route is closed before restore")
	check(session.realm_save(save_path, true), "interactive restore loads the saved route state")
	panel.refresh()
	check(bool(route_from_snapshot(session.realm_snapshot(1), outgoing[0]).get("open", false)), "save/load restores the reopened route")
	check(bool(session.realm_route_preview(1, destination).get("reachable", false)), "preview follows restored route configuration")
	print("ATLAS ROUTE GOVERNANCE %s: %d checks; region %d -> %d; managed outgoing routes %d" % ["FAIL" if failed else "PASS", checks, origin, destination, outgoing.size()])
	quit(1 if failed else 0)
