extends Control

const MapCanvas = preload("res://ui/prototype/map_canvas.gd")
const RLLab = preload("res://ui/labs/rl_lab.gd")
const RealmPanel = preload("res://ui/realm/realm_panel.gd")
const CampaignPanel = preload("res://ui/campaign/campaign_panel.gd")
const INK = Color("0b191f")
const PANEL = Color("14262b")
const GOLD = Color("d1b575")
const PAPER = Color("e7e2cd")
const MUTED = Color("91a7a4")
const JADE = Color("74b7a5")
var body_font: Font = preload("res://fonts/BeVietnamPro-Regular.ttf")
var bold_font: Font = preload("res://fonts/BeVietnamPro-SemiBold.ttf")
var title_font: Font = preload("res://fonts/Newsreader.ttf")
var session: RefCounted
var map: Control
var date_label: Label
var state_label: Label
var play_button: Button
var title_label: Label
var region_tag: Label
var region_description: Label
var terrain_value: Label
var water_value: Label
var settlement_value: Label
var file_label: Label
var focus_note: Label
var help_shade: ColorRect
var speeds: Array[Button] = []
var last_selection: int = -1
var last_paused: bool = true
var help_open: bool = false
var app_active: bool = true
var rl_lab: Control
var realm_panel: Control
var campaign_panel: Control
var campaign_hidden: Dictionary = {}

func _ready() -> void:
    var app_theme = Theme.new()
    app_theme.default_font=body_font
    app_theme.default_font_size=14
    theme=app_theme
    session=ClassDB.instantiate("PolyEcSession")
    if session==null:
        push_error("PolyEc native extension could not be instantiated")
        return
    _build_ui()
    session.command("select",7)
    _refresh()
    resized.connect(_layout)
    _layout()

func _style(background: Color,border: Color,radius: int=9) -> StyleBoxFlat:
    var s=StyleBoxFlat.new()
    s.bg_color=background
    s.border_color=border
    s.set_border_width_all(1)
    s.set_corner_radius_all(radius)
    s.content_margin_left=14
    s.content_margin_right=14
    s.content_margin_top=9
    s.content_margin_bottom=9
    return s

func _label(parent: Node,text: String,font_size: int,color: Color,font: Font=null) -> Label:
    var label=Label.new()
    label.text=text
    label.add_theme_color_override("font_color",color)
    label.add_theme_font_size_override("font_size",font_size)
    if font!=null: label.add_theme_font_override("font",font)
    parent.add_child(label)
    label.mouse_filter=Control.MOUSE_FILTER_IGNORE
    return label

func _button(parent: Node,text: String,tip: String,callback: Callable) -> Button:
    var button=Button.new()
    button.text=text
    button.tooltip_text=tip
    button.mouse_default_cursor_shape=Control.CURSOR_POINTING_HAND
    button.add_theme_font_size_override("font_size",14)
    button.add_theme_color_override("font_color",PAPER)
    button.add_theme_color_override("font_hover_color",Color.WHITE)
    button.add_theme_color_override("font_pressed_color",GOLD)
    button.add_theme_stylebox_override("normal",_style(Color("162a2f"),Color("30484b")))
    button.add_theme_stylebox_override("hover",_style(Color("244045"),Color("728779")))
    button.add_theme_stylebox_override("pressed",_style(Color("2d4440"),GOLD))
    button.add_theme_stylebox_override("focus",_style(Color.TRANSPARENT,GOLD))
    button.pressed.connect(callback)
    parent.add_child(button)
    return button

func _panel(parent: Node,name_value: String) -> Panel:
    var panel=Panel.new()
    panel.name=name_value
    panel.add_theme_stylebox_override("panel",_style(PANEL,Color("2b4144"),12))
    parent.add_child(panel)
    return panel

