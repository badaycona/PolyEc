extends Control

const FOREIGN_PARTNER_LABELS := ["A", "B", "C"]

const NAMES=["Thanh Điền","Hắc Sơn","Bạch Khâu","Thiết Lĩnh","Lâm Giang","Vân Hải","Đất triều đình"]
const CARDS=["Sổ Bộ Chuẩn Mực","Lò Đào Tạo Hoàng Gia","Người Đứng Giữa","Kho Thóc Tiền Triều","Liên Minh Công Thương","Đoàn Công Trình Hoàng Gia","Môn Sinh Khắp Chốn","Mạng Lưới Thư Trạm","Ngân Quỹ Kiến Quốc","Hiến Ước Liên Lãnh","Học Viện Bách Nghệ","Hội Đồng Khai Quốc"]
const ACTION_NAMES=["Chờ","Mua","Bán","Vận tải","Vay","Trả nợ","Dự án","Tuyển dụng","Thuế","Cứu trợ","Điều tra","Đề nghị","Chấp thuận","Từ chối","Làm việc","Học","Nghỉ","Giao lưu","Tham gia hội","Di cư","Giúp gia đình","Bổ nhiệm","Huy động","Từ chức"]
const GOODS=["lương khô","thực phẩm bổ sung","muối","gỗ","nhiên liệu","sắt","đồng","bạc","vàng","xơ sợi","sáp ong","vải","lụa","gia súc","ngựa","ngựa chiến","công cụ","quân nhu","vũ khí","vật liệu xây dựng","giấy","thuốc","trà","trầm hương","thủy tinh trang trí","đồ sứ tinh xảo","hàng xa xỉ","thủy tinh thường","đồ gốm thường","da thuộc"]
const ESTABLISHMENT_NAMES=["","xưởng dệt","xưởng công cụ","xưởng quân nhu","xưởng muối","xưởng gỗ","mỏ","xưởng giấy","vườn trà","xưởng trầm","trại ngựa","lò rèn vũ khí","lò thủy tinh thường","lò gốm thường","xưởng thuộc da","vườn dược liệu","cơ sở ép và sấy hạt","xưởng cưa","trại nuôi ong","trại nuôi tằm","mỏ vàng"]

const SPECIALTY_TRIAL_PRICES=[90,55,80]
var session: RefCounted
var running=false
var actor=1
var status: Label
var summary: Label
var funds_label: Label
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

static func fact_kind_name(kind: int) -> String:
    if kind >= 0 and kind < ACTION_NAMES.size():
        return ACTION_NAMES[kind]
    return {100:"Qua đời",101:"Trưởng thành",102:"Thiếu lương thực",103:"Nợ đến hạn",104:"Đề nghị hết hạn",105:"Sinh ra",106:"Augment mở sổ",107:"Đóng/mở tuyến",108:"Báo cáo kho",109:"Kết thúc điều tra",110:"Hết ngân sách lương",111:"Bước sang tuổi 75 · nguy cơ tử vong tăng",112:"Qua đời do tuổi cao",113:"Dinh dưỡng giảm do thiếu bữa",114:"Bắt đầu phục vụ quân tình nguyện",115:"Kết thúc phục vụ quân tình nguyện",227:"Bắt đầu kỳ tập kết tình nguyện",228:"Hết kỳ tập kết tình nguyện",229:"Hai bên lập cam kết giao quân tình nguyện",230:"Đến hạn cam kết nhưng còn thiếu người",231:"Người nhận chấp thuận gia hạn nghĩa vụ",232:"Cấp cứu trợ từ quỹ đô thị",234:"Đến hạn cấp lương khô nhưng còn thiếu",235:"Sovereign chấp thuận cam kết cấp lương khô",236:"Sovereign chấp thuận gia hạn lương khô",237:"Tự dùng Medicine để điều trị",238:"Ăn thêm một món thực phẩm bổ sung",239:"Đốt Beeswax cá nhân khi nghỉ · giảm 5 điểm nợ ngủ",248:"Đối tác khởi phát đột kích ven biển",249:"Nam Cảng đẩy lùi đợt đột kích",250:"Đối phương vượt qua phòng thủ",251:"Tình nguyện viên bị đánh gục",252:"Muối bị lấy khỏi cảng"}.get(kind,"Sự việc")

static func good_name(good: int) -> String:
    return GOODS[clampi(good,0,GOODS.size()-1)]

static func tea_social_action_description(amount: int) -> String:
    return "Dùng 1 Tea của chính mình và trả %d đồng · tăng điểm giao lưu thêm 5 (tối đa 100) · bận 7 ngày game" % amount

static func incense_social_action_description(amount: int) -> String:
    return "Dùng 1 Incense của chính mình và trả %d đồng · tăng điểm giao lưu thêm 8 (tối đa 100) · bận 7 ngày game" % amount

static func silver_social_action_description(amount: int) -> String:
    return "Dùng 1 Silver của chính mình và trả %d đồng · tăng điểm giao lưu thêm 10 (tối đa 100) · bận 7 ngày game" % amount

static func silk_social_action_description() -> String:
    return "Dùng 1 Silk của chính mình và trả 4 đồng vào quỹ vùng · tăng điểm giao lưu của bản thân thêm 12 (tối đa 100) · bận 7 ngày game · chỉ Civilian ngoài quân vụ"

static func gold_social_action_description() -> String:
    return "Dùng 1 Gold của chính mình và trả 20 đồng vào quỹ vùng · tăng điểm giao lưu của bản thân thêm 20 (tối đa 100) · bận 14 ngày game · chỉ Civilian trưởng thành ngoài quân vụ"

static func pottery_meal_vessel_description() -> String:
    return "Đồ gốm cá nhân bền: sau bữa lương khô đã ăn, thêm tối đa 5 dinh dưỡng (giới hạn 100); bản thân đồ gốm không bị tiêu."

static func beeswax_rest_description() -> String:
    return "Nghỉ theo lịch. Khi kỳ nghỉ hoàn tất tại chỗ, Civilian ngoài quân vụ có thể đốt 1 Beeswax cá nhân nếu còn nợ ngủ; giảm thêm 5 điểm (tối thiểu 0). Không dùng sáp từ kho chung."

static func travel_action_description(destination: String, days: int, mounted: bool) -> String:
    var text="Đi tới %s · %d ngày game · phí %d đồng" % [destination,days,4+2*days]
    if mounted:
        text+=" · dùng WarHorse cá nhân, giảm một ngày; không tiêu ngựa"
    return text

