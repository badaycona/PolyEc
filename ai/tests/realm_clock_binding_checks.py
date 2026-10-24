import argparse, re, subprocess, sys
import polyec_env as core

CHECKS = 0

def check(ok, msg):
    global CHECKS
    CHECKS += 1
    if not ok:
        raise AssertionError(msg)

def sessions_canonical(w):
    rows = sorted(w.schedule_sessions(), key=lambda s: (s["actor"], s["id"]))
    out = []
    for s in rows:
        out.append(
            "actor=%d id=%d begin=%d end=%d priority=%d demand_rate=%d requested=%d delivered=%d unmet=%d capacity_unmet=%d unresolved_unmet=%d retired=%d retired_tick=%d retired_cause=%d kind=%d region=%d establishment_id=%d;" % (
                s["actor"], s["id"], s["begin"], s["end"], s["priority"], s["demand_rate"],
                s["requested"], s["delivered"], s["unmet"], s["capacity_unmet"], s["unresolved_unmet"],
                int(s["retired"]), s["retired_tick"], s["retired_cause"], int(s["kind"]), s["region"], s["establishment_id"]))
    return "".join(out)

def parse_checkpoints(output):
    checkpoints = []
    current = None
    for line in output.splitlines():
        if line.startswith("CHECKPOINT "):
            current = {"header": line}
            checkpoints.append(current)
        elif line.startswith("SERIALIZED "):
            current["serialized"] = line[len("SERIALIZED "):]
        elif line.startswith("SESSIONS "):
            current["sessions"] = line[len("SESSIONS "):]
    return checkpoints

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--replay", required=True)
    args = parser.parse_args()

    completed = subprocess.check_output([args.replay], text=True)
    cpp_checkpoints = parse_checkpoints(completed)
    py_checkpoints = []

    # Fixture A
    a = core.Realm(7, 2, 128)
    actor = 1
    body = a.person(actor)
    check(body["sleep_debt"] == 0 and body["exertion"] == 0 and body["rest_until_tick"] == 0,
          "body state read-only defaults")
    body_observation = a.observe(actor)
    check(body_observation[108] == body["sleep_debt"] / 100 and
          body_observation[109] == body["exertion"] / 100,
          "observation slots 108/109 match the actor's own normalized body values")
    rest_world = core.Realm.scenario(7, 129, 30, -1, -1, -1, -1, -1, 100, 50)
    rest_actor = rest_world.person(129)
    ok, reason, _ = rest_world.intent_command(129, rest_actor["decisions"] + 1, 16,
                                               rest_actor["region"], 0, 0, 1)
    check(ok and reason == "Committed" and rest_world.person(129)["rest_until_tick"] == 64,
          "Rest deadline is exposed read-only through Python binding")
    seq = a.person(actor)["decisions"] + 1
    ok, reason, sid1 = a.schedule_command(0, actor, seq, 0, 0, 64, 0, 600)
    check(ok and reason == "Committed", "A submit 1")
    seq = a.person(actor)["decisions"] + 1
    ok, reason, sid2 = a.schedule_command(0, actor, seq, 0, 0, 64, 0, 600)
    check(ok and reason == "Committed", "A submit 2")
    a.advance_ticks(64)
    sessions = a.schedule_sessions(actor)
    check(len(sessions) == 2, "A session count")
    for s in sessions:
        check(s["requested"] == 38400, "A requested")
        check(s["delivered"] == 0, "A delivered")
        check(s["unresolved_unmet"] == 38400, "A unresolved")
    py_checkpoints.append((a.serialize("binding-check"), sessions_canonical(a)))

    # Fixture B
    b = core.Realm(7, 2, 128)
    seq = b.person(actor)["decisions"] + 1
    ok, reason, _ = b.schedule_command(0, actor, seq, 0, 0, 128, 0, 5)
    check(ok and reason == "Committed", "B submit")
    b.advance_ticks(60)
    check(b.game_tick == 60, "B tick 60")
    check(b.view()["day"] == 0, "B day 0")
    bs = b.schedule_sessions(actor)[0]
    check(bs["requested"] == 300 and bs["delivered"] == 300, "B 300")
    py_checkpoints.append((b.serialize("binding-check"), sessions_canonical(b)))
    b.advance_ticks(5)
    check(b.game_tick == 65, "B tick 65")
    check(b.view()["day"] == 1, "B day 1")
    bs = b.schedule_sessions(actor)[0]
    check(bs["requested"] == 325 and bs["delivered"] == 325, "B 325")
    py_checkpoints.append((b.serialize("binding-check"), sessions_canonical(b)))
    saved = b.serialize("binding-check")
    restored = core.Realm(1, 1, 128)
    ok, err = restored.deserialize(saved, "binding-check")
    check(ok and err == "", "B restore")
    check(restored.game_tick == b.game_tick, "B restore tick")
    check(restored.serialize("binding-check") == saved, "B restore serialized")
    py_checkpoints.append((restored.serialize("binding-check"), sessions_canonical(restored)))
    uninterrupted = core.Realm(7, 2, 128)
    seq = uninterrupted.person(actor)["decisions"] + 1
    ok, reason, _ = uninterrupted.schedule_command(0, actor, seq, 0, 0, 128, 0, 5)
    check(ok and reason == "Committed", "B uninterrupted submit")
    uninterrupted.advance_ticks(128)
    b.advance_ticks(63)
    restored.advance_ticks(63)
    check(b.game_tick == 128, "B tick 128")
    check(b.view()["day"] == 2, "B day 2")
    bs = b.schedule_sessions(actor)[0]
    check(bs["requested"] == 640 and bs["delivered"] == 640, "B 640")
    check(b.serialize("binding-check") == restored.serialize("binding-check"), "B continuation")
    check(b.serialize("binding-check") == uninterrupted.serialize("binding-check"), "B one-shot equivalence")
    py_checkpoints.append((b.serialize("binding-check"), sessions_canonical(b)))

    # Readonly clock/schedule state cannot be assigned from Python.
    p = core.Realm(7, 2, 128)
    for attr in ("paused", "pause_revision", "game_tick", "timeline_contract",
                 "pilot_policy_compatible", "model_contract", "schedule_cursor",
                 "schedule_next_session_id", "settled_day"):
        try:
            setattr(p, attr, 1)
            check(False, "readonly assignment rejected: " + attr)
        except AttributeError:
            check(True, "readonly assignment rejected: " + attr)

    # Normal pause, stale rejection, paused exceptions, zero no-op, resume.
    before_pause = p.serialize("binding-check")
    ok, reason = p.pause(0)
    check(ok and reason == "Committed", "normal pause expected 0")
    check(p.paused, "normal pause state")
    check(p.pause_revision == 1, "normal pause revision")
    before_stale = p.serialize("binding-check")
    ok, reason = p.resume(0)
    check(not ok and reason == "Stale clock revision", "stale resume exact reason")
    check(p.serialize("binding-check") == before_stale, "stale resume serialized unchanged")
    before_paused = p.serialize("binding-check")
    for action in (lambda: p.advance(1), lambda: p.advance_ticks(1)):
        try:
            action()
            check(False, "paused advance raises")
        except RuntimeError:
            check(True, "paused advance raises")
        check(p.serialize("binding-check") == before_paused, "paused advance serialized unchanged")
    before_zero = p.game_tick
    before_zero_serialized = p.serialize("binding-check")
    p.advance(0)
    p.advance_ticks(0)
    check(p.game_tick == before_zero, "zero advance no-op tick")
    check(p.serialize("binding-check") == before_zero_serialized, "zero advance no-op serialized")
    ok, reason = p.resume(1)
    check(ok and reason == "Committed", "resume expected 1")
    check(not p.paused, "resume state")

    # Mutating a detached session dictionary does not affect Realm state.
    seq = p.person(actor)["decisions"] + 1
    ok, reason, _ = p.schedule_command(0, actor, seq, 0, 0, 64, 0, 5)
    check(ok and reason == "Committed", "mutation fixture submit")
    before_mutation = p.serialize("binding-check")
    detached = p.schedule_sessions(actor)[0]
    detached["requested"] = 999999
    detached["actor"] = 999
    check(p.serialize("binding-check") == before_mutation, "detached session mutation unchanged")

    # Timeline contract keeps the current candidate surface usable for v11 agents.
    check(p.timeline_contract, "timeline contract true")
    check(not p.pilot_policy_compatible, "pilot compatibility false")
    check(p.model_contract == "realm-worksite-payroll-goods30-config-v42",
          "model contract timeline/worksite/catalog action surface with configured gameplay values")
    observed_after_timeline = p.observe(actor)
    mask_after_timeline = p.mask(actor)
    check(len(observed_after_timeline) == core.REALM_OBS, "observation available after timeline contract")
    check(len(mask_after_timeline) == core.REALM_ACTIONS, "candidate mask available after timeline contract")

    # Fixture C
    c = core.Realm(7, 2, 128)
    c_actor = 1
    other_actor = 2
    seq = c.person(c_actor)["decisions"] + 1
    ok, reason, c_session = c.schedule_command(0, c_actor, seq, 0, 0, 64, 0, 5)
    check(ok and reason == "Committed", "C submit")
    seq = c.person(c_actor)["decisions"] + 1
    ok, reason, _ = c.schedule_command(1, c_actor, seq, c_session, 0, 0, 0, 0)
    check(ok and reason == "Committed", "C cancel")
    check(len(c.schedule_sessions(c_actor)) == 0, "C cancel removed")
    seq = c.person(c_actor)["decisions"] + 1
    ok, reason, c_session2 = c.schedule_command(0, c_actor, seq, 0, 0, 64, 0, 5)
    check(ok and reason == "Committed", "C submit 2")
    seq = c.person(c_actor)["decisions"] + 1
    ok, reason, _ = c.schedule_command(2, c_actor, seq, c_session2, 0, 0, 7, 5)
    check(ok and reason == "Committed", "C reprioritize")
    check(c.schedule_sessions(c_actor)[0]["priority"] == 7, "C priority")
    # wrong owner
    before = c.serialize("binding-check")
    seq = c.person(other_actor)["decisions"] + 1
    ok, reason, _ = c.schedule_command(1, other_actor, seq, c_session2, 0, 0, 0, 0)
    check(not ok and reason == "Unknown session", "C wrong owner")
    check(c.serialize("binding-check") == before, "C wrong owner unchanged")
    # stale
    before = c.serialize("binding-check")
    seq = c.person(c_actor)["decisions"]
    ok, reason, _ = c.schedule_command(0, c_actor, seq, 0, 64, 128, 0, 5)
    check(not ok and reason == "Stale or duplicate decision ID", "C stale")
    check(c.serialize("binding-check") == before, "C stale unchanged")
    # unsupported operation
    before = c.serialize("binding-check")
    seq = c.person(c_actor)["decisions"] + 1
    ok, reason, _ = c.schedule_command(9, c_actor, seq, c_session2, 64, 128, 0, 5)
    check(not ok and reason == "Unsupported schedule operation", "C bad operation")
    check(c.serialize("binding-check") == before, "C bad operation unchanged")
    # invalid range
    before = c.serialize("binding-check")
    seq = c.person(c_actor)["decisions"] + 1
    ok, reason, _ = c.schedule_command(0, c_actor, seq, 0, 64, 64, 0, 5)
    check(not ok and reason == "Invalid session range", "C bad range")
    check(c.serialize("binding-check") == before, "C bad range unchanged")
    # invalid rate
    before = c.serialize("binding-check")
    seq = c.person(c_actor)["decisions"] + 1
    ok, reason, _ = c.schedule_command(0, c_actor, seq, 0, 64, 128, 0, 0)
    check(not ok and reason == "Invalid demand rate", "C bad rate")
    check(c.serialize("binding-check") == before, "C bad rate unchanged")
    py_checkpoints.append((c.serialize("binding-check"), sessions_canonical(c)))

    # Fixture D overflow via serialized fixtures.
    base = core.Realm(7, 2, 128).serialize("binding-check")
    max_revision = str(2**64 - 1)
    paused_fixture = re.sub(r"( CLOCK_1 )0 [0-9]+ ", r"\g<1>1 " + max_revision + " ", base, count=1)
    unpaused_fixture = re.sub(r"( CLOCK_1 )0 [0-9]+ ", r"\g<1>0 " + max_revision + " ", base, count=1)
    check(paused_fixture != base and unpaused_fixture != base, "D fixtures changed")
    paused_max = core.Realm(1, 1, 128)
    ok, err = paused_max.deserialize(paused_fixture, "binding-check")
    check(ok and err == "", "D paused restore")
    check(paused_max.paused, "D paused max state")
    check(paused_max.pause_revision == 2**64 - 1, "D paused max revision")
    before_paused_overflow = paused_max.serialize("binding-check")
    ok, reason = paused_max.pause()
    check(ok and reason == "Committed", "D paused pause idempotent")
    check(paused_max.serialize("binding-check") == before_paused_overflow, "D paused pause serialized unchanged")
    check(paused_max.pause_revision == 2**64 - 1, "D paused pause revision")
    before_paused_resume = paused_max.serialize("binding-check")
    ok, reason = paused_max.resume()
    check(not ok and reason == "Pause revision overflow", "D paused resume")
    check(paused_max.serialize("binding-check") == before_paused_resume, "D paused resume serialized unchanged")
    check(paused_max.paused, "D paused resume leaves paused")
    unpaused_max = core.Realm(1, 1, 128)
    ok, err = unpaused_max.deserialize(unpaused_fixture, "binding-check")
    check(ok and err == "", "D unpaused restore")
    check(not unpaused_max.paused, "D unpaused max state")
    check(unpaused_max.pause_revision == 2**64 - 1, "D unpaused max revision")
    before_unpaused_pause = unpaused_max.serialize("binding-check")
    ok, reason = unpaused_max.pause()
    check(not ok and reason == "Pause revision overflow", "D unpaused pause")
    check(unpaused_max.serialize("binding-check") == before_unpaused_pause, "D unpaused pause serialized unchanged")
    check(not unpaused_max.paused, "D unpaused pause leaves unpaused")
    before_unpaused_resume = unpaused_max.serialize("binding-check")
    ok, reason = unpaused_max.resume()
    check(ok and reason == "Committed", "D unpaused resume idempotent")
    check(unpaused_max.serialize("binding-check") == before_unpaused_resume, "D unpaused resume serialized unchanged")
    check(unpaused_max.pause_revision == 2**64 - 1, "D unpaused resume revision")
    py_checkpoints.append((paused_max.serialize("binding-check"), sessions_canonical(paused_max)))

    # Fixture E: typed Work sessions, union attendance, mid-day continuation.
    # Existing eight-argument calls stay valid and must remain Generic.
    e = core.Realm(7, 2, 128)
    e_region = e.person(actor)["region"]
    other_region = (e_region + 1) % 3
    check(other_region != e_region, "E alternate region")
    est_owner = actor if e_region == 2 else e_region + 2
    ok, reason, e_site = e.establishment_command(est_owner, e.person(est_owner)["decisions"] + 1,
                                                 1, e_region, e_region, 1, 1, 20)
    check(ok and reason == "Committed" and e_site > 0, "E work establishment")
    seq = e.person(actor)["decisions"] + 1
    ok, reason, e_generic = e.schedule_command(0, actor, seq, 0, 0, 160, 0, 5)
    check(ok and reason == "Committed", "E legacy generic submit")
    seq = e.person(actor)["decisions"] + 1
    ok, reason, _ = e.schedule_command(0, actor, seq, 0, 96, 128, 0, 5, 1, other_region, e_site)
    check(not ok and reason == "Work establishment region mismatch", "E foreign-region work rejected")
    seq = e.person(actor)["decisions"] + 1
    ok, reason, e_work1 = e.schedule_command(0, actor, seq, 0, 0, 64, 0, 5, 1, e_region, e_site)
    check(ok and reason == "Committed", "E work 1 submit")
    seq = e.person(actor)["decisions"] + 1
    ok, reason, e_work2 = e.schedule_command(0, actor, seq, 0, 32, 96, 0, 5, 1, e_region, e_site)
    check(ok and reason == "Committed", "E overlap work 2 submit")
    typed = {s["id"]: s for s in e.schedule_sessions(actor)}
    check(typed[e_generic]["kind"] == 0 and typed[e_generic]["region"] == -1, "E generic typing")
    check(typed[e_work1]["kind"] == 1 and typed[e_work1]["region"] == e_region, "E work 1 typing")
    check(typed[e_work2]["kind"] == 1 and typed[e_work2]["region"] == e_region, "E work 2 typing")
    check(typed[e_work1]["establishment_id"] == e_site and typed[e_work2]["establishment_id"] == e_site, "E work site binding")
    # Invalid typed fields reject atomically before any state is committed.
    before_typed_reject = e.serialize("binding-check")
    for kind, region, expected in ((2, e_region, "Invalid activity kind"),
                                   (1, -1, "Invalid work region"),
                                   (1, 3, "Invalid work region"),
                                   (0, e_region, "Invalid generic region")):
        seq = e.person(actor)["decisions"] + 1
        ok, reason, _ = e.schedule_command(0, actor, seq, 0, 0, 64, 0, 5, kind, region)
        check(not ok and reason == expected, "E typed reject %d/%d" % (kind, region))
    check(e.serialize("binding-check") == before_typed_reject, "E typed rejects unchanged")
    # Physical presence credits the interval union once, not the overlap sum.
    e.advance_ticks(40)
    check(e.game_tick == 40, "E tick 40")
    check(e.work_present_ticks(actor) == 40, "E attendance 40 union")
    check(e.work_present_ticks(other_actor) == 0, "E other actor attendance zero")
    check(e.work_wage_per_unit == 1, "E wage unit")
    check(e.work_present_ticks_by_region(actor, e_region) == 40, "E regional attendance 40 union")
    check(e.work_present_ticks_by_region(actor, other_region) == 0, "E foreign region attendance zero")
    check(e.work_present_ticks_by_establishment(actor, e_site) == 40, "E site attendance 40 union")
    check(e.paid_work_units_by_region(actor, e_region) == 0, "E no mid-day regional pay")
    check(e.settled_day == 0 and e.settled_work_units(actor) == 0, "E no mid-day settlement")
    # Mid-day checkpoint: restore resumes without losing or repeating credit.
    e_saved = e.serialize("binding-check")
    e_restored = core.Realm(1, 1, 128)
    ok, err = e_restored.deserialize(e_saved, "binding-check")
    check(ok and err == "", "E restore")
    check(e_restored.serialize("binding-check") == e_saved, "E restore serialized")
    check(e_restored.work_present_ticks(actor) == 40, "E restore attendance")
    check(e_restored.settled_day == 0 and e_restored.settled_work_units(actor) == 0, "E restore no settlement")
    e.advance_ticks(120)
    e_restored.advance_ticks(120)
    check(e.game_tick == 160 and e_restored.game_tick == 160, "E tick 160")
    e_body = e.person(actor)
    e_observation = e.observe(actor)
    check(e_body["sleep_debt"] > 0 and
          abs(e_observation[108] - e_body["sleep_debt"] / 100) < 1e-7 and
          abs(e_observation[109] - e_body["exertion"] / 100) < 1e-7,
          "non-default body values are projected to matching Python observation slots")
    check(e.work_present_ticks(actor) == 96, "E attendance 96")
    check(e_restored.work_present_ticks(actor) == 96, "E restored attendance 96")
    check(e.work_present_ticks_by_region(actor, e_region) == 96, "E regional attendance 96")
    check(e_restored.work_present_ticks_by_region(actor, e_region) == 96, "E restored regional attendance 96")
    check(e.establishments()[0]["paid_work_units"].get(actor, 0) == 6, "E site paid 6")
    check(e_restored.establishments()[0]["paid_work_units"].get(actor, 0) == 6, "E restored site paid 6")
    check(e.paid_work_units_by_region(actor, other_region) == 0, "E foreign region paid zero")
    check(e.settled_day == 2 and e.settled_work_units(actor) == 6, "E settlement day 2 units 6")
    check(e_restored.settled_day == 2 and e_restored.settled_work_units(actor) == 6, "E restored settlement")
    check(e.serialize("binding-check") == e_restored.serialize("binding-check"), "E continuation")
    # The same fixture advanced one-shot must serialise identically.
    e_one_shot = core.Realm(7, 2, 128)
    ok, reason, one_shot_site = e_one_shot.establishment_command(est_owner, e_one_shot.person(est_owner)["decisions"] + 1,
                                                                  1, e_region, e_region, 1, 1, 20)
    check(ok and reason == "Committed" and one_shot_site == e_site, "E one-shot work establishment")
    seq = e_one_shot.person(actor)["decisions"] + 1
    ok, reason, _ = e_one_shot.schedule_command(0, actor, seq, 0, 0, 160, 0, 5)
    check(ok and reason == "Committed", "E one-shot generic submit")
    seq = e_one_shot.person(actor)["decisions"] + 1
    ok, reason, _ = e_one_shot.schedule_command(0, actor, seq, 0, 0, 64, 0, 5, 1, e_region, e_site)
    check(ok and reason == "Committed", "E one-shot work 1 submit")
    seq = e_one_shot.person(actor)["decisions"] + 1
    ok, reason, _ = e_one_shot.schedule_command(0, actor, seq, 0, 32, 96, 0, 5, 1, e_region, e_site)
    check(ok and reason == "Committed", "E one-shot work 2 submit")
    e_one_shot.advance_ticks(160)
    check(e_one_shot.work_present_ticks(actor) == 96, "E one-shot attendance 96")
    check(e_one_shot.work_present_ticks_by_region(actor, e_region) == 96, "E one-shot regional attendance 96")
    check(e_one_shot.work_present_ticks_by_establishment(actor, e_site) == 96, "E one-shot site attendance 96")
    check(e_one_shot.establishments()[0]["paid_work_units"].get(actor, 0) == 6, "E one-shot site paid 6")
    check(e_one_shot.serialize("binding-check") == e.serialize("binding-check"), "E one-shot equivalence")
    # A Generic-only timeline never earns Work attendance.
    e_generic_only = core.Realm(7, 2, 128)
    seq = e_generic_only.person(actor)["decisions"] + 1
    ok, reason, _ = e_generic_only.schedule_command(0, actor, seq, 0, 0, 64, 0, 5)
    check(ok and reason == "Committed", "E generic-only submit")
    e_generic_only.advance_ticks(64)
    check(e_generic_only.work_present_ticks(actor) == 0, "E generic-only attendance zero")
    check(e_generic_only.work_present_ticks_by_region(actor, e_region) == 0, "E generic-only regional attendance zero")
    check(e_generic_only.paid_work_units_by_region(actor, e_region) == 0, "E generic-only regional paid zero")
    check(e_generic_only.settled_day == 1 and e_generic_only.settled_work_units(actor) == 0, "E generic-only no units")

    # Compare exact serialized state and sorted session fields to C++.
    check(len(cpp_checkpoints) == len(py_checkpoints), "checkpoint count")
    for i, (cpp, py) in enumerate(zip(cpp_checkpoints, py_checkpoints)):
        check(cpp["serialized"] == py[0], "serialized mismatch at %d" % i)
        check(cpp["sessions"] == py[1], "sessions mismatch at %d" % i)
        check(py[0].startswith("POLYEC_REALM_67 "), "serialized current schema 67 magic at %d" % i)
        check(" SCHEDULE_7 " in py[0], "serialized current schedule tag at %d" % i)

    print("REALM CLOCK BINDING CHECKS: %d Python checks passed" % CHECKS)
    print("REALM CLOCK REPLAY: %d checkpoints matched" % len(py_checkpoints))

if __name__ == "__main__":
    main()
