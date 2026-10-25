extends Control
const MapView = preload("res://territory_map.gd")
const GOLD = Color("dfbf7a")
const TEAL = Color("67c6b2")
var VERBS = ["move","work","buy","meal","drink","rest","offer","talk","accept","study","grant","goal","cancel"]
var VERB_LABELS = ["Đi tới địa điểm","Nhận việc đã đăng","Mua hàng tại site","Chuẩn bị bữa ăn","Uống tại giếng","Nghỉ tại nhà","Chủ đăng việc","Mời trò chuyện","Nhận lời mời","Đọc công báo","Chi công cho người","Đổi mục tiêu","Hủy hoạt động"]
const SITE_NAMES = {10:"Cung điện",11:"Ruộng · food4 + thuế1",12:"Xưởng · tool8 + thuế1",13:"Chợ",14:"Giếng công",15:"Nhà ở"}
const PEOPLE_NAMES = {1:"Vua",2:"Điền · chủ ruộng",3:"Mộc · chủ xưởng",4:"An · người lao động",5:"Bình · người lao động"}
var world: Dictionary = {}
var actor := 1
var debug_mode := false
var playing := false
var elapsed := 0.0
var session_path: String
var save_path: String
var bridge_path: String
var selected_site := 10
var selected_person := 1
var selection_kind := "person"
var demo_index := -1
var last_reply: Dictionary = {}
var clock_label: Label
var mode_label: Label
var status_label: Label
var result_label: Label
var site_label: RichTextLabel
var person_label: RichTextLabel
var log_label: RichTextLabel
var fact_label: RichTextLabel
var action_hint: Label
var demo_label: Label
var save_button: Button
var load_button: Button
var step_button: Button
var play_button: Button
var debug_check: CheckButton
var actor_box: OptionButton
var verb_box: OptionButton
var target_box: OptionButton
var amount_box: SpinBox
var meal_box: CheckBox
var goal_edit: LineEdit
var map_view: Control
var demo_button: Button
var mode_box: OptionButton
var named_controls: Dictionary={}
var named_rows: Dictionary={}
var action_catalog: Dictionary={}
var command_button: Button
var action_test_records: Array=[]
var contract_label: RichTextLabel
var configured_verb := ""
const NEW_LABELS = {"revoke_job":"Thu hồi việc chưa nhận","produce":"Chủ tự sản xuất","manage":"Lấy / gửi kho của mình","transfer":"Chuyển tiền / tặng hàng","equip":"Thay công cụ đã lắp","propose":"Đề nghị có điều khoản","counter":"Trả giá đề nghị","respond":"Trả lời đề nghị","fulfill":"Hoàn thành giao hàng","repay":"Trả nợ","end_lease":"Kết thúc thuê của mình","collect":"Lấy nước vào túi","care":"Chăm sóc hồi phục","invitation":"Từ chối / thu hồi lời mời","inspect":"Quan sát tại chỗ","dismiss":"Bãi nhiệm chức ngân quỹ","announce":"Đăng công báo"}
const PROPOSAL_NAMES = ["Bán hàng","Cho vay","Cho thuê chỗ ở","Nhờ giao hàng","Bổ nhiệm ngân quỹ"]
var demo_actions := [
	[4,"move",11,1,0,"An đến ruộng để xem cơ hội"],
	["step",1,"Di chuyển mất 10 phút"],
	[4,"work",11,1,0,"An tự nhận job; khóa lương6 và giống1"],
	["step",3,"Có công thực, chưa trả lương khi mẻ chưa xong"],
	["save",0,"Lưu giữa hoạt động"],
	["step",6,"Hoàn tất: giống1 thành food3, An nhận6"],
	[4,"buy",11,2,0,"Mua2 food: trả chủ8, công quỹ2"],
	[4,"meal",0,1,1,"Chọn bữa ngon: dùng2 food, chuẩn bị30 phút"],
	["step",3,"Đói giảm35, vui tăng10; hai chỉ số riêng"],
	[1,"grant",5,10,0,"Vua chi công10 cho Bình; có đối ứng thật"],
	[5,"move",12,1,0,"Bình đi xưởng"],
	["step",1,"Đến xưởng"],
	[5,"work",12,1,0,"Bình chấp nhận công việc từ chủ Mộc"],
	["step",9,"Gỗ2 thành tool1; Bình nhận lương6"],
	[5,"buy",12,1,0,"Bình mua tool1: chủ nhận8, thuế1"],
	[4,"move",14,1,0,"An chọn tìm nước"],
	["step",1,"An tới giếng"],
	[4,"drink",0,1,0,"Một đơn vị nước hữu hạn từ giếng"],
	["step",1,"Uống xong, khát giảm45"],
	[4,"move",15,1,0,"An về nơi có quyền cư trú"],
	["step",1,"Đến nhà"],
	[4,"rest",0,1,0,"An chọn nghỉ30 phút"],
	["step",3,"Mệt giảm qua từng bước nghỉ"],
	["save",0,"Lưu cuối vòng; có thể tải lại và kiểm tiếp"]
]
func label(text: String, font_size: int=16, color: Color=Color("dce5dc")) -> Label:
	var l := Label.new()
	l.text=text
	l.add_theme_font_size_override("font_size",font_size)
	l.add_theme_color_override("font_color",color)
	return l