static func establishment_transfer_action_text(good: int, amount: int, site_id: int, deposit: bool) -> String:
    var product=good_name(good)
    var direction="đưa vào xưởng" if deposit else "lấy khỏi xưởng"
    return "%s %s · tối đa %d món · cơ sở #%d" % [direction,product,amount,site_id]

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
    var back=button(self,"←  Về địa đồ",back_to_map)
    back.position=Vector2(1220,30)
    back.size.x=185
    funds_label=label_at(self,"",11,Vector2(32,121),Color("91a7a4"))
    summary=label_at(self,"",15,Vector2(32,142))
    var regions=panel(Vector2(24,184),Vector2(880,265))
    var region_scroll=ScrollContainer.new()
    region_scroll.position=Vector2(20,18)
    region_scroll.size=Vector2(840,230)
    regions.add_child(region_scroll)
    var table=GridContainer.new()
    table.columns=6
    table.add_theme_constant_override("v_separation",10)
    region_scroll.add_child(table)
    var widths=[170,105,115,130,125,60]
    var headers=["LÃNH ĐỊA","NGÂN KHỐ","LƯƠNG THỰC","VIỆC LÀM","HỌC SINH","ĐƯỜNG"]
    for col in range(6):
        var header=label_at(table,headers[col],11,Vector2.ZERO,Color("d1b575"))
        header.custom_minimum_size.x=widths[col]
    for r in range(27):
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
    label_at(inspector,"ID khởi đầu 1–64 cai trị · 65–128 kinh tế",11,Vector2(208,53))
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
    button(bar,"Atlas kiểm thử thủ công",start_atlas_manual_test)
    button(bar,"Một ngày",func(): step_day())
    if OS.get_cmdline_user_args().has("--worksite-smoke"):
        actor=65
        id_input.set_value_no_signal(actor)
        button(bar,"Chu trình xưởng (dev)",run_worksite_smoke)
    button(bar,"Lưu",func(): save_game())
    button(bar,"Tải",func(): load_game())
    button(bar,"F1 · Hướng dẫn",show_guide)
    feedback=label_at(self,"Space chạy/dừng · Esc trở về · Rời cửa sổ sẽ tạm dừng",11,Vector2(32,862),Color("91a7a4"))
    draft_box=HBoxContainer.new()
    draft_box.position=Vector2(30,126)
    draft_box.add_theme_constant_override("separation",12)
    add_child(draft_box)
    timer=Timer.new()
    timer.wait_time=.2
    timer.timeout.connect(func(): advance_automatic_day())
    add_child(timer)
    timer.start()
    if OS.get_cmdline_user_args().has("--atlas-manual-test"):
        start_atlas_manual_test()
        return
    if not session.has_method("realm_schema") or session.realm_schema()!="realm-v37":
        status.text="Lõi mô phỏng không khớp dữ liệu realm-v37; cần build đúng phiên bản"
        return
    var paths=[]
    var hashes=[]
    var model_root="res://models/realm-v37"
    for arg in OS.get_cmdline_user_args():
        if arg.begins_with("--model-root="):
            model_root=arg.trim_prefix("--model-root=")
    for family in ["governance","economic","civilian"]:
        var root=model_root.trim_suffix("/")+"/"+family+"/"
        if root.begins_with("res://"):
            root=ProjectSettings.globalize_path(root)
        var f=FileAccess.open(root+"manifest.json",FileAccess.READ)
        if f==null:
            status.text="Chưa có bundle "+family
            return
        var manifest=JSON.parse_string(f.get_as_text())
        if not manifest is Dictionary or manifest.get("schema","")!="realm-v37":
            status.text="Sai schema policy "+family
            return
        if manifest.get("model_contract","")!=session.realm_model_contract():
            status.text="Model không tương thích với core hiện tại: "+family
            return
        paths.append(root+family+".onnx")
        hashes.append(manifest.sha256)
    if not session.realm_load(paths,hashes,37,10000):
        status.text="Không nạp được model: "+str(session.realm_snapshot().get("error",""))
        return
    refresh()

static func body_status_text(person_data: Dictionary) -> String:
    return "Nợ ngủ %d ngày quy đổi · Gắng sức %d/100 (chỉ số thử)" % [
        person_data.get("sleep_debt", 0), person_data.get("exertion", 0)]

static func region_label(region: int) -> String:
    return NAMES[region] if region>=0 and region<NAMES.size() else "Vùng %d" % region

static func legacy_capability_text(capabilities: Dictionary) -> String:
    var labels={
        "strategy":"Chiến lược", "administration":"Hành chính",
        "economic_planning":"Kế hoạch kinh tế", "diplomacy":"Ngoại giao",
        "investigation":"Điều tra", "military":"Quân sự"
    }
    var rows=[]
    for key in ["strategy","administration","economic_planning","diplomacy","investigation","military"]:
        rows.append("%s %d" % [labels[key],capabilities.get(key,0)])
    return "Năng lực kế thừa: "+" · ".join(rows)

static func player_knowledge_text(knowledge: Dictionary) -> String:
    if int(knowledge.get("kind",0))!=1:
        return "Thông tin từ save cũ chưa rõ loại và nguồn"
    var confidence=int(knowledge.get("confidence_basis_points",-1))
    var confidence_text="độ tin cậy chưa được đo" if confidence<0 else "độ tin cậy %.2f%%" % (float(confidence)/100.0)
    return "Bằng chứng điều tra #%d · %s" % [int(knowledge.get("source_investigation_id",0)),confidence_text]

static func labor_skill_text(value: int) -> String:
    return "Kỹ năng lao động %d/100" % value

static func military_service_text(service: Dictionary, current_day: int) -> String:
    if not bool(service.get("active",false)):
        return "Không phục vụ quân sự"
    var consent="Đã đồng ý tham chiến" if bool(service.get("combat_consent",false)) else "Chưa đồng ý tham chiến"
    var start_day=int(service.get("start_day",current_day))
    var start_note=" · có thể chọn từ ngày %d" % start_day if current_day<start_day else ""
    return "Tình nguyện phục vụ · chỉ huy #%d · còn %d ngày · lương %d đ/ngày · quỹ lương còn %d đ · %s%s" % [int(service.get("commander",0)),maxi(0,int(service.get("end_day",current_day))-current_day),int(service.get("wage_per_day",0)),int(service.get("escrow",0)),consent,start_note]

static func military_combat_consent_button_text(consent: bool) -> String:
    return "Rút lại đồng ý tham chiến" if consent else "Tôi đồng ý tham chiến trong kỳ phục vụ này"

static func urban_census_text(rows: Array, sovereign: bool) -> String:
    if rows.is_empty(): return ""
    var lines=["Nơi cư trú và chỗ ở (sức chứa tính theo người, ước lệ thử nghiệm):"]
    for row in rows:
        var place=str(row.get("site_name", "địa điểm %d" % int(row.get("site_id", 0))))
        lines.append("%s · đô thị %d · sức chứa %d · còn thiếu chỗ %d · giường trống %d · nông thôn %d · chưa rõ %d" % [place,int(row.get("urban_residents",0)),int(row.get("housing_capacity",0)),int(row.get("unhoused_residents",0)),int(row.get("available_beds",0)),int(row.get("rural_residents",0)),int(row.get("unknown_residence",0))])
        var representative=int(row.get("council_representative_id",0))
        var nominee=int(row.get("council_nominee_id",0))
        if representative>0 and int(row.get("council_term_end_day",-1))>0:
            lines.append("  Đại diện hội đồng do lãnh chủ bổ nhiệm: cá thể #%d · hết nhiệm kỳ ngày %d" % [representative,int(row.get("council_term_end_day",0))])
        else:
            lines.append("  Ghế hội đồng đang trống")
        if nominee>0:
            lines.append("  Hồ sơ tự ứng cử của cá thể #%d từ ngày %d · chờ lãnh chủ quyết định; đây chưa phải bầu cử" % [nominee,int(row.get("council_nomination_day",-1))])
    if not sovereign: lines.append("Chỉ hiện vùng cư trú của cá thể đang xem.")
    lines.append("“Chưa rõ” nghĩa là save cũ chưa lưu địa chỉ; không thể kết luận người đó ở nông thôn.")
    return "\n".join(lines)

static func foreign_trade_policy_text(suspended: bool) -> String:
    return "đang tạm đình chỉ lệnh mới" if suspended else "đang cho phép lệnh mới"

static func foreign_trade_history_text(partner: Dictionary) -> String:
    return "Lịch sử hoàn tất: nhập %d đơn, %d DryFood, tổng tiền hàng trước thuế/phí %d đ; xuất %d đơn, %d Salt, tổng tiền hàng trước thuế/phí %d đ" % [int(partner.get("completed_import_orders",0)),int(partner.get("imported_units",0)),int(partner.get("import_gross",0)),int(partner.get("completed_export_orders",0)),int(partner.get("exported_units",0)),int(partner.get("export_gross",0))]

static func foreign_trade_offer_text(offer: Dictionary, current_day: int) -> String:
    var partner_labels=["A","B","C"]
    var partner_id=clampi(int(offer.get("partner_id",0)),0,partner_labels.size()-1)
    var importing=int(offer.get("operation",0))%2==0
    var direction="nhập hàng khô" if importing else "xuất muối"
    var status=int(offer.get("status",0))
    var status_text="đang chờ đối tác phản hồi đến hết ngày %d" % int(offer.get("due_day",current_day))
    if status==1: status_text="đã được chấp thuận; đơn #%s đang đi" % str(offer.get("order_id",""))
    elif status==2: status_text="bị từ chối; tài sản đã được trả lại"
    elif status==3: status_text="hết hạn; tài sản đã được trả lại"
    var held="đang giữ %d đ" % int(offer.get("held_cash",0))
    if int(offer.get("held_good",0))>0: held+=" và %d đơn vị hàng" % int(offer.get("held_good",0))
    if status!=0: held="không còn tài sản bị giữ"
    return "Đề nghị #%s với đối tác %s · %s · %d đơn vị · ngày gửi %d · %s · %s" % [str(offer.get("id","")),partner_labels[partner_id],direction,int(offer.get("quantity",0)),int(offer.get("created_day",0)),status_text,held]

