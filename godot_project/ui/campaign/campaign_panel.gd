extends Control

signal closed

const CampaignMap = preload("res://ui/campaign/campaign_map.gd")
const EVENT_NAMES = {200:"Lương thực xuất kho",201:"Thu hoạch giảm do thời tiết",202:"Đoàn hàng đã tới",204:"Khoản vay đầu kỳ",205:"Cấp vốn đường đèo",206:"Thỏa thuận quyền đèo",207:"Phân xử · bất mãn tăng",208:"Cử người xác minh",209:"Thanh toán cho chủ nợ",210:"Cấp vốn và nhận thợ dệt",211:"Công trình vướng quyền đèo",212:"Đường đèo hoàn thành",213:"Chứng cứ đã về",214:"Xưởng dệt xuất vải",215:"Mở hồ sơ từ thiếu hụt/nghĩa vụ",216:"Đóng hồ sơ · điều kiện đã ổn",217:"Kiểm kê đầu kỳ",218:"Quyền thu phí đèo được ghi nhận",219:"Nợ đến hạn chưa thanh toán"}
var session: RefCounted
var running: bool = false
var selected: int = 1
var elapsed: float = 0.0
var speed: int = 1
var snapshot: Dictionary = {}
var map: Control
var heading: Label
var profile: RichTextLabel
var journal: RichTextLabel
var route_label: Label
var feedback: Label
var run_button: Button
var quantity: SpinBox
var case_label: Label
var help_dialog: AcceptDialog
var layer_select: OptionButton
var save_path: String = ProjectSettings.globalize_path("user://m4-save.polyec")

func style(color: String) -> StyleBoxFlat:
    var s = StyleBoxFlat.new()
    s.bg_color = Color(color)
    s.set_corner_radius_all(7)
    s.content_margin_left = 12
    s.content_margin_right = 12
    s.content_margin_top = 8
    s.content_margin_bottom = 8
    return s

func label(parent: Node, text_value: String, font_size: int = 14) -> Label:
    var l = Label.new()
    l.text = text_value
    l.add_theme_font_size_override("font_size", font_size)
    parent.add_child(l)
    return l

func button(parent: Node, text_value: String, callback: Callable) -> Button:
    var b = Button.new()
    b.text = text_value
    b.custom_minimum_size.y = 34
    b.focus_mode = Control.FOCUS_NONE
    b.add_theme_stylebox_override("normal", style("24453f"))
    b.add_theme_stylebox_override("hover", style("345c50"))
    b.add_theme_stylebox_override("pressed", style("456d58"))
    b.pressed.connect(callback)
    parent.add_child(b)
    return b

