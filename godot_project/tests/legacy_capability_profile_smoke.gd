extends SceneTree

func _initialize() -> void:
    var panel_script=load("res://ui/realm/realm_panel.gd")
    var panel=panel_script.new()
    var labels=panel.legacy_capability_text({
        "strategy":11,"administration":22,"economic_planning":33,
        "diplomacy":44,"investigation":55,"military":66
    })
    var expected="Năng lực kế thừa: Chiến lược 11 · Hành chính 22 · Kế hoạch kinh tế 33 · Ngoại giao 44 · Điều tra 55 · Quân sự 66"
    if labels!=expected:
        push_error("CAPABILITY PROFILE FAIL: unexpected labels: "+labels)
        quit(1)
        return
    panel.free()
    print("CAPABILITY PROFILE SMOKE: 1 display check passed")
    quit(0)