func button(text: String, callback: Callable, parent: Node) -> Button:
	var b := Button.new()
	b.text=text
	b.custom_minimum_size.y=34
	b.pressed.connect(callback)
	parent.add_child(b)
	return b
func rich(height: int) -> RichTextLabel:
	var r := RichTextLabel.new()
	r.bbcode_enabled=true
	r.custom_minimum_size.y=height
	r.fit_content=false
	r.add_theme_font_size_override("normal_font_size",15)
	return r
func _ready() -> void:
	DisplayServer.window_set_min_size(Vector2i(1200,840))
	bridge_path=ProjectSettings.globalize_path("res://polyec_core.exe")
	session_path=ProjectSettings.globalize_path("user://live-a2.pe")
	save_path=ProjectSettings.globalize_path("user://checkpoint-a2.pe")
	if "--ui-test" in OS.get_cmdline_user_args() or "--action-test" in OS.get_cmdline_user_args():
		session_path=ProjectSettings.globalize_path("user://ui-test-live-a2.pe")
		save_path=ProjectSettings.globalize_path("user://ui-test-checkpoint-a2.pe")
	build_ui()
	if FileAccess.file_exists(session_path):
		invoke("view")
	else:
		invoke("reset",["251025"])
	if "--ui-test" in OS.get_cmdline_user_args():
		call_deferred("ui_smoke")
	elif "--action-test" in OS.get_cmdline_user_args():
		call_deferred("ui_action_smoke")
