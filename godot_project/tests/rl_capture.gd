extends SceneTree

func _initialize() -> void:
    call_deferred("capture")

func capture() -> void:
    var target="user://PolyEc-RL.png"
    for argument in OS.get_cmdline_user_args():
        if argument.begins_with("--capture-file="): target=argument.trim_prefix("--capture-file=")
    var app=load("res://main.tscn").instantiate()
    root.add_child(app)
    await process_frame
    app._open_rl()
    await process_frame
    app.session.set_active(true)
    for i in range(18): app.rl_lab.step_once()
    for i in range(5): await process_frame
    await RenderingServer.frame_post_draw
    var error=root.get_texture().get_image().save_png(target)
    if error!=OK:
        push_error("RL capture failed")
        quit(1)
        return
    print("RL CAPTURE: "+target)
    app.queue_free()
    await process_frame
    quit()