func _build_ui() -> void:
    var bg=ColorRect.new()
    bg.color=INK
    bg.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    bg.mouse_filter=Control.MOUSE_FILTER_IGNORE
    add_child(bg)
    var top=Control.new()
    top.name="Top"
    add_child(top)
    var kicker=_label(top,"P O L Y E C   /   V Ư Ơ N G  T R I Ề U",11,GOLD,bold_font)
    kicker.position=Vector2(32,19)
    var name_label=_label(top,"Đại Bàn Cờ Vương Quyền",33,PAPER,title_font)
    name_label.position=Vector2(30,39)
    var subtitle=_label(top,"Đọc địa thế. Chọn miền đất. Bắt đầu quan sát.",12,MUTED)
    subtitle.position=Vector2(32,78)
    var campaign_button=_button(top,"M4 · Đường đèo & nguồn sống","Xử lý thiếu hụt, quyền đèo và nợ trên bản đồ native",_open_campaign)
    campaign_button.position=Vector2(655,0)
    campaign_button.size=Vector2(265,32)
    if session.has_method("rl_available") and session.rl_available():
        var rl_button=_button(top,"RL · Phòng thử nghiệm","Xem learned policy PPO trong sandbox sáu thị trường",_open_rl)
        rl_button.position=Vector2(655,75)
        rl_button.size=Vector2(239,30)
        var realm_button=_button(top,"Quốc gia · Sandbox","Thế giới với ba họ learned policy và cư dân độc lập",_open_realm)
        realm_button.position=Vector2(655,35)
        realm_button.size=Vector2(239,35)
    var prototype=_label(top,"NGUYÊN MẪU / RL LAB" if session.has_method("rl_available") and session.rl_available() else "NGUYÊN MẪU M0a",11,GOLD,bold_font)
    prototype.name="Prototype"
    date_label=_label(top,"Ngày 01 · Tháng 01 · Năm 01",16,PAPER,bold_font)
    date_label.name="Date"
    state_label=_label(top,"●  ĐANG TẠM DỪNG",11,JADE,bold_font)
    state_label.name="State"
    map=MapCanvas.new()
    map.name="Map"
    map.configure(session.atlas_map_data())
    add_child(map)
    map.territory_selected.connect(_select_territory)
    var map_frame=_panel(self,"MapBorder")
    map_frame.mouse_filter=Control.MOUSE_FILTER_IGNORE
    map_frame.add_theme_stylebox_override("panel",_style(Color.TRANSPARENT,Color("283c40"),10))
    var route_toggle=_button(self,"◇  Tuyến đường","Bật/tắt tuyến đường trên địa đồ",func():
        map.show_routes=not map.show_routes
        map.queue_redraw())
    route_toggle.name="RouteToggle"
    var inspector=_panel(self,"Inspector")
    var content=VBoxContainer.new()
    content.name="Content"
    content.add_theme_constant_override("separation",12)
    inspector.add_child(content)
    _label(content,"HỒ SƠ LÃNH ĐỊA",11,MUTED,bold_font)
    title_label=_label(content,"Thanh Điền",39,PAPER,title_font)
    region_tag=_label(content,"ĐỒNG BẰNG PHÍA TÂY",10,JADE,bold_font)
    var divider=HSeparator.new()
    var line_style=StyleBoxLine.new()
    line_style.color=Color("334b4b")
    line_style.thickness=1
    divider.add_theme_stylebox_override("separator",line_style)
    content.add_child(divider)
    region_description=_label(content,"",13,Color("bdccc4"))
    region_description.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
    region_description.custom_minimum_size.y=80
    _label(content,"ĐỊA THẾ & ĐƯỜNG ĐI",11,GOLD,bold_font)
    terrain_value=_fact(content,"ĐỊA HÌNH")
    water_value=_fact(content,"THỦY HỆ")
    settlement_value=_fact(content,"ĐỊA DANH")
    var spacer=Control.new()
    spacer.custom_minimum_size.y=5
    content.add_child(spacer)
    var note=_panel(content,"Note")
    note.custom_minimum_size.y=84
    var note_text=_label(note,"Hai mươi bảy miền đất trong Atlas.\nChọn vùng để xem tên, địa thế và\nđặc sản đã ghi nhận.",11,MUTED)
    note_text.position=Vector2(14,13)
    note_text.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
    note_text.size=Vector2(254,64)
    file_label=_label(content,"HỒ SƠ 007  /  LÃNH ĐỊA",10,MUTED)
    var bottom=_panel(self,"Bottom")
    play_button=_button(bottom,"▶  Tiếp tục","Space · Chạy hoặc tạm dừng đồng hồ",_toggle_pause)
    play_button.name="Play"
    play_button.position=Vector2(16,14)
    play_button.size=Vector2(160,44)
    for speed in [1,2,4]:
        var b=_button(bottom,str(speed)+"×","Một ngày mỗi giây ở 1×",func(): _set_speed(speed))
        b.toggle_mode=true
        b.name="Speed"+str(speed)
        b.position=Vector2(193+speeds.size()*59,14)
        b.size=Vector2(49,44)
        speeds.append(b)
    var rule=_label(bottom,"THỜI GIAN THẾ GIỚI",10,MUTED)
    rule.name="ClockCaption"
    focus_note=_label(bottom,"Một ngày / giây ở tốc độ 1×",12,PAPER)
    focus_note.name="ClockNote"
    var reset=_button(bottom,"↖  Toàn cảnh","Home · Đưa bản đồ về vị trí ban đầu",func(): map.reset_view())
    reset.name="Reset"
    var help_button=_button(bottom,"F1  Hướng dẫn","Điều khiển và thuật ngữ · Mở sẽ tạm dừng",_open_help)
    help_button.name="Help"
    var footer=_label(self,"THANH ĐIỀN  /  HẮC SƠN     —     Địa đồ thử nghiệm · Các hệ kinh tế và RL thuộc mốc sau",10,Color("738c89"))
    footer.name="Footer"
    _build_help()

