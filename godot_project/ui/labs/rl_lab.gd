extends Control

signal closed
const GOLD=Color("d1b575")
const PAPER=Color("e7e2cd")
const MUTED=Color("91a7a4")
const JADE=Color("74b7a5")
const GOODS=["Lương thực","Xơ sợi","Vải","Sắt","Nhiên liệu","Công cụ","Ngựa"]
var session: RefCounted
var seed: int=37
var running=false
var manifest: Dictionary={}
var evidence: Dictionary={}
var timer: Timer
var run_button: Button
var step_button: Button
var cash_label: Label
var region_label: Label
var progress_label: Label
var status_label: Label
var evidence_label: Label
var trace: RichTextLabel
var cargo_values: Array[Label]=[]
var price_labels: Array=[]
var column_labels: Array[Label]=[]
var body: Font=preload("res://fonts/BeVietnamPro-Regular.ttf")
var serif: Font=preload("res://fonts/Newsreader.ttf")

func _ready() -> void:
    set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    var background=ColorRect.new()
    background.color=Color("0b191f")
    background.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    add_child(background)
    _text(self,"PHÒNG THỬ NGHIỆM / REINFORCEMENT LEARNING",12,GOLD,Vector2(32,24))
    _text(self,"Một thương nhân. Sáu thị trường.",39,PAPER,Vector2(30,47),serif)
    _text(self,"Policy học bằng PPO · Suy luận CPU native · Một quyết định tiến bảy ngày mô phỏng",13,MUTED,Vector2(32,102))
    var close=_button(self,"←  Về địa đồ",func(): close_lab())
    close.name="Close"
    status_label=_text(self,"Đang nạp policy…",11,JADE,Vector2(32,136))
    var market=_panel("Market")
    _text(market,"GIÁ CHỢ  /  ĐỒNG BẠC MỖI ĐƠN VỊ",11,GOLD,Vector2(22,20))
    progress_label=_text(market,"Quyết định 00/64 · Ngày 000",13,PAPER,Vector2(22,48))
    var table=GridContainer.new()
    table.name="Table"
    table.columns=7
    table.position=Vector2(22,99)
    table.add_theme_constant_override("h_separation",12)
    table.add_theme_constant_override("v_separation",12)
    market.add_child(table)
    _cell(table,"MẶT HÀNG",11,MUTED,120)
    for name_value in ["Thanh\nĐiền","Hắc\nSơn","Bạch\nKhâu","Thiết\nLĩnh","Lâm\nGiang","Vân\nHải"]:
        column_labels.append(_cell(table,name_value,12,PAPER,90))
    for g in range(7):
        _cell(table,GOODS[g],13,PAPER,120)
        var row=[]
        for r in range(6): row.append(_cell(table,"—",17,MUTED,90))
        price_labels.append(row)
    var legend=_text(market,"Xanh: giá thấp nhất trong sáu chợ. Vàng: thị trường thương nhân đang ở.",10,MUTED,Vector2(22,410))
    legend.name="Legend"
    var account=_panel("Account")
    _text(account,"HỒ SƠ THƯƠNG NHÂN #1",11,MUTED,Vector2(22,20))
    cash_label=_text(account,"500 đồng",38,PAPER,Vector2(20,47),serif)
    region_label=_text(account,"—",14,JADE,Vector2(22,99))
    _text(account,"HÀNG TRÊN XE  /  SỨC CHỞ 16",10,GOLD,Vector2(22,138))
    for g in range(7):
        _text(account,GOODS[g],13,MUTED,Vector2(22,166+g*25))
        cargo_values.append(_text(account,"0",15,PAPER,Vector2(268,164+g*25)))
    _text(account,"QUYẾT ĐỊNH & HỆ QUẢ",11,GOLD,Vector2(22,363))
    trace=RichTextLabel.new()
    trace.position=Vector2(22,393)
    trace.size=Vector2(300,215)
    trace.add_theme_font_override("normal_font",body)
    trace.add_theme_font_size_override("normal_font_size",12)
    trace.add_theme_color_override("default_color",PAPER)
    trace.scroll_following=true
    account.add_child(trace)
    var evidence_panel=_panel("Evidence")
    _text(evidence_panel,"BẰNG CHỨNG HỌC  /  HOLDOUT TÁCH BIỆT",11,GOLD,Vector2(22,18))
    evidence_label=_text(evidence_panel,"",14,PAPER,Vector2(22,48))
    evidence_label.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
    _text(evidence_panel,"Curriculum giao thương. Chưa phải AI chiến dịch; quản trị và cư dân chưa được train.",11,MUTED,Vector2(22,114))
    var controls=_panel("Controls")
    run_button=_button(controls,"▶  Chạy phiên",toggle_run)
    run_button.position=Vector2(15,10)
    run_button.size=Vector2(158,42)
    step_button=_button(controls,"Một quyết định",step_once)
    step_button.position=Vector2(186,10)
    step_button.size=Vector2(166,42)
    var restart=_button(controls,"Phiên mới",func(): seed+=1;load_policy())
    restart.position=Vector2(365,10)
    restart.size=Vector2(130,42)
    _text(controls,"Space: chạy/dừng · Esc: về bản đồ · Rời cửa sổ luôn tạm dừng",11,MUTED,Vector2(522,22))
    timer=Timer.new()
    timer.wait_time=.25
    timer.timeout.connect(step_once)
    add_child(timer)
    resized.connect(_layout)
    _layout()
    load_policy()