static func foreign_trade_policy_button_text(partner_id: int, suspended: bool) -> String:
    var partner_label: String = FOREIGN_PARTNER_LABELS[clampi(partner_id, 0, FOREIGN_PARTNER_LABELS.size() - 1)]
    return "Khôi phục giao dịch với đối tác %s" % partner_label if suspended else "Đình chỉ giao dịch với đối tác %s" % partner_label

static func military_service_offer_text(target: int, wage_per_day: int, term: int) -> String:
    return "Đề nghị cá thể #%d tình nguyện · lương %d đ/ngày · %d ngày · giữ trước %d đ" % [target,wage_per_day,term,wage_per_day*term]

static func military_unit_roster_text(units: Array, region: int) -> String:
    var lines=[]
    for unit in units:
        if int(unit.get("source_region",-1))!=region: continue
        lines.append("Roster tình nguyện #%s · tập kết %s · chỉ huy #%d · %d người · còn %d đ lương · đã trả %d đ" % [
            str(unit.get("id","")),region_label(region),int(unit.get("commander",0)),unit.get("members",[]).size(),
            int(unit.get("payroll_escrow",0)),int(unit.get("paid_total",0))])
        var mobilized_members=unit.get("mobilized_members",[])
        if not mobilized_members.is_empty():
            lines.append("Đang trong kỳ tập kết: %d người · hạn sớm nhất ngày %d" % [mobilized_members.size(),int(unit.get("mobilized_until_day",0))])
    return "\n".join(lines) if not lines.is_empty() else "Chưa có roster tình nguyện tại vùng này"

static func military_raid_text(raid: Dictionary) -> String:
    var partner_labels=["A","B","C"]
    var partner_id=clampi(int(raid.get("partner_id",0)),0,partner_labels.size()-1)
    var status=int(raid.get("status",0))
    var defense_detail="Điểm phòng thủ %d · mỗi người mang Leather cá nhân được thêm 1 điểm và không mất món đó" % int(raid.get("defense_score",0))
    if status==0:
        return "Đối tác %s đã đưa %d đơn vị tới Nam Cảng · đang chờ Lãnh chủ trả lời" % [partner_labels[partner_id],int(raid.get("force_units",0))]
    if status==1:
        return "Đối tác %s bị đẩy lui · %s · %d đơn vị bị mất · %d tình nguyện viên bị đánh gục · %d người đã dùng một quân nhu cá nhân; quy trình tử vong chạy ở ngày kế tiếp" % [partner_labels[partner_id],defense_detail,int(raid.get("attacker_losses",0)),int(raid.get("defender_casualties",0)),raid.get("supplied_defenders",[]).size()]
    if status==2:
        return "Đối tác %s vượt qua phòng thủ · %s · %d tình nguyện viên bị đánh gục · %d người đã dùng một quân nhu cá nhân; quy trình tử vong chạy ở ngày kế tiếp · lấy %d Salt" % [partner_labels[partner_id],defense_detail,int(raid.get("defender_casualties",0)),raid.get("supplied_defenders",[]).size(),int(raid.get("salt_looted",0))]
    return "Lãnh chủ rút quân trước đợt đột kích của đối tác %s · đối phương lấy %d Salt" % [partner_labels[partner_id],int(raid.get("salt_looted",0))]

static func military_obligation_text(items: Array) -> String:
    var lines=[]
    for item in items:
        var delivered: Array=item.get("delivered_members",[])
        var required=int(item.get("required_members",0))
        var status="đã hoàn tất ngày %d" % int(item.get("completed_day",-1)) if int(item.get("completed_day",-1))>=0 else "đang thực hiện"
        var result="Đã tới %d/%d người" % [delivered.size(),required]
        if bool(item.get("due_checked",false)):
            result+=" · thiếu %d người tại hạn gần nhất" % int(item.get("shortage_at_last_deadline",0))
        if bool(item.get("first_deadline_checked",false)):
            result+=" · hạn đầu ngày %d thiếu %d người" % [int(item.get("first_due_day",0)),int(item.get("shortage_at_first_deadline",0))]
        if int(item.get("extensions",0))>0:
            result+=" · đã được gia hạn một lần"
        var member_ids: Array=item.get("members",[])
        var member_labels=[]
        for member_id in member_ids: member_labels.append("#%d" % int(member_id))
        lines.append("Cam kết quân tình nguyện #%d · người giao #%d · người nhận #%d · %d người từ %s tới %s · hạn ngày %d · %s · %s · danh sách: %s" % [
            int(item.get("id",0)),int(item.get("obligor",0)),int(item.get("recipient",0)),required,
            region_label(int(item.get("source_region",-1))),region_label(int(item.get("destination_region",-1))),
            int(item.get("due_day",0)),status,result,", ".join(member_labels)])
    return "\n".join(lines) if not lines.is_empty() else "Chưa có cam kết tập kết quân tình nguyện liên quan"

static func military_supply_obligation_text(items: Array) -> String:
    var lines=[]
    for item in items:
        var required=int(item.get("required",0))
        var delivered=int(item.get("delivered",0))
        var status="đã giao đủ ngày %d" % int(item.get("completed_day",-1)) if int(item.get("completed_day",-1))>=0 else "đang chờ giao"
        var result="đã giao %d/%d phần lương khô" % [delivered,required]
        if bool(item.get("due_checked",false)):
            result+=" · thiếu %d phần tại hạn gần nhất" % int(item.get("shortage_at_last_deadline",0))
        if bool(item.get("first_deadline_checked",false)):
            result+=" · hạn đầu ngày %d thiếu %d phần" % [int(item.get("first_due_day",0)),int(item.get("shortage_at_first_deadline",0))]
        if int(item.get("extensions",0))>0:
            result+=" · đã được gia hạn một lần"
        lines.append("Cam kết quân nhu #%d cho nghĩa vụ tập kết #%d · người mua #%d · người nhận #%d · %s → %s · hạn ngày %d · %s · %s" % [
            int(item.get("id",0)),int(item.get("personnel_obligation_id",0)),int(item.get("obligor",0)),
            int(item.get("recipient",0)),region_label(int(item.get("source_region",-1))),
            region_label(int(item.get("destination_region",-1))),int(item.get("due_day",0)),status,result])
    return "\n".join(lines) if not lines.is_empty() else "Chưa có cam kết cấp lương khô liên quan"

static func establishment_text(items: Array, region: int) -> String:
    var lines=[]
    for site in items:
        if int(site.get("region",-1))!=region: continue
        var industry=clampi(int(site.get("industry",0)),0,ESTABLISHMENT_NAMES.size()-1)
        var stock=site.get("stock",[])
        var product_good=-1
        var product_name="sản phẩm"
        if industry==12: product_good=27;product_name=GOODS[27]
        elif industry==13: product_good=28;product_name=GOODS[28]
        elif industry==14: product_good=29;product_name=GOODS[29]
        elif industry==17: product_good=19;product_name=GOODS[19]
        elif industry==18: product_good=10;product_name=GOODS[10]
        elif industry==19: product_good=12;product_name=GOODS[12]
        elif industry==20: product_good=8;product_name=GOODS[8]
        var line="#%d %s · tồn kho %d %s" % [int(site.get("id",0)),ESTABLISHMENT_NAMES[industry],int(stock[product_good]) if product_good>=0 and product_good<stock.size() else 0,product_name]
        if industry in [12,13,14]:
            var source=int(site.get("source_units",0))
            line+=" · còn nguyên liệu cho tối đa %d mẻ trong ledger nguồn (không phải hàng để bán)" % source
            if industry in [12,13]:
                var fuel=int(stock[4]) if stock.size()>4 else 0
                line+=" · nhiên liệu trong xưởng %d · mỗi mẻ cần 1" % fuel
            line+=" · giá tham chiếu thử %d đ/món, chưa hiệu chuẩn" % SPECIALTY_TRIAL_PRICES[industry-12]
        elif industry==17:
            line+=" · dùng 1 Timber cùng vùng và 16 tick Work cho mỗi vật liệu xây dựng"
        elif industry==18:
            line+=" · còn %d source units, không phải hàng để bán; 1 Beeswax mỗi 16 tick Work" % int(site.get("source_units",0))
        elif industry==19:
            line+=" · còn %d source units, không phải hàng để bán; 1 Silk mỗi 16 tick Work; nguồn bổ sung25 mỗi90 ngày game, tối đa100" % int(site.get("source_units",0))
        elif industry==20:
            line+=" · còn %d source units hữu hạn, không tái tạo; 1 Gold mỗi 16 tick Work" % int(site.get("source_units",0))
        lines.append(line)
    return "\n".join(lines) if not lines.is_empty() else "Chưa có xưởng trong vùng này"