func build_ui() -> void:
	var theme_new := Theme.new()
	theme_new.default_font_size=15
	var box := StyleBoxFlat.new()
	box.bg_color=Color("23383b")
	box.set_corner_radius_all(5)
	box.content_margin_left=12
	box.content_margin_right=12
	box.content_margin_top=4
	box.content_margin_bottom=4
	theme_new.set_stylebox("normal","Button",box)
	var hover := box.duplicate()
	hover.bg_color=Color("38504d")
	theme_new.set_stylebox("hover","Button",hover)
	var pressed := box.duplicate()
	pressed.bg_color=Color("52604e")
	theme_new.set_stylebox("pressed","Button",pressed)
	self.theme=theme_new
	var margin := MarginContainer.new()
	margin.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	for key in ["margin_left","margin_right","margin_top","margin_bottom"]:
		margin.add_theme_constant_override(key,18)
	add_child(margin)
	var root := VBoxContainer.new()
	root.add_theme_constant_override("separation",10)
	margin.add_child(root)
	var head := HBoxContainer.new()
	root.add_child(head)
	head.add_child(label("POLYEC",30,GOLD))
	head.add_child(label("   VƯƠNG QUYỀN & ĐỜI SỐNG",16,Color("91ada6")))
	var spacer := Control.new()
	spacer.size_flags_horizontal=Control.SIZE_EXPAND_FILL
	head.add_child(spacer)
	clock_label=label("",20,TEAL)
	head.add_child(clock_label)
	var toolbar := HBoxContainer.new()
	root.add_child(toolbar)
	play_button=button("▶ Chạy",toggle_play,toolbar)
	step_button=button("+10 phút",func(): invoke("step",["1"]),toolbar)
	save_button=button("Lưu",func(): playing=false; invoke("save",[save_path]),toolbar)
	load_button=button("Tải",func(): playing=false; invoke("load",[save_path]),toolbar)
	button("Thế giới mới",reset_world,toolbar)
	debug_check=CheckButton.new()
	debug_check.text="DEBUG toàn tri / điều khiển NPC"
	debug_check.toggled.connect(set_debug)
	toolbar.add_child(debug_check)
	actor_box=OptionButton.new()
	for id in PEOPLE_NAMES:
		actor_box.add_item(str(id)+" · "+PEOPLE_NAMES[id],id)
	actor_box.item_selected.connect(func(index): actor=actor_box.get_item_id(index); selected_person=actor; selection_kind="person"; invoke("view"); populate_targets())
	actor_box.disabled=true
	toolbar.add_child(actor_box)
	mode_label=label("",14,GOLD)
	root.add_child(mode_label)
	var body := HBoxContainer.new()
	body.add_theme_constant_override("separation",16)
	body.size_flags_vertical=Control.SIZE_EXPAND_FILL
	root.add_child(body)
	var left := VBoxContainer.new()
	left.size_flags_horizontal=Control.SIZE_EXPAND_FILL
	body.add_child(left)
	map_view=MapView.new()
	map_view.custom_minimum_size=Vector2(660,350)
	map_view.size_flags_vertical=Control.SIZE_EXPAND_FILL
	map_view.selected.connect(on_map_selection)
	left.add_child(map_view)
	site_label=rich(66)
	left.add_child(site_label)
	var demo_row := HBoxContainer.new()
	left.add_child(demo_row)
	button("Bắt đầu vòng thử",start_demo,demo_row)
	demo_button=button("Bước kịch bản →",next_demo,demo_row)
	demo_button.disabled=true
	demo_label=label("Kịch bản kiểm luật có điều khiển; chưa có AI học.",13,TEAL)
	demo_label.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
	left.add_child(demo_label)
	var side_scroll := ScrollContainer.new()
	side_scroll.custom_minimum_size.x=410
	side_scroll.size_flags_horizontal=Control.SIZE_FILL
	body.add_child(side_scroll)
	var side := VBoxContainer.new()
	side.custom_minimum_size.x=390
	side.size_flags_horizontal=Control.SIZE_EXPAND_FILL
	side.add_theme_constant_override("separation",8)
	side_scroll.add_child(side)
	side.add_child(label("CÁ NHÂN & HÀNH ĐỘNG",17,GOLD))
	person_label=rich(210)
	side.add_child(person_label)
	verb_box=OptionButton.new()
	for i in range(VERBS.size()):
		verb_box.add_item(VERB_LABELS[i],i)
	verb_box.item_selected.connect(func(_i): populate_targets())
	side.add_child(verb_box)
	target_box=OptionButton.new()
	side.add_child(target_box)
	mode_box=OptionButton.new()
	mode_box.item_selected.connect(func(_i): populate_targets())
	side.add_child(mode_box)
	var params := HBoxContainer.new()
	side.add_child(params)
	params.add_child(label("Lượng / đồng"))
	amount_box=SpinBox.new()
	amount_box.min_value=1
	amount_box.max_value=1000
	amount_box.value=1
	params.add_child(amount_box)
	meal_box=CheckBox.new()
	meal_box.text="Bữa ngon (2 food / 30 phút)"
	side.add_child(meal_box)
	for name in ["good","price","duration","source_site","site","wage"]:
		var row:=HBoxContainer.new()
		side.add_child(row)
		named_rows[name]=row
		var labels={"good":"Loại hàng","price":"Giá / lãi / tiền công","duration":"Hạn (bước10 phút)","source_site":"Nguồn kho của mình","site":"Nơi cho thuê","wage":"Lương mỗi mẻ"}
		row.add_child(label(labels[name],14))
		if name in ["good","source_site","site"]:
			var choice:=OptionButton.new()
			choice.size_flags_horizontal=Control.SIZE_EXPAND_FILL
			if name=="good":
				for good_name in ["Thực phẩm","Giống","Gỗ","Công cụ","Nước"]:
					choice.add_item(good_name,choice.item_count)
			elif name=="source_site":
				choice.add_item("Túi cá nhân",0)
				for site_id in SITE_NAMES:
					choice.add_item(SITE_NAMES[site_id],site_id)
			else:
				choice.add_item("Nhà ở #15",15)
			row.add_child(choice)
			named_controls[name]=choice
		else:
			var spin:=SpinBox.new()
			spin.min_value=0 if name=="price" else 1
			spin.max_value=1440 if name=="duration" else 100
			spin.value=12 if name=="duration" else (6 if name=="wage" else 1)
			row.add_child(spin)
			named_controls[name]=spin
	goal_edit=LineEdit.new()
	goal_edit.placeholder_text="Nội dung mục tiêu / công báo"
	goal_edit.max_length=160
	side.add_child(goal_edit)
	action_hint=label("",13,Color("9cafab"))
	action_hint.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
	side.add_child(action_hint)
	command_button=button("GỬI LỆNH THẬT  →",send_selected,side)
	result_label=label("",14,TEAL)
	result_label.autowrap_mode=TextServer.AUTOWRAP_WORD_SMART
	side.add_child(result_label)
	side.add_child(label("ĐỀ NGHỊ · NỢ · THUÊ",14,GOLD))
	contract_label=rich(110)
	side.add_child(contract_label)
	side.add_child(label("TIN ĐÃ BIẾT · nguồn / thời điểm",14,GOLD))
	fact_label=rich(90)
	fact_label.scroll_following=true
	side.add_child(fact_label)
	var bottom_head := HBoxContainer.new()
	root.add_child(bottom_head)
	bottom_head.add_child(label("DIỄN BIẾN & NGUYÊN NHÂN",15,GOLD))
	status_label=label("",13,Color("91ada6"))
	status_label.size_flags_horizontal=Control.SIZE_EXPAND_FILL
	status_label.horizontal_alignment=HORIZONTAL_ALIGNMENT_RIGHT
	bottom_head.add_child(status_label)
	log_label=rich(116)
	log_label.scroll_following=true
	root.add_child(log_label)
	populate_targets()
