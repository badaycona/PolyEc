extends SceneTree

var checks: int = 0
var failures: int = 0

func expect(value: bool, message: String) -> void:
    checks += 1
    if not value:
        failures += 1
        push_error(message)

func _initialize() -> void:
    call_deferred("run")

func run() -> void:
    var app = load("res://main.tscn").instantiate()
    root.add_child(app)
    await process_frame
    app._open_campaign()
    await process_frame
    var panel = app.campaign_panel
    expect(panel.visible and panel.snapshot.population == 10000, "M4 opens real 10k world without ONNX dependency")
    expect(panel.snapshot.regions.size() == 7, "Native geography includes crown and six provinces")
    expect(not panel.running and panel.snapshot.day == 0, "Starts paused")
    expect(not app.session.m4_step(0) and not app.session.m4_snapshot().error.is_empty(), "Invalid day chunk gives an error reason")
    var at: Vector2 = panel.map.pan + panel.snapshot.regions[2].center * panel.map.zoom
    expect(panel.map.region_at(at) == 2, "Pick Bạch Khâu from native polygon")
    var click = InputEventMouseButton.new()
    click.button_index = MOUSE_BUTTON_LEFT
    click.pressed = true
    click.position = at
    panel.map._gui_input(click)
    expect(panel.selected == 2 and panel.profile.text.contains("Bạch Khâu"), "One click selects inspector")
    var wheel = InputEventMouseButton.new()
    wheel.button_index = MOUSE_BUTTON_WHEEL_UP
    wheel.pressed = true
    wheel.position = at
    panel.map._gui_input(wheel)
    expect(panel.map.region_at(at) == 2, "Cursor anchored zoom preserves picking")
    panel.map.pan += Vector2(-35, 40)
    at = panel.map.pan + panel.snapshot.regions[1].center * panel.map.zoom
    expect(panel.map.region_at(at) == 1, "Picking after pan")
    for layer in [0, 1, 2]:
        panel.map.overlay = layer
        panel.map.queue_redraw()
        await process_frame
        expect(panel.map.region_at(at) == 1, "Overlay does not change picking")
    panel.select_region(1)
    expect(panel.map.route_path.size() > 1 and panel.route_label.text.contains("phí"), "Route preview comes from core")
    var space = InputEventKey.new()
    space.keycode = KEY_SPACE
    space.pressed = true
    root.push_input(space)
    await process_frame
    expect(panel.running, "One Space resumes once")
    space.pressed = false
    root.push_input(space)
    panel._process(0.5)
    expect(not panel.running and panel.snapshot.day == 0, "Long frame pauses without catch up")
    panel.quantity.get_line_edit().grab_focus()
    space.pressed = true
    root.push_input(space)
    await process_frame
    expect(not panel.running, "Space in number entry does not resume")
    space.pressed = false
    root.push_input(space)
    panel.quantity.get_line_edit().release_focus()
    panel.show_guide()
    expect(panel.help_dialog.visible and not panel.running, "Help pauses")
    panel.toggle_run()
    expect(not panel.running, "Help blocks resume")
    panel.help_dialog.hide()
    panel.toggle_run()
    panel._notification(Node.NOTIFICATION_APPLICATION_FOCUS_OUT)
    app._notification(Node.NOTIFICATION_APPLICATION_FOCUS_OUT)
    expect(not panel.running and not app.session.m4_step(1), "Focus loss blocks tick and player actions")
    expect(panel.run_button.text.begins_with("▶"), "Focus loss updates visible pause state")
    app._notification(Node.NOTIFICATION_APPLICATION_FOCUS_IN)
    expect(not panel.running, "Focus return remains paused")
    app.session.set_active(true)
    expect(app.session.m4_start(37, 256), "Small fixture uses same native core")
    panel.refresh()
    var original: String = app.session.m4_fingerprint()
    var seq: int = panel.snapshot.sequence
    var rev: int = panel.snapshot.revision
    expect(not app.session.m4_command(3, 1, 200, seq, rev), "No ruling before evidence")
    expect(app.session.m4_fingerprint() == original, "Rejected UI command leaves world unchanged")
    panel.send_command(4, 1)
    for _i in range(3):
        panel.step_day()
    panel.send_command(3, 1)
    expect(not panel.snapshot.routes[0].claimed, "Investigate and rule alternate path")
    expect(panel.snapshot.regions[1].unrest >= 10, "Ruling has visible political consequence")
    panel.send_command(1, 1)
    expect(not app.session.m4_command(1, 1, 200, seq, rev), "Stale snapshot cannot commit again")
    expect(app.session.m4_step(12), "Project progresses in game time")
    panel.refresh()
    expect(panel.snapshot.routes[0].works_due == 0 and panel.snapshot.routes[0].days == 2, "Completed road changes logistics")
    panel.send_command(0, 1, 200)
    expect(panel.snapshot.cargo.size() == 1 and not panel.snapshot.cargo[0].delivered, "Cargo does not teleport")
    panel.send_command(5, 1)
    expect(panel.snapshot.debt == 0, "Debt repayment reaches ledger")
    panel.save_path = ProjectSettings.globalize_path("user://campaign-smoke.polyec")
    original = app.session.m4_fingerprint()
    panel.save_game()
    expect(panel.feedback.text.begins_with("Đã lưu"), "Native save succeeds")
    panel.save_game()
    expect(FileAccess.file_exists(panel.save_path + ".bak"), "Overwrite keeps backup")
    panel.step_day()
    panel.load_game()
    expect(app.session.m4_fingerprint() == original and not panel.running, "Restore preserves manifest and pauses")
    var arrival: int = panel.snapshot.cargo[0].arrives
    while panel.snapshot.day < arrival + 2:
        panel.step_day()
    expect(panel.snapshot.cargo[0].delivered and panel.snapshot.invariant, "Cargo arrives with conservation")
    var closed = false
    for k in panel.snapshot.cases:
        closed = closed or (k.region == 1 and k.closed >= 0)
    expect(closed, "Complete case can close through native UI commands")
    panel.send_command(6, 2)
    expect(app.session.m4_step(14), "Workshop runs through shared native simulation")
    panel.refresh()
    expect(panel.snapshot.cloth > 0 and panel.snapshot.workshop_workers > 0, "Real labor inputs produce cloth")
    panel.close_panel()
    expect(not panel.visible and not panel.running, "Leaving M4 pauses")
    expect(app.map.visible and not app.help_shade.visible, "Returning restores prior screen visibility")
    for path in [panel.save_path, panel.save_path + ".bak"]:
        if FileAccess.file_exists(path):
            DirAccess.remove_absolute(path)
    print("CAMPAIGN SMOKE: %d/%d checks passed" % [checks - failures, checks])
    app.queue_free()
    await process_frame
    quit(1 if failures else 0)
