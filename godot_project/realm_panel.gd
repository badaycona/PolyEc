extends Control

const NAMES=["Thanh Điền","Hắc Sơn","Bạch Khâu","Thiết Lĩnh","Lâm Giang","Vân Hải","Đất triều đình"]
const CARDS=["Sổ Bộ Chuẩn Mực","Lò Đào Tạo Hoàng Gia","Người Đứng Giữa","Kho Thóc Tiền Triều","Liên Minh Công Thương","Đoàn Công Trình Hoàng Gia","Môn Sinh Khắp Chốn","Mạng Lưới Thư Trạm","Ngân Quỹ Kiến Quốc","Hiến Ước Liên Lãnh","Học Viện Bách Nghệ","Hội Đồng Khai Quốc"]
const ACTION_NAMES=["Chờ","Mua","Bán","Vận tải","Vay","Trả nợ","Dự án","Tuyển dụng","Thuế","Cứu trợ","Điều tra","Đề nghị","Chấp thuận","Từ chối","Làm việc","Học","Nghỉ","Giao lưu","Tham gia hội","Di cư","Giúp gia đình","Bổ nhiệm","Huy động"]
const GOODS=["lương thực","xơ sợi","vải","sắt","nhiên liệu","công cụ","ngựa"]
var session: RefCounted
var running=false
var actor=1
var status: Label
var summary: Label
var region_rows: Array=[]
var id_input: SpinBox
var profile: RichTextLabel
var facts: RichTextLabel
var commands: VBoxContainer
var draft_box: HBoxContainer
var timer: Timer
var run_button: Button
var feedback: Label
var save_path=ProjectSettings.globalize_path("user://realm-save.polyec")

func label_at(parent: Node,text: String,size_value: int,pos: Vector2,color=Color("e7e2cd")) -> Label:
    var l=Label.new()
    l.text=text
    l.position=pos
    l.add_theme_font_size_override("font_size",size_value)
    l.add_theme_color_override("font_color",color)
    parent.add_child(l)
    return l

func button(parent: Node,text: String,callback: Callable) -> Button:
    var b=Button.new()
    b.text=text
    b.custom_minimum_size=Vector2(0,38)
    var style=StyleBoxFlat.new()
    style.bg_color=Color("24463f")
    style.border_color=Color("74b7a5")
    style.set_border_width_all(1)
    style.set_corner_radius_all(7)
    style.content_margin_left=12
    style.content_margin_right=12
    b.add_theme_stylebox_override("normal",style)
    var hover=style.duplicate()
    hover.bg_color=Color("31564b")
    b.add_theme_stylebox_override("hover",hover)
    b.add_theme_stylebox_override("pressed",hover)
    b.pressed.connect(callback)
    parent.add_child(b)
    return b

func panel(pos: Vector2,dimensions: Vector2) -> Panel:
    var p=Panel.new()
    p.position=pos
    p.size=dimensions
    var style=StyleBoxFlat.new()
    style.bg_color=Color("14262b")
    style.border_color=Color("2b4144")
    style.set_border_width_all(1)
    style.set_corner_radius_all(12)
    p.add_theme_stylebox_override("panel",style)
    add_child(p)
    return p