func set_debug(enabled: bool) -> void:
	debug_mode=enabled
	actor_box.disabled=not enabled
	if not enabled:
		actor=1
		actor_box.select(0)
	invoke("view")
func invoke(operation: String, extras: Array=[]) -> Dictionary:
	var output: Array=[]
	var args: Array=[session_path,operation,str(actor if debug_mode else 1),"1" if debug_mode else "0"]
	args.append_array(extras)
	var code := OS.execute(bridge_path,PackedStringArray(args),output,false,false)
	var raw := "".join(output)
	var reply = JSON.parse_string(raw)
	if not reply is Dictionary:
		result_label.text="Lỗi bridge native (%d): %s" % [code,raw.left(160)]
		result_label.modulate=Color("f09b80")
		playing=false
		return {"ok":false,"message":"Bridge unavailable"}
	last_reply=reply
	if reply.has("world"):
		world=reply.world
		refresh()
	result_label.text=("✓ " if reply.get("ok",false) else "TỪ CHỐI · ")+str(reply.get("message",""))
	result_label.modulate=TEAL if reply.get("ok",false) else Color("f09b80")
	if not reply.get("ok",false):
		playing=false
	return reply
func command(id: int,verb: String,target: int=0,amount: int=1,option: int=0,text: String="",params: Dictionary={}) -> Dictionary:
	var args: Array=[str(id),verb,str(target),str(amount),str(option),str(int(world.get("revision",0))),text if not text.is_empty() else "__EMPTY__"]
	for key in params:
		args.append(str(key)+"="+str(int(params[key])))
	return invoke("command",args)
func toggle_play() -> void:
	playing=not playing
	elapsed=0
	refresh()
func _process(delta: float) -> void:
	if not playing:
		return
	if delta>0.5 or not DisplayServer.window_is_focused():
		playing=false
		elapsed=0
		refresh()
		return
	elapsed+=delta
	if elapsed>=1.0:
		elapsed=0
		invoke("step",["1"])
func _notification(what: int) -> void:
	if what==NOTIFICATION_APPLICATION_FOCUS_OUT:
		playing=false
		elapsed=0
		if is_instance_valid(play_button):
			refresh()
func reset_world() -> void:
	playing=false
	demo_index=-1
	demo_button.disabled=true
	invoke("reset",["251025"])
func on_map_selection(kind: String,id: int) -> void:
	selection_kind=kind
	if kind=="person":
		selected_person=id
	else:
		selected_site=id
		if VERBS[verb_box.selected] in ["move","work","buy","offer"]:
			for i in range(target_box.item_count):
				if target_box.get_item_id(i)==id:
					target_box.select(i)
	refresh()
func sync_catalog() -> void:
	for definition in world.get("actions",[]):
		action_catalog[definition.verb]=definition
		if not definition.verb in VERBS:
			VERBS.append(definition.verb)
			var title: String=NEW_LABELS.get(definition.verb,definition.verb)
			VERB_LABELS.append(title)
			verb_box.add_item(title,VERBS.size()-1)
