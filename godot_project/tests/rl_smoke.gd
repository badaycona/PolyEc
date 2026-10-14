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
    var fixture_path="res://tests/rl_expected.json"
    for argument in OS.get_cmdline_user_args():
        if argument.begins_with("--fixture-file="): fixture_path=argument.trim_prefix("--fixture-file=")
    var fixture=JSON.parse_string(FileAccess.get_file_as_string(fixture_path))
    expect(fixture is Dictionary,"Native replay fixture available")
    if not fixture is Dictionary:
        quit(1)
        return
    var app=load("res://main.tscn").instantiate()
    root.add_child(app)
    await process_frame
    expect(app.session.rl_available(),"Native CPU inference linked")
    app._open_rl()
    await process_frame
    var lab=app.rl_lab
    expect(app.session.rl_snapshot().loaded,"Manifest/hash loads trained model")
    expect(lab.evidence_label.text.length()>20,"Evaluation evidence is actually rendered")
    var table=lab.get_node("Market/Table")
    expect(table.position.y+table.size.y<lab.get_node("Market/Legend").position.y,"Market table does not overlap its legend")
    lab.seed=int(fixture.seed)
    lab.load_policy()
    app.session.set_active(true)
    expect(app.session.rl_snapshot().decision==0,"New seed resets decision and memory")
    for i in range(20): app.session.rl_snapshot()
    expect(app.session.rl_snapshot().inferences==0,"Snapshots do not update recurrent memory")
    for index in range(64):
        lab.step_once()
        var v=app.session.rl_snapshot()
        var expected=fixture.steps[index]
        expect(int(v.decision)==index+1,"Exactly one action per command")
        expect(int(v.inferences)==index+1,"Exactly one recurrent inference per decision")
        expect(int(v.history[-1].action)==int(expected.action),"GDExtension action matches independent C++ ORT")
        expect(str(v.fingerprint)==str(expected.fingerprint),"GDExtension transition matches native replay")
        expect(int(v.cash)==int(expected.cash),"Native cash matches replay")
    expect(app.session.rl_snapshot().done,"Episode terminates after64 decisions")
    var final=app.session.rl_snapshot().fingerprint
    expect(not app.session.rl_step().success,"Finished episode cannot advance")
    expect(app.session.rl_snapshot().fingerprint==final,"Finished rejection is atomic")
    var folder=ProjectSettings.globalize_path("res://models/")
    expect(not app.session.rl_load(folder+"economic.onnx","0".repeat(64),37),"Reject wrong model checksum")
    expect(not app.session.rl_snapshot().loaded,"Wrong checksum stops previous model")
    expect(not app.session.rl_step().success,"No fallback after model failure")
    lab.seed=37
    lab.load_policy()
    app.session.set_active(true)
    lab.toggle_run()
    expect(lab.running,"Automatic replay can start")
    lab._notification(Node.NOTIFICATION_APPLICATION_FOCUS_OUT)
    expect(not lab.running and lab.timer.is_stopped(),"Focus loss stops automatic decision dispatch")
    lab.close_lab()
    expect(not lab.visible and app.session.snapshot().paused,"Closing lab leaves map world paused")
    app.queue_free()
    await process_frame
    print("RL SMOKE: %d/%d checks passed" % [checks-failures,checks])
    quit(0 if failures==0 else 1)