func refresh() -> void:
    var v=session.realm_snapshot(actor)
    if not v.get("loaded",false):
        running=false
        status.text="DỪNG: "+str(v.get("error",""))
        return
    run_button.text="Ⅱ  Tạm dừng" if running else "▶  Chạy thế giới"
    status.text="KIỂM THỬ ATLAS THỦ CÔNG · NPC không tự ra quyết định; đây chưa phải bản game hoàn chỉnh" if v.get("atlas_manual_test",false) else "Policy realm-v37 · quyết định tự chủ chỉ bật sau khi có đủ 3 bộ tương thích đã huấn luyện và kiểm tra"
    summary.text="Ngày %d  ·  %d cá thể / %d còn sống / %d trẻ em  ·  %d quyết định  ·  %d nợ / %d dự án" % [v.day,v.persons,v.alive,v.children,v.decisions,v.loans,v.projects]
    var atlas_names={}
    for site in v.get("urban_census",[]):
        atlas_names[int(site.get("territory_id",-1))]=str(site.get("site_name",""))
    for r in range(min(region_rows.size(),v.regions.size())):
        var region=v.regions[r]
        var region_name=str(atlas_names.get(r,region_label(r)))
        var cells=[region_name,"%d đ" % region.treasury,str(region.stock[0]),"%d / %d" % [region.workers,region.jobs],"%d / %d" % [region.students,region.school],str(region.road)]
        for col in range(6): region_rows[r][col].text=cells[col]
    funds_label.visible=v.chosen>=0
    if v.chosen>=0:
        funds_label.text="%s · Quỹ tín dụng %d đ · Quỹ hạ tầng %d đ" % [CARDS[v.chosen],v.credit_pool,v.restricted_fund]
    var p=v.person
    id_input.set_value_no_signal(actor)
    var role_text=["Cai trị","Kinh tế","Cư dân"][p.role]
    if p.role==0: role_text="Lãnh chủ · quyền thuế" if p.authority else "Cố vấn"
    if p.candidate: role_text+=" · Ứng tuyển"
    var service_text=military_service_text(p.military_service,v.day)
    var ration_stock=0
    var supplementary_ration_stock=0
    if p.region>=0 and p.region<v.regions.size():
        ration_stock=int(v.regions[p.region].get("military_ration_stock",0))
        supplementary_ration_stock=int(v.regions[p.region].get("military_supplementary_ration_stock",0))
    profile.text="Cá thể #%d · %d tuổi · %s\n%s · %s\nTiền %d · Nợ %d · Sức khỏe %d\nDinh dưỡng %d · %s\nKho quân nhu vùng này: Lương khô %d phần · Khẩu phần bổ sung %d phần\n%s\n%s\n%s\nGia đình #%d · Quyết định %d · Mục tiêu %d" % [p.id,p.age,role_text,region_label(p.region),"Còn sống" if p.alive else "Đã mất",p.cash,p.debt,p.health,p.nutrition,labor_skill_text(p.skill),ration_stock,supplementary_ration_stock,body_status_text(p),legacy_capability_text(p.legacy_capabilities),service_text,p.household,p.decisions,p.goal]
    var personal_goods: Array=p.get("inventory",[])
    if p.role==2 and not bool(p.get("military_service",{}).get("active",false)) and personal_goods.size()>28 and int(personal_goods[28])>0:
        profile.text+="\n"+pottery_meal_vessel_description()
    if p.role==0: profile.text+="\n"+military_unit_roster_text(v.get("military_unit_rosters",[]),p.region)
    profile.text+="\n"+establishment_text(v.get("establishments",[]),p.region)
    var residence_text=urban_census_text(v.get("urban_census",[]),bool(p.get("sovereign",false)))
    if not residence_text.is_empty(): profile.text+="\n"+residence_text
    if not v.get("military_obligations",[]).is_empty():
        profile.text+="\n"+military_obligation_text(v.get("military_obligations",[]))
    if not v.get("military_supply_obligations",[]).is_empty():
        profile.text+="\n"+military_supply_obligation_text(v.get("military_supply_obligations",[]))
    if p.role==0 and p.region>=0 and p.region<v.regions.size():
        profile.text+="\nQuỹ công trình đô thị thử nghiệm tại vùng này: %d đ" % int(v.regions[p.region].get("urban_fund",0))
    var foreign_trade=v.get("foreign_trade",{})
    if not foreign_trade.is_empty():
        profile.text+="\nBa sổ đối tác mô phỏng tại Nam Cảng (endowment thử, không phải số liệu lịch sử):"
        for partner in foreign_trade.get("partners",[]):
            var partner_labels=["A","B","C"]
            var partner_id=clampi(int(partner.get("partner_id",0)),0,2)
            var suspended=bool(partner.get("trade_suspended",false))
            profile.text+="\nĐối tác %s: %s · %d đ · còn %d hàng khô để bán · còn chỗ nhận %d muối" % [partner_labels[partner_id],foreign_trade_policy_text(suspended),int(partner.get("cash",0)),int(partner.get("dry_food",0)),int(partner.get("salt_capacity",0))]
            profile.text+="\n  "+foreign_trade_history_text(partner)
            if int(partner.get("completed_import_orders",0))>0 or int(partner.get("completed_export_orders",0))>0:
                profile.text+="\n  Số tiền trên là tổng lịch sử hàng hóa trước thuế và phí, không phải lợi nhuận."
            if bool(p.get("sovereign",false)):
                var policy_button=button(commands,foreign_trade_policy_button_text(partner_id,suspended),func(): run_foreign_trade_suspension(actor,partner_id,not suspended,str(v.fingerprint)))
                policy_button.tooltip_text="Chỉ chặn lệnh mới với đối tác này. Đơn hàng đã được chấp nhận vẫn đến và quyết toán theo lịch."
        var import_route=foreign_trade.get("import_route",{})
        var export_route=foreign_trade.get("export_route",{})
        profile.text+="\nTuyến nhập Hải Châu → Nam Cảng: %s · %d ngày · phí tàu %d đ/món + phí cảng 2 đ/món" % ["đang mở" if import_route.get("open",false) else "đang đóng",int(import_route.get("travel_days",0)),int(import_route.get("fee_per_unit",0))]
        profile.text+="\nTuyến xuất Nam Cảng → Hải Châu: %s · %d ngày · phí tàu %d đ/món + phí cảng 2 đ/món" % ["đang mở" if export_route.get("open",false) else "đang đóng",int(export_route.get("travel_days",0)),int(export_route.get("fee_per_unit",0))]
        for order in foreign_trade.get("orders",[]):
            var direction="nhập hàng khô" if int(order.get("operation",0))%2==0 else "xuất muối"
            var partner_labels=["A","B","C"]
            var partner_id=clampi(int(order.get("partner_id",0)),0,2)
            profile.text+="\nĐơn của đối tác %s đang đi: %s · %d đơn vị · dự kiến đến cuối ngày %d" % [partner_labels[partner_id],direction,int(order.get("quantity",0)),int(order.get("arrival_day",0))]
        for offer in foreign_trade.get("offers",[]):
            profile.text+="\n"+foreign_trade_offer_text(offer,v.day)
    for child in commands.get_children(): commands.remove_child(child);child.queue_free()
    if v.get("atlas_manual_test",false):
        add_private_housing_controls(v,p)
    var inventory: Array=p.get("inventory",[])
    if int(p.get("role",-1))==2 and int(p.get("age",0))>=6 and (inventory.size()<=7 or int(inventory[7])==0):
        add_silver_social_market_control(v,p)
    var military_raids: Array=v.get("military_raids",[])
    if bool(p.get("sovereign",false)):
        var pending_raid: Dictionary={}
        for raid in military_raids:
            if int(raid.get("status",-1))==0: pending_raid=raid;break
        if not pending_raid.is_empty():
            var battle_notice=Label.new()
            battle_notice.text=military_raid_text(pending_raid)
            commands.add_child(battle_notice)
            var battle_candidates: Array=v.get("military_battle_candidates",[])
            var defender_ids=PackedInt64Array()
            for candidate in battle_candidates: defender_ids.append(int(candidate.get("id",0)))
            var defend_button=button(commands,"Phòng thủ bằng %d tình nguyện viên đủ điều kiện tại cảng" % defender_ids.size(),func(): run_military_battle(actor,int(pending_raid.get("id",0)),false,defender_ids,str(v.fingerprint)))
            defend_button.disabled=defender_ids.is_empty()
            defend_button.tooltip_text="Chỉ chọn người còn sống, khỏe mạnh, đã đồng ý tham chiến, đang trong kỳ tập kết và hiện có mặt tại Nam Cảng. Lõi sẽ kiểm tra lại toàn bộ điều kiện khi nhận lệnh."
            button(commands,"Rút quân khỏi cảng",func(): run_military_battle(actor,int(pending_raid.get("id",0)),true,PackedInt64Array(),str(v.fingerprint)))
        else:
            var trade_data=v.get("foreign_trade",{})
            if not trade_data.is_empty():
                var dev_notice=Label.new()
                dev_notice.text="Công cụ thử trận: hiện chưa có chính sách tự chủ cho ba đối tác; nút dưới đây mô phỏng lệnh do đối tác gửi và tiêu tốn hàng thật trong sổ của họ."
                commands.add_child(dev_notice)
                var route_open=bool(trade_data.get("import_route",{}).get("open",false))
                for partner in trade_data.get("partners",[]):
                    var partner_id=int(partner.get("partner_id",0))
                    var label: String=FOREIGN_PARTNER_LABELS[clampi(partner_id,0,FOREIGN_PARTNER_LABELS.size()-1)]
                    var raid_button=button(commands,"[DEV/TEST] Mô phỏng đối tác %s đột kích với 1 đơn vị" % label,func(): run_military_raid(partner_id,1,str(v.fingerprint)))
                    raid_button.disabled=not route_open
                    raid_button.tooltip_text="Lệnh dành cho kiểm tra gameplay. Khi có AI đối tác, hành vi khởi phát sẽ do hệ thống đó quyết định; nút này chỉ tạo tình huống thử."
    for raid in military_raids:
        var raid_history=Label.new()
        raid_history.text=military_raid_text(raid)
        commands.add_child(raid_history)
    var service=p.military_service
    if p.role==2 and bool(service.get("active",false)):
        if v.day>=int(service.get("start_day",v.day)) and v.day<int(service.get("end_day",v.day)):
            var current_consent=bool(service.get("combat_consent",false))
            var consent_button=button(commands,military_combat_consent_button_text(current_consent),func(): run_military_combat_consent(actor,not current_consent,str(v.fingerprint)))
            consent_button.tooltip_text="Chỉ thay đổi lựa chọn của cá thể đang xem. Đồng ý phục vụ có lương không tự đồng nghĩa đồng ý tham chiến."
        else:
            var start_notice=Label.new()
            start_notice.text="Kỳ phục vụ bắt đầu ngày %d; chưa thể chọn tham chiến trước ngày đó." % int(service.get("start_day",v.day))
            commands.add_child(start_notice)
    for a in v.actions:
        var text="%s → %s · %d đv · %d ngày" % [a.name,str(a.target),a.amount,a.term]
        if a.name in ["Mua","Bán"]: text="%s %s · %d đơn vị" % [a.name,GOODS[clampi(a.good,0,GOODS.size()-1)],a.amount]
        elif a.name=="Giao lưu cùng trà": text=tea_social_action_description(a.amount)
        elif a.name=="Giao lưu cùng trầm hương": text=incense_social_action_description(a.amount)
        elif a.name=="Giao lưu cùng bạc": text=silver_social_action_description(a.amount)
        elif a.name=="Giao lưu bằng lụa": text=silk_social_action_description()
        elif a.name=="Giao lưu với trang sức vàng": text=gold_social_action_description()
        elif a.name=="Nghỉ": text=beeswax_rest_description()
        elif a.name in ["Vận tải","Di cư"]: text=travel_action_description(region_label(a.target),a.term,a.good==15)
        elif a.name=="Mua lương quân vụ": text="Mua %d phần %s · trả từ ngân khố vùng" % [a.amount,"lương khô" if a.good==0 else "khẩu phần bổ sung"]
        elif a.name=="Gửi lương quân vụ": text="Gửi tối đa %d phần %s tới vùng #%d" % [a.amount,"lương khô" if a.good==0 else "khẩu phần bổ sung",a.target]
        elif a.name=="Cấp quỹ công trình đô thị": text="Cấp %d%% ngân khố vào quỹ đô thị thử nghiệm · %d đ" % [a.good,a.amount]
        elif a.name=="Cứu trợ từ quỹ đô thị": text="Chia %d đ từ quỹ đô thị cho cư dân đang sống trong vùng này" % a.amount
        elif a.name=="Xây nhà ở công cộng": text="Xây thêm 8 chỗ ở · dùng 80 đ quỹ đô thị, 4 vật liệu xây dựng và 2 công cụ · hoàn thành sau 60 ngày game"
        elif a.name.begins_with("Tự ứng cử đại diện hội đồng tại "): text="Tự nguyện nộp hồ sơ làm đại diện cho settlement này · không mất tiền; lãnh chủ sẽ chấp thuận hoặc từ chối"
        elif a.name.begins_with("Rút hồ sơ ứng cử tại "): text="Rút hồ sơ của chính mình trước khi lãnh chủ quyết định"
        elif a.name.begins_with("Từ chức đại diện hội đồng tại "): text="Rời ghế đại diện; quyền dùng quỹ công trình đô thị dừng ngay"
        elif a.name.begins_with("Bổ nhiệm người tự ứng cử tại "): text="Bổ nhiệm người đã tự nguyện ứng cử tại đúng settlement này · nhiệm kỳ thử 4 năm game; đây không phải bầu cử"
        elif a.name.begins_with("Từ chối hồ sơ ứng cử tại "): text="Từ chối hồ sơ tại đúng settlement này; ghế được để trống"
        elif a.name=="Tạo cơ sở sản xuất":
            var industry=clampi(a.good,0,ESTABLISHMENT_NAMES.size()-1)
            text="Mở %s tại %s · vốn %d đ" % [ESTABLISHMENT_NAMES[industry],region_label(a.target),a.amount]
            if industry in [12,13,14]:
                text+=" · một mẻ cần 16 Work tick; nguồn ban đầu 100 mẻ tối đa; giá tham chiếu trial, chưa hiệu chuẩn"
                if industry in [12,13]: text+=" · cần thêm 1 nhiên liệu/mẻ"
                if a.term==-2: text="Dùng quỹ đô thị để "+text.trim_prefix("Mở ")
            elif industry==17:
                text+=" · mỗi mẻ cần 1 Timber và 16 tick Work"
            elif industry==18:
                text+=" · mỗi mẻ cần 1 source unit và 16 tick Work; nguồn đầu25/100, bổ sung25 mỗi90 ngày game"
            elif industry==19:
                text+=" · mỗi mẻ cần 1 source unit và 16 tick Work; nguồn đầu25/100, bổ sung25 mỗi90 ngày game"
            elif industry==20:
                text+=" · mỗi mẻ cần 1 source unit và 16 tick Work; nguồn đầu100/100, hữu hạn và không tái tạo"
        elif a.name=="Chuyển hàng xưởng":
            text=establishment_transfer_action_text(a.good,a.amount,a.target,a.term==2)
        elif a.name.begins_with("Nhập từ đối tác "): text="Nhập %d hàng khô từ %s · giá 24 đ/món, thuế 5%%, phí tàu %d và phí cảng 2 đ/món · đến cuối ngày %d" % [a.amount,a.name.trim_prefix("Nhập từ ").trim_suffix(" qua cảng"),int(foreign_trade.get("import_route",{}).get("fee_per_unit",0)),v.day+1+int(foreign_trade.get("import_route",{}).get("travel_days",0))]
        elif a.name.begins_with("Xuất cho đối tác "): text="Xuất %d muối cho %s · đối tác trả 28 đ/món, thuế 5%%, phí tàu %d và phí cảng 2 đ/món · thanh toán cuối ngày %d" % [a.amount,a.name.trim_prefix("Xuất cho ").trim_suffix(" qua cảng"),int(foreign_trade.get("export_route",{}).get("fee_per_unit",0)),v.day+1+int(foreign_trade.get("export_route",{}).get("travel_days",0))]
        elif a.name=="Đề nghị phục vụ": text=military_service_offer_text(a.target,a.amount,a.term)
        elif a.name=="Đề nghị tập kết quân tình nguyện": text="Đề nghị giao %d người đã tình nguyện tới người đứng đầu #%d trong 14 ngày" % [a.amount,a.target]
        elif a.name=="Xin gia hạn nghĩa vụ quân sự": text="Xin thêm 7 ngày cho cam kết đang thiếu người · người đứng đầu #%d quyết định · chỉ được xin một lần" % a.target
        elif a.name=="Đề nghị cung cấp lương thực quân vụ": text="Đề nghị cấp %d phần lương khô từ kho vùng #%d tới nơi tập kết · hạn14 ngày · người đứng đầu quyết định" % [a.amount,a.target]
        elif a.name=="Xin gia hạn cung cấp lương thực quân vụ": text="Xin thêm 7 ngày cho cam kết cấp lương khô đang thiếu · người đứng đầu #%d quyết định · chỉ được xin một lần" % a.target
        elif a.name=="Dự án": text="%s · %d đ · %d ngày cơ sở" % [["Xây đường","Xây trường","Đào tạo nghề","Đào tạo giáo viên"][a.good],a.amount,a.term]
        elif a.name=="Làm việc": text="Làm việc tại %s · %d ngày" % [region_label(a.target),a.term]
        elif a.name=="Bổ nhiệm": text="Bổ nhiệm ứng viên tiếp theo · 20 đ/tháng"
        elif a.name in ["Chấp thuận","Từ chối"]:
            text="%s hồ sơ #%d" % [a.name,a.target]
            for offer in v.offers:
                if offer.id==a.target:
                    if offer.kind==2: text+=" · đóng %d đ + vật tư" % offer.amount
                    elif offer.kind==4: text+=" · phục vụ %d ngày, lương %d đ/ngày, quỹ còn %d đ" % [offer.service_term,offer.amount,offer.escrow]
                    elif offer.kind==5:
                        var named_members=[]
                        for member_id in offer.get("roster_members",[]): named_members.append("#%d" % int(member_id))
                        text+=" · giao %d tình nguyện viên theo đúng danh sách %s · họ tự nguyện và phải tự tới nơi · hạn14 ngày" % [offer.amount,", ".join(named_members)]
                    elif offer.kind==6: text+=" · xin thêm7 ngày cho cam kết thiếu người; người nhận chấp thuận hoặc từ chối"
                    elif offer.kind==7: text+=" · cam kết cấp %d phần lương khô cho người tình nguyện · hạn14 ngày; phải mua bằng ngân khố và giao qua tuyến công" % offer.amount
                    elif offer.kind==8: text+=" · xin thêm7 ngày để giao nốt lương khô còn thiếu; người nhận chấp thuận hoặc từ chối"
                    else: text+=" · nhận %d đ" % offer.amount
                    break
        var action_index=int(a.index)
        var b=button(commands,text,func(): run_actor_command(actor,action_index,str(v.fingerprint)))
        b.set_meta("realm_actor",actor)
        b.set_meta("realm_action_index",action_index)
        b.disabled=not a.legal
        if a.name.begins_with("Nhập từ đối tác ") or a.name.begins_with("Xuất cho đối tác "):
            var partner_id=int(int(a.term)/2)
            var has_pending_offer=false
            for offer in foreign_trade.get("offers",[]):
                if int(offer.get("partner_id",-1))==partner_id and int(offer.get("status",-1))==0:
                    has_pending_offer=true
            var offer_direction="nhập" if int(a.term)%2==0 else "xuất"
            var offer_button=button(commands,"Đề nghị %s %d đơn vị với đối tác %s" % [offer_direction,a.amount,FOREIGN_PARTNER_LABELS[partner_id]],func(): run_foreign_trade_offer(actor,partner_id,int(a.term),a.good,a.amount,str(v.fingerprint)))
            offer_button.disabled=not a.legal or has_pending_offer
            offer_button.tooltip_text="Đối tác phải tự phản hồi. Tiền, hàng và chỗ chứa cần thiết được giữ tối đa 7 ngày game; nếu bị từ chối hoặc hết hạn, chúng được trả lại."
    facts.clear()
    var knowledge_by_fact={}
    for knowledge in v.get("knowledge",[]):
        var fact_id=int(knowledge.fact)
        if not knowledge_by_fact.has(fact_id): knowledge_by_fact[fact_id]=[]
        knowledge_by_fact[fact_id].append(player_knowledge_text(knowledge))
    for f in v.facts:
        var name_value=fact_kind_name(int(f.kind))
        var knowledge_note=" · "+" · ".join(knowledge_by_fact[int(f.id)]) if knowledge_by_fact.has(int(f.id)) else ""
        facts.append_text("Ngày %d · #%d  %s · %s · %d%s\n" % [f.day,f.actor,name_value,region_label(f.region),f.amount,knowledge_note])
    draft_box.visible=v.chosen<0
    summary.visible=v.chosen>=0
    if draft_box.visible and draft_box.get_child_count()==0:
        for card_value in v.cards:
            var card=int(card_value)
            button(draft_box,CARDS[card],func(): session.realm_draft(card,false);refresh())
        var reroll=button(draft_box,"Đổi thẻ đầu (1 lần)",func(): session.realm_draft(0,true);for child in draft_box.get_children(): draft_box.remove_child(child);child.queue_free();refresh())
        reroll.disabled=v.rerolled