func _fact(parent: Node,caption: String) -> Label:
    var row=VBoxContainer.new()
    row.add_theme_constant_override("separation",4)
    parent.add_child(row)
    _label(row,caption,9,MUTED,bold_font)
    var value=_label(row,"",13,PAPER)
    value.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
    return value

func _build_help() -> void:
    help_shade=ColorRect.new()
    help_shade.name="HelpShade"
    help_shade.color=Color(0.01,0.025,0.03,0.84)
    help_shade.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    add_child(help_shade)
    var card=_panel(help_shade,"Card")
    card.size=Vector2(650,540)
    var content=VBoxContainer.new()
    content.position=Vector2(34,28)
    content.size=Vector2(582,450)
    content.add_theme_constant_override("separation",19)
    card.add_child(content)
    _label(content,"SỔ TAY VƯƠNG TRIỀU",11,GOLD,bold_font)
    _label(content,"Bắt đầu từ địa đồ",36,PAPER,title_font)
    var intro=_label(content,"Khám phá hai lãnh địa và quan sát thời gian của thế giới.\nĐây là nguyên mẫu nền tảng M0a, chưa có kinh tế hay NPC RL.",14,MUTED)
    intro.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
    _label(content,"Chuột trái               Chọn lãnh địa và mở hồ sơ\nCuộn chuột              Thu / phóng tại vị trí con trỏ\nKéo giữa hoặc phải    Di chuyển địa đồ\nHome                       Trở về toàn cảnh\nSpace                       Tiếp tục / tạm dừng\nF1 hoặc Esc              Mở / đóng sổ tay",14,PAPER)
    var terms=_label(content,"Lãnh địa: một miền đất có hồ sơ riêng.\nNgày game: một bước mô phỏng; 1× bằng một ngày mỗi giây.\nLịch giả tưởng: 30 ngày/tháng, 360 ngày/năm.",12,MUTED)
    terms.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
    _label(content,"Mở sổ tay hoặc rời cửa sổ sẽ tạm dừng.\nĐóng sổ tay không tự tiếp tục đồng hồ.",12,JADE)
    var close=_button(card,"Đã hiểu · Quay lại địa đồ","Esc · Giữ thế giới tạm dừng",_close_help)
    close.position=Vector2(34,465)
    close.size=Vector2(582,44)
    help_shade.hide()

