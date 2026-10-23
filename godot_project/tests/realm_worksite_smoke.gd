extends SceneTree

const RealmPanel = preload("res://ui/realm/realm_panel.gd")

var checks := 0
var failed := false

func check(ok: bool, message: String) -> void:
	checks += 1
	if not ok:
		failed = true
		push_error("WORKSITE GODOT FAIL: " + message)

func _initialize() -> void:
	call_deferred("run")

func run() -> void:
	var session = ClassDB.instantiate("PolyEcSession")
	var schema = session.realm_schema()
	check(schema == "realm-v31", "native core schema")
	check(session.realm_model_contract() == "realm-household-fund-goods30-config-v33", "native core model contract")
	for case in [[10, 12, 27, "thủy tinh thường", 90], [15, 13, 28, "đồ gốm thường", 55], [17, 14, 29, "da thuộc", 80]]:
		var stock := []
		stock.resize(30)
		stock.fill(0)
		stock[case[2]] = 1
		var line: String = RealmPanel.establishment_text([{"id": 51, "region": case[0], "industry": case[1], "stock": stock, "source_units": 99}], case[0])
		check(line.contains(case[3]) and line.contains("tồn kho 1") and line.contains("tối đa 99 mẻ") and line.contains("giá tham chiếu thử %d" % case[4]),
			"Atlas specialty UI explains the product, output, finite source and trial reference price")
	var paths := []
	var hashes := []
	for family in ["governance", "economic", "civilian"]:
		var base = ProjectSettings.globalize_path("res://models/realm-v6/" + family + "/")
		var manifest = JSON.parse_string(FileAccess.get_file_as_string(base + "manifest.json"))
		paths.append(base + family + ".onnx")
		hashes.append(manifest.sha256)
	var loaded = session.realm_load(paths, hashes, 37, 10000)
	check(not loaded, "historical realm-v6 model bundle is rejected after service actions and observation length changed")
	var rejected = session.realm_snapshot(65)
	check(not rejected.loaded, "incompatible bundle leaves Realm unloaded")
	check(rejected.error.find("model bundle") >= 0,
		"GDExtension explains why an old model bundle cannot be used: " + rejected.error)
	if failed:
		print("HOUSEHOLD MODEL GUARD GODOT SMOKE: FAILED (", checks, " checks)")
		quit(1)
	else:
		print("HOUSEHOLD MODEL GUARD GODOT SMOKE: ", checks, "/", checks, " checks passed")
		quit(0)