func populate_targets() -> void:
	if not is_instance_valid(target_box) or not is_instance_valid(mode_box):
		return
	var previous: int=target_box.get_selected_id()
	target_box.clear()
	var verb: String=VERBS[verb_box.selected]
	if configured_verb!=verb:
		configured_verb=verb
		mode_box.clear()
		var modes: Dictionary={"propose":PROPOSAL_NAMES,"respond":["Chấp nhận","Từ chối","Thu hồi / hủy cam kết"],"manage":["Lấy từ kho","Gửi vào kho"],"transfer":["Tặng tiền","Tặng hàng"],"drink":["Uống tại giếng","Dùng nước trong túi"],"invitation":["Người nhận từ chối","Người mời thu hồi"]}
		for title in modes.get(verb,[]):
			mode_box.add_item(title,mode_box.item_count)
		mode_box.visible=mode_box.item_count>0
	meal_box.visible=verb=="meal"
	goal_edit.visible=verb in ["goal","announce"]
	var names: Array=action_catalog.get(verb,{}).get("named",[])
	if verb=="propose":
		var proposal_fields: Array=[["good","price","duration","source_site"],["price","duration"],["price","duration","site"],["good","price","duration"],["duration"]]
		names=proposal_fields[max(0,mode_box.selected)]
	amount_box.get_parent().visible=verb in ["buy","transfer","manage","grant","repay"] or (verb=="propose" and mode_box.selected in [0,1,3])
	for name in named_rows:
		named_rows[name].visible=name in names
	if verb in ["talk","grant","transfer","propose"]:
		if verb=="propose" and mode_box.selected in [0,2]:
			target_box.add_item("Chào công khai tại đây",0)
		for id in PEOPLE_NAMES:
			if id!=actor:
				target_box.add_item(str(id)+" · "+PEOPLE_NAMES[id],id)
	elif verb in ["accept","invitation"]:
		for r in world.get("requests",[]):
			if int(r.status)==0 and (int(r.to)==actor or (verb=="invitation" and int(r.from)==actor)):
				target_box.add_item("Lời mời #%d: %d → %d" % [r.id,r.from,r.to],int(r.id))
	elif verb in ["respond","counter","fulfill"]:
		for p in world.get("proposals",[]):
			if int(p.status)==0 or (verb in ["respond","fulfill"] and int(p.status)==1):
				target_box.add_item("#%d · %s · %d → %d · chờ%d" % [p.id,PROPOSAL_NAMES[int(p.kind)],p.from,p.to,p.awaiting],int(p.id))
	elif verb=="repay":
		for debt in world.get("debts",[]):
			if int(debt.borrower)==actor and int(debt.outstanding)>0:
				target_box.add_item("Nợ #%d · còn%d · hạn t%d" % [debt.id,debt.outstanding,debt.due],int(debt.id))
	elif verb=="end_lease":
		for lease in world.get("leases",[]):
			if int(lease.tenant)==actor and int(lease.status)==0:
				target_box.add_item("Thuê #%d · nơi%d · hết t%d" % [lease.id,lease.site,lease.end],int(lease.id))
	elif verb in ["move","work","buy","offer","revoke_job","produce","manage","equip"]:
		for id in SITE_NAMES:
			target_box.add_item(SITE_NAMES[id],id)
			if id==selected_site:
				target_box.select(target_box.item_count-1)
	else:
		target_box.add_item("Tại vị trí hiện tại / quyền của actor",0)
	if target_box.item_count==0:
		target_box.add_item("Chưa có đối tượng phù hợp được biết",0)
	for i in range(target_box.item_count):
		if target_box.get_item_id(i)==previous:
			target_box.select(i)
	if is_instance_valid(action_hint):
		var hints={"propose":"Giữ tài sản bên đưa đề nghị; chỉ chốt khi bên kia chấp nhận. Giá: mỗi đơn vị hàng / lãi cả khoản vay / tiền thuê cả kỳ / thưởng giao hàng. Hạn là bước10 phút.","respond":"Phải đúng bên đang chờ phản hồi. Nhận lời giao hàng chỉ tạo cam kết; còn phải đem hàng thật tới nơi và Hoàn thành.","counter":"Chỉ trả giá sale/lease đang chờ. Sau trả giá, phía kia phải đồng ý; tiền chưa bị trừ.","repay":"Chỉ người vay tự trả từ ví riêng. Quá hạn không tự trừ tiền hoặc sinh tiền phạt.","end_lease":"Người thuê kết thúc quyền; tiền thuê trả trước không tự hoàn. Người tại chỗ không bị xóa/teleport.","care":"Ở nhà có quyền; dùng food1 + nước1 và60 phút. Nhu cầu quá thiếu sẽ cản hồi phục.","equip":"Chỉ chủ ở site, không có mẻ đang làm; dùng1 tool thật, thay thiết bị cũ và khôi phục12 lượt dùng.","produce":"Chủ tự dùng công sức và nguyên liệu của mình; không giả trả lương cho chính mình.","dismiss":"Vua bãi nhiệm quyền ngân quỹ; tài sản công vẫn thuộc tổ chức.","grant":"Chỉ người đang giữ chức ngân quỹ được chi, tối đa50/lệnh.","meal":"Bữa đơn giản food1/10 phút/vui+2; bữa ngon food2/30 phút/vui+10. Cùng giảm đói35."}
		action_hint.text=hints.get(verb,action_catalog.get(verb,{}).get("description",""))
