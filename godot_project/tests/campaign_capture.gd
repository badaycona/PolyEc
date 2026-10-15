extends SceneTree

func _initialize() -> void:
    call_deferred("run")

func run() -> void:
    var app = load("res://main.tscn").instantiate()
    root.add_child(app)
    root.size = Vector2i(1100, 720) if OS.get_cmdline_user_args().has("--small") else Vector2i(1440, 900)
    await process_frame
    app._open_campaign()
    var panel = app.campaign_panel
    if OS.get_cmdline_user_args().has("--resolved"):
        panel.send_command(2, 1)
        panel.send_command(1, 1)
        panel.send_command(6, 2)
        for _i in range(12): panel.step_day()
        panel.send_command(5, 1)
        panel.send_command(0, 1, 5000)
        var arrival: int = panel.snapshot.cargo[0].arrives
        while panel.snapshot.day < arrival + 2: panel.step_day()
    else:
        panel.send_command(1, 1)
        for _i in range(15): panel.step_day()
    panel.map.overlay = 1
    panel.layer_select.select(1)
    panel.refresh()
    await process_frame
    await process_frame
    await RenderingServer.frame_post_draw
    var path = ProjectSettings.globalize_path("user://M4-preview.png")
    for arg in OS.get_cmdline_user_args():
        if arg.begins_with("--capture-file="):
            path = arg.trim_prefix("--capture-file=")
    var error = root.get_texture().get_image().save_png(path)
    print("M4 CAPTURE: ", path, " error=", error)
    app.queue_free()
    await process_frame
    quit(0 if error == OK else 1)
