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
	check(RealmPanel.good_name(10) == "sáp ong" and RealmPanel.beeswax_rest_description().contains("giảm thêm 5 điểm") and
		RealmPanel.fact_kind_name(239).contains("Beeswax"),
		"the player-facing Rest description and event explain the personal Beeswax cost and sleep-debt benefit")
	check(RealmPanel.good_name(12) == "lụa" and RealmPanel.silk_social_action_description().contains("trả 4 đồng") and
		RealmPanel.silk_social_action_description().contains("thêm 12") and RealmPanel.silk_social_action_description().contains("ngoài quân vụ"),
		"the player-facing Silk action explains its personal item, fund payment, social gain and eligibility")
	check(RealmPanel.good_name(8) == "vàng" and RealmPanel.gold_social_action_description().contains("trả 20 đồng") and
		RealmPanel.gold_social_action_description().contains("thêm 20") and RealmPanel.gold_social_action_description().contains("trưởng thành"),
		"the player-facing Gold action explains the personal item, fund payment, social gain and adult eligibility")
	var session = ClassDB.instantiate("PolyEcSession")
	var schema = session.realm_schema()
	check(schema == "realm-v45", "native core schema")
	check(session.realm_model_contract() == "realm-household-fund-goods30-config-v54-atlas-north-pass-v1-salted-meal-v1", "native core model contract")
	check(session.realm_atlas_manual_start(20261098, 512), "manual test can start the current Realm contract without an old policy")
	var realm_snapshot: Dictionary = session.realm_snapshot(129)
	check(realm_snapshot.has("apprenticeships") and realm_snapshot.apprenticeships is Array,
		"native snapshot exposes the course list for the player and Python-facing contract")
	check(realm_snapshot.apprenticeships.is_empty(), "a new world reports no invented apprenticeship records")
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
	var apiary_stock := []
	apiary_stock.resize(30)
	apiary_stock.fill(0)
	apiary_stock[10] = 1
	var apiary_line: String = RealmPanel.establishment_text([{"id": 53, "region": 21, "industry": 18, "stock": apiary_stock, "source_units": 24}], 21)
	check(apiary_line.contains("trại nuôi ong") and apiary_line.contains("sáp ong") and apiary_line.contains("24 source units") and apiary_line.contains("16 tick Work"),
		"Apiary UI separates saleable Beeswax stock from the finite source and explains production work")
	var silk_stock := []
	silk_stock.resize(30)
	silk_stock.fill(0)
	silk_stock[12] = 1
	var silk_farm_line: String = RealmPanel.establishment_text([{"id": 54, "region": 12, "industry": 19, "stock": silk_stock, "source_units": 25}], 12)
	check(silk_farm_line.contains("trại nuôi tằm") and silk_farm_line.contains("lụa") and silk_farm_line.contains("25 source units") and silk_farm_line.contains("16 tick Work"),
		"Silkworm Farm UI distinguishes its finite source from saleable Silk and explains production work")
	var gold_stock := []
	gold_stock.resize(30)
	gold_stock.fill(0)
	gold_stock[8] = 1
	var gold_mine_line: String = RealmPanel.establishment_text([{"id": 55, "region": 23, "industry": 20, "stock": gold_stock, "source_units": 100}], 23)
	check(gold_mine_line.contains("mỏ vàng") and gold_mine_line.contains("vàng") and gold_mine_line.contains("100 source units") and gold_mine_line.contains("không tái tạo") and gold_mine_line.contains("16 tick Work"),
		"Gold Mine UI distinguishes finite ore from saleable Gold and explains production work")
	var livestock_stock := []
	livestock_stock.resize(30)
	livestock_stock.fill(0)
	livestock_stock[13] = 1
	var ranch_line: String = RealmPanel.establishment_text([{"id": 56, "region": 18, "industry": 21, "stock": livestock_stock, "source_units": 25}], 18)
	check(ranch_line.contains("trại chăn nuôi") and ranch_line.contains("gia súc") and ranch_line.contains("25/100") and ranch_line.contains("không mô phỏng từng con vật"),
		"Livestock Ranch UI distinguishes the abstract herd from saleable animals")
	var butcher_stock := []
	butcher_stock.resize(30)
	butcher_stock.fill(0)
	butcher_stock[0] = 3
	var butchery_line: String = RealmPanel.establishment_text([{"id": 57, "region": 18, "industry": 22, "stock": butcher_stock}], 18)
	check(butchery_line.contains("lò mổ và chế biến") and butchery_line.contains("1 Livestock") and butchery_line.contains("3 DryFood"),
		"Butchery UI explains the real Livestock input and DryFood output")
	var decorative_stock := []
	decorative_stock.resize(30)
	decorative_stock.fill(0)
	decorative_stock[24] = 1
	decorative_stock[27] = 2
	decorative_stock[4] = 3
	var decorative_line: String = RealmPanel.establishment_text([{"id": 58, "region": 10, "industry": 23, "stock": decorative_stock}], 10)
	check(decorative_line.contains("xưởng kính trang trí") and decorative_line.contains("thủy tinh trang trí") and decorative_line.contains("2 kính thường") and decorative_line.contains("3 nhiên liệu") and decorative_line.contains("16 tick Work"),
		"Decorative Glass Works UI explains its output and both real recipe inputs")
	check(RealmPanel.decorative_glass_social_description().contains("trả 6 đồng") and RealmPanel.decorative_glass_social_description().contains("thêm 15") and RealmPanel.decorative_glass_social_description().contains("từ 14 tuổi"),
		"the player-facing DecorativeGlass action explains personal item, money, benefit and age eligibility")
	var porcelain_stock := []
	porcelain_stock.resize(30)
	porcelain_stock.fill(0)
	porcelain_stock[25] = 1
	porcelain_stock[28] = 2
	porcelain_stock[4] = 3
	var porcelain_line: String = RealmPanel.establishment_text([{"id": 59, "region": 15, "industry": 24, "stock": porcelain_stock}], 15)
	check(porcelain_line.contains("xưởng hoàn thiện đồ sứ") and porcelain_line.contains("đồ sứ tinh xảo") and porcelain_line.contains("2 đồ gốm thường") and porcelain_line.contains("3 nhiên liệu") and porcelain_line.contains("24 tick Work"),
		"FinePorcelain Works UI explains its finished product and both real recipe inputs")
	check(RealmPanel.fine_porcelain_social_description().contains("trả 12 đồng") and RealmPanel.fine_porcelain_social_description().contains("thêm 18") and RealmPanel.fine_porcelain_social_description().contains("trưởng thành"),
		"the hosting action explains personal item, money, benefit and adult eligibility")
	var luxury_stock := []
	luxury_stock.resize(30)
	luxury_stock.fill(0)
	luxury_stock[26] = 3
	var luxury_line: String = RealmPanel.establishment_text([{"id": 60, "region": 12, "industry": 25, "stock": luxury_stock}], 12)
	check(luxury_line.contains("xưởng chế tác hàng xa xỉ") and luxury_line.contains("3 hàng xa xỉ") and luxury_line.contains("1 trà") and luxury_line.contains("1 trầm hương") and luxury_line.contains("1 lụa") and luxury_line.contains("48 tick Work"),
		"General Luxury UI explains its actual three inputs, output and work requirement")
	check(RealmPanel.general_luxury_social_description().contains("trả 20 đồng") and RealmPanel.general_luxury_social_description().contains("thêm 25") and RealmPanel.general_luxury_social_description().contains("14 ngày") and RealmPanel.general_luxury_social_description().contains("trưởng thành"),
		"the luxury event explains personal item, money, benefit, duration and adult eligibility")
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
	var loaded = session.realm_load(paths, hashes, 39, 10000)
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