func send_selected() -> void:
	var verb: String=VERBS[verb_box.selected]
	var params: Dictionary={}
	for name in action_catalog.get(verb,{}).get("named",[]):
		if not named_rows[name].visible:
			continue
		var control: Control=named_controls[name]
		params[name]=control.get_selected_id() if control is OptionButton else int(control.value)
	var option: int=mode_box.get_selected_id() if mode_box.visible else 0
	if verb=="meal":
		option=1 if meal_box.button_pressed else 0
	command(actor,verb,target_box.get_selected_id(),int(amount_box.value) if amount_box.get_parent().visible else 1,option,goal_edit.text,params)
func inv_text(inv: Array) -> String:
	return "food%d · giống%d · gỗ%d · tool%d · nước%d" % [inv[0],inv[1],inv[2],inv[3],inv[4]]
func refresh() -> void:
	if world.is_empty():
		return
	sync_catalog()
	if selection_kind=="person":
		for p in world.people:
			if int(p.id)==selected_person:
				selected_site=int(p.location)
	var t: int=int(world.tick)*10+480
	clock_label.text="Ngày %d   %02d:%02d" % [t/1440+1,(t/60)%24,t%60]
	mode_label.text=("DEBUG TOÀN TRI · lệnh NPC để kiểm luật · chưa có learned policy" if debug_mode else "NGƯỜI CHƠI: VUA · chỉ thấy thông tin đã biết, bản đồ công khai")
	play_button.text="Ⅱ Tạm dừng" if playing else "▶ Chạy"
	status_label.text="Công quỹ %d đồng  ·  seed %d  ·  rev %d  ·  10 phút/bước" % [world.treasury,world.seed,world.revision]
	map_view.world=world
	map_view.selected_site=selected_site
	map_view.selected_person=selected_person
	map_view.queue_redraw()
	for l in world.locations:
		if int(l.id)==selected_site:
			site_label.text="[color=#dfbf7a]%s[/color]   · chủ #%d\n" % [SITE_NAMES[int(l.id)],l.owner]
			if l.has("stock"):
				site_label.text+="Kho: "+inv_text(l.stock)+("   • job lương%d đang mở" % int(l.wage) if l.job else "   • không có job mở")+" · tool còn%d mẻ" % int(l.tool_uses)
			else:
				site_label.text+="Kho/việc chưa quan sát ở hiện tại. Đến nơi để biết; tin cũ nằm trong hồ sơ."
	var selected: Dictionary={}
	for p in world.people:
		if int(p.id)==selected_person:
			selected=p
	if selected.is_empty():
		person_label.text="Người này ngoài tầm quan sát hiện tại.\nChuyển DEBUG nếu cần kiểm toàn bộ world."
	elif not selected.has("money"):
		person_label.text="[color=#dfbf7a]%s #%d[/color]\nCùng địa điểm; ví, cơ thể và mục tiêu là riêng tư." % [selected.name,selected.id]
	else:
		var p: Dictionary=selected
		person_label.text="[color=#dfbf7a]%s #%d[/color]  ·  %d đồng  ·  tại #%d\n" % [p.name,p.id,p.money,p.location]
		person_label.text+="Đói %.1f · Khát %.1f · Mệt %.1f\nSức khỏe %.1f · Vui %.1f\n" % [p.hunger,p.thirst,p.fatigue,p.health,p.joy]
		person_label.text+=inv_text(p.inventory)+"\n"
		person_label.text+="[color=#67c6b2]%s %.1f / %.1f[/color]  · escrow %d\n" % [p.activity,p.progress,p.required,p.escrow]
		person_label.text+="Mục tiêu: %s\nXP ruộng %.0f / xưởng %.0f / giao tiếp %.0f\n" % [p.goal,p.skills[0],p.skills[1],p.skills[2]]
		person_label.text+="Xu hướng an toàn / hưởng thụ / dịch chuyển: %d / %d / %d" % [p.tendencies[0],p.tendencies[1],p.tendencies[2]]
	log_label.text=""
	var events: Array=world.get("events",[])
	for i in range(max(0,events.size()-6),events.size()):
		var e: Dictionary=events[i]
		log_label.text+="[color=#67c6b2]t%d[/color]  %s\n" % [e.tick,e.text]
	contract_label.text=""
	for p in world.get("proposals",[]):
		if int(p.status) in [0,1]:
			contract_label.text+="#%d %s · %d → %d · giá%d · chờ%d · hạn t%d\n" % [p.id,PROPOSAL_NAMES[int(p.kind)],p.from,p.to,p.price,p.awaiting,p.deadline]
	for debt in world.get("debts",[]):
		contract_label.text+="Nợ#%d: %d → %d · còn%d · hạn t%d\n" % [debt.id,debt.borrower,debt.lender,debt.outstanding,debt.due]
	for lease in world.get("leases",[]):
		contract_label.text+="Thuê#%d: người%d nơi%d · hết t%d · trạng thái%d\n" % [lease.id,lease.tenant,lease.site,lease.end,lease.status]
	populate_targets()
	fact_label.text=""
	var facts: Array=world.get("facts",[])
	for i in range(max(0,facts.size()-3),facts.size()):
		var f: Dictionary=facts[i]
		fact_label.text+="#%d · nguồn%d · xảy ra t%d / nhận t%d: %s\n" % [f.event,f.source,f.happened,f.received,f.text]
