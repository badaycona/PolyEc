extends SceneTree

func _initialize() -> void:
    var session=ClassDB.instantiate("PolyEcSession")
    var catalog=session.capability_catalog()
    if catalog.size()!=7 or catalog[0].id!=0 or catalog[0].key!="strategy" or catalog[0].minimum!=0 or catalog[0].maximum!=100 or not catalog[0].scenario_configurable or catalog[0].disabled_value!=50:
        push_error("legacy capability metadata must describe its stable ID, range and neutral value")
        quit(1)
        return
    if catalog[6].id!=6 or catalog[6].key!="labor_skill" or catalog[6].unit!="labor skill point" or catalog[6].owner!="Person" or catalog[6].scenario_configurable or not catalog[6].always_active or catalog[6].disabled_value!=-1:
        push_error("labor skill metadata must describe its Person ownership and always-active range")
        quit(1)
        return
    var panel_script=load("res://ui/realm/realm_panel.gd")
    var evidence=panel_script.player_knowledge_text({
        "kind":1,"source_investigation_id":73,"confidence_basis_points":-1})
    if not evidence.contains("Bằng chứng điều tra #73") or not evidence.contains("độ tin cậy chưa được đo"):
        push_error("investigation evidence must be labelled without claiming verification")
        quit(1)
        return
    var legacy=panel_script.player_knowledge_text({"kind":0})
    if not legacy.contains("save cũ chưa rõ loại và nguồn"):
        push_error("legacy knowledge must retain an explicit unknown label")
        quit(1)
        return
    if not panel_script.labor_skill_text(73).contains("Kỹ năng lao động 73/100"):
        push_error("labor capability must name its person-owned 0–100 scale")
        quit(1)
        return
    print("CAPABILITY CATALOG AND LABEL: 5/5 checks passed")
    quit(0)
