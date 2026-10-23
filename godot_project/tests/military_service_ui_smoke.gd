extends SceneTree

const RealmPanel = preload("res://ui/realm/realm_panel.gd")

func _initialize() -> void:
	var checks := 0
	var inactive := RealmPanel.military_service_text({"active": false}, 10)
	assert(inactive == "Không phục vụ quân sự")
	checks += 1

	var active := RealmPanel.military_service_text({
		"active": true,
		"commander": 7,
		"start_day": 8,
		"end_day": 25,
		"combat_consent": false,
		"wage_per_day": 4,
		"escrow": 60,
	}, 10)
	assert(active.contains("chỉ huy #7"))
	assert(active.contains("còn 15 ngày"))
	assert(active.contains("lương 4 đ/ngày"))
	assert(active.contains("quỹ lương còn 60 đ"))
	assert(active.contains("Chưa đồng ý tham chiến"))
	assert(active.contains("có thể chọn từ ngày 8") == false)
	checks += 5
	var waiting := RealmPanel.military_service_text({
		"active": true, "commander": 7, "start_day": 8, "end_day": 25,
		"combat_consent": false, "wage_per_day": 4, "escrow": 60,
	}, 5)
	assert(waiting.contains("Chưa đồng ý tham chiến"))
	assert(waiting.contains("có thể chọn từ ngày 8"))
	checks += 2

	var consent_button := RealmPanel.military_combat_consent_button_text(false)
	assert(consent_button == "Tôi đồng ý tham chiến trong kỳ phục vụ này")
	assert(RealmPanel.military_combat_consent_button_text(true) == "Rút lại đồng ý tham chiến")
	checks += 2

	assert(RealmPanel.foreign_trade_policy_text(false) == "đang cho phép lệnh mới")
	assert(RealmPanel.foreign_trade_policy_text(true) == "đang tạm đình chỉ lệnh mới")
	assert(RealmPanel.foreign_trade_policy_button_text(0, false) == "Đình chỉ giao dịch với đối tác A")
	assert(RealmPanel.foreign_trade_policy_button_text(2, true) == "Khôi phục giao dịch với đối tác C")
	var trade_history := RealmPanel.foreign_trade_history_text({
		"completed_import_orders": 2, "imported_units": 5, "import_gross": 120,
		"completed_export_orders": 1, "exported_units": 4, "export_gross": 112,
	})
	assert(trade_history.contains("nhập 2 đơn, 5 DryFood, tổng tiền hàng trước thuế/phí 120 đ"))
	assert(trade_history.contains("xuất 1 đơn, 4 Salt, tổng tiền hàng trước thuế/phí 112 đ"))
	checks += 6
	var pending_trade_offer := RealmPanel.foreign_trade_offer_text({
		"id": "91", "partner_id": 1, "operation": 2, "quantity": 4,
		"created_day": 10, "due_day": 17, "status": 0, "held_cash": 108,
		"held_good": 4, "order_id": "0",
	}, 12)
	assert(pending_trade_offer.contains("đối tác B · nhập hàng khô · 4 đơn vị"))
	assert(pending_trade_offer.contains("đến hết ngày 17"))
	assert(pending_trade_offer.contains("đang giữ 108 đ và 4 đơn vị hàng"))
	var rejected_trade_offer := RealmPanel.foreign_trade_offer_text({
		"id": "92", "partner_id": 2, "operation": 5, "quantity": 1,
		"created_day": 3, "due_day": 10, "status": 2, "held_cash": 0,
		"held_good": 0, "order_id": "0",
	}, 12)
	assert(rejected_trade_offer.contains("đối tác C · xuất muối · 1 đơn vị"))
	assert(rejected_trade_offer.contains("bị từ chối; tài sản đã được trả lại"))
	checks += 5

	var offer := RealmPanel.military_service_offer_text(42, 4, 30)
	assert(offer.contains("cá thể #42"))
	assert(offer.contains("4 đ/ngày"))
	assert(offer.contains("30 ngày"))
	assert(offer.contains("giữ trước 120 đ"))
	checks += 4

	var roster := RealmPanel.military_unit_roster_text([{
		"id": "4294967303", "source_region": 0, "commander": 7,
		"members": [42, 55], "payroll_escrow": 80, "paid_total": 40,
		"mobilized_members": [42], "mobilized_until_day": 17,
	}], 0)
	assert(roster.contains("tập kết"))
	assert(roster.contains("chỉ huy #7 · 2 người"))
	assert(roster.contains("còn 80 đ lương · đã trả 40 đ"))
	assert(roster.contains("Đang trong kỳ tập kết: 1 người · hạn sớm nhất ngày 17"))
	checks += 4

	var supply := RealmPanel.military_supply_obligation_text([{
		"id": 9, "personnel_obligation_id": 8, "obligor": 7, "recipient": 3,
		"source_region": 0, "destination_region": 2, "due_day": 31,
		"required": 2, "delivered": 1, "completed_day": -1, "extensions": 1,
		"due_checked": true, "shortage_at_last_deadline": 1,
		"first_deadline_checked": true, "first_due_day": 24,
		"shortage_at_first_deadline": 2,
	}])
	assert(supply.contains("Cam kết quân nhu #9 cho nghĩa vụ tập kết #8"))
	assert(supply.contains("đã giao 1/2 phần lương khô"))
	assert(supply.contains("hạn đầu ngày 24 thiếu 2 phần"))
	assert(supply.contains("đã được gia hạn một lần"))
	checks += 4

	print("MILITARY, SUPPLY AND FOREIGN TRADE POLICY UI SMOKE: %d checks passed" % checks)
	quit(0)