func back_to_map() -> void:
    if not pause_core_and_stop_ui():
        if is_node_ready():
            refresh()
        return
    hide()
    session.command("pause")

func close_panel() -> void:
    if pause_core_and_stop_ui():
        hide()
    elif is_node_ready():
        refresh()

func pause_core_and_stop_ui() -> bool:
    running=false
    if not session.realm_pause():
        feedback.text="Không tạm dừng được lõi Realm"
        return false
    return true

func advance_automatic_day() -> void:
    if not running:
        return
    if session.realm_paused():
        running=false
        feedback.text="Lõi Realm đang tạm dừng"
        refresh()
        return
    if not session.realm_step(1):
        running=false
        feedback.text=str(session.realm_snapshot(actor).get("error","World đã dừng"))
    refresh()

func step_day() -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    if not session.realm_step_one_day():
        running=false
        feedback.text=str(session.realm_snapshot(actor).get("error","World đã dừng"))
    else:
        feedback.text="Đã tiến một ngày"
    refresh()

func start_atlas_manual_test() -> void:
    if running:
        pause_core_and_stop_ui()
    var ok=session.realm_atlas_manual_start(20261038,512)
    running=false
    feedback.text="Đã tạo thế giới Atlas 27 vùng, 512 cá thể. NPC không tự quyết định; mọi lệnh vẫn đi qua luật Realm." if ok else str(session.realm_snapshot(actor).get("error","Không tạo được thế giới Atlas"))
    refresh()