func _panel(node_name: String) -> Panel:
    var p=Panel.new()
    p.name=node_name
    var style=StyleBoxFlat.new()
    style.bg_color=Color("14262b")
    style.border_color=Color("2b4144")
    style.set_border_width_all(1)
    style.set_corner_radius_all(12)
    p.add_theme_stylebox_override("panel",style)
    add_child(p)
    return p

func _text(parent: Node,text: String,font_size: int,color: Color,at: Vector2,font: Font=null) -> Label:
    var l=Label.new()
    l.text=text
    l.position=at
    l.add_theme_font_override("font",font if font!=null else body)
    l.add_theme_font_size_override("font_size",font_size)
    l.add_theme_color_override("font_color",color)
    l.mouse_filter=Control.MOUSE_FILTER_IGNORE
    parent.add_child(l)
    return l

func _cell(parent: Node,text: String,font_size: int,color: Color,width: int) -> Label:
    var l=_text(parent,text,font_size,color,Vector2.ZERO)
    l.custom_minimum_size=Vector2(width,24)
    return l

func _button(parent: Node,text: String,callback: Callable) -> Button:
    var b=Button.new()
    b.text=text
    b.mouse_default_cursor_shape=Control.CURSOR_POINTING_HAND
    b.add_theme_font_override("font",body)
    b.add_theme_font_size_override("font_size",13)
    b.add_theme_color_override("font_color",PAPER)
    for kind in ["normal","hover","pressed","focus"]:
        var s=StyleBoxFlat.new()
        s.bg_color=Color("24473f") if kind=="normal" else Color("34594b")
        s.border_color=JADE if kind=="normal" else GOLD
        s.set_border_width_all(1)
        s.set_corner_radius_all(8)
        b.add_theme_stylebox_override(kind,s)
    b.pressed.connect(callback)
    parent.add_child(b)
    return b

func _layout() -> void:
    if not has_node("Controls"): return
    get_node("Close").position=Vector2(size.x-213,33)
    get_node("Close").size=Vector2(181,40)
    get_node("Market").position=Vector2(24,174)
    get_node("Market").size=Vector2(size.x-424,442)
    get_node("Account").position=Vector2(size.x-376,174)
    get_node("Account").size=Vector2(352,size.y-274)
    trace.size.y=maxf(120,size.y-700)
    get_node("Evidence").position=Vector2(24,634)
    get_node("Evidence").size=Vector2(size.x-424,size.y-734)
    evidence_label.size=Vector2(size.x-470,70)
    get_node("Controls").position=Vector2(24,size.y-82)
    get_node("Controls").size=Vector2(size.x-48,62)