func _layout() -> void:
    if map==null: return
    var w=size.x
    var h=size.y
    map.position=Vector2(24,117)
    map.size=Vector2(w-394,h-233)
    var map_border=get_node("MapBorder")
    map_border.position=map.position
    map_border.size=map.size
    get_node("RouteToggle").position=Vector2(w-570,132)
    get_node("RouteToggle").size=Vector2(152,36)
    var inspector=get_node("Inspector")
    inspector.position=Vector2(w-350,117)
    inspector.size=Vector2(326,h-233)
    var content=inspector.get_node("Content")
    content.position=Vector2(22,24)
    content.size=Vector2(282,h-270)
    get_node("Top/Prototype").position=Vector2(w-209,22)
    date_label.position=Vector2(w-355,47)
    state_label.position=Vector2(w-205,79)
    var bottom=get_node("Bottom")
    bottom.position=Vector2(24,h-99)
    bottom.size=Vector2(w-48,72)
    bottom.get_node("ClockCaption").position=Vector2(398,14)
    focus_note.position=Vector2(398,34)
    bottom.get_node("Reset").position=Vector2(w-421,14)
    bottom.get_node("Reset").size=Vector2(143,44)
    bottom.get_node("Help").position=Vector2(w-262,14)
    bottom.get_node("Help").size=Vector2(196,44)
    get_node("Footer").position=Vector2(30,h-20)
    help_shade.get_node("Card").position=(size-Vector2(650,540))*0.5

func _select_territory(id: int) -> void:
    session.command("select",id)
    _refresh()

func _set_speed(speed: int) -> void:
    session.set_speed(float(speed))
    _refresh()

func _toggle_pause() -> void:
    if help_open: return
    session.command("toggle_pause")
    _refresh()

func _open_help() -> void:
    session.command("pause")
    help_open=true
    help_shade.show()
    help_shade.get_node("Card").get_child(help_shade.get_node("Card").get_child_count()-1).grab_focus()
    _refresh()

func _close_help() -> void:
    help_open=false
    help_shade.hide()
    play_button.grab_focus()
    _refresh()

func _input(event: InputEvent) -> void:
    if not event is InputEventKey or not event.pressed or event.echo: return
    if campaign_panel!=null and campaign_panel.visible:
        var focused=get_viewport().gui_get_focus_owner()
        if (focused is LineEdit or focused is TextEdit) and event.keycode not in [KEY_ESCAPE,KEY_F1]: return
        if event.keycode==KEY_ESCAPE: campaign_panel.close_panel()
        elif event.keycode==KEY_SPACE: campaign_panel.toggle_run()
        elif event.keycode==KEY_F1: campaign_panel.show_guide()
        elif event.keycode==KEY_HOME: campaign_panel.map.reset_view()
        else: return
        get_viewport().set_input_as_handled()
        return
    if realm_panel!=null and realm_panel.visible:
        if event.keycode==KEY_ESCAPE: realm_panel.close_panel();session.command("pause")
        elif event.keycode==KEY_SPACE: realm_panel.toggle_run()
        elif event.keycode==KEY_F1: realm_panel.show_guide()
        get_viewport().set_input_as_handled()
        return
    if rl_lab!=null and rl_lab.visible:
        if event.keycode==KEY_ESCAPE: rl_lab.close_lab()
        elif event.keycode==KEY_SPACE: rl_lab.toggle_run()
        get_viewport().set_input_as_handled()
        return
    if event.keycode==KEY_F1:
        if help_open: _close_help()
        else: _open_help()
        get_viewport().set_input_as_handled()
    elif event.keycode==KEY_ESCAPE and help_open:
        _close_help()
        get_viewport().set_input_as_handled()
    elif event.keycode==KEY_SPACE:
        _toggle_pause()
        get_viewport().set_input_as_handled()
    elif event.keycode==KEY_HOME:
        map.reset_view()
        get_viewport().set_input_as_handled()

