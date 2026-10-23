extends SceneTree

func _initialize() -> void:
    var panel_script=load("res://ui/realm/realm_panel.gd")
    var panel=panel_script.new()
    var text=panel.body_status_text({"sleep_debt":3,"exertion":57})
    if text!="Nợ ngủ 3 ngày quy đổi · Gắng sức 57/100 (chỉ số thử)":
        push_error("BODY PROFILE FAIL: unexpected body status text: "+text)
        quit(1)
        return
    panel.free()
    print("BODY PROFILE SMOKE: 1 display check passed")
    quit(0)