func start_demo() -> void:
	playing=false
	debug_check.set_pressed_no_signal(true)
	debug_mode=true
	actor_box.disabled=false
	actor=4
	actor_box.select(3)
	selected_person=4
	invoke("reset",["251025"])
	demo_index=0
	demo_button.disabled=false
	demo_label.text="SCRIPT KIỂM LUẬT · Bấm từng bước; không có NPC tự quyết định thay bạn."
func next_demo() -> void:
	if demo_index<0 or demo_index>=demo_actions.size():
		return
	playing=false
	var a: Array=demo_actions[demo_index]
	var r: Dictionary
	if a[0] is String:
		r=invoke(str(a[0]),[save_path] if a[0]=="save" else [str(a[1])])
	else:
		actor=int(a[0])
		actor_box.select(actor-1)
		selected_person=actor
		r=command(actor,str(a[1]),int(a[2]),int(a[3]),int(a[4]))
	if not r.get("ok",false):
		demo_label.text="SCRIPT DỪNG · "+str(r.get("message","lỗi"))
		return
	demo_label.text="SCRIPT %d/%d · %s" % [demo_index+1,demo_actions.size(),a.back()]
	demo_index+=1
	if demo_index==demo_actions.size():
		demo_button.disabled=true
		demo_label.text+=" · Vòng thử hoàn tất. Tiếp tục ra lệnh hoặc tải checkpoint."
func ui_smoke() -> void:
	var report: Array=[]
	start_demo()
	for i in range(demo_actions.size()):
		demo_button.pressed.emit()
		report.append({"index":i,"ok":last_reply.get("ok",false),"tick":world.get("tick",-1),"message":last_reply.get("message","")})
		if not last_reply.get("ok",false):
			break
		if i==8 and "--capture" in OS.get_cmdline_user_args():
			selected_site=11
			refresh()
			await get_tree().process_frame
			await RenderingServer.frame_post_draw
			var capture_args:=OS.get_cmdline_user_args()
			var path: String=capture_args[capture_args.find("--capture")+1]
			get_viewport().get_texture().get_image().save_png(path)
	var final_state=world.duplicate(true)
	save_button.pressed.emit()
	step_button.pressed.emit()
	load_button.pressed.emit()
	report.append({"save_load_exact":world==final_state})
	debug_check.set_pressed_no_signal(false)
	set_debug(false)
	var private_filtered := true
	for p in world.people:
		if int(p.id)!=1 and p.has("money"):
			private_filtered=false
	report.append({"player_filtered":private_filtered,"debug":world.debug})
	var tick_before: int=int(world.tick)
	playing=true
	_notification(NOTIFICATION_APPLICATION_FOCUS_OUT)
	_process(0.2)
	report.append({"focus_pause":not playing and int(world.tick)==tick_before})
	playing=true
	_process(3.0)
	report.append({"sleep_no_catchup":not playing and int(world.tick)==tick_before})
	var f:=FileAccess.open(ProjectSettings.globalize_path("user://ui-smoke.json"),FileAccess.WRITE)
	f.store_string(JSON.stringify(report,"  "))
	print("UI_SMOKE ",JSON.stringify(report))
	var passed: bool=demo_index==demo_actions.size()
	for record in report:
		for key in ["ok","save_load_exact","player_filtered","focus_pause","sleep_no_catchup"]:
			if record.has(key) and not record[key]:
				passed=false
	get_tree().quit(0 if passed else 1)