func _ready() -> void:
    set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    var background = Panel.new()
    background.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    background.add_theme_stylebox_override("panel", style("0d2025"))
    add_child(background)
    var margin = MarginContainer.new()
    margin.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    for side in ["left", "right", "top", "bottom"]:
        margin.add_theme_constant_override("margin_" + side, 18)
    add_child(margin)
    var page = VBoxContainer.new()
    page.add_theme_constant_override("separation", 10)
    margin.add_child(page)
    var header = HBoxContainer.new()
    page.add_child(header)
    var title = label(header, "ĐƯỜNG ĐÈO & NGUỒN SỐNG", 27)
    title.add_theme_font_override("font", preload("res://fonts/Newsreader.ttf"))
    title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    button(header, "F1 · Hướng dẫn", show_guide)
    button(header, "← Địa đồ", close_panel)
    label(page, "M4 · Lát cắt kiểm chứng · Controller kiểm thử cho việc thường lệ; chưa phải NPC RL hoàn chỉnh", 12)
    heading = label(page, "Đang mở sổ…", 15)
    var toolbar = HBoxContainer.new()
    toolbar.add_theme_constant_override("separation", 8)
    page.add_child(toolbar)
    run_button = button(toolbar, "▶ Tiếp tục", toggle_run)
    button(toolbar, "+1 ngày", step_day)
    var speeds = OptionButton.new()
    for value in [1, 2, 4]:
        speeds.add_item("%d×" % value, value)
    speeds.item_selected.connect(func(index): speed = [1, 2, 4][index]; elapsed = 0.0)
    toolbar.add_child(speeds)
    button(toolbar, "Lưu", save_game)
    button(toolbar, "Tải", load_game)
    var layer = OptionButton.new()
    layer_select = layer
    for text_value in ["Địa hình & tuyến", "Dự trữ lương thực", "Hồ sơ đã báo cáo"]:
        layer.add_item(text_value)
    layer.item_selected.connect(func(index): map.overlay = index; map.queue_redraw())
    toolbar.add_child(layer)
    button(toolbar, "Đặt lại bản đồ", func(): map.reset_view())
    var body = HBoxContainer.new()
    body.size_flags_vertical = Control.SIZE_EXPAND_FILL
    body.add_theme_constant_override("separation", 14)
    page.add_child(body)
    var left = VBoxContainer.new()
    left.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    left.size_flags_stretch_ratio = 2.1
    body.add_child(left)
    map = CampaignMap.new()
    map.custom_minimum_size = Vector2(480, 280)
    map.size_flags_vertical = Control.SIZE_EXPAND_FILL
    map.territory_selected.connect(select_region)
    left.add_child(map)
    label(left, "Nâu: quyền thu phí · Vàng: công trình/đoàn hàng · Sáng: tuyến dự kiến · Sông: cần cầu", 11)
    route_label = label(left, "", 12)
    route_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    label(left, "SỔ VIỆC · báo cáo trễ 2 ngày · # là căn cứ nối sự việc", 12)
    journal = RichTextLabel.new()
    journal.custom_minimum_size.y = 130
    journal.scroll_following = true
    left.add_child(journal)
    var right_scroll = ScrollContainer.new()
    right_scroll.custom_minimum_size.x = 370
    right_scroll.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    body.add_child(right_scroll)
    var right = VBoxContainer.new()
    right.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    right.add_theme_constant_override("separation", 7)
    right_scroll.add_child(right)
    profile = RichTextLabel.new()
    profile.custom_minimum_size.y = 145
    right.add_child(profile)
    case_label = label(right, "", 12)
    case_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    label(right, "CẤP LƯƠNG THỰC TỪ KHO HOÀNG GIA", 12)
    quantity = SpinBox.new()
    quantity.min_value = 1
    quantity.max_value = 5000
    quantity.value = 500
    quantity.step = 1
    quantity.value_changed.connect(func(_value): refresh_route())
    right.add_child(quantity)
    button(right, "Xuất kho đến vùng đang chọn", func(): send_command(0, selected, int(quantity.value)))
    label(right, "QUYỀN ĐÈO HẮC SƠN · NHIỀU CÁCH XỬ LÝ", 12)
    button(right, "Thỏa thuận quyền đèo · 120 đ", func(): send_command(2, 1))
    button(right, "Xác minh hồ sơ · 30 đ / 3 ngày", func(): send_command(4, 1))
    button(right, "Phân xử theo chứng cứ · bất mãn +10", func(): send_command(3, 1))
    button(right, "Xây đường · 300 đ + sắt/công cụ", func(): send_command(1, 1))
    button(right, "Thanh toán nợ Hắc Sơn cho chủ nợ", func(): send_command(5, 1))
    label(right, "CƠ SỞ SẢN XUẤT · BẠCH KHÂU", 12)
    button(right, "Mở xưởng dệt · 200 đ + công cụ", func(): send_command(6, 2))
    feedback = label(page, "Space chạy/dừng · Home đặt lại bản đồ · Rời cửa sổ sẽ dừng", 12)
    feedback.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    help_dialog = AcceptDialog.new()
    help_dialog.title = "M4 · Quyền, kho và hệ quả"
    help_dialog.dialog_text = "Chọn vùng trên bản đồ. Lớp dự trữ cho biết số ngày kho còn đủ.\nHồ sơ mở từ thiếu hụt, nợ quá hạn hoặc công trình mắc quyền, báo cáo trễ hai ngày.\nHắc Sơn: thỏa thuận quyền đèo hoặc xác minh rồi phân xử. Phân xử tăng bất mãn.\nXây đường cần tiền, sắt, công cụ và 12 ngày; quyền chưa giải quyết có thể làm đình trệ.\nCấp lương thực tốn tiền mua và vận tải. Hàng nằm trên tuyến trước khi tới, không dịch chuyển tức thời.\nThanh toán nợ trả đúng chủ nợ. Hồ sơ đóng khi đủ 21 ngày lương thực, không nợ quá hạn/công trình mắc quyền.\nXưởng dệt Bạch Khâu dùng thợ, xơ, nhiên liệu, công cụ và vốn; có lương và bán hàng cho thị trường có tiền.\nSpace không chạy khi đang nhập số. Pause, mất focus và tải game đều giữ lịch đứng.\nĐây là điều kiện khởi tạo M4 và controller kiểm thử có nhãn, không phải một campaign RL hoàn chỉnh."
    add_child(help_dialog)
    if not session.m4_start(37, 10000):
        feedback.text = "Không khởi tạo được core M4"
    running = false
    elapsed = 0.0
    refresh()

