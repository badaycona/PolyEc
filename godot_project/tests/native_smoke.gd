extends SceneTree

var checks=0
var failures=0
func expect(value: bool,message: String) -> void:
    checks+=1
    if not value:
        failures+=1
        push_error(message)

func _initialize() -> void:
    call_deferred("run")

func run() -> void:
    expect(ClassDB.class_exists("PolyEcSession"),"Native class registered")
    if failures>0:
        quit(1)
        return
    var session: RefCounted=ClassDB.instantiate("PolyEcSession")
    var initial: Dictionary=session.snapshot()
    expect(initial.paused and initial.day==0,"Starts paused on day zero")
    expect(initial.territories.size()==2,"Exactly two core territories")
    expect(initial.territories[0].id==7 and initial.territories[1].id==8,"Stable IDs")
    expect(initial.territories[0].name=="Thanh Điền","Vietnamese names from native core")
    initial.territories[0].name="modified copy"
    expect(session.snapshot().territories[0].name=="Thanh Điền","Snapshot mutation cannot mutate core")
    expect(session.command("select",8).success,"Select valid native ID")
    expect(not session.command("select",900).success,"Reject unknown ID")
    expect(session.snapshot().selected==8,"Rejection preserves selection")
    expect(not session.command("unknown").success,"Reject unknown command")
    expect(not session.set_speed(5.0),"Reject unsupported speed")
    expect(session.set_speed(4.0),"Accept 4x")
    expect(session.command("resume").success,"Resume")
    for i in range(12):
        await create_timer(0.05).timeout
        session.tick()
    var advanced: int=session.snapshot().day
    expect(advanced>=1,"Real monotonic native clock advances")
    session.command("pause")
    await create_timer(0.3).timeout
    session.tick()
    expect(session.snapshot().day==advanced,"Pause does not advance")
    session.set_active(false)
    expect(not session.command("resume").success,"Inactive window cannot resume")
    session.set_active(true)
    expect(session.snapshot().paused,"Returning focus remains paused")
    session.command("resume")
    await create_timer(0.4).timeout
    session.tick()
    expect(session.snapshot().paused,"Long gap auto-pauses")
    expect(session.snapshot().day==advanced,"No catch-up after long gap")
    session=null
    print("NATIVE SMOKE: %d/%d checks passed" % [checks-failures,checks])
    quit(0 if failures==0 else 1)