func run_private_residence_build(holding_id: int, expected: String) -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var result=session.realm_private_residence_build(actor,holding_id,expected)
    feedback.text="Đã bắt đầu xây nhà trên thửa #%d; cần 30 ngày game để hoàn tất." % holding_id if result.success else private_housing_error_text(str(result.error))
    refresh()

func run_private_housing_market_order(good: int, quantity: int, maximum_unit_price: int, region: int, expected: String) -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var result=session.realm_market_order(actor,0,0,region,good,quantity,maximum_unit_price,expected)
    feedback.text="Đã giữ tiền cho đơn mua; chợ sẽ giao hàng khi hết ngày game." if result.success else "Không đặt được đơn mua: "+str(result.get("error","Realm từ chối giao dịch"))
    refresh()

func add_silver_social_market_control(v: Dictionary, p: Dictionary) -> void:
    var region=int(p.get("region",-1))
    if region<0 or region>=v.get("regions",[]).size(): return
    var market: Dictionary=v.regions[region]
    var stock: Array=market.get("stock",[])
    var prices: Array=market.get("prices",[])
    var available=int(stock[7] if stock.size()>7 else 0)
    if available<1: return
    var price=int(prices[7] if prices.size()>7 else 0)
    var purchase=button(commands,"Đặt lệnh mua 1 Silver cho lựa chọn giao lưu",func():
        run_private_housing_market_order(7,1,price,region,str(v.fingerprint)))
    purchase.disabled=price<=0 or int(p.get("cash",0))<price
    purchase.tooltip_text="Cần %d đồng để giữ giá tối đa cho đơn; chợ giao hàng sau một ngày nếu còn tồn." % price

