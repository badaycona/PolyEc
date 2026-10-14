extends SceneTree
var checks=0
func check(ok: bool,message: String) -> void:
    checks+=1
    if not ok:
        push_error("REALM FAIL: "+message)
        quit(1)

func _initialize() -> void:
    call_deferred("run")

func run() -> void:
    var session=ClassDB.instantiate("PolyEcSession")
    var paths=[]
    var hashes=[]
    for family in ["governance","economic","civilian"]:
        var base=ProjectSettings.globalize_path("res://models/realm-v1/"+family+"/")
        var manifest=JSON.parse_string(FileAccess.get_file_as_string(base+"manifest.json"))
        paths.append(base+family+".onnx")
        hashes.append(manifest.sha256)
    check(session.realm_load(paths,hashes,37,10000),"three native models load")
    var v=session.realm_snapshot(1)
    check(v.persons==10000,"individual count")
    check(v.edges>19000,"real sparse relations")
    check(v.invariant,"opening accounting")
    check(not session.realm_step(1),"draft required")
    check(session.realm_draft(0,true),"reroll")
    check(not session.realm_draft(1,true),"only one reroll")
    v=session.realm_snapshot(1)
    check(session.realm_draft(v.cards[0],false),"choose")
    check(not session.realm_draft(v.cards[0],false),"apply once")
    var money=session.realm_snapshot(1).money
    session.set_active(true)
    for day in range(35):
        check(session.realm_step(1),"learned day "+str(day))
        v=session.realm_snapshot(1)
        check(v.invariant and v.money==money,"funded population decisions")
    check(v.inferences>10000,"actual per-person inference, not empty records")
    var path=ProjectSettings.globalize_path("user://realm-smoke.polyec")
    check(session.realm_save(path,false),"save world and policy memory")
    var saved=session.realm_snapshot(1).fingerprint
    check(session.realm_step(1),"advance after save")
    var next=session.realm_snapshot(1).fingerprint
    check(next!=saved,"world advances")
    check(session.realm_save(path,true),"load exact")
    check(session.realm_snapshot(1).fingerprint==saved,"restore world")
    check(session.realm_step(1),"resume restored")
    check(session.realm_snapshot(1).fingerprint==next,"same sampled actions after restore")
    check(session.realm_save(path,false),"overwrite save keeps backup")
    var file=FileAccess.open(path,FileAccess.WRITE)
    file.store_string("corrupt")
    file.close()
    var stable=session.realm_snapshot(1).fingerprint
    check(not session.realm_save(path,true),"corrupt save rejected")
    check(session.realm_snapshot(1).fingerprint==stable,"failed load atomic")
    session.set_active(false)
    check(not session.realm_step(1),"inactive no time progress")
    session.set_active(true)
    check(not session.realm_command(0,1),"invalid player actor")
    var app=load("res://main.tscn").instantiate()
    root.add_child(app)
    await process_frame
    app._open_realm()
    await process_frame
    check(app.realm_panel.visible,"realm panel opens")
    app.realm_panel.running=true
    app.realm_panel._notification(Control.NOTIFICATION_APPLICATION_FOCUS_OUT)
    check(not app.realm_panel.running,"focus loss stops loop")
    app.realm_panel.show_guide()
    check(not app.realm_panel.running,"guide pauses")
    app.queue_free()
    await process_frame
    print("REALM SMOKE: ",checks,"/",checks," checks passed")
    quit(0)
