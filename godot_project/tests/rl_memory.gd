extends SceneTree
func _initialize() -> void:
    call_deferred("run")
func run() -> void:
    var app=load("res://main.tscn").instantiate()
    root.add_child(app)
    await process_frame
    app._open_rl()
    await process_frame
    app.session.set_active(true)
    for i in range(64):
        if not app.session.rl_step().success:
            push_error("Memory smoke inference failed")
            quit(1)
            return
    app.rl_lab.refresh()
    print("RL MEMORY:64 native decisions complete; keeping rendered app alive for process sampling")
    await create_timer(5.0).timeout
    app.queue_free()
    await process_frame
    quit()