func run_private_housing_command(residence_id: int, operation: int, household_id: int, expected: String) -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var result=session.realm_private_housing_command(actor,residence_id,operation,household_id,2,expected)
    feedback.text="Đã gửi lệnh nhà ở; thời hạn, quyền và khoản tiền được Realm kiểm tra." if result.success else private_housing_error_text(str(result.error))
    refresh()

static func private_housing_error_text(reason: String) -> String:
    var messages={
        "Unknown actor":"Không tìm thấy cá thể thực hiện lệnh.",
        "Decision counter overflow":"Cá thể đã đạt giới hạn số lệnh.",
        "Stale or duplicate decision ID":"Lệnh đã cũ hoặc đã được gửi trước đó; hãy làm mới hồ sơ.",
        "A private home requires an available adult living at an Atlas urban site":"Chỉ người trưởng thành còn sống, đang rảnh và cư trú tại đô thị Atlas mới được xây nhà.",
        "Actor must own an urban holding at their current residence":"Cá thể phải đứng tên một thửa đất đô thị tại nơi mình đang cư trú.",
        "This holding already has a private home":"Thửa đất này đã có nhà tư.",
        "Building requires 40 personal coins, two ConstructionMaterial, one Tools, and one PlainGlass":"Cần 40 đồng cá nhân, hai vật liệu xây dựng, một công cụ và một kính thường.",
        "Private home construction limit reached":"Đã chạm giới hạn ngày hoặc số hồ sơ xây nhà.",
        "Private leases are only available in Atlas":"Nhà tư chỉ hoạt động trong thế giới Atlas.",
        "Trial rent is fixed at two coins per thirty game days":"Giá thuê thử nghiệm cố định là 2 đồng mỗi 30 ngày game.",
        "Unknown private home":"Không tìm thấy căn nhà này.",
        "Private home has no land title":"Căn nhà không còn hồ sơ quyền sở hữu đất hợp lệ.",
        "Only the living title holder can offer a completed vacant home to another household authority":"Chỉ chủ đất còn sống mới được mời hộ khác thuê căn nhà đã xây xong và đang trống.",
        "Tenant must be an adult household authority in an Atlas urban household":"Hộ thuê phải có người đại diện trưởng thành và đang cư trú tại đô thị Atlas.",
        "Household already has a private-home lease or offer":"Hộ này đã có hợp đồng thuê hoặc lời mời thuê đang chờ.",
        "This lease offer is not addressed to the actor's household":"Lời mời này không gửi cho hộ của cá thể đang chọn.",
        "Tenant must still live at this site and the home must be ready":"Người thuê phải còn cư trú tại đô thị này và căn nhà phải hoàn tất.",
        "Actor is neither the landlord nor tenant household authority":"Cá thể này không phải chủ nhà hay người đại diện hộ thuê.",
        "Unknown private housing operation":"Loại lệnh nhà ở không hợp lệ."
    }
    return messages.get(reason,"Realm đã từ chối lệnh nhà ở: "+reason)

func add_private_housing_controls(v: Dictionary, p: Dictionary) -> void:
    var heading=Label.new()
    heading.text="NHÀ TƯ / CHO THUÊ · mọi khoản tiền và quyền do Realm kiểm tra"
    heading.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
    commands.add_child(heading)
    var market_region=clampi(int(p.get("region",0)),0,26)
    var region_market: Dictionary=v.get("regions",[])[market_region]
    var stock: Array=region_market.get("stock",[])
    var prices: Array=region_market.get("prices",[])
    var inventory: Array=p.get("inventory",[])
    for requirement in [{"good":19,"quantity":2,"label":"vật liệu xây dựng"},{"good":16,"quantity":1,"label":"công cụ"},{"good":27,"quantity":1,"label":"kính thường"}]:
        var good=int(requirement.good)
        var owned=int(inventory[good] if good<inventory.size() else 0)
        var needed=maxi(0,int(requirement.quantity)-owned)
        if needed==0: continue
        var available=int(stock[good] if good<stock.size() else 0)
        var maximum_price=int(prices[good] if good<prices.size() else 0)
        var buy=button(commands,"Đặt lệnh mua %d %s · tối đa %d đ/món · hoàn tất sau một ngày" % [needed,str(requirement.label),maximum_price],func():
            run_private_housing_market_order(good,needed,maximum_price,int(p.get("region",0)),str(v.fingerprint)))
        buy.disabled=available<needed or maximum_price<=0 or int(p.get("cash",0))<needed*maximum_price
        if available<needed:
            buy.tooltip_text="Chợ vùng hiện chỉ có %d món; cần %d. Hàng phải được sản xuất hoặc vận chuyển vào chợ trước." % [available,needed]
        elif int(p.get("cash",0))<needed*maximum_price:
            buy.tooltip_text="Tiền cá nhân chưa đủ khoản tối đa cần giữ cho đơn hàng này."
    var holdings: Array=v.get("land_holdings",[])
    var own=OptionButton.new()
    var own_count=0
    for holding in holdings:
        if not holding.get("owned_by_actor",false): continue
        own.add_item("Thửa #%d · vùng %d" % [int(holding.id),int(holding.region)])
        own.set_item_metadata(own.item_count-1,int(holding.id))
        own_count+=1
    commands.add_child(own)
    var build=button(commands,"Xây nhà (40 đồng, 2 vật liệu, 1 công cụ, 1 kính thường; 30 ngày)",func():
        if own.item_count>0: run_private_residence_build(int(own.get_item_metadata(own.selected)),str(v.fingerprint)))
    var build_inventory: Array=p.get("inventory",[])
    var build_affordable=int(p.get("cash",0))>=40 and int(build_inventory[19] if build_inventory.size()>19 else 0)>=2 and int(build_inventory[16] if build_inventory.size()>16 else 0)>=1 and int(build_inventory[27] if build_inventory.size()>27 else 0)>=1
    build.disabled=own_count==0 or not build_affordable
    build.tooltip_text="Cần thửa đô thị do mình đứng tên, 40 đồng cá nhân, hai vật liệu xây dựng, một công cụ và một kính thường."
    if own_count==0:
        var no_title=Label.new();no_title.text="Cá thể này chưa đứng tên thửa đất nào được phép xem.";no_title.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART;commands.add_child(no_title)
    var tenant_picker=OptionButton.new()
    var households: Array=v.get("private_housing_households",[])
    for household in households:
        if int(household.get("authority_id",0))==actor: continue
        tenant_picker.add_item("Hộ #%d · người đại diện #%d" % [int(household.household_id),int(household.authority_id)])
        tenant_picker.set_item_metadata(tenant_picker.item_count-1,int(household.household_id))
    commands.add_child(tenant_picker)
    var homes: Array=v.get("private_residences",[])
    if homes.is_empty():
        var empty=Label.new();empty.text="Chưa có nhà tư trong phạm vi cá thể này được xem.";empty.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART;commands.add_child(empty)
    for home in homes:
        var home_id=int(home.get("id",0))
        var owner=int(home.get("owner_id",0))
        var tenant=int(home.get("tenant_household",0))
        var offered=int(home.get("offered_household",0))
        var line=Label.new()
        line.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
        line.text="Nhà #%d · thửa #%d · vùng %d · chủ #%d · %s" % [home_id,int(home.get("holding_id",0)),int(home.get("region",0)),owner,"đã xây xong" if home.get("completed",false) else "đang xây đến ngày %d" % int(home.get("construction_due_day",0))]
        if tenant>0: line.text+=" · hộ thuê #%d · %d đồng/30 ngày · kỳ tới ngày %d · hợp đồng hết ngày %d" % [tenant,int(home.get("rent_per_period",0)),int(home.get("next_rent_day",-1)),int(home.get("lease_end_day",-1))]
        elif offered>0: line.text+=" · đang mời hộ #%d · %d đồng/30 ngày · hết hạn ngày %d" % [offered,int(home.get("offered_rent",0)),int(home.get("offer_expires_day",-1))]
        else: line.text+=" · đang trống"
        commands.add_child(line)
        if owner==actor and home.get("completed",false) and tenant==0 and offered==0:
            var offer=button(commands,"Mời hộ được chọn thuê · 2 đồng/30 ngày",func():
                if tenant_picker.item_count>0: run_private_housing_command(home_id,0,int(tenant_picker.get_item_metadata(tenant_picker.selected)),str(v.fingerprint)))
            offer.disabled=tenant_picker.item_count==0
        if offered>0 and int(p.get("household",0))==offered:
            button(commands,"Chấp thuận đề nghị nhà #%d" % home_id,func(): run_private_housing_command(home_id,1,offered,str(v.fingerprint)))
            button(commands,"Từ chối đề nghị nhà #%d" % home_id,func(): run_private_housing_command(home_id,2,offered,str(v.fingerprint)))
        if tenant>0 and (owner==actor or int(p.get("household",0))==tenant):
            button(commands,"Kết thúc hợp đồng nhà #%d" % home_id,func(): run_private_housing_command(home_id,3,tenant,str(v.fingerprint)))

