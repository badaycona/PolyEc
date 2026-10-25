extends SceneTree

var checks := 0
var failed := false

func check(ok: bool, message: String) -> void:
	checks += 1
	if not ok:
		failed = true
		push_error("APPRENTICESHIP GODOT FAIL: " + message)

func find_action(snapshot: Dictionary, exact_name: String, require_legal := true) -> Dictionary:
	for action in snapshot.get("actions", []):
		if str(action.get("name", "")) == exact_name and (not require_legal or bool(action.get("legal", false))):
			return action
	return {}

func find_apprenticeship_offer(snapshot: Dictionary) -> Dictionary:
	for action in snapshot.get("actions", []):
		if str(action.get("name", "")).begins_with("Mời học nghề cho người #") and bool(action.get("legal", false)):
			return action
	return {}

func _initialize() -> void:
	call_deferred("run")

func run() -> void:
	var session = ClassDB.instantiate("PolyEcSession")
	check(session.realm_schema() == "realm-v43", "native extension reports the v2 action contract")
	check(session.realm_model_contract() == "realm-household-fund-goods30-config-v54", "native extension reports config v54")
	check(session.realm_atlas_manual_start(444438, 512), "deterministic Atlas fixture starts without an old policy")
	if failed:
		quit(1)
		return

	var mentor_id := 0
	var mentor_skill := -1
	for id in range(1, 513):
		var person: Dictionary = session.realm_snapshot(id).get("person", {})
		if int(person.get("role", -1)) == 2 and int(person.get("age", 0)) >= 18 and int(person.get("age", 0)) < 65 and int(person.get("skill", 0)) > mentor_skill:
			mentor_id = id
			mentor_skill = int(person.skill)
	check(mentor_id > 0 and mentor_skill == 34, "seed selects an adult Civilian who starts just below the mentor requirement")
	var mentor: Dictionary = session.realm_snapshot(mentor_id)
	var workshop_action: Dictionary = {}
	for action in mentor.get("actions", []):
		if action.get("name", "") == "Tạo cơ sở sản xuất" and int(action.get("good", 0)) == 1 and bool(action.get("legal", false)):
			workshop_action = action
			break
	check(not workshop_action.is_empty(), "the future mentor can fund a private workshop before the study period")
	if workshop_action.is_empty():
		quit(1)
		return
	check(session.realm_command(mentor_id, int(workshop_action.index), str(mentor.fingerprint)), "player workshop command succeeds through the native action endpoint")

	mentor = session.realm_snapshot(mentor_id)
	var study := find_action(mentor, "Học")
	check(not study.is_empty() and session.realm_command(mentor_id, int(study.index), str(mentor.fingerprint)), "mentor raises skill through the existing Study command")
	check(session.realm_resume(), "manual fixture resumes for the scheduled study days")
	check(session.realm_step(30), "30 game days complete the Study action")
	mentor = session.realm_snapshot(mentor_id)
	check(int(mentor.person.skill) >= 40, "mentor now meets the skill requirement")
	check(int(mentor.person.cash) < 25, "the fixture reproduces low personal cash after study and survival costs")

	var offer := find_apprenticeship_offer(mentor)
	check(not offer.is_empty(), "a learner with available funds can receive a legal v2 offer after the 30-day study period")
	if offer.is_empty():
		quit(1)
		return
	check(int(offer.amount) == 10 and int(offer.term) == 30, "the visible offer carries the v2 tuition and course duration")
	var site_id := int(offer.good)
	var workshop_cash_before := 0
	for site in mentor.get("establishments", []):
		if int(site.get("id", 0)) == site_id:
			workshop_cash_before = int(site.get("cash", 0))
	check(workshop_cash_before == 25, "the offer names the funded workshop owned by the mentor")
	check(session.realm_command(mentor_id, int(offer.index), str(mentor.fingerprint)), "player sends the apprenticeship invitation through the native action endpoint")

	var learner_id := int(offer.target)
	var learner: Dictionary = session.realm_snapshot(learner_id)
	var accept := find_action(learner, "Chấp thuận")
	check(not accept.is_empty(), "learner receives the ordinary Accept action for the pending invitation")
	if accept.is_empty():
		quit(1)
		return
	check(session.realm_command(learner_id, int(accept.index), str(learner.fingerprint)), "learner accepts through the same native action endpoint")

	learner = session.realm_snapshot(learner_id)
	var active_course := false
	for course in learner.get("apprenticeships", []):
		if int(course.get("mentor", 0)) == mentor_id and int(course.get("apprentice", 0)) == learner_id and int(course.get("establishment_id", 0)) == site_id and int(course.get("status", -1)) == 0 and int(course.get("tuition", 0)) == 10:
			active_course = true
	check(active_course, "native snapshot shows the accepted active course, exact participants, workshop and tuition")
	var mentor_after: Dictionary = session.realm_snapshot(mentor_id)
	for site in mentor_after.get("establishments", []):
		if int(site.get("id", 0)) == site_id:
			check(int(site.get("cash", 0)) == workshop_cash_before + 10, "workshop ledger received the tuition exactly once")
			break

	if failed:
		print("APPRENTICESHIP GODOT ACTION SMOKE: FAILED (", checks, " checks)")
		quit(1)
	else:
		print("APPRENTICESHIP GODOT ACTION SMOKE: ", checks, "/", checks, " checks passed")
		quit(0)
