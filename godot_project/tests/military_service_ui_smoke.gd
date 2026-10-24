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

	var pending_raid := RealmPanel.military_raid_text({
		"partner_id": 0, "force_units": 2, "status": 0,
	})
	assert(pending_raid.contains("Đối tác A đã đưa 2 đơn vị tới Nam Cảng"))
	assert(pending_raid.contains("đang chờ Lãnh chủ trả lời"))
	var repelled_raid := RealmPanel.military_raid_text({
		"partner_id": 1, "force_units": 3, "status": 1,
		"attacker_losses": 3, "defender_casualties": 0, "supplied_defenders": [21, 22], "defense_score": 5,
	})
	assert(repelled_raid.contains("Đối tác B bị đẩy lui"))
	assert(repelled_raid.contains("3 đơn vị bị mất"))
	assert(repelled_raid.contains("Điểm phòng thủ 5"))
	assert(repelled_raid.contains("mỗi người mang Leather cá nhân được thêm 1 điểm và không mất món đó"))
	assert(repelled_raid.contains("2 người đã dùng một quân nhu cá nhân"))
	var successful_raid := RealmPanel.military_raid_text({
		"partner_id": 2, "force_units": 4, "status": 2,
		"defender_casualties": 2, "salt_looted": 3, "supplied_defenders": [21], "defense_score": 2,
	})
	assert(successful_raid.contains("Đối tác C vượt qua phòng thủ"))
	assert(successful_raid.contains("2 tình nguyện viên bị đánh gục"))
	assert(successful_raid.contains("1 người đã dùng một quân nhu cá nhân"))
	assert(successful_raid.contains("lấy 3 Salt"))
	var withdrawn_raid := RealmPanel.military_raid_text({
		"partner_id": 0, "force_units": 2, "status": 3, "salt_looted": 1,
	})
	assert(withdrawn_raid.contains("Lãnh chủ rút quân trước đợt đột kích của đối tác A"))
	assert(withdrawn_raid.contains("đối phương lấy 1 Salt"))
	checks += 8
	var local_census := RealmPanel.urban_census_text([{
		"site_id": 8, "site_name": "Nam Cảng", "urban_residents": 20,
		"housing_capacity": 24, "unhoused_residents": 0, "available_beds": 4,
		"rural_residents": 80, "unknown_residence": 3,
	}], false)
	assert(local_census.contains("Nam Cảng · đô thị 20 · sức chứa 24 · còn thiếu chỗ 0 · giường trống 4 · nông thôn 80 · chưa rõ 3"))
	assert(local_census.contains("Chỉ hiện vùng cư trú của cá thể đang xem."))
	assert(local_census.contains("save cũ chưa lưu địa chỉ"))
	var sovereign_census := RealmPanel.urban_census_text([
		{"site_id": 0, "site_name": "Long Phố", "urban_residents": 1},
		{"site_id": 1, "site_name": "Hà Trạch", "urban_residents": 2},
	], true)
	assert(sovereign_census.contains("Long Phố") and sovereign_census.contains("Hà Trạch"))
	assert(not sovereign_census.contains("Chỉ hiện vùng cư trú"))
	assert(RealmPanel.urban_census_text([], true).is_empty())
	checks += 6
	assert(RealmPanel.GOODS[21] == "thuốc")
	assert(RealmPanel.ESTABLISHMENT_NAMES[15] == "vườn dược liệu")
	assert(RealmPanel.ESTABLISHMENT_NAMES[16] == "cơ sở ép và sấy hạt")
	assert(RealmPanel.fact_kind_name(237) == "Tự dùng Medicine để điều trị")
	checks += 4

	print("MILITARY, SUPPLY, TRADE, URBAN CENSUS AND MEDICINE UI SMOKE: %d checks passed" % checks)
	quit(0)