func toggle_run() -> void:
    if running:
        pause_core_and_stop_ui()
        refresh()
        return
    if session.realm_resume():
        running=true
        feedback.text="Đang chạy"
    else:
        running=false
        feedback.text=str(session.realm_snapshot(actor).get("error","Không tiếp tục được"))
    refresh()

func run_actor_command(actor_id: int, action_index: int, expected: String) -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var ok=session.realm_command(actor_id,action_index,expected)
    feedback.text="Đã thực hiện lệnh" if ok else str(session.realm_snapshot(actor_id).get("error","Lệnh bị từ chối"))
    refresh()

func run_military_combat_consent(actor_id: int, consent: bool, expected: String) -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var ok=session.realm_military_combat_consent(actor_id,consent,expected)
    feedback.text="Đã lưu lựa chọn tham chiến của cá thể này" if ok else str(session.realm_snapshot(actor_id).get("error","Lựa chọn bị từ chối"))
    refresh()

func run_military_raid(partner_id: int, force_units: int, expected: String) -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var result=session.realm_military_raid(partner_id,force_units,expected)
    feedback.text="Đã tạo tình huống đột kích thử; sổ tiền và lương khô của đối tác đã bị trừ." if result.success else str(result.error)
    refresh()

func run_military_battle(actor_id: int, raid_id: int, retreat: bool, defenders: PackedInt64Array, expected: String) -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var result=session.realm_military_battle(actor_id,raid_id,retreat,defenders,expected)
    feedback.text=("Đã ra lệnh rút quân." if retreat else "Đã xử lý trận đánh; xem kết quả và thương vong trong lịch sử.") if result.success else str(result.error)
    refresh()

func run_foreign_trade_suspension(actor_id: int, partner_id: int, suspended: bool, expected: String) -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var ok=session.realm_foreign_trade_suspension(actor_id,partner_id,suspended,expected)
    if ok:
        var partner_label: String = FOREIGN_PARTNER_LABELS[clampi(partner_id, 0, FOREIGN_PARTNER_LABELS.size() - 1)]
        feedback.text="Đã đình chỉ lệnh mới với đối tác %s" % partner_label if suspended else "Đã khôi phục lệnh mới với đối tác %s" % partner_label
    else:
        feedback.text=str(session.realm_snapshot(actor_id).get("error","Chính sách bị từ chối"))
    refresh()

func run_foreign_trade_offer(actor_id: int, partner_id: int, operation: int, good: int, quantity: int, expected: String) -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var ok=session.realm_foreign_trade_offer(actor_id,partner_id,operation,good,quantity,expected)
    if ok:
        feedback.text="Đã gửi đề nghị cho đối tác %s. Tài sản cần thiết được giữ tối đa 7 ngày game; đối tác phải tự phản hồi." % FOREIGN_PARTNER_LABELS[clampi(partner_id,0,FOREIGN_PARTNER_LABELS.size()-1)]
    else:
        feedback.text=str(session.realm_snapshot(actor_id).get("error","Đề nghị bị từ chối"))
    refresh()

func run_worksite_smoke() -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    var current=session.realm_snapshot(actor)
    if current.get("chosen",-1)<0:
        feedback.text="Chọn một thẻ trước khi chạy chu trình xưởng"
        refresh()
        return
    var result=session.realm_debug_worksite_cycle(actor,str(current.fingerprint))
    if result.success:
        feedback.text="Xưởng #%d · xơ sợi %d · vải %d · đã trả %d công · quỹ còn %d đ" % [result.establishment_id,result.fiber,result.cloth,result.paid_units,result.site_cash]
    else:
        feedback.text=str(result.error)
    refresh()

func save_game() -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    feedback.text="Đã lưu world + RNG + memory" if session.realm_save(save_path,false) else "Lưu thất bại"
    refresh()

func load_game() -> void:
    if not pause_core_and_stop_ui():
        feedback.text="Không tạm dừng được lõi Realm"
        refresh()
        return
    feedback.text="Đã khôi phục; đang tạm dừng" if session.realm_save(save_path,true) else "Tải thất bại; giữ world hiện tại"
    refresh()

func show_guide() -> void:
    pause_core_and_stop_ui()
    var dialog=AcceptDialog.new()
    dialog.title="Hướng dẫn sandbox"
    dialog.dialog_text="Chọn một thẻ mở sổ, rồi chạy thế giới hoặc tiến một ngày.\nBa policy PPO dùng chung trọng số theo họ; từng người có memory và sampling riêng.\nĐổi ID để xem cá thể, quyền biết và lệnh hợp lệ. Lệnh UI đi cùng pipeline với AI.\nViệc làm, học, vận tải và điều tra có thời lượng; nợ/dự án có hạn ngày game.\nLưu chứa world, RNG, counters và memory; tải luôn tạm dừng.\nĐây là sandbox M3 thử nghiệm; không phải campaign hoàn chỉnh.\nMột số augment/chính trị/stat còn giới hạn được ghi trong báo cáo."
    add_child(dialog)
    dialog.popup_centered(Vector2i(700,320))
    refresh()

func _process(delta: float) -> void:
    if delta>.25:
        if pause_core_and_stop_ui():
            feedback.text="Khung hình quá dài; đã tạm dừng"
        refresh()

func _notification(what: int) -> void:
    if what==NOTIFICATION_APPLICATION_FOCUS_OUT or what==NOTIFICATION_WM_WINDOW_FOCUS_OUT:
        pause_core_and_stop_ui()
        if is_node_ready():
            refresh()