func _ready() -> void:
    set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    var bg=ColorRect.new()
    bg.color=Color("0b191f")
    bg.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    add_child(bg)
    label_at(self,"POLYEC / QUỐC GIA TỰ VẬN HÀNH",12,Vector2(30,20),Color("d1b575"))
    var title=label_at(self,"Đất nước. Con người. Quyết định.",36,Vector2(30,46))
    title.add_theme_font_override("font",preload("res://fonts/Newsreader.ttf"))
    status=label_at(self,"Đang nạp ba policy…",12,Vector2(32,100),Color("74b7a5"))
    var back=button(self,"←  Về địa đồ",func(): running=false;hide();session.command("pause"))
    back.position=Vector2(1220,30)
    back.size.x=185
    summary=label_at(self,"",15,Vector2(32,142))
    var regions=panel(Vector2(24,184),Vector2(880,265))
    var table=GridContainer.new()
    table.position=Vector2(20,18)
    table.columns=6
    table.add_theme_constant_override("v_separation",10)
    regions.add_child(table)
    var widths=[170,105,115,130,125,60]
    var headers=["LÃNH ĐỊA","NGÂN KHỐ","LƯƠNG THỰC","VIỆC LÀM","HỌC SINH","ĐƯỜNG"]
    for col in range(6):
        var header=label_at(table,headers[col],11,Vector2.ZERO,Color("d1b575"))
        header.custom_minimum_size.x=widths[col]
    for r in range(7):
        var row=[]
        for col in range(6):
            var cell=label_at(table,"",13,Vector2.ZERO)
            cell.custom_minimum_size.x=widths[col]
            row.append(cell)
        region_rows.append(row)
    var inspector=panel(Vector2(924,184),Vector2(492,600))
    label_at(inspector,"HỒ SƠ CÁ THỂ / QUYỀN & KHẢ NĂNG",11,Vector2(20,16),Color("d1b575"))
    id_input=SpinBox.new()
    id_input.min_value=1
    id_input.max_value=10000
    id_input.value=actor
    id_input.position=Vector2(20,44)
    id_input.size=Vector2(170,38)
    id_input.value_changed.connect(func(value): actor=int(value);refresh())
    inspector.add_child(id_input)
    label_at(inspector,"ID 1–64: cai trị · 65–128: kinh tế",11,Vector2(208,53))
    profile=RichTextLabel.new()
    profile.position=Vector2(20,96)
    profile.size=Vector2(450,160)
    inspector.add_child(profile)
    label_at(inspector,"LỆNH HỢP LỆ / CÙNG VALIDATOR VỚI AI",11,Vector2(20,270),Color("d1b575"))
    var scroll=ScrollContainer.new()
    scroll.position=Vector2(20,300)
    scroll.size=Vector2(450,278)
    inspector.add_child(scroll)
    commands=VBoxContainer.new()
    commands.size_flags_horizontal=Control.SIZE_EXPAND_FILL
    scroll.add_child(commands)
    var history=panel(Vector2(24,465),Vector2(880,319))
    label_at(history,"SỔ VIỆC / FACTS ĐÃ BIẾT BỞI CÁ THỂ ĐANG XEM",11,Vector2(20,16),Color("d1b575"))
    facts=RichTextLabel.new()
    facts.position=Vector2(20,47)
    facts.size=Vector2(840,245)
    facts.scroll_following=true
    history.add_child(facts)
    var bar=HBoxContainer.new()
    bar.position=Vector2(30,812)
    bar.add_theme_constant_override("separation",12)
    add_child(bar)
    run_button=button(bar,"▶  Chạy thế giới",toggle_run)
    button(bar,"Một ngày",func(): step_day())
    button(bar,"Lưu",func(): running=false;feedback.text="Đã lưu world + RNG + memory" if session.realm_save(save_path,false) else "Lưu thất bại";refresh())
    button(bar,"Tải",func(): running=false;feedback.text="Đã khôi phục; đang tạm dừng" if session.realm_save(save_path,true) else "Tải thất bại; giữ world hiện tại";refresh())
    button(bar,"F1 · Hướng dẫn",show_guide)
    feedback=label_at(self,"Space chạy/dừng · Esc trở về · Rời cửa sổ sẽ tạm dừng",11,Vector2(32,862),Color("91a7a4"))
    draft_box=HBoxContainer.new()
    draft_box.position=Vector2(30,126)
    draft_box.add_theme_constant_override("separation",12)
    add_child(draft_box)
    timer=Timer.new()
    timer.wait_time=.2
    timer.timeout.connect(func(): if running: step_day())
    add_child(timer)
    timer.start()
    var paths=[]
    var hashes=[]
    for family in ["governance","economic","civilian"]:
        var root=ProjectSettings.globalize_path("res://models/realm-v1/"+family+"/")
        var f=FileAccess.open(root+"manifest.json",FileAccess.READ)
        if f==null:
            status.text="Chưa có bundle "+family
            return
        var manifest=JSON.parse_string(f.get_as_text())
        if not manifest is Dictionary or manifest.get("schema","") not in ["realm-v1","realm-v2"]:
            status.text="Sai schema policy "+family
            return
        paths.append(root+family+".onnx")
        hashes.append(manifest.sha256)
    if not session.realm_load(paths,hashes,37,10000):
        status.text="Không nạp được model: "+str(session.realm_snapshot().get("error",""))
        return
    refresh()

