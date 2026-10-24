extends SceneTree

var checks := 0
var failures := 0

func expect(value: bool, message: String) -> void:
	checks += 1
	if not value:
		failures += 1
		push_error("PRIVATE HOUSING UI FAIL: " + message)

func _initialize() -> void:
	call_deferred("run")

func find_button(node: Node, text: String, prefix := false) -> Button:
	if node is Button and (node.text.begins_with(text) if prefix else node.text == text):
		return node
	for child in node.get_children():
		var found := find_button(child, text, prefix)
		if found != null:
			return found
	return null

func find_candidate(view: Dictionary, name: String, target := -1, good := -999, term := -999, amount := -999) -> Dictionary:
	for action in view.get("actions", []):
		if not action.legal or action.name != name:
			continue
		if target >= 0 and int(action.target) != target:
			continue
		if good != -999 and int(action.good) != good:
			continue
		if term != -999 and int(action.term) != term:
			continue
		if amount != -999 and int(action.amount) != amount:
			continue
		return action
	return {}

func click_candidate(panel: Control, session: Object, actor: int, action: Dictionary, message: String) -> bool:
	if action.is_empty():
		expect(false, message + ": Realm không đưa ra lựa chọn hợp lệ")
		return false
	panel.actor = actor
	panel.refresh()
	for child in panel.commands.get_children():
		if child is Button and int(child.get_meta("realm_actor", -1)) == actor and int(child.get_meta("realm_action_index", -1)) == int(action.index):
			expect(not child.disabled, message + ": nút trên giao diện phải bật")
			child.pressed.emit()
			return true
	expect(false, message + ": không tìm thấy nút lệnh đang hiển thị")
	return false

func click_day(panel: Control, expected_day: int) -> void:
	var next_day := find_button(panel, "Một ngày")
	expect(next_day != null, "người chơi có nút tiến đúng một ngày game")
	if next_day != null:
		next_day.pressed.emit()
		expect(int(panel.session.realm_snapshot(panel.actor).day) == expected_day + 1, "nút ngày tiến đúng một ngày và giữ Realm chạy được")

