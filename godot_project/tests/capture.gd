extends SceneTree

func _initialize() -> void:
    call_deferred("capture")

func capture() -> void:
    var destination="user://PolyEc-preview.png"
    for argument in OS.get_cmdline_user_args():
        if argument.begins_with("--capture-file="):
            destination=argument.trim_prefix("--capture-file=")
    var app=load("res://main.tscn").instantiate()
    root.add_child(app)
    for i in range(8): await process_frame
    await RenderingServer.frame_post_draw
    var error=root.get_texture().get_image().save_png(destination)
    if error!=OK:
        push_error("Capture failed: "+str(error))
        quit(1)
        return
    print("CAPTURE: "+destination)
    app._select_territory(8)
    await process_frame
    await RenderingServer.frame_post_draw
    root.get_texture().get_image().save_png(destination.replace(".png","-hac-son.png"))
    app._open_help()
    await process_frame
    await RenderingServer.frame_post_draw
    root.get_texture().get_image().save_png(destination.replace(".png","-help.png"))
    app.queue_free()
    await process_frame
    quit()