func refresh() -> void:
    var v=session.realm_snapshot(actor)
    if not v.get("loaded",false):
        running=false
        status.text="DỪNG: "+str(v.get("error",""))
        return
    run_button.text="Ⅱ  Tạm dừng" if running else "▶  Chạy thế giới"
    status.text="PPO / 3 HỌ POLICY · SUY LUẬN CPU · SANDBOX THỬ NGHIỆM M3 · Gate chất lượng xem báo cáo"
    summary.text="Ngày %d  ·  %d cá thể / %d còn sống / %d trẻ em  ·  %d quyết định  ·  %d nợ / %d dự án" % [v.day,v.persons,v.alive,v.children,v.decisions,v.loans,v.projects]
    for r in range(7):
        var region=v.regions[r]
        var cells=[NAMES[r],"%d đ" % region.treasury,str(region.stock[0]),"%d / %d" % [region.workers,region.jobs],"%d / %d" % [region.students,region.school],str(region.road)]
        for col in range(6): region_rows[r][col].text=cells[col]
    var p=v.person
    id_input.set_value_no_signal(actor)
    profile.text="Cá thể #%d · %d tuổi · %s\n%s · %s\nTiền %d đ · Nợ %d đ · Sức khỏe %d · Kỹ năng %d\nINT/ADM/ECO/DIP/INTG/MIL: %s\nGia đình #%d · Quyết định %d · Goal %d" % [p.id,p.age,["Cai trị","Kinh tế","Cư dân"][p.role],NAMES[p.region],"Còn sống" if p.alive else "Đã mất",p.cash,p.debt,p.health,p.skill,str(p.stats),p.household,p.decisions,p.goal]
    for child in commands.get_children(): commands.remove_child(child);child.queue_free()
    for a in v.actions:
        var text="%s → %s · %d đv · %d ngày" % [a.name,str(a.target),a.amount,a.term]
        if a.index>=1 and a.index<=14 and p.role==1: text="%s %s · %d đơn vị" % [a.name,GOODS[a.good],a.amount]
        var action_index=int(a.index)
        var b=button(commands,text,func(): running=false;var ok=session.realm_command(actor,action_index);feedback.text="Đã thực hiện lệnh" if ok else str(session.realm_snapshot(actor).get("error","Lệnh bị từ chối"));refresh())
        b.disabled=not a.legal
    facts.clear()
    for f in v.facts:
        var name_value=ACTION_NAMES[f.kind] if f.kind<ACTION_NAMES.size() else {100:"Qua đời",101:"Trưởng thành",102:"Thiếu lương thực",103:"Nợ đến hạn",104:"Đề nghị hết hạn",105:"Sinh ra",106:"Augment mở sổ"}.get(int(f.kind),"Sự việc")
        facts.append_text("Ngày %d · #%d  %s · %s · %d\n" % [f.day,f.actor,name_value,NAMES[f.region],f.amount])
    draft_box.visible=v.chosen<0
    summary.visible=v.chosen>=0
    if draft_box.visible and draft_box.get_child_count()==0:
        for card_value in v.cards:
            var card=int(card_value)
            button(draft_box,CARDS[card],func(): session.realm_draft(card,false);refresh())
        var reroll=button(draft_box,"Đổi thẻ đầu (1 lần)",func(): session.realm_draft(0,true);for child in draft_box.get_children(): draft_box.remove_child(child);child.queue_free();refresh())
        reroll.disabled=v.rerolled

func step_day() -> void:
    if not session.realm_step(1):
        running=false
        feedback.text=str(session.realm_snapshot(actor).get("error","World đã dừng"))
    refresh()

func toggle_run() -> void:
    running=not running
    refresh()

func show_guide() -> void:
    running=false
    var dialog=AcceptDialog.new()
    dialog.title="Hướng dẫn sandbox"
    dialog.dialog_text="Chọn một thẻ mở sổ, rồi chạy thế giới hoặc tiến một ngày.\nBa policy PPO dùng chung trọng số theo họ; từng người có memory và sampling riêng.\nĐổi ID để xem cá thể, quyền biết và lệnh hợp lệ. Lệnh UI đi cùng pipeline với AI.\nViệc làm, học, vận tải và điều tra có thời lượng; nợ/dự án có hạn ngày game.\nLưu chứa world, RNG, counters và memory; tải luôn tạm dừng.\nĐây là sandbox M3 thử nghiệm; không phải campaign hoàn chỉnh.\nMột số augment/chính trị/stat còn giới hạn được ghi trong báo cáo."
    add_child(dialog)
    dialog.popup_centered(Vector2i(700,320))
    refresh()

func _process(delta: float) -> void:
    if delta>.25: running=false

func _notification(what: int) -> void:
    if what==NOTIFICATION_APPLICATION_FOCUS_OUT or what==NOTIFICATION_WM_WINDOW_FOCUS_OUT:
        running=false
