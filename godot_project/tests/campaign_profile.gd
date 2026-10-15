extends SceneTree

func percentile(values: Array, fraction: float) -> float:
    values.sort()
    return float(values[mini(values.size() - 1, int(values.size() * fraction))])

func _initialize() -> void:
    call_deferred("run")

func run() -> void:
    var app = load("res://main.tscn").instantiate()
    root.add_child(app)
    root.size = Vector2i(1440, 900)
    await process_frame
    app._open_campaign()
    var panel = app.campaign_panel
    panel.send_command(1, 1)
    panel.send_command(6, 2)
    panel.send_command(0, 1, 1000)
    if panel.snapshot.cargo.size() != 1:
        push_error("Profile shipment was not funded")
        quit(1)
        return
    var frames: Array = []
    var days: Array = []
    var input_latency: Array = []
    var previous = Time.get_ticks_usec()
    for frame in range(300):
        await process_frame
        var now = Time.get_ticks_usec()
        if frame >= 30:
            frames.append(float(now - previous) / 1000.0)
        previous = now
        if frame % 10 == 0:
            var start = Time.get_ticks_usec()
            panel.step_day()
            days.append(float(Time.get_ticks_usec() - start) / 1000.0)
        if frame % 20 == 0:
            var id = [1, 2, 4][int(frame / 20) % 3]
            var click = InputEventMouseButton.new()
            click.button_index = MOUSE_BUTTON_LEFT
            click.pressed = true
            click.position = panel.map.global_position + panel.map.pan + panel.snapshot.regions[id].center * panel.map.zoom
            var start = Time.get_ticks_usec()
            root.push_input(click)
            await process_frame
            input_latency.append(float(Time.get_ticks_usec() - start) / 1000.0)
            if panel.selected != id:
                push_error("Profile input did not reach inspector")
                quit(1)
                return
            click.pressed = false
            root.push_input(click)
            previous = Time.get_ticks_usec()
    var path = ProjectSettings.globalize_path("user://campaign-profile.polyec")
    var start = Time.get_ticks_usec()
    if not app.session.m4_save(path, false):
        push_error("Profile save failed")
        quit(1)
        return
    var save_ms = float(Time.get_ticks_usec() - start) / 1000.0
    var bytes = FileAccess.open(path, FileAccess.READ).get_length()
    start = Time.get_ticks_usec()
    var restored = app.session.m4_save(path, true)
    var restore_ms = float(Time.get_ticks_usec() - start) / 1000.0
    var report = {"os":OS.get_name(), "cpu":OS.get_processor_name(), "gpu":RenderingServer.get_video_adapter_name(), "viewport":[1440,900], "population":panel.snapshot.population, "controller":"M4 labelled test controller; no ONNX inference", "workload":"30 days, 1000 food in transit, blocked road project, 6 weaving workers, monthly payroll, picking, save/restore", "frames":frames.size(), "frame_p50_ms":percentile(frames,0.5), "frame_p95_ms":percentile(frames,0.95), "frame_p99_ms":percentile(frames,0.99), "day_with_snapshot_p95_ms":percentile(days,0.95), "input_to_inspector_next_frame_p95_ms":percentile(input_latency,0.95), "save_ms":save_ms, "restore_ms":restore_ms, "save_bytes":bytes, "restore_success":restored, "invariant":app.session.m4_snapshot().invariant}
    var report_path = ProjectSettings.globalize_path("user://M4-profile.json")
    for arg in OS.get_cmdline_user_args():
        if arg.begins_with("--report-file="):
            report_path = arg.trim_prefix("--report-file=")
    FileAccess.open(report_path, FileAccess.WRITE).store_string(JSON.stringify(report, "  "))
    DirAccess.remove_absolute(path)
    print("M4 PROFILE: ", JSON.stringify(report))
    app.queue_free()
    await process_frame
    quit(0 if restored else 1)