func ui_action(id: int,verb: String,target: int=0,amount: int=1,option: int=0,params: Dictionary={},text: String="") -> void:
	actor=id
	selected_person=id
	actor_box.select(id-1)
	invoke("view")
	verb_box.select(VERBS.find(verb))
	verb_box.item_selected.emit(verb_box.selected)
	if mode_box.visible:
		mode_box.select(option)
		mode_box.item_selected.emit(option)
	for name in params:
		var input: Control=named_controls[name]
		if input is OptionButton:
			for i in range(input.item_count):
				if input.get_item_id(i)==int(params[name]):
					input.select(i)
		else:
			input.value=int(params[name])
	amount_box.value=amount
	goal_edit.text=text
	var found := false
	for i in range(target_box.item_count):
		if target_box.get_item_id(i)==target:
			target_box.select(i)
			found=true
	if not found:
		action_test_records.append({"verb":verb,"ok":false,"message":"UI target missing: "+str(target)})
		return
	command_button.pressed.emit()
	action_test_records.append({"verb":verb,"actor":id,"ok":last_reply.get("ok",false),"message":last_reply.get("message","")})
func action_tick(n: int) -> void:
	invoke("step",[str(n)])
	action_test_records.append({"verb":"step","ok":last_reply.get("ok",false)})
func ui_action_smoke() -> void:
	debug_check.set_pressed_no_signal(true)
	set_debug(true)
	invoke("reset",["251025"])
	ui_action(4,"propose",5,1,0,{"price":2,"duration":20,"good":0,"source_site":0})
	ui_action(5,"counter",1,1,0,{"price":1})
	ui_action(4,"respond",1)
	ui_action(5,"propose",4,1,1,{"price":1,"duration":8})
	ui_action(4,"respond",2)
	ui_action(4,"repay",1,2)
	ui_action(4,"propose",5,1,3,{"price":1,"duration":8,"good":0})
	ui_action(5,"respond",3)
	ui_action(5,"fulfill",3)
	ui_action(1,"propose",4,1,4)
	ui_action(4,"respond",4)
	ui_action(4,"grant",5,5)
	ui_action(1,"dismiss")
	ui_action(2,"move",15)
	action_tick(1)
	ui_action(5,"move",15)
	action_tick(1)
	ui_action(2,"propose",5,1,2,{"price":2,"duration":2,"site":15})
	if "--capture" in OS.get_cmdline_user_args():
		await get_tree().process_frame
		await RenderingServer.frame_post_draw
		var capture_args:=OS.get_cmdline_user_args()
		var path: String=capture_args[capture_args.find("--capture")+1]
		get_viewport().get_texture().get_image().save_png(path)
	ui_action(5,"respond",5)
	ui_action(5,"rest")
	action_tick(3)
	action_test_records.append({"lease_expired_occupant_stays":world.leases.back().status==1 and world.people[4].location==15,"ok":world.leases.back().status==1 and world.people[4].location==15})
	ui_action(3,"revoke_job",12)
	ui_action(3,"produce",12)
	action_tick(9)
	ui_action(3,"equip",12,1,0,{"source_site":12})
	ui_action(4,"move",14)
	action_tick(1)
	ui_action(4,"collect")
	action_tick(1)
	ui_action(4,"move",15)
	action_tick(1)
	ui_action(4,"talk",5)
	ui_action(5,"invitation",1)
	ui_action(1,"announce",0,1,0,{},"Public office is vacant")
	var snapshot=world.duplicate(true)
	save_button.pressed.emit()
	step_button.pressed.emit()
	load_button.pressed.emit()
	action_test_records.append({"save_load_exact":world==snapshot,"ok":world==snapshot})
	action_test_records.append({"catalog_count":world.actions.size(),"ui_count":VERBS.size(),"ok":world.actions.size()==VERBS.size()})
	var passed := true
	for record in action_test_records:
		if not record.get("ok",false):
			passed=false
	var f:=FileAccess.open("user://ui-actions-a2.json",FileAccess.WRITE)
	f.store_string(JSON.stringify(action_test_records,"  "))
	print("UI_ACTIONS ",JSON.stringify(action_test_records))
	get_tree().quit(0 if passed else 1)