func run() -> void:
	var session: Object = ClassDB.instantiate("PolyEcSession")
	var panel_script = load("res://ui/realm/realm_panel.gd")
	expect(panel_script != null, "giao diện Realm nạp được")
	expect(session.has_method("realm_atlas_manual_start") and session.has_method("realm_private_residence_build") and session.has_method("realm_private_housing_command"), "GDExtension cung cấp thế giới thủ công và các lệnh nhà tư")
	var panel = panel_script.new()
	panel.session = session
	root.add_child(panel)
	await process_frame
	var manual_button := find_button(panel, "Atlas kiểm thử thủ công")
	expect(manual_button != null, "nút mở Atlas kiểm thử hiển thị khi chưa có model tương thích")
	if manual_button == null:
		quit(1)
		return
	manual_button.pressed.emit()
	expect(session.realm_snapshot(1).get("atlas_manual_test", false), "nút mở cùng C++ Realm ở chế độ kiểm thử có nhãn")
	expect(panel.status.text.contains("NPC không tự ra quyết định") and panel.region_rows.size() == 27, "màn hình nói rõ NPC chưa tự quyết định và hiển thị 27 vùng")
	var initial: Dictionary = session.realm_snapshot(34)
	expect(initial.person.inventory.size() == 30 and initial.person.inventory[19] == 0 and initial.person.inventory[16] == 0, "người mua bắt đầu không có sẵn vật liệu xây dựng hoặc công cụ")
	expect(initial.person.cash >= 40 and initial.land_holdings.any(func(land): return int(land.owner_id) == 34 and int(land.region) == 6), "người chơi có đủ tiền và một thửa đô thị đứng tên mình theo dữ liệu Atlas")
	expect(initial.regions[6].stock[3] == 0 and initial.regions[6].stock[19] == 0, "chợ vùng bắt đầu không có sẵn Timber hoặc vật liệu xây dựng")

	# Create Timber through the public Forestry, then move it through the ordinary market and Sawmill commands.
	var authority := 8
	var forest_action := find_candidate(session.realm_snapshot(authority), "Tạo cơ sở sản xuất", 6, 5, -1, 25)
	click_candidate(panel, session, authority, forest_action, "cơ quan vùng lập xưởng rừng bằng quỹ công")
	var forest_id := int(session.realm_snapshot(authority).establishments.back().id)
	var forest_authorization := find_candidate(session.realm_snapshot(authority), "Cho phép làm tại xưởng", forest_id)
	click_candidate(panel, session, authority, forest_authorization, "cơ quan vùng cho phép một lao động làm ở xưởng rừng")
	var forest_worker := int(forest_authorization.good)
	for day in range(4):
		var work := find_candidate(session.realm_snapshot(forest_worker), "Làm tại xưởng", forest_id)
		click_candidate(panel, session, forest_worker, work, "lao động thực hiện ngày làm thứ %d tại xưởng rừng" % (day + 1))
		click_day(panel, int(session.realm_snapshot(forest_worker).day))
	expect(session.realm_snapshot(authority).establishments[0].stock[3] >= 2, "lao động tạo Timber thật từ nguồn gỗ có giới hạn")
	for unit in range(2):
		var withdraw := find_candidate(session.realm_snapshot(authority), "Chuyển hàng xưởng", forest_id, 3, 3, 1)
		click_candidate(panel, session, authority, withdraw, "chuyển Timber đã sản xuất vào chợ")
	var sawmill_action := find_candidate(session.realm_snapshot(authority), "Tạo cơ sở sản xuất", 6, 17, -1, 25)
	click_candidate(panel, session, authority, sawmill_action, "cơ quan vùng lập xưởng cưa theo công thức đã định")
	var sawmill_id := int(session.realm_snapshot(authority).establishments.back().id)
	for unit in range(2):
		var deposit := find_candidate(session.realm_snapshot(authority), "Chuyển hàng xưởng", sawmill_id, 3, 2, 1)
		click_candidate(panel, session, authority, deposit, "chuyển Timber từ chợ vào xưởng cưa")
	var sawmill_authorization := find_candidate(session.realm_snapshot(authority), "Cho phép làm tại xưởng", sawmill_id)
	click_candidate(panel, session, authority, sawmill_authorization, "cơ quan vùng cho phép lao động thứ hai làm tại xưởng cưa")
	var sawmill_worker := int(sawmill_authorization.good)
	for day in range(8):
		var work := find_candidate(session.realm_snapshot(sawmill_worker), "Làm tại xưởng", sawmill_id)
		click_candidate(panel, session, sawmill_worker, work, "lao động thực hiện ngày làm thứ %d tại xưởng cưa" % (day + 1))
		click_day(panel, int(session.realm_snapshot(sawmill_worker).day))
	expect(session.realm_snapshot(authority).establishments.back().stock[19] >= 2, "xưởng cưa tiêu Timber và tạo đủ hai vật liệu xây dựng")
	for unit in range(2):
		var withdraw := find_candidate(session.realm_snapshot(authority), "Chuyển hàng xưởng", sawmill_id, 19, 3, 1)
		click_candidate(panel, session, authority, withdraw, "đưa vật liệu xây dựng từ xưởng vào chợ")
	expect(session.realm_snapshot(34).regions[6].stock[19] >= 2, "chợ chỉ có vật liệu sau khi xưởng thật sự sản xuất và giao hàng")

	# The private buyer uses the visible market-order buttons, then the visible housing commands.
	panel.actor = 34
	panel.refresh()
	var buy_materials := find_button(panel.commands, "Đặt lệnh mua 2 vật liệu xây dựng", true)
	var buy_tools := find_button(panel.commands, "Đặt lệnh mua 1 công cụ", true)
	expect(buy_materials != null and buy_tools != null and not buy_materials.disabled and not buy_tools.disabled, "giao diện chỉ bật hai đơn mua khi chợ có hàng và người chơi đủ tiền")
	if buy_materials != null: buy_materials.pressed.emit()
	# The first order refreshes and replaces all controls; find the Tools order again.
	buy_tools = find_button(panel.commands, "Đặt lệnh mua 1 công cụ", true)
	if buy_tools != null: buy_tools.pressed.emit()
	var order_day := int(session.realm_snapshot(34).day)
	click_day(panel, order_day)
	var supplied: Dictionary = session.realm_snapshot(34)
	expect(supplied.person.inventory[19] == 2 and supplied.person.inventory[16] == 1 and supplied.person.cash >= 40, "chợ giao đúng hai vật liệu và một công cụ; người chơi vẫn đủ 40 đồng xây nhà")
	if supplied.person.inventory[19] != 2 or supplied.person.inventory[16] != 1:
		panel.queue_free()
		quit(1)
		return
	var build_button := find_button(panel.commands, "Xây nhà (40 đồng", true)
	expect(build_button != null and not build_button.disabled, "người chơi có thể chọn chính thửa đất mình đứng tên")
	if build_button != null: build_button.pressed.emit()
	var building_fingerprint := str(session.realm_snapshot(34).fingerprint)
	var save_build_button := find_button(panel, "Lưu")
	if save_build_button != null: save_build_button.pressed.emit()
	click_day(panel, int(session.realm_snapshot(34).day))
	var load_build_button := find_button(panel, "Tải")
	if load_build_button != null: load_build_button.pressed.emit()
	expect(str(session.realm_snapshot(34).fingerprint) == building_fingerprint and not session.realm_snapshot(34).private_residences[0].completed, "lưu và tải trong lúc xây giữ nguyên tiến độ còn dang dở")
	var home_id := int(session.realm_snapshot(34).private_residences.back().id)
	for day in range(30):
		click_day(panel, int(session.realm_snapshot(34).day))
	var completed: Dictionary = session.realm_snapshot(34)
	expect(completed.private_residences[0].completed, "căn nhà hoàn thành sau 30 ngày game")
	var tenant_picker: OptionButton = null
	for child in panel.commands.get_children():
		if child is OptionButton and child.item_count > 0:
			tenant_picker = child
			break
	expect(tenant_picker != null, "chủ nhà thấy lựa chọn các hộ trưởng thành đủ điều kiện thuê")
	var richest_tenant_option := 0
	var richest_tenant_cash := -1
	if tenant_picker != null:
		for option in range(tenant_picker.item_count):
			var household_id := int(tenant_picker.get_item_metadata(option))
			for household in completed.private_housing_households:
				if int(household.household_id) == household_id:
					var tenant_cash := int(session.realm_snapshot(int(household.authority_id)).person.cash)
					if tenant_cash > richest_tenant_cash:
						richest_tenant_cash = tenant_cash
						richest_tenant_option = option
	if tenant_picker != null: tenant_picker.select(richest_tenant_option)
	var offer_button := find_button(panel.commands, "Mời hộ được chọn thuê", true)
	expect(offer_button != null and not offer_button.disabled, "chủ nhà có thể gửi đề nghị thuê căn đã hoàn thành")
	if offer_button != null: offer_button.pressed.emit()
	var offered: Dictionary = session.realm_snapshot(34).private_residences[0]
	var tenant_household := int(offered.offered_household)
	var tenant_authority := 0
	for household in session.realm_snapshot(34).private_housing_households:
		if int(household.household_id) == tenant_household:
			tenant_authority = int(household.authority_id)
			break
	expect(tenant_household > 0 and tenant_authority > 0, "đề nghị gắn với hộ và người đại diện đã chọn trong cùng đô thị")
	panel.actor = tenant_authority
	panel.refresh()
	var accept_button := find_button(panel.commands, "Chấp thuận đề nghị nhà #%d" % home_id)
	expect(accept_button != null, "chính người đại diện hộ nhận được nút chấp thuận")
	if accept_button != null: accept_button.pressed.emit()
	var rent_due_day := int(session.realm_snapshot(34).private_residences[0].next_rent_day)
	var days_until_rent := rent_due_day - int(session.realm_snapshot(tenant_authority).day)
	for day in range(days_until_rent):
		click_day(panel, int(session.realm_snapshot(tenant_authority).day))
	var paid: Dictionary = session.realm_snapshot(34)
	expect(int(paid.private_residences[0].tenant_household) == tenant_household and int(paid.private_residences[0].next_rent_day) == rent_due_day + 30, "đến hạn, hợp đồng tiếp tục và Realm chuyển kỳ thu tiền sang đúng 30 ngày game kế tiếp")
	expect(paid.invariant, "sổ tiền, hàng, quyền đất và hợp đồng vẫn thỏa các điều kiện Realm")
	panel.actor = 34
	panel.refresh()
	var save_button := find_button(panel, "Lưu")
	if save_button != null: save_button.pressed.emit()
	var saved := str(session.realm_snapshot(34).fingerprint)
	click_day(panel, int(session.realm_snapshot(34).day))
	var load_button := find_button(panel, "Tải")
	if load_button != null: load_button.pressed.emit()
	expect(str(session.realm_snapshot(34).fingerprint) == saved and session.realm_snapshot(34).invariant, "nút lưu và tải khôi phục căn nhà, hợp đồng thuê và sổ tiền chính xác")
	panel.queue_free()
	await process_frame
	print("PRIVATE HOUSING POSITIVE UI CHAIN: %d/%d checks passed" % [checks - failures, checks])
	quit(0 if failures == 0 else 1)