func select_region(id: int) -> void:
    selected = id
    map.selected = id
    refresh()

func refresh() -> void:
    snapshot = session.m4_snapshot()
    if not snapshot.get("loaded", false):
        return
    var core_paused = bool(snapshot.get("paused", true))
    heading.text = "Ngày %d · %s · %d / %d người còn sống · Ngân khố %d đ" % [snapshot.day, "ĐANG CHẠY" if running and not core_paused else "TẠM DỪNG", snapshot.alive, snapshot.population, snapshot.crown]
    run_button.text = "Ⅱ Tạm dừng" if running and not core_paused else "▶ Tiếp tục"
    map.update_world(snapshot)
    var r = snapshot.regions[selected]
    profile.text = "%s\n%s\n%d cư dân · %d lương thực / khoảng %d ngày\nGiá lương thực %d đ · Ngân khố vùng %d đ\nBất mãn %d · Năng suất địa hình %d%%\nHắc Sơn: còn nợ %d đ%s\nDệt Bạch Khâu: %d thợ · %d vải · vốn %d đ" % [r.name, r.terrain, r.residents, r.food, r.cover, r.price, r.treasury, r.unrest, r.harvest, snapshot.debt, " · quá hạn" if snapshot.overdue else "", snapshot.workshop_workers, snapshot.cloth, snapshot.workshop_cash]
    var text_value = "Hồ sơ đã báo cáo:\n"
    for k in snapshot.cases:
        if k.region == selected:
            text_value += "#%d · %s%s\n" % [k.id, "Đã đóng ngày %d" % k.closed if k.closed >= 0 else "Đang mở", " · có chứng cứ" if k.evidence else ""]
    if snapshot.inspecting_until > 0:
        text_value += "Người xác minh về ngày %d\n" % snapshot.inspecting_until
    if snapshot.routes[0].works_due > 0:
        text_value += "Đường đèo dự kiến ngày %d%s" % [snapshot.routes[0].works_due, " · đang vướng quyền" if snapshot.routes[0].claimed else ""]
    case_label.text = text_value
    journal.clear()
    var start = maxi(0, snapshot.journal.size() - 30)
    for i in range(start, snapshot.journal.size()):
        var f = snapshot.journal[i]
        journal.append_text("Ngày %d · #%d ← #%d · %s · %s · %d\n" % [f.day, f.id, f.parent, snapshot.regions[f.region].name, EVENT_NAMES.get(int(f.kind), "Sự việc"), f.amount])
    for c in snapshot.cargo:
        if not c.delivered:
            journal.append_text("Đoàn #%d: %d lương thực → %s · tới ngày %d\n" % [c.id, c.quantity, snapshot.regions[c.to].name, c.arrives])
    refresh_route()

