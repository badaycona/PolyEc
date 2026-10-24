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
    var realm_panel_script=load("res://ui/realm/realm_panel.gd")
    expect(realm_panel_script.fact_kind_name(238)=="Ăn thêm một món thực phẩm bổ sung","supplementary civilian meal event has a clear player-facing label")
    var tea_social_text=realm_panel_script.tea_social_action_description(1)
    expect(tea_social_text.contains("Dùng 1 Tea của chính mình") and tea_social_text.contains("tăng điểm giao lưu thêm 5 (tối đa 100)") and tea_social_text.contains("bận 7 ngày game"),"Tea social choice explains its personal item cost, benefit cap and duration")
    var incense_social_text=realm_panel_script.incense_social_action_description(2)
    expect(incense_social_text.contains("Dùng 1 Incense của chính mình") and incense_social_text.contains("trả 2 đồng") and incense_social_text.contains("tăng điểm giao lưu thêm 8 (tối đa 100)") and incense_social_text.contains("bận 7 ngày game"),"Incense social choice explains its personal item, cost, benefit cap and duration")
    var mounted_travel_text=realm_panel_script.travel_action_description("Đông Viên",3,true)
    expect(mounted_travel_text.contains("Đi tới Đông Viên · 3 ngày game · phí 10 đồng") and mounted_travel_text.contains("dùng WarHorse cá nhân") and mounted_travel_text.contains("không tiêu ngựa"),"mounted travel explains the reduced route time, resulting fee and durable personal horse")
    var council_text=realm_panel_script.urban_census_text([{
        "site_id": 3,"territory_id": 3,"site_name":"Đông Viên",
        "urban_residents": 20,"housing_capacity": 24,"unhoused_residents": 0,
        "available_beds": 4,"rural_residents": 12,"unknown_residence": 0,
        "council_representative_id": 41,"council_nominee_id": 42,
        "council_nomination_day": 20,"council_term_end_day": 1480,
    }],true)
    expect(council_text.contains("cá thể #41") and council_text.contains("hết nhiệm kỳ ngày 1480"),"Council view identifies the appointed representative and term end")
    expect(council_text.contains("cá thể #42") and council_text.contains("chưa phải bầu cử"),"Council view explains that a pending nomination is not an election")
    var packed: PackedScene=load("res://main.tscn")
    var app=packed.instantiate()
    root.add_child(app)
    await process_frame
    expect(app.session.snapshot().selected==7,"Initial selection is a native command")
    expect(app.session.has_method("realm_military_raid"),"GDExtension exposes the external raid command")
    expect(app.session.has_method("realm_military_battle"),"GDExtension exposes the Sovereign battle command")
    expect(app.title_label.text=="Xích Nham","Inspector reads Atlas name")
    expect(app.map.regions.size()==27,"Main map loads all 27 Atlas territories")
    expect(app.map.settlements.size()==27,"Main map loads all 27 fixed Atlas urban sites")
    for id in range(27):
        var geometry=app.map.region_for_id(id)
        var site=app.map.settlement_for_territory(id)
        expect(not site.is_empty(),"Atlas territory %d has one named central site" % id)
        expect(int(site.get("id",-1))==id and not str(site.get("name","")).is_empty(),"Site ID and name match Atlas territory %d" % id)
        var site_position: Vector2=site.get("position",Vector2.ZERO)
        expect(Geometry2D.is_point_in_polygon(site_position,geometry["polygon"]),"Central site lies inside territory %d" % id)
        var anchor: Vector2=geometry.anchor
        var screen=app.map.pan+anchor*app.map.zoom
        expect(app.map.region_at(screen)==id,"Polygon picking matches Atlas ID %d" % id)
    expect(app.map.region_at(Vector2(0,0))==-1,"Outside map is not mistaken for territory zero")
    var at: Vector2=app.map.pan+app.map.region_for_id(8).anchor*app.map.zoom
    expect(app.map.region_at(at)==8,"Polygon picking at Atlas anchor")
    var click=InputEventMouseButton.new()
    click.button_index=MOUSE_BUTTON_LEFT
    click.pressed=true
    click.position=at
    app.map._gui_input(click)
    expect(app.session.snapshot().selected==8,"Map click sends native select")
    expect(app.title_label.text=="Nam Cảng","Inspector follows Atlas selection")
    var wheel=InputEventMouseButton.new()
    wheel.button_index=MOUSE_BUTTON_WHEEL_UP
    wheel.pressed=true
    wheel.position=at
    app.map._gui_input(wheel)
    expect(app.map.region_at(at)==8,"Cursor-anchored zoom preserves picking")
    app.map.pan+=Vector2(-37,25)
    var shifted=app.map.pan+app.map.region_for_id(7).anchor*app.map.zoom
    expect(app.map.region_at(shifted)==7,"Picking follows pan")
    app._select_territory(0)
    expect(app.session.snapshot().selected==0,"Atlas territory zero survives native ID translation")
    expect(app.title_label.text=="Thanh Điền","Territory zero opens its matching inspector")
    app._set_speed(2)
    expect(app.session.snapshot().speed==2,"Speed button reaches core controller")
    app._open_help()
    expect(app.help_open and app.help_shade.visible,"Help opens")
    expect(app.session.snapshot().paused,"Help pauses")
    app._close_help()
    expect(not app.help_open and not app.help_shade.visible,"Help closes")
    expect(app.session.snapshot().paused,"Closing help does not resume")
    app._notification(Node.NOTIFICATION_APPLICATION_FOCUS_OUT)
    expect(not app.session.command("resume").success,"Focus loss blocks resume")
    app._notification(Node.NOTIFICATION_APPLICATION_FOCUS_IN)
    expect(app.session.snapshot().paused,"Focus return remains paused")
    app.session.set_active(true)
    app.play_button.grab_focus()
    var space=InputEventKey.new()
    space.keycode=KEY_SPACE
    space.pressed=true
    root.push_input(space)
    await process_frame
    expect(not app.session.snapshot().paused,"One Space resumes even with button focus")
    space.pressed=false
    root.push_input(space)
    var f1=InputEventKey.new()
    f1.keycode=KEY_F1
    f1.pressed=true
    root.push_input(f1)
    await process_frame
    expect(app.help_open and app.session.snapshot().paused,"F1 opens help and pauses")
    f1.pressed=false
    root.push_input(f1)
    var escape=InputEventKey.new()
    escape.keycode=KEY_ESCAPE
    escape.pressed=true
    root.push_input(escape)
    await process_frame
    expect(not app.help_open,"Escape closes help")
    expect(app.session.snapshot().paused,"Escape keeps the calendar paused")
    app.queue_free()
    await process_frame
    print("UI SMOKE: %d/%d checks passed" % [checks-failures,checks])
    quit(0 if failures==0 else 1)