func load_policy() -> void:
    stop_running()
    var folder=ProjectSettings.globalize_path("res://models/")
    var parsed=JSON.parse_string(FileAccess.get_file_as_string(folder+"manifest.json"))
    if not parsed is Dictionary:
        _error("Không tìm thấy manifest của learned policy.")
        return
    manifest=parsed
    var report=JSON.parse_string(FileAccess.get_file_as_string(folder+"evaluation.json"))
    evidence=report if report is Dictionary else {}
    if not session.rl_load(folder+"economic.onnx",str(manifest.get("sha256","")),seed):
        _error("Không nạp được policy hoặc checksum không khớp. Phiên RL đã dừng.")
        return
    run_button.disabled=false
    step_button.disabled=false
    status_label.text="PPO ĐÃ TRAIN  ·  CPU / GRU64  ·  SEED THẾ GIỚI %d  ·  MODEL %s" % [seed,str(manifest.sha256).left(12)]
    status_label.add_theme_color_override("font_color",JADE)
    if not evidence.is_empty():
        var selected="learned_seed_"+str(int(evidence.selected_seed))
        var learned=float(evidence.evaluation[selected].mean_profit)
        var greedy=float(evidence.evaluation.greedy.mean_profit)
        evidence_label.text="%d tình huống test · Policy RL: %+0.1f đồng / phiên\nBaseline tham chiếu: %+0.1f đồng · Giữ tiền: 0 đồng\nCheckpoint chọn bằng validation; test không dùng để chọn model." % [evidence.test_scenarios,learned,greedy]
    refresh()

func refresh() -> void:
    var v: Dictionary=session.rl_snapshot()
    cash_label.text="%d đồng" % int(v.cash)
    region_label.text=str(v.markets[int(v.region)].name)
    progress_label.text="Quyết định %02d/64  ·  Ngày %03d  ·  %d lần suy luận" % [int(v.decision),int(v.day),int(v.inferences)]
    for r in range(6):
        column_labels[r].add_theme_color_override("font_color",GOLD if r==int(v.region) else PAPER)
    for g in range(7):
        cargo_values[g].text=str(v.cargo[g])
        var minimum=1000000
        for m in v.markets: minimum=mini(minimum,int(m.prices[g]))
        for r in range(6):
            var price=int(v.markets[r].prices[g])
            price_labels[g][r].text=str(price)
            price_labels[g][r].add_theme_color_override("font_color",JADE if price==minimum else (GOLD if r==int(v.region) else PAPER))
    var lines=PackedStringArray()
    for item in v.history:
        var action=int(item.action)
        var text="Chờ"
        if action>=1 and action<=7: text="Mua "+GOODS[action-1].to_lower()
        elif action>=8 and action<=14: text="Bán "+GOODS[action-8].to_lower()
        elif action>=15: text="Đi "+str(v.markets[action-15].name)
        lines.append("%02d  %s\n     Ngày %d · %d đồng · CPU %.2f ms" % [item.id,text,item.day,item.cash,item.inference_ms])
    trace.text="\n\n".join(lines) if lines.size()>0 else "Chưa có quyết định.\n\nBấm Một quyết định để xem policy\nchọn hành động và core thực hiện."
    if v.done:
        stop_running()
        step_button.disabled=true
        status_label.text="PHIÊN KẾT THÚC · TẤT TOÁN THEO GIÁ MUA THỰC · LỢI NHUẬN %+d ĐỒNG" % int(v.profit)

func step_once() -> void:
    var result: Dictionary=session.rl_step()
    if not result.get("success",false):
        stop_running()
        if not session.rl_snapshot().done: _error(str(result.get("error","Phiên RL đã dừng.")))
        return
    refresh()

func toggle_run() -> void:
    if running: stop_running()
    elif not run_button.disabled and not session.rl_snapshot().done:
        running=true
        run_button.text="Ⅱ  Tạm dừng"
        timer.start()

func stop_running() -> void:
    running=false
    if timer!=null: timer.stop()
    if run_button!=null: run_button.text="▶  Chạy phiên"

func _error(message: String) -> void:
    stop_running()
    status_label.text=message
    status_label.add_theme_color_override("font_color",Color("df9f85"))
    run_button.disabled=true
    step_button.disabled=true

func close_lab() -> void:
    stop_running()
    hide()
    closed.emit()

func _process(delta: float) -> void:
    if running and delta>.25: stop_running()

func _notification(what: int) -> void:
    if what==NOTIFICATION_APPLICATION_FOCUS_OUT or what==NOTIFICATION_WM_WINDOW_FOCUS_OUT:
        stop_running()
