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
	check(RealmPanel.good_name(4) == "nhiên liệu" and RealmPanel.good_name(6) == "đồng",
		"canonical GoodId labels identify Fuel and Copper at their actual indices")
	check(RealmPanel.establishment_transfer_action_text(6, 1, 53, true) == "đưa vào xưởng đồng · tối đa 1 món · cơ sở #53",
		"Copper deposit candidate is named clearly for the player")
	check(RealmPanel.establishment_transfer_action_text(4, 1, 53, true) == "đưa vào xưởng nhiên liệu · tối đa 1 món · cơ sở #53",
		"Fuel deposit candidate remains clearly named after Copper is supplied")
	var session = ClassDB.instantiate("PolyEcSession")
	var schema = session.realm_schema()
	check(schema == "realm-v34", "native core schema")
	check(session.realm_model_contract() == "realm-household-fund-goods30-config-v43", "native core model contract")
	for case in [[10, 12, 27, "thủy tinh thường", 90], [15, 13, 28, "đồ gốm thường", 55], [17, 14, 29, "da thuộc", 80]]:
		var stock := []
		stock.resize(30)
		stock.fill(0)
		stock[case[2]] = 1
		var line: String = RealmPanel.establishment_text([{"id": 51, "region": case[0], "industry": case[1], "stock": stock, "source_units": 99}], case[0])
		check(line.contains(case[3]) and line.contains("tồn kho 1") and line.contains("tối đa 99 mẻ") and line.contains("giá tham chiếu thử %d" % case[4]),
			"Atlas specialty UI explains the product, output, finite source and trial reference price")
	var sawmill_stock := []
	sawmill_stock.resize(30)
	sawmill_stock.fill(0)
	sawmill_stock[19] = 1
	var sawmill_line: String = RealmPanel.establishment_text([{"id": 52, "region": 6, "industry": 17, "stock": sawmill_stock}], 6)
	check(sawmill_line.contains("xưởng cưa") and sawmill_line.contains("vật liệu xây dựng") and sawmill_line.contains("Timber") and sawmill_line.contains("16 tick Work"),
		"Sawmill UI names its real Timber input, construction output and Work requirement")
	var census_line: String = RealmPanel.urban_census_text([{"site_name": "Nam Cảng", "urban_residents": 24, "housing_capacity": 24, "unhoused_residents": 0, "available_beds": 0}], true)
	check(census_line.contains("sức chứa 24") and census_line.contains("còn thiếu chỗ 0") and census_line.contains("giường trống 0"),
		"housing census UI explains resident count, capacity, unhoused residents and vacant beds")
	var council_line: String = RealmPanel.urban_census_text([{"site_name":"Đông Viên", "urban_residents":20,
		"housing_capacity":24, "unhoused_residents":0, "available_beds":4, "rural_residents":12,
		"unknown_residence":0, "council_representative_id":41, "council_nominee_id":42,
		"council_nomination_day":20, "council_term_end_day":1480}], true)
	check(council_line.contains("cá thể #41") and council_line.contains("hết nhiệm kỳ ngày 1480"),
		"council UI identifies the appointed representative and term end")
	check(council_line.contains("cá thể #42") and council_line.contains("chưa phải bầu cử"),
		"council UI distinguishes pending self-nomination from an election")
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


