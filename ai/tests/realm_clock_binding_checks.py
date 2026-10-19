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
            "actor=%d id=%d begin=%d end=%d priority=%d demand_rate=%d requested=%d delivered=%d unmet=%d capacity_unmet=%d unresolved_unmet=%d retired=%d retired_tick=%d retired_cause=%d;" % (
                s["actor"], s["id"], s["begin"], s["end"], s["priority"], s["demand_rate"],
                s["requested"], s["delivered"], s["unmet"], s["capacity_unmet"], s["unresolved_unmet"],
                int(s["retired"]), s["retired_tick"], s["retired_cause"]))
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
                 "schedule_next_session_id"):
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

    # Timeline contract makes old pilot legacy observation/mask pathways reject.
    check(p.timeline_contract, "timeline contract true")
    check(not p.pilot_policy_compatible, "pilot compatibility false")
    check(p.model_contract == "realm-timeline-v1", "model contract timeline")
    for legacy in (lambda: p.observe(actor), lambda: p.mask(actor)):
        try:
            legacy()
            check(False, "legacy pathway rejected")
        except RuntimeError:
            check(True, "legacy pathway rejected")

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

    # Compare exact serialized state and sorted session fields to C++.
    check(len(cpp_checkpoints) == len(py_checkpoints), "checkpoint count")
    for i, (cpp, py) in enumerate(zip(cpp_checkpoints, py_checkpoints)):
        check(cpp["serialized"] == py[0], "serialized mismatch at %d" % i)
        check(cpp["sessions"] == py[1], "sessions mismatch at %d" % i)

    print("REALM CLOCK BINDING CHECKS: %d Python checks passed" % CHECKS)
    print("REALM CLOCK REPLAY: %d checkpoints matched" % len(py_checkpoints))

if __name__ == "__main__":
    main()
