extends SceneTree
func _initialize() -> void: call_deferred("run")
func run() -> void:
    var app=load("res://main.tscn").instantiate()
    root.add_child(app)
    await process_frame
    app._open_realm()
    await process_frame
    app.session.set_active(true)
    var v=app.session.realm_snapshot(1)
    app.session.realm_draft(v.cards[0],false)
    for day in range(35): app.session.realm_step(1)
    app.realm_panel.actor=130
    app.realm_panel.refresh()
    await process_frame
    await RenderingServer.frame_post_draw
    var path="user://realm-preview.png"
    for arg in OS.get_cmdline_user_args():
        if arg.begins_with("--capture-file="): path=arg.trim_prefix("--capture-file=")
    var image=root.get_texture().get_image()
    var error=image.save_png(path)
    if error!=OK:
        push_error("Capture save failed: "+str(error))
        quit(1)
        return
    print("REALM CAPTURE: ",path)
    quit(0)