func refresh_route() -> void:
    if snapshot.is_empty() or quantity == null:
        return
    var p = session.m4_preview(6, selected, int(quantity.value))
    map.route_path = p.get("path", [])
    map.queue_redraw()
    route_label.text = "Kho hoàng gia → %s · %d ngày vận tải · phí %d đ + tiền mua · lượng hàng và đèo/cầu ảnh hưởng thời gian" % [snapshot.regions[selected].name, p.get("days", 0), p.get("fee", 0)]

func pause_core_and_stop_ui() -> bool:
    running = false
    elapsed = 0.0
    if session.m4_pause():
        return true
    var error = str(session.m4_snapshot().get("error", ""))
    feedback.text = "Không tạm dừng được lõi Campaign" + (": " + error if error != "" else "")
    return false

func send_command(action: int, target: int, amount: int = 200) -> void:
    if not pause_core_and_stop_ui():
        refresh()
        return
    var ok = session.m4_command(action, target, amount, snapshot.sequence, snapshot.revision)
    feedback.text = "Đã ghi lệnh vào sổ; hãy tiến thời gian để thấy hệ quả" if ok else str(session.m4_snapshot().error)
    refresh()

func step_day() -> void:
    if not pause_core_and_stop_ui():
        refresh()
        return
    if not session.m4_step_one_day():
        feedback.text = "Không tiến ngày: " + str(session.m4_snapshot().get("error", ""))
    else:
        feedback.text = "Đã tiến đúng một ngày; lịch vẫn tạm dừng"
    refresh()

func toggle_run() -> void:
    if help_dialog.visible:
        return
    elapsed = 0.0
    if running:
        pause_core_and_stop_ui()
        refresh()
        return
    if session.m4_resume():
        running = true
        feedback.text = "Đang chạy; rời cửa sổ hoặc khung hình dài sẽ tạm dừng"
    else:
        running = false
        feedback.text = "Không tiếp tục được: " + str(session.m4_snapshot().get("error", ""))
    refresh()

func advance_automatic_day() -> void:
    if not running:
        return
    if session.m4_paused():
        running = false
        elapsed = 0.0
        feedback.text = "Lõi Campaign đang tạm dừng; hãy tiếp tục trước"
    elif not session.m4_step(1):
        var error = str(session.m4_snapshot().get("error", ""))
        running = false
        elapsed = 0.0
        session.m4_pause()
        feedback.text = "Không tiến ngày: " + error
    refresh()

func save_game() -> void:
    if not pause_core_and_stop_ui():
        refresh()
        return
    feedback.text = "Đã lưu; bản cũ giữ trong .bak" if session.m4_save(save_path, false) else str(session.m4_snapshot().error)
    refresh()

func load_game() -> void:
    if not pause_core_and_stop_ui():
        refresh()
        return
    feedback.text = "Đã tải; đang tạm dừng" if session.m4_save(save_path, true) else "Tải thất bại; giữ world hiện tại: " + str(session.m4_snapshot().error)
    refresh()

func show_guide() -> void:
    if not pause_core_and_stop_ui():
        refresh()
        return
    refresh()
    help_dialog.popup_centered(Vector2i(850, 450))

func close_panel() -> void:
    if not pause_core_and_stop_ui():
        refresh()
        return
    help_dialog.hide()
    hide()
    closed.emit()

func _process(delta: float) -> void:
    if not visible or not running:
        return
    if delta > 0.25:
        if pause_core_and_stop_ui():
            feedback.text = "Khung hình quá dài; đã tạm dừng"
        refresh()
        return
    if session.m4_paused():
        running = false
        elapsed = 0.0
        feedback.text = "Lõi Campaign đang tạm dừng; hãy tiếp tục trước"
        refresh()
        return
    elapsed += delta * speed
    if elapsed >= 1.0:
        elapsed -= 1.0
        advance_automatic_day()

func _notification(what: int) -> void:
    if what in [NOTIFICATION_APPLICATION_FOCUS_OUT, NOTIFICATION_WM_WINDOW_FOCUS_OUT]:
        if is_node_ready():
            pause_core_and_stop_ui()
            refresh()
