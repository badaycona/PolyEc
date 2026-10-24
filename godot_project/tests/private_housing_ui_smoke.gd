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

func find_button(node: Node,text: String) -> Button:
    if node is Button and node.text==text: return node
    for child in node.get_children():
        var found=find_button(child,text)
        if found!=null: return found
    return null

func run() -> void:
    var panel_script=load("res://ui/realm/realm_panel.gd")
    expect(panel_script!=null,"realm panel with private-housing controls parses")
    var session=ClassDB.instantiate("PolyEcSession")
    expect(session.has_method("realm_atlas_manual_start"),"manual Atlas entry point is available")
    expect(session.has_method("realm_private_residence_build") and session.has_method("realm_private_housing_command"),
        "build, offer, accept, decline and end-lease API is available")
    var panel=panel_script.new()
    panel.session=session
    root.add_child(panel)
    await process_frame
    var manual_button=find_button(panel,"Atlas kiểm thử thủ công")
    expect(manual_button!=null,"the Atlas manual-test button is visible even when no compatible model bundle is loaded")
    if manual_button!=null: manual_button.pressed.emit()
    expect(session.realm_snapshot(1).get("atlas_manual_test",false),"the visible Atlas button starts the real 27-region Realm in labeled manual mode")
    expect(panel.status.text.contains("NPC không tự ra quyết định") and panel.region_rows.size()==27,
        "player UI explains the mode and displays all Atlas regions")
    var has_housing_heading=false
    var has_build_button=false
    for control in panel.commands.get_children():
        if control is Label and control.text.contains("NHÀ TƯ"): has_housing_heading=true
        if control is Button and control.text.begins_with("Xây nhà"): has_build_button=true
    expect(has_housing_heading and has_build_button,"player UI provides the private-housing section and build action")
    var view=session.realm_snapshot(1)
    expect(view.get("atlas_manual_test",false),"snapshot labels manual mode")
    expect(view.get("persons",0)==512 and view.get("regions",[]).size()==27,"manual Realm contains its documented 512-person population and all Atlas regions")
    expect(view.get("private_residences",[]).is_empty(),"new Atlas Realm starts with no unearned private homes")
    expect(view.has("land_holdings") and view.has("private_housing_households"),"snapshot exposes scoped title and eligible household choices")
    var fingerprint=str(view.get("fingerprint",""))
    var rejected=session.realm_private_residence_build(1,9223372036854775807,fingerprint)
    expect(not rejected.success,"an unknown title is rejected")
    expect(session.realm_snapshot(1).get("fingerprint","")==fingerprint and session.realm_snapshot(1).get("invariant",false),
        "rejected build leaves all Realm state unchanged")
    var day=int(view.get("day",0))
    expect(session.realm_step_one_day(),"a manual day advances through the Realm clock")
    expect(session.realm_snapshot(1).get("day",day)==day+1,"clock advances exactly one day")
    var saved_fingerprint=str(session.realm_snapshot(1).fingerprint)
    var path=ProjectSettings.globalize_path("user://private-housing-ui-smoke.polyec")
    expect(session.realm_save(path,false),"save the manual Atlas world")
    expect(session.realm_step_one_day(),"advance after saving")
    expect(session.realm_save(path,true),"restore the saved manual Atlas world")
    var restored=session.realm_snapshot(1)
    expect(restored.get("fingerprint","")==saved_fingerprint and restored.get("atlas_manual_test",false),
        "save/load preserves housing-ready Atlas world and manual-mode label")
    panel.queue_free()
    await process_frame
    print("PRIVATE HOUSING UI SMOKE: %d/%d checks passed" % [checks-failures,checks])
    quit(0 if failures==0 else 1)