func _notification(what: int) -> void:
    if session==null: return
    if what==NOTIFICATION_APPLICATION_FOCUS_OUT or what==NOTIFICATION_WM_WINDOW_FOCUS_OUT:
        if campaign_panel!=null and campaign_panel.visible:
            campaign_panel.pause_core_and_stop_ui()
        elif realm_panel!=null:
            realm_panel.pause_core_and_stop_ui()
        else:
            session.realm_pause()
        app_active=false
        session.set_active(false)
    elif what==NOTIFICATION_APPLICATION_FOCUS_IN or what==NOTIFICATION_WM_WINDOW_FOCUS_IN:
        app_active=true
        session.set_active(true)

func _process(_delta: float) -> void:
    if session==null: return
    if campaign_panel!=null and campaign_panel.visible: return
    session.tick()
    _refresh()

func _refresh() -> void:
    if date_label==null: return
    var view: Dictionary=session.snapshot()
    var day=int(view.get("day",0))
    var day_in_month=day%30+1
    var month=(day%360)/30+1
    var year=day/360+1
    date_label.text="Ngày %02d · Tháng %02d · Năm %02d" % [day_in_month,month,year]
    var paused=bool(view.get("paused",true))
    play_button.text="▶  Tiếp tục" if paused else "Ⅱ  Tạm dừng"
    play_button.add_theme_stylebox_override("normal",_style(Color("24473f") if paused else Color("303d34"),JADE if paused else GOLD))
    state_label.text="●  ĐANG TẠM DỪNG" if paused else "●  THẾ GIỚI ĐANG CHẠY"
    state_label.add_theme_color_override("font_color",JADE if paused else GOLD)
    var speed=int(view.get("speed",1))
    for i in range(speeds.size()): speeds[i].set_pressed_no_signal(speed==[1,2,4][i])
    var id=int(view.get("selected",0))
    map.set_selected(id)
    if id!=last_selection:
        last_selection=id
        _refresh_inspector(id,view)
    last_paused=paused

func _open_rl() -> void:
    session.command("pause")
    if rl_lab==null:
        rl_lab=RLLab.new()
        rl_lab.session=session
        add_child(rl_lab)
    else:
        rl_lab.show()
    _refresh()

func _open_realm() -> void:
    session.command("pause")
    if realm_panel==null:
        realm_panel=RealmPanel.new()
        realm_panel.session=session
        add_child(realm_panel)
    else:
        realm_panel.show()
    _refresh()

func _open_campaign() -> void:
    session.command("pause")
    if campaign_panel==null:
        campaign_panel=CampaignPanel.new()
        campaign_panel.session=session
        campaign_panel.closed.connect(_close_campaign)
        add_child(campaign_panel)
    else:
        campaign_panel.show()
        campaign_panel.refresh()
    # Stop rendering the older map and rebuilding its button styles underneath
    # the full-screen M4 view. Keep each prior visibility state for navigation.
    for child in get_children():
        if child is CanvasItem and child!=campaign_panel:
            campaign_hidden[child]=child.visible
            child.hide()

func _close_campaign() -> void:
    for child in campaign_hidden:
        if is_instance_valid(child): child.visible=campaign_hidden[child]
    campaign_hidden.clear()
    _refresh()

func _refresh_inspector(id: int,view: Dictionary) -> void:
    var territory: Dictionary=map.region_for_id(id)
    if territory.is_empty():
        title_label.text="Chọn một lãnh địa"
        region_tag.text="ĐỊA ĐỒ VƯƠNG QUỐC"
        region_description.text="Chọn một miền đất trên bản đồ để xem thông tin được ghi trong Atlas."
        terrain_value.text="—";water_value.text="—";settlement_value.text="—"
        return
    title_label.text=str(territory.get("name", "Lãnh địa"))
    file_label.text="ATLAS %02d  /  %s" % [id, "KHU TRIỀU ĐÌNH" if territory.get("royal", false) else "LÃNH ĐỊA"]
    region_tag.text=str(territory.get("group", "")).to_upper()
    region_description.text=str(territory.get("description", ""))
    terrain_value.text="Được xếp vào nhóm: "+str(territory.get("group", "chưa phân nhóm"))
    water_value.text="Dữ liệu địa hình chi tiết chưa được hiệu chuẩn"
    settlement_value.text="Đặc sản ghi trong Atlas: "+str(territory.get("specialty", "chưa ghi nhận"))
